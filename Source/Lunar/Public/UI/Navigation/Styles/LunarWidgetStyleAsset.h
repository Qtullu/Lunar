// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LunarWidgetStyleAsset.generated.h"

/**
 * @file LunarWidgetStyleAsset.h
 * @brief Base Data Asset for strongly typed Lunar control styles.
 * @ingroup LunarNavigationStyles
 */

/**
 * @brief Abstract parent for strongly typed Lunar control style assets.
 * @ingroup LunarNavigationStyles
 */
UCLASS(Abstract, BlueprintType)
class LUNAR_API ULunarWidgetStyleAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** @brief Optional compatible parent resolved before this asset. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Style")
	TObjectPtr<ULunarWidgetStyleAsset> ParentStyle;
};
