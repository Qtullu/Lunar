// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Navigation/Core/LunarNavigableWidget.h"
#include "LunarButton.generated.h"

/**
 * @file LunarButton.h
 * @brief Navigable Lunar push-button control
 * @ingroup LunarNavigationControls
 */

/**
 * @brief Reference Lunar push button with release-based navigation and pointer activation
 * @ingroup LunarNavigationControls
 */
UCLASS(Blueprintable)
class LUNAR_API ULunarButton : public ULunarNavigableWidget
{
	GENERATED_BODY()

public:
	/**
	 * @brief Creates a Lunar push button with selectable pointer-enabled defaults
	 * @param ObjectInitializer Unreal object initializer
	 */
	ULunarButton(const FObjectInitializer& ObjectInitializer);

	/**
	 * @brief Performs the same guarded activation used by pointer and navigation release
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|Button")
	void Click();

	/** Broadcast after the button completes a permitted activation. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation|Button")
	FLunarButtonClickedSignature OnClicked;

protected:
	/**
	 * @brief Checks whether this button can consume a semantic Lunar action
	 * @param ActionContext Semantic action and input context being queried
	 * @return True when the action can be handled by this button
	 */
	virtual bool NativeCanHandleLunarAction(const FLunarUIActionContext& ActionContext) const override;

	/**
	 * @brief Handles Accept press and release semantics for the button
	 * @param ActionContext Semantic action and input context to process
	 * @return Handling result reported to the navigation subsystem
	 */
	virtual ELunarUIActionResult NativeHandleLunarAction(const FLunarUIActionContext& ActionContext) override;

	/** @brief Broadcasts the button-specific click event after Lunar activation. */
	virtual void NativeOnLunarActivated() override;

	/**
	 * @brief Resolves the compatible button style chain into a common style patch
	 * @param OutStyle Receives the resolved common style values
	 * @param OutError Receives a configuration error when resolution fails
	 * @return True when the button style was resolved successfully
	 */
	virtual bool ResolveCommonStylePatch(FLunarCommonStylePatch& OutStyle, FString& OutError) const override;
};
