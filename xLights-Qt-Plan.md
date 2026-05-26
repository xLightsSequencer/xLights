# xLights Qt — MVP Implementation Plan

## Locked Decisions

| Question | Answer |
|---|---|
| Output directory | `bin-qt/` (separate from `bin/`) |
| Build | CMake with `XLIGHTS_BUILD_QT=ON`, regenerate via `VS2022.bat` (includes `-DCMAKE_PREFIX_PATH="D:/Qt/6.8.3/msvc2022_64"`) |
| Effects panel | QTabWidget with 4 tabs (see Effect Panel Redesign below) |
| Palette | PaletteWidget (8 swatches) at top of Tab 1; color = QColorDialog |
| Model preview | PreviewWidget — node positions when available, else 2D grid / 1D strand |
| House preview | HousePreviewWidget — physical yard layout from globalPositions |
| Rendering | **All** effect rendering uses src-core (`RenderableEffect::Render`) via `QtRenderBridge`, matching the wx and iPad apps. |
| Playback | Audio-synced via QMediaPlayer; 40ms tick; previews update live |
| Render All | Snapshot of current frame for all models → house preview; on stop/pause too |
| Sequencer editing | View + select + move (same row + cross-row) + resize + drag-drop from toolbar |
| CMakeLists | New `.cpp` files in `src-ui-qt/` must be added to `XL_QT_SOURCES` in `CMakeLists.txt` |

## Architecture

```
src-core/             ← unchanged, shared with wx + iPad
src-ui-wx/            ← unchanged, wx build unaffected
src-ui-qt/
  App/                MainWindow, QtXLightsApp, PlaybackController, TransportToolBar
                      ExportDialog, RenderDetailDialog
  Bridge/             QtSequenceDoc, QtRenderBridge, QtRenderContext
                      QtModelStub, QtTextDrawingContext, FseqWriter, SubBufferUtil
                      QtEffectRenderer (structs only — software renderer removed)
  Sequencer/          SequencerModel, SequencerController, SequencerGridCanvas
                      RowHeaderWidget, TimingRulerCanvas, SequencerWidget
  Effects/            EffectPanelWidget, EffectControlBuilder, EffectToolBar
                      PaletteWidget, BufferWidget, SubBufferWidget
                      controls/ (SliderSpinRow, CheckRow, ComboRow, TextRow,
                                 FilePickerRow, ColorSwatchButton)
  Preview/            PreviewWidget
  Layout/             ModelLayoutCanvas (base: geometry, drawing, selection, group highlight)
                      HousePreviewWidget (extends ModelLayoutCanvas; adds setModelPixels/clearPixels)
                      LayoutWindow (Phase 11 — tabbed list + property panel + canvas)
                      ModelEditDialog (Phase 12 — sub-model/face/state editor)
                      ModelNodePreview (node dot canvas with lasso selection)
  Controllers/        ControllerVisualizerWindow (Phase 16 — port wiring + drag-drop)
  Info/               ControllerInfoWindow (kept for controller list; Visualize button removed)
  Graphics/           RhiCanvas
CMakeLists.txt        xLights-Qt target, bin-qt/ output
```

## Completed Work

### Phases 1–6a — Scaffold through software renderer ✓
### Phase 6b — Model topology + house preview ✓
### Phase 6c — Real src-core rendering ✓
### Phase 6d — Group rendering in sequencer ✓
### Phase 6e — Remove software renderer ✓
### Phase 7 — FSEQ export ✓
### Phase 8 — House preview / layout ✓ (read-only; interactive editing in Phase 11)
### Phase 9 — Effect drag-drop onto timeline ✓

### Rendering model (current) ✓
- **Block selected** → `renderAllLayers()` → src-core render of that model → model preview + house entry
- **Render All** → `renderAllModels()` → snapshot of all models at current playhead → house preview
- **During playback** → `renderAllLayers()` each frame for selected model (live)
- Rendering uses real timing (startMs/endMs/curMs/frameMs) and correct layer blend modes
- `onBlockSelected` does NOT call `renderAllModels()` — only selected model is rendered

### Phase 11 — Layout window ✓
- `LayoutWindow`: horizontal splitter — left (tabbed list: Models/Groups/Controllers + property table + buttons), right (ModelLayoutCanvas)
- Loads from `xlights_rgbeffects.xml` directly when no sequence is open (show-folder fallback)
- Properties show model/group/controller attributes; model props include Controller/Port
- **Edit** button (Models tab only): opens ModelEditDialog for selected model
- **Visualize** + **Upload** buttons (Controllers tab only): open visualizer / emit uploadRequested
- Tab switch clears selection and disables irrelevant buttons
- `_data` member stores effective sequence (live or fallback); all property lookups use it

### Phase 12 — Sub-model / Face / State editor ✓
- `ModelEditDialog`: 3-tab dialog + `ModelNodePreview` right panel
- **Sub-Models tab**: list + name/layout/type/bufferstyle form + node ranges table (line0/line1/…)
  - Row click → preview highlights nodes in that range; uses `itemClicked` (fires on every click)
- **Faces tab**: list + type (NodeRange/SingleNode/Matrix) + phoneme grid (Feature|Nodes|Color swatch)
  - Nodes column: plain text for all types (comma-separated ranges like "1,3-5,10")
  - Matrix type: file-path rows with browse buttons
  - "Force custom colors" checkbox shows/hides Color column; persists as `CustomColors` attribute
- **States tab**: list + type (NodeRange/SingleNode) + Name|Nodes|Color swatch table
  - Parses `s1="nodes"`, `s1-Color="color"`, `s1-Name="name"` triplets; sorted by N; up to 200
  - Saves in same format; state entry names normalized to lowercase alphanumeric
  - Vertical header shows s1, s2, … row numbers; blank rows preserved
  - "Force custom colors" checkbox shows/hides Color column
  - Row click → preview highlights nodes; uses `itemClicked`
- **ModelNodePreview**: renders to off-screen QPixmap (no QPainter-on-widget crash);
  highlights selected row's nodes in yellow; lasso rubber-band selection writes range back to row
  - `nodeCount` and `nodePositions` fixed: wiring order (positions[i] = node i+1), not buffer order
  - `qBound(min, val, max)` argument order fixed; radius: `qBound(1.5, 180/size, 3.5)`
- All table ops wrapped in `setUpdatesEnabled(false/true)` to suppress Qt qBound asserts at high DPI

### Phase 13 — Controller upload ✓
- `ControllerUploadDialog`: non-modal dialog with progress bar + scrolling log
- `QtUploadCallbacks` implements full `UICallbacks` interface (prompts auto-accept defaults)
- Upload runs on `QThread`; signals cross to main thread via QueuedConnection
- `QtRenderBridge::upload()`: calls `SetInputUniverses()` first then `SetOutputs()`; uses existing s_ctx/s_mm
- Entry point: **Upload** button in LayoutWindow's Controllers tab

### Phase 16 — Controller visualizer ✓
- `ControllerVisualizerWindow`: controller selector + scrollable port rows + available models list
- `PortRowWidget`: custom port rows with `ModelBoxButton` widgets (colored, drag-to-move)
- Overflow indicator when assigned channels exceed port capacity
- Drag model boxes between ports or back to available list; saves Controller/ControllerConnection
- Uses `_data` member (same live/fallback pattern as LayoutWindow)
- Entry point: **Visualize** button in LayoutWindow's Controllers tab
- `QtControllerInfo` now includes `pixelPortCount`/`serialPortCount`/`pixelPortChannels` from ControllerCaps

### Phase 18a — ModelManager XML loading ✓
- `QtRenderBridge::ensureInitialized()`: eagerly creates `s_ctx` + `s_mm` when `setShowFolder()` is called (not lazy on first render), so `modelManager()` returns valid data immediately.
- `QtRenderBridge::modelManager()`: public accessor returning `s_mm.get()` (returns `nullptr` if not yet initialized).
- `LayoutWindow::setRenderBridge(QtRenderBridge*)`: stores bridge so `refresh()` can access `modelManager()`.
- `LayoutWindow::refresh()` calls `_canvas->loadLayoutFromManager(_bridge->modelManager())` — falls back to `loadLayout(_data)` if bridge unavailable.

### Phase 18b — Screen-location geometry ✓
- `ModelLayoutCanvas` now has two drawing modes:
  - **Rect mode** (`loadLayoutFromManager(mm)`): iterates all `Model*` in `ModelManager`, reads `GetModelScreenLocation().GetLeft/Right/Top/Bottom()` → stores as `ModelRect { name, left, right, top, bottom, isGroup }`.
  - **Dot mode** (`loadLayout(seq)`): existing per-node dot drawing from `globalPositions` (unchanged; still used by `HousePreviewWidget`).
- `paintRects()`: Y-flipped world→widget mapping (`wy = (1 - (y - minY)/range) * height`). Models drawn as filled blue rectangles, selected = yellow border, group members = outlined. Group highlight draws dashed bounding box around union of member rects.
- `mousePressEvent` rect mode: click selects smallest rect that contains the click (handles 1D models with ±1.5px padding).
- `ModelGroup*` instances are skipped when drawing rects (they appear only as group-highlight outlines when their group is selected in the Groups tab).

### Data layer additions ✓
- `QtModelInfo`: `controllerName`, `controllerPort`, `subModels`, `faces`, `states`
- `QtControllerInfo`: `pixelPortCount`, `serialPortCount`, `pixelPortChannels`
- `QtSubModelInfo`: `name`, `layout`, `type`, `bufferStyle`, `ranges` (line0/line1/… format)
- `QtFaceInfo`: `name`, `type`, `forceColor`, `attrs` map
- `QtStateEntry`: `name`, `nodes`, `color` (from sN-Name, sN, sN-Color attributes)
- `QtStateInfo`: `name`, `type`, `forceColor`, `entries`
- `nodeCount` = `nodes.size()` (physical nodes, not `bufW*bufH`); `nodePositions` in wiring order

### Crash / assert fixes ✓
- QPainter 0x3F0 crash: `ModelNodePreview` renders to QPixmap cache; `paintEvent` only blits
- Qt qBound assert at high DPI: all `setColumnWidth` removed; table mods wrapped in `setUpdatesEnabled`
- `qBound` arg order bug in `rebuildCache`: fixed to `qBound(min, val, max)` not `qBound(min, max, val)`
- `nodeCount = bufW*bufH` bug: was returning buffer size (e.g., 6000) instead of node count (200)

## Effect Panel Redesign (right-side panel)

Replace the current single-pane `EffectPanelWidget` with a `QTabWidget` containing four tabs.

### Tab 1 — Effect Settings
- **Top**: `PaletteWidget` (8 colour swatches, unchanged)
- **Below palette**: scrollable effect controls generated by `EffectControlBuilder` from
  `effectmetadata/*.json`

### Tab 2 — Colour Settings
- Secondary colour controls populated from `C_*` keys in the effect's settings map.

### Tab 3 — Layer Blending
- Blend mode selector, layer opacity / brightness.

### Tab 4 — Buffer Settings
- Buffer style combo, buffer transform, sub-buffer region controls, 2D buffer canvas.

## Remaining Work

### Phase 6d — Multi-layer sequencer rows
`SequencerRow` → `QList<SequencerLayer>` (each layer has blocks + blend mode + collapsed flag).
Grid canvas draws collapsible sub-rows per layer with blend mode label.
QtSequenceDoc parses EffectLayer children as separate layers (currently merged to one).

### Phase 10 — Timing tracks
Dedicated timing row type. TimingTrackRow: list of marks (frame + label).
TimingRulerCanvas extended: click to add marks, drag to move.
Import/export .xtiming. Snap-to-timing-mark in SequencerController.

#### Phase 13b — FPP Connect (TODO)
FPP Connect uploads FSEQ files + media to Falcon Player instances. The core code is
wx-free (`src-core/controllers/FPP.h/.cpp`) and ready to use.

**What to add to `ControllerUploadDialog`:**
- "FPP Connect" section below the standard upload log
- Discover FPP instances on the network via `FPP::PrepareDiscovery()` (multicast + HTTP)
- List discovered instances in a table (hostname, IP, version, upload checkbox)
- "Upload FSEQ + Media" button: for each checked instance:
  1. `FPP::AuthenticateAndUpdateVersions()` — verify connection
  2. `FPP::PrepareUploadSequence(fseqFile, seqPath, mediaPath, type)`
  3. Frame-by-frame `AddFrameToUpload()` + `FinalizeUploadSequence()`
  4. `FPP::CheckUploadMedia(mediaPath, baseName)` — upload audio/video if missing
  5. `FPP::UploadPlaylist(playlistName)` — optional playlist update
- Progress reported through the existing `UICallbacks` / progress bar
- FSEQ path comes from the currently exported sequence (ExportDialog output) or a file picker
- Media path from the sequence's `mediaFile` field

**Key FPP API facts:**
- FPP class inherits `BaseController`; `SetOutputs()` / `SetInputUniverses()` also available
- Upload endpoints: `/api/file/sequences`, `/api/media/`, `/api/playlist/`
- FPP v7+ uses chunked 16 MB uploads via `uploadFileV7()`
- Discovery: UDP multicast `239.70.80.80:4048` + HTTP `/api/fppd/multiSyncSystems`
- No wx dependencies; uses libcurl + nlohmann/json + zstd

### Phase 14 — Import sequence dialog
ImportSequenceDialog: file picker + model-to-model mapping table.

### Phase 15 — Batch render
BatchRenderDialog: sequence list, output folder, per-file progress on QThreadPool.

### Phase 17 — Value curves and color curves in effect panel
Value curve editor: graphical envelope beside each slider (VC button → curve mode).
Color curve editor: per-palette-slot gradient over effect duration.
Uses existing ValueCurve / ColorCurve types from src-core/render/.

### Phase 18 — Model/House Preview rewrite using src-core screen primitives

Phases 18a and 18b are complete (see Completed Work). Remaining optional phases:

#### 18c — Mesh-based DMX model previews (optional, lower priority)

For DMX models that carry 3D OBJ meshes:

- `DMX/Mesh.h` — `class Mesh` wraps TinyObjLoader; `Load(path)`, `vertices`, `normals`,
  `faces` — no wx
- `graphics/xlMesh.h` — lighter weight; same TinyObjLoader backend
- Render path: load OBJ → extract vertex list → project 3D → draw polygon outline in
  QPainter (or hand off to a `QOpenGLWidget` once Phase 18d lands)

#### 18d — GPU-accelerated canvas via QOpenGLWidget (future)

Google ANGLE (a GL ES → D3D/Vulkan/Metal translator) is **not** shipped with
Qt 6 on Windows — Qt dropped bundled ANGLE after Qt 5.
`QOpenGLWidget` is the Qt-recommended path for GPU-accelerated custom rendering:

- Subclass `QOpenGLWidget`, override `initializeGL()` / `resizeGL()` / `paintGL()`
- Qt on Windows selects the best backend automatically (native OpenGL, or ANGLE via
  a custom `QSurfaceFormat` with `RenderableType::OpenGLES` if ANGLE is installed)
- `QOpenGLWidget` can coexist in a QSplitter / QLayout alongside normal widgets
- **When to use**: replace `ModelLayoutCanvas` QPainter with a `QOpenGLWidget` if
  mesh rendering (Phase 18c) becomes a performance bottleneck, or when Phase 18d
  targets waveform/particle effects in the layout view
- **Prerequisite**: keep Phase 18a–18c on QPainter first; migrate to GPU only if
  frame rate demands it

### Phase 19 — Model property panel parity (in progress)

**Goal:** replace the small read-only `QTableWidget` in
`src-ui-qt/Layout/LayoutWindow.cpp` with a Qt equivalent of wx's
`wxPropertyGrid` model panel. Editable, all settings visible,
per-model-type properties, popup dialogs.

**Why:** the current Qt model panel shows ~10 fields
(Name/Type/Nodes/Buffer/StartChannel/World/Scale/Controller/Port).
The wx adapter (`src-ui-wx/modelproperties/ModelPropertyAdapter.cpp`,
1702 lines) plus ~25 per-type adapters
(`src-ui-wx/modelproperties/adapters/`) expose hundreds of fields
with categories, controller-connection sub-tree, and dialogs
(sub-models / faces / states / dimming / aliases).

**Widget approach:** `QTreeWidget` + item delegates (no third-party
deps). Categories are bold top-level items; rows are
`Property | Value`. Data pulled from `Model*` via
`QtRenderBridge::modelManager()` — NOT from the slim `QtModelInfo`
snapshot in `QtSequenceDoc.h`.

#### Phase 19a — Foundation + common props ✓
- `src-ui-qt/Layout/LayoutPropertyTree.{h,cpp}` (renamed from
  `ModelPropertyTree` once it grew group + controller support).
- Categories implemented (read-only):
  Identity, Sizing & Channels, Layout, Appearance,
  String Properties, Controller Connection (basic),
  Sub-Models / Faces / States counts.
- Wired into `LayoutWindow`; the flat `QTableWidget` and
  `QStackedWidget` have been removed — one tree serves all
  three tabs.
- Added to `CMakeLists.txt` `XL_QT_SOURCES`.

#### Phase 19a' — Group + Controller property trees ✓
- `LayoutPropertyTree::showGroup(name)` adds categories:
  Model Group, Render Buffer, Bounds, Appearance, Members.
  Reads `ModelGroup::ModelNames/GetLayout/GetCentreX/Y/Min/Max/`
  `GetDefaultBufferWi/Ht/GetDefaultCamera/GetTagColour`.
- `LayoutPropertyTree::showController(name)` adds categories:
  controller-type header, Connection, Output Range, Capabilities
  (when `ControllerCaps` available), Outputs (per-output universe).
  Reads `Controller::GetName/Type/Description/Vendor/Model/`
  `Variant/Active/IsAutoLayout/Size/Upload/GetProtocol/`
  `GetStartChannel/EndChannel/Channels/GetUniverseString/`
  `GetOutputs`. ControllerEthernet adds IP / FPP Proxy / Priority;
  ControllerSerial adds Port / Speed.

#### Phase 19b — Controller Connection sub-tree ✓
Mirrors `ModelPropertyAdapter::AddControllerProperties`:
- Port + Protocol always shown.
- Smart Remote block (pixel proto + `caps->GetSmartRemoteCount() > 0`):
  Use Smart Remote / Smart Remote Type / Smart Remote (A/B/…) /
  Max Cascade Remotes / Cascade On Port (cascade rows only when
  `GetNumPhysicalStrings() > 1`).
- Serial proto: DMX channel + protocol speed.
- Pixel proto: "Set X / X" parent-child rows for each property,
  gated by `ControllerCaps` (`SupportsPixelPortNullPixels`,
  `…EndNullPixels`, `…Brightness`, `…Gamma`, `…ColourOrder`,
  `…Direction`, `…Grouping`, `SupportsPixelZigZag`, `SupportsTs`).
  Inactive children are rendered dim so the user can see at a
  glance which fields are overridden vs. inherited.

#### Phase 19c — Per-model-type properties (partial ✓)
`LayoutPropertyTree::populateModelTypeProperties(Model* m)`
dispatches on `m->GetDisplayAs()`.  Coverage today (with verified
public accessors):
- **Matrix** — Direction, # Strings, Nodes/Lights per String,
  Strands/String, Alternate Nodes, Don't Zig Zag, Starting
  Location.
- **Arches** — Layered flag, # Arches, Nodes/Arch, Layer Count,
  Hollow %, Zig-Zag, Lights/Node, Arc Degrees, Gap, Starting
  Location.
- **Tree** — Tree Type, Degrees, Rotation, Spiral Rotations,
  Bottom/Top Ratio, Perspective, First Strand.
- **Star** — Star Ratio, Inner %, Start Location, Layers.
- **Cube** — Strands, Nodes/Strand.
- **CandyCanes** — # Canes, Lights per Node.
- **PolyLine** — # Strings, Drop Points, Segments, Lights/Node,
  Model Height, Alternate Nodes.
- **WindowFrame** — Rotation.
- **Spinner** — # Strings, Arms/String, Nodes/Arm, Hollow %,
  Arc Angle, Start Angle, Zig-Zag, Alternate Nodes.

Less-common types (Custom, Circle, Icicles, Wreath,
ChannelBlock, SingleLine, MultiPoint, Sphere, all DMX*) currently
show a category header + `# Strings` + "(detailed properties
pending)" marker — they need either core-side getters or a fall-
back that walks the model's XML attributes.  Tracked for a later
pass; not blocking.  3D objects (Mesh / Image / Terrain / Ruler /
Gridlines) not yet handled.

#### Phase 19d — Popup dialogs ✓ (partial)
Sub-Models / Faces / States are already covered by the existing
`ModelEditDialog` (Phase 12). Reuse it — do **not** split into
three separate dialogs. The property-tree rows for "Sub-Models",
"Faces", "States" all open `ModelEditDialog` on the appropriate
tab (`ModelEditDialog::openForModel(name, _data, tab)`).

New dialogs still to implement (no existing Qt equivalent):
`ModelDimmingCurveDialog`, `EditAliasesDialog`,
`StrandNodeNamesDialog`, `StartChannelDialog`,
`ModelChainDialog`. Triggered from a "…" button in the value
cell (handled by the editing delegate).

#### Phase 19e — Editing wiring ✓ (initial pass)

`LayoutPropertyDelegate` (`QStyledItemDelegate`) produces per-row
editors keyed off a `LayoutPropertyTree::Kind` enum stored on the
value cell's `Qt::UserRole+10`:
- `String` → `QLineEdit`
- `Int` → `QSpinBox` (0..100000)
- `IntPercent` → `QSpinBox` 0..100 with " %" suffix
- `Double` → `QDoubleSpinBox`
- `Bool` → `QComboBox` ("yes" / "no")
- `Color` → `QColorDialog` (opens immediately)

Editable fields (model only — group/controller editing pending):
Description, Active, Pixel Size, Transparency,
Black Transparency, Tag Color, # Strings (Matrix → SetNumMatrixStrings,
Arches → SetNumArches, Spinner → SetNumSpinnerStrings).

Commit path: `itemChanged` → `commitModelField(fieldId, value)` →
`Model::Set*()` → emits `modelChanged(name)` → `LayoutWindow`
refreshes the canvas and re-shows the model props.

**Persistence to `xlights_rgbeffects.xml` is NOT wired yet** —
edits live in-memory in the `ModelManager` only and are lost on
app exit. The phase 19f / 19g work below covers that.

#### Phase 19f — Persistence ✓

`QtRenderBridge::saveModelToShowFile(modelName)` writes the
mutated model back to `xlights_rgbeffects.xml`:

1. Load the existing file via pugixml.
2. Walk to `<xrgb>/<models>/<model name="X">` (or
   `<modelGroups>/<modelGroup name="X">` for groups).
3. Build a fresh `pugi::xml_document` for the model using
   `XmlSerializer::SerializeModel(Model*)`.
4. Replace the existing node (`insert_copy_before` + `remove_child`)
   so the rest of the file is preserved verbatim — other models,
   controllers, layout groups, perspectives, view objects, etc.
   are untouched.
5. Write the file back with `doc.save_file(..., "  ")`.

Wired in `LayoutWindow`: the property-tree's `modelChanged`
signal calls `_bridge->saveModelToShowFile(name)` before
refreshing the canvas and re-showing the props. Save is
synchronous; if it becomes sluggish on slow disks we can debounce
on a QTimer like `QtRenderBridge::request()` does for renders.

Diverges from the wx path (which builds the WHOLE rgbeffects XML
from scratch via `BuildEffectsXml()` and rewrites the file) —
Qt's per-model surgical edit is safer since the Qt UI doesn't yet
manage perspectives / color manager / viewpoints / sequence views,
and we'd rather not clobber what wx wrote.

#### Phase 19g — Editable fields for group + controller ✓

Added a new `Kind::Enum` to `LayoutPropertyTree` (QComboBox with
options stored in `Qt::UserRole+12`) so multi-value enums like
controller Active state work.

The itemChanged dispatcher now branches on `_currentKind`
(Model / Group / Controller) and routes to one of:
- `commitModelField` (existing)
- `commitGroupField` → `ModelGroup::SetActive/SetLayoutGroup/`
  `SetLayout/SetDefaultCamera/SetTagColour`.
- `commitControllerField` → `Controller::SetDescription/SetActive/`
  `SetAutoLayout/SetAutoUpload/SetVendor/SetModel/SetVariant`,
  `ControllerEthernet::SetIP/SetFPPProxy/SetPriority/SetProtocol`,
  `ControllerSerial::SetPort/SetSpeed`.

Editable group fields: Layout (combo: Default / Minimal Grid /
Horizontal Stack / Vertical Stack / Overlay-Center / Overlay-
Scaled / Per Model Default), Layout Group, Active, Tag Color,
Default Camera.

Editable controller fields: Description, Vendor, Model, Variant,
Active (enum), Auto Layout, Auto Upload; Ethernet adds IP / FPP
Proxy / Priority; Serial adds Port / Speed.

Persistence: groups re-use `QtRenderBridge::saveModelToShowFile`
(it already branches on `dynamic_cast<ModelGroup*>` to write
`<modelGroup>` nodes). Controllers go via new
`QtRenderBridge::saveControllersToShowFile()` which calls
`OutputManager::Save()` to rewrite `xlights_networks.xml`.

#### Phase 19h — Editable fields: remaining per-type
Currently editable: Matrix / Arches / Spinner `# Strings`. Add
setters for the other types as their core-side `Set*` accessors
become available (Circle, Wreath, ChannelBlock, Tree, PolyLine,
CandyCanes, all DMX*, etc.).

Undo: integrate with whatever undo stack the Qt UI ends up
adopting (currently none — open question).

### Phase 20 — Add / delete entities (models, groups, controllers) ✓ (20a–20e + 20g)

**Goal:** let the user create new models, model groups, and
controllers from inside the Qt Layout window, and delete existing
ones. Today the Qt UI only allows editing pre-existing entities
that were created in the wx UI.

**Why now:** the editable property tree (Phase 19) is meaningless
in an empty show — the user has no way to put anything in the
show without going back to the wx app. Add/delete closes that
loop.

#### Phase 20a — UI affordances

Each of the three left-side tabs (Models, Groups, Controllers)
gets a small toolbar above its `QListWidget` with **+** and
**−** buttons:

```
[Models | Groups | Controllers]
[+] [−]                                   ← new toolbar
   ...list items...
```

- `+` → opens the matching "New …" dialog (below).
- `−` → confirms then deletes the selected item.

Keyboard: `Insert` triggers `+`, `Delete` triggers `−` (when the
list has focus).

Right-click context menu on the list duplicates the same two
actions plus "Duplicate…" (creates a copy with " (copy)" suffix
on the name).

#### Phase 20b — Add Model

A new `AddModelDialog` (Qt analog of the wx model-type chooser):

- **Type** combo populated from `DisplayAsType` (filter out
  `ModelGroup`, `SubModel`, `Unknown`, view-object types) — the
  same list `xLightsFrame::CreateDefaultModelXxx` covers, plus
  DMX subtypes.
- **Name** line edit, pre-filled with `<Type>_001` (incremented
  to the first unused name).
- **Layout group** combo (defaults to currently-selected layout
  group on the canvas; falls back to "Default").
- **Start channel** — defaults to "next free" (`OutputManager`
  can compute), editable.
- **OK** invokes
  `ModelManager::CreateDefaultModel(typeStr, startChannel)` →
  `ModelManager::AddModel(Model*)` → set Name + LayoutGroup +
  WorldPos(canvas centre) → `saveModelToShowFile(name)` →
  refresh.

For *interactive* placement (wx supports click-to-drop in the
canvas), defer to Phase 20f.

#### Phase 20c — Add Group

A new `AddGroupDialog`:

- **Name** line edit, pre-filled with `Group_001`.
- **Layout** combo with the seven layout options (same list the
  property tree already uses).
- **Layout group** combo.
- **Members** dual-list selector (available models on the left,
  group members on the right, add/remove buttons in the middle).

OK → construct `ModelGroup(modelManager)` → set its name /
layout / members via the existing `ModelGroup::Set*` methods →
`modelManager.AddModel(group)` (groups go through the same path
as models in `ModelManager`) → `saveModelToShowFile(name)` (the
existing path already routes `<modelGroup>` via dynamic_cast) →
refresh.

#### Phase 20d — Add Controller

A new `AddControllerDialog`:

- **Type** combo: Ethernet / Serial / Null.
- **Name** line edit, pre-filled with `<Type>_001`.
- **Vendor / Model / Variant** cascading combos populated from
  `ControllerCaps::GetVendors(type)` →
  `GetModels(type, vendor)` → `GetVariants(...)`. Same
  population pattern the property tree already uses.
- **For Ethernet**: Protocol dropdown (same logic as the
  property tree — from `caps->GetInputProtocols()` or default
  list), IP line edit (placeholder text shows the current
  subnet's broadcast).
- **For Serial**: Port dropdown
  (`SerialOutput::GetPossibleSerialPorts()`), Speed dropdown
  (`SerialOutput::GetPossibleBaudRates()`).

OK constructs the right subclass and inserts it via
`OutputManager::AddController(c)`:

```cpp
switch (type) {
    case Ethernet: c = new ControllerEthernet(om, /*acceptDup*/false); break;
    case Serial:   c = new ControllerSerial(om);                       break;
    case Null:     c = new ControllerNull(om);                         break;
}
c->SetName(name); c->SetVendor(...); ...
om->AddController(c, /*pos=*/-1);
saveControllersToShowFile();
```

#### Phase 20e — Delete

Deletes go through src-core's own remove methods (so dependent
state — auto-channel-renumber, group-member cleanup, controller-
output free, etc. — stays consistent):

- Model: `ModelManager::Delete(name)` → `saveModelToShowFile`
  isn't enough (we need to remove the node, not replace it); add
  a small `QtRenderBridge::removeModelFromShowFile(name)` that
  finds `<model name="X">` (or `<modelGroup>`) and erases it.
  Plus iterate any model groups that contain it and rewrite them
  too — `ModelGroup::Delete` will have done the in-memory update
  already.
- Group: `ModelManager::Delete(name)` + the same remove path; no
  cascading cleanup needed (groups don't contain models, they
  reference them by name).
- Controller: `OutputManager::DeleteController(name)` →
  `saveControllersToShowFile()`. Models referencing the deleted
  controller by name become orphaned (Controller="" effectively)
  — same as wx; flag with a warning dialog.

Delete confirmation: standard `QMessageBox::question` with the
entity name, defaulting to No.

#### Phase 20f — Interactive placement (deferred)

In wx, dropping a new model places it at the click point in the
layout canvas. The Qt `ModelLayoutCanvas` supports
`modelClicked` but not "drop a new model here" yet. After
Phase 20b–20e land we can wire a mode where the Add Model dialog
sets a pending type, then the next canvas click places the model
there (rather than at canvas centre).

Defer until 20a–20e prove out.

#### Phase 20g — Persistence (already mostly free)

Existing paths cover the new flow:
- `saveModelToShowFile` already creates the `<model>` node if it
  doesn't exist (the `if (target)` / `else` branch). New models /
  groups Just Work.
- `saveControllersToShowFile` calls `OutputManager::Save()` which
  re-serialises everything. New controllers Just Work.

Only new code: `removeModelFromShowFile(name)` for deletes (see
20e).

## Out of Scope

xSchedule (separate app), full xlGraphicsContext parity (OpenGL shader library).
