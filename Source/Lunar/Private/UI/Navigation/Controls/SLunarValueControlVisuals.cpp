// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "UI/Navigation/Controls/SLunarValueControlVisuals.h"

#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Kismet/KismetMathLibrary.h"
#include "Rendering/DrawElements.h"
#include "Rendering/SlateRenderer.h"
#include "Settings/LunarSettings.h"
#include "Styling/CoreStyle.h"
#include "Styling/StyleDefaults.h"
#include "UI/Navigation/Styles/LunarStyleResolver.h"

/**
 * @file SLunarValueControlVisuals.cpp
 * @brief Slate rendering and transitions for Lunar option-slider and switch presentations
 * @ingroup LunarNavigationControls
 */

namespace LunarValueControlVisuals_Private
{
	/**
	 * @brief Fills missing OptionSlider fields with native fallback values
	 * @param ResolvedStyle Partially resolved option-slider style
	 * @return Complete style snapshot suitable for transitions and painting
	 */
	FLunarOptionSliderStylePatch MaterializeOptionSliderStyle(
		const FLunarOptionSliderStylePatch& ResolvedStyle)
	{
		FLunarOptionSliderStylePatch Result = ResolvedStyle;
		if (!Result.bOverridePreviousArrowBrush)
		{
			Result.bOverridePreviousArrowBrush = true;
			Result.PreviousArrowBrush = *FStyleDefaults::GetNoBrush();
		}
		if (!Result.bOverrideNextArrowBrush)
		{
			Result.bOverrideNextArrowBrush = true;
			Result.NextArrowBrush = *FStyleDefaults::GetNoBrush();
		}
		if (!Result.bOverridePreviousArrowTint)
		{
			Result.bOverridePreviousArrowTint = true;
			Result.PreviousArrowTint = FLinearColor::White;
		}
		if (!Result.bOverrideNextArrowTint)
		{
			Result.bOverrideNextArrowTint = true;
			Result.NextArrowTint = FLinearColor::White;
		}
		if (!Result.bOverrideValueTextColor)
		{
			Result.bOverrideValueTextColor = true;
			Result.ValueTextColor = Result.Common.bOverrideTextColor
				? Result.Common.TextColor
				: FSlateColor::UseForeground();
		}
		if (!Result.bOverrideValueFont)
		{
			Result.bOverrideValueFont = true;
			Result.ValueFont = Result.Common.bOverrideFont
				? Result.Common.Font
				: FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 10);
		}
		return Result;
	}

	/**
	 * @brief Fills missing Switch fields with native fallback values
	 * @param ResolvedStyle Partially resolved switch style
	 * @return Complete style snapshot suitable for transitions and painting
	 */
	FLunarSwitchStylePatch MaterializeSwitchStyle(const FLunarSwitchStylePatch& ResolvedStyle)
	{
		FLunarSwitchStylePatch Result = ResolvedStyle;
		if (!Result.bOverrideTrackBrush)
		{
			Result.bOverrideTrackBrush = true;
			Result.TrackBrush = *FStyleDefaults::GetNoBrush();
		}
		if (!Result.bOverrideHandleBrush)
		{
			Result.bOverrideHandleBrush = true;
			Result.HandleBrush = *FStyleDefaults::GetNoBrush();
		}
		if (!Result.bOverrideTrackTint)
		{
			Result.bOverrideTrackTint = true;
			Result.TrackTint = FLinearColor::White;
		}
		if (!Result.bOverrideHandleTint)
		{
			Result.bOverrideHandleTint = true;
			Result.HandleTint = FLinearColor::White;
		}
		if (!Result.bOverrideTrackSize)
		{
			Result.bOverrideTrackSize = true;
			Result.TrackSize = FVector2D(Result.TrackBrush.ImageSize);
		}
		if (!Result.bOverrideHandleSize)
		{
			Result.bOverrideHandleSize = true;
			Result.HandleSize = FVector2D(Result.HandleBrush.ImageSize);
		}
		if (!Result.bOverrideHandleOffset)
		{
			Result.bOverrideHandleOffset = true;
			Result.HandleOffset = FVector2D::ZeroVector;
		}
		return Result;
	}

/** Compares one optional specialized style field including its override flag. */
#define LUNAR_SPECIALIZED_FIELD_DIFFERS(A, B, FieldName) \
	((A).bOverride##FieldName != (B).bOverride##FieldName \
		|| ((A).bOverride##FieldName && !((A).FieldName == (B).FieldName)))

	/**
	 * @brief Compares two materialized OptionSlider styles
	 * @param A First style
	 * @param B Second style
	 * @return True when all common and specialized fields are equivalent
	 */
	bool AreOptionSliderStylesEquivalent(
		const FLunarOptionSliderStylePatch& A,
		const FLunarOptionSliderStylePatch& B)
	{
		return LunarStyleResolver::AreCommonStylesEquivalent(A.Common, B.Common)
			&& !LUNAR_SPECIALIZED_FIELD_DIFFERS(A, B, PreviousArrowBrush)
			&& !LUNAR_SPECIALIZED_FIELD_DIFFERS(A, B, NextArrowBrush)
			&& !LUNAR_SPECIALIZED_FIELD_DIFFERS(A, B, PreviousArrowTint)
			&& !LUNAR_SPECIALIZED_FIELD_DIFFERS(A, B, NextArrowTint)
			&& !LUNAR_SPECIALIZED_FIELD_DIFFERS(A, B, ValueTextColor)
			&& !LUNAR_SPECIALIZED_FIELD_DIFFERS(A, B, ValueFont);
	}

	/**
	 * @brief Compares two materialized Switch styles
	 * @param A First style
	 * @param B Second style
	 * @return True when all common and specialized fields are equivalent
	 */
	bool AreSwitchStylesEquivalent(const FLunarSwitchStylePatch& A, const FLunarSwitchStylePatch& B)
	{
		return LunarStyleResolver::AreCommonStylesEquivalent(A.Common, B.Common)
			&& !LUNAR_SPECIALIZED_FIELD_DIFFERS(A, B, TrackBrush)
			&& !LUNAR_SPECIALIZED_FIELD_DIFFERS(A, B, HandleBrush)
			&& !LUNAR_SPECIALIZED_FIELD_DIFFERS(A, B, TrackTint)
			&& !LUNAR_SPECIALIZED_FIELD_DIFFERS(A, B, HandleTint)
			&& !LUNAR_SPECIALIZED_FIELD_DIFFERS(A, B, TrackSize)
			&& !LUNAR_SPECIALIZED_FIELD_DIFFERS(A, B, HandleSize)
			&& !LUNAR_SPECIALIZED_FIELD_DIFFERS(A, B, HandleOffset);
	}

#undef LUNAR_SPECIALIZED_FIELD_DIFFERS

	/**
	 * @brief Interpolates continuous fields between two OptionSlider styles
	 * @param Source Transition source
	 * @param Target Transition target
	 * @param Alpha Normalized interpolation alpha
	 * @param bUseSourceDiscreteFields Whether brushes and font remain sourced until completion
	 * @return Interpolated style snapshot
	 */
	FLunarOptionSliderStylePatch InterpolateOptionSliderStyle(
		const FLunarOptionSliderStylePatch& Source,
		const FLunarOptionSliderStylePatch& Target,
		const float Alpha,
		const bool bUseSourceDiscreteFields)
	{
		FLunarOptionSliderStylePatch Result = Target;
		Result.Common = LunarStyleResolver::InterpolateCommonStylePatch(
			Source.Common,
			Target.Common,
			Alpha);
		if (Source.bOverridePreviousArrowTint && Target.bOverridePreviousArrowTint)
		{
			Result.PreviousArrowTint = FMath::Lerp(Source.PreviousArrowTint, Target.PreviousArrowTint, Alpha);
		}
		if (Source.bOverrideNextArrowTint && Target.bOverrideNextArrowTint)
		{
			Result.NextArrowTint = FMath::Lerp(Source.NextArrowTint, Target.NextArrowTint, Alpha);
		}
		if (bUseSourceDiscreteFields)
		{
			LunarStyleResolver::ApplyCommonDiscreteFields(Result.Common, Source.Common);
			Result.bOverridePreviousArrowBrush = Source.bOverridePreviousArrowBrush;
			Result.PreviousArrowBrush = Source.PreviousArrowBrush;
			Result.bOverrideNextArrowBrush = Source.bOverrideNextArrowBrush;
			Result.NextArrowBrush = Source.NextArrowBrush;
			Result.bOverrideValueFont = Source.bOverrideValueFont;
			Result.ValueFont = Source.ValueFont;
		}
		return Result;
	}

	/**
	 * @brief Interpolates continuous fields between two Switch styles
	 * @param Source Transition source
	 * @param Target Transition target
	 * @param Alpha Normalized interpolation alpha
	 * @param bUseSourceDiscreteFields Whether brushes remain sourced until completion
	 * @return Interpolated style snapshot
	 */
	FLunarSwitchStylePatch InterpolateSwitchStyle(
		const FLunarSwitchStylePatch& Source,
		const FLunarSwitchStylePatch& Target,
		const float Alpha,
		const bool bUseSourceDiscreteFields)
	{
		FLunarSwitchStylePatch Result = Target;
		Result.Common = LunarStyleResolver::InterpolateCommonStylePatch(
			Source.Common,
			Target.Common,
			Alpha);
		if (Source.bOverrideTrackTint && Target.bOverrideTrackTint)
		{
			Result.TrackTint = FMath::Lerp(Source.TrackTint, Target.TrackTint, Alpha);
		}
		if (Source.bOverrideHandleTint && Target.bOverrideHandleTint)
		{
			Result.HandleTint = FMath::Lerp(Source.HandleTint, Target.HandleTint, Alpha);
		}
		if (Source.bOverrideTrackSize && Target.bOverrideTrackSize)
		{
			Result.TrackSize = FMath::Lerp(Source.TrackSize, Target.TrackSize, Alpha);
		}
		if (Source.bOverrideHandleSize && Target.bOverrideHandleSize)
		{
			Result.HandleSize = FMath::Lerp(Source.HandleSize, Target.HandleSize, Alpha);
		}
		if (Source.bOverrideHandleOffset && Target.bOverrideHandleOffset)
		{
			Result.HandleOffset = FMath::Lerp(Source.HandleOffset, Target.HandleOffset, Alpha);
		}
		if (bUseSourceDiscreteFields)
		{
			LunarStyleResolver::ApplyCommonDiscreteFields(Result.Common, Source.Common);
			Result.bOverrideTrackBrush = Source.bOverrideTrackBrush;
			Result.TrackBrush = Source.TrackBrush;
			Result.bOverrideHandleBrush = Source.bOverrideHandleBrush;
			Result.HandleBrush = Source.HandleBrush;
		}
		return Result;
	}

	/** @return True when global navigation accessibility requests reduced motion. */
	bool ShouldReduceMotion()
	{
		const ULunarSettings* Settings = GetDefault<ULunarSettings>();
		return Settings && Settings->Navigation.Accessibility.bReduceMotion;
	}

	/**
	 * @brief Resolves a drawable brush size
	 * @param Brush Brush whose image size should be used
	 * @param Fallback Size used when the brush has no positive image extent
	 * @return Zero for NoDraw brushes, otherwise the brush or fallback size
	 */
	FVector2D ResolveBrushSize(const FSlateBrush& Brush, const FVector2D& Fallback)
	{
		if (Brush.DrawAs == ESlateBrushDrawType::NoDrawType)
		{
			return FVector2D::ZeroVector;
		}
		const FVector2D BrushSize(Brush.ImageSize);
		return BrushSize.X > 0.0f && BrushSize.Y > 0.0f ? BrushSize : Fallback;
	}

	/**
	 * @brief Combines brush, authored, and inherited color multipliers
	 * @param Brush Brush contributing its own tint
	 * @param AuthoredTint Explicit specialized tint
	 * @param WidgetStyle Inherited Slate widget style
	 * @return Multiplicative color used for painting
	 */
	FLinearColor ResolveBrushTint(
		const FSlateBrush& Brush,
		const FLinearColor& AuthoredTint,
		const FWidgetStyle& WidgetStyle)
	{
		return Brush.GetTint(WidgetStyle)
			* AuthoredTint
			* WidgetStyle.GetColorAndOpacityTint();
	}

	/**
	 * @brief Appends one non-empty brush box to a Slate paint list
	 * @param OutDrawElements Draw-element list to append to
	 * @param LayerId Paint layer for the box
	 * @param Geometry Widget geometry used to build paint geometry
	 * @param Position Local top-left position
	 * @param Size Local box size
	 * @param Brush Brush to draw
	 * @param Tint Authored specialized tint
	 * @param WidgetStyle Inherited Slate widget style
	 */
	void DrawBrush(
		FSlateWindowElementList& OutDrawElements,
		const int32 LayerId,
		const FGeometry& Geometry,
		const FVector2D& Position,
		const FVector2D& Size,
		const FSlateBrush& Brush,
		const FLinearColor& Tint,
		const FWidgetStyle& WidgetStyle)
	{
		if (Size.X <= 0.0f || Size.Y <= 0.0f)
		{
			return;
		}
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			Geometry.ToPaintGeometry(Size, FSlateLayoutTransform(Position)),
			&Brush,
			ESlateDrawEffect::None,
			ResolveBrushTint(Brush, Tint, WidgetStyle));
	}
}

/** @cond DOXYGEN_INTERNAL */
void SLunarOptionSliderVisual::Construct(const FArguments& InArgs)
{
	SetCanTick(true);
	SetVisibility(EVisibility::HitTestInvisible);
#if WITH_ACCESSIBILITY
	SetAccessibleBehavior(EAccessibleBehavior::NotAccessible);
#endif
}
/** @endcond */

void SLunarOptionSliderVisual::SetStyleTarget(const FLunarOptionSliderStylePatch& NewStyle)
{
	using namespace LunarValueControlVisuals_Private;
	const FLunarOptionSliderStylePatch MaterializedStyle = MaterializeOptionSliderStyle(NewStyle);
	if (!bHasDisplayedStyle || ShouldReduceMotion())
	{
		ApplyImmediateStyle(MaterializedStyle);
		return;
	}
	if (AreOptionSliderStylesEquivalent(LogicalTargetStyle, MaterializedStyle))
	{
		return;
	}

	if (bTransitionActive)
	{
		const bool bReturnsToSource = !bTransitionReversing
			&& AreOptionSliderStylesEquivalent(TransitionSourceLogicalStyle, MaterializedStyle);
		const bool bReturnsToForwardTarget = bTransitionReversing
			&& AreOptionSliderStylesEquivalent(TransitionTargetStyle, MaterializedStyle);
		if (bReturnsToSource || bReturnsToForwardTarget)
		{
			LogicalTargetStyle = MaterializedStyle;
			bTransitionReversing = bReturnsToSource;
			DisplayedStyle = InterpolateOptionSliderStyle(
				TransitionSourceStyle,
				TransitionTargetStyle,
				TransitionPaintAlpha,
				bTransitionReversing);
			Invalidate(EInvalidateWidgetReason::Paint | EInvalidateWidgetReason::Layout);
			return;
		}
	}

	if (!MaterializedStyle.Common.Transition.bEnabled || MaterializedStyle.Common.Transition.Duration <= 0.0f
		|| AreOptionSliderStylesEquivalent(DisplayedStyle, MaterializedStyle))
	{
		ApplyImmediateStyle(MaterializedStyle);
		return;
	}

	const bool bRetargetingTransition = bTransitionActive;
	const FLunarOptionSliderStylePatch PreviousLogicalStyle = LogicalTargetStyle;
	TransitionSourceStyle = DisplayedStyle;
	TransitionSourceLogicalStyle = bRetargetingTransition
		? DisplayedStyle
		: PreviousLogicalStyle;
	if (bHasLastPaintedValueTextColor)
	{
		if (bRetargetingTransition
			&& LastPaintedValueTextColor != LastPaintedLogicalTargetValueTextColor)
		{
			// Preserve complete visual identity when the displayed color is truly
			// between endpoints, while retaining authored dynamic identity at an endpoint.
			TransitionSourceLogicalStyle.bOverrideValueTextColor = true;
			TransitionSourceLogicalStyle.ValueTextColor = FSlateColor(LastPaintedValueTextColor);
		}
		TransitionSourceStyle.bOverrideValueTextColor = true;
		TransitionSourceStyle.ValueTextColor = FSlateColor(LastPaintedValueTextColor);
	}
	TransitionTargetStyle = MaterializedStyle;
	LogicalTargetStyle = MaterializedStyle;
	TransitionElapsed = 0.0f;
	TransitionDuration = MaterializedStyle.Common.Transition.Duration;
	bTransitionActive = true;
	bTransitionReversing = false;
	TransitionPaintAlpha = 0.0f;
	DisplayedStyle = InterpolateOptionSliderStyle(TransitionSourceStyle, TransitionTargetStyle, 0.0f, false);
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
	const FVector2D PreviousSize = DisplayedStyle.bOverridePreviousArrowBrush
		? ResolveBrushSize(DisplayedStyle.PreviousArrowBrush, FVector2D(16.0f))
		: FVector2D::ZeroVector;
	const FVector2D NextSize = DisplayedStyle.bOverrideNextArrowBrush
		? ResolveBrushSize(DisplayedStyle.NextArrowBrush, FVector2D(16.0f))
		: FVector2D::ZeroVector;
	FVector2D TextSize = FVector2D::ZeroVector;
	if (!ValueText.IsEmpty() && FSlateApplication::IsInitialized())
	{
		const FSlateFontInfo& Font = DisplayedStyle.ValueFont;
		TextSize = FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(ValueText, Font);
	}
	return Orientation == Orient_Horizontal
		? FVector2D(
			PreviousSize.X + TextSize.X + NextSize.X,
			FMath::Max3(PreviousSize.Y, TextSize.Y, NextSize.Y))
		: FVector2D(
			FMath::Max3(PreviousSize.X, TextSize.X, NextSize.X),
			PreviousSize.Y + TextSize.Y + NextSize.Y);
}

void SLunarOptionSliderVisual::Tick(
	const FGeometry& AllottedGeometry,
	const double InCurrentTime,
	const float InDeltaTime)
{
	SLeafWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	if (!bTransitionActive)
	{
		return;
	}
	using namespace LunarValueControlVisuals_Private;
	if (ShouldReduceMotion())
	{
		ApplyImmediateStyle(LogicalTargetStyle);
		return;
	}

	TransitionElapsed += bTransitionReversing ? -FMath::Max(0.0f, InDeltaTime) : FMath::Max(0.0f, InDeltaTime);
	TransitionElapsed = FMath::Clamp(TransitionElapsed, 0.0f, TransitionDuration);
	const float Alpha = TransitionDuration > 0.0f ? TransitionElapsed / TransitionDuration : 1.0f;
	const float EasedAlpha = static_cast<float>(UKismetMathLibrary::Ease(
		0.0,
		1.0,
		Alpha,
		TransitionTargetStyle.Common.Transition.Easing));
	TransitionPaintAlpha = FMath::Clamp(EasedAlpha, 0.0f, 1.0f);
	DisplayedStyle = InterpolateOptionSliderStyle(
		TransitionSourceStyle,
		TransitionTargetStyle,
		EasedAlpha,
		bTransitionReversing);
	const bool bFinished = bTransitionReversing ? Alpha <= 0.0f : Alpha >= 1.0f;
	if (bFinished)
	{
		DisplayedStyle = bTransitionReversing ? TransitionSourceLogicalStyle : TransitionTargetStyle;
		TransitionPaintAlpha = bTransitionReversing ? 0.0f : 1.0f;
		bTransitionActive = false;
		bTransitionReversing = false;
	}
	Invalidate(EInvalidateWidgetReason::Paint | EInvalidateWidgetReason::Layout);
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
	const FVector2D PreviousSize = DisplayedStyle.bOverridePreviousArrowBrush
		? ResolveBrushSize(DisplayedStyle.PreviousArrowBrush, FVector2D(16.0f))
		: FVector2D::ZeroVector;
	const FVector2D NextSize = DisplayedStyle.bOverrideNextArrowBrush
		? ResolveBrushSize(DisplayedStyle.NextArrowBrush, FVector2D(16.0f))
		: FVector2D::ZeroVector;
	const FVector2D PreviousPosition = Orientation == Orient_Horizontal
		? FVector2D(0.0f, (AllottedSize.Y - PreviousSize.Y) * 0.5f)
		: FVector2D((AllottedSize.X - PreviousSize.X) * 0.5f, 0.0f);
	const FVector2D NextPosition = Orientation == Orient_Horizontal
		? FVector2D(AllottedSize.X - NextSize.X, (AllottedSize.Y - NextSize.Y) * 0.5f)
		: FVector2D((AllottedSize.X - NextSize.X) * 0.5f, AllottedSize.Y - NextSize.Y);

	if (DisplayedStyle.bOverridePreviousArrowBrush)
	{
		DrawBrush(
			OutDrawElements,
			LayerId,
			AllottedGeometry,
			PreviousPosition,
			PreviousSize,
			DisplayedStyle.PreviousArrowBrush,
			DisplayedStyle.bOverridePreviousArrowTint ? DisplayedStyle.PreviousArrowTint : FLinearColor::White,
			InWidgetStyle);
	}
	if (DisplayedStyle.bOverrideNextArrowBrush)
	{
		DrawBrush(
			OutDrawElements,
			LayerId,
			AllottedGeometry,
			NextPosition,
			NextSize,
			DisplayedStyle.NextArrowBrush,
			DisplayedStyle.bOverrideNextArrowTint ? DisplayedStyle.NextArrowTint : FLinearColor::White,
			InWidgetStyle);
	}

	if (!ValueText.IsEmpty() && FSlateApplication::IsInitialized())
	{
		const FSlateFontInfo Font = DisplayedStyle.bOverrideValueFont
			? DisplayedStyle.ValueFont
			: FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 10);
		const TSharedRef<FSlateFontMeasure> FontMeasure =
			FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		const FVector2D TextSize = FontMeasure->Measure(ValueText, Font);
		const FVector2D TextPosition = (AllottedSize - TextSize) * 0.5f;
		FLinearColor ResolvedTextColor = DisplayedStyle.bOverrideValueTextColor
			? DisplayedStyle.ValueTextColor.GetColor(InWidgetStyle)
			: InWidgetStyle.GetForegroundColor();
		if (bTransitionActive
			&& TransitionSourceStyle.bOverrideValueTextColor
			&& TransitionTargetStyle.bOverrideValueTextColor)
		{
			ResolvedTextColor = FMath::Lerp(
				TransitionSourceStyle.ValueTextColor.GetColor(InWidgetStyle),
				TransitionTargetStyle.ValueTextColor.GetColor(InWidgetStyle),
				TransitionPaintAlpha);
		}
		LastPaintedValueTextColor = ResolvedTextColor;
		LastPaintedLogicalTargetValueTextColor = LogicalTargetStyle.bOverrideValueTextColor
			? LogicalTargetStyle.ValueTextColor.GetColor(InWidgetStyle)
			: InWidgetStyle.GetForegroundColor();
		bHasLastPaintedValueTextColor = true;
		const FLinearColor TextColor = ResolvedTextColor * InWidgetStyle.GetColorAndOpacityTint();
		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId + 1,
			AllottedGeometry.ToPaintGeometry(TextSize, FSlateLayoutTransform(TextPosition)),
			ValueText,
			Font,
			ESlateDrawEffect::None,
			TextColor);
	}
	return LayerId + 1;
}

void SLunarOptionSliderVisual::ApplyImmediateStyle(const FLunarOptionSliderStylePatch& NewStyle)
{
	DisplayedStyle = NewStyle;
	TransitionSourceStyle = NewStyle;
	TransitionSourceLogicalStyle = NewStyle;
	TransitionTargetStyle = NewStyle;
	LogicalTargetStyle = NewStyle;
	TransitionElapsed = 0.0f;
	TransitionDuration = 0.0f;
	TransitionPaintAlpha = 1.0f;
	bHasDisplayedStyle = true;
	bTransitionActive = false;
	bTransitionReversing = false;
	Invalidate(EInvalidateWidgetReason::Paint | EInvalidateWidgetReason::Layout);
}

/** @cond DOXYGEN_INTERNAL */
void SLunarSwitchVisual::Construct(const FArguments& InArgs)
{
	SetCanTick(true);
	SetVisibility(EVisibility::HitTestInvisible);
#if WITH_ACCESSIBILITY
	SetAccessibleBehavior(EAccessibleBehavior::NotAccessible);
#endif
}
/** @endcond */

void SLunarSwitchVisual::SetStyleTarget(const FLunarSwitchStylePatch& NewStyle)
{
	using namespace LunarValueControlVisuals_Private;
	const FLunarSwitchStylePatch MaterializedStyle = MaterializeSwitchStyle(NewStyle);
	if (!bHasDisplayedStyle || ShouldReduceMotion())
	{
		ApplyImmediateStyle(MaterializedStyle);
		return;
	}
	if (AreSwitchStylesEquivalent(LogicalTargetStyle, MaterializedStyle))
	{
		return;
	}

	if (bTransitionActive)
	{
		const bool bReturnsToSource = !bTransitionReversing
			&& AreSwitchStylesEquivalent(TransitionSourceStyle, MaterializedStyle);
		const bool bReturnsToForwardTarget = bTransitionReversing
			&& AreSwitchStylesEquivalent(TransitionTargetStyle, MaterializedStyle);
		if (bReturnsToSource || bReturnsToForwardTarget)
		{
			LogicalTargetStyle = MaterializedStyle;
			bTransitionReversing = bReturnsToSource;
			const FLunarSwitchStylePatch& DiscreteTarget = bTransitionReversing
				? TransitionSourceStyle
				: TransitionTargetStyle;
			LunarStyleResolver::ApplyCommonDiscreteFields(DisplayedStyle.Common, DiscreteTarget.Common);
			DisplayedStyle.bOverrideTrackBrush = DiscreteTarget.bOverrideTrackBrush;
			DisplayedStyle.TrackBrush = DiscreteTarget.TrackBrush;
			DisplayedStyle.bOverrideHandleBrush = DiscreteTarget.bOverrideHandleBrush;
			DisplayedStyle.HandleBrush = DiscreteTarget.HandleBrush;
			Invalidate(EInvalidateWidgetReason::Paint | EInvalidateWidgetReason::Layout);
			return;
		}
	}

	if (!MaterializedStyle.Common.Transition.bEnabled || MaterializedStyle.Common.Transition.Duration <= 0.0f
		|| AreSwitchStylesEquivalent(DisplayedStyle, MaterializedStyle))
	{
		ApplyImmediateStyle(MaterializedStyle);
		return;
	}

	TransitionSourceStyle = DisplayedStyle;
	TransitionTargetStyle = MaterializedStyle;
	LogicalTargetStyle = MaterializedStyle;
	TransitionElapsed = 0.0f;
	TransitionDuration = MaterializedStyle.Common.Transition.Duration;
	bTransitionActive = true;
	bTransitionReversing = false;
	DisplayedStyle = InterpolateSwitchStyle(TransitionSourceStyle, TransitionTargetStyle, 0.0f, false);
	Invalidate(EInvalidateWidgetReason::Paint | EInvalidateWidgetReason::Layout);
}

void SLunarSwitchVisual::SetValue(
	const bool bNewIsOn,
	const ELunarSwitchDirectionMode NewDirectionMode)
{
	using namespace LunarValueControlVisuals_Private;
	const bool bDirectionChanged = DirectionMode != NewDirectionMode;
	const float NewValueAlpha = bNewIsOn ? 1.0f : 0.0f;
	if (bHasDisplayedValue && bIsOn == bNewIsOn)
	{
		DirectionMode = NewDirectionMode;
		if (bDirectionChanged)
		{
			Invalidate(EInvalidateWidgetReason::Paint | EInvalidateWidgetReason::Layout);
		}
		return;
	}

	bIsOn = bNewIsOn;
	DirectionMode = NewDirectionMode;
	if (bValueTransitionActive)
	{
		const bool bReturnsToSource = !bValueTransitionReversing
			&& NewValueAlpha == ValueTransitionSourceAlpha;
		const bool bReturnsToForwardTarget = bValueTransitionReversing
			&& NewValueAlpha == ValueTransitionTargetAlpha;
		if (bReturnsToSource || bReturnsToForwardTarget)
		{
			LogicalValueTargetAlpha = NewValueAlpha;
			bValueTransitionReversing = bReturnsToSource;
			Invalidate(EInvalidateWidgetReason::Paint);
			return;
		}
	}

	if (!bHasDisplayedValue
		|| !bHasDisplayedStyle
		|| ShouldReduceMotion()
		|| !LogicalTargetStyle.Common.Transition.bEnabled
		|| LogicalTargetStyle.Common.Transition.Duration <= 0.0f)
	{
		ApplyImmediateValue(NewValueAlpha);
		return;
	}

	ValueTransitionSourceAlpha = DisplayedValueAlpha;
	ValueTransitionTargetAlpha = NewValueAlpha;
	LogicalValueTargetAlpha = NewValueAlpha;
	ValueTransitionElapsed = 0.0f;
	ValueTransitionDuration = LogicalTargetStyle.Common.Transition.Duration;
	ValueTransitionEasing = LogicalTargetStyle.Common.Transition.Easing;
	bValueTransitionActive = true;
	bValueTransitionReversing = false;
	Invalidate(EInvalidateWidgetReason::Paint | EInvalidateWidgetReason::Layout);
}

FVector2D SLunarSwitchVisual::ComputeDesiredSize(const float LayoutScaleMultiplier) const
{
	using namespace LunarValueControlVisuals_Private;
	if (DisplayedStyle.bOverrideTrackSize)
	{
		return DisplayedStyle.TrackSize;
	}
	return DisplayedStyle.bOverrideTrackBrush
		? ResolveBrushSize(DisplayedStyle.TrackBrush, FVector2D::ZeroVector)
		: FVector2D::ZeroVector;
}

void SLunarSwitchVisual::Tick(
	const FGeometry& AllottedGeometry,
	const double InCurrentTime,
	const float InDeltaTime)
{
	SLeafWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	using namespace LunarValueControlVisuals_Private;
	if (ShouldReduceMotion())
	{
		if (bTransitionActive)
		{
			ApplyImmediateStyle(LogicalTargetStyle);
		}
		if (bValueTransitionActive)
		{
			ApplyImmediateValue(LogicalValueTargetAlpha);
		}
		return;
	}

	bool bNeedsInvalidation = false;
	if (bTransitionActive)
	{
		TransitionElapsed += bTransitionReversing
			? -FMath::Max(0.0f, InDeltaTime)
			: FMath::Max(0.0f, InDeltaTime);
		TransitionElapsed = FMath::Clamp(TransitionElapsed, 0.0f, TransitionDuration);
		const float Alpha = TransitionDuration > 0.0f ? TransitionElapsed / TransitionDuration : 1.0f;
		const float EasedAlpha = static_cast<float>(UKismetMathLibrary::Ease(
			0.0,
			1.0,
			Alpha,
			TransitionTargetStyle.Common.Transition.Easing));
		DisplayedStyle = InterpolateSwitchStyle(
			TransitionSourceStyle,
			TransitionTargetStyle,
			EasedAlpha,
			bTransitionReversing);
		const bool bFinished = bTransitionReversing ? Alpha <= 0.0f : Alpha >= 1.0f;
		if (bFinished)
		{
			DisplayedStyle = bTransitionReversing ? TransitionSourceStyle : TransitionTargetStyle;
			bTransitionActive = false;
			bTransitionReversing = false;
		}
		bNeedsInvalidation = true;
	}

	if (bValueTransitionActive)
	{
		ValueTransitionElapsed += bValueTransitionReversing
			? -FMath::Max(0.0f, InDeltaTime)
			: FMath::Max(0.0f, InDeltaTime);
		ValueTransitionElapsed = FMath::Clamp(
			ValueTransitionElapsed,
			0.0f,
			ValueTransitionDuration);
		const float Alpha = ValueTransitionDuration > 0.0f
			? ValueTransitionElapsed / ValueTransitionDuration
			: 1.0f;
		const float EasedAlpha = static_cast<float>(UKismetMathLibrary::Ease(
			0.0,
			1.0,
			Alpha,
			ValueTransitionEasing));
		DisplayedValueAlpha = FMath::Lerp(
			ValueTransitionSourceAlpha,
			ValueTransitionTargetAlpha,
			EasedAlpha);
		const bool bFinished = bValueTransitionReversing ? Alpha <= 0.0f : Alpha >= 1.0f;
		if (bFinished)
		{
			DisplayedValueAlpha = bValueTransitionReversing
				? ValueTransitionSourceAlpha
				: ValueTransitionTargetAlpha;
			bValueTransitionActive = false;
			bValueTransitionReversing = false;
		}
		bNeedsInvalidation = true;
	}

	if (bNeedsInvalidation)
	{
		Invalidate(EInvalidateWidgetReason::Paint | EInvalidateWidgetReason::Layout);
	}
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
	const FVector2D TrackSize = DisplayedStyle.bOverrideTrackSize
		? DisplayedStyle.TrackSize
		: (DisplayedStyle.bOverrideTrackBrush
			? ResolveBrushSize(DisplayedStyle.TrackBrush, AllottedSize)
			: AllottedSize);
	const FVector2D TrackPosition = (AllottedSize - TrackSize) * 0.5f;
	if (DisplayedStyle.bOverrideTrackBrush)
	{
		DrawBrush(
			OutDrawElements,
			LayerId,
			AllottedGeometry,
			TrackPosition,
			TrackSize,
			DisplayedStyle.TrackBrush,
			DisplayedStyle.bOverrideTrackTint ? DisplayedStyle.TrackTint : FLinearColor::White,
			InWidgetStyle);
	}

	if (DisplayedStyle.bOverrideHandleBrush)
	{
		const FVector2D HandleSize = DisplayedStyle.bOverrideHandleSize
			? DisplayedStyle.HandleSize
			: ResolveBrushSize(DisplayedStyle.HandleBrush, TrackSize * 0.5f);
		FVector2D HandlePosition;
		if (DirectionMode == ELunarSwitchDirectionMode::Vertical)
		{
			HandlePosition = FVector2D(
				TrackPosition.X + (TrackSize.X - HandleSize.X) * 0.5f,
				TrackPosition.Y + (TrackSize.Y - HandleSize.Y) * (1.0f - DisplayedValueAlpha));
		}
		else
		{
			HandlePosition = FVector2D(
				TrackPosition.X + (TrackSize.X - HandleSize.X) * DisplayedValueAlpha,
				TrackPosition.Y + (TrackSize.Y - HandleSize.Y) * 0.5f);
		}
		if (DisplayedStyle.bOverrideHandleOffset)
		{
			HandlePosition += DisplayedStyle.HandleOffset;
		}
		DrawBrush(
			OutDrawElements,
			LayerId + 1,
			AllottedGeometry,
			HandlePosition,
			HandleSize,
			DisplayedStyle.HandleBrush,
			DisplayedStyle.bOverrideHandleTint ? DisplayedStyle.HandleTint : FLinearColor::White,
			InWidgetStyle);
	}
	return LayerId + 1;
}

void SLunarSwitchVisual::ApplyImmediateStyle(const FLunarSwitchStylePatch& NewStyle)
{
	DisplayedStyle = NewStyle;
	TransitionSourceStyle = NewStyle;
	TransitionTargetStyle = NewStyle;
	LogicalTargetStyle = NewStyle;
	TransitionElapsed = 0.0f;
	TransitionDuration = 0.0f;
	bHasDisplayedStyle = true;
	bTransitionActive = false;
	bTransitionReversing = false;
	if (bValueTransitionActive
		&& (LunarValueControlVisuals_Private::ShouldReduceMotion()
			|| !NewStyle.Common.Transition.bEnabled
			|| NewStyle.Common.Transition.Duration <= 0.0f))
	{
		ApplyImmediateValue(LogicalValueTargetAlpha);
	}
	Invalidate(EInvalidateWidgetReason::Paint | EInvalidateWidgetReason::Layout);
}

void SLunarSwitchVisual::ApplyImmediateValue(const float NewValueAlpha)
{
	DisplayedValueAlpha = FMath::Clamp(NewValueAlpha, 0.0f, 1.0f);
	ValueTransitionSourceAlpha = DisplayedValueAlpha;
	ValueTransitionTargetAlpha = DisplayedValueAlpha;
	LogicalValueTargetAlpha = DisplayedValueAlpha;
	ValueTransitionElapsed = 0.0f;
	ValueTransitionDuration = 0.0f;
	ValueTransitionEasing = LogicalTargetStyle.Common.Transition.Easing;
	bHasDisplayedValue = true;
	bValueTransitionActive = false;
	bValueTransitionReversing = false;
	Invalidate(EInvalidateWidgetReason::Paint | EInvalidateWidgetReason::Layout);
}
