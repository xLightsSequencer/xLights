# 15. Desktop Platform Matrix — macOS vs Windows vs Linux

_Generated from code on 2026-07-31._

Scope: user-visible differences between the macOS, Windows and Linux builds of the
**desktop** app (`src-ui-wx/` + `src-core/` + `common/` + `macOS/src-apple-core/` +
`macOS/src-mac-ui/`). Derived entirely from source and build files. All line
numbers are against the tree at the date above.

Build configurations referenced throughout:

| Build | Config used | Defines that gate features |
|---|---|---|
| macOS | `macOS/xLights.xcodeproj`, `Release` | *(none of the `HAVE_*` set)*; `__XLIGHTS_HAS_TOUCHBARS__` via `macOS/src-mac-ui/osxUtilsUI/TouchBars.h:19` |
| Windows | `xLights/Xlights.vcxproj`, `Release\|x64` | `HAVE_VULKAN`, `HAVE_VULKAN_SHADER`, `HAVE_ORT`, `IGNORE_VENDORS`, `PYTHON_RUNNER` (`Xlights.vcxproj:275`, `:121`) |
| Windows ARM | `Release\|ARM64EC` / `Release\|ARM` | `IGNORE_VENDORS` only — no `HAVE_VULKAN*`, no `HAVE_ORT` (`Xlights.vcxproj:319`, `:363`) |
| Linux | `xLights/xLights.cbp`, `Linux_Release` (via cbp2make) | `LINUX`, `HAVE_VULKAN`, `HAVE_VULKAN_SHADER` (`xLights.cbp:104`, `:163-166`) |
| Linux (CMake) | `CMakeLists.txt` | never defines `LINUX` — see row 58 |

---

## Practical impact summary

| Area | macOS | Windows | Linux | Impact |
|---|---|---|---|---|
| Preview canvas | Metal (always) | OpenGL default, Vulkan opt-in | OpenGL default, Vulkan opt-in | Preview antialiasing and screenshot/export fidelity differ; only Win/Linux expose a backend choice. |
| GPU effect compute | Metal, always on when a GPU exists | Vulkan, declined on software devices | Vulkan, declined on software devices | Same 22 effects are GPU-accelerated everywhere, but Win/Linux silently drop to CPU on llvmpipe/WARP. |
| Shader effect | Metal (MSL via glslang) | Vulkan SPIR-V (Release x64 only) | Vulkan SPIR-V | Windows Debug and Windows ARM have no native shader path; Windows GL fallback is single-threaded by default. |
| Text rendering | wx → CoreText | Direct2D/DirectWrite | FreeType + HarfBuzz | Text effect typeface fallback, emoji colour and RTL support all differ by OS. |
| Render byte-parity | `-O3 -ffast-math` + ThinLTO | `/O2 /fp:precise`, no fast-math | `-O2 -ffast-math`, no LTO | Same sequence is **not** guaranteed to render byte-identical FSEQ across the three OSes. |
| ISPC SIMD kernels | `-O3`, precise math | no `-O`, precise math | `--math-lib=fast` | A fourth source of per-OS pixel divergence; absent entirely on Windows ARM. |
| Video decode | AVFoundation (HW on by default), FFmpeg fallback | **Media Foundation + D3D11 video processor, HW on by default** (2026-08-03), FFmpeg fallback | FFmpeg, HW pref hidden | Windows now matches macOS on decode colour/frame selection and is 2-3x faster on video-heavy sequences. Both still take the stateful frame path; only macOS is frame-parallel. |
| Video encode | VideoToolbox via AVFoundation | NVENC/AMF/QSV with retry chain | software libx264/libx265 only | Video export is markedly slower on Linux and fails hard where Windows would degrade. |
| Audio playback | AVAudioEngine | SDL2 | SDL2 | Speed change is instant on macOS, stalls (and can silently no-op) on Win/Linux. |
| Audio analysis rate | 48 kHz | 44.1 kHz | 44.1 kHz | Timing marks and audio-reactive effect values are not identical across OSes for the same song. |
| ML / AI features | CoreML stems, SoundAnalysis, Apple Intelligence (Apple Silicon) | ONNX/DirectML stems | none of them in the stock build | Whole waveform menu entries exist only on some OSes. |
| Filesystem | Sandbox + security-scoped bookmarks, iCloud download, version history | plain paths | plain paths | Only macOS prompts for folder access; only macOS finds cloud-evicted media and offers File ▸ Revert To. |
| Serial ports | live `/dev/cu.*` scan | COM1–COM20 fixed | 16 fixed `/dev/tty*` names | A Linux adapter at `ttyUSB6+` or a Windows `COM21+` can't be selected at all. |
| Controller ping | HTTP probe only | real ICMP | HTTP probe only | Non-HTTP controllers show red on macOS/Linux and green on Windows. |
| mDNS discovery | Bonjour (always) | native `dnsapi.dll` (Win10 1703+) | none in the shipped build | WLED controllers are never auto-discovered on Linux. |
| Update check | none in-app (App Store handles store builds) | GitHub `.exe` + installer launch (skipped for MSIX/Store builds) | GitHub `.AppImage`, download only | Mostly moot on macOS: ~98% of macOS installs are App Store (auto-updating); only direct-download users (~2%) get no new-version signal. |
| Crash reports | backtrace + all-threads + MetricKit | minidump + stack walker | backtrace only | Crash-bundle content differs substantially per OS. |
| Settings / logs | `~/Library/…` | `%APPDATA%\xLights\` | `~/.config/xLights/`, log in **`/tmp`** | Linux logs are lost on reboot, so "attach your log" often yields nothing. |
| Touch Bar / IAP | both present | absent | absent | macOS-only hardware and App Store surfaces. |

---

## Detailed differences

| # | Area | Behavior difference | macOS | Windows | Linux | Evidence |
|---|---|---|---|---|---|---|
| 1 | Graphics backend | Canvas base class selected at compile time | Metal (`wxMetalCanvas.hpp` satisfies `__has_include`) | Vulkan-capable `xlVulkanCanvas` | Vulkan-capable `xlVulkanCanvas` | `src-ui-wx/graphics/xlGraphicsBase.h:5-21`; header exists only at `macOS/src-mac-ui/osxUtilsUI/wxMetalCanvas.hpp` |
| 2 | Graphics backend | `HAVE_VULKAN` defined? | no (0 hits in pbxproj) | yes | yes | `CMakeLists.txt:355`, `:601`; `xLights/Xlights.vcxproj:275`; `xLights/xLights.cbp:164` |
| 3 | Preferences | "Preview graphics (restart required): Auto / OpenGL / Vulkan" | absent | present | present | `src-ui-wx/preferences/OtherSettingsPanel.cpp:245-264`, persisted `:317-323`, restored `:352-359` |
| 4 | Graphics backend | "Auto" only picks Vulkan when software GL is detected; the `GL_RENDERER` probe is Linux-only, so a Windows box on WARP still gets OpenGL | n/a | Auto→GL unless `LIBGL_ALWAYS_SOFTWARE`/`GALLIUM_DRIVER` set | Auto→GL unless probe reports llvmpipe | `src-ui-wx/graphics/vulkan/xlVulkanCanvas.cpp:66-83`, `:85-113`; probe `graphics/vulkan/xlSoftwareGLProbe.cpp:14-95`, stubbed off-Linux `:97-104` |
| 5 | Graphics backend | Vulkan failure silently reverts to OpenGL mid-session (surface creation, present support, format mismatch) | n/a | yes | yes | `xlVulkanCanvas.cpp:122-124`, `:281-291`, `:293-312`, `:314-320`, `:322-341`, `:648-655` |
| 6 | Preview quality | Multisampling on the preview canvas | 2× MSAA on 3D previews | 4× under Vulkan, **none** under the default OpenGL path | same as Windows | Metal `macOS/src-mac-ui/osxUtilsUI/wxMetalCanvas.mm:113-114`, `:141`, sample count `macOS/src-apple-core/osxUtils/MetalDeviceManager.mm:47-53`; Vulkan `src-core/graphics/vulkan/VulkanPipelineCache.h:101`; GL requests no samplers `src-ui-wx/graphics/opengl/xlGLCanvas.cpp:95-124` |
| 7 | Preview capture | Preview screen-capture / video export needs `TRANSFER_SRC` on the swapchain; returns null without it, and capture-only repaints still present to screen | n/a | Vulkan path only | Vulkan path only | `xlVulkanCanvas.cpp:392-395`, `:186-189`, `:673-684` |
| 8 | Display server | Vulkan surface creation | n/a | Win32 only | X11 (Xlib) **and** native Wayland | `xlVulkanCanvas.cpp:253-288`; `graphics/vulkan/xlVulkanWaylandSurface.cpp:18-56`; `graphics/vulkan/xlVulkanX11Surface.cpp:17-38` |
| 9 | Display server | VM detection forces Mesa software rendering; GTK backend forced to X11; GLX forced over wx 3.3's EGL default | no | no | yes — previews work in VMs but software-rendered | `src-ui-wx/xLightsApp.cpp:478-513`, `:809-816` |
| 10 | OpenGL | FBO capability assumed vs probed; a partial `wglGetProcAddress`/`glXGetProcAddress` load makes shader init bail | assumed present (`return true`) | probed, null-guard active | probed, null-guard active | `src-ui-wx/graphics/opengl/xlGLCanvas.cpp:21-45`, `:46-70`; `graphics/opengl/xlOGL3GraphicsContext.cpp:98-102`, `:222-237` |
| 11 | OpenGL | Pre-GL3.3 driver: modal "OpenGL 1.x/2.x no longer supported" plus degraded screenshot path (no scale-to-size FBO) | unreachable (Metal) | reachable | reachable | `xlGLCanvas.cpp:538-566`, `:591-602`, `:414-417` |
| 12 | OpenGL | Canvas renders into a not-yet-shown window | requires `IsShownOnScreen()` | requires `IsShownOnScreen()` | only requires a realized X window | `xlGLCanvas.cpp:499-509`, `:527-535` |
| 13 | GPU effects | 22 effects have a GPU compute path (Bars, ColorWash, Shockwave, Fan, Spirals, Galaxy, Circles, Plasma, Butterfly, Pinwheel, Kaleidoscope, Warp, Tree, Shimmer, Candle, Wave, Garlands, Fill, Meteors, Twinkle, Life, Fire) | Metal | Vulkan | Vulkan | dispatch `src-core/effects/EffectManager.cpp:81-95`; `src-core/effects/metal/MetalEffectManager.mm:186-233`; `src-core/effects/vulkan/VulkanEffectManager.cpp:211-266` |
| 14 | GPU effects | GPU compute declined on a software/CPU Vulkan device — those 22 effects drop to the ISPC CPU path while Shader stays on GPU | never (Metal enables whenever present) | yes, on lavapipe/CPU devices | yes, on lavapipe/CPU devices | `src-core/effects/vulkan/VulkanComputeUtilities.cpp:382-386`, `.h:243-246` vs `src-core/effects/metal/MetalComputeUtilities.hpp:126-128` |
| 15 | GPU effects | Blend layers, transitions, tent blur, box blur, roto-zoom on GPU | yes (Metal) | yes (Vulkan) | yes (Vulkan) | `MetalEffectManager.mm:87-141`; `VulkanEffectManager.cpp:106-156`; shim `src-core/render/GPURenderUtils.h:72-141`, null-instance CPU fallback `GPURenderUtils.cpp:6` |
| 16 | Render concurrency | Thread-pool size uses a real GPU core count vs a 4-bucket guess (16 discrete / 8 integrated / 4 unknown / 2 CPU) | IORegistry `gpu-core-count` | heuristic | heuristic | `src-core/render/RenderEngine.cpp:4539-4545`; `MetalComputeUtilities.mm:1217-1242`; `VulkanComputeUtilities.cpp:596-618` |
| 17 | Preferences | "GPU Rendering" checkbox hidden when no usable GPU compute device exists | never hidden | hidden on machines with no usable Vulkan device | same as Windows | `src-ui-wx/preferences/OtherSettingsPanel.cpp:193-195`, `:277-281` |
| 18 | GPU effects | Whole GPU stack absent on Windows ARM (no `HAVE_VULKAN`, no ISPC custom-build entries) | n/a | **ARM64EC/ARM: no GPU compute, no Vulkan preview, no SIMD kernels** | n/a | `Xlights.vcxproj:319`, `:363`; ISPC blocks conditioned on x64 only `:1886-2149` |
| 19 | Shader effect | Native shader backend | Metal, glslang + SPIRV-Cross → MSL | Vulkan SPIR-V, **`Release\|x64` only** (`HAVE_VULKAN_SHADER` absent in Debug) | Vulkan SPIR-V, all targets | `src-core/effects/SPIRVShaderEffect.cpp:194-218`; `src-core/effects/metal/MetalShaderEffect.mm:74`; `VulkanShaderEffect.cpp:22`; `Xlights.vcxproj:275` vs `:161`; `xLights.cbp:165` |
| 20 | Shader effect | GL-path shader calls serialized onto one worker thread unless the user opts in — multi-row sequences using the GL fallback are materially slower | direct on calling thread | **serialized by default** | direct on calling thread | `src-core/graphics/GLContextManager.h:85-99`; Windows dispatch `GLContextManager.cpp:770-778` |
| 21 | Preferences | "Render shaders on background threads" checkbox | hidden | visible + wired | hidden | `src-ui-wx/preferences/OtherSettingsPanel.cpp:105`, `:266-274`; wiring `src-ui-wx/xLightsMain.cpp:2132-2139` |
| 22 | Shader effect | Compiled-program cache retention — sequences cycling many `.fs` files recompile more often off-Apple (per-effect first-frame stall) | 24 programs | 10 | 10 | `src-core/effects/ShaderEffect.cpp:422-424`, used `:586` |
| 23 | Shader effect | A hidden off-screen GL canvas is created per Shader panel purely as a context provider | not created | created | created | `src-ui-wx/effectpanels/ShaderPanel.h:19-21`, `:46-48`, `:103-106`; `ShaderPanel.cpp:44-61`, `:80-86` |
| 24 | Check Sequence | CRITICAL "shader effects but OpenGL version lower than 3" check — the same sequence yields different error totals | never runs | runs | runs | `src-ui-wx/xLightsMain.cpp:5392-5418` |
| 25 | Text rendering | Backend used by the Text effect (and the Shape effect's Emoji shape) | wxGraphicsContext → CoreGraphics/CoreText | Direct2D + DirectWrite | FreeType + HarfBuzz + Fontconfig | selection `src-ui-wx/graphics/wxTextDrawingContext.cpp:419-467`, `:23-27`; `src-core/render/D2DTextDrawingContext.cpp:559`; `src-core/render/FreeTypeTextDrawingContext.cpp:36-41`, mac stubs `:1038-1061` |
| 26 | Text rendering | Fallback typeface when a saved font name doesn't resolve — same sequence renders a different face, so glyph widths and line breaks change | CoreText default | Segoe UI | DejaVu Sans → Liberation Sans | `FreeTypeTextDrawingContext.cpp:92-102`, `:1069-1074`; `wxTextDrawingContext.cpp:452-456` |
| 27 | Text rendering | Colour emoji | colour | **monochrome** (`D2D1_DRAW_TEXT_OPTIONS_NONE`; the colour-font flag appears nowhere in the tree) | colour (`FT_LOAD_COLOR`) | `D2DTextDrawingContext.cpp:459-460`; `FreeTypeTextDrawingContext.cpp:756-758`, `:840-842`; face names `src-core/render/TextDrawingContext.h:27-36` |
| 28 | Text rendering | RTL / bidi text | bidi via CoreText | bidi via DirectWrite | **forced LTR** — `hb_buffer_set_direction(LTR)` precedes `guess_segment_properties`, which only fills unset properties | `FreeTypeTextDrawingContext.cpp:594-595` |
| 29 | Text rendering | Per-character extents (used for per-character colouring) computed three ways → x-positions can differ by a pixel or two | `GetPartialTextExtents` | `DWRITE_CLUSTER_METRICS` | O(n²) HarfBuzz prefix re-shape | `wxTextDrawingContext.cpp:398-413`; `D2DTextDrawingContext.cpp:482-516`; `FreeTypeTextDrawingContext.cpp:986-995` |
| 30 | Text rendering | Antialiasing is off on all three but implemented differently (wx `ANTIALIAS_NONE` / D2D `ALIASED` / FT 1bpp `LOAD_TARGET_MONO`) → subtly different glyph edges | — | — | — | `wxTextDrawingContext.cpp:281`; `D2DTextDrawingContext.cpp:207-208`, `:382`; `FreeTypeTextDrawingContext.cpp:751-756` |
| 31 | Render determinism | Release float semantics | `-ffast-math` (`GCC_FAST_MATH=YES` + explicit flag), `-O3`, ThinLTO, `gnu++23` | **no `/fp:` element at all** → `/fp:precise`; `/O2` + `/GL`+`/LTCG`, `stdcpp20` | `-ffast-math`, `-O2`, **no LTO**, `-std=gnu++20` | `macOS/xLights.xcodeproj/project.pbxproj:2102`, `:2106`, `:2141`, `:2147` (and target-level `:2423`, `:2427`, `:2465`, `:2471`); `Xlights.vcxproj` has 0 `FloatingPointModel` hits, `:272-283`; `xLights.cbp:92-95` |
| 32 | Render determinism | Consequence: a sequence rendered on Windows is not guaranteed byte-identical to macOS/Linux (fast-math enables finite-math-only, no-signed-zeros, reciprocal and unsafe optimisations); macOS and Linux share fast-math but differ in `-O3`+LTO vs `-O2`, so they are not guaranteed identical to each other either | — | — | — | as row 31; macOS also silences the related clang diagnostic (`project.pbxproj:2150`, `-Wno-nan-infinity-disabled`) |
| 33 | Render determinism | ISPC kernel flags — a fourth set of math semantics. `--math-lib=fast` swaps ispc's `sin`/`cos`/`exp`/`pow` for approximations, so Linux SIMD effects (Plasma, Butterfly, Galaxy, Warp, Fire, …) diverge from macOS/Windows even with GPU off | `-O3`, default math lib | no `-O`, default math lib | **`--math-lib=fast`** | `macOS/scripts/ispc_compile:3-8`, `:24-29`; `Xlights.vcxproj:1890`, `:1902`, `:1907`; `build_scripts/linux/ispc.mak:12-18` |
| 34 | Render determinism | ISPC SIMD targets otherwise match (`avx2/avx1/sse4.2/sse2` on x86-64, `neon-i32x8` on aarch64) | yes | x64 only | yes | `macOS/scripts/ispc_compile:24-29`; `build_scripts/linux/ispc.mak:4-8`; `Xlights.vcxproj:1890` |
| 35 | Video decode | Decode backend; same container/extension list is accepted everywhere so no format is lost | AVFoundation first, transparent FFmpeg fallback | FFmpeg | FFmpeg | `src-core/media/VideoReader.cpp:37-52`, `:79-98` |
| 36 | Video decode | "Hardware Video Decoding" preference means different things and defaults differently | **on**; doubles as the AVFoundation↔FFmpeg switch | **on** (2026-08-03, was off); enables the Media Foundation reader or cuda/qsv/d3d11va/vulkan | **checkbox hidden**, off | `VideoReader.cpp:62`, `:81`; defaults `src-ui-wx/xLightsMain.cpp:2109` vs `:2123-2126`; hidden `src-ui-wx/preferences/OtherSettingsPanel.cpp:266-270` |
| 37 | Video decode | HW decoder families offered | videotoolbox | cuda / qsv / d3d11va / vulkan (+ `*_cuvid`) | vaapi / vdpau, but unreachable because the pref is hidden and defaults off | `src-core/media/FFmpegVideoReader.cpp:386-412`, `:455-470` |
| 38 | Preferences | Hardware video **renderer** choice (DirectX11 / FFmpeg Auto / CUDA / QSV / Vulkan / AMF) | hidden | visible + wired | hidden | `OtherSettingsPanel.cpp:96-102`, `:268`, `:273`, applied `:296-299` |
| 39 | Video decode | `WindowsHardwareVideoReader` (Media Foundation) — Windows-only. **Now the default** (2026-08-03): hardware decode defaults on and the renderer defaults to DirectX11, and it drives `ID3D11VideoProcessor` directly so the YUV→RGB colour space is stated rather than guessed. Formats it cannot open fall back to FFmpeg, and since 2026-08-05 so do files it stops decoding **mid-stream**: reads have a deadline and a failed/stalled seek retires the reader, because under many concurrent readers MF would otherwise park every render thread forever. How many decoders may run at once is learned at runtime (no API reports the driver's session limit) and lowered when decode fails with several open | not compiled | compiled, **on by default** | not compiled | `FFmpegVideoReader.cpp:163-190`, `FallBackFromHardwareReader`; `WindowsHardwareVideoReader.cpp` `InitVideoProcessor`/`BltFromSample`/`HandleReadTimeout`; defaults `xLightsMain.cpp:2124-2125` |
| 40 | Render speed | Video effect frame parallelism — Windows/Linux always take the stateful path (extra frame-0 decode + seek per reader open) because `FFmpegVideoReader` never overrides `SupportsFrameIndependentAccess()` | `FrameParallelism::Pure` for all-intra files | always `Stateful` | always `Stateful` | `src-core/media/VideoReaderImpl.h:53`; `src-core/media/AVFoundationVideoReader.cpp:110-112`; consumers `src-core/effects/VideoEffect.cpp:148-150`, `:690-703` |
| 41 | Render speed | Decode-at-target-size hint is populated by the render pre-pass on all OSes but read only by the AVFoundation reader, so 4K sources decode at native res then scale per reader instance off-Apple | consumed | ignored | ignored | `AVFoundationVideoReader.cpp:57-62` vs `src-core/effects/VideoEffect.cpp:332`, `:380` |
| 42 | Video encode | Encoder selection for H.264/H.265 | `h264_videotoolbox` / `hevc_videotoolbox` | NVENC→AMF→QSV→software with a second-chance retry | **plain software libx264/libx265, no VAAPI/NVENC preference, no retry** — slower export, and a codec-open failure aborts instead of degrading | `src-core/media/VideoWriter.cpp:42-54`; `src-core/media/FFmpegVideoWriter.cpp:187-238`, Windows retry `:507-520` |
| 43 | Video encode | Quality knobs at the same nominal target → different file size/quality for the same export | `preset=fast, crf=18` | per-vendor rate control (`nvenc cq=18 p4`, `amf qp=18`, `qsv global_quality=18`) | `preset=fast, crf=18` | `FFmpegVideoWriter.cpp:441-473` |
| 44 | Video export | "Uncompressed Video, *.avi" entry in the Export Sequence format list | **absent** | present | present | `src-ui-wx/import_export/SeqExportDialog.cpp:70-88` (guard `:81-83`) |
| 45 | Video export | House-preview export resolution on a HiDPI display | real content scale | content scale forced to `1.0` → half the pixel dimensions | real content scale | `src-ui-wx/xLightsMain.cpp:3726-3745` |
| 46 | Audio | Playback backend | AVAudioEngine | SDL2 | SDL2 | `src-core/media/AudioManager.cpp:51-58` |
| 47 | Audio | Playback speed change (0.25×–4×) | `AVAudioUnitTimePitch` node, instant | whole track re-rendered through FFmpeg `atempo` with playback stopped; **falls back to 1× with only a log warning if it fails** | same as Windows | `src-core/media/SDLAudioOutput.cpp:52-197`, `:900-909`, `:976-1010` vs `macOS/src-apple-core/media/AVAudioEngineOutputBridge.mm:428-432` |
| 48 | Audio | Internal resample rate — every downstream analysis (FFT frame data, VU meter, Vamp timing marks, onset/pitch) runs on a differently-resampled track | 48000 | 44100 | 44100 | `src-core/media/AudioManager.cpp:62-66`, consumed `:870`, `:878` |
| 49 | Audio | Sequence-open time — the FFmpeg decoder does a full counting pre-pass then decodes again | single pass | ~2× decode for long tracks | ~2× decode for long tracks | `src-core/media/FFmpegAudioDecoder.cpp:44-77`, `:166` vs `macOS/src-apple-core/media/AudioToolboxBridge.mm:251-278` |
| 50 | Audio | "No audio device" indication — waveform panel turns red | never (hard-coded false) | when `SDL_Init` fails | when `SDL_Init` fails | `src-core/media/AVAudioEngineOutput.cpp:107`, `:151-153` vs `SDLAudioOutput.cpp:1162-1164`; consumer `src-ui-wx/sequencer/Waveform.cpp:1091-1093` |
| 51 | Audio export | Save-audio format | MP3 unavailable — dialog offers only `*.m4a`, a `.mp3` target is silently rewritten | `*.mp3;*.m4a` | `*.mp3;*.m4a` | `src-ui-wx/xLightsMain.cpp:8556-8576`; `src-ui-wx/sequencer/SeqSettingsDialog.cpp:2316-2323`; encoders `AudioToolboxBridge.mm:412-435` vs `FFmpegAudioDecoder.cpp:357-360` |
| 52 | Audio export | A `.wav` target | real 16-bit LPCM | **not honored** — FFmpeg picks AAC only for `*m4a`, else MP3 regardless of extension | same as Windows | `FFmpegAudioDecoder.cpp:357` vs `AudioToolboxBridge.mm:427-429` |
| 53 | Media warnings | Audio-compatibility probe backing the "Media Compatibility Warning" dialog on sequence open | real AudioToolbox probe with specific messages | **always returns `""`** — audio files are never listed | **always returns `""`** | `src-core/media/MediaCompatibility.cpp:91-99`; dialog `src-ui-wx/import_export/SeqFileUtilities.cpp:649-720` |
| 54 | Media warnings | Video-compatibility probe → different verdicts for the same file | real AVFoundation probe (decodes a sample; flags stride-misaligned rawvideo `.mov`) | FFmpeg heuristic: container name contains `mov`/`mp4`/`m4v` and codec in an 8-entry allow-list | same heuristic | `MediaCompatibility.cpp:30-88` vs `macOS/src-apple-core/media/MediaCompatibilityBridge.mm:51-127` |
| 55 | ML features | Waveform ▸ "Stem — Drums / Bass / Other / Vocals" | CoreML, macOS 12+ | ONNX Runtime + DirectML (`HAVE_ORT`, `Release\|x64` only) | **absent** — neither `HAVE_ORT` nor `HAVE_OPENVINO` is defined by `xLights.cbp`, so the menu items are never created and `SeparateStems` warns and returns false | menu `src-ui-wx/sequencer/Waveform.cpp:269-292`; backend `src-core/media/StemSeparator.cpp:104`, `:133`, `:494-498`; defines `Xlights.vcxproj:275`, `xLights.cbp:29`, `CMakeLists.txt:914` |
| 56 | ML features | Waveform ▸ "Classify Audio…" / "Clear sound-class gating" and the `CLASSIFIED` waveform/playback gate | present | **absent** | **absent** | `src-core/media/SoundClassifier.cpp:17`, `:73` (whole file inside `#ifdef __APPLE__`); menu `Waveform.cpp:295-305`, `:311-315`, handler `:456-508` |
| 57 | AI features | "Apple Intelligence" service in Preferences ▸ Services — free on-device palette generation, ImagePlayground image generation, and the "AI Speech 2 Lyrics" / "AI Lyrics from Audio" timing-track entries with no API key | Apple Silicon only | absent — cloud services with an API key required | absent | `src-core/ai/ServiceManager.cpp:14-16`, `:54-66`; `src-core/ai/AppleIntelligence.cpp:50-62`, `:116-207`; gating `src-ui-wx/sequencer/RowHeading.cpp:801-805`, `:1251-1253` |
| 58 | Build variance | `LINUX` is defined by `xLights.cbp:104` but **never** by `CMakeLists.txt` (0 hits), so a CMake-built Linux binary loses `USE_MMAP_BLOCKS` (huge-page sequence data), `XInitThreads()` and `glutInit()` | n/a | n/a | two materially different Linux binaries | `xLights.cbp:104`; `src-core/render/SequenceData.h:21-27`, `SequenceData.cpp:289-300`; `src-ui-wx/xLightsApp.cpp:478-479`, `:1730-1732`; `src-core/utils/JobPool.cpp:119-121` |
| 59 | Memory | Sequence frame-data strategy — a sequence that renders on macOS by paging to disk fails out-of-memory elsewhere at the same RAM size | file-backed mapping above 50% of physical RAM | plain `calloc`, never file-backed, no huge pages | anonymous `MAP_HUGETLB` + `MADV_HUGEPAGE`, never file-backed | `src-core/render/SequenceData.cpp:116-129`, `:222-300` |
| 60 | Memory | Render-cache memory back-off | `vm.swapusage` > 24 GB | pagefile > multiplier × installed RAM | **always `false`** — the cache never self-limits and can drive the box into swap/OOM | `src-core/utils/UtilFunctions.cpp:583-618`; consumers `src-core/render/RenderCache.cpp:63`, `:275`, `:711` |
| 61 | Playback timing | Frame timer driving output/playback | `CADisplayLink` per `NSScreen` on macOS 14+ (refresh-locked), `wxTimer` below | dedicated thread at `DEFAULT_PRIORITY+1`, wall-clock locked, logs missed frames and deliberately drops a frame if the previous is still processing | same as Windows | `src-ui-wx/shared/utils/xLightsTimer.cpp:19-23` (whole file `#ifndef __WXOSX__`), `:106-139`, `:151-198`; `xLightsTimer.mm:70-143`, `:210-212` |
| 62 | Render throughput | Render-worker thread QoS — on Apple Silicon the scheduler moves work between E- and P-cores | `QOS_CLASS_BACKGROUND` idle / `USER_INITIATED` running | `SetThreadQOS(a)` is an empty macro | empty macro | `src-core/utils/ExternalHooks.h:35`; `macOS/src-apple-core/osxUtils/xlAppleUtilsCppCore.mm:84-90`; call sites `src-core/utils/JobPool.cpp:300`, `:443`, `:459` |
| 63 | Render stability | Worker-thread stack size — deep-recursion effects have ~8× more headroom on Linux | `pthread_create` with explicit 1 MB | `std::thread` default (~1 MB) | `std::thread` default (~8 MB) | `src-core/utils/JobPool.cpp:136-149` |
| 64 | Filesystem | Show-folder access: security-scoped bookmarks, folder re-prompt after a move/rename | sandboxed, `ObtainAccessToURL` real | `inline … return true` | `inline … return true` | `src-core/utils/ExternalHooks.h:37`; `macOS/src-apple-core/osxUtils/xlAppleUtilsCppCore.mm:56-69`; call site `src-ui-wx/xLightsApp.cpp:615-627` |
| 65 | Filesystem | Cloud-evicted media: `FileExists()` waits for the download and `GetAllFilesInDir()` resolves `.icloud` placeholders | yes | no — a OneDrive/GDrive placeholder is simply "missing" | no | `macOS/src-mac-ui/osxUtilsUI/xlMacUtilsCppUI.mm:20-58`; non-mac stubs `src-ui-wx/shared/utils/wxUtilities.cpp:590-599` |
| 66 | File menu | "Revert To… ▸ Last Saved / <revisions>" version-history submenu, backed by `NSFileVersion` snapshots taken on every save | present | **absent** | **absent** | menu `src-ui-wx/xLightsMain.cpp:2076-2107`; stubs `src-core/utils/ExternalHooks.h:44-46`; real impl `macOS/src-apple-core/osxUtils/xlAppleUtils.swift:305-345`; snapshot `src-core/render/SequenceFile.cpp:1561` |
| 67 | File menu | "Open New xLights Instance" — the item is created on macOS with a Dock-icon equivalent; the shared handler body is entirely inside `#ifdef __WXOSX__` | works | no-op if reached | no-op if reached | `xLightsMain.cpp:2114-2119`, handler `:5577-5586`; Dock menu `:592-622`, instantiated `:2121`; spawn helper `src-ui-wx/xLightsApp.cpp:196-232` |
| 68 | File open | Double-clicking a sequence in the file manager: `MacOpenFiles` walks up to the show folder, requests sandbox access, serialises queued opens and spawns a fresh instance for `.xsqz`/`.zip` when one is already open | in-process | file arrives only as a launch argument | same as Windows | `src-ui-wx/xLightsApp.cpp:570-688`; `macOS/Assets/xLights/Info.plist:7-43` |
| 69 | Show safety | Sleep prevention while outputting to lights | `DisableSleepModes()` real | empty macro — the machine can sleep mid-show | empty macro | `src-core/utils/ExternalHooks.h:33-34`; `macOS/src-apple-core/osxUtils/ExternalHooksAppleCore.mm:24-30`; call sites `xLightsMain.cpp:2967`, `:2994`, `:3028` |
| 70 | Settings | `settings.json` location | `~/Library/Application Support/xLights/` | `%APPDATA%\xLights\` | `$XDG_CONFIG_HOME/xLights/` else `~/.config/xLights/` | `src-ui-wx/settings/XLightsConfigAdapter.cpp:33-56` |
| 71 | Logs | `xLights_spdlog.log` location — on Linux the log sits directly in `/tmp`, wiped on reboot and subject to `systemd-tmpfiles`, so "please attach your log" often yields nothing | `~/Library/Logs/` | `%APPDATA%\xLights\Logs\` | **`/tmp/`** | `XLightsConfigAdapter.cpp:58-74`, `:76-82` |
| 72 | Settings | One-time wxConfig→JSON migration imports the bookmarks section only on macOS, while the wipe deletes it everywhere → upgrading loses saved bookmarks | kept | **lost** | **lost** | `XLightsConfigAdapter.cpp:136-150` (guard `:141-143`), `:153-158` |
| 73 | Appearance | Dark mode enablement | automatic (system appearance) | **must be explicitly opted into** via `MSWEnableDarkMode()` at startup | whatever the GTK theme reports | `src-ui-wx/xLightsApp.cpp:835-839` |
| 74 | Preferences | Colors ▸ "Suppress Dark Mode" — only Windows can force a light theme while the OS is dark; every dark-conditional colour in the app follows | hidden | visible (restart prompt) | hidden | `src-ui-wx/preferences/ColorManagerSettingsPanel.cpp:94-96`, `:116-126`; `src-ui-wx/shared/utils/wxUtilities.cpp:566-587` |
| 75 | Preferences | Preferences window chrome — on Linux a page taller than the dialog is clipped with no scrollbar | native `wxPreferencesEditor`, applies immediately, pages `SetMinSize(500,-1)` | modal `wxListbook` with OK/Cancel, pages wrapped in a scrolled window at 90%×45% of screen | modal `wxListbook`, **neither scrolling nor a min width** | `src-ui-wx/preferences/xLightsPreferences.cpp:60-95`, `:103-107`, `:242-259` |
| 76 | Preferences | "Ignore vendor model recommendations" — recommendations are unsuppressable on macOS/Linux and off by default on Windows | hidden, forced false | visible, setting defaults **true** | hidden, forced false | `OtherSettingsPanel.cpp:142-144`, `:362-367`; default `src-ui-wx/xLightsMain.cpp:1748-1756` |
| 77 | DPI | Controller Visualiser manual DPI scaling — identical layouts render at different physical sizes on HiDPI | `ScaleWithSystemDPI` returns the value unchanged; scale from the backing-scale query | every margin/gap/port/font × `GetPPI().y / 96.0` | same as Windows | `src-ui-wx/setup/ControllerModelDialog.cpp:106-117`, `:178-195`, `:1481-1500`; `src-ui-wx/shared/utils/ExternalHooksUI.h:32-36` (non-mac returns `1.0`) |
| 78 | Icons | Toolbar/navigation artwork | live SF Symbols (follow system appearance, scale natively) | bundled XPM bitmaps, `wxIMAGE_QUALITY_BICUBIC` rescale | bundled XPM bitmaps, `wxIMAGE_QUALITY_HIGH` | `src-ui-wx/shared/utils/BitmapCache.cpp:354-359`, `:597-614`, `:694-703`; `src-ui-wx/layout/ViewsModelsPanel.cpp:307-336`; `src-ui-wx/layout/LayoutPanel.cpp:573-590` |
| 79 | UI | Unsaved-layout Save-button highlight | native button background tint | `SetBackgroundColour(255,108,108)` | same as Windows | `src-ui-wx/app-shell/TabSetup.cpp:808-822`; `macOS/src-mac-ui/osxUtilsUI/ExternalHooksMacOSUI.mm:41-43` |
| 80 | UI | Row-heading label sizing | fractional point size | integer `SetPixelSize` | fractional point size | `src-ui-wx/sequencer/RowHeading.cpp:2554-2568` |
| 81 | Hardware | Touch Bar — House/Model preview toggles, transport, zoom, a colour bar with the 8 palette swatches and the sparkle-frequency slider, plus one button per effect | present | absent | absent | `macOS/src-mac-ui/osxUtilsUI/TouchBars.h:19`; `src-ui-wx/sequencer/MainSequencer.cpp:1242-1320`; `src-ui-wx/color/ColorPanel.cpp:1325-1351`; `src-ui-wx/sequencer/tabSequencer.cpp:235-236` |
| 82 | Hardware | 3Dconnexion SpaceMouse in the Layout preview | native `3DconnexionClient.framework` + hidapi fallback | hidapi (packets ignored unless the pointer is over the focused window) | **no support at all** — `__USE_HIDAPI__` is defined only for `__WXOSX__ \|\| __WXMSW__`, so no detection thread starts | `src-ui-wx/layout/Mouse3DManager.h:8-10`; `Mouse3DManager.cpp:421-429`, `:719-745` |
| 83 | Store | Help ▸ Donate | App Store build: native StoreKit purchase sheet with six SKUs; otherwise PayPal | PayPal in browser | PayPal in browser | `src-ui-wx/xLightsMain.cpp:6655-6669`; `src-core/utils/ExternalHooks.h:38`; `macOS/src-mac-ui/osxUtilsUI/osxInAppPurchases.mm:208-249` |
| 84 | Updates | Help ▸ "Check for Updates" and the automatic startup check | menu item removed at startup and the auto-check skipped — no in-app update path; low impact since macOS distribution is ~98% App Store (auto-updating), affecting only direct-download users (~2%) | present; GitHub `.exe`, then "xLights will now close and the installer will launch" | present; GitHub `.AppImage`, then "Downloaded to: <path>" and nothing else | `xLightsMain.cpp:1204-1206`, `:1991-1994`, `:2584-2595`, `:7559-7563`; `src-ui-wx/app-shell/UpdaterDialog.cpp:149-170` |
| 85 | Crash reports | Contents of the uploaded crash bundle | backtrace + an extra `all-threads.txt` (mach suspension) + decoded `NSException` + MetricKit JSON payloads; a modal first asks the user to enable Share Analytics | binary minidump (`wxCrashReport::Generate`) + exception address + `xlStackWalker` symbolic trace + wx context XML | `backtrace()`/`backtrace_symbols()` + wx context XML only | `common/xlBaseApp.cpp:25-30`, `:60-242`, `:263-268`, `:286-355`, `:433-451`; MetricKit `macOS/src-apple-core/osxUtils/XLMetricKit.mm:105-146`, harvest `xLightsMain.cpp:4566-4592` |
| 86 | Serial | Port dropdown contents — a Linux adapter at `ttyUSB6+`/`ttyACM6+`/`ttyAMA*` or a Windows `COM21+` can never be selected | live scan of `/dev/cu.*` | fixed `COM1`–`COM9` + `\\.\COM10`–`COM20` | fixed 16 names (`ttyS0-3`, `ttyUSB0-5`, `ttyACM0-5`) | `src-core/outputs/SerialOutput.cpp:99-159` |
| 87 | Serial | Filtered "available ports" list (auto-assign, Check Sequence) | `/dev` scan excluding Bluetooth `cu.B*` | `HKLM\HARDWARE\DEVICEMAP\SERIALCOMM` registry | **not implemented** — returns the literal string `"port enumeration not supported on Linux"`, which then becomes a new serial controller's default port value | `SerialOutput.cpp:184-236`; consumed `src-core/outputs/OutputManager.cpp:1023-1044`, `src-core/outputs/ControllerSerial.cpp:132` |
| 88 | Serial | Non-standard DMX baud (250000+): all three offer it, by three mechanisms | `ioctl(IOSSIOSPEED)`; failure only **logs** — the port opens at the fallback rate | `dcb.BaudRate` direct | `TIOCSSERIAL` + `ASYNC_SPD_CUST` (deprecated on many modern drivers); failure returns -1 and the open fails | `src-core/outputs/serial_osx.cpp:41-70`, `:166-172`; `serial_win32.cpp:82`; `serial_posix.cpp:42-71`, `:167-186`; rate list `SerialOutput.cpp:164-182` |
| 89 | Serial | New serial controller defaults | pre-populated vendor/model `FPP` with its capability set | blank | blank | `src-core/outputs/ControllerSerial.cpp:134-138` |
| 90 | Networking | Controller ping — a controller answering ICMP but serving no HTTP on port 80 (plain E1.31/ArtNet gateways, web UI disabled or on another port) shows red off-Windows; per-controller cost is 2 s vs 1 s | HTTP GET only → `PING_WEBOK`/`ALLFAILED`, never `PING_OK` | real ICMP echo (no proxy) → `PING_OK` | HTTP GET only | `src-core/outputs/IPOutput.cpp:74-126`; `src-ui-wx/app-shell/TabSetup.cpp:829-855`, `:1284` (`isPingable` set only for `PING_OK`) |
| 91 | Networking | "Force Local IP" dropdown contents — `getifaddrs()` returns loopback and nothing filters it, so `127.0.0.1` is selectable and silently sends all output nowhere | `127.0.0.1` listed | not listed (`GetAdaptersInfo` skips loopback) | `127.0.0.1` listed | `src-core/utils/ip_utils.cpp:283-343`; `src-ui-wx/preferences/OutputSettingsPanel.cpp:82-99`; corroborating filter `src-core/discovery/Discovery.cpp:126-130` |
| 92 | Networking | Socket error text surfaced in logs/errors | `strerror(errno)` sentence | bare numeric WSA code (e.g. `10049`) | `strerror(errno)` sentence | `src-core/outputs/SocketAbstraction.h:51-58` |
| 93 | Discovery | mDNS/Bonjour backend. On Linux neither `-ldns_sd` nor an avahi include path appears in `xLights.cbp`, `CMakeLists.txt` or the vcxproj (0 hits), so `_DNS_SD_H` is undefined and `BonjourData`'s ctor/dtor/`handleEvents` compile to empty bodies. Consequence: **WLED controllers, discovered only via Bonjour, are never auto-found on Linux**; FPP degrades gracefully because it also uses multicast + curl probes | `dns_sd.h` in the SDK, always works | native `DnsServiceBrowse` from `dnsapi.dll` — **Windows 10 1703+ only**, older Windows silently has no mDNS | **none** | `Discovery.cpp:37-51`, `:326-359`, `:360-423`; `src-core/controllers/WLED.cpp:145-147`; FPP fallbacks `src-core/controllers/FPP.cpp:3675-3690`, `:4085` |
| 94 | Controllers | HinksPix SD-card destination list — on Fedora/RHEL/Arch (`/run/media/…`) or a manual `/mnt` mount the card never appears; the "Select Folder" fallback keeps it from being a hard block | every directory under `/Volumes` | `wxFSVolume` removable+mounted | `/media/<user>/<label>` two-level scan only | `src-ui-wx/controllers/HinksPixExportDialog.cpp:983-1022` |
| 95 | Controllers | HinksPix write-permission pre-flight — a read-only/full card is only discovered mid-upload off-Apple | writes and deletes a test file | unconditional `return true` | unconditional `return true` | `HinksPixExportDialog.cpp:2066-2084` |
| 96 | Controllers | FPP Connect sequence-name column: case-insensitive show-dir prefix match | case-sensitive → shows bare `name.fseq` instead of `subfolder/name.fseq` when the stored path differs in case | case-insensitive | case-sensitive | `src-ui-wx/controllers/FPPConnectDialog.cpp:1978-1990`; same pattern `src-core/utils/FileUtils.cpp:330-333`, `:361-375` |
| 97 | Packaging | Package Sequence bundles the OS font files used by Text/Shape effects | CoreText resolution (skipping `/System/Library/Fonts/`) | registry font lookup | **`return {}` unconditionally** — the `.xsqz` contains no `Fonts/` folder, so typefaces fall back on the recipient's machine; only a debug-level log line records it | `src-core/render/SequencePackage.cpp:1399-1522`, consumed `:1526-1545` |
| 98 | Packaging | Zip entry names have spaces adjacent to path separators stripped, and a `MAX_PATH` overflow warning is emitted | verbatim | scrubbed + warning | verbatim | `SequencePackage.cpp:260-274`, `:280-284`; same in `HinksPixExportDialog.cpp:599-603` |
| 99 | Scripting | Tools ▸ Run Scripts lists and runs `*.py`. Lua works everywhere | **absent** — the Xcode project references neither Python nor pybind11 | present (`PYTHON_RUNNER`; vcxproj puts pybind11 and `$(Python_ROOT_DIR)\include` on the include path) | **absent** — CMake adds pybind11 headers but no Python include dir | gate `src-ui-wx/automation/PythonRunner.cpp:24-33`; listing `src-ui-wx/automation/ScriptsDialog.cpp:239-247`; `Xlights.vcxproj:121`; `CMakeLists.txt:589` |
| 100 | Automation | `xlDo` dispatch from the main binary (`xLights -xlDo …` or an `xlDo`-named symlink) | works | **not available** — the dispatch is `#ifndef __WXMSW__`; Windows ships a separate `xlDo.exe` | works | `src-ui-wx/xLightsApp.cpp:518-531`; `xlDo/xlDo.cpp:78` |
| 101 | Automation | Generated variable-output syntax — scripts consuming it are not portable | `EXPORT KEY=VALUE` | `@set KEY=VALUE` | `EXPORT KEY=VALUE` | `src-ui-wx/automation/automation.cpp:45-56` |
| 102 | CLI | `-x/--xschedule`, `-xs/--xsmsdaemon`, `-c/--xcapture`, `-f/--xfade`, `-n/--xscanner` launchers for the sibling apps in the AppImage | unrecognised switch | unrecognised switch | present | `src-ui-wx/xLightsApp.cpp:885-891`, `:897-952` |
| 103 | Headless | `--headless` / `--fseqcmp` process behaviour | demoted to a background app — no Dock icon, no focus steal | attaches to the parent console so output is visible from `cmd` (a Windows-subsystem binary otherwise prints nothing) | neither needed | `src-ui-wx/xLightsApp.cpp:728-767`, `:779-787`, `:1180-1184`; `macOS/src-mac-ui/osxUtilsUI/ExternalHooksMacOSUI.mm:23-31` |
| 104 | Timing tracks | Vamp plugin install (beat/onset generation) — the SDK is compiled into all three builds and no plugins are bundled anywhere, but only Windows gets one-click install and an auto-configured `VAMP_PATH` | menu opens vamp-plugins.org in a browser | downloads and silently installs the official pack (MSIX) or launches the installer; `VAMP_PATH` set at startup | opens vamp-plugins.org in a browser | `src-ui-wx/shared/utils/wxUtilities.cpp:256-317`; called `src-ui-wx/xLightsApp.cpp:831-833`; menu `src-ui-wx/sequencer/SeqSettingsDialog.cpp:1133-1155` |
| 105 | Editing | Pasting a spreadsheet range into the node/custom-model grids | native clipboard read preserves tab/column structure, wx fallback | wx `wxDF_TEXT` only — column structure can be lost | wx only | `src-ui-wx/shared/utils/wxUtilities.cpp:608` (stub); `macOS/src-mac-ui/osxUtilsUI/xlMacUtilsCppUI.mm:27-30`; callers `src-ui-wx/model/NodeSelectGrid.cpp:1093-1110`, `src-ui-wx/model/CustomModelDialog.cpp:1039` |
| 106 | Import | Import Channel Map: drag-and-drop mechanism, and clearing the "Find" box scrolls the tree to the top | native dataview drop (rejects drops between rows) | custom text drop target; Find-clear scrolls to top | custom text drop target; no scroll-to-top | `src-ui-wx/import_export/xLightsImportChannelMapDialog.cpp:1347-1354`, `:2800-2808`, `:4477-4490` |
| 107 | Import | Import SubModel file filter | `*.xml` — a renamed rgbeffects file can be picked | `xlights_rgbeffects.xml` only | `xlights_rgbeffects.xml` only | `src-ui-wx/model/SubModelsPanel.cpp:4679-4693` |
| 108 | Test dialog | Tools ▸ Test: double-clicking a model row toggles its checkbox (a deliberate workaround for mac checkbox behaviour) | yes | no | no | `src-ui-wx/setup/PixelTestDialog.cpp:1447-1452`, `:2382-2396` |
| 109 | Help | Help ▸ Release Notes | downloads the version-matched `README.txt` from GitHub and opens it in the browser | opens the bundled local `README.txt` with the registered `.txt` handler | same as Windows | `src-ui-wx/xLightsMain.cpp:5588-5608` |
| 110 | Startup | One-off warnings | "Full Disk Access granted… strongly discouraged" | "Show folder inside your Program Files folder…" (blocks folder selection) and a Windows 7 "known issues rendering some effects" box | none | `xLightsMain.cpp:1953-1957`, `:2601-2612`, `:2613-2619`, `:3107-3118`; `src-ui-wx/app-shell/TabSetup.cpp:598-603` |
| 111 | Help | Tip of the Day content — four tips are excluded by platform tag | 4 tips hidden | all shown | all shown | `src-ui-wx/app-shell/TipOfTheDayDialog.cpp:261-277`; `TipOfDay/tod.xml` (`exclude="OSX"`) |
| 112 | Shortcuts | Keybinding editor column headers and modifier matching — some shortcuts that fire with either Ctrl on Windows/Linux are stricter on macOS | Command ⌘ / Option ⌥ / Shift ⇧ / Control ⌃, exact match | Control / Alt / Shift / Control, either Ctrl matches | same as Windows | `src-ui-wx/app-shell/KeyBindingEditDialog.cpp:120-131`; `src-ui-wx/app-shell/KeyBindings.cpp:885-895` |
| 113 | About | About box shows an "- App Store" version suffix and a EULA hyperlink | App Store build only | no | no | `src-ui-wx/xLightsApp.cpp:380-384`; `xLightsMain.cpp:2700-2704` |

**Row counts:** practical-impact summary 19 rows; detailed differences 113 rows.

---

## Notes

**Structural observations**

- Two mechanisms carry almost all of the divergence, and they behave very
  differently. The **`ExternalHooks.h` stub pattern**
  (`src-core/utils/ExternalHooks.h:33-46`) makes eight capabilities silently
  no-op off-Apple — `DisableSleepModes`, `SetThreadQOS`, `ObtainAccessToURL`,
  `IsFromAppStore`, `GetFileRevisions` and friends. Nothing in the UI says the
  feature is absent; the call just does nothing. Rows 62, 64, 66, 69 and 83 are
  all this one pattern. The **`#ifdef` in the panel constructor** pattern
  (`OtherSettingsPanel.cpp:266-277`) hides five preference controls on some OSes
  while the underlying setting still exists and is still read, so the value a
  user cannot see still changes behavior (row 76 is the sharpest case — Windows
  defaults it to `true`, the other two force it `false`).

- **The GPU story is much closer to parity than the file layout suggests.**
  `src-core/effects/vulkan/` mirrors `src-core/effects/metal/` effect-for-effect
  (22 + Shader), and both register the same blend/blur/roto/transition overrides.
  The real per-OS gap is not *which* effects are accelerated but *when the
  acceleration is declined*: Vulkan refuses compute on a software device
  (`VulkanComputeUtilities.cpp:382-386`) where Metal has no equivalent check.

- **`__LINUX__` is a wxWidgets macro, not a project one, and several core files
  test it without including any wx header.** `FFmpegVideoReader.cpp:77-84` and
  `OutputManager.cpp:1023-1029` both have Linux guards that are compiled out on
  Linux. The second one is why a new serial controller on Linux gets the string
  `"port enumeration not supported on Linux"` as its port name (row 87). Any new
  core-side Linux guard must use `LINUX` (the `.cbp` define) — and even that is
  absent from the CMake build (row 58).

**Biggest risks of divergence**

1. **Render byte-parity across OSes is not a property this codebase has.** Four
   independent sources stack: fast-math on macOS/Linux but not Windows,
   `-O3`+ThinLTO vs `/O2`+`/GL` vs `-O2`, ISPC `--math-lib=fast` on Linux only,
   and GPU-vs-CPU selection that depends on the runtime device. Any regression
   process that diffs FSEQ output must compare same-OS runs; a cross-OS diff will
   always show noise and can never distinguish it from a real bug. Rows 31-34, 14.

2. **Two "silently absent" features that a user will read as a bug, not a
   platform limit.** WLED controllers never appear in discovery on Linux (row 93)
   and packaged sequences carry no fonts from Linux (row 97). Neither surfaces any
   UI message — the WLED case is an empty function body, the font case a
   debug-level log line. Both are one-line stubs that would be cheap to turn into
   a visible warning.

3. **The macOS build has no in-app update path — but it barely matters** (row 84).
   The menu item is removed and the auto-check skipped, with no Sparkle
   equivalent in the tree. Context from the maintainer: macOS distribution is
   ~98% App Store, where updates are automatic — the same reasoning behind
   Windows skipping its check for MSIX/Store-packaged builds. The gap is real
   only for the ~2% direct-download tail; a lightweight new-version notice for
   that tail is nice-to-have, not a priority.

4. **Linux is the weak leg on operational plumbing.** Logs in `/tmp` (row 71),
   no render-cache memory back-off (row 60), fixed serial-port names (rows 86-87),
   no mDNS (row 93), software-only video encode (row 42), no stem separation
   (row 55), no 3D-mouse support (row 82), and no fonts in packaged sequences
   (row 97). Several are single-function gaps rather than deep platform issues.

5. **Windows-ARM is an unflagged third Windows target** (row 18) with no Vulkan,
   no GPU compute, no native Shader path and no SIMD kernels — a materially
   different app from `Release|x64` shipping under the same name. Nothing in the
   UI distinguishes them.

6. **Preference controls that are hidden but still live** (rows 36, 38, 21, 74,
   76) are a recurring correctness hazard: the setting is read on every platform
   but adjustable on only some, so support answers of the form "uncheck X" are
   wrong on two OSes out of three.
