// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "Subsystems/RawInput/LunarRawInputSubsystem.h"

#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Application/SlateApplication.h"
#include "Input/Events.h"
#include "Subsystems/RawInput/LunarRawInputProcessor.h"
#include "UI/Navigation/Core/LunarNavigationSubsystem.h"

/**
 * @file LunarRawInputSubsystem.cpp
 * @brief Raw Slate input capture, player attribution, and Lunar navigation routing
 * @ingroup LunarRawInputSubsystem
 */

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
	Snapshot.LastLocalPlayerIndex = INDEX_NONE;
	Snapshot.LastSlateUserIndex = INDEX_NONE;

	PreviousMousePosition = FVector2D::ZeroVector;
	bHasPreviousMousePosition = false;
}

bool ULunarRawInputSubsystem::HandleKeyDown(const FKeyEvent& KeyEvent)
{
	ULocalPlayer* LocalPlayer = ResolveOwningLocalPlayer(KeyEvent);
	RecordInputOwner(LocalPlayer, KeyEvent.GetUserIndex());
	HandleKeyDown(KeyEvent.GetKey());

	if (LocalPlayer)
	{
		if (ULunarNavigationSubsystem* NavigationSubsystem = LocalPlayer->GetSubsystem<ULunarNavigationSubsystem>())
		{
			return NavigationSubsystem->HandleNavigationKeyDown(KeyEvent);
		}
	}

	return false;
}

bool ULunarRawInputSubsystem::HandleKeyUp(const FKeyEvent& KeyEvent)
{
	ULocalPlayer* LocalPlayer = ResolveOwningLocalPlayer(KeyEvent);
	RecordInputOwner(LocalPlayer, KeyEvent.GetUserIndex());
	HandleKeyUp(KeyEvent.GetKey());

	if (LocalPlayer)
	{
		if (ULunarNavigationSubsystem* NavigationSubsystem = LocalPlayer->GetSubsystem<ULunarNavigationSubsystem>())
		{
			return NavigationSubsystem->HandleNavigationKeyUp(KeyEvent);
		}
	}

	return false;
}

bool ULunarRawInputSubsystem::HandleAnalogInput(const FAnalogInputEvent& AnalogEvent)
{
	const FKey Key = AnalogEvent.GetKey();
	const ELunarInputDeviceType InputDevice = Key.IsGamepadKey()
		? ELunarInputDeviceType::Gamepad
		: ELunarInputDeviceType::KeyboardMouse;

	ULocalPlayer* LocalPlayer = ResolveOwningLocalPlayer(AnalogEvent);
	RecordInputOwner(LocalPlayer, AnalogEvent.GetUserIndex());
	MarkInputReceived(InputDevice, Key);

	if (LocalPlayer)
	{
		if (ULunarNavigationSubsystem* NavigationSubsystem = LocalPlayer->GetSubsystem<ULunarNavigationSubsystem>())
		{
			return NavigationSubsystem->HandleNavigationAnalog(AnalogEvent);
		}
	}

	return false;
}

void ULunarRawInputSubsystem::HandleMouseMove(const FPointerEvent& PointerEvent)
{
	RecordInputOwner(ResolveOwningLocalPlayer(PointerEvent), PointerEvent.GetUserIndex());
	const ELunarInputDeviceType InputDevice = PointerEvent.IsTouchEvent()
		? ELunarInputDeviceType::Touch
		: ELunarInputDeviceType::KeyboardMouse;
	HandleMouseMove(PointerEvent.GetScreenSpacePosition(), PointerEvent.GetCursorDelta(), InputDevice);

	if (!PointerEvent.GetCursorDelta().IsNearlyZero())
	{
		NotifyPointerInput(PointerEvent);
	}
}

void ULunarRawInputSubsystem::HandleMouseButtonDown(const FPointerEvent& PointerEvent)
{
	RecordInputOwner(ResolveOwningLocalPlayer(PointerEvent), PointerEvent.GetUserIndex());
	const ELunarInputDeviceType InputDevice = PointerEvent.IsTouchEvent()
		? ELunarInputDeviceType::Touch
		: ELunarInputDeviceType::KeyboardMouse;
	HandleMouseButtonDown(PointerEvent.GetEffectingButton(), PointerEvent.GetScreenSpacePosition(), InputDevice);
	NotifyPointerInput(PointerEvent);
}

void ULunarRawInputSubsystem::HandleMouseButtonUp(const FPointerEvent& PointerEvent)
{
	RecordInputOwner(ResolveOwningLocalPlayer(PointerEvent), PointerEvent.GetUserIndex());
	const ELunarInputDeviceType InputDevice = PointerEvent.IsTouchEvent()
		? ELunarInputDeviceType::Touch
		: ELunarInputDeviceType::KeyboardMouse;
	HandleMouseButtonUp(PointerEvent.GetEffectingButton(), PointerEvent.GetScreenSpacePosition(), InputDevice);
	NotifyPointerInput(PointerEvent);
}

void ULunarRawInputSubsystem::HandleMouseWheel(const FPointerEvent& PointerEvent)
{
	RecordInputOwner(ResolveOwningLocalPlayer(PointerEvent), PointerEvent.GetUserIndex());
	const ELunarInputDeviceType InputDevice = PointerEvent.IsTouchEvent()
		? ELunarInputDeviceType::Touch
		: ELunarInputDeviceType::KeyboardMouse;
	HandleMouseWheel(PointerEvent.GetWheelDelta(), PointerEvent.GetScreenSpacePosition(), InputDevice);

	if (!FMath::IsNearlyZero(PointerEvent.GetWheelDelta()))
	{
		NotifyPointerInput(PointerEvent);
	}
}

void ULunarRawInputSubsystem::NotifyPointerInput(const FPointerEvent& PointerEvent)
{
	if (ULocalPlayer* LocalPlayer = ResolveOwningLocalPlayer(PointerEvent))
	{
		if (ULunarNavigationSubsystem* NavigationSubsystem = LocalPlayer->GetSubsystem<ULunarNavigationSubsystem>())
		{
			const ELunarInputDeviceType InputDevice = PointerEvent.IsTouchEvent()
				? ELunarInputDeviceType::Touch
				: ELunarInputDeviceType::KeyboardMouse;

			NavigationSubsystem->NotifyPointerInput(InputDevice);
		}
	}
}

void ULunarRawInputSubsystem::HandleKeyDown(const FKey& Key)
{
	const ELunarInputDeviceType InputDevice = Key.IsGamepadKey()
		? ELunarInputDeviceType::Gamepad
		: ELunarInputDeviceType::KeyboardMouse;
	HandleKeyDownForDevice(Key, InputDevice);
}

void ULunarRawInputSubsystem::HandleKeyDownForDevice(const FKey& Key, const ELunarInputDeviceType InputDevice)
{
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
	HandleKeyUpForDevice(Key, InputDevice);
}

void ULunarRawInputSubsystem::HandleKeyUpForDevice(const FKey& Key, const ELunarInputDeviceType InputDevice)
{
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

void ULunarRawInputSubsystem::HandleMouseMove(
	const FVector2D& ScreenPosition,
	const FVector2D& CursorDelta,
	const ELunarInputDeviceType InputDevice)
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
		MarkInputReceived(InputDevice);
		OnMouseMoved.Broadcast(ScreenPosition, RawDelta);
	}
}

void ULunarRawInputSubsystem::HandleMouseButtonDown(
	const FKey& Key,
	const FVector2D& ScreenPosition,
	const ELunarInputDeviceType InputDevice)
{
	Snapshot.MousePosition = ScreenPosition;
	PreviousMousePosition = ScreenPosition;
	bHasPreviousMousePosition = true;

	HandleKeyDownForDevice(Key, InputDevice);
}

void ULunarRawInputSubsystem::HandleMouseButtonUp(
	const FKey& Key,
	const FVector2D& ScreenPosition,
	const ELunarInputDeviceType InputDevice)
{
	Snapshot.MousePosition = ScreenPosition;
	PreviousMousePosition = ScreenPosition;
	bHasPreviousMousePosition = true;

	HandleKeyUpForDevice(Key, InputDevice);
}

void ULunarRawInputSubsystem::HandleMouseWheel(
	const float WheelDelta,
	const FVector2D& ScreenPosition,
	const ELunarInputDeviceType InputDevice)
{
	Snapshot.MousePosition = ScreenPosition;
	PreviousMousePosition = ScreenPosition;
	bHasPreviousMousePosition = true;

	PendingMouseWheelDelta += WheelDelta;

	if (!FMath::IsNearlyZero(WheelDelta))
	{
		MarkInputReceived(InputDevice);
		OnMouseWheel.Broadcast(WheelDelta, ScreenPosition);
	}
}

ULocalPlayer* ULunarRawInputSubsystem::ResolveOwningLocalPlayer(const FInputEvent& InputEvent) const
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return nullptr;
	}

	const FPlatformUserId PlatformUserId = InputEvent.GetPlatformUserId();
	if (PlatformUserId.IsValid())
	{
		if (ULocalPlayer* LocalPlayer = GameInstance->FindLocalPlayerFromPlatformUserId(PlatformUserId))
		{
			return LocalPlayer;
		}
	}

	const uint32 UserIndex = InputEvent.GetUserIndex();
	if (UserIndex <= static_cast<uint32>(MAX_int32))
	{
		if (ULocalPlayer* LocalPlayer = GameInstance->GetLocalPlayerByIndex(static_cast<int32>(UserIndex)))
		{
			return LocalPlayer;
		}
	}

	const TArray<ULocalPlayer*>& LocalPlayers = GameInstance->GetLocalPlayers();
	return LocalPlayers.Num() == 1 ? LocalPlayers[0] : nullptr;
}

void ULunarRawInputSubsystem::RecordInputOwner(ULocalPlayer* LocalPlayer, const uint32 SlateUserIndex)
{
	Snapshot.LastLocalPlayerIndex = LocalPlayer ? LocalPlayer->GetLocalPlayerIndex() : INDEX_NONE;
	Snapshot.LastSlateUserIndex = SlateUserIndex <= static_cast<uint32>(MAX_int32)
		? static_cast<int32>(SlateUserIndex)
		: INDEX_NONE;
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
