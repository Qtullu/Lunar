// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "LunarTypesAudio.generated.h"

class UAudioComponent;
class USoundBase;
class USoundClass;
class USoundConcurrency;
class USoundMix;

/**
 * @file LunarTypesAudio.h
 * @brief Audio shared types
 */

 /**
  * @brief Audio frequency range used for reactive audio values
  * @ingroup LunarTypesAudio
  */
UENUM(BlueprintType)
enum class ELunarAudioFrequencyRange : uint8
{
	SubBass UMETA(DisplayName = "Sub Bass"),
	Bass UMETA(DisplayName = "Bass"),
	LowMid UMETA(DisplayName = "Low Mid"),
	Mid UMETA(DisplayName = "Mid"),
	HighMid UMETA(DisplayName = "High Mid"),
	High UMETA(DisplayName = "High"),
	Full UMETA(DisplayName = "Full")
};

/**
 * @brief Sound class volume setting used by audio settings
 * @ingroup LunarTypesAudio
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarSoundClassVolumeSetting
{
	GENERATED_BODY()

	/**
	 * @brief Sound class that receives volume override
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Audio")
	TObjectPtr<USoundClass> SoundClass = nullptr;

	/**
	 * @brief Normalized volume from zero to one
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Audio", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float Volume = 1.0f;

	/**
	 * @brief Pitch multiplier used by this sound class
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Audio", meta = (ClampMin = "0.01", UIMin = "0.01"))
	float Pitch = 1.0f;

	/**
	 * @brief Fade time used when applying volume
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Audio", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float FadeTime = 0.0f;

	/**
	 * @brief Whether child sound classes receive this override
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Audio")
	bool bApplyToChildren = true;
};

/**
 * @brief Music setting used by runtime audio options
 * @ingroup LunarTypesAudio
 */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarMusicSetting
{
	GENERATED_BODY()

	/**
	 * @brief Sound mix that receives the music sound class override
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Audio")
	TObjectPtr<USoundMix> SoundMix = nullptr;

	/**
	 * @brief Music sound class that receives volume override
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Audio")
	TObjectPtr<USoundClass> SoundClass = nullptr;

	/**
	 * @brief Normalized music volume from zero to one
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Audio", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float Volume = 1.0f;

	/**
	 * @brief Music sound class pitch multiplier
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Audio", meta = (ClampMin = "0.01", UIMin = "0.01"))
	float Pitch = 1.0f;

	/**
	 * @brief Fade time for sound class volume
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Audio", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float VolumeFadeTime = 0.0f;

	/**
	 * @brief Whether child sound classes receive this override
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Audio")
	bool bApplyToChildren = true;

	/**
	 * @brief Whether sound mix should be pushed before applying volume
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Audio")
	bool bPushSoundMix = true;

	/**
	 * @brief Whether a new music track should be started
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Audio")
	bool bTransitionToNewMusic = false;

	/**
	 * @brief New music sound used when transition is enabled
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Audio")
	TObjectPtr<USoundBase> NewMusic = nullptr;

	/**
	 * @brief Fade time for current and new music components
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Audio", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float MusicFadeTime = 1.0f;

	/**
	 * @brief Start time for new music
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Audio", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float NewMusicStartTime = 0.0f;

	/**
	 * @brief New music pitch multiplier
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Audio", meta = (ClampMin = "0.01", UIMin = "0.01"))
	float NewMusicPitch = 1.0f;

	/**
	 * @brief Optional concurrency settings for new music
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Audio")
	TObjectPtr<USoundConcurrency> NewMusicConcurrencySettings = nullptr;

	/**
	 * @brief Whether new music persists across level transition
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Audio")
	bool bPersistAcrossLevelTransition = false;

	/**
	 * @brief Whether new music component should destroy itself after playback
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Audio")
	bool bAutoDestroyNewMusic = false;

	/**
	 * @brief Whether current music component should fade out during transition
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Audio")
	bool bFadeOutCurrentMusic = true;
};