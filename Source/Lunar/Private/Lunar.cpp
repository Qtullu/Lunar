// Copyright Epic Games, Inc. All Rights Reserved.

#include "Lunar.h"
#include "GameplayTagsManager.h"


#define LOCTEXT_NAMESPACE "FLunarModule"

void FLunarModule::StartupModule()
{
	UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();

	TagsManager.AddNativeGameplayTag(TEXT("Lunar"), TEXT("Root tag for Lunar plugin."));
	TagsManager.AddNativeGameplayTag(TEXT("Lunar.Console"), TEXT("Lunar console messages and commands."));
}
void FLunarModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLunarModule, Lunar)