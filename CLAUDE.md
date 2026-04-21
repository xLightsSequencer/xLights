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
```
msbuild -restore -m:10 xLights.sln -p:Configuration="Release" -p:Platform="x64"
```
Open in Visual Studio (vcxproj files) or Code::Blocks.

### wxSmith Generated Code
Some dialogs and panels use wxSmith (wxWidgets RAD tool). Generated code is delimited by `//(* ... //*)` guards in `.cpp`/`.h` files. **Any changes within these guards MUST also be reflected in the corresponding `.wxs` file** in `src-ui-wx/wxsmith/`. Otherwise the changes will be overwritten the next time the `.wxs` file is opened in wxSmith. If adding new controls, event handlers, or modifying existing ones inside the guards, update the `.wxs` XML to match.

### Adding New Source Files
When adding new `.cpp`/`.h` files, place them in `src-core/` (wx-free core) or `src-ui-wx/` (wxWidgets UI) as appropriate. The following project files must be updated manually:
- **`xLights/xLights.cbp`** — add `<Unit filename="...">` entries with paths relative to `xLights/` (e.g., `../src-core/render/Foo.cpp`)
- **`xLights/Xlights.vcxproj`** — add `<ClCompile>` for `.cpp` and `<ClInclude>` for `.h` with paths relative to `xLights/` (e.g., `..\src-core\render\Foo.cpp`)
- **`xLights/Xlights.vcxproj.filters`** — add corresponding filter entries to place files in the correct VS folder

The macOS Xcode project (`macOS/xLights.xcodeproj/project.pbxproj`) uses `PBXFileSystemSynchronizedRootGroup` for both `src-core/` and `src-ui-wx/` — files in those directories are auto-discovered. No manual pbxproj editing needed for new files in existing directories.

### Release Notes
`README.txt` contains ongoing release notes at the top of the file. When implementing new features or fixing bugs, add a single summary line with no code to the current release section:
- **Enhancements**: `    -enh (author)                Description of the enhancement`
- **Bug fixes**: `    -bug (author)                Description of the bug fix`
- **Changes**: `    -change (author)             Description of the change`

Keep descriptions concise (1-2 lines). Indent continuation lines to align with the description start. If the release at the top has a concrete date with no ? in it, start a new release above it.

**Do NOT add iPad-specific changes to `README.txt`.** The iPad app has not shipped yet and its changelog would only clutter the desktop release notes. Instead, keep `iPad-xLights-Plan.md` (next to `README.txt`) up to date: move items from "pending" to "done" as they land, and record any landed-fix details (root cause, follow-ups) inside the matching phase entry. Changes that touch shared `src-core/` code *and* user-visible desktop behavior still belong in `README.txt`; the iPad-only side goes in the plan.

### Verifying Changes
After making code changes (especially during code reviews), always do a build to make sure nothing is broken. On macOS, use:
```bash
xcodebuild -configuration Debug   # Fastest iterative build (Debug, single arch, uses PCH)
xcodebuild                        # Normal iterative build (Release, universal, uses PCH)
xcodebuild GCC_PREPROCESSOR_DEFINITIONS='$(inherited) NO_PCH '   # Final verification only
```
Adding `-configuration Debug` builds only for the native architecture in Debug mode, which is much quicker for iterative build testing during development. The default Release build is universal (arm64 + x86_64) and optimized, so it takes longer. The `NO_PCH` flag disables pre-compiled headers, making the macOS build closer to Linux/Windows behavior and helping catch missing `#include` directives that the PCH would otherwise mask. However, disabling PCH significantly slows the build, so use it only as a final verification before committing — not for iterative development.

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

**Core packages** (enforced by `ci_scripts/check_core_include_boundaries.sh`): `discovery/`, `graphics/`, `media/`, `render/`, `effects/`, `models/`, `outputs/`, `controllers/`, `utils/`, `XmlSerializer/`. These directories **must not** include `ui/` headers, `xLightsMain.h`, or `xLightsApp.h` in their public headers or implementation files. New violations are blocked in strict mode; approved exceptions live in `ci_scripts/include_policy_allowlist.txt`.

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
- **Colors**: Use `xlColor` (defined in `src-core/utils/Color.h`) instead of `wxColour`. `xlColor` is now wx-free.
- **wx↔std conversions**: Place any `wxColour`/`wxString`/wx-collection ↔ std conversion helpers in `src-ui-wx/shared/utils/wxUtilities.h` (and `.cpp` for non-inline implementations). Currently provides:
  - `xlColorToWxColour(const xlColor&) → wxColour`
  - `wxColourToXlColor(const wxColour&) → xlColor`
- **When wx constants suffice**: If a wx API accepts a wx constant directly (e.g. `*wxBLACK`, `*wxWHITE` for `SetTextForeground`), use the wx constant rather than converting an `xl*` constant through `xlColorToWxColour`.
- **Exceptions**: xLights has nearly non-existent exception handling — do NOT use `std::stoi`, `std::stol`, `std::stod`, etc. as they throw on invalid input. Use `std::strtol`, `std::strtod` (and friends) instead. These return 0/default on bad input without throwing.
- **File existence checks**: Use `FileExists()` from `ExternalHooks.h` instead of `std::filesystem::exists()` or `wxFile::Exists()` directly. On macOS, `FileExists()` triggers iCloud downloads for files that have been evicted to the cloud, which `std::filesystem::exists()` does not. For directory existence, use `std::filesystem::exists()` with the `std::error_code` overload (to avoid exceptions).

## PR Self-Review Checklist

This repo runs an automated Copilot code review on every PR. Over many rounds of review on this codebase, Copilot has repeatedly flagged the same 20 issue patterns — walking the diff against this checklist *before* pushing catches most of them. When you deliberately don't fix one of these, call that out in the PR body with a rationale so the maintainer can see the decision was intentional.

### Safety / correctness

1. **Return values** — does every `Delete`, `Compile`, `LoadFile`, `Write`, `Open`, etc. return a status? Check it or cast to `(void)` deliberately. `ModelManager::Delete(name)` returning `false` is especially important — silently proceeding leaves a stray renamed model behind.
2. **Unique temp names** — any `"__internal_prefix_" + counter` written to a shared map (e.g. `AllModels.AddModel(m)`) must loop until `map[name] == nullptr` before committing. `AddModel()` silently deletes a colliding entry — catastrophic data loss if the user happens to have a model with that name.
3. **Ownership transfer** — if a function takes a pointer and frees it internally (e.g. `Model::CreateDefaultModelFromSavedModelNode`), do NOT also `delete` it in the caller on the failure path. Read the function body, not just the signature.
4. **Static / cached state lifecycle** — any `static`, member cache, or dialog-level filter state must be reset in both the constructor and destructor so a reopened dialog starts clean instead of inheriting the previous session's state.
5. **Validate numeric input** — `std::strtod` / `strtol` accept `"nan"`, `"inf"`, and values outside a physically meaningful range. Always guard with `std::isfinite()` plus an explicit range check (use the same range the property grid uses if one exists — e.g. rotation is `[-180, 180]`).
6. **RAII over manual restore** — every `saved = X; override; … do stuff; restore(saved);` pattern should become a small struct with a destructor. Exception-safe, future-proof against new return paths. Applies to preferences, global flags, `OutputModelManager::DisableASAPWork`, etc.
7. **Thread safety on shared managers** — any manager-level `std::string` / `std::function` / container accessed from a `parallel_for` either needs a mutex or a `DisableXxx()` guard that brackets the parallel section. `OutputModelManager::AddASAPWork` is the known footgun — use `DisableASAPWork(true/false)` around bulk loads.

### Event & selection hygiene

8. **`ChangeValue` vs `SetValue` on wx text controls** — `SetValue("")` fires `wxEVT_TEXT`, which re-invokes your own text-changed handler and causes double work / flicker. Use `ChangeValue("")` when you want the widget updated without re-entering the event chain.
9. **Tree-widget native selection ≠ internal tracking** — `UnSelectAllModels()` clears per-`Model` `Selected()` flags and internal vectors but does NOT call `TreeListViewModels->UnselectAll()`. When the user expects a clean slate (e.g. before paste), clear BOTH. On a `wxTL_MULTIPLE` tree, a surviving native row means subsequent `Select(item)` calls ADD to the selection rather than replacing it.
10. **Never clear state before verifying success** — `UnSelectAllModels()` / cache-clear / preference-override calls must be INSIDE the success guard (`if (nd) {}` / `if (dlg.ShowModal() == wxID_OK) {}`). If the operation fails, the user's prior state should be untouched.
11. **`CreateUndoPoint("All", model, ...)` — the second arg must be a real model name**, not an operation label. It's stored as `undoBuffer[i].model` and passed to `AddASAPWork(..., selectedModel)` during `DoUndo`, where a non-existent model name breaks post-undo selection logic.

### UI polish

12. **Theme-aware colors** — no hard-coded RGB triples for "dimmed" / "disabled" / "placeholder" text. Blend between `__textForeground` (or the active theme's equivalent) and `__backgroundBrush` with `IsDarkMode()`-tuned weights so dark mode doesn't invert the visual meaning (a "pale grey" dim becomes MORE prominent on a dark background).
13. **Debounce heavy per-keystroke work** — `wxEVT_TEXT` handlers that call anything as expensive as `UpdateModelList(true)` (preview recompute + full tree rebuild) should wrap that call behind a 150 ms `wxTimer::StartOnce` so fast-typed bursts collapse into a single refresh. Have Enter / search-button / cancel explicitly `timer.Stop()` and refresh synchronously so they still feel immediate.
14. **Silent safety caps** — loop guards (`while (moved && loopGuard++ < 500)`, etc.) must `spdlog::warn(...)` when they trip, including the relevant state. A silent cap makes the failure case invisible to anyone debugging later.

### wxSmith / structural

15. **Anything inside `//(* ... //*)` guards must be mirrored in the corresponding `.wxs` file** — OR live outside the guard entirely. Handler declarations that are inside the guard but wired via `Bind()` (instead of wxSmith's generated event table) get dropped the next time someone opens the `.wxs` in wxSmith and regenerates.
16. **Match surrounding indentation style** — tabs if the file uses tabs, spaces if spaces. `wxSmith`-generated blocks in this codebase use tabs; hand-written new blocks pasted next to them should match.

### Scope discipline

17. **Widget draw functions often serve multiple panes** — e.g. `ModelCMObject::Draw` runs in BOTH the Controller Visualiser's left pane AND the "available models" right-pane list. Scope pane-specific behaviour to the right caller via an unambiguous signal (`portMargin > 0` for the visualizer pane, which the models list passes as 0). Don't rely on flags like `_dragging` that only exist in your head — verify they're actually set somewhere.
18. **Keep comments and code in sync** — "cheap `Contains()` without re-lowercasing each paint" in a comment while the code actually does `wxString(name).Lower()` per call is a review hit every time. Either cache what you claim to cache, or rewrite the comment to match what the code actually does.

### Dead-code detection

19. **Every guard variable you read must be written somewhere** — `!_dragging` where `_dragging` on `BaseCMObject` is a `bool` nothing ever writes is dead code that Copilot notices immediately. `grep` the setter; if there is none, delete the check (or wire up the real state).
20. **Bulk operations shouldn't inherit single-operation side effects** — the most common case is the "Save old name as alias?" prompt inside `Model::Rename`. A wholesale replace/duplicate flow that internally does `N × 3` renames will fire the prompt `N × 3` times. Save/set/restore the user's preference (wrapped in RAII guard — see #6) for the duration of the bulk op so the prompt fires zero times; the user's global setting stays untouched for subsequent rename operations.

### How to apply

Before `git commit` on a change, walk the diff against this list (or a shorter "likely-to-hit" subset based on what the change actually touches). For each item you deliberately don't fix, note it in the PR body with the reason, so the maintainer can see the decision was intentional rather than missed.

Also apply the list to your OWN follow-up commits that address Copilot feedback — Copilot re-reviews every push and often surfaces new findings on the fix itself.

## Key Dependencies

wxWidgets 3.3 (custom fork `xLightsSequencer/wxWidgets`), spdlog, nlohmann/json, FFmpeg, SDL2, Lua 5.4, libcurl, zstd, LiquidFun/Box2D, libxlsxwriter, nanosvg, ISPC (SIMD kernels).

