# xLights Qt — MVP Implementation Plan

## Locked Decisions

| Question | Answer |
|---|---|
| Output directory | `bin-qt/` (separate from `bin/`) |
| Effects panel | One generic panel, controls generated from `effectmetadata/*.json` |
| Palette | PaletteWidget (8 swatches) at top of every effect panel; color = QColorDialog |
| Layer blend modes | Standard xLights set in BufferWidget combo: Normal, Add, Subtract, Max, Layered, etc. |
| Buffer widget | Shows raw 2D render buffer (effect output before model mapping); QPainter grid |
| Buffer style | Combo in BufferWidget: Default, Per Model Strand, Single Line, etc. |
| Model preview | QtEffectRenderer (software, Phase 6a done); real src-core render in Phase 7 |
| Graphics | Qt RHI → D3D11 (Win), Metal (macOS), Vulkan/OpenGL (Linux) |
| Shader scope | MVP subset: solid rectangles, text glyphs, textured quads only |
| xSchedule | Out of scope |
| Sequencer editing | View + select + move + drag-to-resize (with ghost preview) |
| Live preview | 40ms debounce on slider/palette change → QtRenderBridge → QtEffectRenderer |
| Scope | MVP only |

## Architecture

```
src-core/             ← unchanged, shared with wx + iPad
src-ui-wx/            ← unchanged, wx build unaffected
src-ui-qt/
  App/                QApplication, main window, menu bar
                      PlaybackController, TransportToolBar
  Bridge/             C++ wrappers over src-core (no wx types)
                      QtSequenceDoc, QtEffectRenderer, QtRenderBridge
  Sequencer/          Timeline grid, row headers, timing ruler, playhead
  Effects/            EffectToolBar (icon row), PaletteWidget, BufferWidget
                      EffectPanelWidget, EffectControlBuilder
                      controls/ (SliderSpinRow, CheckRow, ComboRow, etc.)
  Preview/            PreviewWidget — 1D strip or 2D grid of rendered pixels
  Layout/             Model layout canvas (Phase 8, read-only)
  Graphics/           RhiCanvas (QRhiWidget base)
CMakeLists.txt        xLights-Qt target, bin-qt/ output
```

## Completed Phases

### Phase 1 — Scaffold ✓
QRhiWidget builds on Win/macOS/Linux, bin-qt/, D3D11/Metal/Vulkan.

### Phase 2 — Core bridge ✓
`QtSequenceDoc`: pugixml .xsq parser. Fix: sequenceDuration stored in seconds (×1000).
Fallback: compute totalFrames from max block end time when sequenceDuration absent.
`QtXLightsApp`: loads effectmetadata/*.json, owns effect list + metadata map.
`SequencerModel::loadFromSequence()`: converts ms→frames.

### Phase 3 — Generic effects panel ✓
`EffectControlBuilder`: JSON → QWidget for slider/checkbox/choice/text/filepicker/spin/custom.
`EffectPanelWidget`: palette row + buffer view (collapsible splitter) + effect controls scroll.
Emits `settingsChanged` and `paletteChanged` after 40ms debounce. Fires initial render on showEffect().
`PaletteWidget`: 8 ColorSwatchButton swatches, default xLights palette, QColorDialog on click.
`BufferWidget`: 2D pixel grid canvas + blend mode combo (Normal/Add/Subtract/…) + buffer style combo.
`ColorSwatchButton`: checkerboard alpha preview, click → QColorDialog.

### Phase 4 — Sequencer grid ✓
`SequencerGridCanvas`: effect blocks + ghost drag, resize handles, red playhead line.
`SequencerController`: Body/ResizeLeft/ResizeRight zones, move+resize, snap.
`RowHeaderWidget`, `TimingRulerCanvas`: headers + ruler + playhead indicator.
`SequencerWidget`: QGridLayout, synced scrollbars, `setPlayhead(frame)` forwarding.
`SequencerModel`: playhead frame, `setPlayhead()` emits geometryChanged.

### Phase 5 — Transport + Effects toolbar ✓
`PlaybackController`: real-time playback via QElapsedTimer, 40ms tick.
`TransportToolBar`: play/pause/stop + scrub slider + time display (m:ss.mmm).
`EffectToolBar`: scrollable icon row across top; colored icons generated per effect name.
`PreviewWidget`: 1D strip (strand) or 2D grid (matrix) of rendered pixels below sequencer.
Mock colors used as placeholder before first render bridge result arrives.

### Phase 6a — Software effect renderer ✓
`QtEffectRenderer`: software renderer covering 12+ effects using real palette colors:
- **On, Off** — solid fill or black
- **Bars** — respects BarCount, Cycles, Direction settings
- **Rainbow** — hue gradient cycling over time
- **Colorloop** — full-model hue cycle
- **Fire** — bottom-hot gradient with sine turbulence; uses palette if provided
- **Chase / Meteor** — moving head with fading trail, respects Cycles
- **Sparkle / Twinkle** — random pixel illumination seeded by playback position
- **Spiral** — angle+distance palette mapping
- **Snowflakes** — random falling dots
- **Generic fallback** — scrolling palette gradient for any unrecognised effect

`QtRenderBridge`: 40ms debounce → `QtEffectRenderer::render()` → `frameReady` signal.
Progress value computed from playhead position within clicked block (not always 0.5).
Result routes to: BufferWidget (2D grid) + PreviewWidget (1D or 2D depending on bufH).

## Remaining Work

### Phase 6b — Model topology mapping
Parse model pixel node positions from xsq `<models>` section in QtSequenceDoc.
Store model dimensions (W×H for matrices, N for strands) in QtSequenceInfo.
PreviewWidget: when model has 2D topology, draw nodes at real pixel coordinates
(e.g. star shape, spiral tree) rather than a rectangular grid approximation.
`QtEffectRenderer::Request`: expose `bufferW/H` from model dimensions, not hardcoded 100×1.

### Phase 6c — Real src-core rendering (replaces software renderer)
Link src-core to xLights-Qt target in CMakeLists.txt (same SRC_CORE glob).
`QtRenderContext` (`src-ui-qt/Bridge/`): subclass of RenderContext, mirrors iPadRenderContext.
`QtRenderBridge` extended: runs RenderableEffect on QThreadPool worker instead of software renderer.
Render flow: Request → QThreadPool → RenderableEffect::Render(buffer) → frameReady.

### Phase 6d — Multi-layer sequencer rows (Phase 4 extension)
`SequencerRow` → `QList<SequencerLayer>` (each layer has blocks + blend mode + collapsed flag).
Grid canvas draws collapsible sub-rows per layer with blend mode label.
QtSequenceDoc parses EffectLayer children as separate layers (currently merged to one).

### Phase 7 — FSEQ export
Render all frames to RenderBuffer; write v2 FSEQ binary (row: channel data, little-endian).
ExportDialog: progress bar, output path picker, frame range.

### Phase 8 — Layout view (read-only)
`LayoutViewCanvas` (RhiCanvas): parse `<modelGroup>` positions from xsq, draw wireframes.

## Phase Order

```
Phase 1  Scaffold                            ✓ done
Phase 2  Core bridge / .xsq parser          ✓ done
Phase 3  Generic effects panel + palette     ✓ done
Phase 4  Sequencer grid + playhead           ✓ done
Phase 5  Transport + icon toolbar            ✓ done
Phase 6a Software effect renderer            ✓ done
Phase 6b Model topology (node positions)     ← next
Phase 6c Real src-core rendering             ← after 6b
Phase 6d Multi-layer sequencer rows          ← parallel with 6b/6c
Phase 7  FSEQ export
Phase 8  Layout view
Phase 9  Effect drag-drop onto timeline
Phase 10 Timing track editing
Phase 11 Model editing
Phase 12 Face / State / Sub-model editing
Phase 13 Controller upload
Phase 14 Import sequence dialog
Phase 15 Batch render
Phase 16 Controller visualizer
Phase 17 Model & Controller info viewer
```

## Full File Inventory

```
src-ui-qt/
  App/
    main.cpp ✓
    MainWindow.{h,cpp} ✓
    QtXLightsApp.{h,cpp} ✓
    PlaybackController.{h,cpp} ✓
    TransportToolBar.{h,cpp} ✓
  Bridge/
    QtSequenceDoc.{h,cpp} ✓
    QtEffectRenderer.{h,cpp} ✓
    QtRenderBridge.{h,cpp} ✓
    QtRenderContext.{h,cpp}     ← Phase 6c
  Graphics/
    RhiCanvas.{h,cpp} ✓
  Sequencer/
    SequencerWidget.{h,cpp} ✓
    SequencerGridCanvas.{h,cpp} ✓
    RowHeaderWidget.{h,cpp} ✓
    TimingRulerCanvas.{h,cpp} ✓
    SequencerController.{h,cpp} ✓
    SequencerModel.{h,cpp} ✓
  Effects/
    EffectPanelWidget.{h,cpp} ✓
    EffectControlBuilder.{h,cpp} ✓
    EffectToolBar.{h,cpp} ✓
    PaletteWidget.{h,cpp} ✓
    BufferWidget.{h,cpp} ✓
    controls/
      SliderSpinRow.{h,cpp} ✓
      CheckRow.{h,cpp} ✓
      ComboRow.{h,cpp} ✓
      TextRow.{h,cpp} ✓
      FilePickerRow.{h,cpp} ✓
      ColorSwatchButton.{h,cpp} ✓
  Preview/
    PreviewWidget.{h,cpp} ✓
  Layout/
    LayoutViewCanvas.{h,cpp}    ← Phase 8
```

## Post-MVP Phases

### Phase 9 — Effect drag-drop onto timeline
Drag an effect icon from EffectToolBar onto a sequencer row to create a new block.
- `SequencerGridCanvas`: accept drop events, show insertion ghost while dragging over row
- On drop: create `EffectBlock` at nearest frame boundary, emit `modelChanged`
- Drag source: `EffectToolBar` buttons set drag data (effect name)

### Phase 10 — Timing tracks
Dedicated timing row type in SequencerModel (separate from model rows).
- `TimingTrackRow`: list of timing marks (frame positions + label)
- `TimingRulerCanvas` extended: click to add marks, drag to move
- Import/export `.xtiming` format (already parsed by QtSequenceDoc — just needs UI)
- Snap-to-timing-mark in SequencerController (currently snaps to frame boundary only)

### Phase 11 — Model editing
Edit model definitions inline without leaving the Qt app.
- `ModelEditorDialog`: name, type (strand/matrix/tree/star/etc.), dimensions, starting channel
- Write changes back to the xsq `<models>` section via QtSequenceDoc
- `LayoutViewCanvas` (Phase 8) becomes interactive: drag models to reposition
- Sub-model editor: define sub-ranges within a model (e.g. "top half of matrix")

### Phase 12 — Face / State / Sub-model editing
- **Faces**: map model nodes to phoneme regions (mouth shapes for singing faces)
  `FaceEditorDialog`: node picker + phoneme assignment table
- **States**: define named lighting states (e.g. "all on", "chaser running")
  `StateEditorDialog`: list of states, each with per-node color overrides
- **Sub-models**: carve a parent model into named sub-ranges
  `SubModelEditorDialog`: contiguous or non-contiguous node range picker
- All three stored as child XML under `<model>` in the xsq; parsed + written by QtSequenceDoc

### Phase 13 — Controller upload
Upload FSEQ + show file to a Falcon Player or other controller.
- `ControllerUploadDialog`: IP/hostname, progress bar, upload status
- Uses existing `src-core/controllers/FPP.cpp` logic (requires Phase 6c src-core link)
- Also supports direct E1.31/ArtNet test output via `src-core/outputs/`
- xSchedule integration: out of scope (separate app)

### Phase 14 — Import sequence dialog
Import effects from another sequence file into the current sequence.
- `ImportSequenceDialog`: file picker + model mapping table
- Source formats:
  - `.xsq` — import effects from another xLights sequence (model-to-model mapping)
  - `.lsp` / `.msq` — Light-O-Rama / Vixen / HLS import via `src-core/import_export/`
  - `.gled` — GrandMA, etc.
- Model mapping table: left column = source model names, right column = target model dropdown
- Effect import: copies EffectBlocks into the target SequencerModel rows, respecting frame offsets
- Timing import: optionally pull timing marks from the source sequence
- Uses existing `src-core/import_export/MapXLightsEffects` and `LoadMapHintsFile` logic
  (these are already compiled into the cmake build via src-core/import_export/*.cpp)

### Phase 16 — Batch render
Render multiple sequences to FSEQ in one operation.
- `BatchRenderDialog`: sequence file list, output folder, progress per file
- Runs Phase 7 FSEQ export logic per sequence on a QThreadPool worker

### Phase 16 — Controller visualizer
Visual representation of controller port assignments — shows which models are wired
to which ports and validates channel counts before upload.
- `ControllerVisualizerWidget`: grid of port rows (port number | pixel count | model name | channel range)
- Color-coded by model: each model gets a consistent hue, matching its sequencer row color
- Overflow indicator: port exceeds pixel/channel limit shown in red
- Live: updates as models are added/moved in the layout editor (Phase 11)
- Supports common controller families: Falcon (F4/F16/F48), FPP, WLED, ESPixelStick
  — port count and pixel limits loaded from `src-core/controllers/` vendor data
- Dock as a panel alongside the layout view or as a separate dialog

### Phase 17 — Model & Controller Info Viewer
Read-only panel showing all models and controllers loaded from the show folder.
Data sources: `xlights_rgbeffects.xml` (models) + `xlights_networks.xml` (controllers),
both already parsed at sequence open time.

**Models tab**
- Table: Name | Type (DisplayAs) | Start Channel | Nodes | Buffer W×H | Controller
- Sortable by any column; filter box for quick search by model name
- Selecting a row highlights the corresponding sequencer row and fires a preview render
  with the model's current dimensions (same as clicking a block)
- Double-click → quick view of node positions in PreviewWidget (empty/Off effect)

**Controllers tab**
- Table: Name | Type (Ethernet/Serial) | IP / Port | Universe / Start | Channel Count
- Expand arrow per row → sub-table of ports: Port# | Protocol | Nodes | Model(s) assigned
- Port overflow shown in amber/red (node count exceeds controller cap)
- Source data from `OutputManager::GetControllers()` — available immediately after
  `OutputManager::Load(showDir)` is called in `loadModels`

**Implementation notes**
- Two **separate non-modal windows** — both can be open simultaneously alongside the
  main window. `QWidget` with `Qt::Window` flag (not QDialog) so they don't block input.
- **`ModelInfoWindow`**: table of models from `QtSequenceInfo::models`
- **`ControllerInfoWindow`**: table of controllers from `QtControllerInfo` list stored
  in `QtSequenceInfo` (populated from `OutputManager::GetControllers()` in `loadModels`)
- `QtControllerInfo` struct added to `QtSequenceDoc.h`; mirrors what `OutputManager`
  exposes without needing to hold a live `OutputManager` reference beyond load time
- Accessible via **View** menu: "Models…" (`Ctrl+M`) and "Controllers…" (`Ctrl+K`)
- Windows are created once, hidden by default, shown/raised on menu action
- Refresh automatically when a new sequence is opened (connect to `QtXLightsApp` signal)
- No editing — mutations belong to Phase 11 (Model editing) and Phase 13 (Upload)
- New files: `src-ui-qt/Info/ModelInfoWindow.{h,cpp}`,
             `src-ui-qt/Info/ControllerInfoWindow.{h,cpp}`

## Out of Scope

xSchedule (separate app).
