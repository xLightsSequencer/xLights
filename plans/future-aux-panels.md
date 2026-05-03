# Future — Auxiliary Sequencer Panels & Bulk-Edit

Eight desktop sequencer panels that live alongside the grid +
inspector and aren't yet on iPad, plus the seven generic
bulk-edit dialogs and the long tail of right-click context menus
that haven't been ported.

Source: §2.19 + §2.20 of the 2026-04-23 gap analysis (Phase L).

## Status

**Partial.** B97 Find / Replace landed inside the grid; AP-9
(BufferPanel), AP-12 (ValueCurvesPanel), and AP-13 (BlendingPanel)
landed as part of the Phase C inspector. Everything else here is
desktop-only.

## Already shipped (verified 2026-05-02)

- **AP-9 BufferPanel** — Buffer tab in `EffectSettingsView.swift`
  with per-layer buffer-style controls.
- **AP-12 ValueCurvesPanel** — `ValueCurveEditor.swift` +
  `ValueCurvePresetSheets.swift` + `XLValueCurve` bridge cover
  custom point editing + preset save/load.
- **AP-13 BlendingPanel** — `BlendingPanelViews.swift`
  (LayerMorphRow, LayerMethodRow, Canvas, In/Out transitions)
  exposed via the Blending tab in `EffectSettingsView.swift`.

## Auxiliary panels (still open)

| # | Panel | Severity | Effort |
|---|---|---|---|
| AP-1 | **SearchPanel** — full-text search across effects + timing labels | P1 | M |
| AP-2 | FindDataPanel — search by attribute (brightness / hue / channel value); covers B24 | P2 | M |
| AP-3 | SelectPanel — filter sequencer rows by effect type / name | P2 | S |
| AP-4 | **EffectTreeDialog / EffectListDialog** — preset tree, drag-to-apply, manage / search / import / export presets, GIF preview animation. Couples with [`future-effect-presets.md`](future-effect-presets.md) (the disk-persistent half). | P1 | M |
| AP-5 | JukeboxPanel — custom button grid for jump-to-time, with `LinkJukeboxButtonDialog` wiring | P2 | M |
| AP-6 | TopEffectsPanel — quick-access toolbar of recently-used effects | P3 | S |
| AP-7 | PerspectivesPanel | — | Skip on iPad (single-window layout) |
| AP-8 | SequenceVideoPanel — embedded reference video synced to playhead | P2 | M |
| AP-10 | EffectAssist sub-panels | (= EA-* in `future-pictures-frame-editor.md` + future-sketch-assist work) | XL each |
| AP-11 | ColoursPanel / ColourReplaceDialog / ColorPanel parity | P2 | M |

## Bulk-edit dialogs

| # | Dialog | Use | Severity | Effort |
|---|---|---|---|---|
| BE-1 | BulkEditColourPickerDialog | colour for N effects | P2 | S |
| BE-2 | BulkEditComboDialog | dropdown property for N | P2 | S |
| BE-3 | BulkEditFontPickerDialog | font for N text effects | P2 | S |
| BE-4 | BulkEditSliderDialog | numeric / value-curve for N | P2 | M |
| BE-5 | AlignmentDialog | model alignment 3×3 anchor | (= LA-8 in [`future-layout-editing.md`](future-layout-editing.md)) | M |
| BE-6 | ColourReplaceDialog | find/replace colour across N effects | P2 | M |
| BE-7 | EditAliasesDialog | model alias management | (= MA-12) | S |

## Right-click context menus

The 2026-04-20 parity audit covered the high-traffic Sequencer
menus (effect-cell, timing track, row header, ruler, waveform,
Display Elements). Open gaps in **non-Sequencer surfaces** —
worth a sweep before this phase wraps:

- Layout-canvas (model / empty / model group right-clicks). Lands
  with [`future-layout-editing.md`](future-layout-editing.md).
- Layout model tree — sort submenu has 11 options.
- Layout model-group editor.
- View Objects tree.
- Preview window.
- Effect tree.
- Color panel.

## Why deferred

- The Sequencer canvas already covers ~95% of authoring ops; the
  panels here are mostly secondary navigation/discovery.
- AP-1 SearchPanel is the highest-value piece — large sequences
  are tedious to navigate without it. The rest are quality-of-life.
- AP-4 EffectTreeDialog is the natural pair to disk-persistent
  presets ([`future-effect-presets.md`](future-effect-presets.md));
  ship them together when one or the other is scheduled.

## When to come back

- After Phase H. AP-1 + AP-4 are the two items most likely to be
  pulled into the testing window if testers report navigation
  pain on large sequences.
