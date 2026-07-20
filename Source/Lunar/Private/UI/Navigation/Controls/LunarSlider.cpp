// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "UI/Navigation/Controls/LunarSlider.h"

#include "Curves/CurveFloat.h"
#include "Engine/LocalPlayer.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "UI/Navigation/Core/LunarNavigationSubsystem.h"
#include "UI/Navigation/Types/LunarGameplayTags.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/SOverlay.h"

#include <cmath>

/**
 * @file LunarSlider.cpp
 * @brief Continuous Lunar slider behavior and native-part rendering
 * @ingroup LunarNavigationControls
 */

#define LOCTEXT_NAMESPACE "LunarSlider"

namespace LunarSlider_Private
{
	/** Default desired length of the neutral native presentation. */
	constexpr float DefaultSliderLength = 100.0f;

	/** @param Brush Brush contributing tint. @param WidgetStyle Inherited style. @param Tint Authored tint. @return Final paint tint. */
	FLinearColor ResolveTint(const FSlateBrush& Brush, const FWidgetStyle& WidgetStyle, const FLinearColor& Tint)
	{
		return Brush.GetTint(WidgetStyle) * Tint * WidgetStyle.GetColorAndOpacityTint();
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
		/** Slider whose value and native parts should be rendered. */
		SLATE_ARGUMENT(ULunarSlider*, Owner)
	SLATE_END_ARGS()
	/** @endcond */

	/**
	 * @brief Initializes the native slider presentation and pointer hit target
	 * @param InArgs Declarative arguments containing the owner
	 */
	void Construct(const FArguments& InArgs)
	{
		Owner = InArgs._Owner;
		SetCanTick(false);
		// SOverlay defaults to SelfHitTestInvisible. The native leaf must therefore remain
		// hit-testable so an otherwise empty Slider Blueprint can receive pointer input.
		SetVisibility(EVisibility::Visible);
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

		const FVector2D ThumbSize = Slider->ThumbSize.ComponentMax(FVector2D::ZeroVector);
		const float TrackThickness = FMath::Max(0.0f, Slider->TrackThickness);
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

		const FSlateBrush* TrackBrush = &Slider->TrackBrush;
		const FSlateBrush* FillBrush = &Slider->FillBrush;
		const FSlateBrush* ThumbBrush = &Slider->ThumbBrush;
		const FVector2D ThumbSize = Slider->ThumbSize.ComponentMax(FVector2D::ZeroVector).ComponentMin(LocalSize);
		const float TrackThickness = FMath::Min(
			FMath::Max(0.0f, Slider->TrackThickness),
			Slider->Orientation == Orient_Horizontal ? LocalSize.Y : LocalSize.X);

		const float MinimumValue = Slider->GetMinimumValue();
		const float MaximumValue = Slider->GetMaximumValue();
		const float Range = MaximumValue - MinimumValue;
		const float NormalizedValue = Range > 0.0f
			? FMath::Clamp((Slider->GetDisplayedValue() - MinimumValue) / Range, 0.0f, 1.0f)
			: 0.0f;
		const ESlateDrawEffect DrawEffects = ESlateDrawEffect::None;
		const FLinearColor TrackTint = LunarSlider_Private::ResolveTint(*TrackBrush, InWidgetStyle, Slider->TrackTint);
		const FLinearColor FillTint = LunarSlider_Private::ResolveTint(*FillBrush, InWidgetStyle, Slider->FillTint);
		const FLinearColor ThumbTint = LunarSlider_Private::ResolveTint(*ThumbBrush, InWidgetStyle, Slider->ThumbTint);

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
	/** Weak owner supplying value, geometry semantics, and native-part presentation. */
	TWeakObjectPtr<ULunarSlider> Owner;
};

ULunarSlider::ULunarSlider(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCanReceiveLunarSelection = true;
	bCanInteractWithPointer = true;
	bEnableInputPrompt = true;
	TrackBrush = *FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));
	FillBrush = TrackBrush;
	ThumbBrush = TrackBrush;

	static ConstructorHelpers::FObjectFinder<UCurveFloat> DefaultInterpolationCurve(
		TEXT("/Lunar/Curves/Float/CF_EaseOutExpo.CF_EaseOutExpo"));
	if (DefaultInterpolationCurve.Succeeded())
	{
		ValueInterpolationCurve = DefaultInterpolationCurve.Object;
	}

	FLunarPromptActionRequest DecreasePrompt;
	DecreasePrompt.ActionTag = LunarGameplayTags::UI_Action_Decrease.GetTag();
	PromptActions.Add(DecreasePrompt);

	FLunarPromptActionRequest IncreasePrompt;
	IncreasePrompt.ActionTag = LunarGameplayTags::UI_Action_Increase.GetTag();
	PromptActions.Add(IncreasePrompt);

	PreviewValue = Value;
	LastObservedCommitMode = CommitMode;
}

void ULunarSlider::SetTrackBrush(const FSlateBrush& NewBrush)
{
	TrackBrush = NewBrush;
	InvalidateSliderPresentation(true);
}

void ULunarSlider::SetTrackTint(const FLinearColor NewTint)
{
	TrackTint = NewTint;
	InvalidateSliderPresentation();
}

void ULunarSlider::SetTrackThickness(const float NewThickness)
{
	TrackThickness = FMath::Max(0.0f, NewThickness);
	InvalidateSliderPresentation(true);
}

void ULunarSlider::SetFillBrush(const FSlateBrush& NewBrush)
{
	FillBrush = NewBrush;
	InvalidateSliderPresentation(true);
}

void ULunarSlider::SetFillTint(const FLinearColor NewTint)
{
	FillTint = NewTint;
	InvalidateSliderPresentation();
}

void ULunarSlider::SetThumbBrush(const FSlateBrush& NewBrush)
{
	ThumbBrush = NewBrush;
	InvalidateSliderPresentation(true);
}

void ULunarSlider::SetThumbTint(const FLinearColor NewTint)
{
	ThumbTint = NewTint;
	InvalidateSliderPresentation();
}

void ULunarSlider::SetThumbSize(const FVector2D NewSize)
{
	ThumbSize = NewSize.ComponentMax(FVector2D::ZeroVector);
	InvalidateSliderPresentation(true);
}

void ULunarSlider::ConfigureTrackPresentation(const FSlateBrush& NewBrush, const FLinearColor NewTint, const float NewThickness)
{
	TrackBrush = NewBrush;
	TrackTint = NewTint;
	TrackThickness = FMath::Max(0.0f, NewThickness);
	InvalidateSliderPresentation(true);
}

void ULunarSlider::GetTrackPresentation(FSlateBrush& OutBrush, FLinearColor& OutTint, float& OutThickness) const
{
	OutBrush = TrackBrush;
	OutTint = TrackTint;
	OutThickness = TrackThickness;
}

void ULunarSlider::ConfigureFillPresentation(const FSlateBrush& NewBrush, const FLinearColor NewTint)
{
	FillBrush = NewBrush;
	FillTint = NewTint;
	InvalidateSliderPresentation(true);
}

void ULunarSlider::GetFillPresentation(FSlateBrush& OutBrush, FLinearColor& OutTint) const
{
	OutBrush = FillBrush;
	OutTint = FillTint;
}

void ULunarSlider::ConfigureThumbPresentation(const FSlateBrush& NewBrush, const FLinearColor NewTint, const FVector2D NewSize)
{
	ThumbBrush = NewBrush;
	ThumbTint = NewTint;
	ThumbSize = NewSize.ComponentMax(FVector2D::ZeroVector);
	InvalidateSliderPresentation(true);
}

void ULunarSlider::GetThumbPresentation(FSlateBrush& OutBrush, FLinearColor& OutTint, FVector2D& OutSize) const
{
	OutBrush = ThumbBrush;
	OutTint = ThumbTint;
	OutSize = ThumbSize;
}

void ULunarSlider::ConfigureSliderPresentation(
	const FSlateBrush& NewTrackBrush,
	const FLinearColor NewTrackTint,
	const FSlateBrush& NewFillBrush,
	const FLinearColor NewFillTint,
	const FSlateBrush& NewThumbBrush,
	const FLinearColor NewThumbTint,
	const float NewTrackThickness,
	const FVector2D NewThumbSize)
{
	TrackBrush = NewTrackBrush;
	TrackTint = NewTrackTint;
	FillBrush = NewFillBrush;
	FillTint = NewFillTint;
	ThumbBrush = NewThumbBrush;
	ThumbTint = NewThumbTint;
	TrackThickness = FMath::Max(0.0f, NewTrackThickness);
	ThumbSize = NewThumbSize.ComponentMax(FVector2D::ZeroVector);
	InvalidateSliderPresentation(true);
}

void ULunarSlider::GetSliderPresentation(
	FSlateBrush& OutTrackBrush,
	FLinearColor& OutTrackTint,
	FSlateBrush& OutFillBrush,
	FLinearColor& OutFillTint,
	FSlateBrush& OutThumbBrush,
	FLinearColor& OutThumbTint,
	float& OutTrackThickness,
	FVector2D& OutThumbSize) const
{
	OutTrackBrush = TrackBrush;
	OutTrackTint = TrackTint;
	OutFillBrush = FillBrush;
	OutFillTint = FillTint;
	OutThumbBrush = ThumbBrush;
	OutThumbTint = ThumbTint;
	OutTrackThickness = TrackThickness;
	OutThumbSize = ThumbSize;
}
void ULunarSlider::SetValue(
	const float NewValue,
	const ELunarChangeNotificationPolicy NotificationPolicy)
{
	SetCommittedValueInternal(
		NewValue,
		false,
		NotificationPolicy == ELunarChangeNotificationPolicy::Notify);
}

float ULunarSlider::GetValue() const
{
	return Value;
}

void ULunarSlider::SetValueRange(
	const float NewMinValue,
	const float NewMaxValue,
	const ELunarChangeNotificationPolicy NotificationPolicy)
{
	const bool bNotify = NotificationPolicy == ELunarChangeNotificationPolicy::Notify;
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
	RefreshDisplayedValueTarget(true);
	InvalidateSliderPresentation(true);
	RefreshLunarAccessibility();

	if (bNotify && PreviousValue != Value)
	{
		OnValueChanged.Broadcast(this, Value);
	}
	if (bNotify && PreviousPreviewValue != PreviewValue)
	{
		OnPreviewValueChanged.Broadcast(this, PreviewValue);
		NotifyAccessibleValue(PreviewValue);
	}
}

float ULunarSlider::GetPreviewValue() const
{
	return PreviewValue;
}

float ULunarSlider::GetDisplayedValue() const
{
	return bDisplayedValueInitialized ? DisplayedValue : PreviewValue;
}

void ULunarSlider::CommitPreviewValue(const ELunarChangeNotificationPolicy NotificationPolicy)
{
	const bool bNotify = NotificationPolicy == ELunarChangeNotificationPolicy::Notify;
	const float PreviousValue = Value;
	const float PreviousPreviewValue = PreviewValue;
	const float CommittedValue = ClampValue(PreviewValue);
	Value = CommittedValue;
	PreviewValue = CommittedValue;
	bHasPendingPreview = false;
	RefreshDisplayedValueTarget(true);
	InvalidateSliderPresentation();
	RefreshLunarAccessibility();

	if (bNotify && PreviousValue != CommittedValue)
	{
		OnValueChanged.Broadcast(this, CommittedValue);
	}
	if (bNotify && PreviousPreviewValue != CommittedValue)
	{
		OnPreviewValueChanged.Broadcast(this, CommittedValue);
		NotifyAccessibleValue(CommittedValue);
	}
	if (bNotify)
	{
		OnValueCommitted.Broadcast(this, CommittedValue);
	}
}

void ULunarSlider::CancelPreviewValue(const ELunarChangeNotificationPolicy NotificationPolicy)
{
	const bool bNotify = NotificationPolicy == ELunarChangeNotificationPolicy::Notify;
	const float PreviousPreviewValue = PreviewValue;
	PreviewValue = Value;
	bHasPendingPreview = false;
	RefreshDisplayedValueTarget(true);
	InvalidateSliderPresentation();
	RefreshLunarAccessibility();
	if (bNotify && PreviousPreviewValue != PreviewValue)
	{
		OnPreviewValueChanged.Broadcast(this, PreviewValue);
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
	RefreshDisplayedValueTarget(false);
	InvalidateSliderPresentation(true);
}

void ULunarSlider::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	UpdateContinuousStickInput(InDeltaTime);
	UpdateDisplayedValueInterpolation(InDeltaTime);
}

void ULunarSlider::NativeDestruct()
{
	if (bPointerInteractionActive)
	{
		CancelPointerInteraction();
	}
	Super::NativeDestruct();
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
	if (!bAllowTouchInput
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
		const bool bContinuousStickAction = StickInputMode == ELunarSliderStickInputMode::Continuous
			&& ActionContext.InputDevice == ELunarInputDeviceType::Gamepad
			&& ActionContext.AnalogMagnitude > 0.0f;
		if ((ActionContext.InputEvent == IE_Pressed || ActionContext.InputEvent == IE_Repeat)
			&& !bContinuousStickAction)
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
	RefreshDisplayedValueTarget(true);
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

float ULunarSlider::SnapPointerValue(const float InValue) const
{
	const float ClampedValue = ClampValue(InValue);
	const float SafeStepSize = FMath::IsFinite(PointerStepSize)
		? FMath::Max(0.0f, PointerStepSize)
		: 0.0f;
	if (SafeStepSize <= 0.0f)
	{
		return ClampedValue;
	}

	const float MinimumValue = GetMinimumValue();
	const float MaximumValue = GetMaximumValue();
	if (ClampedValue <= MinimumValue || ClampedValue >= MaximumValue)
	{
		return ClampedValue;
	}

	const float RelativeValue = ClampedValue - MinimumValue;
	const float LowerValue = ClampValue(
		MinimumValue + FMath::FloorToFloat(RelativeValue / SafeStepSize) * SafeStepSize);
	const float UpperValue = FMath::Min(MaximumValue, LowerValue + SafeStepSize);
	return ClampedValue - LowerValue < UpperValue - ClampedValue
		? LowerValue
		: UpperValue;
}

void ULunarSlider::UpdateContinuousStickInput(const float DeltaTime)
{
	if (StickInputMode != ELunarSliderStickInputMode::Continuous
		|| bPointerInteractionActive
		|| !IsLunarSelected()
		|| !NativeCanActivateLunarWidget()
		|| !FMath::IsFinite(DeltaTime)
		|| DeltaTime <= 0.0f
		|| !FMath::IsFinite(ContinuousStickRangePerSecond)
		|| ContinuousStickRangePerSecond <= 0.0f)
	{
		return;
	}

	const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	const ULunarNavigationSubsystem* NavigationSubsystem = LocalPlayer
		? LocalPlayer->GetSubsystem<ULunarNavigationSubsystem>()
		: nullptr;
	ELunarNavigationDirection Direction = ELunarNavigationDirection::Up;
	float Magnitude = 0.0f;
	if (!NavigationSubsystem
		|| !NavigationSubsystem->GetActiveAnalogNavigationForWidget(this, Direction, Magnitude))
	{
		return;
	}

	FGameplayTag ActionTag;
	if (!NativeResolveDirectionalLunarControlAction(Direction, ActionTag))
	{
		return;
	}
	const bool bIncrease = ActionTag == LunarGameplayTags::UI_Action_Increase.GetTag();
	const float CurrentValue = CommitMode == ELunarSliderCommitMode::OnAccept ? PreviewValue : Value;
	const float Boundary = bIncrease ? GetMaximumValue() : GetMinimumValue();
	if (CurrentValue == Boundary)
	{
		return;
	}

	const float Range = FMath::Max(0.0f, GetMaximumValue() - GetMinimumValue());
	const float DeltaValue = Range
		* ContinuousStickRangePerSecond
		* FMath::Clamp(Magnitude, 0.0f, 1.0f)
		* DeltaTime;
	if (!FMath::IsFinite(DeltaValue) || DeltaValue <= 0.0f)
	{
		return;
	}

	const float UnclampedValue = CurrentValue + (bIncrease ? DeltaValue : -DeltaValue);
	float NewValue = bIncrease && UnclampedValue >= GetMaximumValue()
		? GetMaximumValue()
		: (!bIncrease && UnclampedValue <= GetMinimumValue() ? GetMinimumValue() : ClampValue(UnclampedValue));
	if (NewValue == CurrentValue)
	{
		NewValue = std::nextafter(CurrentValue, Boundary);
		if (NewValue == CurrentValue)
		{
			return;
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
}

void ULunarSlider::UpdateDisplayedValueInterpolation(const float DeltaTime)
{
	RefreshDisplayedValueTarget(true);
	if (!bDisplayedValueInterpolationActive
		|| !FMath::IsFinite(DeltaTime)
		|| DeltaTime <= 0.0f)
	{
		return;
	}

	DisplayedValueInterpolationElapsed += DeltaTime;
	const float Progress = FMath::Clamp(
		DisplayedValueInterpolationElapsed * ValueInterpolationSpeed,
		0.0f,
		1.0f);
	float CurveAlpha = Progress;
	if (IsValid(ValueInterpolationCurve))
	{
		const float EvaluatedAlpha = ValueInterpolationCurve->GetFloatValue(Progress);
		if (FMath::IsFinite(EvaluatedAlpha))
		{
			CurveAlpha = EvaluatedAlpha;
		}
	}

	const float NewDisplayedValue = Progress >= 1.0f
		? DisplayedValueInterpolationTarget
		: FMath::Lerp(
			DisplayedValueInterpolationSource,
			DisplayedValueInterpolationTarget,
			CurveAlpha);
	if (Progress >= 1.0f)
	{
		bDisplayedValueInterpolationActive = false;
	}
	SetDisplayedValueInternal(NewDisplayedValue, true);
}

void ULunarSlider::RefreshDisplayedValueTarget(const bool bBroadcast)
{
	const float ClampedTarget = ClampValue(PreviewValue);
	const bool bMustSnap = !bDisplayedValueInitialized
		|| IsDesignTime()
		|| !bInterpolateValueChanges
		|| !FMath::IsFinite(ValueInterpolationSpeed)
		|| ValueInterpolationSpeed <= 0.0f;
	if (bMustSnap)
	{
		DisplayedValueInterpolationSource = ClampedTarget;
		DisplayedValueInterpolationTarget = ClampedTarget;
		DisplayedValueInterpolationElapsed = 0.0f;
		bDisplayedValueInterpolationActive = false;
		SetDisplayedValueInternal(ClampedTarget, bBroadcast);
		return;
	}

	if (!bDisplayedValueInterpolationActive
		|| DisplayedValueInterpolationTarget != ClampedTarget)
	{
		DisplayedValueInterpolationSource = GetDisplayedValue();
		DisplayedValueInterpolationTarget = ClampedTarget;
		DisplayedValueInterpolationElapsed = 0.0f;
		bDisplayedValueInterpolationActive = DisplayedValueInterpolationSource != ClampedTarget;
	}
}

void ULunarSlider::SetDisplayedValueInternal(const float NewDisplayedValue, const bool bBroadcast)
{
	const float ClampedValue = ClampValue(NewDisplayedValue);
	if (bDisplayedValueInitialized && DisplayedValue == ClampedValue)
	{
		return;
	}

	DisplayedValue = ClampedValue;
	bDisplayedValueInitialized = true;
	InvalidateSliderPresentation();
	if (bBroadcast)
	{
		OnDisplayedValueChanged.Broadcast(this, DisplayedValue);
	}
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
	RefreshDisplayedValueTarget(true);
	InvalidateSliderPresentation();
	RefreshLunarAccessibility();
	OnPreviewValueChanged.Broadcast(this, PreviewValue);
	NotifyAccessibleValue(PreviewValue);
}

void ULunarSlider::SetCommittedValueInternal(
	const float NewValue,
	const bool bBroadcastCommitted,
	const bool bNotifyChange)
{
	const float ClampedValue = ClampValue(NewValue);
	const float PreviousValue = Value;
	const float PreviousPreviewValue = PreviewValue;
	Value = ClampedValue;
	PreviewValue = ClampedValue;
	bHasPendingPreview = false;
	RefreshDisplayedValueTarget(true);
	InvalidateSliderPresentation();
	RefreshLunarAccessibility();

	if (bNotifyChange && PreviousValue != ClampedValue)
	{
		OnValueChanged.Broadcast(this, ClampedValue);
	}
	if (bNotifyChange && PreviousPreviewValue != ClampedValue)
	{
		OnPreviewValueChanged.Broadcast(this, ClampedValue);
		NotifyAccessibleValue(ClampedValue);
	}
	if (bNotifyChange && bBroadcastCommitted)
	{
		OnValueCommitted.Broadcast(this, ClampedValue);
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
	ContinuousStickRangePerSecond = FMath::IsFinite(ContinuousStickRangePerSecond)
		? FMath::Max(0.0f, ContinuousStickRangePerSecond)
		: 0.0f;
	PointerStepSize = FMath::IsFinite(PointerStepSize) ? FMath::Max(0.0f, PointerStepSize) : 0.0f;
	ValueInterpolationSpeed = FMath::IsFinite(ValueInterpolationSpeed)
		? FMath::Max(0.0f, ValueInterpolationSpeed)
		: 0.0f;
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

	const FVector2D EffectiveThumbSize = ThumbSize.ComponentMax(FVector2D::ZeroVector).ComponentMin(LocalSize);
	const FVector2D LocalPosition = InteractionGeometry.AbsoluteToLocal(ScreenSpacePosition);
	const float AxisExtent = Orientation == Orient_Horizontal ? LocalSize.X : LocalSize.Y;
	const float ThumbExtent = Orientation == Orient_Horizontal ? EffectiveThumbSize.X : EffectiveThumbSize.Y;
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
	SetPreviewValueInternal(SnapPointerValue(
		FMath::Lerp(GetMinimumValue(), GetMaximumValue(), Fraction)));
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
	RefreshDisplayedValueTarget(true);
	InvalidateSliderPresentation();
	RefreshLunarAccessibility();

	if (PreviousValue != Value)
	{
		OnValueChanged.Broadcast(this, Value);
	}
	if (PreviousPreviewValue != PreviewValue)
	{
		OnPreviewValueChanged.Broadcast(this, PreviewValue);
		NotifyAccessibleValue(PreviewValue);
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
