// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "UI/Navigation/Controls/LunarComboBoxEmptyVisual.h"

#include "UI/Navigation/Controls/LunarComboBox.h"

void ULunarComboBoxEmptyVisual::InitializeFromComboBox(
	ULunarComboBox* ComboBox,
	const FText& InitialFilterText)
{
	OwningComboBox = ComboBox;
	FilterText = InitialFilterText;
	bHasPublishedFilterText = false;
}

void ULunarComboBoxEmptyVisual::ApplyFilterText(const FText& NewFilterText)
{
	if (bHasPublishedFilterText && FilterText.EqualTo(NewFilterText))
	{
		return;
	}

	const FText PreviousFilterText = FilterText;
	FilterText = NewFilterText;
	bHasPublishedFilterText = true;
	BP_OnEmptyFilterChanged(PreviousFilterText, FilterText);
}
