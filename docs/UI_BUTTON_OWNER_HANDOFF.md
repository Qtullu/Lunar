# W_Button Owner Handoff

This checklist describes the Content work that may be performed **after** the Phase 3 C++ implementation is integrated. The implementation pass does not modify, save, reparent, or otherwise mutate `W_Button` or any other `.uasset`.

## Reparenting

1. Open `/Lunar/Widgets/Base/W_Button` in the Unreal Editor.
2. Reparent it to `ULunarButton` only after confirming that the existing Blueprint does not depend on an incompatible native parent contract.
3. Compile the Blueprint and resolve any owner-specific presentation errors without changing the C++ navigation contract.
4. Keep arbitrary button content in the Blueprint hierarchy; `ULunarButton` does not impose a named content widget.

Do not wrap the Lunar control in a native `UButton` that independently handles the same click. That would create two activation paths. If a native button remains for presentation, its click handling must not rebroadcast Lunar activation.

## Presentation Binding

- Bind project-specific presentation reactions to `BP_OnLunarVisualStateChanged`.
- Use the resolved `FLunarUIVisualState`; do not reproduce input-device, selected, pressed, or disabled state resolution in Blueprint. Common C++ style fields are applied by the native control.
- Bind gameplay or menu behavior to `OnClicked` (or override `BP_OnLunarActivated`). Navigation Accept, pointer release, touch tap, and `Click()` all converge on that guarded activation path.
- Use `OnLunarPressed` and `OnLunarReleased` only for presentation. Do not trigger the action on Pressed; Button activation is release-based.
- Leave `StyleAsset`, prompt classes, icon sets, sounds, and haptics unassigned until their owner-created assets exist. No guessed package paths are required.

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
