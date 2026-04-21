# Phase D — Model Preview + preview polish

**Status: ✓ complete (2026-04-20).**

All in-scope Phase D work is done. Layout-editor-only overlays and
model manipulation are parked in
[`future-layout-editing.md`](future-layout-editing.md). Multi-
`LayoutGroup` editing and per-view visibility management stay in
Phase F (Window System + Display Elements).

## Summary of what landed

Core render fixes for 2D mode (virtual canvas size from
`<settings><previewWidth/Height>`, `Display2DCenter0` in the view
matrix, correct `is_3d` flag to `DisplayModelOnWindow`).

Per-item:

- **D-3 Saved viewpoints** — `iPadRenderContext` owns a `ViewpointMgr`;
  `SaveViewpoints()` rewrites just the `<Viewpoints>` subtree of
  `xlights_rgbeffects.xml`. `XLMetalBridge` exposes per-pane
  list / apply / save-as / delete / restore-default. SwiftUI menu in
  `PreviewControlsOverlay` with the applied name shown next to the
  `camera.viewfinder` icon.
- **D-4 2D/3D toggle** — House-Preview-only segmented picker. Initial
  value reads `<settings><LayoutMode3D>`; session overrides don't
  write back (layout editing stays desktop-only).
- **D-5 Model placement sourcing** — verified. All House Preview
  placement data (`previewWidth`, `previewHeight`, `Display2DCenter0`,
  `LayoutMode3D`, `backgroundImage*`, `layoutGroups`, `Viewpoints`) is
  read from `rgbeffects.xml`. iPad never writes back; no schema
  migration needed.
- **D-6 Fit All / Fit Selected** — bridge methods walk model bounds
  via `ModelScreenLocation::GetHcenterPos` / `GetMWidth` etc. For 2D
  we pick `zoom = min(virtualW/bboxW, virtualH/bboxH) * 0.95` and pan
  to centre (honouring `Display2DCenter0`). For 3D we keep the
  rotation angles, pan so bbox centre sits at world origin, and pick
  `distance = max(bboxH/2, bboxW/2/aspect) / tan(22.5°) / 0.95`.
  Fit Selected uses `viewModel.previewModelName` (the currently-
  selected Model Preview target), and no-ops when the selected model
  isn't visible in the active layout group.
- **D-7 Background image** — read-only display in 2D House Preview.
  Honours per-`LayoutGroup` overrides. Lazy-loaded via
  `CGImageSource`, texture cached on the bridge. Gated behind the
  D-13 "View Objects" toggle.
- **D-10 Preview image export** — share-sheet button. MTKView snapshot
  via `drawHierarchy(in:afterScreenUpdates:)` → `UIActivityViewController`
  (Files / Photos / Mail / AirDrop / Copy / Print out of the box).
- **D-11 House Preview transport parity** — Rewind-to-start / Rewind
  10s / Stop / Play-Pause / FF 10s in the main toolbar. Scrubbing
  handled by the existing sequencer-ruler playhead drag.
- **D-13 "View Objects" toggle** — one switch for the whole visual
  backdrop: background image + view objects (house-mesh / terrain /
  gridlines / ground images). Future layout-editor overlays will hook
  the same flag.
- **D-14 Alternate LayoutGroup previews** — `iPadRenderContext` loads
  `<layoutGroups>`. House Preview filter mirrors desktop
  `UpdateModelsList`: `layout_group == active || "All Previews"` plus
  ModelGroup expansion. Switcher Menu in the overlay.

Dropped for desktop parity:

- **D-12 Pixel / point-size control** — no desktop counterpart; kept
  the hardcoded 2.0. Would mislead users diffing iPad vs desktop.
- **D-15 FPS / render-time overlay** — diagnostic noise; causes
  support confusion.

## Deferred to future-layout-editing.md

- Model-name / info / first-pixel overlays (L-1).
- 2D grid / bounding-box overlays (L-2). `Display2DCenter0` is already
  in the view matrix; only the grid/bbox overlays themselves are
  parked.
- Model selection, drag-to-move, resize handles, polyline vertex
  editing, property grid, align/distribute, flip, resize-to-match,
  CAD/DXF export, wiring view, bulk edit.

---

## Explicitly out of scope for Phase D (kept for audit reference)

- **Per-model show/hide + Views management** — Display Elements
  dialog, Phase F-6. D-13 only provides the coarse view-objects
  toggle as a stop-gap.
- **Detach previews to external display / separate window** — Phase
  F-1 (scene-level window system).
- **3D Connexion / space-mouse input** — desktop-only peripheral.
- **Keyboard shortcut camera nudging** — iPad input is touch-first;
  gesture equivalents already cover these.
