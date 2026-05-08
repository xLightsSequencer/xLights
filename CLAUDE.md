# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

xLights is a cross-platform (Windows/macOS/Linux) C++20 show sequencer for controlling lighting via USB/DMX/sACN/ArtNet/DDP. Built on wxWidgets 3.3 (custom fork). Companion apps (xSchedule, xCapture, xFade, xScanner) have been moved to their own repositories.

**Minimum supported platforms:** macOS 10.15 (Catalina), Linux (Debian 12 / Ubuntu 24.04), Windows 8.

### iPad app

This repository also contains the **iPad app** (SwiftUI, iOS 26+). It is
NOT a port — it's a second UI that shares the same wx-free C++ core
(`src-core/`), render engine, effect manager, sequence file / elements,
and audio manager as the desktop app. iPad-specific code lives in
`src-iPad/` (SwiftUI views + view model, ObjC++ bridges,
Metal canvases). The iPad app is pre-release and tracked in
[`iPad-xLights-Plan.md`](iPad-xLights-Plan.md) + `plans/`.

### Desktop ↔ iPad parity (important)

Any change that touches a feature that exists on both platforms
**should have equivalent changes applied to the other** in the same
PR whenever possible. Concretely:

- **Changes to `src-core/`** affect both apps automatically (both link
  the same core). Review both code paths when touching core — a new
  edge case in `src-core/render/` or `src-core/effects/` needs to
  behave correctly through the wx UI *and* the SwiftUI UI.
- **UI-layer features in `src-ui-wx/`** that have an iPad counterpart
  in `src-iPad/App/` should be updated together. Example: a new
  entry in the desktop row-heading right-click menu should also
  land in `ModelRowHeader.contextMenu` on iPad (and vice versa).
- **Bridge surface in `src-iPad/Bridge/`** mirrors or wraps desktop
  behavior. When the desktop adds a new mutating op on `Effect` /
  `EffectLayer` / `Element` / `SequenceElements`, the iPad bridge
  usually needs a matching wrapper. Follow the
  `NS_SWIFT_NAME(…)` convention already in `XLSequenceDocument.h`.
- **Features iPad can't support** (e.g. controller setup, layout
  editor, FFmpeg-only audio filters) are fine to leave desktop-only
  — track them in the "Deferred / out of MVP" section of
  `iPad-xLights-Plan.md` so we know why.
- **Features iPad has that desktop doesn't** (rare so far — two-
  finger marquee, long-press menus, trackpad `allowedScrollTypesMask`)
  are fine; they're touch-idioms without desktop equivalents.

When unsure, err toward matching: it's cheaper to add a parallel UI
entry than to ship a behavior gap that users discover when they
move between the two clients.

## Build Commands

### macOS (desktop)
```bash
# Xcode project at macOS/xLights.xcodeproj
xcodebuild                    # CLI build
# Or open in Xcode. Requires Xcode 26.3 or newer.
# Dependencies auto-download if using matching Xcode version.
# Libraries install to /opt/local/lib (release) and /opt/local/libdbg (debug)
```

### iPad (SwiftUI app)
```bash
# Same Xcode project. iPad deps live at /opt/xLights-macOS-dependencies/lib-ios/.
xcodebuild -project macOS/xLights.xcodeproj \
           -scheme xLights-iPadLib \
           -configuration Debug \
           -destination 'generic/platform=iOS' build
```
`xLights-iPadLib` builds the static library that the
`xLights-iPad` app target links against. For fast iterative
checks prefer this scheme — it skips the app bundle wrap and
catches the vast majority of errors. Build the `xLights-iPad`
scheme only when you need to deploy to a device / simulator.

### Linux
```bash
make                          # Full build (wxWidgets + deps + all apps)
make debug                    # Debug build of all apps
make clean                    # Clean all
make xLights                  # Build just xLights (release)
# Output binaries go to bin/
```

Build uses Code::Blocks .cbp project files converted to makefiles via cbp2make. Object files go to `.objs_debug/` or `.objs_release/`.

### Windows
```
msbuild -restore -m:10 xLights.sln -p:Configuration="Release" -p:Platform="x64"
```
Open in Visual Studio (vcxproj files) or Code::Blocks.

### wxSmith Generated Code
Some dialogs and panels use wxSmith (wxWidgets RAD tool). Generated code is delimited by `//(* ... //*)` guards in `.cpp`/`.h` files. **Any changes within these guards MUST also be reflected in the corresponding `.wxs` file** in `src-ui-wx/wxsmith/`. Otherwise the changes will be overwritten the next time the `.wxs` file is opened in wxSmith. If adding new controls, event handlers, or modifying existing ones inside the guards, update the `.wxs` XML to match.

### Adding New Source Files
When adding new `.cpp`/`.h` files, place them in `src-core/` (wx-free
core, linked by both desktop and iPad), `src-ui-wx/` (wxWidgets
desktop UI), or `src-iPad/` (SwiftUI iPad app + ObjC++ bridge). The
following project files must be updated manually:
- **`xLights/xLights.cbp`** — add `<Unit filename="...">` entries with paths relative to `xLights/` (e.g., `../src-core/render/Foo.cpp`)
- **`xLights/Xlights.vcxproj`** — add `<ClCompile>` for `.cpp` and `<ClInclude>` for `.h` with paths relative to `xLights/` (e.g., `..\src-core\render\Foo.cpp`)
- **`xLights/Xlights.vcxproj.filters`** — add corresponding filter entries to place files in the correct VS folder

The macOS Xcode project (`macOS/xLights.xcodeproj/project.pbxproj`)
uses `PBXFileSystemSynchronizedRootGroup` for `src-core/`,
`src-ui-wx/`, and `src-iPad/` — files in those directories are
auto-discovered. No manual pbxproj editing needed for new files in
existing directories. Note that Windows/Linux builds intentionally
do not compile `src-iPad/`, so new iPad files never need to land in
the `.cbp` / `.vcxproj` files.

### Release Notes
`README.txt` contains ongoing release notes at the top of the file. When implementing new features or fixing bugs, add a single very brief summary line with no code to the current release section:
- **Enhancements**: `    -enh (author)                Description of the enhancement`
- **Bug fixes**: `    -bug (author)                Description of the bug fix`
- **Changes**: `    -change (author)             Description of the change`

Keep description summary very brief (1-2 lines). Indent continuation lines to align with the description start. If the release at the top has a concrete date with no ? in it, start a new release above it.

**Do NOT add iPad-specific changes to `README.txt`.** The iPad app has not shipped yet and its changelog would only clutter the desktop release notes. Instead, keep `iPad-xLights-Plan.md` (next to `README.txt`) up to date: move items from "pending" to "done" as they land, and record any landed-fix details (root cause, follow-ups) inside the matching phase entry. Changes that touch shared `src-core/` code *and* user-visible desktop behavior still belong in `README.txt`; the iPad-only side goes in the plan.

### Verifying Changes
After making code changes (especially during code reviews), always do a build to make sure nothing is broken. On macOS, use:
```bash
xcodebuild -configuration Debug   # Fastest iterative build (Debug, single arch, uses PCH)
xcodebuild                        # Normal iterative build (Release, universal, uses PCH)
xcodebuild GCC_PREPROCESSOR_DEFINITIONS='$(inherited) NO_PCH '   # Final verification only
```
Adding `-configuration Debug` builds only for the native architecture in Debug mode, which is much quicker for iterative build testing during development. The default Release build is universal (arm64 + x86_64) and optimized, so it takes longer. The `NO_PCH` flag disables pre-compiled headers, making the macOS build closer to Linux/Windows behavior and helping catch missing `#include` directives that the PCH would otherwise mask. However, disabling PCH significantly slows the build, so use it only as a final verification before committing — not for iterative development.

**When a change touches `src-core/` or `src-iPad/`, also build the
iPad library** to catch platform-specific breaks that the desktop
build won't surface (e.g. `#ifdef __APPLE__` paths, ObjC++ bridge
compilation, Swift interop):
```bash
xcodebuild -project macOS/xLights.xcodeproj -scheme xLights-iPadLib \
           -configuration Debug -destination 'generic/platform=iOS' build
```

## Architecture

### Core Application (`src-core/`) and UI (`src-ui-wx/`)
- **Entry point**: `src-ui-wx/xLightsApp.cpp` (wxApp subclass) → `src-ui-wx/xLightsMain.cpp` (`xLightsFrame`, the main window)
- **Sequencer**: `src-ui-wx/sequencer/` — timeline UI, effect grid, effect layers, undo manager. `MainSequencer` is the primary sequencer widget; `tabSequencer.cpp` handles sequencer-related event handlers on xLightsFrame.
- **Effects**: `src-core/effects/` — 55 effects, each as a pair: `FooEffect.cpp` (rendering logic, `adjustSettings` for migration) + `FooPanel.cpp` (UI controls). All inherit from `RenderableEffect`. See [Effects Reference](#effects-reference) below.
- **Models**: `src-core/models/` — 20+ model types plus 9 DMX models. All inherit from `Model`. See [Models Reference](#models-reference) below.
- **Outputs**: `src-core/outputs/` — protocol handlers and controller connection config. See [Outputs & Controllers Reference](#outputs--controllers-reference) below.
- **Controllers**: `src-core/controllers/` — vendor-specific hardware upload handlers (Falcon, FPP, WLED, etc.). See [Outputs & Controllers Reference](#outputs--controllers-reference) below.
- **Media**: `src-core/media/` — audio decoding, encoding, and playback. Platform-neutral `AudioManager` with abstract interfaces (`IAudioDecoder`, `IAudioOutput`). On Apple platforms uses AudioToolbox (decoding) and AVAudioEngine (playback); on Linux/Windows uses FFmpeg and SDL2. Also contains `xLightsVamp` (VAMP audio analysis plugins) and `AudioLoader` (specialized FFmpeg-based loader).
- **Discovery core**: `src-core/discovery/` — shared controller/output discovery data structures and discovery API used by core layers.
- **XmlSerializer**: `src-core/XmlSerializer/` — XML serialization/deserialization for models, objects, and rgb effects. Includes GDTF parser. Core layer — should not depend on wx or UI.
- **SequenceMedia** (`src-core/render/SequenceMedia.cpp`): Manages image caching and embedding for sequences. Images can be embedded in .xsq files or referenced externally. Uses `FixFile()` to resolve relative paths.

### Core vs UI Layer Architecture

The codebase physically separates wx-free core logic from wxWidgets UI code. Core code lives in `src-core/`, UI code in `src-ui-wx/`. The goal is a platform-neutral core usable without wx (e.g., for an iPad app). Build system files remain in `xLights/`.

**Core rule:** anything under `src-core/` **must not** include anything from `src-ui-wx/` / `src-ui*/` or any `wx/…` header. This applies to every file in every core package — public headers and implementation files alike. The goal is that `src-core/` compiles and links without wxWidgets so the iPad app (and any future non-wx host) can reuse it unchanged. Core packages enforced by `ci_scripts/check_core_include_boundaries.sh`: `discovery/`, `graphics/`, `lyrics/`, `media/`, `render/`, `effects/`, `models/`, `outputs/`, `controllers/`, `utils/`, `XmlSerializer/`. New violations are blocked in strict mode; approved exceptions live in `ci_scripts/include_policy_allowlist.txt`.

**`src-core/graphics/`** — wx-free core graphics abstraction layer:
- `xlGraphicsContext.h` — abstract GPU context interface (no wx types; no window pointer; use `setContextualValue`/`getContextualValue` for passing context like `IModelPreview*`)
- `xlGraphicsAccumulators.h` — geometry/vertex accumulator interfaces
- `IModelPreview.h` — wx-free pure-virtual interface for model preview (used by models/ and effects/)
- `xlFontInfo.h` — font metadata using `xlImage` (no wxImage in public API)
- `xlImage.h` — wx-free RGBA pixel class (see also `utils/xlImage.h`)
- `xlMesh.h/.cpp` — 3D mesh loading (std::filesystem, no wx)

**`src-ui-wx/graphics/`** — wx-dependent canvas implementations (NOT core):
- `xlGraphicsBase.h` — selects Metal vs OpenGL canvas at compile time; defines `GRAPHICS_BASE_CLASS`
- `opengl/xlGLCanvas.h/.cpp` — OpenGL canvas (wxGLCanvas subclass)
- `opengl/xlOGL3GraphicsContext.h/.cpp` — OpenGL graphics context implementation
- `metal/xlMetalCanvas.h/.mm` — Metal canvas (macOS)
- `metal/xlMetalGraphicsContext.h/.mm` — Metal graphics context implementation

**Include conventions:**
- Core files (`graphics/`, `models/`, etc.) include `graphics/IModelPreview.h` — never `layout/ModelPreview.h` (which is the wx-dependent subclass)
- Canvas/UI files include `graphics/xlGraphicsBase.h` — this resolves to `src-ui-wx/graphics/xlGraphicsBase.h` because `src-core/graphics/` has no file by that name
- wx↔core conversion helpers (e.g., `wxImageToXlImage()`, `wxImagesToXlImages()`) live in `shared/utils/wxUtilities.h`

### Key Patterns
- **Settings/SettingsMap**: Effects store settings as string key-value pairs (e.g., `E_TEXTCTRL_Pictures_Filename`). Prefix conventions: `E_` for effect settings, `T_` for transitions, `B_` for buffer settings, `C_` for color.
- **`adjustSettings()`**: Called on each effect when loading sequences to migrate old settings to current format. NOT called for newly created effects (e.g., drag-and-drop).
- **`FixFile()`**: Resolves file paths by searching show directory, media directories, and common locations. Used throughout for portable path resolution.
- **`MakeRelativePath()`/`IsInShowOrMediaFolder()`**: Convert absolute paths to relative (for portability) and check file location. Relative paths are relative to the show directory or a media directory.
- **Show directory vs Media directories**: The show directory is the primary project folder. Additional media directories can be configured. Both are searched when resolving relative paths.
- **`ObtainAccessToURL(path, enforceWritable)`**: Must be called before reading/writing files on macOS to handle App Sandbox security-scoped bookmarks. Defined in `macOS/macOS-src/osxUtils/ExternalHooksMacOS.h`, implemented in Swift (`xlMacUtils.swift`). Returns `bool` — `true` if access granted. Pass `enforceWritable=true` when writing. Bookmarks are persisted in UserDefaults so access survives app restarts. No explicit release call needed. On non-macOS platforms this is a no-op. Call `ObtainAccessToURL` whenever a path comes from user input (file dialogs, drag-and-drop, text fields, etc.) so that persistent bookmarks are created/updated for that path. This ensures future access to the file/directory works even after app restart.

### iPad app layout (`src-iPad/`)

The iPad app is a SwiftUI client that shares `src-core/` with the
desktop and adds its own UI + bridge layers.

```
src-iPad/
  App/            SwiftUI views + @Observable view model
                  (SequencerViewModel, SequencerView, SequencerGridV2View,
                   EffectsMetalGridView, TimingEffectsMetalGridView,
                   TopChromeMetalGridView, RowHeaderViews, …)
  Bridge/         ObjC++ bridges (XLSequenceDocument, iPadRenderContext,
                  XLiPadInit, CoreGraphicsTextDrawingContext,
                  XLValueCurve)
  Metal/          xlStandaloneMetalCanvas, iPadModelPreview,
                  XLMetalBridge, iPadGridPreview, XLGridMetalBridge
  Metadata/       EffectMetadata.swift (JSON model for effectmetadata/*.json)
```

Key patterns:

- **`XLSequenceDocument`** (`src-iPad/Bridge/`) is the ObjC++ bridge
  for Swift callers. Every method here is callable from Swift; use
  `NS_SWIFT_NAME(…)` to control the imported Swift name when the
  default transformation is awkward.
- **`iPadRenderContext`** (`src-iPad/Bridge/`) subclasses
  `RenderContext` from `src-core/render/` — it's the iPad's
  concrete render context implementation, mirroring
  `xLightsFrame`'s role in the desktop app.
- **`SequencerViewModel`** (`src-iPad/App/`) is the single
  `@Observable` class that SwiftUI views read. All mutating ops go
  through it so undo registration + row reloads happen in one
  place.
- **Metal-backed grid**: `EffectsMetalGridView` /
  `TimingEffectsMetalGridView` / `TopChromeMetalGridView` render
  via `XLGridMetalBridge` → `xlStandaloneMetalCanvas` →
  `xlMetalGraphicsContext`. The iPad grid is Metal-only; the
  desktop grid uses `xlGraphicsBase`'s OpenGL/Metal switch.
- **Sub-plans**: `plans/phase-b-grid-parity.md` tracks the gap
  analysis against desktop behavior. 

### Data Formats
- `.xsq` — Sequence files (XML-based, can contain embedded images as base64)
- `.fseq` — Binary playback format for Falcon Player
- `.xtiming` — Standalone timing-track export (`<timing>` or
  `<timings>` wrapper XML, read/written by both desktop and iPad).
- `.pgo` — Papagayo lipsync file (desktop import only today).
- Sequence settings stored as XML attributes with typed prefixes

### Sub-applications
Companion apps (xSchedule, xCapture, xFade, xScanner) have been moved to their own repositories under the xLightsSequencer GitHub organization. Only `xlDo/` remains in this repo.

## Code Style

- C++20 with GNU extensions (`-std=gnu++20`)
- 4-space indentation, no tabs (`.clang-format` configured)
- No column limit (ColumnLimit: 0)
- Opening braces on same line (K&R style)
- Match the style of nearby code — the codebase is not perfectly consistent
- Avoid purely cosmetic changes in PRs
- **Comments**: Default to writing no comments. Only add one when the WHY is non-obvious: a hidden constraint, a subtle invariant, a workaround for a specific bug, or behavior that would surprise a reader. Don't explain what the code does (well-named identifiers do that), and don't reference the current task or fix (those belong in the commit message).

## Prefer std::* Over wx* Types

Core data types and algorithms should use standard C++ equivalents rather than wxWidgets types, to keep non-UI code portable and wx-free:

- **Strings**: Use `std::string` instead of `wxString`. Convert at wx API boundaries with `.ToStdString()` / `wxString(str)`.
- **Collections**: Use `std::vector`, `std::map`, `std::unordered_map`, etc. instead of `wxArrayString`, `wxList`, etc.
- **Colors**: Use `xlColor` (defined in `src-core/utils/Color.h`) instead of `wxColour`. `xlColor` is now wx-free.
- **wx↔std conversions**: Place any `wxColour`/`wxString`/wx-collection ↔ std conversion helpers in `src-ui-wx/shared/utils/wxUtilities.h` (and `.cpp` for non-inline implementations). Currently provides:
  - `xlColorToWxColour(const xlColor&) → wxColour`
  - `wxColourToXlColor(const wxColour&) → xlColor`
- **When wx constants suffice**: If a wx API accepts a wx constant directly (e.g. `*wxBLACK`, `*wxWHITE` for `SetTextForeground`), use the wx constant rather than converting an `xl*` constant through `xlColorToWxColour`.
- **Exceptions**: xLights has nearly non-existent exception handling — do NOT use `std::stoi`, `std::stol`, `std::stod`, etc. as they throw on invalid input. Use `std::strtol`, `std::strtod` (and friends) instead. These return 0/default on bad input without throwing.
- **File existence checks**: Use `FileExists()` from `ExternalHooks.h` instead of `std::filesystem::exists()` or `wxFile::Exists()` directly. On macOS, `FileExists()` triggers iCloud downloads for files that have been evicted to the cloud, which `std::filesystem::exists()` does not. For directory existence, use `std::filesystem::exists()` with the `std::error_code` overload (to avoid exceptions).

## Key Dependencies

wxWidgets 3.3 (custom fork `xLightsSequencer/wxWidgets`), spdlog, nlohmann/json, FFmpeg, SDL2, Lua 5.4, libcurl, zstd, LiquidFun/Box2D, libxlsxwriter, nanosvg, ISPC (SIMD kernels).

