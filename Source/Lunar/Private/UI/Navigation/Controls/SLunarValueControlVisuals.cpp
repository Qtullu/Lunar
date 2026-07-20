// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "UI/Navigation/Controls/SLunarValueControlVisuals.h"

#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/DrawElements.h"
#include "Rendering/SlateRenderer.h"
#include "Styling/CoreStyle.h"

/**
 * @file SLunarValueControlVisuals.cpp
 * @brief Native rendering for Lunar option-slider and switch parts.
 * @ingroup LunarNavigationControls
 */

namespace LunarValueControlVisuals_Private
{
	/** @param Brush Brush to inspect. @param Fallback Fallback size. @return Drawable size. */
	FVector2D ResolveBrushSize(const FSlateBrush& Brush, const FVector2D& Fallback)
	{
		if (Brush.DrawAs == ESlateBrushDrawType::NoDrawType)
		{
			return FVector2D::ZeroVector;
		}
		const FVector2D BrushSize(Brush.ImageSize);
		return BrushSize.X > 0.0f && BrushSize.Y > 0.0f ? BrushSize : Fallback;
	}

	/** @brief Draws one native brush. @param OutDrawElements Draw list. @param LayerId Paint layer. @param Geometry Owner geometry. @param Position Local layout position. @param Size Local brush size. @param Brush Brush to draw. @param Tint Additional tint. @param WidgetStyle Inherited style. @param RenderTransform Optional part-specific render transform. */
	void DrawBrush(
		FSlateWindowElementList& OutDrawElements,
		const int32 LayerId,
		const FGeometry& Geometry,
		const FVector2D& Position,
		const FVector2D& Size,
		const FSlateBrush& Brush,
		const FLinearColor& Tint,
		const FWidgetStyle& WidgetStyle,
		const FWidgetTransform* RenderTransform = nullptr)
	{
		if (Size.X <= 0.0f || Size.Y <= 0.0f || Brush.DrawAs == ESlateBrushDrawType::NoDrawType)
		{
			return;
		}
		const FPaintGeometry PaintGeometry = RenderTransform
			? Geometry.ToPaintGeometry(Size, FSlateLayoutTransform(Position), RenderTransform->ToSlateRenderTransform())
			: Geometry.ToPaintGeometry(Size, FSlateLayoutTransform(Position));
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			PaintGeometry,
			&Brush,
			ESlateDrawEffect::None,
			Brush.GetTint(WidgetStyle) * Tint * WidgetStyle.GetColorAndOpacityTint());
	}
}

void SLunarOptionSliderVisual::Construct(const FArguments& InArgs)
{
	SetVisibility(EVisibility::HitTestInvisible);
	ValueFont = FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 10);
#if WITH_ACCESSIBILITY
	SetAccessibleBehavior(EAccessibleBehavior::NotAccessible);
#endif
}

void SLunarOptionSliderVisual::SetPresentation(
	const FSlateBrush& InPreviousArrowBrush,
	const FLinearColor& InPreviousArrowTint,
	const FWidgetTransform& InPreviousArrowTransform,
	const FSlateBrush& InNextArrowBrush,
	const FLinearColor& InNextArrowTint,
	const FWidgetTransform& InNextArrowTransform,
	const FSlateColor& InValueTextColor,
	const FSlateFontInfo& InValueFont)
{
	PreviousArrowBrush = InPreviousArrowBrush;
	PreviousArrowTint = InPreviousArrowTint;
	PreviousArrowTransform = InPreviousArrowTransform;
	NextArrowBrush = InNextArrowBrush;
	NextArrowTint = InNextArrowTint;
	NextArrowTransform = InNextArrowTransform;
	ValueTextColor = InValueTextColor;
	ValueFont = InValueFont;
	Invalidate(EInvalidateWidgetReason::Paint | EInvalidateWidgetReason::Layout);
}

void SLunarOptionSliderVisual::SetValue(FText NewValueText, const EOrientation NewOrientation)
{
	ValueText = MoveTemp(NewValueText);
	Orientation = NewOrientation;
	Invalidate(EInvalidateWidgetReason::Paint | EInvalidateWidgetReason::Layout);
}

FVector2D SLunarOptionSliderVisual::ComputeDesiredSize(const float LayoutScaleMultiplier) const
{
	using namespace LunarValueControlVisuals_Private;
	const FVector2D PreviousSize = ResolveBrushSize(PreviousArrowBrush, FVector2D(16.0f));
	const FVector2D NextSize = ResolveBrushSize(NextArrowBrush, FVector2D(16.0f));
	FVector2D TextSize = FVector2D::ZeroVector;
	if (!ValueText.IsEmpty() && FSlateApplication::IsInitialized())
	{
		TextSize = FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(ValueText, ValueFont);
	}
	return Orientation == Orient_Horizontal
		? FVector2D(PreviousSize.X + TextSize.X + NextSize.X, FMath::Max3(PreviousSize.Y, TextSize.Y, NextSize.Y))
		: FVector2D(FMath::Max3(PreviousSize.X, TextSize.X, NextSize.X), PreviousSize.Y + TextSize.Y + NextSize.Y);
}

int32 SLunarOptionSliderVisual::OnPaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	const int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	const bool bParentEnabled) const
{
	using namespace LunarValueControlVisuals_Private;
	const FVector2D AllottedSize = AllottedGeometry.GetLocalSize();
	const FVector2D PreviousSize = ResolveBrushSize(PreviousArrowBrush, FVector2D(16.0f));
	const FVector2D NextSize = ResolveBrushSize(NextArrowBrush, FVector2D(16.0f));
	const FVector2D PreviousPosition = Orientation == Orient_Horizontal
		? FVector2D(0.0f, (AllottedSize.Y - PreviousSize.Y) * 0.5f)
		: FVector2D((AllottedSize.X - PreviousSize.X) * 0.5f, 0.0f);
	const FVector2D NextPosition = Orientation == Orient_Horizontal
		? FVector2D(AllottedSize.X - NextSize.X, (AllottedSize.Y - NextSize.Y) * 0.5f)
		: FVector2D((AllottedSize.X - NextSize.X) * 0.5f, AllottedSize.Y - NextSize.Y);
	DrawBrush(OutDrawElements, LayerId, AllottedGeometry, PreviousPosition, PreviousSize, PreviousArrowBrush, PreviousArrowTint, InWidgetStyle, &PreviousArrowTransform);
	DrawBrush(OutDrawElements, LayerId, AllottedGeometry, NextPosition, NextSize, NextArrowBrush, NextArrowTint, InWidgetStyle, &NextArrowTransform);

	if (!ValueText.IsEmpty() && FSlateApplication::IsInitialized())
	{
		const FVector2D TextSize = FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(ValueText, ValueFont);
		const FVector2D TextPosition = (AllottedSize - TextSize) * 0.5f;
		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId + 1,
			AllottedGeometry.ToPaintGeometry(TextSize, FSlateLayoutTransform(TextPosition)),
			ValueText,
			ValueFont,
			ESlateDrawEffect::None,
			ValueTextColor.GetColor(InWidgetStyle) * InWidgetStyle.GetColorAndOpacityTint());
	}
	return LayerId + 1;
}

void SLunarSwitchVisual::Construct(const FArguments& InArgs)
{
	SetVisibility(EVisibility::HitTestInvisible);
	TrackBrush.DrawAs = ESlateBrushDrawType::Box;
	TrackBrush.ImageSize = TrackSize;
	HandleBrush.DrawAs = ESlateBrushDrawType::Box;
	HandleBrush.ImageSize = HandleSize;
#if WITH_ACCESSIBILITY
	SetAccessibleBehavior(EAccessibleBehavior::NotAccessible);
#endif
}

void SLunarSwitchVisual::SetPresentation(
	const FSlateBrush& InTrackBrush,
	const FLinearColor& InTrackTint,
	const FVector2D& InTrackSize,
	const FSlateBrush& InHandleBrush,
	const FLinearColor& InHandleTint,
	const FVector2D& InHandleSize,
	const FVector2D& InHandleOffset)
{
	TrackBrush = InTrackBrush;
	TrackTint = InTrackTint;
	TrackSize = InTrackSize.ComponentMax(FVector2D::ZeroVector);
	HandleBrush = InHandleBrush;
	HandleTint = InHandleTint;
	HandleSize = InHandleSize.ComponentMax(FVector2D::ZeroVector);
	HandleOffset = InHandleOffset;
	Invalidate(EInvalidateWidgetReason::Paint | EInvalidateWidgetReason::Layout);
}

void SLunarSwitchVisual::SetHandleAlpha(const float NewHandleAlpha, const ELunarSwitchDirectionMode NewDirectionMode)
{
	HandleAlpha = FMath::Clamp(NewHandleAlpha, 0.0f, 1.0f);
	DirectionMode = NewDirectionMode;
	Invalidate(EInvalidateWidgetReason::Paint);
}

FVector2D SLunarSwitchVisual::ComputeDesiredSize(const float LayoutScaleMultiplier) const
{
	return TrackSize;
}

int32 SLunarSwitchVisual::OnPaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	const int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	const bool bParentEnabled) const
{
	using namespace LunarValueControlVisuals_Private;
	const FVector2D AllottedSize = AllottedGeometry.GetLocalSize();
	const FVector2D EffectiveTrackSize = TrackSize.ComponentMin(AllottedSize);
	const FVector2D TrackPosition = (AllottedSize - EffectiveTrackSize) * 0.5f;
	DrawBrush(OutDrawElements, LayerId, AllottedGeometry, TrackPosition, EffectiveTrackSize, TrackBrush, TrackTint, InWidgetStyle);

	FVector2D HandlePosition;
	if (DirectionMode == ELunarSwitchDirectionMode::Vertical)
	{
		HandlePosition = FVector2D(
			TrackPosition.X + (EffectiveTrackSize.X - HandleSize.X) * 0.5f,
			TrackPosition.Y + (EffectiveTrackSize.Y - HandleSize.Y) * (1.0f - HandleAlpha));
	}
	else
	{
		HandlePosition = FVector2D(
			TrackPosition.X + (EffectiveTrackSize.X - HandleSize.X) * HandleAlpha,
			TrackPosition.Y + (EffectiveTrackSize.Y - HandleSize.Y) * 0.5f);
	}
	HandlePosition += HandleOffset;
	DrawBrush(OutDrawElements, LayerId + 1, AllottedGeometry, HandlePosition, HandleSize, HandleBrush, HandleTint, InWidgetStyle);
	return LayerId + 1;
}