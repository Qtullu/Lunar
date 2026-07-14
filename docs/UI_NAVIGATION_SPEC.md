# Lunar Custom UI Navigation System Specification

Status: Design complete; implementation not started

Target: Unreal Engine 5.8, Win64

Roadmap milestone: M1 UI Foundation

This document records the finalized custom Lunar UI navigation design agreed before implementation. Public class, property, enum, function, delegate, Gameplay Tag, module, and source-folder names in this document are final for V1. Content asset names and paths remain owner-controlled and are not created or changed by the C++ implementation pass.

## Purpose

The Lunar UI Navigation System provides deterministic keyboard and gamepad navigation for Lunar widgets without relying on the standard UMG navigation graph.

The system must provide:

- one authoritative Lunar selection model per active local player;
- explicit and automatic directional navigation;
- nested navigation scopes with selection restoration;
- configurable keyboard and gamepad input routing;
- gameplay input isolation while Lunar UI is active;
- reusable state-driven styles for every Lunar control;
- extensible input prompts with device-specific icon sets;
- common behavior across Button, Slider, OptionSlider, Switch, Radio, ScrollBox, ListView, ComboBox, ContextMenu, and Tabs;
- actionable diagnostics through the Lunar Console.

The feature is a subsystem and widget framework, not a collection of isolated Blueprint fixes.

## Core Principles

1. Lunar Selection is the source of truth for its owning local player.
2. Native Slate/UMG focus follows Lunar Selection and does not choose navigation targets.
3. An active Lunar screen owns UI focus exclusively.
4. Standard UMG navigation rules are not part of the supported Lunar workflow.
5. Navigation behavior lives in C++; Blueprint assets provide default presentation and project-specific customization.
6. Visual composition does not imply navigation participation.
7. Pointer interaction and Lunar navigation participation are independently configurable.
8. Keyboard and gamepad share the same Lunar Navigation channel.
9. Every navigation result is deterministic and diagnosable.
10. Invalid configuration fails visibly and reports an actionable Lunar Console message.

## Terminology

### Lunar Selection

The Lunar widget currently selected for keyboard or gamepad interaction by a particular local player. Selection controls navigation, Navigation visual states, activation, prompt visibility, and native focus synchronization for that player.

### Native Focus

The Slate/UMG focus used to route keyboard and UI events. Lunar sets native focus after changing Lunar Selection. Native focus is not used to discover the next navigation target.

### Navigation Scope

A logical collection of Lunar navigation widgets with its own initial selection, last selection, wrap behavior, input capture policy, and navigation graph.

Scopes are non-visual runtime objects. They must not add wrapper widgets to the UMG Designer hierarchy.

### Navigation Group

An optional subdivision inside one scope. Geometric fallback prefers candidates in the same group before considering other widgets in the scope.

### Prompt Widget

A presentation widget that receives resolved input action data. The default implementation may show icons, key names, and text, but custom implementations may render arbitrary UI.

## Runtime Architecture

The following core class names are final for V1:

- `ULunarNavigationSubsystem`;
- `ULunarNavigationScope`;
- `ULunarScreenWidget`;
- `ULunarNavigableWidget`.

The following concrete control class names are final for V1:

- `ULunarButton`;
- `ULunarSlider`;
- `ULunarOptionSlider`;
- `ULunarSwitch`;
- `ULunarRadio`;
- `ULunarRadioGroup`;
- `ULunarScrollBox`;
- `ULunarListView`;
- `ULunarComboBox`;
- `ULunarContextMenu`;
- `ULunarTabs`;
- `ULunarTabHeader`.

### `ULunarNavigationSubsystem`

`ULunarNavigationSubsystem` derives from `ULocalPlayerSubsystem`. Unreal creates one instance for each `ULocalPlayer`, and the instance is initialized and deinitialized with that local player.

Each subsystem owns the runtime navigation state of its local player and is responsible for:

- the active scope stack;
- Lunar Selection and native focus synchronization;
- widget registration and removal;
- input mapping and consumption;
- explicit and fallback navigation;
- selection restoration and reset operations;
- input device handoff;
- navigation repeat;
- validation, diagnostics, and debug visualization;
- resolving shared settings, styles, sounds, haptics, prompts, and icon sets for its local player.

V1 supports multiple local players simultaneously through these independent subsystem instances.

The existing `ULunarRawInputSubsystem` remains a `UGameInstanceSubsystem` and owns the game-instance-level Slate input processor. It records input, identifies the owning local player from the input event, and routes the event to that player's `ULunarNavigationSubsystem`. It does not own Selection, Scope Stack, repeat, or other per-player navigation state.

### `ULunarNavigationScope`

The scope is a non-visual runtime object containing at least:

- a root widget;
- registered widgets;
- initial selection;
- last selection;
- horizontal and vertical wrap settings;
- gameplay input blocking policy;
- validation state;
- parent scope relationship;
- temporary nested scope state.

### `ULunarScreenWidget`

A C++ base class for top-level Lunar screens and windows. It automatically creates and activates its navigation scope when opened and removes it when closed.

Screen scope settings include:

- initial selection;
- restore last selection;
- horizontal wrap;
- vertical wrap;
- block all gameplay input;
- debug and validation behavior.

Normal controls do not inherit from `ULunarScreenWidget`.

### `ULunarNavigableWidget`

The common C++ base for Lunar interactive controls. It provides shared infrastructure only:

- automatic registration and removal;
- Lunar selection eligibility;
- pointer interaction eligibility;
- selection and native focus lifecycle;
- directional links;
- stable navigation ID;
- navigation group and priority;
- style state resolution;
- sounds and haptics;
- input prompt hosting;
- common C++ virtual functions and Blueprint events.

Value-specific behavior remains in specialized control classes.

### Specialized C++ Controls

The system will provide specialized C++ behavior classes for these V1 controls:

- Button;
- Slider;
- OptionSlider;
- Switch;
- Radio and Radio Group;
- ScrollBox;
- ListView and virtualized row presentation;
- ComboBox;
- ContextMenu;
- Tabs and Tab Header.

### Explicit Exclusion: Draggable Window

`ULunarDraggableWindow` is not part of the Lunar UI Navigation control hierarchy. Its existing C++ implementation and `W_DraggableWindow` asset remain unchanged and mouse-only.

The class must not be reparented to `ULunarNavigableWidget`, automatically registered, assigned Lunar Selection, or given navigation style, prompt, sound, or haptic behavior. A navigation screen may be placed visually inside a draggable window without making the window itself navigable.

### Blueprint Presentation Layer

Existing `W_*` assets remain owner-managed visual Blueprint assets. After the C++ controls exist, the owner may explicitly reparent or rebuild those assets as the default visual subclasses; the implementation pass does not perform that Content work.

Examples:

- `W_Button` provides the standard Button layout and Content Slot;
- `W_Slider` provides the standard Slider visuals;
- the planned owner-created `W_InputPrompt` provides the default prompt presentation;
- custom projects may derive new Blueprint widgets without replacing C++ navigation behavior.

### Content Ownership Boundary

The C++ implementation pass must not create, edit, move, rename, reparent, save, or delete any `.uasset`. It must not run editor automation that mutates Content. Blueprint reparenting, default visual composition, Data Asset instances, icon imports, example menus, and other Content work belong to the owner.

Existing relevant widget assets are currently located at:

- `/Lunar/Widgets/Base/W_Button`;
- `/Lunar/Widgets/Base/W_Slider`;
- `/Lunar/Widgets/Base/W_OptionSlider`;
- `/Lunar/Widgets/Base/W_Switch`;
- `/Lunar/Widgets/Base/W_Radio`;
- `/Lunar/Widgets/Base/W_ComboBox`;
- `/Lunar/Widgets/Base/W_ContextMenu`;
- `/Lunar/Widgets/Base/W_Tabs`.

These paths are reference inventory only. Their assets remain untouched until the owner explicitly performs the required Content changes. `W_InputPrompt`, default navigation Style Assets, Action Registry, input Icon Sets, haptic assets, and any additional navigation Content do not receive guessed or predeclared package paths.

C++ default asset references remain null or empty until the owner creates the corresponding assets. A consistent `TODO(LunarUI)` comment is placed only beside a settings or constructor field that is intentionally expected to receive plugin-supplied default Content later. TODO comments are not added to every optional override or normal runtime resolution path.

Required default-asset TODO locations include:

- default Keyboard/Mouse Icon Set Data Asset;
- default Xbox Icon Set Data Asset;
- default PlayStation 5 Icon Set Data Asset;
- each per-control default Style Asset field intended to ship with a Lunar default;
- default Prompt Widget class if Lunar will ship one;
- default Action Registry Data Asset if Lunar will ship one;
- global sound or haptic fields only when a Lunar default asset is intended.

Optional per-widget overrides, project-only classes, and example content remain empty without TODO markers.

After the owner creates an asset, its verified package path may be added to code or settings in a separate explicitly requested pass. C++ must never use `ConstructorHelpers` or another hard reference to a nonexistent guessed asset path.

### Module Ownership

All navigation runtime behavior belongs to the existing `Lunar` Runtime module. The editor and PIE use the same runtime classes and code paths as packaged builds.

A separate `LunarEditor` Editor module contains only Details customization, editor-facing validation presentation, and automated editor fixes. It does not implement Selection, scopes, input routing, control behavior, styles, prompts, or any other runtime decision.

`LunarEditor` may depend on PropertyEditor, UMGEditor, and UnrealEd. The `Lunar` Runtime module must not depend on `LunarEditor` or editor-only modules, and packaged builds exclude `LunarEditor` entirely.

### C++ Source Layout

Navigation C++ uses a feature-oriented root inside the existing runtime module:

- `Source/Lunar/Public/UI/Navigation/Core`;
- `Source/Lunar/Public/UI/Navigation/Controls`;
- `Source/Lunar/Public/UI/Navigation/Styles`;
- `Source/Lunar/Public/UI/Navigation/Prompts`;
- `Source/Lunar/Public/UI/Navigation/Data`;
- `Source/Lunar/Public/UI/Navigation/Types`.

Runtime implementation files mirror the same feature folders under `Source/Lunar/Private/UI/Navigation`. Editor-only files use:

- `Source/LunarEditor/Private/UI/Navigation/Customizations`;
- `Source/LunarEditor/Private/UI/Navigation/Validation`.

Folder responsibilities are:

- Core: subsystem, scope, screen base, navigable-widget base, and private routing helpers;
- Controls: concrete Button, Slider, OptionSlider, Switch, Radio, ScrollBox, ListView, ComboBox, ContextMenu, Tabs, Tab Header, and related widget classes;
- Styles: style Data Asset classes, typed style patches, and style resolution helpers;
- Prompts: prompt receiver interface, default C++ prompt base, and prompt host behavior;
- Data: Icon Set and Action Registry Data Asset classes;
- Types: shared public enums, structs, delegates, links, settings fragments, and resolved contexts.

Only headers required by projects or Blueprint-facing runtime APIs belong under Public. Internal helpers remain under the mirrored Private folders.

Public UObject and interface files use the Unreal class name without the `U` or `I` prefix, and implementation files mirror the same relative path. Representative final mappings are:

| Type | Public header | Private implementation |
| --- | --- | --- |
| `ULunarNavigationSubsystem` | `Source/Lunar/Public/UI/Navigation/Core/LunarNavigationSubsystem.h` | `Source/Lunar/Private/UI/Navigation/Core/LunarNavigationSubsystem.cpp` |
| `ULunarNavigationScope` | `Source/Lunar/Public/UI/Navigation/Core/LunarNavigationScope.h` | `Source/Lunar/Private/UI/Navigation/Core/LunarNavigationScope.cpp` |
| `ULunarScreenWidget` | `Source/Lunar/Public/UI/Navigation/Core/LunarScreenWidget.h` | `Source/Lunar/Private/UI/Navigation/Core/LunarScreenWidget.cpp` |
| `ULunarNavigableWidget` | `Source/Lunar/Public/UI/Navigation/Core/LunarNavigableWidget.h` | `Source/Lunar/Private/UI/Navigation/Core/LunarNavigableWidget.cpp` |
| `ULunarButton` | `Source/Lunar/Public/UI/Navigation/Controls/LunarButton.h` | `Source/Lunar/Private/UI/Navigation/Controls/LunarButton.cpp` |
| `ULunarScrollBox` | `Source/Lunar/Public/UI/Navigation/Controls/LunarScrollBox.h` | `Source/Lunar/Private/UI/Navigation/Controls/LunarScrollBox.cpp` |
| `ULunarInputPromptWidget` | `Source/Lunar/Public/UI/Navigation/Prompts/LunarInputPromptWidget.h` | `Source/Lunar/Private/UI/Navigation/Prompts/LunarInputPromptWidget.cpp` |
| `ULunarInputIconSet` | `Source/Lunar/Public/UI/Navigation/Data/LunarInputIconSet.h` | `Source/Lunar/Private/UI/Navigation/Data/LunarInputIconSet.cpp` |
| `ULunarUIActionRegistry` | `Source/Lunar/Public/UI/Navigation/Data/LunarUIActionRegistry.h` | `Source/Lunar/Private/UI/Navigation/Data/LunarUIActionRegistry.cpp` |

All other concrete controls and style assets follow the same one-class-per-file rule, for example `Controls/LunarComboBox.h` and `Styles/LunarComboBoxStyleAsset.h`. Shared declarations are grouped by responsibility in `Types/LunarNavigationTypes.h`, `Types/LunarNavigationSettings.h`, `Types/LunarUIStyleTypes.h`, `Types/LunarUIFeedbackTypes.h`, and `Types/LunarInputPromptTypes.h`. Private-only helpers do not become public headers merely to mirror a folder.

Existing unrelated Settings, Raw Input, Console, Performance, Function Libraries, and Widgets retain their current paths. This source layout does not create or reorganize any Content folder.

## Selection And Native Focus

Lunar Selection is authoritative.

When Lunar selects a widget:

1. the previous widget receives Unselected;
2. the new widget receives Selected;
3. the new Navigation visual state is applied;
4. native UMG focus is assigned to the selected widget;
5. the owning ScrollBox may scroll it into view;
6. selection and focus events are broadcast.

While a Lunar screen is active:

- non-Lunar widgets cannot take UI focus accidentally;
- unexpected external focus is reclaimed by the current Lunar selection;
- a selected Lunar widget may explicitly delegate native focus to an internal native child such as an editable text control;
- delegated focus remains owned by the selected Lunar widget and active scope.

Standard inherited UMG focus and Navigation settings will be hidden from the normal Lunar Details workflow where possible. The inherited C++ API cannot be removed, but Lunar code will not use standard UMG navigation rules as its public configuration surface.

### Native Focus Delegation And Text Entry

A selected Lunar widget may start an explicit native-focus delegation session for one designated descendant native control. Normal text entry begins through `Accept` or a pointer click on the editable control.

During delegation:

- Lunar Selection remains on the owning Lunar widget;
- native focus is assigned to the delegated descendant;
- the owner remains responsible for the delegated focus;
- scope-level directional navigation is suspended for that local player;
- keyboard text and editing input, including character keys, Space, arrows, Home, End, and deletion keys, is routed to the delegated control instead of being resolved as Lunar navigation;
- D-pad and stick input cannot move Lunar Selection and may be handled by the delegated control or platform text-entry UI;
- relevant editing input is isolated from gameplay.

For the default single-line text-entry policy:

- keyboard Enter or gamepad Accept commits the current value and ends delegation;
- Space inserts text and does not act as Lunar Accept while editing;
- Back cancels the edit, restores the value captured when delegation began, ends delegation, and is consumed before scope-level Back handling;
- ending delegation restores native focus to the owning Lunar widget while preserving Lunar Selection.

A multiline control may override Accept handling so Enter inserts a new line rather than committing. The owner still retains Lunar Selection and delegated-focus ownership.

Delegation termination depends on the cause:

- a pointer click or explicit programmatic Selection change to another eligible Lunar widget commits the current text before moving Selection;
- pushing a child scope commits the current text before the parent scope deactivates;
- the owner becoming Hidden, Collapsed, disabled, destroyed, or otherwise unavailable cancels the edit and restores the value captured at delegation start;
- closing or removing the owning scope cancels the edit and restores the captured value;
- an unexpected native-focus request outside the delegated descendant and active scope does not end editing; Lunar rejects the request and restores native focus to the delegated control.

After either commit or cancel, the delegation session is cleared before normal Selection recovery or scope transition continues.

### Per-Local-Player Navigation State

V1 supports multiple local players with independent navigation state.

Each active local player owns at least:

- a separate Lunar Selection;
- a separate scope stack and active scope;
- separate saved selection and restoration state;
- separate native focus ownership;
- separate navigation repeat state;
- separate active-input-device and pointer/navigation handoff state.

Widgets and scopes belong to exactly one local-player navigation context. Directional links, restoration, fallback, and nested scopes cannot cross between local players.

Styles, icon sets, input bindings, and other default configuration may be shared, but resolving and applying them uses the state of the owning local player.

## Scope Lifecycle And Stack

### Automatic Screen Scope

A `ULunarScreenWidget` automatically creates its scope in the owning local player's navigation context when opened and removes it when closed. No manual Blueprint registration is required for the normal case.

### Nested Scopes

Nested UI opens a new scope on top of the owning local player's current scope stack.

Examples include:

- ComboBox option lists;
- ContextMenu;
- ContextMenu submenus;
- modal windows;
- other popup controls.

Only the top scope for that local player receives navigation. Parent scope selection is retained while the child is active.

When the child scope closes:

1. the child scope is removed;
2. the parent scope becomes active;
3. the previous parent selection is restored;
4. native focus returns to the restored widget.

`Back` is dispatched only inside the top scope and can close at most that one scope per input event.

Back dispatch order is:

1. the selected control receives the action first;
2. if the control declines it, the top scope receives it;
3. the default scope behavior closes that top scope;
4. if both the control and scope decline the action, Lunar emits `Rejected` feedback and consumes the input.

An unhandled `Back` never propagates to a parent scope or gameplay in the same input event. This rule applies even when `bBlockAllGameplayInput` is disabled. The matching key-up event is consumed with the key-down event.

### Initial And Restored Selection

Each scope supports an explicit initial selection by widget reference or stable `NavigationId`.

Selection rules are:

1. restore the last valid selection when reopening a used scope;
2. otherwise use the configured initial selection;
3. otherwise choose the first available widget using a deterministic fallback;
4. allow selection to remain temporarily empty only when no eligible widgets exist.

### Reset Operations

The planned API includes separate operations:

- `ResetSelection()` resets only the active scope to its initial selection;
- `ResetSelectionForScope()` resets a specified scope;
- `ResetAllSelections()` clears and resets saved selection for the whole navigation stack.

If an initial widget is unavailable, reset uses the standard deterministic fallback. A successful reset updates the stored last selection.

## Widget Registration And Eligibility

### Automatic Registration

Lunar navigation widgets automatically register during construction and unregister during destruction.

Each widget automatically finds the nearest owning scope from the UI hierarchy. An explicit scope override is available for unusual layouts.

### Participation Settings

The working selection property name is `bCanReceiveLunarSelection`, avoiding collision with native UMG `IsFocusable` fields.

Defaults are:

| Widget type | Pointer interaction | Lunar selection |
| --- | --- | --- |
| Interactive controls | Enabled | Enabled |
| `ULunarNavigableWidget` base | Configurable | Disabled |
| Screens and logical containers | Not implied | Disabled |
| Prompt, GIF, Emissive, decorative content | Not implied | Disabled |

Pointer interaction and Lunar selection can be enabled or disabled independently on each interactive widget.

Keyboard and gamepad are one Navigation channel. A widget that accepts Lunar Selection is available to D-pad, stick, WASD, and arrow-key navigation.

### Runtime Eligibility

By default, a widget is excluded when it is:

- Hidden;
- Collapsed;
- destroyed or pending destruction;
- disabled;
- outside the active scope;
- configured not to receive Lunar Selection.

A disabled widget may opt into selection through a separate setting. This supports tooltips, disabled-reason text, prompts, and Rejected feedback without allowing activation.

### Selection Recovery

If the selected widget becomes unavailable, the subsystem selects the nearest eligible replacement inside the active scope.

If no replacement exists:

- selection is cleared without closing the scope;
- the scope keeps its intended initial and last selection state;
- selection is restored when an eligible widget becomes available.

### Stable Navigation IDs

Each widget may define an optional stable `NavigationId`.

IDs allow selection and links to survive widget reconstruction, dynamic list changes, and regenerated instances. Direct widget references remain supported for static Designer layouts.

Duplicate IDs inside one scope are configuration errors and are reported to the Lunar Console.

Virtualized ListView items use a separate stable `ItemNavigationId` owned by the data item rather than the generated row widget. Item IDs are unique within their ListView.

## Input Model

### Semantic Actions

Lunar navigation uses semantic Gameplay Tags rather than hard-coded presentation data. The initial built-in actions include:

| Action | Default keyboard keys | Default gamepad input |
| --- | --- | --- |
| `Lunar.UI.Action.Navigate.Up` | `W`, `Up` | D-pad Up, left stick Up |
| `Lunar.UI.Action.Navigate.Down` | `S`, `Down` | D-pad Down, left stick Down |
| `Lunar.UI.Action.Navigate.Left` | `A`, `Left` | D-pad Left, left stick Left |
| `Lunar.UI.Action.Navigate.Right` | `D`, `Right` | D-pad Right, left stick Right |
| `Lunar.UI.Action.Accept` | `Enter`, `Space` | Face Button Bottom |
| `Lunar.UI.Action.Back` | `Escape` | Face Button Right |
| `Lunar.UI.Action.Increase` | `W`, `Up`, `D`, `Right` | D-pad Up/Right, left stick Up/Right |
| `Lunar.UI.Action.Decrease` | `S`, `Down`, `A`, `Left` | D-pad Down/Left, left stick Down/Left |

Tab, Next, and Previous are not part of Lunar navigation.

### Action Registry

Every built-in or project-defined semantic UI action must be registered through data before use. Registration is provided by an `FLunarUIActionDefinition` entry in Lunar Settings or in an assigned Action Registry Data Asset; no C++ registration is required.

An action definition is keyed by its Gameplay Tag and contains the action's configurable input bindings and prompt metadata. Built-in `Lunar.UI.Action.*` actions are supplied as default registered definitions. Project tags use the same contract.

V1 does not create or infer action definitions dynamically at runtime. Duplicate definitions and invalid tags are configuration errors.

When a widget, scope, or prompt requests an unknown action tag:

- the tag cannot execute an input action;
- the subsystem reports a deduplicated configuration Error to the Lunar Console with the tag and requesting owner;
- a requested prompt remains present using the standard missing placeholder rather than silently disappearing.

Changes to registry data or settings invalidate affected resolutions and refresh active prompts and bindings.

Bindings are configurable:

- multiple keys may map to one action;
- keys may be added or removed;
- individual bindings may be disabled;
- keyboard navigation may be disabled completely;
- no mandatory Enhanced Input dependency is required for the core system;
- an Enhanced Input adapter may be added later.

Directional Navigation bindings and Increase/Decrease bindings intentionally overlap. One physical event is resolved contextually and is never broadcast as multiple semantic actions:

1. an active native-focus delegation session receives editing input first;
2. the selected Lunar control may claim a supported semantic control action;
3. an orientation-aware value control claims Increase or Decrease only for the directional axis it uses;
4. an unclaimed direction resolves to the corresponding `Navigate.*` action;
5. Accept and Back then follow their control and scope rules.

For example, Right on a horizontal Slider resolves to Increase, while Up remains Navigate Up. On a vertical Slider, Up resolves to Increase and Right remains Navigate Right. `bInvertValueDirection` reverses which Increase or Decrease action is derived from that direction, so the action name continues to describe the resulting value change. A control reports whether it supports an action through `CanHandleLunarAction` and processes it through `HandleLunarAction`. `Unhandled` continues through the ordered fallback, while `Handled` and `Rejected` are consumed; `Rejected` also emits the standard rejection feedback.

### Input Routing And Gameplay Isolation

Raw Input records the device, player identity, and event before the corresponding local player's Lunar Navigation subsystem decides whether to consume it.

When an active scope handles an input:

1. Lunar Raw Input records the input device, player identity, and key or axis;
2. Lunar Raw Input routes the event to the owning local player's `ULunarNavigationSubsystem`;
3. Lunar Navigation resolves the semantic action;
4. navigation or control behavior executes;
5. the Slate input processor reports the event as handled;
6. the event does not reach Pawn or gameplay input.

Handled key-down and matching key-up events are consumed together. Handled analog axes are also consumed while navigation owns them.

An input event is evaluated only by the navigation context of its owning local player. When that player has no active Lunar scope, Lunar navigation inputs pass through to gameplay.

Each scope has `bBlockAllGameplayInput`, disabled by default:

- disabled: only inputs handled by Lunar UI are consumed;
- enabled: the active scope blocks all gameplay input, suitable for pause and full-screen menus.

### Pointer And Navigation Handoff

Pointer hover does not move Lunar Selection.

When the mouse becomes active:

- pointer Hovered and Pressed styles are used;
- the last Lunar Selection remains stored but its Navigation Selected style is hidden;
- a mouse click assigns the clicked Lunar widget as the new Lunar Selection.

When keyboard or gamepad navigation resumes:

- the stored selection becomes visible again;
- native focus is restored to it;
- Navigation styles and prompts update to the active device.

### Cursor, Capture, And Pointer Lock

Pointer policy is stored per scope. Working policies cover:

- cursor visibility: Inherit, Always Visible, Auto Hide On Navigation, or Always Hidden;
- gameplay pointer capture and lock: Inherit, Release, or Preserve.

Top-level screens default to Auto Hide On Navigation plus Release. Nested scopes inherit their parent policy unless explicitly overridden.

When the first applicable scope activates, the owning local player's subsystem snapshots the pre-UI cursor visibility, capture, and lock state. Release policy releases gameplay mouse capture and pointer lock while the UI remains active. Closing a nested scope reapplies its parent's policy; removing the last scope restores the original snapshot.

Under Auto Hide On Navigation:

- mouse movement or a mouse button shows the cursor and activates pointer presentation;
- keyboard or gamepad navigation hides the cursor without restoring gameplay capture or pointer lock;
- later mouse movement shows it again.

Only the local-player context identified as the owner of a pointer event may change shared cursor state. Gamepad-only scopes belonging to other local players do not take ownership of the hardware cursor.

A navigation control may take temporary pointer capture for an active interaction such as Slider or ScrollBox dragging. It must release capture on pointer up, cancellation, removal, or scope closure. Temporary widget capture never replaces the scope's saved pre-UI pointer state. The existing `ULunarDraggableWindow` continues to manage its own mouse interaction outside this navigation contract.

### Touch Input

Win64 V1 supports primary-touch direct interaction only.

- touch press, release, and tap use Pointer Pressed and activation behavior;
- a successful tap assigns Lunar Selection to the touched Lunar widget, while Navigation Selected presentation remains hidden during touch interaction;
- primary-touch drag is supported for controls that already implement pointer dragging, including Slider and ScrollBox;
- handled touch events are consumed before gameplay; unhandled touch follows the active scope's normal gameplay-blocking policy;
- active device may report Touch, but V1 does not resolve or require touch prompt icons.

V1 does not support multi-touch, pinch or rotation gestures, swipe-to-navigate, long-press semantics, a virtual cursor, an on-screen keyboard supplied by Lunar, or a separate touch navigation graph. These are outside V1 rather than silently emulated as gamepad navigation.

### Analog Stick

The left stick and D-pad navigate by default.

The V1 default analog settings are:

- radial dead zone: `0.25`;
- direction activation threshold: magnitude `0.60`;
- direction release threshold: magnitude below `0.40`;
- direction-change dominance margin: `0.15`;
- repeat interval at the activation threshold: `0.12` seconds;
- repeat interval at full magnitude: `0.06` seconds.

Activation and release use the separate thresholds as hysteresis. While the current cardinal direction remains above the release threshold, a different axis becomes dominant only when its absolute magnitude exceeds the current axis by at least `0.15`. Once the current direction falls below the release threshold, a new direction activates normally when it reaches `0.60`.

For analog repeat, calculate `AccelerationAlpha = SmoothStep(0.60, 1.00, Magnitude)` and `RepeatInterval = Lerp(0.12, 0.06, AccelerationAlpha)`.

Analog magnitude may affect repeat speed but does not change Slider step size.

### Navigation Repeat

Held D-pad, stick, WASD, and arrow inputs support repeat:

1. the first action executes immediately;
2. a default initial delay of `0.35` seconds begins;
3. digital inputs repeat at a default interval of `0.10` seconds;
4. analog inputs repeat using the magnitude-based interval defined above;
5. returning the stick below the release threshold resets repeat state.

The defaults are globally configurable. Controls such as Slider may override the global initial delay, digital interval, and analog interval range.

## Directional Navigation

### Explicit Links

Each navigable widget supports optional Up, Down, Left, and Right links.

Each link may target:

- a direct widget reference;
- a stable `NavigationId`;
- an explicit Block rule.

Resolution behavior is:

1. use the explicit target when it is valid and eligible;
2. use geometric fallback when the explicit target is missing or unavailable;
3. stop when the direction is explicitly Blocked.

### Geometric Fallback

When no valid explicit target exists, the subsystem finds a candidate in the requested screen-space direction.

Fallback uses the centers of the current and candidate widgets' arranged Slate geometries in screen space.

For each candidate:

1. normalize the requested direction as `Direction`;
2. calculate `Delta = CandidateCenter - CurrentCenter`;
3. calculate `Forward = Dot(Delta, Direction)`;
4. calculate `Lateral = Abs(Cross2D(Delta, Direction))`;
5. reject the candidate when `Forward <= 0` or the angle between `Delta` and `Direction` is greater than `60` degrees;
6. calculate `Score = Forward + 2 * Lateral`.

The candidate with the lowest Score wins. Candidates with equal Score are ordered by greater `NavigationPriority`, then by stable registration order.

Fallback remains constrained by:

- active scope;
- Navigation Group;
- runtime eligibility.

### Navigation Priority

Each widget has `NavigationPriority`, default `0`.

When geometry produces equivalent candidates, the widget with greater priority wins. Stable registration order resolves any remaining tie.

### Navigation Groups

Each widget may have an optional Navigation Group.

Each configured group has `bAllowCrossGroupFallback`, enabled by default.

When `bAllowCrossGroupFallback` is enabled, fallback searches:

1. eligible candidates in the same group;
2. then eligible candidates in the rest of the active scope.

When `bAllowCrossGroupFallback` is disabled, automatic fallback and wrap remain inside that group. Failure to find an eligible candidate in the group produces no automatic cross-group move.

Explicit links may always cross groups, regardless of `bAllowCrossGroupFallback`.

### Wrap

Wrap is configured separately for horizontal and vertical directions at scope level. Both are disabled by default.

When enabled, navigation from the boundary moves to the opposite eligible boundary within the same scope and applicable group rules.

## Scroll Containers

`ULunarScrollBox` derives from `UScrollBox` and is the fully supported Lunar scroll-container class for V1. It is not a `ULunarNavigableWidget`, does not receive Lunar Selection, and instead registers its container capabilities with the owning local player's navigation subsystem.

When selection moves to a widget inside a `ULunarScrollBox`, the owning scroll container automatically scrolls the widget fully into view.

Scrolling supports configurable:

- viewport padding;
- smooth or immediate movement;
- transition duration or speed;
- nested scroll-container behavior.

Nested ScrollBoxes use inner-first priority.

When Selection moves to a nested descendant:

1. the nearest owning ScrollBox applies the minimum scroll needed to reveal the widget with its configured viewport padding;
2. Lunar updates or re-evaluates arranged geometry after the inner movement;
3. each ancestor ScrollBox repeats the minimum-scroll operation from the inside outward;
4. processing stops when the widget is fully visible in every owning ScrollBox.

For direct scrolling such as mouse-wheel input, the innermost ScrollBox consumes only the portion of the delta that it can apply. After it reaches its boundary, any remaining delta chains to the next ancestor ScrollBox. Multiple containers do not apply the full original delta simultaneously.

Each `ULunarScrollBox` has `bAllowScrollChaining`, enabled by default. When disabled, that ScrollBox consumes or discards the remaining delta at its boundary instead of forwarding it to an ancestor. This setting does not disable selection-driven scroll-into-view for outer containers.

`ULunarScrollBox` fully supports:

- horizontal and vertical orientation;
- selection-driven scroll into view;
- per-instance viewport padding, smooth or immediate movement, transition duration, and speed;
- nested inner-first scroll chaining;
- mouse-wheel and pointer-drag scrolling;
- primary-touch dragging;
- cancellation and capture release during scope or widget teardown;
- typed scrollbar style configuration;
- local-player and active-scope ownership.

A native `UScrollBox` is not the supported Lunar V1 container and does not receive the complete per-instance Lunar contract. Validation warns when eligible Lunar navigation widgets are placed in a native `UScrollBox` where full Lunar scroll behavior is expected.

## Input Prompt System

### Prompt Ownership

Each Lunar widget may enable an optional Input Prompt host.

The default visibility policy is When Lunar Selected. Visibility remains configurable so a widget may show prompts persistently or control them manually.

### Prompt Actions

A widget supplies an array of semantic prompt actions rather than a single hard-coded key or icon.

The owner may replace or modify its requested action tags at runtime through this API:

- `SetPromptActions` replaces the full requested array;
- `AddPromptAction` adds one action if it is not already requested;
- `RemovePromptAction` removes one requested action.

Each successful change causes the subsystem to resolve and send a new complete prompt snapshot.

Examples:

- Button: Accept;
- Slider: Decrease, Increase, optional Accept;
- Back control: Back;
- composite controls: multiple actions at the same time.

The system resolves each action into a context containing at least:

- semantic action tag;
- active input device;
- resolved key or control;
- icon set;
- resolved icon entry;
- display text when intentionally configured;
- owning Lunar widget;
- selection and enabled state.

### Prompt Presentation

The owner-created default `W_InputPrompt` may show icons, key names, and text.

A custom Prompt Widget may render arbitrary UI, including:

- one or multiple icons;
- text;
- animation;
- composite layouts;
- project-specific hint widgets;
- no standard Lunar visual elements at all.

A Prompt Widget class may be any `UUserWidget` implementing `ILunarInputPromptReceiver`; inheritance from a Lunar base class is not required. The standard `ULunarInputPromptWidget` base implements this interface for the default presentation layer.

The receiver contract exposes one required `BlueprintNativeEvent`:

`ApplyResolvedPromptActions(const TArray<FLunarResolvedPromptAction>& Actions)`

The array is always the complete current snapshot in requested presentation order, not an incremental patch. An empty array clears the current prompt presentation.

The prompt host creates its receiver instance once and reuses it. The subsystem calls `ApplyResolvedPromptActions` again without recreating the widget when relevant state changes, including:

- active input device;
- input bindings;
- icon-set data or overrides;
- requested actions;
- owning-widget Selection or enabled state;
- other resolved prompt context.

A configured Prompt Widget class that does not implement `ILunarInputPromptReceiver` is invalid and produces an actionable Lunar Console error.

### Prompt Layout And Overrides

Per-widget prompt configuration may override:

- Prompt Widget class;
- action array;
- icon set;
- individual icons;
- size;
- position;
- alignment;
- padding;
- visibility policy;
- other presentation settings exposed by the prompt contract.

### Icon Sets

Keyboard and mouse, Xbox, and PlayStation 5 icon sets are separate Data Assets.

Lunar Settings assigns global default icon sets. A control or Prompt Widget may override the selected set.

The owner will add the default icon content later.

### Missing Icon Errors

A missing icon is a configuration error. It does not silently fall back to text.

The default presentation shows a visible white square placeholder and sends an Error to `ULunarConsoleSubsystem` containing:

- owning widget name and path;
- Prompt Widget class;
- semantic action;
- resolved key;
- input device;
- icon set;
- missing mapping information.

The control remains functional.

Errors are deduplicated per unique `Widget + Action + Device + Key + IconSet` combination. The error may be emitted again after relevant settings or icon data change.

## Style System

### Layered State Model

Value state and interaction state are independent.

Value state examples include:

- Normal;
- Checked;
- Active or Selected value;
- On or Off;
- Disabled;
- control-specific value states.

Interaction state examples include:

- Pointer Normal;
- Pointer Hovered;
- Pointer Pressed;
- Navigation Normal;
- Navigation Selected;
- Navigation Pressed.

Keyboard and gamepad use the same Navigation states. Input prompts and device-specific presentation still use the active device type.

The layered model allows combinations such as:

- Checked but not navigation-selected;
- Checked and Navigation Selected;
- Active Tab and Navigation Pressed;
- Disabled but selectable for explanation.

### Style Assets And Overrides

Visual styles are strongly typed per-control Data Assets. V1 does not use a generic name-to-variant property map.

Every common visual style patch contains independently overridable fields:

| Field | Type |
| --- | --- |
| `BackgroundBrush` | `FSlateBrush` |
| `BorderBrush` | `FSlateBrush` |
| `ForegroundBrush` | `FSlateBrush` |
| `BackgroundTint` | `FLinearColor` |
| `ContentColor` | `FSlateColor` |
| `TextColor` | `FSlateColor` |
| `Font` | `FSlateFontInfo` |
| `TextShadowColor` | `FLinearColor` |
| `TextShadowOffset` | `FVector2D` |
| `Padding` | `FMargin` |
| `ContentPadding` | `FMargin` |
| `Opacity` | `float` |
| `RenderTransform` | `FWidgetTransform` |
| `RenderTransformPivot` | `FVector2D` |
| `MinDesiredSize` | `FVector2D` |
| `MaxDesiredSize` | `FVector2D` |

Every field has its own explicit override flag. An unset field inherits without replacing the parent value.

Specialized control style patches add strongly typed, named parts rather than arbitrary keys. These include at least:

- Slider: Track, Fill, and Thumb brushes, tints, thickness, and size;
- OptionSlider: Previous and Next arrow brushes and tints plus value text styling;
- Switch: Track and Handle brushes, tints, size, and offset;
- Radio: Mark brush, tint, and size;
- ScrollBox: Scrollbar Style and scrollbar padding;
- ListView: Row Style and Scrollbar Style references;
- ComboBox: Arrow and Popup brushes plus Row Style and Scrollbar Style references;
- ContextMenu: Panel, Separator, and Submenu Arrow presentation plus Row Style reference;
- Tabs: Header Style, Active Indicator presentation, and page padding.

Button uses the common patch and arbitrary Content Widget presentation. Additional control-specific fields must be added to the corresponding typed style struct rather than a generic property map.

Each style asset contains a Base patch, value-state patches, and interaction-state patches. Resolution occurs in two stages:

1. resolve corresponding fields through the global default style and the assigned asset's parent chain from root to leaf;
2. merge the resolved patches in this order:
   1. Base patch;
   2. current value-state patch;
   3. current interaction-state patch;
   4. per-instance overrides.

An interaction-state field therefore wins over a value-state field only when both explicitly override the same property.

Style parents must use a compatible style type. Parent cycles and incompatible parent types are invalid and must report an actionable configuration error.

### Style Transitions

Each state patch may define destination transition settings containing `Duration` and `Easing`. Transitions are disabled by default; a disabled transition or zero duration applies the resolved style immediately.

One transition operates on the complete resolved visual snapshot.

Interpolated fields include:

- colors and opacity;
- numeric control-specific presentation values;
- margins, sizes, and vectors;
- render transforms and pivots.

Discrete resources such as brushes and fonts switch immediately when the target state becomes active and are not cross-faded by the standard transition system.

Interruption behavior is:

1. when the state returns to the transition's original source before completion, Lunar reverses the same transition from its current progress and plays it back to the source;
2. when a third target state arrives, Lunar captures the currently displayed interpolated values as a new source snapshot and starts a transition to the new resolved target using that target patch's `Duration` and `Easing`;
3. repeated resolution of the current logical target does not restart an unchanged transition;
4. disabling transitions during playback immediately applies the current target snapshot.

Reversal and retargeting must not jump to the previous source or target values before continuing.

Complex project-specific animation remains available through Blueprint events.

### State Events

Every control exposes C++ virtual functions and Blueprint events or delegates for at least:

- Selected;
- Unselected;
- Pressed;
- Released;
- Activated;
- Rejected;
- InputDeviceChanged;
- VisualStateChanged.

`VisualStateChanged` includes value state, interaction state, and input device so arbitrary nested content may animate or replace standard presentation.

### Sounds

Lunar Settings provides global default sounds for:

| Pointer events | Navigation events |
| --- | --- |
| Hovered | Selected |
| Pressed | Pressed |
| Activated | Activated |
| Rejected when applicable | Rejected |

Keyboard uses Navigation sounds.

Every per-widget sound uses a three-mode override:

- Use Global;
- Disabled;
- Custom.

Sound configuration uses `FLunarUISoundSpec` containing:

- `TObjectPtr<USoundBase> Sound`;
- volume multiplier;
- pitch multiplier;
- optional `TObjectPtr<USoundConcurrency> Concurrency`.

Widgets request semantic sound feedback but never create or own playback components. The owning local player's `ULunarNavigationSubsystem` creates non-spatial UI `UAudioComponent` instances with auto-destroy. UI sounds may overlap; an assigned Sound Concurrency asset controls overlap and suppression. Active owned components are stopped when the subsystem deinitializes.

### Haptics

Gamepad haptic feedback supports Selected, Pressed, Activated, and Rejected events.

Haptic overrides use the same three modes as sounds: Use Global, Disabled, or Custom. Global effects may be assigned later without changing the widget API.

Haptic configuration uses `FLunarUIHapticSpec` containing a `TObjectPtr<UForceFeedbackEffect> Effect`. Playback uses non-looping `FForceFeedbackParameters` with `bPlayWhilePaused = true`, `bIgnoreTimeDilation = true`, and one stable Lunar UI playback tag.

The owning local player's `ULunarNavigationSubsystem` sends force feedback only to that player's `APlayerController`. A new Lunar UI haptic effect replaces the previous effect on the same channel instead of stacking. Active Lunar UI force feedback stops when the player's scope stack becomes empty or the subsystem deinitializes.

## Visual Composition

Lunar controls support arbitrary nested content instead of hard-coded labels.

Examples include:

- Button with text, icon, or any custom Content Widget;
- Tab Header with arbitrary visual composition;
- Tab with a separate page Content Widget;
- Prompt host with a default or custom Prompt Widget.

Nested visual content does not become navigable automatically. A nested Lunar widget participates only when its own Lunar selection setting is enabled.

## Control Behavior

### Button

Navigation activation is release-based:

1. Accept Down applies Navigation Pressed;
2. Accept remains held while the Pressed state is visible;
3. Accept Up activates Click;
4. losing selection or canceling before release prevents Click.

Pointer behavior uses Pointer Normal, Hovered, and Pressed states. Button content is arbitrary.

### Slider

Slider direction depends on orientation.

| Orientation | Value controls | Navigation controls |
| --- | --- | --- |
| Horizontal | Left and Right | Up and Down |
| Vertical | Up and Down | Left and Right |

Additional rules:

- value direction can be inverted;
- reaching minimum or maximum does not navigate away from the Slider;
- keyboard and gamepad use the same discrete step settings;
- D-pad, stick, WASD, and arrows use repeat;
- analog magnitude affects repeat speed, not step size;
- each Slider may override repeat timing.

Slider step modes are:

- Absolute: add a fixed numeric amount;
- Percentage: add a percentage of `MaxValue - MinValue`.

Values are clamped, and the final step must reach the exact range boundary.

Slider uses `ELunarSliderCommitMode` with two modes:

- Immediate, the default;
- On Accept.

In Immediate mode, every navigation step updates the committed value immediately. Accept is not included in the Slider prompt; if pressed, it produces `Rejected` feedback and is consumed without changing the value.

In On Accept mode:

1. selecting the Slider initializes a preview value from the committed value;
2. value-direction input changes only the preview and emits preview-change feedback;
3. Accept commits the preview, emits the committed-value event, and keeps the Slider selected;
4. Back with an uncommitted preview restores the committed value and is handled by the Slider without closing the scope;
5. Back with no pending change follows normal scope Back dispatch;
6. losing Selection before Accept cancels the preview and restores the committed value.

Pointer dragging or clicking updates the preview while held and commits on pointer release in both modes. Cancelled pointer capture restores the value from before that pointer interaction. The Accept prompt is present only in On Accept mode.

### OptionSlider

OptionSlider follows Slider orientation rules but changes a discrete option index.

`bWrapOptions` is available and disabled by default. Without wrap, attempts beyond the first or last option keep the widget selected and unchanged.

### Switch

Accept always toggles the Switch.

Directional value control uses a mode rather than a boolean:

| Mode | Value controls | Navigation controls |
| --- | --- | --- |
| Disabled | Accept only | All directions |
| Horizontal | Left = Off, Right = On | Up and Down |
| Vertical | Up and Down set the value | Left and Right |

Horizontal is the default mode and is intended for a Switch inside a vertical menu. Vertical mode supports a horizontal menu layout.

Repeating the value direction when the requested value is already active keeps the widget selected.

### Radio And Radio Group

Lunar Selection and checked state are independent.

- navigation only changes focus and highlight;
- Accept commits the checked Radio;
- `bRequireSelection` is enabled by default;
- when selection is not required, `bAllowDeselect` may allow Accept on the checked item to clear the group;
- `bAllowDeselect` is disabled by default and cannot override required selection.

### ListView

`ULunarListView` is a composite navigation owner. The ListView itself is the single navigable widget registered in its containing scope and remains the Lunar Selection target while the player moves between its items.

Generated and recycled row widgets are presentation proxies. They do not register separately in the outer scope and do not own Lunar Selection. The ListView projects its internal active-item state into the currently generated proxy.

Each data item must provide a stable, non-empty `ItemNavigationId` that is unique within the ListView. Missing and duplicate item IDs are configuration errors reported to the Lunar Console.

The ListView stores:

- the current `ActiveItemId`;
- the active item's last logical index;
- pending scroll and row-generation state;
- per-item eligibility required for navigation.

Navigation along the ListView orientation moves through eligible data items in logical list order rather than using row-widget geometry. Navigation perpendicular to the ListView orientation uses the ListView's normal explicit links or geometric fallback to leave the composite control.

When an off-screen item becomes active:

1. the logical `ActiveItemId` updates;
2. the ListView requests scrolling to that item;
3. the ListView waits for its row proxy to be generated;
4. the generated proxy receives the active-item visual state;
5. native focus remains on the `ULunarListView` owner throughout the transition.

After refresh or row regeneration, restoration first searches for the stored `ActiveItemId`. If that item no longer exists or is ineligible, restoration starts at the clamped previous logical index, searches forward for the first eligible item, and then searches backward. If no eligible item exists, `ActiveItemId` becomes empty while the ListView may remain Lunar-selected; activation then produces `Rejected` feedback.

### ComboBox

Opening the option list pushes a nested scope.

- temporary selection starts at the committed value;
- if the committed value is unavailable, the first eligible option is selected;
- navigation changes only temporary selection;
- Accept commits the temporary option and closes the scope;
- Back closes without applying;
- clicking outside closes without applying;
- clicking an option commits it and closes the scope.

The popup option list uses the virtualized `ULunarListView` composite model. Every option has a stable, unique Option ID, display `FText`, enabled state, and optional disabled-reason `FText`. The committed and temporary values are stored by Option ID rather than generated row instance.

Search is optional and disabled by default. When enabled:

- the search field is a navigable control in the popup scope but does not replace the committed option as the default initial item;
- entering the field uses the standard text-entry delegation contract;
- the default matcher performs a case-insensitive, culture-aware substring search on display text;
- C++ and Blueprint may supply a custom filter predicate;
- the query resets every time the popup opens by default, with optional per-ComboBox restoration.

Filtering never modifies the source option array or committed value. After a filter change, temporary selection remains on the same Option ID when it is still visible and eligible. Otherwise it moves to the first eligible filtered result. With no eligible result, temporary selection becomes empty and Accept produces `Rejected`.

Disabled options remain visible. By default they are skipped by navigation and cannot be committed by pointer. An option may opt into disabled selection so it can expose its reason and prompts; Accept or pointer activation then produces `Rejected` and keeps the popup open.

Missing or duplicate Option IDs are configuration errors. Refresh and row regeneration use the same ID-first, previous-index fallback as `ULunarListView`.

### ContextMenu

ContextMenu pushes a nested scope and resets selection to initial or first available item every time it opens by default.

Optional restoration may be enabled for a specific menu.

ContextMenu supports nested submenus. Each submenu pushes another scope, and Back closes only the top submenu.

### Tabs

Navigation focus and active tab are independent. Navigation highlights a Tab Header; Accept activates its page.

| Orientation | Move between Tab Headers | Leave the Tab strip |
| --- | --- | --- |
| Horizontal | Left and Right | Up and Down |
| Vertical | Up and Down | Left and Right |

Tab Headers and page contents accept arbitrary nested widgets.

Each tab has a stable Tab ID and either a page widget class or an explicitly supplied page instance. Duplicate Tab IDs and missing page sources are configuration errors.

Tabs use `ELunarTabPageLifetime`:

- Lazy Cached, the default: create a page on first activation and retain it until the Tabs control is destroyed;
- Eager: create every page when the Tabs control initializes;
- Recreate On Activation: destroy the inactive page and create a new instance the next time it activates.

Inactive retained pages use Collapsed visibility. Their descendants remain registered with the scope but are ineligible while Collapsed. Activating a page makes it visible before graph validation and scroll/fallback calculation.

Accept on a Tab Header activates the page but keeps Lunar Selection on the header. Perpendicular navigation may then leave the strip and enter the active page. Each cached page stores its last valid descendant Navigation ID so re-entry can restore its previous item; otherwise it uses deterministic fallback.

When a page switch is requested programmatically while Selection is inside the old page, Selection first moves to the destination Tab Header. Any delegated text edit follows the deliberate-transition commit rule.

If page creation fails, the current active page remains active, the requested header receives `Rejected`, and an Error is sent to the Lunar Console. Cached pages are destroyed with the Tabs control; Recreate On Activation pages are destroyed after deactivation and Selection transfer completes.

## Accessibility And Localization

V1 uses Unreal's Slate and platform accessibility path rather than implementing a separate screen reader or text-to-speech engine.

Every navigable Lunar control exposes localized `FText` fields for an accessible name and optional description. Value controls also expose their current value or checked state, and disabled-selectable controls expose their disabled reason. Lunar Selection changes are mirrored to native accessible focus, while delegated text controls are announced as the active editable child.

Controls emit native accessibility notifications for focus, activation, value changes, checked-state changes, and relevant disabled feedback. Prompt icons and decorative content are not the only source of action meaning and are excluded from the accessible tree by default unless a project explicitly supplies accessible text.

A missing accessible name on an eligible navigable control produces a validation Warning. It does not prevent runtime navigation.

Accessibility settings include a reduced-motion option. When enabled, standard Lunar style transitions apply immediately; project-specific Blueprint animations receive the setting through state context and remain responsible for honoring it.

All built-in user-visible text uses `FText` and `LOCTEXT`, String Tables, or equivalent localized resources. Public display-name, prompt-text, option-text, disabled-reason, and accessible-text fields use `FText`; stable IDs and Gameplay Tags are never localized.

Runtime culture changes invalidate active prompt text, ComboBox search results, accessible text, and other derived display data. Default search uses culture-aware case folding. Slate owns shaping and right-to-left text flow. Lunar does not automatically mirror explicit directional links; geometric fallback continues to represent physical screen directions.

## Editor Integration

Native UMG navigation and focus fields are hidden for Lunar navigation widgets through the `LunarEditor` editor-only module. The `Lunar` runtime module must not acquire PropertyEditor, UMGEditor, or UnrealEd dependencies.

The editor module registers `IDetailCustomization` handlers through `FPropertyEditorModule` for Lunar navigable widget classes and their Blueprint subclasses. The customization:

- hides inherited `UWidget::Navigation` configuration;
- hides native focusability fields whose values are controlled internally by Lunar;
- leaves those fields unchanged for non-Lunar widgets;
- exposes the Lunar Selection, links, groups, scope, style, prompt, sound, and haptic properties in Lunar categories;
- shows an explanatory read-only row that native UMG navigation is managed by Lunar.

The module unregisters every class customization during shutdown and refreshes affected Details views after registration changes.

Hidden fields remain technically reachable through C++, serialization, and uncommon editor surfaces. Therefore validation also detects non-empty native UMG Navigation data on a Lunar widget, reports an actionable Error, and offers an editor fix to clear it. Runtime Lunar navigation ignores native UMG navigation data, so it cannot override Lunar Selection or target resolution.

The internal native focusability needed to synchronize Slate focus is assigned by the Lunar C++ base class and is not treated as designer-configurable selection eligibility. Designers use `bCanReceiveLunarSelection` instead.

## Diagnostics And Validation

### Scope Validation

The subsystem validates a scope when it activates and when its graph changes.

Validation includes:

- duplicate Navigation IDs;
- unresolved explicit references;
- references outside the intended scope;
- missing or invalid initial selection;
- empty scopes;
- inaccessible or unreachable widgets;
- duplicate, invalid, or unknown semantic action definitions;
- invalid style parent cycles;
- missing prompt icon entries;
- missing accessible names on eligible controls;
- native UMG navigation data configured on Lunar widgets;
- other configuration that would produce nondeterministic navigation.

Messages are deduplicated until relevant configuration changes.

### Debug Overlay

The built-in debug overlay may display:

- active local-player navigation contexts;
- active scope and scope stack;
- current Lunar Selection;
- native focus owner;
- Navigation IDs;
- Navigation Groups;
- Navigation Priority;
- explicit directional links;
- calculated fallback links;
- blocked directions;
- widget eligibility state.

The subsystem can also dump the active graph to the Lunar Console.

## Final Public API Naming

### Naming Conventions

- UObject classes use `ULunar...`;
- interfaces use the Unreal `ULunar...` and `ILunar...` pair;
- structs and delegates use `FLunar...`;
- enums use `ELunar...`;
- stable identifier fields use the `Id` suffix, for example `NavigationId`, `ItemId`, and `TabId`;
- Boolean properties use the `b` prefix;
- multicast delegate properties use the `On...` prefix;
- overridable C++ lifecycle and state hooks use `NativeOn...`; action-policy hooks use `NativeCan...` and `NativeHandle...`;
- Blueprint extension events use `BP_On...`;
- Blueprint categories begin with `Lunar|UI|Navigation`;
- private implementation helpers may use narrower names but must not create a second public naming vocabulary.

### Final Support Classes And Interfaces

The finalized public support types are:

- `ULunarWidgetStyleAsset`, the abstract style Data Asset base;
- `ULunarButtonStyleAsset`;
- `ULunarSliderStyleAsset`;
- `ULunarOptionSliderStyleAsset`;
- `ULunarSwitchStyleAsset`;
- `ULunarRadioStyleAsset`;
- `ULunarScrollBoxStyleAsset`;
- `ULunarListViewStyleAsset`;
- `ULunarComboBoxStyleAsset`;
- `ULunarContextMenuStyleAsset`;
- `ULunarTabsStyleAsset`;
- `ULunarInputPromptStyleAsset`;
- `ULunarInputIconSet`, the input-icon Data Asset class;
- `ULunarUIActionRegistry`, the semantic-action Data Asset class;
- `ULunarInputPromptReceiver` and `ILunarInputPromptReceiver`;
- `ULunarInputPromptWidget`, the default abstract C++ prompt widget base;
- `ULunarListViewItem` and `ILunarListViewItem`, the virtualized item-data contract.

Style, Icon Set, and Action Registry classes derive from `UDataAsset`. `ULunarInputPromptWidget` derives from `UUserWidget`. `ULunarRadioGroup` is a non-visual `UObject`; visual controls derive from `ULunarNavigableWidget` except `ULunarScrollBox`, which derives from `UScrollBox`.

### Final Enums

| Enum | Values |
| --- | --- |
| `ELunarNavigationDirection` | `Up`, `Down`, `Left`, `Right` |
| `ELunarNavigationLinkMode` | `Automatic`, `Widget`, `NavigationId`, `Block` |
| `ELunarUIInteractionState` | `PointerNormal`, `PointerHovered`, `PointerPressed`, `NavigationNormal`, `NavigationSelected`, `NavigationPressed` |
| `ELunarUIActionResult` | `Unhandled`, `Handled`, `Rejected` |
| `ELunarPromptVisibilityPolicy` | `WhenSelected`, `Always`, `Manual` |
| `ELunarFeedbackOverrideMode` | `UseGlobal`, `Disabled`, `Custom` |
| `ELunarScrollIntoViewMode` | `Immediate`, `Smooth` |
| `ELunarCursorVisibilityPolicy` | `Inherit`, `AlwaysVisible`, `AutoHideOnNavigation`, `AlwaysHidden` |
| `ELunarPointerCapturePolicy` | `Inherit`, `Release`, `Preserve` |
| `ELunarSliderStepMode` | `Absolute`, `Percentage` |
| `ELunarSliderCommitMode` | `Immediate`, `OnAccept` |
| `ELunarSwitchDirectionMode` | `Disabled`, `Horizontal`, `Vertical` |
| `ELunarTabPageLifetime` | `LazyCached`, `Eager`, `RecreateOnActivation` |
| `ELunarNavigationValidationLevel` | `Disabled`, `ErrorsOnly`, `WarningsAndErrors`, `All` |

The existing `ELunarInputDeviceType` remains the canonical device enum, and `ELunarConsoleMessageVerbosity` is reused by validation messages sent to the Lunar Console. Standard Unreal `EOrientation`, `EInputEvent`, and `EEasingFunc::Type` are reused instead of adding Lunar duplicates.

### Final Core Structs

| Struct | Final primary fields or role |
| --- | --- |
| `FLunarNavigationLink` | `Mode`, `Widget`, `NavigationId` |
| `FLunarNavigationGroupSettings` | `GroupId`, `bAllowCrossGroupFallback` |
| `FLunarNavigationRepeatSettings` | `InitialDelay`, `DigitalRepeatInterval`, `AnalogIntervalAtThreshold`, `AnalogIntervalAtFullMagnitude` |
| `FLunarAnalogNavigationSettings` | `RadialDeadZone`, `ActivationThreshold`, `ReleaseThreshold`, `DirectionChangeDominanceMargin` |
| `FLunarScrollIntoViewSettings` | `Mode`, `ViewportPadding`, `TransitionDuration`, `ScrollSpeed` |
| `FLunarPointerPolicy` | `CursorVisibilityPolicy`, `PointerCapturePolicy` |
| `FLunarNavigationScopeSettings` | `InitialSelectionWidget`, `InitialSelectionId`, `bRestoreLastSelection`, `bWrapHorizontal`, `bWrapVertical`, `bBlockAllGameplayInput`, `PointerPolicy`, `NavigationGroups` |
| `FLunarUIActionBinding` | `Key`, `bEnabled` |
| `FLunarUIActionDefinition` | `ActionTag`, `Bindings`, `DisplayText` |
| `FLunarUIActionContext` | `ActionTag`, `Key`, `InputDevice`, `InputEvent`, `bHasNavigationDirection`, `NavigationDirection`, `bIsRepeat`, `AnalogMagnitude`, `LocalPlayer` |
| `FLunarPromptActionRequest` | `ActionTag`, `PreferredKey`, `DisplayTextOverride`, `IconOverride`, `bEnabled` |
| `FLunarResolvedPromptAction` | `ActionTag`, `InputDevice`, `ResolvedKey`, `IconSet`, `Icon`, `DisplayText`, `OwnerWidget`, `bSelected`, `bEnabled` |
| `FLunarInputIconEntry` | `InputKey`, `Icon` |
| `FLunarUIVisualState` | `ValueStateTag`, `InteractionState`, `InputDevice` |
| `FLunarStyleTransitionSettings` | `bEnabled`, `Duration`, `Easing` |
| `FLunarCommonStylePatch` | the common typed style fields and their override flags |
| `FLunarUISoundSpec` | `Sound`, `VolumeMultiplier`, `PitchMultiplier`, `Concurrency` |
| `FLunarUIHapticSpec` | `Effect` |
| `FLunarUISoundOverride` | `Mode`, `CustomSound` |
| `FLunarUIHapticOverride` | `Mode`, `CustomHaptic` |
| `FLunarUISoundOverrides` | named Pointer and Navigation sound-event overrides |
| `FLunarUIHapticOverrides` | named Navigation haptic-event overrides |
| `FLunarComboBoxOption` | `OptionId`, `DisplayText`, `bEnabled`, `bCanReceiveSelectionWhenDisabled`, `DisabledReason`, `Payload` |
| `FLunarTabDescriptor` | `TabId`, `HeaderWidgetClass`, `PageWidgetClass`, `PageWidgetInstance`, `bEnabled`, `DisabledReason` |
| `FLunarNavigationValidationMessage` | `Verbosity`, `Code`, `Message`, `OwnerPath` |

Typed style patches use the control prefix, for example `FLunarButtonStylePatch`, `FLunarSliderStylePatch`, and `FLunarSwitchStylePatch`. They do not use anonymous maps or `Variant` suffixes.

### Final Settings Structs And Properties

`ULunarSettings` exposes one property named `Navigation` of type `FLunarNavigationSettings`.

`FLunarNavigationSettings` contains:

- `Input` of type `FLunarNavigationInputSettings`;
- `Behavior` of type `FLunarNavigationBehaviorSettings`;
- `DefaultStyles` of type `FLunarNavigationDefaultStyleSettings`;
- `Audio` of type `FLunarUIAudioSettings`;
- `Haptics` of type `FLunarUIHapticSettings`;
- `Prompts` of type `FLunarInputPromptSettings`;
- `Accessibility` of type `FLunarUIAccessibilitySettings`;
- `Diagnostics` of type `FLunarNavigationDiagnosticsSettings`.

The finalized fields of those settings structs are:

| Settings struct | Final fields |
| --- | --- |
| `FLunarNavigationInputSettings` | `ActionDefinitions`, `DefaultActionRegistry`, `bEnableKeyboardNavigation`, `RepeatSettings`, `AnalogSettings` |
| `FLunarNavigationBehaviorSettings` | `bEnableGeometricFallback`, `FallbackConeHalfAngleDegrees`, `FallbackLateralWeight`, `bAllowCrossGroupFallbackByDefault`, `bRecoverSelectionAutomatically`, `DefaultScrollIntoViewSettings`, `DefaultPointerPolicy`, `bAllowScrollChainingByDefault` |
| `FLunarNavigationDefaultStyleSettings` | the `Default...Style` fields listed below |
| `FLunarUIAudioSettings` | the `Default...Sound` fields listed below |
| `FLunarUIHapticSettings` | the `Default...Haptic` fields listed below |
| `FLunarInputPromptSettings` | `DefaultPromptWidgetClass`, `DefaultPromptVisibilityPolicy`, `DefaultKeyboardMouseIconSet`, `DefaultXboxIconSet`, `DefaultPlayStation5IconSet` |
| `FLunarUIAccessibilitySettings` | `bReduceMotion`, `bValidateAccessibleNames` |
| `FLunarNavigationDiagnosticsSettings` | `ValidationLevel`, `bEnableDebugOverlayByDefault`, `bIncludeCalculatedLinksInGraphDump` |

Every `ULunarWidgetStyleAsset` uses the property names `ParentStyle`, `BaseStyle`, `ValueStateStyles`, and `InteractionStateStyles`. Concrete style subclasses replace the state-set types with their typed control-specific structs.

The default Content reference fields are:

- `DefaultKeyboardMouseIconSet`;
- `DefaultXboxIconSet`;
- `DefaultPlayStation5IconSet`;
- `DefaultPromptWidgetClass`;
- `DefaultActionRegistry`;
- `DefaultButtonStyle`;
- `DefaultSliderStyle`;
- `DefaultOptionSliderStyle`;
- `DefaultSwitchStyle`;
- `DefaultRadioStyle`;
- `DefaultScrollBoxStyle`;
- `DefaultListViewStyle`;
- `DefaultComboBoxStyle`;
- `DefaultContextMenuStyle`;
- `DefaultTabsStyle`;
- `DefaultInputPromptStyle`.

`FLunarUIAudioSettings` uses:

- `DefaultPointerHoveredSound`;
- `DefaultPointerPressedSound`;
- `DefaultPointerActivatedSound`;
- `DefaultPointerRejectedSound`;
- `DefaultNavigationSelectedSound`;
- `DefaultNavigationPressedSound`;
- `DefaultNavigationActivatedSound`;
- `DefaultNavigationRejectedSound`.

`FLunarUIHapticSettings` uses:

- `DefaultNavigationSelectedHaptic`;
- `DefaultNavigationPressedHaptic`;
- `DefaultNavigationActivatedHaptic`;
- `DefaultNavigationRejectedHaptic`.

These fields remain null with the documented `TODO(LunarUI)` markers until the owner creates the assets.

### Final Subsystem And Scope API

`ULunarNavigationSubsystem` public Blueprint-facing functions are:

- `PushNavigationScope`;
- `PopNavigationScope`;
- `GetActiveNavigationScope`;
- `GetNavigationScopeStack`;
- `SetSelectedWidget`;
- `SetSelectedWidgetById`;
- `GetSelectedWidget`;
- `ClearSelection`;
- `ResetSelection`;
- `ResetSelectionForScope`;
- `ResetAllSelections`;
- `Navigate`;
- `RefreshNavigationGraph`;
- `ValidateNavigationScope`;
- `DumpActiveNavigationGraph`;
- `SetNavigationDebugOverlayEnabled`;
- `IsNavigationDebugOverlayEnabled`;
- `GetLastInputDevice`.

Registration and focus-routing functions are public C++ infrastructure but are not normal Blueprint workflow:

- `RegisterNavigableWidget` and `UnregisterNavigableWidget`;
- `RegisterScrollBox` and `UnregisterScrollBox`;
- `BeginNativeFocusDelegation`;
- `CommitNativeFocusDelegation`;
- `CancelNativeFocusDelegation`;
- `IsNativeFocusDelegationActive`.

Subsystem delegates are:

- `OnSelectionChanged` using `FLunarSelectionChangedSignature`;
- `OnActiveScopeChanged` using `FLunarActiveScopeChangedSignature`;
- `OnNavigationRejected` using `FLunarNavigationRejectedSignature`;
- the existing `OnInputDeviceChanged` contract from Raw Input.

`ULunarNavigationScope` exposes `RootWidget`, `Settings`, and read-only runtime state through:

- `GetOwningLocalPlayer`;
- `GetParentScope`;
- `GetRegisteredWidgets`;
- `GetInitialSelectionId`;
- `GetLastSelectionId`;
- `IsActive`;
- `RequestValidation`.

Scope lifecycle functions are named `InitializeScope`, `ActivateScope`, and `DeactivateScope` and are owned by the subsystem rather than normal Blueprint callers.

### Final Screen And Navigable Widget API

`ULunarScreenWidget` properties are:

- `NavigationScopeSettings`;
- `bAutoActivateNavigationScope`;
- read-only `NavigationScope`.

Its functions are `OpenScreen`, `CloseScreen`, `IsScreenOpen`, and `GetNavigationScope`. Its delegates are `OnScreenOpened` and `OnScreenClosed`.

`ULunarNavigableWidget` configuration properties are:

- `bCanReceiveLunarSelection`;
- `bCanInteractWithPointer`;
- `bCanReceiveSelectionWhenDisabled`;
- `NavigationId`;
- `NavigationGroup`;
- `NavigationPriority`;
- `UpLink`, `DownLink`, `LeftLink`, and `RightLink`;
- `NavigationScopeOverride`;
- `StyleAsset` and `StyleOverrides`;
- `bEnableInputPrompt`;
- `PromptWidgetClass`;
- `PromptActions`;
- `PromptVisibilityPolicy`;
- `PromptIconSetOverride`;
- `SoundOverrides`;
- `HapticOverrides`;
- `bScrollIntoViewOnSelection`;
- `AccessibleName`;
- `AccessibleDescription`;
- `DisabledReason`.

Its public functions are:

- `RequestLunarSelection`;
- `IsLunarSelected`;
- `CanReceiveLunarSelection`;
- `ActivateLunarWidget`;
- `CanHandleLunarAction`;
- `HandleLunarAction`;
- `SetStyleAsset`;
- `RefreshVisualState`;
- `SetPromptActions`;
- `AddPromptAction`;
- `RemovePromptAction`;
- `BeginNativeFocusDelegation`;
- `CommitNativeFocusDelegation`;
- `CancelNativeFocusDelegation`;
- `IsNativeFocusDelegationActive`.

Common multicast delegate properties are:

- `OnLunarSelected`;
- `OnLunarUnselected`;
- `OnLunarPressed`;
- `OnLunarReleased`;
- `OnLunarActivated`;
- `OnLunarRejected`;
- `OnLunarInputDeviceChanged`;
- `OnLunarVisualStateChanged`.

The corresponding C++ state hooks use `NativeOnLunarSelected`, `NativeOnLunarUnselected`, `NativeOnLunarPressed`, `NativeOnLunarReleased`, `NativeOnLunarActivated`, `NativeOnLunarRejected`, `NativeOnLunarInputDeviceChanged`, and `NativeOnLunarVisualStateChanged`. Blueprint extension events use the same suffixes with the `BP_` prefix.

Action dispatch enters the non-overridable public wrappers `CanHandleLunarAction` and `HandleLunarAction`. They call `NativeCanHandleLunarAction` and `NativeHandleLunarAction`; the default native implementations expose `BP_CanHandleLunarAction` and `BP_HandleLunarAction`. The `Can...` chain returns `bool`; the `Handle...` chain returns `ELunarUIActionResult`. Each wrapper evaluates one deterministic native-to-Blueprint override chain for a given control and event.

### Final Control API

| Class | Final primary properties | Final primary functions | Final delegates |
| --- | --- | --- | --- |
| `ULunarButton` | base properties only | `Click` | `OnClicked` |
| `ULunarSlider` | `MinValue`, `MaxValue`, `Value`, `StepMode`, `StepSize`, `Orientation`, `bInvertValueDirection`, `CommitMode`, `bOverrideRepeatSettings`, `RepeatSettingsOverride` | `SetValue`, `GetValue`, `SetValueRange`, `GetPreviewValue`, `CommitPreviewValue`, `CancelPreviewValue` | `OnValueChanged`, `OnPreviewValueChanged`, `OnValueCommitted` |
| `ULunarOptionSlider` | `Options`, `SelectedIndex`, `bWrapOptions`, `Orientation`, `bInvertValueDirection` | `SetOptions`, `SetSelectedIndex`, `GetSelectedIndex`, `GetSelectedOption` | `OnSelectedIndexChanged` |
| `ULunarSwitch` | `bIsOn`, `DirectionMode` | `SetIsOn`, `IsOn`, `Toggle` | `OnSwitchChanged` |
| `ULunarRadio` | `RadioId`, `RadioGroup`, `bIsChecked` | `SetChecked`, `IsChecked`, `RequestChecked` | `OnCheckedChanged` |
| `ULunarRadioGroup` | `bRequireSelection`, `bAllowDeselect`, `SelectedRadioId` | `SetSelectedRadio`, `SetSelectedRadioById`, `GetSelectedRadio`, `GetSelectedRadioId`, `ClearSelectedRadio` | `OnSelectionChanged` |
| `ULunarScrollBox` | `ScrollIntoViewSettings`, `bAllowScrollChaining`, `StyleAsset` | `ScrollWidgetIntoLunarView`, `CancelLunarScroll`, `IsLunarScrollActive` | `OnLunarScrollStarted`, `OnLunarScrollFinished` |
| `ULunarListView` | `Items`, `ActiveItemId`, `Orientation` | `SetItems`, `SetActiveItemById`, `GetActiveItem`, `GetActiveItemId`, `ClearActiveItem`, `RefreshNavigationItems` | `OnActiveItemChanged` |
| `ULunarComboBox` | `Options`, `SelectedOptionId`, `bEnableSearch`, `bRestoreSearchQuery` | `SetOptions`, `SetSelectedOptionById`, `GetSelectedOptionId`, `OpenComboBox`, `CloseComboBox`, `IsComboBoxOpen`, `SetSearchQuery` | `OnSelectionChanged`, `OnComboBoxOpened`, `OnComboBoxClosed` |
| `ULunarContextMenu` | `bRestoreSelectionOnOpen` | `OpenContextMenu`, `CloseContextMenu`, `IsContextMenuOpen` | `OnContextMenuOpened`, `OnContextMenuClosed` |
| `ULunarTabs` | `TabDescriptors`, `ActiveTabId`, `PageLifetime` | `SetTabs`, `ActivateTabById`, `GetActiveTabId`, `GetPageWidgetById` | `OnActiveTabChanged` |
| `ULunarTabHeader` | `TabId`, `TabsOwner` | `GetTabId`, `GetTabsOwner` | base delegates only |

`ILunarListViewItem` functions are `GetItemNavigationId`, `IsItemNavigationEnabled`, `CanSelectItemWhenDisabled`, and `GetItemDisabledReason`.

`ILunarInputPromptReceiver` exposes only `ApplyResolvedPromptActions`. `ULunarInputIconSet` exposes `ResolveIconForKey`, and `ULunarUIActionRegistry` exposes `FindActionDefinition` and `ValidateRegistry`.

### Final Delegate Type Names

- `FLunarSelectionChangedSignature`;
- `FLunarActiveScopeChangedSignature`;
- `FLunarNavigationRejectedSignature`;
- `FLunarNavigableWidgetEventSignature`;
- `FLunarVisualStateChangedSignature`;
- `FLunarButtonClickedSignature`;
- `FLunarSliderValueChangedSignature`;
- `FLunarSliderValueCommittedSignature`;
- `FLunarOptionSliderIndexChangedSignature`;
- `FLunarSwitchChangedSignature`;
- `FLunarRadioCheckedChangedSignature`;
- `FLunarRadioGroupSelectionChangedSignature`;
- `FLunarScrollStateChangedSignature`;
- `FLunarListViewActiveItemChangedSignature`;
- `FLunarComboBoxSelectionChangedSignature`;
- `FLunarContextMenuStateChangedSignature`;
- `FLunarTabsActiveTabChangedSignature`.

The existing `FLunarInputDeviceChangedSignature` remains shared with Raw Input rather than being duplicated.

### Final Gameplay Tags

Built-in semantic actions use:

- `Lunar.UI.Action.Navigate.Up`;
- `Lunar.UI.Action.Navigate.Down`;
- `Lunar.UI.Action.Navigate.Left`;
- `Lunar.UI.Action.Navigate.Right`;
- `Lunar.UI.Action.Accept`;
- `Lunar.UI.Action.Back`;
- `Lunar.UI.Action.Increase`;
- `Lunar.UI.Action.Decrease`.

Built-in value-state tags use:

- `Lunar.UI.State.Value.Normal`;
- `Lunar.UI.State.Value.Disabled`;
- `Lunar.UI.State.Value.Checked`;
- `Lunar.UI.State.Value.Unchecked`;
- `Lunar.UI.State.Value.Active`;
- `Lunar.UI.State.Value.Inactive`;
- `Lunar.UI.State.Value.On`;
- `Lunar.UI.State.Value.Off`.

C++ native tag declarations live in the `LunarGameplayTags` namespace and use identifiers such as `UI_Action_Navigate_Up` and `UI_State_Value_Checked`. Navigation IDs, Group IDs, Item IDs, Option IDs, and Tab IDs remain `FName` values and are not Gameplay Tags.

## Settings Layout

The Lunar Settings sections are:

- Navigation Input
  - semantic action bindings;
  - keyboard navigation enabled;
  - analog thresholds and dead zone;
  - repeat delay and interval;
- Navigation Behavior
  - fallback defaults;
  - selection recovery;
  - scroll-into-view behavior;
  - cursor visibility, pointer capture, and pointer-lock defaults;
- Default Styles
  - per-control global Style Assets;
- UI Audio
  - Pointer and Navigation sounds;
- UI Haptics
  - default gamepad effects;
- Input Prompts
  - default Prompt Widget class;
  - default visibility policy;
  - Keyboard/Mouse, Xbox, and PlayStation 5 Icon Sets;
- Accessibility
  - reduced motion;
  - validation verbosity for accessible metadata;
- Diagnostics
  - validation verbosity;
  - debug overlay defaults;
  - graph dump behavior.

## Design Closure

All V1 architecture, behavior, lifecycle, input, validation, accessibility, editor integration, source layout, and public API naming decisions are resolved. Implementation has not started.

Owner-created Content asset instances and their future verified package paths are deferred by the explicit Content ownership boundary. They are integration inputs, not unresolved C++ design questions.

## Implementation Plan

### Phase 1: Types And Settings

- implement the finalized navigation, scope, link, style, sound, haptic, prompt, and icon-set declarations;
- add the finalized native Gameplay Tags and Action Registry defaults;
- add Lunar Settings sections and safe defaults;
- expand the roadmap milestone from this specification.

### Phase 2: Core Runtime

- implement the navigation subsystem;
- implement scope stack and screen lifecycle;
- integrate input routing and gameplay input consumption;
- implement registration, IDs, groups, priority, explicit links, and fallback;
- implement selection, native focus synchronization, restore, reset, and recovery.

### Phase 3: Base Widget And Button

- implement `ULunarNavigableWidget`;
- implement `ULunarScrollBox` as the reference non-selectable Lunar navigation container;
- implement style, event, sound, haptic, prompt, and scroll hooks;
- implement Button as the reference control;
- document the owner-performed reparenting and binding steps for `W_Button` without modifying the asset.

### Phase 4: Value Controls

- implement Slider;
- implement OptionSlider;
- implement Switch;
- implement Radio and Radio Group.

### Phase 5: Composite Controls

- implement ListView logical navigation, virtualization, and item restoration;
- implement ComboBox nested scope behavior;
- implement ContextMenu and submenu scopes;
- implement horizontal and vertical Tabs.

### Phase 6: Prompt And Presentation Assets

- implement the default Prompt Widget contract;
- implement the Icon Set, Style, and Action Registry Data Asset classes without creating asset instances;
- connect device switching;
- implement error placeholders and null-safe resolution for unassigned defaults;
- leave `TODO(LunarUI)` markers at every default Content connection point;
- integrate owner-created styles, sounds, haptics, prompts, and icons only in a later explicitly requested pass.

### Phase 7: Debugging, Editor Integration, And Owner Handoff

- implement scope validation and graph diagnostics;
- implement the debug overlay;
- implement editor-only Details customization and validation fixes;
- provide the code support and checklist for an owner-created example menu covering every control and nested scope;
- manually verify mouse, keyboard, Xbox, and PlayStation 5 behavior;
- verify that handled UI input does not reach gameplay;
- update roadmap, README, and Doxygen documentation; use only owner-provided screenshots and Content examples.

## Acceptance Criteria

The Lunar UI Navigation System V1 is complete when:

- all supported controls use the same selection, scope, style, prompt, sound, and haptic contracts;
- an owner-created example menu is fully operable without a mouse after Content integration;
- focus cannot become lost while eligible widgets exist;
- simultaneous local players keep independent Selection, Scope Stack, native focus, and input-device state;
- nested scopes restore parent selection correctly;
- virtualized ListView selection survives row recycling and item refresh through stable item IDs;
- explicit navigation, fallback, groups, priority, wrap, and Block rules are deterministic;
- mouse and Navigation handoff preserves the intended selection;
- keyboard and gamepad UI inputs do not trigger Pawn actions while consumed;
- Slider, OptionSlider, Switch, Radio, ScrollBox, ListView, ComboBox, ContextMenu, and Tabs follow this specification;
- prompts update when the input device changes;
- missing icon and invalid graph configuration are clearly visible and reported to the Lunar Console;
- native UMG navigation settings cannot accidentally override Lunar behavior through the normal Details workflow;
- primary touch supports direct tap and drag without becoming a second navigation graph;
- accessible metadata, native focus notifications, reduced motion, and localized `FText` contracts follow this specification;
- the UE 5.8 Win64 project builds successfully;
- the changed UI behavior is manually verified in its intended runtime flow.
