// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "FunctionLibraries/LunarFLJSON.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "HAL/FileManager.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "NativeGameplayTags.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Policies/PrettyJsonPrintPolicy.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Subsystems/LunarConsoleSubsystem.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Lunar_JSON, "Lunar.JSON");

bool ULunarFLJSON::JsonToStruct(const UObject* WorldContextObject, const FString& Json, int32& OutStruct)
{
	return false;
}

bool ULunarFLJSON::StructToJson(const UObject* WorldContextObject, const int32& Struct, FString& OutJson, bool bPretty)
{
	OutJson.Empty();
	return false;
}

bool ULunarFLJSON::JsonFileToStruct(const UObject* WorldContextObject, const FString& FilePath, int32& OutStruct)
{
	return false;
}

bool ULunarFLJSON::StructToJsonFile(const UObject* WorldContextObject, const int32& Struct, const FString& FilePath, bool bPretty)
{
	return false;
}

bool ULunarFLJSON::JsonToContainer(const UObject* WorldContextObject, const FString& Json, int32& OutContainer)
{
	return false;
}

bool ULunarFLJSON::ContainerToJson(const UObject* WorldContextObject, const int32& Container, FString& OutJson, bool bPretty)
{
	OutJson.Empty();
	return false;
}

bool ULunarFLJSON::IsValidJson(const UObject* WorldContextObject, const FString& Json)
{
	if (Json.TrimStartAndEnd().IsEmpty())
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_JSON, ELunarConsoleMessageVerbosity::Warning, TEXT("IsValidJson failed: JSON text is empty"));
		return false;
	}

	TSharedPtr<FJsonObject> JsonObject;
	const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Json);
	const bool bResult = FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid();

	if (!bResult)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_JSON, ELunarConsoleMessageVerbosity::Warning, TEXT("IsValidJson failed: JSON text is not a valid JSON object"));
		return false;
	}

	return true;
}

DEFINE_FUNCTION(ULunarFLJSON::execJsonToStruct)
{
	P_GET_OBJECT(UObject, WorldContextObject);
	P_GET_PROPERTY(FStrProperty, Json);

	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* StructAddress = Stack.MostRecentPropertyAddress;
	FStructProperty* StructProperty = CastField<FStructProperty>(Stack.MostRecentProperty);

	P_FINISH;

	P_NATIVE_BEGIN;

	bool bResult = false;

	do
	{
		if (!StructProperty || !StructProperty->Struct || !StructAddress)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_JSON, ELunarConsoleMessageVerbosity::Error, TEXT("JsonToStruct failed: Struct pin is invalid"));
			break;
		}

		if (Json.TrimStartAndEnd().IsEmpty())
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_JSON, ELunarConsoleMessageVerbosity::Error, TEXT("JsonToStruct failed: JSON text is empty"));
			break;
		}

		TSharedPtr<FJsonObject> JsonObject;
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Json);

		if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_JSON, ELunarConsoleMessageVerbosity::Error, TEXT("JsonToStruct failed: JSON text is not a valid JSON object"));
			break;
		}

		FText FailReason;
		bResult = FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), StructProperty->Struct, StructAddress, 0, 0, false, &FailReason, nullptr);

		if (!bResult)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_JSON, ELunarConsoleMessageVerbosity::Error, FString::Printf(TEXT("JsonToStruct failed: %s"), FailReason.IsEmpty() ? TEXT("Unknown conversion error") : *FailReason.ToString()));
			break;
		}
	} while (false);

	*(bool*)RESULT_PARAM = bResult;

	P_NATIVE_END;
}

DEFINE_FUNCTION(ULunarFLJSON::execStructToJson)
{
	P_GET_OBJECT(UObject, WorldContextObject);

	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* StructAddress = Stack.MostRecentPropertyAddress;
	FStructProperty* StructProperty = CastField<FStructProperty>(Stack.MostRecentProperty);

	P_GET_PROPERTY_REF(FStrProperty, OutJson);
	P_GET_UBOOL(bPretty);

	P_FINISH;

	P_NATIVE_BEGIN;

	bool bResult = false;
	OutJson.Empty();

	do
	{
		if (!StructProperty || !StructProperty->Struct || !StructAddress)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_JSON, ELunarConsoleMessageVerbosity::Error, TEXT("StructToJson failed: Struct pin is invalid"));
			break;
		}

		bResult = FJsonObjectConverter::UStructToJsonObjectString(StructProperty->Struct, StructAddress, OutJson, 0, 0, 0, nullptr, bPretty);

		if (!bResult)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_JSON, ELunarConsoleMessageVerbosity::Error, TEXT("StructToJson failed: Struct could not be converted to JSON"));
			break;
		}
	} while (false);

	*(bool*)RESULT_PARAM = bResult;

	P_NATIVE_END;
}

DEFINE_FUNCTION(ULunarFLJSON::execJsonFileToStruct)
{
	P_GET_OBJECT(UObject, WorldContextObject);
	P_GET_PROPERTY(FStrProperty, FilePath);

	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* StructAddress = Stack.MostRecentPropertyAddress;
	FStructProperty* StructProperty = CastField<FStructProperty>(Stack.MostRecentProperty);

	P_FINISH;

	P_NATIVE_BEGIN;

	bool bResult = false;

	do
	{
		if (!StructProperty || !StructProperty->Struct || !StructAddress)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_JSON, ELunarConsoleMessageVerbosity::Error, TEXT("JsonFileToStruct failed: Struct pin is invalid"));
			break;
		}

		if (FilePath.TrimStartAndEnd().IsEmpty())
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_JSON, ELunarConsoleMessageVerbosity::Error, TEXT("JsonFileToStruct failed: File path is empty"));
			break;
		}

		FString Json;

		if (!FFileHelper::LoadFileToString(Json, *FilePath))
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_JSON, ELunarConsoleMessageVerbosity::Error, FString::Printf(TEXT("JsonFileToStruct failed: File could not be loaded from path %s"), *FilePath));
			break;
		}

		if (Json.TrimStartAndEnd().IsEmpty())
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_JSON, ELunarConsoleMessageVerbosity::Error, TEXT("JsonFileToStruct failed: Loaded JSON text is empty"));
			break;
		}

		TSharedPtr<FJsonObject> JsonObject;
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Json);

		if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_JSON, ELunarConsoleMessageVerbosity::Error, TEXT("JsonFileToStruct failed: Loaded text is not a valid JSON object"));
			break;
		}

		FText FailReason;
		bResult = FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), StructProperty->Struct, StructAddress, 0, 0, false, &FailReason, nullptr);

		if (!bResult)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_JSON, ELunarConsoleMessageVerbosity::Error, FString::Printf(TEXT("JsonFileToStruct failed: %s"), FailReason.IsEmpty() ? TEXT("Unknown conversion error") : *FailReason.ToString()));
			break;
		}
	} while (false);

	*(bool*)RESULT_PARAM = bResult;

	P_NATIVE_END;
}

DEFINE_FUNCTION(ULunarFLJSON::execStructToJsonFile)
{
	P_GET_OBJECT(UObject, WorldContextObject);

	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* StructAddress = Stack.MostRecentPropertyAddress;
	FStructProperty* StructProperty = CastField<FStructProperty>(Stack.MostRecentProperty);

	P_GET_PROPERTY(FStrProperty, FilePath);
	P_GET_UBOOL(bPretty);

	P_FINISH;

	P_NATIVE_BEGIN;

	bool bResult = false;

	do
	{
		if (!StructProperty || !StructProperty->Struct || !StructAddress)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_JSON, ELunarConsoleMessageVerbosity::Error, TEXT("StructToJsonFile failed: Struct pin is invalid"));
			break;
		}

		if (FilePath.TrimStartAndEnd().IsEmpty())
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_JSON, ELunarConsoleMessageVerbosity::Error, TEXT("StructToJsonFile failed: File path is empty"));
			break;
		}

		FString Json;
		const bool bConverted = FJsonObjectConverter::UStructToJsonObjectString(StructProperty->Struct, StructAddress, Json, 0, 0, 0, nullptr, bPretty);

		if (!bConverted)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_JSON, ELunarConsoleMessageVerbosity::Error, TEXT("StructToJsonFile failed: Struct could not be converted to JSON"));
			break;
		}

		const FString DirectoryPath = FPaths::GetPath(FilePath);

		if (!DirectoryPath.IsEmpty() && !IFileManager::Get().DirectoryExists(*DirectoryPath))
		{
			IFileManager::Get().MakeDirectory(*DirectoryPath, true);
		}

		if (!FFileHelper::SaveStringToFile(Json, *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_None))
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_JSON, ELunarConsoleMessageVerbosity::Error, FString::Printf(TEXT("StructToJsonFile failed: File could not be saved to path %s"), *FilePath));
			break;
		}

		bResult = true;
	} while (false);

	*(bool*)RESULT_PARAM = bResult;

	P_NATIVE_END;
}

DEFINE_FUNCTION(ULunarFLJSON::execJsonToContainer)
{
	P_GET_OBJECT(UObject, WorldContextObject);
	P_GET_PROPERTY(FStrProperty, Json);

	Stack.StepCompiledIn<FProperty>(nullptr);
	void* ContainerAddress = Stack.MostRecentPropertyAddress;
	FProperty* ContainerProperty = Stack.MostRecentProperty;

	P_FINISH;

	P_NATIVE_BEGIN;

	bool bResult = false;

	do
	{
		if (!ContainerProperty || !ContainerAddress)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_JSON, ELunarConsoleMessageVerbosity::Error, TEXT("JsonToContainer failed: Container pin is invalid"));
			break;
		}

		const bool bIsArray = CastField<FArrayProperty>(ContainerProperty) != nullptr;
		const bool bIsMap = CastField<FMapProperty>(ContainerProperty) != nullptr;
		const bool bIsSet = CastField<FSetProperty>(ContainerProperty) != nullptr;

		if (!bIsArray && !bIsMap && !bIsSet)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_JSON, ELunarConsoleMessageVerbosity::Error, TEXT("JsonToContainer failed: Container pin must be an array map or set"));
			break;
		}

		if (Json.TrimStartAndEnd().IsEmpty())
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_JSON, ELunarConsoleMessageVerbosity::Error, TEXT("JsonToContainer failed: JSON text is empty"));
			break;
		}

		TSharedPtr<FJsonValue> JsonValue;

		if (bIsArray || bIsSet)
		{
			TArray<TSharedPtr<FJsonValue>> JsonArray;
			const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Json);

			if (!FJsonSerializer::Deserialize(JsonReader, JsonArray))
			{
				ULunarConsoleSubsystem::AddMessage(TAG_Lunar_JSON, ELunarConsoleMessageVerbosity::Error, TEXT("JsonToContainer failed: JSON text is not a valid JSON array"));
				break;
			}

			JsonValue = MakeShared<FJsonValueArray>(JsonArray);
		}

		if (bIsMap)
		{
			TSharedPtr<FJsonObject> JsonObject;
			const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Json);

			if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
			{
				ULunarConsoleSubsystem::AddMessage(TAG_Lunar_JSON, ELunarConsoleMessageVerbosity::Error, TEXT("JsonToContainer failed: JSON text is not a valid JSON object"));
				break;
			}

			JsonValue = MakeShared<FJsonValueObject>(JsonObject.ToSharedRef());
		}

		if (!JsonValue.IsValid())
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_JSON, ELunarConsoleMessageVerbosity::Error, TEXT("JsonToContainer failed: JSON value is invalid"));
			break;
		}

		FText FailReason;
		bResult = FJsonObjectConverter::JsonValueToUProperty(JsonValue, ContainerProperty, ContainerAddress, 0, 0, false, &FailReason, nullptr);

		if (!bResult)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_JSON, ELunarConsoleMessageVerbosity::Error, FString::Printf(TEXT("JsonToContainer failed: %s"), FailReason.IsEmpty() ? TEXT("Unknown conversion error") : *FailReason.ToString()));
			break;
		}
	} while (false);

	*(bool*)RESULT_PARAM = bResult;

	P_NATIVE_END;
}

DEFINE_FUNCTION(ULunarFLJSON::execContainerToJson)
{
	P_GET_OBJECT(UObject, WorldContextObject);

	Stack.StepCompiledIn<FProperty>(nullptr);
	void* ContainerAddress = Stack.MostRecentPropertyAddress;
	FProperty* ContainerProperty = Stack.MostRecentProperty;

	P_GET_PROPERTY_REF(FStrProperty, OutJson);
	P_GET_UBOOL(bPretty);

	P_FINISH;

	P_NATIVE_BEGIN;

	bool bResult = false;
	OutJson.Empty();

	do
	{
		if (!ContainerProperty || !ContainerAddress)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_JSON, ELunarConsoleMessageVerbosity::Error, TEXT("ContainerToJson failed: Container pin is invalid"));
			break;
		}

		const bool bIsArray = CastField<FArrayProperty>(ContainerProperty) != nullptr;
		const bool bIsMap = CastField<FMapProperty>(ContainerProperty) != nullptr;
		const bool bIsSet = CastField<FSetProperty>(ContainerProperty) != nullptr;

		if (!bIsArray && !bIsMap && !bIsSet)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_JSON, ELunarConsoleMessageVerbosity::Error, TEXT("ContainerToJson failed: Container pin must be an array map or set"));
			break;
		}

		const TSharedPtr<FJsonValue> JsonValue = FJsonObjectConverter::UPropertyToJsonValue(ContainerProperty, ContainerAddress, 0, 0, nullptr, nullptr, EJsonObjectConversionFlags::None);

		if (!JsonValue.IsValid())
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_JSON, ELunarConsoleMessageVerbosity::Error, TEXT("ContainerToJson failed: Container could not be converted to JSON"));
			break;
		}

		if (bPretty)
		{
			TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&OutJson);
			bResult = FJsonSerializer::Serialize(JsonValue.ToSharedRef(), TEXT(""), JsonWriter);
			JsonWriter->Close();
		}
		else
		{
			TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&OutJson);
			bResult = FJsonSerializer::Serialize(JsonValue.ToSharedRef(), TEXT(""), JsonWriter);
			JsonWriter->Close();
		}

		if (!bResult)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_JSON, ELunarConsoleMessageVerbosity::Error, TEXT("ContainerToJson failed: JSON text could not be written"));
			break;
		}
	} while (false);

	*(bool*)RESULT_PARAM = bResult;

	P_NATIVE_END;
}