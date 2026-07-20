// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "UI/Navigation/Controls/LunarSwitch.h"

#include "Curves/CurveFloat.h"
#include "Settings/LunarSettings.h"
#include "UI/Navigation/Controls/SLunarValueControlVisuals.h"
#include "UI/Navigation/Types/LunarGameplayTags.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"

/**
 * @file LunarSwitch.cpp
 * @brief Binary Lunar switch behavior and native presentation
 * @ingroup LunarNavigationControls
 */

ULunarSwitch::ULunarSwitch(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCanReceiveLunarSelection = true;
	bCanInteractWithPointer = true;
	bEnableInputPrompt = true;

	static ConstructorHelpers::FObjectFinder<UCurveFloat> DefaultInterpolationCurve(
		TEXT("/Lunar/Curves/Float/CF_EaseOutExpo.CF_EaseOutExpo"));
	if (DefaultInterpolationCurve.Succeeded())
	{
		HandleInterpolationCurve = DefaultInterpolationCurve.Object;
	}

	FLunarPromptActionRequest AcceptPrompt;
	AcceptPrompt.ActionTag = LunarGameplayTags::UI_Action_Accept.GetTag();
	PromptActions.Add(AcceptPrompt);
}

void ULunarSwitch::SetTrackBrush(const FSlateBrush& NewBrush)
{
	TrackBrush = NewBrush;
	SynchronizeSpecializedPresentation();
}

void ULunarSwitch::SetTrackTint(const FLinearColor NewTint)
{
	TrackTint = NewTint;
	SynchronizeSpecializedPresentation();
}

void ULunarSwitch::SetTrackSize(const FVector2D NewSize)
{
	TrackSize = NewSize.ComponentMax(FVector2D::ZeroVector);
	SynchronizeSpecializedPresentation();
}

void ULunarSwitch::SetHandleBrush(const FSlateBrush& NewBrush)
{
	HandleBrush = NewBrush;
	SynchronizeSpecializedPresentation();
}

void ULunarSwitch::SetHandleTint(const FLinearColor NewTint)
{
	HandleTint = NewTint;
	SynchronizeSpecializedPresentation();
}

void ULunarSwitch::SetHandleSize(const FVector2D NewSize)
{
	HandleSize = NewSize.ComponentMax(FVector2D::ZeroVector);
	SynchronizeSpecializedPresentation();
}

void ULunarSwitch::SetHandleOffset(const FVector2D NewOffset)
{
	HandleOffset = NewOffset;
	SynchronizeSpecializedPresentation();
}

void ULunarSwitch::ConfigureTrackPresentation(
	const FSlateBrush& NewBrush,
	const FLinearColor NewTint,
	const FVector2D NewSize)
{
	TrackBrush = NewBrush;
	TrackTint = NewTint;
	TrackSize = NewSize.ComponentMax(FVector2D::ZeroVector);
	SynchronizeSpecializedPresentation();
}

void ULunarSwitch::GetTrackPresentation(
	FSlateBrush& OutBrush,
	FLinearColor& OutTint,
	FVector2D& OutSize) const
{
	OutBrush = TrackBrush;
	OutTint = TrackTint;
	OutSize = TrackSize;
}

void ULunarSwitch::ConfigureHandlePresentation(
	const FSlateBrush& NewBrush,
	const FLinearColor NewTint,
	const FVector2D NewSize,
	const FVector2D NewOffset)
{
	HandleBrush = NewBrush;
	HandleTint = NewTint;
	HandleSize = NewSize.ComponentMax(FVector2D::ZeroVector);
	HandleOffset = NewOffset;
	SynchronizeSpecializedPresentation();
}

void ULunarSwitch::GetHandlePresentation(
	FSlateBrush& OutBrush,
	FLinearColor& OutTint,
	FVector2D& OutSize,
	FVector2D& OutOffset) const
{
	OutBrush = HandleBrush;
	OutTint = HandleTint;
	OutSize = HandleSize;
	OutOffset = HandleOffset;
}

void ULunarSwitch::ConfigureSwitchPresentation(
	const FSlateBrush& NewTrackBrush,
	const FLinearColor NewTrackTint,
	const FVector2D NewTrackSize,
	const FSlateBrush& NewHandleBrush,
	const FLinearColor NewHandleTint,
	const FVector2D NewHandleSize,
	const FVector2D NewHandleOffset)
{
	TrackBrush = NewTrackBrush;
	TrackTint = NewTrackTint;
	TrackSize = NewTrackSize.ComponentMax(FVector2D::ZeroVector);
	HandleBrush = NewHandleBrush;
	HandleTint = NewHandleTint;
	HandleSize = NewHandleSize.ComponentMax(FVector2D::ZeroVector);
	HandleOffset = NewHandleOffset;
	SynchronizeSpecializedPresentation();
}

void ULunarSwitch::GetSwitchPresentation(
	FSlateBrush& OutTrackBrush,
	FLinearColor& OutTrackTint,
	FVector2D& OutTrackSize,
	FSlateBrush& OutHandleBrush,
	FLinearColor& OutHandleTint,
	FVector2D& OutHandleSize,
	FVector2D& OutHandleOffset) const
{
	OutTrackBrush = TrackBrush;
	OutTrackTint = TrackTint;
	OutTrackSize = TrackSize;
	OutHandleBrush = HandleBrush;
	OutHandleTint = HandleTint;
	OutHandleSize = HandleSize;
	OutHandleOffset = HandleOffset;
}

void ULunarSwitch::SetIsOn(
	const bool bNewIsOn,
	const ELunarChangeNotificationPolicy NotificationPolicy)
{
	if (bIsOn == bNewIsOn)
	{
		ApplyValueState();
		RefreshDisplayedHandleTarget(true);
		return;
	}

	bIsOn = bNewIsOn;
	ApplyValueState();
	RefreshDisplayedHandleTarget(true);
	RefreshLunarAccessibility();
	if (NotificationPolicy == ELunarChangeNotificationPolicy::Notify)
	{
		OnSwitchChanged.Broadcast(this, bIsOn);
		NotifyLunarAccessibleValueChanged(NativeGetLunarAccessibleValueText());
	}
}

void ULunarSwitch::Toggle(const ELunarChangeNotificationPolicy NotificationPolicy)
{
	SetIsOn(!bIsOn, NotificationPolicy);
}

float ULunarSwitch::GetDisplayedHandleAlpha() const
{
	return bDisplayedHandleAlphaInitialized ? DisplayedHandleAlpha : (bIsOn ? 1.0f : 0.0f);
}

TSharedRef<SWidget> ULunarSwitch::RebuildWidget()
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

TSharedPtr<SWidget> ULunarSwitch::RebuildLunarSpecializedPresentation()
{
	SAssignNew(SwitchVisual, SLunarSwitchVisual);
	SynchronizeSpecializedPresentation();
	return SwitchVisual;
}

void ULunarSwitch::ReleaseSlateResources(const bool bReleaseChildren)
{
	SwitchVisual.Reset();
	Super::ReleaseSlateResources(bReleaseChildren);
}

void ULunarSwitch::SynchronizeProperties()
{
	NormalizeInterpolationSettings();
	ApplyValueState();
	Super::SynchronizeProperties();
	RefreshDisplayedHandleTarget(false);
	RefreshLunarAccessibility();
	SynchronizeSpecializedPresentation();
}

void ULunarSwitch::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	UpdateDisplayedHandleInterpolation(InDeltaTime);
}

bool ULunarSwitch::NativeCanHandleLunarAction(const FLunarUIActionContext& ActionContext) const
{
	using namespace LunarGameplayTags;
	if (ActionContext.ActionTag == UI_Action_Accept.GetTag())
	{
		return IsLunarSelected();
	}

	const bool bValueAction = ActionContext.ActionTag == UI_Action_Increase.GetTag()
		|| ActionContext.ActionTag == UI_Action_Decrease.GetTag();
	if (!bValueAction || DirectionMode == ELunarSwitchDirectionMode::Disabled)
	{
		return Super::NativeCanHandleLunarAction(ActionContext);
	}
	if (!IsLunarSelected())
	{
		return false;
	}
	const bool bIncrease = ActionContext.ActionTag == UI_Action_Increase.GetTag();
	if (ActionContext.InputEvent == IE_Pressed && bIsOn == bIncrease)
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

ELunarUIActionResult ULunarSwitch::NativeHandleLunarAction(const FLunarUIActionContext& ActionContext)
{
	using namespace LunarGameplayTags;
	if (ActionContext.ActionTag == UI_Action_Accept.GetTag())
	{
		if (!IsLunarSelected())
		{
			return ELunarUIActionResult::Unhandled;
		}
		if (!NativeCanActivateLunarWidget())
		{
			return ELunarUIActionResult::Rejected;
		}
		if (ActionContext.InputEvent == IE_Released)
		{
			ActivateLunarWidget();
		}
		return ELunarUIActionResult::Handled;
	}

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
	if (!NativeCanActivateLunarWidget())
	{
		return ELunarUIActionResult::Rejected;
	}

	if (ActionContext.InputEvent == IE_Pressed || ActionContext.InputEvent == IE_Repeat)
	{
		SetIsOn(bIncrease);
	}
	// Repeats from the original value-changing hold stay consumed until release.
	return ELunarUIActionResult::Handled;
}

bool ULunarSwitch::NativeResolveDirectionalLunarControlAction(
	const ELunarNavigationDirection Direction,
	FGameplayTag& OutActionTag) const
{
	bool bIncrease = false;
	switch (DirectionMode)
	{
	case ELunarSwitchDirectionMode::Horizontal:
		if (Direction != ELunarNavigationDirection::Left && Direction != ELunarNavigationDirection::Right)
		{
			OutActionTag = FGameplayTag();
			return false;
		}
		bIncrease = Direction == ELunarNavigationDirection::Right;
		break;

	case ELunarSwitchDirectionMode::Vertical:
		if (Direction != ELunarNavigationDirection::Up && Direction != ELunarNavigationDirection::Down)
		{
			OutActionTag = FGameplayTag();
			return false;
		}
		bIncrease = Direction == ELunarNavigationDirection::Up;
		break;

	case ELunarSwitchDirectionMode::Disabled:
	default:
		OutActionTag = FGameplayTag();
		return false;
	}

	OutActionTag = bIncrease
		? LunarGameplayTags::UI_Action_Increase.GetTag()
		: LunarGameplayTags::UI_Action_Decrease.GetTag();
	return true;
}

void ULunarSwitch::NativeOnLunarActivated()
{
	Super::NativeOnLunarActivated();
	Toggle();
}

FText ULunarSwitch::NativeGetLunarAccessibleValueText() const
{
	return bIsOn
		? NSLOCTEXT("LunarSwitch", "AccessibleOn", "On")
		: NSLOCTEXT("LunarSwitch", "AccessibleOff", "Off");
}


void ULunarSwitch::ApplyValueState()
{
	SetLunarValueState(bIsOn
		? LunarGameplayTags::UI_State_Value_On.GetTag()
		: LunarGameplayTags::UI_State_Value_Off.GetTag());
}

void ULunarSwitch::UpdateDisplayedHandleInterpolation(const float DeltaTime)
{
	RefreshDisplayedHandleTarget(true);
	if (!bDisplayedHandleInterpolationActive
		|| !FMath::IsFinite(DeltaTime)
		|| DeltaTime <= 0.0f)
	{
		return;
	}

	DisplayedHandleInterpolationElapsed += DeltaTime;
	const float Progress = FMath::Clamp(
		DisplayedHandleInterpolationElapsed * HandleInterpolationSpeed,
		0.0f,
		1.0f);
	float CurveAlpha = Progress;
	if (IsValid(HandleInterpolationCurve))
	{
		const float EvaluatedAlpha = HandleInterpolationCurve->GetFloatValue(Progress);
		if (FMath::IsFinite(EvaluatedAlpha))
		{
			CurveAlpha = EvaluatedAlpha;
		}
	}

	const float NewDisplayedHandleAlpha = Progress >= 1.0f
		? DisplayedHandleInterpolationTarget
		: FMath::Lerp(
			DisplayedHandleInterpolationSource,
			DisplayedHandleInterpolationTarget,
			CurveAlpha);
	if (Progress >= 1.0f)
	{
		bDisplayedHandleInterpolationActive = false;
	}
	SetDisplayedHandleAlphaInternal(NewDisplayedHandleAlpha, true);
}

void ULunarSwitch::RefreshDisplayedHandleTarget(const bool bBroadcast)
{
	const float TargetAlpha = bIsOn ? 1.0f : 0.0f;
	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	const bool bReduceMotion = Settings && Settings->Navigation.Accessibility.bReduceMotion;
	const bool bMustSnap = !bDisplayedHandleAlphaInitialized
		|| IsDesignTime()
		|| bReduceMotion
		|| !bInterpolateHandleMovement
		|| !FMath::IsFinite(HandleInterpolationSpeed)
		|| HandleInterpolationSpeed <= 0.0f;
	if (bMustSnap)
	{
		DisplayedHandleInterpolationSource = TargetAlpha;
		DisplayedHandleInterpolationTarget = TargetAlpha;
		DisplayedHandleInterpolationElapsed = 0.0f;
		bDisplayedHandleInterpolationActive = false;
		SetDisplayedHandleAlphaInternal(TargetAlpha, bBroadcast);
		return;
	}

	if (!bDisplayedHandleInterpolationActive
		|| DisplayedHandleInterpolationTarget != TargetAlpha)
	{
		DisplayedHandleInterpolationSource = GetDisplayedHandleAlpha();
		DisplayedHandleInterpolationTarget = TargetAlpha;
		DisplayedHandleInterpolationElapsed = 0.0f;
		bDisplayedHandleInterpolationActive = DisplayedHandleInterpolationSource != TargetAlpha;
	}
}

void ULunarSwitch::SetDisplayedHandleAlphaInternal(
	const float NewDisplayedHandleAlpha,
	const bool bBroadcast)
{
	const float SafeAlpha = FMath::IsFinite(NewDisplayedHandleAlpha)
		? FMath::Clamp(NewDisplayedHandleAlpha, 0.0f, 1.0f)
		: (bIsOn ? 1.0f : 0.0f);
	if (bDisplayedHandleAlphaInitialized && DisplayedHandleAlpha == SafeAlpha)
	{
		return;
	}

	DisplayedHandleAlpha = SafeAlpha;
	bDisplayedHandleAlphaInitialized = true;
	SynchronizeSpecializedPresentation();
	if (bBroadcast)
	{
		OnDisplayedHandleAlphaChanged.Broadcast(this, DisplayedHandleAlpha);
	}
}

void ULunarSwitch::NormalizeInterpolationSettings()
{
	HandleInterpolationSpeed = FMath::IsFinite(HandleInterpolationSpeed)
		? FMath::Max(0.0f, HandleInterpolationSpeed)
		: 0.0f;
}

void ULunarSwitch::SynchronizeSpecializedPresentation()
{
	if (SwitchVisual.IsValid())
	{
		SwitchVisual->SetPresentation(TrackBrush, TrackTint, TrackSize, HandleBrush, HandleTint, HandleSize, HandleOffset);
		SwitchVisual->SetHandleAlpha(GetDisplayedHandleAlpha(), DirectionMode);
	}
}
