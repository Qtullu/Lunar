// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Application/IInputProcessor.h"

class FSlateApplication;
struct FAnalogInputEvent;
struct FKeyEvent;
struct FPointerEvent;
class ICursor;
class ULunarRawInputSubsystem;

/**
 * @file LunarRawInputProcessor.h
 * @brief Slate input preprocessor that forwards raw events to the Lunar raw-input subsystem
 * @ingroup LunarRawInputSubsystem
 */

/**
 * @brief Captures Slate input events before widget routing and forwards them to Lunar systems
 * @ingroup LunarRawInputSubsystem
 */
class FLunarRawInputProcessor final : public IInputProcessor
{
public:
	/**
	 * @brief Creates an input processor for a raw-input subsystem
	 * @param InSubsystem Raw-input subsystem that receives captured events
	 */
	explicit FLunarRawInputProcessor(TWeakObjectPtr<ULunarRawInputSubsystem> InSubsystem);

	/**
	 * @brief Performs the input-processor frame update
	 * @param DeltaTime Time elapsed since the previous frame
	 * @param SlateApp Slate application being processed
	 * @param Cursor Active platform cursor
	 */
	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override;

	/**
	 * @brief Forwards a key-down event to raw input and player navigation
	 * @param SlateApp Slate application that received the event
	 * @param InKeyEvent Key-down event to route
	 * @return True when Lunar navigation consumes the event
	 */
	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;

	/**
	 * @brief Forwards a key-up event to raw input and player navigation
	 * @param SlateApp Slate application that received the event
	 * @param InKeyEvent Key-up event to route
	 * @return True when Lunar navigation consumes the event
	 */
	virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;

	/**
	 * @brief Forwards an analog event to raw input and player navigation
	 * @param SlateApp Slate application that received the event
	 * @param InAnalogInputEvent Analog input event to route
	 * @return True when Lunar navigation consumes the event
	 */
	virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent) override;

	/**
	 * @brief Forwards pointer movement to raw input and player navigation
	 * @param SlateApp Slate application that received the event
	 * @param MouseEvent Pointer-move event to route
	 * @return False so ordinary Slate pointer routing continues
	 */
	virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;

	/**
	 * @brief Forwards a pointer-button-down event to raw input and player navigation
	 * @param SlateApp Slate application that received the event
	 * @param MouseEvent Pointer-button event to route
	 * @return False so ordinary Slate pointer routing continues
	 */
	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;

	/**
	 * @brief Forwards a pointer-button-up event to raw input and player navigation
	 * @param SlateApp Slate application that received the event
	 * @param MouseEvent Pointer-button event to route
	 * @return False so ordinary Slate pointer routing continues
	 */
	virtual bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;

	/**
	 * @brief Forwards pointer-wheel or gesture input to raw input and player navigation
	 * @param SlateApp Slate application that received the event
	 * @param MouseEvent Pointer-wheel event to route
	 * @param GestureEvent Optional gesture event supplied by Slate
	 * @return False so ordinary Slate pointer routing continues
	 */
	virtual bool HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent, const FPointerEvent* GestureEvent) override;

private:
	/** Raw-input subsystem that owns this processor. */
	TWeakObjectPtr<ULunarRawInputSubsystem> Subsystem;
};
