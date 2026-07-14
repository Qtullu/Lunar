// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarComboBoxOptionItem.cpp
 * @brief Implements the private ComboBox-to-ListView option adapter.
 * @ingroup LunarNavigationControls
 */

#include "UI/Navigation/Controls/LunarComboBoxOptionItem.h"

#include "UI/Navigation/Controls/LunarComboBox.h"

void ULunarComboBoxOptionItem::Initialize(ULunarComboBox* InOwner, const FName InOptionId)
{
	Owner = InOwner;
	OptionId = InOptionId;
}

FName ULunarComboBoxOptionItem::GetItemNavigationId_Implementation() const
{
	return OptionId;
}

bool ULunarComboBoxOptionItem::IsItemNavigationEnabled_Implementation() const
{
	if (const FLunarComboBoxOption* Option = Owner ? Owner->FindOptionById(OptionId) : nullptr)
	{
		return Option->bEnabled;
	}
	return false;
}

bool ULunarComboBoxOptionItem::CanSelectItemWhenDisabled_Implementation() const
{
	if (const FLunarComboBoxOption* Option = Owner ? Owner->FindOptionById(OptionId) : nullptr)
	{
		return Option->bCanReceiveSelectionWhenDisabled;
	}
	return false;
}

FText ULunarComboBoxOptionItem::GetItemDisabledReason_Implementation() const
{
	if (const FLunarComboBoxOption* Option = Owner ? Owner->FindOptionById(OptionId) : nullptr)
	{
		return Option->DisabledReason;
	}
	return FText::GetEmpty();
}
