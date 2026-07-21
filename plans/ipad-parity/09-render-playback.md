# 09 · Render & Playback

> **Status:** Render & Playback is one of
> the most mature parity themes. Both apps share the same `src-core/render/`
> engine, `RenderCache`, and `OutputManager`, so the *render logic* and
> *network output* are identical — the iPad genuinely streams DDP/E1.31/
> ArtNet to controllers (`OutputManager::StartOutput`), runs the same
> multi-threaded `RenderAll`, and persists `.fseq` to disk on save. The
> surviving gaps are almost entirely **UI surface**: the iPad has no
> discrete volume-preset menu (it uses a toolbar slider instead), no
> separate Pause toolbar button (it folds pause into the play/pause
> toggle), and no "Log Render State" diagnostic. iPad now has a
> dedicated Replay-Section toolbar button + Playback-menu entry. Conversely the iPad has **richer touch idioms** —
> ruler drag-to-scrub with audible 50 ms bursts, long-press loop-region
> banding, and double-tap-a-timing-mark-to-loop — that have only rough
> keybinding analogues on desktop. The continuous transport
> position slider for coarse seeking — formerly the single most notable
> *iPad-missing* item — now ships on iPad as a transport-strip `Slider`
> (audible 50 ms scrub bursts during drag, final seek on release)
> alongside the Metal-ruler tap/scrub. Loop-region and "render just this region" exist
> on **both** platforms (desktop: `PLAY_LOOP` keybind + Replay-Section +
> Waveform "Render Selected Region"; iPad: B32/B33/B44 loop region).

> **Note (2026-07):** the Linux/Windows desktop gained a Vulkan GPU compute
> backend (`src-core/effects/vulkan/`, `HAVE_VULKAN`) mirroring the Metal
> backend's `GPURenderUtils` implementation (blur/rotozoom/transitions/layer
> blending). Apple platforms (desktop + iPad) keep the Metal backend — the
> Vulkan sources compile to nothing there, so no iPad action is needed.

## Parity scorecard

| Feature | Surface | Desktop | iPad | Gap | Priority | Ease | Feasibility | Notes |
|---|---|---|---|---|---|---|---|---|
| Play / Pause toggle | toolbar | ✅ | ✅ | parity | P1 | easy | feasible | Desktop has *separate* Play + Pause buttons; iPad uses one toggle button. Space bar toggles on both. |
| Pause (dedicated button) | toolbar | ✅ | ❌ | desktop-missing | P3 | easy | feasible | Desktop `ID_AUITOOLBAR_PAUSE` + HousePreview PauseButton; iPad folds pause into the play toggle. Cosmetic only. |
| Stop | toolbar | ✅ | ✅ | parity | P1 | easy | feasible | Resets head to 0, stops audio on both. |
| First Frame / Rewind to Start | toolbar / menu / key | ✅ | ✅ | parity | P1 | easy | feasible | Desktop toolbar + HousePreview Rewind; iPad toolbar `backward.end.fill` + Playback menu + Home key. |
| Last Frame / Jump to End | toolbar / menu / key | ✅ | ✅ | parity | P1 | easy | feasible | Desktop toolbar `ID_AUITOOLBAR_LAST_FRAME`; iPad Playback menu "Jump to End" + End key (no dedicated iPad toolbar btn). |
| Back 10 Seconds | toolbar / menu | ✅ | ✅ | parity | P2 | easy | feasible | Desktop HousePreview Rewind10 button (`SequenceRewind10`); iPad toolbar `gobackward.10` + Playback menu. |
| Forward 10 Seconds | toolbar / menu | ✅ | ✅ | parity | P2 | easy | feasible | Desktop HousePreview FForward10 button (`SequenceFForward10`); iPad toolbar `goforward.10` + Playback menu. |
| Replay Section / loop selected region | toolbar / keybind | ✅ | ✅ | parity | P2 | medium | feasible | Desktop = Replay-Section toolbar btn (`mLoopAudio=true`) + `PLAY_LOOP` keybind; iPad = `repeat` toolbar button + Playback ▸ Replay Section (⇧Space) → `viewModel.replaySection()` sets the loop region to the selection bounds and starts loop play (reuses B32/B33 plumbing). Also B32/B33 loop region + "Play Loop Region" context action. |
| Playback Speed (8 rates 0.25–4x) | menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop Audio menu radio set; iPad Playback ▸ Speed submenu. Identical 8 options. |
| Volume presets (Loud/Med/Quiet/V.Quiet/Silent) | menu | ✅ | ❌ | desktop-missing | P3 | easy | feasible | Desktop Audio-menu radio group (100/66/33/10/0). iPad replaces with a continuous slider; covers the same range. |
| Volume continuous slider | toolbar | ❌ | ✅ | ipad-missing | P3 | easy | feasible | iPad toolbar slider 0–100 (`setVolume`), shown only when `hasAudio`. Desktop uses discrete menu presets — no slider. |
| Position / seek slider | toolbar | ✅ | ✅ | parity | P2 | medium | feasible | Desktop HousePreview `SliderPosition` (0–100 scrub). iPad transport-strip `Slider` bound to `playPositionMS`/`sequenceDurationMS` (`SequencerView.swift` position-slider block) — local drag state, `scrubSeekTo` bursts during drag, final `seekTo` on release. |
| Seek via ruler/timeline drag | gesture | ✅ | ✅ | parity | P1 | easy | feasible | Both drag the playhead; iPad `TopChromeMetalGridView` tap-to-seek + drag. |
| Audio scrub (audible bursts on drag) | gesture | ❌ | ✅ | ipad-missing | P2 | medium | feasible | iPad B40 `scrubSeekTo` plays throttled 50 ms audio snippets while dragging the ruler. Desktop scrubs silently. |
| Next Frame | menu / key | ✅ | ✅ | parity | P2 | easy | feasible | iPad Playback menu "." key. Desktop via keybinding/HousePreview; both step by frame interval. |
| Previous Frame | menu / key | ✅ | ✅ | parity | P2 | easy | feasible | iPad Playback menu "," key. Desktop keybinding. |
| Render All | toolbar / menu / key | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `ID_AUITOOLBAR_RENDERALL` (⌘R); iPad toolbar `paintpalette` + Playback ▸ Render All (⌘R). Shared core `RenderAll`. 2026-07: shared `src-core/render/` scheduler reworked to suspend/requeue (plans/render-scheduler.md) — auto-applied to both apps; iPad pool sized in `iPadRenderContext::EnsureRenderEngine`, stall watchdog wired into `IsRenderDone` poll. 2026-07-14: `PixelBufferClass::GetColors`/`SetColors` (the per-node copy in/out of seqData) made serial instead of fanning out to the shared `ParallelJobPool` — `src-core/render/PixelBuffer.cpp`, so **auto-applied to iPad**; no UI or bridge surface. Output stage -76%, total render CPU -30%, byte-identical. iPad benefits at least as much: it has fewer cores, so the pool contention this removes was proportionally worse there. 2026-07-20: Video effect frame-parallel machinery added in shared `src-core/effects/VideoEffect.cpp` + `src-core/media/VideoReader*` (byte-gated; default OFF via XL_VIDEO_PARALLEL until the decoder-lock rework — see plans/render-perf/02) — **auto-applied to iPad** when enabled; the shipped serial-path decoder fixes apply to iPad unconditionally; no UI or bridge surface. |
| Batch Render (multi-sequence) | toolbar / menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop Tools ▸ Batch Render (`BatchRenderDialog`); iPad = **sequence-browser toolbar** button → `BatchRenderSheet` (NOT the Tools menu). |
| Purge Render Cache | menu | ✅ | ✅ | parity | P2 | easy | feasible | Desktop Tools ▸ Purge Render Cache; iPad Tools ▸ Purge Render Cache (`purgeRenderCache`). Shared `RenderCache`. |
| Render Selected / Loop Region | context-menu | ✅ | ✅ | parity | P2 | easy | feasible | Desktop Waveform ▸ "Render Selected Region" (`ID_WAVE_MNU_RENDER`); iPad loop-region menu ▸ "Render Loop Region" (B44). |
| Output to Lights | toolbar / keybind | ✅ | ✅ | parity | P1 | easy | feasible | Both via `OutputManager::StartOutput`. iPad streams DDP/E1.31/ArtNet only (network). See "Infeasible/restricted". |
| Render abort / cancel | internal / keybind | ✅ | ✅ | parity | P1 | medium | feasible | Desktop `AbortRender` + `CANCEL_RENDER` keybind; iPad `abortRenderAndWait`. iPad has no user-facing cancel button (auto on memory pressure). |
| Background / toggle render | preference / keybind | ✅ | ✅ | parity | P3 | medium | feasible | Desktop `TOGGLE_RENDER` keybind + `_suspendRender`. iPad: `SequencerViewModel.renderSuspended` + `toggleRenderSuspended()` (`SequencerViewModel.swift`) — while suspended the two render wrappers (`renderEffectAndTrack` / `renderRangeAndTrack`) defer instead of kicking the renderer, accumulating a `suspendedDirtyRange`; resuming renders the union (focused range when one row, `beginFreshRender` render-all when several). Surfaced as Playback ▸ Suspend/Resume Background Render (⇧⌘R, `XLightsCommands.swift`) + a command-palette entry. |
| Render-on-edit (single effect) | internal | ✅ | ✅ | parity | P1 | easy | feasible | Both auto-render the edited model via `RenderEffectForModel`. Internal, not a user menu item on either. |
| Render-on-edit (time range) | internal | ✅ | ✅ | parity | P1 | easy | feasible | Both render the affected row+range via `RenderEffectForModel(model,start,end)`. iPad `renderRangeAndTrack` (private). |
| Render on save (write .fseq) | preference / internal | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `xLightsRenderOnSave` pref; iPad always writes `.fseq` alongside `.xsq` on save. |
| Low-Definition Render | preference | ✅ | ✅ | parity | P3 | medium | feasible | Desktop pref `xLightsLowDefinitionRender`; iPad FolderConfig toggle `render.lowDefinition` → `iPadRenderContext::IsLowDefinitionRender` (`src-iPad/Bridge/iPadRenderContext.cpp:1781-1795`, reads CFPreferences `render.lowDefinition`). |
| Render progress indicator | dialog / toolbar | ✅ | ✅ | parity | P2 | easy | feasible | Desktop modal `RenderProgressDialog` (per-job gauges); iPad shows a determinate toolbar gauge with percent (`SequencerView.swift` render button), driven by `viewModel.renderProgress` polled from the bridge's `renderProgressFraction` (`SequencerViewModel.beginFreshRender`). Overall fraction, not per-job bars — sufficient on iPad. |
| Play Loop for timing mark | gesture | 🟡 | ✅ | parity | P2 | easy | feasible | iPad B92 double-tap a timing mark → loop that mark (`playLoopForTimingMark`). Desktop loops via Replay-Section but no per-mark double-tap. |
| Go To Tag (10 numbered) | menu / key | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `PLAY_PRIOR/NEXT_TAG` + numbered jumps; iPad Playback ▸ Go To Tag (Ctrl+0–9). |
| Set Tag at Play Head | menu / key | ✅ | ✅ | parity | P2 | easy | feasible | Desktop keybind; iPad Playback ▸ Set Tag (Ctrl+Shift+0–9). |
| Clear All Tags | menu | ✅ | ✅ | parity | P3 | easy | feasible | iPad Playback ▸ Clear All Tags (`clearAllTags`); desktop tag-clear. |
| Log Render State (diagnostic) | menu | ✅ | ❌ | desktop-missing | P3 | hard | feasible | Desktop Tools ▸ Log Render State (`ID_MNU_DUMPRENDERSTATE`) dumps thread-pool/render state. No iPad analogue (could write to View Log). |
| Render bell (audio cue on done) | preference | ✅ | ✅ | parity | P3 | easy | feasible | Desktop `xLightsRenderBell` pref. iPad: `bellOnRenderComplete` `@AppStorage` toggle in `FolderConfigView.swift:237` (default OFF); `beginFreshRender`'s completion poll plays a system chime (`AudioServicesPlaySystemSound(1057)`) when set (`SequencerViewModel.swift:2750`). |
| Frame interval (fps) | preference / dialog | ✅ | ✅ | parity | P2 | easy | feasible | Both read frame interval from the sequence (Sequence Settings); drives timer tick + frame-step size. |
| Model blending modes (layer mix types) | render | ✅ | ✅ | parity | P1 | easy | feasible | Shared `src-core` render path. `resources/effectmetadata/shared/Blending.json` (LayerMethodRow, LayerMorphRow, EffectLayerMix). |
| Cross-model blending (model blending toggle) | render | ✅ | ✅ | parity | P1 | easy | feasible | `src-iPad/Bridge/XLSequenceDocument.mm:711` sequenceSupportsModelBlending, :716/:720 setSequenceSupportsModelBlending; shared `src-core` render. |
| Per-layer brightness / contrast / HSV adjust at render | render | ✅ | ✅ | parity | P1 | easy | feasible | Shared `src-core` render path. `resources/effectmetadata/shared/Color.json` (Brightness, Contrast, HueAdjust, SaturationAdjust, ValueAdjust). |
| Per-layer sparkles | render | ✅ | ✅ | parity | P2 | easy | feasible | `resources/effectmetadata/shared/Color.json` SparklesRow; shared `src-core/render/PixelBuffer.cpp` sparkles. |
| Per-layer blur | render | ✅ | ✅ | parity | P2 | easy | feasible | `resources/effectmetadata/shared/Buffer.json` Blur; shared `src-core/render/PixelBuffer.cpp` Blur. |
| Per-layer RotoZoom (rotation/zoom/pivot/3D rotation) | render | ✅ | ✅ | parity | P2 | easy | feasible | `resources/effectmetadata/shared/Buffer.json` (Rotation, Zoom); shared `src-core/render/PixelBuffer.cpp`. 2026-07: fixed an OOB scatter in the shared Metal X/Y rotate kernels (`src-core/effects/metal/RotoZoomFunctions.metal`, bounds-check the rounded index) that corrupted an adjacent layer's buffer run-to-run on Per-Model group buffers — auto-applied to both apps via the shared `EffectComputeFunctions.metallib`. |
| Per-layer in/out transitions | render | ✅ | ✅ | parity | P1 | easy | feasible | `resources/effectmetadata/shared/Blending.json` In_Transition/Out_Transition; shared `src-core/render/PixelBuffer.cpp`. |
| Per-layer canvas (persistent) mode and overlay-background | render | ✅ | ✅ | parity | P2 | easy | feasible | `resources/effectmetadata/shared/Blending.json` CanvasRow; `Buffer.json` OverlayBkg; shared `src-core` render. |
| Per-layer freeze-at-frame and suppress-until | render | ✅ | ✅ | parity | P2 | easy | feasible | `resources/effectmetadata/shared/Blending.json` FreezeEffectAtFrame, SuppressEffectUntil; shared `src-core` render. |
| Per-layer chroma key | render | ✅ | ✅ | parity | P2 | easy | feasible | `resources/effectmetadata/shared/Color.json` ChromaKeyRow; shared `src-core/render/PixelBuffer.cpp`. |
| Per-layer buffer style / transform / sub-buffer | render | ✅ | ✅ | parity | P1 | easy | feasible | `resources/effectmetadata/shared/Buffer.json` BufferStyle, BufferTransform, SubBuffer; shared `src-core` render. |
| Per-layer buffer stagger | render | ✅ | ✅ | parity | P2 | easy | feasible | `resources/effectmetadata/shared/Buffer.json` BufferStagger; shared `src-core/render/PixelBuffer.cpp` Init. |
| Brightness-level (brightness-as-output) flag | render | ✅ | ✅ | parity | P3 | easy | feasible | `resources/effectmetadata/shared/Color.json` BrightnessLevelRow; shared `src-core/render/PixelBuffer.cpp`. |
| Disable/Enable render per model | render | ✅ | ✅ | parity | P2 | easy | feasible | `src-iPad/Bridge/XLSequenceDocument.mm:1027/1030/1033/1036` element render-disable; :9359/9363/9366/9370 per-effect render. |
| Dimming curve / gamma applied at output | render | ✅ | ✅ | parity | P2 | easy | feasible | Shared `src-core/render/DimmingCurve.cpp` applied in `PixelBufferClass::GetColors`; `src-iPad/App/LayoutEditorView.swift`. |
| Color/palette engine (gradients, value curves, spatial color) | render | ✅ | ✅ | parity | P1 | easy | feasible | Shared `src-core/render/RenderBuffer.h` PaletteClass + `ColorCurve.cpp` + `ValueCurve.cpp`; `src-iPad/App/ColorCurveEditor.swift`. |
| Sequence channel data buffer (SequenceData) | render | ✅ | ✅ | parity | P1 | easy | feasible | `src-iPad/Bridge/iPadRenderContext.cpp:1519` EnsureSequenceDataSized + _sequenceData; shared `src-core/render/SequenceData`. |
| Embedded/external image media at render (SequenceMedia) | render | ✅ | ✅ | parity | P2 | easy | feasible | Shared `src-core/render/SequenceMedia.cpp`; `iPadRenderContext.cpp` PurgePreviewCaches; `src-iPad/App/MediaManagerSheet.swift`. |
| GPU (Metal) / ISPC effect kernels — Tree, Shimmer, Candle perNode (2026-07) | render | ✅ | ✅ | parity | P2 | easy | feasible | Core-only, auto-applied: shared `src-core/effects/metal/` + `effects/ispc/` kernels build into `xLights-iPadLib` (Metal on-device, NEON ISPC fallback). Bit-exact vs scalar; no UI surface on either platform. |

These render-pipeline rows are shared-core features at parity by construction — both apps link the same `src-core/render/` engine.

## iPad gaps (desktop has, iPad missing)

### P3

- **Pause as a discrete control.** Desktop has a standalone Pause button
  (`ID_AUITOOLBAR_PAUSE`, `xLightsMain.cpp:793`) distinct from Play. iPad
  folds it into `togglePlayPause` (`SequencerView.swift:424`). This is an
  idiom difference, not a true capability gap — **leave as-is** unless a
  tester specifically wants a separate pause affordance.
- **Volume presets in a menu.** Desktop Audio menu has five radio presets
  (`ID_MNU_LOUDVOLUME`…`ID_MNU_SILENT`, `xLightsMain.cpp:1253–1262`). The
  iPad's continuous slider (`SequencerView.swift:543`) covers the same
  range; a menu would only add discrete snap points. **Low value.**
- **Log Render State.** Desktop Tools ▸ Log Render State
  (`ID_MNU_DUMPRENDERSTATE`, `xLightsMain.cpp:1139`) dumps render-thread
  state to the log. iPad could route the same `RenderContext` diagnostics
  into the existing View Log sheet, but it's a debug-only feature.
  **Ease: hard** (needs a new bridge method exposing the dump + wiring to
  the log sheet).
- **Render bell / completion chime.** *Landed.* `bellOnRenderComplete`
  `@AppStorage` toggle (`FolderConfigView.swift:237`, default OFF) plays a
  system chime when a render-all completes (`SequencerViewModel.swift:2750`).
- **Explicit "toggle background render" / suspend.** *Landed.* Desktop
  `TOGGLE_RENDER` keybind + `_suspendRender`; iPad
  `SequencerViewModel.renderSuspended` + `toggleRenderSuspended()`. While
  suspended the render wrappers (`renderEffectAndTrack` /
  `renderRangeAndTrack`) record a `suspendedDirtyRange` instead of
  kicking the renderer; resuming renders the union (focused range for a
  single repeatedly-edited row, full `beginFreshRender` render-all when
  several rows were dirtied). Playback ▸ Suspend/Resume Background Render
  (⇧⌘R) + command-palette entry. Auto-render stays the default; the
  toggle is for power users batching heavy edits on large shows.

## Desktop gaps (iPad has, desktop missing)

### P2

- **Audio scrub (audible bursts while dragging).** iPad B40 `scrubSeekTo`
  (`SequencerViewModel.swift:1945`) plays throttled 50 ms audio snippets
  as the user drags the ruler, so they can *hear* where they're landing.
  Desktop scrubs silently via the slider. *Desktop work:* on
  `SliderPosition` drag, call `AudioManager::PlaySegment`-style short
  bursts. **Ease: medium** (FFmpeg/SDL2 vs AVAudioEngine path differs;
  core would need a portable segment-play helper). Genuinely nice on
  desktop too.

### P3

- **Continuous volume slider.** iPad toolbar slider (0–100) vs desktop's
  five discrete presets. A desktop slider in the HousePreview transport
  would be a small UX upgrade. **Ease: easy**, **P3**.
- **Double-tap-timing-mark-to-loop (B92).** iPad `playLoopForTimingMark`
  (`SequencerViewModel.swift:2001`) sets the loop region to a mark's
  bounds and starts looping on a double-tap. Desktop can Replay-Section
  but has no one-gesture "loop just this phrase". A desktop equivalent
  could hang off a Waveform/timing-row double-click. **Ease: medium**,
  **P3** (Replay-Section + manual region select already approximates it).

## Infeasible / restricted on iPad

- **Output to Lights — USB / serial / DMX dongles.** The iPad path
  (`OutputManager::StartOutput`) works for **network** protocols only
  (DDP, E1.31/sACN, ArtNet, FPP/Player). Raw USB-serial DMX/Renard
  outputs need a USB UART the iPad can't drive; those output types are
  **infeasible** on iPad. Network output is full parity.
- **Output requires controllers pre-configured.** `toggleOutput` errors
  with "Set up controllers in desktop xLights and copy the show folder"
  when `outputCount() == 0`. Adding/editing **closed-firmware** controller
  definitions on iPad is **restricted** (IAP-gated, P3) per the firmware
  rule; open-source firmware (FPP/WLED/ESPixelStick/DDP/generic) config is
  in scope but lives under the Controllers theme, not here.
- **Log Render State** is feasible but low value (see above) — left
  desktop-only by choice, not a platform limit. (Render bell has since
  landed on iPad — see the scorecard row.)

## Recommended sequencing

1. **Position / seek slider on iPad (P2).** ✅ Done — transport-strip
   `Slider` bound to `playPositionMS`/`sequenceDurationMS`, with
   `scrubSeekTo` bursts during drag and a final `seekTo` on release.
2. **Render progress on iPad (P2→polish).** ✅ Done — determinate
   toolbar gauge fed from `renderProgressFraction` (overall fraction,
   polled in `beginFreshRender`).
3. **Audio scrub on desktop (P2, reverse parity).** Port the iPad's 50 ms
   audible-burst scrub to the desktop slider; needs a portable
   `PlaySegment` helper in `src-core/media`, which also de-duplicates the
   two scrub paths.
4. ✅ **Done — toggle/suspend background render on iPad (P3).**
   `renderSuspended` + `toggleRenderSuspended()` defer per-edit renders
   into a `suspendedDirtyRange` and render the union on resume; Playback
   ▸ Suspend/Resume Background Render (⇧⌘R) + command-palette entry.
5. **Nice-to-haves last (P3):** desktop volume slider, desktop
   double-tap-to-loop, iPad Log-Render-State into View Log, render bell.
   Defer; none block users.
