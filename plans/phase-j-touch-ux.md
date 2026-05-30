# Phase J — iPad Layout Editor: touch + Pencil UX

Sibling to [`phase-j-layout-editor.md`](phase-j-layout-editor.md).
Engineering and the bulk of the gesture/Pencil work landed
2026-05-11..15 (toolbar tool selection, Snap / Uniform / Lock-Axis
modifiers, 3D body drag, pinch / twist on model, Pencil hover,
Pencil double-tap, Pencil Pro squeeze, long-press contextual menus,
inline + multi-select action bars, rubber-band multi-select, model
label overlays, Add Model / create-on-canvas / multi-vertex polyline
create, camera-aware drags). What follows is what's left.

---

## Remaining concrete follow-ups

- **Multi-select body drag.** Currently a body drag on any
  member of the selection moves only the touched model;
  `beginBodyDrag3D` / `dragBody3D` in `XLMetalBridge.mm` take a
  single model name. Desktop drags the whole selection by the
  same delta. Needs the bridge to capture a per-selected-model
  offset table at drag-begin and apply the delta to all of them
  on each update + commit. Same per-model undo pattern as
  align / distribute / match-size.
- **Sidebar multi-select UI.** The Models roster list is still
  single-`String?` bound (`modelListBinding`). Multi-select is
  only reachable from canvas taps + marquee today. Adding an
  `EditMode = .active` pass with checkmark rows would let users
  build a selection from the roster (useful when the relevant
  models are off-screen or visually overlap).

---

## Open design questions (for future work)

- **Pencil pressure** — does it actually help? Or is it a gimmick
  here? Prototype, then decide.
- **Sketch-mode polyline create** — pencil-draws-a-path → auto-
  vertices. Compelling but post-MVP.
- **3D camera precision** — the current "drag-on-empty-3D = orbit"
  is fast and approximate. Do we add a separate precision-camera
  mode (e.g. on-screen joystick) for fine framing?
- **Apple-Pencil-not-paired fallback.** When the Pencil button
  shows in the toolbar (visible indicator that "Pencil enhances
  this app"), or hidden when no Pencil is configured? Apple's HIG
  prefers hidden until detected.
- **Accessibility.** VoiceOver users can't use most of the canvas
  interactions. Need a parallel "list of models + property panel"
  path that's already partially there from J-1. Audit pending.

---

## Design principles (kept for context)

1. **Recognisable as xLights.** Same handle palette and
   conceptual model as desktop.
2. **Touch-first, not touch-tolerant.** Multi-finger gestures
   are first-class.
3. **Pencil-enhanced, never Pencil-required.** Every operation
   reachable with finger-only.
4. **Discoverable.** No "click the orange sphere twice to cycle
   tools." Modes live in a persistent toolbar.
5. **Apple-HIG-compatible.** Pencil for precision, fingers for
   navigation.
