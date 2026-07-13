// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Blueprint/UserWidget.h"
#include "LunarTypesPerformance.generated.h"

/**
 * @file LunarTypesPerformance.h
 * @brief Performance shared types
 * @ingroup LunarTypesPerformance
 */

 /**
  * @brief Defines how detailed generated performance summaries should be
  * @ingroup LunarTypesPerformance
  */
UENUM(BlueprintType)
enum class ELunarPerformanceSummaryDetail : uint8
{
	Low UMETA(DisplayName = "Low"),
	Normal UMETA(DisplayName = "Normal"),
	High UMETA(DisplayName = "High"),
	Full UMETA(DisplayName = "Full")
};

/**
 * @brief Defines memory unit used for performance memory values
 * @ingroup LunarTypesPerformance
 */
UENUM(BlueprintType)
enum class ELunarMemoryUnit : uint8
{
	Bytes UMETA(DisplayName = "Bytes"),
	Kilobytes UMETA(DisplayName = "Kilobytes"),
	Megabytes UMETA(DisplayName = "Megabytes"),
	Gigabytes UMETA(DisplayName = "Gigabytes"),
	Terabytes UMETA(DisplayName = "Terabytes")
};

/**
 * @brief Stores float value history with average minimum maximum and duration
 * @ingroup LunarTypesPerformance
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarPerformanceFloatHistory
{
	GENERATED_BODY()

public:
	/** Recorded values */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|History")
	TArray<float> Values;

	/** Average recorded value */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|History")
	float Average = 0.0f;

	/** Minimum recorded value */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|History")
	float Min = 0.0f;

	/** Maximum recorded value */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|History")
	float Max = 0.0f;

	/** History duration in seconds */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|History")
	float HistoryDurationSeconds = 60.0f;
};

/**
 * @brief Stores current frame performance statistics
 * @ingroup LunarTypesPerformance
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarPerformanceFrameStats
{
	GENERATED_BODY()

	/** Integer frames per second */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|Frame")
	int32 FPS = 0;

	/** Floating point frames per second */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|Frame")
	float FPSFloat = 0.0f;

	/** Frame time in milliseconds */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|Frame")
	float FrameTimeMS = 0.0f;
};

/**
 * @brief Stores physical memory performance statistics
 * @ingroup LunarTypesPerformance
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarPerformanceMemoryStats
{
	GENERATED_BODY()

	/** Total physical memory */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|RAM")
	float TotalPhysicalMemory = 0.0f;

	/** Used physical memory */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|RAM")
	float UsedPhysicalMemory = 0.0f;

	/** Available physical memory */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|RAM")
	float AvailablePhysicalMemory = 0.0f;

	/** Used physical memory percent */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|RAM")
	float PhysicalMemoryUsedPercent = 0.0f;

	/** Current process physical memory */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|RAM")
	float ProcessPhysicalMemory = 0.0f;

	/** Peak process physical memory */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|RAM")
	float PeakProcessPhysicalMemory = 0.0f;
};

/**
 * @brief Stores current process GPU engine usage statistics
 * @ingroup LunarTypesPerformance
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarPerformanceProcessGPUStats
{
	GENERATED_BODY()

	/** True if process GPU statistics are available */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	bool bStatsAvailable = false;

	/** Total process GPU usage percent */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float TotalUsagePercent = 0.0f;

	/** Process GPU graphics 3D usage percent */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float Graphics3DUsagePercent = 0.0f;

	/** Process GPU compute usage percent */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float ComputeUsagePercent = 0.0f;

	/** Process GPU copy usage percent */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float CopyUsagePercent = 0.0f;

	/** Process GPU video decode usage percent */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float VideoDecodeUsagePercent = 0.0f;

	/** Process GPU video encode usage percent */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float VideoEncodeUsagePercent = 0.0f;

	/** Process GPU status message */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	FString Message;
};

/**
 * @brief Stores GPU memory and process GPU performance statistics
 * @ingroup LunarTypesPerformance
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarPerformanceGPUStats
{
	GENERATED_BODY()

	/** True if GPU statistics are available */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	bool bStatsAvailable = false;

	/** Dedicated video memory */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float DedicatedVideoMemory = 0.0f;

	/** Total graphics memory */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float TotalGraphicsMemory = 0.0f;

	/** Total device working memory */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float TotalDeviceWorkingMemory = 0.0f;

	/** RHI texture memory */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float RHITextureMemory = 0.0f;

	/** Streaming texture memory */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float StreamingTextureMemory = 0.0f;

	/** Non streaming texture memory */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float NonStreamingTextureMemory = 0.0f;

	/** Texture pool size */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float TexturePoolSize = 0.0f;

	/** Free texture pool memory */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float TexturePoolFree = 0.0f;

	/** Used texture pool percent */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	float TexturePoolUsedPercent = 0.0f;

	/** Current process GPU statistics */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|GPU")
	FLunarPerformanceProcessGPUStats ProcessGPU;
};

/**
 * @brief Stores CPU performance statistics for system process and logical cores
 * @ingroup LunarTypesPerformance
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarPerformanceCPUStats
{
	GENERATED_BODY()

	/** True if CPU statistics are available */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|CPU")
	bool bStatsAvailable = false;

	/** Legacy alias for system CPU usage percent */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|CPU")
	float CPUUsagePercent = 0.0f;

	/** Whole machine CPU usage percent */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|CPU")
	float SystemCPUUsagePercent = 0.0f;

	/** Current process CPU usage percent */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|CPU")
	float ProcessCPUUsagePercent = 0.0f;

	/** Whole machine logical core usage percents */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|CPU")
	TArray<float> CPUCoreUsagePercents;

	/** CPU status message */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|CPU")
	FString Message;
};

/**
 * @brief Stores disk performance statistics for current process and project drive
 * @ingroup LunarTypesPerformance
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarPerformanceDiskStats
{
	GENERATED_BODY()

	/** True if disk statistics are available */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|Disk")
	bool bStatsAvailable = false;

	/** Current process read speed */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|Disk")
	float ProcessReadSpeed = 0.0f;

	/** Current process write speed */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|Disk")
	float ProcessWriteSpeed = 0.0f;

	/** Project drive active percent */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|Disk")
	float ProjectDriveActivePercent = 0.0f;

	/** Disk status message */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance|Disk")
	FString Message;
};

/**
 * @brief Stores complete performance snapshot for frame memory GPU CPU disk and timestamp
 * @ingroup LunarTypesPerformance
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarPerformanceSnapshot
{
	GENERATED_BODY()

	/** Frame performance statistics */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance")
	FLunarPerformanceFrameStats Frame;

	/** Memory performance statistics */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance")
	FLunarPerformanceMemoryStats Memory;

	/** GPU performance statistics */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance")
	FLunarPerformanceGPUStats GPU;

	/** CPU performance statistics */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance")
	FLunarPerformanceCPUStats CPU;

	/** Disk performance statistics */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance")
	FLunarPerformanceDiskStats Disk;

	/** Snapshot timestamp */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance")
	FDateTime Timestamp;

	/** Memory value unit */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance")
	ELunarMemoryUnit Unit = ELunarMemoryUnit::Bytes;

	/** Application time in seconds */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Performance")
	float AppTimeSeconds = 0.0f;
};

/**
 * @brief Stores general performance monitoring settings
 * @ingroup LunarTypesPerformance
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarPerformanceGeneralSettings
{
	GENERATED_BODY()

	/** Enables performance monitoring when the game instance is initialized */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General", meta = (DisplayName = "Start Monitoring Enabled", ToolTip = "If enabled, the Lunar Performance Subsystem starts monitoring automatically when the Game Instance is initialized."))
	bool bStartMonitoringEnabled = true;

	/** Performance statistics update interval in seconds */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General", meta = (ClampMin = "0.05", UIMin = "0.05", DisplayName = "Update Interval", ToolTip = "How often performance stats are updated, in seconds. Lower values update the monitor more often."))
	float UpdateInterval = 1.0f;
};

/**
 * @brief Stores periodic performance summary output settings
 * @ingroup LunarTypesPerformance
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarPerformanceCollectionSettings
{
	GENERATED_BODY()

	/** Enables opt-in periodic performance summary output */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Collection", meta = (DisplayName = "Output Performance Summary", ToolTip = "If explicitly enabled, the subsystem periodically sends the current performance summary to the Lunar console."))
	bool bCollectPerformanceData = false;

	/** Detail level used for periodic performance summary output */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Collection", meta = (DisplayName = "Output Detail Level", ToolTip = "Controls how much of the collected performance snapshot is sent to the Lunar console. This is independent of the performance widget detail level."))
	ELunarPerformanceSummaryDetail OutputDetailLevel = ELunarPerformanceSummaryDetail::Full;

	/** Performance summary output interval in seconds */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Collection", meta = (ClampMin = "0.1", UIMin = "0.1", DisplayName = "Output Interval", ToolTip = "How often the current performance summary is sent to the Lunar console, in seconds."))
	float CollectPerformanceDataInterval = 10.0f;
};

/**
 * @brief Stores environment rules for performance monitoring
 * @ingroup LunarTypesPerformance
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarPerformanceEnvironmentSettings
{
	GENERATED_BODY()

	/** Allows performance monitoring in editor builds */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Environment", meta = (DisplayName = "Enabled In Editor", ToolTip = "Allows performance monitoring in editor builds."))
	bool bEnabledInEditor = true;

	/** Allows performance monitoring in debug builds */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Environment", meta = (DisplayName = "Enabled In Debug", ToolTip = "Allows performance monitoring in Debug builds."))
	bool bEnabledInDebug = true;

	/** Allows performance monitoring in development builds */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Environment", meta = (DisplayName = "Enabled In Development", ToolTip = "Allows performance monitoring in Development builds."))
	bool bEnabledInDevelopment = true;

	/** Allows performance monitoring in test builds */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Environment", meta = (DisplayName = "Enabled In Test", ToolTip = "Allows performance monitoring in Test builds."))
	bool bEnabledInTest = true;

	/** Allows performance monitoring in shipping builds */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Environment", meta = (DisplayName = "Enabled In Shipping", ToolTip = "Allows performance monitoring in Shipping builds. Usually disabled by default."))
	bool bEnabledInShipping = false;

	/** Allows performance monitoring while running commandlets */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Environment", meta = (DisplayName = "Enabled In Commandlet", ToolTip = "Allows performance monitoring while running commandlets. Usually disabled by default."))
	bool bEnabledInCommandlet = false;

	/** Allows performance monitoring on dedicated servers */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Environment", meta = (DisplayName = "Enabled On Dedicated Server", ToolTip = "Allows performance monitoring on dedicated servers. Usually disabled by default."))
	bool bEnabledOnDedicatedServer = false;
};

/**
 * @brief Stores performance widget display and input settings
 * @ingroup LunarTypesPerformance
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarPerformanceDisplaySettings
{
	GENERATED_BODY()

	/** Enables performance widget display */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Display", meta = (DisplayName = "Display Enabled", ToolTip = "If disabled, the performance widget UI is not created, shown, or controlled by hotkeys. Performance data can still be collected by the subsystem."))
	bool bDisplayEnabled = true;

	/** Widget class used to display performance data */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Display", meta = (DisplayName = "Widget Class", ToolTip = "Widget class used to display Lunar performance data."))
	TSoftClassPtr<UUserWidget> WidgetClass = TSoftClassPtr<UUserWidget>(FSoftClassPath(TEXT("/Lunar/Widgets/Global/W_Performance.W_Performance_C")));

	/** Allows performance widget UI in editor builds */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Display|Environment", meta = (DisplayName = "Enabled In Editor", ToolTip = "Allows the performance widget UI in editor builds."))
	bool bEnabledInEditor = true;

	/** Allows performance widget UI in debug builds */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Display|Environment", meta = (DisplayName = "Enabled In Debug", ToolTip = "Allows the performance widget UI in Debug builds."))
	bool bEnabledInDebug = true;

	/** Allows performance widget UI in development builds */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Display|Environment", meta = (DisplayName = "Enabled In Development", ToolTip = "Allows the performance widget UI in Development builds."))
	bool bEnabledInDevelopment = true;

	/** Allows performance widget UI in test builds */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Display|Environment", meta = (DisplayName = "Enabled In Test", ToolTip = "Allows the performance widget UI in Test builds."))
	bool bEnabledInTest = true;

	/** Allows performance widget UI in shipping builds */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Display|Environment", meta = (DisplayName = "Enabled In Shipping", ToolTip = "Allows the performance widget UI in Shipping builds. Usually disabled by default."))
	bool bEnabledInShipping = false;

	/** Enables performance widget toggle hotkey */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Display|Input", meta = (DisplayName = "Enable Toggle Hotkey", ToolTip = "If enabled, the performance widget can be shown or hidden using the configured hotkey."))
	bool bEnableToggleHotkey = true;

	/** Keyboard key used to show or hide the performance widget */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Display|Input", meta = (DisplayName = "Toggle Widget Hotkey", ToolTip = "Keyboard key used to show or hide the performance widget."))
	FKey ToggleWidgetHotkey = EKeys::F10;
};

/**
 * @brief Stores all performance monitoring settings
 * @ingroup LunarTypesPerformance
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarPerformanceSettings
{
	GENERATED_BODY()

	/** General performance monitoring settings */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (DisplayName = "General", ToolTip = "General performance monitoring settings."))
	FLunarPerformanceGeneralSettings General;

	/** Periodic performance data collection settings */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (DisplayName = "Collection", ToolTip = "Periodic performance data collection settings."))
	FLunarPerformanceCollectionSettings Collection;

	/** Build and runtime environment settings */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (DisplayName = "Environment", ToolTip = "Controls in which build and runtime environments performance monitoring is allowed."))
	FLunarPerformanceEnvironmentSettings Environment;

	/** Performance widget and UI display settings */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (DisplayName = "Display", ToolTip = "Default widget and UI display settings for the performance monitor."))
	FLunarPerformanceDisplaySettings Display;
};
