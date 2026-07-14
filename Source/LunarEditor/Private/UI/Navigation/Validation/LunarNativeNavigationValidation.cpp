// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarNativeNavigationValidation.cpp
 * @brief Implements editor detection and repair of unsupported native UMG navigation data.
 * @ingroup LunarNavigationEditor
 */

#include "UI/Navigation/Validation/LunarNativeNavigationValidation.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Blueprint/WidgetNavigation.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Widget.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "ScopedTransaction.h"
#include "WidgetBlueprint.h"

#define LOCTEXT_NAMESPACE "LunarNativeNavigationValidation"

namespace
{
	/**
	 * @brief Finds the Widget Blueprint that persists a preview or template widget.
	 * @param Widget Widget whose owning asset is required.
	 * @return Owning Widget Blueprint, or null when the widget cannot be mapped to one.
	 */
	UWidgetBlueprint* FindOwningWidgetBlueprint(const UWidget* Widget)
	{
		if (!Widget)
		{
			return nullptr;
		}

		if (UWidgetBlueprint* Blueprint = Widget->GetTypedOuter<UWidgetBlueprint>())
		{
			return Blueprint;
		}

		// A nested ULunarNavigableWidget is itself a UUserWidget. Prefer its
		// containing preview root so we map back to the parent WidgetTree template
		// instead of mutating the child widget class CDO.
		const UUserWidget* PreviewRoot = Widget->GetTypedOuter<UUserWidget>();
		if (!PreviewRoot)
		{
			PreviewRoot = Cast<UUserWidget>(Widget);
		}

		return PreviewRoot ? Cast<UWidgetBlueprint>(PreviewRoot->GetClass()->ClassGeneratedBy) : nullptr;
	}

	/**
	 * @brief Collects preview and persistent copies that share a widget's native navigation data.
	 * @param Widget Starting preview or template widget.
	 * @param OutWidgets Receives unique widget copies that may require inspection or repair.
	 * @param OutBlueprints Receives Widget Blueprints that must participate in the transaction.
	 */
	void GatherNavigationCopies(UWidget* Widget, TArray<UWidget*>& OutWidgets, TSet<UWidgetBlueprint*>& OutBlueprints)
	{
		if (!Widget)
		{
			return;
		}

		OutWidgets.AddUnique(Widget);

		UWidgetBlueprint* Blueprint = FindOwningWidgetBlueprint(Widget);

		if (!Blueprint)
		{
			return;
		}

		OutBlueprints.Add(Blueprint);

		// See FindOwningWidgetBlueprint: outer-first is required for nested
		// UUserWidget-derived controls selected in a containing Widget Blueprint.
		UUserWidget* PreviewRoot = Widget->GetTypedOuter<UUserWidget>();
		if (!PreviewRoot)
		{
			PreviewRoot = Cast<UUserWidget>(Widget);
		}

		if (PreviewRoot == Widget)
		{
			if (Blueprint->GeneratedClass)
			{
				OutWidgets.AddUnique(Blueprint->GeneratedClass->GetDefaultObject<UUserWidget>());
			}

			return;
		}

		if (Blueprint->WidgetTree)
		{
			OutWidgets.AddUnique(Blueprint->WidgetTree->FindWidget(Widget->GetFName()));
		}
	}
}

bool LunarNativeNavigationValidation::HasNonDefaultNavigation(const UWidget* Widget)
{
	return Widget && Widget->Navigation && !Widget->Navigation->IsDefaultNavigation();
}

bool LunarNativeNavigationValidation::HasNonDefaultNavigation(
	const TArray<TWeakObjectPtr<UWidget>>& Widgets)
{
	TArray<UWidget*> NavigationCopies;
	TSet<UWidgetBlueprint*> AffectedBlueprints;

	for (const TWeakObjectPtr<UWidget>& WeakWidget : Widgets)
	{
		GatherNavigationCopies(WeakWidget.Get(), NavigationCopies, AffectedBlueprints);
	}

	for (const UWidget* Widget : NavigationCopies)
	{
		if (HasNonDefaultNavigation(Widget))
		{
			return true;
		}
	}

	return false;
}

bool LunarNativeNavigationValidation::ClearNonDefaultNavigation(
	const TArray<TWeakObjectPtr<UWidget>>& Widgets)
{
	TArray<UWidget*> NavigationCopies;
	TSet<UWidgetBlueprint*> AffectedBlueprints;

	for (const TWeakObjectPtr<UWidget>& WeakWidget : Widgets)
	{
		GatherNavigationCopies(WeakWidget.Get(), NavigationCopies, AffectedBlueprints);
	}

	const bool bHasNavigationToClear = HasNonDefaultNavigation(Widgets);

	if (!bHasNavigationToClear)
	{
		return false;
	}

	const FScopedTransaction Transaction(LOCTEXT("ClearNativeNavigation", "Clear Native UMG Navigation"));
	bool bChanged = false;
	for (UWidgetBlueprint* Blueprint : AffectedBlueprints)
	{
		if (Blueprint)
		{
			Blueprint->Modify();
		}
	}

	for (UWidget* Widget : NavigationCopies)
	{
		if (Widget && Widget->Navigation)
		{
			Widget->Modify();
			Widget->Navigation = nullptr;
			bChanged = true;
		}
	}

	if (bChanged)
	{
		for (UWidgetBlueprint* Blueprint : AffectedBlueprints)
		{
			if (Blueprint)
			{
				FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
			}
		}
	}

	return bChanged;
}

#undef LOCTEXT_NAMESPACE
