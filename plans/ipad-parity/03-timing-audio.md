# 03 · Timing Tracks & Audio

> **Status:** This theme is **near full parity**. The iPad app implements the entire waveform analysis surface: every filter band (Raw/Bass/Treble/Alto/Non-Vocals/Custom/LUFS/Vocals), on-device HTDemucs stem separation, SoundAnalysis classification, spectrogram view, pitch contour, onset overlay, double-height toggle, and an alternate-audio-track switcher — all in `SequencerGridV2View`'s waveform `confirmationDialog`. Timing-track creation (`AddTimingTrackSheet`) covers Empty / fixed 25-50-100 / custom-interval / metronome (+random +tags) / FPP Commands / FPP Effects / Audio Onsets / Tempo / Chords / AI Lyrics / LRCLIB — matching the desktop `NewTimingDialog`. Mark operations (add, split-at-playhead, merge-with-next, move/resize-edge drag, delete, rename, breakdown phrases/words, remove words/phonemes, -shimmer toggle, auto-label, halve, divide, subdivide, select-all) are all present on iPad, and almost every one has a desktop equivalent (the desktop mark ops live in `EffectsGrid::rightClick`, not just `RowHeading`). Playback speed (all 8 rates), volume, tags (set/go-to/clear), zoom-to-fit/selection, and hide/show-all-timing reach parity. The **real iPad gaps** are: VAMP Queen-Mary plugins (no FFmpeg/VAMP host on iPad), **Import Notes** (MIDI / MusicXML / Audacity / Polyphonic-Transcription → note-labelled timing track), **Prepare Audio** (Reaper/xAudio), **Shift Effects**, **Shift Selected Effects** & **Shift Effects And Timing**, several **preferences** (snap-to-timing-marks, alternate timing format, small waveform, timing-play-on-double-click toggle, exclude-audio-from-package), and minor tag affordances (Next/Prior tag nav, per-tag delete). A couple of iPad touch idioms (double-tap-mark to loop-play, Pencil-precision edge grab) have no desktop equivalent but are additive, not gaps.

## Parity scorecard

| Feature | Surface | Desktop | iPad | Gap | Priority | Ease | Feasibility | Notes |
|---|---|---|---|---|---|---|---|---|
| Create timing track — Empty | dialog | ✅ | ✅ | parity | P1 | easy | feasible | `NewTimingDialog`/`RowHeading.cpp:1162` ↔ `AddTimingTrackSheet.swift:132` |
| Create timing track — fixed 25/50/100 ms | dialog | ✅ | ✅ | parity | P1 | easy | feasible | `NewTimingDialog.cpp:41-43` ↔ `AddTimingTrackSheet.swift:133-135`; iPad gates 25/50 on frame rate too |
| Create timing track — fixed custom interval | dialog | ✅ | ✅ | parity | P1 | easy | feasible | `AddTimingTrackSheet.swift:136,189`; both round to frame |
| Create metronome | dialog | ✅ | ✅ | parity | P1 | easy | feasible | `NewTimingDialog.cpp:44` ↔ `AddTimingTrackSheet.swift:137,209` |
| Metronome — random interval range | dialog | ✅ | ✅ | parity | P2 | easy | feasible | `AddTimingTrackSheet.swift:220-234` |
| Metronome — cycling tag labels (+randomize) | dialog | ✅ | ✅ | parity | P2 | easy | feasible | desktop "Metronome w/ Tags" `NewTimingDialog.cpp:45` ↔ `AddTimingTrackSheet.swift:236-250` |
| Create FPP Commands track | dialog | ✅ | ✅ | parity | P2 | easy | feasible | `NewTimingDialog.cpp:46`, `RowHeading.cpp:1132` ↔ `AddTimingTrackSheet.swift:138,504` |
| Create FPP Effects track | dialog | ✅ | ✅ | parity | P2 | easy | feasible | `NewTimingDialog.cpp:47` ↔ `AddTimingTrackSheet.swift:139,506` |
| Generate timing from Audio Onsets | dialog | ✅ | ✅ | parity | P1 | easy | feasible | `RowHeading.cpp:1074,1421` ↔ `AddTimingTrackSheet.swift:141,554` (shared `OnsetDetector`) |
| Generate timing from Audio Tempo | dialog | ✅ | ✅ | parity | P2 | easy | feasible | `RowHeading.cpp:1075,1534` ↔ `AddTimingTrackSheet.swift:142,565` |
| Generate timing from Audio Chords | dialog | ✅ | ✅ | parity | P2 | easy | feasible | `RowHeading.cpp:1076,1469` ↔ `AddTimingTrackSheet.swift:143,598` |
| AI Speech-to-Lyrics (on-device) | dialog | ✅ | ✅ | parity | P1 | medium | feasible | `RowHeading.cpp:636,1083` ↔ `AddTimingTrackSheet.swift:145,623`; both gate on a SPEECH2TEXT service |
| Search lyrics online (LRCLIB) | dialog | ✅ | ✅ | parity | P2 | medium | feasible | `RowHeading.cpp:641` ↔ `AddTimingTrackSheet.swift:148,283` (integrated search/preview UI) |
| VAMP Queen-Mary plugin analyzers | dialog | ✅ | ❌ | ipad-missing | P3 | hard | infeasible | `RowHeading.cpp:1090-1092`; no VAMP host / FFmpeg on iPad. Built-in detectors cover the common cases |
| Rename timing track | context-menu | ✅ | ✅ | parity | P1 | easy | feasible | `RowHeading.cpp:621` ↔ `RowHeaderViews.swift:159,271` |
| Delete timing track | context-menu | ✅ | ✅ | parity | P1 | easy | feasible | `RowHeading.cpp:622` ↔ `RowHeaderViews.swift:266` |
| Export timing track (.xtiming/.pgo) | context-menu | ✅ | ✅ | parity | P2 | easy | feasible | `RowHeading.cpp:624,1607` ↔ `RowHeaderViews.swift:219` + bulk in `SequenceSettingsSheet` Timings tab |
| Import timing track (xtiming/LOR/PGO/SRT/Vixen3/LSP/xsq…) | dialog | ✅ | ✅ | parity | P2 | medium | feasible | `RowHeading.cpp:623` → `ImportTimingElement` ↔ `SequenceSettingsSheet.swift:283-356` (more formats, multi-select picker) |
| Make fixed timing track variable | context-menu | ✅ | ✅ | parity | P2 | easy | feasible | `RowHeading.cpp:625` ↔ `RowHeaderViews.swift:197` |
| Hide / Show all timing tracks | menu | ✅ | ✅ | parity | P3 | easy | feasible | `RowHeading.cpp:512,627` ↔ `SequencerGridV2View.swift:1351-1357` (B81) |
| Add timing tracks to all views | menu | ✅ | ✅ | parity | P2 | medium | feasible | `RowHeading.cpp:629` ↔ `SequencerGridV2View.swift:1361` (B82) |
| Select all timing marks on row | context-menu | ✅ | ✅ | parity | P2 | easy | feasible | `RowHeading.cpp:630` ↔ `RowHeaderViews.swift:261` |
| Generate subdivided timing tracks (½⅓¼⅙⅛ / 2×4×8×) | dialog | ✅ | ✅ | parity | P2 | medium | feasible | `RowHeading.cpp:631,1307` (`SubdivisionOptionsDialog` split+combine) ↔ `RowHeaderViews.swift:203-217` |
| Import Notes (MIDI / MusicXML / Audacity / Polyphonic) | dialog | ✅ | ❌ | ipad-missing | P3 | medium | hard | `RowHeading.cpp:632` → `ExecuteImportNotes` `tabSequencer.cpp:4024`; needs `NoteImportDialog` + loaders bridge; Polyphonic path is VAMP (infeasible on iPad) |
| Auto-Label marks | context-menu | ✅ | ✅ | parity | P2 | medium | feasible | desktop `EffectsGrid.cpp:516,1107` (`AutoLabelDialog`) ↔ `RowHeaderViews.swift:237`, `SequencerViewModel.swift:3769` |
| Halve / Divide marks (÷2…÷8) | context-menu | ✅ | ✅ | parity | P2 | medium | feasible | desktop "Divide Timings" `EffectsGrid.cpp:514,1218` (number prompt) ↔ iPad Halve `RowHeaderViews.swift:243` + Divide `:249` |
| Import lyrics (inline text) | context-menu | ✅ | ✅ | parity | P2 | medium | feasible | `RowHeading.cpp:640` (`LyricsDialog`) ↔ `RowHeaderViews.swift:231` |
| Breakdown phrases | context-menu | ✅ | ✅ | parity | P2 | medium | feasible | `RowHeading.cpp:644`, `EffectsGrid.cpp:504` ↔ `RowHeaderViews.swift:163` (shared `LyricBreakdown`) |
| Breakdown words | context-menu | ✅ | ✅ | parity | P2 | medium | feasible | `RowHeading.cpp:646`, `EffectsGrid.cpp:509` ↔ `RowHeaderViews.swift:169` |
| Breakdown single phrase/word (per-mark) | context-menu | ✅ | ✅ | parity | P3 | easy | feasible | desktop `EffectsGrid.cpp:504-511` ↔ iPad per-mark `SequencerGridV2View.swift:743` |
| Remove words layer | context-menu | ✅ | ✅ | parity | P3 | easy | feasible | `RowHeading.cpp:649` ↔ `RowHeaderViews.swift:175` |
| Remove phonemes / words+phonemes | context-menu | ✅ | ✅ | parity | P3 | easy | feasible | `RowHeading.cpp:653,656` ↔ `RowHeaderViews.swift:176-195` |
| Add "-shimmer" / Remove "-shimmer" on mark | context-menu | ✅ | ✅ | parity | P3 | easy | feasible | desktop phoneme-layer only `EffectsGrid.cpp:519` ↔ iPad any-mark `SequencerGridV2View.swift:721` |
| Create timing from effects (model) | menu | ✅ | ✅ | parity | P2 | medium | feasible | `RowHeading.cpp:602` / `EffectsGrid.cpp:386` ↔ `SequencerGridV2View.swift:1309`, `SequencerViewModel.swift:4747` |
| Add timing mark (tap/long-press to place) | gesture | ✅ | ✅ | parity | P1 | easy | feasible | desktop click in band ↔ `TimingEffectsMetalGridView.swift:23`, `SequencerViewModel.swift:3612` |
| Split timing mark (at point / playhead) | gesture/menu | ✅ | ✅ | parity | P2 | medium | feasible | desktop split ↔ iPad "Split at Play Marker" `SequencerGridV2View.swift:728`, `SequencerViewModel.swift:3634` |
| Merge timing mark with next | context-menu | 🟡 | ✅ | parity | P2 | medium | feasible | desktop "combine" via SubdivisionOptionsDialog 2×/4×/8× (`RowHeading.cpp:84`); iPad has explicit per-mark "Merge with Next" `SequencerViewModel.swift:3662`. Desktop lacks a single-pair merge button |
| Move timing mark (drag) | gesture | ✅ | ✅ | parity | P1 | medium | feasible | desktop effect-drag ↔ `TimingEffectsMetalGridView.swift:26,87` (kind=move) |
| Resize timing mark edge (L/R drag) | gesture | ✅ | ✅ | parity | P1 | medium | feasible | desktop drags effect edges ↔ iPad `MarkDragKind.resizeLeft/Right` `TimingEffectsMetalGridView.swift:87` |
| Rename / edit mark label | context-menu | ✅ | ✅ | parity | P2 | easy | feasible | desktop via `LyricsDialog`/inline ↔ iPad "Rename Mark" `SequencerGridV2View.swift:709` |
| Edit mark times (start/end) | dialog | ✅ | ✅ | parity | P2 | easy | feasible | desktop drag/numeric ↔ iPad "Edit Timing" alert `SequencerGridV2View.swift:765` (seconds) |
| Delete timing mark | context-menu | ✅ | ✅ | parity | P1 | easy | feasible | `EffectsGrid` delete ↔ `SequencerGridV2View.swift:750`, `SequencerViewModel.swift:4306` |
| Double-tap mark → loop-play | gesture | ❌ | ✅ | desktop-missing | P3 | easy | feasible | iPad-only touch idiom `TimingEffectsMetalGridView.swift:31`, `SequencerViewModel.swift:2001`; desktop's `mTimingPlayOnDClick` is a generic play-on-dclick pref, not mark-range loop |
| Apple Pencil precision edge grab | gesture | ❌ | ✅ | desktop-missing | P3 | n/a | infeasible | iPad-only; halves edge slop `TimingEffectsMetalGridView.swift:82` |
| Waveform — Raw / Full Range | panel | ✅ | ✅ | parity | P1 | medium | feasible | `Waveform.cpp:261` ↔ `WaveformFilter.raw` `SequencerViewModel.swift:180` |
| Waveform filter — Bass | context-menu | ✅ | ✅ | parity | P2 | medium | feasible | `Waveform.cpp:262` ↔ `WaveformFilter.bass` |
| Waveform filter — Treble | context-menu | ✅ | ✅ | parity | P2 | medium | feasible | `Waveform.cpp:263` ↔ `WaveformFilter.treble` `SequencerViewModel.swift:180` |
| Waveform filter — Alto | context-menu | ✅ | ✅ | parity | P2 | medium | feasible | `Waveform.cpp:264` ↔ `WaveformFilter.alto` |
| Waveform filter — Custom band | context-menu | ✅ | ✅ | parity | P2 | medium | feasible | `Waveform.cpp:265` ↔ iPad custom-band sheet (MIDI-note bounds) `SequencerGridV2View.swift:1032` |
| Waveform filter — Non-Vocals | context-menu | ✅ | ✅ | parity | P2 | medium | feasible | `Waveform.cpp:266` ↔ `WaveformFilter.nonVocals` |
| Waveform filter — Vocals (center extract) | context-menu | ✅ | ✅ | parity | P2 | medium | feasible | `Waveform.cpp:267` ↔ `WaveformFilter.vocals` |
| Waveform filter — Perceptual (LUFS) | context-menu | ✅ | ✅ | parity | P2 | medium | feasible | `Waveform.cpp:268` ↔ `WaveformFilter.lufs` |
| Waveform — HTDemucs stem separation (drums/bass/other/vocals) | context-menu | ✅ | ✅ | parity | P2 | hard | feasible | `Waveform.cpp:276-284` (macOS 12+) ↔ iPad `prepareStems` + install sheet `SequencerGridV2View.swift:898,1005`; both on-device CoreML, iOS 15+ |
| Waveform — double-height toggle | menu | ✅ | ✅ | parity | P3 | easy | feasible | `Waveform.cpp:307` ↔ `waveformDoubleHeight` `SequencerGridV2View.swift:233,1369` (B42) |
| Waveform — show onsets overlay | context-menu | ✅ | ✅ | parity | P1 | medium | feasible | `Waveform.cpp:308` ↔ `SequencerGridV2View.swift:918`, `SequencerViewModel.swift:239` |
| Waveform — show pitch contour | context-menu | ✅ | ✅ | parity | P2 | medium | feasible | `Waveform.cpp:309` ↔ `SequencerGridV2View.swift:929`, `SequencerViewModel.swift:250` |
| Waveform — view as spectrogram | context-menu | ✅ | ✅ | parity | P2 | hard | feasible | `Waveform.cpp:310` ↔ `SequencerGridV2View.swift:942`, `spectrogramBGRA` bridge |
| Sound classification (SoundAnalysis, Apple) | context-menu | 🟡 | ✅ | parity | P3 | hard | feasible | macOS `Waveform.cpp:303` (`__APPLE__` only) ↔ iPad A7 `SequencerGridV2View.swift:982`, `classifySound` bridge. Both Apple-only; Win/Linux desktop lacks it |
| Waveform — switch displayed audio track (Main + alts) | context-menu | ✅ | ✅ | parity | P2 | medium | feasible | `Waveform.cpp:321-344` ↔ iPad B43 `SequencerGridV2View.swift:955`. Desktop also retargets VAMP to the alt track; iPad switch is waveform-only |
| Alternate audio track CRUD (add/remove/rename/replace) | dialog | ✅ | ✅ | parity | P2 | medium | feasible | `SeqSettingsDialog.cpp:2320-2401` ↔ `SequenceSettingsSheet.swift:794` (AudioTracksTab). Desktop Add is now multi-select (`wxFD_MULTIPLE`, b4bfd9e13); iPad Add picks one file at a time via the single-URL `mediaRelocationPrompt` (`SequenceSettingsSheet.swift:814`) — minor: multi-add would need per-file relocation queueing. |
| Render selected region | context-menu | ✅ | ✅ | parity | P2 | medium | feasible | `Waveform.cpp:254` (`ID_WAVE_MNU_RENDER`) ↔ iPad B44 `SequencerViewModel.swift:145`, `renderRangeForRow` bridge |
| Waveform click/tap to seek (snap) | gesture | ✅ | ✅ | parity | P1 | easy | feasible | `Waveform::mouseLeftDown` ↔ `TopChromeMetalGridView.swift:31`, onSeek |
| Waveform right-click / long-press menu | context-menu | ✅ | ✅ | parity | P1 | easy | feasible | `Waveform::rightClick` ↔ `onWaveformMenu` `TopChromeMetalGridView.swift:541` → confirmationDialog |
| Drag-select a timeline range | gesture | ✅ | 🟡 | ipad-missing | P3 | medium | feasible | desktop `Waveform::mouseLeftUp` sets selection region (feeds Render Selected); iPad uses per-effect marquee + loop region, no waveform range-drag |
| Numbered tags 0–9 — set at playhead | menu | ✅ | ✅ | parity | P2 | easy | feasible | `TimeLine.cpp:61,119` ↔ `tagsMenuSection` `SequencerGridV2View.swift:1466`, `SequencerViewModel.swift:2028` |
| Numbered tags — go to | menu | ✅ | ✅ | parity | P2 | easy | feasible | `TimeLine::GoToTag` ↔ `SequencerViewModel.swift:2048` |
| Tags — clear all | menu | ✅ | ✅ | parity | P3 | easy | feasible | `TimeLine.cpp:78,109` ↔ `clearAllTags` `SequencerViewModel.swift:2042` |
| Tags — delete one | context-menu | ✅ | ❌ | ipad-missing | P3 | easy | feasible | desktop per-tag Delete submenu `TimeLine.cpp:81-87`; iPad only clears all |
| Tags — go to Next / Prior | shortcut | ✅ | ❌ | ipad-missing | P3 | easy | feasible | `TimeLine::GoToNextTag/GoToPriorTag` `MainSequencer.cpp:729,757`; iPad has no next/prior navigation |
| Zoom to selection | menu | ✅ | ✅ | parity | P2 | medium | feasible | `TimeLine.cpp:58` ↔ iPad B36 `SequencerGridV2View.swift:1327` |
| Zoom to fit / reset zoom | menu | ✅ | ✅ | parity | P2 | easy | feasible | `TimeLine.cpp:60` ↔ iPad B37 `SequencerGridV2View.swift:1319` |
| Playback speed (¼ ½ ¾ 1 1.5 2 3 4×) | menu | ✅ | ✅ | parity | P1 | easy | feasible | `xLightsMain.cpp:1236-1250` ↔ `XLPlaybackSpeeds` `XLightsCommands.swift:605-618` (identical 8) |
| Playback volume | menu | ✅ | 🟡 | parity | P2 | easy | feasible | desktop 5 named presets `xLightsMain.cpp:1253-1262`; iPad continuous 0–100 slider `SequencerViewModel.swift:2068`. Functionally equivalent; iPad just lacks the named radio presets |
| Pref — snap to timing marks | preference | ✅ | ❌ | ipad-missing | P2 | easy | feasible | `xLightsMain.cpp:1699` (`xLightsSnapToTimingMarks`); iPad timing-snap is always-on, not configurable |
| Pref — show alternate timing format | preference | ✅ | ❌ | ipad-missing | P3 | medium | feasible | `xLightsMain.cpp:1989`, `SetShowAlternateTimingFormat`; iPad ruler format not user-selectable |
| Pref — small waveform | preference | ✅ | ❌ | ipad-missing | P3 | easy | feasible | `xLightsMain.cpp:1684` (`xLightsSmallWaveform`); iPad has double-height toggle but no "small" mode |
| Pref — timing play on double-click (toggle) | preference | ✅ | 🟡 | ipad-missing | P2 | easy | feasible | `xLightsMain.cpp:1997` (`mTimingPlayOnDClick`); iPad double-tap-loop is always on with no disable toggle |
| Pref — exclude audio from packaged sequences | preference | ✅ | ❌ | ipad-missing | P3 | easy | feasible | `xLightsMain.cpp:1871`; iPad Package sheet (`PackageSequenceSheet`) has no audio-exclusion toggle |
| Shift Effects (time offset, effects only) | menu | ✅ | ❌ | ipad-missing | P2 | medium | feasible | `xLightsMain.cpp:1090` (`ID_SHIFT_EFFECTS`); no iPad equivalent |
| Shift Effects And Timing (time offset, all) | menu | ✅ | ❌ | ipad-missing | P2 | medium | feasible | `xLightsMain.cpp:1088` (`ID_SHIFT_EFFECTS_AND_TIMING`); no iPad equivalent |
| Shift Selected Effects (time offset, selection only) | menu | ✅ | ❌ | ipad-missing | P2 | medium | feasible | `xLightsMain.cpp:1092` (`MenuItemShiftSelectedEffects`), handler `:5902`, `ShiftSelectedEffectsOnLayer` `:5978`, caller `:5875-5893`; no iPad equivalent |
| Audio export / encode | menu | ✅ | 🟡 | ipad-weaker | P3 | medium | feasible | iPad exports WAV via `XLSequenceDocument.h:601` `writeCurrentToTempWav`; desktop also offers full encode via `AudioManager.h:285` `WriteCurrentAudio`, `:286` `EncodeAudio`. iPad lacks encoded-format export |
| Prepare Audio (Reaper/xAudio import) | dialog | ✅ | ❌ | ipad-missing | P3 | hard | infeasible | `xLightsMain.cpp:1154`, `OnMenuItem_PrepareAudioSelected`; needs Reaper .rpp automation / FFmpeg — not viable on iOS |

## iPad gaps (desktop has, iPad missing)

### P2

- **Shift Effects And Timing / Shift Effects / Shift Selected Effects** — desktop `Menu3` items wired at `xLightsMain.cpp:1088-1092` (`OnMenuItemShiftEffectsAndTimingSelected` / `OnMenuItemShiftEffectsSelected` / `MenuItemShiftSelectedEffects`, declared `xLightsMain.h:680,589`; selection handler `:5902`, `ShiftSelectedEffectsOnLayer` `:5978`). They bulk-offset every effect (and optionally every timing mark, or only the selection) by a millisecond delta, using `ShiftEffectsOnLayer` (`xLightsMain.h:2011`). **iPad work:** add a bridge method on `XLSequenceDocument` (e.g. `shiftAllEffects(byMS:includeTiming:selectedOnly:)`) over the existing layer-iteration core, plus a small sheet with a frames/ms field. **Ease: medium** (new bridge + sheet; core shift logic already exists).
- **Snap-to-timing-marks preference** — desktop `_snapToTimingMarks` (`xLightsMain.cpp:1699`, default on) makes effect/mark drags snap to nearby timing marks. iPad snapping is hardcoded behavior with no user toggle. **iPad work:** an `@AppStorage` flag read by the drag-end handlers in `EffectsMetalGridView`/`TimingEffectsMetalGridView`, surfaced in a Preferences sheet. **Ease: easy.**
- **Timing play-on-double-click toggle** — desktop `mTimingPlayOnDClick` (`xLightsMain.cpp:1997`) lets the user disable double-click-to-play. iPad always loops on double-tap-mark (`SequencerViewModel.swift:2001`) with no opt-out. **iPad work:** an `@AppStorage` gate around `onDoubleTapMark`. **Ease: easy.**
- **Drag-select a waveform/timeline range** — desktop drag on the waveform sets a start/end region (`Waveform::mouseLeftUp` → `EVT_WAVE_FORM_HIGHLIGHT`) that drives "Render Selected Region" and "Zoom to Selection". iPad has a loop region and effect marquee but no waveform range-drag to define an arbitrary render/zoom window from the audio strip. **iPad work:** a two-finger or handle-based range gesture on `TopChromeMetalGridView` feeding the existing `waveformStartMS/EndMS` (B44). **Ease: medium.**

### P3

- **Import Notes (MIDI / MusicXML / Audacity / Polyphonic Transcription)** — desktop `RowHeading.cpp:632` → `ExecuteImportNotes` (`tabSequencer.cpp:4024`) builds a MIDI-note-labelled timing track via `LoadMIDIFile` / `LoadMusicXMLFile` / `LoadAudacityFile` / `LoadPolyphonicTranscription`. **iPad work:** a `NoteImportDialog` equivalent + bridge wrappers for the file loaders. The Polyphonic-Transcription source is a VAMP plugin and is **infeasible** on iPad; the MIDI/MusicXML/Audacity sources are **hard** (need the loaders bridged, but they are wx-light core). **Ease: medium / Feasibility: hard.**
- **Per-tag delete & Next/Prior tag navigation** — desktop deletes individual tags via the timeline Delete submenu (`TimeLine.cpp:81-87`) and navigates with `GoToNextTag`/`GoToPriorTag` (`MainSequencer.cpp:729,757`). iPad only offers Clear-All and direct Go-To. **iPad work:** add `goToNextTag()`/`goToPriorTag()` (bridge already exposes tag positions) and a per-tag delete in `tagsMenuSection`. **Ease: easy.**
- **Volume named presets** — desktop offers Loud/Medium/Quiet/Very Quiet/Silent radio items (`xLightsMain.cpp:1253-1262`). iPad's continuous slider already covers the range; adding labelled quick-picks is cosmetic. **Ease: easy.**
- **Small-waveform & alternate-timing-format & exclude-audio-from-package preferences** — three desktop config flags (`xLightsMain.cpp:1684,1989,1871`) with no iPad surface. Each is a one-line `@AppStorage` + a Preferences row (and, for exclude-audio, a checkbox in `PackageSequenceSheet`). **Ease: easy.**
- **Audio export / encode** — iPad writes the current audio to a temp WAV (`XLSequenceDocument.h:601` `writeCurrentToTempWav`), but desktop additionally exposes a full encode path (`AudioManager.h:285` `WriteCurrentAudio`, `:286` `EncodeAudio`) for encoded output formats. **iPad work:** bridge the encode entry point and add a format picker to the export flow. **Ease: medium.**

## Desktop gaps (iPad has, desktop missing)

- **Double-tap-mark to loop-play** (`TimingEffectsMetalGridView.swift:31` → `playLoopForTimingMark`): iPad sets the loop region to the tapped mark's range and plays it. Desktop has no single-gesture "loop just this mark"; closest is the generic play-on-double-click. Touch idiom — low value to port.
- **Apple Pencil precision edge grab** (`TimingEffectsMetalGridView.swift:82`): halves mark-edge hit slop when a Pencil is active. No desktop analog (mouse precision is already exact). Infeasible/irrelevant on desktop.
- **Explicit per-mark "Merge with Next"** (`SequencerViewModel.swift:3662`): desktop only merges by the SubdivisionOptionsDialog "combine every N marks" path (`RowHeading.cpp:84`), not a single-pair merge button. Minor desktop convenience gap; could add to `EffectsGrid::rightClick`.

## Infeasible / restricted on iPad

- **VAMP Queen-Mary plugin analyzers** (`RowHeading.cpp:1090`): no VAMP host and no FFmpeg on iPad. The shipped built-in detectors (Onsets/Tempo/Chords) cover the common analyses without VAMP. *Infeasible.*
- **Prepare Audio (Reaper .rpp / xAudio)** (`xLightsMain.cpp:1154`): depends on Reaper-project automation and heavy audio transcoding (FFmpeg). Not viable inside the iOS sandbox. *Infeasible.*
- **Polyphonic Transcription "Import Notes" source**: that note source is itself a VAMP plugin — *infeasible* even if the other Import-Notes loaders are bridged.
- **Sound classification on Windows/Linux desktop**: parity with iPad exists only on the Apple desktop build (`Waveform.cpp` is `#ifdef __APPLE__`); this is a *desktop platform* limit, not an iPad one.
- *No firmware-restricted items in this theme* — FPP Commands/Effects timing tracks are sequence metadata, not controller uploads, so they are fully in-scope and already at parity.

## Recommended sequencing

1. **Shift Effects / Shift Effects And Timing** (P2) — the single highest-value missing *operation*; common during audio re-syncs. One bridge method + a small sheet over existing core shift logic.
2. **Snap-to-timing-marks + timing-play-on-double-click toggles** (P2) — cheap `@AppStorage` gates that remove "iPad behaves differently with no way to change it" friction. Bundle with a first Preferences pass.
3. **Waveform/timeline range-drag selection** (P2) — unlocks a cleaner "Render Selected Region" and "Zoom to Selection" from the audio strip, which iPad currently reaches only via effect selection.
4. **Tag polish: Next/Prior nav + per-tag delete** (P3) — small, finishes tag parity.
5. **Import Notes (MIDI / MusicXML / Audacity)** (P3) — bridge the three non-VAMP loaders + a note-import sheet; leave Polyphonic out (VAMP).
6. **Remaining cosmetic prefs** (small waveform, alternate timing format, exclude-audio-from-package) and **volume named presets** — batch into the Preferences sweep last.
