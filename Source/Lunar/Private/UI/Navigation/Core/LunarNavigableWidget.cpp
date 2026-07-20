// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarNavigableWidget.cpp
 * @brief Implements common Lunar control input, state publication, prompt, feedback, focus, and accessibility behavior.
 * @ingroup LunarNavigationCore
 */

#include "UI/Navigation/Core/LunarNavigableWidget.h"

#include "Application/SlateApplicationBase.h"
#include "Components/EditableText.h"
#include "Components/EditableTextBox.h"
#include "Components/MultiLineEditableText.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/PanelWidget.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "Settings/LunarSettings.h"
#include "UI/Navigation/Core/LunarNavigationSubsystem.h"
#include "UI/Navigation/Data/LunarUIHapticFeedbackAsset.h"
#include "UI/Navigation/Data/LunarUISoundFeedbackAsset.h"
#include "UI/Navigation/Prompts/LunarInputPromptReceiver.h"
#include "UI/Navigation/Types/LunarGameplayTags.h"
#include "Widgets/Accessibility/SlateAccessibleMessageHandler.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SWidget.h"

DEFINE_LOG_CATEGORY_STATIC(LogLunarNavigableWidget, Log, All);

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

	/** @param Override Per-event source mode. @param GlobalSound Project default. @param DataAssetSound Matching reusable asset entry, or nullptr. @return Effective sound spec, or nullptr when disabled or unset. */
	const FLunarUISoundSpec* ResolveSound(
		const FLunarUISoundOverride& Override,
		const FLunarUISoundSpec& GlobalSound,
		const FLunarUISoundSpec* DataAssetSound)
	{
		switch (Override.Mode)
		{
		case ELunarFeedbackOverrideMode::Disabled:
			return nullptr;
		case ELunarFeedbackOverrideMode::Custom:
			return Override.CustomSound.Sound ? &Override.CustomSound : nullptr;
		case ELunarFeedbackOverrideMode::UseDataAsset:
			return DataAssetSound && DataAssetSound->Sound ? DataAssetSound : nullptr;
		case ELunarFeedbackOverrideMode::UseGlobal:
		default:
			return GlobalSound.Sound ? &GlobalSound : nullptr;
		}
	}

	/** @param Override Per-event source mode. @param GlobalHaptic Project default. @param DataAssetHaptic Matching reusable asset entry, or nullptr. @return Effective haptic spec, or nullptr when disabled or unset. */
	const FLunarUIHapticSpec* ResolveHaptic(
		const FLunarUIHapticOverride& Override,
		const FLunarUIHapticSpec& GlobalHaptic,
		const FLunarUIHapticSpec* DataAssetHaptic)
	{
		switch (Override.Mode)
		{
		case ELunarFeedbackOverrideMode::Disabled:
			return nullptr;
		case ELunarFeedbackOverrideMode::Custom:
			return Override.CustomHaptic.Effect ? &Override.CustomHaptic : nullptr;
		case ELunarFeedbackOverrideMode::UseDataAsset:
			return DataAssetHaptic && DataAssetHaptic->Effect ? DataAssetHaptic : nullptr;
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

#if WITH_EDITOR
const FText ULunarNavigableWidget::GetPaletteCategory()
{
	return NSLOCTEXT("LunarNavigationPalette", "Category", "Lunar Navigation");
}
#endif

TSharedRef<SWidget> ULunarNavigableWidget::RebuildWidget()
{
	// Serialized Blueprint defaults must not disable the native focus bridge that Lunar owns internally.
	SetIsFocusable(true);
	// Lunar owns logical availability. Slate stays enabled so it neither adds DisabledEffect nor suppresses hover.
	UWidget::SetIsEnabled(true);
	DetachInputPromptReceiver();
	const TSharedRef<SWidget> UserContent = Super::RebuildWidget();

	SAssignNew(LunarInputPromptHost, SBox)
		.Visibility(EVisibility::Collapsed);
	AttachInputPromptReceiver();
	UpdateInputPromptHostVisibility();

	TSharedRef<SOverlay> Presentation = SNew(SOverlay);
	if (const TSharedPtr<SWidget> SpecializedPresentation = RebuildLunarSpecializedPresentation())
	{
		Presentation->AddSlot()
		[
			SpecializedPresentation.ToSharedRef()
		];
	}
	Presentation->AddSlot()
	[
		UserContent
	];
	Presentation->AddSlot()
	[
		LunarInputPromptHost.ToSharedRef()
	];
	return Presentation;
}

TSharedPtr<SWidget> ULunarNavigableWidget::RebuildLunarSpecializedPresentation()
{
	return nullptr;
}

void ULunarNavigableWidget::ReleaseSlateResources(const bool bReleaseChildren)
{
	DetachInputPromptReceiver();
	LunarInputPromptHost.Reset();
	bPromptDirty = true;
	bHasPublishedVisualState = false;
	Super::ReleaseSlateResources(bReleaseChildren);
}

void ULunarNavigableWidget::SynchronizeProperties()
{
	SetIsFocusable(true);
	Super::SynchronizeProperties();
	// Super may reapply a serialized native bIsEnabled value. Keep only Lunar's logical state authoritative.
	UWidget::SetIsEnabled(true);
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
	ResetLunarHoldTracking();

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
	if ((bPointerHovered || bPointerPressed) && !CanReceiveLunarPointerPresentation())
	{
		CancelPointerPress();
		bPointerHovered = false;
		RefreshVisualState();
		UpdateInputPromptHostVisibility();
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
	AdvanceLunarHoldTracking(InDeltaTime);


}

bool ULunarNavigableWidget::RequestLunarSelection()
{
	if (ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem())
	{
		return NavigationSubsystem->SetSelectedWidget(this);
	}
	return false;
}

bool ULunarNavigableWidget::RequestLunarPointerSelection()
{
	if (ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem())
	{
		return NavigationSubsystem->SetSelectedWidgetFromPointer(this);
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
		|| !IsLunarHierarchyAvailable(bCanReceiveSelectionWhenDisabled)
		|| HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed))
	{
		return false;
	}
	return true;
}

bool ULunarNavigableWidget::IsNavigationInputAllowed(const ELunarInputDeviceType InputDevice) const
{
	switch (InputDevice)
	{
	case ELunarInputDeviceType::KeyboardMouse:
		return bAllowKeyboardInput;
	case ELunarInputDeviceType::Gamepad:
		return bAllowGamepadInput;
	case ELunarInputDeviceType::Touch:
		return bAllowTouchInput;
	case ELunarInputDeviceType::Unknown:
	default:
		return true;
	}
}

void ULunarNavigableWidget::SetLunarEnabled(const bool bEnabled)
{
	// Never expose the logical disabled state to Slate; Slate would dim and remove this widget from pointer routing.
	UWidget::SetIsEnabled(true);
	if (bLunarEnabled == bEnabled)
	{
		return;
	}

	bLunarEnabled = bEnabled;
	if (!bLunarEnabled)
	{
		CancelPointerPress();
		if (bNavigationPressed)
		{
			bNavigationPressed = false;
			NativeOnLunarReleased();
		}
		bRejectedNavigationPressed = false;
		ResetLunarHoldTracking();
	}
	if (!CanReceiveLunarPointerPresentation())
	{
		bPointerHovered = false;
	}

	bLastKnownEffectivelyInteractive = IsEffectivelyInteractive();
	SynchronizeLunarAccessibility();
	InvalidateInputPrompt();
	RefreshVisualState();
	UpdateInputPromptHostVisibility();
	if (ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem())
	{
		NavigationSubsystem->NotifyNavigableWidgetConfigurationChanged(this, false);
	}
}

void ULunarNavigableWidget::SetIsEnabled(const bool bInIsEnabled)
{
	SetLunarEnabled(bInIsEnabled);
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
	if (ActionContext.InputEvent == IE_Released && bRejectedNavigationPressed)
	{
		bRejectedNavigationPressed = false;
	}
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
	else if (Result == ELunarUIActionResult::Rejected && ActionContext.InputEvent == IE_Pressed)
	{
		// Rejected input receives only a pressed visual; it never starts the Pressed/Hold lifecycle.
		bRejectedNavigationPressed = true;
	}
	RefreshVisualState();
	return Result;
}

void ULunarNavigableWidget::RefreshVisualState()
{
	if (!CurrentValueStateTag.IsValid())
	{
		CurrentValueStateTag = LunarGameplayTags::UI_State_Value_Normal.GetTag();
	}

	FLunarUIVisualState NewVisualState;
	bool bIsDesignerPreview = false;

#if WITH_EDITOR
	bIsDesignerPreview = IsDesignTime();
#endif

#if WITH_EDITORONLY_DATA
	if (bIsDesignerPreview && PreviewMode == ELunarVisualStatePreviewMode::Custom)
	{
		NewVisualState = PreviewVisualState;
	}
	else
#endif
	{
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
			const bool bNavigationPresentationAllowed = IsNavigationInputAllowed(NewVisualState.InputDevice);
			NewVisualState.InteractionState = bNavigationPresentationAllowed
				&& (bNavigationPressed || bRejectedNavigationPressed)
				&& bLunarSelected
				? ELunarUIInteractionState::NavigationPressed
				: (bNavigationPresentationAllowed && bLunarSelected
					? ELunarUIInteractionState::NavigationSelected
					: ELunarUIInteractionState::NavigationNormal);
		}
	}

	CurrentVisualState = NewVisualState;
	const bool bStateChanged = !bHasPublishedVisualState
		|| !LunarNavigableWidget_Private::VisualStatesEqual(LastPublishedVisualState, NewVisualState);
	if (!bStateChanged)
	{
		return;
	}

	const FLunarUIVisualState PreviousState = bHasPublishedVisualState
		? LastPublishedVisualState
		: NewVisualState;
	LastPublishedVisualState = NewVisualState;
	bHasPublishedVisualState = true;
	NativeOnLunarVisualStateChanged(PreviousState, NewVisualState, bIsDesignerPreview);
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
		{
			const ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem();
			const bool bPointerPresentationActive = NavigationSubsystem
				&& NavigationSubsystem->IsPointerPresentationActive();
			bShowHost = bPointerPresentationActive
				? bPointerHovered
				: bLunarSelected;
			break;
		}
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
	if (CanReceiveLunarPointerPresentation() && !InMouseEvent.IsTouchEvent())
	{
		bPointerHovered = true;
		PlayPointerHoveredFeedback();
		RefreshVisualState();
		UpdateInputPromptHostVisibility();
	}
}

void ULunarNavigableWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	bPointerHovered = false;
	RefreshVisualState();
	UpdateInputPromptHostVisibility();
	Super::NativeOnMouseLeave(InMouseEvent);
}

FReply ULunarNavigableWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!bCanInteractWithPointer || InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	const bool bInteractivePress = IsEffectivelyInteractive();
	const bool bInspectableDisabledPress = !bInteractivePress
		&& CanInspectDisabledControlWithDirectInput(bCanInteractWithPointer);
	if (bCanReceiveLunarSelection
		&& (bInteractivePress || bInspectableDisabledPress)
		&& !RequestLunarPointerSelection())
	{
		NativeOnLunarRejected();
		return FReply::Handled();
	}
	if (!bInteractivePress)
	{
		// A disabled control cannot own a successful press lifecycle. Reject the
		// attempt immediately because disabled Slate paths are not guaranteed to
		// deliver the matching release after preview handling or capture changes.
		NativeOnLunarRejected();
		return FReply::Handled();
	}
	bPointerPressed = true;
	bPointerActivationEligible = true;
	bPointerPressRejected = false;
	PointerPressScreenPosition = InMouseEvent.GetScreenSpacePosition();
	NativeOnLunarPressed();
	return FReply::Handled().CaptureMouse(TakeWidget());
}

FReply ULunarNavigableWidget::NativeOnPreviewMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (bCanInteractWithPointer
		&& InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton
		&& !IsEffectivelyInteractive())
	{
		return NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}
	return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
}

FReply ULunarNavigableWidget::NativeOnMouseButtonDoubleClick(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (!bCanInteractWithPointer || InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
	}

	// Slate routes a rapid second press here instead of through MouseButtonDown.
	// Treat it as the next ordinary press so every completed click activates exactly once.
	return NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply ULunarNavigableWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!bPointerPressed || InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
	}

	const bool bActivate = bPointerActivationEligible
		&& InGeometry.IsUnderLocation(InMouseEvent.GetScreenSpacePosition());
	const bool bRejectedPress = bPointerPressRejected;
	bPointerPressed = false;
	bPointerActivationEligible = false;
	bPointerPressRejected = false;
	if (bRejectedPress)
	{
		RefreshVisualState();
		if (bActivate)
		{
			NativeOnLunarRejected();
		}
	}
	else
	{
		NativeOnLunarReleased();
		if (bActivate)
		{
			ActivateLunarWidget();
		}
	}
	return FReply::Handled().ReleaseMouseCapture();
}

void ULunarNavigableWidget::NativeOnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent)
{
	if (bPointerPressed)
	{
		const bool bRejectedPress = bPointerPressRejected;
		bPointerPressed = false;
		bPointerActivationEligible = false;
		bPointerPressRejected = false;
		if (bRejectedPress)
		{
			RefreshVisualState();
		}
		else
		{
			NativeOnLunarReleased();
		}
	}
	Super::NativeOnMouseCaptureLost(CaptureLostEvent);
}

FReply ULunarNavigableWidget::NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	if (!bAllowTouchInput || !InGestureEvent.IsTouchEvent() || InGestureEvent.GetPointerIndex() != 0)
	{
		return Super::NativeOnTouchStarted(InGeometry, InGestureEvent);
	}
	const bool bInteractivePress = IsEffectivelyInteractive();
	const bool bInspectableDisabledPress = !bInteractivePress
		&& CanInspectDisabledControlWithDirectInput(bAllowTouchInput);
	if (bCanReceiveLunarSelection
		&& (bInteractivePress || bInspectableDisabledPress)
		&& !RequestLunarPointerSelection())
	{
		NativeOnLunarRejected();
		return FReply::Handled();
	}
	if (!bInteractivePress)
	{
		// Mirror pointer rejection: disabled touch attempts publish one rejection
		// without beginning a press, release, or activation lifecycle.
		NativeOnLunarRejected();
		return FReply::Handled();
	}
	bPointerPressed = true;
	bPointerActivationEligible = true;
	bPointerPressRejected = false;
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
	const bool bRejectedPress = bPointerPressRejected;
	bPointerPressed = false;
	bPointerActivationEligible = false;
	bPointerPressRejected = false;
	if (bRejectedPress)
	{
		RefreshVisualState();
		if (bActivate)
		{
			NativeOnLunarRejected();
		}
	}
	else
	{
		NativeOnLunarReleased();
		if (bActivate)
		{
			ActivateLunarWidget();
		}
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
	OnLunarSelected.Broadcast(this);
	PlaySelectedFeedback();
}

void ULunarNavigableWidget::NativeOnLunarUnselected()
{
	bRejectedNavigationPressed = false;
	if (bNavigationPressed)
	{
		bNavigationPressed = false;
		NativeOnLunarReleased();
	}
	CancelPointerPress();
	RefreshVisualState();
	InvalidateInputPrompt();
	BP_OnLunarUnselected();
	OnLunarUnselected.Broadcast(this);
}

void ULunarNavigableWidget::NativeOnLunarPressed()
{
	RefreshVisualState();
	BP_OnLunarPressed();
	OnLunarPressed.Broadcast(this);
	PlayPressedFeedback();
	BeginLunarHoldTracking();
}

void ULunarNavigableWidget::NativeOnLunarHoldProgress(
	const float HoldSeconds,
	const float PressedSeconds,
	const float DelayLeft,
	const bool bDelayElapsed)
{
	BP_OnLunarHoldProgress(HoldSeconds, PressedSeconds, DelayLeft, bDelayElapsed);
	OnLunarHoldProgress.Broadcast(this, HoldSeconds, PressedSeconds, DelayLeft, bDelayElapsed);
}

void ULunarNavigableWidget::NativeOnLunarReleased()
{
	ResetLunarHoldTracking();
	RefreshVisualState();
	BP_OnLunarReleased();
	OnLunarReleased.Broadcast(this);
}

void ULunarNavigableWidget::NativeOnLunarActivated()
{
	BP_OnLunarActivated();
	OnLunarActivated.Broadcast(this);
	PlayClickedFeedback();
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
	OnLunarRejected.Broadcast(this);
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
					FVariant(DisabledReason),
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
	OnLunarInputDeviceChanged.Broadcast(this, NewInputDevice);
}

void ULunarNavigableWidget::NativeOnLunarVisualStateChanged(
	const FLunarUIVisualState& PreviousState,
	const FLunarUIVisualState& NewState,
	const bool bIsDesignerPreview)
{
	BP_OnLunarVisualStateChanged(PreviousState, NewState, bIsDesignerPreview);
	OnLunarVisualStateChanged.Broadcast(this, PreviousState, NewState, bIsDesignerPreview);
}

void ULunarNavigableWidget::NativeOnInputPromptInvalidated()
{
}

FText ULunarNavigableWidget::NativeGetLunarAccessibleValueText() const
{
	return FText::GetEmpty();
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

void ULunarNavigableWidget::BeginLunarHoldTracking()
{
	ResetLunarHoldTracking();
	if ((!bPointerPressed && !bNavigationPressed) || !IsEffectivelyInteractive())
	{
		return;
	}

	bLunarHoldTracking = true;
	const float DelayLeft = FMath::Max(0.0f, HoldStartDelay);
	bLunarHoldDelayElapsed = DelayLeft <= 0.0f;
	NativeOnLunarHoldProgress(0.0f, 0.0f, DelayLeft, bLunarHoldDelayElapsed);
}

void ULunarNavigableWidget::AdvanceLunarHoldTracking(const float DeltaSeconds)
{
	if (!bLunarHoldTracking)
	{
		return;
	}
	if ((!bPointerPressed && !bNavigationPressed) || !IsEffectivelyInteractive())
	{
		ResetLunarHoldTracking();
		return;
	}

	const float SafeDeltaSeconds = FMath::Max(0.0f, DeltaSeconds);
	LunarPressedSeconds += SafeDeltaSeconds;
	const float DelayLeft = FMath::Max(0.0f, FMath::Max(0.0f, HoldStartDelay) - LunarPressedSeconds);
	if (!bLunarHoldDelayElapsed)
	{
		if (DelayLeft <= 0.0f)
		{
			bLunarHoldDelayElapsed = true;
			LunarHoldSeconds = 0.0f;
		}
	}
	else
	{
		LunarHoldSeconds += SafeDeltaSeconds;
	}
	NativeOnLunarHoldProgress(LunarHoldSeconds, LunarPressedSeconds, DelayLeft, bLunarHoldDelayElapsed);
}

void ULunarNavigableWidget::ResetLunarHoldTracking()
{
	bLunarHoldTracking = false;
	bLunarHoldDelayElapsed = false;
	LunarHoldSeconds = 0.0f;
	LunarPressedSeconds = 0.0f;
}

void ULunarNavigableWidget::CancelPointerPress()
{
	if (!bPointerPressed)
	{
		return;
	}
	const bool bRejectedPress = bPointerPressRejected;
	bPointerPressed = false;
	bPointerActivationEligible = false;
	bPointerPressRejected = false;
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
	if (bRejectedPress)
	{
		RefreshVisualState();
	}
	else
	{
		NativeOnLunarReleased();
	}
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
	return IsLunarHierarchyAvailable(false);
}

bool ULunarNavigableWidget::IsLunarHierarchyAvailable(const bool bAllowSelfDisabled) const
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
		const ULunarNavigableWidget* LunarWidget = Cast<ULunarNavigableWidget>(Current);
		const bool bCurrentEnabled = LunarWidget
			? LunarWidget->bLunarEnabled
			: Current->GetIsEnabled();
		const bool bIgnoreSelfDisabled = Current == this && bAllowSelfDisabled;
		if (!Current->IsVisible() || (!bCurrentEnabled && !bIgnoreSelfDisabled))
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

bool ULunarNavigableWidget::CanInspectDisabledControlWithDirectInput(const bool bInputAllowed) const
{
	return bInputAllowed
		&& !bLunarEnabled
		&& bCanReceiveSelectionWhenDisabled
		&& IsLunarHierarchyAvailable(true);
}

bool ULunarNavigableWidget::CanReceiveLunarPointerPresentation() const
{
	return bCanInteractWithPointer
		&& (IsEffectivelyInteractive() || CanInspectDisabledControlWithDirectInput(bCanInteractWithPointer));
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
			SlateWidget->SetAccessibleBehavior(
				EAccessibleBehavior::Custom,
				TAttribute<FText>(FText::FromString(AccessibleName)),
				EAccessibleType::Main);
		}

		FText AccessibleSummary = FText::FromString(AccessibleDescription);
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
				? FText::FromString(DisabledReason)
				: FText::Format(
					NSLOCTEXT("LunarNavigation", "AccessibleDisabledSummary", "{0} {1}"),
					AccessibleSummary,
					FText::FromString(DisabledReason));
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

void ULunarNavigableWidget::PlaySelectedFeedback()
{
	ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem();
	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	if (!NavigationSubsystem || !Settings || NavigationSubsystem->IsPointerPresentationActive())
	{
		return;
	}
	if (const FLunarUISoundSpec* Sound = LunarNavigableWidget_Private::ResolveSound(
		SoundOverrides.NavigationSelected,
		Settings->Navigation.Audio.DefaultNavigationSelectedSound,
		SoundFeedbackAsset ? &SoundFeedbackAsset->Sounds.NavigationSelected : nullptr))
	{
		NavigationSubsystem->PlayUISound(*Sound);
	}
	if (NavigationSubsystem->GetLastInputDevice() == ELunarInputDeviceType::Gamepad)
	{
		if (const FLunarUIHapticSpec* Haptic = LunarNavigableWidget_Private::ResolveHaptic(
			HapticOverrides.NavigationSelected,
			Settings->Navigation.Haptics.DefaultNavigationSelectedHaptic,
			HapticFeedbackAsset ? &HapticFeedbackAsset->Haptics.NavigationSelected : nullptr))
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
	const FLunarUISoundSpec* DataAssetSound = SoundFeedbackAsset
		? (bPointer ? &SoundFeedbackAsset->Sounds.PointerPressed : &SoundFeedbackAsset->Sounds.NavigationPressed)
		: nullptr;
	if (const FLunarUISoundSpec* Sound = LunarNavigableWidget_Private::ResolveSound(Override, Global, DataAssetSound))
	{
		NavigationSubsystem->PlayUISound(*Sound);
	}
	if (!bPointer && NavigationSubsystem->GetLastInputDevice() == ELunarInputDeviceType::Gamepad)
	{
		if (const FLunarUIHapticSpec* Haptic = LunarNavigableWidget_Private::ResolveHaptic(
			HapticOverrides.NavigationPressed,
			Settings->Navigation.Haptics.DefaultNavigationPressedHaptic,
			HapticFeedbackAsset ? &HapticFeedbackAsset->Haptics.NavigationPressed : nullptr))
		{
			NavigationSubsystem->PlayUIHaptic(*Haptic);
		}
	}
}

void ULunarNavigableWidget::PlayClickedFeedback()
{
	ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem();
	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	if (!NavigationSubsystem || !Settings)
	{
		return;
	}
	const bool bPointer = NavigationSubsystem->IsPointerPresentationActive();
	const FLunarUISoundOverride& Override = bPointer ? SoundOverrides.PointerClicked : SoundOverrides.NavigationClicked;
	const FLunarUISoundSpec& Global = bPointer ? Settings->Navigation.Audio.DefaultPointerClickedSound : Settings->Navigation.Audio.DefaultNavigationClickedSound;
	const FLunarUISoundSpec* DataAssetSound = SoundFeedbackAsset
		? (bPointer ? &SoundFeedbackAsset->Sounds.PointerClicked : &SoundFeedbackAsset->Sounds.NavigationClicked)
		: nullptr;
	if (const FLunarUISoundSpec* Sound = LunarNavigableWidget_Private::ResolveSound(Override, Global, DataAssetSound))
	{
		NavigationSubsystem->PlayUISound(*Sound);
	}
	if (!bPointer && NavigationSubsystem->GetLastInputDevice() == ELunarInputDeviceType::Gamepad)
	{
		if (const FLunarUIHapticSpec* Haptic = LunarNavigableWidget_Private::ResolveHaptic(
			HapticOverrides.NavigationClicked,
			Settings->Navigation.Haptics.DefaultNavigationClickedHaptic,
			HapticFeedbackAsset ? &HapticFeedbackAsset->Haptics.NavigationClicked : nullptr))
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
	const FLunarUISoundSpec* DataAssetSound = SoundFeedbackAsset
		? (bPointer ? &SoundFeedbackAsset->Sounds.PointerRejected : &SoundFeedbackAsset->Sounds.NavigationRejected)
		: nullptr;
	if (const FLunarUISoundSpec* Sound = LunarNavigableWidget_Private::ResolveSound(Override, Global, DataAssetSound))
	{
		NavigationSubsystem->PlayUISound(*Sound);
	}
	if (!bPointer && NavigationSubsystem->GetLastInputDevice() == ELunarInputDeviceType::Gamepad)
	{
		if (const FLunarUIHapticSpec* Haptic = LunarNavigableWidget_Private::ResolveHaptic(
			HapticOverrides.NavigationRejected,
			Settings->Navigation.Haptics.DefaultNavigationRejectedHaptic,
			HapticFeedbackAsset ? &HapticFeedbackAsset->Haptics.NavigationRejected : nullptr))
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
	if (const FLunarUISoundSpec* Sound = LunarNavigableWidget_Private::ResolveSound(
		SoundOverrides.PointerHovered,
		Settings->Navigation.Audio.DefaultPointerHoveredSound,
		SoundFeedbackAsset ? &SoundFeedbackAsset->Sounds.PointerHovered : nullptr))
	{
		NavigationSubsystem->PlayUISound(*Sound);
	}
}
