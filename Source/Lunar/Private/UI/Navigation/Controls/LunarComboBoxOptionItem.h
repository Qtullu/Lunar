// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarComboBoxOptionItem.h
 * @brief Declares the private ListView adapter for ComboBox option descriptors.
 * @ingroup LunarNavigationControls
 */

#pragma once

#include "CoreMinimal.h"
#include "UI/Navigation/Controls/LunarListViewItem.h"
#include "LunarComboBoxOptionItem.generated.h"

class ULunarComboBox;

/**
 * Internal data adapter that lets the virtualized ListView consume one stable ComboBox option.
 * @ingroup LunarNavigationControls
 */
UCLASS()
class ULunarComboBoxOptionItem final : public UObject, public ILunarListViewItem
{
	GENERATED_BODY()

public:
	/** Binds the adapter to one owner and stable option ID. @param InOwner Owning ComboBox. @param InOptionId Stable option ID. */
	void Initialize(ULunarComboBox* InOwner, FName InOptionId);

	/** Returns the adapted stable option ID. @return Option ID. */
	virtual FName GetItemNavigationId_Implementation() const override;
	/** Reads enabled state from the owning option descriptor. @return True when enabled. */
	virtual bool IsItemNavigationEnabled_Implementation() const override;
	/** Reads disabled-selection policy from the owner. @return True when disabled selection is allowed. */
	virtual bool CanSelectItemWhenDisabled_Implementation() const override;
	/** Reads the localized disabled explanation from the owner. @return Disabled reason text. */
	virtual FText GetItemDisabledReason_Implementation() const override;

private:
	/** ComboBox that owns the authoritative option descriptor. */
	UPROPERTY(Transient)
	TObjectPtr<ULunarComboBox> Owner;

	/** Stable option ID represented by this adapter. */
	UPROPERTY(Transient)
	FName OptionId = NAME_None;
};
