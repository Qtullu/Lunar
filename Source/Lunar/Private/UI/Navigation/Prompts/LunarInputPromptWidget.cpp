// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "UI/Navigation/Prompts/LunarInputPromptWidget.h"

/**
 * @file LunarInputPromptWidget.cpp
 * @brief Implements prompt snapshot storage for owner-authored presentation.
 * @ingroup LunarNavigationPrompts
 */

void ULunarInputPromptWidget::ApplyResolvedPromptActions_Implementation(
	const TArray<FLunarResolvedPromptAction>& Actions)
{
	ResolvedPromptActions = Actions;
}