// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarComboBox.cpp
 * @brief Implements ComboBox option filtering, popup scope ownership, and outside-pointer dismissal.
 * @ingroup LunarNavigationControls
 */

#include "UI/Navigation/Controls/LunarComboBox.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Application/IInputProcessor.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "Internationalization/Internationalization.h"
#include "Styling/StyleDefaults.h"
#include "Subsystems/Console/LunarConsoleSubsystem.h"
#include "UI/Navigation/Controls/LunarListView.h"
#include "UI/Navigation/Controls/LunarListViewItem.h"
#include "UI/Navigation/Controls/LunarComboBoxOptionItem.h"
#include "UI/Navigation/Core/LunarNavigationScope.h"
#include "UI/Navigation/Core/LunarNavigationSubsystem.h"
#include "UI/Navigation/Styles/LunarStyleResolver.h"
#include "UI/Navigation/Types/LunarGameplayTags.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SOverlay.h"

/** Private log channel for actionable ComboBox configuration failures. */
DEFINE_LOG_CATEGORY_STATIC(LogLunarComboBox, Log, All);

/**
 * Observes pointer-down before normal routing for outside dismissal and trigger-toggle suppression.
 * @ingroup LunarNavigationControls
 */
class FLunarComboBoxOutsideClickInputProcessor final : public IInputProcessor
{
public:
	/** Creates a weakly owned preprocessor. @param InOwner ComboBox receiving global pointer events. */
	explicit FLunarComboBoxOutsideClickInputProcessor(ULunarComboBox* InOwner)
		: Owner(InOwner)
	{
	}

	/** Performs no per-frame work. @param DeltaTime Elapsed seconds. @param SlateApp Slate application. @param Cursor Active cursor. */
	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override
	{
	}

	/** Routes eligible pointer-down events to the owner. @param SlateApp Slate application. @param MouseEvent Pointer event. @return True when the owner consumed the event. */
	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override
	{
		if ((MouseEvent.IsTouchEvent() || MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
			&& Owner.IsValid())
		{
			return Owner->HandleGlobalPointerDown(
				MouseEvent.GetScreenSpacePosition(),
				MouseEvent.GetUserIndex());
		}
		return false;
	}

	/** Treats double-click as a pointer-down for dismissal. @param SlateApp Slate application. @param MouseEvent Pointer event. @return True when consumed. */
	virtual bool HandleMouseButtonDoubleClickEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override
	{
		return HandleMouseButtonDownEvent(SlateApp, MouseEvent);
	}

	/** Returns the Slate diagnostics label. @return Static processor name. */
	virtual const TCHAR* GetDebugName() const override
	{
		return TEXT("LunarComboBoxOutsideClick");
	}

private:
	/** Weak owner that prevents the Slate preprocessor from extending widget lifetime. */
	TWeakObjectPtr<ULunarComboBox> Owner;
};

ULunarComboBox::ULunarComboBox(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCanReceiveLunarSelection = true;
	bCanInteractWithPointer = true;
	bEnableInputPrompt = true;

	FLunarPromptActionRequest AcceptPrompt;
	AcceptPrompt.ActionTag = LunarGameplayTags::UI_Action_Accept.GetTag();
	PromptActions.Add(AcceptPrompt);

	PopupScopeSettings.bRestoreLastSelection = false;
}

void ULunarComboBox::SetOptions(const TArray<FLunarComboBoxOption>& NewOptions)
{
	const FName PreviousTemporaryId = TemporaryOptionId;
	Options = NewOptions;
	RebuildOptionAdapters();

	if (!SelectedOptionId.IsNone() && !FindOptionById(SelectedOptionId))
	{
		ApplyCommittedSelection(NAME_None, true);
	}

	TemporaryOptionId = PreviousTemporaryId;
	RebuildFilteredOptions(true, true);
	UpdateCommittedPresentation();
}

void ULunarComboBox::SetSelectedOptionById(const FName OptionId)
{
	if (!OptionId.IsNone() && !FindOptionById(OptionId))
	{
		const FString Message = FString::Printf(
			TEXT("%s cannot select unknown ComboBox OptionId '%s'."),
			*GetPathName(),
			*OptionId.ToString());
		UE_LOG(LogLunarComboBox, Error, TEXT("%s"), *Message);
		ULunarConsoleSubsystem::AddMessage(FGameplayTag(), ELunarConsoleMessageVerbosity::Error, Message);
		return;
	}

	ApplyCommittedSelection(OptionId, true);
	if (bComboBoxOpen)
	{
		TemporaryOptionId = SelectedOptionId;
		RebuildFilteredOptions(true);
	}
}

bool ULunarComboBox::OpenComboBox()
{
	if (bComboBoxOpen || bOpeningComboBox)
	{
		return true;
	}

	ULunarNavigationSubsystem* NavigationSubsystem = GetNavigationSubsystem();
	if (!PopupRootWidget || !OptionsListView || !NavigationSubsystem)
	{
		const FString Message = FString::Printf(
			TEXT("%s cannot open: PopupRootWidget, OptionsListView, and the owning local-player navigation subsystem are required."),
			*GetPathName());
		UE_LOG(LogLunarComboBox, Error, TEXT("%s"), *Message);
		ULunarConsoleSubsystem::AddMessage(FGameplayTag(), ELunarConsoleMessageVerbosity::Error, Message);
		return false;
	}

	if (!bRestoreSearchQuery)
	{
		SearchQuery = FText::GetEmpty();
	}
	RebuildOptionAdapters();
	SetTemporarySelectionToCommittedOrFirstEligible();

	PopupRootWidget->SetVisibility(ESlateVisibility::Visible);
	if (SearchFieldNavigationWidget)
	{
		SearchFieldNavigationWidget->SetVisibility(
			bEnableSearch ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (!PopupScope)
	{
		PopupScope = NewObject<ULunarNavigationScope>(this);
	}
	FLunarNavigationScopeSettings ScopeSettings = PopupScopeSettings;
	ScopeSettings.bRestoreLastSelection = false;
	ScopeSettings.InitialSelectionWidget = OptionsListView;
	ScopeSettings.InitialSelectionId = OptionsListView->GetNavigationId();
	PopupScope->RootWidget = PopupRootWidget;
	PopupScope->Settings = ScopeSettings;

	bOpeningComboBox = true;
	bCloseRequestedWhileOpening = false;
	const bool bPushed = NavigationSubsystem->PushNavigationScope(PopupScope);
	bOpeningComboBox = false;
	if (!bPushed || !NavigationSubsystem->GetNavigationScopeStack().Contains(PopupScope))
	{
		bCloseRequestedWhileOpening = false;
		PopupRootWidget->SetVisibility(ESlateVisibility::Collapsed);
		return false;
	}

	bComboBoxOpen = true;
	RegisterOutsideClickProcessor();
	OnComboBoxOpened.Broadcast();

	const bool bShouldClose = bCloseRequestedWhileOpening;
	bCloseRequestedWhileOpening = false;
	if (bShouldClose && bComboBoxOpen)
	{
		CloseComboBox();
	}
	return bComboBoxOpen;
}

bool ULunarComboBox::CloseComboBox()
{
	if (bOpeningComboBox)
	{
		bCloseRequestedWhileOpening = true;
		return true;
	}
	if (!bComboBoxOpen)
	{
		return true;
	}
	if (bClosingComboBox)
	{
		bCloseRequestedWhileClosing = true;
		return true;
	}

	bClosingComboBox = true;
	bCloseRequestedWhileClosing = false;
	ULunarNavigationSubsystem* NavigationSubsystem = GetNavigationSubsystem();
	if (NavigationSubsystem)
	{
		if (PopupScope && NavigationSubsystem->GetNavigationScopeStack().Contains(PopupScope)
			&& !NavigationSubsystem->PopNavigationScope(PopupScope))
		{
			bClosingComboBox = false;
			bCloseRequestedWhileClosing = false;
			return false;
		}
	}

	const bool bPopupScopeStillActive = NavigationSubsystem
		&& PopupScope
		&& NavigationSubsystem->GetNavigationScopeStack().Contains(PopupScope);
	if (bComboBoxOpen && !bPopupScopeStillActive)
	{
		FinalizePopupClosed();
	}
	bClosingComboBox = false;
	const bool bShouldCloseAgain = bCloseRequestedWhileClosing;
	bCloseRequestedWhileClosing = false;
	if (bShouldCloseAgain && bComboBoxOpen)
	{
		return CloseComboBox();
	}
	return true;
}

void ULunarComboBox::SetSearchQuery(const FText& NewSearchQuery)
{
	if (SearchQuery.EqualTo(NewSearchQuery))
	{
		return;
	}
	SearchQuery = NewSearchQuery;
	RebuildFilteredOptions(true);
}

bool ULunarComboBox::DoesOptionMatchSearch_Implementation(
	const FLunarComboBoxOption& Option,
	const FText& Query) const
{
	if (Query.IsEmpty())
	{
		return true;
	}
	return Option.DisplayText.ToLower().ToString().Contains(Query.ToLower().ToString());
}

TSharedPtr<SWidget> ULunarComboBox::RebuildLunarSpecializedPresentation()
{
	TSharedRef<SOverlay> Presentation = SNew(SOverlay);
	Presentation->AddSlot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		[
			SAssignNew(ArrowImage, SImage)
			.Visibility(EVisibility::HitTestInvisible)
		];
	ArrowImage->SetAccessibleBehavior(EAccessibleBehavior::NotAccessible);
	ApplySpecializedStyle();
	return Presentation;
}

void ULunarComboBox::ReleaseSlateResources(const bool bReleaseChildren)
{
	ArrowImage.Reset();
	Super::ReleaseSlateResources(bReleaseChildren);
}

void ULunarComboBox::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	RebuildOptionAdapters();
	if (!SelectedOptionId.IsNone() && !FindOptionById(SelectedOptionId))
	{
		SelectedOptionId = NAME_None;
	}
	if (!bComboBoxOpen && PopupRootWidget)
	{
		PopupRootWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (SearchFieldNavigationWidget)
	{
		SearchFieldNavigationWidget->SetVisibility(
			bEnableSearch ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	TemporaryOptionId = SelectedOptionId;
	RebuildFilteredOptions(true, true);
	UpdateCommittedPresentation();
	ApplySpecializedStyle();
}

void ULunarComboBox::NativeConstruct()
{
	Super::NativeConstruct();

	if (OptionsListView)
	{
		OptionsListView->OnActiveItemChanged.RemoveDynamic(this, &ULunarComboBox::HandleListActiveItemChanged);
		OptionsListView->OnActiveItemChanged.AddDynamic(this, &ULunarComboBox::HandleListActiveItemChanged);
		OptionsListView->OnLunarActivated.RemoveDynamic(this, &ULunarComboBox::HandleListActivated);
		OptionsListView->OnLunarActivated.AddDynamic(this, &ULunarComboBox::HandleListActivated);

		const TWeakObjectPtr<ULunarComboBox> WeakThis(this);
		OptionsListView->SetExternalItemTextResolver(
			[WeakThis](const UObject* Item) -> FText
			{
				const ULunarComboBox* ComboBox = WeakThis.Get();
				if (!ComboBox || !Item || !Item->GetClass()->ImplementsInterface(ULunarListViewItem::StaticClass()))
				{
					return FText::GetEmpty();
				}
				const FName ItemId = ILunarListViewItem::Execute_GetItemNavigationId(
					const_cast<UObject*>(Item));
				if (const FLunarComboBoxOption* Option = ComboBox->FindOptionById(ItemId))
				{
					return Option->DisplayText;
				}
				return FText::GetEmpty();
			});
	}

	if (ULunarNavigationSubsystem* NavigationSubsystem = GetNavigationSubsystem())
	{
		NavigationSubsystem->OnActiveScopeChanged.RemoveDynamic(this, &ULunarComboBox::HandleActiveScopeChanged);
		NavigationSubsystem->OnActiveScopeChanged.AddDynamic(this, &ULunarComboBox::HandleActiveScopeChanged);
	}
	if (!CultureChangedHandle.IsValid())
	{
		CultureChangedHandle = FInternationalization::Get().OnCultureChanged().AddUObject(
			this,
			&ULunarComboBox::HandleCultureChanged);
	}

	RebuildOptionAdapters();
	TemporaryOptionId = SelectedOptionId;
	RebuildFilteredOptions(true, true);
	UpdateCommittedPresentation();
}

void ULunarComboBox::NativeDestruct()
{
	if (OptionsListView)
	{
		OptionsListView->OnActiveItemChanged.RemoveDynamic(this, &ULunarComboBox::HandleListActiveItemChanged);
		OptionsListView->OnLunarActivated.RemoveDynamic(this, &ULunarComboBox::HandleListActivated);
		OptionsListView->SetExternalItemTextResolver({});
		OptionsListView->SetExternalPresentationStyle(nullptr, nullptr);
	}
	if (ULunarNavigationSubsystem* NavigationSubsystem = GetNavigationSubsystem())
	{
		NavigationSubsystem->OnActiveScopeChanged.RemoveDynamic(this, &ULunarComboBox::HandleActiveScopeChanged);
	}
	if (CultureChangedHandle.IsValid())
	{
		FInternationalization::Get().OnCultureChanged().Remove(CultureChangedHandle);
		CultureChangedHandle.Reset();
	}

	CloseComboBox();
	UnregisterOutsideClickProcessor();
	Super::NativeDestruct();
}

bool ULunarComboBox::NativeCanHandleLunarAction(const FLunarUIActionContext& ActionContext) const
{
	if (ActionContext.ActionTag == LunarGameplayTags::UI_Action_Accept.GetTag())
	{
		return IsLunarSelected();
	}
	return Super::NativeCanHandleLunarAction(ActionContext);
}

ELunarUIActionResult ULunarComboBox::NativeHandleLunarAction(const FLunarUIActionContext& ActionContext)
{
	if (ActionContext.ActionTag != LunarGameplayTags::UI_Action_Accept.GetTag())
	{
		return Super::NativeHandleLunarAction(ActionContext);
	}
	if (!IsLunarSelected())
	{
		return ELunarUIActionResult::Unhandled;
	}
	if (!NativeCanActivateLunarWidget())
	{
		return ELunarUIActionResult::Rejected;
	}
	if (ActionContext.InputEvent == IE_Released)
	{
		ActivateLunarWidget();
	}
	return ELunarUIActionResult::Handled;
}

bool ULunarComboBox::NativeCanActivateLunarWidget() const
{
	return Super::NativeCanActivateLunarWidget()
		&& PopupRootWidget
		&& OptionsListView
		&& GetNavigationSubsystem();
}

void ULunarComboBox::NativeOnLunarActivated()
{
	Super::NativeOnLunarActivated();
	OpenComboBox();
}

FText ULunarComboBox::NativeGetLunarAccessibleValueText() const
{
	if (const FLunarComboBoxOption* Option = FindOptionById(SelectedOptionId))
	{
		return Option->DisplayText;
	}
	return FText::GetEmpty();
}

bool ULunarComboBox::ResolveCommonStylePatch(FLunarCommonStylePatch& OutStyle, FString& OutError) const
{
	FLunarComboBoxStylePatch ResolvedStyle;
	if (!LunarStyleResolver::ResolveComboBoxStyle(
		StyleAsset,
		GetLunarVisualState(),
		StyleOverrides,
		ResolvedStyle,
		&OutError))
	{
		return false;
	}

	ULunarComboBox* MutableThis = const_cast<ULunarComboBox*>(this);
	MutableThis->ResolvedComboBoxStyle = ResolvedStyle;
	OutStyle = ResolvedStyle.Common;
	return true;
}

void ULunarComboBox::ApplyResolvedCommonStyle(const FLunarCommonStylePatch& ResolvedStyle)
{
	Super::ApplyResolvedCommonStyle(ResolvedStyle);
	ApplySpecializedStyle();
}

const FLunarComboBoxOption* ULunarComboBox::FindOptionById(const FName OptionId) const
{
	if (const int32* OptionIndex = OptionIndexById.Find(OptionId))
	{
		return Options.IsValidIndex(*OptionIndex) ? &Options[*OptionIndex] : nullptr;
	}
	return nullptr;
}

bool ULunarComboBox::IsOptionEligible(const FName OptionId) const
{
	if (const FLunarComboBoxOption* Option = FindOptionById(OptionId))
	{
		return Option->bEnabled || Option->bCanReceiveSelectionWhenDisabled;
	}
	return false;
}

void ULunarComboBox::RebuildOptionAdapters()
{
	OptionItems.Reset();
	OptionIndexById.Reset();
	TSet<FString> CurrentErrors;
	TMap<FName, int32> OptionIdCounts;

	for (int32 OptionIndex = 0; OptionIndex < Options.Num(); ++OptionIndex)
	{
		const FLunarComboBoxOption& Option = Options[OptionIndex];
		if (Option.OptionId.IsNone())
		{
			CurrentErrors.Add(FString::Printf(
				TEXT("%s Options[%d] has an empty OptionId."),
				*GetPathName(),
				OptionIndex));
			continue;
		}
		OptionIdCounts.FindOrAdd(Option.OptionId)++;
	}

	for (const TPair<FName, int32>& Pair : OptionIdCounts)
	{
		if (Pair.Value > 1)
		{
			CurrentErrors.Add(FString::Printf(
				TEXT("%s contains %d options with duplicate OptionId '%s'. All duplicate entries are excluded."),
				*GetPathName(),
				Pair.Value,
				*Pair.Key.ToString()));
		}
	}

	for (int32 OptionIndex = 0; OptionIndex < Options.Num(); ++OptionIndex)
	{
		const FLunarComboBoxOption& Option = Options[OptionIndex];
		if (Option.OptionId.IsNone() || OptionIdCounts.FindRef(Option.OptionId) != 1)
		{
			continue;
		}

		OptionIndexById.Add(Option.OptionId, OptionIndex);
		ULunarComboBoxOptionItem* Item = NewObject<ULunarComboBoxOptionItem>(this);
		Item->Initialize(this, Option.OptionId);
		OptionItems.Add(Item);
	}

	if (!SelectedOptionId.IsNone() && !OptionIndexById.Contains(SelectedOptionId))
	{
		CurrentErrors.Add(FString::Printf(
			TEXT("%s SelectedOptionId '%s' does not identify a valid unique option."),
			*GetPathName(),
			*SelectedOptionId.ToString()));
	}
	ReportConfigurationErrors(CurrentErrors);
}

void ULunarComboBox::RebuildFilteredOptions(
	const bool bPreserveTemporarySelection,
	const bool bUseListViewRestoration)
{
	if (!OptionsListView)
	{
		return;
	}

	const FName PreviousTemporaryId = bPreserveTemporarySelection ? TemporaryOptionId : NAME_None;
	TArray<UObject*> FilteredItems;
	TSet<FName> FilteredIds;
	for (ULunarComboBoxOptionItem* Item : OptionItems)
	{
		if (!Item)
		{
			continue;
		}
		const FName ItemId = Item->GetItemNavigationId_Implementation();
		const FLunarComboBoxOption* Option = FindOptionById(ItemId);
		if (!Option || (bEnableSearch && !DoesOptionMatchSearch(*Option, SearchQuery)))
		{
			continue;
		}
		FilteredItems.Add(Item);
		FilteredIds.Add(ItemId);
	}

	OptionsListView->SetItems(FilteredItems);
	if (!PreviousTemporaryId.IsNone()
		&& FilteredIds.Contains(PreviousTemporaryId)
		&& IsOptionEligible(PreviousTemporaryId)
		&& OptionsListView->SetActiveItemById(PreviousTemporaryId))
	{
		TemporaryOptionId = PreviousTemporaryId;
		return;
	}

	if (bUseListViewRestoration)
	{
		const FName RestoredItemId = OptionsListView->GetActiveItemId();
		if (!RestoredItemId.IsNone()
			&& FilteredIds.Contains(RestoredItemId)
			&& IsOptionEligible(RestoredItemId))
		{
			TemporaryOptionId = RestoredItemId;
			return;
		}
	}

	for (UObject* ItemObject : FilteredItems)
	{
		const FName ItemId = ILunarListViewItem::Execute_GetItemNavigationId(ItemObject);
		if (IsOptionEligible(ItemId) && OptionsListView->SetActiveItemById(ItemId))
		{
			TemporaryOptionId = ItemId;
			return;
		}
	}

	OptionsListView->ClearActiveItem();
	TemporaryOptionId = NAME_None;
}

void ULunarComboBox::SetTemporarySelectionToCommittedOrFirstEligible()
{
	TemporaryOptionId = SelectedOptionId;
	RebuildFilteredOptions(true);
}

void ULunarComboBox::CommitTemporarySelection()
{
	const FLunarComboBoxOption* Option = FindOptionById(TemporaryOptionId);
	if (!Option || !Option->bEnabled)
	{
		return;
	}

	ApplyCommittedSelection(TemporaryOptionId, true);
	CloseComboBox();
}

void ULunarComboBox::ApplyCommittedSelection(const FName NewOptionId, const bool bNotify)
{
	if (SelectedOptionId == NewOptionId)
	{
		UpdateCommittedPresentation();
		return;
	}

	const FName PreviousOptionId = SelectedOptionId;
	SelectedOptionId = NewOptionId;
	UpdateCommittedPresentation();
	if (bNotify)
	{
		OnSelectionChanged.Broadcast(PreviousOptionId, SelectedOptionId);
		NotifyLunarAccessibleValueChanged(NativeGetLunarAccessibleValueText());
	}
}

void ULunarComboBox::UpdateCommittedPresentation()
{
	if (SelectedOptionTextBlock)
	{
		SelectedOptionTextBlock->SetText(NativeGetLunarAccessibleValueText());
	}
}

void ULunarComboBox::ApplySpecializedStyle()
{
	if (ArrowImage.IsValid())
	{
		ArrowImage->SetImage(
			ResolvedComboBoxStyle.bOverrideArrowBrush
				? &ResolvedComboBoxStyle.ArrowBrush
				: FStyleDefaults::GetNoBrush());
	}

	if (PopupBackgroundImage)
	{
		if (!bHasPopupBaselineBrush)
		{
			PopupBaselineBrush = PopupBackgroundImage->GetBrush();
			bHasPopupBaselineBrush = true;
		}
		PopupBackgroundImage->SetBrush(
			ResolvedComboBoxStyle.bOverridePopupBrush
				? ResolvedComboBoxStyle.PopupBrush
				: PopupBaselineBrush);
	}

	if (OptionsListView)
	{
		OptionsListView->SetExternalPresentationStyle(
			ResolvedComboBoxStyle.bOverrideRowStyle ? &ResolvedComboBoxStyle.RowStyle : nullptr,
			ResolvedComboBoxStyle.bOverrideScrollBarStyle ? &ResolvedComboBoxStyle.ScrollBarStyle : nullptr);
	}
}

void ULunarComboBox::ReportConfigurationErrors(const TSet<FString>& CurrentErrors)
{
	for (const FString& Error : CurrentErrors)
	{
		if (!ReportedConfigurationErrors.Contains(Error))
		{
			UE_LOG(LogLunarComboBox, Error, TEXT("%s"), *Error);
			ULunarConsoleSubsystem::AddMessage(FGameplayTag(), ELunarConsoleMessageVerbosity::Error, Error);
		}
	}
	ReportedConfigurationErrors = CurrentErrors;
}

ULunarNavigationSubsystem* ULunarComboBox::GetNavigationSubsystem() const
{
	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		return LocalPlayer->GetSubsystem<ULunarNavigationSubsystem>();
	}
	return nullptr;
}

void ULunarComboBox::FinalizePopupClosed()
{
	const bool bWasOpen = bComboBoxOpen;
	bComboBoxOpen = false;
	bCloseRequestedWhileOpening = false;
	UnregisterOutsideClickProcessor();
	if (PopupRootWidget)
	{
		PopupRootWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	TemporaryOptionId = NAME_None;
	if (bWasOpen)
	{
		OnComboBoxClosed.Broadcast();
	}
}

void ULunarComboBox::RegisterOutsideClickProcessor()
{
	if (OutsideClickProcessor.IsValid() || !FSlateApplication::IsInitialized())
	{
		return;
	}
	OutsideClickProcessor = MakeShared<FLunarComboBoxOutsideClickInputProcessor>(this);
	if (!FSlateApplication::Get().RegisterInputPreProcessor(OutsideClickProcessor))
	{
		OutsideClickProcessor.Reset();
	}
}

void ULunarComboBox::UnregisterOutsideClickProcessor()
{
	if (!OutsideClickProcessor.IsValid())
	{
		return;
	}
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(OutsideClickProcessor);
	}
	OutsideClickProcessor.Reset();
}

bool ULunarComboBox::HandleGlobalPointerDown(const FVector2D& ScreenPosition, const uint32 SlateUserIndex)
{
	if (!bComboBoxOpen)
	{
		return false;
	}
	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (const TSharedPtr<FSlateUser> SlateUser = LocalPlayer->GetSlateUser())
		{
			if (SlateUser->GetUserIndex() != SlateUserIndex)
			{
				return false;
			}
		}
	}

	const bool bInsidePopup = PopupRootWidget
		&& PopupRootWidget->IsVisible()
		&& PopupRootWidget->GetCachedGeometry().IsUnderLocation(ScreenPosition);
	const bool bInsideTrigger = IsVisible()
		&& GetCachedGeometry().IsUnderLocation(ScreenPosition);
	if (!bInsidePopup && bInsideTrigger)
	{
		// Consume this pointer-down after closing. The matching release then reaches the
		// trigger without a base press and cannot immediately reopen the ComboBox.
		return CloseComboBox();
	}
	if (!bInsidePopup)
	{
		// Outside dismissal cancels the popup without stealing the click from the
		// destination widget underneath it.
		CloseComboBox();
	}
	return false;
}

void ULunarComboBox::HandleCultureChanged()
{
	if (bEnableSearch)
	{
		RebuildFilteredOptions(true);
	}
	UpdateCommittedPresentation();
	RefreshLunarAccessibility();
}

void ULunarComboBox::HandleListActiveItemChanged(const FName PreviousItemId, const FName NewItemId)
{
	TemporaryOptionId = NewItemId;
}

void ULunarComboBox::HandleListActivated()
{
	TemporaryOptionId = OptionsListView ? OptionsListView->GetActiveItemId() : NAME_None;
	CommitTemporarySelection();
}

void ULunarComboBox::HandleActiveScopeChanged(
	ULunarNavigationScope* /*PreviousScope*/,
	ULunarNavigationScope* /*NewScope*/)
{
	if (!bComboBoxOpen || !PopupScope)
	{
		return;
	}

	ULunarNavigationSubsystem* NavigationSubsystem = GetNavigationSubsystem();
	if (NavigationSubsystem
		&& NavigationSubsystem->GetNavigationScopeStack().Contains(PopupScope))
	{
		return;
	}
	if (bOpeningComboBox)
	{
		bCloseRequestedWhileOpening = true;
		return;
	}
	FinalizePopupClosed();
}
