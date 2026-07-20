// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "NativeGameplayTags.h"

/**
 * @file LunarGameplayTags.h
 * @brief Native Gameplay Tags used by Lunar UI Navigation.
 * @ingroup LunarNavigationTypes
 */

/** @brief Native semantic action and value-state Gameplay Tags. @ingroup LunarNavigationTypes */
namespace LunarGameplayTags
{
	/** @brief Navigate toward the top of the active scope. */
	LUNAR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Action_Navigate_Up);
	/** @brief Navigate toward the bottom of the active scope. */
	LUNAR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Action_Navigate_Down);
	/** @brief Navigate toward the left of the active scope. */
	LUNAR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Action_Navigate_Left);
	/** @brief Navigate toward the right of the active scope. */
	LUNAR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Action_Navigate_Right);
	/** @brief Accept or activate the selected control. */
	LUNAR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Action_Accept);
	/** @brief Cancel the current interaction or close the top scope. */
	LUNAR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Action_Back);
	/** @brief Increase an orientation-aware value control. */
	LUNAR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Action_Increase);
	/** @brief Decrease an orientation-aware value control. */
	LUNAR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Action_Decrease);
	/** @brief Toggle selection of the active item in a multi-select control. */
	LUNAR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Action_Selection_Toggle);

	/** @brief Normal control value state. */
	LUNAR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_State_Value_Normal);
	/** @brief Disabled control value state. */
	LUNAR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_State_Value_Disabled);
	/** @brief Checked control value state. */
	LUNAR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_State_Value_Checked);
	/** @brief Unchecked control value state. */
	LUNAR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_State_Value_Unchecked);
	/** @brief Active control value state. */
	LUNAR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_State_Value_Active);
	/** @brief Inactive control value state. */
	LUNAR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_State_Value_Inactive);
	/** @brief On control value state. */
	LUNAR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_State_Value_On);
	/** @brief Off control value state. */
	LUNAR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_State_Value_Off);
}
