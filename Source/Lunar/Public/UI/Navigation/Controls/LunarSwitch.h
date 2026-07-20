// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Navigation/Core/LunarNavigableWidget.h"
#include "LunarSwitch.generated.h"

/** @file LunarSwitch.h @brief Binary navigable Lunar switch control. @ingroup LunarNavigationControls */
class SLunarSwitchVisual;
class UCurveFloat;

/** @brief Binary value control toggled by Accept and optionally set by one directional axis. */
UCLASS(Blueprintable)
class LUNAR_API ULunarSwitch : public ULunarNavigableWidget
{
	GENERATED_BODY()

public:
	/** @param ObjectInitializer Unreal object initializer. */
	ULunarSwitch(const FObjectInitializer& ObjectInitializer);
	/** @param bNewIsOn New on/off value. @param NotificationPolicy Whether to publish normal change notifications; defaults to Notify. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Switch", meta = (AdvancedDisplay = "NotificationPolicy")) void SetIsOn(bool bNewIsOn, ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);
	/** @return True when the switch is on. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Switch") bool IsOn() const { return bIsOn; }
	/** @return True when the switch is off. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Switch") bool IsOff() const { return !bIsOn; }
	/** @brief Inverts the current value. @param NotificationPolicy Whether to publish normal change notifications; defaults to Notify. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Switch", meta = (AdvancedDisplay = "NotificationPolicy")) void Toggle(ELunarChangeNotificationPolicy NotificationPolicy = ELunarChangeNotificationPolicy::Notify);
	/**
	 * @brief Gets the transient normalized position currently rendered by the native Handle.
	 * @return Displayed Handle position in the inclusive 0..1 range.
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Switch")
	float GetDisplayedHandleAlpha() const;

	/** @param NewBrush New track brush. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Switch|Presentation") void SetTrackBrush(const FSlateBrush& NewBrush);
	/** @return Current track brush. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Switch|Presentation") FSlateBrush GetTrackBrush() const { return TrackBrush; }
	/** @param NewTint New track tint. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Switch|Presentation") void SetTrackTint(FLinearColor NewTint);
	/** @return Current track tint. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Switch|Presentation") FLinearColor GetTrackTint() const { return TrackTint; }
	/** @param NewSize New track size. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Switch|Presentation") void SetTrackSize(FVector2D NewSize);
	/** @return Current track size. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Switch|Presentation") FVector2D GetTrackSize() const { return TrackSize; }
	/** @param NewBrush New handle brush. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Switch|Presentation") void SetHandleBrush(const FSlateBrush& NewBrush);
	/** @return Current handle brush. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Switch|Presentation") FSlateBrush GetHandleBrush() const { return HandleBrush; }
	/** @param NewTint New handle tint. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Switch|Presentation") void SetHandleTint(FLinearColor NewTint);
	/** @return Current handle tint. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Switch|Presentation") FLinearColor GetHandleTint() const { return HandleTint; }
	/** @param NewSize New handle size. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Switch|Presentation") void SetHandleSize(FVector2D NewSize);
	/** @return Current handle size. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Switch|Presentation") FVector2D GetHandleSize() const { return HandleSize; }
	/** @param NewOffset New handle offset. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Switch|Presentation") void SetHandleOffset(FVector2D NewOffset);
	/** @return Current handle offset. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Switch|Presentation") FVector2D GetHandleOffset() const { return HandleOffset; }
	/** @brief Configures all native track fields. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Switch|Presentation") void ConfigureTrackPresentation(const FSlateBrush& NewBrush, FLinearColor NewTint, FVector2D NewSize);
	/** @brief Returns all native track fields. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Switch|Presentation") void GetTrackPresentation(FSlateBrush& OutBrush, FLinearColor& OutTint, FVector2D& OutSize) const;
	/** @brief Configures all native handle fields. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Switch|Presentation") void ConfigureHandlePresentation(const FSlateBrush& NewBrush, FLinearColor NewTint, FVector2D NewSize, FVector2D NewOffset);
	/** @brief Returns all native handle fields. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Switch|Presentation") void GetHandlePresentation(FSlateBrush& OutBrush, FLinearColor& OutTint, FVector2D& OutSize, FVector2D& OutOffset) const;
	/** @brief Configures every native switch part. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Switch|Presentation") void ConfigureSwitchPresentation(const FSlateBrush& NewTrackBrush, FLinearColor NewTrackTint, FVector2D NewTrackSize, const FSlateBrush& NewHandleBrush, FLinearColor NewHandleTint, FVector2D NewHandleSize, FVector2D NewHandleOffset);
	/** @brief Returns every native switch presentation value. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Switch|Presentation") void GetSwitchPresentation(FSlateBrush& OutTrackBrush, FLinearColor& OutTrackTint, FVector2D& OutTrackSize, FSlateBrush& OutHandleBrush, FLinearColor& OutHandleTint, FVector2D& OutHandleSize, FVector2D& OutHandleOffset) const;

	/** Current logical on/off value. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Switch") bool bIsOn = false;

	/** Enables visual interpolation between the Off and On Handle positions. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Switch|Interpolation", meta = (DisplayName = "Interpolate Handle Movement"))
	bool bInterpolateHandleMovement = false;

	/** Normalized curve traversals per second; zero applies the logical value immediately. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Switch|Interpolation", meta = (DisplayName = "Handle Interpolation Speed", ClampMin = "0.0", UIMin = "0.0", EditCondition = "bInterpolateHandleMovement"))
	float HandleInterpolationSpeed = 12.0f;

	/** Normalized 0..1 response curve used for visual interpolation; null falls back to linear interpolation. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Switch|Interpolation", meta = (DisplayName = "Handle Interpolation Curve", EditCondition = "bInterpolateHandleMovement"))
	TObjectPtr<UCurveFloat> HandleInterpolationCurve = nullptr;

	/** Optional directional axis. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Switch") ELunarSwitchDirectionMode DirectionMode = ELunarSwitchDirectionMode::Horizontal;

	/** Broadcast after value changes. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Switch") FLunarSwitchChangedSignature OnSwitchChanged;

	/** Broadcast whenever the rendered Handle position changes, including each interpolation frame. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Switch|Interpolation")
	FLunarSwitchDisplayedHandleAlphaChangedSignature OnDisplayedHandleAlphaChanged;

protected:
	/** @return Lunar presentation wrapped by the Switch-owned pointer interaction surface. */
	virtual TSharedRef<SWidget> RebuildWidget() override;
	/** @return Native switch layer. */
	virtual TSharedPtr<SWidget> RebuildLunarSpecializedPresentation() override;
	/** @param bReleaseChildren Whether child Slate resources are released. */
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	/** @brief Synchronizes authored value and presentation. */
	virtual void SynchronizeProperties() override;
	/** @brief Advances optional Handle-position interpolation. @param MyGeometry Current widget geometry. @param InDeltaTime Frame time in seconds. */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	/** @return True when the action is supported. */
	virtual bool NativeCanHandleLunarAction(const FLunarUIActionContext& ActionContext) const override;
	/** @return Action handling result. */
	virtual ELunarUIActionResult NativeHandleLunarAction(const FLunarUIActionContext& ActionContext) override;
	/** @return True when one direction maps to a value action. */
	virtual bool NativeResolveDirectionalLunarControlAction(ELunarNavigationDirection Direction, FGameplayTag& OutActionTag) const override;
	/** @brief Toggles after permitted Accept activation. */
	virtual void NativeOnLunarActivated() override;
	/** @return Localized on/off accessibility text. */
	virtual FText NativeGetLunarAccessibleValueText() const override;

private:
	/** @brief Updates the published value state. */
	void ApplyValueState();
	/** @brief Advances optional visual interpolation toward the logical Switch value. @param DeltaTime Non-negative frame time in seconds. */
	void UpdateDisplayedHandleInterpolation(float DeltaTime);
	/** @brief Snaps or retargets the rendered Handle position. @param bBroadcast Whether to broadcast OnDisplayedHandleAlphaChanged. */
	void RefreshDisplayedHandleTarget(bool bBroadcast);
	/** @brief Stores a normalized rendered Handle position. @param NewDisplayedHandleAlpha Requested 0..1 position. @param bBroadcast Whether to broadcast OnDisplayedHandleAlphaChanged. */
	void SetDisplayedHandleAlphaInternal(float NewDisplayedHandleAlpha, bool bBroadcast);
	/** @brief Normalizes authored interpolation settings. */
	void NormalizeInterpolationSettings();
	/** @brief Pushes cached value and presentation into Slate. */
	void SynchronizeSpecializedPresentation();

	/** Cached native track brush. */
	UPROPERTY(Transient) FSlateBrush TrackBrush;
	/** Cached native track tint. */
	UPROPERTY(Transient) FLinearColor TrackTint = FLinearColor(0.35f, 0.35f, 0.35f, 1.0f);
	/** Cached native track size. */
	UPROPERTY(Transient) FVector2D TrackSize = FVector2D(48.0f, 24.0f);
	/** Cached native handle brush. */
	UPROPERTY(Transient) FSlateBrush HandleBrush;
	/** Cached native handle tint. */
	UPROPERTY(Transient) FLinearColor HandleTint = FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);
	/** Cached native handle size. */
	UPROPERTY(Transient) FVector2D HandleSize = FVector2D(20.0f, 20.0f);
	/** Cached native handle offset. */
	UPROPERTY(Transient) FVector2D HandleOffset = FVector2D::ZeroVector;
	/** Transient normalized Handle position rendered by native or owner-authored presentation. */
	UPROPERTY(Transient) float DisplayedHandleAlpha = 0.0f;
	/** Whether DisplayedHandleAlpha has been initialized from a valid logical Switch value. */
	UPROPERTY(Transient) bool bDisplayedHandleAlphaInitialized = false;
	/** Rendered Handle position captured when the current curve traversal began. */
	float DisplayedHandleInterpolationSource = 0.0f;
	/** Logical Handle position targeted by the current curve traversal. */
	float DisplayedHandleInterpolationTarget = 0.0f;
	/** Seconds elapsed since the current curve traversal began. */
	float DisplayedHandleInterpolationElapsed = 0.0f;
	/** Whether a curve traversal toward DisplayedHandleInterpolationTarget is active. */
	bool bDisplayedHandleInterpolationActive = false;
	/** Native track-and-handle presentation. */
	TSharedPtr<SLunarSwitchVisual> SwitchVisual;
};