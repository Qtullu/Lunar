// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "LunarRawInputTypes.generated.h"

UENUM(BlueprintType)
enum class ELunarInputDeviceType : uint8
{
	Unknown UMETA(DisplayName = "Unknown"),
	KeyboardMouse UMETA(DisplayName = "Keyboard & Mouse"),
	Gamepad UMETA(DisplayName = "Gamepad"),
	Touch UMETA(DisplayName = "Touch")
};

USTRUCT(BlueprintType)
struct FLunarRawInputSnapshot
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Input")
	ELunarInputDeviceType LastInputDevice = ELunarInputDeviceType::Unknown;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Input")
	FVector2D MousePosition = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Input")
	FVector2D MouseDelta = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Input")
	FVector2D RawMouseDelta = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Input")
	float MouseWheelDelta = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Input")
	float SecondsSinceLastInput = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Lunar|Input")
	FKey LastKey = EKeys::Invalid;
};