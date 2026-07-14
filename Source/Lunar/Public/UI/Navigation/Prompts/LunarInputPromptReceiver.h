// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UI/Navigation/Types/LunarInputPromptTypes.h"
#include "LunarInputPromptReceiver.generated.h"

/**
 * @file LunarInputPromptReceiver.h
 * @brief Interface for resolved Lunar input-prompt snapshots.
 * @ingroup LunarNavigationPrompts
 */

/**
 * @brief Unreal interface type for resolved Lunar input-prompt receivers.
 * @ingroup LunarNavigationPrompts
 */
UINTERFACE(BlueprintType)
class LUNAR_API ULunarInputPromptReceiver : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief Presentation contract implemented by every Lunar input-prompt widget class.
 * @ingroup LunarNavigationPrompts
 */
class LUNAR_API ILunarInputPromptReceiver
{
	GENERATED_BODY()

public:
	/**
	 * @brief Applies the complete prompt snapshot in requested presentation order.
	 * @param Actions Resolved actions to present; an empty array clears the presentation.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Lunar|UI|Navigation|Prompts")
	void ApplyResolvedPromptActions(const TArray<FLunarResolvedPromptAction>& Actions);
	/**
	 * @brief Default native implementation of ApplyResolvedPromptActions.
	 * @param Actions Resolved actions to present; an empty array clears the presentation.
	 */
	virtual void ApplyResolvedPromptActions_Implementation(const TArray<FLunarResolvedPromptAction>& Actions);
};
