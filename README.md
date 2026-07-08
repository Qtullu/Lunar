\# Lunar Plugin



\*\*Unreal Engine 5.8 development plugin\*\*  

by \*\*Edgar Frolenkov\*\*



\## Documentation



Full generated documentation is available here:



\*\*\[Online Documentation](https://qtullu.github.io/Lunar/)\*\*



\---



\## Overview



\*\*Lunar Plugin\*\* is a development acceleration plugin for Unreal Engine 5.8



It provides a growing set of runtime subsystems Blueprint function libraries shared types widgets settings and utility tools designed to reduce repetitive project code and make common game development tasks faster to implement



The plugin is intended as a reusable foundation for Unreal Engine projects and includes systems and helpers for console tooling performance monitoring raw input tracking file operations platform queries transform manipulation save helpers UI utilities audio utilities debugging math randomization data tables JSON physics blockout workflows materials actors sound design and other production utilities



\---



\## Documentation sections



\- \[Function Libraries](https://qtullu.github.io/Lunar/group\_\_\_lunar\_function\_libraries.html)

\- \[Subsystems](https://qtullu.github.io/Lunar/group\_\_\_lunar\_subsystems.html)

\- \[Settings](https://qtullu.github.io/Lunar/group\_\_\_lunar\_settings.html)

\- \[Types](https://qtullu.github.io/Lunar/group\_\_\_lunar\_types.html)

\- \[Widgets](https://qtullu.github.io/Lunar/group\_\_\_lunar\_widgets.html)



\---



\## Subsystems



Subsystems are persistent runtime services used by the plugin and exposed to Blueprint where needed



\### Console



\[Console Subsystem](https://qtullu.github.io/Lunar/group\_\_\_lunar\_console\_subsystem.html) stores console messages executes Lunar console commands manages command suggestions controls the console widget and broadcasts console events



\### Performance



\[Performance Subsystem](https://qtullu.github.io/Lunar/group\_\_\_lunar\_performance\_subsystem.html) collects runtime performance snapshots stores performance history tracks FPS frame time memory GPU CPU disk metrics and controls the performance widget



\### Raw Input



\[Raw Input Subsystem](https://qtullu.github.io/Lunar/group\_\_\_lunar\_raw\_input\_subsystem.html) tracks raw input state detects the last active input device stores key mouse and wheel input snapshots and broadcasts raw input events



\---



\## Function libraries



Function libraries expose small focused Blueprint utility functions



\### Game



\[Game](https://qtullu.github.io/Lunar/group\_\_\_lunar\_f\_l\_game.html) provides build configuration runtime environment platform project and clipboard helpers



\### Networking



\[Networking](https://qtullu.github.io/Lunar/group\_\_\_lunar\_f\_l\_networking.html) provides networking helper functions



\### Math



\[Math](https://qtullu.github.io/Lunar/group\_\_\_lunar\_f\_l\_math.html) provides math helper functions



\### Random



\[Random](https://qtullu.github.io/Lunar/group\_\_\_lunar\_f\_l\_random.html) provides random value helper functions



\### String



\[String](https://qtullu.github.io/Lunar/group\_\_\_lunar\_f\_l\_string.html) provides string formatting helper functions



\### Chrono



\[Chrono](https://qtullu.github.io/Lunar/group\_\_\_lunar\_f\_l\_chrono.html) provides time and chrono helper functions



\### UI



\[UI](https://qtullu.github.io/Lunar/group\_\_\_lunar\_f\_l\_u\_i.html) provides user interface helper functions



\### Debug



\[Debug](https://qtullu.github.io/Lunar/group\_\_\_lunar\_f\_l\_debug.html) provides debug helper functions



\### Audio



\[Audio](https://qtullu.github.io/Lunar/group\_\_\_lunar\_f\_l\_audio.html) provides audio helper functions



\### Data Table



\[Data Table](https://qtullu.github.io/Lunar/group\_\_\_lunar\_f\_l\_data\_table.html) provides data table helper functions



\### Save



\[Save](https://qtullu.github.io/Lunar/group\_\_\_lunar\_f\_l\_save.html) provides save helper functions



\### Physics



\[Physics](https://qtullu.github.io/Lunar/group\_\_\_lunar\_f\_l\_physics.html) provides physics helper functions



\### Blockout



\[Blockout](https://qtullu.github.io/Lunar/group\_\_\_lunar\_f\_l\_blockout.html) provides blockout workflow helper functions



\### JSON



\[JSON](https://qtullu.github.io/Lunar/group\_\_\_lunar\_f\_l\_j\_s\_o\_n.html) provides JSON helper functions



\### Performance



\[Performance](https://qtullu.github.io/Lunar/group\_\_\_lunar\_f\_l\_performance.html) provides runtime performance metric snapshot summary history and graph painting helpers



\### Console



\[Console](https://qtullu.github.io/Lunar/group\_\_\_lunar\_f\_l\_console.html) provides Lunar console print command execution command suggestion and log export helpers



\### Transform



\[Transform](https://qtullu.github.io/Lunar/group\_\_\_lunar\_f\_l\_transform.html) provides actor and scene component transform axis helpers



\### File



\[File](https://qtullu.github.io/Lunar/group\_\_\_lunar\_f\_l\_file.html) provides native folder selection and text file saving helpers



\---



\## Types



Types contain shared enums structs settings records snapshots and data definitions used by the plugin



\### Raw Input



\[Raw Input Types](https://qtullu.github.io/Lunar/group\_\_\_lunar\_types\_raw\_input.html) contains raw input device and input snapshot types



\### Console



\[Console Types](https://qtullu.github.io/Lunar/group\_\_\_lunar\_types\_console.html) contains console message command table command definition command parameter and console settings types



\### Game



\[Game Types](https://qtullu.github.io/Lunar/group\_\_\_lunar\_types\_game.html) contains platform type and platform family types



\### Performance



\[Performance Types](https://qtullu.github.io/Lunar/group\_\_\_lunar\_types\_performance.html) contains performance memory units snapshots histories metric structures and performance settings



\### Transform



\[Transform Types](https://qtullu.github.io/Lunar/group\_\_\_lunar\_types\_transform.html) contains transform axis and transform data type enums



\### UI



\[UI Types](https://qtullu.github.io/Lunar/group\_\_\_lunar\_types\_u\_i.html) contains user interface shared types



\---



\## Settings



\[Plugin Settings](https://qtullu.github.io/Lunar/group\_\_\_lunar\_settings.html) stores project level Lunar settings exposed through Unreal Developer Settings



The settings currently configure Lunar console defaults and Lunar performance monitoring defaults



\---



\## Widgets



\[Draggable Window](https://qtullu.github.io/Lunar/group\_\_\_lunar\_widgets.html) is a reusable draggable and resizable UMG window widget with bounds resize modes handle states and movement events



\---



\## Public headers



The plugin exposes aggregate headers for faster access to public API groups



\### Lunar.h



Main public plugin include



\### LunarFL.h



Aggregate include for all Lunar function libraries



```cpp

\#include "LunarFL.h"



FString PlatformName = LunarFL::Game::GetPlatformName();

FString PercentText = LunarFL::String::FormatPercent01(0.75f);

