// Copyright 2026 Edgar Frolenkov All rights reserved.

using UnrealBuildTool;

public class LunarEditor : ModuleRules
{
    public LunarEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "AssetTools",
                "ContentBrowser",
                "ContentBrowserData",
                "Core",
                "CoreUObject",
                "Engine",
                "Lunar",
                "PropertyEditor",
                "Slate",
                "SlateCore",
                "ToolMenus",
                "UMG",
                "UMGEditor",
                "UnrealEd"
            }
        );
    }
}
