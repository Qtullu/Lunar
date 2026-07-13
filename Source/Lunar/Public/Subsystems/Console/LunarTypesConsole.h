// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "InputCoreTypes.h"
#include "LunarTypesConsole.generated.h"

/**
 * @file LunarTypesConsole.h
 * @brief Console shared types
 * @ingroup LunarTypesConsole
 */

 /**
  * @brief Defines Lunar console message verbosity
  * @ingroup LunarTypesConsole
  */
UENUM(BlueprintType)
enum class ELunarConsoleMessageVerbosity : uint8
{
	Message UMETA(DisplayName = "Message"),
	Success UMETA(DisplayName = "Success"),
	Info UMETA(DisplayName = "Info"),
	Warning UMETA(DisplayName = "Warning"),
	Error UMETA(DisplayName = "Error"),
	Fatal UMETA(DisplayName = "Fatal"),
	Debug UMETA(DisplayName = "Debug"),
	Trace UMETA(DisplayName = "Trace")
};

/**
 * @brief Defines parameter type accepted by Lunar console commands
 * @ingroup LunarTypesConsole
 */
UENUM(BlueprintType)
enum class ELunarConsoleCommandParameterType : uint8
{
	None UMETA(DisplayName = "None"),
	Bool UMETA(DisplayName = "Bool"),
	Int UMETA(DisplayName = "Int"),
	Float UMETA(DisplayName = "Float"),
	Vector2D UMETA(DisplayName = "Vector2D"),
	Vector3D UMETA(DisplayName = "Vector3D"),
	String UMETA(DisplayName = "String")
};

/**
 * @brief Defines input mode behavior after closing the Lunar console widget
 * @ingroup LunarTypesConsole
 */
UENUM(BlueprintType)
enum class ELunarConsoleInputModeAfterClose : uint8
{
	DoNotChange UMETA(DisplayName = "Do Not Change"),
	GameOnly UMETA(DisplayName = "Game Only"),
	GameAndUI UMETA(DisplayName = "Game And UI"),
	UIOnly UMETA(DisplayName = "UI Only")
};

/**
 * @brief Stores data table registration for Lunar console commands
 * @ingroup LunarTypesConsole
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarConsoleCommandTable
{
	GENERATED_BODY()

public:
	/** Command prefix used for commands from this table */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console")
	FName Prefix = NAME_None;

	/** Data table that contains console command rows */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console")
	TObjectPtr<UDataTable> CommandsDataTable = nullptr;

	/** Enables this command table */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console")
	bool bEnabled = true;
};

/**
 * @brief Stores one Lunar console message
 * @ingroup LunarTypesConsole
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarConsoleMessage
{
	GENERATED_BODY()

public:
	/** Message time */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Console")
	FDateTime Time;

	/** Message category */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Console")
	FGameplayTag Category;

	/** Message verbosity */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Console")
	ELunarConsoleMessageVerbosity Verbosity = ELunarConsoleMessageVerbosity::Message;

	/** Raw message text */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Console")
	FString Text;

	/** Formatted message text */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Console")
	FString FormattedText;

	/** Message display color */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Console")
	FLinearColor Color = FLinearColor::White;
};

/**
 * @brief Defines one Lunar console command row loaded from a data table
 * @ingroup LunarTypesConsole
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarConsoleCommandRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	/** Target class that owns the command function */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console")
	TSubclassOf<UObject> Class;

	/** Target function name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console")
	FName FunctionName = NAME_None;

	/** Console command name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console")
	FName Command = NAME_None;

	/** Command parameter type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console")
	ELunarConsoleCommandParameterType ParameterType = ELunarConsoleCommandParameterType::None;

	/** Preview value used in command suggestions */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console")
	FString PreviewValue;

	/** Command description */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console")
	FString Description;

	/** Enables this command row */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console")
	bool bEnabled = true;
};

/**
 * @brief Stores resolved Lunar console command data
 * @ingroup LunarTypesConsole
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarConsoleCommandDefinition
{
	GENERATED_BODY()

public:
	/** Target class that owns the command function */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Console")
	TSubclassOf<UObject> Class;

	/** Target function name */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Console")
	FName FunctionName = NAME_None;

	/** Console command name */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Console")
	FName Command = NAME_None;

	/** Command parameter type */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Console")
	ELunarConsoleCommandParameterType ParameterType = ELunarConsoleCommandParameterType::None;

	/** Preview value used in command suggestions */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Console")
	FString PreviewValue;

	/** Command description */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Console")
	FString Description;

	/** Enables this command definition */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Console")
	bool bEnabled = true;
};

/**
 * @brief Stores Lunar console configuration
 * @ingroup LunarTypesConsole
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarConsoleSettings
{
	GENERATED_BODY()

public:
	/** Enables Lunar console */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console")
	bool bEnabled = true;

	/** Enables Lunar console in editor builds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Environment")
	bool bEnabledInEditor = true;

	/** Enables Lunar console in debug builds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Environment")
	bool bEnabledInDebug = true;

	/** Enables Lunar console in development builds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Environment")
	bool bEnabledInDevelopment = true;

	/** Enables Lunar console in test builds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Environment")
	bool bEnabledInTest = true;

	/** Enables Lunar console in shipping builds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Environment")
	bool bEnabledInShipping = false;

	/** Maximum stored console message count */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Messages", meta = (ClampMin = "1"))
	int32 MaxMessages = 10000;

	/** Registered console command tables */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Commands")
	TArray<FLunarConsoleCommandTable> CommandTables;

	/** Key used to toggle the console widget */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Widget")
	FKey ConsoleHotkey = EKeys::F9;

	/** Console widget class path */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Widget", meta = (MetaClass = "/Script/UMG.UserWidget"))
	FSoftClassPath ConsoleWidgetClass = FSoftClassPath(TEXT("/Lunar/Widgets/Global/W_Console.W_Console_C"));

	/** Allows Lunar console to manage mouse cursor state */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Widget")
	bool bManageMouseCursor = true;

	/** Shows mouse cursor while the console widget is open */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Widget")
	bool bShowMouseCursorWhenOpen = true;

	/** Applies input mode while the console widget is open */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Widget")
	bool bSetInputModeWhenOpen = true;

	/** Input mode applied after closing the console widget */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Widget")
	ELunarConsoleInputModeAfterClose InputModeAfterClose = ELunarConsoleInputModeAfterClose::GameOnly;

	/** Prints console messages to output log */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Log")
	bool bPrintToOutputLog = true;

	/** Adds timestamp to formatted console text */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Log")
	bool bAddTimestampToFormattedText = true;

	/** Timestamp format used for formatted console text */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Log")
	FString TimestampFormat = TEXT("%d.%m.%Y %H:%M:%S");

	/** Color used for message verbosity */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Colors")
	FLinearColor MessageColor = FLinearColor(0.80f, 0.80f, 0.80f, 1.0f);

	/** Color used for success verbosity */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Colors")
	FLinearColor SuccessColor = FLinearColor(0.25f, 0.90f, 0.35f, 1.0f);

	/** Color used for info verbosity */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Colors")
	FLinearColor InfoColor = FLinearColor(0.35f, 0.65f, 1.00f, 1.0f);

	/** Color used for warning verbosity */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Colors")
	FLinearColor WarningColor = FLinearColor(1.00f, 0.75f, 0.20f, 1.0f);

	/** Color used for error verbosity */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Colors")
	FLinearColor ErrorColor = FLinearColor(1.00f, 0.20f, 0.20f, 1.0f);

	/** Color used for fatal verbosity */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Colors")
	FLinearColor FatalColor = FLinearColor(1.00f, 0.00f, 0.00f, 1.0f);

	/** Color used for debug verbosity */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Colors")
	FLinearColor DebugColor = FLinearColor(0.70f, 0.45f, 1.00f, 1.0f);

	/** Color used for trace verbosity */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Colors")
	FLinearColor TraceColor = FLinearColor(0.45f, 0.45f, 0.45f, 1.0f);
};