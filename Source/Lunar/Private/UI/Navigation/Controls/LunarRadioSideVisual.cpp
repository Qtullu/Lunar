// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "UI/Navigation/Controls/LunarRadioSideVisual.h"

/**
 * @file LunarRadioSideVisual.cpp
 * @brief Runtime data and state publication for generated Lunar Radio side visuals.
 * @ingroup LunarNavigationControls
 */

/** @brief Private equality helpers for Radio side-visual snapshots. */
namespace LunarRadioSideVisual_Private
{
	/** @param A First data snapshot. @param B Second data snapshot. @return True when both fields are equivalent. */
	bool DataEquals(const FLunarRadioSideVisualData& A, const FLunarRadioSideVisualData& B)
	{
		return A.StringValue == B.StringValue && A.DisplayText.EqualTo(B.DisplayText);
	}

	/** @param A First visual snapshot. @param B Second visual snapshot. @return True when every published state field is equivalent. */
	bool VisualStateEquals(const FLunarUIVisualState& A, const FLunarUIVisualState& B)
	{
		return A.ValueStateTag == B.ValueStateTag
			&& A.InteractionState == B.InteractionState
			&& A.InputDevice == B.InputDevice
			&& A.bReduceMotion == B.bReduceMotion;
	}
}

void ULunarRadioSideVisual::InitializeFromRadio(
	ULunarRadio* Radio,
	const int32 NewOptionIndex,
	const FLunarRadioSideVisualData& InitialData,
	const FLunarUIVisualState& InitialState,
	const bool bInitiallyChecked)
{
	OwningRadio = Radio;
	OptionIndex = NewOptionIndex;
	OptionData = InitialData;
	VisualState = InitialState;
	bIsChecked = bInitiallyChecked;
	bHasPublishedVisualState = false;
}

void ULunarRadioSideVisual::ApplyDataFromRadio(const FLunarRadioSideVisualData& NewData)
{
	if (LunarRadioSideVisual_Private::DataEquals(OptionData, NewData))
	{
		return;
	}

	const FLunarRadioSideVisualData PreviousData = OptionData;
	OptionData = NewData;
	BP_OnRadioOptionDataChanged(PreviousData, OptionData);
}

void ULunarRadioSideVisual::ApplyVisualStateFromRadio(
	const FLunarUIVisualState& NewState,
	const bool bNewIsChecked)
{
	if (bHasPublishedVisualState
		&& bIsChecked == bNewIsChecked
		&& LunarRadioSideVisual_Private::VisualStateEquals(VisualState, NewState))
	{
		return;
	}

	VisualState = NewState;
	bIsChecked = bNewIsChecked;
	bHasPublishedVisualState = true;
	BP_OnRadioOptionVisualStateChanged(VisualState, bIsChecked);
}
