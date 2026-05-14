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
