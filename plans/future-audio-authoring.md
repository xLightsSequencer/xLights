# Future — Audio-Driven Authoring

Audio-analysis features that turn the waveform from a *display*
into an *authoring* surface. Engine pieces are partly in place;
the gap is mostly UI wiring.

Source: §2.13 of the 2026-04-23 gap analysis (Phase J).

## What's already in place

- Waveform with the four standard filter modes (LUFS, VOCALS,
  NONVOCALS, STEM_*) and the 2026-04-22 alt-track switch (B43).
- Audio scrub during ruler drag (B40).
- VAMP analysis is *unreachable* on iPad — `vamp-hostsdk` is
  desktop-only and there's no iOS replacement.
- B79 "AI Speech 2 Lyrics" tracked separately in
  [`phase-b-grid-parity.md`](phase-b-grid-parity.md) — it's
  effectively the speech-recognition slice of this surface.

## Already shipped (verified 2026-05-02)

- **A-1** — Generate timing track from audio onsets.
  `XLSequenceDocument.detectOnsets(sensitivity:)` →
  `SequencerViewModel.generateTimingTrackFromOnsets(name:)` →
  `AddTimingTrackSheet.commitAudioOnsets`. Wired into the unified
  Add Timing Track sheet alongside metronome / FPP / tempo.
- **A-2** — Generate timing track from beats / tempo.
  `XLSequenceDocument.detectTempo` → `SequencerViewModel.detectTempo`
  → `AddTimingTrackSheet.commitAudioTempo`. Same sheet entry.
- **A-3** — Onset markers overlaid on waveform.
  `TopChromeMetalGridView` draws marks when `showOnsets` is true;
  cached `onsetTimesMS` populated from `detectOnsets`.
- **A-8** — Incompatible-video warning at sequence load. Done as
  part of the TestFlight-quality sweep; see iPad-xLights-Plan.md.
- **A-9** — Spectrogram view. `SequencerGridV2View` exposes a
  toggle (`toggleShowSpectrogram`) on the waveform context menu.
- **A-10** — Pitch-contour view. `SequencerGridV2View.toggleShowPitchContour`.
- **A-11** — Show-onsets toggle.
  `SequencerGridV2View.toggleShowOnsets`.

## Gap (still open)

_None._ All P1/P2 audio-authoring items have shipped on iPad. The
remaining "hard misses" below are blocked on infrastructure, not
priority.

## Phantom items

- **A-12** "Music Generator (procedural)" — confirmed 2026-05-02
  not a real desktop feature. Searched `src-ui-wx/` + `src-core/`
  for `Music Generator` / `MusicGen` / `GenerateMusic` /
  `MusicMaker` / `ProceduralTiming` — no matches. The 2026-04-23
  gap analysis flagged it speculatively from a B79 cross-reference
  ("likely doesn't exist on desktop") and that caveat was correct.

## Hard misses (no iOS path)

- **A-7** Polyphonic transcription. VAMP-only on desktop; no
  iOS-native equivalent. CoreML-based replacement is XXL and
  doesn't cover user-installed VAMP plugins.
- **A-13** VAMPPluginDialog. Same blocker.

## Tracking — desktop work in flight

- **A-14 Stem separation via ONNX Runtime / OpenVINO**
  (desktop, started 2026-04-26, `0ccb37b25`). Desktop is
  experimenting with ONNX-runtime-based stem separation as a
  replacement for the existing HTDemucs path. iOS has both
  CoreML and ONNX Runtime available. Wait until the desktop
  side stabilises before committing to an iPad bridge — the
  model-format choice (ONNX vs CoreML conversion) drives the
  iPad work materially.

## When to come back

- Only if the iOS audio analysis story changes — e.g. a CoreML-
  based VAMP-equivalent that would unblock A-7 / A-13 — or if a
  new desktop authoring feature lands and needs a port.
