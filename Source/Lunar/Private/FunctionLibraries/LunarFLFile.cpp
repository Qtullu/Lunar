// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "FunctionLibraries/LunarFLFile.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "GameplayTagContainer.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Subsystems/LunarConsoleSubsystem.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <shobjidl.h>
#include <shlobj.h>
#include <combaseapi.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

bool ULunarFLFile::SelectFolder(const UObject* WorldContextObject, const FString& DialogTitle, const FString& DefaultPath, FString& OutSelectedFolderPath, FString& OutError)
{
	OutSelectedFolderPath.Empty();
	OutError.Empty();

	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;

	if (!World)
	{
		OutError = TEXT("SelectFolder failed: World is invalid.");
		return false;
	}

	UGameInstance* GameInstance = World->GetGameInstance();

	if (!GameInstance)
	{
		OutError = TEXT("SelectFolder failed: GameInstance is invalid.");
		return false;
	}

	ULunarConsoleSubsystem* ConsoleSubsystem = GameInstance->GetSubsystem<ULunarConsoleSubsystem>();
	const FGameplayTag ConsoleCategory = FGameplayTag::RequestGameplayTag(TEXT("Lunar.Console"), false);

#if PLATFORM_WINDOWS
	const FString FinalDialogTitle = DialogTitle.IsEmpty() ? TEXT("Select Folder") : DialogTitle;
	FString FinalDefaultPath = DefaultPath.IsEmpty() ? FPaths::ProjectSavedDir() : DefaultPath;
	FinalDefaultPath = FPaths::ConvertRelativePathToFull(FinalDefaultPath);
	FPaths::NormalizeDirectoryName(FinalDefaultPath);

	HRESULT InitResult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	const bool bShouldUninitializeCom = SUCCEEDED(InitResult);

	if (FAILED(InitResult) && InitResult != RPC_E_CHANGED_MODE)
	{
		OutError = FString::Printf(TEXT("SelectFolder failed: Could not initialize COM. HRESULT: 0x%08X"), static_cast<uint32>(InitResult));
		ConsoleSubsystem->AddMessage(ConsoleCategory, ELunarConsoleMessageVerbosity::Error, OutError);
		return false;
	}

	IFileOpenDialog* OpenDialog = nullptr;
	HRESULT DialogResult = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&OpenDialog));

	if (FAILED(DialogResult) || !OpenDialog)
	{
		if (bShouldUninitializeCom)
		{
			CoUninitialize();
		}

		OutError = FString::Printf(TEXT("SelectFolder failed: Could not create folder dialog. HRESULT: 0x%08X"), static_cast<uint32>(DialogResult));
		ConsoleSubsystem->AddMessage(ConsoleCategory, ELunarConsoleMessageVerbosity::Error, OutError);
		return false;
	}

	DWORD DialogOptions = 0;
	OpenDialog->GetOptions(&DialogOptions);
	OpenDialog->SetOptions(DialogOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
	OpenDialog->SetTitle(*FinalDialogTitle);

	if (FPaths::DirectoryExists(FinalDefaultPath))
	{
		IShellItem* DefaultFolderItem = nullptr;

		if (SUCCEEDED(SHCreateItemFromParsingName(*FinalDefaultPath, nullptr, IID_PPV_ARGS(&DefaultFolderItem))) && DefaultFolderItem)
		{
			OpenDialog->SetFolder(DefaultFolderItem);
			DefaultFolderItem->Release();
		}
	}

	HRESULT ShowResult = OpenDialog->Show(nullptr);

	if (ShowResult == HRESULT_FROM_WIN32(ERROR_CANCELLED))
	{
		OpenDialog->Release();

		if (bShouldUninitializeCom)
		{
			CoUninitialize();
		}

		OutError = TEXT("SelectFolder cancelled.");
		ConsoleSubsystem->AddMessage(ConsoleCategory, ELunarConsoleMessageVerbosity::Warning, OutError);
		return false;
	}

	if (FAILED(ShowResult))
	{
		OpenDialog->Release();

		if (bShouldUninitializeCom)
		{
			CoUninitialize();
		}

		OutError = FString::Printf(TEXT("SelectFolder failed: Could not show folder dialog. HRESULT: 0x%08X"), static_cast<uint32>(ShowResult));
		ConsoleSubsystem->AddMessage(ConsoleCategory, ELunarConsoleMessageVerbosity::Error, OutError);
		return false;
	}

	IShellItem* ResultItem = nullptr;
	HRESULT ResultItemResult = OpenDialog->GetResult(&ResultItem);

	if (FAILED(ResultItemResult) || !ResultItem)
	{
		OpenDialog->Release();

		if (bShouldUninitializeCom)
		{
			CoUninitialize();
		}

		OutError = FString::Printf(TEXT("SelectFolder failed: Could not get selected folder. HRESULT: 0x%08X"), static_cast<uint32>(ResultItemResult));
		ConsoleSubsystem->AddMessage(ConsoleCategory, ELunarConsoleMessageVerbosity::Error, OutError);
		return false;
	}

	PWSTR FolderPath = nullptr;
	HRESULT FolderPathResult = ResultItem->GetDisplayName(SIGDN_FILESYSPATH, &FolderPath);

	if (FAILED(FolderPathResult) || !FolderPath)
	{
		ResultItem->Release();
		OpenDialog->Release();

		if (bShouldUninitializeCom)
		{
			CoUninitialize();
		}

		OutError = FString::Printf(TEXT("SelectFolder failed: Could not get selected folder path. HRESULT: 0x%08X"), static_cast<uint32>(FolderPathResult));
		ConsoleSubsystem->AddMessage(ConsoleCategory, ELunarConsoleMessageVerbosity::Error, OutError);
		return false;
	}

	OutSelectedFolderPath = FString(FolderPath);
	FPaths::NormalizeDirectoryName(OutSelectedFolderPath);

	CoTaskMemFree(FolderPath);
	ResultItem->Release();
	OpenDialog->Release();

	if (bShouldUninitializeCom)
	{
		CoUninitialize();
	}

	return true;
#else
	OutError = TEXT("SelectFolder failed: Folder selection dialog is not supported on this platform.");
	ConsoleSubsystem->AddMessage(ConsoleCategory, ELunarConsoleMessageVerbosity::Warning, OutError);
	return false;
#endif
}

bool ULunarFLFile::SaveTextFileAs(const UObject* WorldContextObject, const FString& DirectoryPath, const FString& FileNameWithExtension, const FString& Text, FString& OutSavedFilePath, FString& OutError)
{
	OutSavedFilePath.Empty();
	OutError.Empty();

	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;

	if (!World)
	{
		OutError = TEXT("SaveTextFileAs failed: World is invalid.");
		return false;
	}

	UGameInstance* GameInstance = World->GetGameInstance();

	if (!GameInstance)
	{
		OutError = TEXT("SaveTextFileAs failed: GameInstance is invalid.");
		return false;
	}

	ULunarConsoleSubsystem* ConsoleSubsystem = GameInstance->GetSubsystem<ULunarConsoleSubsystem>();
	const FGameplayTag ConsoleCategory = FGameplayTag::RequestGameplayTag(TEXT("Lunar.Console"), false);

	FString FinalDirectoryPath = DirectoryPath;
	FinalDirectoryPath.TrimStartAndEndInline();

	if (FinalDirectoryPath.IsEmpty())
	{
		OutError = TEXT("SaveTextFileAs failed: DirectoryPath is empty.");
		ConsoleSubsystem->AddMessage(ConsoleCategory, ELunarConsoleMessageVerbosity::Error, OutError);
		return false;
	}

	FinalDirectoryPath = FPaths::ConvertRelativePathToFull(FinalDirectoryPath);
	FPaths::NormalizeDirectoryName(FinalDirectoryPath);

	FString CleanFileName = FPaths::GetCleanFilename(FileNameWithExtension);
	CleanFileName.TrimStartAndEndInline();

	if (CleanFileName.IsEmpty())
	{
		OutError = TEXT("SaveTextFileAs failed: FileNameWithExtension is empty.");
		ConsoleSubsystem->AddMessage(ConsoleCategory, ELunarConsoleMessageVerbosity::Error, OutError);
		return false;
	}

	CleanFileName = FPaths::MakeValidFileName(CleanFileName);

	if (FPaths::GetExtension(CleanFileName).IsEmpty())
	{
		CleanFileName += TEXT(".txt");
	}

	if (!IFileManager::Get().DirectoryExists(*FinalDirectoryPath) && !IFileManager::Get().MakeDirectory(*FinalDirectoryPath, true))
	{
		OutError = FString::Printf(TEXT("SaveTextFileAs failed: Could not create directory: %s"), *FinalDirectoryPath);
		ConsoleSubsystem->AddMessage(ConsoleCategory, ELunarConsoleMessageVerbosity::Error, OutError);
		return false;
	}

	OutSavedFilePath = FPaths::Combine(FinalDirectoryPath, CleanFileName);
	FPaths::NormalizeFilename(OutSavedFilePath);

	if (!FFileHelper::SaveStringToFile(Text, *OutSavedFilePath, FFileHelper::EEncodingOptions::ForceUTF8))
	{
		OutError = FString::Printf(TEXT("SaveTextFileAs failed: Could not save file: %s"), *OutSavedFilePath);
		ConsoleSubsystem->AddMessage(ConsoleCategory, ELunarConsoleMessageVerbosity::Error, OutError);
		return false;
	}

	return true;
}