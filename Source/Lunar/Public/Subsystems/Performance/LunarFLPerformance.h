// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/Performance/LunarTypesPerformance.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "Engine/Font.h"
#include "Fonts/SlateFontInfo.h"
#include "LunarFLPerformance.generated.h"

/**
 * @file LunarFLPerformance.h
 * @brief Performance helper function library
 * @ingroup LunarFLPerformance
 */


struct FTextureMemoryStats;
class UWidget;

/**
 * @brief Blueprint utility functions for performance
 * @ingroup LunarFLPerformance
 */
UCLASS()
class LUNAR_API ULunarFLPerformance : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Frame

	/**
	 * @brief Gets current integer frames per second
	 * @return Current integer frames per second
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Frame")
	static int32 GetFPS();

	/**
	 * @brief Gets current floating point frames per second
	 * @return Current floating point frames per second
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Frame")
	static float GetFPSFloat();

	/**
	 * @brief Gets current frame time in milliseconds
	 * @return Current frame time in milliseconds
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Frame")
	static float GetFrameTimeMS();

	/**
	 * @brief Converts frames per second to frame time in milliseconds
	 * @param FPS Frames per second
	 * @return Frame time in milliseconds
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Frame")
	static float GetFrameTimeFromFPSMS(float FPS);

	/**
	 * @brief Converts frame time in milliseconds to frames per second
	 * @param FrameTimeMS Frame time in milliseconds
	 * @return Frames per second
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Frame")
	static float GetFPSFromFrameTimeMS(float FrameTimeMS);

	/**
	 * @brief Checks whether current frames per second is below target
	 * @param TargetFPS Target frames per second
	 * @return True if current frames per second is below target
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Frame")
	static bool IsFPSBelow(float TargetFPS);

	/**
	 * @brief Checks whether current frame time is above target
	 * @param TargetFrameTimeMS Target frame time in milliseconds
	 * @return True if current frame time is above target
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Frame")
	static bool IsFrameTimeAboveMS(float TargetFrameTimeMS);

	/**
	 * @brief Gets formatted frame performance summary
	 * @return Frame performance summary
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Frame")
	static FString GetFrameSummaryString();

	// RAM

	/**
	 * @brief Gets total physical memory
	 * @param Unit Memory unit
	 * @return Total physical memory
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static float GetTotalPhysicalMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	/**
	 * @brief Gets available physical memory
	 * @param Unit Memory unit
	 * @return Available physical memory
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static float GetAvailablePhysicalMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	/**
	 * @brief Gets used physical memory
	 * @param Unit Memory unit
	 * @return Used physical memory
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static float GetUsedPhysicalMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	/**
	 * @brief Gets total virtual memory
	 * @param Unit Memory unit
	 * @return Total virtual memory
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static float GetTotalVirtualMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	/**
	 * @brief Gets available virtual memory
	 * @param Unit Memory unit
	 * @return Available virtual memory
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static float GetAvailableVirtualMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	/**
	 * @brief Gets used virtual memory
	 * @param Unit Memory unit
	 * @return Used virtual memory
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static float GetUsedVirtualMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	/**
	 * @brief Gets current process physical memory
	 * @param Unit Memory unit
	 * @return Current process physical memory
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static float GetProcessPhysicalMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	/**
	 * @brief Gets current process virtual memory
	 * @param Unit Memory unit
	 * @return Current process virtual memory
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static float GetProcessVirtualMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	/**
	 * @brief Gets peak process physical memory
	 * @param Unit Memory unit
	 * @return Peak process physical memory
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static float GetPeakProcessPhysicalMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	/**
	 * @brief Gets peak process virtual memory
	 * @param Unit Memory unit
	 * @return Peak process virtual memory
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static float GetPeakProcessVirtualMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	/**
	 * @brief Gets used physical memory percent
	 * @return Used physical memory percent
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static float GetPhysicalMemoryUsedPercent();

	/**
	 * @brief Gets available physical memory percent
	 * @return Available physical memory percent
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static float GetPhysicalMemoryAvailablePercent();

	/**
	 * @brief Gets used virtual memory percent
	 * @return Used virtual memory percent
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static float GetVirtualMemoryUsedPercent();

	/**
	 * @brief Gets available virtual memory percent
	 * @return Available virtual memory percent
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static float GetVirtualMemoryAvailablePercent();

	/**
	 * @brief Gets formatted memory summary
	 * @param Detail Summary detail level
	 * @param Unit Memory unit
	 * @return Memory summary
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static FString GetMemorySummaryString(
		ELunarPerformanceSummaryDetail Detail = ELunarPerformanceSummaryDetail::Normal,
		ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes
	);

	// VRAM / GPU memory
	// Uses RHI texture/graphics memory stats
	// This may not exactly match Task Manager / MSI Afterburner

	/**
	 * @brief Checks whether GPU memory statistics are available
	 * @return True if GPU memory statistics are available
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static bool IsGPUMemoryStatsAvailable();

	/**
	 * @brief Gets dedicated video memory
	 * @param Unit Memory unit
	 * @return Dedicated video memory
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static float GetDedicatedVideoMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	/**
	 * @brief Gets dedicated system memory
	 * @param Unit Memory unit
	 * @return Dedicated system memory
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static float GetDedicatedSystemMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	/**
	 * @brief Gets shared system memory
	 * @param Unit Memory unit
	 * @return Shared system memory
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static float GetSharedSystemMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	/**
	 * @brief Gets total graphics memory
	 * @param Unit Memory unit
	 * @return Total graphics memory
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static float GetTotalGraphicsMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	/**
	 * @brief Gets total device working memory
	 * @param Unit Memory unit
	 * @return Total device working memory
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static float GetTotalDeviceWorkingMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	/**
	 * @brief Gets streaming texture memory
	 * @param Unit Memory unit
	 * @return Streaming texture memory
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static float GetStreamingTextureMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	/**
	 * @brief Gets non streaming texture memory
	 * @param Unit Memory unit
	 * @return Non streaming texture memory
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static float GetNonStreamingTextureMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	/**
	 * @brief Gets total texture memory
	 * @param Unit Memory unit
	 * @return Total texture memory
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static float GetTotalTextureMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	/**
	 * @brief Gets texture pool size
	 * @param Unit Memory unit
	 * @return Texture pool size
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static float GetTexturePoolSize(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	/**
	 * @brief Gets available texture pool memory
	 * @param Unit Memory unit
	 * @return Available texture pool memory
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static float GetAvailableTexturePoolMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	/**
	 * @brief Gets largest contiguous graphics allocation
	 * @param Unit Memory unit
	 * @return Largest contiguous graphics allocation
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static float GetLargestContiguousGraphicsAllocation(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	/**
	 * @brief Gets used texture pool percent
	 * @return Used texture pool percent
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static float GetTexturePoolUsedPercent();

	/**
	 * @brief Checks whether limited texture pool size is used
	 * @return True if limited texture pool size is used
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static bool IsUsingLimitedTexturePoolSize();

	/**
	 * @brief Gets formatted GPU memory summary
	 * @param Unit Memory unit
	 * @return GPU memory summary
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static FString GetGPUMemorySummaryString(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	// GPU

	/**
	 * @brief Gets current process GPU usage statistics
	 * @param OutStats Process GPU usage statistics
	 * @return True if statistics were collected
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|GPU")
	static bool GetProcessGPUUsageStats(FLunarPerformanceProcessGPUStats& OutStats);

	/**
	 * @brief Gets formatted current process GPU usage summary
	 * @return Current process GPU usage summary
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|GPU")
	static FString GetProcessGPUUsageSummaryString();

	// CPU usage
	// System CPU is whole machine
	// Process CPU is current process normalized to whole machine percent
	// System CPU cores are whole machine logical core usage values

	/**
	 * @brief Gets legacy CPU usage percent
	 * @param OutCPUUsagePercent CPU usage percent
	 * @param OutMessage Status message
	 * @return True if CPU usage was collected
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|CPU")
	static bool GetCPUUsagePercent(float& OutCPUUsagePercent, FString& OutMessage);

	/**
	 * @brief Gets whole machine CPU usage percent
	 * @param OutSystemCPUUsagePercent System CPU usage percent
	 * @param OutMessage Status message
	 * @return True if system CPU usage was collected
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|CPU")
	static bool GetSystemCPUUsagePercent(float& OutSystemCPUUsagePercent, FString& OutMessage);

	/**
	 * @brief Gets current process CPU usage percent
	 * @param OutProcessCPUUsagePercent Process CPU usage percent
	 * @param OutMessage Status message
	 * @return True if process CPU usage was collected
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|CPU")
	static bool GetProcessCPUUsagePercent(float& OutProcessCPUUsagePercent, FString& OutMessage);

	/**
	 * @brief Gets whole machine logical core usage percents
	 * @param OutCoreUsagePercents Logical core usage percents
	 * @param OutMessage Status message
	 * @return True if core usage values were collected
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|CPU")
	static bool GetCPUCoreUsagePercents(TArray<float>& OutCoreUsagePercents, FString& OutMessage);

	/**
	 * @brief Gets formatted legacy CPU usage summary
	 * @return CPU usage summary
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|CPU")
	static FString GetCPUUsageSummaryString();

	/**
	 * @brief Gets formatted system CPU usage summary
	 * @return System CPU usage summary
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|CPU")
	static FString GetSystemCPUUsageSummaryString();

	/**
	 * @brief Gets formatted process CPU usage summary
	 * @return Process CPU usage summary
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|CPU")
	static FString GetProcessCPUUsageSummaryString();

	/**
	 * @brief Gets formatted logical core usage summary
	 * @param MaxCoresToShow Maximum core count to show
	 * @return Logical core usage summary
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|CPU")
	static FString GetCPUCoreUsageSummaryString(int32 MaxCoresToShow = -1);

	// Disk

	/**
	 * @brief Gets current process disk read speed
	 * @param OutReadSpeed Process read speed
	 * @param Unit Memory unit used for speed value
	 * @param OutMessage Status message
	 * @return True if disk read speed was collected
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|Disk")
	static bool GetProcessDiskReadSpeed(float& OutReadSpeed, ELunarMemoryUnit Unit, FString& OutMessage);

	/**
	 * @brief Gets current process disk write speed
	 * @param OutWriteSpeed Process write speed
	 * @param Unit Memory unit used for speed value
	 * @param OutMessage Status message
	 * @return True if disk write speed was collected
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|Disk")
	static bool GetProcessDiskWriteSpeed(float& OutWriteSpeed, ELunarMemoryUnit Unit, FString& OutMessage);

	/**
	 * @brief Gets current process disk read and write speeds
	 * @param OutReadSpeed Process read speed
	 * @param OutWriteSpeed Process write speed
	 * @param Unit Memory unit used for speed values
	 * @param OutMessage Status message
	 * @return True if disk speeds were collected
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|Disk")
	static bool GetProcessDiskSpeeds(float& OutReadSpeed, float& OutWriteSpeed, ELunarMemoryUnit Unit, FString& OutMessage);

	/**
	 * @brief Gets project drive active percent
	 * @param OutDiskActivePercent Project drive active percent
	 * @param OutMessage Status message
	 * @return True if project drive active percent was collected
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|Disk")
	static bool GetProjectDriveDiskActivePercent(float& OutDiskActivePercent, FString& OutMessage);

	/**
	 * @brief Gets formatted disk performance summary
	 * @param Unit Memory unit used for speed values
	 * @return Disk performance summary
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|Disk")
	static FString GetDiskSummaryString(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	// Hardware / system

	/**
	 * @brief Gets physical CPU core count
	 * @return Physical CPU core count
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Hardware")
	static int32 GetPhysicalCoreCount();

	/**
	 * @brief Gets logical CPU core count
	 * @return Logical CPU core count
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Hardware")
	static int32 GetLogicalCoreCount();

	/**
	 * @brief Gets CPU brand name
	 * @return CPU brand name
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Hardware")
	static FString GetCPUBrand();

	/**
	 * @brief Gets primary GPU brand name
	 * @return Primary GPU brand name
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Hardware")
	static FString GetPrimaryGPUBrand();

	/**
	 * @brief Gets operating system version
	 * @return Operating system version
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Hardware")
	static FString GetOSVersion();

	/**
	 * @brief Gets device make and model
	 * @return Device make and model
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Hardware")
	static FString GetDeviceMakeAndModel();

	/**
	 * @brief Gets formatted hardware summary
	 * @return Hardware summary
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Hardware")
	static FString GetHardwareSummaryString();

	// Memory formatting

	/**
	 * @brief Converts bytes to memory unit
	 * @param Bytes Byte count
	 * @param Unit Target memory unit
	 * @return Converted memory value
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Memory")
	static float ConvertBytesToUnit(int64 Bytes, ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	/**
	 * @brief Gets suffix for memory unit
	 * @param Unit Memory unit
	 * @return Memory unit suffix
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Memory")
	static FString GetMemoryUnitSuffix(ELunarMemoryUnit Unit);

	/**
	 * @brief Formats byte count as memory text
	 * @param Bytes Byte count
	 * @param Unit Memory unit
	 * @return Formatted memory text
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Memory")
	static FString FormatMemory(int64 Bytes, ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	// Snapshot structs

	/**
	 * @brief Gets current frame performance statistics
	 * @return Frame performance statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|Snapshot")
	static FLunarPerformanceFrameStats GetFrameStats();

	/**
	 * @brief Gets current memory performance statistics
	 * @param Unit Memory unit
	 * @return Memory performance statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|Snapshot")
	static FLunarPerformanceMemoryStats GetMemoryStats(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	/**
	 * @brief Gets current GPU performance statistics
	 * @param Unit Memory unit
	 * @return GPU performance statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|Snapshot")
	static FLunarPerformanceGPUStats GetGPUStats(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	/**
	 * @brief Gets current CPU performance statistics
	 * @return CPU performance statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|Snapshot")
	static FLunarPerformanceCPUStats GetCPUStats();

	/**
	 * @brief Gets current disk performance statistics
	 * @param Unit Memory unit used for speed values
	 * @return Disk performance statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|Snapshot")
	static FLunarPerformanceDiskStats GetDiskStats(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	/**
	 * @brief Gets full current performance snapshot
	 * @param Unit Memory unit
	 * @return Performance snapshot
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|Snapshot")
	static FLunarPerformanceSnapshot GetPerformanceSnapshot(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	// Summary

	/**
	 * @brief Gets formatted performance summary
	 * @param Detail Summary detail level
	 * @param Unit Memory unit
	 * @return Performance summary
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|Summary")
	static FString GetPerformanceSummaryString(
		ELunarPerformanceSummaryDetail Detail = ELunarPerformanceSummaryDetail::Normal,
		ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes
	);

	/**
	 * @brief Gets formatted multiline performance summary
	 * @param Detail Summary detail level
	 * @param Unit Memory unit
	 * @return Multiline performance summary
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|Summary")
	static FString GetPerformanceSummaryMultilineString(
		ELunarPerformanceSummaryDetail Detail = ELunarPerformanceSummaryDetail::Full,
		ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes
	);

	/**
	 * @brief Formats an existing performance snapshot as a multiline summary without collecting new statistics
	 * @param Snapshot Performance snapshot to format
	 * @param Detail Summary detail level
	 * @return Multiline performance summary
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Summary")
	static FString FormatPerformanceSnapshotMultilineString(
		const FLunarPerformanceSnapshot& Snapshot,
		ELunarPerformanceSummaryDetail Detail = ELunarPerformanceSummaryDetail::Full
	);

	// Settings

	/**
	 * @brief Gets Lunar performance settings
	 * @return Lunar performance settings
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Settings")
	static FLunarPerformanceSettings GetPerformanceSettings();

	// Paint

	/**
	 * @brief Paints a float history graph into a UMG paint context
	 * @param PaintOwnerWidget Widget that owns paint operation
	 * @param GraphWidget Widget used as graph area
	 * @param Context Paint context
	 * @param History Float history to draw
	 * @param UpdateInterval History update interval
	 * @param SecondsSinceLastSample Seconds since last sample
	 * @param GraphLabel Graph label text
	 * @param ValueSuffix Value suffix text
	 * @param MaxVisibleSamples Maximum visible sample count
	 * @param Padding Graph padding
	 * @param LeftLabelWidth Left label area width
	 * @param YTickCount Vertical tick count
	 * @param bUseZeroBaseline Uses zero as graph baseline
	 * @param bDrawLabel Draws graph label
	 * @param bDrawYTicks Draws vertical ticks
	 * @param bDrawTopBorder Draws top border
	 * @param bDrawBottomBorder Draws bottom border
	 * @param bDrawLeftBorder Draws left border
	 * @param bDrawRightBorder Draws right border
	 * @param bDrawTargetLine Draws target value line
	 * @param TargetValue Target line value
	 * @param GraphTint Graph line tint
	 * @param TargetLineTint Target line tint
	 * @param BorderTint Border tint
	 * @param TickTint Tick tint
	 * @param TextTint Text tint
	 * @param LineThickness Graph line thickness
	 * @param TargetLineThickness Target line thickness
	 * @param BorderThickness Border thickness
	 * @param TickLength Tick length
	 * @param LabelFont Label font
	 * @param LabelFontSize Label font size
	 * @param LabelFontTypeface Label font typeface
	 * @param SplineTangentStrength Spline tangent strength
	 */
	UFUNCTION(
		BlueprintCallable,
		Category = "Lunar|Performance|Paint",
		meta = (AdvancedDisplay = "MaxVisibleSamples,Padding,LeftLabelWidth,YTickCount,bUseZeroBaseline,bDrawLabel,bDrawYTicks,bDrawTopBorder,bDrawBottomBorder,bDrawLeftBorder,bDrawRightBorder,bDrawTargetLine,TargetValue,GraphTint,TargetLineTint,BorderTint,TickTint,TextTint,LineThickness,TargetLineThickness,BorderThickness,TickLength,LabelFont,LabelFontSize,LabelFontTypeface,SplineTangentStrength")
	)
	static void PaintFloatHistoryGraph(
		UWidget* PaintOwnerWidget,
		UWidget* GraphWidget,
		UPARAM(ref) FPaintContext& Context,
		const FLunarPerformanceFloatHistory& History,
		float UpdateInterval,
		float SecondsSinceLastSample,
		FString GraphLabel,
		FString ValueSuffix,
		int32 MaxVisibleSamples = 120,
		float Padding = 8.0f,
		float LeftLabelWidth = 36.0f,
		int32 YTickCount = 4,
		bool bUseZeroBaseline = true,
		bool bDrawLabel = true,
		bool bDrawYTicks = true,
		bool bDrawTopBorder = true,
		bool bDrawBottomBorder = true,
		bool bDrawLeftBorder = true,
		bool bDrawRightBorder = true,
		bool bDrawTargetLine = false,
		float TargetValue = 0.0f,
		FLinearColor GraphTint = FLinearColor(1.0f, 0.476f, 0.319f, 1.0f),
		FLinearColor TargetLineTint = FLinearColor(1.0f, 1.0f, 1.0f, 0.35f),
		FLinearColor BorderTint = FLinearColor(1.0f, 1.0f, 1.0f, 0.25f),
		FLinearColor TickTint = FLinearColor(1.0f, 1.0f, 1.0f, 0.25f),
		FLinearColor TextTint = FLinearColor(1.0f, 1.0f, 1.0f, 0.75f),
		float LineThickness = 1.5f,
		float TargetLineThickness = 1.0f,
		float BorderThickness = 1.0f,
		float TickLength = 5.0f,
		UFont* LabelFont = nullptr,
		int32 LabelFontSize = 10,
		FName LabelFontTypeface = NAME_None,
		float SplineTangentStrength = 0.35f
	);

private:
	/**
	 * @brief Calculates safe percentage value
	 * @param Value Current value
	 * @param MaxValue Maximum value
	 * @return Percentage value
	 */
	static float SafePercent(double Value, double MaxValue);

	/**
	 * @brief Builds undefined metric message
	 * @param MetricName Metric name
	 * @return Undefined metric message
	 */
	static FString GetUndefinedMessage(const FString& MetricName);

	/**
	 * @brief Gets raw GPU texture memory statistics
	 * @param OutStats GPU texture memory statistics
	 * @return True if statistics were collected
	 */
	static bool GetGPUTextureMemoryStatsRaw(FTextureMemoryStats& OutStats);

#if PLATFORM_WINDOWS
	/**
	 * @brief Gets project drive name on Windows
	 * @return Project drive name
	 */
	static FString GetProjectDriveName();

	/**
	 * @brief Converts Windows file time parts to unsigned integer
	 * @param LowDateTime Low date time value
	 * @param HighDateTime High date time value
	 * @return Combined file time value
	 */
	static uint64 FileTimeToUInt64(uint32 LowDateTime, uint32 HighDateTime);

	/**
	 * @brief Samples logical CPU core usage on Windows
	 * @param OutCoreUsagePercents Logical core usage percents
	 * @param OutMessage Status message
	 * @return True if core usage was sampled
	 */
	static bool SampleCPUCoreUsageWindows(TArray<float>& OutCoreUsagePercents, FString& OutMessage);

	/**
	 * @brief Samples current process CPU usage on Windows
	 * @param OutProcessCPUUsagePercent Process CPU usage percent
	 * @param OutMessage Status message
	 * @return True if process CPU usage was sampled
	 */
	static bool SampleProcessCPUUsageWindows(float& OutProcessCPUUsagePercent, FString& OutMessage);

	/**
	 * @brief Samples project drive active percent on Windows
	 * @param OutDiskActivePercent Project drive active percent
	 * @param OutMessage Status message
	 * @return True if project drive active percent was sampled
	 */
	static bool SampleProjectDriveDiskActivePercentWindows(float& OutDiskActivePercent, FString& OutMessage);

	/**
	 * @brief Samples current process disk speed on Windows
	 * @param OutReadSpeed Process read speed
	 * @param OutWriteSpeed Process write speed
	 * @param Unit Memory unit used for speed values
	 * @param OutMessage Status message
	 * @return True if process disk speed was sampled
	 */
	static bool SampleProcessDiskSpeedWindows(float& OutReadSpeed, float& OutWriteSpeed, ELunarMemoryUnit Unit, FString& OutMessage);

	/**
	 * @brief Samples current process GPU usage on Windows
	 * @param OutStats Process GPU usage statistics
	 * @return True if process GPU usage was sampled
	 */
	static bool SampleProcessGPUUsageWindows(FLunarPerformanceProcessGPUStats& OutStats);

	/**
	 * @brief Accumulates process GPU engine usage value
	 * @param InstanceName GPU engine instance name
	 * @param Value GPU engine usage value
	 * @param InOutStats Process GPU usage statistics
	 */
	static void AccumulateProcessGPUEngineValue(const FString& InstanceName, float Value, FLunarPerformanceProcessGPUStats& InOutStats);
#endif
};
