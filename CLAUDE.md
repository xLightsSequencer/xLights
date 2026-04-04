# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

xLights is a cross-platform (Windows/macOS/Linux) C++20 show sequencer for controlling lighting via USB/DMX/sACN/ArtNet/DDP. Built on wxWidgets 3.3 (custom fork). Companion apps (xSchedule, xCapture, xFade, xScanner) have been moved to their own repositories.

**Minimum supported platforms:** macOS 11, Linux (Debian 12 / Ubuntu 24.04), Windows 8.

## Build Commands

### macOS
```bash
# Xcode project at macOS/xLights.xcodeproj
xcodebuild                    # CLI build
# Or open in Xcode. Requires Xcode 26.3 or newer.
# Dependencies auto-download if using matching Xcode version.
# Libraries install to /opt/local/lib (release) and /opt/local/libdbg (debug)
```

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
Open in Visual Studio (vcxproj files) or Code::Blocks.

### wxSmith Generated Code
Some dialogs and panels use wxSmith (wxWidgets RAD tool). Generated code is delimited by `//(* ... //*)` guards in `.cpp`/`.h` files. **Any changes within these guards MUST also be reflected in the corresponding `.wxs` file** in `xLights/wxsmith/`. Otherwise the changes will be overwritten the next time the `.wxs` file is opened in wxSmith. If adding new controls, event handlers, or modifying existing ones inside the guards, update the `.wxs` XML to match.

### Adding New Source Files
When adding new `.cpp`/`.h` files, the following project files must be updated manually:
- **`xLights/xLights.cbp`** — add `<Unit filename="...">` entries (used by Linux build via cbp2make)
- **`xLights/Xlights.vcxproj`** — add `<ClCompile>` for `.cpp` and `<ClInclude>` for `.h`
- **`xLights/Xlights.vcxproj.filters`** — add corresponding filter entries to place files in the correct VS folder

The macOS Xcode project (`macOS/xLights.xcodeproj/project.pbxproj`) may also need updating — some directories use `PBXFileSystemSynchronizedRootGroup` (auto-discovered), others require manual `PBXFileReference`/`PBXBuildFile` entries. Check on a case-by-case basis.

### Release Notes
`README.txt` contains ongoing release notes at the top of the file. When implementing new features or fixing bugs, add a line to the current release section:
- **Enhancements**: `    -enh (author)                Description of the enhancement`
- **Bug fixes**: `    -bug (author)                Description of the bug fix`
- **Changes**: `    -change (author)             Description of the change`

Keep descriptions concise (1-2 lines). Indent continuation lines to align with the description start. If the release at the top has a concrete date with no ? in it, start a new release above it.

### Verifying Changes
After making code changes (especially during code reviews), always do a build to make sure nothing is broken. On macOS, use:
```bash
xcodebuild -configuration Debug   # Fastest iterative build (Debug, single arch, uses PCH)
xcodebuild                        # Normal iterative build (Release, universal, uses PCH)
xcodebuild GCC_PREPROCESSOR_DEFINITIONS='$(inherited) NO_PCH '   # Final verification only
```
Adding `-configuration Debug` builds only for the native architecture in Debug mode, which is much quicker for iterative build testing during development. The default Release build is universal (arm64 + x86_64) and optimized, so it takes longer. The `NO_PCH` flag disables pre-compiled headers, making the macOS build closer to Linux/Windows behavior and helping catch missing `#include` directives that the PCH would otherwise mask. However, disabling PCH significantly slows the build, so use it only as a final verification before committing — not for iterative development.

## Architecture

### Core Application (`xLights/`)
- **Entry point**: `xLightsApp.cpp` (wxApp subclass) → `xLightsMain.cpp` (`xLightsFrame`, the main window)
- **Sequencer**: `xLights/sequencer/` — timeline UI, effect grid, effect layers, undo manager. `MainSequencer` is the primary sequencer widget; `tabSequencer.cpp` handles sequencer-related event handlers on xLightsFrame.
- **Effects**: `xLights/effects/` — 55 effects, each as a pair: `FooEffect.cpp` (rendering logic, `adjustSettings` for migration) + `FooPanel.cpp` (UI controls). All inherit from `RenderableEffect`. See [Effects Reference](#effects-reference) below.
- **Models**: `xLights/models/` — 20+ model types plus 9 DMX models. All inherit from `Model`. See [Models Reference](#models-reference) below.
- **Outputs**: `xLights/outputs/` — protocol handlers and controller connection config. See [Outputs & Controllers Reference](#outputs--controllers-reference) below.
- **Controllers**: `xLights/controllers/` — vendor-specific hardware upload handlers (Falcon, FPP, WLED, etc.). See [Outputs & Controllers Reference](#outputs--controllers-reference) below.
- **Discovery core**: `xLights/discovery/` — shared controller/output discovery data structures and discovery API used by core layers.
- **XmlSerializer**: `xLights/XmlSerializer/` — XML serialization/deserialization for models, objects, and rgb effects. Includes GDTF parser. Core layer — should not depend on wx or UI.
- **SequenceMedia** (`sequencer/SequenceMedia.cpp`): Manages image caching and embedding for sequences. Images can be embedded in .xsq files or referenced externally. Uses `FixFile()` to resolve relative paths.

### Core vs UI Layer Architecture

The codebase is being refactored to separate wx-free core logic from wxWidgets UI code. The goal is a platform-neutral core usable without wx (e.g., for an iPad app).

**Core packages** (enforced by `ci_scripts/check_core_include_boundaries.sh`): `discovery/`, `graphics/`, `render/`, `effects/`, `models/`, `outputs/`, `controllers/`, `utils/`, `XmlSerializer/`. These directories **must not** include `ui/` headers, `xLightsMain.h`, or `xLightsApp.h` in their public headers or implementation files. New violations are blocked in strict mode; approved exceptions live in `ci_scripts/include_policy_allowlist.txt`.

**`xLights/graphics/`** — wx-free core graphics abstraction layer:
- `xlGraphicsContext.h` — abstract GPU context interface (no wx types; no window pointer; use `setContextualValue`/`getContextualValue` for passing context like `IModelPreview*`)
- `xlGraphicsAccumulators.h` — geometry/vertex accumulator interfaces
- `IModelPreview.h` — wx-free pure-virtual interface for model preview (used by models/ and effects/)
- `xlFontInfo.h` — font metadata using `xlImage` (no wxImage in public API)
- `xlImage.h` — wx-free RGBA pixel class (see also `utils/xlImage.h`)
- `xlMesh.h/.cpp` — 3D mesh loading (std::filesystem, no wx)

**`xLights/ui/graphics/`** — wx-dependent canvas implementations (NOT core):
- `xlGraphicsBase.h` — selects Metal vs OpenGL canvas at compile time; defines `GRAPHICS_BASE_CLASS`
- `opengl/xlGLCanvas.h/.cpp` — OpenGL canvas (wxGLCanvas subclass)
- `opengl/xlOGL3GraphicsContext.h/.cpp` — OpenGL graphics context implementation
- `metal/xlMetalCanvas.h/.mm` — Metal canvas (macOS)
- `metal/xlMetalGraphicsContext.h/.mm` — Metal graphics context implementation

**Include conventions:**
- Core files (`graphics/`, `models/`, etc.) include `graphics/IModelPreview.h` — never `ui/layout/ModelPreview.h`
- Canvas/UI files include `ui/graphics/xlGraphicsBase.h` — never `graphics/xlGraphicsBase.h` (that forwarder was removed)
- wx↔core conversion helpers (e.g., `wxImageToXlImage()`, `wxImagesToXlImages()`) live in `ui/wxUtilities.h`

### Key Patterns
- **Settings/SettingsMap**: Effects store settings as string key-value pairs (e.g., `E_TEXTCTRL_Pictures_Filename`). Prefix conventions: `E_` for effect settings, `T_` for transitions, `B_` for buffer settings, `C_` for color.
- **`adjustSettings()`**: Called on each effect when loading sequences to migrate old settings to current format. NOT called for newly created effects (e.g., drag-and-drop).
- **`FixFile()`**: Resolves file paths by searching show directory, media directories, and common locations. Used throughout for portable path resolution.
- **`MakeRelativePath()`/`IsInShowOrMediaFolder()`**: Convert absolute paths to relative (for portability) and check file location. Relative paths are relative to the show directory or a media directory.
- **Show directory vs Media directories**: The show directory is the primary project folder. Additional media directories can be configured. Both are searched when resolving relative paths.
- **`ObtainAccessToURL(path, enforceWritable)`**: Must be called before reading/writing files on macOS to handle App Sandbox security-scoped bookmarks. Defined in `macOS/macOS-src/osxUtils/ExternalHooksMacOS.h`, implemented in Swift (`xlMacUtils.swift`). Returns `bool` — `true` if access granted. Pass `enforceWritable=true` when writing. Bookmarks are persisted in UserDefaults so access survives app restarts. No explicit release call needed. On non-macOS platforms this is a no-op. Call `ObtainAccessToURL` whenever a path comes from user input (file dialogs, drag-and-drop, text fields, etc.) so that persistent bookmarks are created/updated for that path. This ensures future access to the file/directory works even after app restart.

### Data Formats
- `.xsq` — Sequence files (XML-based, can contain embedded images as base64)
- `.fseq` — Binary playback format for Falcon Player
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

## Prefer std::* Over wx* Types

Core data types and algorithms should use standard C++ equivalents rather than wxWidgets types, to keep non-UI code portable and wx-free:

- **Strings**: Use `std::string` instead of `wxString`. Convert at wx API boundaries with `.ToStdString()` / `wxString(str)`.
- **Collections**: Use `std::vector`, `std::map`, `std::unordered_map`, etc. instead of `wxArrayString`, `wxList`, etc.
- **Colors**: Use `xlColor` (defined in `xLights/Color.h`) instead of `wxColour`. `xlColor` is now wx-free.
- **wx↔std conversions**: Place any `wxColour`/`wxString`/wx-collection ↔ std conversion helpers in `xLights/ui/wxUtilities.h` (and `.cpp` for non-inline implementations). Currently provides:
  - `xlColorToWxColour(const xlColor&) → wxColour`
  - `wxColourToXlColor(const wxColour&) → xlColor`
- **When wx constants suffice**: If a wx API accepts a wx constant directly (e.g. `*wxBLACK`, `*wxWHITE` for `SetTextForeground`), use the wx constant rather than converting an `xl*` constant through `xlColorToWxColour`.
- **Exceptions**: xLights has nearly non-existent exception handling — do NOT use `std::stoi`, `std::stol`, `std::stod`, etc. as they throw on invalid input. Use `std::strtol`, `std::strtod` (and friends) instead. These return 0/default on bad input without throwing.
- **File existence checks**: Use `FileExists()` from `ExternalHooks.h` instead of `std::filesystem::exists()` or `wxFile::Exists()` directly. On macOS, `FileExists()` triggers iCloud downloads for files that have been evicted to the cloud, which `std::filesystem::exists()` does not. For directory existence, use `std::filesystem::exists()` with the `std::error_code` overload (to avoid exceptions).

## Key Dependencies

wxWidgets 3.3 (custom fork `xLightsSequencer/wxWidgets`), spdlog, FFmpeg, SDL2, Lua 5.4, libcurl, zstd, LiquidFun/Box2D, libxlsxwriter, nanosvg, ISPC (SIMD kernels).

