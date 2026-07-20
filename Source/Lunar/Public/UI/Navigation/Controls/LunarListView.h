// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarListView.h
 * @brief Declares the virtualized, navigation-aware Lunar list control.
 * @ingroup LunarNavigationControls
 */

#pragma once

#include "CoreMinimal.h"
#include "UI/Navigation/Core/LunarNavigableWidget.h"
#include "LunarListView.generated.h"

class ITableRow;
class STableViewBase;
class SWidget;
template <typename ItemType> class SListView;
class ULunarListView;
class ULunarListViewEntry;

/**
 * Concrete transient identity object used only by the internal Slate list.
 *
 * UObject itself is abstract in UE 5.8, so allocating bare UObject adapters
 * produces editor validation warnings for every logical item.
 *
 * @ingroup LunarNavigationControls
 */
UCLASS(Transient, NotBlueprintable)
class LUNAR_API ULunarListViewItemObject final : public UObject
{
	GENERATED_BODY()
};

/** @brief Determines whether a ListView owns one selected item or an Explorer-style selected set. */
UENUM(BlueprintType)
enum class ELunarListViewSelectionMode : uint8
{
	Single, ///< ActiveItemId is the sole selected item.
	Multi   ///< ActiveItemId is an independent navigation cursor over SelectedItemIds.
};

/**
 * @brief Inline logical data authored directly on a Lunar ListView.
 * @ingroup LunarNavigationControls
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarListViewItemData
{
	GENERATED_BODY()

	/** Stable non-empty ID that must be unique within the owning ListView. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|ListView")
	FName ItemId = NAME_None;

	/** Text exposed to generated entries and accessibility. Defaults non-localizable; authors may enable localization explicitly. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|ListView")
	FText DisplayText = FText::AsCultureInvariant(TEXT(""));

	/** Whether Accept may activate this item. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|ListView")
	bool bEnabled = true;

	/** Whether a disabled item remains internally selectable for explanation and rejected feedback. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|ListView", meta = (EditCondition = "!bEnabled"))
	bool bCanReceiveSelectionWhenDisabled = false;

	/** Explanation announced when activation of this disabled item is rejected. Defaults non-localizable; authors may enable localization explicitly. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|ListView", meta = (EditCondition = "!bEnabled", MultiLine = "true"))
	FText DisabledReason = FText::AsCultureInvariant(TEXT(""));

	/** Optional owner-defined object forwarded unchanged to generated entry presentations. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|ListView")
	TObjectPtr<UObject> Payload = nullptr;
};

/** @brief Broadcast after a ListView successfully activates its current logical item. @ingroup LunarNavigationControls */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FLunarListViewItemActivatedSignature,
	ULunarListView*, ListView,
	int32, ItemIndex,
	FLunarListViewItemData, ItemData);

/** @brief Broadcast after the logical selected-item set changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FLunarListViewSelectedItemsChangedSignature,
	ULunarListView*, ListView,
	const TArray<FName>&, PreviousSelectedItemIds,
	const TArray<FName>&, NewSelectedItemIds);

/**
 * Virtualized composite control whose outer widget exclusively owns Lunar selection and native focus.
 *
 * Rows and optional ULunarListViewEntry instances are transient presentation proxies. Stable item IDs,
 * eligibility, activation, and active-item state remain owned by this widget across virtualization.
 *
 * @ingroup LunarNavigationControls
 */
UCLASS(Blueprintable)
class LUNAR_API ULunarListView : public ULunarNavigableWidget
{
	GENERATED_BODY()

public:
	/** Creates a ListView with the default Lunar navigation policies. @param ObjectInitializer Unreal object initializer. */
	ULunarListView(const FObjectInitializer& ObjectInitializer);

	/** Replaces the inline logical data set and restores active state by stable ID. @param NewItems New item descriptors. @param NotificationPolicy Whether to publish notifications if restoration changes the active item; defaults to Notify. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ListView", meta = (AdvancedDisplay = "NotificationPolicy"))
	void SetItems(const TArray<FLunarListViewItemData>& NewItems, ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);

	/** Appends one valid item. ItemId must be non-empty and unique. @param NewItem Item to append. @param NotificationPolicy Whether active-item restoration may publish notifications. @return True when the item was added. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ListView", meta = (AdvancedDisplay = "NotificationPolicy"))
	bool AddItem(const FLunarListViewItemData& NewItem, ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);

	/** Atomically appends valid items. Every ItemId must be non-empty and unique against both arrays. @param NewItems Items to append. @param NotificationPolicy Whether active-item restoration may publish notifications. @return True when every item was added; false leaves Items unchanged. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ListView", meta = (AdvancedDisplay = "NotificationPolicy"))
	bool AddItems(const TArray<FLunarListViewItemData>& NewItems, ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);

	/** Removes one item by stable ID. @param ItemId Stable ID to remove. @param NotificationPolicy Whether active-item restoration may publish notifications. @return True when an item was removed. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ListView", meta = (AdvancedDisplay = "NotificationPolicy"))
	bool RemoveItem(FName ItemId, ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);

	/** Removes every item whose stable ID appears in ItemIds. @param ItemIds Stable IDs to remove. @param NotificationPolicy Whether active-item restoration may publish notifications. @return Number of removed items. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ListView", meta = (AdvancedDisplay = "NotificationPolicy"))
	int32 RemoveItems(const TArray<FName>& ItemIds, ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);

	/** Removes all logical items and clears the active item. @param NotificationPolicy Whether clearing the active item may publish notifications. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ListView", meta = (AdvancedDisplay = "NotificationPolicy"))
	void ClearItems(ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);

	/** Returns a copy of every authored logical item. @return Current item descriptors in logical order. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ListView")
	TArray<FLunarListViewItemData> GetItems() const { return Items; }

	/** Replaces one logical item and restores active state by stable ID. @param ItemIndex Logical index to update. @param NewItemData Replacement descriptor. @param NotificationPolicy Whether to publish notifications if active state changes. @return True when the index was valid. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ListView", meta = (AdvancedDisplay = "NotificationPolicy"))
	bool SetItemDataAt(int32 ItemIndex, const FLunarListViewItemData& NewItemData, ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);

	/** Returns one logical descriptor. @param ItemIndex Logical index. @return Item data, or a default descriptor when invalid. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ListView")
	FLunarListViewItemData GetItemDataAt(int32 ItemIndex) const;

	/** Returns the logical item count. @return Number of inline descriptors. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ListView")
	int32 GetNumItems() const { return Items.Num(); }

	/** Selects an eligible logical item by stable ID. @param ItemId Stable item ID. @param NotificationPolicy Whether to publish normal change notifications; defaults to Notify. @return True when the requested item became active. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ListView", meta = (AdvancedDisplay = "NotificationPolicy"))
	bool SetActiveItemById(FName ItemId, ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);

	/** Returns the active logical data snapshot. @return Active item, or a default descriptor when empty. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ListView")
	FLunarListViewItemData GetActiveItem() const;

	/** Returns the active logical index. @return Active index, or INDEX_NONE when empty. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ListView")
	int32 GetActiveItemIndex() const;

	/** Returns the stable ID of the active logical item. @return Active ID, or NAME_None when empty. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ListView")
	FName GetActiveItemId() const;

	/** Clears the current logical item. @param NotificationPolicy Whether to publish normal change notifications; defaults to Notify. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ListView", meta = (AdvancedDisplay = "NotificationPolicy"))
	void ClearActiveItem(ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);

	/** Changes between single and Explorer-style multi-selection. @param NewSelectionMode New ownership mode. @param NotificationPolicy Whether normalization publishes selection changes. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ListView|Selection", meta = (AdvancedDisplay = "NotificationPolicy"))
	void SetSelectionMode(ELunarListViewSelectionMode NewSelectionMode, ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);

	/** @return Current selection ownership mode. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ListView|Selection")
	ELunarListViewSelectionMode GetSelectionMode() const { return SelectionMode; }

	/** @return Stable selected IDs in logical item order. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ListView|Selection")
	TArray<FName> GetSelectedItemIds() const { return SelectedItemIds; }

	/** @return Selected item snapshots in logical item order. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ListView|Selection")
	TArray<FLunarListViewItemData> GetSelectedItems() const;

	/** @param ItemId Stable ID to test. @return True when it belongs to the selected set. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ListView|Selection")
	bool IsItemSelected(FName ItemId) const;

	/** Adds or removes one eligible item from the selected set. Single mode selects and activates it. @return True when ItemId resolved to an eligible item. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ListView|Selection", meta = (AdvancedDisplay = "NotificationPolicy"))
	bool SetItemSelected(FName ItemId, bool bSelected, ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);

	/** Atomically replaces the selected set with valid eligible IDs. Single mode keeps only the first logical match. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ListView|Selection", meta = (AdvancedDisplay = "NotificationPolicy"))
	void SetSelectedItemIds(const TArray<FName>& NewSelectedItemIds, ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);

	/** Selects every eligible item in Multi mode; Single mode selects the active item. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ListView|Selection", meta = (AdvancedDisplay = "NotificationPolicy"))
	void SelectAllItems(ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);

	/** Clears the selected set without clearing the active cursor in Multi mode. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ListView|Selection", meta = (AdvancedDisplay = "NotificationPolicy"))
	void ClearSelection(ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);

	/** Revalidates inline data and restores the active item deterministically. @param NotificationPolicy Whether to publish notifications if restoration changes the active item; defaults to Notify. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ListView", meta = (AdvancedDisplay = "NotificationPolicy"))
	void RefreshNavigationItems(ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);

	/**
	 * Applies owner-provided row and scroll-bar presentation to the generated Slate list.
	 * @param RowStyle Optional external row style; null restores the ListView style.
	 * @param ScrollBarStyle Optional external scroll-bar style; null restores the ListView style.
	 */
	void SetExternalPresentationStyle(
		const FTableRowStyle* RowStyle,
		const FScrollBarStyle* ScrollBarStyle);

	/** Applies an optional fallback text style and padding when EntryWidgetClass is null. */
	void SetExternalFallbackPresentation(
		const FTextBlockStyle* TextStyle,
		const FMargin& EntryPadding);

	/** @param NewStyle New generated-row style. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ListView|Presentation")
	void SetRowPresentationStyle(const FTableRowStyle& NewStyle);

	/** @return Current generated-row style. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ListView|Presentation")
	FTableRowStyle GetRowPresentationStyle() const { return RowPresentationStyle; }

	/** @param NewStyle New native scroll-bar style. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ListView|Presentation")
	void SetScrollBarPresentationStyle(const FScrollBarStyle& NewStyle);

	/** @return Current native scroll-bar style. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ListView|Presentation")
	FScrollBarStyle GetScrollBarPresentationStyle() const { return ScrollBarPresentationStyle; }

	/** Configures all native ListView presentation. @param NewRowStyle New row style. @param NewScrollBarStyle New scroll-bar style. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ListView|Presentation")
	void ConfigureListViewPresentation(const FTableRowStyle& NewRowStyle, const FScrollBarStyle& NewScrollBarStyle);

	/** Returns all cached native ListView presentation. @param OutRowStyle Receives the row style. @param OutScrollBarStyle Receives the scroll-bar style. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ListView|Presentation")
	void GetListViewPresentation(FTableRowStyle& OutRowStyle, FScrollBarStyle& OutScrollBarStyle) const;

	/** Returns a currently generated Blueprint entry. @param ItemIndex Logical index. @return Visible entry, or null while virtualized out. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ListView|Presentation")
	ULunarListViewEntry* GetGeneratedEntryAt(int32 ItemIndex) const;

	/** Inline logical items. Every entry must expose a unique, non-empty ItemId. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ListView")
	TArray<FLunarListViewItemData> Items;

	/** Optional Blueprint presentation generated only for visible rows. Null uses native DisplayText fallback. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ListView", meta = (DesignerRebuild, AllowAbstract = "false", BlueprintBaseOnly = "true"))
	TSubclassOf<ULunarListViewEntry> EntryWidgetClass;

#if WITH_EDITORONLY_DATA
	/** Number of temporary dummy entries generated only in the UMG Designer; zero previews the authored Items array. */
	UPROPERTY(EditAnywhere, Category = "Lunar|UI|ListView|Designer Preview", meta = (ClampMin = "0", ClampMax = "20", UIMin = "0", UIMax = "20", DesignerRebuild))
	int32 NumDesignerPreviewEntries = 5;
#endif

	/** Stable ID of the logical active item. Generated rows never own this state. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ListView")
	FName ActiveItemId = NAME_None;

	/** Whether active and selected state are coupled or independently owned. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ListView|Selection")
	ELunarListViewSelectionMode SelectionMode = ELunarListViewSelectionMode::Single;

	/** Stable selected IDs retained in logical item order. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ListView|Selection")
	TArray<FName> SelectedItemIds;

	/** Axis used by directional navigation and by the generated Slate list. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ListView")
	TEnumAsByte<EOrientation> Orientation = Orient_Vertical;

	/** Whether directional navigation past an edge continues from the opposite edge. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ListView")
	bool bWrapNavigation = false;

	/** Broadcast after the logical active item changes. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|ListView")
	FLunarListViewActiveItemChangedSignature OnActiveItemChanged;

	/** Broadcast after the active enabled item successfully activates. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|ListView")
	FLunarListViewItemActivatedSignature OnItemActivated;

	/** Broadcast after the logical selected-item set changes. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|ListView|Selection")
	FLunarListViewSelectedItemsChangedSignature OnSelectedItemsChanged;

protected:
	/** Builds the internal virtualized Slate list. @return Specialized Slate presentation. */
	virtual TSharedPtr<SWidget> RebuildLunarSpecializedPresentation() override;

	/** Releases generated Slate resources. @param bReleaseChildren Whether child Slate resources must also be released. */
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	/** Synchronizes item data, orientation, active state, and native presentation with Slate. */
	virtual void SynchronizeProperties() override;

	/** Runs Blueprint PreConstruct before final Designer validation and row generation. */
	virtual void NativePreConstruct() override;

	/** Unregisters transient list state during widget destruction. */
	virtual void NativeDestruct() override;

	/** Tracks the generated row beneath the mouse. @param InGeometry Cached geometry. @param InMouseEvent Mouse event. @return Slate handling reply. */
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** Clears entry-specific hover state when the pointer leaves. @param InMouseEvent Mouse event. */
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	/** Begins a mouse item press. @param InGeometry Cached widget geometry. @param InMouseEvent Mouse event. @return Handled reply when accepted. */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** Treats a Multi-mode double click as activation after the second release. */
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** Completes or cancels a mouse item press. @param InGeometry Cached widget geometry. @param InMouseEvent Mouse event. @return Slate handling reply. */
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** Clears pointer press state when Slate capture is lost. @param CaptureLostEvent Slate capture-loss event. */
	virtual void NativeOnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent) override;


	/** Begins touch item tracking. @param InGeometry Cached widget geometry. @param InGestureEvent Touch event. @return Slate handling reply. */
	virtual FReply NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;

	/** Updates touch tap eligibility while moving. @param InGeometry Cached widget geometry. @param InGestureEvent Touch event. @return Slate handling reply. */
	virtual FReply NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;

	/** Completes a primary touch item tap. @param InGeometry Cached widget geometry. @param InGestureEvent Touch event. @return Slate handling reply. */
	virtual FReply NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;

	/** Tests whether this list can handle a Lunar action. @param ActionContext Routed action context. @return True when supported. */
	virtual bool NativeCanHandleLunarAction(const FLunarUIActionContext& ActionContext) const override;

	/** Handles navigation and activation. @param ActionContext Routed action context. @return Lunar routing result. */
	virtual ELunarUIActionResult NativeHandleLunarAction(const FLunarUIActionContext& ActionContext) override;

	/** Tests whether the active item can activate. @return True when activation is valid. */
	virtual bool NativeCanActivateLunarWidget() const override;

	/** Publishes item-specific activation after the composite activates. */
	virtual void NativeOnLunarActivated() override;

	/** Announces the disabled reason for a rejected active item. */
	virtual void NativeOnLunarRejected() override;

	/** Projects owner state into generated entries. @param PreviousState Previous composite state. @param NewState New composite state. @param bIsDesignerPreview Whether Designer preview authored the state. */
	virtual void NativeOnLunarVisualStateChanged(const FLunarUIVisualState& PreviousState, const FLunarUIVisualState& NewState, bool bIsDesignerPreview) override;

	/** Builds accessible value text from the active item. @return Localized accessible value. */
	virtual FText NativeGetLunarAccessibleValueText() const override;

	/** Owner Blueprint hook for logical active-item changes. @param PreviousItemId Previous ID. @param NewItemId New ID. @param NewItemIndex New logical index. @param NewItemData New data snapshot. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lunar|UI|ListView|Events", meta = (DisplayName = "On Lunar Active Item Changed"))
	void BP_OnLunarActiveItemChanged(FName PreviousItemId, FName NewItemId, int32 NewItemIndex, FLunarListViewItemData NewItemData);

	/** Owner Blueprint hook for successful item activation. @param ItemIndex Active logical index. @param ItemData Active data snapshot. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lunar|UI|ListView|Events", meta = (DisplayName = "On Lunar Item Activated"))
	void BP_OnLunarItemActivated(int32 ItemIndex, FLunarListViewItemData ItemData);

	/** Owner Blueprint hook for logical selected-set changes. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lunar|UI|ListView|Events", meta = (DisplayName = "On Lunar Selected Items Changed"))
	void BP_OnLunarSelectedItemsChanged(const TArray<FName>& PreviousSelectedItemIds, const TArray<FName>& NewSelectedItemIds);

private:
	/** Internal Explorer-style operation applied to one target index. */
	enum class ESelectionOperation : uint8
	{
		Replace,
		Toggle,
		RangeReplace,
		RangeAdd,
		ActiveOnly
	};
#if WITH_EDITOR
	/** Rebuilds temporary dummy data from NumDesignerPreviewEntries without mutating runtime Items. */
	void RefreshDesignerPreviewItems();
#endif

	/** Tests whether a direction moves along the configured list axis. @param Direction Candidate direction. @return True when axis-aligned. */
	bool IsDirectionAlongOrientation(ELunarNavigationDirection Direction) const;

	/** Moves the active item in logical order and applies the requested selection operation. */
	bool MoveActiveItem(ELunarNavigationDirection Direction, ESelectionOperation SelectionOperation);

	/** Resolves a selection operation from range/additive modifier state. */
	ESelectionOperation ResolveSelectionOperation(bool bRangeModifier, bool bAdditiveModifier) const;

	/** Applies one selection operation to an eligible target. */
	void ApplySelectionOperation(int32 ItemIndex, ESelectionOperation Operation, bool bBroadcastChange);

	/** Replaces selected IDs after validation and logical ordering. */
	void ApplySelectedItemIdsInternal(const TArray<FName>& NewSelectedItemIds, bool bBroadcastChange, bool bUpdateAnchor);

	/** Builds eligible IDs spanning the current anchor and target. */
	TArray<FName> BuildSelectionRange(int32 TargetItemIndex) const;

	/** Returns valid eligible IDs in logical order and enforces the current mode. */
	TArray<FName> NormalizeSelectedItemIds(const TArray<FName>& CandidateIds) const;

	/** Publishes one selected-set change. */
	void BroadcastSelectedItemsChanged(const TArray<FName>& PreviousSelectedItemIds);

	/** Tests whether an item is selectable. @param ItemIndex Logical index. @return True when selectable. */
	bool IsItemEligible(int32 ItemIndex) const;

	/** Tests whether an item is enabled. @param ItemIndex Logical index. @return True when enabled. */
	bool IsItemEnabled(int32 ItemIndex) const;

	/** Tests whether a logical item belongs to SelectedItemIds. */
	bool IsItemSelectedByIndex(int32 ItemIndex) const;

	/** Locates an internal adapter by identity. @param Item Adapter object. @return Logical index, or INDEX_NONE. */
	int32 FindLogicalIndexForItem(const UObject* Item) const;

	/** Resolves deterministic restoration after refresh. @param PreviousItemId Previous ID. @param PreviousLogicalIndex Previous index. @return Restored index, or INDEX_NONE. */
	int32 FindRestoredItemIndex(FName PreviousItemId, int32 PreviousLogicalIndex) const;

	/** Resolves localized DisplayText or stable-ID fallback text. @param Item Internal adapter. @return Display text. */
	FText ResolveItemDisplayText(const UObject* Item) const;

	/** Resolves item-specific interaction and value state. @param ItemIndex Logical index. @return Entry presentation state. */
	FLunarUIVisualState ResolveItemVisualState(int32 ItemIndex) const;

	/** Refreshes data and state on every generated Blueprint entry. */
	void RefreshGeneratedEntries();

	/** Changes the mouse-hovered item. @param NewHoveredItemIndex Logical index or INDEX_NONE. */
	void SetHoveredItemIndex(int32 NewHoveredItemIndex);

	/** Assigns active state by index. @param ItemIndex Logical index. @param bBroadcastChange Whether to notify. */
	void SetActiveItemInternal(int32 ItemIndex, bool bBroadcastChange);

	/** Clears active state. @param bBroadcastChange Whether to notify. */
	void ClearActiveItemInternal(bool bBroadcastChange);

	/** Rebuilds transient adapter objects and refreshes Slate. */
	void SynchronizeSlateItems();

	/** Synchronizes Slate row selection and reveals the adapter representing ActiveItemId. */
	void SynchronizeSlateSelectionAndActiveItem();

	/** Applies cached row and scroll-bar presentation. */
	void ApplyListViewPresentation();

	/** Emits newly introduced configuration errors. @param CurrentErrors Current key/message map. */
	void ReportConfigurationErrors(const TMap<FString, FString>& CurrentErrors);

	/** Resolves the owning Slate user. @return Slate user index. */
	uint32 GetOwningSlateUserIndex() const;

	/** Finds the generated item beneath a screen-space coordinate. @param ScreenPosition Screen position. @return Adapter object, or null. */
	UObject* FindGeneratedItemAtScreenPosition(const FVector2D& ScreenPosition) const;

	/** Tests whether an adapter may start pointer activation. @param Item Adapter object. @return True when eligible. */
	bool IsPointerItemEligible(const UObject* Item) const;

	/** Clears mouse/touch item-press state. */
	void ResetPointerItemPress();

	/** Generates one virtualized row. @param Item Internal adapter. @param OwnerTable Owning table. @return Generated row. */
	TSharedRef<ITableRow> HandleGenerateRow(UObject* Item, const TSharedRef<STableViewBase>& OwnerTable);

	/** Applies row and entry metadata after generation. @param Item Internal adapter. @param Row Generated row. */
	void HandleEntryInitialized(UObject* Item, const TSharedRef<ITableRow>& Row);

	/** Releases the Blueprint entry associated with a recycled row. @param Row Row leaving the generated window. */
	void HandleRowReleased(const TSharedRef<ITableRow>& Row);

	/** Finalizes a pending active-row synchronization. @param Item Internal adapter. @param Row Generated row, if available. */
	void HandleItemScrolledIntoView(UObject* Item, const TSharedPtr<ITableRow>& Row);

	/** Cached generated-row style reapplied after rebuild. */
	UPROPERTY(Transient) FTableRowStyle RowPresentationStyle;

	/** Cached native scroll-bar style reapplied after rebuild. */
	UPROPERTY(Transient) FScrollBarStyle ScrollBarPresentationStyle;
	/** Cached native fallback text style used when EntryWidgetClass is null. */
	UPROPERTY(Transient) FTextBlockStyle FallbackTextStyle;
	/** Cached native padding around fallback DisplayText. */
	UPROPERTY(Transient) FMargin FallbackEntryPadding = FMargin(8.0f, 4.0f);



	/** Internal adapter captured by the active mouse or touch press. */
	UPROPERTY(Transient) TObjectPtr<UObject> PointerPressedItem;

	/** Internal adapter objects retained while Slate references their raw pointers. */
	UPROPERTY(Transient) TArray<TObjectPtr<UObject>> ItemObjects;

	/** Currently generated Blueprint entries; only visible virtualized rows are retained. */
	UPROPERTY(Transient) TArray<TObjectPtr<ULunarListViewEntry>> GeneratedEntries;

#if WITH_EDITORONLY_DATA
	/** Authored Items snapshot retained while the transient Designer instance displays dummy entries. */
	TArray<FLunarListViewItemData> DesignerPreviewItems;

	/** Whether the transient Designer instance currently displays dummy entries. */
	bool bUsingDesignerPreviewItems = false;
#endif

	/** Raw adapter array consumed by SListView. */
	TArray<UObject*> SlateItems;

	/** Stable IDs cached in logical order. */
	TArray<FName> CachedItemIds;

	/** Enabled flags cached in logical order. */
	TArray<bool> CachedItemEnabled;

	/** Disabled-selection flags cached in logical order. */
	TArray<bool> CachedItemSelectableWhenDisabled;

	/** Per-item validation results cached in logical order. */
	TArray<bool> CachedItemConfigurationValid;

	/** Localized disabled explanations cached in logical order. */
	TArray<FText> CachedItemDisabledReasons;

	/** Lookup from stable item ID to logical index. */
	TMap<FName, int32> ItemIndexById;

	/** Lookup from transient adapter identity to logical index. */
	TMap<TObjectKey<UObject>, int32> ItemIndexByObject;

	/** Lookup from generated row identity to its optional Blueprint entry. */
	TMap<const ITableRow*, TWeakObjectPtr<ULunarListViewEntry>> GeneratedEntryByRow;

	/** Deduplication keys for configuration errors already reported. */
	TSet<FString> ReportedConfigurationErrors;

	/** Generated virtualized Slate list. */
	TSharedPtr<SListView<UObject*>> ListViewWidget;

	/** Active item ID awaiting row generation and scroll completion. */
	FName PendingRowGenerationItemId = NAME_None;

	/** Most recently active logical index retained for fallback. */
	int32 LastActiveItemIndex = INDEX_NONE;

	/** Screen position captured when a pointer item press began. */
	FVector2D PointerItemPressScreenPosition = FVector2D::ZeroVector;

	/** Whether the press began over an eligible item. */
	bool bPointerPressedOverEligibleItem = false;

	/** Whether a pointer-item press is being tracked. */
	bool bTrackingPointerItemPress = false;

	/** Whether the active touch gesture remains a tap candidate. */
	bool bTouchItemTapEligible = false;

	/** Mouse-hovered logical item. */
	int32 HoveredItemIndex = INDEX_NONE;

	/** Logical item captured by the current pointer press. */
	int32 PressedItemIndex = INDEX_NONE;

	/** Whether the current item press originated from primary touch. */
	bool bPointerItemPressIsTouch = false;

	/** Range modifier captured at pointer-down time. */
	bool bPointerRangeSelectionModifier = false;

	/** Additive modifier captured at pointer-down time. */
	bool bPointerAdditiveSelectionModifier = false;

	/** Whether the current Multi-mode click should activate on release. */
	bool bActivatePointerItemOnRelease = false;

	/** Stable anchor used by Shift/LB range selection. */
	FName SelectionAnchorItemId = NAME_None;

	/** Latest owner state projected into generated entries. */
	UPROPERTY(Transient) FLunarUIVisualState LastOwnerVisualState;

	/** Whether LastOwnerVisualState originated from Designer preview. */
	bool bLastOwnerVisualStateIsDesignerPreview = false;
};