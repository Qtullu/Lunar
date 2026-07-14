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
template <typename ItemType> class SListView;

/**
 * Virtualized composite control whose outer widget exclusively owns Lunar selection and native focus.
 *
 * Rows are transient Slate presentation objects. Stable item IDs, eligibility, activation, and the
 * active item are owned by this widget so virtualization never changes logical navigation state.
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

	/** Replaces the logical data set and rebuilds the stable navigation cache. @param NewItems New interface-backed data objects. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|ListView")
	void SetItems(const TArray<UObject*>& NewItems);

	/** Selects an eligible logical item by stable ID. @param ItemId Stable item ID. @return True when the requested item became active. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|ListView")
	bool SetActiveItemById(FName ItemId);

	/** Returns the active logical data object. @return Active item, or null when no item is active. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation|ListView")
	UObject* GetActiveItem() const;

	/** Returns the stable ID of the active logical item. @return Active ID, or NAME_None when empty. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation|ListView")
	FName GetActiveItemId() const;

	/** Clears the current logical item and broadcasts the resulting state change. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|ListView")
	void ClearActiveItem();

	/** Re-reads stable IDs and eligibility after item data changes, then restores the active item deterministically. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|ListView")
	void RefreshNavigationItems();

	/**
	 * Applies owner-provided row and scroll-bar presentation to the generated Slate list.
	 * @param RowStyle Optional external row style; null restores the ListView style.
	 * @param ScrollBarStyle Optional external scroll-bar style; null restores the ListView style.
	 */
	void SetExternalPresentationStyle(
		const FTableRowStyle* RowStyle,
		const FScrollBarStyle* ScrollBarStyle);

	/**
	 * Installs an owner-specific row-label resolver.
	 * @param ItemTextResolver Callable that maps an item to display text; an empty callable restores stable-ID text.
	 */
	void SetExternalItemTextResolver(TFunction<FText(const UObject*)> ItemTextResolver);

	/** Logical item objects. Every entry must implement ILunarListViewItem and expose a unique, non-empty ID. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|ListView", meta = (MustImplement = "/Script/Lunar.LunarListViewItem"))
	TArray<TObjectPtr<UObject>> Items;

	/** Stable ID of the logical active item. Generated rows never own this state. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|ListView")
	FName ActiveItemId = NAME_None;

	/** Axis used by directional navigation and by the generated Slate list. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|ListView")
	TEnumAsByte<EOrientation> Orientation = Orient_Vertical;

	/** Broadcast after the logical active item changes. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation|ListView")
	FLunarListViewActiveItemChangedSignature OnActiveItemChanged;

protected:
	/** Builds the internal virtualized Slate list. @return Specialized Slate presentation. */
	virtual TSharedPtr<SWidget> RebuildLunarSpecializedPresentation() override;
	/** Releases generated Slate resources. @param bReleaseChildren Whether child Slate resources must also be released. */
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	/** Synchronizes item data, orientation, active state, and presentation styles with Slate. */
	virtual void SynchronizeProperties() override;
	/** Unregisters transient list state during widget destruction. */
	virtual void NativeDestruct() override;
	/** Begins a mouse item press. @param InGeometry Cached widget geometry. @param InMouseEvent Mouse event. @return Handled reply when an eligible row was pressed. */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	/** Completes or cancels a mouse item press. @param InGeometry Cached widget geometry. @param InMouseEvent Mouse event. @return Reply describing whether the release was consumed. */
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	/** Clears pointer press state when Slate capture is lost. @param CaptureLostEvent Slate capture-loss event. */
	virtual void NativeOnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent) override;
	/** Routes wheel scrolling through the generated list. @param InGeometry Cached widget geometry. @param InMouseEvent Wheel event. @return Slate handling reply. */
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	/** Begins touch item tracking. @param InGeometry Cached widget geometry. @param InGestureEvent Touch event. @return Slate handling reply. */
	virtual FReply NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	/** Updates touch tap eligibility while the gesture moves. @param InGeometry Cached widget geometry. @param InGestureEvent Touch event. @return Slate handling reply. */
	virtual FReply NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	/** Completes a touch item tap when it remains eligible. @param InGeometry Cached widget geometry. @param InGestureEvent Touch event. @return Slate handling reply. */
	virtual FReply NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;

	/** Tests whether this list can handle a Lunar action. @param ActionContext Routed action context. @return True when the action is supported in the current list state. */
	virtual bool NativeCanHandleLunarAction(const FLunarUIActionContext& ActionContext) const override;
	/** Handles navigation, activation, and list scrolling actions. @param ActionContext Routed action context. @return Lunar action-routing result. */
	virtual ELunarUIActionResult NativeHandleLunarAction(const FLunarUIActionContext& ActionContext) override;
	/** Tests whether the current active item can be activated. @return True when activation is valid. */
	virtual bool NativeCanActivateLunarWidget() const override;
	/** Announces the disabled reason for a rejected active item. */
	virtual void NativeOnLunarRejected() override;
	/** Builds the accessible value from the active item. @return Localized accessible value text. */
	virtual FText NativeGetLunarAccessibleValueText() const override;
	/** Resolves common and ListView-specific style layers. @param OutStyle Resolved common patch. @param OutError Actionable failure text. @return True on success. */
	virtual bool ResolveCommonStylePatch(FLunarCommonStylePatch& OutStyle, FString& OutError) const override;
	/** Applies a resolved style and schedules ListView-specific presentation updates. @param ResolvedStyle Resolved common style patch. */
	virtual void ApplyResolvedCommonStyle(const FLunarCommonStylePatch& ResolvedStyle) override;

private:
	/** Tests whether a direction moves along the configured list axis. @param Direction Candidate direction. @return True for an axis-aligned direction. */
	bool IsDirectionAlongOrientation(ELunarNavigationDirection Direction) const;
	/** Moves the logical active item in the requested direction. @param Direction Axis-aligned direction. @return True when the item changed. */
	bool MoveActiveItem(ELunarNavigationDirection Direction);
	/** Tests whether an indexed item is a valid logical selection target. @param ItemIndex Cached item index. @return True when selectable. */
	bool IsItemEligible(int32 ItemIndex) const;
	/** Tests the enabled flag cached for an indexed item. @param ItemIndex Cached item index. @return True when enabled. */
	bool IsItemEnabled(int32 ItemIndex) const;
	/** Locates a logical data object by identity. @param Item Item object. @return Item index, or INDEX_NONE. */
	int32 FindLogicalIndexForItem(const UObject* Item) const;
	/** Resolves deterministic restoration after data refresh. @param PreviousItemId Previous stable ID. @param PreviousLogicalIndex Previous index. @return Restored eligible index, or INDEX_NONE. */
	int32 FindRestoredItemIndex(FName PreviousItemId, int32 PreviousLogicalIndex) const;
	/** Resolves owner text or stable-ID fallback text. @param Item Logical item. @return Display text. */
	FText ResolveItemDisplayText(const UObject* Item) const;
	/** Assigns the active item by index. @param ItemIndex Cached item index. @param bBroadcastChange Whether to broadcast the change. */
	void SetActiveItemInternal(int32 ItemIndex, bool bBroadcastChange);
	/** Clears active state. @param bBroadcastChange Whether to broadcast the change. */
	void ClearActiveItemInternal(bool bBroadcastChange);
	/** Copies logical item pointers into the Slate list and requests refresh. */
	void SynchronizeSlateItems();
	/** Scrolls and selects the generated row that represents the active logical item. */
	void SynchronizeSlateActiveItem();
	/** Applies resolved or externally supplied row and scroll-bar styles. */
	void ApplyListViewPresentationStyle();
	/** Emits newly introduced configuration errors and clears resolved deduplication keys. @param CurrentErrors Current error key/message map. */
	void ReportConfigurationErrors(const TMap<FString, FString>& CurrentErrors);
	/** Resolves the Slate user that owns this LocalPlayer widget. @return Slate user index. */
	uint32 GetOwningSlateUserIndex() const;
	/** Finds the generated item beneath a screen-space coordinate. @param ScreenPosition Screen-space pointer position. @return Item object, or null. */
	UObject* FindGeneratedItemAtScreenPosition(const FVector2D& ScreenPosition) const;
	/** Tests whether a row item can begin pointer activation. @param Item Logical item. @return True when pointer eligible. */
	bool IsPointerItemEligible(const UObject* Item) const;
	/** Clears all mouse and touch item-press tracking state. */
	void ResetPointerItemPress();

	/** Generates a Slate row for one logical item. @param Item Logical item. @param OwnerTable Owning table view. @return Generated table row. */
	TSharedRef<ITableRow> HandleGenerateRow(UObject* Item, const TSharedRef<STableViewBase>& OwnerTable);
	/** Applies row metadata after generation. @param Item Logical item. @param Row Generated row. */
	void HandleEntryInitialized(UObject* Item, const TSharedRef<ITableRow>& Row);
	/** Finalizes a pending active-row synchronization. @param Item Logical item. @param Row Generated row, if still available. */
	void HandleItemScrolledIntoView(UObject* Item, const TSharedPtr<ITableRow>& Row);

	/** Last successfully resolved ListView-specific style patch. */
	UPROPERTY(Transient)
	FLunarListViewStylePatch ResolvedListViewStyle;

	/** ListView-specific style currently presented by Slate. */
	UPROPERTY(Transient)
	FLunarListViewStylePatch DisplayedListViewStyle;

	/** Copy of the external row style, retained independently of its caller. */
	UPROPERTY(Transient)
	FTableRowStyle ExternalRowStyle;

	/** Copy of the external scroll-bar style, retained independently of its caller. */
	UPROPERTY(Transient)
	FScrollBarStyle ExternalScrollBarStyle;

	/** Item captured by the active mouse or touch press. */
	UPROPERTY(Transient)
	TObjectPtr<UObject> PointerPressedItem;

	/** Raw item array consumed by SListView. */
	TArray<UObject*> SlateItems;
	/** Stable IDs cached in logical item order. */
	TArray<FName> CachedItemIds;
	/** Enabled flags cached in logical item order. */
	TArray<bool> CachedItemEnabled;
	/** Disabled-selection policy cached in logical item order. */
	TArray<bool> CachedItemSelectableWhenDisabled;
	/** Per-item contract validation result cached in logical order. */
	TArray<bool> CachedItemConfigurationValid;
	/** Localized disabled explanations cached in logical order. */
	TArray<FText> CachedItemDisabledReasons;
	/** Lookup from stable item ID to logical index. */
	TMap<FName, int32> ItemIndexById;
	/** Deduplication keys for configuration errors already reported. */
	TSet<FString> ReportedConfigurationErrors;
	/** Optional owner callback used to display logical item text. */
	TFunction<FText(const UObject*)> ExternalItemTextResolver;
	/** Generated virtualized Slate list. */
	TSharedPtr<SListView<UObject*>> ListViewWidget;
	/** Active item ID awaiting row generation and scroll completion. */
	FName PendingRowGenerationItemId = NAME_None;
	/** Most recently active logical index, retained for deterministic fallback. */
	int32 LastActiveItemIndex = INDEX_NONE;
	/** Screen position captured when a pointer item press began. */
	FVector2D PointerItemPressScreenPosition = FVector2D::ZeroVector;
	/** Whether the current press began over an eligible item. */
	bool bPointerPressedOverEligibleItem = false;
	/** Whether a pointer-item press is currently being tracked. */
	bool bTrackingPointerItemPress = false;
	/** Whether the active touch gesture is still eligible to become a tap. */
	bool bTouchItemTapEligible = false;
	/** Whether ExternalRowStyle currently overrides the resolved row style. */
	bool bHasExternalRowStyle = false;
	/** Whether ExternalScrollBarStyle currently overrides the resolved scroll-bar style. */
	bool bHasExternalScrollBarStyle = false;
	/** Whether specialized style changes still need to be presented to Slate. */
	bool bListViewStylePending = true;
};
