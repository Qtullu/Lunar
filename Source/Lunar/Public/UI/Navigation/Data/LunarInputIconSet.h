// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UI/Navigation/Types/LunarInputPromptTypes.h"
#include "LunarInputIconSet.generated.h"

/**
 * @file LunarInputIconSet.h
 * @brief Device-family input-key to prompt-icon mappings.
 * @ingroup LunarNavigationData
 */

/**
 * @brief Device-family-specific mapping from Unreal input keys to prompt icons.
 * @ingroup LunarNavigationData
 */
UCLASS(BlueprintType)
class LUNAR_API ULunarInputIconSet : public UDataAsset
{
	GENERATED_BODY()

public:
	/** @brief Icon mappings resolved by key. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Prompts")
	TArray<FLunarInputIconEntry> IconEntries;

	/**
	 * @brief Resolves exactly one icon entry for a valid input key.
	 * @param InputKey Unreal input key to resolve.
	 * @param OutIcon Receives the configured icon brush on success.
	 * @return True when exactly one valid mapping exists; false for missing, invalid, or duplicate mappings.
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation|Prompts")
	bool ResolveIconForKey(FKey InputKey, FSlateBrush& OutIcon) const;
};
