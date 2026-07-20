// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarScreenWidget.h
 * @brief Declares a top-level widget with an automatically managed navigation scope.
 * @ingroup LunarNavigationCore
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Navigation/Types/LunarNavigationTypes.h"
#include "LunarScreenWidget.generated.h"

class ULunarNavigationScope;
class ULunarNavigationSubsystem;
class ULunarScreenWidget;

/** @brief Broadcast when a Lunar screen finishes opening or closing its scope, including its source object. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLunarScreenEventSignature, ULunarScreenWidget*, ScreenWidget);

/**
 * @brief Top-level Lunar screen that owns one automatically managed navigation scope.
 * @ingroup LunarNavigationCore
 *
 * The widget creates its runtime scope on demand and coordinates its lifetime
 * with the owning ULunarNavigationSubsystem.
 */
UCLASS(Abstract, Blueprintable)
class LUNAR_API ULunarScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** @brief Creates a screen widget with navigation focus disabled at the UWidget layer. @param ObjectInitializer Unreal object initializer. */
	ULunarScreenWidget(const FObjectInitializer& ObjectInitializer);

	/** @brief Opens this screen's navigation scope. @return True when the screen is open after the request. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation")
	bool OpenScreen();

	/** @brief Closes this screen's scope and any child scopes above it. @return True when the close request succeeded. */
	UFUNCTION(BlueprintCallable, Category = "Lunar|UI|Navigation")
	bool CloseScreen();

	/** @return True when this screen currently owns a scope in its local-player stack. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation")
	bool IsScreenOpen() const;

	/** @return Non-visual scope owned by this screen, or nullptr while closed. */
	UFUNCTION(BlueprintPure, Category = "Lunar|UI|Navigation")
	ULunarNavigationScope* GetNavigationScope() const;

	/** Broadcast after the screen scope has been pushed successfully. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation")
	FLunarScreenEventSignature OnScreenOpened;

	/** Broadcast after the screen scope has been removed from the stack. */
	UPROPERTY(BlueprintAssignable, Category = "Lunar|UI|Navigation")
	FLunarScreenEventSignature OnScreenClosed;

	/** Settings copied into the runtime scope when the screen opens. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation")
	FLunarNavigationScopeSettings NavigationScopeSettings;

	/** Automatically opens the scope during NativeConstruct. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lunar|UI|Navigation")
	bool bAutoActivateNavigationScope = true;

	/** Runtime scope owned by this screen. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Lunar|UI|Navigation")
	TObjectPtr<ULunarNavigationScope> NavigationScope;

protected:
	/** @brief Reapplies the non-focusable screen policy after property synchronization. */
	virtual void SynchronizeProperties() override;

	/** @brief Optionally opens the navigation scope after native construction. */
	virtual void NativeConstruct() override;

	/** @brief Closes the scope before native destruction completes. */
	virtual void NativeDestruct() override;

private:
	/** @return Navigation subsystem for the owning local player, or nullptr when unavailable. */
	ULunarNavigationSubsystem* ResolveNavigationSubsystem() const;

	/** @brief Finalizes local state after the subsystem removes this screen's scope. */
	void HandleNavigationScopePopped();

	/** Whether the screen currently has an owned scope in the stack. */
	UPROPERTY(Transient)
	bool bScreenOpen = false;

	/** Reentrancy guard used while OpenScreen is creating and pushing a scope. */
	UPROPERTY(Transient)
	bool bOpeningScreen = false;

	/** Deferred close request received while OpenScreen is still in progress. */
	UPROPERTY(Transient)
	bool bCloseRequestedWhileOpening = false;

	friend class ULunarNavigationSubsystem; ///< Notifies the screen when its scope is popped externally.
};
