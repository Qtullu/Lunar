// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarStyleResolver.h
 * @brief Declares private helpers that compose typed Lunar navigation styles.
 * @ingroup LunarNavigationStyles
 */

#pragma once

#include "CoreMinimal.h"
#include "UI/Navigation/Types/LunarNavigationTypes.h"
#include "UI/Navigation/Types/LunarUIStyleTypes.h"

class ULunarWidgetStyleAsset;
class ULunarScrollBoxStyleAsset;

/**
 * @brief Private, strongly typed style composition helpers shared by Lunar controls.
 * @ingroup LunarNavigationStyles
 */
namespace LunarStyleResolver
{
	/**
	 * @brief Applies every explicitly overridden source field on top of a common style patch.
	 * @param InOutStyle Accumulated style patch to update.
	 * @param Patch Authored patch whose enabled overrides are applied.
	 */
	void MergeCommonStylePatch(FLunarCommonStylePatch& InOutStyle, const FLunarCommonStylePatch& Patch);

	/**
	 * @brief Builds one displayed transition snapshot.
	 *
	 * Numeric, color, margin, vector, and transform values interpolate; discrete resources use Target immediately.
	 * @param Source Resolved style at the start of the transition.
	 * @param Target Resolved style at the end of the transition.
	 * @param Alpha Normalized transition progress; values outside zero-to-one are clamped.
	 * @return Interpolated common style snapshot.
	 */
	FLunarCommonStylePatch InterpolateCommonStylePatch(
		const FLunarCommonStylePatch& Source,
		const FLunarCommonStylePatch& Target,
		float Alpha);

	/**
	 * @brief Applies resource and transition fields that switch atomically at a transition endpoint.
	 * @param InOutStyle Displayed style snapshot to update.
	 * @param LogicalTarget Resolved logical target that supplies discrete fields.
	 */
	void ApplyCommonDiscreteFields(
		FLunarCommonStylePatch& InOutStyle,
		const FLunarCommonStylePatch& LogicalTarget);

	/**
	 * @brief Compares authored fields and destination transitions of two common style snapshots.
	 * @param A First resolved style snapshot.
	 * @param B Second resolved style snapshot.
	 * @return True when the two snapshots are equivalent.
	 */
	bool AreCommonStylesEquivalent(const FLunarCommonStylePatch& A, const FLunarCommonStylePatch& B);

	/**
	 * @brief Resolves a Button common style in this order:
	 * global Button default, assigned Button parent chain root-to-leaf, then Base/value/interaction/instance layers.
	 * AssignedStyle is intentionally generic so an incompatible assignment can produce an actionable error.
	 * @param AssignedStyle Optional instance-assigned style asset.
	 * @param VisualState Current value and interaction state.
	 * @param InstanceOverrides Per-widget common style overrides.
	 * @param OutStyle Receives the resolved common style.
	 * @param OutError Optional destination for an actionable resolution error.
	 * @return True when every referenced style is valid and resolution succeeds.
	 */
	bool ResolveButtonCommonStyle(
		const ULunarWidgetStyleAsset* AssignedStyle,
		const FLunarUIVisualState& VisualState,
		const FLunarCommonStylePatch& InstanceOverrides,
		FLunarCommonStylePatch& OutStyle,
		FString* OutError = nullptr);

	/**
	 * @brief Resolves the complete typed Slider style.
	 * @param AssignedStyle Optional instance-assigned style asset.
	 * @param VisualState Current value and interaction state.
	 * @param InstanceOverrides Per-widget common style overrides.
	 * @param OutStyle Receives the resolved Slider style.
	 * @param OutError Optional destination for an actionable resolution error.
	 * @return True when every referenced style is valid and resolution succeeds.
	 */
	bool ResolveSliderStyle(
		const ULunarWidgetStyleAsset* AssignedStyle,
		const FLunarUIVisualState& VisualState,
		const FLunarCommonStylePatch& InstanceOverrides,
		FLunarSliderStylePatch& OutStyle,
		FString* OutError = nullptr);

	/**
	 * @brief Resolves the complete typed OptionSlider style.
	 * @param AssignedStyle Optional instance-assigned style asset.
	 * @param VisualState Current value and interaction state.
	 * @param InstanceOverrides Per-widget common style overrides.
	 * @param OutStyle Receives the resolved OptionSlider style.
	 * @param OutError Optional destination for an actionable resolution error.
	 * @return True when every referenced style is valid and resolution succeeds.
	 */
	bool ResolveOptionSliderStyle(
		const ULunarWidgetStyleAsset* AssignedStyle,
		const FLunarUIVisualState& VisualState,
		const FLunarCommonStylePatch& InstanceOverrides,
		FLunarOptionSliderStylePatch& OutStyle,
		FString* OutError = nullptr);

	/**
	 * @brief Resolves the complete typed Switch style.
	 * @param AssignedStyle Optional instance-assigned style asset.
	 * @param VisualState Current value and interaction state.
	 * @param InstanceOverrides Per-widget common style overrides.
	 * @param OutStyle Receives the resolved Switch style.
	 * @param OutError Optional destination for an actionable resolution error.
	 * @return True when every referenced style is valid and resolution succeeds.
	 */
	bool ResolveSwitchStyle(
		const ULunarWidgetStyleAsset* AssignedStyle,
		const FLunarUIVisualState& VisualState,
		const FLunarCommonStylePatch& InstanceOverrides,
		FLunarSwitchStylePatch& OutStyle,
		FString* OutError = nullptr);

	/**
	 * @brief Resolves the complete typed Radio style.
	 * @param AssignedStyle Optional instance-assigned style asset.
	 * @param VisualState Current value and interaction state.
	 * @param InstanceOverrides Per-widget common style overrides.
	 * @param OutStyle Receives the resolved Radio style.
	 * @param OutError Optional destination for an actionable resolution error.
	 * @return True when every referenced style is valid and resolution succeeds.
	 */
	bool ResolveRadioStyle(
		const ULunarWidgetStyleAsset* AssignedStyle,
		const FLunarUIVisualState& VisualState,
		const FLunarCommonStylePatch& InstanceOverrides,
		FLunarRadioStylePatch& OutStyle,
		FString* OutError = nullptr);

	/**
	 * @brief Resolves the complete typed ListView style.
	 * @param AssignedStyle Optional instance-assigned style asset.
	 * @param VisualState Current value and interaction state.
	 * @param InstanceOverrides Per-widget common style overrides.
	 * @param OutStyle Receives the resolved ListView style.
	 * @param OutError Optional destination for an actionable resolution error.
	 * @return True when every referenced style is valid and resolution succeeds.
	 */
	bool ResolveListViewStyle(
		const ULunarWidgetStyleAsset* AssignedStyle,
		const FLunarUIVisualState& VisualState,
		const FLunarCommonStylePatch& InstanceOverrides,
		FLunarListViewStylePatch& OutStyle,
		FString* OutError = nullptr);

	/**
	 * @brief Resolves the complete typed ComboBox style.
	 * @param AssignedStyle Optional instance-assigned style asset.
	 * @param VisualState Current value and interaction state.
	 * @param InstanceOverrides Per-widget common style overrides.
	 * @param OutStyle Receives the resolved ComboBox style.
	 * @param OutError Optional destination for an actionable resolution error.
	 * @return True when every referenced style is valid and resolution succeeds.
	 */
	bool ResolveComboBoxStyle(
		const ULunarWidgetStyleAsset* AssignedStyle,
		const FLunarUIVisualState& VisualState,
		const FLunarCommonStylePatch& InstanceOverrides,
		FLunarComboBoxStylePatch& OutStyle,
		FString* OutError = nullptr);

	/**
	 * @brief Resolves the complete typed ContextMenu style.
	 * @param AssignedStyle Optional instance-assigned style asset.
	 * @param VisualState Current value and interaction state.
	 * @param InstanceOverrides Per-widget common style overrides.
	 * @param OutStyle Receives the resolved ContextMenu style.
	 * @param OutError Optional destination for an actionable resolution error.
	 * @return True when every referenced style is valid and resolution succeeds.
	 */
	bool ResolveContextMenuStyle(
		const ULunarWidgetStyleAsset* AssignedStyle,
		const FLunarUIVisualState& VisualState,
		const FLunarCommonStylePatch& InstanceOverrides,
		FLunarContextMenuStylePatch& OutStyle,
		FString* OutError = nullptr);

	/**
	 * @brief Resolves the complete typed Tabs style.
	 * @param AssignedStyle Optional instance-assigned style asset.
	 * @param VisualState Current value and interaction state.
	 * @param InstanceOverrides Per-widget common style overrides.
	 * @param OutStyle Receives the resolved Tabs style.
	 * @param OutError Optional destination for an actionable resolution error.
	 * @return True when every referenced style is valid and resolution succeeds.
	 */
	bool ResolveTabsStyle(
		const ULunarWidgetStyleAsset* AssignedStyle,
		const FLunarUIVisualState& VisualState,
		const FLunarCommonStylePatch& InstanceOverrides,
		FLunarTabsStylePatch& OutStyle,
		FString* OutError = nullptr);

	/**
	 * @brief Resolves the complete typed input-prompt style.
	 * @param AssignedStyle Optional instance-assigned style asset.
	 * @param VisualState Current value and interaction state.
	 * @param InstanceOverrides Per-widget common style overrides.
	 * @param OutStyle Receives the resolved input-prompt style.
	 * @param OutError Optional destination for an actionable resolution error.
	 * @return True when every referenced style is valid and resolution succeeds.
	 */
	bool ResolveInputPromptStyle(
		const ULunarWidgetStyleAsset* AssignedStyle,
		const FLunarUIVisualState& VisualState,
		const FLunarCommonStylePatch& InstanceOverrides,
		FLunarInputPromptStylePatch& OutStyle,
		FString* OutError = nullptr);

	/**
	 * @brief Resolves a complete ScrollBox patch in this order:
	 * global ScrollBox default, assigned ScrollBox parent chain root-to-leaf, then Base/value/interaction layers.
	 * ParentStyle remains generic on the asset base class, so incompatible parents are rejected with an actionable error.
	 * @param AssignedStyle Optional instance-assigned ScrollBox style asset.
	 * @param VisualState Current value and interaction state.
	 * @param OutStyle Receives the resolved ScrollBox style.
	 * @param OutError Optional destination for an actionable resolution error.
	 * @return True when every referenced style is valid and resolution succeeds.
	 */
	bool ResolveScrollBoxStyle(
		const ULunarScrollBoxStyleAsset* AssignedStyle,
		const FLunarUIVisualState& VisualState,
		FLunarScrollBoxStylePatch& OutStyle,
		FString* OutError = nullptr);
}
