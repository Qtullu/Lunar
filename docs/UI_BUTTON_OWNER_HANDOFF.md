# W_Button Owner Handoff

This checklist describes the Content work that may be performed **after** the Phase 3 C++ implementation is integrated. The implementation pass does not modify, save, reparent, or otherwise mutate `W_Button` or any other `.uasset`.

## Reparenting

1. Open `/Lunar/Widgets/Base/W_Button` in the Unreal Editor.
2. Reparent it to `ULunarButton` only after confirming that the existing Blueprint does not depend on an incompatible native parent contract.
3. Compile the Blueprint and resolve any owner-specific presentation errors without changing the C++ navigation contract.
4. Keep arbitrary button content in the Blueprint hierarchy; `ULunarButton` does not impose a named content widget.

Do not wrap the Lunar control in a native `UButton` that independently handles the same click. That would create two activation paths. If a native button remains for presentation, its click handling must not rebroadcast Lunar activation.

## Presentation Binding

- Implement `On Lunar Visual State Changed` in the Button Blueprint. It receives Previous State, New State, and Designer Preview; use New State to drive arbitrary owner-created Borders, Images, TextBlocks, animations, and other content.
- The first callback initializes presentation even when Previous State equals New State. Do not wait for a later hover or selection transition before drawing the neutral state.
- Use `Preview Mode = Custom` and `Preview Visual State` in the UMG Designer to inspect enabled, disabled, pointer, navigation, device, and reduced-motion combinations without PIE. Return Preview Mode to None when not authoring a preview.
- `ULunarButton` has no Style Asset or automatic field traversal. It never guesses which owner widget is the background, border, label, or font target.
- Implement `On Lunar Clicked` inside the Button Blueprint for its behavior. External objects may bind to `OnClicked(ClickedButton)` and every common Lunar delegate identifies the emitting widget.
- Use Pressed, Hold Progress, Released, and Rejected overrides for their own lifecycle logic. Button activation remains release-based; do not trigger the main click action from Pressed.
- Leave prompt classes, icon sets, reusable Sound/Haptic Feedback Data Assets, and global feedback defaults unassigned until the owner-created assets exist. No guessed package paths are required.

## Instance Setup

- Give every navigable instance a stable `NavigationId` when it is referenced explicitly or restored by ID.
- Set `NavigationGroup`, priority, and directional links only where the scope design requires them.
- Supply `AccessibleName`; optionally supply `AccessibleDescription` and `DisabledReason`.
- Disable `bCanReceiveLunarSelection` only for decorative or pointer-only instances.
- Use `bCanReceiveSelectionWhenDisabled` only when a disabled Button must remain selectable and produce Rejected feedback.

## Manual Verification

1. Mouse/touch down shows Pointer Pressed; release inside emits one `OnClicked`; release outside emits none.
2. Accept down shows Navigation Pressed; Accept up emits one `OnClicked`.
3. Losing Lunar Selection before Accept up prevents Click.
4. Disabled activation emits Rejected feedback and no `OnClicked`.
5. Keyboard/gamepad selection, native focus, prompt invalidation, and scroll-into-view remain owned by Lunar Navigation.
