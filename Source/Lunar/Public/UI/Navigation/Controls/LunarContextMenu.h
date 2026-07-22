// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarContextMenu.h
 * @brief Declares the non-selectable popup container used for nested navigation menus.
 * @ingroup LunarNavigationControls
 */

#pragma once

#include "CoreMinimal.h"
#include "UI/Navigation/Core/LunarNavigableWidget.h"
#include "UI/Navigation/Types/LunarNavigationTypes.h"
#include "LunarContextMenu.generated.h"

class ULunarNavigationScope;
class ULunarNavigationSubsystem;
class UWidget;
class FLunarContextMenuOutsideClickInputProcessor;

/**
 * Non-selectable popup container that owns one nested Lunar navigation scope.
 *
 * A submenu is another ULunarContextMenu. Opening it naturally pushes one more
 * scope, so the subsystem's normal Back policy closes only the top submenu.
 *
 * @ingroup LunarNavigationControls
 */
UCLASS(Blueprintable)
class LUNAR_API ULunarContextMenu : public ULunarNavigableWidget
{
	GENERATED_BODY()

public:
	/** Creates a non-focusable ContextMenu container. @param ObjectInitializer Unreal object initializer. */
	ULunarContextMenu(const FObjectInitializer& ObjectInitializer);

	/** Makes the popup visible and pushes its nested navigation scope. @return True when open or successfully opened. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Context Menu")
	bool OpenContextMenu();

	/** Closes this menu and any child submenu scopes above it. @return True when closed or closure was accepted. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Context Menu")
	bool CloseContextMenu();

	/** Returns whether this menu currently owns a scope in its local-player stack. @return True while open. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Context Menu")
	bool IsContextMenuOpen() const;

	/** Returns the runtime scope owned by this menu, if it has been created. @return Owned scope, or null. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Context Menu")
	ULunarNavigationScope* GetContextMenuScope() const;

	/**
	 * Assigns the visual subtree that participates in this menu's scope.
	 * Passing null restores the default of ContextMenuPopupWidget, then this widget.
	 * @param NewScopeRootWidget New scope root, or null for automatic resolution.
	 * @return True when the assignment is valid and was applied.
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Context Menu")
	bool SetContextMenuScopeRootWidget(UWidget* NewScopeRootWidget);

	/**
	 * Assigns the popup bounds used by native outside-pointer handling.
	 * The owner Blueprint remains solely responsible for the popup presentation.
	 * @param NewPopupWidget New popup-bounds widget, or null.
	 * @return True when the assignment is valid and was applied.
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Context Menu")
	bool SetContextMenuPopupWidget(UWidget* NewPopupWidget);

	/**
	 * Explicit outside-pointer dismissal seam. The top menu closes itself or its
	 * complete ContextMenu chain according to the root menu dismissal policy.
	 * @return True when this menu consumed the outside pointer and initiated closure.
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Context Menu|Pointer")
	bool HandleContextMenuOutsidePointer();

public:
	/** Restores this menu's last valid selection instead of resetting on every open. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Context Menu")
	bool bRestoreSelectionOnOpen = false;

	/** Outside pointer-down closes the complete chain when this menu is its root. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Context Menu|Dismissal")
	bool bCloseEntireMenuChainOnOutsidePointer = true;

	/** Closes this popup after the mouse leaves it for PointerLeaveCloseDelay seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Context Menu|Dismissal")
	bool bCloseOnPointerLeave = false;

	/** Seconds the mouse must remain outside before pointer-leave dismissal occurs. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Context Menu|Dismissal", meta = (ClampMin = "0.0", UIMin = "0.0", EditCondition = "bCloseOnPointerLeave"))
	float PointerLeaveCloseDelay = 0.15f;

	/** Scope policy; bRestoreLastSelection is derived from bRestoreSelectionOnOpen. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Context Menu")
	FLunarNavigationScopeSettings NavigationScopeSettings;

	/**
	 * Optional scope root inside this ContextMenu. When unset, the popup widget or
	 * this ContextMenu is used. The reference may also be assigned at runtime.
	 */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Lunar|UI|Context Menu", meta = (BindWidgetOptional))
	TObjectPtr<UWidget> ContextMenuScopeRootWidget;

	/** Optional visual popup bounds used to distinguish inside and outside clicks. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Lunar|UI|Context Menu", meta = (BindWidgetOptional))
	TObjectPtr<UWidget> ContextMenuPopupWidget;

	/** Runtime scope owned by this menu. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Lunar|UI|Context Menu")
	TObjectPtr<ULunarNavigationScope> NavigationScope;

	/** Broadcast after this menu has opened and pushed its scope. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Context Menu")
	FLunarContextMenuStateChangedSignature OnContextMenuOpened;

	/** Broadcast after this menu has closed and released its scope. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Context Menu")
	FLunarContextMenuStateChangedSignature OnContextMenuClosed;

protected:
	/** Applies runtime focus policy and binds navigation-state observation. */
	virtual void NativeConstruct() override;
	/** Closes and unbinds the menu during destruction. */
	virtual void NativeDestruct() override;
	/** Monitors owner visibility and scope lifetime. @param MyGeometry Cached widget geometry. @param InDeltaTime Elapsed seconds. */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	/** Synchronizes focus policy and popup bindings. */
	virtual void SynchronizeProperties() override;
	/** Closes on mouse presses outside the popup. @param InGeometry Cached widget geometry. @param InMouseEvent Mouse event. @return Slate handling reply. */
	virtual FReply NativeOnPreviewMouseButtonDown(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;
	/** Closes on touch presses outside the popup. @param InGeometry Cached widget geometry. @param InGestureEvent Touch event. @return Slate handling reply. */
	virtual FReply NativeOnTouchStarted(
		const FGeometry& InGeometry,
		const FPointerEvent& InGestureEvent) override;
private:
	/** Detects external scope-stack changes. @param PreviousScope Previously active scope. @param NewScope Newly active scope. */
	UFUNCTION()
	void HandleActiveScopeChanged(ULunarNavigationScope* PreviousScope, ULunarNavigationScope* NewScope);

	/** Resolves the owning LocalPlayer navigation subsystem. @return Navigation subsystem, or null. */
	ULunarNavigationSubsystem* ResolveContextMenuNavigationSubsystem() const;
	/** Resolves the configured scope root fallback chain. @return Root widget used by the nested scope. */
	UWidget* ResolveScopeRootWidget() const;
	/** Resolves the configured popup-bounds fallback. @return Popup widget, or null. */
	UWidget* ResolvePopupWidget() const;
	/** Tests ownership of a candidate widget. @param Candidate Candidate widget. @return True when it belongs to this menu subtree. */
	bool IsOwnedWidget(const UWidget* Candidate) const;
	/** Tests whether the owned scope is the current top scope. @param NavigationSubsystem Subsystem to inspect. @return True when topmost. */
	bool IsOurScopeTop(const ULunarNavigationSubsystem* NavigationSubsystem) const;
	/** Tests whether the active scope is this menu or a contiguous child ContextMenu. @param NavigationSubsystem Subsystem to inspect. @return True while this menu belongs to the active ContextMenu chain. */
	bool IsInActiveContextMenuChain(const ULunarNavigationSubsystem* NavigationSubsystem) const;
	/** Resolves the first contiguous ContextMenu in this scope chain. @return Root ContextMenu, or this menu. */
	ULunarContextMenu* ResolveContextMenuChainRoot() const;
	/** Resolves the nearest open ancestor menu whose popup contains the pointer. @param ScreenSpacePosition Screen-space pointer position. @return Hovered ancestor, or null. */
	ULunarContextMenu* ResolveAncestorMenuContainingPointer(const FVector2D& ScreenSpacePosition) const;
	/** Tests whether an eligible mouse-selectable Lunar item occupies a pointer coordinate in a menu. @param Menu Menu scope to inspect. @param ScreenSpacePosition Screen-space pointer position. @return True when a Lunar item is under the pointer. */
	bool IsPointerOverSelectableItemInMenu(const ULunarContextMenu* Menu, const FVector2D& ScreenSpacePosition) const;
	/** Routes presentation-only pointer hover to one eligible item in inactive ancestor menus. @param ScreenSpacePosition Screen-space pointer position. */
	void UpdateAncestorPointerHover(const FVector2D& ScreenSpacePosition);
	/** Clears presentation-only pointer hover previously routed to ancestor menus. */
	void ClearAncestorPointerHover();
	/** Closes the direct child branch above an ancestor menu. @param AncestorMenu Ancestor that must remain open. @return True when the child branch closed. */
	bool CloseDescendantMenusToAncestor(ULunarContextMenu* AncestorMenu);
	/** Closes the complete contiguous ContextMenu scope chain. @return True when closure was accepted. */
	bool CloseEntireContextMenuChain();
	/** Tests a pointer coordinate against popup bounds. @param ScreenSpacePosition Screen-space pointer position. @return True when inside. */
	bool IsPointerInsidePopup(const FVector2D& ScreenSpacePosition) const;
	/** Tests this popup and all active child ContextMenu popups as one hover region. @param ScreenSpacePosition Screen-space pointer position. @param NavigationSubsystem Subsystem containing the active chain. @return True when inside any popup in this menu's active subtree. */
	bool IsPointerInsideActiveContextMenuChain(const FVector2D& ScreenSpacePosition, const ULunarNavigationSubsystem* NavigationSubsystem) const;
	/** Tests whether a Slate pointer user belongs to this menu's LocalPlayer. @param SlateUserIndex Slate user index. @return True when owned or no Slate user is available. */
	bool IsPointerOwnedByLocalPlayer(uint32 SlateUserIndex) const;
	/** Tests whether a widget is inside this menu while its scope is closing. @param Candidate Candidate widget. @return True when contained. */
	bool IsWidgetInsideClosingMenu(const UWidget* Candidate) const;
	/** Binds active-scope observation. @param NavigationSubsystem Subsystem to observe. */
	void BindNavigationSubsystem(ULunarNavigationSubsystem* NavigationSubsystem);
	/** Removes active-scope observation and clears the weak binding. */
	void UnbindNavigationSubsystem();
	/** Registers global primary-pointer observation for compact popup dismissal. */
	void RegisterOutsideClickProcessor();
	/** Removes global primary-pointer observation. */
	void UnregisterOutsideClickProcessor();
	/** Handles a globally observed primary pointer-down. @param ScreenPosition Screen-space pointer position. @param SlateUserIndex Owning Slate user. @param bTouchEvent Whether the pointer is touch. @return True when this top menu closed and consumed the event. */
	bool HandleGlobalPointerDown(const FVector2D& ScreenPosition, uint32 SlateUserIndex, bool bTouchEvent);
	/** Tracks globally observed mouse movement for delayed popup-leave dismissal. @param ScreenPosition Screen-space pointer position. @param SlateUserIndex Owning Slate user. */
	void HandleGlobalPointerMove(const FVector2D& ScreenPosition, uint32 SlateUserIndex);
	/** Advances pending pointer-leave dismissal. @param DeltaTime Elapsed seconds. */
	void TickPointerLeaveDismissal(float DeltaTime);
	/** Clears all transient pointer-leave state. */
	void ResetPointerLeaveState();
	/** Finalizes menu state after its scope leaves the stack. @param NavigationSubsystem Subsystem that owned the scope. */
	void FinalizeContextMenuClosed(ULunarNavigationSubsystem* NavigationSubsystem);
	/** Enforces non-selectable, non-focusable container policies. */
	void ApplyRuntimePolicies();

private:
	/** Navigation subsystem currently observed for scope-stack changes. */
	TWeakObjectPtr<ULunarNavigationSubsystem> BoundNavigationSubsystem;
	/** Global pointer observer active only while this menu is open. */
	TSharedPtr<FLunarContextMenuOutsideClickInputProcessor> OutsideClickProcessor;
	/** Whether this menu currently owns an open scope. */
	bool bContextMenuOpen = false;
	/** Reentrancy guard for opening. */
	bool bOpeningContextMenu = false;
	/** Reentrancy guard for closure. */
	bool bClosingContextMenu = false;
	/** Deferred close request raised during opening. */
	bool bCloseRequestedWhileOpening = false;
	/** Suppresses normal closure callbacks during object teardown. */
	bool bTearingDownContextMenu = false;
	/** Whether the mouse has entered this popup since its current activation. */
	bool bPointerWasInsidePopup = false;
	/** Whether pointer-leave dismissal is counting down. */
	bool bPointerLeaveClosePending = false;
	/** Elapsed seconds in the current pointer-leave countdown. */
	float PointerLeaveCloseElapsed = 0.0f;

	friend class FLunarContextMenuOutsideClickInputProcessor;
};
