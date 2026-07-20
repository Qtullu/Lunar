// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Navigation/Controls/LunarListView.h"
#include "LunarListViewEntry.generated.h"

/**
 * @file LunarListViewEntry.h
 * @brief Blueprint-owned non-navigable presentation for a generated ListView row.
 * @ingroup LunarNavigationControls
 */

/**
 * @brief Virtualized presentation proxy generated for one logical ListView item.
 * @ingroup LunarNavigationControls
 *
 * The owning ListView initializes every exposed field before PreConstruct and Construct.
 * Entries never register in the outer Lunar navigation graph and may be recycled at any time.
 */
UCLASS(Abstract, Blueprintable)
class LUNAR_API ULunarListViewEntry : public UUserWidget
{
	GENERATED_BODY()

public:
	/** @return ListView that currently owns this generated entry. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ListView|Entry")
	ULunarListView* GetOwningListView() const { return OwningListView; }

	/** @return Current zero-based logical item index. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ListView|Entry")
	int32 GetItemIndex() const { return ItemIndex; }

	/** @return Current logical item data snapshot. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ListView|Entry")
	FLunarListViewItemData GetItemData() const { return ItemData; }

	/** @return Current item-specific visual state resolved by the owning ListView. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ListView|Entry")
	FLunarUIVisualState GetItemVisualState() const { return VisualState; }

	/** @return True when this entry represents the ListView's logical active item. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ListView|Entry")
	bool IsActiveItem() const { return bIsActiveItem; }

	/** @return True when this entry belongs to the ListView's logical selected set. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ListView|Entry")
	bool IsSelectedItem() const { return bIsSelectedItem; }

	/** ListView that generated this virtualized presentation. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|ListView|Entry")
	TObjectPtr<ULunarListView> OwningListView = nullptr;

	/** Zero-based logical index assigned before construction. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|ListView|Entry")
	int32 ItemIndex = INDEX_NONE;

	/** Logical data assigned before construction and refreshed when the owner data changes. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|ListView|Entry")
	FLunarListViewItemData ItemData;

	/** Item-specific pointer, navigation, value, device, and reduced-motion state. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|ListView|Entry")
	FLunarUIVisualState VisualState;

	/** Whether this entry currently represents ActiveItemId. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|ListView|Entry")
	bool bIsActiveItem = false;

	/** Whether this entry currently belongs to SelectedItemIds. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|ListView|Entry")
	bool bIsSelectedItem = false;

protected:
	/** Native extension hook invoked before the Blueprint data-change event. */
	virtual void NativeOnListViewItemDataChanged(
		const FLunarListViewItemData& PreviousData,
		const FLunarListViewItemData& NewData);

	/** Native extension hook invoked before the Blueprint visual-state event. */
	virtual void NativeOnListViewItemVisualStateChanged(
		const FLunarUIVisualState& NewState,
		bool bNewIsActiveItem,
		bool bNewIsSelectedItem);


	/** @param PreviousData Data visible before the update. @param NewData Newly assigned data. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lunar|UI|ListView|Entry|Events", meta = (DisplayName = "On List View Item Data Changed"))
	void BP_OnListViewItemDataChanged(const FLunarListViewItemData& PreviousData, const FLunarListViewItemData& NewData);

	/** @param NewState Newly resolved item-specific state. @param bNewIsActiveItem Whether this is now active. @param bNewIsSelectedItem Whether this belongs to the selected set. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lunar|UI|ListView|Entry|Events", meta = (DisplayName = "On List View Item Visual State Changed"))
	void BP_OnListViewItemVisualStateChanged(const FLunarUIVisualState& NewState, bool bNewIsActiveItem, bool bNewIsSelectedItem);

private:
	/** Initializes all fields before the Blueprint construction lifecycle. @param ListView Owning composite. @param NewItemIndex Logical index. @param InitialData Initial data. @param InitialState Initial visual state. @param bInitiallyActive Initial active flag. */
	void InitializeFromListView(
		ULunarListView* ListView,
		int32 NewItemIndex,
		const FLunarListViewItemData& InitialData,
		const FLunarUIVisualState& InitialState,
		bool bInitiallyActive,
		bool bInitiallySelected);

	/** Applies a runtime data snapshot. @param NewItemIndex Current logical index. @param NewData New data. */
	void ApplyDataFromListView(int32 NewItemIndex, const FLunarListViewItemData& NewData);

	/** Applies current presentation state with independent active and selected flags. */
	void ApplyVisualStateFromListView(const FLunarUIVisualState& NewState, bool bNewIsActiveItem, bool bNewIsSelectedItem);

	/** Whether the first state snapshot has already been published to Blueprint. */
	bool bHasPublishedVisualState = false;

	/** Whether the first data snapshot has already been published to Blueprint. */
	bool bHasPublishedData = false;

	/** Grants the composite owner controlled initialization and refresh access. */
	friend class ULunarListView;
};
