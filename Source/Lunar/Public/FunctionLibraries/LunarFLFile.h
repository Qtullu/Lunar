// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LunarFLFile.generated.h"

/**
 * 
 */
UCLASS()
class LUNAR_API ULunarFLFile : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Lunar|File", meta = (WorldContext = "WorldContextObject", DisplayName = "Select Folder"))
	static bool SelectFolder(const UObject* WorldContextObject, const FString& DialogTitle, const FString& DefaultPath, FString& OutSelectedFolderPath, FString& OutError);

	UFUNCTION(BlueprintCallable, Category = "Lunar|File", meta = (WorldContext = "WorldContextObject", DisplayName = "Save Text File As"))
	static bool SaveTextFileAs(const UObject* WorldContextObject, const FString& DirectoryPath, const FString& FileNameWithExtension, const FString& Text, FString& OutSavedFilePath, FString& OutError);
};
