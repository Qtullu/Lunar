// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class IPropertyUtilities;
class UWidget;

/**
 * @file LunarNavigableWidgetDetails.h
 * @brief Details customization for Lunar navigation widgets and containers.
 * @ingroup LunarNavigationEditor
 */

/**
 * @brief Hides unsupported UMG fields and exposes a transactional native-navigation repair action.
 * @ingroup LunarNavigationEditor
 */
class FLunarNavigableWidgetDetails final : public IDetailCustomization
{
public:
	/** @brief Creates a Details customization instance. @return New customization instance. */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** @brief Builds the Lunar navigation Details policy. @param DetailBuilder Active Details layout builder. */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	/** @brief Checks the current selection for unsupported native navigation. @return True when repair is available. */
	bool HasNonDefaultNativeNavigation() const;

	/** @brief Resolves visibility for the native-navigation warning row. @return Visible when repair is available. */
	EVisibility GetNativeNavigationErrorVisibility() const;

	/** @brief Clears unsupported native navigation from the current selection. @return Handled Slate reply. */
	FReply ClearNativeNavigation();

	/** @brief Weak references to the widgets represented by the customized Details panel. */
	TArray<TWeakObjectPtr<UWidget>> CustomizedWidgets;

	/** @brief Details utilities used to request a refresh after transactional fixes. */
	TWeakPtr<IPropertyUtilities> PropertyUtilities;
};
