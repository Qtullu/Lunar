// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarNavigableWidget.cpp
 * @brief Implements common Lunar control input, style, prompt, feedback, focus, and accessibility behavior.
 * @ingroup LunarNavigationCore
 */

#include "UI/Navigation/Core/LunarNavigableWidget.h"

#include "Application/SlateApplicationBase.h"
#include "Blueprint/WidgetTree.h"
#include "Components/EditableText.h"
#include "Components/EditableTextBox.h"
#include "Components/MultiLineEditableText.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "Kismet/KismetMathLibrary.h"
#include "Settings/LunarSettings.h"
#include "Styling/StyleDefaults.h"
#include "Styling/WidgetStyle.h"
#include "UI/Navigation/Core/LunarNavigationSubsystem.h"
#include "UI/Navigation/Prompts/LunarInputPromptReceiver.h"
#include "UI/Navigation/Styles/LunarStyleResolver.h"
#include "UI/Navigation/Styles/LunarWidgetStyleAsset.h"
#include "UI/Navigation/Types/LunarGameplayTags.h"
#include "Widgets/Accessibility/SlateAccessibleMessageHandler.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SWidget.h"

DEFINE_LOG_CATEGORY_STATIC(LogLunarNavigableWidget, Log, All);

/**
 * @brief Caches the inherited Slate foreground used to materialize semantic FSlateColor values.
 * @ingroup LunarNavigationCore
 */
class SLunarContentStyleLayer : public SBorder
{
public:
	/** @param Color Semantic color to resolve. @return Linear color resolved against the incoming parent style. */
	FLinearColor ResolveAgainstInheritedStyle(const FSlateColor& Color) const
	{
		return Color.GetColor(CachedIncomingWidgetStyle);
	}

	/** @param Color Semantic color to resolve. @return Linear color resolved as a child of this layer. */
	FLinearColor ResolveForChild(const FSlateColor& Color) const
	{
		FWidgetStyle ChildStyle = CachedIncomingWidgetStyle;
		ChildStyle.SetForegroundColor(GetForegroundColor().GetColor(CachedIncomingWidgetStyle));
		return Color.GetColor(ChildStyle);
	}

	/**
	 * @brief Captures the incoming widget style before painting the border and descendants.
	 * @param Args Slate paint arguments.
	 * @param AllottedGeometry Geometry allocated to this widget.
	 * @param MyCullingRect Active culling rectangle.
	 * @param OutDrawElements Draw-element output list.
	 * @param LayerId Starting layer identifier.
	 * @param InWidgetStyle Inherited parent style.
	 * @param bParentEnabled Whether the parent hierarchy is enabled.
	 * @return Highest layer used while painting.
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
		CachedIncomingWidgetStyle = InWidgetStyle;
		return SBorder::OnPaint(
			Args,
			AllottedGeometry,
			MyCullingRect,
			OutDrawElements,
			LayerId,
			InWidgetStyle,
			bParentEnabled);
	}

private:
	/** Most recent inherited style observed during paint. */
	mutable FWidgetStyle CachedIncomingWidgetStyle;
};

/** @brief Private implementation helpers for the common navigable control base. */
namespace LunarNavigableWidget_Private
{
	/** @param Widget Editable widget candidate. @param OutText Receives current text. @return True for supported editable widget types. */
	bool TryGetEditableText(const UWidget* Widget, FText& OutText)
	{
		if (const UEditableText* EditableText = Cast<UEditableText>(Widget))
		{
			OutText = EditableText->GetText();
			return true;
		}
		if (const UEditableTextBox* EditableTextBox = Cast<UEditableTextBox>(Widget))
		{
			OutText = EditableTextBox->GetText();
			return true;
		}
		if (const UMultiLineEditableText* MultiLineEditableText = Cast<UMultiLineEditableText>(Widget))
		{
			OutText = MultiLineEditableText->GetText();
			return true;
		}
		if (const UMultiLineEditableTextBox* MultiLineEditableTextBox = Cast<UMultiLineEditableTextBox>(Widget))
		{
			OutText = MultiLineEditableTextBox->GetText();
			return true;
		}
		return false;
	}

	/** @param Widget Editable widget candidate. @param Text Text to restore. @return True for supported editable widget types. */
	bool TrySetEditableText(UWidget* Widget, const FText& Text)
	{
		if (UEditableText* EditableText = Cast<UEditableText>(Widget))
		{
			EditableText->SetText(Text);
			return true;
		}
		if (UEditableTextBox* EditableTextBox = Cast<UEditableTextBox>(Widget))
		{
			EditableTextBox->SetText(Text);
			return true;
		}
		if (UMultiLineEditableText* MultiLineEditableText = Cast<UMultiLineEditableText>(Widget))
		{
			MultiLineEditableText->SetText(Text);
			return true;
		}
		if (UMultiLineEditableTextBox* MultiLineEditableTextBox = Cast<UMultiLineEditableTextBox>(Widget))
		{
			MultiLineEditableTextBox->SetText(Text);
			return true;
		}
		return false;
	}

	/** @param Color Slate color to materialize. @param Foreground Explicit inherited foreground. @return Resolved linear color. */
	FLinearColor ResolveSlateColor(
		const FSlateColor& Color,
		const FLinearColor& Foreground = FLinearColor::White)
	{
		FWidgetStyle WidgetStyle;
		WidgetStyle.SetForegroundColor(Foreground);
		return Color.GetColor(WidgetStyle);
	}

	/** @param Override Per-widget sound override. @param GlobalSound Project default. @return Effective sound spec, or nullptr when disabled or unset. */
	const FLunarUISoundSpec* ResolveSound(
		const FLunarUISoundOverride& Override,
		const FLunarUISoundSpec& GlobalSound)
	{
		switch (Override.Mode)
		{
		case ELunarFeedbackOverrideMode::Disabled:
			return nullptr;
		case ELunarFeedbackOverrideMode::Custom:
			return Override.CustomSound.Sound ? &Override.CustomSound : nullptr;
		case ELunarFeedbackOverrideMode::UseGlobal:
		default:
			return GlobalSound.Sound ? &GlobalSound : nullptr;
		}
	}

	/** @param Override Per-widget haptic override. @param GlobalHaptic Project default. @return Effective haptic spec, or nullptr when disabled or unset. */
	const FLunarUIHapticSpec* ResolveHaptic(
		const FLunarUIHapticOverride& Override,
		const FLunarUIHapticSpec& GlobalHaptic)
	{
		switch (Override.Mode)
		{
		case ELunarFeedbackOverrideMode::Disabled:
			return nullptr;
		case ELunarFeedbackOverrideMode::Custom:
			return Override.CustomHaptic.Effect ? &Override.CustomHaptic : nullptr;
		case ELunarFeedbackOverrideMode::UseGlobal:
		default:
			return GlobalHaptic.Effect ? &GlobalHaptic : nullptr;
		}
	}

	/** @param A First visual state. @param B Second visual state. @return True when every state component is equal. */
	bool VisualStatesEqual(const FLunarUIVisualState& A, const FLunarUIVisualState& B)
	{
		return A.ValueStateTag == B.ValueStateTag
			&& A.InteractionState == B.InteractionState
			&& A.InputDevice == B.InputDevice
			&& A.bReduceMotion == B.bReduceMotion;
	}

#if WITH_ACCESSIBILITY
	/** @param Widget Widget whose owning Slate user is queried. @return Accessible user index, falling back to zero. */
	FAccessibleUserIndex GetAccessibleUserIndex(const ULunarNavigableWidget* Widget)
	{
		if (const ULocalPlayer* LocalPlayer = Widget ? Widget->GetOwningLocalPlayer() : nullptr)
		{
			if (const TSharedPtr<const FSlateUser> SlateUser = LocalPlayer->GetSlateUser())
			{
				return SlateUser->GetUserIndex();
			}
		}
		return 0;
	}
#endif
}

ULunarNavigableWidget::ULunarNavigableWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
	if (const ULunarSettings* Settings = GetDefault<ULunarSettings>())
	{
		// This becomes the native/class default; serialized Blueprint and instance values still override it.
		PromptVisibilityPolicy = Settings->Navigation.Prompts.DefaultPromptVisibilityPolicy;
	}
	CurrentValueStateTag = LunarGameplayTags::UI_State_Value_Normal.GetTag();
	CurrentVisualState.ValueStateTag = CurrentValueStateTag;
}

TSharedRef<SWidget> ULunarNavigableWidget::RebuildWidget()
{
	// Serialized Blueprint defaults must not disable the native focus bridge that Lunar owns internally.
	SetIsFocusable(true);
	DetachInputPromptReceiver();
	const TSharedRef<SWidget> UserContent = Super::RebuildWidget();

	SAssignNew(LunarBackgroundImage, SImage)
		.Image(FStyleDefaults::GetNoBrush())
		.DesiredSizeOverride(TOptional<FVector2D>(FVector2D::ZeroVector))
		.Visibility(EVisibility::HitTestInvisible);

	SAssignNew(LunarForegroundImage, SImage)
		.Image(FStyleDefaults::GetNoBrush())
		.DesiredSizeOverride(TOptional<FVector2D>(FVector2D::ZeroVector))
		.Visibility(EVisibility::HitTestInvisible);

	SAssignNew(LunarInputPromptHost, SBox)
		.Visibility(EVisibility::Collapsed);
	AttachInputPromptReceiver();
	UpdateInputPromptHostVisibility();

	SAssignNew(LunarContentStyleLayer, SLunarContentStyleLayer)
		.BorderImage(FStyleDefaults::GetNoBrush())
		.Padding(0.0f)
		[
			UserContent
		];

	TSharedRef<SOverlay> ContentPresentation = SNew(SOverlay);
	if (const TSharedPtr<SWidget> SpecializedPresentation = RebuildLunarSpecializedPresentation())
	{
		ContentPresentation->AddSlot()
		[
			SpecializedPresentation.ToSharedRef()
		];
	}
	ContentPresentation->AddSlot()
	[
		LunarContentStyleLayer.ToSharedRef()
	];

	SAssignNew(LunarBorderLayer, SBorder)
		.BorderImage(FStyleDefaults::GetNoBrush())
		.Padding(0.0f)
		[
			ContentPresentation
		];

	TSharedRef<SOverlay> PresentationOverlay = SNew(SOverlay)
		+ SOverlay::Slot()
		[
			LunarBackgroundImage.ToSharedRef()
		]
		+ SOverlay::Slot()
		[
			LunarBorderLayer.ToSharedRef()
		]
		+ SOverlay::Slot()
		[
			LunarForegroundImage.ToSharedRef()
		]
		+ SOverlay::Slot()
		[
			LunarInputPromptHost.ToSharedRef()
		];

	SAssignNew(LunarOuterPaddingLayer, SBorder)
		.BorderImage(FStyleDefaults::GetNoBrush())
		.Padding(0.0f)
		[
			PresentationOverlay
		];

	SAssignNew(LunarStyleSizeBox, SBox)
		[
			LunarOuterPaddingLayer.ToSharedRef()
		];

	return LunarStyleSizeBox.ToSharedRef();
}

TSharedPtr<SWidget> ULunarNavigableWidget::RebuildLunarSpecializedPresentation()
{
	return nullptr;
}

void ULunarNavigableWidget::ReleaseSlateResources(const bool bReleaseChildren)
{
	DetachInputPromptReceiver();
	LunarStyleSizeBox.Reset();
	LunarOuterPaddingLayer.Reset();
	LunarBorderLayer.Reset();
	LunarContentStyleLayer.Reset();
	LunarBackgroundImage.Reset();
	LunarForegroundImage.Reset();
	LunarInputPromptHost.Reset();
	bPromptDirty = true;
	Super::ReleaseSlateResources(bReleaseChildren);
}

void ULunarNavigableWidget::SynchronizeProperties()
{
	SetIsFocusable(true);
	Super::SynchronizeProperties();
	SynchronizeLunarAccessibility();
	RefreshVisualState();
	InvalidateInputPrompt();
}

void ULunarNavigableWidget::NativeConstruct()
{
	Super::NativeConstruct();
	bPromptOwnerConstructed = true;

	bLastKnownEffectivelyInteractive = IsEffectivelyInteractive();
	if (ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem())
	{
		LastObservedInputDevice = NavigationSubsystem->GetLastInputDevice();
		bLastObservedPointerPresentation = NavigationSubsystem->IsPointerPresentationActive();
		InputPresentationChangedHandle = NavigationSubsystem->OnInputPresentationChangedNative().AddUObject(
			this,
			&ULunarNavigableWidget::HandleInputPresentationChanged);
		NavigationSubsystem->RegisterNavigableWidget(this);
	}

	SynchronizeLunarAccessibility();
	RefreshVisualState();
	AttachInputPromptReceiver();
	InvalidateInputPrompt();
}

void ULunarNavigableWidget::NativeDestruct()
{
	bPromptOwnerConstructed = false;
	CancelPointerPress();
	if (bNavigationPressed)
	{
		bNavigationPressed = false;
		NativeOnLunarReleased();
	}

	if (ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem())
	{
		if (InputPresentationChangedHandle.IsValid())
		{
			NavigationSubsystem->OnInputPresentationChangedNative().Remove(InputPresentationChangedHandle);
			InputPresentationChangedHandle.Reset();
		}
		NavigationSubsystem->UnregisterNavigableWidget(this);
	}

	ClearInputPromptSnapshot(true);
	DetachInputPromptReceiver();
	UpdateInputPromptHostVisibility();
	bPromptDirty = true;
	DelegatedFocusTextSnapshotWidget.Reset();
	DelegatedFocusTextSnapshot = FText::GetEmpty();
	bHasDelegatedFocusTextSnapshot = false;

	Super::NativeDestruct();
}

void ULunarNavigableWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const bool bEffectivelyInteractive = IsEffectivelyInteractive();
	if (bLastKnownEffectivelyInteractive != bEffectivelyInteractive)
	{
		bLastKnownEffectivelyInteractive = bEffectivelyInteractive;
		if (ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem())
		{
			NavigationSubsystem->ScheduleNavigationScopeValidation(
				NavigationSubsystem->ResolveOwningScopeForWidget(this));
		}
		InvalidateInputPrompt();
		SynchronizeLunarAccessibility();
		RefreshVisualState();
	}

	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	const bool bReduceMotion = Settings && Settings->Navigation.Accessibility.bReduceMotion;
	if (CurrentVisualState.bReduceMotion != bReduceMotion)
	{
		RefreshVisualState();
	}

	if (bPromptDirty && !bPromptUpdateInProgress)
	{
		ProcessInputPromptUpdate();
	}

	if (!bStyleTransitionActive)
	{
		return;
	}
	if (bReduceMotion)
	{
		bStyleTransitionActive = false;
		bStyleTransitionReversing = false;
		bUseCapturedTextTransitionSource = false;
		ApplyDisplayedStyle(LogicalTargetCommonStyle);
		return;
	}

	const float TransitionDelta = FMath::Max(0.0f, InDeltaTime);
	StyleTransitionElapsed += bStyleTransitionReversing ? -TransitionDelta : TransitionDelta;
	StyleTransitionElapsed = FMath::Clamp(StyleTransitionElapsed, 0.0f, StyleTransitionDuration);
	const float Alpha = StyleTransitionDuration > 0.0f
		? FMath::Clamp(StyleTransitionElapsed / StyleTransitionDuration, 0.0f, 1.0f)
		: 1.0f;
	const float EasedAlpha = static_cast<float>(UKismetMathLibrary::Ease(
		0.0,
		1.0,
		Alpha,
		TransitionTargetCommonStyle.Transition.Easing));
	FLunarCommonStylePatch TransitionSnapshot = LunarStyleResolver::InterpolateCommonStylePatch(
		TransitionSourceCommonStyle,
		TransitionTargetCommonStyle,
		EasedAlpha);
	if (bStyleTransitionReversing)
	{
		LunarStyleResolver::ApplyCommonDiscreteFields(TransitionSnapshot, TransitionSourceCommonStyle);
	}
	ApplyDisplayedStyle(TransitionSnapshot);

	const bool bTransitionFinished = bStyleTransitionReversing ? Alpha <= 0.0f : Alpha >= 1.0f;
	if (bTransitionFinished)
	{
		ApplyDisplayedStyle(bStyleTransitionReversing ? TransitionSourceCommonStyle : TransitionTargetCommonStyle);
		bStyleTransitionActive = false;
		bStyleTransitionReversing = false;
		bUseCapturedTextTransitionSource = false;
		ApplyResolvedCommonStyle(DisplayedCommonStyle);
	}
}

bool ULunarNavigableWidget::RequestLunarSelection()
{
	if (ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem())
	{
		return NavigationSubsystem->SetSelectedWidget(this);
	}
	return false;
}

bool ULunarNavigableWidget::IsLunarSelected() const
{
	return bLunarSelected;
}

bool ULunarNavigableWidget::CanReceiveLunarSelection() const
{
	if (!bCanReceiveLunarSelection
		|| !IsVisible()
		|| (!GetIsEnabled() && !bCanReceiveSelectionWhenDisabled)
		|| HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed))
	{
		return false;
	}

	const UWidget* Current = this;
	TSet<TObjectKey<UWidget>> Visited;
	while (Current && !Visited.Contains(TObjectKey<UWidget>(const_cast<UWidget*>(Current))))
	{
		Visited.Add(TObjectKey<UWidget>(const_cast<UWidget*>(Current)));
		if (!Current->IsVisible() || (Current != this && !Current->GetIsEnabled()))
		{
			return false;
		}

		if (const UWidget* Parent = Current->GetParent())
		{
			Current = Parent;
			continue;
		}

		const UWidget* OuterWidget = nullptr;
		for (const UObject* Outer = Current->GetOuter(); Outer; Outer = Outer->GetOuter())
		{
			if (const UWidget* Candidate = Cast<UWidget>(Outer))
			{
				OuterWidget = Candidate;
				break;
			}
		}
		Current = OuterWidget;
	}
	return true;
}

void ULunarNavigableWidget::ActivateLunarWidget()
{
	if (!NativeCanActivateLunarWidget())
	{
		NativeOnLunarRejected();
		return;
	}
	NativeOnLunarActivated();
}

bool ULunarNavigableWidget::CanHandleLunarAction(const FLunarUIActionContext& ActionContext) const
{
	return NativeCanHandleLunarAction(ActionContext);
}

ELunarUIActionResult ULunarNavigableWidget::HandleLunarAction(const FLunarUIActionContext& ActionContext)
{
	if (ActionContext.InputEvent == IE_Released && bNavigationPressed)
	{
		bNavigationPressed = false;
		NativeOnLunarReleased();
	}

	const ELunarUIActionResult Result = NativeHandleLunarAction(ActionContext);
	if (Result == ELunarUIActionResult::Handled && ActionContext.InputEvent == IE_Pressed && !bNavigationPressed)
	{
		bNavigationPressed = true;
		NativeOnLunarPressed();
	}
	RefreshVisualState();
	return Result;
}

void ULunarNavigableWidget::SetStyleAsset(ULunarWidgetStyleAsset* NewStyleAsset)
{
	if (StyleAsset != NewStyleAsset)
	{
		StyleAsset = NewStyleAsset;
		RefreshVisualState();
		if (ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem())
		{
			NavigationSubsystem->NotifyNavigableWidgetConfigurationChanged(this, false);
		}
	}
}

void ULunarNavigableWidget::RefreshVisualState()
{
	if (!CurrentValueStateTag.IsValid())
	{
		CurrentValueStateTag = LunarGameplayTags::UI_State_Value_Normal.GetTag();
	}

	FLunarUIVisualState NewVisualState;
	NewVisualState.ValueStateTag = IsEffectivelyInteractive()
		? CurrentValueStateTag
		: LunarGameplayTags::UI_State_Value_Disabled.GetTag();

	const ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem();
	const bool bPointerPresentation = NavigationSubsystem && NavigationSubsystem->IsPointerPresentationActive();
	NewVisualState.InputDevice = NavigationSubsystem
		? NavigationSubsystem->GetLastInputDevice()
		: ELunarInputDeviceType::Unknown;
	if (const ULunarSettings* Settings = GetDefault<ULunarSettings>())
	{
		NewVisualState.bReduceMotion = Settings->Navigation.Accessibility.bReduceMotion;
	}

	if (bPointerPresentation)
	{
		NewVisualState.InteractionState = bPointerPressed
			? ELunarUIInteractionState::PointerPressed
			: (bPointerHovered ? ELunarUIInteractionState::PointerHovered : ELunarUIInteractionState::PointerNormal);
	}
	else
	{
		NewVisualState.InteractionState = bNavigationPressed && bLunarSelected
			? ELunarUIInteractionState::NavigationPressed
			: (bLunarSelected ? ELunarUIInteractionState::NavigationSelected : ELunarUIInteractionState::NavigationNormal);
	}

	const bool bStateChanged = !LunarNavigableWidget_Private::VisualStatesEqual(CurrentVisualState, NewVisualState);
	CurrentVisualState = NewVisualState;

	FLunarCommonStylePatch ResolvedStyle;
	FString StyleError;
	if (ResolveCommonStylePatch(ResolvedStyle, StyleError))
	{
		ApplyStyleTarget(ResolvedStyle);
	}
	else if (!StyleError.IsEmpty())
	{
		UE_LOG(LogLunarNavigableWidget, Error, TEXT("%s: %s"), *GetPathName(), *StyleError);
	}

	if (bStateChanged)
	{
		NativeOnLunarVisualStateChanged(CurrentVisualState);
	}
}

void ULunarNavigableWidget::SetPromptActions(const TArray<FLunarPromptActionRequest>& NewPromptActions)
{
	PromptActions.Reset();
	for (const FLunarPromptActionRequest& Request : NewPromptActions)
	{
		if (Request.ActionTag.IsValid()
			&& !PromptActions.ContainsByPredicate([&Request](const FLunarPromptActionRequest& Existing)
			{
				return Existing.ActionTag == Request.ActionTag;
			}))
		{
			PromptActions.Add(Request);
		}
	}
	InvalidateInputPrompt();
	if (ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem())
	{
		NavigationSubsystem->NotifyNavigableWidgetConfigurationChanged(this, true);
	}
}

bool ULunarNavigableWidget::AddPromptAction(const FLunarPromptActionRequest& PromptAction)
{
	if (!PromptAction.ActionTag.IsValid()
		|| PromptActions.ContainsByPredicate([&PromptAction](const FLunarPromptActionRequest& Existing)
		{
			return Existing.ActionTag == PromptAction.ActionTag;
		}))
	{
		return false;
	}
	PromptActions.Add(PromptAction);
	InvalidateInputPrompt();
	if (ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem())
	{
		NavigationSubsystem->NotifyNavigableWidgetConfigurationChanged(this, true);
	}
	return true;
}

bool ULunarNavigableWidget::RemovePromptAction(const FGameplayTag ActionTag)
{
	const int32 Removed = PromptActions.RemoveAll([&ActionTag](const FLunarPromptActionRequest& Existing)
	{
		return Existing.ActionTag == ActionTag;
	});
	if (Removed > 0)
	{
		InvalidateInputPrompt();
		if (ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem())
		{
			NavigationSubsystem->NotifyNavigableWidgetConfigurationChanged(this, true);
		}
		return true;
	}
	return false;
}

UClass* ULunarNavigableWidget::ResolveInputPromptReceiverClass() const
{
	if (PromptWidgetClass)
	{
		return PromptWidgetClass.Get();
	}

	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	if (!Settings || Settings->Navigation.Prompts.DefaultPromptWidgetClass.IsNull())
	{
		return nullptr;
	}

	return Settings->Navigation.Prompts.DefaultPromptWidgetClass.LoadSynchronous();
}

void ULunarNavigableWidget::ProcessInputPromptUpdate()
{
	// Clear before invoking project code so a reentrant invalidation remains pending for the next tick.
	bPromptDirty = false;

	if (!bEnableInputPrompt)
	{
		ClearInputPromptSnapshot();
		UpdateInputPromptHostVisibility();
		return;
	}

	UClass* ResolvedReceiverClass = ResolveInputPromptReceiverClass();
	if (InputPromptReceiver && InputPromptReceiver->GetClass() != ResolvedReceiverClass)
	{
		// A class handoff always clears the old receiver before it is detached and replaced.
		ClearInputPromptSnapshot(true);
		DetachInputPromptReceiver();
		InputPromptReceiver = nullptr;
	}

	if (!ResolvedReceiverClass)
	{
		UpdateInputPromptHostVisibility();
		return;
	}

	if (!ResolvedReceiverClass->ImplementsInterface(ULunarInputPromptReceiver::StaticClass()))
	{
		if (InputPromptReceiver)
		{
			ClearInputPromptSnapshot(true);
			DetachInputPromptReceiver();
			InputPromptReceiver = nullptr;
		}
		if (ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem())
		{
			NavigationSubsystem->ReportInputPromptReceiverClassError(this, ResolvedReceiverClass);
		}
		UpdateInputPromptHostVisibility();
		return;
	}

	if (!InputPromptReceiver)
	{
		InputPromptReceiver = CreateWidget<UUserWidget>(this, ResolvedReceiverClass);
		if (!InputPromptReceiver)
		{
			UE_LOG(
				LogLunarNavigableWidget,
				Error,
				TEXT("%s: failed to create input prompt receiver of class %s"),
				*GetPathName(),
				*GetNameSafe(ResolvedReceiverClass));
			UpdateInputPromptHostVisibility();
			return;
		}
	}

	AttachInputPromptReceiver();

	if (ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem())
	{
		TArray<FLunarResolvedPromptAction> ResolvedActions;
		NavigationSubsystem->ResolveInputPromptActions(
			this,
			ResolvedReceiverClass,
			PromptActions,
			PromptIconSetOverride,
			ResolvedActions);
		ApplyInputPromptSnapshot(ResolvedActions);
	}
	else
	{
		ClearInputPromptSnapshot();
	}

	UpdateInputPromptHostVisibility();
}

void ULunarNavigableWidget::AttachInputPromptReceiver()
{
	if (LunarInputPromptHost.IsValid() && IsValid(InputPromptReceiver))
	{
		LunarInputPromptHost->SetContent(InputPromptReceiver->TakeWidget());
	}
}

void ULunarNavigableWidget::DetachInputPromptReceiver()
{
	if (LunarInputPromptHost.IsValid())
	{
		LunarInputPromptHost->SetContent(SNullWidget::NullWidget);
	}
}

void ULunarNavigableWidget::ApplyInputPromptSnapshot(
	const TArray<FLunarResolvedPromptAction>& Actions)
{
	if (!IsValid(InputPromptReceiver))
	{
		return;
	}
	if (bPromptUpdateInProgress)
	{
		bPromptDirty = true;
		return;
	}

	{
		TGuardValue<bool> UpdateGuard(bPromptUpdateInProgress, true);
		ILunarInputPromptReceiver::Execute_ApplyResolvedPromptActions(InputPromptReceiver, Actions);
	}
	bPromptReceiverHasCurrentSnapshot = true;
	if (bPromptClearPending)
	{
		bPromptClearPending = false;
		ClearInputPromptSnapshot(true);
	}
}

void ULunarNavigableWidget::ClearInputPromptSnapshot(const bool bForceDelivery)
{
	if (!IsValid(InputPromptReceiver))
	{
		bPromptReceiverHasCurrentSnapshot = false;
		return;
	}
	if (!bForceDelivery && !bPromptReceiverHasCurrentSnapshot)
	{
		return;
	}
	if (bPromptUpdateInProgress)
	{
		bPromptClearPending = true;
		bPromptDirty = true;
		return;
	}

	bPromptReceiverHasCurrentSnapshot = false;
	bPromptClearPending = false;
	TGuardValue<bool> UpdateGuard(bPromptUpdateInProgress, true);
	const TArray<FLunarResolvedPromptAction> EmptySnapshot;
	ILunarInputPromptReceiver::Execute_ApplyResolvedPromptActions(InputPromptReceiver, EmptySnapshot);
}

void ULunarNavigableWidget::UpdateInputPromptHostVisibility()
{
	if (!LunarInputPromptHost.IsValid())
	{
		return;
	}

	bool bShowHost = bPromptOwnerConstructed
		&& bEnableInputPrompt
		&& IsValid(InputPromptReceiver);
	if (bShowHost)
	{
		switch (PromptVisibilityPolicy)
		{
		case ELunarPromptVisibilityPolicy::WhenSelected:
			bShowHost = bLunarSelected;
			break;
		case ELunarPromptVisibilityPolicy::Always:
		case ELunarPromptVisibilityPolicy::Manual:
			break;
		default:
			bShowHost = false;
			break;
		}
	}

	// Manual policy leaves the receiver's own visibility untouched while the host stays presentation-only.
	LunarInputPromptHost->SetVisibility(
		bShowHost ? EVisibility::HitTestInvisible : EVisibility::Collapsed);
}

bool ULunarNavigableWidget::BeginNativeFocusDelegation(UWidget* NativeFocusWidget)
{
	if (ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem())
	{
		return NavigationSubsystem->BeginNativeFocusDelegation(this, NativeFocusWidget);
	}
	return false;
}

bool ULunarNavigableWidget::CommitNativeFocusDelegation()
{
	if (ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem())
	{
		return NavigationSubsystem->CommitNativeFocusDelegation(this);
	}
	return false;
}

bool ULunarNavigableWidget::CancelNativeFocusDelegation()
{
	if (ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem())
	{
		return NavigationSubsystem->CancelNativeFocusDelegation(this);
	}
	return false;
}

bool ULunarNavigableWidget::IsNativeFocusDelegationActive() const
{
	if (const ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem())
	{
		return NavigationSubsystem->IsNativeFocusDelegationActive(this);
	}
	return false;
}

const FLunarNavigationLink& ULunarNavigableWidget::GetNavigationLink(const ELunarNavigationDirection Direction) const
{
	switch (Direction)
	{
	case ELunarNavigationDirection::Up: return UpLink;
	case ELunarNavigationDirection::Down: return DownLink;
	case ELunarNavigationDirection::Left: return LeftLink;
	case ELunarNavigationDirection::Right:
	default: return RightLink;
	}
}

bool ULunarNavigableWidget::ResolveDirectionalLunarControlAction(
	const ELunarNavigationDirection Direction,
	FGameplayTag& OutActionTag) const
{
	OutActionTag = FGameplayTag();
	return NativeResolveDirectionalLunarControlAction(Direction, OutActionTag)
		&& OutActionTag.IsValid();
}

bool ULunarNavigableWidget::GetLunarRepeatSettingsOverride(
	FLunarNavigationRepeatSettings& OutRepeatSettings) const
{
	return NativeGetLunarRepeatSettingsOverride(OutRepeatSettings);
}

void ULunarNavigableWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	if (bCanInteractWithPointer && !InMouseEvent.IsTouchEvent())
	{
		bPointerHovered = true;
		PlayPointerHoveredFeedback();
		RefreshVisualState();
	}
}

void ULunarNavigableWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	bPointerHovered = false;
	RefreshVisualState();
	Super::NativeOnMouseLeave(InMouseEvent);
}

FReply ULunarNavigableWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!bCanInteractWithPointer || InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	if (bCanReceiveLunarSelection)
	{
		RequestLunarSelection();
	}
	bPointerPressed = true;
	bPointerActivationEligible = true;
	PointerPressScreenPosition = InMouseEvent.GetScreenSpacePosition();
	NativeOnLunarPressed();
	return FReply::Handled().CaptureMouse(TakeWidget());
}

FReply ULunarNavigableWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!bPointerPressed || InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
	}

	const bool bActivate = bPointerActivationEligible
		&& InGeometry.IsUnderLocation(InMouseEvent.GetScreenSpacePosition());
	bPointerPressed = false;
	bPointerActivationEligible = false;
	NativeOnLunarReleased();
	if (bActivate)
	{
		ActivateLunarWidget();
	}
	return FReply::Handled().ReleaseMouseCapture();
}

void ULunarNavigableWidget::NativeOnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent)
{
	if (bPointerPressed)
	{
		bPointerPressed = false;
		bPointerActivationEligible = false;
		NativeOnLunarReleased();
	}
	Super::NativeOnMouseCaptureLost(CaptureLostEvent);
}

FReply ULunarNavigableWidget::NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	if (!bCanInteractWithPointer || !InGestureEvent.IsTouchEvent() || InGestureEvent.GetPointerIndex() != 0)
	{
		return Super::NativeOnTouchStarted(InGeometry, InGestureEvent);
	}
	bPointerPressed = true;
	bPointerActivationEligible = true;
	PointerPressScreenPosition = InGestureEvent.GetScreenSpacePosition();
	NativeOnLunarPressed();
	return FReply::Handled().CaptureMouse(TakeWidget());
}

FReply ULunarNavigableWidget::NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	if (bPointerPressed
		&& FVector2D::Distance(PointerPressScreenPosition, InGestureEvent.GetScreenSpacePosition())
			> FSlateApplication::Get().GetDragTriggerDistance())
	{
		bPointerActivationEligible = false;
	}
	return bPointerPressed ? FReply::Handled() : Super::NativeOnTouchMoved(InGeometry, InGestureEvent);
}

FReply ULunarNavigableWidget::NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	if (!bPointerPressed || InGestureEvent.GetPointerIndex() != 0)
	{
		return Super::NativeOnTouchEnded(InGeometry, InGestureEvent);
	}
	const bool bActivate = bPointerActivationEligible
		&& InGeometry.IsUnderLocation(InGestureEvent.GetScreenSpacePosition());
	bPointerPressed = false;
	bPointerActivationEligible = false;
	NativeOnLunarReleased();
	if (bActivate)
	{
		if (bCanReceiveLunarSelection)
		{
			RequestLunarSelection();
		}
		ActivateLunarWidget();
	}
	return FReply::Handled().ReleaseMouseCapture();
}

bool ULunarNavigableWidget::NativeCanHandleLunarAction(const FLunarUIActionContext& ActionContext) const
{
	return BP_CanHandleLunarAction(ActionContext);
}

ELunarUIActionResult ULunarNavigableWidget::NativeHandleLunarAction(const FLunarUIActionContext& ActionContext)
{
	return BP_HandleLunarAction(ActionContext);
}

bool ULunarNavigableWidget::NativeResolveDirectionalLunarControlAction(
	const ELunarNavigationDirection Direction,
	FGameplayTag& OutActionTag) const
{
	return false;
}

bool ULunarNavigableWidget::NativeGetLunarRepeatSettingsOverride(
	FLunarNavigationRepeatSettings& OutRepeatSettings) const
{
	return false;
}

bool ULunarNavigableWidget::NativeCanActivateLunarWidget() const
{
	return IsEffectivelyInteractive();
}

/** @cond DOXYGEN_INTERNAL */
bool ULunarNavigableWidget::BP_CanHandleLunarAction_Implementation(const FLunarUIActionContext& ActionContext) const
{
	return false;
}

ELunarUIActionResult ULunarNavigableWidget::BP_HandleLunarAction_Implementation(const FLunarUIActionContext& ActionContext)
{
	return ELunarUIActionResult::Unhandled;
}
/** @endcond */

void ULunarNavigableWidget::NativeOnLunarSelected()
{
	RefreshVisualState();
	InvalidateInputPrompt();
	BP_OnLunarSelected();
	OnLunarSelected.Broadcast();
	PlaySelectedFeedback();
}

void ULunarNavigableWidget::NativeOnLunarUnselected()
{
	if (bNavigationPressed)
	{
		bNavigationPressed = false;
		NativeOnLunarReleased();
	}
	CancelPointerPress();
	RefreshVisualState();
	InvalidateInputPrompt();
	BP_OnLunarUnselected();
	OnLunarUnselected.Broadcast();
}

void ULunarNavigableWidget::NativeOnLunarPressed()
{
	RefreshVisualState();
	BP_OnLunarPressed();
	OnLunarPressed.Broadcast();
	PlayPressedFeedback();
}

void ULunarNavigableWidget::NativeOnLunarReleased()
{
	RefreshVisualState();
	BP_OnLunarReleased();
	OnLunarReleased.Broadcast();
}

void ULunarNavigableWidget::NativeOnLunarActivated()
{
	BP_OnLunarActivated();
	OnLunarActivated.Broadcast();
	PlayActivatedFeedback();
#if WITH_ACCESSIBILITY
	if (FSlateApplication::IsInitialized())
	{
		if (const TSharedPtr<SWidget> SlateWidget = GetCachedWidget())
		{
			FSlateApplicationBase::Get().GetAccessibleMessageHandler()->OnWidgetEventRaised(
				FSlateAccessibleMessageHandler::FSlateWidgetAccessibleEventArgs(
					SlateWidget.ToSharedRef(),
					EAccessibleEvent::Activate,
					FVariant(),
					FVariant(),
					LunarNavigableWidget_Private::GetAccessibleUserIndex(this)));
		}
	}
#endif
}

void ULunarNavigableWidget::NativeOnLunarRejected()
{
	BP_OnLunarRejected();
	OnLunarRejected.Broadcast();
	PlayRejectedFeedback();
#if WITH_ACCESSIBILITY
	if (!DisabledReason.IsEmpty() && !IsEffectivelyInteractive() && FSlateApplication::IsInitialized())
	{
		if (const TSharedPtr<SWidget> SlateWidget = GetCachedWidget())
		{
			FSlateApplicationBase::Get().GetAccessibleMessageHandler()->OnWidgetEventRaised(
				FSlateAccessibleMessageHandler::FSlateWidgetAccessibleEventArgs(
					SlateWidget.ToSharedRef(),
					EAccessibleEvent::Notification,
					FVariant(),
					FVariant(DisabledReason.ToString()),
					LunarNavigableWidget_Private::GetAccessibleUserIndex(this)));
		}
	}
#endif
}

void ULunarNavigableWidget::NativeOnLunarInputDeviceChanged(const ELunarInputDeviceType NewInputDevice)
{
	RefreshVisualState();
	InvalidateInputPrompt();
	BP_OnLunarInputDeviceChanged(NewInputDevice);
	OnLunarInputDeviceChanged.Broadcast(NewInputDevice);
}

void ULunarNavigableWidget::NativeOnLunarVisualStateChanged(const FLunarUIVisualState& VisualState)
{
	BP_OnLunarVisualStateChanged(VisualState);
	OnLunarVisualStateChanged.Broadcast(VisualState);
}

void ULunarNavigableWidget::NativeOnInputPromptInvalidated()
{
}

FText ULunarNavigableWidget::NativeGetLunarAccessibleValueText() const
{
	return FText::GetEmpty();
}

bool ULunarNavigableWidget::ResolveCommonStylePatch(FLunarCommonStylePatch& OutStyle, FString& OutError) const
{
	OutStyle = FLunarCommonStylePatch();
	if (StyleAsset)
	{
		OutError = FString::Printf(TEXT("Style asset %s is not compatible with abstract navigable widget %s"), *StyleAsset->GetName(), *GetClass()->GetName());
		return false;
	}
	LunarStyleResolver::MergeCommonStylePatch(OutStyle, StyleOverrides);
	return true;
}

void ULunarNavigableWidget::EnsureTextBlockStyleBaselines()
{
	TextBlockStyleBaselines.RemoveAll([](const FLunarTextBlockStyleBaseline& Baseline)
	{
		return !Baseline.TextBlock.IsValid();
	});
	if (!WidgetTree)
	{
		return;
	}

	WidgetTree->ForEachWidget([this](UWidget* Widget)
	{
		UTextBlock* TextBlock = Cast<UTextBlock>(Widget);
		if (!TextBlock || TextBlockStyleBaselines.ContainsByPredicate(
			[TextBlock](const FLunarTextBlockStyleBaseline& Existing)
			{
				return Existing.TextBlock.Get() == TextBlock;
			}))
		{
			return;
		}

		FLunarTextBlockStyleBaseline& Baseline = TextBlockStyleBaselines.AddDefaulted_GetRef();
		Baseline.TextBlock = TextBlock;
		Baseline.ColorAndOpacity = TextBlock->GetColorAndOpacity();
		Baseline.Font = TextBlock->GetFont();
		Baseline.ShadowColorAndOpacity = TextBlock->GetShadowColorAndOpacity();
		Baseline.ShadowOffset = TextBlock->GetShadowOffset();
	});
}

void ULunarNavigableWidget::EnsureStyleBaseline()
{
	EnsureTextBlockStyleBaselines();
	if (bHasStyleBaseline)
	{
		return;
	}

#define LUNAR_ENABLE_COMMON_BASELINE(FieldName) StyleBaselineCommonStyle.bOverride##FieldName = true
	LUNAR_ENABLE_COMMON_BASELINE(BackgroundBrush);
	LUNAR_ENABLE_COMMON_BASELINE(BorderBrush);
	LUNAR_ENABLE_COMMON_BASELINE(ForegroundBrush);
	LUNAR_ENABLE_COMMON_BASELINE(BackgroundTint);
	LUNAR_ENABLE_COMMON_BASELINE(ContentColor);
	LUNAR_ENABLE_COMMON_BASELINE(TextColor);
	LUNAR_ENABLE_COMMON_BASELINE(Font);
	LUNAR_ENABLE_COMMON_BASELINE(TextShadowColor);
	LUNAR_ENABLE_COMMON_BASELINE(TextShadowOffset);
	LUNAR_ENABLE_COMMON_BASELINE(Padding);
	LUNAR_ENABLE_COMMON_BASELINE(ContentPadding);
	LUNAR_ENABLE_COMMON_BASELINE(Opacity);
	LUNAR_ENABLE_COMMON_BASELINE(RenderTransform);
	LUNAR_ENABLE_COMMON_BASELINE(RenderTransformPivot);
	LUNAR_ENABLE_COMMON_BASELINE(MinDesiredSize);
	LUNAR_ENABLE_COMMON_BASELINE(MaxDesiredSize);
#undef LUNAR_ENABLE_COMMON_BASELINE

	StyleBaselineCommonStyle.BackgroundBrush = *FStyleDefaults::GetNoBrush();
	StyleBaselineCommonStyle.BorderBrush = *FStyleDefaults::GetNoBrush();
	StyleBaselineCommonStyle.ForegroundBrush = *FStyleDefaults::GetNoBrush();
	StyleBaselineCommonStyle.BackgroundTint = FLinearColor::White;
	StyleBaselineCommonStyle.ContentColor = FSlateColor(FLinearColor::White);
	StyleBaselineCommonStyle.TextColor = FSlateColor(FLinearColor::White);
	StyleBaselineCommonStyle.TextShadowColor = FLinearColor::Transparent;
	StyleBaselineCommonStyle.TextShadowOffset = FVector2D::ZeroVector;
	if (!TextBlockStyleBaselines.IsEmpty())
	{
		StyleBaselineCommonStyle.Font = TextBlockStyleBaselines[0].Font;
		StyleBaselineCommonStyle.TextShadowColor = TextBlockStyleBaselines[0].ShadowColorAndOpacity;
		StyleBaselineCommonStyle.TextShadowOffset = TextBlockStyleBaselines[0].ShadowOffset;
	}
	StyleBaselineCommonStyle.Padding = FMargin(0.0f);
	StyleBaselineCommonStyle.ContentPadding = FMargin(0.0f);
	StyleBaselineCommonStyle.Opacity = GetRenderOpacity();
	StyleBaselineCommonStyle.RenderTransform = GetRenderTransform();
	StyleBaselineCommonStyle.RenderTransformPivot = GetRenderTransformPivot();
	StyleBaselineCommonStyle.MinDesiredSize = FVector2D::ZeroVector;
	StyleBaselineCommonStyle.MaxDesiredSize = FVector2D::ZeroVector;
	bHasStyleBaseline = true;
}

FLunarCommonStylePatch ULunarNavigableWidget::MaterializeCommonStyleSnapshot(
	const FLunarCommonStylePatch& ResolvedStyle) const
{
	FLunarCommonStylePatch Result = StyleBaselineCommonStyle;
	LunarStyleResolver::MergeCommonStylePatch(Result, ResolvedStyle);
	Result.TextColor = FSlateColor(
		LunarContentStyleLayer.IsValid()
			? LunarContentStyleLayer->ResolveAgainstInheritedStyle(Result.TextColor)
			: LunarNavigableWidget_Private::ResolveSlateColor(
				Result.TextColor,
				StyleBaselineCommonStyle.TextColor.GetSpecifiedColor()));
	Result.ContentColor = FSlateColor(LunarNavigableWidget_Private::ResolveSlateColor(
		Result.ContentColor,
		Result.TextColor.GetSpecifiedColor()));
	Result.Transition = ResolvedStyle.Transition;
	return Result;
}

void ULunarNavigableWidget::ApplyResolvedTextStyle(const FLunarCommonStylePatch& ResolvedStyle)
{
	EnsureTextBlockStyleBaselines();
	const bool bInterpolating = bStyleTransitionActive && StyleTransitionDuration > UE_SMALL_NUMBER;
	float EasedAlpha = 1.0f;
	if (bInterpolating)
	{
		const float Alpha = FMath::Clamp(StyleTransitionElapsed / StyleTransitionDuration, 0.0f, 1.0f);
		EasedAlpha = static_cast<float>(UKismetMathLibrary::Ease(
			0.0,
			1.0,
			Alpha,
			TransitionTargetCommonStyle.Transition.Easing));
	}

	for (const FLunarTextBlockStyleBaseline& Baseline : TextBlockStyleBaselines)
	{
		UTextBlock* TextBlock = Baseline.TextBlock.Get();
		if (!TextBlock)
		{
			continue;
		}

		if (bInterpolating)
		{
			const FLinearColor SourceTextColor = bUseCapturedTextTransitionSource
				? Baseline.CapturedTransitionSourceColor
				: (TransitionSourceAuthoredCommonStyle.bOverrideTextColor
					? TransitionSourceCommonStyle.TextColor.GetSpecifiedColor()
					: LunarNavigableWidget_Private::ResolveSlateColor(
						Baseline.ColorAndOpacity,
						TransitionSourceCommonStyle.TextColor.GetSpecifiedColor()));
			const FLinearColor TargetTextColor = TransitionTargetAuthoredCommonStyle.bOverrideTextColor
				? TransitionTargetCommonStyle.TextColor.GetSpecifiedColor()
				: LunarNavigableWidget_Private::ResolveSlateColor(
					Baseline.ColorAndOpacity,
					TransitionTargetCommonStyle.TextColor.GetSpecifiedColor());
			TextBlock->SetColorAndOpacity(FSlateColor(FMath::Lerp(SourceTextColor, TargetTextColor, EasedAlpha)));

			const FLinearColor SourceShadowColor = bUseCapturedTextTransitionSource
				? Baseline.CapturedTransitionSourceShadowColor
				: (TransitionSourceAuthoredCommonStyle.bOverrideTextShadowColor
					? TransitionSourceCommonStyle.TextShadowColor
					: Baseline.ShadowColorAndOpacity);
			const FLinearColor TargetShadowColor = TransitionTargetAuthoredCommonStyle.bOverrideTextShadowColor
				? TransitionTargetCommonStyle.TextShadowColor
				: Baseline.ShadowColorAndOpacity;
			TextBlock->SetShadowColorAndOpacity(FMath::Lerp(SourceShadowColor, TargetShadowColor, EasedAlpha));

			const FVector2D SourceShadowOffset = bUseCapturedTextTransitionSource
				? Baseline.CapturedTransitionSourceShadowOffset
				: (TransitionSourceAuthoredCommonStyle.bOverrideTextShadowOffset
					? TransitionSourceCommonStyle.TextShadowOffset
					: Baseline.ShadowOffset);
			const FVector2D TargetShadowOffset = TransitionTargetAuthoredCommonStyle.bOverrideTextShadowOffset
				? TransitionTargetCommonStyle.TextShadowOffset
				: Baseline.ShadowOffset;
			TextBlock->SetShadowOffset(FMath::Lerp(SourceShadowOffset, TargetShadowOffset, EasedAlpha));
		}
		else
		{
			TextBlock->SetColorAndOpacity(
				LogicalTargetAuthoredCommonStyle.bOverrideTextColor
					? LogicalTargetAuthoredCommonStyle.TextColor
					: Baseline.ColorAndOpacity);
			TextBlock->SetShadowColorAndOpacity(
				LogicalTargetAuthoredCommonStyle.bOverrideTextShadowColor
					? LogicalTargetAuthoredCommonStyle.TextShadowColor
					: Baseline.ShadowColorAndOpacity);
			TextBlock->SetShadowOffset(
				LogicalTargetAuthoredCommonStyle.bOverrideTextShadowOffset
					? LogicalTargetAuthoredCommonStyle.TextShadowOffset
					: Baseline.ShadowOffset);
		}

		TextBlock->SetFont(
			LogicalTargetAuthoredCommonStyle.bOverrideFont
				? LogicalTargetAuthoredCommonStyle.Font
				: Baseline.Font);
	}
}

void ULunarNavigableWidget::ApplyResolvedCommonStyle(const FLunarCommonStylePatch& ResolvedStyle)
{
	EnsureStyleBaseline();
	if (LunarBackgroundImage.IsValid())
	{
		LunarBackgroundImage->SetImage(&DisplayedCommonStyle.BackgroundBrush);
		LunarBackgroundImage->SetColorAndOpacity(ResolvedStyle.BackgroundTint);
	}
	if (LunarForegroundImage.IsValid())
	{
		LunarForegroundImage->SetImage(&DisplayedCommonStyle.ForegroundBrush);
	}
	if (LunarOuterPaddingLayer.IsValid())
	{
		LunarOuterPaddingLayer->SetPadding(ResolvedStyle.Padding);
	}
	if (LunarBorderLayer.IsValid())
	{
		LunarBorderLayer->SetBorderImage(&DisplayedCommonStyle.BorderBrush);
		LunarBorderLayer->SetPadding(ResolvedStyle.ContentPadding);
	}
	if (LunarContentStyleLayer.IsValid())
	{
		const FSlateColor EffectiveTextColor = !bStyleTransitionActive
			? (LogicalTargetAuthoredCommonStyle.bOverrideTextColor
				? LogicalTargetAuthoredCommonStyle.TextColor
				: FSlateColor::UseForeground())
			: ResolvedStyle.TextColor;
		const FLinearColor EffectiveForeground =
			LunarContentStyleLayer->ResolveAgainstInheritedStyle(EffectiveTextColor);
		const FLinearColor EffectiveContentColor = !bStyleTransitionActive
			? (LogicalTargetAuthoredCommonStyle.bOverrideContentColor
				? LunarNavigableWidget_Private::ResolveSlateColor(
					LogicalTargetAuthoredCommonStyle.ContentColor,
					EffectiveForeground)
				: FLinearColor::White)
			: ResolvedStyle.ContentColor.GetSpecifiedColor();
		LunarContentStyleLayer->SetColorAndOpacity(EffectiveContentColor);
		LunarContentStyleLayer->SetForegroundColor(EffectiveTextColor);
	}
	if (LunarStyleSizeBox.IsValid())
	{
		LunarStyleSizeBox->SetMinDesiredWidth(
			ResolvedStyle.MinDesiredSize.X > 0.0f
				? FOptionalSize(ResolvedStyle.MinDesiredSize.X)
				: FOptionalSize());
		LunarStyleSizeBox->SetMinDesiredHeight(
			ResolvedStyle.MinDesiredSize.Y > 0.0f
				? FOptionalSize(ResolvedStyle.MinDesiredSize.Y)
				: FOptionalSize());
		LunarStyleSizeBox->SetMaxDesiredWidth(
			ResolvedStyle.MaxDesiredSize.X > 0.0f
				? FOptionalSize(ResolvedStyle.MaxDesiredSize.X)
				: FOptionalSize());
		LunarStyleSizeBox->SetMaxDesiredHeight(
			ResolvedStyle.MaxDesiredSize.Y > 0.0f
				? FOptionalSize(ResolvedStyle.MaxDesiredSize.Y)
				: FOptionalSize());
	}

	SetRenderOpacity(FMath::Clamp(ResolvedStyle.Opacity, 0.0f, 1.0f));
	SetRenderTransform(ResolvedStyle.RenderTransform);
	SetRenderTransformPivot(ResolvedStyle.RenderTransformPivot);
	ApplyResolvedTextStyle(ResolvedStyle);
}

void ULunarNavigableWidget::NativeOnNativeFocusDelegationStarted(UWidget* NativeFocusWidget)
{
	DelegatedFocusTextSnapshotWidget.Reset();
	DelegatedFocusTextSnapshot = FText::GetEmpty();
	bHasDelegatedFocusTextSnapshot = false;

	FText CapturedText;
	if (LunarNavigableWidget_Private::TryGetEditableText(NativeFocusWidget, CapturedText))
	{
		DelegatedFocusTextSnapshotWidget = NativeFocusWidget;
		DelegatedFocusTextSnapshot = MoveTemp(CapturedText);
		bHasDelegatedFocusTextSnapshot = true;
	}
}

void ULunarNavigableWidget::NativeOnNativeFocusDelegationCommitted(UWidget* NativeFocusWidget)
{
	DelegatedFocusTextSnapshotWidget.Reset();
	DelegatedFocusTextSnapshot = FText::GetEmpty();
	bHasDelegatedFocusTextSnapshot = false;
}

void ULunarNavigableWidget::NativeOnNativeFocusDelegationCancelled(UWidget* NativeFocusWidget)
{
	UWidget* SnapshotWidget = DelegatedFocusTextSnapshotWidget.Get();
	const FText SnapshotText = DelegatedFocusTextSnapshot;
	const bool bRestoreSnapshot = bHasDelegatedFocusTextSnapshot && IsValid(SnapshotWidget);

	DelegatedFocusTextSnapshotWidget.Reset();
	DelegatedFocusTextSnapshot = FText::GetEmpty();
	bHasDelegatedFocusTextSnapshot = false;

	if (bRestoreSnapshot)
	{
		LunarNavigableWidget_Private::TrySetEditableText(SnapshotWidget, SnapshotText);
	}
}

bool ULunarNavigableWidget::NativeShouldCommitNativeFocusDelegationOnAccept(const FKey& Key) const
{
	return true;
}

void ULunarNavigableWidget::SetLunarValueState(const FGameplayTag NewValueStateTag)
{
	const FGameplayTag ResolvedTag = NewValueStateTag.IsValid()
		? NewValueStateTag
		: LunarGameplayTags::UI_State_Value_Normal.GetTag();
	if (CurrentValueStateTag != ResolvedTag)
	{
		CurrentValueStateTag = ResolvedTag;
		RefreshVisualState();
	}
}

void ULunarNavigableWidget::RefreshLunarAccessibility()
{
	SynchronizeLunarAccessibility();
}

void ULunarNavigableWidget::NotifyLunarAccessibleValueChanged(const FText& NewValueText)
{
	SynchronizeLunarAccessibility();
#if WITH_ACCESSIBILITY
	if (!NewValueText.IsEmpty() && FSlateApplication::IsInitialized())
	{
		if (const TSharedPtr<SWidget> SlateWidget = GetCachedWidget())
		{
			FSlateApplicationBase::Get().GetAccessibleMessageHandler()->OnWidgetEventRaised(
				FSlateAccessibleMessageHandler::FSlateWidgetAccessibleEventArgs(
					SlateWidget.ToSharedRef(),
					EAccessibleEvent::Notification,
					FVariant(),
					FVariant(NewValueText.ToString()),
					LunarNavigableWidget_Private::GetAccessibleUserIndex(this)));
		}
	}
#endif
}

void ULunarNavigableWidget::CancelPointerPress()
{
	if (!bPointerPressed)
	{
		return;
	}
	bPointerPressed = false;
	bPointerActivationEligible = false;
	if (FSlateApplication::IsInitialized())
	{
		if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
		{
			if (const TSharedPtr<FSlateUser> SlateUser = LocalPlayer->GetSlateUser())
			{
				FSlateApplication::Get().ReleaseAllPointerCapture(SlateUser->GetUserIndex());
			}
		}
	}
	NativeOnLunarReleased();
}

ULunarNavigationSubsystem* ULunarNavigableWidget::ResolveNavigationSubsystem() const
{
	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		return LocalPlayer->GetSubsystem<ULunarNavigationSubsystem>();
	}
	return nullptr;
}

void ULunarNavigableWidget::SetLunarSelectedFromSubsystem(const bool bSelected)
{
	if (bLunarSelected == bSelected)
	{
		return;
	}
	bLunarSelected = bSelected;
	if (bLunarSelected)
	{
		NativeOnLunarSelected();
	}
	else
	{
		NativeOnLunarUnselected();
	}
}

void ULunarNavigableWidget::NotifyLunarRejectedFromSubsystem(const FLunarUIActionContext& ActionContext)
{
	NativeOnLunarRejected();
}

void ULunarNavigableWidget::HandleInputPresentationChanged(const ELunarInputDeviceType InputDevice, const bool bPointerPresentationActive)
{
	const bool bDeviceChanged = LastObservedInputDevice != InputDevice;
	const bool bPointerPresentationChanged = bLastObservedPointerPresentation != bPointerPresentationActive;
	LastObservedInputDevice = InputDevice;
	bLastObservedPointerPresentation = bPointerPresentationActive;
	if (bDeviceChanged)
	{
		NativeOnLunarInputDeviceChanged(InputDevice);
	}
	else
	{
		if (bPointerPresentationChanged)
		{
			RefreshVisualState();
		}
		// The subsystem also rebroadcasts the current presentation to invalidate bindings,
		// icon sets, settings, and registry-derived prompt data without changing device.
		InvalidateInputPrompt();
	}
}

void ULunarNavigableWidget::InvalidateInputPrompt()
{
	bPromptDirty = true;
	NativeOnInputPromptInvalidated();
}

bool ULunarNavigableWidget::IsEffectivelyInteractive() const
{
	if (HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed))
	{
		return false;
	}

	const UWidget* Current = this;
	TSet<TObjectKey<UWidget>> Visited;
	while (Current && !Visited.Contains(TObjectKey<UWidget>(const_cast<UWidget*>(Current))))
	{
		Visited.Add(TObjectKey<UWidget>(const_cast<UWidget*>(Current)));
		if (!Current->IsVisible() || !Current->GetIsEnabled())
		{
			return false;
		}

		if (const UWidget* Parent = Current->GetParent())
		{
			Current = Parent;
			continue;
		}

		const UWidget* OuterWidget = nullptr;
		for (const UObject* Outer = Current->GetOuter(); Outer; Outer = Outer->GetOuter())
		{
			if (const UWidget* Candidate = Cast<UWidget>(Outer))
			{
				OuterWidget = Candidate;
				break;
			}
		}
		Current = OuterWidget;
	}
	return true;
}

void ULunarNavigableWidget::SynchronizeLunarAccessibility()
{
	if (const TSharedPtr<SWidget> SlateWidget = GetCachedWidget())
	{
		if (AccessibleName.IsEmpty())
		{
			SlateWidget->SetAccessibleBehavior(EAccessibleBehavior::Auto);
		}
		else
		{
			SlateWidget->SetAccessibleBehavior(EAccessibleBehavior::Custom, TAttribute<FText>(AccessibleName), EAccessibleType::Main);
		}

		FText AccessibleSummary = AccessibleDescription;
		const FText AccessibleValue = NativeGetLunarAccessibleValueText();
		if (!AccessibleValue.IsEmpty())
		{
			AccessibleSummary = AccessibleSummary.IsEmpty()
				? AccessibleValue
				: FText::Format(
					NSLOCTEXT("LunarNavigation", "AccessibleDescriptionAndValue", "{0} {1}"),
					AccessibleSummary,
					AccessibleValue);
		}
		if (!IsEffectivelyInteractive() && !DisabledReason.IsEmpty())
		{
			AccessibleSummary = AccessibleSummary.IsEmpty()
				? DisabledReason
				: FText::Format(
					NSLOCTEXT("LunarNavigation", "AccessibleDisabledSummary", "{0} {1}"),
					AccessibleSummary,
					DisabledReason);
		}

		if (!AccessibleSummary.IsEmpty())
		{
			SlateWidget->SetAccessibleBehavior(EAccessibleBehavior::Custom, TAttribute<FText>(AccessibleSummary), EAccessibleType::Summary);
		}
		else
		{
			SlateWidget->SetAccessibleBehavior(EAccessibleBehavior::Auto, TAttribute<FText>(), EAccessibleType::Summary);
		}
	}
}

void ULunarNavigableWidget::ApplyStyleTarget(const FLunarCommonStylePatch& NewTarget)
{
	EnsureStyleBaseline();
	const FLunarCommonStylePatch MaterializedTarget = MaterializeCommonStyleSnapshot(NewTarget);
	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	const bool bReduceMotion = Settings && Settings->Navigation.Accessibility.bReduceMotion;
	if (bReduceMotion)
	{
		bStyleTransitionActive = false;
		bStyleTransitionReversing = false;
		bUseCapturedTextTransitionSource = false;
		StyleTransitionElapsed = 0.0f;
		StyleTransitionDuration = 0.0f;
		TransitionSourceCommonStyle = MaterializedTarget;
		TransitionTargetCommonStyle = MaterializedTarget;
		LogicalTargetCommonStyle = MaterializedTarget;
		TransitionSourceAuthoredCommonStyle = NewTarget;
		TransitionTargetAuthoredCommonStyle = NewTarget;
		LogicalTargetAuthoredCommonStyle = NewTarget;
		ApplyDisplayedStyle(MaterializedTarget);
		return;
	}

	if (bHasDisplayedStyle
		&& LunarStyleResolver::AreCommonStylesEquivalent(LogicalTargetAuthoredCommonStyle, NewTarget))
	{
		ApplyResolvedCommonStyle(DisplayedCommonStyle);
		return;
	}

	const bool bWasTransitionActive = bStyleTransitionActive;
	if (bStyleTransitionActive)
	{
		const bool bReturnsToSource = !bStyleTransitionReversing
			&& LunarStyleResolver::AreCommonStylesEquivalent(TransitionSourceCommonStyle, MaterializedTarget);
		const bool bReturnsToForwardTarget = bStyleTransitionReversing
			&& LunarStyleResolver::AreCommonStylesEquivalent(TransitionTargetCommonStyle, MaterializedTarget);
		if (bReturnsToSource || bReturnsToForwardTarget)
		{
			LogicalTargetCommonStyle = MaterializedTarget;
			LogicalTargetAuthoredCommonStyle = NewTarget;
			bStyleTransitionReversing = bReturnsToSource;
			FLunarCommonStylePatch TransitionSnapshot = DisplayedCommonStyle;
			LunarStyleResolver::ApplyCommonDiscreteFields(
				TransitionSnapshot,
				bStyleTransitionReversing ? TransitionSourceCommonStyle : TransitionTargetCommonStyle);
			ApplyDisplayedStyle(TransitionSnapshot);
			return;
		}
	}

	const bool bCanTransition = bHasDisplayedStyle
		&& MaterializedTarget.Transition.bEnabled
		&& MaterializedTarget.Transition.Duration > 0.0f;
	if (!bCanTransition)
	{
		bStyleTransitionActive = false;
		bStyleTransitionReversing = false;
		bUseCapturedTextTransitionSource = false;
		StyleTransitionElapsed = 0.0f;
		StyleTransitionDuration = 0.0f;
		TransitionSourceCommonStyle = MaterializedTarget;
		TransitionTargetCommonStyle = MaterializedTarget;
		LogicalTargetCommonStyle = MaterializedTarget;
		TransitionSourceAuthoredCommonStyle = NewTarget;
		TransitionTargetAuthoredCommonStyle = NewTarget;
		LogicalTargetAuthoredCommonStyle = NewTarget;
		ApplyDisplayedStyle(MaterializedTarget);
		return;
	}

	if (LunarStyleResolver::AreCommonStylesEquivalent(DisplayedCommonStyle, MaterializedTarget))
	{
		bStyleTransitionActive = false;
		bStyleTransitionReversing = false;
		bUseCapturedTextTransitionSource = false;
		StyleTransitionElapsed = 0.0f;
		StyleTransitionDuration = 0.0f;
		TransitionSourceCommonStyle = MaterializedTarget;
		TransitionTargetCommonStyle = MaterializedTarget;
		LogicalTargetCommonStyle = MaterializedTarget;
		TransitionSourceAuthoredCommonStyle = NewTarget;
		TransitionTargetAuthoredCommonStyle = NewTarget;
		LogicalTargetAuthoredCommonStyle = NewTarget;
		ApplyDisplayedStyle(MaterializedTarget);
		return;
	}

	bUseCapturedTextTransitionSource = true;
	if (bUseCapturedTextTransitionSource)
	{
		EnsureTextBlockStyleBaselines();
		for (FLunarTextBlockStyleBaseline& Baseline : TextBlockStyleBaselines)
		{
			if (const UTextBlock* TextBlock = Baseline.TextBlock.Get())
			{
				Baseline.CapturedTransitionSourceColor = LunarContentStyleLayer.IsValid()
					? LunarContentStyleLayer->ResolveForChild(TextBlock->GetColorAndOpacity())
					: LunarNavigableWidget_Private::ResolveSlateColor(
						TextBlock->GetColorAndOpacity(),
						StyleBaselineCommonStyle.TextColor.GetSpecifiedColor());
				Baseline.CapturedTransitionSourceShadowColor = TextBlock->GetShadowColorAndOpacity();
				Baseline.CapturedTransitionSourceShadowOffset = TextBlock->GetShadowOffset();
			}
		}
	}
	TransitionSourceCommonStyle = DisplayedCommonStyle;
	TransitionTargetCommonStyle = MaterializedTarget;
	LogicalTargetCommonStyle = MaterializedTarget;
	TransitionSourceAuthoredCommonStyle = bWasTransitionActive
		? DisplayedCommonStyle
		: LogicalTargetAuthoredCommonStyle;
	TransitionTargetAuthoredCommonStyle = NewTarget;
	LogicalTargetAuthoredCommonStyle = NewTarget;
	StyleTransitionElapsed = 0.0f;
	StyleTransitionDuration = MaterializedTarget.Transition.Duration;
	bStyleTransitionActive = true;
	bStyleTransitionReversing = false;
	ApplyDisplayedStyle(LunarStyleResolver::InterpolateCommonStylePatch(
		TransitionSourceCommonStyle,
		TransitionTargetCommonStyle,
		0.0f));
}

void ULunarNavigableWidget::ApplyDisplayedStyle(const FLunarCommonStylePatch& NewDisplayedStyle)
{
	DisplayedCommonStyle = NewDisplayedStyle;
	bHasDisplayedStyle = true;
	ApplyResolvedCommonStyle(DisplayedCommonStyle);
}

void ULunarNavigableWidget::PlaySelectedFeedback()
{
	ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem();
	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	if (!NavigationSubsystem || !Settings || NavigationSubsystem->IsPointerPresentationActive())
	{
		return;
	}
	if (const FLunarUISoundSpec* Sound = LunarNavigableWidget_Private::ResolveSound(SoundOverrides.NavigationSelected, Settings->Navigation.Audio.DefaultNavigationSelectedSound))
	{
		NavigationSubsystem->PlayUISound(*Sound);
	}
	if (NavigationSubsystem->GetLastInputDevice() == ELunarInputDeviceType::Gamepad)
	{
		if (const FLunarUIHapticSpec* Haptic = LunarNavigableWidget_Private::ResolveHaptic(HapticOverrides.NavigationSelected, Settings->Navigation.Haptics.DefaultNavigationSelectedHaptic))
		{
			NavigationSubsystem->PlayUIHaptic(*Haptic);
		}
	}
}

void ULunarNavigableWidget::PlayPressedFeedback()
{
	ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem();
	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	if (!NavigationSubsystem || !Settings)
	{
		return;
	}
	const bool bPointer = bPointerPressed || NavigationSubsystem->IsPointerPresentationActive();
	const FLunarUISoundOverride& Override = bPointer ? SoundOverrides.PointerPressed : SoundOverrides.NavigationPressed;
	const FLunarUISoundSpec& Global = bPointer ? Settings->Navigation.Audio.DefaultPointerPressedSound : Settings->Navigation.Audio.DefaultNavigationPressedSound;
	if (const FLunarUISoundSpec* Sound = LunarNavigableWidget_Private::ResolveSound(Override, Global))
	{
		NavigationSubsystem->PlayUISound(*Sound);
	}
	if (!bPointer && NavigationSubsystem->GetLastInputDevice() == ELunarInputDeviceType::Gamepad)
	{
		if (const FLunarUIHapticSpec* Haptic = LunarNavigableWidget_Private::ResolveHaptic(HapticOverrides.NavigationPressed, Settings->Navigation.Haptics.DefaultNavigationPressedHaptic))
		{
			NavigationSubsystem->PlayUIHaptic(*Haptic);
		}
	}
}

void ULunarNavigableWidget::PlayActivatedFeedback()
{
	ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem();
	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	if (!NavigationSubsystem || !Settings)
	{
		return;
	}
	const bool bPointer = NavigationSubsystem->IsPointerPresentationActive();
	const FLunarUISoundOverride& Override = bPointer ? SoundOverrides.PointerActivated : SoundOverrides.NavigationActivated;
	const FLunarUISoundSpec& Global = bPointer ? Settings->Navigation.Audio.DefaultPointerActivatedSound : Settings->Navigation.Audio.DefaultNavigationActivatedSound;
	if (const FLunarUISoundSpec* Sound = LunarNavigableWidget_Private::ResolveSound(Override, Global))
	{
		NavigationSubsystem->PlayUISound(*Sound);
	}
	if (!bPointer && NavigationSubsystem->GetLastInputDevice() == ELunarInputDeviceType::Gamepad)
	{
		if (const FLunarUIHapticSpec* Haptic = LunarNavigableWidget_Private::ResolveHaptic(HapticOverrides.NavigationActivated, Settings->Navigation.Haptics.DefaultNavigationActivatedHaptic))
		{
			NavigationSubsystem->PlayUIHaptic(*Haptic);
		}
	}
}

void ULunarNavigableWidget::PlayRejectedFeedback()
{
	ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem();
	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	if (!NavigationSubsystem || !Settings)
	{
		return;
	}
	const bool bPointer = NavigationSubsystem->IsPointerPresentationActive();
	const FLunarUISoundOverride& Override = bPointer ? SoundOverrides.PointerRejected : SoundOverrides.NavigationRejected;
	const FLunarUISoundSpec& Global = bPointer ? Settings->Navigation.Audio.DefaultPointerRejectedSound : Settings->Navigation.Audio.DefaultNavigationRejectedSound;
	if (const FLunarUISoundSpec* Sound = LunarNavigableWidget_Private::ResolveSound(Override, Global))
	{
		NavigationSubsystem->PlayUISound(*Sound);
	}
	if (!bPointer && NavigationSubsystem->GetLastInputDevice() == ELunarInputDeviceType::Gamepad)
	{
		if (const FLunarUIHapticSpec* Haptic = LunarNavigableWidget_Private::ResolveHaptic(HapticOverrides.NavigationRejected, Settings->Navigation.Haptics.DefaultNavigationRejectedHaptic))
		{
			NavigationSubsystem->PlayUIHaptic(*Haptic);
		}
	}
}

void ULunarNavigableWidget::PlayPointerHoveredFeedback()
{
	ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem();
	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	if (!NavigationSubsystem || !Settings)
	{
		return;
	}
	if (const FLunarUISoundSpec* Sound = LunarNavigableWidget_Private::ResolveSound(SoundOverrides.PointerHovered, Settings->Navigation.Audio.DefaultPointerHoveredSound))
	{
		NavigationSubsystem->PlayUISound(*Sound);
	}
}
