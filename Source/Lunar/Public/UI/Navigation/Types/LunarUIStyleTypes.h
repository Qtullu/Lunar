// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"
#include "Kismet/KismetMathLibrary.h"
#include "Slate/WidgetTransform.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateColor.h"
#include "Styling/SlateTypes.h"
#include "UI/Navigation/Types/LunarNavigationTypes.h"
#include "LunarUIStyleTypes.generated.h"

/**
 * @file LunarUIStyleTypes.h
 * @brief Shared typed style patches and transition settings for Lunar UI controls.
 * @ingroup LunarNavigationTypes
 */

/** @brief Defines the transition used when a resolved Lunar visual state becomes active. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarStyleTransitionSettings
{
	GENERATED_BODY()

	/** Enables interpolation instead of applying the destination immediately. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style")
	bool bEnabled = false;

	/** Transition duration in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (ClampMin = "0.0"))
	float Duration = 0.0f;

	/** Easing function used by the transition. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style")
	TEnumAsByte<EEasingFunc::Type> Easing = EEasingFunc::Linear;
};

/** @brief Strongly typed common visual patch shared by all Lunar controls. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarCommonStylePatch
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideBackgroundBrush = false; ///< Enables BackgroundBrush for this patch.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideBackgroundBrush")) FSlateBrush BackgroundBrush; ///< Background image brush.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideBorderBrush = false; ///< Enables BorderBrush for this patch.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideBorderBrush")) FSlateBrush BorderBrush; ///< Border image brush.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideForegroundBrush = false; ///< Enables ForegroundBrush for this patch.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideForegroundBrush")) FSlateBrush ForegroundBrush; ///< Foreground overlay brush.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideBackgroundTint = false; ///< Enables BackgroundTint for this patch.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideBackgroundTint")) FLinearColor BackgroundTint = FLinearColor::White; ///< Tint applied to the background brush.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideContentColor = false; ///< Enables ContentColor for this patch.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideContentColor")) FSlateColor ContentColor = FSlateColor(FLinearColor::White); ///< Color multiplier inherited by arbitrary widget content.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideTextColor = false; ///< Enables TextColor for this patch.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideTextColor")) FSlateColor TextColor = FSlateColor(FLinearColor::White); ///< Foreground color applied to descendant text blocks.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideFont = false; ///< Enables Font for this patch.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideFont")) FSlateFontInfo Font; ///< Font applied to descendant text blocks.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideTextShadowColor = false; ///< Enables TextShadowColor for this patch.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideTextShadowColor")) FLinearColor TextShadowColor = FLinearColor::Transparent; ///< Shadow color applied to descendant text blocks.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideTextShadowOffset = false; ///< Enables TextShadowOffset for this patch.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideTextShadowOffset")) FVector2D TextShadowOffset = FVector2D::ZeroVector; ///< Shadow offset applied to descendant text blocks.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverridePadding = false; ///< Enables Padding for this patch.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverridePadding")) FMargin Padding; ///< Outer style padding.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideContentPadding = false; ///< Enables ContentPadding for this patch.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideContentPadding")) FMargin ContentPadding; ///< Padding between the border and arbitrary content.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideOpacity = false; ///< Enables Opacity for this patch.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideOpacity", ClampMin = "0.0", ClampMax = "1.0")) float Opacity = 1.0f; ///< Render opacity applied to the widget.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideRenderTransform = false; ///< Enables RenderTransform for this patch.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideRenderTransform")) FWidgetTransform RenderTransform; ///< Render-space transform applied to the widget.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideRenderTransformPivot = false; ///< Enables RenderTransformPivot for this patch.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideRenderTransformPivot")) FVector2D RenderTransformPivot = FVector2D(0.5f, 0.5f); ///< Normalized pivot used by the render transform.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideMinDesiredSize = false; ///< Enables MinDesiredSize for this patch.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideMinDesiredSize")) FVector2D MinDesiredSize = FVector2D::ZeroVector; ///< Optional minimum desired width and height.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideMaxDesiredSize = false; ///< Enables MaxDesiredSize for this patch.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideMaxDesiredSize")) FVector2D MaxDesiredSize = FVector2D::ZeroVector; ///< Optional maximum desired width and height.

	/** Destination transition settings for this patch. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style")
	FLunarStyleTransitionSettings Transition;
};

/** @brief Typed Button style patch. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarButtonStylePatch
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") FLunarCommonStylePatch Common; ///< Common visual fields for the Button.
};

/** @brief Typed Slider style patch. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarSliderStylePatch
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") FLunarCommonStylePatch Common; ///< Common visual fields for the Slider.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideTrackBrush = false; ///< Enables TrackBrush.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideTrackBrush")) FSlateBrush TrackBrush; ///< Slider track brush.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideFillBrush = false; ///< Enables FillBrush.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideFillBrush")) FSlateBrush FillBrush; ///< Filled-range brush.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideThumbBrush = false; ///< Enables ThumbBrush.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideThumbBrush")) FSlateBrush ThumbBrush; ///< Slider thumb brush.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideTrackTint = false; ///< Enables TrackTint.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideTrackTint")) FLinearColor TrackTint = FLinearColor::White; ///< Track color multiplier.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideFillTint = false; ///< Enables FillTint.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideFillTint")) FLinearColor FillTint = FLinearColor::White; ///< Fill color multiplier.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideThumbTint = false; ///< Enables ThumbTint.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideThumbTint")) FLinearColor ThumbTint = FLinearColor::White; ///< Thumb color multiplier.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideTrackThickness = false; ///< Enables TrackThickness.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideTrackThickness", ClampMin = "0.0")) float TrackThickness = 0.0f; ///< Track thickness in Slate units.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideThumbSize = false; ///< Enables ThumbSize.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideThumbSize")) FVector2D ThumbSize = FVector2D::ZeroVector; ///< Desired thumb size.
};

/** @brief Typed OptionSlider style patch. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarOptionSliderStylePatch
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") FLunarCommonStylePatch Common; ///< Common visual fields for the OptionSlider.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverridePreviousArrowBrush = false; ///< Enables PreviousArrowBrush.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverridePreviousArrowBrush")) FSlateBrush PreviousArrowBrush; ///< Brush for the previous-option arrow.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideNextArrowBrush = false; ///< Enables NextArrowBrush.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideNextArrowBrush")) FSlateBrush NextArrowBrush; ///< Brush for the next-option arrow.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverridePreviousArrowTint = false; ///< Enables PreviousArrowTint.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverridePreviousArrowTint")) FLinearColor PreviousArrowTint = FLinearColor::White; ///< Color multiplier for the previous-option arrow.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideNextArrowTint = false; ///< Enables NextArrowTint.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideNextArrowTint")) FLinearColor NextArrowTint = FLinearColor::White; ///< Color multiplier for the next-option arrow.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideValueTextColor = false; ///< Enables ValueTextColor.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideValueTextColor")) FSlateColor ValueTextColor = FSlateColor(FLinearColor::White); ///< Color of the selected option text.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideValueFont = false; ///< Enables ValueFont.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideValueFont")) FSlateFontInfo ValueFont; ///< Font of the selected option text.
};

/** @brief Typed Switch style patch. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarSwitchStylePatch
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") FLunarCommonStylePatch Common; ///< Common visual fields for the Switch.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideTrackBrush = false; ///< Enables TrackBrush.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideTrackBrush")) FSlateBrush TrackBrush; ///< Switch track brush.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideHandleBrush = false; ///< Enables HandleBrush.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideHandleBrush")) FSlateBrush HandleBrush; ///< Switch handle brush.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideTrackTint = false; ///< Enables TrackTint.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideTrackTint")) FLinearColor TrackTint = FLinearColor::White; ///< Color multiplier for the switch track.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideHandleTint = false; ///< Enables HandleTint.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideHandleTint")) FLinearColor HandleTint = FLinearColor::White; ///< Color multiplier for the switch handle.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideTrackSize = false; ///< Enables TrackSize.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideTrackSize")) FVector2D TrackSize = FVector2D::ZeroVector; ///< Desired switch track size.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideHandleSize = false; ///< Enables HandleSize.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideHandleSize")) FVector2D HandleSize = FVector2D::ZeroVector; ///< Desired switch handle size.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideHandleOffset = false; ///< Enables HandleOffset.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideHandleOffset")) FVector2D HandleOffset = FVector2D::ZeroVector; ///< Positional offset applied to the switch handle.
};

/** @brief Typed Radio style patch. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarRadioStylePatch
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") FLunarCommonStylePatch Common; ///< Common visual fields for the Radio control.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideMarkBrush = false; ///< Enables MarkBrush.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideMarkBrush")) FSlateBrush MarkBrush; ///< Brush used for the checked mark.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideMarkTint = false; ///< Enables MarkTint.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideMarkTint")) FLinearColor MarkTint = FLinearColor::White; ///< Color multiplier for the checked mark.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideMarkSize = false; ///< Enables MarkSize.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideMarkSize")) FVector2D MarkSize = FVector2D::ZeroVector; ///< Desired size of the checked mark.
};

/** @brief Typed ScrollBox style patch. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarScrollBoxStylePatch
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") FLunarCommonStylePatch Common; ///< Common visual fields for the ScrollBox.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideScrollBarStyle = false; ///< Enables ScrollBarStyle.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideScrollBarStyle")) FScrollBarStyle ScrollBarStyle; ///< Complete Slate scrollbar style.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideScrollBarPadding = false; ///< Enables ScrollBarPadding.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideScrollBarPadding")) FMargin ScrollBarPadding; ///< Padding around the scrollbar.
};

/** @brief Typed ListView style patch. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarListViewStylePatch
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") FLunarCommonStylePatch Common; ///< Common visual fields for the ListView.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideRowStyle = false; ///< Enables RowStyle.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideRowStyle")) FTableRowStyle RowStyle; ///< Complete Slate table-row style.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideScrollBarStyle = false; ///< Enables ScrollBarStyle.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideScrollBarStyle")) FScrollBarStyle ScrollBarStyle; ///< Complete Slate scrollbar style.
};

/** @brief Typed ComboBox style patch. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarComboBoxStylePatch
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") FLunarCommonStylePatch Common; ///< Common visual fields for the ComboBox.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideArrowBrush = false; ///< Enables ArrowBrush.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideArrowBrush")) FSlateBrush ArrowBrush; ///< Brush for the combo-box dropdown arrow.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverridePopupBrush = false; ///< Enables PopupBrush.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverridePopupBrush")) FSlateBrush PopupBrush; ///< Background brush of the options popup.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideRowStyle = false; ///< Enables RowStyle.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideRowStyle")) FTableRowStyle RowStyle; ///< Complete Slate style for option rows.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideScrollBarStyle = false; ///< Enables ScrollBarStyle.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideScrollBarStyle")) FScrollBarStyle ScrollBarStyle; ///< Complete Slate scrollbar style for the popup list.
};

/** @brief Typed ContextMenu style patch. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarContextMenuStylePatch
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") FLunarCommonStylePatch Common; ///< Common visual fields for the ContextMenu.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverridePanelBrush = false; ///< Enables PanelBrush.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverridePanelBrush")) FSlateBrush PanelBrush; ///< Background brush of the menu panel.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideSeparatorBrush = false; ///< Enables SeparatorBrush.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideSeparatorBrush")) FSlateBrush SeparatorBrush; ///< Brush used for separator rows.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideSubmenuArrowBrush = false; ///< Enables SubmenuArrowBrush.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideSubmenuArrowBrush")) FSlateBrush SubmenuArrowBrush; ///< Brush indicating that a row owns a submenu.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideRowStyle = false; ///< Enables RowStyle.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideRowStyle")) FTableRowStyle RowStyle; ///< Complete Slate style for menu rows.
};

/** @brief Typed Tabs style patch. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarTabsStylePatch
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") FLunarCommonStylePatch Common; ///< Common visual fields for the Tabs container.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideHeaderStyle = false; ///< Enables HeaderStyle.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideHeaderStyle")) FLunarButtonStylePatch HeaderStyle; ///< Button-style patch applied to tab headers.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideActiveIndicatorBrush = false; ///< Enables ActiveIndicatorBrush.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideActiveIndicatorBrush")) FSlateBrush ActiveIndicatorBrush; ///< Brush used to mark the active tab.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverrideActiveIndicatorTint = false; ///< Enables ActiveIndicatorTint.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverrideActiveIndicatorTint")) FLinearColor ActiveIndicatorTint = FLinearColor::White; ///< Color multiplier for the active-tab indicator.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") bool bOverridePagePadding = false; ///< Enables PagePadding.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style", meta = (EditCondition = "bOverridePagePadding")) FMargin PagePadding; ///< Padding around the active tab page.
};

/** @brief Typed input-prompt style patch. @ingroup LunarNavigationTypes */
USTRUCT(BlueprintType)
struct LUNAR_API FLunarInputPromptStylePatch
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style") FLunarCommonStylePatch Common; ///< Common visual fields for an input prompt.
};
