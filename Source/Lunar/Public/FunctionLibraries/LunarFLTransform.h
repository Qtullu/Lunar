// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/HitResult.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LunarTypes.h"
#include "LunarFLTransform.generated.h"

/**
 *
 */
UCLASS()
class LUNAR_API ULunarFLTransform : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	//Transform

	/**
	 * Sets selected transform axes on Actor or SceneComponent
	 * Actor uses world transform when Relative is false
	 * Actor uses root component relative transform when Relative is true
	 * SceneComponent uses world or relative transform based on Relative
	 * Values X affects X axis
	 * Values Y affects Y axis
	 * Values Z affects Z axis
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Transform", meta = (DisplayName = "Set Transform Axis", AdvancedDisplay = "bRelative,bSweep,bTeleport"))
	static bool SetTransformAxis(UObject* ActorOrComponent, ELunarLocationRotationScale TransformType, ELunarAxisFull Axis, FVector Values, FHitResult& SweepHitResult, bool bRelative = false, bool bSweep = false, bool bTeleport = false);

	/**
	 * Adds values to selected transform axes on Actor or SceneComponent
	 * Actor uses world transform when Relative is false
	 * Actor uses root component relative transform when Relative is true
	 * SceneComponent uses world or relative transform based on Relative
	 * Values X affects X axis
	 * Values Y affects Y axis
	 * Values Z affects Z axis
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Transform", meta = (DisplayName = "Add Transform Axis", AdvancedDisplay = "bRelative,bSweep,bTeleport"))
	static bool AddTransformAxis(UObject* ActorOrComponent, ELunarLocationRotationScale TransformType, ELunarAxisFull Axis, FVector Values, FHitResult& SweepHitResult, bool bRelative = false, bool bSweep = false, bool bTeleport = false);

	//Locations

	/**
	 * Sets selected location axes on Actor or SceneComponent
	 * Actor uses world location when Relative is false
	 * Actor uses root component relative location when Relative is true
	 * SceneComponent uses world or relative location based on Relative
	 * Values X affects X location
	 * Values Y affects Y location
	 * Values Z affects Z location
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Transform", meta = (DisplayName = "Set Location Axis", AdvancedDisplay = "bRelative,bSweep,bTeleport"))
	static bool SetLocationAxis(UObject* ActorOrComponent, ELunarAxisFull Axis, FVector Values, FHitResult& SweepHitResult, bool bRelative = false, bool bSweep = false, bool bTeleport = false);

	/**
	 * Adds values to selected location axes on Actor or SceneComponent
	 * Actor uses world location when Relative is false
	 * Actor uses root component relative location when Relative is true
	 * SceneComponent uses world or relative location based on Relative
	 * Values X affects X location
	 * Values Y affects Y location
	 * Values Z affects Z location
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Transform", meta = (DisplayName = "Add Location Axis", AdvancedDisplay = "bRelative,bSweep,bTeleport"))
	static bool AddLocationAxis(UObject* ActorOrComponent, ELunarAxisFull Axis, FVector Values, FHitResult& SweepHitResult, bool bRelative = false, bool bSweep = false, bool bTeleport = false);

	//Rotations

	/**
	 * Sets selected rotation axes on Actor or SceneComponent
	 * Actor uses world rotation when Relative is false
	 * Actor uses root component relative rotation when Relative is true
	 * SceneComponent uses world or relative rotation based on Relative
	 * Values X affects Roll
	 * Values Y affects Pitch
	 * Values Z affects Yaw
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Transform", meta = (DisplayName = "Set Rotation Axis", AdvancedDisplay = "bRelative,bSweep,bTeleport"))
	static bool SetRotationAxis(UObject* ActorOrComponent, ELunarAxisFull Axis, FVector Values, FHitResult& SweepHitResult, bool bRelative = false, bool bSweep = false, bool bTeleport = false);

	/**
	 * Adds values to selected rotation axes on Actor or SceneComponent
	 * Actor uses world rotation when Relative is false
	 * Actor uses root component relative rotation when Relative is true
	 * SceneComponent uses world or relative rotation based on Relative
	 * Values X affects Roll
	 * Values Y affects Pitch
	 * Values Z affects Yaw
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Transform", meta = (DisplayName = "Add Rotation Axis", AdvancedDisplay = "bRelative,bSweep,bTeleport"))
	static bool AddRotationAxis(UObject* ActorOrComponent, ELunarAxisFull Axis, FVector Values, FHitResult& SweepHitResult, bool bRelative = false, bool bSweep = false, bool bTeleport = false);

	//Scales

	/**
	 * Sets selected scale axes on Actor or SceneComponent
	 * Actor uses world scale when Relative is false
	 * Actor uses root component relative scale when Relative is true
	 * SceneComponent uses world or relative scale based on Relative
	 * Values X affects X scale
	 * Values Y affects Y scale
	 * Values Z affects Z scale
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Transform", meta = (DisplayName = "Set Scale Axis", AdvancedDisplay = "bRelative"))
	static bool SetScaleAxis(UObject* ActorOrComponent, ELunarAxisFull Axis, FVector Values, bool bRelative = false);

	/**
	 * Adds values to selected scale axes on Actor or SceneComponent
	 * Actor uses world scale when Relative is false
	 * Actor uses root component relative scale when Relative is true
	 * SceneComponent uses world or relative scale based on Relative
	 * Values X affects X scale
	 * Values Y affects Y scale
	 * Values Z affects Z scale
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Transform", meta = (DisplayName = "Add Scale Axis", AdvancedDisplay = "bRelative"))
	static bool AddScaleAxis(UObject* ActorOrComponent, ELunarAxisFull Axis, FVector Values, bool bRelative = false);
};