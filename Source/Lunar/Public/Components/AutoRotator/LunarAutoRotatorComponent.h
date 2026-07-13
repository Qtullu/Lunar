// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AutoRotator/LunarAutoRotatorTypes.h"
#include "Types/LunarTypesTransform.h"
#include "LunarAutoRotatorComponent.generated.h"

/**
 * @file LunarAutoRotatorComponent.h
 * @brief Runtime and editor camera facing actor component
 * @ingroup LunarAutoRotatorComponent
 */

class AActor;
class USceneComponent;

/**
 * @brief Rotates an actor or scene component so it looks at a target
 *
 * The component can rotate an explicitly assigned actor or scene component.
 * When no explicit object is assigned it can resolve a scene component on its
 * owner by Component Tag, then falls back to the owner actor when no tag is set.
 *
 * The look target can be an actor, a scene component, Player Controller 0 camera,
 * or the active level editor viewport camera when editor updates are enabled.
 * Selected rotation axes can be preserved, a rotation offset can be applied,
 * and rotation can be immediate, interpolated, or interpolated at constant speed.
 *
 * The component is local and does not replicate rotation state. Configuration
 * failures are reported through the Lunar console once until state changes.
 *
 * @ingroup LunarAutoRotatorComponent
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = (Lunar), meta = (BlueprintSpawnableComponent, DisplayName = "Lunar Auto Rotator"))
class LUNAR_API ULunarAutoRotatorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Creates auto rotator component defaults */
	ULunarAutoRotatorComponent();

	/**
	 * @brief Updates automatic rotation
	 * @param DeltaTime Time passed since last tick
	 * @param TickType Tick type
	 * @param ThisTickFunction Tick function that triggered the update
	 */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	/** Enables automatic updates in Every Tick mode */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Auto Rotator|Update", meta = (DisplayName = "Auto Rotation Enabled"))
	bool bAutoRotationEnabled = true;

	/** Automatic rotation update mode */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Auto Rotator|Update", meta = (DisplayName = "Update Mode"))
	ELunarAutoRotatorUpdateMode UpdateMode = ELunarAutoRotatorUpdateMode::EveryTick;

	/**
	 * Actor or scene component to rotate
	 * Explicit object has priority over Component Tag
	 */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Lunar|Auto Rotator|Object", meta = (DisplayName = "Object To Rotate", AllowedClasses = "/Script/Engine.Actor,/Script/Engine.SceneComponent"))
	TObjectPtr<UObject> ObjectToRotate = nullptr;

	/**
	 * Component Tag used to find a scene component on the owner
	 * Used only when Object To Rotate is not assigned
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Auto Rotator|Object", meta = (DisplayName = "Component Tag"))
	FName ComponentTag = NAME_None;

	/** Actor or scene component whose world location is used as look target */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Lunar|Auto Rotator|Target", meta = (DisplayName = "Look Target", AllowedClasses = "/Script/Engine.Actor,/Script/Engine.SceneComponent"))
	TObjectPtr<UObject> LookTarget = nullptr;

	/**
	 * Uses Player Controller 0 camera when Look Target is not assigned
	 * Uses active level editor viewport camera outside game worlds
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Auto Rotator|Target", meta = (DisplayName = "Use Player Camera"))
	bool bUsePlayerCamera = true;

	/**
	 * Rotation axes modified by the component
	 * X affects Roll, Y affects Pitch, and Z affects Yaw
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Auto Rotator|Rotation", meta = (DisplayName = "Rotation Axes"))
	ELunarAxisFull RotationAxes = ELunarAxisFull::XYZ;

	/** Rotation added to the calculated look at rotation before axis filtering */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Auto Rotator|Rotation", meta = (DisplayName = "Rotation Offset"))
	FRotator RotationOffset = FRotator::ZeroRotator;

	/**
	 * Uses relative rotation when true
	 * Actors use their root component relative rotation
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Auto Rotator|Rotation", meta = (DisplayName = "Relative"))
	bool bRelative = false;

	/** Rotation interpolation mode */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Auto Rotator|Interpolation", meta = (DisplayName = "Interpolation Mode"))
	ELunarAutoRotatorInterpolationMode InterpolationMode = ELunarAutoRotatorInterpolationMode::Instant;

	/** Rotation interpolation speed, zero applies target rotation immediately */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|Auto Rotator|Interpolation", meta = (ClampMin = "0.0", UIMin = "0.0", DisplayName = "Interpolation Speed"))
	float InterpolationSpeed = 5.0f;

	/** Allows automatic updates in non-game editor worlds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Auto Rotator|Editor", meta = (DisplayName = "Update In Editor"))
	bool bUpdateInEditor = false;

public:
	/**
	 * @brief Enables or disables automatic rotation updates
	 * Manual Update Rotation Now calls remain available when disabled
	 * @param bEnabled New automatic rotation state
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Auto Rotator|Update", meta = (DisplayName = "Set Auto Rotation Enabled"))
	void SetAutoRotationEnabled(bool bEnabled);

	/**
	 * @brief Checks whether automatic rotation updates are enabled
	 * @return True when automatic updates are enabled
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Auto Rotator|Update", meta = (DisplayName = "Is Auto Rotation Enabled"))
	bool IsAutoRotationEnabled() const;

	/**
	 * @brief Sets automatic rotation update mode
	 * @param NewUpdateMode New update mode
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Auto Rotator|Update", meta = (DisplayName = "Set Update Mode"))
	void SetUpdateMode(ELunarAutoRotatorUpdateMode NewUpdateMode);

	/**
	 * @brief Calculates and applies one rotation update
	 * Interpolation modes perform one step using current world frame delta
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lunar|Auto Rotator|Update", meta = (DisplayName = "Update Rotation Now"))
	void UpdateRotationNow();

public:
	/**
	 * @brief Sets actor or scene component to rotate
	 * Passing null clears explicit object selection
	 * @param NewObjectToRotate Actor or scene component to rotate
	 * @return True if object selection was accepted
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Auto Rotator|Object", meta = (DisplayName = "Set Object To Rotate"))
	bool SetObjectToRotate(UObject* NewObjectToRotate);

	/** Clears explicit object selection */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Auto Rotator|Object", meta = (DisplayName = "Clear Object To Rotate"))
	void ClearObjectToRotate();

	/**
	 * @brief Sets Component Tag used to resolve scene component on owner
	 * @param NewComponentTag New Component Tag
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Auto Rotator|Object", meta = (DisplayName = "Set Component Tag"))
	void SetObjectToRotateComponentTag(FName NewComponentTag);

public:
	/**
	 * @brief Sets actor or scene component used as look target
	 * Passing null clears explicit look target
	 * @param NewLookTarget Actor or scene component to look at
	 * @return True if target selection was accepted
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Auto Rotator|Target", meta = (DisplayName = "Set Look Target"))
	bool SetLookTarget(UObject* NewLookTarget);

	/** Clears explicit look target selection */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Auto Rotator|Target", meta = (DisplayName = "Clear Look Target"))
	void ClearLookTarget();

	/**
	 * @brief Enables or disables Player Controller 0 camera fallback
	 * @param bEnabled New player camera fallback state
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Auto Rotator|Target", meta = (DisplayName = "Set Use Player Camera"))
	void SetUsePlayerCamera(bool bEnabled);

public:
	/**
	 * @brief Sets rotation axes modified by the component
	 * @param NewRotationAxes New rotation axes
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Auto Rotator|Rotation", meta = (DisplayName = "Set Rotation Axes"))
	void SetRotationAxes(ELunarAxisFull NewRotationAxes);

	/**
	 * @brief Sets rotation offset applied after look at calculation
	 * @param NewRotationOffset New rotation offset
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Auto Rotator|Rotation", meta = (DisplayName = "Set Rotation Offset"))
	void SetRotationOffset(FRotator NewRotationOffset);

	/**
	 * @brief Selects world or relative rotation
	 * @param bNewRelative Uses relative rotation when true
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Auto Rotator|Rotation", meta = (DisplayName = "Set Relative"))
	void SetRelative(bool bNewRelative);

	/**
	 * @brief Sets rotation interpolation mode
	 * @param NewInterpolationMode New interpolation mode
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Auto Rotator|Interpolation", meta = (DisplayName = "Set Interpolation Mode"))
	void SetInterpolationMode(ELunarAutoRotatorInterpolationMode NewInterpolationMode);

	/**
	 * @brief Sets rotation interpolation speed
	 * Negative values are clamped to zero
	 * @param NewInterpolationSpeed New interpolation speed
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Auto Rotator|Interpolation", meta = (DisplayName = "Set Interpolation Speed"))
	void SetInterpolationSpeed(float NewInterpolationSpeed);

private:
	/** Calculates and applies rotation update */
	bool UpdateRotationInternal(float DeltaTime);

	/** Resolves actor or scene component to rotate */
	bool ResolveObjectToRotate(AActor*& OutActor, USceneComponent*& OutComponent);

	/** Resolves look target world location */
	bool ResolveLookTargetLocation(FVector& OutTargetLocation);

	/** Resolves runtime or editor camera world location */
	bool ResolvePlayerCameraLocation(FVector& OutCameraLocation);

	/** Gets current rotation in selected world or relative space */
	FRotator GetCurrentRotation(AActor* Actor, USceneComponent* Component) const;

	/** Converts desired world rotation to selected world or relative space */
	FRotator ConvertWorldRotationToSelectedSpace(const FRotator& WorldRotation, USceneComponent* Component) const;

	/** Applies final rotation to resolved actor or scene component */
	bool ApplyRotation(AActor* Actor, USceneComponent* Component, const FRotator& NewRotation);

	/** Reports an error once until state changes or an update succeeds */
	void ReportErrorOnce(const FString& ErrorMessage);

	/** Clears currently latched error */
	void ClearReportedError();

	/** Checks whether object is a supported actor or scene component */
	static bool IsSupportedActorOrComponent(const UObject* Object);

private:
	/** Last reported error used to prevent per-tick message spam */
	FString LastReportedError;
};
