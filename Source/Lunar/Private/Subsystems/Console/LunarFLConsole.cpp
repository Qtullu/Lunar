// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "Subsystems/Console/LunarFLConsole.h"
#include "Subsystems/Console/LunarConsoleSubsystem.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"

#include "FunctionLibraries/LunarFLFile.h"
#include "Engine/Engine.h"
#include "GameplayTagContainer.h"
#include "HAL/PlatformProperties.h"
#include "Misc/App.h"
#include "Misc/Paths.h"

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


bool ULunarFLConsole::SaveConsoleLogToTextFile(const UObject* WorldContextObject, FString& OutSavedFilePath, FString& OutError)
{
	OutSavedFilePath.Empty();
	OutError.Empty();

	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;

	if (!World)
	{
		OutError = TEXT("SaveConsoleLogToTextFile failed: World is invalid.");
		return false;
	}

	UGameInstance* GameInstance = World->GetGameInstance();

	if (!GameInstance)
	{
		OutError = TEXT("SaveConsoleLogToTextFile failed: GameInstance is invalid.");
		return false;
	}

	ULunarConsoleSubsystem* ConsoleSubsystem = GameInstance->GetSubsystem<ULunarConsoleSubsystem>();

	if (!ConsoleSubsystem)
	{
		OutError = TEXT("SaveConsoleLogToTextFile failed: LunarConsoleSubsystem is invalid.");
		return false;
	}

	const FGameplayTag ConsoleCategory = FGameplayTag::RequestGameplayTag(TEXT("Lunar.Console"), false);
	const FDateTime SaveTime = FDateTime::Now();
	const FString FileNameWithExtension = SaveTime.ToString(TEXT("%Y.%m.%d %H-%M-%S console log.txt"));
	FString TargetDirectoryPath;

#if PLATFORM_WINDOWS
	if (!ULunarFLFile::SelectFolder(WorldContextObject, TEXT("Select Folder For Lunar Console Log"), FPaths::ProjectSavedDir(), TargetDirectoryPath, OutError))
	{
		return false;
	}
#else
	TargetDirectoryPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Lunar"), TEXT("ConsoleLogs"));
#endif

	const TArray<FLunarConsoleMessage>& Messages = ConsoleSubsystem->GetMessages();

	FString LogText;
	LogText.Reserve(Messages.Num() * 180);

	auto AppendLine = [&LogText](const FString& Line)
		{
			LogText += Line;
			LogText += LINE_TERMINATOR;
		};

	AppendLine(TEXT("Lunar Console Log"));
	AppendLine(TEXT("================================================================================"));
	AppendLine(FString::Printf(TEXT("Saved At: %s"), *SaveTime.ToString(TEXT("%d.%m.%Y %H:%M:%S"))));
	AppendLine(FString::Printf(TEXT("Project: %s"), FApp::GetProjectName()));
	AppendLine(FString::Printf(TEXT("Platform: %s"), ANSI_TO_TCHAR(FPlatformProperties::PlatformName())));
	AppendLine(FString::Printf(TEXT("Message Count: %d"), Messages.Num()));
	AppendLine(TEXT("================================================================================"));
	AppendLine(TEXT(""));

	for (const FLunarConsoleMessage& Message : Messages)
	{
		AppendLine(Message.FormattedText);
	}

	AppendLine(TEXT(""));
	AppendLine(TEXT("================================================================================"));
	AppendLine(TEXT("End Of Console Log"));

	if (!ULunarFLFile::SaveTextFileAs(WorldContextObject, TargetDirectoryPath, FileNameWithExtension, LogText, OutSavedFilePath, OutError))
	{
		return false;
	}

	ConsoleSubsystem->AddMessage(ConsoleCategory, ELunarConsoleMessageVerbosity::Success, FString::Printf(TEXT("Console log saved: %s"), *OutSavedFilePath));
	return true;
}
