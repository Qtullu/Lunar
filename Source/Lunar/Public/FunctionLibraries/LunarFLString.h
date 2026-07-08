// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LunarFLString.generated.h"

/**
 * @file LunarFLString.h
 * @brief String helper function library
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
	UFUNCTION(BlueprintPure, Category = "Lunar|String|Format", meta = (ClampMin = "0", UIMin = "0"))
	static FString FormatPercent(float Percent, int32 FractionalDigits = 1);

	UFUNCTION(BlueprintPure, Category = "Lunar|String|Format", meta = (ClampMin = "0", UIMin = "0"))
	static FString FormatPercent01(float Value01, int32 FractionalDigits = 1);
};
