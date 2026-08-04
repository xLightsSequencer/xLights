# 04. Effects Catalog & Settings Panels

_Generated from code on 2026-07-31. Status: ✅ parity | 🟡 partial | ❌ missing on iPad | 🚫 infeasible/restricted on iPad (reason required) | 🔵 iPad-only._

**Authoritative effect set:** the 56 entries of `EffectManager::RGB_EFFECTS_e`
(`src-core/effects/EffectManager.h:24-80`, `eff_OFF` … `eff_WAVE`, terminated by
`eff_LASTEFFECT`), instantiated in `EffectManager.cpp:182-239`. The 58 `*Effect.cpp`
files in `src-core/effects/` include two non-catalog entries (`RenderableEffect.cpp`
base class, `SPIRVShaderEffect.cpp` shader backend), so the file count overshoots the
registered set by two.

**Structural headline:** effect settings panels are driven from **one shared JSON
schema on both platforms**. `resources/effectmetadata/<Effect>.json` (56 files, one
per registered effect, plus `shared/{Color,Buffer,Blending}.json` and `_schema.json`)
is consumed by desktop's `JsonEffectPanel` (`src-ui-wx/effectpanels/JsonEffectPanel.cpp:99`
`BuildFromJson`) and by iPad's `EffectMetadataPanel` /
`EffectPropertyView` (`src-iPad/App/EffectSettingsView.swift:563`,
`src-iPad/App/EffectPropertyView.swift:126`). 55 of 56 desktop panels register through
that engine (`EffectPanelManager.cpp:104-161`); only Moving Head still uses a
hand-built wx panel (`EffectPanelManager.cpp:132`). Consequently the per-effect
parity picture is unusually strong — the interesting gaps are in the desktop
*subclass* hooks layered on top of the JSON engine, and in three assist/browser
surfaces.

**Every JSON `custom` row has an iPad implementation.** Cross-checking all 48
`controlType: "custom"` property ids declared across the 56 effect JSONs plus the
three shared panels against the dispatcher at
`src-iPad/App/EffectPropertyView.swift:164-283` yields zero unhandled ids — no effect
falls through to the read-only `customPlaceholder` (`EffectPropertyView.swift:482`).

---

## Per-effect matrix

| # | Effect | Renders on iPad | iPad settings UI | Desktop assist/extras | GPU/SIMD (platforms) | Notes |
|---|---|---|---|---|---|---|
| 1 | Off | ✅ `OffEffect.cpp`; no platform gate | ✅ `Off.json` (1 prop) | `RegisterJson` — no subclass (`EffectPanelManager.cpp:104`) | — | Simplest effect; pure shared core. |
| 2 | On | ✅ `OnEffect.cpp` | ✅ `On.json` (5 props) | `RegisterJson` (`:105`) | — | Default effect for a bare iPad drop (`SequencerViewModel.swift:7063`). |
| 3 | Adjust | ✅ `AdjustEffect.cpp` | ✅ `Adjust.json` (6 props); dynamic label via `dynamicLabelOverride` (`EffectSettingsView.swift:774`) | `RegisterJsonSubclass<AdjustPanel>` (`:106`) — `ValidateWindow` only (`AdjustPanel.h:18`) | — | Action-dependent labels ("Adjust by:" / "Minimum:") work on both. |
| 4 | Bars | ✅ `BarsEffect.cpp` | ✅ `Bars.json` (8 props) | `RegisterJson` (`:107`) | ISPC `ispc/BarsFunctions.ispc` (all); Metal `MetalBarsEffect` (`MetalEffectManager.mm:234`) macOS+iPad; Vulkan (`VulkanEffectManager.cpp`) Win/Linux | ISPC path skipped for Custom Horz/Vert (needs spatial data, `BarsEffect.cpp:137`). |
| 5 | Butterfly | ✅ | ✅ `Butterfly.json` (6) | `RegisterJson` (`:108`) | ISPC + Metal (`:196`) + Vulkan | |
| 6 | Candle | ✅ | ✅ `Candle.json` (6) | `RegisterJson` (`:109`) | ISPC + Metal (`:218`) + Vulkan | |
| 7 | Circles | ✅ | ✅ `Circles.json` (11) | `RegisterJson` (`:110`) | ISPC + Metal (`:236`) + Vulkan | |
| 8 | Color Wash | ✅ | ✅ `ColorWash.json` (6) | `RegisterJson` (`:111`) | ISPC + Metal (`:230`) + Vulkan | |
| 9 | Curtain | ✅ | ✅ `Curtain.json` (5) | `RegisterJson` (`:112`) | — | |
| 10 | DMX | ✅ `DMXEffect.cpp`, no gate | ✅ `DMX.json` (2 custom rows) → `DMXChannelsNotebookView`, `DMXButtonsRow` (`EffectPropertyView.swift:268-271`); remap via `DMXRemapGridSheet.swift` | `RegisterJsonSubclass<DMXPanel>` (`:113`); `OnRemapClick`/`OnRemapRClick`/`OnRemapBulkEdit` (`DMXPanel.h:44-46`) | — | Desktop `SetDefaultParameters` (`DMXPanel.cpp:171-185`) zeroes all 32 channel sliders + value curves on a fresh effect; iPad relies on metadata defaults instead (see cross-cutting #2). |
| 11 | Duplicate | ✅ | ✅ `Duplicate.json` (7) | `RegisterJsonSubclass<DuplicatePanel>` (`:114`) — `SetPanelStatus` only (`DuplicatePanel.h:18`) | — | |
| 12 | Faces | 🟡 renders (shared core, no gate) but needs model face definitions | 🟡 `Faces.json` (12 props, 2 custom → `FacesMouthMovementsRowView`, `TransparentBlackRowView`); `dynamicOptions: faces`/`states` resolved by `SequencerViewModel.swift:3568-3570` | `RegisterJsonSubclass<FacesPanel>` (`:115`); `PopulateLyricTimingTracks` filters to 3-layer lyric tracks (`FacesPanel.cpp:166-177`) | — | **Gap:** desktop `FacesPanel::SetDefaultParameters` (`FacesPanel.cpp:154-164`) preselects phoneme mode + the first lyric timing track on a new effect; iPad's `addEffect` seeds `settings: ""` for a type change (`SequencerViewModel.swift:7094`), so a fresh Faces effect has no track chosen. 2026-08: auto eye blink is now computed purely per frame in shared core (`FacesEffect::IsAutoBlinkClosed`), making Faces frame-parallel `Pure` — auto-applied to both platforms. |
| 13 | Fan | ✅ | ✅ `Fan.json` (16) | `RegisterJson` (`:116`) | ISPC + Metal (`:208`) + Vulkan | |
| 14 | Fill | ✅ | ✅ `Fill.json` (8) | `RegisterJson` (`:117`) | ISPC + Metal (`:232`) + Vulkan | |
| 15 | Fire | ✅ | ✅ `Fire.json` (6) | `RegisterJson` (`:118`) | ISPC + Metal (`:222`) + Vulkan | |
| 16 | Fireworks | ✅ | ✅ `Fireworks.json` (14) | `RegisterJson` (`:119`) | — | |
| 17 | Galaxy | ✅ | ✅ `Galaxy.json` (14) | `RegisterJson` (`:120`) | ISPC + Metal (`:210`) + Vulkan | |
| 18 | Garlands | ✅ | ✅ `Garlands.json` (4) | `RegisterJson` (`:121`) | ISPC + Metal (`:226`) + Vulkan | |
| 19 | Glediator | ✅ `GlediatorEffect.cpp`, no gate | ✅ `Glediator.json` (2 props); `Glediator_Filename` is a plain `filepicker` → `FilepickerPropertyView` (`EffectPropertyView.swift:137`) | `RegisterJson` (`:122`) | — | `.gled` asset access goes through the standard iPad file picker + sandbox bookmark path, same as any other file property. |
| 20 | Guitar | ✅ | ✅ `Guitar.json` (10) | `RegisterJson` (`:123`) | — | |
| 21 | Kaleidoscope | ✅ | ✅ `Kaleidoscope.json` (5) | `RegisterJson` (`:124`) | ISPC + Metal (`:206`) + Vulkan | |
| 22 | Life | ✅ | ✅ `Life.json` (3) | `RegisterJson` (`:125`) | ISPC + Metal (`:220`) + Vulkan | |
| 23 | Lightning | ✅ | ✅ `Lightning.json` (8) | `RegisterJson` (`:126`) | — | |
| 24 | Lines | ✅ | ✅ `Lines.json` (6) | `RegisterJson` (`:127`) | — | |
| 25 | Liquid | ✅ LiquidFun/Box2D links on every Apple target (`libliquidfun.a`, `macOS/xLights.xcodeproj/project.pbxproj:354`) | ✅ `Liquid.json` — the largest panel at 45 props, 0 custom rows | `RegisterJson` (`:128`) | ISPC `ispc/LiquidFunctions.ispc` (all platforms); **no** GPU kernel on any platform | Physics effect — per AGENTS.md §4 it is a known non-deterministic source for headless-vs-desktop fseq diffs on *all* platforms, not an iPad-specific issue. |
| 26 | Marquee | ✅ | ✅ `Marquee.json` (14) | `RegisterJson` (`:129`) | — | |
| 27 | Meteors | ✅ | ✅ `Meteors.json` (11) | `RegisterJson` (`:130`) | ISPC + Metal (`:238`) + Vulkan | |
| 28 | Morph | ✅ | ✅ `Morph.json` (22 props, 3 custom → `MorphSwapRowView`, `MorphQuickSetRowView`, `MorphLineEditorRowView`); `Morph_LineEditor` is `"platform": "ipad"` | `RegisterJsonSubclass<MorphPanel>` (`:131`); **assist panel** `HasAssistPanel()==true` (`MorphPanel.h:26`) → `xlGridCanvasMorph` | — | The desktop assist canvas and the iPad inline `MorphLineEditorRowView` are two designs for the same start/end line editing; both reach the same settings keys. |
| 29 | Moving Head | ✅ `MovingHeadEffect.cpp`, no gate | 🟡 `MovingHead.json` (42 props, 7 iPad-only custom rows: `MHFixtures`, `MHIpadInfo`, `MHColorRow`, `MHColorWheelRow`, `MHDimmerRow`, `MHPathRow`, `MHLinkRow`) | **Only non-JSON desktop panel** — `Register<MovingHeadPanel>` (`:132`), with `MovingHeadPanels/` sub-panels (`MovingHeadCanvasPanel`, `MHRgbPickerPanel`, `MHColorWheelPanel`, `MovingHeadDimmerPanel`, preset bitmap buttons) | — | The **settings-key split is deliberate and not a gap**: the 9 `"platform": "desktop"` entries (`MHPathDef`, `MH1_Settings`…`MH8_Settings`) are hidden `wxTextCtrl` serialization sinks (`MovingHeadPanel.cpp:373-374` — `TextCtrl_MHPathDef->Hide()`), and the iPad rows write the identical `E_TEXTCTRL_MH<n>_Settings` keys (`MovingHeadFixtureRowView.swift:8,63`). iPad path editor round-trips desktop's Bezier format verbatim (`MovingHeadPathEditorRowView.swift:3-16`). **Three real sub-gaps keep this partial** (see cross-cutting #40): (a) desktop's `MHColorWheelPanel` and `MHRgbPickerPanel` each hold a `std::vector<HandlePoint> m_handles` (`MHColorWheelPanel.h:93`, `MHRgbPickerPanel.h:82`) — multiple colour handles serialized by `GetColour()` — while iPad's `MovingHeadColorWheelRowView` writes exactly one `hue,sat,val` triple per pick (`MovingHeadPresetViews.swift:83`); (b) desktop scans a user preset directory for `*.xmh` and populates all four preset grids — Position, Path, Dimmer, and now Pattern (`MHPatternPresetBitmapButton`, `MovingHeadPanel.cpp` `ProcessPresetDir`/`LoadMHPreset`, save via `OnButtonSavePatternPresetClick`/`GetPatternDef`) — where iPad ships hardcoded strips only (`MovingHeadPresetViews.swift:19`, `MovingHeadPathEditorRowView.swift:512`), and has no Pattern strip at all; (c) desktop's panel "Reset to Default" button (`MovingHeadPanel.cpp:611`, handler `:2983` — clears every VC, dimmer and path command) has no iPad counterpart — grepped `src-iPad/` for `ResetToDefault`, `resetMovingHead`: no hits; (d) desktop's fixture quick-select toolbar (`Button_All`/`_None`/`_Evens`/`_Odds`/`_Left`/`_Right`, `MovingHeadPanel.cpp:OnButton_*Click`) has no iPad equivalent — `MovingHeadFixtureRowView` (`MovingHeadFixtureRowView.swift:16-92`) only exposes the 8 individual per-fixture toggle buttons, no bulk-select shortcuts. |
| 30 | Music Effect | ✅ — reads `AudioManager::GetFrameData` (`MusicEffect.cpp:364`), which is **outside** the iOS gate | ✅ `MusicEffect.json` (11) | `RegisterJsonSubclass<MusicPanel>` (`:135`) — `ValidateWindow` only | — | Registration name must match `MusicEffect::Name()` exactly (`EffectPanelManager.cpp:133-135`). |
| 31 | Piano | 🟡 renders, but its note source is a 3-layer note timing track (`PianoEffect.cpp:72`, `E_CHOICE_Piano_MIDITrack_APPLYLAST`) | ✅ `Piano.json` (8 props); track list via `dynamicOptions: timingTracks` | `RegisterJsonSubclass<PianoPanel>` (`:136`) — `ValidateWindow` only | — | **Gap:** `AudioManager::DoPolyphonicTranscription` — the way that note track gets generated — is compiled out on iOS (`src-core/media/AudioManager.h:233` `#if !TARGET_OS_IPHONE`), along with the whole `xLightsVamp` host (`AudioManager.h:22,78,165`). A desktop-authored note track renders correctly on iPad; the iPad cannot originate one. Track *creation* is theme 03's row. |
| 32 | Pictures | ✅ | 🟡 `Pictures.json` (17 props, 2 custom → `EffectFilenameBlockView`, `TransparentBlackRowView`); add-time resize prompt in `PicturesResizeOnAddSheet.swift`; `xyCenter` group → `XYCenterPadView` (`EffectSettingsView.swift:693`) | `RegisterJsonSubclass<PicturesPanel>` (`:137`); **assist panel** (`PicturesPanel.h:40`) → `PicturesAssistPanel` + `xlGridCanvasPictures` — a per-pixel image editor with Load/Save/Save-As image buttons (`PicturesAssistPanel.h:51-55`) | — | **Gap:** no iPad counterpart to the pixel-grid image editor. Searched `src-iPad/` for `PicturesAssist`, `GridCanvasPictures`, `PixelEditor` — only unrelated `LORPixelEditor` import-format hits (`XLImportSession.mm:15`); no drawing surface exists. |
| 33 | Pinwheel | ✅ | ✅ `Pinwheel.json` (11) | `RegisterJson` (`:138`) | ISPC + Metal (`:202`) + Vulkan | |
| 34 | Plasma | ✅ | ✅ `Plasma.json` (4) | `RegisterJson` (`:139`) | ISPC + Metal (`:198`) + Vulkan | |
| 35 | Ripple | ✅ | ✅ `Ripple.json` (17 props, 1 custom → `RippleSVGRowView`) | `RegisterJsonSubclass<RipplePanel>` (`:140`); `CreateCustomControl`, `OnObjectChanged`/`OnStyleChanged` (`RipplePanel.h:21-27`) | — | |
| 36 | Servo | ✅ | ✅ `Servo.json` (6 props, 2 custom → `ServoStartEndRowView`, `ServoButtonRowView`) | `RegisterJsonSubclass<ServoPanel>` (`:141`); `OnSyncClicked`/`OnEqualClicked` link buttons (`ServoPanel.h:40-43`) | — | Desktop `SetDefaultParameters` only resets to static values (`ServoPanel.cpp:337-345`) — iPad's metadata defaults cover the same ground. |
| 37 | Shader | 🟡 **different backend on iPad.** Desktop = OpenGL (`ShaderEffect.cpp:125-624`, `#if !TARGET_OS_IPHONE`). iOS has no GL: `MetalShaderEffect` translates ISF GLSL → MSL via `MetalShaderTranslator` with `forIOS=true` (`MetalShaderEffect.mm:69-74`). If Metal compute is unavailable the base `Render` paints the buffer solid red (`ShaderEffect.cpp:1138-1142`) | 🟡 `Shader.json` (7 props, 3 custom → `EffectFilenameBlockView`, `ShaderSpeedRowView`, `ShaderDynamicParamsView`) | `RegisterJsonSubclass<ShaderPanel>` (`:142`); `OnDownloadClicked` → `ShaderDownloadDialog` (`ShaderPanel.h:63`, `src-ui-wx/effects/ShaderDownloadDialog.cpp`) | Metal (`MetalEffectManager.mm:240`) macOS+iPad; Vulkan on Linux and Windows — but Windows Vulkan is **compute-only**, `HAVE_VULKAN_SHADER` is gated to `(UNIX AND NOT APPLE) OR WIN32` with the note that Windows links it only via the vcpkg path (`CMakeLists.txt:608-615`) | **Gap:** no shader-download browser on iPad — searched `src-iPad/` for `ShaderDownload`, `shadertoy`: no hits. iPad users must side-load `.fs` files through `EffectFilenameBlockView`. |
| 38 | Shape | ✅ | 🟡 `Shape.json` (27 props, 4 custom → `ShapeFontRowView`, `ShapeCharRowView`, `ShapeSkinToneRowView`, `ShapeSVGRowView`) | `RegisterJsonSubclass<ShapePanel>` (`:143`); `OnEmojiContextMenu`/`OnEmojiMenuItem` → `CharMapDialog` glyph grid (`ShapePanel.cpp:243`) | — | **Weaker on iPad:** emoji selection is a numeric code-point stepper (range 32..917631) with a single-glyph preview (`EffectCustomRowsExtra.swift:425-483`), versus desktop's browsable character-map grid. Functional, but you have to know the code point. |
| 39 | Shimmer | ✅ | ✅ `Shimmer.json` (4) | `RegisterJson` (`:144`) | ISPC + Metal (`:216`) + Vulkan | |
| 40 | Shockwave | ✅ | ✅ `Shockwave.json` (14) | `RegisterJson` (`:145`) | ISPC + Metal (`:204`) + Vulkan | |
| 41 | SingleStrand | ✅ | ✅ `SingleStrand.json` (18) | `RegisterJson` (`:146`) | — | Render-side settings decode is cached per effect in `SingleStrandRenderCache` (`SingleStrandEffect.cpp`) — `src-core/`, so both platforms get it; no UI surface. |
| 42 | Sketch | ✅ `SketchEffect.cpp` + `SketchEffectDrawing.cpp` + `SketchSVGImport.cpp`, no gate | ✅ `Sketch.json` (9 props, 4 custom → `SketchInfoRowView`, `SketchDefRowView`, `SketchPathEditorRowView`, `SketchBackgroundRowView`); `Sketch_PathEditor` is `"platform": "ipad"` | `RegisterJsonSubclass<SketchPanel>` (`:147`); **assist panel** (`SketchPanel.h:51`) → `SketchAssistPanel` + `SketchCanvasPanel` | — | Unlike Pictures, the Sketch assist canvas *does* have an iPad counterpart — inline as a settings row rather than a docked pane. |
| 43 | Snowflakes | ✅ | ✅ `Snowflakes.json` (5) | `RegisterJsonSubclass<SnowflakesPanel>` (`:148`) — `ValidateWindow` only | — | |
| 44 | Snowstorm | ✅ | ✅ `Snowstorm.json` (3) | `RegisterJson` (`:149`) | — | |
| 45 | Spirals | ✅ | ✅ `Spirals.json` (8) | `RegisterJson` (`:150`) | ISPC + Metal (`:228`) + Vulkan | |
| 46 | Spirograph | ✅ | ✅ `Spirograph.json` (7) | `RegisterJson` (`:151`) | — | |
| 47 | State | 🟡 renders; needs model state definitions | 🟡 `State.json` (5 props, 1 custom → `StateStateSourceRowView`); `dynamicOptions: states` (`SequencerViewModel.swift:3568`) | `RegisterJsonSubclass<StatePanel>` (`:152`); `SetRenderableEffect`, `SetPanelStatus` (`StatePanel.h:25-32`) | — | **Gap:** same root cause as Faces — `StatePanel::SetDefaultParameters` (`StatePanel.cpp:114-121`) selects state[0] and timing-track[0] on a new effect; iPad seeds an empty settings string. |
| 48 | Strobe | ✅ | ✅ `Strobe.json` (4) | `RegisterJson` (`:153`) | — | |
| 49 | Tendril | ✅ | ✅ `Tendril.json` (13) | `RegisterJson` (`:154`) | — | |
| 50 | Text | ✅ — **different text backend.** Desktop uses the wx `TextDrawingContext`; iOS registers a CoreGraphics implementation (`src-iPad/Bridge/CoreGraphicsTextDrawingContext.mm:80,389` — `RegisterCoreGraphicsTextDrawingContext` installs it via `TextDrawingContext::RegisterFactory`) | ✅ `Text.json` (17 props, 2 custom → `EffectFilenameBlockView` for `Text_File_Row`, `TextFontXLRowView`); plus a `fontpicker` → `FontpickerPropertyView`; `dynamicOptions: lyricTimingTracks` | `RegisterJsonSubclass<TextPanel>` (`:155`); `SetPanelStatus`, `CreateCustomControl` (`TextPanel.h:27-32`) | — | The CoreGraphics context deliberately matches desktop's `antiAliased=false` behavior (`CoreGraphicsTextDrawingContext.mm:239`). Glyph rasterization differs between wx and CoreGraphics, so pixel-exact text output is not expected to match across platforms. |
| 51 | Tree | ✅ | ✅ `Tree.json` (3) | `RegisterJson` (`:156`) | ISPC + Metal (`:212`) + Vulkan | |
| 52 | Twinkle | ✅ | ✅ `Twinkle.json` (5) | `RegisterJson` (`:157`) | ISPC + Metal (`:214`) + Vulkan | |
| 53 | Video | ✅ — **different decoder.** `VideoReader` is AVFoundation-backed on Apple (shared per-file decoder + pts-indexed frame cache, position-independent) and FFmpeg-backed elsewhere (forward-decoding) (`VideoEffect.cpp:70-78`) | ✅ `Video.json` (13 props, 3 custom → `EffectFilenameBlockView`, `VideoDurationRowView`, `TransparentBlackRowView`) | `RegisterJsonSubclass<VideoPanel>` (`:158`); `OnSelectClick`/`OnClearClick`, live preview thumbnail (`VideoPanel.h:76-78`, `VideoPanel.cpp:125` `UpdatePreview`) | ISPC `ispc/VideoFunctions.ispc` (all platforms); **no** GPU kernel | Frame-parallel video render is opt-in behind `XL_VIDEO_PARALLEL=1` until the AVFoundation bridge handles it (`VideoEffect.cpp:105`). |
| 54 | VU Meter | ✅ — `GetFrameData` (`VUMeterEffect.cpp:1135`), outside the iOS gate | ✅ `VUMeter.json` (16 props); `dynamicOptions: audioTracks` → alt/stem tracks, resolved via `altTrackCount`/`altTrackDisplayName` (`SequencerViewModel.swift:3546-3554`) | `RegisterJsonSubclass<VUMeterPanel>` (`:159`); `SetPanelStatus` (`VUMeterPanel.h:28-29`) | — | Alt-track (stem) *production* is an audio-tools concern (theme 03); the effect-side selector is at parity. |
| 55 | Warp | ✅ | ✅ `Warp.json` (7) | `RegisterJsonSubclass<WarpPanel>` (`:160`) — `ValidateWindow` only | ISPC + Metal (`:200`) + Vulkan | |
| 56 | Wave | ✅ | ✅ `Wave.json` (9) | `RegisterJsonSubclass<WavePanel>` (`:161`) — `ValidateWindow` only | ISPC + Metal (`:224`) + Vulkan | |

**Acceleration summary.** 23 effects have a GPU compute variant, and the Metal and
Vulkan factories cover an **identical** effect set (Bars, Butterfly, Candle, Circles,
Color Wash, Fan, Fill, Fire, Galaxy, Garlands, Kaleidoscope, Life, Meteors, Pinwheel,
Plasma, Shader, Shimmer, Shockwave, Spirals, Tree, Twinkle, Warp, Wave) —
`MetalEffectManager.mm:196-241` vs `VulkanEffectManager.cpp:211+`. 24 effects have an
ISPC SIMD kernel: the same 22 non-Shader GPU effects plus **Liquid** and **Video**,
which are ISPC-only. Shader is the only GPU-only entry. Every Metal effect subclasses
its CPU counterpart and falls back to the base `Render` when Metal isn't viable, so
the ISPC/scalar path is always the floor.

**iPad can create every effect.** `availableEffectNames`
(`src-iPad/Bridge/XLSequenceDocument.mm:10479-10489`) walks the whole `EffectManager`
with no filtering, and `EffectPaletteView.swift:31` renders that list directly — there
is no iPad allow-list or per-effect suppression anywhere in the palette path.

---

## Cross-cutting effect-panel features

| # | Feature | Desktop evidence | iPad status | iPad evidence / gap |
|---|---|---|---|---|
| 1 | JSON-driven panel engine | `JsonEffectPanel::BuildFromJson` (`JsonEffectPanel.cpp:99`); 55/56 effects registered through it (`EffectPanelManager.cpp:83-95,104-161`) | ✅ | Same `resources/effectmetadata/*.json` parsed into `EffectMetadata` (`src-iPad/Metadata/EffectMetadata.swift:34-132`) and laid out by `EffectMetadataPanel` (`EffectSettingsView.swift:563-646`). iPad's `layoutItems` explicitly reproduces desktop's "insert group at its first member's index" ordering (`EffectSettingsView.swift:568`). |
| 2 | Panel subclass hooks (`SetDefaultParameters` on new effect) | 7 panels override it to seed model/sequence-derived defaults: Faces (`FacesPanel.cpp:154`), State (`StatePanel.cpp:114`), DMX (`DMXPanel.cpp:171`), Servo (`ServoPanel.cpp:337`), Shader (`ShaderPanel.cpp:506`), Video (`VideoPanel.cpp:120`), Pictures (`PicturesPanel.cpp:306`). Invoked from `EffectsPanel::SetDefaultEffectValues` (`EffectsPanel.cpp:127`) | 🟡 | No equivalent hook. `addEffectToRow:` takes whatever settings string the caller passes and calls `EffectLayer::AddEffect` directly (`XLSequenceDocument.mm:12102-12115`); `seedsForNewEffect` returns `settings: ""` whenever the new effect's type differs from the current selection (`SequencerViewModel.swift:7085-7096`). Static defaults still resolve at display/render time from the JSON `default`, so **only the list-derived selections are lost** — Faces' first lyric track and State's state[0]/track[0]. |
| 3 | `custom` control-type rows | `JsonEffectPanel::CreateCustomControl` overridden by 13 panel subclasses | ✅ | All 48 custom ids across the 56 effect JSONs + 3 shared JSONs are dispatched (`EffectPropertyView.swift:164-283`); zero fall through to `customPlaceholder` (`:482`). Verified by diffing the JSON id set against the Swift `case` labels. |
| 4 | Visibility rules (`show`/`hide`) | `JsonEffectPanel.cpp:1685-1705` (parse), `:1736-1769` (evaluate) | ✅ | `isPropertyVisible` (`EffectSettingsView.swift:728-744`) with the same show/hide semantics. |
| 5 | Enable/disable rules (grey out, stay visible) | `JsonEffectPanel.cpp` `setEnabled` path in `ApplyVisibilityRules` | ✅ | `isPropertyEnabledByRules` (`EffectSettingsView.swift:753-767`) feeds `ruleDisabled` → `effectiveDisabled` on every control branch (`EffectPropertyView.swift:79`). |
| 6 | Compound conditions (`allOf`, `any`, `oneOf`, `notOneOf`, `greaterThan`, `startsWith`) | `JsonEffectPanel.cpp:1744-1769` | ✅ | `evaluateCondition` recurses through all six forms (`EffectSettingsView.swift:791-835`). |
| 7 | Runtime disable not expressible in JSON | `BlendingPanel::ValidateWindow` compound fade/type check (`BlendingPanel.cpp:472,515`) | ✅ | Hardcoded mirror for the six transition Adjust/Reverse/Blur rows (`EffectPropertyView.swift:57-97`). |
| 8 | `dynamicLabel` (label driven by a sibling's value) | JSON-level feature consumed by `JsonEffectPanel` | ✅ | `dynamicLabelOverride` (`EffectSettingsView.swift:774-780`) → `displayLabelOverride` (`EffectPropertyView.swift:21`). |
| 9 | `dynamicOptions` sources | 8 sources in the JSONs; desktop resolves them in `JsonEffectPanel.cpp:399,1973,2004-2116` (timingTracks, lyricTimingTracks, effect, states, faces, modelNodeNames) plus `BufferPanel.cpp:171-190` (cameras) and `VUMeterPanel.cpp:77` (audioTracks) | ✅ | All 8 implemented in `SequencerViewModel.dynamicOptions` (`SequencerViewModel.swift:3536-3572`). |
| 10 | Group types: `tabs`, `section`, `xyCenter` | `JsonEffectPanel.cpp:134-152` (tabs/section), `:146,176,315` (xyCenter) | ✅ | `groupView` handles all three (`EffectSettingsView.swift:654-702`) → `NotebookTabsView`, inline section, `XYCenterPadView`. iPad additionally derives an xyCenter pad's heading from the enclosing tab label so Pictures' Start/End Position pads aren't both titled "Position" (`EffectSettingsView.swift:711-721`). |
| 11 | Control types: slider, checkbox/togglebutton, choice/combobox, spin, text, filepicker, fontpicker, point2d | `JsonEffectPanel::BuildPropertyRow` | ✅ | All eight dispatched (`EffectPropertyView.swift:126-155`); `FilepickerPropertyView`, `FontpickerPropertyView`, `Point2DPropertyView` are dedicated views. |
| 12 | Typed numeric entry alongside sliders/steppers | wx sliders pair with a text control | ✅ | `EditableNumberField` on both slider (`EffectPropertyView.swift:330`) and spin (`:438`) rows, clamped to `[min,max]` — needed for ranges like `FreezeEffectAtFrame` (default 999999). |
| 13 | Per-control right-click → bulk edit across selection | `BulkEditControls.cpp:367-516` — `wxEVT_RIGHT_DOWN` on slider, font picker, colour picker, value-curve button, button, text ctrl, file picker (Filename/Path/Show Folder variants), spin, combobox, choice, checkbox (Checked/Unchecked submenu) | ✅ | `propertyContextMenu` (`PropertyContextMenu.swift:102-135`) — "Apply to N Other Selected", plus the checkbox Checked/Unchecked pair and the file-picker "Apply Filename Only" variant, matching desktop's split. Attached to every control branch. |
| 14 | Per-control "Copy Value" / "Paste Value" | Absent — grepped `src-ui-wx/` for `Copy Value`/`Paste Value`: no hits | 🔵 | iPad-only (`PropertyContextMenu.swift:54-82`), including value-curve copy/paste (`:157-176`). |
| 15 | Per-control "Reset to Default" | Absent from the desktop bulk-edit menus (`BulkEditControls.cpp:374-516` offers only Bulk Edit variants) | 🔵 | iPad-only (`PropertyContextMenu.swift:86-91`). |
| 16 | Effect-level "Reset to Defaults" | User entry point is the **grid** layer context menu "Reset Effect" (`EffectsGrid.cpp:556` `ID_GRID_MNU_RESETEFFECT` → `:1123` → `ResetEffect()`); the panel-side seeding it drives is `xLightsFrame::ResetPanelDefaultSettings` (`tabSequencer.cpp:3210-3220`) → `EffectsPanel::SetDefaultEffectValues` (`EffectsPanel.cpp:127`). The same seeding also fires implicitly on every effect-type change (`tabSequencer.cpp:1273`, `:1283`, `:2939`) | ✅ | Inspector overflow menu → `resetSelectedEffectToDefaults()` (`EffectSettingsView.swift:110`). The grid-menu twin is [02-sequencer-grid.md](02-sequencer-grid.md) #81; this row is the panel-side view of the same operation. |
| 17 | Randomize an effect's settings | `EffectsPanel::GetRandomEffectString` / `GetRandomEffectStringFromWindow` (`EffectsPanel.cpp:215,288`) | ✅ | `randomizeSelectedEffects()` walks the metadata and randomizes per control type, preserving palette + shared panels (`SequencerViewModel.swift:4638-4650`). |
| 18 | Fill a selected grid range with random effects | `EffectsGrid::FillRandomEffects` (`EffectsGrid.cpp:1524-1545`), reached from the layer context menu "Create Random Effects" (`:551`, `ID_GRID_MNU_RANDOM_EFFECTS`) — fills every row × timing-mark cell in the selection | ❌ | Grepped all of `src-iPad/` for `fillRandom`, `FillRandom`, `Create Random`: no hits. The adjacent iPad capabilities are separate features, not a partial implementation of this one — settings-randomization is #17 and the palette "random effect" arm button is `EffectPaletteView.swift:10-28`; neither walks a cell range. Matches [02-sequencer-grid.md](02-sequencer-grid.md) #43, which also blocks on the missing cell-range selection (#12 there). |
| 19 | Buffer panel: render styles, buffer type, transformations, rotations/roto-zoom | `BufferPanel` is itself a `JsonEffectPanel` subclass over `shared/Buffer.json` (`BufferPanel.cpp:35-58`) | ✅ | Same JSON drives the iPad Buffer tab (`EffectSettingsView.swift:13,331`); `RotoZoomPresetRowView` covers the roto-zoom custom row (`EffectPropertyView.swift:240`). 20 properties, 3 custom rows, all handled. |
| 20 | Sub-buffer editor | `SubBufferPanel` (`src-ui-wx/sequencer/SubBufferPanel.h:19`) | ✅ | `SubBufferEditorView` (`src-iPad/App/SubBufferEditorView.swift:30`, 679 lines), dispatched for the `SubBuffer` custom row (`EffectPropertyView.swift:238`). |
| 21 | Per-preview camera choice (buffer style) | `BufferPanel.cpp:171-190`, gated by `BufferStyles::CanRenderBufferUseCamera` | ✅ | `dynamicOptions: "cameras"` → `perPreviewCameraNames()` (`SequencerViewModel.swift:3542-3545`). |
| 22 | Layer blending: 24 mix modes | `BlendingPanel` is a `JsonEffectPanel` subclass over `shared/Blending.json` (`BlendingPanel.cpp:101-111`); list at `BlendingPanel.cpp:89-99` | ✅ | `LayerMethodRowView` (`BlendingPanelViews.swift:138`) over `kLayerMethods` (`:24-33`). Diffed programmatically against desktop's `LAYER_METHODS`: **24 vs 24, same members, same order.** See Notes for the duplication risk. |
| 23 | Canvas mode | `CanvasRow` custom row in `shared/Blending.json` | ✅ | `CanvasRowView` (`EffectPropertyView.swift:202`) — checkbox + Layers… button. |
| 24 | Layer morph + effect-layer mix | `LayerMorphRow` in `shared/Blending.json` | ✅ | `LayerMorphRowView` (`EffectPropertyView.swift:198`). |
| 25 | In/out transitions: 22 types + fade time, Adjust, Reverse, Blur | `BlendingPanel::BuildTransitionHeader` (`BlendingPanel.cpp:248-275`); `TRANSITION_TYPES` at `:261`, `TRANSITIONS_NO_ADJUST` at `:70`, `TRANSITIONS_NO_REVERSE` at `:76`, `TRANSITIONS_WITH_BLUR` at `:82` | ✅ | `TransitionHeaderRowView` (`BlendingPanelViews.swift:440`) over `kTransitionTypes` (`:40-45`), `kTransitionsNoAdjust`/`kTransitionsNoReverse`/`kTransitionsWithBlur` (`:51-68`). All four lists diffed programmatically against their desktop counterparts: 22/10/12/10 entries respectively, **identical members in identical order**. |
| 26 | Persistent render flag | `Persistent` checkbox in `shared/Buffer.json:87`; `Effect::IsPersistent()` (`src-core/render/Effect.h:139`) | ✅ | Shared JSON → rendered by the generic checkbox branch on the iPad Buffer tab. |
| 27 | `SuppressEffectUntil` / `FreezeEffectAtFrame` | `shared/Blending.json` spin properties | ✅ | Generic spin branch with typed entry (`EffectPropertyView.swift:421-452`). |
| 28 | Effect icon rendering (grid + palette) | `EffectIconCache::GetEffectIcon` → `wxBitmapBundle` from compiled-in XPMs, keyed by (effect, size) (`src-ui-wx/effectpanels/EffectIconCache.h:19-27`) | ✅ | `EffectIconCache` (`src-iPad/App/EffectIconCache.swift:9-45`) — same 16/24/32/48/64 bucket ladder, pulls BGRA from `iconBGRA(forEffectNamed:desiredSize:outputSize:)` which parses the same desktop XPM data; caches misses so a missing icon isn't re-probed per frame. |
| 29 | Docked "Effect Assist" pane | `xLightsFrame::UpdateEffectAssistWindow` (`xLightsMain.cpp:4001-4021`) auto-shows/hides the pane based on `HasAssistPanel`; 3 effects supply one — Morph (`MorphPanel.h:26`), Pictures (`PicturesPanel.h:40`), Sketch (`SketchPanel.h:51`) | 🟡 | iPad has no assist *pane* concept; the editors are inline settings rows instead. Morph → `MorphLineEditorRowView`, Sketch → `SketchPathEditorRowView` (both `"platform": "ipad"` rows). Pictures has no counterpart (see per-effect row 32). |
| 30 | Shader download browser | `ShaderPanel::OnDownloadClicked` (`ShaderPanel.h:63`) → `src-ui-wx/effects/ShaderDownloadDialog.cpp` | ❌ | Grepped `src-iPad/` for `ShaderDownload`, `shadertoy`, `Shader.*[Dd]ownload`: no hits. `VendorBrowserSheet.swift` exists but is a controller-vendor browser, not a shader source. |
| 31 | Character-map picker for Shape glyphs | `CharMapDialog` glyph grid (`ShapePanel.cpp:243`, `src-ui-wx/effects/CharMapDialog.cpp`) | 🟡 | `ShapeCharRowView` is a code-point stepper + single-glyph preview (`EffectCustomRowsExtra.swift:425-483`); no browsable grid. |
| 32 | Convert selected effects to another type | `EffectsPanel.cpp:344-357` — `wxSingleChoiceDialog` over the effect list → `MainSequencer::ConvertSelectedEffectsTo` | ✅ | `ConvertEffectTypeSheet.swift:9` — searchable list over the same choice set, same operation (a flat 56-item menu is unusable on touch). |
| 33 | JSON `"platform"` gate | `JsonEffectPanel.cpp:897-906` — `"platform": "ipad"` entries are skipped on desktop | ✅ | Symmetric: `PropertyMetadata.isForIPad` (`EffectMetadata.swift:99`) drops `"platform": "desktop"` entries, checked first in `isPropertyVisible` (`EffectSettingsView.swift:732`). Only 3 JSONs use the gate (Morph 1, Sketch 1, MovingHead 16). |
| 34 | Detach a settings tab into its own window | Desktop panes are dockable via wxAUI | ✅ | iPad `DetachedInspectorRoot` scene windows with Stage Manager titles (`EffectSettingsView.swift:386-522`); dock placeholder at `:352`. |
| 35 | Multi-select "Update All" from the anchor effect | Per-control Bulk Edit only (#13) — no single "push every value" action found in `BulkEditControls.cpp` | 🔵 | iPad `updateAllLikeAnchor()` flushes every inspector value from the anchor across the selection in one action (`EffectSettingsView.swift:152`), with "N effects selected / mixed effect types" chrome (`:134-176`). |
| 36 | "Reset panel when changing effects" preference | Three independent wxConfig-backed checkboxes, one per shared panel (`ResetPanelRow` in `shared/{Color,Buffer,Blending}.json`): `xLightsResetColorPanel` (`ColorPanel.cpp:214` read, `:993-996` write), `xLightsResetBufferPanel` (`BufferPanel.cpp:65`, `:155-158`), `xLightsResetBlendingPanel` (`BlendingPanel.cpp:137`, `:364-367`). **Live, not decorative** — `BufferPanel::SetDefaultControls` (`:246-250`) short-circuits the whole reset when the box is unchecked, so the previous effect's panel values carry forward | 🚫 | iPad renders nothing for this row rather than a dead control (`EffectPropertyView.swift:272-280`). Reason: the preference governs *sticky panel state*, which iPad does not have — its inspector re-reads the selected effect's stored settings on every selection change, so there is no carry-forward to suppress. Not a missing feature; a behavioral-model difference. |
| 37 | Per-property tooltip from the JSON `tooltip` field | `JsonEffectPanel.cpp:917` reads `prop.value("tooltip", "")`, then attaches it to the property's primary control (`:1625-1628`) or, for custom rows with no single primary control, to the custom control itself (`:1615-1616`) | ✅ | Same metadata field (`EffectMetadata.swift:36`, decoded `:87`) surfaced as a SwiftUI hover string on every property row — `EffectPropertyView.swift:115-118` applies `.help(tip)` to `propertyBody`, so it covers every control branch uniformly rather than per-type. |
| 38 | `canvasMode` auto-enable (effects that require Canvas layer mode turn it on) | `JsonEffectPanel.cpp:2211-2219` — when the effect metadata carries `"canvasMode": true`, the panel reaches the frame's `BlendingPanel` and force-sets `CheckBox_Canvas`. Three effects declare it: `Adjust.json:3`, `Kaleidoscope.json:3`, `Warp.json:4` (schema `_schema.json:12`) | ✅ | `SequencerViewModel.swift:3288-3298` — on selecting an effect whose metadata has `canvasMode == true`, writes `T_CHECKBOX_Canvas = 1` **only when the key is still absent**, so an explicit user un-check survives. Desktop's version is unconditional, making the iPad behaviour slightly more conservative rather than weaker. |
| 39 | DMX Save As State / Load From State (write the effect's 32 channel values into the model's state definitions, and read them back) | `DMXPanel.cpp:140-144` builds both buttons; `OnSaveAsStateClick` (`:363`) prompts for a name, sanitizes it, offers overwrite on collision, and builds a `SingleNode`/`CustomColors` state attribute map across `DMX_CHANNELS`; `OnLoadFromStateClick` (`:422`) | ✅ | `DMXButtonsRowView` (`EffectCustomRowsFinal.swift:639`) → `dmxSaveState` (`:805`) / `dmxLoadState` (`:773`) → bridge `dmxSaveStateForRow:` (`XLSequenceDocument.h:2892`, impl `.mm:15270`) / `dmxLoadStateForRow:` (`.h:2905`, impl `.mm:15323`). iPad additionally persists immediately — it marks the model dirty and calls `SaveModelStates()` so the state survives a show-folder close, logging a warning if that write fails (`.mm:15313-15320`). |
| 40 | Moving Head bespoke sub-panels (the one panel with no JSON representation) | `MovingHeadPanels/` — `MHColorWheelPanel` / `MHRgbPickerPanel` multi-handle colour authoring (`MHColorWheelPanel.h:93`, `MHRgbPickerPanel.h:82`), `.xmh` user preset scan + save (`MovingHeadPanel.cpp:897`, `:915`, `:1116`) feeding four preset grids via `MHPresetBitmapButton` / `MHPathPresetBitmapButton` / `MHDimmerPresetBitmapButton` / `MHPatternPresetBitmapButton`, and a panel-level "Reset to Default" (`:611` → `:2983`) | 🟡 | iPad reimplements the *common* cases as metadata custom rows — `MovingHeadColorWheelRowView` (single wheel-slot pick, `MovingHeadPresetViews.swift:74-83`), `MovingHeadDimmerPresetStrip` (5 built-ins, `:19`), `MovingHeadPathPresetStrip` (`MovingHeadPathEditorRowView.swift:512`) — and substitutes the native SwiftUI `ColorPicker` for `MHRgbPickerPanel`. Not reached: multi-handle colour authoring, any user-supplied `.xmh`, a Pattern preset strip, and the reset button. **This is the structural cost of the one non-JSON panel** — every gap in this theme's missing/partial column that is *not* about default seeding traces back to a desktop control with no JSON `custom` id, and Moving Head is where that concentrates. |

---

## Desktop platform differences (macOS vs Windows vs Linux)

| Behavior | macOS | Windows | Linux | Evidence |
|---|---|---|---|---|
| GPU compute backend for effects | Metal — `CreateGPUEffect` → `CreateMetalEffect`, 23 effects | Vulkan when `HAVE_VULKAN`, else no GPU acceleration | Vulkan when `HAVE_VULKAN`, else none | `EffectManager.cpp:81-95` (`#ifdef __APPLE__` / `#elif defined(HAVE_VULKAN)` / `#else return nullptr`); `MetalEffectManager.mm:193-247`; `VulkanEffectManager.cpp:211`; `CMakeLists.txt:600-601` |
| Native Shader effect (ISF GLSL) | OpenGL on desktop; Metal via `MetalShaderEffect` + `MetalShaderTranslator` | Vulkan **compute-only** unless the vcpkg glslang path is taken — `HAVE_VULKAN_SHADER` is conditional | Vulkan graphics pipeline + glslang GLSL→SPIR-V (the reference implementation) | `ShaderEffect.cpp:125-624`; `CMakeLists.txt:608-615` ("Windows Vulkan is compute-only until the glslang packaging/link is sorted there") |
| ISPC SIMD targets | `neon-i32x8`/aarch64 on Apple Silicon; `avx2,avx1,sse4.2,sse2`/x86_64 on Intel — chosen per `CURRENT_ARCH` | `avx2,avx1,sse4.2,sse2` x86_64 only (no ARM64 rule) | `neon-i32x8`/aarch64 or the four x86_64 targets, selected in the makefile | `macOS/scripts/ispc_compile:24-31`; `xLights/Xlights.vcxproj:1890-1907`; `build_scripts/linux/ispc.mak:5-7` |
| Video decode | AVFoundation — shared per-file decoder, pts-indexed frame cache, position-independent seeking, memory-budgeted scaled-output cache (`XL_VIDEO_SCALED_CACHE_MB`, default 128MB/file) so a clip reused by several effects at one output size decodes once. Shared with iPad — the bridge compiles for iOS, so the same saving applies there | FFmpeg — forward-decoding reader | FFmpeg — forward-decoding reader | `VideoEffect.cpp:70-78`; `VideoReader.h:45`; `macOS/src-apple-core/media/AVFoundationVideoBridge.mm` (`insertScaled`) |
| Audio decode / output | AudioToolbox decode + AVAudioEngine playback | FFmpeg + SDL2 | FFmpeg + SDL2 | AGENTS.md §2 `media/`; `src-core/media/AudioToolboxDecoder.cpp` |
| Text rasterization | wx `TextDrawingContext` (desktop); CoreGraphics on iOS via a registered factory | wx `TextDrawingContext` | wx `TextDrawingContext` | `src-core/render/TextDrawingContext.h`; `src-iPad/Bridge/CoreGraphicsTextDrawingContext.mm:389-393` |
| Release float semantics | `-ffast-math` at `-O3` + ThinLTO on both desktop and iPad | not set today | not set today | AGENTS.md §6 — affects every effect's `.cpp`; `infinity()` sentinels and `std::isnan` guards must use `::max()` / `xl::isnan` |
| ISPC kernel registration burden | pbxproj membership-exception list per kernel (`macOS/` is a submodule) | `<CustomBuild>` block per kernel in `Xlights.vcxproj` + `.filters` | `OBJ_LINUX_{DEBUG,RELEASE}` pair in `ispc.mak` | AGENTS.md §5; a missing entry fails the x86_64 link only |

**iOS-only core gates** (these are iPad-vs-all-desktop, not macOS-vs-Windows, and are
listed here because they are the render-side platform conditionals encountered in this
theme):

- `xLightsVamp` — the entire VAMP plugin host is excluded from the iOS build
  (`src-core/media/AudioManager.h:22,78,165`), as is
  `DoPolyphonicTranscription` (`:233`). `GetFrameData` (the FFT/spectral path Music and
  VU Meter use) is **not** gated and works on iPad.
- `ShaderEffect`'s entire OpenGL implementation is `#if !TARGET_OS_IPHONE`
  (`ShaderEffect.cpp:125-204,419-624,634-1154`); the iOS branch stubs the GL helpers and
  paints solid red if `MetalShaderEffect` didn't take over (`:1138-1142`).
- `MetalComputeUtilities` loads `EffectComputeFunctions.metallib` from the main bundle
  on both Apple platforms (`MetalComputeUtilities.mm:1304`); the `.metal` and `.ispc`
  membership-exception lists in the Xcode project cover the iPad targets, so **iPad gets
  both the Metal and the NEON-ISPC acceleration paths** that macOS Apple Silicon does.

---

## Notes

- **The per-effect matrix is nearly saturated, and that is the real finding.** Because
  `JsonEffectPanel` and `EffectPropertyView` consume the same
  `resources/effectmetadata/*.json`, adding an effect setting on desktop lands on iPad
  for free. **49 of 56 effects are ✅ on both the render and the settings-UI column, and
  none is ❌.** The 7 partials — Faces, Moving Head, Piano, Pictures, Shader, Shape,
  State — every one of them fails for a reason *outside* the JSON engine (default
  seeding, a compiled-out audio host, a missing assist canvas, a missing download
  browser, a weaker glyph picker, and — for the one hand-built desktop panel — bespoke
  sub-panels with no metadata representation at all). Counted per column: 52/56 ✅ on
  rendering, 50/56 ✅ on settings UI.
  Future desktop effect work should stay inside the JSON schema — anything that goes
  into a `CreateCustomControl` override without a matching JSON `custom` id is where
  parity actually breaks.

- **Suggested porting order.** (1) `SetDefaultParameters` equivalence — one shared
  seeding hook callable from `addEffectToRow:` would close Faces and State together,
  and is the only gap in this theme that silently produces a *wrong-looking* effect
  rather than a missing button. (2) Pictures assist pixel editor — the single largest
  missing surface, and the one desktop assist with no iPad analogue (Morph and Sketch
  already have inline row editors to copy the pattern from). (3) Moving Head's `.xmh`
  user-preset loader — cheap next to the other items (the parser already exists on
  desktop and the iPad Position/Path/Dimmer strips are already built; only the file
  scan is missing, plus a new Pattern strip to match desktop's now-four preset grids)
  and it unblocks every preset a user has already authored. (4) Shader download browser.
  (5) Shape character-map grid. (6) Range-fill random effects — blocked on the grid
  cell-range selection first (02 #12). Deliberately last: Moving Head multi-handle
  colour authoring, which needs a new touch canvas for a feature single-slot picking
  mostly covers.

- **Three hardcoded copies of the layer-method list.** `MixTypesMap` in
  `src-core/render/PixelBuffer.cpp` (the render-side truth, exposed as
  `PixelBufferClass::GetMixTypes()` at `:1125`), `LAYER_METHODS` in
  `BlendingPanel.cpp:89`, and `kLayerMethods` in `BlendingPanelViews.swift:24`. They
  agree today (verified member-for-member and in order), but nothing enforces it — the
  desktop panel does not call `GetMixTypes()`. The four transition lists are duplicated
  the same way (`BlendingPanel.cpp:70,76,82,261` vs `BlendingPanelViews.swift:40,51,60,68`,
  with the Swift comments citing the desktop lines as their source) — all four also
  currently agree exactly. Moving both sets of lists into
  `src-core` and exposing them through the metadata JSON's `options` array (or a bridge
  call) would remove a whole class of silent drift.

- **The `"platform"` gate is a real, symmetric mechanism — use it.** Both engines honor
  it (`JsonEffectPanel.cpp:897-906` skips `"ipad"`, `EffectMetadata.swift:99` skips
  `"desktop"`), and Moving Head shows the pattern working at scale: 9 desktop-only
  serialization sinks and 7 iPad-only touch editors over one shared settings-key
  vocabulary. This is the right tool for a genuinely platform-shaped control, and it
  keeps the divergence visible in a data file rather than buried in a subclass.

- **Metal and Vulkan cover an identical 23-effect set.** That symmetry is worth
  preserving deliberately: a new GPU kernel added to only one backend creates a
  performance cliff that is invisible in correctness testing, since both fall back to
  the same ISPC/scalar `Render`. Liquid and Video are ISPC-only on every platform, and
  Shader is GPU-only — those three are the standing exceptions.

- **Two audio-adjacent effects have a production-vs-consumption split.** Piano
  (note timing tracks) and VU Meter (alt/stem audio tracks) render and configure fine on
  iPad, but the artifacts they consume are produced by audio tooling that is partly
  compiled out on iOS. Their scorecard rows belong here; the *generation* side is
  theme 03's, and the two themes should not both claim the fix.
