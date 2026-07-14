// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarTabHeader.h
 * @brief Declares the selectable header generated and owned by ULunarTabs.
 * @ingroup LunarNavigationControls
 */

#pragma once

#include "CoreMinimal.h"
#include "UI/Navigation/Core/LunarNavigableWidget.h"
#include "LunarTabHeader.generated.h"

class ULunarTabs;

/**
 * Selectable header owned by one ULunarTabs control.
 * Selection and active-page state are independent so users may navigate the strip before activating a page.
 * @ingroup LunarNavigationControls
 */
UCLASS(Blueprintable)
class LUNAR_API ULunarTabHeader : public ULunarNavigableWidget
{
	GENERATED_BODY()

public:
	/** Creates an unbound tab header. @param ObjectInitializer Unreal object initializer. */
	ULunarTabHeader(const FObjectInitializer& ObjectInitializer);

	/** Returns the stable ID represented by this header. @return Stable tab ID. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation|Tabs")
	FName GetTabId() const { return TabId; }

	/** Returns the Tabs control that generated this header. @return Owning Tabs, or null before initialization. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation|Tabs")
	ULunarTabs* GetTabsOwner() const { return TabsOwner; }

	/** Stable tab ID assigned by the owning Tabs control. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Tabs")
	FName TabId = NAME_None;

	/** Tabs control that owns routing, activation, and presentation for this header. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Tabs")
	TObjectPtr<ULunarTabs> TabsOwner = nullptr;

protected:
	/** Tests whether owner-managed header navigation or activation can handle an action. @param ActionContext Routed action context. @return True when supported. */
	virtual bool NativeCanHandleLunarAction(const FLunarUIActionContext& ActionContext) const override;
	/** Routes header directions and activation through the owning Tabs control. @param ActionContext Routed action context. @return Lunar routing result. */
	virtual ELunarUIActionResult NativeHandleLunarAction(const FLunarUIActionContext& ActionContext) override;
	/** Tests whether this owned enabled tab may activate. @return True when activation is valid. */
	virtual bool NativeCanActivateLunarWidget() const override;
	/** Requests activation of the owned tab. */
	virtual void NativeOnLunarActivated() override;
	/** Returns the active/inactive state for accessibility. @return Localized accessible value. */
	virtual FText NativeGetLunarAccessibleValueText() const override;
	/** Delegates header style resolution to the owning Tabs control. @param OutStyle Resolved common patch. @param OutError Actionable failure text. @return True on success. */
	virtual bool ResolveCommonStylePatch(FLunarCommonStylePatch& OutStyle, FString& OutError) const override;
	/** Delegates common and indicator presentation to the owning Tabs control. @param ResolvedStyle Resolved common style. */
	virtual void ApplyResolvedCommonStyle(const FLunarCommonStylePatch& ResolvedStyle) override;

private:
	/** Initializes the generated header. @param InTabsOwner Owning Tabs. @param InTabId Stable tab ID. @param bInEnabled Enabled state. @param InDisabledReason Localized disabled explanation. */
	void InitializeTabHeader(ULunarTabs* InTabsOwner, FName InTabId, bool bInEnabled, const FText& InDisabledReason);
	/** Updates the independent active-page visual flag. @param bInActive Whether this header owns the active page. */
	void SetActiveTabHeader(bool bInActive);
	/** Requests owner activation for this header's tab. @return True on success. */
	bool TryActivateOwnedTab();
	/** Emits rejected feedback for a disabled owned tab. */
	void NotifyOwnedTabRejected();

	/** Whether this header currently represents the active page. */
	UPROPERTY(Transient)
	bool bActiveTabHeader = false;

	/** Grants the owning Tabs control access to generated-header initialization. */
	friend class ULunarTabs;
};
