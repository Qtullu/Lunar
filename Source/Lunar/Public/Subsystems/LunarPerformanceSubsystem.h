// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "LunarTypes.h"
#include "LunarPerformanceSubsystem.generated.h"

/**
 * @file LunarPerformanceSubsystem.h
 * @brief Performance subsystem
 * @ingroup LunarPerformanceSubsystem
 */

class ULunarRawInputSubsystem;
class UUserWidget;

/**
 * @brief Called when performance snapshot is updated
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FLunarPerformanceSnapshotUpdatedSignature,
	const FLunarPerformanceSnapshot&,
	Snapshot
);

/**
 * @brief Called when performance detail level is changed
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FLunarPerformanceDetailLevelChangedSignature,
	ELunarPerformanceSummaryDetail,
	NewDetailLevel
);

/**
 * @brief Collects runtime performance data stores histories and controls performance widget state
 * @ingroup LunarPerformanceSubsystem
 */
UCLASS()
class LUNAR_API ULunarPerformanceSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	/**
	 * @brief Initializes performance subsystem state
	 * @param Collection Subsystem collection
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * @brief Deinitializes performance subsystem state
	 */
	virtual void Deinitialize() override;

	/**
	 * @brief Updates performance subsystem tick logic
	 * @param DeltaTime Time passed since last tick
	 */
	virtual void Tick(float DeltaTime) override;

	/**
	 * @brief Gets tick stat identifier
	 * @return Tick stat identifier
	 */
	virtual TStatId GetStatId() const override;

	/**
	 * @brief Checks whether subsystem can tick
	 * @return True if subsystem can tick
	 */
	virtual bool IsTickable() const override;

	// Events

	/** Performance snapshot updated event */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|Subsystems|Performance")
	FLunarPerformanceSnapshotUpdatedSignature OnPerformanceSnapshotUpdated;

	/** Performance detail level changed event */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|Subsystems|Performance|Detail")
	FLunarPerformanceDetailLevelChangedSignature OnDetailLevelChanged;

	// Runtime monitoring control

	/**
	 * @brief Enables or disables performance monitoring
	 * @param bEnabled New monitoring state
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Performance")
	void SetMonitoringEnabled(bool bEnabled);

	/**
	 * @brief Checks whether performance monitoring is enabled
	 * @return True if performance monitoring is enabled
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance")
	bool IsMonitoringEnabled() const;

	// How often performance monitor updates in seconds

	/**
	 * @brief Sets performance update interval
	 * @param Interval Update interval in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Performance")
	void SetUpdateInterval(float Interval = 1.0f);

	/**
	 * @brief Gets performance update interval
	 * @return Update interval in seconds
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance")
	float GetUpdateInterval() const;

	// Returns seconds since the last performance sample was collected
	// Useful for smooth moving performance graphs between subsystem updates

	/**
	 * @brief Gets seconds since last performance sample
	 * @return Seconds since last performance sample
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance")
	float GetSecondsSinceLastSample() const;

	// Manually updates performance stats
	// If bCollectData is true data is also sent to the console or log system

	/**
	 * @brief Updates performance statistics immediately
	 * @param bCollectData Sends collected data to console or log system
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Performance")
	void UpdatePerformanceStats(bool bCollectData = false);

	// Snapshot

	/**
	 * @brief Gets current performance snapshot
	 * @return Current performance snapshot
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance")
	FLunarPerformanceSnapshot GetCurrentSnapshot() const;

	/**
	 * @brief Checks whether current performance snapshot exists
	 * @return True if current snapshot exists
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance")
	bool HasCurrentSnapshot() const;

	// Snapshot history

	/**
	 * @brief Gets stored performance snapshot history
	 * @return Performance snapshot history
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|History")
	TArray<FLunarPerformanceSnapshot> GetSnapshotHistory() const;

	/**
	 * @brief Clears stored performance snapshot history
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Performance|History")
	void ClearPerformanceHistory();

	/**
	 * @brief Sets performance history duration
	 * @param DurationSeconds History duration in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Performance|History")
	void SetHistoryDurationSeconds(float DurationSeconds = 60.0f);

	/**
	 * @brief Gets performance history duration
	 * @return History duration in seconds
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|History")
	float GetHistoryDurationSeconds() const;

	// Float histories

	/**
	 * @brief Gets FPS history
	 * @return FPS history
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|History")
	FLunarPerformanceFloatHistory GetFPSHistory() const;

	/**
	 * @brief Gets frame time history
	 * @return Frame time history
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|History")
	FLunarPerformanceFloatHistory GetFrameTimeHistory() const;

	/**
	 * @brief Gets process memory history
	 * @return Process memory history
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|History")
	FLunarPerformanceFloatHistory GetProcessMemoryHistory() const;

	/**
	 * @brief Gets physical memory used percent history
	 * @return Physical memory used percent history
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|History")
	FLunarPerformanceFloatHistory GetPhysicalMemoryUsedPercentHistory() const;

	/**
	 * @brief Gets system CPU usage history
	 * @return System CPU usage history
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|History")
	FLunarPerformanceFloatHistory GetSystemCPUUsageHistory() const;

	/**
	 * @brief Gets process CPU usage history
	 * @return Process CPU usage history
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|History")
	FLunarPerformanceFloatHistory GetProcessCPUUsageHistory() const;

	/**
	 * @brief Gets GPU usage history
	 * @return GPU usage history
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|History")
	FLunarPerformanceFloatHistory GetGPUUsageHistory() const;

	/**
	 * @brief Gets texture pool used percent history
	 * @return Texture pool used percent history
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|History")
	FLunarPerformanceFloatHistory GetTexturePoolUsedPercentHistory() const;

	/**
	 * @brief Gets disk read speed history
	 * @return Disk read speed history
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|History")
	FLunarPerformanceFloatHistory GetDiskReadSpeedHistory() const;

	/**
	 * @brief Gets disk write speed history
	 * @return Disk write speed history
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|History")
	FLunarPerformanceFloatHistory GetDiskWriteSpeedHistory() const;

	// Detail level

	/**
	 * @brief Sets performance summary detail level
	 * @param NewDetailLevel New detail level
	 * @return Applied detail level
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Performance|Detail")
	ELunarPerformanceSummaryDetail SetDetailLevel(ELunarPerformanceSummaryDetail NewDetailLevel);

	/**
	 * @brief Gets performance summary detail level
	 * @return Current detail level
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|Detail")
	ELunarPerformanceSummaryDetail GetDetailLevel() const;

	// Data collection

	/**
	 * @brief Enables or disables periodic performance data collection
	 * @param bCollect New collection state
	 * @return True if collection state was applied
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Performance")
	bool SetCollectPerformanceData(bool bCollect);

	/**
	 * @brief Checks whether periodic performance data collection is enabled
	 * @return True if collection is enabled
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance")
	bool IsCollectDataEnabled() const;

	/**
	 * @brief Sets periodic performance data collection interval
	 * @param Interval Collection interval in seconds
	 * @return True if collection interval was applied
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Performance")
	bool SetCollectPerformanceDataInterval(float Interval = 10.0f);

	/**
	 * @brief Gets periodic performance data collection interval
	 * @return Collection interval in seconds
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance")
	float GetCollectPerformanceDataInterval() const;

	/**
	 * @brief Checks whether performance monitoring can run in current environment
	 * @return True if performance monitoring can run
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance")
	bool CanMonitorInCurrentEnvironment() const;

	// Performance widget

	/**
	 * @brief Checks whether performance widget is active
	 * @return True if performance widget is active
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|Widget")
	bool IsPerformanceWidgetActive() const;

	/**
	 * @brief Shows performance widget
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Performance|Widget")
	void ShowPerformanceWidget();

	/**
	 * @brief Hides performance widget
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Performance|Widget")
	void HidePerformanceWidget();

	/**
	 * @brief Toggles performance widget detail level
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Performance|Widget")
	void TogglePerformanceWidgetDetailLevel();

private:
	/**
	 * @brief Applies settings to performance subsystem runtime state
	 */
	void ApplySettings();

	/**
	 * @brief Adds snapshot to performance history
	 * @param Snapshot Snapshot to add
	 */
	void AddSnapshotToHistory(const FLunarPerformanceSnapshot& Snapshot);

	/**
	 * @brief Recalculates all performance float histories
	 */
	void RecalculatePerformanceHistories();

	/**
	 * @brief Resets float history values and statistics
	 * @param History History to reset
	 */
	void ResetFloatHistory(FLunarPerformanceFloatHistory& History) const;

	/**
	 * @brief Recalculates float history statistics
	 * @param History History to recalculate
	 */
	void RecalculateFloatHistoryStats(FLunarPerformanceFloatHistory& History) const;

	/**
	 * @brief Binds raw input subsystem events used by performance widget hotkey
	 */
	void BindRawInputSubsystem();

	/**
	 * @brief Unbinds raw input subsystem events used by performance widget hotkey
	 */
	void UnbindRawInputSubsystem();

	/**
	 * @brief Handles raw input key click events
	 * @param Key Clicked key
	 */
	UFUNCTION()
	void HandleRawInputKeyClicked(FKey Key);

	/**
	 * @brief Checks whether performance widget hotkey can be used
	 * @return True if performance widget hotkey can be used
	 */
	bool CanUsePerformanceWidgetHotkey() const;

	/**
	 * @brief Resolves next performance detail level
	 * @param CurrentDetailLevel Current detail level
	 * @param OutNextDetailLevel Next detail level
	 * @return True if next detail level was resolved
	 */
	bool TryGetNextDetailLevel(ELunarPerformanceSummaryDetail CurrentDetailLevel, ELunarPerformanceSummaryDetail& OutNextDetailLevel) const;

private:
	/** Current performance snapshot */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance", meta = (AllowPrivateAccess = "true"))
	FLunarPerformanceSnapshot CurrentSnapshot;

	/** True if current performance snapshot exists */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance", meta = (AllowPrivateAccess = "true"))
	bool bHasCurrentSnapshot = false;

	/** Stored performance snapshot history */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|History", meta = (AllowPrivateAccess = "true"))
	TArray<FLunarPerformanceSnapshot> SnapshotHistory;

	/** FPS value history */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|History", meta = (AllowPrivateAccess = "true"))
	FLunarPerformanceFloatHistory FPSHistory;

	/** Frame time value history */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|History", meta = (AllowPrivateAccess = "true"))
	FLunarPerformanceFloatHistory FrameTimeHistory;

	/** Process memory value history */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|History", meta = (AllowPrivateAccess = "true"))
	FLunarPerformanceFloatHistory ProcessMemoryHistory;

	/** Physical memory used percent history */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|History", meta = (AllowPrivateAccess = "true"))
	FLunarPerformanceFloatHistory PhysicalMemoryUsedPercentHistory;

	/** System CPU usage history */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|History", meta = (AllowPrivateAccess = "true"))
	FLunarPerformanceFloatHistory SystemCPUUsageHistory;

	/** Process CPU usage history */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|History", meta = (AllowPrivateAccess = "true"))
	FLunarPerformanceFloatHistory ProcessCPUUsageHistory;

	/** GPU usage history */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|History", meta = (AllowPrivateAccess = "true"))
	FLunarPerformanceFloatHistory GPUUsageHistory;

	/** Texture pool used percent history */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|History", meta = (AllowPrivateAccess = "true"))
	FLunarPerformanceFloatHistory TexturePoolUsedPercentHistory;

	/** Disk read speed history */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|History", meta = (AllowPrivateAccess = "true"))
	FLunarPerformanceFloatHistory DiskReadSpeedHistory;

	/** Disk write speed history */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|History", meta = (AllowPrivateAccess = "true"))
	FLunarPerformanceFloatHistory DiskWriteSpeedHistory;

	/** History duration in seconds */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|History", meta = (AllowPrivateAccess = "true", ClampMin = "1.0", UIMin = "1.0"))
	float HistoryDurationSeconds = 60.0f;

	/** Current performance summary detail level */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|Detail", meta = (AllowPrivateAccess = "true"))
	ELunarPerformanceSummaryDetail DetailLevel = ELunarPerformanceSummaryDetail::Low;

	/** Cached raw input subsystem */
	UPROPERTY(Transient)
	TObjectPtr<ULunarRawInputSubsystem> RawInputSubsystem = nullptr;

	/** Active performance widget instance */
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> PerformanceWidgetInstance = nullptr;

	/** Loaded performance widget class */
	UPROPERTY(Transient)
	TSubclassOf<UUserWidget> PerformanceWidgetClass;

	/** Last performance sample time in seconds */
	double LastSampleTimeSeconds = 0.0;

	/** Performance update interval in seconds */
	float UpdateInterval = 1.0f;

	/** Time since last performance update */
	float TimeSinceLastUpdate = 0.0f;

	/** Performance data collection interval in seconds */
	float CollectPerformanceDataInterval = 10.0f;

	/** Time since last performance data collection */
	float TimeSinceLastCollectPerformanceData = 0.0f;

	/** Performance widget Z order */
	int32 PerformanceWidgetZOrder = 9999;

	/** Performance widget toggle key */
	FKey PerformanceWidgetKey = EKeys::F10;

	/** Runtime performance monitoring state */
	bool bMonitoringEnabled = false;

	/** Runtime performance data collection state */
	bool bCollectPerformanceData = false;

	/** Allows monitoring in editor builds */
	bool bEnabledInEditor = true;

	/** Allows monitoring in debug builds */
	bool bEnabledInDebug = true;

	/** Allows monitoring in development builds */
	bool bEnabledInDevelopment = true;

	/** Allows monitoring in test builds */
	bool bEnabledInTest = true;

	/** Allows monitoring in shipping builds */
	bool bEnabledInShipping = false;

	/** Allows monitoring in commandlets */
	bool bEnabledInCommandlet = false;

	/** Allows monitoring on dedicated servers */
	bool bEnabledOnDedicatedServer = false;

	/** Enables performance widget display */
	bool bDisplayEnabled = true;

	/** Allows performance widget in editor builds */
	bool bDisplayEnabledInEditor = true;

	/** Allows performance widget in debug builds */
	bool bDisplayEnabledInDebug = true;

	/** Allows performance widget in development builds */
	bool bDisplayEnabledInDevelopment = true;

	/** Allows performance widget in test builds */
	bool bDisplayEnabledInTest = true;

	/** Allows performance widget in shipping builds */
	bool bDisplayEnabledInShipping = false;

	/** Enables performance widget hotkey */
	bool bEnablePerformanceWidgetHotkey = true;
};