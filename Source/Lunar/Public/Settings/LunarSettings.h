// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Subsystems/LunarPerformanceSubsystem.h"
#include "LunarTypesConsole.h"
#include "LunarSettings.generated.h"

/**
 * 
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Lunar"))
class LUNAR_API ULunarSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override
	{
		return TEXT("Game");
	}

	virtual FName GetSectionName() const override
	{
		return TEXT("Lunar");
	}

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (DisplayName = "Settings", ToolTip = "Lunar Performance Subsystem default settings."))
	FLunarPerformanceSettings Performance;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Console", meta = (DisplayName = "Settings", ToolTip = "Lunar Console default settings."))
	FLunarConsoleSettings Console;
};
