// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarTabs.cpp
 * @brief Implements generated tab layout, page lifetime, composite navigation, and style transitions.
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
#include "Engine/LocalPlayer.h"
#include "Kismet/KismetMathLibrary.h"
#include "Styling/StyleDefaults.h"
#include "Subsystems/Console/LunarConsoleSubsystem.h"
#include "UI/Navigation/Controls/LunarTabHeader.h"
#include "UI/Navigation/Core/LunarNavigationSubsystem.h"
#include "UI/Navigation/Styles/LunarStyleResolver.h"
#include "UObject/GCObject.h"
#include "Widgets/SWidget.h"

/** Private log channel for actionable Tabs configuration and creation failures. */
DEFINE_LOG_CATEGORY_STATIC(LogLunarTabs, Log, All);

/**
 * Non-UObject transition storage that explicitly reports Slate-brush UObject resources to GC.
 * @ingroup LunarNavigationControls
 */
struct FLunarTabsRuntimeState final : FGCObject
{
	/** Mutable state for one independently animated Tabs style channel. */
	struct FTransitionState
	{
		/** Style currently visible to the user. */
		FLunarTabsStylePatch DisplayedStyle;
		/** Style captured when the active transition began. */
		FLunarTabsStylePatch TransitionSourceStyle;
		/** Materialized style reached by the active transition. */
		FLunarTabsStylePatch TransitionTargetStyle;
		/** Latest logical target, retained separately for reversal decisions. */
		FLunarTabsStylePatch LogicalTargetStyle;
		/** Seconds elapsed in the active transition. */
		float TransitionElapsed = 0.0f;
		/** Total duration of the active transition in seconds. */
		float TransitionDuration = 0.0f;
		/** Whether DisplayedStyle has been initialized. */
		bool bHasDisplayedStyle = false;
		/** Whether interpolation is currently active. */
		bool bTransitionActive = false;
		/** Whether the active transition reversed toward a previous target. */
		bool bTransitionReversing = false;
	};

	/** Per-tab active-indicator transition channels. */
	TMap<FName, FTransitionState> HeaderIndicatorTransitions;
	/** Shared transition channel for the active page host padding. */
	FTransitionState PagePaddingTransition;

	/** Reports brush resource objects retained by transition snapshots. @param Collector Active GC reference collector. */
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override
	{
		auto AddStyleBrush = [&Collector](FLunarTabsStylePatch& Style)
		{
			TObjectPtr<UObject> ResourceObject = Style.ActiveIndicatorBrush.GetResourceObject();
			if (ResourceObject)
			{
				Collector.AddReferencedObject(ResourceObject);
				Style.ActiveIndicatorBrush.SetResourceObject(ResourceObject.Get());
			}
		};
		auto AddStateBrushes = [&AddStyleBrush](FTransitionState& State)
		{
			AddStyleBrush(State.DisplayedStyle);
			AddStyleBrush(State.TransitionSourceStyle);
			AddStyleBrush(State.TransitionTargetStyle);
			AddStyleBrush(State.LogicalTargetStyle);
		};

		for (TPair<FName, FTransitionState>& Pair : HeaderIndicatorTransitions)
		{
			AddStateBrushes(Pair.Value);
		}
		AddStateBrushes(PagePaddingTransition);
	}

	/** Returns a stable diagnostic name for Unreal's GC reference graph. @return Referencer name. */
	virtual FString GetReferencerName() const override
	{
		return TEXT("FLunarTabsRuntimeState");
	}
};

/** Private style, construction, and UMG hierarchy helpers for ULunarTabs. */
namespace LunarTabs_Private
{
	/** Compares transition settings used by specialized style channels. @param A First settings. @param B Second settings. @return True when equivalent. */
	bool AreTransitionsEquivalent(
		const FLunarStyleTransitionSettings& A,
		const FLunarStyleTransitionSettings& B)
	{
		return A.bEnabled == B.bEnabled
			&& A.Duration == B.Duration
			&& A.Easing == B.Easing;
	}

	/** Materializes indicator defaults so interpolation never depends on sparse fields. @param Style Sparse style. @return Complete indicator style. */
	FLunarTabsStylePatch MaterializeIndicatorStyle(const FLunarTabsStylePatch& Style)
	{
		FLunarTabsStylePatch Result;
		Result.bOverrideActiveIndicatorBrush = true;
		Result.ActiveIndicatorBrush = Style.bOverrideActiveIndicatorBrush
			? Style.ActiveIndicatorBrush
			: *FStyleDefaults::GetNoBrush();
		Result.bOverrideActiveIndicatorTint = true;
		Result.ActiveIndicatorTint = Style.bOverrideActiveIndicatorTint
			? Style.ActiveIndicatorTint
			: FLinearColor::White;
		Result.Common.Transition = Style.Common.Transition;
		return Result;
	}

	/** Materializes page-padding defaults so interpolation never depends on sparse fields. @param Style Sparse style. @return Complete page-padding style. */
	FLunarTabsStylePatch MaterializePagePaddingStyle(const FLunarTabsStylePatch& Style)
	{
		FLunarTabsStylePatch Result;
		Result.bOverridePagePadding = true;
		Result.PagePadding = Style.bOverridePagePadding
			? Style.PagePadding
			: FMargin(0.0f);
		Result.Common.Transition = Style.Common.Transition;
		return Result;
	}

	/** Compares complete indicator visuals and transition settings. @param A First style. @param B Second style. @return True when equivalent. */
	bool AreIndicatorStylesEquivalent(
		const FLunarTabsStylePatch& A,
		const FLunarTabsStylePatch& B)
	{
		return A.ActiveIndicatorBrush == B.ActiveIndicatorBrush
			&& A.ActiveIndicatorTint == B.ActiveIndicatorTint
			&& AreTransitionsEquivalent(A.Common.Transition, B.Common.Transition);
	}

	/** Compares only currently visible indicator fields. @param A First style. @param B Second style. @return True when visually equivalent. */
	bool AreIndicatorVisualsEquivalent(
		const FLunarTabsStylePatch& A,
		const FLunarTabsStylePatch& B)
	{
		return A.ActiveIndicatorBrush == B.ActiveIndicatorBrush
			&& A.ActiveIndicatorTint == B.ActiveIndicatorTint;
	}

	/** Compares page padding and its transition settings. @param A First style. @param B Second style. @return True when equivalent. */
	bool ArePagePaddingStylesEquivalent(
		const FLunarTabsStylePatch& A,
		const FLunarTabsStylePatch& B)
	{
		return A.PagePadding == B.PagePadding
			&& AreTransitionsEquivalent(A.Common.Transition, B.Common.Transition);
	}

	/** Interpolates all four margin components. @param Source Starting margin. @param Target Target margin. @param Alpha Normalized blend alpha. @return Interpolated margin. */
	FMargin InterpolateMargin(const FMargin& Source, const FMargin& Target, const float Alpha)
	{
		return FMargin(
			FMath::Lerp(Source.Left, Target.Left, Alpha),
			FMath::Lerp(Source.Top, Target.Top, Alpha),
			FMath::Lerp(Source.Right, Target.Right, Alpha),
			FMath::Lerp(Source.Bottom, Target.Bottom, Alpha));
	}

	/** Creates an owner-associated UserWidget page or header. @param Tabs Owning Tabs widget. @param WidgetClass Class to instantiate. @return New widget, or null. */
	UUserWidget* CreateUserWidgetForTabs(const ULunarTabs* Tabs, const TSubclassOf<UUserWidget> WidgetClass)
	{
		if (!Tabs || !WidgetClass)
		{
			return nullptr;
		}
		return CreateWidget<UUserWidget>(const_cast<ULunarTabs*>(Tabs), WidgetClass);
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
	, RuntimeState(new FLunarTabsRuntimeState())
{
	bCanReceiveLunarSelection = false;
	bCanInteractWithPointer = false;
	bEnableInputPrompt = false;
}

ULunarTabs::~ULunarTabs()
{
	delete RuntimeState;
	RuntimeState = nullptr;
}

void ULunarTabs::SetTabs(const TArray<FLunarTabDescriptor>& NewTabs)
{
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
		if (PreviousActiveTabId != ActiveTabId)
		{
			OnActiveTabChanged.Broadcast(PreviousActiveTabId, ActiveTabId);
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
		ActivateTabByIdInternal(InitialTabId, true, false);
	}
	else
	{
		RefreshAllHeaderPresentations();
	}

	bTabsInitialized = true;
	if (PreviousActiveTabId != ActiveTabId)
	{
		OnActiveTabChanged.Broadcast(PreviousActiveTabId, ActiveTabId);
	}

	if (ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem())
	{
		NavigationSubsystem->RefreshNavigationGraph();
	}
}

bool ULunarTabs::ActivateTabById(const FName TabId)
{
	if (!bTabsInitialized)
	{
		const TArray<FLunarTabDescriptor> InitialDescriptors = TabDescriptors;
		SetTabs(InitialDescriptors);
	}
	return ActivateTabByIdInternal(TabId, true, true);
}

UUserWidget* ULunarTabs::GetPageWidgetById(const FName TabId) const
{
	if (const TObjectPtr<UUserWidget>* Page = PageWidgets.Find(TabId))
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
	ApplyOrientation();
}

void ULunarTabs::NativeConstruct()
{
	Super::NativeConstruct();
	EnsureNativeLayout();
	if (!bTabsInitialized)
	{
		const TArray<FLunarTabDescriptor> InitialDescriptors = TabDescriptors;
		SetTabs(InitialDescriptors);
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
	if (AppliedOrientation != Orientation)
	{
		ApplyOrientation();
	}
	TickSpecializedStyleTransitions(InDeltaTime);
	ApplyPendingHeaderNavigation();
	CaptureActivePageSelection();
}

bool ULunarTabs::ResolveCommonStylePatch(FLunarCommonStylePatch& OutStyle, FString& OutError) const
{
	FLunarTabsStylePatch ResolvedStyle;
	if (!LunarStyleResolver::ResolveTabsStyle(
		StyleAsset,
		GetLunarVisualState(),
		StyleOverrides,
		ResolvedStyle,
		&OutError))
	{
		return false;
	}

	ULunarTabs* MutableThis = const_cast<ULunarTabs*>(this);
	MutableThis->ResolvedTabsStyle = ResolvedStyle;
	OutStyle = ResolvedStyle.Common;
	return true;
}

void ULunarTabs::ApplyResolvedCommonStyle(const FLunarCommonStylePatch& ResolvedStyle)
{
	Super::ApplyResolvedCommonStyle(ResolvedStyle);
	SetPagePaddingStyleTarget(
		LunarTabs_Private::MaterializePagePaddingStyle(ResolvedTabsStyle),
		GetLunarVisualState().bReduceMotion);
	RefreshAllHeaderPresentations();
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
			if (!PageClass || PageClass->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists))
			{
				OutError = FString::Printf(
					TEXT("Tab '%s' uses a non-instantiable page class."),
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
	PageContainer = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("LunarTabs_Pages"));

	if (!GeneratedRoot || !HorizontalLayout || !HorizontalHeaderStrip || !HorizontalPageHost
		|| !VerticalLayout || !VerticalHeaderStrip || !VerticalPageHost || !PageContainer)
	{
		return;
	}

	WidgetTree->RootWidget = GeneratedRoot;
	GeneratedRoot->AddChildToOverlay(HorizontalLayout);
	GeneratedRoot->AddChildToOverlay(VerticalLayout);

	if (UVerticalBoxSlot* HeaderSlot = HorizontalLayout->AddChildToVerticalBox(HorizontalHeaderStrip))
	{
		HeaderSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	}
	if (UVerticalBoxSlot* PageSlot = HorizontalLayout->AddChildToVerticalBox(HorizontalPageHost))
	{
		PageSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}
	if (UHorizontalBoxSlot* HeaderSlot = VerticalLayout->AddChildToHorizontalBox(VerticalHeaderStrip))
	{
		HeaderSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	}
	if (UHorizontalBoxSlot* PageSlot = VerticalLayout->AddChildToHorizontalBox(VerticalPageHost))
	{
		PageSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	HorizontalPageHost->SetContent(PageContainer);
	if (PreservedOwnerContent && PreservedOwnerContent != GeneratedRoot)
	{
		GeneratedRoot->AddChildToOverlay(PreservedOwnerContent);
	}

	bNativeLayoutInitialized = true;
	if (RuntimeState->PagePaddingTransition.bHasDisplayedStyle)
	{
		ApplyDisplayedPagePaddingStyle(RuntimeState->PagePaddingTransition.DisplayedStyle);
	}
	AppliedOrientation = Orientation;
	ApplyOrientation();
}

void ULunarTabs::ApplyOrientation()
{
	if (!bNativeLayoutInitialized)
	{
		return;
	}

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
	if (Orientation == Orient_Horizontal)
	{
		HorizontalPageHost->SetContent(PageContainer);
		HorizontalLayout->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		VerticalLayout->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		VerticalPageHost->SetContent(PageContainer);
		HorizontalLayout->SetVisibility(ESlateVisibility::Collapsed);
		VerticalLayout->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	AppliedOrientation = Orientation;
	InvalidateLayoutAndVolatility();
	if (ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem())
	{
		NavigationSubsystem->RefreshNavigationGraph();
	}
}

void ULunarTabs::ResetGeneratedTabs()
{
	for (const TPair<FName, TObjectPtr<UUserWidget>>& Pair : PageWidgets)
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
	RuntimeState->HeaderIndicatorTransitions.Reset();
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
	UUserWidget* HeaderWidget = LunarTabs_Private::CreateUserWidgetForTabs(this, Descriptor.HeaderWidgetClass);
	ULunarTabHeader* Header = Cast<ULunarTabHeader>(HeaderWidget);
	if (Header)
	{
		Header->InitializeTabHeader(this, Descriptor.TabId, Descriptor.bEnabled, Descriptor.DisabledReason);
	}
	return Header;
}

UUserWidget* ULunarTabs::CreatePageWidget(const FLunarTabDescriptor& Descriptor)
{
	if (TObjectPtr<UUserWidget>* ExistingPage = PageWidgets.Find(Descriptor.TabId))
	{
		return IsValid(ExistingPage->Get()) ? ExistingPage->Get() : nullptr;
	}

	UUserWidget* Page = Descriptor.PageWidgetInstance;
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

	Page->SetVisibility(ESlateVisibility::Collapsed);
	if (!PageContainer || !PageContainer->AddChildToOverlay(Page))
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
	if (TObjectPtr<UUserWidget>* Page = PageWidgets.Find(TabId))
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
		if (UUserWidget* ExistingPage = GetPageWidgetById(TabId))
		{
			ExistingPage->SetVisibility(ESlateVisibility::Visible);
			return true;
		}
	}

	bActivationInProgress = true;
	const FName PreviousTabId = ActiveTabId;
	UUserWidget* PreviousPage = GetPageWidgetById(PreviousTabId);
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

	UUserWidget* RequestedPage = CreatePageWidget(*Descriptor);
	if (!RequestedPage)
	{
		bActivationInProgress = false;
		// PreviousPage has not been hidden: page creation failure is atomic.
		RejectRequestedHeader();
		return false;
	}

	// The page becomes eligible before graph rebuild, restoration, or fallback calculation.
	RequestedPage->SetVisibility(ESlateVisibility::Visible);
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

	if (NavigationSubsystem)
	{
		NavigationSubsystem->RefreshNavigationGraph();
	}
	if (!PreviousTabId.IsNone() && PreviousTabId != ActiveTabId)
	{
		DestroyRecreatedPage(PreviousTabId);
	}

	bActivationInProgress = false;
	if (bBroadcastChange && PreviousTabId != ActiveTabId)
	{
		OnActiveTabChanged.Broadcast(PreviousTabId, ActiveTabId);
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
	if (bAlongStrip)
	{
		return FindAdjacentHeader(CurrentHeader, Direction);
	}

	const bool bTowardPage = Orientation == Orient_Horizontal
		? Direction == ELunarNavigationDirection::Down
		: Direction == ELunarNavigationDirection::Right;
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

bool ULunarTabs::ResolveHeaderStyle(
	const FLunarUIVisualState& HeaderVisualState,
	const FLunarCommonStylePatch& HeaderInstanceOverrides,
	FLunarCommonStylePatch& OutCommonStyle,
	FLunarTabsStylePatch* OutTabsStyle,
	FString& OutError) const
{
	FLunarTabsStylePatch HeaderTabsStyle;
	if (!LunarStyleResolver::ResolveTabsStyle(
		StyleAsset,
		HeaderVisualState,
		StyleOverrides,
		HeaderTabsStyle,
		&OutError))
	{
		return false;
	}

	OutCommonStyle = HeaderTabsStyle.HeaderStyle.Common;
	LunarStyleResolver::MergeCommonStylePatch(OutCommonStyle, HeaderInstanceOverrides);
	if (OutTabsStyle)
	{
		*OutTabsStyle = HeaderTabsStyle;
	}
	return true;
}

void ULunarTabs::ApplyHeaderPresentation(
	ULunarTabHeader* Header,
	const FLunarUIVisualState& HeaderVisualState)
{
	if (!Header)
	{
		return;
	}
	UImage* Indicator = HeaderIndicators.FindRef(Header->TabId);
	if (!Indicator)
	{
		return;
	}
	Indicator->SetVisibility(Header->bActiveTabHeader
		? ESlateVisibility::HitTestInvisible
		: ESlateVisibility::Collapsed);

	FLunarCommonStylePatch HeaderCommonStyle;
	FLunarTabsStylePatch HeaderTabsStyle;
	FString StyleError;
	if (!ResolveHeaderStyle(
		HeaderVisualState,
		FLunarCommonStylePatch(),
		HeaderCommonStyle,
		&HeaderTabsStyle,
		StyleError))
	{
		return;
	}

	SetHeaderIndicatorStyleTarget(
		Header->TabId,
		LunarTabs_Private::MaterializeIndicatorStyle(HeaderTabsStyle),
		HeaderVisualState.bReduceMotion);
}

void ULunarTabs::SetHeaderIndicatorStyleTarget(
	const FName TabId,
	const FLunarTabsStylePatch& NewTarget,
	const bool bReduceMotion)
{
	if (!HeaderIndicators.FindRef(TabId))
	{
		return;
	}

	FLunarTabsRuntimeState::FTransitionState& State =
		RuntimeState->HeaderIndicatorTransitions.FindOrAdd(TabId);
	if (!State.bHasDisplayedStyle || bReduceMotion)
	{
		ApplyImmediateIndicatorStyle(TabId, NewTarget);
		return;
	}
	if (LunarTabs_Private::AreIndicatorStylesEquivalent(State.LogicalTargetStyle, NewTarget))
	{
		// Reapply even for an unchanged target: the UImage may have been rebuilt or
		// an assigned style asset may have been reset without a visual-state change.
		ApplyDisplayedIndicatorStyle(TabId, State.DisplayedStyle);
		return;
	}

	if (State.bTransitionActive)
	{
		const bool bReturnsToSource = !State.bTransitionReversing
			&& LunarTabs_Private::AreIndicatorStylesEquivalent(State.TransitionSourceStyle, NewTarget);
		const bool bReturnsToForwardTarget = State.bTransitionReversing
			&& LunarTabs_Private::AreIndicatorStylesEquivalent(State.TransitionTargetStyle, NewTarget);
		if (bReturnsToSource || bReturnsToForwardTarget)
		{
			State.LogicalTargetStyle = NewTarget;
			State.bTransitionReversing = bReturnsToSource;
			State.DisplayedStyle.ActiveIndicatorBrush = NewTarget.ActiveIndicatorBrush;
			State.DisplayedStyle.Common.Transition = NewTarget.Common.Transition;
			ApplyDisplayedIndicatorStyle(TabId, State.DisplayedStyle);
			return;
		}
	}

	if (!NewTarget.Common.Transition.bEnabled
		|| NewTarget.Common.Transition.Duration <= 0.0f
		|| LunarTabs_Private::AreIndicatorVisualsEquivalent(State.DisplayedStyle, NewTarget))
	{
		ApplyImmediateIndicatorStyle(TabId, NewTarget);
		return;
	}

	State.TransitionSourceStyle = State.DisplayedStyle;
	State.TransitionTargetStyle = NewTarget;
	State.LogicalTargetStyle = NewTarget;
	State.TransitionElapsed = 0.0f;
	State.TransitionDuration = NewTarget.Common.Transition.Duration;
	State.bTransitionActive = true;
	State.bTransitionReversing = false;
	// Brushes are discrete. Apply the new logical target immediately while tint
	// interpolation starts from the exact currently displayed snapshot.
	State.DisplayedStyle.ActiveIndicatorBrush = NewTarget.ActiveIndicatorBrush;
	State.DisplayedStyle.Common.Transition = NewTarget.Common.Transition;
	ApplyDisplayedIndicatorStyle(TabId, State.DisplayedStyle);
}

void ULunarTabs::SetPagePaddingStyleTarget(
	const FLunarTabsStylePatch& NewTarget,
	const bool bReduceMotion)
{
	FLunarTabsRuntimeState::FTransitionState& State = RuntimeState->PagePaddingTransition;
	if (!State.bHasDisplayedStyle || bReduceMotion)
	{
		ApplyImmediatePagePaddingStyle(NewTarget);
		return;
	}
	if (LunarTabs_Private::ArePagePaddingStylesEquivalent(State.LogicalTargetStyle, NewTarget))
	{
		ApplyDisplayedPagePaddingStyle(State.DisplayedStyle);
		return;
	}

	if (State.bTransitionActive)
	{
		const bool bReturnsToSource = !State.bTransitionReversing
			&& LunarTabs_Private::ArePagePaddingStylesEquivalent(State.TransitionSourceStyle, NewTarget);
		const bool bReturnsToForwardTarget = State.bTransitionReversing
			&& LunarTabs_Private::ArePagePaddingStylesEquivalent(State.TransitionTargetStyle, NewTarget);
		if (bReturnsToSource || bReturnsToForwardTarget)
		{
			State.LogicalTargetStyle = NewTarget;
			State.bTransitionReversing = bReturnsToSource;
			State.DisplayedStyle.Common.Transition = NewTarget.Common.Transition;
			ApplyDisplayedPagePaddingStyle(State.DisplayedStyle);
			return;
		}
	}

	if (!NewTarget.Common.Transition.bEnabled
		|| NewTarget.Common.Transition.Duration <= 0.0f
		|| State.DisplayedStyle.PagePadding == NewTarget.PagePadding)
	{
		ApplyImmediatePagePaddingStyle(NewTarget);
		return;
	}

	State.TransitionSourceStyle = State.DisplayedStyle;
	State.TransitionTargetStyle = NewTarget;
	State.LogicalTargetStyle = NewTarget;
	State.TransitionElapsed = 0.0f;
	State.TransitionDuration = NewTarget.Common.Transition.Duration;
	State.bTransitionActive = true;
	State.bTransitionReversing = false;
	State.DisplayedStyle.Common.Transition = NewTarget.Common.Transition;
	ApplyDisplayedPagePaddingStyle(State.DisplayedStyle);
}

void ULunarTabs::ApplyImmediateIndicatorStyle(
	const FName TabId,
	const FLunarTabsStylePatch& NewStyle)
{
	FLunarTabsRuntimeState::FTransitionState& State =
		RuntimeState->HeaderIndicatorTransitions.FindOrAdd(TabId);
	State.DisplayedStyle = NewStyle;
	State.TransitionSourceStyle = NewStyle;
	State.TransitionTargetStyle = NewStyle;
	State.LogicalTargetStyle = NewStyle;
	State.TransitionElapsed = 0.0f;
	State.TransitionDuration = 0.0f;
	State.bHasDisplayedStyle = true;
	State.bTransitionActive = false;
	State.bTransitionReversing = false;
	ApplyDisplayedIndicatorStyle(TabId, State.DisplayedStyle);
}

void ULunarTabs::ApplyImmediatePagePaddingStyle(const FLunarTabsStylePatch& NewStyle)
{
	FLunarTabsRuntimeState::FTransitionState& State = RuntimeState->PagePaddingTransition;
	State.DisplayedStyle = NewStyle;
	State.TransitionSourceStyle = NewStyle;
	State.TransitionTargetStyle = NewStyle;
	State.LogicalTargetStyle = NewStyle;
	State.TransitionElapsed = 0.0f;
	State.TransitionDuration = 0.0f;
	State.bHasDisplayedStyle = true;
	State.bTransitionActive = false;
	State.bTransitionReversing = false;
	ApplyDisplayedPagePaddingStyle(State.DisplayedStyle);
}

void ULunarTabs::TickSpecializedStyleTransitions(const float InDeltaTime)
{
	if (GetLunarVisualState().bReduceMotion)
	{
		if (RuntimeState->PagePaddingTransition.bTransitionActive)
		{
			const FLunarTabsStylePatch LogicalTarget =
				RuntimeState->PagePaddingTransition.LogicalTargetStyle;
			ApplyImmediatePagePaddingStyle(LogicalTarget);
		}
		for (TPair<FName, FLunarTabsRuntimeState::FTransitionState>& Pair :
			RuntimeState->HeaderIndicatorTransitions)
		{
			if (Pair.Value.bTransitionActive)
			{
				const FLunarTabsStylePatch LogicalTarget = Pair.Value.LogicalTargetStyle;
				ApplyImmediateIndicatorStyle(Pair.Key, LogicalTarget);
			}
		}
		return;
	}

	TickPagePaddingTransition(InDeltaTime);
	for (const TPair<FName, FLunarTabsRuntimeState::FTransitionState>& Pair :
		RuntimeState->HeaderIndicatorTransitions)
	{
		TickHeaderIndicatorTransition(Pair.Key, InDeltaTime);
	}
}

void ULunarTabs::TickHeaderIndicatorTransition(
	const FName TabId,
	const float InDeltaTime)
{
	FLunarTabsRuntimeState::FTransitionState* StatePointer =
		RuntimeState->HeaderIndicatorTransitions.Find(TabId);
	if (!StatePointer || !StatePointer->bTransitionActive)
	{
		return;
	}
	FLunarTabsRuntimeState::FTransitionState& State = *StatePointer;

	const float TransitionDelta = FMath::Max(0.0f, InDeltaTime);
	State.TransitionElapsed += State.bTransitionReversing ? -TransitionDelta : TransitionDelta;
	State.TransitionElapsed = FMath::Clamp(
		State.TransitionElapsed,
		0.0f,
		State.TransitionDuration);
	const float Alpha = State.TransitionDuration > 0.0f
		? State.TransitionElapsed / State.TransitionDuration
		: 1.0f;
	const float EasedAlpha = static_cast<float>(UKismetMathLibrary::Ease(
		0.0,
		1.0,
		Alpha,
		State.TransitionTargetStyle.Common.Transition.Easing));

	State.DisplayedStyle.ActiveIndicatorBrush = State.LogicalTargetStyle.ActiveIndicatorBrush;
	State.DisplayedStyle.ActiveIndicatorTint = FMath::Lerp(
		State.TransitionSourceStyle.ActiveIndicatorTint,
		State.TransitionTargetStyle.ActiveIndicatorTint,
		EasedAlpha);
	State.DisplayedStyle.Common.Transition = State.LogicalTargetStyle.Common.Transition;
	const bool bFinished = State.bTransitionReversing ? Alpha <= 0.0f : Alpha >= 1.0f;
	if (bFinished)
	{
		State.DisplayedStyle = State.LogicalTargetStyle;
		State.bTransitionActive = false;
		State.bTransitionReversing = false;
	}
	ApplyDisplayedIndicatorStyle(TabId, State.DisplayedStyle);
}

void ULunarTabs::TickPagePaddingTransition(const float InDeltaTime)
{
	FLunarTabsRuntimeState::FTransitionState& State = RuntimeState->PagePaddingTransition;
	if (!State.bTransitionActive)
	{
		return;
	}

	const float TransitionDelta = FMath::Max(0.0f, InDeltaTime);
	State.TransitionElapsed += State.bTransitionReversing ? -TransitionDelta : TransitionDelta;
	State.TransitionElapsed = FMath::Clamp(
		State.TransitionElapsed,
		0.0f,
		State.TransitionDuration);
	const float Alpha = State.TransitionDuration > 0.0f
		? State.TransitionElapsed / State.TransitionDuration
		: 1.0f;
	const float EasedAlpha = static_cast<float>(UKismetMathLibrary::Ease(
		0.0,
		1.0,
		Alpha,
		State.TransitionTargetStyle.Common.Transition.Easing));

	State.DisplayedStyle.PagePadding = LunarTabs_Private::InterpolateMargin(
		State.TransitionSourceStyle.PagePadding,
		State.TransitionTargetStyle.PagePadding,
		EasedAlpha);
	State.DisplayedStyle.Common.Transition = State.LogicalTargetStyle.Common.Transition;
	const bool bFinished = State.bTransitionReversing ? Alpha <= 0.0f : Alpha >= 1.0f;
	if (bFinished)
	{
		State.DisplayedStyle = State.LogicalTargetStyle;
		State.bTransitionActive = false;
		State.bTransitionReversing = false;
	}
	ApplyDisplayedPagePaddingStyle(State.DisplayedStyle);
}

void ULunarTabs::ApplyDisplayedIndicatorStyle(
	const FName TabId,
	const FLunarTabsStylePatch& DisplayedStyle) const
{
	if (UImage* Indicator = HeaderIndicators.FindRef(TabId))
	{
		Indicator->SetBrush(DisplayedStyle.ActiveIndicatorBrush);
		Indicator->SetColorAndOpacity(DisplayedStyle.ActiveIndicatorTint);
	}
}

void ULunarTabs::ApplyDisplayedPagePaddingStyle(
	const FLunarTabsStylePatch& DisplayedStyle) const
{
	if (HorizontalPageHost)
	{
		HorizontalPageHost->SetPadding(DisplayedStyle.PagePadding);
	}
	if (VerticalPageHost)
	{
		VerticalPageHost->SetPadding(DisplayedStyle.PagePadding);
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
}

void ULunarTabs::ReportTabsError(const FString& Message) const
{
	ULunarConsoleSubsystem::AddMessage(
		FGameplayTag::RequestGameplayTag(TEXT("Lunar.Console"), false),
		ELunarConsoleMessageVerbosity::Error,
		Message);
	UE_LOG(LogLunarTabs, Error, TEXT("%s (%s)"), *Message, *GetPathName());
}
