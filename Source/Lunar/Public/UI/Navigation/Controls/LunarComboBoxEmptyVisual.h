// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LunarComboBoxEmptyVisual.generated.h"

class ULunarComboBox;

/** Optional Blueprint presentation generated when a ComboBox filter has no visible options. */
UCLASS(Abstract, Blueprintable)
class LUNAR_API ULunarComboBoxEmptyVisual : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Returns the ComboBox that owns this empty-results visual. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ComboBox|Empty Visual")
	ULunarComboBox* GetOwningComboBox() const { return OwningComboBox; }

	/** Returns the current external filter text. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ComboBox|Empty Visual")
	FText GetFilterText() const { return FilterText; }

	/** ComboBox instance that generated this visual. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Empty Visual")
	TObjectPtr<ULunarComboBox> OwningComboBox = nullptr;

	/** Current external filter text that produced no visible options. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Empty Visual")
	FText FilterText;

protected:
	/** Blueprint event emitted when the empty-results filter text changes. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lunar|UI|ComboBox|Empty Visual|Events", meta = (DisplayName = "On Combo Box Empty Filter Changed"))
	void BP_OnEmptyFilterChanged(const FText& PreviousFilterText, const FText& NewFilterText);

private:
	/** Initializes this generated visual with its owner and filter snapshot. */
	void InitializeFromComboBox(ULunarComboBox* ComboBox, const FText& InitialFilterText);
	/** Applies and publishes a new filter snapshot. */
	void ApplyFilterText(const FText& NewFilterText);

	/** Guards the initial filter publication. */
	bool bHasPublishedFilterText = false;

	friend class ULunarComboBox;
};
