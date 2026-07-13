// Copyright 2026 Edgar Frolenkov All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "LunarFLGame.h"
#include "LunarFLNetworking.h"
#include "LunarFLMath.h"
#include "LunarFLRandom.h"
#include "LunarFLString.h"
#include "LunarFLChrono.h"
#include "LunarFLUI.h"
#include "LunarFLDebug.h"
#include "LunarFLAudio.h"
#include "LunarFLDataTable.h"
#include "LunarFLSave.h"
#include "LunarFLPhysics.h"
#include "LunarFLBlockout.h"
#include "LunarFLJSON.h"
#include "Subsystems/Performance/LunarFLPerformance.h"
#include "Subsystems/Console/LunarFLConsole.h"
#include "LunarFLTransform.h"
#include "LunarFLFile.h"

/**
 * @file LunarFL.h
 * @brief Aggregate header for Lunar function libraries
 */

 /**
  * @brief Namespace aliases for Lunar function libraries
  */
namespace LunarFL
{
	/** Game and platform helper library */
	using Game = ULunarFLGame;

	/** Networking helper library */
	using Networking = ULunarFLNetworking;

	/** Math helper library */
	using Math = ULunarFLMath;

	/** Random helper library */
	using Random = ULunarFLRandom;

	/** String helper library */
	using String = ULunarFLString;

	/** Time helper library */
	using Chrono = ULunarFLChrono;

	/** User interface helper library */
	using UI = ULunarFLUI;

	/** Debug helper library */
	using Debug = ULunarFLDebug;

	/** Audio helper library */
	using Audio = ULunarFLAudio;

	/** Data table helper library */
	using DataTable = ULunarFLDataTable;

	/** Save helper library */
	using Save = ULunarFLSave;

	/** Physics helper library */
	using Physics = ULunarFLPhysics;

	/** Blockout helper library */
	using Blockout = ULunarFLBlockout;

	/** JSON helper library */
	using JSON = ULunarFLJSON;

	/** Performance helper library */
	using Performance = ULunarFLPerformance;

	/** Console helper library */
	using Console = ULunarFLConsole;

	/** Transform helper library */
	using Transform = ULunarFLTransform;

	/** File helper library */
	using File = ULunarFLFile;
}
