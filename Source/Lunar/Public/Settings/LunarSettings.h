// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Subsystems/Console/LunarTypesConsole.h"
#include "Subsystems/Performance/LunarTypesPerformance.h"
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
UCLASS(Config = Game, DefaultConfig, BlueprintType, meta = (DisplayName = "Lunar Settings"))
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
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (DisplayName = "Performance Settings", ToolTip = "Lunar Performance Subsystem default settings."))
	FLunarPerformanceSettings Performance;

	/** Lunar console default settings */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Console", meta = (DisplayName = "Console Settings", ToolTip = "Lunar Console default settings."))
	FLunarConsoleSettings Console;

	/**
	 * @brief Gets default Lunar project settings
	 * @return Lunar project settings
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Settings", meta = (DisplayName = "Get Lunar Settings"))
	static ULunarSettings* GetLunarSettings();

	/**
	 * @brief Gets console color for message verbosity
	 * @param Verbosity Message verbosity
	 * @return Console color for message verbosity
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Settings|Console", meta = (DisplayName = "Get Verbosity Color"))
	static FLinearColor GetVerbosityColor(ELunarConsoleMessageVerbosity Verbosity);
};
