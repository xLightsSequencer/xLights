# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

xLights is a cross-platform (Windows/macOS/Linux) C++20 show sequencer for controlling lighting via USB/DMX/sACN/ArtNet/DDP. Built on wxWidgets 3.3 (custom fork). The main application is xLights; companion apps include xSchedule, xCapture, xFade, xScanner.

## Build Commands

### macOS
```bash
# Xcode project at macOS/xLights.xcodeproj
xcodebuild                    # CLI build
# Or open in Xcode. Dependencies auto-download if using matching Xcode version.
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
- **`ObtainAccessToURL(path, enforceWritable)`**: Must be called before reading/writing files on macOS to handle App Sandbox security-scoped bookmarks. Defined in `macOS/macOS-src/osxUtils/ExternalHooksMacOS.h`, implemented in Swift (`xlMacUtils.swift`). Returns `bool` — `true` if access granted. Pass `enforceWritable=true` when writing. Bookmarks are persisted in UserDefaults so access survives app restarts. No explicit release call needed. On non-macOS platforms this is a no-op.

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

## Key Dependencies

wxWidgets 3.3 (custom fork `xLightsSequencer/wxWidgets`), log4cpp, FFmpeg, SDL2, Lua 5.4, libcurl, zstd, LiquidFun/Box2D, libxlsxwriter, nanosvg, ISPC (SIMD kernels).

---

## Models Reference

### Model Inheritance Hierarchy

```
BaseObject
└── Model
    └── ModelWithScreenLocation<T>  [template, T = screen location type]
        ├── ArchesModel             [ThreePointScreenLocation]
        ├── CandyCaneModel          [ThreePointScreenLocation]
        ├── ChannelBlockModel       [TwoPointScreenLocation]
        ├── CircleModel             [BoxedScreenLocation]
        ├── CubeModel               [BoxedScreenLocation]
        ├── CustomModel             [BoxedScreenLocation]
        ├── DmxModel                [BoxedScreenLocation]  ← DMX base
        │   ├── DmxGeneral
        │   ├── DmxMovingHeadComm   ← moving head base
        │   │   ├── DmxMovingHead
        │   │   └── DmxMovingHeadAdv
        │   ├── DmxFloodlight
        │   ├── DmxServo
        │   ├── DmxServo3d
        │   └── DmxSkull
        ├── IciclesModel            [ThreePointScreenLocation]
        ├── ImageModel              [BoxedScreenLocation]
        ├── MatrixModel             [BoxedScreenLocation]
        │   ├── SphereModel
        │   └── TreeModel
        ├── ModelGroup              [BoxedScreenLocation]
        ├── MultiPointModel         [MultiPointScreenLocation]
        ├── PolyLineModel           [PolyPointScreenLocation]
        ├── SingleLineModel         [TwoPointScreenLocation]
        ├── SpinnerModel            [BoxedScreenLocation]
        ├── StarModel               [BoxedScreenLocation]
        ├── WindowFrameModel        [BoxedScreenLocation]
        └── WreathModel             [BoxedScreenLocation]
    └── SubModel  [delegates screen location to parent]
```

### Models Directory Structure

```
xLights/models/
├── Model.h/cpp                    # Base model class
├── BaseObject.h/cpp               # Abstract base for all objects
├── BaseObjectVisitor.h            # Visitor pattern interface
├── ModelManager.h/cpp             # Factory/registry (CreateModel, GetModel, etc.)
├── ObjectManager.h/cpp            # Base manager class
├── ModelGroup.h/cpp               # Groups multiple models
├── SubModel.h/cpp                 # Named subset of parent model nodes
├── Node.h/cpp                     # Individual pixel node (color + coordinates)
├── DisplayAsType.h/cpp            # Type enum + string conversion
│
├── [Screen location classes]
├── ModelScreenLocation.h/cpp      # Base location
├── BoxedScreenLocation.h/cpp      # Center + width/height (most common)
├── TwoPointScreenLocation.h/cpp   # Start/end points (line-based)
├── ThreePointScreenLocation.h/cpp # Triangle with angle/shear (curved)
├── MultiPointScreenLocation.h/cpp # Arbitrary points
├── PolyPointScreenLocation.h/cpp  # Polygon/polyline
├── TerrainScreenLocation.h/cpp    # Terrain-specific
│
├── [All concrete model .h/.cpp]   # ArchesModel, MatrixModel, etc.
│
├── DMX/                           # DMX model implementations
│   ├── DmxModel.h/cpp             # Base DMX class
│   ├── Dmx*.h/cpp                 # Concrete DMX models
│   ├── DmxColorAbility.h/cpp      # Ability: color mixing (RGB/CMY/Wheel)
│   ├── DmxDimmerAbility.h/cpp     # Ability: brightness
│   ├── DmxBeamAbility.h/cpp       # Ability: focus/zoom/iris
│   ├── DmxShutterAbility.h/cpp    # Ability: shutter/strobe
│   ├── DmxPresetAbility.h/cpp     # Ability: preset/macro
│   ├── Servo.h/cpp                # Servo motor control
│   ├── Mesh.h/cpp                 # 3D mesh rendering
│   └── MovingHeads/               # Advanced moving head config
│       ├── MhChannel.h/cpp
│       └── MhFeature.h/cpp
│
├── [View objects]
├── ViewObject.h/cpp               # Base view object
├── ViewObjectManager.h/cpp
├── GridlinesObject, TerrainObject, MeshObject, RulerObject, ImageObject
│
└── Shapes.h/cpp, PWMOutput.h, ControllerConnection.h  # Utilities
```

### Key Model Virtual Methods

When creating a new model, override these from `Model`:

```cpp
// REQUIRED - screen location (handled by ModelWithScreenLocation<T>)
virtual const ModelScreenLocation& GetModelScreenLocation() const = 0;
virtual ModelScreenLocation& GetModelScreenLocation() = 0;

// REQUIRED - initialization
virtual void InitModel();  // Set up nodes, buffer size, coordinates

// REQUIRED
virtual bool SupportsExportAsCustom() const = 0;
virtual bool SupportsWiringView() const = 0;

// Property grid UI
virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager);
virtual void UpdateTypeProperties(wxPropertyGridInterface* grid);
virtual int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event);

// Buffer configuration
virtual const std::vector<std::string>& GetBufferStyles() const;
virtual void GetBufferSize(const std::string& type, ..., int& BufferWi, int& BufferHi, ...) const;
virtual void InitRenderBufferNodes(const std::string& type, ..., std::vector<NodeBaseClassPtr>& Nodes, ...) const;

// Rendering
virtual void DisplayModelOnWindow(ModelPreview* preview, xlGraphicsContext* ctx, ...);

// Validation
virtual std::list<std::string> CheckModelSettings();
```

### Key Model Member Variables

```cpp
std::vector<NodeBaseClassPtr> Nodes;  // Pixel nodes
int BufferHt, BufferWi, BufferDp;     // Default buffer dimensions
long parm1, parm2, parm3;            // Model parameters (strings, nodes/string, strands)
std::string ModelStartChannel;        // Start channel
std::string StringType;               // "RGB Nodes", "3 Channel RGB", etc.
std::string rgbOrder;                 // "RGB", "BGR", etc.
std::vector<int32_t> stringStartChan; // Start channel per string
bool SingleNode, SingleChannel;       // String type flags
```

### Creating a New Model Type

1. **Add DisplayAsType** in `DisplayAsType.h` enum + `.cpp` string mappings
2. **Create FooModel.h/cpp** inheriting `ModelWithScreenLocation<BoxedScreenLocation>` (or appropriate location type)
3. **Override**: `InitModel()`, `SupportsExportAsCustom()`, `SupportsWiringView()`, `AddTypeProperties()`, `OnPropertyGridChange()`
4. **Register** in `ModelManager::CreateModel()` factory switch
5. **Update project files** (.cbp, .vcxproj, .vcxproj.filters; Xcode may auto-discover)

### DMX Model Abilities

DMX models use optional ability composition (via `unique_ptr`):
- **DmxColorAbility**: RGB/CMY/Color Wheel
- **DmxDimmerAbility**: Master brightness
- **DmxBeamAbility**: Focus, Zoom, Iris
- **DmxShutterAbility**: Shutter/strobe
- **DmxPresetAbility**: Preset/macro selection

Check with `HasXxxAbility()` methods. Standard models use *nodes* (pixels); DMX models use *channels*.

---

## Effects Reference

### All Effects (55 total)

Adjust, Bars, Butterfly, Candle, Circles, ColorWash, Curtain, DMX, Duplicate, Faces, Fan, Fill, Fire, Fireworks, Galaxy, Garlands, Glediator, Guitar, Kaleidoscope, Life, Lightning, Lines, Liquid, Marquee, Meteors, Morph, MovingHead, Music, Off, On, Piano, Pictures, Pinwheel, Plasma, Ripple, Servo, Shader, Shape, Shimmer, Shockwave, SingleStrand, Sketch, Snowflakes, Snowstorm, Spirals, Spirograph, State, Strobe, Tendril, Text, Tree, Twinkle, Video, VUMeter, Warp, Wave

Each effect has: `FooEffect.h/.cpp` (logic) + `FooPanel.h/.cpp` (UI).

### Effects Directory Structure

```
xLights/effects/
├── RenderableEffect.h/cpp        # Base class for all effects
├── EffectManager.h/cpp            # Effect registry (RGB_EFFECTS_e enum, factory)
├── EffectPanelUtils.h             # UI helpers
├── FX.h/cpp                       # Additional rendering functions
├── GIFImage.h/cpp                 # GIF handling
├── [All FooEffect.h/.cpp + FooPanel.h/.cpp]
│
├── assist/                        # Assist panels (interactive editing UI)
│   ├── AssistPanel.h/cpp
│   ├── PicturesAssistPanel, SketchAssistPanel, etc.
│   └── xlGridCanvas*.h/cpp
│
├── metal/                         # macOS GPU-accelerated effects (Metal)
│   ├── MetalEffectManager.mm
│   ├── Metal*.mm                  # GPU implementations
│   └── *Functions.metal           # Metal shading kernels
│
├── ispc/                          # SIMD-optimized rendering (Intel SPMD)
│
└── MovingHeadPanels/              # Moving head effect UI components
```

### Effect Registration

Effects are registered in `EffectManager`:
1. Each effect has a unique ID in `RGB_EFFECTS_e` enum
2. `EffectManager` constructor creates all effects via `createEffect()`
3. Stored in maps by ID and by name
4. GPU effects (Metal) created via `CreateGPUEffect()` with CPU fallback

### Key RenderableEffect Virtual Methods

```cpp
// REQUIRED (pure virtual)
virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) = 0;
virtual xlEffectPanel* CreatePanel(wxWindow* parent) = 0;

// Recommended overrides
virtual void SetDefaultParameters();              // Initialize UI defaults
virtual void SetPanelStatus(Model*);              // Update UI for model type
virtual bool CanBeRandom();                       // Randomization support (default: true)
virtual bool CanRenderPartialTimeInterval();       // Partial frame rendering (default: false)
virtual bool CanRenderOnBackgroundThread(...);     // Thread safety (default: true)
virtual bool SupportsRenderCache(...);             // Cache rendered output (default: false)
virtual bool SupportsLinearColorCurves(...);       // Gradient colors (default: false)
virtual bool SupportsRadialColorCurves(...);       // Radial gradients (default: false)
virtual int GetColorSupportedCount();              // Colors used, -1=unlimited (default: -1)

// Migration
virtual bool needToAdjustSettings(const std::string& version);  // Version check
virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults);
virtual void RemoveDefaults(const std::string& version, Effect* effect);

// File/resource tracking
virtual std::list<std::string> GetFileReferences(Model*, const SettingsMap&);
virtual std::list<std::string> GetFacesUsed(const SettingsMap&);
virtual bool CleanupFileLocations(xLightsFrame*, SettingsMap&);

// Validation
virtual std::list<std::string> CheckEffectSettings(const SettingsMap&, AudioManager*, Model*, Effect*, bool);

// Value curve limits (override per-setting)
virtual double GetSettingVCMin(const std::string& name);
virtual double GetSettingVCMax(const std::string& name);
virtual int GetSettingVCDivisor(const std::string& name);  // Default: 1
```

### SettingsMap Prefix Conventions (Detail)

| Prefix | Category | Control Examples |
|--------|----------|-----------------|
| `E_SLIDER_*` | Effect integer slider | `E_SLIDER_Bars_BarCount` |
| `E_TEXTCTRL_*` | Effect text field | `E_TEXTCTRL_Pictures_Filename` |
| `E_CHOICE_*` | Effect dropdown | `E_CHOICE_Bars_Direction` |
| `E_CHECKBOX_*` | Effect boolean | `E_CHECKBOX_Fire_GrowFire` |
| `E_VALUECURVE_*` | Effect animation curve | `E_VALUECURVE_Bars_BarCount` |
| `E_FILEPICKER_*` | Effect file path | `E_FILEPICKER_Shader_File` |
| `C_SLIDER_*` | Color/palette | `C_SLIDER_Brightness`, `C_SLIDER_SparkleFrequency` |
| `C_CHECKBOX_*` | Color enable | `C_CHECKBOX_Palette1` |
| `T_SLIDER_*` | Timing/transition | `T_SLIDER_Speed`, `T_SLIDER_EffectLayerMix` |
| `T_CHOICE_*` | Timing selection | `T_CHOICE_LayerMethod` |
| `T_CHECKBOX_*` | Timing toggle | `T_CHECKBOX_FitToTime`, `T_CHECKBOX_LayerMorph` |
| `T_TEXTCTRL_*` | Timing values | `T_TEXTCTRL_Fadein`, `T_TEXTCTRL_Fadeout` |
| `B_CHOICE_*` | Buffer settings | `B_CHOICE_BufferStyle` |
| `B_VALUECURVE_*` | Buffer animation | `B_VALUECURVE_Rotations`, `B_VALUECURVE_Zoom` |

### SettingsMap API

```cpp
std::string Get(key, defaultValue);   // String with default
int GetInt(key, defaultValue);        // Integer
double GetDouble(key, defaultValue);  // Double
bool GetBool(key, defaultValue);      // Boolean
bool Contains(key);                   // Key exists
void erase(key);                      // Remove key
```

### RenderBuffer Key API

```cpp
// Pixel access
void SetPixel(int x, int y, const xlColor& color, bool wrap = false);
const xlColor& GetPixel(int x, int y) const;
void Fill(const xlColor& color);

// Dimensions (public members)
int BufferWi, BufferHt;

// Drawing primitives
void DrawLine(x1, y1, x2, y2, color);
void DrawCircle(xc, yc, r, color, filled);
void DrawBox(x1, y1, x2, y2, color);
void DrawHLine(y, xstart, xend, color);
void DrawVLine(x, ystart, yend, color);

// Timing/progress
float GetEffectTimeIntervalPosition();             // 0.0–1.0 across effect
float GetEffectTimeIntervalPosition(float cycles);  // With repeat cycles
long GetStartTimeMS() const;
long GetEndTimeMS() const;

// Palette/colors
size_t GetColorCount();
void Get2ColorBlend(idx1, idx2, ratio, result);
void GetMultiColorBlend(float n, bool circular, result);

// Utility
PathDrawingContext* GetPathDrawingContext();    // Vector drawing
TextDrawingContext* GetTextDrawingContext();    // Text rendering
double RandomRange(double num1, double num2);
```

### Value Curves in Effects

Value curves allow settings to animate over the effect duration. Define min/max/divisor:
```cpp
#define EFFECT_PARAM_MIN 0
#define EFFECT_PARAM_MAX 100
#define EFFECT_PARAM_DIVISOR 10  // Supports one decimal place: stored 15 = display 1.5
```

Use in Render:
```cpp
float offset = buffer.GetEffectTimeIntervalPosition();
int value = GetValueCurveInt("ParamName", defaultVal, settings, offset,
                            PARAM_MIN, PARAM_MAX,
                            buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
double dvalue = GetValueCurveDouble("ParamName", defaultVal, settings, offset,
                                   PARAM_MIN, PARAM_MAX,
                                   buffer.GetStartTimeMS(), buffer.GetEndTimeMS(),
                                   PARAM_DIVISOR);
```

### Creating a New Effect

1. **Add enum** to `RGB_EFFECTS_e` in `EffectManager.h`
2. **Create FooEffect.h/cpp**: inherit `RenderableEffect`, implement `Render()` and `CreatePanel()`
3. **Create FooPanel.h/cpp**: inherit `xlEffectPanel`, implement `ValidateWindow()`, define UI controls
4. **Register** in `EffectManager` constructor and `createEffect()` factory
5. **Define** min/max/divisor constants for all value-curve-capable settings
6. **Implement** `SetDefaultParameters()` to initialize UI
7. **Override** `GetSettingVCMin/Max/Divisor()` for value curve settings
8. **Update project files** (.cbp, .vcxproj, .vcxproj.filters)

### Effect Panel UI Controls

Panels use BulkEdit wrapper controls for multi-effect editing:
- `BulkEditSlider` — Integer slider
- `BulkEditSliderF1` — Float slider (one decimal)
- `BulkEditTextCtrl` — Text input
- `BulkEditCheckBox` — Checkbox
- `BulkEditChoice` — Dropdown
- `BulkEditValueCurveButton` — Value curve editor
- `BulkEditColorPicker` — Color picker

### wxSmith UI Definitions (`xLights/wxsmith/`)

Effect panels and most other UI panels/dialogs are designed using **wxSmith** with `.wxs` files in `xLights/wxsmith/` (~189 files). These are XML-based UI layout definitions that generate C++ code.

**Categories of .wxs files:**
- **Effect panels**: `CandlePanel.wxs`, `FirePanel.wxs`, etc. (one per effect)
- **Dialogs**: `AboutDialog.wxs`, `CustomTimingDialog.wxs`, import/export dialogs, etc.
- **Main UI**: `xLightsframe.wxs`, `MainSequencer.wxs`, `PreviewPane.wxs`
- **Settings panels**: `ColorManagerSettingsPanel.wxs`, `ViewSettingsPanel.wxs`, etc.

**How it works:** Each `.wxs` file generates sections in the corresponding `.h`/`.cpp` files, delimited by `//(*...//*)` markers. Manual code is added outside these markers and preserved across regeneration. UI changes should be made in the `.wxs` file; the generated C++ sections should not be hand-edited.

### adjustSettings() Migration Pattern

```cpp
bool FooEffect::needToAdjustSettings(const std::string& version) {
    return IsVersionOlder("2024.05", version);
}
void FooEffect::adjustSettings(const std::string& version, Effect* effect, bool removeDefaults) {
    SettingsMap& s = effect->GetSettings();
    if (IsVersionOlder("2023.01", version)) {
        // Migrate old setting name → new setting name
        // Fix value curve scales with vc.FixScale(oldScale)
    }
    if (RenderableEffect::needToAdjustSettings(version)) {
        RenderableEffect::adjustSettings(version, effect, removeDefaults);
    }
}
```

---

## Outputs & Controllers Reference

### Two Abstraction Layers

xLights separates **protocol handling** (Outputs) from **device management** (Controllers) and **vendor-specific upload** (BaseController):

```
OutputManager (singular, manages everything)
└── Controller instances (connection config, lives in outputs/)
    ├── ControllerEthernet → owns multiple IPOutput instances
    │   └── E131Output, ArtNetOutput, DDPOutput, etc.
    ├── ControllerSerial → owns one SerialOutput instance
    │   └── DMXOutput, LOROutput, RenardOutput, etc.
    └── ControllerNull → owns NullOutput (testing)

BaseController subclasses (vendor upload, lives in controllers/)
    └── Falcon, FPP, WLED, ESPixelStick, etc.
    └── Created temporarily to push config to hardware
```

- **Controller** (`outputs/Controller.h`): Connection configuration — where/how to send data (IP, port, protocol). Owns `Output` objects.
- **Output** (`outputs/Output.h`): Protocol handler — buffers channel data, transmits on `EndFrame()`.
- **BaseController** (`controllers/BaseController.h`): Vendor-specific upload — pushes port/universe config to hardware via HTTP/WebSocket.

### Output Inheritance Hierarchy

```
Output (abstract base)
├── IPOutput (IP/UDP protocols)
│   ├── E131Output        (sACN, 512 ch/universe, UDP 5568)
│   ├── ArtNetOutput      (Art-Net, 512 ch/universe, UDP 6454)
│   ├── DDPOutput         (DDP, up to 2M channels, UDP 4048)
│   ├── KinetOutput       (Kinet protocol)
│   ├── OPCOutput         (Open Pixel Control, up to 65535 ch)
│   ├── TwinklyOutput     (Twinkly smart lights)
│   ├── ZCPPOutput        (ZCPP protocol)
│   ├── PixelNetOutput
│   ├── OpenPixelNetOutput
│   └── xxxEthernetOutput (generic/fallback)
├── SerialOutput (serial/COM port protocols)
│   ├── DMXOutput         (DMX512, up to 4800 ch, 250kbaud)
│   ├── LOROutput         (Light-O-Rama, up to 20480 ch)
│   ├── LOROptimisedOutput
│   ├── RenardOutput      (up to 1015 ch)
│   ├── OpenDMXOutput
│   ├── GenericSerialOutput
│   ├── uDMXOutput
│   └── xxxSerialOutput
└── NullOutput (virtual, no transmission)
```

### Key Output Virtual Methods

```cpp
// Frame pipeline (called every frame in order)
virtual bool Open();                              // Initialize connection
virtual void StartFrame(long msec);               // Begin frame
virtual void SetOneChannel(int32_t ch, uchar data); // Set channel value
virtual void SetManyChannels(int32_t ch, uchar* data, size_t size);
virtual void EndFrame(int suppressFrames);        // Transmit buffered data
virtual void Close();                             // Shutdown

// Required overrides
virtual Output* Copy() = 0;
virtual std::string GetType() const = 0;          // "E131", "ArtNet", etc.
virtual int GetMaxChannels() const = 0;
virtual bool IsValidChannelCount(int32_t) const = 0;
virtual bool IsIpOutput() const = 0;
virtual bool IsSerialOutput() const = 0;
virtual void AllOff() = 0;                        // Safety: all channels to 0
```

### OutputManager (`outputs/OutputManager.h`)

Central authority for all controllers/outputs. Key responsibilities:
- Loads/saves `networks.xml`
- Maps absolute channels to correct Output
- Coordinates frame start/end across all outputs
- Manages parallel transmission, sync universes, duplicate suppression

```cpp
OutputManager::SetOneChannel(absoluteChannel, data)
  → finds Controller for that channel range
  → Controller routes to correct Output
  → Output buffers data in protocol format
  → EndFrame() transmits all outputs
```

### Outputs Directory Structure

```
xLights/outputs/
├── Output.h/cpp                 # Abstract base
├── IPOutput.h/cpp               # Base for IP protocols
├── SerialOutput.h/cpp           # Base for serial protocols
├── NullOutput.h/cpp             # Virtual output (testing)
├── OutputManager.h/cpp          # Central manager
│
├── Controller.h/cpp             # Abstract controller base
├── ControllerEthernet.h/cpp     # IP-based controller
├── ControllerSerial.h/cpp       # Serial port controller
├── ControllerNull.h/cpp         # Null controller
│
├── [Protocol implementations]   # E131Output, ArtNetOutput, DDPOutput, etc.
├── serial.h/cpp                 # Cross-platform serial port abstraction
├── serial_osx.cpp / serial_posix.cpp / serial_win32.cpp
├── TestPreset.h/cpp             # Test pattern presets
└── LorController.h/cpp          # LOR-specific controller management
```

### Vendor Controllers (`controllers/`)

**BaseController** (`controllers/BaseController.h`) is the base for vendor-specific upload handlers. Factory method `BaseController::CreateBaseController(Controller*)` maps vendor name to subclass.

**All vendor implementations:**

| Vendor | Class | Protocol | Notes |
|--------|-------|----------|-------|
| Falcon | Falcon | HTTP REST | F4/F16/F48, V2/V3/V4 firmware variants |
| FPP | FPP | HTTP JSON | Falcon Player on RPi/BeagleBone, also acts as proxy |
| Pixlite16 | Pixlite16 | UDP binary | Custom binary protocol, multiple config versions |
| ESPixelStick | ESPixelStick | HTTP/WebSocket | V3 (HTTP), V4 (WebSocket) |
| AlphaPix | AlphaPix | HTTP forms | Requires HTTP 0.9 for older hardware |
| HinksPix | HinksPix | HTTP JSON | Expansion boards, smart receivers |
| SanDevices | SanDevices | HTTP HTML/JSON | E682/E680/E681, V4 (HTML) vs V5 (JSON) |
| Minleon | Minleon | HTTP JSON | NDB/NDB Pro |
| WLED | WLED | HTTP JSON REST | Open-source WiFi LED firmware |
| J1Sys | J1Sys | HTTP params | Auth support |
| Experience | Experience | HTTP JSON REST | E-fuse detection |
| ILightThat | ILightThat | HTTP | Minimal implementation |

**Key BaseController virtual methods:**
```cpp
// REQUIRED
virtual bool SetOutputs(ModelManager*, OutputManager*, Controller*, wxWindow*) = 0;
virtual bool UsesHTTP() const = 0;

// OPTIONAL
virtual bool SetInputUniverses(Controller*, wxWindow*);          // Configure input reception
virtual bool UploadForImmediateOutput(ModelManager*, OutputManager*, Controller*, wxWindow*);  // Sequence upload
```

**Utility methods inherited from BaseController:**
```cpp
std::string GetURL(url, username, password);   // HTTP GET with proxy support
std::string PutURL(url, body, username, password, contentType);  // HTTP PUT
```

### ControllerCaps (`controllers/ControllerCaps.h`)

Capability registry loaded from `.xcontroller` XML files in the top-level `controllers/` directory. Defines what each controller vendor/model/variant supports:
- Static map: `vendor → model → variant → ControllerCaps`
- 60+ capability query methods: `SupportsUpload()`, `GetMaxPixelPort()`, `SupportsSmartRemotes()`, etc.
- `GetConfigDriver()` returns the driver name that maps to a BaseController subclass

### Controller Definition Files (`controllers/*.xcontroller`)

24 XML files (one per vendor) define controller capabilities. Loaded by `ControllerCaps::LoadControllers()` at startup.

**Vendors:** advatek, entec, espixelstick, experience, falcon, fpp, hanson, hinkspix, holidaycoro, ilightthat, j1sys, kulp, lor, mattosdesigns, microcyb, minleon, RGB2Go, sandevices, scott, Twinkly, wallyslights, wasatch, wled, yps

**XML structure:**
```xml
<Vendor Name="VendorName">
    <AbstractVariant Name="BaseSettings">
        <!-- Reusable base config (never instantiated directly) -->
        <MaxPixelPort>16</MaxPixelPort>
        <SupportsUpload/>
    </AbstractVariant>

    <Controller Name="ModelName">
        <Variant Name="VariantName" ID="UniqueID" Base="VendorName:BaseSettings">
            <!-- Overrides/additions to base -->
            <MaxSerialPort>4</MaxSerialPort>
            <ConfigDriver>Falcon</ConfigDriver>
        </Variant>
    </Controller>
</Vendor>
```

**Inheritance:** Variants inherit from `AbstractVariant` or other Variants via `Base="Vendor:Name"`. Chaining supported. Child elements override parent.

**Key capability fields:**
- **Boolean flags** (presence = true): `<SupportsUpload/>`, `<SupportsFullxLightsControl/>`, `<SupportsSmartRemotes>6</SupportsSmartRemotes>`, `<SupportsPixelPortBrightness/>`, `<SupportsVirtualStrings/>`, `<SupportsAutoLayout/>`, `<DDPStartsAtOne/>`, etc.
- **Integer limits**: `<MaxPixelPort>`, `<MaxSerialPort>`, `<MaxPixelPortChannels>`, `<MaxStartNulls>`, `<MaxGroup>`, `<MaxInputUniverses>`, etc.
- **String config**: `<ConfigDriver>` (maps to BaseController subclass), `<PreferredInputProtocol>`, FPP-specific fields (`<fppStringFileName>`, `<fppStringDriverType>`, `<fppSerialPort1>`)
- **Protocol lists**: `<PixelProtocols>`, `<SerialProtocols>`, `<InputProtocols>` — each contains `<Protocol>` children (e.g., `ws2811`, `dmx`, `e131`)
- **Smart remotes**: `<SmartRemoteTypes>` with `<Type>` children (e.g., `falcon_v1`, `falcon_v2`)
- **Extra properties**: `<ExtraProperties>` with `<Property name="" label="">` for vendor-specific UI config (supports `Enum` and `String` types)

**Adding a new controller:** Create `controllers/vendorname.xcontroller` with `<Vendor>`, add `<Controller>` and `<Variant>` entries with required fields: `MaxPixelPort`, `MaxSerialPort`, `MaxPixelPortChannels`, `PixelProtocols`, `SerialProtocols`, `InputProtocols`, and `ConfigDriver` (must match a BaseController subclass name).

### ControllerUploadData (`controllers/ControllerUploadData.h`)

Data structure for organizing model-to-port mappings during upload:
- **UDController**: Top-level container with pixel/serial/PWM port maps
- **UDControllerPort**: Physical port with list of models and virtual strings
- **UDVirtualString**: Groups models into a single logical string
- **UDControllerPortModel**: Single model mapped to a port

### Upload Workflow

```
User clicks "Upload" → UDController built from Controller + models
  → BaseController::CreateBaseController() instantiates vendor class
  → vendor.SetOutputs() extracts config from UDController
  → HTTP/WebSocket calls push config to hardware
  → Optional: SetInputUniverses() for input config
  → Device reboots and applies new configuration
```

### Data Flow: Effects to Hardware

```
Sequencer renders effects → RenderBuffer (channel values)
  → OutputManager::SetManyChannels(absoluteChannel, data)
  → Controller routes to correct Output by channel range
  → Output::SetOneChannel() buffers in protocol format
  → OutputManager::EndFrame() → all Output::EndFrame()
  → UDP send (IP) or serial write (serial) to hardware
```

---

## Render Pipeline Reference

### Overall Render Flow

```
User edits effect / "Render All" / opens sequence
  → RenderRange() dispatched via RenderCommandEvent
  → BuildRenderTree() — builds channel-overlap dependency graph
  → Render() — creates one RenderJob + AggregatorRenderer per model
      → Clears SequenceData for target frame range
      → Pushes independent jobs (no upstream deps) to JobPool first
      → Pushes dependent jobs to JobPool second
      → Starts 100ms RenderStatusTimer for progress monitoring
  → JobPool threads execute RenderJob::Process() concurrently
      → For each frame: ProcessFrame() per layer, sub-model, strand, node
      → Signal downstream via FrameDone()
  → UpdateRenderStatus() on main thread (100ms poll)
      → RenderMainThreadEffects() for non-thread-safe effects
      → When all jobs complete: cleanup, callback, refresh UI
```

### Key Render Classes (`Render.cpp`)

- **`RenderJob`** (`Job` + `NextRenderer`): One per model. Owns a `PixelBufferClass` (main buffer), plus sub-model/strand/node buffers. `Process()` is the main frame loop executed on a JobPool thread.
- **`NextRenderer`**: Base class for the inter-job dependency chain. Tracks `previousFrameDone` (atomic int), notifies downstream renderers via `FrameDone()`. Uses condition variable with 10ms poll for `waitForFrame()`.
- **`AggregatorRenderer`** (`NextRenderer`): For models sharing channels. Per-frame atomic counters; only propagates `FrameDone()` when ALL upstream jobs have reported for that frame.
- **`EffectLayerInfo`**: Per-element render state: `PixelBufferClass`, current effects per layer, settings maps, valid-layer flags. Extra slot for "blend layer" (model blending).
- **`RenderEvent`**: Synchronization envelope for dispatching effects that must render on the main thread (mutex + condition variable).

**Key constant**: `END_OF_RENDER_FRAME = INT_MAX` — sentinel indicating all frames complete.

### RenderJob::Process() Flow (per model)

1. Acquire `rowToRender->GetRenderLock()` (recursive_timed_mutex)
2. Merge job's frame range with element's dirty range
3. For each layer, find starting effect and initialize settings
4. **Main frame loop** (startFrame → endFrame):
   - Check `abort` flag — if set, save remaining range as dirty and break
   - `ProcessFrame()` for main model layers (bottom-to-top)
   - `ProcessFrame()` for each sub-model/strand
   - Process node-level effects
   - `FrameDone(frame)` — notify downstream
5. Propagate `END_OF_RENDER_FRAME` downstream, cleanup

### ProcessFrame() — Per-Frame Layer Processing

Layers iterated bottom-to-top (highest index first). For each layer:
1. `findEffectForFrame()` — cached index for O(1) amortized lookup
2. Handle Duplicate effects (clone from mirrored model/layer)
3. Detect effect changes → re-initialize
4. Handle persistence (keep buffer between frames) and freeze (stop updating)
5. Canvas mix: pre-load buffer with composited lower layers
6. `RenderEffectFromMap()` — invoke the effect's `Render()` method
7. `HandleLayerBlurZoom()` — apply blur, rotation, zoom transforms
8. `HandleLayerTransitions()` — apply fade in/out transitions
9. `CalcOutput()` — composite all valid layers
10. `GetColors()` — write final pixel data to `SequenceData`

### RenderEffectFromMap() — Effect Invocation

```cpp
bool RenderEffectFromMap(bool suppress, Effect* effectObj, int layer, int period,
    SettingsMap& settings, PixelBufferClass& buffer, bool& resetEffectState,
    bool bgThread, RenderEvent* event)
```

- **Main-thread dispatch**: If effect's `CanRenderOnBackgroundThread()` returns false, packages into a `RenderEvent`, pushes to `mainThreadRenderEvents` queue, waits on condition variable (10s timeout)
- **Per-model parallelism**: For ModelGroups with per-model buffers (`bufCnt > 1`), uses `parallel_for` with `PER_MODEL_POOL`
- **Render cache**: If `SupportsRenderCache()` and cache enabled, tries `effectObj->GetFrame()` first; on miss, renders and calls `effectObj->AddFrame()`
- **Slow frame logging**: Frames >150ms are logged as warnings

### RenderTree — Channel Overlap Dependencies

`RenderTreeData` tracks each model's channel ranges. `BuildRenderTree()` compares all models pairwise for channel overlap. Overlapping models are added to each other's `renderOrder`, creating bidirectional dependencies resolved via `AggregatorRenderer`.

### Render Entry Points

| Method | Trigger | Scope |
|--------|---------|-------|
| `RenderDirtyModels()` | Interactive editing | Only models with dirty ranges |
| `RenderEffectForModel()` | Single effect modified | One model + overlapping models |
| `RenderGridToSeqData()` | "Render All" | All models, all frames |
| `RenderTimeSlice()` | "Render Selected" | All models, time range |

---

## PixelBufferClass Reference

### Overview

`PixelBufferClass` manages a stack of effect layers, each with its own `RenderBuffer`. It handles layer compositing (blending), transitions, sparkle/brightness/contrast adjustments, blur, rotation/zoom, sub-buffer cropping, and per-model buffer decomposition for model groups.

### Layer Architecture

```
PixelBufferClass (one per model element)
├── LayerInfo[0]          ← bottom layer
│   └── RenderBuffer      (effect renders here)
├── LayerInfo[1]
│   └── RenderBuffer
├── ...
├── LayerInfo[N-1]        ← top layer
│   └── RenderBuffer
└── LayerInfo[N]          ← blend layer (for model blending with existing SequenceData)
```

Each `LayerInfo` holds: its `RenderBuffer`, blend mode (`MixTypes`), transition settings, sparkle/brightness/contrast/HSV adjustments, blur, rotation/zoom parameters (all with optional `ValueCurve` animation), sub-buffer cropping, chroma key settings, freeze/suppress timing, and optional per-model buffers for model groups.

### MixTypes (Blend Modes)

| String Name | Enum |
|-------------|------|
| "Normal" | `Mix_Normal` |
| "Effect 1" | `Mix_Effect1` |
| "Effect 2" | `Mix_Effect2` |
| "1 is Mask" | `Mix_Mask1` |
| "2 is Mask" | `Mix_Mask2` |
| "1 is Unmask" | `Mix_Unmask1` |
| "2 is Unmask" | `Mix_Unmask2` |
| "1 is True Unmask" | `Mix_TrueUnmask1` |
| "2 is True Unmask" | `Mix_TrueUnmask2` |
| "1 reveals 2" | `Mix_1_reveals_2` |
| "2 reveals 1" | `Mix_2_reveals_1` |
| "Shadow 1 on 2" | `Mix_Shadow_1on2` |
| "Shadow 2 on 1" | `Mix_Shadow_2on1` |
| "Layered" | `Mix_Layered` |
| "Highlight" | `Mix_Highlight` |
| "Highlight Vibrant" | `Mix_Highlight_Vibrant` |
| "Additive" | `Mix_Additive` |
| "Subtractive" | `Mix_Subtractive` |
| "Brightness" | `Mix_AsBrightness` |
| "Average" | `Mix_Average` |
| "Bottom-Top" | `Mix_BottomTop` |
| "Left-Right" | `Mix_LeftRight` |
| "Max" | `Mix_Max` |
| "Min" | `Mix_Min` |

### Transition Types (22 total)

**Fade** (default): Modifies `fadeFactor` directly.

**Mask-based** (generate per-pixel mask array): Wipe, Clock, From Middle, Square Explode, Circle Explode, Blinds, Blend, Slide Checks, Slide Bars

**Shader-style** (pixel shader on color buffer): Fold, Dissolve, Circular Swirl, Bow Tie, Zoom, Doorway, Blobs, Pinwheel, Star, Swap, Shatter, Circles

### CalcOutput() — Layer Compositing

```cpp
void CalcOutput(int EffectPeriod, const std::vector<bool>& validLayers,
                int saveLayer = 0, bool saveToPixels = false)
```

1. Count valid layers, detect if sparkles needed
2. Initialize sparkle vector if needed (`rand() % 10000` per node)
3. Try GPU blending: `GPURenderUtils::BlendLayers()` — returns if successful
4. CPU fallback: ISPC SIMD-optimized `blendLayers()`:
   - For each node (parallelized in blocks of 4096):
     - Copy pixel data from each valid layer's RenderBuffer into per-node blend buffer
     - Apply per-layer: HSV adjust → sparkles → brightness/contrast → brightness-level
     - Blend layers: first valid layer uses `FirstLayerFade`; subsequent layers apply mix-type-specific blend function
   - Set blended color on each node of save layer

### GPU Acceleration Tiers

1. **Metal** (macOS): `GPURenderUtils` → `MetalComputeUtilities` for blur, roto-zoom, transitions, layer blending
2. **ISPC** (all platforms): SIMD-vectorized kernels for color operations, sparkle, brightness, blending
3. **Pure C++**: Final fallback for all operations

### GetColors() — Writing to SequenceData

```cpp
void GetColors(unsigned char* fdata, const std::vector<bool>& restrictRange)
```

Iterates layer 0's nodes. For each node: applies dimming curve if present, then writes channel data to `fdata` at the node's `ActChan` offset. Parallelized with `parallel_for` (grain 500) for models with ≥1000 nodes.

### Buffer Initialization

- `InitBuffer(Model, layers, timing)` — main model: creates `layers+1` LayerInfo objects
- `InitStrandBuffer(Model, strand, timing, layers)` — single strand
- `InitNodeBuffer(Model, strand, node, timing)` — single node (2 layers)
- `InitPerModelBuffers(ModelGroup, layer, timing)` — shallow per-model buffers
- `InitPerModelBuffersDeep(ModelGroup, layer, timing)` — deep (flat) per-model buffers

### Post-Processing Pipeline (per layer, per frame)

1. **Blur**: Value-curve-animated Gaussian blur (3-pass box blur approximation). GPU-accelerated on macOS.
2. **RotoZoom**: 3-axis rotation (X, Y, Z configurable order) + zoom with pivot point. GPU-accelerated on macOS.
3. **Transitions**: Fade in/out with 22 transition types (mask-based or shader-style).
4. **Sparkle**: Random pixels set to sparkle color, music-reactive count optional.
5. **Brightness/Contrast**: HSV-based brightness scaling and contrast curve.
6. **HSV Adjust**: Hue/Saturation/Value offsets with value curve animation.

---

## RenderBuffer Reference

### Overview

`RenderBuffer` is the 2D pixel buffer that effects render into. Each effect layer gets its own instance. Coordinate system: **(0,0) at lower-left**, x rightward, y upward. Pixel storage is row-major: `pixels[y * BufferWi + x]`.

### Key Members

```cpp
int BufferWi, BufferHt;              // Buffer dimensions
xlColor* pixels;                     // Active pixel array (may point to GPU shared memory)
xlColor* tempbuf;                    // Secondary buffer for double-buffering
std::vector<NodeBaseClassPtr> Nodes; // Node list mapping buffer pixels to model nodes
PaletteClass palette;                // Effect color palette
int curPeriod;                       // Current frame number
int curEffStartPer, curEffEndPer;    // Effect start/end frame numbers
int frameTimeInMs;                   // Frame duration (default 50ms = 20fps)
bool needToInit;                     // True when effect needs re-initialization
bool allowAlpha;                     // Alpha channel enabled
bool dmx_buffer;                     // True for DMX model buffers
std::map<int, EffectRenderCache*> infoCache;  // Per-effect persistent state
void* gpuRenderData;                 // Opaque GPU renderer attachment
```

### Pixel Operations

```cpp
void SetPixel(int x, int y, const xlColor& color, bool wrap = false, bool useAlpha = false);
const xlColor& GetPixel(int x, int y) const;
void SetNodePixel(int nodeNum, const xlColor& color);
void Fill(const xlColor& color);                    // std::fill_n for performance
void CopyPixel(int srcx, int srcy, int destx, int desty);
void ProcessPixel(int x, int y, const xlColor& color, bool wrap_x, bool wrap_y);
// Direct access (no bounds check, for inner loops)
void SetPixelDirect(int x, int y, const xlColor& color);
const xlColor& GetPixelDirect(int x, int y) const;
```

### Drawing Primitives

```cpp
void DrawLine(x1, y1, x2, y2, color, useAlpha);          // Bresenham's algorithm
void DrawThickLine(x1, y1, x2, y2, color, thickness);    // Multiple offset lines
void DrawCircle(xc, yc, r, color, filled, wrap);          // Midpoint circle algorithm
void DrawFadingCircle(x0, y0, radius, color, wrap);       // Brightness fades from center
void DrawBox(x1, y1, x2, y2, color, wrap, useAlpha);     // Filled rectangle
void DrawHLine(y, xstart, xend, color, wrap);
void DrawVLine(x, ystart, yend, color, wrap);
void FillConvexPoly(poly, color);                          // Scan-line convex polygon fill
```

### Double Buffering (Temp Buffer)

```cpp
void ClearTempBuf();
void SetTempPixel(int x, int y, const xlColor& color);
const xlColor& GetTempPixel(int x, int y);
void CopyTempBufToPixels();    // memcpy tempbuf → pixels
void CopyPixelsToTempBuf();    // memcpy pixels → tempbuf
```

Common pattern: copy current frame to temp, clear pixels, read from temp while writing new pixels.

### Timing Helpers

```cpp
float GetEffectTimeIntervalPosition();                // [0..1] across effect duration
float GetEffectTimeIntervalPosition(float cycles);    // [0..1] within current cycle
long GetStartTimeMS() const;
long GetEndTimeMS() const;
static double calcAccel(double ratio, double accel);  // Ease-in/out curve, accel in [-10, 10]
```

### Color Palette Access

```cpp
// PaletteClass (accessed via buffer.palette)
size_t Size();                                         // Palette color count
const xlColor& GetColor(size_t idx);                   // Static color
xlColor GetColor(size_t idx, float progress);           // Time-varying gradient
void GetSpatialColor(idx, x, y, color);                // Position-based color

// RenderBuffer color blending
void Get2ColorBlend(idx1, idx2, ratio, color);
void GetMultiColorBlend(float n, bool circular, color, reserveColors);
void Get2ColorAlphaBlend(c1, c2, ratio, color);
```

**Spatial color curve types**: TC_TIME (0), TC_RIGHT (1), TC_DOWN (2), TC_LEFT (3), TC_UP (4), TC_RADIALIN (5), TC_RADIALOUT (6), TC_CW (7), TC_CCW (8)

### Drawing Contexts

```cpp
PathDrawingContext* GetPathDrawingContext();   // Vector path drawing (shapes, curves)
TextDrawingContext* GetTextDrawingContext();   // Text rendering with font management
```

Both lazily allocated from thread-safe pools. Use `FlushAndGetImage()` to get rendered wxImage, then copy pixels to buffer. Anti-aliasing disabled for pixel-resolution output.

### EffectRenderCache

```cpp
class EffectRenderCache { virtual ~EffectRenderCache(); };
```

Base class for effects to store persistent state between frames (particle positions, wave phase, etc.). Stored in `buffer.infoCache` keyed by integer identifier.

### Node Index Mapping

`InitBuffer()` builds `indexVector` — precomputed mapping from node index to pixel buffer offset:
- **Simple nodes** (single coordinate): direct flat index
- **Multi-coordinate nodes**: high bit flag (`0x80000000`) + chained secondary indices
- **Out-of-bounds**: `0xFFFFFFFF`

---

## JobPool Reference

### Overview

`JobPool` is the thread pool used for parallel rendering. Classic producer-consumer pattern with dynamic thread scaling and automatic shrinkage after 30 seconds of idle time.

### Job Interface

```cpp
class Job {
    virtual void Process() = 0;                    // Execute on worker thread
    virtual bool DeleteWhenComplete() { return false; }  // Auto-delete after Process()
    virtual bool SetThreadName() { return true; }  // Set OS thread name during execution
    virtual std::string GetStatus();
    virtual const std::string GetName() const;
};
```

**Known subclasses**: `RenderJob` (effect rendering), `ParallelJob` (parallel_for chunks), `ParallelListJob` (parallel list iteration), `ResolveJob` (async DNS)

### Thread Pool Configuration

```cpp
JobPool(const std::string& threadNameBase);
void Start(size_t poolSize, size_t minPoolSize = 0);  // Configure limits, no threads created
void PushJob(Job* job);          // Queue job, create threads on demand
void PushJobs(const std::list<Job*>& jobs);  // Batch queue
```

- Threads created lazily on demand (minimum 4 on first creation)
- Max capped at 250 threads
- Idle workers exit after 30 seconds if above `minNumThreads`
- FIFO dispatch (no priority queue)

### Synchronization

| Mechanism | Purpose |
|-----------|---------|
| `queueLock` (mutex) | Protects job queue |
| `threadLock` (mutex) | Protects worker vector |
| `signal` (condition_variable) | Wakes idle workers |
| `numThreads` (atomic) | Live thread count |
| `idleThreads` (atomic) | Blocked-on-queue count |
| `inFlight` (atomic) | Jobs pushed but not completed |

### Pool Instances

- `xLightsFrame::jobPool` — "RenderPool", main render thread pool
- `ParallelJobPool::POOL` — "parallel_tasks", for `parallel_for()` operations
- `ParallelJobPool::PER_MODEL_POOL` — "per_model_pool", for per-model buffer rendering within model groups
- `RESOLVE_POOL` — "RESOLVE_POOL" (max 128), for async DNS resolution

---

## RenderCache Reference

### Overview

Disk-backed cache for rendered effect frames. Stores raw pixel buffers indexed by effect identity and settings. Per-sequence storage.

### Cache Location

```
<show_directory>/RenderCache/<sequence_filename>_RENDER_CACHE/
```
Or custom path if configured.

### Cache File Format

**File naming**: `<EffectName>_<ElementName>_<LayerNumber>_<StartTimeMS>.cache`

**Binary format**:
1. **Header**: null-terminated key-value string pairs (`key\0value\0...RC_HEADEREND\0`)
2. **Model info**: per model: `modelName\0frameCount\0frameSize\0`
3. **Frame data**: raw `xlColor` arrays per model per frame

On macOS, uses `mmap()` for zero-copy frame reads.

### Cache Matching (`IsMatch`)

All must match: StartMS, EndMS, EffectLayer, Element name, FPS, buffer size, effect name, property count, all settings key-value pairs, all palette entries.

### Cache Modes

| Mode | Behavior |
|------|----------|
| "Enabled" | All effects cached |
| "Locked Only" | Only effects with `X_Effect_Locked=True` |
| "Disabled" | No caching; existing cache deleted on sequence open |

### Effects Excluded from Caching

- Canvas mode enabled (`T_CHECKBOX_Canvas=1`)
- Persistent overlay (`B_CHECKBOX_OverlayBkg=1`)
- Memory usage exceeds 3× physical memory
- Effect's `SupportsRenderCache()` returns false

### Integration with Render Pipeline

```cpp
// In RenderEffectFromMap():
if (reff->SupportsRenderCache(settings) && _renderCache.IsEnabled()) {
    if (!effectObj->GetFrame(*rb, _renderCache)) {  // cache miss
        reff->Render(effectObj, settings, *rb);       // render normally
        effectObj->AddFrame(*rb, _renderCache);       // store in cache
    }
}
```

### Thread Safety

- `_cacheLock` (recursive_mutex): protects top-level cache map
- `PerEffectCache::lock` (shared_mutex): per-effect reader-writer lock
- `_loadMutex` (mutex): held during background cache loading
- Cache items are removed from shared list during use (exclusive access without locking the item itself)


