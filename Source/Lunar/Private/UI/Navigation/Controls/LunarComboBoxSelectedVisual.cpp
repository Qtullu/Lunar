// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "UI/Navigation/Controls/LunarComboBoxSelectedVisual.h"

#include "UI/Navigation/Controls/LunarComboBox.h"

namespace LunarComboBoxSelectedVisual_Private
{
	bool OptionEquals(const FLunarComboBoxOption& A, const FLunarComboBoxOption& B)
	{
		return A.OptionId == B.OptionId
			&& A.DisplayText.EqualTo(B.DisplayText)
			&& A.bEnabled == B.bEnabled
			&& A.bCanReceiveSelectionWhenDisabled == B.bCanReceiveSelectionWhenDisabled
			&& A.DisabledReason == B.DisabledReason
			&& A.Payload == B.Payload;
	}

	bool VisualStateEquals(const FLunarUIVisualState& A, const FLunarUIVisualState& B)
	{
		return A.ValueStateTag == B.ValueStateTag
			&& A.InteractionState == B.InteractionState
			&& A.InputDevice == B.InputDevice
			&& A.bReduceMotion == B.bReduceMotion;
	}
}

void ULunarComboBoxSelectedVisual::InitializeFromComboBox(
	ULunarComboBox* ComboBox,
	const FLunarComboBoxOption& InitialOption,
	const bool bInitiallyEmpty,
	const FLunarUIVisualState& InitialState)
{
	OwningComboBox = ComboBox;
	SelectedOption = InitialOption;
	bIsEmpty = bInitiallyEmpty;
	VisualState = InitialState;
	bHasPublishedOption = false;
	bHasPublishedVisualState = false;
}

void ULunarComboBoxSelectedVisual::ApplySelectedOption(
	const FLunarComboBoxOption& NewOption,
	const bool bNewIsEmpty)
{
	if (bHasPublishedOption
		&& bIsEmpty == bNewIsEmpty
		&& LunarComboBoxSelectedVisual_Private::OptionEquals(SelectedOption, NewOption))
	{
		return;
	}

	const FLunarComboBoxOption PreviousOption = SelectedOption;
	SelectedOption = NewOption;
	bIsEmpty = bNewIsEmpty;
	bHasPublishedOption = true;
	BP_OnSelectedOptionChanged(PreviousOption, SelectedOption, bIsEmpty);
}

void ULunarComboBoxSelectedVisual::ApplyVisualState(const FLunarUIVisualState& NewState)
{
	if (bHasPublishedVisualState
		&& LunarComboBoxSelectedVisual_Private::VisualStateEquals(VisualState, NewState))
	{
		return;
	}

	VisualState = NewState;
	bHasPublishedVisualState = true;
	BP_OnComboBoxVisualStateChanged(VisualState);
}
