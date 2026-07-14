// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UI/Navigation/Types/LunarNavigationTypes.h"
#include "LunarUIActionRegistry.generated.h"

/**
 * @file LunarUIActionRegistry.h
 * @brief Data-driven semantic Lunar UI action registry.
 * @ingroup LunarNavigationData
 */

/**
 * @brief Stores built-in and project-defined semantic Lunar UI actions.
 * @ingroup LunarNavigationData
 */
UCLASS(BlueprintType)
class LUNAR_API ULunarUIActionRegistry : public UDataAsset
{
	GENERATED_BODY()

public:
	/** @brief Complete action definitions keyed by semantic Gameplay Tag. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Input")
	TArray<FLunarUIActionDefinition> ActionDefinitions;

	/**
	 * @brief Finds exactly one definition for a semantic action tag.
	 * @param ActionTag Semantic Lunar UI action to find.
	 * @param OutDefinition Receives the unique definition on success.
	 * @return True when exactly one valid definition exists; false for invalid, missing, or duplicate definitions.
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation|Input")
	bool FindActionDefinition(FGameplayTag ActionTag, FLunarUIActionDefinition& OutDefinition) const;

	/**
	 * @brief Validates action tags, definitions, and enabled physical bindings.
	 * @return Actionable configuration messages for every detected registry problem.
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation|Validation")
	TArray<FLunarNavigationValidationMessage> ValidateRegistry() const;
};
