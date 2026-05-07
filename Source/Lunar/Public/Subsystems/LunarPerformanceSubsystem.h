// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "LunarTypes.h"
#include "LunarPerformanceSubsystem.generated.h"

class ULunarRawInputSubsystem;
class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FLunarPerformanceSnapshotUpdatedSignature,
	const FLunarPerformanceSnapshot&,
	Snapshot
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FLunarPerformanceDetailLevelChangedSignature,
	ELunarPerformanceSummaryDetail,
	NewDetailLevel
);

UCLASS()
class LUNAR_API ULunarPerformanceSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;

	// Events.

	UPROPERTY(BlueprintAssignable, Category = "Lunar|Subsystems|Performance")
	FLunarPerformanceSnapshotUpdatedSignature OnPerformanceSnapshotUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Lunar|Subsystems|Performance|Detail")
	FLunarPerformanceDetailLevelChangedSignature OnDetailLevelChanged;

	// Runtime monitoring control.

	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Performance")
	void SetMonitoringEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance")
	bool IsMonitoringEnabled() const;

	// How often performance monitor updates, in seconds.

	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Performance")
	void SetUpdateInterval(float Interval = 1.0f);

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance")
	float GetUpdateInterval() const;

	// Returns seconds since the last performance sample was collected.
	// Useful for smooth moving performance graphs between subsystem updates.

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance")
	float GetSecondsSinceLastSample() const;

	// Manually updates performance stats. If bCollectData is true, data is also sent to the console/log system.

	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Performance")
	void UpdatePerformanceStats(bool bCollectData = false);

	// Snapshot.

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance")
	FLunarPerformanceSnapshot GetCurrentSnapshot() const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance")
	bool HasCurrentSnapshot() const;

	// Snapshot history.

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|History")
	TArray<FLunarPerformanceSnapshot> GetSnapshotHistory() const;

	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Performance|History")
	void ClearPerformanceHistory();

	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Performance|History")
	void SetHistoryDurationSeconds(float DurationSeconds = 60.0f);

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|History")
	float GetHistoryDurationSeconds() const;

	// Float histories.

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|History")
	FLunarPerformanceFloatHistory GetFPSHistory() const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|History")
	FLunarPerformanceFloatHistory GetFrameTimeHistory() const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|History")
	FLunarPerformanceFloatHistory GetProcessMemoryHistory() const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|History")
	FLunarPerformanceFloatHistory GetPhysicalMemoryUsedPercentHistory() const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|History")
	FLunarPerformanceFloatHistory GetSystemCPUUsageHistory() const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|History")
	FLunarPerformanceFloatHistory GetProcessCPUUsageHistory() const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|History")
	FLunarPerformanceFloatHistory GetGPUUsageHistory() const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|History")
	FLunarPerformanceFloatHistory GetTexturePoolUsedPercentHistory() const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|History")
	FLunarPerformanceFloatHistory GetDiskReadSpeedHistory() const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|History")
	FLunarPerformanceFloatHistory GetDiskWriteSpeedHistory() const;

	// Detail level.

	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Performance|Detail")
	ELunarPerformanceSummaryDetail SetDetailLevel(ELunarPerformanceSummaryDetail NewDetailLevel);

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|Detail")
	ELunarPerformanceSummaryDetail GetDetailLevel() const;

	// Data collection.

	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Performance")
	bool SetCollectPerformanceData(bool bCollect);

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance")
	bool IsCollectDataEnabled() const;

	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Performance")
	bool SetCollectPerformanceDataInterval(float Interval = 10.0f);

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance")
	float GetCollectPerformanceDataInterval() const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance")
	bool CanMonitorInCurrentEnvironment() const;

	// Performance widget.

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|Widget")
	bool IsPerformanceWidgetActive() const;

	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Performance|Widget")
	void ShowPerformanceWidget();

	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Performance|Widget")
	void HidePerformanceWidget();

	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Performance|Widget")
	void TogglePerformanceWidgetDetailLevel();

private:
	void ApplySettings();

	void AddSnapshotToHistory(const FLunarPerformanceSnapshot& Snapshot);
	void RecalculatePerformanceHistories();

	void ResetFloatHistory(FLunarPerformanceFloatHistory& History) const;
	void RecalculateFloatHistoryStats(FLunarPerformanceFloatHistory& History) const;

	void BindRawInputSubsystem();
	void UnbindRawInputSubsystem();

	UFUNCTION()
	void HandleRawInputKeyClicked(FKey Key);

	bool CanUsePerformanceWidgetHotkey() const;
	bool TryGetNextDetailLevel(ELunarPerformanceSummaryDetail CurrentDetailLevel, ELunarPerformanceSummaryDetail& OutNextDetailLevel) const;

private:
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance", meta = (AllowPrivateAccess = "true"))
	FLunarPerformanceSnapshot CurrentSnapshot;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance", meta = (AllowPrivateAccess = "true"))
	bool bHasCurrentSnapshot = false;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|History", meta = (AllowPrivateAccess = "true"))
	TArray<FLunarPerformanceSnapshot> SnapshotHistory;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|History", meta = (AllowPrivateAccess = "true"))
	FLunarPerformanceFloatHistory FPSHistory;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|History", meta = (AllowPrivateAccess = "true"))
	FLunarPerformanceFloatHistory FrameTimeHistory;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|History", meta = (AllowPrivateAccess = "true"))
	FLunarPerformanceFloatHistory ProcessMemoryHistory;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|History", meta = (AllowPrivateAccess = "true"))
	FLunarPerformanceFloatHistory PhysicalMemoryUsedPercentHistory;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|History", meta = (AllowPrivateAccess = "true"))
	FLunarPerformanceFloatHistory SystemCPUUsageHistory;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|History", meta = (AllowPrivateAccess = "true"))
	FLunarPerformanceFloatHistory ProcessCPUUsageHistory;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|History", meta = (AllowPrivateAccess = "true"))
	FLunarPerformanceFloatHistory GPUUsageHistory;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|History", meta = (AllowPrivateAccess = "true"))
	FLunarPerformanceFloatHistory TexturePoolUsedPercentHistory;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|History", meta = (AllowPrivateAccess = "true"))
	FLunarPerformanceFloatHistory DiskReadSpeedHistory;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|History", meta = (AllowPrivateAccess = "true"))
	FLunarPerformanceFloatHistory DiskWriteSpeedHistory;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|History", meta = (AllowPrivateAccess = "true", ClampMin = "1.0", UIMin = "1.0"))
	float HistoryDurationSeconds = 60.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|Detail", meta = (AllowPrivateAccess = "true"))
	ELunarPerformanceSummaryDetail DetailLevel = ELunarPerformanceSummaryDetail::Low;

	UPROPERTY(Transient)
	TObjectPtr<ULunarRawInputSubsystem> RawInputSubsystem = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> PerformanceWidgetInstance = nullptr;

	UPROPERTY(Transient)
	TSubclassOf<UUserWidget> PerformanceWidgetClass;

	double LastSampleTimeSeconds = 0.0;

	float UpdateInterval = 1.0f;
	float TimeSinceLastUpdate = 0.0f;

	float CollectPerformanceDataInterval = 10.0f;
	float TimeSinceLastCollectPerformanceData = 0.0f;

	int32 PerformanceWidgetZOrder = 9999;

	FKey PerformanceWidgetKey = EKeys::F10;

	bool bMonitoringEnabled = false;
	bool bCollectPerformanceData = false;

	bool bEnabledInEditor = true;
	bool bEnabledInDebug = true;
	bool bEnabledInDevelopment = true;
	bool bEnabledInTest = true;
	bool bEnabledInShipping = false;

	bool bEnabledInCommandlet = false;
	bool bEnabledOnDedicatedServer = false;

	bool bDisplayEnabled = true;
	bool bDisplayEnabledInEditor = true;
	bool bDisplayEnabledInDebug = true;
	bool bDisplayEnabledInDevelopment = true;
	bool bDisplayEnabledInTest = true;
	bool bDisplayEnabledInShipping = false;

	bool bEnablePerformanceWidgetHotkey = true;
};