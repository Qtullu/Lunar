// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarNavigationSubsystem.cpp
 * @brief Implements per-local-player scope, graph, input, feedback, prompt, and diagnostic authority.
 * @ingroup LunarNavigationCore
 */

#include "UI/Navigation/Core/LunarNavigationSubsystem.h"

#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetNavigation.h"
#include "Brushes/SlateColorBrush.h"
#include "Components/AudioComponent.h"
#include "Components/InputComponent.h"
#include "Components/PanelWidget.h"
#include "Components/ScrollBox.h"
#include "Components/Widget.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "GameFramework/ForceFeedbackParameters.h"
#include "GameFramework/InputDeviceSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "GenericPlatform/InputDeviceRegistry.h"
#include "Input/Events.h"
#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "Kismet/GameplayStatics.h"
#include "Settings/LunarSettings.h"
#include "Subsystems/Console/LunarConsoleSubsystem.h"
#include "UI/Navigation/Controls/LunarRadio.h"
#include "UI/Navigation/Controls/LunarScrollBox.h"
#include "UI/Navigation/Core/LunarNavigableWidget.h"
#include "UI/Navigation/Core/LunarNavigationScope.h"
#include "UI/Navigation/Core/LunarScreenWidget.h"
#include "UI/Navigation/Data/LunarInputIconSet.h"
#include "UI/Navigation/Data/LunarUIActionRegistry.h"
#include "UI/Navigation/Data/LunarUIHapticFeedbackAsset.h"
#include "UI/Navigation/Data/LunarUISoundFeedbackAsset.h"
#include "UI/Navigation/Prompts/LunarInputPromptWidget.h"
#include "UI/Navigation/Types/LunarGameplayTags.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Input/SEditableText.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Text/SMultiLineEditableText.h"

DEFINE_LOG_CATEGORY_STATIC(LogLunarNavigation, Log, All);

#define LOCTEXT_NAMESPACE "LunarNavigationSubsystem"

/** @brief Private implementation helpers and shared viewport state for Lunar navigation. */
namespace LunarNavigationSubsystem_Private
{
	/** Maximum frames spent waiting for valid selection-scroll geometry. */
	constexpr uint8 MaxSelectionScrollGeometryRetries = 8;

	/** Floating-point tolerance used when comparing geometry scores. */
	constexpr float ScoreTieTolerance = KINDA_SMALL_NUMBER;
	/** Force-feedback tag used to isolate UI haptics from gameplay effects. */
	const FName UIHapticPlaybackTag(TEXT("Lunar.UI.Haptic"));
	/** Viewport Z-order reserved for the navigation debug overlay. */
	const int32 DebugOverlayZOrder = 100000;

	/** @brief Gamepad glyph family inferred from platform hardware metadata. */
	enum class EPromptGamepadFamily : uint8
	{
		Unknown, ///< Hardware could not be classified.
		Xbox, ///< Xbox or XInput-compatible glyph family.
		PlayStation ///< DualShock, DualSense, or PS5 glyph family.
	};

	/** @param Key Physical binding key. @param InputDevice Device family to test. @return True when the binding belongs to the family. */
	bool IsBindingForInputDevice(const FKey& Key, const ELunarInputDeviceType InputDevice)
	{
		switch (InputDevice)
		{
		case ELunarInputDeviceType::KeyboardMouse:
			return Key.IsValid() && !Key.IsGamepadKey() && !Key.IsTouch();
		case ELunarInputDeviceType::Gamepad:
			return Key.IsValid() && Key.IsGamepadKey();
		default:
			return false;
		}
	}

	/** @param Key Candidate gamepad key. @return True when the key mirrors one cardinal direction of the raw left-stick axes. */
	bool IsLeftStickDirectionKey(const FKey& Key)
	{
		return Key == EKeys::Gamepad_LeftStick_Up
			|| Key == EKeys::Gamepad_LeftStick_Down
			|| Key == EKeys::Gamepad_LeftStick_Left
			|| Key == EKeys::Gamepad_LeftStick_Right;
	}

	/** @param Key Candidate key. @return True for Shift or the left gamepad shoulder. */
	bool IsRangeSelectionModifierKey(const FKey& Key)
	{
		return Key == EKeys::LeftShift
			|| Key == EKeys::RightShift
			|| Key == EKeys::Gamepad_LeftShoulder;
	}

	/** @param Key Candidate key. @return True for Control or the right gamepad shoulder. */
	bool IsAdditiveSelectionModifierKey(const FKey& Key)
	{
		return Key == EKeys::LeftControl
			|| Key == EKeys::RightControl
			|| Key == EKeys::Gamepad_RightShoulder;
	}

	/**
	 * Synchronizes keyboard selection modifiers from the modifier snapshot carried by a Slate key event.
	 * Modifier-only key events are not guaranteed to reach every viewport input path, while the following
	 * letter or direction event still carries the authoritative Shift/Control state.
	 */
	void SynchronizeKeyboardSelectionModifiers(
		const FKeyEvent& KeyEvent,
		TSet<FKey>& HeldSelectionModifierKeys,
		const bool bTreatEventKeyAsPressed)
	{
		const FKey Key = KeyEvent.GetKey();
		if (Key.IsGamepadKey())
		{
			return;
		}

		HeldSelectionModifierKeys.Remove(EKeys::LeftShift);
		HeldSelectionModifierKeys.Remove(EKeys::RightShift);
		HeldSelectionModifierKeys.Remove(EKeys::LeftControl);
		HeldSelectionModifierKeys.Remove(EKeys::RightControl);

		const bool bShiftKeyEvent = Key == EKeys::LeftShift || Key == EKeys::RightShift;
		const bool bControlKeyEvent = Key == EKeys::LeftControl || Key == EKeys::RightControl;
		if (KeyEvent.IsShiftDown() || (bTreatEventKeyAsPressed && bShiftKeyEvent))
		{
			HeldSelectionModifierKeys.Add(EKeys::LeftShift);
		}
		if (KeyEvent.IsControlDown() || (bTreatEventKeyAsPressed && bControlKeyEvent))
		{
			HeldSelectionModifierKeys.Add(EKeys::LeftControl);
		}

		// Some Slate paths report the just-released modifier in the event snapshot. The key-up itself wins.
		if (!bTreatEventKeyAsPressed && bShiftKeyEvent)
		{
			HeldSelectionModifierKeys.Remove(EKeys::LeftShift);
		}
		if (!bTreatEventKeyAsPressed && bControlKeyEvent)
		{
			HeldSelectionModifierKeys.Remove(EKeys::LeftControl);
		}
	}

	/** @param FocusedWidget Focused Slate widget. @return True when the widget or one of its Slate ancestors is a supported editable-text control. */
	bool IsSlateEditableTextFocus(const TSharedPtr<SWidget>& FocusedWidget)
	{
		static const FName EditableTextType = SEditableText::StaticWidgetClass().GetWidgetType();
		static const FName EditableTextBoxType = SEditableTextBox::StaticWidgetClass().GetWidgetType();
		static const FName MultiLineEditableTextType = SMultiLineEditableText::StaticWidgetClass().GetWidgetType();
		static const FName MultiLineEditableTextBoxType = SMultiLineEditableTextBox::StaticWidgetClass().GetWidgetType();

		for (TSharedPtr<SWidget> Widget = FocusedWidget; Widget.IsValid(); Widget = Widget->GetParentWidget())
		{
			const FName WidgetType = Widget->GetWidgetClass().GetWidgetType();
			if (WidgetType == EditableTextType
				|| WidgetType == EditableTextBoxType
				|| WidgetType == MultiLineEditableTextType
				|| WidgetType == MultiLineEditableTextBoxType)
			{
				return true;
			}
		}
		return false;
	}

	/** @return Small fallback brush used when a prompt icon cannot be resolved. */
	FSlateBrush MakeMissingPromptIconBrush()
	{
		FSlateColorBrush Brush(FLinearColor::White);
		Brush.ImageSize = FVector2D(16.0f, 16.0f);
		return Brush;
	}

	/** @param InputDevice Device enum to format. @return Stable display name. */
	FString GetInputDeviceDisplayName(const ELunarInputDeviceType InputDevice)
	{
		if (const UEnum* Enum = StaticEnum<ELunarInputDeviceType>())
		{
			return Enum->GetNameStringByValue(static_cast<int64>(InputDevice));
		}
		return TEXT("Unknown");
	}

	/** @param HardwareClass Platform hardware class. @param HardwareIdentifier Device identifier. @return Inferred glyph family. */
	EPromptGamepadFamily ResolvePromptGamepadFamily(
		const FName HardwareClass,
		const FName HardwareIdentifier)
	{
#if defined(PLATFORM_PS5) && PLATFORM_PS5
		return EPromptGamepadFamily::PlayStation;
#else
		static const FName DualSenseName(TEXT("DualSense"));
		static const FName DualShock4Name(TEXT("DualShock4"));
		static const FName XboxOneName(TEXT("XboxOne"));
		static const FName Xbox360Name(TEXT("Xbox360"));
		static const FName XInputControllerName(TEXT("XInputController"));

		if (HardwareIdentifier == DualSenseName || HardwareIdentifier == DualShock4Name)
		{
			return EPromptGamepadFamily::PlayStation;
		}
		if (HardwareIdentifier == XboxOneName
			|| HardwareIdentifier == Xbox360Name
			|| HardwareIdentifier == XInputControllerName)
		{
			return EPromptGamepadFamily::Xbox;
		}

		(void)HardwareClass;
		return EPromptGamepadFamily::Unknown;
#endif
	}

	/** @brief Aggregates pointer-release requests from local players sharing one viewport. */
	struct FSharedViewportPointerState
	{
		/** Shared viewport whose policy is coordinated. */
		TWeakObjectPtr<UGameViewportClient> ViewportClient;
		/** Capture mode saved before the first release request became active. */
		EMouseCaptureMode PreReleaseCaptureMode = EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown;
		/** Lock mode saved before the first release request became active. */
		EMouseLockMode PreReleaseLockMode = EMouseLockMode::LockOnCapture;
		/** Per-local-player request to release pointer capture and lock. */
		TMap<TWeakObjectPtr<ULunarNavigationSubsystem>, bool> ReleaseRequests;
		/** Whether release policy is currently applied to the viewport. */
		bool bReleaseActive = false;
	};

	/** Shared viewport state indexed independently of local-player subsystem lifetime. */
	TMap<TObjectKey<UGameViewportClient>, FSharedViewportPointerState> SharedViewportPointerStates;

	/** @param State Shared viewport state whose dead owners should be removed. */
	void PruneInvalidPointerPolicyRequests(FSharedViewportPointerState& State)
	{
		for (auto Iterator = State.ReleaseRequests.CreateIterator(); Iterator; ++Iterator)
		{
			if (!Iterator.Key().IsValid())
			{
				Iterator.RemoveCurrent();
			}
		}
	}

	/** @param State Shared state whose aggregate release policy should be applied. */
	void ApplySharedViewportPointerPolicy(FSharedViewportPointerState& State)
	{
		UGameViewportClient* ViewportClient = State.ViewportClient.Get();
		if (!ViewportClient)
		{
			return;
		}

		PruneInvalidPointerPolicyRequests(State);
		bool bRelease = false;
		for (const TPair<TWeakObjectPtr<ULunarNavigationSubsystem>, bool>& Request : State.ReleaseRequests)
		{
			if (Request.Value)
			{
				bRelease = true;
				break;
			}
		}
		if (bRelease && !State.bReleaseActive)
		{
			State.PreReleaseCaptureMode = ViewportClient->GetMouseCaptureMode();
			State.PreReleaseLockMode = ViewportClient->GetMouseLockMode();
		}

		if (bRelease)
		{
			ViewportClient->SetMouseCaptureMode(EMouseCaptureMode::NoCapture);
			ViewportClient->SetMouseLockMode(EMouseLockMode::DoNotLock);
		}
		else if (State.bReleaseActive)
		{
			ViewportClient->SetMouseCaptureMode(State.PreReleaseCaptureMode);
			ViewportClient->SetMouseLockMode(State.PreReleaseLockMode);
		}
		State.bReleaseActive = bRelease;
	}

	/** @param Owner Local-player subsystem joining coordination. @param ViewportClient Shared viewport. */
	void RegisterSharedViewportPointerPolicy(ULunarNavigationSubsystem* Owner, UGameViewportClient* ViewportClient)
	{
		if (!Owner || !ViewportClient)
		{
			return;
		}

		const TObjectKey<UGameViewportClient> ViewportKey(ViewportClient);
		FSharedViewportPointerState* ExistingState = SharedViewportPointerStates.Find(ViewportKey);
		if (!ExistingState)
		{
			FSharedViewportPointerState& NewState = SharedViewportPointerStates.Add(ViewportKey);
			NewState.ViewportClient = ViewportClient;
			NewState.PreReleaseCaptureMode = ViewportClient->GetMouseCaptureMode();
			NewState.PreReleaseLockMode = ViewportClient->GetMouseLockMode();
			ExistingState = &NewState;
		}

		ExistingState->ReleaseRequests.FindOrAdd(Owner) = false;
	}

	/** @param Owner Requesting subsystem. @param ViewportClient Shared viewport. @param bRelease Whether this player requests release. */
	void UpdateSharedViewportPointerPolicy(
		ULunarNavigationSubsystem* Owner,
		UGameViewportClient* ViewportClient,
		const bool bRelease)
	{
		RegisterSharedViewportPointerPolicy(Owner, ViewportClient);
		if (FSharedViewportPointerState* State = SharedViewportPointerStates.Find(TObjectKey<UGameViewportClient>(ViewportClient)))
		{
			State->ReleaseRequests.FindOrAdd(Owner) = bRelease;
			ApplySharedViewportPointerPolicy(*State);
		}
	}

	/** @param Owner Subsystem leaving coordination. @param ViewportClient Shared viewport. */
	void UnregisterSharedViewportPointerPolicy(ULunarNavigationSubsystem* Owner, UGameViewportClient* ViewportClient)
	{
		if (!Owner || !ViewportClient)
		{
			return;
		}

		const TObjectKey<UGameViewportClient> ViewportKey(ViewportClient);
		FSharedViewportPointerState* State = SharedViewportPointerStates.Find(ViewportKey);
		if (!State)
		{
			return;
		}

		State->ReleaseRequests.Remove(Owner);
		PruneInvalidPointerPolicyRequests(*State);
		if (State->ReleaseRequests.IsEmpty())
		{
			if (State->bReleaseActive)
			{
				ViewportClient->SetMouseCaptureMode(State->PreReleaseCaptureMode);
				ViewportClient->SetMouseLockMode(State->PreReleaseLockMode);
			}
			SharedViewportPointerStates.Remove(ViewportKey);
			return;
		}

		ApplySharedViewportPointerPolicy(*State);
	}

	/** @param ActionTag Semantic action to inspect. @return True for cardinal navigation actions. */
	bool IsNavigationAction(const FGameplayTag& ActionTag)
	{
		using namespace LunarGameplayTags;
		return ActionTag == UI_Action_Navigate_Up.GetTag()
			|| ActionTag == UI_Action_Navigate_Down.GetTag()
			|| ActionTag == UI_Action_Navigate_Left.GetTag()
			|| ActionTag == UI_Action_Navigate_Right.GetTag();
	}

	/** @param ActionTag Semantic navigation action. @param OutDirection Receives its cardinal direction. @return True when conversion succeeds. */
	bool ActionToDirection(const FGameplayTag& ActionTag, ELunarNavigationDirection& OutDirection)
	{
		using namespace LunarGameplayTags;

		if (ActionTag == UI_Action_Navigate_Up.GetTag())
		{
			OutDirection = ELunarNavigationDirection::Up;
			return true;
		}
		if (ActionTag == UI_Action_Navigate_Down.GetTag())
		{
			OutDirection = ELunarNavigationDirection::Down;
			return true;
		}
		if (ActionTag == UI_Action_Navigate_Left.GetTag())
		{
			OutDirection = ELunarNavigationDirection::Left;
			return true;
		}
		if (ActionTag == UI_Action_Navigate_Right.GetTag())
		{
			OutDirection = ELunarNavigationDirection::Right;
			return true;
		}

		return false;
	}

	/** @param Widget Candidate descendant. @param Ancestor Candidate ancestor. @return True when logically nested, including widget-tree boundaries. */
	bool IsWidgetDescendantOf(const UWidget* Widget, const UWidget* Ancestor)
	{
		if (!Widget || !Ancestor || Widget == Ancestor)
		{
			return Widget == Ancestor;
		}

		for (const UObject* Outer = Widget; Outer; Outer = Outer->GetOuter())
		{
			if (Outer == Ancestor)
			{
				return true;
			}
		}

		auto ResolveLogicalParent = [](const UWidget* CurrentWidget) -> const UWidget*
		{
			if (!CurrentWidget)
			{
				return nullptr;
			}
			if (const UWidget* PanelParent = CurrentWidget->GetParent())
			{
				return PanelParent;
			}
			if (const UWidgetTree* WidgetTree = CurrentWidget->GetTypedOuter<UWidgetTree>())
			{
				return Cast<UWidget>(WidgetTree->GetOuter());
			}
			return nullptr;
		};

		const UWidget* Current = Widget;
		TSet<TObjectKey<UWidget>> Visited;
		while (Current && !Visited.Contains(TObjectKey<UWidget>(const_cast<UWidget*>(Current))))
		{
			Visited.Add(TObjectKey<UWidget>(const_cast<UWidget*>(Current)));
			Current = ResolveLogicalParent(Current);
			if (Current == Ancestor)
			{
				return true;
			}
		}

		return false;
	}

	/** @param Widget Widget whose panel and widget-tree parents are traversed. @return Logical hierarchy depth. */
	int32 GetLogicalWidgetDepth(const UWidget* Widget)
	{
		int32 Depth = 0;
		const UWidget* Current = Widget;
		TSet<TObjectKey<UWidget>> Visited;
		while (Current && !Visited.Contains(TObjectKey<UWidget>(const_cast<UWidget*>(Current))))
		{
			Visited.Add(TObjectKey<UWidget>(const_cast<UWidget*>(Current)));
			++Depth;
			if (const UWidget* PanelParent = Current->GetParent())
			{
				Current = PanelParent;
			}
			else if (const UWidgetTree* WidgetTree = Current->GetTypedOuter<UWidgetTree>())
			{
				Current = Cast<UWidget>(WidgetTree->GetOuter());
			}
			else
			{
				Current = nullptr;
			}
		}
		return Depth;
	}

	/** @param Direction Cardinal direction. @return Stable diagnostic name. */
	const TCHAR* GetDirectionName(const ELunarNavigationDirection Direction)
	{
		switch (Direction)
		{
		case ELunarNavigationDirection::Up:
			return TEXT("Up");
		case ELunarNavigationDirection::Down:
			return TEXT("Down");
		case ELunarNavigationDirection::Left:
			return TEXT("Left");
		case ELunarNavigationDirection::Right:
			return TEXT("Right");
		default:
			return TEXT("Unknown");
		}
	}

	/** @param Verbosity Console verbosity. @return True for error or fatal severity. */
	bool IsErrorVerbosity(const ELunarConsoleMessageVerbosity Verbosity)
	{
		return Verbosity == ELunarConsoleMessageVerbosity::Error
			|| Verbosity == ELunarConsoleMessageVerbosity::Fatal;
	}

	/** @param Verbosity Console verbosity. @return True for warning severity. */
	bool IsWarningVerbosity(const ELunarConsoleMessageVerbosity Verbosity)
	{
		return Verbosity == ELunarConsoleMessageVerbosity::Warning;
	}

	/** @param Widget Widget whose ancestors are searched. @return First unsupported native scroll-box ancestor, or nullptr. */
	const UScrollBox* FindUnsupportedNativeScrollBoxAncestor(const UWidget* Widget)
	{
		const UWidget* Current = Widget;
		TSet<TObjectKey<UWidget>> Visited;
		while (Current && !Visited.Contains(TObjectKey<UWidget>(const_cast<UWidget*>(Current))))
		{
			Visited.Add(TObjectKey<UWidget>(const_cast<UWidget*>(Current)));
			const UWidget* Parent = Current->GetParent();
			if (!Parent)
			{
				if (const UWidgetTree* WidgetTree = Current->GetTypedOuter<UWidgetTree>())
				{
					Parent = Cast<UWidget>(WidgetTree->GetOuter());
				}
			}
			if (const UScrollBox* ScrollBox = Cast<UScrollBox>(Parent))
			{
				if (!ScrollBox->IsA<ULunarScrollBox>())
				{
					return ScrollBox;
				}
			}
			Current = Parent;
		}
		return nullptr;
	}

	/** @param IconSet Icon set to inspect. @param Key Physical key to count. @return Number of matching entries. */
	int32 CountIconEntriesForKey(const ULunarInputIconSet* IconSet, const FKey& Key)
	{
		if (!IconSet || !Key.IsValid())
		{
			return 0;
		}

		int32 Count = 0;
		for (const FLunarInputIconEntry& Entry : IconSet->IconEntries)
		{
			if (Entry.InputKey == Key)
			{
				++Count;
			}
		}
		return Count;
	}
}

void ULunarNavigationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	bDebugOverlayEnabled = Settings && Settings->Navigation.Diagnostics.bEnableDebugOverlayByDefault;
	RebuildActionDefinitions();
	if (const FCulturePtr CurrentCulture = FInternationalization::Get().GetCurrentCulture())
	{
		LastObservedCultureName = CurrentCulture->GetName();
	}
	PromptConfigurationChangedHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddUObject(
		this,
		&ULunarNavigationSubsystem::HandlePromptConfigurationObjectChanged);
	if (UInputDeviceSubsystem* InputDeviceSubsystem = GEngine
		? GEngine->GetEngineSubsystem<UInputDeviceSubsystem>()
		: nullptr)
	{
		InputHardwareDeviceChangedHandle = InputDeviceSubsystem->OnInputHardwareDeviceChangedNative.AddUObject(
			this,
			&ULunarNavigationSubsystem::HandleInputHardwareDeviceChanged);
	}
	bInitialized = true;
	UpdateNavigationDebugOverlay();
}

void ULunarNavigationSubsystem::Deinitialize()
{
	bInitialized = false;
	RemoveNavigationDebugOverlay();
	if (PromptConfigurationChangedHandle.IsValid())
	{
		FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(PromptConfigurationChangedHandle);
		PromptConfigurationChangedHandle.Reset();
	}
	if (InputHardwareDeviceChangedHandle.IsValid())
	{
		if (UInputDeviceSubsystem* InputDeviceSubsystem = GEngine
			? GEngine->GetEngineSubsystem<UInputDeviceSubsystem>()
			: nullptr)
		{
			InputDeviceSubsystem->OnInputHardwareDeviceChangedNative.Remove(InputHardwareDeviceChangedHandle);
		}
		InputHardwareDeviceChangedHandle.Reset();
	}
	ResetSelectionScrollChain(true);
	if (DelegatedFocusOwner)
	{
		CancelNativeFocusDelegation(DelegatedFocusOwner);
	}
	CancelAllRegisteredScrolls();
	StopUIHaptic();
	StopAllUISounds();
	RemoveGameplayInputBlocker();
	RestorePreNavigationPointerState();
	ResetRepeatState();
	ConsumedKeyUps.Reset();
	HeldDigitalKeys.Reset();
	HeldSelectionModifierKeys.Reset();
	PressedActions.Reset();
	InputPresentationChangedNative.Clear();

	DelegatedFocusOwner = nullptr;
	DelegatedFocusWidget = nullptr;
	ClearSelection();

	for (int32 Index = ScopeStack.Num() - 1; Index >= 0; --Index)
	{
		if (ULunarNavigationScope* Scope = ScopeStack[Index])
		{
			Scope->DeactivateScope();
		}
	}

	ScopeStack.Reset();
	PendingNavigableWidgets.Reset();
	PendingValidationScopes.Reset();
	RegisteredScrollBoxes.Reset();
	RegistrationOrders.Reset();
	LastKnownWidgetEligibility.Reset();
	LastSelectionWidgets.Reset();
	ResolvedActionDefinitions.Reset();
	ActionDefinitionValidationMessages.Reset();
	ReportedValidationKeys.Reset();
	ReportedScopeValidationKeys.Reset();
	ReportedPromptErrorKeys.Reset();
	LastInputDeviceId = INPUTDEVICEID_NONE;
	LastInputHardwareClass = NAME_None;
	LastInputHardwareIdentifier = NAME_None;
	LastObservedCultureName.Reset();
	Super::Deinitialize();
}

void ULunarNavigationSubsystem::Tick(float DeltaTime)
{
	if (!bInitialized)
	{
		return;
	}
	const FCulturePtr CurrentCulture = FInternationalization::Get().GetCurrentCulture();
	const FString CurrentCultureName = CurrentCulture ? CurrentCulture->GetName() : FString();
	if (CurrentCultureName != LastObservedCultureName)
	{
		LastObservedCultureName = CurrentCultureName;
		InvalidateInputPromptPresentation(false);
	}
	PruneFinishedUISounds();
	UpdateGameplayInputBlocking();
	UpdateNavigationDebugOverlay();
	PollNavigationEligibilityChanges();
	if (bSelectionScrollAdvancePending && GFrameCounter >= SelectionScrollResumeFrame)
	{
		bSelectionScrollAdvancePending = false;
		AdvanceSelectionScrollChain();
	}

	PendingNavigableWidgets.RemoveAll([](const TWeakObjectPtr<ULunarNavigableWidget>& Widget)
	{
		return !Widget.IsValid();
	});
	if (!PendingValidationScopes.IsEmpty())
	{
		TArray<TWeakObjectPtr<ULunarNavigationScope>> ScopesToValidate;
		for (auto Iterator = PendingValidationScopes.CreateIterator(); Iterator; ++Iterator)
		{
			ULunarNavigationScope* Scope = Iterator.Key().Get();
			if (!Scope || !ScopeStack.Contains(Scope))
			{
				Iterator.RemoveCurrent();
				continue;
			}
			if (GFrameCounter >= Iterator.Value())
			{
				ScopesToValidate.Add(Scope);
				Iterator.RemoveCurrent();
			}
		}
		for (const TWeakObjectPtr<ULunarNavigationScope>& PendingScope : ScopesToValidate)
		{
			if (ULunarNavigationScope* Scope = PendingScope.Get())
			{
				ValidateNavigationScope(Scope);
			}
		}
	}

	if (ULunarNavigationScope* ActiveScope = GetActiveNavigationScope())
	{
		if (!IsWidgetEligibleInScope(SelectedWidget, ActiveScope))
		{
			if (DelegatedFocusOwner)
			{
				CancelNativeFocusDelegation(DelegatedFocusOwner);
			}

			const ULunarSettings* Settings = GetDefault<ULunarSettings>();
			if (!Settings || Settings->Navigation.Behavior.bRecoverSelectionAutomatically)
			{
				RecoverSelection(true);
			}
			else if (SelectedWidget)
			{
				ClearSelection();
			}
		}

		// Pointer and touch interactions may temporarily focus an intentional non-Lunar overlay.
		// Reclaim focus only after navigation presentation resumes, unless a delegated editor owns it.
		if (!bPointerPresentationActive || IsNativeFocusDelegationActive())
		{
			SynchronizeNativeFocus();
		}
		TickNavigationRepeat(FPlatformTime::Seconds());
	}
	else
	{
		ResetRepeatState();
	}
}

TStatId ULunarNavigationSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULunarNavigationSubsystem, STATGROUP_Tickables);
}

bool ULunarNavigationSubsystem::IsTickable() const
{
	return bInitialized && !HasAnyFlags(RF_ClassDefaultObject | RF_BeginDestroyed | RF_FinishDestroyed);
}

bool ULunarNavigationSubsystem::PushNavigationScope(ULunarNavigationScope* Scope)
{
	if (bScopeStackMutationInProgress || !IsValid(Scope) || ScopeStack.Contains(Scope))
	{
		return false;
	}
	if (Scope->GetOwningLocalPlayer() && Scope->GetOwningLocalPlayer() != GetLocalPlayer())
	{
		return false;
	}
	if (const UUserWidget* RootUserWidget = Cast<UUserWidget>(Scope->RootWidget))
	{
		if (RootUserWidget->GetOwningLocalPlayer() && RootUserWidget->GetOwningLocalPlayer() != GetLocalPlayer())
		{
			return false;
		}
	}
	bScopeStackMutationInProgress = true;

	ULunarNavigationScope* PreviousScope = GetActiveNavigationScope();
	if (!PreviousScope)
	{
		CapturePreNavigationPointerState();
	}

	if (DelegatedFocusOwner)
	{
		CommitNativeFocusDelegation(DelegatedFocusOwner);
	}

	if (PreviousScope)
	{
		CancelScrollsForScope(PreviousScope);
		PreviousScope->DeactivateScope();
	}

	ClearSelection();
	Scope->InitializeScope(GetLocalPlayer(), Scope->RootWidget, Scope->Settings, PreviousScope);
	ScopeStack.Add(Scope);
	Scope->ActivateScope();
	UpdateGameplayInputBlocking();
	ApplyActiveScopePointerPolicy(true);

	bSuppressGraphSelectionRecovery = true;
	RefreshNavigationGraph(Scope);
	bSuppressGraphSelectionRecovery = false;
	ULunarNavigableWidget* InitialSelection = ResolveInitialOrRestoredSelection(Scope, Scope->Settings.bRestoreLastSelection);
	if (InitialSelection)
	{
		SetSelectedWidget(InitialSelection);
	}
	else
	{
		ClearNativeUserFocus();
	}

	bScopeStackMutationInProgress = false;
	OnActiveScopeChanged.Broadcast(PreviousScope, Scope);
	return true;
}

bool ULunarNavigationSubsystem::PopNavigationScope(ULunarNavigationScope* Scope)
{
	if (bScopeStackMutationInProgress)
	{
		return false;
	}

	ULunarNavigationScope* PreviousScope = GetActiveNavigationScope();
	if (!PreviousScope)
	{
		return false;
	}

	const int32 TargetIndex = Scope ? ScopeStack.IndexOfByKey(Scope) : ScopeStack.Num() - 1;
	if (TargetIndex == INDEX_NONE)
	{
		return false;
	}
	bScopeStackMutationInProgress = true;

	if (DelegatedFocusOwner)
	{
		CancelNativeFocusDelegation(DelegatedFocusOwner);
	}

	ClearSelection();
	TArray<TWeakObjectPtr<ULunarScreenWidget>> RemovedScreens;
	for (int32 Index = ScopeStack.Num() - 1; Index >= TargetIndex; --Index)
	{
		if (ULunarNavigationScope* RemovedScope = ScopeStack[Index])
		{
			CancelScrollsForScope(RemovedScope);
			RemovedScope->DeactivateScope();
			if (ULunarScreenWidget* Screen = Cast<ULunarScreenWidget>(RemovedScope->RootWidget))
			{
				RemovedScreens.Add(Screen);
			}
		}
	}
	ScopeStack.RemoveAt(TargetIndex, ScopeStack.Num() - TargetIndex, EAllowShrinking::No);
	UpdateGameplayInputBlocking();

	ULunarNavigationScope* NewScope = GetActiveNavigationScope();
	if (NewScope)
	{
		NewScope->ActivateScope();
		ApplyActiveScopePointerPolicy(true);
		bSuppressGraphSelectionRecovery = true;
		RefreshNavigationGraph(NewScope);
		bSuppressGraphSelectionRecovery = false;

		if (ULunarNavigableWidget* RestoredSelection = ResolveInitialOrRestoredSelection(NewScope, true))
		{
			SetSelectedWidget(RestoredSelection);
		}
		else
		{
			ClearNativeUserFocus();
		}
	}
	else
	{
		StopUIHaptic();
		RestorePreNavigationPointerState();
		ResetRepeatState();
		ClearNativeUserFocus();
	}

	bScopeStackMutationInProgress = false;
	OnActiveScopeChanged.Broadcast(PreviousScope, NewScope);
	for (const TWeakObjectPtr<ULunarScreenWidget>& RemovedScreen : RemovedScreens)
	{
		if (ULunarScreenWidget* Screen = RemovedScreen.Get())
		{
			Screen->HandleNavigationScopePopped();
		}
	}
	return true;
}

ULunarNavigationScope* ULunarNavigationSubsystem::GetActiveNavigationScope() const
{
	return ScopeStack.IsEmpty() ? nullptr : ScopeStack.Last().Get();
}

TArray<ULunarNavigationScope*> ULunarNavigationSubsystem::GetNavigationScopeStack() const
{
	TArray<ULunarNavigationScope*> Result;
	Result.Reserve(ScopeStack.Num());
	for (ULunarNavigationScope* Scope : ScopeStack)
	{
		Result.Add(Scope);
	}
	return Result;
}

bool ULunarNavigationSubsystem::SetSelectedWidget(ULunarNavigableWidget* Widget)
{
	ULunarNavigationScope* ActiveScope = GetActiveNavigationScope();
	if (!IsWidgetEligibleInScope(Widget, ActiveScope))
	{
		return false;
	}

	if (!ActiveScrollNavigationConfinement.IsValid()
		|| !ActiveScrollNavigationConfinement->bConstrainNavigation)
	{
		ActiveScrollNavigationConfinement.Reset();
		ScrollNavigationReturnWidget.Reset();
	}
	if (!bReleasingScrollNavigationConfinement
		&& !IsWidgetAllowedByScrollConfinement(Widget))
	{
		return false;
	}

	if (!ActiveScrollNavigationConfinement.IsValid())
	{
		if (ULunarScrollBox* ConfiningScrollBox = FindDeepestContainingScrollBox(
			Widget,
			true,
			false,
			ELunarNavigationDirection::Down))
		{
			const bool bSelectionAlreadyInside = SelectedWidget
				&& LunarNavigationSubsystem_Private::IsWidgetDescendantOf(SelectedWidget, ConfiningScrollBox);
			if (!bSelectionAlreadyInside)
			{
				ActiveScrollNavigationConfinement = ConfiningScrollBox;
				ScrollNavigationReturnWidget = SelectedWidget;
			}
		}
	}

	if (SelectedWidget == Widget)
	{
		SynchronizeNativeFocus();
		ScrollSelectionIntoView();
		return true;
	}

	if (DelegatedFocusOwner && DelegatedFocusOwner != Widget)
	{
		CommitNativeFocusDelegation(DelegatedFocusOwner);
	}

	ULunarNavigableWidget* PreviousSelection = SelectedWidget;
	PressedActions.Reset();

	if (PreviousSelection)
	{
		PreviousSelection->SetLunarSelectedFromSubsystem(false);
	}

	SelectedWidget = Widget;
	SelectedWidget->SetLunarSelectedFromSubsystem(true);

	ActiveScope->SetLastSelectedWidget(Widget);
	ActiveScope->LastSelectionId = Widget->GetNavigationId();
	LastSelectionWidgets.FindOrAdd(TObjectKey<ULunarNavigationScope>(ActiveScope)) = Widget;
	SynchronizeNativeFocus();
	ScrollSelectionIntoView();
	if (SelectedWidget == Widget)
	{
		OnSelectionChanged.Broadcast(PreviousSelection, Widget);
	}
	return true;
}

bool ULunarNavigationSubsystem::SetSelectedWidgetFromPointer(ULunarNavigableWidget* Widget)
{
	if (!IsWidgetEligibleInScope(Widget, GetActiveNavigationScope()))
	{
		return false;
	}

	if (!IsWidgetAllowedByScrollConfinement(Widget))
	{
		ReleaseScrollNavigationConfinement(false);
	}
	return SetSelectedWidget(Widget);
}

bool ULunarNavigationSubsystem::SetSelectedWidgetById(const FName NavigationId)
{
	if (NavigationId.IsNone())
	{
		return false;
	}

	return SetSelectedWidget(FindWidgetById(GetActiveNavigationScope(), NavigationId));
}

ULunarNavigableWidget* ULunarNavigationSubsystem::GetSelectedWidget() const
{
	return SelectedWidget;
}

void ULunarNavigationSubsystem::ClearSelection()
{
	ResetSelectionScrollChain(true);
	ActiveScrollNavigationConfinement.Reset();
	ScrollNavigationReturnWidget.Reset();
	if (DelegatedFocusOwner)
	{
		CommitNativeFocusDelegation(DelegatedFocusOwner);
	}

	if (!SelectedWidget)
	{
		ClearNativeUserFocus();
		return;
	}

	ULunarNavigableWidget* PreviousSelection = SelectedWidget;
	PressedActions.Reset();
	SelectedWidget = nullptr;
	ClearNativeUserFocus();
	PreviousSelection->SetLunarSelectedFromSubsystem(false);
	OnSelectionChanged.Broadcast(PreviousSelection, nullptr);
}

bool ULunarNavigationSubsystem::ResetSelection()
{
	return ResetSelectionForScope(GetActiveNavigationScope());
}

bool ULunarNavigationSubsystem::ResetSelectionForScope(ULunarNavigationScope* Scope)
{
	if (!IsValid(Scope) || !ScopeStack.Contains(Scope))
	{
		return false;
	}

	Scope->SetLastSelectedWidget(nullptr);
	Scope->LastSelectionId = NAME_None;
	LastSelectionWidgets.Remove(TObjectKey<ULunarNavigationScope>(Scope));

	if (Scope != GetActiveNavigationScope())
	{
		return true;
	}

	ClearSelection();
	if (ULunarNavigableWidget* InitialSelection = ResolveInitialOrRestoredSelection(Scope, false))
	{
		return SetSelectedWidget(InitialSelection);
	}

	return false;
}

void ULunarNavigationSubsystem::ResetAllSelections()
{
	for (ULunarNavigationScope* Scope : ScopeStack)
	{
		if (Scope)
		{
			Scope->SetLastSelectedWidget(nullptr);
			Scope->LastSelectionId = NAME_None;
		}
	}

	LastSelectionWidgets.Reset();
	ResetSelection();
}

bool ULunarNavigationSubsystem::RegisterNavigableWidget(ULunarNavigableWidget* Widget)
{
	if (!IsValid(Widget) || Widget->GetOwningLocalPlayer() != GetLocalPlayer())
	{
		return false;
	}

	AssignRegistrationOrder(Widget);
	PendingNavigableWidgets.AddUnique(Widget);

	if (ULunarNavigationScope* Scope = ResolveOwningScopeForWidget(Widget))
	{
		for (ULunarNavigationScope* OtherScope : ScopeStack)
		{
			if (OtherScope && OtherScope != Scope)
			{
				OtherScope->RemoveRegisteredWidget(Widget);
			}
		}

		Scope->AddRegisteredWidget(Widget);
		PendingNavigableWidgets.Remove(Widget);
		ScheduleNavigationScopeValidation(Scope);

		if (Scope == GetActiveNavigationScope() && !SelectedWidget)
		{
			const ULunarSettings* Settings = GetDefault<ULunarSettings>();
			if (!Settings || Settings->Navigation.Behavior.bRecoverSelectionAutomatically)
			{
				RecoverSelection(false);
			}
		}
	}

	return true;
}

void ULunarNavigationSubsystem::UnregisterNavigableWidget(ULunarNavigableWidget* Widget)
{
	if (!Widget)
	{
		return;
	}

	const bool bWasSelected = SelectedWidget == Widget;
	const FVector2D PreviousCenter = GetWidgetCenter(Widget);

	PendingNavigableWidgets.Remove(Widget);
	RegistrationOrders.Remove(TObjectKey<ULunarNavigableWidget>(Widget));
	LastKnownWidgetEligibility.Remove(TObjectKey<ULunarNavigableWidget>(Widget));
	for (ULunarNavigationScope* Scope : ScopeStack)
	{
		if (Scope)
		{
			const bool bContainedWidget = Scope->RegisteredWidgets.Contains(Widget);
			Scope->RemoveRegisteredWidget(Widget);
			if (bContainedWidget)
			{
				ScheduleNavigationScopeValidation(Scope);
			}
		}
	}

	for (auto Iterator = LastSelectionWidgets.CreateIterator(); Iterator; ++Iterator)
	{
		if (Iterator.Value().Get() == Widget)
		{
			Iterator.RemoveCurrent();
		}
	}

	if (DelegatedFocusOwner == Widget)
	{
		CancelNativeFocusDelegation(Widget);
	}

	if (bWasSelected)
	{
		ClearSelection();
		const ULunarSettings* Settings = GetDefault<ULunarSettings>();
		if ((!Settings || Settings->Navigation.Behavior.bRecoverSelectionAutomatically)
			&& GetActiveNavigationScope())
		{
			if (ULunarNavigableWidget* Replacement = FindNearestEligibleWidget(GetActiveNavigationScope(), PreviousCenter))
			{
				SetSelectedWidget(Replacement);
			}
		}
	}
}

bool ULunarNavigationSubsystem::RegisterScrollBox(ULunarScrollBox* ScrollBox)
{
	if (!IsValid(ScrollBox) || ScrollBox->GetOwningLocalPlayer() != GetLocalPlayer())
	{
		return false;
	}

	const bool bRegistered = RegisteredScrollBoxes.Add(TWeakObjectPtr<ULunarScrollBox>(ScrollBox)).IsValidId();
	for (ULunarNavigationScope* Scope : ScopeStack)
	{
		if (IsValid(Scope)
			&& LunarNavigationSubsystem_Private::IsWidgetDescendantOf(ScrollBox, Scope->RootWidget))
		{
			ScheduleNavigationScopeValidation(Scope);
		}
	}
	return bRegistered;
}

void ULunarNavigationSubsystem::UnregisterScrollBox(ULunarScrollBox* ScrollBox)
{
	if (ActiveScrollNavigationConfinement.Get() == ScrollBox)
	{
		ReleaseScrollNavigationConfinement(true);
	}
	for (ULunarNavigationScope* Scope : ScopeStack)
	{
		if (IsValid(Scope)
			&& LunarNavigationSubsystem_Private::IsWidgetDescendantOf(ScrollBox, Scope->RootWidget))
		{
			ScheduleNavigationScopeValidation(Scope);
		}
	}
	RegisteredScrollBoxes.Remove(TWeakObjectPtr<ULunarScrollBox>(ScrollBox));
	const bool bWasActiveSelectionScroll = ActiveSelectionScrollBox.Get() == ScrollBox;
	if (bWasActiveSelectionScroll)
	{
		ResetSelectionScrollChain(false);
	}
	else
	{
		PendingSelectionScrollBoxes.Remove(TWeakObjectPtr<ULunarScrollBox>(ScrollBox));
		if (PendingSelectionScrollBoxes.IsEmpty() && !ActiveSelectionScrollBox.IsValid())
		{
			SelectionScrollTarget.Reset();
			bSelectionScrollAdvancePending = false;
			SelectionScrollResumeFrame = 0;
		}
	}
	if (IsValid(ScrollBox))
	{
		ScrollBox->CancelLunarScrollInternal(false, true);
	}
}

void ULunarNavigationSubsystem::ScrollSelectionIntoView()
{
	const uint64 RequestGeneration = ResetSelectionScrollChain(true);
	if (!bInitialized || RequestGeneration != SelectionScrollGeneration)
	{
		return;
	}
	if (!IsValid(SelectedWidget) || !SelectedWidget->bScrollIntoViewOnSelection)
	{
		return;
	}

	struct FScrollCandidate
	{
		ULunarScrollBox* ScrollBox = nullptr;
		int32 HierarchyDepth = 0;
		FString StablePath;
	};

	TArray<FScrollCandidate> Candidates;
	for (auto Iterator = RegisteredScrollBoxes.CreateIterator(); Iterator; ++Iterator)
	{
		ULunarScrollBox* ScrollBox = Iterator->Get();
		if (!IsValid(ScrollBox))
		{
			Iterator.RemoveCurrent();
			continue;
		}
		if (!LunarNavigationSubsystem_Private::IsWidgetDescendantOf(SelectedWidget, ScrollBox))
		{
			continue;
		}

		FScrollCandidate& Candidate = Candidates.AddDefaulted_GetRef();
		Candidate.ScrollBox = ScrollBox;
		Candidate.HierarchyDepth = LunarNavigationSubsystem_Private::GetLogicalWidgetDepth(ScrollBox);
		Candidate.StablePath = ScrollBox->GetPathName();
	}

	Candidates.Sort([](const FScrollCandidate& Left, const FScrollCandidate& Right)
	{
		if (Left.HierarchyDepth != Right.HierarchyDepth)
		{
			return Left.HierarchyDepth > Right.HierarchyDepth;
		}
		return Left.StablePath < Right.StablePath;
	});

	SelectionScrollTarget = SelectedWidget;
	PendingSelectionScrollBoxes.Reserve(Candidates.Num());
	for (const FScrollCandidate& Candidate : Candidates)
	{
		if (IsValid(Candidate.ScrollBox))
		{
			PendingSelectionScrollBoxes.Add(Candidate.ScrollBox);
		}
	}
	// Selection-state presentation may change desired geometry. Wait until the
	// next frame before calculating the reveal offset so wrap never uses stale bounds.
	bSelectionScrollAdvancePending = true;
	SelectionScrollResumeFrame = GFrameCounter + 1;
}

void ULunarNavigationSubsystem::AdvanceSelectionScrollChain()
{
	if (bAdvancingSelectionScrollChain
		|| bSelectionScrollAdvancePending
		|| !SelectionScrollTarget.IsValid()
		|| SelectionScrollTarget.Get() != SelectedWidget)
	{
		return;
	}

	const uint64 RequestGeneration = SelectionScrollGeneration;
	TGuardValue<bool> AdvancingGuard(bAdvancingSelectionScrollChain, true);
	while (!PendingSelectionScrollBoxes.IsEmpty())
	{
		if (RequestGeneration != SelectionScrollGeneration)
		{
			return;
		}

		ULunarScrollBox* ScrollBox = PendingSelectionScrollBoxes[0].Get();
		PendingSelectionScrollBoxes.RemoveAt(0, 1, EAllowShrinking::No);
		if (!IsValid(ScrollBox))
		{
			continue;
		}
		if (!SelectionScrollTarget.IsValid()
			|| SelectionScrollTarget.Get() != SelectedWidget
			|| !ScrollBox->IsValidLunarScrollTarget(SelectionScrollTarget.Get()))
		{
			ResetSelectionScrollChain(false);
			return;
		}

		float TargetOffset = 0.0f;
		if (!ScrollBox->CalculateMinimumScrollOffset(SelectionScrollTarget.Get(), TargetOffset))
		{
			// A scope can select its initial widget before Slate's first layout pass.
			// Retry briefly, then treat a persistently zero-size target as a no-op so
			// malformed content cannot keep the subsystem in an endless chain.
			if (++SelectionScrollGeometryRetryCount
				<= LunarNavigationSubsystem_Private::MaxSelectionScrollGeometryRetries)
			{
				PendingSelectionScrollBoxes.Insert(ScrollBox, 0);
				bSelectionScrollAdvancePending = true;
				SelectionScrollResumeFrame = GFrameCounter + 1;
				return;
			}
			SelectionScrollGeometryRetryCount = 0;
			continue;
		}
		SelectionScrollGeometryRetryCount = 0;
		if (ScrollBox->IsLunarScrollActive())
		{
			// Clear an unrelated programmatic animation before attaching this chain's
			// completion listener, otherwise its cancellation would be mistaken for
			// cancellation of the new selection-driven stage.
			ScrollBox->CancelLunarScrollInternal(false, true);
			if (RequestGeneration != SelectionScrollGeneration)
			{
				return;
			}
		}

		ActiveSelectionScrollBox = ScrollBox;
		ActiveSelectionScrollFinishedHandle = ScrollBox->OnLunarScrollFinishedNative().AddUObject(
			this,
			&ULunarNavigationSubsystem::HandleSelectionScrollFinishedNative);
		ScrollBox->ScrollWidgetIntoLunarView(SelectionScrollTarget.Get());

		if (RequestGeneration != SelectionScrollGeneration || bSelectionScrollAdvancePending)
		{
			return;
		}
		if (ScrollBox->IsLunarScrollActive())
		{
			return;
		}

		if (ActiveSelectionScrollBox.Get() == ScrollBox)
		{
			if (ActiveSelectionScrollFinishedHandle.IsValid())
			{
				ScrollBox->OnLunarScrollFinishedNative().Remove(ActiveSelectionScrollFinishedHandle);
				ActiveSelectionScrollFinishedHandle.Reset();
			}
			ActiveSelectionScrollBox.Reset();
		}
	}

	if (RequestGeneration == SelectionScrollGeneration)
	{
		SelectionScrollTarget.Reset();
	}
}

void ULunarNavigationSubsystem::HandleSelectionScrollFinishedNative(
	ULunarScrollBox* ScrollBox,
	const bool bCompleted)
{
	if (!IsValid(ScrollBox) || ActiveSelectionScrollBox.Get() != ScrollBox)
	{
		return;
	}
	if (ActiveSelectionScrollFinishedHandle.IsValid())
	{
		ScrollBox->OnLunarScrollFinishedNative().Remove(ActiveSelectionScrollFinishedHandle);
		ActiveSelectionScrollFinishedHandle.Reset();
	}
	ActiveSelectionScrollBox.Reset();
	SelectionScrollGeometryRetryCount = 0;

	if (!bCompleted)
	{
		ResetSelectionScrollChain(false);
		return;
	}

	if (SelectionScrollTarget.IsValid()
		&& SelectionScrollTarget.Get() == SelectedWidget
		&& !PendingSelectionScrollBoxes.IsEmpty())
	{
		// Wait for the next frame so Slate can arrange the inner container at its new offset.
		bSelectionScrollAdvancePending = true;
		SelectionScrollResumeFrame = GFrameCounter + 1;
	}
	else
	{
		PendingSelectionScrollBoxes.Reset();
		SelectionScrollTarget.Reset();
	}
}

uint64 ULunarNavigationSubsystem::ResetSelectionScrollChain(const bool bCancelActiveScroll)
{
	const uint64 ResetGeneration = ++SelectionScrollGeneration;
	ULunarScrollBox* ActiveScrollBox = ActiveSelectionScrollBox.Get();
	if (ActiveScrollBox && ActiveSelectionScrollFinishedHandle.IsValid())
	{
		ActiveScrollBox->OnLunarScrollFinishedNative().Remove(ActiveSelectionScrollFinishedHandle);
	}
	ActiveSelectionScrollFinishedHandle.Reset();
	ActiveSelectionScrollBox.Reset();
	PendingSelectionScrollBoxes.Reset();
	SelectionScrollTarget.Reset();
	bSelectionScrollAdvancePending = false;
	SelectionScrollGeometryRetryCount = 0;
	SelectionScrollResumeFrame = 0;

	if (bCancelActiveScroll && IsValid(ActiveScrollBox))
	{
		// Internal supersede/teardown must not synchronously invoke Blueprint and
		// create a replacement chain inside an outer selection mutation.
		ActiveScrollBox->CancelLunarScrollInternal(false, true);
	}
	return ResetGeneration;
}

void ULunarNavigationSubsystem::CancelSelectionScrollChainForDirectInput(ULunarScrollBox* ScrollBox)
{
	if (IsValid(ScrollBox)
		&& ScrollBox->GetOwningLocalPlayer() == GetLocalPlayer()
		&& (ActiveSelectionScrollBox.IsValid()
			|| !PendingSelectionScrollBoxes.IsEmpty()
			|| SelectionScrollTarget.IsValid()))
	{
		ResetSelectionScrollChain(true);
	}
}

void ULunarNavigationSubsystem::CancelScrollsForScope(const ULunarNavigationScope* Scope)
{
	const UWidget* ScopeRoot = Scope ? Scope->RootWidget.Get() : nullptr;
	if (!ScopeRoot)
	{
		return;
	}

	TArray<TWeakObjectPtr<ULunarScrollBox>> ScrollBoxesToCancel;
	for (auto Iterator = RegisteredScrollBoxes.CreateIterator(); Iterator; ++Iterator)
	{
		ULunarScrollBox* ScrollBox = Iterator->Get();
		if (!IsValid(ScrollBox))
		{
			Iterator.RemoveCurrent();
			continue;
		}

		const bool bScrollBoxInsideScope = LunarNavigationSubsystem_Private::IsWidgetDescendantOf(ScrollBox, ScopeRoot);
		const bool bScopeInsideScrollBox = LunarNavigationSubsystem_Private::IsWidgetDescendantOf(ScopeRoot, ScrollBox);
		if (bScrollBoxInsideScope || bScopeInsideScrollBox)
		{
			ScrollBoxesToCancel.Add(ScrollBox);
		}
	}

	for (const TWeakObjectPtr<ULunarScrollBox>& ScrollBox : ScrollBoxesToCancel)
	{
		if (ScrollBox.IsValid())
		{
			ScrollBox->CancelLunarScrollInternal(false, true);
		}
	}
}

void ULunarNavigationSubsystem::CancelAllRegisteredScrolls()
{
	TArray<TWeakObjectPtr<ULunarScrollBox>> ScrollBoxesToCancel;
	for (auto Iterator = RegisteredScrollBoxes.CreateIterator(); Iterator; ++Iterator)
	{
		if (ULunarScrollBox* ScrollBox = Iterator->Get(); IsValid(ScrollBox))
		{
			ScrollBoxesToCancel.Add(ScrollBox);
		}
		else
		{
			Iterator.RemoveCurrent();
		}
	}

	for (const TWeakObjectPtr<ULunarScrollBox>& ScrollBox : ScrollBoxesToCancel)
	{
		if (ScrollBox.IsValid())
		{
			ScrollBox->CancelLunarScrollInternal(false, true);
		}
	}
}

void ULunarNavigationSubsystem::PlayUISound(const FLunarUISoundSpec& SoundSpec)
{
	if (!SoundSpec.Sound || !GetWorld())
	{
		return;
	}

	PruneFinishedUISounds();
	if (UAudioComponent* AudioComponent = UGameplayStatics::SpawnSound2D(
		GetWorld(),
		SoundSpec.Sound,
		FMath::Max(0.0f, SoundSpec.VolumeMultiplier),
		FMath::Max(0.0f, SoundSpec.PitchMultiplier),
		0.0f,
		SoundSpec.Concurrency,
		false,
		true))
	{
		ActiveUISoundComponents.Add(AudioComponent);
	}
}

void ULunarNavigationSubsystem::PlayUIHaptic(const FLunarUIHapticSpec& HapticSpec)
{
	if (!HapticSpec.Effect || !GetLocalPlayer())
	{
		return;
	}

	APlayerController* PlayerController = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (!PlayerController)
	{
		return;
	}

	StopUIHaptic();
	FForceFeedbackParameters Parameters;
	Parameters.Tag = LunarNavigationSubsystem_Private::UIHapticPlaybackTag;
	Parameters.bLooping = false;
	Parameters.bIgnoreTimeDilation = true;
	Parameters.bPlayWhilePaused = true;
	PlayerController->ClientPlayForceFeedback(HapticSpec.Effect, Parameters);
	ActiveUIHapticEffect = HapticSpec.Effect;
	ActiveUIHapticPlayerController = PlayerController;
}

void ULunarNavigationSubsystem::StopUIHaptic()
{
	APlayerController* PlayerController = ActiveUIHapticPlayerController.Get();
	if (!PlayerController && GetLocalPlayer())
	{
		PlayerController = GetLocalPlayer()->GetPlayerController(GetWorld());
	}
	if (PlayerController)
	{
		PlayerController->ClientStopForceFeedback(nullptr, LunarNavigationSubsystem_Private::UIHapticPlaybackTag);
	}

	ActiveUIHapticEffect.Reset();
	ActiveUIHapticPlayerController.Reset();
}

void ULunarNavigationSubsystem::RefreshNavigationGraph(ULunarNavigationScope* Scope)
{
	Scope = Scope ? Scope : GetActiveNavigationScope();
	if (!IsValid(Scope) || !ScopeStack.Contains(Scope))
	{
		return;
	}

	TArray<ULunarNavigableWidget*> DiscoveredWidgets;
	GatherNavigableDescendants(Scope->RootWidget, DiscoveredWidgets);

	TArray<ULunarNavigableWidget*> DesiredWidgets;
	for (ULunarNavigableWidget* Widget : DiscoveredWidgets)
	{
		if (!IsValid(Widget) || Widget->GetOwningLocalPlayer() != GetLocalPlayer())
		{
			continue;
		}

		AssignRegistrationOrder(Widget);
		if (ResolveOwningScopeForWidget(Widget) == Scope)
		{
			DesiredWidgets.AddUnique(Widget);
		}
	}

	for (const TWeakObjectPtr<ULunarNavigableWidget>& PendingWidget : PendingNavigableWidgets)
	{
		ULunarNavigableWidget* Widget = PendingWidget.Get();
		if (Widget && ResolveOwningScopeForWidget(Widget) == Scope)
		{
			AssignRegistrationOrder(Widget);
			DesiredWidgets.AddUnique(Widget);
		}
	}

	DesiredWidgets.StableSort([this](const ULunarNavigableWidget& Left, const ULunarNavigableWidget& Right)
	{
		return GetRegistrationOrder(&Left) < GetRegistrationOrder(&Right);
	});

	const TArray<ULunarNavigableWidget*> ExistingWidgets = Scope->GetRegisteredWidgets();
	for (ULunarNavigableWidget* Widget : ExistingWidgets)
	{
		Scope->RemoveRegisteredWidget(Widget);
	}
	for (ULunarNavigableWidget* Widget : DesiredWidgets)
	{
		for (ULunarNavigationScope* OtherScope : ScopeStack)
		{
			if (OtherScope && OtherScope != Scope)
			{
				OtherScope->RemoveRegisteredWidget(Widget);
			}
		}
		Scope->AddRegisteredWidget(Widget);
		PendingNavigableWidgets.Remove(Widget);
	}

	ScheduleNavigationScopeValidation(Scope);

	if (Scope == GetActiveNavigationScope() && !bSuppressGraphSelectionRecovery)
	{
		const ULunarSettings* Settings = GetDefault<ULunarSettings>();
		if (!Settings || Settings->Navigation.Behavior.bRecoverSelectionAutomatically)
		{
			RecoverSelection(SelectedWidget != nullptr);
		}
		else if (SelectedWidget && !IsWidgetEligibleInScope(SelectedWidget, Scope))
		{
			ClearSelection();
		}
	}
}

ULunarNavigationScope* ULunarNavigationSubsystem::ResolveOwningScopeForWidget(const ULunarNavigableWidget* Widget) const
{
	if (!IsValid(Widget) || Widget->GetOwningLocalPlayer() != GetLocalPlayer())
	{
		return nullptr;
	}

	if (ULunarNavigationScope* OverrideScope = Widget->GetNavigationScopeOverride())
	{
		return ScopeStack.Contains(OverrideScope) && OverrideScope->GetOwningLocalPlayer() == GetLocalPlayer()
			? OverrideScope
			: nullptr;
	}

	for (int32 Index = ScopeStack.Num() - 1; Index >= 0; --Index)
	{
		ULunarNavigationScope* Scope = ScopeStack[Index];
		if (Scope && IsWidgetWithinScopeRoot(Widget, Scope))
		{
			return Scope;
		}
	}

	return nullptr;
}

bool ULunarNavigationSubsystem::ScopeContainsWidget(const ULunarNavigationScope* Scope, const ULunarNavigableWidget* Widget) const
{
	if (!Scope || !Widget)
	{
		return false;
	}

	return Scope->RegisteredWidgets.Contains(Widget);
}

bool ULunarNavigationSubsystem::IsWidgetWithinScopeRoot(const UWidget* Widget, const ULunarNavigationScope* Scope) const
{
	if (!Widget || !Scope || !Scope->RootWidget)
	{
		return false;
	}
	if (Widget == Scope->RootWidget)
	{
		return true;
	}

	TArray<ULunarNavigableWidget*> Descendants;
	GatherNavigableDescendants(Scope->RootWidget, Descendants);
	return Descendants.Contains(Widget);
}

bool ULunarNavigationSubsystem::IsWidgetEligibleInScope(const ULunarNavigableWidget* Widget, const ULunarNavigationScope* Scope) const
{
	return Scope
		&& Scope == GetActiveNavigationScope()
		&& Scope->IsActive()
		&& IsWidgetEligibleInGraphScope(Widget, Scope);
}

bool ULunarNavigationSubsystem::IsWidgetEligibleInGraphScope(
	const ULunarNavigableWidget* Widget,
	const ULunarNavigationScope* Scope) const
{
	return IsValid(Widget)
		&& Scope
		&& ScopeContainsWidget(Scope, Widget)
		&& Widget->CanReceiveLunarSelection();
}

bool ULunarNavigationSubsystem::IsWidgetEligibleForNavigationInput(
	const ULunarNavigableWidget* Widget,
	const ULunarNavigationScope* Scope,
	const ELunarInputDeviceType InputDevice) const
{
	return IsWidgetEligibleInGraphScope(Widget, Scope)
		&& (InputDevice == ELunarInputDeviceType::Unknown
			|| Widget->IsNavigationInputAllowed(InputDevice));
}

void ULunarNavigationSubsystem::GatherNavigableDescendants(UWidget* RootWidget, TArray<ULunarNavigableWidget*>& OutWidgets) const
{
	OutWidgets.Reset();
	if (!RootWidget)
	{
		return;
	}

	TArray<UWidget*> PendingWidgets;
	TSet<TObjectKey<UWidget>> VisitedWidgets;
	PendingWidgets.Add(RootWidget);

	while (!PendingWidgets.IsEmpty())
	{
		UWidget* Widget = PendingWidgets.Pop(EAllowShrinking::No);
		if (!IsValid(Widget) || VisitedWidgets.Contains(TObjectKey<UWidget>(Widget)))
		{
			continue;
		}
		VisitedWidgets.Add(TObjectKey<UWidget>(Widget));
		if (Widget != RootWidget && Widget->IsA<ULunarScreenWidget>())
		{
			continue;
		}

		if (ULunarNavigableWidget* NavigableWidget = Cast<ULunarNavigableWidget>(Widget))
		{
			OutWidgets.Add(NavigableWidget);
		}

		if (UUserWidget* UserWidget = Cast<UUserWidget>(Widget))
		{
			if (UserWidget->WidgetTree)
			{
				TArray<UWidget*> TreeWidgets;
				UserWidget->WidgetTree->GetAllWidgets(TreeWidgets);
				for (int32 Index = TreeWidgets.Num() - 1; Index >= 0; --Index)
				{
					PendingWidgets.Add(TreeWidgets[Index]);
				}
			}
		}

		if (UPanelWidget* PanelWidget = Cast<UPanelWidget>(Widget))
		{
			for (int32 ChildIndex = PanelWidget->GetChildrenCount() - 1; ChildIndex >= 0; --ChildIndex)
			{
				PendingWidgets.Add(PanelWidget->GetChildAt(ChildIndex));
			}
		}
	}
}

void ULunarNavigationSubsystem::AssignRegistrationOrder(ULunarNavigableWidget* Widget)
{
	if (Widget && !RegistrationOrders.Contains(TObjectKey<ULunarNavigableWidget>(Widget)))
	{
		RegistrationOrders.Add(TObjectKey<ULunarNavigableWidget>(Widget), NextRegistrationOrder++);
	}
}

uint64 ULunarNavigationSubsystem::GetRegistrationOrder(const ULunarNavigableWidget* Widget) const
{
	if (const uint64* Order = RegistrationOrders.Find(TObjectKey<ULunarNavigableWidget>(const_cast<ULunarNavigableWidget*>(Widget))))
	{
		return *Order;
	}
	return MAX_uint64;
}

ULunarNavigableWidget* ULunarNavigationSubsystem::ResolveInitialOrRestoredSelection(ULunarNavigationScope* Scope, const bool bAllowRestore) const
{
	if (!Scope)
	{
		return nullptr;
	}

	if (bAllowRestore)
	{
		if (const TWeakObjectPtr<ULunarNavigableWidget>* SavedWidget = LastSelectionWidgets.Find(TObjectKey<ULunarNavigationScope>(Scope)))
		{
			if (IsWidgetEligibleInScope(SavedWidget->Get(), Scope))
			{
				return SavedWidget->Get();
			}
		}

		if (IsWidgetEligibleInScope(Scope->GetLastSelectedWidget(), Scope))
		{
			return Scope->GetLastSelectedWidget();
		}

		if (!Scope->GetLastSelectionId().IsNone())
		{
			if (ULunarNavigableWidget* LastById = FindWidgetById(Scope, Scope->GetLastSelectionId()))
			{
				return LastById;
			}
		}
	}

	if (IsWidgetEligibleInScope(Scope->Settings.InitialSelectionWidget, Scope))
	{
		return Scope->Settings.InitialSelectionWidget;
	}
	if (!Scope->GetInitialSelectionId().IsNone())
	{
		if (ULunarNavigableWidget* InitialById = FindWidgetById(Scope, Scope->GetInitialSelectionId()))
		{
			return InitialById;
		}
	}

	return FindFirstEligibleWidget(Scope);
}

ULunarNavigableWidget* ULunarNavigationSubsystem::FindFirstEligibleWidget(ULunarNavigationScope* Scope) const
{
	if (!Scope)
	{
		return nullptr;
	}

	for (ULunarNavigableWidget* Widget : Scope->RegisteredWidgets)
	{
		if (IsWidgetEligibleInScope(Widget, Scope))
		{
			return Widget;
		}
	}
	return nullptr;
}

ULunarNavigableWidget* ULunarNavigationSubsystem::FindWidgetById(ULunarNavigationScope* Scope, const FName NavigationId) const
{
	if (!Scope || NavigationId.IsNone())
	{
		return nullptr;
	}

	for (ULunarNavigableWidget* Widget : Scope->RegisteredWidgets)
	{
		if (Widget && Widget->GetNavigationId() == NavigationId && IsWidgetEligibleInScope(Widget, Scope))
		{
			return Widget;
		}
	}
	return nullptr;
}

ULunarNavigableWidget* ULunarNavigationSubsystem::FindNearestEligibleWidget(ULunarNavigationScope* Scope, const FVector2D& Origin) const
{
	ULunarNavigableWidget* BestWidget = nullptr;
	double BestDistanceSquared = TNumericLimits<double>::Max();

	if (!Scope)
	{
		return nullptr;
	}

	for (ULunarNavigableWidget* Candidate : Scope->RegisteredWidgets)
	{
		if (!IsWidgetEligibleInScope(Candidate, Scope))
		{
			continue;
		}

		const double DistanceSquared = FVector2D::DistSquared(GetWidgetCenter(Candidate), Origin);
		if (!BestWidget
			|| DistanceSquared < BestDistanceSquared - LunarNavigationSubsystem_Private::ScoreTieTolerance
			|| (FMath::IsNearlyEqual(DistanceSquared, BestDistanceSquared)
				&& Candidate->GetNavigationPriority() > BestWidget->GetNavigationPriority()))
		{
			BestWidget = Candidate;
			BestDistanceSquared = DistanceSquared;
		}
	}

	return BestWidget;
}

bool ULunarNavigationSubsystem::RecoverSelection(const bool bUsePreviousGeometry)
{
	ULunarNavigationScope* Scope = GetActiveNavigationScope();
	if (!Scope)
	{
		return false;
	}

	if (IsWidgetEligibleInScope(SelectedWidget, Scope))
	{
		return true;
	}

	const bool bHadSelection = SelectedWidget != nullptr;
	const FVector2D PreviousCenter = bUsePreviousGeometry && bHadSelection
		? GetWidgetCenter(SelectedWidget)
		: FVector2D::ZeroVector;
	ClearSelection();

	ULunarNavigableWidget* Replacement = bUsePreviousGeometry && bHadSelection
		? FindNearestEligibleWidget(Scope, PreviousCenter)
		: ResolveInitialOrRestoredSelection(Scope, true);

	if (!Replacement && bUsePreviousGeometry && bHadSelection)
	{
		Replacement = ResolveInitialOrRestoredSelection(Scope, true);
	}

	return Replacement && SetSelectedWidget(Replacement);
}

bool ULunarNavigationSubsystem::Navigate(const ELunarNavigationDirection Direction)
{
	ULunarNavigationScope* Scope = GetActiveNavigationScope();
	if (!Scope)
	{
		return false;
	}

	if (!IsWidgetEligibleInScope(SelectedWidget, Scope) && !RecoverSelection(false))
	{
		return false;
	}

	bool bBlocked = false;
	ULunarNavigableWidget* Target = ResolveNavigationTarget(SelectedWidget, Direction, bBlocked);
	return !bBlocked && Target && SetSelectedWidget(Target);
}

ULunarNavigableWidget* ULunarNavigationSubsystem::ResolveNavigationTarget(
	ULunarNavigableWidget* Current,
	const ELunarNavigationDirection Direction,
	bool& bOutBlocked) const
{
	return ResolveNavigationTargetInScope(Current, Direction, GetActiveNavigationScope(), bOutBlocked);
}

ULunarNavigableWidget* ULunarNavigationSubsystem::ResolveNavigationTargetInScope(
	ULunarNavigableWidget* Current,
	const ELunarNavigationDirection Direction,
	const ULunarNavigationScope* Scope,
	bool& bOutBlocked) const
{
	bOutBlocked = false;
	if (!IsWidgetEligibleInGraphScope(Current, Scope))
	{
		return nullptr;
	}

	auto ResolveAllowedTarget = [&](ULunarNavigableWidget* Candidate) -> ULunarNavigableWidget*
	{
		if (!IsWidgetEligibleForNavigationInput(Candidate, Scope, NavigationDispatchInputDevice))
		{
			return nullptr;
		}
		if (!IsWidgetAllowedByScrollConfinement(Candidate))
		{
			bOutBlocked = true;
			return nullptr;
		}
		return Candidate;
	};

	const FLunarNavigationLink& Link = Current->GetNavigationLink(Direction);
	switch (Link.Mode)
	{
	case ELunarNavigationLinkMode::Block:
		bOutBlocked = true;
		return nullptr;

	case ELunarNavigationLinkMode::Widget:
		if (ULunarNavigableWidget* LinkedWidget = ResolveAllowedTarget(Link.Widget))
		{
			return LinkedWidget;
		}
		if (bOutBlocked)
		{
			return nullptr;
		}
		break;

	case ELunarNavigationLinkMode::NavigationId:
		for (ULunarNavigableWidget* LinkedById : Scope->RegisteredWidgets)
		{
			if (LinkedById && LinkedById->GetNavigationId() == Link.NavigationId)
			{
				if (ULunarNavigableWidget* AllowedWidget = ResolveAllowedTarget(LinkedById))
				{
					return AllowedWidget;
				}
				if (bOutBlocked)
				{
					return nullptr;
				}
			}
		}
		break;

	case ELunarNavigationLinkMode::Automatic:
	default:
		break;
	}

	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	const bool bUseGeometricFallback = !Settings || Settings->Navigation.Behavior.bEnableGeometricFallback;
	ULunarScrollBox* ActiveConfinement = ActiveScrollNavigationConfinement.Get();
	ULunarScrollBox* AxisScrollBox = FindDeepestContainingScrollBox(Current, false, true, Direction);
	if (ActiveConfinement
		&& AxisScrollBox
		&& !LunarNavigationSubsystem_Private::IsWidgetDescendantOf(AxisScrollBox, ActiveConfinement))
	{
		AxisScrollBox = nullptr;
	}
	if (bUseGeometricFallback && AxisScrollBox)
	{
		if (ULunarNavigableWidget* LocalTarget = FindGeometricTargetInScope(
			Current,
			Direction,
			false,
			Scope,
			AxisScrollBox))
		{
			return LocalTarget;
		}
		if (AxisScrollBox->bWrapNavigation)
		{
			return FindGeometricTargetInScope(Current, Direction, true, Scope, AxisScrollBox);
		}
	}

	if (ActiveConfinement
		&& ActiveConfinement->bConstrainNavigation
		&& LunarNavigationSubsystem_Private::IsWidgetDescendantOf(Current, ActiveConfinement))
	{
		if (bUseGeometricFallback)
		{
			if (ULunarNavigableWidget* ConstrainedTarget = FindGeometricTargetInScope(
				Current,
				Direction,
				false,
				Scope,
				ActiveConfinement))
			{
				return ConstrainedTarget;
			}
		}
		bOutBlocked = true;
		return nullptr;
	}

	if (bUseGeometricFallback)
	{
		if (ULunarNavigableWidget* GeometricTarget = FindGeometricTargetInScope(Current, Direction, false, Scope))
		{
			return GeometricTarget;
		}
	}

	// A ScrollBox owns wrap along its primary axis. When local wrap is disabled,
	// navigation may leave toward a real forward target but never inherits screen wrap.
	if (AxisScrollBox)
	{
		return nullptr;
	}

	const bool bWrap = IsHorizontalDirection(Direction)
		? Scope->Settings.bWrapHorizontal
		: Scope->Settings.bWrapVertical;
	return bWrap ? FindGeometricTargetInScope(Current, Direction, true, Scope) : nullptr;
}

ULunarNavigableWidget* ULunarNavigationSubsystem::FindGeometricTarget(
	ULunarNavigableWidget* Current,
	const ELunarNavigationDirection Direction,
	const bool bWrap) const
{
	return FindGeometricTargetInScope(Current, Direction, bWrap, GetActiveNavigationScope());
}

ULunarNavigableWidget* ULunarNavigationSubsystem::FindGeometricTargetInScope(
	ULunarNavigableWidget* Current,
	const ELunarNavigationDirection Direction,
	const bool bWrap,
	const ULunarNavigationScope* Scope,
	const ULunarScrollBox* RestrictToScrollBox) const
{
	if (!IsWidgetEligibleInGraphScope(Current, Scope)
		|| (RestrictToScrollBox
			&& !LunarNavigationSubsystem_Private::IsWidgetDescendantOf(Current, RestrictToScrollBox)))
	{
		return nullptr;
	}

	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	const float ConeHalfAngle = Settings
		? Settings->Navigation.Behavior.FallbackConeHalfAngleDegrees
		: 60.0f;
	const float LateralWeight = Settings
		? Settings->Navigation.Behavior.FallbackLateralWeight
		: 2.0f;
	const float ConeCosine = FMath::Cos(FMath::DegreesToRadians(FMath::Clamp(ConeHalfAngle, 0.0f, 90.0f)));

	const FVector2D CurrentCenter = GetWidgetCenter(Current);
	const FVector2D DirectionVector = GetDirectionVector(Direction);
	const FName CurrentGroup = Current->GetNavigationGroup();
	const bool bAllowCrossGroup = CanFallbackCrossGroup(Scope, CurrentGroup);

	auto FindInGroupPass = [&](const bool bSameGroupOnly) -> ULunarNavigableWidget*
	{
		ULunarNavigableWidget* BestWidget = nullptr;
		double BestPrimaryScore = TNumericLimits<double>::Max();
		double BestSecondaryScore = TNumericLimits<double>::Max();

		for (ULunarNavigableWidget* Candidate : Scope->RegisteredWidgets)
		{
			if (Candidate == Current
				|| !IsWidgetEligibleForNavigationInput(Candidate, Scope, NavigationDispatchInputDevice)
				|| (RestrictToScrollBox
					&& !LunarNavigationSubsystem_Private::IsWidgetDescendantOf(Candidate, RestrictToScrollBox)))
			{
				continue;
			}

			const bool bSameGroup = Candidate->GetNavigationGroup() == CurrentGroup;
			if ((bSameGroupOnly && !bSameGroup) || (!bSameGroupOnly && bSameGroup))
			{
				continue;
			}

			const FVector2D CandidateCenter = GetWidgetCenter(Candidate);
			const FVector2D Delta = CandidateCenter - CurrentCenter;
			const double Distance = Delta.Size();
			if (Distance <= UE_SMALL_NUMBER)
			{
				continue;
			}

			double PrimaryScore = 0.0;
			double SecondaryScore = 0.0;
			if (!bWrap)
			{
				const double Forward = FVector2D::DotProduct(Delta, DirectionVector);
				if (Forward <= 0.0 || Forward / Distance < ConeCosine)
				{
					continue;
				}

				const double Lateral = FMath::Abs(Delta.X * DirectionVector.Y - Delta.Y * DirectionVector.X);
				PrimaryScore = Forward + LateralWeight * Lateral;
				SecondaryScore = 0.0;
			}
			else
			{
				// The smallest projection in the requested coordinate is the opposite boundary.
				PrimaryScore = FVector2D::DotProduct(CandidateCenter, DirectionVector);
				SecondaryScore = FMath::Abs(Delta.X * DirectionVector.Y - Delta.Y * DirectionVector.X);
			}

			const bool bPrimaryTie = FMath::IsNearlyEqual(PrimaryScore, BestPrimaryScore, LunarNavigationSubsystem_Private::ScoreTieTolerance);
			const bool bSecondaryTie = FMath::IsNearlyEqual(SecondaryScore, BestSecondaryScore, LunarNavigationSubsystem_Private::ScoreTieTolerance);
			const bool bBetterPriority = BestWidget && Candidate->GetNavigationPriority() > BestWidget->GetNavigationPriority();
			const bool bBetterOrder = BestWidget
				&& Candidate->GetNavigationPriority() == BestWidget->GetNavigationPriority()
				&& GetRegistrationOrder(Candidate) < GetRegistrationOrder(BestWidget);

			if (!BestWidget
				|| PrimaryScore < BestPrimaryScore - LunarNavigationSubsystem_Private::ScoreTieTolerance
				|| (bPrimaryTie && SecondaryScore < BestSecondaryScore - LunarNavigationSubsystem_Private::ScoreTieTolerance)
				|| (bPrimaryTie && bSecondaryTie && (bBetterPriority || bBetterOrder)))
			{
				BestWidget = Candidate;
				BestPrimaryScore = PrimaryScore;
				BestSecondaryScore = SecondaryScore;
			}
		}

		return BestWidget;
	};

	if (ULunarNavigableWidget* SameGroupTarget = FindInGroupPass(true))
	{
		return SameGroupTarget;
	}
	return bAllowCrossGroup ? FindInGroupPass(false) : nullptr;
}

FVector2D ULunarNavigationSubsystem::GetWidgetCenter(const UWidget* Widget) const
{
	if (!Widget)
	{
		return FVector2D::ZeroVector;
	}

	const FGeometry& Geometry = Widget->GetCachedGeometry();
	return Geometry.LocalToAbsolute(Geometry.GetLocalSize() * 0.5f);
}

FVector2D ULunarNavigationSubsystem::GetDirectionVector(const ELunarNavigationDirection Direction) const
{
	switch (Direction)
	{
	case ELunarNavigationDirection::Up:
		return FVector2D(0.0f, -1.0f);
	case ELunarNavigationDirection::Down:
		return FVector2D(0.0f, 1.0f);
	case ELunarNavigationDirection::Left:
		return FVector2D(-1.0f, 0.0f);
	case ELunarNavigationDirection::Right:
	default:
		return FVector2D(1.0f, 0.0f);
	}
}

bool ULunarNavigationSubsystem::CanFallbackCrossGroup(const ULunarNavigationScope* Scope, const FName GroupId) const
{
	if (Scope)
	{
		for (const FLunarNavigationGroupSettings& GroupSettings : Scope->Settings.NavigationGroups)
		{
			if (GroupSettings.GroupId == GroupId)
			{
				return GroupSettings.bAllowCrossGroupFallback;
			}
		}
	}

	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	return !Settings || Settings->Navigation.Behavior.bAllowCrossGroupFallbackByDefault;
}

bool ULunarNavigationSubsystem::IsHorizontalDirection(const ELunarNavigationDirection Direction) const
{
	return Direction == ELunarNavigationDirection::Left || Direction == ELunarNavigationDirection::Right;
}

ULunarScrollBox* ULunarNavigationSubsystem::FindDeepestContainingScrollBox(
	const UWidget* Widget,
	const bool bRequireConfinement,
	const bool bRequireMatchingOrientation,
	const ELunarNavigationDirection Direction) const
{
	ULunarScrollBox* BestScrollBox = nullptr;
	int32 BestDepth = INDEX_NONE;
	for (const TWeakObjectPtr<ULunarScrollBox>& WeakScrollBox : RegisteredScrollBoxes)
	{
		ULunarScrollBox* ScrollBox = WeakScrollBox.Get();
		if (!IsValid(ScrollBox)
			|| (bRequireConfinement && !ScrollBox->bConstrainNavigation)
			|| !LunarNavigationSubsystem_Private::IsWidgetDescendantOf(Widget, ScrollBox)
			|| (bRequireMatchingOrientation
				&& (ScrollBox->GetOrientation() == Orient_Horizontal) != IsHorizontalDirection(Direction)))
		{
			continue;
		}

		const int32 Depth = LunarNavigationSubsystem_Private::GetLogicalWidgetDepth(ScrollBox);
		if (!BestScrollBox || Depth > BestDepth)
		{
			BestScrollBox = ScrollBox;
			BestDepth = Depth;
		}
	}
	return BestScrollBox;
}

bool ULunarNavigationSubsystem::IsWidgetAllowedByScrollConfinement(
	const ULunarNavigableWidget* Widget) const
{
	const ULunarScrollBox* ActiveConfinement = ActiveScrollNavigationConfinement.Get();
	return !ActiveConfinement
		|| !ActiveConfinement->bConstrainNavigation
		|| LunarNavigationSubsystem_Private::IsWidgetDescendantOf(Widget, ActiveConfinement);
}

bool ULunarNavigationSubsystem::IsScrollBoxAllowedByNavigationConfinement(
	const ULunarScrollBox* ScrollBox) const
{
	const ULunarScrollBox* ActiveConfinement = ActiveScrollNavigationConfinement.Get();
	return !ActiveConfinement
		|| !ActiveConfinement->bConstrainNavigation
		|| LunarNavigationSubsystem_Private::IsWidgetDescendantOf(ScrollBox, ActiveConfinement);
}

bool ULunarNavigationSubsystem::ReleaseScrollNavigationConfinement(
	const bool bRestorePreviousSelection)
{
	if (!ActiveScrollNavigationConfinement.IsValid())
	{
		ActiveScrollNavigationConfinement.Reset();
		ScrollNavigationReturnWidget.Reset();
		return false;
	}

	ULunarNavigableWidget* ReturnWidget = ScrollNavigationReturnWidget.Get();
	ActiveScrollNavigationConfinement.Reset();
	ScrollNavigationReturnWidget.Reset();
	if (bRestorePreviousSelection
		&& IsWidgetEligibleInScope(ReturnWidget, GetActiveNavigationScope()))
	{
		TGuardValue<bool> ReleaseGuard(bReleasingScrollNavigationConfinement, true);
		SetSelectedWidget(ReturnWidget);
	}
	return true;
}

bool ULunarNavigationSubsystem::BeginNativeFocusDelegation(ULunarNavigableWidget* OwnerWidget, UWidget* NativeFocusWidget)
{
	if (!IsWidgetEligibleInScope(OwnerWidget, GetActiveNavigationScope())
		|| OwnerWidget != SelectedWidget
		|| !IsValid(NativeFocusWidget)
		|| NativeFocusWidget == OwnerWidget
		|| !LunarNavigationSubsystem_Private::IsWidgetDescendantOf(NativeFocusWidget, OwnerWidget))
	{
		return false;
	}

	if (DelegatedFocusOwner == OwnerWidget && DelegatedFocusWidget == NativeFocusWidget)
	{
		SynchronizeNativeFocus();
		return true;
	}

	if (DelegatedFocusOwner)
	{
		CommitNativeFocusDelegation(DelegatedFocusOwner);
	}

	DelegatedFocusOwner = OwnerWidget;
	DelegatedFocusWidget = NativeFocusWidget;
	OwnerWidget->NativeOnNativeFocusDelegationStarted(NativeFocusWidget);
	ResetRepeatState();
	UpdateGameplayInputBlocking();
	SynchronizeNativeFocus();
	return true;
}

bool ULunarNavigationSubsystem::CommitNativeFocusDelegation(ULunarNavigableWidget* OwnerWidget)
{
	if (!DelegatedFocusOwner || (OwnerWidget && OwnerWidget != DelegatedFocusOwner))
	{
		return false;
	}

	ULunarNavigableWidget* PreviousOwner = DelegatedFocusOwner;
	UWidget* PreviousFocusWidget = DelegatedFocusWidget;
	DelegatedFocusOwner = nullptr;
	DelegatedFocusWidget = nullptr;
	UpdateGameplayInputBlocking();
	if (IsValid(PreviousOwner))
	{
		PreviousOwner->NativeOnNativeFocusDelegationCommitted(PreviousFocusWidget);
	}
	SynchronizeNativeFocus();
	return true;
}

bool ULunarNavigationSubsystem::CancelNativeFocusDelegation(ULunarNavigableWidget* OwnerWidget)
{
	if (!DelegatedFocusOwner || (OwnerWidget && OwnerWidget != DelegatedFocusOwner))
	{
		return false;
	}

	ULunarNavigableWidget* PreviousOwner = DelegatedFocusOwner;
	UWidget* PreviousFocusWidget = DelegatedFocusWidget;
	DelegatedFocusOwner = nullptr;
	DelegatedFocusWidget = nullptr;
	UpdateGameplayInputBlocking();
	if (IsValid(PreviousOwner))
	{
		PreviousOwner->NativeOnNativeFocusDelegationCancelled(PreviousFocusWidget);
	}
	SynchronizeNativeFocus();
	return true;
}

bool ULunarNavigationSubsystem::IsNativeFocusDelegationActive(const ULunarNavigableWidget* OwnerWidget) const
{
	return DelegatedFocusOwner && (!OwnerWidget || DelegatedFocusOwner == OwnerWidget);
}

void ULunarNavigationSubsystem::SynchronizeNativeFocus()
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	APlayerController* PlayerController = LocalPlayer ? LocalPlayer->GetPlayerController(GetWorld()) : nullptr;
	if (!PlayerController || !GetActiveNavigationScope())
	{
		return;
	}

	if (DelegatedFocusOwner)
	{
		if (DelegatedFocusOwner != SelectedWidget
			|| !IsWidgetEligibleInScope(DelegatedFocusOwner, GetActiveNavigationScope())
			|| !IsValid(DelegatedFocusWidget)
			|| !DelegatedFocusWidget->IsVisible()
			|| !DelegatedFocusWidget->GetIsEnabled())
		{
			CancelNativeFocusDelegation(DelegatedFocusOwner);
			return;
		}
		else if (!DelegatedFocusWidget->HasUserFocus(PlayerController)
			&& !DelegatedFocusWidget->HasUserFocusedDescendants(PlayerController))
		{
			DelegatedFocusWidget->SetUserFocus(PlayerController);
			return;
		}
		else
		{
			return;
		}
	}

	if (SelectedWidget
		&& !bPointerPresentationActive
		&& LastInputDevice != ELunarInputDeviceType::Unknown
		&& !SelectedWidget->IsNavigationInputAllowed(LastInputDevice))
	{
		ClearNativeUserFocus();
		return;
	}

	if (SelectedWidget
		&& !SelectedWidget->HasUserFocus(PlayerController))
	{
		SelectedWidget->SetUserFocus(PlayerController);
	}
}

void ULunarNavigationSubsystem::ClearNativeUserFocus()
{
	if (!FSlateApplication::IsInitialized() || !GetLocalPlayer())
	{
		return;
	}

	GetLocalPlayer()->GetSlateOperations().CancelFocusRequest();
	if (const TSharedPtr<FSlateUser> SlateUser = GetLocalPlayer()->GetSlateUser())
	{
		FSlateApplication::Get().ClearUserFocus(SlateUser->GetUserIndex(), EFocusCause::Cleared);
	}
}

bool ULunarNavigationSubsystem::ShouldBlockAllGameplayInput() const
{
	const ULunarNavigationScope* ActiveScope = GetActiveNavigationScope();
	return ActiveScope && ActiveScope->Settings.bBlockAllGameplayInput;
}

void ULunarNavigationSubsystem::UpdateGameplayInputBlocking()
{
	APlayerController* PlayerController = GetLocalPlayer()
		? GetLocalPlayer()->GetPlayerController(GetWorld())
		: nullptr;
	const bool bShouldBlock = ShouldBlockAllGameplayInput() || IsNativeFocusDelegationActive();

	if (GameplayBlockInputComponent
		&& (!bShouldBlock || GameplayBlockPlayerController.Get() != PlayerController))
	{
		RemoveGameplayInputBlocker();
	}

	if (!bShouldBlock || !PlayerController || GameplayBlockInputComponent)
	{
		return;
	}

	UInputComponent* InputBlocker = NewObject<UInputComponent>(
		PlayerController,
		UInputComponent::StaticClass(),
		NAME_None,
		RF_Transient);
	if (!InputBlocker)
	{
		return;
	}

	InputBlocker->Priority = MAX_int32;
	InputBlocker->bBlockInput = true;
	InputBlocker->RegisterComponent();
	bSavedEnableClickEvents = PlayerController->bEnableClickEvents;
	bSavedEnableTouchEvents = PlayerController->bEnableTouchEvents;
	bSavedEnableMouseOverEvents = PlayerController->bEnableMouseOverEvents;
	bSavedEnableTouchOverEvents = PlayerController->bEnableTouchOverEvents;
	bHasGameplayPointerFlagsSnapshot = true;
	PlayerController->bEnableClickEvents = false;
	PlayerController->bEnableTouchEvents = false;
	PlayerController->bEnableMouseOverEvents = false;
	PlayerController->bEnableTouchOverEvents = false;
	PlayerController->PushInputComponent(InputBlocker);
	GameplayBlockInputComponent = InputBlocker;
	GameplayBlockPlayerController = PlayerController;
}

void ULunarNavigationSubsystem::RemoveGameplayInputBlocker()
{
	if (!GameplayBlockInputComponent)
	{
		GameplayBlockPlayerController.Reset();
		return;
	}

	if (APlayerController* PlayerController = GameplayBlockPlayerController.Get())
	{
		PlayerController->PopInputComponent(GameplayBlockInputComponent);
		if (bHasGameplayPointerFlagsSnapshot)
		{
			PlayerController->bEnableClickEvents = bSavedEnableClickEvents;
			PlayerController->bEnableTouchEvents = bSavedEnableTouchEvents;
			PlayerController->bEnableMouseOverEvents = bSavedEnableMouseOverEvents;
			PlayerController->bEnableTouchOverEvents = bSavedEnableTouchOverEvents;
		}
	}
	GameplayBlockInputComponent->DestroyComponent();
	GameplayBlockInputComponent = nullptr;
	GameplayBlockPlayerController.Reset();
	bHasGameplayPointerFlagsSnapshot = false;
}

void ULunarNavigationSubsystem::RebuildActionDefinitions()
{
	ResolvedActionDefinitions.Reset();
	ActionDefinitionValidationMessages.Reset();
	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	if (!Settings)
	{
		return;
	}

	struct FDefinitionSource
	{
		const FLunarUIActionDefinition* Definition = nullptr;
		FString OwnerPath;
	};

	TArray<FDefinitionSource> Sources;
	Sources.Reserve(Settings->Navigation.Input.ActionDefinitions.Num());
	for (int32 Index = 0; Index < Settings->Navigation.Input.ActionDefinitions.Num(); ++Index)
	{
		FDefinitionSource& Source = Sources.AddDefaulted_GetRef();
		Source.Definition = &Settings->Navigation.Input.ActionDefinitions[Index];
		Source.OwnerPath = FString::Printf(
			TEXT("%s.Navigation.Input.ActionDefinitions[%d]"),
			*Settings->GetPathName(),
			Index);
	}

	const TSoftObjectPtr<ULunarUIActionRegistry>& RegistryReference =
		Settings->Navigation.Input.DefaultActionRegistry;
	ULunarUIActionRegistry* Registry = RegistryReference.LoadSynchronous();
	if (!RegistryReference.IsNull() && !Registry)
	{
		FLunarNavigationValidationMessage Message;
		Message.Verbosity = ELunarConsoleMessageVerbosity::Error;
		Message.Code = TEXT("UnresolvedActionRegistry");
		Message.OwnerPath = RegistryReference.ToSoftObjectPath().ToString();
		Message.Message = FText::Format(
			LOCTEXT(
				"UnresolvedActionRegistry",
				"Default Action Registry '{0}' could not be loaded. Clear or repair the configured asset reference."),
			FText::FromString(Message.OwnerPath));
		ActionDefinitionValidationMessages.Add(Message);
		ReportValidationMessage(Message);
	}
	if (Registry)
	{
		Sources.Reserve(Sources.Num() + Registry->ActionDefinitions.Num());
		for (int32 Index = 0; Index < Registry->ActionDefinitions.Num(); ++Index)
		{
			FDefinitionSource& Source = Sources.AddDefaulted_GetRef();
			Source.Definition = &Registry->ActionDefinitions[Index];
			Source.OwnerPath = FString::Printf(
				TEXT("%s.ActionDefinitions[%d]"),
				*Registry->GetPathName(),
				Index);
		}
	}

	TMap<FGameplayTag, int32> DefinitionCounts;
	for (const FDefinitionSource& Source : Sources)
	{
		if (Source.Definition && Source.Definition->ActionTag.IsValid())
		{
			++DefinitionCounts.FindOrAdd(Source.Definition->ActionTag);
		}
	}

	for (const FDefinitionSource& Source : Sources)
	{
		if (!Source.Definition)
		{
			continue;
		}

		const FLunarUIActionDefinition& Definition = *Source.Definition;
		bool bDefinitionValid = true;
		if (!Definition.ActionTag.IsValid())
		{
			FLunarNavigationValidationMessage Message;
			Message.Verbosity = ELunarConsoleMessageVerbosity::Error;
			Message.Code = TEXT("InvalidActionTag");
			Message.OwnerPath = Source.OwnerPath;
			Message.Message = FText::Format(
				LOCTEXT("InvalidResolvedActionTag", "Action definition '{0}' has no valid semantic ActionTag and was excluded."),
				FText::FromString(Source.OwnerPath));
			ActionDefinitionValidationMessages.Add(Message);
			ReportValidationMessage(Message);
			bDefinitionValid = false;
		}
		else if (DefinitionCounts.FindRef(Definition.ActionTag) > 1)
		{
			FLunarNavigationValidationMessage Message;
			Message.Verbosity = ELunarConsoleMessageVerbosity::Error;
			Message.Code = TEXT("DuplicateActionTag");
			Message.OwnerPath = Source.OwnerPath;
			Message.Message = FText::Format(
				LOCTEXT(
					"DuplicateResolvedActionTag",
					"ActionTag '{0}' has multiple definitions across Lunar Settings and the assigned Action Registry. Every duplicate was excluded."),
				FText::FromName(Definition.ActionTag.GetTagName()));
			ActionDefinitionValidationMessages.Add(Message);
			ReportValidationMessage(Message);
			bDefinitionValid = false;
		}

		TMap<FKey, int32> EnabledBindingCounts;
		for (const FLunarUIActionBinding& Binding : Definition.Bindings)
		{
			if (Binding.bEnabled && Binding.Key.IsValid())
			{
				++EnabledBindingCounts.FindOrAdd(Binding.Key);
			}
		}
		for (int32 BindingIndex = 0; BindingIndex < Definition.Bindings.Num(); ++BindingIndex)
		{
			const FLunarUIActionBinding& Binding = Definition.Bindings[BindingIndex];
			if (!Binding.bEnabled)
			{
				continue;
			}
			if (!Binding.Key.IsValid() || EnabledBindingCounts.FindRef(Binding.Key) > 1)
			{
				FLunarNavigationValidationMessage Message;
				Message.Verbosity = ELunarConsoleMessageVerbosity::Error;
				Message.Code = Binding.Key.IsValid()
					? FName(TEXT("DuplicateActionBindingKey"))
					: FName(TEXT("InvalidActionBindingKey"));
				Message.OwnerPath = FString::Printf(TEXT("%s.Bindings[%d]"), *Source.OwnerPath, BindingIndex);
				Message.Message = Binding.Key.IsValid()
					? FText::Format(
						LOCTEXT(
							"DuplicateResolvedActionBinding",
							"Enabled key '{0}' is duplicated inside ActionTag '{1}'. Keep exactly one enabled binding for this key."),
						Binding.Key.GetDisplayName(),
						FText::FromName(Definition.ActionTag.GetTagName()))
					: FText::Format(
						LOCTEXT(
							"InvalidResolvedActionBinding",
							"ActionTag '{0}' contains an enabled binding with an invalid key. Assign a key or disable the binding."),
						FText::FromName(Definition.ActionTag.GetTagName()));
				ActionDefinitionValidationMessages.Add(Message);
				ReportValidationMessage(Message);
			}
		}

		if (bDefinitionValid)
		{
			ResolvedActionDefinitions.Add(Definition);
		}
	}
}

const FLunarUIActionDefinition* ULunarNavigationSubsystem::FindResolvedActionDefinition(
	const FGameplayTag ActionTag) const
{
	return ResolvedActionDefinitions.FindByPredicate([ActionTag](const FLunarUIActionDefinition& Definition)
	{
		return Definition.ActionTag == ActionTag;
	});
}

ULunarInputIconSet* ULunarNavigationSubsystem::ResolveDefaultPromptIconSet(
	const ELunarInputDeviceType InputDevice) const
{
	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	if (!Settings)
	{
		return nullptr;
	}

	if (InputDevice == ELunarInputDeviceType::KeyboardMouse)
	{
		return Settings->Navigation.Prompts.DefaultKeyboardMouseIconSet.LoadSynchronous();
	}
	if (InputDevice != ELunarInputDeviceType::Gamepad)
	{
		return nullptr;
	}

	switch (LunarNavigationSubsystem_Private::ResolvePromptGamepadFamily(
		LastInputHardwareClass,
		LastInputHardwareIdentifier))
	{
	case LunarNavigationSubsystem_Private::EPromptGamepadFamily::Xbox:
		return Settings->Navigation.Prompts.DefaultXboxIconSet.LoadSynchronous();
	case LunarNavigationSubsystem_Private::EPromptGamepadFamily::PlayStation:
		return Settings->Navigation.Prompts.DefaultPlayStation5IconSet.LoadSynchronous();
	default:
		return nullptr;
	}
}

void ULunarNavigationSubsystem::ResolveInputPromptActions(
	const ULunarNavigableWidget* OwnerWidget,
	UClass* PromptReceiverClass,
	const TArray<FLunarPromptActionRequest>& Requests,
	ULunarInputIconSet* IconSetOverride,
	TArray<FLunarResolvedPromptAction>& OutActions)
{
	OutActions.Reset();
	if (!OwnerWidget)
	{
		return;
	}

	const bool bOwnerAllowsActiveInput = [this, OwnerWidget]()
	{
		switch (LastInputDevice)
		{
		case ELunarInputDeviceType::KeyboardMouse:
			return bPointerPresentationActive
				? OwnerWidget->IsMouseInputAllowed()
				: OwnerWidget->IsKeyboardInputAllowed();
		case ELunarInputDeviceType::Gamepad:
			return OwnerWidget->IsGamepadInputAllowed();
		case ELunarInputDeviceType::Touch:
			return OwnerWidget->IsTouchInputAllowed();
		case ELunarInputDeviceType::Unknown:
		default:
			return true;
		}
	}();
	if (!bOwnerAllowsActiveInput)
	{
		return;
	}

	RebuildActionDefinitions();
	ULunarInputIconSet* ResolvedIconSet = IconSetOverride
		? IconSetOverride
		: ResolveDefaultPromptIconSet(LastInputDevice);
	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	const bool bKeyboardActionsEnabled = !Settings
		|| Settings->Navigation.Input.bEnableKeyboardNavigation;

	for (const FLunarPromptActionRequest& Request : Requests)
	{
		FLunarResolvedPromptAction& Resolved = OutActions.AddDefaulted_GetRef();
		Resolved.ActionTag = Request.ActionTag;
		Resolved.InputDevice = LastInputDevice;
		Resolved.IconSet = ResolvedIconSet;
		Resolved.OwnerWidget = const_cast<ULunarNavigableWidget*>(OwnerWidget);
		Resolved.bSelected = OwnerWidget->IsLunarSelected();
		Resolved.bEnabled = Request.bEnabled
			&& OwnerWidget->IsEffectivelyInteractive()
			&& (LastInputDevice != ELunarInputDeviceType::KeyboardMouse || bKeyboardActionsEnabled);
		Resolved.ResolvedKey = LunarNavigationSubsystem_Private::IsBindingForInputDevice(
			Request.PreferredKey,
			LastInputDevice)
			? Request.PreferredKey
			: EKeys::Invalid;

		const FLunarUIActionDefinition* Definition = FindResolvedActionDefinition(Request.ActionTag);
		if (!Definition)
		{
			Resolved.bEnabled = false;
			Resolved.DisplayText = Request.DisplayTextOverride;
			Resolved.Icon = Request.bRequireIcon
				? LunarNavigationSubsystem_Private::MakeMissingPromptIconBrush()
				: FSlateBrush();
			const FString ErrorKey = FString::Printf(
				TEXT("UnknownPromptAction|%s|%s"),
				*OwnerWidget->GetPathName(),
				*Request.ActionTag.ToString());
			ReportPromptConfigurationError(
				ErrorKey,
				FText::Format(
					LOCTEXT(
						"UnknownPromptAction",
						"Input prompt on widget '{0}' ({1}) requests unknown semantic action '{2}'. Register the action in Lunar Settings or the assigned Action Registry."),
					FText::FromString(OwnerWidget->GetName()),
					FText::FromString(OwnerWidget->GetPathName()),
					FText::FromName(Request.ActionTag.GetTagName())));
			continue;
		}

		Resolved.DisplayText = Request.DisplayTextOverride.IsEmpty()
			? Definition->DisplayText
			: Request.DisplayTextOverride;
		if (!Resolved.ResolvedKey.IsValid())
		{
			for (const FLunarUIActionBinding& Binding : Definition->Bindings)
			{
				if (Binding.bEnabled
					&& LunarNavigationSubsystem_Private::IsBindingForInputDevice(Binding.Key, LastInputDevice))
				{
					Resolved.ResolvedKey = Binding.Key;
					break;
				}
			}
		}

		if (Request.bOverrideIcon)
		{
			if (Request.IconOverride.GetResourceObject()
				|| !Request.IconOverride.GetResourceName().IsNone())
			{
				Resolved.Icon = Request.IconOverride;
			}
			else
			{
				Resolved.Icon = LunarNavigationSubsystem_Private::MakeMissingPromptIconBrush();
				const FString ErrorKey = FString::Printf(
					TEXT("InvalidPromptIconOverride|%s|%s"),
					*OwnerWidget->GetPathName(),
					*Request.ActionTag.ToString());
				ReportPromptConfigurationError(
					ErrorKey,
					FText::Format(
						LOCTEXT(
							"InvalidPromptIconOverride",
							"Input prompt on widget '{0}' enables Icon Override for action '{1}', but the brush has no Texture or Material resource."),
						FText::FromString(OwnerWidget->GetPathName()),
						FText::FromName(Request.ActionTag.GetTagName())));
			}
			continue;
		}
		if (LastInputDevice == ELunarInputDeviceType::Touch)
		{
			Resolved.Icon = FSlateBrush();
			Resolved.IconSet = nullptr;
			continue;
		}
		if (LastInputDevice == ELunarInputDeviceType::Unknown)
		{
			Resolved.Icon = Request.bRequireIcon
				? LunarNavigationSubsystem_Private::MakeMissingPromptIconBrush()
				: FSlateBrush();
			continue;
		}

		FSlateBrush ResolvedIcon;
		const bool bIconResolved = Resolved.ResolvedKey.IsValid()
			&& ResolvedIconSet
			&& ResolvedIconSet->ResolveIconForKey(Resolved.ResolvedKey, ResolvedIcon);
		if (bIconResolved)
		{
			Resolved.Icon = ResolvedIcon;
			continue;
		}
		if (!Request.bRequireIcon)
		{
			Resolved.Icon = FSlateBrush();
			continue;
		}

		Resolved.Icon = LunarNavigationSubsystem_Private::MakeMissingPromptIconBrush();
		const FString IconSetPath = GetPathNameSafe(ResolvedIconSet);
		const FString ErrorKey = FString::Printf(
			TEXT("MissingPromptIcon|%s|%s|%s|%s|%s"),
			*OwnerWidget->GetPathName(),
			*Request.ActionTag.ToString(),
			*LunarNavigationSubsystem_Private::GetInputDeviceDisplayName(LastInputDevice),
			*Resolved.ResolvedKey.ToString(),
			*IconSetPath);
		const FText MissingInformation = !Resolved.ResolvedKey.IsValid()
			? LOCTEXT("MissingPromptBinding", "No enabled binding matches the active input device.")
			: (!ResolvedIconSet
				? FText::Format(
					LOCTEXT(
						"MissingPromptIconSet",
						"No icon set resolves for hardware '{0}::{1}'. Assign the corresponding default Icon Set or a widget override."),
					FText::FromName(LastInputHardwareClass),
					FText::FromName(LastInputHardwareIdentifier))
				: LOCTEXT(
					"MissingPromptIconMapping",
					"The resolved Icon Set has no unique configured icon for this key."));
		ReportPromptConfigurationError(
			ErrorKey,
			FText::Format(
				LOCTEXT(
					"MissingPromptIcon",
					"Missing input-prompt icon. Widget='{0}' Path='{1}' PromptClass='{2}' Action='{3}' Key='{4}' Device='{5}' IconSet='{6}'. {7}"),
				FText::FromString(OwnerWidget->GetName()),
				FText::FromString(OwnerWidget->GetPathName()),
				FText::FromString(GetPathNameSafe(PromptReceiverClass)),
				FText::FromName(Request.ActionTag.GetTagName()),
				Resolved.ResolvedKey.IsValid()
					? Resolved.ResolvedKey.GetDisplayName()
					: LOCTEXT("InvalidPromptKey", "Invalid"),
				FText::FromString(LunarNavigationSubsystem_Private::GetInputDeviceDisplayName(LastInputDevice)),
				FText::FromString(IconSetPath),
				MissingInformation));
	}
}

void ULunarNavigationSubsystem::ReportInputPromptReceiverClassError(
	const ULunarNavigableWidget* OwnerWidget,
	UClass* PromptReceiverClass)
{
	if (!OwnerWidget || !PromptReceiverClass)
	{
		return;
	}
	const FString ErrorKey = FString::Printf(
		TEXT("InvalidPromptReceiver|%s|%s"),
		*OwnerWidget->GetPathName(),
		*PromptReceiverClass->GetPathName());
	ReportPromptConfigurationError(
		ErrorKey,
		FText::Format(
			LOCTEXT(
				"InvalidPromptReceiverClass",
				"Prompt Widget class '{0}' configured for '{1}' does not implement ILunarInputPromptReceiver."),
			FText::FromString(PromptReceiverClass->GetPathName()),
			FText::FromString(OwnerWidget->GetPathName())));
}

void ULunarNavigationSubsystem::ReportPromptConfigurationError(
	const FString& DeduplicationKey,
	const FText& Message)
{
	if (ReportedPromptErrorKeys.Contains(DeduplicationKey))
	{
		return;
	}
	ReportedPromptErrorKeys.Add(DeduplicationKey);
	ULunarConsoleSubsystem::AddMessage(
		FGameplayTag::RequestGameplayTag(TEXT("Lunar.Console"), false),
		ELunarConsoleMessageVerbosity::Error,
		Message.ToString());
	UE_LOG(LogLunarNavigation, Error, TEXT("%s"), *Message.ToString());
}

void ULunarNavigationSubsystem::HandlePromptConfigurationObjectChanged(
	UObject* Object,
	FPropertyChangedEvent& PropertyChangedEvent)
{
	if (!bInitialized || !Object)
	{
		return;
	}
	if (Object->IsA<ULunarSettings>()
		&& PropertyChangedEvent.MemberProperty
		&& PropertyChangedEvent.MemberProperty->GetFName() != GET_MEMBER_NAME_CHECKED(ULunarSettings, Navigation))
	{
		return;
	}

	const bool bAffectsGlobalConfiguration = Object->IsA<ULunarSettings>()
		|| Object->IsA<ULunarUIActionRegistry>()
		|| Object->IsA<ULunarInputIconSet>()
		|| Object->IsA<ULunarUISoundFeedbackAsset>()
		|| Object->IsA<ULunarUIHapticFeedbackAsset>();
	const bool bAffectsScopeGraph = Object->IsA<ULunarNavigableWidget>()
		|| Object->IsA<ULunarScrollBox>()
		|| Object->IsA<ULunarInputPromptWidget>()
		|| Object->IsA<ULunarNavigationScope>()
		|| Object->IsA<UWidgetNavigation>();
	if (!bAffectsGlobalConfiguration && !bAffectsScopeGraph)
	{
		return;
	}

	if (bAffectsGlobalConfiguration)
	{
		ReportedValidationKeys.Reset();
		ReportedScopeValidationKeys.Reset();
		if (Object->IsA<ULunarSettings>() || Object->IsA<ULunarUIActionRegistry>())
		{
			RebuildActionDefinitions();
		}
		if (Object->IsA<ULunarSettings>()
			|| Object->IsA<ULunarUIActionRegistry>()
			|| Object->IsA<ULunarInputIconSet>())
		{
			InvalidateInputPromptPresentation(true);
		}

		for (ULunarNavigationScope* Scope : ScopeStack)
		{
			if (IsValid(Scope))
			{
				ScheduleNavigationScopeValidation(Scope);
			}
		}
		return;
	}

	ULunarNavigableWidget* ChangedWidget = Cast<ULunarNavigableWidget>(Object);
	if (!ChangedWidget && Object->IsA<UWidgetNavigation>())
	{
		ChangedWidget = Object->GetTypedOuter<ULunarNavigableWidget>();
	}
	TArray<ULunarNavigationScope*> AffectedScopes;
	if (ULunarNavigationScope* ChangedScope = Cast<ULunarNavigationScope>(Object))
	{
		AffectedScopes.Add(ChangedScope);
	}
	if (ChangedWidget)
	{
		ReportedPromptErrorKeys.Reset();
		ChangedWidget->InvalidateInputPrompt();
		for (ULunarNavigationScope* Scope : ScopeStack)
		{
			if (Scope && Scope->RegisteredWidgets.Contains(ChangedWidget))
			{
				AffectedScopes.AddUnique(Scope);
			}
		}
		if (ULunarNavigationScope* ResolvedScope = ResolveOwningScopeForWidget(ChangedWidget))
		{
			AffectedScopes.AddUnique(ResolvedScope);
		}
	}
	if (ULunarScrollBox* ChangedScrollBox = Cast<ULunarScrollBox>(Object))
	{
		for (ULunarNavigationScope* Scope : ScopeStack)
		{
			if (IsValid(Scope)
				&& LunarNavigationSubsystem_Private::IsWidgetDescendantOf(ChangedScrollBox, Scope->RootWidget))
			{
				AffectedScopes.AddUnique(Scope);
			}
		}
	}
	if (ULunarInputPromptWidget* ChangedPromptWidget = Cast<ULunarInputPromptWidget>(Object))
	{
		if (ULunarNavigableWidget* PromptOwner = ChangedPromptWidget->GetTypedOuter<ULunarNavigableWidget>())
		{
			if (ULunarNavigationScope* PromptScope = ResolveOwningScopeForWidget(PromptOwner))
			{
				AffectedScopes.AddUnique(PromptScope);
			}
		}
		else
		{
			for (ULunarNavigationScope* Scope : ScopeStack)
			{
				if (IsValid(Scope))
				{
					AffectedScopes.AddUnique(Scope);
				}
			}
		}
	}
	for (ULunarNavigationScope* Scope : AffectedScopes)
	{
		if (IsValid(Scope) && ScopeStack.Contains(Scope))
		{
			RefreshNavigationGraph(Scope);
		}
	}
}

void ULunarNavigationSubsystem::HandleInputHardwareDeviceChanged(
	const FPlatformUserId UserId,
	const FInputDeviceId InputDeviceId)
{
	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!bInitialized
		|| LastInputDevice != ELunarInputDeviceType::Gamepad
		|| !LocalPlayer
		|| LocalPlayer->GetPlatformUserId() != UserId
		|| !InputDeviceId.IsValid())
	{
		return;
	}

	SetLastInputDeviceInternal(ELunarInputDeviceType::Gamepad, false, InputDeviceId);
}

void ULunarNavigationSubsystem::InvalidateInputPromptPresentation(const bool bResetPromptErrors)
{
	if (bResetPromptErrors)
	{
		ReportedPromptErrorKeys.Reset();
	}
	InputPresentationChangedNative.Broadcast(LastInputDevice, bPointerPresentationActive);
}

void ULunarNavigationSubsystem::GetBoundActions(const FKey& Key, TArray<FGameplayTag>& OutActions)
{
	RebuildActionDefinitions();
	OutActions.Reset();
	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	if (!Key.IsGamepadKey() && Settings && !Settings->Navigation.Input.bEnableKeyboardNavigation)
	{
		return;
	}

	for (const FLunarUIActionDefinition& Definition : ResolvedActionDefinitions)
	{
		if (!Definition.ActionTag.IsValid() || OutActions.Contains(Definition.ActionTag))
		{
			continue;
		}

		for (const FLunarUIActionBinding& Binding : Definition.Bindings)
		{
			if (Binding.bEnabled && Binding.Key == Key)
			{
				OutActions.Add(Definition.ActionTag);
				break;
			}
		}
	}
}

bool ULunarNavigationSubsystem::IsEventForOwningLocalPlayer(const FKeyEvent& KeyEvent) const
{
	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	const TSharedPtr<const FSlateUser> SlateUser = LocalPlayer ? LocalPlayer->GetSlateUser() : nullptr;
	return !SlateUser.IsValid() || static_cast<uint32>(SlateUser->GetUserIndex()) == KeyEvent.GetUserIndex();
}

bool ULunarNavigationSubsystem::IsEventForOwningLocalPlayer(const FAnalogInputEvent& AnalogEvent) const
{
	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	const TSharedPtr<const FSlateUser> SlateUser = LocalPlayer ? LocalPlayer->GetSlateUser() : nullptr;
	return !SlateUser.IsValid() || static_cast<uint32>(SlateUser->GetUserIndex()) == AnalogEvent.GetUserIndex();
}

ELunarUIActionResult ULunarNavigationSubsystem::DispatchActionToSelected(const FLunarUIActionContext& ActionContext)
{
	if (!IsWidgetEligibleInScope(SelectedWidget, GetActiveNavigationScope())
		|| !SelectedWidget->IsNavigationInputAllowed(ActionContext.InputDevice)
		|| !SelectedWidget->CanHandleLunarAction(ActionContext))
	{
		return ELunarUIActionResult::Unhandled;
	}

	const ELunarUIActionResult Result = SelectedWidget->HandleLunarAction(ActionContext);
	if (Result == ELunarUIActionResult::Rejected)
	{
		BroadcastRejected(ActionContext);
	}
	return Result;
}

bool ULunarNavigationSubsystem::DispatchPhysicalAction(
	const FKey& Key,
	const EInputEvent InputEvent,
	const bool bIsRepeat,
	const float AnalogMagnitude,
	FPressedActionState* OutPressedAction,
	bool* bOutHandledByWidget)
{
	if (OutPressedAction)
	{
		*OutPressedAction = FPressedActionState();
	}
	if (bOutHandledByWidget)
	{
		*bOutHandledByWidget = false;
	}
	if (!GetActiveNavigationScope())
	{
		return false;
	}

	const ELunarInputDeviceType InputDevice = Key.IsGamepadKey()
		? ELunarInputDeviceType::Gamepad
		: ELunarInputDeviceType::KeyboardMouse;
	TGuardValue<ELunarInputDeviceType> InputDeviceGuard(NavigationDispatchInputDevice, InputDevice);

	TArray<FGameplayTag> BoundActions;
	GetBoundActions(Key, BoundActions);
	if (BoundActions.IsEmpty())
	{
		return false;
	}

	bool bHasPhysicalDirection = false;
	const bool bHasBoundValueAction = BoundActions.Contains(LunarGameplayTags::UI_Action_Increase.GetTag())
		|| BoundActions.Contains(LunarGameplayTags::UI_Action_Decrease.GetTag());
	ELunarNavigationDirection PhysicalDirection = ELunarNavigationDirection::Up;
	for (const FGameplayTag& BoundAction : BoundActions)
	{
		if (LunarNavigationSubsystem_Private::ActionToDirection(BoundAction, PhysicalDirection))
		{
			bHasPhysicalDirection = true;
			break;
		}
	}

	auto MakeContext = [&](const FGameplayTag& ActionTag) -> FLunarUIActionContext
	{
		FLunarUIActionContext Context;
		Context.ActionTag = ActionTag;
		Context.Key = Key;
		Context.InputDevice = Key.IsGamepadKey() ? ELunarInputDeviceType::Gamepad : ELunarInputDeviceType::KeyboardMouse;
		Context.InputEvent = InputEvent;
		Context.bIsRepeat = bIsRepeat;
		Context.bRangeSelectionModifier = HeldSelectionModifierKeys.Contains(EKeys::LeftShift)
			|| HeldSelectionModifierKeys.Contains(EKeys::RightShift)
			|| HeldSelectionModifierKeys.Contains(EKeys::Gamepad_LeftShoulder);
		Context.bAdditiveSelectionModifier = HeldSelectionModifierKeys.Contains(EKeys::LeftControl)
			|| HeldSelectionModifierKeys.Contains(EKeys::RightControl)
			|| HeldSelectionModifierKeys.Contains(EKeys::Gamepad_RightShoulder);
		Context.AnalogMagnitude = AnalogMagnitude;
		Context.LocalPlayer = GetLocalPlayer();
		Context.bHasNavigationDirection = LunarNavigationSubsystem_Private::ActionToDirection(ActionTag, Context.NavigationDirection);
		if (!Context.bHasNavigationDirection && bHasPhysicalDirection)
		{
			Context.bHasNavigationDirection = true;
			Context.NavigationDirection = PhysicalDirection;
		}
		return Context;
	};

	auto TrySelected = [&](const FGameplayTag& ActionTag) -> ELunarUIActionResult
	{
		const FLunarUIActionContext Context = MakeContext(ActionTag);
		const ELunarUIActionResult Result = DispatchActionToSelected(Context);
		if (Result == ELunarUIActionResult::Handled)
		{
			if (bOutHandledByWidget)
			{
				*bOutHandledByWidget = true;
			}
			if (OutPressedAction)
			{
				OutPressedAction->OwnerWidget = SelectedWidget;
				OutPressedAction->ActionTag = ActionTag;
				OutPressedAction->bHasDirection = Context.bHasNavigationDirection;
				OutPressedAction->Direction = Context.NavigationDirection;
				OutPressedAction->bRangeSelectionModifier = Context.bRangeSelectionModifier;
				OutPressedAction->bAdditiveSelectionModifier = Context.bAdditiveSelectionModifier;
			}
		}
		return Result;
	};

	using namespace LunarGameplayTags;
	if (bHasPhysicalDirection && bHasBoundValueAction && IsValid(SelectedWidget))
	{
		FGameplayTag DerivedControlAction;
		if (SelectedWidget->ResolveDirectionalLunarControlAction(PhysicalDirection, DerivedControlAction))
		{
			const ELunarUIActionResult Result = TrySelected(DerivedControlAction);
			if (Result != ELunarUIActionResult::Unhandled)
			{
				return true;
			}
		}
	}

	// Non-directional bindings may target Increase or Decrease directly. Directional keys
	// use the control-derived semantic above so inversion never mislabels the resulting change.
	if (!bHasPhysicalDirection)
	{
		for (const FGameplayTag& ActionTag : BoundActions)
		{
			if (ActionTag == UI_Action_Increase.GetTag() || ActionTag == UI_Action_Decrease.GetTag())
			{
				const ELunarUIActionResult Result = TrySelected(ActionTag);
				if (Result != ELunarUIActionResult::Unhandled)
				{
					return true;
				}
			}
		}
	}

	for (const FGameplayTag& ActionTag : BoundActions)
	{
		if (!LunarNavigationSubsystem_Private::IsNavigationAction(ActionTag)
			&& ActionTag != UI_Action_Increase.GetTag()
			&& ActionTag != UI_Action_Decrease.GetTag()
			&& ActionTag != UI_Action_Accept.GetTag()
			&& ActionTag != UI_Action_Back.GetTag())
		{
			const ELunarUIActionResult Result = TrySelected(ActionTag);
			if (Result != ELunarUIActionResult::Unhandled)
			{
				return true;
			}
		}
	}

	for (const FGameplayTag& ActionTag : BoundActions)
	{
		ELunarNavigationDirection Direction;
		if (!LunarNavigationSubsystem_Private::ActionToDirection(ActionTag, Direction))
		{
			continue;
		}

		const ELunarUIActionResult Result = TrySelected(ActionTag);
		if (Result != ELunarUIActionResult::Unhandled)
		{
			return true;
		}

		if (!Navigate(Direction))
		{
			BroadcastRejected(MakeContext(ActionTag));
		}
		return true;
	}

	if (BoundActions.Contains(UI_Action_Accept.GetTag()))
	{
		if (IsWidgetEligibleInScope(SelectedWidget, GetActiveNavigationScope())
			&& !SelectedWidget->IsNavigationInputAllowed(InputDevice))
		{
			return true;
		}
		const ELunarUIActionResult Result = TrySelected(UI_Action_Accept.GetTag());
		if (Result == ELunarUIActionResult::Unhandled)
		{
			BroadcastRejected(MakeContext(UI_Action_Accept.GetTag()));
		}
		return true;
	}

	if (BoundActions.Contains(UI_Action_Back.GetTag()))
	{
		const ELunarUIActionResult Result = TrySelected(UI_Action_Back.GetTag());
		if (Result == ELunarUIActionResult::Unhandled)
		{
			if (!ReleaseScrollNavigationConfinement(true)
				&& !PopNavigationScope(GetActiveNavigationScope()))
			{
				BroadcastRejected(MakeContext(UI_Action_Back.GetTag()));
			}
		}
		return true;
	}

	return false;
}

bool ULunarNavigationSubsystem::HandleNavigationKeyDown(const FKeyEvent& KeyEvent)
{
	if (!IsEventForOwningLocalPlayer(KeyEvent))
	{
		return false;
	}

	const FKey Key = KeyEvent.GetKey();
	const ELunarInputDeviceType InputDevice = Key.IsGamepadKey()
		? ELunarInputDeviceType::Gamepad
		: ELunarInputDeviceType::KeyboardMouse;
	LunarNavigationSubsystem_Private::SynchronizeKeyboardSelectionModifiers(
		KeyEvent,
		HeldSelectionModifierKeys,
		true);
	if (!Key.IsGamepadKey() && IsExternalNativeTextInputActive())
	{
		return DelegatedFocusOwner && !DelegatedFocusOwner->IsKeyboardInputAllowed();
	}
	SetLastInputDeviceInternal(
		InputDevice,
		false,
		KeyEvent.GetInputDeviceId());
	if (!GetActiveNavigationScope())
	{
		return false;
	}

	if (LunarNavigationSubsystem_Private::IsRangeSelectionModifierKey(Key)
		|| LunarNavigationSubsystem_Private::IsAdditiveSelectionModifierKey(Key))
	{
		HeldSelectionModifierKeys.Add(Key);
	}

	if (IsNativeFocusDelegationActive())
	{
		if (!DelegatedFocusOwner->IsNavigationInputAllowed(InputDevice))
		{
			return true;
		}
		TArray<FGameplayTag> Actions;
		GetBoundActions(Key, Actions);
		if (Actions.Contains(LunarGameplayTags::UI_Action_Back.GetTag()))
		{
			CancelNativeFocusDelegation(DelegatedFocusOwner);
			ConsumedKeyUps.Add(Key);
			return true;
		}
		if (Actions.Contains(LunarGameplayTags::UI_Action_Accept.GetTag()))
		{
			if ((Key.IsGamepadKey() || Key == EKeys::Enter)
				&& DelegatedFocusOwner->NativeShouldCommitNativeFocusDelegationOnAccept(Key))
			{
				CommitNativeFocusDelegation(DelegatedFocusOwner);
				ConsumedKeyUps.Add(Key);
				return true;
			}
		}
		return false;
	}

	// Raw Gamepad_LeftX/Y analog events own left-stick dispatch, hysteresis, and repeat timing.
	// Some platform paths also emit cardinal mirror keys; consume them without dispatching twice.
	if (LunarNavigationSubsystem_Private::IsLeftStickDirectionKey(Key))
	{
		return true;
	}

	if (KeyEvent.IsRepeat() || HeldDigitalKeys.Contains(Key))
	{
		return ConsumedKeyUps.Contains(Key) || ShouldBlockAllGameplayInput();
	}

	FPressedActionState PressedAction;
	bool bHandledByWidget = false;
	const bool bHandled = DispatchPhysicalAction(Key, IE_Pressed, false, 0.0f, &PressedAction, &bHandledByWidget);
	if (bHandled || ShouldBlockAllGameplayInput())
	{
		ConsumedKeyUps.Add(Key);
	}

	if (bHandled)
	{
		TArray<FGameplayTag> Actions;
		GetBoundActions(Key, Actions);
		const bool bRepeatableDirection = Actions.ContainsByPredicate([](const FGameplayTag& ActionTag)
		{
			return LunarNavigationSubsystem_Private::IsNavigationAction(ActionTag);
		});
		if (bRepeatableDirection)
		{
			HeldDigitalKeys.Add(Key);
			DigitalRepeatKey = Key;
			NextDigitalRepeatTime = FPlatformTime::Seconds()
				+ FMath::Max(ResolveRepeatSettingsForSelected(Key).InitialDelay, 0.0f);
		}
		if (bHandledByWidget && PressedAction.OwnerWidget.IsValid())
		{
			PressedActions.Add(Key, PressedAction);
		}
	}

	return bHandled || ShouldBlockAllGameplayInput();
}

bool ULunarNavigationSubsystem::HandleNavigationKeyUp(const FKeyEvent& KeyEvent)
{
	if (!IsEventForOwningLocalPlayer(KeyEvent))
	{
		return false;
	}

	const FKey Key = KeyEvent.GetKey();
	LunarNavigationSubsystem_Private::SynchronizeKeyboardSelectionModifiers(
		KeyEvent,
		HeldSelectionModifierKeys,
		false);
	HeldSelectionModifierKeys.Remove(Key);
	if (!Key.IsGamepadKey() && IsExternalNativeTextInputActive())
	{
		return false;
	}
	SetLastInputDeviceInternal(
		Key.IsGamepadKey() ? ELunarInputDeviceType::Gamepad : ELunarInputDeviceType::KeyboardMouse,
		false,
		KeyEvent.GetInputDeviceId());
	if (LunarNavigationSubsystem_Private::IsLeftStickDirectionKey(Key))
	{
		HeldDigitalKeys.Remove(Key);
		ConsumedKeyUps.Remove(Key);
		PressedActions.Remove(Key);
		if (DigitalRepeatKey == Key)
		{
			DigitalRepeatKey = EKeys::Invalid;
			NextDigitalRepeatTime = 0.0;
		}
		return GetActiveNavigationScope() != nullptr;
	}
	HeldDigitalKeys.Remove(Key);
	if (DigitalRepeatKey == Key)
	{
		DigitalRepeatKey = EKeys::Invalid;
		NextDigitalRepeatTime = 0.0;
	}

	if (!ConsumedKeyUps.Remove(Key))
	{
		return ShouldBlockAllGameplayInput();
	}

	if (FPressedActionState* PressedAction = PressedActions.Find(Key))
	{
		ULunarNavigableWidget* OwnerWidget = PressedAction->OwnerWidget.Get();
		if (OwnerWidget && OwnerWidget == SelectedWidget && IsWidgetEligibleInScope(OwnerWidget, GetActiveNavigationScope()))
		{
			FLunarUIActionContext Context;
			Context.ActionTag = PressedAction->ActionTag;
			Context.Key = Key;
			Context.InputDevice = Key.IsGamepadKey() ? ELunarInputDeviceType::Gamepad : ELunarInputDeviceType::KeyboardMouse;
			Context.InputEvent = IE_Released;
			Context.bHasNavigationDirection = PressedAction->bHasDirection;
			Context.NavigationDirection = PressedAction->Direction;
			Context.bRangeSelectionModifier = PressedAction->bRangeSelectionModifier;
			Context.bAdditiveSelectionModifier = PressedAction->bAdditiveSelectionModifier;
			Context.LocalPlayer = GetLocalPlayer();
			// The press already established ownership. Release must not re-run CanHandleLunarAction:
			// controls may intentionally change the state that made the press claimable.
			const ELunarUIActionResult ReleaseResult = OwnerWidget->HandleLunarAction(Context);
			const ULunarScrollBox* ActiveConfinement = ActiveScrollNavigationConfinement.Get();
			if (ReleaseResult == ELunarUIActionResult::Handled
				&& Context.ActionTag == LunarGameplayTags::UI_Action_Accept.GetTag()
				&& ActiveConfinement
				&& ActiveConfinement->bExitConfinementOnNavigationAccept
				&& LunarNavigationSubsystem_Private::IsWidgetDescendantOf(OwnerWidget, ActiveConfinement))
			{
				ReleaseScrollNavigationConfinement(true);
			}
		}
		PressedActions.Remove(Key);
	}

	return true;
}

bool ULunarNavigationSubsystem::HandleNavigationAnalog(const FAnalogInputEvent& AnalogEvent)
{
	if (!IsEventForOwningLocalPlayer(AnalogEvent))
	{
		return false;
	}

	const FKey Key = AnalogEvent.GetKey();
	SetLastInputDeviceInternal(
		Key.IsGamepadKey() ? ELunarInputDeviceType::Gamepad : ELunarInputDeviceType::KeyboardMouse,
		false,
		AnalogEvent.GetInputDeviceId());
	if (!GetActiveNavigationScope())
	{
		return false;
	}
	if (IsNativeFocusDelegationActive())
	{
		return false;
	}

	if (Key != EKeys::Gamepad_LeftX && Key != EKeys::Gamepad_LeftY)
	{
		return ShouldBlockAllGameplayInput();
	}

	if (Key == EKeys::Gamepad_LeftX)
	{
		AnalogVector.X = AnalogEvent.GetAnalogValue();
	}
	else
	{
		AnalogVector.Y = AnalogEvent.GetAnalogValue();
	}

	bool bDispatched = false;
	UpdateAnalogDirection(FMath::Clamp(AnalogVector.Size(), 0.0f, 1.0f), FPlatformTime::Seconds(), bDispatched);
	TArray<FGameplayTag> ActiveDirectionActions;
	if (bAnalogDirectionActive)
	{
		GetBoundActions(GetAnalogDirectionKey(AnalogDirection), ActiveDirectionActions);
	}
	return bDispatched || !ActiveDirectionActions.IsEmpty() || ShouldBlockAllGameplayInput();
}

void ULunarNavigationSubsystem::NotifyPointerInput(const ELunarInputDeviceType InputDevice)
{
	SetLastInputDeviceInternal(InputDevice, true);
	ResetRepeatState();
}

void ULunarNavigationSubsystem::SetLastInputDeviceInternal(
	const ELunarInputDeviceType InputDevice,
	const bool bPointerInput,
	const FInputDeviceId InputDeviceId)
{
	const bool bInputDeviceChanged = LastInputDevice != InputDevice;
	const bool bPointerPresentationChanged = bPointerPresentationActive != bPointerInput;

	const FInputDeviceId NewInputDeviceId = InputDevice == ELunarInputDeviceType::Gamepad
		? InputDeviceId
		: INPUTDEVICEID_NONE;
	FName NewHardwareClass = NAME_None;
	FName NewHardwareIdentifier = NAME_None;
	if (NewInputDeviceId.IsValid())
	{
		if (const UInputDeviceSubsystem* InputDeviceSubsystem = GEngine
			? GEngine->GetEngineSubsystem<UInputDeviceSubsystem>()
			: nullptr)
		{
			const FHardwareDeviceIdentifier Hardware =
				InputDeviceSubsystem->GetInputDeviceHardwareIdentifier(NewInputDeviceId);
			if (Hardware.IsValid())
			{
				NewHardwareClass = Hardware.InputClassName;
				NewHardwareIdentifier = Hardware.HardwareDeviceIdentifier;
			}
		}
		if (NewHardwareIdentifier.IsNone())
		{
			const TOptional<FInputDeviceDescriptor> Descriptor =
				FInputDeviceRegistry::FindDescriptor(NewInputDeviceId);
			if (Descriptor.IsSet())
			{
				NewHardwareClass = Descriptor->InputDeviceName;
				NewHardwareIdentifier = Descriptor->HardwareDeviceIdentifier;
			}
		}
	}
	const bool bHardwareIdentityChanged = LastInputDeviceId != NewInputDeviceId
		|| LastInputHardwareClass != NewHardwareClass
		|| LastInputHardwareIdentifier != NewHardwareIdentifier;
	LastInputDeviceId = NewInputDeviceId;
	LastInputHardwareClass = NewHardwareClass;
	LastInputHardwareIdentifier = NewHardwareIdentifier;

	bPointerPresentationActive = bPointerInput;
	bHardwareCursorPresentationActive = bPointerInput && InputDevice == ELunarInputDeviceType::KeyboardMouse;
	ApplyActiveScopePointerPolicy();
	if (bInputDeviceChanged)
	{
		LastInputDevice = InputDevice;
		OnInputDeviceChanged.Broadcast(InputDevice);
	}
	if (bPointerPresentationChanged && !bPointerInput)
	{
		SynchronizeNativeFocus();
	}
	if (bInputDeviceChanged || bPointerPresentationChanged || bHardwareIdentityChanged)
	{
		InputPresentationChangedNative.Broadcast(LastInputDevice, bPointerPresentationActive);
	}
}

FLunarPointerPolicy ULunarNavigationSubsystem::ResolveActivePointerPolicy() const
{
	FLunarPointerPolicy ResolvedPolicy;
	if (const ULunarSettings* Settings = GetDefault<ULunarSettings>())
	{
		ResolvedPolicy = Settings->Navigation.Behavior.DefaultPointerPolicy;
	}
	else
	{
		ResolvedPolicy.CursorVisibilityPolicy = ELunarCursorVisibilityPolicy::AutoHideOnNavigation;
		ResolvedPolicy.PointerCapturePolicy = ELunarPointerCapturePolicy::Release;
	}

	for (const ULunarNavigationScope* Scope : ScopeStack)
	{
		if (!Scope)
		{
			continue;
		}

		const FLunarPointerPolicy& ScopePolicy = Scope->Settings.PointerPolicy;
		if (ScopePolicy.CursorVisibilityPolicy != ELunarCursorVisibilityPolicy::Inherit)
		{
			ResolvedPolicy.CursorVisibilityPolicy = ScopePolicy.CursorVisibilityPolicy;
		}
		if (ScopePolicy.PointerCapturePolicy != ELunarPointerCapturePolicy::Inherit)
		{
			ResolvedPolicy.PointerCapturePolicy = ScopePolicy.PointerCapturePolicy;
		}
	}

	return ResolvedPolicy;
}

void ULunarNavigationSubsystem::CapturePreNavigationPointerState()
{
	if (bHasPointerStateSnapshot)
	{
		return;
	}

	APlayerController* PlayerController = GetLocalPlayer()
		? GetLocalPlayer()->GetPlayerController(GetWorld())
		: nullptr;
	UGameInstance* GameInstance = GetLocalPlayer() ? GetLocalPlayer()->GetGameInstance() : nullptr;
	UGameViewportClient* ViewportClient = GameInstance ? GameInstance->GetGameViewportClient() : nullptr;

	PointerStatePlayerController = PlayerController;
	PointerStateViewportClient = ViewportClient;
	if (PlayerController)
	{
		bSavedShowMouseCursor = PlayerController->bShowMouseCursor;
	}
	LunarNavigationSubsystem_Private::RegisterSharedViewportPointerPolicy(this, ViewportClient);

	bHasPointerStateSnapshot = true;
}

void ULunarNavigationSubsystem::ApplyActiveScopePointerPolicy(const bool bReleaseExistingCapture)
{
	if (!bHasPointerStateSnapshot || !GetActiveNavigationScope())
	{
		return;
	}

	const FLunarPointerPolicy Policy = ResolveActivePointerPolicy();
	APlayerController* PlayerController = GetLocalPlayer()
		? GetLocalPlayer()->GetPlayerController(GetWorld())
		: nullptr;
	if (PlayerController != PointerStatePlayerController.Get())
	{
		if (APlayerController* PreviousPlayerController = PointerStatePlayerController.Get())
		{
			PreviousPlayerController->bShowMouseCursor = bSavedShowMouseCursor;
		}
		PointerStatePlayerController = PlayerController;
		bSavedShowMouseCursor = PlayerController ? PlayerController->bShowMouseCursor : false;
	}
	if (PlayerController)
	{
		switch (Policy.CursorVisibilityPolicy)
		{
		case ELunarCursorVisibilityPolicy::AlwaysVisible:
			PlayerController->bShowMouseCursor = true;
			break;
		case ELunarCursorVisibilityPolicy::AutoHideOnNavigation:
			PlayerController->bShowMouseCursor = bHardwareCursorPresentationActive;
			break;
		case ELunarCursorVisibilityPolicy::AlwaysHidden:
			PlayerController->bShowMouseCursor = false;
			break;
		case ELunarCursorVisibilityPolicy::Inherit:
		default:
			break;
		}
	}

	UGameInstance* GameInstance = GetLocalPlayer() ? GetLocalPlayer()->GetGameInstance() : nullptr;
	UGameViewportClient* ViewportClient = GameInstance ? GameInstance->GetGameViewportClient() : nullptr;
	if (ViewportClient != PointerStateViewportClient.Get())
	{
		LunarNavigationSubsystem_Private::UnregisterSharedViewportPointerPolicy(this, PointerStateViewportClient.Get());
		PointerStateViewportClient = ViewportClient;
		LunarNavigationSubsystem_Private::RegisterSharedViewportPointerPolicy(this, ViewportClient);
	}
	if (!ViewportClient)
	{
		return;
	}

	const bool bReleaseCapture = Policy.PointerCapturePolicy == ELunarPointerCapturePolicy::Release;
	LunarNavigationSubsystem_Private::UpdateSharedViewportPointerPolicy(this, ViewportClient, bReleaseCapture);
	if (bReleaseCapture && bReleaseExistingCapture && FSlateApplication::IsInitialized() && GetLocalPlayer())
	{
		if (const TSharedPtr<FSlateUser> SlateUser = GetLocalPlayer()->GetSlateUser())
		{
			FSlateApplication::Get().ReleaseAllPointerCapture(SlateUser->GetUserIndex());
		}
	}
}

void ULunarNavigationSubsystem::RestorePreNavigationPointerState()
{
	if (!bHasPointerStateSnapshot)
	{
		return;
	}
	const bool bPointerPresentationChanged = bPointerPresentationActive;

	if (APlayerController* PlayerController = PointerStatePlayerController.Get())
	{
		PlayerController->bShowMouseCursor = bSavedShowMouseCursor;
	}
	LunarNavigationSubsystem_Private::UnregisterSharedViewportPointerPolicy(this, PointerStateViewportClient.Get());

	PointerStatePlayerController.Reset();
	PointerStateViewportClient.Reset();
	bHasPointerStateSnapshot = false;
	bPointerPresentationActive = false;
	bHardwareCursorPresentationActive = false;
	if (bPointerPresentationChanged)
	{
		InputPresentationChangedNative.Broadcast(LastInputDevice, false);
	}
}

ELunarInputDeviceType ULunarNavigationSubsystem::GetLastInputDevice() const
{
	return LastInputDevice;
}

bool ULunarNavigationSubsystem::GetActiveAnalogNavigationForWidget(
	const ULunarNavigableWidget* Widget,
	ELunarNavigationDirection& OutDirection,
	float& OutMagnitude) const
{
	OutDirection = ELunarNavigationDirection::Up;
	OutMagnitude = 0.0f;
	if (!Widget
		|| Widget != SelectedWidget
		|| !bAnalogDirectionActive
		|| !bHasAnalogPressedAction
		|| AnalogPressedAction.OwnerWidget.Get() != Widget)
	{
		return false;
	}

	OutDirection = AnalogDirection;
	OutMagnitude = FMath::Clamp(ActiveAnalogMagnitude, 0.0f, 1.0f);
	return OutMagnitude > 0.0f;
}

bool ULunarNavigationSubsystem::IsPointerPresentationActive() const
{
	return bPointerPresentationActive;
}

bool ULunarNavigationSubsystem::IsExternalNativeTextInputActive() const
{
	if (!bPointerPresentationActive
		|| IsNativeFocusDelegationActive()
		|| !FSlateApplication::IsInitialized()
		|| !GetLocalPlayer())
	{
		return false;
	}

	APlayerController* PlayerController = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (PlayerController
		&& SelectedWidget
		&& (SelectedWidget->HasUserFocus(PlayerController)
			|| SelectedWidget->HasUserFocusedDescendants(PlayerController)))
	{
		return false;
	}

	const TSharedPtr<const FSlateUser> SlateUser = GetLocalPlayer()->GetSlateUser();
	const TSharedPtr<SWidget> FocusedWidget = SlateUser.IsValid()
		? FSlateApplication::Get().GetUserFocusedWidget(SlateUser->GetUserIndex())
		: nullptr;
	return LunarNavigationSubsystem_Private::IsSlateEditableTextFocus(FocusedWidget);
}

void ULunarNavigationSubsystem::PruneFinishedUISounds()
{
	ActiveUISoundComponents.RemoveAll([](const TObjectPtr<UAudioComponent>& AudioComponent)
	{
		return !IsValid(AudioComponent);
	});
}

void ULunarNavigationSubsystem::StopAllUISounds()
{
	for (UAudioComponent* AudioComponent : ActiveUISoundComponents)
	{
		if (IsValid(AudioComponent))
		{
			AudioComponent->Stop();
		}
	}
	ActiveUISoundComponents.Reset();
}

void ULunarNavigationSubsystem::TickNavigationRepeat(const double CurrentTimeSeconds)
{
	if (IsNativeFocusDelegationActive() || bPointerPresentationActive)
	{
		return;
	}

	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	if (DigitalRepeatKey.IsValid()
		&& HeldDigitalKeys.Contains(DigitalRepeatKey)
		&& CurrentTimeSeconds >= NextDigitalRepeatTime)
	{
		DispatchPhysicalAction(DigitalRepeatKey, IE_Repeat, true, 0.0f);
		const FLunarNavigationRepeatSettings DigitalRepeatSettings =
			ResolveRepeatSettingsForSelected(DigitalRepeatKey);
		NextDigitalRepeatTime = CurrentTimeSeconds
			+ FMath::Max(DigitalRepeatSettings.DigitalRepeatInterval, 0.001f);
	}

	if (bAnalogDirectionActive && CurrentTimeSeconds >= NextAnalogRepeatTime)
	{
		DispatchPhysicalAction(GetAnalogDirectionKey(AnalogDirection), IE_Repeat, true, ActiveAnalogMagnitude);
		const FLunarNavigationRepeatSettings AnalogRepeatSettings =
			ResolveRepeatSettingsForSelected(GetAnalogDirectionKey(AnalogDirection));

		const FLunarAnalogNavigationSettings AnalogSettings = Settings
			? Settings->Navigation.Input.AnalogSettings
			: FLunarAnalogNavigationSettings();
		const float Denominator = FMath::Max(1.0f - AnalogSettings.ActivationThreshold, UE_SMALL_NUMBER);
		const float LinearAlpha = FMath::Clamp(
			(ActiveAnalogMagnitude - AnalogSettings.ActivationThreshold) / Denominator,
			0.0f,
			1.0f);
		const float AccelerationAlpha = LinearAlpha * LinearAlpha * (3.0f - 2.0f * LinearAlpha);
		const float RepeatInterval = FMath::Lerp(
			AnalogRepeatSettings.AnalogIntervalAtThreshold,
			AnalogRepeatSettings.AnalogIntervalAtFullMagnitude,
			AccelerationAlpha);
		NextAnalogRepeatTime = CurrentTimeSeconds + FMath::Max(RepeatInterval, 0.001f);
	}
}

FLunarNavigationRepeatSettings ULunarNavigationSubsystem::ResolveRepeatSettingsForSelected(
	const FKey& DirectionKey)
{
	FLunarNavigationRepeatSettings RepeatSettings;
	if (const ULunarSettings* Settings = GetDefault<ULunarSettings>())
	{
		RepeatSettings = Settings->Navigation.Input.RepeatSettings;
	}

	TArray<FGameplayTag> BoundActions;
	GetBoundActions(DirectionKey, BoundActions);
	ELunarNavigationDirection Direction = ELunarNavigationDirection::Up;
	const bool bHasDirection = BoundActions.ContainsByPredicate([&Direction](const FGameplayTag& ActionTag)
	{
		return LunarNavigationSubsystem_Private::ActionToDirection(ActionTag, Direction);
	});
	const bool bHasBoundValueAction = BoundActions.Contains(LunarGameplayTags::UI_Action_Increase.GetTag())
		|| BoundActions.Contains(LunarGameplayTags::UI_Action_Decrease.GetTag());
	FGameplayTag DerivedControlAction;
	FLunarNavigationRepeatSettings WidgetOverride;
	if (bHasDirection
		&& bHasBoundValueAction
		&& IsValid(SelectedWidget)
		&& SelectedWidget->ResolveDirectionalLunarControlAction(Direction, DerivedControlAction)
		&& SelectedWidget->GetLunarRepeatSettingsOverride(WidgetOverride))
	{
		RepeatSettings = WidgetOverride;
	}
	return RepeatSettings;
}

void ULunarNavigationSubsystem::ReleaseAnalogPressedAction()
{
	if (!bHasAnalogPressedAction)
	{
		return;
	}

	ULunarNavigableWidget* OwnerWidget = AnalogPressedAction.OwnerWidget.Get();
	if (OwnerWidget
		&& OwnerWidget == SelectedWidget
		&& IsWidgetEligibleInScope(OwnerWidget, GetActiveNavigationScope()))
	{
		FLunarUIActionContext Context;
		Context.ActionTag = AnalogPressedAction.ActionTag;
		Context.Key = GetAnalogDirectionKey(AnalogPressedAction.Direction);
		Context.InputDevice = ELunarInputDeviceType::Gamepad;
		Context.InputEvent = IE_Released;
		Context.bHasNavigationDirection = AnalogPressedAction.bHasDirection;
		Context.NavigationDirection = AnalogPressedAction.Direction;
		Context.bRangeSelectionModifier = AnalogPressedAction.bRangeSelectionModifier;
		Context.bAdditiveSelectionModifier = AnalogPressedAction.bAdditiveSelectionModifier;
		Context.LocalPlayer = GetLocalPlayer();
		OwnerWidget->HandleLunarAction(Context);
	}

	AnalogPressedAction = FPressedActionState();
	bHasAnalogPressedAction = false;
}

void ULunarNavigationSubsystem::ResetRepeatState()
{
	ReleaseAnalogPressedAction();
	HeldDigitalKeys.Reset();
	HeldSelectionModifierKeys.Reset();
	DigitalRepeatKey = EKeys::Invalid;
	NextDigitalRepeatTime = 0.0;
	AnalogVector = FVector2D::ZeroVector;
	bAnalogDirectionActive = false;
	ActiveAnalogMagnitude = 0.0f;
	NextAnalogRepeatTime = 0.0;
}

FKey ULunarNavigationSubsystem::GetAnalogDirectionKey(const ELunarNavigationDirection Direction) const
{
	switch (Direction)
	{
	case ELunarNavigationDirection::Up:
		return EKeys::Gamepad_LeftStick_Up;
	case ELunarNavigationDirection::Down:
		return EKeys::Gamepad_LeftStick_Down;
	case ELunarNavigationDirection::Left:
		return EKeys::Gamepad_LeftStick_Left;
	case ELunarNavigationDirection::Right:
	default:
		return EKeys::Gamepad_LeftStick_Right;
	}
}

bool ULunarNavigationSubsystem::UpdateAnalogDirection(
	const float Magnitude,
	const double CurrentTimeSeconds,
	bool& bOutDispatched)
{
	bOutDispatched = false;
	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	const FLunarAnalogNavigationSettings AnalogSettings = Settings
		? Settings->Navigation.Input.AnalogSettings
		: FLunarAnalogNavigationSettings();
	ActiveAnalogMagnitude = Magnitude;
	if (Magnitude < FMath::Max(AnalogSettings.RadialDeadZone, AnalogSettings.ReleaseThreshold))
	{
		ReleaseAnalogPressedAction();
		bAnalogDirectionActive = false;
		ActiveAnalogMagnitude = 0.0f;
		NextAnalogRepeatTime = 0.0;
		return false;
	}

	const float AbsX = FMath::Abs(AnalogVector.X);
	const float AbsY = FMath::Abs(AnalogVector.Y);
	const bool bHorizontalDominant = AbsX > AbsY;
	const ELunarNavigationDirection CandidateDirection = bHorizontalDominant
		? (AnalogVector.X < 0.0f ? ELunarNavigationDirection::Left : ELunarNavigationDirection::Right)
		: (AnalogVector.Y < 0.0f ? ELunarNavigationDirection::Down : ELunarNavigationDirection::Up);
	const float CandidateAxisMagnitude = bHorizontalDominant ? AbsX : AbsY;

	bool bActivateCandidate = false;
	if (!bAnalogDirectionActive)
	{
		bActivateCandidate = Magnitude >= AnalogSettings.ActivationThreshold;
	}
	else
	{
		const bool bCurrentHorizontal = IsHorizontalDirection(AnalogDirection);
		float CurrentAxisMagnitude = bCurrentHorizontal ? AbsX : AbsY;
		const bool bCurrentDirectionStillHasSameSign =
			(AnalogDirection == ELunarNavigationDirection::Right && AnalogVector.X >= 0.0f)
			|| (AnalogDirection == ELunarNavigationDirection::Left && AnalogVector.X <= 0.0f)
			|| (AnalogDirection == ELunarNavigationDirection::Up && AnalogVector.Y >= 0.0f)
			|| (AnalogDirection == ELunarNavigationDirection::Down && AnalogVector.Y <= 0.0f);
		if (!bCurrentDirectionStillHasSameSign)
		{
			CurrentAxisMagnitude = 0.0f;
		}
		if (CurrentAxisMagnitude < AnalogSettings.ReleaseThreshold)
		{
			ReleaseAnalogPressedAction();
			bAnalogDirectionActive = false;
			bActivateCandidate = Magnitude >= AnalogSettings.ActivationThreshold;
		}
		else if (CandidateDirection != AnalogDirection
			&& CandidateAxisMagnitude >= CurrentAxisMagnitude + AnalogSettings.DirectionChangeDominanceMargin)
		{
			ReleaseAnalogPressedAction();
			bActivateCandidate = Magnitude >= AnalogSettings.ActivationThreshold;
		}
	}

	if (!bActivateCandidate)
	{
		return bAnalogDirectionActive;
	}

	AnalogDirection = CandidateDirection;
	bAnalogDirectionActive = true;
	FPressedActionState PressedAction;
	bool bHandledByWidget = false;
	bOutDispatched = DispatchPhysicalAction(
		GetAnalogDirectionKey(AnalogDirection),
		IE_Pressed,
		false,
		Magnitude,
		&PressedAction,
		&bHandledByWidget);
	if (bHandledByWidget && PressedAction.OwnerWidget.IsValid())
	{
		AnalogPressedAction = PressedAction;
		bHasAnalogPressedAction = true;
	}
	NextAnalogRepeatTime = CurrentTimeSeconds
		+ FMath::Max(
			ResolveRepeatSettingsForSelected(GetAnalogDirectionKey(AnalogDirection)).InitialDelay,
			0.0f);
	return true;
}

void ULunarNavigationSubsystem::BroadcastRejected(const FLunarUIActionContext& ActionContext)
{
	if (IsValid(SelectedWidget))
	{
		SelectedWidget->NotifyLunarRejectedFromSubsystem(ActionContext);
	}
	OnNavigationRejected.Broadcast(SelectedWidget, ActionContext);
}

TArray<FLunarNavigationValidationMessage> ULunarNavigationSubsystem::ValidateNavigationScope(ULunarNavigationScope* Scope)
{
	TArray<FLunarNavigationValidationMessage> UnfilteredMessages;
	TArray<FLunarNavigationValidationMessage> Messages;
	Scope = Scope ? Scope : GetActiveNavigationScope();
	if (!IsValid(Scope))
	{
		return Messages;
	}

	const FString ScopeOwnerPath = Scope->RootWidget
		? Scope->RootWidget->GetPathName()
		: Scope->GetPathName();
	TSet<int32> GlobalMessageIndices;
	auto AddMessage = [&UnfilteredMessages](
		const ELunarConsoleMessageVerbosity Verbosity,
		const FName Code,
		FString OwnerPath,
		FText MessageText) -> int32
	{
		FLunarNavigationValidationMessage& Message = UnfilteredMessages.AddDefaulted_GetRef();
		Message.Verbosity = Verbosity;
		Message.Code = Code;
		Message.OwnerPath = MoveTemp(OwnerPath);
		Message.Message = MoveTemp(MessageText);
		return UnfilteredMessages.Num() - 1;
	};
	auto AddGlobalMessage = [&AddMessage, &GlobalMessageIndices](
		const ELunarConsoleMessageVerbosity Verbosity,
		const FName Code,
		FString OwnerPath,
		FText MessageText)
	{
		GlobalMessageIndices.Add(AddMessage(Verbosity, Code, MoveTemp(OwnerPath), MoveTemp(MessageText)));
	};
	auto FindRegisteredWidgetById = [](const ULunarNavigationScope* SearchScope, const FName NavigationId)
		-> ULunarNavigableWidget*
	{
		if (!SearchScope || NavigationId.IsNone())
		{
			return nullptr;
		}
		for (ULunarNavigableWidget* Candidate : SearchScope->RegisteredWidgets)
		{
			if (IsValid(Candidate) && Candidate->GetNavigationId() == NavigationId)
			{
				return Candidate;
			}
		}
		return nullptr;
	};
	auto FindOtherScopeForId = [this, Scope, &FindRegisteredWidgetById](const FName NavigationId)
		-> ULunarNavigationScope*
	{
		for (ULunarNavigationScope* OtherScope : ScopeStack)
		{
			if (OtherScope != Scope && FindRegisteredWidgetById(OtherScope, NavigationId))
			{
				return OtherScope;
			}
		}
		return nullptr;
	};

	if (!Scope->RootWidget)
	{
		AddMessage(
			ELunarConsoleMessageVerbosity::Error,
			TEXT("MissingScopeRoot"),
			ScopeOwnerPath,
			LOCTEXT("MissingScopeRoot", "Navigation scope has no RootWidget. Recreate the scope with a valid visual root."));
	}
	if (Scope->GetOwningLocalPlayer() != GetLocalPlayer())
	{
		AddMessage(
			ELunarConsoleMessageVerbosity::Error,
			TEXT("ScopeLocalPlayerMismatch"),
			ScopeOwnerPath,
			LOCTEXT("ScopeLocalPlayerMismatch", "Navigation scope belongs to a different local player than the validating subsystem."));
	}

	TMap<FName, ULunarNavigableWidget*> SeenIds;
	for (ULunarNavigableWidget* Widget : Scope->RegisteredWidgets)
	{
		if (!IsValid(Widget) || Widget->GetNavigationId().IsNone())
		{
			continue;
		}

		if (ULunarNavigableWidget** ExistingWidget = SeenIds.Find(Widget->GetNavigationId()))
		{
			AddMessage(
				ELunarConsoleMessageVerbosity::Error,
				TEXT("DuplicateNavigationId"),
				ScopeOwnerPath,
				FText::Format(
					LOCTEXT(
						"DuplicateNavigationId",
						"Duplicate NavigationId '{0}' in scope '{1}': '{2}' and '{3}'. Assign a unique ID to each widget."),
					FText::FromName(Widget->GetNavigationId()),
					FText::FromString(ScopeOwnerPath),
					FText::FromString(GetPathNameSafe(*ExistingWidget)),
					FText::FromString(Widget->GetPathName())));
		}
		else
		{
			SeenIds.Add(Widget->GetNavigationId(), Widget);
		}
	}

	TMap<FName, int32> GroupSettingCounts;
	for (const FLunarNavigationGroupSettings& GroupSettings : Scope->Settings.NavigationGroups)
	{
		if (!GroupSettings.GroupId.IsNone())
		{
			++GroupSettingCounts.FindOrAdd(GroupSettings.GroupId);
		}
	}
	for (const TPair<FName, int32>& GroupCount : GroupSettingCounts)
	{
		if (GroupCount.Value > 1)
		{
			AddMessage(
				ELunarConsoleMessageVerbosity::Error,
				TEXT("DuplicateNavigationGroupSettings"),
				ScopeOwnerPath,
				FText::Format(
					LOCTEXT(
						"DuplicateNavigationGroupSettings",
						"Navigation Group '{0}' has {1} scope settings entries. Keep exactly one entry so cross-group fallback is deterministic."),
					FText::FromName(GroupCount.Key),
					FText::AsNumber(GroupCount.Value)));
		}
	}

	TArray<ULunarNavigableWidget*> EligibleWidgets;
	for (ULunarNavigableWidget* Widget : Scope->RegisteredWidgets)
	{
		if (IsWidgetEligibleInGraphScope(Widget, Scope))
		{
			EligibleWidgets.Add(Widget);
		}
	}
	if (EligibleWidgets.IsEmpty())
	{
		AddMessage(
			ELunarConsoleMessageVerbosity::Warning,
			TEXT("EmptyNavigationScope"),
			ScopeOwnerPath,
			LOCTEXT(
				"EmptyNavigationScope",
				"Navigation scope has no currently eligible Lunar widgets. Selection will remain empty until an eligible widget becomes available."));
	}

	ULunarNavigableWidget* InitialWidget = Scope->Settings.InitialSelectionWidget;
	const FName EffectiveInitialSelectionId = Scope->GetInitialSelectionId();
	ULunarNavigableWidget* InitialById = FindRegisteredWidgetById(Scope, EffectiveInitialSelectionId);
	if (InitialWidget)
	{
		if (!ScopeContainsWidget(Scope, InitialWidget))
		{
			AddMessage(
				ELunarConsoleMessageVerbosity::Error,
				TEXT("InitialSelectionOutsideScope"),
				ScopeOwnerPath,
				FText::Format(
					LOCTEXT(
						"InitialSelectionOutsideScope",
						"Initial selection widget '{0}' is not registered in scope '{1}'. Assign a widget owned by this scope."),
					FText::FromString(GetPathNameSafe(InitialWidget)),
					FText::FromString(ScopeOwnerPath)));
		}
		else if (!IsWidgetEligibleInGraphScope(InitialWidget, Scope))
		{
			AddMessage(
				ELunarConsoleMessageVerbosity::Warning,
				TEXT("InitialSelectionUnavailable"),
				InitialWidget->GetPathName(),
				FText::Format(
					LOCTEXT(
						"InitialSelectionUnavailable",
						"Initial selection widget '{0}' is currently ineligible. Lunar will use deterministic fallback until it becomes available."),
					FText::FromString(InitialWidget->GetPathName())));
		}
	}
	if (!EffectiveInitialSelectionId.IsNone())
	{
		if (!InitialById)
		{
			if (ULunarNavigationScope* OtherScope = FindOtherScopeForId(EffectiveInitialSelectionId))
			{
				AddMessage(
					ELunarConsoleMessageVerbosity::Error,
					TEXT("InitialSelectionIdOutsideScope"),
					ScopeOwnerPath,
					FText::Format(
						LOCTEXT(
							"InitialSelectionIdOutsideScope",
							"InitialSelectionId '{0}' resolves only in another scope '{1}'. Navigation IDs cannot cross local-player scopes."),
						FText::FromName(EffectiveInitialSelectionId),
						FText::FromString(GetPathNameSafe(OtherScope))));
			}
			else
			{
				AddMessage(
					ELunarConsoleMessageVerbosity::Error,
					TEXT("UnresolvedInitialSelectionId"),
					ScopeOwnerPath,
					FText::Format(
						LOCTEXT(
							"UnresolvedInitialSelectionId",
							"InitialSelectionId '{0}' does not resolve to a registered widget in scope '{1}'."),
						FText::FromName(EffectiveInitialSelectionId),
						FText::FromString(ScopeOwnerPath)));
			}
		}
		else if (!IsWidgetEligibleInGraphScope(InitialById, Scope))
		{
			AddMessage(
				ELunarConsoleMessageVerbosity::Warning,
				TEXT("InitialSelectionIdUnavailable"),
				InitialById->GetPathName(),
				FText::Format(
					LOCTEXT(
						"InitialSelectionIdUnavailable",
						"InitialSelectionId '{0}' resolves to a currently ineligible widget. Lunar will use deterministic fallback."),
					FText::FromName(EffectiveInitialSelectionId)));
		}
	}
	if (InitialWidget
		&& ScopeContainsWidget(Scope, InitialWidget)
		&& InitialById
		&& InitialWidget != InitialById)
	{
		AddMessage(
			ELunarConsoleMessageVerbosity::Error,
			TEXT("ConflictingInitialSelection"),
			ScopeOwnerPath,
			FText::Format(
				LOCTEXT(
					"ConflictingInitialSelection",
					"InitialSelectionWidget '{0}' and InitialSelectionId '{1}' resolve to different widgets. Clear one or make them agree."),
				FText::FromString(InitialWidget->GetPathName()),
				FText::FromName(EffectiveInitialSelectionId)));
	}

	const ELunarNavigationDirection Directions[] =
	{
		ELunarNavigationDirection::Up,
		ELunarNavigationDirection::Down,
		ELunarNavigationDirection::Left,
		ELunarNavigationDirection::Right
	};
	for (ULunarNavigableWidget* Widget : Scope->RegisteredWidgets)
	{
		if (!IsValid(Widget))
		{
			continue;
		}

		if (const ULunarRadio* Radio = Cast<ULunarRadio>(Widget))
		{
			TMap<FString, int32> SeenRadioValues;
			const TArray<FLunarRadioSideVisualData>& RadioData = Radio->GetSideVisualDataArray();
			for (int32 OptionIndex = 0; OptionIndex < RadioData.Num(); ++OptionIndex)
			{
				const FString& StringValue = RadioData[OptionIndex].StringValue;
				if (StringValue.IsEmpty())
				{
					AddMessage(
						ELunarConsoleMessageVerbosity::Error,
						TEXT("EmptyRadioStringValue"),
						Radio->GetPathName(),
						FText::Format(
							LOCTEXT("EmptyRadioStringValue", "Radio '{0}' option {1} has an empty StringValue. Assign a unique technical ID."),
							FText::FromString(Radio->GetPathName()),
							FText::AsNumber(OptionIndex)));
					continue;
				}
				if (const int32* ExistingIndex = SeenRadioValues.Find(StringValue))
				{
					AddMessage(
						ELunarConsoleMessageVerbosity::Error,
						TEXT("DuplicateRadioStringValue"),
						Radio->GetPathName(),
						FText::Format(
							LOCTEXT("DuplicateRadioStringValue", "Radio '{0}' options {1} and {2} share StringValue '{3}'. Assign a unique technical ID to every option."),
							FText::FromString(Radio->GetPathName()),
							FText::AsNumber(*ExistingIndex),
							FText::AsNumber(OptionIndex),
							FText::FromString(StringValue)));
				}
				else
				{
					SeenRadioValues.Add(StringValue, OptionIndex);
				}
			}
		}

		for (const ELunarNavigationDirection Direction : Directions)
		{
			const FLunarNavigationLink& Link = Widget->GetNavigationLink(Direction);
			const FString DirectionName = LunarNavigationSubsystem_Private::GetDirectionName(Direction);
			if (Link.Mode == ELunarNavigationLinkMode::Widget)
			{
				if (!IsValid(Link.Widget))
				{
					AddMessage(
						ELunarConsoleMessageVerbosity::Error,
						TEXT("UnresolvedExplicitWidgetLink"),
						Widget->GetPathName(),
						FText::Format(
							LOCTEXT(
								"UnresolvedExplicitWidgetLink",
								"Widget '{0}' has an unresolved explicit {1} widget link. Assign a valid target or use Automatic/Block."),
							FText::FromString(Widget->GetPathName()),
							FText::FromString(DirectionName)));
				}
				else if (!ScopeContainsWidget(Scope, Link.Widget))
				{
					AddMessage(
						ELunarConsoleMessageVerbosity::Error,
						TEXT("ExplicitWidgetLinkOutsideScope"),
						Widget->GetPathName(),
						FText::Format(
							LOCTEXT(
								"ExplicitWidgetLinkOutsideScope",
								"Widget '{0}' has a {1} link to '{2}', which is outside scope '{3}'. Explicit links cannot cross scopes."),
							FText::FromString(Widget->GetPathName()),
							FText::FromString(DirectionName),
							FText::FromString(Link.Widget->GetPathName()),
							FText::FromString(ScopeOwnerPath)));
				}
				else if (Link.Widget == Widget)
				{
					AddMessage(
						ELunarConsoleMessageVerbosity::Error,
						TEXT("SelfReferentialNavigationLink"),
						Widget->GetPathName(),
						FText::Format(
							LOCTEXT(
								"SelfReferentialNavigationLink",
								"Widget '{0}' has a self-referential {1} link. Choose another target or Block the direction."),
							FText::FromString(Widget->GetPathName()),
							FText::FromString(DirectionName)));
				}
				else if (!IsWidgetEligibleInGraphScope(Link.Widget, Scope))
				{
					AddMessage(
						ELunarConsoleMessageVerbosity::Warning,
						TEXT("ExplicitLinkTargetUnavailable"),
						Widget->GetPathName(),
						FText::Format(
							LOCTEXT(
								"ExplicitWidgetLinkTargetUnavailable",
								"Widget '{0}' has a {1} link to currently ineligible target '{2}'. Lunar will use geometric fallback while it remains unavailable."),
							FText::FromString(Widget->GetPathName()),
							FText::FromString(DirectionName),
							FText::FromString(Link.Widget->GetPathName())));
				}
			}
			else if (Link.Mode == ELunarNavigationLinkMode::NavigationId)
			{
				if (Link.NavigationId.IsNone())
				{
					AddMessage(
						ELunarConsoleMessageVerbosity::Error,
						TEXT("EmptyExplicitNavigationId"),
						Widget->GetPathName(),
						FText::Format(
							LOCTEXT(
								"EmptyExplicitNavigationId",
								"Widget '{0}' uses NavigationId mode for {1}, but the target ID is empty."),
							FText::FromString(Widget->GetPathName()),
							FText::FromString(DirectionName)));
					continue;
				}

				ULunarNavigableWidget* Target = FindRegisteredWidgetById(Scope, Link.NavigationId);
				if (!Target)
				{
					if (ULunarNavigationScope* OtherScope = FindOtherScopeForId(Link.NavigationId))
					{
						AddMessage(
							ELunarConsoleMessageVerbosity::Error,
							TEXT("ExplicitNavigationIdOutsideScope"),
							Widget->GetPathName(),
							FText::Format(
								LOCTEXT(
									"ExplicitNavigationIdOutsideScope",
									"Widget '{0}' has a {1} link to NavigationId '{2}', which resolves only in another scope '{3}'."),
								FText::FromString(Widget->GetPathName()),
								FText::FromString(DirectionName),
								FText::FromName(Link.NavigationId),
								FText::FromString(GetPathNameSafe(OtherScope))));
					}
					else
					{
						AddMessage(
							ELunarConsoleMessageVerbosity::Error,
							TEXT("UnresolvedExplicitNavigationId"),
							Widget->GetPathName(),
							FText::Format(
								LOCTEXT(
									"UnresolvedExplicitNavigationId",
									"Widget '{0}' has a {1} link to unknown NavigationId '{2}' in scope '{3}'."),
								FText::FromString(Widget->GetPathName()),
								FText::FromString(DirectionName),
								FText::FromName(Link.NavigationId),
								FText::FromString(ScopeOwnerPath)));
					}
				}
				else if (Target == Widget)
				{
					AddMessage(
						ELunarConsoleMessageVerbosity::Error,
						TEXT("SelfReferentialNavigationLink"),
						Widget->GetPathName(),
						FText::Format(
							LOCTEXT(
								"SelfReferentialNavigationIdLink",
								"Widget '{0}' has a self-referential {1} NavigationId link '{2}'."),
							FText::FromString(Widget->GetPathName()),
							FText::FromString(DirectionName),
							FText::FromName(Link.NavigationId)));
				}
				else if (!IsWidgetEligibleInGraphScope(Target, Scope))
				{
					AddMessage(
						ELunarConsoleMessageVerbosity::Warning,
						TEXT("ExplicitLinkTargetUnavailable"),
						Widget->GetPathName(),
						FText::Format(
							LOCTEXT(
								"ExplicitNavigationIdTargetUnavailable",
								"Widget '{0}' has a {1} link to NavigationId '{2}', whose target '{3}' is currently ineligible. Lunar will use geometric fallback."),
							FText::FromString(Widget->GetPathName()),
							FText::FromString(DirectionName),
							FText::FromName(Link.NavigationId),
							FText::FromString(Target->GetPathName())));
				}
			}
		}

		if (Widget->Navigation && !Widget->Navigation->IsDefaultNavigation())
		{
			AddMessage(
				ELunarConsoleMessageVerbosity::Error,
				TEXT("NativeUMGNavigationConfigured"),
				Widget->GetPathName(),
				FText::Format(
					LOCTEXT(
						"NativeUMGNavigationConfigured",
						"Lunar widget '{0}' contains native UMG Navigation data. Lunar ignores it; clear the native rules to remove conflicting configuration."),
					FText::FromString(Widget->GetPathName())));
		}

		if (IsWidgetEligibleInGraphScope(Widget, Scope))
		{
			if (const UScrollBox* NativeScrollBox = LunarNavigationSubsystem_Private::FindUnsupportedNativeScrollBoxAncestor(Widget))
			{
				AddMessage(
					ELunarConsoleMessageVerbosity::Warning,
					TEXT("UnsupportedNativeScrollBox"),
					Widget->GetPathName(),
					FText::Format(
						LOCTEXT(
							"UnsupportedNativeScrollBox",
							"Eligible Lunar widget '{0}' is inside native UScrollBox '{1}'. Use ULunarScrollBox when full Lunar scroll behavior is expected."),
						FText::FromString(Widget->GetPathName()),
						FText::FromString(NativeScrollBox->GetPathName())));
			}
		}

		const ULunarSettings* Settings = GetDefault<ULunarSettings>();
		if (IsWidgetEligibleInGraphScope(Widget, Scope)
			&& (!Settings || Settings->Navigation.Accessibility.bValidateAccessibleNames)
			&& Widget->AccessibleName.IsEmpty())
		{
			AddMessage(
				ELunarConsoleMessageVerbosity::Warning,
				TEXT("MissingAccessibleName"),
				Widget->GetPathName(),
				FText::Format(
					LOCTEXT(
						"MissingAccessibleName",
						"Eligible Lunar widget '{0}' has no AccessibleName. Add accessible text for assistive technology."),
					FText::FromString(Widget->GetPathName())));
		}

		auto ValidateSoundFeedback = [&AddMessage, Widget](
			const FLunarUISoundOverride& Override,
			const FLunarUISoundSpec* DataAssetEntry,
			const TCHAR* EventName)
		{
			if (Override.Mode != ELunarFeedbackOverrideMode::UseDataAsset)
			{
				return;
			}
			if (!IsValid(Widget->SoundFeedbackAsset))
			{
				AddMessage(
					ELunarConsoleMessageVerbosity::Warning,
					FName(*FString::Printf(TEXT("MissingSoundFeedbackAsset_%s"), EventName)),
					Widget->GetPathName(),
					FText::Format(
						LOCTEXT(
							"MissingSoundFeedbackAsset",
							"Widget '{0}' uses Data Asset sound feedback for '{1}', but SoundFeedbackAsset is not assigned. Assign a sound feedback asset, choose Custom/Use Global, or disable this event."),
						FText::FromString(Widget->GetPathName()),
						FText::FromString(EventName)));
				return;
			}
			if (!DataAssetEntry || !IsValid(DataAssetEntry->Sound))
			{
				AddMessage(
					ELunarConsoleMessageVerbosity::Warning,
					FName(*FString::Printf(TEXT("MissingSoundFeedbackEntry_%s"), EventName)),
					Widget->GetPathName(),
					FText::Format(
						LOCTEXT(
							"MissingSoundFeedbackEntry",
							"Sound feedback asset '{0}' has no sound for '{1}', requested by widget '{2}'. Configure the entry, choose Custom/Use Global, or disable this event. Lunar intentionally does not fall back."),
						FText::FromString(Widget->SoundFeedbackAsset->GetPathName()),
						FText::FromString(EventName),
						FText::FromString(Widget->GetPathName())));
			}
		};
		ValidateSoundFeedback(Widget->SoundOverrides.PointerHovered, Widget->SoundFeedbackAsset ? &Widget->SoundFeedbackAsset->Sounds.PointerHovered : nullptr, TEXT("Pointer Hovered"));
		ValidateSoundFeedback(Widget->SoundOverrides.PointerPressed, Widget->SoundFeedbackAsset ? &Widget->SoundFeedbackAsset->Sounds.PointerPressed : nullptr, TEXT("Pointer Pressed"));
		ValidateSoundFeedback(Widget->SoundOverrides.PointerClicked, Widget->SoundFeedbackAsset ? &Widget->SoundFeedbackAsset->Sounds.PointerClicked : nullptr, TEXT("Pointer Clicked"));
		ValidateSoundFeedback(Widget->SoundOverrides.PointerRejected, Widget->SoundFeedbackAsset ? &Widget->SoundFeedbackAsset->Sounds.PointerRejected : nullptr, TEXT("Pointer Rejected"));
		ValidateSoundFeedback(Widget->SoundOverrides.NavigationSelected, Widget->SoundFeedbackAsset ? &Widget->SoundFeedbackAsset->Sounds.NavigationSelected : nullptr, TEXT("Navigation Selected"));
		ValidateSoundFeedback(Widget->SoundOverrides.NavigationPressed, Widget->SoundFeedbackAsset ? &Widget->SoundFeedbackAsset->Sounds.NavigationPressed : nullptr, TEXT("Navigation Pressed"));
		ValidateSoundFeedback(Widget->SoundOverrides.NavigationClicked, Widget->SoundFeedbackAsset ? &Widget->SoundFeedbackAsset->Sounds.NavigationClicked : nullptr, TEXT("Navigation Clicked"));
		ValidateSoundFeedback(Widget->SoundOverrides.NavigationRejected, Widget->SoundFeedbackAsset ? &Widget->SoundFeedbackAsset->Sounds.NavigationRejected : nullptr, TEXT("Navigation Rejected"));

		auto ValidateHapticFeedback = [&AddMessage, Widget](
			const FLunarUIHapticOverride& Override,
			const FLunarUIHapticSpec* DataAssetEntry,
			const TCHAR* EventName)
		{
			if (Override.Mode != ELunarFeedbackOverrideMode::UseDataAsset)
			{
				return;
			}
			if (!IsValid(Widget->HapticFeedbackAsset))
			{
				AddMessage(
					ELunarConsoleMessageVerbosity::Warning,
					FName(*FString::Printf(TEXT("MissingHapticFeedbackAsset_%s"), EventName)),
					Widget->GetPathName(),
					FText::Format(
						LOCTEXT(
							"MissingHapticFeedbackAsset",
							"Widget '{0}' uses Data Asset haptic feedback for '{1}', but HapticFeedbackAsset is not assigned. Assign a haptic feedback asset, choose Custom/Use Global, or disable this event."),
						FText::FromString(Widget->GetPathName()),
						FText::FromString(EventName)));
				return;
			}
			if (!DataAssetEntry || !IsValid(DataAssetEntry->Effect))
			{
				AddMessage(
					ELunarConsoleMessageVerbosity::Warning,
					FName(*FString::Printf(TEXT("MissingHapticFeedbackEntry_%s"), EventName)),
					Widget->GetPathName(),
					FText::Format(
						LOCTEXT(
							"MissingHapticFeedbackEntry",
							"Haptic feedback asset '{0}' has no effect for '{1}', requested by widget '{2}'. Configure the entry, choose Custom/Use Global, or disable this event. Lunar intentionally does not fall back."),
						FText::FromString(Widget->HapticFeedbackAsset->GetPathName()),
						FText::FromString(EventName),
						FText::FromString(Widget->GetPathName())));
			}
		};
		ValidateHapticFeedback(Widget->HapticOverrides.NavigationSelected, Widget->HapticFeedbackAsset ? &Widget->HapticFeedbackAsset->Haptics.NavigationSelected : nullptr, TEXT("Navigation Selected"));
		ValidateHapticFeedback(Widget->HapticOverrides.NavigationPressed, Widget->HapticFeedbackAsset ? &Widget->HapticFeedbackAsset->Haptics.NavigationPressed : nullptr, TEXT("Navigation Pressed"));
		ValidateHapticFeedback(Widget->HapticOverrides.NavigationClicked, Widget->HapticFeedbackAsset ? &Widget->HapticFeedbackAsset->Haptics.NavigationClicked : nullptr, TEXT("Navigation Clicked"));
		ValidateHapticFeedback(Widget->HapticOverrides.NavigationRejected, Widget->HapticFeedbackAsset ? &Widget->HapticFeedbackAsset->Haptics.NavigationRejected : nullptr, TEXT("Navigation Rejected"));
	}

	ULunarNavigableWidget* ReachabilityRoot = nullptr;
	if (IsWidgetEligibleInGraphScope(InitialWidget, Scope))
	{
		ReachabilityRoot = InitialWidget;
	}
	else if (IsWidgetEligibleInGraphScope(InitialById, Scope))
	{
		ReachabilityRoot = InitialById;
	}
	else if (!EligibleWidgets.IsEmpty())
	{
		ReachabilityRoot = EligibleWidgets[0];
	}

	if (ReachabilityRoot)
	{
		TArray<ULunarNavigableWidget*> PendingReachability;
		TSet<TObjectKey<ULunarNavigableWidget>> ReachableWidgets;
		PendingReachability.Add(ReachabilityRoot);
		ReachableWidgets.Add(TObjectKey<ULunarNavigableWidget>(ReachabilityRoot));
		while (!PendingReachability.IsEmpty())
		{
			ULunarNavigableWidget* Current = PendingReachability.Pop(EAllowShrinking::No);
			for (const ELunarNavigationDirection Direction : Directions)
			{
				bool bBlocked = false;
				ULunarNavigableWidget* Target = ResolveNavigationTargetInScope(Current, Direction, Scope, bBlocked);
				if (!bBlocked
					&& IsWidgetEligibleInGraphScope(Target, Scope)
					&& !ReachableWidgets.Contains(TObjectKey<ULunarNavigableWidget>(Target)))
				{
					ReachableWidgets.Add(TObjectKey<ULunarNavigableWidget>(Target));
					PendingReachability.Add(Target);
				}
			}
		}

		for (ULunarNavigableWidget* Widget : EligibleWidgets)
		{
			if (!ReachableWidgets.Contains(TObjectKey<ULunarNavigableWidget>(Widget)))
			{
				AddMessage(
					ELunarConsoleMessageVerbosity::Warning,
					TEXT("UnreachableNavigationWidget"),
					Widget->GetPathName(),
					FText::Format(
						LOCTEXT(
							"UnreachableNavigationWidget",
							"Eligible widget '{0}' is unreachable from navigation start '{1}' through explicit, fallback, or wrap links."),
						FText::FromString(Widget->GetPathName()),
						FText::FromString(ReachabilityRoot->GetPathName())));
			}
		}
	}

	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	auto ResolveDefaultIconSet = [&AddGlobalMessage](
		const TSoftObjectPtr<ULunarInputIconSet>& IconSetReference,
		const TCHAR* FieldName) -> ULunarInputIconSet*
	{
		ULunarInputIconSet* IconSet = IconSetReference.LoadSynchronous();
		if (!IconSetReference.IsNull() && !IconSet)
		{
			const FString OwnerPath = IconSetReference.ToSoftObjectPath().ToString();
			AddGlobalMessage(
				ELunarConsoleMessageVerbosity::Error,
				TEXT("UnresolvedPromptIconSet"),
				OwnerPath,
				FText::Format(
					LOCTEXT(
						"UnresolvedPromptIconSet",
						"Default prompt icon-set field '{0}' cannot load asset '{1}'. Clear or repair the reference."),
					FText::FromString(FieldName),
					FText::FromString(OwnerPath)));
		}
		return IconSet;
	};
	ULunarInputIconSet* DefaultKeyboardIconSet = Settings
		? ResolveDefaultIconSet(
			Settings->Navigation.Prompts.DefaultKeyboardMouseIconSet,
			TEXT("DefaultKeyboardMouseIconSet"))
		: nullptr;
	ULunarInputIconSet* DefaultXboxIconSet = Settings
		? ResolveDefaultIconSet(
			Settings->Navigation.Prompts.DefaultXboxIconSet,
			TEXT("DefaultXboxIconSet"))
		: nullptr;
	ULunarInputIconSet* DefaultPlayStationIconSet = Settings
		? ResolveDefaultIconSet(
			Settings->Navigation.Prompts.DefaultPlayStation5IconSet,
			TEXT("DefaultPlayStation5IconSet"))
		: nullptr;
	for (ULunarNavigableWidget* Widget : Scope->RegisteredWidgets)
	{
		if (!IsValid(Widget) || !Widget->bEnableInputPrompt)
		{
			continue;
		}
		for (const FLunarPromptActionRequest& Request : Widget->PromptActions)
		{
			if (!Request.bEnabled)
			{
				continue;
			}
			if (!Request.ActionTag.IsValid())
			{
				AddMessage(
					ELunarConsoleMessageVerbosity::Error,
					TEXT("InvalidPromptActionTag"),
					Widget->GetPathName(),
					FText::Format(
						LOCTEXT(
							"InvalidPromptActionTag",
							"Prompt request on widget '{0}' has no valid semantic ActionTag."),
						FText::FromString(Widget->GetPathName())));
				continue;
			}

			const FLunarUIActionDefinition* Definition = FindResolvedActionDefinition(Request.ActionTag);
			if (!Definition)
			{
				AddMessage(
					ELunarConsoleMessageVerbosity::Error,
					TEXT("UnknownSemanticAction"),
					Widget->GetPathName(),
					FText::Format(
						LOCTEXT(
							"UnknownPromptSemanticAction",
							"Prompt request on widget '{0}' uses unknown or invalid semantic ActionTag '{1}'. Add one unique valid action definition."),
						FText::FromString(Widget->GetPathName()),
						FText::FromName(Request.ActionTag.GetTagName())));
				continue;
			}
			if (Request.bOverrideIcon)
			{
				if (!Request.IconOverride.GetResourceObject()
					&& Request.IconOverride.GetResourceName().IsNone())
				{
					AddMessage(
						ELunarConsoleMessageVerbosity::Error,
						TEXT("InvalidPromptIconOverride"),
						Widget->GetPathName(),
						FText::Format(
							LOCTEXT(
								"InvalidPromptIconOverrideValidation",
								"Prompt request for ActionTag '{0}' on widget '{1}' enables Icon Override, but its brush has no Texture or Material resource."),
							FText::FromName(Request.ActionTag.GetTagName()),
							FText::FromString(Widget->GetPathName())));
				}
				continue;
			}
			if (!Request.bRequireIcon)
			{
				continue;
			}

			TArray<FKey> KeysToValidate;
			if (Request.PreferredKey.IsValid())
			{
				KeysToValidate.Add(Request.PreferredKey);
			}
			else
			{
				for (const FLunarUIActionBinding& Binding : Definition->Bindings)
				{
					if (Binding.bEnabled && Binding.Key.IsValid() && !Binding.Key.IsTouch())
					{
						KeysToValidate.AddUnique(Binding.Key);
					}
				}
			}

			for (const FKey& Key : KeysToValidate)
			{
				if (Key.IsTouch())
				{
					continue;
				}
				TArray<ULunarInputIconSet*> IconSets;
				if (Widget->PromptIconSetOverride)
				{
					IconSets.Add(Widget->PromptIconSetOverride);
				}
				else if (Key.IsGamepadKey())
				{
					if (DefaultXboxIconSet)
					{
						IconSets.Add(DefaultXboxIconSet);
					}
					if (DefaultPlayStationIconSet)
					{
						IconSets.Add(DefaultPlayStationIconSet);
					}
				}
				else if (DefaultKeyboardIconSet)
				{
					IconSets.Add(DefaultKeyboardIconSet);
				}

				for (ULunarInputIconSet* IconSet : IconSets)
				{
					const int32 EntryCount = LunarNavigationSubsystem_Private::CountIconEntriesForKey(IconSet, Key);
					FSlateBrush ResolvedIcon;
					if (IconSet->ResolveIconForKey(Key, ResolvedIcon))
					{
						continue;
					}

					FName ErrorCode;
					FText ErrorText;
					if (EntryCount == 0)
					{
						ErrorCode = TEXT("MissingPromptIconEntry");
						ErrorText = FText::Format(
							LOCTEXT(
								"MissingPromptIconEntry",
								"Prompt icon set '{0}' has no entry for key '{1}' requested by ActionTag '{2}' on widget '{3}'."),
							FText::FromString(IconSet->GetPathName()),
							Key.GetDisplayName(),
							FText::FromName(Request.ActionTag.GetTagName()),
							FText::FromString(Widget->GetPathName()));
					}
					else if (EntryCount > 1)
					{
						ErrorCode = TEXT("DuplicatePromptIconEntry");
						ErrorText = FText::Format(
							LOCTEXT(
								"DuplicatePromptIconEntry",
								"Prompt icon set '{0}' has {4} entries for key '{1}' requested by ActionTag '{2}' on widget '{3}'. Keep exactly one."),
							FText::FromString(IconSet->GetPathName()),
							Key.GetDisplayName(),
							FText::FromName(Request.ActionTag.GetTagName()),
							FText::FromString(Widget->GetPathName()),
							FText::AsNumber(EntryCount));
					}
					else
					{
						ErrorCode = TEXT("InvalidPromptIconBrush");
						ErrorText = FText::Format(
							LOCTEXT(
								"InvalidPromptIconBrush",
								"Prompt icon set '{0}' maps key '{1}' for ActionTag '{2}' on widget '{3}', but the icon brush is unset. Assign a valid brush resource."),
							FText::FromString(IconSet->GetPathName()),
							Key.GetDisplayName(),
							FText::FromName(Request.ActionTag.GetTagName()),
							FText::FromString(Widget->GetPathName()));
					}
					AddMessage(
						ELunarConsoleMessageVerbosity::Error,
						ErrorCode,
						Widget->GetPathName(),
						MoveTemp(ErrorText));
				}
			}
		}
	}

	const int32 ActionMessageStartIndex = UnfilteredMessages.Num();
	UnfilteredMessages.Append(ActionDefinitionValidationMessages);
	for (int32 MessageIndex = ActionMessageStartIndex; MessageIndex < UnfilteredMessages.Num(); ++MessageIndex)
	{
		GlobalMessageIndices.Add(MessageIndex);
	}
	for (int32 MessageIndex = 0; MessageIndex < UnfilteredMessages.Num(); ++MessageIndex)
	{
		const FLunarNavigationValidationMessage& Message = UnfilteredMessages[MessageIndex];
		if (ShouldIncludeValidationMessage(Message))
		{
			Messages.Add(Message);
			ReportValidationMessage(Message, GlobalMessageIndices.Contains(MessageIndex) ? nullptr : Scope);
		}
	}
	Scope->LastValidationMessages = Messages;
	return Messages;
}

FString ULunarNavigationSubsystem::DumpActiveNavigationGraph()
{
	ULunarNavigationScope* Scope = GetActiveNavigationScope();
	if (!Scope)
	{
		const FString EmptyDump = TEXT("Lunar Navigation Graph\nNo active scope.");
		UE_LOG(LogLunarNavigation, Log, TEXT("%s"), *EmptyDump);
		ULunarConsoleSubsystem::AddMessage(
			FGameplayTag::RequestGameplayTag(TEXT("Lunar.Console"), false),
			ELunarConsoleMessageVerbosity::Info,
			EmptyDump);
		return EmptyDump;
	}

	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	const bool bIncludeCalculated = !Settings || Settings->Navigation.Diagnostics.bIncludeCalculatedLinksInGraphDump;
	const FString Dump = BuildNavigationGraphText(Scope, bIncludeCalculated);

	UE_LOG(LogLunarNavigation, Log, TEXT("%s"), *Dump);
	ULunarConsoleSubsystem::AddMessage(
		FGameplayTag::RequestGameplayTag(TEXT("Lunar.Console"), false),
		ELunarConsoleMessageVerbosity::Info,
		Dump);
	return Dump;
}

void ULunarNavigationSubsystem::SetNavigationDebugOverlayEnabled(const bool bEnabled)
{
	bDebugOverlayEnabled = bEnabled;
	if (bDebugOverlayEnabled)
	{
		UpdateNavigationDebugOverlay();
	}
	else
	{
		RemoveNavigationDebugOverlay();
	}
}

bool ULunarNavigationSubsystem::IsNavigationDebugOverlayEnabled() const
{
	return bDebugOverlayEnabled;
}

bool ULunarNavigationSubsystem::ShouldIncludeValidationMessage(
	const FLunarNavigationValidationMessage& Message) const
{
	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	const ELunarNavigationValidationLevel Level = Settings
		? Settings->Navigation.Diagnostics.ValidationLevel
		: ELunarNavigationValidationLevel::WarningsAndErrors;
	switch (Level)
	{
	case ELunarNavigationValidationLevel::Disabled:
		return false;
	case ELunarNavigationValidationLevel::ErrorsOnly:
		return LunarNavigationSubsystem_Private::IsErrorVerbosity(Message.Verbosity);
	case ELunarNavigationValidationLevel::WarningsAndErrors:
		return LunarNavigationSubsystem_Private::IsErrorVerbosity(Message.Verbosity)
			|| LunarNavigationSubsystem_Private::IsWarningVerbosity(Message.Verbosity);
	case ELunarNavigationValidationLevel::All:
	default:
		return true;
	}
}

void ULunarNavigationSubsystem::InvalidateNavigationScopeValidation(const ULunarNavigationScope* Scope)
{
	if (!Scope)
	{
		return;
	}

	const TObjectKey<ULunarNavigationScope> ScopeKey(const_cast<ULunarNavigationScope*>(Scope));
	if (const TSet<FString>* ScopeKeys = ReportedScopeValidationKeys.Find(ScopeKey))
	{
		for (const FString& Key : *ScopeKeys)
		{
			ReportedValidationKeys.Remove(Key);
		}
	}
	ReportedScopeValidationKeys.Remove(ScopeKey);
	const_cast<ULunarNavigationScope*>(Scope)->LastValidationMessages.Reset();
}

void ULunarNavigationSubsystem::ScheduleNavigationScopeValidation(ULunarNavigationScope* Scope)
{
	if (!IsValid(Scope))
	{
		return;
	}
	InvalidateNavigationScopeValidation(Scope);
	// Wait until at least the next engine frame so newly constructed UMG trees
	// complete a Slate prepass/paint before geometric reachability is evaluated.
	PendingValidationScopes.FindOrAdd(TWeakObjectPtr<ULunarNavigationScope>(Scope)) = GFrameCounter + 1;
}

void ULunarNavigationSubsystem::PollNavigationEligibilityChanges()
{
	TSet<TObjectKey<ULunarNavigableWidget>> ObservedWidgets;
	for (ULunarNavigationScope* Scope : ScopeStack)
	{
		if (!IsValid(Scope))
		{
			continue;
		}

		for (ULunarNavigableWidget* Widget : Scope->RegisteredWidgets)
		{
			if (!IsValid(Widget))
			{
				continue;
			}

			const TObjectKey<ULunarNavigableWidget> WidgetKey(Widget);
			ObservedWidgets.Add(WidgetKey);
			const bool bEligible = IsWidgetEligibleInGraphScope(Widget, Scope);
			if (bool* bPreviousEligibility = LastKnownWidgetEligibility.Find(WidgetKey))
			{
				if (*bPreviousEligibility != bEligible)
				{
					*bPreviousEligibility = bEligible;
					ScheduleNavigationScopeValidation(Scope);
				}
			}
			else
			{
				LastKnownWidgetEligibility.Add(WidgetKey, bEligible);
			}
		}
	}

	for (auto Iterator = LastKnownWidgetEligibility.CreateIterator(); Iterator; ++Iterator)
	{
		if (!ObservedWidgets.Contains(Iterator.Key()))
		{
			Iterator.RemoveCurrent();
		}
	}
}

void ULunarNavigationSubsystem::NotifyNavigableWidgetConfigurationChanged(
	ULunarNavigableWidget* Widget,
	const bool bPromptConfigurationChanged)
{
	if (!IsValid(Widget))
	{
		return;
	}
	if (bPromptConfigurationChanged)
	{
		ReportedPromptErrorKeys.Reset();
	}
	ScheduleNavigationScopeValidation(ResolveOwningScopeForWidget(Widget));
}

FString ULunarNavigationSubsystem::BuildNavigationGraphText(
	const ULunarNavigationScope* Scope,
	const bool bIncludeCalculatedLinks) const
{
	if (!Scope)
	{
		return TEXT("Lunar Navigation Graph\nNo active scope.");
	}

	FString Result = FString::Printf(
		TEXT("Lunar Navigation Graph\nLocalPlayer=%d ActiveScope=%s Root=%s Widgets=%d Selection=%s Validation=%d\n"),
		GetLocalPlayer() ? GetLocalPlayer()->GetLocalPlayerIndex() : INDEX_NONE,
		*GetPathNameSafe(Scope),
		*GetPathNameSafe(Scope->RootWidget),
		Scope->RegisteredWidgets.Num(),
		*GetPathNameSafe(SelectedWidget),
		Scope->LastValidationMessages.Num());

	Result += TEXT("ScopeStack=");
	for (int32 ScopeIndex = 0; ScopeIndex < ScopeStack.Num(); ++ScopeIndex)
	{
		Result += FString::Printf(
			TEXT("%s[%d]%s"),
			ScopeIndex > 0 ? TEXT(" -> ") : TEXT(""),
			ScopeIndex,
			*GetPathNameSafe(ScopeStack[ScopeIndex]));
	}
	Result += TEXT("\n");

	const ELunarNavigationDirection Directions[] =
	{
		ELunarNavigationDirection::Up,
		ELunarNavigationDirection::Down,
		ELunarNavigationDirection::Left,
		ELunarNavigationDirection::Right
	};
	for (ULunarNavigableWidget* Widget : Scope->RegisteredWidgets)
	{
		if (!IsValid(Widget))
		{
			continue;
		}

		Result += FString::Printf(
			TEXT("%s %s Id=%s Group=%s Priority=%d Eligible=%s Input=M:%s T:%s K:%s G:%s"),
			Widget == SelectedWidget ? TEXT(">") : TEXT("-"),
			*Widget->GetPathName(),
			*Widget->GetNavigationId().ToString(),
			*Widget->GetNavigationGroup().ToString(),
			Widget->GetNavigationPriority(),
			IsWidgetEligibleInGraphScope(Widget, Scope) ? TEXT("true") : TEXT("false"),
			Widget->IsMouseInputAllowed() ? TEXT("on") : TEXT("off"),
			Widget->IsTouchInputAllowed() ? TEXT("on") : TEXT("off"),
			Widget->IsKeyboardInputAllowed() ? TEXT("on") : TEXT("off"),
			Widget->IsGamepadInputAllowed() ? TEXT("on") : TEXT("off"));

		for (const ELunarNavigationDirection Direction : Directions)
		{
			const FLunarNavigationLink& Link = Widget->GetNavigationLink(Direction);
			Result += FString::Printf(TEXT(" %s="), LunarNavigationSubsystem_Private::GetDirectionName(Direction));
			switch (Link.Mode)
			{
			case ELunarNavigationLinkMode::Block:
				Result += TEXT("Block");
				break;
			case ELunarNavigationLinkMode::Widget:
				Result += FString::Printf(TEXT("Widget:%s"), *GetPathNameSafe(Link.Widget));
				break;
			case ELunarNavigationLinkMode::NavigationId:
				Result += FString::Printf(TEXT("Id:%s"), *Link.NavigationId.ToString());
				break;
			case ELunarNavigationLinkMode::Automatic:
			default:
				Result += TEXT("Automatic");
				break;
			}

			if (bIncludeCalculatedLinks && IsWidgetEligibleInGraphScope(Widget, Scope))
			{
				bool bBlocked = false;
				ULunarNavigableWidget* ResolvedTarget = ResolveNavigationTargetInScope(
					Widget,
					Direction,
					Scope,
					bBlocked);
				if (!bBlocked)
				{
					Result += FString::Printf(TEXT("->%s"), *GetPathNameSafe(ResolvedTarget));
				}
			}
		}
		Result += TEXT("\n");
	}
	return Result;
}

FString ULunarNavigationSubsystem::BuildNavigationDebugOverlayText() const
{
	FString NativeFocusOwner = TEXT("None");
	FString NativeSlateFocus = TEXT("None");
	TSharedPtr<SWidget> FocusedSlateWidget;
	if (FSlateApplication::IsInitialized() && GetLocalPlayer())
	{
		if (const TSharedPtr<const FSlateUser> SlateUser = GetLocalPlayer()->GetSlateUser())
		{
			FocusedSlateWidget = FSlateApplication::Get().GetUserFocusedWidget(SlateUser->GetUserIndex());
			if (FocusedSlateWidget.IsValid())
			{
				NativeSlateFocus = FocusedSlateWidget->GetTypeAsString();
			}
		}
	}

	if (DelegatedFocusOwner)
	{
		NativeFocusOwner = DelegatedFocusOwner->GetPathName();
	}
	else if (FocusedSlateWidget.IsValid())
	{
		for (ULunarNavigationScope* Scope : ScopeStack)
		{
			if (!Scope)
			{
				continue;
			}
			for (ULunarNavigableWidget* Widget : Scope->RegisteredWidgets)
			{
				if (Widget && Widget->GetCachedWidget() == FocusedSlateWidget)
				{
					NativeFocusOwner = Widget->GetPathName();
					break;
				}
			}
			if (NativeFocusOwner != TEXT("None"))
			{
				break;
			}
		}
		if (NativeFocusOwner == TEXT("None"))
		{
			NativeFocusOwner = TEXT("External/descendant");
		}
	}

	FString Header = FString::Printf(
		TEXT("[Lunar Navigation Debug] LocalPlayer=%d Device=%s Pointer=%s\nNativeFocusOwner=%s SlateFocus=%s\n"),
		GetLocalPlayer() ? GetLocalPlayer()->GetLocalPlayerIndex() : INDEX_NONE,
		*LunarNavigationSubsystem_Private::GetInputDeviceDisplayName(LastInputDevice),
		IsPointerPresentationActive() ? TEXT("true") : TEXT("false"),
		*NativeFocusOwner,
		*NativeSlateFocus);
	return Header + BuildNavigationGraphText(GetActiveNavigationScope(), true);
}

void ULunarNavigationSubsystem::UpdateNavigationDebugOverlay()
{
	if (!bInitialized || !bDebugOverlayEnabled || !GetLocalPlayer())
	{
		RemoveNavigationDebugOverlay();
		return;
	}

	UGameViewportClient* ViewportClient = GetWorld() ? GetWorld()->GetGameViewport() : nullptr;
	if (!ViewportClient)
	{
		RemoveNavigationDebugOverlay();
		return;
	}
	if (DebugOverlayWidget.IsValid() && DebugOverlayViewportClient.Get() == ViewportClient)
	{
		return;
	}

	RemoveNavigationDebugOverlay();
	const TWeakObjectPtr<ULunarNavigationSubsystem> WeakThis(this);
	TSharedRef<SBorder> Overlay = SNew(SBorder)
		.Visibility(EVisibility::HitTestInvisible)
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(FMargin(8.0f))
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.78f))
		[
			SNew(STextBlock)
			.Text_Lambda([WeakThis]()
			{
				return WeakThis.IsValid()
					? FText::FromString(WeakThis->BuildNavigationDebugOverlayText())
					: FText::GetEmpty();
			})
			.ColorAndOpacity(FLinearColor::White)
			.ShadowColorAndOpacity(FLinearColor::Black)
			.ShadowOffset(FVector2D(1.0f, 1.0f))
		];
	DebugOverlayWidget = Overlay;
	DebugOverlayViewportClient = ViewportClient;
	ViewportClient->AddViewportWidgetForPlayer(GetLocalPlayer(), Overlay, LunarNavigationSubsystem_Private::DebugOverlayZOrder);
}

void ULunarNavigationSubsystem::RemoveNavigationDebugOverlay()
{
	if (DebugOverlayWidget.IsValid())
	{
		if (UGameViewportClient* ViewportClient = DebugOverlayViewportClient.Get())
		{
			if (GetLocalPlayer())
			{
				ViewportClient->RemoveViewportWidgetForPlayer(GetLocalPlayer(), DebugOverlayWidget.ToSharedRef());
			}
		}
	}
	DebugOverlayWidget.Reset();
	DebugOverlayViewportClient.Reset();
}

void ULunarNavigationSubsystem::ReportValidationMessage(
	const FLunarNavigationValidationMessage& Message,
	const ULunarNavigationScope* Scope)
{
	if (!ShouldIncludeValidationMessage(Message))
	{
		return;
	}
	const FString DeduplicationKey = FString::Printf(
		TEXT("%s|%s|%s"),
		*Message.OwnerPath,
		*Message.Code.ToString(),
		*Message.Message.ToString());
	if (ReportedValidationKeys.Contains(DeduplicationKey))
	{
		return;
	}
	ReportedValidationKeys.Add(DeduplicationKey);
	if (Scope)
	{
		ReportedScopeValidationKeys
			.FindOrAdd(TObjectKey<ULunarNavigationScope>(const_cast<ULunarNavigationScope*>(Scope)))
			.Add(DeduplicationKey);
	}

	ULunarConsoleSubsystem::AddMessage(
		FGameplayTag::RequestGameplayTag(TEXT("Lunar.Console"), false),
		Message.Verbosity,
		Message.Message.ToString());
	if (LunarNavigationSubsystem_Private::IsErrorVerbosity(Message.Verbosity))
	{
		UE_LOG(LogLunarNavigation, Error, TEXT("[%s] %s"), *Message.Code.ToString(), *Message.Message.ToString());
	}
	else
	{
		UE_LOG(LogLunarNavigation, Warning, TEXT("[%s] %s"), *Message.Code.ToString(), *Message.Message.ToString());
	}
}

#undef LOCTEXT_NAMESPACE
