// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LunarTypes.h"
#include "LunarFLGame.generated.h"

UCLASS()
class LUNAR_API ULunarFLGame : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Build configuration

	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Build")
	static bool IsDebug();

	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Build")
	static bool IsDebugGame();

	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Build")
	static bool IsDevelopment();

	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Build")
	static bool IsTest();

	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Build")
	static bool IsShipping();

	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Build")
	static FString GetBuildConfigurationName();

	// Runtime

	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Runtime")
	static bool IsEditor();

	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Runtime")
	static bool IsEditorOnlyDataEnabled();

	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Runtime")
	static bool IsCommandlet();

	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Runtime")
	static bool IsDedicatedServer();

	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Runtime")
	static bool IsGame();

	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Runtime", meta = (WorldContext = "WorldContextObject"))
	static bool IsPIE(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Runtime", meta = (WorldContext = "WorldContextObject"))
	static bool IsGameWorld(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Runtime", meta = (WorldContext = "WorldContextObject"))
	static bool IsEditorWorld(const UObject* WorldContextObject);

	// Platform

	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Platform")
	static bool IsWindows();

	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Platform")
	static bool IsMac();

	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Platform")
	static bool IsLinux();

	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Platform")
	static bool IsAndroid();

	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Platform")
	static bool IsIOS();

	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Platform")
	static bool IsConsolePlatform();

	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Platform")
	static bool IsDesktopPlatform();

	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Platform")
	static FString GetPlatformName();

	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Platform")
	static ELunarPlatformType GetPlatformType();

	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Platform")
	static ELunarPlatformFamily GetPlatformFamily();

	// Project

	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Project")
	static FString GetProjectName();

	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Project")
	static FString GetProjectVersion();

	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Project")
	static FString GetProjectDisplayedTitle();

	//Platform utilities
	UFUNCTION(BlueprintCallable, Category = "Lunar|Game|Clipboard", meta = (DisplayName = "Copy String To Clipboard"))
	static void CopyStringToClipboard(const FString& Text);

	UFUNCTION(BlueprintPure, Category = "Lunar|Game|Clipboard", meta = (DisplayName = "Paste String From Clipboard"))
	static FString PasteStringFromClipboard();
};