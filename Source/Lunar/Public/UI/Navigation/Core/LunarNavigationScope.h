// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarNavigationScope.h
 * @brief Declares the non-visual navigation graph owned by one local player.
 * @ingroup LunarNavigationCore
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UI/Navigation/Types/LunarNavigationTypes.h"
#include "LunarNavigationScope.generated.h"

class ULocalPlayer;
class ULunarNavigableWidget;
class ULunarNavigationSubsystem;
class ULunarScreenWidget;

/**
 * @brief Non-visual runtime navigation graph and restoration state for one local player.
 * @ingroup LunarNavigationCore
 *
 * A scope stores its stable widget order, selection restoration data, and its
 * relationship to the surrounding scope stack. Stack mutation remains the
 * responsibility of ULunarNavigationSubsystem.
 */
UCLASS(BlueprintType)
class LUNAR_API ULunarNavigationScope : public UObject
{
	GENERATED_BODY()

public:
	/** @return Local player whose subsystem owns this scope, or nullptr before initialization. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation")
	ULocalPlayer* GetOwningLocalPlayer() const;

	/** @return Scope immediately below this one in the owning stack, or nullptr for the root scope. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation")
	ULunarNavigationScope* GetParentScope() const;

	/** @return Copy of the current widget registration snapshot in stable navigation order. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation")
	TArray<ULunarNavigableWidget*> GetRegisteredWidgets() const;

	/** @return Stable ID configured as the initial selection, or NAME_None when unset. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation")
	FName GetInitialSelectionId() const;

	/** @return Stable ID of the most recently selected widget, or NAME_None when unavailable. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation")
	FName GetLastSelectionId() const;

	/** @return True when this scope is the active top entry of its owning stack. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation")
	bool IsActive() const;

	/** @brief Schedules validation through the owning local-player subsystem. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation")
	void RequestValidation();

	/** Root visual widget whose descendants may participate in this scope. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|Navigation")
	TObjectPtr<UWidget> RootWidget;

	/** Immutable configuration copied when the runtime scope is initialized. */
	UPROPERTY(BlueprintReadOnly, Category = "Lunar|UI|Navigation")
	FLunarNavigationScopeSettings Settings;

private:
	/**
	 * @brief Initializes the scope before it is pushed onto the stack.
	 * @param InLocalPlayer Local player that owns the subsystem and scope.
	 * @param InRootWidget Visual root used for hierarchy membership checks.
	 * @param InSettings Configuration copied into the scope.
	 * @param InParentScope Scope immediately below this scope, if any.
	 */
	void InitializeScope(ULocalPlayer* InLocalPlayer, UWidget* InRootWidget, const FLunarNavigationScopeSettings& InSettings, ULunarNavigationScope* InParentScope);

	/** @brief Marks this scope as the active stack entry. */
	void ActivateScope();

	/** @brief Marks this scope as inactive without destroying its restoration state. */
	void DeactivateScope();

	/** @brief Adds a widget to the stable registration snapshot. @param Widget Widget to register. */
	void AddRegisteredWidget(ULunarNavigableWidget* Widget);

	/** @brief Removes a widget from the registration snapshot. @param Widget Widget to remove. */
	void RemoveRegisteredWidget(ULunarNavigableWidget* Widget);

	/** @brief Updates selection restoration state. @param Widget Newly selected widget, or nullptr to clear it. */
	void SetLastSelectedWidget(ULunarNavigableWidget* Widget);

	/** @return Most recently selected live widget, or nullptr when none remains. */
	ULunarNavigableWidget* GetLastSelectedWidget() const;

	/** Local player that owns this scope through its navigation subsystem. */
	UPROPERTY(Transient)
	TObjectPtr<ULocalPlayer> OwningLocalPlayer;

	/** Scope directly below this one in the owning stack. */
	UPROPERTY(Transient)
	TObjectPtr<ULunarNavigationScope> ParentScope;

	/** Participating widgets in deterministic registration order. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<ULunarNavigableWidget>> RegisteredWidgets;

	/** Weakly meaningful object reference used for last-selection restoration. */
	UPROPERTY(Transient)
	TObjectPtr<ULunarNavigableWidget> LastSelectedWidget;

	/** Initial stable selection ID resolved from Settings. */
	UPROPERTY(Transient)
	FName InitialSelectionId = NAME_None;

	/** Stable ID of the last successfully selected widget. */
	UPROPERTY(Transient)
	FName LastSelectionId = NAME_None;

	/** Whether this scope currently occupies the top of the stack. */
	UPROPERTY(Transient)
	bool bActive = false;

	/** Most recent diagnostics snapshot, retained for runtime debug presentation. */
	UPROPERTY(Transient)
	TArray<FLunarNavigationValidationMessage> LastValidationMessages;

	friend class ULunarNavigationSubsystem; ///< Owns stack mutation and graph state.
	friend class ULunarScreenWidget; ///< Initializes the scope for its visual screen.
};
