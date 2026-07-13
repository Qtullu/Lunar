// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "Components/AutoRotator/LunarAutoRotatorComponent.h"

#include "Camera/PlayerCameraManager.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/App.h"
#include "NativeGameplayTags.h"
#include "Subsystems/LunarConsoleSubsystem.h"

#if WITH_EDITOR
#include "Editor.h"
#include "LevelEditorViewport.h"
#endif

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Lunar_AutoRotator, "Lunar.AutoRotator");

namespace LunarAutoRotator_Private
{
	static bool HasAxisX(const ELunarAxisFull Axis)
	{
		return Axis == ELunarAxisFull::X || Axis == ELunarAxisFull::XY || Axis == ELunarAxisFull::XZ || Axis == ELunarAxisFull::XYZ;
	}

	static bool HasAxisY(const ELunarAxisFull Axis)
	{
		return Axis == ELunarAxisFull::Y || Axis == ELunarAxisFull::XY || Axis == ELunarAxisFull::YZ || Axis == ELunarAxisFull::XYZ;
	}

	static bool HasAxisZ(const ELunarAxisFull Axis)
	{
		return Axis == ELunarAxisFull::Z || Axis == ELunarAxisFull::XZ || Axis == ELunarAxisFull::YZ || Axis == ELunarAxisFull::XYZ;
	}

	static FRotator ApplyAxisFilter(const FRotator& CurrentRotation, const FRotator& TargetRotation, const ELunarAxisFull Axis)
	{
		FRotator Result = CurrentRotation;

		if (HasAxisX(Axis))
		{
			Result.Roll = TargetRotation.Roll;
		}

		if (HasAxisY(Axis))
		{
			Result.Pitch = TargetRotation.Pitch;
		}

		if (HasAxisZ(Axis))
		{
			Result.Yaw = TargetRotation.Yaw;
		}

		Result.Normalize();
		return Result;
	}

	static FRotator InterpolateRotation(const FRotator& CurrentRotation, const FRotator& TargetRotation, const float DeltaTime, const ELunarAutoRotatorInterpolationMode Mode, const float Speed)
	{
		if (Mode == ELunarAutoRotatorInterpolationMode::Instant || Speed <= 0.0f)
		{
			return TargetRotation;
		}

		switch (Mode)
		{
		case ELunarAutoRotatorInterpolationMode::InterpTo:
			return FMath::RInterpTo(CurrentRotation, TargetRotation, FMath::Max(DeltaTime, 0.0f), Speed);

		case ELunarAutoRotatorInterpolationMode::InterpToConstant:
			return FMath::RInterpConstantTo(CurrentRotation, TargetRotation, FMath::Max(DeltaTime, 0.0f), Speed);

		case ELunarAutoRotatorInterpolationMode::Instant:
		default:
			return TargetRotation;
		}
	}
}

ULunarAutoRotatorComponent::ULunarAutoRotatorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;

	bTickInEditor = true;
	bAutoActivate = true;
	SetIsReplicatedByDefault(false);
}

void ULunarAutoRotatorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bAutoRotationEnabled || UpdateMode != ELunarAutoRotatorUpdateMode::EveryTick)
	{
		return;
	}

	const UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

#if WITH_EDITOR
	if (!World->IsGameWorld() && !bUpdateInEditor)
	{
		return;
	}
#endif

	UpdateRotationInternal(DeltaTime);
}

void ULunarAutoRotatorComponent::SetAutoRotationEnabled(bool bEnabled)
{
	bAutoRotationEnabled = bEnabled;
	ClearReportedError();
}

bool ULunarAutoRotatorComponent::IsAutoRotationEnabled() const
{
	return bAutoRotationEnabled;
}

void ULunarAutoRotatorComponent::SetUpdateMode(ELunarAutoRotatorUpdateMode NewUpdateMode)
{
	UpdateMode = NewUpdateMode;
	ClearReportedError();
}

void ULunarAutoRotatorComponent::UpdateRotationNow()
{
	float DeltaTime = 0.0f;

	if (const UWorld* World = GetWorld())
	{
		DeltaTime = World->GetDeltaSeconds();
	}

	if (DeltaTime <= 0.0f)
	{
		DeltaTime = static_cast<float>(FApp::GetDeltaTime());
	}

	if (DeltaTime <= 0.0f)
	{
		DeltaTime = 1.0f / 60.0f;
	}

	UpdateRotationInternal(DeltaTime);
}

bool ULunarAutoRotatorComponent::SetObjectToRotate(UObject* NewObjectToRotate)
{
	if (NewObjectToRotate && (!IsValid(NewObjectToRotate) || !IsSupportedActorOrComponent(NewObjectToRotate)))
	{
		ReportErrorOnce(FString::Printf(TEXT("%s on actor '%s' failed to set Object To Rotate: object must be Actor or SceneComponent."), *GetNameSafe(this), *GetNameSafe(GetOwner())));
		return false;
	}

	ObjectToRotate = NewObjectToRotate;
	ClearReportedError();
	return true;
}

void ULunarAutoRotatorComponent::ClearObjectToRotate()
{
	ObjectToRotate = nullptr;
	ClearReportedError();
}

void ULunarAutoRotatorComponent::SetObjectToRotateComponentTag(FName NewComponentTag)
{
	ComponentTag = NewComponentTag;
	ClearReportedError();
}

bool ULunarAutoRotatorComponent::SetLookTarget(UObject* NewLookTarget)
{
	if (NewLookTarget && (!IsValid(NewLookTarget) || !IsSupportedActorOrComponent(NewLookTarget)))
	{
		ReportErrorOnce(FString::Printf(TEXT("%s on actor '%s' failed to set Look Target: object must be Actor or SceneComponent."), *GetNameSafe(this), *GetNameSafe(GetOwner())));
		return false;
	}

	LookTarget = NewLookTarget;
	ClearReportedError();
	return true;
}

void ULunarAutoRotatorComponent::ClearLookTarget()
{
	LookTarget = nullptr;
	ClearReportedError();
}

void ULunarAutoRotatorComponent::SetUsePlayerCamera(bool bEnabled)
{
	bUsePlayerCamera = bEnabled;
	ClearReportedError();
}

void ULunarAutoRotatorComponent::SetRotationAxes(ELunarAxisFull NewRotationAxes)
{
	RotationAxes = NewRotationAxes;
	ClearReportedError();
}

void ULunarAutoRotatorComponent::SetRotationOffset(FRotator NewRotationOffset)
{
	NewRotationOffset.Normalize();
	RotationOffset = NewRotationOffset;
	ClearReportedError();
}

void ULunarAutoRotatorComponent::SetRelative(bool bNewRelative)
{
	bRelative = bNewRelative;
	ClearReportedError();
}

void ULunarAutoRotatorComponent::SetInterpolationMode(ELunarAutoRotatorInterpolationMode NewInterpolationMode)
{
	InterpolationMode = NewInterpolationMode;
	ClearReportedError();
}

void ULunarAutoRotatorComponent::SetInterpolationSpeed(float NewInterpolationSpeed)
{
	InterpolationSpeed = FMath::Max(NewInterpolationSpeed, 0.0f);
	ClearReportedError();
}

bool ULunarAutoRotatorComponent::UpdateRotationInternal(float DeltaTime)
{
	using namespace LunarAutoRotator_Private;

	AActor* RotationActor = nullptr;
	USceneComponent* RotationComponent = nullptr;

	if (!ResolveObjectToRotate(RotationActor, RotationComponent))
	{
		return false;
	}

	FVector TargetLocation = FVector::ZeroVector;

	if (!ResolveLookTargetLocation(TargetLocation))
	{
		return false;
	}

	const FVector SourceLocation = RotationActor && !bRelative ? RotationActor->GetActorLocation() : RotationComponent->GetComponentLocation();
	const FVector LookDirection = TargetLocation - SourceLocation;

	if (LookDirection.IsNearlyZero())
	{
		ReportErrorOnce(FString::Printf(TEXT("%s on actor '%s' failed: rotation object and Look Target have the same world location."), *GetNameSafe(this), *GetNameSafe(GetOwner())));
		return false;
	}

	FRotator DesiredWorldRotation = LookDirection.Rotation() + RotationOffset;
	DesiredWorldRotation.Normalize();

	const FRotator CurrentRotation = GetCurrentRotation(RotationActor, RotationComponent);
	const FRotator DesiredRotation = ConvertWorldRotationToSelectedSpace(DesiredWorldRotation, RotationComponent);
	const FRotator FilteredRotation = ApplyAxisFilter(CurrentRotation, DesiredRotation, RotationAxes);
	const FRotator FinalRotation = InterpolateRotation(CurrentRotation, FilteredRotation, DeltaTime, InterpolationMode, InterpolationSpeed);

	if (!ApplyRotation(RotationActor, RotationComponent, FinalRotation))
	{
		return false;
	}

	ClearReportedError();
	return true;
}

bool ULunarAutoRotatorComponent::ResolveObjectToRotate(AActor*& OutActor, USceneComponent*& OutComponent)
{
	OutActor = nullptr;
	OutComponent = nullptr;

	AActor* Owner = GetOwner();

	if (!IsValid(Owner))
	{
		ReportErrorOnce(FString::Printf(TEXT("%s failed: Owner Actor is invalid."), *GetNameSafe(this)));
		return false;
	}

	if (ObjectToRotate)
	{
		if (!IsValid(ObjectToRotate))
		{
			ReportErrorOnce(FString::Printf(TEXT("%s on actor '%s' failed: Object To Rotate is invalid."), *GetNameSafe(this), *GetNameSafe(Owner)));
			return false;
		}

		OutActor = Cast<AActor>(ObjectToRotate);
		OutComponent = Cast<USceneComponent>(ObjectToRotate);

		if (!OutActor && !OutComponent)
		{
			ReportErrorOnce(FString::Printf(TEXT("%s on actor '%s' failed: Object To Rotate is not Actor or SceneComponent."), *GetNameSafe(this), *GetNameSafe(Owner)));
			return false;
		}
	}
	else if (!ComponentTag.IsNone())
	{
		TInlineComponentArray<USceneComponent*> SceneComponents;
		Owner->GetComponents(SceneComponents);

		for (USceneComponent* SceneComponent : SceneComponents)
		{
			if (IsValid(SceneComponent) && SceneComponent->ComponentHasTag(ComponentTag))
			{
				OutComponent = SceneComponent;
				break;
			}
		}

		if (!OutComponent)
		{
			ReportErrorOnce(FString::Printf(TEXT("%s on actor '%s' failed: SceneComponent with Component Tag '%s' was not found."), *GetNameSafe(this), *GetNameSafe(Owner), *ComponentTag.ToString()));
			return false;
		}
	}
	else
	{
		OutActor = Owner;
	}

	if (OutActor && bRelative)
	{
		OutComponent = OutActor->GetRootComponent();

		if (!IsValid(OutComponent))
		{
			ReportErrorOnce(FString::Printf(TEXT("%s on actor '%s' failed: rotation Actor '%s' has no valid root SceneComponent for relative rotation."), *GetNameSafe(this), *GetNameSafe(Owner), *GetNameSafe(OutActor)));
			return false;
		}
	}

	return true;
}

bool ULunarAutoRotatorComponent::ResolveLookTargetLocation(FVector& OutTargetLocation)
{
	OutTargetLocation = FVector::ZeroVector;

	if (LookTarget)
	{
		if (!IsValid(LookTarget))
		{
			ReportErrorOnce(FString::Printf(TEXT("%s on actor '%s' failed: Look Target is invalid."), *GetNameSafe(this), *GetNameSafe(GetOwner())));
			return false;
		}

		if (const AActor* TargetActor = Cast<AActor>(LookTarget))
		{
			OutTargetLocation = TargetActor->GetActorLocation();
			return true;
		}

		if (const USceneComponent* TargetComponent = Cast<USceneComponent>(LookTarget))
		{
			OutTargetLocation = TargetComponent->GetComponentLocation();
			return true;
		}

		ReportErrorOnce(FString::Printf(TEXT("%s on actor '%s' failed: Look Target is not Actor or SceneComponent."), *GetNameSafe(this), *GetNameSafe(GetOwner())));
		return false;
	}

	if (!bUsePlayerCamera)
	{
		ReportErrorOnce(FString::Printf(TEXT("%s on actor '%s' failed: Look Target is not set and Use Player Camera is disabled."), *GetNameSafe(this), *GetNameSafe(GetOwner())));
		return false;
	}

	return ResolvePlayerCameraLocation(OutTargetLocation);
}

bool ULunarAutoRotatorComponent::ResolvePlayerCameraLocation(FVector& OutCameraLocation)
{
	OutCameraLocation = FVector::ZeroVector;

	const UWorld* World = GetWorld();

	if (!World)
	{
		ReportErrorOnce(FString::Printf(TEXT("%s on actor '%s' failed: World is invalid."), *GetNameSafe(this), *GetNameSafe(GetOwner())));
		return false;
	}

#if WITH_EDITOR
	if (!World->IsGameWorld())
	{
		if (!GCurrentLevelEditingViewportClient)
		{
			ReportErrorOnce(FString::Printf(TEXT("%s on actor '%s' failed: active level editor viewport camera is unavailable."), *GetNameSafe(this), *GetNameSafe(GetOwner())));
			return false;
		}

		OutCameraLocation = GCurrentLevelEditingViewportClient->GetViewLocation();
		return true;
	}
#endif

	const APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);

	if (!IsValid(CameraManager))
	{
		ReportErrorOnce(FString::Printf(TEXT("%s on actor '%s' failed: Player Controller 0 camera is unavailable."), *GetNameSafe(this), *GetNameSafe(GetOwner())));
		return false;
	}

	OutCameraLocation = CameraManager->GetCameraLocation();
	return true;
}

FRotator ULunarAutoRotatorComponent::GetCurrentRotation(AActor* Actor, USceneComponent* Component) const
{
	if (Actor && !bRelative)
	{
		return Actor->GetActorRotation();
	}

	return bRelative ? Component->GetRelativeRotation() : Component->GetComponentRotation();
}

FRotator ULunarAutoRotatorComponent::ConvertWorldRotationToSelectedSpace(const FRotator& WorldRotation, USceneComponent* Component) const
{
	if (!bRelative || !Component)
	{
		return WorldRotation;
	}

	const USceneComponent* AttachParent = Component->GetAttachParent();

	if (!AttachParent)
	{
		return WorldRotation;
	}

	FRotator RelativeRotation = AttachParent->GetComponentTransform().InverseTransformRotation(WorldRotation.Quaternion()).Rotator();
	RelativeRotation.Normalize();
	return RelativeRotation;
}

bool ULunarAutoRotatorComponent::ApplyRotation(AActor* Actor, USceneComponent* Component, const FRotator& NewRotation)
{
	if (Actor && !bRelative)
	{
		if (!Actor->SetActorRotation(NewRotation, ETeleportType::None))
		{
			ReportErrorOnce(FString::Printf(TEXT("%s on actor '%s' failed: could not apply rotation to Actor '%s'."), *GetNameSafe(this), *GetNameSafe(GetOwner()), *GetNameSafe(Actor)));
			return false;
		}

		return true;
	}

	if (!IsValid(Component))
	{
		ReportErrorOnce(FString::Printf(TEXT("%s on actor '%s' failed: resolved rotation SceneComponent is invalid."), *GetNameSafe(this), *GetNameSafe(GetOwner())));
		return false;
	}

	if (bRelative)
	{
		Component->SetRelativeRotation(NewRotation, false, nullptr, ETeleportType::None);
	}
	else
	{
		Component->SetWorldRotation(NewRotation, false, nullptr, ETeleportType::None);
	}

	return true;
}

void ULunarAutoRotatorComponent::ReportErrorOnce(const FString& ErrorMessage)
{
	if (ErrorMessage.IsEmpty() || LastReportedError == ErrorMessage)
	{
		return;
	}

	LastReportedError = ErrorMessage;
	ULunarConsoleSubsystem::AddMessage(TAG_Lunar_AutoRotator, ELunarConsoleMessageVerbosity::Error, ErrorMessage);
}

void ULunarAutoRotatorComponent::ClearReportedError()
{
	LastReportedError.Reset();
}

bool ULunarAutoRotatorComponent::IsSupportedActorOrComponent(const UObject* Object)
{
	return Object && (Object->IsA<AActor>() || Object->IsA<USceneComponent>());
}
