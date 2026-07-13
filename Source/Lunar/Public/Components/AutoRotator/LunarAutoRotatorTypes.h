// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "LunarAutoRotatorTypes.generated.h"

/**
 * @file LunarAutoRotatorTypes.h
 * @brief Auto rotator component shared types
 * @ingroup LunarAutoRotatorComponent
 */

/**
 * @brief Selects how the auto rotator updates rotation
 * @ingroup LunarAutoRotatorComponent
 */
UENUM(BlueprintType)
enum class ELunarAutoRotatorUpdateMode : uint8
{
	/** Updates rotation automatically every component tick */
	EveryTick UMETA(DisplayName = "Every Tick"),

	/** Updates rotation only when Update Rotation Now is called */
	Manual UMETA(DisplayName = "Manual")
};

/**
 * @brief Selects how the auto rotator moves from current to target rotation
 * @ingroup LunarAutoRotatorComponent
 */
UENUM(BlueprintType)
enum class ELunarAutoRotatorInterpolationMode : uint8
{
	/** Applies target rotation immediately */
	Instant UMETA(DisplayName = "Instant"),

	/** Uses distance-scaled rotation interpolation with ease out */
	InterpTo UMETA(DisplayName = "Interp To"),

	/** Uses constant-speed rotation interpolation */
	InterpToConstant UMETA(DisplayName = "Interp To Constant")
};
