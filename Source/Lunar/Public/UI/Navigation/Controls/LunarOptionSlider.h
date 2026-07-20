// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Slate/WidgetTransform.h"
#include "UI/Navigation/Core/LunarNavigableWidget.h"
#include "LunarOptionSlider.generated.h"

/** @file LunarOptionSlider.h @brief Discrete navigable Lunar option-slider control. @ingroup LunarNavigationControls */
class SLunarOptionSliderVisual;

/** @brief Discrete orientation-aware option selector controlled by Increase and Decrease actions. */
UCLASS(Blueprintable)
class LUNAR_API ULunarOptionSlider : public ULunarNavigableWidget
{
	GENERATED_BODY()

public:
	/** @param ObjectInitializer Unreal object initializer. */
	ULunarOptionSlider(const FObjectInitializer& ObjectInitializer);
	/** @param NewOptions Localized option labels to store. @param NotificationPolicy Whether to publish notifications if normalization changes selection; defaults to Notify. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Option Slider", meta = (AdvancedDisplay = "NotificationPolicy")) void SetOptions(const TArray<FText>& NewOptions, ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);
	/** @param NewSelectedIndex Requested option index. @param NotificationPolicy Whether to publish normal change notifications; defaults to Notify. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Option Slider", meta = (AdvancedDisplay = "NotificationPolicy")) void SetSelectedIndex(int32 NewSelectedIndex, ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);
	/** @return Selected index, or INDEX_NONE when empty. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Option Slider") int32 GetSelectedIndex() const { return SelectedIndex; }
	/** @return Selected localized label, or empty text. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Option Slider") FText GetSelectedOption() const;

	/** @param Arrow Arrow whose brush is changed. @param NewBrush New native arrow brush. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Option Slider|Presentation") void SetArrowBrush(ELunarOptionSliderArrow Arrow, const FSlateBrush& NewBrush);
	/** @param Arrow Arrow whose brush is requested. @return Current native brush for the requested arrow. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Option Slider|Presentation") FSlateBrush GetArrowBrush(ELunarOptionSliderArrow Arrow) const;
	/** @param Arrow Arrow whose tint is changed. @param NewTint New native arrow tint. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Option Slider|Presentation") void SetArrowTint(ELunarOptionSliderArrow Arrow, FLinearColor NewTint);
	/** @param Arrow Arrow whose tint is requested. @return Current native tint for the requested arrow. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Option Slider|Presentation") FLinearColor GetArrowTint(ELunarOptionSliderArrow Arrow) const;
	/** @param Arrow Arrow whose transform is changed. @param NewTransform New native render transform. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Option Slider|Presentation") void SetArrowTransform(ELunarOptionSliderArrow Arrow, const FWidgetTransform& NewTransform);
	/** @param Arrow Arrow whose transform is requested. @return Current native render transform for the requested arrow. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Option Slider|Presentation") FWidgetTransform GetArrowTransform(ELunarOptionSliderArrow Arrow) const;
	/** @param NewColor New value-label color. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Option Slider|Presentation") void SetValueTextColor(FSlateColor NewColor);
	/** @return Current value-label color. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Option Slider|Presentation") FSlateColor GetValueTextColor() const { return ValueTextColor; }
	/** @param NewFont New value-label font. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Option Slider|Presentation") void SetValueFont(const FSlateFontInfo& NewFont);
	/** @return Current value-label font. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Option Slider|Presentation") FSlateFontInfo GetValueFont() const { return ValueFont; }
	/** @param Arrow Arrow whose presentation is configured. @param NewBrush New native brush. @param NewTint New native tint. @param NewTransform New native render transform. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Option Slider|Presentation") void ConfigureArrowPresentation(ELunarOptionSliderArrow Arrow, const FSlateBrush& NewBrush, FLinearColor NewTint, const FWidgetTransform& NewTransform);
	/** @param Arrow Arrow whose presentation is requested. @param OutBrush Receives its native brush. @param OutTint Receives its native tint. @param OutTransform Receives its native render transform. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Option Slider|Presentation") void GetArrowPresentation(ELunarOptionSliderArrow Arrow, FSlateBrush& OutBrush, FLinearColor& OutTint, FWidgetTransform& OutTransform) const;
	/** @param Arrow Arrow whose state is requested. @return Independently resolved interaction state for the requested native arrow region. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Option Slider|Presentation") ELunarUIInteractionState GetArrowInteractionState(ELunarOptionSliderArrow Arrow) const;
	/** @brief Configures every native option-slider part in one call. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Option Slider|Presentation")
	void ConfigureOptionSliderPresentation(const FSlateBrush& NewPreviousArrowBrush, FLinearColor NewPreviousArrowTint, const FSlateBrush& NewNextArrowBrush, FLinearColor NewNextArrowTint, FSlateColor NewValueTextColor, const FSlateFontInfo& NewValueFont);
	/** @brief Returns every cached native option-slider presentation value. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Option Slider|Presentation")
	void GetOptionSliderPresentation(FSlateBrush& OutPreviousArrowBrush, FLinearColor& OutPreviousArrowTint, FSlateBrush& OutNextArrowBrush, FLinearColor& OutNextArrowTint, FSlateColor& OutValueTextColor, FSlateFontInfo& OutValueFont) const;

	/** Ordered values available for selection. Newly added empty entries default non-localizable; authors may enable localization explicitly. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Option Slider") TArray<FText> Options;
	/** Index of the selected option. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Option Slider", meta = (ClampMin = "-1")) int32 SelectedIndex = INDEX_NONE;
	/** Whether endpoints wrap. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Option Slider") bool bWrapOptions = false;
	/** Claimed directional axis. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Option Slider") TEnumAsByte<EOrientation> Orientation = Orient_Horizontal;
	/** Whether physical value direction is inverted. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Option Slider") bool bInvertValueDirection = false;
	/** Broadcast after selected index changes. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Option Slider") FLunarOptionSliderIndexChangedSignature OnSelectedIndexChanged;
	/** Broadcast once for each native arrow whose independently resolved interaction state changes. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Option Slider|Events") FLunarOptionSliderArrowVisualStateChangedSignature OnArrowVisualStateChanged;
	/** @param NewIndex Newly selected option index, or INDEX_NONE when the option list becomes empty. @param NewOption Newly selected localized option, or empty text. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lunar|UI|Option Slider|Events", meta = (DisplayName = "On Lunar Selected Index Changed"))
	void BP_OnLunarSelectedIndexChanged(int32 NewIndex, const FText& NewOption);
	/** @param ChangedArrow Arrow whose independently resolved state changed. @param NewState New state of ChangedArrow. @param OtherArrow The opposite arrow. @param OtherArrowState Current state of OtherArrow. @param bIsDesignerPreview True only for a Designer preview publication. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lunar|UI|Option Slider|Events", meta = (DisplayName = "On Lunar Arrow Visual State Changed"))
	void BP_OnLunarArrowVisualStateChanged(ELunarOptionSliderArrow ChangedArrow, ELunarUIInteractionState NewState, ELunarOptionSliderArrow OtherArrow, ELunarUIInteractionState OtherArrowState, bool bIsDesignerPreview);

protected:
	/** @return Lunar presentation wrapped by the OptionSlider-owned pointer interaction surface. */
	virtual TSharedRef<SWidget> RebuildWidget() override;
	/** @return Native arrow and value-label layer. */
	virtual TSharedPtr<SWidget> RebuildLunarSpecializedPresentation() override;
	/** @param bReleaseChildren Whether child Slate resources are released. */
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	/** @brief Normalizes authored state and synchronizes presentation. */
	virtual void SynchronizeProperties() override;
	/** @param InGeometry Current geometry. @param InMouseEvent Pointer event that entered. */
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	/** @param InGeometry Current geometry. @param InMouseEvent Pointer-move event. @return Base routing reply. */
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	/** @param InMouseEvent Pointer event that left. */
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	/** @brief Captures pointer presses before owner-authored child content can consume native Previous/Next interaction. @param InGeometry Cached widget geometry. @param InMouseEvent Mouse event. @return Slate handling reply. */
	virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	/** @brief Captures a previous/next mouse-arrow press. @param InGeometry Cached widget geometry. @param InMouseEvent Mouse event. @return Slate handling reply. */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	/** @brief Validates and completes a mouse-arrow press. @param InGeometry Cached widget geometry. @param InMouseEvent Mouse event. @return Slate handling reply. */
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	/** @brief Clears a pending pointer step after capture loss. @param CaptureLostEvent Slate capture-loss event. */
	virtual void NativeOnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent) override;
	/** @brief Captures a previous/next touch-arrow press. @param InGeometry Cached widget geometry. @param InGestureEvent Touch event. @return Slate handling reply. */
	virtual FReply NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	/** @brief Validates and completes a touch-arrow press. @param InGeometry Cached widget geometry. @param InGestureEvent Touch event. @return Slate handling reply. */
	virtual FReply NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	/** @return True when the semantic action is supported. */
	virtual bool NativeCanHandleLunarAction(const FLunarUIActionContext& ActionContext) const override;
	/** @return Action handling result. */
	virtual ELunarUIActionResult NativeHandleLunarAction(const FLunarUIActionContext& ActionContext) override;
	/** @return True when one direction maps to a value action. */
	virtual bool NativeResolveDirectionalLunarControlAction(ELunarNavigationDirection Direction, FGameplayTag& OutActionTag) const override;
	/** @return Selected option accessibility text. */
	virtual FText NativeGetLunarAccessibleValueText() const override;
	/** @brief Applies the validated pointer-arrow step before publishing the generic Lunar click. */
	virtual void NativeOnLunarActivated() override;
	/** @brief Derives and publishes independent arrow states from the owner state. @param PreviousState Previously published owner state. @param NewState Newly published owner state. @param bIsDesignerPreview True only for Designer preview. */
	virtual void NativeOnLunarVisualStateChanged(const FLunarUIVisualState& PreviousState, const FLunarUIVisualState& NewState, bool bIsDesignerPreview) override;

#if WITH_EDITOR
	/** Ensures newly added empty option labels begin as non-localizable text. */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	/** @return Normalized option index. */
	int32 NormalizeIndex(int32 RequestedIndex) const;
	/** @brief Stores one normalized selection. */
	void ApplySelectedIndex(int32 NewSelectedIndex, bool bNotify);
	/** @brief Publishes one owner override and external delegate notification for the current selected index. */
	void NotifySelectedIndexChanged();
	/** @brief Pushes cached behavior and presentation into Slate. */
	void SynchronizeSpecializedPresentation();
	/** @brief Resolves and publishes the complete Previous/Next arrow state snapshot when it changes. */
	void RefreshArrowVisualState();
	/** @param ChangedArrow Arrow whose newly cached state is published. @brief Publishes one arrow-specific owner event and external delegate notification. */
	void NotifyArrowVisualStateChanged(ELunarOptionSliderArrow ChangedArrow);
	/** @param NewHoveredStep Minus one for Previous, plus one for Next, or zero for neither. @brief Updates the hovered pointer region and republishes arrow state when necessary. */
	void SetHoveredPointerOptionStep(int32 NewHoveredStep);
	/** @param InGeometry Fallback widget geometry. @param ScreenPosition Screen-space pointer coordinate. @return Minus one for Previous, plus one for Next, or zero outside arrow hit zones. */
	int32 ResolvePointerOptionStep(const FGeometry& InGeometry, const FVector2D& ScreenPosition) const;
	/** @param DirectionStep Minus one for Previous or plus one for Next. @return True when the selected option changed. @brief Applies one wrapped or clamped pointer step with normal notifications. */
	bool ApplyPointerOptionStep(int32 DirectionStep);

	/** Cached previous-arrow brush reapplied after rebuild. */
	UPROPERTY(Transient) FSlateBrush PreviousArrowBrush;
	/** Cached previous-arrow tint reapplied after rebuild. */
	UPROPERTY(Transient) FLinearColor PreviousArrowTint = FLinearColor::White;
	/** Cached previous-arrow render transform reapplied after rebuild. */
	UPROPERTY(Transient) FWidgetTransform PreviousArrowTransform;
	/** Cached next-arrow brush reapplied after rebuild. */
	UPROPERTY(Transient) FSlateBrush NextArrowBrush;
	/** Cached next-arrow tint reapplied after rebuild. */
	UPROPERTY(Transient) FLinearColor NextArrowTint = FLinearColor::White;
	/** Cached next-arrow render transform reapplied after rebuild. */
	UPROPERTY(Transient) FWidgetTransform NextArrowTransform;
	/** Cached value-label color reapplied after rebuild. */
	UPROPERTY(Transient) FSlateColor ValueTextColor = FSlateColor::UseForeground();
	/** Cached value-label font reapplied after rebuild. */
	UPROPERTY(Transient) FSlateFontInfo ValueFont;
	/** Native presentation. */
	TSharedPtr<SLunarOptionSliderVisual> OptionSliderVisual;
	/** Previous/next step captured from the active mouse or touch press; zero means no arrow was pressed. */
	int32 PendingPointerOptionStep = 0;
	/** Previous/next region currently hovered by the mouse; zero means neither arrow region. */
	int32 HoveredPointerOptionStep = 0;
	/** Previous/next arrow currently pressed through semantic navigation; zero means no directional press. */
	int32 ActiveNavigationOptionStep = 0;
	/** Whether the pending pointer press originated from primary touch rather than mouse input. */
	bool bPointerOptionPressIsTouch = false;
	/** Last complete owner visual state used to derive independent arrow states. */
	UPROPERTY(Transient) FLunarUIVisualState LastOwnerVisualState;
	/** Current independently resolved previous-arrow interaction state. */
	UPROPERTY(Transient) ELunarUIInteractionState CurrentPreviousArrowInteractionState = ELunarUIInteractionState::PointerNormal;
	/** Current independently resolved next-arrow interaction state. */
	UPROPERTY(Transient) ELunarUIInteractionState CurrentNextArrowInteractionState = ELunarUIInteractionState::PointerNormal;
	/** Whether the arrow-specific visual-state event has published its initial snapshot. */
	bool bHasPublishedArrowVisualState = false;
	/** Whether the last owner visual-state publication came from Designer preview. */
	bool bLastOwnerVisualStateIsDesignerPreview = false;
};