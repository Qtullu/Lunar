// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "UI/Navigation/Controls/LunarScrollBox.h"

#include "Components/ScrollBoxSlot.h"
#include "Curves/CurveFloat.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "Settings/LunarSettings.h"
#include "Types/SlateConstants.h"
#include "UI/Navigation/Core/LunarNavigationScope.h"
#include "UI/Navigation/Core/LunarNavigationSubsystem.h"
#include "UObject/ConstructorHelpers.h"

/**
 * @file LunarScrollBox.cpp
 * @brief Lunar scroll-container input ownership, reveal scrolling, and native presentation
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
	 * @brief Resolves a descendant offset from freshly arranged Slate geometry
	 * @param WidgetToReveal Descendant Slate widget to locate
	 * @param ViewportPadding Safe viewport padding supplied by the UMG owner
	 * @param OutTargetOffset Receives the minimum clamped desired offset
	 * @return True when the descendant and valid viewport geometry were found
	 */
	bool CalculateLunarDescendantOffset(
		const TSharedPtr<SWidget>& WidgetToReveal,
		const FMargin& ViewportPadding,
		float& OutTargetOffset)
	{
		if (!WidgetToReveal.IsValid())
		{
			return false;
		}

		const FGeometry& ScrollGeometry = GetTickSpaceGeometry();
		const FVector2D ViewportSize = ScrollGeometry.GetLocalSize();
		const bool bHorizontal = GetOrientation() == Orient_Horizontal;
		const float ViewportExtent = bHorizontal ? ViewportSize.X : ViewportSize.Y;
		if (ViewportExtent <= UE_SMALL_NUMBER)
		{
			return false;
		}

		TSet<TSharedRef<SWidget>> WidgetsToFind;
		WidgetsToFind.Add(WidgetToReveal.ToSharedRef());
		TMap<TSharedRef<SWidget>, FArrangedWidget> ArrangedDescendants;
		FindChildGeometries(ScrollGeometry, WidgetsToFind, ArrangedDescendants);
		const FArrangedWidget* ArrangedWidget = ArrangedDescendants.Find(WidgetToReveal.ToSharedRef());
		if (!ArrangedWidget)
		{
			return false;
		}

		const FVector2D TargetStartLocal = ScrollGeometry.AbsoluteToLocal(
			ArrangedWidget->Geometry.GetAbsolutePosition());
		const FVector2D TargetSize = ArrangedWidget->Geometry.GetLocalSize();
		const float TargetStart = bHorizontal ? TargetStartLocal.X : TargetStartLocal.Y;
		const float TargetEnd = TargetStart + (bHorizontal ? TargetSize.X : TargetSize.Y);
		const float LeadingPadding = FMath::Max(
			0.0f,
			bHorizontal ? ViewportPadding.Left : ViewportPadding.Top);
		const float TrailingPadding = FMath::Max(
			0.0f,
			bHorizontal ? ViewportPadding.Right : ViewportPadding.Bottom);
		const float VisibleStart = FMath::Min(LeadingPadding, ViewportExtent);
		const float VisibleEnd = FMath::Max(VisibleStart, ViewportExtent - TrailingPadding);

		float RequiredDelta = 0.0f;
		if (TargetStart < VisibleStart)
		{
			RequiredDelta = TargetStart - VisibleStart;
		}
		else if (TargetEnd > VisibleEnd)
		{
			RequiredDelta = TargetEnd - VisibleEnd;
		}

		const float MaximumOffset = FMath::Max(0.0f, GetScrollOffsetOfEnd());
		OutTargetOffset = FMath::Clamp(GetScrollOffset() + RequiredDelta, 0.0f, MaximumOffset);
		return true;
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

		return SScrollBox::OnMouseButtonDown(MyGeometry, MouseEvent);
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
		}
		return Reply;
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

}

ULunarScrollBox::ULunarScrollBox(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (const ULunarSettings* Settings = GetDefault<ULunarSettings>())
	{
		bAllowScrollChaining = Settings->Navigation.Behavior.bAllowScrollChainingByDefault;
	}

	static ConstructorHelpers::FObjectFinder<UCurveFloat> DefaultInterpolationCurve(
		TEXT("/Lunar/Curves/Float/CF_EaseOutExpo.CF_EaseOutExpo"));
	if (DefaultInterpolationCurve.Succeeded())
	{
		ScrollInterpolationCurve = DefaultInterpolationCurve.Object;
	}

	NormalizeInterpolationSettings();
	ScrollBarPresentationStyle = GetWidgetBarStyle();
	ScrollBarPresentationPadding = GetScrollbarPadding();
	ApplyRuntimePolicies();
	ApplyNativePresentation();
}

#if WITH_EDITOR
const FText ULunarScrollBox::GetPaletteCategory()
{
	return NSLOCTEXT("LunarNavigationPalette", "Category", "Lunar Navigation");
}
#endif

void ULunarScrollBox::SetScrollBarPresentationStyle(const FScrollBarStyle& NewStyle)
{
	ScrollBarPresentationStyle = NewStyle;
	ApplyNativePresentation();
}

void ULunarScrollBox::SetScrollBarPresentationPadding(const FMargin NewPadding)
{
	ScrollBarPresentationPadding = NewPadding;
	ApplyNativePresentation();
}

void ULunarScrollBox::ConfigureScrollBoxPresentation(
	const FScrollBarStyle& NewStyle,
	const FMargin NewPadding)
{
	ScrollBarPresentationStyle = NewStyle;
	ScrollBarPresentationPadding = NewPadding;
	ApplyNativePresentation();
}

void ULunarScrollBox::GetScrollBoxPresentation(
	FScrollBarStyle& OutStyle,
	FMargin& OutPadding) const
{
	OutStyle = ScrollBarPresentationStyle;
	OutPadding = ScrollBarPresentationPadding;
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

	NormalizeInterpolationSettings();
	if (ShouldSnapScrollIntoView())
	{
		ActiveScrollTarget = WidgetToReveal;
		bLunarScrollActive = true;
		OnLunarScrollStarted.Broadcast(this);

		if (bLunarScrollActive)
		{
			ApplyLunarScrollOffset(TargetOffset);
			FinishLunarScroll(true, true);
		}
		return;
	}

	StartInterpolatedLunarScroll(WidgetToReveal, TargetOffset);
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
	NormalizeInterpolationSettings();
	ApplyRuntimePolicies();
	ApplyNativePresentation();
}

void ULunarScrollBox::ReleaseSlateResources(const bool bReleaseChildren)
{
	OnUserScrolled.RemoveDynamic(this, &ULunarScrollBox::HandleNativeUserScrolled);
	CancelLunarScrollInternal(false, true);
	UnregisterFromNavigationSubsystem();
	Super::ReleaseSlateResources(bReleaseChildren);
}

void ULunarScrollBox::BeginDestroy()
{
	OnUserScrolled.RemoveDynamic(this, &ULunarScrollBox::HandleNativeUserScrolled);
	CancelLunarScrollInternal(false, true);
	UnregisterFromNavigationSubsystem();
	Super::BeginDestroy();
}

TSharedRef<SWidget> ULunarScrollBox::RebuildWidget()
{
	ApplyNativePresentation();
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
	ApplyNativePresentation();
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

void ULunarScrollBox::ApplyNativePresentation()
{
	SetWidgetBarStyle(ScrollBarPresentationStyle);
	SetScrollbarPadding(ScrollBarPresentationPadding);
}
void ULunarScrollBox::ApplyRuntimePolicies()
{
	SetOrientation(ScrollOrientation);

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

bool ULunarScrollBox::CalculateMinimumScrollOffset(
	const UWidget* WidgetToReveal,
	float& OutTargetOffset) const
{
	if (!WidgetToReveal || !MyScrollBox.IsValid())
	{
		return false;
	}

	const TSharedPtr<SWidget> TargetSlateWidget = WidgetToReveal->GetCachedWidget();
	const TSharedPtr<SLunarNavigationScrollBox> LunarSlateScrollBox =
		StaticCastSharedPtr<SLunarNavigationScrollBox>(MyScrollBox);
	return LunarSlateScrollBox.IsValid()
		&& LunarSlateScrollBox->CalculateLunarDescendantOffset(
			TargetSlateWidget,
			ViewportPadding,
			OutTargetOffset);
}
void ULunarScrollBox::ApplyLunarScrollOffset(const float NewOffset)
{
	LastAuthorizedScrollOffset = NewOffset;
	TGuardValue<bool> ApplyingOffsetGuard(bApplyingLunarScrollOffset, true);
	SetScrollOffset(NewOffset);
}

void ULunarScrollBox::StartInterpolatedLunarScroll(UWidget* WidgetToReveal, const float TargetOffset)
{
	ActiveScrollTarget = WidgetToReveal;
	LunarScrollStartOffset = GetScrollOffset();
	LunarScrollTargetOffset = TargetOffset;
	LunarScrollElapsed = 0.0f;
	LunarScrollTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &ULunarScrollBox::TickInterpolatedLunarScroll));
	bLunarScrollActive = true;
	OnLunarScrollStarted.Broadcast(this);
}

bool ULunarScrollBox::TickInterpolatedLunarScroll(const float DeltaTime)
{
	if (!bLunarScrollActive || !ActiveScrollTarget.IsValid() || !MyScrollBox.IsValid())
	{
		LunarScrollTickerHandle.Reset();
		FinishLunarScroll(true, false);
		return false;
	}

	LunarScrollElapsed += FMath::Max(0.0f, DeltaTime);
	const float Progress = FMath::Clamp(LunarScrollElapsed * ScrollInterpolationSpeed, 0.0f, 1.0f);
	const float CurveAlpha = EvaluateScrollInterpolationCurve(Progress);
	ApplyLunarScrollOffset(Progress >= 1.0f
		? LunarScrollTargetOffset
		: FMath::Lerp(LunarScrollStartOffset, LunarScrollTargetOffset, CurveAlpha));

	if (Progress >= 1.0f)
	{
		LunarScrollTickerHandle.Reset();
		FinishLunarScroll(true, true);
		return false;
	}

	return true;
}

float ULunarScrollBox::EvaluateScrollInterpolationCurve(const float Progress) const
{
	const float SafeProgress = FMath::Clamp(Progress, 0.0f, 1.0f);
	if (!IsValid(ScrollInterpolationCurve))
	{
		return SafeProgress;
	}

	const float EvaluatedAlpha = ScrollInterpolationCurve->GetFloatValue(SafeProgress);
	return FMath::IsFinite(EvaluatedAlpha)
		? FMath::Clamp(EvaluatedAlpha, 0.0f, 1.0f)
		: SafeProgress;
}

bool ULunarScrollBox::ShouldSnapScrollIntoView() const
{
	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	const bool bReduceMotion = Settings && Settings->Navigation.Accessibility.bReduceMotion;
	bool bDesignerPreview = false;
#if WITH_EDITOR
	bDesignerPreview = IsDesignTime();
#endif
	return bDesignerPreview
		|| bReduceMotion
		|| !bInterpolateScrollIntoView
		|| !FMath::IsFinite(ScrollInterpolationSpeed)
		|| ScrollInterpolationSpeed <= 0.0f;
}

void ULunarScrollBox::NormalizeInterpolationSettings()
{
	ScrollInterpolationSpeed = FMath::IsFinite(ScrollInterpolationSpeed)
		? FMath::Max(0.0f, ScrollInterpolationSpeed)
		: 0.0f;
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

	if (bWasActive)
	{
		LunarScrollFinishedNative.Broadcast(this, bCompleted);
		if (bBroadcastFinished)
		{
			OnLunarScrollFinished.Broadcast(this);
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
	if (FSlateApplicationBase::IsInitialized()
		&& FSlateApplication::IsInitialized()
		&& MyScrollBox.IsValid()
		&& MyScrollBox->HasMouseCapture())
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
		&& NavigationSubsystem->IsScrollBoxAllowedByNavigationConfinement(this)
		&& ScopeRoot
		&& (this == ScopeRoot || LunarScrollBox_Private::IsDescendantOf(this, ScopeRoot));
}
