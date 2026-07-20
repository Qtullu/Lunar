// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarNavigationSubsystem.h
 * @brief Declares the per-local-player authority for Lunar UI navigation.
 * @ingroup LunarNavigationCore
 */

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Subsystems/RawInput/LunarRawInputTypes.h"
#include "UI/Navigation/Types/LunarInputPromptTypes.h"
#include "Tickable.h"
#include "UI/Navigation/Types/LunarNavigationTypes.h"
#include "UI/Navigation/Types/LunarUIFeedbackTypes.h"
#include "LunarNavigationSubsystem.generated.h"

class UWidget;
class APlayerController;
class UAudioComponent;
class UForceFeedbackEffect;
class UGameViewportClient;
class UInputComponent;
class SWidget;
class ULunarNavigableWidget;
class ULunarNavigationScope;
class ULunarScrollBox;
class ULunarInputIconSet;
struct FAnalogInputEvent;
struct FKeyEvent;
struct FPropertyChangedEvent;

/**
 * @brief Native notification for presentation-device or pointer-handoff changes.
 * The payload contains the newly active ELunarInputDeviceType followed by a
 * Boolean indicating whether controls should present pointer interaction state.
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(
	FOnLunarInputPresentationChanged,
	ELunarInputDeviceType /* InputDevice */,
	bool /* bPointerPresentationActive */);

/**
 * @brief Per-local-player authority for Lunar selection, scope routing, and UI input.
 * @ingroup LunarNavigationCore
 *
 * The subsystem owns the scope stack, deterministic graph resolution, semantic
 * action dispatch, native focus delegation, feedback, pointer policy, prompt
 * resolution, validation, and debug presentation for exactly one local player.
 */
UCLASS()
class LUNAR_API ULunarNavigationSubsystem : public ULocalPlayerSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	/** @param Collection Subsystem collection used to establish initialization dependencies. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	/** @brief Releases input, pointer, feedback, focus, and diagnostic state. */
	virtual void Deinitialize() override;

	/** @param DeltaTime Seconds since the previous subsystem tick. */
	virtual void Tick(float DeltaTime) override;
	/** @return Tick-stat identifier used by Unreal Insights and stat collection. */
	virtual TStatId GetStatId() const override;
	/** @return True while the subsystem is initialized and may tick. */
	virtual bool IsTickable() const override;

	/** @brief Activates a scope above the current scope. @param Scope Initialized scope to push. @return True when the scope became active. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation")
	bool PushNavigationScope(ULunarNavigationScope* Scope);

	/** @brief Removes the top scope or a specified scope and its children. @param Scope Scope to remove, or nullptr for the top scope. @return True when a scope was removed. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation")
	bool PopNavigationScope(ULunarNavigationScope* Scope = nullptr);

	/** @return Active top scope, or nullptr when the stack is empty. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation")
	ULunarNavigationScope* GetActiveNavigationScope() const;

	/** @return Copy of the scope stack ordered from bottom to top. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation")
	TArray<ULunarNavigationScope*> GetNavigationScopeStack() const;

	/** @brief Changes authoritative selection inside the active scope. @param Widget Eligible widget to select. @return True when selection is valid after the request. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation")
	bool SetSelectedWidget(ULunarNavigableWidget* Widget);

	/** @brief Selects an eligible widget by stable ID. @param NavigationId Stable ID to resolve. @return True when a matching widget was selected. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation")
	bool SetSelectedWidgetById(FName NavigationId);

	/** @return Current authoritative selection, or nullptr. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation")
	ULunarNavigableWidget* GetSelectedWidget() const;

	/** @brief Clears authoritative selection and native user focus. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation")
	void ClearSelection();

	/** @brief Resolves a fresh selection for the active scope. @return True when an eligible widget was selected. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation")
	bool ResetSelection();

	/** @brief Resolves a fresh selection for one scope. @param Scope Scope whose restoration state is reset. @return True when reset succeeds. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation")
	bool ResetSelectionForScope(ULunarNavigationScope* Scope);

	/** @brief Clears restoration state for every scope and resets the active selection. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation")
	void ResetAllSelections();

	/** @brief Resolves an explicit link, deterministic geometry fallback, and optional wrap. @param Direction Requested cardinal direction. @return True when navigation selected a target. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation")
	bool Navigate(ELunarNavigationDirection Direction);

	/** @brief Re-discovers hierarchy participants and rebuilds stable registration order. @param Scope Scope to refresh, or nullptr for the active scope. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation")
	void RefreshNavigationGraph(ULunarNavigationScope* Scope = nullptr);

	/** @brief Validates graph, actions, styles, prompts, and accessibility configuration. @param Scope Scope to validate. @return Current diagnostic snapshot. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation")
	TArray<FLunarNavigationValidationMessage> ValidateNavigationScope(ULunarNavigationScope* Scope);

	/** @return Human-readable graph dump for the active scope. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation")
	FString DumpActiveNavigationGraph();

	/** @param bEnabled Whether the per-player navigation debug overlay should be visible. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation")
	void SetNavigationDebugOverlayEnabled(bool bEnabled);

	/** @return True when the navigation debug overlay is enabled. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation")
	bool IsNavigationDebugOverlayEnabled() const;

	/** @return Last input device attributed to this local player. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation")
	ELunarInputDeviceType GetLastInputDevice() const;

	/**
	 * @brief Resolves the active left-stick direction retained by one control.
	 * @param Widget Expected owner of the handled analog press.
	 * @param OutDirection Receives the active cardinal stick direction.
	 * @param OutMagnitude Receives the current normalized stick magnitude.
	 * @return True only while Widget remains selected and owns the active analog press.
	 */
	bool GetActiveAnalogNavigationForWidget(
		const ULunarNavigableWidget* Widget,
		ELunarNavigationDirection& OutDirection,
		float& OutMagnitude) const;

	/** @brief Registers a constructed navigable control. @param Widget Control to register. @return True when registration is accepted. */
	bool RegisterNavigableWidget(ULunarNavigableWidget* Widget);
	/** @brief Removes a navigable control from every runtime registry. @param Widget Control to unregister. */
	void UnregisterNavigableWidget(ULunarNavigableWidget* Widget);
	/** @brief Registers a Lunar scroll container. @param ScrollBox Container to register. @return True when registration is accepted. */
	bool RegisterScrollBox(ULunarScrollBox* ScrollBox);
	/** @brief Removes a Lunar scroll container from runtime registries. @param ScrollBox Container to unregister. */
	void UnregisterScrollBox(ULunarScrollBox* ScrollBox);

	/** @brief Begins native focus ownership for a descendant. @param OwnerWidget Selected Lunar owner. @param NativeFocusWidget Descendant to focus. @return True when delegation begins. */
	bool BeginNativeFocusDelegation(ULunarNavigableWidget* OwnerWidget, UWidget* NativeFocusWidget);
	/** @brief Commits an owner's delegated editing session. @param OwnerWidget Expected delegation owner. @return True when committed. */
	bool CommitNativeFocusDelegation(ULunarNavigableWidget* OwnerWidget);
	/** @brief Cancels an owner's delegated editing session. @param OwnerWidget Expected delegation owner. @return True when cancelled. */
	bool CancelNativeFocusDelegation(ULunarNavigableWidget* OwnerWidget);
	/** @param OwnerWidget Optional owner filter. @return True when matching native focus delegation is active. */
	bool IsNativeFocusDelegationActive(const ULunarNavigableWidget* OwnerWidget = nullptr) const;

	/** @brief Processes one routed key-down event. @param KeyEvent Slate key event. @return True when UI consumed it. */
	bool HandleNavigationKeyDown(const FKeyEvent& KeyEvent);
	/** @brief Processes one routed key-up event. @param KeyEvent Slate key event. @return True when UI consumed it. */
	bool HandleNavigationKeyUp(const FKeyEvent& KeyEvent);
	/** @brief Processes one routed analog event. @param AnalogEvent Slate analog event. @return True when UI consumed it. */
	bool HandleNavigationAnalog(const FAnalogInputEvent& AnalogEvent);
	/** @brief Attributes pointer activity to this player and updates presentation. @param InputDevice Pointer-capable device. */
	void NotifyPointerInput(ELunarInputDeviceType InputDevice);
	/** @return True when the active scope requests all gameplay input to be blocked. */
	bool ShouldBlockAllGameplayInput() const;

public:
	/** Broadcast whenever authoritative selection changes. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation")
	FLunarSelectionChangedSignature OnSelectionChanged;

	/** Broadcast whenever the active top scope changes. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation")
	FLunarActiveScopeChangedSignature OnActiveScopeChanged;

	/** Broadcast when navigation or semantic action dispatch is rejected. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation")
	FLunarNavigationRejectedSignature OnNavigationRejected;

	/** Broadcast when the active presentation device changes. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation")
	FLunarInputDeviceChangedSignature OnInputDeviceChanged;

private:
	/** @brief Semantic action retained between physical press and release. */
	struct FPressedActionState
	{
		/** Widget that handled the press and should receive its matching release. */
		TWeakObjectPtr<ULunarNavigableWidget> OwnerWidget;
		/** Semantic action produced by the physical binding. */
		FGameplayTag ActionTag;
		/** Whether this action also carries a cardinal direction. */
		bool bHasDirection = false;
		/** Cardinal direction associated with the action when bHasDirection is true. */
		ELunarNavigationDirection Direction = ELunarNavigationDirection::Up;
		/** Range-selection modifier captured when the press was dispatched. */
		bool bRangeSelectionModifier = false;
		/** Additive-selection modifier captured when the press was dispatched. */
		bool bAdditiveSelectionModifier = false;
	};

	/** @param Widget Widget to resolve. @return Owning scope selected by override and hierarchy, or nullptr. */
	ULunarNavigationScope* ResolveOwningScopeForWidget(const ULunarNavigableWidget* Widget) const;
	/** @param Scope Scope to inspect. @param Widget Widget to test. @return True when registered in the scope. */
	bool ScopeContainsWidget(const ULunarNavigationScope* Scope, const ULunarNavigableWidget* Widget) const;
	/** @param Widget Widget to test. @param Scope Scope whose root defines membership. @return True when logically beneath the root. */
	bool IsWidgetWithinScopeRoot(const UWidget* Widget, const ULunarNavigationScope* Scope) const;
	/** @param Widget Candidate widget. @param Scope Expected active scope. @return True when selectable now. */
	bool IsWidgetEligibleInScope(const ULunarNavigableWidget* Widget, const ULunarNavigationScope* Scope) const;
	/** @param Widget Candidate widget. @param Scope Graph scope, active or inactive. @return True when graph-eligible. */
	bool IsWidgetEligibleInGraphScope(const ULunarNavigableWidget* Widget, const ULunarNavigationScope* Scope) const;
	/** @param Widget Candidate widget. @param Scope Graph scope. @param InputDevice Active keyboard or gamepad channel. @return True when graph-eligible for that channel. */
	bool IsWidgetEligibleForNavigationInput(
		const ULunarNavigableWidget* Widget,
		const ULunarNavigationScope* Scope,
		ELunarInputDeviceType InputDevice) const;
	/** @param RootWidget Hierarchy root. @param OutWidgets Receives all descendant Lunar controls. */
	void GatherNavigableDescendants(UWidget* RootWidget, TArray<ULunarNavigableWidget*>& OutWidgets) const;
	/** @param Widget Widget that needs a stable lifetime order. */
	void AssignRegistrationOrder(ULunarNavigableWidget* Widget);
	/** @param Widget Registered widget. @return Stable lifetime order, or zero when unregistered. */
	uint64 GetRegistrationOrder(const ULunarNavigableWidget* Widget) const;

	/** @param Scope Scope receiving selection. @param bAllowRestore Whether last selection may win. @return Best eligible initial widget. */
	ULunarNavigableWidget* ResolveInitialOrRestoredSelection(ULunarNavigationScope* Scope, bool bAllowRestore) const;
	/** @param Scope Scope to search. @return First eligible widget in stable order. */
	ULunarNavigableWidget* FindFirstEligibleWidget(ULunarNavigationScope* Scope) const;
	/** @param Scope Scope to search. @param Origin Geometry origin. @return Nearest eligible widget. */
	ULunarNavigableWidget* FindNearestEligibleWidget(ULunarNavigationScope* Scope, const FVector2D& Origin) const;
	/** @param Scope Scope to search. @param NavigationId Stable ID to match. @return Matching widget or nullptr. */
	ULunarNavigableWidget* FindWidgetById(ULunarNavigationScope* Scope, FName NavigationId) const;
	/** @param Current Source selection. @param Direction Direction to resolve. @param bOutBlocked Receives explicit-block status. @return Resolved target or nullptr. */
	ULunarNavigableWidget* ResolveNavigationTarget(ULunarNavigableWidget* Current, ELunarNavigationDirection Direction, bool& bOutBlocked) const;
	/** @param Current Source widget. @param Direction Direction to resolve. @param Scope Graph scope. @param bOutBlocked Receives explicit-block status. @return Resolved target or nullptr. */
	ULunarNavigableWidget* ResolveNavigationTargetInScope(
		ULunarNavigableWidget* Current,
		ELunarNavigationDirection Direction,
		const ULunarNavigationScope* Scope,
		bool& bOutBlocked) const;
	/** @param Current Source widget. @param Direction Direction to search. @param bWrap Whether opposite-edge wrap is allowed. @return Best geometric target. */
	ULunarNavigableWidget* FindGeometricTarget(ULunarNavigableWidget* Current, ELunarNavigationDirection Direction, bool bWrap) const;
	/** @param Current Source widget. @param Direction Direction to search. @param bWrap Whether wrap is allowed. @param Scope Graph scope. @return Best geometric target. */
	ULunarNavigableWidget* FindGeometricTargetInScope(
		ULunarNavigableWidget* Current,
		ELunarNavigationDirection Direction,
		bool bWrap,
		const ULunarNavigationScope* Scope,
		const ULunarScrollBox* RestrictToScrollBox = nullptr) const;
	/** @param Widget Descendant used to locate a container. @param bRequireConfinement Whether the container must trap navigation. @param bRequireMatchingOrientation Whether its orientation must match Direction. @param Direction Direction used by the optional orientation filter. @return Deepest matching registered container. */
	ULunarScrollBox* FindDeepestContainingScrollBox(
		const UWidget* Widget,
		bool bRequireConfinement,
		bool bRequireMatchingOrientation,
		ELunarNavigationDirection Direction) const;
	/** @param Widget Candidate interaction target. @return True when no active ScrollBox confinement excludes it. */
	bool IsWidgetAllowedByScrollConfinement(const ULunarNavigableWidget* Widget) const;
	/** @param ScrollBox Candidate direct-scroll owner. @return True when no active confinement excludes it. */
	bool IsScrollBoxAllowedByNavigationConfinement(const ULunarScrollBox* ScrollBox) const;
	/** @param Widget Pointer-selected target. @return True when the eligible target was selected after releasing an enclosing confinement when required. */
	bool SetSelectedWidgetFromPointer(ULunarNavigableWidget* Widget);
	/** @param bRestorePreviousSelection Whether to restore the widget selected before automatic entry. @return True when an active confinement was released. */
	bool ReleaseScrollNavigationConfinement(bool bRestorePreviousSelection);
	/** @param bUsePreviousGeometry Whether recovery should start near the lost selection. @return True when selection was recovered. */
	bool RecoverSelection(bool bUsePreviousGeometry);
	/** @brief Synchronizes Slate user focus with delegated focus or Lunar selection. */
	void SynchronizeNativeFocus();
	/** @brief Clears Slate focus for this local player's Slate user. */
	void ClearNativeUserFocus();

	/** @brief Loads and validates project action definitions into deterministic order. */
	void RebuildActionDefinitions();
	/** @param Key Physical key to resolve. @param OutActions Receives matching semantic actions in dispatch order. */
	void GetBoundActions(const FKey& Key, TArray<FGameplayTag>& OutActions);
	/** @param OwnerWidget Prompt owner. @param PromptReceiverClass Receiver contract class. @param Requests Requested actions. @param IconSetOverride Optional icon override. @param OutActions Receives resolved prompt entries. */
	void ResolveInputPromptActions(
		const ULunarNavigableWidget* OwnerWidget,
		UClass* PromptReceiverClass,
		const TArray<FLunarPromptActionRequest>& Requests,
		ULunarInputIconSet* IconSetOverride,
		TArray<FLunarResolvedPromptAction>& OutActions);
	/** @param OwnerWidget Prompt owner. @param PromptReceiverClass Invalid receiver class. */
	void ReportInputPromptReceiverClassError(
		const ULunarNavigableWidget* OwnerWidget,
		UClass* PromptReceiverClass);
	/** @param ActionTag Semantic tag to locate. @return Resolved action definition or nullptr. */
	const FLunarUIActionDefinition* FindResolvedActionDefinition(FGameplayTag ActionTag) const;
	/** @param InputDevice Active device. @return Loaded default icon set for the device, or nullptr. */
	ULunarInputIconSet* ResolveDefaultPromptIconSet(ELunarInputDeviceType InputDevice) const;
	/** @param DeduplicationKey Stable error key. @param Message Actionable console message. */
	void ReportPromptConfigurationError(const FString& DeduplicationKey, const FText& Message);
	/** @param Object Changed settings or navigation object. @param PropertyChangedEvent Editor property-change metadata. */
	void HandlePromptConfigurationObjectChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent);
	/** @param UserId Platform user whose hardware changed. @param InputDeviceId New device identifier. */
	void HandleInputHardwareDeviceChanged(FPlatformUserId UserId, FInputDeviceId InputDeviceId);
	/** @param bResetPromptErrors Whether diagnostic deduplication should also be reset. */
	void InvalidateInputPromptPresentation(bool bResetPromptErrors);
	/** @param KeyEvent Key event to attribute. @return True when it belongs to this local player. */
	bool IsEventForOwningLocalPlayer(const FKeyEvent& KeyEvent) const;
	/** @param AnalogEvent Analog event to attribute. @return True when it belongs to this local player. */
	bool IsEventForOwningLocalPlayer(const FAnalogInputEvent& AnalogEvent) const;
	/** @param Key Physical key. @param InputEvent Press/release kind. @param bIsRepeat Whether dispatch is repeated. @param AnalogMagnitude Analog magnitude. @param OutPressedAction Optional retained press state. @param bOutHandledByWidget Optional widget-handled flag. @return True when UI consumed the action. */
	bool DispatchPhysicalAction(const FKey& Key, EInputEvent InputEvent, bool bIsRepeat, float AnalogMagnitude,
		FPressedActionState* OutPressedAction = nullptr, bool* bOutHandledByWidget = nullptr);
	/** @param ActionContext Semantic action to deliver. @return Selected-widget handling result. */
	ELunarUIActionResult DispatchActionToSelected(const FLunarUIActionContext& ActionContext);
	/** @param ActionContext Rejected action broadcast to observers. */
	void BroadcastRejected(const FLunarUIActionContext& ActionContext);
	/** @return True when controls should use pointer presentation. */
	bool IsPointerPresentationActive() const;
	/** @return True while an intentional pointer-selected external Slate text editor owns keyboard input. */
	bool IsExternalNativeTextInputActive() const;
	/** @return Native multicast used by constructed controls. */
	FOnLunarInputPresentationChanged& OnInputPresentationChangedNative() { return InputPresentationChangedNative; }
	/** @param SoundSpec Sound configuration to play for this local player. */
	void PlayUISound(const FLunarUISoundSpec& SoundSpec);
	/** @param HapticSpec Force-feedback configuration to play. */
	void PlayUIHaptic(const FLunarUIHapticSpec& HapticSpec);
	/** @brief Stops the active navigation haptic effect. */
	void StopUIHaptic();
	/** @brief Builds and starts the ancestor scroll chain for current selection. */
	void ScrollSelectionIntoView();
	/** @param ScrollBox Container receiving direct input that must stop the chain. */
	void CancelSelectionScrollChainForDirectInput(ULunarScrollBox* ScrollBox);
	/** @brief Advances reveal-selection scrolling to the next ancestor container. */
	void AdvanceSelectionScrollChain();
	/** @param bCancelActiveScroll Whether the current container animation is cancelled. @return New chain generation. */
	uint64 ResetSelectionScrollChain(bool bCancelActiveScroll);
	/** @param ScrollBox Container that completed. @param bCompleted Whether it reached its target. */
	void HandleSelectionScrollFinishedNative(ULunarScrollBox* ScrollBox, bool bCompleted);
	/** @param InputDevice Newly active device. @param bPointerInput Whether activity came from a pointer. @param InputDeviceId Hardware identifier when known. */
	void SetLastInputDeviceInternal(
		ELunarInputDeviceType InputDevice,
		bool bPointerInput,
		FInputDeviceId InputDeviceId = INPUTDEVICEID_NONE);
	/** @return Pointer policy contributed by the active scope. */
	FLunarPointerPolicy ResolveActivePointerPolicy() const;
	/** @brief Captures gameplay and viewport pointer state before first scope policy is applied. */
	void CapturePreNavigationPointerState();
	/** @param bReleaseExistingCapture Whether Slate pointer capture should be released first. */
	void ApplyActiveScopePointerPolicy(bool bReleaseExistingCapture = false);
	/** @brief Restores gameplay and viewport pointer state captured before navigation. */
	void RestorePreNavigationPointerState();
	/** @brief Installs or removes the gameplay input blocker required by the active scope. */
	void UpdateGameplayInputBlocking();
	/** @brief Removes the subsystem-owned gameplay input component. */
	void RemoveGameplayInputBlocker();
	/** @param CurrentTimeSeconds Current monotonic time used by repeat schedules. */
	void TickNavigationRepeat(double CurrentTimeSeconds);
	/** @param DirectionKey Physical direction key being repeated. @return Effective repeat settings. */
	FLunarNavigationRepeatSettings ResolveRepeatSettingsForSelected(const FKey& DirectionKey);
	/** @brief Sends release for the retained analog action and clears it. */
	void ReleaseAnalogPressedAction();
	/** @brief Clears digital and analog direction-repeat state. */
	void ResetRepeatState();
	/** @brief Removes completed transient UI audio components. */
	void PruneFinishedUISounds();
	/** @brief Stops and releases all transient UI sounds. */
	void StopAllUISounds();
	/** @param Scope Scope whose registered scroll animations must stop. */
	void CancelScrollsForScope(const ULunarNavigationScope* Scope);
	/** @brief Cancels animations in all registered Lunar scroll boxes. */
	void CancelAllRegisteredScrolls();
	/** @param Direction Cardinal direction. @return Canonical gamepad analog direction key. */
	FKey GetAnalogDirectionKey(ELunarNavigationDirection Direction) const;
	/** @param Magnitude Current stick magnitude. @param CurrentTimeSeconds Monotonic time. @param bOutDispatched Receives whether an action was dispatched. @return True when direction remains active. */
	bool UpdateAnalogDirection(float Magnitude, double CurrentTimeSeconds, bool& bOutDispatched);

	/** @param Widget Widget whose cached geometry is queried. @return Absolute center position, or zero when unavailable. */
	FVector2D GetWidgetCenter(const UWidget* Widget) const;
	/** @param Direction Cardinal direction. @return Unit vector for geometry scoring. */
	FVector2D GetDirectionVector(ELunarNavigationDirection Direction) const;
	/** @param Scope Scope settings owner. @param GroupId Source group. @return True when fallback may cross the group boundary. */
	bool CanFallbackCrossGroup(const ULunarNavigationScope* Scope, FName GroupId) const;
	/** @param Direction Direction to inspect. @return True for Left or Right. */
	bool IsHorizontalDirection(ELunarNavigationDirection Direction) const;
	/** @param Message Diagnostic candidate. @return True when active validation level includes it. */
	bool ShouldIncludeValidationMessage(const FLunarNavigationValidationMessage& Message) const;
	/** @param Scope Scope whose retained validation snapshot is stale. */
	void InvalidateNavigationScopeValidation(const ULunarNavigationScope* Scope);
	/** @param Scope Scope to validate after geometry and hierarchy settle. */
	void ScheduleNavigationScopeValidation(ULunarNavigationScope* Scope);
	/** @brief Detects eligibility changes that do not tick their hidden widget. */
	void PollNavigationEligibilityChanges();
	/** @param Widget Widget whose configuration changed. @param bPromptConfigurationChanged Whether prompt diagnostics must reset. */
	void NotifyNavigableWidgetConfigurationChanged(
		ULunarNavigableWidget* Widget,
		bool bPromptConfigurationChanged);
	/** @param Message Diagnostic to log and retain. @param Scope Optional owning scope. */
	void ReportValidationMessage(
		const FLunarNavigationValidationMessage& Message,
		const ULunarNavigationScope* Scope = nullptr);
	/** @param Scope Scope to serialize. @param bIncludeCalculatedLinks Whether to append resolved target links. @return Human-readable graph text. */
	FString BuildNavigationGraphText(const ULunarNavigationScope* Scope, bool bIncludeCalculatedLinks) const;
	/** @return Human-readable debug overlay text for current player state. */
	FString BuildNavigationDebugOverlayText() const;
	/** @brief Creates or refreshes the per-player debug overlay widget. */
	void UpdateNavigationDebugOverlay();
	/** @brief Removes the debug overlay from its viewport. */
	void RemoveNavigationDebugOverlay();

private:
	/** Navigation scopes ordered from bottom to active top. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<ULunarNavigationScope>> ScopeStack;

	/** Authoritative selected widget in the active scope. */
	UPROPERTY(Transient)
	TObjectPtr<ULunarNavigableWidget> SelectedWidget;

	/** Lunar control that owns explicit native focus delegation. */
	UPROPERTY(Transient)
	TObjectPtr<ULunarNavigableWidget> DelegatedFocusOwner;

	/** Native descendant currently receiving delegated Slate focus. */
	UPROPERTY(Transient)
	TObjectPtr<UWidget> DelegatedFocusWidget;

	/** High-priority input component installed while gameplay input is blocked. */
	UPROPERTY(Transient)
	TObjectPtr<UInputComponent> GameplayBlockInputComponent;

	/** Constructed controls awaiting an owning scope. */
	TArray<TWeakObjectPtr<ULunarNavigableWidget>> PendingNavigableWidgets;
	/** Scopes scheduled for validation and their earliest validation frame. */
	TMap<TWeakObjectPtr<ULunarNavigationScope>, uint64> PendingValidationScopes;
	/** Live Lunar scroll containers available to reveal selection. */
	TSet<TWeakObjectPtr<ULunarScrollBox>> RegisteredScrollBoxes;
	/** Ordered ancestor containers remaining in the active reveal-selection chain. */
	TArray<TWeakObjectPtr<ULunarScrollBox>> PendingSelectionScrollBoxes;
	/** Selection for which the current scroll chain was created. */
	TWeakObjectPtr<ULunarNavigableWidget> SelectionScrollTarget;
	/** Container currently animating within the scroll chain. */
	TWeakObjectPtr<ULunarScrollBox> ActiveSelectionScrollBox;
	/** Subscription to completion of the active scroll container. */
	FDelegateHandle ActiveSelectionScrollFinishedHandle;
	/** ScrollBox currently trapping directional and pointer selection. */
	TWeakObjectPtr<ULunarScrollBox> ActiveScrollNavigationConfinement;
	/** Selection restored when the active ScrollBox confinement exits. */
	TWeakObjectPtr<ULunarNavigableWidget> ScrollNavigationReturnWidget;

	/** Transient audio components playing UI feedback for this local player. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UAudioComponent>> ActiveUISoundComponents;

	/** Force-feedback asset currently playing as UI haptics. */
	TWeakObjectPtr<UForceFeedbackEffect> ActiveUIHapticEffect;
	/** Player controller on which the active UI haptic is playing. */
	TWeakObjectPtr<APlayerController> ActiveUIHapticPlayerController;
	/** Native presentation-change multicast consumed by registered controls. */
	FOnLunarInputPresentationChanged InputPresentationChangedNative;
	/** Stable lifetime order assigned to each registered navigable widget. */
	TMap<TObjectKey<ULunarNavigableWidget>, uint64> RegistrationOrders;
	/** Last polled graph eligibility used to detect hidden-widget changes. */
	TMap<TObjectKey<ULunarNavigableWidget>, bool> LastKnownWidgetEligibility;
	/** Last selected object retained separately for each scope. */
	TMap<TObjectKey<ULunarNavigationScope>, TWeakObjectPtr<ULunarNavigableWidget>> LastSelectionWidgets;
	/** Action definitions resolved from the project registry in dispatch order. */
	TArray<FLunarUIActionDefinition> ResolvedActionDefinitions;
	/** Diagnostics produced while resolving the action registry. */
	TArray<FLunarNavigationValidationMessage> ActionDefinitionValidationMessages;
	/** Global diagnostic keys already reported to Lunar Console. */
	TSet<FString> ReportedValidationKeys;
	/** Per-scope diagnostic keys already reported to Lunar Console. */
	TMap<TObjectKey<ULunarNavigationScope>, TSet<FString>> ReportedScopeValidationKeys;
	/** Prompt-configuration error keys already reported. */
	TSet<FString> ReportedPromptErrorKeys;

	/** Input channel temporarily filtering navigation candidates during physical dispatch. */
	ELunarInputDeviceType NavigationDispatchInputDevice = ELunarInputDeviceType::Unknown;

	/** Physical keys whose release must remain consumed after a handled press. */
	TSet<FKey> ConsumedKeyUps;
	/** Digital keys currently held by this local player. */
	TSet<FKey> HeldDigitalKeys;
	/** Keyboard and gamepad keys currently acting as multi-selection modifiers. */
	TSet<FKey> HeldSelectionModifierKeys;
	/** Semantic press state retained by physical key. */
	TMap<FKey, FPressedActionState> PressedActions;
	/** Digital direction key that currently owns repeat timing. */
	FKey DigitalRepeatKey = EKeys::Invalid;
	/** Monotonic timestamp of the next digital repeat. */
	double NextDigitalRepeatTime = 0.0;

	/** Latest two-axis navigation vector. */
	FVector2D AnalogVector = FVector2D::ZeroVector;
	/** Whether analog input has crossed the activation threshold. */
	bool bAnalogDirectionActive = false;
	/** Quantized active analog direction. */
	ELunarNavigationDirection AnalogDirection = ELunarNavigationDirection::Up;
	/** Magnitude of the active analog direction. */
	float ActiveAnalogMagnitude = 0.0f;
	/** Monotonic timestamp of the next analog repeat. */
	double NextAnalogRepeatTime = 0.0;
	/** Semantic press state retained for the active analog direction. */
	FPressedActionState AnalogPressedAction;
	/** Whether AnalogPressedAction currently requires a matching release. */
	bool bHasAnalogPressedAction = false;

	/** Last presentation device attributed to this player. */
	ELunarInputDeviceType LastInputDevice = ELunarInputDeviceType::Unknown;
	/** Platform input-device identifier associated with LastInputDevice. */
	FInputDeviceId LastInputDeviceId = INPUTDEVICEID_NONE;
	/** Hardware class used to choose gamepad prompt family. */
	FName LastInputHardwareClass = NAME_None;
	/** Hardware identifier used to choose gamepad prompt family. */
	FName LastInputHardwareIdentifier = NAME_None;
	/** Culture name used to detect prompt-label localization changes. */
	FString LastObservedCultureName;
	/** Editor property-change subscription for live prompt invalidation. */
	FDelegateHandle PromptConfigurationChangedHandle;
	/** Platform hardware-change subscription. */
	FDelegateHandle InputHardwareDeviceChangedHandle;
	/** Controller whose gameplay pointer flags were captured. */
	TWeakObjectPtr<APlayerController> PointerStatePlayerController;
	/** Controller that currently owns GameplayBlockInputComponent. */
	TWeakObjectPtr<APlayerController> GameplayBlockPlayerController;
	/** Viewport whose pointer capture and lock state were captured. */
	TWeakObjectPtr<UGameViewportClient> PointerStateViewportClient;
	/** Viewport containing the current debug overlay. */
	TWeakObjectPtr<UGameViewportClient> DebugOverlayViewportClient;
	/** Slate widget used to display per-player graph diagnostics. */
	TSharedPtr<SWidget> DebugOverlayWidget;
	/** Next monotonically increasing widget registration order. */
	uint64 NextRegistrationOrder = 1;
	/** Whether the per-player graph overlay is requested. */
	bool bDebugOverlayEnabled = false;
	/** Whether pointer interaction currently determines control presentation. */
	bool bPointerPresentationActive = false;
	/** Whether a hardware cursor is currently available for presentation. */
	bool bHardwareCursorPresentationActive = false;
	/** Gameplay controller cursor visibility captured before navigation policy. */
	bool bSavedShowMouseCursor = false;
	/** Gameplay click-event flag captured before navigation policy. */
	bool bSavedEnableClickEvents = false;
	/** Gameplay touch-event flag captured before navigation policy. */
	bool bSavedEnableTouchEvents = false;
	/** Gameplay mouse-over flag captured before navigation policy. */
	bool bSavedEnableMouseOverEvents = false;
	/** Gameplay touch-over flag captured before navigation policy. */
	bool bSavedEnableTouchOverEvents = false;
	/** Whether gameplay pointer flags have a valid restoration snapshot. */
	bool bHasGameplayPointerFlagsSnapshot = false;
	/** Whether viewport pointer capture and lock have a valid restoration snapshot. */
	bool bHasPointerStateSnapshot = false;
	/** Guard suppressing automatic selection recovery during explicit graph mutation. */
	bool bSuppressGraphSelectionRecovery = false;
	/** Reentrancy guard for push/pop operations and callbacks. */
	bool bScopeStackMutationInProgress = false;
	/** Guard allowing the stored return selection to cross the active ScrollBox boundary. */
	bool bReleasingScrollNavigationConfinement = false;
	/** Whether the scroll chain should resume on a later tick. */
	bool bSelectionScrollAdvancePending = false;
	/** Reentrancy guard for scroll-chain advancement. */
	bool bAdvancingSelectionScrollChain = false;
	/** Frames spent waiting for valid geometry before abandoning the scroll target. */
	uint8 SelectionScrollGeometryRetryCount = 0;
	/** Earliest engine frame on which the scroll chain may resume. */
	uint64 SelectionScrollResumeFrame = 0;
	/** Generation token invalidating stale asynchronous scroll completion callbacks. */
	uint64 SelectionScrollGeneration = 0;
	/** Whether Initialize completed and ticking is permitted. */
	bool bInitialized = false;

	friend class ULunarNavigableWidget; ///< Registers controls and consumes presentation and feedback services.
	friend class ULunarScrollBox; ///< Coordinates selection reveal and direct-input cancellation.
};
