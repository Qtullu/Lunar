// Copyright 2026 Edgar Frolenkov All rights reserved.


#include "FunctionLibraries/LunarFLString.h"

FString ULunarFLString::FormatPercent(float Percent, int32 FractionalDigits)
{
	const int32 SafeFractionalDigits = FMath::Max(0, FractionalDigits);

	return FString::Printf(
		TEXT("%.*f%%"),
		SafeFractionalDigits,
		Percent
	);
}

FString ULunarFLString::FormatPercent01(float Value01, int32 FractionalDigits)
{
	return FormatPercent(Value01 * 100.0f, FractionalDigits);
}

bool ULunarFLString::IsNotEmpty(const FString& String)
{
	return !String.IsEmpty();
}

bool ULunarFLString::IsNotBlank(const FString& String)
{
	return !String.TrimStartAndEnd().IsEmpty();
}