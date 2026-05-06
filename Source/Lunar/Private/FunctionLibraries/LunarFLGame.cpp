// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "FunctionLibraries/LunarFLGame.h"

#include "Engine/World.h"
#include "GeneralProjectSettings.h"
#include "Misc/App.h"

bool ULunarFLGame::IsDebug()
{
#if UE_BUILD_DEBUG
	return true;
#else
	return false;
#endif
}

bool ULunarFLGame::IsDebugGame()
{
#if defined(UE_BUILD_DEBUGGAME) && UE_BUILD_DEBUGGAME
	return true;
#else
	return false;
#endif
}

bool ULunarFLGame::IsDevelopment()
{
#if UE_BUILD_DEVELOPMENT
	return true;
#else
	return false;
#endif
}

bool ULunarFLGame::IsTest()
{
#if UE_BUILD_TEST
	return true;
#else
	return false;
#endif
}

bool ULunarFLGame::IsShipping()
{
#if UE_BUILD_SHIPPING
	return true;
#else
	return false;
#endif
}

FString ULunarFLGame::GetBuildConfigurationName()
{
#if UE_BUILD_DEBUG
	return TEXT("Debug");
#elif defined(UE_BUILD_DEBUGGAME) && UE_BUILD_DEBUGGAME
	return TEXT("DebugGame");
#elif UE_BUILD_DEVELOPMENT
	return TEXT("Development");
#elif UE_BUILD_TEST
	return TEXT("Test");
#elif UE_BUILD_SHIPPING
	return TEXT("Shipping");
#else
	return TEXT("Unknown");
#endif
}

bool ULunarFLGame::IsEditor()
{
#if WITH_EDITOR
	return true;
#else
	return false;
#endif
}

bool ULunarFLGame::IsEditorOnlyDataEnabled()
{
#if WITH_EDITORONLY_DATA
	return true;
#else
	return false;
#endif
}

bool ULunarFLGame::IsCommandlet()
{
	return ::IsRunningCommandlet();
}

bool ULunarFLGame::IsDedicatedServer()
{
	return ::IsRunningDedicatedServer();
}

bool ULunarFLGame::IsGame()
{
	return FApp::IsGame();
}

bool ULunarFLGame::IsPIE(const UObject* WorldContextObject)
{
	if (!IsValid(WorldContextObject))
	{
		return false;
	}

	const UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return false;
	}

	return World->WorldType == EWorldType::PIE;
}

bool ULunarFLGame::IsGameWorld(const UObject* WorldContextObject)
{
	if (!IsValid(WorldContextObject))
	{
		return false;
	}

	const UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return false;
	}

	return World->IsGameWorld();
}

bool ULunarFLGame::IsEditorWorld(const UObject* WorldContextObject)
{
	if (!IsValid(WorldContextObject))
	{
		return false;
	}

	const UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return false;
	}

	return World->WorldType == EWorldType::Editor;
}

bool ULunarFLGame::IsWindows()
{
#if PLATFORM_WINDOWS
	return true;
#else
	return false;
#endif
}

bool ULunarFLGame::IsMac()
{
#if PLATFORM_MAC
	return true;
#else
	return false;
#endif
}

bool ULunarFLGame::IsLinux()
{
#if PLATFORM_LINUX
	return true;
#else
	return false;
#endif
}

bool ULunarFLGame::IsAndroid()
{
#if PLATFORM_ANDROID
	return true;
#else
	return false;
#endif
}

bool ULunarFLGame::IsIOS()
{
#if PLATFORM_IOS
	return true;
#else
	return false;
#endif
}

bool ULunarFLGame::IsConsolePlatform()
{
#if PLATFORM_DESKTOP
	return false;
#elif PLATFORM_ANDROID || PLATFORM_IOS
	return false;
#else
	return true;
#endif
}

bool ULunarFLGame::IsDesktopPlatform()
{
#if PLATFORM_DESKTOP
	return true;
#else
	return false;
#endif
}

FString ULunarFLGame::GetPlatformName()
{
#if PLATFORM_WINDOWS
	return TEXT("Windows");
#elif PLATFORM_MAC
	return TEXT("Mac");
#elif PLATFORM_LINUX
	return TEXT("Linux");
#elif PLATFORM_ANDROID
	return TEXT("Android");
#elif PLATFORM_IOS
	return TEXT("iOS");
#else
	return TEXT("Unknown");
#endif
}

FString ULunarFLGame::GetProjectName()
{
	return FApp::GetProjectName();
}

FString ULunarFLGame::GetProjectVersion()
{
	const UGeneralProjectSettings* ProjectSettings = GetDefault<UGeneralProjectSettings>();
	return ProjectSettings ? ProjectSettings->ProjectVersion : FString();
}

FString ULunarFLGame::GetProjectDisplayedTitle()
{
	const UGeneralProjectSettings* ProjectSettings = GetDefault<UGeneralProjectSettings>();
	return ProjectSettings ? ProjectSettings->ProjectDisplayedTitle.ToString() : FString();
}

ELunarPlatformType ULunarFLGame::GetPlatformType()
{
#if PLATFORM_WINDOWS
	return ELunarPlatformType::Windows;
#elif PLATFORM_MAC
	return ELunarPlatformType::Mac;
#elif PLATFORM_LINUX
	return ELunarPlatformType::Linux;
#elif PLATFORM_ANDROID
	return ELunarPlatformType::Android;
#elif PLATFORM_IOS
	return ELunarPlatformType::IOS;
#elif PLATFORM_PS4 || PLATFORM_PS5
	return ELunarPlatformType::PlayStation;
#elif PLATFORM_XBOXONE || PLATFORM_XSX
	return ELunarPlatformType::Xbox;
#elif PLATFORM_SWITCH
	return ELunarPlatformType::Switch;
#else
	return ELunarPlatformType::Unknown;
#endif
}

ELunarPlatformFamily ULunarFLGame::GetPlatformFamily()
{
#if PLATFORM_DESKTOP
	return ELunarPlatformFamily::Desktop;
#elif PLATFORM_ANDROID || PLATFORM_IOS
	return ELunarPlatformFamily::Mobile;
#elif PLATFORM_PS4 || PLATFORM_PS5 || PLATFORM_XBOXONE || PLATFORM_XSX || PLATFORM_SWITCH
	return ELunarPlatformFamily::Console;
#else
	return ELunarPlatformFamily::Unknown;
#endif
}