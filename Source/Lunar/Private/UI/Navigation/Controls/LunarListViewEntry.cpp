// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "UI/Navigation/Controls/LunarListViewEntry.h"

/**
 * @file LunarListViewEntry.cpp
 * @brief Runtime data and state publication for generated ListView entry widgets.
 * @ingroup LunarNavigationControls
 */

/** @brief Private equality helpers for virtualized entry snapshots. */
namespace LunarListViewEntry_Private
{
	/** @param A First data snapshot. @param B Second data snapshot. @return True when all authored fields are equivalent. */
	bool DataEquals(const FLunarListViewItemData& A, const FLunarListViewItemData& B)
	{
		return A.ItemId == B.ItemId
			&& A.DisplayText.EqualTo(B.DisplayText)
			&& A.bEnabled == B.bEnabled
			&& A.bCanReceiveSelectionWhenDisabled == B.bCanReceiveSelectionWhenDisabled
			&& A.DisabledReason.EqualTo(B.DisabledReason)
			&& A.Payload == B.Payload;
	}

	/** @param A First visual snapshot. @param B Second visual snapshot. @return True when every state field is equivalent. */
	bool VisualStateEquals(const FLunarUIVisualState& A, const FLunarUIVisualState& B)
	{
		return A.ValueStateTag == B.ValueStateTag
			&& A.InteractionState == B.InteractionState
			&& A.InputDevice == B.InputDevice
			&& A.bReduceMotion == B.bReduceMotion;
	}
}

void ULunarListViewEntry::InitializeFromListView(
	ULunarListView* ListView,
	const int32 NewItemIndex,
	const FLunarListViewItemData& InitialData,
	const FLunarUIVisualState& InitialState,
	const bool bInitiallyActive,
	const bool bInitiallySelected)
{
	OwningListView = ListView;
	ItemIndex = NewItemIndex;
	ItemData = InitialData;
	VisualState = InitialState;
	bIsActiveItem = bInitiallyActive;
	bIsSelectedItem = bInitiallySelected;
	bHasPublishedData = false;
	bHasPublishedVisualState = false;
}

void ULunarListViewEntry::ApplyDataFromListView(
	const int32 NewItemIndex,
	const FLunarListViewItemData& NewData)
{
	const bool bChanged = ItemIndex != NewItemIndex
		|| !LunarListViewEntry_Private::DataEquals(ItemData, NewData);
	if (bHasPublishedData && !bChanged)
	{
		return;
	}

	const FLunarListViewItemData PreviousData = ItemData;
	ItemIndex = NewItemIndex;
	ItemData = NewData;
	bHasPublishedData = true;
	NativeOnListViewItemDataChanged(PreviousData, ItemData);
	BP_OnListViewItemDataChanged(PreviousData, ItemData);
}

void ULunarListViewEntry::ApplyVisualStateFromListView(
	const FLunarUIVisualState& NewState,
	const bool bNewIsActiveItem,
	const bool bNewIsSelectedItem)
{
	if (bHasPublishedVisualState
		&& bIsActiveItem == bNewIsActiveItem
		&& bIsSelectedItem == bNewIsSelectedItem
		&& LunarListViewEntry_Private::VisualStateEquals(VisualState, NewState))
	{
		return;
	}

	VisualState = NewState;
	bIsActiveItem = bNewIsActiveItem;
	bIsSelectedItem = bNewIsSelectedItem;
	bHasPublishedVisualState = true;
	NativeOnListViewItemVisualStateChanged(VisualState, bIsActiveItem, bIsSelectedItem);
	BP_OnListViewItemVisualStateChanged(VisualState, bIsActiveItem, bIsSelectedItem);
}

void ULunarListViewEntry::NativeOnListViewItemDataChanged(
	const FLunarListViewItemData& /*PreviousData*/,
	const FLunarListViewItemData& /*NewData*/)
{
}

void ULunarListViewEntry::NativeOnListViewItemVisualStateChanged(
	const FLunarUIVisualState& /*NewState*/,
	const bool /*bNewIsActiveItem*/,
	const bool /*bNewIsSelectedItem*/)
{
}
