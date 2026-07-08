// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LunarTypes.h"
#include "LunarFLGame.generated.h"

/**
 * @file LunarFLGame.h
 * @brief Game helper function library
 * @ingroup LunarFLGame
 */

 /**
  * @brief Blueprint utility functions for game and platform features
  * @ingroup LunarFLGame
  */
UCLASS()
class LUNAR_API ULunarFLGame : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Build configuration

	/**
	 * @brief Checks whether current build configuration is Debug
	 * @return True if current build configuration is Debug
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Build")
	static bool IsDebug();

	/**
	 * @brief Checks whether current build configuration is DebugGame
	 * @return True if current build configuration is DebugGame
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Build")
	static bool IsDebugGame();

	/**
	 * @brief Checks whether current build configuration is Development
	 * @return True if current build configuration is Development
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Build")
	static bool IsDevelopment();

	/**
	 * @brief Checks whether current build configuration is Test
	 * @return True if current build configuration is Test
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Build")
	static bool IsTest();

	/**
	 * @brief Checks whether current build configuration is Shipping
	 * @return True if current build configuration is Shipping
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Build")
	static bool IsShipping();

	/**
	 * @brief Gets current build configuration name
	 * @return Current build configuration name
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Build")
	static FString GetBuildConfigurationName();

	// Runtime

	/**
	 * @brief Checks whether code is running with editor support
	 * @return True if code is running with editor support
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Runtime")
	static bool IsEditor();

	/**
	 * @brief Checks whether editor only data is enabled
	 * @return True if editor only data is enabled
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Runtime")
	static bool IsEditorOnlyDataEnabled();

	/**
	 * @brief Checks whether code is running as commandlet
	 * @return True if code is running as commandlet
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Runtime")
	static bool IsCommandlet();

	/**
	 * @brief Checks whether code is running on dedicated server
	 * @return True if code is running on dedicated server
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Runtime")
	static bool IsDedicatedServer();

	/**
	 * @brief Checks whether code is running as game
	 * @return True if code is running as game
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Runtime")
	static bool IsGame();

	/**
	 * @brief Checks whether world is Play In Editor
	 * @param WorldContextObject World context object
	 * @return True if world is Play In Editor
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Runtime", meta = (WorldContext = "WorldContextObject"))
	static bool IsPIE(const UObject* WorldContextObject);

	/**
	 * @brief Checks whether world context belongs to a game world
	 * @param WorldContextObject World context object
	 * @return True if world context belongs to a game world
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Runtime", meta = (WorldContext = "WorldContextObject"))
	static bool IsGameWorld(const UObject* WorldContextObject);

	/**
	 * @brief Checks whether world context belongs to an editor world
	 * @param WorldContextObject World context object
	 * @return True if world context belongs to an editor world
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Runtime", meta = (WorldContext = "WorldContextObject"))
	static bool IsEditorWorld(const UObject* WorldContextObject);

	// Platform

	/**
	 * @brief Checks whether current platform is Windows
	 * @return True if current platform is Windows
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Platform")
	static bool IsWindows();

	/**
	 * @brief Checks whether current platform is Mac
	 * @return True if current platform is Mac
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Platform")
	static bool IsMac();

	/**
	 * @brief Checks whether current platform is Linux
	 * @return True if current platform is Linux
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Platform")
	static bool IsLinux();

	/**
	 * @brief Checks whether current platform is Android
	 * @return True if current platform is Android
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Platform")
	static bool IsAndroid();

	/**
	 * @brief Checks whether current platform is iOS
	 * @return True if current platform is iOS
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Platform")
	static bool IsIOS();

	/**
	 * @brief Checks whether current platform is console
	 * @return True if current platform is console
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Platform")
	static bool IsConsolePlatform();

	/**
	 * @brief Checks whether current platform is desktop
	 * @return True if current platform is desktop
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Platform")
	static bool IsDesktopPlatform();

	/**
	 * @brief Gets current platform name
	 * @return Current platform name
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Platform")
	static FString GetPlatformName();

	/**
	 * @brief Gets current platform type
	 * @return Current platform type
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Platform")
	static ELunarPlatformType GetPlatformType();

	/**
	 * @brief Gets current platform family
	 * @return Current platform family
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Platform")
	static ELunarPlatformFamily GetPlatformFamily();

	// Project

	/**
	 * @brief Gets project name
	 * @return Project name
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Project")
	static FString GetProjectName();

	/**
	 * @brief Gets project version
	 * @return Project version
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Project")
	static FString GetProjectVersion();

	/**
	 * @brief Gets project displayed title
	 * @return Project displayed title
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Project")
	static FString GetProjectDisplayedTitle();

	//Platform utilities

	/**
	 * @brief Copies string to system clipboard
	 * @param Text Text to copy
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Game|Clipboard", meta = (DisplayName = "Copy String To Clipboard"))
	static void CopyStringToClipboard(const FString& Text);

	/**
	 * @brief Pastes string from system clipboard
	 * @return Clipboard text
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Clipboard", meta = (DisplayName = "Paste String From Clipboard"))
	static FString PasteStringFromClipboard();
};