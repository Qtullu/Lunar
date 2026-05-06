// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Blueprint/UserWidget.h"
#include "LunarTypesPerformance.generated.h"

UENUM(BlueprintType)
enum class ELunarPerformanceSummaryDetail : uint8
{
	Low UMETA(DisplayName = "Low"),
	Normal UMETA(DisplayName = "Normal"),
	High UMETA(DisplayName = "High"),
	Full UMETA(DisplayName = "Full")
};

UENUM(BlueprintType)
enum class ELunarMemoryUnit : uint8
{
	Bytes UMETA(DisplayName = "Bytes"),
	Kilobytes UMETA(DisplayName = "Kilobytes"),
	Megabytes UMETA(DisplayName = "Megabytes"),
	Gigabytes UMETA(DisplayName = "Gigabytes"),
	Terabytes UMETA(DisplayName = "Terabytes")
};

USTRUCT(BlueprintType)
struct FLunarPerformanceHistory
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|History")
	TArray<float> FPSValues;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|History")
	float AverageFPS = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|History")
	float MinFPS = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|History")
	float MaxFPS = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|History")
	float HistoryDurationSeconds = 60.0f;
};

USTRUCT(BlueprintType)
struct FLunarPerformanceFrameStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|Frame")
	int32 FPS = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|Frame")
	float FPSFloat = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|Frame")
	float FrameTimeMS = 0.0f;
};

USTRUCT(BlueprintType)
struct FLunarPerformanceMemoryStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|RAM")
	float TotalPhysicalMemory = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|RAM")
	float UsedPhysicalMemory = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|RAM")
	float AvailablePhysicalMemory = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|RAM")
	float PhysicalMemoryUsedPercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|RAM")
	float ProcessPhysicalMemory = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|RAM")
	float PeakProcessPhysicalMemory = 0.0f;
};

USTRUCT(BlueprintType)
struct FLunarPerformanceProcessGPUStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	bool bStatsAvailable = false;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float TotalUsagePercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float Graphics3DUsagePercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float ComputeUsagePercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float CopyUsagePercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float VideoDecodeUsagePercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float VideoEncodeUsagePercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	FString Message;
};

USTRUCT(BlueprintType)
struct FLunarPerformanceGPUStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	bool bStatsAvailable = false;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float DedicatedVideoMemory = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float TotalGraphicsMemory = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float TotalDeviceWorkingMemory = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float RHITextureMemory = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float StreamingTextureMemory = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float NonStreamingTextureMemory = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float TexturePoolSize = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float TexturePoolFree = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float TexturePoolUsedPercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	FLunarPerformanceProcessGPUStats ProcessGPU;
};

USTRUCT(BlueprintType)
struct FLunarPerformanceCPUStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|CPU")
	bool bStatsAvailable = false;

	// Backward-compatible alias. Same as SystemCPUUsagePercent.
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|CPU")
	float CPUUsagePercent = 0.0f;

	// Whole machine CPU usage.
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|CPU")
	float SystemCPUUsagePercent = 0.0f;

	// Current process CPU usage.
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|CPU")
	float ProcessCPUUsagePercent = 0.0f;

	// Whole machine logical core usage.
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|CPU")
	TArray<float> CPUCoreUsagePercents;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|CPU")
	FString Message;
};

USTRUCT(BlueprintType)
struct FLunarPerformanceDiskStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|Disk")
	bool bStatsAvailable = false;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|Disk")
	float ProcessReadSpeed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|Disk")
	float ProcessWriteSpeed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|Disk")
	float ProjectDriveActivePercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|Disk")
	FString Message;
};

USTRUCT(BlueprintType)
struct FLunarPerformanceSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance")
	FLunarPerformanceFrameStats Frame;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance")
	FLunarPerformanceMemoryStats Memory;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance")
	FLunarPerformanceGPUStats GPU;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance")
	FLunarPerformanceCPUStats CPU;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance")
	FLunarPerformanceDiskStats Disk;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance")
	FDateTime Timestamp;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance")
	ELunarMemoryUnit Unit = ELunarMemoryUnit::Bytes;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance")
	float AppTimeSeconds = 0.0f;
};

USTRUCT(BlueprintType)
struct FLunarPerformanceGeneralSettings
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General", meta = (DisplayName = "Start Monitoring Enabled", ToolTip = "If enabled, the Lunar Performance Subsystem starts monitoring automatically when the Game Instance is initialized."))
	bool bStartMonitoringEnabled = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General", meta = (ClampMin = "0.05", UIMin = "0.05", DisplayName = "Update Interval", ToolTip = "How often performance stats are updated, in seconds. Lower values update the monitor more often."))
	float UpdateInterval = 1.0f;
};

USTRUCT(BlueprintType)
struct FLunarPerformanceCollectionSettings
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Collection", meta = (DisplayName = "Collect Performance Data", ToolTip = "If enabled, the subsystem periodically sends collected performance data to the logging or console system."))
	bool bCollectPerformanceData = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Collection", meta = (ClampMin = "0.1", UIMin = "0.1", DisplayName = "Collection Interval", ToolTip = "How often performance data is collected and sent to the logging or console system, in seconds."))
	float CollectPerformanceDataInterval = 10.0f;
};

USTRUCT(BlueprintType)
struct FLunarPerformanceEnvironmentSettings
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Environment", meta = (DisplayName = "Enabled In Editor", ToolTip = "Allows performance monitoring in editor builds."))
	bool bEnabledInEditor = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Environment", meta = (DisplayName = "Enabled In Debug", ToolTip = "Allows performance monitoring in Debug builds."))
	bool bEnabledInDebug = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Environment", meta = (DisplayName = "Enabled In Development", ToolTip = "Allows performance monitoring in Development builds."))
	bool bEnabledInDevelopment = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Environment", meta = (DisplayName = "Enabled In Test", ToolTip = "Allows performance monitoring in Test builds."))
	bool bEnabledInTest = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Environment", meta = (DisplayName = "Enabled In Shipping", ToolTip = "Allows performance monitoring in Shipping builds. Usually disabled by default."))
	bool bEnabledInShipping = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Environment", meta = (DisplayName = "Enabled In Commandlet", ToolTip = "Allows performance monitoring while running commandlets. Usually disabled by default."))
	bool bEnabledInCommandlet = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Environment", meta = (DisplayName = "Enabled On Dedicated Server", ToolTip = "Allows performance monitoring on dedicated servers. Usually disabled by default."))
	bool bEnabledOnDedicatedServer = false;
};

USTRUCT(BlueprintType)
struct FLunarPerformanceDisplaySettings
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Display", meta = (DisplayName = "Display Enabled", ToolTip = "If disabled, the performance widget UI is not created, shown, or controlled by hotkeys. Performance data can still be collected by the subsystem."))
	bool bDisplayEnabled = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Display", meta = (DisplayName = "Widget Class", ToolTip = "Widget class used to display Lunar performance data."))
	TSoftClassPtr<UUserWidget> WidgetClass = TSoftClassPtr<UUserWidget>(FSoftClassPath(TEXT("/Lunar/Widgets/Global/W_Performance.W_Performance_C")));

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Display|Input", meta = (DisplayName = "Enable Toggle Hotkey", ToolTip = "If enabled, the performance widget can be shown or hidden using the configured hotkey."))
	bool bEnableToggleHotkey = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Display|Input", meta = (DisplayName = "Toggle Widget Hotkey", ToolTip = "Keyboard key used to show or hide the performance widget."))
	FKey ToggleWidgetHotkey = EKeys::F10;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Display", meta = (DisplayName = "Default Detail Level", ToolTip = "Default amount of performance information shown by the performance widget."))
	ELunarPerformanceSummaryDetail DefaultDetailLevel = ELunarPerformanceSummaryDetail::Full;
};

USTRUCT(BlueprintType)
struct FLunarPerformanceSettings
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (DisplayName = "General", ToolTip = "General performance monitoring settings."))
	FLunarPerformanceGeneralSettings General;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (DisplayName = "Collection", ToolTip = "Periodic performance data collection settings."))
	FLunarPerformanceCollectionSettings Collection;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (DisplayName = "Environment", ToolTip = "Controls in which build and runtime environments performance monitoring is allowed."))
	FLunarPerformanceEnvironmentSettings Environment;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (DisplayName = "Display", ToolTip = "Default widget and UI display settings for the performance monitor."))
	FLunarPerformanceDisplaySettings Display;
};