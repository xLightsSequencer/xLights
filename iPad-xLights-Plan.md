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
transfers from desktop xLights (1000s of existing users, deviation is
a non-starter) while adapting individual interactions for touch. The
current SwiftUI grid (`SequencerView.swift` + `EffectBlockView.swift`)
is the replacement target, not the starting point.

1. **Layout spec: six regions, 3 rows x 2 columns.** Top chrome is
   vertically locked; left column is horizontally locked. The
   bottom-right (main grid) is the only cell that scrolls in both
   axes. The timing-tracks band is its own vertically-locked row so
   timing marks stay visible while the user scrolls through hundreds
   of model rows.

   ```
   +- View/Time --------+- Ruler + Waveform ---------------+
   | (locked both)      | (h-scrolls, v-locked)             | row 1
   +--------------------+-----------------------------------+
   | Timing headers     | Timing effects                    |
   | (locked both)      | (h-scrolls, v-locked)             | row 2
   +--------------------+-----------------------------------+
   | Model headers      | Model effects (main grid)         |
   | (h-locked,         | (h-scrolls,                       | row 3
   |  v-scrolls)        |  v-scrolls)                       |
   +--------------------+-----------------------------------+
   ```

   Two shared state observables coordinate scroll:
   - `TimelineState { hScrollMS, pixelsPerMS }` -- column 2's three
     cells bind to this; driving h-scroll in any one of them updates
     all three.
   - `RowsScrollState { vScrollPx }` -- row 3's two cells bind to
     this.

   Row 2 (timing band) grows with the number of timing elements in
   the active view, up to a cap (roughly 1/3 of available grid
   height). If a view has more timing tracks than fit, the band
   gains its own internal vertical scroll -- rare in practice but
   needs to not break.

   Upper-left region (row 1 left cell) holds the View picker
   dropdown ("Master", etc.) and a monospace time readout
   `H:MM:SS.FF / H:MM:SS.FF`. This removes the time display from the
   main toolbar (it was redundant); the toolbar keeps play/stop,
   zoom, volume, output, inspector / preview toggles.

2. **Rendering approach.** Drop the ZStack-of-`EffectBlockView`s.
   Main grid and timing grid each become a `UIViewRepresentable`
   wrapping `UIScrollView` + a custom `UIView` that draws all
   visible effects via Core Graphics in a single `draw(_:)` pass.
   Reasons:
   - SwiftUI view-graph buckles past ~100 effects; real sequences
     carry thousands.
   - Core Graphics gives pixel-accurate parity with desktop's
     `DrawEffects` (`src-ui-wx/ui/sequencer/EffectsGrid.cpp:6619`).
   - `UIScrollView` provides real pinch-zoom via `setZoomScale` +
     `viewForZooming`, and handles momentum / rubber-banding the way
     users expect.
   - UIKit gesture recognizers are cleaner than SwiftUI gestures for
     overlapping controls (row scroll vs effect drag vs edge
     resize).

   Migration to `MTKView` is a later option if CG isn't fast enough;
   none of the public API changes when that happens because all
   drawing goes through the `UIView` subclass's `draw(_:)`.

3. **Effect visual spec.** Per-effect drawing matches desktop
   line-for-line. Let `x1 = startMS * pixelsPerMS`,
   `x2 = endMS * pixelsPerMS`, `y = rowMidline`:
   - Vertical bracket: line `(x1, rowTop) -> (x1, rowBottom)` and
     `(x2, rowTop) -> (x2, rowBottom)`.
   - Horizontal centerline: line from `(x1, y)` to icon-left-edge,
     and from icon-right-edge to `(x2, y)`.
   - Icon: sprite from the existing `effectIconTexture` PNG atlas
     (already shipped for desktop OpenGL), centered at
     `((x1+x2)/2, y)`, sized to roughly `rowHeight - 6`. If the
     effect's on-screen width is below `MINIMUM_EFFECT_WIDTH_FOR_SMALL_RECT`
     the icon is replaced with a centerline dash -- matches desktop.
   - Transition bars: green rectangle at top-left spanning fade-in
     width (`T_TEXTCTRL_Fadein` seconds converted via
     `pixelsPerMS`), red rectangle at top-right for fade-out. Same
     styling as desktop's `DrawFadeHints`
     (`EffectsGrid.cpp:6899`); yellow where they overlap.
   - Line / fill colors: `effect-default` unselected,
     `effect-selected` selected, `effect-locked` if `IsLocked()`,
     `effect-disabled` if `IsRenderDisabled()`. Pull from the
     existing `ColorManager` palette -- shared with desktop so
     custom user palettes round-trip.
   - Rendered effect-background thumbnails (the per-effect textured
     fill some effects produce -- Fireworks, Pictures, Ripple --
     visible in `Screenshot 2026-04-18 at 8.11.35 AM.png`) are
     **deferred to a Phase B v2 polish pass.** Plumbing the
     per-effect `DrawEffectBackground` virtual
     (`RenderableEffect::DrawEffectBackground`) into an iPad bitmap
     cache is non-trivial: it currently writes into
     `xlColorVertexAccumulator`, which has no CG target. Ship
     without this; the bracket-and-icon look is already a massive
     upgrade over the current colored blocks.

4. **Grid metrics -- configurable, designed for future preferences.**
   All sizes live in a single struct so a future preference pane
   (post-MVP) can adjust them without touching the grid code:

   ```swift
   struct GridMetrics {
       var rowHeight: CGFloat = 24
       var selectedRowMultiplier: CGFloat = 1.5
       var timingRowHeight: CGFloat = 24
       var rulerHeight: CGFloat = 24
       var waveformHeight: CGFloat = 48
       var rowHeaderWidth: CGFloat = 180
       var edgeHandleHitWidth: CGFloat = 24
       var transitionHandleHitWidth: CGFloat = 20
   }
   ```

   Selected row grows to `rowHeight * selectedRowMultiplier`.
   Default 1.5x (36pt at `rowHeight=24`) per current sizing
   feedback; if on-device testing shows fingers can't reliably hit
   the handles, bump the multiplier (or `edgeHandleHitWidth`) in
   one place. The waveform is deliberately shorter than desktop
   (48pt vs desktop's ~80pt) to conserve vertical screen space.
   Zoom does not touch these values -- desktop parity, zoom only
   changes `pixelsPerMS`. Future preference exposure: `rowHeight`
   as small/medium/large (matches desktop's preference for same),
   `selectedRowMultiplier` as a standalone setting.

5. **Gesture set**, in priority order:
   - Tap empty grid cell -> deselect (or drop palette effect if one
     is armed in `EffectPaletteView`).
   - Tap effect -> select; row grows to selected height; edge
     handles + transition handles appear.
   - Long-press effect -> context menu (copy / paste / delete /
     lock / disable).
   - Drag center of a selected effect -> move the effect. Snaps to
     timing marks if a timing row is the active "snap source"
     (matches desktop's selected-timing-row snap behavior).
   - Drag left or right edge handle -> resize that edge; opposite
     edge anchored. Hit zone = `edgeHandleHitWidth` (24pt);
     visible only when selected.
   - Drag fade-in or fade-out handle -> adjust `T_TEXTCTRL_Fadein`
     / `Fadeout`. Hit zone = `transitionHandleHitWidth`. Visible
     only when selected.
   - Pinch on main grid -> horizontal zoom; `pixelsPerMS` clamped
     0.005..2.0. Vertical extent unchanged.
   - Multi-select via toolbar toggle -- follow-up, not in first
     landed grid.
   - Apple Pencil precision (finer edge slop, hover preview) --
     follow-up.

6. **Row header spec.**
   - **Timing rows (row 2 left):** colored background sourced from
     `ColorManager::GetTimingColor(colorIndex)` -- same palette as
     desktop. Tapping anywhere on the label toggles the element's
     `GetActive()` / `SetActive()` (controls whether vertical
     timing-mark lines draw on the grid). A filled circle at the
     left edge indicates active; an outlined circle indicates
     inactive. Long-press -> rename / delete / convert type menu
     (follow-up; first cut shows only active-toggle).
   - **Model rows (row 3 left):** display name; `[+]` / `[-]` icon
     when `layerCount > 1` (tap toggles `Element::SetCollapsed`);
     group icon when `DisplayAsType::ModelGroup`. Sub-layers
     render as indented nested rows labeled `"  Layer N"`. Tapping
     the row header selects that model for the Model Preview
     (preserves existing behavior).

7. **Bridge additions.** New ObjC++ methods on `XLSequenceDocument`
   that thin-wrap existing desktop types. No new C++ in
   `src-core/`.
   - `timingRowIndices` -> `NSArray<NSNumber*>` of row indices
     whose element type is `ELEMENT_TYPE_TIMING`.
   - `timingRowIsActive(idx)` / `setTimingRowActive(idx, bool)` --
     wraps `TimingElement::GetActive` / `SetActive`.
   - `elementIsModelGroup(idx)`, `elementLayerCount(idx)`,
     `elementIsCollapsed(idx)`, `toggleElementCollapsed(idx)` --
     drives header indicators and collapse/expand.
   - `effectFadeInMS(row, idx)` / `setEffectFadeInMS(value, row, idx)`
     + fade-out siblings. Read/write `T_TEXTCTRL_Fadein` /
     `Fadeout` in the effect's SettingsMap; writes trigger a
     targeted re-render.
   - `resizeEffectEdge(row, idx, edge: Left|Right, newMS)` -> BOOL
     -- sibling of existing `moveEffectInRow`, anchors the
     opposite edge. Validates no overlap with adjacent effects on
     the same layer.
   - `availableViews` -> `NSArray<NSString*>`, `currentViewIndex` /
     `setCurrentViewIndex(idx)` -- wraps
     `SequenceElements::GetViews` / `SetCurrentView`. Drives the
     upper-left View picker.
   - All mutating methods return `BOOL` and kick
     `renderEffectForModel` for the affected range on success.

8. **Edit actions trigger targeted re-render.** The inspector
   already calls `RenderEffectForModel(model, startMS, endMS, clear=false)`
   on a single setting change. Grid edits must do the same -- every
   mutating action re-renders the affected model(s) across the
   affected range:
   - **Add** -- render `[startMS, endMS]` on the new effect's
     model.
   - **Delete** -- render `[startMS, endMS]` with `clear=true` so
     the removed effect's output is wiped from `SequenceData`.
   - **Move / resize** -- render the union of old and new
     `[start, end]` on the model; if moved across rows, render on
     source and destination models.
   - **Transition edit** -- render the effect's own
     `[startMS, endMS]` range.
   - Multi-effect operations coalesce by model before dispatch so
     we don't queue N overlapping renders.
   The `RenderEngine` API already supports this; bridge item 7
   widens `XLSequenceDocument`'s edit surface to match.

9. **Selection-scoped preview scrub.** When an effect is selected
   and audio isn't playing, the preview windows play the effect's
   time range on loop with no audio: play-head advances
   `startMS -> endMS` at frame rate, wraps at `endMS`. Desktop
   parity -- the user sees the effect's output without committing
   to full-sequence playback.
   - Start loop on selection, stop on deselection / audio play.
   - Only drives `playPositionMS` for preview; does not move the
     sequence timeline's main play-head marker (or moves a
     distinct "preview" marker -- TBD during implementation).
   - Multi-select: loop across the union range
     `[min(starts), max(ends)]`.

10. **View picker.** Upper-left region (row 1 left cell) holds a
    SwiftUI Menu-style dropdown. "Master" is always first. Switching
    views calls `setCurrentViewIndex` -> grid re-reads row info and
    re-lays-out. Persist last-used view per sequence via
    `SceneStorage`. Writing rarely (view hasn't changed) is the
    common case; debounce.

11. **Undo manager plumbing.** `document.undoManager` through
    `SequencerViewModel`; register inverse actions for add / delete
    / move / resize / setting-change / transition-change. Keyboard:
    Cmd+Z / Cmd+Shift+Z; discoverable from the Cmd menu (Phase E-4).

**Implementation order.** Each step independently shippable and
verifiable on device:

- B-1 (this spec) ->
- B-2 six-region shell with scroll coupling, no effect drawing yet
  (verify sticky-top + sticky-left work with placeholder content)
  **[landed. Originally shipped behind a `useBetaGrid` `@AppStorage`
  toggle so testers could A/B against the old ZStack-of-
  `EffectBlockView`s grid; the legacy grid + its `EffectBlockView.swift`
  file + the `useBetaGrid` toggle have since been deleted outright
  since the legacy path was unusable. `TimelineState` now lives on
  `SequencerView` and is passed down into `SequencerGridV2View` so
  toolbar zoom buttons and in-grid pinch-to-zoom share the same
  state. Fix 1: initial load was showing an empty timeline + waveform
  until the user zoomed in several times, then they rendered
  misaligned until a scroll-to-left. Root cause was a stale
  `TimelineState.sequenceDurationMS` State property that defaulted to
  0 and only synced via `.onAppear`, so the first body eval produced
  a collapsed `contentW`; removed the mirrored State — `contentW` is
  now computed inline from `viewModel.sequenceDurationMS`. Fix 2: the
  top chrome (timeline + waveform) and the grid/timing band were
  visibly out of sync and the top chrome didn't render on initial load
  for typical multi-minute sequences. Root cause was a SwiftUI `Canvas`
  wrapped by `UIHostingController` inside `SyncedScrollView` — it does
  not render reliably at multi-thousand-pixel widths and took a
  different zoom/scroll path than the UIView-backed timing + grid
  canvases. Replaced both SwiftUI canvases with a single
  `TopChromeCanvasUIView` (Core Graphics draw, `intrinsicContentSize`)
  that mirrors `TimingCanvasUIView`/`EffectsCanvasUIView`, deleted
  `WaveformView.swift` and the legacy `TimelineRulerView`. Pinch-to-
  zoom is now installed on all three canvases via a shared
  `pinchZoomAction` on `SequencerGridV2View`, and pan scrolls
  naturally in all four regions through the outer `SyncedScrollView`s.
  Follow-up polish: waveform now renders as a filled light-blue
  polygon with a white outline (desktop `COLOR_WAVEFORM` at 130/178/207)
  rather than per-column vertical lines. The peak buffer is also
  re-sampled on zoom: `SequencerViewModel.refreshWaveformForZoom`
  targets ~1 min/max bucket per 2 pixels of rendered timeline
  (clamped 2000..400000 samples) and is called from an
  `.onChange(of: timeline.pixelsPerMS)` on the grid, debounced
  120 ms so a continuous pinch only kicks one bridge call at the
  end. Skips the reload when the current buffer is already within
  1.5x of the new target -- at high zoom you now see finer
  envelope detail, matching the desktop `WaveView` cache-per-zoom
  behavior in `Waveform.cpp`. Ruler ticks use a nice-
  interval table so e.g. a 5-second span subdivides at 1-second
  major ticks instead of producing an off-by-one "0.02" label, and
  labels are formatted `mm:ss.fff` (precision dropping to seconds
  once the major interval >= 1 s). Zoom-to-fit on sequence load:
  `fitIfNeeded` sets `timeline.pixelsPerMS` so the full duration
  fills the available content width, keyed off the duration value
  so it only runs once per sequence open (later user pinch/zoom
  isn't clobbered). The timing-band left header was previously
  wrapped in a standalone SwiftUI `ScrollView`, so dragging the
  timing headers vertically didn't move the timing-effects band
  next to it; both panes are now bound to a shared
  `timingScroll: RowsScrollState` via `SyncedScrollView`, so
  vertical scroll in either pane keeps them aligned. The "Start
  Output" toolbar button was removed for the MVP -- controller
  output isn't in scope for initial ship. Canvas virtualization:
  the ruler+waveform, timing-effects, and model-effects canvases
  would previously blow past Metal's ~16k texture limit at high
  zoom (CoreAnimation logged "Ignoring bogus layer size
  (26013, 3096)" then blanked the view). CATiledLayer was tried
  first and crashed with `_dispatch_assert_queue_fail` on
  `com.apple.coreanimation.imageprovider.concurrent` — our mix of
  CG + UIKit text drawing isn't background-safe. Settled on manual
  horizontal tiling: each canvas UIView is a gesture-owning
  container that, in `layoutSubviews`, creates/reuses a set of
  `CanvasTileView` subviews (≤ 4096 logical px wide). Each tile
  translates CG by `-tileOriginX` in its `draw(_:)` and proxies
  back to the parent's `drawContent(cg:worldRect:)` — drawing code
  keeps using world-x coords unchanged. Tiles set
  `isUserInteractionEnabled = false` so taps/drags still hit the
  parent. Per-tile dirty invalidation layered on top:
  `ModelEffectsCanvas`/`TimingEffectsCanvas` `updateUIView` diffs
  old vs new state — structural changes (row count, effect count,
  zoom, timing marks) still do a full invalidate, but per-effect
  edits (move/resize/rename) compute the union x-range of each
  slot's old+new extent and call a new `invalidate(xRanges:)` that
  sets `setNeedsDisplay(_ rect:)` on just the overlapping tiles.
  Selection change invalidates old + new selected effect extents
  only. The drag-loop in `EffectsCanvasUIView.onPan` does the same
  thing per frame — invalidates the union of previous-frame and
  current-frame live range, so each drag tick touches 1–2 tiles
  instead of all 150 of them at max zoom.]** ->
- B-7 bridge additions (timing rows, layer counts, views, resize,
  fades) **[landed; `XLSequenceDocument` gained `timingRowIndices`,
  `timingRowIsActive`/`setTimingRowActive`, `rowIsModelGroup`,
  `rowLayerCount`, `rowIsElementCollapsed`/`toggleElementCollapsed`,
  `availableViews` / `currentViewIndex` / `setCurrentViewIndex`,
  `effectFadeInSeconds` / `effectFadeOutSeconds` with setters,
  `resizeEffectEdge`]** ->
- B-6 row headers (timing + model, active toggle, collapse/expand)
  **[landed as `RowHeaderViews.swift`; `TimingRowHeader` toggles
  `TimingElement::GetActive()` on tap; `ModelRowHeader` shows group
  icon for ModelGroups + collapse button when layerCount > 1 + tap
  selects the model for the preview pane. Follow-up polish: timing
  row backgrounds now draw the desktop Timing1..Timing5 palette
  (cyan / red / green / blue / yellow, darkened for legible white
  text) derived from `SequenceElements::RowInformation::colorIndex`
  via new bridge methods `timingRowColorIndexAtIndex`,
  `timingRowElementNameAtIndex`, `rowLayerNameAtIndex`. The
  per-element active-dot + tap handler only render on the first
  layer (`layerIndex == 0`), matching desktop behavior for lyric
  tracks with phrase / word / phoneme sub-layers. `SequencerViewModel`
  gained a `TimingRowInfo` sub-struct on `RowInfo`, and
  `reloadRows()` falls back to the element name for non-collapsed
  multi-layer timing rows that have empty `displayName`, producing
  "[N] LayerName" labels for higher layers. Model row headers now
  mirror the timing-row layer convention: the first-layer row
  shows "`ModelName [N]`" when the element has more than one layer
  (N = total layer count), and sub-layer rows show just
  "`[layerIndex+1]`" — matching the bracket notation used on the
  timing side for phrase/word/phoneme rows.]** ->
- B-3 effect drawing (brackets + icons + transitions + vertical
  timing-mark lines) **[landed as `EffectCanvasViews.swift`;
  `ModelEffectsCanvas` draws brackets + centerline + icon +
  green/red/yellow fade bars + vertical timing lines from active
  timing rows; `TimingEffectsCanvas` draws mark brackets + labels.
  Real effect icons now render from the desktop's compiled-in XPM
  data: new bridge method
  `iconBGRAForEffectNamed:desiredSize:outputSize:` on
  `XLSequenceDocument` looks up `RenderableEffect` via
  `iPadRenderContext::GetEffectManager()`, pulls
  `GetIconData(sizeIdx)` for the appropriate bucket
  (16/24/32/48/64), and parses the XPM char-array into a
  BGRA-premultiplied byte buffer. Swift-side `EffectIconCache`
  singleton wraps it in a `CGImage` cached per
  (effect-name, bucket). `drawEffect` blits the bitmap with a CTM
  flip (CG bottom-up vs our top-down context) and falls back to
  the 3-letter abbreviation only when an effect's icon can't be
  loaded. Timing-effect grid now draws two alternating band
  colors keyed on `row.timing?.elementName` rather than row index,
  so all layers of a single timing track share one stripe and the
  next track takes the other. Per-effect rendered background
  thumbnails (desktop's `RenderBuffer` preview blit) still
  deferred to a v2 polish pass]** ->
- B-10 view picker **[landed; top-left corner renders a SwiftUI
  `Menu` listing `SequenceElements::GetViews()`; selecting a view
  calls `setCurrentViewIndex` and reloads rows]** ->
- B-5 gesture set (tap, drag, edge resize, transition handles, pinch
  zoom) with B-8 targeted re-render wired in **[landed on
  `EffectsCanvasUIView`: tap selects/deselects, pan-from-center-of-
  selected-effect moves, pan-from-edge-of-selected-effect resizes,
  pinch zooms `pixelsPerMS` with anchor-under-finger preservation.
  All mutating actions call `renderEffect(forRow:at:)` for targeted
  re-render. Long-press on an effect selects it and fires
  `onRequestContextMenu`, which the grid shell presents as a
  `.confirmationDialog` with Copy / Paste Here / Lock-Unlock /
  Disable-Enable / Delete / Cancel. Only fade-handle drag gestures
  remain deferred]** ->
- B-9 selection-scoped scrub **[landed; `SequencerViewModel.startScrub`
  runs a frame-interval timer that loops `playPositionMS` over the
  selected effect's range. Starts on `selectEffect`, stops on
  `clearSelection` or real playback. No audio, preview-only.
  Follow-up fix: the Model Preview pane was freezing on the first
  scrub frame because `PreviewPaneView.updateUIView` only started
  its `CADisplayLink` while `viewModel.isPlaying == true`.
  `SequencerViewModel` now exposes an observable `isScrubbing`
  flag toggled by `startScrub`/`stopScrub`, and the preview pane
  runs its display link when `isPlaying || isScrubbing` — the
  scrub loop actually animates in the pane now. `selectEffect`
  also calls a new non-toggling `setPreviewModel(rowIndex:)` that
  routes the Model Preview to the effect's own model (the
  existing `selectPreviewModel(rowIndex:)` from the row header
  still toggles on re-tap, which would defeat scrub if reused
  here)]** ->
- B-11 undo **[landed; `SequencerViewModel` gained a `@MainActor`
  isolation + an `UndoManager`. Move / resize / add / delete / lock /
  disable register inverse actions via the `UndoManager`; add/delete
  capture the full settings+palette string (via new
  `addEffectWithSettings` path) so undoing a delete recreates the
  effect with its original settings intact. Toolbar buttons +
  Cmd+Z / Cmd+Shift+Z keyboard shortcuts invoke undo/redo.
  Setting-change (inspector) undo wiring is still a follow-up]**.

B-3 v2 (rendered effect backgrounds) is a final polish pass once all
of the above is stable.

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
   waits on Phase B-9). Desktop always has two previews alongside
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
     Phase B-9.
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
   is how users create and edit the named Views that Phase B-10
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
