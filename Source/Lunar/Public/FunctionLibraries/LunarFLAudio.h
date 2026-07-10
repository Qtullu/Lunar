// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/LunarTypesAudio.h"
#include "LunarFLAudio.generated.h"

class UAudioComponent;
class UConstantQNRT;
class ULoudnessNRT;
class USoundClass;
class USoundMix;

/**
 * @file LunarFLAudio.h
 * @brief Audio helper function library
 */

 /**
  * @brief Blueprint utility functions for audio
  * @ingroup LunarFLAudio
  */
UCLASS()
class LUNAR_API ULunarFLAudio : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * @brief Sets normalized sound class volume from zero to one
	 * @ingroup LunarFLAudio
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Audio|Sound Class", meta = (WorldContext = "WorldContextObject", DisplayName = "Set Sound Class Volume", AdvancedDisplay = "5"))
	static bool SetSoundClassVolume(const UObject* WorldContextObject, USoundMix* SoundMix, USoundClass* SoundClass, UPARAM(meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0")) float Volume, float FadeTime = 0.0f, bool bApplyToChildren = true, float Pitch = 1.0f, bool bPushSoundMix = true);

	/**
	 * @brief Sets sound class muted state
	 * @ingroup LunarFLAudio
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Audio|Sound Class", meta = (WorldContext = "WorldContextObject", DisplayName = "Set Sound Class Muted", AdvancedDisplay = "4"))
	static bool SetSoundClassMuted(const UObject* WorldContextObject, USoundMix* SoundMix, USoundClass* SoundClass, bool bMuted, float FadeTime = 0.0f, bool bApplyToChildren = true, bool bPushSoundMix = true);

	/**
	 * @brief Applies one sound class volume setting
	 * @ingroup LunarFLAudio
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Audio|Sound Class", meta = (WorldContext = "WorldContextObject", DisplayName = "Apply Sound Class Volume Setting"))
	static bool ApplySoundClassVolumeSetting(const UObject* WorldContextObject, USoundMix* SoundMix, const FLunarSoundClassVolumeSetting& Setting, bool bPushSoundMix = true);

	/**
	 * @brief Applies many sound class volume settings
	 * @ingroup LunarFLAudio
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Audio|Sound Class", meta = (WorldContext = "WorldContextObject", DisplayName = "Apply Sound Class Volume Settings"))
	static int32 ApplySoundClassVolumeSettings(const UObject* WorldContextObject, USoundMix* SoundMix, const TArray<FLunarSoundClassVolumeSetting>& Settings, bool bPushSoundMix = true);

	/**
	 * @brief Applies music volume and optional music transition
	 * @ingroup LunarFLAudio
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Audio|Music", meta = (WorldContext = "WorldContextObject", DisplayName = "Apply Music Setting"))
	static UAudioComponent* ApplyMusicSetting(const UObject* WorldContextObject, UAudioComponent* CurrentMusicComponent, const FLunarMusicSetting& Setting);

	/**
	 * @brief Gets normalized loudness at time
	 * @details Use this function when you need the general loudness of a prepared sound at an exact time in seconds
	 * @details The function reads a Loudness NRT asset that was created for the same SoundWave that you want to analyze
	 * @details The returned value is normalized from zero to one
	 * @details Zero means silence or almost no loudness
	 * @details One means very high loudness for this analyzer
	 * @details This is useful for global light brightness music driven ambience camera shake intensity or material emissive strength
	 * @details Basic workflow
	 * @details Step 1 Create a Loudness NRT asset for your music SoundWave
	 * @details Step 2 Play the same SoundWave with an Audio Component or another music system
	 * @details Step 3 Get current playback time in seconds from playback percent multiplied by SoundWave duration
	 * @details Step 4 Call this function with that time
	 * @details Step 5 Use the returned value to drive light intensity material emissive or Niagara parameters
	 * @details Example use case
	 * @details A club level has a background song and ceiling lights should glow stronger when the whole track becomes louder
	 * @details Use LoudnessAnalyzer with TimeSeconds and multiply the result by the maximum light intensity
	 * @param LoudnessAnalyzer Loudness NRT asset that contains precomputed loudness data for the analyzed sound
	 * @param TimeSeconds Playback time in seconds inside the analyzed sound
	 * @return Normalized loudness value from zero to one
	 * @ingroup LunarFLAudio
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Audio|Reactive", meta = (DisplayName = "Get Normalized Loudness At Time"))
	static float GetNormalizedLoudnessAtTime(ULoudnessNRT* LoudnessAnalyzer, float TimeSeconds);

	/**
	 * @brief Gets normalized loudness by playback percent
	 * @details Use this function when you have current playback percent and need the general loudness of the analyzed sound
	 * @details This is the most convenient version when you use Audio Component event On Audio Playback Percent
	 * @details PlaybackPercent must be from zero to one
	 * @details Zero means the beginning of the sound
	 * @details One means the end of the sound
	 * @details The function converts playback percent to time internally using analyzer duration
	 * @details The returned value is normalized from zero to one
	 * @details This value represents total perceived loudness and not a specific frequency range
	 * @details Basic workflow
	 * @details Step 1 Create a Loudness NRT asset for the same SoundWave that will play
	 * @details Step 2 Bind On Audio Playback Percent on your music Audio Component
	 * @details Step 3 Store Playback Percent in a variable
	 * @details Step 4 Call this function every Tick or Timer with the stored Playback Percent
	 * @details Step 5 Use the result for light brightness material emissive post process weight or other visual reaction
	 * @details Example use case
	 * @details A neon sign should breathe with the whole song volume
	 * @details Use this function with Playback Percent and map the result to emissive intensity
	 * @param LoudnessAnalyzer Loudness NRT asset that contains precomputed loudness data for the analyzed sound
	 * @param PlaybackPercent Current playback position from zero to one
	 * @return Normalized loudness value from zero to one
	 * @ingroup LunarFLAudio
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Audio|Reactive", meta = (DisplayName = "Get Normalized Loudness At Playback Percent"))
	static float GetNormalizedLoudnessAtPlaybackPercent(ULoudnessNRT* LoudnessAnalyzer, UPARAM(meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0")) float PlaybackPercent);

	/**
	 * @brief Gets normalized frequency value at time
	 * @details Use this function when you need bass mids highs or full spectrum value at an exact time in seconds
	 * @details The function reads a Constant Q NRT asset that was created for the same SoundWave that you want to analyze
	 * @details This function is for frequency based reactions
	 * @details Use Bass for kick drums bass lines and strong club beat reactions
	 * @details Use LowMid or Mid for body movement voice driven motion and general music energy
	 * @details Use HighMid or High for hats claps sparks strobes and fast small visual details
	 * @details Use Full when you want one average value for the whole analyzed spectrum
	 * @details The returned value is normalized from zero to one
	 * @details Basic workflow
	 * @details Step 1 Create a Synesthesia NRT -> Constant Q NRT asset for your music SoundWave
	 * @details Step 2 Play the same SoundWave with an Audio Component or another music system
	 * @details Step 3 Get current playback time in seconds
	 * @details Step 4 Choose FrequencyRange such as Bass or High
	 * @details Step 5 Call this function with TimeSeconds
	 * @details Step 6 Use the returned value to drive lights materials Niagara camera shake or animation parameters
	 * @details Example use case
	 * @details A club spotlight should flash on bass hits
	 * @details Use FrequencyRange Bass and pass the output into Get Audio Beat Pulse or directly into light intensity
	 * @details Another example use case
	 * @details A particle system should sparkle on high frequencies
	 * @details Use FrequencyRange High and send the result into a Niagara float parameter
	 * @param ConstantQAnalyzer Constant Q NRT asset that contains precomputed frequency data for the analyzed sound
	 * @param TimeSeconds Playback time in seconds inside the analyzed sound
	 * @param FrequencyRange Frequency range that should be sampled
	 * @param Channel Audio channel index where zero is the first channel
	 * @return Normalized frequency value from zero to one
	 * @ingroup LunarFLAudio
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Audio|Reactive", meta = (DisplayName = "Get Audio Frequency Value At Time"))
	static float GetAudioFrequencyValueAtTime(UConstantQNRT* ConstantQAnalyzer, float TimeSeconds, ELunarAudioFrequencyRange FrequencyRange = ELunarAudioFrequencyRange::Bass, int32 Channel = 0);

	/**
	 * @brief Gets normalized frequency value by playback percent
	 * @details Use this function when you have current playback percent and need bass mids highs or full spectrum value
	 * @details This is the most convenient version for Blueprint music playback because Audio Component can provide Playback Percent
	 * @details PlaybackPercent must be from zero to one
	 * @details Zero means the beginning of the sound
	 * @details One means the end of the sound
	 * @details The function converts playback percent to time internally using analyzer duration
	 * @details The function reads a Constant Q NRT asset and returns a normalized frequency value from zero to one
	 * @details Basic workflow
	 * @details Step 1 Create a Synesthesia NRT -> Constant Q NRT asset for the same SoundWave that will play
	 * @details Step 2 Bind On Audio Playback Percent on the Audio Component that plays the music
	 * @details Step 3 Store Playback Percent in a variable
	 * @details Step 4 Call this function every Tick or Timer with that Playback Percent
	 * @details Step 5 Select FrequencyRange Bass for beat driven light or High for spark effects
	 * @details Step 6 Optionally pass the result through Smooth Audio Reactive Value
	 * @details Step 7 Optionally pass the current and previous value into Get Audio Beat Pulse
	 * @details Step 8 Apply the final value to Point Light intensity Rect Light intensity material emissive Niagara parameters or camera effects
	 * @details Example use case
	 * @details A club dance floor should pulse with the kick drum
	 * @details Use FrequencyRange Bass with Playback Percent from the music Audio Component
	 * @details Store previous bass value and call Get Audio Beat Pulse for sharper flashes
	 * @details Example Blueprint logic
	 * @details On Audio Playback Percent stores CurrentPlaybackPercent
	 * @details Tick calls this function with CurrentPlaybackPercent
	 * @details Tick stores current value as PreviousValue after visual update
	 * @param ConstantQAnalyzer Constant Q NRT asset that contains precomputed frequency data for the analyzed sound
	 * @param PlaybackPercent Current playback position from zero to one
	 * @param FrequencyRange Frequency range that should be sampled
	 * @param Channel Audio channel index where zero is the first channel
	 * @return Normalized frequency value from zero to one
	 * @ingroup LunarFLAudio
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Audio|Reactive", meta = (DisplayName = "Get Audio Frequency Value At Playback Percent"))
	static float GetAudioFrequencyValueAtPlaybackPercent(UConstantQNRT* ConstantQAnalyzer, UPARAM(meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0")) float PlaybackPercent, ELunarAudioFrequencyRange FrequencyRange = ELunarAudioFrequencyRange::Bass, int32 Channel = 0);

	/**
	 * @brief Gets pulse value from reactive audio difference
	 * @ingroup LunarFLAudio
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Audio|Reactive", meta = (DisplayName = "Get Audio Beat Pulse"))
	static float GetAudioBeatPulse(float CurrentValue, float PreviousValue, float Threshold = 0.25f, float Sensitivity = 1.0f);

	/**
	 * @brief Smooths reactive audio value
	 * @ingroup LunarFLAudio
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Audio|Reactive", meta = (DisplayName = "Smooth Audio Reactive Value"))
	static float SmoothAudioReactiveValue(float CurrentValue, float TargetValue, float DeltaTime, float InterpSpeed = 8.0f);

	/**
	 * @brief Converts linear volume to decibels
	 * @ingroup LunarFLAudio
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Audio|Math", meta = (DisplayName = "Linear Volume To Decibels"))
	static float LinearVolumeToDecibels(float LinearVolume);

	/**
	 * @brief Converts decibels to linear volume
	 * @ingroup LunarFLAudio
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Audio|Math", meta = (DisplayName = "Decibels To Linear Volume"))
	static float DecibelsToLinearVolume(float Decibels);
};