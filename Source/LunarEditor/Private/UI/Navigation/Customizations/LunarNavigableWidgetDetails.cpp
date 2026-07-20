// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarNavigableWidgetDetails.cpp
 * @brief Implements the Details policy for Lunar navigation widgets and containers.
 * @ingroup LunarNavigationEditor
 */

#include "UI/Navigation/Customizations/LunarNavigableWidgetDetails.h"

#include "Blueprint/UserWidget.h"
#include "Components/ScrollBox.h"
#include "Components/Widget.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IPropertyUtilities.h"
#include "Styling/AppStyle.h"
#include "UI/Navigation/Validation/LunarNativeNavigationValidation.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "LunarNavigableWidgetDetails"

TSharedRef<IDetailCustomization> FLunarNavigableWidgetDetails::MakeInstance()
{
	return MakeShared<FLunarNavigableWidgetDetails>();
}

void FLunarNavigableWidgetDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	CustomizedWidgets.Reset();
	PropertyUtilities = DetailBuilder.GetPropertyUtilities();

	TArray<TWeakObjectPtr<UObject>> CustomizedObjects;
	DetailBuilder.GetObjectsBeingCustomized(CustomizedObjects);

	for (const TWeakObjectPtr<UObject>& WeakObject : CustomizedObjects)
	{
		if (UWidget* Widget = Cast<UWidget>(WeakObject.Get()))
		{
			CustomizedWidgets.Add(Widget);
		}
	}

	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UWidget, Navigation), UWidget::StaticClass());
	DetailBuilder.HideProperty(TEXT("bIsEnabled"), UWidget::StaticClass());
	DetailBuilder.HideProperty(TEXT("bIsFocusable"), UUserWidget::StaticClass());
	DetailBuilder.HideProperty(TEXT("bIsFocusable"), UScrollBox::StaticClass());

	IDetailCategoryBuilder& LunarCategory = DetailBuilder.EditCategory(
		TEXT("LunarNavigationEditor"),
		LOCTEXT("LunarNavigationCategory", "Lunar Navigation"),
		ECategoryPriority::Important);

	LunarCategory.AddCustomRow(LOCTEXT("ManagedNativeNavigationSearch", "Native UMG Navigation Focus Enabled Lunar"))
		.WholeRowContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Top)
			.Padding(0.0f, 1.0f, 6.0f, 0.0f)
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("Icons.Info"))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT(
					"ManagedNativeNavigationMessage",
					"Native UMG navigation, focusability, and enabled presentation are managed internally by Lunar. Use Lunar Enabled for logical availability, style Disabled through Lunar value and interaction states, and configure Lunar links and selection eligibility on selectable controls."))
				.AutoWrapText(true)
			]
		];

	LunarCategory.AddCustomRow(LOCTEXT("NativeNavigationErrorSearch", "Native UMG Navigation Error Clear Fix"))
		.Visibility(TAttribute<EVisibility>::Create(
			TAttribute<EVisibility>::FGetter::CreateSP(this, &FLunarNavigableWidgetDetails::GetNativeNavigationErrorVisibility)))
		.WholeRowContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Top)
			.Padding(0.0f, 2.0f, 6.0f, 0.0f)
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("Icons.Error"))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT(
					"NativeNavigationErrorMessage",
					"Error: this Lunar widget contains native UMG navigation data. Lunar ignores it; clear it to keep the asset deterministic."))
				.AutoWrapText(true)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(8.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("ClearNativeNavigationButton", "Clear Native Navigation"))
				.ToolTipText(LOCTEXT(
					"ClearNativeNavigationTooltip",
					"Clears the inherited UWidget Navigation object from the selected Lunar widget and its Blueprint template."))
				.OnClicked(this, &FLunarNavigableWidgetDetails::ClearNativeNavigation)
			]
		];
}

bool FLunarNavigableWidgetDetails::HasNonDefaultNativeNavigation() const
{
	return LunarNativeNavigationValidation::HasNonDefaultNavigation(CustomizedWidgets);
}

EVisibility FLunarNavigableWidgetDetails::GetNativeNavigationErrorVisibility() const
{
	return HasNonDefaultNativeNavigation() ? EVisibility::Visible : EVisibility::Collapsed;
}

FReply FLunarNavigableWidgetDetails::ClearNativeNavigation()
{
	if (LunarNativeNavigationValidation::ClearNonDefaultNavigation(CustomizedWidgets))
	{
		if (const TSharedPtr<IPropertyUtilities> Utilities = PropertyUtilities.Pin())
		{
			Utilities->RequestForceRefresh();
		}
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
