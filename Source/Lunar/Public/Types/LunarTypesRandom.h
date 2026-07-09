// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "LunarTypesRandom.generated.h"

/**
 * @brief Defines random generator quality and speed level
 * @ingroup LunarTypes
 */
UENUM(BlueprintType)
enum class ELunarRandomQuality : uint8
{
	/**
	 * @brief Uses std minstd rand with about 1x speed and lower quality
	 */
	VeryFast UMETA(DisplayName = "Very Fast"),

	/**
	 * @brief Uses std mt19937 with about 2x to 5x slower speed and good quality
	 */
	Fast UMETA(DisplayName = "Fast"),

	/**
	 * @brief Uses std mt19937 64 with about 2x to 6x slower speed and good 64 bit quality
	 */
	Normal UMETA(DisplayName = "Normal"),

	/**
	 * @brief Uses std ranlux24 with about 5x to 20x slower speed and higher quality
	 */
	Slow UMETA(DisplayName = "Slow"),

	/**
	 * @brief Uses std ranlux48 with about 10x to 50x slower speed and highest quality in this set
	 */
	VerySlow UMETA(DisplayName = "Very Slow")
};

/**
 * @brief Stores deterministic random sequence state
 * @ingroup LunarTypes
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarRandomStream
{
	GENERATED_BODY()

public:
	/**
	 * @brief Initial stream seed
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Random")
	int64 Seed = 0;

	/**
	 * @brief Current stream step
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Random")
	int64 Step = 0;

	/**
	 * @brief Random generator quality used by this stream
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Random")
	ELunarRandomQuality Quality = ELunarRandomQuality::Normal;
};