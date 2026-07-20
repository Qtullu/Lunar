// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarEditorModule.cpp
 * @brief Implements the Lunar editor module and its navigation Details registrations.
 * @ingroup LunarNavigationEditor
 */

#include "Blueprint/UserWidget.h"
#include "Components/AutoRotator/LunarAutoRotatorEditorBridge.h"
#include "Editor.h"
#include "LevelEditorViewport.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "UI/Navigation/Controls/LunarScrollBox.h"
#include "UI/Navigation/Core/LunarNavigableWidget.h"
#include "UI/Navigation/Core/LunarScreenWidget.h"
#include "UI/Navigation/Creation/LunarAssetCreationMenu.h"
#include "UI/Navigation/Customizations/LunarNavigableWidgetDetails.h"
#include "UMGEditorProjectSettings.h"

namespace
{
	/**
	 * @brief Returns classes that share the Lunar navigation Details customization.
	 * @return Stable list of customized runtime class names.
	 */
	const TArray<FName>& GetCustomizedNavigationClassNames()
	{
		static const TArray<FName> ClassNames = {
			ULunarNavigableWidget::StaticClass()->GetFName(),
			ULunarScreenWidget::StaticClass()->GetFName(),
			ULunarScrollBox::StaticClass()->GetFName()
		};
		return ClassNames;
	}

	/**
	 * @brief Reads the active level-editor viewport camera location for the runtime editor bridge.
	 * @param OutCameraLocation Receives the active viewport camera location.
	 * @return True when an active level-editor viewport client is available.
	 */
	bool ResolveEditorViewportCameraLocation(FVector& OutCameraLocation)
	{
		if (!GCurrentLevelEditingViewportClient)
		{
			return false;
		}

		OutCameraLocation = GCurrentLevelEditingViewportClient->GetViewLocation();
		return true;
	}
}

/**
 * @brief Registers Lunar editor integrations while the LunarEditor module is loaded.
 * @ingroup LunarNavigationEditor
 */
class FLunarEditorModule final : public IModuleInterface
{
public:
	/** @brief Binds editor bridges and registers Lunar navigation Details customizations. */
	virtual void StartupModule() override
	{
		AssetCreationMenu = MakeUnique<FLunarAssetCreationMenu>();
		AssetCreationMenu->Register();
		RegisterFavoriteWidgetParentClasses();
		GetLunarResolveEditorViewportCameraLocationDelegate().BindStatic(&ResolveEditorViewportCameraLocation);

		FPropertyEditorModule& PropertyEditorModule =
			FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
		for (const FName ClassName : GetCustomizedNavigationClassNames())
		{
			PropertyEditorModule.RegisterCustomClassLayout(
				ClassName,
				FOnGetDetailCustomizationInstance::CreateStatic(&FLunarNavigableWidgetDetails::MakeInstance));
		}
		PropertyEditorModule.NotifyCustomizationModuleChanged();
	}

	/** @brief Unbinds editor bridges and unregisters Lunar navigation Details customizations. */
	virtual void ShutdownModule() override
	{
		if (AssetCreationMenu)
		{
			AssetCreationMenu->Unregister();
			AssetCreationMenu.Reset();
		}
		GetLunarResolveEditorViewportCameraLocationDelegate().Unbind();

		if (FPropertyEditorModule* PropertyEditorModule =
			FModuleManager::GetModulePtr<FPropertyEditorModule>(TEXT("PropertyEditor")))
		{
			for (const FName ClassName : GetCustomizedNavigationClassNames())
			{
				PropertyEditorModule->UnregisterCustomClassLayout(ClassName);
			}
			PropertyEditorModule->NotifyCustomizationModuleChanged();
		}
	}

private:
	/** @brief Adds only the two general-purpose Lunar bases to the Widget Blueprint COMMON picker. */
	void RegisterFavoriteWidgetParentClasses()
	{
		UUMGEditorProjectSettings* Settings = GetMutableDefault<UUMGEditorProjectSettings>();
		if (!Settings)
		{
			return;
		}

		Settings->FavoriteWidgetParentClasses.AddUnique(
			TSoftClassPtr<UUserWidget>(ULunarScreenWidget::StaticClass()));
		Settings->FavoriteWidgetParentClasses.AddUnique(
			TSoftClassPtr<UUserWidget>(ULunarNavigableWidget::StaticClass()));
	}

	/** @brief Owns the Lunar Content Browser asset-creation menu registration. */
	TUniquePtr<FLunarAssetCreationMenu> AssetCreationMenu;
};

IMPLEMENT_MODULE(FLunarEditorModule, LunarEditor)
