// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ScrollBox.h"
#include "Containers/Ticker.h"
#include "UI/Navigation/Types/LunarNavigationTypes.h"
#include "UI/Navigation/Types/LunarUIStyleTypes.h"
#include "LunarScrollBox.generated.h"

/**
 * @file LunarScrollBox.h
 * @brief Non-selectable Lunar scroll container
 * @ingroup LunarNavigationControls
 */

class ULunarNavigationSubsystem;
class ULunarScrollBoxStyleAsset;
class SLunarNavigationScrollBox;
class ULunarScrollBox;

/**
 * @brief Native completion notification for selection-driven Lunar scrolling
 *
 * Receivers get the scroll container whose operation ended and whether the
 * target offset was reached instead of cancelled.
 *
 * @ingroup LunarNavigationControls
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(
	FOnLunarScrollFinishedNative,
	ULunarScrollBox* /* ScrollBox */,
	bool /* bCompleted */);

/**
 * @brief Non-selectable Lunar scroll container
 *
 * The owning local player's navigation subsystem uses this container to reveal
 * selected descendants. Native UScrollBox wheel, right-drag, and touch input
 * remain responsible for direct scrolling.
 *
 * @ingroup LunarNavigationControls
 */
UCLASS(BlueprintType)
class LUNAR_API ULunarScrollBox : public UScrollBox
{
	GENERATED_BODY()

public:
	/**
	 * @brief Creates a scroll container with Lunar runtime policies and settings defaults
	 * @param ObjectInitializer Unreal object initializer
	 */
	ULunarScrollBox(const FObjectInitializer& ObjectInitializer);

	/**
	 * @brief Reveals a descendant using the minimum required offset for this container
	 * @param WidgetToReveal Descendant widget that should become visible
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|Scroll")
	void ScrollWidgetIntoLunarView(UWidget* WidgetToReveal);

	/** Stops the current Lunar scroll and releases any direct-scroll capture. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|Scroll")
	void CancelLunarScroll();

	/**
	 * @brief Checks whether a smooth selection-driven Lunar scroll is active
	 * @return True while a Lunar smooth-scroll operation is running
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation|Scroll")
	bool IsLunarScrollActive() const;

	/** @brief Synchronizes runtime policies, settings, and resolved style. */
	virtual void SynchronizeProperties() override;
	/**
	 * @brief Releases custom Slate and ticker resources
	 * @param bReleaseChildren Whether child Slate resources should also be released
	 */
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	/** @brief Cancels outstanding ticker work before UObject destruction. */
	virtual void BeginDestroy() override;

	/** Per-instance reveal behavior. A positive duration takes precedence over speed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Scroll")
	FLunarScrollIntoViewSettings ScrollIntoViewSettings;

	/** Lets an unused wheel delta bubble to an ancestor scroll container. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Scroll")
	bool bAllowScrollChaining = true;

	/** Strongly typed ScrollBox style. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Style")
	TObjectPtr<ULunarScrollBoxStyleAsset> StyleAsset;

	/** Broadcast when a selection-driven smooth scroll begins. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation|Scroll")
	FLunarScrollStateChangedSignature OnLunarScrollStarted;

	/** Broadcast when a selection-driven smooth scroll completes or is cancelled. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation|Scroll")
	FLunarScrollStateChangedSignature OnLunarScrollFinished;

protected:
	/**
	 * @brief Builds the customized native SScrollBox used for direct-input ownership and chaining
	 * @return Rebuilt Slate widget
	 */
	virtual TSharedRef<SWidget> RebuildWidget() override;
	/** @brief Registers the rebuilt container and binds native scroll notifications. */
	virtual void OnWidgetRebuilt() override;

private:
	/**
	 * @brief Handles native scroll-offset changes and enforces scope ownership
	 * @param CurrentOffset Offset reported by the native scroll box
	 */
	UFUNCTION()
	void HandleNativeUserScrolled(float CurrentOffset);

	/** @return Navigation subsystem for the owning local player, or null when unavailable. */
	ULunarNavigationSubsystem* ResolveNavigationSubsystem() const;
	/** @brief Registers this container with its current local-player subsystem. */
	void RegisterWithNavigationSubsystem();
	/** @brief Removes this container from its previously registered subsystem. */
	void UnregisterFromNavigationSubsystem();
	/** @brief Resolves and applies the active strongly typed ScrollBox style. */
	void ApplyResolvedStyle();
	/** @brief Captures native style values used when an override stops applying. */
	void EnsureStyleBaseline();
	/**
	 * @brief Expands a resolved style patch against captured native baselines
	 * @param ResolvedStyle Partial style patch produced by style resolution
	 * @return Complete materialized style snapshot suitable for transitions
	 */
	FLunarScrollBoxStylePatch MaterializeStyleSnapshot(const FLunarScrollBoxStylePatch& ResolvedStyle) const;
	/**
	 * @brief Starts, reverses, or immediately applies a materialized style target
	 * @param NewTarget New logical style target
	 */
	void ApplyStyleTarget(const FLunarScrollBoxStylePatch& NewTarget);
	/**
	 * @brief Stores and applies one interpolated style snapshot
	 * @param NewDisplayedStyle Style snapshot to display
	 */
	void ApplyDisplayedStyle(const FLunarScrollBoxStylePatch& NewDisplayedStyle);
	/**
	 * @brief Advances the active style transition
	 * @param DeltaTime Elapsed ticker time in seconds
	 * @return True to keep the ticker active, false after completion
	 */
	bool TickStyleTransition(float DeltaTime);
	/** @brief Stops and unregisters the style-transition ticker. */
	void StopStyleTransition();
	/** @brief Enforces non-focusable Lunar container input and clipping policies. */
	void ApplyRuntimePolicies();
	/**
	 * @brief Updates the direct-pointer interaction state and refreshes style
	 * @param NewInteractionState New pointer interaction state
	 */
	void SetDirectInteractionState(ELunarUIInteractionState NewInteractionState);

	/**
	 * @brief Validates that a reveal target is a visible descendant of this container
	 * @param WidgetToReveal Candidate reveal target
	 * @return True when the target can be used for Lunar scrolling
	 */
	bool IsValidLunarScrollTarget(const UWidget* WidgetToReveal) const;
	/**
	 * @brief Calculates the smallest padded offset that fully reveals a descendant
	 * @param WidgetToReveal Valid descendant to reveal
	 * @param OutTargetOffset Receives the required scroll offset
	 * @return True when valid cached geometry is available for the calculation
	 */
	bool CalculateMinimumScrollOffset(const UWidget* WidgetToReveal, float& OutTargetOffset) const;
	/**
	 * @brief Applies a clamped authorized selection-driven offset
	 * @param NewOffset Requested offset in Slate units
	 */
	void ApplyLunarScrollOffset(float NewOffset);
	/**
	 * @brief Starts a smooth reveal operation toward a target offset
	 * @param WidgetToReveal Descendant whose validity must remain tracked
	 * @param TargetOffset Destination scroll offset
	 */
	void StartSmoothLunarScroll(UWidget* WidgetToReveal, float TargetOffset);
	/**
	 * @brief Advances the active smooth reveal operation
	 * @param DeltaTime Elapsed ticker time in seconds
	 * @return True to keep the ticker active, false after completion or cancellation
	 */
	bool TickSmoothLunarScroll(float DeltaTime);
	/**
	 * @brief Finalizes an active smooth reveal operation
	 * @param bBroadcastFinished Whether to broadcast completion notifications
	 * @param bCompleted True when the target was reached, false when cancelled
	 */
	void FinishLunarScroll(bool bBroadcastFinished, bool bCompleted);
	/**
	 * @brief Cancels active smooth scrolling and optionally releases pointer capture
	 * @param bBroadcastFinished Whether to broadcast a cancelled completion
	 * @param bReleasePointerCapture Whether direct-scroll capture should also be released
	 */
	void CancelLunarScrollInternal(bool bBroadcastFinished, bool bReleasePointerCapture);
	/** @brief Releases pointer capture owned by the native Lunar scroll box. */
	void ReleaseOwnedPointerCapture();
	/** @brief Cancels selection-driven scrolling when direct user input takes control. */
	void NotifyDirectScrollInput();
	/**
	 * @brief Applies one direct-input delta while respecting ownership and bounds
	 * @param ScrollDelta Requested scroll delta in Slate units
	 * @param bOutApplied Receives whether any offset was applied
	 * @param bOutConsumed Receives whether this container intentionally consumed the request
	 * @param bRequireSameOrientation Whether a mismatched orientation must reject the delta
	 * @return Unapplied remainder that may be forwarded to an ancestor
	 */
	float ApplyDirectScrollDelta(float ScrollDelta, bool& bOutApplied, bool& bOutConsumed, bool bRequireSameOrientation);
	/**
	 * @brief Forwards an unused direct-input delta to the nearest eligible ancestor
	 * @param ScrollDelta Unused scroll delta in Slate units
	 */
	void ForwardDirectScrollRemainder(float ScrollDelta);
	/**
	 * @brief Finds the nearest ancestor Lunar scroll box eligible for chaining
	 * @param bRequireSameOrientation Whether the ancestor must use the same orientation
	 * @return Eligible ancestor, or null when none exists
	 */
	ULunarScrollBox* FindNearestChainedAncestor(bool bRequireSameOrientation) const;
	/** @return True when this container belongs to the active local-player scope. */
	bool IsOwnedByActiveScope() const;
	/** @return Mutable native completion delegate used by subsystem coordination. */
	FOnLunarScrollFinishedNative& OnLunarScrollFinishedNative() { return LunarScrollFinishedNative; }

	/** Local-player subsystem with which this container is currently registered. */
	TWeakObjectPtr<ULunarNavigationSubsystem> RegisteredNavigationSubsystem;
	/** Descendant whose reveal operation is currently active. */
	TWeakObjectPtr<UWidget> ActiveScrollTarget;
	/** Core ticker handle for the active selection-driven smooth scroll. */
	FTSTicker::FDelegateHandle LunarScrollTickerHandle;
	/** Core ticker handle for the active visual-style transition. */
	FTSTicker::FDelegateHandle StyleTransitionTickerHandle;

	/** Scroll offset captured at the start of a smooth reveal operation. */
	float LunarScrollStartOffset = 0.0f;
	/** Destination offset of the active smooth reveal operation. */
	float LunarScrollTargetOffset = 0.0f;
	/** Elapsed time in the active smooth reveal operation. */
	float LunarScrollElapsed = 0.0f;
	/** Calculated duration of the active smooth reveal operation. */
	float LunarScrollDuration = 0.0f;
	/** Last offset authorized by Lunar or direct input ownership. */
	float LastAuthorizedScrollOffset = 0.0f;
	/** Whether a selection-driven smooth reveal operation is active. */
	bool bLunarScrollActive = false;
	/** Guards the native scroll callback while Lunar applies an authorized offset. */
	bool bApplyingLunarScrollOffset = false;
	/** Direct mouse or touch interaction state used for style resolution. */
	ELunarUIInteractionState DirectInteractionState = ELunarUIInteractionState::PointerNormal;
	/** Complete style snapshot currently applied to the container. */
	FLunarScrollBoxStylePatch DisplayedStyle;
	/** Style snapshot at the start of the active transition. */
	FLunarScrollBoxStylePatch TransitionSourceStyle;
	/** Materialized destination of the active style transition. */
	FLunarScrollBoxStylePatch TransitionTargetStyle;
	/** Latest logical style target, including discrete fields. */
	FLunarScrollBoxStylePatch LogicalTargetStyle;
	/** Elapsed time in the active style transition. */
	float StyleTransitionElapsed = 0.0f;
	/** Duration of the active style transition. */
	float StyleTransitionDuration = 0.0f;
	/** Native render opacity captured before Lunar style overrides. */
	float StyleBaselineRenderOpacity = 1.0f;
	/** Native render transform captured before Lunar style overrides. */
	FWidgetTransform StyleBaselineRenderTransform;
	/** Native render-transform pivot captured before Lunar style overrides. */
	FVector2D StyleBaselineRenderTransformPivot = FVector2D::ZeroVector;
	/** Native scroll-bar style captured before Lunar style overrides. */
	FScrollBarStyle StyleBaselineScrollBarStyle;
	/** Native scroll-bar padding captured before Lunar style overrides. */
	FMargin StyleBaselineScrollBarPadding;
	/** Whether a visual-style transition is currently active. */
	bool bStyleTransitionActive = false;
	/** Whether the active transition is reversing toward its previous source. */
	bool bStyleTransitionReversing = false;
	/** Whether DisplayedStyle contains a valid materialized snapshot. */
	bool bHasDisplayedStyle = false;
	/** Whether native baseline values have been captured. */
	bool bHasStyleBaseline = false;
	/** Native completion signal used by the navigation subsystem. */
	FOnLunarScrollFinishedNative LunarScrollFinishedNative;

	/** Grants the customized Slate scroll box access to direct-input helpers. */
	friend class SLunarNavigationScrollBox;
	/** Grants the navigation subsystem access to native completion coordination. */
	friend class ULunarNavigationSubsystem;
};
