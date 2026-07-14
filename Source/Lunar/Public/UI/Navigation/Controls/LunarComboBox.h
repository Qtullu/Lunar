// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarComboBox.h
 * @brief Declares the data-backed Lunar ComboBox and its nested popup navigation behavior.
 * @ingroup LunarNavigationControls
 */

#pragma once

#include "CoreMinimal.h"
#include "UI/Navigation/Core/LunarNavigableWidget.h"
#include "LunarComboBox.generated.h"

class FLunarComboBoxOutsideClickInputProcessor;
class SImage;
class UImage;
class ULunarComboBox;
class ULunarComboBoxOptionItem;
class ULunarListView;
class ULunarNavigationScope;
class ULunarNavigationSubsystem;
class UTextBlock;
class UWidget;

/**
 * Data-backed ComboBox whose popup owns a nested navigation scope and virtualized option ListView.
 *
 * Opening creates a temporary selection inside the popup. Only activation commits that value to
 * SelectedOptionId; cancellation closes the nested scope without changing the committed value.
 *
 * @ingroup LunarNavigationControls
 */
UCLASS(Blueprintable)
class LUNAR_API ULunarComboBox : public ULunarNavigableWidget
{
	GENERATED_BODY()

public:
	/** Creates a ComboBox with closed-popup navigation policies. @param ObjectInitializer Unreal object initializer. */
	ULunarComboBox(const FObjectInitializer& ObjectInitializer);

	/** Replaces the option data set and rebuilds stable lookup adapters. @param NewOptions New option descriptors. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|ComboBox")
	void SetOptions(const TArray<FLunarComboBoxOption>& NewOptions);

	/** Programmatically commits an existing option ID. @param OptionId Existing option ID, or NAME_None to clear. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|ComboBox")
	void SetSelectedOptionById(FName OptionId);

	/** Returns the committed option ID. @return Stable selected ID, or NAME_None. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation|ComboBox")
	FName GetSelectedOptionId() const { return SelectedOptionId; }

	/** Opens the popup and pushes its nested navigation scope. @return True when open or successfully opened. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|ComboBox")
	bool OpenComboBox();

	/** Closes the popup without committing its temporary selection. @return True when closed or close was accepted. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|ComboBox")
	bool CloseComboBox();

	/** Tests whether the popup currently owns an open scope. @return True while open. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation|ComboBox")
	bool IsComboBoxOpen() const { return bComboBoxOpen; }

	/** Updates the search query and filtered option list. @param NewSearchQuery New localized query text. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|ComboBox|Search")
	void SetSearchQuery(const FText& NewSearchQuery);

	/**
	 * Tests an option against a search query.
	 * @param Option Candidate option.
	 * @param Query Search query.
	 * @return True when the option remains visible.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Lunar|UI|Navigation|ComboBox|Search")
	bool DoesOptionMatchSearch(const FLunarComboBoxOption& Option, const FText& Query) const;
	/** Default culture-aware case-insensitive substring matcher. @param Option Candidate option. @param Query Search query. @return True on match. */
	virtual bool DoesOptionMatchSearch_Implementation(const FLunarComboBoxOption& Option, const FText& Query) const;

	/** Authored option descriptors, each identified by a unique, non-empty stable ID. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|ComboBox")
	TArray<FLunarComboBoxOption> Options;

	/** Stable ID of the committed value. Popup navigation changes only the private temporary ID. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|ComboBox")
	FName SelectedOptionId = NAME_None;

	/** Enables query-based option filtering inside the popup. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|ComboBox|Search")
	bool bEnableSearch = false;

	/** Disabled by default: the query is cleared before every popup open. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|ComboBox|Search")
	bool bRestoreSearchQuery = false;

	/** Current popup search query. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "Lunar|UI|Navigation|ComboBox|Search")
	FText SearchQuery;

	/** Settings copied into the popup's nested scope. The option ListView remains the default initial item. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|ComboBox")
	FLunarNavigationScopeSettings PopupScopeSettings;

	/** Broadcast when the committed option ID changes. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation|ComboBox")
	FLunarComboBoxSelectionChangedSignature OnSelectionChanged;

	/** Broadcast after the popup scope has opened. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation|ComboBox")
	FLunarNavigableWidgetEventSignature OnComboBoxOpened;

	/** Broadcast after the popup scope has closed. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation|ComboBox")
	FLunarNavigableWidgetEventSignature OnComboBoxClosed;

protected:
	/** Visible popup root whose descendants form the nested scope. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Lunar|UI|Navigation|ComboBox|Owner Bindings")
	TObjectPtr<UWidget> PopupRootWidget;

	/** Required virtualized option-list owner binding. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Lunar|UI|Navigation|ComboBox|Owner Bindings")
	TObjectPtr<ULunarListView> OptionsListView;

	/** Optional owner-created image receiving the resolved popup brush. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Lunar|UI|Navigation|ComboBox|Owner Bindings")
	TObjectPtr<UImage> PopupBackgroundImage;

	/** Optional owner-created text receiving the committed option's display text. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Lunar|UI|Navigation|ComboBox|Owner Bindings")
	TObjectPtr<UTextBlock> SelectedOptionTextBlock;

	/** Optional navigable search control. It participates in the popup scope but is never the default item. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Lunar|UI|Navigation|ComboBox|Owner Bindings")
	TObjectPtr<ULunarNavigableWidget> SearchFieldNavigationWidget;

	/** Builds the compact ComboBox Slate presentation. @return Specialized Slate presentation. */
	virtual TSharedPtr<SWidget> RebuildLunarSpecializedPresentation() override;
	/** Releases generated Slate presentation resources. @param bReleaseChildren Whether child resources must also be released. */
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	/** Synchronizes options, bindings, selection text, and resolved style. */
	virtual void SynchronizeProperties() override;
	/** Binds popup owner controls and culture-change notifications. */
	virtual void NativeConstruct() override;
	/** Closes the popup and releases runtime delegates during destruction. */
	virtual void NativeDestruct() override;
	/** Tests whether this ComboBox handles the routed Lunar action. @param ActionContext Routed action context. @return True when supported. */
	virtual bool NativeCanHandleLunarAction(const FLunarUIActionContext& ActionContext) const override;
	/** Opens, closes, or delegates actions to the popup. @param ActionContext Routed action context. @return Lunar routing result. */
	virtual ELunarUIActionResult NativeHandleLunarAction(const FLunarUIActionContext& ActionContext) override;
	/** Tests whether the closed ComboBox may be activated. @return True when activation is allowed. */
	virtual bool NativeCanActivateLunarWidget() const override;
	/** Opens the ComboBox in response to standard activation. */
	virtual void NativeOnLunarActivated() override;
	/** Returns the selected option label for accessibility. @return Localized accessible value. */
	virtual FText NativeGetLunarAccessibleValueText() const override;
	/** Resolves common and ComboBox-specific styles. @param OutStyle Resolved common patch. @param OutError Actionable failure text. @return True on success. */
	virtual bool ResolveCommonStylePatch(FLunarCommonStylePatch& OutStyle, FString& OutError) const override;
	/** Applies common style and schedules ComboBox-specific presentation. @param ResolvedStyle Resolved common style patch. */
	virtual void ApplyResolvedCommonStyle(const FLunarCommonStylePatch& ResolvedStyle) override;

private:
	/** Finds an authored option by stable ID. @param OptionId Stable option ID. @return Option descriptor, or null. */
	const FLunarComboBoxOption* FindOptionById(FName OptionId) const;
	/** Tests whether an option is a valid enabled selection target. @param OptionId Stable option ID. @return True when eligible. */
	bool IsOptionEligible(FName OptionId) const;
	/** Rebuilds UObject ListView adapters and validates stable option IDs. */
	void RebuildOptionAdapters();
	/** Rebuilds the search-filtered ListView data. @param bPreserveTemporarySelection Preserve popup selection when possible. @param bUseListViewRestoration Let ListView choose a deterministic fallback. */
	void RebuildFilteredOptions(bool bPreserveTemporarySelection, bool bUseListViewRestoration = false);
	/** Seeds temporary popup selection from the committed value or first eligible option. */
	void SetTemporarySelectionToCommittedOrFirstEligible();
	/** Commits the popup's temporary selection and closes the popup. */
	void CommitTemporarySelection();
	/** Applies a committed value. @param NewOptionId New stable ID, or NAME_None. @param bNotify Whether to broadcast the change. */
	void ApplyCommittedSelection(FName NewOptionId, bool bNotify);
	/** Updates compact display text from the committed option. */
	void UpdateCommittedPresentation();
	/** Applies arrow, popup, row, and scroll-bar style fields. */
	void ApplySpecializedStyle();
	/** Reports newly introduced option configuration errors. @param CurrentErrors Current deduplication keys. */
	void ReportConfigurationErrors(const TSet<FString>& CurrentErrors);
	/** Resolves the owning LocalPlayer navigation subsystem. @return Navigation subsystem, or null. */
	ULunarNavigationSubsystem* GetNavigationSubsystem() const;
	/** Finalizes visual and delegate state after the popup scope closes. */
	void FinalizePopupClosed();
	/** Registers the Slate preprocessor used for clicks outside the popup. */
	void RegisterOutsideClickProcessor();
	/** Unregisters the outside-click Slate preprocessor. */
	void UnregisterOutsideClickProcessor();
	/** Handles a global pointer press before regular widget routing. @param ScreenPosition Screen-space pointer position. @param SlateUserIndex Originating Slate user. @return True when the popup was closed. */
	bool HandleGlobalPointerDown(const FVector2D& ScreenPosition, uint32 SlateUserIndex);
	/** Reapplies culture-sensitive filtering after the active culture changes. */
	void HandleCultureChanged();

	/** Tracks temporary popup selection. @param PreviousItemId Previous item ID. @param NewItemId New item ID. */
	UFUNCTION()
	void HandleListActiveItemChanged(FName PreviousItemId, FName NewItemId);

	/** Commits the active option when the nested ListView activates. */
	UFUNCTION()
	void HandleListActivated();

	/** Detects external scope-stack closure. @param PreviousScope Previously active scope. @param NewScope Newly active scope. */
	UFUNCTION()
	void HandleActiveScopeChanged(ULunarNavigationScope* PreviousScope, ULunarNavigationScope* NewScope);

	/** Internal UObject adapters exposed to the nested ListView. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<ULunarComboBoxOptionItem>> OptionItems;

	/** Runtime nested scope owned by the open popup. */
	UPROPERTY(Transient)
	TObjectPtr<ULunarNavigationScope> PopupScope;

	/** Last successfully resolved ComboBox-specific style. */
	UPROPERTY(Transient)
	FLunarComboBoxStylePatch ResolvedComboBoxStyle;

	/** Lookup from stable option ID to authored descriptor index. */
	TMap<FName, int32> OptionIndexById;
	/** Deduplication keys for option configuration errors already reported. */
	TSet<FString> ReportedConfigurationErrors;
	/** Generated compact arrow image. */
	TSharedPtr<SImage> ArrowImage;
	/** Global pointer input preprocessor installed while the popup is open. */
	TSharedPtr<FLunarComboBoxOutsideClickInputProcessor> OutsideClickProcessor;
	/** Handle for the active-culture change delegate. */
	FDelegateHandle CultureChangedHandle;
	/** Owner popup brush captured before resolved style is applied. */
	FSlateBrush PopupBaselineBrush;
	/** Uncommitted option ID currently highlighted in the popup. */
	FName TemporaryOptionId = NAME_None;
	/** Whether PopupBaselineBrush contains a captured owner brush. */
	bool bHasPopupBaselineBrush = false;
	/** Whether the nested popup scope is currently open. */
	bool bComboBoxOpen = false;
	/** Reentrancy guard for popup opening. */
	bool bOpeningComboBox = false;
	/** Reentrancy guard for popup closure. */
	bool bClosingComboBox = false;
	/** Deferred close request raised while opening is in progress. */
	bool bCloseRequestedWhileOpening = false;
	/** Deferred close request raised while another close operation is in progress. */
	bool bCloseRequestedWhileClosing = false;

	/** Grants the Slate input preprocessor access to the guarded outside-click handler. */
	friend class FLunarComboBoxOutsideClickInputProcessor;
	/** Grants option adapters read-only access to their owning descriptor data. */
	friend class ULunarComboBoxOptionItem;
};
