// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "UI/Navigation/Data/LunarUIActionRegistry.h"

/**
 * @file LunarUIActionRegistry.cpp
 * @brief Implements semantic UI action lookup and registry validation.
 * @ingroup LunarNavigationData
 */

#define LOCTEXT_NAMESPACE "LunarUIActionRegistry"

namespace LunarUIActionRegistry_Private
{
	/**
	 * @brief Builds the validation owner path for an action definition.
	 * @param Registry Registry that owns the definition.
	 * @param DefinitionIndex Index of the definition in the registry.
	 * @return Stable property path used by validation diagnostics.
	 */
	static FString MakeDefinitionOwnerPath(const ULunarUIActionRegistry& Registry, const int32 DefinitionIndex)
	{
		return FString::Printf(
			TEXT("%s.ActionDefinitions[%d]"),
			*Registry.GetPathName(),
			DefinitionIndex);
	}

	/**
	 * @brief Builds the validation owner path for a physical action binding.
	 * @param Registry Registry that owns the binding.
	 * @param DefinitionIndex Index of the action definition.
	 * @param BindingIndex Index of the binding within the definition.
	 * @return Stable property path used by validation diagnostics.
	 */
	static FString MakeBindingOwnerPath(
		const ULunarUIActionRegistry& Registry,
		const int32 DefinitionIndex,
		const int32 BindingIndex)
	{
		return FString::Printf(
			TEXT("%s.ActionDefinitions[%d].Bindings[%d]"),
			*Registry.GetPathName(),
			DefinitionIndex,
			BindingIndex);
	}

	/**
	 * @brief Appends one actionable validation error.
	 * @param Messages Destination message array.
	 * @param Code Stable machine-readable error code.
	 * @param MessageText Localized diagnostic text.
	 * @param OwnerPath Property path associated with the error.
	 */
	static void AddError(
		TArray<FLunarNavigationValidationMessage>& Messages,
		const FName Code,
		const FText& MessageText,
		FString OwnerPath)
	{
		FLunarNavigationValidationMessage& Message = Messages.AddDefaulted_GetRef();
		Message.Verbosity = ELunarConsoleMessageVerbosity::Error;
		Message.Code = Code;
		Message.Message = MessageText;
		Message.OwnerPath = MoveTemp(OwnerPath);
	}
}

bool ULunarUIActionRegistry::FindActionDefinition(
	const FGameplayTag ActionTag,
	FLunarUIActionDefinition& OutDefinition) const
{
	OutDefinition = FLunarUIActionDefinition();
	if (!ActionTag.IsValid())
	{
		return false;
	}

	const FLunarUIActionDefinition* ResolvedDefinition = nullptr;
	for (const FLunarUIActionDefinition& Definition : ActionDefinitions)
	{
		if (Definition.ActionTag != ActionTag)
		{
			continue;
		}

		if (ResolvedDefinition)
		{
			return false;
		}

		ResolvedDefinition = &Definition;
	}

	if (!ResolvedDefinition)
	{
		return false;
	}

	OutDefinition = *ResolvedDefinition;
	return true;
}

TArray<FLunarNavigationValidationMessage> ULunarUIActionRegistry::ValidateRegistry() const
{
	using namespace LunarUIActionRegistry_Private;

	TArray<FLunarNavigationValidationMessage> Messages;
	TMap<FGameplayTag, int32> ActionTagCounts;
	for (const FLunarUIActionDefinition& Definition : ActionDefinitions)
	{
		if (Definition.ActionTag.IsValid())
		{
			++ActionTagCounts.FindOrAdd(Definition.ActionTag);
		}
	}

	for (int32 DefinitionIndex = 0; DefinitionIndex < ActionDefinitions.Num(); ++DefinitionIndex)
	{
		const FLunarUIActionDefinition& Definition = ActionDefinitions[DefinitionIndex];
		if (!Definition.ActionTag.IsValid())
		{
			AddError(
				Messages,
				TEXT("InvalidActionTag"),
				FText::Format(
					LOCTEXT(
						"InvalidActionTagMessage",
						"Action definition at index {0} has an invalid ActionTag. Assign a valid semantic UI action tag or remove the definition."),
					FText::AsNumber(DefinitionIndex)),
				MakeDefinitionOwnerPath(*this, DefinitionIndex));
		}
		else if (const int32* DuplicateCount = ActionTagCounts.Find(Definition.ActionTag);
			DuplicateCount && *DuplicateCount > 1)
		{
			AddError(
				Messages,
				TEXT("DuplicateActionTag"),
				FText::Format(
					LOCTEXT(
						"DuplicateActionTagMessage",
						"Action definition at index {0} uses ActionTag '{1}', which appears {2} times in this registry. Keep exactly one definition for this tag."),
					FText::AsNumber(DefinitionIndex),
					FText::FromName(Definition.ActionTag.GetTagName()),
					FText::AsNumber(*DuplicateCount)),
				MakeDefinitionOwnerPath(*this, DefinitionIndex));
		}

		TMap<FKey, int32> EnabledBindingCounts;
		for (const FLunarUIActionBinding& Binding : Definition.Bindings)
		{
			if (Binding.bEnabled && Binding.Key.IsValid())
			{
				++EnabledBindingCounts.FindOrAdd(Binding.Key);
			}
		}

		for (int32 BindingIndex = 0; BindingIndex < Definition.Bindings.Num(); ++BindingIndex)
		{
			const FLunarUIActionBinding& Binding = Definition.Bindings[BindingIndex];
			if (!Binding.bEnabled)
			{
				continue;
			}

			if (!Binding.Key.IsValid())
			{
				AddError(
					Messages,
					TEXT("InvalidActionBindingKey"),
					FText::Format(
						LOCTEXT(
							"InvalidActionBindingKeyMessage",
							"Enabled binding at action index {0}, binding index {1} has an invalid key. Assign a valid key or disable the binding."),
						FText::AsNumber(DefinitionIndex),
						FText::AsNumber(BindingIndex)),
					MakeBindingOwnerPath(*this, DefinitionIndex, BindingIndex));
				continue;
			}

			const int32 DuplicateCount = EnabledBindingCounts.FindRef(Binding.Key);
			if (DuplicateCount > 1)
			{
				AddError(
					Messages,
					TEXT("DuplicateActionBindingKey"),
					FText::Format(
						LOCTEXT(
							"DuplicateActionBindingKeyMessage",
							"Enabled key '{0}' appears {1} times in action definition {2}. Keep exactly one enabled binding for this key."),
						Binding.Key.GetDisplayName(),
						FText::AsNumber(DuplicateCount),
						FText::AsNumber(DefinitionIndex)),
					MakeBindingOwnerPath(*this, DefinitionIndex, BindingIndex));
			}
		}
	}

	return Messages;
}

#undef LOCTEXT_NAMESPACE
