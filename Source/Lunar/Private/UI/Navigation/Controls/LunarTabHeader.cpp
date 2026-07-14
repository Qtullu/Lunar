// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarTabHeader.cpp
 * @brief Implements owner-routed navigation, activation, and presentation for generated tab headers.
 * @ingroup LunarNavigationControls
 */

#include "UI/Navigation/Controls/LunarTabHeader.h"

#include "UI/Navigation/Controls/LunarTabs.h"
#include "UI/Navigation/Styles/LunarStyleResolver.h"
#include "UI/Navigation/Types/LunarGameplayTags.h"

#define LOCTEXT_NAMESPACE "LunarTabHeader"

/** Private helpers for translating TabHeader action tags. */
namespace LunarTabHeader_Private
{
	/** Maps a canonical navigation action tag to its direction. @param ActionTag Canonical action tag. @param OutDirection Resolved direction. @return True when the tag is directional. */
	bool ActionToDirection(const FGameplayTag& ActionTag, ELunarNavigationDirection& OutDirection)
	{
		using namespace LunarGameplayTags;
		if (ActionTag == UI_Action_Navigate_Up.GetTag())
		{
			OutDirection = ELunarNavigationDirection::Up;
			return true;
		}
		if (ActionTag == UI_Action_Navigate_Down.GetTag())
		{
			OutDirection = ELunarNavigationDirection::Down;
			return true;
		}
		if (ActionTag == UI_Action_Navigate_Left.GetTag())
		{
			OutDirection = ELunarNavigationDirection::Left;
			return true;
		}
		if (ActionTag == UI_Action_Navigate_Right.GetTag())
		{
			OutDirection = ELunarNavigationDirection::Right;
			return true;
		}
		return false;
	}
}

ULunarTabHeader::ULunarTabHeader(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCanReceiveLunarSelection = true;
	bCanInteractWithPointer = true;
	bEnableInputPrompt = true;

	FLunarPromptActionRequest AcceptPrompt;
	AcceptPrompt.ActionTag = LunarGameplayTags::UI_Action_Accept.GetTag();
	PromptActions.Add(AcceptPrompt);
}

bool ULunarTabHeader::NativeCanHandleLunarAction(const FLunarUIActionContext& ActionContext) const
{
	if (!IsLunarSelected())
	{
		return Super::NativeCanHandleLunarAction(ActionContext);
	}

	if (ActionContext.ActionTag == LunarGameplayTags::UI_Action_Accept.GetTag())
	{
		return true;
	}

	ELunarNavigationDirection Direction = ELunarNavigationDirection::Up;
	if (LunarTabHeader_Private::ActionToDirection(ActionContext.ActionTag, Direction))
	{
		// Authored links always retain authority. Automatic strip routing is claimed
		// only when the Tabs owner has a concrete internal destination.
		return GetNavigationLink(Direction).Mode == ELunarNavigationLinkMode::Automatic
			&& TabsOwner
			&& TabsOwner->CanRouteHeaderDirection(this, Direction);
	}

	return Super::NativeCanHandleLunarAction(ActionContext);
}

ELunarUIActionResult ULunarTabHeader::NativeHandleLunarAction(const FLunarUIActionContext& ActionContext)
{
	if (ActionContext.ActionTag == LunarGameplayTags::UI_Action_Accept.GetTag())
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
			if (!TryActivateOwnedTab())
			{
				return ELunarUIActionResult::Rejected;
			}
			Super::NativeOnLunarActivated();
		}
		return ELunarUIActionResult::Handled;
	}

	ELunarNavigationDirection Direction = ELunarNavigationDirection::Up;
	if (LunarTabHeader_Private::ActionToDirection(ActionContext.ActionTag, Direction))
	{
		if (GetNavigationLink(Direction).Mode != ELunarNavigationLinkMode::Automatic
			|| !TabsOwner
			|| !IsLunarSelected())
		{
			return ELunarUIActionResult::Unhandled;
		}
		if (ActionContext.InputEvent == IE_Released)
		{
			return ELunarUIActionResult::Handled;
		}
		return TabsOwner->QueueHeaderDirection(this, Direction)
			? ELunarUIActionResult::Handled
			: ELunarUIActionResult::Unhandled;
	}

	return Super::NativeHandleLunarAction(ActionContext);
}

bool ULunarTabHeader::NativeCanActivateLunarWidget() const
{
	return Super::NativeCanActivateLunarWidget()
		&& TabsOwner
		&& !TabId.IsNone()
		&& TabsOwner->IsTabEnabled(TabId);
}

void ULunarTabHeader::NativeOnLunarActivated()
{
	if (TryActivateOwnedTab())
	{
		Super::NativeOnLunarActivated();
	}
	else
	{
		NativeOnLunarRejected();
	}
}

FText ULunarTabHeader::NativeGetLunarAccessibleValueText() const
{
	return bActiveTabHeader
		? LOCTEXT("ActiveTabAccessibleValue", "Active tab")
		: LOCTEXT("InactiveTabAccessibleValue", "Inactive tab");
}

bool ULunarTabHeader::ResolveCommonStylePatch(FLunarCommonStylePatch& OutStyle, FString& OutError) const
{
	OutStyle = FLunarCommonStylePatch();
	if (TabsOwner)
	{
		const FLunarCommonStylePatch EmptyHeaderOverrides;
		if (!TabsOwner->ResolveHeaderStyle(
			GetLunarVisualState(),
			StyleAsset ? EmptyHeaderOverrides : StyleOverrides,
			OutStyle,
			nullptr,
			OutError))
		{
			return false;
		}
	}

	if (StyleAsset)
	{
		FLunarCommonStylePatch AssignedHeaderStyle;
		if (!LunarStyleResolver::ResolveButtonCommonStyle(
			StyleAsset,
			GetLunarVisualState(),
			StyleOverrides,
			AssignedHeaderStyle,
			&OutError))
		{
			return false;
		}
		LunarStyleResolver::MergeCommonStylePatch(OutStyle, AssignedHeaderStyle);
	}
	else if (!TabsOwner)
	{
		LunarStyleResolver::MergeCommonStylePatch(OutStyle, StyleOverrides);
	}
	return true;
}

void ULunarTabHeader::ApplyResolvedCommonStyle(const FLunarCommonStylePatch& ResolvedStyle)
{
	Super::ApplyResolvedCommonStyle(ResolvedStyle);
	if (TabsOwner)
	{
		// This path runs for every style resolution, including unchanged visual state
		// and asset reset, so the Tabs-owned indicator cannot retain stale visuals.
		TabsOwner->ApplyHeaderPresentation(this, GetLunarVisualState());
	}
}

void ULunarTabHeader::InitializeTabHeader(
	ULunarTabs* InTabsOwner,
	const FName InTabId,
	const bool bInEnabled,
	const FText& InDisabledReason)
{
	TabsOwner = InTabsOwner;
	TabId = InTabId;
	DisabledReason = InDisabledReason;
	SetIsEnabled(bInEnabled);
	SetActiveTabHeader(false);
	RefreshVisualState();
	RefreshLunarAccessibility();
}

void ULunarTabHeader::SetActiveTabHeader(const bool bInActive)
{
	const bool bChanged = bActiveTabHeader != bInActive;
	bActiveTabHeader = bInActive;
	SetLunarValueState(bActiveTabHeader
		? LunarGameplayTags::UI_State_Value_Active.GetTag()
		: LunarGameplayTags::UI_State_Value_Inactive.GetTag());
	if (bChanged)
	{
		NotifyLunarAccessibleValueChanged(NativeGetLunarAccessibleValueText());
	}
}

bool ULunarTabHeader::TryActivateOwnedTab()
{
	return NativeCanActivateLunarWidget()
		&& TabsOwner->TryActivateTabFromHeader(this);
}

void ULunarTabHeader::NotifyOwnedTabRejected()
{
	NativeOnLunarRejected();
}

#undef LOCTEXT_NAMESPACE
