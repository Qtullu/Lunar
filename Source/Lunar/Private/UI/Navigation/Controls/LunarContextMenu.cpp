// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarContextMenu.cpp
 * @brief Implements nested scope ownership, outside-pointer dismissal, and popup lifetime.
 * @ingroup LunarNavigationControls
 */

#include "UI/Navigation/Controls/LunarContextMenu.h"

#include "Components/PanelWidget.h"
#include "Engine/LocalPlayer.h"
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
	SetVisibility(ESlateVisibility::Visible);
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
	return bContextMenuOpen
		&& IsOurScopeTop(ResolveContextMenuNavigationSubsystem())
		&& CloseContextMenu();
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
	}
}

void ULunarContextMenu::SynchronizeProperties()
{
	Super::SynchronizeProperties();
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

void ULunarContextMenu::FinalizeContextMenuClosed(ULunarNavigationSubsystem* NavigationSubsystem)
{
	if (!bContextMenuOpen || bClosingContextMenu)
	{
		return;
	}

	TGuardValue<bool> ClosingGuard(bClosingContextMenu, true);
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
