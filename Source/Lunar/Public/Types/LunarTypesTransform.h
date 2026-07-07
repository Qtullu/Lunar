// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "LunarTypesTransform.generated.h"

UENUM(BlueprintType)
enum class ELunarAxisShort : uint8
{
	X UMETA(DisplayName = "X"),
	Y UMETA(DisplayName = "Y"),
	Z UMETA(DisplayName = "Z")
};

UENUM(BlueprintType)
enum class ELunarAxisFull : uint8
{
	X   UMETA(DisplayName = "X"),
	Y   UMETA(DisplayName = "Y"),
	Z   UMETA(DisplayName = "Z"),

	XY  UMETA(DisplayName = "XY"),
	XZ  UMETA(DisplayName = "XZ"),
	YZ  UMETA(DisplayName = "YZ"),

	XYZ UMETA(DisplayName = "XYZ")
};

UENUM(BlueprintType)
enum class ELunarLocationRotationScale : uint8
{
	Location UMETA(DisplayName = "Location"),
	Rotation UMETA(DisplayName = "Rotation"),
	Scale UMETA(DisplayName = "Scale")
};