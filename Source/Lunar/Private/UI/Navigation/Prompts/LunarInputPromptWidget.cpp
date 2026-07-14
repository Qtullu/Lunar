// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "UI/Navigation/Prompts/LunarInputPromptWidget.h"

#include "UI/Navigation/Core/LunarNavigableWidget.h"
#include "UI/Navigation/Styles/LunarInputPromptStyleAsset.h"
#include "UI/Navigation/Styles/LunarStyleResolver.h"
#include "UI/Navigation/Types/LunarGameplayTags.h"

/**
 * @file LunarInputPromptWidget.cpp
 * @brief Implements prompt snapshot storage and typed style resolution.
 * @ingroup LunarNavigationPrompts
 */

/** Log channel for actionable prompt-style resolution errors. */
DEFINE_LOG_CATEGORY_STATIC(LogLunarInputPromptWidget, Log, All);

void ULunarInputPromptWidget::ApplyResolvedPromptActions_Implementation(
	const TArray<FLunarResolvedPromptAction>& Actions)
{
	ResolvedPromptActions = Actions;
	RefreshResolvedPromptStyle();
}

void ULunarInputPromptWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	RefreshResolvedPromptStyle();
}

void ULunarInputPromptWidget::RefreshResolvedPromptStyle()
{
	FLunarUIVisualState VisualState;
	VisualState.ValueStateTag = LunarGameplayTags::UI_State_Value_Normal.GetTag();
	VisualState.InteractionState = ELunarUIInteractionState::NavigationNormal;
	VisualState.InputDevice = ELunarInputDeviceType::Unknown;

	if (!ResolvedPromptActions.IsEmpty())
	{
		const FLunarResolvedPromptAction& PrimaryAction = ResolvedPromptActions[0];
		if (const ULunarNavigableWidget* OwnerWidget = PrimaryAction.OwnerWidget)
		{
			VisualState = OwnerWidget->GetLunarVisualState();
		}
		else
		{
			VisualState.ValueStateTag = PrimaryAction.bEnabled
				? LunarGameplayTags::UI_State_Value_Normal.GetTag()
				: LunarGameplayTags::UI_State_Value_Disabled.GetTag();
			VisualState.InteractionState = PrimaryAction.bSelected
				? ELunarUIInteractionState::NavigationSelected
				: ELunarUIInteractionState::NavigationNormal;
			VisualState.InputDevice = PrimaryAction.InputDevice;
		}
	}

	FString StyleError;
	if (!LunarStyleResolver::ResolveInputPromptStyle(
		StyleAsset.Get(),
		VisualState,
		StyleOverrides,
		ResolvedPromptStyle,
		&StyleError))
	{
		ResolvedPromptStyle = FLunarInputPromptStylePatch();
		if (!StyleError.IsEmpty())
		{
			UE_LOG(LogLunarInputPromptWidget, Error, TEXT("%s: %s"), *GetPathName(), *StyleError);
		}
	}
}
