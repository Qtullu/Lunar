// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Navigation/Core/LunarNavigableWidget.h"
#include "LunarSwitch.generated.h"

/**
 * @file LunarSwitch.h
 * @brief Binary navigable Lunar switch control
 * @ingroup LunarNavigationControls
 */

class SLunarSwitchVisual;

/**
 * @brief Binary value control toggled by Accept and optionally set by one directional axis
 * @ingroup LunarNavigationControls
 */
UCLASS(Blueprintable)
class LUNAR_API ULunarSwitch : public ULunarNavigableWidget
{
	GENERATED_BODY()

public:
	/**
	 * @brief Creates a selectable switch with prompt and pointer support
	 * @param ObjectInitializer Unreal object initializer
	 */
	ULunarSwitch(const FObjectInitializer& ObjectInitializer);

	/**
	 * @brief Sets the logical switch value and emits change notifications when needed
	 * @param bNewIsOn New on/off value
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|Switch")
	void SetIsOn(bool bNewIsOn);

	/**
	 * @brief Checks the logical switch value
	 * @return True when the switch is on
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation|Switch")
	bool IsOn() const { return bIsOn; }

	/** @brief Inverts the current logical switch value. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|Switch")
	void Toggle();

	/** Current logical on/off value. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Switch")
	bool bIsOn = false;

	/** Optional directional axis used to set the switch value directly. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Switch")
	ELunarSwitchDirectionMode DirectionMode = ELunarSwitchDirectionMode::Horizontal;

	/** Broadcast after the logical on/off value changes. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation|Switch")
	FLunarSwitchChangedSignature OnSwitchChanged;

protected:
	/** @brief Creates the native switch track and handle presentation layer. */
	virtual TSharedPtr<SWidget> RebuildLunarSpecializedPresentation() override;
	/**
	 * @brief Releases the native switch presentation
	 * @param bReleaseChildren Whether child Slate resources should also be released
	 */
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	/** @brief Synchronizes authored value, visual state, and native presentation. */
	virtual void SynchronizeProperties() override;
	/**
	 * @brief Checks whether this switch can consume Accept or a claimed direction
	 * @param ActionContext Semantic action and input context being queried
	 * @return True when this switch can handle the action
	 */
	virtual bool NativeCanHandleLunarAction(const FLunarUIActionContext& ActionContext) const override;
	/**
	 * @brief Applies Accept, Increase, or Decrease semantics
	 * @param ActionContext Semantic action and input context to process
	 * @return Handling result reported to the navigation subsystem
	 */
	virtual ELunarUIActionResult NativeHandleLunarAction(const FLunarUIActionContext& ActionContext) override;
	/**
	 * @brief Maps a direction on DirectionMode's axis to Increase or Decrease
	 * @param Direction Physical navigation direction
	 * @param OutActionTag Receives the resolved value-action tag
	 * @return True when the switch claims the supplied direction
	 */
	virtual bool NativeResolveDirectionalLunarControlAction(
		ELunarNavigationDirection Direction,
		FGameplayTag& OutActionTag) const override;
	/** @brief Toggles the switch after a permitted Accept activation. */
	virtual void NativeOnLunarActivated() override;
	/**
	 * @brief Builds accessibility text for the current switch state
	 * @return Localized on/off value text
	 */
	virtual FText NativeGetLunarAccessibleValueText() const override;
	/**
	 * @brief Resolves the compatible switch style and caches specialized fields
	 * @param OutStyle Receives the resolved common style values
	 * @param OutError Receives a configuration error when resolution fails
	 * @return True when the switch style was resolved successfully
	 */
	virtual bool ResolveCommonStylePatch(FLunarCommonStylePatch& OutStyle, FString& OutError) const override;
	/**
	 * @brief Applies common style values and updates the specialized native style
	 * @param ResolvedStyle Resolved common style values
	 */
	virtual void ApplyResolvedCommonStyle(const FLunarCommonStylePatch& ResolvedStyle) override;

private:
	/** @brief Updates the Lunar value-state tag and accessibility value. */
	void ApplyValueState();
	/** @brief Pushes logical value, direction mode, and style into Slate. */
	void SynchronizeSpecializedPresentation();

	/** Specialized style patch produced by the latest style resolution. */
	UPROPERTY(Transient)
	FLunarSwitchStylePatch ResolvedSwitchStyle;

	/** Previously resolved specialized style used to detect target changes. */
	UPROPERTY(Transient)
	FLunarSwitchStylePatch PreviousSwitchStyle;

	/** Native track-and-handle switch presentation. */
	TSharedPtr<SLunarSwitchVisual> SwitchVisual;
};
