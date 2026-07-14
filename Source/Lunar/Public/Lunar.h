// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

/**
 * @file Lunar.h
 * @brief Declares the primary runtime module for the Lunar plugin.
 */

/** @brief Owns startup and shutdown of the Lunar runtime module. */
class FLunarModule : public IModuleInterface
{
public:

	/** @brief Registers runtime resources required when the module starts. */
	virtual void StartupModule() override;

	/** @brief Releases runtime resources before the module unloads. */
	virtual void ShutdownModule() override;
};
