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
class UCurveFloat;

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
	 * @param NotificationPolicy Whether to publish normal change notifications; defaults to Notify
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Slider", meta = (AdvancedDisplay = "NotificationPolicy"))
	void SetValue(float NewValue, ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);

	/**
	 * @brief Gets the committed slider value
	 * @return Current committed value
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Slider")
	float GetValue() const;

	/**
	 * @brief Replaces the value range and normalizes the current committed and preview values
	 * @param NewMinValue New lower bound
	 * @param NewMaxValue New upper bound
	 * @param NotificationPolicy Whether to publish notifications for values changed by range normalization; defaults to Notify
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Slider", meta = (AdvancedDisplay = "NotificationPolicy"))
	void SetValueRange(float NewMinValue, float NewMaxValue, ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);

	/**
	 * @brief Gets the logical preview target used for editing and commit semantics
	 * @return Pending preview target when present, otherwise the committed value
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Slider")
	float GetPreviewValue() const;

	/**
	 * @brief Gets the transient value currently rendered by the slider presentation
	 * @return Interpolated display value when interpolation is enabled, otherwise the preview value
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Slider")
	float GetDisplayedValue() const;

	/** @brief Commits the pending preview value. @param NotificationPolicy Whether to publish normal commit notifications; defaults to Notify. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Slider", meta = (AdvancedDisplay = "NotificationPolicy"))
	void CommitPreviewValue(ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);

	/** @brief Discards the pending preview and restores the committed value. @param NotificationPolicy Whether to publish normal preview notifications; defaults to Notify. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Slider", meta = (AdvancedDisplay = "NotificationPolicy"))
	void CancelPreviewValue(ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);

	/** @param NewBrush New native track brush. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Slider|Presentation") void SetTrackBrush(const FSlateBrush& NewBrush);
	/** @return Current native track brush. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Slider|Presentation") FSlateBrush GetTrackBrush() const { return TrackBrush; }
	/** @param NewTint New native track tint. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Slider|Presentation") void SetTrackTint(FLinearColor NewTint);
	/** @return Current native track tint. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Slider|Presentation") FLinearColor GetTrackTint() const { return TrackTint; }
	/** @param NewThickness New non-negative track thickness. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Slider|Presentation") void SetTrackThickness(float NewThickness);
	/** @return Current track thickness. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Slider|Presentation") float GetTrackThickness() const { return TrackThickness; }
	/** @param NewBrush New native fill brush. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Slider|Presentation") void SetFillBrush(const FSlateBrush& NewBrush);
	/** @return Current native fill brush. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Slider|Presentation") FSlateBrush GetFillBrush() const { return FillBrush; }
	/** @param NewTint New native fill tint. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Slider|Presentation") void SetFillTint(FLinearColor NewTint);
	/** @return Current native fill tint. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Slider|Presentation") FLinearColor GetFillTint() const { return FillTint; }
	/** @param NewBrush New native thumb brush. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Slider|Presentation") void SetThumbBrush(const FSlateBrush& NewBrush);
	/** @return Current native thumb brush. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Slider|Presentation") FSlateBrush GetThumbBrush() const { return ThumbBrush; }
	/** @param NewTint New native thumb tint. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Slider|Presentation") void SetThumbTint(FLinearColor NewTint);
	/** @return Current native thumb tint. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Slider|Presentation") FLinearColor GetThumbTint() const { return ThumbTint; }
	/** @param NewSize New non-negative thumb size. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Slider|Presentation") void SetThumbSize(FVector2D NewSize);
	/** @return Current thumb size. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Slider|Presentation") FVector2D GetThumbSize() const { return ThumbSize; }
	/** @brief Configures the native unfilled track. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Slider|Presentation") void ConfigureTrackPresentation(const FSlateBrush& NewBrush, FLinearColor NewTint, float NewThickness);
	/** @brief Returns the native unfilled track presentation. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Slider|Presentation") void GetTrackPresentation(FSlateBrush& OutBrush, FLinearColor& OutTint, float& OutThickness) const;
	/** @brief Configures the native filled track. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Slider|Presentation") void ConfigureFillPresentation(const FSlateBrush& NewBrush, FLinearColor NewTint);
	/** @brief Returns the native filled track presentation. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Slider|Presentation") void GetFillPresentation(FSlateBrush& OutBrush, FLinearColor& OutTint) const;
	/** @brief Configures the native thumb. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Slider|Presentation") void ConfigureThumbPresentation(const FSlateBrush& NewBrush, FLinearColor NewTint, FVector2D NewSize);
	/** @brief Returns the native thumb presentation. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Slider|Presentation") void GetThumbPresentation(FSlateBrush& OutBrush, FLinearColor& OutTint, FVector2D& OutSize) const;
	/** @brief Configures every native slider part in one call. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Slider|Presentation") void ConfigureSliderPresentation(const FSlateBrush& NewTrackBrush, FLinearColor NewTrackTint, const FSlateBrush& NewFillBrush, FLinearColor NewFillTint, const FSlateBrush& NewThumbBrush, FLinearColor NewThumbTint, float NewTrackThickness, FVector2D NewThumbSize);
	/** @brief Returns every cached native slider presentation value. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Slider|Presentation") void GetSliderPresentation(FSlateBrush& OutTrackBrush, FLinearColor& OutTrackTint, FSlateBrush& OutFillBrush, FLinearColor& OutFillTint, FSlateBrush& OutThumbBrush, FLinearColor& OutThumbTint, float& OutTrackThickness, FVector2D& OutThumbSize) const;
	/** Lower endpoint of the configured slider range. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Slider")
	float MinValue = 0.0f;

	/** Upper endpoint of the configured slider range. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Slider")
	float MaxValue = 1.0f;

	/** Last committed value, clamped between the normalized range endpoints. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Slider")
	float Value = 0.0f;

	/** Determines whether the keyboard, D-pad, and stepped-stick size is absolute or range-relative. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Slider", meta = (DisplayName = "Navigation Step Mode"))
	ELunarSliderStepMode StepMode = ELunarSliderStepMode::Absolute;

	/** Positive value change applied for one keyboard, D-pad, or stepped-stick action. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Slider", meta = (DisplayName = "Navigation Step Size", ClampMin = "0.0", UIMin = "0.0"))
	float StepSize = 0.1f;

	/** Determines whether left-stick input uses navigation steps or continuous value motion. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Slider", meta = (DisplayName = "Stick Input Mode"))
	ELunarSliderStickInputMode StickInputMode = ELunarSliderStickInputMode::Stepped;

	/** Fraction of the complete value range traversed per second at full stick magnitude. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Slider", meta = (DisplayName = "Continuous Stick Range Per Second", ClampMin = "0.0", UIMin = "0.0", EditCondition = "StickInputMode == ELunarSliderStickInputMode::Continuous", EditConditionHides))
	float ContinuousStickRangePerSecond = 1.0f;

	/** Absolute mouse and touch snap interval; zero keeps pointer input continuous. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Slider", meta = (DisplayName = "Pointer Step Size (Mouse/Touch)", ClampMin = "0.0", UIMin = "0.0"))
	float PointerStepSize = 0.0f;

	/** Enables visual interpolation from the rendered value to every logical preview-value change. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Slider|Interpolation", meta = (DisplayName = "Interpolate Value Changes"))
	bool bInterpolateValueChanges = false;

	/** Normalized curve traversals per second; zero applies the logical value immediately. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Slider|Interpolation", meta = (DisplayName = "Value Interpolation Speed", ClampMin = "0.0", UIMin = "0.0", EditCondition = "bInterpolateValueChanges"))
	float ValueInterpolationSpeed = 12.0f;

	/** Normalized 0..1 response curve used for visual interpolation; null falls back to linear interpolation. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Slider|Interpolation", meta = (DisplayName = "Value Interpolation Curve", EditCondition = "bInterpolateValueChanges"))
	TObjectPtr<UCurveFloat> ValueInterpolationCurve = nullptr;

	/** Axis used for pointer projection and directional value actions. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Slider")
	TEnumAsByte<EOrientation> Orientation = Orient_Horizontal;

	/** Reverses the physical direction that increases the logical value. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Slider")
	bool bInvertValueDirection = false;

	/** Determines whether navigation changes commit immediately or wait for Accept. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Slider")
	ELunarSliderCommitMode CommitMode = ELunarSliderCommitMode::Immediate;

	/** Enables use of RepeatSettingsOverride instead of global navigation repeat timing. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Slider|Repeat")
	bool bOverrideRepeatSettings = false;

	/** Per-slider digital and analog repeat timing used when overriding global settings. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Slider|Repeat", meta = (EditCondition = "bOverrideRepeatSettings"))
	FLunarNavigationRepeatSettings RepeatSettingsOverride;

	/** Broadcast whenever the committed value changes. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Slider")
	FLunarSliderValueChangedSignature OnValueChanged;

	/** Broadcast whenever the logical preview target changes. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Slider")
	FLunarSliderValueChangedSignature OnPreviewValueChanged;

	/** Broadcast whenever the rendered value changes, including each visual interpolation frame. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Slider")
	FLunarSliderValueChangedSignature OnDisplayedValueChanged;

	/** Broadcast when a preview becomes the committed value. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Slider")
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

	/** @brief Applies continuous left-stick value motion for the active analog owner. @param MyGeometry Current widget geometry. @param InDeltaTime Frame time in seconds. */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** @brief Cancels active pointer and preview state before destruction. */
	virtual void NativeDestruct() override;


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
	 * @brief Builds accessibility text for the current logical preview value
	 * @return Localized accessible value text
	 */
	virtual FText NativeGetLunarAccessibleValueText() const override;


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

	/** @param InValue Raw pointer-projected value. @return Clamped value snapped to PointerStepSize when enabled. */
	float SnapPointerValue(float InValue) const;

	/** @brief Advances continuous left-stick editing for this frame. @param DeltaTime Non-negative frame time in seconds. */
	void UpdateContinuousStickInput(float DeltaTime);

	/** @brief Advances optional visual interpolation toward PreviewValue. @param DeltaTime Non-negative frame time in seconds. */
	void UpdateDisplayedValueInterpolation(float DeltaTime);

	/** @brief Snaps the rendered value to PreviewValue when interpolation is inactive. @param bBroadcast Whether to broadcast OnDisplayedValueChanged. */
	void RefreshDisplayedValueTarget(bool bBroadcast);

	/** @brief Stores a clamped rendered value. @param NewDisplayedValue Requested rendered value. @param bBroadcast Whether to broadcast OnDisplayedValueChanged. */
	void SetDisplayedValueInternal(float NewDisplayedValue, bool bBroadcast);

	/**
	 * @brief Applies one navigation step to the preview or committed value
	 * @param bIncrease True to increase, false to decrease
	 * @return True when the logical preview or committed value changed
	 */
	bool ApplyNavigationStep(bool bIncrease);

	/**
	 * @brief Updates pending preview state without committing it
	 * @param NewPreviewValue Requested preview value
	 */
	void SetPreviewValueInternal(float NewPreviewValue);

	/**
	 * @brief Updates the committed value and logical preview target
	 * @param NewValue Requested committed value
	 * @param bBroadcastCommitted Whether to broadcast OnValueCommitted
	 * @param bNotifyChange Whether to publish value, preview, and accessibility change notifications
	 */
	void SetCommittedValueInternal(float NewValue, bool bBroadcastCommitted, bool bNotifyChange = true);

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
	 * @brief Invalidates the native slider presentation
	 * @param bInvalidateLayout Whether desired size may also have changed
	 */
	void InvalidateSliderPresentation(bool bInvalidateLayout = false) const;

	/**
	 * @brief Emits an accessibility value-changed notification
	 * @param NewValue New value exposed to accessibility services
	 */
	void NotifyAccessibleValue(float NewValue);

	/** Logical value edited before an optional commit and used as the visual interpolation target. */
	UPROPERTY(Transient)
	float PreviewValue = 0.0f;

	/** Transient value rendered by native or owner-authored presentation while interpolation advances. */
	UPROPERTY(Transient)
	float DisplayedValue = 0.0f;

	/** Whether DisplayedValue has been initialized from a valid logical preview value. */
	UPROPERTY(Transient)
	bool bDisplayedValueInitialized = false;

	/** Rendered value captured when the current curve traversal began. */
	float DisplayedValueInterpolationSource = 0.0f;

	/** Logical preview target captured for the current curve traversal. */
	float DisplayedValueInterpolationTarget = 0.0f;

	/** Seconds elapsed since the current curve traversal began. */
	float DisplayedValueInterpolationElapsed = 0.0f;

	/** Whether a curve traversal toward DisplayedValueInterpolationTarget is active. */
	bool bDisplayedValueInterpolationActive = false;

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

	/** Cached native track brush reapplied after rebuild. */
	UPROPERTY(Transient) FSlateBrush TrackBrush;
	/** Cached native fill brush reapplied after rebuild. */
	UPROPERTY(Transient) FSlateBrush FillBrush;
	/** Cached native thumb brush reapplied after rebuild. */
	UPROPERTY(Transient) FSlateBrush ThumbBrush;
	/** Cached native track tint. */
	UPROPERTY(Transient) FLinearColor TrackTint = FLinearColor(0.16f, 0.16f, 0.16f, 1.0f);
	/** Cached native fill tint. */
	UPROPERTY(Transient) FLinearColor FillTint = FLinearColor(0.10f, 0.48f, 0.95f, 1.0f);
	/** Cached native thumb tint. */
	UPROPERTY(Transient) FLinearColor ThumbTint = FLinearColor::White;
	/** Cached native track thickness. */
	UPROPERTY(Transient) float TrackThickness = 4.0f;
	/** Cached native thumb size. */
	UPROPERTY(Transient) FVector2D ThumbSize = FVector2D(14.0f);
	/** Native presentation for track, fill, and thumb rendering. */
	TSharedPtr<SLunarSliderPresentation> SliderPresentation;
	/** Prevents recursive prompt invalidation while updating the Accept request. */
	bool bSynchronizingAcceptPrompt = false;
	/** Commit mode observed during the previous synchronization pass. */
	ELunarSliderCommitMode LastObservedCommitMode = ELunarSliderCommitMode::Immediate;

	/** Grants the native presentation read access to slider behavior and cached native parts. */
	friend class SLunarSliderPresentation;
};
