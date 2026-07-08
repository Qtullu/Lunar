// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputCoreTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Types/LunarTypesConsole.h"
#include "LunarConsoleSubsystem.generated.h"

/**
 * @file LunarConsoleSubsystem.h
 * @brief Console subsystem
 * @ingroup LunarConsoleSubsystem
 */

class APlayerController;
class UUserWidget;

/**
 * @brief Called when a Lunar console message is added
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLunarConsoleMessageAddedSignature, const FLunarConsoleMessage&, Message);

/**
 * @brief Called when Lunar console messages are removed
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLunarConsoleMessagesRemovedSignature, int32, RemovedCount);

/**
 * @brief Called when Lunar console messages are cleared
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLunarConsoleMessagesClearedSignature);

/**
 * @brief Stores console messages executes commands and controls console widget state
 * @ingroup LunarConsoleSubsystem
 */
UCLASS()
class LUNAR_API ULunarConsoleSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * @brief Initializes console subsystem state
	 * @param Collection Subsystem collection
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * @brief Deinitializes console subsystem state
	 */
	virtual void Deinitialize() override;

public:

	/**
	 * @brief Gets active Lunar console subsystem instance
	 * @return Active Lunar console subsystem instance
	 */
	static ULunarConsoleSubsystem* Get();

	/**
	 * @brief Adds a message to the active Lunar console subsystem
	 * @param Category Message category
	 * @param Verbosity Message verbosity
	 * @param Text Message text
	 */
	static void AddMessage(FGameplayTag Category, ELunarConsoleMessageVerbosity Verbosity, const FString& Text);

	/**
	 * @brief Adds a message to the Lunar console
	 * @param Category Message category
	 * @param Verbosity Message verbosity
	 * @param Text Message text
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Console", meta = (DisplayName = "Add Message"))
	void BP_AddMessage(FGameplayTag Category, ELunarConsoleMessageVerbosity Verbosity, const FString& Text);

	/**
	 * @brief Clears all Lunar console messages
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Console")
	void ClearMessages();

	/**
	 * @brief Gets stored Lunar console messages
	 * @return Stored Lunar console messages
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Console")
	const TArray<FLunarConsoleMessage>& GetMessages() const;

	/**
	 * @brief Reloads Lunar console command definitions
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Console")
	void ReloadCommands();

	/**
	 * @brief Gets loaded Lunar console command definitions
	 * @return Loaded Lunar console command definitions
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Console")
	TArray<FLunarConsoleCommandDefinition> GetCommands() const;

	/**
	 * @brief Gets command suggestions for input text
	 * @param Input Console input text
	 * @return Matching console command definitions
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Console")
	TArray<FLunarConsoleCommandDefinition> GetCommandSuggestions(const FString& Input) const;

	/**
	 * @brief Gets command suggestion strings for input text
	 * @param Input Console input text
	 * @return Matching console command suggestion strings
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Console")
	TArray<FString> GetCommandSuggestionStrings(const FString& Input) const;

	/**
	 * @brief Gets all command suggestion strings
	 * @return All console command suggestion strings
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Console")
	TArray<FString> GetAllCommandSuggestionStrings() const;

	/**
	 * @brief Executes a console command string
	 * @param Input Console command input
	 * @param OutError Error text when command execution fails
	 * @return True if command was executed
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Console")
	bool ExecuteCommandString(const FString& Input, FString& OutError);

	/**
	 * @brief Toggles console widget visibility
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Console")
	void ToggleConsoleWidget();

	/**
	 * @brief Shows console widget
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Console")
	void ShowConsoleWidget();

	/**
	 * @brief Hides console widget
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Console")
	void HideConsoleWidget();

	/**
	 * @brief Checks whether console widget is visible
	 * @return True if console widget is visible
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Console")
	bool IsConsoleWidgetVisible() const;

	/**
	 * @brief Checks whether Lunar console can be used in current runtime environment
	 * @return True if Lunar console can be used
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Console")
	bool CanUseConsole() const;

public:
	/** Console message added event */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|Subsystems|Console")
	FLunarConsoleMessageAddedSignature OnMessageAdded;

	/** Console messages removed event */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|Subsystems|Console")
	FLunarConsoleMessagesRemovedSignature OnMessagesRemoved;

	/** Console messages cleared event */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|Subsystems|Console")
	FLunarConsoleMessagesClearedSignature OnMessagesCleared;


private:
	/** Stored console messages */
	UPROPERTY()
	TArray<FLunarConsoleMessage> Messages;

	/** Loaded command definitions by command name */
	UPROPERTY()
	TMap<FName, FLunarConsoleCommandDefinition> CommandsByName;

	/** Active console widget instance */
	UPROPERTY()
	TObjectPtr<UUserWidget> ConsoleWidgetInstance = nullptr;

	/** Cached player controller input state before opening console */
	bool bCachedPlayerControllerState = false;

	/** Cached mouse cursor state before opening console */
	bool bCachedShowMouseCursor = false;

private:
	/**
	 * @brief Gets console settings from Lunar settings
	 * @return Console settings
	 */
	const FLunarConsoleSettings& GetConsoleSettings() const;

	/**
	 * @brief Gets display color for message verbosity
	 * @param Verbosity Message verbosity
	 * @return Display color
	 */
	FLinearColor GetColorForVerbosity(ELunarConsoleMessageVerbosity Verbosity) const;

	/**
	 * @brief Builds formatted console message text
	 * @param Message Console message
	 * @return Formatted message text
	 */
	FString BuildFormattedText(const FLunarConsoleMessage& Message) const;

	/**
	 * @brief Trims stored messages to configured limit
	 * @return Removed message count
	 */
	int32 TrimMessagesIfNeeded();

	/**
	 * @brief Parses console input into command and parameter text
	 * @param Input Console input text
	 * @param OutCommand Parsed command name
	 * @param OutParameter Parsed parameter text
	 * @return True if input was parsed
	 */
	bool ParseInput(const FString& Input, FName& OutCommand, FString& OutParameter) const;

	/**
	 * @brief Executes resolved console command
	 * @param Command Command definition
	 * @param ParameterString Command parameter text
	 * @param OutError Error text when command execution fails
	 * @return True if command was executed
	 */
	bool ExecuteCommand(const FLunarConsoleCommandDefinition& Command, const FString& ParameterString, FString& OutError);

	/**
	 * @brief Resolves target object for command execution
	 * @param Command Command definition
	 * @return Target object or null
	 */
	UObject* ResolveTargetObject(const FLunarConsoleCommandDefinition& Command) const;

	/**
	 * @brief Writes command parameter text to reflected function parameter buffer
	 * @param Command Command definition
	 * @param ParameterString Command parameter text
	 * @param ParamBuffer Function parameter buffer
	 * @param Property Reflected property to write
	 * @param OutError Error text when parameter conversion fails
	 * @return True if parameter was written
	 */
	bool WriteParameterToBuffer(const FLunarConsoleCommandDefinition& Command, const FString& ParameterString, uint8* ParamBuffer, FProperty* Property, FString& OutError) const;

	/**
	 * @brief Gets default preview value for command parameter type
	 * @param ParameterType Command parameter type
	 * @return Default preview value
	 */
	FString GetDefaultPreviewValueForParameterType(ELunarConsoleCommandParameterType ParameterType) const;

	/**
	 * @brief Builds command suggestion string
	 * @param Command Command definition
	 * @return Command suggestion string
	 */
	FString BuildSuggestionString(const FLunarConsoleCommandDefinition& Command) const;

	/**
	 * @brief Gets default console category tag
	 * @return Default console category tag
	 */
	FGameplayTag GetDefaultConsoleCategoryTag() const;

	/**
	 * @brief Loads configured console widget class
	 * @return Console widget class
	 */
	TSubclassOf<UUserWidget> LoadConsoleWidgetClass() const;

	/**
	 * @brief Binds raw input events used by console hotkey
	 */
	void BindRawInput();

	/**
	 * @brief Unbinds raw input events used by console hotkey
	 */
	void UnbindRawInput();

	/**
	 * @brief Handles raw input key click events
	 * @param Key Clicked key
	 */
	UFUNCTION()
	void HandleRawInputKeyClicked(FKey Key);

	/**
	 * @brief Gets first local player controller
	 * @return First local player controller or null
	 */
	APlayerController* GetFirstLocalPlayerController() const;

	/**
	 * @brief Applies input mode for opened console widget
	 */
	void ApplyConsoleInputMode();

	/**
	 * @brief Restores input mode after closing console widget
	 */
	void RestorePreviousInputMode();

	/** Active console subsystem weak reference */
	static TWeakObjectPtr<ULunarConsoleSubsystem> ActiveConsoleSubsystem;
};