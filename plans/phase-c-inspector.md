# Phase C — Effect Settings Inspector (Pending Work)

Scope: the four panels next to a selected effect — **Effect settings**,
**Color**, **Buffer**, **Blending**. iPad today round-trips every
primitive control type, all ~40 custom rows desktop ships, the full
visibility-rule engine, the value-curve editor (including shader
preview via the render engine), the palette ColorCurve editor, the
media picker with in-sequence reuse + thumbnails, folder grouping,
and the per-property context menu. Items tracked below are what's
still missing.

---

## TL;DR

Five buckets, ordered by user-visible impact:

1. **Multi-effect operations** — bulk-edit, lock + randomize. Grid
   supports multi-select; inspector doesn't yet consume it.
2. **Sequence-wide media manager** — per-effect picker handles reuse;
   still no standalone view for embed / extract / rename / remove-
   unused / video compat.
3. **Specialised editors** — Moving Head, Sketch path, Morph line,
   DMX Remap / Save State / Load State.

Plus a couple of device-verification items and two gaps that are
already tracked under Phase F (tear-out, keyboard shortcuts).

Related: [`future-effect-presets.md`](future-effect-presets.md)
(G12 — deferred).

---

## 1. Inspector scaffolding

- **G1 — Tear-out / docking of tabs into separate windows.** Deferred
  to Phase F (`plans/phase-f-window-system.md`). Not a correctness
  gap; noted here so it isn't lost.
- **G2-c — Shader dynamic uniform grouping for large `.fs` files.**
  Most shaders declare < 10 uniforms so grouping isn't needed. Pack
  shaders with 20+ uniforms turn into a flat scroll. If
  `GLSL_GROUP:` comment conventions exist in desktop's shader
  parser, respect them in
  `ShaderConfig::GetDynamicPropertiesJson()` so grouping carries
  across. Deferred until a real shader pack trips the issue. P2.

---

## 2. Effect settings tab

### Specialised editors (desktop-authored data iPad can render but not edit)

- **G3 — Moving Head fixture editor.** Desktop's one non-JSON panel:
  hand-built in wxSmith for DMX fixture mapping, pan / tilt /
  colour wheels, position curves. ~30+ controls. P1.
- **G4 — Sketch path editor.** `SketchInfoRowView` /
  `SketchDefRowView` / `SketchBackgroundRowView` read the encoded
  sketch definition but don't offer a polyline editor. P1.
- **G5 — Morph line editor.** Desktop's `xlGridCanvasMorph` — drag
  start / end line endpoints on a 100×100 grid. iPad has QuickSet
  presets + Swap but no direct line editing. P1.
- **G8 — DMX Remap / Save State / Load State buttons.** Rendered
  disabled today. Depends on model-state read / write bridge paths
  not yet on iPad. P1.

### Per-property & effect-level actions

- **G10 — Per-property Lock + Randomize.** `EffectMetadata.swift`
  parses the `lockable` flag but no lock UI. No per-control
  Randomize menu entry, no top-level "Randomize Effect" button.
  Desktop's lock is in-memory session state (not persisted) so
  iPad should match. P1 (low priority — uncertain whether the
  randomize UX even belongs on iPad).
- **G11 — Bulk Edit.** Desktop's `SetSupportsBulkEdit(true)`
  controls get an "Apply to all selected" context menu entry. iPad
  has no such entry and no multi-effect carry-over to the inspector.
  Needs G41 to land first (grid→inspector multi-selection). P1.
- **G14 — "Update all like this" batch update.** Desktop's top-bar
  "Update" writes the current panel values across all selected.
  Falls out of G11. P2.

### Device verification

Code-side audits complete (2026-04-20); the inspector renders the
controls as expected. Need a device pass to confirm no runtime
surprises before these close:

- **G19 — HSV adjustment sliders on the Colors tab.** Five sliders
  (Brightness, Contrast, Hue/Sat/Value Adjust). Four have VC pills;
  Contrast doesn't (matches desktop).
- **G25 — `SuppressEffectUntil` / `FreezeEffectAtFrame` on the
  Blending tab.** Spin 0..999999 with `suppressIfDefault`.
  `EditableNumberField` handles the typed input.

---

## 3. Color panel

- ~~**G17 — Palette save / load / import / export.**~~ **Landed
  2026-04-20.** Palette-header overflow menu (Save / Save As… /
  Load Saved… / Import from Text… / Copy Palette String). New
  bridge: `savedPalettes`, `savePaletteString(_:asName:)`,
  `deleteSavedPalette`, `currentPaletteString(forRow:at:)`,
  `applyPaletteString(_:toRow:at:)`. On-disk format is
  byte-for-byte compatible with desktop's
  `ColorPanel::GetCurrentPalette` / `LoadColorsToButtons` —
  `.xpalette` files written under `<showFolder>/Palettes/` round-
  trip both ways. Load sheet (`PaletteLoadSheet`) shows an 8-
  swatch preview per saved palette with swipe-to-delete;
  ColorCurve slots render as a `~` placeholder so we don't run
  the gradient sampler in list rows. Import dialog validates
  input via `PaletteSerializer.isValidPalette` and pre-populates
  from `UIPasteboard` if the clipboard looks palette-shaped.
  Palette Shift-left / Shift-right / Reverse dropped; AI Generate
  deferred to
  [`future-ai-palette-generate.md`](future-ai-palette-generate.md).

---

## 4. Value Curves

All C6 items landed 2026-04-20:

- ~~**G36 — Preset load / save from disk.**~~ **Landed.** Load
  Preset… / Save As Preset… in the VC editor sheet. Bridge methods
  `savedValueCurves`, `saveValueCurveSerialised(_:asName:)`,
  `deleteSavedValueCurve` use core's `LoadXVC` / `SaveXVC` so files
  are byte-for-byte compatible with desktop's `.xvc` format. Scans
  `<showFolder>/valuecurves/` + bundled `valuecurves/` in
  resources; dedupes by serialised body. Load list shows a thumbnail
  + swipe-to-delete.
- ~~**G37 — VC copy / paste.**~~ **Landed.** Copy / Paste buttons
  in the VC editor; "Copy Value Curve" / "Paste Value Curve" entries
  in the per-property context menu (appear only when the property
  has `valueCurve: true` and, for Copy, a stored curve is active).
  Clipboard uses an `xlvc:v1:` prefix so arbitrary text pastes don't
  land as garbage on curve controls.
- ~~**G38 — Reverse / Flip transforms.**~~ **Landed.** Desktop has
  two transforms on the curve (Reverse = mirror across X, Flip =
  mirror across Y) — scope was originally mis-stated as Flip /
  Mirror / Repeat. Buttons exposed in the VC editor; back-end wraps
  core's existing `Reverse()` / `Flip()`.

---

## 5. Media picker and sequence-wide media management

The per-effect picker is done (in-sequence reuse list, folder
grouping, thumbnails with GIF / video / shader frame cycling,
current-selection highlight). What remains is the standalone
sequence-wide media manager.

- **G28 — No sequence-wide media manager view.** Build an iPad
  equivalent of `ManageMediaPanel` — full list of sequence media
  with per-file preview and status (Embedded / External / broken).
  Layered on the bridge that already serves the per-effect picker. P1.
- **G29 — No embed / extract UI.** `SequenceMedia` on iPad already
  round-trips embedded media correctly; no way to turn embedding
  on / off from iPad. P1.
- **G30 — No rename-embedded-with-reference-update.** Desktop's
  `EmbedWithRename()` / `ExtractWithRename()` walk all effects
  updating settings when an embedded filename changes. P2.
- **G31 — No "Remove unused media" action.** Sequences heavily
  edited on desktop may carry orphaned payload indefinitely. P2.
- **G32 — No video compat check / transcode.** Desktop's
  `MaybeConvertIncompatibleVideo()` catches AVFoundation-
  incompatible containers at pick time and prompts to transcode
  .avi / .mkv → .mov. iPad accepts silently; effect fails to render
  later with no clear error path. P1.
- **G33 — No AI image generation entry point.** Desktop has it in
  both `PicturesPanel` and `ManageMediaPanel`. Depends on an iOS
  bridge for the AI service. P2.

---

## 6. Cross-cutting

- **G40 — Drag / drop in inspector.** Desktop supports drag-drop
  across palette slots and between controls. iPad has none. P2.
- **G41 — Multi-effect selection → inspector.** Grid supports
  multi-select; inspector doesn't materialise anything when
  multiple effects are selected. Desktop shows the first effect's
  panel and enables "Apply to all" context actions. iPad model
  decision: "3 effects selected" chrome vs. inspector bulk mode.
  Prerequisite for G11 / G14. P1.

---

## 7. Deferred to other phases

- **G1 — Tab tear-out / multi-window.** Phase F
  (`plans/phase-f-window-system.md`). P2.
- **G15 — Keyboard shortcuts in the inspector.** Rolls in with
  Phase F's app-level menu bar + discoverable shortcuts. P2.

---

## 8. Severity summary (pending only)

| # | Gap | Area | Severity |
|---|---|---|---|
| G3  | Moving Head fixture editor | Effect | P1 |
| G4  | Sketch path editor | Effect | P1 |
| G5  | Morph line editor | Effect | P1 |
| G8  | DMX Remap / Save State / Load State dialogs | Effect | P1 |
| G10 | Per-property Lock + Randomize | Effect | P1 |
| G11 | Bulk Edit (apply to N selected effects) | Effect | P1 |
| G14 | "Update all like this" batch update | Effect | P2 |
| ~~G17~~ | ~~Palette save / load / import / export~~ | ~~Color~~ | ~~landed~~ |
| G19 | Device-verify HSV adjustment sliders | Color | P1 |
| G25 | Device-verify Suppress / Freeze spins | Blending | P1 |
| G28 | Sequence-wide media manager view | Media | P1 |
| G29 | Embed / extract UI | Media | P1 |
| G30 | Rename-embedded with reference update | Media | P2 |
| G31 | "Remove unused media" action | Media | P2 |
| G32 | Video compat check / transcode | Media | P1 |
| G33 | AI image generation entry point | Media | P2 |
| ~~G36~~ | ~~Value-curve preset load / save from disk~~ | ~~VC~~ | ~~landed~~ |
| ~~G37~~ | ~~Value-curve copy / paste~~ | ~~VC~~ | ~~landed~~ |
| ~~G38~~ | ~~Value-curve Reverse / Flip transforms~~ | ~~VC~~ | ~~landed~~ |
| G40 | Drag / drop in inspector | Cross-cutting | P2 |
| G41 | Multi-effect selection in inspector | Cross-cutting | P1 |
| G1  | Tab tear-out / multi-window | Scaffolding | P2 (Phase F) |
| G2-c | Shader uniform grouping for large .fs | Scaffolding | P2 |
| G15 | Keyboard shortcuts in inspector | Effect | P2 (Phase F) |

---

## 9. Suggested phasing

**C3 — Colour workflow polish**
- ~~G17~~ **landed** — palette save / load / import / export via
  named `.xpalette` files.

**C4 — Multi-effect operations**
- G41: wire grid multi-select into inspector.
- G11: "Apply to all selected" per-control.
- G14: "Update all like this" as a consequence of G11.
- G10: Lock + single-control Randomize in the context menu; top-
  level "Randomize effect" button.

**C5 — Sequence-wide media manager**
- G28: standalone ManageMediaPanel equivalent.
- G29: Embed / Extract buttons.
- G30: Rename-embedded with reference update.
- G31: Remove unused media.
- G32: Video compat check on pick (shared
  `MaybeConvertIncompatibleVideo` path).

**C6 — Value-curve polish** ✓ complete (G36 / G37 / G38)

**C7 — Specialised editors (bigger chunks — one per session)**
- G3: Moving Head.
- G4: Sketch path editor.
- G5: Morph line editor.
- G8: DMX dialog ports.
- G33: AI image generation.

**Ship order recommendation:** C3 first (self-contained, leverages
G16 work), then C4 (biggest workflow lift once multi-select lands),
then C5 (layered on existing media bridge). C6 / C7 as time allows.

---

## 10. Out of scope

- Core renderer parity (already verified on device).
- Rendering performance / JobPool tuning (main plan).
- Sequence-lifecycle / save / sequence-settings UI (Phase E).
- Controller output, iCloud document handling, App Store submission
  (Phases G / H).
- Model / layout editing (separate surface).
- Effect presets — deferred to
  [`future-effect-presets.md`](future-effect-presets.md).
- Pictures frame / GIF timing editor — deferred to
  [`future-pictures-frame-editor.md`](future-pictures-frame-editor.md).
- Palette Shift-left / Shift-right / Reverse — dropped (low value).
- Drag colours between palette slots — deferred to
  [`future-palette-drag.md`](future-palette-drag.md).
- AI palette generation — deferred to
  [`future-ai-palette-generate.md`](future-ai-palette-generate.md).
