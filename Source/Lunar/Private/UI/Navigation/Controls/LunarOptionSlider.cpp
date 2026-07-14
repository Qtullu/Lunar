// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "UI/Navigation/Controls/LunarOptionSlider.h"

#include "UI/Navigation/Controls/SLunarValueControlVisuals.h"
#include "UI/Navigation/Styles/LunarStyleResolver.h"
#include "UI/Navigation/Types/LunarGameplayTags.h"

/**
 * @file LunarOptionSlider.cpp
 * @brief Discrete Lunar option-slider behavior and style synchronization
 * @ingroup LunarNavigationControls
 */

ULunarOptionSlider::ULunarOptionSlider(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCanReceiveLunarSelection = true;
	bCanInteractWithPointer = true;
}

void ULunarOptionSlider::SetOptions(const TArray<FText>& NewOptions)
{
	const int32 PreviousIndex = SelectedIndex;
	const FText PreviousOption = GetSelectedOption();
	Options = NewOptions;

	const int32 NewIndex = NormalizeIndex(PreviousIndex);
	SelectedIndex = NewIndex;
	if (PreviousIndex != NewIndex)
	{
		OnSelectedIndexChanged.Broadcast(NewIndex);
	}

	const FText CurrentOption = GetSelectedOption();
	if (!PreviousOption.EqualTo(CurrentOption))
	{
		NotifyLunarAccessibleValueChanged(CurrentOption);
	}
	SynchronizeSpecializedPresentation();
}

void ULunarOptionSlider::SetSelectedIndex(const int32 NewSelectedIndex)
{
	ApplySelectedIndex(NormalizeIndex(NewSelectedIndex), true);
}

FText ULunarOptionSlider::GetSelectedOption() const
{
	return Options.IsValidIndex(SelectedIndex)
		? Options[SelectedIndex]
		: FText::GetEmpty();
}

TSharedPtr<SWidget> ULunarOptionSlider::RebuildLunarSpecializedPresentation()
{
	SAssignNew(OptionSliderVisual, SLunarOptionSliderVisual);
	SynchronizeSpecializedPresentation();
	return OptionSliderVisual;
}

void ULunarOptionSlider::ReleaseSlateResources(const bool bReleaseChildren)
{
	OptionSliderVisual.Reset();
	Super::ReleaseSlateResources(bReleaseChildren);
}

void ULunarOptionSlider::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	ApplySelectedIndex(NormalizeIndex(SelectedIndex), false);
	SetLunarValueState(LunarGameplayTags::UI_State_Value_Normal.GetTag());
	RefreshLunarAccessibility();
	SynchronizeSpecializedPresentation();
}

bool ULunarOptionSlider::NativeCanHandleLunarAction(const FLunarUIActionContext& ActionContext) const
{
	using namespace LunarGameplayTags;
	const bool bValueAction = ActionContext.ActionTag == UI_Action_Increase.GetTag()
		|| ActionContext.ActionTag == UI_Action_Decrease.GetTag();
	if (!bValueAction)
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

ELunarUIActionResult ULunarOptionSlider::NativeHandleLunarAction(const FLunarUIActionContext& ActionContext)
{
	using namespace LunarGameplayTags;
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
		const int32 OptionCount = Options.Num();
		if (OptionCount > 0)
		{
			const int32 DirectionStep = bIncrease ? 1 : -1;
			const int32 CurrentIndex = Options.IsValidIndex(SelectedIndex)
				? SelectedIndex
				: (bIncrease ? -1 : OptionCount);
			const int32 RequestedIndex = CurrentIndex + DirectionStep;
			const int32 NewIndex = bWrapOptions
				? ((RequestedIndex % OptionCount) + OptionCount) % OptionCount
				: FMath::Clamp(RequestedIndex, 0, OptionCount - 1);
			ApplySelectedIndex(NewIndex, true);
		}
	}

	// Boundary attempts remain owned by the value control so Selection never escapes.
	return ELunarUIActionResult::Handled;
}

bool ULunarOptionSlider::NativeResolveDirectionalLunarControlAction(
	const ELunarNavigationDirection Direction,
	FGameplayTag& OutActionTag) const
{
	const bool bHorizontal = Orientation == Orient_Horizontal;
	const bool bValueAxisDirection = bHorizontal
		? (Direction == ELunarNavigationDirection::Left || Direction == ELunarNavigationDirection::Right)
		: (Direction == ELunarNavigationDirection::Up || Direction == ELunarNavigationDirection::Down);
	if (!bValueAxisDirection)
	{
		OutActionTag = FGameplayTag();
		return false;
	}

	bool bIncrease = Direction == ELunarNavigationDirection::Right
		|| Direction == ELunarNavigationDirection::Up;
	if (bInvertValueDirection)
	{
		bIncrease = !bIncrease;
	}
	OutActionTag = bIncrease
		? LunarGameplayTags::UI_Action_Increase.GetTag()
		: LunarGameplayTags::UI_Action_Decrease.GetTag();
	return true;
}

bool ULunarOptionSlider::ResolveCommonStylePatch(FLunarCommonStylePatch& OutStyle, FString& OutError) const
{
	FLunarOptionSliderStylePatch ResolvedStyle;
	if (!LunarStyleResolver::ResolveOptionSliderStyle(
		StyleAsset,
		GetLunarVisualState(),
		StyleOverrides,
		ResolvedStyle,
		&OutError))
	{
		return false;
	}
	ULunarOptionSlider* MutableThis = const_cast<ULunarOptionSlider*>(this);
	if (!FLunarOptionSliderStylePatch::StaticStruct()->CompareScriptStruct(
		&MutableThis->ResolvedOptionSliderStyle,
		&ResolvedStyle,
		0))
	{
		MutableThis->PreviousOptionSliderStyle = MutableThis->ResolvedOptionSliderStyle;
		MutableThis->ResolvedOptionSliderStyle = ResolvedStyle;
	}
	OutStyle = MutableThis->ResolvedOptionSliderStyle.Common;
	return true;
}

void ULunarOptionSlider::ApplyResolvedCommonStyle(const FLunarCommonStylePatch& ResolvedStyle)
{
	Super::ApplyResolvedCommonStyle(ResolvedStyle);
	if (OptionSliderVisual.IsValid())
	{
		FLunarOptionSliderStylePatch CompleteTarget = ResolvedOptionSliderStyle;
		CompleteTarget.Common = MaterializeCommonStyleSnapshot(ResolvedOptionSliderStyle.Common);
		OptionSliderVisual->SetStyleTarget(CompleteTarget);
	}
}

FText ULunarOptionSlider::NativeGetLunarAccessibleValueText() const
{
	return GetSelectedOption();
}

int32 ULunarOptionSlider::NormalizeIndex(const int32 RequestedIndex) const
{
	return Options.IsEmpty()
		? INDEX_NONE
		: FMath::Clamp(RequestedIndex, 0, Options.Num() - 1);
}

void ULunarOptionSlider::ApplySelectedIndex(const int32 NewSelectedIndex, const bool bNotify)
{
	const int32 NormalizedIndex = NormalizeIndex(NewSelectedIndex);
	if (SelectedIndex == NormalizedIndex)
	{
		return;
	}

	SelectedIndex = NormalizedIndex;
	if (bNotify)
	{
		OnSelectedIndexChanged.Broadcast(SelectedIndex);
		NotifyLunarAccessibleValueChanged(GetSelectedOption());
	}
	SynchronizeSpecializedPresentation();
}

void ULunarOptionSlider::SynchronizeSpecializedPresentation()
{
	if (OptionSliderVisual.IsValid())
	{
		OptionSliderVisual->SetValue(GetSelectedOption(), Orientation);
	}
}
