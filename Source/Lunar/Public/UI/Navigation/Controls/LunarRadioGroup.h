// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UI/Navigation/Types/LunarNavigationTypes.h"
#include "LunarRadioGroup.generated.h"

/**
 * @file LunarRadioGroup.h
 * @brief Non-visual selection owner for Lunar radio controls
 * @ingroup LunarNavigationControls
 */

class ULunarRadio;

/**
 * @brief Non-visual owner that keeps one logical radio selection consistent
 * @ingroup LunarNavigationControls
 */
UCLASS(BlueprintType, Blueprintable, EditInlineNew)
class LUNAR_API ULunarRadioGroup : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * @brief Selects a radio assigned to this group
	 * @param Radio Registered radio to select, or null to follow ClearSelectedRadio rules
	 * @return True when the requested group state is valid and was applied
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|Radio")
	bool SetSelectedRadio(ULunarRadio* Radio);

	/**
	 * @brief Selects the registered radio with a stable non-empty ID
	 * @param RadioId ID of the registered radio to select
	 * @return True when the ID identifies a registered radio and selection succeeds
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|Radio")
	bool SetSelectedRadioById(FName RadioId);

	/**
	 * @brief Gets the live selected radio
	 * @return Selected registered radio, or null when the group is empty
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation|Radio")
	ULunarRadio* GetSelectedRadio() const;

	/**
	 * @brief Gets the stable ID of the current or deferred selection
	 * @return Selected radio ID, or NAME_None when no selection exists
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation|Radio")
	FName GetSelectedRadioId() const;

	/**
	 * @brief Clears the group only when selection is optional or no radio remains
	 * @return True when an empty selection is permitted and applied
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|Radio")
	bool ClearSelectedRadio();

	/** Whether a non-empty group must always keep exactly one radio checked. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Radio")
	bool bRequireSelection = true;

	/** Whether Accept on the checked radio may clear an optional-selection group. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Radio", meta = (EditCondition = "!bRequireSelection"))
	bool bAllowDeselect = false;

	/** Stable ID of the selected or deferred radio; use SetSelectedRadioById at runtime. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Radio")
	FName SelectedRadioId = NAME_None;

	/** Broadcast after the group's selected radio changes. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation|Radio")
	FLunarRadioGroupSelectionChangedSignature OnSelectionChanged;

private:
	/**
	 * @brief Adds a radio to the group while validating ID and selection invariants
	 * @param Radio Radio requesting registration
	 * @return True when registration succeeds
	 */
	bool RegisterRadio(ULunarRadio* Radio);
	/**
	 * @brief Removes a radio and normalizes any affected selection
	 * @param Radio Registered radio being removed
	 */
	void UnregisterRadio(ULunarRadio* Radio);
	/**
	 * @brief Applies one group selection transaction and synchronizes checked states
	 * @param NewSelection Registered radio to select, or null for an empty selection
	 * @param bAllowRequiredEmpty Whether this operation may temporarily leave a required group empty
	 * @return True when the requested selection was accepted
	 */
	bool ApplySelection(ULunarRadio* NewSelection, bool bAllowRequiredEmpty);
	/**
	 * @brief Tests whether a live radio belongs to this group
	 * @param Radio Radio to test
	 * @return True when the radio is present in RegisteredRadios
	 */
	bool IsRegisteredRadio(const ULunarRadio* Radio) const;
	/**
	 * @brief Tests whether a radio's non-empty ID is unique within this group
	 * @param Radio Radio whose ID should be tested
	 * @return True when no other registered radio uses the same ID
	 */
	bool IsRadioIdAvailable(const ULunarRadio* Radio) const;
	/** @brief Removes expired and duplicate weak registrations. */
	void CompactRegisteredRadios();
	/** @brief Restores a valid checked selection for a required non-empty group. */
	void NormalizeRequiredSelection();
	/**
	 * @brief Finds a live registered radio by stable ID
	 * @param RadioId Stable radio ID to find
	 * @return Matching radio, or null when no registration matches
	 */
	ULunarRadio* FindRegisteredRadioById(FName RadioId) const;

	/** Weak registrations owned by radio instances rather than the group object. */
	TArray<TWeakObjectPtr<ULunarRadio>> RegisteredRadios;
	/** Live radio selected by the last completed selection transaction. */
	TWeakObjectPtr<ULunarRadio> SelectedRadio;
	/** Selection requested recursively while another transaction is being applied. */
	TWeakObjectPtr<ULunarRadio> DeferredSelectionAfterApply;
	/** Authored or runtime ID waiting for a matching radio to register. */
	FName PendingSelectedRadioId = NAME_None;
	/** Guards checked-state callbacks during a group selection transaction. */
	bool bApplyingSelection = false;
	/** Requests required-selection normalization after the current transaction. */
	bool bNormalizeSelectionAfterApply = false;
	/** Requests finalization of an empty selection after the current transaction. */
	bool bFinalizeEmptySelectionAfterApply = false;

	/** Grants radio instances access to registration and transaction helpers. */
	friend class ULunarRadio;
};
