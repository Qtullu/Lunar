// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "Subsystems/RawInput/LunarRawInputProcessor.h"

#include "Framework/Application/SlateApplication.h"
#include "Input/Events.h"
#include "Subsystems/LunarRawInputSubsystem.h"

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
		RawInputSubsystem->HandleKeyDown(InKeyEvent.GetKey());
	}

	return false;
}

bool FLunarRawInputProcessor::HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	if (ULunarRawInputSubsystem* RawInputSubsystem = Subsystem.Get())
	{
		RawInputSubsystem->HandleKeyUp(InKeyEvent.GetKey());
	}

	return false;
}

bool FLunarRawInputProcessor::HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (ULunarRawInputSubsystem* RawInputSubsystem = Subsystem.Get())
	{
		RawInputSubsystem->HandleMouseMove(MouseEvent.GetScreenSpacePosition(), MouseEvent.GetCursorDelta());
	}

	return false;
}

bool FLunarRawInputProcessor::HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (ULunarRawInputSubsystem* RawInputSubsystem = Subsystem.Get())
	{
		RawInputSubsystem->HandleMouseButtonDown(MouseEvent.GetEffectingButton(), MouseEvent.GetScreenSpacePosition());
	}

	return false;
}

bool FLunarRawInputProcessor::HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (ULunarRawInputSubsystem* RawInputSubsystem = Subsystem.Get())
	{
		RawInputSubsystem->HandleMouseButtonUp(MouseEvent.GetEffectingButton(), MouseEvent.GetScreenSpacePosition());
	}

	return false;
}

bool FLunarRawInputProcessor::HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent, const FPointerEvent* GestureEvent)
{
	if (ULunarRawInputSubsystem* RawInputSubsystem = Subsystem.Get())
	{
		RawInputSubsystem->HandleMouseWheel(MouseEvent.GetWheelDelta(), MouseEvent.GetScreenSpacePosition());
	}

	return false;
}