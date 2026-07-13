// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "LunarFLDebug.h"

#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "NativeGameplayTags.h"
#include "Subsystems/Console/LunarConsoleSubsystem.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Lunar_Debug, "Lunar.Debug");

FString ULunarFLDebug::GetObjectDebugInfo(const UObject* Object)
{
	if (!IsValid(Object))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("GetObjectDebugInfo failed because Object is invalid"));
		return TEXT("Object: Invalid");
	}

	return FString::Printf(TEXT("Name: %s | Class: %s | Path: %s | Outer: %s"), *Object->GetName(), *GetNameSafe(Object->GetClass()), *Object->GetPathName(), *GetNameSafe(Object->GetOuter()));
}

FString ULunarFLDebug::GetActorDebugInfo(const AActor* Actor)
{
	if (!IsValid(Actor))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("GetActorDebugInfo failed because Actor is invalid"));
		return TEXT("Actor: Invalid");
	}

	const FVector Location = Actor->GetActorLocation();
	const FRotator Rotation = Actor->GetActorRotation();
	const FVector Scale = Actor->GetActorScale3D();
	const FVector Velocity = Actor->GetVelocity();

	return FString::Printf(TEXT("Name: %s | Class: %s | Location: X=%.2f Y=%.2f Z=%.2f | Rotation: P=%.2f Y=%.2f R=%.2f | Scale: X=%.2f Y=%.2f Z=%.2f | Velocity: X=%.2f Y=%.2f Z=%.2f | Hidden: %s | Tick: %s"), *Actor->GetName(), *GetNameSafe(Actor->GetClass()), Location.X, Location.Y, Location.Z, Rotation.Pitch, Rotation.Yaw, Rotation.Roll, Scale.X, Scale.Y, Scale.Z, Velocity.X, Velocity.Y, Velocity.Z, Actor->IsHidden() ? TEXT("True") : TEXT("False"), Actor->IsActorTickEnabled() ? TEXT("Enabled") : TEXT("Disabled"));
}

FString ULunarFLDebug::GetComponentDebugInfo(const UActorComponent* Component)
{
	if (!IsValid(Component))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("GetComponentDebugInfo failed because Component is invalid"));
		return TEXT("Component: Invalid");
	}

	FString TransformInformation = TEXT("Transform: Unavailable");

	if (const USceneComponent* SceneComponent = Cast<USceneComponent>(Component))
	{
		const FVector Location = SceneComponent->GetComponentLocation();
		const FRotator Rotation = SceneComponent->GetComponentRotation();
		const FVector Scale = SceneComponent->GetComponentScale();

		TransformInformation = FString::Printf(TEXT("Location: X=%.2f Y=%.2f Z=%.2f | Rotation: P=%.2f Y=%.2f R=%.2f | Scale: X=%.2f Y=%.2f Z=%.2f"), Location.X, Location.Y, Location.Z, Rotation.Pitch, Rotation.Yaw, Rotation.Roll, Scale.X, Scale.Y, Scale.Z);
	}

	return FString::Printf(TEXT("Name: %s | Class: %s | Owner: %s | Registered: %s | Active: %s | Tick: %s | %s"), *Component->GetName(), *GetNameSafe(Component->GetClass()), *GetNameSafe(Component->GetOwner()), Component->IsRegistered() ? TEXT("True") : TEXT("False"), Component->IsActive() ? TEXT("True") : TEXT("False"), Component->IsComponentTickEnabled() ? TEXT("Enabled") : TEXT("Disabled"), *TransformInformation);
}

FString ULunarFLDebug::GetWorldDebugInfo(const UObject* WorldContextObject)
{
	if (!IsValid(WorldContextObject))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("GetWorldDebugInfo failed because WorldContextObject is invalid"));
		return TEXT("World: Invalid");
	}

	if (!GEngine)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("GetWorldDebugInfo failed because GEngine is invalid"));
		return TEXT("World: Invalid");
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);

	if (!IsValid(World))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("GetWorldDebugInfo failed because World could not be resolved"));
		return TEXT("World: Invalid");
	}

	FString WorldType;

	switch (World->WorldType)
	{
	case EWorldType::None:
		WorldType = TEXT("None");
		break;

	case EWorldType::Game:
		WorldType = TEXT("Game");
		break;

	case EWorldType::Editor:
		WorldType = TEXT("Editor");
		break;

	case EWorldType::PIE:
		WorldType = TEXT("PIE");
		break;

	case EWorldType::EditorPreview:
		WorldType = TEXT("Editor Preview");
		break;

	case EWorldType::GamePreview:
		WorldType = TEXT("Game Preview");
		break;

	case EWorldType::GameRPC:
		WorldType = TEXT("Game RPC");
		break;

	case EWorldType::Inactive:
		WorldType = TEXT("Inactive");
		break;

	default:
		WorldType = TEXT("Unknown");
		break;
	}

	FString NetMode;

	switch (World->GetNetMode())
	{
	case NM_Standalone:
		NetMode = TEXT("Standalone");
		break;

	case NM_DedicatedServer:
		NetMode = TEXT("Dedicated Server");
		break;

	case NM_ListenServer:
		NetMode = TEXT("Listen Server");
		break;

	case NM_Client:
		NetMode = TEXT("Client");
		break;

	default:
		NetMode = TEXT("Unknown");
		break;
	}

	return FString::Printf(TEXT("Name: %s | Map: %s | Type: %s | Net Mode: %s | Time: %.2f | Real Time: %.2f | Delta Seconds: %.4f | Paused: %s"), *World->GetName(), *World->GetMapName(), *WorldType, *NetMode, World->GetTimeSeconds(), World->GetRealTimeSeconds(), World->GetDeltaSeconds(), World->IsPaused() ? TEXT("True") : TEXT("False"));
}

FString ULunarFLDebug::GetActorNetworkDebugInfo(const AActor* Actor)
{
	if (!IsValid(Actor))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("GetActorNetworkDebugInfo failed because Actor is invalid"));
		return TEXT("Actor Network: Invalid");
	}

	const UWorld* World = Actor->GetWorld();

	if (!IsValid(World))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("GetActorNetworkDebugInfo failed because Actor World is invalid"));
		return TEXT("Actor Network: Invalid");
	}

	FString NetMode;

	switch (World->GetNetMode())
	{
	case NM_Standalone:
		NetMode = TEXT("Standalone");
		break;

	case NM_DedicatedServer:
		NetMode = TEXT("Dedicated Server");
		break;

	case NM_ListenServer:
		NetMode = TEXT("Listen Server");
		break;

	case NM_Client:
		NetMode = TEXT("Client");
		break;

	default:
		NetMode = TEXT("Unknown");
		break;
	}

	const FString LocalRole = UEnum::GetValueAsString(Actor->GetLocalRole());
	const FString RemoteRole = UEnum::GetValueAsString(Actor->GetRemoteRole());

	return FString::Printf(TEXT("Name: %s | Net Mode: %s | Local Role: %s | Remote Role: %s | Authority: %s | Replicated: %s | Replicate Movement: %s | Net Startup Actor: %s"), *Actor->GetName(), *NetMode, *LocalRole, *RemoteRole, Actor->HasAuthority() ? TEXT("True") : TEXT("False"), Actor->GetIsReplicated() ? TEXT("True") : TEXT("False"), Actor->IsReplicatingMovement() ? TEXT("True") : TEXT("False"), Actor->IsNetStartupActor() ? TEXT("True") : TEXT("False"));
}

void ULunarFLDebug::DrawDebugPointWithLabel(const UObject* WorldContextObject, FVector Location, const FString& Label, float PointSize, FLinearColor Color, float Duration, bool bPersistentLines)
{
	if (!IsValid(WorldContextObject))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugPointWithLabel failed because WorldContextObject is invalid"));
		return;
	}

	if (!GEngine)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugPointWithLabel failed because GEngine is invalid"));
		return;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);

	if (!IsValid(World))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugPointWithLabel failed because World could not be resolved"));
		return;
	}

	if (PointSize <= 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugPointWithLabel failed because PointSize must be greater than zero"));
		return;
	}

	if (Duration < 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugPointWithLabel failed because Duration cannot be negative"));
		return;
	}

	const FColor DebugColor = Color.ToFColor(true);

	DrawDebugPoint(World, Location, PointSize, DebugColor, bPersistentLines, Duration);

	if (!Label.IsEmpty())
	{
		DrawDebugString(World, Location + FVector(0.0f, 0.0f, PointSize), Label, nullptr, DebugColor, Duration, true);
	}
}

void ULunarFLDebug::DrawDebugLineWithLabel(const UObject* WorldContextObject, FVector Start, FVector End, const FString& Label, FLinearColor Color, float Thickness, float Duration, bool bPersistentLines)
{
	if (!IsValid(WorldContextObject))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugLineWithLabel failed because WorldContextObject is invalid"));
		return;
	}

	if (!GEngine)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugLineWithLabel failed because GEngine is invalid"));
		return;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);

	if (!IsValid(World))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugLineWithLabel failed because World could not be resolved"));
		return;
	}

	if (Thickness < 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugLineWithLabel failed because Thickness cannot be negative"));
		return;
	}

	if (Duration < 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugLineWithLabel failed because Duration cannot be negative"));
		return;
	}

	const FColor DebugColor = Color.ToFColor(true);

	DrawDebugLine(World, Start, End, DebugColor, bPersistentLines, Duration, 0, Thickness);

	if (!Label.IsEmpty())
	{
		DrawDebugString(World, FMath::Lerp(Start, End, 0.5f), Label, nullptr, DebugColor, Duration, true);
	}
}

void ULunarFLDebug::DrawDebugArrowWithLabel(const UObject* WorldContextObject, FVector Start, FVector End, const FString& Label, float ArrowSize, FLinearColor Color, float Thickness, float Duration, bool bPersistentLines)
{
	if (!IsValid(WorldContextObject))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugArrowWithLabel failed because WorldContextObject is invalid"));
		return;
	}

	if (!GEngine)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugArrowWithLabel failed because GEngine is invalid"));
		return;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);

	if (!IsValid(World))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugArrowWithLabel failed because World could not be resolved"));
		return;
	}

	if (ArrowSize <= 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugArrowWithLabel failed because ArrowSize must be greater than zero"));
		return;
	}

	if (Thickness < 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugArrowWithLabel failed because Thickness cannot be negative"));
		return;
	}

	if (Duration < 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugArrowWithLabel failed because Duration cannot be negative"));
		return;
	}

	const FColor DebugColor = Color.ToFColor(true);

	DrawDebugDirectionalArrow(World, Start, End, ArrowSize, DebugColor, bPersistentLines, Duration, 0, Thickness);

	if (!Label.IsEmpty())
	{
		DrawDebugString(World, End, Label, nullptr, DebugColor, Duration, true);
	}
}

void ULunarFLDebug::DrawActorTransform(const AActor* Actor, float AxisLength, float Thickness, float Duration, bool bPersistentLines)
{
	if (!IsValid(Actor))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawActorTransform failed because Actor is invalid"));
		return;
	}

	UWorld* World = Actor->GetWorld();

	if (!IsValid(World))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawActorTransform failed because Actor World is invalid"));
		return;
	}

	if (AxisLength <= 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawActorTransform failed because AxisLength must be greater than zero"));
		return;
	}

	if (Thickness < 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawActorTransform failed because Thickness cannot be negative"));
		return;
	}

	if (Duration < 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawActorTransform failed because Duration cannot be negative"));
		return;
	}

	const FTransform Transform = Actor->GetActorTransform();
	const FVector Location = Transform.GetLocation();

	DrawDebugLine(World, Location, Location + Transform.GetUnitAxis(EAxis::X) * AxisLength, FColor::Red, bPersistentLines, Duration, 0, Thickness);
	DrawDebugLine(World, Location, Location + Transform.GetUnitAxis(EAxis::Y) * AxisLength, FColor::Green, bPersistentLines, Duration, 0, Thickness);
	DrawDebugLine(World, Location, Location + Transform.GetUnitAxis(EAxis::Z) * AxisLength, FColor::Blue, bPersistentLines, Duration, 0, Thickness);
}

void ULunarFLDebug::DrawComponentTransform(const USceneComponent* Component, float AxisLength, float Thickness, float Duration, bool bPersistentLines)
{
	if (!IsValid(Component))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawComponentTransform failed because Component is invalid"));
		return;
	}

	UWorld* World = Component->GetWorld();

	if (!IsValid(World))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawComponentTransform failed because Component World is invalid"));
		return;
	}

	if (AxisLength <= 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawComponentTransform failed because AxisLength must be greater than zero"));
		return;
	}

	if (Thickness < 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawComponentTransform failed because Thickness cannot be negative"));
		return;
	}

	if (Duration < 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawComponentTransform failed because Duration cannot be negative"));
		return;
	}

	const FTransform Transform = Component->GetComponentTransform();
	const FVector Location = Transform.GetLocation();

	DrawDebugLine(World, Location, Location + Transform.GetUnitAxis(EAxis::X) * AxisLength, FColor::Red, bPersistentLines, Duration, 0, Thickness);
	DrawDebugLine(World, Location, Location + Transform.GetUnitAxis(EAxis::Y) * AxisLength, FColor::Green, bPersistentLines, Duration, 0, Thickness);
	DrawDebugLine(World, Location, Location + Transform.GetUnitAxis(EAxis::Z) * AxisLength, FColor::Blue, bPersistentLines, Duration, 0, Thickness);
}

void ULunarFLDebug::DrawDebugFieldOfView(const UObject* WorldContextObject, FVector Location, FRotator Rotation, float Distance, float HorizontalAngle, float VerticalAngle, FLinearColor Color, float Thickness, float Duration, bool bPersistentLines)
{
	if (!IsValid(WorldContextObject))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugFieldOfView failed because WorldContextObject is invalid"));
		return;
	}

	if (!GEngine)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugFieldOfView failed because GEngine is invalid"));
		return;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);

	if (!IsValid(World))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugFieldOfView failed because World could not be resolved"));
		return;
	}

	if (Distance <= 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugFieldOfView failed because Distance must be greater than zero"));
		return;
	}

	if (HorizontalAngle <= 0.0f || HorizontalAngle >= 360.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugFieldOfView failed because HorizontalAngle must be greater than zero and less than 360"));
		return;
	}

	if (VerticalAngle <= 0.0f || VerticalAngle >= 360.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugFieldOfView failed because VerticalAngle must be greater than zero and less than 360"));
		return;
	}

	if (Thickness < 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugFieldOfView failed because Thickness cannot be negative"));
		return;
	}

	if (Duration < 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugFieldOfView failed because Duration cannot be negative"));
		return;
	}

	const FVector Forward = Rotation.Vector();

	DrawDebugCone(World, Location, Forward, Distance, FMath::DegreesToRadians(HorizontalAngle * 0.5f), FMath::DegreesToRadians(VerticalAngle * 0.5f), 32, Color.ToFColor(true), bPersistentLines, Duration, 0, Thickness);
}

void ULunarFLDebug::DrawActorVelocity(const AActor* Actor, float Scale, float ArrowSize, FLinearColor Color, float Thickness, float Duration, bool bPersistentLines)
{
	if (!IsValid(Actor))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawActorVelocity failed because Actor is invalid"));
		return;
	}

	UWorld* World = Actor->GetWorld();

	if (!IsValid(World))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawActorVelocity failed because Actor World is invalid"));
		return;
	}

	if (!FMath::IsFinite(Scale))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawActorVelocity failed because Scale is not finite"));
		return;
	}

	if (ArrowSize <= 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawActorVelocity failed because ArrowSize must be greater than zero"));
		return;
	}

	if (Thickness < 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawActorVelocity failed because Thickness cannot be negative"));
		return;
	}

	if (Duration < 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawActorVelocity failed because Duration cannot be negative"));
		return;
	}

	const FVector Start = Actor->GetActorLocation();
	const FVector End = Start + Actor->GetVelocity() * Scale;

	DrawDebugDirectionalArrow(World, Start, End, ArrowSize, Color.ToFColor(true), bPersistentLines, Duration, 0, Thickness);
}

void ULunarFLDebug::DrawDebugPath(const UObject* WorldContextObject, const TArray<FVector>& Points, bool bClosed, bool bDrawPointIndices, FLinearColor Color, float PointSize, float Thickness, float Duration, bool bPersistentLines)
{
	if (!IsValid(WorldContextObject))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugPath failed because WorldContextObject is invalid"));
		return;
	}

	if (!GEngine)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugPath failed because GEngine is invalid"));
		return;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);

	if (!IsValid(World))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugPath failed because World could not be resolved"));
		return;
	}

	if (Points.IsEmpty())
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugPath failed because Points is empty"));
		return;
	}

	if (PointSize <= 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugPath failed because PointSize must be greater than zero"));
		return;
	}

	if (Thickness < 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugPath failed because Thickness cannot be negative"));
		return;
	}

	if (Duration < 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugPath failed because Duration cannot be negative"));
		return;
	}

	const FColor DebugColor = Color.ToFColor(true);

	for (int32 Index = 0; Index < Points.Num(); ++Index)
	{
		DrawDebugPoint(World, Points[Index], PointSize, DebugColor, bPersistentLines, Duration);

		if (bDrawPointIndices)
		{
			DrawDebugString(World, Points[Index] + FVector(0.0f, 0.0f, PointSize), FString::FromInt(Index), nullptr, DebugColor, Duration, true);
		}

		if (Index < Points.Num() - 1)
		{
			DrawDebugLine(World, Points[Index], Points[Index + 1], DebugColor, bPersistentLines, Duration, 0, Thickness);
		}
	}

	if (bClosed && Points.Num() > 2)
	{
		DrawDebugLine(World, Points.Last(), Points[0], DebugColor, bPersistentLines, Duration, 0, Thickness);
	}
}

void ULunarFLDebug::DrawDebugPoints(const UObject* WorldContextObject, const TArray<FVector>& Points, float Radius, FLinearColor Color, float Duration, bool bPersistentLines)
{
	if (!IsValid(WorldContextObject))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugPoints failed because WorldContextObject is invalid"));
		return;
	}

	if (!GEngine)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugPoints failed because GEngine is invalid"));
		return;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);

	if (!IsValid(World))
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugPoints failed because World could not be resolved"));
		return;
	}

	if (Points.IsEmpty())
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugPoints failed because Points is empty"));
		return;
	}

	if (Radius <= 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugPoints failed because Radius must be greater than zero"));
		return;
	}

	if (Duration < 0.0f)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("DrawDebugPoints failed because Duration cannot be negative"));
		return;
	}

	const FColor DebugColor = Color.ToFColor(true);

	for (const FVector& Point : Points)
	{
		DrawDebugSphere(World, Point, Radius, 12, DebugColor, bPersistentLines, Duration);
	}
}

FString ULunarFLDebug::TransformToDebugString(const FTransform& Transform, int32 Precision)
{
	if (Precision < 0 || Precision > 9)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("TransformToDebugString failed because Precision must be between zero and nine"));
		return FString();
	}

	const FVector Location = Transform.GetLocation();
	const FRotator Rotation = Transform.Rotator();
	const FVector Scale = Transform.GetScale3D();

	return FString::Printf(TEXT("Location: X=%.*f Y=%.*f Z=%.*f | Rotation: P=%.*f Y=%.*f R=%.*f | Scale: X=%.*f Y=%.*f Z=%.*f"), Precision, Location.X, Precision, Location.Y, Precision, Location.Z, Precision, Rotation.Pitch, Precision, Rotation.Yaw, Precision, Rotation.Roll, Precision, Scale.X, Precision, Scale.Y, Precision, Scale.Z);
}

FString ULunarFLDebug::VectorToDebugString(FVector Vector, int32 Precision)
{
	if (Precision < 0 || Precision > 9)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("VectorToDebugString failed because Precision must be between zero and nine"));
		return FString();
	}

	return FString::Printf(TEXT("X=%.*f Y=%.*f Z=%.*f"), Precision, Vector.X, Precision, Vector.Y, Precision, Vector.Z);
}

FString ULunarFLDebug::RotatorToDebugString(FRotator Rotator, int32 Precision)
{
	if (Precision < 0 || Precision > 9)
	{
		ULunarConsoleSubsystem::AddMessage(TAG_Lunar_Debug, ELunarConsoleMessageVerbosity::Error, TEXT("RotatorToDebugString failed because Precision must be between zero and nine"));
		return FString();
	}

	return FString::Printf(TEXT("Pitch=%.*f Yaw=%.*f Roll=%.*f"), Precision, Rotator.Pitch, Precision, Rotator.Yaw, Precision, Rotator.Roll);
}

FString ULunarFLDebug::ObjectArrayToDebugString(const TArray<UObject*>& Objects, bool bIncludeClassNames)
{
	if (Objects.IsEmpty())
	{
		return TEXT("[]");
	}

	TArray<FString> ObjectStrings;
	ObjectStrings.Reserve(Objects.Num());

	for (int32 Index = 0; Index < Objects.Num(); ++Index)
	{
		const UObject* Object = Objects[Index];

		if (!IsValid(Object))
		{
			ObjectStrings.Add(FString::Printf(TEXT("[%d] Invalid"), Index));
			continue;
		}

		ObjectStrings.Add(bIncludeClassNames ? FString::Printf(TEXT("[%d] %s (%s)"), Index, *Object->GetName(), *GetNameSafe(Object->GetClass())) : FString::Printf(TEXT("[%d] %s"), Index, *Object->GetName()));
	}

	return FString::Printf(TEXT("[%s]"), *FString::Join(ObjectStrings, TEXT(", ")));
}

void ULunarFLDebug::TriggerDebugBreak(bool bCondition)
{
	if (bCondition)
	{
		UE_DEBUG_BREAK();
	}
}
