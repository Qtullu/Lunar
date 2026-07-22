// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarContextMenu.cpp
 * @brief Implements nested scope ownership, outside-pointer dismissal, and popup lifetime.
 * @ingroup LunarNavigationControls
 */

#include "UI/Navigation/Controls/LunarContextMenu.h"

#include "Components/PanelWidget.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Application/IInputProcessor.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "UI/Navigation/Core/LunarNavigationScope.h"
#include "UI/Navigation/Core/LunarNavigationSubsystem.h"

/** Private log channel for ContextMenu diagnostics. */
DEFINE_LOG_CATEGORY_STATIC(LogLunarContextMenu, Log, All);

/** Private helpers for walking UMG widget ownership across panel and outer boundaries. */
namespace LunarContextMenu_Private
{
	/** Resolves the structural parent used by popup containment checks. @param Widget Source widget. @return Parent widget, or null. */
	const UWidget* GetStructuralParent(const UWidget* Widget)
	{
		if (!Widget)
		{
			return nullptr;
		}
		if (const UWidget* Parent = Widget->GetParent())
		{
			return Parent;
		}

		for (const UObject* Outer = Widget->GetOuter(); Outer; Outer = Outer->GetOuter())
		{
			if (const UWidget* OuterWidget = Cast<UWidget>(Outer))
			{
				return OuterWidget;
			}
		}
		return nullptr;
	}

	/** Tests ancestry while protecting against malformed ownership cycles. @param Candidate Candidate descendant. @param Ancestor Expected ancestor. @return True when contained or identical. */
	bool IsDescendantOrSelf(const UWidget* Candidate, const UWidget* Ancestor)
	{
		if (!Candidate || !Ancestor)
		{
			return false;
		}

		TSet<TObjectKey<UWidget>> Visited;
		for (const UWidget* Current = Candidate; Current; Current = GetStructuralParent(Current))
		{
			const TObjectKey<UWidget> CurrentKey(const_cast<UWidget*>(Current));
			if (Visited.Contains(CurrentKey))
			{
				return false;
			}
			Visited.Add(CurrentKey);

			if (Current == Ancestor)
			{
				return true;
			}
		}
		return false;
	}
}

/** Observes global primary pointer-down so a compact ContextMenu can dismiss outside its own geometry. */
class FLunarContextMenuOutsideClickInputProcessor final : public IInputProcessor
{
public:
	/** @param InOwner Weak ContextMenu owner receiving pointer-down callbacks. */
	explicit FLunarContextMenuOutsideClickInputProcessor(ULunarContextMenu* InOwner)
		: Owner(InOwner)
	{
	}

	/** IInputProcessor tick; no work is required. */
	virtual void Tick(float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override
	{
	}

	/** Routes mouse movement for optional delayed pointer-leave dismissal. */
	virtual bool HandleMouseMoveEvent(
		FSlateApplication& SlateApp,
		const FPointerEvent& MouseEvent) override
	{
		if (Owner.IsValid() && !MouseEvent.IsTouchEvent())
		{
			Owner->HandleGlobalPointerMove(
				MouseEvent.GetScreenSpacePosition(),
				MouseEvent.GetUserIndex());
		}
		return false;
	}

	/** Routes primary mouse and touch pointer-down before normal widget routing. */
	virtual bool HandleMouseButtonDownEvent(
		FSlateApplication& SlateApp,
		const FPointerEvent& MouseEvent) override
	{
		const bool bPrimaryPointer = MouseEvent.IsTouchEvent()
			? MouseEvent.GetPointerIndex() == 0
			: MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton;
		if (Owner.IsValid() && bPrimaryPointer)
		{
			return Owner->HandleGlobalPointerDown(
				MouseEvent.GetScreenSpacePosition(),
				MouseEvent.GetUserIndex(),
				MouseEvent.IsTouchEvent());
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
		return TEXT("LunarContextMenuOutsideClick");
	}

private:
	/** Weak owner prevents Slate from extending the UWidget lifetime. */
	TWeakObjectPtr<ULunarContextMenu> Owner;
};

ULunarContextMenu::ULunarContextMenu(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// The menu panel owns descendants and a nested scope; it is never itself a selection target.
	NavigationScopeSettings.bRestoreLastSelection = false;
	ApplyRuntimePolicies();
}

bool ULunarContextMenu::OpenContextMenu()
{
	if (bTearingDownContextMenu)
	{
		return false;
	}
	if (bClosingContextMenu)
	{
		return false;
	}
	if (bContextMenuOpen)
	{
		return true;
	}
	if (bOpeningContextMenu)
	{
		return true;
	}

	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	ULunarNavigationSubsystem* NavigationSubsystem = ResolveContextMenuNavigationSubsystem();
	UWidget* ScopeRootWidget = ResolveScopeRootWidget();
	if (!LocalPlayer
		|| !NavigationSubsystem
		|| !NavigationSubsystem->GetActiveNavigationScope()
		|| !IsValid(ScopeRootWidget)
		|| !IsOwnedWidget(ScopeRootWidget))
	{
		return false;
	}

	BindNavigationSubsystem(NavigationSubsystem);
	if (!NavigationScope
		|| (NavigationScope->GetOwningLocalPlayer()
			&& NavigationScope->GetOwningLocalPlayer() != LocalPlayer))
	{
		NavigationScope = NewObject<ULunarNavigationScope>(this);
	}

	FLunarNavigationScopeSettings ScopeSettings = NavigationScopeSettings;
	ScopeSettings.bRestoreLastSelection = bRestoreSelectionOnOpen;
	NavigationScope->RootWidget = ScopeRootWidget;
	NavigationScope->Settings = ScopeSettings;

	const ESlateVisibility PreviousVisibility = GetVisibility();
	// The ContextMenu wrapper may cover much more space than its compact popup.
	// Keep only its children hit-testable so transparent wrapper geometry cannot
	// suppress hover on an open ancestor menu.
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	RefreshVisualState();

	bOpeningContextMenu = true;
	bCloseRequestedWhileOpening = false;
	const bool bPushed = NavigationSubsystem->PushNavigationScope(NavigationScope);
	bOpeningContextMenu = false;
	if (!bPushed || !NavigationSubsystem->GetNavigationScopeStack().Contains(NavigationScope))
	{
		bCloseRequestedWhileOpening = false;
		SetVisibility(PreviousVisibility);
		RefreshVisualState();
		return false;
	}

	bContextMenuOpen = true;
	ResetPointerLeaveState();
	RegisterOutsideClickProcessor();
	OnContextMenuOpened.Broadcast(this);

	const bool bShouldClose = bCloseRequestedWhileOpening;
	bCloseRequestedWhileOpening = false;
	if (bShouldClose && bContextMenuOpen)
	{
		CloseContextMenu();
	}
	return bContextMenuOpen;
}

bool ULunarContextMenu::CloseContextMenu()
{
	if (bOpeningContextMenu)
	{
		bCloseRequestedWhileOpening = true;
		return true;
	}

	ULunarNavigationSubsystem* NavigationSubsystem = ResolveContextMenuNavigationSubsystem();
	if ((!NavigationSubsystem
		|| (NavigationScope && !NavigationSubsystem->GetNavigationScopeStack().Contains(NavigationScope)))
		&& BoundNavigationSubsystem.IsValid())
	{
		NavigationSubsystem = BoundNavigationSubsystem.Get();
	}
	const bool bScopeInStack = NavigationSubsystem
		&& NavigationScope
		&& NavigationSubsystem->GetNavigationScopeStack().Contains(NavigationScope);
	if (!bContextMenuOpen && !bScopeInStack)
	{
		UnregisterOutsideClickProcessor();
		return true;
	}

	if (bScopeInStack && !NavigationSubsystem->PopNavigationScope(NavigationScope))
	{
		return false;
	}

	if (bContextMenuOpen)
	{
		FinalizeContextMenuClosed(NavigationSubsystem);
	}
	return true;
}

bool ULunarContextMenu::IsContextMenuOpen() const
{
	return bContextMenuOpen;
}

ULunarNavigationScope* ULunarContextMenu::GetContextMenuScope() const
{
	return NavigationScope;
}

bool ULunarContextMenu::SetContextMenuScopeRootWidget(UWidget* NewScopeRootWidget)
{
	if (bTearingDownContextMenu
		|| bContextMenuOpen
		|| bOpeningContextMenu
		|| (NewScopeRootWidget && !IsOwnedWidget(NewScopeRootWidget)))
	{
		return false;
	}
	if (ContextMenuScopeRootWidget != NewScopeRootWidget)
	{
		ContextMenuScopeRootWidget = NewScopeRootWidget;
	}
	return true;
}

bool ULunarContextMenu::SetContextMenuPopupWidget(UWidget* NewPopupWidget)
{
	if (bTearingDownContextMenu
		|| bContextMenuOpen
		|| bOpeningContextMenu
		|| (NewPopupWidget && !IsOwnedWidget(NewPopupWidget)))
	{
		return false;
	}
	if (ContextMenuPopupWidget != NewPopupWidget)
	{
		ContextMenuPopupWidget = NewPopupWidget;
	}
	return true;
}

bool ULunarContextMenu::HandleContextMenuOutsidePointer()
{
	if (!bContextMenuOpen
		|| !IsOurScopeTop(ResolveContextMenuNavigationSubsystem()))
	{
		return false;
	}

	ULunarContextMenu* ChainRoot = ResolveContextMenuChainRoot();
	return ChainRoot && ChainRoot->bCloseEntireMenuChainOnOutsidePointer
		? CloseEntireContextMenuChain()
		: CloseContextMenu();
}

void ULunarContextMenu::NativeConstruct()
{
	bTearingDownContextMenu = false;
	ApplyRuntimePolicies();
	Super::NativeConstruct();
	BindNavigationSubsystem(ResolveContextMenuNavigationSubsystem());
	if (!bContextMenuOpen && !bOpeningContextMenu)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		RefreshVisualState();
	}
}

void ULunarContextMenu::NativeDestruct()
{
	bTearingDownContextMenu = true;
	if (!CloseContextMenu())
	{
		UE_LOG(
			LogLunarContextMenu,
			Warning,
			TEXT("Unable to pop ContextMenu scope while destructing '%s'."),
			*GetPathName());
	}
	UnregisterOutsideClickProcessor();
	UnbindNavigationSubsystem();
	Super::NativeDestruct();
}

void ULunarContextMenu::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (!bContextMenuOpen || bClosingContextMenu)
	{
		return;
	}

	ULunarNavigationSubsystem* NavigationSubsystem = BoundNavigationSubsystem.Get();
	if (!NavigationSubsystem)
	{
		NavigationSubsystem = ResolveContextMenuNavigationSubsystem();
	}
	if (!NavigationSubsystem
		|| !NavigationScope
		|| !NavigationSubsystem->GetNavigationScopeStack().Contains(NavigationScope))
	{
		FinalizeContextMenuClosed(NavigationSubsystem);
		return;
	}

	TickPointerLeaveDismissal(InDeltaTime);
}

void ULunarContextMenu::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	PointerLeaveCloseDelay = FMath::IsFinite(PointerLeaveCloseDelay)
		? FMath::Max(0.0f, PointerLeaveCloseDelay)
		: 0.15f;
	if (!bCloseOnPointerLeave)
	{
		ResetPointerLeaveState();
	}
	ApplyRuntimePolicies();
}

FReply ULunarContextMenu::NativeOnPreviewMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (!InMouseEvent.IsTouchEvent()
		&& bContextMenuOpen
		&& !IsPointerInsidePopup(InMouseEvent.GetScreenSpacePosition())
		&& HandleContextMenuOutsidePointer())
	{
		return FReply::Handled();
	}
	return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
}

FReply ULunarContextMenu::NativeOnTouchStarted(
	const FGeometry& InGeometry,
	const FPointerEvent& InGestureEvent)
{
	if (InGestureEvent.GetPointerIndex() == 0
		&& bContextMenuOpen
		&& !IsPointerInsidePopup(InGestureEvent.GetScreenSpacePosition())
		&& HandleContextMenuOutsidePointer())
	{
		return FReply::Handled();
	}
	return Super::NativeOnTouchStarted(InGeometry, InGestureEvent);
}

void ULunarContextMenu::HandleActiveScopeChanged(
	ULunarNavigationScope* PreviousScope,
	ULunarNavigationScope* NewScope)
{
	ULunarNavigationSubsystem* NavigationSubsystem = BoundNavigationSubsystem.Get();
	if (!bContextMenuOpen || !NavigationScope || !NavigationSubsystem)
	{
		return;
	}
	if (!NavigationSubsystem->GetNavigationScopeStack().Contains(NavigationScope))
	{
		FinalizeContextMenuClosed(NavigationSubsystem);
		return;
	}
	if (!IsInActiveContextMenuChain(NavigationSubsystem))
	{
		ResetPointerLeaveState();
	}
}

ULunarNavigationSubsystem* ULunarContextMenu::ResolveContextMenuNavigationSubsystem() const
{
	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		return LocalPlayer->GetSubsystem<ULunarNavigationSubsystem>();
	}
	return nullptr;
}

UWidget* ULunarContextMenu::ResolveScopeRootWidget() const
{
	if (IsValid(ContextMenuScopeRootWidget))
	{
		return ContextMenuScopeRootWidget;
	}
	if (IsValid(ContextMenuPopupWidget))
	{
		return ContextMenuPopupWidget;
	}
	return const_cast<ULunarContextMenu*>(this);
}

UWidget* ULunarContextMenu::ResolvePopupWidget() const
{
	if (IsValid(ContextMenuPopupWidget))
	{
		return ContextMenuPopupWidget;
	}
	if (IsValid(ContextMenuScopeRootWidget))
	{
		return ContextMenuScopeRootWidget;
	}
	return const_cast<ULunarContextMenu*>(this);
}

bool ULunarContextMenu::IsOwnedWidget(const UWidget* Candidate) const
{
	return LunarContextMenu_Private::IsDescendantOrSelf(Candidate, this);
}

bool ULunarContextMenu::IsOurScopeTop(const ULunarNavigationSubsystem* NavigationSubsystem) const
{
	return NavigationSubsystem
		&& NavigationScope
		&& NavigationSubsystem->GetActiveNavigationScope() == NavigationScope;
}

bool ULunarContextMenu::IsInActiveContextMenuChain(
	const ULunarNavigationSubsystem* NavigationSubsystem) const
{
	if (!NavigationSubsystem || !NavigationScope)
	{
		return false;
	}

	ULunarNavigationScope* Scope = NavigationSubsystem->GetActiveNavigationScope();
	while (Scope)
	{
		if (Scope == NavigationScope)
		{
			return true;
		}

		const ULunarContextMenu* ChildMenu = Cast<ULunarContextMenu>(Scope->GetOuter());
		if (!ChildMenu || !ChildMenu->bContextMenuOpen)
		{
			return false;
		}
		Scope = Scope->GetParentScope();
	}
	return false;
}

ULunarContextMenu* ULunarContextMenu::ResolveContextMenuChainRoot() const
{
	ULunarContextMenu* ChainRoot = const_cast<ULunarContextMenu*>(this);
	ULunarNavigationScope* ParentScope = NavigationScope
		? NavigationScope->GetParentScope()
		: nullptr;
	while (ParentScope)
	{
		ULunarContextMenu* ParentMenu = Cast<ULunarContextMenu>(ParentScope->GetOuter());
		if (!ParentMenu || !ParentMenu->bContextMenuOpen)
		{
			break;
		}
		ChainRoot = ParentMenu;
		ParentScope = ParentScope->GetParentScope();
	}
	return ChainRoot;
}

ULunarContextMenu* ULunarContextMenu::ResolveAncestorMenuContainingPointer(
	const FVector2D& ScreenSpacePosition) const
{
	ULunarNavigationScope* Scope = NavigationScope
		? NavigationScope->GetParentScope()
		: nullptr;
	while (Scope)
	{
		ULunarContextMenu* AncestorMenu = Cast<ULunarContextMenu>(Scope->GetOuter());
		if (!AncestorMenu || !AncestorMenu->bContextMenuOpen)
		{
			break;
		}
		if (AncestorMenu->IsPointerInsidePopup(ScreenSpacePosition))
		{
			return AncestorMenu;
		}
		Scope = Scope->GetParentScope();
	}
	return nullptr;
}

bool ULunarContextMenu::IsPointerOverSelectableItemInMenu(
	const ULunarContextMenu* Menu,
	const FVector2D& ScreenSpacePosition) const
{
	if (!Menu || !Menu->NavigationScope)
	{
		return false;
	}

	for (const ULunarNavigableWidget* Widget : Menu->NavigationScope->GetRegisteredWidgets())
	{
		if (!IsValid(Widget)
			|| !Widget->CanReceiveLunarSelection()
			|| !Widget->IsMouseInputAllowed())
		{
			continue;
		}

		const FGeometry& WidgetGeometry = Widget->GetCachedGeometry();
		if (!WidgetGeometry.GetLocalSize().IsNearlyZero()
			&& WidgetGeometry.IsUnderLocation(ScreenSpacePosition))
		{
			return true;
		}
	}
	return false;
}

void ULunarContextMenu::UpdateAncestorPointerHover(
	const FVector2D& ScreenSpacePosition)
{
	bool bTargetResolved = false;
	ULunarNavigationScope* Scope = NavigationScope
		? NavigationScope->GetParentScope()
		: nullptr;
	while (Scope)
	{
		ULunarContextMenu* AncestorMenu = Cast<ULunarContextMenu>(Scope->GetOuter());
		if (!AncestorMenu || !AncestorMenu->bContextMenuOpen)
		{
			break;
		}

		for (ULunarNavigableWidget* Widget : Scope->GetRegisteredWidgets())
		{
			bool bShouldHover = false;
			if (!bTargetResolved
				&& IsValid(Widget)
				&& Widget->CanReceiveLunarSelection()
				&& Widget->IsMouseInputAllowed())
			{
				const FGeometry& WidgetGeometry = Widget->GetCachedGeometry();
				bShouldHover = !WidgetGeometry.GetLocalSize().IsNearlyZero()
					&& WidgetGeometry.IsUnderLocation(ScreenSpacePosition);
				bTargetResolved = bShouldHover;
			}
			if (IsValid(Widget))
			{
				Widget->SetContextMenuPointerHovered(bShouldHover);
			}
		}
		Scope = Scope->GetParentScope();
	}
}

void ULunarContextMenu::ClearAncestorPointerHover()
{
	ULunarNavigationScope* Scope = NavigationScope
		? NavigationScope->GetParentScope()
		: nullptr;
	while (Scope)
	{
		ULunarContextMenu* AncestorMenu = Cast<ULunarContextMenu>(Scope->GetOuter());
		if (!AncestorMenu)
		{
			break;
		}
		for (ULunarNavigableWidget* Widget : Scope->GetRegisteredWidgets())
		{
			if (IsValid(Widget))
			{
				Widget->SetContextMenuPointerHovered(false);
			}
		}
		Scope = Scope->GetParentScope();
	}
}

bool ULunarContextMenu::CloseDescendantMenusToAncestor(
	ULunarContextMenu* AncestorMenu)
{
	if (!AncestorMenu || !NavigationScope || !AncestorMenu->NavigationScope)
	{
		return false;
	}

	ULunarNavigationScope* DirectChildScope = NavigationScope;
	while (ULunarNavigationScope* ParentScope = DirectChildScope->GetParentScope())
	{
		if (ParentScope == AncestorMenu->NavigationScope)
		{
			if (ULunarContextMenu* DirectChildMenu =
				Cast<ULunarContextMenu>(DirectChildScope->GetOuter()))
			{
				return DirectChildMenu->CloseContextMenu();
			}
			return false;
		}
		DirectChildScope = ParentScope;
	}
	return false;
}

bool ULunarContextMenu::CloseEntireContextMenuChain()
{
	if (ULunarContextMenu* ChainRoot = ResolveContextMenuChainRoot())
	{
		return ChainRoot->CloseContextMenu();
	}
	return false;
}

bool ULunarContextMenu::IsPointerInsidePopup(const FVector2D& ScreenSpacePosition) const
{
	const UWidget* PopupWidget = ResolvePopupWidget();
	if (!IsValid(PopupWidget))
	{
		return true;
	}

	const FGeometry& PopupGeometry = PopupWidget->GetCachedGeometry();
	if (PopupGeometry.GetLocalSize().IsNearlyZero())
	{
		// Do not close from stale pre-layout geometry.
		return true;
	}
	return PopupGeometry.IsUnderLocation(ScreenSpacePosition);
}

bool ULunarContextMenu::IsPointerInsideActiveContextMenuChain(
	const FVector2D& ScreenSpacePosition,
	const ULunarNavigationSubsystem* NavigationSubsystem) const
{
	if (!IsInActiveContextMenuChain(NavigationSubsystem))
	{
		return false;
	}

	ULunarNavigationScope* Scope = NavigationSubsystem->GetActiveNavigationScope();
	while (Scope)
	{
		const ULunarContextMenu* Menu = Cast<ULunarContextMenu>(Scope->GetOuter());
		if (!Menu || !Menu->bContextMenuOpen)
		{
			return false;
		}
		if (Menu->IsPointerInsidePopup(ScreenSpacePosition))
		{
			return true;
		}
		if (Scope == NavigationScope)
		{
			break;
		}
		Scope = Scope->GetParentScope();
	}
	return false;
}

bool ULunarContextMenu::IsPointerOwnedByLocalPlayer(const uint32 SlateUserIndex) const
{
	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (const TSharedPtr<FSlateUser> SlateUser = LocalPlayer->GetSlateUser())
		{
			return SlateUser->GetUserIndex() == SlateUserIndex;
		}
	}
	return true;
}

bool ULunarContextMenu::IsWidgetInsideClosingMenu(const UWidget* Candidate) const
{
	const UWidget* ScopeRootWidget = ResolveScopeRootWidget();
	return Candidate && ScopeRootWidget
		&& LunarContextMenu_Private::IsDescendantOrSelf(Candidate, ScopeRootWidget);
}

void ULunarContextMenu::BindNavigationSubsystem(ULunarNavigationSubsystem* NavigationSubsystem)
{
	if (BoundNavigationSubsystem.Get() == NavigationSubsystem)
	{
		return;
	}
	UnbindNavigationSubsystem();
	if (NavigationSubsystem)
	{
		NavigationSubsystem->OnActiveScopeChanged.AddUniqueDynamic(
			this,
			&ULunarContextMenu::HandleActiveScopeChanged);
		BoundNavigationSubsystem = NavigationSubsystem;
	}
}

void ULunarContextMenu::UnbindNavigationSubsystem()
{
	if (ULunarNavigationSubsystem* NavigationSubsystem = BoundNavigationSubsystem.Get())
	{
		NavigationSubsystem->OnActiveScopeChanged.RemoveDynamic(
			this,
			&ULunarContextMenu::HandleActiveScopeChanged);
	}
	BoundNavigationSubsystem.Reset();
}

void ULunarContextMenu::RegisterOutsideClickProcessor()
{
	if (OutsideClickProcessor.IsValid()
		|| !FSlateApplication::IsInitialized())
	{
		return;
	}

	OutsideClickProcessor =
		MakeShared<FLunarContextMenuOutsideClickInputProcessor>(this);
	if (!FSlateApplication::Get().RegisterInputPreProcessor(
		OutsideClickProcessor))
	{
		OutsideClickProcessor.Reset();
	}
}

void ULunarContextMenu::UnregisterOutsideClickProcessor()
{
	ResetPointerLeaveState();
	ClearAncestorPointerHover();
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

bool ULunarContextMenu::HandleGlobalPointerDown(
	const FVector2D& ScreenPosition,
	const uint32 SlateUserIndex,
	const bool bTouchEvent)
{
	if (!bContextMenuOpen || bClosingContextMenu)
	{
		return false;
	}

	ULunarNavigationSubsystem* NavigationSubsystem =
		ResolveContextMenuNavigationSubsystem();
	if (!IsOurScopeTop(NavigationSubsystem)
		|| !IsPointerOwnedByLocalPlayer(SlateUserIndex))
	{
		return false;
	}

	if (IsPointerInsidePopup(ScreenPosition))
	{
		if (!bTouchEvent && bCloseOnPointerLeave)
		{
			bPointerWasInsidePopup = true;
			bPointerLeaveClosePending = false;
			PointerLeaveCloseElapsed = 0.0f;
		}
		return false;
	}
	if (ULunarContextMenu* AncestorMenu =
		ResolveAncestorMenuContainingPointer(ScreenPosition))
	{
		if (IsPointerOverSelectableItemInMenu(AncestorMenu, ScreenPosition))
		{
			CloseDescendantMenusToAncestor(AncestorMenu);
		}
		return false;
	}
	return HandleContextMenuOutsidePointer();
}

void ULunarContextMenu::HandleGlobalPointerMove(
	const FVector2D& ScreenPosition,
	const uint32 SlateUserIndex)
{
	ULunarNavigationSubsystem* NavigationSubsystem =
		ResolveContextMenuNavigationSubsystem();
	if (!bContextMenuOpen
		|| bClosingContextMenu
		|| !IsPointerOwnedByLocalPlayer(SlateUserIndex))
	{
		ResetPointerLeaveState();
		return;
	}
	if (IsOurScopeTop(NavigationSubsystem))
	{
		UpdateAncestorPointerHover(ScreenPosition);
	}

	if (!bCloseOnPointerLeave
		|| !IsInActiveContextMenuChain(NavigationSubsystem))
	{
		ResetPointerLeaveState();
		return;
	}

	if (IsPointerInsideActiveContextMenuChain(ScreenPosition, NavigationSubsystem))
	{
		bPointerWasInsidePopup = true;
		bPointerLeaveClosePending = false;
		PointerLeaveCloseElapsed = 0.0f;
	}
	else if (bPointerWasInsidePopup && !bPointerLeaveClosePending)
	{
		bPointerLeaveClosePending = true;
		PointerLeaveCloseElapsed = 0.0f;
	}
}

void ULunarContextMenu::TickPointerLeaveDismissal(const float DeltaTime)
{
	if (!bPointerLeaveClosePending)
	{
		return;
	}

	ULunarNavigationSubsystem* NavigationSubsystem =
		ResolveContextMenuNavigationSubsystem();
	if (!bCloseOnPointerLeave || !IsInActiveContextMenuChain(NavigationSubsystem))
	{
		ResetPointerLeaveState();
		return;
	}

	PointerLeaveCloseElapsed += FMath::Max(0.0f, DeltaTime);
	if (PointerLeaveCloseElapsed < PointerLeaveCloseDelay)
	{
		return;
	}

	const bool bIsRootMenu = ResolveContextMenuChainRoot() == this;
	ResetPointerLeaveState();
	if (bIsRootMenu)
	{
		CloseEntireContextMenuChain();
	}
	else
	{
		CloseContextMenu();
	}
}

void ULunarContextMenu::ResetPointerLeaveState()
{
	bPointerWasInsidePopup = false;
	bPointerLeaveClosePending = false;
	PointerLeaveCloseElapsed = 0.0f;
}

void ULunarContextMenu::FinalizeContextMenuClosed(ULunarNavigationSubsystem* NavigationSubsystem)
{
	if (!bContextMenuOpen || bClosingContextMenu)
	{
		return;
	}

	TGuardValue<bool> ClosingGuard(bClosingContextMenu, true);
	UnregisterOutsideClickProcessor();
	bContextMenuOpen = false;
	bCloseRequestedWhileOpening = false;
	SetVisibility(ESlateVisibility::Collapsed);
	RefreshVisualState();

	// During Pop the parent graph is rebuilt before this close callback. If it had
	// no saved selection, a still-visible submenu descendant could have been chosen.
	// Collapse first, then reset only that invalid edge case to the parent's normal
	// initial/first-eligible selection.
	if (NavigationSubsystem
		&& NavigationSubsystem->GetActiveNavigationScope()
		&& IsWidgetInsideClosingMenu(NavigationSubsystem->GetSelectedWidget()))
	{
		NavigationSubsystem->ResetSelection();
	}

	OnContextMenuClosed.Broadcast(this);
}

void ULunarContextMenu::ApplyRuntimePolicies()
{
	bCanReceiveLunarSelection = false;
	bCanInteractWithPointer = false;
	bAllowTouchInput = false;
	SetIsFocusable(false);
}
