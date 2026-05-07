# Phase B — Effects Grid Parity (Pending Work)

Scope: the sequencer canvas the user spends 90 % of their time in —
the **effects grid**, **timeline ruler**, **waveform**, **row
headings**, and the **timing tracks** strip between them.

## Status (2026-05-02)

All P0 and P1 gaps are closed. Phase B and B-Metal closed out the
rebuild and the Metal render pipeline; the multi-select / align /
split / playhead-follow / timing-mark editing / lyric breakdown /
loop-region / waveform-filter / row-heading-expansion / clipboard /
keyboard-editing / scrollbar / find-replace / `.xtiming` I/O work all
landed in the 2026-04-20 → 2026-04-28 sessions. B79 (AI Speech 2
Lyrics) shipped via `XLAIServices.generateLyricTrack` + the unified
Add Timing Track sheet. What remains is a short tail of P2 polish
plus one named open item (B77 MIDI import).

**Current counts:** 0 × P0, 0 × P1, **1 × P2 named**, **3 × Deferred**,
1 × Removed.

B91 LRCLIB synced-lyrics import shipped 2026-05-02 (parser hoisted
to `src-core/lyrics/LRCParser.{h,cpp}`; iPad bridge in
`src-iPad/Bridge/XLLyricsImport.{h,mm}`; SwiftUI tab in
`AddTimingTrackSheet`). Both clients now go through the same
`lrc::ParseLRC` + `lrc::SanitizePhraseText`.

---

## Open items

### Named P2 — needs new bridge surface

| # | Gap | What's needed |
|---|---|---|
| **B77** | Import Notes (MIDI) | New iOS-side MIDI parser (or AVFoundation `MIDIFile`) + `XLSequenceDocument` bridge that converts note-on events into timing marks on a chosen track. Acceptance: pick a `.mid` from `.fileImporter`, choose target timing track, marks appear at note-on times with note-name labels. |

### Deferred — substantial new work, not parity gaps in practice

| # | Gap | Why deferred |
|---|---|---|
| **B16** | Drag-from-palette with live ghost preview | Tap-to-arm + tap-to-place works well on touch. Revisit if users ask for drag-cancel mid-gesture. |
| **B24** | Find Possible Source Effects | Diagnostic-only desktop feature in `EffectsGrid::FindEffectsForData` + `FindDataPanel`. Needs new SwiftUI panel + per-element/strand/node/submodel channel-resolution bridge. Most users never invoke it. |
| **B56 (Convert To Effect)** | Data-row → effects conversion | Desktop's `DoConvertDataRowToEffects` walks rendered `SequenceData` via `RampLenColor` / `isOnLineColor` helpers in `tabSequencer.cpp`. Needs those helpers lifted to core plus per-strand `SingleLineModel` construction. Promote-Node-Effects (the other half of B56) shipped 2026-04-28. |

### Out of Phase B scope

- **B59 — Edit Display Elements.** Phase F.
- **B19 full-storage impl.** Phase C / G12 (`EffectPresetManager`-backed disk store). Session-only stub already shipped.
- **B86 — Breakdown Phoneme.** Not a real desktop feature — `RowHeading.cpp` only has `BreakdownPhrases` (B84) and `BreakdownWords` (B85) menu items. B85 is the deepest breakdown desktop offers and is fully shipped on iPad (`breakdownWordsAtRow`, backed by the phoneme dictionary lazy-loaded by `iPadRenderContext` from the bundled `dictionaries/` resource folder). The Phrases→Words→Phonemes chain works end-to-end.

---

## Out of scope for Phase B (tracked elsewhere)

- Core rendering (complete, verified on-device).
- Model / layout editing — desktop-only.
- Controller output — post-MVP in the top-level plan.
- Sequence-lifecycle plumbing — Phase E (complete).
- App Store submission — Phase H.
- Document / iCloud handling — Phase G.
- Audio-filter enhancements beyond the four standard filters —
  `plans/audio-analysis-enhancements.md`.
