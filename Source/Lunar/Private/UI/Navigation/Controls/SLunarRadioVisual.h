// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Navigation/Controls/LunarRadio.h"
#include "Widgets/SCompoundWidget.h"

class SBox;
class SGridPanel;
class SImage;
class SOverlay;

/**
 * @file SLunarRadioVisual.h
 * @brief Native generated-option presentation for the composite Lunar Radio control.
 * @ingroup LunarNavigationControls
 */

/**
 * @brief Slate presentation that arranges Radio options and renders one shared Checked indicator.
 * @ingroup LunarNavigationControls
 */
class SLunarRadioVisual final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLunarRadioVisual) {}
	SLATE_END_ARGS()

	/** @param InArgs Declarative Slate arguments. @brief Builds an initially empty hit-test-invisible Radio presentation. */
	void Construct(const FArguments& InArgs);

	/** @param AllottedGeometry Current arranged geometry. @param InCurrentTime Current Slate time. @param InDeltaTime Seconds since the previous Slate tick. @brief Repositions the shared Checked indicator after layout in runtime and UMG Designer preview. */
	virtual void Tick(const FGeometry& AllottedGeometry, double InCurrentTime, float InDeltaTime) override;

	/** @param NewOptionCount Number of generated options. @param NewSideVisualWidgets Optional side presentation for each option. @brief Rebuilds option layout children without changing logical selection. */
	void SetItems(int32 NewOptionCount, const TArray<TSharedPtr<SWidget>>& NewSideVisualWidgets);

	/** @param NewOrientation Axis used to arrange options. @param NewButtonSize Logical indicator hit/layout size. @param NewButtonSpacing Gap between adjacent options. @param NewPlacement Side-visual placement around each indicator. @param NewSideVisualSpacing Horizontal and vertical side-visual gaps. @brief Applies all native Radio layout fields. */
	void SetLayout(
		EOrientation NewOrientation,
		const FVector2D& NewButtonSize,
		float NewButtonSpacing,
		ELunarRadioSideVisualPlacement NewPlacement,
		const FVector2D& NewSideVisualSpacing);

	/** @param OptionIndex Option whose Unchecked visual is updated. @param NewStyle Resolved state-specific style. @brief Applies one generated Unchecked presentation. */
	void SetUncheckedStyle(int32 OptionIndex, const FLunarRadioVisualStyle& NewStyle);

	/** @param NewStyle Resolved style for the shared Checked indicator. @brief Applies Checked brush, tint, size, and authored transform. */
	void SetCheckedStyle(const FLunarRadioVisualStyle& NewStyle);

	/** @param NewDisplayedSelectionPosition Fractional option coordinate rendered by the shared indicator. @param NewCheckedOpacity Opacity used by edge-wrap fade transitions. @brief Applies the current presentation-only selection animation state. */
	void SetDisplayedSelection(float NewDisplayedSelectionPosition, float NewCheckedOpacity);

	/** @brief Recalculates the Checked transform from the latest arranged option geometry. */
	void RefreshCheckedGeometry();

	/** @param ScreenPosition Absolute Slate pointer coordinate. @return Zero-based option under the pointer, or INDEX_NONE. */
	int32 ResolveOptionIndex(const FVector2D& ScreenPosition) const;

private:
	/** @brief Rebuilds the orientation box, option grids, indicator anchors, and shared Checked host. */
	void RebuildLayout();

	/** @param Grid Target option grid. @param SideVisual Optional side visual to add. @brief Adds a side visual at the configured three-by-three placement. */
	void AddSideVisualToGrid(const TSharedRef<SGridPanel>& Grid, const TSharedPtr<SWidget>& SideVisual) const;

	/** @param Style Style to normalize. @return Copy with non-negative finite size and transform values suitable for Slate. */
	static FLunarRadioVisualStyle NormalizeStyle(const FLunarRadioVisualStyle& Style);

	/** Root overlay containing generated option layout and the shared Checked host. */
	TSharedPtr<SOverlay> RootOverlay;

	/** Number of generated Radio options represented by this Slate widget. */
	int32 OptionCount = 0;

	/** Optional side presentation widgets aligned one-to-one with generated options. */
	TArray<TSharedPtr<SWidget>> SideVisualWidgets;

	/** Cached hit/layout boxes spanning indicator, side visual, and their internal gap. */
	TArray<TSharedPtr<SBox>> OptionHitBoxes;

	/** Cached fixed-size indicator anchors whose centers define Checked positions. */
	TArray<TSharedPtr<SBox>> IndicatorAnchorBoxes;

	/** Cached size boxes that own each option's Unchecked brush and transform. */
	TArray<TSharedPtr<SBox>> UncheckedSizeBoxes;

	/** Cached images that render each option's state-specific Unchecked brush. */
	TArray<TSharedPtr<SImage>> UncheckedImages;

	/** Last resolved Unchecked style for every generated option. */
	TArray<FLunarRadioVisualStyle> UncheckedStyles;

	/** Size and transform host for the one shared Checked image. */
	TSharedPtr<SBox> CheckedSizeBox;

	/** Image that renders the one shared Checked brush. */
	TSharedPtr<SImage> CheckedImage;

	/** Last resolved state-specific style for the shared Checked indicator. */
	FLunarRadioVisualStyle CheckedStyle;

	/** Axis used to arrange generated options. */
	EOrientation Orientation = Orient_Vertical;

	/** Fixed logical indicator region reserved inside every option. */
	FVector2D ButtonSize = FVector2D(24.0f);

	/** Gap inserted between adjacent option hit/layout boxes. */
	float ButtonSpacing = 8.0f;

	/** Three-by-three placement of each optional side visual around its indicator. */
	ELunarRadioSideVisualPlacement SideVisualPlacement = ELunarRadioSideVisualPlacement::CenterRight;

	/** Independent horizontal and vertical gaps between indicator and side visual. */
	FVector2D SideVisualSpacing = FVector2D(8.0f, 0.0f);

	/** Fractional option coordinate currently rendered by the Checked indicator. */
	float DisplayedSelectionPosition = 0.0f;

	/** Current presentation-only Checked opacity used by edge-wrap fades. */
	float CheckedOpacity = 1.0f;
};
