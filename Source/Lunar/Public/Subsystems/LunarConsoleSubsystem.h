// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputCoreTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Types/LunarTypesConsole.h"
#include "LunarConsoleSubsystem.generated.h"

class APlayerController;
class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLunarConsoleMessageAddedSignature, const FLunarConsoleMessage&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLunarConsoleMessagesClearedSignature);

UCLASS()
class LUNAR_API ULunarConsoleSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Console")
	void AddMessage(FGameplayTag Category, ELunarConsoleMessageVerbosity Verbosity, const FString& Text);

	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Console")
	void ClearMessages();

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Console")
	const TArray<FLunarConsoleMessage>& GetMessages() const;

	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Console")
	void ReloadCommands();

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Console")
	TArray<FLunarConsoleCommandDefinition> GetCommands() const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Console")
	TArray<FLunarConsoleCommandDefinition> GetCommandSuggestions(const FString& Input) const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Console")
	TArray<FString> GetCommandSuggestionStrings(const FString& Input) const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Console")
	TArray<FString> GetAllCommandSuggestionStrings() const;

	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Console")
	bool ExecuteCommandString(const FString& Input, FString& OutError);

	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Console")
	void ToggleConsoleWidget();

	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Console")
	void ShowConsoleWidget();

	UFUNCTION(BlueprintCallable, Category = "Lunar|Subsystems|Console")
	void HideConsoleWidget();

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Console")
	bool IsConsoleWidgetVisible() const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Subsystems|Console")
	bool CanUseConsole() const;

public:
	UPROPERTY(BlueprintAssignable, Category = "Lunar|Subsystems|Console")
	FLunarConsoleMessageAddedSignature OnMessageAdded;

	UPROPERTY(BlueprintAssignable, Category = "Lunar|Subsystems|Console")
	FLunarConsoleMessagesClearedSignature OnMessagesCleared;

private:
	UPROPERTY()
	TArray<FLunarConsoleMessage> Messages;

	UPROPERTY()
	TMap<FName, FLunarConsoleCommandDefinition> CommandsByName;

	UPROPERTY()
	TObjectPtr<UUserWidget> ConsoleWidgetInstance = nullptr;

	bool bCachedPlayerControllerState = false;
	bool bCachedShowMouseCursor = false;

private:
	const FLunarConsoleSettings& GetConsoleSettings() const;

	FLinearColor GetColorForVerbosity(ELunarConsoleMessageVerbosity Verbosity) const;
	FString BuildFormattedText(const FLunarConsoleMessage& Message) const;
	void TrimMessagesIfNeeded();

	bool ParseInput(const FString& Input, FName& OutCommand, FString& OutParameter) const;
	bool ExecuteCommand(const FLunarConsoleCommandDefinition& Command, const FString& ParameterString, FString& OutError);

	UObject* ResolveTargetObject(const FLunarConsoleCommandDefinition& Command) const;

	bool WriteParameterToBuffer(const FLunarConsoleCommandDefinition& Command, const FString& ParameterString, uint8* ParamBuffer, FProperty* Property, FString& OutError) const;

	FString GetDefaultPreviewValueForParameterType(ELunarConsoleCommandParameterType ParameterType) const;
	FString BuildSuggestionString(const FLunarConsoleCommandDefinition& Command) const;

	FGameplayTag GetDefaultConsoleCategoryTag() const;

	TSubclassOf<UUserWidget> LoadConsoleWidgetClass() const;

	void BindRawInput();
	void UnbindRawInput();

	UFUNCTION()
	void HandleRawInputKeyClicked(FKey Key);

	APlayerController* GetFirstLocalPlayerController() const;
	void ApplyConsoleInputMode();
	void RestorePreviousInputMode();
};