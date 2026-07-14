// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "UI/Navigation/Controls/LunarRadioGroup.h"

#include "UI/Navigation/Controls/LunarRadio.h"

/**
 * @file LunarRadioGroup.cpp
 * @brief Non-visual Lunar radio-group registration and selection invariants
 * @ingroup LunarNavigationControls
 */

/** Private diagnostic category for invalid radio-group membership and ID configuration. */
DEFINE_LOG_CATEGORY_STATIC(LogLunarRadioGroup, Log, All);

bool ULunarRadioGroup::SetSelectedRadio(ULunarRadio* Radio)
{
	CompactRegisteredRadios();
	if (!Radio)
	{
		return ClearSelectedRadio();
	}
	if (Radio->RadioGroup != this)
	{
		UE_LOG(
			LogLunarRadioGroup,
			Warning,
			TEXT("%s cannot select %s because the Radio is assigned to another group."),
			*GetPathName(),
			*GetPathNameSafe(Radio));
		return false;
	}
	if (Radio->RadioId.IsNone())
	{
		UE_LOG(
			LogLunarRadioGroup,
			Warning,
			TEXT("%s cannot register or select %s because RadioId is empty."),
			*GetPathName(),
			*GetPathNameSafe(Radio));
		return false;
	}
	if (!IsRegisteredRadio(Radio) && !RegisterRadio(Radio))
	{
		return false;
	}
	if (!IsRadioIdAvailable(Radio))
	{
		UE_LOG(
			LogLunarRadioGroup,
			Warning,
			TEXT("%s cannot select %s because RadioId '%s' is duplicated."),
			*GetPathName(),
			*GetPathNameSafe(Radio),
			*Radio->RadioId.ToString());
		return false;
	}
	PendingSelectedRadioId = NAME_None;
	return ApplySelection(Radio, false);
}

bool ULunarRadioGroup::SetSelectedRadioById(const FName RadioId)
{
	CompactRegisteredRadios();
	if (RadioId.IsNone())
	{
		UE_LOG(LogLunarRadioGroup, Warning, TEXT("%s cannot select an empty RadioId."), *GetPathName());
		return false;
	}

	ULunarRadio* Radio = FindRegisteredRadioById(RadioId);
	if (!Radio)
	{
		UE_LOG(
			LogLunarRadioGroup,
			Warning,
			TEXT("%s has no registered Radio with ID '%s'."),
			*GetPathName(),
			*RadioId.ToString());
		return false;
	}
	PendingSelectedRadioId = NAME_None;
	return ApplySelection(Radio, false);
}

ULunarRadio* ULunarRadioGroup::GetSelectedRadio() const
{
	ULunarRadio* Radio = SelectedRadio.Get();
	return IsValid(Radio)
		&& Radio->RadioGroup == this
		&& Radio->RadioId == SelectedRadioId
		&& IsRegisteredRadio(Radio)
		? Radio
		: nullptr;
}

FName ULunarRadioGroup::GetSelectedRadioId() const
{
	if (const ULunarRadio* Radio = GetSelectedRadio())
	{
		return Radio->RadioId;
	}
	return SelectedRadioId;
}

bool ULunarRadioGroup::ClearSelectedRadio()
{
	CompactRegisteredRadios();
	if (bRequireSelection && !RegisteredRadios.IsEmpty())
	{
		return false;
	}
	PendingSelectedRadioId = NAME_None;
	return ApplySelection(nullptr, true);
}

bool ULunarRadioGroup::RegisterRadio(ULunarRadio* Radio)
{
	CompactRegisteredRadios();
	if (!IsValid(Radio) || Radio->RadioGroup != this || Radio->RadioId.IsNone())
	{
		if (IsValid(Radio))
		{
			UE_LOG(
				LogLunarRadioGroup,
				Warning,
				TEXT("%s rejected Radio registration for %s: the group must match and RadioId must be non-empty."),
				*GetPathName(),
				*GetPathNameSafe(Radio));
			const bool bPreviousChecked = Radio->bIsChecked;
			if (Radio->ApplyCheckedStateSilently(false))
			{
				Radio->BroadcastCheckedStateChanged(bPreviousChecked);
			}
		}
		return false;
	}

	if (!IsRadioIdAvailable(Radio))
	{
		UE_LOG(
			LogLunarRadioGroup,
			Warning,
			TEXT("%s rejected %s because RadioId '%s' is already registered."),
			*GetPathName(),
			*GetPathNameSafe(Radio),
			*Radio->RadioId.ToString());
		const bool bPreviousChecked = Radio->bIsChecked;
		if (Radio->ApplyCheckedStateSilently(false))
		{
			Radio->BroadcastCheckedStateChanged(bPreviousChecked);
		}
		return false;
	}

	const bool bWasRegistered = IsRegisteredRadio(Radio);
	if (!bWasRegistered)
	{
		RegisteredRadios.Add(Radio);
	}

	ULunarRadio* CurrentSelection = SelectedRadio.Get();
	if (!CurrentSelection
		&& PendingSelectedRadioId.IsNone()
		&& !bFinalizeEmptySelectionAfterApply
		&& !SelectedRadioId.IsNone()
		&& Radio->RadioId != SelectedRadioId)
	{
		// Construction order is not semantic. Keep the configured target while a required
		// group uses a temporary fallback until that Radio registers.
		PendingSelectedRadioId = SelectedRadioId;
	}
	const FName DesiredRadioId = !PendingSelectedRadioId.IsNone()
		? PendingSelectedRadioId
		: SelectedRadioId;
	if (bApplyingSelection)
	{
		const bool bWouldBecomeSelection =
			(!DesiredRadioId.IsNone() && Radio->RadioId == DesiredRadioId)
			|| (Radio->bIsChecked && CurrentSelection != Radio)
			|| (!CurrentSelection && bRequireSelection);
		if (bWouldBecomeSelection)
		{
			DeferredSelectionAfterApply = Radio;
			bNormalizeSelectionAfterApply = true;
		}

		// Registration is complete even though selection mutation must wait until the
		// current delegate chain unwinds. Keep the one-checked invariant meanwhile.
		if (CurrentSelection != Radio)
		{
			const bool bPreviousChecked = Radio->bIsChecked;
			if (Radio->ApplyCheckedStateSilently(false))
			{
				Radio->BroadcastCheckedStateChanged(bPreviousChecked);
			}
		}
		return true;
	}
	if (!DesiredRadioId.IsNone() && Radio->RadioId == DesiredRadioId && CurrentSelection != Radio)
	{
		PendingSelectedRadioId = NAME_None;
		return ApplySelection(Radio, false);
	}
	if (Radio->bIsChecked && CurrentSelection != Radio)
	{
		return ApplySelection(Radio, false);
	}
	if (CurrentSelection)
	{
		if (Radio == CurrentSelection)
		{
			return ApplySelection(CurrentSelection, false);
		}
		const bool bPreviousChecked = Radio->bIsChecked;
		if (Radio->ApplyCheckedStateSilently(false))
		{
			Radio->BroadcastCheckedStateChanged(bPreviousChecked);
		}
		return true;
	}
	if (bRequireSelection)
	{
		return ApplySelection(Radio, false);
	}

	return true;
}

void ULunarRadioGroup::UnregisterRadio(ULunarRadio* Radio)
{
	if (!Radio)
	{
		return;
	}
	CompactRegisteredRadios();
	if (!IsRegisteredRadio(Radio))
	{
		return;
	}

	const bool bWasSelected = SelectedRadio.Get() == Radio;
	if (bWasSelected)
	{
		ULunarRadio* Replacement = nullptr;
		if (bRequireSelection)
		{
			for (const TWeakObjectPtr<ULunarRadio>& CandidatePtr : RegisteredRadios)
			{
				ULunarRadio* Candidate = CandidatePtr.Get();
				if (Candidate && Candidate != Radio)
				{
					Replacement = Candidate;
					break;
				}
			}
		}
		if (!ApplySelection(Replacement, true))
		{
			const bool bPreviousChecked = Radio->bIsChecked;
			if (Radio->ApplyCheckedStateSilently(false))
			{
				Radio->BroadcastCheckedStateChanged(bPreviousChecked);
			}
		}
	}
	else
	{
		const bool bPreviousChecked = Radio->bIsChecked;
		if (Radio->ApplyCheckedStateSilently(false))
		{
			Radio->BroadcastCheckedStateChanged(bPreviousChecked);
		}
	}

	RegisteredRadios.RemoveAll([Radio](const TWeakObjectPtr<ULunarRadio>& Candidate)
	{
		return Candidate.Get() == Radio;
	});
	if (SelectedRadioId == Radio->RadioId && SelectedRadio.Get() != Radio)
	{
		SelectedRadioId = SelectedRadio.IsValid() ? SelectedRadio->RadioId : NAME_None;
	}
	if (bWasSelected && SelectedRadio.Get() == Radio)
	{
		SelectedRadio.Reset();
		SelectedRadioId = bApplyingSelection
			? Radio->RadioId
			: NAME_None;
		bFinalizeEmptySelectionAfterApply = bApplyingSelection;
		NormalizeRequiredSelection();
	}
}

bool ULunarRadioGroup::ApplySelection(ULunarRadio* NewSelection, const bool bAllowRequiredEmpty)
{
	CompactRegisteredRadios();
	if (bApplyingSelection)
	{
		UE_LOG(LogLunarRadioGroup, Warning, TEXT("%s ignored a re-entrant selection change."), *GetPathName());
		return false;
	}
	if (NewSelection
		&& (!IsRegisteredRadio(NewSelection)
			|| NewSelection->RadioGroup != this
			|| NewSelection->RadioId.IsNone()
			|| !IsRadioIdAvailable(NewSelection)))
	{
		return false;
	}
	if (!NewSelection && bRequireSelection && !bAllowRequiredEmpty && !RegisteredRadios.IsEmpty())
	{
		return false;
	}

	{
		TGuardValue<bool> ApplyingSelectionGuard(bApplyingSelection, true);
		const FName PreviousRadioId = SelectedRadio.IsValid() ? SelectedRadio->RadioId : SelectedRadioId;
		const FName NewRadioId = NewSelection ? NewSelection->RadioId : NAME_None;

		struct FChangedRadio
		{
			TWeakObjectPtr<ULunarRadio> Radio;
			bool bPreviousChecked = false;
		};
		TArray<FChangedRadio> ChangedRadios;
		ChangedRadios.Reserve(RegisteredRadios.Num());
		for (const TWeakObjectPtr<ULunarRadio>& RadioPtr : RegisteredRadios)
		{
			if (ULunarRadio* RegisteredRadio = RadioPtr.Get())
			{
				const bool bPreviousChecked = RegisteredRadio->bIsChecked;
				if (RegisteredRadio->ApplyCheckedStateSilently(RegisteredRadio == NewSelection))
				{
					ChangedRadios.Add({RegisteredRadio, bPreviousChecked});
				}
			}
		}

		SelectedRadio = NewSelection;
		SelectedRadioId = NewRadioId;
		for (const FChangedRadio& Change : ChangedRadios)
		{
			if (ULunarRadio* ChangedRadio = Change.Radio.Get())
			{
				ChangedRadio->BroadcastCheckedStateChanged(Change.bPreviousChecked);
			}
		}
		if (PreviousRadioId != NewRadioId)
		{
			OnSelectionChanged.Broadcast(PreviousRadioId, NewRadioId);
		}
	}

	if (ULunarRadio* DeferredSelection = DeferredSelectionAfterApply.Get())
	{
		DeferredSelectionAfterApply.Reset();
		if (DeferredSelection->RadioGroup == this
			&& IsRegisteredRadio(DeferredSelection)
			&& IsRadioIdAvailable(DeferredSelection))
		{
			if (PendingSelectedRadioId == DeferredSelection->RadioId)
			{
				PendingSelectedRadioId = NAME_None;
			}
			ApplySelection(DeferredSelection, false);
		}
	}
	if (bNormalizeSelectionAfterApply)
	{
		bNormalizeSelectionAfterApply = false;
		NormalizeRequiredSelection();
	}
	if (bFinalizeEmptySelectionAfterApply)
	{
		bFinalizeEmptySelectionAfterApply = false;
		if (!GetSelectedRadio())
		{
			// Complete the re-entrant selected -> empty transition only after the
			// outer notification has published its selected endpoint.
			ApplySelection(nullptr, true);
		}
	}
	return true;
}

bool ULunarRadioGroup::IsRegisteredRadio(const ULunarRadio* Radio) const
{
	return Radio && RegisteredRadios.ContainsByPredicate([Radio](const TWeakObjectPtr<ULunarRadio>& Candidate)
	{
		return Candidate.Get() == Radio;
	});
}

bool ULunarRadioGroup::IsRadioIdAvailable(const ULunarRadio* Radio) const
{
	if (!Radio || Radio->RadioId.IsNone())
	{
		return false;
	}
	return !RegisteredRadios.ContainsByPredicate([Radio](const TWeakObjectPtr<ULunarRadio>& Candidate)
	{
		const ULunarRadio* OtherRadio = Candidate.Get();
		return OtherRadio && OtherRadio != Radio && OtherRadio->RadioId == Radio->RadioId;
	});
}

void ULunarRadioGroup::CompactRegisteredRadios()
{
	const FName PreviousRadioId = SelectedRadio.IsValid() ? SelectedRadio->RadioId : SelectedRadioId;
	bool bLostSelection = false;
	RegisteredRadios.RemoveAll([](const TWeakObjectPtr<ULunarRadio>& Candidate)
	{
		return !Candidate.IsValid();
	});

	if (SelectedRadio.IsStale())
	{
		SelectedRadio.Reset();
		SelectedRadioId = NAME_None;
		bLostSelection = true;
	}
	else if (ULunarRadio* Radio = SelectedRadio.Get(); Radio && !IsRegisteredRadio(Radio))
	{
		SelectedRadio.Reset();
		SelectedRadioId = NAME_None;
		bLostSelection = true;
	}

	if (bLostSelection && bRequireSelection && !bApplyingSelection && !RegisteredRadios.IsEmpty())
	{
		if (ULunarRadio* Fallback = RegisteredRadios[0].Get())
		{
			SelectedRadioId = PreviousRadioId;
			ApplySelection(Fallback, false);
		}
	}
	else if (bLostSelection && bApplyingSelection)
	{
		bNormalizeSelectionAfterApply = true;
	}
}

void ULunarRadioGroup::NormalizeRequiredSelection()
{
	if (bApplyingSelection)
	{
		bNormalizeSelectionAfterApply = true;
		return;
	}
	CompactRegisteredRadios();
	if (ULunarRadio* CurrentSelection = SelectedRadio.Get())
	{
		ApplySelection(CurrentSelection, false);
		return;
	}
	if (!SelectedRadioId.IsNone())
	{
		if (ULunarRadio* ConfiguredSelection = FindRegisteredRadioById(SelectedRadioId))
		{
			ApplySelection(ConfiguredSelection, false);
			return;
		}
	}
	if (bRequireSelection && !RegisteredRadios.IsEmpty())
	{
		ApplySelection(RegisteredRadios[0].Get(), false);
	}
}

ULunarRadio* ULunarRadioGroup::FindRegisteredRadioById(const FName RadioId) const
{
	if (RadioId.IsNone())
	{
		return nullptr;
	}
	for (const TWeakObjectPtr<ULunarRadio>& RadioPtr : RegisteredRadios)
	{
		ULunarRadio* Radio = RadioPtr.Get();
		if (Radio && Radio->RadioId == RadioId)
		{
			return Radio;
		}
	}
	return nullptr;
}
