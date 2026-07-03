# AGENTS.md — xLights Developer Guide

Single source of truth for AI agents working on the xLights codebase
(replaces `CLAUDE.md`). Authoritative — these instructions override default
agent behavior.

---

## 1. Project Overview

xLights is a cross-platform (Windows/macOS/Linux) C++20 show sequencer for
controlling lighting via USB/DMX/sACN/ArtNet/DDP. Built on wxWidgets 3.3
(custom fork: `xLightsSequencer/wxWidgets`).

**Minimum supported platforms:** macOS 10.15 (Catalina), Linux (Debian 12 /
Ubuntu 24.04), Windows 8.

### Repository layout

| Directory | Purpose |
|---|---|
| `src-core/` | wx-free C++ core — linked by **both** desktop and iPad |
| `src-ui-wx/` | wxWidgets desktop UI (Windows/macOS/Linux) |
| `src-iPad/` | SwiftUI iPad app + ObjC++ bridge layers |
| `macOS/` | Xcode project at `macOS/xLights.xcodeproj` |
| `xLights/` | Build system files (.cbp, .vcxproj, etc.) |
| `plans/ipad-parity/` | Per-theme parity scorecards (see §3) |
| `ci_scripts/` | Include-boundary checks, CI tooling |
| `xlDo/` | Remaining sub-app in this repo |
| `common/` | Shared code (ARC-enabled `.mm` files) |

### The iPad app

This repo also contains the **iPad app** (SwiftUI, iOS 26+). It is **not a
port** — it's a second UI that shares the same wx-free C++ core (`src-core/`),
render engine, effect manager, sequence file/elements, and audio manager as the
desktop. The iPad app has **shipped to the App Store** and is now in
desktop-parity / ongoing-update mode (beyond MVP). Status and per-theme parity
plans live in [`iPad-xLights-Plan.md`](iPad-xLights-Plan.md) and
[`plans/ipad-parity/`](plans/ipad-parity/) (see §3).

### Companion apps

xSchedule, xCapture, xFade, xScanner → moved to their own repos under
`xLightsSequencer` on GitHub. Only `xlDo/` remains here.

---

## 2. Architecture Layers

### Core (`src-core/`) — wx-free, shared by both apps

| Package | Description |
|---|---|
| `effects/` | 55 effects: `FooEffect.cpp` (render logic, `adjustSettings` migration) + `FooPanel.cpp` (UI). All inherit `RenderableEffect`. |
| `models/` | 20+ model types + 9 DMX models. All inherit `Model`. |
| `outputs/` | Protocol handlers + controller connection config (sACN, ArtNet, DDP, USB, etc.) |
| `controllers/` | Vendor upload handlers (Falcon, FPP, WLED, etc.) |
| `media/` | Audio decode/encode/playback. `AudioManager` with abstract `IAudioDecoder`/`IAudioOutput`. Apple: AudioToolbox (decode) + AVAudioEngine (playback). Linux/Windows: FFmpeg + SDL2. Also `xLightsVamp` (VAMP analysis), `AudioLoader` (FFmpeg loader). |
| `render/` | Rendering engine. `SequenceMedia.cpp` manages image cache/embed (`.xsq` base64 or external refs), resolves paths via `FixFile()`. |
| `graphics/` | wx-free GPU abstraction (see below). |
| `discovery/` | Controller/output discovery data structures + API. |
| `XmlSerializer/` | XML (de)serialization for models, objects, RGB effects. Includes GDTF parser. Must not depend on wx/UI. |
| `lyrics/`, `utils/` | `utils/Color.h` (`xlColor`), `utils/FloatChecks.h` (`xl::isnan`/`isinf`/`isfinite`), etc. |

Entry points are in the desktop UI layer: `src-ui-wx/xLightsApp.cpp` (wxApp) →
`src-ui-wx/xLightsMain.cpp` (`xLightsFrame`, the main window).

**Core rule — strictly enforced:** Nothing in `src-core/` may include anything
from `src-ui-wx/`, `src-ui*/`, or any `wx/…` header — public headers and `.cpp`
alike. The goal: `src-core/` compiles and links without wxWidgets so the iPad
app (and any future non-wx host) reuses it unchanged. Enforced by
`ci_scripts/check_core_include_boundaries.sh` over these packages: `discovery/`,
`graphics/`, `lyrics/`, `media/`, `render/`, `effects/`, `models/`, `outputs/`,
`controllers/`, `utils/`, `XmlSerializer/`. New violations are blocked in strict
mode; approved exceptions live in `ci_scripts/include_policy_allowlist.txt`.

**`src-core/graphics/` — wx-free graphics abstraction:**

| Header | Role |
|---|---|
| `xlGraphicsContext.h` | Abstract GPU context — no wx types, no window pointer; pass context (e.g. `IModelPreview*`) via `setContextualValue`/`getContextualValue` |
| `xlGraphicsAccumulators.h` | Geometry/vertex accumulator interfaces |
| `IModelPreview.h` | wx-free pure-virtual model-preview interface (used by `models/`, `effects/`) |
| `xlFontInfo.h` | Font metadata using `xlImage` (no wxImage in public API) |
| `xlImage.h` | wx-free RGBA pixel class (see also `utils/xlImage.h`) |
| `xlMesh.h/.cpp` | 3D mesh loading (std::filesystem, no wx) |

### Desktop UI (`src-ui-wx/`) — wxWidgets

| Component | Description |
|---|---|
| `xLightsApp.cpp` | wxApp entry point |
| `xLightsMain.cpp` | `xLightsFrame` — the main window |
| `sequencer/` | Timeline UI: `MainSequencer` widget, effect grid, layers, undo manager |
| `tabSequencer.cpp` | Sequencer event handlers on `xLightsFrame` |
| `graphics/` | wx-dependent canvas impls — **NOT core**: OpenGL (`opengl/xlGLCanvas`, `xlOGL3GraphicsContext`), Metal (`metal/xlMetalCanvas.mm`, `xlMetalGraphicsContext.mm`). `xlGraphicsBase.h` selects Metal vs OpenGL at compile time and defines `GRAPHICS_BASE_CLASS`. |
| `shared/utils/wxUtilities.h` | wx↔core conversion helpers (`wxImageToXlImage`, `wxImagesToXlImages`, `xlColorToWxColour`, `wxColourToXlColor`) |

### iPad UI (`src-iPad/`) — SwiftUI + ObjC++

| Directory | Purpose |
|---|---|
| `App/` | SwiftUI views + `@Observable SequencerViewModel` (SequencerView, SequencerGridV2View, EffectsMetalGridView, RowHeaderViews, …) |
| `Bridge/` | ObjC++ bridges: `XLSequenceDocument`, `iPadRenderContext`, `XLiPadInit`, `CoreGraphicsTextDrawingContext`, `XLValueCurve` |
| `Metal/` | `xlStandaloneMetalCanvas`, `iPadModelPreview`, `XLMetalBridge`, `iPadGridPreview`, `XLGridMetalBridge` |
| `Metadata/` | `EffectMetadata.swift` (JSON model for `effectmetadata/*.json`) |

Key iPad patterns:
- **`XLSequenceDocument`** (`Bridge/`): ObjC++ bridge callable from Swift. Use
  `NS_SWIFT_NAME(…)` to control the imported Swift name. When the desktop adds a
  new mutating op on `Effect`/`EffectLayer`/`Element`/`SequenceElements`, this
  bridge usually needs a matching wrapper.
- **`iPadRenderContext`** (`Bridge/`): subclasses `RenderContext` from
  `src-core/render/`, mirroring `xLightsFrame`'s role on desktop.
- **`SequencerViewModel`** (`App/`): single `@Observable` class SwiftUI reads.
  All mutating ops go through it so undo registration + row reloads happen in
  one place.
- **Metal grid**: `EffectsMetalGridView` / `TimingEffectsMetalGridView` /
  `TopChromeMetalGridView` render via `XLGridMetalBridge` →
  `xlStandaloneMetalCanvas` → `xlMetalGraphicsContext`. The iPad grid is
  Metal-only; desktop uses `xlGraphicsBase`'s OpenGL/Metal switch.

### Include conventions

| Situation | Right include | Wrong include |
|---|---|---|
| Core file needs model preview | `graphics/IModelPreview.h` | `layout/ModelPreview.h` (wx subclass) |
| Canvas/UI file needs graphics base | `graphics/xlGraphicsBase.h` (resolves to `src-ui-wx/graphics/…` — `src-core/graphics/` has no such file) | — |
| wx↔core conversion needed | `shared/utils/wxUtilities.h` | Manual wx→core conversion inline |

---

## 3. Desktop ↔ iPad Parity (Mandatory)

**Every desktop change must be accounted for on the iPad side.** This is the
single most important rule in this project. A change to `src-ui-wx/` or
`src-core/` is **not done** until one of two states is reached:

### State A — Reflected immediately

When the iPad has (or easily can have) an equivalent surface:

1. Apply the matching change to `src-iPad/` in the **same PR**.
2. Build `xLights-iPadLib` to verify (see §4).
3. Update the relevant `plans/ipad-parity/` scorecard row to ✅ / 🟡.

### State B — Captured in parity plans

When immediate porting isn't straightforward (large SwiftUI/Metal work, bridge
extraction needed, genuinely infeasible/restricted):

1. Add/update a scorecard row in the matching `plans/ipad-parity/` theme doc
   with `file:line` evidence for **both** platforms.
2. Pick the gap label: `ipad-missing` / `ipad-weaker` / `parity` /
   `desktop-missing`.
3. If adding a sizable new gap, bump `00-overview.md` headline counts + roadmap.
4. Infeasible/restricted items go in that doc's *Infeasible / restricted*
   section with the reason.

**Never** leave a desktop change unaccounted-for — "I'll track it later" loses
the change. Core-only (`src-core/`) changes that need no iPad UI work still
warrant a one-line note on the relevant scorecard row that the behavior is
shared/auto-applied. The decision of which state applies, and the supporting
analysis, belong in the PR description.

### Concrete guidance by layer

| Change touches | iPad action |
|---|---|
| `src-core/` | Auto-applied (both link same core). Review **both** code paths for edge cases. |
| `src-ui-wx/` UI feature with iPad counterpart | Update matching `src-iPad/App/` view in same PR (e.g. a new row-heading right-click entry → `ModelRowHeader.contextMenu`) |
| `src-ui-wx/` adds mutating op on Effect/EffectLayer/Element/SequenceElements | Add matching wrapper in `src-iPad/Bridge/` (`XLSequenceDocument`) |
| iPad-only feature (two-finger marquee, long-press menus, trackpad scroll) | Fine — touch idioms without desktop equivalents |
| Desktop-only by platform limit (FFmpeg-only audio filters, raw serial/DMX, proprietary-firmware uploads) | Record in plans' *Infeasible / restricted* section with the reason |

When unsure, err toward matching — cheaper to add a parallel UI entry than to
ship a behavior gap users discover when switching clients.

### Parity plan docs

Start at [`plans/ipad-parity/README.md`](plans/ipad-parity/README.md). The
overview ([`00-overview.md`](plans/ipad-parity/00-overview.md)) holds the live
headline numbers, the P1/P2 roadmap, and reverse-parity candidates — **read it
there rather than trusting any count copied here, which will rot.** As of the
last full audit the iPad sat at **~72% parity**, with the biggest backlogs in
Preferences (11), Layout (06), and Tools (13).

The 14 theme docs:

| # | Theme |
|---|---|
| `00-overview` | Headline numbers, P1/P2 roadmap, reverse-parity candidates |
| `01-file-lifecycle` | File lifecycle & sequence management |
| `02-sequencer-grid-editing` | Sequencer grid & effect editing |
| `03-timing-audio` | Timing tracks & audio |
| `04-effects-and-panels` | Effects & effect setting panels |
| `05-color-and-value-curves` | Color panel, palettes & value/color curves |
| `06-layout-models-preview` | Layout: models, groups, preview, 3D, submodels/DMX |
| `07-setup-controllers-upload` | Setup, controllers, outputs & upload |
| `08-import-export` | Import & export |
| `09-render-playback` | Render & playback |
| `10-presets-jukebox-views-perspectives` | Presets, jukebox, display elements, views & perspectives |
| `11-preferences-settings` | Preferences, settings & keyboard shortcuts |
| `12-ai-automation-scripting` | AI, automation & scripting |
| `13-tools-diagnostics-help` | Tools, diagnostics & help |
| `14-reverse-parity-ipad-only` | Reverse-parity — iPad-only features |

---

## 4. Build Commands

### macOS — Desktop

```bash
# Fast iterative (Debug, single native arch, uses PCH)
xcodebuild -configuration Debug

# Normal release build (Release, universal arm64+x86_64, optimized, uses PCH)
xcodebuild

# Final verification only (no PCH — catches missing #include the PCH masks)
xcodebuild GCC_PREPROCESSOR_DEFINITIONS='$(inherited) NO_PCH '
```

Requires Xcode 26.3+. Dependencies auto-download with a matching Xcode version.
Libraries install to `/opt/local/lib` (release) / `/opt/local/libdbg` (debug).
`NO_PCH` significantly slows the build — use it only for final verification, not
iterative work.

### macOS — iPad library

```bash
xcodebuild -project macOS/xLights.xcodeproj \
           -scheme xLights-iPadLib \
           -configuration Debug \
           -destination 'generic/platform=iOS' build
```

`xLights-iPadLib` is the static library the `xLights-iPad` app target links
against. Prefer this scheme for fast iterative checks — it skips the app-bundle
wrap and catches the vast majority of errors. Use the `xLights-iPad` scheme only
when deploying to device/simulator.

**Build the iPad library when** your change touches `src-core/` or `src-iPad/` —
the desktop build won't surface iOS-specific breaks (e.g. `#ifdef __APPLE__`
paths, ObjC++ bridge compilation, Swift interop). iPad deps live at
`/opt/xLights-macOS-dependencies/lib-ios/`.

### Linux

```bash
make                     # Full build (wxWidgets + deps + all apps)
make debug               # Debug build of all apps
make clean               # Clean all
make xLights             # Build just xLights (release)
```

Output binaries → `bin/`. Build uses Code::Blocks `.cbp` → makefiles via
`cbp2make`. Objects → `.objs_debug/` or `.objs_release/`.

### Windows

```bash
msbuild -restore -m:10 xLights.sln -p:Configuration="Release" -p:Platform="x64"
```

Open in Visual Studio (`.vcxproj`) or Code::Blocks.

### CMake (cross-platform — alternative)

```bash
cmake -S . -B build      # Configure
cmake --build build      # Build
```

Top-level `CMakeLists.txt`, primarily used on Windows as an alternative to the
`.sln`/`.vcxproj` files (out-of-source build dir). Source files are discovered
via `file(GLOB_RECURSE ...)` — see §5 for when a manual CMakeLists.txt edit is
needed.

### wxSmith generated code

Some dialogs/panels use the wxSmith RAD tool. Generated code is guarded by
`//(* ... //*)` markers in `.cpp`/`.h` files. **Changes within these guards MUST
also be reflected in the corresponding `.wxs` file** in `src-ui-wx/wxsmith/` —
otherwise they're overwritten next time the `.wxs` is opened in wxSmith. New
controls, event handlers, or modifications inside the guards require matching
`.wxs` XML edits.

### Verifying changes

After making code changes (especially during code reviews), always build to make
sure nothing is broken. When a change touches `src-core/` or `src-iPad/`, also
build the iPad library (above). See the [build checklist](#build-checklist-for-changes)
in §11.

---

## 5. Adding New Source Files

Place files in one of:
- `src-core/` — wx-free core (linked by both apps)
- `src-ui-wx/` — wxWidgets desktop UI
- `src-iPad/` — SwiftUI iPad app + ObjC++ bridge

### Project files that need manual updates

| File | What to add |
|---|---|
| `xLights/xLights.cbp` | `<Unit filename="...">` with path relative to `xLights/` (e.g. `../src-core/render/Foo.cpp`) |
| `xLights/Xlights.vcxproj` | `<ClCompile>` for `.cpp`, `<ClInclude>` for `.h`, path relative to `xLights/` (e.g. `..\src-core\render\Foo.cpp`) |
| `xLights/Xlights.vcxproj.filters` | Filter entries for VS folder organization |

### Auto-discovered (no manual edit needed)

| Platform | Mechanism |
|---|---|
| macOS (Xcode) | `PBXFileSystemSynchronizedRootGroup` for `src-core/`, `src-ui-wx/`, `src-iPad/` — files auto-discovered, no pbxproj edit |
| CMake build | `file(GLOB_RECURSE SRC_UI ...)` / `file(GLOB_RECURSE SRC_CORE ...)` over existing dirs. New files inside the glob patterns are picked up on the next configure. Only files placed **outside** the existing glob patterns need a new `file(GLOB ...)` or `list(APPEND ...)` line. |

Windows/Linux builds intentionally don't compile `src-iPad/` — new iPad files
never need `.cbp`/`.vcxproj` entries.

### ISPC kernels (`src-core/effects/ispc/*.ispc`) — extra steps

A new SIMD kernel `FooFunctions.ispc` is **not** auto-discovered anywhere and is
**gitignored** (`.gitignore`'s bare `ispc` pattern matches the whole `ispc/`
dir). Every one of these is required:

| File | What to add |
|---|---|
| the `.ispc` + generated `.ispc.h` | `git add -f` both (gitignored). Commit the `.ispc.h` (checked in; build regenerates + overwrites only on change). |
| `macOS/xLights.xcodeproj/project.pbxproj` | Add `effects/ispc/FooFunctions.ispc,` to the `PBXFileSystemSynchronizedGroupBuildPhaseMembershipExceptionSet` for the **`ISPCEffectComputeFunctions`** target. Not auto-discovered; missing it fails the x86_64 link (arm64 / static-lib iPad hide it). **`macOS/` is a git submodule** — commit there + bump the pointer. |
| `build_scripts/linux/ispc.mak` | Add the `OBJ_LINUX_DEBUG +=` / `OBJ_LINUX_RELEASE += …/FooFunctions.o` pair. |
| `xLights/xLights.cbp` | `<Unit>` for the `.ispc` (with `<Option link="1"/>`) and the `.ispc.h`. |
| `xLights/Xlights.vcxproj` (+ `.filters`) | `<CustomBuild>` for the `.ispc` (copy an existing kernel's ispc.exe block) and `<ClInclude>` for the `.ispc.h`. |
| CMake | Auto-globs `*.ispc` — no edit. |

Generate the committed header with the same flags the build's header step uses:
`ispc --target-os=macos --target=avx2-i32x16 --target=avx1-i32x16 --arch=x86_64
-h Foo.ispc.h Foo.ispc`, then `sed -i '' '/.ispc.h/d' Foo.ispc.h`.

### Metal kernels (`src-core/effects/metal/*.metal` + `Metal*Effect.mm`) — Apple only

A GPU effect is a `.metal` shader + a `Metal<Foo>Effect.mm` wrapper that **subclasses**
the CPU effect and overrides `Render` (fall back to the base `Render` when Metal
isn't viable — no GPU, buffer < `metalBufferSizeThreshold`, or unsupported options).
Neither file is gitignored (plain `git add`). Metal is Apple-only — **no** `.cbp`,
`.vcxproj`, or `ispc.mak` edits.

| Piece | What to do |
|---|---|
| `Foo.metal` | Compute kernel `kernel void FooEffect(constant MetalFooData&, device uchar4*, uint index)`. **Auto-compiled** into `EffectComputeFunctions.metallib` by the `EffectComputeFunctions` target (it syncs all of `src-core`, no per-file list). |
| `MetalEffectDataTypes.h` | Add `MetalFooData` struct (shared by `.mm` and `.metal`). |
| `MetalEffects.hpp` | Declare `class MetalFooEffect : public FooEffect` + `class MetalFooEffectData;`. |
| `MetalFooEffect.mm` | Wrapper: `data->fn = FindComputeFunction("FooEffect")`, fill the struct, dispatch one thread/pixel. Auto-discovered by `xLights-core`. |
| `MetalEffectManager.mm` | Add `case eff_FOO: return new MetalFooEffect(eff);` (the `#ifdef __APPLE__ CreateMetalEffect` factory). |
| `macOS/.../project.pbxproj` | Add `effects/metal/FooFunctions.metal,` to the **`xLights-core`** target's membership-exception list (so xLights-core doesn't also compile it). Verify the symbol landed: `xcrun metal-nm .../EffectComputeFunctions.metallib \| grep FooEffect`. `macOS/` is a submodule. |

---

## 6. Code Conventions & Gotchas

### C++ style

- C++20 with GNU extensions (`-std=gnu++20`)
- 4-space indentation, no tabs (`.clang-format` configured)
- No column limit (`ColumnLimit: 0`)
- Opening braces on same line (K&R style)
- Match the style of nearby code (the codebase is not perfectly consistent)
- **Avoid purely cosmetic changes** in PRs

### Comments

Default to **no comments**. Only add one when the WHY is non-obvious: a hidden
constraint, a subtle invariant, a workaround for a specific bug, or behavior that
would surprise a reader. Don't explain what the code does (well-named identifiers
do that). Don't reference the current task or fix (that belongs in the commit
message).

### Prefer `std::*` over wx types in core

| wx type | Prefer | Notes |
|---|---|---|
| `wxString` | `std::string` | Convert at wx boundaries: `.ToStdString()` / `wxString(str)` |
| `wxArrayString`, `wxList` | `std::vector`, `std::map`, `std::unordered_map` | |
| `wxColour` | `xlColor` (`src-core/utils/Color.h`) | `xlColor` is wx-free |
| wx↔std conversions | `shared/utils/wxUtilities.h` | `xlColorToWxColour()`, `wxColourToXlColor()` |

When a wx API accepts a wx constant directly (e.g. `*wxBLACK`, `*wxWHITE` for
`SetTextForeground`), use the wx constant rather than converting an `xl*`
constant through `xlColorToWxColour`.

### Exception safety

xLights has nearly non-existent exception handling. **Do not** use
`std::stoi`/`std::stol`/`std::stod` — they throw on invalid input. Use
`std::strtol`, `std::strtod` (and friends), which return 0/default on bad input
without throwing.

### File existence checks

Use `FileExists()` from `ExternalHooks.h` instead of `std::filesystem::exists()`
or `wxFile::Exists()` directly. On macOS, `FileExists()` triggers iCloud
downloads for files evicted to the cloud (which `std::filesystem::exists()` does
not). For directory existence, use `std::filesystem::exists()` with the
`std::error_code` overload (to avoid exceptions).

### macOS sandbox: `ObtainAccessToURL`

Call `ObtainAccessToURL(path, enforceWritable)` before reading/writing files on
macOS — it handles App Sandbox security-scoped bookmarks. Defined in
`macOS/macOS-src/osxUtils/ExternalHooksMacOS.h`, implemented in Swift
(`xlMacUtils.swift`). Returns `bool` (`true` = access granted). Pass
`enforceWritable=true` when writing. Bookmarks persist in UserDefaults (survive
restarts); no explicit release call. On non-macOS it's a no-op. Call whenever a
path comes from user input (file dialogs, drag-and-drop, text fields) so
persistent bookmarks get created/updated.

### Objective-C++ ARC

**Every Xcode target uses ARC** (`CLANG_ENABLE_OBJC_ARC = YES`):
`xLights-Apple-core`, `xLights-core`, `xLights-macOSLib-UI`, `xLights-iPadLib`,
`xLights-iPad`, and the desktop `xLights` app. Every `.mm` file in
`src-apple-core/`, `src-core/`, `src-mac-ui/`, `src-ui-wx/`, `src-iPad/`, and
`common/` compiles under ARC.

**Do NOT write** `retain` / `release` / `autorelease` / `[obj release]; obj = nil;`
/ `[super dealloc]` in any `.mm` file — manual retain/release is a compile error.
An explicit `-dealloc` is allowed only to clean up non-ObjC state (e.g. `delete
_bgTexture` for a C++ object in an ObjC ivar); never call `[super dealloc]`.

ARC rules:
- ObjC pointers in C++ classes/structs default to `__strong` — retained on
  assignment, released on destruction (ARC understands C++ destructors). Just
  declare `id<MTLTexture> texture = nil;`. Same for `std::vector<NSFoo*>`,
  `std::map`, etc. (element types default to `__strong`).
- Pass ObjC pointers **by value**, not by `id<…>&` reference — reference params
  default to `__autoreleasing` under ARC, which doesn't bind to `__strong` ivars
  at the call site and risks name-mangling mismatches across ARC↔MRC.
- Use `[NSData dataWithBytes:…]`, `[NSString stringWithUTF8String:…]` freely.
- For blocks capturing `self` via an ivar (`_foo->bar()`), prefer explicit
  `self->_foo->bar()` to silence `-Wimplicit-retain-self` when intentional.
- Cross-language refcounting (CF↔ObjC): use `CFBridgingRetain` /
  `CFBridgingRelease` and `__bridge` / `__bridge_retained` / `__bridge_transfer`
  casts. Casts between `id` and `void*` require explicit `(__bridge void*)obj`.
- For `NS_RETURNS_RETAINED` blocks (like `MPSCopyAllocator`), annotate the block
  literal with `__attribute__((ns_returns_retained))` after the return type.
- Prefer `__unsafe_unretained` over `__weak` for the long-lived parent-reference
  pattern (documented lifetime guarantee, no zero-out overhead).

### wxWidgets ARC boundary gotchas

wxWidgets is an external MRC library; the ARC↔MRC boundary has two gotchas:

**+1 ownership transfer.** When you pass an NSView to `wxWidgetCocoaImpl(this,
view, …)`, wx takes +1 ownership (stores the pointer without an extra retain,
balances with `[release]` in its destructor). Under ARC the `__strong` local
holding the alloc/init result consumes that +1 and auto-releases at end of
scope, so wx's later destructor `release` over-releases (zombie crash on the
next autorelease-pool drain). **Fix:** call `CFBridgingRetain(view)` (discard the
`CFTypeRef` return) right after `alloc`/`init` to add an extra retain ARC won't
release. Used in `wxMetalCanvas::Create` for `wxCustomMTKView`. Apply to any new
site handing an NSView/NSObject to wx with MRC-style ownership.

**Reference-param mangling.** A function with an `id<…>&` or `NSFoo*&` parameter
shared between our ARC headers and wx MRC code mangles differently (ARC adds
`__autoreleasing` implicitly; the wx MRC TU doesn't) → link failure. **Fix:** pass
by value (preferred) or pin the qualifier with `__unsafe_unretained` (a no-op
under MRC).

### `-ffast-math` release builds

Release builds on macOS desktop **and iPad** use `-ffast-math` (`GCC_FAST_MATH =
YES` plus an explicit `-ffast-math` in `OTHER_CFLAGS` on the Release/Archive
configs), at `-O3` with `LLVM_LTO=YES_THIN`. `xLights-iPadLib` inherits these via
`$(inherited)`. This affects every `.cpp`/`.mm` in `src-core/`, `src-ui-wx/`,
`src-iPad/`. Linux/Windows release builds may not set it today, but write code
that doesn't depend on its absence.

`-ffast-math` implies `-ffinite-math-only` (optimizer assumes no `inf`/`NaN`).
Under `-O3` + LTO this silently breaks two source-correct patterns:

| Pattern | Problem | Fix |
|---|---|---|
| `float best = std::numeric_limits<float>::infinity()` as a max-so-far sentinel | `infinity()` folded to 0; first `if (v < best)` fails, value silently dropped | Use `std::numeric_limits<float>::max()` (or `::lowest()` for `-inf`). The legacy `1000000000.0f` idiom is also fine. Same for `HUGE_VALF`, `INFINITY`, `1.0f/0.0f`. |
| `std::isnan(x)` / `std::isinf(x)` / `std::isfinite(x)` as defensive guards | Folded to `false`/`false`/`true` — guard becomes a no-op | Use `xl::isnan` / `xl::isinf` / `xl::isfinite` from `src-core/utils/FloatChecks.h` (maps to `__builtin_*` on clang/gcc, preserved under `-ffinite-math-only`; `std::*` on MSVC). Do **not** call `__builtin_isnan` directly — MSVC lacks it and Windows fails to build. |

Don't write code depending on NaN propagation, `-0.0` sign preservation, or inf
arithmetic surviving — fast-math may reorder, fuse, or eliminate those. ISPC
files (`*.ispc`) have their own compile flags and are exempt. Vendored
third-party headers we've patched carry a `// xLights local patch:` marker —
preserve them across upstream merges.

---

## 7. Key Patterns & Idioms

| Pattern | Description |
|---|---|
| **Settings/SettingsMap** | Effects store settings as string key-value pairs (e.g. `E_TEXTCTRL_Pictures_Filename`). Prefixes: `E_` (effect), `T_` (transition), `B_` (buffer), `C_` (color). |
| **`adjustSettings()`** | Called on each effect when loading sequences — migrates old settings to the current format. **NOT** called for newly created effects (e.g. drag-and-drop). |
| **`FixFile()`** | Resolves file paths by searching show directory, media directories, and common locations. Used throughout for portable path resolution. |
| **`MakeRelativePath()` / `IsInShowOrMediaFolder()`** | Convert absolute paths to relative (portability); check file location. Relative paths are relative to the show directory or a media directory. |
| **Show directory vs Media directories** | Show directory = primary project folder. Additional media directories can be configured. Both are searched when resolving relative paths. |
| **`ObtainAccessToURL()`** | macOS sandbox access (see §6). Call before file I/O when a path comes from user input. |

---

## 8. Data Formats

| Extension | Description |
|---|---|
| `.xsq` | Sequence files — XML-based, can embed images as base64 |
| `.fseq` | Binary playback format for Falcon Player |
| `.xtiming` | Standalone timing-track export (`<timing>` or `<timings>` wrapper XML, read/written by both desktop and iPad) |
| `.pgo` | Papagayo lipsync file (desktop import only today) |
| Sequence settings | Stored as XML attributes with typed prefixes (see §7) |

---

## 9. Key Dependencies

| Dependency | Notes |
|---|---|
| wxWidgets 3.3 | Custom fork: `xLightsSequencer/wxWidgets` |
| spdlog | Logging (bundles fmt — include `<spdlog/fmt/fmt.h>`) |
| nlohmann/json | JSON parsing |
| FFmpeg | Media decode/encode (Linux/Windows) |
| SDL2 | Audio output (Linux/Windows) |
| Lua 5.4 | Scripting |
| libcurl | HTTP networking |
| zstd | Compression |
| LiquidFun / Box2D | Physics simulation |
| libxlsxwriter | Excel export |
| nanosvg | SVG rasterization |
| ISPC | SIMD kernels (`*.ispc` files, separate compile flags) |

---

## 10. Release Notes

`README.txt` contains ongoing release notes at the top of the file. When
implementing new features or fixing bugs, add a single very brief summary line
(no code) to the current release section:

```
    -enh (author)                Description of the enhancement
    -bug (author)                Description of the bug fix
    -change (author)             Description of the change
```

Keep descriptions brief (1-2 lines). Indent continuation lines to align with the
description start. If the release at the top has a concrete date with no `?` in
it, start a new release above it.

**Do NOT add iPad-specific changes to `README.txt`.** It is the **desktop**
release-notes file — iPad-only entries just clutter it. iPad changes go in the
parity plans (§3): update the matching feature's scorecard status (→ ✅ / 🟡).
Git history is the iPad changelog. Changes that touch shared `src-core/` code
**and** user-visible desktop behavior still belong in `README.txt`.

---

## 11. Quick Reference

### Build checklist for changes

| Change affects | Must build |
|---|---|
| `src-core/` only | macOS desktop (Debug) + iPad library |
| `src-ui-wx/` only | macOS desktop (Debug); Linux/Windows if relevant |
| `src-iPad/` only | iPad library (Debug) |
| `src-core/` + `src-ui-wx/` | macOS desktop (Debug) + iPad library |
| `src-core/` + `src-iPad/` | macOS desktop (Debug) + iPad library |
| Cross-platform (`.cbp`/`.vcxproj`/`CMakeLists.txt`) | macOS + Linux + Windows |

### Common pitfalls

- **Missing include in core** → build with `NO_PCH` to catch it
- **wx include leaked into core** → `check_core_include_boundaries.sh` blocks it
- **ARC manual retain/release** → compile error in `.mm` files
- **`infinity()` sentinel under `-ffast-math`** → silently 0, use `::max()`
- **`std::isnan` under `-ffast-math`** → folded to false, use `xl::isnan`
- **wxSmith guard edit** → also update the `.wxs` file
- **iPad parity gap** → must be reflected or captured before the PR is complete
- **macOS sandbox** → call `ObtainAccessToURL` for user-supplied paths
- **Exception-throwing parsers** → use `strtol`/`strtod`, not `stoi`/`stod`
- **`FileExists()`** → use it (not `std::filesystem::exists`) so iCloud files download
