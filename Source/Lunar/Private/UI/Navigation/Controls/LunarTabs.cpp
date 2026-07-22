// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarTabs.cpp
 * @brief Implements generated tab layout, page lifetime, composite navigation, and native presentation.
 * @ingroup LunarNavigationControls
 */

#include "UI/Navigation/Controls/LunarTabs.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/PanelWidget.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WidgetSwitcher.h"
#include "Engine/LocalPlayer.h"
#include "Subsystems/Console/LunarConsoleSubsystem.h"
#include "UI/Navigation/Controls/LunarTabHeader.h"
#include "UI/Navigation/Controls/LunarTabPage.h"
#include "UI/Navigation/Core/LunarNavigationSubsystem.h"
#include "Widgets/SWidget.h"

/** Private log channel for actionable Tabs configuration and creation failures. */
DEFINE_LOG_CATEGORY_STATIC(LogLunarTabs, Log, All);

/** Private construction and UMG hierarchy helpers for ULunarTabs. */
namespace LunarTabs_Private
{
	/** Creates an owner-associated typed UserWidget page or header. @param Tabs Owning Tabs widget. @param WidgetClass Class to instantiate. @return New widget, or null. */
	template <typename WidgetType>
	WidgetType* CreateUserWidgetForTabs(const ULunarTabs* Tabs, const TSubclassOf<WidgetType> WidgetClass)
	{
		if (!Tabs || !WidgetClass)
		{
			return nullptr;
		}
		return CreateWidget<WidgetType>(const_cast<ULunarTabs*>(Tabs), WidgetClass);
	}

	/** Resolves panel or nested UserWidget ownership for descendant checks. @param Widget Source widget. @return Logical parent, or null. */
	const UWidget* ResolveLogicalParent(const UWidget* Widget)
	{
		if (!Widget)
		{
			return nullptr;
		}
		if (const UWidget* Parent = Widget->GetParent())
		{
			return Parent;
		}
		if (const UWidgetTree* OwnerTree = Widget->GetTypedOuter<UWidgetTree>())
		{
			return Cast<UWidget>(OwnerTree->GetOuter());
		}
		return nullptr;
	}
}
ULunarTabs::ULunarTabs(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCanReceiveLunarSelection = false;
	bCanInteractWithPointer = false;
	bAllowTouchInput = false;
	bEnableInputPrompt = false;
	ActiveIndicatorBrush.DrawAs = ESlateBrushDrawType::NoDrawType;
}

void ULunarTabs::SetActiveIndicatorBrush(const FSlateBrush& NewBrush)
{
	ActiveIndicatorBrush = NewBrush;
	ApplyNativePresentation();
}

void ULunarTabs::SetActiveIndicatorTint(const FLinearColor NewTint)
{
	ActiveIndicatorTint = NewTint;
	ApplyNativePresentation();
}

void ULunarTabs::SetActiveIndicatorSize(const FVector2D NewSize)
{
	ActiveIndicatorSize = FVector2D(FMath::Max(0.0f, NewSize.X), FMath::Max(0.0f, NewSize.Y));
	ApplyNativePresentation();
}

void ULunarTabs::ConfigureActiveIndicatorPresentation(
	const FSlateBrush& NewBrush,
	const FLinearColor NewTint,
	const FVector2D NewSize)
{
	ActiveIndicatorBrush = NewBrush;
	ActiveIndicatorTint = NewTint;
	ActiveIndicatorSize = FVector2D(FMath::Max(0.0f, NewSize.X), FMath::Max(0.0f, NewSize.Y));
	ApplyNativePresentation();
}

void ULunarTabs::GetActiveIndicatorPresentation(
	FSlateBrush& OutBrush,
	FLinearColor& OutTint,
	FVector2D& OutSize) const
{
	OutBrush = ActiveIndicatorBrush;
	OutTint = ActiveIndicatorTint;
	OutSize = ActiveIndicatorSize;
}

void ULunarTabs::SetPagePresentationPadding(const FMargin NewPadding)
{
	PagePresentationPadding = NewPadding;
	ApplyNativePresentation();
}

void ULunarTabs::ConfigureTabsPresentation(
	const FSlateBrush& NewBrush,
	const FLinearColor NewTint,
	const FVector2D NewSize,
	const FMargin NewPagePadding)
{
	ActiveIndicatorBrush = NewBrush;
	ActiveIndicatorTint = NewTint;
	ActiveIndicatorSize = FVector2D(FMath::Max(0.0f, NewSize.X), FMath::Max(0.0f, NewSize.Y));
	PagePresentationPadding = NewPagePadding;
	ApplyNativePresentation();
}

void ULunarTabs::GetTabsPresentation(
	FSlateBrush& OutBrush,
	FLinearColor& OutTint,
	FVector2D& OutSize,
	FMargin& OutPagePadding) const
{
	OutBrush = ActiveIndicatorBrush;
	OutTint = ActiveIndicatorTint;
	OutSize = ActiveIndicatorSize;
	OutPagePadding = PagePresentationPadding;
}
void ULunarTabs::SetTabs(
	const TArray<FLunarTabDescriptor>& NewTabs,
	const ELunarChangeNotificationPolicy NotificationPolicy)
{
	const bool bNotify = NotificationPolicy == ELunarChangeNotificationPolicy::Notify;
	TArray<FLunarTabDescriptor> RequestedTabs = NewTabs;
	FString ValidationError;
	if (!ValidateTabDescriptors(RequestedTabs, ValidationError))
	{
		ReportTabsError(ValidationError);
		return;
	}

	EnsureNativeLayout();
	if (!bNativeLayoutInitialized)
	{
		ReportTabsError(TEXT("Tabs could not create its native layout."));
		return;
	}

	const FName PreviousActiveTabId = ActiveTabId;
	const FName RequestedActiveTabId = ChooseInitialActiveTabId(PreviousActiveTabId);

	ResetGeneratedTabs();
	TabDescriptors = MoveTemp(RequestedTabs);

	// Choose again against the copied descriptor array. The first call intentionally preserves
	// a configured ActiveTabId when SetTabs aliases the public TabDescriptors property.
	const FName InitialTabId = ChooseInitialActiveTabId(
		RequestedActiveTabId.IsNone() ? PreviousActiveTabId : RequestedActiveTabId);
	if (!BuildHeaders())
	{
		ActiveTabId = NAME_None;
		bTabsInitialized = true;
		if (bNotify && PreviousActiveTabId != ActiveTabId)
		{
			OnActiveTabChanged.Broadcast(this, PreviousActiveTabId, ActiveTabId);
		}
		return;
	}

	if (PageLifetime == ELunarTabPageLifetime::Eager)
	{
		for (const FLunarTabDescriptor& Descriptor : TabDescriptors)
		{
			CreatePageWidget(Descriptor);
		}
	}

	ActiveTabId = NAME_None;
	if (!InitialTabId.IsNone())
	{
		ActivateTabByIdInternal(InitialTabId, bNotify, false);
	}
	else
	{
		RefreshAllHeaderPresentations();
	}

	bTabsInitialized = true;
	if (bNotify && PreviousActiveTabId != ActiveTabId)
	{
		OnActiveTabChanged.Broadcast(this, PreviousActiveTabId, ActiveTabId);
	}

	if (ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem())
	{
		NavigationSubsystem->RefreshNavigationGraph();
	}
}

bool ULunarTabs::ActivateTabById(
	const FName TabId,
	const ELunarChangeNotificationPolicy NotificationPolicy)
{
	const bool bNotify = NotificationPolicy == ELunarChangeNotificationPolicy::Notify;
	if (!bTabsInitialized)
	{
		const TArray<FLunarTabDescriptor> InitialDescriptors = TabDescriptors;
		SetTabs(InitialDescriptors, ELunarChangeNotificationPolicy::Silent);
	}
	return ActivateTabByIdInternal(TabId, bNotify, bNotify);
}

bool ULunarTabs::ClearActiveTab(const ELunarChangeNotificationPolicy NotificationPolicy)
{
	if (!bTabsInitialized)
	{
		const TArray<FLunarTabDescriptor> InitialDescriptors = TabDescriptors;
		SetTabs(InitialDescriptors, ELunarChangeNotificationPolicy::Silent);
	}
	return ClearActiveTabInternal(NotificationPolicy == ELunarChangeNotificationPolicy::Notify);
}

ULunarTabPage* ULunarTabs::GetPageWidgetById(const FName TabId) const
{
	if (const TObjectPtr<ULunarTabPage>* Page = PageWidgets.Find(TabId))
	{
		return Page->Get();
	}
	return nullptr;
}

TSharedRef<SWidget> ULunarTabs::RebuildWidget()
{
	EnsureNativeLayout();
	return Super::RebuildWidget();
}

void ULunarTabs::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	EnsureNativeLayout();
#if WITH_EDITOR
	if (IsDesignTime())
	{
		const bool bHasCompletePreview = HeaderWidgets.Num() == TabDescriptors.Num()
			&& PageWidgets.Num() == TabDescriptors.Num();
		if (bDesignerPreviewSelectionChangeInProgress && bHasCompletePreview)
		{
			ApplyDesignerPreviewSelection();
		}
		else
		{
			RebuildDesignerPreview();
		}
		return;
	}
#endif
	ApplyOrientation();
	ApplyNativePresentation();
}

void ULunarTabs::NativeConstruct()
{
	Super::NativeConstruct();
	EnsureNativeLayout();
	if (!bTabsInitialized)
	{
		const TArray<FLunarTabDescriptor> InitialDescriptors = TabDescriptors;
		SetTabs(InitialDescriptors, ELunarChangeNotificationPolicy::Silent);
	}
	else
	{
		ApplyOrientation();
		RefreshAllHeaderPresentations();
		if (ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem())
		{
			NavigationSubsystem->RefreshNavigationGraph();
		}
	}
}

void ULunarTabs::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (AppliedOrientation != Orientation || AppliedPagePlacement != PagePlacement)
	{
		ApplyOrientation();
	}
	ApplyPendingHeaderNavigation();
	CaptureActivePageSelection();
}

bool ULunarTabs::ValidateTabDescriptors(
	const TArray<FLunarTabDescriptor>& Descriptors,
	FString& OutError) const
{
	TSet<FName> SeenTabIds;
	TSet<TObjectKey<UUserWidget>> SeenPageInstances;
	for (int32 Index = 0; Index < Descriptors.Num(); ++Index)
	{
		const FLunarTabDescriptor& Descriptor = Descriptors[Index];
		if (Descriptor.TabId.IsNone())
		{
			OutError = FString::Printf(TEXT("Tab descriptor %d has no TabId."), Index);
			return false;
		}
		if (SeenTabIds.Contains(Descriptor.TabId))
		{
			OutError = FString::Printf(TEXT("Duplicate TabId '%s'."), *Descriptor.TabId.ToString());
			return false;
		}
		SeenTabIds.Add(Descriptor.TabId);

		UClass* HeaderClass = Descriptor.HeaderWidgetClass.Get();
		if (!HeaderClass || !HeaderClass->IsChildOf(ULunarTabHeader::StaticClass()))
		{
			OutError = FString::Printf(
				TEXT("Tab '%s' requires a HeaderWidgetClass derived from ULunarTabHeader."),
				*Descriptor.TabId.ToString());
			return false;
		}
		if (HeaderClass->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists))
		{
			OutError = FString::Printf(
				TEXT("Tab '%s' uses a non-instantiable header class '%s'."),
				*Descriptor.TabId.ToString(),
				*HeaderClass->GetPathName());
			return false;
		}

		const bool bHasPageClass = Descriptor.PageWidgetClass != nullptr;
		const bool bHasPageInstance = IsValid(Descriptor.PageWidgetInstance);
		if (bHasPageClass == bHasPageInstance)
		{
			OutError = FString::Printf(
				TEXT("Tab '%s' must provide exactly one page source: PageWidgetClass or PageWidgetInstance."),
				*Descriptor.TabId.ToString());
			return false;
		}

		if (bHasPageClass)
		{
			UClass* PageClass = Descriptor.PageWidgetClass.Get();
			if (!PageClass || !PageClass->IsChildOf(ULunarTabPage::StaticClass())
				|| PageClass->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists))
			{
				OutError = FString::Printf(
					TEXT("Tab '%s' uses a non-instantiable Lunar Tab Page class."),
					*Descriptor.TabId.ToString());
				return false;
			}
		}
		else
		{
			UUserWidget* PageInstance = Descriptor.PageWidgetInstance;
			if (PageInstance == this || SeenPageInstances.Contains(TObjectKey<UUserWidget>(PageInstance)))
			{
				OutError = FString::Printf(
					TEXT("Tab '%s' uses a duplicate or self-referential PageWidgetInstance."),
					*Descriptor.TabId.ToString());
				return false;
			}
			SeenPageInstances.Add(TObjectKey<UUserWidget>(PageInstance));

			const ULocalPlayer* PageLocalPlayer = PageInstance->GetOwningLocalPlayer();
			if (PageLocalPlayer && GetOwningLocalPlayer() && PageLocalPlayer != GetOwningLocalPlayer())
			{
				OutError = FString::Printf(
					TEXT("Tab '%s' page instance belongs to a different local player."),
					*Descriptor.TabId.ToString());
				return false;
			}
			if (PageLifetime == ELunarTabPageLifetime::RecreateOnActivation)
			{
				OutError = FString::Printf(
					TEXT("Tab '%s' supplies a page instance, which cannot use RecreateOnActivation."),
					*Descriptor.TabId.ToString());
				return false;
			}
		}
	}

	OutError.Reset();
	return true;
}

void ULunarTabs::EnsureNativeLayout()
{
	if (bNativeLayoutInitialized || !WidgetTree)
	{
		return;
	}

	PreservedOwnerContent = WidgetTree->RootWidget;
	GeneratedRoot = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("LunarTabs_GeneratedRoot"));
	HorizontalLayout = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LunarTabs_HorizontalLayout"));
	HorizontalHeaderStrip = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("LunarTabs_HorizontalHeaders"));
	HorizontalPageHost = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LunarTabs_HorizontalPageHost"));
	VerticalLayout = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("LunarTabs_VerticalLayout"));
	VerticalHeaderStrip = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LunarTabs_VerticalHeaders"));
	VerticalPageHost = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LunarTabs_VerticalPageHost"));
	PageContainer = WidgetTree->ConstructWidget<UWidgetSwitcher>(UWidgetSwitcher::StaticClass(), TEXT("LunarTabs_Pages"));

	if (!GeneratedRoot || !HorizontalLayout || !HorizontalHeaderStrip || !HorizontalPageHost
		|| !VerticalLayout || !VerticalHeaderStrip || !VerticalPageHost || !PageContainer)
	{
		return;
	}

	WidgetTree->RootWidget = GeneratedRoot;
	GeneratedRoot->AddChildToOverlay(HorizontalLayout);
	GeneratedRoot->AddChildToOverlay(VerticalLayout);
	if (PreservedOwnerContent && PreservedOwnerContent != GeneratedRoot)
	{
		GeneratedRoot->AddChildToOverlay(PreservedOwnerContent);
	}

	bNativeLayoutInitialized = true;
	AppliedOrientation = Orientation;
	AppliedPagePlacement = PagePlacement;
	ApplyOrientation();
	ApplyNativePresentation();
}

void ULunarTabs::ApplyOrientation()
{
	if (!bNativeLayoutInitialized)
	{
		return;
	}

	HorizontalLayout->ClearChildren();
	VerticalLayout->ClearChildren();
	HorizontalHeaderStrip->ClearChildren();
	VerticalHeaderStrip->ClearChildren();
	for (const FLunarTabDescriptor& Descriptor : TabDescriptors)
	{
		UOverlay* Wrapper = HeaderWrappers.FindRef(Descriptor.TabId);
		if (!Wrapper)
		{
			continue;
		}
		if (Orientation == Orient_Horizontal)
		{
			HorizontalHeaderStrip->AddChildToHorizontalBox(Wrapper);
		}
		else
		{
			VerticalHeaderStrip->AddChildToVerticalBox(Wrapper);
		}
	}

	PageContainer->RemoveFromParent();
	const ELunarTabPagePlacement ResolvedPlacement = ResolvePagePlacement();
	UWidget* HeaderStrip = Orientation == Orient_Horizontal
		? static_cast<UWidget*>(HorizontalHeaderStrip)
		: static_cast<UWidget*>(VerticalHeaderStrip);
	if (ResolvedPlacement == ELunarTabPagePlacement::Top
		|| ResolvedPlacement == ELunarTabPagePlacement::Bottom)
	{
		HorizontalPageHost->SetContent(PageContainer);
		if (ResolvedPlacement == ELunarTabPagePlacement::Top)
		{
			if (UVerticalBoxSlot* PageSlot = HorizontalLayout->AddChildToVerticalBox(HorizontalPageHost))
			{
				PageSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			}
			if (UVerticalBoxSlot* HeaderSlot = HorizontalLayout->AddChildToVerticalBox(HeaderStrip))
			{
				HeaderSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
			}
		}
		else
		{
			if (UVerticalBoxSlot* HeaderSlot = HorizontalLayout->AddChildToVerticalBox(HeaderStrip))
			{
				HeaderSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
			}
			if (UVerticalBoxSlot* PageSlot = HorizontalLayout->AddChildToVerticalBox(HorizontalPageHost))
			{
				PageSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			}
		}
		HorizontalLayout->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		VerticalLayout->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		VerticalPageHost->SetContent(PageContainer);
		if (ResolvedPlacement == ELunarTabPagePlacement::Left)
		{
			if (UHorizontalBoxSlot* PageSlot = VerticalLayout->AddChildToHorizontalBox(VerticalPageHost))
			{
				PageSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			}
			if (UHorizontalBoxSlot* HeaderSlot = VerticalLayout->AddChildToHorizontalBox(HeaderStrip))
			{
				HeaderSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
			}
		}
		else
		{
			if (UHorizontalBoxSlot* HeaderSlot = VerticalLayout->AddChildToHorizontalBox(HeaderStrip))
			{
				HeaderSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
			}
			if (UHorizontalBoxSlot* PageSlot = VerticalLayout->AddChildToHorizontalBox(VerticalPageHost))
			{
				PageSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			}
		}
		HorizontalLayout->SetVisibility(ESlateVisibility::Collapsed);
		VerticalLayout->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	AppliedOrientation = Orientation;
	AppliedPagePlacement = PagePlacement;
	InvalidateLayoutAndVolatility();
	if (ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem())
	{
		NavigationSubsystem->RefreshNavigationGraph();
	}
}

ELunarTabPagePlacement ULunarTabs::ResolvePagePlacement() const
{
	if (PagePlacement != ELunarTabPagePlacement::Automatic)
	{
		return PagePlacement;
	}
	return Orientation == Orient_Horizontal
		? ELunarTabPagePlacement::Bottom
		: ELunarTabPagePlacement::Right;
}

void ULunarTabs::ResetGeneratedTabs()
{
	for (const TPair<FName, TObjectPtr<ULunarTabPage>>& Pair : PageWidgets)
	{
		if (Pair.Value)
		{
			Pair.Value->RemoveFromParent();
		}
	}
	for (const TPair<FName, TObjectPtr<UOverlay>>& Pair : HeaderWrappers)
	{
		if (Pair.Value)
		{
			Pair.Value->RemoveFromParent();
		}
	}

	if (HorizontalHeaderStrip)
	{
		HorizontalHeaderStrip->ClearChildren();
	}
	if (VerticalHeaderStrip)
	{
		VerticalHeaderStrip->ClearChildren();
	}
	if (PageContainer)
	{
		PageContainer->ClearChildren();
	}

	HeaderWidgets.Reset();
	HeaderWrappers.Reset();
	HeaderIndicators.Reset();
	PageWidgets.Reset();
	LastPageDescendantNavigationIds.Reset();
	PendingHeaderNavigationSource.Reset();
	PendingHeaderNavigationTarget.Reset();
	ActiveTabId = NAME_None;
}

bool ULunarTabs::BuildHeaders()
{
	for (const FLunarTabDescriptor& Descriptor : TabDescriptors)
	{
		ULunarTabHeader* Header = CreateHeaderWidget(Descriptor);
		if (!Header)
		{
			ReportTabsError(FString::Printf(
				TEXT("Failed to create header for tab '%s'."),
				*Descriptor.TabId.ToString()));
			ResetGeneratedTabs();
			return false;
		}

		UOverlay* Wrapper = WidgetTree->ConstructWidget<UOverlay>();
		UImage* Indicator = WidgetTree->ConstructWidget<UImage>();
		if (!Wrapper || !Indicator)
		{
			ReportTabsError(FString::Printf(
				TEXT("Failed to create presentation widgets for tab '%s'."),
				*Descriptor.TabId.ToString()));
			ResetGeneratedTabs();
			return false;
		}

		Indicator->SetVisibility(ESlateVisibility::Collapsed);
#if WITH_ACCESSIBILITY
		Indicator->TakeWidget()->SetAccessibleBehavior(EAccessibleBehavior::NotAccessible);
#endif
		Wrapper->AddChildToOverlay(Header);
		Wrapper->AddChildToOverlay(Indicator);
		HeaderWidgets.Add(Descriptor.TabId, Header);
		HeaderWrappers.Add(Descriptor.TabId, Wrapper);
		HeaderIndicators.Add(Descriptor.TabId, Indicator);
	}

	ApplyOrientation();
	RefreshAllHeaderPresentations();
	return true;
}

ULunarTabHeader* ULunarTabs::CreateHeaderWidget(const FLunarTabDescriptor& Descriptor)
{
	ULunarTabHeader* Header = LunarTabs_Private::CreateUserWidgetForTabs(this, Descriptor.HeaderWidgetClass);
	if (Header)
	{
		Header->InitializeTabHeader(this, Descriptor.TabId, Descriptor.bEnabled, Descriptor.DisabledReason);
	}
	return Header;
}

ULunarTabPage* ULunarTabs::CreatePageWidget(const FLunarTabDescriptor& Descriptor)
{
	if (TObjectPtr<ULunarTabPage>* ExistingPage = PageWidgets.Find(Descriptor.TabId))
	{
		return IsValid(ExistingPage->Get()) ? ExistingPage->Get() : nullptr;
	}

	ULunarTabPage* Page = Descriptor.PageWidgetInstance;
	if (Page)
	{
		if (Page->GetParent() && Page->GetParent() != PageContainer)
		{
			Page->RemoveFromParent();
		}
	}
	else
	{
		Page = LunarTabs_Private::CreateUserWidgetForTabs(this, Descriptor.PageWidgetClass);
	}

	if (!Page)
	{
		ReportTabsError(FString::Printf(
			TEXT("Failed to create page for tab '%s'."),
			*Descriptor.TabId.ToString()));
		return nullptr;
	}

	Page->AssignTabContext(this, Descriptor.TabId);
	Page->SetVisibility(ESlateVisibility::Collapsed);
	if (!PageContainer || !PageContainer->AddChild(Page))
	{
		ReportTabsError(FString::Printf(
			TEXT("Failed to attach page for tab '%s'."),
			*Descriptor.TabId.ToString()));
		Page->RemoveFromParent();
		return nullptr;
	}

	PageWidgets.Add(Descriptor.TabId, Page);
	return Page;
}

void ULunarTabs::DestroyRecreatedPage(const FName TabId)
{
	if (PageLifetime != ELunarTabPageLifetime::RecreateOnActivation || TabId == ActiveTabId)
	{
		return;
	}
	if (TObjectPtr<ULunarTabPage>* Page = PageWidgets.Find(TabId))
	{
		if (Page->Get())
		{
			Page->Get()->RemoveFromParent();
		}
		PageWidgets.Remove(TabId);
	}
}

bool ULunarTabs::ActivateTabByIdInternal(
	const FName TabId,
	const bool bNotifyRequestedHeader,
	const bool bBroadcastChange)
{
	ULunarTabHeader* RequestedHeader = HeaderWidgets.FindRef(TabId);
	auto RejectRequestedHeader = [&]()
	{
		if (bNotifyRequestedHeader && RequestedHeader)
		{
			RequestedHeader->NotifyOwnedTabRejected();
		}
	};

	if (bActivationInProgress || TabId.IsNone())
	{
		RejectRequestedHeader();
		return false;
	}

	const FLunarTabDescriptor* Descriptor = FindTabDescriptor(TabId);
	if (!Descriptor || !Descriptor->bEnabled || !RequestedHeader)
	{
		RejectRequestedHeader();
		return false;
	}

	if (TabId == ActiveTabId)
	{
		if (ULunarTabPage* ExistingPage = GetPageWidgetById(TabId))
		{
			ExistingPage->SetVisibility(ESlateVisibility::Visible);
			PageContainer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			PageContainer->SetActiveWidget(ExistingPage);
			return true;
		}
	}

	bActivationInProgress = true;
	const FName PreviousTabId = ActiveTabId;
	ULunarTabPage* PreviousPage = GetPageWidgetById(PreviousTabId);
	ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem();

	if (NavigationSubsystem && PreviousPage)
	{
		ULunarNavigableWidget* Selection = NavigationSubsystem->GetSelectedWidget();
		if (Selection && IsWidgetInsidePage(Selection, PreviousPage))
		{
			if (!Selection->GetNavigationId().IsNone())
			{
				LastPageDescendantNavigationIds.Add(PreviousTabId, Selection->GetNavigationId());
			}
			// Programmatic switches deliberately commit delegated text focus by moving
			// Lunar Selection to the destination header before the page changes.
			if (!NavigationSubsystem->SetSelectedWidget(RequestedHeader))
			{
				bActivationInProgress = false;
				ReportTabsError(FString::Printf(
					TEXT("Could not transfer Selection to the destination header '%s'."),
					*TabId.ToString()));
				RejectRequestedHeader();
				return false;
			}
		}
	}

	ULunarTabPage* RequestedPage = CreatePageWidget(*Descriptor);
	if (!RequestedPage)
	{
		bActivationInProgress = false;
		// PreviousPage has not been hidden: page creation failure is atomic.
		RejectRequestedHeader();
		return false;
	}

	// The page becomes eligible before graph rebuild, restoration, or fallback calculation.
	RequestedPage->SetVisibility(ESlateVisibility::Visible);
	PageContainer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	PageContainer->SetActiveWidget(RequestedPage);
	if (PreviousPage && PreviousPage != RequestedPage)
	{
		PreviousPage->SetVisibility(ESlateVisibility::Collapsed);
	}

	ActiveTabId = TabId;
	for (const TPair<FName, TObjectPtr<ULunarTabHeader>>& Pair : HeaderWidgets)
	{
		if (Pair.Value)
		{
			Pair.Value->SetActiveTabHeader(Pair.Key == ActiveTabId);
		}
	}
	ApplyNativePresentation();

	if (NavigationSubsystem)
	{
		NavigationSubsystem->RefreshNavigationGraph();
	}
	if (!PreviousTabId.IsNone() && PreviousTabId != ActiveTabId)
	{
		DestroyRecreatedPage(PreviousTabId);
		PageContainer->SetActiveWidget(RequestedPage);
	}

	bActivationInProgress = false;
	if (bBroadcastChange && PreviousTabId != ActiveTabId)
	{
		OnActiveTabChanged.Broadcast(this, PreviousTabId, ActiveTabId);
	}
	return true;
}

bool ULunarTabs::ClearActiveTabInternal(const bool bBroadcastChange)
{
	if (!bAllowNoActiveTab || bActivationInProgress)
	{
		return false;
	}
	if (ActiveTabId.IsNone())
	{
		return true;
	}

	bActivationInProgress = true;
	const FName PreviousTabId = ActiveTabId;
	ULunarTabPage* PreviousPage = GetPageWidgetById(PreviousTabId);
	ULunarTabHeader* PreviousHeader = HeaderWidgets.FindRef(PreviousTabId);
	ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem();
	if (NavigationSubsystem && PreviousPage)
	{
		ULunarNavigableWidget* Selection = NavigationSubsystem->GetSelectedWidget();
		if (Selection && IsWidgetInsidePage(Selection, PreviousPage))
		{
			if (!Selection->GetNavigationId().IsNone())
			{
				LastPageDescendantNavigationIds.Add(PreviousTabId, Selection->GetNavigationId());
			}
			if (!PreviousHeader || !NavigationSubsystem->SetSelectedWidget(PreviousHeader))
			{
				bActivationInProgress = false;
				ReportTabsError(FString::Printf(
					TEXT("Could not transfer Selection to header '%s' before clearing the active tab."),
					*PreviousTabId.ToString()));
				return false;
			}
		}
	}

	if (PreviousPage)
	{
		PreviousPage->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (PageContainer)
	{
		PageContainer->SetVisibility(ESlateVisibility::Collapsed);
	}
	ActiveTabId = NAME_None;
	for (const TPair<FName, TObjectPtr<ULunarTabHeader>>& Pair : HeaderWidgets)
	{
		if (Pair.Value)
		{
			Pair.Value->SetActiveTabHeader(false);
		}
	}
	ApplyNativePresentation();
	if (NavigationSubsystem)
	{
		NavigationSubsystem->RefreshNavigationGraph();
	}
	DestroyRecreatedPage(PreviousTabId);
	bActivationInProgress = false;
	if (bBroadcastChange)
	{
		OnActiveTabChanged.Broadcast(this, PreviousTabId, NAME_None);
	}
	return true;
}

bool ULunarTabs::TryActivateTabFromHeader(ULunarTabHeader* Header)
{
	return Header
		&& Header->TabsOwner == this
		&& HeaderWidgets.FindRef(Header->TabId) == Header
		&& ActivateTabByIdInternal(Header->TabId, false, true);
}

bool ULunarTabs::IsTabEnabled(const FName TabId) const
{
	const FLunarTabDescriptor* Descriptor = FindTabDescriptor(TabId);
	return Descriptor && Descriptor->bEnabled;
}

const FLunarTabDescriptor* ULunarTabs::FindTabDescriptor(const FName TabId) const
{
	return TabDescriptors.FindByPredicate([TabId](const FLunarTabDescriptor& Descriptor)
	{
		return Descriptor.TabId == TabId;
	});
}

FName ULunarTabs::ChooseInitialActiveTabId(const FName RequestedTabId) const
{
	if (RequestedTabId.IsNone() && bAllowNoActiveTab)
	{
		return NAME_None;
	}
	if (IsTabEnabled(RequestedTabId))
	{
		return RequestedTabId;
	}
	for (const FLunarTabDescriptor& Descriptor : TabDescriptors)
	{
		if (Descriptor.bEnabled)
		{
			return Descriptor.TabId;
		}
	}
	return NAME_None;
}

ULunarNavigationSubsystem* ULunarTabs::ResolveNavigationSubsystem() const
{
	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		return LocalPlayer->GetSubsystem<ULunarNavigationSubsystem>();
	}
	return nullptr;
}

bool ULunarTabs::CanRouteHeaderDirection(
	const ULunarTabHeader* CurrentHeader,
	const ELunarNavigationDirection Direction) const
{
	return ResolveNavigationSubsystem()
		&& ResolveHeaderDirectionTarget(CurrentHeader, Direction) != nullptr;
}

bool ULunarTabs::QueueHeaderDirection(
	ULunarTabHeader* CurrentHeader,
	const ELunarNavigationDirection Direction)
{
	if (!CurrentHeader
		|| CurrentHeader->GetNavigationLink(Direction).Mode != ELunarNavigationLinkMode::Automatic
		|| !ResolveNavigationSubsystem())
	{
		return false;
	}

	ULunarNavigableWidget* Target = ResolveHeaderDirectionTarget(CurrentHeader, Direction);
	if (!Target)
	{
		return false;
	}

	// Selection is applied on the owner's next tick. This lets the source header's
	// action wrapper establish its pressed state before unselection clears it.
	PendingHeaderNavigationSource = CurrentHeader;
	PendingHeaderNavigationTarget = Target;
	return true;
}

ULunarNavigableWidget* ULunarTabs::ResolveHeaderDirectionTarget(
	const ULunarTabHeader* CurrentHeader,
	const ELunarNavigationDirection Direction) const
{
	if (!CurrentHeader
		|| CurrentHeader->TabsOwner != this
		|| HeaderWidgets.FindRef(CurrentHeader->TabId) != CurrentHeader
		|| CurrentHeader->GetNavigationLink(Direction).Mode != ELunarNavigationLinkMode::Automatic)
	{
		return nullptr;
	}

	const bool bAlongStrip = Orientation == Orient_Horizontal
		? Direction == ELunarNavigationDirection::Left || Direction == ELunarNavigationDirection::Right
		: Direction == ELunarNavigationDirection::Up || Direction == ELunarNavigationDirection::Down;
	const ELunarTabPagePlacement ResolvedPlacement = ResolvePagePlacement();
	const bool bTowardPage =
		(ResolvedPlacement == ELunarTabPagePlacement::Top && Direction == ELunarNavigationDirection::Up)
		|| (ResolvedPlacement == ELunarTabPagePlacement::Bottom && Direction == ELunarNavigationDirection::Down)
		|| (ResolvedPlacement == ELunarTabPagePlacement::Left && Direction == ELunarNavigationDirection::Left)
		|| (ResolvedPlacement == ELunarTabPagePlacement::Right && Direction == ELunarNavigationDirection::Right);
	if (bAlongStrip)
	{
		if (ULunarTabHeader* AdjacentHeader = FindAdjacentHeader(CurrentHeader, Direction))
		{
			return AdjacentHeader;
		}
		return bTowardPage ? ResolveActivePageEntryTarget() : nullptr;
	}
	return bTowardPage ? ResolveActivePageEntryTarget() : nullptr;
}

ULunarTabHeader* ULunarTabs::FindAdjacentHeader(
	const ULunarTabHeader* CurrentHeader,
	const ELunarNavigationDirection Direction) const
{
	const int32 CurrentIndex = CurrentHeader
		? TabDescriptors.IndexOfByPredicate([CurrentHeader](const FLunarTabDescriptor& Descriptor)
		{
			return Descriptor.TabId == CurrentHeader->TabId;
		})
		: INDEX_NONE;
	if (CurrentIndex == INDEX_NONE)
	{
		return nullptr;
	}

	const bool bForward = Direction == ELunarNavigationDirection::Right
		|| Direction == ELunarNavigationDirection::Down;
	const int32 Step = bForward ? 1 : -1;
	for (int32 Index = CurrentIndex + Step; TabDescriptors.IsValidIndex(Index); Index += Step)
	{
		const FLunarTabDescriptor& CandidateDescriptor = TabDescriptors[Index];
		ULunarTabHeader* Candidate = HeaderWidgets.FindRef(CandidateDescriptor.TabId);
		if (CandidateDescriptor.bEnabled && Candidate && Candidate->CanReceiveLunarSelection())
		{
			return Candidate;
		}
	}
	return nullptr;
}

ULunarNavigableWidget* ULunarTabs::ResolveActivePageEntryTarget() const
{
	UUserWidget* ActivePage = GetPageWidgetById(ActiveTabId);
	if (!ActivePage || ActivePage->GetVisibility() == ESlateVisibility::Collapsed)
	{
		return nullptr;
	}

	ULunarNavigableWidget* Target = nullptr;
	if (const FName* RestoredNavigationId = LastPageDescendantNavigationIds.Find(ActiveTabId))
	{
		Target = FindPageDescendantByNavigationId(ActivePage, *RestoredNavigationId);
		if (Target && !Target->CanReceiveLunarSelection())
		{
			Target = nullptr;
		}
	}
	return Target ? Target : FindFirstEligiblePageDescendant(ActivePage);
}

void ULunarTabs::ApplyPendingHeaderNavigation()
{
	ULunarTabHeader* Source = PendingHeaderNavigationSource.Get();
	ULunarNavigableWidget* Target = PendingHeaderNavigationTarget.Get();
	PendingHeaderNavigationSource.Reset();
	PendingHeaderNavigationTarget.Reset();

	ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem();
	if (!Source || !Target || !NavigationSubsystem
		|| NavigationSubsystem->GetSelectedWidget() != Source
		|| !Target->CanReceiveLunarSelection())
	{
		return;
	}
	NavigationSubsystem->SetSelectedWidget(Target);
}

void ULunarTabs::CaptureActivePageSelection()
{
	UUserWidget* ActivePage = GetPageWidgetById(ActiveTabId);
	ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem();
	ULunarNavigableWidget* Selection = NavigationSubsystem ? NavigationSubsystem->GetSelectedWidget() : nullptr;
	if (ActivePage && Selection && IsWidgetInsidePage(Selection, ActivePage)
		&& !Selection->GetNavigationId().IsNone())
	{
		LastPageDescendantNavigationIds.Add(ActiveTabId, Selection->GetNavigationId());
	}
}

ULunarNavigableWidget* ULunarTabs::FindPageDescendantByNavigationId(
	UUserWidget* Page,
	const FName InNavigationId) const
{
	if (!Page || InNavigationId.IsNone())
	{
		return nullptr;
	}
	TArray<ULunarNavigableWidget*> Descendants;
	GatherPageNavigableDescendants(Page, Descendants);
	for (ULunarNavigableWidget* Descendant : Descendants)
	{
		if (Descendant && Descendant->GetNavigationId() == InNavigationId)
		{
			return Descendant;
		}
	}
	return nullptr;
}

ULunarNavigableWidget* ULunarTabs::FindFirstEligiblePageDescendant(UUserWidget* Page) const
{
	TArray<ULunarNavigableWidget*> Descendants;
	GatherPageNavigableDescendants(Page, Descendants);
	for (ULunarNavigableWidget* Descendant : Descendants)
	{
		if (Descendant && Descendant->CanReceiveLunarSelection())
		{
			return Descendant;
		}
	}
	return nullptr;
}

void ULunarTabs::GatherPageNavigableDescendants(
	UUserWidget* Page,
	TArray<ULunarNavigableWidget*>& OutWidgets) const
{
	OutWidgets.Reset();
	if (!Page)
	{
		return;
	}

	TArray<UWidget*> PendingWidgets;
	TSet<TObjectKey<UWidget>> VisitedWidgets;
	PendingWidgets.Add(Page);
	for (int32 PendingIndex = 0; PendingIndex < PendingWidgets.Num(); ++PendingIndex)
	{
		UWidget* Widget = PendingWidgets[PendingIndex];
		if (!IsValid(Widget) || VisitedWidgets.Contains(TObjectKey<UWidget>(Widget)))
		{
			continue;
		}
		VisitedWidgets.Add(TObjectKey<UWidget>(Widget));

		if (ULunarNavigableWidget* NavigableWidget = Cast<ULunarNavigableWidget>(Widget))
		{
			OutWidgets.Add(NavigableWidget);
		}
		if (UUserWidget* UserWidget = Cast<UUserWidget>(Widget))
		{
			if (UserWidget->WidgetTree)
			{
				TArray<UWidget*> TreeWidgets;
				UserWidget->WidgetTree->GetAllWidgets(TreeWidgets);
				PendingWidgets.Append(TreeWidgets);
			}
		}
		if (UPanelWidget* PanelWidget = Cast<UPanelWidget>(Widget))
		{
			for (int32 ChildIndex = 0; ChildIndex < PanelWidget->GetChildrenCount(); ++ChildIndex)
			{
				PendingWidgets.Add(PanelWidget->GetChildAt(ChildIndex));
			}
		}
	}
}

bool ULunarTabs::IsWidgetInsidePage(const UWidget* Widget, const UUserWidget* Page) const
{
	if (!Widget || !Page)
	{
		return false;
	}
	const UWidget* Current = Widget;
	TSet<TObjectKey<UWidget>> Visited;
	while (Current && !Visited.Contains(TObjectKey<UWidget>(const_cast<UWidget*>(Current))))
	{
		if (Current == Page)
		{
			return true;
		}
		Visited.Add(TObjectKey<UWidget>(const_cast<UWidget*>(Current)));
		Current = LunarTabs_Private::ResolveLogicalParent(Current);
	}
	return false;
}
void ULunarTabs::ApplyNativePresentation()
{
	for (const TPair<FName, TObjectPtr<UImage>>& Pair : HeaderIndicators)
	{
		UImage* Indicator = Pair.Value;
		const ULunarTabHeader* Header = HeaderWidgets.FindRef(Pair.Key);
		if (!Indicator)
		{
			continue;
		}
		Indicator->SetBrush(ActiveIndicatorBrush);
		Indicator->SetColorAndOpacity(ActiveIndicatorTint);
		Indicator->SetDesiredSizeOverride(ActiveIndicatorSize);
		Indicator->SetVisibility(Header && Header->bActiveTabHeader
			? ESlateVisibility::HitTestInvisible
			: ESlateVisibility::Collapsed);
	}
	if (HorizontalPageHost)
	{
		HorizontalPageHost->SetPadding(PagePresentationPadding);
	}
	if (VerticalPageHost)
	{
		VerticalPageHost->SetPadding(PagePresentationPadding);
	}
}
void ULunarTabs::RefreshAllHeaderPresentations()
{
	for (const TPair<FName, TObjectPtr<ULunarTabHeader>>& Pair : HeaderWidgets)
	{
		if (Pair.Value)
		{
			Pair.Value->RefreshVisualState();
		}
	}
	ApplyNativePresentation();
}

TArray<FString> ULunarTabs::GetDesignerPreviewTabIdOptions() const
{
	TArray<FString> Options;
	Options.Reserve(TabDescriptors.Num());
	for (const FLunarTabDescriptor& Descriptor : TabDescriptors)
	{
		if (!Descriptor.TabId.IsNone())
		{
			Options.Add(Descriptor.TabId.ToString());
		}
	}
	return Options;
}

#if WITH_EDITOR
void ULunarTabs::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName ChangedPropertyName = PropertyChangedEvent.GetPropertyName();
	const FName MemberPropertyName = PropertyChangedEvent.MemberProperty
		? PropertyChangedEvent.MemberProperty->GetFName()
		: NAME_None;
	const bool bPreviewSelectionChanged =
		ChangedPropertyName == GET_MEMBER_NAME_CHECKED(ULunarTabs, DesignerPreviewTabId)
		|| MemberPropertyName == GET_MEMBER_NAME_CHECKED(ULunarTabs, DesignerPreviewTabId)
		|| ChangedPropertyName == GET_MEMBER_NAME_CHECKED(ULunarTabs, bDesignerPreviewNoActiveTab)
		|| MemberPropertyName == GET_MEMBER_NAME_CHECKED(ULunarTabs, bDesignerPreviewNoActiveTab);
	TGuardValue<bool> SelectionChangeGuard(
		bDesignerPreviewSelectionChangeInProgress,
		bPreviewSelectionChanged);
	const bool bHadCachedSlateWidget = GetCachedWidget().IsValid();
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (!IsDesignTime())
	{
		return;
	}

	// UWidget::PostEditChangeProperty already calls SynchronizeProperties when a live Slate widget
	// exists. Designer templates have no cached Slate widget, so synchronize only that path here.
	if (!bHadCachedSlateWidget)
	{
		SynchronizeProperties();
	}
	if (PageContainer)
	{
		PageContainer->InvalidateLayoutAndVolatility();
	}
	InvalidateLayoutAndVolatility();
	ForceLayoutPrepass();
}

void ULunarTabs::RebuildDesignerPreview()
{
	if (bDesignerPreviewRebuildInProgress || !IsDesignTime() || !bNativeLayoutInitialized)
	{
		return;
	}

	TGuardValue<bool> RebuildGuard(bDesignerPreviewRebuildInProgress, true);
	FString ValidationError;
	if (!ValidateTabDescriptors(TabDescriptors, ValidationError))
	{
		ResetGeneratedTabs();
		ApplyOrientation();
		ApplyNativePresentation();
		return;
	}

	const FName AuthoredActiveTabId = ActiveTabId;
	const bool bWasTabsInitialized = bTabsInitialized;
	const FName PreviewTabId = ResolveDesignerPreviewTabId();
	ActiveTabId = PreviewTabId;
	const TArray<FLunarTabDescriptor> PreviewDescriptors = TabDescriptors;
	SetTabs(PreviewDescriptors, ELunarChangeNotificationPolicy::Silent);
	for (const FLunarTabDescriptor& Descriptor : TabDescriptors)
	{
		if (!CreatePageWidget(Descriptor))
		{
			ResetGeneratedTabs();
			ApplyOrientation();
			ApplyNativePresentation();
			ActiveTabId = AuthoredActiveTabId;
			bTabsInitialized = bWasTabsInitialized;
			return;
		}
	}
	ActiveTabId = AuthoredActiveTabId;
	bTabsInitialized = bWasTabsInitialized;
	ApplyDesignerPreviewSelection();
}

void ULunarTabs::ApplyDesignerPreviewSelection()
{
	const FName PreviewTabId = ResolveDesignerPreviewTabId();
	ULunarTabPage* PreviewPage = nullptr;
	for (const TPair<FName, TObjectPtr<ULunarTabPage>>& Pair : PageWidgets)
	{
		if (Pair.Value)
		{
			if (Pair.Key == PreviewTabId)
			{
				PreviewPage = Pair.Value;
			}
			Pair.Value->SetVisibility(Pair.Key == PreviewTabId
				? ESlateVisibility::Visible
				: ESlateVisibility::Collapsed);
		}
	}
	for (const TPair<FName, TObjectPtr<ULunarTabHeader>>& Pair : HeaderWidgets)
	{
		if (Pair.Value)
		{
			Pair.Value->SetActiveTabHeader(Pair.Key == PreviewTabId);
		}
	}
	RefreshAllHeaderPresentations();
	if (PageContainer)
	{
		PageContainer->SetVisibility(PreviewPage
			? ESlateVisibility::SelfHitTestInvisible
			: ESlateVisibility::Collapsed);
		if (PreviewPage)
		{
			PageContainer->SetActiveWidget(PreviewPage);
		}
		PageContainer->InvalidateLayoutAndVolatility();
	}
}

FName ULunarTabs::ResolveDesignerPreviewTabId() const
{
	if (bDesignerPreviewNoActiveTab && bAllowNoActiveTab)
	{
		return NAME_None;
	}
	const FName RequestedTabId = DesignerPreviewTabId.IsNone()
		? ActiveTabId
		: DesignerPreviewTabId;
	return ChooseInitialActiveTabId(RequestedTabId);
}
#endif

void ULunarTabs::ReportTabsError(const FString& Message) const
{
	ULunarConsoleSubsystem::AddMessage(
		FGameplayTag::RequestGameplayTag(TEXT("Lunar.Console"), false),
		ELunarConsoleMessageVerbosity::Error,
		Message);
	UE_LOG(LogLunarTabs, Error, TEXT("%s (%s)"), *Message, *GetPathName());
}
