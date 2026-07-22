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
class UWidgetSwitcher;
class ULunarNavigationSubsystem;
class ULunarTabHeader;
class ULunarTabPage;

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

	/** Replaces descriptors and rebuilds generated headers and pages. @param NewTabs New tab descriptors. @param NotificationPolicy Whether to publish notifications if the active tab changes; defaults to Notify. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Tabs", meta = (AdvancedDisplay = "NotificationPolicy"))
	void SetTabs(const TArray<FLunarTabDescriptor>& NewTabs, ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);

	/** Activates an enabled tab by stable ID. @param TabId Stable tab ID. @param NotificationPolicy Whether to publish normal change notifications; defaults to Notify. @return True when the tab is active. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Tabs", meta = (AdvancedDisplay = "NotificationPolicy"))
	bool ActivateTabById(FName TabId, ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);

	/** Clears the active page when bAllowNoActiveTab is enabled. @param NotificationPolicy Whether to publish the active-ID change; defaults to Notify. @return True when no tab is active. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Tabs", meta = (AdvancedDisplay = "NotificationPolicy"))
	bool ClearActiveTab(ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);

	/** Returns the stable ID of the active page. @return Active tab ID, or NAME_None. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Tabs")
	FName GetActiveTabId() const { return ActiveTabId; }

	/** Returns a created page instance by stable tab ID. @param TabId Stable tab ID. @return Page widget, or null when not created. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Tabs")
	ULunarTabPage* GetPageWidgetById(FName TabId) const;

	/** Sets the native active-indicator brush. @param NewBrush New indicator brush. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Tabs|Presentation")
	void SetActiveIndicatorBrush(const FSlateBrush& NewBrush);

	/** Returns the native active-indicator brush. @return Current indicator brush. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Tabs|Presentation")
	FSlateBrush GetActiveIndicatorBrush() const { return ActiveIndicatorBrush; }

	/** Sets the native active-indicator tint. @param NewTint New indicator tint. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Tabs|Presentation")
	void SetActiveIndicatorTint(FLinearColor NewTint);

	/** Returns the native active-indicator tint. @return Current indicator tint. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Tabs|Presentation")
	FLinearColor GetActiveIndicatorTint() const { return ActiveIndicatorTint; }

	/** Sets the native active-indicator desired size. @param NewSize New indicator size. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Tabs|Presentation")
	void SetActiveIndicatorSize(FVector2D NewSize);

	/** Returns the native active-indicator desired size. @return Current indicator size. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Tabs|Presentation")
	FVector2D GetActiveIndicatorSize() const { return ActiveIndicatorSize; }

	/** Configures every native active-indicator value. @param NewBrush New brush. @param NewTint New tint. @param NewSize New desired size. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Tabs|Presentation")
	void ConfigureActiveIndicatorPresentation(const FSlateBrush& NewBrush, FLinearColor NewTint, FVector2D NewSize);

	/** Returns every native active-indicator value. @param OutBrush Current brush. @param OutTint Current tint. @param OutSize Current desired size. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Tabs|Presentation")
	void GetActiveIndicatorPresentation(FSlateBrush& OutBrush, FLinearColor& OutTint, FVector2D& OutSize) const;

	/** Sets padding around the native page host. @param NewPadding New page padding. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Tabs|Presentation")
	void SetPagePresentationPadding(FMargin NewPadding);

	/** Returns padding around the native page host. @return Current page padding. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Tabs|Presentation")
	FMargin GetPagePresentationPadding() const { return PagePresentationPadding; }

	/** Configures every native Tabs presentation value. @param NewBrush New indicator brush. @param NewTint New indicator tint. @param NewSize New indicator size. @param NewPagePadding New page padding. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Tabs|Presentation")
	void ConfigureTabsPresentation(const FSlateBrush& NewBrush, FLinearColor NewTint, FVector2D NewSize, FMargin NewPagePadding);

	/** Returns every cached native Tabs presentation value. @param OutBrush Current indicator brush. @param OutTint Current indicator tint. @param OutSize Current indicator size. @param OutPagePadding Current page padding. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Tabs|Presentation")
	void GetTabsPresentation(FSlateBrush& OutBrush, FLinearColor& OutTint, FVector2D& OutSize, FMargin& OutPagePadding) const;
	/** Authored stable tab descriptors used to generate headers and pages. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Tabs", meta = (ExposeOnSpawn = "true"))
	TArray<FLunarTabDescriptor> TabDescriptors;

	/** Stable ID of the active page. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Tabs")
	FName ActiveTabId = NAME_None;

	/** Creation and retention policy applied to generated page widgets. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Tabs")
	ELunarTabPageLifetime PageLifetime = ELunarTabPageLifetime::LazyCached;

	/** Allows Tabs to intentionally have no active page. Disabled by default. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Tabs")
	bool bAllowNoActiveTab = false;

	/** Direction in which generated Tab Headers are arranged. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Tabs")
	TEnumAsByte<EOrientation> Orientation = Orient_Horizontal;

	/** Position of the active page relative to the header strip. Automatic preserves Bottom/Right legacy behavior. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Tabs")
	ELunarTabPagePlacement PagePlacement = ELunarTabPagePlacement::Automatic;

	/** Broadcast after the active tab ID changes. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Tabs")
	FLunarTabsActiveTabChangedSignature OnActiveTabChanged;

#if WITH_EDITORONLY_DATA
	/** Stable tab ID displayed as active in the UMG Designer; None uses ActiveTabId and normal fallback. */
	UPROPERTY(EditAnywhere, Category = "Lunar|UI|Tabs|Designer Preview", meta = (GetOptions = "GetDesignerPreviewTabIdOptions"))
	FName DesignerPreviewTabId = NAME_None;

	/** Previews Tabs with no active page. Available only when bAllowNoActiveTab is enabled. */
	UPROPERTY(EditAnywhere, Category = "Lunar|UI|Tabs|Designer Preview", meta = (EditCondition = "bAllowNoActiveTab", EditConditionHides))
	bool bDesignerPreviewNoActiveTab = false;
#endif

protected:
	/** Builds the owner-preserving native layout used by the tab strip and page host. @return Root Slate widget. */
	virtual TSharedRef<SWidget> RebuildWidget() override;
	/** Synchronizes descriptors, orientation, active page, and native presentation. */
	virtual void SynchronizeProperties() override;
	/** Initializes generated tab content after the widget enters the tree. */
	virtual void NativeConstruct() override;
	/** Applies deferred navigation and page-selection capture. @param MyGeometry Cached widget geometry. @param InDeltaTime Elapsed seconds. */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
#if WITH_EDITOR
	/** Rebuilds the live UMG Designer preview immediately after authored properties change. */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
private:
	/** Validates stable IDs, classes, and descriptor consistency. @param Descriptors Descriptors to inspect. @param OutError Actionable failure text. @return True when valid. */
	bool ValidateTabDescriptors(const TArray<FLunarTabDescriptor>& Descriptors, FString& OutError) const;
	/** Creates the generated UMG layout once while preserving authored owner content. */
	void EnsureNativeLayout();
	/** Places the header strip and page host according to Orientation and PagePlacement. */
	void ApplyOrientation();
	/** Resolves Automatic into the conventional placement for the current header orientation. @return Effective page placement. */
	ELunarTabPagePlacement ResolvePagePlacement() const;
	/** Removes generated headers and pages and clears their runtime caches. */
	void ResetGeneratedTabs();
	/** Creates one header per valid descriptor. @return True when all headers were built. */
	bool BuildHeaders();
	/** Creates and initializes a generated header. @param Descriptor Source descriptor. @return Header widget, or null on failure. */
	ULunarTabHeader* CreateHeaderWidget(const FLunarTabDescriptor& Descriptor);
	/** Creates a page instance for a descriptor. @param Descriptor Source descriptor. @return New page widget, or null on failure. */
	ULunarTabPage* CreatePageWidget(const FLunarTabDescriptor& Descriptor);
	/** Removes a page managed by RecreateOnActivation. @param TabId Stable tab ID. */
	void DestroyRecreatedPage(FName TabId);
	/** Activates a tab with caller-controlled notifications. @param TabId Stable tab ID. @param bNotifyRequestedHeader Whether the requested header receives activation feedback. @param bBroadcastChange Whether to broadcast active-ID changes. @return True on success. */
	bool ActivateTabByIdInternal(FName TabId, bool bNotifyRequestedHeader, bool bBroadcastChange);
	/** Clears the current page with caller-controlled notifications. @param bBroadcastChange Whether to publish OnActiveTabChanged. @return True when no tab is active. */
	bool ClearActiveTabInternal(bool bBroadcastChange);
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
	/** Applies cached presentation to native indicators and page hosts. */
	void ApplyNativePresentation();
	/** Refreshes every generated header visual state and native indicator. */
	void RefreshAllHeaderPresentations();
	/** Returns descriptor IDs for the Designer preview dropdown. @return Stable authored tab IDs. */
	UFUNCTION()
	TArray<FString> GetDesignerPreviewTabIdOptions() const;
#if WITH_EDITOR
	/** Rebuilds generated headers and every page for UMG Designer preview. */
	void RebuildDesignerPreview();
	/** Applies the selected Designer preview ID without recreating generated headers or pages. */
	void ApplyDesignerPreviewSelection();
	/** Resolves the effective Designer preview ID against authored state and enabled descriptors. @return Preview tab ID, or NAME_None. */
	FName ResolveDesignerPreviewTabId() const;
#endif
	/** Reports an actionable Tabs configuration or runtime error. @param Message Error text. */
	void ReportTabsError(const FString& Message) const;

private:
	/** Generated root that overlays preserved owner content and native Tabs content. */
	UPROPERTY(Transient) TObjectPtr<UOverlay> GeneratedRoot = nullptr;
	/** Authored content preserved while the native layout owns the root. */
	UPROPERTY(Transient) TObjectPtr<UWidget> PreservedOwnerContent = nullptr;
	/** Vertical arrangement used for Top and Bottom page placement. */
	UPROPERTY(Transient) TObjectPtr<UVerticalBox> HorizontalLayout = nullptr;
	/** Header strip used by horizontal orientation. */
	UPROPERTY(Transient) TObjectPtr<UHorizontalBox> HorizontalHeaderStrip = nullptr;
	/** Page host used by Top and Bottom placement. */
	UPROPERTY(Transient) TObjectPtr<UBorder> HorizontalPageHost = nullptr;
	/** Horizontal arrangement used for Left and Right page placement. */
	UPROPERTY(Transient) TObjectPtr<UHorizontalBox> VerticalLayout = nullptr;
	/** Header strip used by vertical orientation. */
	UPROPERTY(Transient) TObjectPtr<UVerticalBox> VerticalHeaderStrip = nullptr;
	/** Page host used by Left and Right placement. */
	UPROPERTY(Transient) TObjectPtr<UBorder> VerticalPageHost = nullptr;
	/** Switcher containing generated page widgets with exactly one active Slate child. */
	UPROPERTY(Transient) TObjectPtr<UWidgetSwitcher> PageContainer = nullptr;

	/** Generated headers indexed by stable tab ID. */
	UPROPERTY(Transient) TMap<FName, TObjectPtr<ULunarTabHeader>> HeaderWidgets;
	/** Header presentation wrappers indexed by stable tab ID. */
	UPROPERTY(Transient) TMap<FName, TObjectPtr<UOverlay>> HeaderWrappers;
	/** Generated indicator images indexed by stable tab ID. */
	UPROPERTY(Transient) TMap<FName, TObjectPtr<UImage>> HeaderIndicators;
	/** Cached native active-indicator brush reapplied after hierarchy reconstruction. */
	UPROPERTY(Transient) FSlateBrush ActiveIndicatorBrush;
	/** Cached native active-indicator tint reapplied after hierarchy reconstruction. */
	UPROPERTY(Transient) FLinearColor ActiveIndicatorTint = FLinearColor::White;
	/** Cached native active-indicator desired size. */
	UPROPERTY(Transient) FVector2D ActiveIndicatorSize = FVector2D::ZeroVector;
	/** Cached native page-host padding. */
	UPROPERTY(Transient) FMargin PagePresentationPadding;
	/** Created page widgets indexed by stable tab ID. */
	UPROPERTY(Transient) TMap<FName, TObjectPtr<ULunarTabPage>> PageWidgets;
	/** Last selected descendant ID retained independently for each page. */
	UPROPERTY(Transient) TMap<FName, FName> LastPageDescendantNavigationIds;
	/** Header that initiated the deferred composite navigation request. */
	UPROPERTY(Transient) TWeakObjectPtr<ULunarTabHeader> PendingHeaderNavigationSource;
	/** Resolved destination for deferred composite navigation. */
	UPROPERTY(Transient) TWeakObjectPtr<ULunarNavigableWidget> PendingHeaderNavigationTarget;

	/** Orientation currently represented by the generated hierarchy. */
	TEnumAsByte<EOrientation> AppliedOrientation = Orient_Horizontal;
	/** Page placement currently represented by the generated hierarchy. */
	ELunarTabPagePlacement AppliedPagePlacement = ELunarTabPagePlacement::Automatic;
	/** Whether the native generated layout has been created. */
	bool bNativeLayoutInitialized = false;
	/** Whether descriptors have been validated and generated controls initialized. */
	bool bTabsInitialized = false;
	/** Reentrancy guard for active-tab changes. */
	bool bActivationInProgress = false;
#if WITH_EDITOR
	/** Prevents recursive Designer preview rebuilds during property synchronization. */
	bool bDesignerPreviewRebuildInProgress = false;
	/** Selects the lightweight visibility-only path while a preview-selection property changes. */
	bool bDesignerPreviewSelectionChangeInProgress = false;
#endif

	/** Grants generated headers access to owner-controlled routing and activation. */
	friend class ULunarTabHeader;
};
