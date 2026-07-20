// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "LunarUIFeedbackTypes.generated.h"

class UForceFeedbackEffect;
class USoundBase;
class USoundConcurrency;

/**
 * @file LunarUIFeedbackTypes.h
 * @brief Sound and haptic feedback configuration for Lunar UI controls.
 * @ingroup LunarNavigationTypes
 */

/** @brief Defines how a widget resolves a sound or haptic feedback override. @ingroup LunarNavigationTypes */
UENUM(BlueprintType)
enum class ELunarFeedbackOverrideMode : uint8
{
	UseGlobal UMETA(DisplayName = "Use Global"), ///< Use the corresponding global Lunar setting.
	UseDataAsset UMETA(DisplayName = "Use Data Asset"), ///< Use the corresponding entry from the widget's assigned feedback Data Asset.
	Disabled UMETA(DisplayName = "Disabled"), ///< Suppress feedback for this event.
	Custom UMETA(DisplayName = "Custom") ///< Use the per-widget custom feedback specification.
};

/** @brief Describes one non-spatial Lunar UI sound request. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarUISoundSpec
{
	GENERATED_BODY()

	/** Sound played for the feedback event. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback")
	TObjectPtr<USoundBase> Sound = nullptr;

	/** Playback volume multiplier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback", meta = (ClampMin = "0.0"))
	float VolumeMultiplier = 1.0f;

	/** Playback pitch multiplier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback", meta = (ClampMin = "0.0"))
	float PitchMultiplier = 1.0f;

	/** Optional concurrency policy used for overlapping UI sounds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback")
	TObjectPtr<USoundConcurrency> Concurrency = nullptr;
};

/** @brief Describes one non-looping Lunar UI haptic request. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarUIHapticSpec
{
	GENERATED_BODY()

	/** Force-feedback effect played for the feedback event. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback")
	TObjectPtr<UForceFeedbackEffect> Effect = nullptr;
};

/** @brief Complete reusable sound-feedback set stored by a Lunar sound feedback Data Asset. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarUISoundFeedbackSet
{
	GENERATED_BODY()

	/** Sound played when pointer hover begins. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback") FLunarUISoundSpec PointerHovered;
	/** Sound played when a pointer press begins. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback") FLunarUISoundSpec PointerPressed;
	/** Sound played when pointer click succeeds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback") FLunarUISoundSpec PointerClicked;
	/** Sound played when pointer click is rejected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback") FLunarUISoundSpec PointerRejected;
	/** Sound played when Lunar Selection enters a control. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback") FLunarUISoundSpec NavigationSelected;
	/** Sound played when navigation press begins. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback") FLunarUISoundSpec NavigationPressed;
	/** Sound played when navigation click succeeds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback") FLunarUISoundSpec NavigationClicked;
	/** Sound played when navigation click is rejected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback") FLunarUISoundSpec NavigationRejected;
};

/** @brief Complete reusable haptic-feedback set stored by a Lunar haptic feedback Data Asset. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarUIHapticFeedbackSet
{
	GENERATED_BODY()

	/** Haptic effect played when Lunar Selection enters a control. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback") FLunarUIHapticSpec NavigationSelected;
	/** Haptic effect played when navigation press begins. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback") FLunarUIHapticSpec NavigationPressed;
	/** Haptic effect played when navigation click succeeds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback") FLunarUIHapticSpec NavigationClicked;
	/** Haptic effect played when navigation click is rejected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback") FLunarUIHapticSpec NavigationRejected;
};
/** @brief Selects the global, Data Asset, disabled, or custom sound for one feedback event. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarUISoundOverride
{
	GENERATED_BODY()

	/** Sound resolution mode. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback")
	ELunarFeedbackOverrideMode Mode = ELunarFeedbackOverrideMode::UseGlobal;

	/** Sound used when Mode is Custom. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback", meta = (EditCondition = "Mode == ELunarFeedbackOverrideMode::Custom", EditConditionHides))
	FLunarUISoundSpec CustomSound;
};

/** @brief Selects the global, Data Asset, disabled, or custom haptic effect for one feedback event. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarUIHapticOverride
{
	GENERATED_BODY()

	/** Haptic resolution mode. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback")
	ELunarFeedbackOverrideMode Mode = ELunarFeedbackOverrideMode::UseGlobal;

	/** Haptic effect used when Mode is Custom. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback", meta = (EditCondition = "Mode == ELunarFeedbackOverrideMode::Custom", EditConditionHides))
	FLunarUIHapticSpec CustomHaptic;
};

/** @brief Per-widget sound overrides for pointer and Lunar Navigation feedback events. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarUISoundOverrides
{
	GENERATED_BODY()

	/** @brief Sound played when pointer hover begins. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback")
	FLunarUISoundOverride PointerHovered;

	/** @brief Sound played when a pointer press begins. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback")
	FLunarUISoundOverride PointerPressed;

	/** @brief Sound played when a pointer click succeeds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback")
	FLunarUISoundOverride PointerClicked;

	/** @brief Sound played when a pointer interaction is rejected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback")
	FLunarUISoundOverride PointerRejected;

	/** @brief Sound played when Lunar Selection enters the control. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback")
	FLunarUISoundOverride NavigationSelected;

	/** @brief Sound played when the selected control enters Navigation Pressed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback")
	FLunarUISoundOverride NavigationPressed;

	/** @brief Sound played when navigation click succeeds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback")
	FLunarUISoundOverride NavigationClicked;

	/** @brief Sound played when navigation click is rejected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback")
	FLunarUISoundOverride NavigationRejected;
};

/** @brief Per-widget haptic overrides for Lunar Navigation feedback events. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarUIHapticOverrides
{
	GENERATED_BODY()

	/** @brief Haptic effect played when Lunar Selection enters the control. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback")
	FLunarUIHapticOverride NavigationSelected;

	/** @brief Haptic effect played when the control enters Navigation Pressed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback")
	FLunarUIHapticOverride NavigationPressed;

	/** @brief Haptic effect played when navigation click succeeds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback")
	FLunarUIHapticOverride NavigationClicked;

	/** @brief Haptic effect played when navigation click is rejected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Feedback")
	FLunarUIHapticOverride NavigationRejected;
};
