// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "UI/Navigation/Controls/LunarScrollBox.h"

#include "Components/ScrollBoxSlot.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "Kismet/KismetMathLibrary.h"
#include "Settings/LunarSettings.h"
#include "Types/SlateConstants.h"
#include "UI/Navigation/Core/LunarNavigationScope.h"
#include "UI/Navigation/Core/LunarNavigationSubsystem.h"
#include "UI/Navigation/Styles/LunarScrollBoxStyleAsset.h"
#include "UI/Navigation/Styles/LunarStyleResolver.h"
#include "UI/Navigation/Types/LunarGameplayTags.h"

/**
 * @file LunarScrollBox.cpp
 * @brief Lunar scroll-container input ownership, reveal scrolling, and style transitions
 * @ingroup LunarNavigationControls
 */

/**
 * @brief Native ScrollBox that enforces Lunar scope ownership and nested scroll chaining
 * @ingroup LunarNavigationControls
 */
class SLunarNavigationScrollBox : public SScrollBox
{
public:
	/**
	 * @brief Associates the native widget with its UMG owner
	 * @param InOwner Lunar scroll box receiving callbacks
	 */
	void SetLunarOwner(ULunarScrollBox* InOwner)
	{
		LunarOwner = InOwner;
	}

	/**
	 * @brief Applies wheel scrolling through Lunar ownership and chaining rules
	 * @param MyGeometry Current widget geometry
	 * @param MouseEvent Mouse-wheel event
	 * @return Handled when this container applies or consumes the delta
	 */
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		ULunarScrollBox* Owner = LunarOwner.Get();
		if (!Owner)
		{
			return SScrollBox::OnMouseWheel(MyGeometry, MouseEvent);
		}

		EndInertialScrolling();
		const float ScrollDelta = -MouseEvent.GetWheelDelta()
			* GetGlobalScrollAmount()
			* Owner->GetWheelScrollMultiplier();
		bool bApplied = false;
		bool bConsumed = false;
		Owner->ApplyDirectScrollDelta(ScrollDelta, bApplied, bConsumed, false);
		return bApplied || bConsumed ? FReply::Handled() : FReply::Unhandled();
	}

	/**
	 * @brief Rejects preview pointer input when this container is outside the active scope
	 * @param MyGeometry Current widget geometry
	 * @param MouseEvent Mouse-button event
	 * @return Base reply for owned input, otherwise unhandled
	 */
	virtual FReply OnPreviewMouseButtonDown(
		const FGeometry& MyGeometry,
		const FPointerEvent& MouseEvent) override
	{
		if (const ULunarScrollBox* Owner = LunarOwner.Get(); Owner && !Owner->IsOwnedByActiveScope())
		{
			return FReply::Unhandled();
		}
		return SScrollBox::OnPreviewMouseButtonDown(MyGeometry, MouseEvent);
	}

	/**
	 * @brief Processes drag scrolling and forwards any unused delta to an ancestor
	 * @param MyGeometry Current widget geometry
	 * @param MouseEvent Mouse or touch move event
	 * @return Native scroll-box input reply
	 */
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (const ULunarScrollBox* Owner = LunarOwner.Get(); Owner && !Owner->IsOwnedByActiveScope())
		{
			return FReply::Unhandled();
		}

		const bool bTouchCapturedBefore = MouseEvent.IsTouchEvent()
			&& HasMouseCaptureByUser(MouseEvent.GetUserIndex(), MouseEvent.GetPointerIndex());
		const float OffsetBefore = GetScrollOffset();
		const FReply Reply = SScrollBox::OnMouseMove(MyGeometry, MouseEvent);

		ULunarScrollBox* Owner = LunarOwner.Get();
		const bool bRightDragHandled = !MouseEvent.IsTouchEvent()
			&& MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton)
			&& Reply.IsEventHandled();
		if (Owner && Owner->bAllowScrollChaining && (bTouchCapturedBefore || bRightDragHandled))
		{
			const FVector2D CursorDelta = MouseEvent.GetCursorDelta();
			const float AxisDelta = GetOrientation() == Orient_Horizontal ? CursorDelta.X : CursorDelta.Y;
			const float RequestedDelta = -AxisDelta / FMath::Max(MyGeometry.Scale, UE_SMALL_NUMBER);
			const float AppliedDelta = GetScrollOffset() - OffsetBefore;
			Owner->ForwardDirectScrollRemainder(RequestedDelta - AppliedDelta);
		}
		return Reply;
	}

	/**
	 * @brief Updates hover interaction state when a mouse enters an owned container
	 * @param MyGeometry Current widget geometry
	 * @param MouseEvent Mouse-enter event
	 */
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		SScrollBox::OnMouseEnter(MyGeometry, MouseEvent);
		if (ULunarScrollBox* Owner = LunarOwner.Get();
			Owner && Owner->IsOwnedByActiveScope() && !MouseEvent.IsTouchEvent())
		{
			Owner->SetDirectInteractionState(ELunarUIInteractionState::PointerHovered);
		}
	}

	/**
	 * @brief Clears hover state after the pointer leaves without capture
	 * @param MouseEvent Mouse-leave event
	 */
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override
	{
		SScrollBox::OnMouseLeave(MouseEvent);
		if (ULunarScrollBox* Owner = LunarOwner.Get(); Owner && !HasMouseCapture())
		{
			Owner->SetDirectInteractionState(ELunarUIInteractionState::PointerNormal);
		}
	}

	/**
	 * @brief Begins native touch or right-drag scrolling for an owned container
	 * @param MyGeometry Current widget geometry
	 * @param MouseEvent Mouse or touch press event
	 * @return Native scroll-box input reply, or unhandled outside the active scope
	 */
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (const ULunarScrollBox* Owner = LunarOwner.Get(); Owner && !Owner->IsOwnedByActiveScope())
		{
			return FReply::Unhandled();
		}

		const FReply Reply = SScrollBox::OnMouseButtonDown(MyGeometry, MouseEvent);
		if (ULunarScrollBox* Owner = LunarOwner.Get();
			Owner && (MouseEvent.IsTouchEvent() || MouseEvent.GetEffectingButton() == EKeys::RightMouseButton))
		{
			Owner->SetDirectInteractionState(ELunarUIInteractionState::PointerPressed);
		}
		return Reply;
	}

	/**
	 * @brief Ends pointer dragging and restores hover or normal interaction state
	 * @param MyGeometry Current widget geometry
	 * @param MouseEvent Mouse or touch release event
	 * @return Native scroll-box input reply
	 */
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		const FReply Reply = SScrollBox::OnMouseButtonUp(MyGeometry, MouseEvent);
		if (ULunarScrollBox* Owner = LunarOwner.Get())
		{
			if (Owner->bAllowScrollChaining)
			{
				EndInertialScrolling();
			}
			Owner->SetDirectInteractionState(
				!MouseEvent.IsTouchEvent() && MyGeometry.IsUnderLocation(MouseEvent.GetScreenSpacePosition())
					? ELunarUIInteractionState::PointerHovered
					: ELunarUIInteractionState::PointerNormal);
		}
		return Reply;
	}

	/**
	 * @brief Ends touch scrolling and restores normal interaction state
	 * @param MyGeometry Current widget geometry
	 * @param TouchEvent Touch-end event
	 * @return Native scroll-box input reply
	 */
	virtual FReply OnTouchEnded(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) override
	{
		const FReply Reply = SScrollBox::OnTouchEnded(MyGeometry, TouchEvent);
		if (ULunarScrollBox* Owner = LunarOwner.Get())
		{
			if (Owner->bAllowScrollChaining)
			{
				EndInertialScrolling();
			}
			Owner->SetDirectInteractionState(ELunarUIInteractionState::PointerNormal);
		}
		return Reply;
	}

	/**
	 * @brief Restores normal interaction state after native pointer capture is lost
	 * @param CaptureLostEvent Capture-loss context
	 */
	virtual void OnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent) override
	{
		SScrollBox::OnMouseCaptureLost(CaptureLostEvent);
		if (ULunarScrollBox* Owner = LunarOwner.Get())
		{
			Owner->SetDirectInteractionState(ELunarUIInteractionState::PointerNormal);
		}
	}

private:
	/** Weak UMG owner used for policy, chaining, and visual-state callbacks. */
	TWeakObjectPtr<ULunarScrollBox> LunarOwner;
};

namespace LunarScrollBox_Private
{
	/**
	 * @brief Resolves a widget's structural UMG parent across panel and outer relationships
	 * @param Widget Widget whose parent should be resolved
	 * @return Structural parent widget, or null at the hierarchy root
	 */
	const UWidget* GetStructuralParent(const UWidget* Widget)
	{
		if (!Widget)
		{
			return nullptr;
		}

		if (const UWidget* Parent = Widget->GetParent())
		{
			return Parent;
		}

		for (const UObject* Outer = Widget->GetOuter(); Outer; Outer = Outer->GetOuter())
		{
			if (const UWidget* OuterWidget = Cast<UWidget>(Outer))
			{
				return OuterWidget;
			}
		}

		return nullptr;
	}

	/**
	 * @brief Tests structural descendant membership with cycle protection
	 * @param Candidate Candidate descendant
	 * @param Ancestor Expected ancestor
	 * @return True when Candidate is strictly below Ancestor
	 */
	bool IsDescendantOf(const UWidget* Candidate, const UWidget* Ancestor)
	{
		if (!Candidate || !Ancestor || Candidate == Ancestor)
		{
			return false;
		}

		TSet<TObjectKey<UWidget>> Visited;
		for (const UWidget* Current = Candidate; Current; Current = GetStructuralParent(Current))
		{
			const TObjectKey<UWidget> CurrentKey(const_cast<UWidget*>(Current));
			if (Visited.Contains(CurrentKey))
			{
				return false;
			}
			Visited.Add(CurrentKey);

			if (Current == Ancestor)
			{
				return true;
			}
		}

		return false;
	}

	/**
	 * @brief Applies supported common presentation fields to a scroll container
	 * @param ScrollBox Scroll container to update
	 * @param Patch Common style patch to apply
	 */
	void ApplyCommonStylePatch(ULunarScrollBox& ScrollBox, const FLunarCommonStylePatch& Patch)
	{
		if (Patch.bOverrideOpacity)
		{
			ScrollBox.SetRenderOpacity(FMath::Clamp(Patch.Opacity, 0.0f, 1.0f));
		}
		if (Patch.bOverrideRenderTransform)
		{
			ScrollBox.SetRenderTransform(Patch.RenderTransform);
		}
		if (Patch.bOverrideRenderTransformPivot)
		{
			ScrollBox.SetRenderTransformPivot(Patch.RenderTransformPivot);
		}
	}

	/**
	 * @brief Applies common and ScrollBox-specific fields from a materialized style
	 * @param ScrollBox Scroll container to update
	 * @param Patch Materialized style snapshot
	 */
	void ApplyStylePatch(ULunarScrollBox& ScrollBox, const FLunarScrollBoxStylePatch& Patch)
	{
		ApplyCommonStylePatch(ScrollBox, Patch.Common);
		if (Patch.bOverrideScrollBarStyle)
		{
			ScrollBox.SetWidgetBarStyle(Patch.ScrollBarStyle);
		}
		if (Patch.bOverrideScrollBarPadding)
		{
			ScrollBox.SetScrollbarPadding(Patch.ScrollBarPadding);
		}
	}

	/**
	 * @brief Interpolates every component of a Slate margin
	 * @param Source Transition source margin
	 * @param Target Transition target margin
	 * @param Alpha Normalized interpolation alpha
	 * @return Interpolated margin
	 */
	FMargin InterpolateMargin(const FMargin& Source, const FMargin& Target, const float Alpha)
	{
		return FMargin(
			FMath::Lerp(Source.Left, Target.Left, Alpha),
			FMath::Lerp(Source.Top, Target.Top, Alpha),
			FMath::Lerp(Source.Right, Target.Right, Alpha),
			FMath::Lerp(Source.Bottom, Target.Bottom, Alpha));
	}

	/**
	 * @brief Interpolates continuous fields between two ScrollBox styles
	 * @param Source Transition source
	 * @param Target Transition target
	 * @param Alpha Normalized interpolation alpha
	 * @return Interpolated style snapshot
	 */
	FLunarScrollBoxStylePatch InterpolateStylePatch(
		const FLunarScrollBoxStylePatch& Source,
		const FLunarScrollBoxStylePatch& Target,
		const float Alpha)
	{
		FLunarScrollBoxStylePatch Result = Target;
		Result.Common = LunarStyleResolver::InterpolateCommonStylePatch(Source.Common, Target.Common, Alpha);
		if (Source.bOverrideScrollBarPadding && Target.bOverrideScrollBarPadding)
		{
			Result.ScrollBarPadding = InterpolateMargin(Source.ScrollBarPadding, Target.ScrollBarPadding, Alpha);
		}
		return Result;
	}

	/**
	 * @brief Compares two materialized ScrollBox style patches
	 * @param A First style
	 * @param B Second style
	 * @return True when all common and specialized fields are equivalent
	 */
	bool AreStylePatchesEquivalent(
		const FLunarScrollBoxStylePatch& A,
		const FLunarScrollBoxStylePatch& B)
	{
		if (!LunarStyleResolver::AreCommonStylesEquivalent(A.Common, B.Common)
			|| A.bOverrideScrollBarStyle != B.bOverrideScrollBarStyle
			|| A.bOverrideScrollBarPadding != B.bOverrideScrollBarPadding)
		{
			return false;
		}
		if (A.bOverrideScrollBarStyle
			&& !FScrollBarStyle::StaticStruct()->CompareScriptStruct(&A.ScrollBarStyle, &B.ScrollBarStyle, 0))
		{
			return false;
		}
		return !A.bOverrideScrollBarPadding || A.ScrollBarPadding == B.ScrollBarPadding;
	}

	/**
	 * @brief Copies brushes, fonts, transition data, and bar style from a logical target
	 * @param InOutStyle Interpolated style to update
	 * @param LogicalTarget Style supplying discrete fields
	 */
	void ApplyDiscreteStyleFields(
		FLunarScrollBoxStylePatch& InOutStyle,
		const FLunarScrollBoxStylePatch& LogicalTarget)
	{
	/** Copies one non-interpolated common style field and its override flag. */
	#define LUNAR_COPY_SCROLL_DISCRETE_COMMON(FieldName) \
		InOutStyle.Common.bOverride##FieldName = LogicalTarget.Common.bOverride##FieldName; \
		InOutStyle.Common.FieldName = LogicalTarget.Common.FieldName;

		LUNAR_COPY_SCROLL_DISCRETE_COMMON(BackgroundBrush)
		LUNAR_COPY_SCROLL_DISCRETE_COMMON(BorderBrush)
		LUNAR_COPY_SCROLL_DISCRETE_COMMON(ForegroundBrush)
		LUNAR_COPY_SCROLL_DISCRETE_COMMON(Font)

#undef LUNAR_COPY_SCROLL_DISCRETE_COMMON
		InOutStyle.Common.Transition = LogicalTarget.Common.Transition;
		InOutStyle.bOverrideScrollBarStyle = LogicalTarget.bOverrideScrollBarStyle;
		InOutStyle.ScrollBarStyle = LogicalTarget.ScrollBarStyle;
	}
}

ULunarScrollBox::ULunarScrollBox(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (const ULunarSettings* Settings = GetDefault<ULunarSettings>())
	{
		ScrollIntoViewSettings = Settings->Navigation.Behavior.DefaultScrollIntoViewSettings;
		bAllowScrollChaining = Settings->Navigation.Behavior.bAllowScrollChainingByDefault;
	}

	ApplyRuntimePolicies();
}

void ULunarScrollBox::ScrollWidgetIntoLunarView(UWidget* WidgetToReveal)
{
	if (!IsValidLunarScrollTarget(WidgetToReveal))
	{
		return;
	}

	float TargetOffset = 0.0f;
	if (!CalculateMinimumScrollOffset(WidgetToReveal, TargetOffset))
	{
		return;
	}

	const float CurrentOffset = GetScrollOffset();
	if (FMath::IsNearlyEqual(CurrentOffset, TargetOffset, KINDA_SMALL_NUMBER))
	{
		return;
	}

	CancelLunarScrollInternal(false, false);
	EndInertialScrolling();

	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	const bool bReduceMotion = Settings && Settings->Navigation.Accessibility.bReduceMotion;
	if (ScrollIntoViewSettings.Mode == ELunarScrollIntoViewMode::Immediate || bReduceMotion)
	{
		ActiveScrollTarget = WidgetToReveal;
		bLunarScrollActive = true;
		OnLunarScrollStarted.Broadcast();

		if (bLunarScrollActive)
		{
			ApplyLunarScrollOffset(TargetOffset);
			FinishLunarScroll(true, true);
		}
		return;
	}

	StartSmoothLunarScroll(WidgetToReveal, TargetOffset);
}

void ULunarScrollBox::CancelLunarScroll()
{
	CancelLunarScrollInternal(true, true);
}

bool ULunarScrollBox::IsLunarScrollActive() const
{
	return bLunarScrollActive;
}

void ULunarScrollBox::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	ApplyRuntimePolicies();
	ApplyResolvedStyle();
}

void ULunarScrollBox::ReleaseSlateResources(const bool bReleaseChildren)
{
	OnUserScrolled.RemoveDynamic(this, &ULunarScrollBox::HandleNativeUserScrolled);
	CancelLunarScrollInternal(false, true);
	StopStyleTransition();
	bHasDisplayedStyle = false;
	UnregisterFromNavigationSubsystem();
	Super::ReleaseSlateResources(bReleaseChildren);
}

void ULunarScrollBox::BeginDestroy()
{
	OnUserScrolled.RemoveDynamic(this, &ULunarScrollBox::HandleNativeUserScrolled);
	CancelLunarScrollInternal(false, true);
	StopStyleTransition();
	UnregisterFromNavigationSubsystem();
	Super::BeginDestroy();
}

TSharedRef<SWidget> ULunarScrollBox::RebuildWidget()
{
	TSharedRef<SLunarNavigationScrollBox> SlateScrollBox = MakeShared<SLunarNavigationScrollBox>();
	SlateScrollBox->SetLunarOwner(this);
	SlateScrollBox->Construct(
		SScrollBox::FArguments()
			.Style(&GetWidgetStyle())
			.ScrollBarStyle(&GetWidgetBarStyle())
			.Orientation(GetOrientation())
			.ConsumeMouseWheel(GetConsumeMouseWheel())
			.NavigationDestination(GetNavigationDestination())
			.NavigationScrollPadding(GetNavigationScrollPadding())
			.ScrollWhenFocusChanges(GetScrollWhenFocusChanges())
			.BackPadScrolling(IsBackPadScrolling())
			.FrontPadScrolling(IsFrontPadScrolling())
			.AnimateWheelScrolling(IsAnimateWheelScrolling())
			.ScrollAnimationInterpSpeed(GetScrollAnimationInterpolationSpeed())
			.WheelScrollMultiplier(GetWheelScrollMultiplier())
			.EnableTouchScrolling(GetIsTouchScrollingEnabled())
			.ConsumePointerInput(GetConsumePointerInput())
			.OnUserScrolled(BIND_UOBJECT_DELEGATE(FOnUserScrolled, SlateHandleUserScrolled))
			.OnScrollBarVisibilityChanged(BIND_UOBJECT_DELEGATE(FOnScrollBarVisibilityChanged, SlateHandleScrollBarVisibilityChanged))
			.OnFocusReceived(BIND_UOBJECT_DELEGATE(FOnScrollBoxFocusReceived, SlateHandleFocusReceived))
			.OnFocusLost(BIND_UOBJECT_DELEGATE(FOnScrollBoxFocusLost, SlateHandleFocusLost))
			.OnFocusChanging(BIND_UOBJECT_DELEGATE(FOnScrollBoxFocusChanging, SlateHandleFocusChanging)));
	MyScrollBox = SlateScrollBox;

	for (UPanelSlot* PanelSlot : Slots)
	{
		if (UScrollBoxSlot* ScrollBoxSlot = Cast<UScrollBoxSlot>(PanelSlot))
		{
			ScrollBoxSlot->Parent = this;
			ScrollBoxSlot->BuildSlot(MyScrollBox.ToSharedRef());
		}
	}
	return MyScrollBox.ToSharedRef();
}

void ULunarScrollBox::OnWidgetRebuilt()
{
	Super::OnWidgetRebuilt();
	LastAuthorizedScrollOffset = GetScrollOffset();
	OnUserScrolled.AddUniqueDynamic(this, &ULunarScrollBox::HandleNativeUserScrolled);
	RegisterWithNavigationSubsystem();
}

void ULunarScrollBox::HandleNativeUserScrolled(const float CurrentOffset)
{
	if (bApplyingLunarScrollOffset)
	{
		LastAuthorizedScrollOffset = CurrentOffset;
		return;
	}

	if (!IsOwnedByActiveScope())
	{
		ApplyLunarScrollOffset(LastAuthorizedScrollOffset);
		return;
	}

	LastAuthorizedScrollOffset = CurrentOffset;
	NotifyDirectScrollInput();
	if (bLunarScrollActive)
	{
		// Direct wheel, drag, touch, or scrollbar input takes ownership from the animation.
		FinishLunarScroll(false, false);
	}
}

ULunarNavigationSubsystem* ULunarScrollBox::ResolveNavigationSubsystem() const
{
	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		return LocalPlayer->GetSubsystem<ULunarNavigationSubsystem>();
	}

	return nullptr;
}

void ULunarScrollBox::RegisterWithNavigationSubsystem()
{
	ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem();
	if (RegisteredNavigationSubsystem.Get() == NavigationSubsystem)
	{
		if (NavigationSubsystem)
		{
			NavigationSubsystem->RegisterScrollBox(this);
		}
		return;
	}

	UnregisterFromNavigationSubsystem();
	if (NavigationSubsystem)
	{
		NavigationSubsystem->RegisterScrollBox(this);
		RegisteredNavigationSubsystem = NavigationSubsystem;
	}
}

void ULunarScrollBox::UnregisterFromNavigationSubsystem()
{
	if (ULunarNavigationSubsystem* NavigationSubsystem = RegisteredNavigationSubsystem.Get())
	{
		NavigationSubsystem->UnregisterScrollBox(this);
	}
	RegisteredNavigationSubsystem.Reset();
}

void ULunarScrollBox::ApplyResolvedStyle()
{
	FLunarUIVisualState VisualState;
	VisualState.ValueStateTag = LunarGameplayTags::UI_State_Value_Normal.GetTag();
	VisualState.InteractionState = DirectInteractionState;
	if (const ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem())
	{
		VisualState.InputDevice = NavigationSubsystem->GetLastInputDevice();
	}

	FLunarScrollBoxStylePatch ResolvedStyle;
	FString StyleError;
	if (LunarStyleResolver::ResolveScrollBoxStyle(StyleAsset, VisualState, ResolvedStyle, &StyleError))
	{
		ApplyStyleTarget(ResolvedStyle);
	}
}

void ULunarScrollBox::EnsureStyleBaseline()
{
	if (bHasStyleBaseline)
	{
		return;
	}

	StyleBaselineRenderOpacity = GetRenderOpacity();
	StyleBaselineRenderTransform = GetRenderTransform();
	StyleBaselineRenderTransformPivot = GetRenderTransformPivot();
	StyleBaselineScrollBarStyle = GetWidgetBarStyle();
	StyleBaselineScrollBarPadding = GetScrollbarPadding();
	bHasStyleBaseline = true;
}

FLunarScrollBoxStylePatch ULunarScrollBox::MaterializeStyleSnapshot(
	const FLunarScrollBoxStylePatch& ResolvedStyle) const
{
	FLunarScrollBoxStylePatch Result = ResolvedStyle;
	if (!Result.Common.bOverrideOpacity)
	{
		Result.Common.bOverrideOpacity = true;
		Result.Common.Opacity = StyleBaselineRenderOpacity;
	}
	if (!Result.Common.bOverrideRenderTransform)
	{
		Result.Common.bOverrideRenderTransform = true;
		Result.Common.RenderTransform = StyleBaselineRenderTransform;
	}
	if (!Result.Common.bOverrideRenderTransformPivot)
	{
		Result.Common.bOverrideRenderTransformPivot = true;
		Result.Common.RenderTransformPivot = StyleBaselineRenderTransformPivot;
	}
	if (!Result.bOverrideScrollBarStyle)
	{
		Result.bOverrideScrollBarStyle = true;
		Result.ScrollBarStyle = StyleBaselineScrollBarStyle;
	}
	if (!Result.bOverrideScrollBarPadding)
	{
		Result.bOverrideScrollBarPadding = true;
		Result.ScrollBarPadding = StyleBaselineScrollBarPadding;
	}
	return Result;
}

void ULunarScrollBox::ApplyStyleTarget(const FLunarScrollBoxStylePatch& NewTarget)
{
	EnsureStyleBaseline();
	const FLunarScrollBoxStylePatch MaterializedTarget = MaterializeStyleSnapshot(NewTarget);
	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	const bool bReduceMotion = Settings && Settings->Navigation.Accessibility.bReduceMotion;
	if (bReduceMotion)
	{
		StopStyleTransition();
		TransitionSourceStyle = MaterializedTarget;
		TransitionTargetStyle = MaterializedTarget;
		LogicalTargetStyle = MaterializedTarget;
		ApplyDisplayedStyle(MaterializedTarget);
		return;
	}
	if (bHasDisplayedStyle
		&& LunarScrollBox_Private::AreStylePatchesEquivalent(LogicalTargetStyle, MaterializedTarget))
	{
		return;
	}

	if (bStyleTransitionActive)
	{
		const bool bReturnsToSource = !bStyleTransitionReversing
			&& LunarScrollBox_Private::AreStylePatchesEquivalent(TransitionSourceStyle, MaterializedTarget);
		const bool bReturnsToForwardTarget = bStyleTransitionReversing
			&& LunarScrollBox_Private::AreStylePatchesEquivalent(TransitionTargetStyle, MaterializedTarget);
		if (bReturnsToSource || bReturnsToForwardTarget)
		{
			LogicalTargetStyle = MaterializedTarget;
			bStyleTransitionReversing = bReturnsToSource;
			FLunarScrollBoxStylePatch TransitionSnapshot = DisplayedStyle;
			LunarScrollBox_Private::ApplyDiscreteStyleFields(
				TransitionSnapshot,
				bStyleTransitionReversing ? TransitionSourceStyle : TransitionTargetStyle);
			ApplyDisplayedStyle(TransitionSnapshot);
			return;
		}
	}

	const bool bCanTransition = bHasDisplayedStyle
		&& MaterializedTarget.Common.Transition.bEnabled
		&& MaterializedTarget.Common.Transition.Duration > 0.0f;
	if (!bCanTransition)
	{
		StopStyleTransition();
		TransitionSourceStyle = MaterializedTarget;
		TransitionTargetStyle = MaterializedTarget;
		LogicalTargetStyle = MaterializedTarget;
		ApplyDisplayedStyle(MaterializedTarget);
		return;
	}

	TransitionSourceStyle = DisplayedStyle;
	TransitionTargetStyle = MaterializedTarget;
	LogicalTargetStyle = MaterializedTarget;
	StyleTransitionElapsed = 0.0f;
	StyleTransitionDuration = MaterializedTarget.Common.Transition.Duration;
	bStyleTransitionActive = true;
	bStyleTransitionReversing = false;
	if (!StyleTransitionTickerHandle.IsValid())
	{
		StyleTransitionTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateUObject(this, &ULunarScrollBox::TickStyleTransition));
	}
	ApplyDisplayedStyle(LunarScrollBox_Private::InterpolateStylePatch(
		TransitionSourceStyle,
		TransitionTargetStyle,
		0.0f));
}

void ULunarScrollBox::ApplyDisplayedStyle(const FLunarScrollBoxStylePatch& NewDisplayedStyle)
{
	EnsureStyleBaseline();
	DisplayedStyle = NewDisplayedStyle;
	bHasDisplayedStyle = true;
	SetRenderOpacity(StyleBaselineRenderOpacity);
	SetRenderTransform(StyleBaselineRenderTransform);
	SetRenderTransformPivot(StyleBaselineRenderTransformPivot);
	SetWidgetBarStyle(StyleBaselineScrollBarStyle);
	SetScrollbarPadding(StyleBaselineScrollBarPadding);
	LunarScrollBox_Private::ApplyStylePatch(*this, DisplayedStyle);
}

bool ULunarScrollBox::TickStyleTransition(const float DeltaTime)
{
	if (!bStyleTransitionActive || StyleTransitionDuration <= UE_SMALL_NUMBER)
	{
		StyleTransitionTickerHandle.Reset();
		return false;
	}
	if (const ULunarSettings* Settings = GetDefault<ULunarSettings>();
		Settings && Settings->Navigation.Accessibility.bReduceMotion)
	{
		ApplyDisplayedStyle(LogicalTargetStyle);
		bStyleTransitionActive = false;
		bStyleTransitionReversing = false;
		StyleTransitionTickerHandle.Reset();
		return false;
	}

	const float Step = FMath::Max(0.0f, DeltaTime);
	StyleTransitionElapsed += bStyleTransitionReversing ? -Step : Step;
	StyleTransitionElapsed = FMath::Clamp(StyleTransitionElapsed, 0.0f, StyleTransitionDuration);
	const float Alpha = StyleTransitionElapsed / StyleTransitionDuration;
	const float EasedAlpha = static_cast<float>(UKismetMathLibrary::Ease(
		0.0,
		1.0,
		Alpha,
		TransitionTargetStyle.Common.Transition.Easing));
	FLunarScrollBoxStylePatch TransitionSnapshot = LunarScrollBox_Private::InterpolateStylePatch(
		TransitionSourceStyle,
		TransitionTargetStyle,
		EasedAlpha);
	if (bStyleTransitionReversing)
	{
		LunarScrollBox_Private::ApplyDiscreteStyleFields(TransitionSnapshot, TransitionSourceStyle);
	}
	ApplyDisplayedStyle(TransitionSnapshot);

	const bool bFinished = bStyleTransitionReversing ? Alpha <= 0.0f : Alpha >= 1.0f;
	if (bFinished)
	{
		ApplyDisplayedStyle(bStyleTransitionReversing ? TransitionSourceStyle : TransitionTargetStyle);
		bStyleTransitionActive = false;
		bStyleTransitionReversing = false;
		StyleTransitionTickerHandle.Reset();
		return false;
	}
	return true;
}

void ULunarScrollBox::StopStyleTransition()
{
	if (StyleTransitionTickerHandle.IsValid())
	{
		FTSTicker::RemoveTicker(StyleTransitionTickerHandle);
		StyleTransitionTickerHandle.Reset();
	}
	bStyleTransitionActive = false;
	bStyleTransitionReversing = false;
	StyleTransitionElapsed = 0.0f;
	StyleTransitionDuration = 0.0f;
}

void ULunarScrollBox::ApplyRuntimePolicies()
{
	// A Lunar ScrollBox is a container and never participates in Lunar or native focus selection.
	SetIsFocusable(false);
	SetScrollWhenFocusChanges(EScrollWhenFocusChanges::NoScroll);

	// Keep native SScrollBox direct interaction. Chaining uses normal Slate event bubbling.
	SetIsTouchScrollingEnabled(true);
	SetConsumePointerInput(true);
	SetAllowRightClickDragScrolling(true);
	SetAllowOverscroll(false);
	SetConsumeMouseWheel(
		bAllowScrollChaining
			? EConsumeMouseWheel::WhenScrollingPossible
			: EConsumeMouseWheel::Always);
}

void ULunarScrollBox::SetDirectInteractionState(const ELunarUIInteractionState NewInteractionState)
{
	if (DirectInteractionState != NewInteractionState)
	{
		DirectInteractionState = NewInteractionState;
		ApplyResolvedStyle();
	}
}

bool ULunarScrollBox::IsValidLunarScrollTarget(const UWidget* WidgetToReveal) const
{
	if (!IsValid(WidgetToReveal)
		|| WidgetToReveal->HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed)
		|| !LunarScrollBox_Private::IsDescendantOf(WidgetToReveal, this))
	{
		return false;
	}

	const ULocalPlayer* OwningLocalPlayer = GetOwningLocalPlayer();
	const ULocalPlayer* TargetLocalPlayer = WidgetToReveal->GetOwningLocalPlayer();
	if (!OwningLocalPlayer || !TargetLocalPlayer || OwningLocalPlayer != TargetLocalPlayer)
	{
		return false;
	}

	const ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem();
	const ULunarNavigationScope* ActiveScope = NavigationSubsystem
		? NavigationSubsystem->GetActiveNavigationScope()
		: nullptr;
	const UWidget* ScopeRoot = ActiveScope ? ActiveScope->RootWidget.Get() : nullptr;
	if (!ScopeRoot)
	{
		return false;
	}

	const bool bContainerInActiveScope = this == ScopeRoot || LunarScrollBox_Private::IsDescendantOf(this, ScopeRoot);
	const bool bTargetInActiveScope = WidgetToReveal == ScopeRoot
		|| LunarScrollBox_Private::IsDescendantOf(WidgetToReveal, ScopeRoot);
	return bContainerInActiveScope && bTargetInActiveScope;
}

bool ULunarScrollBox::CalculateMinimumScrollOffset(const UWidget* WidgetToReveal, float& OutTargetOffset) const
{
	const FGeometry& ContainerGeometry = GetCachedGeometry();
	const FGeometry& TargetGeometry = WidgetToReveal->GetCachedGeometry();
	const FVector2D ContainerSize = ContainerGeometry.GetLocalSize();
	const FVector2D TargetSize = TargetGeometry.GetLocalSize();
	const bool bHorizontal = GetOrientation() == Orient_Horizontal;
	const float ViewportExtent = bHorizontal ? ContainerSize.X : ContainerSize.Y;
	if (ViewportExtent <= UE_SMALL_NUMBER || TargetSize.IsNearlyZero())
	{
		return false;
	}

	const FVector2D TargetCorners[] =
	{
		FVector2D::ZeroVector,
		FVector2D(TargetSize.X, 0.0f),
		FVector2D(0.0f, TargetSize.Y),
		TargetSize
	};

	float TargetMin = TNumericLimits<float>::Max();
	float TargetMax = TNumericLimits<float>::Lowest();
	for (const FVector2D& TargetCorner : TargetCorners)
	{
		const FVector2D ContainerLocal = ContainerGeometry.AbsoluteToLocal(TargetGeometry.LocalToAbsolute(TargetCorner));
		const float AxisValue = bHorizontal ? ContainerLocal.X : ContainerLocal.Y;
		TargetMin = FMath::Min(TargetMin, AxisValue);
		TargetMax = FMath::Max(TargetMax, AxisValue);
	}

	const float LeadingPadding = FMath::Max(
		0.0f,
		bHorizontal ? ScrollIntoViewSettings.ViewportPadding.Left : ScrollIntoViewSettings.ViewportPadding.Top);
	const float TrailingPadding = FMath::Max(
		0.0f,
		bHorizontal ? ScrollIntoViewSettings.ViewportPadding.Right : ScrollIntoViewSettings.ViewportPadding.Bottom);
	const float VisibleStart = FMath::Min(LeadingPadding, ViewportExtent);
	const float VisibleEnd = FMath::Max(VisibleStart, ViewportExtent - TrailingPadding);

	const float CurrentOffset = GetScrollOffset();
	float RequiredDelta = 0.0f;
	if (TargetMin < VisibleStart)
	{
		RequiredDelta = TargetMin - VisibleStart;
	}
	else if (TargetMax > VisibleEnd)
	{
		RequiredDelta = TargetMax - VisibleEnd;
	}

	const float MaximumOffset = FMath::Max(0.0f, GetScrollOffsetOfEnd());
	OutTargetOffset = FMath::Clamp(CurrentOffset + RequiredDelta, 0.0f, MaximumOffset);
	return true;
}

void ULunarScrollBox::ApplyLunarScrollOffset(const float NewOffset)
{
	LastAuthorizedScrollOffset = NewOffset;
	TGuardValue<bool> ApplyingOffsetGuard(bApplyingLunarScrollOffset, true);
	SetScrollOffset(NewOffset);
}

void ULunarScrollBox::StartSmoothLunarScroll(UWidget* WidgetToReveal, const float TargetOffset)
{
	const float CurrentOffset = GetScrollOffset();
	const float Distance = FMath::Abs(TargetOffset - CurrentOffset);
	const float ConfiguredDuration = FMath::Max(0.0f, ScrollIntoViewSettings.TransitionDuration);
	const float DurationFromSpeed = ScrollIntoViewSettings.ScrollSpeed > UE_SMALL_NUMBER
		? Distance / ScrollIntoViewSettings.ScrollSpeed
		: 0.0f;
	const float EffectiveDuration = ConfiguredDuration > UE_SMALL_NUMBER
		? ConfiguredDuration
		: DurationFromSpeed;

	if (EffectiveDuration <= UE_SMALL_NUMBER)
	{
		ActiveScrollTarget = WidgetToReveal;
		bLunarScrollActive = true;
		OnLunarScrollStarted.Broadcast();
		if (bLunarScrollActive)
		{
			ApplyLunarScrollOffset(TargetOffset);
			FinishLunarScroll(true, true);
		}
		return;
	}

	ActiveScrollTarget = WidgetToReveal;
	LunarScrollStartOffset = CurrentOffset;
	LunarScrollTargetOffset = TargetOffset;
	LunarScrollElapsed = 0.0f;
	LunarScrollDuration = EffectiveDuration;
	LunarScrollTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &ULunarScrollBox::TickSmoothLunarScroll));
	bLunarScrollActive = true;
	OnLunarScrollStarted.Broadcast();
}

bool ULunarScrollBox::TickSmoothLunarScroll(const float DeltaTime)
{
	if (!bLunarScrollActive || !ActiveScrollTarget.IsValid() || !MyScrollBox.IsValid())
	{
		LunarScrollTickerHandle.Reset();
		FinishLunarScroll(true, false);
		return false;
	}

	LunarScrollElapsed += FMath::Max(0.0f, DeltaTime);
	const float Alpha = LunarScrollDuration > UE_SMALL_NUMBER
		? FMath::Clamp(LunarScrollElapsed / LunarScrollDuration, 0.0f, 1.0f)
		: 1.0f;
	ApplyLunarScrollOffset(FMath::Lerp(LunarScrollStartOffset, LunarScrollTargetOffset, Alpha));

	if (Alpha >= 1.0f)
	{
		LunarScrollTickerHandle.Reset();
		FinishLunarScroll(true, true);
		return false;
	}

	return true;
}

void ULunarScrollBox::FinishLunarScroll(
	const bool bBroadcastFinished,
	const bool bCompleted)
{
	const bool bWasActive = bLunarScrollActive;
	if (LunarScrollTickerHandle.IsValid())
	{
		FTSTicker::RemoveTicker(LunarScrollTickerHandle);
		LunarScrollTickerHandle.Reset();
	}

	bLunarScrollActive = false;
	ActiveScrollTarget.Reset();
	LunarScrollElapsed = 0.0f;
	LunarScrollDuration = 0.0f;

	if (bWasActive)
	{
		LunarScrollFinishedNative.Broadcast(this, bCompleted);
		if (bBroadcastFinished)
		{
			OnLunarScrollFinished.Broadcast();
		}
	}
}

void ULunarScrollBox::CancelLunarScrollInternal(
	const bool bBroadcastFinished,
	const bool bReleasePointerCapture)
{
	FinishLunarScroll(bBroadcastFinished, false);
	EndInertialScrolling();
	if (bReleasePointerCapture)
	{
		ReleaseOwnedPointerCapture();
	}
}

void ULunarScrollBox::ReleaseOwnedPointerCapture()
{
	if (MyScrollBox.IsValid()
		&& MyScrollBox->HasMouseCapture()
		&& FSlateApplication::IsInitialized())
	{
		if (const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
		{
			if (const TSharedPtr<const FSlateUser> SlateUser = LocalPlayer->GetSlateUser())
			{
				FSlateApplication::Get().ReleaseAllPointerCapture(SlateUser->GetUserIndex());
			}
		}
	}
}

float ULunarScrollBox::ApplyDirectScrollDelta(
	const float ScrollDelta,
	bool& bOutApplied,
	bool& bOutConsumed,
	const bool bRequireSameOrientation)
{
	if (FMath::IsNearlyZero(ScrollDelta))
	{
		return 0.0f;
	}
	if (!IsOwnedByActiveScope())
	{
		return ScrollDelta;
	}

	NotifyDirectScrollInput();
	CancelLunarScrollInternal(false, false);
	EndInertialScrolling();

	const float PreviousOffset = GetScrollOffset();
	const float MaximumOffset = FMath::Max(0.0f, GetScrollOffsetOfEnd());
	const float NewOffset = FMath::Clamp(PreviousOffset + ScrollDelta, 0.0f, MaximumOffset);
	const float AppliedDelta = NewOffset - PreviousOffset;
	if (!FMath::IsNearlyZero(AppliedDelta))
	{
		SetScrollOffset(NewOffset);
		SlateHandleUserScrolled(NewOffset);
		bOutApplied = true;
	}

	float RemainingDelta = ScrollDelta - AppliedDelta;
	if (FMath::IsNearlyZero(RemainingDelta))
	{
		return 0.0f;
	}
	if (!bAllowScrollChaining)
	{
		bOutConsumed = true;
		return 0.0f;
	}
	if (ULunarScrollBox* Ancestor = FindNearestChainedAncestor(bRequireSameOrientation))
	{
		RemainingDelta = Ancestor->ApplyDirectScrollDelta(
			RemainingDelta,
			bOutApplied,
			bOutConsumed,
			bRequireSameOrientation);
	}
	return RemainingDelta;
}

void ULunarScrollBox::NotifyDirectScrollInput()
{
	if (ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem())
	{
		NavigationSubsystem->CancelSelectionScrollChainForDirectInput(this);
	}
}

void ULunarScrollBox::ForwardDirectScrollRemainder(const float ScrollDelta)
{
	if (FMath::IsNearlyZero(ScrollDelta))
	{
		return;
	}
	if (!bAllowScrollChaining)
	{
		return;
	}
	if (ULunarScrollBox* Ancestor = FindNearestChainedAncestor(true))
	{
		bool bApplied = false;
		bool bConsumed = false;
		Ancestor->ApplyDirectScrollDelta(ScrollDelta, bApplied, bConsumed, true);
	}
}

ULunarScrollBox* ULunarScrollBox::FindNearestChainedAncestor(
	const bool bRequireSameOrientation) const
{
	for (const UWidget* Current = LunarScrollBox_Private::GetStructuralParent(this);
		Current;
		Current = LunarScrollBox_Private::GetStructuralParent(Current))
	{
		ULunarScrollBox* Ancestor = const_cast<ULunarScrollBox*>(Cast<ULunarScrollBox>(Current));
		if (Ancestor
			&& Ancestor->GetOwningLocalPlayer() == GetOwningLocalPlayer()
			&& Ancestor->IsOwnedByActiveScope()
			&& (!bRequireSameOrientation || Ancestor->GetOrientation() == GetOrientation()))
		{
			return Ancestor;
		}
	}
	return nullptr;
}

bool ULunarScrollBox::IsOwnedByActiveScope() const
{
	const ULocalPlayer* OwningLocalPlayer = GetOwningLocalPlayer();
	const ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem();
	const ULunarNavigationScope* ActiveScope = NavigationSubsystem
		? NavigationSubsystem->GetActiveNavigationScope()
		: nullptr;
	const UWidget* ScopeRoot = ActiveScope ? ActiveScope->RootWidget.Get() : nullptr;
	return OwningLocalPlayer
		&& NavigationSubsystem
		&& NavigationSubsystem->GetLocalPlayer() == OwningLocalPlayer
		&& ScopeRoot
		&& (this == ScopeRoot || LunarScrollBox_Private::IsDescendantOf(this, ScopeRoot));
}
