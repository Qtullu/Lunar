// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Navigation/Styles/LunarWidgetStyleAsset.h"
#include "UI/Navigation/Types/LunarUIStyleTypes.h"
#include "LunarTabsStyleAsset.generated.h"

/** @file LunarTabsStyleAsset.h @brief Typed Tabs style Data Asset. @ingroup LunarNavigationStyles */

/** @brief Strongly typed style asset for ULunarTabs. @ingroup LunarNavigationStyles */
UCLASS(BlueprintType)
class LUNAR_API ULunarTabsStyleAsset : public ULunarWidgetStyleAsset
{
	GENERATED_BODY()
public:
	/** @brief Base style layer resolved before value and interaction states. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Style") FLunarTabsStylePatch BaseStyle;
	/** @brief Optional style patches selected by the current value-state Gameplay Tag. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Style", meta = (Categories = "Lunar.UI.State.Value")) TMap<FGameplayTag, FLunarTabsStylePatch> ValueStateStyles;
	/** @brief Optional style patches selected by the current pointer or navigation interaction state. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Style") TMap<ELunarUIInteractionState, FLunarTabsStylePatch> InteractionStateStyles;
};
