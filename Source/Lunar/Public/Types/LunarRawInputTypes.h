// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "LunarRawInputTypes.generated.h"

/**
 * @file LunarRawInputTypes.h
 * @brief Raw input shared types
 * @ingroup LunarTypesRawInput
 */

 /**
  * @brief Defines the last detected input device type
  * @ingroup LunarTypesRawInput
  */
UENUM(BlueprintType)
enum class ELunarInputDeviceType : uint8
{
	Unknown UMETA(DisplayName = "Unknown"),
	KeyboardMouse UMETA(DisplayName = "Keyboard & Mouse"),
	Gamepad UMETA(DisplayName = "Gamepad"),
	Touch UMETA(DisplayName = "Touch")
};

/**
 * @brief Stores raw input state captured by Lunar input systems
 * @ingroup LunarTypesRawInput
 */
USTRUCT(BlueprintType)
struct FLunarRawInputSnapshot
{
	GENERATED_BODY()

public:
	/** Last detected input device */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Input")
	ELunarInputDeviceType LastInputDevice = ELunarInputDeviceType::Unknown;

	/** Current mouse position */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Input")
	FVector2D MousePosition = FVector2D::ZeroVector;

	/** Processed mouse movement delta */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Input")
	FVector2D MouseDelta = FVector2D::ZeroVector;

	/** Raw mouse movement delta */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Input")
	FVector2D RawMouseDelta = FVector2D::ZeroVector;

	/** Mouse wheel movement delta */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Input")
	float MouseWheelDelta = 0.0f;

	/** Seconds passed since last input */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Input")
	float SecondsSinceLastInput = 0.0f;

	/** Last pressed key */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Input")
	FKey LastKey = EKeys::Invalid;
};