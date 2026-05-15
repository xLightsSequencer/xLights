# Phase D — Model Preview + preview polish

**Status: complete (2026-04-20).**

All in-scope Phase D work shipped: saved viewpoints (D-3),
2D/3D toggle (D-4), model placement sourcing (D-5), Fit All /
Fit Selected (D-6), background image display (D-7), preview
image export (D-10), House Preview transport parity (D-11),
"View Objects" toggle (D-13), and alternate `LayoutGroup`
previews (D-14). 2D-mode core render fixes (virtual canvas
size from `<settings><previewWidth/Height>`, `Display2DCenter0`
in the view matrix, correct `is_3d` flag to
`DisplayModelOnWindow`) all landed.

## Explicitly dropped

- **D-12 pixel / point-size control** — no desktop counterpart;
  iPad keeps the hardcoded 2.0 to avoid misleading users
  diffing the two clients.
- **D-15 FPS / render-time overlay** — diagnostic noise that
  caused support confusion.

## Deferred to other plans

- Layout-editor overlays and authoring (model name / info /
  first-pixel labels, 2D grid / bbox overlays, drag-to-move,
  resize handles, polyline editing, property grid, align /
  distribute, CAD/DXF export, wiring view, bulk edit) — parked
  in [`future-layout-editing.md`](future-layout-editing.md).
  `Display2DCenter0` is already in the view matrix; only the
  grid/bbox overlays themselves are parked.
- Per-model show/hide and Views management — covered by Phase
  F-6 Display Elements editor (already complete).
- Detached-scene-owned preview state (per-scene is3D, camera,
  layoutGroup) — Phase F deferral; detached scenes share the
  main window's preview state via the `@Observable` view model.

## Caveats

- The "View Objects" toggle is a single coarse switch over
  background image + house mesh / terrain / gridlines / ground
  images. Future per-category toggles would need new UI.
- Fit Selected uses `viewModel.previewModelName` and silently
  no-ops when the selected model isn't visible in the active
  layout group — not an error state, but worth knowing if a
  tester reports "Fit Selected does nothing".

## Landed fixes

- **2026-05-15 — fast-math infinity sentinel.** `XLMetalBridge.mm`
  Fit All / Fit Selected bounding-box accumulation and 3D tap-to-
  select (both Model and ViewObject) seeded `min/max/bestDist`
  with `std::numeric_limits<float>::infinity()`. iPadLib Release
  inherits `-ffast-math` + `-O3` + `LLVM_LTO=YES_THIN` from the
  project-level config, and `-ffinite-math-only` folds the
  sentinel to 0 — silently breaking Fit-All and 3D taps in
  shipped builds. Swapped to `numeric_limits::max()`/`lowest()`.
  Same root cause as the desktop `HitTest.cpp` fix; see
  `feedback_fastmath_infinity_sentinel` for the pattern. No
  TestFlight bug filed — caught by the post-mortem audit.
