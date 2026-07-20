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
	 * Owner-facing outside-click seam for a full-screen catcher or popup host.
	 * Only the menu that owns the current top scope is allowed to close.
	 * @return True when this menu consumed the outside pointer and initiated closure.
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Context Menu|Pointer")
	bool HandleContextMenuOutsidePointer();

public:
	/** Restores this menu's last valid selection instead of resetting on every open. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Context Menu")
	bool bRestoreSelectionOnOpen = false;

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
	/** Tests a pointer coordinate against popup bounds. @param ScreenSpacePosition Screen-space pointer position. @return True when inside. */
	bool IsPointerInsidePopup(const FVector2D& ScreenSpacePosition) const;
	/** Tests whether a widget is inside this menu while its scope is closing. @param Candidate Candidate widget. @return True when contained. */
	bool IsWidgetInsideClosingMenu(const UWidget* Candidate) const;
	/** Binds active-scope observation. @param NavigationSubsystem Subsystem to observe. */
	void BindNavigationSubsystem(ULunarNavigationSubsystem* NavigationSubsystem);
	/** Removes active-scope observation and clears the weak binding. */
	void UnbindNavigationSubsystem();
	/** Finalizes menu state after its scope leaves the stack. @param NavigationSubsystem Subsystem that owned the scope. */
	void FinalizeContextMenuClosed(ULunarNavigationSubsystem* NavigationSubsystem);
	/** Enforces non-selectable, non-focusable container policies. */
	void ApplyRuntimePolicies();

private:
	/** Navigation subsystem currently observed for scope-stack changes. */
	TWeakObjectPtr<ULunarNavigationSubsystem> BoundNavigationSubsystem;
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
};
