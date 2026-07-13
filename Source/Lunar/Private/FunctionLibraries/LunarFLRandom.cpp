// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "FunctionLibraries/LunarFLRandom.h"

#include "Containers/ScriptArray.h"
#include "NativeGameplayTags.h"
#include "Subsystems/Console/LunarConsoleSubsystem.h"
#include "UObject/UnrealType.h"

#include <random>

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Lunar_Random, "Lunar.Random");

namespace LunarRandomInternal
{
	static uint64 MixSeed(uint64 Value)
	{
		Value += 0x9E3779B97F4A7C15ull;
		Value = (Value ^ (Value >> 30)) * 0xBF58476D1CE4E5B9ull;
		Value = (Value ^ (Value >> 27)) * 0x94D049BB133111EBull;
		return Value ^ (Value >> 31);
	}

	static uint64 MakeStreamSeed(const FLunarRandomStream& Stream)
	{
		return MixSeed(static_cast<uint64>(Stream.Seed) ^ MixSeed(static_cast<uint64>(Stream.Step)));
	}

	static int64 MakeRuntimeSeed()
	{
		return static_cast<int64>(MixSeed(static_cast<uint64>(FPlatformTime::Cycles64())));
	}

	template<typename TEngine>
	static TEngine MakeSeededEngine(uint64 Seed)
	{
		return TEngine(static_cast<typename TEngine::result_type>(Seed));
	}

	template<typename TCallback>
	static auto UseRandomEngine(ELunarRandomQuality Quality, TCallback&& Callback) -> decltype(Callback(std::declval<std::mt19937_64&>()))
	{
		switch (Quality)
		{
		case ELunarRandomQuality::VeryFast:
		{
			static thread_local std::minstd_rand Engine(static_cast<std::minstd_rand::result_type>(MixSeed(static_cast<uint64>(FPlatformTime::Cycles64()))));
			return Callback(Engine);
		}
		case ELunarRandomQuality::Fast:
		{
			static thread_local std::mt19937 Engine(static_cast<std::mt19937::result_type>(MixSeed(static_cast<uint64>(FPlatformTime::Cycles64()))));
			return Callback(Engine);
		}
		case ELunarRandomQuality::Slow:
		{
			static thread_local std::ranlux24 Engine(static_cast<std::ranlux24::result_type>(MixSeed(static_cast<uint64>(FPlatformTime::Cycles64()))));
			return Callback(Engine);
		}
		case ELunarRandomQuality::VerySlow:
		{
			static thread_local std::ranlux48 Engine(static_cast<std::ranlux48::result_type>(MixSeed(static_cast<uint64>(FPlatformTime::Cycles64()))));
			return Callback(Engine);
		}
		case ELunarRandomQuality::Normal:
		default:
		{
			static thread_local std::mt19937_64 Engine(static_cast<std::mt19937_64::result_type>(MixSeed(static_cast<uint64>(FPlatformTime::Cycles64()))));
			return Callback(Engine);
		}
		}
	}

	template<typename TCallback>
	static auto UseSeededRandomEngine(ELunarRandomQuality Quality, uint64 Seed, TCallback&& Callback) -> decltype(Callback(std::declval<std::mt19937_64&>()))
	{
		switch (Quality)
		{
		case ELunarRandomQuality::VeryFast:
		{
			std::minstd_rand Engine = MakeSeededEngine<std::minstd_rand>(Seed);
			return Callback(Engine);
		}
		case ELunarRandomQuality::Fast:
		{
			std::mt19937 Engine = MakeSeededEngine<std::mt19937>(Seed);
			return Callback(Engine);
		}
		case ELunarRandomQuality::Slow:
		{
			std::ranlux24 Engine = MakeSeededEngine<std::ranlux24>(Seed);
			return Callback(Engine);
		}
		case ELunarRandomQuality::VerySlow:
		{
			std::ranlux48 Engine = MakeSeededEngine<std::ranlux48>(Seed);
			return Callback(Engine);
		}
		case ELunarRandomQuality::Normal:
		default:
		{
			std::mt19937_64 Engine = MakeSeededEngine<std::mt19937_64>(Seed);
			return Callback(Engine);
		}
		}
	}

	static double FixChance(double Chance)
	{
		if (Chance < 0.0)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, TEXT("Random chance was less than zero and was clamped to zero."));
			return 0.0;
		}

		if (Chance > 1.0)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, TEXT("Random chance was greater than one and was clamped to one."));
			return 1.0;
		}

		return Chance;
	}

	template<typename TValue>
	static void FixRange(TValue& Min, TValue& Max, const FString& FunctionName)
	{
		if (Min <= Max)
		{
			return;
		}

		Swap(Min, Max);
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, FString::Printf(TEXT("%s received Min greater than Max and swapped values."), *FunctionName));
	}

	static double FixWeight(double Weight, const FString& FunctionName)
	{
		if (Weight >= 0.0)
		{
			return Weight;
		}

		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, FString::Printf(TEXT("%s received negative weight and changed it to zero."), *FunctionName));
		return 0.0;
	}

	static TArray<double> MakeFixedWeights(const TArray<double>& Weights, const FString& FunctionName)
	{
		TArray<double> FixedWeights;
		FixedWeights.Reserve(Weights.Num());

		bool bHasNegativeWeight = false;

		for (double Weight : Weights)
		{
			if (Weight < 0.0)
			{
				bHasNegativeWeight = true;
			}

			FixedWeights.Add(FMath::Max(0.0, Weight));
		}

		if (bHasNegativeWeight)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, FString::Printf(TEXT("%s received negative weight and changed it to zero."), *FunctionName));
		}

		return FixedWeights;
	}

	static double GetTotalWeight(const TArray<double>& Weights)
	{
		double TotalWeight = 0.0;

		for (double Weight : Weights)
		{
			TotalWeight += FMath::Max(0.0, Weight);
		}

		return TotalWeight;
	}

	static int32 GetPositiveWeightCount(const TArray<double>& Weights)
	{
		int32 Count = 0;

		for (double Weight : Weights)
		{
			if (Weight > 0.0)
			{
				++Count;
			}
		}

		return Count;
	}

	static bool GetRandomBoolByChanceInternal(double Chance, ELunarRandomQuality Quality)
	{
		if (Chance <= 0.0)
		{
			return false;
		}

		if (Chance >= 1.0)
		{
			return true;
		}

		return UseRandomEngine(Quality, [Chance](auto& Engine)
			{
				std::uniform_real_distribution<double> Distribution(0.0, 1.0);
				return Distribution(Engine) <= Chance;
			});
	}

	static bool GetRandomBoolByChanceFromSeedInternal(double Chance, int64 Seed, ELunarRandomQuality Quality)
	{
		if (Chance <= 0.0)
		{
			return false;
		}

		if (Chance >= 1.0)
		{
			return true;
		}

		return UseSeededRandomEngine(Quality, static_cast<uint64>(Seed), [Chance](auto& Engine)
			{
				std::uniform_real_distribution<double> Distribution(0.0, 1.0);
				return Distribution(Engine) <= Chance;
			});
	}

	static int32 GetRandomIntInRangeInternal(int32 Min, int32 Max, ELunarRandomQuality Quality)
	{
		return UseRandomEngine(Quality, [Min, Max](auto& Engine)
			{
				std::uniform_int_distribution<int32> Distribution(Min, Max);
				return Distribution(Engine);
			});
	}

	static int32 GetRandomIntInRangeFromSeedInternal(int32 Min, int32 Max, int64 Seed, ELunarRandomQuality Quality)
	{
		return UseSeededRandomEngine(Quality, static_cast<uint64>(Seed), [Min, Max](auto& Engine)
			{
				std::uniform_int_distribution<int32> Distribution(Min, Max);
				return Distribution(Engine);
			});
	}

	static int64 GetRandomInt64InRangeInternal(int64 Min, int64 Max, ELunarRandomQuality Quality)
	{
		return UseRandomEngine(Quality, [Min, Max](auto& Engine)
			{
				std::uniform_int_distribution<int64> Distribution(Min, Max);
				return Distribution(Engine);
			});
	}

	static int64 GetRandomInt64InRangeFromSeedInternal(int64 Min, int64 Max, int64 Seed, ELunarRandomQuality Quality)
	{
		return UseSeededRandomEngine(Quality, static_cast<uint64>(Seed), [Min, Max](auto& Engine)
			{
				std::uniform_int_distribution<int64> Distribution(Min, Max);
				return Distribution(Engine);
			});
	}

	static float GetRandomFloatInRangeInternal(float Min, float Max, ELunarRandomQuality Quality)
	{
		return UseRandomEngine(Quality, [Min, Max](auto& Engine)
			{
				std::uniform_real_distribution<float> Distribution(Min, Max);
				return Distribution(Engine);
			});
	}

	static float GetRandomFloatInRangeFromSeedInternal(float Min, float Max, int64 Seed, ELunarRandomQuality Quality)
	{
		return UseSeededRandomEngine(Quality, static_cast<uint64>(Seed), [Min, Max](auto& Engine)
			{
				std::uniform_real_distribution<float> Distribution(Min, Max);
				return Distribution(Engine);
			});
	}

	static double GetRandomDoubleInRangeInternal(double Min, double Max, ELunarRandomQuality Quality)
	{
		return UseRandomEngine(Quality, [Min, Max](auto& Engine)
			{
				std::uniform_real_distribution<double> Distribution(Min, Max);
				return Distribution(Engine);
			});
	}

	static double GetRandomDoubleInRangeFromSeedInternal(double Min, double Max, int64 Seed, ELunarRandomQuality Quality)
	{
		return UseSeededRandomEngine(Quality, static_cast<uint64>(Seed), [Min, Max](auto& Engine)
			{
				std::uniform_real_distribution<double> Distribution(Min, Max);
				return Distribution(Engine);
			});
	}

	static FVector GetRandomUnitVectorInternal(ELunarRandomQuality Quality)
	{
		return UseRandomEngine(Quality, [](auto& Engine)
			{
				std::uniform_real_distribution<double> Distribution(0.0, 1.0);
				const double Z = Distribution(Engine) * 2.0 - 1.0;
				const double Angle = Distribution(Engine) * 2.0 * UE_DOUBLE_PI;
				const double Radius = FMath::Sqrt(FMath::Max(0.0, 1.0 - Z * Z));
				return FVector(Radius * FMath::Cos(Angle), Radius * FMath::Sin(Angle), Z);
			});
	}

	static FVector GetRandomUnitVectorFromSeedInternal(int64 Seed, ELunarRandomQuality Quality)
	{
		const double Z = GetRandomDoubleInRangeFromSeedInternal(-1.0, 1.0, Seed + 1, Quality);
		const double Angle = GetRandomDoubleInRangeFromSeedInternal(0.0, 2.0 * UE_DOUBLE_PI, Seed + 2, Quality);
		const double Radius = FMath::Sqrt(FMath::Max(0.0, 1.0 - Z * Z));
		return FVector(Radius * FMath::Cos(Angle), Radius * FMath::Sin(Angle), Z);
	}

	static int32 GetWeightedIndexFromFixedWeightsInternal(const TArray<double>& FixedWeights, double RandomValue)
	{
		double CurrentWeight = 0.0;

		for (int32 Index = 0; Index < FixedWeights.Num(); ++Index)
		{
			CurrentWeight += FixedWeights[Index];

			if (RandomValue <= CurrentWeight)
			{
				return Index;
			}
		}

		return FixedWeights.Num() - 1;
	}

	static int32 GetRandomIndexByFixedWeightsFromSeedInternal(const TArray<double>& FixedWeights, int64 Seed, bool& bSuccess, ELunarRandomQuality Quality, const FString& FunctionName)
	{
		bSuccess = false;

		if (FixedWeights.Num() <= 0)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, FString::Printf(TEXT("%s failed because weights array is empty."), *FunctionName));
			return INDEX_NONE;
		}

		const double TotalWeight = GetTotalWeight(FixedWeights);

		if (TotalWeight <= 0.0)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Error, FString::Printf(TEXT("%s failed because total weight is zero."), *FunctionName));
			return INDEX_NONE;
		}

		const double RandomValue = GetRandomDoubleInRangeFromSeedInternal(0.0, TotalWeight, Seed, Quality);
		bSuccess = true;
		return GetWeightedIndexFromFixedWeightsInternal(FixedWeights, RandomValue);
	}

	static int32 GetRandomIndexByFixedWeightsInternal(const TArray<double>& FixedWeights, bool& bSuccess, ELunarRandomQuality Quality, const FString& FunctionName)
	{
		bSuccess = false;

		if (FixedWeights.Num() <= 0)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, FString::Printf(TEXT("%s failed because weights array is empty."), *FunctionName));
			return INDEX_NONE;
		}

		const double TotalWeight = GetTotalWeight(FixedWeights);

		if (TotalWeight <= 0.0)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Error, FString::Printf(TEXT("%s failed because total weight is zero."), *FunctionName));
			return INDEX_NONE;
		}

		const double RandomValue = GetRandomDoubleInRangeInternal(0.0, TotalWeight, Quality);
		bSuccess = true;
		return GetWeightedIndexFromFixedWeightsInternal(FixedWeights, RandomValue);
	}

	static int32 GetRandomIndexByWeightsInternal(const TArray<double>& Weights, bool& bSuccess, ELunarRandomQuality Quality, const FString& FunctionName)
	{
		const TArray<double> FixedWeights = MakeFixedWeights(Weights, FunctionName);
		return GetRandomIndexByFixedWeightsInternal(FixedWeights, bSuccess, Quality, FunctionName);
	}

	static int32 GetRandomIndexByWeightsFromSeedInternal(const TArray<double>& Weights, int64 Seed, bool& bSuccess, ELunarRandomQuality Quality, const FString& FunctionName)
	{
		const TArray<double> FixedWeights = MakeFixedWeights(Weights, FunctionName);
		return GetRandomIndexByFixedWeightsFromSeedInternal(FixedWeights, Seed, bSuccess, Quality, FunctionName);
	}

	static bool AreArrayTypesValid(const FArrayProperty* SourceArrayProperty, const FArrayProperty* TargetArrayProperty, const FString& FunctionName)
	{
		if (!SourceArrayProperty || !TargetArrayProperty)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Error, FString::Printf(TEXT("%s failed because array property is invalid."), *FunctionName));
			return false;
		}

		if (!SourceArrayProperty->Inner || !TargetArrayProperty->Inner || !SourceArrayProperty->Inner->SameType(TargetArrayProperty->Inner))
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Error, FString::Printf(TEXT("%s failed because array item types are different."), *FunctionName));
			return false;
		}

		return true;
	}

	static bool IsArrayItemTypeValid(const FArrayProperty* ArrayProperty, const FProperty* ItemProperty, const FString& FunctionName)
	{
		if (!ArrayProperty || !ArrayProperty->Inner || !ItemProperty)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Error, FString::Printf(TEXT("%s failed because array or item property is invalid."), *FunctionName));
			return false;
		}

		if (!ArrayProperty->Inner->SameType(ItemProperty))
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Error, FString::Printf(TEXT("%s failed because array item type and output item type are different."), *FunctionName));
			return false;
		}

		return true;
	}

	static void ClearItemValue(FProperty* ItemProperty, void* ItemAddress)
	{
		if (!ItemProperty || !ItemAddress)
		{
			return;
		}

		ItemProperty->DestroyValue(ItemAddress);
		ItemProperty->InitializeValue(ItemAddress);
	}

	static void ClearArrayValue(FArrayProperty* ArrayProperty, void* ArrayAddress)
	{
		if (!ArrayProperty || !ArrayAddress)
		{
			return;
		}

		FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayAddress);
		ArrayHelper.EmptyValues();
	}

	static void GenericGetRandomArrayItem(void* ArrayAddress, FArrayProperty* ArrayProperty, void* ItemAddress, FProperty* ItemProperty, bool& bSuccess, ELunarRandomQuality Quality, const FString& FunctionName)
	{
		bSuccess = false;

		if (!IsArrayItemTypeValid(ArrayProperty, ItemProperty, FunctionName))
		{
			ClearItemValue(ItemProperty, ItemAddress);
			return;
		}

		FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayAddress);

		if (ArrayHelper.Num() <= 0)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, FString::Printf(TEXT("%s failed because source array is empty."), *FunctionName));
			ClearItemValue(ItemProperty, ItemAddress);
			return;
		}

		const int32 Index = GetRandomIntInRangeInternal(0, ArrayHelper.Num() - 1, Quality);
		ItemProperty->CopyCompleteValue(ItemAddress, ArrayHelper.GetRawPtr(Index));
		bSuccess = true;
	}

	static void GenericGetRandomArrayItemFromSeed(void* ArrayAddress, FArrayProperty* ArrayProperty, void* ItemAddress, FProperty* ItemProperty, int64 Seed, bool& bSuccess, ELunarRandomQuality Quality, const FString& FunctionName)
	{
		bSuccess = false;

		if (!IsArrayItemTypeValid(ArrayProperty, ItemProperty, FunctionName))
		{
			ClearItemValue(ItemProperty, ItemAddress);
			return;
		}

		FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayAddress);

		if (ArrayHelper.Num() <= 0)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, FString::Printf(TEXT("%s failed because source array is empty."), *FunctionName));
			ClearItemValue(ItemProperty, ItemAddress);
			return;
		}

		const int32 Index = GetRandomIntInRangeFromSeedInternal(0, ArrayHelper.Num() - 1, Seed, Quality);
		ItemProperty->CopyCompleteValue(ItemAddress, ArrayHelper.GetRawPtr(Index));
		bSuccess = true;
	}

	static void GenericGetRandomArrayItemByWeightsFromSeed(void* ArrayAddress, FArrayProperty* ArrayProperty, const TArray<double>& Weights, void* ItemAddress, FProperty* ItemProperty, int64 Seed, bool& bSuccess, ELunarRandomQuality Quality, const FString& FunctionName)
	{
		bSuccess = false;

		if (!IsArrayItemTypeValid(ArrayProperty, ItemProperty, FunctionName))
		{
			ClearItemValue(ItemProperty, ItemAddress);
			return;
		}

		FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayAddress);

		if (ArrayHelper.Num() <= 0)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, FString::Printf(TEXT("%s failed because source array is empty."), *FunctionName));
			ClearItemValue(ItemProperty, ItemAddress);
			return;
		}

		if (ArrayHelper.Num() != Weights.Num())
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Error, FString::Printf(TEXT("%s failed because array length and weights length are different."), *FunctionName));
			ClearItemValue(ItemProperty, ItemAddress);
			return;
		}

		const int32 Index = GetRandomIndexByWeightsFromSeedInternal(Weights, Seed, bSuccess, Quality, FunctionName);

		if (!bSuccess || !ArrayHelper.IsValidIndex(Index))
		{
			ClearItemValue(ItemProperty, ItemAddress);
			bSuccess = false;
			return;
		}

		ItemProperty->CopyCompleteValue(ItemAddress, ArrayHelper.GetRawPtr(Index));
		bSuccess = true;
	}

	static void GenericGetRandomArrayItemsFromSeed(void* ArrayAddress, FArrayProperty* SourceArrayProperty, int32 Count, bool bUnique, void* ItemsAddress, FArrayProperty* TargetArrayProperty, int64 Seed, bool& bSuccess, ELunarRandomQuality Quality, const FString& FunctionName)
	{
		bSuccess = false;

		if (!AreArrayTypesValid(SourceArrayProperty, TargetArrayProperty, FunctionName))
		{
			ClearArrayValue(TargetArrayProperty, ItemsAddress);
			return;
		}

		FScriptArrayHelper SourceArrayHelper(SourceArrayProperty, ArrayAddress);
		FScriptArrayHelper TargetArrayHelper(TargetArrayProperty, ItemsAddress);
		TargetArrayHelper.EmptyValues();

		if (SourceArrayHelper.Num() <= 0)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, FString::Printf(TEXT("%s failed because source array is empty."), *FunctionName));
			return;
		}

		if (Count <= 0)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, FString::Printf(TEXT("%s failed because count is less than one."), *FunctionName));
			return;
		}

		int32 FixedCount = Count;

		if (bUnique && FixedCount > SourceArrayHelper.Num())
		{
			FixedCount = SourceArrayHelper.Num();
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, FString::Printf(TEXT("%s received count greater than source array length and changed it to maximum possible count."), *FunctionName));
		}

		if (bUnique)
		{
			TArray<int32> Indices;
			Indices.Reserve(SourceArrayHelper.Num());

			for (int32 Index = 0; Index < SourceArrayHelper.Num(); ++Index)
			{
				Indices.Add(Index);
			}

			for (int32 Index = Indices.Num() - 1; Index > 0; --Index)
			{
				const int32 SwapIndex = GetRandomIntInRangeFromSeedInternal(0, Index, static_cast<int64>(MixSeed(static_cast<uint64>(Seed) + static_cast<uint64>(Index))), Quality);
				Indices.Swap(Index, SwapIndex);
			}

			for (int32 Index = 0; Index < FixedCount; ++Index)
			{
				const int32 NewIndex = TargetArrayHelper.AddValue();
				TargetArrayProperty->Inner->CopyCompleteValue(TargetArrayHelper.GetRawPtr(NewIndex), SourceArrayHelper.GetRawPtr(Indices[Index]));
			}
		}
		else
		{
			for (int32 Index = 0; Index < FixedCount; ++Index)
			{
				const int32 SourceIndex = GetRandomIntInRangeFromSeedInternal(0, SourceArrayHelper.Num() - 1, static_cast<int64>(MixSeed(static_cast<uint64>(Seed) + static_cast<uint64>(Index))), Quality);
				const int32 NewIndex = TargetArrayHelper.AddValue();
				TargetArrayProperty->Inner->CopyCompleteValue(TargetArrayHelper.GetRawPtr(NewIndex), SourceArrayHelper.GetRawPtr(SourceIndex));
			}
		}

		bSuccess = TargetArrayHelper.Num() > 0;
	}

	static void GenericGetRandomArrayItemsByWeightsFromSeed(void* ArrayAddress, FArrayProperty* SourceArrayProperty, const TArray<double>& Weights, int32 Count, bool bUnique, void* ItemsAddress, FArrayProperty* TargetArrayProperty, int64 Seed, bool& bSuccess, ELunarRandomQuality Quality, const FString& FunctionName)
	{
		bSuccess = false;

		if (!AreArrayTypesValid(SourceArrayProperty, TargetArrayProperty, FunctionName))
		{
			ClearArrayValue(TargetArrayProperty, ItemsAddress);
			return;
		}

		FScriptArrayHelper SourceArrayHelper(SourceArrayProperty, ArrayAddress);
		FScriptArrayHelper TargetArrayHelper(TargetArrayProperty, ItemsAddress);
		TargetArrayHelper.EmptyValues();

		if (SourceArrayHelper.Num() <= 0)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, FString::Printf(TEXT("%s failed because source array is empty."), *FunctionName));
			return;
		}

		if (SourceArrayHelper.Num() != Weights.Num())
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Error, FString::Printf(TEXT("%s failed because array length and weights length are different."), *FunctionName));
			return;
		}

		if (Count <= 0)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, FString::Printf(TEXT("%s failed because count is less than one."), *FunctionName));
			return;
		}

		TArray<double> FixedWeights = MakeFixedWeights(Weights, FunctionName);
		int32 FixedCount = Count;

		if (bUnique)
		{
			const int32 PositiveWeightCount = GetPositiveWeightCount(FixedWeights);

			if (PositiveWeightCount <= 0)
			{
				ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Error, FString::Printf(TEXT("%s failed because total weight is zero."), *FunctionName));
				return;
			}

			if (FixedCount > PositiveWeightCount)
			{
				FixedCount = PositiveWeightCount;
				ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, FString::Printf(TEXT("%s received count greater than positive weight count and changed it to maximum possible count."), *FunctionName));
			}

			for (int32 Index = 0; Index < FixedCount; ++Index)
			{
				bool bIndexSuccess = false;
				const int32 SourceIndex = GetRandomIndexByFixedWeightsFromSeedInternal(FixedWeights, static_cast<int64>(MixSeed(static_cast<uint64>(Seed) + static_cast<uint64>(Index))), bIndexSuccess, Quality, FunctionName);

				if (!bIndexSuccess || !SourceArrayHelper.IsValidIndex(SourceIndex))
				{
					break;
				}

				const int32 NewIndex = TargetArrayHelper.AddValue();
				TargetArrayProperty->Inner->CopyCompleteValue(TargetArrayHelper.GetRawPtr(NewIndex), SourceArrayHelper.GetRawPtr(SourceIndex));
				FixedWeights[SourceIndex] = 0.0;
			}
		}
		else
		{
			if (GetTotalWeight(FixedWeights) <= 0.0)
			{
				ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Error, FString::Printf(TEXT("%s failed because total weight is zero."), *FunctionName));
				return;
			}

			for (int32 Index = 0; Index < FixedCount; ++Index)
			{
				bool bIndexSuccess = false;
				const int32 SourceIndex = GetRandomIndexByFixedWeightsFromSeedInternal(FixedWeights, static_cast<int64>(MixSeed(static_cast<uint64>(Seed) + static_cast<uint64>(Index))), bIndexSuccess, Quality, FunctionName);

				if (!bIndexSuccess || !SourceArrayHelper.IsValidIndex(SourceIndex))
				{
					break;
				}

				const int32 NewIndex = TargetArrayHelper.AddValue();
				TargetArrayProperty->Inner->CopyCompleteValue(TargetArrayHelper.GetRawPtr(NewIndex), SourceArrayHelper.GetRawPtr(SourceIndex));
			}
		}

		bSuccess = TargetArrayHelper.Num() > 0;
	}

	static void GenericShuffleArrayFromSeed(void* ArrayAddress, FArrayProperty* ArrayProperty, int64 Seed, bool& bSuccess, ELunarRandomQuality Quality, const FString& FunctionName)
	{
		bSuccess = false;

		if (!ArrayProperty || !ArrayAddress)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Error, FString::Printf(TEXT("%s failed because array property is invalid."), *FunctionName));
			return;
		}

		FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayAddress);

		if (ArrayHelper.Num() <= 0)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, FString::Printf(TEXT("%s failed because array is empty."), *FunctionName));
			return;
		}

		for (int32 Index = ArrayHelper.Num() - 1; Index > 0; --Index)
		{
			const int32 SwapIndex = GetRandomIntInRangeFromSeedInternal(0, Index, static_cast<int64>(MixSeed(static_cast<uint64>(Seed) + static_cast<uint64>(Index))), Quality);
			ArrayHelper.SwapValues(Index, SwapIndex);
		}

		bSuccess = true;
	}

	static void GetRandomIntArrayItemsFromSeed(const TArray<int32>& Array, int32 Count, bool bUnique, TArray<int32>& Items, int64 Seed, bool& bSuccess, ELunarRandomQuality Quality, const FString& FunctionName)
	{
		bSuccess = false;
		Items.Reset();

		if (Array.Num() <= 0)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, FString::Printf(TEXT("%s failed because source array is empty."), *FunctionName));
			return;
		}

		if (Count <= 0)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, FString::Printf(TEXT("%s failed because count is less than one."), *FunctionName));
			return;
		}

		int32 FixedCount = Count;

		if (bUnique && FixedCount > Array.Num())
		{
			FixedCount = Array.Num();
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, FString::Printf(TEXT("%s received count greater than source array length and changed it to maximum possible count."), *FunctionName));
		}

		if (bUnique)
		{
			TArray<int32> Indices;
			Indices.Reserve(Array.Num());

			for (int32 Index = 0; Index < Array.Num(); ++Index)
			{
				Indices.Add(Index);
			}

			for (int32 Index = Indices.Num() - 1; Index > 0; --Index)
			{
				const int32 SwapIndex = GetRandomIntInRangeFromSeedInternal(0, Index, static_cast<int64>(MixSeed(static_cast<uint64>(Seed) + static_cast<uint64>(Index))), Quality);
				Indices.Swap(Index, SwapIndex);
			}

			for (int32 Index = 0; Index < FixedCount; ++Index)
			{
				Items.Add(Array[Indices[Index]]);
			}
		}
		else
		{
			for (int32 Index = 0; Index < FixedCount; ++Index)
			{
				const int32 SourceIndex = GetRandomIntInRangeFromSeedInternal(0, Array.Num() - 1, static_cast<int64>(MixSeed(static_cast<uint64>(Seed) + static_cast<uint64>(Index))), Quality);
				Items.Add(Array[SourceIndex]);
			}
		}

		bSuccess = Items.Num() > 0;
	}

	static void GetRandomIntArrayItemsByWeightsFromSeed(const TArray<int32>& Array, const TArray<double>& Weights, int32 Count, bool bUnique, TArray<int32>& Items, int64 Seed, bool& bSuccess, ELunarRandomQuality Quality, const FString& FunctionName)
	{
		bSuccess = false;
		Items.Reset();

		if (Array.Num() <= 0)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, FString::Printf(TEXT("%s failed because source array is empty."), *FunctionName));
			return;
		}

		if (Array.Num() != Weights.Num())
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Error, FString::Printf(TEXT("%s failed because array length and weights length are different."), *FunctionName));
			return;
		}

		if (Count <= 0)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, FString::Printf(TEXT("%s failed because count is less than one."), *FunctionName));
			return;
		}

		TArray<double> FixedWeights = MakeFixedWeights(Weights, FunctionName);
		int32 FixedCount = Count;

		if (bUnique)
		{
			const int32 PositiveWeightCount = GetPositiveWeightCount(FixedWeights);

			if (PositiveWeightCount <= 0)
			{
				ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Error, FString::Printf(TEXT("%s failed because total weight is zero."), *FunctionName));
				return;
			}

			if (FixedCount > PositiveWeightCount)
			{
				FixedCount = PositiveWeightCount;
				ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, FString::Printf(TEXT("%s received count greater than positive weight count and changed it to maximum possible count."), *FunctionName));
			}

			for (int32 Index = 0; Index < FixedCount; ++Index)
			{
				bool bIndexSuccess = false;
				const int32 SourceIndex = GetRandomIndexByFixedWeightsFromSeedInternal(FixedWeights, static_cast<int64>(MixSeed(static_cast<uint64>(Seed) + static_cast<uint64>(Index))), bIndexSuccess, Quality, FunctionName);

				if (!bIndexSuccess || !Array.IsValidIndex(SourceIndex))
				{
					break;
				}

				Items.Add(Array[SourceIndex]);
				FixedWeights[SourceIndex] = 0.0;
			}
		}
		else
		{
			if (GetTotalWeight(FixedWeights) <= 0.0)
			{
				ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Error, FString::Printf(TEXT("%s failed because total weight is zero."), *FunctionName));
				return;
			}

			for (int32 Index = 0; Index < FixedCount; ++Index)
			{
				bool bIndexSuccess = false;
				const int32 SourceIndex = GetRandomIndexByFixedWeightsFromSeedInternal(FixedWeights, static_cast<int64>(MixSeed(static_cast<uint64>(Seed) + static_cast<uint64>(Index))), bIndexSuccess, Quality, FunctionName);

				if (!bIndexSuccess || !Array.IsValidIndex(SourceIndex))
				{
					break;
				}

				Items.Add(Array[SourceIndex]);
			}
		}

		bSuccess = Items.Num() > 0;
	}
}

FLunarRandomStream ULunarFLRandom::MakeRandomStream(int64 Seed, ELunarRandomQuality Quality)
{
	FLunarRandomStream Stream;
	Stream.Seed = Seed;
	Stream.Step = 0;
	Stream.Quality = Quality;
	return Stream;
}

void ULunarFLRandom::ResetRandomStream(FLunarRandomStream& Stream)
{
	Stream.Step = 0;
}

void ULunarFLRandom::AdvanceRandomStream(FLunarRandomStream& Stream, int64 StepCount)
{
	Stream.Step += StepCount;
}

FString ULunarFLRandom::GetRandomQualityDescription(ELunarRandomQuality Quality)
{
	switch (Quality)
	{
	case ELunarRandomQuality::VeryFast:
		return TEXT("std minstd rand fastest generator with lower quality for cheap gameplay random");
	case ELunarRandomQuality::Fast:
		return TEXT("std mt19937 fast generator with good quality for common gameplay random");
	case ELunarRandomQuality::Normal:
		return TEXT("std mt19937 64 default generator with good 64 bit quality");
	case ELunarRandomQuality::Slow:
		return TEXT("std ranlux24 slower generator with higher quality");
	case ELunarRandomQuality::VerySlow:
		return TEXT("std ranlux48 slowest generator with highest quality in this set");
	default:
		return TEXT("Unknown random quality");
	}
}

FString ULunarFLRandom::GetRandomQualitySlowdown(ELunarRandomQuality Quality)
{
	switch (Quality)
	{
	case ELunarRandomQuality::VeryFast:
		return TEXT("About 1x baseline speed");
	case ELunarRandomQuality::Fast:
		return TEXT("About 2x to 5x slower than Very Fast");
	case ELunarRandomQuality::Normal:
		return TEXT("About 2x to 6x slower than Very Fast");
	case ELunarRandomQuality::Slow:
		return TEXT("About 5x to 20x slower than Very Fast");
	case ELunarRandomQuality::VerySlow:
		return TEXT("About 10x to 50x slower than Very Fast");
	default:
		return TEXT("Unknown slowdown");
	}
}

bool ULunarFLRandom::GetRandomBool(ELunarRandomQuality Quality)
{
	return LunarRandomInternal::GetRandomBoolByChanceInternal(0.5, Quality);
}

bool ULunarFLRandom::GetRandomBoolFromSeed(int64 Seed, ELunarRandomQuality Quality)
{
	return LunarRandomInternal::GetRandomBoolByChanceFromSeedInternal(0.5, Seed, Quality);
}

bool ULunarFLRandom::GetRandomBoolFromStream(FLunarRandomStream& Stream)
{
	const bool Result = LunarRandomInternal::GetRandomBoolByChanceFromSeedInternal(0.5, static_cast<int64>(LunarRandomInternal::MakeStreamSeed(Stream)), Stream.Quality);
	++Stream.Step;
	return Result;
}

bool ULunarFLRandom::GetRandomBoolWithChance(const UObject* WorldContextObject, double Chance, ELunarRandomQuality Quality)
{
	return LunarRandomInternal::GetRandomBoolByChanceInternal(LunarRandomInternal::FixChance(Chance), Quality);
}

bool ULunarFLRandom::GetRandomBoolWithChanceFromSeed(const UObject* WorldContextObject, double Chance, int64 Seed, ELunarRandomQuality Quality)
{
	return LunarRandomInternal::GetRandomBoolByChanceFromSeedInternal(LunarRandomInternal::FixChance(Chance), Seed, Quality);
}

bool ULunarFLRandom::GetRandomBoolWithChanceFromStream(const UObject* WorldContextObject, FLunarRandomStream& Stream, double Chance)
{
	const bool Result = LunarRandomInternal::GetRandomBoolByChanceFromSeedInternal(LunarRandomInternal::FixChance(Chance), static_cast<int64>(LunarRandomInternal::MakeStreamSeed(Stream)), Stream.Quality);
	++Stream.Step;
	return Result;
}

bool ULunarFLRandom::GetRandomBoolByWeight(const UObject* WorldContextObject, double Weight, double TotalWeight, ELunarRandomQuality Quality)
{
	const double FixedWeight = LunarRandomInternal::FixWeight(Weight, TEXT("Random Bool By Weight"));
	const double FixedTotalWeight = LunarRandomInternal::FixWeight(TotalWeight, TEXT("Random Bool By Weight"));

	if (FixedTotalWeight <= 0.0)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Error, TEXT("Random Bool By Weight failed because total weight is zero."));
		return false;
	}

	return GetRandomBoolWithChance(WorldContextObject, FixedWeight / FixedTotalWeight, Quality);
}

bool ULunarFLRandom::GetRandomBoolByWeightFromSeed(const UObject* WorldContextObject, double Weight, double TotalWeight, int64 Seed, ELunarRandomQuality Quality)
{
	const double FixedWeight = LunarRandomInternal::FixWeight(Weight, TEXT("Random Bool By Weight From Seed"));
	const double FixedTotalWeight = LunarRandomInternal::FixWeight(TotalWeight, TEXT("Random Bool By Weight From Seed"));

	if (FixedTotalWeight <= 0.0)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Error, TEXT("Random Bool By Weight From Seed failed because total weight is zero."));
		return false;
	}

	return GetRandomBoolWithChanceFromSeed(WorldContextObject, FixedWeight / FixedTotalWeight, Seed, Quality);
}

bool ULunarFLRandom::GetRandomBoolByWeightFromStream(const UObject* WorldContextObject, FLunarRandomStream& Stream, double Weight, double TotalWeight)
{
	const double FixedWeight = LunarRandomInternal::FixWeight(Weight, TEXT("Random Bool By Weight From Stream"));
	const double FixedTotalWeight = LunarRandomInternal::FixWeight(TotalWeight, TEXT("Random Bool By Weight From Stream"));

	if (FixedTotalWeight <= 0.0)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Error, TEXT("Random Bool By Weight From Stream failed because total weight is zero."));
		return false;
	}

	return GetRandomBoolWithChanceFromStream(WorldContextObject, Stream, FixedWeight / FixedTotalWeight);
}

int32 ULunarFLRandom::GetRandomIntegerInRange(const UObject* WorldContextObject, int32 Min, int32 Max, ELunarRandomQuality Quality)
{
	LunarRandomInternal::FixRange(Min, Max, TEXT("Random Integer In Range"));
	return LunarRandomInternal::GetRandomIntInRangeInternal(Min, Max, Quality);
}

int32 ULunarFLRandom::GetRandomIntegerInRangeFromSeed(const UObject* WorldContextObject, int32 Min, int32 Max, int64 Seed, ELunarRandomQuality Quality)
{
	LunarRandomInternal::FixRange(Min, Max, TEXT("Random Integer In Range From Seed"));
	return LunarRandomInternal::GetRandomIntInRangeFromSeedInternal(Min, Max, Seed, Quality);
}

int32 ULunarFLRandom::GetRandomIntegerInRangeFromStream(const UObject* WorldContextObject, FLunarRandomStream& Stream, int32 Min, int32 Max)
{
	LunarRandomInternal::FixRange(Min, Max, TEXT("Random Integer In Range From Stream"));
	const int32 Result = LunarRandomInternal::GetRandomIntInRangeFromSeedInternal(Min, Max, static_cast<int64>(LunarRandomInternal::MakeStreamSeed(Stream)), Stream.Quality);
	++Stream.Step;
	return Result;
}

int64 ULunarFLRandom::GetRandomInteger64InRange(const UObject* WorldContextObject, int64 Min, int64 Max, ELunarRandomQuality Quality)
{
	LunarRandomInternal::FixRange(Min, Max, TEXT("Random Integer 64 In Range"));
	return LunarRandomInternal::GetRandomInt64InRangeInternal(Min, Max, Quality);
}

int64 ULunarFLRandom::GetRandomInteger64InRangeFromSeed(const UObject* WorldContextObject, int64 Min, int64 Max, int64 Seed, ELunarRandomQuality Quality)
{
	LunarRandomInternal::FixRange(Min, Max, TEXT("Random Integer 64 In Range From Seed"));
	return LunarRandomInternal::GetRandomInt64InRangeFromSeedInternal(Min, Max, Seed, Quality);
}

int64 ULunarFLRandom::GetRandomInteger64InRangeFromStream(const UObject* WorldContextObject, FLunarRandomStream& Stream, int64 Min, int64 Max)
{
	LunarRandomInternal::FixRange(Min, Max, TEXT("Random Integer 64 In Range From Stream"));
	const int64 Result = LunarRandomInternal::GetRandomInt64InRangeFromSeedInternal(Min, Max, static_cast<int64>(LunarRandomInternal::MakeStreamSeed(Stream)), Stream.Quality);
	++Stream.Step;
	return Result;
}

float ULunarFLRandom::GetRandomFloatInRange(const UObject* WorldContextObject, float Min, float Max, ELunarRandomQuality Quality)
{
	LunarRandomInternal::FixRange(Min, Max, TEXT("Random Float In Range"));
	return LunarRandomInternal::GetRandomFloatInRangeInternal(Min, Max, Quality);
}

float ULunarFLRandom::GetRandomFloatInRangeFromSeed(const UObject* WorldContextObject, float Min, float Max, int64 Seed, ELunarRandomQuality Quality)
{
	LunarRandomInternal::FixRange(Min, Max, TEXT("Random Float In Range From Seed"));
	return LunarRandomInternal::GetRandomFloatInRangeFromSeedInternal(Min, Max, Seed, Quality);
}

float ULunarFLRandom::GetRandomFloatInRangeFromStream(const UObject* WorldContextObject, FLunarRandomStream& Stream, float Min, float Max)
{
	LunarRandomInternal::FixRange(Min, Max, TEXT("Random Float In Range From Stream"));
	const float Result = LunarRandomInternal::GetRandomFloatInRangeFromSeedInternal(Min, Max, static_cast<int64>(LunarRandomInternal::MakeStreamSeed(Stream)), Stream.Quality);
	++Stream.Step;
	return Result;
}

double ULunarFLRandom::GetRandomDoubleInRange(const UObject* WorldContextObject, double Min, double Max, ELunarRandomQuality Quality)
{
	LunarRandomInternal::FixRange(Min, Max, TEXT("Random Double In Range"));
	return LunarRandomInternal::GetRandomDoubleInRangeInternal(Min, Max, Quality);
}

double ULunarFLRandom::GetRandomDoubleInRangeFromSeed(const UObject* WorldContextObject, double Min, double Max, int64 Seed, ELunarRandomQuality Quality)
{
	LunarRandomInternal::FixRange(Min, Max, TEXT("Random Double In Range From Seed"));
	return LunarRandomInternal::GetRandomDoubleInRangeFromSeedInternal(Min, Max, Seed, Quality);
}

double ULunarFLRandom::GetRandomDoubleInRangeFromStream(const UObject* WorldContextObject, FLunarRandomStream& Stream, double Min, double Max)
{
	LunarRandomInternal::FixRange(Min, Max, TEXT("Random Double In Range From Stream"));
	const double Result = LunarRandomInternal::GetRandomDoubleInRangeFromSeedInternal(Min, Max, static_cast<int64>(LunarRandomInternal::MakeStreamSeed(Stream)), Stream.Quality);
	++Stream.Step;
	return Result;
}

int32 ULunarFLRandom::GetRandomIndex(const UObject* WorldContextObject, int32 Count, bool& bSuccess, ELunarRandomQuality Quality)
{
	bSuccess = false;

	if (Count <= 0)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, TEXT("Random Index failed because count is less than one."));
		return INDEX_NONE;
	}

	bSuccess = true;
	return LunarRandomInternal::GetRandomIntInRangeInternal(0, Count - 1, Quality);
}

int32 ULunarFLRandom::GetRandomIndexFromSeed(const UObject* WorldContextObject, int32 Count, int64 Seed, bool& bSuccess, ELunarRandomQuality Quality)
{
	bSuccess = false;

	if (Count <= 0)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, TEXT("Random Index From Seed failed because count is less than one."));
		return INDEX_NONE;
	}

	bSuccess = true;
	return LunarRandomInternal::GetRandomIntInRangeFromSeedInternal(0, Count - 1, Seed, Quality);
}

int32 ULunarFLRandom::GetRandomIndexFromStream(const UObject* WorldContextObject, FLunarRandomStream& Stream, int32 Count, bool& bSuccess)
{
	bSuccess = false;

	if (Count <= 0)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, TEXT("Random Index From Stream failed because count is less than one."));
		return INDEX_NONE;
	}

	const int32 Result = LunarRandomInternal::GetRandomIntInRangeFromSeedInternal(0, Count - 1, static_cast<int64>(LunarRandomInternal::MakeStreamSeed(Stream)), Stream.Quality);
	++Stream.Step;
	bSuccess = true;
	return Result;
}

int32 ULunarFLRandom::GetRandomIndexByWeights(const UObject* WorldContextObject, const TArray<double>& Weights, bool& bSuccess, ELunarRandomQuality Quality)
{
	return LunarRandomInternal::GetRandomIndexByWeightsInternal(Weights, bSuccess, Quality, TEXT("Random Index By Weights"));
}

int32 ULunarFLRandom::GetRandomIndexByWeightsFromSeed(const UObject* WorldContextObject, const TArray<double>& Weights, int64 Seed, bool& bSuccess, ELunarRandomQuality Quality)
{
	return LunarRandomInternal::GetRandomIndexByWeightsFromSeedInternal(Weights, Seed, bSuccess, Quality, TEXT("Random Index By Weights From Seed"));
}

int32 ULunarFLRandom::GetRandomIndexByWeightsFromStream(const UObject* WorldContextObject, FLunarRandomStream& Stream, const TArray<double>& Weights, bool& bSuccess)
{
	const int32 Result = LunarRandomInternal::GetRandomIndexByWeightsFromSeedInternal(Weights, static_cast<int64>(LunarRandomInternal::MakeStreamSeed(Stream)), bSuccess, Stream.Quality, TEXT("Random Index By Weights From Stream"));
	++Stream.Step;
	return Result;
}

int32 ULunarFLRandom::GetRandomSign(ELunarRandomQuality Quality)
{
	return GetRandomBool(Quality) ? 1 : -1;
}

int32 ULunarFLRandom::GetRandomSignFromSeed(int64 Seed, ELunarRandomQuality Quality)
{
	return GetRandomBoolFromSeed(Seed, Quality) ? 1 : -1;
}

int32 ULunarFLRandom::GetRandomSignFromStream(FLunarRandomStream& Stream)
{
	return GetRandomBoolFromStream(Stream) ? 1 : -1;
}

FVector ULunarFLRandom::GetRandomVectorInRange(const UObject* WorldContextObject, FVector Min, FVector Max, ELunarRandomQuality Quality)
{
	return FVector(GetRandomDoubleInRange(WorldContextObject, Min.X, Max.X, Quality), GetRandomDoubleInRange(WorldContextObject, Min.Y, Max.Y, Quality), GetRandomDoubleInRange(WorldContextObject, Min.Z, Max.Z, Quality));
}

FVector ULunarFLRandom::GetRandomVectorInRangeFromSeed(const UObject* WorldContextObject, FVector Min, FVector Max, int64 Seed, ELunarRandomQuality Quality)
{
	return FVector(GetRandomDoubleInRangeFromSeed(WorldContextObject, Min.X, Max.X, Seed + 1, Quality), GetRandomDoubleInRangeFromSeed(WorldContextObject, Min.Y, Max.Y, Seed + 2, Quality), GetRandomDoubleInRangeFromSeed(WorldContextObject, Min.Z, Max.Z, Seed + 3, Quality));
}

FVector ULunarFLRandom::GetRandomVectorInRangeFromStream(const UObject* WorldContextObject, FLunarRandomStream& Stream, FVector Min, FVector Max)
{
	return FVector(GetRandomDoubleInRangeFromStream(WorldContextObject, Stream, Min.X, Max.X), GetRandomDoubleInRangeFromStream(WorldContextObject, Stream, Min.Y, Max.Y), GetRandomDoubleInRangeFromStream(WorldContextObject, Stream, Min.Z, Max.Z));
}

FVector ULunarFLRandom::GetRandomUnitVector(ELunarRandomQuality Quality)
{
	return LunarRandomInternal::GetRandomUnitVectorInternal(Quality);
}

FVector ULunarFLRandom::GetRandomUnitVectorFromSeed(int64 Seed, ELunarRandomQuality Quality)
{
	return LunarRandomInternal::GetRandomUnitVectorFromSeedInternal(Seed, Quality);
}

FVector ULunarFLRandom::GetRandomUnitVectorFromStream(FLunarRandomStream& Stream)
{
	const FVector Result = LunarRandomInternal::GetRandomUnitVectorFromSeedInternal(static_cast<int64>(LunarRandomInternal::MakeStreamSeed(Stream)), Stream.Quality);
	++Stream.Step;
	return Result;
}

FVector ULunarFLRandom::GetRandomPointInBox(const UObject* WorldContextObject, FVector BoxExtent, ELunarRandomQuality Quality)
{
	const FVector FixedExtent = FVector(FMath::Abs(BoxExtent.X), FMath::Abs(BoxExtent.Y), FMath::Abs(BoxExtent.Z));
	return GetRandomVectorInRange(WorldContextObject, -FixedExtent, FixedExtent, Quality);
}

FVector ULunarFLRandom::GetRandomPointInBoxFromSeed(const UObject* WorldContextObject, FVector BoxExtent, int64 Seed, ELunarRandomQuality Quality)
{
	const FVector FixedExtent = FVector(FMath::Abs(BoxExtent.X), FMath::Abs(BoxExtent.Y), FMath::Abs(BoxExtent.Z));
	return GetRandomVectorInRangeFromSeed(WorldContextObject, -FixedExtent, FixedExtent, Seed, Quality);
}

FVector ULunarFLRandom::GetRandomPointInBoxFromStream(const UObject* WorldContextObject, FLunarRandomStream& Stream, FVector BoxExtent)
{
	const FVector FixedExtent = FVector(FMath::Abs(BoxExtent.X), FMath::Abs(BoxExtent.Y), FMath::Abs(BoxExtent.Z));
	return GetRandomVectorInRangeFromStream(WorldContextObject, Stream, -FixedExtent, FixedExtent);
}

FVector ULunarFLRandom::GetRandomPointInSphere(const UObject* WorldContextObject, double Radius, ELunarRandomQuality Quality)
{
	if (Radius < 0.0)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, TEXT("Random Point In Sphere received negative radius and changed it to positive."));
		Radius = FMath::Abs(Radius);
	}

	return GetRandomUnitVector(Quality) * FMath::Pow(GetRandomDoubleInRange(WorldContextObject, 0.0, 1.0, Quality), 1.0 / 3.0) * Radius;
}

FVector ULunarFLRandom::GetRandomPointInSphereFromSeed(const UObject* WorldContextObject, double Radius, int64 Seed, ELunarRandomQuality Quality)
{
	if (Radius < 0.0)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, TEXT("Random Point In Sphere From Seed received negative radius and changed it to positive."));
		Radius = FMath::Abs(Radius);
	}

	return GetRandomUnitVectorFromSeed(Seed + 1, Quality) * FMath::Pow(GetRandomDoubleInRangeFromSeed(WorldContextObject, 0.0, 1.0, Seed + 2, Quality), 1.0 / 3.0) * Radius;
}

FVector ULunarFLRandom::GetRandomPointInSphereFromStream(const UObject* WorldContextObject, FLunarRandomStream& Stream, double Radius)
{
	if (Radius < 0.0)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, TEXT("Random Point In Sphere From Stream received negative radius and changed it to positive."));
		Radius = FMath::Abs(Radius);
	}

	return GetRandomUnitVectorFromStream(Stream) * FMath::Pow(GetRandomDoubleInRangeFromStream(WorldContextObject, Stream, 0.0, 1.0), 1.0 / 3.0) * Radius;
}

FVector ULunarFLRandom::GetRandomPointInCircle(const UObject* WorldContextObject, double Radius, ELunarRandomQuality Quality)
{
	if (Radius < 0.0)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, TEXT("Random Point In Circle received negative radius and changed it to positive."));
		Radius = FMath::Abs(Radius);
	}

	const double Angle = GetRandomDoubleInRange(WorldContextObject, 0.0, 2.0 * UE_DOUBLE_PI, Quality);
	const double Distance = FMath::Sqrt(GetRandomDoubleInRange(WorldContextObject, 0.0, 1.0, Quality)) * Radius;
	return FVector(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.0);
}

FVector ULunarFLRandom::GetRandomPointInCircleFromSeed(const UObject* WorldContextObject, double Radius, int64 Seed, ELunarRandomQuality Quality)
{
	if (Radius < 0.0)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, TEXT("Random Point In Circle From Seed received negative radius and changed it to positive."));
		Radius = FMath::Abs(Radius);
	}

	const double Angle = GetRandomDoubleInRangeFromSeed(WorldContextObject, 0.0, 2.0 * UE_DOUBLE_PI, Seed + 1, Quality);
	const double Distance = FMath::Sqrt(GetRandomDoubleInRangeFromSeed(WorldContextObject, 0.0, 1.0, Seed + 2, Quality)) * Radius;
	return FVector(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.0);
}

FVector ULunarFLRandom::GetRandomPointInCircleFromStream(const UObject* WorldContextObject, FLunarRandomStream& Stream, double Radius)
{
	if (Radius < 0.0)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, TEXT("Random Point In Circle From Stream received negative radius and changed it to positive."));
		Radius = FMath::Abs(Radius);
	}

	const double Angle = GetRandomDoubleInRangeFromStream(WorldContextObject, Stream, 0.0, 2.0 * UE_DOUBLE_PI);
	const double Distance = FMath::Sqrt(GetRandomDoubleInRangeFromStream(WorldContextObject, Stream, 0.0, 1.0)) * Radius;
	return FVector(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.0);
}

FRotator ULunarFLRandom::GetRandomRotator(ELunarRandomQuality Quality)
{
	return FRotator(LunarRandomInternal::GetRandomDoubleInRangeInternal(-180.0, 180.0, Quality), LunarRandomInternal::GetRandomDoubleInRangeInternal(-180.0, 180.0, Quality), LunarRandomInternal::GetRandomDoubleInRangeInternal(-180.0, 180.0, Quality));
}

FRotator ULunarFLRandom::GetRandomRotatorFromSeed(int64 Seed, ELunarRandomQuality Quality)
{
	return FRotator(LunarRandomInternal::GetRandomDoubleInRangeFromSeedInternal(-180.0, 180.0, Seed + 1, Quality), LunarRandomInternal::GetRandomDoubleInRangeFromSeedInternal(-180.0, 180.0, Seed + 2, Quality), LunarRandomInternal::GetRandomDoubleInRangeFromSeedInternal(-180.0, 180.0, Seed + 3, Quality));
}

FRotator ULunarFLRandom::GetRandomRotatorFromStream(FLunarRandomStream& Stream)
{
	return FRotator(GetRandomDoubleInRangeFromStream(nullptr, Stream, -180.0, 180.0), GetRandomDoubleInRangeFromStream(nullptr, Stream, -180.0, 180.0), GetRandomDoubleInRangeFromStream(nullptr, Stream, -180.0, 180.0));
}

FRotator ULunarFLRandom::GetRandomYawRotator(ELunarRandomQuality Quality)
{
	return FRotator(0.0, LunarRandomInternal::GetRandomDoubleInRangeInternal(-180.0, 180.0, Quality), 0.0);
}

FRotator ULunarFLRandom::GetRandomYawRotatorFromSeed(int64 Seed, ELunarRandomQuality Quality)
{
	return FRotator(0.0, LunarRandomInternal::GetRandomDoubleInRangeFromSeedInternal(-180.0, 180.0, Seed, Quality), 0.0);
}

FRotator ULunarFLRandom::GetRandomYawRotatorFromStream(FLunarRandomStream& Stream)
{
	return FRotator(0.0, GetRandomDoubleInRangeFromStream(nullptr, Stream, -180.0, 180.0), 0.0);
}

FLinearColor ULunarFLRandom::GetRandomLinearColor(bool bRandomAlpha, ELunarRandomQuality Quality)
{
	return FLinearColor(LunarRandomInternal::GetRandomFloatInRangeInternal(0.0f, 1.0f, Quality), LunarRandomInternal::GetRandomFloatInRangeInternal(0.0f, 1.0f, Quality), LunarRandomInternal::GetRandomFloatInRangeInternal(0.0f, 1.0f, Quality), bRandomAlpha ? LunarRandomInternal::GetRandomFloatInRangeInternal(0.0f, 1.0f, Quality) : 1.0f);
}

FLinearColor ULunarFLRandom::GetRandomLinearColorFromSeed(int64 Seed, bool bRandomAlpha, ELunarRandomQuality Quality)
{
	return FLinearColor(LunarRandomInternal::GetRandomFloatInRangeFromSeedInternal(0.0f, 1.0f, Seed + 1, Quality), LunarRandomInternal::GetRandomFloatInRangeFromSeedInternal(0.0f, 1.0f, Seed + 2, Quality), LunarRandomInternal::GetRandomFloatInRangeFromSeedInternal(0.0f, 1.0f, Seed + 3, Quality), bRandomAlpha ? LunarRandomInternal::GetRandomFloatInRangeFromSeedInternal(0.0f, 1.0f, Seed + 4, Quality) : 1.0f);
}

FLinearColor ULunarFLRandom::GetRandomLinearColorFromStream(FLunarRandomStream& Stream, bool bRandomAlpha)
{
	return FLinearColor(GetRandomFloatInRangeFromStream(nullptr, Stream, 0.0f, 1.0f), GetRandomFloatInRangeFromStream(nullptr, Stream, 0.0f, 1.0f), GetRandomFloatInRangeFromStream(nullptr, Stream, 0.0f, 1.0f), bRandomAlpha ? GetRandomFloatInRangeFromStream(nullptr, Stream, 0.0f, 1.0f) : 1.0f);
}

void ULunarFLRandom::GetRandomArrayItem(const UObject* WorldContextObject, const TArray<int32>& Array, int32& Item, bool& bSuccess, ELunarRandomQuality Quality)
{
	bSuccess = false;

	if (Array.Num() <= 0)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, TEXT("Random Array Item failed because source array is empty."));
		Item = 0;
		return;
	}

	Item = Array[LunarRandomInternal::GetRandomIntInRangeInternal(0, Array.Num() - 1, Quality)];
	bSuccess = true;
}

void ULunarFLRandom::GetRandomArrayItemFromSeed(const UObject* WorldContextObject, const TArray<int32>& Array, int32& Item, int64 Seed, bool& bSuccess, ELunarRandomQuality Quality)
{
	bSuccess = false;

	if (Array.Num() <= 0)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, TEXT("Random Array Item From Seed failed because source array is empty."));
		Item = 0;
		return;
	}

	Item = Array[LunarRandomInternal::GetRandomIntInRangeFromSeedInternal(0, Array.Num() - 1, Seed, Quality)];
	bSuccess = true;
}

void ULunarFLRandom::GetRandomArrayItemFromStream(const UObject* WorldContextObject, FLunarRandomStream& Stream, const TArray<int32>& Array, int32& Item, bool& bSuccess)
{
	bSuccess = false;

	if (Array.Num() <= 0)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, TEXT("Random Array Item From Stream failed because source array is empty."));
		Item = 0;
		return;
	}

	Item = Array[LunarRandomInternal::GetRandomIntInRangeFromSeedInternal(0, Array.Num() - 1, static_cast<int64>(LunarRandomInternal::MakeStreamSeed(Stream)), Stream.Quality)];
	++Stream.Step;
	bSuccess = true;
}

void ULunarFLRandom::GetRandomArrayItemByWeights(const UObject* WorldContextObject, const TArray<int32>& Array, const TArray<double>& Weights, int32& Item, bool& bSuccess, ELunarRandomQuality Quality)
{
	bSuccess = false;

	if (Array.Num() != Weights.Num())
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Error, TEXT("Random Array Item By Weights failed because array length and weights length are different."));
		Item = 0;
		return;
	}

	const int32 Index = GetRandomIndexByWeights(WorldContextObject, Weights, bSuccess, Quality);

	if (!bSuccess || !Array.IsValidIndex(Index))
	{
		Item = 0;
		bSuccess = false;
		return;
	}

	Item = Array[Index];
	bSuccess = true;
}

void ULunarFLRandom::GetRandomArrayItemByWeightsFromSeed(const UObject* WorldContextObject, const TArray<int32>& Array, const TArray<double>& Weights, int32& Item, int64 Seed, bool& bSuccess, ELunarRandomQuality Quality)
{
	bSuccess = false;

	if (Array.Num() != Weights.Num())
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Error, TEXT("Random Array Item By Weights From Seed failed because array length and weights length are different."));
		Item = 0;
		return;
	}

	const int32 Index = GetRandomIndexByWeightsFromSeed(WorldContextObject, Weights, Seed, bSuccess, Quality);

	if (!bSuccess || !Array.IsValidIndex(Index))
	{
		Item = 0;
		bSuccess = false;
		return;
	}

	Item = Array[Index];
	bSuccess = true;
}

void ULunarFLRandom::GetRandomArrayItemByWeightsFromStream(const UObject* WorldContextObject, FLunarRandomStream& Stream, const TArray<int32>& Array, const TArray<double>& Weights, int32& Item, bool& bSuccess)
{
	bSuccess = false;

	if (Array.Num() != Weights.Num())
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Error, TEXT("Random Array Item By Weights From Stream failed because array length and weights length are different."));
		Item = 0;
		return;
	}

	const int32 Index = GetRandomIndexByWeightsFromStream(WorldContextObject, Stream, Weights, bSuccess);

	if (!bSuccess || !Array.IsValidIndex(Index))
	{
		Item = 0;
		bSuccess = false;
		return;
	}

	Item = Array[Index];
	bSuccess = true;
}

void ULunarFLRandom::GetRandomArrayItems(const UObject* WorldContextObject, const TArray<int32>& Array, int32 Count, bool bUnique, TArray<int32>& Items, bool& bSuccess, ELunarRandomQuality Quality)
{
	LunarRandomInternal::GetRandomIntArrayItemsFromSeed(Array, Count, bUnique, Items, LunarRandomInternal::MakeRuntimeSeed(), bSuccess, Quality, TEXT("Random Array Items"));
}

void ULunarFLRandom::GetRandomArrayItemsFromSeed(const UObject* WorldContextObject, const TArray<int32>& Array, int32 Count, bool bUnique, TArray<int32>& Items, int64 Seed, bool& bSuccess, ELunarRandomQuality Quality)
{
	LunarRandomInternal::GetRandomIntArrayItemsFromSeed(Array, Count, bUnique, Items, Seed, bSuccess, Quality, TEXT("Random Array Items From Seed"));
}

void ULunarFLRandom::GetRandomArrayItemsFromStream(const UObject* WorldContextObject, FLunarRandomStream& Stream, const TArray<int32>& Array, int32 Count, bool bUnique, TArray<int32>& Items, bool& bSuccess)
{
	LunarRandomInternal::GetRandomIntArrayItemsFromSeed(Array, Count, bUnique, Items, static_cast<int64>(LunarRandomInternal::MakeStreamSeed(Stream)), bSuccess, Stream.Quality, TEXT("Random Array Items From Stream"));
	Stream.Step += FMath::Max(1, Count);
}

void ULunarFLRandom::GetRandomArrayItemsByWeights(const UObject* WorldContextObject, const TArray<int32>& Array, const TArray<double>& Weights, int32 Count, bool bUnique, TArray<int32>& Items, bool& bSuccess, ELunarRandomQuality Quality)
{
	LunarRandomInternal::GetRandomIntArrayItemsByWeightsFromSeed(Array, Weights, Count, bUnique, Items, LunarRandomInternal::MakeRuntimeSeed(), bSuccess, Quality, TEXT("Random Array Items By Weights"));
}

void ULunarFLRandom::GetRandomArrayItemsByWeightsFromSeed(const UObject* WorldContextObject, const TArray<int32>& Array, const TArray<double>& Weights, int32 Count, bool bUnique, TArray<int32>& Items, int64 Seed, bool& bSuccess, ELunarRandomQuality Quality)
{
	LunarRandomInternal::GetRandomIntArrayItemsByWeightsFromSeed(Array, Weights, Count, bUnique, Items, Seed, bSuccess, Quality, TEXT("Random Array Items By Weights From Seed"));
}

void ULunarFLRandom::GetRandomArrayItemsByWeightsFromStream(const UObject* WorldContextObject, FLunarRandomStream& Stream, const TArray<int32>& Array, const TArray<double>& Weights, int32 Count, bool bUnique, TArray<int32>& Items, bool& bSuccess)
{
	LunarRandomInternal::GetRandomIntArrayItemsByWeightsFromSeed(Array, Weights, Count, bUnique, Items, static_cast<int64>(LunarRandomInternal::MakeStreamSeed(Stream)), bSuccess, Stream.Quality, TEXT("Random Array Items By Weights From Stream"));
	Stream.Step += FMath::Max(1, Count);
}

void ULunarFLRandom::ShuffleArray(const UObject* WorldContextObject, TArray<int32>& Array, bool& bSuccess, ELunarRandomQuality Quality)
{
	bSuccess = false;

	if (Array.Num() <= 0)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, TEXT("Shuffle Array failed because array is empty."));
		return;
	}

	for (int32 Index = Array.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = LunarRandomInternal::GetRandomIntInRangeInternal(0, Index, Quality);
		Array.Swap(Index, SwapIndex);
	}

	bSuccess = true;
}

void ULunarFLRandom::ShuffleArrayFromSeed(const UObject* WorldContextObject, TArray<int32>& Array, int64 Seed, bool& bSuccess, ELunarRandomQuality Quality)
{
	bSuccess = false;

	if (Array.Num() <= 0)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Random, ELunarConsoleMessageVerbosity::Warning, TEXT("Shuffle Array From Seed failed because array is empty."));
		return;
	}

	for (int32 Index = Array.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = LunarRandomInternal::GetRandomIntInRangeFromSeedInternal(0, Index, static_cast<int64>(LunarRandomInternal::MixSeed(static_cast<uint64>(Seed) + static_cast<uint64>(Index))), Quality);
		Array.Swap(Index, SwapIndex);
	}

	bSuccess = true;
}

void ULunarFLRandom::ShuffleArrayFromStream(const UObject* WorldContextObject, FLunarRandomStream& Stream, TArray<int32>& Array, bool& bSuccess)
{
	ShuffleArrayFromSeed(WorldContextObject, Array, static_cast<int64>(LunarRandomInternal::MakeStreamSeed(Stream)), bSuccess, Stream.Quality);
	++Stream.Step;
}

DEFINE_FUNCTION(ULunarFLRandom::execGetRandomArrayItem)
{
	P_GET_OBJECT(UObject, WorldContextObject);

	Stack.StepCompiledIn<FArrayProperty>(nullptr);
	void* ArrayAddress = Stack.MostRecentPropertyAddress;
	FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);

	Stack.StepCompiledIn<FProperty>(nullptr);
	void* ItemAddress = Stack.MostRecentPropertyAddress;
	FProperty* ItemProperty = Stack.MostRecentProperty;

	P_GET_UBOOL_REF(bSuccess);
	P_GET_ENUM(ELunarRandomQuality, Quality);
	P_FINISH;

	P_NATIVE_BEGIN;
	LunarRandomInternal::GenericGetRandomArrayItem(ArrayAddress, ArrayProperty, ItemAddress, ItemProperty, bSuccess, Quality, TEXT("Random Array Item"));
	P_NATIVE_END;
}

DEFINE_FUNCTION(ULunarFLRandom::execGetRandomArrayItemFromSeed)
{
	P_GET_OBJECT(UObject, WorldContextObject);

	Stack.StepCompiledIn<FArrayProperty>(nullptr);
	void* ArrayAddress = Stack.MostRecentPropertyAddress;
	FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);

	Stack.StepCompiledIn<FProperty>(nullptr);
	void* ItemAddress = Stack.MostRecentPropertyAddress;
	FProperty* ItemProperty = Stack.MostRecentProperty;

	P_GET_PROPERTY(FInt64Property, Seed);
	P_GET_UBOOL_REF(bSuccess);
	P_GET_ENUM(ELunarRandomQuality, Quality);
	P_FINISH;

	P_NATIVE_BEGIN;
	LunarRandomInternal::GenericGetRandomArrayItemFromSeed(ArrayAddress, ArrayProperty, ItemAddress, ItemProperty, Seed, bSuccess, Quality, TEXT("Random Array Item From Seed"));
	P_NATIVE_END;
}

DEFINE_FUNCTION(ULunarFLRandom::execGetRandomArrayItemFromStream)
{
	P_GET_OBJECT(UObject, WorldContextObject);
	P_GET_STRUCT_REF(FLunarRandomStream, Stream);

	Stack.StepCompiledIn<FArrayProperty>(nullptr);
	void* ArrayAddress = Stack.MostRecentPropertyAddress;
	FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);

	Stack.StepCompiledIn<FProperty>(nullptr);
	void* ItemAddress = Stack.MostRecentPropertyAddress;
	FProperty* ItemProperty = Stack.MostRecentProperty;

	P_GET_UBOOL_REF(bSuccess);
	P_FINISH;

	P_NATIVE_BEGIN;
	LunarRandomInternal::GenericGetRandomArrayItemFromSeed(ArrayAddress, ArrayProperty, ItemAddress, ItemProperty, static_cast<int64>(LunarRandomInternal::MakeStreamSeed(Stream)), bSuccess, Stream.Quality, TEXT("Random Array Item From Stream"));
	++Stream.Step;
	P_NATIVE_END;
}

DEFINE_FUNCTION(ULunarFLRandom::execGetRandomArrayItemByWeights)
{
	P_GET_OBJECT(UObject, WorldContextObject);

	Stack.StepCompiledIn<FArrayProperty>(nullptr);
	void* ArrayAddress = Stack.MostRecentPropertyAddress;
	FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);

	P_GET_TARRAY_REF(double, Weights);

	Stack.StepCompiledIn<FProperty>(nullptr);
	void* ItemAddress = Stack.MostRecentPropertyAddress;
	FProperty* ItemProperty = Stack.MostRecentProperty;

	P_GET_UBOOL_REF(bSuccess);
	P_GET_ENUM(ELunarRandomQuality, Quality);
	P_FINISH;

	P_NATIVE_BEGIN;
	LunarRandomInternal::GenericGetRandomArrayItemByWeightsFromSeed(ArrayAddress, ArrayProperty, Weights, ItemAddress, ItemProperty, LunarRandomInternal::MakeRuntimeSeed(), bSuccess, Quality, TEXT("Random Array Item By Weights"));
	P_NATIVE_END;
}

DEFINE_FUNCTION(ULunarFLRandom::execGetRandomArrayItemByWeightsFromSeed)
{
	P_GET_OBJECT(UObject, WorldContextObject);

	Stack.StepCompiledIn<FArrayProperty>(nullptr);
	void* ArrayAddress = Stack.MostRecentPropertyAddress;
	FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);

	P_GET_TARRAY_REF(double, Weights);

	Stack.StepCompiledIn<FProperty>(nullptr);
	void* ItemAddress = Stack.MostRecentPropertyAddress;
	FProperty* ItemProperty = Stack.MostRecentProperty;

	P_GET_PROPERTY(FInt64Property, Seed);
	P_GET_UBOOL_REF(bSuccess);
	P_GET_ENUM(ELunarRandomQuality, Quality);
	P_FINISH;

	P_NATIVE_BEGIN;
	LunarRandomInternal::GenericGetRandomArrayItemByWeightsFromSeed(ArrayAddress, ArrayProperty, Weights, ItemAddress, ItemProperty, Seed, bSuccess, Quality, TEXT("Random Array Item By Weights From Seed"));
	P_NATIVE_END;
}

DEFINE_FUNCTION(ULunarFLRandom::execGetRandomArrayItemByWeightsFromStream)
{
	P_GET_OBJECT(UObject, WorldContextObject);
	P_GET_STRUCT_REF(FLunarRandomStream, Stream);

	Stack.StepCompiledIn<FArrayProperty>(nullptr);
	void* ArrayAddress = Stack.MostRecentPropertyAddress;
	FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);

	P_GET_TARRAY_REF(double, Weights);

	Stack.StepCompiledIn<FProperty>(nullptr);
	void* ItemAddress = Stack.MostRecentPropertyAddress;
	FProperty* ItemProperty = Stack.MostRecentProperty;

	P_GET_UBOOL_REF(bSuccess);
	P_FINISH;

	P_NATIVE_BEGIN;
	LunarRandomInternal::GenericGetRandomArrayItemByWeightsFromSeed(ArrayAddress, ArrayProperty, Weights, ItemAddress, ItemProperty, static_cast<int64>(LunarRandomInternal::MakeStreamSeed(Stream)), bSuccess, Stream.Quality, TEXT("Random Array Item By Weights From Stream"));
	++Stream.Step;
	P_NATIVE_END;
}

DEFINE_FUNCTION(ULunarFLRandom::execGetRandomArrayItems)
{
	P_GET_OBJECT(UObject, WorldContextObject);

	Stack.StepCompiledIn<FArrayProperty>(nullptr);
	void* ArrayAddress = Stack.MostRecentPropertyAddress;
	FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);

	P_GET_PROPERTY(FIntProperty, Count);
	P_GET_UBOOL(bUnique);

	Stack.StepCompiledIn<FArrayProperty>(nullptr);
	void* ItemsAddress = Stack.MostRecentPropertyAddress;
	FArrayProperty* ItemsProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);

	P_GET_UBOOL_REF(bSuccess);
	P_GET_ENUM(ELunarRandomQuality, Quality);
	P_FINISH;

	P_NATIVE_BEGIN;
	LunarRandomInternal::GenericGetRandomArrayItemsFromSeed(ArrayAddress, ArrayProperty, Count, bUnique, ItemsAddress, ItemsProperty, LunarRandomInternal::MakeRuntimeSeed(), bSuccess, Quality, TEXT("Random Array Items"));
	P_NATIVE_END;
}

DEFINE_FUNCTION(ULunarFLRandom::execGetRandomArrayItemsFromSeed)
{
	P_GET_OBJECT(UObject, WorldContextObject);

	Stack.StepCompiledIn<FArrayProperty>(nullptr);
	void* ArrayAddress = Stack.MostRecentPropertyAddress;
	FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);

	P_GET_PROPERTY(FIntProperty, Count);
	P_GET_UBOOL(bUnique);

	Stack.StepCompiledIn<FArrayProperty>(nullptr);
	void* ItemsAddress = Stack.MostRecentPropertyAddress;
	FArrayProperty* ItemsProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);

	P_GET_PROPERTY(FInt64Property, Seed);
	P_GET_UBOOL_REF(bSuccess);
	P_GET_ENUM(ELunarRandomQuality, Quality);
	P_FINISH;

	P_NATIVE_BEGIN;
	LunarRandomInternal::GenericGetRandomArrayItemsFromSeed(ArrayAddress, ArrayProperty, Count, bUnique, ItemsAddress, ItemsProperty, Seed, bSuccess, Quality, TEXT("Random Array Items From Seed"));
	P_NATIVE_END;
}

DEFINE_FUNCTION(ULunarFLRandom::execGetRandomArrayItemsFromStream)
{
	P_GET_OBJECT(UObject, WorldContextObject);
	P_GET_STRUCT_REF(FLunarRandomStream, Stream);

	Stack.StepCompiledIn<FArrayProperty>(nullptr);
	void* ArrayAddress = Stack.MostRecentPropertyAddress;
	FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);

	P_GET_PROPERTY(FIntProperty, Count);
	P_GET_UBOOL(bUnique);

	Stack.StepCompiledIn<FArrayProperty>(nullptr);
	void* ItemsAddress = Stack.MostRecentPropertyAddress;
	FArrayProperty* ItemsProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);

	P_GET_UBOOL_REF(bSuccess);
	P_FINISH;

	P_NATIVE_BEGIN;
	LunarRandomInternal::GenericGetRandomArrayItemsFromSeed(ArrayAddress, ArrayProperty, Count, bUnique, ItemsAddress, ItemsProperty, static_cast<int64>(LunarRandomInternal::MakeStreamSeed(Stream)), bSuccess, Stream.Quality, TEXT("Random Array Items From Stream"));
	Stream.Step += FMath::Max(1, Count);
	P_NATIVE_END;
}

DEFINE_FUNCTION(ULunarFLRandom::execGetRandomArrayItemsByWeights)
{
	P_GET_OBJECT(UObject, WorldContextObject);

	Stack.StepCompiledIn<FArrayProperty>(nullptr);
	void* ArrayAddress = Stack.MostRecentPropertyAddress;
	FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);

	P_GET_TARRAY_REF(double, Weights);
	P_GET_PROPERTY(FIntProperty, Count);
	P_GET_UBOOL(bUnique);

	Stack.StepCompiledIn<FArrayProperty>(nullptr);
	void* ItemsAddress = Stack.MostRecentPropertyAddress;
	FArrayProperty* ItemsProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);

	P_GET_UBOOL_REF(bSuccess);
	P_GET_ENUM(ELunarRandomQuality, Quality);
	P_FINISH;

	P_NATIVE_BEGIN;
	LunarRandomInternal::GenericGetRandomArrayItemsByWeightsFromSeed(ArrayAddress, ArrayProperty, Weights, Count, bUnique, ItemsAddress, ItemsProperty, LunarRandomInternal::MakeRuntimeSeed(), bSuccess, Quality, TEXT("Random Array Items By Weights"));
	P_NATIVE_END;
}

DEFINE_FUNCTION(ULunarFLRandom::execGetRandomArrayItemsByWeightsFromSeed)
{
	P_GET_OBJECT(UObject, WorldContextObject);

	Stack.StepCompiledIn<FArrayProperty>(nullptr);
	void* ArrayAddress = Stack.MostRecentPropertyAddress;
	FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);

	P_GET_TARRAY_REF(double, Weights);
	P_GET_PROPERTY(FIntProperty, Count);
	P_GET_UBOOL(bUnique);

	Stack.StepCompiledIn<FArrayProperty>(nullptr);
	void* ItemsAddress = Stack.MostRecentPropertyAddress;
	FArrayProperty* ItemsProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);

	P_GET_PROPERTY(FInt64Property, Seed);
	P_GET_UBOOL_REF(bSuccess);
	P_GET_ENUM(ELunarRandomQuality, Quality);
	P_FINISH;

	P_NATIVE_BEGIN;
	LunarRandomInternal::GenericGetRandomArrayItemsByWeightsFromSeed(ArrayAddress, ArrayProperty, Weights, Count, bUnique, ItemsAddress, ItemsProperty, Seed, bSuccess, Quality, TEXT("Random Array Items By Weights From Seed"));
	P_NATIVE_END;
}

DEFINE_FUNCTION(ULunarFLRandom::execGetRandomArrayItemsByWeightsFromStream)
{
	P_GET_OBJECT(UObject, WorldContextObject);
	P_GET_STRUCT_REF(FLunarRandomStream, Stream);

	Stack.StepCompiledIn<FArrayProperty>(nullptr);
	void* ArrayAddress = Stack.MostRecentPropertyAddress;
	FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);

	P_GET_TARRAY_REF(double, Weights);
	P_GET_PROPERTY(FIntProperty, Count);
	P_GET_UBOOL(bUnique);

	Stack.StepCompiledIn<FArrayProperty>(nullptr);
	void* ItemsAddress = Stack.MostRecentPropertyAddress;
	FArrayProperty* ItemsProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);

	P_GET_UBOOL_REF(bSuccess);
	P_FINISH;

	P_NATIVE_BEGIN;
	LunarRandomInternal::GenericGetRandomArrayItemsByWeightsFromSeed(ArrayAddress, ArrayProperty, Weights, Count, bUnique, ItemsAddress, ItemsProperty, static_cast<int64>(LunarRandomInternal::MakeStreamSeed(Stream)), bSuccess, Stream.Quality, TEXT("Random Array Items By Weights From Stream"));
	Stream.Step += FMath::Max(1, Count);
	P_NATIVE_END;
}

DEFINE_FUNCTION(ULunarFLRandom::execShuffleArray)
{
	P_GET_OBJECT(UObject, WorldContextObject);

	Stack.StepCompiledIn<FArrayProperty>(nullptr);
	void* ArrayAddress = Stack.MostRecentPropertyAddress;
	FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);

	P_GET_UBOOL_REF(bSuccess);
	P_GET_ENUM(ELunarRandomQuality, Quality);
	P_FINISH;

	P_NATIVE_BEGIN;
	LunarRandomInternal::GenericShuffleArrayFromSeed(ArrayAddress, ArrayProperty, LunarRandomInternal::MakeRuntimeSeed(), bSuccess, Quality, TEXT("Shuffle Array"));
	P_NATIVE_END;
}

DEFINE_FUNCTION(ULunarFLRandom::execShuffleArrayFromSeed)
{
	P_GET_OBJECT(UObject, WorldContextObject);

	Stack.StepCompiledIn<FArrayProperty>(nullptr);
	void* ArrayAddress = Stack.MostRecentPropertyAddress;
	FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);

	P_GET_PROPERTY(FInt64Property, Seed);
	P_GET_UBOOL_REF(bSuccess);
	P_GET_ENUM(ELunarRandomQuality, Quality);
	P_FINISH;

	P_NATIVE_BEGIN;
	LunarRandomInternal::GenericShuffleArrayFromSeed(ArrayAddress, ArrayProperty, Seed, bSuccess, Quality, TEXT("Shuffle Array From Seed"));
	P_NATIVE_END;
}

DEFINE_FUNCTION(ULunarFLRandom::execShuffleArrayFromStream)
{
	P_GET_OBJECT(UObject, WorldContextObject);
	P_GET_STRUCT_REF(FLunarRandomStream, Stream);

	Stack.StepCompiledIn<FArrayProperty>(nullptr);
	void* ArrayAddress = Stack.MostRecentPropertyAddress;
	FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);

	P_GET_UBOOL_REF(bSuccess);
	P_FINISH;

	P_NATIVE_BEGIN;
	LunarRandomInternal::GenericShuffleArrayFromSeed(ArrayAddress, ArrayProperty, static_cast<int64>(LunarRandomInternal::MakeStreamSeed(Stream)), bSuccess, Stream.Quality, TEXT("Shuffle Array From Stream"));
	++Stream.Step;
	P_NATIVE_END;
}
