// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "LunarFLTransform.h"

#include "Components/SceneComponent.h"
#include "Engine/HitResult.h"
#include "FunctionLibraries/LunarFLRandom.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"
#include "Subsystems/Console/LunarConsoleSubsystem.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Lunar_Transform, "Lunar.Transform");

namespace LunarFLTransform_Private
{
	static ETeleportType GetTeleportType(const bool bTeleport)
	{
		return bTeleport ? ETeleportType::TeleportPhysics : ETeleportType::None;
	}

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

	static void SetVectorAxisValues(FVector& Vector, const ELunarAxisFull Axis, const FVector& Values)
	{
		if (HasAxisX(Axis))
		{
			Vector.X = Values.X;
		}

		if (HasAxisY(Axis))
		{
			Vector.Y = Values.Y;
		}

		if (HasAxisZ(Axis))
		{
			Vector.Z = Values.Z;
		}
	}

	static void AddVectorAxisValues(FVector& Vector, const ELunarAxisFull Axis, const FVector& Values)
	{
		if (HasAxisX(Axis))
		{
			Vector.X += Values.X;
		}

		if (HasAxisY(Axis))
		{
			Vector.Y += Values.Y;
		}

		if (HasAxisZ(Axis))
		{
			Vector.Z += Values.Z;
		}
	}

	static void SetRotatorAxisValues(FRotator& Rotator, const ELunarAxisFull Axis, const FVector& Values)
	{
		if (HasAxisX(Axis))
		{
			Rotator.Roll = Values.X;
		}

		if (HasAxisY(Axis))
		{
			Rotator.Pitch = Values.Y;
		}

		if (HasAxisZ(Axis))
		{
			Rotator.Yaw = Values.Z;
		}
	}

	static void AddRotatorAxisValues(FRotator& Rotator, const ELunarAxisFull Axis, const FVector& Values)
	{
		if (HasAxisX(Axis))
		{
			Rotator.Roll += Values.X;
		}

		if (HasAxisY(Axis))
		{
			Rotator.Pitch += Values.Y;
		}

		if (HasAxisZ(Axis))
		{
			Rotator.Yaw += Values.Z;
		}
	}

	static bool ResolveActorOrComponent(UObject* ActorOrComponent, bool bRelative, AActor*& OutActor, USceneComponent*& OutComponent, const TCHAR* FunctionName)
	{
		OutActor = nullptr;
		OutComponent = nullptr;

		if (!IsValid(ActorOrComponent))
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Transform, ELunarConsoleMessageVerbosity::Error, FString::Printf(TEXT("%s failed: ActorOrComponent is invalid."), FunctionName));
			return false;
		}

		OutActor = Cast<AActor>(ActorOrComponent);
		OutComponent = Cast<USceneComponent>(ActorOrComponent);

		if (OutActor && bRelative)
		{
			OutComponent = OutActor->GetRootComponent();
		}

		if (!OutActor && !OutComponent)
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Transform, ELunarConsoleMessageVerbosity::Error, FString::Printf(TEXT("%s failed: ActorOrComponent is not Actor or SceneComponent."), FunctionName));
			return false;
		}

		if (OutActor && bRelative && !IsValid(OutComponent))
		{
			ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Transform, ELunarConsoleMessageVerbosity::Error, FString::Printf(TEXT("%s failed: Actor root component is invalid."), FunctionName));
			return false;
		}

		return true;
	}
}

//Transform

bool ULunarFLTransform::SetTransformAxis(UObject* ActorOrComponent, ELunarLocationRotationScale TransformType, ELunarAxisFull Axis, FVector Values, FHitResult& SweepHitResult, bool bRelative, bool bSweep, bool bTeleport)
{
	SweepHitResult = FHitResult();

	switch (TransformType)
	{
	case ELunarLocationRotationScale::Location:
		return SetLocationAxis(ActorOrComponent, Axis, Values, SweepHitResult, bRelative, bSweep, bTeleport);

	case ELunarLocationRotationScale::Rotation:
		return SetRotationAxis(ActorOrComponent, Axis, Values, SweepHitResult, bRelative, bSweep, bTeleport);

	case ELunarLocationRotationScale::Scale:
		return SetScaleAxis(ActorOrComponent, Axis, Values, bRelative);

	default:
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Transform, ELunarConsoleMessageVerbosity::Error, TEXT("SetTransformAxis failed: TransformType is invalid."));
		return false;
	}
}

bool ULunarFLTransform::AddTransformAxis(UObject* ActorOrComponent, ELunarLocationRotationScale TransformType, ELunarAxisFull Axis, FVector Values, FHitResult& SweepHitResult, bool bRelative, bool bSweep, bool bTeleport)
{
	SweepHitResult = FHitResult();

	switch (TransformType)
	{
	case ELunarLocationRotationScale::Location:
		return AddLocationAxis(ActorOrComponent, Axis, Values, SweepHitResult, bRelative, bSweep, bTeleport);

	case ELunarLocationRotationScale::Rotation:
		return AddRotationAxis(ActorOrComponent, Axis, Values, SweepHitResult, bRelative, bSweep, bTeleport);

	case ELunarLocationRotationScale::Scale:
		return AddScaleAxis(ActorOrComponent, Axis, Values, bRelative);

	default:
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Transform, ELunarConsoleMessageVerbosity::Error, TEXT("AddTransformAxis failed: TransformType is invalid."));
		return false;
	}
}

//Locations

bool ULunarFLTransform::SetLocationAxis(UObject* ActorOrComponent, ELunarAxisFull Axis, FVector Values, FHitResult& SweepHitResult, bool bRelative, bool bSweep, bool bTeleport)
{
	using namespace LunarFLTransform_Private;

	SweepHitResult = FHitResult();

	AActor* Actor = nullptr;
	USceneComponent* Component = nullptr;

	if (!ResolveActorOrComponent(ActorOrComponent, bRelative, Actor, Component, TEXT("SetLocationAxis")))
	{
		return false;
	}

	const ETeleportType TeleportType = GetTeleportType(bTeleport);

	if (Actor && !bRelative)
	{
		FVector Location = Actor->GetActorLocation();
		SetVectorAxisValues(Location, Axis, Values);
		Actor->SetActorLocation(Location, bSweep, &SweepHitResult, TeleportType);
		return true;
	}

	FVector Location = bRelative ? Component->GetRelativeLocation() : Component->GetComponentLocation();
	SetVectorAxisValues(Location, Axis, Values);

	if (bRelative)
	{
		Component->SetRelativeLocation(Location, bSweep, &SweepHitResult, TeleportType);
	}
	else
	{
		Component->SetWorldLocation(Location, bSweep, &SweepHitResult, TeleportType);
	}

	return true;
}

bool ULunarFLTransform::AddLocationAxis(UObject* ActorOrComponent, ELunarAxisFull Axis, FVector Values, FHitResult& SweepHitResult, bool bRelative, bool bSweep, bool bTeleport)
{
	using namespace LunarFLTransform_Private;

	SweepHitResult = FHitResult();

	AActor* Actor = nullptr;
	USceneComponent* Component = nullptr;

	if (!ResolveActorOrComponent(ActorOrComponent, bRelative, Actor, Component, TEXT("AddLocationAxis")))
	{
		return false;
	}

	const ETeleportType TeleportType = GetTeleportType(bTeleport);

	if (Actor && !bRelative)
	{
		FVector Location = Actor->GetActorLocation();
		AddVectorAxisValues(Location, Axis, Values);
		Actor->SetActorLocation(Location, bSweep, &SweepHitResult, TeleportType);
		return true;
	}

	FVector Location = bRelative ? Component->GetRelativeLocation() : Component->GetComponentLocation();
	AddVectorAxisValues(Location, Axis, Values);

	if (bRelative)
	{
		Component->SetRelativeLocation(Location, bSweep, &SweepHitResult, TeleportType);
	}
	else
	{
		Component->SetWorldLocation(Location, bSweep, &SweepHitResult, TeleportType);
	}

	return true;
}

//Rotations

bool ULunarFLTransform::SetRotationAxis(UObject* ActorOrComponent, ELunarAxisFull Axis, FVector Values, FHitResult& SweepHitResult, bool bRelative, bool bSweep, bool bTeleport)
{
	using namespace LunarFLTransform_Private;

	SweepHitResult = FHitResult();

	AActor* Actor = nullptr;
	USceneComponent* Component = nullptr;

	if (!ResolveActorOrComponent(ActorOrComponent, bRelative, Actor, Component, TEXT("SetRotationAxis")))
	{
		return false;
	}

	const ETeleportType TeleportType = GetTeleportType(bTeleport);

	if (Actor && !bRelative)
	{
		FRotator Rotation = Actor->GetActorRotation();
		SetRotatorAxisValues(Rotation, Axis, Values);
		Actor->SetActorRotation(Rotation, TeleportType);
		return true;
	}

	FRotator Rotation = bRelative ? Component->GetRelativeRotation() : Component->GetComponentRotation();
	SetRotatorAxisValues(Rotation, Axis, Values);

	if (bRelative)
	{
		Component->SetRelativeRotation(Rotation, bSweep, &SweepHitResult, TeleportType);
	}
	else
	{
		Component->SetWorldRotation(Rotation, bSweep, &SweepHitResult, TeleportType);
	}

	return true;
}

bool ULunarFLTransform::SetRandomRotationAxis(UObject* ActorOrComponent, ELunarAxisFull Axis, FVector Min, FVector Max, ELunarRandomQuality Quality, bool bRelative)
{
	using namespace LunarFLTransform_Private;

	AActor* Actor = nullptr;
	USceneComponent* Component = nullptr;

	if (!ResolveActorOrComponent(ActorOrComponent, bRelative, Actor, Component, TEXT("SetRandomRotationAxis")))
	{
		return false;
	}

	const FVector Values = ULunarFLRandom::GetRandomVectorInRange(ActorOrComponent, Min, Max, Quality);
	FHitResult SweepHitResult;
	return SetRotationAxis(ActorOrComponent, Axis, Values, SweepHitResult, bRelative, false, false);
}

bool ULunarFLTransform::SetRandomRotationAxisFromSeed(UObject* ActorOrComponent, ELunarAxisFull Axis, int64 Seed, FVector Min, FVector Max, ELunarRandomQuality Quality, bool bRelative)
{
	using namespace LunarFLTransform_Private;

	AActor* Actor = nullptr;
	USceneComponent* Component = nullptr;

	if (!ResolveActorOrComponent(ActorOrComponent, bRelative, Actor, Component, TEXT("SetRandomRotationAxisFromSeed")))
	{
		return false;
	}

	const FVector Values = ULunarFLRandom::GetRandomVectorInRangeFromSeed(ActorOrComponent, Min, Max, Seed, Quality);
	FHitResult SweepHitResult;
	return SetRotationAxis(ActorOrComponent, Axis, Values, SweepHitResult, bRelative, false, false);
}

bool ULunarFLTransform::SetRandomRotationAxisFromStream(UObject* ActorOrComponent, ELunarAxisFull Axis, FLunarRandomStream& Stream, FVector Min, FVector Max, bool bRelative)
{
	using namespace LunarFLTransform_Private;

	AActor* Actor = nullptr;
	USceneComponent* Component = nullptr;

	if (!ResolveActorOrComponent(ActorOrComponent, bRelative, Actor, Component, TEXT("SetRandomRotationAxisFromStream")))
	{
		return false;
	}

	const FVector Values = ULunarFLRandom::GetRandomVectorInRangeFromStream(ActorOrComponent, Stream, Min, Max);
	FHitResult SweepHitResult;
	return SetRotationAxis(ActorOrComponent, Axis, Values, SweepHitResult, bRelative, false, false);
}

bool ULunarFLTransform::AddRotationAxis(UObject* ActorOrComponent, ELunarAxisFull Axis, FVector Values, FHitResult& SweepHitResult, bool bRelative, bool bSweep, bool bTeleport)
{
	using namespace LunarFLTransform_Private;

	SweepHitResult = FHitResult();

	AActor* Actor = nullptr;
	USceneComponent* Component = nullptr;

	if (!ResolveActorOrComponent(ActorOrComponent, bRelative, Actor, Component, TEXT("AddRotationAxis")))
	{
		return false;
	}

	const ETeleportType TeleportType = GetTeleportType(bTeleport);

	if (Actor && !bRelative)
	{
		FRotator Rotation = Actor->GetActorRotation();
		AddRotatorAxisValues(Rotation, Axis, Values);
		Actor->SetActorRotation(Rotation, TeleportType);
		return true;
	}

	FRotator Rotation = bRelative ? Component->GetRelativeRotation() : Component->GetComponentRotation();
	AddRotatorAxisValues(Rotation, Axis, Values);

	if (bRelative)
	{
		Component->SetRelativeRotation(Rotation, bSweep, &SweepHitResult, TeleportType);
	}
	else
	{
		Component->SetWorldRotation(Rotation, bSweep, &SweepHitResult, TeleportType);
	}

	return true;
}

//Scales

bool ULunarFLTransform::SetScaleAxis(UObject* ActorOrComponent, ELunarAxisFull Axis, FVector Values, bool bRelative)
{
	using namespace LunarFLTransform_Private;

	AActor* Actor = nullptr;
	USceneComponent* Component = nullptr;

	if (!ResolveActorOrComponent(ActorOrComponent, bRelative, Actor, Component, TEXT("SetScaleAxis")))
	{
		return false;
	}

	if (Actor && !bRelative)
	{
		FVector Scale = Actor->GetActorScale3D();
		SetVectorAxisValues(Scale, Axis, Values);
		Actor->SetActorScale3D(Scale);
		return true;
	}

	FVector Scale = bRelative ? Component->GetRelativeScale3D() : Component->GetComponentScale();
	SetVectorAxisValues(Scale, Axis, Values);

	if (bRelative)
	{
		Component->SetRelativeScale3D(Scale);
	}
	else
	{
		Component->SetWorldScale3D(Scale);
	}

	return true;
}

bool ULunarFLTransform::AddScaleAxis(UObject* ActorOrComponent, ELunarAxisFull Axis, FVector Values, bool bRelative)
{
	using namespace LunarFLTransform_Private;

	AActor* Actor = nullptr;
	USceneComponent* Component = nullptr;

	if (!ResolveActorOrComponent(ActorOrComponent, bRelative, Actor, Component, TEXT("AddScaleAxis")))
	{
		return false;
	}

	if (Actor && !bRelative)
	{
		FVector Scale = Actor->GetActorScale3D();
		AddVectorAxisValues(Scale, Axis, Values);
		Actor->SetActorScale3D(Scale);
		return true;
	}

	FVector Scale = bRelative ? Component->GetRelativeScale3D() : Component->GetComponentScale();
	AddVectorAxisValues(Scale, Axis, Values);

	if (bRelative)
	{
		Component->SetRelativeScale3D(Scale);
	}
	else
	{
		Component->SetWorldScale3D(Scale);
	}

	return true;
}
