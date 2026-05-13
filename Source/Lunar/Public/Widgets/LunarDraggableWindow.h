// Copyright 2026 Edgar Frolenkov. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"
#include "LunarDraggableWindow.generated.h"

class UBorder;
class UCanvasPanel;
class UCanvasPanelSlot;
class USizeBox;
class UWidget;

UENUM(BlueprintType)
enum class ELunarDraggableWindowBoundsMode : uint8
{
	None UMETA(DisplayName = "None"),
	Clamp UMETA(DisplayName = "Clamp"),
	SoftClamp UMETA(DisplayName = "Soft Clamp")
};

UENUM(BlueprintType)
enum class ELunarDraggableWindowResizeMode : uint8
{
	None UMETA(DisplayName = "None"),
	BottomRightHandle UMETA(DisplayName = "Bottom Right Handle"),
	EdgesAndCorners UMETA(DisplayName = "Edges And Corners"),
	Anywhere UMETA(DisplayName = "Anywhere")
};

UENUM(BlueprintType)
enum class ELunarDraggableWindowResizeDirection : uint8
{
	None UMETA(DisplayName = "None"),

	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right"),
	Top UMETA(DisplayName = "Top"),
	Bottom UMETA(DisplayName = "Bottom"),

	TopLeft UMETA(DisplayName = "Top Left"),
	TopRight UMETA(DisplayName = "Top Right"),
	BottomLeft UMETA(DisplayName = "Bottom Left"),
	BottomRight UMETA(DisplayName = "Bottom Right")
};

UENUM(BlueprintType)
enum class ELunarDraggableWindowHandleState : uint8
{
	Normal UMETA(DisplayName = "Normal"),
	Hovered UMETA(DisplayName = "Hovered"),
	Pressed UMETA(DisplayName = "Pressed")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLunarDraggableWindowMoveStateChangedSignature, bool, bIsMoving);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLunarDraggableWindowMoveStartedSignature, FVector2D, Position);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLunarDraggableWindowMovingSignature, FVector2D, Position);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLunarDraggableWindowMoveStoppedSignature, FVector2D, Position);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLunarDraggableWindowResizeHandleStateChangedSignature, ELunarDraggableWindowHandleState, NewState);

UCLASS()
class LUNAR_API ULunarDraggableWindow : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Lunar|Draggable Window")
	void SetWindowPosition(FVector2D NewPosition);

	UFUNCTION(BlueprintPure, Category = "Lunar|Draggable Window")
	FVector2D GetWindowPosition() const;

	UFUNCTION(BlueprintCallable, Category = "Lunar|Draggable Window")
	void SetWindowSize(FVector2D NewSize);

	UFUNCTION(BlueprintPure, Category = "Lunar|Draggable Window")
	FVector2D GetWindowSize() const;

	UFUNCTION(BlueprintCallable, Category = "Lunar|Draggable Window")
	void SetBoundsWidget(UWidget* NewBoundsWidget);

	UFUNCTION(BlueprintCallable, Category = "Lunar|Draggable Window")
	void RefreshWindowLayout();

	UFUNCTION(BlueprintPure, Category = "Lunar|Draggable Window|Drag")
	bool IsDragging() const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Draggable Window|Resize")
	bool IsResizing() const;

	UFUNCTION(BlueprintPure, Category = "Lunar|Draggable Window|Input")
	bool IsInteracting() const;

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Lunar|Draggable Window|Widgets")
	TObjectPtr<UCanvasPanel> RootCanvas = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Lunar|Draggable Window|Widgets")
	TObjectPtr<USizeBox> WindowRoot = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Lunar|Draggable Window|Widgets")
	TObjectPtr<UBorder> HeadHitBox = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Lunar|Draggable Window|Widgets")
	TObjectPtr<UBorder> BodyHitBox = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Lunar|Draggable Window|Widgets")
	TObjectPtr<UBorder> ResizeHandle = nullptr;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Layout")
	FVector2D InitialWindowPosition = FVector2D(100.0f, 100.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Layout")
	FVector2D InitialWindowSize = FVector2D(600.0f, 400.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Drag")
	bool bDraggable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Drag")
	bool bAllowDragFromBody = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Bounds")
	ELunarDraggableWindowBoundsMode BoundsMode = ELunarDraggableWindowBoundsMode::Clamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Bounds")
	bool bUseParentAsBounds = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Bounds")
	TObjectPtr<UWidget> BoundsWidget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Bounds", meta = (ClampMin = "0.0"))
	float SoftClampOutsideMargin = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Snap")
	bool bEnableEdgeSnap = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Snap", meta = (ClampMin = "0.0"))
	float EdgeSnapDistance = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Resize")
	ELunarDraggableWindowResizeMode ResizeMode = ELunarDraggableWindowResizeMode::BottomRightHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Resize", meta = (ClampMin = "1.0"))
	float ResizeBorderThickness = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Resize", meta = (ClampMin = "1.0"))
	FVector2D ResizeHandleSize = FVector2D(24.0f, 24.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Resize")
	FVector2D MinWindowSize = FVector2D(300.0f, 200.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Resize")
	bool bUseMaxWindowSize = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Resize", meta = (EditCondition = "bUseMaxWindowSize", EditConditionHides))
	FVector2D MaxWindowSize = FVector2D(1600.0f, 900.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Resize")
	bool bClampResizeInsideBounds = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Resize")
	bool bRequireModifierForAnywhereResize = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Resize")
	FKey ResizeModifierKey = EKeys::LeftAlt;

protected:
	UPROPERTY(BlueprintAssignable, Category = "Lunar|Draggable Window|Events")
	FLunarDraggableWindowMoveStateChangedSignature OnMoveStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Lunar|Draggable Window|Events")
	FLunarDraggableWindowMoveStartedSignature OnMoveStarted;

	UPROPERTY(BlueprintAssignable, Category = "Lunar|Draggable Window|Events")
	FLunarDraggableWindowMovingSignature OnMoving;

	UPROPERTY(BlueprintAssignable, Category = "Lunar|Draggable Window|Events")
	FLunarDraggableWindowMoveStoppedSignature OnMoveStopped;

	UPROPERTY(BlueprintAssignable, Category = "Lunar|Draggable Window|Events")
	FLunarDraggableWindowResizeHandleStateChangedSignature OnResizeHandleStateChanged;

private:
	bool bIsDragging = false;
	bool bIsResizing = false;

	FVector2D DragOffset = FVector2D::ZeroVector;

	FVector2D CachedWindowPosition = FVector2D::ZeroVector;
	FVector2D CachedWindowSize = FVector2D::ZeroVector;

	FVector2D ResizeStartMousePosition = FVector2D::ZeroVector;
	FVector2D ResizeStartWindowPosition = FVector2D::ZeroVector;
	FVector2D ResizeStartWindowSize = FVector2D::ZeroVector;

	ELunarDraggableWindowResizeDirection ActiveResizeDirection = ELunarDraggableWindowResizeDirection::None;
	ELunarDraggableWindowHandleState ResizeHandleState = ELunarDraggableWindowHandleState::Normal;

private:
	void ApplyInitialLayout();
	void ApplyWindowPositionAndSize(FVector2D NewPosition, FVector2D NewSize);
	void UpdateResizeHandleLayout();
	void UpdateResizeHandleVisibility();

	bool IsLeftMouseButton(const FPointerEvent& InMouseEvent) const;
	bool IsModifierPressed(const FPointerEvent& InMouseEvent) const;

	bool IsWidgetUnderMouse(const UWidget* Widget, const FPointerEvent& InMouseEvent) const;
	bool IsHeadUnderMouse(const FPointerEvent& InMouseEvent) const;
	bool IsBodyUnderMouse(const FPointerEvent& InMouseEvent) const;
	bool IsResizeHandleUnderMouse(const FPointerEvent& InMouseEvent) const;

	bool CanStartDragFromMouseEvent(const FPointerEvent& InMouseEvent) const;
	bool CanStartResizeFromMouseEvent(const FPointerEvent& InMouseEvent, ELunarDraggableWindowResizeDirection& OutDirection) const;

	void StartDrag(const FPointerEvent& InMouseEvent);
	void StopDrag();

	void StartResize(const FPointerEvent& InMouseEvent, ELunarDraggableWindowResizeDirection Direction);
	void StopResize();

	void ProcessDrag(const FPointerEvent& InMouseEvent);
	void ProcessResize(const FPointerEvent& InMouseEvent);

	void SetResizeHandleState(ELunarDraggableWindowHandleState NewState);
	void UpdateResizeHandleStateFromMouse(const FPointerEvent& InMouseEvent);

	FVector2D GetMousePositionInRootCanvas(const FPointerEvent& InMouseEvent) const;

	ELunarDraggableWindowResizeDirection GetResizeDirectionFromWindowEdges(const FPointerEvent& InMouseEvent) const;

	bool GetBoundsRectInRootCanvas(FVector2D& OutBoundsPosition, FVector2D& OutBoundsSize) const;
	FVector2D ApplyBoundsToPosition(FVector2D Position, FVector2D Size) const;
	FVector2D ApplySnapToPosition(FVector2D Position, FVector2D Size) const;
	FVector2D ClampSize(FVector2D Position, FVector2D Size) const;

	UCanvasPanelSlot* GetWindowCanvasSlot() const;
	UCanvasPanelSlot* GetResizeHandleCanvasSlot() const;
};