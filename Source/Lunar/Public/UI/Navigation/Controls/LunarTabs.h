// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarTabs.h
 * @brief Declares the composite Lunar tab strip and page-lifetime controller.
 * @ingroup LunarNavigationControls
 */

#pragma once

#include "CoreMinimal.h"
#include "UI/Navigation/Core/LunarNavigableWidget.h"
#include "LunarTabs.generated.h"

class UBorder;
class UHorizontalBox;
class UImage;
class UOverlay;
class UUserWidget;
class UVerticalBox;
class UWidget;
class ULunarNavigationSubsystem;
class ULunarTabHeader;
struct FLunarTabsRuntimeState;

/**
 * Composite tab strip with stable IDs, independently selectable headers, and owned page lifetimes.
 *
 * Header selection is distinct from the active page. The owner generates headers, creates pages
 * according to PageLifetime, and restores per-page descendant selection by stable navigation ID.
 *
 * @ingroup LunarNavigationControls
 */
UCLASS(Blueprintable)
class LUNAR_API ULunarTabs : public ULunarNavigableWidget
{
	GENERATED_BODY()

public:
	/** Creates an empty Tabs control. @param ObjectInitializer Unreal object initializer. */
	ULunarTabs(const FObjectInitializer& ObjectInitializer);
	/** Releases non-UObject transition state and its GC references. */
	virtual ~ULunarTabs() override;

	/** Replaces descriptors and rebuilds generated headers and pages. @param NewTabs New tab descriptors. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|Tabs")
	void SetTabs(const TArray<FLunarTabDescriptor>& NewTabs);

	/** Activates an enabled tab by stable ID. @param TabId Stable tab ID. @return True when the tab is active. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|Tabs")
	bool ActivateTabById(FName TabId);

	/** Returns the stable ID of the active page. @return Active tab ID, or NAME_None. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation|Tabs")
	FName GetActiveTabId() const { return ActiveTabId; }

	/** Returns a created page instance by stable tab ID. @param TabId Stable tab ID. @return Page widget, or null when not created. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation|Tabs")
	UUserWidget* GetPageWidgetById(FName TabId) const;

	/** Authored stable tab descriptors used to generate headers and pages. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Tabs", meta = (ExposeOnSpawn = "true"))
	TArray<FLunarTabDescriptor> TabDescriptors;

	/** Stable ID of the active page. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Tabs")
	FName ActiveTabId = NAME_None;

	/** Creation and retention policy applied to generated page widgets. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Tabs")
	ELunarTabPageLifetime PageLifetime = ELunarTabPageLifetime::LazyCached;

	/** Horizontal places the strip above the page; Vertical places it to the left. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Tabs")
	TEnumAsByte<EOrientation> Orientation = Orient_Horizontal;

	/** Broadcast after the active tab ID changes. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation|Tabs")
	FLunarTabsActiveTabChangedSignature OnActiveTabChanged;

protected:
	/** Builds the owner-preserving native layout used by the tab strip and page host. @return Root Slate widget. */
	virtual TSharedRef<SWidget> RebuildWidget() override;
	/** Synchronizes descriptors, orientation, active page, and style state. */
	virtual void SynchronizeProperties() override;
	/** Initializes generated tab content after the widget enters the tree. */
	virtual void NativeConstruct() override;
	/** Applies deferred navigation and specialized style transitions. @param MyGeometry Cached widget geometry. @param InDeltaTime Elapsed seconds. */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	/** Resolves common and Tabs-specific style layers. @param OutStyle Resolved common patch. @param OutError Actionable failure text. @return True on success. */
	virtual bool ResolveCommonStylePatch(FLunarCommonStylePatch& OutStyle, FString& OutError) const override;
	/** Applies common style and refreshes Tabs-specific presentation targets. @param ResolvedStyle Resolved common style patch. */
	virtual void ApplyResolvedCommonStyle(const FLunarCommonStylePatch& ResolvedStyle) override;

private:
	/** Validates stable IDs, classes, and descriptor consistency. @param Descriptors Descriptors to inspect. @param OutError Actionable failure text. @return True when valid. */
	bool ValidateTabDescriptors(const TArray<FLunarTabDescriptor>& Descriptors, FString& OutError) const;
	/** Creates the generated UMG layout once while preserving authored owner content. */
	void EnsureNativeLayout();
	/** Places the header strip and page host according to Orientation. */
	void ApplyOrientation();
	/** Removes generated headers and pages and clears their runtime caches. */
	void ResetGeneratedTabs();
	/** Creates one header per valid descriptor. @return True when all headers were built. */
	bool BuildHeaders();
	/** Creates and initializes a generated header. @param Descriptor Source descriptor. @return Header widget, or null on failure. */
	ULunarTabHeader* CreateHeaderWidget(const FLunarTabDescriptor& Descriptor);
	/** Creates a page instance for a descriptor. @param Descriptor Source descriptor. @return New page widget, or null on failure. */
	UUserWidget* CreatePageWidget(const FLunarTabDescriptor& Descriptor);
	/** Removes a page managed by RecreateOnActivation. @param TabId Stable tab ID. */
	void DestroyRecreatedPage(FName TabId);
	/** Activates a tab with caller-controlled notifications. @param TabId Stable tab ID. @param bNotifyRequestedHeader Whether the requested header receives activation feedback. @param bBroadcastChange Whether to broadcast active-ID changes. @return True on success. */
	bool ActivateTabByIdInternal(FName TabId, bool bNotifyRequestedHeader, bool bBroadcastChange);
	/** Activates the tab represented by an owned header. @param Header Candidate owned header. @return True on success. */
	bool TryActivateTabFromHeader(ULunarTabHeader* Header);
	/** Tests whether a descriptor is enabled. @param TabId Stable tab ID. @return True when enabled. */
	bool IsTabEnabled(FName TabId) const;
	/** Finds a descriptor by stable ID. @param TabId Stable tab ID. @return Descriptor, or null. */
	const FLunarTabDescriptor* FindTabDescriptor(FName TabId) const;
	/** Chooses an enabled initial tab deterministically. @param RequestedTabId Preferred ID. @return Chosen ID, or NAME_None. */
	FName ChooseInitialActiveTabId(FName RequestedTabId) const;
	/** Resolves the owning LocalPlayer navigation subsystem. @return Navigation subsystem, or null. */
	ULunarNavigationSubsystem* ResolveNavigationSubsystem() const;
	/** Tests whether a header may route a direction through this composite. @param CurrentHeader Source header. @param Direction Requested direction. @return True when a target exists. */
	bool CanRouteHeaderDirection(const ULunarTabHeader* CurrentHeader, ELunarNavigationDirection Direction) const;
	/** Queues composite header navigation for post-action application. @param CurrentHeader Source header. @param Direction Requested direction. @return True when queued. */
	bool QueueHeaderDirection(ULunarTabHeader* CurrentHeader, ELunarNavigationDirection Direction);
	/** Resolves a header direction to an adjacent header or page entry. @param CurrentHeader Source header. @param Direction Requested direction. @return Target navigable widget, or null. */
	ULunarNavigableWidget* ResolveHeaderDirectionTarget(
		const ULunarTabHeader* CurrentHeader,
		ELunarNavigationDirection Direction) const;
	/** Finds the adjacent enabled header along the tab-strip axis. @param CurrentHeader Source header. @param Direction Requested direction. @return Adjacent header, or null. */
	ULunarTabHeader* FindAdjacentHeader(
		const ULunarTabHeader* CurrentHeader,
		ELunarNavigationDirection Direction) const;
	/** Resolves the selected or first eligible descendant of the active page. @return Page-entry target, or null. */
	ULunarNavigableWidget* ResolveActivePageEntryTarget() const;
	/** Applies a previously queued header-to-header or header-to-page selection change. */
	void ApplyPendingHeaderNavigation();
	/** Stores the active page's selected descendant by stable navigation ID. */
	void CaptureActivePageSelection();
	/** Finds a page descendant by stable navigation ID. @param Page Page root. @param InNavigationId Stable navigation ID. @return Matching descendant, or null. */
	ULunarNavigableWidget* FindPageDescendantByNavigationId(UUserWidget* Page, FName InNavigationId) const;
	/** Finds the first eligible navigable descendant of a page. @param Page Page root. @return First eligible descendant, or null. */
	ULunarNavigableWidget* FindFirstEligiblePageDescendant(UUserWidget* Page) const;
	/** Collects all Lunar navigable descendants in deterministic widget-tree order. @param Page Page root. @param OutWidgets Destination array. */
	void GatherPageNavigableDescendants(UUserWidget* Page, TArray<ULunarNavigableWidget*>& OutWidgets) const;
	/** Tests whether a widget belongs to a generated page subtree. @param Widget Candidate widget. @param Page Page root. @return True when contained. */
	bool IsWidgetInsidePage(const UWidget* Widget, const UUserWidget* Page) const;
	/** Resolves a header visual state through Tabs and instance style layers. @param HeaderVisualState Header state flags. @param HeaderInstanceOverrides Header-specific overrides. @param OutCommonStyle Resolved common style. @param OutTabsStyle Optional resolved Tabs patch. @param OutError Actionable failure text. @return True on success. */
	bool ResolveHeaderStyle(
		const FLunarUIVisualState& HeaderVisualState,
		const FLunarCommonStylePatch& HeaderInstanceOverrides,
		FLunarCommonStylePatch& OutCommonStyle,
		FLunarTabsStylePatch* OutTabsStyle,
		FString& OutError) const;
	/** Applies resolved common and specialized style to one generated header. @param Header Header widget. @param HeaderVisualState Header state flags. */
	void ApplyHeaderPresentation(ULunarTabHeader* Header, const FLunarUIVisualState& HeaderVisualState);
	/** Updates the indicator transition target for a tab. @param TabId Stable tab ID. @param NewTarget New Tabs style target. @param bReduceMotion Whether to apply immediately. */
	void SetHeaderIndicatorStyleTarget(
		FName TabId,
		const FLunarTabsStylePatch& NewTarget,
		bool bReduceMotion);
	/** Updates the page-padding transition target. @param NewTarget New Tabs style target. @param bReduceMotion Whether to apply immediately. */
	void SetPagePaddingStyleTarget(
		const FLunarTabsStylePatch& NewTarget,
		bool bReduceMotion);
	/** Applies indicator style without interpolation. @param TabId Stable tab ID. @param NewStyle Style to display. */
	void ApplyImmediateIndicatorStyle(FName TabId, const FLunarTabsStylePatch& NewStyle);
	/** Applies page padding without interpolation. @param NewStyle Style to display. */
	void ApplyImmediatePagePaddingStyle(const FLunarTabsStylePatch& NewStyle);
	/** Advances all Tabs-specific visual transitions. @param InDeltaTime Elapsed seconds. */
	void TickSpecializedStyleTransitions(float InDeltaTime);
	/** Advances one header-indicator transition. @param TabId Stable tab ID. @param InDeltaTime Elapsed seconds. */
	void TickHeaderIndicatorTransition(FName TabId, float InDeltaTime);
	/** Advances the active page-padding transition. @param InDeltaTime Elapsed seconds. */
	void TickPagePaddingTransition(float InDeltaTime);
	/** Presents an interpolated indicator style. @param TabId Stable tab ID. @param DisplayedStyle Interpolated style. */
	void ApplyDisplayedIndicatorStyle(
		FName TabId,
		const FLunarTabsStylePatch& DisplayedStyle) const;
	/** Presents interpolated page padding. @param DisplayedStyle Interpolated style. */
	void ApplyDisplayedPagePaddingStyle(const FLunarTabsStylePatch& DisplayedStyle) const;
	/** Re-resolves and presents every generated header's current visual state. */
	void RefreshAllHeaderPresentations();
	/** Reports an actionable Tabs configuration or runtime error. @param Message Error text. */
	void ReportTabsError(const FString& Message) const;

private:
	/** Generated root that overlays preserved owner content and native Tabs content. */
	UPROPERTY(Transient) TObjectPtr<UOverlay> GeneratedRoot = nullptr;
	/** Authored content preserved while the native layout owns the root. */
	UPROPERTY(Transient) TObjectPtr<UWidget> PreservedOwnerContent = nullptr;
	/** Layout used when the header strip sits above the page. */
	UPROPERTY(Transient) TObjectPtr<UVerticalBox> HorizontalLayout = nullptr;
	/** Header strip used by horizontal orientation. */
	UPROPERTY(Transient) TObjectPtr<UHorizontalBox> HorizontalHeaderStrip = nullptr;
	/** Page host used by horizontal orientation. */
	UPROPERTY(Transient) TObjectPtr<UBorder> HorizontalPageHost = nullptr;
	/** Layout used when the header strip sits to the left of the page. */
	UPROPERTY(Transient) TObjectPtr<UHorizontalBox> VerticalLayout = nullptr;
	/** Header strip used by vertical orientation. */
	UPROPERTY(Transient) TObjectPtr<UVerticalBox> VerticalHeaderStrip = nullptr;
	/** Page host used by vertical orientation. */
	UPROPERTY(Transient) TObjectPtr<UBorder> VerticalPageHost = nullptr;
	/** Overlay containing generated page widgets. */
	UPROPERTY(Transient) TObjectPtr<UOverlay> PageContainer = nullptr;

	/** Generated headers indexed by stable tab ID. */
	UPROPERTY(Transient) TMap<FName, TObjectPtr<ULunarTabHeader>> HeaderWidgets;
	/** Header presentation wrappers indexed by stable tab ID. */
	UPROPERTY(Transient) TMap<FName, TObjectPtr<UOverlay>> HeaderWrappers;
	/** Generated indicator images indexed by stable tab ID. */
	UPROPERTY(Transient) TMap<FName, TObjectPtr<UImage>> HeaderIndicators;
	/** Created page widgets indexed by stable tab ID. */
	UPROPERTY(Transient) TMap<FName, TObjectPtr<UUserWidget>> PageWidgets;
	/** Last selected descendant ID retained independently for each page. */
	UPROPERTY(Transient) TMap<FName, FName> LastPageDescendantNavigationIds;
	/** Last successfully resolved Tabs-specific style. */
	UPROPERTY(Transient) FLunarTabsStylePatch ResolvedTabsStyle;
	/** Header that initiated the deferred composite navigation request. */
	UPROPERTY(Transient) TWeakObjectPtr<ULunarTabHeader> PendingHeaderNavigationSource;
	/** Resolved destination for deferred composite navigation. */
	UPROPERTY(Transient) TWeakObjectPtr<ULunarNavigableWidget> PendingHeaderNavigationTarget;
	/** Heap-owned transition state that reports its UObject references to GC. */
	FLunarTabsRuntimeState* RuntimeState = nullptr;

	/** Orientation currently represented by the generated hierarchy. */
	TEnumAsByte<EOrientation> AppliedOrientation = Orient_Horizontal;
	/** Whether the native generated layout has been created. */
	bool bNativeLayoutInitialized = false;
	/** Whether descriptors have been validated and generated controls initialized. */
	bool bTabsInitialized = false;
	/** Reentrancy guard for active-tab changes. */
	bool bActivationInProgress = false;

	/** Grants generated headers access to owner-controlled routing and activation. */
	friend class ULunarTabHeader;
};
