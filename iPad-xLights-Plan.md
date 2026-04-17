# xLights iPad -- Implementation Plan

This plan is based on the current state of the code. It covers what
exists today, what still needs to be built, and the order to build it
in.

---

## Current state

### Goal of the work so far

The work so far has been scoped to the **core rendering path**: load a
sequence, render it through the same C++ effect pipeline as desktop
xLights, and display the house preview during playback. That is done
and verified on a physical iPad. Shaders, video effects, and most
effects render correctly.

Known core gap: the **Text effect** uses a callback into UI code for
text layout and still needs to be rerouted through a platform-neutral
text drawing path. An ObjC++ stub
(`CoreGraphicsTextDrawingContext.mm`) is in place but not fully wired.

Everything above the core baseline -- effects grid, inspector, output,
preview chrome -- was built far enough to prove the sequence loaded
and the pipeline ran. It is not production UI.

### Code layout

iPad code lives at `src-iPad/` at the repo root (peer to `src-ui-wx/`).
It is not under `macOS/`: code in `macOS/` cannot depend on anything
outside `macOS/`, so the iPad UI sits alongside the other UI layer
instead.

```
src-iPad/
  App/            SwiftUI views + view model (~2000 LOC)
  Bridge/         ObjC++ bridges (XLSequenceDocument, iPadRenderContext,
                  XLiPadInit, CoreGraphicsTextDrawingContext)
  Metal/          xlStandaloneMetalCanvas, iPadModelPreview, XLMetalBridge
  Metadata/       EffectMetadata.swift (JSON model for effectmetadata/*.json)
```

Shared core that iPad consumes:

- `src-core/` -- wx-free C++ core (renderers, models, outputs, render
  engine, effect manager, sequence file/elements, audio manager).
- `macOS/src-apple-core/` -- Apple shared code (Metal device manager,
  external hooks, Apple utilities in Swift and ObjC++).

### Xcode targets

| Target | Purpose | Multi-platform |
|---|---|---|
| `xLights-core` | wx-free core (`src-core/`) | macOS + iOS |
| `xLights-Apple-core` | Apple shared code (`macOS/src-apple-core/`) | macOS + iOS |
| `xLights-macOSLib-UI` | macOS-only UI (`macOS/src-mac-ui/`) | macOS only |
| `xLights` | Desktop app | macOS only |
| `EffectComputeFunctions` | Metal shaders | macOS + iOS |
| `UIMetalShaders` | UI Metal shaders | macOS only |
| `ISPCEffectComputeFunctions` | SIMD kernels | macOS + iOS |
| `xLights-iPadLib` | iPad bridge code (`src-iPad/`) | iOS |
| `xLights-iPad` | SwiftUI iPad app | iOS 26+ |

iOS dependencies live at `/opt/xLights-macOS-dependencies/lib-ios/`:
`libcurl.a`, `libEGL.xcframework`, `libGLESv2.xcframework` (ANGLE),
`libliquidfun.a`, `liblua.a`, `libxlsxwriter.a`, `libzstd.a`. Debug
variants in `libdbg-ios/`.

### What is verified on device

- Sequence load from a show-folder the user picks via `FolderConfigView`
  (security-scoped bookmarks survive relaunch).
- Background render via `iPadRenderContext` -> `RenderEngine` -> `JobPool`.
- House preview during playback (`iPadModelPreview : IModelPreview`,
  `HousePreviewView` = MTKView + `CADisplayLink`).
- Audio playback + waveform (`AudioManager` bridge, 30fps view-model
  timer, Space keyboard shortcut).
- Shaders, video effects, most effect renderers.

### What is thin / placeholder

- **Effects grid (`SequencerView.swift`)** -- to be scrapped. Its
  gestures and layout don't match desktop xLights; keeping it
  encourages muscle memory that won't transfer.
- **Effect settings inspector** -- single scrolling list with
  collapsible Buffer/Color/Timing sections. Does not scale past a
  handful of properties; users can't find anything.
- **Model preview chrome** -- draws, but 250pt inline strip. No
  rotate/zoom, no separate window.
- **`isRenderDone`** -- bridge method returns `YES` whenever
  `SequenceData` is valid, not "engine is actually done." Harmless
  today, wrong in principle.
- **JobPool** -- hard-coded at 4 threads. This is a known hazard:
  desktop sees workers stalling while waiting on other models' frames,
  and with only 4 workers a contended sequence can deadlock. Needs to
  be at least 24.
- **Visibility-rule engine in inspector** -- `equals` / `oneOf` only.

### Known core issues

- **Text effect** -- uses UI-layer callback for text layout. Needs
  platform-neutral drawing path wired through
  `CoreGraphicsTextDrawingContext`.

### What does not exist yet

- Multi-window scenes / external display / Stage Manager integration
- iPadOS 26 menu bar (`Commands { CommandMenu(...) }`) and discoverable
  keyboard shortcuts
- Undo manager
- Pinch-zoom, long-press context menu, edge-resize handles, multi-select
- Copy/paste, Apple Pencil precision gestures
- Transition editing (`T_*` keys) and most palette color pickers
  beyond the built-in shared-panel UI
- iCloud Drive document handling beyond "user picked the folder"
- Memory-pressure monitoring (`os_proc_available_memory`)
- App Store entitlements, icons, launch screens, privacy manifest,
  review-submission pipeline

---

## Plan

Guiding principle: **the core render path works; shore up the bits
that make the app trustworthy, rebuild the UI that was scaffolded,
then productize.** Controller output is out of MVP scope.

### Phase A: Core-path hardening

Small fixes with high blast radius. These make the "core path is
verified" guarantee hold up under load.

1. **JobPool thread count** -- **done** -- bumped from 4 to
   `max(24, hardware_concurrency * 2)` in `iPadRenderContext::RenderAll`.
   - Longer-term option (out of scope here, worth tracking): redesign
     the "waiting on another model's frame" path to re-enqueue the
     job instead of blocking a thread. That's the real fix, but it is
     a cross-platform refactor of `RenderEngine` and shouldn't block
     iPad.
2. **Text effect** -- **done.** Abstraction is shared:
   `src-core/render/TextDrawingContext.{h,cpp}` (platform-neutral
   interface + factory), desktop uses
   `src-ui-wx/ui/graphics/wxTextDrawingContext`, iPad uses
   `src-iPad/Bridge/CoreGraphicsTextDrawingContext.mm` (CoreText +
   CGBitmapContext), and `XLiPadInit::initialize` registers the CG
   factory at startup. `TextEffect.cpp` has no wx includes. Font /
   size parity with desktop verified side-by-side via the Model
   Preview pane shipped in Phase D. Fixes along the way:
   - `ParseFontString` now handles wx's native user-desc format
     (`"bold arial 26 utf-8"`) by classifying each token
     independently and discarding charset markers, instead of only
     popping the last token as size.
   - Anti-aliasing and font smoothing disabled on the CG context so
     pixel-grid text renders with the same hard edges as desktop
     (matches `antiAliased = false` in `wxTextDrawingContext`).
   - Vertical positioning matches wxDC semantics: baseline placed at
     `y + ascent` (not `y + descent`), with ascent/descent/leading
     pulled from `CTLineGetTypographicBounds` -- same source
     `wxOSXCG::DoGetTextExtent` uses -- so `GetTextExtent` height and
     baseline offset can't disagree and shift the block.
   - Font face lookup is case-insensitive: `CTFontCreateWithName`
     silently falls back to the system font on misses, so
     `CoreGraphicsTextDrawingContext` now resolves faces via a
     cached scan of `CTFontManagerCopyAvailableFontFamilyNames`
     before handing the canonical name to CoreText. wx writes face
     names to `.xsq` files in whatever case it feels like
     (`"arial"` / `"Arial"` / `"ARIAL"`); all land on the same
     installed family now.
3. **`isRenderDone`** -- **done** -- replaced the UI-drain-dependent
   detection with a self-signaling completion path. `RenderProgressInfo`
   now carries a `jobsRemaining` atomic and a `completed` atomic. Each
   `RenderJob::Process()` has a `FinishNotifier` RAII guard that calls
   `RenderEngine::NotifyJobFinished(rpi)` on every exit path (normal
   completion, abort, early-bail, exception). The thread that brings
   `jobsRemaining` to zero CAS-flips `completed` exactly once. The
   callback itself is NOT invoked from the worker thread -- desktop's
   callback manipulates wx widgets directly, so firing from a worker
   would be a UI-thread violation. Instead, each platform's main-thread
   drain fires the callback: desktop's `UpdateRenderStatus` calls it
   after `CleanupJobs()`, and iPad's `IsRenderDone()` walks
   `_renderProgressInfo`, cleans up every entry whose `completed` atomic
   is set, fires the callback, and erases. Cancellation is handled
   correctly -- `SignalAbort` sets `abort` on each job, jobs break out
   of their frame loop and exit normally, the guard signals completion,
   and `_abortedRenderJobs > 0` flows into the callback.
4. **Memory-pressure sink** -- **done** -- registered
   `DispatchSource.makeMemoryPressureSource([.warning, .critical])`
   and `UIApplication.didReceiveMemoryWarningNotification` in
   `SequencerViewModel.startMemoryMonitoring`. Bridge methods
   `handleMemoryWarning` / `handleMemoryCritical` signal abort and
   purge `_renderCache`. A 5-second poll on
   `os_proc_available_memory()` flips `memoryWarning` on below 256 MB
   and clears at 384 MB (hysteresis); `ContentView` surfaces an orange
   banner while active.
5. **File resolution for cross-machine sequences** -- **done** --
   sequences saved on a desktop store absolute Mac paths (e.g.
   `/Users/.../Shows/Christmas/BeepBeep/BeepBeep1.mp4`) that don't
   exist at that location on the iPad. Two fixes:
   - `iPadRenderContext::LoadShowFolder` now wires
     `FileUtils::SetFixFileShowDir(showDir)` +
     `SetFixFileDirectories(_mediaFolders)` (and
     `ClearNonExistentFiles()` on reload). Previously iPad had no way
     to populate the FixFile search path; media-folder assets stayed
     unresolvable even though the bookmarks worked.
   - `SequenceMedia::ResolvePath` and three inline copies
     (`GetImage`, `AddAnimatedImage`, `VideoMediaCacheEntry::Load`)
     no longer skip `FixFile` on absolute paths. The `is_absolute()`
     short-circuit was a desktop-only optimization; on iPad it let
     the saved Mac paths pass straight through without
     re-resolution. `FixFile`'s fast path is still
     `if (FileExists(file, false)) return file;` so desktop behaviour
     is unchanged when the saved path still works.

Verified with iPad-simulator arm64 build of `xLights-iPad` and on
device (audio, 3D assets, shaders, and videos now resolve).

Not yet landed: porting the desktop re-prompt-on-failed
`ObtainAccessToURL` pattern to iPad. Today we ignore the return
value, so a stale bookmark leads to silent lookup failure. Minimum
version is checking the return, logging, and dropping the failed
folder from `_mediaFolders` before handing it to `FileUtils`; full
re-prompt UX needs a Swift callback + `UIDocumentPickerViewController`.

### Phase B: Rebuild the effects grid

Goal: UI that behaves like the desktop effects grid so muscle memory
transfers. The current SwiftUI grid is the replacement target, not the
starting point.

1. Layout study -- define a spec that matches the desktop grid's row
   model, snap behavior, drag behavior, and visual treatment. This is
   intentionally its own step before any code: rebuilding without a
   spec is how the current grid ended up the way it did.
2. Implement the grid on top of a proper coordinate/hit-testing layer
   (likely a `UIViewRepresentable` wrapping a custom `UIView` or
   `MTKView`, not a SwiftUI `ZStack` of `EffectBlockView`s). Effects
   are dense; SwiftUI's view graph buckles once sequences get real.
3. Gesture set, in priority order:
   - Tap to select
   - Pinch to zoom timeline
   - Long press -> context menu (copy / paste / delete)
   - Drag to move effects
   - Edge-grab to resize (20pt hit zones)
   - Multi-select via selection-mode toolbar toggle
   - Apple Pencil precision (finer slop, hover preview)
4. **Edit actions trigger targeted re-render.** The inspector already
   calls `RenderEffectForModel(model, startMS, endMS, clear=false)`
   when a single setting changes. Grid edits must do the same -- every
   mutating action re-renders the affected model(s) across the
   affected time range:
   - **Add** -- render `[startMS, endMS]` on the new effect's model.
   - **Delete** -- render `[startMS, endMS]` with `clear=true` so the
     removed effect's output is cleared from `SequenceData`.
   - **Move / resize** -- render the union of old and new `[start, end]`
     on the model; if moved to a different row, render on both source
     and destination models.
   - Multi-effect operations coalesce by model before dispatch so we
     don't queue N overlapping renders.
   The `RenderEngine` API already supports this, but
   `XLSequenceDocument` currently exposes only the single-effect path;
   bridge methods need to widen.
5. **Selection-scoped preview scrub.** When an effect is selected (and
   audio isn't playing), the preview windows play the effect's time
   range on loop with no audio: play-head advances `startMS -> endMS`
   at frame rate, wraps. This is how the desktop app lets you see what
   an effect looks like without committing to full-sequence playback.
   - Start loop on selection, stop on deselection / audio play.
   - Only drives `playPositionMS` for preview; does not move the
     sequence timeline's main play-head marker (or does so with a
     distinct "preview" marker -- TBD in spec).
   - Multi-select: loop across the union range
     `[min(starts), max(ends)]`.
6. **View picker.** The desktop app organizes the row set via named
   views ("Master", "Trees", "Matrices", "Candy Canes", ...); the
   current iPad grid always shows every visible row from
   `SequenceElements::GetRowInformation()`, which is unusable on a
   sequence with hundreds of models.
   - Bridge `SequenceElements::GetViews()` /
     `SequenceElements::SetCurrentView(viewIndex)` through
     `XLSequenceDocument`.
   - Toolbar picker (menu button with view names) switches the active
     view; grid re-reads row info after the switch.
   - Persist last-used view per sequence via `SceneStorage`.
   - "Master" is always the first entry.
7. Undo manager plumbing -- `document.undoManager` through
   `SequencerViewModel`, register inverse actions for
   add/delete/move/setting-change. Cmd+Z / Cmd+Shift+Z.

### Phase C: Effect settings inspector, tabbed

Replace the single collapsible list with a four-tab sidebar. Icons at
the top of the inspector switch between, mapped to the existing
settings-key prefixes:

1. **Effect** -- `E_*` keys, effect-specific properties from
   `resources/effectmetadata/<Name>.json`.
2. **Colors** -- `C_*` keys (palette map). Color pickers + palette
   swatches.
3. **Blending** -- `B_*` keys (the shared "Buffer" panel today, which
   is a misnomer in the current code and needs renaming internally).
4. **Timing** -- timing range + `T_*` transition keys (shared Timing
   panel plus `T_CHECKBOX_*` / `T_CHOICE_*` transition editors that
   aren't wired yet).

Per-tab work:

- **Effect** -- full visibility-rule engine (`notEquals`,
  `greaterThan`, `lessThan`, `and`, `or`), `xyCenter` group as an XY
  pad, value-curve preview (edit later).
- **Colors** -- wire the existing `ColorPaletteView` to write
  `C_BUTTON_Palette*` keys.
- **Blending** -- layout cleanup on the B_ panel.
- **Timing** -- add `T_CHECKBOX_*` / `T_CHOICE_*` transition editing
  (currently dead).

Tab identity is preserved across selections so the user returns to the
tab they were on, not always "Effect."

**Detaching tabs into windows.** On desktop xLights each of these four
panes is a separate dockable pane. iPadOS 26 can't float palettes the
way macOS does, but SwiftUI's multi-scene API plus Stage Manager and
external displays get us close:

- On compact width (iPad in isolation, Slide Over, narrow split) keep
  the four-tab notebook -- condensing is the only thing that fits.
- On regular width and / or when a second display is attached, expose
  a "Open in new window" action per tab that calls
  `openWindow(id: "inspector", value: InspectorTab.effect)` (or
  `.colors` / `.blending` / `.timing`).
- Declared as a single `WindowGroup(id: "inspector", for: InspectorTab.self)`
  in the app's `Scene` (coordinated with Phase E-1). Each detached tab
  is a real scene window Stage Manager and external displays can host;
  no true floating palette, but each pane can sit on its own.
- State syncs through the shared `SequencerViewModel` (selection + edit
  dispatch); per-window UI state (scroll position, which section is
  expanded within Effect) persists via `@SceneStorage`.
- When a tab is detached, the sidebar in the main window hides that
  tab and the notebook falls back to the remaining tabs; closing the
  detached window restores it.

### Phase D: Model Preview + preview polish

Both previews, fully interactive. Self-contained phase -- deliverables
plug into Phase E's window system as finished building blocks.

1. **Model Preview.** **Done** (minus selection-scoped scrub, which
   waits on Phase B-5). Desktop always has two previews alongside
   the grid: **House Preview** (full display) and **Model Preview**
   (currently selected model, isolated).
   - Shared `PreviewPaneView` (MTKView + `CADisplayLink`) wraps an
     `XLMetalBridge`; `HousePreviewView` and `ModelPreviewView`
     construct it with different bridge names and a
     `previewModelName` binding. `ContentView` splits the preview
     area into Model Preview (left) and House Preview (right).
   - `iPadModelPreview` holds `_currentModel`; empty string = draw
     everything (House), non-empty = restrict drawing to that model
     and skip view objects (Model). `drawModels` short-circuits to
     the selected model's `DisplayEffectOnWindow` in that case.
   - Groups are rendered the same way as single models:
     `Model::DisplayEffectOnWindow` works on `ModelGroup` because
     the group builds pseudo-nodes laid out by its default buffer
     style. Tapping a group row in the sequencer shows the group
     fit-to-window, matching desktop behaviour. The explicit
     `!= DisplayAsType::ModelGroup` filter that used to drop groups
     was removed from `XLMetalBridge.mm`.
   - Selection source: row tap in the effects grid sets the Model
     Preview's model via `setPreviewModel`. The effect-selection
     variant (derive model from selected effect's parent element)
     is deferred until the new grid (Phase B) ships.
   - Selection-scoped preview scrub is deferred -- depends on
     Phase B-5.
   - Model Preview fills its container; the 2D path in
     `iPadModelPreview::StartDrawing` uses
     `_virtualW/_virtualH == 0` as "no virtual-canvas scaling",
     which is the single-model fit-to-window case where
     `Model::DisplayEffectOnWindow` already lays geometry out in
     window pixel space.
2. **Gestures on both previews.** **Done** on device. Wired through
   `PreviewPaneView.Coordinator` onto each preview's `PreviewCamera`:
   - Pinch -> `setCameraZoom` (clamped 0.1..50).
   - One-finger drag -> 3D orbit (`setCameraAngleX/Y`) or 2D pan
     (`setCameraPanX/Y`), switched by `bridge.is3D()`.
   - Two-finger rotate (3D only) -> `setCameraAngleY` roll.
     `UIRotationGestureRecognizer`'s sign convention is opposite to
     what feels natural for a "grab the house and turn it" gesture,
     so the delta is negated in `handleRotate` before being applied.
   - Two-finger pan -> `setCameraPanX/Y`. Carried on the pinch
     recognizer's centroid (`recognizer.location(in: mtkView)`
     delta). A dedicated `min=2/max=2 UIPanGestureRecognizer`
     refused to fire alongside pinch+rotate even with
     simultaneous-recognition enabled (UIKit starves the
     translation component once pinch/rotate are active), so
     piggybacking on the pinch centroid was the reliable route.
     Did not fire in "Designed for iPad on Mac" but works on
     device.
   - Double-tap -> `resetCamera`.
   - Overlay buttons in `HousePreviewView` / `ModelPreviewView`
     post scoped notifications (`previewZoomIn/Out/Reset`,
     `previewResetCamera`) that the matching coordinator picks up
     by preview name. `iPadModelPreview`'s old fixed
     `GetCameraRotationX/Y` stubs are gone; the methods now return
     the live camera state.
3. **Saved camera views.** `xlights_rgbeffects.xml` already stores
   named cameras via `<Viewpoints>` / `ViewpointMgr::Load()` (loaded
   in `TabSequence.cpp:468`; each camera has `is_3d`). Expose them:
   - Bridge `ViewpointMgr` through `iPadRenderContext` (not yet
     wired -- the desktop `TabSequence` owns viewpoint load/save,
     iPad's `LoadShowFolder` currently skips the `<Viewpoints>` node).
   - Long-press on a preview opens a menu listing the named cameras
     filtered by the current 2D/3D mode of that preview.
   - Tap a name -> applies the `PreviewCamera` via `operator=`.
   - "Save current view as..." entry at the bottom of the menu
     captures the live camera state into `ViewpointMgr::AddCamera`.
   - Save path -- `ViewpointMgr::Save` is already plumbed
     desktop-side; iPad needs to trigger a rewrite of
     `xlights_rgbeffects.xml` on changes.
4. **2D vs 3D toggle.** Per-preview toolbar/segmented control that
   switches the `PreviewCamera` between `is_3d = true/false`. This is
   distinct from "current mode comes from the selected saved view" --
   the toggle is a preview-local override and will be persisted at the
   scene level during Phase E.
5. **Model placement on House Preview (2D/3D).** Desktop stores model
   placement for the house preview as part of each `Model`'s
   `ModelScreenLocation` in `xlights_rgbeffects.xml` -- verified in
   place. Make sure:
   - 3D placement attributes round-trip through
     `ModelManager::LoadModels` (they already do on iPad load; need to
     verify Save still writes them once we start editing).
   - If we discover 2D/3D placement state that's *only* stored in a
     desktop-side preference (e.g. layout-panel toolbar state) rather
     than `rgbeffects`, it gets moved into `rgbeffects` so iPad sees
     the same state. TBD during implementation; default assumption is
     that everything needed is already in `rgbeffects` and no schema
     change is required.
6. **Zoom / fit / center shortcuts** in the preview toolbar.
   **Partial.** The per-pane controls overlay (slider-icon button)
   exposes +, -, 1x, and Reset View, which map to
   `setCameraZoom` / `resetCamera`. "Fit All Models" and "Fit
   Selected Model" are not yet wired; both map to existing
   `PreviewCamera` operations so it's a UI addition, no new math.

### Phase E: Window system + Display Elements

Consumes finished previews from Phase D. Focus is the Scene-level
layout, menu bar, multi-window / external display routing, and the
Display Elements dialog.

1. Split `ContentView` into a `Scene`-level layout that supports
   multiple `Window` scenes for previews and inspector tabs:
   ```swift
   var body: some Scene {
       WindowGroup("xLights", id: "sequencer") { SequencerScene() }
       Window("House Preview", id: "house-preview") { HousePreviewWindow() }
           .defaultWindowPlacement { _, ctx in
               if let ext = ctx.displays.first(where: { !$0.isMain }) {
                   return WindowPlacement(ext)
               }
               return WindowPlacement(.trailing(ctx.windows.first!))
           }
       Window("Model Preview", id: "model-preview") { ModelPreviewWindow() }
       WindowGroup(id: "inspector", for: InspectorTab.self) { $tab in
           InspectorTabWindow(tab: tab ?? .effect)
       }
   }
   ```
   Both previews detach-able; House prefers the external display when
   present; Model stays with the main window by default. Inspector
   tabs detach on demand (Phase C) -- one `WindowGroup` keyed by
   `InspectorTab` so each tab opens as its own scene window.
2. **Size-class responsive layout.** Main sequencer window adapts via
   `@Environment(\.horizontalSizeClass)`:
   - **Compact** (narrow split, Slide Over) -- inspector stays as a
     four-tab notebook; previews collapse to a single toggleable inline
     strip. Detach actions are hidden.
   - **Regular** on a single display -- inspector shows the notebook
     but surfaces the per-tab "Open in new window" action; on 12.9"+
     both previews dock side-by-side at the top, on 11" one preview
     docks and the other can detach.
   - **External display attached** -- House Preview auto-routes to the
     external display on first open; inspector tabs and Model Preview
     remain offerable to either stage.
3. Docked layout -- inline strip mode houses both previews. On 12.9"+
   iPads a side-by-side docked pane at the top of the sequencer makes
   sense; on 11" the user toggles which preview is docked (House
   default) and the other lives in a detached window or slide-over.
4. `Commands { CommandMenu(...) }` for File / Edit / View / Playback --
   Cmd+Z, Cmd+C, Cmd+V, Cmd+S, Space discoverable via the iPadOS 26
   menu bar. View menu entries follow the desktop menu (see E-7) --
   each panel toggle lives here, including "Open Effect in New Window"
   / "Open Colors in New Window" / etc. for the four inspector tabs.
5. Persist open sequence + play-head + which panels are visible +
   which are docked vs detached + which inspector tabs are detached
   (and on which display) + per-preview 2D/3D mode (Phase D-4), via
   `SceneStorage` / `@AppStorage`. Currently only folder bookmarks
   survive.
6. **Display Elements dialog.** The desktop "Display Elements" panel
   is how users create and edit the named Views that Phase B-6
   switches between. Without it, the view picker is read-only.
   Implement as a modal sheet (`.sheet`) rather than a dockable panel
   -- it's used infrequently and doesn't need to stay on screen.
   - Lists current views and their member models/submodels/groups.
   - Add / rename / delete view.
   - Add / remove / reorder models within a view.
   - Persists back through `SequenceElements::GetViews()`
     serialization path already used by desktop; no new on-disk
     schema.
   - Accessible from the View menu (E-4) and from a toolbar button
     next to the view picker.
7. **Desktop panels not yet planned for.** The desktop View menu
   lists these panels beyond the six main ones Phases B-E cover
   (Grid, Model Preview, House Preview, Effect Settings with Colors /
   Layer Blending / Layer Settings tabs). None of these block MVP;
   tracking for a future milestone:
   - **Effect Dropper** -- quick effect-type picker for drag-to-place.
     Probably subsumed into our bottom effect palette strip.
   - **Value Curves** -- curve editor. Phase C lists preview-only;
     full editor is post-MVP.
   - **Color Dropper** -- eyedrop a color from any preview into the
     palette.
   - **Effect Assist** -- effect-specific helper panel (varies per
     effect; not all effects define one).
   - **Select Effect / Search Effects / Find Effect Data** -- three
     related search/filter UIs over the effect set in the sequence.
   - **Video Preview** -- plays the sequence's media (video) track
     aligned to the play-head.
   - **Jukebox** -- quick-jump buttons to timing marks.

   Post-MVP milestone "Panel parity" will take a pass at these in
   priority order once the core UI is stable.

### Phase F: Document / iCloud polish

1. File coordination for sequence writes (`NSFileCoordinator`).
2. Ubiquity status -- show "downloading" state for sequences not yet
   materialized locally. `FileExists()` already triggers iCloud
   downloads; surface the progress.
3. Register `.xsq` as a document type
   (`UTExportedTypeDeclarations`) so Files app "open in xLights"
   works.
4. Handle `Scene.willResignActive` -- pause output, flush render
   cache, stop timers.

### Phase G: App Store readiness

1. Icon set (1024x1024 + all iPad sizes), launch screen.
2. Privacy manifest (`PrivacyInfo.xcprivacy`).
3. `Info.plist`: `NSLocalNetworkUsageDescription` + `NSBonjourServices`
   for controller discovery (relevant once output re-enters scope).
4. TestFlight build, external beta group, feedback iteration.
5. Submission.

---

## Phase summary

| Phase | Title | Scope |
|---|---|---|
| A | Core-path hardening | JobPool, Text effect, render-done, memory pressure |
| B | Effects grid rebuild | Spec, new grid, gestures, re-render on edit, selection scrub, view picker, undo |
| C | Inspector tabs | Effect / Colors / Blending / Timing by key prefix |
| D | Model Preview + preview polish | Second preview, gestures, saved cameras, 2D/3D, model placement |
| E | Window system + Display Elements | Multi-Scene layout, menu bar, Display Elements dialog |
| F | iCloud polish | File coordination, download UI, `.xsq` handler |
| G | App Store | Assets, manifest, TestFlight, submission |

**Parallelism.** After Phase A lands, Phases B / C / D are independent
and can run in parallel (different teams / different commits). B
works against the existing `iPadRenderContext` + `XLSequenceDocument`;
C works against `EffectMetadata.swift` + the inspector view; D works
against `iPadModelPreview` + `ViewpointMgr`. Merge conflicts are
limited to `SequencerViewModel` (selection source) and
`XLSequenceDocument` (bridge surface area).

Phase E depends on all three -- it composes the finished pieces into
the final window / menu-bar layout. F and G are sequential at the end.

### Deferred / explicitly out of MVP

- **Controller output** -- infrastructure is in the tree (output
  manager, per-frame send, sACN/ArtNet/DDP/OPC) but not on the MVP
  critical path. Revisit after App Store submission.
- **JobPool requeue redesign** -- desktop-scope refactor to replace
  block-on-other-model-frame with re-enqueue. Tracked separately; the
  iPad workaround is "more threads."
- **Layout editor, controller setup** -- stays desktop-only.

---

## Risks

- **JobPool deadlock on complex sequences** -- mitigated by raising
  the thread count; the underlying "workers block on peers" pattern
  is still there.
- **Memory on mid-tier iPads** -- Phase A-4 addresses it; pressure
  handling is untested.
- **AVFoundation codec coverage** -- video effects with unusual
  codecs fail; no FFmpeg fallback on iPad.
- **Effects-grid rebuild scope creep** -- Phase B is the biggest
  unknown. Spec first (B-1) before implementation.

---

## Open questions

1. For Phase G (App Store), existing Apple Developer team? And is the
   iPad app a separate App Store record or shipped as a universal app
   alongside macOS? (Impacts bundle id and entitlements.)
