# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

xLights is a cross-platform (Windows/macOS/Linux) C++20 show sequencer for controlling lighting via USB/DMX/sACN/ArtNet/DDP. Built on wxWidgets 3.3 (custom fork). The main application is xLights; companion apps include xSchedule, xCapture, xFade, xScanner.

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

### Adding New Source Files
When adding new `.cpp`/`.h` files, the following project files must be updated manually:
- **`xLights/xLights.cbp`** — add `<Unit filename="...">` entries (used by Linux build via cbp2make)
- **`xLights/Xlights.vcxproj`** — add `<ClCompile>` for `.cpp` and `<ClInclude>` for `.h`
- **`xLights/Xlights.vcxproj.filters`** — add corresponding filter entries to place files in the correct VS folder

The macOS Xcode project (`macOS/xLights.xcodeproj/project.pbxproj`) may also need updating — some directories use `PBXFileSystemSynchronizedRootGroup` (auto-discovered), others require manual `PBXFileReference`/`PBXBuildFile` entries. Check on a case-by-case basis.

## Architecture

### Core Application (`xLights/`)
- **Entry point**: `xLightsApp.cpp` (wxApp subclass) → `xLightsMain.cpp` (`xLightsFrame`, the main window)
- **Sequencer**: `xLights/sequencer/` — timeline UI, effect grid, effect layers, undo manager. `MainSequencer` is the primary sequencer widget; `tabSequencer.cpp` handles sequencer-related event handlers on xLightsFrame.
- **Effects**: `xLights/effects/` — 55 effects, each as a pair: `FooEffect.cpp` (rendering logic, `adjustSettings` for migration) + `FooPanel.cpp` (UI controls). All inherit from `RenderableEffect`. See [Effects Reference](#effects-reference) below.
- **Models**: `xLights/models/` — 20+ model types plus 9 DMX models. All inherit from `Model`. See [Models Reference](#models-reference) below.
- **Outputs**: `xLights/outputs/` — protocol handlers and controller connection config. See [Outputs & Controllers Reference](#outputs--controllers-reference) below.
- **Controllers**: `xLights/controllers/` — vendor-specific hardware upload handlers (Falcon, FPP, WLED, etc.). See [Outputs & Controllers Reference](#outputs--controllers-reference) below.
- **SequenceMedia** (`sequencer/SequenceMedia.cpp`): Manages image caching and embedding for sequences. Images can be embedded in .xsq files or referenced externally. Uses `FixFile()` to resolve relative paths.

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
Each in its own directory with its own .cbp project: `xSchedule/`, `xCapture/`, `xFade/`, `xScanner/`, `xlDo/`.

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

wxWidgets 3.3 (custom fork `xLightsSequencer/wxWidgets`), log4cpp, FFmpeg, SDL2, Lua 5.4, libcurl, zstd, LiquidFun/Box2D, libxlsxwriter, nanosvg, ISPC (SIMD kernels).

