// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "Subsystems/LunarConsoleSubsystem.h"
#include "Settings/LunarSettings.h"
#include "Subsystems/LunarRawInputSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/UnrealType.h"

TWeakObjectPtr<ULunarConsoleSubsystem> ULunarConsoleSubsystem::ActiveConsoleSubsystem;

void ULunarConsoleSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency(ULunarRawInputSubsystem::StaticClass());

	ReloadCommands();
	BindRawInput();

	ActiveConsoleSubsystem = this;
}

void ULunarConsoleSubsystem::Deinitialize()
{
	UnbindRawInput();

	if (ActiveConsoleSubsystem.Get() == this)
	{
		ActiveConsoleSubsystem.Reset();
	}

	if (ConsoleWidgetInstance)
	{
		if (ConsoleWidgetInstance->IsInViewport())
		{
			RestorePreviousInputMode();
		}

		ConsoleWidgetInstance->RemoveFromParent();
		ConsoleWidgetInstance = nullptr;
	}

	Messages.Empty();
	CommandsByName.Empty();

	Super::Deinitialize();
}

ULunarConsoleSubsystem* ULunarConsoleSubsystem::Get()
{
	return ActiveConsoleSubsystem.Get();
}

void ULunarConsoleSubsystem::AddMessage(FGameplayTag Category, ELunarConsoleMessageVerbosity Verbosity, const FString& Text)
{
	if (ULunarConsoleSubsystem* ConsoleSubsystem = ActiveConsoleSubsystem.Get())
	{
		ConsoleSubsystem->BP_AddMessage(Category, Verbosity, Text);
	}
}

void ULunarConsoleSubsystem::BP_AddMessage(FGameplayTag Category, ELunarConsoleMessageVerbosity Verbosity, const FString& Text)
{
	const FLunarConsoleSettings& Settings = GetConsoleSettings();

	if (!CanUseConsole())
	{
		return;
	}

	FLunarConsoleMessage Message;
	Message.Time = FDateTime::Now();
	Message.Category = Category;
	Message.Verbosity = Verbosity;
	Message.Text = Text;
	Message.Color = GetColorForVerbosity(Verbosity);
	Message.FormattedText = BuildFormattedText(Message);

	Messages.Add(Message);

	const int32 RemovedCount = TrimMessagesIfNeeded();

	if (RemovedCount > 0)
	{
		OnMessagesRemoved.Broadcast(RemovedCount);
	}

	if (Settings.bPrintToOutputLog)
	{
		switch (Verbosity)
		{
		case ELunarConsoleMessageVerbosity::Warning:
			UE_LOG(LogTemp, Warning, TEXT("%s"), *Message.FormattedText);
			break;

		case ELunarConsoleMessageVerbosity::Error:
		case ELunarConsoleMessageVerbosity::Fatal:
			UE_LOG(LogTemp, Error, TEXT("%s"), *Message.FormattedText);
			break;

		default:
			UE_LOG(LogTemp, Log, TEXT("%s"), *Message.FormattedText);
			break;
		}
	}

	OnMessageAdded.Broadcast(Message);
}

void ULunarConsoleSubsystem::ClearMessages()
{
	Messages.Empty();
	OnMessagesCleared.Broadcast();
}

const TArray<FLunarConsoleMessage>& ULunarConsoleSubsystem::GetMessages() const
{
	return Messages;
}

void ULunarConsoleSubsystem::ReloadCommands()
{
	CommandsByName.Empty();

	const FLunarConsoleSettings& Settings = GetConsoleSettings();

	if (Settings.CommandTables.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("LunarConsole: CommandTables is empty."));
		return;
	}

	for (const FLunarConsoleCommandTable& CommandTable : Settings.CommandTables)
	{
		if (!CommandTable.bEnabled)
		{
			continue;
		}

		if (!CommandTable.CommandsDataTable)
		{
			UE_LOG(LogTemp, Warning, TEXT("LunarConsole: CommandsDataTable is not set for prefix: %s"), *CommandTable.Prefix.ToString());
			continue;
		}

		TArray<FLunarConsoleCommandRow*> Rows;
		CommandTable.CommandsDataTable->GetAllRows<FLunarConsoleCommandRow>(TEXT("LunarConsoleCommands"), Rows);

		for (const FLunarConsoleCommandRow* Row : Rows)
		{
			if (!Row)
			{
				continue;
			}

			if (!Row->bEnabled || Row->Command.IsNone() || Row->FunctionName.IsNone() || !Row->Class)
			{
				continue;
			}

			const FString RawCommandString = Row->Command.ToString();
			FString FullCommandString = RawCommandString;

			if (!CommandTable.Prefix.IsNone())
			{
				FullCommandString = CommandTable.Prefix.ToString() + TEXT(".") + RawCommandString;
			}

			const FName FullCommandName = FName(*FullCommandString);

			if (CommandsByName.Contains(FullCommandName))
			{
				UE_LOG(LogTemp, Warning, TEXT("LunarConsole: Duplicate command skipped: %s"), *FullCommandString);
				continue;
			}

			FLunarConsoleCommandDefinition Definition;
			Definition.Class = Row->Class;
			Definition.FunctionName = Row->FunctionName;
			Definition.Command = FullCommandName;
			Definition.ParameterType = Row->ParameterType;
			Definition.PreviewValue = Row->PreviewValue;
			Definition.Description = Row->Description;
			Definition.bEnabled = Row->bEnabled;

			CommandsByName.Add(Definition.Command, Definition);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("LunarConsole: Loaded commands: %d"), CommandsByName.Num());
}

TArray<FLunarConsoleCommandDefinition> ULunarConsoleSubsystem::GetCommands() const
{
	TArray<FLunarConsoleCommandDefinition> Result;
	CommandsByName.GenerateValueArray(Result);

	Result.Sort([](const FLunarConsoleCommandDefinition& A, const FLunarConsoleCommandDefinition& B)
		{
			return A.Command.LexicalLess(B.Command);
		});

	return Result;
}

TArray<FLunarConsoleCommandDefinition> ULunarConsoleSubsystem::GetCommandSuggestions(const FString& Input) const
{
	FString CleanInput = Input;
	CleanInput.TrimStartAndEndInline();

	FString CommandPart = CleanInput;
	FString ParameterPart;

	if (CleanInput.Split(TEXT("="), &CommandPart, &ParameterPart))
	{
		CommandPart.TrimStartAndEndInline();
	}
	else if (CleanInput.Split(TEXT(" "), &CommandPart, &ParameterPart))
	{
		CommandPart.TrimStartAndEndInline();
	}

	TArray<FLunarConsoleCommandDefinition> Result;

	for (const TPair<FName, FLunarConsoleCommandDefinition>& Pair : CommandsByName)
	{
		const FString CommandString = Pair.Key.ToString();

		if (CommandPart.IsEmpty() || CommandString.StartsWith(CommandPart, ESearchCase::IgnoreCase))
		{
			Result.Add(Pair.Value);
		}
	}

	Result.Sort([](const FLunarConsoleCommandDefinition& A, const FLunarConsoleCommandDefinition& B)
		{
			return A.Command.LexicalLess(B.Command);
		});

	return Result;
}

TArray<FString> ULunarConsoleSubsystem::GetCommandSuggestionStrings(const FString& Input) const
{
	const TArray<FLunarConsoleCommandDefinition> Suggestions = GetCommandSuggestions(Input);

	TArray<FString> Result;
	Result.Reserve(Suggestions.Num());

	for (const FLunarConsoleCommandDefinition& Suggestion : Suggestions)
	{
		Result.Add(BuildSuggestionString(Suggestion));
	}

	return Result;
}

TArray<FString> ULunarConsoleSubsystem::GetAllCommandSuggestionStrings() const
{
	return GetCommandSuggestionStrings(TEXT(""));
}

bool ULunarConsoleSubsystem::ExecuteCommandString(const FString& Input, FString& OutError)
{
	if (!CanUseConsole())
	{
		OutError = TEXT("Console is disabled for this environment.");
		return false;
	}

	FName CommandName;
	FString ParameterString;

	if (!ParseInput(Input, CommandName, ParameterString))
	{
		OutError = TEXT("Empty command.");
		UE_LOG(LogTemp, Error, TEXT("LunarConsole: %s"), *OutError);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("LunarConsole: Input='%s', Command='%s', Parameter='%s'"), *Input, *CommandName.ToString(), *ParameterString);

	AddMessage(GetDefaultConsoleCategoryTag(), ELunarConsoleMessageVerbosity::Info, FString::Printf(TEXT("> %s"), *Input));

	const FLunarConsoleCommandDefinition* Command = CommandsByName.Find(CommandName);

	if (!Command)
	{
		OutError = FString::Printf(TEXT("Unknown command: %s. Loaded commands: %d"), *CommandName.ToString(), CommandsByName.Num());
		UE_LOG(LogTemp, Error, TEXT("LunarConsole: %s"), *OutError);
		AddMessage(GetDefaultConsoleCategoryTag(), ELunarConsoleMessageVerbosity::Error, OutError);
		return false;
	}

	const bool bResult = ExecuteCommand(*Command, ParameterString, OutError);

	if (bResult)
	{
		UE_LOG(LogTemp, Log, TEXT("LunarConsole: Execute success: %s"), *Input);
	}
	else
	{
		AddMessage(GetDefaultConsoleCategoryTag(), ELunarConsoleMessageVerbosity::Error, OutError);
		UE_LOG(LogTemp, Error, TEXT("LunarConsole: Execute failed: %s"), *OutError);
	}

	return bResult;
}

void ULunarConsoleSubsystem::ToggleConsoleWidget()
{
	if (IsConsoleWidgetVisible())
	{
		HideConsoleWidget();
		return;
	}

	ShowConsoleWidget();
}

void ULunarConsoleSubsystem::ShowConsoleWidget()
{
	if (!CanUseConsole())
	{
		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	if (!ConsoleWidgetInstance)
	{
		TSubclassOf<UUserWidget> WidgetClass = LoadConsoleWidgetClass();

		if (!WidgetClass)
		{
			AddMessage(GetDefaultConsoleCategoryTag(), ELunarConsoleMessageVerbosity::Error, TEXT("Console widget class is invalid."));
			return;
		}

		ConsoleWidgetInstance = CreateWidget<UUserWidget>(World, WidgetClass);

		if (!ConsoleWidgetInstance)
		{
			AddMessage(GetDefaultConsoleCategoryTag(), ELunarConsoleMessageVerbosity::Error, TEXT("Failed to create console widget."));
			return;
		}
	}

	if (!ConsoleWidgetInstance->IsInViewport())
	{
		ConsoleWidgetInstance->AddToViewport(9999);
	}

	ApplyConsoleInputMode();
}

void ULunarConsoleSubsystem::HideConsoleWidget()
{
	if (!ConsoleWidgetInstance)
	{
		return;
	}

	if (ConsoleWidgetInstance->IsInViewport())
	{
		ConsoleWidgetInstance->RemoveFromParent();
	}

	RestorePreviousInputMode();
}

bool ULunarConsoleSubsystem::IsConsoleWidgetVisible() const
{
	return ConsoleWidgetInstance && ConsoleWidgetInstance->IsInViewport();
}

bool ULunarConsoleSubsystem::CanUseConsole() const
{
	const FLunarConsoleSettings& Settings = GetConsoleSettings();

	if (!Settings.bEnabled)
	{
		return false;
	}

#if WITH_EDITOR
	if (GIsEditor && !Settings.bEnabledInEditor)
	{
		return false;
	}
#endif

#if UE_BUILD_DEBUG
	return Settings.bEnabledInDebug;
#elif UE_BUILD_DEVELOPMENT
	return Settings.bEnabledInDevelopment;
#elif UE_BUILD_TEST
	return Settings.bEnabledInTest;
#elif UE_BUILD_SHIPPING
	return Settings.bEnabledInShipping;
#else
	return true;
#endif
}

const FLunarConsoleSettings& ULunarConsoleSubsystem::GetConsoleSettings() const
{
	const ULunarSettings* Settings = GetDefault<ULunarSettings>();
	return Settings->Console;
}

FLinearColor ULunarConsoleSubsystem::GetColorForVerbosity(ELunarConsoleMessageVerbosity Verbosity) const
{
	const FLunarConsoleSettings& Settings = GetConsoleSettings();

	switch (Verbosity)
	{
	case ELunarConsoleMessageVerbosity::Success:
		return Settings.SuccessColor;

	case ELunarConsoleMessageVerbosity::Info:
		return Settings.InfoColor;

	case ELunarConsoleMessageVerbosity::Warning:
		return Settings.WarningColor;

	case ELunarConsoleMessageVerbosity::Error:
		return Settings.ErrorColor;

	case ELunarConsoleMessageVerbosity::Fatal:
		return Settings.FatalColor;

	case ELunarConsoleMessageVerbosity::Debug:
		return Settings.DebugColor;

	case ELunarConsoleMessageVerbosity::Trace:
		return Settings.TraceColor;

	case ELunarConsoleMessageVerbosity::Message:
	default:
		return Settings.MessageColor;
	}
}

FString ULunarConsoleSubsystem::BuildFormattedText(const FLunarConsoleMessage& Message) const
{
	const FLunarConsoleSettings& Settings = GetConsoleSettings();

	FString Prefix;

	if (Settings.bAddTimestampToFormattedText)
	{
		Prefix += FString::Printf(TEXT("[%s] "), *Message.Time.ToString(*Settings.TimestampFormat));
	}

	if (Message.Category.IsValid())
	{
		Prefix += FString::Printf(TEXT("[%s] "), *Message.Category.ToString());
	}

	return Prefix + Message.Text;
}

int32 ULunarConsoleSubsystem::TrimMessagesIfNeeded()
{
	const FLunarConsoleSettings& ConsoleSettings = GetConsoleSettings();

	if (ConsoleSettings.MaxMessages <= 0)
	{
		return 0;
	}

	const int32 RemovedCount = Messages.Num() - ConsoleSettings.MaxMessages;

	if (RemovedCount <= 0)
	{
		return 0;
	}

	Messages.RemoveAt(0, RemovedCount, EAllowShrinking::No);
	return RemovedCount;
}

bool ULunarConsoleSubsystem::ParseInput(const FString& Input, FName& OutCommand, FString& OutParameter) const
{
	FString CleanInput = Input;
	CleanInput.TrimStartAndEndInline();

	if (CleanInput.IsEmpty())
	{
		return false;
	}

	FString CommandPart;
	FString ParameterPart;

	if (CleanInput.Split(TEXT("="), &CommandPart, &ParameterPart))
	{
		OutCommand = FName(*CommandPart.TrimStartAndEnd());
		OutParameter = ParameterPart.TrimStartAndEnd();
		return !OutCommand.IsNone();
	}

	if (CleanInput.Split(TEXT(" "), &CommandPart, &ParameterPart))
	{
		OutCommand = FName(*CommandPart.TrimStartAndEnd());
		OutParameter = ParameterPart.TrimStartAndEnd();
		return !OutCommand.IsNone();
	}

	OutCommand = FName(*CleanInput);
	OutParameter.Reset();

	return !OutCommand.IsNone();
}

bool ULunarConsoleSubsystem::ExecuteCommand(const FLunarConsoleCommandDefinition& Command, const FString& ParameterString, FString& OutError)
{
	UObject* Target = ResolveTargetObject(Command);

	if (!Target)
	{
		OutError = FString::Printf(TEXT("Target object not found for command: %s"), *Command.Command.ToString());
		return false;
	}

	UFunction* Function = Target->FindFunction(Command.FunctionName);

	if (!Function)
	{
		OutError = FString::Printf(TEXT("Function not found: %s"), *Command.FunctionName.ToString());
		return false;
	}

	TArray<FProperty*> UserInputProperties;
	FProperty* WorldContextProperty = nullptr;

	for (TFieldIterator<FProperty> It(Function); It && (It->PropertyFlags & CPF_Parm); ++It)
	{
		FProperty* Property = *It;

		if (!Property || Property->HasAnyPropertyFlags(CPF_ReturnParm | CPF_OutParm))
		{
			continue;
		}

		const FString PropertyName = Property->GetName();

		if (PropertyName == TEXT("__WorldContext") || PropertyName == TEXT("WorldContextObject") || PropertyName.Contains(TEXT("WorldContext")))
		{
			WorldContextProperty = Property;
			continue;
		}

		UserInputProperties.Add(Property);
	}

	const int32 ExpectedUserParams = Command.ParameterType == ELunarConsoleCommandParameterType::None ? 0 : 1;

	if (UserInputProperties.Num() != ExpectedUserParams)
	{
		OutError = FString::Printf(
			TEXT("Function parameter count mismatch for command: %s. Expected user params: %d, found user params: %d"),
			*Command.Command.ToString(),
			ExpectedUserParams,
			UserInputProperties.Num()
		);

		return false;
	}

	uint8* ParamsBuffer = static_cast<uint8*>(FMemory_Alloca(Function->ParmsSize));
	FMemory::Memzero(ParamsBuffer, Function->ParmsSize);

	for (TFieldIterator<FProperty> It(Function); It && (It->PropertyFlags & CPF_Parm); ++It)
	{
		It->InitializeValue_InContainer(ParamsBuffer);
	}

	if (WorldContextProperty)
	{
		FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(WorldContextProperty);

		if (ObjectProperty)
		{
			UObject* WorldContextObject = GetWorld();
			ObjectProperty->SetObjectPropertyValue_InContainer(ParamsBuffer, WorldContextObject);
		}
	}

	if (ExpectedUserParams == 1)
	{
		if (!WriteParameterToBuffer(Command, ParameterString, ParamsBuffer, UserInputProperties[0], OutError))
		{
			for (TFieldIterator<FProperty> It(Function); It && (It->PropertyFlags & CPF_Parm); ++It)
			{
				It->DestroyValue_InContainer(ParamsBuffer);
			}

			return false;
		}
	}

	Target->ProcessEvent(Function, ParamsBuffer);

	for (TFieldIterator<FProperty> It(Function); It && (It->PropertyFlags & CPF_Parm); ++It)
	{
		It->DestroyValue_InContainer(ParamsBuffer);
	}

	return true;
}

UObject* ULunarConsoleSubsystem::ResolveTargetObject(const FLunarConsoleCommandDefinition& Command) const
{
	UClass* TargetClass = Command.Class.Get();

	if (!TargetClass)
	{
		return nullptr;
	}

	if (TargetClass->IsChildOf(UBlueprintFunctionLibrary::StaticClass()))
	{
		return TargetClass->GetDefaultObject();
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		return TargetClass->GetDefaultObject();
	}

	if (TargetClass->IsChildOf(AActor::StaticClass()))
	{
		for (TActorIterator<AActor> It(World, TargetClass); It; ++It)
		{
			AActor* Actor = *It;

			if (!Actor || Actor->IsPendingKillPending())
			{
				continue;
			}

			return Actor;
		}

		return nullptr;
	}

	for (TObjectIterator<UObject> It; It; ++It)
	{
		UObject* Object = *It;

		if (!Object || !Object->IsA(TargetClass))
		{
			continue;
		}

		if (Object->GetWorld() != World)
		{
			continue;
		}

		return Object;
	}

	return TargetClass->GetDefaultObject();
}

bool ULunarConsoleSubsystem::WriteParameterToBuffer(const FLunarConsoleCommandDefinition& Command, const FString& ParameterString, uint8* ParamBuffer, FProperty* Property, FString& OutError) const
{
	if (!Property)
	{
		OutError = TEXT("Invalid function parameter.");
		return false;
	}

	if (Command.ParameterType != ELunarConsoleCommandParameterType::String && ParameterString.IsEmpty())
	{
		OutError = FString::Printf(TEXT("Missing parameter for command: %s"), *Command.Command.ToString());
		return false;
	}

	void* ValuePtr = Property->ContainerPtrToValuePtr<void>(ParamBuffer);

	switch (Command.ParameterType)
	{
	case ELunarConsoleCommandParameterType::Bool:
	{
		FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property);

		if (!BoolProperty)
		{
			OutError = TEXT("Function parameter is not bool.");
			return false;
		}

		const FString Lower = ParameterString.ToLower();

		if (Lower == TEXT("true") || Lower == TEXT("1"))
		{
			BoolProperty->SetPropertyValue(ValuePtr, true);
			return true;
		}

		if (Lower == TEXT("false") || Lower == TEXT("0"))
		{
			BoolProperty->SetPropertyValue(ValuePtr, false);
			return true;
		}

		OutError = TEXT("Bool parameter must be true/false or 1/0.");
		return false;
	}

	case ELunarConsoleCommandParameterType::Int:
	{
		FIntProperty* IntProperty = CastField<FIntProperty>(Property);

		if (!IntProperty || !ParameterString.IsNumeric())
		{
			OutError = TEXT("Function parameter is not int or value is invalid.");
			return false;
		}

		IntProperty->SetPropertyValue(ValuePtr, FCString::Atoi(*ParameterString));
		return true;
	}

	case ELunarConsoleCommandParameterType::Float:
	{
		FFloatProperty* FloatProperty = CastField<FFloatProperty>(Property);

		if (!FloatProperty || !ParameterString.IsNumeric())
		{
			OutError = TEXT("Function parameter is not float or value is invalid.");
			return false;
		}

		FloatProperty->SetPropertyValue(ValuePtr, FCString::Atof(*ParameterString));
		return true;
	}

	case ELunarConsoleCommandParameterType::Vector2D:
	{
		FStructProperty* StructProperty = CastField<FStructProperty>(Property);

		if (!StructProperty || StructProperty->Struct != TBaseStructure<FVector2D>::Get())
		{
			OutError = TEXT("Function parameter is not FVector2D.");
			return false;
		}

		FVector2D Value = FVector2D::ZeroVector;

		if (!Value.InitFromString(ParameterString))
		{
			TArray<FString> Parts;
			ParameterString.ParseIntoArrayWS(Parts);

			if (Parts.Num() != 2)
			{
				OutError = TEXT("Vector2D format must be \"X Y\" or \"X=1 Y=2\".");
				return false;
			}

			Value.X = FCString::Atof(*Parts[0]);
			Value.Y = FCString::Atof(*Parts[1]);
		}

		*static_cast<FVector2D*>(ValuePtr) = Value;
		return true;
	}

	case ELunarConsoleCommandParameterType::Vector3D:
	{
		FStructProperty* StructProperty = CastField<FStructProperty>(Property);

		if (!StructProperty || StructProperty->Struct != TBaseStructure<FVector>::Get())
		{
			OutError = TEXT("Function parameter is not FVector.");
			return false;
		}

		FVector Value = FVector::ZeroVector;

		if (!Value.InitFromString(ParameterString))
		{
			TArray<FString> Parts;
			ParameterString.ParseIntoArrayWS(Parts);

			if (Parts.Num() != 3)
			{
				OutError = TEXT("Vector3D format must be \"X Y Z\" or \"X=1 Y=2 Z=3\".");
				return false;
			}

			Value.X = FCString::Atof(*Parts[0]);
			Value.Y = FCString::Atof(*Parts[1]);
			Value.Z = FCString::Atof(*Parts[2]);
		}

		*static_cast<FVector*>(ValuePtr) = Value;
		return true;
	}

	case ELunarConsoleCommandParameterType::String:
	{
		FStrProperty* StringProperty = CastField<FStrProperty>(Property);

		if (!StringProperty)
		{
			OutError = TEXT("Function parameter is not FString.");
			return false;
		}

		StringProperty->SetPropertyValue(ValuePtr, ParameterString);
		return true;
	}

	case ELunarConsoleCommandParameterType::None:
	default:
		return true;
	}
}

FString ULunarConsoleSubsystem::GetDefaultPreviewValueForParameterType(ELunarConsoleCommandParameterType ParameterType) const
{
	switch (ParameterType)
	{
	case ELunarConsoleCommandParameterType::Bool:
		return TEXT("true");

	case ELunarConsoleCommandParameterType::Int:
		return TEXT("0");

	case ELunarConsoleCommandParameterType::Float:
		return TEXT("1.000000");

	case ELunarConsoleCommandParameterType::Vector2D:
		return TEXT("0 0");

	case ELunarConsoleCommandParameterType::Vector3D:
		return TEXT("0 0 0");

	case ELunarConsoleCommandParameterType::String:
		return TEXT("");

	case ELunarConsoleCommandParameterType::None:
	default:
		return TEXT("");
	}
}

FString ULunarConsoleSubsystem::BuildSuggestionString(const FLunarConsoleCommandDefinition& Command) const
{
	FString Result = Command.Command.ToString();

	if (!Command.PreviewValue.IsEmpty())
	{
		Result += TEXT(" ");
		Result += Command.PreviewValue;
		return Result;
	}

	const FString DefaultPreviewValue = GetDefaultPreviewValueForParameterType(Command.ParameterType);

	if (!DefaultPreviewValue.IsEmpty())
	{
		Result += TEXT(" ");
		Result += DefaultPreviewValue;
	}

	return Result;
}

FGameplayTag ULunarConsoleSubsystem::GetDefaultConsoleCategoryTag() const
{
	return FGameplayTag::RequestGameplayTag(TEXT("Lunar.Console"), false);
}

TSubclassOf<UUserWidget> ULunarConsoleSubsystem::LoadConsoleWidgetClass() const
{
	const FLunarConsoleSettings& Settings = GetConsoleSettings();

	UClass* LoadedClass = Settings.ConsoleWidgetClass.TryLoadClass<UUserWidget>();

	if (!LoadedClass)
	{
		return nullptr;
	}

	return LoadedClass;
}

void ULunarConsoleSubsystem::BindRawInput()
{
	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	UGameInstance* GameInstance = World->GetGameInstance();

	if (!GameInstance)
	{
		return;
	}

	ULunarRawInputSubsystem* RawInputSubsystem = GameInstance->GetSubsystem<ULunarRawInputSubsystem>();

	if (!RawInputSubsystem)
	{
		return;
	}

	RawInputSubsystem->OnKeyClicked.RemoveDynamic(this, &ULunarConsoleSubsystem::HandleRawInputKeyClicked);
	RawInputSubsystem->OnKeyClicked.AddDynamic(this, &ULunarConsoleSubsystem::HandleRawInputKeyClicked);
}

void ULunarConsoleSubsystem::UnbindRawInput()
{
	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	UGameInstance* GameInstance = World->GetGameInstance();

	if (!GameInstance)
	{
		return;
	}

	ULunarRawInputSubsystem* RawInputSubsystem = GameInstance->GetSubsystem<ULunarRawInputSubsystem>();

	if (!RawInputSubsystem)
	{
		return;
	}

	RawInputSubsystem->OnKeyClicked.RemoveDynamic(this, &ULunarConsoleSubsystem::HandleRawInputKeyClicked);
}

void ULunarConsoleSubsystem::HandleRawInputKeyClicked(FKey Key)
{
	const FLunarConsoleSettings& Settings = GetConsoleSettings();

	if (Key != Settings.ConsoleHotkey)
	{
		return;
	}

	if (!CanUseConsole())
	{
		return;
	}

	ToggleConsoleWidget();
}

APlayerController* ULunarConsoleSubsystem::GetFirstLocalPlayerController() const
{
	UWorld* World = GetWorld();

	if (!World)
	{
		return nullptr;
	}

	return World->GetFirstPlayerController();
}

void ULunarConsoleSubsystem::ApplyConsoleInputMode()
{
	const FLunarConsoleSettings& Settings = GetConsoleSettings();

	APlayerController* PlayerController = GetFirstLocalPlayerController();

	if (!PlayerController)
	{
		return;
	}

	if (!bCachedPlayerControllerState)
	{
		bCachedShowMouseCursor = PlayerController->bShowMouseCursor;
		bCachedPlayerControllerState = true;
	}

	if (Settings.bManageMouseCursor)
	{
		PlayerController->bShowMouseCursor = Settings.bShowMouseCursorWhenOpen;
	}

	if (!Settings.bSetInputModeWhenOpen)
	{
		return;
	}

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	if (ConsoleWidgetInstance)
	{
		InputMode.SetWidgetToFocus(ConsoleWidgetInstance->TakeWidget());
	}

	PlayerController->SetInputMode(InputMode);
}

void ULunarConsoleSubsystem::RestorePreviousInputMode()
{
	const FLunarConsoleSettings& Settings = GetConsoleSettings();

	APlayerController* PlayerController = GetFirstLocalPlayerController();

	if (!PlayerController)
	{
		bCachedPlayerControllerState = false;
		return;
	}

	if (Settings.bManageMouseCursor && bCachedPlayerControllerState)
	{
		PlayerController->bShowMouseCursor = bCachedShowMouseCursor;
	}

	switch (Settings.InputModeAfterClose)
	{
	case ELunarConsoleInputModeAfterClose::DoNotChange:
	{
		break;
	}

	case ELunarConsoleInputModeAfterClose::GameAndUI:
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(InputMode);
		break;
	}

	case ELunarConsoleInputModeAfterClose::UIOnly:
	{
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(InputMode);
		break;
	}

	case ELunarConsoleInputModeAfterClose::GameOnly:
	default:
	{
		FInputModeGameOnly InputMode;
		PlayerController->SetInputMode(InputMode);
		break;
	}
	}

	bCachedPlayerControllerState = false;
}