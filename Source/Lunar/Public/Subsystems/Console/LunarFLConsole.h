// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Subsystems/Console/LunarTypesConsole.h"
#include "LunarFLConsole.generated.h"

/**
 * @file LunarFLConsole.h
 * @brief Console helper function library
 * @ingroup LunarFLConsole
 */

 /**
  * @brief Blueprint utility functions for Lunar console
  * @ingroup LunarFLConsole
  */
UCLASS()
class LUNAR_API ULunarFLConsole : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * @brief Adds a message to the Lunar console with custom verbosity
	 * @param WorldContextObject World context object
	 * @param Category Message category
	 * @param Verbosity Message verbosity
	 * @param Text Message text
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Console", meta = (WorldContext = "WorldContextObject"))
	static void ConsolePrint(const UObject* WorldContextObject, FGameplayTag Category, ELunarConsoleMessageVerbosity Verbosity, const FString& Text);

	/**
	 * @brief Adds a regular message to the Lunar console
	 * @param WorldContextObject World context object
	 * @param Category Message category
	 * @param Text Message text
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Console", meta = (WorldContext = "WorldContextObject"))
	static void ConsolePrintMessage(const UObject* WorldContextObject, FGameplayTag Category, const FString& Text);

	/**
	 * @brief Adds an info message to the Lunar console
	 * @param WorldContextObject World context object
	 * @param Category Message category
	 * @param Text Message text
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Console", meta = (WorldContext = "WorldContextObject"))
	static void ConsolePrintInfo(const UObject* WorldContextObject, FGameplayTag Category, const FString& Text);

	/**
	 * @brief Adds a success message to the Lunar console
	 * @param WorldContextObject World context object
	 * @param Category Message category
	 * @param Text Message text
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Console", meta = (WorldContext = "WorldContextObject"))
	static void ConsolePrintSuccess(const UObject* WorldContextObject, FGameplayTag Category, const FString& Text);

	/**
	 * @brief Adds a warning message to the Lunar console
	 * @param WorldContextObject World context object
	 * @param Category Message category
	 * @param Text Message text
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Console", meta = (WorldContext = "WorldContextObject"))
	static void ConsolePrintWarning(const UObject* WorldContextObject, FGameplayTag Category, const FString& Text);

	/**
	 * @brief Adds an error message to the Lunar console
	 * @param WorldContextObject World context object
	 * @param Category Message category
	 * @param Text Message text
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Console", meta = (WorldContext = "WorldContextObject"))
	static void ConsolePrintError(const UObject* WorldContextObject, FGameplayTag Category, const FString& Text);

	/**
	 * @brief Executes a Lunar console command string
	 * @param WorldContextObject World context object
	 * @param Input Console command input
	 * @param OutError Error text when execution fails
	 * @return True if command was executed
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Console", meta = (WorldContext = "WorldContextObject"))
	static bool ExecuteConsoleCommandString(const UObject* WorldContextObject, const FString& Input, FString& OutError);

	/**
	 * @brief Gets Lunar console command suggestion strings for input text
	 * @param WorldContextObject World context object
	 * @param Input Console input text
	 * @return Matching command suggestion strings
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Console", meta = (WorldContext = "WorldContextObject"))
	static TArray<FString> GetConsoleCommandSuggestionStrings(const UObject* WorldContextObject, const FString& Input);

	/**
	 * @brief Saves current Lunar console log to a text file
	 * @param WorldContextObject World context object
	 * @param OutSavedFilePath Saved file path
	 * @param OutError Error text when saving fails
	 * @return True if console log was saved
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Console", meta = (WorldContext = "WorldContextObject", DisplayName = "Save Console Log To Text File"))
	static bool SaveConsoleLogToTextFile(const UObject* WorldContextObject, FString& OutSavedFilePath, FString& OutError);
};
