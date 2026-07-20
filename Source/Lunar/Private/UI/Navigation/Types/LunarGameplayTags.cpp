// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "UI/Navigation/Types/LunarGameplayTags.h"

/**
 * @file LunarGameplayTags.cpp
 * @brief Defines native semantic action and value-state Gameplay Tags.
 * @ingroup LunarNavigationTypes
 */

/** Native Gameplay Tags used by Lunar UI Navigation. */
namespace LunarGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Action_Navigate_Up, "Lunar.UI.Action.Navigate.Up", "Navigate to the eligible Lunar widget above the current selection.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Action_Navigate_Down, "Lunar.UI.Action.Navigate.Down", "Navigate to the eligible Lunar widget below the current selection.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Action_Navigate_Left, "Lunar.UI.Action.Navigate.Left", "Navigate to the eligible Lunar widget left of the current selection.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Action_Navigate_Right, "Lunar.UI.Action.Navigate.Right", "Navigate to the eligible Lunar widget right of the current selection.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Action_Accept, "Lunar.UI.Action.Accept", "Accept or activate the current Lunar UI selection.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Action_Back, "Lunar.UI.Action.Back", "Cancel the current interaction or close the top Lunar navigation scope.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Action_Increase, "Lunar.UI.Action.Increase", "Increase the value of the current Lunar control.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Action_Decrease, "Lunar.UI.Action.Decrease", "Decrease the value of the current Lunar control.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_Action_Selection_Toggle, "Lunar.UI.Action.Selection.Toggle", "Toggle selection of the active item in a multi-select Lunar control.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_State_Value_Normal, "Lunar.UI.State.Value.Normal", "Normal Lunar control value state.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_State_Value_Disabled, "Lunar.UI.State.Value.Disabled", "Disabled Lunar control value state.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_State_Value_Checked, "Lunar.UI.State.Value.Checked", "Checked Lunar control value state.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_State_Value_Unchecked, "Lunar.UI.State.Value.Unchecked", "Unchecked Lunar control value state.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_State_Value_Active, "Lunar.UI.State.Value.Active", "Active Lunar control value state.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_State_Value_Inactive, "Lunar.UI.State.Value.Inactive", "Inactive Lunar control value state.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_State_Value_On, "Lunar.UI.State.Value.On", "On Lunar control value state.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UI_State_Value_Off, "Lunar.UI.State.Value.Off", "Off Lunar control value state.");
}
