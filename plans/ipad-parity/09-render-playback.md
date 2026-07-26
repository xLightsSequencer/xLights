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
>
> **Update (2026-07-24):** fixed a Vulkan-only render-nondeterminism where
> GPU-built transition masks (Wipe, etc.) were copied on the CPU from the
> still-in-flight GPU mask buffer before its writing dispatch was fenced,
> producing nondeterministic masked-transition output. The fix binds the GPU
> mask buffer directly to the blend, which is exactly what the Metal backend
> already does (`MetalComputeUtilities.mm` blend loop) — so this brings Vulkan
> to parity with the already-correct Metal path; **no iPad change needed** and
> the Metal render path was never affected.

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
| Render All | toolbar / menu / key | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `ID_AUITOOLBAR_RENDERALL` (⌘R); iPad toolbar `paintpalette` + Playback ▸ Render All (⌘R). Shared core `RenderAll`. 2026-07: shared `src-core/render/` scheduler reworked to suspend/requeue (plans/render-scheduler.md) — auto-applied to both apps; iPad pool sized in `iPadRenderContext::EnsureRenderEngine`, stall watchdog wired into `IsRenderDone` poll. 2026-07-14: `PixelBufferClass::GetColors`/`SetColors` (the per-node copy in/out of seqData) made serial instead of fanning out to the shared `ParallelJobPool` — `src-core/render/PixelBuffer.cpp`, so **auto-applied to iPad**; no UI or bridge surface. Output stage -76%, total render CPU -30%, byte-identical. iPad benefits at least as much: it has fewer cores, so the pool contention this removes was proportionally worse there. 2026-07-20: Video effect frame-parallel machinery added in shared `src-core/effects/VideoEffect.cpp` + `src-core/media/VideoReader*` (byte-gated; default OFF via XL_VIDEO_PARALLEL until the decoder-lock rework — see plans/render-perf/02) — **auto-applied to iPad** when enabled; the shipped serial-path decoder fixes apply to iPad unconditionally; no UI or bridge surface. 2026-07-20: frame-parallel windows extended to submodel/strand-effect rows in shared `src-core/render/RenderEngine.cpp` (byte-gated 56/56, default ON, `XL_PARALLEL_SUBMODEL_ROWS=0` kill switch) — core-only, **auto-applied to iPad**; no UI or bridge surface. 2026-07-24: video decode-time scaling — decode each video file at the largest size any effect actually renders it at (a pre-render scan in shared `src-core/effects/VideoEffect.cpp` writes `src-core/media/VideoDecodeSizeRegistry`, read by `AVFoundationVideoReader` and passed to the AVFoundation bridge) instead of full native — cutting peak video render memory ~40–60%; **auto-applied to iPad**, no UI/bridge surface. **Intentional divergence:** the AA-headroom default is 3 on desktop (favour quality) vs 2 on iPad (favour memory, it's more constrained) via `#if TARGET_OS_IPHONE`, so the same sequence's video renders slightly different pixels on iPad than desktop (each deterministic run-to-run); `XL_VIDEO_DECODE_HEADROOM` overrides. Apple-only — the FFmpeg reader (unused on Apple) is not wired, and the scan is `#ifdef __APPLE__`-gated so non-Apple builds pay nothing. 2026-07-24: hardware-decode selection in shared `src-core/media/FFmpegVideoReader.cpp` now probes each candidate until one actually opens a device (was: first name FFmpeg merely recognised, so a CUDA-enabled build always chose cuda and fell back to software on non-NVIDIA hardware, never reaching qsv/d3d11va/vulkan); absent device types are written off once per session instead of re-probed per reader. **No iPad impact** — iPad is AVFoundation-only and never constructs an `FFmpegVideoReader` (`VideoReader.cpp` `#if TARGET_OS_IPHONE`); on Apple desktop the candidate list is the single entry `videotoolbox`, so selection is unchanged there and only the repeated-probe logging differs. Measured on Windows/AMD: `GreatestShow` 133s → 117s, `IntoTheUnknown` 241s → 107s, output byte-identical to software decode. |
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
| GPU (Metal) / ISPC effect kernels — Tree, Shimmer, Candle perNode (2026-07) | render | ✅ | ✅ | parity | P2 | easy | feasible | Core-only, auto-applied: shared `src-core/effects/metal/` + `effects/ispc/` kernels build into `xLights-iPadLib` (Metal on-device, NEON ISPC fallback). Bit-exact vs scalar; no UI surface on either platform. Render buffers now hand pixel storage back to the CPU vector when a GPU backend shuts down mid-session (`RenderBuffer::ReleasePixelsToCpu`, 2026.14) — core-only, auto-applied to both platforms; verified render-output-neutral (6/6 byte-identical A/B). The Metal render-cache readback that broke the per-layer single-command-queue pipeline for blurred layers is gone (`RenderEngine.cpp` GPU-resident effects skip `AddFrame`; `GPURenderUtils::HasPendingGPUWork`), and the small/narrow box-blur case now runs on-GPU when the layer is already GPU-resident (`MetalRenderBufferComputeData::boxBlur`, bit-identical to CPU) — both core-only, auto-applied to iPad; verified byte-identical (Golden 7784×193268). |
| Headless render mode (`--headless`) | CLI | ✅ | ➖ | n/a | P3 | easy | infeasible-idiom | Desktop renders `.xsq` → `.fseq` with no window and exits, driven by `HeadlessRenderContext` (`src-core/render/`), a wx-free `xLightsShowContext` sibling of the iPad's own `iPadRenderContext`. iOS has no command line, so there is nothing to mirror — but note both apps now derive from the same show-state base, which is why headless exercises the real engine rather than a stub. |
| Render performance wave (frame-parallel rows, AdvanceState, parsed-once settings) | core | ✅ | ✅ | parity | P2 | — | feasible | 2026-06/07 desktop work — frame-concurrent rendering for group / large-model / submodel-carrying rows, the `AdvanceState` split (Snowstorm, Fireworks, Strobe, Lines, Snowflakes, most VU Meter modes, GPU Candle/Circles/Meteors/Twinkle/Wave), settings + value curves parsed once per effect instead of per frame, serial per-node output copy, suspend/reschedule render jobs, and edit-triggered renders jumping ahead of Render All. All of it lives in shared `src-core/render/`, so the iPad inherits the same throughput with no iPad-side change. Output is unchanged by design (byte-identical), which is what makes it safe to inherit. 2026-07-26: the fixed 8/24-frame window + barrier was replaced by a round-robin range pool (`src-core/utils/RangeWorkPool.*`): a row registers its whole run of parallel-safe frames as one growable range, workers rotate between rows at frame granularity, and per-row frame concurrency (and therefore clone-buffer memory) is now capped explicitly instead of being implied by the window length. Core-only, **auto-applied to iPad**, no UI or bridge surface; byte-identical on all four regression sequences. The iPad should benefit at least proportionally - it has fewer cores, so the per-window straggler barrier this removes cost it relatively more. 2026-07-26 (follow-on): rows carrying a Snapshottable effect - whose per-frame simulation advance has to run serially - lost their remaining window cap too. The capture now runs as a pipeline (the owner captures a wave of frames on the real buffers while workers draw the previous wave, snapshots living in a fixed ring sized to the row's concurrency) instead of a whole-window pre-pass, and windows are kept homogeneous so Pure frames never pay a capture. Snapshot windows on Alice's Restaurant went from 2063 capped windows to 1. Core-only, **auto-applied to iPad**; byte-identical on 7 regression sequences, wall-clock neutral on desktop - the pipelining wins where a serial capture is a large share of a window, which by construction it is not for the effects that qualify as Snapshottable. |
| Effect-level speedups (Fire 3x, Meteors bucketing, Kaleidoscope, Pictures, Ripple, Galaxy) | core | ✅ | ✅ | parity | P2 | — | feasible | Shared `src-core/effects/` (CPU + ISPC) and the Metal kernels the iPad already links, so these land on iPad automatically. Fire, Kaleidoscope, Pictures, Ripple and Galaxy are byte-identical rewrites; Meteors' line-bucketing changes CPU, Metal and Vulkan in lockstep. |
| GPU render: cached transition/sparkle buffers, GPU box blur, no cache readback | core | ✅ | ✅ | parity | P2 | — | feasible | The Metal path the iPad shares gains reused transition-mask/sparkle buffers across frames, a GPU small/narrow box blur (bit-identical to CPU), and no forced GPU→CPU readback for the render cache — so effect+blur+rotozoom+blend stay in one command queue. Vulkan gets the same treatment on Linux/Windows (desktop-only backend). |
| Shader effect renders natively on Metal (GLSL→SPIR-V→MSL) | core | ✅ | ✅ | parity | P2 | — | feasible | Explicitly a **macOS/iPad** change: the Shader effect no longer needs a GL context on Apple platforms, translating GLSL via SPIR-V and caching the result (`XL_NO_NATIVE_SHADER=1` forces the old path). This is the single biggest iPad-relevant render change of the period — the iPad previously depended on the ANGLE/GLES path for shaders. Worth an iPad-side spot check on a shader-heavy sequence. |
| Text effect: rendered-text cache shared across rows/frames | core | ✅ | ✅ | parity | P3 | — | feasible | The rasterised-text cache moved off the per-RenderBuffer `EffectRenderCache` (which frame-parallel clone buffers each got a fresh, empty copy of) into a shared mutex-guarded cache with a size cap. Shared `src-core/effects/TextEffect.cpp`, so the iPad inherits it. Byte-identical output. |
| Meteors Implode/Explode: trail draw parallelised across cores | core | ✅ | ✅ | parity | P3 | — | feasible | Radial trails are emitted per meteor, bucketed by buffer row and replayed one row per thread, restoring the parallelism the axis-aligned styles get from their ISPC line kernel. Shared `src-core/effects/MeteorsEffect.cpp`, so the iPad inherits it. Byte-identical output. |
| Pictures effect: image path resolved once, not per frame | core | ✅ | ✅ | parity | P3 | — | feasible | The per-frame `FixFile`/`FileExists` resolve is skipped once `SequenceMedia` knows the image. Shared `src-core/effects/PicturesEffect.cpp`, so the iPad inherits it — and benefits more, since iOS resolves against iCloud-backed storage too. Byte-identical output. |
| Faster `.fseq` read/write (parallel block decompress, threaded zstd) | core | ✅ | ✅ | parity | P3 | — | feasible | Whole-sequence `.fseq` reads use a parallel block decompressor (~10x on multicore) and large `.fseq` saves use multi-threaded zstd. Shared `src-core`, so the iPad's save/render-to-fseq and FPP upload paths inherit both. |
| Preview graphics backend preference (OpenGL / Vulkan / Auto) | preference | ✅ | ➖ | n/a | P3 | — | infeasible-idiom | Windows/Linux gained a Vulkan preview backend (model preview, grid, waveform, colour canvases) selectable in Preferences ▸ Other, with an Auto default that picks Vulkan only when OpenGL would fall back to software. The iPad is Metal-only by construction, so there is no backend to choose. |
| Render profiling / diff tooling (`XL_RENDER_PROFILE`, `XL_GPU_SIZE_THRESHOLD`, `--fseqcmp` extras) | CLI/env | ✅ | ➖ | n/a | P3 | — | infeasible-idiom | Per-model/per-effect timing dumps, the GPU/CPU buffer-size break-even override (Metal + Vulkan, shared `src-core`, so the iPad build carries it too) and the `--fseqcmp` frame-window / PNG-diff / changed-frame-range options are developer CLI + env-var tooling with no iOS equivalent. Listed so the audit is complete, not as a work item. |

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
