// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "UI/Navigation/Controls/LunarRadio.h"

#include "Curves/CurveFloat.h"
#include "Settings/LunarSettings.h"
#include "UI/Navigation/Controls/LunarRadioSideVisual.h"
#include "UI/Navigation/Controls/SLunarRadioVisual.h"
#include "UI/Navigation/Types/LunarGameplayTags.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"

#if WITH_EDITOR
#include "UObject/UnrealType.h"
#endif

/** @file LunarRadio.cpp @brief Composite Radio generation, input, pointer routing, styles, and Checked interpolation. @ingroup LunarNavigationControls */

const FLunarRadioVisualStyle& FLunarRadioInteractionStyleSet::Resolve(
	const ELunarUIInteractionState InteractionState) const
{
	switch (InteractionState)
	{
	case ELunarUIInteractionState::PointerHovered: return PointerHovered;
	case ELunarUIInteractionState::PointerPressed: return PointerPressed;
	case ELunarUIInteractionState::NavigationNormal: return NavigationNormal;
	case ELunarUIInteractionState::NavigationSelected: return NavigationSelected;
	case ELunarUIInteractionState::NavigationPressed: return NavigationPressed;
	case ELunarUIInteractionState::PointerNormal:
	default: return PointerNormal;
	}
}

void FLunarRadioInteractionStyleSet::Set(
	const ELunarUIInteractionState InteractionState,
	const FLunarRadioVisualStyle& NewStyle)
{
	switch (InteractionState)
	{
	case ELunarUIInteractionState::PointerHovered: PointerHovered = NewStyle; break;
	case ELunarUIInteractionState::PointerPressed: PointerPressed = NewStyle; break;
	case ELunarUIInteractionState::NavigationNormal: NavigationNormal = NewStyle; break;
	case ELunarUIInteractionState::NavigationSelected: NavigationSelected = NewStyle; break;
	case ELunarUIInteractionState::NavigationPressed: NavigationPressed = NewStyle; break;
	case ELunarUIInteractionState::PointerNormal:
	default: PointerNormal = NewStyle; break;
	}
}

ULunarRadio::ULunarRadio(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCanReceiveLunarSelection = true;
	bCanInteractWithPointer = true;
	bEnableInputPrompt = false;
	NormalizeSideVisualData(NumOfRadioButtons);

	static ConstructorHelpers::FObjectFinder<UCurveFloat> DefaultInterpolationCurve(
		TEXT("/Lunar/Curves/Float/CF_EaseOutExpo.CF_EaseOutExpo"));
	if (DefaultInterpolationCurve.Succeeded())
	{
		SelectionInterpolationCurve = DefaultInterpolationCurve.Object;
	}
}

void ULunarRadio::SetNumOfRadioButtons(
	const int32 NewNumOfRadioButtons,
	const ELunarChangeNotificationPolicy NotificationPolicy)
{
	const int32 PreviousIndex = SelectedIndex;
	NumOfRadioButtons = FMath::Max(1, NewNumOfRadioButtons);
	NormalizeConfiguration();
	SynchronizeGeneratedSideVisuals();
	SnapDisplayedSelectionToLogical(false);
	SynchronizeRadioPresentation();
	RefreshLunarAccessibility();
	if (PreviousIndex != SelectedIndex && NotificationPolicy == ELunarChangeNotificationPolicy::Notify)
	{
		NotifySelectedIndexChanged(PreviousIndex, SelectedIndex);
		NotifyLunarAccessibleValueChanged(NativeGetLunarAccessibleValueText());
	}
}

void ULunarRadio::SetSelectedIndex(
	const int32 NewSelectedIndex,
	const ELunarChangeNotificationPolicy NotificationPolicy)
{
	ApplySelectedIndex(
		NewSelectedIndex,
		NotificationPolicy == ELunarChangeNotificationPolicy::Notify,
		false);
}

bool ULunarRadio::SetSelectedByStringValue(
	const FString& StringValue,
	const ELunarChangeNotificationPolicy NotificationPolicy)
{
	if (StringValue.IsEmpty())
	{
		return false;
	}

	int32 MatchingIndex = INDEX_NONE;
	for (int32 OptionIndex = 0; OptionIndex < SideVisualData.Num(); ++OptionIndex)
	{
		if (SideVisualData[OptionIndex].StringValue != StringValue)
		{
			continue;
		}
		if (MatchingIndex != INDEX_NONE)
		{
			return false;
		}
		MatchingIndex = OptionIndex;
	}
	if (MatchingIndex == INDEX_NONE)
	{
		return false;
	}
	ApplySelectedIndex(
		MatchingIndex,
		NotificationPolicy == ELunarChangeNotificationPolicy::Notify,
		false);
	return true;
}

FString ULunarRadio::GetSelectedStringValue() const
{
	return SideVisualData.IsValidIndex(SelectedIndex)
		? SideVisualData[SelectedIndex].StringValue
		: FString();
}

FLunarRadioSideVisualData ULunarRadio::GetSelectedData() const
{
	return GetSideVisualDataAt(SelectedIndex);
}

bool ULunarRadio::SetSideVisualDataAt(
	const int32 OptionIndex,
	const FLunarRadioSideVisualData& NewData)
{
	if (!SideVisualData.IsValidIndex(OptionIndex))
	{
		return false;
	}

	const FLunarRadioSideVisualData& ExistingData = SideVisualData[OptionIndex];
	if (ExistingData.StringValue == NewData.StringValue
		&& ExistingData.DisplayText.EqualTo(NewData.DisplayText))
	{
		return true;
	}

	SideVisualData[OptionIndex] = NewData;
	if (GeneratedSideVisuals.IsValidIndex(OptionIndex) && IsValid(GeneratedSideVisuals[OptionIndex]))
	{
		GeneratedSideVisuals[OptionIndex]->ApplyDataFromRadio(NewData);
	}
	if (OptionIndex == SelectedIndex)
	{
		RefreshLunarAccessibility();
	}
	return true;
}

FLunarRadioSideVisualData ULunarRadio::GetSideVisualDataAt(const int32 OptionIndex) const
{
	return SideVisualData.IsValidIndex(OptionIndex)
		? SideVisualData[OptionIndex]
		: FLunarRadioSideVisualData();
}

ULunarRadioSideVisual* ULunarRadio::GetSideVisualAt(const int32 OptionIndex) const
{
	return GeneratedSideVisuals.IsValidIndex(OptionIndex)
		? GeneratedSideVisuals[OptionIndex]
		: nullptr;
}

float ULunarRadio::GetDisplayedSelectionPosition() const
{
	return bDisplayedSelectionPositionInitialized
		? DisplayedSelectionPosition
		: static_cast<float>(SelectedIndex);
}

void ULunarRadio::SetUncheckedStyle(
	const ELunarUIInteractionState InteractionState,
	const FLunarRadioVisualStyle& NewStyle)
{
	UncheckedStyles.Set(InteractionState, NewStyle);
	RefreshOptionVisualStates();
}

FLunarRadioVisualStyle ULunarRadio::GetUncheckedStyle(
	const ELunarUIInteractionState InteractionState) const
{
	return UncheckedStyles.Resolve(InteractionState);
}

void ULunarRadio::SetCheckedIndicatorStyle(
	const ELunarUIInteractionState InteractionState,
	const FLunarRadioVisualStyle& NewStyle)
{
	CheckedIndicatorStyles.Set(InteractionState, NewStyle);
	RefreshOptionVisualStates();
}

FLunarRadioVisualStyle ULunarRadio::GetCheckedIndicatorStyle(
	const ELunarUIInteractionState InteractionState) const
{
	return CheckedIndicatorStyles.Resolve(InteractionState);
}

TSharedRef<SWidget> ULunarRadio::RebuildWidget()
{
	const TSharedRef<SWidget> BasePresentation = Super::RebuildWidget();
	const TSharedRef<SBox> InteractionSurface = SNew(SBox);
	InteractionSurface->SetVisibility(EVisibility::Visible);
	InteractionSurface->SetOnMouseButtonDown(BIND_UOBJECT_DELEGATE(FPointerEventHandler, NativeOnMouseButtonDown));
	InteractionSurface->SetOnMouseButtonUp(BIND_UOBJECT_DELEGATE(FPointerEventHandler, NativeOnMouseButtonUp));
	InteractionSurface->SetOnMouseDoubleClick(BIND_UOBJECT_DELEGATE(FPointerEventHandler, NativeOnMouseButtonDoubleClick));
#if WITH_ACCESSIBILITY
	InteractionSurface->SetAccessibleBehavior(EAccessibleBehavior::NotAccessible);
#endif

	return SNew(SOverlay)
		+ SOverlay::Slot()[BasePresentation]
		+ SOverlay::Slot()[InteractionSurface];
}

TSharedPtr<SWidget> ULunarRadio::RebuildLunarSpecializedPresentation()
{
	NormalizeConfiguration();
	SAssignNew(RadioVisual, SLunarRadioVisual);
	SynchronizeGeneratedSideVisuals();
	SnapDisplayedSelectionToLogical(false);
	SynchronizeRadioPresentation();
	return RadioVisual;
}

void ULunarRadio::ReleaseSlateResources(const bool bReleaseChildren)
{
	RadioVisual.Reset();
	GeneratedSideVisuals.Reset();
	GeneratedSideVisualClass = nullptr;
	Super::ReleaseSlateResources(bReleaseChildren);
}

void ULunarRadio::SynchronizeProperties()
{
	NormalizeConfiguration();
	SetLunarValueState(LunarGameplayTags::UI_State_Value_Normal.GetTag());
	Super::SynchronizeProperties();
	SynchronizeGeneratedSideVisuals();
	SnapDisplayedSelectionToLogical(false);
	SynchronizeRadioPresentation();
	RefreshLunarAccessibility();
}

void ULunarRadio::NativeConstruct()
{
	Super::NativeConstruct();
	SynchronizeGeneratedSideVisuals();
	SnapDisplayedSelectionToLogical(false);
	SynchronizeRadioPresentation();
}

void ULunarRadio::NativeDestruct()
{
	PendingPointerOptionIndex = INDEX_NONE;
	HoveredOptionIndex = INDEX_NONE;
	ActiveNavigationOptionIndex = INDEX_NONE;
	ActiveNavigationDirectionStep = 0;
	bPointerOptionPressIsTouch = false;
	bSelectionInterpolationActive = false;
	bWrapFadeTransitionActive = false;
	Super::NativeDestruct();
}

void ULunarRadio::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	UpdateSelectionInterpolation(InDeltaTime);
	if (RadioVisual.IsValid())
	{
		RadioVisual->RefreshCheckedGeometry();
	}
}

void ULunarRadio::NativeOnMouseEnter(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (!InMouseEvent.IsTouchEvent())
	{
		SetHoveredOptionIndex(ResolvePointerOptionIndex(InMouseEvent.GetScreenSpacePosition()));
	}
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
}

FReply ULunarRadio::NativeOnMouseMove(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (!InMouseEvent.IsTouchEvent())
	{
		SetHoveredOptionIndex(ResolvePointerOptionIndex(InMouseEvent.GetScreenSpacePosition()));
	}
	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

void ULunarRadio::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	SetHoveredOptionIndex(INDEX_NONE);
	Super::NativeOnMouseLeave(InMouseEvent);
}

FReply ULunarRadio::NativeOnPreviewMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (bCanInteractWithPointer
		&& !InMouseEvent.IsTouchEvent()
		&& InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton
		&& ResolvePointerOptionIndex(InMouseEvent.GetScreenSpacePosition()) != INDEX_NONE)
	{
		return NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}
	return FReply::Unhandled();
}

FReply ULunarRadio::NativeOnMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	PendingPointerOptionIndex = INDEX_NONE;
	bPointerOptionPressIsTouch = false;
	if (!bCanInteractWithPointer || InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Unhandled();
	}

	PendingPointerOptionIndex = ResolvePointerOptionIndex(InMouseEvent.GetScreenSpacePosition());
	if (PendingPointerOptionIndex == INDEX_NONE)
	{
		return FReply::Unhandled();
	}
	SetHoveredOptionIndex(PendingPointerOptionIndex);
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply ULunarRadio::NativeOnMouseButtonUp(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton
		&& PendingPointerOptionIndex != ResolvePointerOptionIndex(InMouseEvent.GetScreenSpacePosition()))
	{
		PendingPointerOptionIndex = INDEX_NONE;
	}
	const FReply Reply = Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
	PendingPointerOptionIndex = INDEX_NONE;
	bPointerOptionPressIsTouch = false;
	RefreshOptionVisualStates();
	return Reply;
}

void ULunarRadio::NativeOnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent)
{
	PendingPointerOptionIndex = INDEX_NONE;
	bPointerOptionPressIsTouch = false;
	Super::NativeOnMouseCaptureLost(CaptureLostEvent);
	RefreshOptionVisualStates();
}

FReply ULunarRadio::NativeOnTouchStarted(
	const FGeometry& InGeometry,
	const FPointerEvent& InGestureEvent)
{
	PendingPointerOptionIndex = INDEX_NONE;
	bPointerOptionPressIsTouch = true;
	if (!bAllowTouchInput || !InGestureEvent.IsTouchEvent() || InGestureEvent.GetPointerIndex() != 0)
	{
		return FReply::Unhandled();
	}

	PendingPointerOptionIndex = ResolvePointerOptionIndex(InGestureEvent.GetScreenSpacePosition());
	if (PendingPointerOptionIndex == INDEX_NONE)
	{
		bPointerOptionPressIsTouch = false;
		return FReply::Unhandled();
	}
	return Super::NativeOnTouchStarted(InGeometry, InGestureEvent);
}

FReply ULunarRadio::NativeOnTouchEnded(
	const FGeometry& InGeometry,
	const FPointerEvent& InGestureEvent)
{
	if (InGestureEvent.GetPointerIndex() == 0
		&& PendingPointerOptionIndex != ResolvePointerOptionIndex(InGestureEvent.GetScreenSpacePosition()))
	{
		PendingPointerOptionIndex = INDEX_NONE;
	}
	const FReply Reply = Super::NativeOnTouchEnded(InGeometry, InGestureEvent);
	PendingPointerOptionIndex = INDEX_NONE;
	bPointerOptionPressIsTouch = false;
	RefreshOptionVisualStates();
	return Reply;
}

bool ULunarRadio::NativeCanHandleLunarAction(const FLunarUIActionContext& ActionContext) const
{
	using namespace LunarGameplayTags;
	const bool bSelectionAction = ActionContext.ActionTag == UI_Action_Increase.GetTag()
		|| ActionContext.ActionTag == UI_Action_Decrease.GetTag();
	if (!bSelectionAction)
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

ELunarUIActionResult ULunarRadio::NativeHandleLunarAction(const FLunarUIActionContext& ActionContext)
{
	using namespace LunarGameplayTags;
	const bool bIncrease = ActionContext.ActionTag == UI_Action_Increase.GetTag();
	const bool bDecrease = ActionContext.ActionTag == UI_Action_Decrease.GetTag();
	if (!bIncrease && !bDecrease)
	{
		return Super::NativeHandleLunarAction(ActionContext);
	}

	if (ActionContext.InputEvent == IE_Released)
	{
		ActiveNavigationOptionIndex = INDEX_NONE;
		ActiveNavigationDirectionStep = 0;
		RefreshOptionVisualStates();
		return ELunarUIActionResult::Handled;
	}
	if (!NativeCanHandleLunarAction(ActionContext))
	{
		return ELunarUIActionResult::Unhandled;
	}
	if (!NativeCanActivateLunarWidget())
	{
		return ELunarUIActionResult::Rejected;
	}

	if (ActionContext.InputEvent == IE_Pressed || ActionContext.InputEvent == IE_Repeat)
	{
		const int32 DirectionStep = bIncrease ? 1 : -1;
		const int32 RequestedIndex = SelectedIndex + DirectionStep;
		const bool bAtBlockedEdge = !bWrapSelection
			&& (RequestedIndex < 0 || RequestedIndex >= NumOfRadioButtons);
		if (bAtBlockedEdge)
		{
			return ActiveNavigationDirectionStep == DirectionStep || ActionContext.bIsRepeat
				? ELunarUIActionResult::Handled
				: ELunarUIActionResult::Unhandled;
		}

		ActiveNavigationDirectionStep = DirectionStep;
		ApplySelectionStep(DirectionStep, true);
		ActiveNavigationOptionIndex = SelectedIndex;
	}
	return ELunarUIActionResult::Handled;
}

bool ULunarRadio::NativeResolveDirectionalLunarControlAction(
	const ELunarNavigationDirection Direction,
	FGameplayTag& OutActionTag) const
{
	const bool bHorizontal = Orientation == Orient_Horizontal;
	const bool bInternalAxis = bHorizontal
		? (Direction == ELunarNavigationDirection::Left || Direction == ELunarNavigationDirection::Right)
		: (Direction == ELunarNavigationDirection::Up || Direction == ELunarNavigationDirection::Down);
	if (!bInternalAxis)
	{
		OutActionTag = FGameplayTag();
		return false;
	}

	const bool bIncrease = Direction == ELunarNavigationDirection::Right
		|| Direction == ELunarNavigationDirection::Down;
	const int32 DirectionStep = bIncrease ? 1 : -1;
	const bool bCanMove = bWrapSelection
		|| (SelectedIndex + DirectionStep >= 0 && SelectedIndex + DirectionStep < NumOfRadioButtons)
		|| ActiveNavigationDirectionStep == DirectionStep;
	if (!bCanMove)
	{
		OutActionTag = FGameplayTag();
		return false;
	}

	OutActionTag = bIncrease
		? LunarGameplayTags::UI_Action_Increase.GetTag()
		: LunarGameplayTags::UI_Action_Decrease.GetTag();
	return true;
}

void ULunarRadio::NativeOnLunarSelected()
{
	Super::NativeOnLunarSelected();
	RefreshOptionVisualStates();
}

void ULunarRadio::NativeOnLunarUnselected()
{
	ActiveNavigationOptionIndex = INDEX_NONE;
	ActiveNavigationDirectionStep = 0;
	Super::NativeOnLunarUnselected();
	RefreshOptionVisualStates();
}

void ULunarRadio::NativeOnLunarPressed()
{
	Super::NativeOnLunarPressed();
	RefreshOptionVisualStates();
}

void ULunarRadio::NativeOnLunarReleased()
{
	ActiveNavigationOptionIndex = INDEX_NONE;
	ActiveNavigationDirectionStep = 0;
	Super::NativeOnLunarReleased();
	RefreshOptionVisualStates();
}

void ULunarRadio::NativeOnLunarActivated()
{
	const int32 PointerOptionIndex = PendingPointerOptionIndex;
	PendingPointerOptionIndex = INDEX_NONE;
	if (!SideVisualData.IsValidIndex(PointerOptionIndex))
	{
		return;
	}
	ApplySelectedIndex(PointerOptionIndex, true, false);
	Super::NativeOnLunarActivated();
}

void ULunarRadio::NativeOnLunarVisualStateChanged(
	const FLunarUIVisualState& PreviousState,
	const FLunarUIVisualState& NewState,
	const bool bIsDesignerPreview)
{
	LastOwnerVisualState = NewState;
	bLastOwnerVisualStateIsDesignerPreview = bIsDesignerPreview;
	Super::NativeOnLunarVisualStateChanged(PreviousState, NewState, bIsDesignerPreview);
	if (NewState.bReduceMotion && bSelectionInterpolationActive)
	{
		SnapDisplayedSelectionToLogical(true);
	}
	RefreshOptionVisualStates();
}

FText ULunarRadio::NativeGetLunarAccessibleValueText() const
{
	if (!SideVisualData.IsValidIndex(SelectedIndex))
	{
		return FText::GetEmpty();
	}
	const FLunarRadioSideVisualData& Data = SideVisualData[SelectedIndex];
	return !Data.DisplayText.IsEmpty()
		? Data.DisplayText
		: FText::AsCultureInvariant(Data.StringValue);
}

#if WITH_EDITOR
void ULunarRadio::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	NormalizeConfiguration();
	SynchronizeGeneratedSideVisuals();
	SnapDisplayedSelectionToLogical(false);
	SynchronizeRadioPresentation();
}
#endif

void ULunarRadio::NormalizeConfiguration()
{
	NumOfRadioButtons = FMath::Max(1, NumOfRadioButtons);
	SelectedIndex = FMath::Clamp(SelectedIndex, 0, NumOfRadioButtons - 1);
	ButtonSize.X = FMath::IsFinite(ButtonSize.X) ? FMath::Max(0.0f, ButtonSize.X) : 0.0f;
	ButtonSize.Y = FMath::IsFinite(ButtonSize.Y) ? FMath::Max(0.0f, ButtonSize.Y) : 0.0f;
	ButtonSpacing = FMath::IsFinite(ButtonSpacing) ? FMath::Max(0.0f, ButtonSpacing) : 0.0f;
	SideVisualSpacing.X = FMath::IsFinite(SideVisualSpacing.X) ? FMath::Max(0.0f, SideVisualSpacing.X) : 0.0f;
	SideVisualSpacing.Y = FMath::IsFinite(SideVisualSpacing.Y) ? FMath::Max(0.0f, SideVisualSpacing.Y) : 0.0f;
	SelectionInterpolationSpeed = FMath::IsFinite(SelectionInterpolationSpeed)
		? FMath::Max(0.0f, SelectionInterpolationSpeed)
		: 0.0f;
	NormalizeSideVisualData(NumOfRadioButtons);
}

void ULunarRadio::NormalizeSideVisualData(const int32 RequiredCount)
{
	const int32 PreviousCount = SideVisualData.Num();
	SideVisualData.SetNum(FMath::Max(1, RequiredCount));
	for (int32 OptionIndex = PreviousCount; OptionIndex < SideVisualData.Num(); ++OptionIndex)
	{
		SideVisualData[OptionIndex].StringValue = FString::Printf(TEXT("Radio_%d"), OptionIndex + 1);
		SideVisualData[OptionIndex].DisplayText = FText::AsCultureInvariant(TEXT(""));
	}
}

void ULunarRadio::SynchronizeGeneratedSideVisuals()
{
	if (!RadioVisual.IsValid())
	{
		return;
	}

	bool bMustRebuild = GeneratedSideVisualClass != SideVisualClass
		|| GeneratedSideVisuals.Num() != NumOfRadioButtons;
	if (!bMustRebuild && SideVisualClass)
	{
		for (ULunarRadioSideVisual* SideVisual : GeneratedSideVisuals)
		{
			if (!IsValid(SideVisual) || !SideVisual->IsA(SideVisualClass))
			{
				bMustRebuild = true;
				break;
			}
		}
	}

	if (bMustRebuild)
	{
		GeneratedSideVisuals.SetNum(NumOfRadioButtons);
		TArray<TSharedPtr<SWidget>> SideSlateWidgets;
		SideSlateWidgets.SetNum(NumOfRadioButtons);
		for (int32 OptionIndex = 0; OptionIndex < NumOfRadioButtons; ++OptionIndex)
		{
			GeneratedSideVisuals[OptionIndex] = nullptr;
			if (!SideVisualClass)
			{
				continue;
			}

			ULunarRadioSideVisual* SideVisual = CreateWidget<ULunarRadioSideVisual>(this, SideVisualClass);
			if (!IsValid(SideVisual))
			{
				continue;
			}
			const FLunarUIVisualState InitialState = ResolveOptionVisualState(OptionIndex);
			SideVisual->InitializeFromRadio(
				this,
				OptionIndex,
				SideVisualData[OptionIndex],
				InitialState,
				OptionIndex == SelectedIndex);
			SideVisual->SetVisibility(ESlateVisibility::HitTestInvisible);
			GeneratedSideVisuals[OptionIndex] = SideVisual;
			SideSlateWidgets[OptionIndex] = SideVisual->TakeWidget();
		}
		GeneratedSideVisualClass = SideVisualClass;
		RadioVisual->SetItems(NumOfRadioButtons, SideSlateWidgets);
	}

	for (int32 OptionIndex = 0; OptionIndex < GeneratedSideVisuals.Num(); ++OptionIndex)
	{
		if (IsValid(GeneratedSideVisuals[OptionIndex]))
		{
			GeneratedSideVisuals[OptionIndex]->ApplyDataFromRadio(SideVisualData[OptionIndex]);
		}
	}
}

void ULunarRadio::SynchronizeRadioPresentation()
{
	if (!RadioVisual.IsValid())
	{
		return;
	}
	RadioVisual->SetLayout(
		Orientation,
		ButtonSize,
		ButtonSpacing,
		SideVisualPlacement,
		SideVisualSpacing);
	RefreshOptionVisualStates();
	RadioVisual->SetDisplayedSelection(GetDisplayedSelectionPosition(), DisplayedCheckedOpacity);
}

bool ULunarRadio::ApplySelectedIndex(
	const int32 NewIndex,
	const bool bNotify,
	const bool bWrapTransition)
{
	const int32 NormalizedIndex = FMath::Clamp(NewIndex, 0, NumOfRadioButtons - 1);
	if (SelectedIndex == NormalizedIndex)
	{
		RefreshOptionVisualStates();
		return false;
	}

	const int32 PreviousIndex = SelectedIndex;
	SelectedIndex = NormalizedIndex;
	BeginSelectionTransition(PreviousIndex, SelectedIndex, bWrapTransition);
	RefreshOptionVisualStates();
	RefreshLunarAccessibility();
	if (bNotify)
	{
		NotifySelectedIndexChanged(PreviousIndex, SelectedIndex);
		NotifyLunarAccessibleValueChanged(NativeGetLunarAccessibleValueText());
	}
	return true;
}

bool ULunarRadio::ApplySelectionStep(const int32 DirectionStep, const bool bNotify)
{
	const int32 RequestedIndex = SelectedIndex + FMath::Clamp(DirectionStep, -1, 1);
	const bool bWrapTransition = RequestedIndex < 0 || RequestedIndex >= NumOfRadioButtons;
	const int32 NewIndex = bWrapSelection
		? ((RequestedIndex % NumOfRadioButtons) + NumOfRadioButtons) % NumOfRadioButtons
		: FMath::Clamp(RequestedIndex, 0, NumOfRadioButtons - 1);
	return ApplySelectedIndex(NewIndex, bNotify, bWrapTransition && bWrapSelection);
}

void ULunarRadio::NotifySelectedIndexChanged(
	const int32 PreviousIndex,
	const int32 NewIndex)
{
	const FLunarRadioSideVisualData SelectedData = GetSelectedData();
	BP_OnLunarSelectedIndexChanged(PreviousIndex, NewIndex, SelectedData);
	OnSelectedIndexChanged.Broadcast(this, PreviousIndex, NewIndex, SelectedData);
}

FLunarUIVisualState ULunarRadio::ResolveOptionVisualState(const int32 OptionIndex) const
{
	FLunarUIVisualState Result = LastOwnerVisualState;
	const bool bChecked = OptionIndex == SelectedIndex;
	if (Result.ValueStateTag != LunarGameplayTags::UI_State_Value_Disabled.GetTag())
	{
		Result.ValueStateTag = bChecked
			? LunarGameplayTags::UI_State_Value_Checked.GetTag()
			: LunarGameplayTags::UI_State_Value_Unchecked.GetTag();
	}

	const ELunarUIInteractionState OwnerInteraction = LastOwnerVisualState.InteractionState;
	const bool bOwnerPointerState = OwnerInteraction == ELunarUIInteractionState::PointerNormal
		|| OwnerInteraction == ELunarUIInteractionState::PointerHovered
		|| OwnerInteraction == ELunarUIInteractionState::PointerPressed;
	const bool bLocalPointerInteraction = !bLastOwnerVisualStateIsDesignerPreview
		&& (HoveredOptionIndex != INDEX_NONE || PendingPointerOptionIndex != INDEX_NONE);
	const bool bPointerState = bOwnerPointerState || bLocalPointerInteraction;
	if (bPointerState)
	{
		const int32 EffectiveHoveredIndex = bLastOwnerVisualStateIsDesignerPreview
			? SelectedIndex
			: HoveredOptionIndex;
		const int32 EffectivePressedIndex = bLastOwnerVisualStateIsDesignerPreview
			? SelectedIndex
			: (bPointerOptionPressIsTouch || HoveredOptionIndex == PendingPointerOptionIndex
				? PendingPointerOptionIndex
				: INDEX_NONE);
		const bool bPointerPressActive = OwnerInteraction == ELunarUIInteractionState::PointerPressed
			|| PendingPointerOptionIndex != INDEX_NONE;
		Result.InteractionState = bPointerPressActive
			&& OptionIndex == EffectivePressedIndex
			? ELunarUIInteractionState::PointerPressed
			: (OptionIndex == EffectiveHoveredIndex
				? ELunarUIInteractionState::PointerHovered
				: ELunarUIInteractionState::PointerNormal);
		return Result;
	}

	if (OwnerInteraction == ELunarUIInteractionState::NavigationPressed)
	{
		const int32 PressedIndex = ActiveNavigationOptionIndex != INDEX_NONE
			? ActiveNavigationOptionIndex
			: (bLastOwnerVisualStateIsDesignerPreview ? SelectedIndex : INDEX_NONE);
		Result.InteractionState = OptionIndex == PressedIndex
			? ELunarUIInteractionState::NavigationPressed
			: ELunarUIInteractionState::NavigationNormal;
	}
	else if (OwnerInteraction == ELunarUIInteractionState::NavigationSelected && bChecked)
	{
		Result.InteractionState = ELunarUIInteractionState::NavigationSelected;
	}
	else
	{
		Result.InteractionState = ELunarUIInteractionState::NavigationNormal;
	}
	return Result;
}

void ULunarRadio::RefreshOptionVisualStates()
{
	for (int32 OptionIndex = 0; OptionIndex < NumOfRadioButtons; ++OptionIndex)
	{
		const FLunarUIVisualState OptionState = ResolveOptionVisualState(OptionIndex);
		if (RadioVisual.IsValid())
		{
			RadioVisual->SetUncheckedStyle(OptionIndex, UncheckedStyles.Resolve(OptionState.InteractionState));
		}
		if (GeneratedSideVisuals.IsValidIndex(OptionIndex) && IsValid(GeneratedSideVisuals[OptionIndex]))
		{
			GeneratedSideVisuals[OptionIndex]->ApplyVisualStateFromRadio(OptionState, OptionIndex == SelectedIndex);
		}
	}

	if (RadioVisual.IsValid() && NumOfRadioButtons > 0)
	{
		const FLunarUIVisualState CheckedState = ResolveOptionVisualState(SelectedIndex);
		RadioVisual->SetCheckedStyle(CheckedIndicatorStyles.Resolve(CheckedState.InteractionState));
		RadioVisual->SetDisplayedSelection(GetDisplayedSelectionPosition(), DisplayedCheckedOpacity);
	}
}

void ULunarRadio::SetHoveredOptionIndex(const int32 NewHoveredIndex)
{
	const int32 NormalizedIndex = NewHoveredIndex >= 0 && NewHoveredIndex < NumOfRadioButtons
		? NewHoveredIndex
		: INDEX_NONE;
	if (HoveredOptionIndex == NormalizedIndex)
	{
		return;
	}
	HoveredOptionIndex = NormalizedIndex;
	RefreshOptionVisualStates();
}

int32 ULunarRadio::ResolvePointerOptionIndex(const FVector2D& ScreenPosition) const
{
	return RadioVisual.IsValid()
		? RadioVisual->ResolveOptionIndex(ScreenPosition)
		: INDEX_NONE;
}

void ULunarRadio::BeginSelectionTransition(
	const int32 PreviousIndex,
	const int32 NewIndex,
	const bool bWrapTransition)
{
	if (MustSnapSelectionPresentation() || PreviousIndex == NewIndex)
	{
		SnapDisplayedSelectionToLogical(true);
		return;
	}

	if (!bDisplayedSelectionPositionInitialized)
	{
		DisplayedSelectionPosition = static_cast<float>(PreviousIndex);
		bDisplayedSelectionPositionInitialized = true;
	}
	SelectionInterpolationSource = GetDisplayedSelectionPosition();
	SelectionInterpolationTarget = static_cast<float>(NewIndex);
	SelectionInterpolationElapsed = 0.0f;
	WrapFadeSourceOpacity = DisplayedCheckedOpacity;
	bSelectionInterpolationActive = true;
	bWrapFadeTransitionActive = bWrapTransition;
	bWrapTargetPositionApplied = false;
	if (!bWrapFadeTransitionActive)
	{
		SetDisplayedCheckedOpacity(1.0f);
	}
}

void ULunarRadio::UpdateSelectionInterpolation(const float DeltaTime)
{
	if (!bSelectionInterpolationActive)
	{
		return;
	}
	if (MustSnapSelectionPresentation())
	{
		SnapDisplayedSelectionToLogical(true);
		return;
	}
	if (!FMath::IsFinite(DeltaTime) || DeltaTime <= 0.0f)
	{
		return;
	}

	SelectionInterpolationElapsed += DeltaTime;
	const float Progress = FMath::Clamp(SelectionInterpolationElapsed * SelectionInterpolationSpeed, 0.0f, 1.0f);
	if (bWrapFadeTransitionActive)
	{
		if (Progress < 0.5f)
		{
			const float PhaseAlpha = EvaluateSelectionCurve(Progress * 2.0f);
			SetDisplayedCheckedOpacity(FMath::Lerp(WrapFadeSourceOpacity, 0.0f, PhaseAlpha));
		}
		else
		{
			if (!bWrapTargetPositionApplied)
			{
				SetDisplayedSelectionPositionInternal(SelectionInterpolationTarget, true);
				bWrapTargetPositionApplied = true;
			}
			const float PhaseAlpha = EvaluateSelectionCurve((Progress - 0.5f) * 2.0f);
			SetDisplayedCheckedOpacity(PhaseAlpha);
		}
	}
	else
	{
		const float CurveAlpha = EvaluateSelectionCurve(Progress);
		SetDisplayedSelectionPositionInternal(
			Progress >= 1.0f
				? SelectionInterpolationTarget
				: FMath::Lerp(SelectionInterpolationSource, SelectionInterpolationTarget, CurveAlpha),
			true);
	}

	if (Progress >= 1.0f)
	{
		bSelectionInterpolationActive = false;
		bWrapFadeTransitionActive = false;
		bWrapTargetPositionApplied = false;
		SetDisplayedSelectionPositionInternal(SelectionInterpolationTarget, true);
		SetDisplayedCheckedOpacity(1.0f);
	}
}

void ULunarRadio::SetDisplayedSelectionPositionInternal(
	const float NewDisplayedPosition,
	const bool bBroadcast)
{
	const float SafePosition = FMath::IsFinite(NewDisplayedPosition)
		? FMath::Clamp(NewDisplayedPosition, 0.0f, static_cast<float>(NumOfRadioButtons - 1))
		: static_cast<float>(SelectedIndex);
	const bool bChanged = !bDisplayedSelectionPositionInitialized
		|| !FMath::IsNearlyEqual(DisplayedSelectionPosition, SafePosition);
	DisplayedSelectionPosition = SafePosition;
	bDisplayedSelectionPositionInitialized = true;
	if (RadioVisual.IsValid())
	{
		RadioVisual->SetDisplayedSelection(DisplayedSelectionPosition, DisplayedCheckedOpacity);
	}
	if (bChanged && bBroadcast)
	{
		OnDisplayedSelectionPositionChanged.Broadcast(this, DisplayedSelectionPosition);
	}
}

void ULunarRadio::SetDisplayedCheckedOpacity(const float NewOpacity)
{
	DisplayedCheckedOpacity = FMath::IsFinite(NewOpacity)
		? FMath::Clamp(NewOpacity, 0.0f, 1.0f)
		: 1.0f;
	if (RadioVisual.IsValid())
	{
		RadioVisual->SetDisplayedSelection(GetDisplayedSelectionPosition(), DisplayedCheckedOpacity);
	}
}

void ULunarRadio::SnapDisplayedSelectionToLogical(const bool bBroadcastPosition)
{
	bSelectionInterpolationActive = false;
	bWrapFadeTransitionActive = false;
	bWrapTargetPositionApplied = false;
	SelectionInterpolationSource = static_cast<float>(SelectedIndex);
	SelectionInterpolationTarget = static_cast<float>(SelectedIndex);
	SelectionInterpolationElapsed = 0.0f;
	SetDisplayedSelectionPositionInternal(static_cast<float>(SelectedIndex), bBroadcastPosition);
	SetDisplayedCheckedOpacity(1.0f);
}

float ULunarRadio::EvaluateSelectionCurve(const float Progress) const
{
	const float SafeProgress = FMath::Clamp(Progress, 0.0f, 1.0f);
	if (!IsValid(SelectionInterpolationCurve))
	{
		return SafeProgress;
	}
	const float EvaluatedAlpha = SelectionInterpolationCurve->GetFloatValue(SafeProgress);
	return FMath::IsFinite(EvaluatedAlpha)
		? FMath::Clamp(EvaluatedAlpha, 0.0f, 1.0f)
		: SafeProgress;
}

bool ULunarRadio::MustSnapSelectionPresentation() const
{
	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	const bool bReduceMotion = Settings && Settings->Navigation.Accessibility.bReduceMotion;
	return IsDesignTime()
		|| bReduceMotion
		|| !bInterpolateSelectionMovement
		|| !FMath::IsFinite(SelectionInterpolationSpeed)
		|| SelectionInterpolationSpeed <= 0.0f;
}
