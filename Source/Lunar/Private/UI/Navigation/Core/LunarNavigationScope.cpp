// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarNavigationScope.cpp
 * @brief Implements non-visual navigation scope state and restoration bookkeeping.
 * @ingroup LunarNavigationCore
 */

#include "UI/Navigation/Core/LunarNavigationScope.h"

#include "Engine/LocalPlayer.h"
#include "UI/Navigation/Core/LunarNavigableWidget.h"
#include "UI/Navigation/Core/LunarNavigationSubsystem.h"

ULocalPlayer* ULunarNavigationScope::GetOwningLocalPlayer() const
{
	return OwningLocalPlayer;
}

ULunarNavigationScope* ULunarNavigationScope::GetParentScope() const
{
	return ParentScope;
}

TArray<ULunarNavigableWidget*> ULunarNavigationScope::GetRegisteredWidgets() const
{
	TArray<ULunarNavigableWidget*> Result;
	Result.Reserve(RegisteredWidgets.Num());
	for (ULunarNavigableWidget* Widget : RegisteredWidgets)
	{
		if (IsValid(Widget))
		{
			Result.Add(Widget);
		}
	}
	return Result;
}

FName ULunarNavigationScope::GetInitialSelectionId() const
{
	return InitialSelectionId;
}

FName ULunarNavigationScope::GetLastSelectionId() const
{
	return LastSelectionId;
}

bool ULunarNavigationScope::IsActive() const
{
	return bActive;
}

void ULunarNavigationScope::RequestValidation()
{
	if (OwningLocalPlayer)
	{
		if (ULunarNavigationSubsystem* NavigationSubsystem = OwningLocalPlayer->GetSubsystem<ULunarNavigationSubsystem>())
		{
			NavigationSubsystem->ValidateNavigationScope(this);
		}
	}
}

void ULunarNavigationScope::InitializeScope(
	ULocalPlayer* InLocalPlayer,
	UWidget* InRootWidget,
	const FLunarNavigationScopeSettings& InSettings,
	ULunarNavigationScope* InParentScope)
{
	const bool bPreserveRestorationState = OwningLocalPlayer == InLocalPlayer && RootWidget == InRootWidget;

	OwningLocalPlayer = InLocalPlayer;
	RootWidget = InRootWidget;
	Settings = InSettings;
	ParentScope = InParentScope;
	InitialSelectionId = Settings.InitialSelectionId;
	if (InitialSelectionId.IsNone() && Settings.InitialSelectionWidget)
	{
		InitialSelectionId = Settings.InitialSelectionWidget->GetNavigationId();
	}
	RegisteredWidgets.Reset();
	LastValidationMessages.Reset();
	if (!bPreserveRestorationState)
	{
		LastSelectedWidget = nullptr;
		LastSelectionId = NAME_None;
	}
	bActive = false;
}

void ULunarNavigationScope::ActivateScope()
{
	bActive = true;
}

void ULunarNavigationScope::DeactivateScope()
{
	bActive = false;
}

void ULunarNavigationScope::AddRegisteredWidget(ULunarNavigableWidget* Widget)
{
	if (IsValid(Widget))
	{
		RegisteredWidgets.AddUnique(Widget);
	}
}

void ULunarNavigationScope::RemoveRegisteredWidget(ULunarNavigableWidget* Widget)
{
	RegisteredWidgets.Remove(Widget);
	if (LastSelectedWidget == Widget)
	{
		LastSelectedWidget = nullptr;
	}
}

void ULunarNavigationScope::SetLastSelectedWidget(ULunarNavigableWidget* Widget)
{
	LastSelectedWidget = Widget;
	if (Widget && !Widget->GetNavigationId().IsNone())
	{
		LastSelectionId = Widget->GetNavigationId();
	}
}

ULunarNavigableWidget* ULunarNavigationScope::GetLastSelectedWidget() const
{
	return LastSelectedWidget;
}
