// Copyright 2026 Edgar Frolenkov. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"
#include "LunarDraggableWindow.generated.h"

/**
 * @file LunarDraggableWindow.h
 * @brief Draggable and resizable window widget
 * @ingroup LunarDraggableWindow
 */

class UBorder;
class UCanvasPanel;
class UCanvasPanelSlot;
class USizeBox;
class UWidget;

/**
 * @brief Defines how draggable window bounds are applied
 * @ingroup LunarDraggableWindow
 */
UENUM(BlueprintType)
enum class ELunarDraggableWindowBoundsMode : uint8
{
	None UMETA(DisplayName = "None"),
	Clamp UMETA(DisplayName = "Clamp"),
	SoftClamp UMETA(DisplayName = "Soft Clamp")
};

/**
 * @brief Defines how draggable window resizing is handled
 * @ingroup LunarDraggableWindow
 */
UENUM(BlueprintType)
enum class ELunarDraggableWindowResizeMode : uint8
{
	None UMETA(DisplayName = "None"),
	BottomRightHandle UMETA(DisplayName = "Bottom Right Handle"),
	EdgesAndCorners UMETA(DisplayName = "Edges And Corners"),
	Anywhere UMETA(DisplayName = "Anywhere")
};

/**
 * @brief Defines active resize direction for draggable window
 * @ingroup LunarDraggableWindow
 */
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

/**
 * @brief Defines visual interaction state for draggable window handles
 * @ingroup LunarDraggableWindow
 */
UENUM(BlueprintType)
enum class ELunarDraggableWindowHandleState : uint8
{
	Normal UMETA(DisplayName = "Normal"),
	Hovered UMETA(DisplayName = "Hovered"),
	Pressed UMETA(DisplayName = "Pressed")
};

/**
 * @brief Called when draggable window moving state changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLunarDraggableWindowMoveStateChangedSignature, bool, bIsMoving);

/**
 * @brief Called when draggable window movement starts
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLunarDraggableWindowMoveStartedSignature, FVector2D, Position);

/**
 * @brief Called while draggable window is moving
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLunarDraggableWindowMovingSignature, FVector2D, Position);

/**
 * @brief Called when draggable window movement stops
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLunarDraggableWindowMoveStoppedSignature, FVector2D, Position);

/**
 * @brief Called when resize handle state changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLunarDraggableWindowResizeHandleStateChangedSignature, ELunarDraggableWindowHandleState, NewState);

/**
 * @brief User widget that supports window dragging resizing bounds and edge snapping
 * @ingroup LunarDraggableWindow
 */
UCLASS()
class LUNAR_API ULunarDraggableWindow : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * @brief Sets window position in root canvas space
	 * @param NewPosition New window position
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Draggable Window")
	void SetWindowPosition(FVector2D NewPosition);

	/**
	 * @brief Gets current window position in root canvas space
	 * @return Current window position
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Draggable Window")
	FVector2D GetWindowPosition() const;

	/**
	 * @brief Sets window size
	 * @param NewSize New window size
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Draggable Window")
	void SetWindowSize(FVector2D NewSize);

	/**
	 * @brief Gets current window size
	 * @return Current window size
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Draggable Window")
	FVector2D GetWindowSize() const;

	/**
	 * @brief Sets widget used as movement bounds
	 * @param NewBoundsWidget Bounds widget
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Draggable Window")
	void SetBoundsWidget(UWidget* NewBoundsWidget);

	/**
	 * @brief Refreshes window layout and handle layout
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|Draggable Window")
	void RefreshWindowLayout();

	/**
	 * @brief Checks whether window is being dragged
	 * @return True if window is being dragged
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Draggable Window|Drag")
	bool IsDragging() const;

	/**
	 * @brief Checks whether window is being resized
	 * @return True if window is being resized
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Draggable Window|Resize")
	bool IsResizing() const;

	/**
	 * @brief Checks whether window is being dragged or resized
	 * @return True if window is being interacted with
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|Draggable Window|Input")
	bool IsInteracting() const;

protected:
	/**
	 * @brief Applies editor preview layout state
	 */
	virtual void NativePreConstruct() override;

	/**
	 * @brief Initializes draggable window runtime state
	 */
	virtual void NativeConstruct() override;

	/**
	 * @brief Handles mouse button down input
	 * @param InGeometry Widget geometry
	 * @param InMouseEvent Mouse event
	 * @return Input reply
	 */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/**
	 * @brief Handles mouse button up input
	 * @param InGeometry Widget geometry
	 * @param InMouseEvent Mouse event
	 * @return Input reply
	 */
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/**
	 * @brief Handles mouse movement input
	 * @param InGeometry Widget geometry
	 * @param InMouseEvent Mouse event
	 * @return Input reply
	 */
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/**
	 * @brief Handles mouse leave input
	 * @param InMouseEvent Mouse event
	 */
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

protected:
	/** Root canvas used for layout calculations */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Lunar|Draggable Window|Widgets")
	TObjectPtr<UCanvasPanel> RootCanvas = nullptr;

	/** Window root size box */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Lunar|Draggable Window|Widgets")
	TObjectPtr<USizeBox> WindowRoot = nullptr;

	/** Header hit box used for dragging */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Lunar|Draggable Window|Widgets")
	TObjectPtr<UBorder> HeadHitBox = nullptr;

	/** Body hit box used for optional dragging */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Lunar|Draggable Window|Widgets")
	TObjectPtr<UBorder> BodyHitBox = nullptr;

	/** Optional resize handle widget */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Lunar|Draggable Window|Widgets")
	TObjectPtr<UBorder> ResizeHandle = nullptr;

protected:
	/** Initial window position */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Layout")
	FVector2D InitialWindowPosition = FVector2D(100.0f, 100.0f);

	/** Initial window size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Layout")
	FVector2D InitialWindowSize = FVector2D(600.0f, 400.0f);

	/** Enables window dragging */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Drag")
	bool bDraggable = true;

	/** Allows dragging from body hit box */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Drag")
	bool bAllowDragFromBody = false;

	/** Bounds behavior mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Bounds")
	ELunarDraggableWindowBoundsMode BoundsMode = ELunarDraggableWindowBoundsMode::Clamp;

	/** Uses parent widget as bounds area */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Bounds")
	bool bUseParentAsBounds = true;

	/** Custom bounds widget */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Bounds")
	TObjectPtr<UWidget> BoundsWidget = nullptr;

	/** Distance allowed outside bounds when soft clamp is used */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Bounds", meta = (ClampMin = "0.0"))
	float SoftClampOutsideMargin = 80.0f;

	/** Enables snapping window position to bounds edges */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Snap")
	bool bEnableEdgeSnap = false;

	/** Distance from edge required for snapping */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Snap", meta = (ClampMin = "0.0"))
	float EdgeSnapDistance = 12.0f;

	/** Resize behavior mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Resize")
	ELunarDraggableWindowResizeMode ResizeMode = ELunarDraggableWindowResizeMode::BottomRightHandle;

	/** Edge resize hit thickness */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Resize", meta = (ClampMin = "1.0"))
	float ResizeBorderThickness = 8.0f;

	/** Resize handle size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Resize", meta = (ClampMin = "1.0"))
	FVector2D ResizeHandleSize = FVector2D(24.0f, 24.0f);

	/** Minimum allowed window size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Resize")
	FVector2D MinWindowSize = FVector2D(300.0f, 200.0f);

	/** Enables maximum window size limit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Resize")
	bool bUseMaxWindowSize = false;

	/** Maximum allowed window size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Resize", meta = (EditCondition = "bUseMaxWindowSize", EditConditionHides))
	FVector2D MaxWindowSize = FVector2D(1600.0f, 900.0f);

	/** Keeps resize result inside bounds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Resize")
	bool bClampResizeInsideBounds = true;

	/** Requires modifier key for anywhere resize mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Resize")
	bool bRequireModifierForAnywhereResize = true;

	/** Modifier key used for anywhere resize mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|Draggable Window|Resize")
	FKey ResizeModifierKey = EKeys::LeftAlt;

protected:
	/** Window move state changed event */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|Draggable Window|Events")
	FLunarDraggableWindowMoveStateChangedSignature OnMoveStateChanged;

	/** Window move started event */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|Draggable Window|Events")
	FLunarDraggableWindowMoveStartedSignature OnMoveStarted;

	/** Window moving event */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|Draggable Window|Events")
	FLunarDraggableWindowMovingSignature OnMoving;

	/** Window move stopped event */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|Draggable Window|Events")
	FLunarDraggableWindowMoveStoppedSignature OnMoveStopped;

	/** Resize handle state changed event */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|Draggable Window|Events")
	FLunarDraggableWindowResizeHandleStateChangedSignature OnResizeHandleStateChanged;

private:
	/** Current drag state */
	bool bIsDragging = false;

	/** Current resize state */
	bool bIsResizing = false;

	/** Mouse offset from window position when drag starts */
	FVector2D DragOffset = FVector2D::ZeroVector;

	/** Cached window position */
	FVector2D CachedWindowPosition = FVector2D::ZeroVector;

	/** Cached window size */
	FVector2D CachedWindowSize = FVector2D::ZeroVector;

	/** Mouse position when resize starts */
	FVector2D ResizeStartMousePosition = FVector2D::ZeroVector;

	/** Window position when resize starts */
	FVector2D ResizeStartWindowPosition = FVector2D::ZeroVector;

	/** Window size when resize starts */
	FVector2D ResizeStartWindowSize = FVector2D::ZeroVector;

	/** Active resize direction */
	ELunarDraggableWindowResizeDirection ActiveResizeDirection = ELunarDraggableWindowResizeDirection::None;

	/** Current resize handle state */
	ELunarDraggableWindowHandleState ResizeHandleState = ELunarDraggableWindowHandleState::Normal;

private:
	/**
	 * @brief Applies initial window layout
	 */
	void ApplyInitialLayout();

	/**
	 * @brief Applies window position and size to canvas slot
	 * @param NewPosition New window position
	 * @param NewSize New window size
	 */
	void ApplyWindowPositionAndSize(FVector2D NewPosition, FVector2D NewSize);

	/**
	 * @brief Updates resize handle layout
	 */
	void UpdateResizeHandleLayout();

	/**
	 * @brief Updates resize handle visibility
	 */
	void UpdateResizeHandleVisibility();

	/**
	 * @brief Checks whether mouse event uses left mouse button
	 * @param InMouseEvent Mouse event
	 * @return True if left mouse button is used
	 */
	bool IsLeftMouseButton(const FPointerEvent& InMouseEvent) const;

	/**
	 * @brief Checks whether resize modifier key is pressed
	 * @param InMouseEvent Mouse event
	 * @return True if resize modifier key is pressed
	 */
	bool IsModifierPressed(const FPointerEvent& InMouseEvent) const;

	/**
	 * @brief Checks whether widget is under mouse
	 * @param Widget Widget to check
	 * @param InMouseEvent Mouse event
	 * @return True if widget is under mouse
	 */
	bool IsWidgetUnderMouse(const UWidget* Widget, const FPointerEvent& InMouseEvent) const;

	/**
	 * @brief Checks whether header hit box is under mouse
	 * @param InMouseEvent Mouse event
	 * @return True if header hit box is under mouse
	 */
	bool IsHeadUnderMouse(const FPointerEvent& InMouseEvent) const;

	/**
	 * @brief Checks whether body hit box is under mouse
	 * @param InMouseEvent Mouse event
	 * @return True if body hit box is under mouse
	 */
	bool IsBodyUnderMouse(const FPointerEvent& InMouseEvent) const;

	/**
	 * @brief Checks whether resize handle is under mouse
	 * @param InMouseEvent Mouse event
	 * @return True if resize handle is under mouse
	 */
	bool IsResizeHandleUnderMouse(const FPointerEvent& InMouseEvent) const;

	/**
	 * @brief Checks whether drag can start from mouse event
	 * @param InMouseEvent Mouse event
	 * @return True if drag can start
	 */
	bool CanStartDragFromMouseEvent(const FPointerEvent& InMouseEvent) const;

	/**
	 * @brief Checks whether resize can start from mouse event
	 * @param InMouseEvent Mouse event
	 * @param OutDirection Resolved resize direction
	 * @return True if resize can start
	 */
	bool CanStartResizeFromMouseEvent(const FPointerEvent& InMouseEvent, ELunarDraggableWindowResizeDirection& OutDirection) const;

	/**
	 * @brief Starts window drag
	 * @param InMouseEvent Mouse event
	 */
	void StartDrag(const FPointerEvent& InMouseEvent);

	/**
	 * @brief Stops window drag
	 */
	void StopDrag();

	/**
	 * @brief Starts window resize
	 * @param InMouseEvent Mouse event
	 * @param Direction Resize direction
	 */
	void StartResize(const FPointerEvent& InMouseEvent, ELunarDraggableWindowResizeDirection Direction);

	/**
	 * @brief Stops window resize
	 */
	void StopResize();

	/**
	 * @brief Processes active window drag
	 * @param InMouseEvent Mouse event
	 */
	void ProcessDrag(const FPointerEvent& InMouseEvent);

	/**
	 * @brief Processes active window resize
	 * @param InMouseEvent Mouse event
	 */
	void ProcessResize(const FPointerEvent& InMouseEvent);

	/**
	 * @brief Sets resize handle state
	 * @param NewState New handle state
	 */
	void SetResizeHandleState(ELunarDraggableWindowHandleState NewState);

	/**
	 * @brief Updates resize handle state from mouse position
	 * @param InMouseEvent Mouse event
	 */
	void UpdateResizeHandleStateFromMouse(const FPointerEvent& InMouseEvent);

	/**
	 * @brief Gets mouse position in root canvas space
	 * @param InMouseEvent Mouse event
	 * @return Mouse position in root canvas space
	 */
	FVector2D GetMousePositionInRootCanvas(const FPointerEvent& InMouseEvent) const;

	/**
	 * @brief Gets resize direction from window edges
	 * @param InMouseEvent Mouse event
	 * @return Resize direction
	 */
	ELunarDraggableWindowResizeDirection GetResizeDirectionFromWindowEdges(const FPointerEvent& InMouseEvent) const;

	/**
	 * @brief Gets bounds rectangle in root canvas space
	 * @param OutBoundsPosition Bounds position
	 * @param OutBoundsSize Bounds size
	 * @return True if bounds were resolved
	 */
	bool GetBoundsRectInRootCanvas(FVector2D& OutBoundsPosition, FVector2D& OutBoundsSize) const;

	/**
	 * @brief Applies bounds to window position
	 * @param Position Window position
	 * @param Size Window size
	 * @return Bounded window position
	 */
	FVector2D ApplyBoundsToPosition(FVector2D Position, FVector2D Size) const;

	/**
	 * @brief Applies edge snapping to window position
	 * @param Position Window position
	 * @param Size Window size
	 * @return Snapped window position
	 */
	FVector2D ApplySnapToPosition(FVector2D Position, FVector2D Size) const;

	/**
	 * @brief Clamps window size
	 * @param Position Window position
	 * @param Size Window size
	 * @return Clamped window size
	 */
	FVector2D ClampSize(FVector2D Position, FVector2D Size) const;

	/**
	 * @brief Gets canvas slot for window root
	 * @return Window canvas slot or null
	 */
	UCanvasPanelSlot* GetWindowCanvasSlot() const;

	/**
	 * @brief Gets canvas slot for resize handle
	 * @return Resize handle canvas slot or null
	 */
	UCanvasPanelSlot* GetResizeHandleCanvasSlot() const;
};