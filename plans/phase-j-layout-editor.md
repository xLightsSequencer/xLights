# Phase J — Layout Editor (iPad)

**Status:** J-0 ✓ 2026-05-07; J-1 common-properties surface ✓
2026-05-08; J-2 substantially complete 2026-05-08 — tap-to-select
(2D + 3D), selection rendering via `ScreenLocation::DrawHandles`,
drag-to-move (2D), per-type handle drag (2D + 3D) via the
descriptor pipeline (`GetHandles` / `BeginDrag` / `DragSession`,
see [`handle-system-refactor.md`](handle-system-refactor.md)),
grid + bounding-box overlays, snap-to-grid, keyboard nudge,
layout undo. Pending: 3D body-drag (camera-aware delta math),
text labels, rubber-band multi-select, per-type properties (J-3).
Promoted from [`future-layout-editing.md`](future-layout-editing.md)
on 2026-05-07 after the iPad app entered App Store review. Phase S
of the gap analysis is the engineering reference; this file is
the iPad-side sub-plan.

**Interaction design** for the touch + Pencil UX layer lives in
[`phase-j-touch-ux.md`](phase-j-touch-ux.md) — toolbar-driven
tool selection, persistent modifier toggles (replacing
Shift/Ctrl), Pencil hover as cursor-equivalent, barrel-tap /
double-tap mappings, and what changes vs stays the same in the
descriptor pipeline.

## Why this matters

Today the iPad app loads a show's pre-existing layout and plays it
back; it cannot *arrange* models. Every iPad user is therefore
dependent on a Mac/PC running desktop xLights to do the layout
work — model placement, sizing, rotation, model groups, layout
groups, locking, the whole sidebar. That's the single largest
remaining workflow gap once App Store submission ships and is the
right next phase to take on.

Source pointers:

- Desktop UI: `src-ui-wx/layout/LayoutPanel.cpp` (10,357 lines),
  `LayoutPanel.h`, `ModelPreview.cpp/.h`.
- Property adapters: `src-ui-wx/modelproperties/` (~40 per-model
  adapters subclassing `ModelPropertyAdapter`).
- Edit math (already wx-free, in `src-core/models/`):
  `ModelScreenLocation.h`, `BoxedScreenLocation.cpp`,
  `TwoPointScreenLocation.cpp`, `ThreePointScreenLocation.cpp`,
  `PolyPointScreenLocation.cpp`, `MultiPointScreenLocation.cpp`,
  `TerrainScreenLocation.cpp`. Drawing math, hit detection, handle
  movement, and locking are all reusable as-is from iPad.
- Align / distribute / flip / resize-to-match math:
  `LayoutPanel.cpp` lines ~5620–6630 (8 align ops + 3 distribute +
  flip H/V + resize-to-match width/height/both). All multi-select
  aware, single undo point per op.
- Undo: `LayoutPanel.cpp` lines 8052–8330. `UndoStep` struct stores
  XML snapshots; portable as data.

## Decisions (2026-05-07)

1. **Full Phase S scope this phase** (J-0 → J-4 below). One ship.
2. **Surfacing:** new full-screen detachable `WindowGroup` opened
   from the **Tools menu → "Edit Layout…"** entry in
   `XLightsCommands.swift:247`. No toolbar button on
   `SequencerView`. Keeps the sequencer toolbar simple and matches
   the precedent set by Import Effects / Check Sequence / Package
   Logs.
3. **Stay model-focused for v1.** PR #6311's controller-source-tree
   on the desktop Layout panel is a model-selection convenience
   only — does not extend the property grid to controller config.
   Worth revisiting after J-4 ships, but not in scope here.
   Standalone Controllers tab work continues to live in
   [`future-controllers-tab.md`](future-controllers-tab.md).
4. **3D editing is in scope.** A 6-DOF translate / rotate / scale
   gizmo on the selected model in 3D view. This is the biggest
   single design question in the phase — see "Risks" below.

## Scope

### J-0 — Bridge surface + read-only layout view ✓ 2026-05-07

Got the screen on the device, validated the detached-window
pattern, established the bridge-surface conventions for layout
state, and shipped a usable read-only inspector. Mutation work
runs in J-1+.

**What landed:**

- Bridge: `XLSequenceDocument.modelsInActiveLayoutGroup`,
  `modelLayoutSummary(_:)`, `layoutDisplayState`. All read-only.
- `iPadRenderContext` parses `Display2DGrid` / `Display2DGridSpacing`
  / `Display2DBoundingBox` from `<settings>` in
  `xlights_rgbeffects.xml` (it already had `Display2DCenter0`).
- `WindowGroup("layout-editor")` in `XLightsApp.swift` with
  `LayoutEditorWindowRoot` and the same token-guarded auto-restore
  protection used by F-1 detached previews.
- Tools menu → "Edit Layout…" entry (`EditLayoutMenuItem` in
  `XLightsCommands.swift`), enabled when a show folder is loaded.
- `LayoutEditorView` — full-screen `NavigationSplitView` with a
  Metal canvas (reuses `PreviewPaneView` with previewName
  `"LayoutEditor"`), layout-group picker in the toolbar, sidebar
  with model list + selected-model summary + display-state
  section. Selection is **sidebar-list driven** — tap a model
  name in the list to inspect its summary.
- `SequencerViewModel.layoutEditorSelectedModel`,
  `layoutEditorOpen`.

**Pulled forward to J-2** (clusters with handle / gizmo / overlay
rendering work):

- Tap-to-select inside the Metal canvas — needs a screen→world
  unproject helper on `XLMetalBridge`.
- In-canvas overlays (model name labels, first-pixel markers, 2D
  grid lines, bounding boxes, selection ring) — needs draw paths
  through `iPadModelPreview` / `XLGridMetalBridge`. Shipping these
  alongside the J-2 handle-rendering work avoids two passes
  through the Metal layer.

### J-1 — Property grid for selected model ✓ 2026-05-08 (common-properties surface)

Common-properties editing landed; per-type properties (Arch count,
Tree branches, Custom-model matrix, DMX channel mapping) are J-3
work. Model rename and copy/paste / reset menus are J-3+ as well.

**What landed:**

- Bridge: `XLSequenceDocument.setLayoutModelProperty(name:key:value:)`,
  `saveLayoutChanges`, `hasUnsavedLayoutChanges`. Per-key dispatch
  to `Model::SetHcenterPos` / `SetVcenterPos` / `SetDcenterPos` /
  `SetWidth` / `SetHeight` / `SetDepth`, `ScreenLocation::SetRotateX/Y/Z`
  / `SetLocked`, `BaseObject::SetLayoutGroup`,
  `Model::SetControllerName`. Each successful edit marks the model
  in `iPadRenderContext._dirtyLayoutModels`; nothing hits disk
  until `saveLayoutChanges` is called.
- Save path: `iPadRenderContext::SaveLayoutChanges` round-trips
  `xlights_rgbeffects.xml`, replacing each dirty model's
  `<model>` node with a fresh `XmlSerializer::SerializeModel`
  output (same path desktop uses for export). Mirrors the existing
  `SaveModelStates` pattern.
- UI: `LayoutEditorPropertiesView` replaces J-0's read-only summary.
  Editable cells for centre X/Y/Z, width/height/depth, rotate
  X/Y/Z, locked toggle, layout-group menu, controller-name field.
  Read-only cells preserved for type, channel range, strings,
  nodes. Save button in toolbar with dirty-state gating; alert on
  write failure.
- Layout-group reassignment refreshes the sidebar model list so
  models that move out of the active group disappear from the
  list immediately.

**Pulled to J-3** (per-type adapter zoo):
- Model rename — needs old-name → new-name bookkeeping in the
  dirty set so `SaveLayoutChanges` can find the on-disk node.
- Long-press → Reset / Copy / Paste menu via
  `PropertyContextMenu`.
- Per-type properties (Arch count, Tree branches, etc.).
- Greying-out of position/size/rotation cells when locked is on
  (today they accept input but the bridge silently rejects via
  `BaseObject::IsLocked` check — works correctly but is bad UX).

### J-2 — Direct manipulation + canvas overlays (in progress)

**First cut ✓ 2026-05-08:**
- `XLMetalBridge.pickModel(atScreenPoint:viewSize:for:)` —
  inverse of `iPadModelPreview::StartDrawing`'s 2D View matrix +
  bounding-box hit-test; iterates active-preview models in
  reverse draw order so the topmost rendered model wins. 2D-only
  (3D ray-cast deferred).
- Single-tap recognizer in `PreviewPaneView`, gated to
  `previewName == "LayoutEditor"`, drives
  `viewModel.layoutEditorSelectedModel`. Tap on empty space
  deselects.
- `XLMetalBridge.setSelectedModel(_:)` plus an `addStep` on the
  transparent program in `drawModelsForDocument` that draws a
  cyan bounding-box outline via `xlVertexAccumulator` /
  `drawLines`. Geometry is in world coords so pan / zoom track
  for free.
- Drag-to-move: `handleOneFingerPan`'s `.began` hit-tests the
  touch and, when it lands on the selected model in 2D, sets a
  `draggingLayoutModel` flag that routes `.changed` deltas
  through `XLMetalBridge.moveModel(_:byDeltaDX:dY:viewSize:for:)`
  instead of the camera-pan path. Drag end posts
  `.layoutEditorModelMoved` so the editor side panel refreshes
  the summary + dirty state. Locked models are silently rejected
  by the bridge.

**Pulled forward from J-0:**
- ✓ 2026-05-08: 2D grid + canvas-bounds bounding-box overlays
  drawn via `xlVertexAccumulator` `addStep` calls on the solid /
  transparent programs in `drawModelsForDocument`. Initial state
  seeds from the rgbeffects.xml `Display2DGrid` /
  `Display2DBoundingBox` flags; live toggles in the LayoutEditor
  controls overlay (`Grid` / `Bounds`). Draws snap to spacing
  multiples in 2D; hidden in 3D mode.
- Model-name / model-info / first-pixel label overlays still
  pending — these need text rendering through
  `CoreGraphicsTextDrawingContext` and a coordinate-projection
  helper, more involved than the simple primitive overlays above.

**Layout undo:**
- ✓ 2026-05-08: `iPadRenderContext` owns a 100-deep undo stack of
  `LayoutUndoEntry` snapshots (modelName + every common-property
  field). Bridge: `pushLayoutUndoSnapshotForModel:`,
  `undoLastLayoutChange`, `canUndoLayoutChange`. The LayoutEditor
  pushes a snapshot before every commit + once at drag-began
  (so a single drag = a single undo entry). Toolbar Undo button
  pops + reapplies via the same setter path. Undo includes a
  refresh of the model list because `layoutGroup` and
  `controllerName` may have changed.
- Redo is not implemented — desktop's `LayoutPanel` doesn't have
  redo either, so iPad parity stays "undo only."

**Snap + keyboard nudge:**
- ✓ 2026-05-08: Snap-to-grid toggle in the LayoutEditor controls
  overlay. Bridge's `moveModel` rounds the post-delta centre to
  the nearest `Display2DGridSpacing` multiple when the toggle is
  on. Off by default. Per-session.
- ✓ 2026-05-08: Keyboard nudge — arrow keys move the selected
  model 1 unit, shift+arrow moves 10. Pushed undo per tap so
  each nudge is independently reversible. Posts
  `.layoutEditorModelMoved` so the canvas repaints (the
  notification convention now uses `object: previewName` +
  `userInfo["model"]: name`, with the bridge coordinator
  listening for repaint).

**Resize / rotate / per-type handles + 3D selection:**
- ✓ 2026-05-08: Refactored from a custom 4-corner system to delegate
  to the existing `ModelScreenLocation::DrawHandles` /
  `CheckIfOverHandles` / `MoveHandle` API (plus the 3D variants).
  Same code path desktop's LayoutPanel uses; zero wx
  dependencies in the call chain. `XLMetalBridge.pickHandle` and
  `dragHandle:toScreenPoint:` are now thin shims that build the
  ray (3D) or pixel coords (2D) and forward.
- This brings along, **for free**:
  - **Per-type handle sets**: Boxed gets 4 corners + rotate; 3D
    Boxed gets 8 corners (top/bottom × 4); ThreePoint gets
    endpoints + shear; PolyPoint gets vertices + curve control
    points; etc.
  - **Rotate handle** — already wired into MoveHandle via
    `ROTATE_HANDLE`.
  - **3D handle drag** — `MoveHandle3D` shipped same time as 2D.
    Camera-aware drag math handled inside the existing code.
  - **Polyline vertex editing** — drag a vertex, drag a
    curve control point, all through PolyPointScreenLocation.
- ✓ 2026-05-08: **3D model picking** via `Model::HitTest3D` ray-cast
  through `VectorMath::ScreenPosToWorldRay`. Returns the closest
  hit by intersection distance. Replaces the previous "return
  nil in 3D" stub.
  - **DisplayModelOnWindow params** for the LayoutEditor pane now
    mirror desktop's `ModelPreview::RenderModels` (line 612):
    `color = xlYELLOW` (selected) / `xlLIGHT_GREY` (others) so
    models render in the layout-edit override colour rather than
    effect output, `allowSelected = true` (so PrepareToDraw
    updates ModelMatrix — required by HitTest3D /
    CheckIfOverHandles3D), `wiring = false`, `highlightFirst`
    bound to a "Pixel 1" toggle in the controls overlay. The
    selected model gets `Selected(true)` per draw so
    `Model::DisplayModelOnWindow`'s built-in DrawHandles path
    fires (Model.cpp:3254) — the bridge no longer draws the
    selection ring or handles itself.
  - **Gotcha #1** for future sessions: omitting `allowSelected=true`
    leaves `ModelMatrix` uninitialised, so HitTest3D ray-casts
    against an identity matrix and misses every model not at
    world origin. Was the bug behind "3D tap doesn't select."
  - **Gotcha #2**: `iPadModelPreview::GetCameraZoomForHandles()`
    must return `1.0` in 2D mode (mirroring desktop's
    `ModelPreview::GetCameraZoomForHandles`), not the active
    camera zoom. The handle-width math multiplies by zoom, and
    the View matrix multiplies by zoom AGAIN at draw time;
    returning the live zoom in both places quadratically scales
    handles, making them invisible at zoom < 1 and huge at
    zoom > 1. Was the bug behind "handles don't appear in 2D."
  - **Gotcha #3**: 3D mode in `BoxedScreenLocation::DrawHandles`
    (5-arg) only draws the centre sphere + `DrawAxisTool` gizmo
    (red/green/blue translate arrows, scale cubes, or rotate
    rings depending on `axis_tool` mode) when
    `active_handle != NO_HANDLE`. Desktop sets it to
    `CENTER_HANDLE` on select (`LayoutPanel.cpp:3152`). Without
    this the user sees only the bounding-box wireframe — no
    actionable handles. The bridge now sets / clears
    `SetActiveHandle` on select / deselect alongside the
    `Selected(true)` flag.
  - **Gotcha #4**: 3D `CheckIfOverHandles3D` returns the handle
    index OR'd with `HANDLE_AXIS` (0x0200000) when an axis arrow
    is hit, with the axis (`X_AXIS` / `Y_AXIS` / `Z_AXIS`) in
    the lower 8 bits. Desktop (`LayoutPanel.cpp:3697`) extracts
    the axis, calls `SetActiveAxis(axis)`, and then passes the
    model's CURRENT `active_handle` (CENTER_HANDLE) to
    MoveHandle3D — not the raw return value. Passing the raw
    `HANDLE_AXIS|axis` value to MoveHandle3D crashes via
    `assert(false)` in `ModelScreenLocation::DragHandle`'s
    default switch branch. Bridge `pickHandle` now does this
    extraction; `endHandleDrag` clears the axis on gesture end.
  - **Gotcha #5**: `BaseObject::MoveHandle3D` and
    `Model::MoveHandle` both return early on `IsFromBase()`
    (models imported from a parent "base" show — desktop's
    Christmas-village pattern of inheriting layout from a
    template). Drag handles still **draw** for from-base models
    (in `FROM_BASE_HANDLES_COLOUR = xlPURPLETRANSLUCENT`) so
    visually they look interactive, but the move is silently
    rejected. iPad's `pickHandle` now short-circuits on
    `m->IsFromBase()` so the drag doesn't engage at all and the
    user gets the camera-orbit fallback. Same protection
    already existed for `loc.IsLocked()`. The 3-arg / 4-arg
    `MSLAXIS` enum: values are `X_AXIS=0`, `Y_AXIS=1`,
    `Z_AXIS=2`, `NO_AXIS=3` — first enumerator gets 0, so
    `axis=0` in logs is X_AXIS, not "no axis."
  - **Gotcha #6**: `SubModel::GetModelScreenLocation()` (see
    `SubModel.h:29`) returns a reference to the **parent's**
    `screenLocation` — submodels do NOT have their own. They're
    effects-buffer carve-outs of a parent (a strand of an arch,
    a face on a DMX moving head), not standalone layout
    entities. Desktop's `LayoutPanel` never exposes them as
    selectable / movable. The iPad LayoutEditor pane filters
    them out at the source: `pickModel`, `drawModelsForDocument`,
    and `XLSequenceDocument.modelsInActiveLayoutGroup` all
    `continue` on `model->GetDisplayAs() == DisplayAsType::SubModel`.
    Without the filter, iterating a submodel of the selected
    model wiped the parent's `active_handle` (since
    `model->GetName()` doesn't match `_selectedModelName` →
    "non-selected" branch fires → clears the SHARED
    screenLocation). Symptom: handles drawn but no drag — every
    pickHandle saw `active_handle = NO_HANDLE` because a
    submodel iteration had cleared it.
- 3D body-drag (move-the-whole-model in 3D) still gated on 2D —
  needs camera-aware delta math we haven't shipped. Drag-on-empty-
  space-in-3D continues to orbit camera. Drag-on-handle in 3D works.

**Direct manipulation (still pending):**
- **Resize handles** drawn as overlay quads sized for touch
  (~32 pt). Map touch through
  `BoxedScreenLocation::MoveHandle` / sibling subclasses. Each
  ScreenLocation subclass already implements its own handle set —
  iPad just needs to render them and translate touch → handle
  index. No core math changes.
- **Rotate handle** — same pattern.
- **PolyLine vertex editing** — tap-to-add-vertex, drag vertex,
  long-press to remove. PolyPointScreenLocation already has the
  segment-control-point math.
- **3D gizmo** — see "Risks" for the design open question.
  Working assumption: a translate/rotate/scale gizmo with
  per-axis handles, pinch-on-handle = uniform scale, two-finger
  twist = rotate-about-axis. Detailed design happens in J-2's
  first week before we commit code.
- **Pinch ambiguity**: pinch on empty space = camera zoom (J-0
  already); pinch on selected model = uniform scale (new).
  Decision tree based on hit-test at gesture-start.
- **Rubber-band multi-select** — two-finger drag in empty space.
  Mirrors the existing two-finger marquee idiom in the effects
  grid.
- **Keyboard nudge** — arrow keys (with Bluetooth keyboard) for
  pixel / sub-pixel movement. Reuses `XLightsCommands` keyboard
  routing.
- **Locking** respected: handles don't render on locked models;
  drag is no-op.
- **Undo / redo** — port `UndoStep` struct verbatim from
  `LayoutPanel.cpp:8235`. Hook to `viewModel.undoManager` (already
  on `SequencerViewModel`). Per-op (drag = single undo entry,
  recorded on touch-down state).

### J-3 — Per-type properties + model creation (~4–6 wk)

The long tail. Per-model property pages and the Add Model toolbar.

**Per-type properties:**

- Port adapter logic for the high-traffic models first: Matrix,
  Tree, Arch, Star, Custom, PolyLine, Single Line, Icicles,
  Window Frame, Wreath, Candy Cane, Cube, Sphere, Image. ~14 of
  the ~40 desktop adapters cover ~95% of shipping model usage.
- Remaining adapters land opportunistically; missing per-type
  page = read-only "Edit on desktop" placeholder.
- DMX deep authoring (channel mapping, fixture definitions) stays
  in [`future-custom-models.md`](future-custom-models.md) — this
  phase only covers DMX position/dimension editing.
- Face / State editing also stays in `future-custom-models.md`.

**Model creation:**

- Add Model toolbar at the top of the editor screen with sticky-
  toggle buttons (Arch, Tree, Star, Matrix, Cube, etc.).
- Tap-button-then-tap-canvas to drop. Drag during the drop sets
  initial bounding box.
- Polyline gets a multi-tap mode (each tap adds a vertex; double-
  tap or button-deselect ends).
- Reuses desktop's `CreateNewModel(type)` factory and
  `Model::InitializeLocation()` flow — both already wx-free.

**Model groups:**

- Create / delete / rename model group.
- Drag-to-add-to-group + remove. UI shape TBD; likely a sheet
  with two columns (in-group / available) similar to
  `DisplayElementsSheet`.

### J-23.8 — Fat-finger snap, Pencil hover, Bresenham distribute ✓ 2026-05-15

Three issues, all addressed in `CustomModelEditorSheet`:

**Tap-on-pixel snap.** Finger taps on a pixel were placing new
pixels in adjacent empty cells instead of selecting the touched
pixel — the touch centroid often falls just outside the pixel
glyph. Added `nearestPixelCell(to:cellSide:xOff:yOff:slop:)` —
when the tapped cell is empty, search the active layer for the
closest occupied cell within `slop = max(cellSide * 0.85, 28)`
and treat that as the touch target. Apple Pencil hits were
already accurate, so this only kicks in on misses.

**Drag detection rewrite.** The snap broke drag-to-move: with
the touch snapped to a nearby occupied cell, `dragHoverCell !=
dragOrigin` was trivially true and every tap that snapped fired
a move. Replaced the cell-difference check with a physical-
distance gate — `hypot(end - initialTouch) > 16pt` — so small
finger jitter still resolves as a tap.

**Apple Pencil hover.** Added `.onContinuousHover` over the
canvas: when the Pencil hovers a cell on the active layer, the
cell is outlined (gray for empty, accent for occupied). Helps
the user line up Pencil taps before committing.

**Distribute — Bresenham cell walk.** User reported the
distribute action put every new pixel at the starting pixel's
cell. Root cause was the lerp-and-round formula: when the two
endpoints were close (e.g. `from=(5,3)`, `to=(5,4)`), most
fractional positions rounded to one endpoint or the other, the
collision-skip then dropped every placement, and visually the
selected pixel "absorbed" the requested distribution.

Replaced the formula with Bresenham's line walk
(`cellsAlongLine(from:to:)`), strip the two endpoints, then
sample the intermediate strip at `(i + 0.5) / n` fractions. The
placement count is clamped to the available intermediate cells
so adjacent endpoints just no-op instead of silently dropping
work. Numbering shift is now based on `picked.count` (the
actual placement count) rather than the requested count.

### J-23.7 — Explicit Distribute button + wider W/H fields ✓ 2026-05-15

**Distribute button.** Tap-on-line detection (J-23.5/.6) kept
losing taps to "place new pixel" because both gestures live on
empty cells. Ripped it out and replaced with an explicit
button in the selection bar:

- Tap a pixel to select it.
- The bar now also shows "Distribute → #N" (when the
  selected pixel has a higher-numbered sibling). N is the
  next pixel by number, computed via a single O(w·h·d) scan.
- Tapping the button opens the existing
  `DistributePointsSheet` with that segment preselected.

Unambiguous, hit-test independent, and the bar already had
Renumber / Delete so it's the natural home for a third
per-pixel action.

**Wider W/H fields.** User reported 3-4 digit values (352,
157) were overflowing the W/H spin fields. Two fixes:

- `LayoutEditorIntSpin`'s inner TextField bumped from
  `maxWidth: 70` → `95` (applies app-wide).
- The Custom editor's W/H outer frames bumped from
  `maxWidth: 120` → `160`; D from `100` → `140`.

### J-23.6 — Line-tap detection fix ✓ 2026-05-15

User feedback after J-23.5: tap-on-line wasn't opening the
distribute sheet — it just placed a pixel. Three contributing
issues, fixed together:

**Movement gate too strict.** `movement < 6` was suppressing
legitimate taps where the finger jittered ≥6 pt before lift.
Dropped the movement check entirely — cell-level "the start
and end cell are the same empty cell" is a sufficient signal
that the user didn't intend a drag.

**Threshold too tight on dense grids.** Was `cellSide * 0.5`,
which on a 100×100 grid filling an 800-pt canvas is just 4 pt
— easy to miss. Bumped to `max(cellSide * 0.5, 18)` so the
hit-test always has a generous minimum reach regardless of
cell size.

**Async geoSize race.** The canvas gesture was reading metrics
via `canvasGeoSize` @State which `capture(geoSize:)` updates
through `DispatchQueue.main.async`. Plumbed `geo.size`
directly into the gesture factory (`canvasGesture(geoSize:)`)
so metrics are derived from the same value the body just used
for rendering — no async window, no stale read.

### J-23.5 — Hit-test fix + Pan mode + Distribute-along-line ✓ 2026-05-15

User feedback after J-23.4: hit-tests landed at random cells
after zoom; two-finger pan never worked; and a line +
distribute-points feature would be valuable.

**Hit-test fix:**

The gesture closure was capturing `cellSide` / `xOff` / `yOff`
as local lets at body-eval time. SwiftUI sometimes hangs onto
an older closure across renders, so a tap that fired through
the older closure used stale post-resize / post-zoom math.

Fixed by stashing `geo.size` into a `@State canvasGeoSize` and
adding `currentMetrics()` that re-derives `cellSide` / `xOff` /
`yOff` from live @State on every gesture callback. The
`canvasGesture()` factory no longer takes parameters; metrics
recompute inside `onChanged` / `onEnded`.

**Pan mode toggle:**

Native SwiftUI doesn't expose a touch-count on DragGesture, so
the previous "drag when magnification is active" approach
missed pure pans (no pinch movement → `MagnificationGesture`
doesn't fire → `multiTouchActive` stays false → pan never
triggers). Replaced with a simple toolbar toggle:

- `hand.draw` button in the top bar flips `panMode`.
- When ON: single-finger drag scrolls (`panOffset = panCommitted
  + value.translation`); `.onEnded` snaps `panCommitted` to
  the new value so subsequent drags continue from there.
- When OFF: single-finger drag places / moves pixels (the
  original behaviour).

The pinch gesture still works for live zoom regardless of pan
mode.

**Lines + Distribute Along Line:**

Lines render in a single Canvas stroke pass between
consecutive pixels (sorted by pixel number) for the active
layer. A tap on a line (within `cellSide * 0.5` perpendicular
distance, with `<6 pt` movement so taps-on-pixels still place
new ones) opens `DistributePointsSheet`:

- Shows the segment's two endpoints.
- A spin field asks for the number of points to add.
- On Apply: linearly interpolates `count` positions between
  the from / to cell centres, rounds each to its containing
  cell, and assigns numbers (fromNumber+1) ... (fromNumber+N).
- All pixels with numbers > fromNumber shift up by N so the
  numbering stays contiguous. Cells already taken
  (post-shift) are skipped rather than overwritten.

### J-23.4 — Custom editor full-screen + zoom + pan ✓ 2026-05-15

User feedback: "Editor needs to be a LOT bigger (close to full
screen size on 10" iPad). Likely needs ability to zoom in to
place things better."

**Full-screen presentation:**

- Switched `.sheet(item:)` → `.fullScreenCover(item:)`. Removed
  the `.presentationDetents([.large])` constraint. The canvas
  now gets the entire window on iPad.

**Pinch-to-zoom (+ slider):**

- New `zoom` state (0.5..5.0) multiplied into the base
  `cellSide`. `MagnificationGesture` updates it live; on
  `.onEnded` the committed value snaps to the live value.
- A slider in the top toolbar (with - / + magnifying-glass
  icons and a `1×` reset button) drives the same `zoom`
  state, so users can pick a zoom level explicitly or pinch
  for fine control.

**Two-finger pan:**

- SwiftUI has no built-in two-finger drag. Approximated via a
  second `DragGesture(minimumDistance: 0)` simultaneous with
  the pinch — only updates `panOffset` while `multiTouchActive`
  is true. Pinch sets that flag on .onChanged and clears it
  asynchronously on .onEnded so the trailing finger-up doesn't
  trigger a stray pixel tap.
- Single-finger DragGesture (tap-to-place / drag-to-move) now
  early-returns when `multiTouchActive` is true so pinches /
  pans never mutate pixels.

**Resize resets pan:**

- Changing Width / Height resets `panOffset` so the user
  doesn't get stuck with the canvas off-screen after a
  resize-while-zoomed.

### J-23.1 / J-23.2 — Custom editor polish ✓ 2026-05-15

Follow-ups to the J-23 MVP.

**Drag-to-move:** Single `DragGesture(minimumDistance: 0)` on
the canvas now handles both taps and drags. `onChanged` tracks
the cell under the touch (records the origin's value on first
event so we know whether it's a "tap on empty" / "tap on pixel"
/ "drag a pixel"). `onEnded`:
- If origin had a pixel and the end cell is different → move
  (or swap if the end cell was already occupied).
- Otherwise → fall through to `handleTap` for tap-to-place /
  tap-to-select.

Drop-target preview: while dragging an occupied cell, the
canvas paints a dashed circle + faded number in the hover cell
so the user can see where the pixel will land. Selection
highlight remains on the origin until the gesture ends.

**Background image:** `customModelDataForModel:` now also
surfaces `backgroundImage`, `backgroundScale`,
`backgroundBrightness`. The editor loads the image via
`UIImage(contentsOfFile:)` in a `.task(id:)` block (off the
main thread so a huge texture doesn't stall the canvas), then
draws it under the gridlines inside the `Canvas` pass. Scale %
sizes the image relative to the model bounds (100 = fit);
brightness % modulates `ctx.opacity`.

### J-23.3 — SubModel geometry editor ✓ 2026-05-15

Adds the full per-submodel detail editor on iPad. Replaces the
"swipe to delete only" list with a NavigationLink-per-row that
drills into a per-submodel detail view.

**Bridge:**

- `submodelDetailsForModel:` returns an array of dicts (one per
  submodel) with name, isRanges, isVertical, bufferStyle,
  strands (array of range strings), subBuffer (string).
- `replaceSubModelsOnModel:withEntries:` wholesale-replaces the
  parent's submodel list. Internally calls
  `RemoveAllSubModels()` then re-creates each entry via the
  `new SubModel(...)` + `AddDefaultBuffer` / `AddRangeXY` /
  `AddSubbuffer` pattern desktop uses in
  SubModelsDialog::Save.

**SwiftUI:**

- `SubModelEntry` value type carries the editable fields.
- `SubModelListSheet` rebuilt: list of submodels with
  `NavigationLink(value:)` per row; add + swipe-delete at the
  list level still work via the existing per-submodel
  mutators.
- `SubModelDetailEditor` drills into one submodel: Name +
  Type picker (Ranges vs Sub-buffer) + Orientation toggle +
  Buffer Style picker + ranges editor (Add / Delete / Edit
  per line) or sub-buffer rectangle field.
- Save on the detail view commits via the wholesale-replace
  bridge so type / orientation / buffer style changes survive
  (those fields are `const` on the C++ SubModel — only
  RemoveAllSubModels + re-add can change them).

**Type-change caveat:** Switching a submodel from Ranges to
Sub-buffer (or vice versa) on Save reconstructs ALL submodels
through `replaceSubModelsOnModel:`. That's the same pattern
the desktop dialog uses; it's idempotent for the other
submodels.

### J-23 — Custom-model visual editor (MVP) ✓ 2026-05-15

User feedback: "The current editor on the desktop is complete
crap. Entering node numbers into a grid is slow an annoying."
Replaces the type-numbers-into-cells dialog with a point/click
flow.

**Editor UX:**

- **Grid size:** W / H / D spin buttons in the top bar. Resizing
  preserves existing cells inside the new bounds; new cells
  are empty. Live (no separate "apply" step).
- **Layer slider:** appears when Depth > 1. Selects which 2D
  slice the canvas edits.
- **Canvas:** SwiftUI `Canvas` draws gridlines + a numbered
  accent-color circle in each filled cell. One pass — no
  per-cell UIView spawn, so grids with thousands of cells stay
  responsive.
- **Tap empty cell:** places the next free pixel number, or
  *moves* the currently-selected pixel (mirrors the desktop's
  drag-to-move intent but with a simpler tap-then-tap idiom
  that survives touch loss).
- **Tap occupied cell:** selects it. Bottom bar shows
  "Pixel #N @ (col, row)" plus Renumber / Delete buttons.
- **Renumber:** alert prompts for the new number. If that
  number already exists elsewhere, the editor swaps — the
  other cell takes the old number. (Avoids holes in the
  numbering.)
- **Clear All:** when nothing is selected, the bottom bar
  surfaces a destructive Clear All button that wipes the
  grid.
- **Save:** wholesale-replace via the new
  `setCustomModelData:width:height:depth:locations:` bridge.

**Bridge:**

- `customModelDataForModel:` returns
  `{ width, height, depth, locations: [[[NSNumber]]] }`.
- `setCustomModelData:width:height:depth:locations:` writes
  back; the bridge pads / truncates so dim mismatches don't
  corrupt the grid.

**Still parked:**

- Background image overlay on the editor canvas. (Already
  surfaced as Custom Model props — Image / Scale / Brightness
  — just not rendered behind the grid yet.)
- Drag-to-move (current flow is tap-old-then-tap-new).

### J-22 — Faces / States / Dimming Curve / SubModels editors ✓ 2026-05-15

Closes the popup-editor parity gap on the Model Data section.
Three of the categories (Faces, States, Dimming Curve) share
the desktop's `FaceStateData` shape — `map<name, map<attr,
value>>` — so one bridge surface + one generic SwiftUI editor
covers all three. SubModels keeps its own list-shaped editor
since each submodel is a structured object (name + geometry).

**Generic FaceState editor:**

- Bridge: `setFaceInfo:entries:`, `setStateInfo:entries:`,
  `setDimmingInfo:entries:` (wholesale replace) and the
  read-side `faceInfoForModel:` / `stateInfoForModel:` /
  `dimmingInfoForModel:`. All five route through their
  matching `Model::Set*Info` / `Get*Info` accessors. The
  bridge converts between Swift dictionaries and the C++
  `FaceStateData` via shared helpers.
- States additionally call `MarkModelStateDirty` so the DMX
  state-info save path picks the change up too.
- SwiftUI `FaceStateEditorSheet` is a two-level navigation:
  list of entries (face / state / channel names) → tap to
  drill into a per-attribute editor. Top-level supports
  Add / Delete entries (delete = swipe). Detail view has
  suggested-key chips (standard phonemes for Faces, channel
  attributes for Dimming), Add Attribute, Delete Attribute
  (swipe), and an editable value field per key.
- Wired into the Model Data section: Faces, States, and the
  Dimming Curve row all open the editor. Dimming row keeps its
  trash icon for one-tap clear.

**SubModel add + rename:**

- Bridge: `addSubModelToModel:name:` creates a new SubModel
  with sensible defaults (horizontal / ranges / "Default"
  buffer style / single placeholder "1-1" range), mirrors the
  desktop SubModelsDialog's brand-new-row behaviour.
- Bridge: `renameSubModelNamed:onModel:to:` sanitises via
  `Model::SafeModelName`, refuses collisions, updates the
  SubModel's name field directly + marks the parent dirty.
- `SubModelListSheet` gained an Add row + per-row Rename
  pencil. Delete still by swipe.
- Geometry editing (ranges / lines / sub-buffer) intentionally
  remains desktop-only — needs a real per-submodel grid /
  range editor that's outside this slice.

**Still deferred:**

- SubModel geometry (ranges / lines / sub-buffer) editor.
- Custom-model 2D grid (channel-per-cell) editor.

### J-21.1 — PolyLine segments + corners + drop pattern ✓ 2026-05-15

Closes the last deferred non-grid item from the PolyLine
adapter parity audit. Custom-model 2D grid editor is the only
remaining parked piece.

**Per-segment node count:**

- `GetNumSegments()` is `vertexCount - 1`; each segment gets a
  `Segment N` row (key `PolySegmentSize<N>`) with a spin int.
- Setter routes through `PolyLineModel::SetRawSegmentSize` and
  flips `SetAutoDistribute(false)` so the user's manual count
  survives — mirrors desktop's behaviour.

**Per-corner enum (Leading / Trailing / Neither):**

- One corner per vertex; `numSegments + 1` rows total. Each row
  (`PolyCorner<N>`) is a 3-choice enum routing through
  `SetCornerString` plus the desktop's enum-to-offset math:
  Leading → lead 1.0 / trail 0.0; Trailing → lead 0.0 / trail
  1.0; Neither → 0.5 / 0.5. First / last corners only touch
  one side (the corresponding `SetLeadOffset` or
  `SetTrailOffset`).

**Drop Pattern:**

- Was missing entirely on iPad. Added as a shared `IciclesDrops`
  key (same as Icicles) since both `IciclesModel` and
  `PolyLineModel` expose `SetDropPattern`. Bridge's setter
  dispatches on the live type.

### J-21 — Wrap-up of Layout Editor deferreds ✓ 2026-05-15

Closes the long tail of Models-tab parity gaps. Three items off
the deferred list; one (Custom-model 2D grid editor) stays
parked because it needs its own slice.

**Superstring per-index colour set:**

- iPad bridge's `superStringColour<N>` setter was a no-op
  (returned NO with a TODO). Replaced with a routed call to
  `Model::SetSuperStringColour(int, xlColor)` which the upstream
  Model API already exposes — the bridge just wasn't using it.
- SwiftUI's `Colour N` row was a read-only Text label;
  upgraded to a SwiftUI `ColorPicker` round-tripping through
  the shared sRGB-pinned helpers. Per-index colour edits now
  persist.

**MultiPoint / PolyLine Indiv Start Nodes:**

- Mirrors the desktop `ModelIndividualStartNodes` pattern.
  When `GetNumStrings() > 1`, the model exposes a toggle that
  surfaces N per-string start-node spin fields. Same shape as
  the existing Indiv Start Channels surface from J-18.
- Bridge: `ModelIndividualStartNodes` toggle key, plus per-
  string `IndivStartNode<N>` keys. Routes through
  `Model::SetHasIndivStartNodes` + `SetIndivStartNode`. Add
  / pop entries to match the string count via the existing
  `AddIndivStartNode` / `SetIndivStartNodesCount` mutators.

**Custom Model basic props:**

- Was: read-only matrix-dimension display (Width/Height/Depth).
- Adds: # Strings (1..100), Indiv Start Nodes (shared pattern
  with MultiPoint/PolyLine), Background Image (uses the
  `imageFile` descriptor kind from J-20.2 so the existing
  file-picker pipeline drives it), Background Scale % (10..500),
  Background Brightness % (0..100). Each routes through the
  matching `CustomModel::Set*` mutator.

**Still deferred for a later slice:**

- Custom-model 2D grid editor (the channel-number-per-cell
  popup). Desktop's CustomModelDialog is a substantial 2D
  editor; needs its own design pass. iPad shows the dims
  read-only so users see what they have, edit on desktop.
- PolyLine segment / corner editors — per-vertex segment node
  counts and corner-style (Leading / Trailing / Half) enums.
  Needs a per-vertex list UI; deferred.

### J-20.3 — Editable spin buttons ✓ 2026-05-15

User feedback: the J-20 bare-Stepper rows showed no value —
`.labelsHidden()` was suppressing the label closure that
contained the value Text. The right idiom is a real spin-button
widget: an editable numeric field next to ± buttons.

**New widgets:**

- `LayoutEditorIntSpin` — TextField (numeric keyboard, sRGB-style
  focus / commit / clamp logic mirrored from the existing
  `LayoutEditorDoubleField`) + `Stepper("", value:in:step:)
  .labelsHidden()` to the right. Editing the field commits on
  submit / blur; tapping ± commits immediately.
- `LayoutEditorDoubleSpin` — same shape with step + precision
  parameters so floats render the right number of decimals.

**Replaced sites:**

- Per-type descriptor `int` and `double` cases in
  `typeDescriptorControl` — small-range fields now get spin
  widgets, larger ones keep the plain numeric field.
- `lowDefFactorField` in the Model header.
- `ccIntField` and `ccDoubleField` in the Controller Connection
  section.
- `StartChannelEditorSheet`'s Channel + Universe rows.

**Steppers that stay as-is:**

- The two Steppers inside Form `Section`s in
  `StartChannelEditorSheet` would have shown values fine (label
  visibility is the Form default), but we converted them to
  spin widgets for consistency.

### J-20.2 — Image file picker + DisableUnusedProperties parity + MultiPoint/PolyLine basics ✓ 2026-05-15

Final push on the Models-tab polish before re-prioritising.

**Image file picker:**

- Per-type descriptor pipeline gained a new `imageFile` `kind`.
  SwiftUI renders a truncated path label + folder icon button +
  clear button. Folder button opens a `.fileImporter` scoped to
  image UTTypes; on success the security-scoped path goes
  through `ObtainAccessToURL` (for persistent bookmark) and
  commits via `setPerTypeProperty`.
- `BuildImageProps` now emits the `Image` field as `imageFile`
  instead of plain `string`, so the picker is wired
  automatically.

**DisableUnusedProperties parity:**

- Bridge surface adds a `disabledKeys` array on `modelLayoutSummary`
  listing iPad-bridge keys that the current model type's adapter
  disables on desktop:
  - **Image** disables: faces, states, submodels, strands, nodes,
    stringType, stringColor, rgbwHandlingIndex, dimmingCurve,
    pixelSize.
  - **Label** disables: same as Image plus pixelStyle and
    blackTransparency.
  - **ChannelBlock** disables: stringType, stringColor,
    rgbwHandlingIndex.
- SwiftUI applies `.opacity(0.45)` + `.disabled(true)` to the
  matching rows. Model Data popup rows hide their pencil/eye
  icon when disabled so users don't tap into a non-functional
  editor.

**MultiPoint per-type basics:**

- `BuildMultiPointProps` mirrors the desktop's
  `MultiPointPropertyAdapter` minus the Indiv Start Nodes
  surface: # Lights / # Nodes (read-only — canvas-driven),
  Strings (1..48), Height. Indiv Start Nodes per-string editor
  deferred — would mirror the existing Indiv Start Channel
  pipeline once the desktop pattern is ported.
- Setters route through new MultiPoint dispatches.

**PolyLine per-type basics:**

- `BuildPolyLineProps` exposes # Lights / # Nodes (read-only),
  Lights/Node, Strings, Starting Location (Green/Blue),
  Alternate Drop Nodes, Height. Segment / corner / per-string
  start node editors deferred — those are intricate enough to
  warrant their own slice.
- `AlternateNodes` setter extended so the shared key now flips
  through to PolyLine's setter via dynamic_cast.

**Still deferred:**

- Custom-model node-grid editor.
- DMX family (per user direction).
- PolyLine / MultiPoint Indiv Start Nodes + segment + corner
  editors.
- Superstring per-index colour set (upstream Model API).

### J-20.1 — Model header extras + more per-type audits + stepper polish ✓ 2026-05-15

Follow-up to J-20 covering the remaining model-header rows and
several per-type builders the first audit pass left alone.

**Model header (every model):**

- `Low Definition Factor` (Stepper, 1..100). Only rendered when
  `Model::SupportsLowDefinitionRender()` returns true (matrix /
  image types today).
- `Shadow Model For` picker. Surfaces every non-group model
  other than self via a new `shadowModelOptions` array. Empty-
  string sentinel reads as `(none)`. Mirrors desktop's
  `OTHERMODELLIST` build.
- Bridge: `setLayoutModelProperty` handles `lowDefinitionFactor`
  and `shadowModelFor` keys.

**Image (DisplayAsType::Image):**

- New `BuildImageProps` exposes Image file path, Off Brightness
  (0..200), Read White As Alpha — mirrors desktop
  `ImagePropertyAdapter`. File path is a plain string field for
  now; a proper file picker affordance is a follow-up.
- Setters: `Image`, `OffBrightness`, `WhiteAsAlpha` (each
  clears the image cache when needed).

**Label (DisplayAsType::Label):**

- New `BuildLabelProps` exposes Label Text, Font Size (8..40
  Stepper), Text Color — mirrors desktop `LabelPropertyAdapter`.
- Introduced a new `color` `kind` in the type-descriptor
  pipeline so per-type colour rows route through the same
  sRGB-pinned `hexColor` / `hexFromColor` helpers as the
  layout-editor pickers (no Display-P3 drift).

**Tree:**

- Was calling `BuildMatrixProps` then appending tree-specific
  fields — that produced a duplicate "Direction" enum.
  Desktop `TreePropertyAdapter` replaces Matrix's Direction
  with `StrandDir` ("Strand Direction"). Rebuilt `BuildTreeProps`
  to emit the tree-specific block first (Type + round-tree-only
  floats + Alternate Nodes / Don't Zig Zag with mutex gating),
  then the inherited matrix props, finishing with `StrandDir`
  instead of `MatrixStyle`.
- Setter aliases `StrandDir` → `MatrixModel::SetVertical` so
  the same in-memory state drives both keys.

**Circle:**

- Added the missing `LayerSizes` editor (matches desktop's
  `AddLayerSizeProperty`). Layer-size edit reuses the same
  `LayerSizesEditorSheet` from J-19 / Arches.

**Controller Connection stepper polish:**

- `ccIntField` and `ccDoubleField` now pick a SwiftUI Stepper
  when the range fits the ±-tap idiom (int: span ≤ 1000;
  double: (span / step) ≤ 200). Keyboard text field for
  anything larger. Cuts down on keyboard summons for port,
  brightness, gamma, etc.

**Still deferred:**

- PolyLine / MultiPoint per-type (indiv start nodes + segment +
  corner editors).
- Custom-model node-grid editor.
- DMX family (per user direction).
- Image-file picker UI on the Image type.
- Superstring per-index colour set (upstream Model API).

### J-20 — Controller Connection + Start Channel + model-type audit ✓ 2026-05-15

Continues the J-19 restructuring with the three biggest remaining
items the user called out: a real Controller Connection section,
a structured Start Channel editor, and a parity audit of the
per-type property builders against their desktop adapters.

**Real Controller Connection section:**

- New `controllerConnectionFor:` bridge helper assembles a single
  `controllerConnection` sub-dictionary surfacing every field the
  desktop's `AddControllerProperties` builds:
  - `port` + dynamic `portMax` (driven by `ControllerCaps` and
    the active protocol family — Serial / Pixel / LED Panel /
    Virtual Matrix / PWM each get their own max).
  - `protocolOptions` + `protocolIndex` (driven by
    `Model::GetControllerProtocols`).
  - Smart-Remote subsection (pixel + caps>0): `useSmartRemote`,
    `smartRemoteTypeOptions/Index`, `smartRemoteOptions/Index`
    (the A/B/C… letter values from `GetSmartRemoteValues`),
    `srMaxCascade` + `srCascadeOnPort` when
    `GetNumPhysicalStrings() > 1`.
  - Serial-only: `dmxChannel` (with caps-driven max) +
    `speedOptions/speedIndex`.
  - PWM-only: `pwmGamma`, `pwmBrightness`.
  - Pixel per-property toggle / value pairs gated by
    `caps->Supports*` (or unconditional when caps is null —
    matches desktop's "old controllers" path): Start Null
    Pixels, End Null Pixels, Brightness, Gamma, Color Order,
    Direction, Group Count, Zig Zag, Smart Ts.
- `setLayoutModelProperty` routes ~25 new `cc.*` keys through
  the model's `ControllerConnection` instance, including
  `UpdateProperty(CTRL_PROPS::XXX_ACTIVE)` toggles and the
  matching value setters.
- SwiftUI: brand-new `controllerConnectionFields` ViewBuilder
  on `LayoutEditorPropertiesView` builds the section
  dynamically from the sub-dictionary. Helpers
  (`ccPortRow` / `ccProtocolRow` / `ccSmartRemoteRows` /
  `ccPixelToggleRow` / `ccPixelEnumToggleRow` /
  `ccIntField` / `ccDoubleField` / `ccEnumPicker`) keep the
  per-row plumbing compact.
- The old pseudo-section that mixed Preview + Controller +
  Start Channel into "Controller Connection" is now the
  "Model" header section, sitting between Type and the real
  Controller Connection.

**Start Channel structured editor:**

- New `StartChannelEditorSheet` (iPad equivalent of desktop's
  `StartChannelDialog`). Pencil button next to the Start
  Channel field opens the sheet; reads the existing value,
  parses it into one of the five formats, lets the user edit
  via a mode picker + per-mode fields, recomposes the wire
  string on save. Modes mirror the desktop dialog:
  - **Channel #** — raw integer (`123`).
  - **Universe** — `#<universe>:<channel>` or
    `#<ip>:<universe>:<channel>` (IP "ANY" collapses to the
    2-segment form).
  - **From Model** — `>ModelName:N` (chain relative).
  - **Start Of Model** — `@ModelName:N`.
  - **Controller** — `!ControllerName:N`.
- Live preview row in the sheet shows the recomposed string
  before save. Save commits via the existing
  `setLayoutModelProperty` "modelStartChannel" key.

**Per-type property audit + fixes:**

Walked the iPad's `Build*Props` builders vs the matching
desktop `*PropertyAdapter::AddTypeProperties`. Fixed:

- **Star** — was using `StartCornerOptions()` (4-entry matrix
  corners) for `StarStart`; desktop uses a 12-entry Star-
  specific vocabulary ("Top Ctr-CCW", "Top Ctr-CW", …, "Right
  Bottom-CCW"). New `StarStartLocationOptions()` helper +
  setter fix. Also gained the `LayerSizes` editor (matches
  desktop's `AddLayerSizeProperty(grid)`), and the "Inner
  Layer %" row is now properly conditional on
  `GetNumStrands() > 1`.
- **Sphere** — `Degrees` range was 1..360, desktop is 45..360;
  `Southern Latitude` was -90..90, desktop is -89..-1;
  `Northern Latitude` was -90..90, desktop is 1..89. Labels
  fixed too ("Start" / "End" → "Southern" / "Northern"). Row
  order corrected: Degrees + Latitudes come BEFORE the matrix
  block on desktop.
- **Cube** — was emitting raw int fields for `CubeStyleIdx`
  and `CubeStartIdx` with placeholder ranges (0..99) instead
  of the desktop's named enums. Now exposes the 8-named
  starting locations + 6-named direction styles + 3-named
  strand styles. Added the missing `StrandPerLayer` (Layers
  All Start in Same Place) toggle. Width/Height/Depth ranges
  corrected from 1..1000 to 1..100.
- **Spinner** — full rebuild. Was missing Starting Location
  enum entirely (6 choices: Center CCW/CW, End CCW/CW, Center
  Alternate CCW/CW). Key names corrected to match desktop
  (`FoldCount`, `SpinnerArmNodeCount`, `Hollow`, `Arc`,
  `StartAngle`, `ZigZag`) — old `Spinner*` aliases kept as
  fallbacks for compatibility. Ranges fixed (e.g. # Strings
  1..100 → 1..640, Arms/String → 1..250, Hollow 0..95 →
  0..80, Start Angle 0..360 → -360..360). Zig-Zag now gated
  on Alternate Nodes being off. New
  `SpinnerModel::DecodeStartLocation`-aware setter for
  `MatrixStart`.

Models verified parity-equivalent on a quick scan (no changes
needed today): Matrix, Tree, Icicles, CandyCane, Circle,
Wreath, SingleLine, WindowFrame, ChannelBlock. Custom remains
read-only — needs a grid editor that's still deferred.

**Steppers for small-range numeric inputs:**

- `typeDescriptorControl(.int)` now picks a SwiftUI `Stepper`
  when `max - min ≤ 1000`, and the regular numeric text field
  otherwise. Sweet-spot threshold matches the kind of values
  users want to nudge by ±1 — 360 degrees, percent, number of
  strings, etc. Bigger ranges (10 000 nodes) keep the keyboard
  field because step-by-1 is impractical.

**Deferred for later:**

- Per-type rebuild for: Image / Label / MovingHead / Servo /
  MultiPoint / PolyLine / SinglePixel / various DMX types
  (DMX family explicitly deferred by user).
- Custom model node-grid editor.
- StartChannelDialog "from this preview only" filter and
  controller validation (warn when channel > controller
  channels).
- Superstring per-index colour set (still requires upstream
  Model API).

### J-19 — Models-tab restructuring + colour-space audit ✓ 2026-05-15

Wholesale correction of the Models-tab property pane against the
desktop reference (`src-ui-wx/modelproperties/`). Pass-1 (J-18)
was a simplified facsimile that got several things wrong — this
restores the desktop's actual section layout, dynamic surfaces,
and screen-location class branching.

**Section ordering + Description (Phase 1):**

- Description moved out of Appearance and into the model-header
  row (between Type and the per-type section), matching desktop.
- The bottom-of-pane "Display" roll-up (canvas / 2D center=0 /
  grid / bounding box) deleted from the Models tab and surfaced
  read-only on the 2D Background object instead — desktop treats
  these as per-layout-group settings, not model settings.
- Default Mode (3D / 2D toggle) removed entirely — it's a
  per-session preview state, not a model property.

**Arches per-type rebuild (Phase 2):**

- `BuildArchesProps` now mirrors `ArchesPropertyAdapter::AddType
  Properties` verbatim:
  - `Layered Arches` checkbox is the FIRST property.
  - When OFF: `# Arches` + `Nodes Per Arch`; when ON: a single
    `Nodes` field + the per-layer size editor + `Hollow %` +
    `Zig-Zag Layers`.
  - `Lights Per Node` + `Arc Degrees` always; `Arch Tilt`
    always (was missing); `Gap Between Arches` only when not
    layered.
  - `Starting Location` has 2 choices when not layered, 4
    choices (Green/Blue × Inside/Outside) when layered.
- Bridge gained setters for `LayeredArches`, `LayerSizes`
  (wholesale-replace via NSArray<NSNumber*>), `ArchesSkew`,
  `ArchesGap`, `ArchesStart` (encodes the 2/4-choice variants
  via `SetDirection` / `SetStartSide` / `SetIsBtoT`).
- New `LayerSizesEditorSheet` — `NavigationStack` + `List` of
  per-layer node-count fields with swipe-to-delete and an
  "Add layer" button.

**Controller picker + Start-Channel gate (Phase 3):**

- Controller is now an enum picker, not a free-text field.
  Options: `Use Start Channel` + `No Controller` + every
  auto-layout controller name from
  `OutputManager::GetAutoLayoutControllerNames()`. Mirrors
  desktop's CONTROLLERS array exactly.
- Bridge writes back via a new `controllerSelection` key:
  `Use Start Channel` becomes `""` on the model;
  `No Controller` and named controllers round-trip verbatim.
- Start Channel field is now read-only display unless the
  picker is set to `Use Start Channel`. Matches desktop's
  `Enable(GetControllerName() == "" || _controller == 0)`.
- Indiv-string Start Channel rows get the same gate.

**String Properties dynamic surface (Phase 4):**

- Sub-controls under String Type now depend on the type:
  - `Single Color` / `Single Color Intensity` / `Node Single
    Color`: a `Color` row with a `ColorPicker` driving
    `Model::SetCustomColor` via the new `stringColor` key.
  - `Superstring`: a `Colours` count field (1..32) + per-index
    `Colour N` display rows. Count writes through to
    `Model::SetSuperStringColours`; per-index colour set is
    deferred (Model lacks a vector mutator — would require
    adding `SetSuperStringColour(int, xlColor)` upstream).
  - Other types: disabled `—` placeholder.
- `RGBW Color Handling` picker (5 options) appears on every
  type but is enabled only when `!HasSingleChannel && Channel
  Count >= 4`. Bridge: `rgbwHandlingIndex` → `Model::
  SetRGBWHandling` via the index-to-string mapping.

**Size/Location by screen-location class (Phase 5):**

- Bridge now exposes `screenLocationKind` (`boxed` /
  `twoPoint` / `threePoint` / `other`) + `screenLocation
  Fields` dictionary with class-appropriate keys.
- SwiftUI's Size/Location section branches:
  - **Boxed**: `X`/`Y`/`Z` + `ScaleX`/`ScaleY`/`ScaleZ` (with
    `supportsZScaling` gating ScaleZ) + `RotateX`/`RotateY`/
    `RotateZ`.
  - **TwoPoint**: `World X/Y/Z` + `X1/Y1/Z1` + `X2/Y2/Z2`.
    Editing X1 shifts world + back-shifts X2 to keep the far
    endpoint stationary (matches desktop).
  - **ThreePoint**: TwoPoint + `Height` (clamped to |h|≥0.01)
    + `Shear` (when `GetSupportsShear()`) + `RotateX`.
  - **Other (PolyPoint etc.)**: World only + a "edit via
    handles" hint — per-vertex editing already lives in the
    canvas gesture path.

**Colour-space audit (cross-cutting):**

Triggered by the user's note: SwiftUI's `ColorPicker` hands
back `Color` values in the *device* colour space — on Display-P3
iPads that's extended-sRGB, and reading those components
directly mangles the exact `#RRGGBB` hex the user typed. The
fix is to always convert through `CGColorSpace.sRGB` before
reading bytes.

Audited call sites:
- `Color.hexString` extension (`LayoutEditorView.swift`):
  was `UIColor(self).getRed`; now does `cgColor.converted(to:
  sRGB)` and reads components from the converted CGColor.
- `Color(hexString:)` extension: already sRGB-pinned via
  `init(.sRGB, ...)`.
- `colorFromHex` (`ColorPanelCustomRows.swift`): switched
  from `Color(red:green:blue:)` (device space) to
  `Color(.sRGB, red:green:blue:opacity:)`.
- `ColorPaletteView.colorFromHex` (struct method): same fix.
- Layout-editor `hexColor` / `hexFromColor` helpers used by
  the new `stringColorRow`: both pinned to sRGB.
- `XLColorCurve.mm xlColorFromUI` (ObjC++): was using
  `UIColor.getRed` which reads in the source colour space;
  now always converts via `CGColorCreateCopyByMatchingTo
  ColorSpace(sRGB, …)` before reading bytes. UIColor used in
  curve `setPoint` calls comes from `UIColor(newColor)` which
  preserves the source space, so the conversion at the C++
  boundary is what makes the round-trip bit-exact.

**Deferred items (recorded but not in this pass):**

- The REAL "Controller Connection" section: port / protocol /
  smart-remote subsection / null-pixels / brightness / gamma /
  color order / direction / group count / zig-zag / smart Ts.
  Big bridge surface — its own slice.
- StartChannelDialog: special editor with format picker
  (`1` / `#1:N` / `>Model1:1` / `@ip:univ:ch` / `&start_def`).
  Currently the field is gated read-only when not editable
  but the format-picker dialog isn't ported.
- Per-type rebuild parity for the other model adapters
  (Icicles / Cube / Sphere / Tree / Matrix / SingleLine /
  CandyCane / Spinner / WindowFrame / ChannelBlock / Custom).
  J-18 versions remain in place. Arches established the
  template; the rest are straight ports.
- Superstring colour per-index set: requires upstream Model
  API (`SetSuperStringColour(int, xlColor)`). Reads + count
  edits work today; per-index colour edits return NO.

### J-18 pass 6 — Dimming Curve clear ✓ 2026-05-14

First write on the Dimming Curve category — clear only. Curve
editing isn't in scope (would need a curve-picker UI + per-
type parameters — separate slice).

**Bridge** (`XLSequenceDocument`):

- `clearDimmingCurveOnModel:` — guards on `GetDimmingCurve() !=
  nullptr`, calls `Model::SetDimmingInfo({})` which deletes the
  cached `modelDimmingCurve` and empties the `dimmingInfo` map.
  Save's re-serialization writes no `<dimmingCurve>` child
  block when the map is empty (via
  `BaseSerializingVisitor::WriteDimmingCurve`).

**SwiftUI:**

- Dimming Curve row gained an inline trash button visible only
  when `hasDimmingCurve` is true. Tap → destructive alert with
  Cancel / Clear. Clear routes to a new
  `clearDimmingCurve(modelName:)` helper that pushes undo +
  bridge call + bumps save state.
- Alert message warns the user that editing isn't on iPad yet,
  so this is one-way (until the desktop puts a curve back).

### J-18 pass 5 — In Model Groups → tap to navigate ✓ 2026-05-14

UI-only: the "In Model Groups" entry on the Models tab is now a
tappable shortcut to the Groups tab with the picked group
selected. Editing membership still happens on the Groups tab —
this just removes the friction of switching tabs + scrolling to
find the group manually.

**SwiftUI:**

- New `GroupRefListSheet` — `List` of group names with a
  `chevron.right` affordance. Tapping a row calls back to
  close the sheet and navigate.
- `LayoutEditorPropertiesView` gained an `onNavigateToGroup`
  callback wired in the parent (`LayoutEditorView.body`) to
  flip `sidebarTab` to `.groups` and set
  `viewModel.layoutEditorSelectedGroup`.

### J-18 pass 4 — SubModel delete ✓ 2026-05-14

First write op on the SubModels category. Delete only — add /
rename / geometry editing each need their own slice because
SubModels carry per-instance range or line geometry that the
iPad has no UI for yet.

**Bridge** (`XLSequenceDocument`):

- `deleteSubModelNamed:onModel:` — looks up the parent Model,
  verifies the named submodel exists, aborts render, calls
  `Model::RemoveSubModel` (which deletes the SubModel object
  and erases it from `subModels` + `sortedSubModels`), marks
  the parent dirty so the next save rewrites the parent
  `<model>` node without the `<subModel>` child.
- Group / effect references to "Parent/SubModel" are NOT
  auto-cleaned — matches desktop behaviour. Users get a stale-
  member warning on next reload.

**SwiftUI:**

- New `SubModelListSheet` — `List` with `onDelete` swipe.
  Footer text explains delete is the only available op; editing
  requires desktop.
- `ModelDataKind.isEditable` now covers `.submodels` so the
  pencil icon appears on the row.
- The property pane gained an `onDeleteSubModel` callback
  routed to a new `deleteSubModel(modelName:submodelName:)`
  helper that pushes undo + the bridge call + bumps the
  save / undo state.

**Deferred for later passes:** SubModel add / rename / geometry
editing. Faces / States / Dimming Curve editing.

### J-18 pass 3 — Strand + Node name editing ✓ 2026-05-14

Next two popup categories lifted from read-only to editable.
Strand and node names are positional labels — slot N maps to
strand-N / node-N — so the editor is a fixed-length renamer,
not a list manager. Empty slots are preserved so positional
ordering survives a save → reload.

**Bridge** (`XLSequenceDocument`):

- `setStrandNames:names:` and `setNodeNames:names:` — wholesale-
  replace the model's strand / node name string. Both call into
  `Model::SetStrandNames` / `SetNodeNames` which parse a
  comma-delimited string into `strandNames` / `nodeNames`
  vectors. A new `joinIndexedNames:` helper does the
  array-of-NSString → comma-joined std::string conversion,
  stripping commas inside each entry (the delimiter would
  otherwise corrupt the wire format).
- Both methods abort render first (J-18.5), increment the
  change count, and mark the model dirty so the save path
  re-serializes the `<model>` node — strand/node-name attrs
  ride along via `BaseSerializingVisitor`.

**SwiftUI:**

- New `IndexedNamesEditorSheet` — `NavigationStack` + `List` of
  numbered `TextField` rows. The slot count is whatever the
  bridge sent in `extras` (lazy-filled to `GetNumStrands()` /
  `GetNodeCount()`); placeholders default to "Strand N" /
  "Node N" matching the desktop `GetStrandName(i, def=true)`
  fallback.
- `ModelDataKind.isEditable` now returns `true` for
  `.aliases`, `.strands`, `.nodes`.
- The property pane gained an `onCommitIndexedNames` callback
  routed to a new `commitIndexedNames(modelName:kind:names:)`
  that pushes undo + the bridge call + bumps the usual save /
  undo state.

**Deferred:** Faces / States / SubModels / Dimming Curve are
still view-only — those each warrant their own slice.

### J-18.5 — Abort render before every layout mutation ✓ 2026-05-14

**Why this matters:** the render engine holds raw `Model*` /
`ViewObject*` pointers across the whole frame-graph build. If a
mutation rewrites that state mid-render (rename, delete, move,
property change, …), the worker dereferences stale geometry and
crashes — the failure mode is rare in dev but lethal in
production once a real sequence has a real render in flight.

Desktop's `xLightsFrame::AbortRender` is called at the top of
every `LayoutPanel` mutation. iPad had the call site but the
underlying implementation didn't actually wait.

**Fix in three parts:**

1. `iPadRenderContext::AbortRender(int maxTimeMs)` now mirrors
   the desktop contract — signal abort, then block on
   `IsRenderDone()` (10 ms poll) until the in-flight jobs drain
   or the timeout elapses. Was a fire-and-forget `SignalAbort`
   that returned `true` whether or not the render was actually
   done.
2. `XLSequenceDocument.mm` — every layout-mutation entry point
   (about 20 methods) now calls `_context->AbortRender(5000)`
   before touching model / group / view-object state:
   - `setLayoutModelProperty`, `setLayoutModelGroupProperty`,
     `setLayoutViewObjectProperty`, `setPerTypeProperty`
   - `deleteModel`, `renameModel`, `deleteModelGroup`,
     `renameModelGroup`, `createModelGroup`, `addModel:toGroup:`,
     `removeModel:fromGroup:`, `deleteViewObject`,
     `renameViewObject`, `duplicateViewObject`,
     `createViewObjectWithType:`
   - `setModelAliases`, `setStrandNames`, `setNodeNames`,
     `setCurve`, `deleteVertexAtIndex`, `insertVertexInSegment`
   - `undoLastLayoutChange` (undo can rewrite arbitrary fields)
3. `XLMetalBridge.mm` — gesture and canvas-driven mutations:
   - `createModelOfType`, `importXmodelFromPath`,
     `duplicateModels`
   - `alignModels`, `distributeModels`, `matchSizeOfModels`,
     `flipModels`
   - `dragHandle`, `endHandleDragForDocument`, `moveModel`,
     `moveViewObject`, `editTerrainHeight`,
     `dragBody3DToScreenPoint`, `applyPinchScaleFactor`,
     `applyTwistRotationRadians`

**Cost when no render is in flight:** one virtual call +
`IsRenderDone()` atomic check, returns immediately. Effectively
free for the gesture hot path.

**Cost during an active render:** the first mutation in any
sequence blocks the main thread until workers drain (typically
tens of ms for a single-model render, longer for full-sequence
batches). Subsequent mutations in the same gesture are free —
the render is gone. Matches the desktop behaviour the user
already understands.

### J-18 pass 2 — Alias editing ✓ 2026-05-14

First popup category lifted from read-only to editable. Aliases
are the simplest of the popup-dialog surfaces (just a flat list
of lowercase strings), so they're the right vehicle to establish
the editor-sheet pattern.

**Bridge:**

- `setModelAliases:aliases:` — wholesale-replace the model's
  alias list. Strings get trimmed + lowercased + de-duped
  matching `Model::SetAliases` semantics. Marks the model
  dirty; SaveLayoutChanges fully re-serializes the `<model>`
  node, so child `<aliases>` elements come along automatically
  via `BaseSerializingVisitor::WriteAliases`.

**SwiftUI:**

- New `AliasEditorSheet` — Form with an "Add alias" row
  (TextField + Add button + live "Will save as 'foo'" preview)
  and an Aliases list with swipe-to-delete. Cancel discards;
  Save commits the full new list.
- `ModelDataKind.isEditable` gates the row icon: editable
  categories show a pencil (and stay tappable when empty so the
  first entry can be added); read-only categories keep the
  list-bullet and hide when empty.
- The property pane gained an `onCommitAliases` callback —
  separate from `commit` because aliases are a collection. The
  parent (`LayoutEditorView`) routes the callback to a new
  `commitAliases(modelName:aliases:)` that pushes undo + the
  bridge call + bumps `summaryToken` / `hasUnsavedChanges`.

**Deferred:**

- Faces / States / SubModels / Dimming Curve editing still
  view-only; each is its own pass.
- No per-alias edit (rename of an existing alias) — delete +
  re-add is fine for a list this short.

### J-18 — Models tab pass 1 ✓ 2026-05-14

First slice of the (huge) Models-tab editing surface. Scope picked
in conversation: rename + controller-connection editing +
read-only summaries for the popup categories. DMX models, popup
editing, and per-type Custom-model nodes are deferred.

**Model rename:**

- `renameModel:to:` — sanitizes via `Model::SafeModelName`,
  refuses collisions, refuses SubModel rename (those round-trip
  through the parent), calls `ModelManager::Rename` for the
  in-memory swap, then `MarkModelRenamed` + dirties the renamed
  model AND every group that directly contains the new name
  (the in-memory rename doesn't dirty group XML on its own).
- `iPadRenderContext::_renamedModels` map + `MarkModelRenamed`
  helper + extended HasDirty / Clear / SaveLayoutChanges guard.
- `SaveLayoutChanges` model patch branch consults the rename
  map: locates the on-disk `<model>` element by OLD name, then
  patches the `name` attribute + rest of the dirty fields.

**Controller Connection editing:**

- New summary keys: `modelStartChannel` (string),
  `hasIndividualStartChannels` (bool), `individualStartChannels`
  (array of per-string channel strings, lazy-filled to
  `GetNumStrings()`), `hasMultipleStrings`, `numStrings`,
  `modelChain`, `modelChainOptions`, `modelChainApplicable`.
- `setLayoutModelProperty` gained cases for
  `modelStartChannel`, `hasIndividualStartChannels`,
  `individualStartChannel<N>` (parses the trailing index off the
  key — keeps the per-string fields keyed independently so
  partial edits route correctly), and `modelChain` (translates
  the `"Beginning"` sentinel back to the empty string).
- UI: when the model has multiple strings, an "Indiv Start
  Chans" toggle shows. OFF — one Start Channel field; ON — N
  String-K fields, one per string. Single-string models always
  show one Start Channel. Model Chain picker appears only when
  controller + protocol + port are all set.

**Read-only popup summaries (PopupDialog parity, view-only):**

- `extrasFor:` helper assembles `submodelNames`, `faceNames`,
  `stateNames`, `aliasNames`, `strandNames`, `nodeNames`,
  `inModelGroups`, `hasDimmingCurve`. The Models tab gains a
  "Model Data" section listing each category with its count
  and (when non-empty) a list-bullet icon that opens
  `ModelDataViewerSheet` — generic read-only `NavigationStack`
  with a `List` of the entries. Dimming Curve is just Set / —.
- Editing these is intentionally out of scope for J-18; the
  desktop popups (Face editor, State editor, Aliases dialog,
  …) each warrant their own slice. The viewer is the bridge so
  users can at least *see* what's defined.

**SwiftUI:**

- `LayoutEditorPropertiesView` Name row gained a pencil icon
  →`RenameGroupSheet(kindLabel: "Model")`.
- New collapsible "Model Data" section ordered between String
  Properties and Appearance.
- `RenameGroupSheet.footerText` now switches by kindLabel for
  Group / Model / Object.

**Deferred (tracked for Models-tab pass 2+):**

- DMX models — different property model entirely, defer the
  whole family.
- Popup editing (Faces, States, SubModels, Aliases, Dimming
  curve, Strand/Node-name editor) — each is its own sub-sheet;
  view-only for now.
- Custom-model node grid + per-channel layout editing.
- Multi-model rename / bulk Start-Channel re-assignment.

### J-17 — Objects tab finish ✓ 2026-05-14

Closes the remaining Objects-tab deferred items in one pass.

**View-object rename:**

- `renameViewObject:to:` — sanitizes via `Model::SafeModelName`,
  refuses collisions / 2D Background. Calls
  `ViewObjectManager::Rename` (returns false on the desktop
  because its cross-reference iteration is commented out — we
  verify by lookup instead).
- `iPadRenderContext::_renamedViewObjects` map + extended
  HasDirty / Clear / SaveLayoutChanges entry guard.
- `SaveLayoutChanges` view-object patch branch: when a dirty
  VO has a pending rename, locates the on-disk element by the
  OLD name, updates the `name` attribute, then patches the
  rest.

**View-object duplicate (shallow):**

- `duplicateViewObject:` — round-trips through
  `XmlSerializingVisitor` → `ViewObjectManager::CreateObject`
  so per-type attrs (Mesh ObjFile, Image bitmap, Terrain
  heightmap PointData) come along automatically. Unique name
  via `GenerateObjectName`, position offset (+50, +50, 0),
  lock cleared. 2D Background refused.

**Unified undo for VOs + heightmap:**

- `LayoutUndoEntry` now discriminated by `UndoTarget::
  {Model, ViewObject, ViewObjectHeightmap}`. VO entries
  snapshot world position + scale (via
  `BoxedScreenLocation::GetScaleX/Y/Z` for boxed VOs;
  `GetScaleMatrix()` fallback for others) + rotation + locked
  + layoutGroup. Heightmap entries snapshot just the
  PointData string.
- New bridge methods:
  `pushLayoutUndoSnapshotForViewObject:` and
  `pushTerrainHeightmapUndoSnapshot:`.
- `UndoLastLayoutChange` dispatches by entry kind. One Undo
  button reverts whatever the user did last regardless of
  kind.
- Push sites added: `commitObjectProperty`, every VO drag /
  pinch / twist gesture begin (handle drag, body drag in
  2D + 3D, pinch, twist), and the terrain heightmap tap
  before each brush.

**SwiftUI:**

- VO property pane "Name" row gets pencil + duplicate icons
  alongside the name. Pencil → opens
  `RenameGroupSheet` (now generic — accepts `kindLabel:
  "Object"` to title the sheet correctly). Duplicate → fires
  the bridge, auto-selects the new copy so the user can drag
  it into place.
- `RenameGroupSheet` generalized: new `kindLabel` parameter
  (defaults to "Group") swaps title + footer text. Same
  sanitization preview + collision check.
- `ViewObjectCrudModifiers` gained the rename sheet sheet
  hosting + sanitize callback + existing-names lookup
  (model + group + object names combined for collision).

### J-16 — Group name sanitization ✓ 2026-05-14

Follow-up to the rename work: neither the create nor the rename
sheet validated against the desktop's character restrictions. The
canonical sanitizer is `Model::SafeModelName` which strips
`, ~ ! ; < > " ' & : | @ / \ \t \r \n` plus surrounding
whitespace. (`,` and `/` are wire-format delimiters — `,`
separates members in a group's `models` attribute, `/` separates
parent from submodel in fully-qualified names.)

**Bridge** (`XLSequenceDocument`):

- New `sanitizedModelName:` wraps `Model::SafeModelName` so the
  SwiftUI sheets can preview the sanitized form before submit.
- `createModelGroup:` and `renameModelGroup:to:` now run the
  same sanitizer internally as defence-in-depth — passing a
  name with illegal characters doesn't fail, it just takes
  effect with the bad characters silently stripped (matches
  desktop convention).

**SwiftUI:**

- `NewGroupSheet` and `RenameGroupSheet` both:
  - Live-preview the sanitized name with an info banner
    ("Will save as 'X'") when input differs from sanitized.
  - Caption listing the disallowed characters so the user knows
    what got stripped without trial-and-error.
  - Disable Create/Rename when sanitized result is empty or
    collides with another model/group.
  - Submit the sanitized name (not the raw text).

### J-16 — Group rename ✓ 2026-05-14

Closes the last J-7 / J-9 group-CRUD deferral.

**Bridge** (`XLSequenceDocument`):

- `renameModelGroup:to:` — refuses collisions with existing
  model/group names, empty/same-as-old. Calls
  `ModelManager::Rename` which updates the in-memory
  references in every group containing this one (member list
  vectors get the name fix). After rename, walks all groups
  and marks any that now directly reference the new name as
  dirty so the save patcher rewrites their `models` attribute.

**Render context** (`iPadRenderContext`):

- New `_renamedGroups: map<newName → oldOnDiskName>` slot. The
  `MarkGroupRenamed` helper handles the edge cases:
  rename-after-rename collapses to the original on-disk name;
  renaming back to the original drops the pending rename;
  rename of an in-memory-only created group just retitles the
  pending creation.
- `MarkGroupDeleted` cleans up any pending rename for the
  deleted name so the delete pass finds the right element on
  disk.
- `HasDirtyLayoutModels` + `ClearDirtyLayoutModels` /
  `SaveLayoutChanges` entry guard now include
  `_renamedGroups`.
- `SaveLayoutChanges` group-patch branch: when a dirty group
  has a renamed-from entry, finds the element by the OLD name
  and updates its `name` attribute before patching the rest.

**SwiftUI:**

- New `RenameGroupSheet` — text field pre-filled with the
  current name, live collision check against models AND groups
  (excluding the current name so submitting unchanged closes
  cleanly), Cancel / Rename buttons.
- Group property pane's "Name" row gains a small pencil icon.
  Tap → opens the rename sheet.
- `handleRenameGroup` re-points the sidebar selection to the
  new name on success so the property pane reopens on the
  renamed group.

**Dropped from the deferred list:**

- ~~Cross-pane drag from Models list onto Group member list~~
  — out of scope; the AddMember sheet's tree picker covers
  the use case.
- ~~Member-count cap warnings~~ — controllers don't see
  groups; the cap concept only applies to models.

**Still deferred (with re-framed semantics):**

- Group duplicate — when prioritized, will be a **shallow
  copy**: new group named `<name>-1` with the same
  `ModelNames` vector + copy of group settings. Members are
  not cloned. (The earlier "semantics question" framing was
  me overthinking — shallow is what users want.)

### J-15 — VO 3D drag + pinch + twist ✓ 2026-05-14

Closes the J-13 deferred gestures: 3D body-drag, pinch-to-scale,
and two-finger twist-to-rotate all now work on the selected
view object.

**Bridge** (`XLMetalBridge`):

- New begin methods: `beginBodyDrag3DForViewObject:atScreenPoint:viewSize:forDocument:`,
  `beginPinchScaleForViewObject:forDocument:`,
  `beginTwistRotateForViewObject:forDocument:`. Each grabs the
  VO's `ObjectScreenLocation`, latches the same saved-state
  fields the model path uses, and flips a target-is-VO flag.
- The existing `dragBody3DToScreenPoint:`,
  `applyPinchScaleFactor:`, `applyTwistRotationRadians:`,
  `endBodyDrag3D`, `endPinchScale`, `endTwistRotate` now branch
  on the flag: read the screen location from
  `ViewObjectManager` instead of `ModelManager`, and call
  `MarkLayoutViewObjectDirty` at the dirty-mark step.

**SwiftUI** (`PreviewPaneView`):

- `handlePinch` `.began`: if no model is the target and a VO is
  selected, tries `beginPinchScale(forViewObject:…)`. Falls
  through to camera zoom on miss / non-applicable target.
- `handleRotate` `.began`: same pattern for twist-to-rotate
  using `beginTwistRotate(forViewObject:…)`.
- `handleOneFingerPan` `.began`: VO branch upgrades 3D path
  to call `beginBodyDrag3D(forViewObject:…)` instead of
  bailing out (2D path unchanged — uses `moveViewObject:`).
- `.changed` for `draggingLayoutViewObject`: 3D dispatches
  through `dragBody3D(toScreenPoint:)`; 2D continues to use
  `moveViewObject:byDeltaDX:dY:`.
- `.ended`: VO 3D drag calls `endBodyDrag3D()` (state cleanup
  on the bridge).

### J-14 — VO endpoint handle drag ✓ 2026-05-14

Drag-to-move on a view object body works (J-13), but the handles
that draw at the endpoints of a selected Ruler / any boxed VO
corner weren't tappable — clicking them fell through to body
drag.

**Bridge** (`XLMetalBridge`):

- New `pickViewObjectHandleAtScreenPoint:viewSize:forDocument:`.
  Mirrors `pickHandleAtScreenPoint:` for models but routes
  through the SELECTED view object's
  `ObjectScreenLocation`. Calls `loc.GetHandles(...)` →
  `handles::HitTest` → `loc.CreateDragSession(...)`. On a hit,
  stashes the VO name in a new `_dragSessionViewObjectName`
  state slot. Refuses the 2D Background pseudo-object.
- `endHandleDragForDocument:` extended: when
  `_dragSessionViewObjectName` is non-empty at commit time,
  marks the VO dirty + reloads it (so the canvas re-reads the
  new endpoint positions) instead of the model dirty path.
  Cleared on session end.

**SwiftUI** (`PreviewPaneView`):

- Pan `.began` for a selected VO tries
  `pickViewObjectHandle:` first. On a hit, sets
  `draggingLayoutHandle` (reused from the model path) and
  pipes touch updates through the existing
  `bridge.dragHandle:` — which routes via `_dragSession`
  regardless of target type. Falls through to body drag
  (`moveViewObject:`) on a miss.

The result: tapping a ruler's start or end endpoint and
dragging now moves just that endpoint, with the other end
staying put — matching desktop behaviour.

**Deferred:** 3D body drag for VOs (still no plane-anchor
session); pinch-to-scale / twist-to-rotate on view objects.

### J-14 — VO field gaps from J-12 + J-13 testing ✓ 2026-05-14

User report: missing Mesh-only toggle, missing Scale X/Y/Z on
Mesh+Image, Ruler showing center+rotate instead of two-point
endpoints.

**Bridge** (`XLSequenceDocument.viewObjectLayoutSummary:` +
`setLayoutViewObjectProperty:`):

- Mesh: `meshOnly` (BOOL), `scaleX/Y/Z` (read via
  `BoxedScreenLocation::GetScaleX/Y/Z`, written via the
  matching setters).
- Image: `scaleX/Y/Z` (same `BoxedScreenLocation` plumbing).
- Ruler (and any future two-point view object): `twoPoint = YES`
  discriminator plus `p1X/Y/Z` (absolute world coords of point 1
  = `WorldPos`) and `p2X/Y/Z` (absolute world coords of point 2 =
  `WorldPos + (X2,Y2,Z2)`). Setting `p1*` keeps the absolute
  point 2 unchanged by re-basing `X2/Y2/Z2`; setting `p2*` just
  updates the offset.

**Save patcher** (`iPadRenderContext::SaveLayoutChanges`):

- Mesh: writes `MeshOnly`.
- Ruler / any two-point VO: writes `X2/Y2/Z2`. WorldPos (X/Y/Z =
  point 1) is already written by the common path.

**SwiftUI** (`LayoutEditorObjectPropertiesView`):

- Mesh per-type rows: added Mesh-only toggle + Scale X/Y/Z
  (3-decimal precision via new `scaleField` helper).
- Image per-type rows: added Scale X/Y/Z.
- Size/Location section reorganizes for two-point VOs: shows
  Point 1 X/Y/Z and Point 2 X/Y/Z instead of Centre + Rotate.
  Locked stays at the bottom.
- Dimensions section hidden for two-point VOs (the line's extent
  comes from its endpoints, not from bounding-box dimensions).

### J-13 — VO drag-to-move + basic terrain heightmap editor ✓ 2026-05-14

**Drag-to-move for view objects (2D)**

Closes the J-6 deferred "drag-to-move on canvas" gap for Mesh /
Image / Gridlines / Terrain. 2D only for now — 3D body-drag
re-uses the existing plane-anchor math, but that work is deferred.

- New `pickViewObjectAtScreenPoint:viewSize:forDocument:` —
  mirrors `pickModelAtScreenPoint`, searches
  `ViewObjectManager`. 2D box-test + 3D `HitTest3D` ray cast.
  Returns the topmost (last-drawn) hit.
- New `moveViewObject:byDeltaDX:dY:viewSize:forDocument:` —
  delta-based 2D move with identical math to `moveModel:` but
  writes through `vo->SetHcenterPos / SetVcenterPos` and marks
  the VO dirty. Honours locked, isFromBase, snap-to-grid.
- `PreviewPaneView.handleSingleTap`: when no model is under the
  touch, try the VO hit-test next. Matching VO selection flips
  to the Objects tab via the J-11 mutex.
- `PreviewPaneView` one-finger pan: state vars
  `draggingLayoutViewObject` + `layoutDragViewObjectName`
  parallel the model drag slots. `.began` checks for a selected
  VO under the touch and starts a VO drag rather than falling
  through to camera-pan; `.changed` dispatches to
  `moveViewObject:`; `.ended` posts the standard
  `layoutEditorModelMoved` notification so the property pane
  re-reads the new transform.

**Basic terrain heightmap editor**

- New `editTerrainHeight:atScreenPoint:viewSize:delta:brushRadiusPoints:forDocument:`
  on `XLMetalBridge`. Unprojects the touch (2D or 3D ray-onto-
  plane), maps to the terrain's `(u, v)` grid using its
  spacing/width/depth, reads the current `PointData` heights,
  applies `delta` at the nearest grid point. When
  `brushRadiusPoints > 0`, applies a cosine falloff to
  neighbouring points within the radius for a smoother
  deformation. Writes the new heights back to
  `TerrainScreenLocation::SetDataFromString`, increments change
  count, marks the VO dirty.
- View-model state on `SequencerViewModel`: `terrainEditTarget`
  (terrain name when edit mode is on, nil otherwise),
  `terrainEditDelta` (per-tap magnitude), `terrainEditRaise`
  (Raise / Lower direction toggle), `terrainEditBrushPoints`
  (brush radius in screen points; 0 = single-point edit).
- Property pane: `LayoutEditorObjectPropertiesView.terrainEditControls`
  block renders inside the per-type Terrain section. Edit
  Heightmap toggle, Raise/Lower segmented control, Step
  magnitude slider (0.1–10), Brush radius slider (0–80pt).
- Canvas tap handler short-circuits when
  `terrainEditTarget != nil`: signs the delta from the Raise
  toggle, calls `editTerrainHeight`, repaints. Miss falls
  through to normal pick.
- Edit mode auto-clears when: the selected object changes away
  from the terrain, the user clears selection, or the editor
  window closes.

**Deferred:**

- 3D body-drag for view objects (existing plane-anchor pipeline
  is model-specific; same lift as a parallel
  `beginViewObjectBodyDrag3D` set of methods).
- Heightmap undo (each tap is a separate dirty event; would need
  a snapshot stack analogous to `_layoutUndoStack` for VOs).
- Smoothing / level-out / flatten tools (current editor only
  raises or lowers; no brush profiles).
- Mesh-only mode toggle for Mesh objects.

### J-12 — Objects tab: per-type editing, file pickers, create/delete ✓ 2026-05-14

Brought the Objects tab up to feature parity with the Models +
Groups tabs.

**Bridge** (`XLSequenceDocument`):

- `viewObjectLayoutSummary:` now returns `typeKind` discriminator
  + per-type fields:
  - Mesh: `objFile`, `brightness`
  - Image: `imageFile`, `brightness`, `transparency`
  - Gridlines: `gridSpacing`, `gridWidth`, `gridHeight`,
    `gridColor`, `hasAxis`, `pointToFront`
  - Terrain: `imageFile`, `brightness`, `transparency`,
    `gridSpacing`, `gridWidth`, `gridDepth`, `hideGrid`,
    `hideImage`, `gridColor`
  - Ruler: `units` (0..5 enum index), `length`, `unitOptions`
- `setLayoutViewObjectProperty:` accepts every key above plus
  `active`. Dynamic-cast guards mean a key sent to the wrong
  type silently no-ops.
- `availableViewObjectTypes` — types the iPad accepts for
  creation. Ruler is filtered out when one already exists
  (it's a show-singleton).
- `createViewObjectWithType:` — calls
  `ViewObjectManager::CreateAndAddObject`, defaults layout
  group to active, records the auto-generated name in
  `_createdViewObjects` for the save patcher.
- `deleteViewObject:` — `ViewObjectManager::Delete` + record
  in `_deletedViewObjects`. Refuses `2D Background` since it's
  a pseudo-object.

**Render context** (`iPadRenderContext`):

- New `_createdViewObjects` / `_deletedViewObjects` sets +
  `MarkViewObjectCreated` / `MarkViewObjectDeleted` helpers
  (parallel to the J-7 group plumbing).
- `SaveLayoutChanges`:
  - Pass: drop deleted `<view_object>` elements.
  - Pass: append fresh elements for created objects (just the
    `name` + `DisplayAs` attrs; the existing dirty-patcher
    fills in the rest).
  - Existing dirty-patcher: now also writes per-type attrs
    (`ObjFile`, `Brightness`, `Image`, `Transparency`,
    `GridLineSpacing`/`GridWidth`/`GridHeight`/`GridColor`/
    `GridAxis`/`PointToFront`, terrain's `Terrian*` legacy
    spellings, ruler `Units`/`Length`).
- `HasDirtyLayoutModels` / `ClearDirtyLayoutModels` cover the
  new sets.

**SwiftUI:**

- `LayoutEditorObjectPropertiesView` rewritten with
  `DisclosureGroup`s in the same order as the Models tab:
  Header → per-type (expanded) → Appearance → Dimensions →
  Size/Location. Per-type body switches on the `typeKind`
  discriminator so each VO type shows the right control set.
- Tag / Grid color via the same `ColorPicker` + hex-bridge
  helpers used by the Models tab.
- File pickers for Mesh `.obj`, Image bitmap, and Terrain
  image. Generic `onPickFile(key, types)` callback in the
  property view → `handleObjectFilePick` in
  LayoutEditorView writes through `commitObjectProperty`.
- Objects roster: `+` button in the section header opens an
  `AddViewObjectSheet` with the available types; swipe-left
  on a row → delete confirmation. "2D Background" is filtered
  out of both affordances (pseudo-object).
- `ViewObjectCrudModifiers` factored out as a `ViewModifier`
  (sheet + delete alert + file picker) to keep
  `LayoutEditorView.body` under the type-checker's complexity
  budget.

**Deferred:** view-object undo (parallel snapshot stack for
ScreenLocation + per-type fields), drag-to-move on the canvas
(existing drag pipeline assumes ModelManager), terrain
heightmap editing (deep canvas tool).

### J-11 — Sidebar selection mutual exclusion ✓ 2026-05-14

User report: with a group selected on the Groups tab, tapping a
model on the canvas left BOTH selections active (group's cyan
member tint + model's yellow primary highlight). Confusing.

New rule: **at most one of {model, group, object} is selected at
any time**, and the sidebar tab follows the active selection.

**Behaviour:**

- Pick a model (sidebar Models list OR canvas tap) → clears any
  group/object selection AND flips the sidebar to the Models tab
  so the property pane matches what's selected on the canvas.
- Pick a group → clears any model / object selection. Tab switch
  is implicit (sidebar tap was already on Groups).
- Pick an object (or "2D Background" pseudo-object) → clears any
  model / group selection.
- Switch tabs via the segmented picker → clears whatever
  selections were live in the tabs you're leaving. Eliminates
  "I'm on the Groups tab but a model is still tinted on the
  canvas from earlier".

**Implementation:** new `SidebarSelectionMutex` `ViewModifier`
hosts the four `.onChange` handlers (model / group / object /
sidebarTab). Factored out because `LayoutEditorView.body` was
already at the type-checker's complexity ceiling — adding the
four chained `.onChange` modifiers inline pushed it over.

### J-10 — Group polish round 2 ✓ 2026-05-14

Three follow-ups from J-9 testing:

**1. Members list height.** The J-9 SwiftUI `List` inside the
property pane's outer `ScrollView` collapsed to a ~2-row internal
scroll area, making drag-to-reorder nearly impossible. Replaced
with a manual `VStack` of rows + per-row `.draggable` /
`.dropDestination` so the outer pane scrolls the whole content
and every member is visible. Drop-target highlight shows where
the drag will land. A trailing drop on the container (outside any
row) appends to the end.

**2. Layout Style options.** Bumped from the J-9 4-entry set to
all 15 desktop options (`ModelGroupPanel.wxs`). The 4 special
entries keep their compact lowercase wire form
(`grid`/`minimalGrid`/`horizontal`/`vertical`); the rest
round-trip their display label verbatim, matching desktop's
`OnChoiceModelLayoutTypeSelect` fall-through.

**3. Group canvas tint.** Cyan didn't differentiate against the
default grey at pixel sizes 1–2 (the common case). Switched to
yellow for both primary selection and group members — the active
gizmo / handles still distinguish "what's actively editable" from
"what's a member of the selected group".

Submodel-only groups: when a member is `Parent/Sub` (resolved to
a `SubModel*` via `ModelManager`), the new render path collects
those into `selectedGroupSubmodelsByParent`. After the main model
render loop, each such submodel is drawn as a yellow overlay on
top of its parent — `DisplayModelOnWindow` on a `SubModel`
iterates only that submodel's `Nodes`, so only those node ranges
light up. Mirrors desktop's "group of submodels → highlight just
the submodel regions" behaviour.

**Deferred:** drag from outside the property pane (e.g. drop a
model from the Models tab list onto the member list). Touch-only
gesture work; the in-list reorder is sufficient for the common
authoring flow.

### J-9 — Group property polish ✓ 2026-05-14

Closed every J-7 group-CRUD deferral that affects daily authoring.
Settings get proper pickers, members get a tree-picker for
submodels and drag-to-reorder, plus a tag color.

**Bridge** (`XLSequenceDocument`):

- `modelGroupLayoutSummary:` now also returns:
  - `layoutStyleOptions`: NSArray<NSDictionary{value,label}>.
    Four hard-coded styles matching desktop's
    ModelGroupPanel.wxs (`grid`/`minimalGrid`/`horizontal`/
    `vertical`, with their friendly labels).
  - `defaultCameraOptions`: NSArray<NSString>. Always starts
    with `"2D"`, then every 3D camera the show has saved via
    `ViewpointMgr`.
  - `tagColor`: `#RRGGBB` (round-trips via `Model::SetTagColourAsString`
    which ModelGroup inherits).
- `setLayoutModelGroupProperty:`:
  - `tagColor` (NSString) — round-trips through
    `SetTagColourAsString`.
  - `members` (NSArray<NSString>) — replaces the entire member
    list. Used by drag-to-reorder; future bulk-move can reuse
    the same path.
- New `submodelsForModel:` returns the full-name list
  (`Parent/Sub`) for one parent — lazy lookup so the
  AddMember tree only loads what the user expands.

**SwiftUI:**

- `LayoutEditorGroupPropertiesView`:
  - Default Camera → Menu populated from
    `defaultCameraOptions`. Tap to select; check mark on the
    current value.
  - Layout Style → Menu with the 4 friendly labels. If the on-
    disk value isn't in the list (legacy XML), shows "Custom: …"
    label + "Currently: X" footer entry so the user can see what's
    there without losing it.
  - Grid Size → int field (precision 0), narrower frame.
  - Tag Color → `ColorPicker` row with hex preview (same bridge
    helper used by the Models tab).
  - Members → wrapped in a SwiftUI `List` with `.onMove` for
    drag-to-reorder + `.onDelete` for swipe-to-remove. An Edit
    toggle on the Members header switches between view-mode
    (red minus button per row) and edit-mode (drag handles).
- `AddMemberSheet`:
  - Switched from a flat `editMode: .active` selection list to
    a custom tree. Top-level rows for models / groups; chevron
    button toggles a row to surface its submodels (lazy-fetched
    via the new `submodelsForModel:` bridge).
  - Tap-to-pick at either level — user can add `Arch1`,
    `Arch1/Inner`, or both. Picked rows show a filled checkmark;
    rows already in the group dim out with an "(already a
    member)" hint so the tree stays informative.
  - Search filter matches against parent name OR any submodel
    name, so typing `"Inner"` surfaces the parent row whose
    submodel contains it (parallels the vendor search filter).

**Deferred:**

- Group rename. Same plan as J-7 — needs an old-name → new-name
  map threaded into the save patcher.
- Drag from Models tab into a Group's member list (would skip
  the Add Member sheet entirely for casual edits). Touch-only
  gesture work; not blocking.

### J-8 — 2D Background as a synthetic Objects-tab entry ✓ 2026-05-14

Moved the per-layout-group background settings (image / brightness
/ alpha / scale) out of the read-only "Display" roll-up and into
the Objects tab as a synthetic "2D Background" pseudo-object. It's
not in `ViewObjectManager` — it's an attribute of the active
layout group — so the bridge special-cases it.

**Bridge** (`iPadRenderContext`):

- `SetActiveBackgroundImage:` / `Brightness:` / `Alpha:` /
  `ScaleBackgroundImage:` write through to the active group's
  storage (top-level `_backgroundImage…` for Default, or the
  matching `_namedLayoutGroups[i]` for a named group). Each
  records the group name in `_dirtyBackgroundGroups`.
- `HasDirtyLayoutModels` / `ClearDirtyLayoutModels` now also
  include the background dirty set.
- `SaveLayoutChanges`: new pass writes `backgroundImage`,
  `backgroundBrightness`, `backgroundAlpha`, `scaleImage`
  attributes onto the matching XML target (`<settings>` for
  Default, `<layoutGroups><layoutGroup name="...">` for named).

**Bridge** (`XLSequenceDocument`):

- `viewObjectsInActiveLayoutGroup` always returns
  `"2D Background"` at index 0 (even before a show is loaded).
- `viewObjectLayoutSummary:@"2D Background"` returns a
  schema-flagged dictionary (`isBackground: YES`,
  `backgroundImage`, `backgroundBrightness`, `backgroundAlpha`,
  `scaleBackgroundImage`).
- `setLayoutViewObjectProperty:` routes 2D Background keys to
  the new `iPadRenderContext` setters instead of
  `ViewObjectManager`.

**SwiftUI:**

- New `LayoutEditorBackgroundPropertiesView` — image path with
  folder-picker + clear button, Brightness + Alpha int fields,
  Scale-to-fit toggle, plus a one-liner explaining that this
  is a per-group attribute rather than a moveable object.
- `propertyPaneBody` dispatches to it when the selection's
  `isBackground` is true.
- Background image uses `.fileImporter` with image UTTypes
  (png/jpeg/tiff/bmp/gif). Same security-scoped-bookmark dance
  as the existing model importer.
- "Background" row removed from the Models tab's `displaySection`
  — it's now editable on the Objects tab instead of read-only on
  every tab.

**Deferred:**

- "Show 2D Background only when relevant" — currently visible
  even in 3D-default shows. Desktop hides backgrounds when the
  layout is 3D-only.

### J-8 — Desktop-order collapsible property sections ✓ 2026-05-14

Reorganized the Models tab property pane to match desktop's
wxPropertyGrid ordering, and made the sections collapsible
(SwiftUI `DisclosureGroup`).

**New section order:**

1. Header (always visible): Name, Type
2. **Model Properties** — per-type descriptors. Expanded by
   default. Section title = the model's `displayAs` so each
   model type shows its own category name (matches desktop).
3. **Controller Connection** — Preview (layoutGroup), Controller,
   Channel range (read-only).
4. **String Properties** — String Type (NODE_TYPE picker),
   Strings, Nodes.
5. **Appearance** — Active, Pixel Size, Pixel Style, Transparency,
   Black Transparency, Tag Color, Description.
6. **Dimensions** — Width, Height, Depth.
7. **Size/Location** — Centre X/Y/Z, Rotate X/Y/Z, Locked.

**Bridge additions** to `modelLayoutSummary:`: `active`,
`pixelSize`, `pixelStyle` (+`pixelStyleOptions`), `transparency`,
`blackTransparency`, `tagColor`, `stringType`
(+`stringTypeOptions` — desktop's NODE_TYPE_VALUES verbatim),
`description`. Corresponding cases added to
`setLayoutModelProperty:key:value:`.

**SwiftUI changes:**

- `LayoutEditorPropertiesView` rewritten as a stack of
  `DisclosureGroup`s. Per-type descriptor rendering moved
  inline (the standalone `LayoutEditorTypePropertiesView` is
  retired for the Models tab — kept around for future reuse).
- `section(_:title:)` generic helper materializes the content
  before handing it to `DisclosureGroup`, sidestepping
  "non-escaping captured by escaping closure" on the content
  builder.
- Tag Color uses SwiftUI's `ColorPicker` with a `#RRGGBB`
  bridge via new `Color.toHexString()` / `Color(hexString:)`
  extension helpers.
- Per-section expanded state is `@State Bool`. Default:
  Model Properties expanded, everything else collapsed.

### J-7 — Bridge null-guard fix ✓ 2026-05-14

User reported `EXC_BAD_ACCESS` at address 0x28 in
`modelGroupsInActiveLayoutGroup`. Stack: libc++ `__tree::begin()`
→ `map::map(const map&)` (copy ctor) → `ModelManager::GetModels()`
→ the new bridge method.

**Root cause:** offset 0x28 (= 40) is exactly the offset of
`std::map<std::string, Model*> models;` inside `ModelManager`
(after vtable + 2 pointers + bool/padding + 2 ints). So the copy
ctor's source map address is `(ModelManager*)0 + 40` →
`_modelManager` is `nullptr` and `*_modelManager` was UB.
`iPadRenderContext::GetModelManager()` returns `*_modelManager`
without guarding — `_modelManager` is only created inside
`LoadShowFolder`. The Layout Editor can be reached before a
show is loaded.

**Fix:**

- New `iPadRenderContext::HasModelManager()` /
  `HasViewObjectManager()` accessors return true iff the
  unique_ptr is set.
- Every J-5/J-6/J-7 bridge method that calls
  `GetModelManager()` / `GetAllObjects()` direct (not via
  `GetModelsForActivePreview()` which guards internally) now
  early-returns when these are nil. Affected methods:
  - `modelGroupsInActiveLayoutGroup`,
    `modelGroupLayoutSummary:`,
    `setLayoutModelGroupProperty:key:value:`
  - `viewObjectsInActiveLayoutGroup`,
    `viewObjectLayoutSummary:`,
    `setLayoutViewObjectProperty:key:value:`
  - `perTypePropertiesForModel:`,
    `setPerTypeProperty:onModel:value:`
  - `addModel:toGroup:`, `removeModel:fromGroup:`,
    `createModelGroup:members:`, `deleteModelGroup:`
  - `flipModels:axis:forDocument:`,
    `duplicateModels:forDocument:` (on XLMetalBridge).

### J-7 — Multi-select: Flip / Duplicate / Group-from-selection ✓ 2026-05-14

Three high-frequency operations that closed remaining J-4 (multi-
select) gaps and made the single-model + multi-select action bars
feature-complete for the common authoring workflows.

**Bridge surface** (`XLMetalBridge`):

- `flipModels:axis:forDocument:` — `horizontal` rotates 180°
  about Y_AXIS, `vertical` about X_AXIS. Mirrors desktop's
  `BaseObject::FlipHorizontal` / `FlipVertical`. Each model
  flips about its own origin (not the selection's centroid);
  this matches desktop and avoids a separate "selection
  bounding box" computation.
- `duplicateModels:forDocument:` — round-trips each source
  through `XmlSerializer::SerializeModel`, gets a unique name
  via `ModelManager::GenerateModelName`, clears controller
  mapping, unlocks, offsets by (+50, +50, 0), and re-deserializes
  via `ModelManager::CreateModel`. Returns the new names in
  source order. ModelGroup duplication is intentionally skipped
  (member-reference semantics are ambiguous — share with the
  source, or deep-copy?).

**SwiftUI:**

- `MultiSelectActionBar`: gained a Flip ▾ menu (H / V) before
  Match Size; Duplicate + Group buttons after, separated from
  Clear by a divider.
- `InlineModelActionBar` (single-model): gained a Duplicate
  button next to Delete. Same `performDuplicate(of: [selected])`
  helper so the single + multi paths share one code path.
- `NewGroupSheet` reused for Group-from-selection: a new
  `pendingGroupFromSelection: [String]?` state slot tells
  `handleCreateGroup` whether to pass `nil` (empty group) or
  the captured selection (group-from-selection). On success
  the sidebar flips to the Groups tab so the user immediately
  sees the new group + its members.
- `performDuplicate` shifts the multi-selection to the new
  duplicates so the user can drag / nudge them right away.

**Deferred:**

- ModelGroup duplicate (semantics question).
- View-object duplicate (would need a parallel
  `duplicateViewObjects:` since they live in `ViewObjectManager`).
- "Flip around selection centroid" — common desktop affordance
  but needs centroid math + a UI toggle distinguishing per-
  model vs. group flip.

### J-7 — Model group CRUD ✓ 2026-05-14

The Groups tab is no longer settings-only. Users can create new
groups, delete groups, and edit membership (add / remove) from
the sidebar without bouncing back to desktop.

**Bridge surface** (`XLSequenceDocument.h`):

- `addModel:toGroup:` / `removeModel:fromGroup:` — member ops.
  `add` is a no-op for already-direct members; both fire
  `MarkLayoutModelDirty(group)` so the patcher rewrites the
  `<modelGroup>` element on save.
- `createModelGroup:members:` — creates the group with curated
  defaults (layout="minimalGrid", gridSize=400,
  DefaultCamera="2D", LayoutGroup=active). Refuses to collide
  with an existing model/group name.
- `deleteModelGroup:` — removes from `ModelManager` AND records
  the name in `_deletedGroups` so the save patcher drops the
  `<modelGroup>` element.

**Render context** (`iPadRenderContext`):

- New `_createdGroups` + `_deletedGroups` sets. `MarkGroupCreated`
  / `MarkGroupDeleted` helpers handle the create-then-delete-
  then-create-with-same-name edge cases.
- `HasDirtyLayoutModels` now also reports created/deleted state
  (the "unsaved changes" indicator stays accurate).
- `SaveLayoutChanges` extended:
  - Pass 0a: remove `<modelGroup>` elements for every deleted
    name.
  - Pass 0b: append fresh `<modelGroup>` elements for created
    names (full attribute set including `models`,
    `centreDefined`, `selected`).
  - Existing group-patch path: now also writes the `models`
    comma-delimited member list so add/remove edits persist.

**SwiftUI:**

- `NewGroupSheet` — name field + collision check (existing
  models or groups in `ModelManager`). Defaults focus to the
  text field. Created group is auto-selected so the user can
  start adding members.
- `AddMemberSheet` — multi-select picker, lists every model in
  the active layout group that isn't yet a direct member.
  Filterable. `editMode = .active` so SwiftUI's
  selection-with-checkmarks UI lights up.
- Groups list: swipe-left on a row → confirm-then-delete. Plus
  button in the section header opens NewGroupSheet. Empty-state
  surface includes a "Create a Group" CTA.
- Group property pane: each member row gets a red minus button
  (one-tap remove); "Members" row gets a plus button that opens
  AddMemberSheet. Read-only roll-up replaced.
- `GroupCrudModifiers` factored out as a `ViewModifier` so the
  three new presentations (two sheets + alert) don't blow
  LayoutEditorView's body past the type-checker's complexity
  budget.

**Deferred:**

- Group rename. Would need a per-name "renamed from → to" map
  threaded through save so the patcher can find the old
  `<modelGroup>` element. Manageable but not blocking.
- Drag-to-reorder members in the property pane (member order
  affects the buffer style — but the bridge doesn't currently
  expose a "move member to position N" op).
- Member-count cap warnings (some controllers have soft limits).

### J-6 — Sidebar → canvas selection sync ✓ 2026-05-14

Closes the deferred-from-J-5 promise that "tapping a group selects
it on the canvas". Picking in the Groups tab now tints every
member of the selected group cyan in the preview, so the user
sees at a glance what the group contains. Picking in the Objects
tab puts handles on the view object so it can be inspected /
edited directly via the canvas in addition to the property pane.

**View model** (`SequencerViewModel`):

- New `layoutEditorSelectedGroup: String?` and
  `layoutEditorSelectedObject: String?`. Independent of
  `layoutEditorSelectedModel`, so flipping tabs preserves every
  tab's selection.
- `LayoutEditorView` switched from local `@State` to view-model
  bindings (`groupListBinding` / `objectListBinding`).

**Canvas bridge** (`XLMetalBridge`):

- `setSelectedGroup:` / `setSelectedViewObject:` setters.
- Model render loop resolves the selected group's flat members
  once per frame via `ModelGroup::GetFlatModels(true, false)` and
  tints any member that isn't already the primary / extras
  selection with `xlCYAN`. Primary stays yellow; extras stay
  yellow.
- View-object render loop calls `Selected(true)` and
  `Draw(..., allowSelected=true)` on the matching object so its
  ScreenLocation handles appear.

**PreviewPaneView** diffs the new selections against
`Coordinator.lastPushedGroup` / `lastPushedObject` and only
repaints on Δ.

**Deferred:**

- Drag-to-move on a selected group (would move every member —
  needs a separate gesture handler).
- Drag-to-move / resize on a selected view object via canvas
  handles (the handles draw, but the existing model-drag
  pipeline assumes the target is in `ModelManager`).
- "Group bounding box" overlay — groups don't have a natural
  geometric extent so cyan member tint is the chosen affordance
  rather than a bounding box.

### J-6 — View object editing + save ✓ 2026-05-14

Removes the "use desktop to edit view objects" footnote from the
J-5 Objects tab. Mesh / Image / Gridlines / Terrain / Ruler now
support the same common-property surface as models (centre X/Y/Z,
width/height/depth, rotate X/Y/Z, locked, layoutGroup).

**Bridge surface** (`XLSequenceDocument.h`):

- `setLayoutViewObjectProperty:key:value:` — single setter
  covering the eleven keys above. Mirrors
  `setLayoutModelProperty:` for symmetry.

**Save infrastructure** (`iPadRenderContext`):

- New `_dirtyLayoutViewObjects` set parallels
  `_dirtyLayoutModels`.
- `MarkLayoutViewObjectDirty(name)` helper added.
- `SaveLayoutChanges` extended: after patching `<model>` /
  `<modelGroup>` elements, walks `<view_objects>` for each dirty
  view object and patches `WorldPosX/Y/Z`, `ScaleX/Y/Z`,
  `RotateX/Y/Z`, `LayoutGroup`, `Locked` attributes in place.
  Per-type attributes (Mesh `ObjFile`, Image bitmap path,
  Terrain heightmap path) round-trip untouched.
- `ClearDirtyLayoutChanges` drains both sets so the Discard
  Changes path leaves no stale dirty marks.

**SwiftUI:** `LayoutEditorObjectPropertiesView` upgraded from
read-only labels to the same editable rows the Models tab uses
(`LayoutEditorDoubleField` + Toggle + layoutGroup Menu). The
"View object editing on iPad is read-only" footer removed.
`commitObjectProperty(...)` mirrors `commitProperty` minus undo
bookkeeping — view objects don't ride the model undo stack yet.

**Deferred:**

- View-object undo (parallel `LayoutUndoEntry` variant for
  ScreenLocation snapshots).
- Per-type editing (Mesh `.obj` path picker, Image bitmap
  picker, Terrain heightmap, Ruler unit). These need their own
  file pickers + dependency-resolution paths.

### J-6 — Per-type model properties ✓ 2026-05-14

Closes the biggest user-visible gap right after the sidebar
refactor: picking a Tree without seeing "# Branches" / "Bottom-
Top Ratio" or a Matrix without seeing "# Strings" / "Nodes per
String" was jarring. The new Models tab property pane shows a
"Type Properties" section below the common transform fields.

**Bridge surface** (`XLSequenceDocument.h`):

- `perTypePropertiesForModel:` — returns
  `NSArray<NSDictionary>` of descriptors. Each entry has `key`,
  `label`, `kind` (`int`/`double`/`bool`/`enum`/`string`),
  `value`, plus optional `min`/`max`/`step`/`precision`/`options`/
  `enabled`/`help`. Iterating descriptors avoids 18 hand-written
  SwiftUI views.
- `setPerTypeProperty:onModel:value:` — single setter that
  dispatches on the descriptor key. Calls `Reinitialize()` so
  geometry / node count updates land immediately, then marks the
  model dirty.

**Types covered:** Matrix, Tree (extends Matrix), Sphere (extends
Matrix), Star, Arches, Icicles, Circle, Wreath, Single Line,
Candy Canes (partial — Sticks/Reverse/AlternateNodes editable;
height + nodes/cane setter not exposed by header), Spinner,
Window Frame, Cube, Channel Block, Custom (read-only matrix dims).

**SwiftUI renderer:** `LayoutEditorTypePropertiesView` is the
single generic component — it reads the descriptor list and
renders a numeric / toggle / menu / text control per `kind`.
Same `commit(key, value)` callback shape as
`LayoutEditorPropertiesView`. Wired into the Models tab via a
new `commitPerTypeProperty(...)` that pushes undo, sets, bumps
the summary token, repaints the canvas, and refreshes the model
list (string-count edits can shift channel ranges).

**Deferred to a later pass:**

- DMX models (per-fixture channel mapping, moving-head config).
- Image / Label / SubModel.
- PolyLine / MultiPoint vertex editor (the existing tap-and-
  manipulate canvas affordance already covers most of this).
- Custom-model matrix authoring (needs a 2D grid editor).
- CandyCane `nodesPerCane` + `height` setters (header exposes
  getters but not corresponding `Set…` methods).

### J-5 — Sidebar tabs (Models / Groups / Objects) ✓ 2026-05-14

Replaces the single scrolling sidebar where selecting a model
required scrolling past the entire model list to reach its
properties. The sidebar is now a vertical split with a draggable
divider:

- **Top half:** segmented Picker → Models / Groups / Objects.
  Each tab has its own search field and its own selection
  (switching tabs preserves the others'). Controllers tab
  intentionally skipped — iPad has no controller editor yet.
- **Bottom half:** scrollable property pane bound to whichever
  tab is active. Models reuse the existing
  `LayoutEditorPropertiesView`; Groups get
  `LayoutEditorGroupPropertiesView` (layout group, default
  camera, layout style, grid size, 2D centre, locked + read-only
  members roll-up); Objects get `LayoutEditorObjectPropertiesView`
  (transform/locked, all read-only in J-5).
- Canvas selection still follows the Models tab only — picking a
  group/object shows its properties but does not select it on the
  canvas (no canvas-side group/object selection model exists yet
  on iPad).

**Bridge additions** (`XLSequenceDocument.h`):

- `modelGroupsInActiveLayoutGroup` — names of ModelGroups whose
  layout_group matches the active group (or "All Previews").
- `modelGroupLayoutSummary:` — dictionary mirroring
  `modelLayoutSummary` plus group-specific fields (members,
  defaultCamera, layout, gridSize, centerX/Y, centerDefined).
- `setLayoutModelGroupProperty:key:value:` — supports
  layoutGroup, locked, defaultCamera, layout, gridSize,
  centerX/Y. Uses the same dirty/undo plumbing as model edits.
- `viewObjectsInActiveLayoutGroup` / `viewObjectLayoutSummary:`
  — read-only view-object surface (no setter in J-5).

`iPadRenderContext::SaveLayoutChanges` now patches the
`<modelGroup>` element's attributes in place when a dirty name
resolves to a ModelGroup (groups don't round-trip cleanly through
`XmlSerializer::SerializeModel` because their on-disk form is a
flat attribute list under `<modelGroups>`, not `<models>`).

**Deferred to a later pass:**

- ViewObject editing + save infrastructure (`<view_objects>`
  patcher in `SaveLayoutChanges`).
- Group/object canvas selection (highlight the group's bounding
  box when picked from the sidebar).
- Default-camera picker (needs a `<perspectives>` enumeration
  bridge); layout-style picker (needs the enum of valid styles
  exposed).
- Controllers tab (whole controller-setup surface).

### J-4 — Multi-select operations (~1–2 wk)

Direct ports of the desktop math; the work is the contextual
toolbar + selection handling.

- **Align (8 ops):** top, bottom, ground, left, right, h-center,
  v-center, d-center.
- **Distribute (3):** horizontal, vertical, depth.
- **Flip (2):** horizontal, vertical.
- **Resize to match:** width, height, both.
- **Bulk edit:** select N models → set one property → applies to
  all.
- All available from a contextual toolbar that surfaces only when
  ≥2 models are selected. Single undo entry per op.

## Out of scope (S-pro / future)

These ride in [`future-layout-editing.md`](future-layout-editing.md)
or sibling `future-*.md` files; do not creep them into J-0..J-4:

- Custom-model authoring (matrix editor, draw model). Lives in
  [`future-custom-models.md`](future-custom-models.md).
- Face / State definition editors.
- DMX channel / fixture authoring.
- Wiring view / Visualise port-mapping.
- DXF / STL / VRML export.
- ImportPreviewsModelsDialog.
- ViewObject heavy editing — MeshObject, TerrainObject heightmap
  painting, RulerObject. (J-0..J-4 covers ImageObject and
  GridlinesObject only.)
- ControllerModelDialog (port-mapping diagram), PixelTestDialog —
  in [`future-controllers-tab.md`](future-controllers-tab.md).
- The PR #6311 controller-source-tree — revisit once J-4 ships.

## Bridge surface — full list

For J-0..J-4, `XLSequenceDocument.h` gains:

```objc
// Layout state
- (NSArray<NSString*>*)modelsInActiveLayoutGroup;
- (NSDictionary*)modelLayoutSummary:(NSString*)name;
- (NSDictionary*)layoutBackgroundState;
- (BOOL)setLayoutBackgroundProperty:(NSString*)key value:(id)value;

// Per-model property edits (generic; routes to Model::SetProperty)
- (id)getModelProperty:(NSString*)name key:(NSString*)key;
- (BOOL)setModelProperty:(NSString*)name key:(NSString*)key value:(id)value;

// Model lifecycle
- (NSString*)createModelOfType:(NSString*)type
                       atPoint:(CGPoint)worldXY
                          size:(CGSize)worldWH;
- (BOOL)deleteModel:(NSString*)name;
- (BOOL)renameModel:(NSString*)oldName to:(NSString*)newName;
- (BOOL)setModelLocked:(NSString*)name locked:(BOOL)locked;

// Direct manipulation (called from gesture handlers; same shape
// as desktop's CheckIfOverHandles / MoveHandle)
- (NSInteger)hitTestHandleForModel:(NSString*)name
                          atPoint:(CGPoint)worldXY
                            zoom:(CGFloat)zoom;
- (BOOL)moveHandle:(NSInteger)handleIndex
          forModel:(NSString*)name
           toPoint:(CGPoint)worldXY;

// Multi-select ops (J-4)
- (BOOL)alignModels:(NSArray<NSString*>*)names mode:(NSInteger)mode;
- (BOOL)distributeModels:(NSArray<NSString*>*)names axis:(NSInteger)axis;
- (BOOL)flipModels:(NSArray<NSString*>*)names axis:(NSInteger)axis;
- (BOOL)resizeModels:(NSArray<NSString*>*)names
              matchWidth:(BOOL)w
             matchHeight:(BOOL)h;

// Layout undo
- (NSData*)snapshotLayoutState;
- (BOOL)restoreLayoutState:(NSData*)snapshot;

// Model-group editing
- (BOOL)createModelGroup:(NSString*)name;
- (BOOL)deleteModelGroup:(NSString*)name;
- (BOOL)addModel:(NSString*)model toGroup:(NSString*)group;
- (BOOL)removeModel:(NSString*)model fromGroup:(NSString*)group;
```

The exact signatures will firm up during J-0 implementation; the
above is the surface to plan against, not a contract.

## Risks

- **3D gizmo design.** Touch 3D editing is the genuinely novel
  part — the desktop relies on mouse precision the iPad doesn't
  have. Plan to spend the first week of J-2 on a focused design
  pass (paper / Figma) before committing code. Fallback if it
  blocks: ship 2D editing in J-2, treat 3D gizmo as a post-J-4
  follow-up. Don't let it gate the rest of the phase.
- **Per-type adapter coverage.** 40 adapters is a lot; J-3 covers
  ~14 high-traffic ones. The remaining tail risks user reports of
  "I can't edit X on iPad." Mitigation: read-only fallback that
  links to "edit on desktop" for missing types.
- **Undo coherence with sequencer undo.** `SequencerViewModel`
  already owns an `UndoManager` for effect edits. Layout edits
  can share the same undo manager (and therefore the same Cmd-Z
  surface) or live in their own. Decide during J-2 — leaning
  toward separate, since layout edits change `xlights_rgbeffects.xml`
  while effect edits change the sequence document.
- **Memory and per-frame cost of overlays.** Drawing N model labels
  + N first-pixel markers + grid lines per frame is fine on
  desktop; on iPad we'll want to batch through `XLGridMetalBridge`
  primitives and likely cache labels.
- **Coordinate conversions.** Desktop's `ViewportMgr` does
  world↔screen mapping; iPad will need the same helper exposed.
  Already partly there — `XLMetalBridge.fitAllModelsForDocument`
  knows how to compute camera bounds — but explicit
  `unproject(touchPoint) → worldXY` and the inverse are the next
  step.
- **`xlights_rgbeffects.xml` write contention.** Layout edits
  modify the same file the desktop writes. iCloud sync between a
  Mac running xLights and the iPad open in the editor could
  collide. v1 ships with last-writer-wins; if it bites, revisit
  with a real merge strategy.

## Open questions

- Should layout undo share `SequencerViewModel.undoManager` or get
  its own? (J-2 decision.)
- Single layout-group property page or one shared with the model
  property page? (J-1 decision.)
- 3D gizmo gesture set — translate axes, rotate rings, scale handles
  — exact mapping. (J-2 design week.)
- Add Model toolbar — top bar, side rail, or popover? Affects how
  much horizontal canvas space we keep. (J-3 decision.)
