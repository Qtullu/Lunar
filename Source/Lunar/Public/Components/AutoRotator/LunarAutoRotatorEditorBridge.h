// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * @file LunarAutoRotatorEditorBridge.h
 * @brief Runtime-facing delegate bridge for editor viewport camera queries
 * @ingroup LunarAutoRotatorComponent
 */

#if WITH_EDITOR

/**
 * @brief Resolves the active editor viewport camera without adding an UnrealEd dependency to Lunar
 * @param OutCameraLocation Receives the active editor viewport camera location
 * @return True when an editor viewport camera location was resolved
 * @ingroup LunarAutoRotatorComponent
 */
DECLARE_DELEGATE_RetVal_OneParam(bool, FLunarResolveEditorViewportCameraLocation, FVector&);

/**
 * @brief Gets the editor viewport camera resolver registered by LunarEditor
 * @return Shared editor viewport camera resolver delegate
 * @ingroup LunarAutoRotatorComponent
 */
LUNAR_API FLunarResolveEditorViewportCameraLocation& GetLunarResolveEditorViewportCameraLocationDelegate();

#endif
