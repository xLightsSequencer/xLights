# Plan: Add ANGLE Support to xLights (Windows + Linux, runtime opt-in)

> Status: **WORKING on Windows (2026-06-07).** An ANGLE build of xLights compiles, links,
> launches, and renders its previews through **OpenGL ES 3.0 on Direct3D 11**
> (`glVer: OpenGL ES 3.0.0 (ANGLE … D3D11)`), with the `#version 300 es` shaders compiling
> cleanly. Build recipe: `cmake -G "Visual Studio 18 2026" -A x64 -DXLIGHTS_USE_ANGLE=ON`
> (VS18 toolset required to match the prebuilt wx libs). Bring-up fixes worth remembering:
> (1) `gl*` symbol resolution — opengl32 (pulled by wx) shadowed ANGLE's libGLESv2 for the
> GL 1.1 functions → black canvas; fixed by stripping opengl32 from `wxWidgets_LIBRARIES` and
> re-adding it via `/DEFAULTLIB` so it's searched last; (2) the cmake post-build copies
> 32-bit DLLs from `bin/` into the x64 output → `0xc000007b`; replace with x64;
> (3) **shader precision** — GLSL ES 3.00 defaults `int` to `highp` in the vertex stage but
> leaves it undeclared in the fragment stage, so a uniform `int` (e.g. `RenderType`) declared
> in both stages fails to link ("Precisions of uniform … differ") → that program's geometry
> (and, downstream, the background) didn't draw; fixed by adding `precision highp int;` to
> every ES shader stage. The Layout preview now renders the background image + models
> correctly. Linux not yet attempted.
>
> Original scope: Windows + Linux; both render paths
> (on-screen wx canvases *and* the off-screen `ShaderEffect` pool); runtime opt-in
> with automatic fallback. macOS already uses ANGLE-on-Metal for the shader pool and
> is out of scope here except for keeping its existing `USE_GLES` path working.

## DESIGN CHANGE (2026-06-05): compile-time backend, not runtime

The original plan was a **runtime** opt-in (one binary, switch native↔ANGLE at startup).
Investigation showed that's disproportionately expensive on Windows: the **core** `gl*`
functions (`glClear`, `glGetString`, `glDrawArrays`, …) bind statically to `opengl32`, and
ANGLE's `libGLESv2` exports the same names — you can't link both, and a runtime switch would
require routing *every* `gl*` call (core + extension) through a glad-style dispatch table
across every GL TU.

Decision: make the backend a **compile-time** choice — build **either** the native-GL binary
**or** the ANGLE binary, selected by the existing `USE_GLES` macro (already the Apple
ANGLE-on-Metal selector) extended to Windows/Linux. The ANGLE build links `libGLESv2`+`libEGL`
(and **not** `opengl32`/`libGL`), so `gl*` bind directly to ANGLE — no dispatch table. This
mirrors how Apple already does it and removes the hardest part of the work.

Consequences / reconciliation of the earlier runtime work:
- The in-app **"Rendering Backend" View-preference is removed** (can't switch a compile-time
  choice at runtime) — `ViewSettingsPanel` + `.wxs` + `Get/SetGLBackendString` reverted.
- `GLBackend` is reduced to a **compile-time reporter** (`CompiledBackend()` → ANGLE if
  `USE_GLES` else Native) used for startup logging; the preference/env-override/auto-fallback
  machinery is dropped.
- The `xlGLProc` indirection in `LoadGLFunctions` reverts to native-only (the ANGLE build
  uses ES3 prototypes directly, like Apple — no `wglGetProcAddress` loader).
- `AngleEGL` (EGL display/window/pbuffer management) is **still needed** and is gated on
  `USE_GLES`. The vendored ANGLE binaries + CMake/vcpkg wiring stay; the CMake option now
  *switches* the link set (`libGLESv2`+`libEGL` vs `OpenGL::GL`) rather than adding ANGLE
  alongside native.
- Ship model: two binaries (e.g. `xLights` and an ANGLE variant), user/installer picks.

## Implementation progress

Landed (compile-time `USE_GLES` model; native build is byte-for-byte unchanged):
- ✅ **Backend reporter** — `src-core/graphics/GLBackend.*` reduced to `CompiledBackend()`
  (ANGLE iff `USE_GLES`) + `ToString`, logged at startup in `xlGLCanvas`. (The earlier
  runtime resolver, `XLIGHTS_GL_BACKEND` env override, and View-prefs "Rendering Backend"
  control were removed — moot under compile-time selection.)
- ✅ **Phase 0 / binaries** — ANGLE extracted from vcpkg and vendored for the MSBuild build
  (`include/{EGL,GLES2,GLES3,KHR}`, `lib/windows64/`, `bin64/` + `z.dll`). ANGLE deliberately
  NOT in CI manifest.
- ✅ **Build flag** — CMake `XLIGHTS_USE_ANGLE` (default OFF) now *switches* the link set:
  ON → link `libGLESv2`+`libEGL`, define `USE_GLES`, no `opengl32`; OFF → native `OpenGL::GL`.
  Build one or the other.
- ✅ **Spike** — wx is WGL-only on Windows → custom on-screen EGL path needed (both platforms).
- ✅ **EGL backbone** — `src-core/graphics/AngleEGL.*`: isolated EGL module (EGL/GLES headers
  confined to the `.cpp`), opaque API for window + pbuffer contexts sharing one root group;
  gated on `USE_GLES` (non-Apple) with native stubs. Syntax-checked against the real vendored
  EGL headers, both configs.

Remaining (the `USE_GLES` rendering path — large, GPU build+test required):
- ⬜ **MSBuild ANGLE configuration** — a VS build config that defines `USE_GLES` and links
  `libGLESv2`/`libEGL` instead of `opengl32` (the CMake side is done).
- ⬜ **`xlGLCanvas` under `USE_GLES`** — create the ANGLE EGL window surface via `xlAngleEGL`
  (using the wxGLCanvas HWND) instead of a `wxGLContext`; route `SetCurrent`/swap through EGL.
- ⬜ **`GLContextManager`** Win/Linux `USE_GLES` pool — off-screen ShaderEffect pool via
  `xlAngleEGL::CreatePbufferContext`.
- 🔧 **GLES3 compile path** (8 GL TUs): under `USE_GLES`, ES3 prototypes (no `wgl` loader),
  `#version 300 es` shaders, shader-based points. **7/8 structurally converted:**
  `OpenGLShaders.cpp` (validated), `ShaderEffect.cpp`, `GLContextManager.cpp` (validated;
  off-screen pool via `xlAngleEGL` pbuffers), `Waveform/EffectsGrid/ModelPreview.cpp` (header
  swap only — no direct `gl*`), and `xlGLCanvas.cpp/.h` (EGL window surface via `xlAngleEGL`
  in `CreateGLContext`/`SetCurrent`/`FinishDrawing`/dtor; desktop debug-ARB code gated;
  Windows HWND wired, **Linux native-handle extraction TBD**). **Remaining: `xlOGL3GraphicsContext.cpp`**
  — include/loader gated, but still needs the ~15 embedded GLSL shaders ported to
  `#version 300 es` and `glPointParameterf`/`GL_POINT_SMOOTH` point rendering replaced with a
  shader-based path. Nothing GPU-tested yet (requires a real Windows ANGLE build + run).

## Goal & rationale

Let users run xLights' OpenGL rendering through **ANGLE** (EGL → OpenGL ES 3.0, backed
by D3D11 on Windows / Vulkan on Linux) instead of the native GL driver. This fixes the
recurring class of bugs where xLights is unusable on **bad/old/virtualized GL drivers** —
RDP sessions, VMs, old Intel GPUs, the Windows software GDI fallback (GL 1.1). ANGLE
ships a known-good ES implementation, so "update your video drivers" stops being the
only answer.

Decisions:
- **Platforms:** Windows + Linux. (macOS stays Metal-native; its shader pool already
  uses ANGLE.)
- **Render paths:** both — on-screen wx canvases *and* the off-screen `ShaderEffect` pool.
- **Integration:** runtime opt-in with automatic fallback — native GL stays the default
  fast path, ANGLE is selectable in settings and auto-engaged when native GL is detected
  as software/blacklisted.

## Current-state facts driving the design

xLights has **two independent GL surfaces**, and ANGLE must cover both:

1. **On-screen** — `xlGLCanvas : wxGLCanvas` (`src-ui-wx/graphics/opengl/xlGLCanvas.cpp`).
   Context created via `wxGLContext` requesting GL 3.3 core (`xlGLCanvas.cpp:538-539`),
   with 2.1 and platform-default fallbacks. Draws every preview, the layout, and the
   sequencer. `GRAPHICS_BASE_CLASS` is a **compile-time** typedef in
   `src-ui-wx/graphics/xlGraphicsBase.h`.
2. **Off-screen** — `GLContextManager` (`src-core/graphics/GLContextManager.cpp`), a
   pooled context manager used by `ShaderEffect`. **Already has a full EGL/ANGLE path** —
   but `#ifdef USE_GLES` is wired only for Apple (ANGLE-on-Metal). Windows uses raw WGL
   (`GLContextManager.cpp:508-974`); Linux uses GLX with an EGL pbuffer fallback
   (`GLContextManager.cpp:987-1333`).

Key technical constraints found:
- **Shaders are desktop-GLSL only.** Embedded on-screen shaders in
  `xlOGL3GraphicsContext.cpp` use `#version 330 core` and a `#version 120` fallback (no
  ES variant). `ShaderEffect.cpp` *already* emits `#version 300 es` under `USE_GLES`.
- **Desktop-GL-only calls in the on-screen path:** `glPointParameterf`
  (`xlOGL3GraphicsContext.cpp:83,146`) and `GL_POINT_SMOOTH` (`:1295-1373`) — neither
  exists in GL ES. Point rendering must move to a shader-based path for ANGLE.
- **GL is loaded manually** via `wglGetProcAddress`/`glXGetProcAddress` against desktop
  `<GL/gl.h>` headers (`xlOGL3GraphicsContext.cpp:105-177`). ANGLE exposes ES 3.0 via
  `libGLESv2`/`libEGL` — a different loading/linking model.
- `USE_GLES` is currently a build flag (`macOS/ANGLE.xcconfig`), not a runtime switch.
- No `wxGLCanvasEGL`/`EGL` references in the vendored wx fork headers — **wx-on-Windows
  uses WGL**; the on-screen ANGLE path likely can't go through `wxGLCanvas` and needs a
  custom EGL canvas (see Phase 4 spike).
- Deps are moving to vcpkg/conan (`conanfile.py`, `conan/`, `vcpkg_installed/` in tree).
  vcpkg has an `angle` port — that's the delivery vehicle for `libEGL`/`libGLESv2`.

## Design overview

Introduce a **runtime-selectable GL backend** with two axes:

- **Backend enum** `{ Native, ANGLE }`, persisted as an xLights preference and overridable
  via env var / command-line for support triage. A startup auto-detector promotes
  `Native → ANGLE` when the native renderer string matches a software/blacklist heuristic
  (e.g. `GDI Generic`, `llvmpipe`, GL version `< 3.0`).
- A **GL entry-point loader abstraction** so the same `xlOGL3GraphicsContext` drawing code
  can bind either the native desktop-GL functions or ANGLE's `libGLESv2` functions at
  runtime (`eglGetProcAddress`).

Because `GRAPHICS_BASE_CLASS` is compile-time today, the on-screen canvas needs to become
**runtime-polymorphic** between the WGL/GLX `wxGLCanvas` path and a new EGL/ANGLE canvas.

## Phased implementation

### Phase 0 — Dependency & build plumbing
The repo has **two** Windows build paths with different dependency models, so ANGLE
plumbing differs per path:

- **CMake + vcpkg/conan path** (`cmake_vs/`, Linux) — DONE in source:
  - `vcpkg.json` gains an `angle` dependency, restricted to `windows | linux` (macOS
    uses its own ANGLE via `macOS/ANGLE.xcconfig`).
  - `CMakeLists.txt` adds `option(XLIGHTS_USE_ANGLE ... ON)`, then
    `find_package(unofficial-angle CONFIG)`; when the `unofficial::angle::libEGL` /
    `libGLESv2` targets exist it links them and defines **`HAVE_ANGLE=1`**.  vcpkg's
    applocal post-build step deploys `libEGL.dll`/`libGLESv2.dll`; on Linux the `.so`s
    resolve via the vcpkg-installed lib dir.  If ANGLE isn't found the build degrades to
    native-GL-only (no `HAVE_ANGLE`) — never a hard failure.
  - `conanfile.py` documents that Conan Center has no `angle` recipe; a Conan build wanting
    ANGLE must point `CMAKE_PREFIX_PATH` at an install shipping `unofficial-angle`.
- **MSBuild path** (`Xlights.vcxproj`, `VcpkgEnabled=false`) — DONE (binaries vendored from
  the vcpkg `angle:x64-windows` build, `chromium_7258`):
  - Headers extracted to `include/` — `EGL/` (incl. `eglext_angle.h`), `GLES2/`, `GLES3/`,
    `KHR/khrplatform.h`.  Already on the `..\include` include path.
  - Import libs `libEGL.lib`/`libGLESv2.lib` → `lib/windows64/` (already on the lib path).
  - Runtime DLLs `libEGL.dll`, `libGLESv2.dll`, **`z.dll`** (zlib — `libGLESv2.dll` links it
    dynamically; xLights otherwise static-links zlib) → `bin64/`, force-added past the
    `bin64/*.dll` ignore rule like the other vendored runtime DLLs (SDL2, avcodec, …).  The
    existing PostBuildEvent copies `bin64\*.dll` to the target dir, so no PostBuildEvent edit.
  - `HAVE_ANGLE` added to the **x64** Debug/Release `PreprocessorDefinitions` (ARM/ARM64EC
    configs have no ANGLE binaries and are left native-GL-only).
  - Remaining: the actual `#pragma comment(lib, "libEGL"/"libGLESv2")` link directives land
    with the Phase 2 ANGLE code in `GLContextManager.cpp` (guarded by
    `defined(_WIN32) && defined(HAVE_ANGLE) && !defined(XLIGHTS_CMAKE_BUILD)`), since nothing
    references the libs until then.
- **`HAVE_ANGLE` is the compile gate:** all Phase 2+ EGL/GLES code is wrapped in
  `#ifdef HAVE_ANGLE`, and the runtime resolver only returns ANGLE when the build defines it.
  This keeps every build green regardless of whether ANGLE binaries are present.
- **CI decision (settled):** `angle` is deliberately NOT added to `ci_scripts/vcpkg.json` —
  the ANGLE port is a long, disk-heavy vcpkg build and CI shouldn't pay it.  Consequence:
  CI does **not** compile-cover the `#ifdef HAVE_ANGLE` paths, so ANGLE code must be built
  and exercised **locally** (the full-manifest dev/release `vcpkg.json` does carry `angle`,
  and the MSBuild build defines `HAVE_ANGLE` via the vendored binaries).  Be careful that
  nothing under `#ifdef HAVE_ANGLE` is required for a correct native-GL build.
- No code behavior change in this phase — it only makes ANGLE headers/libs available and sets
  the gate macro.

### Phase 1 — Backend selection core (no rendering change)
- New `src-core/graphics/GLBackend.h/.cpp`: an enum + a global resolver
  `GLBackend ResolveActiveBackend()` reading the preference / env override, plus the
  software-renderer auto-detect heuristic (fed the `GL_RENDERER`/`GL_VERSION` string after
  a probe). **Must stay wx-free** (core boundary rule).
- Wire a **preference** ("Rendering Backend: Native / ANGLE / Auto") into the existing
  xLights settings UI (`src-ui-wx`), defaulting to **Auto**. Add a one-line README.txt
  enhancement entry.
- This phase compiles and ships with ANGLE selectable but not yet used — selection just
  logs.

### Phase 2 — ANGLE in the off-screen `ShaderEffect` pool (lowest-risk, highest-isolation)
This is the smallest, most contained win and validates the whole toolchain.
- Generalize `GLContextManager.cpp`'s `USE_GLES` EGL implementation so it's reachable on
  **Windows and Linux at runtime** (not just Apple at compile time). Refactor the Apple
  `#ifdef USE_GLES` EGL block into a platform-neutral `EGL/ANGLE` backend variant selected
  by `GLBackend`, sitting alongside the existing WGL (Windows) and GLX (Linux) variants.
  - Windows: when backend == ANGLE, `AcquireContext` builds EGL pbuffer contexts via
    `eglGetPlatformDisplay(EGL_PLATFORM_ANGLE_ANGLE, …, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)`
    instead of WGL dummy-HWND contexts. The existing worker-thread serialization
    (`ExecuteOnGLThread`) is kept.
  - Linux: add an explicit ANGLE EGL display type to the existing EGL fallback path; reuse
    the pbuffer logic already there.
- `ShaderEffect.cpp` already emits ES shaders under `USE_GLES`; convert that compile-time
  gate to the runtime backend check so ES shader source is used when the active backend is
  ANGLE.
- **Verify:** shaders render identically through ANGLE on a Win + Linux box.

### Phase 3 — ES-compatible shaders & primitives for the on-screen context
Prepare `xlOGL3GraphicsContext` to run under ES before swapping the canvas.
- Add `#version 300 es` variants (with `precision highp float;` / sampler precision) for
  all embedded programs in `xlOGL3GraphicsContext.cpp` (singleColor3 / texture3 / normal3 /
  meshTexture / meshSolid), selected by active backend. Keep the existing 330-core/120
  desktop variants.
- Replace `glPointParameterf` + `GL_POINT_SMOOTH` point rendering with the **shader-based
  point path** for ES (compute point size in the vertex shader, antialias via
  `gl_PointCoord` in the fragment shader — the fragment shaders already use `gl_PointCoord`).
  Gate the desktop fixed-function point calls behind backend == Native.
- Audit remaining draw calls for other ES gaps (e.g. `glMapBufferRange` flags, FBO formats,
  `GL_BGRA` uploads, default VAO assumptions). Fix any found.

### Phase 4 — On-screen ANGLE canvas (largest, highest-risk)
**Spike DONE (2026-06-04):** the wx fork is **WGL-only on Windows** (`src/msw/glcanvas.cpp`
uses `wglCreateContext`/`GetHDC`; no EGL).  Its EGL canvas (`src/unix/glegl.cpp`, guarded by
`wxUSE_GLCANVAS && wxHAS_EGL`) is **wxGTK-only** and binds the system `libEGL`, not ANGLE's.
→ Outcome **(B)**: add a custom `xlEGLCanvas` on **both** Windows and Linux that creates an
ANGLE EGL **window surface from the native handle** (`GetHandle()` HWND on Windows, the
X11/Wayland handle on Linux) via `eglGetPlatformDisplay` + `eglCreateWindowSurface`, and
presents via `eglSwapBuffers`.  This keeps full control over using *ANGLE's* `libEGL`
uniformly, rather than relying on wx's GTK-only EGL canvas.

**IMPORTANT — whole-process finding:** the off-screen ShaderEffect pool and the on-screen
canvas **share one global `gl*` function-pointer table** (the `extern PFNGL…PROC` symbols
defined in `xlOGL3GraphicsContext.cpp`, loaded once via `wglGetProcAddress`/`glXGetProcAddress`
and used by `OpenGLShaders.cpp` + `ShaderEffect.cpp` too).  ANGLE's `libGLESv2` exports the
same `gl*` names as different entry points that must come from `eglGetProcAddress`.  A single
process therefore cannot mix native on-screen + ANGLE off-screen — **the GL backend is a
whole-process choice** (consistent with the resolver being an app-wide startup decision).
Phases 2/3/4 are thus implemented together as one "ANGLE-mode" switch (chosen by the user) —
not as independently shippable steps on Windows/Linux.

Then:
- Convert `xlGraphicsBase.h`'s compile-time `GRAPHICS_BASE_CLASS` typedef into a **runtime
  factory** on Windows/Linux: `ModelPreview` and the other canvas hosts construct either
  `xlGLCanvas` (Native) or `xlEGLCanvas` (ANGLE) based on `ResolveActiveBackend()`. macOS
  Metal path is untouched.
- Route on-screen GL function loading through the loader abstraction (Phase 1) so
  `xlOGL3GraphicsContext` binds `libGLESv2` entry points when the canvas is ANGLE.
- Because backend is chosen at startup before any canvas exists, selection can be a
  **restart-required** setting initially (simplest; avoids tearing down live GL objects).
  A later enhancement could hot-swap.

### Phase 5 — Auto-fallback, robustness, and polish
- Hook the Phase 1 auto-detector into real startup: probe native GL once, and if it's
  software/blacklisted and the user is on **Auto**, switch to ANGLE and log it prominently.
- If ANGLE init fails (missing DLLs, D3D unavailable), fall back to Native and surface a
  clear log/notice — never a black window.
- Add a diagnostics line to the existing GL-info logging showing the **active backend** and
  the resolved ANGLE backend (D3D11/Vulkan), to make support tickets triagable.
- Final `NO_PCH` desktop build + iPad lib build sanity (Phases 2/3 touch `src-core/`, so the
  iPad/Metal `USE_GLES` path must still compile — keep the Apple branch behavior
  byte-for-byte).

## Risks & open questions
- **wx EGL support on Windows** is the biggest unknown — Phase 4 spike resolves it and gates
  the on-screen path. If the spike is hard, Phases 0–3 + 5 still deliver real value (ANGLE
  for the shader pool) and on-screen ANGLE can ship later.
- **ES shader parity:** subtle differences (precision, integer handling, no `gl_FragColor`)
  can change pixels. Mitigate with side-by-side visual diffs in Phase 3.
- **DLL shipping / load order:** ANGLE's `libGLESv2.dll` must not collide with the native
  `opengl32`. Stage them in the app dir and load explicitly.
- **`-ffast-math`** is macOS-only today; the new core code (`GLBackend`, EGL paths) must
  follow the `xl::isnan`/sentinel rules per CLAUDE.md regardless.
- Per CLAUDE.md core-boundary rule: `GLBackend.*` and the `GLContextManager` changes live in
  `src-core/` and **must stay wx-free**; the canvas/preference/UI changes live in
  `src-ui-wx/`.

## Suggested landing order (incremental, each independently shippable)
1. Phase 0 + 1 — deps + selectable preference (no behavior change).
2. Phase 2 — ANGLE shader pool on Win/Linux (isolated, testable).
3. Phase 3 — ES-ready on-screen shaders (still drawn by native canvas).
4. Phase 4 — on-screen ANGLE canvas (after spike).
5. Phase 5 — auto-fallback + diagnostics.

## Key files
- `src-ui-wx/graphics/opengl/xlGLCanvas.cpp` — on-screen canvas / context creation
  (`:521-633`).
- `src-ui-wx/graphics/xlGraphicsBase.h` — compile-time `GRAPHICS_BASE_CLASS` typedef to
  make runtime.
- `src-ui-wx/graphics/opengl/xlOGL3GraphicsContext.cpp` — GL func loading (`:105-177`),
  embedded shaders (`#version` strings), desktop-only point rendering (`:1295-1373`).
- `src-core/graphics/GLContextManager.cpp` — off-screen pool; existing Apple ANGLE/EGL path
  (`:26-269`), Windows WGL (`:508-974`), Linux GLX/EGL (`:987-1333`).
- `src-core/graphics/GLContextManager.h` — `InitParams`, public API.
- `src-core/effects/ShaderEffect.cpp` / `src-core/effects/OpenGLShaders.cpp` — existing
  `USE_GLES` ES shader generation to flip to runtime.
- `macOS/ANGLE.xcconfig` — existing macOS ANGLE build config (reference / don't regress).
- `CMakeLists.txt`, `xLights/Xlights.vcxproj` (+ `.filters`), `conanfile.py` — build/deps.
