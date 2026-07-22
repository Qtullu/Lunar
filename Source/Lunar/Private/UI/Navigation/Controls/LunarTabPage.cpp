// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarTabPage.cpp
 * @brief Implements stable Tabs context assignment for owner-authored pages.
 * @ingroup LunarNavigationControls
 */

#include "UI/Navigation/Controls/LunarTabPage.h"

#include "UI/Navigation/Controls/LunarTabs.h"

void ULunarTabPage::AssignTabContext(ULunarTabs* InTabsOwner, const FName InTabId)
{
	const FName PreviousTabId = TabId;
	const bool bContextChanged = TabsOwner != InTabsOwner || TabId != InTabId;
	TabsOwner = InTabsOwner;
	TabId = InTabId;
	if (bContextChanged)
	{
		BP_OnLunarTabContextChanged(TabsOwner, PreviousTabId, TabId);
	}
}
