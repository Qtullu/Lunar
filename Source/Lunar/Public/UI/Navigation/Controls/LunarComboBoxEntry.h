// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Navigation/Controls/LunarListViewEntry.h"
#include "UI/Navigation/Types/LunarNavigationTypes.h"
#include "LunarComboBoxEntry.generated.h"

class ULunarComboBox;

/** Complete logical presentation state for one generated ComboBox entry. */
UENUM(BlueprintType)
enum class ELunarComboBoxEntryVisualState : uint8
{
	Enabled,
	EnabledHighlighted,
	EnabledCommitted,
	EnabledHighlightedCommitted,
	Disabled,
	DisabledHighlighted,
	DisabledCommitted,
	DisabledHighlightedCommitted
};

/** Optional Blueprint presentation generated for one visible ComboBox option row. */
UCLASS(Abstract, Blueprintable)
class LUNAR_API ULunarComboBoxEntry : public ULunarListViewEntry
{
	GENERATED_BODY()

public:
	/** Returns the ComboBox that owns this generated row. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ComboBox|Entry")
	ULunarComboBox* GetOwningComboBox() const { return OwningComboBox; }

	/** Returns the full option snapshot represented by this row. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ComboBox|Entry")
	FLunarComboBoxOption GetComboBoxOption() const { return OptionData; }

	/** Returns whether this row is the popup's temporary navigation cursor. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ComboBox|Entry")
	bool IsOptionHighlighted() const { return bIsHighlighted; }

	/** Returns whether this option is the currently committed ComboBox value. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ComboBox|Entry")
	bool IsOptionCommitted() const { return bIsCommitted; }

	/** Returns the combined enabled, highlighted, and committed presentation state. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ComboBox|Entry")
	ELunarComboBoxEntryVisualState GetComboBoxEntryVisualState() const { return EntryVisualState; }

	/** ComboBox instance that generated this presentation row. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Entry")
	TObjectPtr<ULunarComboBox> OwningComboBox = nullptr;

	/** Full option data copied from the owning ComboBox. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Entry")
	FLunarComboBoxOption OptionData;

	/** True while the popup's temporary cursor points at this option. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Entry")
	bool bIsHighlighted = false;

	/** True when this option is the value committed on the closed face. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Entry")
	bool bIsCommitted = false;

	/** Combined logical state derived from enabled, highlighted, and committed flags. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Entry")
	ELunarComboBoxEntryVisualState EntryVisualState = ELunarComboBoxEntryVisualState::Enabled;

protected:
	/** Resolves typed option data for Designer Preview. */
	virtual void NativePreConstruct() override;
	/** Converts generic ListView data changes into ComboBox option changes. */
	virtual void NativeOnListViewItemDataChanged(const FLunarListViewItemData& PreviousData, const FLunarListViewItemData& NewData) override;
	/** Publishes highlight, committed, enabled, and Lunar interaction state. */
	virtual void NativeOnListViewItemVisualStateChanged(const FLunarUIVisualState& NewState, bool bNewIsActiveItem, bool bNewIsSelectedItem) override;

	/** Blueprint event emitted whenever this recycled row receives different option data. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lunar|UI|ComboBox|Entry|Events", meta = (DisplayName = "On Combo Box Option Data Changed"))
	void BP_OnComboBoxOptionDataChanged(const FLunarComboBoxOption& PreviousOption, const FLunarComboBoxOption& NewOption);

	/** Blueprint event emitted whenever this row's popup presentation state changes. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lunar|UI|ComboBox|Entry|Events", meta = (DisplayName = "On Combo Box Entry Visual State Changed"))
	void BP_OnComboBoxEntryVisualStateChanged(const FLunarUIVisualState& NewState, ELunarComboBoxEntryVisualState NewEntryState, bool bNewIsHighlighted, bool bNewIsCommitted, bool bNewIsEnabled);

private:
	/** Converts the generic list payload into the strongly typed ComboBox option snapshot. */
	FLunarComboBoxOption ConvertItemData(const FLunarListViewItemData& SourceItemData) const;
	/** Resolves the typed owner from the generic ListView owner. */
	void ResolveOwningComboBox();
	/** Refreshes cached typed data and optionally publishes the data event. */
	void RefreshTypedSnapshot(bool bPublishDataEvent, const FLunarListViewItemData* PreviousData = nullptr);
	/** Publishes the latest visual state with typed ComboBox flags. */
	void PublishTypedVisualState(const FLunarUIVisualState& NewState, bool bNewIsHighlighted);
	/** Resolves one exhaustive combined state from the three logical entry flags. */
	static ELunarComboBoxEntryVisualState ResolveEntryVisualState(bool bHighlighted, bool bCommitted, bool bEnabled);
};
