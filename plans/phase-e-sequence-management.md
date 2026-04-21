# Phase E — Sequence management

Everything to do with the *lifecycle of a `.xsq` document*: create it,
open it, edit it, save it, close it, and manage the per-sequence
settings that exist outside the effect grid (timing tracks, metadata,
media file, sequence type, audio tracks, data layers, render mode).

The iPad today is effectively a read-only viewer for sequences:
`XLSequenceDocument` can `openSequence:` / `closeSequence`, but there
is no save, no save-as, no new, no dirty-state tracking, no
sequence-settings surface, no load-time migration awareness, and no
missing-media prompting. All edits made via the grid / inspector live
in memory and vanish on exit.

Phase E closes all of that. It needs to land before Phase F (window
system) because F's File menu needs these commands to bind to, and
before any real user testing because users lose every edit right now.

## E-1. Document lifecycle — save / save-as / close

Bridge gaps today: `XLSequenceDocument.h` has no save method. The
desktop path is `TabSequence::SaveSequence()` / `SaveAsSequence()`
(src-ui-wx/app-shell/TabSequence.cpp) — it serializes the
`SequenceElements` tree back to the XML document owned by
`CurrentSeqXmlFile`, compresses the large `<DisplayElements>` /
`<ElementEffects>` payloads, and writes the `.xsq`. Also writes a
compiled `.fseq` to the FSEQ directory for playback.

- Add `-saveSequence`, `-saveSequenceAs:`, and `-sequenceIsDirty`
  Objective-C methods on `XLSequenceDocument`. Reuse the desktop save
  path (`SequenceFile::Save` or equivalent core-side entry point); do
  not re-implement serialization.
- Add `saveSequence()`, `saveAsSequence(to:)`, `closeSequence()` to
  `SequencerViewModel` (already has `closeSequence()` but without
  dirty-state prompting).
- Save As must use `UIDocumentPickerViewController` with the show
  folder as the default suggested location. The desktop enforces
  save-under-show-folder; iPad should warn (not block) if the user
  picks somewhere else — iCloud Drive / Files app location is fine.
- Close: if dirty, show a confirmation sheet with Save / Discard /
  Cancel. Pattern mirrors desktop `CloseSequence()` dirty prompt
  (src-ui-wx/import_export/SeqFileUtilities.cpp:935).
- Dirty tracking: compare `_sequenceElements.GetChangeCount()` (core
  already maintains this counter) against the count taken at the last
  save; expose as `-sequenceIsDirty` through the bridge and
  `isDirty: Bool` via `@Observable` on the view model. Inspector edits,
  grid edits, timing edits, layer add/remove, and sequence-settings
  edits all bump the counter automatically; no new hooks needed.
- `.fseq` emission: desktop writes it alongside the save. For Phase E
  keep parity — this is what downstream playback (e.g. Falcon Player
  export, even if just a future phase) expects. No UI change, just
  plumb through the same compile call.

## E-2. New Sequence wizard

Desktop's "New" closes the current sequence and launches
`SeqSettingsDialog` in wizard mode
(src-ui-wx/import_export/SeqFileUtilities.cpp:94). Required data
before the sequence can be saved:

1. Sequence type (Musical / Animation / Effect — drives which wizard
   pages appear).
2. Media file path (Musical only — otherwise just duration).
3. Duration (Animation / Effect — in seconds or mm:ss).
4. Frame rate (25 / 50 ms are the stock picks; allow custom).
5. Timing-track import (optional — can pick from an existing sequence,
   a `.xtiming` file, or skip).

iPad plan:

- SwiftUI full-screen sheet (`NavigationStack` with step-by-step
  pages). Use the same terminology and order as desktop so docs /
  tutorials still apply.
- Page 1: type picker. Page 2a (Musical): media picker (routes
  through existing `MediaRelocation` modifier so the file ends up
  under the show folder or a configured media folder). Page 2b
  (Animation / Effect): duration + frame rate. Page 3: timing track
  import (optional).
- On finish: bridge `-newSequenceWithType:mediaPath:durationMS:frameMS:`
  creates a blank `SequenceElements` tree, imports the picked timing
  tracks, loads into `SequencerViewModel`. Sequence is marked dirty
  until the first save so the user can't lose the wizard work by
  accident.
- Entry points: File menu (F-4), empty-state screen on app launch
  when no recent sequence is open, toolbar "+" next to the sequence
  name.

## E-3. Sequence Settings dialog

Post-open editor for sequence-wide settings. Desktop
`SeqSettingsDialog` has six notebook tabs (Info, Timings, Metadata,
Data Layers, Audio, plus wizard pages hidden outside the wizard
flow). Source: `src-ui-wx/sequencer/SeqSettingsDialog.{h,cpp}`.

Implement as a `.sheet` with a segmented control or sidebar tabs —
not a wizard. Desktop shares the dialog between New and
Sequence-Settings workflows; iPad splits them (E-2 handles New).

Tabs to port, ordered by how often users touch them:

- **Timings** — add / rename / delete timing tracks; import from
  another sequence or `.xtiming`; export to `.xtiming`. Partial
  support already exists (rename / delete via row-header long-press);
  the dialog centralizes it and adds import/export.
- **Metadata** — song, artist, album, author, website, comment,
  music URL. Plain text fields persisted on `SequenceFile` metadata
  attributes. New bridge getters/setters needed.
- **Info** — read-only summary (filename, file version, model count)
  plus sequence type selector. Rarely changed post-creation.
- **Media file** — swap the media file. Routes through
  `MediaRelocation` again; may re-run the audio-analysis path if the
  file changes.
- **Audio Tracks** — list of alternate audio tracks (add / remove /
  rename / pick file). Deferred behind a feature flag if we ship
  Phase E without alt-track playback; the data still round-trips
  through the XML either way so it's not destructive to skip the UI.
- **Render Mode / Blending** — GPU blend mode dropdown + model-
  blending toggle. Rarely changed; small surface.
- **Data Layers** — image-data layers UI. Lowest priority; deferred
  post-MVP unless someone is using them on the iPad.

Entry: File menu → "Sequence Settings…" and a gear icon in the
sequencer toolbar.

## E-4. Load-time migration + media availability

Two pieces that already partially happen invisibly but have no iPad
UX yet:

**Effect version migrations.** `SequenceFile::LoadSequence` already
runs the `RenderableEffect::adjustSettings(version, effect)` pass
across every effect (src-core/render/SequenceFile.cpp:1895–1933).
It runs silently on iPad today because the bridge just calls the
core loader. What's missing:

- Bubble the sequence's original file version up to Swift so the UI
  can tell the user "this sequence was created in xLights 2024.07 and
  has been migrated; save to update". Non-modal banner at the top of
  the sequencer, dismissible.
- Mark the sequence dirty after a migration so the user's first save
  persists the upgraded form.

**Missing media.** `LoadSequence` logs but does not prompt when the
media file or alternate audio tracks are missing
(SequenceFile.cpp:470–496). iPad should:

- On open, iterate the resolved media paths and flag any that
  `FileExists()` reports false. (Do *not* accept the silent log
  behaviour — on iPadOS, missing media is usually an iCloud /
  bookmark issue the user can fix.)
- Present a "Relocate media" sheet reusing the existing
  `MediaRelocation` flow: for each missing file, let the user pick
  a replacement via `UIDocumentPickerViewController`, then route
  through `-moveFileToShowFolder:` or `-copyFileToMediaFolder:` to
  copy it into a tracked root. Update the sequence's stored path
  (relative if under show folder) and mark dirty.
- Allow "skip" per file — sequence still opens, just with that audio
  track disabled, matching desktop's tolerance.

## E-5. Recent documents + empty state

Desktop has an MRU list surfaced via the File menu. iPad should have
the equivalent plus a launch-time empty state:

- Persist the last N opened `.xsq` bookmarks (security-scoped) in
  UserDefaults. Surface as "Open Recent" in the File menu (F-4) and
  as cards on a launch-screen view when no sequence is open.
- Launch-screen view: "New Sequence" button (E-2), "Open Sequence"
  button (picker), "Open Recent" list. Replaces the current cold-
  launch state where the user has to manually tap into the sequencer
  with nothing loaded.

## E-6. Autosave / crash recovery (`.xbkp`)

Desktop periodically writes an `.xbkp` snapshot to the show folder so
a crash doesn't lose more than the autosave interval. On next open, if
a newer `.xbkp` exists alongside the `.xsq`, it's offered for
recovery. Source: `src-ui-wx/import_export/SeqFileUtilities.cpp`
autosave timer + the close-time timestamp adjustment that suppresses
the offer after an explicit discard.

- Port the autosave timer into `SequencerViewModel` (a
  `Timer.publish` at a configurable interval; default 5 min).
- On sequence open, check for `<basename>.xbkp` newer than the
  `.xsq`; if present, sheet the user: "Recover changes from
  <time>?" Apply or discard via the same bridge save path E-1
  provides.
- On explicit Close → Save / Close → Discard, update the `.xbkp`
  mtime so recovery isn't re-offered.

---

## Explicitly out of scope for Phase E

- **iCloud Drive coordination** (`NSFileCoordinator`, ubiquity
  status) — Phase G covers this. E-1 writes through plain file I/O.
- **Files-app "Open in xLights" routing** — Phase G-3 registers the
  document type. Phase E opens via in-app pickers only.
- **Data Layers UI** — deferred until a user actually needs it on
  iPad. The XML round-trips through the loader unchanged.
- **Full `SeqSettingsDialog` wizard fidelity** — Phase E-2 uses the
  same data flow but doesn't attempt pixel parity with the desktop
  wxSmith layout.
