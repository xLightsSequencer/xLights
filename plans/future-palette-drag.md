# Future — Drag Colours Between Palette Slots

Not in the first-pass Phase C scope.

## Gap

**G18 — Drag colours between palette slots.** Desktop supports
drag-drop within the 8-slot palette so users can re-arrange
colours without having to re-pick each one. iPad has no drag-drop
support in the inspector.

## Why deferred

- Low impact — re-arrangement is infrequent. Users can manually
  re-pick colours in ~15 seconds.
- SwiftUI has `.draggable` / `.dropDestination`, but composing
  that with the existing palette row's Toggle / ColorPicker /
  curve preview + long-press context menu needs care (competing
  gestures).
- Not blocking any real user workflow.

## Scope when we come back

- Slot becomes a drop target that accepts another slot's payload.
- Drag payload: the slot's stored string (hex or ColorCurve blob)
  + source slot index for move-vs-copy decision.
- Hold modifier (drag-with-long-press-first) = copy; plain drag =
  swap. Or always swap and skip the copy gesture.
- Haptic feedback on drop.
