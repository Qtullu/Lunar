// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "UI/Navigation/Controls/LunarComboBoxEntry.h"

#include "UI/Navigation/Controls/LunarComboBox.h"
#include "UI/Navigation/Controls/LunarListView.h"

void ULunarComboBoxEntry::NativePreConstruct()
{
	ResolveOwningComboBox();
	RefreshTypedSnapshot(false);
	Super::NativePreConstruct();
}

void ULunarComboBoxEntry::NativeOnListViewItemDataChanged(
	const FLunarListViewItemData& PreviousData,
	const FLunarListViewItemData& /*NewData*/)
{
	ResolveOwningComboBox();
	RefreshTypedSnapshot(true, &PreviousData);
}

void ULunarComboBoxEntry::NativeOnListViewItemVisualStateChanged(
	const FLunarUIVisualState& NewState,
	const bool bNewIsActiveItem,
	const bool /*bNewIsSelectedItem*/)
{
	ResolveOwningComboBox();
	PublishTypedVisualState(NewState, bNewIsActiveItem);
}

FLunarComboBoxOption ULunarComboBoxEntry::ConvertItemData(
	const FLunarListViewItemData& SourceItemData) const
{
	FLunarComboBoxOption Result;
	Result.OptionId = SourceItemData.ItemId;
	Result.DisplayText = SourceItemData.DisplayText;
	Result.bEnabled = SourceItemData.bEnabled;
	Result.bCanReceiveSelectionWhenDisabled = SourceItemData.bCanReceiveSelectionWhenDisabled;
	Result.DisabledReason = SourceItemData.DisabledReason.ToString();
	Result.Payload = SourceItemData.Payload;
	return Result;
}

void ULunarComboBoxEntry::ResolveOwningComboBox()
{
	if (!IsValid(OwningComboBox))
	{
		OwningComboBox = GetTypedOuter<ULunarComboBox>();
	}
}

void ULunarComboBoxEntry::RefreshTypedSnapshot(
	const bool bPublishDataEvent,
	const FLunarListViewItemData* PreviousData)
{
	const FLunarComboBoxOption PreviousOption = PreviousData
		? ConvertItemData(*PreviousData)
		: OptionData;
	OptionData = ConvertItemData(GetItemData());
	bIsHighlighted = IsActiveItem();
	bIsCommitted = IsValid(OwningComboBox)
		&& OwningComboBox->GetSelectedOptionId() == OptionData.OptionId;
	EntryVisualState = ResolveEntryVisualState(
		bIsHighlighted,
		bIsCommitted,
		OptionData.bEnabled);
	if (bPublishDataEvent)
	{
		BP_OnComboBoxOptionDataChanged(PreviousOption, OptionData);
	}
}

void ULunarComboBoxEntry::PublishTypedVisualState(
	const FLunarUIVisualState& NewState,
	const bool bNewIsHighlighted)
{
	bIsHighlighted = bNewIsHighlighted;
	bIsCommitted = IsValid(OwningComboBox)
		&& OwningComboBox->GetSelectedOptionId() == OptionData.OptionId;
	EntryVisualState = ResolveEntryVisualState(
		bIsHighlighted,
		bIsCommitted,
		OptionData.bEnabled);
	BP_OnComboBoxEntryVisualStateChanged(
		NewState,
		EntryVisualState,
		bIsHighlighted,
		bIsCommitted,
		OptionData.bEnabled);
}

ELunarComboBoxEntryVisualState ULunarComboBoxEntry::ResolveEntryVisualState(
	const bool bHighlighted,
	const bool bCommitted,
	const bool bEnabled)
{
	if (bEnabled)
	{
		if (bHighlighted)
		{
			return bCommitted
				? ELunarComboBoxEntryVisualState::EnabledHighlightedCommitted
				: ELunarComboBoxEntryVisualState::EnabledHighlighted;
		}
		return bCommitted
			? ELunarComboBoxEntryVisualState::EnabledCommitted
			: ELunarComboBoxEntryVisualState::Enabled;
	}

	if (bHighlighted)
	{
		return bCommitted
			? ELunarComboBoxEntryVisualState::DisabledHighlightedCommitted
			: ELunarComboBoxEntryVisualState::DisabledHighlighted;
	}
	return bCommitted
		? ELunarComboBoxEntryVisualState::DisabledCommitted
		: ELunarComboBoxEntryVisualState::Disabled;
}
