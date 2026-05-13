// Copyright 2026 Edgar Frolenkov. All rights reserved.

#include "Widgets/LunarDraggableWindow.h"

#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Components/Widget.h"

void ULunarDraggableWindow::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (!IsDesignTime())
	{
		return;
	}

	CachedWindowPosition = InitialWindowPosition;
	CachedWindowSize = InitialWindowSize;

	RefreshWindowLayout();
}

void ULunarDraggableWindow::NativeConstruct()
{
	Super::NativeConstruct();

	ApplyInitialLayout();
	UpdateResizeHandleVisibility();
	UpdateResizeHandleLayout();
	SetResizeHandleState(ELunarDraggableWindowHandleState::Normal);
}

FReply ULunarDraggableWindow::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (!IsLeftMouseButton(InMouseEvent))
	{
		return FReply::Unhandled();
	}

	ELunarDraggableWindowResizeDirection ResizeDirection = ELunarDraggableWindowResizeDirection::None;

	if (CanStartResizeFromMouseEvent(InMouseEvent, ResizeDirection))
	{
		StartResize(InMouseEvent, ResizeDirection);
		return FReply::Handled().CaptureMouse(TakeWidget());
	}

	if (CanStartDragFromMouseEvent(InMouseEvent))
	{
		StartDrag(InMouseEvent);
		return FReply::Handled().CaptureMouse(TakeWidget());
	}

	return FReply::Unhandled();
}

FReply ULunarDraggableWindow::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

	if (!IsLeftMouseButton(InMouseEvent))
	{
		return FReply::Unhandled();
	}

	const bool bWasInteracting = bIsDragging || bIsResizing;

	StopDrag();
	StopResize();

	if (bWasInteracting)
	{
		UpdateResizeHandleStateFromMouse(InMouseEvent);
		return FReply::Handled().ReleaseMouseCapture();
	}

	return FReply::Unhandled();
}

FReply ULunarDraggableWindow::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseMove(InGeometry, InMouseEvent);

	UpdateResizeHandleStateFromMouse(InMouseEvent);

	if (bIsResizing)
	{
		ProcessResize(InMouseEvent);
		return FReply::Handled();
	}

	if (bIsDragging)
	{
		ProcessDrag(InMouseEvent);
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void ULunarDraggableWindow::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	if (!bIsResizing)
	{
		SetResizeHandleState(ELunarDraggableWindowHandleState::Normal);
	}
}

void ULunarDraggableWindow::SetWindowPosition(FVector2D NewPosition)
{
	NewPosition = ApplyBoundsToPosition(NewPosition, CachedWindowSize);
	NewPosition = ApplySnapToPosition(NewPosition, CachedWindowSize);

	ApplyWindowPositionAndSize(NewPosition, CachedWindowSize);
}

FVector2D ULunarDraggableWindow::GetWindowPosition() const
{
	return CachedWindowPosition;
}

void ULunarDraggableWindow::SetWindowSize(FVector2D NewSize)
{
	NewSize = ClampSize(CachedWindowPosition, NewSize);

	FVector2D NewPosition = ApplyBoundsToPosition(CachedWindowPosition, NewSize);
	NewPosition = ApplySnapToPosition(NewPosition, NewSize);

	ApplyWindowPositionAndSize(NewPosition, NewSize);
}

FVector2D ULunarDraggableWindow::GetWindowSize() const
{
	return CachedWindowSize;
}

void ULunarDraggableWindow::SetBoundsWidget(UWidget* NewBoundsWidget)
{
	BoundsWidget = NewBoundsWidget;

	FVector2D NewPosition = ApplyBoundsToPosition(CachedWindowPosition, CachedWindowSize);
	NewPosition = ApplySnapToPosition(NewPosition, CachedWindowSize);

	ApplyWindowPositionAndSize(NewPosition, CachedWindowSize);
}

void ULunarDraggableWindow::RefreshWindowLayout()
{
	CachedWindowSize.X = FMath::Max(CachedWindowSize.X, MinWindowSize.X);
	CachedWindowSize.Y = FMath::Max(CachedWindowSize.Y, MinWindowSize.Y);

	if (bUseMaxWindowSize)
	{
		CachedWindowSize.X = FMath::Min(CachedWindowSize.X, MaxWindowSize.X);
		CachedWindowSize.Y = FMath::Min(CachedWindowSize.Y, MaxWindowSize.Y);
	}

	ApplyWindowPositionAndSize(CachedWindowPosition, CachedWindowSize);
	UpdateResizeHandleVisibility();
	UpdateResizeHandleLayout();
}

bool ULunarDraggableWindow::IsDragging() const
{
	return bIsDragging;
}

bool ULunarDraggableWindow::IsResizing() const
{
	return bIsResizing;
}

bool ULunarDraggableWindow::IsInteracting() const
{
	return bIsDragging || bIsResizing;
}

void ULunarDraggableWindow::ApplyInitialLayout()
{
	CachedWindowPosition = InitialWindowPosition;
	CachedWindowSize = ClampSize(CachedWindowPosition, InitialWindowSize);

	FVector2D NewPosition = ApplyBoundsToPosition(CachedWindowPosition, CachedWindowSize);
	NewPosition = ApplySnapToPosition(NewPosition, CachedWindowSize);

	ApplyWindowPositionAndSize(NewPosition, CachedWindowSize);
}

void ULunarDraggableWindow::ApplyWindowPositionAndSize(FVector2D NewPosition, FVector2D NewSize)
{
	CachedWindowPosition = NewPosition;
	CachedWindowSize = NewSize;

	if (WindowRoot)
	{
		WindowRoot->SetWidthOverride(CachedWindowSize.X);
		WindowRoot->SetHeightOverride(CachedWindowSize.Y);

		if (UCanvasPanelSlot* CanvasSlot = GetWindowCanvasSlot())
		{
			CanvasSlot->SetAutoSize(false);
			CanvasSlot->SetPosition(CachedWindowPosition);
			CanvasSlot->SetSize(CachedWindowSize);
		}
	}

	UpdateResizeHandleLayout();
}

void ULunarDraggableWindow::UpdateResizeHandleLayout()
{
	if (!ResizeHandle)
	{
		return;
	}

	UCanvasPanelSlot* CanvasSlot = GetResizeHandleCanvasSlot();

	if (!CanvasSlot)
	{
		return;
	}

	const FVector2D HandlePosition = CachedWindowPosition + CachedWindowSize - ResizeHandleSize;

	CanvasSlot->SetAutoSize(false);
	CanvasSlot->SetSize(ResizeHandleSize);
	CanvasSlot->SetPosition(HandlePosition);
}

void ULunarDraggableWindow::UpdateResizeHandleVisibility()
{
	if (!ResizeHandle)
	{
		return;
	}

	const bool bShouldShowHandle = ResizeMode == ELunarDraggableWindowResizeMode::BottomRightHandle;
	ResizeHandle->SetVisibility(bShouldShowHandle ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	if (!bShouldShowHandle)
	{
		SetResizeHandleState(ELunarDraggableWindowHandleState::Normal);
	}
}

bool ULunarDraggableWindow::IsLeftMouseButton(const FPointerEvent& InMouseEvent) const
{
	return InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton || InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton);
}

bool ULunarDraggableWindow::IsModifierPressed(const FPointerEvent& InMouseEvent) const
{
	if (ResizeModifierKey == EKeys::LeftAlt || ResizeModifierKey == EKeys::RightAlt)
	{
		return InMouseEvent.IsAltDown();
	}

	if (ResizeModifierKey == EKeys::LeftControl || ResizeModifierKey == EKeys::RightControl)
	{
		return InMouseEvent.IsControlDown();
	}

	if (ResizeModifierKey == EKeys::LeftShift || ResizeModifierKey == EKeys::RightShift)
	{
		return InMouseEvent.IsShiftDown();
	}

	if (ResizeModifierKey == EKeys::LeftCommand || ResizeModifierKey == EKeys::RightCommand)
	{
		return InMouseEvent.IsCommandDown();
	}

	return false;
}

bool ULunarDraggableWindow::IsWidgetUnderMouse(const UWidget* Widget, const FPointerEvent& InMouseEvent) const
{
	if (!Widget)
	{
		return false;
	}

	return Widget->GetCachedGeometry().IsUnderLocation(InMouseEvent.GetScreenSpacePosition());
}

bool ULunarDraggableWindow::IsHeadUnderMouse(const FPointerEvent& InMouseEvent) const
{
	return IsWidgetUnderMouse(HeadHitBox, InMouseEvent);
}

bool ULunarDraggableWindow::IsBodyUnderMouse(const FPointerEvent& InMouseEvent) const
{
	return IsWidgetUnderMouse(BodyHitBox, InMouseEvent);
}

bool ULunarDraggableWindow::IsResizeHandleUnderMouse(const FPointerEvent& InMouseEvent) const
{
	return ResizeHandle && ResizeMode == ELunarDraggableWindowResizeMode::BottomRightHandle && IsWidgetUnderMouse(ResizeHandle, InMouseEvent);
}

bool ULunarDraggableWindow::CanStartDragFromMouseEvent(const FPointerEvent& InMouseEvent) const
{
	if (!bDraggable)
	{
		return false;
	}

	if (IsHeadUnderMouse(InMouseEvent))
	{
		return true;
	}

	if (bAllowDragFromBody && IsBodyUnderMouse(InMouseEvent))
	{
		return true;
	}

	return false;
}

bool ULunarDraggableWindow::CanStartResizeFromMouseEvent(const FPointerEvent& InMouseEvent, ELunarDraggableWindowResizeDirection& OutDirection) const
{
	OutDirection = ELunarDraggableWindowResizeDirection::None;

	if (ResizeMode == ELunarDraggableWindowResizeMode::None)
	{
		return false;
	}

	if (ResizeMode == ELunarDraggableWindowResizeMode::BottomRightHandle)
	{
		if (IsResizeHandleUnderMouse(InMouseEvent))
		{
			OutDirection = ELunarDraggableWindowResizeDirection::BottomRight;
			return true;
		}

		return false;
	}

	if (ResizeMode == ELunarDraggableWindowResizeMode::EdgesAndCorners)
	{
		OutDirection = GetResizeDirectionFromWindowEdges(InMouseEvent);
		return OutDirection != ELunarDraggableWindowResizeDirection::None;
	}

	if (ResizeMode == ELunarDraggableWindowResizeMode::Anywhere)
	{
		if (bRequireModifierForAnywhereResize && !IsModifierPressed(InMouseEvent))
		{
			return false;
		}

		if (IsHeadUnderMouse(InMouseEvent) || IsBodyUnderMouse(InMouseEvent))
		{
			OutDirection = ELunarDraggableWindowResizeDirection::BottomRight;
			return true;
		}
	}

	return false;
}

void ULunarDraggableWindow::StartDrag(const FPointerEvent& InMouseEvent)
{
	bIsDragging = true;
	bIsResizing = false;

	const FVector2D MousePosition = GetMousePositionInRootCanvas(InMouseEvent);
	DragOffset = MousePosition - CachedWindowPosition;

	OnMoveStateChanged.Broadcast(true);
	OnMoveStarted.Broadcast(CachedWindowPosition);
}

void ULunarDraggableWindow::StopDrag()
{
	if (!bIsDragging)
	{
		return;
	}

	bIsDragging = false;
	DragOffset = FVector2D::ZeroVector;

	OnMoveStateChanged.Broadcast(false);
	OnMoveStopped.Broadcast(CachedWindowPosition);
}

void ULunarDraggableWindow::StartResize(const FPointerEvent& InMouseEvent, ELunarDraggableWindowResizeDirection Direction)
{
	bIsResizing = true;
	bIsDragging = false;

	ActiveResizeDirection = Direction;

	ResizeStartMousePosition = GetMousePositionInRootCanvas(InMouseEvent);
	ResizeStartWindowPosition = CachedWindowPosition;
	ResizeStartWindowSize = CachedWindowSize;

	SetResizeHandleState(ELunarDraggableWindowHandleState::Pressed);
}

void ULunarDraggableWindow::StopResize()
{
	if (!bIsResizing)
	{
		return;
	}

	bIsResizing = false;
	ActiveResizeDirection = ELunarDraggableWindowResizeDirection::None;

	ResizeStartMousePosition = FVector2D::ZeroVector;
	ResizeStartWindowPosition = FVector2D::ZeroVector;
	ResizeStartWindowSize = FVector2D::ZeroVector;

	SetResizeHandleState(ELunarDraggableWindowHandleState::Normal);
}

void ULunarDraggableWindow::ProcessDrag(const FPointerEvent& InMouseEvent)
{
	const FVector2D MousePosition = GetMousePositionInRootCanvas(InMouseEvent);

	FVector2D NewPosition = MousePosition - DragOffset;
	NewPosition = ApplyBoundsToPosition(NewPosition, CachedWindowSize);
	NewPosition = ApplySnapToPosition(NewPosition, CachedWindowSize);

	ApplyWindowPositionAndSize(NewPosition, CachedWindowSize);
	OnMoving.Broadcast(CachedWindowPosition);
}

void ULunarDraggableWindow::ProcessResize(const FPointerEvent& InMouseEvent)
{
	const FVector2D MousePosition = GetMousePositionInRootCanvas(InMouseEvent);
	const FVector2D MouseDelta = MousePosition - ResizeStartMousePosition;

	FVector2D NewPosition = ResizeStartWindowPosition;
	FVector2D NewSize = ResizeStartWindowSize;

	switch (ActiveResizeDirection)
	{
	case ELunarDraggableWindowResizeDirection::Left:
	{
		NewPosition.X = ResizeStartWindowPosition.X + MouseDelta.X;
		NewSize.X = ResizeStartWindowSize.X - MouseDelta.X;
		break;
	}

	case ELunarDraggableWindowResizeDirection::Right:
	{
		NewSize.X = ResizeStartWindowSize.X + MouseDelta.X;
		break;
	}

	case ELunarDraggableWindowResizeDirection::Top:
	{
		NewPosition.Y = ResizeStartWindowPosition.Y + MouseDelta.Y;
		NewSize.Y = ResizeStartWindowSize.Y - MouseDelta.Y;
		break;
	}

	case ELunarDraggableWindowResizeDirection::Bottom:
	{
		NewSize.Y = ResizeStartWindowSize.Y + MouseDelta.Y;
		break;
	}

	case ELunarDraggableWindowResizeDirection::TopLeft:
	{
		NewPosition.X = ResizeStartWindowPosition.X + MouseDelta.X;
		NewPosition.Y = ResizeStartWindowPosition.Y + MouseDelta.Y;
		NewSize.X = ResizeStartWindowSize.X - MouseDelta.X;
		NewSize.Y = ResizeStartWindowSize.Y - MouseDelta.Y;
		break;
	}

	case ELunarDraggableWindowResizeDirection::TopRight:
	{
		NewPosition.Y = ResizeStartWindowPosition.Y + MouseDelta.Y;
		NewSize.X = ResizeStartWindowSize.X + MouseDelta.X;
		NewSize.Y = ResizeStartWindowSize.Y - MouseDelta.Y;
		break;
	}

	case ELunarDraggableWindowResizeDirection::BottomLeft:
	{
		NewPosition.X = ResizeStartWindowPosition.X + MouseDelta.X;
		NewSize.X = ResizeStartWindowSize.X - MouseDelta.X;
		NewSize.Y = ResizeStartWindowSize.Y + MouseDelta.Y;
		break;
	}

	case ELunarDraggableWindowResizeDirection::BottomRight:
	{
		NewSize.X = ResizeStartWindowSize.X + MouseDelta.X;
		NewSize.Y = ResizeStartWindowSize.Y + MouseDelta.Y;
		break;
	}

	default:
	{
		return;
	}
	}

	const FVector2D OldSize = NewSize;
	NewSize = ClampSize(NewPosition, NewSize);

	if (ActiveResizeDirection == ELunarDraggableWindowResizeDirection::Left || ActiveResizeDirection == ELunarDraggableWindowResizeDirection::TopLeft || ActiveResizeDirection == ELunarDraggableWindowResizeDirection::BottomLeft)
	{
		NewPosition.X += OldSize.X - NewSize.X;
	}

	if (ActiveResizeDirection == ELunarDraggableWindowResizeDirection::Top || ActiveResizeDirection == ELunarDraggableWindowResizeDirection::TopLeft || ActiveResizeDirection == ELunarDraggableWindowResizeDirection::TopRight)
	{
		NewPosition.Y += OldSize.Y - NewSize.Y;
	}

	NewPosition = ApplyBoundsToPosition(NewPosition, NewSize);
	NewPosition = ApplySnapToPosition(NewPosition, NewSize);

	ApplyWindowPositionAndSize(NewPosition, NewSize);
}

void ULunarDraggableWindow::SetResizeHandleState(ELunarDraggableWindowHandleState NewState)
{
	if (ResizeHandleState == NewState)
	{
		return;
	}

	ResizeHandleState = NewState;
	OnResizeHandleStateChanged.Broadcast(ResizeHandleState);
}

void ULunarDraggableWindow::UpdateResizeHandleStateFromMouse(const FPointerEvent& InMouseEvent)
{
	if (ResizeMode != ELunarDraggableWindowResizeMode::BottomRightHandle || !ResizeHandle)
	{
		SetResizeHandleState(ELunarDraggableWindowHandleState::Normal);
		return;
	}

	if (bIsResizing)
	{
		SetResizeHandleState(ELunarDraggableWindowHandleState::Pressed);
		return;
	}

	if (IsResizeHandleUnderMouse(InMouseEvent))
	{
		SetResizeHandleState(ELunarDraggableWindowHandleState::Hovered);
		return;
	}

	SetResizeHandleState(ELunarDraggableWindowHandleState::Normal);
}

FVector2D ULunarDraggableWindow::GetMousePositionInRootCanvas(const FPointerEvent& InMouseEvent) const
{
	if (!RootCanvas)
	{
		return FVector2D::ZeroVector;
	}

	return RootCanvas->GetCachedGeometry().AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
}

ELunarDraggableWindowResizeDirection ULunarDraggableWindow::GetResizeDirectionFromWindowEdges(const FPointerEvent& InMouseEvent) const
{
	if (!WindowRoot)
	{
		return ELunarDraggableWindowResizeDirection::None;
	}

	const FGeometry WindowGeometry = WindowRoot->GetCachedGeometry();
	const FVector2D LocalMousePosition = WindowGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
	const FVector2D LocalSize = WindowGeometry.GetLocalSize();

	const bool bInsideX = LocalMousePosition.X >= 0.0f && LocalMousePosition.X <= LocalSize.X;
	const bool bInsideY = LocalMousePosition.Y >= 0.0f && LocalMousePosition.Y <= LocalSize.Y;

	if (!bInsideX || !bInsideY)
	{
		return ELunarDraggableWindowResizeDirection::None;
	}

	const bool bNearLeft = LocalMousePosition.X <= ResizeBorderThickness;
	const bool bNearRight = LocalMousePosition.X >= LocalSize.X - ResizeBorderThickness;
	const bool bNearTop = LocalMousePosition.Y <= ResizeBorderThickness;
	const bool bNearBottom = LocalMousePosition.Y >= LocalSize.Y - ResizeBorderThickness;

	if (bNearLeft && bNearTop)
	{
		return ELunarDraggableWindowResizeDirection::TopLeft;
	}

	if (bNearRight && bNearTop)
	{
		return ELunarDraggableWindowResizeDirection::TopRight;
	}

	if (bNearLeft && bNearBottom)
	{
		return ELunarDraggableWindowResizeDirection::BottomLeft;
	}

	if (bNearRight && bNearBottom)
	{
		return ELunarDraggableWindowResizeDirection::BottomRight;
	}

	if (bNearLeft)
	{
		return ELunarDraggableWindowResizeDirection::Left;
	}

	if (bNearRight)
	{
		return ELunarDraggableWindowResizeDirection::Right;
	}

	if (bNearTop)
	{
		return ELunarDraggableWindowResizeDirection::Top;
	}

	if (bNearBottom)
	{
		return ELunarDraggableWindowResizeDirection::Bottom;
	}

	return ELunarDraggableWindowResizeDirection::None;
}

bool ULunarDraggableWindow::GetBoundsRectInRootCanvas(FVector2D& OutBoundsPosition, FVector2D& OutBoundsSize) const
{
	OutBoundsPosition = FVector2D::ZeroVector;
	OutBoundsSize = FVector2D::ZeroVector;

	if (!RootCanvas)
	{
		return false;
	}

	if (BoundsWidget)
	{
		const FGeometry RootGeometry = RootCanvas->GetCachedGeometry();
		const FGeometry BoundsGeometry = BoundsWidget->GetCachedGeometry();

		const FVector2D BoundsAbsoluteTopLeft = BoundsGeometry.LocalToAbsolute(FVector2D::ZeroVector);
		const FVector2D BoundsAbsoluteBottomRight = BoundsGeometry.LocalToAbsolute(BoundsGeometry.GetLocalSize());

		OutBoundsPosition = RootGeometry.AbsoluteToLocal(BoundsAbsoluteTopLeft);
		OutBoundsSize = RootGeometry.AbsoluteToLocal(BoundsAbsoluteBottomRight) - OutBoundsPosition;

		return OutBoundsSize.X > 0.0f && OutBoundsSize.Y > 0.0f;
	}

	if (bUseParentAsBounds)
	{
		OutBoundsPosition = FVector2D::ZeroVector;
		OutBoundsSize = RootCanvas->GetCachedGeometry().GetLocalSize();

		return OutBoundsSize.X > 0.0f && OutBoundsSize.Y > 0.0f;
	}

	return false;
}

FVector2D ULunarDraggableWindow::ApplyBoundsToPosition(FVector2D Position, FVector2D Size) const
{
	if (BoundsMode == ELunarDraggableWindowBoundsMode::None)
	{
		return Position;
	}

	FVector2D BoundsPosition;
	FVector2D BoundsSize;

	if (!GetBoundsRectInRootCanvas(BoundsPosition, BoundsSize))
	{
		return Position;
	}

	float MinX = BoundsPosition.X;
	float MinY = BoundsPosition.Y;
	float MaxX = BoundsPosition.X + BoundsSize.X - Size.X;
	float MaxY = BoundsPosition.Y + BoundsSize.Y - Size.Y;

	if (BoundsMode == ELunarDraggableWindowBoundsMode::SoftClamp)
	{
		MinX -= SoftClampOutsideMargin;
		MinY -= SoftClampOutsideMargin;
		MaxX += SoftClampOutsideMargin;
		MaxY += SoftClampOutsideMargin;
	}

	if (MaxX < MinX)
	{
		Position.X = MinX;
	}
	else
	{
		Position.X = FMath::Clamp(Position.X, MinX, MaxX);
	}

	if (MaxY < MinY)
	{
		Position.Y = MinY;
	}
	else
	{
		Position.Y = FMath::Clamp(Position.Y, MinY, MaxY);
	}

	return Position;
}

FVector2D ULunarDraggableWindow::ApplySnapToPosition(FVector2D Position, FVector2D Size) const
{
	if (!bEnableEdgeSnap || EdgeSnapDistance <= 0.0f)
	{
		return Position;
	}

	FVector2D BoundsPosition;
	FVector2D BoundsSize;

	if (!GetBoundsRectInRootCanvas(BoundsPosition, BoundsSize))
	{
		return Position;
	}

	const float MinX = BoundsPosition.X;
	const float MinY = BoundsPosition.Y;
	const float MaxX = BoundsPosition.X + BoundsSize.X - Size.X;
	const float MaxY = BoundsPosition.Y + BoundsSize.Y - Size.Y;

	if (MaxX >= MinX)
	{
		if (FMath::Abs(Position.X - MinX) <= EdgeSnapDistance)
		{
			Position.X = MinX;
		}

		if (FMath::Abs(Position.X - MaxX) <= EdgeSnapDistance)
		{
			Position.X = MaxX;
		}
	}

	if (MaxY >= MinY)
	{
		if (FMath::Abs(Position.Y - MinY) <= EdgeSnapDistance)
		{
			Position.Y = MinY;
		}

		if (FMath::Abs(Position.Y - MaxY) <= EdgeSnapDistance)
		{
			Position.Y = MaxY;
		}
	}

	return Position;
}

FVector2D ULunarDraggableWindow::ClampSize(FVector2D Position, FVector2D Size) const
{
	Size.X = FMath::Max(Size.X, MinWindowSize.X);
	Size.Y = FMath::Max(Size.Y, MinWindowSize.Y);

	if (bUseMaxWindowSize)
	{
		Size.X = FMath::Min(Size.X, MaxWindowSize.X);
		Size.Y = FMath::Min(Size.Y, MaxWindowSize.Y);
	}

	if (!bClampResizeInsideBounds)
	{
		return Size;
	}

	FVector2D BoundsPosition;
	FVector2D BoundsSize;

	if (!GetBoundsRectInRootCanvas(BoundsPosition, BoundsSize))
	{
		return Size;
	}

	const float BoundsRight = BoundsPosition.X + BoundsSize.X;
	const float BoundsBottom = BoundsPosition.Y + BoundsSize.Y;

	const float MaxWidthByBounds = FMath::Max(MinWindowSize.X, BoundsRight - Position.X);
	const float MaxHeightByBounds = FMath::Max(MinWindowSize.Y, BoundsBottom - Position.Y);

	Size.X = FMath::Min(Size.X, MaxWidthByBounds);
	Size.Y = FMath::Min(Size.Y, MaxHeightByBounds);

	return Size;
}

UCanvasPanelSlot* ULunarDraggableWindow::GetWindowCanvasSlot() const
{
	if (!WindowRoot)
	{
		return nullptr;
	}

	return Cast<UCanvasPanelSlot>(WindowRoot->Slot);
}

UCanvasPanelSlot* ULunarDraggableWindow::GetResizeHandleCanvasSlot() const
{
	if (!ResizeHandle)
	{
		return nullptr;
	}

	return Cast<UCanvasPanelSlot>(ResizeHandle->Slot);
}