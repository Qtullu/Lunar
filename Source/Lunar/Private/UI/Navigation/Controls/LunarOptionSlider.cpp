// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "UI/Navigation/Controls/LunarOptionSlider.h"

#include "UI/Navigation/Controls/SLunarValueControlVisuals.h"
#include "Styling/CoreStyle.h"
#include "UI/Navigation/Types/LunarGameplayTags.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"

/**
 * @file LunarOptionSlider.cpp
 * @brief Discrete Lunar option-slider behavior and native presentation
 * @ingroup LunarNavigationControls
 */

ULunarOptionSlider::ULunarOptionSlider(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCanReceiveLunarSelection = true;
	bCanInteractWithPointer = true;
	ValueFont = FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 10);
}

void ULunarOptionSlider::SetArrowBrush(
	const ELunarOptionSliderArrow Arrow,
	const FSlateBrush& NewBrush)
{
	FSlateBrush& ArrowBrush = Arrow == ELunarOptionSliderArrow::Previous
		? PreviousArrowBrush
		: NextArrowBrush;
	ArrowBrush = NewBrush;
	SynchronizeSpecializedPresentation();
}

FSlateBrush ULunarOptionSlider::GetArrowBrush(const ELunarOptionSliderArrow Arrow) const
{
	return Arrow == ELunarOptionSliderArrow::Previous
		? PreviousArrowBrush
		: NextArrowBrush;
}

void ULunarOptionSlider::SetArrowTint(
	const ELunarOptionSliderArrow Arrow,
	const FLinearColor NewTint)
{
	FLinearColor& ArrowTint = Arrow == ELunarOptionSliderArrow::Previous
		? PreviousArrowTint
		: NextArrowTint;
	ArrowTint = NewTint;
	SynchronizeSpecializedPresentation();
}

FLinearColor ULunarOptionSlider::GetArrowTint(const ELunarOptionSliderArrow Arrow) const
{
	return Arrow == ELunarOptionSliderArrow::Previous
		? PreviousArrowTint
		: NextArrowTint;
}

void ULunarOptionSlider::SetArrowTransform(
	const ELunarOptionSliderArrow Arrow,
	const FWidgetTransform& NewTransform)
{
	FWidgetTransform& ArrowTransform = Arrow == ELunarOptionSliderArrow::Previous
		? PreviousArrowTransform
		: NextArrowTransform;
	ArrowTransform = NewTransform;
	SynchronizeSpecializedPresentation();
}

FWidgetTransform ULunarOptionSlider::GetArrowTransform(const ELunarOptionSliderArrow Arrow) const
{
	return Arrow == ELunarOptionSliderArrow::Previous
		? PreviousArrowTransform
		: NextArrowTransform;
}

void ULunarOptionSlider::SetValueTextColor(const FSlateColor NewColor)
{
	ValueTextColor = NewColor;
	SynchronizeSpecializedPresentation();
}

void ULunarOptionSlider::SetValueFont(const FSlateFontInfo& NewFont)
{
	ValueFont = NewFont;
	SynchronizeSpecializedPresentation();
}

void ULunarOptionSlider::ConfigureArrowPresentation(
	const ELunarOptionSliderArrow Arrow,
	const FSlateBrush& NewBrush,
	const FLinearColor NewTint,
	const FWidgetTransform& NewTransform)
{
	if (Arrow == ELunarOptionSliderArrow::Previous)
	{
		PreviousArrowBrush = NewBrush;
		PreviousArrowTint = NewTint;
		PreviousArrowTransform = NewTransform;
	}
	else
	{
		NextArrowBrush = NewBrush;
		NextArrowTint = NewTint;
		NextArrowTransform = NewTransform;
	}
	SynchronizeSpecializedPresentation();
}

void ULunarOptionSlider::GetArrowPresentation(
	const ELunarOptionSliderArrow Arrow,
	FSlateBrush& OutBrush,
	FLinearColor& OutTint,
	FWidgetTransform& OutTransform) const
{
	if (Arrow == ELunarOptionSliderArrow::Previous)
	{
		OutBrush = PreviousArrowBrush;
		OutTint = PreviousArrowTint;
		OutTransform = PreviousArrowTransform;
	}
	else
	{
		OutBrush = NextArrowBrush;
		OutTint = NextArrowTint;
		OutTransform = NextArrowTransform;
	}
}

ELunarUIInteractionState ULunarOptionSlider::GetArrowInteractionState(
	const ELunarOptionSliderArrow Arrow) const
{
	return Arrow == ELunarOptionSliderArrow::Previous
		? CurrentPreviousArrowInteractionState
		: CurrentNextArrowInteractionState;
}


void ULunarOptionSlider::ConfigureOptionSliderPresentation(
	const FSlateBrush& NewPreviousArrowBrush,
	const FLinearColor NewPreviousArrowTint,
	const FSlateBrush& NewNextArrowBrush,
	const FLinearColor NewNextArrowTint,
	const FSlateColor NewValueTextColor,
	const FSlateFontInfo& NewValueFont)
{
	PreviousArrowBrush = NewPreviousArrowBrush;
	PreviousArrowTint = NewPreviousArrowTint;
	NextArrowBrush = NewNextArrowBrush;
	NextArrowTint = NewNextArrowTint;
	ValueTextColor = NewValueTextColor;
	ValueFont = NewValueFont;
	SynchronizeSpecializedPresentation();
}

void ULunarOptionSlider::GetOptionSliderPresentation(
	FSlateBrush& OutPreviousArrowBrush,
	FLinearColor& OutPreviousArrowTint,
	FSlateBrush& OutNextArrowBrush,
	FLinearColor& OutNextArrowTint,
	FSlateColor& OutValueTextColor,
	FSlateFontInfo& OutValueFont) const
{
	OutPreviousArrowBrush = PreviousArrowBrush;
	OutPreviousArrowTint = PreviousArrowTint;
	OutNextArrowBrush = NextArrowBrush;
	OutNextArrowTint = NextArrowTint;
	OutValueTextColor = ValueTextColor;
	OutValueFont = ValueFont;
}
void ULunarOptionSlider::SetOptions(
	const TArray<FText>& NewOptions,
	const ELunarChangeNotificationPolicy NotificationPolicy)
{
	const bool bNotify = NotificationPolicy == ELunarChangeNotificationPolicy::Notify;
	const int32 PreviousIndex = SelectedIndex;
	const FText PreviousOption = GetSelectedOption();
	Options = NewOptions;

	const int32 NewIndex = NormalizeIndex(PreviousIndex);
	SelectedIndex = NewIndex;
	if (bNotify && PreviousIndex != NewIndex)
	{
		NotifySelectedIndexChanged();
	}

	const FText CurrentOption = GetSelectedOption();
	if (bNotify && !PreviousOption.EqualTo(CurrentOption))
	{
		NotifyLunarAccessibleValueChanged(CurrentOption);
	}
	RefreshLunarAccessibility();
	SynchronizeSpecializedPresentation();
}

void ULunarOptionSlider::SetSelectedIndex(
	const int32 NewSelectedIndex,
	const ELunarChangeNotificationPolicy NotificationPolicy)
{
	ApplySelectedIndex(
		NormalizeIndex(NewSelectedIndex),
		NotificationPolicy == ELunarChangeNotificationPolicy::Notify);
}

FText ULunarOptionSlider::GetSelectedOption() const
{
	return Options.IsValidIndex(SelectedIndex)
		? Options[SelectedIndex]
		: FText::GetEmpty();
}

TSharedRef<SWidget> ULunarOptionSlider::RebuildWidget()
{
	const TSharedRef<SWidget> BasePresentation = Super::RebuildWidget();
	const TSharedRef<SBox> InteractionSurface = SNew(SBox);
	InteractionSurface->SetVisibility(EVisibility::Visible);
	InteractionSurface->SetOnMouseButtonDown(
		BIND_UOBJECT_DELEGATE(FPointerEventHandler, NativeOnMouseButtonDown));
	InteractionSurface->SetOnMouseButtonUp(
		BIND_UOBJECT_DELEGATE(FPointerEventHandler, NativeOnMouseButtonUp));
	InteractionSurface->SetOnMouseDoubleClick(
		BIND_UOBJECT_DELEGATE(FPointerEventHandler, NativeOnMouseButtonDoubleClick));
#if WITH_ACCESSIBILITY
	InteractionSurface->SetAccessibleBehavior(EAccessibleBehavior::NotAccessible);
#endif

	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			BasePresentation
		]
		+ SOverlay::Slot()
		[
			InteractionSurface
		];
}
TSharedPtr<SWidget> ULunarOptionSlider::RebuildLunarSpecializedPresentation()
{
	SAssignNew(OptionSliderVisual, SLunarOptionSliderVisual);
	SynchronizeSpecializedPresentation();
	return OptionSliderVisual;
}

void ULunarOptionSlider::ReleaseSlateResources(const bool bReleaseChildren)
{
	OptionSliderVisual.Reset();
	Super::ReleaseSlateResources(bReleaseChildren);
}

void ULunarOptionSlider::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	ApplySelectedIndex(NormalizeIndex(SelectedIndex), false);
	SetLunarValueState(LunarGameplayTags::UI_State_Value_Normal.GetTag());
	RefreshLunarAccessibility();
	SynchronizeSpecializedPresentation();
}

#if WITH_EDITOR
void ULunarOptionSlider::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName ChangedPropertyName = PropertyChangedEvent.GetPropertyName();
	const FName MemberPropertyName = PropertyChangedEvent.MemberProperty
		? PropertyChangedEvent.MemberProperty->GetFName()
		: NAME_None;
	if (ChangedPropertyName != GET_MEMBER_NAME_CHECKED(ULunarOptionSlider, Options)
		&& MemberPropertyName != GET_MEMBER_NAME_CHECKED(ULunarOptionSlider, Options))
	{
		return;
	}

	for (FText& Option : Options)
	{
		if (Option.IsEmpty() && !Option.IsCultureInvariant())
		{
			Option = FText::AsCultureInvariant(TEXT(""));
		}
	}
}
#endif

void ULunarOptionSlider::NativeOnMouseEnter(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (!InMouseEvent.IsTouchEvent())
	{
		SetHoveredPointerOptionStep(ResolvePointerOptionStep(InGeometry, InMouseEvent.GetScreenSpacePosition()));
	}
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
}

FReply ULunarOptionSlider::NativeOnMouseMove(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (!InMouseEvent.IsTouchEvent())
	{
		SetHoveredPointerOptionStep(ResolvePointerOptionStep(InGeometry, InMouseEvent.GetScreenSpacePosition()));
	}
	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

void ULunarOptionSlider::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	SetHoveredPointerOptionStep(0);
	Super::NativeOnMouseLeave(InMouseEvent);
}

FReply ULunarOptionSlider::NativeOnPreviewMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (bCanInteractWithPointer
		&& !InMouseEvent.IsTouchEvent()
		&& InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}
	return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
}
FReply ULunarOptionSlider::NativeOnMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	PendingPointerOptionStep = 0;
	bPointerOptionPressIsTouch = false;
	if (bCanInteractWithPointer
		&& InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		PendingPointerOptionStep = ResolvePointerOptionStep(
			InGeometry,
			InMouseEvent.GetScreenSpacePosition());
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply ULunarOptionSlider::NativeOnMouseButtonUp(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton
		&& (PendingPointerOptionStep == 0
			|| PendingPointerOptionStep != ResolvePointerOptionStep(
				InGeometry,
				InMouseEvent.GetScreenSpacePosition())))
	{
		PendingPointerOptionStep = 0;
	}
	const FReply Reply = Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
	PendingPointerOptionStep = 0;
	bPointerOptionPressIsTouch = false;
	RefreshArrowVisualState();
	return Reply;
}

void ULunarOptionSlider::NativeOnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent)
{
	PendingPointerOptionStep = 0;
	bPointerOptionPressIsTouch = false;
	Super::NativeOnMouseCaptureLost(CaptureLostEvent);
}

FReply ULunarOptionSlider::NativeOnTouchStarted(
	const FGeometry& InGeometry,
	const FPointerEvent& InGestureEvent)
{
	PendingPointerOptionStep = 0;
	bPointerOptionPressIsTouch = true;
	if (bAllowTouchInput
		&& InGestureEvent.IsTouchEvent()
		&& InGestureEvent.GetPointerIndex() == 0)
	{
		PendingPointerOptionStep = ResolvePointerOptionStep(
			InGeometry,
			InGestureEvent.GetScreenSpacePosition());
	}
	return Super::NativeOnTouchStarted(InGeometry, InGestureEvent);
}

FReply ULunarOptionSlider::NativeOnTouchEnded(
	const FGeometry& InGeometry,
	const FPointerEvent& InGestureEvent)
{
	if (InGestureEvent.GetPointerIndex() == 0
		&& (PendingPointerOptionStep == 0
			|| PendingPointerOptionStep != ResolvePointerOptionStep(
				InGeometry,
				InGestureEvent.GetScreenSpacePosition())))
	{
		PendingPointerOptionStep = 0;
	}
	const FReply Reply = Super::NativeOnTouchEnded(InGeometry, InGestureEvent);
	PendingPointerOptionStep = 0;
	bPointerOptionPressIsTouch = false;
	RefreshArrowVisualState();
	return Reply;
}

bool ULunarOptionSlider::NativeCanHandleLunarAction(const FLunarUIActionContext& ActionContext) const
{
	using namespace LunarGameplayTags;
	const bool bValueAction = ActionContext.ActionTag == UI_Action_Increase.GetTag()
		|| ActionContext.ActionTag == UI_Action_Decrease.GetTag();
	if (!bValueAction)
	{
		return Super::NativeCanHandleLunarAction(ActionContext);
	}

	if (!IsLunarSelected())
	{
		return false;
	}

	if (ActionContext.bHasNavigationDirection)
	{
		FGameplayTag ExpectedAction;
		return NativeResolveDirectionalLunarControlAction(ActionContext.NavigationDirection, ExpectedAction)
			&& ExpectedAction == ActionContext.ActionTag;
	}
	return true;
}

ELunarUIActionResult ULunarOptionSlider::NativeHandleLunarAction(const FLunarUIActionContext& ActionContext)
{
	using namespace LunarGameplayTags;
	const bool bIncrease = ActionContext.ActionTag == UI_Action_Increase.GetTag();
	const bool bDecrease = ActionContext.ActionTag == UI_Action_Decrease.GetTag();
	if (!bIncrease && !bDecrease)
	{
		return Super::NativeHandleLunarAction(ActionContext);
	}
	if (!NativeCanHandleLunarAction(ActionContext))
	{
		return ELunarUIActionResult::Unhandled;
	}
	if (ActionContext.InputEvent == IE_Released)
	{
		ActiveNavigationOptionStep = 0;
	}
	else if (ActionContext.InputEvent == IE_Pressed || ActionContext.InputEvent == IE_Repeat)
	{
		ActiveNavigationOptionStep = bIncrease ? 1 : -1;
	}
	if (!NativeCanActivateLunarWidget())
	{
		return ELunarUIActionResult::Rejected;
	}

	if (ActionContext.InputEvent == IE_Pressed || ActionContext.InputEvent == IE_Repeat)
	{
		const int32 OptionCount = Options.Num();
		if (OptionCount <= 0)
		{
			return ActionContext.bIsRepeat
				? ELunarUIActionResult::Handled
				: ELunarUIActionResult::Rejected;
		}

		const int32 DirectionStep = bIncrease ? 1 : -1;
		const int32 CurrentIndex = Options.IsValidIndex(SelectedIndex)
			? SelectedIndex
			: (bIncrease ? -1 : OptionCount);
		const int32 RequestedIndex = CurrentIndex + DirectionStep;
		const int32 NewIndex = bWrapOptions
			? ((RequestedIndex % OptionCount) + OptionCount) % OptionCount
			: FMath::Clamp(RequestedIndex, 0, OptionCount - 1);
		if (NewIndex == CurrentIndex)
		{
			return ActionContext.bIsRepeat
				? ELunarUIActionResult::Handled
				: ELunarUIActionResult::Rejected;
		}
		ApplySelectedIndex(NewIndex, true);
	}

	// Value-axis actions remain owned by the control so Selection never escapes.
	return ELunarUIActionResult::Handled;
}

bool ULunarOptionSlider::NativeResolveDirectionalLunarControlAction(
	const ELunarNavigationDirection Direction,
	FGameplayTag& OutActionTag) const
{
	const bool bHorizontal = Orientation == Orient_Horizontal;
	const bool bValueAxisDirection = bHorizontal
		? (Direction == ELunarNavigationDirection::Left || Direction == ELunarNavigationDirection::Right)
		: (Direction == ELunarNavigationDirection::Up || Direction == ELunarNavigationDirection::Down);
	if (!bValueAxisDirection)
	{
		OutActionTag = FGameplayTag();
		return false;
	}

	bool bIncrease = Direction == ELunarNavigationDirection::Right
		|| Direction == ELunarNavigationDirection::Up;
	if (bInvertValueDirection)
	{
		bIncrease = !bIncrease;
	}
	OutActionTag = bIncrease
		? LunarGameplayTags::UI_Action_Increase.GetTag()
		: LunarGameplayTags::UI_Action_Decrease.GetTag();
	return true;
}


FText ULunarOptionSlider::NativeGetLunarAccessibleValueText() const
{
	return GetSelectedOption();
}

void ULunarOptionSlider::NativeOnLunarActivated()
{
	const int32 DirectionStep = PendingPointerOptionStep;
	PendingPointerOptionStep = 0;
	if (DirectionStep != 0 && !ApplyPointerOptionStep(DirectionStep))
	{
		NativeOnLunarRejected();
		return;
	}
	Super::NativeOnLunarActivated();
}

void ULunarOptionSlider::NativeOnLunarVisualStateChanged(
	const FLunarUIVisualState& PreviousState,
	const FLunarUIVisualState& NewState,
	const bool bIsDesignerPreview)
{
	LastOwnerVisualState = NewState;
	bLastOwnerVisualStateIsDesignerPreview = bIsDesignerPreview;
	Super::NativeOnLunarVisualStateChanged(PreviousState, NewState, bIsDesignerPreview);
	RefreshArrowVisualState();
}


int32 ULunarOptionSlider::NormalizeIndex(const int32 RequestedIndex) const
{
	return Options.IsEmpty()
		? INDEX_NONE
		: FMath::Clamp(RequestedIndex, 0, Options.Num() - 1);
}

void ULunarOptionSlider::ApplySelectedIndex(const int32 NewSelectedIndex, const bool bNotify)
{
	const int32 NormalizedIndex = NormalizeIndex(NewSelectedIndex);
	if (SelectedIndex == NormalizedIndex)
	{
		return;
	}

	SelectedIndex = NormalizedIndex;
	RefreshLunarAccessibility();
	if (bNotify)
	{
		NotifySelectedIndexChanged();
		NotifyLunarAccessibleValueChanged(GetSelectedOption());
	}
	SynchronizeSpecializedPresentation();
}

void ULunarOptionSlider::NotifySelectedIndexChanged()
{
	const FText SelectedOption = GetSelectedOption();
	BP_OnLunarSelectedIndexChanged(SelectedIndex, SelectedOption);
	OnSelectedIndexChanged.Broadcast(this, SelectedIndex, SelectedOption);
}

void ULunarOptionSlider::SynchronizeSpecializedPresentation()
{
	if (OptionSliderVisual.IsValid())
	{
		OptionSliderVisual->SetPresentation(
			PreviousArrowBrush,
			PreviousArrowTint,
			PreviousArrowTransform,
			NextArrowBrush,
			NextArrowTint,
			NextArrowTransform,
			ValueTextColor,
			ValueFont);
		OptionSliderVisual->SetValue(GetSelectedOption(), Orientation);
	}
}

void ULunarOptionSlider::RefreshArrowVisualState()
{
	ELunarUIInteractionState PreviousArrowState = LastOwnerVisualState.InteractionState;
	ELunarUIInteractionState NextArrowState = LastOwnerVisualState.InteractionState;
	if (!bLastOwnerVisualStateIsDesignerPreview)
	{
		switch (LastOwnerVisualState.InteractionState)
		{
		case ELunarUIInteractionState::PointerNormal:
			PreviousArrowState = ELunarUIInteractionState::PointerNormal;
			NextArrowState = ELunarUIInteractionState::PointerNormal;
			break;

		case ELunarUIInteractionState::PointerHovered:
			PreviousArrowState = HoveredPointerOptionStep < 0
				? ELunarUIInteractionState::PointerHovered
				: ELunarUIInteractionState::PointerNormal;
			NextArrowState = HoveredPointerOptionStep > 0
				? ELunarUIInteractionState::PointerHovered
				: ELunarUIInteractionState::PointerNormal;
			break;

		case ELunarUIInteractionState::PointerPressed:
		{
			const int32 PressedStep = bPointerOptionPressIsTouch
				? PendingPointerOptionStep
				: (HoveredPointerOptionStep == PendingPointerOptionStep ? PendingPointerOptionStep : 0);
			PreviousArrowState = PressedStep < 0
				? ELunarUIInteractionState::PointerPressed
				: ELunarUIInteractionState::PointerNormal;
			NextArrowState = PressedStep > 0
				? ELunarUIInteractionState::PointerPressed
				: ELunarUIInteractionState::PointerNormal;
			break;
		}

		case ELunarUIInteractionState::NavigationPressed:
			PreviousArrowState = ActiveNavigationOptionStep < 0
				? ELunarUIInteractionState::NavigationPressed
				: ELunarUIInteractionState::NavigationNormal;
			NextArrowState = ActiveNavigationOptionStep > 0
				? ELunarUIInteractionState::NavigationPressed
				: ELunarUIInteractionState::NavigationNormal;
			break;

		case ELunarUIInteractionState::NavigationNormal:
		case ELunarUIInteractionState::NavigationSelected:
			PreviousArrowState = ELunarUIInteractionState::NavigationNormal;
			NextArrowState = ELunarUIInteractionState::NavigationNormal;
			break;
		default:
			break;
		}
	}

	const bool bPreviousStateChanged = !bHasPublishedArrowVisualState
		|| CurrentPreviousArrowInteractionState != PreviousArrowState;
	const bool bNextStateChanged = !bHasPublishedArrowVisualState
		|| CurrentNextArrowInteractionState != NextArrowState;
	CurrentPreviousArrowInteractionState = PreviousArrowState;
	CurrentNextArrowInteractionState = NextArrowState;
	if (!bPreviousStateChanged && !bNextStateChanged)
	{
		return;
	}

	bHasPublishedArrowVisualState = true;
	if (bPreviousStateChanged)
	{
		NotifyArrowVisualStateChanged(ELunarOptionSliderArrow::Previous);
	}
	if (bNextStateChanged)
	{
		NotifyArrowVisualStateChanged(ELunarOptionSliderArrow::Next);
	}
}
void ULunarOptionSlider::NotifyArrowVisualStateChanged(
	const ELunarOptionSliderArrow ChangedArrow)
{
	const ELunarOptionSliderArrow OtherArrow = ChangedArrow == ELunarOptionSliderArrow::Previous
		? ELunarOptionSliderArrow::Next
		: ELunarOptionSliderArrow::Previous;
	const ELunarUIInteractionState NewState = GetArrowInteractionState(ChangedArrow);
	const ELunarUIInteractionState OtherArrowState = GetArrowInteractionState(OtherArrow);
	BP_OnLunarArrowVisualStateChanged(
		ChangedArrow,
		NewState,
		OtherArrow,
		OtherArrowState,
		bLastOwnerVisualStateIsDesignerPreview);
	OnArrowVisualStateChanged.Broadcast(
		this,
		ChangedArrow,
		NewState,
		OtherArrow,
		OtherArrowState,
		bLastOwnerVisualStateIsDesignerPreview);
}


void ULunarOptionSlider::SetHoveredPointerOptionStep(const int32 NewHoveredStep)
{
	const int32 NormalizedStep = FMath::Clamp(NewHoveredStep, -1, 1);
	if (HoveredPointerOptionStep == NormalizedStep)
	{
		return;
	}
	HoveredPointerOptionStep = NormalizedStep;
	RefreshArrowVisualState();
}

int32 ULunarOptionSlider::ResolvePointerOptionStep(
	const FGeometry& InGeometry,
	const FVector2D& ScreenPosition) const
{
	FGeometry InteractionGeometry = InGeometry;
	if (OptionSliderVisual.IsValid())
	{
		const FGeometry& PresentationGeometry = OptionSliderVisual->GetCachedGeometry();
		const FVector2D PresentationSize = PresentationGeometry.GetLocalSize();
		if (PresentationSize.X > UE_SMALL_NUMBER && PresentationSize.Y > UE_SMALL_NUMBER)
		{
			InteractionGeometry = PresentationGeometry;
		}
	}
	if (!InteractionGeometry.IsUnderLocation(ScreenPosition))
	{
		return 0;
	}


	const FVector2D LocalSize = InteractionGeometry.GetLocalSize();
	const float AxisExtent = Orientation == Orient_Horizontal ? LocalSize.X : LocalSize.Y;
	if (AxisExtent <= UE_SMALL_NUMBER)
	{
		return 0;
	}

	const FVector2D LocalPosition = InteractionGeometry.AbsoluteToLocal(ScreenPosition);
	const float AxisPosition = Orientation == Orient_Horizontal ? LocalPosition.X : LocalPosition.Y;
	const float AxisFraction = AxisPosition / AxisExtent;
	if (AxisFraction <= 0.25f)
	{
		return -1;
	}
	if (AxisFraction >= 0.75f)
	{
		return 1;
	}
	return 0;
}

bool ULunarOptionSlider::ApplyPointerOptionStep(const int32 DirectionStep)
{
	if (DirectionStep == 0 || Options.IsEmpty())
	{
		return false;
	}

	const int32 CurrentIndex = Options.IsValidIndex(SelectedIndex)
		? SelectedIndex
		: (DirectionStep > 0 ? -1 : Options.Num());
	const int32 RequestedIndex = CurrentIndex + FMath::Clamp(DirectionStep, -1, 1);
	const int32 NewIndex = bWrapOptions
		? ((RequestedIndex % Options.Num()) + Options.Num()) % Options.Num()
		: FMath::Clamp(RequestedIndex, 0, Options.Num() - 1);
	if (NewIndex == CurrentIndex)
	{
		return false;
	}
	ApplySelectedIndex(NewIndex, true);
	return true;
}
