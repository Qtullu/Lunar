// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"

class UClass;
class UFactory;
class UToolMenu;
class FDeferredCleanupSlateBrush;

/**
 * @file LunarAssetCreationMenu.h
 * @brief Declares the editor-only Lunar asset-creation menu integration.
 * @ingroup LunarNavigationEditor
 */

/**
 * @brief Adds curated Lunar widget and navigation Data Asset commands to the Content Browser.
 * @ingroup LunarNavigationEditor
 */
class FLunarAssetCreationMenu final
{
public:
	/** @brief Registers the deferred ToolMenus startup callback. */
	void Register();

	/** @brief Removes the startup callback and every ToolMenus entry owned by this integration. */
	void Unregister();

private:
	/** @brief Loads the GC-safe brush used by the Lunar root-menu tile. */
	void RegisterRootMenuIcon();

	/** @brief Releases the Lunar root-menu tile brush. */
	void UnregisterRootMenuIcon();

	/** @brief Extends the Content Browser Add New menu after ToolMenus initialization. */
	void RegisterMenus();

	/**
	 * @brief Adds the Lunar root submenu when one writable package path is selected.
	 * @param Menu Active Content Browser Add New menu.
	 */
	void PopulateAddNewMenu(UToolMenu* Menu);

	/**
	 * @brief Adds the top-level Lunar asset groups.
	 * @param Menu Lunar submenu being populated.
	 * @param PackagePath Internal package path that receives new assets.
	 */
	void PopulateLunarMenu(UToolMenu* Menu, FString PackagePath);

	/**
	 * @brief Adds the supported Lunar Widget Blueprint parent commands.
	 * @param Menu Widgets submenu being populated.
	 * @param PackagePath Internal package path that receives new assets.
	 */
	void PopulateWidgetMenu(UToolMenu* Menu, FString PackagePath);

	/**
	 * @brief Adds the navigation configuration Data Asset commands.
	 * @param Menu UI Navigation submenu being populated.
	 * @param PackagePath Internal package path that receives new assets.
	 */
	void PopulateNavigationMenu(UToolMenu* Menu, FString PackagePath);

	/**
	 * @brief Adds one Widget Blueprint creation command.
	 * @param Menu Menu receiving the entry.
	 * @param SectionName Section that groups the entry.
	 * @param EntryName Stable ToolMenus entry identifier.
	 * @param Label User-visible entry label.
	 * @param ToolTip User-visible entry description.
	 * @param PackagePath Internal package path that receives the asset.
	 * @param ParentClass Lunar UserWidget parent class.
	 * @param BaseAssetName Default editable asset name before uniqueness suffixing.
	 */
	static void AddWidgetBlueprintEntry(
		UToolMenu* Menu,
		FName SectionName,
		FName EntryName,
		const FText& Label,
		const FText& ToolTip,
		const FString& PackagePath,
		UClass* ParentClass,
		const FString& BaseAssetName);

	/**
	 * @brief Adds one reusable Blueprint class creation command for a non-UserWidget control.
	 * @param Menu Menu receiving the entry.
	 * @param SectionName Section that groups the entry.
	 * @param EntryName Stable ToolMenus entry identifier.
	 * @param Label User-visible entry label.
	 * @param ToolTip User-visible entry description.
	 * @param PackagePath Internal package path that receives the asset.
	 * @param ParentClass Blueprintable Lunar control class.
	 * @param BaseAssetName Default editable asset name before uniqueness suffixing.
	 */
	static void AddBlueprintClassEntry(
		UToolMenu* Menu,
		FName SectionName,
		FName EntryName,
		const FText& Label,
		const FText& ToolTip,
		const FString& PackagePath,
		UClass* ParentClass,
		const FString& BaseAssetName);

	/**
	 * @brief Adds one concrete Data Asset creation command.
	 * @param Menu Menu receiving the entry.
	 * @param SectionName Section that groups the entry.
	 * @param EntryName Stable ToolMenus entry identifier.
	 * @param Label User-visible entry label.
	 * @param ToolTip User-visible entry description.
	 * @param PackagePath Internal package path that receives the asset.
	 * @param DataAssetClass Concrete UDataAsset subclass to instantiate.
	 * @param BaseAssetName Default editable asset name before uniqueness suffixing.
	 */
	static void AddDataAssetEntry(
		UToolMenu* Menu,
		FName SectionName,
		FName EntryName,
		const FText& Label,
		const FText& ToolTip,
		const FString& PackagePath,
		UClass* DataAssetClass,
		const FString& BaseAssetName);

	/**
	 * @brief Begins inline creation of a Widget Blueprint with a fixed Lunar parent.
	 * @param PackagePath Internal package path that receives the asset.
	 * @param ParentClass Weak Lunar UserWidget parent class.
	 * @param BaseAssetName Default editable asset name.
	 */
	static void CreateWidgetBlueprint(
		FString PackagePath,
		TWeakObjectPtr<UClass> ParentClass,
		FString BaseAssetName);

	/**
	 * @brief Begins inline creation of a reusable Blueprint subclass for a Lunar UWidget control.
	 * @param PackagePath Internal package path that receives the asset.
	 * @param ParentClass Weak Blueprintable Lunar control class.
	 * @param BaseAssetName Default editable asset name.
	 */
	static void CreateBlueprintClass(
		FString PackagePath,
		TWeakObjectPtr<UClass> ParentClass,
		FString BaseAssetName);

	/**
	 * @brief Begins inline creation of one concrete Lunar Data Asset.
	 * @param PackagePath Internal package path that receives the asset.
	 * @param DataAssetClass Weak concrete UDataAsset class.
	 * @param BaseAssetName Default editable asset name.
	 */
	static void CreateDataAsset(
		FString PackagePath,
		TWeakObjectPtr<UClass> DataAssetClass,
		FString BaseAssetName);

	/**
	 * @brief Sends a configured factory through the standard Content Browser inline-creation flow.
	 * @param PackagePath Internal package path that receives the asset.
	 * @param BaseAssetName Base name used to generate a unique editable asset name.
	 * @param AssetClass Asset class recorded by the Content Browser creation item.
	 * @param Factory Configured factory that creates the final asset after rename confirmation.
	 */
	static void BeginAssetCreation(
		const FString& PackagePath,
		const FString& BaseAssetName,
		UClass* AssetClass,
		UFactory* Factory);

	/**
	 * @brief Resolves one writable internal package path from the active Add New menu context.
	 * @param Menu Active Content Browser Add New menu.
	 * @param OutPackagePath Receives the writable internal package path.
	 * @return True when Lunar asset commands may be shown for the context.
	 */
	static bool TryResolvePackagePath(const UToolMenu* Menu, FString& OutPackagePath);

	/** @brief GC-safe brush retaining the configured Lunar texture while the editor module is active. */
	TSharedPtr<FDeferredCleanupSlateBrush> RootMenuIconBrush;
};
