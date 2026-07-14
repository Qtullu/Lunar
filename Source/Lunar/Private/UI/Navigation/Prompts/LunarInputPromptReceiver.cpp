// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "UI/Navigation/Prompts/LunarInputPromptReceiver.h"

/**
 * @file LunarInputPromptReceiver.cpp
 * @brief Implements the default input-prompt receiver contract.
 * @ingroup LunarNavigationPrompts
 */

void ILunarInputPromptReceiver::ApplyResolvedPromptActions_Implementation(
	const TArray<FLunarResolvedPromptAction>& Actions)
{
	(void)Actions;
}
