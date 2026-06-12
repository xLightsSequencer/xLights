# 05 · Color Panel, Palettes & Value/Color Curves

> This theme is at **strong parity**. The **Brightness / Contrast / Hue /
> Saturation / Value adjustment sliders are present on iPad**: those are plain
> `controlType: "slider"` properties in `resources/effectmetadata/shared/Color.json`,
> and the iPad Color tab renders the entire Color.json property list
> generically via `EffectMetadataPanel` → `EffectPropertyView.sliderView`,
> which also attaches a `ValueCurveButton` to every slider whose metadata
> sets `valueCurve: true`. So all five HSV/brightness sliders **and their
> value curves are present on iPad today.** The ValueCurve editor is
> near-complete on iPad (23 types, P1–P4, range, wrap/real/offset,
> timing/audio/filter, reverse, flip, copy/paste, load/save preset). The
> ColorCurve gradient editor is present on iPad (9 modes, point add/move/
> delete, position field, flip) and now has **preset Load / Save-As /
> Export** matching desktop's `ColorCurveDialog` (`.xcc` files interchange
> byte-for-byte). The remaining real gaps are mostly
> sequencer-level **dropper panels** (Color Dropper / Value-Curve Dropper —
> View-menu dockable panes with drag-to-slot, plus the F11
> `COLOR_DROPPER_TOGGLE` keybinding) which iPad has no equivalent for, plus
> a couple of palette-menu items (Update Palette to bulk-apply across
> selection). iPad's leads here — in-dialog VC clipboard copy/paste and
> palette "Copy String" — are implemented for desktop on the
> `desktop-pullbacks` branch (shared `xlvc:v1:` / `GetCurrentPalette()`
> clipboard formats), pending post-release merge. The color-picker gaps
> (CSS named-color swatches and persisted recent-color swatches) are now closed:
> `XLColorSwatchPicker` delivers both via the "Pick Color…" long-press entry on
> palette slots. The residual iPad-ahead items are per-VC-preset swipe-delete
> and a ColorCurve position field.

## Parity scorecard

| Feature | Surface | Desktop | iPad | Gap | Priority | Ease | Feasibility | Notes |
|---|---|---|---|---|---|---|---|---|
| Palette grid (8 slots) | panel | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `ColorCurveButton`×8 (`ColorPanel.cpp:280+`); iPad `ColorPaletteView` 8 rows. |
| Per-slot enable checkbox (`C_CHECKBOX_PaletteN`) | panel | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `ID_CHECKBOX_Palette%d` (`ColorPanel.cpp:280`); iPad Toggle (`ColorPaletteView.swift:206`). |
| Slot color picker (hex) | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop left-click → `xlColourData` (`wxColorCurveButton.cpp:38`); iPad `ColorPicker` (`ColorPaletteView.swift:259`). |
| Slot gradient mode (ColorCurve) | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop right-click → `ColorCurveDialog` (`wxColorCurveButton.cpp:53`); iPad long-press / tap strip → `ColorCurveEditorSheet`. |
| Palette menu: Update Palette (bulk-apply to selection) | menu | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `ID_MNU_UPDATE`→`UpdateColor()` (`ColorPanel.cpp:1049,974`); iPad "Update Palette" in `ColorPaletteView.paletteMenuContent()` → `SequencerViewModel.updatePaletteOnAllSelected()` (`ColorPaletteView.swift:95`, `SequencerViewModel.swift:2836`). Confirmation alert when >1 effect selected; single undo step via `restoreBulkSettings`. |
| Palette menu: Save Palette | menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `ID_MNU_SAVE`→`SavePalette(false)`; iPad `savePaletteString(_:asName:nil)`. |
| Palette menu: Save Palette As | menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `ID_MNU_SAVE_AS`; iPad `PaletteSaveAsSheet`. |
| Palette menu: Load Saved Palette | menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop loads from disk; iPad `PaletteLoadSheet` (swatch rows, swipe-delete). |
| Palette: Delete saved palette | menu/gesture | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `ID_MNU_DELETE` (enabled only for custom on-disk palettes); iPad swipe-to-delete in `PaletteLoadSheet`. UX differs, function equal. |
| Palette menu: Import from text | menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `ID_MNU_IMPORT`→`ImportPalette`; iPad `PaletteImportSheet` (validates + prefills from pasteboard). |
| Palette menu: AI Generate Palette | menu | ✅ | ✅ | parity | P2 | medium | feasible | Both gated on AI ColorPalettes capability. Desktop `AIColorPaletteDialog`; iPad `AIPaletteGenerationSheet`. |
| Palette: Copy palette string | menu | ❌ | ✅ | desktop-missing | P3 | easy | feasible | **Implemented on branch `desktop-pullbacks`; pending post-release merge.** Desktop adds "Copy Palette String" to `OnBitmapButton_MenuPaletteClick` (`ID_MNU_COPY` → `ColorPanel::CopyPaletteString` writes `GetCurrentPalette()` to `wxTheClipboard`). iPad menu → `UIPasteboard` (`ColorPaletteView.swift:114`). |
| Palette shift left | toolbar/menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `_leftShiftColoursButton` (`ColorPanel.cpp:257`); iPad menu "Shift Left". |
| Palette shift right | toolbar/menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `_rightShiftColoursButton`; iPad menu "Shift Right". |
| Palette reverse colors | toolbar/menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `_reverseColoursButton`; iPad menu "Reverse Colors". |
| Palette slots are drag-drop targets | gesture | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop `ColourTextDropTarget` on each `ColorCurveButton` (`ColorPanel.cpp:167`) accepts drops from the dropper panels. iPad has no dropper panels to drag from; tap pickers replace the idiom. |
| Chroma Key enable toggle | panel | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `_enableChromaCheck`; iPad `ChromaKeyRowView` (`C_CHECKBOX_Chroma`). |
| Chroma Key color picker | panel | ✅ | ✅ | parity | P1 | easy | feasible | `C_COLOURPICKERCTRL_ChromaColour`, default `#000000`. |
| Chroma Key sensitivity slider | panel | ✅ | ✅ | parity | P1 | easy | feasible | `C_SLIDER_ChromaSensitivity` 1..255 (`ColorPanel.cpp:374`). |
| Sparkles frequency slider | panel | ✅ | ✅ | parity | P1 | easy | feasible | `C_SLIDER_SparkleFrequency` 0..200 (`ColorPanel.cpp:415`). |
| Sparkles frequency value curve | panel | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `_sparkleFrequencyVC` + lock; iPad synthesises metadata + `ValueCurveButton` (`ColorPanelCustomRows.swift:128`). |
| Sparkles reflect-music toggle | panel | ✅ | ✅ | parity | P1 | easy | feasible | `C_CHECKBOX_MusicSparkles`. |
| Sparkles color picker | panel | ✅ | ✅ | parity | P1 | easy | feasible | `C_COLOURPICKERCTRL_SparklesColour`, default `#FFFFFF`. |
| Sparkles / Brightness lock buttons | panel | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop `xlLockButton`s + `lockable:true` in Color.json prevent bulk-edit propagation. iPad has no lock-button concept anywhere. |
| Brightness Level checkbox | panel | ✅ | ✅ | parity | P2 | easy | feasible | Legacy key `CHECKBOXBRIGHTNESSLEVEL` (no underscore). Desktop `_brightnessLevelCheck`; iPad `BrightnessLevelRowView`. |
| Brightness slider (0–400%) | panel | ✅ | ✅ | parity | P1 | easy | feasible | `C_SLIDER_Brightness` standard slider in Color.json; iPad renders it via generic `sliderView`. |
| Brightness value curve | panel | ✅ | ✅ | parity | P1 | easy | feasible | `valueCurve:true` → iPad `ValueCurveButton` auto-attaches (`EffectPropertyView.swift:315`). |
| Contrast slider (−100..100) | panel | ✅ | ✅ | parity | P1 | easy | feasible | `C_SLIDER_Contrast` standard slider; rendered generically on iPad. |
| Hue adjust slider (−100..100) | panel | ✅ | ✅ | parity | P1 | easy | feasible | `C_SLIDER_Color_HueAdjust`, in "Adjustment" section group; iPad renders section + slider. |
| Hue adjust value curve | panel | ✅ | ✅ | parity | P1 | easy | feasible | `valueCurve:true` → VC button auto-attaches on iPad. |
| Saturation adjust slider | panel | ✅ | ✅ | parity | P1 | easy | feasible | `C_SLIDER_Color_SaturationAdjust`. |
| Saturation adjust value curve | panel | ✅ | ✅ | parity | P1 | easy | feasible | `valueCurve:true` → VC button on iPad. |
| Value adjust slider | panel | ✅ | ✅ | parity | P1 | easy | feasible | `C_SLIDER_Color_ValueAdjust`. |
| Value adjust value curve | panel | ✅ | ✅ | parity | P1 | easy | feasible | `valueCurve:true` → VC button on iPad. |
| "Adjustment" collapsible section | panel | ✅ | ✅ | parity | P3 | easy | feasible | Color.json `groups[type=section]`; iPad `EffectMetadataPanel.groupView(section)` renders header + members. |
| ValueCurve type selector (23 types) | dialog | ✅ | ✅ | parity | P1 | easy | feasible | iPad `XLValueCurve.availableTypes()` lists 23 (`XLValueCurve.mm:190`). |
| ValueCurve custom-point canvas | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `ValueCurvePanel`; iPad `ValueCurveCustomPointEditor` (`ValueCurveCanvases.swift`). |
| ValueCurve P1–P4 sliders + text | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Per-type ranges via `XLValueCurve.range(forParameter:…)` → `GetRangeParm` (`XLValueCurve.mm:180`); labels in `parameterLabel` (`ValueCurveEditor.swift:563`). Custom P1=Cycles 1–10 (#6488) auto-surfaces since core `GetRangeParm1` now returns 1..10. |
| ValueCurve Custom "Cycles" (P1 1–10) | dialog | ✅ | ✅ | parity | P3 | easy | feasible | Core replication shared (`ValueCurve.cpp` RenderType Custom, #6488). Desktop labels P1 "Cycles", disables point-edit/Reverse/Flip when >1. iPad now mirrors: P1 labeled "Cycles" for Custom + Points canvas and Reverse/Flip gated on cycles>1 (`ValueCurveEditor.swift` `customCyclesLocked`). |
| ValueCurve min/max range fields | dialog | ✅ | ✅ | parity | P2 | easy | feasible | iPad "Range" section (`ValueCurveEditor.swift:372`). |
| ValueCurve wrap checkbox | dialog | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `CheckBox_WrapValues`; iPad Toggle "Wrap". |
| ValueCurve real-values checkbox | dialog | ✅ | ✅ | parity | P2 | easy | feasible | iPad Toggle "Real Values". |
| ValueCurve time offset | dialog | ✅ | ✅ | parity | P2 | easy | feasible | iPad "Time Offset (ms)" field. |
| ValueCurve timing-track selector | dialog | ✅ | ✅ | parity | P2 | easy | feasible | iPad picker via `dynamicOptions("timingTracks")`. |
| ValueCurve audio-track field | dialog | ✅ | ✅ | parity | P2 | easy | feasible | iPad TextField for Music/Inverted-Music/MusicTriggerFade. |
| ValueCurve filter label + regex | dialog | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `CheckBox_FilterLabelRegex`; iPad TextField + Toggle. |
| ValueCurve flip | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `Button_Flip`; iPad "Flip". |
| ValueCurve reverse | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `Button_Reverse`; iPad "Reverse". |
| ValueCurve load preset | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `ButtonLoad` + preset bitmap row; iPad `ValueCurveLoadPresetSheet`. |
| ValueCurve save-as preset | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop saves to `<show>/valuecurves/`; iPad `ValueCurveSaveAsSheet`. |
| ValueCurve delete preset | dialog/gesture | ❌ | ✅ | desktop-missing | P3 | easy | feasible | iPad swipe-delete in load sheet (`deleteSavedValueCurve`). Desktop dialog has only a CLEAR-curve button, no per-preset delete (file managed in Finder). |
| ValueCurve export-to-file | dialog | ✅ | ✅ | parity | P2 | medium | feasible | Desktop `ButtonExport` writes `.xvc` to an arbitrary path. iPad "Export…" in `ValueCurveEditorSheet` (`ValueCurveEditor.swift`) → `.fileExporter` over `valueCurveXvcDocument` bridge (`XLSequenceDocument.mm`), which routes through core `ValueCurve::SaveXVC` so bytes match desktop. |
| ValueCurve copy to clipboard | dialog | ❌ | ✅ | desktop-missing | P2 | easy | feasible | **Implemented on branch `desktop-pullbacks`; pending post-release merge.** Desktop `ValueCurveDialog` 'Copy' button (`OnButton_CopyClick`) writes `xlvc:v1:` + `ValueCurve::Serialise()` to `wxTheClipboard`, matching iPad `ValueCurveClipboard.wrap` so curves interchange. |
| ValueCurve paste from clipboard | dialog | ❌ | ✅ | desktop-missing | P2 | easy | feasible | **Implemented on branch `desktop-pullbacks`; pending post-release merge.** Desktop `ValueCurveDialog` 'Paste' button (`OnButton_PasteClick`) unwraps the `xlvc:v1:` envelope, `Deserialise`s into the curve, then resyncs the dialog controls. Matches iPad "Paste". |
| ValueCurve undo of point edits | dialog | ✅ | 🟡 | ipad-missing | P3 | medium | feasible | Desktop `ValueCurvePanel::Undo()` per-point undo stack. iPad has no in-editor undo; sequence-level Foundation undo covers the whole curve write. |
| ValueCurve preview strip | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop paints in panel; iPad `ValueCurvePreviewStrip`. |
| ColorCurve editor: active toggle | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop dialog Ok=active; iPad "Use gradient" toggle (writes `Active=FALSE\|`). |
| ColorCurve mode picker (9 modes) | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `Choice1` "Blend Mode"; iPad `ColorCurveModePicker` (Time / 4 linear / 2 radial / 2 rotation) with per-effect enable. |
| ColorCurve gradient point canvas | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `ColorCurvePanel`; iPad `ColorCurveGradientStrip` (tap-add / drag-move / long-press-delete). |
| ColorCurve point color picker | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `wxColourData`; iPad `ColorPicker` for selected point. |
| ColorCurve point position field | dialog | ❌ | ✅ | desktop-missing | P3 | easy | feasible | iPad `ColorCurvePositionField` numeric x for selected point. Desktop moves points by drag only. |
| ColorCurve delete point | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop double-click; iPad "Delete Point" / long-press. |
| ColorCurve flip | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop "Flip Colours" (`ColorCurveDialog.cpp:131`); iPad "Flip Horizontally". |
| ColorCurve load preset | dialog | ✅ | ✅ | parity | P2 | medium | feasible | Desktop `ButtonLoad` + preset bitmap row (`OnButtonPresetClick`). iPad `ColorCurveLoadSheet` (`ColorCurvePresetSheets.swift`) lists `.xcc` from `<show>/colorcurves/` + bundled via `savedColorCurves` bridge (`XLSequenceDocument.mm`), gradient-strip thumbnails, swipe-delete. |
| ColorCurve save-as preset | dialog | ✅ | ✅ | parity | P2 | medium | feasible | Desktop saves `.xcc` to `<show>/colorcurves/`. iPad `ColorCurveSaveAsSheet` → `saveColorCurveSerialised(_:asName:)` bridge writes the same `<colorcurve data="…" SourceVersion="…">` XML so presets interchange. |
| ColorCurve export-to-file | dialog | ✅ | ✅ | parity | P3 | medium | feasible | Desktop `ButtonExport`. iPad "Export…" in `ColorCurveEditorSheet` → `.fileExporter` over `colorCurveXccDocument` bridge writes a `.xcc` anywhere (same bytes as `OnButtonExportClick`). |
| ColorCurve: convert slot to plain color | context-menu | 🟡 | ✅ | parity | P3 | easy | feasible | iPad long-press → "Convert to Plain Colour" (`ColorPaletteView.swift:276`). Desktop achieves same via left-click color dialog (deactivates curve). Functionally equal; iPad is more explicit. |
| ColorCurve: edit-as-gradient entry | context-menu | ✅ | ✅ | parity | P3 | easy | feasible | Desktop right-click slot; iPad long-press → "Edit as Gradient". |
| Color Replace dialog | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop Edit ▸ Color Replace → `ColourReplaceDialog` (`xLightsMain.cpp:1095`); iPad `ColorReplaceSheet` (`SequencerGridV2View.swift:1083`). |
| Color Replace: existing-color dropdown | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `ColoursList` owner-drawn combo; iPad picker from `usedColours()`. |
| Color Replace: replacement color | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop color dialog; iPad `ColorPicker`. |
| Color Replace: selected-effects-only | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `CheckBox_SelectedEffectsOnly`; iPad Toggle (only when selection exists). |
| Color Replace: replace action | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Both undoable, no auto-render. iPad uses `replaceColour(from:to:…)` bridge. |
| Color Dropper panel (View menu pane) | panel | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop dockable `_coloursPanel` "Colours" pane (`tabSequencer.cpp:203`), toggled View ▸ Color Dropper (`ID_MNU_COLOURDROPPER`); drag colours onto slots. No iPad equivalent. |
| Value-Curve Dropper panel (View menu pane) | panel | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop dockable `_valueCurvesPanel` "Value Curves" pane (`tabSequencer.cpp:202`); drag VC presets onto sliders. No iPad equivalent (presets reached via in-editor sheet instead). |
| `COLOR_DROPPER_TOGGLE` keybinding (F11) | shortcut | ✅ | ❌ | ipad-missing | P3 | hard | infeasible | `KeyBindings.cpp:124,377` toggles the Color Dropper pane. iPad has no dropper pane and the binding governs a panel that doesn't exist there. |
| Palette size (Normal/Large) preference | preference | ✅ | 🟡 | ipad-missing | P3 | easy | feasible | Desktop reads wxConfig `PaletteSize` (`ColorPanel.cpp:77`). iPad sizes fixed by SwiftUI; no user toggle. |
| "Reset panel on effect change" preference | preference | ✅ | ❌ | ipad-missing | P3 | easy | feasible | Desktop `BuildResetPanelRow` → wxConfig `xLightsResetColorPanel`. iPad doesn't reset panel state on effect change, so the toggle has nothing to govern (`EffectPropertyView.swift:247` renders nothing for `ResetPanelRow`). |
| Recent-palettes dropdown | menu/panel | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop `_colourList` (`ColourList` owner-drawn combo, `ColorPanel.cpp:330`) lists bundled + on-disk palettes inline. iPad only has the Load sheet. |
| Color Replace: color-curve replacement target | dialog | ✅ | 🟡 | ipad-weaker | P3 | easy | feasible | Desktop `ColourReplaceDialog` carries a `ColorCurveButton _ccb` (`ColourReplaceDialog.h:28`, created `ColourReplaceDialog.cpp:137`, rendered `:78-79`) so the replacement can be a color curve. iPad `ColorReplaceSheet` only offers a plain replacement color. |
| Common (CSS) named color swatches | dialog | ✅ | ✅ | parity | P3 | easy | feasible | Desktop `xlColourPickerDialog` CSS grid (`xlColourPickerDialog.cpp:32,104,266`); iPad `XLColorSwatchPicker` 140-entry `cssColors[]` grid (20 cols × 7 rows, `XLColorSwatchPicker.swift:cssColors`), opened via "Pick Color…" long-press context menu on plain palette slots (`ColorPaletteView.swift`). |
| Recent colors swatches (persisted) | dialog | ✅ | ✅ | parity | P3 | easy | feasible | Desktop `xlColourPickerDialog` persisted recents (`xlColourPickerDialog.cpp:280,383`); iPad `XLRecentColors` (`XLColorSwatchPicker.swift:XLRecentColors`) — `@AppStorage`-style `UserDefaults` hex list, deduped move-to-front, capped at 24, updated on every color commit from both the swatch picker and the system `ColorPicker` (`ColorPaletteView.swift:colorBinding.set`). |

## iPad gaps (desktop has, iPad missing)

### P2

- **ColorCurve presets (Load / Save / Export).** ✅ **Landed.** Mirrors the
  desktop `ColorCurveDialog` trio (`ButtonLoad`, the preset bitmap row
  `OnButtonPresetClick`, and `ButtonExport`,
  `src-ui-wx/color/ColorCurveDialog.cpp:131-140,782-856`). Bridge:
  `savedColorCurves` / `saveColorCurveSerialised(_:asName:)` /
  `deleteSavedColorCurve` / `colorCurveXccDocument:` on
  `XLSequenceDocument.h/.mm` (cloned from the `savedValueCurves` trio; they
  enumerate `<show>/colorcurves/*.xcc` + bundled `colorcurves/`, round-trip
  via core `ColorCurve::LoadXCC` / `Serialise`). UI:
  `ColorCurveLoadSheet` + `ColorCurveSaveAsSheet` in
  `src-iPad/App/ColorCurvePresetSheets.swift` (gradient-strip thumbnails,
  swipe-delete), wired into `ColorCurveEditorSheet`
  (`src-iPad/App/ColorCurveEditor.swift`) with Load / Save-As / Export
  buttons. The on-disk `.xcc` is the same `<colorcurve data="…"
  SourceVersion="…">` envelope desktop writes, so presets interchange
  byte-for-byte.

- **ValueCurve export-to-file.** ✅ **Landed.** Matches desktop `ButtonExport`
  (`ValueCurve::SaveXVC`). iPad adds an "Export…" action in
  `ValueCurveEditorSheet` (`src-iPad/App/ValueCurveEditor.swift`) →
  SwiftUI `.fileExporter` carrying bytes from the new
  `valueCurveXvcDocument:` bridge, which routes through core
  `ValueCurve::SaveXVC` (a temp file read back) so the limit / scale
  normalisation and XML envelope match desktop exactly.

- ~~**Update Palette (bulk-apply to all selected effects).**~~ **Landed.**
  "Update Palette" menu item added to `ColorPaletteView.paletteMenuContent()`
  (`ColorPaletteView.swift:85`). Shows a confirmation alert when the selection
  size is > 1; calls `SequencerViewModel.updatePaletteOnAllSelected()`
  (`SequencerViewModel.swift:2832`) which reads the anchor's 8-slot palette via
  `effectPalette(forRow:at:)` (`SequencerViewModel.swift:2836`), writes each
  changed key to every other selected effect with `setEffectSettingValue`, and
  registers a single `restoreBulkSettings` undo step. Full parity with desktop
  `UpdateColor()` / `ID_MNU_UPDATE`.

### P3

- **Color Dropper & Value-Curve Dropper panes** (`tabSequencer.cpp:202-203`)
  plus the **`COLOR_DROPPER_TOGGLE` F11 keybinding** (`KeyBindings.cpp:124`).
  Dockable drag-source panels; the touch app replaces drag-from-panel with
  tap pickers + in-editor preset sheets, so these are low value on iPad and
  the F11 binding governs a panel that doesn't exist. Treat as a future
  "colours-used quick palette" if desired, not a parity must.
- **Per-slider lock buttons** (`lockable:true` + `xlLockButton`). Bulk-edit
  propagation guard; iPad has no lock-button concept at all (cross-theme
  gap, not Color-specific).
- **Palette drag-drop targets**, **recent-palettes inline dropdown**,
  **Palette size preference**, **Reset-panel-on-effect-change preference**,
  **in-editor per-point VC undo** — all minor; see scorecard.
- **Color Replace color-curve target.** Desktop `ColourReplaceDialog`
  carries a `ColorCurveButton _ccb` (`ColourReplaceDialog.h:28`,
  `ColourReplaceDialog.cpp:137`, `:78-79`) so the replacement can be a
  color curve; iPad `ColorReplaceSheet` only offers a plain color.

## Desktop gaps (iPad has, desktop missing)

### P2

- **ValueCurve clipboard Copy / Paste.** *Done.* Desktop `ValueCurveDialog`
  now has 'Copy'/'Paste' buttons next to Load/Export
  (`OnButton_CopyClick`/`OnButton_PasteClick`) using `wxClipboard` and the
  same `xlvc:v1:` + `ValueCurve::Serialise()` envelope as iPad
  `ValueCurveClipboard`, so curves interchange between the two clients.

### P3

- **Palette: Copy palette string to clipboard.** *Done.* Desktop
  `OnBitmapButton_MenuPaletteClick` now has a "Copy Palette String"
  item (`ID_MNU_COPY` → `ColorPanel::CopyPaletteString`).
- **ValueCurve per-preset delete.** iPad swipe-deletes presets in the load
  sheet (`deleteSavedValueCurve`). Desktop's dialog/panel only has a
  CLEAR-curve button (`ValueCurvesPanel.cpp:134`), no per-file delete — users
  manage `.xvc` files in Finder/Explorer.
- **ColorCurve point position numeric field.** iPad `ColorCurvePositionField`
  lets you type the selected point's x; desktop only drags on the canvas.

## Infeasible / restricted on iPad

- **`COLOR_DROPPER_TOGGLE` (F11)** — *infeasible as-is*: it toggles a
  dockable wxAUI pane that has no iPad analog, and iPad has no hardware-key
  requirement. Not a closed-firmware restriction — just no target.
- Nothing in this theme is **restricted** (no controller-firmware coupling):
  palettes, curves, chroma, sparkles, and color replace are all pure
  `src-core` render-setting edits shared by both apps.
- No FFmpeg / raw-serial / sandbox blockers apply here; every gap above is
  bridge + SwiftUI work, not a platform limit.

## Recommended sequencing

1. **ColorCurve presets (Load/Save/Export) on iPad** — ✅ **done.** Bridge
   trio + `colorCurveXccDocument:` and two sheets
   (`ColorCurvePresetSheets.swift`), directly cloned from the ValueCurve
   preset code; `.xcc` interchanges byte-for-byte with desktop.
2. ~~**Update Palette menu item on iPad**~~ — **landed** (see scorecard row, `ColorPaletteView.swift:95`, `SequencerViewModel.swift:2836`).
3. **ValueCurve export-to-file on iPad** — ✅ **done.** "Export…" +
   `.fileExporter` over the `valueCurveXvcDocument:` bridge
   (`ValueCurveEditor.swift`), routing through core `ValueCurve::SaveXVC`.
4. ~~**Desktop ValueCurve clipboard copy/paste + palette Copy String**~~ —
   **done.** Both ship via `wxClipboard` using the shared `xlvc:v1:` envelope /
   `GetCurrentPalette()` string, making cross-platform curve/palette sharing
   symmetric.
5. Defer the dropper panes, lock buttons, and panel-size/reset preferences —
   low value on a touch UI; revisit only if users ask.
