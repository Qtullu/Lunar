// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "UI/Navigation/Controls/LunarSwitch.h"

#include "UI/Navigation/Controls/SLunarValueControlVisuals.h"
#include "UI/Navigation/Styles/LunarStyleResolver.h"
#include "UI/Navigation/Types/LunarGameplayTags.h"

/**
 * @file LunarSwitch.cpp
 * @brief Binary Lunar switch behavior and style synchronization
 * @ingroup LunarNavigationControls
 */

ULunarSwitch::ULunarSwitch(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCanReceiveLunarSelection = true;
	bCanInteractWithPointer = true;
	bEnableInputPrompt = true;

	FLunarPromptActionRequest AcceptPrompt;
	AcceptPrompt.ActionTag = LunarGameplayTags::UI_Action_Accept.GetTag();
	PromptActions.Add(AcceptPrompt);

	ApplyValueState();
}

void ULunarSwitch::SetIsOn(const bool bNewIsOn)
{
	if (bIsOn == bNewIsOn)
	{
		ApplyValueState();
		return;
	}

	bIsOn = bNewIsOn;
	ApplyValueState();
	OnSwitchChanged.Broadcast(bIsOn);
	NotifyLunarAccessibleValueChanged(NativeGetLunarAccessibleValueText());
}

void ULunarSwitch::Toggle()
{
	SetIsOn(!bIsOn);
}

TSharedPtr<SWidget> ULunarSwitch::RebuildLunarSpecializedPresentation()
{
	SAssignNew(SwitchVisual, SLunarSwitchVisual);
	SynchronizeSpecializedPresentation();
	return SwitchVisual;
}

void ULunarSwitch::ReleaseSlateResources(const bool bReleaseChildren)
{
	SwitchVisual.Reset();
	Super::ReleaseSlateResources(bReleaseChildren);
}

void ULunarSwitch::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	ApplyValueState();
	RefreshLunarAccessibility();
	SynchronizeSpecializedPresentation();
}

bool ULunarSwitch::NativeCanHandleLunarAction(const FLunarUIActionContext& ActionContext) const
{
	using namespace LunarGameplayTags;
	if (ActionContext.ActionTag == UI_Action_Accept.GetTag())
	{
		return IsLunarSelected();
	}

	const bool bValueAction = ActionContext.ActionTag == UI_Action_Increase.GetTag()
		|| ActionContext.ActionTag == UI_Action_Decrease.GetTag();
	if (!bValueAction || DirectionMode == ELunarSwitchDirectionMode::Disabled)
	{
		return Super::NativeCanHandleLunarAction(ActionContext);
	}
	if (!IsLunarSelected())
	{
		return false;
	}

	if (ActionContext.bHasNavigationDirection)
	{
		FGameplayTag ExpectedAction;
		return NativeResolveDirectionalLunarControlAction(ActionContext.NavigationDirection, ExpectedAction)
			&& ExpectedAction == ActionContext.ActionTag;
	}
	return true;
}

ELunarUIActionResult ULunarSwitch::NativeHandleLunarAction(const FLunarUIActionContext& ActionContext)
{
	using namespace LunarGameplayTags;
	if (ActionContext.ActionTag == UI_Action_Accept.GetTag())
	{
		if (!IsLunarSelected())
		{
			return ELunarUIActionResult::Unhandled;
		}
		if (!NativeCanActivateLunarWidget())
		{
			return ELunarUIActionResult::Rejected;
		}
		if (ActionContext.InputEvent == IE_Released)
		{
			ActivateLunarWidget();
		}
		return ELunarUIActionResult::Handled;
	}

	const bool bIncrease = ActionContext.ActionTag == UI_Action_Increase.GetTag();
	const bool bDecrease = ActionContext.ActionTag == UI_Action_Decrease.GetTag();
	if (!bIncrease && !bDecrease)
	{
		return Super::NativeHandleLunarAction(ActionContext);
	}
	if (!NativeCanHandleLunarAction(ActionContext))
	{
		return ELunarUIActionResult::Unhandled;
	}
	if (!NativeCanActivateLunarWidget())
	{
		return ELunarUIActionResult::Rejected;
	}

	if (ActionContext.InputEvent == IE_Pressed || ActionContext.InputEvent == IE_Repeat)
	{
		SetIsOn(bIncrease);
	}
	// Repeating an already active value remains consumed and cannot move Selection.
	return ELunarUIActionResult::Handled;
}

bool ULunarSwitch::NativeResolveDirectionalLunarControlAction(
	const ELunarNavigationDirection Direction,
	FGameplayTag& OutActionTag) const
{
	bool bIncrease = false;
	switch (DirectionMode)
	{
	case ELunarSwitchDirectionMode::Horizontal:
		if (Direction != ELunarNavigationDirection::Left && Direction != ELunarNavigationDirection::Right)
		{
			OutActionTag = FGameplayTag();
			return false;
		}
		bIncrease = Direction == ELunarNavigationDirection::Right;
		break;

	case ELunarSwitchDirectionMode::Vertical:
		if (Direction != ELunarNavigationDirection::Up && Direction != ELunarNavigationDirection::Down)
		{
			OutActionTag = FGameplayTag();
			return false;
		}
		bIncrease = Direction == ELunarNavigationDirection::Up;
		break;

	case ELunarSwitchDirectionMode::Disabled:
	default:
		OutActionTag = FGameplayTag();
		return false;
	}

	OutActionTag = bIncrease
		? LunarGameplayTags::UI_Action_Increase.GetTag()
		: LunarGameplayTags::UI_Action_Decrease.GetTag();
	return true;
}

void ULunarSwitch::NativeOnLunarActivated()
{
	Super::NativeOnLunarActivated();
	Toggle();
}

FText ULunarSwitch::NativeGetLunarAccessibleValueText() const
{
	return bIsOn
		? NSLOCTEXT("LunarSwitch", "AccessibleOn", "On")
		: NSLOCTEXT("LunarSwitch", "AccessibleOff", "Off");
}

bool ULunarSwitch::ResolveCommonStylePatch(FLunarCommonStylePatch& OutStyle, FString& OutError) const
{
	FLunarSwitchStylePatch ResolvedStyle;
	if (!LunarStyleResolver::ResolveSwitchStyle(
		StyleAsset,
		GetLunarVisualState(),
		StyleOverrides,
		ResolvedStyle,
		&OutError))
	{
		return false;
	}
	ULunarSwitch* MutableThis = const_cast<ULunarSwitch*>(this);
	if (!FLunarSwitchStylePatch::StaticStruct()->CompareScriptStruct(
		&MutableThis->ResolvedSwitchStyle,
		&ResolvedStyle,
		0))
	{
		MutableThis->PreviousSwitchStyle = MutableThis->ResolvedSwitchStyle;
		MutableThis->ResolvedSwitchStyle = ResolvedStyle;
	}
	OutStyle = MutableThis->ResolvedSwitchStyle.Common;
	return true;
}

void ULunarSwitch::ApplyResolvedCommonStyle(const FLunarCommonStylePatch& ResolvedStyle)
{
	Super::ApplyResolvedCommonStyle(ResolvedStyle);
	if (SwitchVisual.IsValid())
	{
		FLunarSwitchStylePatch CompleteTarget = ResolvedSwitchStyle;
		CompleteTarget.Common = MaterializeCommonStyleSnapshot(ResolvedSwitchStyle.Common);
		SwitchVisual->SetStyleTarget(CompleteTarget);
	}
}

void ULunarSwitch::ApplyValueState()
{
	SetLunarValueState(bIsOn
		? LunarGameplayTags::UI_State_Value_On.GetTag()
		: LunarGameplayTags::UI_State_Value_Off.GetTag());
	SynchronizeSpecializedPresentation();
}

void ULunarSwitch::SynchronizeSpecializedPresentation()
{
	if (SwitchVisual.IsValid())
	{
		SwitchVisual->SetValue(bIsOn, DirectionMode);
	}
}
