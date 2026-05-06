// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "LunarTypesGame.generated.h"

UENUM(BlueprintType)
enum class ELunarPlatformType : uint8
{
	Unknown UMETA(DisplayName = "Unknown"),

	Windows UMETA(DisplayName = "Windows"),
	Mac UMETA(DisplayName = "Mac"),
	Linux UMETA(DisplayName = "Linux"),

	Android UMETA(DisplayName = "Android"),
	IOS UMETA(DisplayName = "iOS"),

	PlayStation UMETA(DisplayName = "PlayStation"),
	Xbox UMETA(DisplayName = "Xbox"),
	Switch UMETA(DisplayName = "Nintendo Switch")
};

UENUM(BlueprintType)
enum class ELunarPlatformFamily : uint8
{
	Unknown UMETA(DisplayName = "Unknown"),
	Desktop UMETA(DisplayName = "Desktop"),
	Mobile UMETA(DisplayName = "Mobile"),
	Console UMETA(DisplayName = "Console")
};