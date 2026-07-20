// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Slate/WidgetTransform.h"
#include "UI/Navigation/Types/LunarNavigationTypes.h"
#include "Widgets/SLeafWidget.h"

/**
 * @file SLunarValueControlVisuals.h
 * @brief Native Slate presentations for Lunar option-slider and switch controls.
 * @ingroup LunarNavigationControls
 */

/** @brief Native OptionSlider parts rendered behind arbitrary owner content. */
class SLunarOptionSliderVisual final : public SLeafWidget
{
public:
	/** @cond DOXYGEN_INTERNAL */
	SLATE_BEGIN_ARGS(SLunarOptionSliderVisual) {}
	SLATE_END_ARGS()
	/** @endcond */

	/** @param InArgs Declarative Slate arguments. */
	void Construct(const FArguments& InArgs);
	/** @param InPreviousArrowBrush Previous-arrow brush. @param InPreviousArrowTint Previous-arrow tint. @param InPreviousArrowTransform Previous-arrow render transform. @param InNextArrowBrush Next-arrow brush. @param InNextArrowTint Next-arrow tint. @param InNextArrowTransform Next-arrow render transform. @param InValueTextColor Value-label color. @param InValueFont Value-label font. */
	void SetPresentation(
		const FSlateBrush& InPreviousArrowBrush,
		const FLinearColor& InPreviousArrowTint,
		const FWidgetTransform& InPreviousArrowTransform,
		const FSlateBrush& InNextArrowBrush,
		const FLinearColor& InNextArrowTint,
		const FWidgetTransform& InNextArrowTransform,
		const FSlateColor& InValueTextColor,
		const FSlateFontInfo& InValueFont);
	/** @param NewValueText Localized value label. @param NewOrientation Layout axis. */
	void SetValue(FText NewValueText, EOrientation NewOrientation);
	/** @param LayoutScaleMultiplier Current Slate scale. @return Desired size. */
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	/** @return Highest paint layer used. */
	virtual int32 OnPaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;

private:
	FSlateBrush PreviousArrowBrush; ///< Previous-arrow brush.
	FLinearColor PreviousArrowTint = FLinearColor::White; ///< Previous-arrow tint.
	FWidgetTransform PreviousArrowTransform; ///< Previous-arrow render transform.
	FSlateBrush NextArrowBrush; ///< Next-arrow brush.
	FLinearColor NextArrowTint = FLinearColor::White; ///< Next-arrow tint.
	FWidgetTransform NextArrowTransform; ///< Next-arrow render transform.
	FSlateColor ValueTextColor = FSlateColor::UseForeground(); ///< Value-label color.
	FSlateFontInfo ValueFont; ///< Value-label font.
	FText ValueText; ///< Current value label.
	EOrientation Orientation = Orient_Horizontal; ///< Current layout axis.
};

/** @brief Native Switch track and handle rendered behind arbitrary owner content. */
class SLunarSwitchVisual final : public SLeafWidget
{
public:
	/** @cond DOXYGEN_INTERNAL */
	SLATE_BEGIN_ARGS(SLunarSwitchVisual) {}
	SLATE_END_ARGS()
	/** @endcond */

	/** @param InArgs Declarative Slate arguments. */
	void Construct(const FArguments& InArgs);
	/** @param InTrackBrush Track brush. @param InTrackTint Track tint. @param InTrackSize Track size. @param InHandleBrush Handle brush. @param InHandleTint Handle tint. @param InHandleSize Handle size. @param InHandleOffset Handle offset. */
	void SetPresentation(
		const FSlateBrush& InTrackBrush,
		const FLinearColor& InTrackTint,
		const FVector2D& InTrackSize,
		const FSlateBrush& InHandleBrush,
		const FLinearColor& InHandleTint,
		const FVector2D& InHandleSize,
		const FVector2D& InHandleOffset);
	/** @param NewHandleAlpha Normalized 0..1 rendered Handle position. @param NewDirectionMode Handle axis. */
	void SetHandleAlpha(float NewHandleAlpha, ELunarSwitchDirectionMode NewDirectionMode);
	/** @param LayoutScaleMultiplier Current Slate scale. @return Desired size. */
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	/** @return Highest paint layer used. */
	virtual int32 OnPaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;

private:
	FSlateBrush TrackBrush; ///< Track brush.
	FLinearColor TrackTint = FLinearColor(0.35f, 0.35f, 0.35f, 1.0f); ///< Track tint.
	FVector2D TrackSize = FVector2D(48.0f, 24.0f); ///< Track size.
	FSlateBrush HandleBrush; ///< Handle brush.
	FLinearColor HandleTint = FLinearColor(0.8f, 0.8f, 0.8f, 1.0f); ///< Handle tint.
	FVector2D HandleSize = FVector2D(20.0f, 20.0f); ///< Handle size.
	FVector2D HandleOffset = FVector2D::ZeroVector; ///< Handle offset.
	ELunarSwitchDirectionMode DirectionMode = ELunarSwitchDirectionMode::Horizontal; ///< Handle axis.
	float HandleAlpha = 0.0f; ///< Current normalized rendered Handle position.
};