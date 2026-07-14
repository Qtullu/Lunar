# UI Navigation Owner Handoff

This checklist is the owner-only Content integration and manual verification pass for the Lunar UI Navigation System. The C++ types named below are present in `Source/Lunar`, but no result in this document is marked as manually verified. All runtime rows remain pending until the owner creates or reparents the required Blueprint and Data Asset content and tests the intended game flow.

Do not use this handoff to modify `ULunarDraggableWindow`. It remains mouse-only and outside the Lunar navigation hierarchy.

## 1. Content Preflight

- [ ] Create an example menu Blueprint derived from `ULunarScreenWidget`; choose the asset name and package path as the owner.
- [ ] Create it for the intended owning local player and add it through that player's UI flow.
- [ ] Add a two-local-player test variant. Give each player an independently owned menu instance and enough controls and nested scopes to expose accidental cross-player routing.
- [ ] Configure `NavigationScopeSettings.InitialSelectionId` (or `InitialSelectionWidget`), restoration, horizontal and vertical wrap, pointer policy, navigation groups, and `bBlockAllGameplayInput` for the test screen.
- [ ] Give every selectable control a stable, unique `NavigationId`, an `AccessibleName`, and any required `AccessibleDescription` or `DisabledReason`.
- [ ] Leave native UMG Navigation rules empty. Use `UpLink`, `DownLink`, `LeftLink`, and `RightLink` only for deliberate Widget, NavigationId, or Block rules; let Automatic cover the remaining directions.
- [ ] Reparent or create presentation Blueprints only after checking their existing Blueprint parent contracts. Follow [W_Button Owner Handoff](UI_BUTTON_OWNER_HANDOFF.md) for the button asset.
- [ ] Do not put a native interactive control around a Lunar control when both would handle the same action.

### Editor integration preflight

- [ ] Create a disposable non-Lunar Blueprint derived from `UUserWidget`. Confirm native `Navigation` and `bIsFocusable` are visible, enable focusability, and author one non-default native Navigation rule.
- [ ] Duplicate that Blueprint and reparent the duplicate to `ULunarScreenWidget` or `ULunarNavigableWidget`. Confirm the two native fields are now hidden, Lunar properties remain visible in Lunar categories, and the explanatory row appears. Leave the original non-Lunar Blueprint unchanged as the control case.
- [ ] Confirm validation reports the preserved native rule on the reparented Lunar Blueprint, then use `Clear Native Navigation`. Verify the rule clears transactionally, the Lunar links remain unchanged, and Undo/Redo restores and clears the native rule without affecting the non-Lunar control Blueprint.

## 2. Owner-Created Presentation Data

- [ ] Create a Prompt Widget derived from `ULunarInputPromptWidget`, or a `UUserWidget` that implements `ILunarInputPromptReceiver`. If the standard base is overridden, call the parent `ApplyResolvedPromptActions` implementation so its complete snapshot remains current.
- [ ] Create separate `ULunarInputIconSet` assets for Keyboard/Mouse, Xbox, and PlayStation 5. Add one unique `FLunarInputIconEntry` per resolved `FKey`; duplicate key entries are invalid.
- [ ] Create a project Action Registry as `ULunarUIActionRegistry`, assign it to `Navigation.Input.DefaultActionRegistry`, and do not define the same action tag in both `Navigation.Input.ActionDefinitions` and the registry.
- [ ] Add one owner-defined Gameplay Tag and action definition only to the registry, with a binding and display text. Request that action in a control prompt and handle or log it through the control action hook. Confirm both input dispatch and prompt key, text, and icon resolution use the registry entry.
- [ ] During an editor PIE diagnostic run, change that registry entry's binding or display text. Confirm the active prompt and action lookup refresh without recreating the prompt receiver, validation deduplication resets for the changed configuration, and the obsolete binding no longer dispatches.
- [ ] Create only the typed style assets that the example uses: Button, Slider, OptionSlider, Switch, Radio, ScrollBox, ListView, ComboBox, ContextMenu, Tabs, and Input Prompt. Keep each `ParentStyle` chain type-compatible and acyclic.
- [ ] Assign verified assets in Project Settings under `Navigation.DefaultStyles`, `Navigation.Prompts`, `Navigation.Audio`, and `Navigation.Haptics`. Do not add guessed package paths to C++.
- [ ] For each prompt owner, enable `bEnableInputPrompt`, supply ordered `PromptActions`, and choose `WhenSelected`, `Always`, or owner-driven `Manual` visibility. Confirm the receiver displays the complete array and clears on an empty array.
- [ ] Include a deliberate missing-icon case in a non-shipping test variant to verify the white-square placeholder and actionable Lunar Console error, then restore the mapping.

## 3. Example Menu Control Checklist

| Control | Required owner setup | Manual behavior to prove |
| --- | --- | --- |
| `ULunarButton` | Reparent or create its visual Blueprint; bind behavior once through `OnClicked`; add Accept prompt metadata. | Pointer/touch and Accept activate on release exactly once; leaving or losing selection before release cancels; disabled activation rejects. |
| `ULunarSlider` | Add Immediate and On Accept examples; set range, step mode/size, orientation, and optional repeat override. | The value axis changes the value without leaving the control; the perpendicular axis navigates; On Accept preview commits with Accept and cancels with Back or selection loss. Pointer/touch drag commits on release. |
| `ULunarOptionSlider` | Supply localized `Options`, a valid `SelectedIndex`, orientation, and a wrap/no-wrap case. | Increase/Decrease changes one logical option; bounds retain selection; wrapping occurs only when enabled. |
| `ULunarSwitch` | Add Horizontal, Vertical, or Accept-only examples through `DirectionMode`. | Accept toggles; the configured axis sets the value; a perpendicular direction leaves the control; repeating the already-active value stays selected. |
| `ULunarRadio` + `ULunarRadioGroup` | Construct one shared non-visual group; assign it to at least two Radios with unique `RadioId` values. Configure required selection and, separately, optional deselection if needed. | Lunar Selection moves independently from checked state; Accept changes the checked item; required-selection and optional-deselect invariants hold. |
| `ULunarScrollBox` | Use `ULunarScrollBox`, not a native `UScrollBox`, around a set of navigable descendants. Add a nested inner ScrollBox and configure padding, smooth/immediate reveal, and chaining. | Selection reveals descendants inner-first; wheel and drag use available delta; remaining delta reaches the ancestor only when chaining is enabled; capture releases on teardown. The ScrollBox itself never receives Lunar Selection. |
| `ULunarListView` | Supply item objects implementing `ILunarListViewItem`; give every item a stable, non-empty, unique ID and explicit eligibility/disabled reason. | The ListView remains the one outer selection owner; logical active item survives row recycling and refresh by ID; disabled-selectable items reject; perpendicular navigation exits the list. |
| `ULunarComboBox` | Supply unique `OptionId` values and localized option text. Bind an owner-created popup root and the required `ULunarListView` option owner; add the optional search navigation widget only when search is enabled. | Open pushes one child scope; navigation changes the temporary value; Accept commits; Back and outside click close without commit; filtering preserves an eligible ID or chooses the first eligible result. |
| `ULunarContextMenu` | Keep the container itself non-selectable, bind or set the popup bounds and scope root, and put navigable menu items inside it. Add one submenu implemented as another `ULunarContextMenu`. | Opening the menu pushes one scope and opening the submenu pushes one more; Back closes only the top submenu; outside-pointer handling closes only the current top menu; parent selection restores. |
| `ULunarTabs` + `ULunarTabHeader` | Supply unique `TabId` descriptors, a header Blueprint derived from `ULunarTabHeader`, and exactly one page source per tab. Give page descendants stable IDs. Exercise horizontal and vertical orientations and the selected page lifetime(s). | Strip-axis input moves between headers; Accept activates a page without moving selection off its header; perpendicular input enters the active page; page switching and re-entry restore the last valid descendant. |

For arbitrary nested content, only child Lunar controls with `bCanReceiveLunarSelection` enabled participate in the graph. Decorative children, prompt widgets, and the example screen container must not become accidental selection targets.

## 4. Scope Stack Checklist

- [ ] Open the `ULunarScreenWidget` and confirm exactly one parent scope is active with the configured initial selection.
- [ ] Open the ComboBox and confirm its popup scope is above the screen scope; close it with Accept, Back, and outside click in separate runs.
- [ ] Open the ContextMenu and its submenu and confirm the stack depth increases one scope at a time.
- [ ] Press Back once in the submenu: only the submenu closes. Press Back again: only the parent ContextMenu closes. The original screen selection returns.
- [ ] Switch Tabs pages and confirm they remain part of the screen scope rather than creating popup scopes; inactive page descendants are ineligible while collapsed.
- [ ] Disable, hide, remove, and reconstruct the selected control in separate runs. Confirm deterministic recovery while an eligible target exists and restoration by stable ID after reconstruction.
- [ ] Run `ValidateNavigationScope` after every graph-changing setup step. Resolve every Error and review every Warning before recording device results.
- [ ] Capture `DumpActiveNavigationGraph` output for the base screen, ComboBox popup, ContextMenu, and submenu. Use the debug overlay only as diagnostics; it is not manual-test evidence by itself.
- [ ] With two local players, prove that each player has an independent active scope stack, Lunar Selection, native focus owner, repeat state, active device/prompt handoff, and restoration history. Input assigned to one player must never navigate, activate, close, or replace selection in the other player's scopes.
- [ ] Build a small diagnostic layout with deliberately equivalent candidates. Prove greater `NavigationPriority` wins and stable registration order breaks the remaining tie; repeat after reconstruction to confirm the same registration order produces the same result.
- [ ] Exercise a direct Widget link and a stable `NavigationId` link, including an explicit link across groups. Make each target hidden or otherwise ineligible in turn and confirm geometric fallback is used; restore the target and confirm the explicit link wins again.
- [ ] Exercise an explicit Block rule and confirm the direction never falls back or wraps. Separately verify horizontal and vertical wrap both enabled and disabled at their respective edges.
- [ ] Exercise same-group preference with cross-group fallback allowed, then disable `bAllowCrossGroupFallback` for that group and confirm automatic fallback and wrap remain inside it while an explicit link may still cross the group boundary.

## 5. Pending Manual Verification Matrix

Every row below is **Pending owner Content integration and runtime verification**. A checked box requires a dated PIE, Standalone, or packaged-build result and recorded device/build details; compiling C++ alone does not satisfy it.

| Lane | Owner setup and required coverage | Status | Evidence to record |
| --- | --- | --- | --- |
| Mouse | Real owner presentation; cursor policy; every control; ComboBox outside click; ContextMenu/submenu; nested ScrollBox wheel/drag. Verify hover does not move Lunar Selection, click does, and navigation presentation hides during pointer use. | [ ] Pending | Build/config, screen recording or notes, graph dump, failures fixed. |
| Keyboard | Keyboard navigation enabled; default or owner bindings. Cover arrows and WASD, Enter/Space Accept, Escape Back, repeat, every value-control axis, popup stack, Tabs re-entry, and any delegated text field. | [ ] Pending | Keys used, scope/selection trace, consumed-input trace. |
| Xbox | Physical Xbox-family controller and assigned Xbox Icon Set. Cover D-pad, left-stick thresholds/repeat, Accept/Back, haptics if assigned, all controls, nested scopes, and prompt handoff. | [ ] Pending | Exact controller, connection mode, resolved prompt screenshots, scope trace. |
| PlayStation 5 | Physical DualSense and assigned PlayStation 5 Icon Set. Repeat the Xbox coverage and switch between controller families without restarting the menu. | [ ] Pending | Exact controller/firmware/connection mode, prompt screenshots, scope trace. |
| Two local players | Create independently owned menus for two local players. Exercise selection, nested scopes, repeat, native focus, device/prompt handoff, reconstruction, and closure for each player while the other menu remains active. Prove there is no cross-player navigation or activation. | [ ] Pending | Player/controller assignment, both scope and selection traces, prompt states, failures fixed. |
| Gameplay isolation | Instrument representative Pawn actions plus one gameplay action/key that has no UI binding. With the menu active, prove every handled UI key-down, matching key-up, and analog action does not reach gameplay. With `bBlockAllGameplayInput=false`, prove the unused gameplay action still reaches the Pawn; with it `true`, prove that action is blocked. After the last scope closes, prove normal gameplay input resumes. | [ ] Pending | Pawn counters/logs before, during, and after each scope state and both block settings. |
| Styles and feedback | Use visible global, parent, child, value-state, interaction-state, and per-instance style patches with at least one overlapping field and one specialized control field. Prove merge precedence, immediate discrete-resource changes, interpolation, reversal, retargeting, and reduced-motion or disabled-transition behavior. For pointer and Navigation events, prove sound `Use Global`, `Disabled`, and `Custom` routing. With two local players, prove haptics reach only the owning controller, a new Lunar effect replaces the previous one, and active feedback stops when that player's last scope closes. | [ ] Pending | Asset/settings snapshot, state trace or recording, audio observations, controller routing and stop/replacement observations. |
| Primary touch | Use a real or validated Win64 primary-touch path. Cover tap/release on Button and value controls, Slider drag, ScrollBox drag, selection assignment, teardown capture release, and the absence of required touch prompt icons. | [ ] Pending | Touch source, input trace, recording or notes. |
| Accessibility and localization | Fill accessible names, values, descriptions, and disabled reasons. Observe native accessible focus/activation/value notifications with the target platform tooling; test disabled-selectable feedback, reduced motion, a runtime culture change, and representative right-to-left text. | [ ] Pending | Accessibility tool/platform, culture, reduced-motion state, observations and defects. |

## 6. Completion Gate

- [ ] Every Blueprint and Data Asset compiles with no owner-content errors.
- [ ] Lunar Details customization hides the two native fields, leaves non-Lunar widgets untouched, and `Clear Native Navigation` successfully clears a deliberate test rule.
- [ ] Validation has no unresolved Errors; accepted Warnings are documented.
- [ ] The assigned Action Registry's owner-defined action dispatches and resolves its prompt correctly, and an editor property change refreshes both without retaining the obsolete binding.
- [ ] Explicit Widget and NavigationId links, unavailable-target fallback, groups, priority, registration-order ties, wrap, and Block rules have repeatable graph traces.
- [ ] Mouse, keyboard, Xbox, PlayStation 5, two-local-player, gameplay isolation, styles/feedback, primary touch, and accessibility/localization matrix rows have recorded evidence.
- [ ] The menu is fully operable without a mouse after Content integration, and focus never becomes lost while an eligible target exists.
- [ ] Prompt icons switch correctly between Keyboard/Mouse, Xbox, and PlayStation 5; missing configured non-touch mappings remain visible and diagnosable, while touch requires no prompt icon.
- [ ] Owner-provided screenshots and examples are added only after the corresponding runtime result is verified.
- [ ] `ULunarDraggableWindow` and its asset remain unchanged and outside the navigation graph.

## 7. Ordered Widget Test And Defaults Queue

Work through this queue one row at a time. A row moves to **Verified** only after its owner Content is compiled, its behavior is manually exercised, `ValidateNavigationScope` has no unresolved errors, and the result is recorded. Keep C++ default asset references null until the corresponding owner asset is verified; only then assign it through `ULunarSettings::Navigation`.

| Order | Target | Default or project setting to finalize after verification | Required proof before moving on | Status |
| --- | --- | --- | --- | --- |
| 0 | Test harness: `ULunarScreenWidget`, Action Registry, Prompt Widget, Keyboard/Mouse Icon Set | Input action definitions, repeat/analog behavior, pointer policy, diagnostics, accessibility, default registry, prompt class, and Keyboard/Mouse icon set | One scope activates with deterministic selection; mouse and keyboard routing, prompts, validation, graph dump, and gameplay isolation work | [ ] Pending |
| 1 | `ULunarButton` | Default Button Style plus global/per-widget click, press, reject, sound, and haptic behavior | Mouse, touch, Enter/Space, cancellation on selection loss, disabled rejection, and exactly one release activation | [ ] Pending |
| 2 | `ULunarSlider` | Default Slider Style, default step/commit expectations, and repeat behavior | Immediate and On Accept modes, both orientations, pointer/touch drag, Back cancellation, bounds, and perpendicular navigation | [ ] Pending |
| 3 | `ULunarOptionSlider` | Default OptionSlider Style and wrap policy used by owner widgets | Localized options, both orientations, bounds, optional wrap, repeat, pointer controls, and change delegate | [ ] Pending |
| 4 | `ULunarSwitch` | Default Switch Style and owner convention for directional versus Accept-only operation | Accept toggle, horizontal/vertical modes, perpendicular navigation, repeated active value, pointer/touch, and change delegate | [ ] Pending |
| 5 | `ULunarRadio` + `ULunarRadioGroup` | Default Radio Style and project convention for required selection or optional deselection | Unique IDs, group ownership, checked state independent from Lunar Selection, disabled behavior, and group change delegate | [ ] Pending |
| 6 | `ULunarScrollBox` | Default ScrollBox Style plus reveal mode, padding, speed, and chaining defaults | Immediate/smooth reveal, nested inner-first consumption, wheel/drag/touch, remaining-delta chaining, and teardown capture release | [ ] Pending |
| 7 | `ULunarListView` + item contract | Default ListView Style and owner item-ID/eligibility convention | Virtualized row recycling, stable-ID restoration, refresh, disabled-selectable rejection, scrolling, and perpendicular exit | [ ] Pending |
| 8 | `ULunarComboBox` | Default ComboBox Style and popup/search owner setup | One child scope, temporary selection, Accept commit, Back/outside-click cancellation, filtering, disabled options, and restoration | [ ] Pending |
| 9 | `ULunarContextMenu` | Default ContextMenu Style and submenu/outside-click owner convention | Parent menu plus submenu scope stack, top-only Back/outside close, disabled items, pointer bounds, and parent selection restoration | [ ] Pending |
| 10 | `ULunarTabs` + `ULunarTabHeader` | Default Tabs Style and selected page-lifetime convention | Horizontal/vertical headers, activation, entry into active page, inactive-page eligibility, page lifetime, and descendant restoration | [ ] Pending |
| 11 | Input Prompt presentation | Default Input Prompt Style plus Xbox and PlayStation 5 Icon Sets | Keyboard/Mouse, Xbox, and PS5 live handoff; complete prompt arrays; custom action refresh; missing-icon diagnostic; no required touch icon | [ ] Pending |
| 12 | Cross-widget completion | Final global styles, audio, haptics, reduced motion, localization, and accessibility defaults | Two local players, full device matrix, style precedence/transitions, owner-only haptic routing, culture/RTL, accessibility tooling, PIE/Standalone evidence | [ ] Pending |

When one row is completed, record the tested build, device, Content asset paths, accepted warnings, and chosen defaults in that row or directly below it before starting the next row.
