# Phase B — Effects Grid Parity (Pending Work)

Scope: the sequencer canvas the user spends 90 % of their time in —
the **effects grid**, **timeline ruler**, **waveform**, **row
headings**, and the **timing tracks** strip between them.

All P0/P1 gaps closed. What remains is one named P2 plus 3 deferred.

---

## Open items

### Named P2 — needs new bridge surface

| # | Gap | What's needed |
|---|---|---|
| **B77** | Import Notes (MIDI) | New iOS-side MIDI parser (or AVFoundation `MIDIFile`) + `XLSequenceDocument` bridge that converts note-on events into timing marks on a chosen track. Acceptance: pick a `.mid` from `.fileImporter`, choose target timing track, marks appear at note-on times with note-name labels. **Note:** desktop's 2026.10 fix (#6434) added handling for packed/Type-0 (single-track, merged-channel) `.mid` files — the iPad parser must cover that layout too, not just multi-track Type-1. |

### Deferred — substantial new work, not parity gaps in practice

| # | Gap | Why deferred |
|---|---|---|
| **B16** | Drag-from-palette with live ghost preview | Tap-to-arm + tap-to-place works well on touch. Revisit if users ask for drag-cancel mid-gesture. |
| **B24** | Find Possible Source Effects | Diagnostic-only desktop feature in `EffectsGrid::FindEffectsForData` + `FindDataPanel`. Needs new SwiftUI panel + per-element/strand/node/submodel channel-resolution bridge. Most users never invoke it. |
| **B56 (Convert To Effect)** | Data-row → effects conversion | Desktop's `DoConvertDataRowToEffects` walks rendered `SequenceData` via `RampLenColor` / `isOnLineColor` helpers in `tabSequencer.cpp`. Needs those helpers lifted to core plus per-strand `SingleLineModel` construction. Promote-Node-Effects (the other half of B56) shipped 2026-04-28. |
