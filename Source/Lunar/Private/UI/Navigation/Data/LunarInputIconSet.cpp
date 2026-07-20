// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "UI/Navigation/Data/LunarInputIconSet.h"

/**
 * @file LunarInputIconSet.cpp
 * @brief Implements device-specific input-icon lookup.
 * @ingroup LunarNavigationData
 */

bool ULunarInputIconSet::ResolveIconForKey(const FKey InputKey, FSlateBrush& OutIcon) const
{
	OutIcon = FSlateBrush();
	if (!InputKey.IsValid())
	{
		return false;
	}

	const FLunarInputIconEntry* ResolvedEntry = nullptr;
	for (const FLunarInputIconEntry& Entry : IconEntries)
	{
		if (Entry.InputKey != InputKey)
		{
			continue;
		}

		if (ResolvedEntry)
		{
			return false;
		}

		ResolvedEntry = &Entry;
	}

	if (!ResolvedEntry
		|| (!ResolvedEntry->Icon.GetResourceObject()
			&& ResolvedEntry->Icon.GetResourceName().IsNone()))
	{
		return false;
	}

	OutIcon = ResolvedEntry->Icon;
	return true;
}
