// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputCoreTypes.h"
#include "Styling/SlateBrush.h"
#include "Subsystems/RawInput/LunarRawInputTypes.h"
#include "LunarInputPromptTypes.generated.h"

class ULunarInputIconSet;
class ULunarNavigableWidget;

/**
 * @file LunarInputPromptTypes.h
 * @brief Shared input-prompt enums and resolved presentation structures.
 * @ingroup LunarNavigationTypes
 */

/** @brief Defines when a Lunar widget's input prompt is presented. @ingroup LunarNavigationTypes */
UENUM(BlueprintType)
enum class ELunarPromptVisibilityPolicy : uint8
{
	WhenSelected UMETA(DisplayName = "When Selected"), ///< Follow Lunar Selection during navigation presentation and pointer hover during pointer presentation.
	Always UMETA(DisplayName = "Always"), ///< Show whenever the prompt owner is constructed and enabled.
	Manual UMETA(DisplayName = "Manual") ///< Leave visibility under owner-authored presentation control.
};

/** @brief Requests presentation data for one semantic Lunar UI action. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarPromptActionRequest
{
	GENERATED_BODY()

	/** Semantic action requested by the prompt owner. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Prompts")
	FGameplayTag ActionTag;

	/** Optional key preferred over the action's normal device binding. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Prompts")
	FKey PreferredKey = EKeys::Invalid;

	/** Optional display text supplied by this request. Defaults non-localizable; authors may enable localization explicitly. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Prompts")
	FText DisplayTextOverride = FText::AsCultureInvariant(TEXT(""));

	/** Whether a missing resolved icon is an authoring error; disable when the custom presentation does not require an icon. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Prompts")
	bool bRequireIcon = true;

	/** Enables IconOverride as the final prompt icon instead of resolving an Icon Set entry. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Prompts")
	bool bOverrideIcon = false;

	/** Optional resource-backed icon supplied by this request when bOverrideIcon is enabled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Prompts", meta = (EditCondition = "bOverrideIcon", EditConditionHides))
	FSlateBrush IconOverride;

	/** Enables this prompt action request. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Prompts")
	bool bEnabled = true;
};

/** @brief Complete resolved presentation context for one semantic Lunar UI action. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarResolvedPromptAction
{
	GENERATED_BODY()

	/** Resolved semantic action. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|Navigation|Prompts")
	FGameplayTag ActionTag;

	/** Active input device used for this resolution. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|Navigation|Prompts")
	ELunarInputDeviceType InputDevice = ELunarInputDeviceType::Unknown;

	/** Resolved key or control. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|Navigation|Prompts")
	FKey ResolvedKey = EKeys::Invalid;

	/** Device-specific icon set used for this resolution. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|Navigation|Prompts")
	TObjectPtr<ULunarInputIconSet> IconSet = nullptr;

	/** Resolved input icon or the standard missing-icon placeholder. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|Navigation|Prompts")
	FSlateBrush Icon;

	/** Resolved localized display text. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|Navigation|Prompts")
	FText DisplayText;

	/** Widget that owns the prompt request. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|Navigation|Prompts")
	TObjectPtr<ULunarNavigableWidget> OwnerWidget = nullptr;

	/** Whether the owning Lunar widget is currently selected. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|Navigation|Prompts")
	bool bSelected = false;

	/** Whether the owning Lunar widget and action are enabled. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|Navigation|Prompts")
	bool bEnabled = true;
};

/** @brief Maps one Unreal input key to its device-specific prompt icon. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarInputIconEntry
{
	GENERATED_BODY()

	/** Input key represented by the icon. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Prompts")
	FKey InputKey = EKeys::Invalid;

	/** Icon rendered for the input key. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Prompts")
	FSlateBrush Icon;
};
