// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "LunarFLChrono.h"

#include "NativeGameplayTags.h"
#include "Subsystems/Console/LunarConsoleSubsystem.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Lunar_Chrono, "Lunar.Chrono");

FString ULunarFLChrono::FormatTimer(double TotalSeconds, bool bAlwaysShowHours, bool bShowMilliseconds)
{
	if (!FMath::IsFinite(TotalSeconds))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Chrono, ELunarConsoleMessageVerbosity::Error, TEXT("FormatTimer failed because TotalSeconds is not finite"));
		return FString();
	}

	const bool bIsNegative = TotalSeconds < 0.0;
	const int64 TotalMilliseconds = FMath::FloorToInt64(FMath::Abs(TotalSeconds) * 1000.0);

	const int64 Hours = TotalMilliseconds / 3600000;
	const int32 Minutes = static_cast<int32>((TotalMilliseconds / 60000) % 60);
	const int32 Seconds = static_cast<int32>((TotalMilliseconds / 1000) % 60);
	const int32 Milliseconds = static_cast<int32>(TotalMilliseconds % 1000);

	FString Result;

	if (bAlwaysShowHours || Hours > 0)
	{
		Result = bShowMilliseconds ? FString::Printf(TEXT("%lld:%02d:%02d.%03d"), Hours, Minutes, Seconds, Milliseconds) : FString::Printf(TEXT("%lld:%02d:%02d"), Hours, Minutes, Seconds);
	}
	else
	{
		Result = bShowMilliseconds ? FString::Printf(TEXT("%02d:%02d.%03d"), Minutes, Seconds, Milliseconds) : FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
	}

	return bIsNegative ? TEXT("-") + Result : Result;
}

FString ULunarFLChrono::FormatDuration(const FTimespan& Duration, int32 MaximumParts)
{
	if (MaximumParts <= 0)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Chrono, ELunarConsoleMessageVerbosity::Error, TEXT("FormatDuration failed because MaximumParts must be greater than zero"));
		return FString();
	}

	MaximumParts = FMath::Min(MaximumParts, 5);

	const bool bIsNegative = Duration.GetTicks() < 0;
	int64 RemainingMilliseconds = FMath::RoundToInt64(FMath::Abs(Duration.GetTotalMilliseconds()));

	const int64 Days = RemainingMilliseconds / 86400000;
	RemainingMilliseconds %= 86400000;

	const int64 Hours = RemainingMilliseconds / 3600000;
	RemainingMilliseconds %= 3600000;

	const int64 Minutes = RemainingMilliseconds / 60000;
	RemainingMilliseconds %= 60000;

	const int64 Seconds = RemainingMilliseconds / 1000;
	const int64 Milliseconds = RemainingMilliseconds % 1000;

	TArray<FString> Parts;
	Parts.Reserve(MaximumParts);

	if (Days > 0 && Parts.Num() < MaximumParts)
	{
		Parts.Add(FString::Printf(TEXT("%lld %s"), Days, Days == 1 ? TEXT("day") : TEXT("days")));
	}

	if (Hours > 0 && Parts.Num() < MaximumParts)
	{
		Parts.Add(FString::Printf(TEXT("%lld %s"), Hours, Hours == 1 ? TEXT("hour") : TEXT("hours")));
	}

	if (Minutes > 0 && Parts.Num() < MaximumParts)
	{
		Parts.Add(FString::Printf(TEXT("%lld %s"), Minutes, Minutes == 1 ? TEXT("minute") : TEXT("minutes")));
	}

	if (Seconds > 0 && Parts.Num() < MaximumParts)
	{
		Parts.Add(FString::Printf(TEXT("%lld %s"), Seconds, Seconds == 1 ? TEXT("second") : TEXT("seconds")));
	}

	if (Milliseconds > 0 && Parts.Num() < MaximumParts)
	{
		Parts.Add(FString::Printf(TEXT("%lld ms"), Milliseconds));
	}

	if (Parts.IsEmpty())
	{
		return TEXT("0 seconds");
	}

	const FString Result = FString::Join(Parts, TEXT(" "));

	return bIsNegative ? TEXT("-") + Result : Result;
}

FString ULunarFLChrono::FormatDurationFromSeconds(double TotalSeconds, int32 MaximumParts)
{
	if (!FMath::IsFinite(TotalSeconds))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Chrono, ELunarConsoleMessageVerbosity::Error, TEXT("FormatDurationFromSeconds failed because TotalSeconds is not finite"));
		return FString();
	}

	return FormatDuration(FTimespan::FromSeconds(TotalSeconds), MaximumParts);
}

FString ULunarFLChrono::FormatRelativeDateTime(const FDateTime& DateTime)
{
	const FDateTime CurrentDateTime = FDateTime::UtcNow();
	const FTimespan Difference = DateTime - CurrentDateTime;
	const bool bIsFuture = Difference.GetTicks() > 0;
	const double AbsoluteSeconds = FMath::Abs(Difference.GetTotalSeconds());

	if (AbsoluteSeconds < 5.0)
	{
		return TEXT("just now");
	}

	if (AbsoluteSeconds < 60.0)
	{
		const int32 Seconds = FMath::FloorToInt(AbsoluteSeconds);
		return bIsFuture ? FString::Printf(TEXT("in %d %s"), Seconds, Seconds == 1 ? TEXT("second") : TEXT("seconds")) : FString::Printf(TEXT("%d %s ago"), Seconds, Seconds == 1 ? TEXT("second") : TEXT("seconds"));
	}

	if (AbsoluteSeconds < 3600.0)
	{
		const int32 Minutes = FMath::FloorToInt(AbsoluteSeconds / 60.0);
		return bIsFuture ? FString::Printf(TEXT("in %d %s"), Minutes, Minutes == 1 ? TEXT("minute") : TEXT("minutes")) : FString::Printf(TEXT("%d %s ago"), Minutes, Minutes == 1 ? TEXT("minute") : TEXT("minutes"));
	}

	if (AbsoluteSeconds < 86400.0)
	{
		const int32 Hours = FMath::FloorToInt(AbsoluteSeconds / 3600.0);
		return bIsFuture ? FString::Printf(TEXT("in %d %s"), Hours, Hours == 1 ? TEXT("hour") : TEXT("hours")) : FString::Printf(TEXT("%d %s ago"), Hours, Hours == 1 ? TEXT("hour") : TEXT("hours"));
	}

	if (AbsoluteSeconds < 604800.0)
	{
		const int32 Days = FMath::FloorToInt(AbsoluteSeconds / 86400.0);
		return bIsFuture ? FString::Printf(TEXT("in %d %s"), Days, Days == 1 ? TEXT("day") : TEXT("days")) : FString::Printf(TEXT("%d %s ago"), Days, Days == 1 ? TEXT("day") : TEXT("days"));
	}

	if (AbsoluteSeconds < 2629800.0)
	{
		const int32 Weeks = FMath::FloorToInt(AbsoluteSeconds / 604800.0);
		return bIsFuture ? FString::Printf(TEXT("in %d %s"), Weeks, Weeks == 1 ? TEXT("week") : TEXT("weeks")) : FString::Printf(TEXT("%d %s ago"), Weeks, Weeks == 1 ? TEXT("week") : TEXT("weeks"));
	}

	if (AbsoluteSeconds < 31557600.0)
	{
		const int32 Months = FMath::FloorToInt(AbsoluteSeconds / 2629800.0);
		return bIsFuture ? FString::Printf(TEXT("in %d %s"), Months, Months == 1 ? TEXT("month") : TEXT("months")) : FString::Printf(TEXT("%d %s ago"), Months, Months == 1 ? TEXT("month") : TEXT("months"));
	}

	const int32 Years = FMath::FloorToInt(AbsoluteSeconds / 31557600.0);

	return bIsFuture ? FString::Printf(TEXT("in %d %s"), Years, Years == 1 ? TEXT("year") : TEXT("years")) : FString::Printf(TEXT("%d %s ago"), Years, Years == 1 ? TEXT("year") : TEXT("years"));
}

FDateTime ULunarFLChrono::GetStartOfDay(const FDateTime& DateTime)
{
	return FDateTime(DateTime.GetYear(), DateTime.GetMonth(), DateTime.GetDay());
}

FDateTime ULunarFLChrono::GetEndOfDay(const FDateTime& DateTime)
{
	return GetStartOfDay(DateTime) + FTimespan::FromDays(1.0) - FTimespan(1);
}

FDateTime ULunarFLChrono::GetStartOfWeek(const FDateTime& DateTime)
{
	const FDateTime StartOfDay = GetStartOfDay(DateTime);
	const int32 DaysSinceMonday = (static_cast<int32>(DateTime.GetDayOfWeek()) + 6) % 7;

	return StartOfDay - FTimespan::FromDays(DaysSinceMonday);
}

FDateTime ULunarFLChrono::GetEndOfWeek(const FDateTime& DateTime)
{
	return GetStartOfWeek(DateTime) + FTimespan::FromDays(7.0) - FTimespan(1);
}

FDateTime ULunarFLChrono::GetStartOfMonth(const FDateTime& DateTime)
{
	return FDateTime(DateTime.GetYear(), DateTime.GetMonth(), 1);
}

FDateTime ULunarFLChrono::GetEndOfMonth(const FDateTime& DateTime)
{
	const int32 DaysInMonth = FDateTime::DaysInMonth(DateTime.GetYear(), DateTime.GetMonth());

	return FDateTime(DateTime.GetYear(), DateTime.GetMonth(), DaysInMonth) + FTimespan::FromDays(1.0) - FTimespan(1);
}

int32 ULunarFLChrono::GetDaysInMonth(int32 Year, int32 Month)
{
	if (Year < 1 || Year > 9999)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Chrono, ELunarConsoleMessageVerbosity::Error, TEXT("GetDaysInMonth failed because Year must be between 1 and 9999"));
		return 0;
	}

	if (Month < 1 || Month > 12)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Chrono, ELunarConsoleMessageVerbosity::Error, TEXT("GetDaysInMonth failed because Month must be between 1 and 12"));
		return 0;
	}

	return FDateTime::DaysInMonth(Year, Month);
}

bool ULunarFLChrono::IsSameDay(const FDateTime& A, const FDateTime& B)
{
	return A.GetYear() == B.GetYear() && A.GetMonth() == B.GetMonth() && A.GetDay() == B.GetDay();
}

bool ULunarFLChrono::IsToday(const FDateTime& DateTime)
{
	return IsSameDay(DateTime, FDateTime::Now());
}

bool ULunarFLChrono::IsYesterday(const FDateTime& DateTime)
{
	return IsSameDay(DateTime, FDateTime::Now() - FTimespan::FromDays(1.0));
}

bool ULunarFLChrono::IsTomorrow(const FDateTime& DateTime)
{
	return IsSameDay(DateTime, FDateTime::Now() + FTimespan::FromDays(1.0));
}

int32 ULunarFLChrono::CalculateAge(const FDateTime& BirthDate, const FDateTime& Date)
{
	if (BirthDate > Date)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Chrono, ELunarConsoleMessageVerbosity::Error, TEXT("CalculateAge failed because BirthDate is later than Date"));
		return 0;
	}

	int32 Age = Date.GetYear() - BirthDate.GetYear();

	if (Date.GetMonth() < BirthDate.GetMonth() || Date.GetMonth() == BirthDate.GetMonth() && Date.GetDay() < BirthDate.GetDay())
	{
		--Age;
	}

	return Age;
}

FDateTime ULunarFLChrono::AddCalendarMonths(const FDateTime& DateTime, int32 Months)
{
	const int64 CurrentMonthIndex = static_cast<int64>(DateTime.GetYear() - 1) * 12 + DateTime.GetMonth() - 1;
	const int64 TargetMonthIndex = CurrentMonthIndex + Months;
	constexpr int64 MaximumMonthIndex = static_cast<int64>(9999 - 1) * 12 + 11;

	if (TargetMonthIndex < 0 || TargetMonthIndex > MaximumMonthIndex)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Chrono, ELunarConsoleMessageVerbosity::Error, TEXT("AddCalendarMonths failed because the resulting date is outside the supported range"));
		return DateTime;
	}

	const int32 TargetYear = static_cast<int32>(TargetMonthIndex / 12) + 1;
	const int32 TargetMonth = static_cast<int32>(TargetMonthIndex % 12) + 1;
	const int32 TargetDay = FMath::Min(DateTime.GetDay(), FDateTime::DaysInMonth(TargetYear, TargetMonth));

	return FDateTime(TargetYear, TargetMonth, TargetDay, DateTime.GetHour(), DateTime.GetMinute(), DateTime.GetSecond(), DateTime.GetMillisecond());
}

FDateTime ULunarFLChrono::AddCalendarYears(const FDateTime& DateTime, int32 Years)
{
	const int64 TargetYearValue = static_cast<int64>(DateTime.GetYear()) + Years;

	if (TargetYearValue < 1 || TargetYearValue > 9999)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Chrono, ELunarConsoleMessageVerbosity::Error, TEXT("AddCalendarYears failed because the resulting date is outside the supported range"));
		return DateTime;
	}

	const int32 TargetYear = static_cast<int32>(TargetYearValue);
	const int32 TargetDay = FMath::Min(DateTime.GetDay(), FDateTime::DaysInMonth(TargetYear, DateTime.GetMonth()));

	return FDateTime(TargetYear, DateTime.GetMonth(), TargetDay, DateTime.GetHour(), DateTime.GetMinute(), DateTime.GetSecond(), DateTime.GetMillisecond());
}

double ULunarFLChrono::GetDateTimeProgress(const FDateTime& StartDateTime, const FDateTime& EndDateTime, const FDateTime& CurrentDateTime, bool bClamp)
{
	if (EndDateTime <= StartDateTime)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Chrono, ELunarConsoleMessageVerbosity::Error, TEXT("GetDateTimeProgress failed because EndDateTime must be later than StartDateTime"));
		return 0.0;
	}

	const double Progress = static_cast<double>((CurrentDateTime - StartDateTime).GetTicks()) / static_cast<double>((EndDateTime - StartDateTime).GetTicks());

	return bClamp ? FMath::Clamp(Progress, 0.0, 1.0) : Progress;
}

FTimespan ULunarFLChrono::GetRemainingTime(const FDateTime& TargetDateTime, const FDateTime& CurrentDateTime)
{
	if (TargetDateTime <= CurrentDateTime)
	{
		return FTimespan::Zero();
	}

	return TargetDateTime - CurrentDateTime;
}

FTimespan ULunarFLChrono::GetElapsedTime(const FDateTime& StartDateTime, const FDateTime& CurrentDateTime)
{
	if (CurrentDateTime <= StartDateTime)
	{
		return FTimespan::Zero();
	}

	return CurrentDateTime - StartDateTime;
}

FTimespan ULunarFLChrono::RoundTimeSpan(const FTimespan& TimeSpan, const FTimespan& Interval)
{
	const int64 IntervalTicks = Interval.GetTicks();

	if (IntervalTicks <= 0)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Chrono, ELunarConsoleMessageVerbosity::Error, TEXT("RoundTimeSpan failed because Interval must be greater than zero"));
		return TimeSpan;
	}

	const int64 TimeSpanTicks = TimeSpan.GetTicks();
	int64 Quotient = TimeSpanTicks / IntervalTicks;
	const int64 Remainder = TimeSpanTicks % IntervalTicks;

	if (FMath::Abs(Remainder) >= IntervalTicks - FMath::Abs(Remainder))
	{
		Quotient += TimeSpanTicks >= 0 ? 1 : -1;
	}

	return FTimespan(Quotient * IntervalTicks);
}

FTimespan ULunarFLChrono::FloorTimeSpan(const FTimespan& TimeSpan, const FTimespan& Interval)
{
	const int64 IntervalTicks = Interval.GetTicks();

	if (IntervalTicks <= 0)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Chrono, ELunarConsoleMessageVerbosity::Error, TEXT("FloorTimeSpan failed because Interval must be greater than zero"));
		return TimeSpan;
	}

	const int64 TimeSpanTicks = TimeSpan.GetTicks();
	int64 Quotient = TimeSpanTicks / IntervalTicks;
	const int64 Remainder = TimeSpanTicks % IntervalTicks;

	if (Remainder < 0)
	{
		--Quotient;
	}

	return FTimespan(Quotient * IntervalTicks);
}

FTimespan ULunarFLChrono::CeilTimeSpan(const FTimespan& TimeSpan, const FTimespan& Interval)
{
	const int64 IntervalTicks = Interval.GetTicks();

	if (IntervalTicks <= 0)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Chrono, ELunarConsoleMessageVerbosity::Error, TEXT("CeilTimeSpan failed because Interval must be greater than zero"));
		return TimeSpan;
	}

	const int64 TimeSpanTicks = TimeSpan.GetTicks();
	int64 Quotient = TimeSpanTicks / IntervalTicks;
	const int64 Remainder = TimeSpanTicks % IntervalTicks;

	if (Remainder > 0)
	{
		++Quotient;
	}

	return FTimespan(Quotient * IntervalTicks);
}

FTimespan ULunarFLChrono::ClampTimeSpanToZero(const FTimespan& TimeSpan)
{
	return TimeSpan.GetTicks() < 0 ? FTimespan::Zero() : TimeSpan;
}

double ULunarFLChrono::GetCooldownProgress(double StartTime, double Duration, double CurrentTime, bool bClamp)
{
	if (!FMath::IsFinite(StartTime) || !FMath::IsFinite(Duration) || !FMath::IsFinite(CurrentTime))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Chrono, ELunarConsoleMessageVerbosity::Error, TEXT("GetCooldownProgress failed because one or more values are not finite"));
		return 0.0;
	}

	if (Duration <= 0.0)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Chrono, ELunarConsoleMessageVerbosity::Error, TEXT("GetCooldownProgress failed because Duration must be greater than zero"));
		return 0.0;
	}

	const double Progress = (CurrentTime - StartTime) / Duration;

	return bClamp ? FMath::Clamp(Progress, 0.0, 1.0) : Progress;
}

double ULunarFLChrono::GetCooldownRemaining(double StartTime, double Duration, double CurrentTime)
{
	if (!FMath::IsFinite(StartTime) || !FMath::IsFinite(Duration) || !FMath::IsFinite(CurrentTime))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Chrono, ELunarConsoleMessageVerbosity::Error, TEXT("GetCooldownRemaining failed because one or more values are not finite"));
		return 0.0;
	}

	if (Duration <= 0.0)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Chrono, ELunarConsoleMessageVerbosity::Error, TEXT("GetCooldownRemaining failed because Duration must be greater than zero"));
		return 0.0;
	}

	return FMath::Max(StartTime + Duration - CurrentTime, 0.0);
}

bool ULunarFLChrono::IsCooldownComplete(double StartTime, double Duration, double CurrentTime)
{
	if (!FMath::IsFinite(StartTime) || !FMath::IsFinite(Duration) || !FMath::IsFinite(CurrentTime))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Chrono, ELunarConsoleMessageVerbosity::Error, TEXT("IsCooldownComplete failed because one or more values are not finite"));
		return false;
	}

	if (Duration <= 0.0)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Chrono, ELunarConsoleMessageVerbosity::Error, TEXT("IsCooldownComplete failed because Duration must be greater than zero"));
		return false;
	}

	return CurrentTime >= StartTime + Duration;
}

double ULunarFLChrono::GetNextIntervalTime(double CurrentTime, double Interval, double Offset)
{
	if (!FMath::IsFinite(CurrentTime) || !FMath::IsFinite(Interval) || !FMath::IsFinite(Offset))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Chrono, ELunarConsoleMessageVerbosity::Error, TEXT("GetNextIntervalTime failed because one or more values are not finite"));
		return CurrentTime;
	}

	if (Interval <= 0.0)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Chrono, ELunarConsoleMessageVerbosity::Error, TEXT("GetNextIntervalTime failed because Interval must be greater than zero"));
		return CurrentTime;
	}

	const double IntervalIndex = FMath::FloorToDouble((CurrentTime - Offset) / Interval) + 1.0;

	return Offset + IntervalIndex * Interval;
}
