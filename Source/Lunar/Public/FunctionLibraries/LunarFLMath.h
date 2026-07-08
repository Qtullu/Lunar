// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LunarFLMath.generated.h"

/**
 * @file LunarFLMath.h
 * @brief Math helper function library
 * @ingroup LunarFLMath
 */

 /**
  * @brief Blueprint utility functions for math
  * @ingroup LunarFLMath
  */
UCLASS()
class LUNAR_API ULunarFLMath : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * @brief Returns float value with inverted sign
	 * @param Value Source float value
	 * @return Negated float value
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Math", meta = (DisplayName = "Negate"))
	static float NegateFloat(float Value);

	/**
	 * @brief Returns integer value with inverted sign
	 * @param Value Source integer value
	 * @return Negated integer value
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Math", meta = (DisplayName = "Negate"))
	static int32 NegateInt(int32 Value);
};