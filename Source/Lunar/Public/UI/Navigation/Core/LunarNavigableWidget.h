// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarNavigableWidget.h
 * @brief Declares the shared navigation, presentation, feedback, prompt, and accessibility control base.
 * @ingroup LunarNavigationCore
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Navigation/Types/LunarInputPromptTypes.h"
#include "UI/Navigation/Types/LunarNavigationTypes.h"
#include "UI/Navigation/Types/LunarUIFeedbackTypes.h"
#include "LunarNavigableWidget.generated.h"

class ULunarInputIconSet;
class ULunarNavigationScope;
class ULunarNavigationSubsystem;
class SBox;
class ULunarInputPromptWidget;
class ULunarUISoundFeedbackAsset;
class ULunarUIHapticFeedbackAsset;

/**
 * @brief Shared selection, presentation, feedback, prompt, pointer, and accessibility contract for Lunar controls.
 * @ingroup LunarNavigationCore
 *
 * Concrete Lunar controls inherit this class to join a scope graph, receive
 * semantic actions, publish visual state, and expose consistent input
 * feedback and accessibility behavior.
 */
UCLASS(Abstract, Blueprintable, HideFunctions = (GetIsEnabled, SetIsEnabled))
class LUNAR_API ULunarNavigableWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** @brief Creates a navigable control and enables native focus required for Slate user focus. @param ObjectInitializer Unreal object initializer. */
	ULunarNavigableWidget(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	/** @return Shared UMG Palette category for placeable Lunar navigation controls. */
	virtual const FText GetPaletteCategory() override;
#endif

	/** @brief Requests authoritative selection from the owning subsystem. @return True when this widget becomes selected. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation")
	bool RequestLunarSelection();

	/** @return True when this widget is the subsystem's authoritative selection. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation")
	bool IsLunarSelected() const;

	/** @return True when current configuration and runtime state permit Lunar selection. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation")
	bool CanReceiveLunarSelection() const;

	/** @return True when this widget accepts mouse hover, selection, and activation. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation|Input")
	bool IsMouseInputAllowed() const { return bCanInteractWithPointer; }

	/** @return True when this widget accepts touch selection and activation. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation|Input")
	bool IsTouchInputAllowed() const { return bAllowTouchInput; }

	/** @return True when this widget participates in keyboard navigation and actions. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation|Input")
	bool IsKeyboardInputAllowed() const { return bAllowKeyboardInput; }

	/** @return True when this widget participates in gamepad navigation and actions. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation|Input")
	bool IsGamepadInputAllowed() const { return bAllowGamepadInput; }

	/** @param InputDevice Navigation device to test; KeyboardMouse means the keyboard channel here. @return True when that navigation channel is allowed. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation|Input")
	bool IsNavigationInputAllowed(ELunarInputDeviceType InputDevice) const;

	/** @return True when Lunar permits this control and its descendants to activate. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation")
	bool IsLunarEnabled() const { return bLunarEnabled; }

	/** @brief Changes logical Lunar availability without invoking Slate's native disabled tint or pointer suppression. @param bEnabled New logical availability. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation")
	void SetLunarEnabled(bool bEnabled);

	/** @brief Compatibility bridge that routes native Set Is Enabled calls to SetLunarEnabled while Slate remains presentation-enabled. @param bInIsEnabled New logical Lunar availability. */
	virtual void SetIsEnabled(bool bInIsEnabled) override;

	/** @brief Activates this control or emits Rejected feedback when it is unavailable. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation")
	void ActivateLunarWidget();

	/** @brief Evaluates native and Blueprint action policy. @param ActionContext Semantic action to test. @return True when the control can handle the action. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation")
	bool CanHandleLunarAction(const FLunarUIActionContext& ActionContext) const;

	/** @brief Dispatches a semantic action through native and Blueprint hooks. @param ActionContext Semantic action to dispatch. @return Handling result. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation")
	ELunarUIActionResult HandleLunarAction(const FLunarUIActionContext& ActionContext);

	/** @brief Resolves and publishes the visual state implied by current interaction, value, and Designer preview state. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|Presentation")
	void RefreshVisualState();

	/** @brief Replaces all prompt action requests. @param NewPromptActions New ordered request set. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|Prompts")
	void SetPromptActions(const TArray<FLunarPromptActionRequest>& NewPromptActions);

	/** @brief Adds one prompt request when not already present. @param PromptAction Request to append. @return True when the request was added. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|Prompts")
	bool AddPromptAction(const FLunarPromptActionRequest& PromptAction);

	/** @brief Removes requests for one semantic action. @param ActionTag Action tag to remove. @return True when at least one request was removed. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|Prompts")
	bool RemovePromptAction(FGameplayTag ActionTag);

	/** @brief Delegates Slate user focus to a native descendant. @param NativeFocusWidget Descendant that should receive focus. @return True when delegation began. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|Focus")
	bool BeginNativeFocusDelegation(UWidget* NativeFocusWidget);

	/** @brief Commits the active native-focus editing session. @return True when this widget owned a session that was committed. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|Focus")
	bool CommitNativeFocusDelegation();

	/** @brief Cancels the active native-focus editing session and restores captured text. @return True when this widget owned a session that was cancelled. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|Focus")
	bool CancelNativeFocusDelegation();

	/** @return True when this widget owns the subsystem's active native-focus delegation. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation|Focus")
	bool IsNativeFocusDelegationActive() const;

	/** @return Stable ID used by explicit links and restoration. */
	FName GetNavigationId() const { return NavigationId; }
	/** @return Navigation group used to constrain geometric fallback. */
	FName GetNavigationGroup() const { return NavigationGroup; }
	/** @return Priority used as a deterministic navigation tie breaker. */
	int32 GetNavigationPriority() const { return NavigationPriority; }
	/** @return Explicit scope override, or nullptr for hierarchy-based scope resolution. */
	ULunarNavigationScope* GetNavigationScopeOverride() const { return NavigationScopeOverride; }
	/** @param Direction Cardinal direction to inspect. @return Configured explicit link for that direction. */
	const FLunarNavigationLink& GetNavigationLink(ELunarNavigationDirection Direction) const;

	/** @brief Derives a control value action from a physical direction. @param Direction Direction received. @param OutActionTag Resolved action tag. @return True when the control consumes the direction as a value action. */
	bool ResolveDirectionalLunarControlAction(
		ELunarNavigationDirection Direction,
		FGameplayTag& OutActionTag) const;

	/** @brief Resolves a control-specific navigation repeat policy. @param OutRepeatSettings Receives the override. @return True when the override should replace global timing. */
	bool GetLunarRepeatSettingsOverride(FLunarNavigationRepeatSettings& OutRepeatSettings) const;

public:
	/** Broadcast when this widget becomes the authoritative Lunar selection. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation|Events") FLunarNavigableWidgetEventSignature OnLunarSelected;
	/** Broadcast when authoritative selection leaves this widget. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation|Events") FLunarNavigableWidgetEventSignature OnLunarUnselected;
	/** Broadcast when a pointer or semantic press begins. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation|Events") FLunarNavigableWidgetEventSignature OnLunarPressed;
	/** Broadcast immediately and every tick of a valid hold, including local-delay state. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation|Events") FLunarHoldProgressSignature OnLunarHoldProgress;
	/** Broadcast when the active press ends or is cancelled. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation|Events") FLunarNavigableWidgetEventSignature OnLunarReleased;
	/** Broadcast after this control accepts activation. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation|Events") FLunarNavigableWidgetEventSignature OnLunarActivated;
	/** Broadcast after an activation or action is rejected. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation|Events") FLunarNavigableWidgetEventSignature OnLunarRejected;
	/** Broadcast when the owning player's active presentation device changes. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation|Events") FLunarNavigableInputDeviceChangedSignature OnLunarInputDeviceChanged;
	/** Broadcast after the resolved Lunar visual state changes. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation|Events") FLunarVisualStateChangedSignature OnLunarVisualStateChanged;

	/** Whether this control may participate in the navigation graph. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation") bool bCanReceiveLunarSelection = false;
	/** Whether mouse hover, selection, and activation are accepted. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Input", meta = (DisplayName = "Allow Mouse Input")) bool bCanInteractWithPointer = true;
	/** Whether touch selection and activation are accepted. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Input", meta = (DisplayName = "Allow Touch Input")) bool bAllowTouchInput = true;
	/** Whether keyboard navigation and semantic actions are accepted. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Input", meta = (DisplayName = "Allow Keyboard Input")) bool bAllowKeyboardInput = true;
	/** Whether gamepad navigation and semantic actions are accepted. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Input", meta = (DisplayName = "Allow Gamepad Input")) bool bAllowGamepadInput = true;
	/** Logical availability owned by Lunar; disabled controls retain exact authored styles and optional inspection input. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation", meta = (DisplayName = "Lunar Enabled")) bool bLunarEnabled = true;
	/** Per-widget delay before HoldSeconds begins accumulating; Hold Progress itself starts immediately after Pressed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Hold", meta = (ClampMin = "0.0", UIMin = "0.0", Units = "s")) float HoldStartDelay = 0.0f;
	/** Whether a disabled widget remains eligible for navigation selection. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation") bool bCanReceiveSelectionWhenDisabled = false;
	/** Stable identifier used by explicit links, initial selection, and restoration. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation") FName NavigationId = NAME_None;
	/** Optional group identifier used by geometric navigation fallback. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation") FName NavigationGroup = NAME_None;
	/** Deterministic tie-break priority; higher values are preferred. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation") int32 NavigationPriority = 0;
	/** Explicit or automatic upward navigation policy. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation") FLunarNavigationLink UpLink;
	/** Explicit or automatic downward navigation policy. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation") FLunarNavigationLink DownLink;
	/** Explicit or automatic leftward navigation policy. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation") FLunarNavigationLink LeftLink;
	/** Explicit or automatic rightward navigation policy. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation") FLunarNavigationLink RightLink;
	/** Optional scope assignment that supersedes hierarchy discovery. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation", meta = (AllowPrivateAccess = "true")) TObjectPtr<ULunarNavigationScope> NavigationScopeOverride;
#if WITH_EDITORONLY_DATA
	/** Controls whether the UMG Designer publishes a custom visual state instead of live defaults. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Designer Preview")
	ELunarVisualStatePreviewMode PreviewMode = ELunarVisualStatePreviewMode::None;
	/** Custom state published only while PreviewMode is Custom and the widget is at design time. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Designer Preview", meta = (EditCondition = "PreviewMode == ELunarVisualStatePreviewMode::Custom", EditConditionHides))
	FLunarUIVisualState PreviewVisualState;
#endif

	/** Whether this control creates and drives an input-prompt receiver. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Prompts") bool bEnableInputPrompt = false;
	/** Widget class that receives resolved prompt snapshots. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Prompts", meta = (EditCondition = "bEnableInputPrompt")) TSubclassOf<UUserWidget> PromptWidgetClass;
	/** Ordered semantic actions requested by this control's prompt. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Prompts", meta = (EditCondition = "bEnableInputPrompt")) TArray<FLunarPromptActionRequest> PromptActions;
	/** Rule determining when the prompt host is visible. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Prompts", meta = (EditCondition = "bEnableInputPrompt")) ELunarPromptVisibilityPolicy PromptVisibilityPolicy = ELunarPromptVisibilityPolicy::WhenSelected;
	/** Optional icon set that supersedes the device default for this prompt. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Prompts", meta = (EditCondition = "bEnableInputPrompt")) TObjectPtr<ULunarInputIconSet> PromptIconSetOverride;

	/** Optional reusable sound set selected by per-event Use Data Asset modes. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Feedback") TObjectPtr<ULunarUISoundFeedbackAsset> SoundFeedbackAsset;
	/** Optional reusable haptic set selected by per-event Use Data Asset modes. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Feedback") TObjectPtr<ULunarUIHapticFeedbackAsset> HapticFeedbackAsset;
	/** Per-event sound overrides layered over project feedback defaults. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Feedback") FLunarUISoundOverrides SoundOverrides;
	/** Per-event haptic overrides layered over project feedback defaults. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Feedback") FLunarUIHapticOverrides HapticOverrides;

	/** Whether selecting this widget requests ancestor Lunar scroll boxes to reveal it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Scroll") bool bScrollIntoViewOnSelection = true;
	/** Non-localized accessible display name announced instead of the widget label when set. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Accessibility") FString AccessibleName;
	/** Additional non-localized accessible description exposed to assistive technology. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Accessibility", meta = (MultiLine = "true")) FString AccessibleDescription;
	/** Non-localized reason announced when a disabled control rejects interaction. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Accessibility", meta = (MultiLine = "true")) FString DisabledReason;

protected:
	/** @return Rebuilt Slate hierarchy containing native specialized presentation, owner content, and the prompt host. */
	virtual TSharedRef<SWidget> RebuildWidget() override;
	/** @return Optional control-specific native Slate presentation placed below arbitrary owner content. */
	virtual TSharedPtr<SWidget> RebuildLunarSpecializedPresentation();
	/** @param bReleaseChildren Whether child Slate resources must also be released. */
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	/** @brief Reapplies focus, presentation, prompt, and accessibility properties. */
	virtual void SynchronizeProperties() override;
	/** @brief Registers the control and binds local-player presentation notifications. */
	virtual void NativeConstruct() override;
	/** @brief Cancels transient interaction and unregisters the control. */
	virtual void NativeDestruct() override;
	/** @param MyGeometry Current allotted geometry. @param InDeltaTime Seconds since the previous tick. */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	/** @param InGeometry Current geometry. @param InMouseEvent Pointer event that entered. */
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	/** @param InMouseEvent Pointer event that left. */
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	/** @param InGeometry Current geometry. @param InMouseEvent Button-down event. @return Slate handling reply. */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	/** @param InGeometry Current geometry. @param InMouseEvent Preview button-down event. @return Handled when logical disabled state must intercept descendant input. */
	virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	/** @param InGeometry Current geometry. @param InMouseEvent Double-click event. @return Slate handling reply that begins the next independent pointer activation. */
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	/** @param InGeometry Current geometry. @param InMouseEvent Button-up event. @return Slate handling reply. */
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	/** @param CaptureLostEvent Capture-loss context supplied by Slate. */
	virtual void NativeOnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent) override;
	/** @param InGeometry Current geometry. @param InGestureEvent Touch-start event. @return Slate handling reply. */
	virtual FReply NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	/** @param InGeometry Current geometry. @param InGestureEvent Touch-move event. @return Slate handling reply. */
	virtual FReply NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	/** @param InGeometry Current geometry. @param InGestureEvent Touch-end event. @return Slate handling reply. */
	virtual FReply NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;

	/** @param ActionContext Semantic action to evaluate. @return Native policy result before Blueprint participation. */
	virtual bool NativeCanHandleLunarAction(const FLunarUIActionContext& ActionContext) const;
	/** @param ActionContext Semantic action to dispatch. @return Native handling result. */
	virtual ELunarUIActionResult NativeHandleLunarAction(const FLunarUIActionContext& ActionContext);
	/** @param Direction Physical direction received. @param OutActionTag Resolved value action. @return True when converted. */
	virtual bool NativeResolveDirectionalLunarControlAction(
		ELunarNavigationDirection Direction,
		FGameplayTag& OutActionTag) const;
	/** @param OutRepeatSettings Receives control-specific timing. @return True when an override is available. */
	virtual bool NativeGetLunarRepeatSettingsOverride(FLunarNavigationRepeatSettings& OutRepeatSettings) const;
	/** @return True when activation is currently permitted by the concrete control. */
	virtual bool NativeCanActivateLunarWidget() const;
	/** @brief Native hook invoked when this control becomes selected. */
	virtual void NativeOnLunarSelected();
	/** @brief Native hook invoked when this control becomes unselected. */
	virtual void NativeOnLunarUnselected();
	/** @brief Native hook invoked when an interaction press begins. */
	virtual void NativeOnLunarPressed();
	/** @param HoldSeconds Active hold time after the local delay. @param PressedSeconds Total time since Pressed, including the delay. @param DelayLeft Remaining local-delay time clamped to zero. @param bDelayElapsed Whether the local delay has elapsed. */
	virtual void NativeOnLunarHoldProgress(float HoldSeconds, float PressedSeconds, float DelayLeft, bool bDelayElapsed);
	/** @brief Native hook invoked when an interaction press ends. */
	virtual void NativeOnLunarReleased();
	/** @brief Native hook invoked after accepted activation. */
	virtual void NativeOnLunarActivated();
	/** @brief Native hook invoked after rejected activation or action dispatch. */
	virtual void NativeOnLunarRejected();
	/** @param NewInputDevice Newly active presentation device. */
	virtual void NativeOnLunarInputDeviceChanged(ELunarInputDeviceType NewInputDevice);
	/** @param PreviousState Previously published state. @param NewState Newly resolved state. @param bIsDesignerPreview True only for an editor Designer preview publication. */
	virtual void NativeOnLunarVisualStateChanged(
		const FLunarUIVisualState& PreviousState,
		const FLunarUIVisualState& NewState,
		bool bIsDesignerPreview);
	/** @brief Native hook invoked after prompt configuration or presentation becomes stale. */
	virtual void NativeOnInputPromptInvalidated();
	/** @return Accessible value text announced for the concrete control. */
	virtual FText NativeGetLunarAccessibleValueText() const;
	/** @param NativeFocusWidget Native descendant that began an editing session. */
	virtual void NativeOnNativeFocusDelegationStarted(UWidget* NativeFocusWidget);
	/** @param NativeFocusWidget Native descendant whose editing session was committed. */
	virtual void NativeOnNativeFocusDelegationCommitted(UWidget* NativeFocusWidget);
	/** @param NativeFocusWidget Native descendant whose editing session was cancelled. */
	virtual void NativeOnNativeFocusDelegationCancelled(UWidget* NativeFocusWidget);
	/** @param Key Accept key that triggered the decision. @return True when Accept should commit the native editing session. */
	virtual bool NativeShouldCommitNativeFocusDelegationOnAccept(const FKey& Key) const;

	/** @return Current resolved visual-state descriptor. */
	const FLunarUIVisualState& GetLunarVisualState() const { return CurrentVisualState; }
	/** @param NewValueStateTag Concrete control value-state tag published to Blueprint presentation. */
	void SetLunarValueState(FGameplayTag NewValueStateTag);
	/** @brief Cancels the current pointer press and releases capture when owned. */
	void CancelPointerPress();
	/** @brief Rebuilds accessible behavior and descriptive text. */
	void RefreshLunarAccessibility();
	/** @param NewValueText Updated accessible value announced to the owning Slate user. */
	void NotifyLunarAccessibleValueChanged(const FText& NewValueText);

	/** @param ActionContext Semantic action to evaluate in Blueprint. @return True when Blueprint permits handling. */
	UFUNCTION(BlueprintNativeEvent, Category = "Lunar|UI|Navigation", meta = (DisplayName = "Can Handle Lunar Action"))
	bool BP_CanHandleLunarAction(const FLunarUIActionContext& ActionContext) const;

	/** @param ActionContext Semantic action to dispatch in Blueprint. @return Blueprint handling result. */
	UFUNCTION(BlueprintNativeEvent, Category = "Lunar|UI|Navigation", meta = (DisplayName = "Handle Lunar Action"))
	ELunarUIActionResult BP_HandleLunarAction(const FLunarUIActionContext& ActionContext);

	/** @brief Blueprint notification for authoritative selection. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lunar|UI|Navigation|Events", meta = (DisplayName = "On Lunar Selected")) void BP_OnLunarSelected();
	/** @brief Blueprint notification for authoritative unselection. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lunar|UI|Navigation|Events", meta = (DisplayName = "On Lunar Unselected")) void BP_OnLunarUnselected();
	/** @brief Blueprint notification for interaction press. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lunar|UI|Navigation|Events", meta = (DisplayName = "On Lunar Pressed")) void BP_OnLunarPressed();
	/** @param HoldSeconds Active hold time after the local delay. @param PressedSeconds Total time since Pressed, including the delay. @param DelayLeft Remaining local-delay time clamped to zero. @param bDelayElapsed Whether the local delay has elapsed. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lunar|UI|Navigation|Events", meta = (DisplayName = "On Lunar Hold Progress")) void BP_OnLunarHoldProgress(float HoldSeconds, float PressedSeconds, float DelayLeft, bool bDelayElapsed);
	/** @brief Blueprint notification for interaction release. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lunar|UI|Navigation|Events", meta = (DisplayName = "On Lunar Released")) void BP_OnLunarReleased();
	/** @brief Blueprint notification for accepted activation. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lunar|UI|Navigation|Events", meta = (DisplayName = "On Lunar Activated")) void BP_OnLunarActivated();
	/** @brief Blueprint notification for rejected interaction. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lunar|UI|Navigation|Events", meta = (DisplayName = "On Lunar Rejected")) void BP_OnLunarRejected();
	/** @param NewInputDevice Newly active presentation device. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lunar|UI|Navigation|Events", meta = (DisplayName = "On Lunar Input Device Changed")) void BP_OnLunarInputDeviceChanged(ELunarInputDeviceType NewInputDevice);
	/** @param PreviousState Previously published state. @param NewState Newly resolved state. @param bIsDesignerPreview True only while previewing in the UMG Designer. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lunar|UI|Navigation|Events", meta = (DisplayName = "On Lunar Visual State Changed"))
	void BP_OnLunarVisualStateChanged(const FLunarUIVisualState& PreviousState, const FLunarUIVisualState& NewState, bool bIsDesignerPreview);
private:
	/** @return Navigation subsystem for the owning local player, or nullptr when unavailable. */
	ULunarNavigationSubsystem* ResolveNavigationSubsystem() const;
	/** @param bSelected New authoritative selection state assigned by the subsystem. */
	void SetLunarSelectedFromSubsystem(bool bSelected);
	/** @param ActionContext Rejected action responsible for feedback and notification. */
	void NotifyLunarRejectedFromSubsystem(const FLunarUIActionContext& ActionContext);
	/** @param InputDevice Active device. @param bPointerPresentationActive Whether pointer visuals should be used. */
	void HandleInputPresentationChanged(ELunarInputDeviceType InputDevice, bool bPointerPresentationActive);
	/** @brief Marks prompt output dirty and invokes the prompt invalidation hook. */
	void InvalidateInputPrompt();
	/** @return Prompt receiver class selected from the instance property or project default. */
	UClass* ResolveInputPromptReceiverClass() const;
	/** @brief Resolves and delivers pending prompt presentation updates. */
	void ProcessInputPromptUpdate();
	/** @brief Attaches the prompt receiver to its Slate host. */
	void AttachInputPromptReceiver();
	/** @brief Detaches the prompt receiver from its Slate host. */
	void DetachInputPromptReceiver();
	/** @param Actions Fully resolved prompt actions to deliver to the receiver. */
	void ApplyInputPromptSnapshot(const TArray<FLunarResolvedPromptAction>& Actions);
	/** @param bForceDelivery Whether to deliver an empty snapshot even when already clear. */
	void ClearInputPromptSnapshot(bool bForceDelivery = false);
	/** @brief Applies the configured prompt visibility policy to the Slate host. */
	void UpdateInputPromptHostVisibility();
	/** @brief Starts hold timing for the current valid press and emits its immediate zero sample. */
	void BeginLunarHoldTracking();
	/** @param DeltaSeconds Non-negative frame time used to advance active hold timing. */
	void AdvanceLunarHoldTracking(float DeltaSeconds);
	/** @brief Stops hold timing and clears all transient duration state. */
	void ResetLunarHoldTracking();
	/** @return True when visibility and the complete Lunar/native enabled hierarchy permit activation. */
	bool IsEffectivelyInteractive() const;
	/** @param bAllowSelfDisabled Whether this widget's own Lunar disabled state may be inspected. @return True when visibility and ancestor availability permit the requested use. */
	bool IsLunarHierarchyAvailable(bool bAllowSelfDisabled) const;
	/** @return True when pointer presentation is allowed for an enabled control or an inspectable disabled control. */
	bool CanReceiveLunarPointerPresentation() const;
	/** Applies pointer hover routed through an open descendant ContextMenu. @param bHovered Whether routed pointer geometry currently contains this widget. */
	void SetContextMenuPointerHovered(bool bHovered);
	/** @return True when native Slate hover or ContextMenu-routed hover is active. */
	bool IsPointerHoveredForPresentation() const;
	/** @return True when pointer input selected this widget, releasing ScrollBox confinement when crossing its boundary. */
	bool RequestLunarPointerSelection();
	/** @param bInputAllowed Whether the direct-input channel is enabled. @return True when this logically disabled control may receive selection and rejected activation from that channel. */
	bool CanInspectDisabledControlWithDirectInput(bool bInputAllowed) const;
	/** @brief Synchronizes accessible behavior, name, description, and value. */
	void SynchronizeLunarAccessibility();
	/** @brief Resolves and plays selected feedback. */
	void PlaySelectedFeedback();
	/** @brief Resolves and plays pressed feedback. */
	void PlayPressedFeedback();
	/** @brief Resolves and plays successful-click feedback. */
	void PlayClickedFeedback();
	/** @brief Resolves and plays rejection feedback. */
	void PlayRejectedFeedback();
	/** @brief Resolves and plays pointer-hover feedback. */
	void PlayPointerHoveredFeedback();

private:
	/** Authoritative selection state assigned only by the subsystem. */
	UPROPERTY(Transient) bool bLunarSelected = false;
	/** Whether the pointer currently hovers this control. */
	UPROPERTY(Transient) bool bPointerHovered = false;
	/** Whether an open descendant ContextMenu routes pointer hover to this inactive ancestor item. */
	UPROPERTY(Transient) bool bContextMenuPointerHovered = false;
	/** Whether this control owns an active pointer press. */
	UPROPERTY(Transient) bool bPointerPressed = false;
	/** Whether the current pointer press remains within activation tolerance. */
	UPROPERTY(Transient) bool bPointerActivationEligible = false;
	/** Whether the active pointer press is presentation-only and must finish with Rejected instead of Pressed, Hold, or activation. */
	UPROPERTY(Transient) bool bPointerPressRejected = false;
	/** Whether semantic navigation click is currently held. */
	UPROPERTY(Transient) bool bNavigationPressed = false;
	/** Whether a rejected semantic press is held to expose the Disabled + Navigation Pressed state combination. */
	UPROPERTY(Transient) bool bRejectedNavigationPressed = false;
	/** Whether a valid pointer, touch, or semantic press currently owns hold timing. */
	UPROPERTY(Transient) bool bLunarHoldTracking = false;
	/** Whether HoldStartDelay has elapsed for the current valid press. */
	UPROPERTY(Transient) bool bLunarHoldDelayElapsed = false;
	/** Active hold time accumulated after HoldStartDelay elapses. */
	UPROPERTY(Transient) float LunarHoldSeconds = 0.0f;
	/** Total physical press time accumulated since Pressed, including HoldStartDelay. */
	UPROPERTY(Transient) float LunarPressedSeconds = 0.0f;
	/** Whether prompt output must be resolved before presentation. */
	UPROPERTY(Transient) bool bPromptDirty = true;
	/** Instantiated widget that receives prompt snapshots. */
	UPROPERTY(Transient) TObjectPtr<UUserWidget> InputPromptReceiver;
	/** Concrete control value-state tag published to Blueprint presentation. */
	UPROPERTY(Transient) FGameplayTag CurrentValueStateTag;
	/** Last visual-state descriptor broadcast to observers. */
	UPROPERTY(Transient) FLunarUIVisualState CurrentVisualState;
	/** Previously published visual state supplied to the next transition event. */
	UPROPERTY(Transient) FLunarUIVisualState LastPublishedVisualState;
	/** Whether at least one visual-state publication has occurred. */
	UPROPERTY(Transient) bool bHasPublishedVisualState = false;
	/** Editable descendant whose text was captured when focus delegation began. */
	UPROPERTY(Transient) TWeakObjectPtr<UWidget> DelegatedFocusTextSnapshotWidget;
	/** Text restored if the delegated editing session is cancelled. */
	UPROPERTY(Transient) FText DelegatedFocusTextSnapshot;
	/** Whether delegated focus owns a valid editable-text snapshot. */
	UPROPERTY(Transient) bool bHasDelegatedFocusTextSnapshot = false;
	/** Slate host containing the optional prompt receiver. */
	TSharedPtr<SBox> LunarInputPromptHost;
	/** Subscription to owning-subsystem input presentation changes. */
	FDelegateHandle InputPresentationChangedHandle;
	/** Screen position captured at pointer press for drag-tolerance checks. */
	FVector2D PointerPressScreenPosition = FVector2D::ZeroVector;
	/** Reentrancy guard for prompt resolution and receiver delivery. */
	bool bPromptUpdateInProgress = false;
	/** Whether a prompt clear was requested during an in-progress update. */
	bool bPromptClearPending = false;
	/** Whether the prompt receiver currently owns a delivered non-stale snapshot. */
	bool bPromptReceiverHasCurrentSnapshot = false;
	/** Whether NativeConstruct has completed for the prompt owner. */
	bool bPromptOwnerConstructed = false;
	/** Cached effective-interaction state used to detect runtime eligibility changes. */
	bool bLastKnownEffectivelyInteractive = true;
	/** Last input device reflected in presentation. */
	ELunarInputDeviceType LastObservedInputDevice = ELunarInputDeviceType::Unknown;
	/** Last pointer-presentation state reflected in visuals. */
	bool bLastObservedPointerPresentation = false;

	friend class ULunarNavigationSubsystem; ///< Owns authoritative selection and focus delegation.
	friend class ULunarContextMenu; ///< Routes presentation-only hover across a cascading menu chain.
	friend class ULunarInputPromptWidget; ///< Receives protected prompt invalidation access.
};
