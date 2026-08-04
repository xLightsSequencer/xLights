# Windows Render Performance — opening analysis, 2026-08-03

First measured look at Windows render performance. Until now the standing
policy (`plans/render-perf.md`) was **"Mac/Metal is 100% of the goal, Windows =
correctness only"**; this document opens the Windows work and is where its
findings and item list live.

Read [`plans/render-benchmarking.md`](render-benchmarking.md) first — the
methodology, env vars and correctness gate it describes apply unchanged here.
The one Windows-specific harness note: a remote shell lands in PowerShell 5.1
with a restrictive execution policy, so scripts need
`powershell -ExecutionPolicy Bypass -File`, and `&&` is a parse error (use `;`).

---

## 1. The measurement

| | macOS | Windows |
|---|---|---|
| Machine | M4 Max, 16 cores, 128 GB | Ryzen 5 7640HS, 6c/12t, 29.5 GB |
| GPU | Metal (unified memory) | Radeon 760M iGPU, Vulkan compute |
| Commit | `2ae193f40` | `2ae193f40` |
| Corpus | 56 sequences | same 56 sequences |
| Sum of render times | **227.83s / 229.26s** | **699.99s / 698.14s** |
| Determinism | — | **56 / 56 byte-identical** |
| Run-to-run spread | 0.6% | 0.26% |
| CPU utilisation | 4-6 of 16 cores | 3.09 of 12 cores |

**Windows / macOS = 3.07x.** That ratio is the yardstick used throughout: a
stage or effect *above* 3.07x is a Windows-specific loss, *below* it is a
Windows win. The two machines are not comparable in absolute terms, which is
precisely why nothing below is quoted as an absolute.

### The comparator was validated before anything was concluded from it

`XL_GPU_STATS=1` on one sequence: `blur=636 transitions=29440 blend=31440
effect=24043 setup=1950 blurcall=4596`. Vulkan compute is doing real work.
This check is not ceremony — the headless GL shader path once reported ~2.5x
*faster* on Windows by rendering solid cyan, and an entire investigation was
aimed at a gap that did not exist.

Likewise, several sequences are **faster on Windows** than on the M4 Max
(one CPU-effect-bound sequence at 5.15s vs 9.04s, and three other CPU-effect-bound sequences at roughly parity). That is the same signature the cyan-shader bug produced, so it
was checked: that sequence contains **zero** Shader effects. The sub-1x
ratios are real, and they matter — **Windows is not uniformly slower**, so any
theory explaining the gap with one platform-wide cause is wrong.

---

## 2. Where the time goes

### By stage (all 56 sequences, `XL_RENDER_PROFILE=1`)

| Stage | macOS | Windows | ratio |
|---|--:|--:|--:|
| blurZ (`HandleLayerBlurZoom`) | 14.0s | 92.4s | **6.58x** |
| setCol (seqData -> layer gather) | 44.8s | 280.8s | **6.27x** |
| trans (`HandleLayerTransitions`) | 3.6s | 19.0s | **5.31x** |
| getCol (layer -> seqData scatter) | 48.7s | 199.2s | **4.09x** |
| suspend | 1928.8s | 7351.9s | 3.81x |
| effect (CPU in `Render`) | 287.6s | 753.9s | 2.62x |
| blend (`CalcOutput`) | 365.3s | 780.9s | 2.14x |
| gpuWait | 60.3s | 90.8s | 1.51x |
| **gpu execution** | 68.2s | 63.3s | **0.93x** |

**The 760M is not the problem.** GPU execution is at parity with Metal on an
M4 Max. Neither is the blend, nor effect CPU — both beat the 3.07x yardstick.

**The output stage is.** `getCol` + `setCol` = **480s on Windows vs 93s on
macOS**. Before calling that a Windows defect: both are bandwidth-bound memory
shuffles and an M4 Max has roughly 5x this APU's memory bandwidth, so much of
the gap is hardware. The actionable consequence is not "fix Windows" but
**output-stage algorithmic work is worth far more on Windows than its macOS
measurement implied** — `plans/render-perf/05-output-node-map.md` was shelved on
Mac evidence that no longer represents the whole product.

### By effect (all 56 sequences)

Ranked by Windows `cpu+gpu`. Compare `us/render`, not the totals: per-effect
totals under-report whenever frame-parallel is engaged, and the owner thread's
share of frames differs between platforms.

| Effect | macOS | Windows | ratio | us/render mac -> win |
|---|--:|--:|--:|---|
| **Video** | 104.3s | **374.6s** | **3.59x** | 679 -> 2438 |
| Pinwheel | 16.4s | 40.1s | 2.45x | 68 -> 168 |
| Morph | 14.3s | 30.2s | 2.11x | 26 -> 54 |
| Pictures | 8.5s | 26.4s | 3.12x | 32 -> 98 |
| Shader | 29.7s | 26.1s | **0.88x** | 835 -> 734 |
| (gpu blend) | 23.8s | 24.1s | 1.01x | — |
| Fire | 7.5s | 24.1s | 3.22x | 148 -> 478 |
| Faces | 8.4s | 21.5s | 2.57x | 33 -> 84 |
| Strobe | 8.0s | 20.9s | 2.63x | 25 -> 69 |
| Text | 4.1s | 20.7s | **5.10x** | 18 -> 92 |
| Shockwave | 15.2s | 20.4s | 1.34x | 39 -> 52 |
| Meteors | 2.8s | 19.5s | **6.84x** | 12 -> 80 |
| SingleStrand | 17.9s | 19.4s | 1.09x | 21 -> 22 |
| VU Meter | 9.9s | 15.2s | 1.54x | 29 -> 45 |
| Fan | 8.7s | 14.2s | 1.63x | 49 -> 80 |
| Lines | 16.8s | 12.5s | **0.74x** | 1043 -> 775 |
| Snowflakes | 2.9s | 11.9s | 4.12x | 12 -> 50 |
| Shape | 2.7s | 11.8s | 4.30x | 137 -> 589 |
| Twinkle | 4.3s | 10.8s | 2.52x | 21 -> 52 |
| Spirals | 7.5s | 7.9s | 1.05x | 25 -> 26 |

**Video at 374.6s is more than half the 700s corpus render wall**, at 2.4 ms
per render. It is the single dominant Windows cost by a wide margin, and the
rest of this document is mostly about it.

Shader (0.88x) and Lines (0.74x) are faster on Windows. SingleStrand (1.09x),
Spirals (1.05x) and Shockwave (1.34x) are near parity — all effects with
16-wide `avx2-i32x16` ISPC kernels, which Apple Silicon does not get.

---

### GPU compute is a large net win here — but not for every sequence

Full corpus, same binary, GPU compute on vs `XL_NO_GPU_COMPUTE=1`:

| Config | Total |
|---|--:|
| GPU on (default) | **699.99s** |
| GPU off | **878.87s** (+25.6%) |

There are field reports that GPU rendering is *slower* on Windows. They do not
reproduce as a whole-corpus effect — but they are not wrong either. **15 of 56
sequences are faster with the GPU off**, and someone whose show looks like
those would correctly perceive GPU rendering as a regression:

| faster with GPU off | on | off | | faster with GPU on | on | off |
|---|--:|--:|---|---|--:|--:|
| the smallest-buffer sequence | 6.60 | 5.24 (0.79x) | | a mid-size GPU-heavy sequence | 9.79 | 63.02 (6.44x) |
| small-buffer seq A | 12.90 | 11.56 (0.90x) | | a GPU-heavy sequence | 40.66 | 70.75 (1.74x) |
| small-buffer seq C | 11.57 | 10.39 (0.90x) | | a non-video sequence B | 19.58 | 37.11 (1.90x) |
| small-buffer seq D | 1.34 | 1.09 (0.82x) | | a GPU-sensitive sequence | 6.16 | 13.91 (2.26x) |

Losses are small and bounded (worst 1.37s); wins are large (+53s on one
sequence). The signature — consistent small losses on small work, large wins on
large work — is per-dispatch overhead below some buffer size, which is what
`XL_GPU_SIZE_THRESHOLD` exists to control.

**Caveat: this box is an integrated Radeon 760M on shared memory, so there is
no PCIe transfer at all.** The reports may well be about discrete cards, where
transfer cost is real. That remains unmeasured — the fleet has no
discrete-GPU Windows machine (the other Windows box is an Intel N95 with UHD
graphics, also integrated).

### The shipped GPU threshold is mistuned for Vulkan-on-AMD

Sweep over a subset deliberately containing **both** the GPU losers and the big
GPU winners, so a threshold that helps one side by wrecking the other cannot
look like a win:

| Sequence | gpu-off | 2048 (ship) | 4096 | 8192 | 16384 |
|---|--:|--:|--:|--:|--:|
| the smallest-buffer sequence | 3.35 | 3.67 | 3.40 | 3.41 | 3.38 |
| small-buffer seq C | 8.49 | 9.77 | 8.90 | 8.63 | 8.57 |
| small-buffer seq B | 16.42 | 17.09 | 16.75 | 16.45 | 16.12 |
| small-buffer seq A | 11.26 | 12.91 | 12.37 | 12.54 | 11.82 |
| a mid-size GPU-heavy sequence | 60.29 | 8.98 | 9.01 | 8.87 | 9.03 |
| a non-video sequence B | 35.99 | 18.52 | 18.43 | 18.37 | 18.52 |
| a GPU-heavy sequence | 66.28 | 34.69 | 34.54 | 34.58 | **37.43** |
| subset total | 213.10 | 116.61 | 114.44 | **114.05** | 116.09 |

The small-buffer losers recover most of their loss as the threshold rises while
the winners stay flat. **16384 is too far** — Party Rock regresses +8%.

Full corpus at 8192: **690.89s vs 699.07s mean baseline = -1.2%.** Real (the
noise floor is 0.26%) but modest; the subset overstated it because it was
deliberately loser-weighted.

Two things to weigh before changing the default:

- The corpus win is small. Its value is mostly in **removing a class of
  user-visible "GPU made it slower" regressions**, not in the 1.2%.
- Moving the threshold moves effects between the GPU and CPU kernels, so it
  **changes output** within the documented GPU-vs-CPU tolerance. It is not a
  byte-identical change and must not be gated as one.
- 2048 was presumably tuned on Metal. This measurement says nothing about
  whether Metal wants to move; it should not.

---

## 3. Video: the capability gap, from the code

macOS routes `VideoReader` to `AVFoundationVideoReader` and thence to
`AVFoundationVideoBridge.mm` — 4366 lines of shared decoders, decode lanes,
playback corridors, a frame cache, read-ahead and a scaled-output cache.
Windows routes it to `FFmpegVideoReader`, which has none of that.

| Capability | macOS AVFoundation | Windows FFmpeg (in use) | Windows MF (exists, unreachable) |
|---|---|---|---|
| Decode-time scaling | yes | **no** — decodes native, then per-frame `sws_scale` | **yes** — sets `MF_MT_FRAME_SIZE` to the target |
| `SupportsFrameIndependentAccess` | yes | **no** (never overridden) | no — serial `ReadSample` |
| `Resize()` in place | yes | **no** — delete + reopen the reader | possibly, via `SetCurrentMediaType` |
| Shared decoder / corridors / frame cache | yes | **no** — one reader per Video effect | no |
| Hardware decode | VideoToolbox | d3d11va via FFmpeg hwaccel | D3D11VA via Media Foundation |

Consequences on Windows today: every Video effect init opens a fresh decoder,
decodes frame 0 and seeks (`VideoEffect.cpp:679-692`); frame-parallel video is
off regardless (`VideoEffect.cpp:105-120`, `XL_VIDEO_PARALLEL`); and the
decode-size pre-pass that saved 8.8% of total corpus render time on macOS
(`0eb7362e2`) is compiled out entirely — `RenderEngine.cpp:3820` wraps
`VideoEffect::PrepareDecodeSizes` in `#ifdef __APPLE__`.

### The Media Foundation reader already exists

`src-core/media/WindowsHardwareVideoReader.cpp` (1236 lines) is a Media
Foundation + D3D11 hardware reader, reached from `FFmpegVideoReader`'s
constructor (`FFmpegVideoReader.cpp:163-189`) — but only when the video
renderer preference is `DIRECX11_API` (0). The default, and the setting on the
test box, is `FFMPEG_AUTO` (1), so **it never runs**. It already asks Media
Foundation to emit frames at the target size (`WindowsHardwareVideoReader.cpp:396`),
i.e. it already does the decode-time scaling macOS gained in `0eb7362e2`, GPU-side.

It is also structured for exactly the try-then-fall-back shape a native reader
needs: `IsOk()` false deletes it and falls through to FFmpeg. That fallback is
**mandatory**, not optional — Media Foundation covers H.264, HEVC, VP9, WMV and
(Win11) AV1, but has no Matroska/`.mkv` demuxer at all and only partial AVI and
MPEG-4-part-2 support, where FFmpeg handles all of them.

### Measured: Media Foundation is 34% faster on the video sequences

A/B on the 13 video-bearing sequences, flipping only
`xLightsVideoReaderRenderer` (1 = `FFMPEG_AUTO`, 0 = `DIRECX11_API`):

| Sequence | FFmpeg | MF | delta |
|---|--:|--:|--:|
| **the 58-video sequence** | 115.38 | **49.65** | **-65.7s (2.32x)** |
| **the 44-video sequence** | 98.70 | **39.31** | **-59.4s (2.51x)** |
| a 1-video sequence (2) | 18.40 | 10.95 | -7.5s |
| a 1-video sequence | 27.44 | 23.14 | -4.3s |
| a 3-video sequence | 9.91 | 7.78 | -2.1s |
| a GPU-heavy sequence | 34.35 | 32.60 | -1.7s |
| a 10-video sequence | 12.81 | 12.66 | -0.2s |
| a rawvideo-only sequence (2) | 9.87 | 9.81 | ~0 |
| a mid-size GPU-heavy sequence | 8.89 | 9.15 | +0.3s |
| a rawvideo-only sequence | 13.31 | 13.89 | +0.6s |
| a short 1-video sequence | 3.44 | 3.63 | +0.2s |
| rawvideo-mixed seq A | 12.59 | 14.76 | +2.2s |
| rawvideo-mixed seq B | 27.27 | 29.90 | +2.6s |
| **TOTAL** | **392.36** | **257.23** | **-135.13s (-34.4%)** |

Against the 700s corpus that is roughly **-19% overall**, from a preference
flip on code that already ships.

The path was proven to actually engage rather than assumed: 198 `WHVD` log
lines with renderer=0 vs **0** with renderer=1, and FFmpeg's own hardware-device
probes dropped from 11 to 1.

### Why the regressions happen, and why the coverage is favourable

`ffprobe` over the **43 video files actually referenced by the sequences**
(not the 71 on disk — the 20 `.avi` files are dead leftovers referenced by
nothing):

| Codec | Count | MF | Note |
|---|--:|---|---|
| h264 | 32 | yes | includes 3840x2160 files and a 378 MB clip |
| hevc | 1 | yes | |
| **rawvideo** | **10** | **no** | **all 128x96 or 96x48** |

MF covers **33 of 43 (77%)**, and — the important part — it takes every
*expensive* file and rejects only free ones. Uncompressed 128x96 rawvideo has
no decode work at all, just I/O, which FFmpeg already does cheaply.

That explains every regression in the table: `a rawvideo-only sequence`
(`a 128x96 rawvideo clip`) and `a rawvideo-only sequence (2)` (`a 96x48 rawvideo clip`)
are pure-rawvideo sequences. They came out **byte-identical** to the FFmpeg
run — the tell that MF never decoded them at all — so they gain nothing and pay
only the MF probe. Note the symmetry: those same rawvideo files take the
`isRawDemux()` path on macOS that forces `IsFrameIndependent` false, so they are
the serial case on both platforms for the same reason.

The probe cost looks cheap to remove: **cache the "MF cannot open this path"
verdict per file** so it is paid once instead of once per Video-effect init —
the same shape as the existing `s_absent` hardware-device cache
(`FFmpegVideoReader.cpp:337-368`). With 44 Video effects on one sequence
hitting a handful of files, that should recover most of the +2.6s.

### Why MF output differs: it is ~20% brighter, and it never snaps near-black

11 of 13 sequences produced different channel data. `--fseqcmp` **cannot** be
the gate (different decoder implies different frames), but it localises the
difference, and the answer is specific and fixable.

On `the 58-video sequence`, 27% of samples differ, `maxAbsDiff=255`, 130087 of 193268
channels touched. The frame-offset probe puts the minimum at shift 0, so the
two are **frame-aligned** — this is not a timing bug. Dumping one channel
(`XL_FSEQCMP_DUMPCH`, A = FFmpeg, B = MF):

```
fr  5  A=  9  B= 11      fr 25  A= 40  B= 46
fr 10  A= 18  B= 21      fr 30  A= 48  B= 60
fr 20  A= 32  B= 37      fr 40  A= 62  B= 77
```

**Mean B/A = 1.20** (1.196 for A>=20), and both agree at zero. A systematic,
near-constant brightening — not noise, not dropped frames. (These are
post-render, post-blend channel values rather than raw decoded pixels, so the
factor is indicative of the mechanism, not a clean decoder ratio.)

Two independent causes, both visible in the source:

1. **No colour-range or matrix control.** `MF_MT_VIDEO_NOMINAL_RANGE`,
   `MF_MT_YUV_MATRIX` and `MF_MT_TRANSFER_FUNCTION` appear **zero times** in
   `WindowsHardwareVideoReader.cpp`, so MF's Video Processor picks the YUV->RGB
   mapping itself. H.264 is normally limited range (16-235); the limited->full
   expansion factor is 255/219 = 1.164, in the same family as the measured 1.20.
2. **No near-black snap.** `BitmapFromSample`
   (`WindowsHardwareVideoReader.cpp:551-605`) is a bare channel-swap `memcpy`.
   The macOS bridge deliberately snaps pixels whose **all three** RGB channels
   are `<= 4` to exact `(0,0,0)` (`AVFoundationVideoBridge.mm:3636-3656`),
   because H.264 artifacts in dark regions produce values like `(2,0,2)` —
   sum 4, one over a typical `TransparentBlack` threshold of 3. FFmpeg's
   swscale bicubic clips those during scaling; other scalers preserve them.
   The all-three-channels test (rather than a sum) is deliberate, so
   intentionally-dark single-channel content like `RGB(0,0,10)` survives.

This is exactly why the MF path is not on by default. It matters most for
composite content: on a curtain effect, large regions **must** decode to pure
black or the layers below blend blocky, and a 20% lift plus unsnapped
near-black turns transparent regions opaque.

There is a third, related trap the macOS work already hit and documented: the
scaler's colour space. vImage was chosen because it scales in **encoded sRGB
byte space**, matching swscale; CIContext and VTPixelTransferSession scaled in
linear light, brightening non-black pixels ~10% and shifting near-black away
from 0 (`AVFoundationVideoBridge.mm:3881-3889`). MF's advanced-video-processing
scaler is a third scaler with its own unstated space and is exposed to the same
trap — so fixing nominal range alone may not close the gap.

Still unexplained: **55 `GetNextFrame failed`** on files MF *did* open. Those
are a separate question from the brightness and must also be resolved before
this ships.

### DIAGNOSED (2026-08-03): the video processor scales in the wrong colour space

**The cause is established, and it is the same thing that is making MF fast.**
`MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING` inserts a Video Processor
MFT that does YUV->RGB *and* the downscale on the GPU. That GPU convert+scale
is the entire 2.32x speedup — and the entire colour error.

Proof: take the scaling away from it and the colour converges. With MF asked
for native-size frames and swscale (SWS_BICUBIC, the same scaler
`FFmpegVideoReader` uses) doing the downscale, MF/FFmpeg drops from **1.177 to
1.056** on the sampled channel.

| Config | the 58-video sequence | MF/FFmpeg brightness |
|---|--:|--:|
| FFmpeg (reference) | 115.38s | 1.000 |
| MF, processor scales (current) | **49.29s** | **1.177** |
| MF, native + swscale (`XL_MF_SWSCALE=1`) | 211.39s | **1.056** |

So the fix cannot be "let swscale do it" — that is *slower than FFmpeg itself*
(211s vs 115s), because we then pull full-size RGBA to the CPU, where FFmpeg's
d3d11va path pulls half-size NV12. **Contrast with FFmpeg's hardware path:
d3d11va/d3d12va decode only** — `av_hwframe_transfer_data`
(`FFmpegVideoReader.cpp:739`) brings NV12 to system memory and swscale converts
on the CPU. That is why software and d3d11va decode measured byte-identical
here; the hwaccel cannot shift colour because it never touches colour.

### Four attribute-based fixes were tried. All four failed.

The processor ignores every colour hint offered through media-type attributes:

| Attempt | Result |
|---|---|
| `MF_MT_VIDEO_NOMINAL_RANGE = 0_255` on the **output** type | accepted, **zero** change |
| Manual studio->full expansion `(v-16)*255/219` in `BitmapFromSample` | **worse** — 24.22% -> 27.59% differing; MF is not emitting studio range |
| `MF_MT_VIDEO_NOMINAL_RANGE` + `MF_MT_YUV_MATRIX` on the **decoder output** (input) type | accepted, **zero** change |
| `MF_MT_TRANSFER_FUNCTION = 709` on both input and output | accepted, **zero** change |

Measured source metadata, for the record: `nominalRange=-1 yuvMatrix=-1
transferFn=-1 primaries=-1` — the files declare nothing, so the processor
guesses, and it will not accept being told otherwise
(`XL_MF_COLOR_DEBUG=1` dumps this).

### BUILT (2026-08-03): direct ID3D11VideoProcessor path — fastest yet, colour still unsolved

Implemented behind `XL_MF_D3DVP=1`: MF is used for **decode only** (NV12 at
native size, surface stays on the GPU), then `ID3D11VideoProcessor` does
convert+scale with `VideoProcessorSetStreamColorSpace` /
`SetOutputColorSpace` set explicitly, driver auto-processing (denoise,
sharpening) disabled, and only the small target-size RGB frame read back.

Verified running, not assumed: **58 `WHVD VP: ready` lines, 0 failures**, e.g.
`ready 1920x1080 -> 800x336, matrix BT.709`. (An earlier check looked clean but
could not have failed — the success line was `spdlog::debug`, which headless
filters out. Promoted to `info` before trusting it.)

**Speed — the best of every option, by a wide margin:**

| Path | the 58-video sequence |
|---|--:|
| FFmpeg (current default) | 115.38s |
| MF advanced video processing | 49.29s |
| **MF decode + ID3D11VideoProcessor** | **41.70s** |
| MF native + swscale | 211.39s |

**2.75x faster than FFmpeg**, and 15% faster than MF's own processor — the
readback is smaller and the driver's enhancement passes are off.

**Colour: not solved.** Brightness ratio against the FFmpeg path, sampled over
100 frames on several channels:

| channel | MF auto | D3DVP (BT.709 heuristic) | D3DVP (BT.601 default) |
|---|--:|--:|--:|
| 38913 | 1.228 | 1.065 | **1.022** |
| 102433 | 0.954 | 0.844 | 0.798 |
| 60000 | 1.128 | 1.308 | 1.308 |
| mean deviation from 1.0 | **0.134** | 0.177 | 0.177 |

So the direct processor is **differently** wrong, not less wrong. Three things
this rules in or out:

- **It is per-file, not a global constant.** Ratios span 0.80-1.31 across
  models in one sequence.
- **Nobody declares the metadata.** Neither the source type nor the decoder's
  NV12 output carries `MF_MT_VIDEO_NOMINAL_RANGE` or `MF_MT_YUV_MATRIX`
  (measured `-1` on all 58 opens), so there is no ground truth to read - only
  heuristics.
- **At least two independent causes.** Switching the undeclared-matrix default
  from BT.709 to BT.601 (which is what swscale falls back to) moved ch 38913
  materially (1.065 -> 1.022) and left ch 60000 completely unchanged (1.308).
  One knob cannot explain both.

### SOLVED (2026-08-03): all three decoders now agree, and the default got faster

Once the measurement moved to **raw decoded pixels** (`XL_VIDEO_DUMP`, see
below) the picture inverted: MF was never the outlier — **FFmpeg was**, and its
three defects were all fixable with swscale settings we had simply never set.

`a 1280x720 h264 clip` @16ms, 1280x720 -> 128x96, raw pixels vs macOS/AVFoundation:

| stage | meanAbs | \|d\|>4 | ratio | exact-black px |
|---|--:|--:|--:|--:|
| ffmpeg baseline | 3.601 | 29.4% | 0.9246 | 15 |
| + `sws_setColorspaceDetails` | 2.914 | 5.3% | 0.9326 | 9 |
| + near-black snap | 2.916 | 5.5% | 0.9326 | 107 |
| **+ SWS_AREA for big downscales** | **1.102** | **0.3%** | **0.9861** | **59** |
| mfauto | 1.549 | 6.2% | 0.9993 | 65 |
| mfvp | 2.111 | 9.2% | 0.9888 | 67 |
| **macOS (reference)** | — | — | 1.0000 | **61** |

Three independent FFmpeg defects, all now fixed in `FFmpegVideoReader`:

1. **`sws_setColorspaceDetails` was never called.** swscale defaulted to BT.601
   coefficients and did not emit full-range RGB. Setting the matrix from the
   frame (BT.709/BT.601, resolution fallback when the bitstream is silent) and
   `dstRange=1` cut large errors from 29.4% to 5.3%.
2. **No near-black snap.** macOS has had one for years; FFmpeg produced 15
   exact-black pixels where macOS produced 61. Ported the same all-three-
   channels-<=4 predicate.
3. **Bicubic at a 10:1 reduction.** The real level error. Bicubic samples a few
   taps out of each 10x10 source block and aliases; `SWS_AREA` averages
   properly, which is what vImage and the MF processor already do. This alone
   took the ratio 0.9326 -> 0.9861 and meanAbs 2.916 -> 1.102.

**The scaler fix also made the default path faster** — AREA is cheaper than
bicubic as well as more correct:

| | the 58-video sequence | full 56-sequence corpus |
|---|--:|--:|
| baseline (2026-08-03) | 115.38s | 699.07s (mean of two runs) |
| **with the FFmpeg fixes** | **96.39s** | **664.74s** |
| delta | **-16.5%** | **-4.9%** |

The corpus figure is the one to quote for the default path: -4.9% for every
Windows and Linux user, on top of the colour convergence.

### Across the board: full 56-sequence corpus, the 6c/12t box

| Config | corpus total | vs baseline | the 58-video sequence | the 44-video sequence |
|---|--:|--:|--:|--:|
| baseline (2026-08-03, no fixes) | 699.07s | — | 115.38s | 98.70s |
| FFmpeg + colourspace/snap/AREA | 664.74s | **-4.9%** | 97.04s | 91.54s |
| MF advanced processing | 573.26s | **-18.0%** | 52.93s | 42.18s |
| **MF + ID3D11VideoProcessor** | **532.87s** | **-23.8%** | **44.08s** | **34.67s** |

Only 13 of 56 sequences contain video, so the corpus figure is heavily diluted;
per-sequence the video-heavy ones are where it lands (the 58-video sequence 2.20x,
the 44-video sequence 2.64x against the already-improved FFmpeg path).

**-23.8% on the whole corpus** from the two changes combined, with all three
decoders within ~1.4% of macOS on raw pixels.

### Cross-box, like for like (the 44-video sequence, identical .xsq)

| | Intel N95 4c/4t | Ryzen 7640HS 6c/12t |
|---|--:|--:|
| FFmpeg (fixed) | 294.12s | 91.54s |
| MF advanced processing | 102.16s (**2.88x**) | 42.18s (2.17x) |
| MF + ID3D11VideoProcessor | 97.79s (**3.01x**) | 34.67s (2.64x) |

Same sequence both sides, so the comparison is clean: **3.01x on the weak box
vs 2.64x on the fast one.** Hardware decode is fixed-function, so the weaker
the CPU it is attached to, the larger the multiplier.

### Second Windows box: hardware decode matters MORE on weak hardware

`the 44-video sequence`, verified MD5-identical on both machines, all media present:

| | Intel N95, 4c/4t, UHD | Ryzen 7640HS, 6c/12t, 760M |
|---|--:|--:|
| FFmpeg (fixed) | **294.12s** | **91.54s** |
| MF advanced processing | **102.16s** (2.88x) | (corpus) |
| MF + ID3D11VideoProcessor | **97.79s** (3.01x) | (corpus) |

CPU time falls too - 449.3s to 297.0s - so this is work disappearing, not work
being moved off the critical path.

The striking comparison: **the N95 with hardware decode (97.8s) is about level
with the Ryzen doing software decode (91.5s)**. Quick Sync is fixed-function,
so it barely cares how weak the surrounding CPU is; the weaker the box, the
bigger the multiplier. That argues for defaulting MF on rather than treating it
as a power-user option.

The direct video-processor path was verified to actually engage on Intel, not
silently fall back: **43 `WHVD VP: ready` lines** (`1920x1080 -> 800x286,
matrix BT.601`), no failures. So it works across both AMD and Intel iGPUs.

Two incidental findings on that box:

- `xLightsVideoReaderAccelerated` was **False** - it had been doing pure
  software decode. Worth checking on any machine that "feels slow".
- FFmpeg's hardware-decoder auto-detect probes `cuda` first and fails on this
  NVIDIA-less machine before reaching a usable device. Cached after the first
  miss, so it is cheap, but the ordering does needless work on Intel and AMD.

### DIAGNOSED + FIXED (2026-08-04): uninitialized GPU blend scratch, published by a zero-input canvas blend

The "latent GPU/CPU race" below is solved, and it was **not a race in the
submission path at all**. Root cause: `doBlendLayers` creates its per-model
`tmpBufferBlend` scratch without initializing it, and a **canvas-mode layer
whose below-layers are all empty** produces a blend with ZERO valid input
layers — no dispatch ever writes the scratch, yet `saveToPixels` still
publishes it (PutColors scatters it into the canvas layer's pixel buffer, and
the completion loop copies it into the nodes). Freshly created VMA memory is
arbitrary — frequently a recycled GPU buffer's old contents — so the canvas
base was whatever bytes that memory happened to hold, different every run.
The effect then draws over that base and the output blend feeds it back into
the next frame's preload, so one bad frame persists for the whole effect.

On the affected sequence the shape is: a large tree model with a submodel
row whose canvas layer runs a Pictures effect over empty lower layers. The
divergence window (~112 frames) is exactly that effect's duration, ending
where its fade-out rounds the contamination to zero.

The trail that got there, each step with a reproduced differing pair and
content checksums (new `XL_EFFSUM=<model>` / `XL_BLENDSUM=<model>` /
`XLDBG_PICSUM=<model>` filtered probes; prefix-matched so submodel rows are
included — an equality filter on the model name silently exempts exactly the
rows most likely to matter):

1. **Not a silent GPU→CPU path flip.** New `XL_GPU_STATS2` probes count every
   transient fallback (command-buffer budget, allocation failures, descriptor
   failures): all zero in differing runs, dispatch counts identical.
2. **Not the image pipeline.** The consumed picture pixels hash identical in
   differing runs (and stb_image_resize2's split-parallel resize proved
   byte-deterministic in a 400-iteration adversarial stress test).
3. **Not the blend math or its inputs.** Every per-layer and FINAL blend
   checksum identical across a differing pair — for the main model row.
4. **The submodel row was the producer.** With prefix-filtered probes, the
   first divergent stage is the canvas preload result (`SUM C`) of the
   submodel's layer 0 at the effect's first frame — and that preload's blend
   prints **no input-layer checksums at all**: the zero-valid-layers case.

The fix enforces the invariant at the buffer's single creation point: zero
`tmpBufferBlend` at creation, which is exactly the semantics of the CPU
path's `std::vector` scratch (zero-initialized on growth). The same hole
exists in the Metal backend (`newBufferWithLength:` does not guarantee zeroed
contents) — fixed there too; Apple's allocator happening to hand back zeroed
pages is why macOS never showed it.

Why the bisect table looked like a blend/transition submission race: the
blend and transition toggles remove ~30k GPU dispatches each (blur/roto only
hundreds), which collapses the GPU-buffer churn that determines whether the
fresh scratch lands on recycled (dirty) or new (zeroed) memory — hiding the
bug, not locating it. `XL_NO_GPU_BLEND` also happened to route the affected
row to the (correct) ISPC path. The video colour changes mattered only
because they changed what CONTENT earlier GPU buffers held when their memory
got recycled into the new scratch.

**Verification (the check could fail, and did fail pre-fix):**

| check | result |
|---|---|
| pre-fix, corpus-subset pair loop | diff on iteration 1–2, every attempt (3 of 3 that night) |
| fixed binary, same loop ×8 | **8 × 27/27 identical — zero diffs in 216 sequence-pairs** |
| fixed binary, full corpus ×2, FFmpeg path | **56/56 byte-identical** |
| fixed binary, full corpus ×2, D3DVP path (default) | **56/56 byte-identical** |
| macOS, same sequence rendered twice (Metal + fix) | identical |
| macOS, pre-fix Metal vs fixed Metal A/B (rebuilt control) | byte-identical — Apple's allocator already zeroed, hole latent not active |
| macOS desktop + iPad library builds | clean |

Incidental hardening that came out of the hunt (all landed alongside):
`xlImage::Rescale` now checks every stbir return code and logs failures
instead of silently shipping an uninitialized band (stb_image_resize2's
split-parallel resize itself proved byte-deterministic in a 400-iteration
adversarial stress test); `XL_GPU_STATS2` fallback-probe counters
(cbOverLimit / alloc / descriptor failures, peak command buffers, peak
deferred-free bytes) make "no silent GPU→CPU path flips happened" a
checkable gate condition instead of an assumption.

The original analysis, kept for the record:

| condition | result |
|---|---|
| pre-change binary (control) | clean 56/56 |
| new binary, default | **differs** (3 of 3 runs, same sequence) |
| `SWS_AREA` disabled | clean |
| near-black snap disabled | differs |
| `sws_setColorspaceDetails` disabled | differs |
| destination buffers zeroed | differs |
| `XL_NO_PARALLEL_FRAMES=1` | differs |
| **`XL_SERIAL=1`** | **clean** |
| **`XL_NO_GPU_COMPUTE=1`** | **clean** |
| `XL_NO_GPU_BLEND=1` | **clean** |
| `XL_NO_GPU_TRANS=1` | **clean** |
| `XL_NO_GPU_BLUR=1` | differs |
| `XL_NO_GPU_ROTO=1` | differs |

**The decoder is deterministic.** Hashing every frame served in two failing
runs: the complete multiset of (file, size, requested ms, served ms, pixel hash)
is identical - 75,495 tuples, zero differences either way. So the decoded
content does not vary; only what happens downstream of it does.

**The divergence needs GPU compute AND concurrent CPU work.** Disabling either
axis alone fixes it, and it survives frame-parallel being switched off. Within
the GPU stages, disabling blend or transitions fixes it while blur and rotozoom
do not - the two that fix it share command-buffer machinery, so this points at
the blend/transition submission path rather than at one kernel.

**This cannot live in the video changes**: the commit touches no file under
`render/`, `effects/`, `graphics/` or `vulkan/`. What `SWS_AREA` did was change
the decoded pixel *content* - deterministically - into a pattern that makes an
existing race land. Bicubic's content happened not to trigger it. Reverting the
scaler would hide the race, not fix it, and some other content change would
resurface it later on a user's show rather than on a bench.

Symptom shape, for whoever picks this up: ~100 channels of 193,268 on a single
large model, `maxAbsDiff=88`, frames correctly aligned, 0.00% of samples. It
needs full-corpus contention - the sequence alone never reproduces, in any
configuration.

Next step: instrument the Vulkan blend/transition submission path
(`XL_BLENDSUM=1` gives per-layer blend checksums) under corpus load and find
what is read before its producing command buffer has completed.

### Superseded analysis: the FFmpeg path is not deterministic on the full corpus

Final-build determinism gate, two full-corpus renders per config:

| path | result |
|---|---|
| DirectX11 / D3D11 video processor | **56 / 56 identical** (534.36s / 535.80s) |
| FFmpeg | **55 / 56** - `a mid-size GPU-heavy sequence` differs between two runs |

The pre-change baseline was 56/56 on the FFmpeg path, so the colourspace / snap
/ AREA changes are the prime suspect - but rendering **that sequence alone**,
twice each under default, `XL_NO_PARALLEL_FRAMES=1` and `XL_SERIAL=1`, came back
**identical every time**. So the trigger needs the full-corpus context: state
carried between sequences in one process, not a race inside one render. Worth
checking first: the process-global `s_absent` / `s_everOpened` hardware-device
caches in `FFmpegVideoReader.cpp:337-368`, which persist across sequences and
change decoder selection for later files depending on what earlier ones did.

Until this is understood the three FFmpeg colour changes should **not** ship.
They are independent of the Media Foundation work and were worth -4.9%, against
2.3x from MF - so the natural split is to land MF plus the frame-timing fixes
and hold the FFmpeg colour changes.

### Determinism: 56 / 56 byte-identical on the D3DVP path

Two full-corpus renders in the same config, then `--fseqcmp` on every pair:
**IDENTICAL=56 DIFF=0** (536.77s / 534.17s, 0.5% spread). Frame-parallel plus a
brand-new GPU pipeline is exactly where determinism breaks, so this had to be
re-established rather than assumed from the pre-change baseline.

### Frame-timing verification found a real MF bug (fixed)

Pixel comparison cannot answer "is each decoder returning the *same frame* for
the same request" - two decoders can both look plausible while sitting on
different frames. `XL_VIDEO_TRACE=<file>` logs one line per frame served:
decoder, file, size, requested ms, served ms, and a content hash. Note hashes
are **not** comparable across decoders (they legitimately differ); the
comparable signals are servedMS and distinct-frame count.

On `the 44-video sequence`, against macOS/AVFoundation:

| decoder | mean servedMS delta | 4K clip: frames served (distinct) |
|---|--:|--:|
| ffmpeg | **+0.92 ms** | 618 (210) |
| mfauto | **+9.26 ms** | **563 (195)** |
| mfvp | **+9.26 ms** | **563 (195)** |
| mac | reference | 611 (203) |

MF served **55 fewer frames** while reaching a *later* end position - skipping,
not truncating. Cause: `GetNextFrame`'s loop ran `while (_curPos < timestampMS)`,
which takes the first frame **at or after** the request; macOS and FFmpeg take
the **nearest** frame. Any request landing in the first half of a frame interval
jumped a whole frame - about half of all requests on a 29.97fps clip:

| request | frames available | mac | MF (before fix) |
|--:|---|--:|--:|
| 5350 | 5338, 5372 | 5338 | **5372** |
| 5400 | 5372, 5405 | 5405 | 5405 |
| 5450 | 5438, 5472 | 5438 | **5472** |

Two independent implementations agreeing while MF disagrees is what makes this
a bug and not a judgement call. Fixed by allowing half a frame of lead-in
before advancing. Every other file already matched across all four decoders
(the main video: 4520 distinct frames in all four), so it only showed on that
clip's frame rate - and it was invisible to every pixel-level comparison.

**Not yet done before any of this ships:** the colour analysis is one file at
one timestamp.
It needs the full-corpus determinism gate, a broader multi-file raw comparison,
and visual review on a curtain/composite sequence. The FFmpeg changes alter
output for **every existing Windows and Linux user** (and the macOS FFmpeg
fallback) - in the direction of matching macOS, but it is a real behaviour
change, not a byte-identical one.

### The measurement surface was wrong, and fixing it is what cracked this

Every colour number above is read out of a **post-blend fseq channel**, which
conflates the decoded video pixel with crop, scaling, TransparentBlack, layer
blending and model node mapping. That is far too many layers to attribute a
colour error through, and it is why four separate hypotheses each looked
plausible and then failed.

Before more colour work: build a **raw-frame harness** — decode frame N of one
specific file through FFmpeg, MF-auto and MF-D3DVP, and dump the raw RGB
buffers for direct comparison. That makes the decoder the only variable and
turns this from inference into a direct read. It is a small standalone tool and
would very likely have found the answer faster than any of today's attempts.

`MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING` is a convenience wrapper
with no colour controls. Taking `ID3D11VideoProcessor` in hand instead exposes
`VideoProcessorSetStreamColorSpace1` / `SetOutputColorSpace1` with explicit
`DXGI_COLOR_SPACE_TYPE`, which is the control we actually need. Shape:

1. Ask MF for **NV12 at native size** (hardware decode only, no processing) so
   the GPU surface stays on the GPU.
2. Run `ID3D11VideoProcessor` for convert+scale with the colour space stated
   explicitly, keeping the work on the GPU.
3. Copy back only the final small RGB frame — the same small readback that
   makes the current path fast.

That keeps the 2.32x and removes the guesswork. It is a real chunk of D3D11
work, so it should be costed before starting.

Fallbacks if that stalls: ship MF as a non-default option with the colour
caveat documented, or accept `XL_MF_SWSCALE` for correctness-critical shows
(but it is slower than FFmpeg, so it is not a shipping default).

### Attempt 1 (2026-08-03): nominal-range pin did NOT work; snap helped a little

Two changes, built and measured on `the 58-video sequence`:

| | result |
|---|---|
| `MF_MT_VIDEO_NOMINAL_RANGE = MFNominalRange_0_255` on the output type | **accepted (0 warnings) and changed nothing** |
| near-black snap ported into `BitmapFromSample` | small: corpus-level DIFFER 24.51% -> 24.22% vs macOS |
| render time | 49.29s (was 49.65s) - no regression from the snap |

The channel dump is **byte-identical before and after** on the sampled series:
MF/Mac stays at **1.097**, 0 of 24 sampled frames changed. So the
limited->full range hypothesis is **not supported** — MF was already emitting
full-range, or the source reader ignores that attribute on the output type.

**Refined hypothesis: the video processor scales in linear light.** MF/Mac =
**1.097**, and the macOS bridge documents that CIContext and
VTPixelTransferSession — both linear-light scalers — brightened non-black
pixels **~10%** versus vImage's encoded-space scaling
(`AVFoundationVideoBridge.mm:3881-3889`). That is a close quantitative match,
and it is the same trap macOS already fell into and escaped.

Next steps, cheapest first:

1. **Confirm the attribute is even honoured.** Read `MF_MT_VIDEO_NOMINAL_RANGE`
   back off the current media type after `SetCurrentMediaType` and log it. This
   distinguishes "ignored" from "applied but not the cause" and is a few lines.
2. If ignored, try `MF_MT_TRANSFER_FUNCTION` / `MF_MT_VIDEO_LIGHTING`.
3. **Stop MF from scaling.** Request native-size RGB32 and scale in encoded
   byte space ourselves (swscale is already linked). This is what macOS
   concluded, and it would trade away the decode-scale advantage while keeping
   hardware decode — measure both axes before committing.

Note the three-way spread is *not* MF-only: Windows FFmpeg is **-6.9%** vs
macOS, and that is **not** the d3d11va hardware path — a software-decode render
(`xLightsVideoReaderAccelerated=false`) produced byte-identical values to the
hardware one. So swscale-bicubic vs vImage/VideoToolbox decode-scaling is its
own pre-existing disagreement, independent of anything MF does.

---

## 4. Proposed order of work

Ranked by measured size and by risk, not by novelty.

1. **Make Media Foundation's output match FFmpeg's, then default it on.** The
   timing question is answered — **-34.4% on the video sequences, ~-19% on the
   corpus**. The blocker is colour fidelity, now diagnosed rather than
   suspected. In order:
   1. Set `MF_MT_VIDEO_NOMINAL_RANGE` (and `MF_MT_YUV_MATRIX` /
      `MF_MT_TRANSFER_FUNCTION`) explicitly on the output media type, then
      re-measure the B/A ratio — target 1.00.
   2. Port the near-black snap into `BitmapFromSample`: all three RGB channels
      `<= 4` snap to `(0,0,0)`. Same predicate as macOS, not a sum test.
   3. Check whether MF's scaler works in linear light; if so, that is a third
      correction and may argue for scaling outside MF.
   4. Explain the 55 `GetNextFrame failed`.
   5. Add a per-file "MF cannot open this path" negative cache so a rejected
      file is probed once, not once per Video-effect init — the same shape as
      the `s_absent` hardware-device cache (`FFmpegVideoReader.cpp:337-368`).
      That should recover most of the +2.6s worst-case fallback regression.
   6. Then flip the default, with the fallback matrix documented.

   The gate is **B/A -> 1.0 on the channel dump plus visual review on real
   models** — especially a curtain/composite sequence. Byte-identity against
   FFmpeg is not achievable and must not be used as the gate.
2. **Decode at render size on the FFmpeg path.** Lift the `#ifdef __APPLE__`
   around `PrepareDecodeSizes` and have `FFmpegVideoReader` honour
   `VideoDecodeSizeRegistry`. Software H.264 cannot decode at reduced size, so
   the win here is narrower than on macOS — mostly avoiding oversized
   `sws_scale` and cache pressure. Cheap, low risk, no new platform surface.
3. **A shared decoder for Windows.** The structural fix, and the expensive one:
   per-file decoder dedup, a frame cache, and `SupportsFrameIndependentAccess`
   so readers survive across frame-parallel windows and `XL_VIDEO_PARALLEL` can
   be turned on. `VideoReaderImpl` is already the right seam. This is a
   multi-week port of the design in `AVFoundationVideoBridge.mm`, so it should
   only start once items 1-2 have shown how much is left on the table.
4. **Output stage (`getCol`/`setCol`, 480s).** Not video, and not a Windows bug
   — but the largest non-video line, and macOS measurements systematically
   understate it. Re-evaluate `plans/render-perf/05-output-node-map.md` against
   Windows numbers.
5. **`blurZ` at 6.58x (92.4s).** Unexplained. Smallest of the big ratios and
   worth one profiling session before it earns an item.

6. **Raise `XL_GPU_SIZE_THRESHOLD` to 8192 on the Vulkan backend.** Worth
   -1.2% corpus-wide, but its real value is removing the "GPU rendering made it
   slower" regressions on small-buffer sequences. Windows/Vulkan only — this
   measurement says nothing about whether Metal's 2048 should move, and it
   should not. Not byte-identical (it moves effects between GPU and CPU
   kernels), so it needs the GPU-vs-CPU tolerance treatment, not an fseqcmp gate.

### Explicitly *not* on the list, with reasons

- **Chasing the Shader path.** Windows is 0.88x macOS. `the 107-shader sequence`
  (107 shaders) is slow for scheduling reasons, not shader cost.
- **GPU offload generally.** GPU execution is 0.93x of Metal and switching GPU
  compute off costs +25.6%. The iGPU is keeping up; the problem is not the GPU.
- **The discrete-card question.** Genuinely open, and **not answerable on
  current hardware** — both Windows boxes have integrated GPUs on shared
  memory. Needs a discrete-GPU machine before it can be investigated at all.

---

## 5. Reproducing

Scripts live on the test box under `<winuser>\scratch\`:
`build.ps1` (msbuild + save a tagged binary), `corpus.ps1` (whole-show headless
render, parses render-only times), `run2gate.ps1` (second run + `--fseqcmp`
determinism sweep), `prof.ps1` (arbitrary env + sequence subset),
`mfab.ps1` (Media Foundation vs FFmpeg A/B; saves and restores `settings.json`).

The full baseline record, with provenance, is `BASELINE.md` alongside the
output directories — keep it there rather than in the repo, per the
benchmarking guide's baseline convention.
