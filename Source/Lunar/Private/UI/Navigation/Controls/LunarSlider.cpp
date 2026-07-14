// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "UI/Navigation/Controls/LunarSlider.h"

#include "Kismet/KismetMathLibrary.h"
#include "Rendering/DrawElements.h"
#include "Settings/LunarSettings.h"
#include "Styling/CoreStyle.h"
#include "UI/Navigation/Styles/LunarStyleResolver.h"
#include "UI/Navigation/Types/LunarGameplayTags.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/SOverlay.h"

#include <cmath>

/**
 * @file LunarSlider.cpp
 * @brief Continuous Lunar slider behavior, rendering, and style transitions
 * @ingroup LunarNavigationControls
 */

#define LOCTEXT_NAMESPACE "LunarSlider"

namespace LunarSlider_Private
{
	/** Default cross-axis thickness used when the style does not override the track. */
	constexpr float DefaultTrackThickness = 4.0f;
	/** Default square thumb extent used when no brush or size override is authored. */
	constexpr float DefaultThumbExtent = 14.0f;
	/** Default desired length of the native fallback presentation. */
	constexpr float DefaultSliderLength = 100.0f;
	/** Default unfilled track tint. */
	const FLinearColor DefaultTrackTint(0.16f, 0.16f, 0.16f, 1.0f);
	/** Default filled track tint. */
	const FLinearColor DefaultFillTint(0.10f, 0.48f, 0.95f, 1.0f);
	/** Default slider-thumb tint. */
	const FLinearColor DefaultThumbTint = FLinearColor::White;

	/**
	 * @brief Resolves an optional brush override to a drawable Slate brush
	 * @param bOverrideBrush Whether Brush is explicitly authored
	 * @param Brush Authored brush value
	 * @return Authored brush or the CoreStyle white brush fallback
	 */
	const FSlateBrush* ResolveBrush(const bool bOverrideBrush, const FSlateBrush& Brush)
	{
		return bOverrideBrush ? &Brush : FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));
	}

	/**
	 * @brief Resolves final paint tint from brush, authored, and inherited colors
	 * @param Brush Brush contributing its own tint
	 * @param WidgetStyle Inherited Slate widget style
	 * @param bOverrideTint Whether Tint is explicitly authored
	 * @param Tint Authored tint override
	 * @param FallbackTint Tint used when no override exists
	 * @return Multiplicative color used for painting
	 */
	FLinearColor ResolveTint(
		const FSlateBrush& Brush,
		const FWidgetStyle& WidgetStyle,
		const bool bOverrideTint,
		const FLinearColor& Tint,
		const FLinearColor& FallbackTint)
	{
		return Brush.GetTint(WidgetStyle)
			* (bOverrideTint ? Tint : FallbackTint)
			* WidgetStyle.GetColorAndOpacityTint();
	}

	/**
	 * @brief Resolves non-negative thumb size from size, brush, or default values
	 * @param Style Specialized slider style patch
	 * @return Effective thumb size
	 */
	FVector2D ResolveThumbSize(const FLunarSliderStylePatch& Style)
	{
		if (Style.bOverrideThumbSize)
		{
			return Style.ThumbSize.ComponentMax(FVector2D::ZeroVector);
		}
		if (Style.bOverrideThumbBrush && !Style.ThumbBrush.ImageSize.IsNearlyZero())
		{
			return FVector2D(
				FMath::Max(0.0, static_cast<double>(Style.ThumbBrush.ImageSize.X)),
				FMath::Max(0.0, static_cast<double>(Style.ThumbBrush.ImageSize.Y)));
		}
		return FVector2D(DefaultThumbExtent);
	}

	/**
	 * @brief Fills missing specialized fields with native fallback values
	 * @param ResolvedStyle Partially resolved slider style
	 * @return Complete style snapshot suitable for transitions and painting
	 */
	FLunarSliderStylePatch MaterializeStyle(const FLunarSliderStylePatch& ResolvedStyle)
	{
		FLunarSliderStylePatch Result = ResolvedStyle;
		if (!Result.bOverrideTrackBrush)
		{
			Result.bOverrideTrackBrush = true;
			Result.TrackBrush = *FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));
		}
		if (!Result.bOverrideFillBrush)
		{
			Result.bOverrideFillBrush = true;
			Result.FillBrush = *FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));
		}
		if (!Result.bOverrideThumbBrush)
		{
			Result.bOverrideThumbBrush = true;
			Result.ThumbBrush = *FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));
		}
		if (!Result.bOverrideTrackTint)
		{
			Result.bOverrideTrackTint = true;
			Result.TrackTint = DefaultTrackTint;
		}
		if (!Result.bOverrideFillTint)
		{
			Result.bOverrideFillTint = true;
			Result.FillTint = DefaultFillTint;
		}
		if (!Result.bOverrideThumbTint)
		{
			Result.bOverrideThumbTint = true;
			Result.ThumbTint = DefaultThumbTint;
		}
		if (!Result.bOverrideTrackThickness)
		{
			Result.bOverrideTrackThickness = true;
			Result.TrackThickness = DefaultTrackThickness;
		}
		if (!Result.bOverrideThumbSize)
		{
			Result.bOverrideThumbSize = true;
			Result.ThumbSize = ResolveThumbSize(ResolvedStyle);
		}
		return Result;
	}

	/**
	 * @brief Compares two materialized slider styles
	 * @param A First style
	 * @param B Second style
	 * @return True when all common and specialized fields are equivalent
	 */
	bool AreStylesEquivalent(const FLunarSliderStylePatch& A, const FLunarSliderStylePatch& B)
	{
		return LunarStyleResolver::AreCommonStylesEquivalent(A.Common, B.Common)
			&& FSlateBrush::StaticStruct()->CompareScriptStruct(&A.TrackBrush, &B.TrackBrush, 0)
			&& FSlateBrush::StaticStruct()->CompareScriptStruct(&A.FillBrush, &B.FillBrush, 0)
			&& FSlateBrush::StaticStruct()->CompareScriptStruct(&A.ThumbBrush, &B.ThumbBrush, 0)
			&& A.TrackTint == B.TrackTint
			&& A.FillTint == B.FillTint
			&& A.ThumbTint == B.ThumbTint
			&& A.TrackThickness == B.TrackThickness
			&& A.ThumbSize == B.ThumbSize;
	}

	/**
	 * @brief Interpolates continuous fields between two slider styles
	 * @param Source Transition source
	 * @param Target Transition target
	 * @param Alpha Normalized interpolation alpha
	 * @return Interpolated style snapshot
	 */
	FLunarSliderStylePatch InterpolateStyle(
		const FLunarSliderStylePatch& Source,
		const FLunarSliderStylePatch& Target,
		const float Alpha)
	{
		FLunarSliderStylePatch Result = Target;
		Result.Common = LunarStyleResolver::InterpolateCommonStylePatch(
			Source.Common,
			Target.Common,
			Alpha);
		Result.TrackTint = FMath::Lerp(Source.TrackTint, Target.TrackTint, Alpha);
		Result.FillTint = FMath::Lerp(Source.FillTint, Target.FillTint, Alpha);
		Result.ThumbTint = FMath::Lerp(Source.ThumbTint, Target.ThumbTint, Alpha);
		Result.TrackThickness = FMath::Lerp(Source.TrackThickness, Target.TrackThickness, Alpha);
		Result.ThumbSize = FMath::Lerp(Source.ThumbSize, Target.ThumbSize, Alpha);
		return Result;
	}

	/**
	 * @brief Copies non-interpolated fields from the current logical target
	 * @param InOutStyle Interpolated style to update
	 * @param LogicalTarget Style supplying brushes and common discrete fields
	 */
	void ApplyDiscreteFields(
		FLunarSliderStylePatch& InOutStyle,
		const FLunarSliderStylePatch& LogicalTarget)
	{
		LunarStyleResolver::ApplyCommonDiscreteFields(InOutStyle.Common, LogicalTarget.Common);
		InOutStyle.TrackBrush = LogicalTarget.TrackBrush;
		InOutStyle.FillBrush = LogicalTarget.FillBrush;
		InOutStyle.ThumbBrush = LogicalTarget.ThumbBrush;
	}
}

/**
 * @brief Native fallback presentation used until an owner-provided W_Slider composition is integrated
 * @ingroup LunarNavigationControls
 */
class SLunarSliderPresentation : public SLeafWidget
{
public:
	/** @cond DOXYGEN_INTERNAL */
	/** Declarative arguments containing the owning Lunar slider. */
	SLATE_BEGIN_ARGS(SLunarSliderPresentation) {}
		/** Slider whose state and style should be rendered. */
		SLATE_ARGUMENT(ULunarSlider*, Owner)
	SLATE_END_ARGS()
	/** @endcond */

	/**
	 * @brief Initializes the non-interactive fallback presentation
	 * @param InArgs Declarative arguments containing the owner
	 */
	void Construct(const FArguments& InArgs)
	{
		Owner = InArgs._Owner;
		SetCanTick(false);
		SetVisibility(EVisibility::HitTestInvisible);
#if WITH_ACCESSIBILITY
		SetAccessibleBehavior(EAccessibleBehavior::NotAccessible);
#endif
	}

	/**
	 * @brief Calculates desired size from orientation, track thickness, and thumb size
	 * @param LayoutScaleMultiplier Current Slate layout scale
	 * @return Desired local size
	 */
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override
	{
		const ULunarSlider* Slider = Owner.Get();
		if (!Slider)
		{
			return FVector2D::ZeroVector;
		}

		const FLunarSliderStylePatch& Style = Slider->bHasDisplayedSliderStyle
			? Slider->DisplayedSliderStyle
			: Slider->ResolvedSliderStyle;
		const FVector2D ThumbSize = LunarSlider_Private::ResolveThumbSize(Style);
		const float TrackThickness = Style.bOverrideTrackThickness
			? FMath::Max(0.0f, Style.TrackThickness)
			: LunarSlider_Private::DefaultTrackThickness;
		const float CrossAxisExtent = Slider->Orientation == Orient_Horizontal
			? FMath::Max(TrackThickness, ThumbSize.Y)
			: FMath::Max(TrackThickness, ThumbSize.X);
		return Slider->Orientation == Orient_Horizontal
			? FVector2D(LunarSlider_Private::DefaultSliderLength, CrossAxisExtent)
			: FVector2D(CrossAxisExtent, LunarSlider_Private::DefaultSliderLength);
	}

	/**
	 * @brief Paints the slider track, filled range, and thumb
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
		const int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		const bool bParentEnabled) const override
	{
		const ULunarSlider* Slider = Owner.Get();
		if (!Slider)
		{
			return LayerId;
		}

		const FVector2D LocalSize = AllottedGeometry.GetLocalSize();
		if (LocalSize.X <= UE_SMALL_NUMBER || LocalSize.Y <= UE_SMALL_NUMBER)
		{
			return LayerId;
		}

		const FLunarSliderStylePatch& Style = Slider->bHasDisplayedSliderStyle
			? Slider->DisplayedSliderStyle
			: Slider->ResolvedSliderStyle;
		const FSlateBrush* TrackBrush = LunarSlider_Private::ResolveBrush(
			Style.bOverrideTrackBrush,
			Style.TrackBrush);
		const FSlateBrush* FillBrush = LunarSlider_Private::ResolveBrush(
			Style.bOverrideFillBrush,
			Style.FillBrush);
		const FSlateBrush* ThumbBrush = LunarSlider_Private::ResolveBrush(
			Style.bOverrideThumbBrush,
			Style.ThumbBrush);
		const FVector2D ThumbSize = LunarSlider_Private::ResolveThumbSize(Style).ComponentMin(LocalSize);
		const float TrackThickness = FMath::Min(
			Style.bOverrideTrackThickness
				? FMath::Max(0.0f, Style.TrackThickness)
				: LunarSlider_Private::DefaultTrackThickness,
			Slider->Orientation == Orient_Horizontal ? LocalSize.Y : LocalSize.X);

		const float MinimumValue = Slider->GetMinimumValue();
		const float MaximumValue = Slider->GetMaximumValue();
		const float Range = MaximumValue - MinimumValue;
		const float NormalizedValue = Range > 0.0f
			? FMath::Clamp((Slider->PreviewValue - MinimumValue) / Range, 0.0f, 1.0f)
			: 0.0f;
		const ESlateDrawEffect DrawEffects = bParentEnabled && Slider->GetIsEnabled()
			? ESlateDrawEffect::None
			: ESlateDrawEffect::DisabledEffect;

		const FLinearColor TrackTint = LunarSlider_Private::ResolveTint(
			*TrackBrush,
			InWidgetStyle,
			Style.bOverrideTrackTint,
			Style.TrackTint,
			Style.bOverrideTrackBrush ? FLinearColor::White : LunarSlider_Private::DefaultTrackTint);
		const FLinearColor FillTint = LunarSlider_Private::ResolveTint(
			*FillBrush,
			InWidgetStyle,
			Style.bOverrideFillTint,
			Style.FillTint,
			Style.bOverrideFillBrush ? FLinearColor::White : LunarSlider_Private::DefaultFillTint);
		const FLinearColor ThumbTint = LunarSlider_Private::ResolveTint(
			*ThumbBrush,
			InWidgetStyle,
			Style.bOverrideThumbTint,
			Style.ThumbTint,
			FLinearColor::White);

		FVector2D TrackPosition;
		FVector2D TrackSize;
		FVector2D FillPosition;
		FVector2D FillSize;
		FVector2D ThumbPosition;
		if (Slider->Orientation == Orient_Horizontal)
		{
			const float TrackStart = ThumbSize.X * 0.5f;
			const float TrackEnd = FMath::Max(TrackStart, LocalSize.X - ThumbSize.X * 0.5f);
			const float TrackLength = TrackEnd - TrackStart;
			const float PhysicalFraction = Slider->bInvertValueDirection
				? 1.0f - NormalizedValue
				: NormalizedValue;
			const float ThumbCenter = TrackStart + TrackLength * PhysicalFraction;
			const float FillOrigin = Slider->bInvertValueDirection ? TrackEnd : TrackStart;

			TrackPosition = FVector2D(TrackStart, (LocalSize.Y - TrackThickness) * 0.5f);
			TrackSize = FVector2D(TrackLength, TrackThickness);
			FillPosition = FVector2D(FMath::Min(FillOrigin, ThumbCenter), TrackPosition.Y);
			FillSize = FVector2D(FMath::Abs(ThumbCenter - FillOrigin), TrackThickness);
			ThumbPosition = FVector2D(ThumbCenter - ThumbSize.X * 0.5f, (LocalSize.Y - ThumbSize.Y) * 0.5f);
		}
		else
		{
			const float TrackStart = ThumbSize.Y * 0.5f;
			const float TrackEnd = FMath::Max(TrackStart, LocalSize.Y - ThumbSize.Y * 0.5f);
			const float TrackLength = TrackEnd - TrackStart;
			const float PhysicalFraction = Slider->bInvertValueDirection
				? NormalizedValue
				: 1.0f - NormalizedValue;
			const float ThumbCenter = TrackStart + TrackLength * PhysicalFraction;
			const float FillOrigin = Slider->bInvertValueDirection ? TrackStart : TrackEnd;

			TrackPosition = FVector2D((LocalSize.X - TrackThickness) * 0.5f, TrackStart);
			TrackSize = FVector2D(TrackThickness, TrackLength);
			FillPosition = FVector2D(TrackPosition.X, FMath::Min(FillOrigin, ThumbCenter));
			FillSize = FVector2D(TrackThickness, FMath::Abs(ThumbCenter - FillOrigin));
			ThumbPosition = FVector2D((LocalSize.X - ThumbSize.X) * 0.5f, ThumbCenter - ThumbSize.Y * 0.5f);
		}

		if (TrackSize.X > UE_SMALL_NUMBER && TrackSize.Y > UE_SMALL_NUMBER)
		{
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(TrackSize, FSlateLayoutTransform(TrackPosition)),
				TrackBrush,
				DrawEffects,
				TrackTint);
		}
		if (FillSize.X > UE_SMALL_NUMBER && FillSize.Y > UE_SMALL_NUMBER)
		{
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId + 1,
				AllottedGeometry.ToPaintGeometry(FillSize, FSlateLayoutTransform(FillPosition)),
				FillBrush,
				DrawEffects,
				FillTint);
		}
		if (ThumbSize.X > UE_SMALL_NUMBER && ThumbSize.Y > UE_SMALL_NUMBER)
		{
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId + 2,
				AllottedGeometry.ToPaintGeometry(ThumbSize, FSlateLayoutTransform(ThumbPosition)),
				ThumbBrush,
				DrawEffects,
				ThumbTint);
		}
		return LayerId + 2;
	}

private:
	/** Weak owner supplying value, geometry semantics, and resolved style. */
	TWeakObjectPtr<ULunarSlider> Owner;
};

ULunarSlider::ULunarSlider(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCanReceiveLunarSelection = true;
	bCanInteractWithPointer = true;
	bEnableInputPrompt = true;

	FLunarPromptActionRequest DecreasePrompt;
	DecreasePrompt.ActionTag = LunarGameplayTags::UI_Action_Decrease.GetTag();
	PromptActions.Add(DecreasePrompt);

	FLunarPromptActionRequest IncreasePrompt;
	IncreasePrompt.ActionTag = LunarGameplayTags::UI_Action_Increase.GetTag();
	PromptActions.Add(IncreasePrompt);

	PreviewValue = Value;
	LastObservedCommitMode = CommitMode;
}

void ULunarSlider::SetValue(const float NewValue)
{
	SetCommittedValueInternal(NewValue, false);
}

float ULunarSlider::GetValue() const
{
	return Value;
}

void ULunarSlider::SetValueRange(const float NewMinValue, const float NewMaxValue)
{
	const float SafeMin = FMath::IsFinite(NewMinValue) ? NewMinValue : 0.0f;
	const float SafeMax = FMath::IsFinite(NewMaxValue) ? NewMaxValue : 1.0f;
	MinValue = FMath::Min(SafeMin, SafeMax);
	MaxValue = FMath::Max(SafeMin, SafeMax);

	const float PreviousValue = Value;
	const float PreviousPreviewValue = PreviewValue;
	Value = ClampValue(Value);
	PreviewValue = ClampValue(PreviewValue);
	bHasPendingPreview = CommitMode == ELunarSliderCommitMode::OnAccept
		&& PreviewValue != Value;
	InvalidateSliderPresentation(true);
	RefreshLunarAccessibility();

	if (PreviousValue != Value)
	{
		OnValueChanged.Broadcast(Value);
	}
	if (PreviousPreviewValue != PreviewValue)
	{
		OnPreviewValueChanged.Broadcast(PreviewValue);
		NotifyAccessibleValue(PreviewValue);
	}
}

float ULunarSlider::GetPreviewValue() const
{
	return PreviewValue;
}

void ULunarSlider::CommitPreviewValue()
{
	const float PreviousValue = Value;
	const float PreviousPreviewValue = PreviewValue;
	const float CommittedValue = ClampValue(PreviewValue);
	Value = CommittedValue;
	PreviewValue = CommittedValue;
	bHasPendingPreview = false;
	InvalidateSliderPresentation();
	RefreshLunarAccessibility();

	if (PreviousValue != CommittedValue)
	{
		OnValueChanged.Broadcast(CommittedValue);
	}
	if (PreviousPreviewValue != CommittedValue)
	{
		OnPreviewValueChanged.Broadcast(CommittedValue);
		NotifyAccessibleValue(CommittedValue);
	}
	OnValueCommitted.Broadcast(CommittedValue);
}

void ULunarSlider::CancelPreviewValue()
{
	const float PreviousPreviewValue = PreviewValue;
	PreviewValue = Value;
	bHasPendingPreview = false;
	InvalidateSliderPresentation();
	RefreshLunarAccessibility();
	if (PreviousPreviewValue != PreviewValue)
	{
		OnPreviewValueChanged.Broadcast(PreviewValue);
		NotifyAccessibleValue(PreviewValue);
	}
}

TSharedPtr<SWidget> ULunarSlider::RebuildLunarSpecializedPresentation()
{
	SAssignNew(SliderPresentation, SLunarSliderPresentation)
		.Owner(this);
	return SliderPresentation;
}

void ULunarSlider::ReleaseSlateResources(const bool bReleaseChildren)
{
	SliderPresentation.Reset();
	Super::ReleaseSlateResources(bReleaseChildren);
}

void ULunarSlider::SynchronizeProperties()
{
	NormalizeConfiguredRange();
	Value = ClampValue(Value);
	if (!bPointerInteractionActive)
	{
		PreviewValue = CommitMode == ELunarSliderCommitMode::OnAccept && IsLunarSelected()
			? ClampValue(PreviewValue)
			: Value;
	}
	bHasPendingPreview = CommitMode == ELunarSliderCommitMode::OnAccept
		&& PreviewValue != Value;
	LastObservedCommitMode = CommitMode;
	UpdateAcceptPrompt();
	Super::SynchronizeProperties();
	InvalidateSliderPresentation(true);
}

void ULunarSlider::NativeDestruct()
{
	if (bPointerInteractionActive)
	{
		CancelPointerInteraction();
	}
	Super::NativeDestruct();
}

void ULunarSlider::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	TickSliderStyleTransition(InDeltaTime);
	if (LastObservedCommitMode == CommitMode)
	{
		return;
	}

	if (!bPointerInteractionActive)
	{
		if (CommitMode == ELunarSliderCommitMode::Immediate)
		{
			CancelPreviewValue();
		}
		else
		{
			PreviewValue = Value;
			bHasPendingPreview = false;
			InvalidateSliderPresentation();
			RefreshLunarAccessibility();
		}
	}
	LastObservedCommitMode = CommitMode;
	UpdateAcceptPrompt();
}

FReply ULunarSlider::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.IsTouchEvent()
		|| !bCanInteractWithPointer
		|| InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}
	if (!NativeCanActivateLunarWidget())
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	const FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	if (Reply.IsEventHandled())
	{
		BeginPointerInteraction();
		UpdatePointerPreview(InGeometry, InMouseEvent.GetScreenSpacePosition());
	}
	return Reply;
}

FReply ULunarSlider::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!bPointerInteractionActive || InMouseEvent.IsTouchEvent())
	{
		return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
	}
	UpdatePointerPreview(InGeometry, InMouseEvent.GetScreenSpacePosition());
	return FReply::Handled();
}

FReply ULunarSlider::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!bPointerInteractionActive || InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
	}

	UpdatePointerPreview(InGeometry, InMouseEvent.GetScreenSpacePosition());
	CommitPointerInteraction();
	CancelPointerPress();
	return FReply::Handled().ReleaseMouseCapture();
}

void ULunarSlider::NativeOnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent)
{
	if (bPointerInteractionActive)
	{
		CancelPointerInteraction();
	}
	Super::NativeOnMouseCaptureLost(CaptureLostEvent);
}

FReply ULunarSlider::NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	if (!bCanInteractWithPointer
		|| !InGestureEvent.IsTouchEvent()
		|| InGestureEvent.GetPointerIndex() != 0)
	{
		return Super::NativeOnTouchStarted(InGeometry, InGestureEvent);
	}
	if (!NativeCanActivateLunarWidget())
	{
		return Super::NativeOnTouchStarted(InGeometry, InGestureEvent);
	}

	const FReply Reply = Super::NativeOnTouchStarted(InGeometry, InGestureEvent);
	if (Reply.IsEventHandled())
	{
		BeginPointerInteraction();
		UpdatePointerPreview(InGeometry, InGestureEvent.GetScreenSpacePosition());
	}
	return Reply;
}

FReply ULunarSlider::NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	if (!bPointerInteractionActive || InGestureEvent.GetPointerIndex() != 0)
	{
		return Super::NativeOnTouchMoved(InGeometry, InGestureEvent);
	}
	Super::NativeOnTouchMoved(InGeometry, InGestureEvent);
	UpdatePointerPreview(InGeometry, InGestureEvent.GetScreenSpacePosition());
	return FReply::Handled();
}

FReply ULunarSlider::NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	if (!bPointerInteractionActive || InGestureEvent.GetPointerIndex() != 0)
	{
		return Super::NativeOnTouchEnded(InGeometry, InGestureEvent);
	}

	UpdatePointerPreview(InGeometry, InGestureEvent.GetScreenSpacePosition());
	CommitPointerInteraction();
	CancelPointerPress();
	if (bCanReceiveLunarSelection)
	{
		RequestLunarSelection();
	}
	return FReply::Handled().ReleaseMouseCapture();
}

bool ULunarSlider::NativeCanHandleLunarAction(const FLunarUIActionContext& ActionContext) const
{
	if (!IsLunarSelected())
	{
		return Super::NativeCanHandleLunarAction(ActionContext);
	}

	const FGameplayTag ActionTag = ActionContext.ActionTag;
	const bool bValueAction = ActionTag == LunarGameplayTags::UI_Action_Increase.GetTag()
		|| ActionTag == LunarGameplayTags::UI_Action_Decrease.GetTag();
	if (bValueAction)
	{
		if (ActionContext.bHasNavigationDirection)
		{
			FGameplayTag ExpectedAction;
			return NativeResolveDirectionalLunarControlAction(
				ActionContext.NavigationDirection,
				ExpectedAction)
				&& ExpectedAction == ActionTag;
		}
		return true;
	}
	if (ActionTag == LunarGameplayTags::UI_Action_Accept.GetTag())
	{
		return true;
	}
	if (ActionTag == LunarGameplayTags::UI_Action_Back.GetTag()
		&& ((CommitMode == ELunarSliderCommitMode::OnAccept && bHasPendingPreview)
			|| bBackPreviewCancelPressed))
	{
		return true;
	}
	return Super::NativeCanHandleLunarAction(ActionContext);
}

ELunarUIActionResult ULunarSlider::NativeHandleLunarAction(const FLunarUIActionContext& ActionContext)
{
	const FGameplayTag ActionTag = ActionContext.ActionTag;
	const bool bValueAction = ActionTag == LunarGameplayTags::UI_Action_Increase.GetTag()
		|| ActionTag == LunarGameplayTags::UI_Action_Decrease.GetTag();
	if (bValueAction)
	{
		if (!NativeCanHandleLunarAction(ActionContext))
		{
			return ELunarUIActionResult::Unhandled;
		}
		if (!NativeCanActivateLunarWidget())
		{
			return ActionContext.InputEvent == IE_Released
				? ELunarUIActionResult::Handled
				: ELunarUIActionResult::Rejected;
		}
		if (ActionContext.InputEvent == IE_Pressed || ActionContext.InputEvent == IE_Repeat)
		{
			ApplyNavigationStep(ActionTag == LunarGameplayTags::UI_Action_Increase.GetTag());
		}
		return ELunarUIActionResult::Handled;
	}

	if (ActionTag == LunarGameplayTags::UI_Action_Accept.GetTag())
	{
		if (CommitMode == ELunarSliderCommitMode::Immediate)
		{
			return ActionContext.InputEvent == IE_Released
				? ELunarUIActionResult::Handled
				: ELunarUIActionResult::Rejected;
		}
		if (!NativeCanActivateLunarWidget())
		{
			return ActionContext.InputEvent == IE_Released
				? ELunarUIActionResult::Handled
				: ELunarUIActionResult::Rejected;
		}
		if (ActionContext.InputEvent == IE_Pressed)
		{
			CommitPreviewValue();
		}
		return ELunarUIActionResult::Handled;
	}

	if (ActionTag == LunarGameplayTags::UI_Action_Back.GetTag()
		&& ((CommitMode == ELunarSliderCommitMode::OnAccept && bHasPendingPreview)
			|| bBackPreviewCancelPressed))
	{
		if (ActionContext.InputEvent == IE_Pressed)
		{
			bBackPreviewCancelPressed = true;
			CancelPreviewValue();
		}
		else if (ActionContext.InputEvent == IE_Released)
		{
			bBackPreviewCancelPressed = false;
		}
		return ELunarUIActionResult::Handled;
	}
	return Super::NativeHandleLunarAction(ActionContext);
}

bool ULunarSlider::NativeResolveDirectionalLunarControlAction(
	const ELunarNavigationDirection Direction,
	FGameplayTag& OutActionTag) const
{
	bool bPhysicalIncrease = false;
	if (Orientation == Orient_Horizontal)
	{
		if (Direction != ELunarNavigationDirection::Left && Direction != ELunarNavigationDirection::Right)
		{
			return false;
		}
		bPhysicalIncrease = Direction == ELunarNavigationDirection::Right;
	}
	else
	{
		if (Direction != ELunarNavigationDirection::Up && Direction != ELunarNavigationDirection::Down)
		{
			return false;
		}
		bPhysicalIncrease = Direction == ELunarNavigationDirection::Up;
	}

	const bool bIncrease = bInvertValueDirection ? !bPhysicalIncrease : bPhysicalIncrease;
	OutActionTag = bIncrease
		? LunarGameplayTags::UI_Action_Increase.GetTag()
		: LunarGameplayTags::UI_Action_Decrease.GetTag();
	return true;
}

bool ULunarSlider::NativeGetLunarRepeatSettingsOverride(
	FLunarNavigationRepeatSettings& OutRepeatSettings) const
{
	if (!bOverrideRepeatSettings)
	{
		return false;
	}
	OutRepeatSettings = RepeatSettingsOverride;
	return true;
}

void ULunarSlider::NativeOnLunarSelected()
{
	PreviewValue = Value;
	bHasPendingPreview = false;
	InvalidateSliderPresentation();
	RefreshLunarAccessibility();
	Super::NativeOnLunarSelected();
}

void ULunarSlider::NativeOnLunarUnselected()
{
	bBackPreviewCancelPressed = false;
	if (bPointerInteractionActive)
	{
		CancelPointerInteraction();
	}
	if (CommitMode == ELunarSliderCommitMode::OnAccept)
	{
		CancelPreviewValue();
	}
	Super::NativeOnLunarUnselected();
}

void ULunarSlider::NativeOnInputPromptInvalidated()
{
	Super::NativeOnInputPromptInvalidated();
	UpdateAcceptPrompt();
}

FText ULunarSlider::NativeGetLunarAccessibleValueText() const
{
	return FText::Format(
		LOCTEXT("AccessibleValue", "Value {0}"),
		FText::AsNumber(PreviewValue));
}

bool ULunarSlider::ResolveCommonStylePatch(FLunarCommonStylePatch& OutStyle, FString& OutError) const
{
	FLunarSliderStylePatch ResolvedStyle;
	const bool bResolved = LunarStyleResolver::ResolveSliderStyle(
		StyleAsset,
		GetLunarVisualState(),
		StyleOverrides,
		ResolvedStyle,
		&OutError);
	if (bResolved)
	{
		const_cast<ULunarSlider*>(this)->ResolvedSliderStyle = ResolvedStyle;
		OutStyle = ResolvedStyle.Common;
	}
	return bResolved;
}

void ULunarSlider::ApplyResolvedCommonStyle(const FLunarCommonStylePatch& ResolvedStyle)
{
	Super::ApplyResolvedCommonStyle(ResolvedStyle);
	ApplySliderStyleTarget(ResolvedSliderStyle);
	InvalidateSliderPresentation(true);
}

float ULunarSlider::ClampValue(const float InValue) const
{
	const float MinimumValue = GetMinimumValue();
	const float MaximumValue = GetMaximumValue();
	return FMath::Clamp(FMath::IsFinite(InValue) ? InValue : MinimumValue, MinimumValue, MaximumValue);
}

float ULunarSlider::GetMinimumValue() const
{
	const float SafeMin = FMath::IsFinite(MinValue) ? MinValue : 0.0f;
	const float SafeMax = FMath::IsFinite(MaxValue) ? MaxValue : 1.0f;
	return FMath::Min(SafeMin, SafeMax);
}

float ULunarSlider::GetMaximumValue() const
{
	const float SafeMin = FMath::IsFinite(MinValue) ? MinValue : 0.0f;
	const float SafeMax = FMath::IsFinite(MaxValue) ? MaxValue : 1.0f;
	return FMath::Max(SafeMin, SafeMax);
}

float ULunarSlider::GetStepAmount() const
{
	const float SafeStepSize = FMath::IsFinite(StepSize) ? FMath::Abs(StepSize) : 0.0f;
	if (StepMode == ELunarSliderStepMode::Percentage)
	{
		return SafeStepSize * FMath::Max(0.0f, GetMaximumValue() - GetMinimumValue());
	}
	return SafeStepSize;
}

bool ULunarSlider::ApplyNavigationStep(const bool bIncrease)
{
	const float CurrentValue = CommitMode == ELunarSliderCommitMode::OnAccept
		? PreviewValue
		: Value;
	const float MinimumValue = GetMinimumValue();
	const float MaximumValue = GetMaximumValue();
	const float Boundary = bIncrease ? MaximumValue : MinimumValue;
	if (CurrentValue == Boundary)
	{
		return false;
	}

	const float StepAmount = GetStepAmount();
	if (StepAmount <= 0.0f)
	{
		return false;
	}
	const float UnclampedValue = CurrentValue + (bIncrease ? StepAmount : -StepAmount);
	float NewValue = bIncrease && UnclampedValue >= MaximumValue
		? MaximumValue
		: (!bIncrease && UnclampedValue <= MinimumValue ? MinimumValue : ClampValue(UnclampedValue));
	if (NewValue == CurrentValue)
	{
		// A valid positive step can be smaller than one ULP at large magnitudes.
		// Still advance by the next representable value so repeat cannot stall forever.
		NewValue = std::nextafter(CurrentValue, Boundary);
		if (NewValue == CurrentValue)
		{
			return false;
		}
	}

	if (CommitMode == ELunarSliderCommitMode::OnAccept)
	{
		SetPreviewValueInternal(NewValue);
	}
	else
	{
		SetCommittedValueInternal(NewValue, true);
	}
	return true;
}

void ULunarSlider::SetPreviewValueInternal(const float NewPreviewValue)
{
	const float ClampedValue = ClampValue(NewPreviewValue);
	if (PreviewValue == ClampedValue)
	{
		return;
	}
	PreviewValue = ClampedValue;
	bHasPendingPreview = CommitMode == ELunarSliderCommitMode::OnAccept
		&& PreviewValue != Value;
	InvalidateSliderPresentation();
	RefreshLunarAccessibility();
	OnPreviewValueChanged.Broadcast(PreviewValue);
	NotifyAccessibleValue(PreviewValue);
}

void ULunarSlider::SetCommittedValueInternal(const float NewValue, const bool bBroadcastCommitted)
{
	const float ClampedValue = ClampValue(NewValue);
	const float PreviousValue = Value;
	const float PreviousPreviewValue = PreviewValue;
	Value = ClampedValue;
	PreviewValue = ClampedValue;
	bHasPendingPreview = false;
	InvalidateSliderPresentation();
	RefreshLunarAccessibility();

	if (PreviousValue != ClampedValue)
	{
		OnValueChanged.Broadcast(ClampedValue);
	}
	if (PreviousPreviewValue != ClampedValue)
	{
		OnPreviewValueChanged.Broadcast(ClampedValue);
		NotifyAccessibleValue(ClampedValue);
	}
	if (bBroadcastCommitted)
	{
		OnValueCommitted.Broadcast(ClampedValue);
	}
}

void ULunarSlider::NormalizeConfiguredRange()
{
	MinValue = FMath::IsFinite(MinValue) ? MinValue : 0.0f;
	MaxValue = FMath::IsFinite(MaxValue) ? MaxValue : 1.0f;
	if (MinValue > MaxValue)
	{
		Swap(MinValue, MaxValue);
	}
	StepSize = FMath::IsFinite(StepSize) ? FMath::Max(0.0f, StepSize) : 0.0f;
}

void ULunarSlider::UpdateAcceptPrompt()
{
	if (bSynchronizingAcceptPrompt)
	{
		return;
	}
	TGuardValue<bool> SynchronizingPromptGuard(bSynchronizingAcceptPrompt, true);
	TArray<FLunarPromptActionRequest> UpdatedPromptActions = PromptActions;
	const FGameplayTag AcceptTag = LunarGameplayTags::UI_Action_Accept.GetTag();
	const bool bHasAcceptPrompt = UpdatedPromptActions.ContainsByPredicate(
		[AcceptTag](const FLunarPromptActionRequest& Request)
		{
			return Request.ActionTag == AcceptTag;
		});

	if (CommitMode == ELunarSliderCommitMode::OnAccept && !bHasAcceptPrompt)
	{
		FLunarPromptActionRequest AcceptPrompt;
		AcceptPrompt.ActionTag = AcceptTag;
		UpdatedPromptActions.Add(AcceptPrompt);
	}
	else if (CommitMode == ELunarSliderCommitMode::Immediate && bHasAcceptPrompt)
	{
		UpdatedPromptActions.RemoveAll(
			[AcceptTag](const FLunarPromptActionRequest& Request)
			{
				return Request.ActionTag == AcceptTag;
			});
	}

	if (UpdatedPromptActions.Num() != PromptActions.Num())
	{
		SetPromptActions(UpdatedPromptActions);
	}
}

void ULunarSlider::BeginPointerInteraction()
{
	if (bPointerInteractionActive)
	{
		CancelPointerInteraction();
	}
	bPointerInteractionActive = true;
	PointerStartCommittedValue = Value;
	PointerStartPreviewValue = PreviewValue;
	bPointerStartHadPendingPreview = bHasPendingPreview;
}

void ULunarSlider::UpdatePointerPreview(
	const FGeometry& InGeometry,
	const FVector2D& ScreenSpacePosition)
{
	if (!bPointerInteractionActive)
	{
		return;
	}
	FGeometry InteractionGeometry = InGeometry;
	if (SliderPresentation.IsValid())
	{
		const FGeometry& PresentationGeometry = SliderPresentation->GetCachedGeometry();
		if (PresentationGeometry.GetLocalSize().X > 0.0f
			&& PresentationGeometry.GetLocalSize().Y > 0.0f)
		{
			InteractionGeometry = PresentationGeometry;
		}
	}

	const FVector2D LocalSize = InteractionGeometry.GetLocalSize();
	if (LocalSize.X <= UE_SMALL_NUMBER || LocalSize.Y <= UE_SMALL_NUMBER)
	{
		return;
	}

	const FLunarSliderStylePatch& Style = bHasDisplayedSliderStyle
		? DisplayedSliderStyle
		: ResolvedSliderStyle;
	const FVector2D ThumbSize = LunarSlider_Private::ResolveThumbSize(Style).ComponentMin(LocalSize);
	const FVector2D LocalPosition = InteractionGeometry.AbsoluteToLocal(ScreenSpacePosition);
	const float AxisExtent = Orientation == Orient_Horizontal ? LocalSize.X : LocalSize.Y;
	const float ThumbExtent = Orientation == Orient_Horizontal ? ThumbSize.X : ThumbSize.Y;
	const float TrackStart = ThumbExtent * 0.5f;
	const float TrackEnd = FMath::Max(TrackStart, AxisExtent - ThumbExtent * 0.5f);
	const float TrackLength = TrackEnd - TrackStart;
	if (TrackLength <= 0.0f)
	{
		return;
	}
	const float AxisPosition = Orientation == Orient_Horizontal ? LocalPosition.X : LocalPosition.Y;
	const float PhysicalFraction = FMath::Clamp((AxisPosition - TrackStart) / TrackLength, 0.0f, 1.0f);
	float Fraction = Orientation == Orient_Horizontal ? PhysicalFraction : 1.0f - PhysicalFraction;
	if (bInvertValueDirection)
	{
		Fraction = 1.0f - Fraction;
	}
	SetPreviewValueInternal(FMath::Lerp(GetMinimumValue(), GetMaximumValue(), Fraction));
}

void ULunarSlider::CommitPointerInteraction()
{
	if (!bPointerInteractionActive)
	{
		return;
	}
	bPointerInteractionActive = false;
	CommitPreviewValue();
}

void ULunarSlider::CancelPointerInteraction()
{
	if (!bPointerInteractionActive)
	{
		return;
	}
	bPointerInteractionActive = false;
	const float PreviousValue = Value;
	const float PreviousPreviewValue = PreviewValue;
	Value = ClampValue(PointerStartCommittedValue);
	PreviewValue = ClampValue(PointerStartPreviewValue);
	bHasPendingPreview = CommitMode == ELunarSliderCommitMode::OnAccept
		&& bPointerStartHadPendingPreview
		&& PreviewValue != Value;
	InvalidateSliderPresentation();
	RefreshLunarAccessibility();

	if (PreviousValue != Value)
	{
		OnValueChanged.Broadcast(Value);
	}
	if (PreviousPreviewValue != PreviewValue)
	{
		OnPreviewValueChanged.Broadcast(PreviewValue);
		NotifyAccessibleValue(PreviewValue);
	}
}

void ULunarSlider::ApplySliderStyleTarget(const FLunarSliderStylePatch& NewTarget)
{
	FLunarSliderStylePatch CompleteTarget = NewTarget;
	CompleteTarget.Common = MaterializeCommonStyleSnapshot(NewTarget.Common);
	const FLunarSliderStylePatch MaterializedTarget = LunarSlider_Private::MaterializeStyle(CompleteTarget);
	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	const bool bReduceMotion = Settings && Settings->Navigation.Accessibility.bReduceMotion;
	if (bReduceMotion)
	{
		bSliderStyleTransitionActive = false;
		bSliderStyleTransitionReversing = false;
		TransitionSourceSliderStyle = MaterializedTarget;
		TransitionTargetSliderStyle = MaterializedTarget;
		LogicalTargetSliderStyle = MaterializedTarget;
		ApplyDisplayedSliderStyle(MaterializedTarget);
		return;
	}

	if (bHasDisplayedSliderStyle
		&& LunarSlider_Private::AreStylesEquivalent(LogicalTargetSliderStyle, MaterializedTarget))
	{
		return;
	}

	if (bSliderStyleTransitionActive)
	{
		const bool bReturnsToSource = !bSliderStyleTransitionReversing
			&& LunarSlider_Private::AreStylesEquivalent(TransitionSourceSliderStyle, MaterializedTarget);
		const bool bReturnsToForwardTarget = bSliderStyleTransitionReversing
			&& LunarSlider_Private::AreStylesEquivalent(TransitionTargetSliderStyle, MaterializedTarget);
		if (bReturnsToSource || bReturnsToForwardTarget)
		{
			LogicalTargetSliderStyle = MaterializedTarget;
			bSliderStyleTransitionReversing = bReturnsToSource;
			FLunarSliderStylePatch Snapshot = DisplayedSliderStyle;
			LunarSlider_Private::ApplyDiscreteFields(
				Snapshot,
				bSliderStyleTransitionReversing ? TransitionSourceSliderStyle : TransitionTargetSliderStyle);
			ApplyDisplayedSliderStyle(Snapshot);
			return;
		}
	}

	const bool bCanTransition = bHasDisplayedSliderStyle
		&& MaterializedTarget.Common.Transition.bEnabled
		&& MaterializedTarget.Common.Transition.Duration > 0.0f;
	if (!bCanTransition)
	{
		bSliderStyleTransitionActive = false;
		bSliderStyleTransitionReversing = false;
		TransitionSourceSliderStyle = MaterializedTarget;
		TransitionTargetSliderStyle = MaterializedTarget;
		LogicalTargetSliderStyle = MaterializedTarget;
		ApplyDisplayedSliderStyle(MaterializedTarget);
		return;
	}

	TransitionSourceSliderStyle = DisplayedSliderStyle;
	TransitionTargetSliderStyle = MaterializedTarget;
	LogicalTargetSliderStyle = MaterializedTarget;
	SliderStyleTransitionElapsed = 0.0f;
	SliderStyleTransitionDuration = MaterializedTarget.Common.Transition.Duration;
	bSliderStyleTransitionActive = true;
	bSliderStyleTransitionReversing = false;
	ApplyDisplayedSliderStyle(LunarSlider_Private::InterpolateStyle(
		TransitionSourceSliderStyle,
		TransitionTargetSliderStyle,
		0.0f));
}

void ULunarSlider::ApplyDisplayedSliderStyle(const FLunarSliderStylePatch& NewDisplayedStyle)
{
	DisplayedSliderStyle = NewDisplayedStyle;
	bHasDisplayedSliderStyle = true;
	InvalidateSliderPresentation(true);
}

void ULunarSlider::TickSliderStyleTransition(const float DeltaTime)
{
	if (!bSliderStyleTransitionActive || SliderStyleTransitionDuration <= 0.0f)
	{
		return;
	}
	if (const ULunarSettings* Settings = GetDefault<ULunarSettings>();
		Settings && Settings->Navigation.Accessibility.bReduceMotion)
	{
		bSliderStyleTransitionActive = false;
		bSliderStyleTransitionReversing = false;
		ApplyDisplayedSliderStyle(LogicalTargetSliderStyle);
		return;
	}

	const float Step = FMath::Max(0.0f, DeltaTime);
	SliderStyleTransitionElapsed += bSliderStyleTransitionReversing ? -Step : Step;
	SliderStyleTransitionElapsed = FMath::Clamp(
		SliderStyleTransitionElapsed,
		0.0f,
		SliderStyleTransitionDuration);
	const float Alpha = SliderStyleTransitionElapsed / SliderStyleTransitionDuration;
	const float EasedAlpha = static_cast<float>(UKismetMathLibrary::Ease(
		0.0,
		1.0,
		Alpha,
		TransitionTargetSliderStyle.Common.Transition.Easing));
	FLunarSliderStylePatch Snapshot = LunarSlider_Private::InterpolateStyle(
		TransitionSourceSliderStyle,
		TransitionTargetSliderStyle,
		EasedAlpha);
	if (bSliderStyleTransitionReversing)
	{
		LunarSlider_Private::ApplyDiscreteFields(Snapshot, TransitionSourceSliderStyle);
	}
	ApplyDisplayedSliderStyle(Snapshot);

	const bool bFinished = bSliderStyleTransitionReversing ? Alpha <= 0.0f : Alpha >= 1.0f;
	if (bFinished)
	{
		ApplyDisplayedSliderStyle(
			bSliderStyleTransitionReversing ? TransitionSourceSliderStyle : TransitionTargetSliderStyle);
		bSliderStyleTransitionActive = false;
		bSliderStyleTransitionReversing = false;
	}
}

void ULunarSlider::InvalidateSliderPresentation(const bool bInvalidateLayout) const
{
	if (SliderPresentation.IsValid())
	{
		SliderPresentation->Invalidate(
			bInvalidateLayout
				? EInvalidateWidgetReason::Layout | EInvalidateWidgetReason::Paint
				: EInvalidateWidgetReason::Paint);
	}
}

void ULunarSlider::NotifyAccessibleValue(const float NewValue)
{
	NotifyLunarAccessibleValueChanged(FText::Format(
		LOCTEXT("AccessibleValueChanged", "Value {0}"),
		FText::AsNumber(NewValue)));
}

#undef LOCTEXT_NAMESPACE
