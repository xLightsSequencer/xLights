# Phase F — Window System + Display Elements

**Status: ✓ complete (2026-04-21).**

All F-1 through F-6 work is done. Detached-scene-owned preview
state (is3D, camera, layoutGroup) was deferred — not worth the
refactor given the session-destruction policy adopted for F-5.

## Summary of what landed

### F-6 — Display Elements editor

- New bridge surface on `XLSequenceDocument`: view CRUD +
  reorder, models-in-view add/remove/move, element roster +
  visibility toggles, per-timing-track view membership,
  `addTimingToAllViews:` (B82), show-layout roster via
  `modelsAvailableInShowLayout`, `addModel(toMasterView:)`,
  `elementHasEffects:`, `removeElementFromMasterView:`.
- SwiftUI `DisplayElementsSheet.swift`: NavigationSplitView
  sidebar with Master-locked views list, two-pane transfer UI
  for both Master and user views (Available / In-View) with
  visibility eye toggles and remove-with-warning on Master View
  for effects and timings.
- Entry point in the existing view-picker menu at
  `SequencerGridV2View.swift` ("Edit Display Elements…").
- Master-View semantics match desktop: removing an element from
  Master deletes it (including any effects on it) via the same
  `RemoveSelectedModels` MASTER_VIEW path, with the pre-delete
  `AbortRender()` guard from issue #4134.

### F-4 — Menu bar / `.commands`

- Every keyboard-shortcut button the toolbar used to hide is now
  also reachable via the iPadOS 26 menu bar. File / Edit / View
  / Playback / Help commands wired through a single `.commands
  { … }` block on the main scene. No behaviour changes — just
  discoverability.

### F-1 — Scene-level split (detachable previews + inspector)

- Main sequencer window remains the primary `WindowGroup`; added
  second `WindowGroup`s for detachable House Preview, Model
  Preview, and keyed `WindowGroup` for detachable inspector
  tabs. Detach is opt-in via toolbar buttons inside each pane.
- Dismissing a detached window restores its content to the docked
  layout in the main window.
- Stage Manager caches window position outside SwiftUI's reach,
  so main can relaunch in a detached pane's last-active corner —
  self-corrects after one drag-reposition. Known iPadOS
  limitation; not worth further mitigation until iOS 27 exposes
  scene-position API.

### F-2 / F-3 — Size-class responsive + docked layout

- `SequencerView` adapts between compact (Slide Over) and regular
  width classes. 12.9"+ landscape shows House + Model previews
  side-by-side; narrower regular widths still dock both (no
  notebook-switcher regression); compact falls back to a picker
  between the two.
- Landscape (regular width, inspector visible) tucks the preview
  band above the grid column only so the inspector sidebar runs
  the full vertical span — settings have a lot of controls and
  benefit from the extra height. Portrait keeps the original
  full-width preview band above grid+inspector.
- Inspector sidebar width is draggable via a leading-edge handle
  (mirrors the preview-height handle), persisted via `@AppStorage`
  and clamped 280…720 pt and ≤60% of the viewport. Default bumped
  to 340 pt so the four-tab segmented picker ("Effect | Colors |
  Blending | Buffer") fits without clipping in portrait.

### F-5 — Persistence

- Main scene declares a content-size minimum (1000×700) via
  `.windowResizability(.contentSize)` so iPadOS can't inherit a
  detached pane's small thumbnail geometry on relaunch.
- Persisted scene sessions destroyed at launch via
  `UIApplicationDelegateAdaptor` — avoids iPadOS 26 conflating
  multi-WindowGroup geometry across launches.
- Detach state (which preview + which inspector tabs were
  detached at main-close) captured in `@AppStorage` from both
  the `.inactive` and `.background` scenePhase transitions, and
  replayed on next launch by re-opening the same detached
  scenes. Silent save-on-close covers the pill-X-on-main case
  since Stage Manager's close isn't interceptable.

## Deferred to `future-layout-editing.md`

- Detached-scene-owned preview state (per-scene is3D, camera,
  layoutGroup). Worked around by having detached scenes share
  the main window's preview state via the @Observable view
  model. Revisit if users ask for truly independent camera per
  detached pane.
