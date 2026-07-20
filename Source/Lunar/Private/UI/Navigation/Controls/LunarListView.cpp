// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarListView.cpp
 * @brief Implements inline item data, virtualized entry presentation, navigation, restoration, and scrolling.
 * @ingroup LunarNavigationControls
 */

#include "UI/Navigation/Controls/LunarListView.h"

#include "Engine/LocalPlayer.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "Styling/StyleDefaults.h"
#include "Subsystems/Console/LunarConsoleSubsystem.h"
#include "UI/Navigation/Controls/LunarListViewEntry.h"
#include "UI/Navigation/Types/LunarGameplayTags.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

#define LOCTEXT_NAMESPACE "LunarListView"

/** Private log channel for actionable ListView configuration failures. */
DEFINE_LOG_CATEGORY_STATIC(LogLunarListView, Log, All);

/** @brief Private helpers for ListView input and presentation. */
namespace LunarListView_Private
{
	/**
	 * Paints and hit-tests like a normal table row while leaving pointer/touch
	 * ownership to the composite ULunarListView that owns every virtual row.
	 */
	class SLunarListViewPassthroughRow final : public STableRow<UObject*>
	{
	public:
		SLATE_BEGIN_ARGS(SLunarListViewPassthroughRow)
			: _Style(&FCoreStyle::Get().GetWidgetStyle<FTableRowStyle>(TEXT("TableView.Row")))
			, _Content()
		{
		}

			SLATE_STYLE_ARGUMENT(FTableRowStyle, Style)
			SLATE_DEFAULT_SLOT(FArguments, Content)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable)
		{
			STableRow<UObject*>::Construct(
				STableRow<UObject*>::FArguments()
				.Style(InArgs._Style)
				[
					InArgs._Content.Widget
				],
				InOwnerTable);
		}

		virtual FReply OnMouseButtonDown(
			const FGeometry&,
			const FPointerEvent&) override
		{
			return FReply::Unhandled();
		}

		virtual FReply OnMouseButtonUp(
			const FGeometry&,
			const FPointerEvent&) override
		{
			return FReply::Unhandled();
		}

		virtual FReply OnMouseButtonDoubleClick(
			const FGeometry&,
			const FPointerEvent&) override
		{
			return FReply::Unhandled();
		}

		virtual FReply OnTouchStarted(
			const FGeometry&,
			const FPointerEvent&) override
		{
			return FReply::Unhandled();
		}

		virtual FReply OnTouchEnded(
			const FGeometry&,
			const FPointerEvent&) override
		{
			return FReply::Unhandled();
		}
	};

	/** Maps a canonical navigation action to its direction. @param ActionTag Canonical action tag. @param OutDirection Resolved direction. @return True when directional. */
	bool ActionToDirection(const FGameplayTag& ActionTag, ELunarNavigationDirection& OutDirection)
	{
		using namespace LunarGameplayTags;
		if (ActionTag == UI_Action_Navigate_Up.GetTag())
		{
			OutDirection = ELunarNavigationDirection::Up;
			return true;
		}
		if (ActionTag == UI_Action_Navigate_Down.GetTag())
		{
			OutDirection = ELunarNavigationDirection::Down;
			return true;
		}
		if (ActionTag == UI_Action_Navigate_Left.GetTag())
		{
			OutDirection = ELunarNavigationDirection::Left;
			return true;
		}
		if (ActionTag == UI_Action_Navigate_Right.GetTag())
		{
			OutDirection = ELunarNavigationDirection::Right;
			return true;
		}
		return false;
	}
}

ULunarListView::ULunarListView(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCanReceiveLunarSelection = true;
	bCanInteractWithPointer = true;
	bEnableInputPrompt = true;
	RowPresentationStyle = FCoreStyle::Get().GetWidgetStyle<FTableRowStyle>(TEXT("TableView.Row"));
	ScrollBarPresentationStyle = FAppStyle::Get().GetWidgetStyle<FScrollBarStyle>(TEXT("ScrollBar"));
	FallbackTextStyle = FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>(TEXT("NormalText"));

	FLunarPromptActionRequest AcceptPrompt;
	AcceptPrompt.ActionTag = LunarGameplayTags::UI_Action_Accept.GetTag();
	PromptActions.Add(AcceptPrompt);
}

void ULunarListView::SetItems(
	const TArray<FLunarListViewItemData>& NewItems,
	const ELunarChangeNotificationPolicy NotificationPolicy)
{
	Items = NewItems;
	RefreshNavigationItems(NotificationPolicy);
}

bool ULunarListView::AddItem(
	const FLunarListViewItemData& NewItem,
	const ELunarChangeNotificationPolicy NotificationPolicy)
{
	if (NewItem.ItemId.IsNone()
		|| Items.ContainsByPredicate([&NewItem](const FLunarListViewItemData& Item)
		{
			return Item.ItemId == NewItem.ItemId;
		}))
	{
		return false;
	}

	Items.Add(NewItem);
	RefreshNavigationItems(NotificationPolicy);
	return true;
}

bool ULunarListView::AddItems(
	const TArray<FLunarListViewItemData>& NewItems,
	const ELunarChangeNotificationPolicy NotificationPolicy)
{
	TSet<FName> KnownItemIds;
	KnownItemIds.Reserve(Items.Num() + NewItems.Num());
	for (const FLunarListViewItemData& Item : Items)
	{
		if (!Item.ItemId.IsNone())
		{
			KnownItemIds.Add(Item.ItemId);
		}
	}

	for (const FLunarListViewItemData& NewItem : NewItems)
	{
		if (NewItem.ItemId.IsNone() || KnownItemIds.Contains(NewItem.ItemId))
		{
			return false;
		}
		KnownItemIds.Add(NewItem.ItemId);
	}

	if (!NewItems.IsEmpty())
	{
		Items.Append(NewItems);
		RefreshNavigationItems(NotificationPolicy);
	}
	return true;
}

bool ULunarListView::RemoveItem(
	const FName ItemId,
	const ELunarChangeNotificationPolicy NotificationPolicy)
{
	const int32 ItemIndex = Items.IndexOfByPredicate([ItemId](const FLunarListViewItemData& Item)
	{
		return Item.ItemId == ItemId;
	});
	if (ItemIndex == INDEX_NONE)
	{
		return false;
	}

	Items.RemoveAt(ItemIndex);
	RefreshNavigationItems(NotificationPolicy);
	return true;
}

int32 ULunarListView::RemoveItems(
	const TArray<FName>& ItemIds,
	const ELunarChangeNotificationPolicy NotificationPolicy)
{
	TSet<FName> ItemIdsToRemove;
	ItemIdsToRemove.Reserve(ItemIds.Num());
	for (const FName ItemId : ItemIds)
	{
		if (!ItemId.IsNone())
		{
			ItemIdsToRemove.Add(ItemId);
		}
	}

	const int32 RemovedCount = Items.RemoveAll([&ItemIdsToRemove](const FLunarListViewItemData& Item)
	{
		return ItemIdsToRemove.Contains(Item.ItemId);
	});
	if (RemovedCount > 0)
	{
		RefreshNavigationItems(NotificationPolicy);
	}
	return RemovedCount;
}

void ULunarListView::ClearItems(const ELunarChangeNotificationPolicy NotificationPolicy)
{
	Items.Reset();
	RefreshNavigationItems(NotificationPolicy);
}

bool ULunarListView::SetItemDataAt(
	const int32 ItemIndex,
	const FLunarListViewItemData& NewItemData,
	const ELunarChangeNotificationPolicy NotificationPolicy)
{
	if (!Items.IsValidIndex(ItemIndex))
	{
		return false;
	}

	Items[ItemIndex] = NewItemData;
	RefreshNavigationItems(NotificationPolicy);
	return true;
}

FLunarListViewItemData ULunarListView::GetItemDataAt(const int32 ItemIndex) const
{
	return Items.IsValidIndex(ItemIndex) ? Items[ItemIndex] : FLunarListViewItemData();
}

bool ULunarListView::SetActiveItemById(
	const FName ItemId,
	const ELunarChangeNotificationPolicy NotificationPolicy)
{
	const int32* ItemIndex = ItemIndexById.Find(ItemId);
	if (!ItemIndex || !IsItemEligible(*ItemIndex))
	{
		return false;
	}

	SetActiveItemInternal(
		*ItemIndex,
		NotificationPolicy == ELunarChangeNotificationPolicy::Notify);
	return true;
}

FLunarListViewItemData ULunarListView::GetActiveItem() const
{
	const int32 ItemIndex = GetActiveItemIndex();
	return Items.IsValidIndex(ItemIndex) ? Items[ItemIndex] : FLunarListViewItemData();
}

int32 ULunarListView::GetActiveItemIndex() const
{
	if (const int32* ItemIndex = ItemIndexById.Find(ActiveItemId))
	{
		return *ItemIndex;
	}
	return INDEX_NONE;
}

FName ULunarListView::GetActiveItemId() const
{
	return ActiveItemId;
}

void ULunarListView::ClearActiveItem(const ELunarChangeNotificationPolicy NotificationPolicy)
{
	ClearActiveItemInternal(NotificationPolicy == ELunarChangeNotificationPolicy::Notify);
}

void ULunarListView::SetSelectionMode(
	const ELunarListViewSelectionMode NewSelectionMode,
	const ELunarChangeNotificationPolicy NotificationPolicy)
{
	if (SelectionMode == NewSelectionMode)
	{
		return;
	}

	SelectionMode = NewSelectionMode;
	const bool bNotify = NotificationPolicy == ELunarChangeNotificationPolicy::Notify;
	if (SelectionMode == ELunarListViewSelectionMode::Single)
	{
		const int32 ActiveIndex = GetActiveItemIndex();
		if (IsItemEligible(ActiveIndex))
		{
			ApplySelectedItemIdsInternal({ ActiveItemId }, bNotify, true);
		}
		else
		{
			ApplySelectedItemIdsInternal({}, bNotify, true);
		}
	}
	else
	{
		ApplySelectedItemIdsInternal(SelectedItemIds, bNotify, false);
		if (SelectionAnchorItemId.IsNone())
		{
			SelectionAnchorItemId = ActiveItemId;
		}
	}
}

TArray<FLunarListViewItemData> ULunarListView::GetSelectedItems() const
{
	TArray<FLunarListViewItemData> Result;
	Result.Reserve(SelectedItemIds.Num());
	for (const FName ItemId : SelectedItemIds)
	{
		if (const int32* ItemIndex = ItemIndexById.Find(ItemId); Items.IsValidIndex(ItemIndex ? *ItemIndex : INDEX_NONE))
		{
			Result.Add(Items[*ItemIndex]);
		}
	}
	return Result;
}

bool ULunarListView::IsItemSelected(const FName ItemId) const
{
	return !ItemId.IsNone() && SelectedItemIds.Contains(ItemId);
}

bool ULunarListView::SetItemSelected(
	const FName ItemId,
	const bool bSelected,
	const ELunarChangeNotificationPolicy NotificationPolicy)
{
	const int32* ItemIndex = ItemIndexById.Find(ItemId);
	if (!ItemIndex || !IsItemEligible(*ItemIndex))
	{
		return false;
	}

	const bool bNotify = NotificationPolicy == ELunarChangeNotificationPolicy::Notify;
	if (SelectionMode == ELunarListViewSelectionMode::Single)
	{
		if (bSelected)
		{
			SetActiveItemInternal(*ItemIndex, bNotify);
		}
		else if (SelectedItemIds.Contains(ItemId))
		{
			ClearActiveItemInternal(bNotify);
		}
		return true;
	}

	TArray<FName> NewSelectedItemIds = SelectedItemIds;
	if (bSelected)
	{
		NewSelectedItemIds.AddUnique(ItemId);
	}
	else
	{
		NewSelectedItemIds.Remove(ItemId);
	}
	ApplySelectedItemIdsInternal(NewSelectedItemIds, bNotify, false);
	SelectionAnchorItemId = ItemId;
	return true;
}

void ULunarListView::SetSelectedItemIds(
	const TArray<FName>& NewSelectedItemIds,
	const ELunarChangeNotificationPolicy NotificationPolicy)
{
	const bool bNotify = NotificationPolicy == ELunarChangeNotificationPolicy::Notify;
	const TArray<FName> NormalizedIds = NormalizeSelectedItemIds(NewSelectedItemIds);
	if (SelectionMode == ELunarListViewSelectionMode::Single)
	{
		if (NormalizedIds.IsEmpty())
		{
			ClearActiveItemInternal(bNotify);
		}
		else
		{
			SetActiveItemInternal(ItemIndexById.FindRef(NormalizedIds[0]), bNotify);
		}
		return;
	}
	ApplySelectedItemIdsInternal(NormalizedIds, bNotify, true);
}

void ULunarListView::SelectAllItems(const ELunarChangeNotificationPolicy NotificationPolicy)
{
	const bool bNotify = NotificationPolicy == ELunarChangeNotificationPolicy::Notify;
	if (SelectionMode == ELunarListViewSelectionMode::Single)
	{
		int32 ItemIndex = GetActiveItemIndex();
		if (!IsItemEligible(ItemIndex))
		{
			ItemIndex = FindRestoredItemIndex(NAME_None, 0);
		}
		if (ItemIndex != INDEX_NONE)
		{
			SetActiveItemInternal(ItemIndex, bNotify);
		}
		return;
	}

	TArray<FName> AllEligibleIds;
	for (int32 ItemIndex = 0; ItemIndex < Items.Num(); ++ItemIndex)
	{
		if (IsItemEligible(ItemIndex))
		{
			AllEligibleIds.Add(CachedItemIds[ItemIndex]);
		}
	}
	ApplySelectedItemIdsInternal(AllEligibleIds, bNotify, false);
}

void ULunarListView::ClearSelection(const ELunarChangeNotificationPolicy NotificationPolicy)
{
	const bool bNotify = NotificationPolicy == ELunarChangeNotificationPolicy::Notify;
	if (SelectionMode == ELunarListViewSelectionMode::Single)
	{
		ClearActiveItemInternal(bNotify);
		return;
	}
	ApplySelectedItemIdsInternal({}, bNotify, true);
}

void ULunarListView::SetRowPresentationStyle(const FTableRowStyle& NewStyle)
{
	RowPresentationStyle = NewStyle;
	ApplyListViewPresentation();
}

void ULunarListView::SetScrollBarPresentationStyle(const FScrollBarStyle& NewStyle)
{
	ScrollBarPresentationStyle = NewStyle;
	ApplyListViewPresentation();
}

void ULunarListView::ConfigureListViewPresentation(
	const FTableRowStyle& NewRowStyle,
	const FScrollBarStyle& NewScrollBarStyle)
{
	RowPresentationStyle = NewRowStyle;
	ScrollBarPresentationStyle = NewScrollBarStyle;
	ApplyListViewPresentation();
}

void ULunarListView::GetListViewPresentation(
	FTableRowStyle& OutRowStyle,
	FScrollBarStyle& OutScrollBarStyle) const
{
	OutRowStyle = RowPresentationStyle;
	OutScrollBarStyle = ScrollBarPresentationStyle;
}

ULunarListViewEntry* ULunarListView::GetGeneratedEntryAt(const int32 ItemIndex) const
{
	for (ULunarListViewEntry* Entry : GeneratedEntries)
	{
		if (IsValid(Entry) && Entry->ItemIndex == ItemIndex)
		{
			return Entry;
		}
	}
	return nullptr;
}

void ULunarListView::RefreshNavigationItems(const ELunarChangeNotificationPolicy NotificationPolicy)
{
	const bool bNotify = NotificationPolicy == ELunarChangeNotificationPolicy::Notify;
	const FName PreviousItemId = ActiveItemId;
	const int32 PreviousLogicalIndex = LastActiveItemIndex;
	const TArray<FName> PreviousSelectedItemIds = SelectedItemIds;
	const int32 ItemCount = Items.Num();

	CachedItemIds.Init(NAME_None, ItemCount);
	CachedItemEnabled.Init(false, ItemCount);
	CachedItemSelectableWhenDisabled.Init(false, ItemCount);
	CachedItemConfigurationValid.Init(false, ItemCount);
	CachedItemDisabledReasons.Init(FText::GetEmpty(), ItemCount);
	ItemIndexById.Reset();

	TMap<FName, int32> ItemIdCounts;
	TMap<FString, FString> CurrentErrors;
	for (int32 ItemIndex = 0; ItemIndex < ItemCount; ++ItemIndex)
	{
		const FLunarListViewItemData& Item = Items[ItemIndex];
		CachedItemIds[ItemIndex] = Item.ItemId;
		CachedItemEnabled[ItemIndex] = Item.bEnabled;
		CachedItemSelectableWhenDisabled[ItemIndex] = Item.bCanReceiveSelectionWhenDisabled;
		CachedItemDisabledReasons[ItemIndex] = Item.DisabledReason;

		if (Item.ItemId.IsNone())
		{
			const FString ErrorKey = FString::Printf(TEXT("MissingId:%d"), ItemIndex);
			CurrentErrors.Add(
				ErrorKey,
				FString::Printf(
					TEXT("%s Items[%d] has an empty ItemId."),
					*GetPathName(),
					ItemIndex));
			continue;
		}
		ItemIdCounts.FindOrAdd(Item.ItemId)++;
	}

	for (const TPair<FName, int32>& Pair : ItemIdCounts)
	{
		if (Pair.Value > 1)
		{
			const FString ErrorKey = FString::Printf(TEXT("DuplicateId:%s"), *Pair.Key.ToString());
			CurrentErrors.Add(
				ErrorKey,
				FString::Printf(
					TEXT("%s contains %d ListView items with duplicate ItemId '%s'. All duplicate entries are excluded from logical navigation."),
					*GetPathName(),
					Pair.Value,
					*Pair.Key.ToString()));
		}
	}

	for (int32 ItemIndex = 0; ItemIndex < ItemCount; ++ItemIndex)
	{
		const FName ItemId = CachedItemIds[ItemIndex];
		const int32* IdCount = ItemIdCounts.Find(ItemId);
		const bool bConfigurationValid = !ItemId.IsNone() && IdCount && *IdCount == 1;
		CachedItemConfigurationValid[ItemIndex] = bConfigurationValid;
		if (bConfigurationValid)
		{
			ItemIndexById.Add(ItemId, ItemIndex);
		}
	}

	ReportConfigurationErrors(CurrentErrors);
	SynchronizeSlateItems();

	if (SelectionMode == ELunarListViewSelectionMode::Multi)
	{
		SelectedItemIds = NormalizeSelectedItemIds(PreviousSelectedItemIds);
		const int32* AnchorIndex = ItemIndexById.Find(SelectionAnchorItemId);
		if (!AnchorIndex || !IsItemEligible(*AnchorIndex))
		{
			SelectionAnchorItemId = NAME_None;
		}
	}

	const int32 RestoredItemIndex = FindRestoredItemIndex(PreviousItemId, PreviousLogicalIndex);
	if (RestoredItemIndex != INDEX_NONE)
	{
		SetActiveItemInternal(RestoredItemIndex, bNotify);
	}
	else
	{
		ClearActiveItemInternal(bNotify);
	}

	if (SelectionMode == ELunarListViewSelectionMode::Multi)
	{
		if (SelectionAnchorItemId.IsNone())
		{
			SelectionAnchorItemId = ActiveItemId;
		}
		SynchronizeSlateSelectionAndActiveItem();
		RefreshGeneratedEntries();
		if (bNotify && PreviousSelectedItemIds != SelectedItemIds)
		{
			BroadcastSelectedItemsChanged(PreviousSelectedItemIds);
		}
	}
}
void ULunarListView::SetExternalPresentationStyle(
	const FTableRowStyle* RowStyle,
	const FScrollBarStyle* ScrollBarStyle)
{
	RowPresentationStyle = RowStyle
		? *RowStyle
		: FCoreStyle::Get().GetWidgetStyle<FTableRowStyle>(TEXT("TableView.Row"));
	ScrollBarPresentationStyle = ScrollBarStyle
		? *ScrollBarStyle
		: FAppStyle::Get().GetWidgetStyle<FScrollBarStyle>(TEXT("ScrollBar"));
	ApplyListViewPresentation();
}

void ULunarListView::SetExternalFallbackPresentation(
	const FTextBlockStyle* TextStyle,
	const FMargin& EntryPadding)
{
	FallbackTextStyle = TextStyle
		? *TextStyle
		: FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>(TEXT("NormalText"));
	FallbackEntryPadding = EntryPadding;
	if (ListViewWidget.IsValid())
	{
		ListViewWidget->RequestListRefresh();
	}
}

TSharedPtr<SWidget> ULunarListView::RebuildLunarSpecializedPresentation()
{
	SAssignNew(ListViewWidget, SListView<UObject*>)
		.ListItemsSource(&SlateItems)
		.OnGenerateRow(SListView<UObject*>::FOnGenerateRow::CreateUObject(
			this,
			&ULunarListView::HandleGenerateRow))
		.OnEntryInitialized(SListView<UObject*>::FOnEntryInitialized::CreateUObject(
			this,
			&ULunarListView::HandleEntryInitialized))
		.OnRowReleased(SListView<UObject*>::FOnWidgetToBeRemoved::CreateUObject(
			this,
			&ULunarListView::HandleRowReleased))
		.OnItemScrolledIntoView(SListView<UObject*>::FOnItemScrolledIntoView::CreateUObject(
			this,
			&ULunarListView::HandleItemScrolledIntoView))
		.SelectionMode(ESelectionMode::Multi)
		.ClearSelectionOnClick(false)
		.Orientation(Orientation)
		.ScrollIntoViewAlignment(EScrollIntoViewAlignment::IntoView)
		.ScrollBarStyle(&ScrollBarPresentationStyle)
		.AllowOverscroll(EAllowOverscroll::No)
		.ConsumeMouseWheel(EConsumeMouseWheel::WhenScrollingPossible)
		.HandleGamepadEvents(false)
		.HandleDirectionalNavigation(false)
		.EnableProximateEntryNavigation(true)
		.IsFocusable(false)
		.Visibility(EVisibility::SelfHitTestInvisible);

	ListViewWidget->SetAccessibleBehavior(EAccessibleBehavior::NotAccessible);
	SynchronizeSlateSelectionAndActiveItem();
	return ListViewWidget;
}

void ULunarListView::ReleaseSlateResources(const bool bReleaseChildren)
{
	GeneratedEntryByRow.Reset();
	GeneratedEntries.Reset();
	ListViewWidget.Reset();
	Super::ReleaseSlateResources(bReleaseChildren);
}

#if WITH_EDITOR
void ULunarListView::RefreshDesignerPreviewItems()
{
#if WITH_EDITORONLY_DATA
	if (!IsDesignTime())
	{
		return;
	}

	if (!bUsingDesignerPreviewItems)
	{
		DesignerPreviewItems = Items;
	}

	if (NumDesignerPreviewEntries <= 0)
	{
		Items = DesignerPreviewItems;
		DesignerPreviewItems.Reset();
		bUsingDesignerPreviewItems = false;
		return;
	}

	bUsingDesignerPreviewItems = true;
	Items.Reset(NumDesignerPreviewEntries);
	for (int32 ItemIndex = 0; ItemIndex < NumDesignerPreviewEntries; ++ItemIndex)
	{
		FLunarListViewItemData& PreviewItem = Items.AddDefaulted_GetRef();
		PreviewItem.ItemId = FName(*FString::Printf(TEXT("DesignerPreview_%02d"), ItemIndex + 1));
		PreviewItem.DisplayText = FText::AsCultureInvariant(
			FString::Printf(TEXT("Preview Item %d"), ItemIndex + 1));
	}
#endif
}
#endif

void ULunarListView::SynchronizeProperties()
{
#if WITH_EDITOR
	RefreshDesignerPreviewItems();
	if (IsDesignTime())
	{
		Super::SynchronizeProperties();
		return;
	}
#endif
	RefreshNavigationItems(ELunarChangeNotificationPolicy::Silent);
	Super::SynchronizeProperties();
}

void ULunarListView::NativePreConstruct()
{
	Super::NativePreConstruct();
#if WITH_EDITOR
	if (IsDesignTime())
	{
		RefreshNavigationItems(ELunarChangeNotificationPolicy::Silent);
	}
#endif
}

void ULunarListView::NativeDestruct()
{
	SetHoveredItemIndex(INDEX_NONE);
	ResetPointerItemPress();
	if (ListViewWidget.IsValid())
	{
		ListViewWidget->CancelScrollIntoView();
	}
	Super::NativeDestruct();
}

FReply ULunarListView::NativeOnMouseMove(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (bCanInteractWithPointer && !InMouseEvent.IsTouchEvent())
	{
		SetHoveredItemIndex(FindLogicalIndexForItem(
			FindGeneratedItemAtScreenPosition(InMouseEvent.GetScreenSpacePosition())));
	}
	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

void ULunarListView::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	SetHoveredItemIndex(INDEX_NONE);
	Super::NativeOnMouseLeave(InMouseEvent);
}

FReply ULunarListView::NativeOnMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	bActivatePointerItemOnRelease = false;
	if (bCanInteractWithPointer && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bTrackingPointerItemPress = true;
		bPointerItemPressIsTouch = false;
		bPointerRangeSelectionModifier = InMouseEvent.IsShiftDown();
		bPointerAdditiveSelectionModifier = InMouseEvent.IsControlDown();
		PointerPressedItem = FindGeneratedItemAtScreenPosition(InMouseEvent.GetScreenSpacePosition());
		PressedItemIndex = FindLogicalIndexForItem(PointerPressedItem.Get());
		bPointerPressedOverEligibleItem = IsPointerItemEligible(PointerPressedItem.Get());
		RefreshGeneratedEntries();
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply ULunarListView::NativeOnMouseButtonDoubleClick(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	FReply Reply = NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	if (SelectionMode == ELunarListViewSelectionMode::Multi
		&& InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton
		&& bTrackingPointerItemPress
		&& !bPointerRangeSelectionModifier
		&& !bPointerAdditiveSelectionModifier)
	{
		bActivatePointerItemOnRelease = true;
	}
	return Reply;
}

FReply ULunarListView::NativeOnMouseButtonUp(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	bool bShouldActivate = false;
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bTrackingPointerItemPress)
	{
		UObject* ReleasedItem = FindGeneratedItemAtScreenPosition(InMouseEvent.GetScreenSpacePosition());
		const bool bValidItemClick = bPointerPressedOverEligibleItem
			&& ReleasedItem == PointerPressedItem.Get()
			&& IsPointerItemEligible(ReleasedItem);
		if (!bValidItemClick)
		{
			CancelPointerPress();
			ResetPointerItemPress();
			NativeOnLunarRejected();
			return FReply::Handled().ReleaseMouseCapture();
		}

		const int32 ReleasedItemIndex = FindLogicalIndexForItem(ReleasedItem);
		const TArray<FName> PreviousSelectedItemIds = SelectedItemIds;
		if (SelectionMode == ELunarListViewSelectionMode::Multi)
		{
			ApplySelectionOperation(
				ReleasedItemIndex,
				ResolveSelectionOperation(
					bPointerRangeSelectionModifier,
					bPointerAdditiveSelectionModifier),
				false);
			bShouldActivate = bActivatePointerItemOnRelease;
		}
		SetActiveItemInternal(ReleasedItemIndex, true);
		if (SelectionMode == ELunarListViewSelectionMode::Multi
			&& PreviousSelectedItemIds != SelectedItemIds)
		{
			BroadcastSelectedItemsChanged(PreviousSelectedItemIds);
		}
	}

	if (SelectionMode == ELunarListViewSelectionMode::Multi && !bShouldActivate)
	{
		CancelPointerPress();
		ResetPointerItemPress();
		return FReply::Handled().ReleaseMouseCapture();
	}

	ResetPointerItemPress();
	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void ULunarListView::NativeOnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent)
{
	ResetPointerItemPress();
	Super::NativeOnMouseCaptureLost(CaptureLostEvent);
}

FReply ULunarListView::NativeOnTouchStarted(
	const FGeometry& InGeometry,
	const FPointerEvent& InGestureEvent)
{
	if (bAllowTouchInput
		&& InGestureEvent.IsTouchEvent()
		&& InGestureEvent.GetPointerIndex() == 0)
	{
		bTrackingPointerItemPress = true;
		bTouchItemTapEligible = true;
		bPointerItemPressIsTouch = true;
		bPointerRangeSelectionModifier = false;
		bPointerAdditiveSelectionModifier = false;
		bActivatePointerItemOnRelease = false;
		PointerItemPressScreenPosition = InGestureEvent.GetScreenSpacePosition();
		PointerPressedItem = FindGeneratedItemAtScreenPosition(InGestureEvent.GetScreenSpacePosition());
		PressedItemIndex = FindLogicalIndexForItem(PointerPressedItem.Get());
		bPointerPressedOverEligibleItem = IsPointerItemEligible(PointerPressedItem.Get());
		RefreshGeneratedEntries();
	}
	return Super::NativeOnTouchStarted(InGeometry, InGestureEvent);
}

FReply ULunarListView::NativeOnTouchMoved(
	const FGeometry& InGeometry,
	const FPointerEvent& InGestureEvent)
{
	if (bTrackingPointerItemPress
		&& InGestureEvent.GetPointerIndex() == 0
		&& FVector2D::Distance(PointerItemPressScreenPosition, InGestureEvent.GetScreenSpacePosition())
			> FSlateApplication::Get().GetDragTriggerDistance())
	{
		bTouchItemTapEligible = false;
		RefreshGeneratedEntries();
	}
	return Super::NativeOnTouchMoved(InGeometry, InGestureEvent);
}

FReply ULunarListView::NativeOnTouchEnded(
	const FGeometry& InGeometry,
	const FPointerEvent& InGestureEvent)
{
	if (InGestureEvent.GetPointerIndex() == 0 && bTrackingPointerItemPress)
	{
		UObject* ReleasedItem = FindGeneratedItemAtScreenPosition(InGestureEvent.GetScreenSpacePosition());
		const bool bValidItemTap = bPointerPressedOverEligibleItem
			&& bTouchItemTapEligible
			&& ReleasedItem == PointerPressedItem.Get()
			&& IsPointerItemEligible(ReleasedItem);
		if (!bValidItemTap)
		{
			CancelPointerPress();
			ResetPointerItemPress();
			NativeOnLunarRejected();
			return FReply::Handled().ReleaseMouseCapture();
		}

		const int32 ReleasedItemIndex = FindLogicalIndexForItem(ReleasedItem);
		const TArray<FName> PreviousSelectedItemIds = SelectedItemIds;
		if (SelectionMode == ELunarListViewSelectionMode::Multi)
		{
			ApplySelectionOperation(ReleasedItemIndex, ESelectionOperation::Replace, false);
		}
		SetActiveItemInternal(ReleasedItemIndex, true);
		if (SelectionMode == ELunarListViewSelectionMode::Multi)
		{
			if (PreviousSelectedItemIds != SelectedItemIds)
			{
				BroadcastSelectedItemsChanged(PreviousSelectedItemIds);
			}
			CancelPointerPress();
			ResetPointerItemPress();
			return FReply::Handled().ReleaseMouseCapture();
		}
	}

	ResetPointerItemPress();
	return Super::NativeOnTouchEnded(InGeometry, InGestureEvent);
}
bool ULunarListView::NativeCanHandleLunarAction(const FLunarUIActionContext& ActionContext) const
{
	if (!IsLunarSelected())
	{
		return Super::NativeCanHandleLunarAction(ActionContext);
	}

	if (ActionContext.ActionTag == LunarGameplayTags::UI_Action_Accept.GetTag())
	{
		return true;
	}
	if (SelectionMode == ELunarListViewSelectionMode::Multi
		&& ActionContext.ActionTag == LunarGameplayTags::UI_Action_Selection_Toggle.GetTag())
	{
		return true;
	}
	if (SelectionMode == ELunarListViewSelectionMode::Multi
		&& ActionContext.bAdditiveSelectionModifier
		&& ActionContext.Key == EKeys::A)
	{
		return true;
	}

	ELunarNavigationDirection Direction = ELunarNavigationDirection::Up;
	if (LunarListView_Private::ActionToDirection(ActionContext.ActionTag, Direction)
		&& IsDirectionAlongOrientation(Direction))
	{
		return true;
	}

	return Super::NativeCanHandleLunarAction(ActionContext);
}

ELunarUIActionResult ULunarListView::NativeHandleLunarAction(
	const FLunarUIActionContext& ActionContext)
{
	if (SelectionMode == ELunarListViewSelectionMode::Multi
		&& ActionContext.ActionTag == LunarGameplayTags::UI_Action_Selection_Toggle.GetTag())
	{
		const int32 ActiveIndex = GetActiveItemIndex();
		if (!IsItemEligible(ActiveIndex))
		{
			return ELunarUIActionResult::Rejected;
		}
		if (ActionContext.InputEvent == IE_Released)
		{
			ApplySelectionOperation(ActiveIndex, ESelectionOperation::Toggle, true);
		}
		return ELunarUIActionResult::Handled;
	}

	if (ActionContext.ActionTag == LunarGameplayTags::UI_Action_Accept.GetTag())
	{
		const bool bToggleWithKeyboard = SelectionMode == ELunarListViewSelectionMode::Multi
			&& ActionContext.bAdditiveSelectionModifier
			&& ActionContext.Key == EKeys::SpaceBar;
		if (bToggleWithKeyboard)
		{
			const int32 ActiveIndex = GetActiveItemIndex();
			if (!IsItemEligible(ActiveIndex))
			{
				return ELunarUIActionResult::Rejected;
			}
			if (ActionContext.InputEvent == IE_Released)
			{
				ApplySelectionOperation(ActiveIndex, ESelectionOperation::Toggle, true);
			}
			return ELunarUIActionResult::Handled;
		}

		if (!NativeCanActivateLunarWidget())
		{
			return ELunarUIActionResult::Rejected;
		}
		if (ActionContext.InputEvent == IE_Released && IsLunarSelected())
		{
			ActivateLunarWidget();
		}
		return ELunarUIActionResult::Handled;
	}

	if (SelectionMode == ELunarListViewSelectionMode::Multi
		&& ActionContext.bAdditiveSelectionModifier
		&& ActionContext.Key == EKeys::A)
	{
		if (ActionContext.InputEvent == IE_Pressed)
		{
			SelectAllItems(ELunarChangeNotificationPolicy::Notify);
		}
		return ELunarUIActionResult::Handled;
	}

	ELunarNavigationDirection Direction = ELunarNavigationDirection::Up;
	if (LunarListView_Private::ActionToDirection(ActionContext.ActionTag, Direction)
		&& IsDirectionAlongOrientation(Direction))
	{
		if (!Super::NativeCanActivateLunarWidget())
		{
			return ELunarUIActionResult::Rejected;
		}
		if (ActionContext.InputEvent == IE_Released)
		{
			return ELunarUIActionResult::Handled;
		}
		return MoveActiveItem(
			Direction,
			ResolveSelectionOperation(
				ActionContext.bRangeSelectionModifier,
				ActionContext.bAdditiveSelectionModifier))
			? ELunarUIActionResult::Handled
			: ELunarUIActionResult::Rejected;
	}

	return Super::NativeHandleLunarAction(ActionContext);
}
bool ULunarListView::NativeCanActivateLunarWidget() const
{
	const int32 ItemIndex = GetActiveItemIndex();
	return Super::NativeCanActivateLunarWidget() && IsItemEnabled(ItemIndex);
}

void ULunarListView::NativeOnLunarActivated()
{
	Super::NativeOnLunarActivated();
	const int32 ItemIndex = GetActiveItemIndex();
	if (!Items.IsValidIndex(ItemIndex))
	{
		return;
	}

	const FLunarListViewItemData ItemData = Items[ItemIndex];
	BP_OnLunarItemActivated(ItemIndex, ItemData);
	OnItemActivated.Broadcast(this, ItemIndex, ItemData);
}

void ULunarListView::NativeOnLunarRejected()
{
	Super::NativeOnLunarRejected();
	const int32 ItemIndex = GetActiveItemIndex();
	if (CachedItemDisabledReasons.IsValidIndex(ItemIndex)
		&& !IsItemEnabled(ItemIndex)
		&& !CachedItemDisabledReasons[ItemIndex].IsEmpty())
	{
		NotifyLunarAccessibleValueChanged(CachedItemDisabledReasons[ItemIndex]);
	}
}

void ULunarListView::NativeOnLunarVisualStateChanged(
	const FLunarUIVisualState& PreviousState,
	const FLunarUIVisualState& NewState,
	const bool bIsDesignerPreview)
{
	LastOwnerVisualState = NewState;
	bLastOwnerVisualStateIsDesignerPreview = bIsDesignerPreview;
	Super::NativeOnLunarVisualStateChanged(PreviousState, NewState, bIsDesignerPreview);
	RefreshGeneratedEntries();
}

FText ULunarListView::NativeGetLunarAccessibleValueText() const
{
	const int32 ItemIndex = GetActiveItemIndex();
	if (!Items.IsValidIndex(ItemIndex))
	{
		return LOCTEXT("NoActiveItem", "No active item");
	}

	const FLunarListViewItemData& Item = Items[ItemIndex];
	const FText ItemText = Item.DisplayText.IsEmpty()
		? FText::FromName(Item.ItemId)
		: Item.DisplayText;
	if (!Item.bEnabled && !Item.DisabledReason.IsEmpty())
	{
		return FText::Format(
			LOCTEXT("DisabledActiveItem", "{0}. Unavailable: {1}"),
			ItemText,
			Item.DisabledReason);
	}
	return ItemText;
}

bool ULunarListView::IsDirectionAlongOrientation(
	const ELunarNavigationDirection Direction) const
{
	return Orientation == Orient_Vertical
		? Direction == ELunarNavigationDirection::Up || Direction == ELunarNavigationDirection::Down
		: Direction == ELunarNavigationDirection::Left || Direction == ELunarNavigationDirection::Right;
}

ULunarListView::ESelectionOperation ULunarListView::ResolveSelectionOperation(
	const bool bRangeModifier,
	const bool bAdditiveModifier) const
{
	if (SelectionMode == ELunarListViewSelectionMode::Single)
	{
		return ESelectionOperation::Replace;
	}
	if (bRangeModifier)
	{
		return bAdditiveModifier ? ESelectionOperation::RangeAdd : ESelectionOperation::RangeReplace;
	}
	return bAdditiveModifier ? ESelectionOperation::ActiveOnly : ESelectionOperation::Replace;
}

TArray<FName> ULunarListView::NormalizeSelectedItemIds(const TArray<FName>& CandidateIds) const
{
	TSet<FName> CandidateSet;
	CandidateSet.Reserve(CandidateIds.Num());
	for (const FName CandidateId : CandidateIds)
	{
		if (!CandidateId.IsNone())
		{
			CandidateSet.Add(CandidateId);
		}
	}

	TArray<FName> Result;
	for (int32 ItemIndex = 0; ItemIndex < Items.Num(); ++ItemIndex)
	{
		if (IsItemEligible(ItemIndex) && CandidateSet.Contains(CachedItemIds[ItemIndex]))
		{
			Result.Add(CachedItemIds[ItemIndex]);
			if (SelectionMode == ELunarListViewSelectionMode::Single)
			{
				break;
			}
		}
	}
	return Result;
}

void ULunarListView::ApplySelectedItemIdsInternal(
	const TArray<FName>& NewSelectedItemIds,
	const bool bBroadcastChange,
	const bool bUpdateAnchor)
{
	const TArray<FName> PreviousSelectedItemIds = SelectedItemIds;
	SelectedItemIds = NormalizeSelectedItemIds(NewSelectedItemIds);
	if (bUpdateAnchor)
	{
		SelectionAnchorItemId = SelectedItemIds.IsEmpty() ? NAME_None : SelectedItemIds.Last();
	}

	SynchronizeSlateSelectionAndActiveItem();
	RefreshGeneratedEntries();
	if (bBroadcastChange && PreviousSelectedItemIds != SelectedItemIds)
	{
		BroadcastSelectedItemsChanged(PreviousSelectedItemIds);
	}
}

TArray<FName> ULunarListView::BuildSelectionRange(const int32 TargetItemIndex) const
{
	TArray<FName> Result;
	if (!IsItemEligible(TargetItemIndex))
	{
		return Result;
	}

	const int32* StoredAnchorIndex = ItemIndexById.Find(SelectionAnchorItemId);
	const int32 AnchorIndex = StoredAnchorIndex && IsItemEligible(*StoredAnchorIndex)
		? *StoredAnchorIndex
		: TargetItemIndex;
	const int32 FirstIndex = FMath::Min(AnchorIndex, TargetItemIndex);
	const int32 LastIndex = FMath::Max(AnchorIndex, TargetItemIndex);
	for (int32 ItemIndex = FirstIndex; ItemIndex <= LastIndex; ++ItemIndex)
	{
		if (IsItemEligible(ItemIndex))
		{
			Result.Add(CachedItemIds[ItemIndex]);
		}
	}
	return Result;
}

void ULunarListView::ApplySelectionOperation(
	const int32 ItemIndex,
	const ESelectionOperation Operation,
	const bool bBroadcastChange)
{
	if (!IsItemEligible(ItemIndex) || SelectionMode == ELunarListViewSelectionMode::Single)
	{
		return;
	}

	const FName TargetId = CachedItemIds[ItemIndex];
	TArray<FName> NewSelectedItemIds = SelectedItemIds;
	switch (Operation)
	{
	case ESelectionOperation::Replace:
		NewSelectedItemIds = { TargetId };
		SelectionAnchorItemId = TargetId;
		break;
	case ESelectionOperation::Toggle:
		if (NewSelectedItemIds.Contains(TargetId))
		{
			NewSelectedItemIds.Remove(TargetId);
		}
		else
		{
			NewSelectedItemIds.Add(TargetId);
		}
		SelectionAnchorItemId = TargetId;
		break;
	case ESelectionOperation::RangeReplace:
		NewSelectedItemIds = BuildSelectionRange(ItemIndex);
		break;
	case ESelectionOperation::RangeAdd:
		for (const FName RangeId : BuildSelectionRange(ItemIndex))
		{
			NewSelectedItemIds.AddUnique(RangeId);
		}
		break;
	case ESelectionOperation::ActiveOnly:
		SynchronizeSlateSelectionAndActiveItem();
		RefreshGeneratedEntries();
		return;
	default:
		return;
	}

	ApplySelectedItemIdsInternal(NewSelectedItemIds, bBroadcastChange, false);
}

void ULunarListView::BroadcastSelectedItemsChanged(
	const TArray<FName>& PreviousSelectedItemIds)
{
	BP_OnLunarSelectedItemsChanged(PreviousSelectedItemIds, SelectedItemIds);
	OnSelectedItemsChanged.Broadcast(this, PreviousSelectedItemIds, SelectedItemIds);
}

bool ULunarListView::MoveActiveItem(
	const ELunarNavigationDirection Direction,
	const ESelectionOperation SelectionOperation)
{
	const bool bForward = Direction == ELunarNavigationDirection::Down
		|| Direction == ELunarNavigationDirection::Right;
	int32 StartingIndex = GetActiveItemIndex();
	if (StartingIndex == INDEX_NONE)
	{
		StartingIndex = bForward ? -1 : Items.Num();
	}

	const int32 Step = bForward ? 1 : -1;
	for (int32 ItemIndex = StartingIndex + Step; Items.IsValidIndex(ItemIndex); ItemIndex += Step)
	{
		if (IsItemEligible(ItemIndex))
		{
			const TArray<FName> PreviousSelectedItemIds = SelectedItemIds;
			if (SelectionMode == ELunarListViewSelectionMode::Multi)
			{
				ApplySelectionOperation(ItemIndex, SelectionOperation, false);
			}
			SetActiveItemInternal(ItemIndex, true);
			if (SelectionMode == ELunarListViewSelectionMode::Multi
				&& PreviousSelectedItemIds != SelectedItemIds)
			{
				BroadcastSelectedItemsChanged(PreviousSelectedItemIds);
			}
			return true;
		}
	}
	if (bWrapNavigation && !Items.IsEmpty())
	{
		const int32 WrappedStart = bForward ? 0 : Items.Num() - 1;
		for (int32 ItemIndex = WrappedStart;
			Items.IsValidIndex(ItemIndex) && ItemIndex != StartingIndex;
			ItemIndex += Step)
		{
			if (IsItemEligible(ItemIndex))
			{
				const TArray<FName> PreviousSelectedItemIds = SelectedItemIds;
				if (SelectionMode == ELunarListViewSelectionMode::Multi)
				{
					ApplySelectionOperation(ItemIndex, SelectionOperation, false);
				}
				SetActiveItemInternal(ItemIndex, true);
				if (ListViewWidget.IsValid())
				{
					// A wrapped boundary must land on the exact opposite edge. The
					// normal item-index reveal can stop early for virtualized rows of
					// varying width, especially in horizontal lists.
					ListViewWidget->CancelScrollIntoView();
					PendingRowGenerationItemId = NAME_None;
					if (bForward)
					{
						ListViewWidget->ScrollToTop();
					}
					else
					{
						ListViewWidget->ScrollToBottom();
					}
				}
				if (SelectionMode == ELunarListViewSelectionMode::Multi
					&& PreviousSelectedItemIds != SelectedItemIds)
				{
					BroadcastSelectedItemsChanged(PreviousSelectedItemIds);
				}
				return true;
			}
		}
		return IsItemEligible(StartingIndex);
	}
	return false;
}
bool ULunarListView::IsItemEligible(const int32 ItemIndex) const
{
	return CachedItemConfigurationValid.IsValidIndex(ItemIndex)
		&& CachedItemConfigurationValid[ItemIndex]
		&& (CachedItemEnabled[ItemIndex] || CachedItemSelectableWhenDisabled[ItemIndex]);
}

bool ULunarListView::IsItemEnabled(const int32 ItemIndex) const
{
	return CachedItemConfigurationValid.IsValidIndex(ItemIndex)
		&& CachedItemConfigurationValid[ItemIndex]
		&& CachedItemEnabled[ItemIndex];
}

bool ULunarListView::IsItemSelectedByIndex(const int32 ItemIndex) const
{
	return CachedItemIds.IsValidIndex(ItemIndex)
		&& SelectedItemIds.Contains(CachedItemIds[ItemIndex]);
}

int32 ULunarListView::FindLogicalIndexForItem(const UObject* Item) const
{
	if (!Item)
	{
		return INDEX_NONE;
	}
	if (const int32* ItemIndex = ItemIndexByObject.Find(TObjectKey<UObject>(const_cast<UObject*>(Item))))
	{
		return *ItemIndex;
	}
	return INDEX_NONE;
}

int32 ULunarListView::FindRestoredItemIndex(
	const FName PreviousItemId,
	const int32 PreviousLogicalIndex) const
{
	if (const int32* ExistingIndex = ItemIndexById.Find(PreviousItemId))
	{
		if (IsItemEligible(*ExistingIndex))
		{
			return *ExistingIndex;
		}
	}

	if (Items.IsEmpty())
	{
		return INDEX_NONE;
	}

	const int32 SearchStart = PreviousLogicalIndex == INDEX_NONE
		? 0
		: FMath::Clamp(PreviousLogicalIndex, 0, Items.Num() - 1);
	for (int32 ItemIndex = SearchStart; ItemIndex < Items.Num(); ++ItemIndex)
	{
		if (IsItemEligible(ItemIndex))
		{
			return ItemIndex;
		}
	}
	for (int32 ItemIndex = SearchStart - 1; ItemIndex >= 0; --ItemIndex)
	{
		if (IsItemEligible(ItemIndex))
		{
			return ItemIndex;
		}
	}
	return INDEX_NONE;
}

FText ULunarListView::ResolveItemDisplayText(const UObject* Item) const
{
	const int32 ItemIndex = FindLogicalIndexForItem(Item);
	if (!Items.IsValidIndex(ItemIndex))
	{
		return FText::GetEmpty();
	}
	return Items[ItemIndex].DisplayText.IsEmpty()
		? FText::FromName(Items[ItemIndex].ItemId)
		: Items[ItemIndex].DisplayText;
}

FLunarUIVisualState ULunarListView::ResolveItemVisualState(const int32 ItemIndex) const
{
	FLunarUIVisualState Result = LastOwnerVisualState;
	const bool bActive = ItemIndex == GetActiveItemIndex();
	const bool bOwnerDisabled =
		LastOwnerVisualState.ValueStateTag == LunarGameplayTags::UI_State_Value_Disabled.GetTag();
	Result.ValueStateTag = bOwnerDisabled || !IsItemEnabled(ItemIndex)
		? LunarGameplayTags::UI_State_Value_Disabled.GetTag()
		: LunarGameplayTags::UI_State_Value_Normal.GetTag();

	const ELunarUIInteractionState OwnerInteraction = LastOwnerVisualState.InteractionState;
	const bool bOwnerPointerState = OwnerInteraction == ELunarUIInteractionState::PointerNormal
		|| OwnerInteraction == ELunarUIInteractionState::PointerHovered
		|| OwnerInteraction == ELunarUIInteractionState::PointerPressed;
	const bool bLocalPointerInteraction = !bLastOwnerVisualStateIsDesignerPreview
		&& (HoveredItemIndex != INDEX_NONE || PressedItemIndex != INDEX_NONE);
	if (bOwnerPointerState || bLocalPointerInteraction)
	{
		const int32 EffectiveHoveredIndex = bLastOwnerVisualStateIsDesignerPreview
			? GetActiveItemIndex()
			: HoveredItemIndex;
		const int32 EffectivePressedIndex = bLastOwnerVisualStateIsDesignerPreview
			? GetActiveItemIndex()
			: (bPointerItemPressIsTouch || HoveredItemIndex == PressedItemIndex
				? PressedItemIndex
				: INDEX_NONE);
		const bool bPointerPressActive = OwnerInteraction == ELunarUIInteractionState::PointerPressed
			|| PressedItemIndex != INDEX_NONE;
		Result.InteractionState = bPointerPressActive && ItemIndex == EffectivePressedIndex
			? ELunarUIInteractionState::PointerPressed
			: (ItemIndex == EffectiveHoveredIndex
				? ELunarUIInteractionState::PointerHovered
				: ELunarUIInteractionState::PointerNormal);
		return Result;
	}

	if (OwnerInteraction == ELunarUIInteractionState::NavigationPressed)
	{
		Result.InteractionState = bActive
			? ELunarUIInteractionState::NavigationPressed
			: ELunarUIInteractionState::NavigationNormal;
	}
	else if (OwnerInteraction == ELunarUIInteractionState::NavigationSelected && bActive)
	{
		Result.InteractionState = ELunarUIInteractionState::NavigationSelected;
	}
	else
	{
		Result.InteractionState = ELunarUIInteractionState::NavigationNormal;
	}
	return Result;
}

void ULunarListView::RefreshGeneratedEntries()
{
	for (ULunarListViewEntry* Entry : GeneratedEntries)
	{
		if (!IsValid(Entry) || !Items.IsValidIndex(Entry->ItemIndex))
		{
			continue;
		}
		const int32 ItemIndex = Entry->ItemIndex;
		Entry->ApplyDataFromListView(ItemIndex, Items[ItemIndex]);
		Entry->ApplyVisualStateFromListView(
			ResolveItemVisualState(ItemIndex),
			ItemIndex == GetActiveItemIndex(),
			IsItemSelectedByIndex(ItemIndex));
	}
}

void ULunarListView::SetHoveredItemIndex(const int32 NewHoveredItemIndex)
{
	const int32 NormalizedIndex = Items.IsValidIndex(NewHoveredItemIndex)
		? NewHoveredItemIndex
		: INDEX_NONE;
	if (HoveredItemIndex == NormalizedIndex)
	{
		return;
	}
	HoveredItemIndex = NormalizedIndex;
	RefreshGeneratedEntries();
}

void ULunarListView::SetActiveItemInternal(
	const int32 ItemIndex,
	const bool bBroadcastChange)
{
	if (!IsItemEligible(ItemIndex))
	{
		return;
	}

	const FName PreviousItemId = ActiveItemId;
	const TArray<FName> PreviousSelectedItemIds = SelectedItemIds;
	ActiveItemId = CachedItemIds[ItemIndex];
	LastActiveItemIndex = ItemIndex;
	PendingRowGenerationItemId = ActiveItemId;
	if (SelectionMode == ELunarListViewSelectionMode::Single)
	{
		SelectedItemIds = { ActiveItemId };
		SelectionAnchorItemId = ActiveItemId;
	}
	else if (SelectionAnchorItemId.IsNone())
	{
		SelectionAnchorItemId = ActiveItemId;
	}
	SynchronizeSlateSelectionAndActiveItem();
	RefreshGeneratedEntries();
	RefreshLunarAccessibility();

	if (PreviousItemId != ActiveItemId && bBroadcastChange)
	{
		const FLunarListViewItemData ItemData = Items[ItemIndex];
		NotifyLunarAccessibleValueChanged(NativeGetLunarAccessibleValueText());
		BP_OnLunarActiveItemChanged(PreviousItemId, ActiveItemId, ItemIndex, ItemData);
		OnActiveItemChanged.Broadcast(this, PreviousItemId, ActiveItemId);
	}
	if (bBroadcastChange && PreviousSelectedItemIds != SelectedItemIds)
	{
		BroadcastSelectedItemsChanged(PreviousSelectedItemIds);
	}
}

void ULunarListView::ClearActiveItemInternal(const bool bBroadcastChange)
{
	const FName PreviousItemId = ActiveItemId;
	const TArray<FName> PreviousSelectedItemIds = SelectedItemIds;
	ActiveItemId = NAME_None;
	PendingRowGenerationItemId = NAME_None;
	if (SelectionMode == ELunarListViewSelectionMode::Single)
	{
		SelectedItemIds.Reset();
		SelectionAnchorItemId = NAME_None;
	}
	SynchronizeSlateSelectionAndActiveItem();
	RefreshGeneratedEntries();
	RefreshLunarAccessibility();

	if (!PreviousItemId.IsNone() && bBroadcastChange)
	{
		NotifyLunarAccessibleValueChanged(NativeGetLunarAccessibleValueText());
		BP_OnLunarActiveItemChanged(
			PreviousItemId,
			NAME_None,
			INDEX_NONE,
			FLunarListViewItemData());
		OnActiveItemChanged.Broadcast(this, PreviousItemId, NAME_None);
	}
	if (bBroadcastChange && PreviousSelectedItemIds != SelectedItemIds)
	{
		BroadcastSelectedItemsChanged(PreviousSelectedItemIds);
	}
}

void ULunarListView::SynchronizeSlateItems()
{
	if (ListViewWidget.IsValid())
	{
		ListViewWidget->CancelScrollIntoView();
		ListViewWidget->ClearSelection();
	}

	ItemObjects.Reset();
	SlateItems.Reset();
	ItemIndexByObject.Reset();
	ItemObjects.Reserve(Items.Num());
	SlateItems.Reserve(Items.Num());
	for (int32 ItemIndex = 0; ItemIndex < Items.Num(); ++ItemIndex)
	{
		ULunarListViewItemObject* ItemObject = NewObject<ULunarListViewItemObject>(this);
		ItemObjects.Add(ItemObject);
		SlateItems.Add(ItemObject);
		ItemIndexByObject.Add(TObjectKey<UObject>(ItemObject), ItemIndex);
	}

	if (ListViewWidget.IsValid())
	{
		ListViewWidget->RequestListRefresh();
	}
}

void ULunarListView::SynchronizeSlateSelectionAndActiveItem()
{
	if (!ListViewWidget.IsValid())
	{
		return;
	}

	ListViewWidget->CancelScrollIntoView();
	ListViewWidget->ClearSelection();
	for (const FName SelectedItemId : SelectedItemIds)
	{
		const int32* SelectedIndex = ItemIndexById.Find(SelectedItemId);
		if (SelectedIndex && ItemObjects.IsValidIndex(*SelectedIndex))
		{
			if (UObject* SelectedObject = ItemObjects[*SelectedIndex].Get())
			{
				ListViewWidget->SetItemSelection(SelectedObject, true, ESelectInfo::Direct);
			}
		}
	}
	ListViewWidget->Invalidate(EInvalidateWidgetReason::Paint);

	const int32 ItemIndex = GetActiveItemIndex();
	UObject* ActiveItemObject = ItemObjects.IsValidIndex(ItemIndex)
		? ItemObjects[ItemIndex].Get()
		: nullptr;
	if (!ActiveItemObject || !SlateItems.Contains(ActiveItemObject))
	{
		PendingRowGenerationItemId = NAME_None;
		return;
	}

	const TSharedPtr<ITableRow> ExistingRow = ListViewWidget->WidgetFromItem(ActiveItemObject);
	PendingRowGenerationItemId = ExistingRow.IsValid()
		? NAME_None
		: ActiveItemId;
	ListViewWidget->RequestScrollIntoView(ActiveItemObject, GetOwningSlateUserIndex());
	if (ExistingRow.IsValid())
	{
		HandleEntryInitialized(ActiveItemObject, ExistingRow.ToSharedRef());
	}
}
void ULunarListView::ApplyListViewPresentation()
{
	if (ListViewWidget.IsValid())
	{
		ListViewWidget->RequestListRefresh();
		ListViewWidget->Invalidate(EInvalidateWidgetReason::Layout | EInvalidateWidgetReason::Paint);
	}
}

void ULunarListView::ReportConfigurationErrors(
	const TMap<FString, FString>& CurrentErrors)
{
	const FGameplayTag ConsoleCategory = FGameplayTag::RequestGameplayTag(TEXT("Lunar.Console"), false);
	for (const TPair<FString, FString>& Pair : CurrentErrors)
	{
		if (ReportedConfigurationErrors.Contains(Pair.Key))
		{
			continue;
		}

		ReportedConfigurationErrors.Add(Pair.Key);
		ULunarConsoleSubsystem::AddMessage(
			ConsoleCategory,
			ELunarConsoleMessageVerbosity::Error,
			Pair.Value);
		UE_LOG(LogLunarListView, Error, TEXT("%s"), *Pair.Value);
	}

	for (auto ErrorIt = ReportedConfigurationErrors.CreateIterator(); ErrorIt; ++ErrorIt)
	{
		if (!CurrentErrors.Contains(*ErrorIt))
		{
			ErrorIt.RemoveCurrent();
		}
	}
}

uint32 ULunarListView::GetOwningSlateUserIndex() const
{
	if (const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (const TSharedPtr<const FSlateUser> SlateUser = LocalPlayer->GetSlateUser())
		{
			return static_cast<uint32>(SlateUser->GetUserIndex());
		}
	}
	return 0;
}

UObject* ULunarListView::FindGeneratedItemAtScreenPosition(
	const FVector2D& ScreenPosition) const
{
	if (!ListViewWidget.IsValid())
	{
		return nullptr;
	}

	for (UObject* Item : SlateItems)
	{
		if (const TSharedPtr<ITableRow> Row = ListViewWidget->WidgetFromItem(Item))
		{
			const TSharedRef<SWidget> RowWidget = Row->AsWidget();
			if (RowWidget->GetVisibility().IsVisible()
				&& RowWidget->GetCachedGeometry().IsUnderLocation(ScreenPosition))
			{
				return Item;
			}
		}
	}
	return nullptr;
}

bool ULunarListView::IsPointerItemEligible(const UObject* Item) const
{
	return IsItemEligible(FindLogicalIndexForItem(Item));
}

void ULunarListView::ResetPointerItemPress()
{
	PointerPressedItem = nullptr;
	PointerItemPressScreenPosition = FVector2D::ZeroVector;
	PressedItemIndex = INDEX_NONE;
	bPointerPressedOverEligibleItem = false;
	bTrackingPointerItemPress = false;
	bTouchItemTapEligible = false;
	bPointerItemPressIsTouch = false;
	bPointerRangeSelectionModifier = false;
	bPointerAdditiveSelectionModifier = false;
	bActivatePointerItemOnRelease = false;
	RefreshGeneratedEntries();
}

TSharedRef<ITableRow> ULunarListView::HandleGenerateRow(
	UObject* Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	const int32 ItemIndex = FindLogicalIndexForItem(Item);
	TSharedRef<SWidget> RowContent =
		SNew(SBorder)
		.BorderImage(FStyleDefaults::GetNoBrush())
		.Padding(FallbackEntryPadding)
		[
			SNew(STextBlock)
			.Text(ResolveItemDisplayText(Item))
			.TextStyle(&FallbackTextStyle)
		];

	ULunarListViewEntry* GeneratedEntry = nullptr;
	UClass* ResolvedEntryClass = EntryWidgetClass.Get();
	const bool bCanInstantiateEntry = ResolvedEntryClass
		&& !ResolvedEntryClass->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists);
	if (bCanInstantiateEntry && Items.IsValidIndex(ItemIndex))
	{
		GeneratedEntry = CreateWidget<ULunarListViewEntry>(this, ResolvedEntryClass);
		if (IsValid(GeneratedEntry))
		{
			GeneratedEntry->InitializeFromListView(
				this,
				ItemIndex,
				Items[ItemIndex],
				ResolveItemVisualState(ItemIndex),
				ItemIndex == GetActiveItemIndex(),
			IsItemSelectedByIndex(ItemIndex));
			GeneratedEntry->SetVisibility(ESlateVisibility::HitTestInvisible);
			GeneratedEntries.Add(GeneratedEntry);
			RowContent = GeneratedEntry->TakeWidget();
			GeneratedEntry->ApplyDataFromListView(ItemIndex, Items[ItemIndex]);
			GeneratedEntry->ApplyVisualStateFromListView(
				ResolveItemVisualState(ItemIndex),
				ItemIndex == GetActiveItemIndex(),
			IsItemSelectedByIndex(ItemIndex));
		}
	}

	TSharedRef<LunarListView_Private::SLunarListViewPassthroughRow> Row =
		SNew(LunarListView_Private::SLunarListViewPassthroughRow, OwnerTable)
		.Style(&RowPresentationStyle)
		[
			RowContent
		];
	Row->SetAccessibleBehavior(EAccessibleBehavior::NotAccessible);
	if (GeneratedEntry)
	{
		GeneratedEntryByRow.Add(&Row.Get(), GeneratedEntry);
	}
	return Row;
}

void ULunarListView::HandleEntryInitialized(
	UObject* Item,
	const TSharedRef<ITableRow>& Row)
{
	const int32 ItemIndex = FindLogicalIndexForItem(Item);
	const TSharedRef<SWidget> RowWidget = Row->AsWidget();
	RowWidget->SetAccessibleBehavior(EAccessibleBehavior::NotAccessible);
	RowWidget->SetEnabled(true);

	if (const TWeakObjectPtr<ULunarListViewEntry>* EntryPointer =
		GeneratedEntryByRow.Find(&Row.Get()))
	{
		if (ULunarListViewEntry* Entry = EntryPointer->Get();
			IsValid(Entry) && Items.IsValidIndex(ItemIndex))
		{
			Entry->ApplyDataFromListView(ItemIndex, Items[ItemIndex]);
			Entry->ApplyVisualStateFromListView(
				ResolveItemVisualState(ItemIndex),
				ItemIndex == GetActiveItemIndex(),
			IsItemSelectedByIndex(ItemIndex));
		}
	}

}

void ULunarListView::HandleRowReleased(const TSharedRef<ITableRow>& Row)
{
	if (const TWeakObjectPtr<ULunarListViewEntry>* EntryPointer =
		GeneratedEntryByRow.Find(&Row.Get()))
	{
		if (ULunarListViewEntry* Entry = EntryPointer->Get())
		{
			GeneratedEntries.RemoveSingleSwap(Entry);
		}
		GeneratedEntryByRow.Remove(&Row.Get());
	}
}

void ULunarListView::HandleItemScrolledIntoView(
	UObject* Item,
	const TSharedPtr<ITableRow>& Row)
{
	const int32 ItemIndex = FindLogicalIndexForItem(Item);
	const bool bIsActiveItem = CachedItemIds.IsValidIndex(ItemIndex)
		&& CachedItemIds[ItemIndex] == ActiveItemId;
	const bool bNeedsFinalReveal = Row.IsValid()
		&& bIsActiveItem
		&& PendingRowGenerationItemId == ActiveItemId;

	if (Row.IsValid())
	{
		HandleEntryInitialized(Item, Row.ToSharedRef());
	}
	if (bIsActiveItem)
	{
		PendingRowGenerationItemId = NAME_None;
		if (bNeedsFinalReveal && ListViewWidget.IsValid())
		{
			// The first request may only generate a virtualized edge row. Re-request
			// after that row has valid geometry so horizontal wrap reveals it fully.
			ListViewWidget->RequestScrollIntoView(Item, GetOwningSlateUserIndex());
		}
	}
}

#undef LOCTEXT_NAMESPACE