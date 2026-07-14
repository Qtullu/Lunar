// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "Subsystems/RawInput/LunarRawInputTypes.h"
#include "LunarRawInputSubsystem.generated.h"

/**
 * @file LunarRawInputSubsystem.h
 * @brief Raw input subsystem
 * @ingroup LunarRawInputSubsystem
 */

class FLunarRawInputProcessor;
class ULocalPlayer;
struct FAnalogInputEvent;
struct FInputEvent;
struct FKeyEvent;
struct FPointerEvent;

/**
 * @brief Called when a raw key event occurs
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLunarRawKeySignature, FKey, Key);

/**
 * @brief Called when raw mouse movement occurs
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLunarRawMouseMovedSignature, FVector2D, MousePosition, FVector2D, MouseDelta);

/**
 * @brief Called when any raw input occurs
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLunarRawAnyInputSignature, ELunarInputDeviceType, InputDevice, FKey, Key);

/**
 * @brief Called when raw mouse wheel input occurs
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLunarRawMouseWheelSignature, float, WheelDelta, FVector2D, MousePosition);

/**
 * @brief Tracks raw input state keys mouse movement input device and frame input events
 * @ingroup LunarRawInputSubsystem
 */
UCLASS()
class LUNAR_API ULunarRawInputSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	/**
	 * @brief Initializes raw input subsystem state
	 * @param Collection Subsystem collection
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * @brief Deinitializes raw input subsystem state
	 */
	virtual void Deinitialize() override;

	/**
	 * @brief Updates raw input frame state
	 * @param DeltaTime Time passed since last tick
	 */
	virtual void Tick(float DeltaTime) override;

	/**
	 * @brief Gets tick stat identifier
	 * @return Tick stat identifier
	 */
	virtual TStatId GetStatId() const override;

	/**
	 * @brief Checks whether subsystem can tick
	 * @return True if subsystem can tick
	 */
	virtual bool IsTickable() const override;

public:
	/** Input device changed event */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|Input")
	FLunarInputDeviceChangedSignature OnInputDeviceChanged;

	/** Raw key pressed event */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|Input")
	FLunarRawKeySignature OnKeyPressed;

	/** Raw key released event */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|Input")
	FLunarRawKeySignature OnKeyReleased;

	/** Raw key clicked event */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|Input")
	FLunarRawKeySignature OnKeyClicked;

	/** Raw mouse moved event */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|Input")
	FLunarRawMouseMovedSignature OnMouseMoved;

	/** Any raw input event */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|Input")
	FLunarRawAnyInputSignature OnAnyInput;

	/** Raw mouse wheel event */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|Input")
	FLunarRawMouseWheelSignature OnMouseWheel;

public:
	/**
	 * @brief Gets current raw input snapshot
	 * @return Current raw input snapshot
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Input")
	const FLunarRawInputSnapshot& GetSnapshot() const;

	/**
	 * @brief Gets last detected input device
	 * @return Last detected input device
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Input")
	ELunarInputDeviceType GetLastInputDevice() const;

	/**
	 * @brief Checks whether last detected input device is gamepad
	 * @return True if gamepad is active
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Input")
	bool IsUsingGamepad() const;

	/**
	 * @brief Checks whether last detected input device is keyboard mouse
	 * @return True if keyboard mouse is active
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Input")
	bool IsUsingKeyboardMouse() const;

	/**
	 * @brief Checks whether key is currently down
	 * @param Key Key to check
	 * @return True if key is down
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Input")
	bool IsKeyDown(FKey Key) const;

	/**
	 * @brief Checks whether key was pressed this frame
	 * @param Key Key to check
	 * @return True if key was pressed this frame
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Input")
	bool WasKeyPressedThisFrame(FKey Key) const;

	/**
	 * @brief Checks whether key was released this frame
	 * @param Key Key to check
	 * @return True if key was released this frame
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Input")
	bool WasKeyReleasedThisFrame(FKey Key) const;

	/**
	 * @brief Gets currently held keys
	 * @return Currently held keys
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Input")
	TArray<FKey> GetDownKeys() const;

	/**
	 * @brief Gets keys pressed this frame
	 * @return Keys pressed this frame
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Input")
	TArray<FKey> GetPressedKeysThisFrame() const;

	/**
	 * @brief Gets keys released this frame
	 * @return Keys released this frame
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Input")
	TArray<FKey> GetReleasedKeysThisFrame() const;

	/**
	 * @brief Gets current mouse position
	 * @return Current mouse position
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Input")
	FVector2D GetMousePosition() const;

	/**
	 * @brief Gets processed mouse movement delta
	 * @return Processed mouse movement delta
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Input")
	FVector2D GetMouseDelta() const;

	/**
	 * @brief Gets raw mouse movement delta
	 * @return Raw mouse movement delta
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Input")
	FVector2D GetRawMouseDelta() const;

	/**
	 * @brief Gets mouse wheel movement delta
	 * @return Mouse wheel movement delta
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Input")
	float GetMouseWheelDelta() const;

	/**
	 * @brief Clears raw input runtime state
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Input")
	void ClearInputState();

public:
	/**
	 * @brief Records and routes a complete raw key-down event
	 * @param KeyEvent Key-down event including its player and device identity
	 * @return True when Lunar navigation consumed the event
	 */
	bool HandleKeyDown(const FKeyEvent& KeyEvent);

	/**
	 * @brief Records and routes a complete raw key-up event
	 * @param KeyEvent Key-up event including its player and device identity
	 * @return True when Lunar navigation consumed the event
	 */
	bool HandleKeyUp(const FKeyEvent& KeyEvent);

	/**
	 * @brief Records and routes a complete raw analog input event
	 * @param AnalogEvent Analog event including its player and device identity
	 * @return True when Lunar navigation consumed the event
	 */
	bool HandleAnalogInput(const FAnalogInputEvent& AnalogEvent);

	/**
	 * @brief Records and routes a complete raw pointer-move event
	 * @param PointerEvent Pointer-move event including its player and device identity
	 */
	void HandleMouseMove(const FPointerEvent& PointerEvent);

	/**
	 * @brief Records and routes a complete raw pointer-button-down event
	 * @param PointerEvent Pointer-button event including its player and device identity
	 */
	void HandleMouseButtonDown(const FPointerEvent& PointerEvent);

	/**
	 * @brief Records and routes a complete raw pointer-button-up event
	 * @param PointerEvent Pointer-button event including its player and device identity
	 */
	void HandleMouseButtonUp(const FPointerEvent& PointerEvent);

	/**
	 * @brief Records and routes a complete raw pointer-wheel event
	 * @param PointerEvent Pointer-wheel event including its player and device identity
	 */
	void HandleMouseWheel(const FPointerEvent& PointerEvent);

	/**
	 * @brief Notifies the owning player's navigation context about pointer activity
	 * @param PointerEvent Pointer event including its player and device identity
	 */
	void NotifyPointerInput(const FPointerEvent& PointerEvent);

	/**
	 * @brief Handles raw key down event
	 * @param Key Pressed key
	 */
	void HandleKeyDown(const FKey& Key);

	/**
	 * @brief Handles raw key up event
	 * @param Key Released key
	 */
	void HandleKeyUp(const FKey& Key);

	/**
	 * @brief Handles raw mouse move event
	 * @param ScreenPosition Mouse screen position
	 * @param CursorDelta Mouse cursor delta
	 * @param InputDevice Classified source device for this pointer event
	 */
	void HandleMouseMove(const FVector2D& ScreenPosition, const FVector2D& CursorDelta,
		ELunarInputDeviceType InputDevice = ELunarInputDeviceType::KeyboardMouse);

	/**
	 * @brief Handles raw mouse button down event
	 * @param Key Pressed mouse button
	 * @param ScreenPosition Mouse screen position
	 * @param InputDevice Classified source device for this pointer event
	 */
	void HandleMouseButtonDown(const FKey& Key, const FVector2D& ScreenPosition,
		ELunarInputDeviceType InputDevice = ELunarInputDeviceType::KeyboardMouse);

	/**
	 * @brief Handles raw mouse button up event
	 * @param Key Released mouse button
	 * @param ScreenPosition Mouse screen position
	 * @param InputDevice Classified source device for this pointer event
	 */
	void HandleMouseButtonUp(const FKey& Key, const FVector2D& ScreenPosition,
		ELunarInputDeviceType InputDevice = ELunarInputDeviceType::KeyboardMouse);

	/**
	 * @brief Handles raw mouse wheel event
	 * @param WheelDelta Mouse wheel delta
	 * @param ScreenPosition Mouse screen position
	 * @param InputDevice Classified source device for this pointer event
	 */
	void HandleMouseWheel(float WheelDelta, const FVector2D& ScreenPosition,
		ELunarInputDeviceType InputDevice = ELunarInputDeviceType::KeyboardMouse);

private:
	/**
	 * @brief Resolves the local player that owns an input event
	 * @param InputEvent Input event to resolve
	 * @return Owning local player, or null when the event cannot be assigned safely
	 */
	ULocalPlayer* ResolveOwningLocalPlayer(const FInputEvent& InputEvent) const;

	/**
	 * @brief Records the local-player and Slate-user identity of a routed event
	 * @param LocalPlayer Local player resolved for the event, or null when unresolved
	 * @param SlateUserIndex Slate user index carried by the event
	 */
	void RecordInputOwner(ULocalPlayer* LocalPlayer, uint32 SlateUserIndex);

	/**
	 * @brief Records a key-down transition using an explicit device classification
	 * @param Key Pressed input key
	 * @param InputDevice Device category that produced the key event
	 */
	void HandleKeyDownForDevice(const FKey& Key, ELunarInputDeviceType InputDevice);

	/**
	 * @brief Records a key-up transition using an explicit device classification
	 * @param Key Released input key
	 * @param InputDevice Device category that produced the key event
	 */
	void HandleKeyUpForDevice(const FKey& Key, ELunarInputDeviceType InputDevice);

	/**
	 * @brief Sets last detected input device
	 * @param NewInputDevice New input device
	 */
	void SetLastInputDevice(ELunarInputDeviceType NewInputDevice);

	/**
	 * @brief Marks that raw input was received
	 * @param InputDevice Input device that produced input
	 * @param Key Input key
	 */
	void MarkInputReceived(ELunarInputDeviceType InputDevice, const FKey& Key = EKeys::Invalid);

private:
	/** Current raw input snapshot */
	FLunarRawInputSnapshot Snapshot;

	/** Pending processed mouse movement delta */
	FVector2D PendingMouseDelta = FVector2D::ZeroVector;

	/** Pending raw mouse movement delta */
	FVector2D PendingRawMouseDelta = FVector2D::ZeroVector;

	/** Previous mouse position */
	FVector2D PreviousMousePosition = FVector2D::ZeroVector;

	/** Pending mouse wheel movement delta */
	float PendingMouseWheelDelta = 0.0f;

	/** Currently held keys */
	TSet<FKey> DownKeys;

	/** Keys pressed this frame */
	TSet<FKey> PressedThisFrameKeys;

	/** Keys released this frame */
	TSet<FKey> ReleasedThisFrameKeys;

	/** Raw input processor instance */
	TSharedPtr<FLunarRawInputProcessor> InputProcessor;

	/** Last input time in seconds */
	double LastInputTimeSeconds = 0.0;

	/** True if subsystem was initialized */
	bool bInitialized = false;

	/** True if previous mouse position exists */
	bool bHasPreviousMousePosition = false;
};
