// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "UI/Navigation/Types/LunarNavigationSettings.h"

#include "InputCoreTypes.h"
#include "UI/Navigation/Types/LunarGameplayTags.h"

/**
 * @file LunarNavigationSettings.cpp
 * @brief Default semantic action and behavior settings for Lunar UI navigation
 * @ingroup LunarNavigationTypes
 */

/** Localization namespace used by the built-in action labels. */
#define LOCTEXT_NAMESPACE "LunarNavigationSettings"

namespace LunarNavigationSettings_Private
{
	/**
	 * @brief Creates one semantic UI action definition from a set of input keys
	 * @param ActionTag Semantic action tag represented by the definition
	 * @param DisplayText Localized text displayed for the action
	 * @param Keys Physical keys bound to the action by default
	 * @return Initialized semantic UI action definition
	 */
	static FLunarUIActionDefinition MakeActionDefinition(
		const FGameplayTag ActionTag,
		const FText& DisplayText,
		const TArray<FKey>& Keys)
	{
		FLunarUIActionDefinition Definition;
		Definition.ActionTag = ActionTag;
		Definition.DisplayText = DisplayText;
		Definition.Bindings.Reserve(Keys.Num());

		for (const FKey& Key : Keys)
		{
			FLunarUIActionBinding& Binding = Definition.Bindings.AddDefaulted_GetRef();
			Binding.Key = Key;
			Binding.bEnabled = true;
		}

		return Definition;
	}
}

FLunarNavigationInputSettings::FLunarNavigationInputSettings()
{
	using namespace LunarNavigationSettings_Private;

	ActionDefinitions.Reserve(9);
	ActionDefinitions.Add(MakeActionDefinition(
		LunarGameplayTags::UI_Action_Navigate_Up.GetTag(),
		LOCTEXT("NavigateUpAction", "Navigate Up"),
		{ EKeys::W, EKeys::Up, EKeys::Gamepad_DPad_Up, EKeys::Gamepad_LeftStick_Up }));
	ActionDefinitions.Add(MakeActionDefinition(
		LunarGameplayTags::UI_Action_Navigate_Down.GetTag(),
		LOCTEXT("NavigateDownAction", "Navigate Down"),
		{ EKeys::S, EKeys::Down, EKeys::Gamepad_DPad_Down, EKeys::Gamepad_LeftStick_Down }));
	ActionDefinitions.Add(MakeActionDefinition(
		LunarGameplayTags::UI_Action_Navigate_Left.GetTag(),
		LOCTEXT("NavigateLeftAction", "Navigate Left"),
		{ EKeys::A, EKeys::Left, EKeys::Gamepad_DPad_Left, EKeys::Gamepad_LeftStick_Left }));
	ActionDefinitions.Add(MakeActionDefinition(
		LunarGameplayTags::UI_Action_Navigate_Right.GetTag(),
		LOCTEXT("NavigateRightAction", "Navigate Right"),
		{ EKeys::D, EKeys::Right, EKeys::Gamepad_DPad_Right, EKeys::Gamepad_LeftStick_Right }));
	ActionDefinitions.Add(MakeActionDefinition(
		LunarGameplayTags::UI_Action_Accept.GetTag(),
		LOCTEXT("AcceptAction", "Accept"),
		{ EKeys::Enter, EKeys::SpaceBar, EKeys::Gamepad_FaceButton_Bottom }));
	ActionDefinitions.Add(MakeActionDefinition(
		LunarGameplayTags::UI_Action_Back.GetTag(),
		LOCTEXT("BackAction", "Back"),
		{ EKeys::Escape, EKeys::Gamepad_FaceButton_Right }));
	ActionDefinitions.Add(MakeActionDefinition(
		LunarGameplayTags::UI_Action_Increase.GetTag(),
		LOCTEXT("IncreaseAction", "Increase"),
		{
			EKeys::W,
			EKeys::Up,
			EKeys::D,
			EKeys::Right,
			EKeys::Gamepad_DPad_Up,
			EKeys::Gamepad_DPad_Right,
			EKeys::Gamepad_LeftStick_Up,
			EKeys::Gamepad_LeftStick_Right
		}));
	ActionDefinitions.Add(MakeActionDefinition(
		LunarGameplayTags::UI_Action_Decrease.GetTag(),
		LOCTEXT("DecreaseAction", "Decrease"),
		{
			EKeys::S,
			EKeys::Down,
			EKeys::A,
			EKeys::Left,
			EKeys::Gamepad_DPad_Down,
			EKeys::Gamepad_DPad_Left,
			EKeys::Gamepad_LeftStick_Down,
			EKeys::Gamepad_LeftStick_Left
		}));
	ActionDefinitions.Add(MakeActionDefinition(
		LunarGameplayTags::UI_Action_Selection_Toggle.GetTag(),
		LOCTEXT("ToggleSelectionAction", "Toggle Selection"),
		{ EKeys::Gamepad_FaceButton_Left }));
}

FLunarNavigationBehaviorSettings::FLunarNavigationBehaviorSettings()
{
	DefaultPointerPolicy.CursorVisibilityPolicy = ELunarCursorVisibilityPolicy::AutoHideOnNavigation;
	DefaultPointerPolicy.PointerCapturePolicy = ELunarPointerCapturePolicy::Release;
}

#undef LOCTEXT_NAMESPACE
