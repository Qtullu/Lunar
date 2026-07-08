# Lunar Roadmap

Roadmap for the Lunar plugin after Console V1

Target Unreal Engine version: **5.8**

---

## 0. Completed

- [x] Console System V1
  - [x] Console widget
  - [x] Runtime message storage
  - [x] Message categories
  - [x] Message verbosity types
  - [x] Message colors from Lunar settings
  - [x] Log search by raw message text
  - [x] Filter by multiple verbosity types
  - [x] Command execution
  - [x] Command suggestions
  - [x] Command history
  - [x] Command history limit
  - [x] Console hotkey
  - [x] Input mode handling
  - [x] Mouse cursor restore
  - [x] Project settings integration

---

## 1. UI Core Framework

Goal: create a custom Lunar UI foundation with predictable gamepad navigation and reusable visual states

- [ ] Design shared UI style data
  - [ ] Default state
  - [ ] Hovered state
  - [ ] Pressed state
  - [ ] Selected state
  - [ ] Disabled state
  - [ ] Gamepad selected state
  - [ ] Gamepad unselected state
- [ ] Design base navigation rules
  - [ ] Explicit up navigation
  - [ ] Explicit down navigation
  - [ ] Explicit left navigation
  - [ ] Explicit right navigation
  - [ ] Fallback navigation
  - [ ] Focus restore
  - [ ] First selected widget logic
- [ ] Create `W_Button`
  - [ ] Mouse support
  - [ ] Keyboard support
  - [ ] Gamepad support
  - [ ] Selected and unselected visuals
  - [ ] Click delegates
  - [ ] Focus delegates
  - [ ] Disabled behavior
- [ ] Add common UI helper functions
  - [ ] Find first focusable Lunar widget
  - [ ] Set focus to Lunar widget
  - [ ] Restore previous focused widget
  - [ ] Detect current input device type

---

## 2. Input Prompt Widget

Goal: show correct input prompts for Xbox PlayStation keyboard and mouse

- [ ] Create `W_InputPrompt`
- [ ] Add input device type support
  - [ ] Xbox controller
  - [ ] PlayStation 5 controller
  - [ ] Keyboard
  - [ ] Mouse
- [ ] Add prompt source data
  - [ ] Icon by input action
  - [ ] Text fallback by input action
  - [ ] Platform specific icon set
- [ ] Add usage modes
  - [ ] Standalone hint widget
  - [ ] Hint above button
  - [ ] Hint near button
  - [ ] Footer control hint
  - [ ] Tutorial prompt

---

## 3. UI Widgets Pack

Goal: create production ready Lunar UI widgets using the same navigation and style rules

- [ ] Create `W_ComboBox`
  - [ ] Use Lunar entry widgets
  - [ ] Gamepad open close support
  - [ ] Gamepad selection support
- [ ] Create `W_ContextMenu`
  - [ ] Use Lunar entries
  - [ ] Close on outside click
  - [ ] Keyboard and gamepad close support
- [ ] Create `W_GIF`
  - [ ] Playback control
  - [ ] Loop control
  - [ ] Optional pause control
- [ ] Create `W_OptionSlider`
  - [ ] Button based left right option switching
  - [ ] Gamepad support
  - [ ] Value changed delegate
- [ ] Create `W_Radio`
  - [ ] Group support
  - [ ] Single selection mode
  - [ ] Gamepad support
- [ ] Create `W_Slider`
  - [ ] Mouse drag
  - [ ] Keyboard step
  - [ ] Gamepad step
  - [ ] Value changed delegate
- [ ] Create `W_Switch`
  - [ ] On off state
  - [ ] Mouse support
  - [ ] Gamepad support
- [ ] Create `W_Tabs`
  - [ ] Use `W_Button` for every tab
  - [ ] Keyboard navigation
  - [ ] Gamepad navigation
  - [ ] Active tab state
  - [ ] Tab changed delegate

---

## 4. Loading Screen System

Goal: create a Lunar loading system that can show a widget during level loading without the manual Create Widget then Open Level flow

- [ ] Create `W_LoadingScreen`
- [ ] Create `ULunarLoadingScreenSubsystem`
- [ ] Add loading screen settings
  - [ ] Loading widget class
  - [ ] Minimum display time
  - [ ] Fade in time
  - [ ] Fade out time
  - [ ] Default loading text
- [ ] Add subsystem functions
  - [ ] `ShowLoadingScreen`
  - [ ] `HideLoadingScreen`
  - [ ] `IsLoadingScreenVisible`
  - [ ] `OpenLevelWithLoadingScreen`
- [ ] Add map loading flow
  - [ ] Show before travel
  - [ ] Keep during travel
  - [ ] Hide after map is ready
- [ ] Add validation and Lunar console messages

---

## 5. Level Design Activation System

Goal: create reusable activation tools where objects can activate deactivate and chain other objects

- [ ] Design base activation architecture
  - [ ] `ALunarActivatableActor`
  - [ ] `ULunarActivatableComponent`
  - [ ] `ULunarActivatorComponent`
- [ ] Add activation states
  - [ ] Inactive
  - [ ] Active
  - [ ] Locked
  - [ ] Disabled
- [ ] Add activation events
  - [ ] On activated
  - [ ] On deactivated
  - [ ] On toggled
  - [ ] On locked
  - [ ] On unlocked
- [ ] Add activation links
  - [ ] Activate target actors
  - [ ] Deactivate target actors
  - [ ] Toggle target actors
  - [ ] Delayed activation
  - [ ] One shot activation
- [ ] Create level design actors
  - [ ] Button actor
  - [ ] Pressure plate actor
  - [ ] Lever actor
  - [ ] Trap actor
  - [ ] Door actor
  - [ ] Trigger volume actor
- [ ] Add example chain
  - [ ] Button activates trap
  - [ ] Trap hits character
  - [ ] Trap deactivates button
- [ ] Add validation and Lunar console messages

---

## 6. Blockout Tools

Goal: provide fast level blockout assets and tools

- [ ] Add blockout actors
  - [ ] Blockout cube
  - [ ] Blockout ramp
  - [ ] Blockout cylinder
  - [ ] Blockout stairs
  - [ ] Blockout doorway
  - [ ] Blockout wall
- [ ] Add blockout helper components
  - [ ] Measurement helper
  - [ ] Snap helper
  - [ ] Debug label helper
- [ ] Add blockout textures
  - [ ] Grid texture
  - [ ] Direction texture
  - [ ] Scale reference texture
  - [ ] Collision color texture
- [ ] Add blockout materials
  - [ ] Grid material
  - [ ] Colored blockout material
  - [ ] Directional material
  - [ ] Transparent planning material

---

## 7. Core Macros And Components

Goal: reduce repeated boilerplate and add reusable actor behavior

- [ ] Add `ML_Actor` macros
  - [ ] World validation helpers
  - [ ] Owner validation helpers
  - [ ] Component validation helpers
  - [ ] Lunar subsystem access helpers
- [ ] Add `ML_Object` macros
  - [ ] World context helpers
  - [ ] Game instance helpers
  - [ ] Runtime validation helpers
- [ ] Add useful scene components
  - [ ] Auto rotate component
  - [ ] Follow component
  - [ ] Distance visibility component
  - [ ] Debug visualization component
  - [ ] Runtime tag component
  - [ ] Object state component
- [ ] Add validation and Lunar console messages where needed

---

## 8. Audio Utility Assets

Goal: add reusable audio templates and audio setup assets without shipping music or sound content

- [ ] Add sound cue templates
- [ ] Add sound class presets
- [ ] Add submix presets
- [ ] Add attenuation presets
- [ ] Add concurrency presets
- [ ] Add Blueprint audio helpers
- [ ] Add runtime audio utility functions

---

## 9. Procedural Materials

Goal: add procedural materials created by Vitaliy

- [ ] Add blockout procedural materials
- [ ] Add utility procedural materials
- [ ] Add debug procedural materials
- [ ] Add animated procedural materials
- [ ] Add environment procedural materials
- [ ] Add material instances and examples

---

## 10. Function Libraries Completion

Goal: fill all empty and nearly empty `LunarFL` libraries with useful Blueprint and C++ helpers

- [ ] Review all existing `LunarFL*` libraries
- [ ] List empty libraries
- [ ] List almost empty libraries
- [ ] Add string utilities
- [ ] Add file utilities
- [ ] Add save utilities
- [ ] Add game utilities
- [ ] Add actor utilities
- [ ] Add component utilities
- [ ] Add transform utilities
- [ ] Add UI utilities
- [ ] Add debug utilities
- [ ] Add platform utilities
- [ ] Add validation and Lunar console messages where needed

---

## 11. Network Function Library

Goal: add HTTP request utilities for Blueprint and C++

- [ ] Add HTTP module dependencies
- [ ] Add request data types
- [ ] Add response data types
- [ ] Add delegates
- [ ] Add `HttpGet`
- [ ] Add `HttpPostJson`
- [ ] Add `HttpPostString`
- [ ] Add custom headers support
- [ ] Add query string builder
- [ ] Add timeout support
- [ ] Add response code output
- [ ] Add response body output
- [ ] Add success and failure handling
- [ ] Add validation and Lunar console messages

---

## 12. JSON Function Library

Goal: add Blueprint friendly JSON conversion for structs and containers

- [ ] Research `CustomThunk` implementation for wildcard structs
- [ ] Add `USTRUCT` wildcard to JSON
- [ ] Add JSON to `USTRUCT` wildcard
- [ ] Add array to JSON
- [ ] Add JSON to array
- [ ] Add map to JSON
- [ ] Add JSON to map
- [ ] Add set to JSON
- [ ] Add JSON to set
- [ ] Add pretty print option
- [ ] Add compact output option
- [ ] Add validation and Lunar console messages

---

## 13. Camera Systems

Goal: add reusable camera pawns and presentation tools

- [ ] Create free camera pawn
  - [ ] WASD movement
  - [ ] Mouse look
  - [ ] Gamepad movement
  - [ ] Gamepad look
  - [ ] Speed control
  - [ ] Fast mode
  - [ ] Slow mode
  - [ ] Optional collision
- [ ] Create photo mode pawn
  - [ ] Freeze gameplay
  - [ ] Free camera controls
  - [ ] FOV control
  - [ ] Roll control
  - [ ] Depth of field control
  - [ ] Time dilation control
  - [ ] Hide UI option
  - [ ] Screenshot support
  - [ ] Camera constraints

---

## 14. UI Model Rendering

Goal: render animated 3D models inside UI

- [ ] Design model preview architecture
  - [ ] `W_ModelPreview`
  - [ ] `ALunarUIPreviewActor`
  - [ ] `ULunarModelPreviewSubsystem`
- [ ] Add skeletal mesh preview
- [ ] Add static mesh preview
- [ ] Add animation playback
- [ ] Add rotation preview
- [ ] Add custom lighting
- [ ] Add transparent background support
- [ ] Add item preview use case
- [ ] Add character preview use case

---

## 15. Mesh Proxy Optimization

Goal: replace expensive skeletal meshes at distance with cheaper representations

- [ ] Design proxy component
  - [ ] `ULunarMeshProxyComponent`
- [ ] Add near representation
  - [ ] Skeletal mesh component
- [ ] Add far representation
  - [ ] Static mesh component
- [ ] Add distance switching
- [ ] Add hysteresis to prevent flicker
- [ ] Add tick optimization
- [ ] Add visibility optimization
- [ ] Add optional vertex animation support
- [ ] Add MetaHuman distance optimization test
- [ ] Add crowd optimization test

---

## 16. Browser Widget Optional

Goal: investigate a browser widget similar to WebBrowser

Status: optional and may be removed later

- [ ] Research current WebBrowser support in UE 5.8
- [ ] Decide whether the widget is needed
- [ ] Create `W_Browser` if needed
- [ ] Add URL loading
- [ ] Add reload
- [ ] Add back forward navigation
- [ ] Add loading state
- [ ] Add error state

---

## 17. Documentation And Screenshots

Goal: document everything after systems are stable

- [ ] Add Doxygen comments to all new classes
- [ ] Add Doxygen comments to all new structs
- [ ] Add Doxygen comments to all new enums
- [ ] Add Doxygen comments to all meaningful functions
- [ ] Add system overview pages
- [ ] Add Blueprint examples
- [ ] Add C++ examples
- [ ] Add screenshots one by one
- [ ] Add console screenshots
- [ ] Add UI widget screenshots
- [ ] Add gamepad navigation screenshots
- [ ] Add input prompt screenshots
- [ ] Add loading screen screenshots
- [ ] Add activation system screenshots
- [ ] Add blockout screenshots
- [ ] Add photo mode screenshots
- [ ] Add model preview screenshots
- [ ] Sync `README.md` with `docs/mainpage.md`
- [ ] Verify generated Doxygen site

---

## Future Ideas

- [ ] Add more roadmap items as new systems appear
- [ ] Review plugin structure after each major system
- [ ] Keep APIs simple for Blueprint users
- [ ] Keep C++ usage clean through `LunarFL` aliases where appropriate
- [ ] Keep validation messages routed through `ULunarConsoleSubsystem`
