// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Navigation/Data/LunarInputIconSet.h"
#include "UI/Navigation/Data/LunarUIActionRegistry.h"
#include "UI/Navigation/Styles/LunarButtonStyleAsset.h"
#include "UI/Navigation/Styles/LunarComboBoxStyleAsset.h"
#include "UI/Navigation/Styles/LunarContextMenuStyleAsset.h"
#include "UI/Navigation/Styles/LunarInputPromptStyleAsset.h"
#include "UI/Navigation/Styles/LunarListViewStyleAsset.h"
#include "UI/Navigation/Styles/LunarOptionSliderStyleAsset.h"
#include "UI/Navigation/Styles/LunarRadioStyleAsset.h"
#include "UI/Navigation/Styles/LunarScrollBoxStyleAsset.h"
#include "UI/Navigation/Styles/LunarSliderStyleAsset.h"
#include "UI/Navigation/Styles/LunarSwitchStyleAsset.h"
#include "UI/Navigation/Styles/LunarTabsStyleAsset.h"
#include "UI/Navigation/Types/LunarInputPromptTypes.h"
#include "UI/Navigation/Types/LunarNavigationTypes.h"
#include "UI/Navigation/Types/LunarUIFeedbackTypes.h"
#include "LunarNavigationSettings.generated.h"

/**
 * @file LunarNavigationSettings.h
 * @brief Project-wide settings structures for the Lunar UI navigation system
 * @ingroup LunarNavigationTypes
 */

/**
 * @brief Configurable semantic input and repeat defaults for Lunar navigation
 * @ingroup LunarNavigationTypes
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarNavigationInputSettings
{
	GENERATED_BODY()

	/** @brief Creates the built-in semantic action bindings. */
	FLunarNavigationInputSettings();

	/** Built-in and project-defined semantic UI actions. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Input")
	TArray<FLunarUIActionDefinition> ActionDefinitions;

	/** TODO(LunarUI): Assign the default action registry after its owner-created asset exists. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Input")
	TSoftObjectPtr<ULunarUIActionRegistry> DefaultActionRegistry;

	/** Enables keyboard bindings on the shared Lunar navigation channel. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Input")
	bool bEnableKeyboardNavigation = true;

	/** Global digital and analog navigation repeat timing. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Input")
	FLunarNavigationRepeatSettings RepeatSettings;

	/** Global analog-stick activation and hysteresis settings. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Input")
	FLunarAnalogNavigationSettings AnalogSettings;
};

/**
 * @brief Global navigation behavior and recovery defaults
 * @ingroup LunarNavigationTypes
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarNavigationBehaviorSettings
{
	GENERATED_BODY()

	/** @brief Creates behavior settings with the default pointer policy. */
	FLunarNavigationBehaviorSettings();

	/** Enables deterministic screen-space fallback when no explicit target resolves. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Behavior")
	bool bEnableGeometricFallback = true;

	/** Half-angle of the directional fallback cone in degrees. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Behavior", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float FallbackConeHalfAngleDegrees = 60.0f;

	/** Weight applied to lateral distance during fallback scoring. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Behavior", meta = (ClampMin = "0.0"))
	float FallbackLateralWeight = 2.0f;

	/** Allows automatic fallback to leave a navigation group unless the group overrides it. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Behavior")
	bool bAllowCrossGroupFallbackByDefault = true;

	/** Automatically selects the nearest eligible replacement when selection becomes invalid. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Behavior")
	bool bRecoverSelectionAutomatically = true;

	/** Default selection-driven scroll-into-view behavior. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Behavior")
	FLunarScrollIntoViewSettings DefaultScrollIntoViewSettings;

	/** Default top-level cursor visibility and gameplay pointer-capture policy. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Behavior")
	FLunarPointerPolicy DefaultPointerPolicy;

	/** Allows unused direct-scroll delta to pass from an inner ScrollBox to its parent. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Behavior")
	bool bAllowScrollChainingByDefault = true;
};

/**
 * @brief Global per-control style assets used when a widget has no explicit style
 * @ingroup LunarNavigationTypes
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarNavigationDefaultStyleSettings
{
	GENERATED_BODY()

	/** TODO(LunarUI): Assign the default Button style after its owner-created asset exists. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Default Styles")
	TSoftObjectPtr<ULunarButtonStyleAsset> DefaultButtonStyle;

	/** TODO(LunarUI): Assign the default Slider style after its owner-created asset exists. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Default Styles")
	TSoftObjectPtr<ULunarSliderStyleAsset> DefaultSliderStyle;

	/** TODO(LunarUI): Assign the default OptionSlider style after its owner-created asset exists. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Default Styles")
	TSoftObjectPtr<ULunarOptionSliderStyleAsset> DefaultOptionSliderStyle;

	/** TODO(LunarUI): Assign the default Switch style after its owner-created asset exists. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Default Styles")
	TSoftObjectPtr<ULunarSwitchStyleAsset> DefaultSwitchStyle;

	/** TODO(LunarUI): Assign the default Radio style after its owner-created asset exists. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Default Styles")
	TSoftObjectPtr<ULunarRadioStyleAsset> DefaultRadioStyle;

	/** TODO(LunarUI): Assign the default ScrollBox style after its owner-created asset exists. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Default Styles")
	TSoftObjectPtr<ULunarScrollBoxStyleAsset> DefaultScrollBoxStyle;

	/** TODO(LunarUI): Assign the default ListView style after its owner-created asset exists. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Default Styles")
	TSoftObjectPtr<ULunarListViewStyleAsset> DefaultListViewStyle;

	/** TODO(LunarUI): Assign the default ComboBox style after its owner-created asset exists. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Default Styles")
	TSoftObjectPtr<ULunarComboBoxStyleAsset> DefaultComboBoxStyle;

	/** TODO(LunarUI): Assign the default ContextMenu style after its owner-created asset exists. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Default Styles")
	TSoftObjectPtr<ULunarContextMenuStyleAsset> DefaultContextMenuStyle;

	/** TODO(LunarUI): Assign the default Tabs style after its owner-created asset exists. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Default Styles")
	TSoftObjectPtr<ULunarTabsStyleAsset> DefaultTabsStyle;

	/** TODO(LunarUI): Assign the default Input Prompt style after its owner-created asset exists. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Default Styles")
	TSoftObjectPtr<ULunarInputPromptStyleAsset> DefaultInputPromptStyle;
};

/**
 * @brief Global non-spatial UI sound defaults
 * @ingroup LunarNavigationTypes
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarUIAudioSettings
{
	GENERATED_BODY()

	/** TODO(LunarUI): Assign the default pointer-hover sound if Lunar ships one. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Audio")
	FLunarUISoundSpec DefaultPointerHoveredSound;

	/** TODO(LunarUI): Assign the default pointer-pressed sound if Lunar ships one. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Audio")
	FLunarUISoundSpec DefaultPointerPressedSound;

	/** TODO(LunarUI): Assign the default pointer-activated sound if Lunar ships one. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Audio")
	FLunarUISoundSpec DefaultPointerActivatedSound;

	/** TODO(LunarUI): Assign the default pointer-rejected sound if Lunar ships one. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Audio")
	FLunarUISoundSpec DefaultPointerRejectedSound;

	/** TODO(LunarUI): Assign the default navigation-selected sound if Lunar ships one. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Audio")
	FLunarUISoundSpec DefaultNavigationSelectedSound;

	/** TODO(LunarUI): Assign the default navigation-pressed sound if Lunar ships one. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Audio")
	FLunarUISoundSpec DefaultNavigationPressedSound;

	/** TODO(LunarUI): Assign the default navigation-activated sound if Lunar ships one. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Audio")
	FLunarUISoundSpec DefaultNavigationActivatedSound;

	/** TODO(LunarUI): Assign the default navigation-rejected sound if Lunar ships one. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Audio")
	FLunarUISoundSpec DefaultNavigationRejectedSound;
};

/**
 * @brief Global gamepad haptic defaults
 * @ingroup LunarNavigationTypes
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarUIHapticSettings
{
	GENERATED_BODY()

	/** TODO(LunarUI): Assign the default selected haptic if Lunar ships one. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Haptics")
	FLunarUIHapticSpec DefaultNavigationSelectedHaptic;

	/** TODO(LunarUI): Assign the default pressed haptic if Lunar ships one. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Haptics")
	FLunarUIHapticSpec DefaultNavigationPressedHaptic;

	/** TODO(LunarUI): Assign the default activated haptic if Lunar ships one. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Haptics")
	FLunarUIHapticSpec DefaultNavigationActivatedHaptic;

	/** TODO(LunarUI): Assign the default rejected haptic if Lunar ships one. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Haptics")
	FLunarUIHapticSpec DefaultNavigationRejectedHaptic;
};

/**
 * @brief Default prompt presentation and device icon-set references
 * @ingroup LunarNavigationTypes
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarInputPromptSettings
{
	GENERATED_BODY()

	/** TODO(LunarUI): Assign the default Prompt Widget after its owner-created class exists. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Input Prompts")
	TSoftClassPtr<UUserWidget> DefaultPromptWidgetClass;

	/** Default visibility policy for widget-owned prompts. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Input Prompts")
	ELunarPromptVisibilityPolicy DefaultPromptVisibilityPolicy = ELunarPromptVisibilityPolicy::WhenSelected;

	/** TODO(LunarUI): Assign the default Keyboard/Mouse Icon Set after its owner-created asset exists. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Input Prompts")
	TSoftObjectPtr<ULunarInputIconSet> DefaultKeyboardMouseIconSet;

	/** TODO(LunarUI): Assign the default Xbox Icon Set after its owner-created asset exists. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Input Prompts")
	TSoftObjectPtr<ULunarInputIconSet> DefaultXboxIconSet;

	/** TODO(LunarUI): Assign the default PlayStation 5 Icon Set after its owner-created asset exists. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Input Prompts")
	TSoftObjectPtr<ULunarInputIconSet> DefaultPlayStation5IconSet;
};

/**
 * @brief Accessibility defaults shared by all Lunar navigation controls
 * @ingroup LunarNavigationTypes
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarUIAccessibilitySettings
{
	GENERATED_BODY()

	/** Applies standard Lunar style transitions immediately. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Accessibility")
	bool bReduceMotion = false;

	/** Warns when an eligible control has no accessible name. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Accessibility")
	bool bValidateAccessibleNames = true;
};

/**
 * @brief Validation and graph-debugging defaults
 * @ingroup LunarNavigationTypes
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarNavigationDiagnosticsSettings
{
	GENERATED_BODY()

	/** Runtime validation verbosity. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Diagnostics")
	ELunarNavigationValidationLevel ValidationLevel = ELunarNavigationValidationLevel::WarningsAndErrors;

	/** Enables the navigation debug overlay when a local-player subsystem initializes. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Diagnostics")
	bool bEnableDebugOverlayByDefault = false;

	/** Includes calculated geometric-fallback links in graph dumps. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Diagnostics")
	bool bIncludeCalculatedLinksInGraphDump = true;
};

/**
 * @brief Complete Lunar UI navigation configuration exposed by ULunarSettings
 * @ingroup LunarNavigationTypes
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarNavigationSettings
{
	GENERATED_BODY()

	/** Semantic actions and repeat behavior. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation")
	FLunarNavigationInputSettings Input;

	/** Selection, fallback, pointer, and scrolling behavior. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation")
	FLunarNavigationBehaviorSettings Behavior;

	/** Default style assets for navigation controls and prompts. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation")
	FLunarNavigationDefaultStyleSettings DefaultStyles;

	/** Default non-spatial UI sounds. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation")
	FLunarUIAudioSettings Audio;

	/** Default gamepad haptic feedback. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation")
	FLunarUIHapticSettings Haptics;

	/** Input-prompt widget, visibility, and icon-set defaults. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation")
	FLunarInputPromptSettings Prompts;

	/** Accessibility defaults shared by navigation controls. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation")
	FLunarUIAccessibilitySettings Accessibility;

	/** Runtime validation and graph-debugging defaults. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation")
	FLunarNavigationDiagnosticsSettings Diagnostics;
};
