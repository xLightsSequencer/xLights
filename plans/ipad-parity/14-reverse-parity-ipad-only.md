# 14 · Reverse Parity — iPad features NOT in Desktop

> This theme covers iPad affordances the desktop lacks. Most of the
> "reverse parity" surface is already at parity — the desktop ships timing-mark
> ops, AI lyrics/services, controller-visualizer editing, Export-Models (.xlsx),
> value-curve presets, FPP progress, recent menus, and the 3D ModelPreview; the
> desktop also pinch-zooms / pinch-scales / two-finger-rotates via
> `wxEVT_GESTURE_*` (parity/desktop-weaker). The genuine desktop work is led by a
> **P1**: *adjusting transition (fade) times by dragging on the effect grid*
> (desktop has no grid fade-drag at all). Other feasible pull-backs: **edge auto-
> scroll while dragging**, **snap-to-grid for layout models**, a **persistent
> shaded loop region**, **actionable Check-Sequence navigation**, **`.xsqz` in-
> place round-trip**, **Remove-Unused-Media**, **selected-row-grow**, a
> **draggable XY center pad**, and inline Sketch/Morph editors. The durable
> *infeasible* residue is genuine touch/pencil/camera/iCloud idioms (two-finger
> marquee, pencil-undo, LiDAR scan, cloud docs).

## Parity scorecard

| Feature | Surface | Desktop | iPad | Gap | Priority | Ease | Feasibility | Notes |
|---|---|---|---|---|---|---|---|---|
| **Adjust transition (fade in/out) times on the grid** | gesture | ❌ | ✅ | desktop-missing | **P1** | medium | feasible | iPad drags fade handles directly on the effect (`EffectsMetalGridView.swift:177` `Kind.fadeIn/fadeOut`, `liveFadeInSec/OutSec` :200-201). Desktop has **no** fade-drag on the grid (`EffectsGrid.cpp` has no fade-drag); transition in/out times are set only via the Blending/Layer panel. High-value desktop pull-back. |
| **Selected grid row grows (expands height)** | gesture | ❌ | ✅ | desktop-missing | P2 | medium | feasible | iPad enlarges the selected row (`EffectsMetalGridView.swift` `selectedRowHeight` vs `rowHeight`, ~10 draw sites; `GridMetrics.swift`). Desktop grid rows are fixed height. Touch-driven but a genuine readability win desktop could adopt. |
| **`.xsqz` package read *and write* (in-place round-trip)** | other | 🟡 | ✅ | desktop-weaker | P2 | medium | feasible | iPad opens an `.xsqz` and **repacks it back to the original on Save** (`SequencerViewModel.swift:32-37` copy-repacked-package-back; `XLSequenceDocument.h:75-97`). Desktop opens `.xsqz` by *extracting* to a temp (`xLightsApp.cpp:840` `readOnlyZipFile`) and its "Package Sequence" (`SequencePackage::Pack`, `xLightsMain.cpp:6161`) is a separate **export**, not an in-place round-trip. |
| **"From base" link badge across all layout entities** | other | 🟡 | ✅ | desktop-weaker | P3 | easy | feasible | iPad shows a link/base indicator on models **and** groups/objects/controllers from `isFromBase` (`LayoutEditorView.swift:1026/1057/1120/1810/1824`). Desktop conveys "from base" via row styling, not a dedicated badge across all four entity types. |
| Map-from-Lights structured-light scan → Custom model | dialog | 🟡 | ✅ | parity | P2 | hard | infeasible | Mac has it via **Continuity Camera** (`KLightMapperBridge` + `xLightsMain.cpp:4296`); iPad uses on-device camera **+ LiDAR depth solve**. Mac path = no LiDAR, iPhone-paired only. The LiDAR depth multi-view solve is iPad-only HW. |
| Two-finger marquee select (layout canvas) | gesture | ❌ | ✅ | desktop-missing | P3 | hard | infeasible | `PreviewPaneView.swift:148` two-finger long-press+drag. Desktop = single-pointer left-drag rubber-band (different idiom; already exists for its input model). |
| Two-finger marquee select (effects grid) | gesture | 🟡 | ✅ | parity | P3 | hard | infeasible | iPad `EffectsMetalGridView.swift:58` two-finger marquee (1-finger = scroll). Desktop has equivalent left-drag rubber-band selection. Touch idiom. |
| Apple Pencil Pro double-tap / squeeze → undo | gesture | ❌ | ✅ | desktop-missing | P3 | hard | infeasible | `EffectsMetalGridView.swift:62` `onPencilTapAction`. No stylus layer on wx desktop. |
| Pinch-to-zoom preview | gesture | ✅ | ✅ | parity | P3 | — | — | Desktop pinch-zooms via `ModelPreview::OnZoomGesture` bound to `wxEVT_GESTURE_ZOOM` (`ModelPreview.cpp:134`, bind `:1099/:1126`, `EnableTouchEvents(wxTOUCH_ZOOM_GESTURE)`) on all previews, plus `wxEVT_MAGNIFY` trackpad path. Parity (trackpad/touch). |
| Two-finger pan preview | gesture | ❌ | ✅ | desktop-missing | P3 | hard | infeasible | `PreviewPaneView.swift` centroid pan. Single-pointer desktop. |
| Two-finger rotate (camera roll) | gesture | 🟡 | ✅ | desktop-weaker | P3 | hard | infeasible | Desktop binds a two-finger rotate gesture (`wxEVT_GESTURE_ROTATE` → `LayoutPanel::OnPreviewRotateGesture` `:5282`, bind `:716`) — but it rotates the **selected model** (axis: Shift=X/Ctrl=Y/default-Z), not the camera. Only the *camera-roll* target is iPad-specific (`PreviewPaneView.swift:133`). |
| Double-tap-to-reset camera | gesture | 🟡 | ✅ | parity | P3 | easy | feasible | iPad double-tap; desktop resets via viewpoint menu / keyboard. Minor idiom diff. |
| iCloud Documents auto-sync + status | other | ❌ | ✅ | desktop-missing | P3 | hard | infeasible | iPad is a document-based app on iCloud Drive (`NSFileCoordinator`). Desktop is a classic local-FS app; no cloud doc model. |
| Pinch-to-scale selected layout model | gesture | ✅ | ✅ | parity | P3 | — | — | Desktop pinch scales the selected model: `LayoutPanel::OnPreviewZoomGesture` (`:5326`) calls `selectedBaseObject->Scale(...)` (`:5344`) when a model is selected (else zooms camera). Same gesture idiom as iPad (`PreviewPaneView.swift:366`). Parity. |
| Keyboard menu-bar (iPadOS hardware-keyboard menus) | menu | ✅ | ✅ | parity | P3 | easy | feasible | `XLightsCommands.swift` SwiftUI `Commands`. Desktop has a native wx menubar. Both fine; nothing to port. |
| In-app real-time log tail | dialog | 🟡 | ✅ | desktop-missing | P3 | medium | feasible | Desktop "View Log" (`xLightsMain.cpp:5092`) hands the log file to an **external** text editor; iPad `LogViewerSheet.swift` tails in-app (no external editor on sandboxed iPad). Desktop could add a `wxLogWindow`. |
| Interactive media-relocation assistant | dialog | 🟡 | ✅ | desktop-missing | P3 | medium | feasible | Desktop **auto**-relocates missing media on load + notifies (`xLightsMain.cpp:5570`); iPad `MediaRelocation.swift` is an interactive pick-the-file assistant. Polish gap only. |
| Find / Replace — case-sensitive toggle | dialog | ❌ | ✅ | desktop-missing | P3 | easy | feasible | Desktop `EffectsGrid::Find` always lowercases (`EffectsGrid.cpp:705`, case-insensitive only). iPad `FindReplaceSheet.swift` exposes a toggle. Both operate on timing-mark labels. |
| Layout model 8-mode sort picker | menu | 🟡 | ✅ | desktop-missing | P3 | easy | feasible | Desktop tree has column-sort (`wxCOL_SORTABLE`, persisted). iPad `LayoutEditorView.swift:57` adds an explicit 8-mode picker (name/start-ch/end-ch/type asc+desc + natural). Polish only. |
| Batch-render explicit sort-order picker | menu | 🟡 | ✅ | desktop-missing | P3 | easy | feasible | Desktop sorts via clickable tree columns (`BatchRenderDialog.cpp:138`, persisted). iPad adds a dedicated picker. Polish. |
| Recent sequences / recent folders quick-access | panel | ✅ | ✅ | parity | P3 | easy | feasible | Desktop File-menu submenus (`RecentSequencesMenu`, `RecentShowFoldersMenu`, `xLightsMain.cpp:1027/1055`). iPad sidebar panels. UX diff only. |
| Layout Controllers tab in editor sidebar | tab | ✅ | 🟡 | ipad-missing | P3 | medium | restricted | iPad **omits** the Controllers tab (`LayoutEditorView.swift:29`, J-5) for closed-firmware reasons; desktop always shows it. Reverse of the usual gap — desktop is ahead here. |
| MapFromLights — known-controller lookup / auto-name | dialog | 🟡 | ✅ | parity | P3 | hard | infeasible | iPad maps scanned FPP host → xLights controller name. Mac scan flow saves to a Save-As dialog. Same core, different surface. |
| Expand / Collapse All elements | menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `ID_ROW_MNU_COLLAPSEALLMODELS/LAYERS` (`RowHeading.cpp:536`). iPad `SequencerGridV2View.swift:1336` + bridge `collapseAllElements`/`expandAllElements`. |
| Header collapse/expand chevron toggle | toolbar | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `HitTestCollapseExpand` (`RowHeading.cpp:2246`) toggles via the triangle. iPad chevron `RowHeaderViews.swift:115`. |
| Timing: Breakdown Phrases / Words | context-menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `ID_ROW_MNU_BREAKDOWN_TIMING_PHRASES/WORDS`. iPad `RowHeaderViews.swift:163`. Shared `lyrics/LyricBreakdown`. |
| Timing: Make Variable (unfix fixed timing) | context-menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `ID_ROW_MNU_UNFIX_TIMING_TRACK` → `te->Unfix()` (`RowHeading.cpp:625`). iPad `RowHeaderViews.swift:197`. |
| Timing: Generate Subdivided tracks | context-menu | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `ID_ROW_MNU_GENERATE_SUBDIVIDED_TRACKS` (`RowHeading.cpp:229`). iPad `RowHeaderViews.swift:203`. |
| Timing: Halve / Divide marks (÷2..÷16) | context-menu | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `ID_GRID_MNU_HALVETIMINGS` "Divide Timings" + `MainSequencer::DivideTimingTrack(2..16)`. iPad `RowHeaderViews.swift:243/249`. |
| Timing: Select All Marks | context-menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `ID_ROW_MNU_SELECT_TIMING_EFFECTS` "Select Timing Marks" (`RowHeading.cpp:630`). iPad `RowHeaderViews.swift:261`. |
| Timing: Auto-Label Marks | context-menu | ✅ | ✅ | parity | P1 | medium | feasible | Desktop `ID_GRID_MNU_AUTOLABEL` "Auto Label Timings" + `AutoLabelDialog`. iPad `RowHeaderViews.swift:237`. |
| Timing: Speech to Lyrics (AI transcription) | context-menu | ✅ | ✅ | parity | P1 | hard | feasible | Desktop `ID_ROW_MNU_AI_LYRICS` "AI Speech 2 Lyrics" → `EVT_AI_LYRICS` (`RowHeading.cpp:636`, vocal-isolation pipeline in `tabSequencer.cpp:4573+`). iPad `RowHeaderViews.swift:225`. Same AI service core. |
| Timing: generate from Audio Onsets/Tempo/Chords | context-menu | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `ID_ROW_MNU_GENERATE_FROM_ONSETS/TEMPO/CHORDS`. iPad `AddTimingTrackSheet.swift:30`. |
| Model row: toggle render-disabled | context-menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `ID_ROW_MNU_RENDERENABLE_MODEL` / `RENDERDISABLE_MODEL` (`RowHeading.cpp:196`). iPad `RowHeaderViews.swift:348`. |
| AI palette generation | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `ai/AIColorPaletteDialog.cpp`. iPad `AIPaletteGenerationSheet.swift`. |
| AI image generation | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `ai/AIImageDialog.cpp`. iPad `AIImageGenerationSheet.swift`. |
| AI services configuration | dialog/pref | ✅ | ✅ | parity | P1 | easy | feasible | Desktop **Preferences → Services** (`ServicesPanel`, `xLightsPreferences.cpp:131`). iPad `AIServicesSettingsSheet.swift`. |
| Controller visualizer (wiring view) | panel | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `setup/ControllerModelDialog.cpp` (the canonical visualizer). iPad `ControllerVisualizeView.swift`. |
| Visualizer: set port protocol | context-menu | ✅ | ✅ | parity | P1 | medium | feasible | Desktop `CONTROLLER_PROTOCOL` "Set Protocol" (`ControllerModelDialog.cpp:641`). iPad `ControllerVisualizeView.swift:77`. |
| Visualizer: move models to port | context-menu | ✅ | ✅ | parity | P1 | medium | feasible | Desktop `CONTROLLER_MOVEMODELSTOPORT` (`ControllerModelDialog.cpp:654`). iPad `ControllerVisualizeMoveSheets.swift`. |
| Visualizer: drag-drop wiring | gesture | ✅ | 🟡 | parity | P1 | medium | feasible | Desktop `_dragging` drag-drop in `ControllerModelDialog`. iPad `ControllerVisualizeView.swift:45` `Transferable` (J-32.5, in progress). |
| Visualizer: edit model start-channel / chain | dialog | ✅ | ✅ | parity | P1 | medium | feasible | Desktop `SetModelChain` / `StartChannel` editing in `ControllerModelDialog.cpp`. iPad `ControllerVisualizeMoveSheets.swift` StartChannelTarget. |
| Smart-remote assignment (letter + cascade) | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `CONTROLLER_SETSMARTREMOTE` / type / increment (`ControllerModelDialog.cpp:643`). iPad `ModelSmartRemoteSheet.swift`. |
| FPP Connect — discovery + manual IP | dialog | ✅ | ✅ | parity | P2 | medium | feasible | Desktop `controllers/FPPConnectDialog.cpp`. iPad `FPPConnectSheet.swift`. |
| FPP Connect — per-instance upload progress | dialog | ✅ | ✅ | parity | P2 | medium | feasible | Desktop `FPPUploadProgressDialog` + `wxGauge` per instance (`FPPConnectDialog.cpp:1081`). iPad `FPPConnectSheet.swift`. |
| Batch render — selections + recurse toggle | dialog | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `BatchRenderDialog` checked tree + Filter "Recursive Search / Only Show Directory". iPad `BatchRenderSheet.swift`. |
| Export Models report (.xlsx) | menu | ✅ | ✅ | parity | P2 | medium | feasible | Desktop **File → Export Models** → `ExportModels()` (Models + Controllers sheets, `import_export/ExportEffects.cpp:38`). iPad `LayoutEditorView.swift:4`. |
| Effect presets library | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `EffectTreeDialog`. iPad `PresetBrowserSheet.swift`. |
| Save selection as new preset | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop "New Preset" button (`EffectTreeDialog.cpp:133`, "from current settings"). iPad `PresetBrowserSheet.swift:60`. |
| Preset import / export | menu | ✅ | ✅ | parity | P2 | easy | feasible | Desktop Import/Export buttons (`EffectTreeDialog.cpp:136/142`). iPad `PresetBrowserSheet.swift:67`. |
| Layout-group / preview picker | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `ChoiceLayoutGroups` dropdown (`LayoutPanel.cpp:663`). iPad `LayoutGroupPickerSheet.swift`. |
| Layout editor real-time 3D ModelPreview | panel | ✅ | ✅ | parity | P1 | medium | feasible | Desktop `layout/ModelPreview.cpp` + `HousePreviewPanel` (GL/Metal, the main editor view). iPad `PreviewPaneView`/`HousePreviewView`/`ModelPreviewView`. |
| Value-curve editor with preset library | dialog | ✅ | ✅ | parity | P1 | medium | feasible | Desktop `ValueCurveDialog` (`PopulatePresets`, Load) + `ValueCurvesPanel`. iPad `ValueCurveEditor.swift` + `ValueCurvePresetSheets.swift`. |
| Color-curve editor | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `color/ColorCurveDialog.cpp`. iPad `ColorCurveEditor.swift`. |
| Vendor model catalog browser | dialog | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `import_export/VendorModelDialog.cpp`. iPad `VendorBrowserSheet.swift`. |
| Display Elements editor | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop "Edit Display Elements" (`ID_ROW_MNU_EDIT_DISPLAY_ELEMENTS` + `ViewsModelsPanel`). iPad `DisplayElementsSheet.swift`. |
| Check Sequence report | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop check-sequence report (xLightsMain). iPad `CheckSequenceSheet.swift`. |
| Media manager | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `SeqSettingsDialog` Media tab. iPad `MediaManagerSheet.swift`. |
| Package Sequence (+media) | menu/dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop **File → Package Sequence** (`xLightsMain.cpp:6095`). iPad `PackageSequenceSheet.swift`. |
| Package Log / diagnostic files | menu | ✅ | ✅ | parity | P2 | medium | feasible | Desktop **Package Log Files** (`OnMenuItemPackageDebugFiles`). iPad `XLDiagnosticUploader.swift` (+MetricKit, TestFlight upload). iPad adds device telemetry. |
| New-sequence wizard | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `SeqSettingsDialog` wizard. iPad `NewSequenceWizardView.swift`. |
| Lyrics import via LRCLIB | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop online lyric search (`ID_ROW_MNU_SEARCH_LYRICS_ONLINE`). iPad `LRCLIBClient.swift`. |
| Blending panel (opacity / blend mode) | panel | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `sequencer/BlendingPanel.cpp`. iPad `BlendingPanelViews.swift`. |
| Effect-timing nudge (fine / frame / mark) | shortcut/menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop arrow-key `MoveSelectedEffectLeft/Right(shift,ctrl,alt)` (`MainSequencer.cpp:995`). iPad surfaces them as a visible **Nudge Selection** menu (`XLightsCommands.swift:157`). UX diff only. |
| Drag-drop effects between rows | gesture | ✅ | ✅ | parity | P2 | medium | feasible | Desktop wx drag. iPad `EffectsMetalGridView.swift:66`. |
| Drag-drop models in layout sidebar | gesture | ✅ | 🟡 | parity | P1 | easy | feasible | Desktop wx drag. iPad SwiftUI `dropDestination` (J-32 partial). |
| Color palette — list sort | menu | ❌ | ❌ | parity | P3 | n/a | n/a | **Neither** has palette-list sorting (iPad `ColorPaletteView.swift` = 8 fixed slots, no sort; desktop palette has none). Not a reverse-parity item. |
| Edge auto-scroll (auto-pan) while dragging/resizing/fading an effect | gesture | ❌ | ✅ | desktop-missing | P2 | medium | feasible | On desktop, dragging an effect to an off-screen time forces the user to abort, scroll, and re-grab. A wxTimer-driven edge auto-scroll during mResizing/mDragging (mirroring the iPad margin/speed ramp) would close this. |
| Drag-to-move snap-to-grid for layout models / view objects | capability | ❌ | ✅ | desktop-missing | P2 | easy | feasible | Desktop already stores + renders the 2D grid spacing; adding a 'Snap to Grid' bool (preference or toolbar toggle) and quantising the moved centre to `GetDisplay2DGridSpacing()` in the BoxedScreenLocation drag would snap models to the 2D background grid spacing. |
| Check Sequence: tap an issue to navigate the live sequencer to the offending effect | other | 🟡 | ✅ | desktop-weaker | P2 | medium | feasible | Desktop's report is read-only and external; the user must hand-find the named effect. iPad turns the report into an actionable navigation list (auto-switching to Master View). Desktop could add an in-app wxDataViewCtrl report whose rows call SelectEffect/seek. |
| Persistent, named, draggable shaded loop region on the ruler+waveform strip | gesture | 🟡 | ✅ | desktop-weaker | P2 | medium | feasible | Desktop CAN loop a section but only transiently (selection-tied, cleared on stop) and never visualizes it as a dedicated band. iPad keeps a standing region with a persistent shaded overlay + boundary lines across ruler and waveform, surviving play sessions, with toggleable loop-play that wraps the playhead. |
| Reuse an up-to-date on-disk .fseq on open to skip the initial render | capability | ❌ | ✅ | desktop-missing | P3 | medium | feasible | Re-using an up-to-date on-disk .fseq to avoid the open-time RenderAll() would speed up opening large sequences. Core FSEQ read code already exists (`render/FSEQFile.h`). Internal/perf-facing. |
| Memory-pressure render abort + partial-render guard flag | capability | ❌ | ✅ | desktop-missing | P3 | hard | infeasible | iPad-only: relies on iOS memory-warning callbacks that desktop OSes don't deliver, and desktop runs in a memory-rich environment. Largely infeasible/low-value to port; recorded as intentionally iPad-only. |
| Undo-history memory cap (bounded undo step count) | capability | ❌ | ✅ | desktop-missing | P3 | easy | feasible | The capping API lives in shared core, so desktop COULD opt in with one line to bound undo-snapshot memory growth in long sessions. Low value on memory-rich desktops, hence P3. |
| Copy current palette string to clipboard | context-menu | ❌ | ✅ | desktop-missing | P3 | easy | feasible | Desktop can paste a palette IN (Import Palette text dialog) but cannot copy the current palette OUT as a string. Trivial: one menu item + `wxTheClipboard->SetData(new wxTextDataObject(GetCurrentPalette()))`. |
| Set transition TYPE directly on the effect grid (long-press the in/out fade diamond) | context-menu | ❌ | ✅ | desktop-missing | P3 | medium | feasible | Distinct from the fade-DRAG item (which sets fade TIMES); this sets the transition TYPE. Desktop could add a right-click entry on the in/out fade-stripe region of an effect that pops a transition-type choice. |
| Remove Unused Media — bulk-drop every media entry not referenced by any effect (with confirm + count) | sheet | ❌ | ✅ | desktop-missing | P3 | easy | feasible | The core `SequenceMedia::RemoveUnusedMedia()` the iPad calls already lives in src-core/, so desktop just needs a 'Remove Unused Media' button on ManageMediaPanel (or a Tools menu item). |
| Live invalid-drop red tint during cross-row / overlapping effect move | badge-indicator | ❌ | ✅ | desktop-missing | P3 | medium | feasible | Desktop gives no in-flight signal that a move will land on an overlap (it just clamps silently); the iPad red tint tells the user before they release, and is the only signal for cross-row moves. Feasible as a draw-time overlay. |
| Double-tap empty cell to create a gap-filling effect (one-gesture quick-add) | gesture | ❌ | ✅ | desktop-missing | P3 | medium | feasible | A double-click-empty gap-fill is a discoverable one-gesture quick-add; the underlying gap-fill + create logic already exists in core. Feasible on desktop. |
| Momentum / kinetic scroll on the effect grid after a flick | gesture | ❌ | ✅ | desktop-missing | P3 | medium | hard | Largely a touch idiom: macOS trackpad two-finger scroll already supplies OS-level momentum, so low value for mouse-driven desktop. |
| Controller wiring export to JSON from the Visualizer UI | sheet | 🟡 | ✅ | desktop-missing | P3 | easy | feasible | CSV/spreadsheet export is parity; JSON export from the dialog is the gap. Trivial desktop add — a 'Save As JSON…' menu item next to 'Save As Spreadsheet…' that calls the existing `cud.ExportAsJSON()`. |
| Loop-region context menu (enable/disable loop-play, clear, nudge edges) invoked from the loop band | context-menu | ❌ | ✅ | desktop-missing | P3 | easy | feasible | Natural companion once desktop has a persistent loop region: a right-click on the ruler band to clear / toggle loop-play / nudge edges. Low ROI without the persistent region first. |
| Spatial-mode badge overlaid on palette ColorCurve gradient swatch | badge-indicator | 🟡 | ✅ | desktop-weaker | P3 | easy | feasible | Desktop shows the gradient but not WHICH spatial mode on the swatch itself; the user reads the small CC-row cycle buttons or opens the editor. A one-glyph overlay in `wxColorCurveRenderer::GetColorCurveImage` would close it. |
| Numeric position (x) entry for a ColorCurve gradient point | inline-edit | 🟡 | ✅ | desktop-weaker | P3 | easy | feasible | Desktop offers no way to place a gradient point at a precise x — drag-only on the canvas, hard to hit an exact value. The numeric position field is a real precision affordance desktop could add. |
| Import-Palette dialog pre-fills from clipboard | sheet | 🟡 | ✅ | desktop-weaker | P3 | easy | feasible | Core Import-Palette-from-text is parity, but desktop doesn't auto-fill the import field from the clipboard. Low ROI; bundle with the Copy-Palette-String addition if that lands. |
| Draggable 2D XY pad for effect center/position (xyCenter group) | grid-interaction | 🟡 | ✅ | desktop-weaker | P3 | medium | feasible | Desktop forces users to set X and Y on two separate sliders; iPad places the center in a single drag. Desktop could add an owner-drawn 2D pad widget alongside (or in place of) the paired sliders in BuildXYCenter. |
| Inline Morph line editor in the effect inspector (4 draggable endpoints) | grid-interaction | 🟡 | ✅ | desktop-weaker | P3 | medium | feasible | Capability exists on desktop but is gated behind an off-by-default floating Effect Assist pane and is not visible alongside the Morph effect's own sliders. iPad surfaces it inline so it's always visible while editing. |
| Inline Sketch path editor in the effect inspector (drag endpoints / control points, add-line) | grid-interaction | 🟡 | ✅ | desktop-weaker | P3 | medium | feasible | iPad subset is less capable than the full desktop SketchAssistPanel (no closing paths / SVG import / curve creation) but it is surfaced inline in the inspector. The reverse-parity angle is surfacing it inline. |
| Open-sequence flow shows render status (modified + last-rendered dates) and 'Out of Date First' / 'Rendered Newest' sort | badge-indicator | 🟡 | ✅ | desktop-weaker | P3 | medium | feasible | Desktop could replace/augment the bare wxFileSelector open with a sequence list surfacing the existing fseq-vs-xsq freshness logic + an 'Out of Date First' ordering (date columns + column sort already exist in BatchRenderDialog). |
| Per-effect media-reuse picker persists which folder groups are collapsed across launches | sheet | 🟡 | ✅ | desktop-weaker | P3 | easy | feasible | The media reuse list itself (one-tap 'used in this sequence' + Browse/Add-from-Disk) is parity — desktop SelectMediaDialog shows the sequence's media tree with the current path pre-selected and an Add-from-Disk button. The only iPad delta is persisting collapsed folder-group state across launches. |
| Cursor-anchored live drag readout pill (start/end + duration AND fade-in/out seconds) | inline-edit | 🟡 | ✅ | desktop-weaker | P3 | easy | feasible | Desktop surfaces start/end/duration only in the far-away status bar and omits fade seconds entirely (it has no fade-drag on the grid at all). A cursor-local pill — especially the fade-seconds readout — is a real readability/precision win. |
| Import Effects: incremental list FILTER that hides non-matching model rows with a live matched/total count | inline-edit | 🟡 | ✅ | desktop-weaker | P3 | medium | feasible | On a 300-model show desktop keeps all 300 rows visible and just jumps the viewport to the first hit; iPad collapses the list to matches and shows how many. Desktop could repopulate the wxListCtrl / wxDataViewCtrl from a filtered set. |
| Import Effects: symmetric pre-import frame-rate mismatch notice (any src≠tgt) explaining timings will be snapped | sheet | 🟡 | ✅ | desktop-weaker | P3 | easy | feasible | Desktop catches only one direction with terse wording; iPad warns symmetrically and tells the user what happens to timings. A one-line widening of the existing check reaches parity. |
| Batch Render: in-dialog per-sequence render fraction + combined overall batch bar + "Rendering X of N" | sheet | 🟡 | ✅ | desktop-weaker | P3 | medium | feasible | Both have a progress bar, but desktop's is a coarse 10/90/100-per-file jump with batch position only in status text; iPad shows a true per-sequence frame fraction plus a combined overall bar and an X-of-N count. |
| Terrain heightmap shaping via smooth cosine-falloff brush + tap-to-paint Raise/Lower (additive) | gesture | 🟡 | ✅ | desktop-weaker | P3 | medium | feasible | Capability (brush + neighbour blur) is largely parity. Remaining differences: iPad applies a smooth cosine falloff over a circular radius (natural bump) vs desktop's flat square plateau set-to-value, plus additive tap-to-paint Raise/Lower. |
| Per-sub-layer lyric glyph on timing rows (Phrases / Words / Phonemes each get a distinct icon) | badge-indicator | 🟡 | ✅ | desktop-weaker | P3 | medium | feasible | Desktop signals 'this is a lyric track' with a single header icon but never distinguishes Phrases/Words/Phonemes glyph-wise on each sub-layer row. Desktop could DrawBitmap a per-layerName icon in the sub-layer branch (`:2431-2446`). |
| Double-tap a timing mark to LOOP-play just that mark | gesture | 🟡 | ✅ | desktop-weaker | P3 | easy | feasible | Desktop already plays a single timing mark on double-click; the only delta is it plays once by default whereas iPad LOOPS the mark. Desktop already has the looped primitive (PlayLooped). |
| Tap-to-seek on the ruler snaps the playhead to the nearest timing-mark edge (any mark on any timing row, within ~8px) | gesture | 🟡 | ✅ | desktop-weaker | P3 | easy | feasible | iPad's playhead seek magnetizes to mark boundaries so you land exactly on a phrase/beat edge; desktop seek only frame-rounds. Desktop already has a GetSnapToTimingMarks() preference and the snap math for effect drags — it could reuse it for ruler seek. |

## iPad gaps (desktop has, iPad missing)

There are **no P1/P2 desktop-→-iPad gaps in this theme** (this theme is about
the *reverse* direction; standard iPad-missing items live in the other theme
docs). The only iPad-missing items here are:

### P3
- **Layout editor Controllers tab** — present on desktop
  (`LayoutPanel` notebook, always shown), intentionally omitted on iPad
  (`LayoutEditorView.swift:29`, J-5). This is *restricted*, not a bug: the
  Controllers tab drives controller config/upload, which on iPad is gated to
  open-source firmware (FPP/WLED/ESPixelStick/DDP). Surfacing a partial tab is
  low value until the open-firmware upload path is fleshed out.

## Desktop gaps (iPad has, desktop missing)

Grouped by priority. Many items here are P3 — the desktop ships the
underlying feature, so these are pure UX refinements the desktop *could* adopt
but that carry low ROI. The genuine functional/capability gaps (fade-time drag,
edge auto-scroll, snap-to-grid, persistent loop region, actionable Check-
Sequence, `.xsqz` round-trip, Remove-Unused-Media, etc.) are captured in the
scorecard above. Timing-mark ops, AI services, save-preset, layout-group picker,
export-models, controller-edit, and FPP progress are all present in
`src-ui-wx/` (parity).

### P3 (optional desktop polish)
- **Case-sensitive Find toggle.** `EffectsGrid::Find` is hard-wired
  case-insensitive (`::Lower()` both sides, `EffectsGrid.cpp:705`). iPad's
  `FindReplaceSheet` exposes a toggle. Desktop work: add a wxCheckBox to the
  find dialog (or a small `wxFindReplaceData`-style flag) and skip the `Lower()`
  when set. Easy.
- **In-app log tail.** Desktop "View Log" opens the file in an external editor
  (`xLightsMain.cpp:5092`); iPad tails in-app (`LogViewerSheet`). Desktop could
  add a `wxLogWindow`/text-ctrl tail. Medium. (External-editor handoff is fine
  for most desktop users, so low value.)
- **Interactive media-relocation assistant.** Desktop auto-relocates + notifies
  on load (`xLightsMain.cpp:5570`); iPad has a guided picker
  (`MediaRelocation.swift`). Desktop could add a manual "Locate…" dialog for the
  cases auto-search misses. Medium.
- **Richer sort pickers.** iPad layout-model sort (8 modes) and batch-render
  sort picker are explicit dropdowns; desktop uses clickable/persisted tree
  columns. Functionally equivalent; a picker is just more discoverable. Easy.

## Infeasible / restricted on iPad

These are iPad-only by nature and should **not** be ported to desktop:

- **Apple Pencil Pro double-tap / squeeze → undo** — no stylus event layer in
  wxWidgets; Apple-only HW interaction.
- **Multi-touch gestures** — *partly already on desktop.* wxWidgets exposes
  trackpad/touch gesture events, and the desktop **uses them**: `wxEVT_GESTURE_ZOOM`
  (`ModelPreview::OnZoomGesture` → pinch-zoom the camera; `LayoutPanel::OnPreviewZoomGesture`
  → pinch-scale the selected model) and `wxEVT_GESTURE_ROTATE`
  (`LayoutPanel::OnPreviewRotateGesture` → rotate the selected model, axis via Shift/Ctrl),
  enabled via `EnableTouchEvents(wxTOUCH_ZOOM_GESTURE | wxTOUCH_ROTATE_GESTURE)`, plus a
  `wxEVT_MAGNIFY` trackpad path. So **pinch-zoom and pinch-scale are parity** and
  two-finger rotate is desktop-weaker (rotates model, not camera). Genuinely
  iPad-only/infeasible: **two-finger pan** (no `wxEVT_GESTURE_PAN`/`wxTOUCH_PAN` binding —
  desktop pans via drag/keys), **two-finger marquee** (desktop uses single-pointer
  left-drag rubber-band), and the **camera-roll** target of two-finger rotate.
- **iCloud Documents auto-sync + sync indicator** — iPad is a document-based
  app on iCloud Drive (`NSFileCoordinator`); desktop is a classic local-FS app
  with no cloud-document model. Adding cloud sync is a separate product, not a
  parity item.
- **On-device LiDAR depth solve in Map-from-Lights** — *restricted to iPad HW*.
  The macOS desktop already has the structured-light scan via **Continuity
  Camera** (`KLightMapperBridge` + `xLightsMain.cpp:4296`), but the LiDAR
  depth-based Procrustes multi-view solve is iPad-hardware-only; Mac/non-LiDAR
  iPad fall back to the 2D triangulator. Not portable to a webcam-less desktop.

Reverse-direction restricted item (desktop ahead of iPad):

- **Layout Controllers tab** — *restricted=closed-firmware/IAP*. Present on
  desktop, gated off on iPad until open-firmware controller config/upload lands.
  P3.

## Recommended sequencing

Much of the "reverse parity" surface is already at parity, and many true
iPad-only items are platform idioms that should stay iPad-only. The actionable
desktop work, in order:

1. **Fade-time drag on the grid (desktop, P1)** — the highest-value pull-back;
   desktop has no grid fade-drag at all, so transition in/out times can only be
   set via the Blending/Layer panel.
2. **Edge auto-scroll while dragging (desktop, P2)** — a wxTimer-driven edge
   auto-scroll during resize/drag removes the abort-scroll-regrab cycle.
3. **Snap-to-grid for layout models (desktop, P2)** — quantise the moved centre
   to `GetDisplay2DGridSpacing()` behind a Snap-to-Grid toggle.
4. **Actionable Check-Sequence navigation, persistent loop region, `.xsqz`
   round-trip, Remove-Unused-Media (desktop, P2)** — each turns an existing
   capability into an in-app interaction.
5. **Case-sensitive Find toggle (desktop, P3)** — one checkbox + a branch around
   `::Lower()` in `EffectsGrid::Find`.
6. **In-app log tail (desktop, P3)** — a `wxLogWindow` is cheap.
7. *(Skip)* media-relocation assistant and sort-pickers — equivalent function
   already exists; defer indefinitely.

Everything else here is closed (parity) or out-of-scope (touch/pencil/camera/
cloud idioms, or the firmware-gated Controllers tab).
