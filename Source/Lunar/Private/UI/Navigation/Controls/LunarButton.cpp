// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "UI/Navigation/Controls/LunarButton.h"

#include "UI/Navigation/Types/LunarGameplayTags.h"

/**
 * @file LunarButton.cpp
 * @brief Lunar push-button behavior and semantic activation
 * @ingroup LunarNavigationControls
 */

ULunarButton::ULunarButton(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCanReceiveLunarSelection = true;
	bCanInteractWithPointer = true;
	bEnableInputPrompt = true;

	FLunarPromptActionRequest AcceptPrompt;
	AcceptPrompt.ActionTag = LunarGameplayTags::UI_Action_Accept.GetTag();
	PromptActions.Add(AcceptPrompt);
}

void ULunarButton::Click()
{
	ActivateLunarWidget();
}

bool ULunarButton::NativeCanHandleLunarAction(const FLunarUIActionContext& ActionContext) const
{
	if (ActionContext.ActionTag == LunarGameplayTags::UI_Action_Accept.GetTag())
	{
		return IsLunarSelected();
	}
	return Super::NativeCanHandleLunarAction(ActionContext);
}

ELunarUIActionResult ULunarButton::NativeHandleLunarAction(const FLunarUIActionContext& ActionContext)
{
	if (ActionContext.ActionTag != LunarGameplayTags::UI_Action_Accept.GetTag())
	{
		return Super::NativeHandleLunarAction(ActionContext);
	}

	if (!NativeCanActivateLunarWidget())
	{
		return ELunarUIActionResult::Rejected;
	}

	if (ActionContext.InputEvent == IE_Released)
	{
		if (!IsLunarSelected())
		{
			return ELunarUIActionResult::Handled;
		}
		ActivateLunarWidget();
	}

	return ELunarUIActionResult::Handled;
}

void ULunarButton::NativeOnLunarActivated()
{
	Super::NativeOnLunarActivated();
	BP_OnLunarClicked();
	OnClicked.Broadcast(this);
}
