// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarListViewItem.cpp
 * @brief Implements safe defaults for the ILunarListViewItem data contract.
 * @ingroup LunarNavigationControls
 */

#include "UI/Navigation/Controls/LunarListViewItem.h"

FName ILunarListViewItem::GetItemNavigationId_Implementation() const
{
	return NAME_None;
}

bool ILunarListViewItem::IsItemNavigationEnabled_Implementation() const
{
	return true;
}

bool ILunarListViewItem::CanSelectItemWhenDisabled_Implementation() const
{
	return false;
}

FText ILunarListViewItem::GetItemDisabledReason_Implementation() const
{
	return FText::GetEmpty();
}
