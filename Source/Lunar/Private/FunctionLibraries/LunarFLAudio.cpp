// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "LunarFLAudio.h"

#include "Components/AudioComponent.h"
#include "ConstantQNRT.h"
#include "Kismet/GameplayStatics.h"
#include "LoudnessNRT.h"
#include "NativeGameplayTags.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundConcurrency.h"
#include "Sound/SoundMix.h"
#include "Subsystems/Console/LunarConsoleSubsystem.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Lunar_Audio, "Lunar.Audio");

bool ULunarFLAudio::SetSoundClassVolume(const UObject* WorldContextObject, USoundMix* SoundMix, USoundClass* SoundClass, float Volume, float FadeTime, bool bApplyToChildren, float Pitch, bool bPushSoundMix)
{
	if (!WorldContextObject)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("SetSoundClassVolume failed: WorldContextObject is invalid"));
		return false;
	}

	if (!SoundMix)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("SetSoundClassVolume failed: SoundMix is invalid"));
		return false;
	}

	if (!SoundClass)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("SetSoundClassVolume failed: SoundClass is invalid"));
		return false;
	}

	if (Volume < 0.0f || Volume > 1.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("SetSoundClassVolume failed: Volume must be between zero and one"));
		return false;
	}

	if (FadeTime < 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("SetSoundClassVolume failed: FadeTime is invalid"));
		return false;
	}

	if (Pitch <= 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("SetSoundClassVolume failed: Pitch is invalid"));
		return false;
	}

	if (bPushSoundMix)
	{
		UGameplayStatics::PushSoundMixModifier(WorldContextObject, SoundMix);
	}

	UGameplayStatics::SetSoundMixClassOverride(WorldContextObject, SoundMix, SoundClass, Volume, Pitch, FadeTime, bApplyToChildren);
	return true;
}

bool ULunarFLAudio::SetSoundClassMuted(const UObject* WorldContextObject, USoundMix* SoundMix, USoundClass* SoundClass, bool bMuted, float FadeTime, bool bApplyToChildren, bool bPushSoundMix)
{
	const float Volume = bMuted ? 0.0f : 1.0f;
	return SetSoundClassVolume(WorldContextObject, SoundMix, SoundClass, Volume, FadeTime, bApplyToChildren, 1.0f, bPushSoundMix);
}

bool ULunarFLAudio::ApplySoundClassVolumeSetting(const UObject* WorldContextObject, USoundMix* SoundMix, const FLunarSoundClassVolumeSetting& Setting, bool bPushSoundMix)
{
	if (!WorldContextObject)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("ApplySoundClassVolumeSetting failed: WorldContextObject is invalid"));
		return false;
	}

	if (!SoundMix)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("ApplySoundClassVolumeSetting failed: SoundMix is invalid"));
		return false;
	}

	if (!Setting.SoundClass)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("ApplySoundClassVolumeSetting failed: SoundClass is invalid"));
		return false;
	}

	return SetSoundClassVolume(WorldContextObject, SoundMix, Setting.SoundClass, Setting.Volume, Setting.FadeTime, Setting.bApplyToChildren, Setting.Pitch, bPushSoundMix);
}

int32 ULunarFLAudio::ApplySoundClassVolumeSettings(const UObject* WorldContextObject, USoundMix* SoundMix, const TArray<FLunarSoundClassVolumeSetting>& Settings, bool bPushSoundMix)
{
	if (!WorldContextObject)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("ApplySoundClassVolumeSettings failed: WorldContextObject is invalid"));
		return 0;
	}

	if (!SoundMix)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("ApplySoundClassVolumeSettings failed: SoundMix is invalid"));
		return 0;
	}

	if (Settings.IsEmpty())
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Warning, TEXT("ApplySoundClassVolumeSettings failed: Settings array is empty"));
		return 0;
	}

	if (bPushSoundMix)
	{
		UGameplayStatics::PushSoundMixModifier(WorldContextObject, SoundMix);
	}

	int32 AppliedCount = 0;

	for (int32 Index = 0; Index < Settings.Num(); ++Index)
	{
		const FLunarSoundClassVolumeSetting& Setting = Settings[Index];

		if (!Setting.SoundClass)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, FString::Printf(TEXT("ApplySoundClassVolumeSettings skipped: SoundClass is invalid at index %d"), Index));
			continue;
		}

		if (Setting.Volume < 0.0f || Setting.Volume > 1.0f)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, FString::Printf(TEXT("ApplySoundClassVolumeSettings skipped: Volume is invalid at index %d"), Index));
			continue;
		}

		if (Setting.FadeTime < 0.0f)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, FString::Printf(TEXT("ApplySoundClassVolumeSettings skipped: FadeTime is invalid at index %d"), Index));
			continue;
		}

		if (Setting.Pitch <= 0.0f)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, FString::Printf(TEXT("ApplySoundClassVolumeSettings skipped: Pitch is invalid at index %d"), Index));
			continue;
		}

		UGameplayStatics::SetSoundMixClassOverride(WorldContextObject, SoundMix, Setting.SoundClass, Setting.Volume, Setting.Pitch, Setting.FadeTime, Setting.bApplyToChildren);
		++AppliedCount;
	}

	return AppliedCount;
}

UAudioComponent* ULunarFLAudio::ApplyMusicSetting(const UObject* WorldContextObject, UAudioComponent* CurrentMusicComponent, const FLunarMusicSetting& Setting)
{
	if (!WorldContextObject)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("ApplyMusicSetting failed: WorldContextObject is invalid"));
		return nullptr;
	}

	if (!Setting.SoundMix)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("ApplyMusicSetting failed: SoundMix is invalid"));
		return nullptr;
	}

	if (!Setting.SoundClass)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("ApplyMusicSetting failed: SoundClass is invalid"));
		return nullptr;
	}

	if (Setting.Volume < 0.0f || Setting.Volume > 1.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("ApplyMusicSetting failed: Volume must be between zero and one"));
		return nullptr;
	}

	if (Setting.Pitch <= 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("ApplyMusicSetting failed: Pitch is invalid"));
		return nullptr;
	}

	if (Setting.VolumeFadeTime < 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("ApplyMusicSetting failed: VolumeFadeTime is invalid"));
		return nullptr;
	}

	if (Setting.MusicFadeTime < 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("ApplyMusicSetting failed: MusicFadeTime is invalid"));
		return nullptr;
	}

	if (Setting.NewMusicStartTime < 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("ApplyMusicSetting failed: NewMusicStartTime is invalid"));
		return nullptr;
	}

	if (Setting.NewMusicPitch <= 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("ApplyMusicSetting failed: NewMusicPitch is invalid"));
		return nullptr;
	}

	if (Setting.bPushSoundMix)
	{
		UGameplayStatics::PushSoundMixModifier(WorldContextObject, Setting.SoundMix);
	}

	UGameplayStatics::SetSoundMixClassOverride(WorldContextObject, Setting.SoundMix, Setting.SoundClass, Setting.Volume, Setting.Pitch, Setting.VolumeFadeTime, Setting.bApplyToChildren);

	if (!Setting.bTransitionToNewMusic)
	{
		return CurrentMusicComponent;
	}

	if (!Setting.NewMusic)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("ApplyMusicSetting failed: NewMusic is invalid"));
		return nullptr;
	}

	if (CurrentMusicComponent)
	{
		if (Setting.bFadeOutCurrentMusic)
		{
			CurrentMusicComponent->FadeOut(Setting.MusicFadeTime, 0.0f);
		}
		else
		{
			CurrentMusicComponent->Stop();
		}
	}

	const float InitialMusicVolume = Setting.MusicFadeTime > 0.0f ? 0.0f : 1.0f;
	UAudioComponent* NewMusicComponent = UGameplayStatics::SpawnSound2D(WorldContextObject, Setting.NewMusic, InitialMusicVolume, Setting.NewMusicPitch, Setting.NewMusicStartTime, Setting.NewMusicConcurrencySettings, Setting.bPersistAcrossLevelTransition, Setting.bAutoDestroyNewMusic);

	if (!NewMusicComponent)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("ApplyMusicSetting failed: NewMusicComponent was not created"));
		return nullptr;
	}

	if (Setting.MusicFadeTime > 0.0f)
	{
		NewMusicComponent->FadeIn(Setting.MusicFadeTime, 1.0f, Setting.NewMusicStartTime);
	}

	return NewMusicComponent;
}

float ULunarFLAudio::GetNormalizedLoudnessAtTime(ULoudnessNRT* LoudnessAnalyzer, float TimeSeconds)
{
	if (!LoudnessAnalyzer)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("GetNormalizedLoudnessAtTime failed: LoudnessAnalyzer is invalid"));
		return 0.0f;
	}

	if (TimeSeconds < 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("GetNormalizedLoudnessAtTime failed: TimeSeconds is invalid"));
		return 0.0f;
	}

	float Loudness = 0.0f;
	LoudnessAnalyzer->GetNormalizedLoudnessAtTime(TimeSeconds, Loudness);
	return FMath::Clamp(Loudness, 0.0f, 1.0f);
}

float ULunarFLAudio::GetNormalizedLoudnessAtPlaybackPercent(ULoudnessNRT* LoudnessAnalyzer, float PlaybackPercent)
{
	if (!LoudnessAnalyzer)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("GetNormalizedLoudnessAtPlaybackPercent failed: LoudnessAnalyzer is invalid"));
		return 0.0f;
	}

	if (LoudnessAnalyzer->DurationInSeconds <= 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("GetNormalizedLoudnessAtPlaybackPercent failed: DurationInSeconds is invalid"));
		return 0.0f;
	}

	const float ClampedPlaybackPercent = FMath::Clamp(PlaybackPercent, 0.0f, 1.0f);
	return GetNormalizedLoudnessAtTime(LoudnessAnalyzer, LoudnessAnalyzer->DurationInSeconds * ClampedPlaybackPercent);
}

float ULunarFLAudio::GetAudioFrequencyValueAtTime(UConstantQNRT* ConstantQAnalyzer, float TimeSeconds, ELunarAudioFrequencyRange FrequencyRange, int32 Channel)
{
	if (!ConstantQAnalyzer)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("GetAudioFrequencyValueAtTime failed: ConstantQAnalyzer is invalid"));
		return 0.0f;
	}

	if (TimeSeconds < 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("GetAudioFrequencyValueAtTime failed: TimeSeconds is invalid"));
		return 0.0f;
	}

	if (Channel < 0)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("GetAudioFrequencyValueAtTime failed: Channel is invalid"));
		return 0.0f;
	}

	TArray<float> ConstantQValues;
	ConstantQAnalyzer->GetNormalizedChannelConstantQAtTime(TimeSeconds, Channel, ConstantQValues);

	if (ConstantQValues.IsEmpty())
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Warning, TEXT("GetAudioFrequencyValueAtTime failed: ConstantQValues array is empty"));
		return 0.0f;
	}

	const int32 LastIndex = ConstantQValues.Num() - 1;
	int32 StartIndex = 0;
	int32 EndIndex = LastIndex;

	switch (FrequencyRange)
	{
	case ELunarAudioFrequencyRange::SubBass:
		StartIndex = 0;
		EndIndex = FMath::Clamp(FMath::FloorToInt(static_cast<float>(LastIndex) * 0.10f), 0, LastIndex);
		break;

	case ELunarAudioFrequencyRange::Bass:
		StartIndex = 0;
		EndIndex = FMath::Clamp(FMath::FloorToInt(static_cast<float>(LastIndex) * 0.20f), 0, LastIndex);
		break;

	case ELunarAudioFrequencyRange::LowMid:
		StartIndex = FMath::Clamp(FMath::FloorToInt(static_cast<float>(LastIndex) * 0.20f), 0, LastIndex);
		EndIndex = FMath::Clamp(FMath::FloorToInt(static_cast<float>(LastIndex) * 0.40f), StartIndex, LastIndex);
		break;

	case ELunarAudioFrequencyRange::Mid:
		StartIndex = FMath::Clamp(FMath::FloorToInt(static_cast<float>(LastIndex) * 0.40f), 0, LastIndex);
		EndIndex = FMath::Clamp(FMath::FloorToInt(static_cast<float>(LastIndex) * 0.60f), StartIndex, LastIndex);
		break;

	case ELunarAudioFrequencyRange::HighMid:
		StartIndex = FMath::Clamp(FMath::FloorToInt(static_cast<float>(LastIndex) * 0.60f), 0, LastIndex);
		EndIndex = FMath::Clamp(FMath::FloorToInt(static_cast<float>(LastIndex) * 0.80f), StartIndex, LastIndex);
		break;

	case ELunarAudioFrequencyRange::High:
		StartIndex = FMath::Clamp(FMath::FloorToInt(static_cast<float>(LastIndex) * 0.80f), 0, LastIndex);
		EndIndex = LastIndex;
		break;

	case ELunarAudioFrequencyRange::Full:
	default:
		StartIndex = 0;
		EndIndex = LastIndex;
		break;
	}

	float Sum = 0.0f;
	int32 Count = 0;

	for (int32 Index = StartIndex; Index <= EndIndex; ++Index)
	{
		Sum += ConstantQValues[Index];
		++Count;
	}

	if (Count <= 0)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("GetAudioFrequencyValueAtTime failed: Frequency range is empty"));
		return 0.0f;
	}

	return FMath::Clamp(Sum / static_cast<float>(Count), 0.0f, 1.0f);
}

float ULunarFLAudio::GetAudioFrequencyValueAtPlaybackPercent(UConstantQNRT* ConstantQAnalyzer, float PlaybackPercent, ELunarAudioFrequencyRange FrequencyRange, int32 Channel)
{
	if (!ConstantQAnalyzer)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("GetAudioFrequencyValueAtPlaybackPercent failed: ConstantQAnalyzer is invalid"));
		return 0.0f;
	}

	if (ConstantQAnalyzer->DurationInSeconds <= 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("GetAudioFrequencyValueAtPlaybackPercent failed: DurationInSeconds is invalid"));
		return 0.0f;
	}

	const float ClampedPlaybackPercent = FMath::Clamp(PlaybackPercent, 0.0f, 1.0f);
	return GetAudioFrequencyValueAtTime(ConstantQAnalyzer, ConstantQAnalyzer->DurationInSeconds * ClampedPlaybackPercent, FrequencyRange, Channel);
}

float ULunarFLAudio::GetAudioBeatPulse(float CurrentValue, float PreviousValue, float Threshold, float Sensitivity)
{
	if (Threshold < 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("GetAudioBeatPulse failed: Threshold is invalid"));
		return 0.0f;
	}

	if (Sensitivity < 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("GetAudioBeatPulse failed: Sensitivity is invalid"));
		return 0.0f;
	}

	const float Delta = CurrentValue - PreviousValue;
	return FMath::Clamp((Delta - Threshold) * Sensitivity, 0.0f, 1.0f);
}

float ULunarFLAudio::SmoothAudioReactiveValue(float CurrentValue, float TargetValue, float DeltaTime, float InterpSpeed)
{
	if (DeltaTime < 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("SmoothAudioReactiveValue failed: DeltaTime is invalid"));
		return CurrentValue;
	}

	if (InterpSpeed < 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Audio, ELunarConsoleMessageVerbosity::Error, TEXT("SmoothAudioReactiveValue failed: InterpSpeed is invalid"));
		return CurrentValue;
	}

	return FMath::Clamp(FMath::FInterpTo(CurrentValue, TargetValue, DeltaTime, InterpSpeed), 0.0f, 1.0f);
}

float ULunarFLAudio::LinearVolumeToDecibels(float LinearVolume)
{
	if (LinearVolume <= 0.0f)
	{
		return -80.0f;
	}

	return 20.0f * FMath::LogX(10.0f, LinearVolume);
}

float ULunarFLAudio::DecibelsToLinearVolume(float Decibels)
{
	return FMath::Pow(10.0f, Decibels / 20.0f);
}
