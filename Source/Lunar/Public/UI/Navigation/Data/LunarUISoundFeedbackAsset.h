// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UI/Navigation/Types/LunarUIFeedbackTypes.h"
#include "LunarUISoundFeedbackAsset.generated.h"

/**
 * @file LunarUISoundFeedbackAsset.h
 * @brief Declares a reusable Lunar UI sound-feedback Data Asset.
 * @ingroup LunarNavigationData
 */

/** @brief Reusable complete sound set assignable to any number of Lunar controls. @ingroup LunarNavigationData */
UCLASS(BlueprintType)
class LUNAR_API ULunarUISoundFeedbackAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Sound specifications resolved by per-event Use Data Asset override modes. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Feedback")
	FLunarUISoundFeedbackSet Sounds;
};
