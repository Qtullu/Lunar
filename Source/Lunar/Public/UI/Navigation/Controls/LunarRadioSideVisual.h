// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Navigation/Controls/LunarRadio.h"
#include "LunarRadioSideVisual.generated.h"

/**
 * @file LunarRadioSideVisual.h
 * @brief Optional non-navigable presentation widget generated beside one Lunar Radio option.
 * @ingroup LunarNavigationControls
 */

/**
 * @brief Blueprint-owned non-navigable presentation generated for one internal Radio option.
 * @ingroup LunarNavigationControls
 *
 * The owning Radio initializes OptionIndex, OptionData, OwningRadio, VisualState,
 * and bIsChecked before this widget's PreConstruct and Construct lifecycle runs.
 */
UCLASS(Abstract, Blueprintable)
class LUNAR_API ULunarRadioSideVisual : public UUserWidget
{
	GENERATED_BODY()

public:
	/** @return Zero-based option index owned by this generated visual. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Radio|Side Visual")
	int32 GetOptionIndex() const { return OptionIndex; }

	/** @return Current technical and localized data for this option. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Radio|Side Visual")
	FLunarRadioSideVisualData GetOptionData() const { return OptionData; }

	/** @return Radio that generated and owns this visual. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Radio|Side Visual")
	ULunarRadio* GetOwningRadio() const { return OwningRadio; }

	/** @return Current option-specific visual state derived by the owning Radio. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Radio|Side Visual")
	FLunarUIVisualState GetRadioOptionVisualState() const { return VisualState; }

	/** @return True when this option is the Radio's current logical selection. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Radio|Side Visual")
	bool IsRadioOptionChecked() const { return bIsChecked; }

	/** Zero-based option index assigned before PreConstruct and Construct. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|Radio|Side Visual")
	int32 OptionIndex = INDEX_NONE;

	/** Technical StringValue and localized DisplayText assigned before construction. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|Radio|Side Visual")
	FLunarRadioSideVisualData OptionData;

	/** Radio that owns this generated presentation widget. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|Radio|Side Visual")
	TObjectPtr<ULunarRadio> OwningRadio = nullptr;

	/** Latest option-specific state resolved from pointer, navigation, value, device, and reduced-motion state. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|Radio|Side Visual")
	FLunarUIVisualState VisualState;

	/** Whether this generated option is the current logical Radio selection. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|Radio|Side Visual")
	bool bIsChecked = false;

protected:
	/** @param PreviousData Data visible before the runtime update. @param NewData Newly assigned option data. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lunar|UI|Radio|Side Visual|Events", meta = (DisplayName = "On Radio Option Data Changed"))
	void BP_OnRadioOptionDataChanged(const FLunarRadioSideVisualData& PreviousData, const FLunarRadioSideVisualData& NewData);

	/** @param NewState Newly resolved option-specific state. @param bNewIsChecked Whether this option is logically selected. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lunar|UI|Radio|Side Visual|Events", meta = (DisplayName = "On Radio Option Visual State Changed"))
	void BP_OnRadioOptionVisualStateChanged(const FLunarUIVisualState& NewState, bool bNewIsChecked);

private:
	/** @param Radio Owning composite Radio. @param NewOptionIndex Zero-based generated index. @param InitialData Data exposed before construction. @param InitialState Initial option presentation state. @param bInitiallyChecked Initial logical selection flag. @brief Initializes fields before the Slate/Blueprint construction lifecycle begins. */
	void InitializeFromRadio(
		ULunarRadio* Radio,
		int32 NewOptionIndex,
		const FLunarRadioSideVisualData& InitialData,
		const FLunarUIVisualState& InitialState,
		bool bInitiallyChecked);

	/** @param NewData Runtime replacement data. @brief Updates data and publishes the Blueprint data-change event when effective content changes. */
	void ApplyDataFromRadio(const FLunarRadioSideVisualData& NewData);

	/** @param NewState Newly resolved option-specific visual state. @param bNewIsChecked New logical selection flag. @brief Updates presentation fields and publishes the Blueprint visual-state event. */
	void ApplyVisualStateFromRadio(const FLunarUIVisualState& NewState, bool bNewIsChecked);

	/** Whether the initial option-specific visual state has been published to Blueprint. */
	bool bHasPublishedVisualState = false;

	/** Grants the composite Radio controlled access to initialization and update helpers. */
	friend class ULunarRadio;
};
