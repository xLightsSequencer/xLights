# Phase J — iPad Layout Editor: touch + Pencil UX design

Sibling to [`phase-j-layout-editor.md`](phase-j-layout-editor.md).
That file scopes the *engineering* phases (bridge surface, property
pages, model creation, multi-select ops). This one is the
*interaction* design — what the user does with finger and Pencil,
how the desktop's mouse vocabulary translates, and what the
toolbar / overlays look like.

The descriptor handle pipeline (R-1..R-10 in
[`handle-system-refactor.md`](handle-system-refactor.md)) was built
gesture-agnostic. A `handles::WorldRay + handles::Modifier` is what
a `DragSession` takes; whether it came from a mouse, a finger, or
a Pencil is irrelevant. The iPad layer just has to produce that
ray + modifier value from whatever gesture the user made.

---

## Design goals

1. **Recognisable as xLights.** Same handle palette (cyan corners,
   blue endpoints, orange centre, yellow active, red/green/blue
   axes, purple locked). Same conceptual model (a model has
   handles; you pick one and act on it). A user who knows desktop
   should grasp iPad in 30 seconds.
2. **Touch-first, not touch-tolerant.** Fingers don't hover. They
   are fat. They come in twos and threes. Gestures (pinch, twist,
   two-finger drag) are first-class, not afterthoughts.
3. **Pencil-enhanced, never Pencil-required.** Every operation
   reachable with finger-only. Pencil makes precision operations
   *better*, not *exist*.
4. **Discoverable.** No "click the orange sphere twice to cycle
   tools." Modes live in a persistent toolbar; affordances are
   labelled.
5. **Apple-HIG-compatible.** Apple's split is *Pencil for
   precision, fingers for navigation*. We follow that.

---

## Input vocabulary

The mental model: **Pencil acts on geometry; finger acts on the
view.** Direct manipulation routes through Pencil when present;
finger taps still work but live in a "casual" tier with looser
hit-test tolerance and bigger UI affordances.

### Pencil

| Gesture | Maps to | Desktop equivalent |
|---|---|---|
| **Hover** (above screen, M2 iPad+) | Cursor-equivalent — paints handle highlights, tooltips, snap previews | Mouse hover |
| **Tap** | Precise handle pick / model select / button | Left-click |
| **Drag** | Handle drag (descriptor `DragSession`) | Left-click-drag |
| **Barrel tap / squeeze** (Pencil Pro) | Contextual menu — Delete Point, Add Curve, etc. | Right-click |
| **Double-tap on barrel** (Pencil 2+) | Tool cycle (Translate → Rotate → Scale → …) | Click centre sphere |
| **Pressure** | Reserved for future ([`Open questions`](#open-questions)) | — |

When no Pencil is detected, the equivalents fall back as below.

### Finger

| Gesture | Action |
|---|---|
| **Single tap on handle / model body** | Same as Pencil tap (handle pick / select). Looser tolerance: 28pt vs Pencil's 14pt. |
| **Single tap on empty canvas** | Deselect |
| **Single-finger drag on handle / selected model body** | Handle drag / model body translate (3D needs unproject helper). |
| **Single-finger drag on empty canvas** | Camera orbit (3D) / camera pan (2D). |
| **Long-press on handle / model** | Contextual menu (Pencil barrel-tap equivalent for non-Pencil users). |
| **Two-finger pinch** | Camera zoom (default); on a selected model body in scale mode → uniform model scale. |
| **Two-finger twist** | Camera rotate in 3D (around look-at point); on a selected model in rotate mode → rotate around Z. |
| **Two-finger drag** | Camera pan in 3D; in empty space in 2D → marquee (multi-select). |
| **Three-finger tap** | Undo (system-standard iOS gesture). |

### Hardware keyboard (optional)

| Key | Action |
|---|---|
| Arrow keys | Nudge selected model 1 world unit |
| Shift + Arrow | Nudge 10 units |
| Cmd-Z / Cmd-Shift-Z | Undo / redo (already wired) |
| Delete / Backspace | Delete selected model |
| Escape | Cancel active drag (`m_dragSession->Revert()`) |
| Tab | Cycle tool mode (Translate / Rotate / Scale) |
| 1 / 2 / 3 | Direct-select tool mode |

---

## Tool-mode UI — toolbar, not CentreCycle

**Decision: replace CentreCycle's "tap centre sphere to advance
axis_tool" with a persistent toolbar.**

Reasons:
- Discoverability. Desktop users learn CentreCycle from the cursor
  changing on hover; iPad has no such teach.
- Touch target. The centre sphere is ~10pt on a typical iPad zoom;
  a toolbar button is 44pt minimum.
- Mode visibility. The toolbar always *shows* which tool is
  active. CentreCycle hides it.

Toolbar (bottom-anchored on iPad in editor mode):

```
┌──────────────────────────────────────────────────────────────┐
│  ◀ Select   ↕ Translate   ⟲ Rotate   ⤡ Scale   ⫯ Vertex      │
│                                                                │
│  Snap □    Uniform □    Lock Axis: ⦿ Free  ○X  ○Y  ○Z         │
└──────────────────────────────────────────────────────────────┘
```

- **Select / Translate / Rotate / Scale / Vertex** — radio-style
  picker. Drives the `axis_tool` member on the active screen
  location (which already feeds `GetHandles(tool=…)` via the
  descriptor pipeline).
- **Vertex** mode is shown only when the selected model is a
  PolyLine / MultiPoint / Custom. In Vertex mode, taps add a
  vertex, drag moves one, long-press deletes.
- **Snap** — replaces desktop's Shift-during-drag snap-to-grid.
  Persistent state.
- **Uniform** — replaces Shift-during-resize aspect-lock. Only
  visible in Scale mode.
- **Lock Axis** — replaces desktop's per-axis-arrow click. The
  user picks the constraint *before* dragging. Free = unconstrained
  (cursor-position drives all axes).

The toolbar visibility / contents adapt to the selected model
type. Locked / from-base models grey out everything except Select.

### What happens to CentreCycle?

Still emitted by `GetHandles` as a `selectionOnly` descriptor at
the model centre — it's the orange selection sphere. Tapping it
is a **no-op** in the new design (we don't cycle by sphere). The
sphere is just a "the model is selected" visual.

Equivalent `AdvanceAxisTool` operations:
- Tab key on hardware keyboard.
- Pencil double-tap-on-barrel.
- (Optional) Three-finger tap if not consumed by undo.

---

## Per-operation specs

### Select

- **Tap on model** (Pencil or finger): selects. Inline action bar
  appears anchored above the model with quick actions (Lock,
  Group, Delete, Duplicate, Properties…).
- **Tap on empty**: deselect.
- **Tap on already-selected model**: no-op (stays selected; in
  desktop this would be a no-op too).
- **Long-press on model**: contextual menu.

### Translate

- **Drag the model body** (Pencil or finger): translate.
  - 2D: ray.origin.xy directly.
  - 3D: unproject touch through the model's depth plane.
- Lock Axis from toolbar constrains the motion.
- Snap toggle rounds to `Display2DGridSpacing`.
- Multi-select: applies offset to all selected (already wired
  through descriptor session multi-select code at
  LayoutPanel.cpp:5247).

The axis-arrow gizmo (red/green/blue arrows) still draws in
Translate mode but functions as a *visual reminder* + *optional
constraint trigger*: tap an arrow → that axis becomes the locked
axis on the toolbar. The arrows are sized larger on iPad (~24pt
arrowhead vs desktop's ~12pt).

### Rotate

- **One-finger drag on model body**: rotate around the active
  axis (per toolbar Lock Axis; if Free, defaults to Z in 2D, last-
  used in 3D).
- **Two-finger twist on model body**: rotate around Z directly,
  ignoring lock axis. Two-finger twist on a multi-touch is the
  natural rotate gesture.
- The axis-ring gizmo (the three rings) draws and is tappable for
  axis-lock; same pattern as arrows.

### Scale

- **Drag a corner handle** (2D): resize. Uniform toggle locks
  aspect.
- **Pinch on model body**: uniform scale. Always (overrides lock).
- **Drag axis cube** (3D): per-axis scale. Uniform toggle scales
  all axes.

### Vertex (PolyLine / MultiPoint / Custom)

- **Tap on segment**: add vertex at tap point.
- **Drag on vertex**: move it.
- **Long-press on vertex**: contextual menu (Delete, Add Curve,
  Remove Curve, Set Hard Curve Point).
- **Drag on curve-control point**: adjust curve.
- **Pencil-only enhancement**: pencil-draw a path → auto-place
  vertices at detected corners. Bezier-fit between. (Future, see
  Open questions.)

### Shear (Arches / Window Frame)

- Same as desktop: the shear sphere is a draggable handle
  (existing descriptor pipeline).
- Pencil + drag works as-is.
- Finger + drag works but the sphere is small; consider showing a
  larger "shear" puck when the model is selected in 2D.

### Create new model

- **Add Model** button in the toolbar → sheet with model types.
- Tap a type, tap on canvas → drop with default size.
- Drag during drop → set initial bounding box (existing
  `BeginCreate` session math, gesture-agnostic).
- PolyLine: each tap adds a vertex; double-tap or Done button
  ends. (Pencil-draw path is a future enhancement.)

### Multi-select

- **Two-finger drag in empty space**: marquee select. Tighter
  than mouse drag because two fingers naturally splay.
- **Tap-and-hold + tap another model**: additive selection (iOS-
  native pattern; Procreate uses this).
- **Tap selected model when others are selected**: removes from
  selection.

### Contextual menus

Replace right-click. Triggered by Pencil barrel-tap or finger
long-press on:
- A model: Lock, Duplicate, Delete, Group, Align (submenu when
  multi-select), Properties.
- A handle / vertex: Delete Point (vertex only), Add Curve / Remove
  Curve (between segments), Set Centre Offset.
- A segment: Add Point, Define Curve.

Menus are SwiftUI `ContextMenu` / `Menu` so they get native iOS
visuals + a11y for free.

---

## Visual affordances (replacing cursor)

| Desktop affordance | iPad replacement |
|---|---|
| Cursor hover → yellow handle | Pencil hover → yellow handle (1:1) |
| Cursor hover → resize cursor | Pencil hover → tooltip label ("Resize NW") |
| Cursor hover → rotate cursor | Pencil hover → tooltip + ring pulse on the handle |
| Active handle (cyan during drag) | Same colour; finger lift triggers a brief haptic + colour flash |
| Cursor changes during multi-select | Two-finger marquee draws as a translucent rectangle |
| Modifier-held state (Shift, etc.) | Toolbar toggle highlight |
| Snap preview lines | Same; drawn during drag when Snap is on |
| Locked = purple handles | Same |

Always-on overlays (controllable from toolbar):
- Grid (2D only)
- Bounding box (per model)
- Model labels (name)
- First pixel marker
- Layout dirty indicator (corner of the editor)

---

## Pencil-specific enhancements

### Hover (M2 iPad + Pencil 2 / Pencil Pro)

The big win. Pencil hover restores cursor-equivalent feedback:
- Handle yellow-tint on hover (existing `MouseOverHandle` path,
  already takes `optional<Id>`).
- Tooltip labels ("Top-Right corner — drag to resize").
- Snap preview lines while the Pencil drifts over a handle.
- Live cursor coordinates in a status indicator.

Implementation: bind to `UIHoverGestureRecognizer` (or
`UIPencilHoverPose` on Pencil Pro for richer state including
hover-distance + angle), forward to the same descriptor hit-test
that finger hit-uses, set `highlighted_handle` via
`MouseOverHandle`.

### Barrel tap / squeeze (Pencil Pro)

`UIPencilInteraction` delivers `.tap` events with a
`preferredTapAction` user setting. Map to **contextual menu** by
default. User can change in iOS Settings → Apple Pencil if they
prefer e.g. "switch tool".

### Double-tap on barrel (Pencil 2 / Pencil Pro)

Defaults to **cycle tool mode** (Translate → Rotate → Scale →
Vertex). Equivalent to `AdvanceAxisTool` but on the *tool* axis,
not the *constraint axis*.

### Pressure (all 2nd-gen+)

Reserved. Possible uses:
- Pressure > threshold during drag → finer (1/10 unit) precision
  motion. Light touch = normal motion.
- Pressure during rotate → fine-grained angle vs snap.
Hold this until prototyping reveals whether it's actually useful.

### Tilt

Unlikely to be useful here. The desktop has nothing analogous.
Reserved.

### Pencil-only sketch mode (future)

In PolyLine create mode, with Pencil, the user could **draw a
path** and we'd Bezier-fit vertices automatically. This is the
"feels like iPad" feature that has no desktop equivalent. Punt
to Phase J-3 or later.

---

## Mapping to the descriptor pipeline

The descriptor pipeline doesn't change. The bridge layer's job is:

```
Gesture → (WorldRay, Modifier) → existing DragSession
```

Specifically:

| New iPad input | Existing descriptor API call |
|---|---|
| Pencil tap on handle | `HitTest(descs, proj, point)` → `BeginDrag(hit.id)` |
| Pencil drag motion | `m_dragSession->Update(ray, mods)` |
| Pencil lift | `m_dragSession->Commit()` |
| Pencil hover | `HitTest` for highlight only; `MouseOverHandle(hit.id)` |
| Toolbar Snap toggle | `Modifier::Shift` bit on every drag update |
| Toolbar Uniform toggle | `Modifier::Shift` (already what Shift means for scale) |
| Toolbar Lock Axis | New `Modifier::AxisLockX/Y/Z` bits — or pre-filter the ray inside the gesture handler before forwarding |
| Pinch on selected model | New `BeginDrag(Id{Role::PinchScale})` or reuse `Role::CentreCycle` with a new session class |
| Two-finger twist | Same, with `Role::TwoFingerTwist` |
| Long-press contextual menu | No drag session; SwiftUI menu reads `selectedHandleId` for gating |

What **does** need to change in the descriptor pipeline:
1. **`Modifier` may need axis-lock bits.** `Modifier::None`,
   `Shift`, `Control` exist today. Adding `AxisLockX/Y/Z` lets the
   toolbar Lock Axis state flow through naturally. Alternative:
   filter the ray before calling `Update` (more bridge-side code,
   simpler descriptor API).
2. **A new descriptor `Role`** if we add pinch-on-model and
   twist-on-model as first-class gestures. `Role::PinchScale`,
   `Role::TwistRotate`. Plus matching session classes per
   subclass (or a generic one in the base if the math is
   uniform).

Everything else lands as gesture wiring in `XLMetalBridge.mm` +
SwiftUI overlays. No new core API.

---

## Prototyping plan

The design doc commits us to:
- Toolbar mode model
- Persistent modifier state (Snap / Uniform / Lock Axis)
- Pencil hover → cursor-equivalent
- Pinch-on-model = uniform scale
- Two-finger twist-on-model = rotate Z
- Long-press → contextual menu

Things to *prototype* before committing code:
1. **Tap-vs-drag threshold.** How much motion before a tap becomes
   a drag? iOS default is 10pt; might need 20pt for fat fingers
   on small handles.
2. **Pencil hover + active drag conflict.** When dragging with a
   finger, does Pencil hover continue to update `highlighted_handle`?
   Probably yes (informational), but verify it doesn't visually
   fight the active-drag cyan.
3. **Toolbar position.** Bottom-anchored vs side rail. Affects
   how much canvas the user gets in landscape; landscape is the
   primary orientation for layout editing.
4. **Inline action bar.** Floats above the selected model? Or
   anchored to the toolbar? Latter is less clever but more
   stable when the model moves.
5. **Three-finger tap.** iOS has a system gesture for it (text
   selection). Consuming it for undo conflicts. Decide.

---

## Open questions

- **Pencil pressure** — does it actually help? Or is it a gimmick
  here? Prototype, then decide.
- **Sketch-mode polyline create** — pencil-draws-a-path → auto-
  vertices. Compelling but post-MVP.
- **3D camera precision** — the current "drag-on-empty-3D = orbit"
  is fast and approximate. Do we add a separate precision-camera
  mode (e.g. on-screen joystick) for fine framing?
- **Spatial undo scope** — `Cmd-Z` undoes the last layout op. But
  if the user is mid-drag, does Cmd-Z cancel the drag (Revert) or
  pop the last *committed* op? Recommend: during-drag = Revert
  (Escape key behaviour); after-commit = pop.
- **Apple-Pencil-not-paired fallback.** When the Pencil button
  shows in the toolbar (visible indicator that "Pencil enhances
  this app"), or hidden when no Pencil is configured? Apple's HIG
  prefers hidden until detected.
- **Accessibility.** VoiceOver users can't use any of this. Need
  a parallel "list of models + property panel" path that's already
  partially there from J-1. Audit during J-2.

---

## What lands in code first

Recommended sequence once design lands:

1. **Toolbar UI** — radio buttons, modifier toggles. SwiftUI; no
   descriptor changes. Wires to existing `axis_tool` /
   `Modifier` plumbing.
2. **Pencil hover** — `UIHoverGestureRecognizer` →
   `MouseOverHandle`. Test on M2 iPad. ~1 day.
3. **Long-press contextual menu** — SwiftUI `ContextMenu` reading
   `selectedHandleId`. ~1 day.
4. **Two-finger gestures (pinch / twist on model body)** — new
   gesture recognisers in `PreviewPaneView`, plus new
   descriptor `Role` + session if we go that direction.
5. **Pencil barrel-tap / double-tap** — `UIPencilInteraction`,
   bind to menu + tool-cycle.
6. **3D body-drag with unproject** — touch-point → world plane.
   Independent of the gesture work.
7. **Rubber-band multi-select** — two-finger drag in empty
   canvas; reuses existing per-model selection bits.

Each piece is independently shippable behind a feature flag
during testing.

---

## Implementation status (2026-05-11)

| Item | Status | Notes |
|---|---|---|
| Toolbar UI (Move / Rotate / Scale) | ✓ landed | `LayoutEditorToolToolbar` in `LayoutEditorView.swift`. Bottom-anchored, model-scoped. CentreCycle path still wired but no longer the only way to change tool. |
| Snap / Uniform / Lock Axis toggles | ✓ landed | Persistent state on the bridge. `uniformModifier` ORs `Shift` into the drag modifier; `lockAxis` pre-filters the world ray onto an axis-aligned line. Session classes need no change. |
| `axis_tool` per-model getter/setter | ✓ landed | `XLSequenceDocument.axisToolForModel:` / `setAxisTool:forModel:`. Tool state is per-model. Toolbar seeds from the selected model on selection change. |
| 3D body-drag (model body, not handle) | ✓ landed | `XLMetalBridge.beginBodyDrag3D` / `dragBody3D` / `endBodyDrag3D`. Anchors a plane through the model's centre Z; subsequent touches project onto that plane. Lock Axis (X/Y) + Snap apply. |
| Pinch on selected model = uniform scale | ✓ landed | `beginPinchScale` / `applyPinchScaleFactor` / `endPinchScale`. Triggered when pinch centroid hits the selected model body at `.began`; pinch elsewhere stays camera zoom. Locked models reject. |
| Two-finger twist on model = rotate Z | ✓ landed | `beginTwistRotate` / `applyTwistRotationRadians` / `endTwistRotate`. Same intent-at-began pattern as pinch. Sign negated to match the "scene follows the fingers" feel. |
| Pencil hover → cursor | ✓ verified 2026-05-13 | `UIHoverGestureRecognizer` on the LayoutEditor pane drives `setHoveredHandleAtScreenPoint:` → `MouseOverHandle`. User confirmed hover working with Pencil. |
| Pencil double-tap (Pencil 2 / Pro) | ✓ landed 2026-05-13 | `UIPencilInteractionDelegate.pencilInteraction(_:didReceiveTap:)` calls `cycleAxisToolForSelectedModelForDocument:` (no-position version of CentreCycle tap). Reads new axis_tool back from bridge and writes `settings.axisTool` so the toolbar pill updates. |
| Pencil Pro squeeze | ✓ landed 2026-05-13 | `pencilInteraction(_:didReceiveSqueeze:)` (.ended phase) posts `.layoutEditorPencilUndo`; LayoutEditorView listens and routes through its existing `performUndo()` so dirty markers + canvas repaint stay correct. |
| Long-press contextual menu | ✓ landed | `UILongPressGestureRecognizer` (0.45s) → `inspectHandleAtScreenPoint:` → `.layoutEditorContextMenu` notification → SwiftUI `.confirmationDialog`. Items per hit type: vertex → Delete Point; segment → Add Point / Define Curve / Remove Curve; curve_control → Remove Curve. Bridge methods on `XLSequenceDocument` (`deleteVertexAtIndex:`, `insertVertexInSegment:`, `setCurve:onSegment:`). |
| Inline action bar above selected model | ✓ landed | Bridge: `screenAnchorPointForModel:` projects the model's top-centre to UIKit screen coords. SwiftUI `InlineModelActionBar` uses `TimelineView(.animation)` to re-query each animation frame so it tracks pan / zoom / orbit / drag without observer wiring. Actions: Lock/Unlock toggle, model name, Fit Selected (viewfinder), dismiss. Duplicate / Delete / Group need new bridge methods + confirmation flow — deferred. |
| Rubber-band multi-select (two-finger marquee) | not started | Needs multi-selection state in the view model (currently single-select only). Foundational for J-4 (align / distribute). |
| Layout editor text overlays (model name labels) | ✓ landed | Toggled from canvas controls ("Labels"). Bridge `modelLabelAnchorsForDocument:` returns batched (name, screen-anchor) for every model in the active layout group (off-screen filtered out). SwiftUI overlay renders one small Text view per anchor, refreshes at 30fps via `TimelineView`. SwiftUI text instead of Metal text — much simpler than `CoreGraphicsTextDrawingContext`, scales fine to typical show sizes. |
| Three-finger tap / swipe = undo | system handles | iOS routes three-finger gestures to the active `UndoManager` automatically; no wiring needed once layout undo plugs into the system manager (currently uses bridge-side `canUndoLayoutChange`). |

**Status as of 2026-05-13:**

Shipped beyond the original design-doc table:

- **Add Model.** Toolbar `+` menu lists a curated 18 model types
  (Arches, Tree, Star, Matrix, Cube, Custom, Poly Line, …). Pick
  one → editor enters "creation mode" with a top-anchored banner
  ("Tap canvas to place Arches") and a Cancel button. Tap or
  drag-release on the canvas places the model via
  `XLMetalBridge.createModelOfType:atScreenPoint:viewSize:for:`,
  which mirrors desktop's `InitializeLocation` →
  `FindPlaneIntersection` so the projection picks the camera-
  appropriate plane (XZ floor / XY wall / YZ side) and the new
  model lands where the user expects (e.g. icicles along a roof
  follow the roof's slope).
- **Drag-during-create.** Single-finger pan on the Add-Model
  banner opens a `BeginCreate` session at touch-down and sizes
  the model as the finger moves; release commits. Mirrors
  desktop click-drag-release. Tap-only (no movement) places at
  default geometry.
- **Single Line / TwoPoint 3D free-plane create.**
  `TwoPointPlaneCreateSession` (new) replaces the axis-locked
  `TwoPointTranslateSession` for the create flow. Reads
  `loc->GetActivePlane()` and intersects the drag ray with that
  plane, so a diagonal drag from a top-down view produces a
  diagonal line on the floor instead of an X-axis-locked
  horizontal segment.
- **PolyLine multi-vertex create.** Bridge:
  `appendVertexToPolyline:atScreenPoint:viewSize:forDocument:`
  and `modelUsesPolyPointLocation:forDocument:`. View model:
  `layoutPolylineInProgress: String?`. SwiftUI: banner switches
  to "Tap to add vertex to <name>  [Done]" after the first vertex
  is placed; each follow-on tap appends a vertex (or drag-creates
  a curved segment), Done / Esc / Return ends the mode. First
  append is special-cased: when `num_points == 2` and vertex 1
  still equals vertex 0 (the post-`CreateDefaultModel`
  placeholder), the bridge promotes vertex 1 instead of appending
  a third coincident vertex — otherwise nodes bunch at the
  origin. `PolyLineModel::AddHandle()` (no-arg) is called after
  each `screenLocation->AddHandle(...)` so `_polyLineSizes` /
  `_polyLeadOffset` / etc. stay sized correctly, and
  `m->Reinitialize()` runs to redistribute the 50 lights across
  the new segment count.
- **Delete model UI.** Trash button in the inline action bar
  with a `.alert("Delete <name>?")` confirmation. Calls
  `viewModel.document.deleteModel(name)`, clears the selection,
  refreshes the sidebar list, posts `.layoutEditorModelMoved`
  for canvas repaint. Persistence still requires Save; Undo or
  Discard rolls it back.
- **Camera-aware drag (vertex + body).** New
  `ModelScreenLocation::RefreshActivePlaneFromCamera(preview)`
  helper wraps `GetBestIntersection` + `SetActivePlane`. Called
  at drag-begin in `pickHandle:` (for any PolyPoint
  vertex/segment/curve drag) and in `appendVertexToPolyline:`
  (for the new-vertex extend). `PolyPointCreationSession::Update`
  rewritten to be plane-aware: derives a full `(lx, ly, lz)`
  from the world intersect and only overwrites the two in-plane
  axes; the perpendicular axis keeps its `_savedLocal` value.
  3D body-drag (`beginBodyDrag3D` / `dragBody3D`) now picks
  XY / XZ / YZ from the camera at drag-begin and tracks the
  plane on `_bodyDrag3DPlane`. Toolbar Lock Axis now honours
  Z (drag-along-floor in top-down view stays at fixed Z).
- **Live vertex re-init.** PolyPoint vertex drags call
  `m->Reinitialize()` on each `dragHandle:toScreenPoint:` update
  (and on session commit) so lights redistribute in real time
  as the user drags. Mirrors desktop's per-frame
  WORK_MODELS_CHANGE_REQUIRING_RERENDER queueing.
- **Axis-handle hit-test tolerance.** New
  `HitTestOptions.axisHandleTolerance` field (defaults to 0 →
  reuse the general value for backwards compat); iPad bridge
  sets it to 28pt while keeping vertex/endpoint/corner handles
  at 60pt. Stops the X/Y/Z arrow heads from swallowing body-
  drag taps when their projections land near the model body
  (e.g. in top-down views).

Remaining concrete follow-ups:

- **Rubber-band multi-select.** Two-finger drag in empty space →
  marquee. Needs `Set<String>` on the view model (replacing the
  current single-`String?` selection) and rendering the marquee
  rectangle during drag. After it lands, the existing
  multi-select drag math in `OnPreviewMouseMove3D` (descriptor
  pipeline, dispatches by role) ports over almost verbatim.
- **Action bar Duplicate / Group.** Bridge methods +
  confirmation flow. Group needs a multi-select foundation
  first.

Everything else from the design doc table is shipped. The next
push beyond Phase J's UX layer is J-3 (per-type property pages)
and J-4 (multi-select operations).
