// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "UI/Navigation/Controls/SLunarRadioVisual.h"

#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"

/**
 * @file SLunarRadioVisual.cpp
 * @brief Slate layout and shared-indicator rendering for Lunar Radio.
 * @ingroup LunarNavigationControls
 */

void SLunarRadioVisual::Construct(const FArguments& InArgs)
{
	SetCanTick(true);
	SetVisibility(EVisibility::HitTestInvisible);
#if WITH_ACCESSIBILITY
	SetAccessibleBehavior(EAccessibleBehavior::NotAccessible);
#endif

	SAssignNew(RootOverlay, SOverlay);
	ChildSlot
	[
		RootOverlay.ToSharedRef()
	];
	RebuildLayout();
}

void SLunarRadioVisual::Tick(
	const FGeometry& AllottedGeometry,
	const double InCurrentTime,
	const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	RefreshCheckedGeometry();
}

void SLunarRadioVisual::SetItems(
	const int32 NewOptionCount,
	const TArray<TSharedPtr<SWidget>>& NewSideVisualWidgets)
{
	OptionCount = FMath::Max(0, NewOptionCount);
	SideVisualWidgets = NewSideVisualWidgets;
	SideVisualWidgets.SetNum(OptionCount);
	UncheckedStyles.SetNum(OptionCount);
	RebuildLayout();
}

void SLunarRadioVisual::SetLayout(
	const EOrientation NewOrientation,
	const FVector2D& NewButtonSize,
	const float NewButtonSpacing,
	const ELunarRadioSideVisualPlacement NewPlacement,
	const FVector2D& NewSideVisualSpacing)
{
	const FVector2D SafeButtonSize(
		FMath::IsFinite(NewButtonSize.X) ? FMath::Max(0.0f, NewButtonSize.X) : 0.0f,
		FMath::IsFinite(NewButtonSize.Y) ? FMath::Max(0.0f, NewButtonSize.Y) : 0.0f);
	const float SafeButtonSpacing = FMath::IsFinite(NewButtonSpacing)
		? FMath::Max(0.0f, NewButtonSpacing)
		: 0.0f;
	const FVector2D SafeSideVisualSpacing(
		FMath::IsFinite(NewSideVisualSpacing.X) ? FMath::Max(0.0f, NewSideVisualSpacing.X) : 0.0f,
		FMath::IsFinite(NewSideVisualSpacing.Y) ? FMath::Max(0.0f, NewSideVisualSpacing.Y) : 0.0f);

	if (Orientation == NewOrientation
		&& ButtonSize == SafeButtonSize
		&& ButtonSpacing == SafeButtonSpacing
		&& SideVisualPlacement == NewPlacement
		&& SideVisualSpacing == SafeSideVisualSpacing)
	{
		return;
	}

	Orientation = NewOrientation;
	ButtonSize = SafeButtonSize;
	ButtonSpacing = SafeButtonSpacing;
	SideVisualPlacement = NewPlacement;
	SideVisualSpacing = SafeSideVisualSpacing;
	RebuildLayout();
}

void SLunarRadioVisual::SetUncheckedStyle(
	const int32 OptionIndex,
	const FLunarRadioVisualStyle& NewStyle)
{
	if (!UncheckedStyles.IsValidIndex(OptionIndex)
		|| !UncheckedSizeBoxes.IsValidIndex(OptionIndex)
		|| !UncheckedImages.IsValidIndex(OptionIndex))
	{
		return;
	}

	const FLunarRadioVisualStyle SafeStyle = NormalizeStyle(NewStyle);
	UncheckedStyles[OptionIndex] = SafeStyle;
	UncheckedImages[OptionIndex]->SetImage(&UncheckedStyles[OptionIndex].Brush);
	UncheckedImages[OptionIndex]->SetColorAndOpacity(FSlateColor(SafeStyle.Tint));
	UncheckedSizeBoxes[OptionIndex]->SetWidthOverride(SafeStyle.Size.X);
	UncheckedSizeBoxes[OptionIndex]->SetHeightOverride(SafeStyle.Size.Y);
	UncheckedSizeBoxes[OptionIndex]->SetRenderTransform(SafeStyle.Transform.ToSlateRenderTransform());
	UncheckedSizeBoxes[OptionIndex]->SetRenderTransformPivot(FVector2D(0.5f));
}

void SLunarRadioVisual::SetCheckedStyle(const FLunarRadioVisualStyle& NewStyle)
{
	CheckedStyle = NormalizeStyle(NewStyle);
	if (!CheckedSizeBox.IsValid() || !CheckedImage.IsValid())
	{
		return;
	}

	CheckedImage->SetImage(&CheckedStyle.Brush);
	CheckedImage->SetColorAndOpacity(FSlateColor(CheckedStyle.Tint));
	CheckedSizeBox->SetWidthOverride(CheckedStyle.Size.X);
	CheckedSizeBox->SetHeightOverride(CheckedStyle.Size.Y);
	RefreshCheckedGeometry();
}

void SLunarRadioVisual::SetDisplayedSelection(
	const float NewDisplayedSelectionPosition,
	const float NewCheckedOpacity)
{
	DisplayedSelectionPosition = FMath::IsFinite(NewDisplayedSelectionPosition)
		? FMath::Clamp(NewDisplayedSelectionPosition, 0.0f, FMath::Max(0.0f, static_cast<float>(OptionCount - 1)))
		: 0.0f;
	CheckedOpacity = FMath::IsFinite(NewCheckedOpacity)
		? FMath::Clamp(NewCheckedOpacity, 0.0f, 1.0f)
		: 1.0f;
	if (CheckedSizeBox.IsValid())
	{
		CheckedSizeBox->SetRenderOpacity(CheckedOpacity);
	}
	RefreshCheckedGeometry();
}

void SLunarRadioVisual::RefreshCheckedGeometry()
{
	if (!RootOverlay.IsValid()
		|| !CheckedSizeBox.IsValid()
		|| IndicatorAnchorBoxes.IsEmpty())
	{
		return;
	}

	const int32 LowerIndex = FMath::Clamp(FMath::FloorToInt(DisplayedSelectionPosition), 0, IndicatorAnchorBoxes.Num() - 1);
	const int32 UpperIndex = FMath::Clamp(FMath::CeilToInt(DisplayedSelectionPosition), 0, IndicatorAnchorBoxes.Num() - 1);
	if (!IndicatorAnchorBoxes[LowerIndex].IsValid() || !IndicatorAnchorBoxes[UpperIndex].IsValid())
	{
		return;
	}

	const FGeometry& RootGeometry = RootOverlay->GetCachedGeometry();
	const FGeometry& LowerGeometry = IndicatorAnchorBoxes[LowerIndex]->GetCachedGeometry();
	const FGeometry& UpperGeometry = IndicatorAnchorBoxes[UpperIndex]->GetCachedGeometry();
	if (RootGeometry.GetLocalSize().IsNearlyZero()
		|| LowerGeometry.GetLocalSize().IsNearlyZero()
		|| UpperGeometry.GetLocalSize().IsNearlyZero())
	{
		return;
	}

	const FVector2D LowerCenterAbsolute = LowerGeometry.LocalToAbsolute(LowerGeometry.GetLocalSize() * 0.5f);
	const FVector2D UpperCenterAbsolute = UpperGeometry.LocalToAbsolute(UpperGeometry.GetLocalSize() * 0.5f);
	const float Fraction = FMath::Clamp(DisplayedSelectionPosition - static_cast<float>(LowerIndex), 0.0f, 1.0f);
	const FVector2D CenterLocal = RootGeometry.AbsoluteToLocal(FMath::Lerp(LowerCenterAbsolute, UpperCenterAbsolute, Fraction));

	FWidgetTransform CombinedTransform = CheckedStyle.Transform;
	CombinedTransform.Translation += CenterLocal - CheckedStyle.Size * 0.5f;
	CheckedSizeBox->SetRenderTransform(CombinedTransform.ToSlateRenderTransform());
	CheckedSizeBox->SetRenderTransformPivot(FVector2D(0.5f));
}

int32 SLunarRadioVisual::ResolveOptionIndex(const FVector2D& ScreenPosition) const
{
	for (int32 OptionIndex = 0; OptionIndex < OptionHitBoxes.Num(); ++OptionIndex)
	{
		if (OptionHitBoxes[OptionIndex].IsValid()
			&& OptionHitBoxes[OptionIndex]->GetCachedGeometry().IsUnderLocation(ScreenPosition))
		{
			return OptionIndex;
		}
	}
	return INDEX_NONE;
}

void SLunarRadioVisual::RebuildLayout()
{
	if (!RootOverlay.IsValid())
	{
		return;
	}

	RootOverlay->ClearChildren();
	OptionHitBoxes.Reset();
	IndicatorAnchorBoxes.Reset();
	UncheckedSizeBoxes.Reset();
	UncheckedImages.Reset();
	UncheckedStyles.SetNum(OptionCount);

	TSharedRef<SHorizontalBox> HorizontalItems = SNew(SHorizontalBox);
	TSharedRef<SVerticalBox> VerticalItems = SNew(SVerticalBox);
	for (int32 OptionIndex = 0; OptionIndex < OptionCount; ++OptionIndex)
	{
		TSharedPtr<SImage> UncheckedImage;
		TSharedPtr<SBox> UncheckedSizeBox;
		TSharedPtr<SBox> IndicatorAnchor;
		TSharedPtr<SBox> OptionHitBox;
		const FLunarRadioVisualStyle SafeStyle = NormalizeStyle(UncheckedStyles[OptionIndex]);
		UncheckedStyles[OptionIndex] = SafeStyle;

		SAssignNew(UncheckedImage, SImage)
			.Image(&UncheckedStyles[OptionIndex].Brush)
			.ColorAndOpacity(FSlateColor(SafeStyle.Tint));
#if WITH_ACCESSIBILITY
		UncheckedImage->SetAccessibleBehavior(EAccessibleBehavior::NotAccessible);
#endif

		SAssignNew(UncheckedSizeBox, SBox)
			.WidthOverride(SafeStyle.Size.X)
			.HeightOverride(SafeStyle.Size.Y)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				UncheckedImage.ToSharedRef()
			];
		UncheckedSizeBox->SetRenderTransform(SafeStyle.Transform.ToSlateRenderTransform());
		UncheckedSizeBox->SetRenderTransformPivot(FVector2D(0.5f));

		SAssignNew(IndicatorAnchor, SBox)
			.WidthOverride(ButtonSize.X)
			.HeightOverride(ButtonSize.Y)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				UncheckedSizeBox.ToSharedRef()
			];

		const TSharedRef<SGridPanel> OptionGrid = SNew(SGridPanel);
		OptionGrid->AddSlot(1, 1)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				IndicatorAnchor.ToSharedRef()
			];
		AddSideVisualToGrid(
			OptionGrid,
			SideVisualWidgets.IsValidIndex(OptionIndex) ? SideVisualWidgets[OptionIndex] : nullptr);

		SAssignNew(OptionHitBox, SBox)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				OptionGrid
			];

		if (Orientation == Orient_Horizontal)
		{
			HorizontalItems->AddSlot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(OptionIndex > 0 ? FMargin(ButtonSpacing, 0.0f, 0.0f, 0.0f) : FMargin(0.0f))
				[
					OptionHitBox.ToSharedRef()
				];
		}
		else
		{
			VerticalItems->AddSlot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(OptionIndex > 0 ? FMargin(0.0f, ButtonSpacing, 0.0f, 0.0f) : FMargin(0.0f))
				[
					OptionHitBox.ToSharedRef()
				];
		}

		OptionHitBoxes.Add(OptionHitBox);
		IndicatorAnchorBoxes.Add(IndicatorAnchor);
		UncheckedSizeBoxes.Add(UncheckedSizeBox);
		UncheckedImages.Add(UncheckedImage);
	}

	RootOverlay->AddSlot()
	[
		Orientation == Orient_Horizontal
			? StaticCastSharedRef<SWidget>(HorizontalItems)
			: StaticCastSharedRef<SWidget>(VerticalItems)
	];

	SAssignNew(CheckedImage, SImage)
		.Image(&CheckedStyle.Brush)
		.ColorAndOpacity(FSlateColor(CheckedStyle.Tint));
#if WITH_ACCESSIBILITY
	CheckedImage->SetAccessibleBehavior(EAccessibleBehavior::NotAccessible);
#endif

	SAssignNew(CheckedSizeBox, SBox)
		.WidthOverride(CheckedStyle.Size.X)
		.HeightOverride(CheckedStyle.Size.Y)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			CheckedImage.ToSharedRef()
		];
	CheckedSizeBox->SetRenderOpacity(CheckedOpacity);
	CheckedSizeBox->SetRenderTransformPivot(FVector2D(0.5f));

	RootOverlay->AddSlot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
	[
		CheckedSizeBox.ToSharedRef()
	];

	Invalidate(EInvalidateWidgetReason::Layout | EInvalidateWidgetReason::Paint);
	RefreshCheckedGeometry();
}

void SLunarRadioVisual::AddSideVisualToGrid(
	const TSharedRef<SGridPanel>& Grid,
	const TSharedPtr<SWidget>& SideVisual) const
{
	if (!SideVisual.IsValid() || SideVisual == SNullWidget::NullWidget)
	{
		return;
	}

	int32 Column = 1;
	int32 Row = 1;
	switch (SideVisualPlacement)
	{
	case ELunarRadioSideVisualPlacement::TopLeft: Column = 0; Row = 0; break;
	case ELunarRadioSideVisualPlacement::TopCenter: Column = 1; Row = 0; break;
	case ELunarRadioSideVisualPlacement::TopRight: Column = 2; Row = 0; break;
	case ELunarRadioSideVisualPlacement::CenterLeft: Column = 0; Row = 1; break;
	case ELunarRadioSideVisualPlacement::Center: Column = 1; Row = 1; break;
	case ELunarRadioSideVisualPlacement::CenterRight: Column = 2; Row = 1; break;
	case ELunarRadioSideVisualPlacement::BottomLeft: Column = 0; Row = 2; break;
	case ELunarRadioSideVisualPlacement::BottomCenter: Column = 1; Row = 2; break;
	case ELunarRadioSideVisualPlacement::BottomRight: Column = 2; Row = 2; break;
	default: break;
	}

	const FMargin Padding(
		Column == 2 ? SideVisualSpacing.X : 0.0f,
		Row == 2 ? SideVisualSpacing.Y : 0.0f,
		Column == 0 ? SideVisualSpacing.X : 0.0f,
		Row == 0 ? SideVisualSpacing.Y : 0.0f);
	Grid->AddSlot(Column, Row)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(Padding)
	[
		SideVisual.ToSharedRef()
	];
}

FLunarRadioVisualStyle SLunarRadioVisual::NormalizeStyle(const FLunarRadioVisualStyle& Style)
{
	FLunarRadioVisualStyle Result = Style;
	Result.Size.X = FMath::IsFinite(Result.Size.X) ? FMath::Max(0.0f, Result.Size.X) : 0.0f;
	Result.Size.Y = FMath::IsFinite(Result.Size.Y) ? FMath::Max(0.0f, Result.Size.Y) : 0.0f;
	Result.Transform.Translation.X = FMath::IsFinite(Result.Transform.Translation.X) ? Result.Transform.Translation.X : 0.0f;
	Result.Transform.Translation.Y = FMath::IsFinite(Result.Transform.Translation.Y) ? Result.Transform.Translation.Y : 0.0f;
	Result.Transform.Scale.X = FMath::IsFinite(Result.Transform.Scale.X) ? Result.Transform.Scale.X : 1.0f;
	Result.Transform.Scale.Y = FMath::IsFinite(Result.Transform.Scale.Y) ? Result.Transform.Scale.Y : 1.0f;
	Result.Transform.Shear.X = FMath::IsFinite(Result.Transform.Shear.X) ? Result.Transform.Shear.X : 0.0f;
	Result.Transform.Shear.Y = FMath::IsFinite(Result.Transform.Shear.Y) ? Result.Transform.Shear.Y : 0.0f;
	Result.Transform.Angle = FMath::IsFinite(Result.Transform.Angle) ? Result.Transform.Angle : 0.0f;
	return Result;
}
