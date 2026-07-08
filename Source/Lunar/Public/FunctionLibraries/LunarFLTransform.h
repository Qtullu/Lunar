// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/HitResult.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LunarTypes.h"
#include "LunarFLTransform.generated.h"

/**
 * @file LunarFLTransform.h
 * @brief Transform helper function library
 * @ingroup LunarFLTransform
 */

 /**
  * @brief Blueprint utility functions for actor and component transforms
  * @ingroup LunarFLTransform
  */
UCLASS()
class LUNAR_API ULunarFLTransform : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	//Transform

	/**
	 * @brief Sets selected transform axes on actor or scene component
	 * Actor uses world transform when Relative is false
	 * Actor uses root component relative transform when Relative is true
	 * Scene component uses world or relative transform based on Relative
	 * Values X affects X axis
	 * Values Y affects Y axis
	 * Values Z affects Z axis
	 * @param ActorOrComponent Target actor or scene component
	 * @param TransformType Transform data type to modify
	 * @param Axis Axes to modify
	 * @param Values Axis values
	 * @param SweepHitResult Sweep hit result when sweep is used
	 * @param bRelative Uses relative transform when true
	 * @param bSweep Uses swept movement when true
	 * @param bTeleport Uses teleport movement when true
	 * @return True if transform was changed
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Transform", meta = (DisplayName = "Set Transform Axis", AdvancedDisplay = "bRelative,bSweep,bTeleport"))
	static bool SetTransformAxis(UObject* ActorOrComponent, ELunarLocationRotationScale TransformType, ELunarAxisFull Axis, FVector Values, FHitResult& SweepHitResult, bool bRelative = false, bool bSweep = false, bool bTeleport = false);

	/**
	 * @brief Adds values to selected transform axes on actor or scene component
	 * Actor uses world transform when Relative is false
	 * Actor uses root component relative transform when Relative is true
	 * Scene component uses world or relative transform based on Relative
	 * Values X affects X axis
	 * Values Y affects Y axis
	 * Values Z affects Z axis
	 * @param ActorOrComponent Target actor or scene component
	 * @param TransformType Transform data type to modify
	 * @param Axis Axes to modify
	 * @param Values Axis values to add
	 * @param SweepHitResult Sweep hit result when sweep is used
	 * @param bRelative Uses relative transform when true
	 * @param bSweep Uses swept movement when true
	 * @param bTeleport Uses teleport movement when true
	 * @return True if transform was changed
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Transform", meta = (DisplayName = "Add Transform Axis", AdvancedDisplay = "bRelative,bSweep,bTeleport"))
	static bool AddTransformAxis(UObject* ActorOrComponent, ELunarLocationRotationScale TransformType, ELunarAxisFull Axis, FVector Values, FHitResult& SweepHitResult, bool bRelative = false, bool bSweep = false, bool bTeleport = false);

	//Locations

	/**
	 * @brief Sets selected location axes on actor or scene component
	 * Actor uses world location when Relative is false
	 * Actor uses root component relative location when Relative is true
	 * Scene component uses world or relative location based on Relative
	 * Values X affects X location
	 * Values Y affects Y location
	 * Values Z affects Z location
	 * @param ActorOrComponent Target actor or scene component
	 * @param Axis Axes to modify
	 * @param Values Location axis values
	 * @param SweepHitResult Sweep hit result when sweep is used
	 * @param bRelative Uses relative location when true
	 * @param bSweep Uses swept movement when true
	 * @param bTeleport Uses teleport movement when true
	 * @return True if location was changed
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Transform", meta = (DisplayName = "Set Location Axis", AdvancedDisplay = "bRelative,bSweep,bTeleport"))
	static bool SetLocationAxis(UObject* ActorOrComponent, ELunarAxisFull Axis, FVector Values, FHitResult& SweepHitResult, bool bRelative = false, bool bSweep = false, bool bTeleport = false);

	/**
	 * @brief Adds values to selected location axes on actor or scene component
	 * Actor uses world location when Relative is false
	 * Actor uses root component relative location when Relative is true
	 * Scene component uses world or relative location based on Relative
	 * Values X affects X location
	 * Values Y affects Y location
	 * Values Z affects Z location
	 * @param ActorOrComponent Target actor or scene component
	 * @param Axis Axes to modify
	 * @param Values Location axis values to add
	 * @param SweepHitResult Sweep hit result when sweep is used
	 * @param bRelative Uses relative location when true
	 * @param bSweep Uses swept movement when true
	 * @param bTeleport Uses teleport movement when true
	 * @return True if location was changed
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Transform", meta = (DisplayName = "Add Location Axis", AdvancedDisplay = "bRelative,bSweep,bTeleport"))
	static bool AddLocationAxis(UObject* ActorOrComponent, ELunarAxisFull Axis, FVector Values, FHitResult& SweepHitResult, bool bRelative = false, bool bSweep = false, bool bTeleport = false);

	//Rotations

	/**
	 * @brief Sets selected rotation axes on actor or scene component
	 * Actor uses world rotation when Relative is false
	 * Actor uses root component relative rotation when Relative is true
	 * Scene component uses world or relative rotation based on Relative
	 * Values X affects Roll
	 * Values Y affects Pitch
	 * Values Z affects Yaw
	 * @param ActorOrComponent Target actor or scene component
	 * @param Axis Axes to modify
	 * @param Values Rotation axis values
	 * @param SweepHitResult Sweep hit result when sweep is used
	 * @param bRelative Uses relative rotation when true
	 * @param bSweep Uses swept movement when true
	 * @param bTeleport Uses teleport movement when true
	 * @return True if rotation was changed
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Transform", meta = (DisplayName = "Set Rotation Axis", AdvancedDisplay = "bRelative,bSweep,bTeleport"))
	static bool SetRotationAxis(UObject* ActorOrComponent, ELunarAxisFull Axis, FVector Values, FHitResult& SweepHitResult, bool bRelative = false, bool bSweep = false, bool bTeleport = false);

	/**
	 * @brief Adds values to selected rotation axes on actor or scene component
	 * Actor uses world rotation when Relative is false
	 * Actor uses root component relative rotation when Relative is true
	 * Scene component uses world or relative rotation based on Relative
	 * Values X affects Roll
	 * Values Y affects Pitch
	 * Values Z affects Yaw
	 * @param ActorOrComponent Target actor or scene component
	 * @param Axis Axes to modify
	 * @param Values Rotation axis values to add
	 * @param SweepHitResult Sweep hit result when sweep is used
	 * @param bRelative Uses relative rotation when true
	 * @param bSweep Uses swept movement when true
	 * @param bTeleport Uses teleport movement when true
	 * @return True if rotation was changed
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Transform", meta = (DisplayName = "Add Rotation Axis", AdvancedDisplay = "bRelative,bSweep,bTeleport"))
	static bool AddRotationAxis(UObject* ActorOrComponent, ELunarAxisFull Axis, FVector Values, FHitResult& SweepHitResult, bool bRelative = false, bool bSweep = false, bool bTeleport = false);

	//Scales

	/**
	 * @brief Sets selected scale axes on actor or scene component
	 * Actor uses world scale when Relative is false
	 * Actor uses root component relative scale when Relative is true
	 * Scene component uses world or relative scale based on Relative
	 * Values X affects X scale
	 * Values Y affects Y scale
	 * Values Z affects Z scale
	 * @param ActorOrComponent Target actor or scene component
	 * @param Axis Axes to modify
	 * @param Values Scale axis values
	 * @param bRelative Uses relative scale when true
	 * @return True if scale was changed
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Transform", meta = (DisplayName = "Set Scale Axis", AdvancedDisplay = "bRelative"))
	static bool SetScaleAxis(UObject* ActorOrComponent, ELunarAxisFull Axis, FVector Values, bool bRelative = false);

	/**
	 * @brief Adds values to selected scale axes on actor or scene component
	 * Actor uses world scale when Relative is false
	 * Actor uses root component relative scale when Relative is true
	 * Scene component uses world or relative scale based on Relative
	 * Values X affects X scale
	 * Values Y affects Y scale
	 * Values Z affects Z scale
	 * @param ActorOrComponent Target actor or scene component
	 * @param Axis Axes to modify
	 * @param Values Scale axis values to add
	 * @param bRelative Uses relative scale when true
	 * @return True if scale was changed
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Transform", meta = (DisplayName = "Add Scale Axis", AdvancedDisplay = "bRelative"))
	static bool AddScaleAxis(UObject* ActorOrComponent, ELunarAxisFull Axis, FVector Values, bool bRelative = false);
};