// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "Subsystems/RawInput/LunarRawInputProcessor.h"

#include "Framework/Application/SlateApplication.h"
#include "Input/Events.h"
#include "Subsystems/RawInput/LunarRawInputSubsystem.h"

/**
 * @file LunarRawInputProcessor.cpp
 * @brief Slate input forwarding for the Lunar raw-input and navigation systems
 * @ingroup LunarRawInputSubsystem
 */

FLunarRawInputProcessor::FLunarRawInputProcessor(TWeakObjectPtr<ULunarRawInputSubsystem> InSubsystem)
	: Subsystem(InSubsystem)
{
}

void FLunarRawInputProcessor::Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor)
{
}

bool FLunarRawInputProcessor::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	if (ULunarRawInputSubsystem* RawInputSubsystem = Subsystem.Get())
	{
		return RawInputSubsystem->HandleKeyDown(InKeyEvent);
	}

	return false;
}

bool FLunarRawInputProcessor::HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	if (ULunarRawInputSubsystem* RawInputSubsystem = Subsystem.Get())
	{
		return RawInputSubsystem->HandleKeyUp(InKeyEvent);
	}

	return false;
}

bool FLunarRawInputProcessor::HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent)
{
	if (ULunarRawInputSubsystem* RawInputSubsystem = Subsystem.Get())
	{
		return RawInputSubsystem->HandleAnalogInput(InAnalogInputEvent);
	}

	return false;
}

bool FLunarRawInputProcessor::HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (ULunarRawInputSubsystem* RawInputSubsystem = Subsystem.Get())
	{
		RawInputSubsystem->HandleMouseMove(MouseEvent);
	}

	return false;
}

bool FLunarRawInputProcessor::HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (ULunarRawInputSubsystem* RawInputSubsystem = Subsystem.Get())
	{
		RawInputSubsystem->HandleMouseButtonDown(MouseEvent);
	}

	return false;
}

bool FLunarRawInputProcessor::HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (ULunarRawInputSubsystem* RawInputSubsystem = Subsystem.Get())
	{
		RawInputSubsystem->HandleMouseButtonUp(MouseEvent);
	}

	return false;
}

bool FLunarRawInputProcessor::HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent, const FPointerEvent* GestureEvent)
{
	if (ULunarRawInputSubsystem* RawInputSubsystem = Subsystem.Get())
	{
		RawInputSubsystem->HandleMouseWheel(MouseEvent);
	}

	return false;
}
