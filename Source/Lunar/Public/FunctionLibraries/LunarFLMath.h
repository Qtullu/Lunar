// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LunarFLMath.generated.h"

/**
 * 
 */
UCLASS()
class LUNAR_API ULunarFLMath : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, Category = "Lunar|Math", meta = (DisplayName = "Negate"))
	static float NegateFloat(float Value);

	UFUNCTION(BlueprintPure, Category = "Lunar|Math", meta = (DisplayName = "Negate"))
	static int32 NegateInt(int32 Value);
};
