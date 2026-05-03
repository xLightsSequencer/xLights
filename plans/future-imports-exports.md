# Future — Additional Imports & Exports

iPad Phase I-1/I-2 shipped `.xsq` / `.xsqz` import; I-4 (`.sup`)
and I-5 (`.lms`/`.las`) are tracked in
[`phase-i-import-effects.md`](phase-i-import-effects.md). This
file catalogues the rest of the import/export surface.

Source: §2.8 + §2.9 of the 2026-04-23 gap analysis (Phase N).

## Already shipped

- IO-7 SuperStar `.sup` import (Phase I-4, 2026-05-02).
- IO-11 `.xsq` / `.xsqz` import (Phase I-2, 2026-04-29).
- IO-17 `.xtiming` round-trip (Phase B-74).
- IO-22 AutoLabelDialog (Phase B-89).
- EX-3 BatchRenderDialog — simplified iPad version landed
  (`BatchRenderSheet.swift` + `BatchRenderRunner.swift`). Desktop's
  feature-parity version (recursive-search columns, last-render-
  date metadata, FPP playlist integration) still open as the
  EX-3 row below.
- EX-10 Whole-sequence `.fseq` emission alongside save.

## In flight (Phase I)

- IO-1 LOR `.lms` / `.las` (timing) — see I-5.
- IO-2 LOR `.lms` (full sequence) — see I-5.

## Imports (still open)

| # | Format | Severity | Effort |
|---|---|---|---|
| IO-3 | LOR S5 `.loredit` | P3 | M |
| IO-4 | LOR Pixel Editor `.lpe` | P3 | M |
| IO-5 | Vixen 2 `.vix` | P3 | M |
| IO-6 | **Vixen 3 `.tim`** | P2 | L |
| IO-8 | HLS `.hlsidata` | P3 | M |
| IO-9 | LSP 2 `.msq` | P3 | M |
| IO-10 | VSA `.vsa` | P3 | M |
| IO-12 | **Papagayo `.pgo`** (lipsync) | P2 | S |
| IO-13 | **Audacity `.txt` labels** | P2 | S |
| IO-14 | **MIDI `.mid`** (NoteImportDialog) — duplicate of B77 in [`phase-b-grid-parity.md`](phase-b-grid-parity.md) | P2 | M |
| IO-15 | Music XML `.xml` / `.mxl` | P3 | M |
| IO-16 | Polyphonic Transcription `.txt` | P3 | S |
| IO-18 | ImportPreviewsModelsDialog (model layout copy from another show) | P3 | M |
| IO-19 | MediaImportOptionsDialog (folder mapping) | P3 | M |
| IO-20 | SeqElementMismatchDialog (resolve missing models on import) | P2 | M |
| IO-21 | MetronomeLabelDialog (auto metronome marks) | P3 | S |
| IO-23 | **LyricsDialog** (text + timing for lipsync) | P2 | S |
| IO-24 | LyricUserDictDialog (custom phoneme dict) | P3 | S |

## Exports (still open)

| # | Item | Severity | Effort |
|---|---|---|---|
| EX-1 | **SeqExportDialog** — 14 format picker: LOR `.lms`/`.las`, LOR Clipboard `.lcb` (v3), LOR S5 Clipboard, Vixen `.vix`, Vixen Routine `.vir`, LSP, HLS `.hlsnc`, xLights/FPP `.fseq`, Compressed `.mp4`, Uncompressed `.mp4`, Uncompressed `.avi`, Lossless `.mov`, Minleon NEC `.bin`, GIF `.gif` | P2 | L |
| EX-2 | ExportModelSelect — model picker for per-model export | P2 | S |
| EX-3 | **BatchRenderDialog** — desktop's feature-parity version: recursive search + sequence checklist + last-render-date columns + Force HD + FPP Playlist integration. (iPad's simpler `BatchRenderSheet` already covers the basic case.) | P2 | M |
| EX-9 | TabConvert::Write*ModelFile — Falcon Pi `.bin`, MP4/AVI/MOV (FFmpeg block on iOS), GIF (FFmpeg), Minleon NEC `.bin` | P2 | L–XL (FFmpeg block) |

## Convert dialog

| # | Item | Severity | Effort |
|---|---|---|---|
| CV-1 | ConvertDialog — universal cross-format converter, 6 inputs / 9 outputs (`.fseq`, `.lms`, `.las`, `.vix`, `.gled`, `.seq`, `.hlsidata` → 9 outputs) with LOR Import Options (Map Empty Channels, Verbose Channel Map, Time Resolution 25/50/100ms) | P3 | XL |
| CV-2 | ConvertLogDialog | P3 | S |

## Hard misses

- **Whole-sequence MP4 / MOV / AVI / GIF export** (EX-9, parts
  of EX-1). FFmpeg isn't a system framework on iOS. Bundling
  is feasible (~50 MB bloat) but invites licensing and App Store
  review risk. AVFoundation video writer would be a separate XL
  effort and only covers MP4/MOV without GIF.

## Why deferred

- Phase I-1/I-2 covers the most common vendor workflow (`.xsq`
  / `.xsqz`); I-4/I-5 cover the next two formats (SuperStar +
  LOR). Beyond that, formats trail off in user prevalence quickly.
- Per-model exports (EX-2 picker on top of EX-1) are simpler to
  port than the whole 14-format SeqExportDialog and would unlock
  the most common "export this one model for FPP" workflow.
- IO-12 Papagayo and IO-13 Audacity are S-effort wins that
  pair well with [`future-audio-authoring.md`](future-audio-authoring.md)
  if/when that lands.

## When to come back

- After Phase I-3/I-4/I-5 ship and we have a measurable signal
  on whether `.xsq`/`.xsqz`/`.sup`/`.lms` cover ~95% of vendor
  imports. If so, the rest can wait.
- IO-6 Vixen 3 `.tim` is the biggest single P2 import; pull in
  if a tester or vendor flags it.
- EX-2 ExportModelSelect + per-model `.fseq` export can ship as
  a small standalone PR — engine path is already there.
