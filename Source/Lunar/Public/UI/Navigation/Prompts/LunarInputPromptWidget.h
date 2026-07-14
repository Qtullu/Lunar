// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Navigation/Prompts/LunarInputPromptReceiver.h"
#include "UI/Navigation/Types/LunarUIStyleTypes.h"
#include "LunarInputPromptWidget.generated.h"

class ULunarInputPromptStyleAsset;
class ULunarNavigationSubsystem;

/**
 * @file LunarInputPromptWidget.h
 * @brief Standard base class for owner-authored Lunar input-prompt widgets.
 * @ingroup LunarNavigationPrompts
 */

/**
 * @brief Stores resolved prompt snapshots and typed style state for owner-authored presentation.
 * @ingroup LunarNavigationPrompts
 */
UCLASS(Abstract, Blueprintable)
class LUNAR_API ULunarInputPromptWidget : public UUserWidget, public ILunarInputPromptReceiver
{
	GENERATED_BODY()

public:
	/**
	 * @brief Replaces the stored complete prompt snapshot.
	 * @param Actions Complete ordered prompt snapshot; Blueprint overrides should call the parent implementation.
	 */
	virtual void ApplyResolvedPromptActions_Implementation(
		const TArray<FLunarResolvedPromptAction>& Actions) override;

protected:
	/** @brief Refreshes editor and runtime properties inherited from UUserWidget. */
	virtual void SynchronizeProperties() override;

	/** Optional strongly typed style override for this owner-authored prompt presentation. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Prompts|Style")
	TObjectPtr<ULunarInputPromptStyleAsset> StyleAsset;

	/** Final per-instance common style layer applied after the global and assigned prompt styles. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Prompts|Style")
	FLunarCommonStylePatch StyleOverrides;

	/** Complete ordered snapshot most recently delivered through ApplyResolvedPromptActions. */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Prompts")
	TArray<FLunarResolvedPromptAction> ResolvedPromptActions;

	/** Resolved typed style snapshot available to the owner-authored presentation. */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Prompts|Style")
	FLunarInputPromptStylePatch ResolvedPromptStyle;

private:
	/** @brief Resolves the prompt style for the current owner and input-device state. */
	void RefreshResolvedPromptStyle();

	/** @brief Allows the navigation subsystem to validate instance-level prompt style configuration. */
	friend class ULunarNavigationSubsystem;
};
