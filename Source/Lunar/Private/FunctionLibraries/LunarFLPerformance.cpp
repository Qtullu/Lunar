// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "FunctionLibraries/LunarFLPerformance.h"

#include "LunarFL.h"

#include "DynamicRHI.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProcess.h"
#include "Misc/App.h"
#include "Misc/Paths.h"
#include "RHIStats.h"
#include "Settings/LunarSettings.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Widget.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/Font.h"
#include "Rendering/DrawElements.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <Windows.h>
#include <Pdh.h>
#include <PdhMsg.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

float ULunarFLPerformance::SafePercent(double Value, double MaxValue)
{
	if (MaxValue <= 0.0)
	{
		return 0.0f;
	}

	return static_cast<float>((Value / MaxValue) * 100.0);
}

FString ULunarFLPerformance::GetUndefinedMessage(const FString& MetricName)
{
	return FString::Printf(TEXT("%s: Undefined on this platform"), *MetricName);
}

// Frame

int32 ULunarFLPerformance::GetFPS()
{
	return FMath::RoundToInt(GetFPSFloat());
}

float ULunarFLPerformance::GetFPSFloat()
{
	const double DeltaTime = FApp::GetDeltaTime();

	if (DeltaTime <= 0.0)
	{
		return 0.0f;
	}

	return static_cast<float>(1.0 / DeltaTime);
}

float ULunarFLPerformance::GetFrameTimeMS()
{
	return static_cast<float>(FApp::GetDeltaTime() * 1000.0);
}

float ULunarFLPerformance::GetFrameTimeFromFPSMS(float FPS)
{
	if (FPS <= 0.0f)
	{
		return 0.0f;
	}

	return 1000.0f / FPS;
}

float ULunarFLPerformance::GetFPSFromFrameTimeMS(float FrameTimeMS)
{
	if (FrameTimeMS <= 0.0f)
	{
		return 0.0f;
	}

	return 1000.0f / FrameTimeMS;
}

bool ULunarFLPerformance::IsFPSBelow(float TargetFPS)
{
	return GetFPSFloat() < TargetFPS;
}

bool ULunarFLPerformance::IsFrameTimeAboveMS(float TargetFrameTimeMS)
{
	return GetFrameTimeMS() > TargetFrameTimeMS;
}

FString ULunarFLPerformance::GetFrameSummaryString()
{
	return FString::Printf(TEXT("FPS: %d | Frame: %.2f ms"), GetFPS(), GetFrameTimeMS());
}

// Memory formatting

float ULunarFLPerformance::ConvertBytesToUnit(int64 Bytes, ELunarMemoryUnit Unit)
{
	const double Value = static_cast<double>(Bytes);

	switch (Unit)
	{
	case ELunarMemoryUnit::Bytes:
		return static_cast<float>(Value);

	case ELunarMemoryUnit::Kilobytes:
		return static_cast<float>(Value / 1024.0);

	case ELunarMemoryUnit::Megabytes:
		return static_cast<float>(Value / 1024.0 / 1024.0);

	case ELunarMemoryUnit::Gigabytes:
		return static_cast<float>(Value / 1024.0 / 1024.0 / 1024.0);

	case ELunarMemoryUnit::Terabytes:
		return static_cast<float>(Value / 1024.0 / 1024.0 / 1024.0 / 1024.0);

	default:
		return static_cast<float>(Value / 1024.0 / 1024.0);
	}
}

FString ULunarFLPerformance::GetMemoryUnitSuffix(ELunarMemoryUnit Unit)
{
	switch (Unit)
	{
	case ELunarMemoryUnit::Bytes:
		return TEXT("B");

	case ELunarMemoryUnit::Kilobytes:
		return TEXT("KB");

	case ELunarMemoryUnit::Megabytes:
		return TEXT("MB");

	case ELunarMemoryUnit::Gigabytes:
		return TEXT("GB");

	case ELunarMemoryUnit::Terabytes:
		return TEXT("TB");

	default:
		return TEXT("MB");
	}
}

FString ULunarFLPerformance::FormatMemory(int64 Bytes, ELunarMemoryUnit Unit)
{
	const FString Suffix = GetMemoryUnitSuffix(Unit);

	if (Unit == ELunarMemoryUnit::Bytes)
	{
		return FString::Printf(TEXT("%lld %s"), Bytes, *Suffix);
	}

	return FString::Printf(TEXT("%.2f %s"), ConvertBytesToUnit(Bytes, Unit), *Suffix);
}

// RAM

float ULunarFLPerformance::GetTotalPhysicalMemory(ELunarMemoryUnit Unit)
{
	const FPlatformMemoryStats Stats = FPlatformMemory::GetStats();
	return ConvertBytesToUnit(static_cast<int64>(Stats.TotalPhysical), Unit);
}

float ULunarFLPerformance::GetAvailablePhysicalMemory(ELunarMemoryUnit Unit)
{
	const FPlatformMemoryStats Stats = FPlatformMemory::GetStats();
	return ConvertBytesToUnit(static_cast<int64>(Stats.AvailablePhysical), Unit);
}

float ULunarFLPerformance::GetUsedPhysicalMemory(ELunarMemoryUnit Unit)
{
	const FPlatformMemoryStats Stats = FPlatformMemory::GetStats();

	const int64 TotalPhysical = static_cast<int64>(Stats.TotalPhysical);
	const int64 AvailablePhysical = static_cast<int64>(Stats.AvailablePhysical);
	const int64 UsedPhysical = FMath::Max<int64>(0, TotalPhysical - AvailablePhysical);

	return ConvertBytesToUnit(UsedPhysical, Unit);
}

float ULunarFLPerformance::GetTotalVirtualMemory(ELunarMemoryUnit Unit)
{
	const FPlatformMemoryStats Stats = FPlatformMemory::GetStats();
	return ConvertBytesToUnit(static_cast<int64>(Stats.TotalVirtual), Unit);
}

float ULunarFLPerformance::GetAvailableVirtualMemory(ELunarMemoryUnit Unit)
{
	const FPlatformMemoryStats Stats = FPlatformMemory::GetStats();
	return ConvertBytesToUnit(static_cast<int64>(Stats.AvailableVirtual), Unit);
}

float ULunarFLPerformance::GetUsedVirtualMemory(ELunarMemoryUnit Unit)
{
	const FPlatformMemoryStats Stats = FPlatformMemory::GetStats();

	const int64 TotalVirtual = static_cast<int64>(Stats.TotalVirtual);
	const int64 AvailableVirtual = static_cast<int64>(Stats.AvailableVirtual);
	const int64 UsedVirtual = FMath::Max<int64>(0, TotalVirtual - AvailableVirtual);

	return ConvertBytesToUnit(UsedVirtual, Unit);
}

float ULunarFLPerformance::GetProcessPhysicalMemory(ELunarMemoryUnit Unit)
{
	const FPlatformMemoryStats Stats = FPlatformMemory::GetStats();
	return ConvertBytesToUnit(static_cast<int64>(Stats.UsedPhysical), Unit);
}

float ULunarFLPerformance::GetProcessVirtualMemory(ELunarMemoryUnit Unit)
{
	const FPlatformMemoryStats Stats = FPlatformMemory::GetStats();
	return ConvertBytesToUnit(static_cast<int64>(Stats.UsedVirtual), Unit);
}

float ULunarFLPerformance::GetPeakProcessPhysicalMemory(ELunarMemoryUnit Unit)
{
	const FPlatformMemoryStats Stats = FPlatformMemory::GetStats();
	return ConvertBytesToUnit(static_cast<int64>(Stats.PeakUsedPhysical), Unit);
}

float ULunarFLPerformance::GetPeakProcessVirtualMemory(ELunarMemoryUnit Unit)
{
	const FPlatformMemoryStats Stats = FPlatformMemory::GetStats();
	return ConvertBytesToUnit(static_cast<int64>(Stats.PeakUsedVirtual), Unit);
}

float ULunarFLPerformance::GetPhysicalMemoryUsedPercent()
{
	const FPlatformMemoryStats Stats = FPlatformMemory::GetStats();

	const double TotalPhysical = static_cast<double>(Stats.TotalPhysical);
	const double AvailablePhysical = static_cast<double>(Stats.AvailablePhysical);
	const double UsedPhysical = FMath::Max(0.0, TotalPhysical - AvailablePhysical);

	return SafePercent(UsedPhysical, TotalPhysical);
}

float ULunarFLPerformance::GetPhysicalMemoryAvailablePercent()
{
	const FPlatformMemoryStats Stats = FPlatformMemory::GetStats();

	return SafePercent(
		static_cast<double>(Stats.AvailablePhysical),
		static_cast<double>(Stats.TotalPhysical)
	);
}

float ULunarFLPerformance::GetVirtualMemoryUsedPercent()
{
	const FPlatformMemoryStats Stats = FPlatformMemory::GetStats();

	const double TotalVirtual = static_cast<double>(Stats.TotalVirtual);
	const double AvailableVirtual = static_cast<double>(Stats.AvailableVirtual);
	const double UsedVirtual = FMath::Max(0.0, TotalVirtual - AvailableVirtual);

	return SafePercent(UsedVirtual, TotalVirtual);
}

float ULunarFLPerformance::GetVirtualMemoryAvailablePercent()
{
	const FPlatformMemoryStats Stats = FPlatformMemory::GetStats();

	return SafePercent(
		static_cast<double>(Stats.AvailableVirtual),
		static_cast<double>(Stats.TotalVirtual)
	);
}

FString ULunarFLPerformance::GetMemorySummaryString(ELunarPerformanceSummaryDetail Detail, ELunarMemoryUnit Unit)
{
	const FPlatformMemoryStats Stats = FPlatformMemory::GetStats();

	const int64 TotalPhysical = static_cast<int64>(Stats.TotalPhysical);
	const int64 AvailablePhysical = static_cast<int64>(Stats.AvailablePhysical);
	const int64 UsedPhysical = FMath::Max<int64>(0, TotalPhysical - AvailablePhysical);

	const int64 TotalVirtual = static_cast<int64>(Stats.TotalVirtual);
	const int64 AvailableVirtual = static_cast<int64>(Stats.AvailableVirtual);
	const int64 UsedVirtual = FMath::Max<int64>(0, TotalVirtual - AvailableVirtual);

	switch (Detail)
	{
	case ELunarPerformanceSummaryDetail::Low:
		return FString::Printf(TEXT("RAM: %s"), *FormatMemory(UsedPhysical, Unit));

	case ELunarPerformanceSummaryDetail::Normal:
		return FString::Printf(
			TEXT("RAM: %s / %s (%s) | Process RAM: %s"),
			*FormatMemory(UsedPhysical, Unit),
			*FormatMemory(TotalPhysical, Unit),
			*LunarFL::String::FormatPercent(GetPhysicalMemoryUsedPercent(), 1),
			*FormatMemory(static_cast<int64>(Stats.UsedPhysical), Unit)
		);

	case ELunarPerformanceSummaryDetail::High:
		return FString::Printf(
			TEXT("RAM: %s / %s (%s) | Process RAM: %s | Peak RAM: %s"),
			*FormatMemory(UsedPhysical, Unit),
			*FormatMemory(TotalPhysical, Unit),
			*LunarFL::String::FormatPercent(GetPhysicalMemoryUsedPercent(), 1),
			*FormatMemory(static_cast<int64>(Stats.UsedPhysical), Unit),
			*FormatMemory(static_cast<int64>(Stats.PeakUsedPhysical), Unit)
		);

	case ELunarPerformanceSummaryDetail::Full:
		return FString::Printf(
			TEXT("RAM: %s / %s (%s) | Virtual: %s / %s (%s) | Process RAM: %s | Process Virtual: %s | Peak RAM: %s | Peak Virtual: %s"),
			*FormatMemory(UsedPhysical, Unit),
			*FormatMemory(TotalPhysical, Unit),
			*LunarFL::String::FormatPercent(GetPhysicalMemoryUsedPercent(), 1),
			*FormatMemory(UsedVirtual, Unit),
			*FormatMemory(TotalVirtual, Unit),
			*LunarFL::String::FormatPercent(GetVirtualMemoryUsedPercent(), 1),
			*FormatMemory(static_cast<int64>(Stats.UsedPhysical), Unit),
			*FormatMemory(static_cast<int64>(Stats.UsedVirtual), Unit),
			*FormatMemory(static_cast<int64>(Stats.PeakUsedPhysical), Unit),
			*FormatMemory(static_cast<int64>(Stats.PeakUsedVirtual), Unit)
		);

	default:
		return TEXT("RAM: Undefined");
	}
}

// VRAM / GPU memory

bool ULunarFLPerformance::GetGPUTextureMemoryStatsRaw(FTextureMemoryStats& OutStats)
{
	if (!GDynamicRHI)
	{
		return false;
	}

	GDynamicRHI->RHIGetTextureMemoryStats(OutStats);

	return
		OutStats.DedicatedVideoMemory >= 0 ||
		OutStats.TotalGraphicsMemory >= 0 ||
		OutStats.StreamingMemorySize > 0 ||
		OutStats.NonStreamingMemorySize > 0 ||
		OutStats.TexturePoolSize > 0;
}

bool ULunarFLPerformance::IsGPUMemoryStatsAvailable()
{
	FTextureMemoryStats Stats;
	return GetGPUTextureMemoryStatsRaw(Stats);
}

float ULunarFLPerformance::GetDedicatedVideoMemory(ELunarMemoryUnit Unit)
{
	FTextureMemoryStats Stats;
	if (!GetGPUTextureMemoryStatsRaw(Stats) || Stats.DedicatedVideoMemory < 0)
	{
		return 0.0f;
	}

	return ConvertBytesToUnit(Stats.DedicatedVideoMemory, Unit);
}

float ULunarFLPerformance::GetDedicatedSystemMemory(ELunarMemoryUnit Unit)
{
	FTextureMemoryStats Stats;
	if (!GetGPUTextureMemoryStatsRaw(Stats) || Stats.DedicatedSystemMemory < 0)
	{
		return 0.0f;
	}

	return ConvertBytesToUnit(Stats.DedicatedSystemMemory, Unit);
}

float ULunarFLPerformance::GetSharedSystemMemory(ELunarMemoryUnit Unit)
{
	FTextureMemoryStats Stats;
	if (!GetGPUTextureMemoryStatsRaw(Stats) || Stats.SharedSystemMemory < 0)
	{
		return 0.0f;
	}

	return ConvertBytesToUnit(Stats.SharedSystemMemory, Unit);
}

float ULunarFLPerformance::GetTotalGraphicsMemory(ELunarMemoryUnit Unit)
{
	FTextureMemoryStats Stats;
	if (!GetGPUTextureMemoryStatsRaw(Stats) || Stats.TotalGraphicsMemory < 0)
	{
		return 0.0f;
	}

	return ConvertBytesToUnit(Stats.TotalGraphicsMemory, Unit);
}

float ULunarFLPerformance::GetTotalDeviceWorkingMemory(ELunarMemoryUnit Unit)
{
	FTextureMemoryStats Stats;
	if (!GetGPUTextureMemoryStatsRaw(Stats))
	{
		return 0.0f;
	}

	const int64 TotalDeviceWorkingMemory = Stats.GetTotalDeviceWorkingMemory();

	if (TotalDeviceWorkingMemory < 0)
	{
		return 0.0f;
	}

	return ConvertBytesToUnit(TotalDeviceWorkingMemory, Unit);
}

float ULunarFLPerformance::GetStreamingTextureMemory(ELunarMemoryUnit Unit)
{
	FTextureMemoryStats Stats;
	if (!GetGPUTextureMemoryStatsRaw(Stats))
	{
		return 0.0f;
	}

	return ConvertBytesToUnit(static_cast<int64>(Stats.StreamingMemorySize), Unit);
}

float ULunarFLPerformance::GetNonStreamingTextureMemory(ELunarMemoryUnit Unit)
{
	FTextureMemoryStats Stats;
	if (!GetGPUTextureMemoryStatsRaw(Stats))
	{
		return 0.0f;
	}

	return ConvertBytesToUnit(static_cast<int64>(Stats.NonStreamingMemorySize), Unit);
}

float ULunarFLPerformance::GetTotalTextureMemory(ELunarMemoryUnit Unit)
{
	FTextureMemoryStats Stats;
	if (!GetGPUTextureMemoryStatsRaw(Stats))
	{
		return 0.0f;
	}

	const uint64 TotalTextureMemory = Stats.StreamingMemorySize + Stats.NonStreamingMemorySize;
	return ConvertBytesToUnit(static_cast<int64>(TotalTextureMemory), Unit);
}

float ULunarFLPerformance::GetTexturePoolSize(ELunarMemoryUnit Unit)
{
	FTextureMemoryStats Stats;
	if (!GetGPUTextureMemoryStatsRaw(Stats) || Stats.TexturePoolSize <= 0)
	{
		return 0.0f;
	}

	return ConvertBytesToUnit(Stats.TexturePoolSize, Unit);
}

float ULunarFLPerformance::GetAvailableTexturePoolMemory(ELunarMemoryUnit Unit)
{
	FTextureMemoryStats Stats;
	if (!GetGPUTextureMemoryStatsRaw(Stats))
	{
		return 0.0f;
	}

	return ConvertBytesToUnit(Stats.ComputeAvailableMemorySize(), Unit);
}

float ULunarFLPerformance::GetLargestContiguousGraphicsAllocation(ELunarMemoryUnit Unit)
{
	FTextureMemoryStats Stats;
	if (!GetGPUTextureMemoryStatsRaw(Stats) || Stats.LargestContiguousAllocation <= 0)
	{
		return 0.0f;
	}

	return ConvertBytesToUnit(Stats.LargestContiguousAllocation, Unit);
}

float ULunarFLPerformance::GetTexturePoolUsedPercent()
{
	FTextureMemoryStats Stats;
	if (!GetGPUTextureMemoryStatsRaw(Stats) || Stats.TexturePoolSize <= 0)
	{
		return 0.0f;
	}

	return SafePercent(
		static_cast<double>(Stats.StreamingMemorySize),
		static_cast<double>(Stats.TexturePoolSize)
	);
}

bool ULunarFLPerformance::IsUsingLimitedTexturePoolSize()
{
	FTextureMemoryStats Stats;
	if (!GetGPUTextureMemoryStatsRaw(Stats))
	{
		return false;
	}

	return Stats.IsUsingLimitedPoolSize();
}

FString ULunarFLPerformance::GetGPUMemorySummaryString(ELunarMemoryUnit Unit)
{
	FTextureMemoryStats Stats;
	if (!GetGPUTextureMemoryStatsRaw(Stats))
	{
		return TEXT("VRAM: Undefined");
	}

	const int64 TotalDeviceWorkingMemory = Stats.GetTotalDeviceWorkingMemory();
	const uint64 TotalTextureMemory = Stats.StreamingMemorySize + Stats.NonStreamingMemorySize;

	const FString DedicatedVideoMemoryString = Stats.DedicatedVideoMemory >= 0
		? FormatMemory(Stats.DedicatedVideoMemory, Unit)
		: TEXT("Undefined");

	const FString TotalGraphicsMemoryString = Stats.TotalGraphicsMemory >= 0
		? FormatMemory(Stats.TotalGraphicsMemory, Unit)
		: TEXT("Undefined");

	const FString DeviceWorkingMemoryString = TotalDeviceWorkingMemory >= 0
		? FormatMemory(TotalDeviceWorkingMemory, Unit)
		: TEXT("Undefined");

	const FString TexturePoolString = Stats.TexturePoolSize > 0
		? FormatMemory(Stats.TexturePoolSize, Unit)
		: TEXT("Unlimited");

	return FString::Printf(
		TEXT("VRAM: Dedicated: %s | Graphics Total: %s | Working: %s | RHI Textures: %s | Streaming Textures: %s | NonStreaming RHI: %s | Streaming Pool: %s | Pool Free: %s | Pool Usage: %s"),
		*DedicatedVideoMemoryString,
		*TotalGraphicsMemoryString,
		*DeviceWorkingMemoryString,
		*FormatMemory(static_cast<int64>(TotalTextureMemory), Unit),
		*FormatMemory(static_cast<int64>(Stats.StreamingMemorySize), Unit),
		*FormatMemory(static_cast<int64>(Stats.NonStreamingMemorySize), Unit),
		*TexturePoolString,
		*FormatMemory(Stats.ComputeAvailableMemorySize(), Unit),
		*LunarFL::String::FormatPercent(GetTexturePoolUsedPercent(), 1)
	);
}

// Windows CPU / Disk / GPU helpers

#if PLATFORM_WINDOWS

FString ULunarFLPerformance::GetProjectDriveName()
{
	const FString FullPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());

	if (FullPath.Len() >= 2 && FullPath[1] == TEXT(':'))
	{
		return FullPath.Left(2);
	}

	return TEXT("C:");
}

uint64 ULunarFLPerformance::FileTimeToUInt64(uint32 LowDateTime, uint32 HighDateTime)
{
	ULARGE_INTEGER Result;
	Result.LowPart = LowDateTime;
	Result.HighPart = HighDateTime;
	return Result.QuadPart;
}

bool ULunarFLPerformance::SampleCPUCoreUsageWindows(TArray<float>& OutCoreUsagePercents, FString& OutMessage)
{
	OutCoreUsagePercents.Reset();
	OutMessage = TEXT("Undefined");

	static PDH_HQUERY Query = nullptr;
	static PDH_HCOUNTER Counter = nullptr;
	static bool bInitialized = false;

	if (!bInitialized)
	{
		if (PdhOpenQueryW(nullptr, 0, &Query) != ERROR_SUCCESS)
		{
			OutMessage = TEXT("CPU usage: PDH query creation failed");
			return false;
		}

		if (PdhAddEnglishCounterW(Query, L"\\Processor(*)\\% Processor Time", 0, &Counter) != ERROR_SUCCESS)
		{
			PdhCloseQuery(Query);
			Query = nullptr;
			OutMessage = TEXT("CPU usage: PDH counter creation failed");
			return false;
		}

		PdhCollectQueryData(Query);
		bInitialized = true;

		OutMessage = TEXT("CPU usage: waiting for second sample");
		return false;
	}

	PdhCollectQueryData(Query);

	DWORD BufferSize = 0;
	DWORD ItemCount = 0;

	PDH_STATUS Status = PdhGetFormattedCounterArrayW(Counter, PDH_FMT_DOUBLE, &BufferSize, &ItemCount, nullptr);
	if (Status != PDH_MORE_DATA)
	{
		OutMessage = TEXT("CPU usage: PDH data unavailable");
		return false;
	}

	TArray<uint8> Buffer;
	Buffer.SetNumZeroed(static_cast<int32>(BufferSize));

	PPDH_FMT_COUNTERVALUE_ITEM_W Items = reinterpret_cast<PPDH_FMT_COUNTERVALUE_ITEM_W>(Buffer.GetData());

	Status = PdhGetFormattedCounterArrayW(Counter, PDH_FMT_DOUBLE, &BufferSize, &ItemCount, Items);
	if (Status != ERROR_SUCCESS)
	{
		OutMessage = TEXT("CPU usage: PDH formatted data failed");
		return false;
	}

	for (DWORD Index = 0; Index < ItemCount; ++Index)
	{
		const FString InstanceName = Items[Index].szName ? FString(Items[Index].szName) : FString();

		if (InstanceName == TEXT("_Total"))
		{
			continue;
		}

		if (Items[Index].FmtValue.CStatus != ERROR_SUCCESS)
		{
			continue;
		}

		const float CoreUsage = FMath::Clamp(static_cast<float>(Items[Index].FmtValue.doubleValue), 0.0f, 100.0f);
		OutCoreUsagePercents.Add(CoreUsage);
	}

	if (OutCoreUsagePercents.Num() <= 0)
	{
		OutMessage = TEXT("CPU usage: no core data");
		return false;
	}

	OutMessage = TEXT("OK");
	return true;
}

bool ULunarFLPerformance::SampleProcessCPUUsageWindows(float& OutProcessCPUUsagePercent, FString& OutMessage)
{
	OutProcessCPUUsagePercent = 0.0f;
	OutMessage = TEXT("Undefined");

	FILETIME CreationTime;
	FILETIME ExitTime;
	FILETIME KernelTime;
	FILETIME UserTime;

	if (!GetProcessTimes(GetCurrentProcess(), &CreationTime, &ExitTime, &KernelTime, &UserTime))
	{
		OutMessage = TEXT("Process CPU usage: GetProcessTimes failed");
		return false;
	}

	static bool bHasPreviousSample = false;
	static uint64 PreviousProcessTime = 0;
	static double PreviousWallTimeSeconds = 0.0;

	const uint64 CurrentKernelTime = FileTimeToUInt64(KernelTime.dwLowDateTime, KernelTime.dwHighDateTime);
	const uint64 CurrentUserTime = FileTimeToUInt64(UserTime.dwLowDateTime, UserTime.dwHighDateTime);
	const uint64 CurrentProcessTime = CurrentKernelTime + CurrentUserTime;
	const double CurrentWallTimeSeconds = FPlatformTime::Seconds();

	if (!bHasPreviousSample)
	{
		bHasPreviousSample = true;
		PreviousProcessTime = CurrentProcessTime;
		PreviousWallTimeSeconds = CurrentWallTimeSeconds;

		OutMessage = TEXT("Process CPU usage: waiting for second sample");
		return false;
	}

	const double DeltaWallTimeSeconds = CurrentWallTimeSeconds - PreviousWallTimeSeconds;
	const uint64 DeltaProcessTime100ns = CurrentProcessTime >= PreviousProcessTime
		? CurrentProcessTime - PreviousProcessTime
		: 0;

	PreviousProcessTime = CurrentProcessTime;
	PreviousWallTimeSeconds = CurrentWallTimeSeconds;

	if (DeltaWallTimeSeconds <= 0.0)
	{
		OutMessage = TEXT("Process CPU usage: invalid delta time");
		return false;
	}

	const int32 LogicalCoreCount = FMath::Max(1, FPlatformMisc::NumberOfCoresIncludingHyperthreads());
	const double DeltaProcessCPUSeconds = static_cast<double>(DeltaProcessTime100ns) * 0.0000001;
	const double UsagePercent = (DeltaProcessCPUSeconds / (DeltaWallTimeSeconds * static_cast<double>(LogicalCoreCount))) * 100.0;

	OutProcessCPUUsagePercent = FMath::Clamp(static_cast<float>(UsagePercent), 0.0f, 100.0f);
	OutMessage = TEXT("OK");

	return true;
}

bool ULunarFLPerformance::SampleProjectDriveDiskActivePercentWindows(float& OutDiskActivePercent, FString& OutMessage)
{
	OutDiskActivePercent = 0.0f;
	OutMessage = TEXT("Undefined");

	static PDH_HQUERY Query = nullptr;
	static PDH_HCOUNTER Counter = nullptr;
	static bool bInitialized = false;
	static FString InitializedDriveName;

	const FString DriveName = GetProjectDriveName();

	if (!bInitialized || InitializedDriveName != DriveName)
	{
		if (Query)
		{
			PdhCloseQuery(Query);
			Query = nullptr;
			Counter = nullptr;
		}

		if (PdhOpenQueryW(nullptr, 0, &Query) != ERROR_SUCCESS)
		{
			OutMessage = TEXT("Disk active: PDH query creation failed");
			return false;
		}

		const FString CounterPath = FString::Printf(TEXT("\\LogicalDisk(%s)\\%% Disk Time"), *DriveName);

		if (PdhAddEnglishCounterW(Query, *CounterPath, 0, &Counter) != ERROR_SUCCESS)
		{
			PdhCloseQuery(Query);
			Query = nullptr;
			OutMessage = FString::Printf(TEXT("Disk active: PDH counter failed for %s"), *DriveName);
			return false;
		}

		PdhCollectQueryData(Query);
		bInitialized = true;
		InitializedDriveName = DriveName;

		OutMessage = TEXT("Disk active: waiting for second sample");
		return false;
	}

	PdhCollectQueryData(Query);

	PDH_FMT_COUNTERVALUE Value;
	if (PdhGetFormattedCounterValue(Counter, PDH_FMT_DOUBLE, nullptr, &Value) != ERROR_SUCCESS || Value.CStatus != ERROR_SUCCESS)
	{
		OutMessage = TEXT("Disk active: PDH data unavailable");
		return false;
	}

	OutDiskActivePercent = FMath::Clamp(static_cast<float>(Value.doubleValue), 0.0f, 100.0f);
	OutMessage = TEXT("OK");

	return true;
}

bool ULunarFLPerformance::SampleProcessDiskSpeedWindows(float& OutReadSpeed, float& OutWriteSpeed, ELunarMemoryUnit Unit, FString& OutMessage)
{
	OutReadSpeed = 0.0f;
	OutWriteSpeed = 0.0f;
	OutMessage = TEXT("Undefined");

	IO_COUNTERS IOCounters;
	if (!GetProcessIoCounters(GetCurrentProcess(), &IOCounters))
	{
		OutMessage = TEXT("Process disk speed: GetProcessIoCounters failed");
		return false;
	}

	static bool bHasPreviousSample = false;
	static uint64 PreviousReadBytes = 0;
	static uint64 PreviousWriteBytes = 0;
	static double PreviousTimeSeconds = 0.0;

	const uint64 CurrentReadBytes = static_cast<uint64>(IOCounters.ReadTransferCount);
	const uint64 CurrentWriteBytes = static_cast<uint64>(IOCounters.WriteTransferCount);
	const double CurrentTimeSeconds = FPlatformTime::Seconds();

	if (!bHasPreviousSample)
	{
		bHasPreviousSample = true;
		PreviousReadBytes = CurrentReadBytes;
		PreviousWriteBytes = CurrentWriteBytes;
		PreviousTimeSeconds = CurrentTimeSeconds;

		OutMessage = TEXT("Process disk speed: waiting for second sample");
		return false;
	}

	const double DeltaTime = CurrentTimeSeconds - PreviousTimeSeconds;
	if (DeltaTime <= 0.0)
	{
		OutMessage = TEXT("Process disk speed: invalid delta time");
		return false;
	}

	const uint64 DeltaReadBytes = CurrentReadBytes >= PreviousReadBytes ? CurrentReadBytes - PreviousReadBytes : 0;
	const uint64 DeltaWriteBytes = CurrentWriteBytes >= PreviousWriteBytes ? CurrentWriteBytes - PreviousWriteBytes : 0;

	PreviousReadBytes = CurrentReadBytes;
	PreviousWriteBytes = CurrentWriteBytes;
	PreviousTimeSeconds = CurrentTimeSeconds;

	const double ReadBytesPerSecond = static_cast<double>(DeltaReadBytes) / DeltaTime;
	const double WriteBytesPerSecond = static_cast<double>(DeltaWriteBytes) / DeltaTime;

	OutReadSpeed = ConvertBytesToUnit(static_cast<int64>(ReadBytesPerSecond), Unit);
	OutWriteSpeed = ConvertBytesToUnit(static_cast<int64>(WriteBytesPerSecond), Unit);
	OutMessage = TEXT("OK");

	return true;
}

bool ULunarFLPerformance::SampleProcessGPUUsageWindows(FLunarPerformanceProcessGPUStats& OutStats)
{
	OutStats = FLunarPerformanceProcessGPUStats();
	OutStats.Message = TEXT("Undefined");

	static PDH_HQUERY Query = nullptr;
	static PDH_HCOUNTER Counter = nullptr;
	static bool bInitialized = false;

	if (!bInitialized)
	{
		if (PdhOpenQueryW(nullptr, 0, &Query) != ERROR_SUCCESS)
		{
			OutStats.Message = TEXT("Process GPU usage: PDH query creation failed");
			return false;
		}

		if (PdhAddEnglishCounterW(Query, L"\\GPU Engine(*)\\Utilization Percentage", 0, &Counter) != ERROR_SUCCESS)
		{
			PdhCloseQuery(Query);
			Query = nullptr;
			OutStats.Message = TEXT("Process GPU usage: PDH counter creation failed");
			return false;
		}

		PdhCollectQueryData(Query);
		bInitialized = true;

		OutStats.Message = TEXT("Process GPU usage: waiting for second sample");
		return false;
	}

	PdhCollectQueryData(Query);

	DWORD BufferSize = 0;
	DWORD ItemCount = 0;

	PDH_STATUS Status = PdhGetFormattedCounterArrayW(Counter, PDH_FMT_DOUBLE, &BufferSize, &ItemCount, nullptr);
	if (Status != PDH_MORE_DATA)
	{
		OutStats.Message = TEXT("Process GPU usage: PDH data unavailable");
		return false;
	}

	TArray<uint8> Buffer;
	Buffer.SetNumZeroed(static_cast<int32>(BufferSize));

	PPDH_FMT_COUNTERVALUE_ITEM_W Items = reinterpret_cast<PPDH_FMT_COUNTERVALUE_ITEM_W>(Buffer.GetData());

	Status = PdhGetFormattedCounterArrayW(Counter, PDH_FMT_DOUBLE, &BufferSize, &ItemCount, Items);
	if (Status != ERROR_SUCCESS)
	{
		OutStats.Message = TEXT("Process GPU usage: PDH formatted data failed");
		return false;
	}

	const uint32 CurrentProcessId = FPlatformProcess::GetCurrentProcessId();
	const FString CurrentProcessIdToken = FString::Printf(TEXT("pid_%u_"), CurrentProcessId);

	bool bFoundAnyEngine = false;

	for (DWORD Index = 0; Index < ItemCount; ++Index)
	{
		if (Items[Index].FmtValue.CStatus != ERROR_SUCCESS)
		{
			continue;
		}

		const FString InstanceName = Items[Index].szName ? FString(Items[Index].szName) : FString();

		if (!InstanceName.Contains(CurrentProcessIdToken))
		{
			continue;
		}

		const float EngineValue = FMath::Max(0.0f, static_cast<float>(Items[Index].FmtValue.doubleValue));

		if (EngineValue <= 0.0f)
		{
			continue;
		}

		bFoundAnyEngine = true;
		AccumulateProcessGPUEngineValue(InstanceName, EngineValue, OutStats);
	}

	OutStats.TotalUsagePercent = FMath::Clamp(OutStats.TotalUsagePercent, 0.0f, 100.0f);
	OutStats.Graphics3DUsagePercent = FMath::Clamp(OutStats.Graphics3DUsagePercent, 0.0f, 100.0f);
	OutStats.ComputeUsagePercent = FMath::Clamp(OutStats.ComputeUsagePercent, 0.0f, 100.0f);
	OutStats.CopyUsagePercent = FMath::Clamp(OutStats.CopyUsagePercent, 0.0f, 100.0f);
	OutStats.VideoDecodeUsagePercent = FMath::Clamp(OutStats.VideoDecodeUsagePercent, 0.0f, 100.0f);
	OutStats.VideoEncodeUsagePercent = FMath::Clamp(OutStats.VideoEncodeUsagePercent, 0.0f, 100.0f);

	OutStats.bStatsAvailable = bFoundAnyEngine;
	OutStats.Message = bFoundAnyEngine ? TEXT("OK") : TEXT("Process GPU usage: no active GPU engine for current process");

	return bFoundAnyEngine;
}

void ULunarFLPerformance::AccumulateProcessGPUEngineValue(const FString& InstanceName, float Value, FLunarPerformanceProcessGPUStats& InOutStats)
{
	InOutStats.TotalUsagePercent += Value;

	if (InstanceName.Contains(TEXT("engtype_3D")))
	{
		InOutStats.Graphics3DUsagePercent += Value;
		return;
	}

	if (InstanceName.Contains(TEXT("engtype_Compute")))
	{
		InOutStats.ComputeUsagePercent += Value;
		return;
	}

	if (InstanceName.Contains(TEXT("engtype_Copy")))
	{
		InOutStats.CopyUsagePercent += Value;
		return;
	}

	if (InstanceName.Contains(TEXT("engtype_VideoDecode")))
	{
		InOutStats.VideoDecodeUsagePercent += Value;
		return;
	}

	if (InstanceName.Contains(TEXT("engtype_VideoEncode")))
	{
		InOutStats.VideoEncodeUsagePercent += Value;
		return;
	}
}

#endif

// Process GPU public

bool ULunarFLPerformance::GetProcessGPUUsageStats(FLunarPerformanceProcessGPUStats& OutStats)
{
	OutStats = FLunarPerformanceProcessGPUStats();
	OutStats.Message = TEXT("Undefined");

#if PLATFORM_WINDOWS
	return SampleProcessGPUUsageWindows(OutStats);
#else
	OutStats.Message = GetUndefinedMessage(TEXT("Process GPU usage"));
	return false;
#endif
}

FString ULunarFLPerformance::GetProcessGPUUsageSummaryString()
{
	FLunarPerformanceProcessGPUStats Stats;

	if (!GetProcessGPUUsageStats(Stats))
	{
		return FString::Printf(TEXT("Process GPU Usage: %s"), *Stats.Message);
	}

	return FString::Printf(
		TEXT("Process GPU Usage: Total: %s | 3D: %s | Compute: %s | Copy: %s | Decode: %s | Encode: %s"),
		*LunarFL::String::FormatPercent(Stats.TotalUsagePercent, 1),
		*LunarFL::String::FormatPercent(Stats.Graphics3DUsagePercent, 1),
		*LunarFL::String::FormatPercent(Stats.ComputeUsagePercent, 1),
		*LunarFL::String::FormatPercent(Stats.CopyUsagePercent, 1),
		*LunarFL::String::FormatPercent(Stats.VideoDecodeUsagePercent, 1),
		*LunarFL::String::FormatPercent(Stats.VideoEncodeUsagePercent, 1)
	);
}

// CPU public

bool ULunarFLPerformance::GetCPUUsagePercent(float& OutCPUUsagePercent, FString& OutMessage)
{
	return GetSystemCPUUsagePercent(OutCPUUsagePercent, OutMessage);
}

bool ULunarFLPerformance::GetSystemCPUUsagePercent(float& OutSystemCPUUsagePercent, FString& OutMessage)
{
	OutSystemCPUUsagePercent = 0.0f;
	OutMessage = TEXT("Undefined");

#if PLATFORM_WINDOWS
	TArray<float> CoreUsages;
	if (!SampleCPUCoreUsageWindows(CoreUsages, OutMessage))
	{
		return false;
	}

	float Sum = 0.0f;
	for (const float Value : CoreUsages)
	{
		Sum += Value;
	}

	OutSystemCPUUsagePercent = CoreUsages.Num() > 0 ? Sum / static_cast<float>(CoreUsages.Num()) : 0.0f;
	OutMessage = TEXT("OK");

	return true;
#else
	OutMessage = GetUndefinedMessage(TEXT("System CPU usage"));
	return false;
#endif
}

bool ULunarFLPerformance::GetProcessCPUUsagePercent(float& OutProcessCPUUsagePercent, FString& OutMessage)
{
	OutProcessCPUUsagePercent = 0.0f;
	OutMessage = TEXT("Undefined");

#if PLATFORM_WINDOWS
	return SampleProcessCPUUsageWindows(OutProcessCPUUsagePercent, OutMessage);
#else
	OutMessage = GetUndefinedMessage(TEXT("Process CPU usage"));
	return false;
#endif
}

bool ULunarFLPerformance::GetCPUCoreUsagePercents(TArray<float>& OutCoreUsagePercents, FString& OutMessage)
{
	OutCoreUsagePercents.Reset();
	OutMessage = TEXT("Undefined");

#if PLATFORM_WINDOWS
	return SampleCPUCoreUsageWindows(OutCoreUsagePercents, OutMessage);
#else
	OutMessage = GetUndefinedMessage(TEXT("System CPU core usage"));
	return false;
#endif
}

FString ULunarFLPerformance::GetCPUUsageSummaryString()
{
	return GetSystemCPUUsageSummaryString();
}

FString ULunarFLPerformance::GetSystemCPUUsageSummaryString()
{
	FLunarPerformanceCPUStats CPUStats = GetCPUStats();

	if (!CPUStats.bStatsAvailable)
	{
		return FString::Printf(TEXT("System CPU Usage: %s"), *CPUStats.Message);
	}

	return FString::Printf(TEXT("System CPU Usage: %s"), *LunarFL::String::FormatPercent(CPUStats.SystemCPUUsagePercent, 1));
}

FString ULunarFLPerformance::GetProcessCPUUsageSummaryString()
{
	float ProcessCPUUsage = 0.0f;
	FString Message;

	if (!GetProcessCPUUsagePercent(ProcessCPUUsage, Message))
	{
		return FString::Printf(TEXT("Process CPU Usage: %s"), *Message);
	}

	return FString::Printf(TEXT("Process CPU Usage: %s"), *LunarFL::String::FormatPercent(ProcessCPUUsage, 1));
}

FString ULunarFLPerformance::GetCPUCoreUsageSummaryString(int32 MaxCoresToShow)
{
	TArray<float> CoreUsages;
	FString Message;

	if (!GetCPUCoreUsagePercents(CoreUsages, Message))
	{
		return FString::Printf(TEXT("System CPU Cores: %s"), *Message);
	}

	if (CoreUsages.Num() <= 0)
	{
		return TEXT("System CPU Cores: Empty");
	}

	const bool bShowAllCores = MaxCoresToShow <= 0;
	const int32 CoresToShow = bShowAllCores
		? CoreUsages.Num()
		: FMath::Min(CoreUsages.Num(), MaxCoresToShow);

	TArray<FString> Parts;
	Parts.Reserve(CoresToShow + 1);

	for (int32 Index = 0; Index < CoresToShow; ++Index)
	{
		Parts.Add(FString::Printf(
			TEXT("C%d: %s"),
			Index,
			*LunarFL::String::FormatPercent(CoreUsages[Index], 1)
		));
	}

	if (!bShowAllCores && CoreUsages.Num() > CoresToShow)
	{
		Parts.Add(FString::Printf(TEXT("+%d more"), CoreUsages.Num() - CoresToShow));
	}

	return FString::Printf(TEXT("System CPU Cores: %s"), *FString::Join(Parts, TEXT(", ")));
}

// Disk public

bool ULunarFLPerformance::GetProcessDiskSpeeds(float& OutReadSpeed, float& OutWriteSpeed, ELunarMemoryUnit Unit, FString& OutMessage)
{
	OutReadSpeed = 0.0f;
	OutWriteSpeed = 0.0f;
	OutMessage = TEXT("Undefined");

#if PLATFORM_WINDOWS
	return SampleProcessDiskSpeedWindows(OutReadSpeed, OutWriteSpeed, Unit, OutMessage);
#else
	OutMessage = GetUndefinedMessage(TEXT("Process disk speed"));
	return false;
#endif
}

bool ULunarFLPerformance::GetProcessDiskReadSpeed(float& OutReadSpeed, ELunarMemoryUnit Unit, FString& OutMessage)
{
	float WriteSpeed = 0.0f;
	return GetProcessDiskSpeeds(OutReadSpeed, WriteSpeed, Unit, OutMessage);
}

bool ULunarFLPerformance::GetProcessDiskWriteSpeed(float& OutWriteSpeed, ELunarMemoryUnit Unit, FString& OutMessage)
{
	float ReadSpeed = 0.0f;
	return GetProcessDiskSpeeds(ReadSpeed, OutWriteSpeed, Unit, OutMessage);
}

bool ULunarFLPerformance::GetProjectDriveDiskActivePercent(float& OutDiskActivePercent, FString& OutMessage)
{
	OutDiskActivePercent = 0.0f;
	OutMessage = TEXT("Undefined");

#if PLATFORM_WINDOWS
	return SampleProjectDriveDiskActivePercentWindows(OutDiskActivePercent, OutMessage);
#else
	OutMessage = GetUndefinedMessage(TEXT("Project drive disk active"));
	return false;
#endif
}

FString ULunarFLPerformance::GetDiskSummaryString(ELunarMemoryUnit Unit)
{
	float ReadSpeed = 0.0f;
	float WriteSpeed = 0.0f;
	float DiskActivePercent = 0.0f;

	FString SpeedMessage;
	FString ActiveMessage;

	const bool bSpeedOK = GetProcessDiskSpeeds(ReadSpeed, WriteSpeed, Unit, SpeedMessage);
	const bool bActiveOK = GetProjectDriveDiskActivePercent(DiskActivePercent, ActiveMessage);

	const FString Suffix = FString::Printf(TEXT("%s/s"), *GetMemoryUnitSuffix(Unit));

	return FString::Printf(
		TEXT("Disk Read: %s | Disk Write: %s | Disk Active: %s"),
		bSpeedOK ? *FString::Printf(TEXT("%.2f %s"), ReadSpeed, *Suffix) : *SpeedMessage,
		bSpeedOK ? *FString::Printf(TEXT("%.2f %s"), WriteSpeed, *Suffix) : *SpeedMessage,
		bActiveOK ? *LunarFL::String::FormatPercent(DiskActivePercent, 1) : *ActiveMessage
	);
}

// Hardware

int32 ULunarFLPerformance::GetPhysicalCoreCount()
{
	return FPlatformMisc::NumberOfCores();
}

int32 ULunarFLPerformance::GetLogicalCoreCount()
{
	return FPlatformMisc::NumberOfCoresIncludingHyperthreads();
}

FString ULunarFLPerformance::GetCPUBrand()
{
	return FPlatformMisc::GetCPUBrand();
}

FString ULunarFLPerformance::GetPrimaryGPUBrand()
{
	return FPlatformMisc::GetPrimaryGPUBrand();
}

FString ULunarFLPerformance::GetOSVersion()
{
	return FPlatformMisc::GetOSVersion();
}

FString ULunarFLPerformance::GetDeviceMakeAndModel()
{
	return FPlatformMisc::GetDeviceMakeAndModel();
}

FString ULunarFLPerformance::GetHardwareSummaryString()
{
	return FString::Printf(
		TEXT("CPU: %s | Cores: %d/%d | GPU: %s | OS: %s"),
		*GetCPUBrand(),
		GetPhysicalCoreCount(),
		GetLogicalCoreCount(),
		*GetPrimaryGPUBrand(),
		*GetOSVersion()
	);
}

// Snapshot structs

FLunarPerformanceFrameStats ULunarFLPerformance::GetFrameStats()
{
	FLunarPerformanceFrameStats Result;

	Result.FPS = GetFPS();
	Result.FPSFloat = GetFPSFloat();
	Result.FrameTimeMS = GetFrameTimeMS();

	return Result;
}

FLunarPerformanceMemoryStats ULunarFLPerformance::GetMemoryStats(ELunarMemoryUnit Unit)
{
	FLunarPerformanceMemoryStats Result;

	Result.TotalPhysicalMemory = GetTotalPhysicalMemory(Unit);
	Result.UsedPhysicalMemory = GetUsedPhysicalMemory(Unit);
	Result.AvailablePhysicalMemory = GetAvailablePhysicalMemory(Unit);
	Result.PhysicalMemoryUsedPercent = GetPhysicalMemoryUsedPercent();
	Result.ProcessPhysicalMemory = GetProcessPhysicalMemory(Unit);
	Result.PeakProcessPhysicalMemory = GetPeakProcessPhysicalMemory(Unit);

	return Result;
}

FLunarPerformanceGPUStats ULunarFLPerformance::GetGPUStats(ELunarMemoryUnit Unit)
{
	FLunarPerformanceGPUStats Result;

	Result.bStatsAvailable = IsGPUMemoryStatsAvailable();

	if (Result.bStatsAvailable)
	{
		Result.DedicatedVideoMemory = GetDedicatedVideoMemory(Unit);
		Result.TotalGraphicsMemory = GetTotalGraphicsMemory(Unit);
		Result.TotalDeviceWorkingMemory = GetTotalDeviceWorkingMemory(Unit);

		Result.RHITextureMemory = GetTotalTextureMemory(Unit);
		Result.StreamingTextureMemory = GetStreamingTextureMemory(Unit);
		Result.NonStreamingTextureMemory = GetNonStreamingTextureMemory(Unit);

		Result.TexturePoolSize = GetTexturePoolSize(Unit);
		Result.TexturePoolFree = GetAvailableTexturePoolMemory(Unit);
		Result.TexturePoolUsedPercent = GetTexturePoolUsedPercent();
	}

	GetProcessGPUUsageStats(Result.ProcessGPU);

	return Result;
}

FLunarPerformanceCPUStats ULunarFLPerformance::GetCPUStats()
{
	FLunarPerformanceCPUStats Result;

	FString CoreMessage;
	FString ProcessMessage;

	TArray<float> CoreUsages;
	float ProcessCPUUsage = 0.0f;

	const bool bCoreUsageOK = GetCPUCoreUsagePercents(CoreUsages, CoreMessage);
	const bool bProcessUsageOK = GetProcessCPUUsagePercent(ProcessCPUUsage, ProcessMessage);

	Result.bStatsAvailable = bCoreUsageOK || bProcessUsageOK;
	Result.CPUCoreUsagePercents = CoreUsages;
	Result.ProcessCPUUsagePercent = ProcessCPUUsage;

	if (CoreUsages.Num() > 0)
	{
		float Sum = 0.0f;

		for (const float CoreUsage : CoreUsages)
		{
			Sum += CoreUsage;
		}

		Result.SystemCPUUsagePercent = Sum / static_cast<float>(CoreUsages.Num());
		Result.CPUUsagePercent = Result.SystemCPUUsagePercent;
	}

	if (bCoreUsageOK && bProcessUsageOK)
	{
		Result.Message = TEXT("OK");
	}
	else
	{
		Result.Message = FString::Printf(TEXT("%s | %s"), *CoreMessage, *ProcessMessage);
	}

	return Result;
}

FLunarPerformanceDiskStats ULunarFLPerformance::GetDiskStats(ELunarMemoryUnit Unit)
{
	FLunarPerformanceDiskStats Result;

	float ReadSpeed = 0.0f;
	float WriteSpeed = 0.0f;
	float DiskActivePercent = 0.0f;

	FString SpeedMessage;
	FString ActiveMessage;

	const bool bSpeedOK = GetProcessDiskSpeeds(ReadSpeed, WriteSpeed, Unit, SpeedMessage);
	const bool bActiveOK = GetProjectDriveDiskActivePercent(DiskActivePercent, ActiveMessage);

	Result.bStatsAvailable = bSpeedOK || bActiveOK;
	Result.ProcessReadSpeed = ReadSpeed;
	Result.ProcessWriteSpeed = WriteSpeed;
	Result.ProjectDriveActivePercent = DiskActivePercent;

	if (bSpeedOK && bActiveOK)
	{
		Result.Message = TEXT("OK");
	}
	else
	{
		Result.Message = FString::Printf(TEXT("%s | %s"), *SpeedMessage, *ActiveMessage);
	}

	return Result;
}

FLunarPerformanceSnapshot ULunarFLPerformance::GetPerformanceSnapshot(ELunarMemoryUnit Unit)
{
	FLunarPerformanceSnapshot Result;

	Result.Frame = GetFrameStats();
	Result.Memory = GetMemoryStats(Unit);
	Result.GPU = GetGPUStats(Unit);
	Result.CPU = GetCPUStats();
	Result.Disk = GetDiskStats(Unit);

	Result.Timestamp = FDateTime::Now();
	Result.AppTimeSeconds = static_cast<float>(FApp::GetCurrentTime());
	Result.Unit = Unit;

	return Result;
}

// Summary

FString ULunarFLPerformance::GetPerformanceSummaryString(ELunarPerformanceSummaryDetail Detail, ELunarMemoryUnit Unit)
{
	switch (Detail)
	{
	case ELunarPerformanceSummaryDetail::Low:
		return GetFrameSummaryString();

	case ELunarPerformanceSummaryDetail::Normal:
		return FString::Printf(
			TEXT("%s | %s | %s"),
			*GetFrameSummaryString(),
			*GetMemorySummaryString(Detail, Unit),
			*GetGPUMemorySummaryString(Unit)
		);

	case ELunarPerformanceSummaryDetail::High:
		return FString::Printf(
			TEXT("%s | %s | %s | %s | %s | %s | %s"),
			*GetFrameSummaryString(),
			*GetMemorySummaryString(Detail, Unit),
			*GetGPUMemorySummaryString(Unit),
			*GetProcessGPUUsageSummaryString(),
			*GetSystemCPUUsageSummaryString(),
			*GetProcessCPUUsageSummaryString(),
			*GetDiskSummaryString(Unit)
		);

	case ELunarPerformanceSummaryDetail::Full:
		return FString::Printf(
			TEXT("%s | %s | %s | %s | %s | %s | %s | %s | %s"),
			*GetFrameSummaryString(),
			*GetMemorySummaryString(Detail, Unit),
			*GetGPUMemorySummaryString(Unit),
			*GetProcessGPUUsageSummaryString(),
			*GetSystemCPUUsageSummaryString(),
			*GetProcessCPUUsageSummaryString(),
			*GetCPUCoreUsageSummaryString(-1),
			*GetDiskSummaryString(Unit),
			*GetHardwareSummaryString()
		);

	default:
		return TEXT("Performance: Undefined");
	}
}

FString ULunarFLPerformance::GetPerformanceSummaryMultilineString(ELunarPerformanceSummaryDetail Detail, ELunarMemoryUnit Unit)
{
	const FLunarPerformanceSnapshot Snapshot = GetPerformanceSnapshot(Unit);
	const FString UnitSuffix = GetMemoryUnitSuffix(Unit);

	TStringBuilder<8192> Builder;

	Builder.Append(TEXT("FPS\n"));
	Builder.Appendf(TEXT("  Current: %d\n"), Snapshot.Frame.FPS);
	Builder.Appendf(TEXT("  Frame Time: %.2f ms\n"), Snapshot.Frame.FrameTimeMS);

	if (Detail >= ELunarPerformanceSummaryDetail::Normal)
	{
		Builder.Append(TEXT("\nRAM\n"));
		Builder.Appendf(
			TEXT("  Used: %.2f / %.2f %s (%s)\n"),
			Snapshot.Memory.UsedPhysicalMemory,
			Snapshot.Memory.TotalPhysicalMemory,
			*UnitSuffix,
			*LunarFL::String::FormatPercent(Snapshot.Memory.PhysicalMemoryUsedPercent, 1)
		);
		Builder.Appendf(TEXT("  Available: %.2f %s\n"), Snapshot.Memory.AvailablePhysicalMemory, *UnitSuffix);
		Builder.Appendf(TEXT("  Process RAM: %.2f %s\n"), Snapshot.Memory.ProcessPhysicalMemory, *UnitSuffix);
		Builder.Appendf(TEXT("  Peak RAM: %.2f %s\n"), Snapshot.Memory.PeakProcessPhysicalMemory, *UnitSuffix);

		Builder.Append(TEXT("\nVRAM\n"));
		if (Snapshot.GPU.bStatsAvailable)
		{
			Builder.Appendf(TEXT("  Dedicated: %.2f %s\n"), Snapshot.GPU.DedicatedVideoMemory, *UnitSuffix);
			Builder.Appendf(TEXT("  Graphics Total: %.2f %s\n"), Snapshot.GPU.TotalGraphicsMemory, *UnitSuffix);
			Builder.Appendf(TEXT("  Working: %.2f %s\n"), Snapshot.GPU.TotalDeviceWorkingMemory, *UnitSuffix);
			Builder.Appendf(TEXT("  RHI Textures: %.2f %s\n"), Snapshot.GPU.RHITextureMemory, *UnitSuffix);
			Builder.Appendf(TEXT("  Streaming Textures: %.2f %s\n"), Snapshot.GPU.StreamingTextureMemory, *UnitSuffix);
			Builder.Appendf(TEXT("  NonStreaming RHI: %.2f %s\n"), Snapshot.GPU.NonStreamingTextureMemory, *UnitSuffix);
			Builder.Appendf(TEXT("  Streaming Pool: %.2f %s\n"), Snapshot.GPU.TexturePoolSize, *UnitSuffix);
			Builder.Appendf(TEXT("  Pool Free: %.2f %s\n"), Snapshot.GPU.TexturePoolFree, *UnitSuffix);
			Builder.Appendf(TEXT("  Pool Usage: %s\n"), *LunarFL::String::FormatPercent(Snapshot.GPU.TexturePoolUsedPercent, 1));
		}
		else
		{
			Builder.Append(TEXT("  Undefined\n"));
		}

		if (Detail >= ELunarPerformanceSummaryDetail::High)
		{
			Builder.Append(TEXT("\nProcess GPU\n"));
			if (Snapshot.GPU.ProcessGPU.bStatsAvailable)
			{
				Builder.Appendf(TEXT("  Total: %s\n"), *LunarFL::String::FormatPercent(Snapshot.GPU.ProcessGPU.TotalUsagePercent, 1));
				Builder.Appendf(TEXT("  3D: %s\n"), *LunarFL::String::FormatPercent(Snapshot.GPU.ProcessGPU.Graphics3DUsagePercent, 1));
				Builder.Appendf(TEXT("  Compute: %s\n"), *LunarFL::String::FormatPercent(Snapshot.GPU.ProcessGPU.ComputeUsagePercent, 1));
				Builder.Appendf(TEXT("  Copy: %s\n"), *LunarFL::String::FormatPercent(Snapshot.GPU.ProcessGPU.CopyUsagePercent, 1));
				Builder.Appendf(TEXT("  Video Decode: %s\n"), *LunarFL::String::FormatPercent(Snapshot.GPU.ProcessGPU.VideoDecodeUsagePercent, 1));
				Builder.Appendf(TEXT("  Video Encode: %s\n"), *LunarFL::String::FormatPercent(Snapshot.GPU.ProcessGPU.VideoEncodeUsagePercent, 1));
			}
			else
			{
				Builder.Appendf(TEXT("  %s\n"), *Snapshot.GPU.ProcessGPU.Message);
			}
		}
	}

	if (Detail >= ELunarPerformanceSummaryDetail::High)
	{
		Builder.Append(TEXT("\nCPU\n"));
		if (Snapshot.CPU.bStatsAvailable)
		{
			Builder.Appendf(TEXT("  System Usage: %s\n"), *LunarFL::String::FormatPercent(Snapshot.CPU.SystemCPUUsagePercent, 1));
			Builder.Appendf(TEXT("  Process Usage: %s\n"), *LunarFL::String::FormatPercent(Snapshot.CPU.ProcessCPUUsagePercent, 1));
		}
		else
		{
			Builder.Appendf(TEXT("  Usage: %s\n"), *Snapshot.CPU.Message);
		}

		if (Detail == ELunarPerformanceSummaryDetail::Full)
		{
			Builder.Append(TEXT("  System Cores:\n"));

			if (Snapshot.CPU.CPUCoreUsagePercents.Num() > 0)
			{
				for (int32 Index = 0; Index < Snapshot.CPU.CPUCoreUsagePercents.Num(); ++Index)
				{
					Builder.Appendf(
						TEXT("    C%d: %s\n"),
						Index,
						*LunarFL::String::FormatPercent(Snapshot.CPU.CPUCoreUsagePercents[Index], 1)
					);
				}
			}
			else
			{
				Builder.Append(TEXT("    Empty\n"));
			}
		}

		Builder.Append(TEXT("\nDisk\n"));
		if (Snapshot.Disk.bStatsAvailable)
		{
			Builder.Appendf(TEXT("  Read: %.2f %s/s\n"), Snapshot.Disk.ProcessReadSpeed, *UnitSuffix);
			Builder.Appendf(TEXT("  Write: %.2f %s/s\n"), Snapshot.Disk.ProcessWriteSpeed, *UnitSuffix);
			Builder.Appendf(TEXT("  Active: %s\n"), *LunarFL::String::FormatPercent(Snapshot.Disk.ProjectDriveActivePercent, 1));
		}
		else
		{
			Builder.Appendf(TEXT("  %s\n"), *Snapshot.Disk.Message);
		}
	}

	if (Detail == ELunarPerformanceSummaryDetail::Full)
	{
		Builder.Append(TEXT("\nHardware\n"));
		Builder.Appendf(TEXT("  CPU: %s\n"), *GetCPUBrand());
		Builder.Appendf(TEXT("  Cores: %d / %d\n"), GetPhysicalCoreCount(), GetLogicalCoreCount());
		Builder.Appendf(TEXT("  GPU: %s\n"), *GetPrimaryGPUBrand());
		Builder.Appendf(TEXT("  OS: %s\n"), *GetOSVersion());
		Builder.Appendf(TEXT("  Device: %s\n"), *GetDeviceMakeAndModel());
	}

	return Builder.ToString();
}

FLunarPerformanceSettings ULunarFLPerformance::GetPerformanceSettings()
{
	const ULunarSettings* LunarSettings = GetDefault<ULunarSettings>();

	if (!LunarSettings)
	{
		return FLunarPerformanceSettings();
	}

	return LunarSettings->Performance;
}

// Paint

void ULunarFLPerformance::PaintFPSGraph(
	UWidget* WidgetSelf,
	FPaintContext& Context,
	const TArray<float>& FPSValues,
	float UpdateInterval,
	float SecondsSinceLastSample,
	int32 MaxVisibleSamples,
	float Padding,
	float LeftLabelWidth,
	int32 YTickCount,
	bool bDrawTopBorder,
	bool bDrawBottomBorder,
	bool bDrawLeftBorder,
	bool bDrawRightBorder,
	bool bDrawTargetLine,
	float TargetFPS,
	FLinearColor GraphTint,
	FLinearColor TargetLineTint,
	FLinearColor BorderTint,
	FLinearColor TickTint,
	FLinearColor TextTint,
	float LineThickness,
	float TargetLineThickness,
	float BorderThickness,
	float TickLength,
	UFont* LabelFont,
	int32 LabelFontSize,
	FName LabelFontTypeface,
	float SplineTangentStrength
)
{
	if (!WidgetSelf)
	{
		return;
	}

	const FVector2D LocalSize = WidgetSelf->GetCachedGeometry().GetLocalSize();

	if (LocalSize.X <= 1.0f || LocalSize.Y <= 1.0f)
	{
		return;
	}

	const int32 SourceCount = FPSValues.Num();

	if (SourceCount <= 0)
	{
		return;
	}

	MaxVisibleSamples = FMath::Max(MaxVisibleSamples, 2);
	YTickCount = FMath::Max(YTickCount, 1);
	UpdateInterval = FMath::Max(UpdateInterval, 0.001f);
	LabelFontSize = FMath::Max(LabelFontSize, 1);
	LineThickness = FMath::Max(LineThickness, 0.1f);
	TargetLineThickness = FMath::Max(TargetLineThickness, 0.1f);
	BorderThickness = FMath::Max(BorderThickness, 0.1f);
	TickLength = FMath::Max(TickLength, 0.0f);
	SplineTangentStrength = FMath::Max(SplineTangentStrength, 0.0f);

	const float GraphLeft = Padding + LeftLabelWidth;
	const float GraphTop = Padding;
	const float GraphRight = LocalSize.X - Padding;
	const float GraphBottom = LocalSize.Y - Padding;

	const float GraphWidth = GraphRight - GraphLeft;
	const float GraphHeight = GraphBottom - GraphTop;

	if (GraphWidth <= 1.0f || GraphHeight <= 1.0f)
	{
		return;
	}

	const int32 VisibleCount = FMath::Min(SourceCount, MaxVisibleSamples);
	const int32 FirstVisibleIndex = SourceCount - VisibleCount;

	float MaxVisibleFPS = 0.0f;

	for (int32 Index = FirstVisibleIndex; Index < SourceCount; ++Index)
	{
		MaxVisibleFPS = FMath::Max(MaxVisibleFPS, FPSValues[Index]);
	}

	if (MaxVisibleFPS <= 0.0f)
	{
		MaxVisibleFPS = 1.0f;
	}

	const float ScrollAlpha = FMath::Clamp(SecondsSinceLastSample / UpdateInterval, 0.0f, 1.0f);
	const float SampleSpacing = GraphWidth / static_cast<float>(MaxVisibleSamples - 1);

	auto MakeGraphPoint = [&](
		const int32 LocalIndex,
		const float FPSValue
		) -> FVector2D
		{
			const float SamplesFromRight = static_cast<float>((VisibleCount - 1) - LocalIndex) + ScrollAlpha;
			const float X = GraphRight - SamplesFromRight * SampleSpacing;

			const float NormalizedY = FMath::Clamp(FPSValue / MaxVisibleFPS, 0.0f, 1.0f);
			const float Y = GraphBottom - NormalizedY * GraphHeight;

			return FVector2D(X, Y);
		};

	// Border.
	if (bDrawTopBorder)
	{
		UWidgetBlueprintLibrary::DrawLine(
			Context,
			FVector2D(GraphLeft, GraphTop),
			FVector2D(GraphRight, GraphTop),
			BorderTint,
			true,
			BorderThickness
		);
	}

	if (bDrawBottomBorder)
	{
		UWidgetBlueprintLibrary::DrawLine(
			Context,
			FVector2D(GraphLeft, GraphBottom),
			FVector2D(GraphRight, GraphBottom),
			BorderTint,
			true,
			BorderThickness
		);
	}

	if (bDrawLeftBorder)
	{
		UWidgetBlueprintLibrary::DrawLine(
			Context,
			FVector2D(GraphLeft, GraphTop),
			FVector2D(GraphLeft, GraphBottom),
			BorderTint,
			true,
			BorderThickness
		);
	}

	if (bDrawRightBorder)
	{
		UWidgetBlueprintLibrary::DrawLine(
			Context,
			FVector2D(GraphRight, GraphTop),
			FVector2D(GraphRight, GraphBottom),
			BorderTint,
			true,
			BorderThickness
		);
	}

	// Y ticks and labels.
	for (int32 TickIndex = 0; TickIndex <= YTickCount; ++TickIndex)
	{
		const float Alpha = static_cast<float>(TickIndex) / static_cast<float>(YTickCount);
		const float FPSValue = MaxVisibleFPS * Alpha;
		const float Y = GraphBottom - Alpha * GraphHeight;

		UWidgetBlueprintLibrary::DrawLine(
			Context,
			FVector2D(GraphLeft - TickLength, Y),
			FVector2D(GraphLeft, Y),
			TickTint,
			true,
			BorderThickness
		);

		const FString LabelString = FString::Printf(TEXT("%.0f"), FPSValue);

		UWidgetBlueprintLibrary::DrawTextFormatted(
			Context,
			FText::FromString(LabelString),
			FVector2D(Padding, Y - static_cast<float>(LabelFontSize) * 0.5f),
			LabelFont,
			LabelFontSize,
			LabelFontTypeface,
			TextTint
		);
	}

	// Target FPS line.
	if (bDrawTargetLine && TargetFPS > 0.0f)
	{
		const float TargetNormalized = FMath::Clamp(TargetFPS / MaxVisibleFPS, 0.0f, 1.0f);
		const float TargetY = GraphBottom - TargetNormalized * GraphHeight;

		UWidgetBlueprintLibrary::DrawLine(
			Context,
			FVector2D(GraphLeft, TargetY),
			FVector2D(GraphRight, TargetY),
			TargetLineTint,
			true,
			TargetLineThickness
		);
	}

	if (VisibleCount < 2)
	{
		return;
	}

	// FPS graph.
	for (int32 LocalIndex = 0; LocalIndex < VisibleCount - 1; ++LocalIndex)
	{
		const int32 CurrentSourceIndex = FirstVisibleIndex + LocalIndex;
		const int32 NextSourceIndex = CurrentSourceIndex + 1;

		const float CurrentFPS = FPSValues[CurrentSourceIndex];
		const float NextFPS = FPSValues[NextSourceIndex];

		const int32 PreviousLocalIndex = FMath::Max(LocalIndex - 1, 0);
		const int32 AfterLocalIndex = FMath::Min(LocalIndex + 2, VisibleCount - 1);

		const int32 PreviousSourceIndex = FirstVisibleIndex + PreviousLocalIndex;
		const int32 AfterSourceIndex = FirstVisibleIndex + AfterLocalIndex;

		const float PreviousFPS = FPSValues[PreviousSourceIndex];
		const float AfterFPS = FPSValues[AfterSourceIndex];

		const FVector2D PreviousPoint = MakeGraphPoint(PreviousLocalIndex, PreviousFPS);
		const FVector2D CurrentPoint = MakeGraphPoint(LocalIndex, CurrentFPS);
		const FVector2D NextPoint = MakeGraphPoint(LocalIndex + 1, NextFPS);
		const FVector2D AfterPoint = MakeGraphPoint(AfterLocalIndex, AfterFPS);

		const FVector2D StartTangent = (NextPoint - PreviousPoint) * SplineTangentStrength;
		const FVector2D EndTangent = (AfterPoint - CurrentPoint) * SplineTangentStrength;

		UWidgetBlueprintLibrary::DrawSpline(
			Context,
			CurrentPoint,
			StartTangent,
			NextPoint,
			EndTangent,
			GraphTint,
			LineThickness
		);
	}
}
