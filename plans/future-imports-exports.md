# Future — Additional Imports & Exports

iPad Phase I-1/I-2 shipped `.xsq` / `.xsqz` import; I-4 (`.sup`)
landed 2026-05-02; I-5 (`.lms`/`.las`) is tracked in
[`phase-i-import-effects.md`](phase-i-import-effects.md).
Papagayo / Audacity / Metronome / Lyrics / `.xtiming` / LOR-timing
import + AutoLabel + BatchRender + Package Sequence all shipped.
This file catalogues the rest.

Source: §2.8 + §2.9 of the 2026-04-23 gap analysis (Phase N).

## Imports (still open)

| # | Format | Severity | Effort |
|---|---|---|---|
| IO-1 | LOR `.lms` / `.las` (timing) | (= I-5) | — |
| IO-2 | LOR `.lms` (full sequence) | (= I-5) | — |
| IO-3 | LOR S5 `.loredit` | P3 | M |
| IO-4 | LOR Pixel Editor `.lpe` | P3 | M |
| IO-5 | Vixen 2 `.vix` | P3 | M |
| IO-6 | **Vixen 3 `.tim`** | P2 | L |
| IO-8 | HLS `.hlsidata` | P3 | M |
| IO-9 | LSP 2 `.msq` | P3 | M |
| IO-10 | VSA `.vsa` | P3 | M |
| IO-14 | **MIDI `.mid`** (NoteImportDialog) — duplicate of B77 in [`phase-b-grid-parity.md`](phase-b-grid-parity.md) | P2 | M |
| IO-15 | Music XML `.xml` / `.mxl` | P3 | M |
| IO-16 | Polyphonic Transcription `.txt` | P3 | S |
| IO-18 | ImportPreviewsModelsDialog (model layout copy from another show) | P3 | M |
| IO-19 | MediaImportOptionsDialog (folder mapping) | P3 | M |
| IO-20 | SeqElementMismatchDialog — full desktop Map-Models flow (alias half shipped) | P3 | M |
| IO-24 | LyricUserDictDialog (custom phoneme dict) | P3 | S |

## Exports (still open)

| # | Item | Severity | Effort |
|---|---|---|---|
| EX-1 | **SeqExportDialog** — 14 format picker: LOR `.lms`/`.las`, LOR Clipboard `.lcb` (v3), LOR S5 Clipboard, Vixen `.vix`, Vixen Routine `.vir`, LSP, HLS `.hlsnc`, xLights/FPP `.fseq`, Compressed `.mp4`, Uncompressed `.mp4`, Uncompressed `.avi`, Lossless `.mov`, Minleon NEC `.bin`, GIF `.gif`. **Tools → Export Models / Export Effects** route per-model and per-sequence variants here; today they each write a `.xlsx` documentation spreadsheet. | P2 | L |
| EX-2 | ExportModelSelect — model picker for per-model export | P2 | S |
| EX-3 | **BatchRenderDialog** — desktop's feature-parity version. iPad's `BatchRenderSheet` now also shows xsq modification + fseq render dates per row (also surfaced in the sequence picker), with sort options (Name / Modified ↓ / Rendered ↓ / Out-of-date first) on the picker and an "Out of date only" filter toggle on the batch sheet. **Remaining:** FPP Playlist integration (selects sequences referenced by a chosen FPP instance — pairs with FPP Connect / EX-4). Force HD is deliberately out: batch render's purpose is to refresh fseqs for show playback, so HD precision should track sequence settings rather than be a per-batch override. | P2 | S |
| EX-9 | TabConvert::Write*ModelFile — Falcon Pi `.bin`, MP4/AVI/MOV (FFmpeg block on iOS), GIF (FFmpeg), Minleon NEC `.bin` | P2 | L–XL (FFmpeg block) |
| EX-12 | **Export Models** spreadsheet — `.xlsx` of every model in the show with channel ranges, controller wiring, strings, smart-remote groupings. **Tools → Export Models** on desktop. | P3 | S |
| EX-13 | **Export Effects** spreadsheet — `.xlsx` of every effect on every model in the current sequence. **Tools → Export Effects** on desktop. | P3 | S |
| EX-14 | **Export Controller Connections** — dump of controller-to-model wiring graph. **Tools → Export Controller Connections** on desktop. | P3 | S |
| EX-15 | **Download Sequences/Lyrics** — vendor-hosted sequence + lyric catalog browser, beyond the vendor-model catalog already shipped via `VendorBrowserSheet`. **Tools → Download Sequences/Lyrics** on desktop. | P3 | M |
| EX-16 | **Generate Lyrics From Data** — `GenerateLyricsDialog`: reverse-engineer phoneme lyric tracks from rendered channel data on a chosen model. Different from B79 (AI Speech 2 Lyrics from audio); useful when an existing sequence already has mouth-shape effects baked in. **Tools → Generate Lyrics From Data** on desktop. | P3 | M |

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

## When to come back

- IO-6 Vixen 3 `.tim` is the biggest single P2 import; pull in
  if a tester or vendor flags it.
- EX-2 ExportModelSelect + per-model `.fseq` export can ship as
  a small standalone PR — engine path is already there.
