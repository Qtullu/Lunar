// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "LunarTypes.h"
#include "LunarPerformanceSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FLunarPerformanceSnapshotUpdatedSignature,
	const FLunarPerformanceSnapshot&,
	Snapshot
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

	// FPS history.

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|History")
	FLunarPerformanceHistory GetFPSHistory() const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|History")
	TArray<FLunarPerformanceSnapshot> GetSnapshotHistory() const;

	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Performance|History")
	void ClearPerformanceHistory();

	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Performance|History")
	void SetHistoryDurationSeconds(float DurationSeconds = 60.0f);

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|History")
	float GetHistoryDurationSeconds() const;

	// Runtime detail level.
	// If runtime override is disabled, the subsystem uses DefaultDetailLevel from Project Settings.
	// If runtime override is enabled, the subsystem uses RuntimeDetailLevel.

	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Performance|Detail")
	void SetUseRuntimeDetailLevelOverride(bool bUseOverride);

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|Detail")
	bool IsUsingRuntimeDetailLevelOverride() const;

	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Performance|Detail")
	void SetRuntimeDetailLevel(ELunarPerformanceSummaryDetail DetailLevel);

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|Detail")
	ELunarPerformanceSummaryDetail GetRuntimeDetailLevel() const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|Detail")
	ELunarPerformanceSummaryDetail GetDefaultDetailLevel() const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Performance|Detail")
	ELunarPerformanceSummaryDetail GetEffectiveDetailLevel() const;

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

private:
	void ApplySettings();

	void AddSnapshotToHistory(const FLunarPerformanceSnapshot& Snapshot);
	void RecalculateFPSHistoryStats();

private:
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance", meta = (AllowPrivateAccess = "true"))
	FLunarPerformanceSnapshot CurrentSnapshot;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance", meta = (AllowPrivateAccess = "true"))
	bool bHasCurrentSnapshot = false;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|History", meta = (AllowPrivateAccess = "true"))
	FLunarPerformanceHistory FPSHistory;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|History", meta = (AllowPrivateAccess = "true"))
	TArray<FLunarPerformanceSnapshot> SnapshotHistory;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|History", meta = (AllowPrivateAccess = "true", ClampMin = "1.0", UIMin = "1.0"))
	float HistoryDurationSeconds = 60.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|Detail", meta = (AllowPrivateAccess = "true"))
	bool bUseRuntimeDetailLevelOverride = false;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|Detail", meta = (AllowPrivateAccess = "true"))
	ELunarPerformanceSummaryDetail RuntimeDetailLevel = ELunarPerformanceSummaryDetail::Normal;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Subsystems|Performance|Detail", meta = (AllowPrivateAccess = "true"))
	ELunarPerformanceSummaryDetail DefaultDetailLevel = ELunarPerformanceSummaryDetail::Normal;

	double LastSampleTimeSeconds = 0.0;

	float UpdateInterval = 1.0f;
	float TimeSinceLastUpdate = 0.0f;

	float CollectPerformanceDataInterval = 10.0f;
	float TimeSinceLastCollectPerformanceData = 0.0f;

	bool bMonitoringEnabled = false;
	bool bCollectPerformanceData = false;

	bool bEnabledInEditor = true;
	bool bEnabledInDebug = true;
	bool bEnabledInDevelopment = true;
	bool bEnabledInTest = true;
	bool bEnabledInShipping = false;

	bool bEnabledInCommandlet = false;
	bool bEnabledOnDedicatedServer = false;
};