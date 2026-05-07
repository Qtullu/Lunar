// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Application/IInputProcessor.h"

class FSlateApplication;
struct FKeyEvent;
struct FPointerEvent;
class ICursor;
class ULunarRawInputSubsystem;

class FLunarRawInputProcessor final : public IInputProcessor
{
public:
	explicit FLunarRawInputProcessor(TWeakObjectPtr<ULunarRawInputSubsystem> InSubsystem);

	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override;

	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;
	virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;

	virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent, const FPointerEvent* GestureEvent) override;

private:
	TWeakObjectPtr<ULunarRawInputSubsystem> Subsystem;
};