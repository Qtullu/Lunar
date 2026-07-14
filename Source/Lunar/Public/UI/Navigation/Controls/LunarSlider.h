// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Navigation/Core/LunarNavigableWidget.h"
#include "LunarSlider.generated.h"

/**
 * @file LunarSlider.h
 * @brief Continuous navigable Lunar slider control
 * @ingroup LunarNavigationControls
 */

class SLunarSliderPresentation;

/**
 * @brief Continuous value control with orientation-aware navigation and preview/commit semantics
 * @ingroup LunarNavigationControls
 */
UCLASS(Blueprintable)
class LUNAR_API ULunarSlider : public ULunarNavigableWidget
{
	GENERATED_BODY()

public:
	/**
	 * @brief Creates a Lunar slider with navigation, pointer, and prompt defaults
	 * @param ObjectInitializer Unreal object initializer
	 */
	ULunarSlider(const FObjectInitializer& ObjectInitializer);

	/**
	 * @brief Sets the committed value after clamping it to the configured range
	 * @param NewValue Value to commit
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|Slider")
	void SetValue(float NewValue);

	/**
	 * @brief Gets the committed slider value
	 * @return Current committed value
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation|Slider")
	float GetValue() const;

	/**
	 * @brief Replaces the value range and normalizes the current committed and preview values
	 * @param NewMinValue New lower bound
	 * @param NewMaxValue New upper bound
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|Slider")
	void SetValueRange(float NewMinValue, float NewMaxValue);

	/**
	 * @brief Gets the value currently shown by the slider
	 * @return Pending preview value when present, otherwise the committed value
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation|Slider")
	float GetPreviewValue() const;

	/** @brief Commits the pending preview value and broadcasts the commit event. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|Slider")
	void CommitPreviewValue();

	/** @brief Discards the pending preview and restores the committed value. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|Slider")
	void CancelPreviewValue();

	/** Lower endpoint of the configured slider range. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Slider")
	float MinValue = 0.0f;

	/** Upper endpoint of the configured slider range. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Slider")
	float MaxValue = 1.0f;

	/** Last committed value, clamped between the normalized range endpoints. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Slider")
	float Value = 0.0f;

	/** Determines whether StepSize is an absolute amount or a fraction of the range. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Slider")
	ELunarSliderStepMode StepMode = ELunarSliderStepMode::Absolute;

	/** Positive value change applied for one Increase or Decrease action. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Slider", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float StepSize = 0.1f;

	/** Axis used for pointer projection and directional value actions. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Slider")
	TEnumAsByte<EOrientation> Orientation = Orient_Horizontal;

	/** Reverses the physical direction that increases the logical value. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Slider")
	bool bInvertValueDirection = false;

	/** Determines whether navigation changes commit immediately or wait for Accept. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Slider")
	ELunarSliderCommitMode CommitMode = ELunarSliderCommitMode::Immediate;

	/** Enables use of RepeatSettingsOverride instead of global navigation repeat timing. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Slider|Repeat")
	bool bOverrideRepeatSettings = false;

	/** Per-slider digital and analog repeat timing used when overriding global settings. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Slider|Repeat", meta = (EditCondition = "bOverrideRepeatSettings"))
	FLunarNavigationRepeatSettings RepeatSettingsOverride;

	/** Broadcast whenever the committed value changes. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation|Slider")
	FLunarSliderValueChangedSignature OnValueChanged;

	/** Broadcast whenever the displayed preview value changes. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation|Slider")
	FLunarSliderValueChangedSignature OnPreviewValueChanged;

	/** Broadcast when a preview becomes the committed value. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation|Slider")
	FLunarSliderValueCommittedSignature OnValueCommitted;

protected:
	/** @brief Creates the native track, fill, and thumb presentation layer. */
	virtual TSharedPtr<SWidget> RebuildLunarSpecializedPresentation() override;

	/**
	 * @brief Releases the native slider presentation
	 * @param bReleaseChildren Whether child Slate resources should also be released
	 */
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	/** @brief Normalizes authored values and synchronizes the native presentation. */
	virtual void SynchronizeProperties() override;

	/** @brief Cancels active pointer and preview state before destruction. */
	virtual void NativeDestruct() override;

	/**
	 * @brief Advances the active slider-style transition
	 * @param MyGeometry Current cached widget geometry
	 * @param InDeltaTime Elapsed frame time in seconds
	 */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/**
	 * @brief Begins mouse-driven value preview when the primary button is pressed
	 * @param InGeometry Current widget geometry
	 * @param InMouseEvent Mouse press event
	 * @return Handled reply with capture when interaction begins, otherwise the base reply
	 */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/**
	 * @brief Updates the value preview while mouse capture is owned
	 * @param InGeometry Current widget geometry
	 * @param InMouseEvent Mouse move event
	 * @return Handled reply during an active pointer interaction, otherwise the base reply
	 */
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/**
	 * @brief Commits the mouse-driven preview on primary-button release
	 * @param InGeometry Current widget geometry
	 * @param InMouseEvent Mouse release event
	 * @return Handled reply when the active interaction ends, otherwise the base reply
	 */
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/**
	 * @brief Cancels an active pointer preview when Slate capture is lost
	 * @param CaptureLostEvent Capture-loss context
	 */
	virtual void NativeOnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent) override;

	/**
	 * @brief Begins touch-driven value preview
	 * @param InGeometry Current widget geometry
	 * @param InGestureEvent Touch-start event
	 * @return Handled reply with capture when interaction begins, otherwise the base reply
	 */
	virtual FReply NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;

	/**
	 * @brief Updates the value preview during an active touch interaction
	 * @param InGeometry Current widget geometry
	 * @param InGestureEvent Touch-move event
	 * @return Handled reply during an active interaction, otherwise the base reply
	 */
	virtual FReply NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;

	/**
	 * @brief Commits the touch-driven preview when touch ends
	 * @param InGeometry Current widget geometry
	 * @param InGestureEvent Touch-end event
	 * @return Handled reply when the active interaction ends, otherwise the base reply
	 */
	virtual FReply NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;

	/**
	 * @brief Checks whether a semantic action can change, commit, or cancel this slider
	 * @param ActionContext Semantic action and input context being queried
	 * @return True when this slider can handle the action
	 */
	virtual bool NativeCanHandleLunarAction(const FLunarUIActionContext& ActionContext) const override;

	/**
	 * @brief Applies Increase, Decrease, Accept, or Back semantics
	 * @param ActionContext Semantic action and input context to process
	 * @return Handling result reported to the navigation subsystem
	 */
	virtual ELunarUIActionResult NativeHandleLunarAction(const FLunarUIActionContext& ActionContext) override;

	/**
	 * @brief Maps one physical direction on the configured axis to Increase or Decrease
	 * @param Direction Physical navigation direction
	 * @param OutActionTag Receives the resolved value-action tag
	 * @return True when this slider claims the supplied direction
	 */
	virtual bool NativeResolveDirectionalLunarControlAction(
		ELunarNavigationDirection Direction,
		FGameplayTag& OutActionTag) const override;

	/**
	 * @brief Returns the optional per-slider repeat timing
	 * @param OutRepeatSettings Receives the active override
	 * @return True when RepeatSettingsOverride is enabled
	 */
	virtual bool NativeGetLunarRepeatSettingsOverride(FLunarNavigationRepeatSettings& OutRepeatSettings) const override;

	/** @brief Synchronizes preview state and prompts when the slider becomes selected. */
	virtual void NativeOnLunarSelected() override;

	/** @brief Cancels pointer interaction and pending preview state when the slider is unselected. */
	virtual void NativeOnLunarUnselected() override;

	/** @brief Rebuilds the conditional Accept prompt after prompt data changes. */
	virtual void NativeOnInputPromptInvalidated() override;

	/**
	 * @brief Builds accessibility text for the currently displayed numeric value
	 * @return Localized accessible value text
	 */
	virtual FText NativeGetLunarAccessibleValueText() const override;

	/**
	 * @brief Resolves the compatible slider style and caches its specialized fields
	 * @param OutStyle Receives the resolved common style values
	 * @param OutError Receives a configuration error when resolution fails
	 * @return True when the slider style was resolved successfully
	 */
	virtual bool ResolveCommonStylePatch(FLunarCommonStylePatch& OutStyle, FString& OutError) const override;

	/**
	 * @brief Applies common style values and starts the specialized slider transition
	 * @param ResolvedStyle Resolved common style values
	 */
	virtual void ApplyResolvedCommonStyle(const FLunarCommonStylePatch& ResolvedStyle) override;

private:
	/** @return Normalized lower range endpoint. */
	float GetMinimumValue() const;

	/** @return Normalized upper range endpoint. */
	float GetMaximumValue() const;

	/**
	 * @brief Clamps a value to the normalized configured range
	 * @param InValue Value to clamp
	 * @return Clamped value
	 */
	float ClampValue(float InValue) const;

	/** @return Effective positive amount for one navigation step. */
	float GetStepAmount() const;

	/**
	 * @brief Applies one navigation step to the preview or committed value
	 * @param bIncrease True to increase, false to decrease
	 * @return True when the displayed value changed
	 */
	bool ApplyNavigationStep(bool bIncrease);

	/**
	 * @brief Updates pending preview state without committing it
	 * @param NewPreviewValue Requested preview value
	 */
	void SetPreviewValueInternal(float NewPreviewValue);

	/**
	 * @brief Updates the committed and displayed values
	 * @param NewValue Requested committed value
	 * @param bBroadcastCommitted Whether to broadcast OnValueCommitted
	 */
	void SetCommittedValueInternal(float NewValue, bool bBroadcastCommitted);

	/** @brief Orders range endpoints and clamps committed and preview values. */
	void NormalizeConfiguredRange();

	/** @brief Adds or removes the Accept prompt required by OnAccept mode. */
	void UpdateAcceptPrompt();

	/** @brief Captures committed and preview state before pointer editing begins. */
	void BeginPointerInteraction();

	/**
	 * @brief Projects a screen-space pointer position onto the slider value axis
	 * @param InGeometry Current widget geometry
	 * @param ScreenSpacePosition Pointer position in screen space
	 */
	void UpdatePointerPreview(const FGeometry& InGeometry, const FVector2D& ScreenSpacePosition);

	/** @brief Commits the active pointer preview and clears pointer state. */
	void CommitPointerInteraction();

	/** @brief Restores the state captured before the active pointer interaction. */
	void CancelPointerInteraction();

	/**
	 * @brief Starts, reverses, or immediately applies a specialized style target
	 * @param NewTarget Materialized specialized style target
	 */
	void ApplySliderStyleTarget(const FLunarSliderStylePatch& NewTarget);

	/**
	 * @brief Stores and displays one interpolated specialized style snapshot
	 * @param NewDisplayedStyle Style snapshot to display
	 */
	void ApplyDisplayedSliderStyle(const FLunarSliderStylePatch& NewDisplayedStyle);

	/**
	 * @brief Advances the active specialized style transition
	 * @param DeltaTime Elapsed frame time in seconds
	 */
	void TickSliderStyleTransition(float DeltaTime);

	/**
	 * @brief Invalidates the native slider presentation
	 * @param bInvalidateLayout Whether desired size may also have changed
	 */
	void InvalidateSliderPresentation(bool bInvalidateLayout = false) const;

	/**
	 * @brief Emits an accessibility value-changed notification
	 * @param NewValue New value exposed to accessibility services
	 */
	void NotifyAccessibleValue(float NewValue);

	/** Value currently displayed and edited before an optional commit. */
	UPROPERTY(Transient)
	float PreviewValue = 0.0f;

	/** Whether PreviewValue differs logically from the committed Value. */
	UPROPERTY(Transient)
	bool bHasPendingPreview = false;

	/** Whether mouse or touch is currently editing the slider. */
	UPROPERTY(Transient)
	bool bPointerInteractionActive = false;

	/** Committed value captured when pointer interaction began. */
	UPROPERTY(Transient)
	float PointerStartCommittedValue = 0.0f;

	/** Preview value captured when pointer interaction began. */
	UPROPERTY(Transient)
	float PointerStartPreviewValue = 0.0f;

	/** Whether a pending preview existed when pointer interaction began. */
	UPROPERTY(Transient)
	bool bPointerStartHadPendingPreview = false;

	/** Tracks the Back press that will cancel a pending preview on release. */
	UPROPERTY(Transient)
	bool bBackPreviewCancelPressed = false;

	/** Specialized style patch produced by the latest style resolution. */
	UPROPERTY(Transient)
	FLunarSliderStylePatch ResolvedSliderStyle;

	/** Specialized style snapshot currently displayed by Slate. */
	UPROPERTY(Transient)
	FLunarSliderStylePatch DisplayedSliderStyle;

	/** Specialized style snapshot at the start of the active transition. */
	UPROPERTY(Transient)
	FLunarSliderStylePatch TransitionSourceSliderStyle;

	/** Materialized destination of the active specialized style transition. */
	UPROPERTY(Transient)
	FLunarSliderStylePatch TransitionTargetSliderStyle;

	/** Latest logical specialized style target, including discrete fields. */
	UPROPERTY(Transient)
	FLunarSliderStylePatch LogicalTargetSliderStyle;

	/** Native fallback presentation for track, fill, and thumb rendering. */
	TSharedPtr<SLunarSliderPresentation> SliderPresentation;
	/** Elapsed time in the active specialized style transition. */
	float SliderStyleTransitionElapsed = 0.0f;
	/** Duration of the active specialized style transition. */
	float SliderStyleTransitionDuration = 0.0f;
	/** Whether a specialized style transition is currently active. */
	bool bSliderStyleTransitionActive = false;
	/** Whether the active transition is reversing toward its previous source. */
	bool bSliderStyleTransitionReversing = false;
	/** Whether DisplayedSliderStyle contains a valid materialized snapshot. */
	bool bHasDisplayedSliderStyle = false;
	/** Prevents recursive prompt invalidation while updating the Accept request. */
	bool bSynchronizingAcceptPrompt = false;
	/** Commit mode observed during the previous synchronization pass. */
	ELunarSliderCommitMode LastObservedCommitMode = ELunarSliderCommitMode::Immediate;

	/** Grants the native presentation read access to slider rendering state. */
	friend class SLunarSliderPresentation;
};
