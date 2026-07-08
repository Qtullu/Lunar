# Lunar Plugin

**Unreal Engine 5.8 development plugin**  
by **Edgar Frolenkov**

## Documentation

Full generated documentation is available here:

[Online Documentation](https://qtullu.github.io/Lunar/)

---

## Overview

**Lunar Plugin** is a development acceleration plugin for Unreal Engine 5.8

It provides a growing set of runtime subsystems Blueprint function libraries shared types widgets settings and utility tools designed to reduce repetitive project code and make common game development tasks faster to implement

The plugin is intended as a reusable foundation for Unreal Engine projects and includes systems and helpers for console tooling performance monitoring raw input tracking file operations platform queries transform manipulation save helpers UI utilities audio utilities debugging math randomization data tables JSON physics blockout workflows materials actors sound design and other production utilities

---

## Documentation sections

- [Function Libraries](https://qtullu.github.io/Lunar/group___lunar_function_libraries.html)
- [Subsystems](https://qtullu.github.io/Lunar/group___lunar_subsystems.html)
- [Settings](https://qtullu.github.io/Lunar/group___lunar_settings.html)
- [Types](https://qtullu.github.io/Lunar/group___lunar_types.html)
- [Widgets](https://qtullu.github.io/Lunar/group___lunar_widgets.html)

---

## Subsystems

Subsystems are persistent runtime services used by the plugin and exposed to Blueprint where needed

### Console

[Console Subsystem](https://qtullu.github.io/Lunar/group___lunar_console_subsystem.html) stores console messages executes Lunar console commands manages command suggestions controls the console widget and broadcasts console events

### Performance

[Performance Subsystem](https://qtullu.github.io/Lunar/group___lunar_performance_subsystem.html) collects runtime performance snapshots stores performance history tracks FPS frame time memory GPU CPU disk metrics and controls the performance widget

### Raw Input

[Raw Input Subsystem](https://qtullu.github.io/Lunar/group___lunar_raw_input_subsystem.html) tracks raw input state detects the last active input device stores key mouse and wheel input snapshots and broadcasts raw input events

---

## Function libraries

Function libraries expose small focused Blueprint utility functions

### Game

[Game](https://qtullu.github.io/Lunar/group___lunar_f_l_game.html) provides build configuration runtime environment platform project and clipboard helpers

### Networking

[Networking](https://qtullu.github.io/Lunar/group___lunar_f_l_networking.html) provides networking helper functions

### Math

[Math](https://qtullu.github.io/Lunar/group___lunar_f_l_math.html) provides math helper functions

### Random

[Random](https://qtullu.github.io/Lunar/group___lunar_f_l_random.html) provides random value helper functions

### String

[String](https://qtullu.github.io/Lunar/group___lunar_f_l_string.html) provides string formatting helper functions

### Chrono

[Chrono](https://qtullu.github.io/Lunar/group___lunar_f_l_chrono.html) provides time and chrono helper functions

### UI

[UI](https://qtullu.github.io/Lunar/group___lunar_f_l_u_i.html) provides user interface helper functions

### Debug

[Debug](https://qtullu.github.io/Lunar/group___lunar_f_l_debug.html) provides debug helper functions

### Audio

[Audio](https://qtullu.github.io/Lunar/group___lunar_f_l_audio.html) provides audio helper functions

### Data Table

[Data Table](https://qtullu.github.io/Lunar/group___lunar_f_l_data_table.html) provides data table helper functions

### Save

[Save](https://qtullu.github.io/Lunar/group___lunar_f_l_save.html) provides save helper functions

### Physics

[Physics](https://qtullu.github.io/Lunar/group___lunar_f_l_physics.html) provides physics helper functions

### Blockout

[Blockout](https://qtullu.github.io/Lunar/group___lunar_f_l_blockout.html) provides blockout workflow helper functions

### JSON

[JSON](https://qtullu.github.io/Lunar/group___lunar_f_l_j_s_o_n.html) provides JSON helper functions

### Performance

[Performance](https://qtullu.github.io/Lunar/group___lunar_f_l_performance.html) provides runtime performance metric snapshot summary history and graph painting helpers

### Console

[Console](https://qtullu.github.io/Lunar/group___lunar_f_l_console.html) provides Lunar console print command execution command suggestion and log export helpers

### Transform

[Transform](https://qtullu.github.io/Lunar/group___lunar_f_l_transform.html) provides actor and scene component transform axis helpers

### File

[File](https://qtullu.github.io/Lunar/group___lunar_f_l_file.html) provides native folder selection and text file saving helpers

---

## Types

Types contain shared enums structs settings records snapshots and data definitions used by the plugin

### Raw Input

[Raw Input Types](https://qtullu.github.io/Lunar/group___lunar_types_raw_input.html) contains raw input device and input snapshot types

### Console

[Console Types](https://qtullu.github.io/Lunar/group___lunar_types_console.html) contains console message command table command definition command parameter and console settings types

### Game

[Game Types](https://qtullu.github.io/Lunar/group___lunar_types_game.html) contains platform type and platform family types

### Performance

[Performance Types](https://qtullu.github.io/Lunar/group___lunar_types_performance.html) contains performance memory units snapshots histories metric structures and performance settings

### Transform

[Transform Types](https://qtullu.github.io/Lunar/group___lunar_types_transform.html) contains transform axis and transform data type enums

### UI

[UI Types](https://qtullu.github.io/Lunar/group___lunar_types_u_i.html) contains user interface shared types

---

## Settings

[Plugin Settings](https://qtullu.github.io/Lunar/group___lunar_settings.html) stores project level Lunar settings exposed through Unreal Developer Settings

The settings currently configure Lunar console defaults and Lunar performance monitoring defaults

---

## Widgets

[Draggable Window](https://qtullu.github.io/Lunar/group___lunar_widgets.html) is a reusable draggable and resizable UMG window widget with bounds resize modes handle states and movement events

---

## Public headers

The plugin exposes aggregate headers for faster access to public API groups

### Lunar.h

Main public plugin include

### LunarFL.h

Aggregate include for all Lunar function libraries

```cpp
#include "LunarFL.h"

FString PlatformName = LunarFL::Game::GetPlatformName();
FString PercentText = LunarFL::String::FormatPercent01(0.75f);
```

### LunarTypes.h

Aggregate include for shared Lunar type headers

```cpp
#include "LunarTypes.h"

ELunarAxisFull Axis = ELunarAxisFull::XYZ;
FLunarRawInputSnapshot InputSnapshot;
ELunarInputDeviceType InputDevice = InputSnapshot.LastInputDevice;
```

---

## C++ usage example

```cpp
#include "LunarFL.h"

FString ProjectName = LunarFL::Game::GetProjectName();

bool bSaved = LunarFL::File::SaveTextFileAs(
    WorldContextObject,
    DirectoryPath,
    FileNameWithExtension,
    Text,
    OutSavedFilePath,
    OutError
);
```

---

## Development target

- Engine: Unreal Engine 5.8
- Plugin: Lunar
- Author: Edgar Frolenkov
- Documentation: Doxygen Graphviz Doxygen Awesome CSS