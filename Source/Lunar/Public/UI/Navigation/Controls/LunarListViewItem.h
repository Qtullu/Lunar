// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarListViewItem.h
 * @brief Declares the logical data-item contract consumed by ULunarListView.
 * @ingroup LunarNavigationControls
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LunarListViewItem.generated.h"

/**
 * Unreal interface wrapper for the Blueprint and C++ ListView item contract.
 * @ingroup LunarNavigationControls
 */
UINTERFACE(BlueprintType)
class LUNAR_API ULunarListViewItem : public UInterface
{
	GENERATED_BODY()
};

/**
 * Supplies stable identity, navigation eligibility, and disabled feedback for one logical ListView item.
 * @ingroup LunarNavigationControls
 */
class LUNAR_API ILunarListViewItem
{
	GENERATED_BODY()

public:
	/** Returns the stable, non-empty ID, which must be unique within the owner. @return Stable item ID. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Lunar|UI|Navigation|ListView")
	FName GetItemNavigationId() const;
	/** Native default for GetItemNavigationId. @return Stable item ID, or NAME_None when not implemented. */
	virtual FName GetItemNavigationId_Implementation() const;

	/** Tests whether Accept may activate the item. @return True when enabled. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Lunar|UI|Navigation|ListView")
	bool IsItemNavigationEnabled() const;
	/** Native default for IsItemNavigationEnabled. @return True unless overridden. */
	virtual bool IsItemNavigationEnabled_Implementation() const;

	/** Tests whether a disabled item remains selectable for explanation and rejected feedback. @return True when disabled selection is allowed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Lunar|UI|Navigation|ListView")
	bool CanSelectItemWhenDisabled() const;
	/** Native default for CanSelectItemWhenDisabled. @return False unless overridden. */
	virtual bool CanSelectItemWhenDisabled_Implementation() const;

	/** Returns the explanation announced for an active disabled item. @return Localized disabled reason. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Lunar|UI|Navigation|ListView")
	FText GetItemDisabledReason() const;
	/** Native default for GetItemDisabledReason. @return Empty text unless overridden. */
	virtual FText GetItemDisabledReason_Implementation() const;
};
