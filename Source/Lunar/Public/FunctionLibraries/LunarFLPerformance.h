// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "LunarTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "Engine/Font.h"
#include "Fonts/SlateFontInfo.h"
#include "LunarFLPerformance.generated.h"

struct FTextureMemoryStats;
class UWidget;

UCLASS()
class LUNAR_API ULunarFLPerformance : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Frame

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Frame")
	static int32 GetFPS();

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Frame")
	static float GetFPSFloat();

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Frame")
	static float GetFrameTimeMS();

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Frame")
	static float GetFrameTimeFromFPSMS(float FPS);

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Frame")
	static float GetFPSFromFrameTimeMS(float FrameTimeMS);

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Frame")
	static bool IsFPSBelow(float TargetFPS);

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Frame")
	static bool IsFrameTimeAboveMS(float TargetFrameTimeMS);

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Frame")
	static FString GetFrameSummaryString();

	// RAM

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static float GetTotalPhysicalMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static float GetAvailablePhysicalMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static float GetUsedPhysicalMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static float GetTotalVirtualMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static float GetAvailableVirtualMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static float GetUsedVirtualMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static float GetProcessPhysicalMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static float GetProcessVirtualMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static float GetPeakProcessPhysicalMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static float GetPeakProcessVirtualMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static float GetPhysicalMemoryUsedPercent();

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static float GetPhysicalMemoryAvailablePercent();

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static float GetVirtualMemoryUsedPercent();

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static float GetVirtualMemoryAvailablePercent();

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|RAM")
	static FString GetMemorySummaryString(
		ELunarPerformanceSummaryDetail Detail = ELunarPerformanceSummaryDetail::Normal,
		ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes
	);

	// VRAM / GPU memory.
	// Uses RHI texture/graphics memory stats. This may not exactly match Task Manager / MSI Afterburner.

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static bool IsGPUMemoryStatsAvailable();

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static float GetDedicatedVideoMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static float GetDedicatedSystemMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static float GetSharedSystemMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static float GetTotalGraphicsMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static float GetTotalDeviceWorkingMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static float GetStreamingTextureMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static float GetNonStreamingTextureMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static float GetTotalTextureMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static float GetTexturePoolSize(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static float GetAvailableTexturePoolMemory(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static float GetLargestContiguousGraphicsAllocation(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static float GetTexturePoolUsedPercent();

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static bool IsUsingLimitedTexturePoolSize();

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|VRAM")
	static FString GetGPUMemorySummaryString(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	//GPU
	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|GPU")
	static bool GetProcessGPUUsageStats(FLunarPerformanceProcessGPUStats& OutStats);

	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|GPU")
	static FString GetProcessGPUUsageSummaryString();

	// CPU usage.
	// System CPU = whole machine.
	// Process CPU = current process, normalized to 0..100 for the whole machine.
	// System CPU cores = whole machine logical core usage.

	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|CPU")
	static bool GetCPUUsagePercent(float& OutCPUUsagePercent, FString& OutMessage);

	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|CPU")
	static bool GetSystemCPUUsagePercent(float& OutSystemCPUUsagePercent, FString& OutMessage);

	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|CPU")
	static bool GetProcessCPUUsagePercent(float& OutProcessCPUUsagePercent, FString& OutMessage);

	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|CPU")
	static bool GetCPUCoreUsagePercents(TArray<float>& OutCoreUsagePercents, FString& OutMessage);

	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|CPU")
	static FString GetCPUUsageSummaryString();

	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|CPU")
	static FString GetSystemCPUUsageSummaryString();

	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|CPU")
	static FString GetProcessCPUUsageSummaryString();

	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|CPU")
	static FString GetCPUCoreUsageSummaryString(int32 MaxCoresToShow = -1);

	// Disk

	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|Disk")
	static bool GetProcessDiskReadSpeed(float& OutReadSpeed, ELunarMemoryUnit Unit, FString& OutMessage);

	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|Disk")
	static bool GetProcessDiskWriteSpeed(float& OutWriteSpeed, ELunarMemoryUnit Unit, FString& OutMessage);

	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|Disk")
	static bool GetProcessDiskSpeeds(float& OutReadSpeed, float& OutWriteSpeed, ELunarMemoryUnit Unit, FString& OutMessage);

	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|Disk")
	static bool GetProjectDriveDiskActivePercent(float& OutDiskActivePercent, FString& OutMessage);

	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|Disk")
	static FString GetDiskSummaryString(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	// Hardware / system

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Hardware")
	static int32 GetPhysicalCoreCount();

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Hardware")
	static int32 GetLogicalCoreCount();

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Hardware")
	static FString GetCPUBrand();

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Hardware")
	static FString GetPrimaryGPUBrand();

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Hardware")
	static FString GetOSVersion();

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Hardware")
	static FString GetDeviceMakeAndModel();

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Hardware")
	static FString GetHardwareSummaryString();

	// Memory formatting

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Memory")
	static float ConvertBytesToUnit(int64 Bytes, ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Memory")
	static FString GetMemoryUnitSuffix(ELunarMemoryUnit Unit);

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Memory")
	static FString FormatMemory(int64 Bytes, ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	// Snapshot structs

	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|Snapshot")
	static FLunarPerformanceFrameStats GetFrameStats();

	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|Snapshot")
	static FLunarPerformanceMemoryStats GetMemoryStats(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|Snapshot")
	static FLunarPerformanceGPUStats GetGPUStats(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|Snapshot")
	static FLunarPerformanceCPUStats GetCPUStats();

	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|Snapshot")
	static FLunarPerformanceDiskStats GetDiskStats(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|Snapshot")
	static FLunarPerformanceSnapshot GetPerformanceSnapshot(ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes);

	// Summary

	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|Summary")
	static FString GetPerformanceSummaryString(
		ELunarPerformanceSummaryDetail Detail = ELunarPerformanceSummaryDetail::Normal,
		ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes
	);

	UFUNCTION(BlueprintCallable, Category = "Lunar|Performance|Summary")
	static FString GetPerformanceSummaryMultilineString(
		ELunarPerformanceSummaryDetail Detail = ELunarPerformanceSummaryDetail::Full,
		ELunarMemoryUnit Unit = ELunarMemoryUnit::Megabytes
	);

	// Settings

	UFUNCTION(BlueprintPure, Category = "Lunar|Performance|Settings")
	static FLunarPerformanceSettings GetPerformanceSettings();

	// Paint

	UFUNCTION(
		BlueprintCallable,
		Category = "Lunar|Performance|Paint",
		meta = (
			AdvancedDisplay = "MaxVisibleSamples,Padding,LeftLabelWidth,YTickCount,bDrawTopBorder,bDrawBottomBorder,bDrawLeftBorder,bDrawRightBorder,bDrawTargetLine,TargetFPS,GraphTint,TargetLineTint,BorderTint,TickTint,TextTint,LineThickness,TargetLineThickness,BorderThickness,TickLength,LabelFont,LabelFontSize,LabelFontTypeface,SplineTangentStrength"
			)
	)
	static void PaintFPSGraph(
		UWidget* WidgetSelf,
		UPARAM(ref) FPaintContext& Context,
		const TArray<float>& FPSValues,
		float UpdateInterval,
		float SecondsSinceLastSample,
		int32 MaxVisibleSamples = 120,
		float Padding = 8.0f,
		float LeftLabelWidth = 36.0f,
		int32 YTickCount = 4,
		bool bDrawTopBorder = true,
		bool bDrawBottomBorder = true,
		bool bDrawLeftBorder = true,
		bool bDrawRightBorder = true,
		bool bDrawTargetLine = true,
		float TargetFPS = 60.0f,
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
	static float SafePercent(double Value, double MaxValue);
	static FString GetUndefinedMessage(const FString& MetricName);

	static bool GetGPUTextureMemoryStatsRaw(FTextureMemoryStats& OutStats);

#if PLATFORM_WINDOWS
	static FString GetProjectDriveName();
	static uint64 FileTimeToUInt64(uint32 LowDateTime, uint32 HighDateTime);
	static bool SampleCPUCoreUsageWindows(TArray<float>& OutCoreUsagePercents, FString& OutMessage);
	static bool SampleProcessCPUUsageWindows(float& OutProcessCPUUsagePercent, FString& OutMessage);
	static bool SampleProjectDriveDiskActivePercentWindows(float& OutDiskActivePercent, FString& OutMessage);
	static bool SampleProcessDiskSpeedWindows(float& OutReadSpeed, float& OutWriteSpeed, ELunarMemoryUnit Unit, FString& OutMessage);

	static bool SampleProcessGPUUsageWindows(FLunarPerformanceProcessGPUStats& OutStats);
	static void AccumulateProcessGPUEngineValue(const FString& InstanceName, float Value, FLunarPerformanceProcessGPUStats& InOutStats);
#endif
};