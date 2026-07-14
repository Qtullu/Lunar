// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Navigation/Core/LunarNavigableWidget.h"
#include "LunarOptionSlider.generated.h"

/**
 * @file LunarOptionSlider.h
 * @brief Discrete navigable Lunar option-slider control
 * @ingroup LunarNavigationControls
 */

class SLunarOptionSliderVisual;

/**
 * @brief Discrete orientation-aware option selector controlled by Increase and Decrease actions
 * @ingroup LunarNavigationControls
 */
UCLASS(Blueprintable)
class LUNAR_API ULunarOptionSlider : public ULunarNavigableWidget
{
	GENERATED_BODY()

public:
	/**
	 * @brief Creates an option slider with selectable pointer-enabled defaults
	 * @param ObjectInitializer Unreal object initializer
	 */
	ULunarOptionSlider(const FObjectInitializer& ObjectInitializer);

	/**
	 * @brief Replaces all options and normalizes the current selection
	 * @param NewOptions Localized option labels to store
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|Option Slider")
	void SetOptions(const TArray<FText>& NewOptions);

	/**
	 * @brief Selects an option after applying bounds and wrapping rules
	 * @param NewSelectedIndex Requested option index
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|Option Slider")
	void SetSelectedIndex(int32 NewSelectedIndex);

	/**
	 * @brief Gets the selected option index
	 * @return Selected index, or INDEX_NONE when the option list is empty
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation|Option Slider")
	int32 GetSelectedIndex() const { return SelectedIndex; }

	/**
	 * @brief Gets the selected option label
	 * @return Selected localized label, or empty text when no option is selected
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation|Option Slider")
	FText GetSelectedOption() const;

	/** Ordered localized values available for selection. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Option Slider")
	TArray<FText> Options;

	/** Index of the selected option, or INDEX_NONE when no option is available. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Option Slider", meta = (ClampMin = "-1"))
	int32 SelectedIndex = INDEX_NONE;

	/** Whether navigation past an endpoint wraps to the opposite endpoint. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Option Slider")
	bool bWrapOptions = false;

	/** Axis claimed for directional Increase and Decrease actions. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Option Slider")
	TEnumAsByte<EOrientation> Orientation = Orient_Horizontal;

	/** Reverses which physical direction increases the selected index. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Option Slider")
	bool bInvertValueDirection = false;

	/** Broadcast after the selected index changes. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation|Option Slider")
	FLunarOptionSliderIndexChangedSignature OnSelectedIndexChanged;

protected:
	/** @brief Creates the native arrows and value-label presentation layer. */
	virtual TSharedPtr<SWidget> RebuildLunarSpecializedPresentation() override;
	/**
	 * @brief Releases the native option-slider presentation
	 * @param bReleaseChildren Whether child Slate resources should also be released
	 */
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	/** @brief Normalizes authored state and synchronizes the native presentation. */
	virtual void SynchronizeProperties() override;
	/**
	 * @brief Checks whether this control can consume a value action
	 * @param ActionContext Semantic action and input context being queried
	 * @return True when this option slider can handle the action
	 */
	virtual bool NativeCanHandleLunarAction(const FLunarUIActionContext& ActionContext) const override;
	/**
	 * @brief Applies Increase or Decrease semantics to the selected option
	 * @param ActionContext Semantic action and input context to process
	 * @return Handling result reported to the navigation subsystem
	 */
	virtual ELunarUIActionResult NativeHandleLunarAction(const FLunarUIActionContext& ActionContext) override;
	/**
	 * @brief Maps one physical direction on the configured axis to Increase or Decrease
	 * @param Direction Physical navigation direction
	 * @param OutActionTag Receives the resolved value-action tag
	 * @return True when this option slider claims the supplied direction
	 */
	virtual bool NativeResolveDirectionalLunarControlAction(
		ELunarNavigationDirection Direction,
		FGameplayTag& OutActionTag) const override;
	/**
	 * @brief Builds accessibility text for the selected option
	 * @return Selected option label exposed to accessibility services
	 */
	virtual FText NativeGetLunarAccessibleValueText() const override;
	/**
	 * @brief Resolves the compatible option-slider style and caches specialized fields
	 * @param OutStyle Receives the resolved common style values
	 * @param OutError Receives a configuration error when resolution fails
	 * @return True when the option-slider style was resolved successfully
	 */
	virtual bool ResolveCommonStylePatch(FLunarCommonStylePatch& OutStyle, FString& OutError) const override;
	/**
	 * @brief Applies common style values and updates the specialized native style
	 * @param ResolvedStyle Resolved common style values
	 */
	virtual void ApplyResolvedCommonStyle(const FLunarCommonStylePatch& ResolvedStyle) override;

private:
	/**
	 * @brief Applies list bounds and optional wrapping to an index
	 * @param RequestedIndex Index to normalize
	 * @return Valid option index, or INDEX_NONE when the list is empty
	 */
	int32 NormalizeIndex(int32 RequestedIndex) const;
	/**
	 * @brief Stores a normalized selection and optionally emits notifications
	 * @param NewSelectedIndex Requested option index
	 * @param bNotify Whether to broadcast and notify accessibility on change
	 */
	void ApplySelectedIndex(int32 NewSelectedIndex, bool bNotify);
	/** @brief Pushes the selected label, orientation, and style into Slate. */
	void SynchronizeSpecializedPresentation();

	/** Specialized style patch produced by the latest style resolution. */
	UPROPERTY(Transient)
	FLunarOptionSliderStylePatch ResolvedOptionSliderStyle;

	/** Previously resolved specialized style used to detect target changes. */
	UPROPERTY(Transient)
	FLunarOptionSliderStylePatch PreviousOptionSliderStyle;

	/** Native arrows and selected-value presentation. */
	TSharedPtr<SLunarOptionSliderVisual> OptionSliderVisual;
};
