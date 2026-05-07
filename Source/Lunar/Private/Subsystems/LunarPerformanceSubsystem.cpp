// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "Subsystems/LunarPerformanceSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "HAL/PlatformTime.h"
#include "LunarFL.h"
#include "Settings/LunarSettings.h"
#include "Subsystems/LunarRawInputSubsystem.h"

void ULunarPerformanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency(ULunarRawInputSubsystem::StaticClass());

	LastSampleTimeSeconds = 0.0;
	DetailLevel = ELunarPerformanceSummaryDetail::Low;

	ApplySettings();

	RecalculatePerformanceHistories();

	BindRawInputSubsystem();
}
void ULunarPerformanceSubsystem::Deinitialize()
{
	UnbindRawInputSubsystem();
	HidePerformanceWidget();

	bMonitoringEnabled = false;
	bCollectPerformanceData = false;

	TimeSinceLastUpdate = 0.0f;
	TimeSinceLastCollectPerformanceData = 0.0f;
	LastSampleTimeSeconds = 0.0;

	bHasCurrentSnapshot = false;
	CurrentSnapshot = FLunarPerformanceSnapshot();

	ClearPerformanceHistory();

	OnPerformanceSnapshotUpdated.Clear();
	OnDetailLevelChanged.Clear();

	Super::Deinitialize();
}

void ULunarPerformanceSubsystem::ApplySettings()
{
	const ULunarSettings* LunarSettings = GetDefault<ULunarSettings>();
	if (!LunarSettings)
	{
		return;
	}

	const FLunarPerformanceSettings& Settings = LunarSettings->Performance;

	UpdateInterval = FMath::Max(Settings.General.UpdateInterval, 0.05f);
	bMonitoringEnabled = Settings.General.bStartMonitoringEnabled;

	bCollectPerformanceData = Settings.Collection.bCollectPerformanceData;
	CollectPerformanceDataInterval = FMath::Max(Settings.Collection.CollectPerformanceDataInterval, 0.1f);

	bEnabledInEditor = Settings.Environment.bEnabledInEditor;
	bEnabledInDebug = Settings.Environment.bEnabledInDebug;
	bEnabledInDevelopment = Settings.Environment.bEnabledInDevelopment;
	bEnabledInTest = Settings.Environment.bEnabledInTest;
	bEnabledInShipping = Settings.Environment.bEnabledInShipping;
	bEnabledInCommandlet = Settings.Environment.bEnabledInCommandlet;
	bEnabledOnDedicatedServer = Settings.Environment.bEnabledOnDedicatedServer;

	bDisplayEnabled = Settings.Display.bDisplayEnabled;
	bDisplayEnabledInEditor = Settings.Display.bEnabledInEditor;
	bDisplayEnabledInDebug = Settings.Display.bEnabledInDebug;
	bDisplayEnabledInDevelopment = Settings.Display.bEnabledInDevelopment;
	bDisplayEnabledInTest = Settings.Display.bEnabledInTest;
	bDisplayEnabledInShipping = Settings.Display.bEnabledInShipping;

	bEnablePerformanceWidgetHotkey = Settings.Display.bEnableToggleHotkey;
	PerformanceWidgetKey = Settings.Display.ToggleWidgetHotkey;
	PerformanceWidgetClass = Settings.Display.WidgetClass.LoadSynchronous();
}

TStatId ULunarPerformanceSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULunarPerformanceSubsystem, STATGROUP_Tickables);
}

bool ULunarPerformanceSubsystem::IsTickable() const
{
	return bMonitoringEnabled && CanMonitorInCurrentEnvironment() && !IsTemplate();
}

void ULunarPerformanceSubsystem::Tick(float DeltaTime)
{
	TimeSinceLastUpdate += DeltaTime;

	if (bCollectPerformanceData)
	{
		TimeSinceLastCollectPerformanceData += DeltaTime;
	}

	if (TimeSinceLastUpdate < UpdateInterval)
	{
		return;
	}

	TimeSinceLastUpdate = 0.0f;

	bool bShouldCollectData = false;

	if (bCollectPerformanceData && TimeSinceLastCollectPerformanceData >= CollectPerformanceDataInterval)
	{
		bShouldCollectData = true;
		TimeSinceLastCollectPerformanceData = 0.0f;
	}

	UpdatePerformanceStats(bShouldCollectData);
}

void ULunarPerformanceSubsystem::SetMonitoringEnabled(bool bEnabled)
{
	if (bMonitoringEnabled == bEnabled)
	{
		return;
	}

	bMonitoringEnabled = bEnabled;

	TimeSinceLastUpdate = 0.0f;
	TimeSinceLastCollectPerformanceData = 0.0f;
	LastSampleTimeSeconds = 0.0;
}

bool ULunarPerformanceSubsystem::IsMonitoringEnabled() const
{
	return bMonitoringEnabled;
}

void ULunarPerformanceSubsystem::SetUpdateInterval(float Interval)
{
	UpdateInterval = FMath::Max(Interval, 0.05f);
	TimeSinceLastUpdate = 0.0f;
}

float ULunarPerformanceSubsystem::GetUpdateInterval() const
{
	return UpdateInterval;
}

float ULunarPerformanceSubsystem::GetSecondsSinceLastSample() const
{
	if (LastSampleTimeSeconds <= 0.0)
	{
		return 0.0f;
	}

	return static_cast<float>(FMath::Max(FPlatformTime::Seconds() - LastSampleTimeSeconds, 0.0));
}

void ULunarPerformanceSubsystem::UpdatePerformanceStats(bool bCollectData)
{
	CurrentSnapshot = LunarFL::Performance::GetPerformanceSnapshot(ELunarMemoryUnit::Gigabytes);
	bHasCurrentSnapshot = true;

	LastSampleTimeSeconds = FPlatformTime::Seconds();

	AddSnapshotToHistory(CurrentSnapshot);

	OnPerformanceSnapshotUpdated.Broadcast(CurrentSnapshot);

	if (bCollectData)
	{
		//TODO send to console
		
		//UE_LOG(
		//	LogTemp,
		//	Log,
		//	TEXT("LunarPerformanceSubsystem:\n%s"),
		//	*LunarFL::Performance::GetPerformanceSummaryMultilineString(DetailLevel, ELunarMemoryUnit::Megabytes)
		//);
	}
}

FLunarPerformanceSnapshot ULunarPerformanceSubsystem::GetCurrentSnapshot() const
{
	return CurrentSnapshot;
}

bool ULunarPerformanceSubsystem::HasCurrentSnapshot() const
{
	return bHasCurrentSnapshot;
}

TArray<FLunarPerformanceSnapshot> ULunarPerformanceSubsystem::GetSnapshotHistory() const
{
	return SnapshotHistory;
}

void ULunarPerformanceSubsystem::ClearPerformanceHistory()
{
	SnapshotHistory.Reset();

	RecalculatePerformanceHistories();
}

void ULunarPerformanceSubsystem::SetHistoryDurationSeconds(float DurationSeconds)
{
	HistoryDurationSeconds = FMath::Max(DurationSeconds, 1.0f);

	if (bHasCurrentSnapshot)
	{
		AddSnapshotToHistory(CurrentSnapshot);
	}
	else
	{
		RecalculatePerformanceHistories();
	}
}

float ULunarPerformanceSubsystem::GetHistoryDurationSeconds() const
{
	return HistoryDurationSeconds;
}

FLunarPerformanceFloatHistory ULunarPerformanceSubsystem::GetFPSHistory() const
{
	return FPSHistory;
}

FLunarPerformanceFloatHistory ULunarPerformanceSubsystem::GetFrameTimeHistory() const
{
	return FrameTimeHistory;
}

FLunarPerformanceFloatHistory ULunarPerformanceSubsystem::GetProcessMemoryHistory() const
{
	return ProcessMemoryHistory;
}

FLunarPerformanceFloatHistory ULunarPerformanceSubsystem::GetPhysicalMemoryUsedPercentHistory() const
{
	return PhysicalMemoryUsedPercentHistory;
}

FLunarPerformanceFloatHistory ULunarPerformanceSubsystem::GetSystemCPUUsageHistory() const
{
	return SystemCPUUsageHistory;
}

FLunarPerformanceFloatHistory ULunarPerformanceSubsystem::GetProcessCPUUsageHistory() const
{
	return ProcessCPUUsageHistory;
}

FLunarPerformanceFloatHistory ULunarPerformanceSubsystem::GetGPUUsageHistory() const
{
	return GPUUsageHistory;
}

FLunarPerformanceFloatHistory ULunarPerformanceSubsystem::GetTexturePoolUsedPercentHistory() const
{
	return TexturePoolUsedPercentHistory;
}

FLunarPerformanceFloatHistory ULunarPerformanceSubsystem::GetDiskReadSpeedHistory() const
{
	return DiskReadSpeedHistory;
}

FLunarPerformanceFloatHistory ULunarPerformanceSubsystem::GetDiskWriteSpeedHistory() const
{
	return DiskWriteSpeedHistory;
}

ELunarPerformanceSummaryDetail ULunarPerformanceSubsystem::SetDetailLevel(ELunarPerformanceSummaryDetail NewDetailLevel)
{
	if (DetailLevel == NewDetailLevel)
	{
		return DetailLevel;
	}

	DetailLevel = NewDetailLevel;

	OnDetailLevelChanged.Broadcast(DetailLevel);

	return DetailLevel;
}

ELunarPerformanceSummaryDetail ULunarPerformanceSubsystem::GetDetailLevel() const
{
	return DetailLevel;
}

bool ULunarPerformanceSubsystem::SetCollectPerformanceData(bool bCollect)
{
	if (bCollectPerformanceData == bCollect)
	{
		return false;
	}

	bCollectPerformanceData = bCollect;
	TimeSinceLastCollectPerformanceData = 0.0f;

	return true;
}

bool ULunarPerformanceSubsystem::IsCollectDataEnabled() const
{
	return bCollectPerformanceData;
}

bool ULunarPerformanceSubsystem::SetCollectPerformanceDataInterval(float Interval)
{
	const float NewInterval = FMath::Max(Interval, 0.1f);

	if (FMath::IsNearlyEqual(CollectPerformanceDataInterval, NewInterval))
	{
		return false;
	}

	CollectPerformanceDataInterval = NewInterval;
	TimeSinceLastCollectPerformanceData = 0.0f;

	return true;
}

float ULunarPerformanceSubsystem::GetCollectPerformanceDataInterval() const
{
	return CollectPerformanceDataInterval;
}

bool ULunarPerformanceSubsystem::CanMonitorInCurrentEnvironment() const
{
	if (LunarFL::Game::IsCommandlet() && !bEnabledInCommandlet)
	{
		return false;
	}

	if (LunarFL::Game::IsDedicatedServer() && !bEnabledOnDedicatedServer)
	{
		return false;
	}

	if (LunarFL::Game::IsEditor() && !bEnabledInEditor)
	{
		return false;
	}

	if (LunarFL::Game::IsDebug() && !bEnabledInDebug)
	{
		return false;
	}

	if (LunarFL::Game::IsDevelopment() && !bEnabledInDevelopment)
	{
		return false;
	}

	if (LunarFL::Game::IsTest() && !bEnabledInTest)
	{
		return false;
	}

	if (LunarFL::Game::IsShipping() && !bEnabledInShipping)
	{
		return false;
	}

	return true;
}

bool ULunarPerformanceSubsystem::IsPerformanceWidgetActive() const
{
	return PerformanceWidgetInstance && PerformanceWidgetInstance->IsInViewport();
}

void ULunarPerformanceSubsystem::ShowPerformanceWidget()
{
	if (!CanUsePerformanceWidgetHotkey())
	{
		return;
	}

	if (PerformanceWidgetInstance && PerformanceWidgetInstance->IsInViewport())
	{
		return;
	}

	if (!PerformanceWidgetInstance)
	{
		UWorld* World = GetWorld();

		if (!World)
		{
			return;
		}

		PerformanceWidgetInstance = CreateWidget<UUserWidget>(World, PerformanceWidgetClass);

		if (!PerformanceWidgetInstance)
		{
			return;
		}
	}

	PerformanceWidgetInstance->AddToViewport(PerformanceWidgetZOrder);
}

void ULunarPerformanceSubsystem::HidePerformanceWidget()
{
	if (PerformanceWidgetInstance)
	{
		PerformanceWidgetInstance->RemoveFromParent();
	}

	PerformanceWidgetInstance = nullptr;
}

void ULunarPerformanceSubsystem::TogglePerformanceWidgetDetailLevel()
{
	if (!IsPerformanceWidgetActive())
	{
		SetDetailLevel(ELunarPerformanceSummaryDetail::Low);
		ShowPerformanceWidget();
		return;
	}

	ELunarPerformanceSummaryDetail NextDetailLevel = ELunarPerformanceSummaryDetail::Low;

	if (TryGetNextDetailLevel(DetailLevel, NextDetailLevel))
	{
		SetDetailLevel(NextDetailLevel);
		return;
	}

	HidePerformanceWidget();
	SetDetailLevel(ELunarPerformanceSummaryDetail::Low);
}

void ULunarPerformanceSubsystem::BindRawInputSubsystem()
{
	if (!GetGameInstance())
	{
		return;
	}

	RawInputSubsystem = GetGameInstance()->GetSubsystem<ULunarRawInputSubsystem>();

	if (!RawInputSubsystem)
	{
		return;
	}

	RawInputSubsystem->OnKeyClicked.RemoveDynamic(this, &ULunarPerformanceSubsystem::HandleRawInputKeyClicked);
	RawInputSubsystem->OnKeyClicked.AddDynamic(this, &ULunarPerformanceSubsystem::HandleRawInputKeyClicked);
}

void ULunarPerformanceSubsystem::UnbindRawInputSubsystem()
{
	if (RawInputSubsystem)
	{
		RawInputSubsystem->OnKeyClicked.RemoveDynamic(this, &ULunarPerformanceSubsystem::HandleRawInputKeyClicked);
	}

	RawInputSubsystem = nullptr;
}

void ULunarPerformanceSubsystem::HandleRawInputKeyClicked(FKey Key)
{
	if (Key != PerformanceWidgetKey)
	{
		return;
	}

	if (!CanUsePerformanceWidgetHotkey())
	{
		return;
	}

	TogglePerformanceWidgetDetailLevel();
}

bool ULunarPerformanceSubsystem::CanUsePerformanceWidgetHotkey() const
{
	if (!bDisplayEnabled)
	{
		return false;
	}

	if (!bEnablePerformanceWidgetHotkey)
	{
		return false;
	}

	if (LunarFL::Game::IsCommandlet())
	{
		return false;
	}

	if (LunarFL::Game::IsDedicatedServer())
	{
		return false;
	}

	if (LunarFL::Game::IsEditor() && !bDisplayEnabledInEditor)
	{
		return false;
	}

	if (LunarFL::Game::IsDebug() && !bDisplayEnabledInDebug)
	{
		return false;
	}

	if (LunarFL::Game::IsDevelopment() && !bDisplayEnabledInDevelopment)
	{
		return false;
	}

	if (LunarFL::Game::IsTest() && !bDisplayEnabledInTest)
	{
		return false;
	}

	if (LunarFL::Game::IsShipping() && !bDisplayEnabledInShipping)
	{
		return false;
	}

	if (!PerformanceWidgetClass)
	{
		return false;
	}

	return true;
}

bool ULunarPerformanceSubsystem::TryGetNextDetailLevel(ELunarPerformanceSummaryDetail CurrentDetailLevel, ELunarPerformanceSummaryDetail& OutNextDetailLevel) const
{
	const UEnum* DetailEnum = StaticEnum<ELunarPerformanceSummaryDetail>();

	if (!DetailEnum)
	{
		return false;
	}

	const int64 CurrentValue = static_cast<int64>(CurrentDetailLevel);

	bool bFoundCurrent = false;

	for (int32 Index = 0; Index < DetailEnum->NumEnums(); ++Index)
	{
		if (DetailEnum->HasMetaData(TEXT("Hidden"), Index))
		{
			continue;
		}

		const FString EnumName = DetailEnum->GetNameStringByIndex(Index);

		if (EnumName.EndsWith(TEXT("MAX")) || EnumName.EndsWith(TEXT("Max")))
		{
			continue;
		}

		const int64 EnumValue = DetailEnum->GetValueByIndex(Index);

		if (bFoundCurrent)
		{
			OutNextDetailLevel = static_cast<ELunarPerformanceSummaryDetail>(EnumValue);
			return true;
		}

		if (EnumValue == CurrentValue)
		{
			bFoundCurrent = true;
		}
	}

	return false;
}

void ULunarPerformanceSubsystem::AddSnapshotToHistory(const FLunarPerformanceSnapshot& Snapshot)
{
	SnapshotHistory.Add(Snapshot);

	const float CurrentTime = Snapshot.AppTimeSeconds;
	const float OldestAllowedTime = CurrentTime - HistoryDurationSeconds;

	for (int32 Index = SnapshotHistory.Num() - 1; Index >= 0; --Index)
	{
		if (SnapshotHistory[Index].AppTimeSeconds < OldestAllowedTime)
		{
			SnapshotHistory.RemoveAt(Index);
		}
	}

	RecalculatePerformanceHistories();
}

void ULunarPerformanceSubsystem::RecalculatePerformanceHistories()
{
	ResetFloatHistory(FPSHistory);
	ResetFloatHistory(FrameTimeHistory);
	ResetFloatHistory(ProcessMemoryHistory);
	ResetFloatHistory(PhysicalMemoryUsedPercentHistory);
	ResetFloatHistory(SystemCPUUsageHistory);
	ResetFloatHistory(ProcessCPUUsageHistory);
	ResetFloatHistory(GPUUsageHistory);
	ResetFloatHistory(TexturePoolUsedPercentHistory);
	ResetFloatHistory(DiskReadSpeedHistory);
	ResetFloatHistory(DiskWriteSpeedHistory);

	if (SnapshotHistory.Num() <= 0)
	{
		return;
	}

	FPSHistory.Values.Reserve(SnapshotHistory.Num());
	FrameTimeHistory.Values.Reserve(SnapshotHistory.Num());
	ProcessMemoryHistory.Values.Reserve(SnapshotHistory.Num());
	PhysicalMemoryUsedPercentHistory.Values.Reserve(SnapshotHistory.Num());
	SystemCPUUsageHistory.Values.Reserve(SnapshotHistory.Num());
	ProcessCPUUsageHistory.Values.Reserve(SnapshotHistory.Num());
	GPUUsageHistory.Values.Reserve(SnapshotHistory.Num());
	TexturePoolUsedPercentHistory.Values.Reserve(SnapshotHistory.Num());
	DiskReadSpeedHistory.Values.Reserve(SnapshotHistory.Num());
	DiskWriteSpeedHistory.Values.Reserve(SnapshotHistory.Num());

	for (const FLunarPerformanceSnapshot& Snapshot : SnapshotHistory)
	{
		FPSHistory.Values.Add(Snapshot.Frame.FPSFloat);
		FrameTimeHistory.Values.Add(Snapshot.Frame.FrameTimeMS);

		ProcessMemoryHistory.Values.Add(Snapshot.Memory.ProcessPhysicalMemory);
		PhysicalMemoryUsedPercentHistory.Values.Add(Snapshot.Memory.PhysicalMemoryUsedPercent);

		SystemCPUUsageHistory.Values.Add(Snapshot.CPU.SystemCPUUsagePercent);
		ProcessCPUUsageHistory.Values.Add(Snapshot.CPU.ProcessCPUUsagePercent);

		GPUUsageHistory.Values.Add(Snapshot.GPU.ProcessGPU.TotalUsagePercent);
		TexturePoolUsedPercentHistory.Values.Add(Snapshot.GPU.TexturePoolUsedPercent);

		DiskReadSpeedHistory.Values.Add(Snapshot.Disk.ProcessReadSpeed);
		DiskWriteSpeedHistory.Values.Add(Snapshot.Disk.ProcessWriteSpeed);
	}

	RecalculateFloatHistoryStats(FPSHistory);
	RecalculateFloatHistoryStats(FrameTimeHistory);
	RecalculateFloatHistoryStats(ProcessMemoryHistory);
	RecalculateFloatHistoryStats(PhysicalMemoryUsedPercentHistory);
	RecalculateFloatHistoryStats(SystemCPUUsageHistory);
	RecalculateFloatHistoryStats(ProcessCPUUsageHistory);
	RecalculateFloatHistoryStats(GPUUsageHistory);
	RecalculateFloatHistoryStats(TexturePoolUsedPercentHistory);
	RecalculateFloatHistoryStats(DiskReadSpeedHistory);
	RecalculateFloatHistoryStats(DiskWriteSpeedHistory);
}

void ULunarPerformanceSubsystem::ResetFloatHistory(FLunarPerformanceFloatHistory& History) const
{
	History.Values.Reset();
	History.Average = 0.0f;
	History.Min = 0.0f;
	History.Max = 0.0f;
	History.HistoryDurationSeconds = HistoryDurationSeconds;
}

void ULunarPerformanceSubsystem::RecalculateFloatHistoryStats(FLunarPerformanceFloatHistory& History) const
{
	History.HistoryDurationSeconds = HistoryDurationSeconds;

	if (History.Values.Num() <= 0)
	{
		History.Average = 0.0f;
		History.Min = 0.0f;
		History.Max = 0.0f;
		return;
	}

	float Sum = 0.0f;
	float MinValue = TNumericLimits<float>::Max();
	float MaxValue = TNumericLimits<float>::Lowest();

	for (const float Value : History.Values)
	{
		Sum += Value;
		MinValue = FMath::Min(MinValue, Value);
		MaxValue = FMath::Max(MaxValue, Value);
	}

	History.Average = Sum / static_cast<float>(History.Values.Num());
	History.Min = MinValue;
	History.Max = MaxValue;
}