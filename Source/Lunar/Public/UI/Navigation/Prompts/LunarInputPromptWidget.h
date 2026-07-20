// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Navigation/Prompts/LunarInputPromptReceiver.h"
#include "LunarInputPromptWidget.generated.h"

/**
 * @file LunarInputPromptWidget.h
 * @brief Standard base class for owner-authored Lunar input-prompt widgets.
 * @ingroup LunarNavigationPrompts
 */

/**
 * Stores the latest resolved prompt snapshot for completely owner-authored presentation.
 * Lunar does not impose a visual style or require icon and text presentation elements.
 * @ingroup LunarNavigationPrompts
 */
UCLASS(Abstract, Blueprintable)
class LUNAR_API ULunarInputPromptWidget : public UUserWidget, public ILunarInputPromptReceiver
{
	GENERATED_BODY()

public:
	/**
	 * Replaces the stored complete prompt snapshot.
	 * @param Actions Complete ordered prompt snapshot; Blueprint overrides should call the parent implementation.
	 */
	virtual void ApplyResolvedPromptActions_Implementation(
		const TArray<FLunarResolvedPromptAction>& Actions) override;

protected:
	/** Complete ordered snapshot most recently delivered through ApplyResolvedPromptActions. */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Prompts")
	TArray<FLunarResolvedPromptAction> ResolvedPromptActions;
};