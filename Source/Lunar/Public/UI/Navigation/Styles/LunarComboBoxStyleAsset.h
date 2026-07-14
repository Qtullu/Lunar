// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Navigation/Styles/LunarWidgetStyleAsset.h"
#include "UI/Navigation/Types/LunarUIStyleTypes.h"
#include "LunarComboBoxStyleAsset.generated.h"

/** @file LunarComboBoxStyleAsset.h @brief Typed ComboBox style Data Asset. @ingroup LunarNavigationStyles */

/** @brief Strongly typed style asset for ULunarComboBox. @ingroup LunarNavigationStyles */
UCLASS(BlueprintType)
class LUNAR_API ULunarComboBoxStyleAsset : public ULunarWidgetStyleAsset
{
	GENERATED_BODY()
public:
	/** @brief Base style layer resolved before value and interaction states. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Style") FLunarComboBoxStylePatch BaseStyle;
	/** @brief Optional style patches selected by the current value-state Gameplay Tag. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Style", meta = (Categories = "Lunar.UI.State.Value")) TMap<FGameplayTag, FLunarComboBoxStylePatch> ValueStateStyles;
	/** @brief Optional style patches selected by the current pointer or navigation interaction state. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Style") TMap<ELunarUIInteractionState, FLunarComboBoxStylePatch> InteractionStateStyles;
};
