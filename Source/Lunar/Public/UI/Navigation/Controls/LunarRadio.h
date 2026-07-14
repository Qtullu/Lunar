// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Navigation/Core/LunarNavigableWidget.h"
#include "LunarRadio.generated.h"

/**
 * @file LunarRadio.h
 * @brief Navigable Lunar radio control
 * @ingroup LunarNavigationControls
 */

class SBox;
class SImage;
class ULunarRadioGroup;

/**
 * @brief Navigable radio whose Lunar selection and checked state remain independent
 * @ingroup LunarNavigationControls
 */
UCLASS(Blueprintable)
class LUNAR_API ULunarRadio : public ULunarNavigableWidget
{
	GENERATED_BODY()

public:
	/**
	 * @brief Creates a selectable radio with prompt and pointer support
	 * @param ObjectInitializer Unreal object initializer
	 */
	ULunarRadio(const FObjectInitializer& ObjectInitializer);

	/**
	 * @brief Updates checked state while preserving the owning group's invariants
	 * @param bNewChecked Requested checked state
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|Radio")
	void SetChecked(bool bNewChecked);

	/**
	 * @brief Checks the logical radio value
	 * @return True when this radio is checked
	 */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation|Radio")
	bool IsChecked() const { return bIsChecked; }

	/**
	 * @brief Applies Accept semantics by selecting this radio or optionally clearing it
	 * @return True when the owning group or standalone radio accepted the request
	 */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation|Radio")
	bool RequestChecked();

	/** Stable non-empty ID required while this radio belongs to a group. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Radio")
	FName RadioId = NAME_None;

	/** Optional shared non-visual owner that enforces group selection invariants. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lunar|UI|Navigation|Radio", meta = (ExposeOnSpawn = "true"))
	TObjectPtr<ULunarRadioGroup> RadioGroup;

	/** Current logical checked state, independent from Lunar navigation selection. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation|Radio")
	bool bIsChecked = false;

	/** Broadcast after the logical checked state changes. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation|Radio")
	FLunarRadioCheckedChangedSignature OnCheckedChanged;

protected:
	/** @brief Creates the native radio-mark presentation layer. */
	virtual TSharedPtr<SWidget> RebuildLunarSpecializedPresentation() override;
	/**
	 * @brief Releases the native radio-mark presentation
	 * @param bReleaseChildren Whether child Slate resources should also be released
	 */
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	/** @brief Synchronizes authored value, registration, and native presentation. */
	virtual void SynchronizeProperties() override;
	/** @brief Registers this radio with its configured group at runtime. */
	virtual void NativeConstruct() override;
	/** @brief Unregisters this radio from its group before destruction. */
	virtual void NativeDestruct() override;
	/**
	 * @brief Advances registration recovery and style transitions
	 * @param MyGeometry Current cached widget geometry
	 * @param InDeltaTime Elapsed frame time in seconds
	 */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	/**
	 * @brief Checks whether this radio can consume the supplied semantic action
	 * @param ActionContext Semantic action and input context being queried
	 * @return True when this radio can handle the action
	 */
	virtual bool NativeCanHandleLunarAction(const FLunarUIActionContext& ActionContext) const override;
	/**
	 * @brief Handles Accept press and release semantics
	 * @param ActionContext Semantic action and input context to process
	 * @return Handling result reported to the navigation subsystem
	 */
	virtual ELunarUIActionResult NativeHandleLunarAction(const FLunarUIActionContext& ActionContext) override;
	/**
	 * @brief Checks group invariants before activation
	 * @return True when RequestChecked can produce a permitted state
	 */
	virtual bool NativeCanActivateLunarWidget() const override;
	/** @brief Applies RequestChecked after a permitted activation. */
	virtual void NativeOnLunarActivated() override;
	/**
	 * @brief Builds accessibility text for the current checked state
	 * @return Localized checked or unchecked value text
	 */
	virtual FText NativeGetLunarAccessibleValueText() const override;
	/**
	 * @brief Resolves the compatible radio style and caches specialized fields
	 * @param OutStyle Receives the resolved common style values
	 * @param OutError Receives a configuration error when resolution fails
	 * @return True when the radio style was resolved successfully
	 */
	virtual bool ResolveCommonStylePatch(FLunarCommonStylePatch& OutStyle, FString& OutError) const override;
	/**
	 * @brief Applies common style values and starts the specialized mark transition
	 * @param ResolvedStyle Resolved common style values
	 */
	virtual void ApplyResolvedCommonStyle(const FLunarCommonStylePatch& ResolvedStyle) override;

private:
	/**
	 * @brief Changes the local checked value without consulting or broadcasting the group
	 * @param bNewChecked Requested checked state
	 * @return True when the stored value changed
	 */
	bool ApplyCheckedStateSilently(bool bNewChecked);
	/**
	 * @brief Broadcasts and announces a previously applied checked-state change
	 * @param bPreviousChecked Checked state before the silent update
	 */
	void BroadcastCheckedStateChanged(bool bPreviousChecked);
	/** @brief Reconciles runtime registration with RadioGroup and RadioId. */
	void SynchronizeRadioGroupRegistration();
	/** @brief Removes this radio from its currently registered group. */
	void UnregisterFromRadioGroup();
	/**
	 * @brief Starts, reverses, or immediately applies a specialized style target
	 * @param NewTarget Materialized specialized style target
	 */
	void ApplyRadioStyleTarget(const FLunarRadioStylePatch& NewTarget);
	/** @brief Applies the latest resolved radio style and updates the mark widgets. */
	void ApplyResolvedRadioStyle();
	/**
	 * @brief Advances the active specialized radio-style transition
	 * @param DeltaTime Elapsed frame time in seconds
	 */
	void TickRadioStyleTransition(float DeltaTime);

	/** Specialized style patch produced by the latest style resolution. */
	UPROPERTY(Transient)
	FLunarRadioStylePatch ResolvedRadioStyle;

	/** Previously resolved specialized style used to detect target changes. */
	UPROPERTY(Transient)
	FLunarRadioStylePatch PreviousRadioStyle;

	/** Specialized style snapshot currently displayed by Slate. */
	UPROPERTY(Transient)
	FLunarRadioStylePatch DisplayedRadioStyle;

	/** Specialized style snapshot at the start of the active transition. */
	UPROPERTY(Transient)
	FLunarRadioStylePatch TransitionSourceRadioStyle;

	/** Materialized destination of the active specialized style transition. */
	UPROPERTY(Transient)
	FLunarRadioStylePatch TransitionTargetRadioStyle;

	/** Latest logical specialized style target, including discrete fields. */
	UPROPERTY(Transient)
	FLunarRadioStylePatch LogicalTargetRadioStyle;
	/** Group with which this radio is currently registered. */
	TWeakObjectPtr<ULunarRadioGroup> RegisteredRadioGroup;
	/** Most recently attempted group, used to avoid repeated invalid registration logs. */
	TWeakObjectPtr<ULunarRadioGroup> AttemptedRadioGroup;
	/** Radio ID used for the most recent registration attempt. */
	FName AttemptedRadioId = NAME_None;
	/** Native image that renders the radio mark. */
	TSharedPtr<SImage> RadioMarkImage;
	/** Native size box that controls radio-mark dimensions. */
	TSharedPtr<SBox> RadioMarkSizeBox;
	/** Whether NativeConstruct has completed and runtime registration is permitted. */
	bool bRadioConstructed = false;
	/** Elapsed time in the active specialized style transition. */
	float RadioStyleTransitionElapsed = 0.0f;
	/** Duration of the active specialized style transition. */
	float RadioStyleTransitionDuration = 0.0f;
	/** Whether a specialized style transition is currently active. */
	bool bRadioStyleTransitionActive = false;
	/** Whether the active transition is reversing toward its previous source. */
	bool bRadioStyleTransitionReversing = false;
	/** Whether DisplayedRadioStyle contains a valid materialized snapshot. */
	bool bHasDisplayedRadioStyle = false;

	/** Grants the group controlled access to silent state and notification helpers. */
	friend class ULunarRadioGroup;
};
