// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "Subsystems/LunarPerformanceSubsystem.h"

#include "HAL/PlatformTime.h"
#include "LunarFL.h"
#include "Settings/LunarSettings.h"

void ULunarPerformanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LastSampleTimeSeconds = 0.0;

	ApplySettings();

	FPSHistory.HistoryDurationSeconds = HistoryDurationSeconds;
}

void ULunarPerformanceSubsystem::Deinitialize()
{
	bMonitoringEnabled = false;
	bCollectPerformanceData = false;

	TimeSinceLastUpdate = 0.0f;
	TimeSinceLastCollectPerformanceData = 0.0f;
	LastSampleTimeSeconds = 0.0;

	bHasCurrentSnapshot = false;
	CurrentSnapshot = FLunarPerformanceSnapshot();

	ClearPerformanceHistory();

	OnPerformanceSnapshotUpdated.Clear();

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

	DefaultDetailLevel = Settings.Display.DefaultDetailLevel;

	if (!bUseRuntimeDetailLevelOverride)
	{
		RuntimeDetailLevel = DefaultDetailLevel;
	}

	FPSHistory.HistoryDurationSeconds = HistoryDurationSeconds;
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

void ULunarPerformanceSubsystem::UpdatePerformanceStats(bool bCollectData)
{
	CurrentSnapshot = LunarFL::Performance::GetPerformanceSnapshot(ELunarMemoryUnit::Gigabytes);
	bHasCurrentSnapshot = true;

	LastSampleTimeSeconds = FPlatformTime::Seconds();

	AddSnapshotToHistory(CurrentSnapshot);

	OnPerformanceSnapshotUpdated.Broadcast(CurrentSnapshot);

	if (bCollectData)
	{
		const ELunarPerformanceSummaryDetail EffectiveDetailLevel = GetEffectiveDetailLevel();

		UE_LOG(
			LogTemp,
			Log,
			TEXT("LunarPerformanceSubsystem:\n%s"),
			*LunarFL::Performance::GetPerformanceSummaryMultilineString(EffectiveDetailLevel, ELunarMemoryUnit::Megabytes)
		);
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

FLunarPerformanceHistory ULunarPerformanceSubsystem::GetFPSHistory() const
{
	return FPSHistory;
}

TArray<FLunarPerformanceSnapshot> ULunarPerformanceSubsystem::GetSnapshotHistory() const
{
	return SnapshotHistory;
}

float ULunarPerformanceSubsystem::GetSecondsSinceLastSample() const
{
	if (LastSampleTimeSeconds <= 0.0)
	{
		return 0.0f;
	}

	return static_cast<float>(
		FMath::Max(FPlatformTime::Seconds() - LastSampleTimeSeconds, 0.0)
		);
}

void ULunarPerformanceSubsystem::ClearPerformanceHistory()
{
	SnapshotHistory.Reset();

	FPSHistory.FPSValues.Reset();
	FPSHistory.AverageFPS = 0.0f;
	FPSHistory.MinFPS = 0.0f;
	FPSHistory.MaxFPS = 0.0f;
	FPSHistory.HistoryDurationSeconds = HistoryDurationSeconds;
}

void ULunarPerformanceSubsystem::SetHistoryDurationSeconds(float DurationSeconds)
{
	HistoryDurationSeconds = FMath::Max(DurationSeconds, 1.0f);
	FPSHistory.HistoryDurationSeconds = HistoryDurationSeconds;

	if (bHasCurrentSnapshot)
	{
		AddSnapshotToHistory(CurrentSnapshot);
	}
	else
	{
		RecalculateFPSHistoryStats();
	}
}

float ULunarPerformanceSubsystem::GetHistoryDurationSeconds() const
{
	return HistoryDurationSeconds;
}

void ULunarPerformanceSubsystem::SetUseRuntimeDetailLevelOverride(bool bUseOverride)
{
	if (bUseRuntimeDetailLevelOverride == bUseOverride)
	{
		return;
	}

	bUseRuntimeDetailLevelOverride = bUseOverride;

	if (!bUseRuntimeDetailLevelOverride)
	{
		RuntimeDetailLevel = DefaultDetailLevel;
	}
}

bool ULunarPerformanceSubsystem::IsUsingRuntimeDetailLevelOverride() const
{
	return bUseRuntimeDetailLevelOverride;
}

void ULunarPerformanceSubsystem::SetRuntimeDetailLevel(ELunarPerformanceSummaryDetail DetailLevel)
{
	RuntimeDetailLevel = DetailLevel;
	bUseRuntimeDetailLevelOverride = true;
}

ELunarPerformanceSummaryDetail ULunarPerformanceSubsystem::GetRuntimeDetailLevel() const
{
	return RuntimeDetailLevel;
}

ELunarPerformanceSummaryDetail ULunarPerformanceSubsystem::GetDefaultDetailLevel() const
{
	return DefaultDetailLevel;
}

ELunarPerformanceSummaryDetail ULunarPerformanceSubsystem::GetEffectiveDetailLevel() const
{
	return bUseRuntimeDetailLevelOverride ? RuntimeDetailLevel : DefaultDetailLevel;
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

	RecalculateFPSHistoryStats();
}

void ULunarPerformanceSubsystem::RecalculateFPSHistoryStats()
{
	FPSHistory.FPSValues.Reset();
	FPSHistory.HistoryDurationSeconds = HistoryDurationSeconds;

	if (SnapshotHistory.Num() <= 0)
	{
		FPSHistory.AverageFPS = 0.0f;
		FPSHistory.MinFPS = 0.0f;
		FPSHistory.MaxFPS = 0.0f;
		return;
	}

	float SumFPS = 0.0f;
	float MinFPS = TNumericLimits<float>::Max();
	float MaxFPS = TNumericLimits<float>::Lowest();

	FPSHistory.FPSValues.Reserve(SnapshotHistory.Num());

	for (const FLunarPerformanceSnapshot& Snapshot : SnapshotHistory)
	{
		const float FPSValue = Snapshot.Frame.FPSFloat;

		FPSHistory.FPSValues.Add(FPSValue);

		SumFPS += FPSValue;
		MinFPS = FMath::Min(MinFPS, FPSValue);
		MaxFPS = FMath::Max(MaxFPS, FPSValue);
	}

	FPSHistory.AverageFPS = SumFPS / static_cast<float>(SnapshotHistory.Num());
	FPSHistory.MinFPS = MinFPS;
	FPSHistory.MaxFPS = MaxFPS;
}