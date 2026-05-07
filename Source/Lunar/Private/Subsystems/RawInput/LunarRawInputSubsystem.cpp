// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "Subsystems/LunarRawInputSubsystem.h"

#include "Framework/Application/SlateApplication.h"
#include "Subsystems/RawInput/LunarRawInputProcessor.h"

void ULunarRawInputSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LastInputTimeSeconds = FPlatformTime::Seconds();

	if (FSlateApplication::IsInitialized())
	{
		InputProcessor = MakeShared<FLunarRawInputProcessor>(this);
		FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor);
	}

	bInitialized = true;
}

void ULunarRawInputSubsystem::Deinitialize()
{
	if (FSlateApplication::IsInitialized() && InputProcessor.IsValid())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor);
		InputProcessor.Reset();
	}

	ClearInputState();

	bInitialized = false;

	Super::Deinitialize();
}

void ULunarRawInputSubsystem::Tick(float DeltaTime)
{
	const double CurrentTimeSeconds = FPlatformTime::Seconds();

	Snapshot.SecondsSinceLastInput = static_cast<float>(CurrentTimeSeconds - LastInputTimeSeconds);

	Snapshot.MouseDelta = PendingMouseDelta;
	Snapshot.RawMouseDelta = PendingRawMouseDelta;
	Snapshot.MouseWheelDelta = PendingMouseWheelDelta;

	PendingMouseDelta = FVector2D::ZeroVector;
	PendingRawMouseDelta = FVector2D::ZeroVector;
	PendingMouseWheelDelta = 0.0f;

	PressedThisFrameKeys.Reset();
	ReleasedThisFrameKeys.Reset();
}

TStatId ULunarRawInputSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULunarRawInputSubsystem, STATGROUP_Tickables);
}

bool ULunarRawInputSubsystem::IsTickable() const
{
	return !HasAnyFlags(RF_ClassDefaultObject) && bInitialized;
}

const FLunarRawInputSnapshot& ULunarRawInputSubsystem::GetSnapshot() const
{
	return Snapshot;
}

ELunarInputDeviceType ULunarRawInputSubsystem::GetLastInputDevice() const
{
	return Snapshot.LastInputDevice;
}

bool ULunarRawInputSubsystem::IsUsingGamepad() const
{
	return Snapshot.LastInputDevice == ELunarInputDeviceType::Gamepad;
}

bool ULunarRawInputSubsystem::IsUsingKeyboardMouse() const
{
	return Snapshot.LastInputDevice == ELunarInputDeviceType::KeyboardMouse;
}

bool ULunarRawInputSubsystem::IsKeyDown(FKey Key) const
{
	return DownKeys.Contains(Key);
}

bool ULunarRawInputSubsystem::WasKeyPressedThisFrame(FKey Key) const
{
	return PressedThisFrameKeys.Contains(Key);
}

bool ULunarRawInputSubsystem::WasKeyReleasedThisFrame(FKey Key) const
{
	return ReleasedThisFrameKeys.Contains(Key);
}

TArray<FKey> ULunarRawInputSubsystem::GetDownKeys() const
{
	return DownKeys.Array();
}

TArray<FKey> ULunarRawInputSubsystem::GetPressedKeysThisFrame() const
{
	return PressedThisFrameKeys.Array();
}

TArray<FKey> ULunarRawInputSubsystem::GetReleasedKeysThisFrame() const
{
	return ReleasedThisFrameKeys.Array();
}

FVector2D ULunarRawInputSubsystem::GetMousePosition() const
{
	return Snapshot.MousePosition;
}

FVector2D ULunarRawInputSubsystem::GetMouseDelta() const
{
	return Snapshot.MouseDelta;
}

FVector2D ULunarRawInputSubsystem::GetRawMouseDelta() const
{
	return Snapshot.RawMouseDelta;
}

float ULunarRawInputSubsystem::GetMouseWheelDelta() const
{
	return Snapshot.MouseWheelDelta;
}

void ULunarRawInputSubsystem::ClearInputState()
{
	DownKeys.Reset();
	PressedThisFrameKeys.Reset();
	ReleasedThisFrameKeys.Reset();

	PendingMouseDelta = FVector2D::ZeroVector;
	PendingRawMouseDelta = FVector2D::ZeroVector;
	PendingMouseWheelDelta = 0.0f;

	Snapshot.MouseDelta = FVector2D::ZeroVector;
	Snapshot.RawMouseDelta = FVector2D::ZeroVector;
	Snapshot.MouseWheelDelta = 0.0f;
	Snapshot.LastKey = EKeys::Invalid;

	PreviousMousePosition = FVector2D::ZeroVector;
	bHasPreviousMousePosition = false;
}

void ULunarRawInputSubsystem::HandleKeyDown(const FKey& Key)
{
	const ELunarInputDeviceType InputDevice = Key.IsGamepadKey()
		? ELunarInputDeviceType::Gamepad
		: ELunarInputDeviceType::KeyboardMouse;

	MarkInputReceived(InputDevice, Key);

	if (!DownKeys.Contains(Key))
	{
		PressedThisFrameKeys.Add(Key);
		OnKeyPressed.Broadcast(Key);
	}

	DownKeys.Add(Key);
}

void ULunarRawInputSubsystem::HandleKeyUp(const FKey& Key)
{
	const ELunarInputDeviceType InputDevice = Key.IsGamepadKey()
		? ELunarInputDeviceType::Gamepad
		: ELunarInputDeviceType::KeyboardMouse;

	MarkInputReceived(InputDevice, Key);

	const bool bWasDown = DownKeys.Contains(Key);

	if (bWasDown)
	{
		ReleasedThisFrameKeys.Add(Key);

		OnKeyReleased.Broadcast(Key);
		OnKeyClicked.Broadcast(Key);
	}

	DownKeys.Remove(Key);
}

void ULunarRawInputSubsystem::HandleMouseMove(const FVector2D& ScreenPosition, const FVector2D& CursorDelta)
{
	FVector2D RawDelta = FVector2D::ZeroVector;

	if (bHasPreviousMousePosition)
	{
		RawDelta = ScreenPosition - PreviousMousePosition;
	}
	else
	{
		bHasPreviousMousePosition = true;
	}

	PreviousMousePosition = ScreenPosition;

	Snapshot.MousePosition = ScreenPosition;

	PendingMouseDelta += CursorDelta;
	PendingRawMouseDelta += RawDelta;

	if (!CursorDelta.IsNearlyZero() || !RawDelta.IsNearlyZero())
	{
		MarkInputReceived(ELunarInputDeviceType::KeyboardMouse);
		OnMouseMoved.Broadcast(ScreenPosition, RawDelta);
	}
}

void ULunarRawInputSubsystem::HandleMouseButtonDown(const FKey& Key, const FVector2D& ScreenPosition)
{
	Snapshot.MousePosition = ScreenPosition;
	PreviousMousePosition = ScreenPosition;
	bHasPreviousMousePosition = true;

	HandleKeyDown(Key);
}

void ULunarRawInputSubsystem::HandleMouseButtonUp(const FKey& Key, const FVector2D& ScreenPosition)
{
	Snapshot.MousePosition = ScreenPosition;
	PreviousMousePosition = ScreenPosition;
	bHasPreviousMousePosition = true;

	HandleKeyUp(Key);
}

void ULunarRawInputSubsystem::HandleMouseWheel(float WheelDelta, const FVector2D& ScreenPosition)
{
	Snapshot.MousePosition = ScreenPosition;
	PreviousMousePosition = ScreenPosition;
	bHasPreviousMousePosition = true;

	PendingMouseWheelDelta += WheelDelta;

	if (!FMath::IsNearlyZero(WheelDelta))
	{
		MarkInputReceived(ELunarInputDeviceType::KeyboardMouse);
		OnMouseWheel.Broadcast(WheelDelta, ScreenPosition);
	}
}

void ULunarRawInputSubsystem::SetLastInputDevice(ELunarInputDeviceType NewInputDevice)
{
	if (Snapshot.LastInputDevice == NewInputDevice)
	{
		return;
	}

	Snapshot.LastInputDevice = NewInputDevice;
	OnInputDeviceChanged.Broadcast(NewInputDevice);
}

void ULunarRawInputSubsystem::MarkInputReceived(ELunarInputDeviceType InputDevice, const FKey& Key)
{
	LastInputTimeSeconds = FPlatformTime::Seconds();

	Snapshot.LastKey = Key;

	SetLastInputDevice(InputDevice);
	OnAnyInput.Broadcast(InputDevice, Key);
}