// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Subsystems/LunarPerformanceSubsystem.h"
#include "LunarTypesConsole.h"
#include "LunarSettings.generated.h"

/**
 * @file LunarSettings.h
 * @brief Lunar plugin settings
 * @ingroup LunarPluginSettings
 */

 /**
  * @brief Stores configurable Lunar plugin settings
  * @ingroup LunarPluginSettings
  */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Lunar"))
class LUNAR_API ULunarSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/**
	 * @brief Gets settings category name
	 * @return Settings category name
	 */
	virtual FName GetCategoryName() const override
	{
		return TEXT("Game");
	}

	/**
	 * @brief Gets settings section name
	 * @return Settings section name
	 */
	virtual FName GetSectionName() const override
	{
		return TEXT("Lunar");
	}

public:
	/** Lunar performance subsystem default settings */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (DisplayName = "Settings", ToolTip = "Lunar Performance Subsystem default settings."))
	FLunarPerformanceSettings Performance;

	/** Lunar console default settings */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Console", meta = (DisplayName = "Settings", ToolTip = "Lunar Console default settings."))
	FLunarConsoleSettings Console;
};