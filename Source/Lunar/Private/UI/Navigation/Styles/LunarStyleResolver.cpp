// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarStyleResolver.cpp
 * @brief Implements layered style composition for Lunar navigation controls.
 * @ingroup LunarNavigationStyles
 */

#include "UI/Navigation/Styles/LunarStyleResolver.h"

#include "Algo/Reverse.h"
#include "Settings/LunarSettings.h"
#include "Styling/WidgetStyle.h"
#include "UI/Navigation/Styles/LunarButtonStyleAsset.h"
#include "UI/Navigation/Styles/LunarComboBoxStyleAsset.h"
#include "UI/Navigation/Styles/LunarContextMenuStyleAsset.h"
#include "UI/Navigation/Styles/LunarInputPromptStyleAsset.h"
#include "UI/Navigation/Styles/LunarListViewStyleAsset.h"
#include "UI/Navigation/Styles/LunarOptionSliderStyleAsset.h"
#include "UI/Navigation/Styles/LunarRadioStyleAsset.h"
#include "UI/Navigation/Styles/LunarScrollBoxStyleAsset.h"
#include "UI/Navigation/Styles/LunarSliderStyleAsset.h"
#include "UI/Navigation/Styles/LunarSwitchStyleAsset.h"
#include "UI/Navigation/Styles/LunarTabsStyleAsset.h"
#include "UI/Navigation/Styles/LunarWidgetStyleAsset.h"

DEFINE_LOG_CATEGORY_STATIC(LogLunarStyleResolver, Log, All);

namespace
{
	/**
	 * @brief Tests whether a common patch authors at least one overridable visual field.
	 * @param Patch Patch to inspect.
	 * @return True when at least one common override is enabled.
	 */
	bool HasAnyCommonOverride(const FLunarCommonStylePatch& Patch)
	{
		return Patch.bOverrideBackgroundBrush
			|| Patch.bOverrideBorderBrush
			|| Patch.bOverrideForegroundBrush
			|| Patch.bOverrideBackgroundTint
			|| Patch.bOverrideContentColor
			|| Patch.bOverrideTextColor
			|| Patch.bOverrideFont
			|| Patch.bOverrideTextShadowColor
			|| Patch.bOverrideTextShadowOffset
			|| Patch.bOverridePadding
			|| Patch.bOverrideContentPadding
			|| Patch.bOverrideOpacity
			|| Patch.bOverrideRenderTransform
			|| Patch.bOverrideRenderTransformPivot
			|| Patch.bOverrideMinDesiredSize
			|| Patch.bOverrideMaxDesiredSize;
	}

	/**
	 * @brief Tests whether a common patch authors visual or transition data.
	 * @param Patch Patch to inspect.
	 * @return True when the patch should replace an inherited transition.
	 */
	bool HasAuthoredTransition(const FLunarCommonStylePatch& Patch)
	{
		return HasAnyCommonOverride(Patch)
			|| Patch.Transition.bEnabled
			|| Patch.Transition.Duration > 0.0f
			|| Patch.Transition.Easing != EEasingFunc::Linear;
	}

	/**
	 * @brief Interpolates every edge of a Slate margin.
	 * @param Source Starting margin.
	 * @param Target Destination margin.
	 * @param Alpha Normalized interpolation progress.
	 * @return Interpolated margin.
	 */
	FMargin InterpolateMargin(const FMargin& Source, const FMargin& Target, const float Alpha)
	{
		return FMargin(
			FMath::Lerp(Source.Left, Target.Left, Alpha),
			FMath::Lerp(Source.Top, Target.Top, Alpha),
			FMath::Lerp(Source.Right, Target.Right, Alpha),
			FMath::Lerp(Source.Bottom, Target.Bottom, Alpha));
	}

	/**
	 * @brief Interpolates every component of a widget render transform.
	 * @param Source Starting transform.
	 * @param Target Destination transform.
	 * @param Alpha Normalized interpolation progress.
	 * @return Interpolated widget transform.
	 */
	FWidgetTransform InterpolateWidgetTransform(
		const FWidgetTransform& Source,
		const FWidgetTransform& Target,
		const float Alpha)
	{
		return FWidgetTransform(
			FMath::Lerp(Source.Translation, Target.Translation, Alpha),
			FMath::Lerp(Source.Scale, Target.Scale, Alpha),
			FMath::Lerp(Source.Shear, Target.Shear, Alpha),
			FMath::Lerp(Source.Angle, Target.Angle, Alpha));
	}

	/**
	 * @brief Resolves and interpolates two Slate colors against the default widget style.
	 * @param Source Starting Slate color.
	 * @param Target Destination Slate color.
	 * @param Alpha Normalized interpolation progress.
	 * @return Interpolated specified Slate color.
	 */
	FSlateColor InterpolateSlateColor(const FSlateColor& Source, const FSlateColor& Target, const float Alpha)
	{
		const FWidgetStyle DefaultWidgetStyle;
		return FSlateColor(FMath::Lerp(
			Source.GetColor(DefaultWidgetStyle),
			Target.GetColor(DefaultWidgetStyle),
			Alpha));
	}

	/**
	 * @brief Compares two authored transition settings.
	 * @param A First transition settings value.
	 * @param B Second transition settings value.
	 * @return True when enablement, duration, and easing match.
	 */
	bool AreTransitionsEquivalent(
		const FLunarStyleTransitionSettings& A,
		const FLunarStyleTransitionSettings& B)
	{
		return A.bEnabled == B.bEnabled
			&& A.Duration == B.Duration
			&& A.Easing == B.Easing;
	}

	/**
	 * @brief Stores and logs a style-resolution error.
	 * @param OutError Optional error-string destination.
	 * @param Message Actionable error message to report.
	 */
	void ReportResolutionError(FString* OutError, FString Message)
	{
		if (OutError)
		{
			*OutError = Message;
		}

		UE_LOG(LogLunarStyleResolver, Error, TEXT("%s"), *Message);
	}

	/**
	 * @brief Validates and orders a Button style inheritance chain.
	 * @param LeafStyle Optional leaf style from which to traverse parents.
	 * @param ChainName Human-readable chain label used in errors.
	 * @param OutRootToLeaf Receives compatible Button styles in merge order.
	 * @param OutError Optional destination for an actionable validation error.
	 * @return True when the chain contains no cycles or incompatible assets.
	 */
	bool BuildButtonStyleChain(
		const ULunarWidgetStyleAsset* LeafStyle,
		const TCHAR* ChainName,
		TArray<const ULunarButtonStyleAsset*>& OutRootToLeaf,
		FString* OutError)
	{
		TSet<const ULunarWidgetStyleAsset*> VisitedStyles;
		const ULunarWidgetStyleAsset* CurrentStyle = LeafStyle;

		while (CurrentStyle)
		{
			if (VisitedStyles.Contains(CurrentStyle))
			{
				ReportResolutionError(
					OutError,
					FString::Printf(
						TEXT("Button style parent cycle in %s: '%s' is referenced more than once."),
						ChainName,
						*CurrentStyle->GetPathName()));
				return false;
			}

			VisitedStyles.Add(CurrentStyle);

			const ULunarButtonStyleAsset* ButtonStyle = Cast<ULunarButtonStyleAsset>(CurrentStyle);
			if (!ButtonStyle)
			{
				ReportResolutionError(
					OutError,
					FString::Printf(
						TEXT("Incompatible style in %s at '%s': expected ULunarButtonStyleAsset, found %s."),
						ChainName,
						*CurrentStyle->GetPathName(),
						*GetNameSafe(CurrentStyle->GetClass())));
				return false;
			}

			OutRootToLeaf.Add(ButtonStyle);
			CurrentStyle = CurrentStyle->ParentStyle;
		}

		Algo::Reverse(OutRootToLeaf);
		return true;
	}

	/**
	 * @brief Merges Button base, value, and interaction layers from root to leaf.
	 * @param RootToLeafChain Validated Button style chain.
	 * @param VisualState Current value and interaction state.
	 * @param InOutBase Accumulated base layer.
	 * @param InOutValue Accumulated value-state layer.
	 * @param InOutInteraction Accumulated interaction-state layer.
	 */
	void MergeButtonChainLayers(
		const TArray<const ULunarButtonStyleAsset*>& RootToLeafChain,
		const FLunarUIVisualState& VisualState,
		FLunarCommonStylePatch& InOutBase,
		FLunarCommonStylePatch& InOutValue,
		FLunarCommonStylePatch& InOutInteraction)
	{
		for (const ULunarButtonStyleAsset* Style : RootToLeafChain)
		{
			LunarStyleResolver::MergeCommonStylePatch(InOutBase, Style->BaseStyle.Common);

			if (VisualState.ValueStateTag.IsValid())
			{
				if (const FLunarButtonStylePatch* ValuePatch = Style->ValueStateStyles.Find(VisualState.ValueStateTag))
				{
					LunarStyleResolver::MergeCommonStylePatch(InOutValue, ValuePatch->Common);
				}
			}

			if (const FLunarButtonStylePatch* InteractionPatch = Style->InteractionStateStyles.Find(VisualState.InteractionState))
			{
				LunarStyleResolver::MergeCommonStylePatch(InOutInteraction, InteractionPatch->Common);
			}
		}
	}

	/**
	 * @brief Validates and orders a ScrollBox style inheritance chain.
	 * @param LeafStyle Optional leaf style from which to traverse parents.
	 * @param ChainName Human-readable chain label used in errors.
	 * @param OutRootToLeaf Receives compatible ScrollBox styles in merge order.
	 * @param OutError Optional destination for an actionable validation error.
	 * @return True when the chain contains no invalid, cyclic, or incompatible assets.
	 */
	bool BuildScrollBoxStyleChain(
		const ULunarScrollBoxStyleAsset* LeafStyle,
		const TCHAR* ChainName,
		TArray<const ULunarScrollBoxStyleAsset*>& OutRootToLeaf,
		FString* OutError)
	{
		TSet<const ULunarWidgetStyleAsset*> VisitedStyles;
		const ULunarWidgetStyleAsset* CurrentStyle = LeafStyle;

		while (CurrentStyle)
		{
			if (!IsValid(CurrentStyle))
			{
				ReportResolutionError(
					OutError,
					FString::Printf(
						TEXT("Invalid or unavailable style object while resolving %s. Reassign the ScrollBox style or its parent."),
						ChainName));
				return false;
			}

			if (VisitedStyles.Contains(CurrentStyle))
			{
				ReportResolutionError(
					OutError,
					FString::Printf(
						TEXT("ScrollBox style parent cycle in %s: '%s' is referenced more than once."),
						ChainName,
						*CurrentStyle->GetPathName()));
				return false;
			}

			VisitedStyles.Add(CurrentStyle);

			const ULunarScrollBoxStyleAsset* ScrollBoxStyle = Cast<ULunarScrollBoxStyleAsset>(CurrentStyle);
			if (!ScrollBoxStyle)
			{
				ReportResolutionError(
					OutError,
					FString::Printf(
						TEXT("Incompatible style in %s at '%s': expected ULunarScrollBoxStyleAsset, found %s. Replace the parent with a ScrollBox style."),
						ChainName,
						*CurrentStyle->GetPathName(),
						*GetNameSafe(CurrentStyle->GetClass())));
				return false;
			}

			OutRootToLeaf.Add(ScrollBoxStyle);
			CurrentStyle = CurrentStyle->ParentStyle;
		}

		Algo::Reverse(OutRootToLeaf);
		return true;
	}

	/**
	 * @brief Applies enabled common and ScrollBox-specific overrides to an accumulated patch.
	 * @param InOutStyle Accumulated ScrollBox style patch.
	 * @param Patch Authored layer to apply.
	 */
	void MergeScrollBoxStylePatch(
		FLunarScrollBoxStylePatch& InOutStyle,
		const FLunarScrollBoxStylePatch& Patch)
	{
		const bool bHasSpecializedOverride = Patch.bOverrideScrollBarStyle
			|| Patch.bOverrideScrollBarPadding;
		LunarStyleResolver::MergeCommonStylePatch(InOutStyle.Common, Patch.Common);
		if (bHasSpecializedOverride)
		{
			InOutStyle.Common.Transition = Patch.Common.Transition;
		}

		if (Patch.bOverrideScrollBarStyle)
		{
			InOutStyle.bOverrideScrollBarStyle = true;
			InOutStyle.ScrollBarStyle = Patch.ScrollBarStyle;
		}

		if (Patch.bOverrideScrollBarPadding)
		{
			InOutStyle.bOverrideScrollBarPadding = true;
			InOutStyle.ScrollBarPadding = Patch.ScrollBarPadding;
		}
	}

	/**
	 * @brief Merges ScrollBox base, value, and interaction layers from root to leaf.
	 * @param RootToLeafChain Validated ScrollBox style chain.
	 * @param VisualState Current value and interaction state.
	 * @param InOutBase Accumulated base layer.
	 * @param InOutValue Accumulated value-state layer.
	 * @param InOutInteraction Accumulated interaction-state layer.
	 */
	void MergeScrollBoxChainLayers(
		const TArray<const ULunarScrollBoxStyleAsset*>& RootToLeafChain,
		const FLunarUIVisualState& VisualState,
		FLunarScrollBoxStylePatch& InOutBase,
		FLunarScrollBoxStylePatch& InOutValue,
		FLunarScrollBoxStylePatch& InOutInteraction)
	{
		for (const ULunarScrollBoxStyleAsset* Style : RootToLeafChain)
		{
			MergeScrollBoxStylePatch(InOutBase, Style->BaseStyle);

			if (VisualState.ValueStateTag.IsValid())
			{
				if (const FLunarScrollBoxStylePatch* ValuePatch = Style->ValueStateStyles.Find(VisualState.ValueStateTag))
				{
					MergeScrollBoxStylePatch(InOutValue, *ValuePatch);
				}
			}

			if (const FLunarScrollBoxStylePatch* InteractionPatch = Style->InteractionStateStyles.Find(VisualState.InteractionState))
			{
				MergeScrollBoxStylePatch(InOutInteraction, *InteractionPatch);
			}
		}
	}

	/**
	 * @brief Applies enabled common and Slider-specific overrides to an accumulated patch.
	 * @param InOutStyle Accumulated Slider style patch.
	 * @param Patch Authored layer to apply.
	 */
	void MergeSliderStylePatch(FLunarSliderStylePatch& InOutStyle, const FLunarSliderStylePatch& Patch)
	{
		const bool bHasSpecializedOverride = Patch.bOverrideTrackBrush
			|| Patch.bOverrideFillBrush
			|| Patch.bOverrideThumbBrush
			|| Patch.bOverrideTrackTint
			|| Patch.bOverrideFillTint
			|| Patch.bOverrideThumbTint
			|| Patch.bOverrideTrackThickness
			|| Patch.bOverrideThumbSize;
		LunarStyleResolver::MergeCommonStylePatch(InOutStyle.Common, Patch.Common);
		if (bHasSpecializedOverride)
		{
			InOutStyle.Common.Transition = Patch.Common.Transition;
		}
#define LUNAR_MERGE_SLIDER_FIELD(FieldName) \
		if (Patch.bOverride##FieldName) \
		{ \
			InOutStyle.bOverride##FieldName = true; \
			InOutStyle.FieldName = Patch.FieldName; \
		}
		LUNAR_MERGE_SLIDER_FIELD(TrackBrush)
		LUNAR_MERGE_SLIDER_FIELD(FillBrush)
		LUNAR_MERGE_SLIDER_FIELD(ThumbBrush)
		LUNAR_MERGE_SLIDER_FIELD(TrackTint)
		LUNAR_MERGE_SLIDER_FIELD(FillTint)
		LUNAR_MERGE_SLIDER_FIELD(ThumbTint)
		LUNAR_MERGE_SLIDER_FIELD(TrackThickness)
		LUNAR_MERGE_SLIDER_FIELD(ThumbSize)
#undef LUNAR_MERGE_SLIDER_FIELD
	}

	/**
	 * @brief Applies enabled common and OptionSlider-specific overrides to an accumulated patch.
	 * @param InOutStyle Accumulated OptionSlider style patch.
	 * @param Patch Authored layer to apply.
	 */
	void MergeOptionSliderStylePatch(
		FLunarOptionSliderStylePatch& InOutStyle,
		const FLunarOptionSliderStylePatch& Patch)
	{
		const bool bHasSpecializedOverride = Patch.bOverridePreviousArrowBrush
			|| Patch.bOverrideNextArrowBrush
			|| Patch.bOverridePreviousArrowTint
			|| Patch.bOverrideNextArrowTint
			|| Patch.bOverrideValueTextColor
			|| Patch.bOverrideValueFont;
		LunarStyleResolver::MergeCommonStylePatch(InOutStyle.Common, Patch.Common);
		if (bHasSpecializedOverride)
		{
			InOutStyle.Common.Transition = Patch.Common.Transition;
		}
#define LUNAR_MERGE_OPTION_SLIDER_FIELD(FieldName) \
		if (Patch.bOverride##FieldName) \
		{ \
			InOutStyle.bOverride##FieldName = true; \
			InOutStyle.FieldName = Patch.FieldName; \
		}
		LUNAR_MERGE_OPTION_SLIDER_FIELD(PreviousArrowBrush)
		LUNAR_MERGE_OPTION_SLIDER_FIELD(NextArrowBrush)
		LUNAR_MERGE_OPTION_SLIDER_FIELD(PreviousArrowTint)
		LUNAR_MERGE_OPTION_SLIDER_FIELD(NextArrowTint)
		LUNAR_MERGE_OPTION_SLIDER_FIELD(ValueTextColor)
		LUNAR_MERGE_OPTION_SLIDER_FIELD(ValueFont)
#undef LUNAR_MERGE_OPTION_SLIDER_FIELD
	}

	/**
	 * @brief Applies enabled common and Switch-specific overrides to an accumulated patch.
	 * @param InOutStyle Accumulated Switch style patch.
	 * @param Patch Authored layer to apply.
	 */
	void MergeSwitchStylePatch(FLunarSwitchStylePatch& InOutStyle, const FLunarSwitchStylePatch& Patch)
	{
		const bool bHasSpecializedOverride = Patch.bOverrideTrackBrush
			|| Patch.bOverrideHandleBrush
			|| Patch.bOverrideTrackTint
			|| Patch.bOverrideHandleTint
			|| Patch.bOverrideTrackSize
			|| Patch.bOverrideHandleSize
			|| Patch.bOverrideHandleOffset;
		LunarStyleResolver::MergeCommonStylePatch(InOutStyle.Common, Patch.Common);
		if (bHasSpecializedOverride)
		{
			InOutStyle.Common.Transition = Patch.Common.Transition;
		}
#define LUNAR_MERGE_SWITCH_FIELD(FieldName) \
		if (Patch.bOverride##FieldName) \
		{ \
			InOutStyle.bOverride##FieldName = true; \
			InOutStyle.FieldName = Patch.FieldName; \
		}
		LUNAR_MERGE_SWITCH_FIELD(TrackBrush)
		LUNAR_MERGE_SWITCH_FIELD(HandleBrush)
		LUNAR_MERGE_SWITCH_FIELD(TrackTint)
		LUNAR_MERGE_SWITCH_FIELD(HandleTint)
		LUNAR_MERGE_SWITCH_FIELD(TrackSize)
		LUNAR_MERGE_SWITCH_FIELD(HandleSize)
		LUNAR_MERGE_SWITCH_FIELD(HandleOffset)
#undef LUNAR_MERGE_SWITCH_FIELD
	}

	/**
	 * @brief Applies enabled common and Radio-specific overrides to an accumulated patch.
	 * @param InOutStyle Accumulated Radio style patch.
	 * @param Patch Authored layer to apply.
	 */
	void MergeRadioStylePatch(FLunarRadioStylePatch& InOutStyle, const FLunarRadioStylePatch& Patch)
	{
		const bool bHasSpecializedOverride = Patch.bOverrideMarkBrush
			|| Patch.bOverrideMarkTint
			|| Patch.bOverrideMarkSize;
		LunarStyleResolver::MergeCommonStylePatch(InOutStyle.Common, Patch.Common);
		if (bHasSpecializedOverride)
		{
			InOutStyle.Common.Transition = Patch.Common.Transition;
		}
#define LUNAR_MERGE_RADIO_FIELD(FieldName) \
		if (Patch.bOverride##FieldName) \
		{ \
			InOutStyle.bOverride##FieldName = true; \
			InOutStyle.FieldName = Patch.FieldName; \
		}
		LUNAR_MERGE_RADIO_FIELD(MarkBrush)
		LUNAR_MERGE_RADIO_FIELD(MarkTint)
		LUNAR_MERGE_RADIO_FIELD(MarkSize)
#undef LUNAR_MERGE_RADIO_FIELD
	}

	/**
	 * @brief Applies enabled common and ListView-specific overrides to an accumulated patch.
	 * @param InOutStyle Accumulated ListView style patch.
	 * @param Patch Authored layer to apply.
	 */
	void MergeListViewStylePatch(FLunarListViewStylePatch& InOutStyle, const FLunarListViewStylePatch& Patch)
	{
		const bool bHasSpecializedOverride = Patch.bOverrideRowStyle || Patch.bOverrideScrollBarStyle;
		LunarStyleResolver::MergeCommonStylePatch(InOutStyle.Common, Patch.Common);
		if (bHasSpecializedOverride)
		{
			InOutStyle.Common.Transition = Patch.Common.Transition;
		}
#define LUNAR_MERGE_LIST_VIEW_FIELD(FieldName) \
		if (Patch.bOverride##FieldName) \
		{ \
			InOutStyle.bOverride##FieldName = true; \
			InOutStyle.FieldName = Patch.FieldName; \
		}
		LUNAR_MERGE_LIST_VIEW_FIELD(RowStyle)
		LUNAR_MERGE_LIST_VIEW_FIELD(ScrollBarStyle)
#undef LUNAR_MERGE_LIST_VIEW_FIELD
	}

	/**
	 * @brief Applies enabled common and ComboBox-specific overrides to an accumulated patch.
	 * @param InOutStyle Accumulated ComboBox style patch.
	 * @param Patch Authored layer to apply.
	 */
	void MergeComboBoxStylePatch(FLunarComboBoxStylePatch& InOutStyle, const FLunarComboBoxStylePatch& Patch)
	{
		const bool bHasSpecializedOverride = Patch.bOverrideArrowBrush
			|| Patch.bOverridePopupBrush
			|| Patch.bOverrideRowStyle
			|| Patch.bOverrideScrollBarStyle;
		LunarStyleResolver::MergeCommonStylePatch(InOutStyle.Common, Patch.Common);
		if (bHasSpecializedOverride)
		{
			InOutStyle.Common.Transition = Patch.Common.Transition;
		}
#define LUNAR_MERGE_COMBO_BOX_FIELD(FieldName) \
		if (Patch.bOverride##FieldName) \
		{ \
			InOutStyle.bOverride##FieldName = true; \
			InOutStyle.FieldName = Patch.FieldName; \
		}
		LUNAR_MERGE_COMBO_BOX_FIELD(ArrowBrush)
		LUNAR_MERGE_COMBO_BOX_FIELD(PopupBrush)
		LUNAR_MERGE_COMBO_BOX_FIELD(RowStyle)
		LUNAR_MERGE_COMBO_BOX_FIELD(ScrollBarStyle)
#undef LUNAR_MERGE_COMBO_BOX_FIELD
	}

	/**
	 * @brief Applies enabled common and ContextMenu-specific overrides to an accumulated patch.
	 * @param InOutStyle Accumulated ContextMenu style patch.
	 * @param Patch Authored layer to apply.
	 */
	void MergeContextMenuStylePatch(
		FLunarContextMenuStylePatch& InOutStyle,
		const FLunarContextMenuStylePatch& Patch)
	{
		const bool bHasSpecializedOverride = Patch.bOverridePanelBrush
			|| Patch.bOverrideSeparatorBrush
			|| Patch.bOverrideSubmenuArrowBrush
			|| Patch.bOverrideRowStyle;
		LunarStyleResolver::MergeCommonStylePatch(InOutStyle.Common, Patch.Common);
		if (bHasSpecializedOverride)
		{
			InOutStyle.Common.Transition = Patch.Common.Transition;
		}
#define LUNAR_MERGE_CONTEXT_MENU_FIELD(FieldName) \
		if (Patch.bOverride##FieldName) \
		{ \
			InOutStyle.bOverride##FieldName = true; \
			InOutStyle.FieldName = Patch.FieldName; \
		}
		LUNAR_MERGE_CONTEXT_MENU_FIELD(PanelBrush)
		LUNAR_MERGE_CONTEXT_MENU_FIELD(SeparatorBrush)
		LUNAR_MERGE_CONTEXT_MENU_FIELD(SubmenuArrowBrush)
		LUNAR_MERGE_CONTEXT_MENU_FIELD(RowStyle)
#undef LUNAR_MERGE_CONTEXT_MENU_FIELD
	}

	/**
	 * @brief Applies enabled common and Tabs-specific overrides to an accumulated patch.
	 * @param InOutStyle Accumulated Tabs style patch.
	 * @param Patch Authored layer to apply.
	 */
	void MergeTabsStylePatch(FLunarTabsStylePatch& InOutStyle, const FLunarTabsStylePatch& Patch)
	{
		const bool bHasSpecializedOverride = Patch.bOverrideHeaderStyle
			|| Patch.bOverrideActiveIndicatorBrush
			|| Patch.bOverrideActiveIndicatorTint
			|| Patch.bOverridePagePadding;
		LunarStyleResolver::MergeCommonStylePatch(InOutStyle.Common, Patch.Common);
		if (bHasSpecializedOverride)
		{
			InOutStyle.Common.Transition = Patch.Common.Transition;
		}
		if (Patch.bOverrideHeaderStyle)
		{
			InOutStyle.bOverrideHeaderStyle = true;
			// HeaderStyle is itself a typed patch. Preserve every inherited common
			// field that this layer did not explicitly override.
			LunarStyleResolver::MergeCommonStylePatch(
				InOutStyle.HeaderStyle.Common,
				Patch.HeaderStyle.Common);
		}
#define LUNAR_MERGE_TABS_FIELD(FieldName) \
		if (Patch.bOverride##FieldName) \
		{ \
			InOutStyle.bOverride##FieldName = true; \
			InOutStyle.FieldName = Patch.FieldName; \
		}
		LUNAR_MERGE_TABS_FIELD(ActiveIndicatorBrush)
		LUNAR_MERGE_TABS_FIELD(ActiveIndicatorTint)
		LUNAR_MERGE_TABS_FIELD(PagePadding)
#undef LUNAR_MERGE_TABS_FIELD
	}

	/**
	 * @brief Applies enabled common overrides to an accumulated input-prompt patch.
	 * @param InOutStyle Accumulated input-prompt style patch.
	 * @param Patch Authored layer to apply.
	 */
	void MergeInputPromptStylePatch(
		FLunarInputPromptStylePatch& InOutStyle,
		const FLunarInputPromptStylePatch& Patch)
	{
		LunarStyleResolver::MergeCommonStylePatch(InOutStyle.Common, Patch.Common);
	}

	/**
	 * @brief Validates and orders a typed style inheritance chain.
	 * @tparam StyleAssetType Expected concrete Lunar style-asset type.
	 * @param LeafStyle Optional leaf style from which to traverse parents.
	 * @param ControlLabel Human-readable control name used in errors.
	 * @param ChainName Human-readable chain label used in errors.
	 * @param OutRootToLeaf Receives compatible typed styles in merge order.
	 * @param OutError Optional destination for an actionable validation error.
	 * @return True when the chain contains no invalid, cyclic, or incompatible assets.
	 */
	template <typename StyleAssetType>
	bool BuildTypedStyleChain(
		const ULunarWidgetStyleAsset* LeafStyle,
		const TCHAR* ControlLabel,
		const TCHAR* ChainName,
		TArray<const StyleAssetType*>& OutRootToLeaf,
		FString* OutError)
	{
		TSet<const ULunarWidgetStyleAsset*> VisitedStyles;
		const ULunarWidgetStyleAsset* CurrentStyle = LeafStyle;
		while (CurrentStyle)
		{
			if (!IsValid(CurrentStyle))
			{
				ReportResolutionError(
					OutError,
					FString::Printf(
						TEXT("Invalid or unavailable style object while resolving %s. Reassign the %s style or its parent."),
						ChainName,
						ControlLabel));
				return false;
			}
			if (VisitedStyles.Contains(CurrentStyle))
			{
				ReportResolutionError(
					OutError,
					FString::Printf(
						TEXT("%s style parent cycle in %s: '%s' is referenced more than once."),
						ControlLabel,
						ChainName,
						*CurrentStyle->GetPathName()));
				return false;
			}

			VisitedStyles.Add(CurrentStyle);
			const StyleAssetType* TypedStyle = Cast<StyleAssetType>(CurrentStyle);
			if (!TypedStyle)
			{
				ReportResolutionError(
					OutError,
					FString::Printf(
						TEXT("Incompatible style in %s at '%s': expected %s, found %s."),
						ChainName,
						*CurrentStyle->GetPathName(),
						*StyleAssetType::StaticClass()->GetName(),
						*GetNameSafe(CurrentStyle->GetClass())));
				return false;
			}

			OutRootToLeaf.Add(TypedStyle);
			CurrentStyle = CurrentStyle->ParentStyle;
		}

		Algo::Reverse(OutRootToLeaf);
		return true;
	}

	/**
	 * @brief Merges base, value, and interaction layers from a typed root-to-leaf chain.
	 * @tparam StyleAssetType Concrete Lunar style-asset type.
	 * @tparam StylePatchType Matching typed style-patch type.
	 * @param RootToLeafChain Validated typed style chain.
	 * @param VisualState Current value and interaction state.
	 * @param InOutBase Accumulated base layer.
	 * @param InOutValue Accumulated value-state layer.
	 * @param InOutInteraction Accumulated interaction-state layer.
	 * @param MergePatch Function that applies one typed patch to another.
	 */
	template <typename StyleAssetType, typename StylePatchType>
	void MergeTypedStyleChainLayers(
		const TArray<const StyleAssetType*>& RootToLeafChain,
		const FLunarUIVisualState& VisualState,
		StylePatchType& InOutBase,
		StylePatchType& InOutValue,
		StylePatchType& InOutInteraction,
		void (*MergePatch)(StylePatchType&, const StylePatchType&))
	{
		for (const StyleAssetType* Style : RootToLeafChain)
		{
			MergePatch(InOutBase, Style->BaseStyle);
			if (VisualState.ValueStateTag.IsValid())
			{
				if (const StylePatchType* ValuePatch = Style->ValueStateStyles.Find(VisualState.ValueStateTag))
				{
					MergePatch(InOutValue, *ValuePatch);
				}
			}
			if (const StylePatchType* InteractionPatch = Style->InteractionStateStyles.Find(VisualState.InteractionState))
			{
				MergePatch(InOutInteraction, *InteractionPatch);
			}
		}
	}

	/**
	 * @brief Resolves a complete typed control style from global, assigned, state, and instance layers.
	 * @tparam StyleAssetType Concrete Lunar style-asset type.
	 * @tparam StylePatchType Matching typed style-patch type.
	 * @param DefaultStyleReference Optional global default style reference.
	 * @param AssignedStyle Optional instance-assigned style asset.
	 * @param ControlLabel Human-readable control name used in errors.
	 * @param VisualState Current value and interaction state.
	 * @param InstanceOverrides Per-widget common style overrides.
	 * @param OutStyle Receives the resolved typed style.
	 * @param OutError Optional destination for an actionable resolution error.
	 * @param MergePatch Function that applies one typed patch to another.
	 * @return True when every referenced style is valid and resolution succeeds.
	 */
	template <typename StyleAssetType, typename StylePatchType>
	bool ResolveTypedControlStyle(
		const TSoftObjectPtr<StyleAssetType>& DefaultStyleReference,
		const ULunarWidgetStyleAsset* AssignedStyle,
		const TCHAR* ControlLabel,
		const FLunarUIVisualState& VisualState,
		const FLunarCommonStylePatch& InstanceOverrides,
		StylePatchType& OutStyle,
		FString* OutError,
		void (*MergePatch)(StylePatchType&, const StylePatchType&))
	{
		OutStyle = StylePatchType();
		if (OutError)
		{
			OutError->Reset();
		}

		TArray<const StyleAssetType*> GlobalChain;
		if (!DefaultStyleReference.IsNull())
		{
			const StyleAssetType* DefaultStyle = DefaultStyleReference.LoadSynchronous();
			if (!DefaultStyle)
			{
				ReportResolutionError(
					OutError,
					FString::Printf(
						TEXT("Unable to load the global default %s style '%s'."),
						ControlLabel,
						*DefaultStyleReference.ToSoftObjectPath().ToString()));
				return false;
			}

			const FString GlobalChainName = FString::Printf(TEXT("global default %s style chain"), ControlLabel);
			if (!BuildTypedStyleChain(DefaultStyle, ControlLabel, *GlobalChainName, GlobalChain, OutError))
			{
				return false;
			}
		}

		TArray<const StyleAssetType*> AssignedChain;
		const FString AssignedChainName = FString::Printf(TEXT("assigned %s style chain"), ControlLabel);
		if (!BuildTypedStyleChain(AssignedStyle, ControlLabel, *AssignedChainName, AssignedChain, OutError))
		{
			return false;
		}

		StylePatchType ResolvedBase;
		StylePatchType ResolvedValue;
		StylePatchType ResolvedInteraction;
		MergeTypedStyleChainLayers(
			GlobalChain,
			VisualState,
			ResolvedBase,
			ResolvedValue,
			ResolvedInteraction,
			MergePatch);
		MergeTypedStyleChainLayers(
			AssignedChain,
			VisualState,
			ResolvedBase,
			ResolvedValue,
			ResolvedInteraction,
			MergePatch);

		MergePatch(OutStyle, ResolvedBase);
		MergePatch(OutStyle, ResolvedValue);
		MergePatch(OutStyle, ResolvedInteraction);
		LunarStyleResolver::MergeCommonStylePatch(OutStyle.Common, InstanceOverrides);
		return true;
	}
}

namespace LunarStyleResolver
{
	void MergeCommonStylePatch(FLunarCommonStylePatch& InOutStyle, const FLunarCommonStylePatch& Patch)
	{
#define LUNAR_MERGE_COMMON_FIELD(FieldName) \
		if (Patch.bOverride##FieldName) \
		{ \
			InOutStyle.bOverride##FieldName = true; \
			InOutStyle.FieldName = Patch.FieldName; \
		}

		LUNAR_MERGE_COMMON_FIELD(BackgroundBrush)
		LUNAR_MERGE_COMMON_FIELD(BorderBrush)
		LUNAR_MERGE_COMMON_FIELD(ForegroundBrush)
		LUNAR_MERGE_COMMON_FIELD(BackgroundTint)
		LUNAR_MERGE_COMMON_FIELD(ContentColor)
		LUNAR_MERGE_COMMON_FIELD(TextColor)
		LUNAR_MERGE_COMMON_FIELD(Font)
		LUNAR_MERGE_COMMON_FIELD(TextShadowColor)
		LUNAR_MERGE_COMMON_FIELD(TextShadowOffset)
		LUNAR_MERGE_COMMON_FIELD(Padding)
		LUNAR_MERGE_COMMON_FIELD(ContentPadding)
		LUNAR_MERGE_COMMON_FIELD(Opacity)
		LUNAR_MERGE_COMMON_FIELD(RenderTransform)
		LUNAR_MERGE_COMMON_FIELD(RenderTransformPivot)
		LUNAR_MERGE_COMMON_FIELD(MinDesiredSize)
		LUNAR_MERGE_COMMON_FIELD(MaxDesiredSize)

#undef LUNAR_MERGE_COMMON_FIELD

		// A default-constructed patch represents no authored layer and therefore inherits its transition.
		if (HasAuthoredTransition(Patch))
		{
			InOutStyle.Transition = Patch.Transition;
		}
	}

	FLunarCommonStylePatch InterpolateCommonStylePatch(
		const FLunarCommonStylePatch& Source,
		const FLunarCommonStylePatch& Target,
		const float Alpha)
	{
		const float ClampedAlpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
		FLunarCommonStylePatch Result = Target;

		if (Source.bOverrideBackgroundTint && Target.bOverrideBackgroundTint)
		{
			Result.BackgroundTint = FMath::Lerp(Source.BackgroundTint, Target.BackgroundTint, ClampedAlpha);
		}
		if (Source.bOverrideContentColor && Target.bOverrideContentColor)
		{
			Result.ContentColor = InterpolateSlateColor(Source.ContentColor, Target.ContentColor, ClampedAlpha);
		}
		if (Source.bOverrideTextColor && Target.bOverrideTextColor)
		{
			Result.TextColor = InterpolateSlateColor(Source.TextColor, Target.TextColor, ClampedAlpha);
		}
		if (Source.bOverrideTextShadowColor && Target.bOverrideTextShadowColor)
		{
			Result.TextShadowColor = FMath::Lerp(Source.TextShadowColor, Target.TextShadowColor, ClampedAlpha);
		}
		if (Source.bOverrideTextShadowOffset && Target.bOverrideTextShadowOffset)
		{
			Result.TextShadowOffset = FMath::Lerp(Source.TextShadowOffset, Target.TextShadowOffset, ClampedAlpha);
		}
		if (Source.bOverridePadding && Target.bOverridePadding)
		{
			Result.Padding = InterpolateMargin(Source.Padding, Target.Padding, ClampedAlpha);
		}
		if (Source.bOverrideContentPadding && Target.bOverrideContentPadding)
		{
			Result.ContentPadding = InterpolateMargin(Source.ContentPadding, Target.ContentPadding, ClampedAlpha);
		}
		if (Source.bOverrideOpacity && Target.bOverrideOpacity)
		{
			Result.Opacity = FMath::Lerp(Source.Opacity, Target.Opacity, ClampedAlpha);
		}
		if (Source.bOverrideRenderTransform && Target.bOverrideRenderTransform)
		{
			Result.RenderTransform = InterpolateWidgetTransform(Source.RenderTransform, Target.RenderTransform, ClampedAlpha);
		}
		if (Source.bOverrideRenderTransformPivot && Target.bOverrideRenderTransformPivot)
		{
			Result.RenderTransformPivot = FMath::Lerp(
				Source.RenderTransformPivot,
				Target.RenderTransformPivot,
				ClampedAlpha);
		}
		if (Source.bOverrideMinDesiredSize && Target.bOverrideMinDesiredSize)
		{
			Result.MinDesiredSize = FMath::Lerp(Source.MinDesiredSize, Target.MinDesiredSize, ClampedAlpha);
		}
		if (Source.bOverrideMaxDesiredSize && Target.bOverrideMaxDesiredSize)
		{
			Result.MaxDesiredSize = FMath::Lerp(Source.MaxDesiredSize, Target.MaxDesiredSize, ClampedAlpha);
		}

		return Result;
	}

	void ApplyCommonDiscreteFields(
		FLunarCommonStylePatch& InOutStyle,
		const FLunarCommonStylePatch& LogicalTarget)
	{
#define LUNAR_COPY_COMMON_DISCRETE_FIELD(FieldName) \
		InOutStyle.bOverride##FieldName = LogicalTarget.bOverride##FieldName; \
		InOutStyle.FieldName = LogicalTarget.FieldName;

		LUNAR_COPY_COMMON_DISCRETE_FIELD(BackgroundBrush)
		LUNAR_COPY_COMMON_DISCRETE_FIELD(BorderBrush)
		LUNAR_COPY_COMMON_DISCRETE_FIELD(ForegroundBrush)
		LUNAR_COPY_COMMON_DISCRETE_FIELD(Font)

#undef LUNAR_COPY_COMMON_DISCRETE_FIELD
		InOutStyle.Transition = LogicalTarget.Transition;
	}

	bool AreCommonStylesEquivalent(const FLunarCommonStylePatch& A, const FLunarCommonStylePatch& B)
	{
#define LUNAR_COMMON_FIELD_DIFFERS(FieldName) \
		(A.bOverride##FieldName != B.bOverride##FieldName \
			|| (A.bOverride##FieldName && !(A.FieldName == B.FieldName)))

		if (LUNAR_COMMON_FIELD_DIFFERS(BackgroundBrush)
			|| LUNAR_COMMON_FIELD_DIFFERS(BorderBrush)
			|| LUNAR_COMMON_FIELD_DIFFERS(ForegroundBrush)
			|| LUNAR_COMMON_FIELD_DIFFERS(BackgroundTint)
			|| LUNAR_COMMON_FIELD_DIFFERS(ContentColor)
			|| LUNAR_COMMON_FIELD_DIFFERS(TextColor)
			|| LUNAR_COMMON_FIELD_DIFFERS(Font)
			|| LUNAR_COMMON_FIELD_DIFFERS(TextShadowColor)
			|| LUNAR_COMMON_FIELD_DIFFERS(TextShadowOffset)
			|| LUNAR_COMMON_FIELD_DIFFERS(Padding)
			|| LUNAR_COMMON_FIELD_DIFFERS(ContentPadding)
			|| LUNAR_COMMON_FIELD_DIFFERS(Opacity)
			|| LUNAR_COMMON_FIELD_DIFFERS(RenderTransform)
			|| LUNAR_COMMON_FIELD_DIFFERS(RenderTransformPivot)
			|| LUNAR_COMMON_FIELD_DIFFERS(MinDesiredSize)
			|| LUNAR_COMMON_FIELD_DIFFERS(MaxDesiredSize))
		{
			return false;
		}

#undef LUNAR_COMMON_FIELD_DIFFERS

		return AreTransitionsEquivalent(A.Transition, B.Transition);
	}

	bool ResolveButtonCommonStyle(
		const ULunarWidgetStyleAsset* AssignedStyle,
		const FLunarUIVisualState& VisualState,
		const FLunarCommonStylePatch& InstanceOverrides,
		FLunarCommonStylePatch& OutStyle,
		FString* OutError)
	{
		OutStyle = FLunarCommonStylePatch();
		if (OutError)
		{
			OutError->Reset();
		}

		TArray<const ULunarButtonStyleAsset*> GlobalChain;
		if (const ULunarSettings* Settings = GetDefault<ULunarSettings>())
		{
			const TSoftObjectPtr<ULunarButtonStyleAsset>& DefaultStyleReference =
				Settings->Navigation.DefaultStyles.DefaultButtonStyle;

			if (!DefaultStyleReference.IsNull())
			{
				const ULunarButtonStyleAsset* DefaultStyle = DefaultStyleReference.LoadSynchronous();
				if (!DefaultStyle)
				{
					ReportResolutionError(
						OutError,
						FString::Printf(
							TEXT("Unable to load the global default Button style '%s'."),
							*DefaultStyleReference.ToSoftObjectPath().ToString()));
					return false;
				}

				if (!BuildButtonStyleChain(DefaultStyle, TEXT("global default Button style chain"), GlobalChain, OutError))
				{
					return false;
				}
			}
		}

		TArray<const ULunarButtonStyleAsset*> AssignedChain;
		if (!BuildButtonStyleChain(AssignedStyle, TEXT("assigned Button style chain"), AssignedChain, OutError))
		{
			return false;
		}

		FLunarCommonStylePatch ResolvedBase;
		FLunarCommonStylePatch ResolvedValue;
		FLunarCommonStylePatch ResolvedInteraction;

		MergeButtonChainLayers(GlobalChain, VisualState, ResolvedBase, ResolvedValue, ResolvedInteraction);
		MergeButtonChainLayers(AssignedChain, VisualState, ResolvedBase, ResolvedValue, ResolvedInteraction);

		MergeCommonStylePatch(OutStyle, ResolvedBase);
		MergeCommonStylePatch(OutStyle, ResolvedValue);
		MergeCommonStylePatch(OutStyle, ResolvedInteraction);
		MergeCommonStylePatch(OutStyle, InstanceOverrides);
		return true;
	}

	bool ResolveSliderStyle(
		const ULunarWidgetStyleAsset* AssignedStyle,
		const FLunarUIVisualState& VisualState,
		const FLunarCommonStylePatch& InstanceOverrides,
		FLunarSliderStylePatch& OutStyle,
		FString* OutError)
	{
		TSoftObjectPtr<ULunarSliderStyleAsset> DefaultStyleReference;
		if (const ULunarSettings* Settings = GetDefault<ULunarSettings>())
		{
			DefaultStyleReference = Settings->Navigation.DefaultStyles.DefaultSliderStyle;
		}
		return ResolveTypedControlStyle(
			DefaultStyleReference,
			AssignedStyle,
			TEXT("Slider"),
			VisualState,
			InstanceOverrides,
			OutStyle,
			OutError,
			&MergeSliderStylePatch);
	}

	bool ResolveOptionSliderStyle(
		const ULunarWidgetStyleAsset* AssignedStyle,
		const FLunarUIVisualState& VisualState,
		const FLunarCommonStylePatch& InstanceOverrides,
		FLunarOptionSliderStylePatch& OutStyle,
		FString* OutError)
	{
		TSoftObjectPtr<ULunarOptionSliderStyleAsset> DefaultStyleReference;
		if (const ULunarSettings* Settings = GetDefault<ULunarSettings>())
		{
			DefaultStyleReference = Settings->Navigation.DefaultStyles.DefaultOptionSliderStyle;
		}
		return ResolveTypedControlStyle(
			DefaultStyleReference,
			AssignedStyle,
			TEXT("OptionSlider"),
			VisualState,
			InstanceOverrides,
			OutStyle,
			OutError,
			&MergeOptionSliderStylePatch);
	}

	bool ResolveSwitchStyle(
		const ULunarWidgetStyleAsset* AssignedStyle,
		const FLunarUIVisualState& VisualState,
		const FLunarCommonStylePatch& InstanceOverrides,
		FLunarSwitchStylePatch& OutStyle,
		FString* OutError)
	{
		TSoftObjectPtr<ULunarSwitchStyleAsset> DefaultStyleReference;
		if (const ULunarSettings* Settings = GetDefault<ULunarSettings>())
		{
			DefaultStyleReference = Settings->Navigation.DefaultStyles.DefaultSwitchStyle;
		}
		return ResolveTypedControlStyle(
			DefaultStyleReference,
			AssignedStyle,
			TEXT("Switch"),
			VisualState,
			InstanceOverrides,
			OutStyle,
			OutError,
			&MergeSwitchStylePatch);
	}

	bool ResolveRadioStyle(
		const ULunarWidgetStyleAsset* AssignedStyle,
		const FLunarUIVisualState& VisualState,
		const FLunarCommonStylePatch& InstanceOverrides,
		FLunarRadioStylePatch& OutStyle,
		FString* OutError)
	{
		TSoftObjectPtr<ULunarRadioStyleAsset> DefaultStyleReference;
		if (const ULunarSettings* Settings = GetDefault<ULunarSettings>())
		{
			DefaultStyleReference = Settings->Navigation.DefaultStyles.DefaultRadioStyle;
		}
		return ResolveTypedControlStyle(
			DefaultStyleReference,
			AssignedStyle,
			TEXT("Radio"),
			VisualState,
			InstanceOverrides,
			OutStyle,
			OutError,
			&MergeRadioStylePatch);
	}

	bool ResolveListViewStyle(
		const ULunarWidgetStyleAsset* AssignedStyle,
		const FLunarUIVisualState& VisualState,
		const FLunarCommonStylePatch& InstanceOverrides,
		FLunarListViewStylePatch& OutStyle,
		FString* OutError)
	{
		TSoftObjectPtr<ULunarListViewStyleAsset> DefaultStyleReference;
		if (const ULunarSettings* Settings = GetDefault<ULunarSettings>())
		{
			DefaultStyleReference = Settings->Navigation.DefaultStyles.DefaultListViewStyle;
		}
		return ResolveTypedControlStyle(
			DefaultStyleReference, AssignedStyle, TEXT("ListView"), VisualState, InstanceOverrides,
			OutStyle, OutError, &MergeListViewStylePatch);
	}

	bool ResolveComboBoxStyle(
		const ULunarWidgetStyleAsset* AssignedStyle,
		const FLunarUIVisualState& VisualState,
		const FLunarCommonStylePatch& InstanceOverrides,
		FLunarComboBoxStylePatch& OutStyle,
		FString* OutError)
	{
		TSoftObjectPtr<ULunarComboBoxStyleAsset> DefaultStyleReference;
		if (const ULunarSettings* Settings = GetDefault<ULunarSettings>())
		{
			DefaultStyleReference = Settings->Navigation.DefaultStyles.DefaultComboBoxStyle;
		}
		return ResolveTypedControlStyle(
			DefaultStyleReference, AssignedStyle, TEXT("ComboBox"), VisualState, InstanceOverrides,
			OutStyle, OutError, &MergeComboBoxStylePatch);
	}

	bool ResolveContextMenuStyle(
		const ULunarWidgetStyleAsset* AssignedStyle,
		const FLunarUIVisualState& VisualState,
		const FLunarCommonStylePatch& InstanceOverrides,
		FLunarContextMenuStylePatch& OutStyle,
		FString* OutError)
	{
		TSoftObjectPtr<ULunarContextMenuStyleAsset> DefaultStyleReference;
		if (const ULunarSettings* Settings = GetDefault<ULunarSettings>())
		{
			DefaultStyleReference = Settings->Navigation.DefaultStyles.DefaultContextMenuStyle;
		}
		return ResolveTypedControlStyle(
			DefaultStyleReference, AssignedStyle, TEXT("ContextMenu"), VisualState, InstanceOverrides,
			OutStyle, OutError, &MergeContextMenuStylePatch);
	}

	bool ResolveTabsStyle(
		const ULunarWidgetStyleAsset* AssignedStyle,
		const FLunarUIVisualState& VisualState,
		const FLunarCommonStylePatch& InstanceOverrides,
		FLunarTabsStylePatch& OutStyle,
		FString* OutError)
	{
		TSoftObjectPtr<ULunarTabsStyleAsset> DefaultStyleReference;
		if (const ULunarSettings* Settings = GetDefault<ULunarSettings>())
		{
			DefaultStyleReference = Settings->Navigation.DefaultStyles.DefaultTabsStyle;
		}
		return ResolveTypedControlStyle(
			DefaultStyleReference, AssignedStyle, TEXT("Tabs"), VisualState, InstanceOverrides,
			OutStyle, OutError, &MergeTabsStylePatch);
	}

	bool ResolveInputPromptStyle(
		const ULunarWidgetStyleAsset* AssignedStyle,
		const FLunarUIVisualState& VisualState,
		const FLunarCommonStylePatch& InstanceOverrides,
		FLunarInputPromptStylePatch& OutStyle,
		FString* OutError)
	{
		TSoftObjectPtr<ULunarInputPromptStyleAsset> DefaultStyleReference;
		if (const ULunarSettings* Settings = GetDefault<ULunarSettings>())
		{
			DefaultStyleReference = Settings->Navigation.DefaultStyles.DefaultInputPromptStyle;
		}
		return ResolveTypedControlStyle(
			DefaultStyleReference, AssignedStyle, TEXT("InputPrompt"), VisualState, InstanceOverrides,
			OutStyle, OutError, &MergeInputPromptStylePatch);
	}

	bool ResolveScrollBoxStyle(
		const ULunarScrollBoxStyleAsset* AssignedStyle,
		const FLunarUIVisualState& VisualState,
		FLunarScrollBoxStylePatch& OutStyle,
		FString* OutError)
	{
		OutStyle = FLunarScrollBoxStylePatch();
		if (OutError)
		{
			OutError->Reset();
		}

		TArray<const ULunarScrollBoxStyleAsset*> GlobalChain;
		if (const ULunarSettings* Settings = GetDefault<ULunarSettings>())
		{
			const TSoftObjectPtr<ULunarScrollBoxStyleAsset>& DefaultStyleReference =
				Settings->Navigation.DefaultStyles.DefaultScrollBoxStyle;

			if (!DefaultStyleReference.IsNull())
			{
				const ULunarScrollBoxStyleAsset* DefaultStyle = DefaultStyleReference.LoadSynchronous();
				if (!DefaultStyle)
				{
					ReportResolutionError(
						OutError,
						FString::Printf(
							TEXT("Unable to load the global default ScrollBox style '%s'. Verify the asset path in Lunar Navigation settings."),
							*DefaultStyleReference.ToSoftObjectPath().ToString()));
					return false;
				}

				if (!BuildScrollBoxStyleChain(
					DefaultStyle,
					TEXT("global default ScrollBox style chain"),
					GlobalChain,
					OutError))
				{
					return false;
				}
			}
		}

		TArray<const ULunarScrollBoxStyleAsset*> AssignedChain;
		if (!BuildScrollBoxStyleChain(
			AssignedStyle,
			TEXT("assigned ScrollBox style chain"),
			AssignedChain,
			OutError))
		{
			return false;
		}

		FLunarScrollBoxStylePatch ResolvedBase;
		FLunarScrollBoxStylePatch ResolvedValue;
		FLunarScrollBoxStylePatch ResolvedInteraction;

		MergeScrollBoxChainLayers(GlobalChain, VisualState, ResolvedBase, ResolvedValue, ResolvedInteraction);
		MergeScrollBoxChainLayers(AssignedChain, VisualState, ResolvedBase, ResolvedValue, ResolvedInteraction);

		MergeScrollBoxStylePatch(OutStyle, ResolvedBase);
		MergeScrollBoxStylePatch(OutStyle, ResolvedValue);
		MergeScrollBoxStylePatch(OutStyle, ResolvedInteraction);
		return true;
	}
}
