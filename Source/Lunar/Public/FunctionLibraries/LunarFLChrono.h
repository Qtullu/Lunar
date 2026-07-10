// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LunarFLChrono.generated.h"

/**
 * @file LunarFLChrono.h
 * @brief Time helper function library
 */

 /**
  * @brief Blueprint utility functions for advanced time operations
  * @ingroup LunarFLChrono
  */
UCLASS()
class LUNAR_API ULunarFLChrono : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * @brief Formats seconds as a compact timer string
	 * @ingroup LunarFLChrono
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Chrono|Formatting", meta = (DisplayName = "Format Timer"))
	static FString FormatTimer(double TotalSeconds, bool bAlwaysShowHours = false, bool bShowMilliseconds = false);

	/**
	 * @brief Formats a time span as a human readable duration
	 * @ingroup LunarFLChrono
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Chrono|Formatting", meta = (DisplayName = "Format Duration"))
	static FString FormatDuration(const FTimespan& Duration, int32 MaximumParts = 2);

	/**
	 * @brief Formats seconds as a human readable duration
	 * @ingroup LunarFLChrono
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Chrono|Formatting", meta = (DisplayName = "Format Duration From Seconds"))
	static FString FormatDurationFromSeconds(double TotalSeconds, int32 MaximumParts = 2);

	/**
	 * @brief Formats a date relative to the current UTC time
	 * @ingroup LunarFLChrono
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Chrono|Formatting", meta = (DisplayName = "Format Relative Date Time"))
	static FString FormatRelativeDateTime(const FDateTime& DateTime);

	/**
	 * @brief Gets the beginning of the day
	 * @ingroup LunarFLChrono
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Chrono|Date", meta = (DisplayName = "Get Start Of Day"))
	static FDateTime GetStartOfDay(const FDateTime& DateTime);

	/**
	 * @brief Gets the end of the day
	 * @ingroup LunarFLChrono
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Chrono|Date", meta = (DisplayName = "Get End Of Day"))
	static FDateTime GetEndOfDay(const FDateTime& DateTime);

	/**
	 * @brief Gets the beginning of the week
	 * @ingroup LunarFLChrono
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Chrono|Date", meta = (DisplayName = "Get Start Of Week"))
	static FDateTime GetStartOfWeek(const FDateTime& DateTime);

	/**
	 * @brief Gets the end of the week
	 * @ingroup LunarFLChrono
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Chrono|Date", meta = (DisplayName = "Get End Of Week"))
	static FDateTime GetEndOfWeek(const FDateTime& DateTime);

	/**
	 * @brief Gets the beginning of the month
	 * @ingroup LunarFLChrono
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Chrono|Date", meta = (DisplayName = "Get Start Of Month"))
	static FDateTime GetStartOfMonth(const FDateTime& DateTime);

	/**
	 * @brief Gets the end of the month
	 * @ingroup LunarFLChrono
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Chrono|Date", meta = (DisplayName = "Get End Of Month"))
	static FDateTime GetEndOfMonth(const FDateTime& DateTime);

	/**
	 * @brief Gets the number of days in the selected month
	 * @ingroup LunarFLChrono
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Chrono|Date", meta = (DisplayName = "Get Days In Month"))
	static int32 GetDaysInMonth(int32 Year, int32 Month);

	/**
	 * @brief Checks whether two values represent the same calendar day
	 * @ingroup LunarFLChrono
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Chrono|Comparison", meta = (DisplayName = "Is Same Day"))
	static bool IsSameDay(const FDateTime& A, const FDateTime& B);

	/**
	 * @brief Checks whether a date represents the current local day
	 * @ingroup LunarFLChrono
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Chrono|Comparison", meta = (DisplayName = "Is Today"))
	static bool IsToday(const FDateTime& DateTime);

	/**
	 * @brief Checks whether a date represents the previous local day
	 * @ingroup LunarFLChrono
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Chrono|Comparison", meta = (DisplayName = "Is Yesterday"))
	static bool IsYesterday(const FDateTime& DateTime);

	/**
	 * @brief Checks whether a date represents the next local day
	 * @ingroup LunarFLChrono
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Chrono|Comparison", meta = (DisplayName = "Is Tomorrow"))
	static bool IsTomorrow(const FDateTime& DateTime);

	/**
	 * @brief Calculates the full age for a selected date
	 * @ingroup LunarFLChrono
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Chrono|Date", meta = (DisplayName = "Calculate Age"))
	static int32 CalculateAge(const FDateTime& BirthDate, const FDateTime& Date);

	/**
	 * @brief Adds calendar months while preserving a valid day
	 * @ingroup LunarFLChrono
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Chrono|Arithmetic", meta = (DisplayName = "Add Calendar Months"))
	static FDateTime AddCalendarMonths(const FDateTime& DateTime, int32 Months);

	/**
	 * @brief Adds calendar years while preserving a valid day
	 * @ingroup LunarFLChrono
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Chrono|Arithmetic", meta = (DisplayName = "Add Calendar Years"))
	static FDateTime AddCalendarYears(const FDateTime& DateTime, int32 Years);

	/**
	 * @brief Calculates progress between two dates
	 * @ingroup LunarFLChrono
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Chrono|Progress", meta = (DisplayName = "Get Date Time Progress"))
	static double GetDateTimeProgress(const FDateTime& StartDateTime, const FDateTime& EndDateTime, const FDateTime& CurrentDateTime, bool bClamp = true);

	/**
	 * @brief Calculates remaining time while preventing negative values
	 * @ingroup LunarFLChrono
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Chrono|Progress", meta = (DisplayName = "Get Remaining Time"))
	static FTimespan GetRemainingTime(const FDateTime& TargetDateTime, const FDateTime& CurrentDateTime);

	/**
	 * @brief Calculates elapsed time while preventing negative values
	 * @ingroup LunarFLChrono
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Chrono|Progress", meta = (DisplayName = "Get Elapsed Time"))
	static FTimespan GetElapsedTime(const FDateTime& StartDateTime, const FDateTime& CurrentDateTime);

	/**
	 * @brief Rounds a time span to the selected interval
	 * @ingroup LunarFLChrono
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Chrono|Rounding", meta = (DisplayName = "Round Time Span"))
	static FTimespan RoundTimeSpan(const FTimespan& TimeSpan, const FTimespan& Interval);

	/**
	 * @brief Floors a time span to the selected interval
	 * @ingroup LunarFLChrono
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Chrono|Rounding", meta = (DisplayName = "Floor Time Span"))
	static FTimespan FloorTimeSpan(const FTimespan& TimeSpan, const FTimespan& Interval);

	/**
	 * @brief Ceils a time span to the selected interval
	 * @ingroup LunarFLChrono
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Chrono|Rounding", meta = (DisplayName = "Ceil Time Span"))
	static FTimespan CeilTimeSpan(const FTimespan& TimeSpan, const FTimespan& Interval);

	/**
	 * @brief Returns a time span with no negative value
	 * @ingroup LunarFLChrono
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Chrono|Utility", meta = (DisplayName = "Clamp Time Span To Zero"))
	static FTimespan ClampTimeSpanToZero(const FTimespan& TimeSpan);

	/**
	 * @brief Calculates a cooldown progress value
	 * @ingroup LunarFLChrono
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Chrono|Progress", meta = (DisplayName = "Get Cooldown Progress"))
	static double GetCooldownProgress(double StartTime, double Duration, double CurrentTime, bool bClamp = true);

	/**
	 * @brief Calculates the remaining cooldown duration
	 * @ingroup LunarFLChrono
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Chrono|Progress", meta = (DisplayName = "Get Cooldown Remaining"))
	static double GetCooldownRemaining(double StartTime, double Duration, double CurrentTime);

	/**
	 * @brief Checks whether a cooldown has completed
	 * @ingroup LunarFLChrono
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Chrono|Progress", meta = (DisplayName = "Is Cooldown Complete"))
	static bool IsCooldownComplete(double StartTime, double Duration, double CurrentTime);

	/**
	 * @brief Calculates the next repeating interval time
	 * @ingroup LunarFLChrono
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Chrono|Utility", meta = (DisplayName = "Get Next Interval Time"))
	static double GetNextIntervalTime(double CurrentTime, double Interval, double Offset = 0.0);
};