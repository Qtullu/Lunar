// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarScreenWidget.cpp
 * @brief Implements automatic screen-scope creation and stack lifetime coordination.
 * @ingroup LunarNavigationCore
 */

#include "UI/Navigation/Core/LunarScreenWidget.h"

#include "Engine/LocalPlayer.h"
#include "UI/Navigation/Core/LunarNavigationScope.h"
#include "UI/Navigation/Core/LunarNavigationSubsystem.h"

ULunarScreenWidget::ULunarScreenWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(false);
}

void ULunarScreenWidget::SynchronizeProperties()
{
	SetIsFocusable(false);
	Super::SynchronizeProperties();
}

void ULunarScreenWidget::NativeConstruct()
{
	SetIsFocusable(false);
	Super::NativeConstruct();
	if (bAutoActivateNavigationScope)
	{
		OpenScreen();
	}
}

void ULunarScreenWidget::NativeDestruct()
{
	CloseScreen();
	Super::NativeDestruct();
}

bool ULunarScreenWidget::OpenScreen()
{
	if (bScreenOpen || bOpeningScreen)
	{
		return true;
	}

	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem();
	if (!LocalPlayer || !NavigationSubsystem)
	{
		return false;
	}

	if (!NavigationScope)
	{
		NavigationScope = NewObject<ULunarNavigationScope>(this);
	}

	NavigationScope->RootWidget = this;
	NavigationScope->Settings = NavigationScopeSettings;
	bOpeningScreen = true;
	bCloseRequestedWhileOpening = false;
	const bool bPushed = NavigationSubsystem->PushNavigationScope(NavigationScope);
	bOpeningScreen = false;
	if (!bPushed || !NavigationSubsystem->GetNavigationScopeStack().Contains(NavigationScope))
	{
		bCloseRequestedWhileOpening = false;
		return false;
	}

	bScreenOpen = true;
	OnScreenOpened.Broadcast();
	const bool bShouldClose = bCloseRequestedWhileOpening;
	bCloseRequestedWhileOpening = false;
	if (bShouldClose && bScreenOpen)
	{
		CloseScreen();
	}
	return bScreenOpen;
}

bool ULunarScreenWidget::CloseScreen()
{
	if (bOpeningScreen)
	{
		bCloseRequestedWhileOpening = true;
		return true;
	}

	if (!bScreenOpen)
	{
		return true;
	}

	if (ULunarNavigationSubsystem* NavigationSubsystem = ResolveNavigationSubsystem())
	{
		if (NavigationScope && !NavigationSubsystem->PopNavigationScope(NavigationScope))
		{
			return false;
		}
	}

	if (bScreenOpen)
	{
		HandleNavigationScopePopped();
	}
	return true;
}

bool ULunarScreenWidget::IsScreenOpen() const
{
	return bScreenOpen;
}

ULunarNavigationScope* ULunarScreenWidget::GetNavigationScope() const
{
	return NavigationScope;
}

ULunarNavigationSubsystem* ULunarScreenWidget::ResolveNavigationSubsystem() const
{
	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		return LocalPlayer->GetSubsystem<ULunarNavigationSubsystem>();
	}
	return nullptr;
}

void ULunarScreenWidget::HandleNavigationScopePopped()
{
	if (!bScreenOpen)
	{
		return;
	}

	bScreenOpen = false;
	OnScreenClosed.Broadcast();
}
