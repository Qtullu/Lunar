// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/LunarTypesRandom.h"
#include "LunarFLRandom.generated.h"

/**
 * @brief Provides random utility functions for Blueprint and C++
 * @ingroup LunarFunctionLibraries
 */
UCLASS()
class LUNAR_API ULunarFLRandom : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * @brief Creates Lunar random stream from seed and quality
	 * @param Seed Initial stream seed
	 * @param Quality Random generator quality
	 * @return New Lunar random stream
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Stream", meta = (DisplayName = "Make Random Stream"))
	static FLunarRandomStream MakeRandomStream(int64 Seed, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Resets Lunar random stream step to zero
	 * @param Stream Random stream to reset
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Random|Stream", meta = (DisplayName = "Reset Random Stream"))
	static void ResetRandomStream(UPARAM(ref) FLunarRandomStream& Stream);

	/**
	 * @brief Advances Lunar random stream by step count
	 * @param Stream Random stream to advance
	 * @param StepCount Number of steps to advance
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Random|Stream", meta = (DisplayName = "Advance Random Stream"))
	static void AdvanceRandomStream(UPARAM(ref) FLunarRandomStream& Stream, int64 StepCount = 1);

	/**
	 * @brief Gets random quality description
	 * @param Quality Random generator quality
	 * @return Random quality description
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Quality", meta = (DisplayName = "Get Random Quality Description"))
	static FString GetRandomQualityDescription(ELunarRandomQuality Quality);

	/**
	 * @brief Gets approximate random quality slowdown
	 * @param Quality Random generator quality
	 * @return Approximate slowdown text
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Quality", meta = (DisplayName = "Get Random Quality Slowdown"))
	static FString GetRandomQualitySlowdown(ELunarRandomQuality Quality);

public:
	/**
	 * @brief Gets random bool value
	 * @param Quality Random generator quality
	 * @return Random bool value
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Bool", meta = (DisplayName = "Random Bool"))
	static bool GetRandomBool(ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random bool value from seed
	 * @param Seed Random seed
	 * @param Quality Random generator quality
	 * @return Random bool value
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Bool", meta = (DisplayName = "Random Bool From Seed"))
	static bool GetRandomBoolFromSeed(int64 Seed, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random bool value from stream
	 * @param Stream Random stream
	 * @return Random bool value
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Random|Bool", meta = (DisplayName = "Random Bool From Stream"))
	static bool GetRandomBoolFromStream(UPARAM(ref) FLunarRandomStream& Stream);

	/**
	 * @brief Gets random bool value using normalized chance
	 * @param WorldContextObject World context object
	 * @param Chance Normalized chance from zero to one
	 * @param Quality Random generator quality
	 * @return Random bool value
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Bool", meta = (DisplayName = "Random Bool With Chance", WorldContext = "WorldContextObject"))
	static bool GetRandomBoolWithChance(const UObject* WorldContextObject, double Chance, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random bool value using normalized chance from seed
	 * @param WorldContextObject World context object
	 * @param Chance Normalized chance from zero to one
	 * @param Seed Random seed
	 * @param Quality Random generator quality
	 * @return Random bool value
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Bool", meta = (DisplayName = "Random Bool With Chance From Seed", WorldContext = "WorldContextObject"))
	static bool GetRandomBoolWithChanceFromSeed(const UObject* WorldContextObject, double Chance, int64 Seed, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random bool value using normalized chance from stream
	 * @param WorldContextObject World context object
	 * @param Stream Random stream
	 * @param Chance Normalized chance from zero to one
	 * @return Random bool value
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Random|Bool", meta = (DisplayName = "Random Bool With Chance From Stream", WorldContext = "WorldContextObject"))
	static bool GetRandomBoolWithChanceFromStream(const UObject* WorldContextObject, UPARAM(ref) FLunarRandomStream& Stream, double Chance);

	/**
	 * @brief Gets random bool value by weight and total weight
	 * @param WorldContextObject World context object
	 * @param Weight Selected weight
	 * @param TotalWeight Total weight
	 * @param Quality Random generator quality
	 * @return Random bool value
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Bool", meta = (DisplayName = "Random Bool By Weight", WorldContext = "WorldContextObject"))
	static bool GetRandomBoolByWeight(const UObject* WorldContextObject, double Weight, double TotalWeight, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random bool value by weight and total weight from seed
	 * @param WorldContextObject World context object
	 * @param Weight Selected weight
	 * @param TotalWeight Total weight
	 * @param Seed Random seed
	 * @param Quality Random generator quality
	 * @return Random bool value
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Bool", meta = (DisplayName = "Random Bool By Weight From Seed", WorldContext = "WorldContextObject"))
	static bool GetRandomBoolByWeightFromSeed(const UObject* WorldContextObject, double Weight, double TotalWeight, int64 Seed, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random bool value by weight and total weight from stream
	 * @param WorldContextObject World context object
	 * @param Stream Random stream
	 * @param Weight Selected weight
	 * @param TotalWeight Total weight
	 * @return Random bool value
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Random|Bool", meta = (DisplayName = "Random Bool By Weight From Stream", WorldContext = "WorldContextObject"))
	static bool GetRandomBoolByWeightFromStream(const UObject* WorldContextObject, UPARAM(ref) FLunarRandomStream& Stream, double Weight, double TotalWeight);

public:
	/**
	 * @brief Gets random integer in inclusive range
	 * @param WorldContextObject World context object
	 * @param Min Minimum value
	 * @param Max Maximum value
	 * @param Quality Random generator quality
	 * @return Random integer value
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Number", meta = (DisplayName = "Random Integer In Range", WorldContext = "WorldContextObject"))
	static int32 GetRandomIntegerInRange(const UObject* WorldContextObject, int32 Min, int32 Max, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random integer in inclusive range from seed
	 * @param WorldContextObject World context object
	 * @param Min Minimum value
	 * @param Max Maximum value
	 * @param Seed Random seed
	 * @param Quality Random generator quality
	 * @return Random integer value
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Number", meta = (DisplayName = "Random Integer In Range From Seed", WorldContext = "WorldContextObject"))
	static int32 GetRandomIntegerInRangeFromSeed(const UObject* WorldContextObject, int32 Min, int32 Max, int64 Seed, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random integer in inclusive range from stream
	 * @param WorldContextObject World context object
	 * @param Stream Random stream
	 * @param Min Minimum value
	 * @param Max Maximum value
	 * @return Random integer value
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Random|Number", meta = (DisplayName = "Random Integer In Range From Stream", WorldContext = "WorldContextObject"))
	static int32 GetRandomIntegerInRangeFromStream(const UObject* WorldContextObject, UPARAM(ref) FLunarRandomStream& Stream, int32 Min, int32 Max);

	/**
	 * @brief Gets random int64 in inclusive range
	 * @param WorldContextObject World context object
	 * @param Min Minimum value
	 * @param Max Maximum value
	 * @param Quality Random generator quality
	 * @return Random int64 value
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Number", meta = (DisplayName = "Random Integer 64 In Range", WorldContext = "WorldContextObject"))
	static int64 GetRandomInteger64InRange(const UObject* WorldContextObject, int64 Min, int64 Max, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random int64 in inclusive range from seed
	 * @param WorldContextObject World context object
	 * @param Min Minimum value
	 * @param Max Maximum value
	 * @param Seed Random seed
	 * @param Quality Random generator quality
	 * @return Random int64 value
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Number", meta = (DisplayName = "Random Integer 64 In Range From Seed", WorldContext = "WorldContextObject"))
	static int64 GetRandomInteger64InRangeFromSeed(const UObject* WorldContextObject, int64 Min, int64 Max, int64 Seed, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random int64 in inclusive range from stream
	 * @param WorldContextObject World context object
	 * @param Stream Random stream
	 * @param Min Minimum value
	 * @param Max Maximum value
	 * @return Random int64 value
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Random|Number", meta = (DisplayName = "Random Integer 64 In Range From Stream", WorldContext = "WorldContextObject"))
	static int64 GetRandomInteger64InRangeFromStream(const UObject* WorldContextObject, UPARAM(ref) FLunarRandomStream& Stream, int64 Min, int64 Max);

	/**
	 * @brief Gets random float in range
	 * @param WorldContextObject World context object
	 * @param Min Minimum value
	 * @param Max Maximum value
	 * @param Quality Random generator quality
	 * @return Random float value
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Number", meta = (DisplayName = "Random Float In Range", WorldContext = "WorldContextObject"))
	static float GetRandomFloatInRange(const UObject* WorldContextObject, float Min, float Max, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random float in range from seed
	 * @param WorldContextObject World context object
	 * @param Min Minimum value
	 * @param Max Maximum value
	 * @param Seed Random seed
	 * @param Quality Random generator quality
	 * @return Random float value
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Number", meta = (DisplayName = "Random Float In Range From Seed", WorldContext = "WorldContextObject"))
	static float GetRandomFloatInRangeFromSeed(const UObject* WorldContextObject, float Min, float Max, int64 Seed, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random float in range from stream
	 * @param WorldContextObject World context object
	 * @param Stream Random stream
	 * @param Min Minimum value
	 * @param Max Maximum value
	 * @return Random float value
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Random|Number", meta = (DisplayName = "Random Float In Range From Stream", WorldContext = "WorldContextObject"))
	static float GetRandomFloatInRangeFromStream(const UObject* WorldContextObject, UPARAM(ref) FLunarRandomStream& Stream, float Min, float Max);

	/**
	 * @brief Gets random double in range
	 * @param WorldContextObject World context object
	 * @param Min Minimum value
	 * @param Max Maximum value
	 * @param Quality Random generator quality
	 * @return Random double value
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Number", meta = (DisplayName = "Random Double In Range", WorldContext = "WorldContextObject"))
	static double GetRandomDoubleInRange(const UObject* WorldContextObject, double Min, double Max, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random double in range from seed
	 * @param WorldContextObject World context object
	 * @param Min Minimum value
	 * @param Max Maximum value
	 * @param Seed Random seed
	 * @param Quality Random generator quality
	 * @return Random double value
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Number", meta = (DisplayName = "Random Double In Range From Seed", WorldContext = "WorldContextObject"))
	static double GetRandomDoubleInRangeFromSeed(const UObject* WorldContextObject, double Min, double Max, int64 Seed, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random double in range from stream
	 * @param WorldContextObject World context object
	 * @param Stream Random stream
	 * @param Min Minimum value
	 * @param Max Maximum value
	 * @return Random double value
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Random|Number", meta = (DisplayName = "Random Double In Range From Stream", WorldContext = "WorldContextObject"))
	static double GetRandomDoubleInRangeFromStream(const UObject* WorldContextObject, UPARAM(ref) FLunarRandomStream& Stream, double Min, double Max);

public:
	/**
	 * @brief Gets random index for item count
	 * @param WorldContextObject World context object
	 * @param Count Item count
	 * @param bSuccess True if index was generated
	 * @param Quality Random generator quality
	 * @return Random index
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Index", meta = (DisplayName = "Random Index", WorldContext = "WorldContextObject"))
	static int32 GetRandomIndex(const UObject* WorldContextObject, int32 Count, bool& bSuccess, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random index for item count from seed
	 * @param WorldContextObject World context object
	 * @param Count Item count
	 * @param Seed Random seed
	 * @param bSuccess True if index was generated
	 * @param Quality Random generator quality
	 * @return Random index
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Index", meta = (DisplayName = "Random Index From Seed", WorldContext = "WorldContextObject"))
	static int32 GetRandomIndexFromSeed(const UObject* WorldContextObject, int32 Count, int64 Seed, bool& bSuccess, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random index for item count from stream
	 * @param WorldContextObject World context object
	 * @param Stream Random stream
	 * @param Count Item count
	 * @param bSuccess True if index was generated
	 * @return Random index
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Random|Index", meta = (DisplayName = "Random Index From Stream", WorldContext = "WorldContextObject"))
	static int32 GetRandomIndexFromStream(const UObject* WorldContextObject, UPARAM(ref) FLunarRandomStream& Stream, int32 Count, bool& bSuccess);

	/**
	 * @brief Gets random index by weights
	 * @param WorldContextObject World context object
	 * @param Weights Weight list
	 * @param bSuccess True if index was generated
	 * @param Quality Random generator quality
	 * @return Random index
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Index", meta = (DisplayName = "Random Index By Weights", WorldContext = "WorldContextObject", AutoCreateRefTerm = "Weights"))
	static int32 GetRandomIndexByWeights(const UObject* WorldContextObject, const TArray<double>& Weights, bool& bSuccess, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random index by weights from seed
	 * @param WorldContextObject World context object
	 * @param Weights Weight list
	 * @param Seed Random seed
	 * @param bSuccess True if index was generated
	 * @param Quality Random generator quality
	 * @return Random index
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Index", meta = (DisplayName = "Random Index By Weights From Seed", WorldContext = "WorldContextObject", AutoCreateRefTerm = "Weights"))
	static int32 GetRandomIndexByWeightsFromSeed(const UObject* WorldContextObject, const TArray<double>& Weights, int64 Seed, bool& bSuccess, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random index by weights from stream
	 * @param WorldContextObject World context object
	 * @param Stream Random stream
	 * @param Weights Weight list
	 * @param bSuccess True if index was generated
	 * @return Random index
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Random|Index", meta = (DisplayName = "Random Index By Weights From Stream", WorldContext = "WorldContextObject", AutoCreateRefTerm = "Weights"))
	static int32 GetRandomIndexByWeightsFromStream(const UObject* WorldContextObject, UPARAM(ref) FLunarRandomStream& Stream, const TArray<double>& Weights, bool& bSuccess);

public:
	/**
	 * @brief Gets random sign
	 * @param Quality Random generator quality
	 * @return Random sign as minus one or one
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Number", meta = (DisplayName = "Random Sign"))
	static int32 GetRandomSign(ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random sign from seed
	 * @param Seed Random seed
	 * @param Quality Random generator quality
	 * @return Random sign as minus one or one
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Number", meta = (DisplayName = "Random Sign From Seed"))
	static int32 GetRandomSignFromSeed(int64 Seed, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random sign from stream
	 * @param Stream Random stream
	 * @return Random sign as minus one or one
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Random|Number", meta = (DisplayName = "Random Sign From Stream"))
	static int32 GetRandomSignFromStream(UPARAM(ref) FLunarRandomStream& Stream);

public:
	/**
	 * @brief Gets random vector in component range
	 * @param WorldContextObject World context object
	 * @param Min Minimum vector
	 * @param Max Maximum vector
	 * @param Quality Random generator quality
	 * @return Random vector
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Vector", meta = (DisplayName = "Random Vector In Range", WorldContext = "WorldContextObject"))
	static FVector GetRandomVectorInRange(const UObject* WorldContextObject, FVector Min, FVector Max, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random vector in component range from seed
	 * @param WorldContextObject World context object
	 * @param Min Minimum vector
	 * @param Max Maximum vector
	 * @param Seed Random seed
	 * @param Quality Random generator quality
	 * @return Random vector
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Vector", meta = (DisplayName = "Random Vector In Range From Seed", WorldContext = "WorldContextObject"))
	static FVector GetRandomVectorInRangeFromSeed(const UObject* WorldContextObject, FVector Min, FVector Max, int64 Seed, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random vector in component range from stream
	 * @param WorldContextObject World context object
	 * @param Stream Random stream
	 * @param Min Minimum vector
	 * @param Max Maximum vector
	 * @return Random vector
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Random|Vector", meta = (DisplayName = "Random Vector In Range From Stream", WorldContext = "WorldContextObject"))
	static FVector GetRandomVectorInRangeFromStream(const UObject* WorldContextObject, UPARAM(ref) FLunarRandomStream& Stream, FVector Min, FVector Max);

	/**
	 * @brief Gets random unit vector
	 * @param Quality Random generator quality
	 * @return Random unit vector
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Vector", meta = (DisplayName = "Random Unit Vector"))
	static FVector GetRandomUnitVector(ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random unit vector from seed
	 * @param Seed Random seed
	 * @param Quality Random generator quality
	 * @return Random unit vector
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Vector", meta = (DisplayName = "Random Unit Vector From Seed"))
	static FVector GetRandomUnitVectorFromSeed(int64 Seed, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random unit vector from stream
	 * @param Stream Random stream
	 * @return Random unit vector
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Random|Vector", meta = (DisplayName = "Random Unit Vector From Stream"))
	static FVector GetRandomUnitVectorFromStream(UPARAM(ref) FLunarRandomStream& Stream);

	/**
	 * @brief Gets random point in box extent
	 * @param WorldContextObject World context object
	 * @param BoxExtent Box extent
	 * @param Quality Random generator quality
	 * @return Random point
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Vector", meta = (DisplayName = "Random Point In Box", WorldContext = "WorldContextObject"))
	static FVector GetRandomPointInBox(const UObject* WorldContextObject, FVector BoxExtent, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random point in box extent from seed
	 * @param WorldContextObject World context object
	 * @param BoxExtent Box extent
	 * @param Seed Random seed
	 * @param Quality Random generator quality
	 * @return Random point
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Vector", meta = (DisplayName = "Random Point In Box From Seed", WorldContext = "WorldContextObject"))
	static FVector GetRandomPointInBoxFromSeed(const UObject* WorldContextObject, FVector BoxExtent, int64 Seed, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random point in box extent from stream
	 * @param WorldContextObject World context object
	 * @param Stream Random stream
	 * @param BoxExtent Box extent
	 * @return Random point
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Random|Vector", meta = (DisplayName = "Random Point In Box From Stream", WorldContext = "WorldContextObject"))
	static FVector GetRandomPointInBoxFromStream(const UObject* WorldContextObject, UPARAM(ref) FLunarRandomStream& Stream, FVector BoxExtent);

	/**
	 * @brief Gets random point in sphere
	 * @param WorldContextObject World context object
	 * @param Radius Sphere radius
	 * @param Quality Random generator quality
	 * @return Random point
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Vector", meta = (DisplayName = "Random Point In Sphere", WorldContext = "WorldContextObject"))
	static FVector GetRandomPointInSphere(const UObject* WorldContextObject, double Radius, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random point in sphere from seed
	 * @param WorldContextObject World context object
	 * @param Radius Sphere radius
	 * @param Seed Random seed
	 * @param Quality Random generator quality
	 * @return Random point
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Vector", meta = (DisplayName = "Random Point In Sphere From Seed", WorldContext = "WorldContextObject"))
	static FVector GetRandomPointInSphereFromSeed(const UObject* WorldContextObject, double Radius, int64 Seed, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random point in sphere from stream
	 * @param WorldContextObject World context object
	 * @param Stream Random stream
	 * @param Radius Sphere radius
	 * @return Random point
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Random|Vector", meta = (DisplayName = "Random Point In Sphere From Stream", WorldContext = "WorldContextObject"))
	static FVector GetRandomPointInSphereFromStream(const UObject* WorldContextObject, UPARAM(ref) FLunarRandomStream& Stream, double Radius);

	/**
	 * @brief Gets random point in circle on XY plane
	 * @param WorldContextObject World context object
	 * @param Radius Circle radius
	 * @param Quality Random generator quality
	 * @return Random point
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Vector", meta = (DisplayName = "Random Point In Circle", WorldContext = "WorldContextObject"))
	static FVector GetRandomPointInCircle(const UObject* WorldContextObject, double Radius, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random point in circle on XY plane from seed
	 * @param WorldContextObject World context object
	 * @param Radius Circle radius
	 * @param Seed Random seed
	 * @param Quality Random generator quality
	 * @return Random point
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Vector", meta = (DisplayName = "Random Point In Circle From Seed", WorldContext = "WorldContextObject"))
	static FVector GetRandomPointInCircleFromSeed(const UObject* WorldContextObject, double Radius, int64 Seed, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random point in circle on XY plane from stream
	 * @param WorldContextObject World context object
	 * @param Stream Random stream
	 * @param Radius Circle radius
	 * @return Random point
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Random|Vector", meta = (DisplayName = "Random Point In Circle From Stream", WorldContext = "WorldContextObject"))
	static FVector GetRandomPointInCircleFromStream(const UObject* WorldContextObject, UPARAM(ref) FLunarRandomStream& Stream, double Radius);

public:
	/**
	 * @brief Gets random rotator
	 * @param Quality Random generator quality
	 * @return Random rotator
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Rotator", meta = (DisplayName = "Random Rotator"))
	static FRotator GetRandomRotator(ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random rotator from seed
	 * @param Seed Random seed
	 * @param Quality Random generator quality
	 * @return Random rotator
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Rotator", meta = (DisplayName = "Random Rotator From Seed"))
	static FRotator GetRandomRotatorFromSeed(int64 Seed, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random rotator from stream
	 * @param Stream Random stream
	 * @return Random rotator
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Random|Rotator", meta = (DisplayName = "Random Rotator From Stream"))
	static FRotator GetRandomRotatorFromStream(UPARAM(ref) FLunarRandomStream& Stream);

	/**
	 * @brief Gets random yaw rotator
	 * @param Quality Random generator quality
	 * @return Random yaw rotator
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Rotator", meta = (DisplayName = "Random Yaw Rotator"))
	static FRotator GetRandomYawRotator(ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random yaw rotator from seed
	 * @param Seed Random seed
	 * @param Quality Random generator quality
	 * @return Random yaw rotator
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Rotator", meta = (DisplayName = "Random Yaw Rotator From Seed"))
	static FRotator GetRandomYawRotatorFromSeed(int64 Seed, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random yaw rotator from stream
	 * @param Stream Random stream
	 * @return Random yaw rotator
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Random|Rotator", meta = (DisplayName = "Random Yaw Rotator From Stream"))
	static FRotator GetRandomYawRotatorFromStream(UPARAM(ref) FLunarRandomStream& Stream);

public:
	/**
	 * @brief Gets random linear color
	 * @param bRandomAlpha True if alpha should be random
	 * @param Quality Random generator quality
	 * @return Random linear color
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Color", meta = (DisplayName = "Random Linear Color"))
	static FLinearColor GetRandomLinearColor(bool bRandomAlpha = false, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random linear color from seed
	 * @param Seed Random seed
	 * @param bRandomAlpha True if alpha should be random
	 * @param Quality Random generator quality
	 * @return Random linear color
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Random|Color", meta = (DisplayName = "Random Linear Color From Seed"))
	static FLinearColor GetRandomLinearColorFromSeed(int64 Seed, bool bRandomAlpha = false, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random linear color from stream
	 * @param Stream Random stream
	 * @param bRandomAlpha True if alpha should be random
	 * @return Random linear color
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Random|Color", meta = (DisplayName = "Random Linear Color From Stream"))
	static FLinearColor GetRandomLinearColorFromStream(UPARAM(ref) FLunarRandomStream& Stream, bool bRandomAlpha = false);

public:
	/**
	 * @brief Gets random array item
	 * @param WorldContextObject World context object
	 * @param Array Source array
	 * @param Item Random item
	 * @param bSuccess True if item was generated
	 * @param Quality Random generator quality
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Lunar|Random|Array", meta = (DisplayName = "Random Array Item", WorldContext = "WorldContextObject", ArrayParm = "Array", ArrayTypeDependentParams = "Item"))
	static void GetRandomArrayItem(const UObject* WorldContextObject, const TArray<int32>& Array, int32& Item, bool& bSuccess, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random array item from seed
	 * @param WorldContextObject World context object
	 * @param Array Source array
	 * @param Item Random item
	 * @param Seed Random seed
	 * @param bSuccess True if item was generated
	 * @param Quality Random generator quality
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Lunar|Random|Array", meta = (DisplayName = "Random Array Item From Seed", WorldContext = "WorldContextObject", ArrayParm = "Array", ArrayTypeDependentParams = "Item"))
	static void GetRandomArrayItemFromSeed(const UObject* WorldContextObject, const TArray<int32>& Array, int32& Item, int64 Seed, bool& bSuccess, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random array item from stream
	 * @param WorldContextObject World context object
	 * @param Stream Random stream
	 * @param Array Source array
	 * @param Item Random item
	 * @param bSuccess True if item was generated
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Lunar|Random|Array", meta = (DisplayName = "Random Array Item From Stream", WorldContext = "WorldContextObject", ArrayParm = "Array", ArrayTypeDependentParams = "Item"))
	static void GetRandomArrayItemFromStream(const UObject* WorldContextObject, UPARAM(ref) FLunarRandomStream& Stream, const TArray<int32>& Array, int32& Item, bool& bSuccess);

	/**
	 * @brief Gets random array item by weights
	 * @param WorldContextObject World context object
	 * @param Array Source array
	 * @param Weights Weight list
	 * @param Item Random item
	 * @param bSuccess True if item was generated
	 * @param Quality Random generator quality
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Lunar|Random|Array", meta = (DisplayName = "Random Array Item By Weights", WorldContext = "WorldContextObject", ArrayParm = "Array", ArrayTypeDependentParams = "Item", AutoCreateRefTerm = "Weights"))
	static void GetRandomArrayItemByWeights(const UObject* WorldContextObject, const TArray<int32>& Array, const TArray<double>& Weights, int32& Item, bool& bSuccess, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random array item by weights from seed
	 * @param WorldContextObject World context object
	 * @param Array Source array
	 * @param Weights Weight list
	 * @param Item Random item
	 * @param Seed Random seed
	 * @param bSuccess True if item was generated
	 * @param Quality Random generator quality
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Lunar|Random|Array", meta = (DisplayName = "Random Array Item By Weights From Seed", WorldContext = "WorldContextObject", ArrayParm = "Array", ArrayTypeDependentParams = "Item", AutoCreateRefTerm = "Weights"))
	static void GetRandomArrayItemByWeightsFromSeed(const UObject* WorldContextObject, const TArray<int32>& Array, const TArray<double>& Weights, int32& Item, int64 Seed, bool& bSuccess, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random array item by weights from stream
	 * @param WorldContextObject World context object
	 * @param Stream Random stream
	 * @param Array Source array
	 * @param Weights Weight list
	 * @param Item Random item
	 * @param bSuccess True if item was generated
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Lunar|Random|Array", meta = (DisplayName = "Random Array Item By Weights From Stream", WorldContext = "WorldContextObject", ArrayParm = "Array", ArrayTypeDependentParams = "Item", AutoCreateRefTerm = "Weights"))
	static void GetRandomArrayItemByWeightsFromStream(const UObject* WorldContextObject, UPARAM(ref) FLunarRandomStream& Stream, const TArray<int32>& Array, const TArray<double>& Weights, int32& Item, bool& bSuccess);

	/**
	 * @brief Gets random array items
	 * @param WorldContextObject World context object
	 * @param Array Source array
	 * @param Count Item count
	 * @param bUnique True if items should be unique
	 * @param Items Random items
	 * @param bSuccess True if items were generated
	 * @param Quality Random generator quality
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Lunar|Random|Array", meta = (DisplayName = "Random Array Items", WorldContext = "WorldContextObject", ArrayParm = "Array,Items", ArrayTypeDependentParams = "Items"))
	static void GetRandomArrayItems(const UObject* WorldContextObject, const TArray<int32>& Array, int32 Count, bool bUnique, TArray<int32>& Items, bool& bSuccess, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random array items from seed
	 * @param WorldContextObject World context object
	 * @param Array Source array
	 * @param Count Item count
	 * @param bUnique True if items should be unique
	 * @param Items Random items
	 * @param Seed Random seed
	 * @param bSuccess True if items were generated
	 * @param Quality Random generator quality
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Lunar|Random|Array", meta = (DisplayName = "Random Array Items From Seed", WorldContext = "WorldContextObject", ArrayParm = "Array,Items", ArrayTypeDependentParams = "Items"))
	static void GetRandomArrayItemsFromSeed(const UObject* WorldContextObject, const TArray<int32>& Array, int32 Count, bool bUnique, TArray<int32>& Items, int64 Seed, bool& bSuccess, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random array items from stream
	 * @param WorldContextObject World context object
	 * @param Stream Random stream
	 * @param Array Source array
	 * @param Count Item count
	 * @param bUnique True if items should be unique
	 * @param Items Random items
	 * @param bSuccess True if items were generated
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Lunar|Random|Array", meta = (DisplayName = "Random Array Items From Stream", WorldContext = "WorldContextObject", ArrayParm = "Array,Items", ArrayTypeDependentParams = "Items"))
	static void GetRandomArrayItemsFromStream(const UObject* WorldContextObject, UPARAM(ref) FLunarRandomStream& Stream, const TArray<int32>& Array, int32 Count, bool bUnique, TArray<int32>& Items, bool& bSuccess);

	/**
	 * @brief Gets random array items by weights
	 * @param WorldContextObject World context object
	 * @param Array Source array
	 * @param Weights Weight list
	 * @param Count Item count
	 * @param bUnique True if items should be unique
	 * @param Items Random items
	 * @param bSuccess True if items were generated
	 * @param Quality Random generator quality
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Lunar|Random|Array", meta = (DisplayName = "Random Array Items By Weights", WorldContext = "WorldContextObject", ArrayParm = "Array,Items", ArrayTypeDependentParams = "Items", AutoCreateRefTerm = "Weights"))
	static void GetRandomArrayItemsByWeights(const UObject* WorldContextObject, const TArray<int32>& Array, const TArray<double>& Weights, int32 Count, bool bUnique, TArray<int32>& Items, bool& bSuccess, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random array items by weights from seed
	 * @param WorldContextObject World context object
	 * @param Array Source array
	 * @param Weights Weight list
	 * @param Count Item count
	 * @param bUnique True if items should be unique
	 * @param Items Random items
	 * @param Seed Random seed
	 * @param bSuccess True if items were generated
	 * @param Quality Random generator quality
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Lunar|Random|Array", meta = (DisplayName = "Random Array Items By Weights From Seed", WorldContext = "WorldContextObject", ArrayParm = "Array,Items", ArrayTypeDependentParams = "Items", AutoCreateRefTerm = "Weights"))
	static void GetRandomArrayItemsByWeightsFromSeed(const UObject* WorldContextObject, const TArray<int32>& Array, const TArray<double>& Weights, int32 Count, bool bUnique, TArray<int32>& Items, int64 Seed, bool& bSuccess, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Gets random array items by weights from stream
	 * @param WorldContextObject World context object
	 * @param Stream Random stream
	 * @param Array Source array
	 * @param Weights Weight list
	 * @param Count Item count
	 * @param bUnique True if items should be unique
	 * @param Items Random items
	 * @param bSuccess True if items were generated
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Lunar|Random|Array", meta = (DisplayName = "Random Array Items By Weights From Stream", WorldContext = "WorldContextObject", ArrayParm = "Array,Items", ArrayTypeDependentParams = "Items", AutoCreateRefTerm = "Weights"))
	static void GetRandomArrayItemsByWeightsFromStream(const UObject* WorldContextObject, UPARAM(ref) FLunarRandomStream& Stream, const TArray<int32>& Array, const TArray<double>& Weights, int32 Count, bool bUnique, TArray<int32>& Items, bool& bSuccess);

	/**
	 * @brief Shuffles array
	 * @param WorldContextObject World context object
	 * @param Array Array to shuffle
	 * @param bSuccess True if array was shuffled
	 * @param Quality Random generator quality
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Lunar|Random|Array", meta = (DisplayName = "Shuffle Array", WorldContext = "WorldContextObject", ArrayParm = "Array"))
	static void ShuffleArray(const UObject* WorldContextObject, UPARAM(ref) TArray<int32>& Array, bool& bSuccess, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Shuffles array from seed
	 * @param WorldContextObject World context object
	 * @param Array Array to shuffle
	 * @param Seed Random seed
	 * @param bSuccess True if array was shuffled
	 * @param Quality Random generator quality
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Lunar|Random|Array", meta = (DisplayName = "Shuffle Array From Seed", WorldContext = "WorldContextObject", ArrayParm = "Array"))
	static void ShuffleArrayFromSeed(const UObject* WorldContextObject, UPARAM(ref) TArray<int32>& Array, int64 Seed, bool& bSuccess, ELunarRandomQuality Quality = ELunarRandomQuality::Normal);

	/**
	 * @brief Shuffles array from stream
	 * @param WorldContextObject World context object
	 * @param Stream Random stream
	 * @param Array Array to shuffle
	 * @param bSuccess True if array was shuffled
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Lunar|Random|Array", meta = (DisplayName = "Shuffle Array From Stream", WorldContext = "WorldContextObject", ArrayParm = "Array"))
	static void ShuffleArrayFromStream(const UObject* WorldContextObject, UPARAM(ref) FLunarRandomStream& Stream, UPARAM(ref) TArray<int32>& Array, bool& bSuccess);

	DECLARE_FUNCTION(execGetRandomArrayItem);
	DECLARE_FUNCTION(execGetRandomArrayItemFromSeed);
	DECLARE_FUNCTION(execGetRandomArrayItemFromStream);
	DECLARE_FUNCTION(execGetRandomArrayItemByWeights);
	DECLARE_FUNCTION(execGetRandomArrayItemByWeightsFromSeed);
	DECLARE_FUNCTION(execGetRandomArrayItemByWeightsFromStream);
	DECLARE_FUNCTION(execGetRandomArrayItems);
	DECLARE_FUNCTION(execGetRandomArrayItemsFromSeed);
	DECLARE_FUNCTION(execGetRandomArrayItemsFromStream);
	DECLARE_FUNCTION(execGetRandomArrayItemsByWeights);
	DECLARE_FUNCTION(execGetRandomArrayItemsByWeightsFromSeed);
	DECLARE_FUNCTION(execGetRandomArrayItemsByWeightsFromStream);
	DECLARE_FUNCTION(execShuffleArray);
	DECLARE_FUNCTION(execShuffleArrayFromSeed);
	DECLARE_FUNCTION(execShuffleArrayFromStream);
};