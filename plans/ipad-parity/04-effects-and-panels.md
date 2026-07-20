# 04 · Effects & Effect Setting Panels

> Effect *authoring* on iPad is in very strong shape. Both platforms are
> **JSON-metadata driven**: the desktop builds every effect panel from
> `resources/effectmetadata/*.json` via `JsonEffectPanel` (36 effects use it
> directly, 20 via `RegisterJsonSubclass` for special rows, only Moving Head is
> fully custom — `EffectPanelManager.cpp:97-160`), and the iPad walks the same
> JSON in `EffectMetadataPanel` (defined inside `EffectSettingsView.swift:553`).
> Core render logic is shared, so visual output parity is automatic. The iPad
> dispatcher (`EffectPropertyView.swift:105-258`) renders the standard control
> types **and** ~45 bespoke custom rows (filename blocks, transition headers,
> palette/chroma/sparkle color rows, SubBuffer/RotoZoom editors, Servo/Shape/
> Faces/State/DMX/MovingHead rows, Sketch/Morph editors). The real gaps are a
> small set of *specialized editing canvases* that need a touch redesign
> (**Pictures paint tools** remain iPad-missing; the **Moving Head path canvas**
> is now at parity — a touch Bezier waypoint editor plus path/dimmer presets and
> a colour-wheel picker landed; **Sketch** is also at parity — close-path, SVG
> import, path reorder, and curve-segment authoring all landed). The **DMX
> arbitrary channel remap** now has a touch grid (`DMXRemapGridSheet`) matching
> the desktop `RemapDMXChannelsDialog`'s per-channel mapping, **resize-on-add**
> for Pictures images landed (`PicturesResizeOnAddSheet`), and a per-effect
> **Reset to Defaults** action now sits in the inspector header menu. The
> **Effect Wheel** is declined as a touch-idiom mismatch (it's a keybinding
> accelerator and iPad has no keybinding store; palette-tap-then-tap-cell is the
> fuller touch equivalent). Going the other way, the iPad's
> uniform per-property **Copy / Paste Value + Reset to Default** context menu has
> *no desktop equivalent* (desktop right-click only offers "Bulk Edit").

## Parity scorecard

| Feature | Surface | Desktop | iPad | Gap | Priority | Ease | Feasibility | Notes |
|---|---|---|---|---|---|---|---|---|
| JSON-metadata-driven panel build | panel | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `JsonEffectPanel::BuildFromJson` (JsonEffectPanel.cpp:99); iPad `EffectMetadataPanel` in EffectSettingsView.swift:553. |
| All ~56 effects have metadata | panel | ✅ | ✅ | parity | P1 | easy | feasible | Music has MusicEffect.json. Every RGB effect has a json; Guitar/Duplicate/Wave render generically on both. |
| Integer slider + live value field | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad sliderView (EffectPropertyView.swift:263) has EditableNumberField for precise typed entry + clamp. |
| Float slider w/ divisor | panel | ✅ | ✅ | parity | P1 | easy | feasible | `effectiveDivisor` on PropertyMetadata mirrors desktop TEXTCTRL-prefix logic. |
| Checkbox / toggle | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad `.toggleStyle(.switch)`; bulk Set Checked/Unchecked in context menu. |
| Choice / combobox | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad Picker(.menu). Dynamic option sources (states, timing tracks) on both. |
| Spin control | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad renders as editable numeric field (no up/down chevrons but functionally equal). |
| Text entry | panel | ✅ | ✅ | parity | P1 | easy | feasible | textView. |
| File picker | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad FilepickerPropertyView (system browser, `fileFilter`). Only 2 effects use `filepicker` controlType; media effects use custom filename blocks. |
| Font picker | panel | ✅ | ✅ | parity | P2 | medium | feasible | iPad FontpickerPropertyView → FontPickerSheet (UIFont.familyNames + symbolic traits). Used by Text.json. |
| Color picker (sRGB) | panel | ✅ | ✅ | parity | P2 | medium | feasible | iPad uses native SwiftUI `ColorPicker` in ColorPanelCustomRows (Chroma/Sparkles), MovingHead, ColorReplace. No effect uses the `colourpicker` controlType (schema-only); colors flow through custom rows + palette. |
| XY Center pad (group) | panel | ✅ | ✅ | parity | P2 | medium | feasible | 5 effects use the `xyCenter` group (Shape/Pictures/Marquee/Pinwheel/Text); iPad XYCenterPadView is an interactive draggable grid. `point2d` controlType is schema-only/unused. |
| Value Curve editor (23 types) | dialog | ✅ | ✅ | parity | P1 | easy | feasible | iPad ValueCurveEditor.swift; types sourced from core via XLValueCurve `availableTypes` (XLValueCurve.mm:190). Serialise/Deserialise route through core `ValueCurve`. Custom-point canvas + P1-P4 sliders. |
| Value Curve reverse / flip | dialog | ✅ | ✅ | parity | P2 | easy | feasible | iPad XLValueCurve `reverse`/`flip` → core `Reverse()`/`Flip()`. |
| Value Curve presets (.xvc load/save) | dialog | ✅ | ✅ | parity | P2 | easy | feasible | iPad ValueCurvePresetSheets.swift (load/save + thumbnails); desktop ValueCurveDialog PopulatePresets/Export. |
| Value Curve clipboard (copy/paste VC) | context-menu | ✅ | ✅ | parity | P2 | easy | feasible | iPad PropertyContextMenu Copy/Paste Value Curve via ValueCurveClipboard; desktop drag-copy + dialog. |
| Per-property Copy Value | context-menu | ❌ | ✅ | desktop-missing | P2 | medium | feasible | iPad PropertyContextMenu.swift:57. Desktop right-click only offers "Bulk Edit" — no per-property value clipboard. |
| Per-property Paste Value | context-menu | ❌ | ✅ | desktop-missing | P2 | medium | feasible | iPad PropertyContextMenu.swift:82 (prefix-guarded pasteboard). No desktop equivalent. |
| Per-property Reset to Default | context-menu | ❌ | ✅ | desktop-missing | P2 | medium | feasible | iPad PropertyContextMenu.swift:91 (writes metadata default + suppressIfDefault). No desktop equivalent. |
| Bulk Edit (apply value to all selected) | context-menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `BulkEditSlider::OnRightDown` → "Bulk Edit" dialog (BulkEditControls.cpp). iPad "Apply to N Other Selected" (PropertyContextMenu.swift:106). Uniform on both. |
| Multi-select Set Checked / Unchecked | context-menu | 🟡 | ✅ | parity | P2 | easy | feasible | Desktop BulkEditCheckBox bulk-edit dialog; iPad explicit Set Checked/Unchecked on N selected (PropertyContextMenu.swift:116). |
| Apply Filename Only to selected | context-menu | ❌ | ✅ | desktop-missing | P3 | easy | feasible | iPad-only nicety (PropertyContextMenu.swift:134) — apply just the filename portion to other selected media effects. |
| Visibility rules (JSON enable/disable/hide) | panel | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `ApplyVisibilityRules` (JsonEffectPanel.cpp:1779); iPad evaluateCondition/isPropertyVisible/isPropertyEnabledByRules (EffectSettingsView.swift:718-787, incl. allOf nesting). |
| Dynamic property labels | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad dynamicLabelOverride (EffectSettingsView.swift:764) + PropertyMetadata.dynamicLabel; desktop custom SetLabel (e.g. AdjustPanel). |
| Grouped: Tabs (notebook) | tab | ✅ | ✅ | parity | P1 | easy | feasible | 12 effects use `tabs`. iPad NotebookTabsView (segmented Picker); desktop wxNotebook. |
| Grouped: Sections | panel | ✅ | ✅ | parity | P2 | easy | feasible | 14 sections across effects. iPad VStack w/ header; desktop wxStaticBox. |
| Grouped: XY Center | panel | ✅ | ✅ | parity | P2 | medium | feasible | See XY pad row above. |
| Scrollable panel overflow | panel | ✅ | ✅ | parity | P1 | easy | feasible | Desktop wxScrolledWindow; iPad ScrollView. |
| Tooltips on property labels | preference | ✅ | ✅ | parity | P3 | easy | feasible | Desktop SetToolTip (JsonEffectPanel.cpp:1602/1614). iPad renders metadata `tooltip` as a `.help()` hover string on every property row (EffectPropertyView.swift body). |
| canvasMode auto-enable | preference | ✅ | ✅ | parity | P3 | easy | feasible | Adjust/Kaleidoscope/Warp have `canvasMode:true`. Desktop auto-checks Canvas (JsonEffectPanel.cpp:2187). iPad: `selectEffect` writes `T_CHECKBOX_Canvas=1` when the metadata flags canvasMode and the setting is still absent (explicit user un-check respected). |
| Canvas / Layers composite control | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad CanvasRow (BlendingPanelViews.swift:241) — Canvas checkbox + Layers… picker. |
| Layer Morph (cross-fade Effect 1↔2) | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad LayerMorphRow (BlendingPanelViews.swift:75). |
| Layer Method (24 blend modes) | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad LayerMethodRow + LayerMethodHelpSheet (BlendingPanelViews.swift:130). |
| Transition type (In/Out, 22 types) | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad TransitionHeaderRow + shared/Blending.json. |
| Transition fade time presets + manual | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad kFadePresets matches desktop Fadein/Fadeout. |
| Transition Adjust disable rules | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad transitionAdjustDisabled (EffectPropertyView.swift:59) vs desktop kTransitionsNoAdjust ValidateWindow. |
| Transition Reverse disable rules | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad transitionReverseDisabled (EffectPropertyView.swift:63) vs desktop kTransitionsNoReverse. |
| Transition Blur slider (In/Out) | panel | ✅ | ✅ | parity | P2 | easy | feasible | Desktop #6523 In/Out_Transition_Blur sliders, greyed unless the type is in `TRANSITIONS_WITH_BLUR` (BlendingPanel.cpp:82, 543/549). iPad renders the sliders generically and gates them via transitionBlurDisabled (EffectPropertyView.swift:88) against kTransitionsWithBlur (BlendingPanelViews.swift:69), with the same fade==0 guard. |
| Buffer: Render style choice | panel | ✅ | ✅ | parity | P2 | easy | feasible | shared/Buffer.json. |
| Buffer: Roto-Zoom sliders | panel | ✅ | ✅ | parity | P2 | easy | feasible | shared/Buffer.json, standard sliders + VC. |
| Buffer: Roto-Zoom preset menu | menu | ✅ | ✅ | parity | P1 | easy | feasible | iPad RotoZoomPresetRowView (same reset values + VC shapes as BufferPanel::OnPresetSelect). |
| Buffer: SubBuffer editor | dialog | ✅ | ✅ | parity | P1 | medium | feasible | iPad SubBufferEditorView (interactive 2D range grid) vs desktop spin controls. |
| Color: Palette header + swatches | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad ColorPaletteView / PaletteHeaderRow. |
| Color: Chroma key row | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad ChromaKeyRowView (ColorPanelCustomRows.swift:24): enable + sensitivity slider + sRGB ColorPicker. |
| Color: Sparkles row | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad SparklesRowView (ColorPanelCustomRows.swift:117): frequency slider + VC + Music toggle + sRGB color. |
| Color: Brightness Level checkbox | panel | ✅ | ✅ | parity | P3 | easy | feasible | iPad BrightnessLevelRowView (legacy toggle). |
| Adjust: Mode enum | panel | ✅ | ✅ | parity | P2 | easy | feasible | Adjust.json standard choice. |
| DMX: Channel notebook (per-page channels) | panel | ✅ | ✅ | parity | P2 | medium | feasible | iPad DMXChannelsNotebookView (EffectCustomRowsFinal.swift:510). |
| DMX: Remap channels | dialog | ✅ | ✅ | parity | P3 | medium | feasible | iPad ships the preset menu (Shift +1/-1, Reverse, Invert All, Double, Half) **plus** a "Custom Mapping…" 48-row touch grid (`DMXRemapGridSheet.swift`) — each target channel gets a source-channel stepper with a live preview column, committed via the new `dmxRemapChannels(forRow:atIndex:mapping:)` bridge (snapshot-then-write so the permutation can't self-collide; source 0 → off). Mirrors desktop `RemapDMXChannelsDialog`'s arbitrary per-channel mapping. Bridge also adds `dmxChannelValues(forRow:atIndex:)` to seed the grid. |
| DMX: Save/Load State | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad DMXButtonsRowView → `dmxSaveStateForRow`/`dmxLoadStateForRow` (mirror DMXPanel Save/Load). |
| Faces: phoneme vs timing-track selector | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad FacesMouthMovementsRowView (segmented + phoneme/track dropdown). (Face *definition* authoring is a model-level feature, not the effect panel.) |
| Faces: sequence-level definitions in dropdown + render | panel/render | ✅ | ✅ | parity | P2 | easy | feasible | 2026-07: Matrix face defs can now live in the `.xsq` (`SequenceFaces` on `SequenceElements`, shared core). Resolution (model wins, sequence fallback) is in `FacesEffect::RenderFaces` — auto-shared. Dropdown lists them on both platforms: desktop `JsonEffectPanel::SetPanelStatus` merge; iPad `facesForRow:atIndex:` (`XLSequenceDocument.mm`) merge. Editor UI is desktop-only — tracked in 01-file-lifecycle "Settings — Faces tab". |
| Faces/State on SubModel: node-index translation | render | ✅ | ✅ | parity | P2 | n/a | feasible | Core render change (#6110/#6337, 2026.10): `FacesEffect.cpp:891` and `StateEffect.cpp:181` `toLocalNode` lambda maps parent node indices to submodel-local via `SubModel::GetNodeIndexMap()`, lighting only nodes that belong to the submodel. Pure `src-core/effects/` — auto-shared with iPad, no UI work. |
| Morph: quick-set preset menu | menu | ✅ | ✅ | parity | P2 | easy | feasible | iPad MorphQuickSetRowView (8 presets). |
| Morph: line geometry editor | panel | ✅ | ✅ | parity | P1 | medium | feasible | iPad MorphLineEditorRowView (touch canvas, 4 colored draggable handles, coordinate readout) ≈ desktop xlGridCanvasMorph. |
| Morph: swap start/end | menu | ✅ | ✅ | parity | P2 | easy | feasible | iPad MorphSwapRowView. |
| Moving Head: fixture + color/dimmer rows | panel | ✅ | ✅ | parity | P2 | medium | feasible | iPad MovingHead{Fixture,Color,Dimmer}RowView: fixture select, pan/tilt/offset/groupings/cycles, single-color, dimmer intensity. |
| Moving Head: path canvas (waypoint drawing) | panel | ✅ | ✅ | parity | P2 | hard | feasible | Desktop MovingHeadCanvasPanel.cpp / SketchCanvasPanel.cpp draws Bezier waypoint paths; iPad MovingHeadPathEditorRowView (MovingHeadPathEditorRowView.swift) is a full touch Canvas — tap-to-add line/quadratic/cubic waypoints, drag endpoints + control handles, close/open paths, undo-point, multi-path, presets. Reuses SketchDefinition (identical L/Q/C/c grammar) and Y-flips coords (canvas top = stored Y≈1) so it round-trips the same `Path:` command the renderer reads (MovingHeadEffect.cpp:396 `(0.5 - pty)`). Writes via the existing movingHeadCommand/setMovingHeadCommand bridge. |
| Moving Head: quick-set preset buttons + color-wheel / RGB picker | panel | ✅ | 🟡 | ipad-weaker | P3 | medium | feasible | iPad now has path presets (MovingHeadPathPresetStrip — the two shipping `.xmh` presets Circle/Diamond + a Sweep), dimmer presets (MovingHeadDimmerPresetStrip — On/Off/FadeIn/FadeOut/Pulse) and a colour-wheel picker (MovingHeadColorWheelRowView, MovingHeadPresetViews.swift) that surfaces the target model's wheel slots via the new `movingHeadWheelColors(forRow:atIndex:)` bridge (XLSequenceDocument.mm) and writes a single-colour `Wheel:` command. The **RGB-picker portion is at parity by substitution** — desktop's bespoke MHRgbPickerPanel HSV canvas is replaced by the native SwiftUI `ColorPicker` (system colour wheel + sliders + recents), which is the standard touch colour-picking idiom and carries no functional deficit for single-colour selection. **Genuinely deferred (kept 🟡):** the f9df1b052 double-selector multi-colour wheel *animation* across the effect (iPad does single-slot wheel picks only), and user-saved `.xmh` dimmer presets (built-ins only). |
| Effect Wheel (radial keybinding quick-drop) | gesture | ✅ | ➖ | declined | P3 | medium | infeasible-idiom | Desktop #6486: double-click empty non-timing cell → EffectWheelDialog drops 1 of ≤18 sequence-scoped EFFECT keybindings via EffectsGrid::DropEffectAt (skips timing rows, #6491). **Declined as a touch-idiom mismatch:** the wheel is a keybinding accelerator (it surfaces the user's bound EFFECT keys), and iPad has no per-user keybinding system (see 02-sequencer-grid-editing.md, 11-preferences-settings.md), so there are no bindings to radial-launch. iPad's palette-tap-then-tap-cell flow (`EffectPaletteView.swift:32` → `addEffectFromPaletteTap`) already gives a single-tap quick-drop with the *full* effect list, which is strictly more capable than an ≤18-slot wheel on a touch surface. Forcing a radial popover would duplicate the palette with fewer effects and no underlying binding store to populate it. |
| Piano: note/octave + labels | panel | ✅ | ✅ | parity | P2 | easy | feasible | Piano.json standard controls. |
| Pictures: filename block + transparent-black | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad EffectFilenameBlockView + TransparentBlackRowView. |
| Pictures: AI image generate | dialog | ✅ | ✅ | parity | P2 | medium | feasible | Desktop PicturesPanel "AI Generate…" (PicturesPanel.cpp:137) ↔ iPad AIImageGenerationSheet (from filename block "AI…"). |
| Pictures: paint/assist canvas (brush/palette) | panel | ✅ | ❌ | ipad-missing | P3 | hard | hard | Desktop PicturesAssistPanel + xlGridCanvasPictures (pencil/eraser/eyedropper, palette load/save, new/open image). No touch bitmap editor on iPad. Workaround: edit externally / on desktop. |
| Servo: start/end slider pair | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad ServoStartEndRowView. |
| Servo: increment/decrement buttons | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad ServoButtonRowView. |
| Shader: file picker (.fs) | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad Shader_FilenameBlock (EffectFilenameBlockView). |
| Shader: native Metal render path (GLSL→SPIR-V→MSL, no OpenGL/ANGLE) | render | ✅ | ✅ | parity | P1 | done | feasible | Shared src-core (MetalShaderEffect + MetalShaderTranslator); default ON both platforms, XL_NO_NATIVE_SHADER=1 falls back to GL/ANGLE. Validated: 300-shader corpus renders + iOS-MSL compiles 300/300; on-device smoke test recommended before App Store push. Core-only (2026-07), auto-applied: `SPIRVShaderEffect::Render` now also gates on `GPURenderUtils::IsEnabled()` so the desktop "GPU rendering" preference keeps the effect off the GPU entirely. No iPad impact — the iPad never calls `SetEnabled`, and a MetalShaderEffect only exists when `computeEnabled()` was already true, so the flag stays true and the iPad never reaches the red-fill `ShaderEffect::Render` stub it has no GL fallback for. |
| Shader: dynamic params from uniforms | panel | ✅ | ✅ | parity | P2 | medium | feasible | iPad ShaderDynamicParamsView → bridge `shaderDynamicPropertiesJsonForPath` (XLSequenceDocument.h:1551) → renders via full EffectPropertyView (so sliders/choices/checkboxes/point2d **with VC buttons**). |
| Shader: time speed slider + VC | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad ShaderSpeedRowView (synthetic PropertyMetadata → slider + VC). |
| Shape: font + emoji category | panel | ✅ | ✅ | parity | P2 | medium | feasible | iPad ShapeFontRowView. |
| Shape: character / emoji picker | panel | ✅ | ✅ | parity | P2 | medium | feasible | iPad ShapeCharRowView. |
| Shape: skin-tone modifier | panel | ✅ | ✅ | parity | P3 | easy | feasible | iPad ShapeSkinToneRowView. |
| Shape: SVG path field | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad ShapeSVGRowView (+ preview). |
| Sketch: path editor canvas | panel | ✅ | ✅ | parity | P1 | medium | feasible | iPad `SketchPathEditorRowView.swift` now matches desktop's authoring set: drag endpoints/control points, **Add Line / Add Curve** modes (cubic with draggable handles), **Close/Open** path (`cN` token), **▲/▼ reorder** (selected-path model), **Import SVG** (shared core `SketchDefFromSVGFile` → nanosvg). Import/Export of the raw def remains via SketchDefRowView (copy/paste). Desktop-only leftovers: continue-from-arbitrary-endpoint and .sketch file Export. |
| Sketch: path management (#5871) | panel | ✅ | ✅ | parity | P2 | medium | feasible | Desktop #5871: per-path list, multi-select, Delete key, Move Up/Down, per-path description labels + description editor (SketchAssistPanel.cpp:111-156). iPad SketchPathEditorRowView: tap-select path list (`Path N - desc` labels), per-row + selected-path Delete (button + hardware ⌫ via `keyboardShortcut(.delete)`), Move Up/Down, description text field. Descriptions round-trip the `D`+hex token (SketchDefinition.swift encodeHex/decodeHex mirrors SketchEffectDrawing). Multi-select deferred (single-select list; touch idiom). |
| Sketch: background image + opacity | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad SketchBackgroundRowView. |
| Sketch: info block (counters) | panel | ✅ | ✅ | parity | P3 | easy | feasible | iPad SketchInfoRowView. |
| Sketch: raw SketchDef field | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad SketchDefRowView (accepts desktop-generated defs incl. curves). |
| Single Strand: timing-track chase cycles (#4771) | panel+render | ✅ | ✅ | parity | P2 | n/a | feasible | `SingleStrand_TimingTrack` choice added to `resources/effectmetadata/SingleStrand.json` (shared) — UI auto-appears on iPad via metadata-driven panel. Render logic in `src-core/effects/SingleStrandEffect.cpp` (`GetTimingEvent`, `RenameTimingTrack`) — auto-shared. `GetSequenceElements(buffer)` routes through `iPadRenderContext::GetSequenceElements()` so timing lookups work on iPad. No bridge or SwiftUI work needed. |
| State: state vs timing-track selector | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad StateStateSourceRowView. |
| Text: font picker (XL font) | panel | ✅ | ✅ | parity | P2 | medium | feasible | iPad TextFontXLRowView. |
| Text: load text from file | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad Text_File_Row. |
| Video: filename + transparent-black | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad Video_FilenameBlock + TransparentBlackRowView. |
| Video: duration / metadata readout | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad VideoDurationRowView. |
| VUMeter: mode choice | panel | ✅ | ✅ | parity | P2 | easy | feasible | VUMeter.json standard choice. |
| Effect Assist (separate window concept) | panel | ✅ | 🟡 | ipad-missing | P2 | hard | feasible | Desktop floating Assist window for Sketch/Morph/Pictures (EffectPanelManager HasAssistPanel — only these 3). iPad inlines Sketch+Morph editors in the inspector; Pictures painting absent; no detached-window concept. |
| Preference: reset Color/Buffer/Blending panel on effect change | preference | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop wxConfig xLightsReset{Color,Buffer,Blending}Panel. iPad ResetPanelRow renders EmptyView by design (inspector always shows anchor values; nothing to govern). (Distinct from the on-demand **Reset to Defaults** action below.) |
| Reset panel to defaults (per-effect) | menu | ✅ | ✅ | parity | P3 | easy | feasible | Desktop right-click → "Reset panel" clears an effect panel's controls to defaults. iPad: inspector header overflow (`ellipsis.circle`) menu → "Reset to Defaults" → `SequencerViewModel.resetSelectedEffectToDefaults()` (`SequencerViewModel.swift`) clears the whole effect settings string (E_/B_/T_) and resets the palette to a two-colour default via the existing `replaceEffectSettings(_:palette:inRow:atIndex:)` bridge, in one undoable step (captures the prior settings + palette strings for Cmd+Z via `restoreEffectSettingsString`). |
| Convert selected effects to a different type | context-menu | ✅ | ❌ | ipad-missing | P2 | hard | feasible | Desktop converts a multi-selection of effects to another effect type; no iPad equivalent. |
| Resize-on-add image dialog | dialog | ✅ | ✅ | parity | P3 | easy | feasible | Desktop ResizeImageDialog (src-ui-wx/media/ResizeImageDialog.cpp) prompts to resize an image when added to a Pictures effect. iPad: after a Pictures image is browsed in (`EffectFilenameBlockView.commitPicked`, Images subdir only), `PicturesResizeOnAddSheet.swift` offers Width/Height steppers seeded from the image's pixel size (optional keep-aspect), writes a resized `<name>_WxH.<ext>` copy via UIGraphicsImageRenderer alongside the original, and repoints the effect filename at it. "Skip" leaves the original untouched. |
| Effects toolbar: customizable order/visibility | preference | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop Preferences > Toolbars (`ToolbarsSettingsPanel.cpp`, new) lets the user hide/reorder entries in the always-visible Effects toolbar via a `wxRearrangeCtrl`, persisted through `preferences/ToolbarLayout.h` and applied live via `xLightsFrame::RebuildEffectsToolbar()`. Right-clicking the toolbar itself (`xLightsFrame::OnEffectsToolBarContextMenu`) jumps straight to that page, and a "Reset to Defaults" button restores every effect, visible, in `EffectManager` registration order. iPad's equivalent picker, `EffectPaletteView.swift`, renders `SequencerViewModel.availableEffects` as a raw, unfilterable, unordered pass-through of `XLSequenceDocument.availableEffectNames()` (`XLSequenceDocument.mm:9982`) — no persisted order/visibility, no equivalent entry point, and no reset exists there yet. |

## iPad gaps (desktop has, iPad missing)

### P1
- **(LANDED) Sketch close-path / SVG-import / path-reorder / curves.** iPad
  `SketchPathEditorRowView.swift` now mirrors desktop `SketchAssistPanel`'s
  authoring set: an Add-Line / Add-Curve mode picker (cubic segments seed with
  draggable control handles), a **Close/Open** toggle emitting the desktop `cN`
  close token, **▲/▼** path reorder (a selected-path model — tap any handle to
  select that path), and **Import SVG** via a document picker. The SVG→sketch
  conversion was extracted from the wx panel into wx-free core
  `src-core/effects/SketchSVGImport.{h,cpp}` (`SketchDefFromSVGFile`, nanosvg-
  based, collinear→line / cp2==end→quad collapse identical to desktop) and the
  desktop panel was refactored to call it, so both clients share one conversion.
  Bridged to Swift via `XLSequenceDocument.sketchDef(fromSVGFile:)`. Import/Export
  of the raw def text stays on the existing `SketchDefRowView` (copy/paste).
  Per-path descriptions (the `D…` token) are now editable on iPad too (#5871
  path list + description field). Remaining desktop-only: continue-from-
  arbitrary-endpoint and .sketch file Export (P3).

### P2
- **Transition Blur enable gate.** Desktop #6523 added In/Out Transition Blur
  (shared Blending.json sliders + core PixelBuffer render — the *rendering* is
  already shared and correct on iPad, and the sliders auto-appear via the
  generic metadata panel). The only iPad delta: desktop greys the slider out
  for transitions that ignore blur (`TRANSITIONS_WITH_BLUR`,
  BlendingPanel.cpp:82); iPad needs a matching `transitionBlurDisabled(isIn:)`
  rule in `EffectPropertyView.swift` beside the existing adjust/reverse gates.
  Easy.
- **Moving Head waypoint path canvas. (Done.)** Desktop
  `MovingHeadCanvasPanel.cpp` / `SketchCanvasPanel.cpp` draws Bezier pan/tilt
  paths; iPad now has `MovingHeadPathEditorRowView`
  (`MovingHeadPathEditorRowView.swift`) — a full touch Canvas (tap-to-add
  line/quadratic/cubic waypoints, drag endpoints + control handles, close/open,
  undo-point, multi-path, path presets). It reuses `SketchDefinition` (the
  `Path:` command shares the Sketch effect's identical `L`/`Q`/`C`/`c` grammar)
  and flips Y so the canvas top maps to stored Y≈1, matching desktop's canvas
  and the renderer's `(0.5 - pty)` up-axis mapping
  (`MovingHeadEffect.cpp:396`). Writes the same `Path:` command desktop reads via
  the existing `movingHeadCommand`/`setMovingHeadCommand` bridge — no new bridge
  was needed. The data model was already shared, so this was purely UI.
- **Effect Assist as a first-class surface.** Only Sketch+Morph editors are
  inlined; Pictures has none. **Work:** see Pictures paint tools (P3) and Sketch
  (P1); no separate "window" needed on iPad — keep inlining in the inspector.
- **Convert selected effects to a different type.** Desktop converts a
  multi-selection of effects to another effect type; iPad has no equivalent.
  **Work:** a selection-aware command that rewrites each selected effect's type
  (and migrates settings where sensible). Hard.

### P3
- ~~**DMX arbitrary channel remap.**~~ — **landed.** `DMXRemapGridSheet.swift`
  is a 48-row touch grid (per-target source-channel stepper + live preview)
  reached from the DMX panel's Remap menu ("Custom Mapping…"). Commits through
  the new `dmxRemapChannels(forRow:atIndex:mapping:)` bridge variant (explicit
  mapping, snapshot-then-write); `dmxChannelValues(forRow:atIndex:)` seeds it.
- ~~**Tooltips on property labels**~~ — **landed 2026-06-11** (`.help()` on every property row).
- ~~**canvasMode auto-enable**~~ — **landed 2026-06-11** (auto-set on selection when absent).
- ~~**Reset panel to defaults (per-effect).**~~ — **landed.** Inspector header
  overflow menu → "Reset to Defaults" clears the effect settings string + resets
  the palette to defaults in one undoable step
  (`SequencerViewModel.resetSelectedEffectToDefaults`).
- **Pictures paint tools.** Desktop pixel editor with brushes + palette mgmt.
  Hard touch redesign; low priority (most users edit images externally).
- ~~**Resize-on-add image dialog.**~~ — **landed.** After a Pictures image is
  browsed in, `PicturesResizeOnAddSheet.swift` offers Width/Height steppers
  (seeded from the image's pixels, optional keep-aspect), writes a resized copy
  alongside the original, and repoints the effect filename. "Skip" keeps the
  original.
- **Reset-panel-on-effect-change preference.** Intentionally a no-op on iPad
  (inspector shows anchor values). Only relevant if iPad ever adopts sticky
  panel state. Low priority.
- **Effects toolbar customization.** Desktop Preferences > Toolbars lets users
  hide/reorder Effects-toolbar entries (`wxRearrangeCtrl`-based, persisted per
  user), reachable both from the Preferences list and via right-click on the
  toolbar itself, with a one-click reset back to the default (all visible,
  `EffectManager` order). iPad's `EffectPaletteView` shows every effect,
  unordered, with no hide/reorder/reset. A SwiftUI reorder/hide UI over
  `availableEffects` would close this; not attempted here since the desktop
  change was scoped narrowly (a Preferences-only feature, no plugin/DLL
  loading, per maintainer feedback on PR #6685).

## Desktop gaps (iPad has, desktop missing)

### P2
- **Per-property Copy Value / Paste Value / Reset to Default.** iPad
  PropertyContextMenu.swift gives every property a uniform long-press menu
  (Copy Value :57, Paste Value :82, Reset to Default :91). Desktop right-click on
  a control only offers "Bulk Edit" (BulkEditSlider::OnRightDown,
  BulkEditControls.cpp) — there is no per-property value clipboard or
  reset-to-default. **Work:** add Copy/Paste/Reset entries to the BulkEdit\*
  context menus (shared core defaults already exist via effect metadata).
  Medium. Genuine convenience gap users would notice moving iPad→desktop.

### P3
- **Apply Filename Only to selected.** iPad PropertyContextMenu.swift:134 — apply
  just the filename to other selected media effects. No desktop equivalent.

## Infeasible / restricted on iPad

- **Effect Wheel (radial keybinding quick-drop)** — *declined as a touch-idiom
  mismatch* (not platform-blocked). The wheel is a keybinding accelerator that
  surfaces the user's bound EFFECT keys; iPad has no per-user keybinding store
  (see 02 / 11), so there are no bindings to launch. The existing
  palette-tap-then-tap-cell flow is a single-tap quick-drop over the *full*
  effect list — strictly more capable than an ≤18-slot wheel on a touch surface.
- *Otherwise none platform-blocked in this theme.* Every other gap above is feasible — the core
  effect/render/value-curve logic is shared (`src-core/`), so all remaining work
  is SwiftUI + thin bridge additions. The harder ones (Moving Head path canvas
  — now done — and Pictures paint tools) are "hard" only because they need new
  interactive touch canvases, not because of any platform limitation.
- Note: nothing in *this* theme touches the closed-firmware controller
  restriction — that lives in the controller/upload themes, not effect panels.

## Recommended sequencing

1. ~~**Sketch parity (P1, medium):** close-path + SVG import + path reorder + curve
   segments in `SketchPathEditorRowView`~~ — ✅ **done 2026-06-11.** Shared SVG→
   sketch conversion now lives in core `SketchSVGImport`; only per-path
   descriptions remain desktop-only.
2. ~~**Tooltips + canvasMode auto-enable (P3, easy)**~~ — ✅ **done 2026-06-11.**
3. **Per-property Copy/Paste/Reset on desktop (P2, medium):** close the one real
   desktop-missing gap by porting the iPad context-menu trio into BulkEditControls
   — keeps the two clients symmetrical.
4. ~~**Moving Head path canvas (P2, hard):** new touch path editor writing the
   shared `Path=` command.~~ — ✅ **done.** `MovingHeadPathEditorRowView` reuses
   `SketchDefinition` with a Y-flip and writes the same `Path:` command via the
   existing MH command bridge. Path/dimmer presets (`MovingHeadPathPresetStrip`,
   `MovingHeadDimmerPresetStrip`) and a colour-wheel picker
   (`MovingHeadColorWheelRowView` + the new `movingHeadWheelColors` bridge) also
   landed; deferred: the RGB picker canvas (native ColorPicker used instead) and
   the multi-colour wheel *animation* double-selector.
5. ~~**DMX arbitrary remap (P3, medium):**~~ — ✅ **done.** Touch grid
   (`DMXRemapGridSheet`) + explicit-mapping bridge variant. **Pictures paint
   tools** remain the only P3 hard item here; lowest value (most users edit
   images externally), tackle only if user demand surfaces.
6. ~~**Reset to Defaults + Resize-on-add (P3, easy):**~~ — ✅ **done.** Header
   menu reset action + `PicturesResizeOnAddSheet` on Pictures image add.
7. **Effect Wheel** — ➖ **declined** (touch-idiom mismatch; no keybinding store
   on iPad, palette tap-to-drop is the fuller equivalent).
