// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "UI/Navigation/Controls/LunarRadio.h"

#include "Kismet/KismetMathLibrary.h"
#include "Settings/LunarSettings.h"
#include "Styling/StyleDefaults.h"
#include "UI/Navigation/Controls/LunarRadioGroup.h"
#include "UI/Navigation/Styles/LunarStyleResolver.h"
#include "UI/Navigation/Types/LunarGameplayTags.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"

/**
 * @file LunarRadio.cpp
 * @brief Lunar radio behavior, group registration, and mark-style transitions
 * @ingroup LunarNavigationControls
 */

namespace LunarRadio_Private
{
	/**
	 * @brief Fills missing radio-mark fields with native fallback values
	 * @param ResolvedStyle Partially resolved radio style
	 * @return Complete style snapshot suitable for transitions and rendering
	 */
	FLunarRadioStylePatch MaterializeStyle(const FLunarRadioStylePatch& ResolvedStyle)
	{
		FLunarRadioStylePatch Result = ResolvedStyle;
		if (!Result.bOverrideMarkBrush)
		{
			Result.bOverrideMarkBrush = true;
			Result.MarkBrush = *FStyleDefaults::GetNoBrush();
		}
		if (!Result.bOverrideMarkTint)
		{
			Result.bOverrideMarkTint = true;
			Result.MarkTint = FLinearColor::White;
		}
		if (!Result.bOverrideMarkSize)
		{
			Result.bOverrideMarkSize = true;
			Result.MarkSize = FVector2D(
				FMath::Max(0.0f, Result.MarkBrush.ImageSize.X),
				FMath::Max(0.0f, Result.MarkBrush.ImageSize.Y));
		}
		return Result;
	}

	/**
	 * @brief Compares two materialized radio styles
	 * @param A First style
	 * @param B Second style
	 * @return True when all common and mark fields are equivalent
	 */
	bool AreStylesEquivalent(const FLunarRadioStylePatch& A, const FLunarRadioStylePatch& B)
	{
		return LunarStyleResolver::AreCommonStylesEquivalent(A.Common, B.Common)
			&& FSlateBrush::StaticStruct()->CompareScriptStruct(&A.MarkBrush, &B.MarkBrush, 0)
			&& A.MarkTint == B.MarkTint
			&& A.MarkSize == B.MarkSize;
	}

	/**
	 * @brief Interpolates continuous fields between two radio styles
	 * @param Source Transition source
	 * @param Target Transition target
	 * @param Alpha Normalized interpolation alpha
	 * @return Interpolated style snapshot
	 */
	FLunarRadioStylePatch InterpolateStyle(
		const FLunarRadioStylePatch& Source,
		const FLunarRadioStylePatch& Target,
		const float Alpha)
	{
		FLunarRadioStylePatch Result = Target;
		Result.Common = LunarStyleResolver::InterpolateCommonStylePatch(
			Source.Common,
			Target.Common,
			Alpha);
		Result.MarkTint = FMath::Lerp(Source.MarkTint, Target.MarkTint, Alpha);
		Result.MarkSize = FMath::Lerp(Source.MarkSize, Target.MarkSize, Alpha);
		return Result;
	}

	/**
	 * @brief Copies brushes and other non-interpolated fields into a style snapshot
	 * @param InOutStyle Interpolated style to update
	 * @param Source Style supplying discrete fields
	 */
	void ApplyDiscreteFields(FLunarRadioStylePatch& InOutStyle, const FLunarRadioStylePatch& Source)
	{
		LunarStyleResolver::ApplyCommonDiscreteFields(InOutStyle.Common, Source.Common);
		InOutStyle.MarkBrush = Source.MarkBrush;
	}
}

ULunarRadio::ULunarRadio(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCanReceiveLunarSelection = true;
	bCanInteractWithPointer = true;
	bEnableInputPrompt = true;
	SetLunarValueState(LunarGameplayTags::UI_State_Value_Unchecked.GetTag());

	FLunarPromptActionRequest AcceptPrompt;
	AcceptPrompt.ActionTag = LunarGameplayTags::UI_Action_Accept.GetTag();
	PromptActions.Add(AcceptPrompt);
}

void ULunarRadio::SetChecked(const bool bNewChecked)
{
	if (bRadioConstructed)
	{
		SynchronizeRadioGroupRegistration();
	}

	if (RadioGroup)
	{
		if (bNewChecked)
		{
			RadioGroup->SetSelectedRadio(this);
			return;
		}
		if (RadioGroup->GetSelectedRadio() == this)
		{
			RadioGroup->ClearSelectedRadio();
			return;
		}
	}

	const bool bPreviousChecked = bIsChecked;
	if (ApplyCheckedStateSilently(bNewChecked))
	{
		BroadcastCheckedStateChanged(bPreviousChecked);
	}
}

bool ULunarRadio::RequestChecked()
{
	if (bRadioConstructed)
	{
		SynchronizeRadioGroupRegistration();
	}

	if (RadioGroup)
	{
		if (RadioGroup->GetSelectedRadio() == this && bIsChecked)
		{
			if (!RadioGroup->bRequireSelection && RadioGroup->bAllowDeselect)
			{
				return RadioGroup->ClearSelectedRadio();
			}
			return true;
		}
		return RadioGroup->SetSelectedRadio(this);
	}

	const bool bPreviousChecked = bIsChecked;
	if (ApplyCheckedStateSilently(true))
	{
		BroadcastCheckedStateChanged(bPreviousChecked);
	}
	return true;
}

TSharedPtr<SWidget> ULunarRadio::RebuildLunarSpecializedPresentation()
{
	SAssignNew(RadioMarkImage, SImage)
		.Image(FStyleDefaults::GetNoBrush())
		.Visibility(EVisibility::Collapsed);
#if WITH_ACCESSIBILITY
	RadioMarkImage->SetAccessibleBehavior(EAccessibleBehavior::NotAccessible);
#endif

	SAssignNew(RadioMarkSizeBox, SBox)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Visibility(EVisibility::HitTestInvisible)
		[
			RadioMarkImage.ToSharedRef()
		];

	ApplyResolvedRadioStyle();
	return RadioMarkSizeBox;
}

void ULunarRadio::ReleaseSlateResources(const bool bReleaseChildren)
{
	RadioMarkImage.Reset();
	RadioMarkSizeBox.Reset();
	Super::ReleaseSlateResources(bReleaseChildren);
}

void ULunarRadio::SynchronizeProperties()
{
	SetLunarValueState(
		bIsChecked
			? LunarGameplayTags::UI_State_Value_Checked.GetTag()
			: LunarGameplayTags::UI_State_Value_Unchecked.GetTag());
	Super::SynchronizeProperties();
	if (bRadioConstructed)
	{
		SynchronizeRadioGroupRegistration();
	}
	ApplyResolvedRadioStyle();
}

void ULunarRadio::NativeConstruct()
{
	bRadioConstructed = true;
	Super::NativeConstruct();
	SynchronizeRadioGroupRegistration();
}

void ULunarRadio::NativeDestruct()
{
	UnregisterFromRadioGroup();
	bRadioConstructed = false;
	Super::NativeDestruct();
}

void ULunarRadio::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (bRadioConstructed
		&& (AttemptedRadioGroup.Get() != RadioGroup || AttemptedRadioId != RadioId))
	{
		SynchronizeRadioGroupRegistration();
	}
	TickRadioStyleTransition(InDeltaTime);
}

bool ULunarRadio::NativeCanHandleLunarAction(const FLunarUIActionContext& ActionContext) const
{
	if (ActionContext.ActionTag == LunarGameplayTags::UI_Action_Accept.GetTag())
	{
		return IsLunarSelected();
	}
	return Super::NativeCanHandleLunarAction(ActionContext);
}

ELunarUIActionResult ULunarRadio::NativeHandleLunarAction(const FLunarUIActionContext& ActionContext)
{
	if (ActionContext.ActionTag != LunarGameplayTags::UI_Action_Accept.GetTag())
	{
		return Super::NativeHandleLunarAction(ActionContext);
	}
	if (!NativeCanActivateLunarWidget())
	{
		return ELunarUIActionResult::Rejected;
	}
	if (ActionContext.InputEvent == IE_Released)
	{
		if (!IsLunarSelected())
		{
			return ELunarUIActionResult::Handled;
		}
		ActivateLunarWidget();
	}
	return ELunarUIActionResult::Handled;
}

void ULunarRadio::NativeOnLunarActivated()
{
	if (RequestChecked())
	{
		Super::NativeOnLunarActivated();
	}
	else
	{
		NativeOnLunarRejected();
	}
}

bool ULunarRadio::NativeCanActivateLunarWidget() const
{
	if (!Super::NativeCanActivateLunarWidget())
	{
		return false;
	}
	if (!RadioGroup)
	{
		return true;
	}
	return !RadioId.IsNone() && RadioGroup->IsRadioIdAvailable(this);
}

FText ULunarRadio::NativeGetLunarAccessibleValueText() const
{
	return bIsChecked
		? NSLOCTEXT("LunarNavigation", "AccessibleRadioChecked", "Checked")
		: NSLOCTEXT("LunarNavigation", "AccessibleRadioUnchecked", "Unchecked");
}

bool ULunarRadio::ResolveCommonStylePatch(FLunarCommonStylePatch& OutStyle, FString& OutError) const
{
	FLunarRadioStylePatch ResolvedStyle;
	const bool bResolved = LunarStyleResolver::ResolveRadioStyle(
		StyleAsset,
		GetLunarVisualState(),
		StyleOverrides,
		ResolvedStyle,
		&OutError);
	if (bResolved)
	{
		ULunarRadio* MutableThis = const_cast<ULunarRadio*>(this);
		if (!FLunarRadioStylePatch::StaticStruct()->CompareScriptStruct(
			&MutableThis->ResolvedRadioStyle,
			&ResolvedStyle,
			0))
		{
			MutableThis->PreviousRadioStyle = MutableThis->ResolvedRadioStyle;
			MutableThis->ResolvedRadioStyle = ResolvedStyle;
		}
		OutStyle = MutableThis->ResolvedRadioStyle.Common;
	}
	return bResolved;
}

void ULunarRadio::ApplyResolvedCommonStyle(const FLunarCommonStylePatch& ResolvedStyle)
{
	Super::ApplyResolvedCommonStyle(ResolvedStyle);
	ApplyRadioStyleTarget(ResolvedRadioStyle);
	ApplyResolvedRadioStyle();
}

bool ULunarRadio::ApplyCheckedStateSilently(const bool bNewChecked)
{
	if (bIsChecked == bNewChecked)
	{
		SetLunarValueState(
			bIsChecked
				? LunarGameplayTags::UI_State_Value_Checked.GetTag()
				: LunarGameplayTags::UI_State_Value_Unchecked.GetTag());
		ApplyResolvedRadioStyle();
		return false;
	}

	bIsChecked = bNewChecked;
	SetLunarValueState(
		bIsChecked
			? LunarGameplayTags::UI_State_Value_Checked.GetTag()
			: LunarGameplayTags::UI_State_Value_Unchecked.GetTag());
	ApplyResolvedRadioStyle();
	return true;
}

void ULunarRadio::BroadcastCheckedStateChanged(const bool bPreviousChecked)
{
	if (bPreviousChecked == bIsChecked)
	{
		return;
	}
	NotifyLunarAccessibleValueChanged(NativeGetLunarAccessibleValueText());
	OnCheckedChanged.Broadcast(bIsChecked);
}

void ULunarRadio::SynchronizeRadioGroupRegistration()
{
	AttemptedRadioGroup = RadioGroup;
	AttemptedRadioId = RadioId;
	ULunarRadioGroup* PreviousGroup = RegisteredRadioGroup.Get();
	if (PreviousGroup && PreviousGroup != RadioGroup)
	{
		PreviousGroup->UnregisterRadio(this);
		RegisteredRadioGroup.Reset();
	}

	if (!RadioGroup)
	{
		return;
	}
	if (RadioGroup->RegisterRadio(this))
	{
		RegisteredRadioGroup = RadioGroup;
		return;
	}
	if (RegisteredRadioGroup.Get() == RadioGroup)
	{
		RadioGroup->UnregisterRadio(this);
		RegisteredRadioGroup.Reset();
	}
}

void ULunarRadio::UnregisterFromRadioGroup()
{
	if (ULunarRadioGroup* Group = RegisteredRadioGroup.Get())
	{
		Group->UnregisterRadio(this);
	}
	RegisteredRadioGroup.Reset();
	AttemptedRadioGroup.Reset();
	AttemptedRadioId = NAME_None;
}

void ULunarRadio::ApplyResolvedRadioStyle()
{
	if (!RadioMarkImage.IsValid() || !RadioMarkSizeBox.IsValid())
	{
		return;
	}

	const FLunarRadioStylePatch& Style = bHasDisplayedRadioStyle
		? DisplayedRadioStyle
		: ResolvedRadioStyle;
	RadioMarkImage->SetImage(
		Style.bOverrideMarkBrush
			? &Style.MarkBrush
			: FStyleDefaults::GetNoBrush());
	RadioMarkImage->SetColorAndOpacity(
		Style.bOverrideMarkTint
			? FSlateColor(Style.MarkTint)
			: FSlateColor(FLinearColor::White));
	RadioMarkImage->SetVisibility(EVisibility::HitTestInvisible);

	if (Style.bOverrideMarkSize)
	{
		RadioMarkSizeBox->SetWidthOverride(FMath::Max(0.0f, Style.MarkSize.X));
		RadioMarkSizeBox->SetHeightOverride(FMath::Max(0.0f, Style.MarkSize.Y));
	}
	else
	{
		RadioMarkSizeBox->SetWidthOverride(FOptionalSize());
		RadioMarkSizeBox->SetHeightOverride(FOptionalSize());
	}
}

void ULunarRadio::ApplyRadioStyleTarget(const FLunarRadioStylePatch& NewTarget)
{
	FLunarRadioStylePatch CompleteTarget = NewTarget;
	CompleteTarget.Common = MaterializeCommonStyleSnapshot(NewTarget.Common);
	const FLunarRadioStylePatch MaterializedTarget = LunarRadio_Private::MaterializeStyle(CompleteTarget);
	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	const bool bReduceMotion = Settings && Settings->Navigation.Accessibility.bReduceMotion;
	if (bReduceMotion)
	{
		bRadioStyleTransitionActive = false;
		bRadioStyleTransitionReversing = false;
		TransitionSourceRadioStyle = MaterializedTarget;
		TransitionTargetRadioStyle = MaterializedTarget;
		LogicalTargetRadioStyle = MaterializedTarget;
		DisplayedRadioStyle = MaterializedTarget;
		bHasDisplayedRadioStyle = true;
		return;
	}
	if (bHasDisplayedRadioStyle
		&& LunarRadio_Private::AreStylesEquivalent(LogicalTargetRadioStyle, MaterializedTarget))
	{
		return;
	}

	if (bRadioStyleTransitionActive)
	{
		const bool bReturnsToSource = !bRadioStyleTransitionReversing
			&& LunarRadio_Private::AreStylesEquivalent(TransitionSourceRadioStyle, MaterializedTarget);
		const bool bReturnsToForwardTarget = bRadioStyleTransitionReversing
			&& LunarRadio_Private::AreStylesEquivalent(TransitionTargetRadioStyle, MaterializedTarget);
		if (bReturnsToSource || bReturnsToForwardTarget)
		{
			LogicalTargetRadioStyle = MaterializedTarget;
			bRadioStyleTransitionReversing = bReturnsToSource;
			FLunarRadioStylePatch Snapshot = DisplayedRadioStyle;
			LunarRadio_Private::ApplyDiscreteFields(
				Snapshot,
				bRadioStyleTransitionReversing ? TransitionSourceRadioStyle : TransitionTargetRadioStyle);
			DisplayedRadioStyle = Snapshot;
			ApplyResolvedRadioStyle();
			return;
		}
	}

	const bool bCanTransition = bHasDisplayedRadioStyle
		&& MaterializedTarget.Common.Transition.bEnabled
		&& MaterializedTarget.Common.Transition.Duration > 0.0f;
	if (!bCanTransition)
	{
		bRadioStyleTransitionActive = false;
		bRadioStyleTransitionReversing = false;
		TransitionSourceRadioStyle = MaterializedTarget;
		TransitionTargetRadioStyle = MaterializedTarget;
		LogicalTargetRadioStyle = MaterializedTarget;
		DisplayedRadioStyle = MaterializedTarget;
		bHasDisplayedRadioStyle = true;
		return;
	}

	TransitionSourceRadioStyle = DisplayedRadioStyle;
	TransitionTargetRadioStyle = MaterializedTarget;
	LogicalTargetRadioStyle = MaterializedTarget;
	RadioStyleTransitionElapsed = 0.0f;
	RadioStyleTransitionDuration = MaterializedTarget.Common.Transition.Duration;
	bRadioStyleTransitionActive = true;
	bRadioStyleTransitionReversing = false;
	DisplayedRadioStyle = LunarRadio_Private::InterpolateStyle(
		TransitionSourceRadioStyle,
		TransitionTargetRadioStyle,
		0.0f);
	bHasDisplayedRadioStyle = true;
}

void ULunarRadio::TickRadioStyleTransition(const float DeltaTime)
{
	if (!bRadioStyleTransitionActive || RadioStyleTransitionDuration <= 0.0f)
	{
		return;
	}
	if (const ULunarSettings* Settings = GetDefault<ULunarSettings>();
		Settings && Settings->Navigation.Accessibility.bReduceMotion)
	{
		bRadioStyleTransitionActive = false;
		bRadioStyleTransitionReversing = false;
		DisplayedRadioStyle = LogicalTargetRadioStyle;
		ApplyResolvedRadioStyle();
		return;
	}

	const float Step = FMath::Max(0.0f, DeltaTime);
	RadioStyleTransitionElapsed += bRadioStyleTransitionReversing ? -Step : Step;
	RadioStyleTransitionElapsed = FMath::Clamp(
		RadioStyleTransitionElapsed,
		0.0f,
		RadioStyleTransitionDuration);
	const float Alpha = RadioStyleTransitionElapsed / RadioStyleTransitionDuration;
	const float EasedAlpha = static_cast<float>(UKismetMathLibrary::Ease(
		0.0,
		1.0,
		Alpha,
		TransitionTargetRadioStyle.Common.Transition.Easing));
	DisplayedRadioStyle = LunarRadio_Private::InterpolateStyle(
		TransitionSourceRadioStyle,
		TransitionTargetRadioStyle,
		EasedAlpha);
	if (bRadioStyleTransitionReversing)
	{
		LunarRadio_Private::ApplyDiscreteFields(DisplayedRadioStyle, TransitionSourceRadioStyle);
	}
	ApplyResolvedRadioStyle();

	const bool bFinished = bRadioStyleTransitionReversing ? Alpha <= 0.0f : Alpha >= 1.0f;
	if (bFinished)
	{
		DisplayedRadioStyle = bRadioStyleTransitionReversing
			? TransitionSourceRadioStyle
			: TransitionTargetRadioStyle;
		bRadioStyleTransitionActive = false;
		bRadioStyleTransitionReversing = false;
		ApplyResolvedRadioStyle();
	}
}
