# Lunar Custom UI Navigation System Specification

Status: C++ implementation complete; owner Content verification in progress

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
- reusable state publication with fully Blueprint-owned presentation for every Lunar control;
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
- resolving shared settings, feedback, prompts, and icon sets for its local player.

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
- visual-state publication;
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
- composite Radio;
- ScrollBox;
- ListView and virtualized row presentation;
- ComboBox;
- ContextMenu;
- Tabs and Tab Header.

### Explicit Exclusion: Draggable Window

`ULunarDraggableWindow` is not part of the Lunar UI Navigation control hierarchy. Its existing C++ implementation and `W_DraggableWindow` asset remain unchanged and mouse-only.

The class must not be reparented to `ULunarNavigableWidget`, automatically registered, assigned Lunar Selection, or given navigation presentation, prompt, sound, or haptic behavior. A navigation screen may be placed visually inside a draggable window without making the window itself navigable.

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

These paths are reference inventory only. Their assets remain untouched until the owner explicitly performs the required Content changes. `W_InputPrompt`, reusable sound/haptic feedback assets, Action Registry, input Icon Sets, and any additional navigation Content do not receive guessed or predeclared package paths.

C++ default asset references remain null or empty until the owner creates the corresponding assets. A consistent `TODO(LunarUI)` comment is placed only beside a settings or constructor field that is intentionally expected to receive plugin-supplied default Content later. TODO comments are not added to every optional override or normal runtime resolution path.

Required default-asset TODO locations include:

- default Keyboard/Mouse Icon Set Data Asset;
- default Xbox Icon Set Data Asset;
- default PlayStation 5 Icon Set Data Asset;
- default Prompt Widget class if Lunar will ship one;
- default Action Registry Data Asset if Lunar will ship one;
- global sound or haptic fields only when a Lunar default asset is intended.

Optional per-widget overrides, project-only classes, and example content remain empty without TODO markers.

After the owner creates an asset, its verified package path may be added to code or settings in a separate explicitly requested pass. C++ must never use `ConstructorHelpers` or another hard reference to a nonexistent guessed asset path.

### Module Ownership

All navigation runtime behavior belongs to the existing `Lunar` Runtime module. The editor and PIE use the same runtime classes and code paths as packaged builds.

A separate `LunarEditor` Editor module contains only Details customization, editor-facing validation presentation, and automated editor fixes. It does not implement Selection, scopes, input routing, control behavior, visual presentation, prompts, or any other runtime decision.

`LunarEditor` may depend on PropertyEditor, UMGEditor, and UnrealEd. The `Lunar` Runtime module must not depend on `LunarEditor` or editor-only modules, and packaged builds exclude `LunarEditor` entirely.

### C++ Source Layout

Navigation C++ uses a feature-oriented root inside the existing runtime module:

- `Source/Lunar/Public/UI/Navigation/Core`;
- `Source/Lunar/Public/UI/Navigation/Controls`;
- `Source/Lunar/Public/UI/Navigation/Prompts`;
- `Source/Lunar/Public/UI/Navigation/Data`;
- `Source/Lunar/Public/UI/Navigation/Types`.

Runtime implementation files mirror the same feature folders under `Source/Lunar/Private/UI/Navigation`. Editor-only files use:

- `Source/LunarEditor/Private/UI/Navigation/Customizations`;
- `Source/LunarEditor/Private/UI/Navigation/Validation`.

Folder responsibilities are:

- Core: subsystem, scope, screen base, navigable-widget base, and private routing helpers;
- Controls: concrete Button, Slider, OptionSlider, Switch, Radio, ScrollBox, ListView, ComboBox, ContextMenu, Tabs, Tab Header, and related widget classes;
- Prompts: prompt receiver interface, default C++ prompt base, and prompt host behavior;
- Data: Icon Set, Action Registry, reusable sound-feedback, and reusable haptic-feedback Data Asset classes;
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
| `ULunarUISoundFeedbackAsset` | `Source/Lunar/Public/UI/Navigation/Data/LunarUISoundFeedbackAsset.h` | header-only Data Asset |
| `ULunarUIHapticFeedbackAsset` | `Source/Lunar/Public/UI/Navigation/Data/LunarUIHapticFeedbackAsset.h` | header-only Data Asset |

All other concrete controls follow the same one-class-per-file rule, for example `Controls/LunarComboBox.h`. Shared declarations are grouped by responsibility in `Types/LunarNavigationTypes.h`, `Types/LunarNavigationSettings.h`, `Types/LunarUIFeedbackTypes.h`, and `Types/LunarInputPromptTypes.h`. Private-only helpers do not become public headers merely to mirror a folder.

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
- while mouse or touch presentation is active and no explicit native-focus delegation exists, an intentional external overlay may temporarily hold native focus long enough to complete pointer press and release; Lunar Selection remains stored and authoritative;
- an external editable-text control intentionally focused by mouse or touch retains native focus and receives keyboard text, caret, commit, and cancellation input until that external text focus ends; those keyboard events do not switch presentation or dispatch Lunar actions, while gamepad input remains owned by Lunar;
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

Feedback Data Assets, icon sets, input bindings, and other default configuration may be shared, but resolving and applying them uses the state of the owning local player.

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

| Widget type | Mouse | Touch | Keyboard | Gamepad | Lunar selection |
| --- | --- | --- | --- | --- | --- |
| Interactive controls | Enabled | Enabled | Enabled | Enabled | Enabled |
| `ULunarNavigableWidget` base | Configurable | Configurable | Configurable | Configurable | Disabled |
| Screens and logical containers | Not implied | Not implied | Not implied | Not implied | Disabled |
| Prompt, GIF, Emissive, decorative content | Not implied | Not implied | Not implied | Not implied | Disabled |

`bCanReceiveLunarSelection` remains the master graph-participation switch. Four independent per-widget input switches then filter interaction by source: `Allow Mouse Input` (serialized C++ name `bCanInteractWithPointer`), `Allow Touch Input` (`bAllowTouchInput`), `Allow Keyboard Input` (`bAllowKeyboardInput`), and `Allow Gamepad Input` (`bAllowGamepadInput`). All four default to enabled so existing interactive controls keep their behavior.

Disabling Mouse removes Lunar hover, click selection, press, and activation from mouse input. Disabling Touch removes direct primary-touch selection, press, drag, and activation. Disabling Keyboard or Gamepad removes the widget from that device's physical directional targets and prevents that device from invoking the widget's semantic actions or native editing delegation. If another channel already selected the widget, a forbidden directional input searches from its geometry for the next allowed candidate; a forbidden Accept is consumed without activation or Rejected feedback, and native focus plus Navigation Selected presentation are withheld for the forbidden device. If no allowed directional target exists, authoritative Lunar Selection remains stored for the channels that may still use it. Scope-level Back behavior remains independent of these widget switches.

Programmatic selection and actions are not inferred to be mouse, touch, keyboard, or gamepad input. `RequestLunarSelection`, `SetSelectedWidget`, and device-`Unknown` action contexts therefore remain device-neutral. Prompt resolution emits an empty snapshot while the active presentation source is disabled for its owner; switching to an allowed source resolves the prompts again. Graph dumps append `Input=M:on/off T:on/off K:on/off G:on/off` for every registered widget.

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

Virtualized ListView items use a separate stable `ItemId` stored in the owner-authored inline `FLunarListViewItemData`, never in the generated row widget. Item IDs are unique within their ListView.

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
| `Lunar.UI.Action.Selection.Toggle` | contextual `Ctrl+Space` | Face Button Left (`X`/`Square`) |

Tab, Next, and Previous are not part of Lunar navigation.

`FLunarUIActionContext` also carries selection modifiers independently of the semantic action: Shift is Range and Ctrl is Additive on keyboard; Left Shoulder (`LB`/`L1`) is Range and Right Shoulder (`RB`/`R1`) is Additive on gamepad. This keeps D-pad and left-stick navigation equivalent while allowing controls such as ListView to implement multi-selection without registering the shoulder presses as fake control activations.

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

### Hold Progress

Every `ULunarNavigableWidget` exposes the same valid-press hold contract for pointer, primary touch, keyboard, gamepad, and project-defined semantic actions. `HoldStartDelay` is configured per widget and defaults to `0.0` seconds; it is not a global navigation setting.

`OnLunarHoldProgress(HoldSeconds, PressedSeconds, DelayLeft, bDelayElapsed)` begins immediately after `OnLunarPressed` and then runs once per widget tick while the press remains valid. `PressedSeconds` grows from zero for the complete physical press. `DelayLeft` starts at the non-negative local `HoldStartDelay`, counts down toward zero, and remains zero after the threshold. Before a positive local delay elapses, `HoldSeconds` remains zero and `bDelayElapsed` is false. The threshold sample reports `HoldSeconds = 0.0`, `DelayLeft = 0.0`, and `bDelayElapsed = true`; later samples grow `HoldSeconds` from zero. A zero delay reports true immediately. Disabled or rejected presses never start Hold Progress. Release, cancellation, selection loss, pointer-capture loss, destruction, or becoming effectively non-interactive stops tracking and clears all transient timing state. Hold Progress does not activate the widget or impose a completion duration; owner-authored Blueprint logic may use the time values and threshold flag to drive progress or a custom semantic action.

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

- pointer Hovered and Pressed visuals are published;
- the last Lunar Selection remains stored but its Navigation Selected presentation is hidden;
- a mouse click assigns the clicked Lunar widget as the new Lunar Selection.

When keyboard or gamepad navigation resumes:

- the stored selection becomes visible again;
- native focus is restored to it;
- Navigation presentation and prompts update to the active device.

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

Slate-classified double-click presses on a Lunar control use the same press/release contract as ordinary clicks. Lunar does not add click debounce: each completed rapid click activates exactly once unless the control itself intentionally defines a different higher-level action.

### Touch Input

Win64 V1 supports primary-touch direct interaction only.

- touch press, release, and tap use Pointer Pressed and activation behavior;
- a successful tap assigns Lunar Selection to the touched Lunar widget, while Navigation Selected presentation remains hidden during touch interaction;
- primary-touch drag is supported for controls that already implement pointer dragging, including Slider and ScrollBox;
- handled touch events are consumed before gameplay; unhandled touch follows the active scope's normal gameplay-blocking policy;
- a higher-z gameplay Touch Interface such as Unreal's default `SVirtualJoystick` must be hidden or deactivated while direct Lunar UI touch is expected, because its full-screen Slate layer owns the hit-test path before lower viewport widgets;
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

Raw `Gamepad_LeftX` and `Gamepad_LeftY` analog events exclusively own left-stick navigation dispatch, hysteresis, and repeat timing. Platform-generated `Gamepad_LeftStick_Up`, `Gamepad_LeftStick_Down`, `Gamepad_LeftStick_Left`, and `Gamepad_LeftStick_Right` mirror key events are consumed while a Lunar scope is active but must not independently dispatch the same navigation action.

For analog repeat, calculate `AccelerationAlpha = SmoothStep(0.60, 1.00, Magnitude)` and `RepeatInterval = Lerp(0.12, 0.06, AccelerationAlpha)`.

In a Slider's default Stepped stick mode, analog magnitude affects repeat speed but does not change navigation step size. A Slider may instead opt into Continuous stick mode, where magnitude scales continuous value velocity.

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

`ULunarScrollBox` derives from `UScrollBox` and is the fully supported Lunar scroll-container class for V1. It is not a `ULunarNavigableWidget`, never receives Lunar Selection, and instead registers its container capabilities with the owning local player's navigation subsystem.

One ScrollBox scrolls along exactly one `ScrollOrientation`, displayed as `Orientation` in Lunar Details: `Vertical` or `Horizontal`. The property drives both native scrolling and local navigation ownership. A two-dimensional layout uses nested ScrollBoxes of different orientations rather than a separate two-axis container.

Selection-driven reveal is configured directly on each ScrollBox:

- `ViewportPadding` keeps safe space between the selected descendant and the visible viewport edges;
- `bInterpolateScrollIntoView`, disabled by default, enables presentation-only interpolation;
- `ScrollInterpolationSpeed`, default `12.0`, controls normalized curve traversals per second and snaps when zero;
- `ScrollInterpolationCurve` defaults to `/Lunar/Curves/Float/CF_EaseOutExpo`, uses linear progress when null, and clamps invalid curve output;
- Designer Preview and global Reduce Motion always snap immediately.

Reveal resolves the selected descendant from freshly arranged Slate child geometry instead of `UWidget` cached geometry, so off-screen first/last wrap targets remain discoverable. A moving selection retargets from the currently displayed offset without waiting for the previous reveal to finish. Logical Selection changes immediately. Wheel, scrollbar, right-drag, and touch scrolling remain direct and never use selection interpolation. When wrap moves from the final descendant to the first or vice versa, reveal traverses the full offset range when interpolation is enabled and snaps when disabled.

Each ScrollBox owns optional local navigation behavior along its primary orientation:

- `bWrapNavigation`, disabled by default, wraps between the opposite eligible descendant boundaries;
- all eligible Lunar descendants participate, including widgets inside nested panels;
- local non-wrap first searches for a real forward target outside the container; if none exists, navigation stops instead of inheriting scope-level wrap on that axis;
- perpendicular directions remain normal scope navigation unless confinement is active.

`bConstrainNavigation`, disabled by default, creates a temporary container boundary without making the ScrollBox selectable. Entering any descendant through navigation or pointer selection activates the boundary and remembers the previous external selection. While active, directional and programmatic selection cannot leave the container. Mouse or touch may immediately select an eligible widget outside the boundary; that pointer selection releases confinement without restoring the previous widget and does not require Accept. An unhandled Back/Cancel releases the boundary and restores the remembered widget. `bExitConfinementOnNavigationAccept`, also disabled by default, optionally makes a handled keyboard/gamepad Accept perform the child action first and then restore the remembered widget. Pointer activation never consults this option. Keeping it disabled lets controls such as Switch change value without leaving the container. If no valid external selection was remembered, release keeps the current descendant selected but removes the boundary. Nested direct scrolling cannot chain outside an active confinement.

When Selection moves to a widget inside nested ScrollBoxes, reveal uses inner-first priority:

1. the nearest owning ScrollBox applies the minimum scroll needed to reveal the widget with its configured viewport padding;
2. Lunar waits for updated arranged geometry after the inner movement;
3. each ancestor ScrollBox repeats the minimum-scroll operation from the inside outward;
4. a new Selection invalidates the old chain and starts again from the current visible offsets;
5. processing stops when the widget is fully visible in every owning ScrollBox.

For direct scrolling, the innermost ScrollBox consumes only the portion of the delta it can apply. After it reaches its boundary, any remainder chains to the next eligible ancestor. `bAllowScrollChaining`, enabled by default, controls this behavior. When disabled, the container consumes or discards the remainder. Selection-driven inner-first reveal is independent of direct-scroll chaining.

`ULunarScrollBox` fully supports horizontal and vertical orientation, local wrap, optional navigation confinement, selection-driven reveal with optional curve interpolation, nested inner-first reveal, wheel/right-drag/touch direct scrolling, cancellation and capture release during teardown, cached native scrollbar presentation, and local-player/active-scope ownership.

A native `UScrollBox` is not the supported Lunar V1 container and does not receive the complete per-instance Lunar contract. Validation warns when eligible Lunar navigation widgets are placed in a native `UScrollBox` where full Lunar scroll behavior is expected.
## Input Prompt System

### Prompt Ownership

Each Lunar widget may enable an optional Input Prompt host.

The default visibility policy is When Lunar Selected. During navigation presentation it follows the authoritative Lunar Selection. During pointer presentation it follows the currently hovered Lunar widget, so a mouse prompt is available before click without transferring Selection; leaving the widget hides that prompt, and a successful click still transfers Selection normally. The previously selected widget's prompt remains suppressed while the pointer targets another widget. Touch presentation does not require prompt icons and therefore keeps a `WhenSelected` prompt suppressed when no pointer hover exists. Visibility remains configurable so a widget may show prompts persistently or control them manually.

### Prompt Actions

A widget supplies an array of semantic prompt actions rather than a single hard-coded key or icon.

The owner may replace or modify its requested action tags at runtime through this API:

- `SetPromptActions` replaces the full requested array;
- `AddPromptAction` adds one action if it is not already requested;
- `RemovePromptAction` removes one requested action.

Each successful change causes the subsystem to resolve and send a new complete prompt snapshot.

Blueprint inheritance requires an explicit authoring check for this array. A Widget Blueprint class default or a placed child-widget instance may serialize a local `PromptActions` override. After that happens, later C++ or parent-Blueprint changes are not guaranteed to propagate into that child or instance. When a new parent default appears stale, use **Reset to Default** on the complete `PromptActions` property or array row, then compile and save the affected Blueprint. Reset only after reviewing intentional local entries, because it replaces the complete local array with the inherited value.

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

A missing icon is a configuration error only when the request has `bRequireIcon` enabled. Strict requests do not silently fall back to text.

When `bRequireIcon` is disabled, Lunar still resolves an Icon Set entry when one exists, but a missing entry produces an empty brush without a placeholder, log entry, or validation error. Display text is independently optional, so a custom prompt receiver may render an icon, text, both, or neither. An enabled `bOverrideIcon` remains an explicit promise of a valid per-request Texture- or Material-backed brush and is validated even when `bRequireIcon` is disabled.

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

`InvalidPromptIconOverride` specifically means `bOverrideIcon` is enabled while `IconOverride` has no Texture or Material resource. Resolve it with one of these authoring paths:

1. Per-request icon: keep `bOverrideIcon` enabled and assign a valid resource-backed brush.
2. Icon Set icon: disable `bOverrideIcon`, keep `bRequireIcon` enabled, and add one unique mapping for the resolved key to the active Icon Set.
3. Presentation does not require an icon: disable both `bOverrideIcon` and `bRequireIcon`.

If the flags or action entries do not match the current parent control defaults, first review and use **Reset to Default** on `PromptActions`, then compile and save the Blueprint. Run `ValidateNavigationScope` again and start a fresh PIE session to confirm the runtime resolver and validation pass no longer emit the diagnostic.

## Visual State And Blueprint Presentation

### Layered State Model

Value state and interaction state are independent. Value examples include Normal, Checked, Active, Inactive, On, Off, Disabled, and control-specific tags. Interaction state is one of Pointer Normal, Pointer Hovered, Pointer Pressed, Navigation Normal, Navigation Selected, or Navigation Pressed. Keyboard and gamepad use the Navigation states; the active input device remains part of the published snapshot.

`ULunarNavigableWidget` owns logical availability through `bLunarEnabled`. Its underlying Slate widget remains natively enabled so Unreal does not apply `DisabledEffect` or remove the control from pointer-hover routing. Native `Is Enabled` authoring is hidden on Lunar widgets; Blueprint and C++ code use `IsLunarEnabled` and `SetLunarEnabled`.

When `bLunarEnabled` is false, the value tag is `Lunar.UI.State.Value.Disabled` while interaction continues to resolve independently. A disabled control may therefore publish Disabled + Pointer Hovered, Disabled + Navigation Selected, Disabled + Navigation Pressed, and the other valid combinations. If `bCanReceiveSelectionWhenDisabled` is true, navigation and pointer selection remain available for explanation, prompts, and rejection feedback, but activation does not emit the successful Pressed/Hold/Released/Activated lifecycle.

### Blueprint-Owned Visuals

Lunar C++ owns behavior and publishes state; the owner Blueprint owns arbitrary visual presentation and animation. There is no Lunar Style Asset hierarchy, style sheet, parent-style chain, local style override map, automatic brush/font traversal, or built-in visual transition resolver. In particular, Lunar never guesses which user-created Border, Image, TextBlock, or other nested widget should receive a field.

Every navigable control exposes `On Lunar Visual State Changed` through its C++ native hook, Blueprint override, and external multicast. The callback receives `PreviousState`, `NewState`, and `bIsDesignerPreview`; external delegates additionally receive the source widget. The first PreConstruct/Synchronize publication always occurs once with `PreviousState == NewState`, allowing a Blueprint to initialize its visuals deterministically. Later publications occur only when the effective state changes.

The Blueprint may switch on `ValueStateTag`, `InteractionState`, `InputDevice`, and `bReduceMotion`, then set any combination of user-owned widgets, start animations, or call the native-part presentation API. This is the single presentation contract for enabled, disabled, pointer, navigation, value, device, and reduced-motion combinations.

### Designer Preview

`ULunarNavigableWidget` exposes editor-only `Preview Mode` and `Preview Visual State` fields. `None` publishes the normal design-time state; `Custom` publishes the selected preview snapshot while `IsDesignTime()` is true and calls the same visual-state event with `bIsDesignerPreview = true`.

Preview fields are compiled only under `WITH_EDITORONLY_DATA`. They do not participate in runtime state, are not shipped as packaged-game behavior, and therefore cannot accidentally force a preview state in a build. Changing the preview in the UMG Designer refreshes the same Blueprint presentation logic used at runtime.

### Native Control-Part Presentation

Some complex controls own small native Slate/UMG parts that a user Blueprint cannot address directly, such as Slider track/fill/thumb, Switch track/handle, OptionSlider arrows, Radio generated Unchecked visuals and shared Checked indicator, ListView row/scrollbar styles, ComboBox arrow/popup row/scrollbar presentation, ScrollBox scrollbar presentation, and Tabs active indicator/page padding.

For most native-owned parts, controls expose three symmetric API levels:

1. atomic `Set...` and `Get...` functions for one brush, tint, size, thickness, padding, or related value;
2. part-level `Configure...` and `Get...` functions for one native part;
3. whole-control `Configure...Presentation` and `Get...Presentation` functions with explicit Blueprint pins.

There is no generic aggregate Lunar style struct. Calls update transient cached presentation and apply it immediately; the cache is reapplied after `RebuildWidget`, `SynchronizeProperties`, and widget reconstruction. The cached presentation is not a project theme. Fixed C++ defaults are minimal and neutral. Radio is the deliberate inline-authoring exception: its generated Unchecked layer and shared Checked indicator each store a six-state style set directly on the owning `ULunarRadio`, and atomic state-based Set/Get functions update those serialized sets. Controls with no required native visual part, such as a fully owner-authored Button or ContextMenu, publish state without traversing or modifying arbitrary owner content.

### State And Lifecycle Events

Every control exposes C++ virtual functions, Blueprint overrides, and external delegates for its applicable Selected, Unselected, Pressed, Hold Progress, Released, Activated, Rejected, Input Device Changed, and Visual State Changed events. Blueprint overrides implemented inside the widget use `self`; external multicast delegates pass the source object so one handler can subscribe to many instances.

### Programmatic State Changes And Notification Policy

Every public state-changing function uses `ELunarChangeNotificationPolicy`. Its default is deliberately `Notify`; `Silent` is an explicit advanced-pin choice for initialization and state restoration only.

- `Notify` performs the mutation and publishes the control's normal Blueprint owner event, external multicast delegate, feedback, and native accessibility value-change notification when an effective value actually changes.
- `Silent` performs the same validation, clamping, normalization, logical-state update, native/Blueprint presentation refresh, and accessibility snapshot refresh, but suppresses outward owner events, external delegates, feedback, and accessibility value-change announcements.
- `Silent` never means "skip the UI update". The visible control and all getters must immediately represent the restored value.
- Passing the current effective value produces no duplicate change publication in either mode.
- Keyboard, gamepad, pointer, and touch interactions always use the normal notifying path; projects cannot make real user input silent through this enum.
- Data replacement and normalization functions obey the same policy when replacing options, items, tabs, or a numeric range changes the current selection/value.

The advanced parameter defaults to `Notify` on these V1 APIs:

| Control | Programmatic state API |
| --- | --- |
| `ULunarSlider` | `SetValue`, `SetValueRange`, `CommitPreviewValue`, `CancelPreviewValue` |
| `ULunarOptionSlider` | `SetOptions`, `SetSelectedIndex` |
| `ULunarSwitch` | `SetIsOn`, `Toggle` |
| `ULunarRadio` | `SetNumOfRadioButtons`, `SetSelectedIndex`, `SetSelectedByStringValue` |
| `ULunarListView` | item mutation and active-item APIs plus `SetSelectionMode`, `SetItemSelected`, `SetSelectedItemIds`, `SelectAllItems`, `ClearSelection`, `RefreshNavigationItems` |
| `ULunarComboBox` | `SetOptions`, `SetSelectedOptionById` |
| `ULunarTabs` | `SetTabs`, `ActivateTabById` |

Saved-state restoration should explicitly select `Silent`. For example, an options menu may load its save data during Pre Construct or Construct and call `SetSelectedIndex(SavedQualityIndex, Silent)` and `SetValue(SavedVolume, Silent)`. The controls visibly restore the saved values, but the graphics/audio apply callback and save-writing callback do not execute again. A later player change uses the default `Notify` pin and publishes exactly once. Using the default `Notify` during restoration is valid when reapplying the setting is intentional, but it can otherwise repeat expensive work or rewrite the save.

Widget Blueprints compiled against an older function signature may require **Refresh Node** followed by Compile after the policy pin is added. Future stateful Lunar controls, including the deferred key selector and spin boxes, must implement this same default-Notify/explicit-Silent contract.

### Sounds

Lunar Settings provides global default sounds for Pointer Hovered, Pointer Pressed, Pointer Clicked, Pointer Rejected, Navigation Selected, Navigation Pressed, Navigation Clicked, and Navigation Rejected. Keyboard uses Navigation sounds.

Each widget event selects one of four modes:

- `Use Global` resolves the corresponding Lunar Settings entry;
- `Use Data Asset` resolves the corresponding entry from the widget's `ULunarUISoundFeedbackAsset`;
- `Custom` uses the event's local `FLunarUISoundSpec`;
- `Disabled` intentionally produces no sound.

`FLunarUISoundSpec` contains `Sound`, volume multiplier, pitch multiplier, and optional Sound Concurrency. One `ULunarUISoundFeedbackAsset` stores the complete pointer and navigation sound set and may be assigned to any number of controls, allowing a group of controls to share one editable source.

`Use Data Asset` never falls back to global feedback. If the widget has no sound asset or the selected asset entry has no Sound, runtime is silent and `ValidateNavigationScope` emits an actionable warning naming the widget and event.

Widgets request semantic sound feedback but never create or own playback components. The owning local player's `ULunarNavigationSubsystem` creates non-spatial UI `UAudioComponent` instances with auto-destroy. UI sounds may overlap; an assigned Sound Concurrency asset controls overlap and suppression. Active owned components are stopped when the subsystem deinitializes.

### Haptics

Gamepad haptic feedback supports Navigation Selected, Pressed, Clicked, and Rejected. Each event uses the same four modes: `Use Global`, `Use Data Asset`, `Custom`, or `Disabled`.

`ULunarUIHapticFeedbackAsset` stores the complete navigation haptic set; each entry is an `FLunarUIHapticSpec` containing a `UForceFeedbackEffect`. Missing Data Asset/entry behavior is silent with a validation warning and no global fallback.

Playback uses non-looping `FForceFeedbackParameters` with `bPlayWhilePaused = true`, `bIgnoreTimeDilation = true`, and one stable Lunar UI playback tag. The owning local player's subsystem sends force feedback only to that player's `APlayerController`. A new Lunar UI effect replaces the previous effect on the same channel. Active Lunar UI force feedback stops when that player's scope stack becomes empty or the subsystem deinitializes.

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
- keyboard, D-pad, WASD, and arrows always use the discrete navigation step settings;
- the left stick defaults to `Stepped`, which uses the same navigation step and repeat timing;
- `Continuous` stick mode changes the value every frame according to stick magnitude and `ContinuousStickRangePerSecond`; repeat timing does not quantize that motion;
- each Slider may override repeat timing used by discrete input.

Slider navigation step modes are:

- Absolute: add a fixed numeric amount;
- Percentage: add a percentage of `MaxValue - MinValue`.

Discrete values are clamped, and the final step must reach the exact range boundary. Pointer input has an independent absolute `PointerStepSize`: zero keeps mouse/touch projection continuous, while a positive value snaps click and drag to a grid anchored at `MinValue`. Both exact range endpoints remain reachable even when the range is not divisible by the pointer step.

Optional visual value interpolation is shared by every value source: keyboard and D-pad steps, Stepped or Continuous stick input, mouse/touch projection, and programmatic `SetValue`. `bInterpolateValueChanges` defaults to false, preserving immediate presentation. When enabled, `ValueInterpolationSpeed` controls normalized curve traversals per second; zero speed applies the target immediately. `ValueInterpolationCurve` maps normalized progress to presentation alpha, falls back to linear when null, and defaults to `/Lunar/Curves/Float/CF_EaseOutExpo` for an `FInterpTo`-like ease-out response. A moving target restarts the curve from the current displayed value so continuous stick and pointer drag remain smooth. The committed value, preview target, commit/cancel rules, accessibility value, and their existing delegates remain exact and immediate. Native Fill/Thumb rendering reads `GetDisplayedValue`, while owner-authored Blueprint presentation can bind `OnDisplayedValueChanged(Slider, DisplayedValue)`. Designer presentation always snaps to the logical target instead of running a runtime interpolation.

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

`bWrapOptions` is available and disabled by default. Without wrap, attempts beyond the first or last option keep the widget selected and unchanged, consume the value-axis action, and publish one rejected interaction for a new press. A held repeat that has already reached the boundary remains consumed without repeatedly publishing rejection. An Increase or Decrease press against an empty option list follows the same rejected contract.

Pointer and touch activation use the native Previous and Next regions. For a horizontal control, the outer left quarter decrements and the outer right quarter increments. For a vertical control, the outer top quarter decrements and the outer bottom quarter increments. Press and release must resolve to the same region; releasing over the middle, outside the control, or over the opposite region does not change the option. A successful pointer step uses the same clamp/wrap rules and notifying change path as keyboard and gamepad input. A same-region release that requests an unavailable boundary step publishes rejection instead of generic activation.

### Switch

Accept always toggles the Switch.

Directional value control uses a mode rather than a boolean:

| Mode | Value controls | Navigation controls |
| --- | --- | --- |
| Disabled | Accept only | All directions |
| Horizontal | Left = Off, Right = On | Up and Down |
| Vertical | Up and Down set the value | Left and Right |

Horizontal is the default mode and is intended for a Switch inside a vertical menu. Vertical mode supports a horizontal menu layout.

A directional press first changes the value when it requests the opposite state. A later, separate press in the same direction after that state is already active is not consumed as a value action and falls through to normal directional navigation. Repeats generated while the original value-changing press remains held stay consumed until release, so holding one direction cannot unexpectedly move selection away from the Switch.

Optional Handle interpolation is presentation-only and applies to Accept, configured directional input, pointer/touch activation, and programmatic `SetIsOn`. `bInterpolateHandleMovement` defaults to false. When enabled, `HandleInterpolationSpeed` controls normalized curve traversals per second; zero speed applies the target immediately. `HandleInterpolationCurve` maps normalized progress to presentation alpha, falls back to linear when null, and defaults to `/Lunar/Curves/Float/CF_EaseOutExpo`. A moving target restarts from the current displayed position. The logical `bIsOn` value, accessibility value, and `OnSwitchChanged` remain exact and immediate. Native Handle rendering reads `GetDisplayedHandleAlpha` (`0` = Off, `1` = On), while owner-authored animation can bind `OnDisplayedHandleAlphaChanged(Switch, DisplayedHandleAlpha)`. Designer presentation and global Reduce Motion always snap to the logical target.

### Radio

`ULunarRadio` is one composite navigable control that generates and owns all of its internal options. The composite receives one outer Lunar Selection; generated options and optional side visuals never register as independent navigation targets. Selection is always required.

- `NumOfRadioButtons` is authoritative, is clamped to at least one, and defaults to three. `SelectedIndex` defaults to zero and is always clamped into the generated range.
- `SideVisualData` is resized with the count. Existing entries are preserved, new entries receive `Radio_1`, `Radio_2`, and later automatic `StringValue` IDs with culture-invariant empty `DisplayText`, and removed tail entries are discarded. Every technical `StringValue` must be non-empty and unique.
- The configured `Orientation` axis changes `SelectedIndex` immediately without Accept. A perpendicular direction exits normally. At a no-wrap edge, a later separate press may exit to normal navigation, while repeats from the held press that performed an internal change remain consumed until release. `bWrapSelection` cycles the internal selection instead.
- Pointer hover and press resolve one generated option independently. Mouse or touch changes selection only after press and release resolve to the same option. The indicator, optional side visual, and their internal spacing form the option hit region; `ButtonSpacing` between options does not.
- Every option retains a fixed Unchecked visual. One shared Checked indicator physically moves between option coordinates. `UncheckedStyles` and `CheckedIndicatorStyles` each contain all six Pointer and Navigation interaction styles with Brush, Tint, Size, and `FWidgetTransform`.
- Optional `bInterpolateSelectionMovement` is presentation-only and defaults to false. `SelectionInterpolationSpeed` defaults to 12 normalized option traversals per second; zero snaps. `SelectionInterpolationCurve` defaults to `/Lunar/Curves/Float/CF_EaseOutExpo` and null is linear. A new target restarts from the displayed coordinate. Designer preview and Reduce Motion snap immediately.
- A wrapped last-to-first or first-to-last transition fades the shared Checked indicator out, teleports it while hidden, then fades it back in using the same speed and curve.
- Logical selection and `OnSelectedIndexChanged` publish immediately. `GetDisplayedSelectionPosition` and `OnDisplayedSelectionPositionChanged` expose the presentation coordinate independently. `Silent` suppresses logical outward notification but still refreshes and animates presentation.
- Runtime count changes rebuild generated options and snap presentation; they publish selection only if clamping changes `SelectedIndex`.

`SideVisualClass` optionally supplies one `ULunarRadioSideVisual` instance per option. `SideVisualPlacement` supports all nine positions around the indicator and defaults to `CenterRight`; `SideVisualSpacing` defaults to `(8, 0)`. `Center` overlays the indicator. The Radio assigns owner, index, data, visual state, and checked state before PreConstruct/Construct. Runtime `SetSideVisualDataAt` updates the existing instance without recreating it.

### ListView

`ULunarListView` is a composite navigation owner. The ListView itself is the single navigable widget registered in its containing scope and remains the outer Lunar Selection target while the player moves between its logical items. Virtualized rows never enter the outer graph.

`Items` is an inline array of `FLunarListViewItemData`. Each descriptor contains `ItemId`, `FText` display text, enabled state, disabled-selection policy, and an `FText` disabled reason. Both editable text fields default culture-invariant and may be made localizable explicitly by the author. Ordinary Blueprint setup therefore does not construct UObject item adapters or implement an item interface.

`SelectionMode` is `Single` by default and may be changed to `Multi`:

- `ActiveItemId` is always the internal navigation cursor and the item revealed by scrolling;
- `SelectedItemIds` is the stable-ID selected set, retained in logical item order;
- in `Single`, the active item is the sole selected item, preserving the original ListView behavior;
- in `Multi`, active and selected state are independent, so the user can move the cursor without destroying an existing selected set.

Multi-selection follows Explorer conventions. "Axis" means `Up/Down` for a vertical ListView and `Left/Right` for a horizontal ListView. An unhandled perpendicular direction leaves the composite ListView through its normal Lunar navigation link or geometric fallback.

Mouse controls:

| Input | Result |
| --- | --- |
| hover | publish pointer presentation for the row without changing Active or Selected |
| left click | move Active and replace Selected with the target item |
| `Ctrl+left click` | move Active and toggle the target item in Selected |
| `Shift+left click` | move Active and replace Selected with the eligible range from the stable anchor |
| `Ctrl+Shift+left click` | move Active and add the anchored eligible range to Selected |
| left double click | perform the plain-click selection first, then activate the target on the second release |
| mouse wheel or native scrollbar | scroll directly without changing Active or Selected |

Keyboard controls:

| Input | Result |
| --- | --- |
| plain axis key | move Active and replace Selected with the target item |
| `Ctrl+axis` | move Active only |
| `Shift+axis` | move Active and replace Selected with the eligible range from the stable anchor |
| `Ctrl+Shift+axis` | move Active and add the anchored eligible range to Selected |
| `Ctrl+A` | select every eligible item without moving Active |
| `Ctrl+Space` | toggle the active item in Selected |
| `Enter` | activate the active item without implicitly toggling its selection |

Gamepad controls:

| Input | Result |
| --- | --- |
| plain D-pad or left-stick axis | move Active and replace Selected with the target item |
| `RB/R1 + axis` | move Active only |
| `LB/L1 + axis` | move Active and replace Selected with the eligible range from the stable anchor |
| `LB/L1 + RB/R1 + axis` | move Active and add the anchored eligible range to Selected |
| `X/Square` | toggle the active item in Selected |
| `A/Cross` | activate the active item without implicitly toggling its selection |

Primary touch selects one item. In Single mode Active and Selected remain coupled, axis navigation moves both together, `Enter` or `A/Cross` activates the active item, and the existing same-item click/tap activation behavior remains unchanged.

`EntryWidgetClass` optionally selects an owner-authored, concrete Blueprint derived from `ULunarListViewEntry`. The class is non-navigable and generated only for visible rows; it receives owner, logical index, item data, visual state, `bIsActiveItem`, and `bIsSelectedItem` before its normal construct lifecycle. Active drives the navigation cursor, `State.InteractionState` drives Normal/Hovered/Pressed presentation, and Selected drives an independent persistent multi-selection marker. An Entry Blueprint must route `On List View Item Visual State Changed.New Is Selected Item` into its visual update function; listening to the owner ListView's aggregate `OnSelectedItemsChanged` delegate alone does not style individual rows. Prefer separate Active and Selected overlay widgets so the interaction-state background cannot overwrite persistent selection. Recycled or off-screen entries are presentation proxies only. With no valid concrete class assigned, the native fallback displays `DisplayText`, or `ItemId` when text is empty, and the Slate row style reflects `SelectedItemIds`.

Every generated Entry publishes its initial `On List View Item Data Changed` and `On List View Item Visual State Changed(State, IsActiveItem, IsSelectedItem)` snapshots once even when the preassigned and current snapshots are equal. Existing Entry Blueprints must Refresh Node and Compile after upgrading to the three-parameter visual-state event.

`SetSelectionMode`, `GetSelectedItemIds`, `GetSelectedItems`, `IsItemSelected`, `SetItemSelected`, `SetSelectedItemIds`, `SelectAllItems`, and `ClearSelection` expose the selected set to Blueprint. `OnSelectedItemsChanged` and `On Lunar Selected Items Changed` publish previous and new stable-ID arrays once per logical mutation. `Silent` updates row and Entry presentation without publishing those owner notifications.

`AddItem` and atomic `AddItems` reject empty or duplicate IDs without changing the list. `RemoveItem` removes by stable ID, `RemoveItems` removes every matching stable ID in one refresh, and `ClearItems` removes all logical data. Every mutation accepts the standard default-Notify/explicit-Silent policy. Active restoration remains ID-first with previous-index fallback; Multi selection independently retains every still-valid eligible selected ID, while Single normalizes selection to the restored active item. Removed or newly ineligible IDs leave the selected set deterministically.

In the UMG Designer, final item validation and row generation run after the owner Blueprint `PreConstruct`. Transient serialized or earlier preview data therefore cannot report stale duplicate-ID errors before a design-time `SetItems`, Add/Remove/Clear sequence, or equivalent owner initialization has established the effective preview array.

Editor-only `NumDesignerPreviewEntries` defaults to `5` and generates `Preview Item 1...N` dummy rows only in the UMG Designer. It is clamped to `0...20`; zero disables dummy data and previews the authored `Items` array. Dummy preview data never changes the saved runtime array and is compiled out of packaged behavior.

Each descriptor must provide a stable, non-empty `ItemId` that is unique within the ListView. Missing and duplicate item IDs are configuration errors reported to the Lunar Console.

Navigation along the ListView orientation moves through eligible data items in logical order rather than using row-widget geometry. Navigation perpendicular to the ListView orientation uses the ListView's normal explicit links or geometric fallback to leave the composite control.

When an off-screen item becomes active:

1. the logical `ActiveItemId` updates;
2. row selection synchronizes from `SelectedItemIds` without replacing the active cursor;
3. the ListView requests scrolling to the active item;
4. the ListView waits for its row proxy to be generated;
5. the generated proxy receives independent active and selected snapshots;
6. native focus remains on the `ULunarListView` owner throughout the transition.

Every logical move requests `ScrollIntoView`, including when Slate has already generated an overscan row outside the visible viewport. This prevents delayed jumps at the virtualization-buffer boundary. Rows remain paintable but do not intercept pointer press/release; the owner ListView resolves hover, same-row press/release, double click, and touch from generated-row geometry. Wheel scrolling and clamping remain native to `SListView`, consume input only while scrolling is possible, and chain at an edge; the native scrollbar remains interactive.

After refresh or row regeneration, restoration first searches for the stored `ActiveItemId`. If that item no longer exists or is ineligible, restoration starts at the clamped previous logical index, searches forward for the first eligible item, and then searches backward. If no eligible item exists, `ActiveItemId` becomes empty while the ListView may remain Lunar-selected; activation then produces `Rejected` feedback. Multi selection may still be empty while a valid active cursor exists.
### ComboBox

`ULunarComboBox` is self-contained. The owner places one control in the screen and supplies `Options`; the control generates its closed face, popup, virtualized internal `ULunarListView`, and nested `ULunarNavigationScope`. An owner Blueprint never has to create or bind a popup root or ListView.

Opening the option list pushes exactly one nested scope.

- temporary selection starts at the committed value or the first eligible filtered option;
- navigation changes only the temporary selection;
- Accept or same-row pointer release commits the temporary option and closes;
- Back, closed-face reactivation, and outside click close without committing;
- vertical and horizontal popup orientations use their matching axis;
- local boundary wrapping occurs only when `bWrapNavigation` is enabled.

Every option has a stable unique `OptionId`, `FText` display text that defaults culture-invariant, enabled state, disabled-selection policy, a non-localized `FString` disabled reason, and optional payload. Disabled options remain visible and may be highlighted only when `bCanReceiveSelectionWhenDisabled` allows it, but they are never committable. Rejected activation keeps the popup open.

The closed value, each popup row, and the no-results view have optional custom Blueprint classes: `ULunarComboBoxSelectedVisual`, `ULunarComboBoxEntry`, and `ULunarComboBoxEmptyVisual`. Every class may remain null. Null uses the ComboBox's inline closed, arrow, popup, row, scrollbar, text, padding, and empty-result styles, so the control is functional without auxiliary assets. `ULunarComboBoxEntry` exposes `ELunarComboBoxEntryVisualState` as the exhaustive eight-way Enabled/Disabled, Highlighted, and Committed combination while retaining the three independent Boolean outputs and the separate `FLunarUIVisualState` interaction layer.

`PopupPlacement` supports Auto plus Below/Above Left/Center/Right. Width may follow the closed face or use an explicit override; maximum width and height constrain the popup and leave overflow to the internal ListView scrolling. Designer Preview may show the popup inline without creating a runtime scope.

Filtering is an external API, not built-in search UI. `SetFilterText`, `ClearFilter`, `RefreshFilter`, and the overridable `DoesOptionMatchFilter` let an owner-created text field or any other system drive filtering. The default predicate performs a case-insensitive substring match against display text and technical Option ID. Closing clears the filter by default; `bPreserveFilterText` opts into restoration.

Filtering never mutates `Options` or the committed value. It preserves the same temporary Option ID while visible and eligible, otherwise chooses the first eligible result. No visible eligible result produces the optional empty-results visual and makes Accept reject.

Empty committed selection is forbidden by default. When `bAllowEmptySelection` is enabled, `SelectedOptionId=None` presents `PlaceholderText`. When a committed option disappears or becomes disabled, Auto recovery searches near the previous logical index; ForceIndex searches near `ForcedSelectionIndex`.

Optional popup interpolation affects presentation only. Logical scope/open/close state changes immediately, while opacity and directional offset use the configured speed and optional normalized curve. Designer Preview and Reduce Motion snap without interpolation.

Missing or duplicate Option IDs are configuration errors. Culture changes rebuild derived filtering and accessible value text. Refresh and row regeneration preserve state by stable ID before index fallback.

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

Every navigable Lunar control exposes non-localized `FString` fields for an accessible name, optional description, and disabled reason, keeping this authoring metadata out of the Localization Dashboard. Value controls also expose their localized current value or checked state. Lunar Selection changes are mirrored to native accessible focus, while delegated text controls are announced as the active editable child.

Controls emit native accessibility notifications for focus, activation, value changes, checked-state changes, and relevant disabled feedback. Prompt icons and decorative content are not the only source of action meaning and are excluded from the accessible tree by default unless a project explicitly supplies accessible text.

A missing accessible name on an eligible navigable control produces a validation Warning. It does not prevent runtime navigation.

Accessibility settings include a reduced-motion option. Project-specific Blueprint presentation receives `bReduceMotion` through the visual-state context and remains responsible for applying an immediate or reduced animation path.

Built-in runtime messages that must follow the active culture use `FText` and `LOCTEXT`, String Tables, or equivalent localized resources. Every editable `FText` exposed by a Lunar widget or its public option, item, or prompt structures defaults culture-invariant, including empty values; non-empty C++ defaults and Designer-preview text use `INVTEXT` or `FText::AsCultureInvariant`. Authors may explicitly enable localization per field in Unreal. Technical IDs, Gameplay Tags, and `FString` accessibility or disabled-reason metadata remain non-localized.

Runtime culture changes invalidate active prompt text, ComboBox search results, accessible text, and other derived display data. Default search uses culture-aware case folding. Slate owns shaping and right-to-left text flow. Lunar does not automatically mirror explicit directional links; geometric fallback continues to represent physical screen directions.

## Editor Integration

Native UMG navigation and focus fields are hidden for Lunar navigation widgets through the `LunarEditor` editor-only module. The `Lunar` runtime module must not acquire PropertyEditor, UMGEditor, or UnrealEd dependencies.

The editor module registers `IDetailCustomization` handlers through `FPropertyEditorModule` for Lunar navigable widget classes and their Blueprint subclasses. The customization:

- hides inherited `UWidget::Navigation` configuration;
- hides native focusability fields whose values are controlled internally by Lunar;
- leaves those fields unchanged for non-Lunar widgets;
- exposes the Lunar Selection, links, groups, scope, preview, prompt, sound, and haptic properties in Lunar categories;
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

- `ULunarInputIconSet`, the input-icon Data Asset class;
- `ULunarUIActionRegistry`, the semantic-action Data Asset class;
- `ULunarUISoundFeedbackAsset`, the reusable complete UI sound set;
- `ULunarUIHapticFeedbackAsset`, the reusable complete UI haptic set;
- `ULunarInputPromptReceiver` and `ILunarInputPromptReceiver`;
- `ULunarInputPromptWidget`, the default abstract C++ prompt widget base;
- `ULunarRadioSideVisual`, the optional abstract non-navigable Blueprint presentation generated once per Radio option;
- `ULunarListViewEntry`, the optional abstract non-navigable Blueprint presentation generated only for visible ListView rows.

Icon Set, Action Registry, Sound Feedback, and Haptic Feedback classes derive from `UDataAsset`. `ULunarInputPromptWidget`, `ULunarRadioSideVisual`, and `ULunarListViewEntry` derive from `UUserWidget`. Visual controls derive from `ULunarNavigableWidget` except `ULunarScrollBox`, which derives from `UScrollBox`; generated Radio side visuals and ListView entries do not participate in navigation.

### Final Enums

| Enum | Values |
| --- | --- |
| `ELunarNavigationDirection` | `Up`, `Down`, `Left`, `Right` |
| `ELunarNavigationLinkMode` | `Automatic`, `Widget`, `NavigationId`, `Block` |
| `ELunarUIInteractionState` | `PointerNormal`, `PointerHovered`, `PointerPressed`, `NavigationNormal`, `NavigationSelected`, `NavigationPressed` |
| `ELunarUIActionResult` | `Unhandled`, `Handled`, `Rejected` |
| `ELunarChangeNotificationPolicy` | `Notify`, `Silent` |
| `ELunarPromptVisibilityPolicy` | `WhenSelected`, `Always`, `Manual` |
| `ELunarFeedbackOverrideMode` | `UseGlobal`, `UseDataAsset`, `Disabled`, `Custom` |
| `ELunarVisualStatePreviewMode` | `None`, `Custom` |
| `ELunarListViewSelectionMode` | `Single`, `Multi` |
| `ELunarComboBoxEntryVisualState` | `Enabled`, `EnabledHighlighted`, `EnabledCommitted`, `EnabledHighlightedCommitted`, `Disabled`, `DisabledHighlighted`, `DisabledCommitted`, `DisabledHighlightedCommitted` |

| `ELunarCursorVisibilityPolicy` | `Inherit`, `AlwaysVisible`, `AutoHideOnNavigation`, `AlwaysHidden` |
| `ELunarPointerCapturePolicy` | `Inherit`, `Release`, `Preserve` |
| `ELunarSliderStepMode` | `Absolute`, `Percentage` |
| `ELunarSliderStickInputMode` | `Stepped`, `Continuous` |
| `ELunarSliderCommitMode` | `Immediate`, `OnAccept` |
| `ELunarSwitchDirectionMode` | `Disabled`, `Horizontal`, `Vertical` |
| `ELunarRadioSideVisualPlacement` | `TopLeft`, `TopCenter`, `TopRight`, `CenterLeft`, `Center`, `CenterRight`, `BottomLeft`, `BottomCenter`, `BottomRight` |
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

| `FLunarPointerPolicy` | `CursorVisibilityPolicy`, `PointerCapturePolicy` |
| `FLunarNavigationScopeSettings` | `InitialSelectionWidget`, `InitialSelectionId`, `bRestoreLastSelection`, `bWrapHorizontal`, `bWrapVertical`, `bBlockAllGameplayInput`, `PointerPolicy`, `NavigationGroups` |
| `FLunarUIActionBinding` | `Key`, `bEnabled` |
| `FLunarUIActionDefinition` | `ActionTag`, `Bindings`, `DisplayText` |
| `FLunarUIActionContext` | `ActionTag`, `Key`, `InputDevice`, `InputEvent`, `bHasNavigationDirection`, `NavigationDirection`, `bIsRepeat`, `AnalogMagnitude`, `LocalPlayer` |
| `FLunarPromptActionRequest` | `ActionTag`, `PreferredKey`, `DisplayTextOverride`, `bRequireIcon`, `bOverrideIcon`, `IconOverride`, `bEnabled` |
| `FLunarResolvedPromptAction` | `ActionTag`, `InputDevice`, `ResolvedKey`, `IconSet`, `Icon`, `DisplayText`, `OwnerWidget`, `bSelected`, `bEnabled` |
| `FLunarInputIconEntry` | `InputKey`, `Icon` |
| `FLunarUIVisualState` | `ValueStateTag`, `InteractionState`, `InputDevice`, `bReduceMotion` |
| `FLunarUISoundSpec` | `Sound`, `VolumeMultiplier`, `PitchMultiplier`, `Concurrency` |
| `FLunarUIHapticSpec` | `Effect` |
| `FLunarUISoundOverride` | `Mode`, `CustomSound` |
| `FLunarUIHapticOverride` | `Mode`, `CustomHaptic` |
| `FLunarUISoundFeedbackSet` | the complete reusable Pointer and Navigation sound set |
| `FLunarUISoundOverrides` | named Pointer and Navigation sound-event overrides |
| `FLunarUIHapticFeedbackSet` | the complete reusable Navigation haptic set |
| `FLunarUIHapticOverrides` | named Navigation haptic-event overrides |
| `FLunarRadioSideVisualData` | technical `StringValue` plus culture-invariant-by-default `FText DisplayText` for one generated option |
| `FLunarRadioVisualStyle` | `Brush`, `Tint`, `Size`, `Transform` for one Radio interaction state |
| `FLunarRadioInteractionStyleSet` | Pointer Normal/Hovered/Pressed plus Navigation Normal/Selected/Pressed styles |
| `FLunarComboBoxOption` | `OptionId`, `DisplayText`, `bEnabled`, `bCanReceiveSelectionWhenDisabled`, `DisabledReason`, `Payload` |
| `FLunarTabDescriptor` | `TabId`, `HeaderWidgetClass`, `PageWidgetClass`, `PageWidgetInstance`, `bEnabled`, `DisabledReason` |
| `FLunarNavigationValidationMessage` | `Verbosity`, `Code`, `Message`, `OwnerPath` |

### Final Settings Structs And Properties

`ULunarSettings` exposes one property named `Navigation` of type `FLunarNavigationSettings`.

`FLunarNavigationSettings` contains:

- `Input` of type `FLunarNavigationInputSettings`;
- `Behavior` of type `FLunarNavigationBehaviorSettings`;
- `Audio` of type `FLunarUIAudioSettings`;
- `Haptics` of type `FLunarUIHapticSettings`;
- `Prompts` of type `FLunarInputPromptSettings`;
- `Accessibility` of type `FLunarUIAccessibilitySettings`;
- `Diagnostics` of type `FLunarNavigationDiagnosticsSettings`.

| Settings struct | Final fields |
| --- | --- |
| `FLunarNavigationInputSettings` | `ActionDefinitions`, `DefaultActionRegistry`, `bEnableKeyboardNavigation`, `RepeatSettings`, `AnalogSettings` |
| `FLunarNavigationBehaviorSettings` | `bEnableGeometricFallback`, fallback weights, selection recovery, default scrolling, pointer policy, and scroll chaining |
| `FLunarUIAudioSettings` | global Pointer and Navigation sound entries |
| `FLunarUIHapticSettings` | global Navigation haptic entries |
| `FLunarInputPromptSettings` | default Prompt Widget class, visibility policy, and Keyboard/Mouse, Xbox, and PlayStation 5 Icon Sets |
| `FLunarUIAccessibilitySettings` | `bReduceMotion`, `bValidateAccessibleNames` |
| `FLunarNavigationDiagnosticsSettings` | validation level, debug-overlay default, and graph-dump behavior |

The default Content reference fields are `DefaultKeyboardMouseIconSet`, `DefaultXboxIconSet`, `DefaultPlayStation5IconSet`, `DefaultPromptWidgetClass`, and `DefaultActionRegistry`. Global audio and haptic entries remain null until the owner verifies the matching assets. Reusable per-widget Sound/Haptic Feedback Data Assets are assigned on widgets and are never guessed by C++.

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

`ULunarScreenWidget` properties are `NavigationScopeSettings`, `bAutoActivateNavigationScope`, and read-only `NavigationScope`. Its functions are `OpenScreen`, `CloseScreen`, `IsScreenOpen`, and `GetNavigationScope`; its delegates identify the source screen.

`ULunarNavigableWidget` configuration properties are:

- selection, independent Mouse/Touch/Keyboard/Gamepad input switches, logical enabled, disabled-selection, hold-delay, ID, group, priority, link, and scope fields;
- editor-only `PreviewMode` and `PreviewVisualState`;
- prompt class, actions, visibility policy, and optional Icon Set override;
- optional `SoundFeedbackAsset` and `HapticFeedbackAsset`;
- per-event `SoundOverrides` and `HapticOverrides`;
- scroll-into-view and accessibility metadata.

Its public behavior functions include selection requests, `IsMouseInputAllowed`, `IsTouchInputAllowed`, `IsKeyboardInputAllowed`, `IsGamepadInputAllowed`, `IsNavigationInputAllowed`, logical enabled state, activation/action handling, visual-state refresh, prompt-action mutation, and native-focus delegation. There is no public `SetStyleAsset`: arbitrary owner visuals are controlled by the visual-state event, while complex native-owned parts use the concrete control's presentation setters/getters.

Common multicast delegates are `OnLunarSelected`, `OnLunarUnselected`, `OnLunarPressed`, `OnLunarHoldProgress`, `OnLunarReleased`, `OnLunarActivated`, `OnLunarRejected`, `OnLunarInputDeviceChanged`, and `OnLunarVisualStateChanged`. Every external multicast passes its source object first. Blueprint overrides implemented inside the emitting widget omit that redundant source because `self` is authoritative.

`OnLunarVisualStateChanged` and the corresponding native/Blueprint hooks receive Previous State, New State, and Designer Preview. The initial publication is guaranteed even when Previous equals New. `ULunarButton` additionally exposes the `On Lunar Clicked` Blueprint override; its `OnClicked(ClickedButton)` multicast identifies the source for external observers.

Action dispatch enters the non-overridable public wrappers `CanHandleLunarAction` and `HandleLunarAction`, which call the native and Blueprint override chain once per event.

### Final Control API

| Class | Final primary properties | Final primary functions | Final delegates |
| --- | --- | --- | --- |
| `ULunarButton` | base properties only | `Click`; Blueprint override `On Lunar Clicked` | `OnClicked(ClickedButton)` |
| `ULunarSlider` | `MinValue`, `MaxValue`, `Value`, `StepMode`, `StepSize`, `StickInputMode`, `ContinuousStickRangePerSecond`, `PointerStepSize`, `bInterpolateValueChanges`, `ValueInterpolationSpeed`, `ValueInterpolationCurve`, `Orientation`, `bInvertValueDirection`, `CommitMode`, `bOverrideRepeatSettings`, `RepeatSettingsOverride` | `SetValue`, `GetValue`, `SetValueRange`, `GetPreviewValue`, `GetDisplayedValue`, `CommitPreviewValue`, `CancelPreviewValue` | `OnValueChanged(Slider, NewValue)`, `OnPreviewValueChanged(Slider, NewValue)`, `OnDisplayedValueChanged(Slider, DisplayedValue)`, `OnValueCommitted(Slider, CommittedValue)` |
| `ULunarOptionSlider` | `Options`, `SelectedIndex`, `bWrapOptions`, `Orientation`, `bInvertValueDirection` | `SetOptions`, `SetSelectedIndex`, `GetSelectedIndex`, `GetSelectedOption` | `OnSelectedIndexChanged(OptionSlider, NewIndex, NewOption)` |
| `ULunarSwitch` | `bIsOn`, `bInterpolateHandleMovement`, `HandleInterpolationSpeed`, `HandleInterpolationCurve`, `DirectionMode` | `SetIsOn`, `IsOn`, `IsOff`, `Toggle`, `GetDisplayedHandleAlpha` | `OnSwitchChanged(Switch, bIsOn)`, `OnDisplayedHandleAlphaChanged(Switch, DisplayedHandleAlpha)` |
| `ULunarRadio` | `NumOfRadioButtons`, `SelectedIndex`, `bWrapSelection`, `Orientation`, `ButtonSize`, `ButtonSpacing`, `UncheckedStyles`, `CheckedIndicatorStyles`, `SideVisualClass`, `SideVisualPlacement`, `SideVisualSpacing`, `SideVisualData`, `bInterpolateSelectionMovement`, `SelectionInterpolationSpeed`, `SelectionInterpolationCurve` | `SetNumOfRadioButtons`, `GetNumOfRadioButtons`, `SetSelectedIndex`, `GetSelectedIndex`, `IsIndexSelected`, `SetSelectedByStringValue`, `GetSelectedStringValue`, `GetSelectedData`, `SetSideVisualDataAt`, `GetSideVisualDataAt`, `GetSideVisualAt`, `GetDisplayedSelectionPosition`, state-based Unchecked/Checked style Set/Get | `OnSelectedIndexChanged(Radio, PreviousIndex, NewIndex, SelectedData)`, `OnDisplayedSelectionPositionChanged(Radio, DisplayedSelectionPosition)` |
| `ULunarRadioSideVisual` | `OptionIndex`, `OptionData`, `OwningRadio`, `VisualState`, `bIsChecked` | corresponding getters | Blueprint events `On Radio Option Data Changed` and `On Radio Option Visual State Changed` |
| `ULunarScrollBox` | `ScrollOrientation` (displayed as `Orientation`), `ViewportPadding`, `bWrapNavigation`, `bConstrainNavigation`, `bExitConfinementOnNavigationAccept`, `bAllowScrollChaining`, `bInterpolateScrollIntoView`, `ScrollInterpolationSpeed`, `ScrollInterpolationCurve` | `ScrollWidgetIntoLunarView`, `CancelLunarScroll`, `IsLunarScrollActive` plus cached Scrollbar presentation Set/Get/Configure/Get | `OnLunarScrollStarted(ScrollBox)`, `OnLunarScrollFinished(ScrollBox)` |
| `ULunarListView` | inline `Items`, optional concrete `EntryWidgetClass`, editor-only `NumDesignerPreviewEntries`, `ActiveItemId`, `SelectionMode`, `SelectedItemIds`, `Orientation` | item mutation/active APIs plus `SetSelectionMode`, `GetSelectedItemIds`, `GetSelectedItems`, `IsItemSelected`, `SetItemSelected`, `SetSelectedItemIds`, `SelectAllItems`, `ClearSelection`, `RefreshNavigationItems`, `GetGeneratedEntryAt` | `OnActiveItemChanged(ListView, PreviousItemId, NewItemId)`, `OnSelectedItemsChanged(ListView, PreviousSelectedItemIds, NewSelectedItemIds)`, `OnItemActivated(ListView, ItemIndex, ItemData)` |
| `ULunarListViewEntry` | `OwningListView`, `ItemIndex`, `ItemData`, `VisualState`, `bIsActiveItem`, `bIsSelectedItem` | corresponding getters | Blueprint events `On List View Item Data Changed` and `On List View Item Visual State Changed(State, IsActiveItem, IsSelectedItem)` |
| `ULunarComboBox` | `Options`, `SelectedOptionId`, `bAllowEmptySelection`, recovery/placeholder, optional Selected/Entry/Empty visual classes, placement, orientation, wrap, popup sizing/scope, external-filter retention, interpolation, inline fallback styles, Designer Preview | option/selection APIs; `OpenComboBox`, `CloseComboBox`, `IsComboBoxOpen`; `SetFilterText`, `GetFilterText`, `ClearFilter`, `RefreshFilter`, `DoesOptionMatchFilter`; Arrow/Popup presentation Set/Get/Configure/Get | `OnSelectionChanged(ComboBox, PreviousOptionId, NewOptionId)`, `OnComboBoxOpened(ComboBox)`, `OnComboBoxClosed(ComboBox)`, `OnFilterTextChanged(ComboBox, PreviousFilterText, NewFilterText)` |
| `ULunarContextMenu` | `bRestoreSelectionOnOpen` | `OpenContextMenu`, `CloseContextMenu`, `IsContextMenuOpen` | `OnContextMenuOpened(ContextMenu)`, `OnContextMenuClosed(ContextMenu)` |
| `ULunarTabs` | `TabDescriptors`, `ActiveTabId`, `PageLifetime` | `SetTabs`, `ActivateTabById`, `GetActiveTabId`, `GetPageWidgetById` | `OnActiveTabChanged(Tabs, PreviousTabId, NewTabId)` |
| `ULunarTabHeader` | `TabId`, `TabsOwner` | `GetTabId`, `GetTabsOwner` | base delegates only |

`FLunarListViewItemData` is the complete logical item contract: stable `ItemId`, culture-invariant-by-default `FText DisplayText`, `bEnabled`, `bCanReceiveSelectionWhenDisabled`, and culture-invariant-by-default `FText DisabledReason`. `ULunarListViewEntry` is presentation-only and must not be registered as a separate Lunar navigation target.

`ILunarInputPromptReceiver` exposes only `ApplyResolvedPromptActions`. `ULunarInputIconSet` exposes `ResolveIconForKey`, and `ULunarUIActionRegistry` exposes `FindActionDefinition` and `ValidateRegistry`.

### Final Delegate Type Names

- `FLunarSelectionChangedSignature`;
- `FLunarActiveScopeChangedSignature`;
- `FLunarNavigationRejectedSignature`;
- `FLunarNavigableWidgetEventSignature`;
- `FLunarHoldProgressSignature`;
- `FLunarNavigableInputDeviceChangedSignature`;
- `FLunarVisualStateChangedSignature`;
- `FLunarButtonClickedSignature`;
- `FLunarSliderValueChangedSignature`;
- `FLunarSliderValueCommittedSignature`;
- `FLunarOptionSliderIndexChangedSignature`;
- `FLunarSwitchChangedSignature`;
- `FLunarSwitchDisplayedHandleAlphaChangedSignature`;
- `FLunarRadioSelectedIndexChangedSignature`;
- `FLunarRadioDisplayedSelectionPositionChangedSignature`;
- `FLunarScrollStateChangedSignature`;
- `FLunarListViewActiveItemChangedSignature`;
- `FLunarListViewSelectedItemsChangedSignature`;
- `FLunarListViewItemActivatedSignature`;
- `FLunarComboBoxSelectionChangedSignature`;
- `FLunarComboBoxStateChangedSignature`;
- `FLunarContextMenuStateChangedSignature`;
- `FLunarTabsActiveTabChangedSignature`.

The existing `FLunarInputDeviceChangedSignature` remains the subsystem and Raw Input device-only signal. Widget instances use `FLunarNavigableInputDeviceChangedSignature` so external subscribers also receive the emitting widget.

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

C++ native tag declarations live in the `LunarGameplayTags` namespace and use identifiers such as `UI_Action_Navigate_Up` and `UI_State_Value_Checked`. Navigation IDs, Group IDs, Item IDs, Option IDs, and Tab IDs remain `FName` values and are not Gameplay Tags. Radio option `StringValue` is the separate non-localized `FString` technical ID used only inside its owning composite Radio.

## Settings Layout

The Lunar Settings sections are:

- Navigation Input: semantic bindings, keyboard navigation, analog thresholds, dead zone, and repeat;
- Navigation Behavior: fallback, recovery, scroll-into-view, cursor, pointer capture, and chaining defaults;
- UI Audio: global Pointer and Navigation sounds;
- UI Haptics: global gamepad effects;
- Input Prompts: default Prompt Widget, visibility policy, and Keyboard/Mouse, Xbox, and PlayStation 5 Icon Sets;
- Accessibility: reduced motion and accessible-metadata validation;
- Diagnostics: validation verbosity, debug overlay, and graph-dump behavior.

There is no Default Styles settings section. Visual presentation is authored in Blueprint from the published state; reusable feedback Data Assets are assigned explicitly to widgets when `Use Data Asset` is selected.

## Design Closure

All V1 architecture, behavior, lifecycle, input, validation, accessibility, editor integration, source layout, and public API naming decisions are resolved. C++ implementation is active; owner runtime verification remains a separate ordered pass.

Owner-created Content asset instances and their future verified package paths are deferred by the explicit Content ownership boundary. They are integration inputs, not unresolved C++ design questions.

## Implementation Plan

### Phase 1: Types And Settings

- implement the finalized navigation, scope, link, visual-state, sound, haptic, prompt, and icon-set declarations;
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
- implement visual-state, event, sound, haptic, prompt, and scroll hooks;
- implement Button as the reference control;
- document the owner-performed reparenting and binding steps for `W_Button` without modifying the asset.

### Phase 4: Value Controls

- implement Slider;
- implement OptionSlider;
- implement Switch;
- implement composite Radio generation, required internal selection, Side Visual presentation, wrap behavior, and optional Checked-indicator interpolation.

### Phase 5: Composite Controls

- implement ListView logical navigation, virtualization, and item restoration;
- implement ComboBox nested scope behavior;
- implement ContextMenu and submenu scopes;
- implement horizontal and vertical Tabs.

### Phase 6: Prompt And Presentation Assets

- implement the default Prompt Widget contract;
- implement the Icon Set, Action Registry, Sound Feedback, and Haptic Feedback Data Asset classes without creating asset instances;
- connect device switching;
- implement error placeholders and null-safe resolution for unassigned defaults;
- leave `TODO(LunarUI)` markers at every default Content connection point;
- integrate owner-created Blueprint presentation, sounds, haptics, prompts, and icons only in a later explicitly requested pass.

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

- all supported controls use the same selection, scope, visual-state, prompt, sound, and haptic contracts;
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
