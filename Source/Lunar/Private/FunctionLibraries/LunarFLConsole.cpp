// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "FunctionLibraries/LunarFLConsole.h"
#include "Subsystems/LunarConsoleSubsystem.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"

void ULunarFLConsole::ConsolePrint(const UObject* WorldContextObject, FGameplayTag Category, ELunarConsoleMessageVerbosity Verbosity, const FString& Text)
{
	if (!WorldContextObject)
	{
		return;
	}

	const UWorld* World = WorldContextObject->GetWorld();

	if (!World)
	{
		return;
	}

	UGameInstance* GameInstance = World->GetGameInstance();

	if (!GameInstance)
	{
		return;
	}

	ULunarConsoleSubsystem* ConsoleSubsystem = GameInstance->GetSubsystem<ULunarConsoleSubsystem>();

	if (!ConsoleSubsystem)
	{
		return;
	}

	ConsoleSubsystem->AddMessage(Category, Verbosity, Text);
}

void ULunarFLConsole::ConsolePrintMessage(const UObject* WorldContextObject, FGameplayTag Category, const FString& Text)
{
	ConsolePrint(WorldContextObject, Category, ELunarConsoleMessageVerbosity::Message, Text);
}

void ULunarFLConsole::ConsolePrintInfo(const UObject* WorldContextObject, FGameplayTag Category, const FString& Text)
{
	ConsolePrint(WorldContextObject, Category, ELunarConsoleMessageVerbosity::Info, Text);
}

void ULunarFLConsole::ConsolePrintSuccess(const UObject* WorldContextObject, FGameplayTag Category, const FString& Text)
{
	ConsolePrint(WorldContextObject, Category, ELunarConsoleMessageVerbosity::Success, Text);
}

void ULunarFLConsole::ConsolePrintWarning(const UObject* WorldContextObject, FGameplayTag Category, const FString& Text)
{
	ConsolePrint(WorldContextObject, Category, ELunarConsoleMessageVerbosity::Warning, Text);
}

void ULunarFLConsole::ConsolePrintError(const UObject* WorldContextObject, FGameplayTag Category, const FString& Text)
{
	ConsolePrint(WorldContextObject, Category, ELunarConsoleMessageVerbosity::Error, Text);
}

bool ULunarFLConsole::ExecuteConsoleCommandString(const UObject* WorldContextObject, const FString& Input, FString& OutError)
{
	if (!WorldContextObject)
	{
		OutError = TEXT("Invalid world context.");
		return false;
	}

	const UWorld* World = WorldContextObject->GetWorld();

	if (!World)
	{
		OutError = TEXT("World not found.");
		return false;
	}

	UGameInstance* GameInstance = World->GetGameInstance();

	if (!GameInstance)
	{
		OutError = TEXT("Game instance not found.");
		return false;
	}

	ULunarConsoleSubsystem* ConsoleSubsystem = GameInstance->GetSubsystem<ULunarConsoleSubsystem>();

	if (!ConsoleSubsystem)
	{
		OutError = TEXT("LunarConsoleSubsystem not found.");
		return false;
	}

	return ConsoleSubsystem->ExecuteCommandString(Input, OutError);
}

TArray<FString> ULunarFLConsole::GetConsoleCommandSuggestionStrings(const UObject* WorldContextObject, const FString& Input)
{
	if (!WorldContextObject)
	{
		return {};
	}

	const UWorld* World = WorldContextObject->GetWorld();

	if (!World)
	{
		return {};
	}

	UGameInstance* GameInstance = World->GetGameInstance();

	if (!GameInstance)
	{
		return {};
	}

	ULunarConsoleSubsystem* ConsoleSubsystem = GameInstance->GetSubsystem<ULunarConsoleSubsystem>();

	if (!ConsoleSubsystem)
	{
		return {};
	}

	return ConsoleSubsystem->GetCommandSuggestionStrings(Input);
}