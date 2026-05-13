// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/LunarTypesConsole.h"
#include "LunarFLConsole.generated.h"

UCLASS()
class LUNAR_API ULunarFLConsole : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Lunar|Console", meta = (WorldContext = "WorldContextObject"))
	static void ConsolePrint(const UObject* WorldContextObject, FGameplayTag Category, ELunarConsoleMessageVerbosity Verbosity, const FString& Text);

	UFUNCTION(BlueprintCallable, Category = "Lunar|Console", meta = (WorldContext = "WorldContextObject"))
	static void ConsolePrintMessage(const UObject* WorldContextObject, FGameplayTag Category, const FString& Text);

	UFUNCTION(BlueprintCallable, Category = "Lunar|Console", meta = (WorldContext = "WorldContextObject"))
	static void ConsolePrintInfo(const UObject* WorldContextObject, FGameplayTag Category, const FString& Text);

	UFUNCTION(BlueprintCallable, Category = "Lunar|Console", meta = (WorldContext = "WorldContextObject"))
	static void ConsolePrintSuccess(const UObject* WorldContextObject, FGameplayTag Category, const FString& Text);

	UFUNCTION(BlueprintCallable, Category = "Lunar|Console", meta = (WorldContext = "WorldContextObject"))
	static void ConsolePrintWarning(const UObject* WorldContextObject, FGameplayTag Category, const FString& Text);

	UFUNCTION(BlueprintCallable, Category = "Lunar|Console", meta = (WorldContext = "WorldContextObject"))
	static void ConsolePrintError(const UObject* WorldContextObject, FGameplayTag Category, const FString& Text);

	UFUNCTION(BlueprintCallable, Category = "Lunar|Console", meta = (WorldContext = "WorldContextObject"))
	static bool ExecuteConsoleCommandString(const UObject* WorldContextObject, const FString& Input, FString& OutError);

	UFUNCTION(BlueprintPure, Category = "Lunar|Console", meta = (WorldContext = "WorldContextObject"))
	static TArray<FString> GetConsoleCommandSuggestionStrings(const UObject* WorldContextObject, const FString& Input);
};