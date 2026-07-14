// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarListView.cpp
 * @brief Implements virtualized item ownership, input, restoration, and presentation for ULunarListView.
 * @ingroup LunarNavigationControls
 */

#include "UI/Navigation/Controls/LunarListView.h"

#include "Engine/LocalPlayer.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "Subsystems/Console/LunarConsoleSubsystem.h"
#include "UI/Navigation/Controls/LunarListViewItem.h"
#include "UI/Navigation/Styles/LunarStyleResolver.h"
#include "UI/Navigation/Types/LunarGameplayTags.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

#define LOCTEXT_NAMESPACE "LunarListView"

/** Private log channel for actionable ListView configuration failures. */
DEFINE_LOG_CATEGORY_STATIC(LogLunarListView, Log, All);

/** Private helpers for translating ListView action tags. */
namespace LunarListView_Private
{
	/** Maps a canonical navigation action tag to its direction. @param ActionTag Canonical action tag. @param OutDirection Resolved direction. @return True when the tag is directional. */
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

	FLunarPromptActionRequest AcceptPrompt;
	AcceptPrompt.ActionTag = LunarGameplayTags::UI_Action_Accept.GetTag();
	PromptActions.Add(AcceptPrompt);
}

void ULunarListView::SetItems(const TArray<UObject*>& NewItems)
{
	Items.Reset(NewItems.Num());
	for (UObject* Item : NewItems)
	{
		Items.Add(Item);
	}
	RefreshNavigationItems();
}

bool ULunarListView::SetActiveItemById(const FName ItemId)
{
	const int32* ItemIndex = ItemIndexById.Find(ItemId);
	if (!ItemIndex || !IsItemEligible(*ItemIndex))
	{
		return false;
	}

	SetActiveItemInternal(*ItemIndex, true);
	return true;
}

UObject* ULunarListView::GetActiveItem() const
{
	const int32* ItemIndex = ItemIndexById.Find(ActiveItemId);
	return ItemIndex && Items.IsValidIndex(*ItemIndex) ? Items[*ItemIndex].Get() : nullptr;
}

FName ULunarListView::GetActiveItemId() const
{
	return ActiveItemId;
}

void ULunarListView::ClearActiveItem()
{
	ClearActiveItemInternal(true);
}

void ULunarListView::RefreshNavigationItems()
{
	const FName PreviousItemId = ActiveItemId;
	const int32 PreviousLogicalIndex = LastActiveItemIndex;
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
		UObject* Item = Items[ItemIndex].Get();
		if (!IsValid(Item))
		{
			const FString ErrorKey = FString::Printf(TEXT("InvalidObject:%d"), ItemIndex);
			CurrentErrors.Add(
				ErrorKey,
				FString::Printf(
					TEXT("%s has a null or invalid ListView item at logical index %d."),
					*GetPathName(),
					ItemIndex));
			continue;
		}

		if (!Item->GetClass()->ImplementsInterface(ULunarListViewItem::StaticClass()))
		{
			const FString ErrorKey = FString::Printf(TEXT("MissingInterface:%s:%d"), *Item->GetPathName(), ItemIndex);
			CurrentErrors.Add(
				ErrorKey,
				FString::Printf(
					TEXT("%s item '%s' at logical index %d does not implement ILunarListViewItem."),
					*GetPathName(),
					*Item->GetPathName(),
					ItemIndex));
			continue;
		}

		const FName ItemId = ILunarListViewItem::Execute_GetItemNavigationId(Item);
		CachedItemIds[ItemIndex] = ItemId;
		CachedItemEnabled[ItemIndex] = ILunarListViewItem::Execute_IsItemNavigationEnabled(Item);
		CachedItemSelectableWhenDisabled[ItemIndex] =
			ILunarListViewItem::Execute_CanSelectItemWhenDisabled(Item);
		CachedItemDisabledReasons[ItemIndex] = ILunarListViewItem::Execute_GetItemDisabledReason(Item);

		if (ItemId.IsNone())
		{
			const FString ErrorKey = FString::Printf(TEXT("MissingId:%s:%d"), *Item->GetPathName(), ItemIndex);
			CurrentErrors.Add(
				ErrorKey,
				FString::Printf(
					TEXT("%s item '%s' at logical index %d returned an empty ItemNavigationId."),
					*GetPathName(),
					*Item->GetPathName(),
					ItemIndex));
			continue;
		}

		ItemIdCounts.FindOrAdd(ItemId)++;
	}

	for (const TPair<FName, int32>& Pair : ItemIdCounts)
	{
		if (Pair.Value > 1)
		{
			const FString ErrorKey = FString::Printf(TEXT("DuplicateId:%s"), *Pair.Key.ToString());
			CurrentErrors.Add(
				ErrorKey,
				FString::Printf(
					TEXT("%s contains %d ListView items with duplicate ItemNavigationId '%s'. All duplicate entries are excluded from logical navigation."),
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

	const int32 RestoredItemIndex = FindRestoredItemIndex(PreviousItemId, PreviousLogicalIndex);
	if (RestoredItemIndex != INDEX_NONE)
	{
		SetActiveItemInternal(RestoredItemIndex, true);
	}
	else
	{
		ClearActiveItemInternal(true);
	}
}

void ULunarListView::SetExternalPresentationStyle(
	const FTableRowStyle* RowStyle,
	const FScrollBarStyle* ScrollBarStyle)
{
	bHasExternalRowStyle = RowStyle != nullptr;
	bHasExternalScrollBarStyle = ScrollBarStyle != nullptr;
	if (RowStyle)
	{
		ExternalRowStyle = *RowStyle;
	}
	if (ScrollBarStyle)
	{
		ExternalScrollBarStyle = *ScrollBarStyle;
	}
	bListViewStylePending = true;
	ApplyListViewPresentationStyle();
}

void ULunarListView::SetExternalItemTextResolver(TFunction<FText(const UObject*)> ItemTextResolver)
{
	ExternalItemTextResolver = MoveTemp(ItemTextResolver);
	if (ListViewWidget.IsValid())
	{
		ListViewWidget->RequestListRefresh();
	}
	RefreshLunarAccessibility();
}

TSharedPtr<SWidget> ULunarListView::RebuildLunarSpecializedPresentation()
{
	ApplyListViewPresentationStyle();
	SAssignNew(ListViewWidget, SListView<UObject*>)
		.ListItemsSource(&SlateItems)
		.OnGenerateRow(SListView<UObject*>::FOnGenerateRow::CreateUObject(
			this,
			&ULunarListView::HandleGenerateRow))
		.OnEntryInitialized(SListView<UObject*>::FOnEntryInitialized::CreateUObject(
			this,
			&ULunarListView::HandleEntryInitialized))
		.OnItemScrolledIntoView(SListView<UObject*>::FOnItemScrolledIntoView::CreateUObject(
			this,
			&ULunarListView::HandleItemScrolledIntoView))
		.SelectionMode(ESelectionMode::Single)
		.ClearSelectionOnClick(false)
		.Orientation(Orientation)
		.ScrollIntoViewAlignment(EScrollIntoViewAlignment::IntoView)
		.ScrollBarStyle(&DisplayedListViewStyle.ScrollBarStyle)
		.AllowOverscroll(EAllowOverscroll::No)
		.ConsumeMouseWheel(EConsumeMouseWheel::Never)
		.HandleGamepadEvents(false)
		.HandleDirectionalNavigation(false)
		.IsFocusable(false)
		.Visibility(EVisibility::HitTestInvisible);

	ListViewWidget->SetAccessibleBehavior(EAccessibleBehavior::NotAccessible);
	SynchronizeSlateActiveItem();
	return ListViewWidget;
}

void ULunarListView::ReleaseSlateResources(const bool bReleaseChildren)
{
	ListViewWidget.Reset();
	Super::ReleaseSlateResources(bReleaseChildren);
}

void ULunarListView::SynchronizeProperties()
{
	RefreshNavigationItems();
	Super::SynchronizeProperties();
}

void ULunarListView::NativeDestruct()
{
	ResetPointerItemPress();
	if (ListViewWidget.IsValid())
	{
		ListViewWidget->CancelScrollIntoView();
	}
	Super::NativeDestruct();
}

FReply ULunarListView::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bCanInteractWithPointer && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bTrackingPointerItemPress = true;
		PointerPressedItem = FindGeneratedItemAtScreenPosition(InMouseEvent.GetScreenSpacePosition());
		bPointerPressedOverEligibleItem = IsPointerItemEligible(PointerPressedItem.Get());
		if (bPointerPressedOverEligibleItem)
		{
			const int32 ItemIndex = FindLogicalIndexForItem(PointerPressedItem.Get());
			SetActiveItemInternal(ItemIndex, true);
		}
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply ULunarListView::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
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
	}

	ResetPointerItemPress();
	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void ULunarListView::NativeOnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent)
{
	ResetPointerItemPress();
	Super::NativeOnMouseCaptureLost(CaptureLostEvent);
}

FReply ULunarListView::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bCanInteractWithPointer && ListViewWidget.IsValid() && InMouseEvent.GetWheelDelta() != 0.0f)
	{
		const float PreviousOffset = ListViewWidget->GetScrollOffset();
		ListViewWidget->AddScrollOffset(-InMouseEvent.GetWheelDelta() * 3.0f, true);
		if (ListViewWidget->GetScrollOffset() != PreviousOffset)
		{
			return FReply::Handled();
		}
	}
	return Super::NativeOnMouseWheel(InGeometry, InMouseEvent);
}

FReply ULunarListView::NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	if (bCanInteractWithPointer && InGestureEvent.IsTouchEvent() && InGestureEvent.GetPointerIndex() == 0)
	{
		bTrackingPointerItemPress = true;
		bTouchItemTapEligible = true;
		PointerItemPressScreenPosition = InGestureEvent.GetScreenSpacePosition();
		PointerPressedItem = FindGeneratedItemAtScreenPosition(InGestureEvent.GetScreenSpacePosition());
		bPointerPressedOverEligibleItem = IsPointerItemEligible(PointerPressedItem.Get());
	}
	return Super::NativeOnTouchStarted(InGeometry, InGestureEvent);
}

FReply ULunarListView::NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	if (bTrackingPointerItemPress && InGestureEvent.GetPointerIndex() == 0
		&& FVector2D::Distance(PointerItemPressScreenPosition, InGestureEvent.GetScreenSpacePosition())
			> FSlateApplication::Get().GetDragTriggerDistance())
	{
		bTouchItemTapEligible = false;
	}
	return Super::NativeOnTouchMoved(InGeometry, InGestureEvent);
}

FReply ULunarListView::NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
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

		SetActiveItemInternal(FindLogicalIndexForItem(ReleasedItem), true);
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

	ELunarNavigationDirection Direction = ELunarNavigationDirection::Up;
	if (LunarListView_Private::ActionToDirection(ActionContext.ActionTag, Direction)
		&& IsDirectionAlongOrientation(Direction))
	{
		return true;
	}

	return Super::NativeCanHandleLunarAction(ActionContext);
}

ELunarUIActionResult ULunarListView::NativeHandleLunarAction(const FLunarUIActionContext& ActionContext)
{
	if (ActionContext.ActionTag == LunarGameplayTags::UI_Action_Accept.GetTag())
	{
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
		return MoveActiveItem(Direction)
			? ELunarUIActionResult::Handled
			: ELunarUIActionResult::Rejected;
	}

	return Super::NativeHandleLunarAction(ActionContext);
}

bool ULunarListView::NativeCanActivateLunarWidget() const
{
	const int32* ItemIndex = ItemIndexById.Find(ActiveItemId);
	return Super::NativeCanActivateLunarWidget()
		&& ItemIndex
		&& IsItemEnabled(*ItemIndex);
}

void ULunarListView::NativeOnLunarRejected()
{
	Super::NativeOnLunarRejected();
	const int32* ItemIndex = ItemIndexById.Find(ActiveItemId);
	if (ItemIndex && !IsItemEnabled(*ItemIndex)
		&& CachedItemDisabledReasons.IsValidIndex(*ItemIndex)
		&& !CachedItemDisabledReasons[*ItemIndex].IsEmpty())
	{
		NotifyLunarAccessibleValueChanged(CachedItemDisabledReasons[*ItemIndex]);
	}
}

FText ULunarListView::NativeGetLunarAccessibleValueText() const
{
	const int32* ItemIndex = ItemIndexById.Find(ActiveItemId);
	if (!ItemIndex || !Items.IsValidIndex(*ItemIndex))
	{
		return LOCTEXT("NoActiveItem", "No active item");
	}

	const FText ItemDisplayText = ResolveItemDisplayText(Items[*ItemIndex].Get());
	const FText ItemText = ItemDisplayText.IsEmpty()
		? LOCTEXT("ActiveItemWithoutLabel", "Active item")
		: FText::Format(
			LOCTEXT("ActiveItem", "Active item {0}"),
			ItemDisplayText);
	if (!IsItemEnabled(*ItemIndex) && CachedItemDisabledReasons.IsValidIndex(*ItemIndex)
		&& !CachedItemDisabledReasons[*ItemIndex].IsEmpty())
	{
		return FText::Format(
			LOCTEXT("DisabledActiveItem", "{0}. Unavailable: {1}"),
			ItemText,
			CachedItemDisabledReasons[*ItemIndex]);
	}
	return ItemText;
}

bool ULunarListView::ResolveCommonStylePatch(FLunarCommonStylePatch& OutStyle, FString& OutError) const
{
	FLunarListViewStylePatch ResolvedStyle;
	const bool bResolved = LunarStyleResolver::ResolveListViewStyle(
		StyleAsset,
		GetLunarVisualState(),
		StyleOverrides,
		ResolvedStyle,
		&OutError);
	if (bResolved)
	{
		ULunarListView* MutableThis = const_cast<ULunarListView*>(this);
		MutableThis->ResolvedListViewStyle = ResolvedStyle;
		MutableThis->bListViewStylePending = true;
		OutStyle = ResolvedStyle.Common;
	}
	return bResolved;
}

void ULunarListView::ApplyResolvedCommonStyle(const FLunarCommonStylePatch& ResolvedStyle)
{
	Super::ApplyResolvedCommonStyle(ResolvedStyle);
	if (bListViewStylePending)
	{
		ApplyListViewPresentationStyle();
	}
}

bool ULunarListView::IsDirectionAlongOrientation(const ELunarNavigationDirection Direction) const
{
	return Orientation == Orient_Vertical
		? Direction == ELunarNavigationDirection::Up || Direction == ELunarNavigationDirection::Down
		: Direction == ELunarNavigationDirection::Left || Direction == ELunarNavigationDirection::Right;
}

bool ULunarListView::MoveActiveItem(const ELunarNavigationDirection Direction)
{
	const bool bForward = Direction == ELunarNavigationDirection::Down
		|| Direction == ELunarNavigationDirection::Right;
	int32 StartingIndex = INDEX_NONE;
	if (const int32* ActiveIndex = ItemIndexById.Find(ActiveItemId))
	{
		StartingIndex = *ActiveIndex;
	}
	else
	{
		StartingIndex = bForward ? -1 : Items.Num();
	}

	const int32 Step = bForward ? 1 : -1;
	for (int32 ItemIndex = StartingIndex + Step; Items.IsValidIndex(ItemIndex); ItemIndex += Step)
	{
		if (IsItemEligible(ItemIndex))
		{
			SetActiveItemInternal(ItemIndex, true);
			return true;
		}
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

int32 ULunarListView::FindLogicalIndexForItem(const UObject* Item) const
{
	for (int32 ItemIndex = 0; ItemIndex < Items.Num(); ++ItemIndex)
	{
		if (Items[ItemIndex].Get() == Item)
		{
			return ItemIndex;
		}
	}
	return INDEX_NONE;
}

int32 ULunarListView::FindRestoredItemIndex(const FName PreviousItemId, const int32 PreviousLogicalIndex) const
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
	if (ExternalItemTextResolver)
	{
		return ExternalItemTextResolver(Item);
	}

	const int32 ItemIndex = FindLogicalIndexForItem(Item);
	const FName ItemId = CachedItemIds.IsValidIndex(ItemIndex)
		? CachedItemIds[ItemIndex]
		: NAME_None;
	return ItemId.IsNone()
		? FText::FromString(GetNameSafe(Item))
		: FText::FromName(ItemId);
}

void ULunarListView::SetActiveItemInternal(const int32 ItemIndex, const bool bBroadcastChange)
{
	if (!IsItemEligible(ItemIndex))
	{
		return;
	}

	const FName PreviousItemId = ActiveItemId;
	ActiveItemId = CachedItemIds[ItemIndex];
	LastActiveItemIndex = ItemIndex;
	PendingRowGenerationItemId = ActiveItemId;
	SynchronizeSlateActiveItem();
	RefreshLunarAccessibility();

	if (PreviousItemId != ActiveItemId)
	{
		NotifyLunarAccessibleValueChanged(NativeGetLunarAccessibleValueText());
		if (bBroadcastChange)
		{
			OnActiveItemChanged.Broadcast(PreviousItemId, ActiveItemId);
		}
	}
}

void ULunarListView::ClearActiveItemInternal(const bool bBroadcastChange)
{
	const FName PreviousItemId = ActiveItemId;
	ActiveItemId = NAME_None;
	PendingRowGenerationItemId = NAME_None;
	if (ListViewWidget.IsValid())
	{
		ListViewWidget->CancelScrollIntoView();
		ListViewWidget->ClearSelection();
	}
	RefreshLunarAccessibility();

	if (!PreviousItemId.IsNone())
	{
		NotifyLunarAccessibleValueChanged(NativeGetLunarAccessibleValueText());
		if (bBroadcastChange)
		{
			OnActiveItemChanged.Broadcast(PreviousItemId, NAME_None);
		}
	}
}

void ULunarListView::SynchronizeSlateItems()
{
	SlateItems.Reset(Items.Num());
	TSet<TObjectKey<UObject>> AddedItems;
	for (const TObjectPtr<UObject>& ItemPointer : Items)
	{
		UObject* Item = ItemPointer.Get();
		if (IsValid(Item) && !AddedItems.Contains(TObjectKey<UObject>(Item)))
		{
			AddedItems.Add(TObjectKey<UObject>(Item));
			SlateItems.Add(Item);
		}
	}

	if (ListViewWidget.IsValid())
	{
		ListViewWidget->CancelScrollIntoView();
		ListViewWidget->RequestListRefresh();
		for (UObject* Item : SlateItems)
		{
			if (const TSharedPtr<ITableRow> Row = ListViewWidget->WidgetFromItem(Item))
			{
				Row->AsWidget()->SetEnabled(IsItemEnabled(FindLogicalIndexForItem(Item)));
			}
		}
	}
}

void ULunarListView::SynchronizeSlateActiveItem()
{
	if (!ListViewWidget.IsValid())
	{
		return;
	}

	ListViewWidget->CancelScrollIntoView();
	UObject* ActiveItem = GetActiveItem();
	if (!ActiveItem || !SlateItems.Contains(ActiveItem))
	{
		ListViewWidget->ClearSelection();
		PendingRowGenerationItemId = NAME_None;
		return;
	}

	ListViewWidget->SetSelection(ActiveItem, ESelectInfo::Direct);
	if (const TSharedPtr<ITableRow> ExistingRow = ListViewWidget->WidgetFromItem(ActiveItem))
	{
		HandleEntryInitialized(ActiveItem, ExistingRow.ToSharedRef());
		PendingRowGenerationItemId = NAME_None;
	}
	else
	{
		PendingRowGenerationItemId = ActiveItemId;
		ListViewWidget->RequestScrollIntoView(ActiveItem, GetOwningSlateUserIndex());
	}
}

void ULunarListView::ApplyListViewPresentationStyle()
{
	DisplayedListViewStyle = ResolvedListViewStyle;
	if (bHasExternalRowStyle)
	{
		DisplayedListViewStyle.bOverrideRowStyle = true;
		DisplayedListViewStyle.RowStyle = ExternalRowStyle;
	}
	else if (!DisplayedListViewStyle.bOverrideRowStyle)
	{
		DisplayedListViewStyle.bOverrideRowStyle = true;
		DisplayedListViewStyle.RowStyle =
			FCoreStyle::Get().GetWidgetStyle<FTableRowStyle>(TEXT("TableView.Row"));
	}
	if (bHasExternalScrollBarStyle)
	{
		DisplayedListViewStyle.bOverrideScrollBarStyle = true;
		DisplayedListViewStyle.ScrollBarStyle = ExternalScrollBarStyle;
	}
	else if (!DisplayedListViewStyle.bOverrideScrollBarStyle)
	{
		DisplayedListViewStyle.bOverrideScrollBarStyle = true;
		DisplayedListViewStyle.ScrollBarStyle =
			FAppStyle::Get().GetWidgetStyle<FScrollBarStyle>(TEXT("ScrollBar"));
	}
	bListViewStylePending = false;

	if (ListViewWidget.IsValid())
	{
		ListViewWidget->RequestListRefresh();
		ListViewWidget->Invalidate(EInvalidateWidgetReason::Layout | EInvalidateWidgetReason::Paint);
	}
}

void ULunarListView::ReportConfigurationErrors(const TMap<FString, FString>& CurrentErrors)
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

UObject* ULunarListView::FindGeneratedItemAtScreenPosition(const FVector2D& ScreenPosition) const
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
	const int32 ItemIndex = FindLogicalIndexForItem(Item);
	return IsItemEligible(ItemIndex);
}

void ULunarListView::ResetPointerItemPress()
{
	PointerPressedItem = nullptr;
	PointerItemPressScreenPosition = FVector2D::ZeroVector;
	bPointerPressedOverEligibleItem = false;
	bTrackingPointerItemPress = false;
	bTouchItemTapEligible = false;
}

TSharedRef<ITableRow> ULunarListView::HandleGenerateRow(
	UObject* Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	const int32 ItemIndex = FindLogicalIndexForItem(Item);
	const FText RowText = ResolveItemDisplayText(Item);

	TSharedRef<STextBlock> TextWidget = SNew(STextBlock).Text(RowText);
	TextWidget->SetAccessibleBehavior(EAccessibleBehavior::NotAccessible);

	TSharedRef<STableRow<UObject*>> Row =
		SNew(STableRow<UObject*>, OwnerTable)
		.Style(&DisplayedListViewStyle.RowStyle)
		.IsEnabled(IsItemEnabled(ItemIndex))
		[
			TextWidget
		];
	Row->SetAccessibleBehavior(EAccessibleBehavior::NotAccessible);
	return Row;
}

void ULunarListView::HandleEntryInitialized(UObject* Item, const TSharedRef<ITableRow>& Row)
{
	const int32 ItemIndex = FindLogicalIndexForItem(Item);
	const TSharedRef<SWidget> RowWidget = Row->AsWidget();
	RowWidget->SetAccessibleBehavior(EAccessibleBehavior::NotAccessible);
	RowWidget->SetEnabled(IsItemEnabled(ItemIndex));

	if (CachedItemIds.IsValidIndex(ItemIndex) && CachedItemIds[ItemIndex] == ActiveItemId)
	{
		PendingRowGenerationItemId = NAME_None;
	}
}

void ULunarListView::HandleItemScrolledIntoView(UObject* Item, const TSharedPtr<ITableRow>& Row)
{
	if (Row.IsValid())
	{
		HandleEntryInitialized(Item, Row.ToSharedRef());
	}
	if (Item == GetActiveItem())
	{
		PendingRowGenerationItemId = NAME_None;
	}
}

#undef LOCTEXT_NAMESPACE
