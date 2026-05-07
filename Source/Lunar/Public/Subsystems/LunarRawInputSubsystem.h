// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "Types/LunarRawInputTypes.h"
#include "LunarRawInputSubsystem.generated.h"

class FLunarRawInputProcessor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLunarInputDeviceChangedSignature, ELunarInputDeviceType, NewInputDevice);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLunarRawKeySignature, FKey, Key);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLunarRawMouseMovedSignature, FVector2D, MousePosition, FVector2D, MouseDelta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLunarRawAnyInputSignature, ELunarInputDeviceType, InputDevice, FKey, Key);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLunarRawMouseWheelSignature, float, WheelDelta, FVector2D, MousePosition);

UCLASS()
class LUNAR_API ULunarRawInputSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Lunar|Input")
	FLunarInputDeviceChangedSignature OnInputDeviceChanged;

	UPROPERTY(BlueprintAssignable, Category = "Lunar|Input")
	FLunarRawKeySignature OnKeyPressed;

	UPROPERTY(BlueprintAssignable, Category = "Lunar|Input")
	FLunarRawKeySignature OnKeyReleased;

	UPROPERTY(BlueprintAssignable, Category = "Lunar|Input")
	FLunarRawKeySignature OnKeyClicked;

	UPROPERTY(BlueprintAssignable, Category = "Lunar|Input")
	FLunarRawMouseMovedSignature OnMouseMoved;

	UPROPERTY(BlueprintAssignable, Category = "Lunar|Input")
	FLunarRawAnyInputSignature OnAnyInput;

	UPROPERTY(BlueprintAssignable, Category = "Lunar|Input")
	FLunarRawMouseWheelSignature OnMouseWheel;

public:
	UFUNCTION(BlueprintPure, Category = "Lunar|Input")
	const FLunarRawInputSnapshot& GetSnapshot() const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Input")
	ELunarInputDeviceType GetLastInputDevice() const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Input")
	bool IsUsingGamepad() const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Input")
	bool IsUsingKeyboardMouse() const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Input")
	bool IsKeyDown(FKey Key) const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Input")
	bool WasKeyPressedThisFrame(FKey Key) const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Input")
	bool WasKeyReleasedThisFrame(FKey Key) const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Input")
	TArray<FKey> GetDownKeys() const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Input")
	TArray<FKey> GetPressedKeysThisFrame() const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Input")
	TArray<FKey> GetReleasedKeysThisFrame() const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Input")
	FVector2D GetMousePosition() const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Input")
	FVector2D GetMouseDelta() const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Input")
	FVector2D GetRawMouseDelta() const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Input")
	float GetMouseWheelDelta() const;

	UFUNCTION(BlueprintCallable, Category = "Lunar|Input")
	void ClearInputState();

public:
	void HandleKeyDown(const FKey& Key);
	void HandleKeyUp(const FKey& Key);
	void HandleMouseMove(const FVector2D& ScreenPosition, const FVector2D& CursorDelta);
	void HandleMouseButtonDown(const FKey& Key, const FVector2D& ScreenPosition);
	void HandleMouseButtonUp(const FKey& Key, const FVector2D& ScreenPosition);
	void HandleMouseWheel(float WheelDelta, const FVector2D& ScreenPosition);

private:
	void SetLastInputDevice(ELunarInputDeviceType NewInputDevice);
	void MarkInputReceived(ELunarInputDeviceType InputDevice, const FKey& Key = EKeys::Invalid);

private:
	FLunarRawInputSnapshot Snapshot;

	FVector2D PendingMouseDelta = FVector2D::ZeroVector;
	FVector2D PendingRawMouseDelta = FVector2D::ZeroVector;
	FVector2D PreviousMousePosition = FVector2D::ZeroVector;

	float PendingMouseWheelDelta = 0.0f;

	TSet<FKey> DownKeys;
	TSet<FKey> PressedThisFrameKeys;
	TSet<FKey> ReleasedThisFrameKeys;

	TSharedPtr<FLunarRawInputProcessor> InputProcessor;

	double LastInputTimeSeconds = 0.0;

	bool bInitialized = false;
	bool bHasPreviousMousePosition = false;
};