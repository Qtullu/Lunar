// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "GameplayTagContainer.h"
#include "InputCoreTypes.h"
#include "Subsystems/Console/LunarTypesConsole.h"
#include "Subsystems/RawInput/LunarRawInputTypes.h"
#include "Types/SlateEnums.h"
#include "LunarNavigationTypes.generated.h"

/**
 * @file LunarNavigationTypes.h
 * @brief Shared enums, data structures, and delegates for Lunar UI Navigation.
 * @ingroup LunarNavigationTypes
 */

class ULocalPlayer;
class ULunarNavigableWidget;
class ULunarNavigationScope;

/** @brief Cardinal direction used by Lunar navigation. @ingroup LunarNavigationTypes */
UENUM(BlueprintType)
enum class ELunarNavigationDirection : uint8
{
	Up,    ///< Move toward the top of the active scope.
	Down,  ///< Move toward the bottom of the active scope.
	Left,  ///< Move toward the left side of the active scope.
	Right  ///< Move toward the right side of the active scope.
};

/** @brief Resolution mode for one explicit navigation link. @ingroup LunarNavigationTypes */
UENUM(BlueprintType)
enum class ELunarNavigationLinkMode : uint8
{
	Automatic,    ///< Use geometric fallback resolution.
	Widget,       ///< Navigate directly to a widget reference.
	NavigationId, ///< Resolve a target by its stable navigation identifier.
	Block         ///< Consume movement in this direction without changing selection.
};

/** @brief Pointer and navigation interaction layer used for visual-state resolution. @ingroup LunarNavigationTypes */
UENUM(BlueprintType)
enum class ELunarUIInteractionState : uint8
{
	PointerNormal,       ///< Pointer mode with no hover or press interaction.
	PointerHovered,      ///< Pointer currently hovers the control.
	PointerPressed,      ///< Pointer currently presses the control.
	NavigationNormal,    ///< Navigation mode while the control is not selected.
	NavigationSelected,  ///< Navigation mode while the control owns selection.
	NavigationPressed    ///< Navigation mode while the selected control is pressed.
};

/** @brief Result returned by a Lunar control after semantic action dispatch. @ingroup LunarNavigationTypes */
UENUM(BlueprintType)
enum class ELunarUIActionResult : uint8
{
	Unhandled, ///< The control did not consume the action.
	Handled,   ///< The control consumed the action successfully.
	Rejected   ///< The control consumed the action but rejected the requested operation.
};

/** @brief Movement mode used when revealing a selected widget in a scroll container. @ingroup LunarNavigationTypes */
UENUM(BlueprintType)
enum class ELunarScrollIntoViewMode : uint8
{
	Immediate, ///< Reveal the target without interpolation.
	Smooth     ///< Interpolate the scroll position toward the target.
};

/** @brief Cursor-visibility policy contributed by a navigation scope. @ingroup LunarNavigationTypes */
UENUM(BlueprintType)
enum class ELunarCursorVisibilityPolicy : uint8
{
	Inherit,              ///< Keep the policy supplied by the previous scope or player controller.
	AlwaysVisible,        ///< Force the cursor to remain visible while the scope is active.
	AutoHideOnNavigation, ///< Hide after navigation input and restore after pointer input.
	AlwaysHidden          ///< Force the cursor to remain hidden while the scope is active.
};

/** @brief Gameplay mouse-capture and pointer-lock policy contributed by a scope. @ingroup LunarNavigationTypes */
UENUM(BlueprintType)
enum class ELunarPointerCapturePolicy : uint8
{
	Inherit, ///< Keep the current player-controller capture state.
	Release, ///< Release pointer capture while the scope is active.
	Preserve ///< Explicitly preserve gameplay pointer capture.
};

/** @brief How one Slider step is interpreted. @ingroup LunarNavigationTypes */
UENUM(BlueprintType)
enum class ELunarSliderStepMode : uint8
{
	Absolute,  ///< Treat the step as an absolute value delta.
	Percentage ///< Treat the step as a fraction of the slider range.
};

/** @brief When a Slider preview becomes its committed value. @ingroup LunarNavigationTypes */
UENUM(BlueprintType)
enum class ELunarSliderCommitMode : uint8
{
	Immediate, ///< Commit every accepted value change immediately.
	OnAccept   ///< Keep a preview value until the Accept action commits it.
};

/** @brief Directions claimed by a Switch for direct value changes. @ingroup LunarNavigationTypes */
UENUM(BlueprintType)
enum class ELunarSwitchDirectionMode : uint8
{
	Disabled,   ///< Do not consume directional actions for value changes.
	Horizontal, ///< Use Left and Right to change the switch value.
	Vertical    ///< Use Up and Down to change the switch value.
};

/** @brief Lifetime policy for pages owned by a Tabs control. @ingroup LunarNavigationTypes */
UENUM(BlueprintType)
enum class ELunarTabPageLifetime : uint8
{
	LazyCached,           ///< Create a page on first activation and retain it afterward.
	Eager,                ///< Create every page when the Tabs control initializes.
	RecreateOnActivation  ///< Recreate a page every time its tab becomes active.
};

/** @brief Amount of navigation validation performed by the runtime. @ingroup LunarNavigationTypes */
UENUM(BlueprintType)
enum class ELunarNavigationValidationLevel : uint8
{
	Disabled,          ///< Skip navigation validation.
	ErrorsOnly,        ///< Emit only validation errors.
	WarningsAndErrors, ///< Emit validation warnings and errors.
	All                ///< Emit all supported validation diagnostics.
};

/** @brief Explicit link configuration for one cardinal direction. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarNavigationLink
{
	GENERATED_BODY()

	/** Link-resolution behavior for this direction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	ELunarNavigationLinkMode Mode = ELunarNavigationLinkMode::Automatic;

	/** Direct destination used when Mode is Widget. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation", meta = (EditCondition = "Mode == ELunarNavigationLinkMode::Widget", EditConditionHides))
	TObjectPtr<ULunarNavigableWidget> Widget = nullptr;

	/** Stable destination identifier used when Mode is NavigationId. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation", meta = (EditCondition = "Mode == ELunarNavigationLinkMode::NavigationId", EditConditionHides))
	FName NavigationId = NAME_None;
};

/** @brief Automatic fallback policy for one optional navigation group. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarNavigationGroupSettings
{
	GENERATED_BODY()

	/** Stable group identifier assigned to participating widgets. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	FName GroupId = NAME_None;

	/** Allows geometric navigation to fall back to widgets outside this group. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	bool bAllowCrossGroupFallback = true;
};

/** @brief Timing shared by digital and analog navigation repeat. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarNavigationRepeatSettings
{
	GENERATED_BODY()

	/** Delay before a held direction begins repeating. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation", meta = (ClampMin = "0.0", UIMin = "0.0", Units = "s"))
	float InitialDelay = 0.35f;

	/** Interval between repeated digital navigation actions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation", meta = (ClampMin = "0.001", UIMin = "0.001", Units = "s"))
	float DigitalRepeatInterval = 0.10f;

	/** Analog repeat interval when stick magnitude first reaches the activation threshold. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation", meta = (ClampMin = "0.001", UIMin = "0.001", Units = "s"))
	float AnalogIntervalAtThreshold = 0.12f;

	/** Analog repeat interval at full stick magnitude. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation", meta = (ClampMin = "0.001", UIMin = "0.001", Units = "s"))
	float AnalogIntervalAtFullMagnitude = 0.06f;
};

/** @brief Thresholds and hysteresis used for left-stick navigation. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarAnalogNavigationSettings
{
	GENERATED_BODY()

	/** Radial dead zone removed before evaluating stick navigation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RadialDeadZone = 0.25f;

	/** Minimum processed magnitude that activates a direction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ActivationThreshold = 0.60f;

	/** Magnitude below which the active analog direction is released. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ReleaseThreshold = 0.40f;

	/** Required component advantage before a held diagonal changes cardinal direction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DirectionChangeDominanceMargin = 0.15f;
};

/** @brief Per-container behavior for revealing the current selection. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarScrollIntoViewSettings
{
	GENERATED_BODY()

	/** Whether the reveal occurs immediately or through smooth interpolation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	ELunarScrollIntoViewMode Mode = ELunarScrollIntoViewMode::Immediate;

	/** Safe padding retained between the selected widget and viewport edges. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	FMargin ViewportPadding = FMargin(0.0f);

	/** Duration of a smooth reveal transition in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation", meta = (ClampMin = "0.0", UIMin = "0.0", Units = "s"))
	float TransitionDuration = 0.20f;

	/** Maximum smooth scrolling speed in Slate units per second. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float ScrollSpeed = 1200.0f;
};

/** @brief Cursor and pointer-capture behavior contributed by a scope. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarPointerPolicy
{
	GENERATED_BODY()

	/** Cursor visibility behavior while the owning scope is active. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	ELunarCursorVisibilityPolicy CursorVisibilityPolicy = ELunarCursorVisibilityPolicy::Inherit;

	/** Pointer-capture behavior while the owning scope is active. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	ELunarPointerCapturePolicy PointerCapturePolicy = ELunarPointerCapturePolicy::Inherit;
};

/** @brief Serializable configuration used to initialize one navigation scope. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarNavigationScopeSettings
{
	GENERATED_BODY()

	/** Preferred initial selection supplied as a direct widget reference. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	TObjectPtr<ULunarNavigableWidget> InitialSelectionWidget = nullptr;

	/** Preferred initial selection resolved by stable navigation identifier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	FName InitialSelectionId = NAME_None;

	/** Restores the last valid selection when the scope becomes active again. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	bool bRestoreLastSelection = true;

	/** Enables automatic wrapping at horizontal scope boundaries. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	bool bWrapHorizontal = false;

	/** Enables automatic wrapping at vertical scope boundaries. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	bool bWrapVertical = false;

	/** Blocks all gameplay input instead of only UI actions handled by Lunar Navigation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	bool bBlockAllGameplayInput = false;

	/** Cursor and pointer-capture policy contributed by this scope. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	FLunarPointerPolicy PointerPolicy;

	/** Optional group-specific fallback policies used by automatic navigation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	TArray<FLunarNavigationGroupSettings> NavigationGroups;
};

/** @brief One configurable physical binding for a semantic UI action. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarUIActionBinding
{
	GENERATED_BODY()

	/** Physical key or gamepad button mapped to the semantic action. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	FKey Key = EKeys::Invalid;

	/** Allows this binding to participate without deleting its configuration. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	bool bEnabled = true;
};

/** @brief Registry entry for one semantic UI action. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarUIActionDefinition
{
	GENERATED_BODY()

	/** Semantic action tag dispatched to navigation controls. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation", meta = (Categories = "Lunar.UI.Action"))
	FGameplayTag ActionTag;

	/** Ordered physical bindings capable of producing this action. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	TArray<FLunarUIActionBinding> Bindings;

	/** Localized label displayed when no more specific prompt text is available. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	FText DisplayText;
};

/** @brief Fully resolved semantic action passed through the Lunar control dispatch chain. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarUIActionContext
{
	GENERATED_BODY()

	/** Semantic action being dispatched. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|Navigation")
	FGameplayTag ActionTag;

	/** Physical key that produced the action. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|Navigation")
	FKey Key = EKeys::Invalid;

	/** Input-device family active when the action was resolved. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|Navigation")
	ELunarInputDeviceType InputDevice = ELunarInputDeviceType::Unknown;

	/** Low-level input event represented by this action. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|Navigation")
	TEnumAsByte<EInputEvent> InputEvent = IE_Pressed;

	/** True when NavigationDirection contains a resolved cardinal direction. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|Navigation")
	bool bHasNavigationDirection = false;

	/** Cardinal direction carried by a directional semantic action. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|Navigation", meta = (EditCondition = "bHasNavigationDirection"))
	ELunarNavigationDirection NavigationDirection = ELunarNavigationDirection::Up;

	/** True when this action originates from held-input repeat. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|Navigation")
	bool bIsRepeat = false;

	/** Normalized analog magnitude used to derive repeat cadence. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|Navigation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AnalogMagnitude = 0.0f;

	/** Local player that owns the dispatch chain. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|Navigation")
	TObjectPtr<ULocalPlayer> LocalPlayer = nullptr;
};

/** @brief Resolved layered state used by control presentation. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarUIVisualState
{
	GENERATED_BODY()

	/** Semantic value-state tag resolved independently from interaction state. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation", meta = (Categories = "Lunar.UI.State.Value"))
	FGameplayTag ValueStateTag;

	/** Current pointer or navigation interaction layer. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	ELunarUIInteractionState InteractionState = ELunarUIInteractionState::PointerNormal;

	/** Active input-device family used for device-specific style resolution. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	ELunarInputDeviceType InputDevice = ELunarInputDeviceType::Unknown;

	/** True when standard Lunar transitions should resolve immediately. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Accessibility")
	bool bReduceMotion = false;
};

/** @brief One data-backed ComboBox option. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarComboBoxOption
{
	GENERATED_BODY()

	/** Stable identifier used for selection persistence and change events. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	FName OptionId = NAME_None;

	/** Localized label presented for this option. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	FText DisplayText;

	/** Allows the option to be accepted. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	bool bEnabled = true;

	/** Allows disabled options to receive navigation selection for explanation or inspection. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	bool bCanReceiveSelectionWhenDisabled = false;

	/** Localized explanation surfaced when the option cannot be accepted. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	FText DisabledReason;

	/** Optional owner-defined data associated with the option. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	TObjectPtr<UObject> Payload = nullptr;
};

/** @brief One stable tab header and page source. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarTabDescriptor
{
	GENERATED_BODY()

	/** Stable identifier used for tab activation and restoration. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	FName TabId = NAME_None;

	/** Runtime validates that the class implements the Lunar Tab Header contract. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	TSubclassOf<UUserWidget> HeaderWidgetClass;

	/** Widget class used when the control needs to construct this tab page. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	TSubclassOf<UUserWidget> PageWidgetClass;

	/** Optional pre-created page instance used instead of PageWidgetClass. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	TObjectPtr<UUserWidget> PageWidgetInstance = nullptr;

	/** Allows this tab to be activated. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	bool bEnabled = true;

	/** Localized explanation surfaced when the tab cannot be activated. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation")
	FText DisabledReason;
};

/** @brief One actionable validation result emitted by Lunar Navigation. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarNavigationValidationMessage
{
	GENERATED_BODY()

	/** Diagnostic severity used for filtering and console presentation. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|Navigation")
	ELunarConsoleMessageVerbosity Verbosity = ELunarConsoleMessageVerbosity::Warning;

	/** Stable machine-readable validation code. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|Navigation")
	FName Code = NAME_None;

	/** Localized human-readable diagnostic. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|Navigation")
	FText Message;

	/** Object or asset path associated with the diagnostic. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|Navigation")
	FString OwnerPath;
};

/** @brief Broadcast when the selected navigable widget changes. @ingroup LunarNavigationTypes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLunarSelectionChangedSignature, ULunarNavigableWidget*, PreviousSelection, ULunarNavigableWidget*, NewSelection);
/** @brief Broadcast when the active navigation scope changes. @ingroup LunarNavigationTypes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLunarActiveScopeChangedSignature, ULunarNavigationScope*, PreviousScope, ULunarNavigationScope*, NewScope);
/** @brief Broadcast when a navigable widget rejects a semantic action. @ingroup LunarNavigationTypes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLunarNavigationRejectedSignature, ULunarNavigableWidget*, Widget, const FLunarUIActionContext&, ActionContext);
/** @brief Parameterless event emitted by a navigable widget. @ingroup LunarNavigationTypes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLunarNavigableWidgetEventSignature);
/** @brief Broadcast when a control resolves a new visual state. @ingroup LunarNavigationTypes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLunarVisualStateChangedSignature, const FLunarUIVisualState&, VisualState);
/** @brief Broadcast after a Lunar Button completes a click. @ingroup LunarNavigationTypes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLunarButtonClickedSignature);
/** @brief Broadcast when a Slider preview value changes. @ingroup LunarNavigationTypes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLunarSliderValueChangedSignature, float, NewValue);
/** @brief Broadcast when a Slider value is committed. @ingroup LunarNavigationTypes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLunarSliderValueCommittedSignature, float, CommittedValue);
/** @brief Broadcast when an OptionSlider selects a new index. @ingroup LunarNavigationTypes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLunarOptionSliderIndexChangedSignature, int32, NewIndex);
/** @brief Broadcast when a Switch changes its Boolean value. @ingroup LunarNavigationTypes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLunarSwitchChangedSignature, bool, bIsOn);
/** @brief Broadcast when a Radio control changes checked state. @ingroup LunarNavigationTypes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLunarRadioCheckedChangedSignature, bool, bIsChecked);
/** @brief Broadcast when a RadioGroup selects a different radio identifier. @ingroup LunarNavigationTypes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLunarRadioGroupSelectionChangedSignature, FName, PreviousRadioId, FName, NewRadioId);
/** @brief Broadcast when the navigation-relevant ScrollBox state changes. @ingroup LunarNavigationTypes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLunarScrollStateChangedSignature);
/** @brief Broadcast when a ListView activates a different item identifier. @ingroup LunarNavigationTypes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLunarListViewActiveItemChangedSignature, FName, PreviousItemId, FName, NewItemId);
/** @brief Broadcast when a ComboBox selects a different option identifier. @ingroup LunarNavigationTypes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLunarComboBoxSelectionChangedSignature, FName, PreviousOptionId, FName, NewOptionId);
/** @brief Broadcast when a ContextMenu opens, closes, or changes submenu state. @ingroup LunarNavigationTypes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLunarContextMenuStateChangedSignature);
/** @brief Broadcast when a Tabs control activates a different tab identifier. @ingroup LunarNavigationTypes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLunarTabsActiveTabChangedSignature, FName, PreviousTabId, FName, NewTabId);
