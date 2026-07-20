// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Slate/WidgetTransform.h"
#include "UI/Navigation/Core/LunarNavigableWidget.h"
#include "LunarRadio.generated.h"

/** @file LunarRadio.h @brief Composite navigable Lunar Radio that generates and owns its internal options. @ingroup LunarNavigationControls */
class SLunarRadioVisual;
class UCurveFloat;
class ULunarRadioSideVisual;

/** @brief Three-by-three placement of an optional side visual around one Radio indicator. @ingroup LunarNavigationControls */
UENUM(BlueprintType)
enum class ELunarRadioSideVisualPlacement : uint8
{
	TopLeft,      ///< Place the side visual above and to the left of the indicator.
	TopCenter,    ///< Place the side visual directly above the indicator.
	TopRight,     ///< Place the side visual above and to the right of the indicator.
	CenterLeft,   ///< Place the side visual directly left of the indicator.
	Center,       ///< Overlay the side visual at the indicator center.
	CenterRight,  ///< Place the side visual directly right of the indicator.
	BottomLeft,   ///< Place the side visual below and to the left of the indicator.
	BottomCenter, ///< Place the side visual directly below the indicator.
	BottomRight   ///< Place the side visual below and to the right of the indicator.
};

/** @brief Technical and localized data assigned to one generated Radio side visual. @ingroup LunarNavigationControls */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarRadioSideVisualData
{
	GENERATED_BODY()

	/** Non-localized technical identifier used by runtime selection and persistence APIs. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Radio") FString StringValue;
	/** Optional text exposed to the generated side presentation. Defaults non-localizable; authors may enable localization explicitly. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Radio") FText DisplayText = FText::AsCultureInvariant(TEXT(""));
};

/** @brief Brush, tint, size, and render transform for one internal Radio interaction state. @ingroup LunarNavigationControls */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarRadioVisualStyle
{
	GENERATED_BODY()

	/** Native Slate brush rendered for this style. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Radio") FSlateBrush Brush;
	/** Additional linear tint multiplied with the brush tint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Radio") FLinearColor Tint = FLinearColor::White;
	/** Native draw size reserved for this state-specific visual. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Radio", meta = (ClampMin = "0.0", UIMin = "0.0")) FVector2D Size = FVector2D(16.0f);
	/** State-specific render translation, scale, shear, and rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Radio") FWidgetTransform Transform;
};

/** @brief Complete six-state inline presentation set for one Radio visual layer. @ingroup LunarNavigationControls */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarRadioInteractionStyleSet
{
	GENERATED_BODY()

	/** Style used in pointer mode without hover or press. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Radio") FLunarRadioVisualStyle PointerNormal;
	/** Style used while the pointer hovers this internal option. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Radio") FLunarRadioVisualStyle PointerHovered;
	/** Style used while the pointer actively presses this internal option. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Radio") FLunarRadioVisualStyle PointerPressed;
	/** Style used for an internal option that is not active in navigation mode. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Radio") FLunarRadioVisualStyle NavigationNormal;
	/** Style used for the selected internal option while the composite Radio owns Lunar Selection. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Radio") FLunarRadioVisualStyle NavigationSelected;
	/** Style used while navigation input presses and changes the selected internal option. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Radio") FLunarRadioVisualStyle NavigationPressed;

	/** @param InteractionState State to resolve. @return Const style matching the supplied state. */
	const FLunarRadioVisualStyle& Resolve(ELunarUIInteractionState InteractionState) const;
	/** @param InteractionState State whose style is replaced. @param NewStyle Replacement inline style. */
	void Set(ELunarUIInteractionState InteractionState, const FLunarRadioVisualStyle& NewStyle);
};

/** @brief Broadcast after a composite Radio changes logical option selection. @ingroup LunarNavigationControls */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FLunarRadioSelectedIndexChangedSignature, ULunarRadio*, Radio, int32, PreviousIndex, int32, NewIndex, FLunarRadioSideVisualData, SelectedData);
/** @brief Broadcast whenever the shared Checked indicator's rendered fractional position changes. @ingroup LunarNavigationControls */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLunarRadioDisplayedSelectionPositionChangedSignature, ULunarRadio*, Radio, float, DisplayedSelectionPosition);

/** @brief One navigable composite that generates N internal Radio options and always owns one selection. @ingroup LunarNavigationControls */
UCLASS(Blueprintable)
class LUNAR_API ULunarRadio : public ULunarNavigableWidget
{
	GENERATED_BODY()

public:
	/** @param ObjectInitializer Unreal object initializer. @brief Creates a composite Radio with three generated options and one required selection. */
	ULunarRadio(const FObjectInitializer& ObjectInitializer);

	/** @param NewNumOfRadioButtons Requested generated-option count clamped to at least one. @param NotificationPolicy Whether to publish when count normalization changes SelectedIndex. @brief Resizes generated options and their side data while preserving existing entries. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Radio", meta = (AdvancedDisplay = "NotificationPolicy"))
	void SetNumOfRadioButtons(int32 NewNumOfRadioButtons, ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);
	/** @return Number of generated internal Radio options. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Radio") int32 GetNumOfRadioButtons() const { return NumOfRadioButtons; }

	/** @param NewSelectedIndex Requested index clamped into the required non-empty selection range. @param NotificationPolicy Whether to publish normal selection notifications. @brief Changes logical selection and optionally animates the shared Checked indicator. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Radio", meta = (AdvancedDisplay = "NotificationPolicy"))
	void SetSelectedIndex(int32 NewSelectedIndex, ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);
	/** @return Current always-valid selected option index. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Radio") int32 GetSelectedIndex() const { return SelectedIndex; }
	/** @param OptionIndex Index to compare. @return True when the supplied index is the current logical selection. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Radio") bool IsIndexSelected(int32 OptionIndex) const { return SelectedIndex == OptionIndex; }

	/** @param StringValue Unique technical value to resolve. @param NotificationPolicy Whether to publish normal selection notifications. @return True when exactly one option matches and becomes selected. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Radio", meta = (AdvancedDisplay = "NotificationPolicy"))
	bool SetSelectedByStringValue(const FString& StringValue, ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);
	/** @return Technical StringValue of the current selection. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Radio") FString GetSelectedStringValue() const;
	/** @return Complete technical and localized data of the current selection. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Radio") FLunarRadioSideVisualData GetSelectedData() const;

	/** @param OptionIndex Generated option to update. @param NewData Replacement technical and localized data. @return True when the index exists and its data was accepted. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Radio") bool SetSideVisualDataAt(int32 OptionIndex, const FLunarRadioSideVisualData& NewData);
	/** @param OptionIndex Generated option to query. @return Stored data or an empty structure when the index is invalid. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Radio") FLunarRadioSideVisualData GetSideVisualDataAt(int32 OptionIndex) const;
	/** @param OptionIndex Generated option to query. @return Live generated side visual, or null when no class or invalid index exists. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Radio") ULunarRadioSideVisual* GetSideVisualAt(int32 OptionIndex) const;

	/** @return Fractional option coordinate currently rendered by the shared Checked indicator. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Radio|Interpolation") float GetDisplayedSelectionPosition() const;
	/** @param InteractionState State whose inline Unchecked style is replaced. @param NewStyle Replacement style. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Radio|Presentation") void SetUncheckedStyle(ELunarUIInteractionState InteractionState, const FLunarRadioVisualStyle& NewStyle);
	/** @param InteractionState State whose inline Unchecked style is requested. @return Current style for that state. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Radio|Presentation") FLunarRadioVisualStyle GetUncheckedStyle(ELunarUIInteractionState InteractionState) const;
	/** @param InteractionState State whose inline Checked-indicator style is replaced. @param NewStyle Replacement style. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Radio|Presentation") void SetCheckedIndicatorStyle(ELunarUIInteractionState InteractionState, const FLunarRadioVisualStyle& NewStyle);
	/** @param InteractionState State whose inline Checked-indicator style is requested. @return Current style for that state. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Radio|Presentation") FLunarRadioVisualStyle GetCheckedIndicatorStyle(ELunarUIInteractionState InteractionState) const;

	/** Authoritative number of generated options; values below one are normalized to one. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Radio", meta = (ClampMin = "1", UIMin = "1")) int32 NumOfRadioButtons = 3;
	/** Current required option selection clamped to the generated range. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Radio", meta = (ClampMin = "0", UIMin = "0")) int32 SelectedIndex = 0;
	/** Whether moving past an internal edge wraps to the opposite option instead of leaving the composite control. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Radio") bool bWrapSelection = false;
	/** Axis used to arrange options and claim direct directional selection input. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Radio|Layout") TEnumAsByte<EOrientation> Orientation = Orient_Vertical;
	/** Fixed indicator layout and pointer-hit size reserved inside every generated option. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Radio|Layout", meta = (ClampMin = "0.0", UIMin = "0.0")) FVector2D ButtonSize = FVector2D(24.0f);
	/** Gap between adjacent generated option hit/layout boxes. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Radio|Layout", meta = (ClampMin = "0.0", UIMin = "0.0")) float ButtonSpacing = 8.0f;
	/** Complete inline state styles for the fixed Unchecked visuals rendered at every option position. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Radio|Unchecked Styles") FLunarRadioInteractionStyleSet UncheckedStyles;
	/** Complete inline state styles for the one shared Checked indicator that moves between options. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Radio|Checked Indicator Styles") FLunarRadioInteractionStyleSet CheckedIndicatorStyles;
	/** Optional non-navigable UserWidget class generated once beside every internal option. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Radio|Side Visual") TSubclassOf<ULunarRadioSideVisual> SideVisualClass;
	/** Three-by-three placement shared by every generated side visual. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Radio|Side Visual") ELunarRadioSideVisualPlacement SideVisualPlacement = ELunarRadioSideVisualPlacement::CenterRight;
	/** Horizontal and vertical indicator-to-side-visual gaps; Center ignores both axes. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Radio|Side Visual", meta = (ClampMin = "0.0", UIMin = "0.0")) FVector2D SideVisualSpacing = FVector2D(8.0f, 0.0f);
	/** One technical StringValue and localized DisplayText entry per generated option; array size follows NumOfRadioButtons. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Radio|Side Visual", meta = (EditFixedSize)) TArray<FLunarRadioSideVisualData> SideVisualData;
	/** Enables presentation-only Checked-indicator motion between non-wrap selections. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Radio|Interpolation", meta = (DisplayName = "Interpolate Selection Movement")) bool bInterpolateSelectionMovement = false;
	/** Normalized curve traversals per second; zero applies logical selection immediately. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Radio|Interpolation", meta = (ClampMin = "0.0", UIMin = "0.0", EditCondition = "bInterpolateSelectionMovement")) float SelectionInterpolationSpeed = 12.0f;
	/** Normalized response curve; null falls back to linear interpolation. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Radio|Interpolation", meta = (EditCondition = "bInterpolateSelectionMovement")) TObjectPtr<UCurveFloat> SelectionInterpolationCurve = nullptr;
	/** Broadcast once after logical selection changes, including previous/new indices and new option data. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Radio|Events") FLunarRadioSelectedIndexChangedSignature OnSelectedIndexChanged;
	/** Broadcast whenever the rendered fractional Checked position changes during interpolation. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Radio|Interpolation") FLunarRadioDisplayedSelectionPositionChangedSignature OnDisplayedSelectionPositionChanged;

	/** @param PreviousIndex Index selected before the change. @param NewIndex Newly selected index. @param SelectedData Technical and localized data for NewIndex. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lunar|UI|Radio|Events", meta = (DisplayName = "On Lunar Selected Index Changed"))
	void BP_OnLunarSelectedIndexChanged(int32 PreviousIndex, int32 NewIndex, const FLunarRadioSideVisualData& SelectedData);
	/** @return Const authored data array used by runtime validation. */
	const TArray<FLunarRadioSideVisualData>& GetSideVisualDataArray() const { return SideVisualData; }

protected:
	/** @return Lunar presentation wrapped by the Radio-owned pointer interaction surface. */
	virtual TSharedRef<SWidget> RebuildWidget() override;
	/** @return Native generated-option and shared-indicator presentation. */
	virtual TSharedPtr<SWidget> RebuildLunarSpecializedPresentation() override;
	/** @param bReleaseChildren Whether child Slate resources are released. @brief Releases generated native and side-visual presentation. */
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	/** @brief Normalizes authored count, selection, data, layout, interpolation, and presentation. */
	virtual void SynchronizeProperties() override;
	/** @brief Ensures generated side visuals and selection animation are initialized at runtime. */
	virtual void NativeConstruct() override;
	/** @brief Cancels internal pointer/navigation state before base teardown. */
	virtual void NativeDestruct() override;
	/** @param MyGeometry Current geometry. @param InDeltaTime Seconds since the previous frame. @brief Advances interpolation and refreshes geometry-dependent Checked placement. */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	/** @param InGeometry Current geometry. @param InMouseEvent Pointer event that entered. @brief Resolves the independently hovered internal option. */
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	/** @param InGeometry Current geometry. @param InMouseEvent Pointer move event. @return Base routing reply. */
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	/** @param InMouseEvent Pointer event that left. @brief Clears independent internal hover state. */
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	/** @param InGeometry Current geometry. @param InMouseEvent Preview press event. @return Radio-owned pointer routing reply. */
	virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	/** @param InGeometry Current geometry. @param InMouseEvent Mouse press event. @return Handled reply only for a generated option hit. */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	/** @param InGeometry Current geometry. @param InMouseEvent Mouse release event. @return Base completion reply after same-option validation. */
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	/** @param CaptureLostEvent Slate capture-loss context. @brief Clears pending internal option activation. */
	virtual void NativeOnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent) override;
	/** @param InGeometry Current geometry. @param InGestureEvent Primary-touch press event. @return Handled reply only for a generated option hit. */
	virtual FReply NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	/** @param InGeometry Current geometry. @param InGestureEvent Primary-touch release event. @return Base completion reply after same-option validation. */
	virtual FReply NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	/** @param ActionContext Semantic action to evaluate. @return True when the selected Radio owns the resolved internal-axis action. */
	virtual bool NativeCanHandleLunarAction(const FLunarUIActionContext& ActionContext) const override;
	/** @param ActionContext Semantic action to process. @return Handled, Rejected, or Unhandled according to internal selection and edge rules. */
	virtual ELunarUIActionResult NativeHandleLunarAction(const FLunarUIActionContext& ActionContext) override;
	/** @param Direction Physical direction to resolve. @param OutActionTag Receives Increase or Decrease. @return True when this press should change or continue owning internal selection. */
	virtual bool NativeResolveDirectionalLunarControlAction(ELunarNavigationDirection Direction, FGameplayTag& OutActionTag) const override;
	/** @brief Refreshes option-specific styles after the composite Radio becomes Lunar-selected. */
	virtual void NativeOnLunarSelected() override;
	/** @brief Clears active internal-axis ownership and refreshes option-specific styles after unselection. */
	virtual void NativeOnLunarUnselected() override;
	/** @brief Refreshes independently resolved Pressed presentation after valid pointer or navigation input begins. */
	virtual void NativeOnLunarPressed() override;
	/** @brief Refreshes independently resolved Selected/Hovered presentation after input release. */
	virtual void NativeOnLunarReleased() override;
	/** @brief Applies a validated same-option pointer/touch selection before generic activation publication. */
	virtual void NativeOnLunarActivated() override;
	/** @param PreviousState Previous composite state. @param NewState New composite state. @param bIsDesignerPreview Whether Designer preview authored the state. @brief Projects owner state into every internal option and the shared Checked indicator. */
	virtual void NativeOnLunarVisualStateChanged(const FLunarUIVisualState& PreviousState, const FLunarUIVisualState& NewState, bool bIsDesignerPreview) override;
	/** @return Localized selected DisplayText, falling back to technical StringValue. */
	virtual FText NativeGetLunarAccessibleValueText() const override;
#if WITH_EDITOR
	/** @param PropertyChangedEvent Editor property mutation context. @brief Keeps the fixed-size side-data array synchronized with NumOfRadioButtons in Details. */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	/** @brief Clamps count, selection, layout, styles, data size, generated defaults, and interpolation speed. */
	void NormalizeConfiguration();
	/** @param RequiredCount Target data count. @brief Preserves existing data, appends Radio_N defaults, and removes only trailing entries. */
	void NormalizeSideVisualData(int32 RequiredCount);
	/** @brief Recreates generated SideVisual UObject instances only when class or option count changes. */
	void SynchronizeGeneratedSideVisuals();
	/** @brief Pushes current layout, generated children, styles, selection position, and opacity into native Slate. */
	void SynchronizeRadioPresentation();
	/** @param NewIndex Requested index. @param bNotify Whether logical change callbacks are published. @param bWrapTransition Whether this change crossed a configured wrap edge. @return True when logical selection changed. */
	bool ApplySelectedIndex(int32 NewIndex, bool bNotify, bool bWrapTransition);
	/** @param DirectionStep Minus one or plus one. @param bNotify Whether selection callbacks are published. @return True when selection moved. */
	bool ApplySelectionStep(int32 DirectionStep, bool bNotify);
	/** @param PreviousIndex Previous logical selection. @param NewIndex Current logical selection. @brief Publishes owner and external change events once. */
	void NotifySelectedIndexChanged(int32 PreviousIndex, int32 NewIndex);
	/** @param OptionIndex Option to resolve. @return Option-specific state derived from the latest composite state and internal interaction caches. */
	FLunarUIVisualState ResolveOptionVisualState(int32 OptionIndex) const;
	/** @brief Re-resolves all internal option states, applies native styles, and updates generated side visuals. */
	void RefreshOptionVisualStates();
	/** @param NewHoveredIndex Option under the pointer, or INDEX_NONE. @brief Updates independent hover state and presentation. */
	void SetHoveredOptionIndex(int32 NewHoveredIndex);
	/** @param ScreenPosition Absolute pointer coordinate. @return Generated option under the pointer, or INDEX_NONE. */
	int32 ResolvePointerOptionIndex(const FVector2D& ScreenPosition) const;
	/** @param PreviousIndex Previous logical selection. @param NewIndex New logical selection. @param bWrapTransition Whether to use short edge fade instead of cross-list movement. @brief Starts, retargets, or snaps presentation-only Checked animation. */
	void BeginSelectionTransition(int32 PreviousIndex, int32 NewIndex, bool bWrapTransition);
	/** @param DeltaTime Non-negative frame time. @brief Advances normal movement or edge-wrap fade animation. */
	void UpdateSelectionInterpolation(float DeltaTime);
	/** @param NewDisplayedPosition Fractional rendered coordinate. @param bBroadcast Whether to publish the interpolation delegate. @brief Stores and applies one presentation-only position. */
	void SetDisplayedSelectionPositionInternal(float NewDisplayedPosition, bool bBroadcast);
	/** @param NewOpacity Presentation-only Checked opacity. @brief Stores and applies wrap-fade opacity. */
	void SetDisplayedCheckedOpacity(float NewOpacity);
	/** @param bBroadcastPosition Whether a changed snapped position is published. @brief Cancels interpolation and places Checked exactly at SelectedIndex. */
	void SnapDisplayedSelectionToLogical(bool bBroadcastPosition);
	/** @param Progress Normalized transition progress. @return Finite response-curve alpha or linear Progress when no curve exists. */
	float EvaluateSelectionCurve(float Progress) const;
	/** @return True when Designer, Reduce Motion, disabled interpolation, or invalid speed requires immediate presentation. */
	bool MustSnapSelectionPresentation() const;

	/** Generated non-navigable side presentation widgets retained for UObject lifetime and Blueprint access. */
	UPROPERTY(Transient) TArray<TObjectPtr<ULunarRadioSideVisual>> GeneratedSideVisuals;
	/** Native generated-option and one-indicator Slate presentation. */
	TSharedPtr<SLunarRadioVisual> RadioVisual;
	/** Class used to build the current GeneratedSideVisuals snapshot. */
	TSubclassOf<ULunarRadioSideVisual> GeneratedSideVisualClass;
	/** Latest composite owner state used to derive option-specific interactions. */
	UPROPERTY(Transient) FLunarUIVisualState LastOwnerVisualState;
	/** Whether LastOwnerVisualState originated from Designer preview. */
	bool bLastOwnerVisualStateIsDesignerPreview = false;
	/** Mouse-hovered internal option, independent from whole-widget Lunar Selection. */
	int32 HoveredOptionIndex = INDEX_NONE;
	/** Option captured by the active pointer/touch press, or INDEX_NONE. */
	int32 PendingPointerOptionIndex = INDEX_NONE;
	/** Internal option rendered as Navigation Pressed for the active directional press. */
	int32 ActiveNavigationOptionIndex = INDEX_NONE;
	/** Direction step owned until physical release so held repeats cannot unexpectedly leave at an edge. */
	int32 ActiveNavigationDirectionStep = 0;
	/** Whether the active PendingPointerOptionIndex originated from primary touch. */
	bool bPointerOptionPressIsTouch = false;
	/** Current fractional position rendered by the shared Checked indicator. */
	UPROPERTY(Transient) float DisplayedSelectionPosition = 0.0f;
	/** Whether DisplayedSelectionPosition has been initialized from a valid logical selection. */
	bool bDisplayedSelectionPositionInitialized = false;
	/** Current presentation-only opacity rendered by the shared Checked indicator. */
	float DisplayedCheckedOpacity = 1.0f;
	/** Fractional position captured when the active transition began or was retargeted. */
	float SelectionInterpolationSource = 0.0f;
	/** Logical option position targeted by the active transition. */
	float SelectionInterpolationTarget = 0.0f;
	/** Seconds elapsed since the active movement or edge-fade traversal began. */
	float SelectionInterpolationElapsed = 0.0f;
	/** Checked opacity captured when an edge-wrap fade was retargeted. */
	float WrapFadeSourceOpacity = 1.0f;
	/** Whether a presentation-only selection transition is currently active. */
	bool bSelectionInterpolationActive = false;
	/** Whether the active transition uses edge fade instead of fractional cross-list movement. */
	bool bWrapFadeTransitionActive = false;
	/** Whether the wrap transition has moved the hidden Checked indicator to its destination edge. */
	bool bWrapTargetPositionApplied = false;
};
