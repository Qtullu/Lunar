// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Navigation/Styles/LunarWidgetStyleAsset.h"
#include "UI/Navigation/Types/LunarUIStyleTypes.h"
#include "LunarScrollBoxStyleAsset.generated.h"

/** @file LunarScrollBoxStyleAsset.h @brief Typed ScrollBox style Data Asset. @ingroup LunarNavigationStyles */

/** @brief Strongly typed style asset for ULunarScrollBox. @ingroup LunarNavigationStyles */
UCLASS(BlueprintType)
class LUNAR_API ULunarScrollBoxStyleAsset : public ULunarWidgetStyleAsset
{
	GENERATED_BODY()
public:
	/** @brief Base style layer resolved before value and interaction states. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Style") FLunarScrollBoxStylePatch BaseStyle;
	/** @brief Optional style patches selected by the current value-state Gameplay Tag. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Style", meta = (Categories = "Lunar.UI.State.Value")) TMap<FGameplayTag, FLunarScrollBoxStylePatch> ValueStateStyles;
	/** @brief Optional style patches selected by the current pointer or navigation interaction state. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Style") TMap<ELunarUIInteractionState, FLunarScrollBoxStylePatch> InteractionStateStyles;
};
