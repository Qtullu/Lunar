// Copyright 2026 Edgar Frolenkov All rights reserved.

#include "Settings/LunarSettings.h"

ULunarSettings* ULunarSettings::GetLunarSettings()
{
	return GetMutableDefault<ULunarSettings>();
}

FLinearColor ULunarSettings::GetVerbosityColor(ELunarConsoleMessageVerbosity Verbosity)
{
	const FLunarConsoleSettings& ConsoleSettings = GetDefault<ULunarSettings>()->Console;

	switch (Verbosity)
	{
	case ELunarConsoleMessageVerbosity::Message:
		return ConsoleSettings.MessageColor;

	case ELunarConsoleMessageVerbosity::Success:
		return ConsoleSettings.SuccessColor;

	case ELunarConsoleMessageVerbosity::Info:
		return ConsoleSettings.InfoColor;

	case ELunarConsoleMessageVerbosity::Warning:
		return ConsoleSettings.WarningColor;

	case ELunarConsoleMessageVerbosity::Error:
		return ConsoleSettings.ErrorColor;

	case ELunarConsoleMessageVerbosity::Fatal:
		return ConsoleSettings.FatalColor;

	case ELunarConsoleMessageVerbosity::Debug:
		return ConsoleSettings.DebugColor;

	case ELunarConsoleMessageVerbosity::Trace:
		return ConsoleSettings.TraceColor;

	default:
		return ConsoleSettings.MessageColor;
	}
}
