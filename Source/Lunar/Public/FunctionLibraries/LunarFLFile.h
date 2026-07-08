// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LunarFLFile.generated.h"

/**
 * @file LunarFLFile.h
 * @brief File helper function library
 * @ingroup LunarFLFile
 */

 /**
  * @brief Blueprint utility functions for files
  * @ingroup LunarFLFile
  */
UCLASS()
class LUNAR_API ULunarFLFile : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * @brief Opens a native folder selection dialog
	 * @param WorldContextObject World context object
	 * @param DialogTitle Dialog title text
	 * @param DefaultPath Initial folder path
	 * @param OutSelectedFolderPath Selected folder path
	 * @param OutError Error text when selection fails
	 * @return True if a folder was selected
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|File", meta = (WorldContext = "WorldContextObject", DisplayName = "Select Folder"))
	static bool SelectFolder(const UObject* WorldContextObject, const FString& DialogTitle, const FString& DefaultPath, FString& OutSelectedFolderPath, FString& OutError);

	/**
	 * @brief Saves text to a selected file path
	 * @param WorldContextObject World context object
	 * @param DirectoryPath Initial directory path
	 * @param FileNameWithExtension Default file name with extension
	 * @param Text Text to save
	 * @param OutSavedFilePath Saved file path
	 * @param OutError Error text when saving fails
	 * @return True if text file was saved
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|File", meta = (WorldContext = "WorldContextObject", DisplayName = "Save Text File As"))
	static bool SaveTextFileAs(const UObject* WorldContextObject, const FString& DirectoryPath, const FString& FileNameWithExtension, const FString& Text, FString& OutSavedFilePath, FString& OutError);
};