# Future — Auxiliary Sequencer Panels & Bulk-Edit

Eight desktop sequencer panels alongside the grid + inspector that
aren't yet on iPad, plus the seven generic bulk-edit dialogs and
the long tail of right-click context menus.

Source: §2.19 + §2.20 of the 2026-04-23 gap analysis (Phase L).

## Auxiliary panels

| # | Panel | Severity | Effort |
|---|---|---|---|
| AP-1 | **SearchPanel** — full-text search across effects + timing labels | P1 | M |
| AP-2 | FindDataPanel — search by attribute (brightness / hue / channel value); covers B24 | P2 | M |
| AP-3 | SelectPanel — filter sequencer rows by effect type / name | P2 | S |
| AP-4 | **EffectTreeDialog / EffectListDialog** — preset tree, drag-to-apply, manage / search / import / export presets, GIF preview animation. Couples with [`future-effect-presets.md`](future-effect-presets.md). | P1 | M |
| AP-5 | JukeboxPanel — custom button grid for jump-to-time, with `LinkJukeboxButtonDialog` wiring | P2 | M |
| AP-6 | TopEffectsPanel — quick-access toolbar of recently-used effects | P3 | S |
| AP-7 | PerspectivesPanel | — | Skip on iPad (single-window layout) |
| AP-8 | SequenceVideoPanel — embedded reference video synced to playhead | P2 | M |
| AP-10 | EffectAssist sub-panels | (= EA-* in `future-pictures-frame-editor.md`) | XL each |
| AP-11 | ColourReplaceDialog (standalone find/replace colour) — compound-widget rows shipped; standalone dialog still open | P2 | M |

## Bulk-edit dialogs

Per-property "Apply to all selected" in `PropertyContextMenu`
covers the common bulk-edit case. The multi-row dialogs below
remain desktop-only.

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

High-traffic Sequencer menus shipped 2026-04-20. Open gaps in
**non-Sequencer surfaces**:

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
  are tedious to navigate without it.
- AP-4 EffectTreeDialog is the natural pair to disk-persistent
  presets ([`future-effect-presets.md`](future-effect-presets.md));
  ship them together.

## When to come back

- AP-1 + AP-4 are the two items most likely to be pulled forward
  if testers report navigation pain on large sequences.
