// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LunarFLString.generated.h"

/**
 * @file LunarFLString.h
 * @brief String helper function library
 * @ingroup LunarFLString
 */

 /**
  * @brief Blueprint utility functions for strings
  * @ingroup LunarFLString
  */
UCLASS()
class LUNAR_API ULunarFLString : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * @brief Formats percent value as text
	 * @param Percent Percent value
	 * @param FractionalDigits Number of fractional digits
	 * @return Formatted percent text
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|String|Format", meta = (ClampMin = "0", UIMin = "0"))
	static FString FormatPercent(float Percent, int32 FractionalDigits = 1);

	/**
	 * @brief Formats normalized value as percent text
	 * @param Value01 Normalized value from zero to one
	 * @param FractionalDigits Number of fractional digits
	 * @return Formatted percent text
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|String|Format", meta = (ClampMin = "0", UIMin = "0"))
	static FString FormatPercent01(float Value01, int32 FractionalDigits = 1);

	/**
	 * @brief Checks if string contains any characters
	 * @param String String to check
	 * @return True if string is not empty
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|String", meta = (DisplayName = "Is Not Empty"))
	static bool IsNotEmpty(const FString& String);

	/**
	 * @brief Checks if string contains any non whitespace characters
	 * @param String String to check
	 * @return True if string contains non whitespace characters
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|String", meta = (DisplayName = "Is Not Blank"))
	static bool IsNotBlank(const FString& String);
};