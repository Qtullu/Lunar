// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"

class UWidget;

/**
 * @file LunarNativeNavigationValidation.h
 * @brief Editor helpers for detecting and clearing unsupported native UMG navigation data.
 * @ingroup LunarNavigationEditor
 */

/**
 * @brief Editor-only helpers for unsupported native UMG navigation rules.
 * @ingroup LunarNavigationEditor
 */
namespace LunarNativeNavigationValidation
{
	/**
	 * @brief Checks one widget for authored native UMG navigation data.
	 * @param Widget Widget to inspect.
	 * @return True when the widget owns non-default native navigation rules.
	 */
	bool HasNonDefaultNavigation(const UWidget* Widget);

	/**
	 * @brief Checks a widget selection for authored native UMG navigation data.
	 * @param Widgets Widgets to inspect.
	 * @return True when at least one valid widget owns non-default native navigation rules.
	 */
	bool HasNonDefaultNavigation(const TArray<TWeakObjectPtr<UWidget>>& Widgets);

	/**
	 * @brief Clears native UMG navigation from selected widgets and their persistent Blueprint counterparts.
	 * @param Widgets Selected widgets to repair.
	 * @return True when at least one persistent or preview widget was modified.
	 */
	bool ClearNonDefaultNavigation(const TArray<TWeakObjectPtr<UWidget>>& Widgets);
}
