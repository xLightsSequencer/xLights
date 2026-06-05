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
> (**Pictures paint tools** and the **Moving Head path canvas** are iPad-missing;
> **Sketch** is iPad-partial — no close-path/SVG-import/reorder), the desktop's
> richer **DMX RemapDMXChannelsDialog** grid (iPad ships a preset-menu subset),
> **canvasMode auto-enable** (parsed but ignored on iPad), and **tooltip
> rendering** (parsed but never shown on iPad). Going the other way, the iPad's
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
| Tooltips on property labels | preference | ✅ | ❌ | ipad-missing | P3 | easy | feasible | Desktop SetToolTip (JsonEffectPanel.cpp:1602/1614). iPad parses `tooltip` (EffectMetadata.swift:36) but never renders it. |
| canvasMode auto-enable | preference | ✅ | ❌ | ipad-missing | P3 | easy | feasible | Adjust/Kaleidoscope/Warp have `canvasMode:true`. Desktop auto-checks Canvas (JsonEffectPanel.cpp:2187). iPad parses the field (EffectMetadata.swift:19) but ignores it — user must toggle CanvasRow manually. |
| Canvas / Layers composite control | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad CanvasRow (BlendingPanelViews.swift:241) — Canvas checkbox + Layers… picker. |
| Layer Morph (cross-fade Effect 1↔2) | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad LayerMorphRow (BlendingPanelViews.swift:75). |
| Layer Method (24 blend modes) | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad LayerMethodRow + LayerMethodHelpSheet (BlendingPanelViews.swift:130). |
| Transition type (In/Out, 22 types) | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad TransitionHeaderRow + shared/Blending.json. |
| Transition fade time presets + manual | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad kFadePresets matches desktop Fadein/Fadeout. |
| Transition Adjust disable rules | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad transitionAdjustDisabled (EffectPropertyView.swift:59) vs desktop kTransitionsNoAdjust ValidateWindow. |
| Transition Reverse disable rules | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad transitionReverseDisabled (EffectPropertyView.swift:63) vs desktop kTransitionsNoReverse. |
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
| DMX: Remap channels | dialog | ✅ | 🟡 | ipad-weaker | P3 | medium | feasible | iPad ships a preset menu (Shift +1/-1, Reverse, Invert All, Double) via `dmxRemapChannelsForRow` (XLSequenceDocument.h:2205). Desktop has full `RemapDMXChannelsDialog` 48-row grid. iPad lacks the arbitrary per-channel remap. |
| DMX: Save/Load State | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad DMXButtonsRowView → `dmxSaveStateForRow`/`dmxLoadStateForRow` (mirror DMXPanel Save/Load). |
| Faces: phoneme vs timing-track selector | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad FacesMouthMovementsRowView (segmented + phoneme/track dropdown). (Face *definition* authoring is a model-level feature, not the effect panel.) |
| Morph: quick-set preset menu | menu | ✅ | ✅ | parity | P2 | easy | feasible | iPad MorphQuickSetRowView (8 presets). |
| Morph: line geometry editor | panel | ✅ | ✅ | parity | P1 | medium | feasible | iPad MorphLineEditorRowView (touch canvas, 4 colored draggable handles, coordinate readout) ≈ desktop xlGridCanvasMorph. |
| Morph: swap start/end | menu | ✅ | ✅ | parity | P2 | easy | feasible | iPad MorphSwapRowView. |
| Moving Head: fixture + color/dimmer rows | panel | ✅ | ✅ | parity | P2 | medium | feasible | iPad MovingHead{Fixture,Color,Dimmer}RowView: fixture select, pan/tilt/offset/groupings/cycles, single-color, dimmer intensity. |
| Moving Head: path canvas (waypoint drawing) | panel | ✅ | ❌ | ipad-missing | P2 | hard | feasible | Desktop MovingHeadCanvasPanel.cpp draws Bezier waypoint paths; iPad MovingHeadPathRowView is **read + clear only** (MovingHeadFixtureRowView.swift:291, "Use desktop's Effect Assist to draw one"). |
| Piano: note/octave + labels | panel | ✅ | ✅ | parity | P2 | easy | feasible | Piano.json standard controls. |
| Pictures: filename block + transparent-black | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad EffectFilenameBlockView + TransparentBlackRowView. |
| Pictures: AI image generate | dialog | ✅ | ✅ | parity | P2 | medium | feasible | Desktop PicturesPanel "AI Generate…" (PicturesPanel.cpp:137) ↔ iPad AIImageGenerationSheet (from filename block "AI…"). |
| Pictures: paint/assist canvas (brush/palette) | panel | ✅ | ❌ | ipad-missing | P3 | hard | hard | Desktop PicturesAssistPanel + xlGridCanvasPictures (pencil/eraser/eyedropper, palette load/save, new/open image). No touch bitmap editor on iPad. Workaround: edit externally / on desktop. |
| Servo: start/end slider pair | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad ServoStartEndRowView. |
| Servo: increment/decrement buttons | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad ServoButtonRowView. |
| Shader: file picker (.fs) | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad Shader_FilenameBlock (EffectFilenameBlockView). |
| Shader: dynamic params from uniforms | panel | ✅ | ✅ | parity | P2 | medium | feasible | iPad ShaderDynamicParamsView → bridge `shaderDynamicPropertiesJsonForPath` (XLSequenceDocument.h:1551) → renders via full EffectPropertyView (so sliders/choices/checkboxes/point2d **with VC buttons**). |
| Shader: time speed slider + VC | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad ShaderSpeedRowView (synthetic PropertyMetadata → slider + VC). |
| Shape: font + emoji category | panel | ✅ | ✅ | parity | P2 | medium | feasible | iPad ShapeFontRowView. |
| Shape: character / emoji picker | panel | ✅ | ✅ | parity | P2 | medium | feasible | iPad ShapeCharRowView. |
| Shape: skin-tone modifier | panel | ✅ | ✅ | parity | P3 | easy | feasible | iPad ShapeSkinToneRowView. |
| Shape: SVG path field | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad ShapeSVGRowView (+ preview). |
| Sketch: path editor canvas | panel | ✅ | 🟡 | ipad-weaker | P1 | medium | feasible | Desktop SketchAssistPanel is full (Start/Continue/End/**Close**, Import/Export Sketch, **Import SVG**, **Move Path Up/Down**, Bezier). iPad SketchPathEditorRowView = add line / new path / undo point / clear / drag handles only. iPad missing: close path, SVG import, reorder, curve segments. |
| Sketch: background image + opacity | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad SketchBackgroundRowView. |
| Sketch: info block (counters) | panel | ✅ | ✅ | parity | P3 | easy | feasible | iPad SketchInfoRowView. |
| Sketch: raw SketchDef field | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad SketchDefRowView (accepts desktop-generated defs incl. curves). |
| State: state vs timing-track selector | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad StateStateSourceRowView. |
| Text: font picker (XL font) | panel | ✅ | ✅ | parity | P2 | medium | feasible | iPad TextFontXLRowView. |
| Text: load text from file | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad Text_File_Row. |
| Video: filename + transparent-black | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad Video_FilenameBlock + TransparentBlackRowView. |
| Video: duration / metadata readout | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad VideoDurationRowView. |
| VUMeter: mode choice | panel | ✅ | ✅ | parity | P2 | easy | feasible | VUMeter.json standard choice. |
| Effect Assist (separate window concept) | panel | ✅ | 🟡 | ipad-missing | P2 | hard | feasible | Desktop floating Assist window for Sketch/Morph/Pictures (EffectPanelManager HasAssistPanel — only these 3). iPad inlines Sketch+Morph editors in the inspector; Pictures painting absent; no detached-window concept. |
| Preference: reset Color/Buffer/Blending panel on effect change | preference | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop wxConfig xLightsReset{Color,Buffer,Blending}Panel. iPad ResetPanelRow renders EmptyView by design (inspector always shows anchor values; nothing to govern). |
| Convert selected effects to a different type | context-menu | ✅ | ❌ | ipad-missing | P2 | hard | feasible | Desktop converts a multi-selection of effects to another effect type; no iPad equivalent. |
| Resize-on-add image dialog | dialog | ✅ | ❌ | ipad-missing | P3 | easy | feasible | Desktop ResizeImageDialog (src-ui-wx/media/ResizeImageDialog.cpp) prompts to resize an image when added to a Pictures effect; iPad has no resize-on-add dialog. |

## iPad gaps (desktop has, iPad missing)

### P1
- **Sketch close-path / SVG-import / path-reorder / curves.** Desktop
  `SketchAssistPanel.cpp:76-84` (Close, Import Sketch, Export Sketch, Import SVG,
  Move Path Up/Down) + Bezier curve segments on the canvas. iPad
  `SketchPathEditorRowView.swift` covers add-line / new-path / undo-point / clear
  / drag-to-reshape only. **Work:** extend SketchPathEditorRowView with a
  close-path gesture/button, path-list reorder UI, a document-picker SVG import
  routed through the existing core SVG→sketch parser, and curved-segment handles.
  Medium. Highest-value iPad authoring gap in this theme.

### P2
- **Moving Head waypoint path canvas.** Desktop `MovingHeadCanvasPanel.cpp`
  draws Bezier pan/tilt paths; iPad `MovingHeadPathRowView` is read+clear only.
  **Work:** a new touch path canvas writing the same `Path=` command the bridge
  already round-trips (`movingHeadCommand`/`setMovingHeadCommand`,
  XLSequenceDocument.h:2141-2149). Hard (new interactive canvas), but the data
  model is shared so it's purely UI.
- **Effect Assist as a first-class surface.** Only Sketch+Morph editors are
  inlined; Pictures has none. **Work:** see Pictures paint tools (P3) and Sketch
  (P1); no separate "window" needed on iPad — keep inlining in the inspector.
- **Convert selected effects to a different type.** Desktop converts a
  multi-selection of effects to another effect type; iPad has no equivalent.
  **Work:** a selection-aware command that rewrites each selected effect's type
  (and migrates settings where sensible). Hard.

### P3
- **DMX arbitrary channel remap.** Desktop `RemapDMXChannelsDialog` is a 48-row
  grid; iPad exposes a preset menu only. **Work:** a touch grid editor calling a
  generalized `dmxRemapChannelsForRow` variant that accepts an explicit mapping.
  Medium; low value (uncommon op).
- **Tooltips on property labels.** Metadata already carries `tooltip`
  (EffectMetadata.swift:36); just unrendered. **Work:** add a `.help()` or
  info-button affordance in EffectPropertyView's label. Easy.
- **canvasMode auto-enable.** Field parsed but unused. **Work:** when
  `selectedEffectMetadata.canvasMode == true`, have the view model set the
  `T_CHECKBOX_Canvas` setting on selection (respecting user override). Easy.
- **Pictures paint tools.** Desktop pixel editor with brushes + palette mgmt.
  Hard touch redesign; low priority (most users edit images externally).
- **Resize-on-add image dialog.** Desktop ResizeImageDialog
  (src-ui-wx/media/ResizeImageDialog.cpp) prompts to resize an image when it's
  added to a Pictures effect; iPad has no resize-on-add dialog. **Work:** a small
  SwiftUI sheet offering resize-to-model-buffer on image add. Easy.
- **Reset-panel-on-effect-change preference.** Intentionally a no-op on iPad
  (inspector shows anchor values). Only relevant if iPad ever adopts sticky
  panel state. Low priority.

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

- *None platform-blocked in this theme.* Every gap above is feasible — the core
  effect/render/value-curve logic is shared (`src-core/`), so all remaining work
  is SwiftUI + thin bridge additions. The harder ones (Moving Head path canvas,
  Pictures paint tools) are "hard" only because they need new interactive touch
  canvases, not because of any platform limitation.
- Note: nothing in *this* theme touches the closed-firmware controller
  restriction — that lives in the controller/upload themes, not effect panels.

## Recommended sequencing

1. **Sketch parity (P1, medium):** close-path + SVG import + path reorder + curve
   segments in `SketchPathEditorRowView`. Biggest authoring gap; core SVG/sketch
   parsing already exists.
2. **Tooltips + canvasMode auto-enable (P3, easy, cheap wins):** render the
   already-parsed `tooltip`, and honor `canvasMode` on selection. Both are tiny
   and remove "why is this different from desktop" friction.
3. **Per-property Copy/Paste/Reset on desktop (P2, medium):** close the one real
   desktop-missing gap by porting the iPad context-menu trio into BulkEditControls
   — keeps the two clients symmetrical.
4. **Moving Head path canvas (P2, hard):** new touch path editor writing the
   shared `Path=` command. Defer until 1-3 land since it's the most UI-heavy.
5. **Pictures paint tools + DMX arbitrary remap (P3, hard/medium):** lowest value;
   tackle only if user demand surfaces. Until then the read/clear + preset-menu
   subsets are acceptable.
