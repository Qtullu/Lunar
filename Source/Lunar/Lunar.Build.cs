// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class Lunar : ModuleRules
{
    public Lunar(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[]
            {
                Path.Combine(ModuleDirectory, "Public", "FunctionLibraries"),
                Path.Combine(ModuleDirectory, "Public", "Types"),
            }
        );

        PrivateIncludePaths.AddRange(
            new string[]
            {
            }
        );

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "DeveloperSettings",
                "EngineSettings",
                "RHI",
                "InputCore",
                "UMG",
                "Slate",
                "SlateCore",
                "GameplayTags"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Slate",
                "SlateCore",
                "ApplicationCore"
            }
        );

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
            }
        );

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicSystemLibraries.Add("pdh.lib");
            PublicSystemLibraries.AddRange(new string[] { "Ole32.lib", "Shell32.lib" });
        }
    }
}