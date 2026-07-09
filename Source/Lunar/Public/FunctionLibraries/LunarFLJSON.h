// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LunarFLJSON.generated.h"

/**
 * @file LunarFLJSON.h
 * @brief JSON helper function library
 */

 /**
  * @brief Blueprint utility functions for JSON
  * @ingroup LunarFLJSON
  */
UCLASS()
class LUNAR_API ULunarFLJSON : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * @brief Converts JSON text to a Blueprint structure
	 * @param WorldContextObject Object used to resolve the active world
	 * @param Json JSON text
	 * @param OutStruct Structure that receives parsed JSON values
	 * @return True if JSON was converted to the structure
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Lunar|JSON", meta = (DisplayName = "Json To Struct", WorldContext = "WorldContextObject", CustomStructureParam = "OutStruct"))
	static bool JsonToStruct(const UObject* WorldContextObject, const FString& Json, int32& OutStruct);

	/**
	 * @brief Converts a Blueprint structure to JSON text
	 * @param WorldContextObject Object used to resolve the active world
	 * @param Struct Structure that will be converted
	 * @param OutJson JSON text created from the structure
	 * @param bPretty Should JSON be created with formatting
	 * @return True if structure was converted to JSON
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Lunar|JSON", meta = (DisplayName = "Struct To Json", WorldContext = "WorldContextObject", CustomStructureParam = "Struct", AdvancedDisplay = "bPretty"))
	static bool StructToJson(const UObject* WorldContextObject, const int32& Struct, FString& OutJson, bool bPretty = false);

	/**
	 * @brief Loads JSON text from a file and converts it to a Blueprint structure
	 * @param WorldContextObject Object used to resolve the active world
	 * @param FilePath File path to load JSON from
	 * @param OutStruct Structure that receives parsed JSON values
	 * @return True if file was loaded and JSON was converted to the structure
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Lunar|JSON", meta = (DisplayName = "Json File To Struct", WorldContext = "WorldContextObject", CustomStructureParam = "OutStruct"))
	static bool JsonFileToStruct(const UObject* WorldContextObject, const FString& FilePath, int32& OutStruct);

	/**
	 * @brief Converts a Blueprint structure to JSON text and saves it to a file
	 * @param WorldContextObject Object used to resolve the active world
	 * @param Struct Structure that will be converted
	 * @param FilePath File path where JSON will be saved
	 * @param bPretty Should JSON be saved with formatting
	 * @return True if structure was converted and file was saved
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Lunar|JSON", meta = (DisplayName = "Struct To Json File", WorldContext = "WorldContextObject", CustomStructureParam = "Struct", AdvancedDisplay = "bPretty"))
	static bool StructToJsonFile(const UObject* WorldContextObject, const int32& Struct, const FString& FilePath, bool bPretty = true);

	/**
	 * @brief Converts JSON text to a Blueprint array map or set
	 * @param WorldContextObject Object used to resolve the active world
	 * @param Json JSON text
	 * @param OutContainer Array map or set that receives parsed JSON values
	 * @return True if JSON was converted to the container
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Lunar|JSON", meta = (DisplayName = "Json To Container", WorldContext = "WorldContextObject", CustomStructureParam = "OutContainer"))
	static bool JsonToContainer(const UObject* WorldContextObject, const FString& Json, int32& OutContainer);

	/**
	 * @brief Converts a Blueprint array map or set to JSON text
	 * @param WorldContextObject Object used to resolve the active world
	 * @param Container Array map or set that will be converted
	 * @param OutJson JSON text created from the container
	 * @param bPretty Should JSON be created with formatting
	 * @return True if container was converted to JSON
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Lunar|JSON", meta = (DisplayName = "Container To Json", WorldContext = "WorldContextObject", CustomStructureParam = "Container", AdvancedDisplay = "bPretty"))
	static bool ContainerToJson(const UObject* WorldContextObject, const int32& Container, FString& OutJson, bool bPretty = false);

	/**
	 * @brief Checks if text is a valid JSON object
	 * @param WorldContextObject Object used to resolve the active world
	 * @param Json JSON text
	 * @return True if text is a valid JSON object
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|JSON", meta = (DisplayName = "Is Valid Json", WorldContext = "WorldContextObject"))
	static bool IsValidJson(const UObject* WorldContextObject, const FString& Json);

public:
	DECLARE_FUNCTION(execJsonToStruct);
	DECLARE_FUNCTION(execStructToJson);
	DECLARE_FUNCTION(execJsonFileToStruct);
	DECLARE_FUNCTION(execStructToJsonFile);
	DECLARE_FUNCTION(execJsonToContainer);
	DECLARE_FUNCTION(execContainerToJson);
};