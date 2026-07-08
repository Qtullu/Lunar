// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "InputCoreTypes.h"
#include "LunarTypesConsole.generated.h"

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

UENUM(BlueprintType)
enum class ELunarConsoleInputModeAfterClose : uint8
{
	DoNotChange UMETA(DisplayName = "Do Not Change"),
	GameOnly UMETA(DisplayName = "Game Only"),
	GameAndUI UMETA(DisplayName = "Game And UI"),
	UIOnly UMETA(DisplayName = "UI Only")
};

USTRUCT(BlueprintType)
struct LUNAR_API FLunarConsoleCommandTable
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console")
	FName Prefix = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console")
	TObjectPtr<UDataTable> CommandsDataTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console")
	bool bEnabled = true;
};

USTRUCT(BlueprintType)
struct LUNAR_API FLunarConsoleMessage
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Console")
	FDateTime Time;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Console")
	FGameplayTag Category;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Console")
	ELunarConsoleMessageVerbosity Verbosity = ELunarConsoleMessageVerbosity::Message;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Console")
	FString Text;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Console")
	FString FormattedText;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Console")
	FLinearColor Color = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct LUNAR_API FLunarConsoleCommandRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console")
	TSubclassOf<UObject> Class;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console")
	FName FunctionName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console")
	FName Command = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console")
	ELunarConsoleCommandParameterType ParameterType = ELunarConsoleCommandParameterType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console")
	FString PreviewValue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console")
	bool bEnabled = true;
};

USTRUCT(BlueprintType)
struct LUNAR_API FLunarConsoleCommandDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Console")
	TSubclassOf<UObject> Class;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Console")
	FName FunctionName = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Console")
	FName Command = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Console")
	ELunarConsoleCommandParameterType ParameterType = ELunarConsoleCommandParameterType::None;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Console")
	FString PreviewValue;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Console")
	FString Description;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Console")
	bool bEnabled = true;
};

USTRUCT(BlueprintType)
struct LUNAR_API FLunarConsoleSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Environment")
	bool bEnabledInEditor = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Environment")
	bool bEnabledInDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Environment")
	bool bEnabledInDevelopment = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Environment")
	bool bEnabledInTest = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Environment")
	bool bEnabledInShipping = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Messages", meta = (ClampMin = "1"))
	int32 MaxMessages = 10000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Commands")
	TArray<FLunarConsoleCommandTable> CommandTables;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Widget")
	FKey ConsoleHotkey = EKeys::F9;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Widget", meta = (MetaClass = "/Script/UMG.UserWidget"))
	FSoftClassPath ConsoleWidgetClass = FSoftClassPath(TEXT("/Lunar/Widgets/Global/W_Console.W_Console_C"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Widget")
	bool bManageMouseCursor = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Widget")
	bool bShowMouseCursorWhenOpen = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Widget")
	bool bSetInputModeWhenOpen = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Widget")
	ELunarConsoleInputModeAfterClose InputModeAfterClose = ELunarConsoleInputModeAfterClose::GameOnly;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Log")
	bool bPrintToOutputLog = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Log")
	bool bAddTimestampToFormattedText = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Log")
	FString TimestampFormat = TEXT("%d.%m.%Y %H:%M:%S");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Colors")
	FLinearColor MessageColor = FLinearColor(0.80f, 0.80f, 0.80f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Colors")
	FLinearColor SuccessColor = FLinearColor(0.25f, 0.90f, 0.35f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Colors")
	FLinearColor InfoColor = FLinearColor(0.35f, 0.65f, 1.00f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Colors")
	FLinearColor WarningColor = FLinearColor(1.00f, 0.75f, 0.20f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Colors")
	FLinearColor ErrorColor = FLinearColor(1.00f, 0.20f, 0.20f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Colors")
	FLinearColor FatalColor = FLinearColor(1.00f, 0.00f, 0.00f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Colors")
	FLinearColor DebugColor = FLinearColor(0.70f, 0.45f, 1.00f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Console|Colors")
	FLinearColor TraceColor = FLinearColor(0.45f, 0.45f, 0.45f, 1.0f);
};