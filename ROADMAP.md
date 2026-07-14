# Lunar Roadmap

Current implementation status and planned work for the Lunar plugin

Target Unreal Engine version: **5.8**

---

## Execution Plan

This section defines the delivery order. The numbered feature backlog below remains the detailed source of tasks, but its section number does not imply priority.

### Current State

Verified against the repository on 2026-07-14:

- Console, Raw Input, Performance Monitor, and Draggable Window systems have substantive C++ implementations
- JSON, Random, Chrono, Audio, Debug, Game, Transform, File, Console, Math, String, and Performance utility libraries contain public Blueprint APIs and C++ implementations
- `W_Button`, the base widget pack, `W_LoadingScreen`, `ML_Actor`, and `ML_Object` assets exist, but their remaining interaction behavior still needs in-editor verification and completion
- `UI`, `Save`, `Physics`, `Networking`, `DataTable`, and `Blockout` function libraries are currently API shells without Blueprint functions
- The UE 5.8 Win64 project build has already completed successfully
- Win64 is the only supported runtime platform
- Opt-in periodic Performance Monitor collection routes the current snapshot summary to the Lunar console without taking a second sample

### Priority Definitions

- **P0**: blocks the next stable plugin release
- **P1**: completes the main reusable gameplay and UI toolset
- **P2**: valuable expansion after the core is stable
- **P3**: research or optional work that should not block a release

### Milestone Order

| Milestone | Priority | Outcome | Depends On |
| --- | --- | --- | --- |
| M0. Release Baseline | P0 | Runtime baseline is ready for the owner's manual Win64 packaging pass | None |
| M1. UI Foundation | P0 | One consistent focus, navigation, style, and input-device model | M0 |
| M2. UI Pack And Loading | P0 | Existing widget assets are production-ready and loading flow works across map travel | M1 |
| M3. Core Utility Completion | P1 | Empty utility libraries expose a small useful API instead of placeholders | M0; UI utilities also depend on M1 |
| M4. Activation And Blockout | P1 | Reusable level-design interaction chain and blockout kit | M0, M3 |
| M5. Content And Audio Expansion | P2 | Reusable materials and audio presets with examples | M0 |
| M6. Advanced Runtime Systems | P2 | Camera, UI model preview, and mesh proxy systems | M0-M2 |
| M7. Browser Decision | P3 | Browser support is either scoped with evidence or removed from the roadmap | M0 |

### M0. Release Baseline — Complete

- [x] Supported runtime platform: Win64 only
- [x] Build the project successfully with UE 5.8 for Win64
- [x] Resolve the unfinished `bCollectData` behavior in Performance Monitor
  - [x] Send the requested performance summary to `ULunarConsoleSubsystem`
  - [x] Reuse the current snapshot instead of collecting performance data twice
  - [x] Keep periodic summary output disabled by default to avoid log spam

M0 implementation work is complete. Final `BuildPlugin` packaging and packaging-only fixes are owner-run release tasks and are intentionally not tracked as implementation work here.

### M1. UI Foundation — In Progress

Implementation follows `docs/UI_NAVIGATION_SPEC.md` in strict phase order. Runtime C++ never creates or mutates Content assets, and `ULunarDraggableWindow` remains outside the navigation hierarchy and unchanged.

- [x] **Phase 1 — Types And Settings**
  - [x] Add finalized navigation enums, links, groups, scope, repeat, analog, pointer, action-context, and validation declarations
  - [x] Add finalized typed style, transition, sound, haptic, prompt, and icon-entry declarations
  - [x] Reuse `ELunarInputDeviceType` and expose the shared `FLunarInputDeviceChangedSignature` contract from Raw Input types
  - [x] Add all finalized `Lunar.UI.Action.*` and `Lunar.UI.State.Value.*` native Gameplay Tags
  - [x] Seed the eight built-in semantic action definitions with their default keyboard and gamepad bindings
  - [x] Add `ULunarSettings::Navigation` with Input, Behavior, Default Styles, Audio, Haptics, Prompts, Accessibility, and Diagnostics sections
  - [x] Keep every owner-created Content reference null and mark only intended Lunar defaults with `TODO(LunarUI)`
  - [x] Verify the Phase 1 declarations build on UE 5.8 Win64
- [x] **Phase 2 — Core Runtime**
  - [x] Implement the per-local-player navigation subsystem and non-visual scope stack
  - [x] Implement automatic screen scope lifecycle, registration, stable IDs, groups, priority, explicit links, geometric fallback, wrap, restore, reset, and recovery
  - [x] Route Raw Input to the owning local player and consume handled key-down, matching key-up, analog, pointer, and touch input before gameplay
  - [x] Synchronize authoritative Lunar Selection with native focus and explicit native-focus delegation
- [x] **Phase 3 — Base Widget And Button**
  - [x] Implement `ULunarNavigableWidget`, shared state, feedback, prompt, accessibility, pointer, and scroll hooks
  - [x] Implement `ULunarScrollBox` as the supported non-selectable navigation container
  - [x] Implement `ULunarButton` as the reference release-activated control
  - [x] Document owner-performed `W_Button` reparenting and binding without changing the asset
- [x] **Phase 4 — Value Controls**
  - [x] Implement Slider and OptionSlider orientation, stepping, preview, commit, cancellation, wrap, and repeat behavior
  - [x] Implement Switch directional modes
  - [x] Implement Radio and non-visual Radio Group selection rules
- [x] **Phase 5 — Composite Controls**
  - [x] Implement ListView logical selection, virtualization, stable item IDs, scrolling, and restoration
  - [x] Implement ComboBox filtering and nested option scope
  - [x] Implement ContextMenu and one-scope-at-a-time submenu Back behavior
  - [x] Implement horizontal and vertical Tabs, page lifetimes, and descendant restoration
- [x] **Phase 6 — Prompt And Presentation Assets**
  - [x] Implement prompt receiver and default C++ prompt widget contracts
  - [x] Implement Icon Set, Action Registry, and strongly typed Style Data Asset resolution without creating asset instances
  - [x] Connect per-player device switching, null-safe defaults, visible missing-icon placeholders, and deduplicated configuration errors
  - [x] Leave every owner-created style, sound, haptic, prompt, and icon integration for a separately requested Content pass
- [ ] **Phase 7 — Debugging, Editor Integration, And Owner Handoff**
  - [x] Implement validation, graph diagnostics, graph dump, and debug overlay
  - [x] Add the editor-only `LunarEditor` module for Details customization and automated validation fixes
  - [x] Provide the owner checklist for an example menu covering every control and nested scope
  - [ ] Manually verify mouse, keyboard, Xbox, PlayStation 5, and gameplay-input isolation after owner Content integration
  - [x] Update README and Doxygen documentation for the C++ system and owner handoff
  - [ ] Add screenshots and Content examples after the owner provides or integrates them

Phase 7 C++ implementation, editor integration, diagnostics, and documentation are complete. The phase remains open only for the owner-run Content integration, device matrix, gameplay-input isolation check, and any resulting screenshots or examples.

The next owner pass is tracked as a strict one-widget-at-a-time sequence in [UI Navigation Owner Handoff — Ordered Widget Test And Defaults Queue](docs/UI_NAVIGATION_OWNER_HANDOFF.md#7-ordered-widget-test-and-defaults-queue). Verify each row before assigning its default assets or moving to the next control.

M1 is complete when the C++ system builds on UE 5.8 Win64 and, after owner Content integration, the example menu can be operated without a mouse, focus never becomes lost, input prompts update on device change, handled UI input cannot reach gameplay, and every supported control follows the shared contract.

### M2. UI Pack And Loading

- [ ] Complete and verify `W_ComboBox`, `W_ContextMenu`, `W_OptionSlider`, `W_Radio`, `W_Slider`, `W_Switch`, and `W_Tabs` against the M1 contract
- [ ] Complete `W_GIF` playback, loop, and pause behavior
- [ ] Add consistent change, click, focus, and selection delegates
- [ ] Verify the widget pack with mouse, keyboard, Xbox controller, and PlayStation 5 controller
- [ ] Implement `ULunarLoadingScreenSubsystem` and settings
- [ ] Implement show, hide, visibility query, and level-open APIs
- [ ] Keep the loading screen visible across travel, enforce minimum display time, then fade out after the destination map is ready
- [ ] Verify repeated travel, failed travel, missing-widget handling, PIE, Standalone, and packaged builds

M2 is complete when every existing widget meets one interaction contract and loading-screen travel works in both editor and packaged runtime without manual widget setup.

### M3. Core Utility Completion

- [ ] Define a small useful V1 API before implementing each empty library
- [ ] Implement Save helpers for slot existence, async save/load, delete, metadata, versioning, and failure reporting
- [ ] Implement Data Table helpers for safe row lookup, row names, filtering, and failure reporting
- [ ] Implement Physics helpers for trace and sweep presets, overlap queries, and result filtering
- [ ] Implement Blockout helpers only for operations required by the M4 blockout actors
- [ ] Implement HTTP networking types, delegates, GET, JSON POST, headers, query parameters, timeout, and error handling
- [ ] Add `HTTP` module dependencies only when the networking API is implemented
- [ ] Add Actor and Component utilities from the detailed backlog
- [ ] Remove or explicitly defer any empty public library that does not have an approved V1 use case

M3 is complete when no public `LunarFL*` class is an undocumented placeholder and every new function handles invalid input safely.

### M4-M7. Later Delivery

- [ ] **M4 / P1:** build the activation base classes and components before individual actors
- [ ] **M4 / P1:** prove the button -> trap -> button example chain, then add door, lever, pressure plate, and trigger variants
- [ ] **M4 / P1:** build the blockout actor, helper, texture, and material kit around measured level-design use cases
- [ ] **M5 / P2:** add audio cue, submix, attenuation, and concurrency templates with a small example map
- [ ] **M5 / P2:** import Vitaliy's procedural materials with naming, instance, performance, and example-map checks
- [ ] **M6 / P2:** deliver Free Camera before Photo Mode
- [ ] **M6 / P2:** prototype UI Model Rendering and measure render-target cost before expanding it
- [ ] **M6 / P2:** prototype Mesh Proxy switching with hysteresis, then benchmark MetaHuman and crowd cases
- [ ] **M7 / P3:** time-box the UE 5.8 browser investigation and either approve a supported architecture or remove the feature
- [ ] **Optional:** add a dedicated `LogLunar` category if filtering Lunar messages separately from `LogTemp` becomes useful
- [ ] **Before a public release:** update `Version` and `VersionName` in `Lunar.uplugin` and add a short changelog

### Continuous Definition Of Done

Apply these checks to every milestone:

- [ ] Public C++ API and Blueprint nodes follow consistent Lunar naming and categories
- [ ] Invalid input fails safely and produces an actionable Lunar log or console message
- [ ] Runtime code has no accidental editor-only dependency
- [ ] Platform-specific behavior is guarded and documented
- [ ] New public classes, structs, enums, and meaningful functions have Doxygen comments
- [ ] `README.md`, `docs/mainpage.md`, screenshots, and examples are updated in the same milestone
- [ ] Generated documentation completes without new warnings
- [ ] The project builds and the changed feature works in its intended editor or runtime flow

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

- [x] Raw Input System V1
  - [x] Game Instance subsystem
  - [x] Slate input preprocessor
  - [x] Keyboard mouse and gamepad detection
  - [x] Key pressed released and clicked events
  - [x] Mouse position movement and wheel snapshots
  - [x] Blueprint input state query API

- [x] Performance Monitor V1
  - [x] Game Instance subsystem
  - [x] Automatic interval based sampling
  - [x] Frame memory GPU CPU and disk metrics
  - [x] Performance snapshots
  - [x] Time based snapshot history
  - [x] Average minimum and maximum history values
  - [x] Performance widget
  - [x] Low normal high and full detail levels
  - [x] Performance widget hotkey
  - [x] Project settings integration
  - [x] UMG performance graph painting helper
  - [x] Opt-in periodic snapshot summaries routed to the Lunar console

- [x] Draggable Window V1
  - [x] Header and body dragging
  - [x] Blueprint position and size API
  - [x] Handle edge corner and anywhere resize modes
  - [x] Minimum and maximum size limits
  - [x] Parent and custom widget bounds
  - [x] Clamp soft bounds and edge snap modes
  - [x] Movement and resize state events

- [x] Implemented Utility Function Libraries
  - [x] Game and platform
  - [x] Math
  - [x] Random
  - [x] String
  - [x] Chrono
  - [x] Debug
  - [x] Audio
  - [x] JSON
  - [x] Performance
  - [x] Console
  - [x] Transform
  - [x] File

- [x] Reusable Default Content
  - [x] Float vector and color curve library
  - [x] Curve example assets and maps
  - [x] Base materials and material presets
  - [x] Base shape and dummy meshes
  - [x] Default textures and UI icons
  - [x] Inter Noto Sans and Golos Text font assets
  - [x] Sound class hierarchy and game sound mix
  - [x] Console and performance widgets and data tables

---

## 1. UI Core Framework

Goal: create a custom Lunar UI foundation with predictable gamepad navigation and reusable visual states

- [x] Design and implement shared UI style data
  - [x] Strongly typed per-control style assets and common patch fields
  - [x] Base, value-state, interaction-state, and per-instance override layers
  - [x] Pointer Normal, Hovered, and Pressed interaction states
  - [x] Navigation Normal, Selected, and Pressed interaction states shared by keyboard and gamepad
  - [x] Normal, control-specific, and Disabled value states
  - [x] Root-to-leaf parent resolution, transitions, and device-aware presentation context
- [x] Design and implement base navigation rules
  - [x] Explicit up navigation
  - [x] Explicit down navigation
  - [x] Explicit left navigation
  - [x] Explicit right navigation
  - [x] Deterministic geometric fallback navigation
  - [x] Selection restore
  - [x] Initial selection logic
- [x] Implement the native `ULunarButton` interaction contract
- [x] Create `W_Button`
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
  - [x] Detect current input device type

---

## 2. Input Prompt Widget

Goal: show correct input prompts for Xbox PlayStation keyboard and mouse

- [x] Implement `ULunarInputPromptWidget` and `ILunarInputPromptReceiver`
- [ ] Create `W_InputPrompt`
- [x] Implement runtime input-device prompt resolution contracts
  - [x] Xbox controller family
  - [x] PlayStation 5 controller family
  - [x] Keyboard
  - [x] Mouse
- [x] Implement prompt source data contracts
  - [x] Icon resolution by semantic action and resolved key
  - [x] Localized text fallback by semantic action
  - [x] Platform-specific icon-set selection
- [ ] Author and assign owner-managed Prompt Widget and Icon Set Content
- [ ] Add usage modes
  - [ ] Standalone hint widget
  - [ ] Hint above button
  - [ ] Hint near button
  - [ ] Footer control hint
  - [ ] Tutorial prompt

---

## 3. UI Widgets Pack

Goal: create production ready Lunar UI widgets using the same navigation and style rules

Existing widget assets are tracked separately from their remaining interaction and navigation work

Native `ULunar*` controls are implemented under M1. The `W_*` rows below track owner Content integration and manual verification only.

- [x] Create `W_ComboBox`
  - [ ] Use Lunar entry widgets
  - [ ] Gamepad open close support
  - [ ] Gamepad selection support
- [x] Create `W_ContextMenu`
  - [ ] Use Lunar entries
  - [ ] Close on outside click
  - [ ] Keyboard and gamepad close support
- [x] Create `W_GIF`
  - [ ] Playback control
  - [ ] Loop control
  - [ ] Optional pause control
- [x] Create `W_Emissive`
- [x] Create `W_OptionSlider`
  - [ ] Button based left right option switching
  - [ ] Gamepad support
  - [ ] Value changed delegate
- [x] Create `W_Radio`
  - [ ] Group support
  - [ ] Single selection mode
  - [ ] Gamepad support
- [x] Create `W_Slider`
  - [ ] Mouse drag
  - [ ] Keyboard step
  - [ ] Gamepad step
  - [ ] Value changed delegate
- [x] Create `W_Switch`
  - [ ] On off state
  - [ ] Mouse support
  - [ ] Gamepad support
- [x] Create `W_Tabs`
  - [ ] Use `W_Button` for every tab
  - [ ] Keyboard navigation
  - [ ] Gamepad navigation
  - [ ] Active tab state
  - [ ] Tab changed delegate

---

## 4. Loading Screen System

Goal: create a Lunar loading system that can show a widget during level loading without the manual Create Widget then Open Level flow

- [x] Create `W_LoadingScreen`
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

- [x] Add `ML_Actor` macro library asset
  - [ ] World validation helpers
  - [ ] Owner validation helpers
  - [ ] Component validation helpers
  - [ ] Lunar subsystem access helpers
- [x] Add `ML_Object` macro library asset
  - [ ] World context helpers
  - [ ] Game instance helpers
  - [ ] Runtime validation helpers
- [ ] Add useful scene components
  - [x] Auto rotate component
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
- [x] Add sound class presets
- [x] Add sound mix preset
- [ ] Add submix presets
- [ ] Add attenuation presets
- [ ] Add concurrency presets
- [x] Add Blueprint audio helpers
- [x] Add runtime audio utility functions

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

- [x] Review all existing `LunarFL*` libraries
- [x] List empty libraries: UI Save Physics Networking DataTable and Blockout
- [x] List almost empty libraries: Math String and File
- [x] Add math utilities
- [x] Add random utilities
- [x] Add string utilities
- [x] Add chrono utilities
- [x] Add file utilities
- [ ] Add save utilities
- [x] Add game utilities
- [ ] Add actor utilities
- [ ] Add component utilities
- [x] Add transform utilities
- [ ] Add UI utilities
- [x] Add debug utilities
- [x] Add audio utilities
- [x] Add JSON utilities
- [x] Add performance utilities
- [x] Add console utilities
- [x] Add platform utilities
- [x] Add validation and Lunar console messages where needed

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

- [x] Research `CustomThunk` implementation for wildcard structs
- [x] Add `USTRUCT` wildcard to JSON
- [x] Add JSON to `USTRUCT` wildcard
- [x] Add array to JSON
- [x] Add JSON to array
- [x] Add map to JSON
- [x] Add JSON to map
- [x] Add set to JSON
- [x] Add JSON to set
- [x] Add pretty print option
- [x] Add compact output option
- [x] Add validation and Lunar console messages

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

- [x] Add Doxygen comments to all new classes
- [x] Add Doxygen comments to all new structs
- [x] Add Doxygen comments to all new enums
- [x] Add Doxygen comments to all meaningful functions
- [x] Add system overview pages
- [ ] Add Blueprint examples
- [x] Add C++ examples
- [x] Add Doxygen generation configuration
- [x] Add GitHub Pages deployment workflow
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
- [x] Verify generated Doxygen site

---

## Future Ideas

- [ ] Add more roadmap items as new systems appear
- [ ] Review plugin structure after each major system
- [ ] Keep APIs simple for Blueprint users
- [ ] Keep C++ usage clean through `LunarFL` aliases where appropriate
- [ ] Keep validation messages routed through `ULunarConsoleSubsystem`
