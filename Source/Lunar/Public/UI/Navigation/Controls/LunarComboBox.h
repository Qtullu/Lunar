// Copyright 2026 Edgar Frolenkov All rights reserved.

/** @file LunarComboBox.h @brief Self-contained data-backed Lunar ComboBox. @ingroup LunarNavigationControls */

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateTypes.h"
#include "UI/Navigation/Core/LunarNavigableWidget.h"
#include "LunarComboBox.generated.h"

class FLunarComboBoxOutsideClickInputProcessor;
class SBorder;
class SBox;
class SImage;
class SMenuAnchor;
class STextBlock;
class SWidgetSwitcher;
class UCurveFloat;
class ULunarComboBoxEmptyVisual;
class ULunarComboBoxEntry;
class ULunarComboBoxSelectedVisual;
class ULunarListView;
class ULunarNavigationScope;
class ULunarNavigationSubsystem;

/** Placement of the generated popup relative to the closed ComboBox face. */
UENUM(BlueprintType)
enum class ELunarComboBoxPopupPlacement : uint8
{
	/** Let Slate choose above or below while preserving left alignment. */
	Auto,
	/** Place the popup below and align its left edge. */
	BelowLeft,
	/** Place the popup below and center it on the closed face. */
	BelowCenter,
	/** Place the popup below and align its right edge. */
	BelowRight,
	/** Place the popup above and align its left edge. */
	AboveLeft,
	/** Place the popup above and center it on the closed face. */
	AboveCenter,
	/** Place the popup above and align its right edge. */
	AboveRight
};

/** Policy used when the committed option disappears or becomes disabled. */
UENUM(BlueprintType)
enum class ELunarComboBoxSelectionRecoveryMode : uint8
{
	/** Recover from the nearest eligible option around the previous logical index. */
	Auto,
	/** Recover from the nearest eligible option around ForcedSelectionIndex. */
	ForceIndex
};

/** Broadcast after the external filter text changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FLunarComboBoxFilterTextChangedSignature,
	ULunarComboBox*, ComboBox,
	FText, PreviousFilterText,
	FText, NewFilterText);

/**
 * Self-contained ComboBox that owns its closed presentation, popup, virtualized list, and nested scope.
 * Owner Blueprints only configure option data, optional visual classes, and inline fallback styles.
 */
UCLASS(Blueprintable)
class LUNAR_API ULunarComboBox : public ULunarNavigableWidget
{
	GENERATED_BODY()

public:
	/** Creates a self-contained ComboBox with null custom visual classes and usable inline defaults. */
	ULunarComboBox(const FObjectInitializer& ObjectInitializer);

	/** Replaces the source option array and normalizes committed and temporary selection. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ComboBox", meta = (AdvancedDisplay = "NotificationPolicy"))
	void SetOptions(const TArray<FLunarComboBoxOption>& NewOptions, ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);

	/** Commits an option by stable ID, or clears it only when empty selection is allowed. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ComboBox", meta = (AdvancedDisplay = "NotificationPolicy"))
	void SetSelectedOptionById(FName OptionId, ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);

	/** Returns the stable ID of the committed option, or None while empty. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ComboBox")
	FName GetSelectedOptionId() const { return SelectedOptionId; }

	/** Returns the source-array index of the committed option, or INDEX_NONE. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ComboBox")
	int32 GetSelectedOptionIndex() const;

	/** Copies the committed option and returns whether a valid option exists. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ComboBox")
	bool GetSelectedOption(FLunarComboBoxOption& OutOption) const;

	/** Opens the generated popup, restores its temporary cursor, and pushes its nested scope. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ComboBox")
	bool OpenComboBox();

	/** Closes the popup without committing its temporary cursor. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ComboBox")
	bool CloseComboBox();

	/** Returns whether the popup is logically open. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ComboBox")
	bool IsComboBoxOpen() const { return bComboBoxOpen; }

	/** Applies externally supplied filter text; the ComboBox never creates a search field itself. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ComboBox|Filter")
	void SetFilterText(const FText& NewFilterText);

	/** Returns the currently applied external filter text. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ComboBox|Filter")
	FText GetFilterText() const { return FilterText; }

	/** Clears external filter text and rebuilds visible options. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ComboBox|Filter")
	void ClearFilter();

	/** Re-evaluates the current external filter against all source options. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ComboBox|Filter")
	void RefreshFilter();

	/** Returns whether one option matches the external query; default matching is case-insensitive over text and ID. */
	UFUNCTION(BlueprintNativeEvent, Category = "Lunar|UI|ComboBox|Filter")
	bool DoesOptionMatchFilter(const FLunarComboBoxOption& Option, const FText& Query) const;
	virtual bool DoesOptionMatchFilter_Implementation(const FLunarComboBoxOption& Option, const FText& Query) const;

	/** Deprecated compatibility alias for SetFilterText. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ComboBox|Filter", meta = (DeprecatedFunction, DeprecationMessage = "Use SetFilterText."))
	void SetSearchQuery(const FText& NewSearchQuery) { SetFilterText(NewSearchQuery); }

	/** Deprecated compatibility hook forwarded to DoesOptionMatchFilter. */
	UFUNCTION(BlueprintNativeEvent, Category = "Lunar|UI|ComboBox|Filter", meta = (DeprecatedFunction, DeprecationMessage = "Override DoesOptionMatchFilter instead."))
	bool DoesOptionMatchSearch(const FLunarComboBoxOption& Option, const FText& Query) const;
	virtual bool DoesOptionMatchSearch_Implementation(const FLunarComboBoxOption& Option, const FText& Query) const;

	/** Replaces the native fallback arrow brush and reapplies presentation immediately. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ComboBox|Presentation")
	void SetArrowBrush(const FSlateBrush& NewBrush);
	/** Returns the native fallback arrow brush. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ComboBox|Presentation")
	FSlateBrush GetArrowBrush() const { return ArrowBrush; }
	/** Replaces the native fallback arrow tint and reapplies presentation immediately. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ComboBox|Presentation")
	void SetArrowTint(FLinearColor NewTint);
	/** Returns the native fallback arrow tint. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ComboBox|Presentation")
	FLinearColor GetArrowTint() const { return ArrowTint; }
	/** Replaces popup row and scrollbar styles used by the internal ListView. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ComboBox|Presentation")
	void ConfigurePopupListPresentation(const FTableRowStyle& NewRowStyle, const FScrollBarStyle& NewScrollBarStyle);
	/** Copies popup row and scrollbar styles used by the internal ListView. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ComboBox|Presentation")
	void GetPopupListPresentation(FTableRowStyle& OutRowStyle, FScrollBarStyle& OutScrollBarStyle) const;
	/** Replaces every native subpart style that is commonly authored together. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|ComboBox|Presentation")
	void ConfigureComboBoxPresentation(const FSlateBrush& NewArrowBrush, FLinearColor NewArrowTint, const FTableRowStyle& NewRowStyle, const FScrollBarStyle& NewScrollBarStyle);
	/** Copies every native subpart style that is commonly authored together. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|ComboBox|Presentation")
	void GetComboBoxPresentation(FSlateBrush& OutArrowBrush, FLinearColor& OutArrowTint, FTableRowStyle& OutRowStyle, FScrollBarStyle& OutScrollBarStyle) const;

	/** Source option data; every non-empty OptionId must be unique. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox", meta = (DesignerRebuild))
	TArray<FLunarComboBoxOption> Options;

	/** Stable ID of the committed value, or None only when empty selection is allowed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox")
	FName SelectedOptionId = NAME_None;

	/** Allows explicit empty selection and PlaceholderText presentation. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Selection")
	bool bAllowEmptySelection = false;

	/** Determines where selection recovers after the committed option disappears or becomes disabled. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Selection")
	ELunarComboBoxSelectionRecoveryMode SelectionRecoveryMode = ELunarComboBoxSelectionRecoveryMode::Auto;

	/** Preferred source-array index used by ForceIndex recovery. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Selection", meta = (ClampMin = "0", UIMin = "0", EditCondition = "SelectionRecoveryMode == ELunarComboBoxSelectionRecoveryMode::ForceIndex", EditConditionHides))
	int32 ForcedSelectionIndex = 0;

	/** Closed-face text shown while empty selection is allowed and active. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Selection")
	FText PlaceholderText;

	/** Optional complete Blueprint class for the closed ComboBox face; null uses inline fallback style. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Visual Classes", meta = (DesignerRebuild, AllowAbstract = "false", BlueprintBaseOnly = "true"))
	TSubclassOf<ULunarComboBoxSelectedVisual> SelectedVisualClass;

	/** Optional Blueprint row class for popup options; null uses the native text-row fallback. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Visual Classes", meta = (DesignerRebuild, AllowAbstract = "false", BlueprintBaseOnly = "true"))
	TSubclassOf<ULunarComboBoxEntry> EntryVisualClass;

	/** Optional Blueprint class for the no-filter-results view; null uses inline fallback text. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Visual Classes", meta = (DesignerRebuild, AllowAbstract = "false", BlueprintBaseOnly = "true"))
	TSubclassOf<ULunarComboBoxEmptyVisual> EmptyResultsVisualClass;

	/** Placement of the generated popup relative to the closed face. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Popup", meta = (DesignerRebuild))
	ELunarComboBoxPopupPlacement PopupPlacement = ELunarComboBoxPopupPlacement::Auto;

	/** Axis used by the internal option ListView and its local directional navigation. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Popup", meta = (DesignerRebuild))
	TEnumAsByte<EOrientation> PopupOrientation = Orient_Vertical;

	/** Wraps the internal temporary cursor between opposite list boundaries. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Popup")
	bool bWrapNavigation = false;

	/** Exact popup width when positive; zero follows the closed face subject to maximum width. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Popup", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float PopupWidthOverride = 0.0f;

	/** Maximum height of popup content before internal scrolling takes over. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Popup", meta = (ClampMin = "1.0", UIMin = "1.0"))
	float MaximumPopupHeight = 320.0f;

	/** Maximum width of popup content before internal scrolling takes over. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Popup", meta = (ClampMin = "1.0", UIMin = "1.0"))
	float MaximumPopupWidth = 640.0f;

	/** Settings for the nested navigation scope owned by the open popup. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Popup")
	FLunarNavigationScopeSettings PopupScopeSettings;

	/** Keeps external filter text across popup closes; disabled clears it on close. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Filter")
	bool bPreserveFilterText = false;

	/** Read-only external filter text currently applied to the source options. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "Lunar|UI|ComboBox|Filter")
	FText FilterText;

	/** Native fallback message shown when the filter exposes no options. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Filter")
	FText EmptyResultsText;

	/** Enables open and close opacity/translation interpolation. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Interpolation", meta = (DisplayName = "Interpolate Popup"))
	bool bInterpolatePopup = false;

	/** Normalized popup animation speed; zero snaps to the target. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Interpolation", meta = (ClampMin = "0.0", UIMin = "0.0", EditCondition = "bInterpolatePopup"))
	float PopupInterpolationSpeed = 12.0f;

	/** Optional normalized animation curve; null uses linear progress. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Interpolation", meta = (EditCondition = "bInterpolatePopup"))
	TObjectPtr<UCurveFloat> PopupInterpolationCurve = nullptr;

	/** Translation magnitude applied opposite the resolved opening direction. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Interpolation", meta = (EditCondition = "bInterpolatePopup"))
	FVector2D PopupAnimationOffset = FVector2D(0.0f, 8.0f);

	/** Native fallback style for the closed interactive surface. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Default Style")
	FButtonStyle ClosedButtonStyle;
	/** Native fallback text style for the committed value or placeholder. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Default Style")
	FTextBlockStyle ClosedTextStyle;
	/** Native fallback arrow brush. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Default Style")
	FSlateBrush ArrowBrush;
	/** Tint applied to the native fallback arrow. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Default Style")
	FLinearColor ArrowTint = FLinearColor::White;
	/** Native popup background brush. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Default Style")
	FSlateBrush PopupBackgroundBrush;
	/** Padding between popup background and generated list or empty visual. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Default Style")
	FMargin PopupPadding = FMargin(4.0f);
	/** Native fallback row style used by the internal ListView. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Default Style")
	FTableRowStyle PopupRowStyle;
	/** Scrollbar style used by the internal ListView. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Default Style")
	FScrollBarStyle PopupScrollBarStyle;
	/** Native fallback option text style. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Default Style")
	FTextBlockStyle EntryTextStyle;
	/** Native fallback option row padding. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Default Style")
	FMargin EntryPadding = FMargin(8.0f, 4.0f);
	/** Native fallback text style for the empty-results message. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Default Style")
	FTextBlockStyle EmptyResultsTextStyle;
	/** Native fallback padding for the empty-results message. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|ComboBox|Default Style")
	FMargin EmptyResultsPadding = FMargin(12.0f, 8.0f);

#if WITH_EDITORONLY_DATA
	/** Shows the generated popup inline in UMG Designer without creating a runtime scope. */
	UPROPERTY(EditAnywhere, Category = "Lunar|UI|ComboBox|Designer Preview", meta = (DesignerRebuild))
	bool bPreviewPopupOpen = false;
#endif

	/** Broadcast after a committed selection change. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|ComboBox|Events")
	FLunarComboBoxSelectionChangedSignature OnSelectionChanged;
	/** Broadcast after the generated popup and nested scope open. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|ComboBox|Events")
	FLunarComboBoxStateChangedSignature OnComboBoxOpened;
	/** Broadcast after the generated popup and nested scope close. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|ComboBox|Events")
	FLunarComboBoxStateChangedSignature OnComboBoxClosed;
	/** Broadcast after external filter text changes. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|ComboBox|Filter|Events")
	FLunarComboBoxFilterTextChangedSignature OnFilterTextChanged;

protected:
	/** @name UWidget and Lunar control lifecycle overrides. @{ */

	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual TSharedPtr<SWidget> RebuildLunarSpecializedPresentation() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual void SynchronizeProperties() override;
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual bool NativeCanHandleLunarAction(const FLunarUIActionContext& ActionContext) const override;
	virtual ELunarUIActionResult NativeHandleLunarAction(const FLunarUIActionContext& ActionContext) override;
	virtual bool NativeCanActivateLunarWidget() const override;
	virtual void NativeOnLunarActivated() override;
	virtual void NativeOnLunarVisualStateChanged(const FLunarUIVisualState& PreviousState, const FLunarUIVisualState& NewState, bool bIsDesignerPreview) override;
	virtual FText NativeGetLunarAccessibleValueText() const override;

	/** @} */

private:
	/** @name Internal option, presentation, popup, animation, scope, and validation helpers. @{ */

	const FLunarComboBoxOption* FindOptionById(FName OptionId) const;
	bool IsOptionNavigable(FName OptionId) const;
	bool IsOptionCommittable(FName OptionId) const;
	void RebuildOptionLookup();
	int32 FindRecoveryIndex(int32 PreferredIndex) const;
	void NormalizeCommittedSelection(int32 PreferredIndex, bool bNotify);
	void RebuildFilteredOptions(bool bPreserveTemporarySelection, bool bUseListViewRestoration = false);
	void SetTemporarySelectionToCommittedOrFirstEligible();
	void CommitTemporarySelection();
	void ApplyCommittedSelection(FName NewOptionId, bool bNotify);
	void UpdateCommittedPresentation();
	void ApplyNativePresentation();
	void ApplyPopupSizing();
	void UpdatePopupContentMode();
	void EnsureInternalWidgets();
	void SynchronizeGeneratedSelectedVisual();
	void SynchronizeGeneratedEmptyVisual();
	TSharedRef<SWidget> BuildClosedPresentation();
	TSharedRef<SWidget> BuildPopupPresentation();
	TSharedRef<SWidget> BuildDesignerPreviewPresentation(const TSharedRef<SWidget>& ClosedPresentation, const TSharedRef<SWidget>& PopupPresentation);
	EMenuPlacement ResolveMenuPlacement() const;
	bool IsResolvedPlacementAbove() const;
	void BeginPopupAnimation(float TargetAlpha);
	void UpdatePopupAnimation(float DeltaTime);
	void ApplyPopupAnimationAlpha(float NewAlpha);
	float EvaluatePopupCurve(float Progress) const;
	bool MustSnapPopupAnimation() const;
	void FinalizePopupClosed();
	void ReportConfigurationErrors(const TSet<FString>& CurrentErrors);
	ULunarNavigationSubsystem* GetNavigationSubsystem() const;
	void RegisterOutsideClickProcessor();
	void UnregisterOutsideClickProcessor();
	bool HandleGlobalPointerDown(const FVector2D& ScreenPosition, uint32 SlateUserIndex);
	void HandleCultureChanged();
	void HandleMenuOpenChanged(bool bIsOpen);


	/** @} */
	/** @name Internal delegate handlers. @{ */
	UFUNCTION()
	void HandleListActiveItemChanged(ULunarListView* SourceListView, FName PreviousItemId, FName NewItemId);
	UFUNCTION()
	void HandleListItemActivated(ULunarListView* SourceListView, int32 ItemIndex, FLunarListViewItemData ItemData);
	UFUNCTION()
	void HandleActiveScopeChanged(ULunarNavigationScope* PreviousScope, ULunarNavigationScope* NewScope);

	/** @} */

	/** @name Generated UObject children owned by this ComboBox. @{ */
	UPROPERTY(Transient) TObjectPtr<ULunarListView> InternalOptionsList;
	UPROPERTY(Transient) TObjectPtr<ULunarComboBoxSelectedVisual> GeneratedSelectedVisual;
	UPROPERTY(Transient) TObjectPtr<ULunarComboBoxEmptyVisual> GeneratedEmptyResultsVisual;
	UPROPERTY(Transient) TObjectPtr<ULunarNavigationScope> PopupScope;

	/** @} */

	/** @name Stable-ID, validation, and culture caches. @{ */
	FName TemporaryOptionId = NAME_None;
	int32 LastCommittedOptionIndex = INDEX_NONE;
	TMap<FName, int32> OptionIndexById;
	TSet<FString> ReportedConfigurationErrors;
	FDelegateHandle CultureChangedHandle;

	/** @} */

	/** @name Cached Slate presentation widgets. @{ */
	TSharedPtr<SMenuAnchor> MenuAnchor;
	TSharedPtr<SBorder> DefaultClosedBorder;
	TSharedPtr<STextBlock> DefaultClosedText;
	TSharedPtr<SImage> DefaultArrowImage;
	TSharedPtr<STextBlock> DefaultArrowGlyph;
	TSharedPtr<SBox> PopupSizingBox;
	TSharedPtr<SBorder> PopupBackgroundBorder;
	TSharedPtr<SWidgetSwitcher> PopupContentSwitcher;
	TSharedPtr<SWidget> PopupAnimationRoot;
	TSharedPtr<FLunarComboBoxOutsideClickInputProcessor> OutsideClickProcessor;

	/** @} */

	/** @name Popup interpolation and re-entrancy state. @{ */
	float PopupAnimationAlpha = 0.0f;
	float PopupAnimationSourceAlpha = 0.0f;
	float PopupAnimationTargetAlpha = 0.0f;
	float PopupAnimationElapsed = 0.0f;
	bool bPopupAnimationActive = false;
	bool bComboBoxOpen = false;
	bool bPopupClosing = false;
	bool bOpeningComboBox = false;
	bool bClosingComboBox = false;
	bool bIgnoreMenuOpenChanged = false;
	bool bCloseRequestedWhileOpening = false;
	bool bCloseRequestedWhileClosing = false;

	/** @} */

	friend class FLunarComboBoxOutsideClickInputProcessor;
};
