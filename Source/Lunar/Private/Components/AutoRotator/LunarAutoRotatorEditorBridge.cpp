// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "Components/AutoRotator/LunarAutoRotatorEditorBridge.h"

/**
 * @file LunarAutoRotatorEditorBridge.cpp
 * @brief Storage for the editor viewport camera resolver delegate
 * @ingroup LunarAutoRotatorComponent
 */

#if WITH_EDITOR

FLunarResolveEditorViewportCameraLocation& GetLunarResolveEditorViewportCameraLocationDelegate()
{
	/** Shared resolver installed by the LunarEditor module. */
	static FLunarResolveEditorViewportCameraLocation Resolver;
	return Resolver;
}

#endif
