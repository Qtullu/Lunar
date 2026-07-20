// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UI/Navigation/Types/LunarUIFeedbackTypes.h"
#include "LunarUIHapticFeedbackAsset.generated.h"

/**
 * @file LunarUIHapticFeedbackAsset.h
 * @brief Declares a reusable Lunar UI haptic-feedback Data Asset.
 * @ingroup LunarNavigationData
 */

/** @brief Reusable complete haptic set assignable to any number of Lunar controls. @ingroup LunarNavigationData */
UCLASS(BlueprintType)
class LUNAR_API ULunarUIHapticFeedbackAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Haptic specifications resolved by per-event Use Data Asset override modes. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Feedback")
	FLunarUIHapticFeedbackSet Haptics;
};
