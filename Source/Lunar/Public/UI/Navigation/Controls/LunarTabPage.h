// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarTabPage.h
 * @brief Declares the owner-authored page base used by ULunarTabs.
 * @ingroup LunarNavigationControls
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LunarTabPage.generated.h"

class ULunarTabs;

/**
 * Non-navigable owner-authored page hosted by one ULunarTabs descriptor.
 *
 * The page itself creates no scope and receives no Lunar Selection. Descendant Lunar controls
 * remain part of the surrounding screen scope. Tabs assigns the stable TabId after creating or
 * attaching the page so one Blueprint class can present multiple descriptors differently.
 *
 * @ingroup LunarNavigationControls
 */
UCLASS(Abstract, Blueprintable)
class LUNAR_API ULunarTabPage : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Returns the stable descriptor ID assigned by the owning Tabs control. @return Assigned tab ID, or NAME_None before assignment. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Tabs")
	FName GetTabId() const { return TabId; }

	/** Returns the Tabs control currently hosting this page. @return Owning Tabs, or null before assignment. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Tabs")
	ULunarTabs* GetTabsOwner() const { return TabsOwner; }

protected:
	/**
	 * Called after Tabs assigns or changes this page's descriptor context.
	 * @param NewTabsOwner Tabs control hosting the page.
	 * @param PreviousTabId Previously assigned stable ID, or NAME_None.
	 * @param NewTabId Newly assigned stable ID.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lunar|UI|Tabs", meta = (DisplayName = "On Lunar Tab Context Changed"))
	void BP_OnLunarTabContextChanged(ULunarTabs* NewTabsOwner, FName PreviousTabId, FName NewTabId);

private:
	/** Assigns the stable descriptor context before the page becomes visible. @param InTabsOwner Owning Tabs. @param InTabId Stable descriptor ID. */
	void AssignTabContext(ULunarTabs* InTabsOwner, FName InTabId);

	/** Stable descriptor ID assigned by the owning Tabs control. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Lunar|UI|Tabs", meta = (AllowPrivateAccess = "true"))
	FName TabId = NAME_None;

	/** Tabs control currently hosting this page. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Lunar|UI|Tabs", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULunarTabs> TabsOwner = nullptr;

	/** Grants ULunarTabs authority to assign page context. */
	friend class ULunarTabs;
};
