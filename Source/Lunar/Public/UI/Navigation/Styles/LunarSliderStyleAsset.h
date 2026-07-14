// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Navigation/Styles/LunarWidgetStyleAsset.h"
#include "UI/Navigation/Types/LunarUIStyleTypes.h"
#include "LunarSliderStyleAsset.generated.h"

/** @file LunarSliderStyleAsset.h @brief Typed Slider style Data Asset. @ingroup LunarNavigationStyles */

/** @brief Strongly typed style asset for ULunarSlider. @ingroup LunarNavigationStyles */
UCLASS(BlueprintType)
class LUNAR_API ULunarSliderStyleAsset : public ULunarWidgetStyleAsset
{
	GENERATED_BODY()
public:
	/** @brief Base style layer resolved before value and interaction states. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Style") FLunarSliderStylePatch BaseStyle;
	/** @brief Optional style patches selected by the current value-state Gameplay Tag. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Style", meta = (Categories = "Lunar.UI.State.Value")) TMap<FGameplayTag, FLunarSliderStylePatch> ValueStateStyles;
	/** @brief Optional style patches selected by the current pointer or navigation interaction state. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Style") TMap<ELunarUIInteractionState, FLunarSliderStylePatch> InteractionStateStyles;
};
