// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarEditorModule.cpp
 * @brief Implements the Lunar editor module and its navigation Details registrations.
 * @ingroup LunarNavigationEditor
 */

#include "Components/AutoRotator/LunarAutoRotatorEditorBridge.h"
#include "Editor.h"
#include "LevelEditorViewport.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "UI/Navigation/Controls/LunarScrollBox.h"
#include "UI/Navigation/Core/LunarNavigableWidget.h"
#include "UI/Navigation/Core/LunarScreenWidget.h"
#include "UI/Navigation/Customizations/LunarNavigableWidgetDetails.h"

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
};

IMPLEMENT_MODULE(FLunarEditorModule, LunarEditor)
