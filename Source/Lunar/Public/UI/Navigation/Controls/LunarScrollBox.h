// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ScrollBox.h"
#include "Containers/Ticker.h"
#include "UI/Navigation/Types/LunarNavigationTypes.h"
#include "LunarScrollBox.generated.h"

/**
 * @file LunarScrollBox.h
 * @brief Non-selectable Lunar scroll container
 * @ingroup LunarNavigationControls
 */

class ULunarNavigationSubsystem;
class SLunarNavigationScrollBox;
class ULunarScrollBox;
class UCurveFloat;

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
UCLASS(BlueprintType, Blueprintable)
class LUNAR_API ULunarScrollBox : public UScrollBox
{
	GENERATED_BODY()

public:
	/**
	 * @brief Creates a scroll container with Lunar runtime policies and settings defaults
	 * @param ObjectInitializer Unreal object initializer
	 */
	ULunarScrollBox(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	/** @return Shared UMG Palette category for Lunar navigation controls and containers. */
	virtual const FText GetPaletteCategory() override;
#endif

	/**
	 * @brief Reveals a descendant using the minimum required offset for this container
	 * @param WidgetToReveal Descendant widget that should become visible
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Scroll")
	void ScrollWidgetIntoLunarView(UWidget* WidgetToReveal);

	/** Stops the current Lunar scroll and releases any direct-scroll capture. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Scroll")
	void CancelLunarScroll();

	/**
	 * @brief Checks whether a smooth selection-driven Lunar scroll is active
	 * @return True while a Lunar smooth-scroll operation is running
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Scroll")
	bool IsLunarScrollActive() const;

	/** Sets the cached native scroll-bar style. @param NewStyle New native scroll-bar style. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Scroll|Presentation")
	void SetScrollBarPresentationStyle(const FScrollBarStyle& NewStyle);

	/** Returns the cached native scroll-bar style. @return Current native scroll-bar style. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Scroll|Presentation")
	FScrollBarStyle GetScrollBarPresentationStyle() const { return ScrollBarPresentationStyle; }

	/** Sets padding around the native scroll bar. @param NewPadding New scroll-bar padding. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Scroll|Presentation")
	void SetScrollBarPresentationPadding(FMargin NewPadding);

	/** Returns padding around the native scroll bar. @return Current scroll-bar padding. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Scroll|Presentation")
	FMargin GetScrollBarPresentationPadding() const { return ScrollBarPresentationPadding; }

	/** Configures every native ScrollBox presentation value. @param NewStyle New scroll-bar style. @param NewPadding New scroll-bar padding. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Scroll|Presentation")
	void ConfigureScrollBoxPresentation(const FScrollBarStyle& NewStyle, FMargin NewPadding);

	/** Returns every cached native ScrollBox presentation value. @param OutStyle Current scroll-bar style. @param OutPadding Current scroll-bar padding. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Scroll|Presentation")
	void GetScrollBoxPresentation(FScrollBarStyle& OutStyle, FMargin& OutPadding) const;
	/** @brief Synchronizes runtime policies, settings, and cached native presentation. */
	virtual void SynchronizeProperties() override;
	/**
	 * @brief Releases custom Slate and ticker resources
	 * @param bReleaseChildren Whether child Slate resources should also be released
	 */
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	/** @brief Cancels outstanding ticker work before UObject destruction. */
	virtual void BeginDestroy() override;

	/** Safe padding retained between a selected descendant and the visible viewport edges. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Scroll")
	FMargin ViewportPadding = FMargin(0.0f);

	/** Primary scrolling and local-navigation axis exposed in Lunar Details. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Scroll", meta = (DisplayName = "Orientation"))
	TEnumAsByte<EOrientation> ScrollOrientation = Orient_Vertical;

	/** Wraps navigation between opposite ends along this container's orientation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Scroll")
	bool bWrapNavigation = false;

	/** Traps Lunar selection inside this container until explicitly released or Back is pressed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Scroll")
	bool bConstrainNavigation = false;

	/** Releases active confinement after a handled navigation Accept; pointer activation is unaffected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Scroll", meta = (EditCondition = "bConstrainNavigation"))
	bool bExitConfinementOnNavigationAccept = false;

	/** Lets an unused wheel delta bubble to an ancestor scroll container. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Scroll")
	bool bAllowScrollChaining = true;

	/** Interpolates selection-driven reveal scrolling instead of snapping immediately. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Scroll|Interpolation", meta = (DisplayName = "Interpolate Scroll Into View"))
	bool bInterpolateScrollIntoView = false;

	/** Controls normalized selection-scroll curve traversals per second; zero snaps immediately. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Scroll|Interpolation", meta = (ClampMin = "0.0", UIMin = "0.0", EditCondition = "bInterpolateScrollIntoView"))
	float ScrollInterpolationSpeed = 12.0f;

	/** Maps normalized reveal progress to presentation alpha; null uses linear interpolation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Scroll|Interpolation", meta = (EditCondition = "bInterpolateScrollIntoView"))
	TObjectPtr<UCurveFloat> ScrollInterpolationCurve = nullptr;

	/** Broadcast when a selection-driven smooth scroll begins. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Scroll")
	FLunarScrollStateChangedSignature OnLunarScrollStarted;

	/** Broadcast when a selection-driven smooth scroll completes or is cancelled. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Scroll")
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
	/** @brief Enforces non-focusable Lunar container input and clipping policies. */
	void ApplyRuntimePolicies();
	/** Applies cached native scroll-bar presentation to UMG and Slate. */
	void ApplyNativePresentation();

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
	 * @brief Starts an interpolated reveal operation toward a target offset
	 * @param WidgetToReveal Descendant whose validity must remain tracked
	 * @param TargetOffset Destination scroll offset
	 */
	void StartInterpolatedLunarScroll(UWidget* WidgetToReveal, float TargetOffset);
	/**
	 * @brief Advances the active smooth reveal operation
	 * @param DeltaTime Elapsed ticker time in seconds
	 * @return True to keep the ticker active, false after completion or cancellation
	 */
	bool TickInterpolatedLunarScroll(float DeltaTime);
	/** @param Progress Normalized interpolation progress. @return Sanitized curve alpha, or Progress when the curve is null or invalid. */
	float EvaluateScrollInterpolationCurve(float Progress) const;
	/** @return True when selection reveal must snap because interpolation is unavailable or suppressed. */
	bool ShouldSnapScrollIntoView() const;
	/** @brief Sanitizes editable interpolation values before they are consumed. */
	void NormalizeInterpolationSettings();
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

	/** Scroll offset captured at the start of a smooth reveal operation. */
	float LunarScrollStartOffset = 0.0f;
	/** Destination offset of the active smooth reveal operation. */
	float LunarScrollTargetOffset = 0.0f;
	/** Elapsed time in the active smooth reveal operation. */
	float LunarScrollElapsed = 0.0f;

	/** Last offset authorized by Lunar or direct input ownership. */
	float LastAuthorizedScrollOffset = 0.0f;
	/** Whether a selection-driven smooth reveal operation is active. */
	bool bLunarScrollActive = false;
	/** Guards the native scroll callback while Lunar applies an authorized offset. */
	bool bApplyingLunarScrollOffset = false;
	/** Cached native scroll-bar style reapplied after Slate reconstruction. */
	UPROPERTY(Transient)
	FScrollBarStyle ScrollBarPresentationStyle;
	/** Cached native scroll-bar padding reapplied after Slate reconstruction. */
	UPROPERTY(Transient)
	FMargin ScrollBarPresentationPadding;
	/** Native completion signal used by the navigation subsystem. */
	FOnLunarScrollFinishedNative LunarScrollFinishedNative;

	/** Grants the customized Slate scroll box access to direct-input helpers. */
	friend class SLunarNavigationScrollBox;
	/** Grants the navigation subsystem access to native completion coordination. */
	friend class ULunarNavigationSubsystem;
};
