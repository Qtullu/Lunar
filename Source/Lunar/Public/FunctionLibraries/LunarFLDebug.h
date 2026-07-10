// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LunarFLDebug.generated.h"

/**
 * @file LunarFLDebug.h
 * @brief Debug helper function library
 */

 /**
  * @brief Blueprint utility functions for advanced debugging
  * @ingroup LunarFLDebug
  */
UCLASS()
class LUNAR_API ULunarFLDebug : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * @brief Gets a readable object debug description
	 * @ingroup LunarFLDebug
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Debug|Information")
	static FString GetObjectDebugInfo(const UObject* Object);

	/**
	 * @brief Gets a readable actor debug description
	 * @ingroup LunarFLDebug
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Debug|Information")
	static FString GetActorDebugInfo(const AActor* Actor);

	/**
	 * @brief Gets a readable component debug description
	 * @ingroup LunarFLDebug
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Debug|Information")
	static FString GetComponentDebugInfo(const UActorComponent* Component);

	/**
	 * @brief Gets a readable world debug description
	 * @ingroup LunarFLDebug
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Debug|Information", meta = (WorldContext = "WorldContextObject"))
	static FString GetWorldDebugInfo(const UObject* WorldContextObject);

	/**
	 * @brief Gets complete actor network debug information
	 * @ingroup LunarFLDebug
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Debug|Network")
	static FString GetActorNetworkDebugInfo(const AActor* Actor);

	/**
	 * @brief Draws a debug point with a text label
	 * @ingroup LunarFLDebug
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Debug|Draw", meta = (WorldContext = "WorldContextObject", DevelopmentOnly))
	static void DrawDebugPointWithLabel(const UObject* WorldContextObject, FVector Location, const FString& Label, float PointSize = 12.0f, FLinearColor Color = FLinearColor::Red, float Duration = 0.0f, bool bPersistentLines = false);

	/**
	 * @brief Draws a debug line with a text label
	 * @ingroup LunarFLDebug
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Debug|Draw", meta = (WorldContext = "WorldContextObject", DevelopmentOnly))
	static void DrawDebugLineWithLabel(const UObject* WorldContextObject, FVector Start, FVector End, const FString& Label, FLinearColor Color = FLinearColor::Red, float Thickness = 1.0f, float Duration = 0.0f, bool bPersistentLines = false);

	/**
	 * @brief Draws a debug arrow with a text label
	 * @ingroup LunarFLDebug
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Debug|Draw", meta = (WorldContext = "WorldContextObject", DevelopmentOnly))
	static void DrawDebugArrowWithLabel(const UObject* WorldContextObject, FVector Start, FVector End, const FString& Label, float ArrowSize = 20.0f, FLinearColor Color = FLinearColor::Red, float Thickness = 1.0f, float Duration = 0.0f, bool bPersistentLines = false);

	/**
	 * @brief Draws actor transform axes
	 * @ingroup LunarFLDebug
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Debug|Draw", meta = (DevelopmentOnly))
	static void DrawActorTransform(const AActor* Actor, float AxisLength = 100.0f, float Thickness = 2.0f, float Duration = 0.0f, bool bPersistentLines = false);

	/**
	 * @brief Draws scene component transform axes
	 * @ingroup LunarFLDebug
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Debug|Draw", meta = (DevelopmentOnly))
	static void DrawComponentTransform(const USceneComponent* Component, float AxisLength = 100.0f, float Thickness = 2.0f, float Duration = 0.0f, bool bPersistentLines = false);

	/**
	 * @brief Draws a field of view cone
	 * @ingroup LunarFLDebug
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Debug|Draw", meta = (WorldContext = "WorldContextObject", DevelopmentOnly))
	static void DrawDebugFieldOfView(const UObject* WorldContextObject, FVector Location, FRotator Rotation, float Distance, float HorizontalAngle, float VerticalAngle, FLinearColor Color = FLinearColor::Yellow, float Thickness = 1.0f, float Duration = 0.0f, bool bPersistentLines = false);

	/**
	 * @brief Draws actor velocity as an arrow
	 * @ingroup LunarFLDebug
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Debug|Draw", meta = (DevelopmentOnly))
	static void DrawActorVelocity(const AActor* Actor, float Scale = 1.0f, float ArrowSize = 20.0f, FLinearColor Color = FLinearColor::Green, float Thickness = 2.0f, float Duration = 0.0f, bool bPersistentLines = false);

	/**
	 * @brief Draws connected debug path points
	 * @ingroup LunarFLDebug
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Debug|Draw", meta = (WorldContext = "WorldContextObject", DevelopmentOnly))
	static void DrawDebugPath(const UObject* WorldContextObject, const TArray<FVector>& Points, bool bClosed = false, bool bDrawPointIndices = true, FLinearColor Color = FLinearColor(0.0f, 1.0f, 1.0f, 1.0f), float PointSize = 8.0f, float Thickness = 2.0f, float Duration = 0.0f, bool bPersistentLines = false);

	/**
	 * @brief Draws debug spheres for every point
	 * @ingroup LunarFLDebug
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Debug|Draw", meta = (WorldContext = "WorldContextObject", DevelopmentOnly))
	static void DrawDebugPoints(const UObject* WorldContextObject, const TArray<FVector>& Points, float Radius = 10.0f, FLinearColor Color = FLinearColor(0.0f, 1.0f, 1.0f, 1.0f), float Duration = 0.0f, bool bPersistentLines = false);

	/**
	 * @brief Creates a compact transform debug string
	 * @ingroup LunarFLDebug
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Debug|Formatting")
	static FString TransformToDebugString(const FTransform& Transform, int32 Precision = 2);

	/**
	 * @brief Creates a compact vector debug string
	 * @ingroup LunarFLDebug
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Debug|Formatting")
	static FString VectorToDebugString(FVector Vector, int32 Precision = 2);

	/**
	 * @brief Creates a compact rotator debug string
	 * @ingroup LunarFLDebug
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Debug|Formatting")
	static FString RotatorToDebugString(FRotator Rotator, int32 Precision = 2);

	/**
	 * @brief Creates a compact object array debug string
	 * @ingroup LunarFLDebug
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Debug|Formatting")
	static FString ObjectArrayToDebugString(const TArray<UObject*>& Objects, bool bIncludeClassNames = true);

	/**
	 * @brief Triggers a native debugger breakpoint when condition is true
	 * @ingroup LunarFLDebug
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Debug", meta = (DevelopmentOnly))
	static void TriggerDebugBreak(bool bCondition = true);
};