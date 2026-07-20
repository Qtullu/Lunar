// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarComboBox.cpp
 * @brief Implements the self-contained Lunar ComboBox presentation, filtering, and nested popup scope.
 * @ingroup LunarNavigationControls
 */

#include "UI/Navigation/Controls/LunarComboBox.h"

#include "Curves/CurveFloat.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Application/IInputProcessor.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "Internationalization/Internationalization.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "Styling/StyleDefaults.h"
#include "Subsystems/Console/LunarConsoleSubsystem.h"
#include "UI/Navigation/Controls/LunarComboBoxEmptyVisual.h"
#include "UI/Navigation/Controls/LunarComboBoxEntry.h"
#include "UI/Navigation/Controls/LunarComboBoxSelectedVisual.h"
#include "UI/Navigation/Controls/LunarListView.h"
#include "UI/Navigation/Core/LunarNavigationScope.h"
#include "UI/Navigation/Core/LunarNavigationSubsystem.h"
#include "UI/Navigation/Types/LunarGameplayTags.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SMenuAnchor.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "LunarComboBox"

DEFINE_LOG_CATEGORY_STATIC(LogLunarComboBox, Log, All);

namespace LunarComboBox_Private
{
	const FSlateBrush* ResolveClosedBrush(
		const FButtonStyle& Style,
		const FLunarUIVisualState& State,
		const bool bEnabled)
	{
		if (!bEnabled)
		{
			return &Style.Disabled;
		}
		switch (State.InteractionState)
		{
		case ELunarUIInteractionState::PointerHovered:
		case ELunarUIInteractionState::NavigationSelected:
			return &Style.Hovered;
		case ELunarUIInteractionState::PointerPressed:
		case ELunarUIInteractionState::NavigationPressed:
			return &Style.Pressed;
		default:
			return &Style.Normal;
		}
	}

	FMargin ResolveClosedPadding(
		const FButtonStyle& Style,
		const FLunarUIVisualState& State)
	{
		return State.InteractionState == ELunarUIInteractionState::PointerPressed
			|| State.InteractionState == ELunarUIInteractionState::NavigationPressed
			? Style.PressedPadding
			: Style.NormalPadding;
	}
}

/** Observes global primary pointer-down for outside-popup dismissal. */
class FLunarComboBoxOutsideClickInputProcessor final : public IInputProcessor
{
public:
	/** @param InOwner Weak ComboBox owner receiving pointer-down callbacks. */
	explicit FLunarComboBoxOutsideClickInputProcessor(ULunarComboBox* InOwner)
		: Owner(InOwner)
	{
	}

	/** IInputProcessor tick; no work is required. */
	virtual void Tick(float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override
	{
	}

	/** Routes primary mouse and touch pointer-down before normal widget routing. */
	virtual bool HandleMouseButtonDownEvent(
		FSlateApplication& SlateApp,
		const FPointerEvent& MouseEvent) override
	{
		if (Owner.IsValid()
			&& (MouseEvent.IsTouchEvent()
				|| MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton))
		{
			return Owner->HandleGlobalPointerDown(
				MouseEvent.GetScreenSpacePosition(),
				MouseEvent.GetUserIndex());
		}
		return false;
	}

	/** Treats double-click as the matching pointer-down dismissal event. */
	virtual bool HandleMouseButtonDoubleClickEvent(
		FSlateApplication& SlateApp,
		const FPointerEvent& MouseEvent) override
	{
		return HandleMouseButtonDownEvent(SlateApp, MouseEvent);
	}

	/** @return Stable diagnostics label. */
	virtual const TCHAR* GetDebugName() const override
	{
		return TEXT("LunarComboBoxOutsideClick");
	}

private:
	/** Weak owner prevents Slate from extending the UWidget lifetime. */
	TWeakObjectPtr<ULunarComboBox> Owner;
};

ULunarComboBox::ULunarComboBox(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCanReceiveLunarSelection = true;
	bCanInteractWithPointer = true;
	bEnableInputPrompt = true;

	ClosedButtonStyle = FAppStyle::Get().GetWidgetStyle<FButtonStyle>(TEXT("Button"));
	ClosedTextStyle = FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>(TEXT("NormalText"));
	PopupRowStyle = FCoreStyle::Get().GetWidgetStyle<FTableRowStyle>(TEXT("TableView.Row"));
	PopupScrollBarStyle = FAppStyle::Get().GetWidgetStyle<FScrollBarStyle>(TEXT("ScrollBar"));
	EntryTextStyle = FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>(TEXT("NormalText"));
	EmptyResultsTextStyle = EntryTextStyle;
	PopupBackgroundBrush = *FAppStyle::Get().GetBrush(TEXT("Menu.Background"));
	ArrowBrush.DrawAs = ESlateBrushDrawType::NoDrawType;
	PlaceholderText = INVTEXT("Select...");
	EmptyResultsText = INVTEXT("No options");

	PopupScopeSettings.bRestoreLastSelection = false;
	PopupScopeSettings.bWrapHorizontal = false;
	PopupScopeSettings.bWrapVertical = false;

	FLunarPromptActionRequest AcceptPrompt;
	AcceptPrompt.ActionTag = LunarGameplayTags::UI_Action_Accept.GetTag();
	PromptActions.Add(AcceptPrompt);
}

void ULunarComboBox::SetOptions(
	const TArray<FLunarComboBoxOption>& NewOptions,
	const ELunarChangeNotificationPolicy NotificationPolicy)
{
	const int32 PreviousIndex = GetSelectedOptionIndex();
	Options = NewOptions;
	RebuildOptionLookup();
	NormalizeCommittedSelection(
		PreviousIndex != INDEX_NONE ? PreviousIndex : LastCommittedOptionIndex,
		NotificationPolicy == ELunarChangeNotificationPolicy::Notify);
	RebuildFilteredOptions(true, true);
	UpdateCommittedPresentation();
}

void ULunarComboBox::SetSelectedOptionById(
	const FName OptionId,
	const ELunarChangeNotificationPolicy NotificationPolicy)
{
	const bool bNotify = NotificationPolicy == ELunarChangeNotificationPolicy::Notify;
	if (OptionId.IsNone())
	{
		if (bAllowEmptySelection)
		{
			ApplyCommittedSelection(NAME_None, bNotify);
		}
		else
		{
			NormalizeCommittedSelection(LastCommittedOptionIndex, bNotify);
		}
		return;
	}

	const FLunarComboBoxOption* Option = FindOptionById(OptionId);
	if (!Option)
	{
		const FString Message = FString::Printf(
			TEXT("%s cannot select unknown or duplicate ComboBox OptionId '%s'."),
			*GetPathName(),
			*OptionId.ToString());
		UE_LOG(LogLunarComboBox, Error, TEXT("%s"), *Message);
		ULunarConsoleSubsystem::AddMessage(
			FGameplayTag(),
			ELunarConsoleMessageVerbosity::Error,
			Message);
		return;
	}
	if (!Option->bEnabled)
	{
		const FString Message = FString::Printf(
			TEXT("%s cannot commit disabled ComboBox OptionId '%s'."),
			*GetPathName(),
			*OptionId.ToString());
		UE_LOG(LogLunarComboBox, Warning, TEXT("%s"), *Message);
		ULunarConsoleSubsystem::AddMessage(
			FGameplayTag(),
			ELunarConsoleMessageVerbosity::Warning,
			Message);
		return;
	}

	ApplyCommittedSelection(OptionId, bNotify);
	if (bComboBoxOpen)
	{
		TemporaryOptionId = SelectedOptionId;
		RebuildFilteredOptions(true);
	}
}

int32 ULunarComboBox::GetSelectedOptionIndex() const
{
	const int32* OptionIndex = OptionIndexById.Find(SelectedOptionId);
	return OptionIndex ? *OptionIndex : INDEX_NONE;
}

bool ULunarComboBox::GetSelectedOption(FLunarComboBoxOption& OutOption) const
{
	if (const FLunarComboBoxOption* Option = FindOptionById(SelectedOptionId))
	{
		OutOption = *Option;
		return true;
	}
	OutOption = FLunarComboBoxOption();
	return false;
}

void ULunarComboBox::SetFilterText(const FText& NewFilterText)
{
	if (FilterText.EqualTo(NewFilterText))
	{
		return;
	}
	const FText PreviousFilterText = FilterText;
	FilterText = NewFilterText;
	SynchronizeGeneratedEmptyVisual();
	RebuildFilteredOptions(true);
	OnFilterTextChanged.Broadcast(this, PreviousFilterText, FilterText);
}

void ULunarComboBox::ClearFilter()
{
	SetFilterText(FText::GetEmpty());
}

void ULunarComboBox::RefreshFilter()
{
	RebuildFilteredOptions(true, true);
}

bool ULunarComboBox::DoesOptionMatchFilter_Implementation(
	const FLunarComboBoxOption& Option,
	const FText& Query) const
{
	if (Query.IsEmpty())
	{
		return true;
	}
	const FString QueryString = Query.ToString();
	return Option.DisplayText.ToString().Contains(QueryString, ESearchCase::IgnoreCase)
		|| Option.OptionId.ToString().Contains(QueryString, ESearchCase::IgnoreCase);
}

bool ULunarComboBox::DoesOptionMatchSearch_Implementation(
	const FLunarComboBoxOption& Option,
	const FText& Query) const
{
	return DoesOptionMatchFilter(Option, Query);
}

void ULunarComboBox::SetArrowBrush(const FSlateBrush& NewBrush)
{
	ArrowBrush = NewBrush;
	ApplyNativePresentation();
}

void ULunarComboBox::SetArrowTint(const FLinearColor NewTint)
{
	ArrowTint = NewTint;
	ApplyNativePresentation();
}

void ULunarComboBox::ConfigurePopupListPresentation(
	const FTableRowStyle& NewRowStyle,
	const FScrollBarStyle& NewScrollBarStyle)
{
	PopupRowStyle = NewRowStyle;
	PopupScrollBarStyle = NewScrollBarStyle;
	ApplyNativePresentation();
}

void ULunarComboBox::GetPopupListPresentation(
	FTableRowStyle& OutRowStyle,
	FScrollBarStyle& OutScrollBarStyle) const
{
	OutRowStyle = PopupRowStyle;
	OutScrollBarStyle = PopupScrollBarStyle;
}

void ULunarComboBox::ConfigureComboBoxPresentation(
	const FSlateBrush& NewArrowBrush,
	const FLinearColor NewArrowTint,
	const FTableRowStyle& NewRowStyle,
	const FScrollBarStyle& NewScrollBarStyle)
{
	ArrowBrush = NewArrowBrush;
	ArrowTint = NewArrowTint;
	PopupRowStyle = NewRowStyle;
	PopupScrollBarStyle = NewScrollBarStyle;
	ApplyNativePresentation();
}

void ULunarComboBox::GetComboBoxPresentation(
	FSlateBrush& OutArrowBrush,
	FLinearColor& OutArrowTint,
	FTableRowStyle& OutRowStyle,
	FScrollBarStyle& OutScrollBarStyle) const
{
	OutArrowBrush = ArrowBrush;
	OutArrowTint = ArrowTint;
	OutRowStyle = PopupRowStyle;
	OutScrollBarStyle = PopupScrollBarStyle;
}

TSharedRef<SWidget> ULunarComboBox::RebuildWidget()
{
	const TSharedRef<SWidget> BasePresentation = Super::RebuildWidget();
	const TSharedRef<SBox> InteractionSurface = SNew(SBox);
	InteractionSurface->SetVisibility(EVisibility::Visible);
	InteractionSurface->SetOnMouseButtonDown(
		BIND_UOBJECT_DELEGATE(FPointerEventHandler, NativeOnMouseButtonDown));
	InteractionSurface->SetOnMouseButtonUp(
		BIND_UOBJECT_DELEGATE(FPointerEventHandler, NativeOnMouseButtonUp));
	InteractionSurface->SetOnMouseDoubleClick(
		BIND_UOBJECT_DELEGATE(FPointerEventHandler, NativeOnMouseButtonDoubleClick));
#if WITH_ACCESSIBILITY
	InteractionSurface->SetAccessibleBehavior(EAccessibleBehavior::NotAccessible);
#endif

	return SNew(SOverlay)
		+ SOverlay::Slot()[BasePresentation]
		+ SOverlay::Slot()[InteractionSurface];
}

TSharedPtr<SWidget> ULunarComboBox::RebuildLunarSpecializedPresentation()
{
	EnsureInternalWidgets();
	RebuildOptionLookup();
	NormalizeCommittedSelection(LastCommittedOptionIndex, false);
	RebuildFilteredOptions(true, true);

	const TSharedRef<SWidget> ClosedPresentation = BuildClosedPresentation();
	const TSharedRef<SWidget> PopupPresentation = BuildPopupPresentation();

#if WITH_EDITORONLY_DATA
	if (IsDesignTime() && bPreviewPopupOpen)
	{
		ApplyPopupAnimationAlpha(1.0f);
		return BuildDesignerPreviewPresentation(
			ClosedPresentation,
			PopupPresentation);
	}
#endif

	SAssignNew(MenuAnchor, SMenuAnchor)
		.Placement(ResolveMenuPlacement())
		.FitInWindow(true)
		.UseApplicationMenuStack(false)
		.ShowMenuBackground(false)
		.OnMenuOpenChanged(BIND_UOBJECT_DELEGATE(
			FOnIsOpenChanged,
			HandleMenuOpenChanged))
		.MenuContent(PopupPresentation)
		[
			ClosedPresentation
		];

	ApplyNativePresentation();
	ApplyPopupAnimationAlpha(0.0f);
	return MenuAnchor;
}

void ULunarComboBox::ReleaseSlateResources(const bool bReleaseChildren)
{
	MenuAnchor.Reset();
	DefaultClosedBorder.Reset();
	DefaultClosedText.Reset();
	DefaultArrowImage.Reset();
	DefaultArrowGlyph.Reset();
	PopupSizingBox.Reset();
	PopupBackgroundBorder.Reset();
	PopupContentSwitcher.Reset();
	PopupAnimationRoot.Reset();
	GeneratedSelectedVisual = nullptr;
	GeneratedEmptyResultsVisual = nullptr;
	InternalOptionsList = nullptr;
	PopupScope = nullptr;
	Super::ReleaseSlateResources(bReleaseChildren);
}

void ULunarComboBox::SynchronizeProperties()
{
	RebuildOptionLookup();
	NormalizeCommittedSelection(LastCommittedOptionIndex, false);
	PopupInterpolationSpeed = FMath::IsFinite(PopupInterpolationSpeed)
		? FMath::Max(0.0f, PopupInterpolationSpeed)
		: 0.0f;
	PopupWidthOverride = FMath::IsFinite(PopupWidthOverride)
		? FMath::Max(0.0f, PopupWidthOverride)
		: 0.0f;
	MaximumPopupHeight = FMath::IsFinite(MaximumPopupHeight)
		? FMath::Max(1.0f, MaximumPopupHeight)
		: 1.0f;
	MaximumPopupWidth = FMath::IsFinite(MaximumPopupWidth)
		? FMath::Max(1.0f, MaximumPopupWidth)
		: 1.0f;

	Super::SynchronizeProperties();
	EnsureInternalWidgets();
	RebuildFilteredOptions(true, true);
	UpdateCommittedPresentation();
	ApplyNativePresentation();

#if WITH_EDITORONLY_DATA
	if (IsDesignTime())
	{
		ApplyPopupAnimationAlpha(bPreviewPopupOpen ? 1.0f : 0.0f);
	}
#endif
}

void ULunarComboBox::NativePreConstruct()
{
	Super::NativePreConstruct();
	RebuildOptionLookup();
	NormalizeCommittedSelection(LastCommittedOptionIndex, false);
	EnsureInternalWidgets();
	RebuildFilteredOptions(true, true);
	UpdateCommittedPresentation();
	ApplyNativePresentation();
}

void ULunarComboBox::NativeConstruct()
{
	Super::NativeConstruct();
	EnsureInternalWidgets();

	if (InternalOptionsList)
	{
		InternalOptionsList->OnActiveItemChanged.RemoveDynamic(
			this,
			&ULunarComboBox::HandleListActiveItemChanged);
		InternalOptionsList->OnActiveItemChanged.AddDynamic(
			this,
			&ULunarComboBox::HandleListActiveItemChanged);
		InternalOptionsList->OnItemActivated.RemoveDynamic(
			this,
			&ULunarComboBox::HandleListItemActivated);
		InternalOptionsList->OnItemActivated.AddDynamic(
			this,
			&ULunarComboBox::HandleListItemActivated);
	}
	if (ULunarNavigationSubsystem* NavigationSubsystem = GetNavigationSubsystem())
	{
		NavigationSubsystem->OnActiveScopeChanged.RemoveDynamic(
			this,
			&ULunarComboBox::HandleActiveScopeChanged);
		NavigationSubsystem->OnActiveScopeChanged.AddDynamic(
			this,
			&ULunarComboBox::HandleActiveScopeChanged);
	}
	if (!CultureChangedHandle.IsValid())
	{
		CultureChangedHandle = FInternationalization::Get()
			.OnCultureChanged()
			.AddUObject(this, &ULunarComboBox::HandleCultureChanged);
	}

	RebuildOptionLookup();
	NormalizeCommittedSelection(LastCommittedOptionIndex, false);
	RebuildFilteredOptions(true, true);
	UpdateCommittedPresentation();
	ApplyNativePresentation();
}

void ULunarComboBox::NativeDestruct()
{
	if (InternalOptionsList)
	{
		InternalOptionsList->OnActiveItemChanged.RemoveDynamic(
			this,
			&ULunarComboBox::HandleListActiveItemChanged);
		InternalOptionsList->OnItemActivated.RemoveDynamic(
			this,
			&ULunarComboBox::HandleListItemActivated);
		InternalOptionsList->SetExternalPresentationStyle(nullptr, nullptr);
		InternalOptionsList->SetExternalFallbackPresentation(nullptr, EntryPadding);
	}
	ULunarNavigationSubsystem* NavigationSubsystem = GetNavigationSubsystem();
	if (NavigationSubsystem)
	{
		NavigationSubsystem->OnActiveScopeChanged.RemoveDynamic(
			this,
			&ULunarComboBox::HandleActiveScopeChanged);
	}
	if (CultureChangedHandle.IsValid())
	{
		FInternationalization::Get().OnCultureChanged().Remove(CultureChangedHandle);
		CultureChangedHandle.Reset();
	}

	bPopupAnimationActive = false;
	bClosingComboBox = true;
	if (NavigationSubsystem
		&& PopupScope
		&& NavigationSubsystem->GetNavigationScopeStack().Contains(PopupScope))
	{
		NavigationSubsystem->PopNavigationScope(PopupScope);
	}
	bIgnoreMenuOpenChanged = true;
	if (MenuAnchor.IsValid() && MenuAnchor->IsOpen())
	{
		MenuAnchor->SetIsOpen(false, false);
	}
	bIgnoreMenuOpenChanged = false;
	bComboBoxOpen = false;
	bPopupClosing = false;
	bClosingComboBox = false;
	TemporaryOptionId = NAME_None;
	UnregisterOutsideClickProcessor();
	Super::NativeDestruct();
}

void ULunarComboBox::NativeTick(
	const FGeometry& MyGeometry,
	const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	UpdatePopupAnimation(InDeltaTime);
}

bool ULunarComboBox::NativeCanHandleLunarAction(
	const FLunarUIActionContext& ActionContext) const
{
	if (ActionContext.ActionTag == LunarGameplayTags::UI_Action_Accept.GetTag())
	{
		return IsLunarSelected();
	}
	return Super::NativeCanHandleLunarAction(ActionContext);
}

ELunarUIActionResult ULunarComboBox::NativeHandleLunarAction(
	const FLunarUIActionContext& ActionContext)
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
		&& InternalOptionsList
		&& MenuAnchor.IsValid()
		&& GetNavigationSubsystem();
}

void ULunarComboBox::NativeOnLunarActivated()
{
	Super::NativeOnLunarActivated();
	OpenComboBox();
}

void ULunarComboBox::NativeOnLunarVisualStateChanged(
	const FLunarUIVisualState& PreviousState,
	const FLunarUIVisualState& NewState,
	const bool bIsDesignerPreview)
{
	Super::NativeOnLunarVisualStateChanged(
		PreviousState,
		NewState,
		bIsDesignerPreview);
	if (IsValid(GeneratedSelectedVisual))
	{
		GeneratedSelectedVisual->ApplyVisualState(NewState);
	}
	ApplyNativePresentation();
}

FText ULunarComboBox::NativeGetLunarAccessibleValueText() const
{
	if (const FLunarComboBoxOption* Option = FindOptionById(SelectedOptionId))
	{
		return Option->DisplayText;
	}
	return PlaceholderText;
}

bool ULunarComboBox::OpenComboBox()
{
	if (bComboBoxOpen && !bPopupClosing)
	{
		return true;
	}
	if (bOpeningComboBox)
	{
		return true;
	}

	EnsureInternalWidgets();
	ULunarNavigationSubsystem* NavigationSubsystem = GetNavigationSubsystem();
	if (!InternalOptionsList || !MenuAnchor.IsValid() || !NavigationSubsystem)
	{
		const FString Message = FString::Printf(
			TEXT("%s cannot open its generated popup because the internal list, menu anchor, or owning navigation subsystem is unavailable."),
			*GetPathName());
		UE_LOG(LogLunarComboBox, Error, TEXT("%s"), *Message);
		ULunarConsoleSubsystem::AddMessage(
			FGameplayTag(),
			ELunarConsoleMessageVerbosity::Error,
			Message);
		return false;
	}

	RebuildOptionLookup();
	NormalizeCommittedSelection(LastCommittedOptionIndex, false);
	SetTemporarySelectionToCommittedOrFirstEligible();
	ApplyPopupSizing();

	if (!PopupScope)
	{
		PopupScope = NewObject<ULunarNavigationScope>(this);
	}
	FLunarNavigationScopeSettings ScopeSettings = PopupScopeSettings;
	ScopeSettings.InitialSelectionWidget = InternalOptionsList;
	ScopeSettings.InitialSelectionId = InternalOptionsList->GetNavigationId();
	ScopeSettings.bRestoreLastSelection = false;
	ScopeSettings.bWrapHorizontal = false;
	ScopeSettings.bWrapVertical = false;
	PopupScope->RootWidget = InternalOptionsList;
	PopupScope->Settings = ScopeSettings;

	bOpeningComboBox = true;
	bCloseRequestedWhileOpening = false;
	bIgnoreMenuOpenChanged = true;
	MenuAnchor->SetMenuPlacement(ResolveMenuPlacement());
	int32 SlateUserIndex = 0;
	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (const TSharedPtr<FSlateUser> SlateUser = LocalPlayer->GetSlateUser())
		{
			SlateUserIndex = SlateUser->GetUserIndex();
		}
	}
	MenuAnchor->SetIsOpen(
		true,
		false,
		SlateUserIndex);
	bIgnoreMenuOpenChanged = false;

	const bool bMenuOpened = MenuAnchor->IsOpen();
	const bool bScopePushed = bMenuOpened
		&& NavigationSubsystem->PushNavigationScope(PopupScope);
	bOpeningComboBox = false;

	if (!bScopePushed
		|| !NavigationSubsystem->GetNavigationScopeStack().Contains(PopupScope))
	{
		bIgnoreMenuOpenChanged = true;
		if (MenuAnchor->IsOpen())
		{
			MenuAnchor->SetIsOpen(false, false);
		}
		bIgnoreMenuOpenChanged = false;
		bCloseRequestedWhileOpening = false;
		return false;
	}

	bComboBoxOpen = true;
	bPopupClosing = false;
	RegisterOutsideClickProcessor();
	BeginPopupAnimation(1.0f);
	OnComboBoxOpened.Broadcast(this);

	const bool bShouldClose = bCloseRequestedWhileOpening;
	bCloseRequestedWhileOpening = false;
	if (bShouldClose)
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
	if (bClosingComboBox || bPopupClosing)
	{
		bCloseRequestedWhileClosing = true;
		return true;
	}

	bClosingComboBox = true;
	bPopupClosing = true;
	bCloseRequestedWhileClosing = false;
	UnregisterOutsideClickProcessor();

	ULunarNavigationSubsystem* NavigationSubsystem = GetNavigationSubsystem();
	if (NavigationSubsystem
		&& PopupScope
		&& NavigationSubsystem->GetNavigationScopeStack().Contains(PopupScope)
		&& !NavigationSubsystem->PopNavigationScope(PopupScope))
	{
		bClosingComboBox = false;
		bPopupClosing = false;
		return false;
	}

	bClosingComboBox = false;
	BeginPopupAnimation(0.0f);
	const bool bShouldCloseAgain = bCloseRequestedWhileClosing;
	bCloseRequestedWhileClosing = false;
	if (bShouldCloseAgain && bComboBoxOpen && !bPopupClosing)
	{
		return CloseComboBox();
	}
	return true;
}

const FLunarComboBoxOption* ULunarComboBox::FindOptionById(
	const FName OptionId) const
{
	if (const int32* OptionIndex = OptionIndexById.Find(OptionId))
	{
		return Options.IsValidIndex(*OptionIndex)
			? &Options[*OptionIndex]
			: nullptr;
	}
	return nullptr;
}

bool ULunarComboBox::IsOptionNavigable(const FName OptionId) const
{
	if (const FLunarComboBoxOption* Option = FindOptionById(OptionId))
	{
		return Option->bEnabled || Option->bCanReceiveSelectionWhenDisabled;
	}
	return false;
}

bool ULunarComboBox::IsOptionCommittable(const FName OptionId) const
{
	if (const FLunarComboBoxOption* Option = FindOptionById(OptionId))
	{
		return Option->bEnabled;
	}
	return false;
}

void ULunarComboBox::RebuildOptionLookup()
{
	OptionIndexById.Reset();
	TSet<FString> CurrentErrors;
	TMap<FName, int32> OptionIdCounts;

	for (int32 OptionIndex = 0; OptionIndex < Options.Num(); ++OptionIndex)
	{
		const FLunarComboBoxOption& Option = Options[OptionIndex];
		if (Option.OptionId.IsNone())
		{
			CurrentErrors.Add(FString::Printf(
				TEXT("%s Options[%d] has an empty OptionId and is excluded."),
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
				TEXT("%s contains %d ComboBox options with duplicate OptionId '%s'. All duplicates are excluded."),
				*GetPathName(),
				Pair.Value,
				*Pair.Key.ToString()));
		}
	}

	for (int32 OptionIndex = 0; OptionIndex < Options.Num(); ++OptionIndex)
	{
		const FLunarComboBoxOption& Option = Options[OptionIndex];
		if (!Option.OptionId.IsNone()
			&& OptionIdCounts.FindRef(Option.OptionId) == 1)
		{
			OptionIndexById.Add(Option.OptionId, OptionIndex);
		}
	}
	ReportConfigurationErrors(CurrentErrors);
}

int32 ULunarComboBox::FindRecoveryIndex(const int32 PreferredIndex) const
{
	if (Options.IsEmpty())
	{
		return INDEX_NONE;
	}

	const int32 RequestedIndex = SelectionRecoveryMode
			== ELunarComboBoxSelectionRecoveryMode::ForceIndex
		? ForcedSelectionIndex
		: (PreferredIndex != INDEX_NONE
			? PreferredIndex
			: (LastCommittedOptionIndex != INDEX_NONE
				? LastCommittedOptionIndex
				: 0));
	const int32 StartIndex = FMath::Clamp(
		RequestedIndex,
		0,
		Options.Num() - 1);

	for (int32 Distance = 0; Distance < Options.Num(); ++Distance)
	{
		const int32 ForwardIndex = StartIndex + Distance;
		if (Options.IsValidIndex(ForwardIndex))
		{
			const FLunarComboBoxOption& Option = Options[ForwardIndex];
			if (OptionIndexById.Contains(Option.OptionId) && Option.bEnabled)
			{
				return ForwardIndex;
			}
		}

		const int32 BackwardIndex = StartIndex - Distance;
		if (Distance > 0 && Options.IsValidIndex(BackwardIndex))
		{
			const FLunarComboBoxOption& Option = Options[BackwardIndex];
			if (OptionIndexById.Contains(Option.OptionId) && Option.bEnabled)
			{
				return BackwardIndex;
			}
		}
	}
	return INDEX_NONE;
}

void ULunarComboBox::NormalizeCommittedSelection(
	const int32 PreferredIndex,
	const bool bNotify)
{
	if (IsOptionCommittable(SelectedOptionId))
	{
		LastCommittedOptionIndex = GetSelectedOptionIndex();
		return;
	}
	if (bAllowEmptySelection)
	{
		ApplyCommittedSelection(NAME_None, bNotify);
		return;
	}

	const int32 RecoveryIndex = FindRecoveryIndex(PreferredIndex);
	ApplyCommittedSelection(
		Options.IsValidIndex(RecoveryIndex)
			? Options[RecoveryIndex].OptionId
			: NAME_None,
		bNotify);
}

void ULunarComboBox::RebuildFilteredOptions(
	const bool bPreserveTemporarySelection,
	const bool bUseListViewRestoration)
{
	if (!InternalOptionsList)
	{
		return;
	}

	const FName PreviousTemporaryId = bPreserveTemporarySelection
		? TemporaryOptionId
		: NAME_None;
	TArray<FLunarListViewItemData> FilteredItems;
	TSet<FName> FilteredIds;

	for (int32 OptionIndex = 0; OptionIndex < Options.Num(); ++OptionIndex)
	{
		const FLunarComboBoxOption& Option = Options[OptionIndex];
		const int32* ValidIndex = OptionIndexById.Find(Option.OptionId);
		if (!ValidIndex
			|| *ValidIndex != OptionIndex
			|| !DoesOptionMatchFilter(Option, FilterText))
		{
			continue;
		}

		FLunarListViewItemData& ItemData =
			FilteredItems.AddDefaulted_GetRef();
		ItemData.ItemId = Option.OptionId;
		ItemData.DisplayText = Option.DisplayText;
		ItemData.bEnabled = Option.bEnabled;
		ItemData.bCanReceiveSelectionWhenDisabled =
			Option.bCanReceiveSelectionWhenDisabled;
		ItemData.DisabledReason = FText::AsCultureInvariant(Option.DisabledReason);
		ItemData.Payload = Option.Payload;
		FilteredIds.Add(Option.OptionId);
	}

	InternalOptionsList->SetItems(
		FilteredItems,
		ELunarChangeNotificationPolicy::Silent);

	auto TryRestore = [this, &FilteredIds](const FName CandidateId)
	{
		return !CandidateId.IsNone()
			&& FilteredIds.Contains(CandidateId)
			&& IsOptionNavigable(CandidateId)
			&& InternalOptionsList->SetActiveItemById(
				CandidateId,
				ELunarChangeNotificationPolicy::Silent);
	};

	if (TryRestore(PreviousTemporaryId))
	{
		TemporaryOptionId = PreviousTemporaryId;
		UpdatePopupContentMode();
		return;
	}

	if (bUseListViewRestoration)
	{
		const FName RestoredId = InternalOptionsList->GetActiveItemId();
		if (TryRestore(RestoredId))
		{
			TemporaryOptionId = RestoredId;
			UpdatePopupContentMode();
			return;
		}
	}

	if (TryRestore(SelectedOptionId))
	{
		TemporaryOptionId = SelectedOptionId;
		UpdatePopupContentMode();
		return;
	}

	for (const FLunarListViewItemData& ItemData : FilteredItems)
	{
		if (TryRestore(ItemData.ItemId))
		{
			TemporaryOptionId = ItemData.ItemId;
			UpdatePopupContentMode();
			return;
		}
	}

	InternalOptionsList->ClearActiveItem(
		ELunarChangeNotificationPolicy::Silent);
	TemporaryOptionId = NAME_None;
	UpdatePopupContentMode();
}

void ULunarComboBox::SetTemporarySelectionToCommittedOrFirstEligible()
{
	TemporaryOptionId = SelectedOptionId;
	RebuildFilteredOptions(true, true);
}

void ULunarComboBox::CommitTemporarySelection()
{
	if (!IsOptionCommittable(TemporaryOptionId))
	{
		return;
	}
	ApplyCommittedSelection(TemporaryOptionId, true);
	CloseComboBox();
}

void ULunarComboBox::ApplyCommittedSelection(
	const FName NewOptionId,
	const bool bNotify)
{
	const FName NormalizedOptionId = IsOptionCommittable(NewOptionId)
		? NewOptionId
		: NAME_None;
	if (SelectedOptionId == NormalizedOptionId)
	{
		UpdateCommittedPresentation();
		return;
	}

	const FName PreviousOptionId = SelectedOptionId;
	SelectedOptionId = NormalizedOptionId;
	if (const int32* NewIndex = OptionIndexById.Find(SelectedOptionId))
	{
		LastCommittedOptionIndex = *NewIndex;
	}
	UpdateCommittedPresentation();
	RefreshLunarAccessibility();

	if (bNotify)
	{
		OnSelectionChanged.Broadcast(
			this,
			PreviousOptionId,
			SelectedOptionId);
		NotifyLunarAccessibleValueChanged(
			NativeGetLunarAccessibleValueText());
	}
}

void ULunarComboBox::UpdateCommittedPresentation()
{
	const FLunarComboBoxOption* SelectedOption =
		FindOptionById(SelectedOptionId);
	const FText DisplayText = SelectedOption
		? SelectedOption->DisplayText
		: PlaceholderText;

	if (DefaultClosedText.IsValid())
	{
		DefaultClosedText->SetText(DisplayText);
	}
	SynchronizeGeneratedSelectedVisual();
	if (InternalOptionsList)
	{
		InternalOptionsList->RefreshNavigationItems(
			ELunarChangeNotificationPolicy::Silent);
	}
}

void ULunarComboBox::ApplyNativePresentation()
{
	if (DefaultClosedBorder.IsValid())
	{
		DefaultClosedBorder->SetBorderImage(
			LunarComboBox_Private::ResolveClosedBrush(
				ClosedButtonStyle,
				GetLunarVisualState(),
				IsLunarEnabled()));
		DefaultClosedBorder->SetPadding(
			LunarComboBox_Private::ResolveClosedPadding(
				ClosedButtonStyle,
				GetLunarVisualState()));
	}
	if (DefaultClosedText.IsValid())
	{
		DefaultClosedText->SetTextStyle(&ClosedTextStyle, true);
	}
	if (DefaultArrowImage.IsValid())
	{
		DefaultArrowImage->SetImage(&ArrowBrush);
		DefaultArrowImage->SetColorAndOpacity(FSlateColor(ArrowTint));
		DefaultArrowImage->SetVisibility(
			ArrowBrush.DrawAs == ESlateBrushDrawType::NoDrawType
				? EVisibility::Collapsed
				: EVisibility::HitTestInvisible);
	}
	if (DefaultArrowGlyph.IsValid())
	{
		DefaultArrowGlyph->SetTextStyle(&ClosedTextStyle, true);
		DefaultArrowGlyph->SetColorAndOpacity(FSlateColor(ArrowTint));
		DefaultArrowGlyph->SetText(
			PopupOrientation == Orient_Horizontal
				? LOCTEXT("HorizontalArrowGlyph", "?")
				: LOCTEXT("VerticalArrowGlyph", "?"));
		DefaultArrowGlyph->SetVisibility(
			ArrowBrush.DrawAs == ESlateBrushDrawType::NoDrawType
				? EVisibility::HitTestInvisible
				: EVisibility::Collapsed);
	}
	if (PopupBackgroundBorder.IsValid())
	{
		PopupBackgroundBorder->SetBorderImage(&PopupBackgroundBrush);
		PopupBackgroundBorder->SetPadding(PopupPadding);
	}
	if (InternalOptionsList)
	{
		InternalOptionsList->EntryWidgetClass = EntryVisualClass;
		InternalOptionsList->Orientation = PopupOrientation;
		InternalOptionsList->bWrapNavigation = bWrapNavigation;
		InternalOptionsList->SetExternalPresentationStyle(
			&PopupRowStyle,
			&PopupScrollBarStyle);
		InternalOptionsList->SetExternalFallbackPresentation(
			&EntryTextStyle,
			EntryPadding);
	}
	SynchronizeGeneratedSelectedVisual();
	SynchronizeGeneratedEmptyVisual();
	ApplyPopupSizing();
}

void ULunarComboBox::ApplyPopupSizing()
{
	if (!PopupSizingBox.IsValid())
	{
		return;
	}

	PopupSizingBox->SetMaxDesiredHeight(
		FOptionalSize(FMath::Max(1.0f, MaximumPopupHeight)));
	PopupSizingBox->SetMaxDesiredWidth(
		FOptionalSize(FMath::Max(1.0f, MaximumPopupWidth)));

	float ResolvedWidth = PopupWidthOverride;
	if (ResolvedWidth <= 0.0f
		&& PopupOrientation == Orient_Vertical)
	{
		ResolvedWidth = GetCachedGeometry().GetLocalSize().X;
	}
	PopupSizingBox->SetWidthOverride(
		ResolvedWidth > 0.0f
			? FOptionalSize(FMath::Min(
				ResolvedWidth,
				MaximumPopupWidth))
			: FOptionalSize());
}

void ULunarComboBox::UpdatePopupContentMode()
{
	SynchronizeGeneratedEmptyVisual();
	if (PopupContentSwitcher.IsValid())
	{
		PopupContentSwitcher->SetActiveWidgetIndex(
			InternalOptionsList && InternalOptionsList->GetNumItems() > 0
				? 0
				: 1);
	}
}

void ULunarComboBox::EnsureInternalWidgets()
{
	if (!InternalOptionsList)
	{
		InternalOptionsList = CreateWidget<ULunarListView>(
			this,
			ULunarListView::StaticClass());
	}
	if (!InternalOptionsList)
	{
		return;
	}

	InternalOptionsList->NavigationId =
		TEXT("LunarComboBox_InternalOptions");
	InternalOptionsList->bCanReceiveLunarSelection = true;
	InternalOptionsList->bCanInteractWithPointer = bCanInteractWithPointer;
	InternalOptionsList->bAllowTouchInput = bAllowTouchInput;
	InternalOptionsList->bAllowKeyboardInput = bAllowKeyboardInput;
	InternalOptionsList->bAllowGamepadInput = bAllowGamepadInput;
	InternalOptionsList->bEnableInputPrompt = false;
	InternalOptionsList->Orientation = PopupOrientation;
	InternalOptionsList->bWrapNavigation = bWrapNavigation;
	InternalOptionsList->EntryWidgetClass = EntryVisualClass;
	InternalOptionsList->SetSelectionMode(
		ELunarListViewSelectionMode::Single,
		ELunarChangeNotificationPolicy::Silent);
	InternalOptionsList->SetExternalPresentationStyle(
		&PopupRowStyle,
		&PopupScrollBarStyle);
	InternalOptionsList->SetExternalFallbackPresentation(
		&EntryTextStyle,
		EntryPadding);
}

void ULunarComboBox::SynchronizeGeneratedSelectedVisual()
{
	if (!IsValid(GeneratedSelectedVisual))
	{
		return;
	}

	FLunarComboBoxOption SelectedOption;
	const bool bHasSelection = GetSelectedOption(SelectedOption);
	GeneratedSelectedVisual->ApplySelectedOption(
		SelectedOption,
		!bHasSelection);
	GeneratedSelectedVisual->ApplyVisualState(GetLunarVisualState());
}

void ULunarComboBox::SynchronizeGeneratedEmptyVisual()
{
	if (IsValid(GeneratedEmptyResultsVisual))
	{
		GeneratedEmptyResultsVisual->ApplyFilterText(FilterText);
	}
}

TSharedRef<SWidget> ULunarComboBox::BuildClosedPresentation()
{
	GeneratedSelectedVisual = nullptr;
	if (SelectedVisualClass)
	{
		GeneratedSelectedVisual = CreateWidget<ULunarComboBoxSelectedVisual>(
			this,
			SelectedVisualClass);
		if (IsValid(GeneratedSelectedVisual))
		{
			FLunarComboBoxOption SelectedOption;
			const bool bHasSelection = GetSelectedOption(SelectedOption);
			GeneratedSelectedVisual->InitializeFromComboBox(
				this,
				SelectedOption,
				!bHasSelection,
				GetLunarVisualState());
			GeneratedSelectedVisual->SetVisibility(
				ESlateVisibility::HitTestInvisible);
			return GeneratedSelectedVisual->TakeWidget();
		}
	}

	TSharedRef<SHorizontalBox> Content = SNew(SHorizontalBox);
	Content->AddSlot()
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		[
			SAssignNew(DefaultClosedText, STextBlock)
			.Text(NativeGetLunarAccessibleValueText())
			.TextStyle(&ClosedTextStyle)
		];
	Content->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(FMargin(8.0f, 0.0f, 0.0f, 0.0f))
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SAssignNew(DefaultArrowImage, SImage)
				.Image(&ArrowBrush)
				.ColorAndOpacity(FSlateColor(ArrowTint))
			]
			+ SOverlay::Slot()
			[
				SAssignNew(DefaultArrowGlyph, STextBlock)
				.TextStyle(&ClosedTextStyle)
			]
		];

	SAssignNew(DefaultClosedBorder, SBorder)
		.BorderImage(&ClosedButtonStyle.Normal)
		.Padding(ClosedButtonStyle.NormalPadding)
		[
			Content
		];
#if WITH_ACCESSIBILITY
	DefaultClosedBorder->SetAccessibleBehavior(
		EAccessibleBehavior::NotAccessible);
#endif
	ApplyNativePresentation();
	return DefaultClosedBorder.ToSharedRef();
}

TSharedRef<SWidget> ULunarComboBox::BuildPopupPresentation()
{
	TSharedRef<SWidget> EmptyPresentation =
		SNew(SBorder)
		.BorderImage(FStyleDefaults::GetNoBrush())
		.Padding(EmptyResultsPadding)
		[
			SNew(STextBlock)
			.Text(EmptyResultsText)
			.TextStyle(&EmptyResultsTextStyle)
		];

	GeneratedEmptyResultsVisual = nullptr;
	if (EmptyResultsVisualClass)
	{
		GeneratedEmptyResultsVisual =
			CreateWidget<ULunarComboBoxEmptyVisual>(
				this,
				EmptyResultsVisualClass);
		if (IsValid(GeneratedEmptyResultsVisual))
		{
			GeneratedEmptyResultsVisual->InitializeFromComboBox(
				this,
				FilterText);
			GeneratedEmptyResultsVisual->SetVisibility(
				ESlateVisibility::HitTestInvisible);
			EmptyPresentation =
				GeneratedEmptyResultsVisual->TakeWidget();
		}
	}

	const TSharedRef<SWidget> ListPresentation = InternalOptionsList
		? InternalOptionsList->TakeWidget()
		: SNullWidget::NullWidget;

	SAssignNew(PopupContentSwitcher, SWidgetSwitcher)
		+ SWidgetSwitcher::Slot()[ListPresentation]
		+ SWidgetSwitcher::Slot()[EmptyPresentation];

	SAssignNew(PopupBackgroundBorder, SBorder)
		.BorderImage(&PopupBackgroundBrush)
		.Padding(PopupPadding)
		[
			PopupContentSwitcher.ToSharedRef()
		];

	SAssignNew(PopupSizingBox, SBox)
		.MaxDesiredWidth(MaximumPopupWidth)
		.MaxDesiredHeight(MaximumPopupHeight)
		[
			PopupBackgroundBorder.ToSharedRef()
		];

	PopupAnimationRoot = PopupSizingBox;
	UpdatePopupContentMode();
	ApplyPopupSizing();
	return PopupSizingBox.ToSharedRef();
}

TSharedRef<SWidget> ULunarComboBox::BuildDesignerPreviewPresentation(
	const TSharedRef<SWidget>& ClosedPresentation,
	const TSharedRef<SWidget>& PopupPresentation)
{
	const bool bAbove = IsResolvedPlacementAbove();
	TSharedRef<SVerticalBox> Preview = SNew(SVerticalBox);
	if (bAbove)
	{
		Preview->AddSlot().AutoHeight()[PopupPresentation];
	}
	Preview->AddSlot().AutoHeight()[ClosedPresentation];
	if (!bAbove)
	{
		Preview->AddSlot().AutoHeight()[PopupPresentation];
	}
	return Preview;
}

EMenuPlacement ULunarComboBox::ResolveMenuPlacement() const
{
	switch (PopupPlacement)
	{
	case ELunarComboBoxPopupPlacement::BelowCenter:
		return MenuPlacement_CenteredBelowAnchor;
	case ELunarComboBoxPopupPlacement::BelowRight:
		return MenuPlacement_BelowRightAnchor;
	case ELunarComboBoxPopupPlacement::AboveLeft:
		return MenuPlacement_AboveAnchor;
	case ELunarComboBoxPopupPlacement::AboveCenter:
		return MenuPlacement_CenteredAboveAnchor;
	case ELunarComboBoxPopupPlacement::AboveRight:
		return MenuPlacement_AboveRightAnchor;
	case ELunarComboBoxPopupPlacement::Auto:
	case ELunarComboBoxPopupPlacement::BelowLeft:
	default:
		return MenuPlacement_BelowAnchor;
	}
}

bool ULunarComboBox::IsResolvedPlacementAbove() const
{
	switch (PopupPlacement)
	{
	case ELunarComboBoxPopupPlacement::AboveLeft:
	case ELunarComboBoxPopupPlacement::AboveCenter:
	case ELunarComboBoxPopupPlacement::AboveRight:
		return true;
	case ELunarComboBoxPopupPlacement::Auto:
		if (MenuAnchor.IsValid() && MenuAnchor->IsOpen())
		{
			return MenuAnchor->GetMenuPosition().Y
				< GetCachedGeometry().GetAbsolutePosition().Y;
		}
		return false;
	default:
		return false;
	}
}

void ULunarComboBox::BeginPopupAnimation(const float TargetAlpha)
{
	const float SafeTarget = FMath::Clamp(TargetAlpha, 0.0f, 1.0f);
	if (MustSnapPopupAnimation())
	{
		bPopupAnimationActive = false;
		PopupAnimationSourceAlpha = SafeTarget;
		PopupAnimationTargetAlpha = SafeTarget;
		PopupAnimationElapsed = 0.0f;
		ApplyPopupAnimationAlpha(SafeTarget);
		if (SafeTarget <= 0.0f && bPopupClosing)
		{
			FinalizePopupClosed();
		}
		return;
	}

	PopupAnimationSourceAlpha = PopupAnimationAlpha;
	PopupAnimationTargetAlpha = SafeTarget;
	PopupAnimationElapsed = 0.0f;
	bPopupAnimationActive =
		!FMath::IsNearlyEqual(
			PopupAnimationSourceAlpha,
			PopupAnimationTargetAlpha);
	if (!bPopupAnimationActive
		&& SafeTarget <= 0.0f
		&& bPopupClosing)
	{
		FinalizePopupClosed();
	}
}

void ULunarComboBox::UpdatePopupAnimation(const float DeltaTime)
{
	if (!bPopupAnimationActive)
	{
		return;
	}
	if (MustSnapPopupAnimation())
	{
		const float TargetAlpha = PopupAnimationTargetAlpha;
		bPopupAnimationActive = false;
		ApplyPopupAnimationAlpha(TargetAlpha);
		if (TargetAlpha <= 0.0f && bPopupClosing)
		{
			FinalizePopupClosed();
		}
		return;
	}
	if (!FMath::IsFinite(DeltaTime) || DeltaTime <= 0.0f)
	{
		return;
	}

	PopupAnimationElapsed += DeltaTime;
	const float Progress = FMath::Clamp(
		PopupAnimationElapsed * PopupInterpolationSpeed,
		0.0f,
		1.0f);
	const float CurveAlpha = EvaluatePopupCurve(Progress);
	const float NewAlpha = Progress >= 1.0f
		? PopupAnimationTargetAlpha
		: FMath::Lerp(
			PopupAnimationSourceAlpha,
			PopupAnimationTargetAlpha,
			CurveAlpha);
	ApplyPopupAnimationAlpha(NewAlpha);

	if (Progress >= 1.0f)
	{
		bPopupAnimationActive = false;
		ApplyPopupAnimationAlpha(PopupAnimationTargetAlpha);
		if (PopupAnimationTargetAlpha <= 0.0f && bPopupClosing)
		{
			FinalizePopupClosed();
		}
	}
}

void ULunarComboBox::ApplyPopupAnimationAlpha(const float NewAlpha)
{
	PopupAnimationAlpha = FMath::Clamp(NewAlpha, 0.0f, 1.0f);
	if (!PopupAnimationRoot.IsValid())
	{
		return;
	}

	PopupAnimationRoot->SetRenderOpacity(PopupAnimationAlpha);
	const float HiddenDirection = IsResolvedPlacementAbove()
		? 1.0f
		: -1.0f;
	FWidgetTransform Transform;
	Transform.Translation = FVector2D(
		PopupAnimationOffset.X * (1.0f - PopupAnimationAlpha),
		PopupAnimationOffset.Y
			* HiddenDirection
			* (1.0f - PopupAnimationAlpha));
	PopupAnimationRoot->SetRenderTransform(
		Transform.ToSlateRenderTransform());
}

float ULunarComboBox::EvaluatePopupCurve(const float Progress) const
{
	const float SafeProgress = FMath::Clamp(Progress, 0.0f, 1.0f);
	if (IsValid(PopupInterpolationCurve))
	{
		const float Evaluated =
			PopupInterpolationCurve->GetFloatValue(SafeProgress);
		if (FMath::IsFinite(Evaluated))
		{
			return FMath::Clamp(Evaluated, 0.0f, 1.0f);
		}
	}
	return SafeProgress;
}

bool ULunarComboBox::MustSnapPopupAnimation() const
{
	return IsDesignTime()
		|| !bInterpolatePopup
		|| GetLunarVisualState().bReduceMotion
		|| !FMath::IsFinite(PopupInterpolationSpeed)
		|| PopupInterpolationSpeed <= 0.0f;
}

void ULunarComboBox::FinalizePopupClosed()
{
	const bool bWasOpen = bComboBoxOpen;
	bPopupAnimationActive = false;
	bComboBoxOpen = false;
	bPopupClosing = false;
	bCloseRequestedWhileOpening = false;
	bCloseRequestedWhileClosing = false;
	UnregisterOutsideClickProcessor();

	bIgnoreMenuOpenChanged = true;
	if (MenuAnchor.IsValid() && MenuAnchor->IsOpen())
	{
		MenuAnchor->SetIsOpen(false, false);
	}
	bIgnoreMenuOpenChanged = false;
	ApplyPopupAnimationAlpha(0.0f);
	TemporaryOptionId = NAME_None;

	if (!bPreserveFilterText && !FilterText.IsEmpty())
	{
		SetFilterText(FText::GetEmpty());
	}
	if (bWasOpen)
	{
		OnComboBoxClosed.Broadcast(this);
	}
}

void ULunarComboBox::ReportConfigurationErrors(
	const TSet<FString>& CurrentErrors)
{
	for (const FString& Error : CurrentErrors)
	{
		if (!ReportedConfigurationErrors.Contains(Error))
		{
			UE_LOG(LogLunarComboBox, Error, TEXT("%s"), *Error);
			ULunarConsoleSubsystem::AddMessage(
				FGameplayTag(),
				ELunarConsoleMessageVerbosity::Error,
				Error);
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

void ULunarComboBox::RegisterOutsideClickProcessor()
{
	if (OutsideClickProcessor.IsValid()
		|| !FSlateApplication::IsInitialized())
	{
		return;
	}

	OutsideClickProcessor =
		MakeShared<FLunarComboBoxOutsideClickInputProcessor>(this);
	if (!FSlateApplication::Get().RegisterInputPreProcessor(
		OutsideClickProcessor))
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
		FSlateApplication::Get().UnregisterInputPreProcessor(
			OutsideClickProcessor);
	}
	OutsideClickProcessor.Reset();
}

bool ULunarComboBox::HandleGlobalPointerDown(
	const FVector2D& ScreenPosition,
	const uint32 SlateUserIndex)
{
	if (!bComboBoxOpen || bPopupClosing)
	{
		return false;
	}
	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (const TSharedPtr<FSlateUser> SlateUser =
			LocalPlayer->GetSlateUser())
		{
			if (SlateUser->GetUserIndex() != SlateUserIndex)
			{
				return false;
			}
		}
	}

	const bool bInsidePopup = PopupAnimationRoot.IsValid()
		&& PopupAnimationRoot->GetVisibility().IsVisible()
		&& PopupAnimationRoot->GetCachedGeometry()
			.IsUnderLocation(ScreenPosition);
	const bool bInsideTrigger = IsVisible()
		&& GetCachedGeometry().IsUnderLocation(ScreenPosition);

	if (!bInsidePopup && bInsideTrigger)
	{
		return CloseComboBox();
	}
	if (!bInsidePopup)
	{
		CloseComboBox();
	}
	return false;
}

void ULunarComboBox::HandleCultureChanged()
{
	RebuildFilteredOptions(true, true);
	UpdateCommittedPresentation();
	RefreshLunarAccessibility();
}

void ULunarComboBox::HandleMenuOpenChanged(const bool bIsOpen)
{
	if (bIgnoreMenuOpenChanged || bIsOpen)
	{
		return;
	}
	if (bOpeningComboBox)
	{
		bCloseRequestedWhileOpening = true;
		return;
	}
	if (bComboBoxOpen && !bPopupClosing)
	{
		CloseComboBox();
	}
}

void ULunarComboBox::HandleListActiveItemChanged(
	ULunarListView* SourceListView,
	const FName PreviousItemId,
	const FName NewItemId)
{
	if (SourceListView == InternalOptionsList)
	{
		TemporaryOptionId = NewItemId;
	}
}

void ULunarComboBox::HandleListItemActivated(
	ULunarListView* SourceListView,
	const int32 ItemIndex,
	const FLunarListViewItemData ItemData)
{
	if (SourceListView != InternalOptionsList)
	{
		return;
	}
	TemporaryOptionId = ItemData.ItemId;
	CommitTemporarySelection();
}

void ULunarComboBox::HandleActiveScopeChanged(
	ULunarNavigationScope* PreviousScope,
	ULunarNavigationScope* NewScope)
{
	if (!bComboBoxOpen || !PopupScope)
	{
		return;
	}

	ULunarNavigationSubsystem* NavigationSubsystem =
		GetNavigationSubsystem();
	if (NavigationSubsystem
		&& NavigationSubsystem->GetNavigationScopeStack()
			.Contains(PopupScope))
	{
		return;
	}
	if (bOpeningComboBox)
	{
		bCloseRequestedWhileOpening = true;
		return;
	}
	if (bClosingComboBox || bPopupClosing)
	{
		return;
	}

	bPopupClosing = true;
	UnregisterOutsideClickProcessor();
	BeginPopupAnimation(0.0f);
}

#undef LOCTEXT_NAMESPACE
