// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Navigation/Types/LunarNavigationTypes.h"
#include "UI/Navigation/Types/LunarUIStyleTypes.h"
#include "Widgets/SLeafWidget.h"

/**
 * @file SLunarValueControlVisuals.h
 * @brief Native Slate presentations for Lunar option-slider and switch controls
 * @ingroup LunarNavigationControls
 */

/**
 * @brief Native OptionSlider presentation rendered behind arbitrary owner content
 * @ingroup LunarNavigationControls
 */
class SLunarOptionSliderVisual final : public SLeafWidget
{
public:
	/** @cond DOXYGEN_INTERNAL */
	/** Declarative arguments for the stateless option-slider presentation. */
	SLATE_BEGIN_ARGS(SLunarOptionSliderVisual) {}
	SLATE_END_ARGS()
	/** @endcond */

	/**
	 * @brief Initializes the non-interactive native presentation
	 * @param InArgs Declarative Slate arguments
	 */
	void Construct(const FArguments& InArgs);
	/**
	 * @brief Starts, reverses, or immediately applies a specialized style target
	 * @param NewStyle Resolved option-slider style patch
	 */
	void SetStyleTarget(const FLunarOptionSliderStylePatch& NewStyle);
	/**
	 * @brief Updates the selected label and layout orientation
	 * @param NewValueText Localized value label to render
	 * @param NewOrientation Axis used to arrange arrows and label
	 */
	void SetValue(FText NewValueText, EOrientation NewOrientation);

	/**
	 * @brief Calculates desired size from arrow brushes, value text, and font
	 * @param LayoutScaleMultiplier Current Slate layout scale
	 * @return Desired local size
	 */
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	/**
	 * @brief Advances the active style transition
	 * @param AllottedGeometry Current allotted geometry
	 * @param InCurrentTime Current Slate application time
	 * @param InDeltaTime Elapsed frame time in seconds
	 */
	virtual void Tick(const FGeometry& AllottedGeometry, double InCurrentTime, float InDeltaTime) override;
	/**
	 * @brief Paints previous/next arrows and the selected value label
	 * @param Args Slate paint arguments
	 * @param AllottedGeometry Geometry allotted to this widget
	 * @param MyCullingRect Active culling rectangle
	 * @param OutDrawElements Draw-element list to append to
	 * @param LayerId Starting paint layer
	 * @param InWidgetStyle Inherited widget style
	 * @param bParentEnabled Whether the parent hierarchy is enabled
	 * @return Highest layer used by this presentation
	 */
	virtual int32 OnPaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;

private:
	/**
	 * @brief Applies a materialized style without interpolation
	 * @param NewStyle Style snapshot to display immediately
	 */
	void ApplyImmediateStyle(const FLunarOptionSliderStylePatch& NewStyle);

	/** Complete option-slider style currently displayed. */
	FLunarOptionSliderStylePatch DisplayedStyle;
	/** Visual style snapshot at the start of the active transition. */
	FLunarOptionSliderStylePatch TransitionSourceStyle;
	/** Logical source used for discrete fields during a reversing transition. */
	FLunarOptionSliderStylePatch TransitionSourceLogicalStyle;
	/** Materialized destination of the active style transition. */
	FLunarOptionSliderStylePatch TransitionTargetStyle;
	/** Latest logical style target requested by the owner. */
	FLunarOptionSliderStylePatch LogicalTargetStyle;
	/** Localized selected value rendered between arrows. */
	FText ValueText;
	/** Axis used to arrange previous and next arrows. */
	EOrientation Orientation = Orient_Horizontal;
	/** Elapsed time in the active style transition. */
	float TransitionElapsed = 0.0f;
	/** Duration of the active style transition. */
	float TransitionDuration = 0.0f;
	/** Normalized transition alpha most recently used for painting. */
	float TransitionPaintAlpha = 1.0f;
	/** Interpolated text color most recently rendered. */
	mutable FLinearColor LastPaintedValueTextColor = FLinearColor::White;
	/** Logical target text color associated with the last paint. */
	mutable FLinearColor LastPaintedLogicalTargetValueTextColor = FLinearColor::White;
	/** Whether last-painted text color state is valid for reversal continuity. */
	mutable bool bHasLastPaintedValueTextColor = false;
	/** Whether DisplayedStyle contains a valid materialized snapshot. */
	bool bHasDisplayedStyle = false;
	/** Whether a visual-style transition is currently active. */
	bool bTransitionActive = false;
	/** Whether the active transition is reversing toward its previous source. */
	bool bTransitionReversing = false;
};

/**
 * @brief Native Switch presentation rendered behind arbitrary owner content
 * @ingroup LunarNavigationControls
 */
class SLunarSwitchVisual final : public SLeafWidget
{
public:
	/** @cond DOXYGEN_INTERNAL */
	/** Declarative arguments for the stateless switch presentation. */
	SLATE_BEGIN_ARGS(SLunarSwitchVisual) {}
	SLATE_END_ARGS()
	/** @endcond */

	/**
	 * @brief Initializes the non-interactive native presentation
	 * @param InArgs Declarative Slate arguments
	 */
	void Construct(const FArguments& InArgs);
	/**
	 * @brief Starts, reverses, or immediately applies a specialized style target
	 * @param NewStyle Resolved switch style patch
	 */
	void SetStyleTarget(const FLunarSwitchStylePatch& NewStyle);
	/**
	 * @brief Updates logical on/off state and direction mode
	 * @param bNewIsOn New logical switch value
	 * @param NewDirectionMode Axis along which the handle moves
	 */
	void SetValue(bool bNewIsOn, ELunarSwitchDirectionMode NewDirectionMode);

	/**
	 * @brief Calculates desired size from track and handle style values
	 * @param LayoutScaleMultiplier Current Slate layout scale
	 * @return Desired local size
	 */
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	/**
	 * @brief Advances active style and value transitions
	 * @param AllottedGeometry Current allotted geometry
	 * @param InCurrentTime Current Slate application time
	 * @param InDeltaTime Elapsed frame time in seconds
	 */
	virtual void Tick(const FGeometry& AllottedGeometry, double InCurrentTime, float InDeltaTime) override;
	/**
	 * @brief Paints the track and interpolated handle position
	 * @param Args Slate paint arguments
	 * @param AllottedGeometry Geometry allotted to this widget
	 * @param MyCullingRect Active culling rectangle
	 * @param OutDrawElements Draw-element list to append to
	 * @param LayerId Starting paint layer
	 * @param InWidgetStyle Inherited widget style
	 * @param bParentEnabled Whether the parent hierarchy is enabled
	 * @return Highest layer used by this presentation
	 */
	virtual int32 OnPaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;

private:
	/**
	 * @brief Applies a materialized style without interpolation
	 * @param NewStyle Style snapshot to display immediately
	 */
	void ApplyImmediateStyle(const FLunarSwitchStylePatch& NewStyle);
	/**
	 * @brief Applies a normalized handle position without interpolation
	 * @param NewValueAlpha Normalized off-to-on handle alpha
	 */
	void ApplyImmediateValue(float NewValueAlpha);

	/** Complete switch style currently displayed. */
	FLunarSwitchStylePatch DisplayedStyle;
	/** Visual style snapshot at the start of the active transition. */
	FLunarSwitchStylePatch TransitionSourceStyle;
	/** Materialized destination of the active style transition. */
	FLunarSwitchStylePatch TransitionTargetStyle;
	/** Latest logical style target requested by the owner. */
	FLunarSwitchStylePatch LogicalTargetStyle;
	/** Axis along which the switch handle moves. */
	ELunarSwitchDirectionMode DirectionMode = ELunarSwitchDirectionMode::Horizontal;
	/** Elapsed time in the active visual-style transition. */
	float TransitionElapsed = 0.0f;
	/** Duration of the active visual-style transition. */
	float TransitionDuration = 0.0f;
	/** Normalized handle position currently displayed. */
	float DisplayedValueAlpha = 0.0f;
	/** Handle position captured at the start of the active value transition. */
	float ValueTransitionSourceAlpha = 0.0f;
	/** Handle-position destination of the active value transition. */
	float ValueTransitionTargetAlpha = 0.0f;
	/** Latest logical handle-position target requested by the owner. */
	float LogicalValueTargetAlpha = 0.0f;
	/** Elapsed time in the active handle-value transition. */
	float ValueTransitionElapsed = 0.0f;
	/** Duration of the active handle-value transition. */
	float ValueTransitionDuration = 0.0f;
	/** Easing function used by the active handle-value transition. */
	TEnumAsByte<EEasingFunc::Type> ValueTransitionEasing = EEasingFunc::Linear;
	/** Latest logical on/off state supplied by the owner. */
	bool bIsOn = false;
	/** Whether DisplayedStyle contains a valid materialized snapshot. */
	bool bHasDisplayedStyle = false;
	/** Whether DisplayedValueAlpha contains a valid initialized value. */
	bool bHasDisplayedValue = false;
	/** Whether a visual-style transition is currently active. */
	bool bTransitionActive = false;
	/** Whether the visual-style transition is reversing toward its previous source. */
	bool bTransitionReversing = false;
	/** Whether a handle-value transition is currently active. */
	bool bValueTransitionActive = false;
	/** Whether the handle-value transition is reversing toward its previous source. */
	bool bValueTransitionReversing = false;
};
