#pragma once

#import <Foundation/Foundation.h>
#import <CoreGraphics/CGBase.h>

// ObjC bridge for iPadRenderContext — callable from Swift.
// Manages show folder loading and sequence access.

@class XLCheckSequenceIssue;

NS_ASSUME_NONNULL_BEGIN

@interface XLSequenceDocument : NSObject

// Show folder
- (BOOL)loadShowFolder:(NSString*)path;
- (BOOL)loadShowFolder:(NSString*)path mediaFolders:(NSArray<NSString*>*)mediaFolders;

// Register/refresh a persistent security-scoped bookmark for a folder path.
// Call this whenever the user picks a folder via UIDocumentPicker so the
// bookmark is stored in UserDefaults and access survives app restart.
+ (BOOL)obtainAccessToPath:(NSString*)path enforceWritable:(BOOL)enforceWritable;

// Show / media folder accessors — needed by the file-relocation logic
// so Swift can tell whether a picked URL is already inside the
// enforced roots.
- (NSString*)showFolderPath;
- (NSArray<NSString*>*)mediaFolderPaths;

// Copy `sourcePath` into `<showFolder>/<subdirectory>`, appending `_N`
// to the basename on collision. Returns the destination absolute path
// on success, nil on failure (no show folder loaded, copy error).
- (nullable NSString*)moveFileToShowFolder:(NSString*)sourcePath
                                 subdirectory:(NSString*)subdirectory;

// Copy `sourcePath` into `<mediaFolderPath>/<subdirectory>`.
// `mediaFolderPath` must already be in `mediaFolderPaths`; unknown
// paths are rejected so the "media always lives in a configured root"
// invariant isn't broken.
- (nullable NSString*)copyFileToMediaFolder:(NSString*)sourcePath
                                mediaFolderPath:(NSString*)mediaFolderPath
                                 subdirectory:(NSString*)subdirectory;

// True iff `path` is under the show folder or any configured media
// folder. Used to decide whether a picked file needs copying.
- (BOOL)pathIsInShowOrMediaFolder:(NSString*)path;

// Compute a show-folder-relative path (e.g. `Images/foo.png`). Absolute
// paths outside the show folder round-trip unchanged so media-folder
// files aren't clobbered.
- (NSString*)makeRelativePath:(NSString*)path;

// Sequence
- (BOOL)openSequence:(NSString*)path;
- (void)closeSequence;
- (BOOL)isSequenceLoaded;

// xsqz open: `pkgPath` is a `.xsqz` / `.zip` / `.piz` package.
// Extracts it to a temp dir (via `SequencePackage`), swaps the
// show folder to that temp dir, and opens the inner `.xsq`.
// On a subsequent `-saveSequence`, the bridge re-packs the
// temp dir back into `pkgPath` atomically — see
// `-[saveSequence]`. `-closeSequence` wipes the temp dir and
// restores whatever show folder was active before the open.
//
// May take a noticeable time for large packages (disk I/O on
// many-MB zips); Swift callers should dispatch this off the
// main queue and show a progress affordance.
//
// Returns NO if extraction fails, the package has no `.xsq`
// inside, or the internal open fails.
- (BOOL)openPackagedSequence:(NSString*)pkgPath NS_SWIFT_NAME(openPackagedSequence(atPath:));

// YES iff the currently-open sequence was opened via
// `-openPackagedSequence:` and hasn't been closed yet. The
// Swift UI uses this to skip "Save As to new xsq" affordances
// that would break out of the package.
- (BOOL)isPackagedSequence;

// Absolute path of the `.xsqz` the current session was opened
// from. Empty when the current sequence isn't packaged.
- (NSString*)packagePath;

// E-2 — create a fresh sequence on disk at `savePath` and
// immediately open it as the active document. `type` is one of
// "Media" / "Animation" / "Effect"; `mediaPath` is required for
// Media and ignored otherwise (paths are resolved through the
// show folder). `durationMS` and `frameMS` write the XML head
// attributes matching the desktop wizard's output. Returns NO
// on failure; closes any previously-open sequence before
// starting.
- (BOOL)newSequenceAtPath:(NSString*)savePath
                       type:(NSString*)type
                  mediaPath:(NSString*)mediaPath
                 durationMS:(int)durationMS
                    frameMS:(int)frameMS;

// Save the currently-open sequence back to its on-disk path. Returns
// NO if there's no sequence loaded, the path is empty, or the XML
// write fails. Marks the sequence clean on success.
- (BOOL)saveSequence;

// Tier 1 memory mitigation — drop the undo / redo history.
// Per-step snapshots (settings + palette strings for every
// captured effect) add up fast in long editing sessions. Called
// from the Swift save path after a successful save so the app
// reclaims memory at the natural "I'm done with those edits"
// checkpoint. Safe to call with no sequence open — no-op.
- (void)clearUndoHistory;

// Save to a new path (Save As / Export). `path` must end in `.xsq`;
// the caller is responsible for obtaining security-scoped access
// to the destination via `-obtainAccessToPath:…` before calling.
// On success updates the sequence's internal path so subsequent
// `-saveSequence` writes to the new location.
- (BOOL)saveSequenceAs:(NSString*)path;

// Absolute on-disk path the sequence was opened from (or last
// saved to). Empty when no sequence is loaded.
- (NSString*)currentSequencePath;

// Dirty tracking (E-1). `SequenceElements` increments a change
// counter on every mutation; the bridge records the counter
// snapshot taken at load / save, and `isSequenceDirty` compares the
// live counter against that snapshot. Call `markSequenceClean`
// after a save-via-non-bridge-path (e.g. a save-as through Swift
// that writes out-of-band).
- (BOOL)isSequenceDirty;
- (void)markSequenceClean;

// E-4 — version metadata. `sequenceFileVersion` is the xLights
// version string recorded in the on-disk `.xsq` at the time it
// was last saved; `currentAppVersion` is the running build's
// `xlights_version_string`. Differ → the sequence was authored
// in a different build, so the inspector shows the migration
// banner. Both are empty when no sequence is loaded.
- (NSString*)sequenceFileVersion;
- (NSString*)currentAppVersion;

// About-screen helpers. `appVersion` (without an instance) and
// `licenseText` are both static — same value across the whole app
// lifetime — so they're class methods so the About sheet doesn't
// need a document handle. `licenseText` is the GPL preamble +
// upstream credit text from `XLIGHTS_LICENSE` in `globals.h`,
// shared with desktop's About dialog.
+ (NSString*)appVersion;
+ (NSString*)licenseText;

// E-6 — autosave / `.xbkp` recovery. Write the current in-memory
// sequence to `<basename>.xbkp` alongside the `.xsq` without
// touching the canonical file or the dirty flag. Temporarily
// swaps the SequenceFile's `mFilePath` (matching desktop's
// `SaveWorking` pattern in `xLightsMain.cpp:4588-4632`), writes,
// then restores. Returns NO if no sequence is open or the
// current path is empty (new-unsaved sequences have nowhere to
// put the backup yet).
- (BOOL)writeAutosaveBackup;

// E-3 — Sequence Settings. Info tab (read-only) + Metadata tab
// (header text fields) + Media file swap + Render Mode /
// blending toggle. All setters mark the sequence dirty via the
// normal `SequenceElements::IncrementChangeCount` path.
//
// Metadata keys accepted by `headerInfoForKey:` /
// `setHeaderInfo:forKey:`: "song", "artist", "album", "author",
// "email", "website", "url", "comment". Other keys return
// empty / no-op.
- (NSString*)headerInfoForKey:(NSString*)key;
- (BOOL)setHeaderInfo:(NSString*)value forKey:(NSString*)key;

// Media file getters/setters. `setMediaFilePath:` routes
// through `SequenceFile::SetMediaFile` with `overwrite_tags=NO`
// so existing song / artist metadata isn't clobbered when the
// user is swapping the audio track on a sequence that already
// has populated header fields. Pass empty to clear the media
// file (turns a Musical sequence into an Animation — matches
// desktop behaviour).
- (NSString*)currentMediaFilePath;
- (BOOL)setMediaFilePath:(NSString*)path;

// ID3-style tags from the loaded audio track. Empty strings when
// no media is loaded or the file has no metadata. Used to
// pre-populate the AI palette generator's "Song" mode.
- (NSString*)audioTitle;
- (NSString*)audioArtist;
- (NSString*)audioAlbum;

// Sequence type (see `newSequenceAtPath:…`): "Media" /
// "Animation" / "Effect". Writes flow through
// `SequenceFile::SetSequenceType` which clears media file +
// audio when switching to Animation / Effect.
- (NSString*)sequenceType;
- (BOOL)setSequenceType:(NSString*)type;

// Per-frame interval in ms (the sequence's tick). Changes to
// this require a full re-render; callers should abort any
// in-flight render before calling.
- (BOOL)setFrameIntervalMS:(int)frameMS;

// Model-blending toggle (global blend mode).
- (BOOL)sequenceSupportsModelBlending;
- (BOOL)setSequenceSupportsModelBlending:(BOOL)enabled;

// Read-only summary for the Info tab.
- (int)sequenceModelCount;

// Sequence metadata
- (int)sequenceDurationMS;
- (int)frameIntervalMS;
- (NSString*)sequenceName;

// Elements & rows
- (int)visibleRowCount;
- (NSString*)rowDisplayNameAtIndex:(int)index;
- (int)rowLayerIndexAtIndex:(int)index;
- (BOOL)rowIsCollapsedAtIndex:(int)index;
// Model name for a row (element->GetName()). Empty for non-model rows (e.g. timings).
- (NSString*)rowModelNameAtIndex:(int)index;

// Timing-row queries (rows whose Element is TIMING). Returns indices into
// the visible-row list used by effectCountForRow: and friends.
- (NSArray<NSNumber*>*)timingRowIndices;
- (BOOL)timingRowIsActiveAtIndex:(int)rowIndex;
- (void)setTimingRowActive:(BOOL)active atIndex:(int)rowIndex;
// B81: hide or show every timing track in the sequence (master
// view). `allTimingTracksHidden` returns YES when none are
// currently in the row-information list, i.e. all have been
// hidden. Caller should `reloadRows` after flipping the state.
- (void)setAllTimingTracksHidden:(BOOL)hidden;
- (BOOL)allTimingTracksHidden;
// B82: add every visible timing track to every non-master view.
- (int)addAllTimingTracksToAllViews;
// Color index assigned sequentially to each timing element (0..4, cycles).
// Layers within the same timing element share the same color index.
- (int)timingRowColorIndexAtIndex:(int)rowIndex;
// Name of the timing element the row belongs to (stable across layers).
- (NSString*)timingRowElementNameAtIndex:(int)rowIndex;
// Effect layer's own label, e.g. "Phrases", "Words", "Phonemes" for
// lyric tracks. Empty if the layer has no explicit name.
- (NSString*)rowLayerNameAtIndex:(int)rowIndex;

// Model-row queries used by row headers.
- (BOOL)rowIsModelGroupAtIndex:(int)rowIndex;
- (int)rowLayerCountAtIndex:(int)rowIndex;
// B48: number of empty layers on the row's element. Use to gate
// the "Delete Unused Layers" row-menu entry + label its count.
- (int)unusedLayerCountAtRow:(int)rowIndex
    NS_SWIFT_NAME(unusedLayerCount(atRow:));
- (BOOL)rowIsElementCollapsedAtIndex:(int)rowIndex;
- (void)toggleElementCollapsedAtIndex:(int)rowIndex;

// Submodel / strand / node row metadata (mirrors
// `Row_Information_Struct.submodel`, `nestDepth`, `strandIndex`,
// `nodeIndex`). `nestDepth` drives visual indent on the left
// column; `strandIndex >= 0` and `nodeIndex >= 0` identify
// strand/node rows for the disclosure affordances below.
- (BOOL)rowIsSubmodelAtIndex:(int)rowIndex;
- (int)rowNestDepthAtIndex:(int)rowIndex;
- (int)rowStrandIndexAtIndex:(int)rowIndex;
- (int)rowNodeIndexAtIndex:(int)rowIndex;

// Submodel / strand disclosure. A row "has submodels" if its
// element is a ModelElement with strand/submodel children (desktop
// `ModelElement::ShowStrands` target); a row "has nodes" if it's a
// StrandElement with at least one node layer
// (`StrandElement::ShowNodes` target). Toggle helpers flip the
// state and repopulate row information so the caller only needs to
// refresh its row cache afterwards.
- (BOOL)rowHasSubmodelsAtIndex:(int)rowIndex;
- (BOOL)rowShowsSubmodelsAtIndex:(int)rowIndex;
- (void)toggleRowShowSubmodelsAtIndex:(int)rowIndex;
- (BOOL)rowHasNodesAtIndex:(int)rowIndex;
- (BOOL)rowShowsNodesAtIndex:(int)rowIndex;
- (void)toggleRowShowNodesAtIndex:(int)rowIndex;

// Layer management on a model / submodel / strand row. Mirrors
// the "Insert Layer Above/Below" and "Delete Layer" entries in
// desktop's `RowHeading` right-click menu
// (`RowHeading.cpp:751-801`). `insertAbove` / `insertBelow` key
// off the row's own `layerIndex`; `remove` returns NO if the
// element is down to its last layer (desktop disables the item in
// that case too). Repopulates row info on success.
- (BOOL)insertEffectLayerAboveAtIndex:(int)rowIndex;
- (BOOL)insertEffectLayerBelowAtIndex:(int)rowIndex;
// B47: insert `count` empty layers below the row's layerIndex.
- (int)insertEffectLayersBelowAtIndex:(int)rowIndex count:(int)count
    NS_SWIFT_NAME(insertEffectLayersBelow(at:count:));
- (BOOL)removeEffectLayerAtIndex:(int)rowIndex;

// B55: rewrite each effect's `B_CHOICE_BufferStyle` setting from
// "Per Preview" / "Default" / "Single Line" / "" to the matching
// "Per Model …" variant. When `acrossAllLayers` is YES, walks every
// layer of the row's element (model-scope menu); otherwise only
// the row's own layer (layer-scope menu, mirrors desktop's two
// menu entries). Returns the count of effects whose setting was
// updated so the UI can show "N converted" if it wants. Triggers a
// re-render of the affected model on success.
- (int)convertEffectsToPerModelOnRow:(int)rowIndex acrossAllLayers:(BOOL)allLayers
    NS_SWIFT_NAME(convertEffectsToPerModel(onRow:acrossAllLayers:));

// B56: promote node-level "On" / "Color Wash" effects up the
// strand → model hierarchy when every node carries an identical
// copy at the same time range. Two-pass: nodes → strand layer 0,
// then strand layer 0 → model layer 0 (only when there's > 1
// strand). No-op for ModelGroup / SubModel rows. Returns the
// number of effects coalesced; on success a model re-render is
// triggered automatically.
- (int)promoteNodeEffectsOnRow:(int)rowIndex
    NS_SWIFT_NAME(promoteNodeEffects(onRow:));

// B48: delete every layer on the row's element that has zero
// effects (keeps the element's ≥ 1 layer invariant). Returns the
// number of layers removed. Caller should `reloadRows` afterward.
- (int)deleteUnusedLayersOnElementAtRow:(int)rowIndex
    NS_SWIFT_NAME(deleteUnusedLayers(onElementAt:));

// B57: global collapse / expand. `collapseAllElements` sets
// `SetCollapsed(true)` on every non-timing Element; `expandAll` does
// the opposite. Repopulates row info so the caller just needs to
// reloadRows afterwards.
- (void)collapseAllElements;
- (void)expandAllElements;

// B46: in-place rename of an effect layer's name (`EffectLayer::
// SetLayerName`). Empty string clears the name. Returns NO if the
// row doesn't resolve to an EffectLayer.
- (BOOL)renameLayerAtRow:(int)rowIndex
                    name:(NSString*)newName
    NS_SWIFT_NAME(renameLayer(atRow:name:));

// B51: Element-level render-disabled toggle. Disables rendering for
// the whole element (model + submodels + strands + nodes). Reads via
// `elementRenderDisabled` / writes via `setElementRenderDisabled`.
- (BOOL)elementRenderDisabledAtRow:(int)rowIndex
    NS_SWIFT_NAME(elementRenderDisabled(atRow:));
- (void)setElementRenderDisabled:(BOOL)disabled atRow:(int)rowIndex
    NS_SWIFT_NAME(setElementRenderDisabled(_:atRow:));

// B50: return the count of effects on the given row's layer. Used
// to gate the "Delete All Effects" menu entry and report "N effects
// will be deleted" in the confirm alert.
- (int)effectCountOnRow:(int)rowIndex
    NS_SWIFT_NAME(effectCountOnRow(_:));

// B87: drop any word + phoneme layers (layers 1 and 2) from the
// timing element the given row belongs to. Rejected if the element
// has no sub-layers, or if any word/phoneme mark is locked. Matches
// desktop's implicit "empty layer when user doesn't need it" cleanup;
// no explicit desktop menu entry — it's an inverse of
// BreakdownPhrases.
- (BOOL)removeWordsAndPhonemesAtRow:(int)rowIndex
    NS_SWIFT_NAME(removeWordsAndPhonemes(atRow:));

// B76: timing-track fixed-vs-variable accessors. Fixed tracks carry
// a non-zero `mFixed` interval (milliseconds-per-mark) that
// prevents per-mark editing. `makeTimingTrackVariable` calls
// `SetFixedTiming(0)` which unlocks user editing while leaving the
// existing fixed-period marks in place. Returns NO if the row isn't
// a timing element.
- (BOOL)timingTrackIsFixedAtRow:(int)rowIndex
    NS_SWIFT_NAME(timingTrackIsFixed(atRow:));
- (BOOL)makeTimingTrackVariableAtRow:(int)rowIndex
    NS_SWIFT_NAME(makeTimingTrackVariable(atRow:));

// B74 import: read an `.xtiming` file (single `<timing>` or
// multi-`<timings>` wrapper) and add each entry as a new timing
// track. Names uniquified on collision (same rule as
// `addTimingTrack`). Returns the number of tracks added (0 on
// parse failure). Routes through
// `SequenceFile::ProcessXTiming({path}, iPadRenderContext)`.
- (int)importXTimingFromPath:(NSString*)path
    NS_SWIFT_NAME(importXTiming(fromPath:));

// LOR `.lms` timing import. Routes through `SequenceFile::
// ProcessLorTiming({path}, iPadRenderContext)` which handles both
// "regular" and "musical" LOR timings. Returns the number of
// tracks added.
- (int)importLorTimingFromPath:(NSString*)path
    NS_SWIFT_NAME(importLorTiming(fromPath:));

// Papagayo `.pgo` lyric-sync import. Routes through
// `SequenceFile::ProcessPapagayo({path}, iPadRenderContext)` which
// builds Phrase / Word / Phoneme layers. Returns the number of
// tracks added.
- (int)importPapagayoTimingFromPath:(NSString*)path
    NS_SWIFT_NAME(importPapagayoTiming(fromPath:));

// B78 import lyrics: replace the target timing element's layers
// with a single phrase layer populated from `phrases`. Each non-
// empty phrase gets one mark spanning its slice of
// [startMS, endMS], end times snapped to the sequence's frame
// period. `phrases` should already have whitespace trimmed per
// line; this method still strips a few common illegal XML chars
// and smart-quote unicode variants. Returns the number of marks
// added (0 on failure / empty input).
- (int)importLyricsAtRow:(int)rowIndex
                 phrases:(NSArray<NSString*>*)phrases
                 startMS:(int)startMS
                   endMS:(int)endMS
    NS_SWIFT_NAME(importLyrics(atRow:phrases:startMS:endMS:));

// B75 export: write the timing element at `rowIndex` to `path` as a
// self-contained `.xtiming` XML document (wraps
// `TimingElement::GetExport()` with the standard `<?xml ...?>` +
// `<timing name=... subType=... SourceVersion=...>` envelope).
// Returns NO if the row isn't a timing row or the write failed.
// Caller is responsible for obtaining security-scoped access to
// `path` before calling.
- (BOOL)exportTimingTrackAtRow:(int)rowIndex toPath:(NSString*)path
    NS_SWIFT_NAME(exportTimingTrack(atRow:toPath:));

// B49: export the rendered channel data for the row's model as a
// compressed FSEQ v2 `.eseq` sub-sequence — the format Falcon
// Player uses to play back per-model data. Uses the current render
// state in `_seqData`, so the caller should ensure a full render
// has completed first (the render engine runs continuously on
// iPad, but a fresh render can be forced by kicking off the
// toolbar render button). When `startMS` >= 0 and `endMS` > 0 the
// exported file only covers the [startMS, endMS] frame range;
// otherwise the whole sequence is written. Returns NO if the row
// isn't a model row, the sequence has no rendered data, or the
// write failed. Caller must call `ObtainAccessToURL` on `path`
// beforehand.
- (BOOL)exportModelAsFSEQAtRow:(int)rowIndex toPath:(NSString*)path
                       startMS:(int)startMS endMS:(int)endMS
    NS_SWIFT_NAME(exportModelAsFSEQ(atRow:toPath:startMS:endMS:));

// Write the rendered sequence to `path` as a v2/zstd/sparse FSEQ matching
// the format produced by desktop's xLightsFrame::WriteFalconPiFile. Returns
// NO if no sequence is loaded, the path is empty, or the underlying write
// fails. Caller must call `ObtainAccessToURL` on `path` beforehand.
- (BOOL)writeFseqToPath:(NSString*)path NS_SWIFT_NAME(writeFseq(toPath:));

// Try to populate sequence frame data from a previously-saved FSEQ file at
// `fseqPath`, allowing the caller to skip an immediate render. Returns NO if
// the file is missing, older than `xsqPath`, or its shape (frame count, step
// time, channel count) doesn't match the loaded sequence. `xsqPath` may be
// empty to skip the staleness check (not recommended in normal flows).
- (BOOL)tryLoadFseqFromPath:(NSString*)fseqPath xsqPath:(NSString*)xsqPath
    NS_SWIFT_NAME(tryLoadFseq(fseqPath:xsqPath:));

// Timing track rename / delete. `renameTiming…` wires through
// `SequenceElements::RenameTimingTrack` so effect references to
// the old name update in-place. `deleteTiming…` goes through
// `SequenceElements::DeleteElement` (which repopulates row info).
// Both return NO if the row isn't a timing row. `renameTiming…`
// also returns NO if `newName` collides with an existing timing
// track.
- (BOOL)renameTimingTrackAtIndex:(int)rowIndex newName:(NSString*)newName;
- (BOOL)deleteTimingTrackAtIndex:(int)rowIndex;

// B73: add a new variable (user-editable) timing track with the given
// name. The new track is made active (`DeactivateAllTimingElements` +
// active=true on the new one). Returns NO if `name` is empty; a
// unique suffix is auto-appended (Timing -> Timing_1) if the name
// collides with an existing track.
- (BOOL)addTimingTrackNamed:(NSString*)name;

// Fixed-interval timing track. `intervalMS = 0` creates an empty
// track (no marks); any positive value creates a fixed-interval
// track with marks at multiples of `intervalMS` from 0 to the
// sequence end. Mirrors the desktop's
// `SequenceFile::AddFixedTimingSection(name, intervalMS, ctx)`.
//
// Returns the (possibly uniquified) track name on success or empty
// string on failure (no sequence loaded, etc).
- (NSString*)addFixedIntervalTimingTrackNamed:(NSString*)name
                                    intervalMS:(int)intervalMS
    NS_SWIFT_NAME(addFixedIntervalTimingTrack(named:intervalMS:));

// Metronome with tags. Each generated mark gets one of `tags` as
// its label, cycled in order (or randomised when `randomize` is
// true and tags has 2+ entries). Empty `tags` defaults to "1"–"10".
// `minIntervalMS = -1` means "fixed interval"; positive means
// "random interval between minIntervalMS and intervalMS per mark"
// (matches the desktop's "random metronome" behaviour). Wraps
// `SequenceFile::AddMetronomeLabelTimingSection`.
- (NSString*)addMetronomeTimingTrackNamed:(NSString*)name
                                intervalMS:(int)intervalMS
                                      tags:(NSArray<NSString*>*)tags
                             minIntervalMS:(int)minIntervalMS
                                 randomize:(BOOL)randomize
    NS_SWIFT_NAME(addMetronomeTimingTrack(named:intervalMS:tags:minIntervalMS:randomize:));

// FPP Commands / FPP Effects timing track. `subType` should be
// "FPP Commands" or "FPP Effects" — those are the only values the
// renderer/exporter recognises. Wraps
// `SequenceFile::AddNewTimingSection(name, ctx, subType)`.
- (NSString*)addFPPTimingTrackNamed:(NSString*)name
                              subType:(NSString*)subType
    NS_SWIFT_NAME(addFPPTimingTrack(named:subType:));

// AI-lyrics flow: create a new timing track and bulk-populate it with
// word entries from `aiBase::GenerateLyricTrack`'s output. Times are
// rounded to the sequence's frame interval; whitespace-only words are
// skipped. Returns the final, possibly-uniquified track name on
// success or an empty string on failure (no sequence loaded, no words).
//
// `words` / `startMS` / `endMS` are parallel arrays — element i forms
// a single word entry. Mismatched lengths return empty.
- (NSString*)addLyricTimingTrackNamed:(NSString*)name
                                 words:(NSArray<NSString*>*)words
                              startMS:(NSArray<NSNumber*>*)startMS
                                endMS:(NSArray<NSNumber*>*)endMS
    NS_SWIFT_NAME(addLyricTimingTrack(named:words:startMS:endMS:));

// True when the loaded audio's HTDemucs vocals stem has been
// computed. The AI-lyrics flow can use this to decide whether to
// offer "use isolated vocals" as a UI affordance, but doesn't need
// it as a precondition — `writeCurrentToTempWav` always writes
// whatever the user has currently selected on the waveform.
- (BOOL)hasVocalsStems;

// Write whatever audio the user currently has selected on the
// waveform (RAW, STEM_VOCALS, a band-passed filter, etc.) to a
// temporary stereo float32 WAV and return its absolute path (or nil
// on failure / no audio loaded). Caller owns the file — delete
// after use.
- (nullable NSString*)writeCurrentToTempWav
    NS_SWIFT_NAME(writeCurrentToTempWav());

// Audio-file path for the sequence's currently-loaded audio, or nil
// if no audio is loaded.
- (nullable NSString*)sequenceAudioFilePath
    NS_SWIFT_NAME(sequenceAudioFilePath());

// B67 / B69: timing-mark primitives. Marks are stored as `Effect`
// entries on the timing row's `EffectLayer`; these wrap the existing
// `addEffectToRow:...` / `deleteEffectInRow:atIndex:` but add a
// rowIsTiming guard and a range-overlap check. `addTimingMark` sets
// the effect name to `label` (the phrase/word/phoneme text on
// lyric tracks; empty string for plain timing marks). Returns the
// new mark's index, or -1 on failure (row isn't a timing row,
// overlap, or sequence end overlap).
- (int)addTimingMarkAtRow:(int)rowIndex
                  startMS:(int)startMS
                    endMS:(int)endMS
                    label:(NSString*)label;
- (BOOL)deleteTimingMarkAtRow:(int)rowIndex atIndex:(int)markIndex;

// B70: rename a timing mark's label in-place. `label` may be empty
// (clearing the label). Returns NO if the row isn't a timing row
// or the index is out of range. No overlap/validation — labels are
// free text.
- (BOOL)setTimingMarkLabelAtRow:(int)rowIndex
                        atIndex:(int)markIndex
                          label:(NSString*)label
    NS_SWIFT_NAME(setTimingMarkLabel(atRow:at:label:));

// B84: break every phrase mark on a timing element into per-word
// sub-marks on layer 1. Rejects rows that aren't the phrase layer
// (layer 0) of a timing element. Discards any existing word + phoneme
// layers on the element and adds a fresh word layer. Rejects when
// any existing word/phoneme effect is locked (matches desktop
// safety check in RowHeading::BreakdownTimingPhrases).
// Returns NO on rejection.
- (BOOL)breakdownPhrasesAtRow:(int)rowIndex
    NS_SWIFT_NAME(breakdownPhrases(atRow:));

// B84 (per-mark): break a single phrase into per-word sub-marks
// without touching the other phrases on the row. Creates the words
// layer if missing; otherwise wipes any existing words + phonemes
// that fall inside the target phrase's [start, end] range and
// substitutes the fresh per-word marks. Rejects rows that aren't
// the phrase layer (layer 0) and rejects when a locked
// word/phoneme mark sits inside the range. Returns NO on rejection.
- (BOOL)breakdownPhraseAtRow:(int)rowIndex atIndex:(int)phraseIndex
    NS_SWIFT_NAME(breakdownPhrase(atRow:atIndex:));

// B85: break every word mark on the Words layer (layer 1) into
// per-phoneme sub-marks on a fresh Phonemes layer (layer 2). Uses
// the phoneme dictionary owned by `iPadRenderContext` (lazy-loaded
// from the bundled `dictionaries/` resource folder on first call).
// Rejects when the element doesn't yet have a Words layer (caller
// should Breakdown Phrases first) or when a locked phoneme mark
// is in the way. Returns NO on rejection.
- (BOOL)breakdownWordsAtRow:(int)rowIndex
    NS_SWIFT_NAME(breakdownWords(atRow:));

// B34 / B35 — 10 numbered tags (0..9) anchored to absolute sequence
// times. -1 = unset. Desktop's `SequenceElements::_tagPositions`
// already persists via the `<TimingTags>` node in the .xsq so
// save/load round-trips automatically.
- (int)tagPositionAtIndex:(int)index
    NS_SWIFT_NAME(tagPosition(at:));
- (void)setTagPositionAtIndex:(int)index positionMS:(int)position
    NS_SWIFT_NAME(setTagPosition(at:positionMS:));
- (void)clearAllTags;

// Views (view picker).
- (NSArray<NSString*>*)availableViews;
- (int)currentViewIndex;
- (void)setCurrentViewIndex:(int)viewIndex;

// MARK: - Display Elements editor (F-6)
//
// View lifecycle + per-view model membership + global element visibility
// + per-timing-track view membership. Each mutation marks the sequence
// dirty via `SequenceElements::IncrementChangeCount` and posts
// `XLViewsChanged` on NotificationCenter so the sequencer view picker
// and any open Display Elements sheet refresh. When a mutation touches
// the *current* view, row information is repopulated so a subsequent
// `reloadRows()` sees the new set.

// Views — CRUD + reorder. `addViewNamed:` / `cloneViewAtIndex:as:`
// reject empty names and collisions. `deleteViewAtIndex:` refuses the
// Master View (index 0); callers should disable the delete affordance
// for that row. `moveView…` operates on the user-view portion of the
// list — attempting to move the Master View is rejected; moving into
// index 0 is rejected.
- (BOOL)addViewNamed:(NSString*)name
    NS_SWIFT_NAME(addView(named:));
- (BOOL)deleteViewAtIndex:(int)idx
    NS_SWIFT_NAME(deleteView(atIndex:));
- (BOOL)renameViewAtIndex:(int)idx to:(NSString*)newName
    NS_SWIFT_NAME(renameView(atIndex:to:));
- (BOOL)cloneViewAtIndex:(int)idx as:(NSString*)newName
    NS_SWIFT_NAME(cloneView(atIndex:as:));
- (BOOL)moveViewUpAtIndex:(int)idx
    NS_SWIFT_NAME(moveViewUp(atIndex:));
- (BOOL)moveViewDownAtIndex:(int)idx
    NS_SWIFT_NAME(moveViewDown(atIndex:));

// Model list for the view at `idx`. Master View (0) returns every
// model Element in the sequence; user views return their
// `SequenceView` members in stored order.
- (NSArray<NSString*>*)modelsInViewAtIndex:(int)idx
    NS_SWIFT_NAME(modelsInView(atIndex:));

// Add / remove / reorder a model in the user view at `idx`. Position
// is 0-based; `-1` means "end of list". `addModel:` requires the
// model to already be in the show's Master-View element set (pick
// from `allModelNamesInShow`); fails with NO otherwise. `moveModel:`
// removes then re-inserts at the requested position. All three reject
// index 0 (Master View membership is derived, not stored).
- (BOOL)addModel:(NSString*)name toViewAtIndex:(int)idx atPosition:(int)pos
    NS_SWIFT_NAME(addModel(_:toViewAtIndex:atPosition:));
- (BOOL)removeModel:(NSString*)name fromViewAtIndex:(int)idx
    NS_SWIFT_NAME(removeModel(_:fromViewAtIndex:));
- (BOOL)moveModel:(NSString*)name inViewAtIndex:(int)idx toPosition:(int)pos
    NS_SWIFT_NAME(moveModel(_:inViewAtIndex:toPosition:));

// Roster for the "members" pane of the editor — Master-View *Elements*
// (models the sequence has opted in, plus every timing track). A model
// being in the show's `ModelManager` does NOT imply it's in the
// sequence; Effect Sequences start with zero models here.
- (NSArray<NSString*>*)allModelNamesInShow;   // Master-View model Elements
- (NSArray<NSString*>*)allTimingTrackNames;   // every TimingElement

// Models that live in the show layout (`ModelManager`) but are not
// yet Elements in this sequence. Drives the "Available" pane when the
// Master View is selected — picking one adds it to the sequence via
// `addModelToMasterView:`. Empty if no show folder is loaded.
- (NSArray<NSString*>*)modelsAvailableInShowLayout;

// Add a show-layout model to the sequence's Master View. Creates a
// new `Element` with one fresh effect layer, matching desktop
// `ViewsModelsPanel::AddSelectedModels` for the MASTER_VIEW branch.
// Fails if the name isn't in `ModelManager` or already exists as an
// Element. Rebuilds row information and fires `XLViewsChanged`.
- (BOOL)addModelToMasterView:(NSString*)name;

// True iff the named Element has any effects on any of its effect
// layers. Used by the remove-confirmation dialog to warn users
// before a `removeElementFromMasterView:` delete (which destroys
// effects and layers along with the Element).
- (BOOL)elementHasEffects:(NSString*)name;

// Remove an Element from the sequence entirely (delegates to
// `SequenceElements::DeleteElement`). Aborts in-flight render jobs
// first so workers don't dereference the about-to-be-deleted
// pointers (matches desktop issue #4134). Silently drops any
// effects on the element. Also removes the element from every user
// view's model-name list. Returns NO if no such Element exists.
- (BOOL)removeElementFromMasterView:(NSString*)name;

// Global element visibility. Models drive `Element::SetVisible`; for
// timings this toggles `TimingElement::SetMasterVisible` (visibility
// in the Master View). Per-non-Master-view timing visibility is
// stored in the timing's `views` CSV — see
// `addTiming:toViewNamed:` below.
- (BOOL)elementVisible:(NSString*)name;
- (BOOL)setElementVisible:(NSString*)name visible:(BOOL)visible;

// Per-view membership for a timing track, via
// `TimingElement::mViews` (comma-separated view names). An entry in
// the CSV means "this timing is visible when that view is active".
// The Master-Visible flag (above) governs Master-View visibility
// separately.
- (NSArray<NSString*>*)viewsContainingTiming:(NSString*)timingName;
- (BOOL)addTiming:(NSString*)timingName toViewNamed:(NSString*)viewName;
- (BOOL)removeTiming:(NSString*)timingName fromViewNamed:(NSString*)viewName;
// B82 — add a timing track to every non-Master view in one call.
- (BOOL)addTimingToAllViews:(NSString*)timingName;

// dynamicOptions sources for JSON `choice` properties. Mirrors the desktop
// repopulate lambdas in JsonEffectPanel (file:1777-1884). Empty arrays on
// lookup failure — never nil. See EffectPropertyView for dispatch.

// All timing tracks in the sequence with <= 1 effect layer (i.e. not lyric).
- (NSArray<NSString*>*)timingTrackNames;
// Timing tracks with exactly 3 layers (phrase / word / phoneme).
- (NSArray<NSString*>*)lyricTimingTrackNames;

// Per-preview cameras available for `B_CHOICE_PerPreviewCamera`:
// always starts with "2D", followed by each 3D camera name from the
// show's `ViewpointMgr` (loaded from `<Viewpoints>` in
// `xlights_rgbeffects.xml` during Phase D-3). Mirrors desktop's
// `BufferPanel::OnBufferStyleChoiceSelect` population.
- (NSArray<NSString*>*)perPreviewCameraNames;

// Per-effect ColorCurve mode availability (G16 — C5). Some
// effects only make sense with linear time-curves, others only
// radial, a handful support both. Desktop calls
// `RenderableEffect::SupportsLinearColorCurves` /
// `SupportsRadialColorCurves` with the current effect's settings
// map; iPad's ColorCurve editor uses the same flags to grey out
// unavailable mode groups in the time/spatial picker.
//
// Returns @{@"linear": NSNumber (BOOL), @"radial": NSNumber (BOOL)}
// for the selected effect. Nil (empty dict) when no effect is
// selected. Always returns linear+radial true for now on iPad
// because the iPad doesn't track an "active palette slot" yet.
- (NSDictionary<NSString*, NSNumber*>*)colorCurveModeSupportForRow:(int)rowIndex
                                                            atIndex:(int)effectIndex;

// Palette save / load / import / export (G17 — C5). Palette files
// are plain-text `.xpalette` under `<showFolder>/Palettes/` plus
// any bundled-in-resources `palettes/` folder. The serialised
// format is the 8-slot comma-separated string that desktop's
// `ColorPanel::GetCurrentPalette` produces: each slot is either a
// `#RRGGBB` hex colour or an `Active=TRUE|…` ColorCurve blob.
// Round-trips byte-for-byte with desktop.

// List every saved palette visible to the app. Each entry is
// @{@"filename": <basename>.xpalette, @"palette": <8-slot string>}.
// Scans `<showFolder>/Palettes/` first, then app-bundled
// `palettes/` in Resources. Duplicates (same palette string
// already loaded) are dropped.
- (NSArray<NSDictionary<NSString*, NSString*>*>*)savedPalettes;

// Write `paletteString` to `<showFolder>/Palettes/<name>.xpalette`.
// `name` is sanitized for filesystem safety. Pass nil / empty to
// auto-generate `PAL001.xpalette` (incrementing to avoid collisions).
// Returns the on-disk filename on success, nil on failure (no
// show folder, unwritable, invalid input).
- (nullable NSString*)savePaletteString:(NSString*)paletteString
                                  asName:(nullable NSString*)name;

// Remove a previously-saved palette file. `filename` is the
// basename returned by `savedPalettes`. Only removes files under
// the show folder — bundled palettes are read-only. Returns YES
// on successful delete.
- (BOOL)deleteSavedPalette:(NSString*)filename;

// Assemble the current 8-slot palette for the effect at row/index
// into the desktop-compatible serialised string
// ("#RRGGBB,#RRGGBB,...," or with `Active=TRUE|…` blobs for curve
// slots). Reads directly from the effect's settings map so the
// output matches what the renderer sees. Empty string if no effect
// is selected.
- (NSString*)currentPaletteStringForRow:(int)rowIndex
                                atIndex:(int)effectIndex;

// Apply a serialised palette string to the effect at row/index:
// parses slots and writes `C_BUTTON_Palette1..8` — mirrors
// desktop's `LoadColorsToButtons`. Leaves the per-slot enable
// checkboxes (`C_CHECKBOX_Palette1..8`) untouched, same as desktop.
// Returns YES on successful parse + apply.
- (BOOL)applyPaletteString:(NSString*)paletteString
                     toRow:(int)rowIndex
                   atIndex:(int)effectIndex;

// Value-curve preset load / save (G36 — C6). `.xvc` files are the
// same XML format desktop reads/writes
// (`<valuecurve data="<serialised>"/>`), stored under
// `<showFolder>/valuecurves/` plus any bundled `valuecurves/` in
// app resources.

// List every saved value curve visible to the app. Each entry is
// @{@"filename": <basename>.xvc, @"serialised": <VC string>}.
// Duplicates (same serialised body already loaded) are dropped.
- (NSArray<NSDictionary<NSString*, NSString*>*>*)savedValueCurves;

// Write `serialised` (a ValueCurve::Serialise() string) into an
// `.xvc` under `<showFolder>/valuecurves/<name>.xvc`. Name is
// sanitised to alphanumerics; pass nil / empty to auto-generate
// `VC001.xvc`. Returns the on-disk filename, or nil on failure.
- (nullable NSString*)saveValueCurveSerialised:(NSString*)serialised
                                         asName:(nullable NSString*)name;

// Delete a saved value curve by basename. Only removes files
// under `<showFolder>/valuecurves/`; bundled presets are read-
// only. Returns YES on success.
- (BOOL)deleteSavedValueCurve:(NSString*)filename;

// Model-scoped sources. Uses the effect's parent element's ModelName to
// resolve a Model; ModelGroups are unwrapped to their first contained
// model, matching desktop (JsonEffectPanel.cpp:1815-1818). Empty on
// unresolvable model.
- (NSArray<NSString*>*)statesForRow:(int)rowIndex atIndex:(int)effectIndex;
- (NSArray<NSString*>*)facesForRow:(int)rowIndex atIndex:(int)effectIndex;
- (NSArray<NSString*>*)modelNodeNamesForRow:(int)rowIndex atIndex:(int)effectIndex;

// Effect-scoped: RenderableEffect::GetSettingOptions(settingId). Returns
// {} for most effects; SingleStrand overrides it to return the WLED FX /
// palette name lists (SingleStrandEffect.cpp:100-131). Matches desktop's
// "effect" dynamicOptions source.
- (NSArray<NSString*>*)effectSettingOptionsForRow:(int)rowIndex
                                          atIndex:(int)effectIndex
                                         settingId:(NSString*)settingId;

// Effects for a given row
- (int)effectCountForRow:(int)rowIndex;

// Effect data — returns arrays parallel to each other
- (NSArray<NSString*>*)effectNamesForRow:(int)rowIndex;
- (NSArray<NSNumber*>*)effectStartTimesForRow:(int)rowIndex;
- (NSArray<NSNumber*>*)effectEndTimesForRow:(int)rowIndex;

// Model preview — sets channel data on models for a given frame
- (void)setModelColorsAtMS:(int)frameMS;

// Opaque pointer to iPadRenderContext for Metal bridge
- (void*)renderContext;

// House Preview layout groups. "Default" (always first) + named groups
// from `<layoutGroups>` in xlights_rgbeffects.xml. Setting an unknown
// name falls back to "Default". Setter posts
// `XLLayoutGroupChanged` on NotificationCenter so preview panes can
// invalidate their background-texture caches.
- (NSArray<NSString*>*)layoutGroups;
- (NSString*)activeLayoutGroup;
- (void)setActiveLayoutGroup:(NSString*)name;

// Desktop's last-used House Preview 3D-vs-2D mode, read from
// `<settings><LayoutMode3D>` at show-folder load. Used as the initial
// value for the House Preview's is3D toggle; not written back since
// iPad layout editing stays desktop-only.
- (BOOL)layoutMode3D;

// MARK: - Layout Editor (Phase J-0, read-only)

// Names of every model visible in the active layout group, in the
// order desktop renders them (`iPadRenderContext::GetModelsForActivePreview`).
// ModelGroup children are expanded; duplicates are NOT suppressed —
// matches desktop's UpdateModelsList behaviour.
- (NSArray<NSString*>*)modelsInActiveLayoutGroup;

// Read-only snapshot of one model's layout-relevant state. Returns
// nil for unknown names. Keys (all NSString unless noted):
//   "name"              — model's display name
//   "displayAs"         — model type string ("Arch", "Tree", "Custom", …)
//   "centerX"/"Y"/"Z"   — NSNumber (double) world-space centre
//   "width"/"height"/"depth" — NSNumber (double) world-space size
//   "rotateX"/"Y"/"Z"   — NSNumber (double) Euler rotation in degrees
//   "locked"            — NSNumber (BOOL)
//   "layoutGroup"       — NSString
//   "controllerName"    — NSString (empty if unassigned)
//   "startChannel"      — NSNumber (uint32)
//   "endChannel"        — NSNumber (uint32)
//   "stringCount"       — NSNumber (int)
//   "nodeCount"         — NSNumber (int)
- (nullable NSDictionary<NSString*, id>*)modelLayoutSummary:(NSString*)name;

// Mutate a single layout-relevant property on `name`. Returns YES
// if the value changed; NO if `name` was unknown, the property key
// was unsupported, the model is locked (for setters that respect
// the lock), or the value was already at the requested setting.
//
// The change is staged in memory and tracked in
// `_dirtyLayoutModels`; nothing hits disk until `saveLayoutChanges`
// is called. Property keys mirror the `modelLayoutSummary` keys
// (Phase J-1 v1, common-properties surface):
//
//   "centerX"/"centerY"/"centerZ"  — NSNumber (double)
//   "width"/"height"/"depth"       — NSNumber (double)
//   "rotateX"/"rotateY"/"rotateZ"  — NSNumber (double, degrees)
//   "locked"                       — NSNumber (BOOL)
//   "layoutGroup"                  — NSString
//   "controllerName"               — NSString
//
// Per-model-type properties (string count, custom-model matrix,
// DMX channel mapping, …) land in J-3.
- (BOOL)setLayoutModelProperty:(NSString*)name
                           key:(NSString*)key
                         value:(id)value;

// Flush every pending layout-property change to
// `xlights_rgbeffects.xml`. Returns YES if the write succeeded (or
// if nothing was dirty). One disk round-trip per call —
// SwiftUI callers should batch by editing in-memory and saving on
// commit / lose-focus / scene-close, not per-keystroke.
- (BOOL)saveLayoutChanges;

// Phase J-2 (touch UX) — read / write the `axis_tool` member on
// the named model's screen location. Drives which descriptor
// handles `GetHandles` emits (translate arrows / rotate rings /
// scale cubes / etc.). Returns "translate" / "rotate" / "scale" /
// "xy_trans" / "elevate" / "none". Not part of the layout
// dirty / undo path — tool selection is a UI mode, not model
// state to be persisted.
- (NSString*)axisToolForModel:(NSString*)modelName;
- (BOOL)setAxisTool:(NSString*)tool forModel:(NSString*)modelName;

// Phase J-2 (touch UX) — vertex / curve operations on PolyLine /
// MultiPoint / Custom models. Mirror the desktop right-click
// menu entries (Delete Point / Add Point / Define Curve / Remove
// Curve). All push a layout-undo snapshot internally and mark the
// model dirty on success. Vertex / segment indices are 0-based;
// the bridge handles the legacy 1-based int conversion.
- (BOOL)deleteVertexAtIndex:(NSInteger)vertexIndex forModel:(NSString*)modelName;
- (BOOL)insertVertexInSegment:(NSInteger)segmentIndex forModel:(NSString*)modelName;
- (BOOL)setCurve:(BOOL)create onSegment:(NSInteger)segmentIndex forModel:(NSString*)modelName;

// Phase J-3 (touch UX) — model creation. The curated subset of
// model types the iPad's Add-Model picker exposes. Desktop's
// `ModelManager::CreateDefaultModel` accepts more types
// (DmxMovingHead, Spinner, etc.); J-3 lifts more in once the per-
// type property pages land.
- (NSArray<NSString*>*)availableModelTypesForCreation;

// Phase J-3 (touch UX) — delete a model from the active layout.
// Used both by Add-Model cancel ("the user tapped + then changed
// their mind") and by the inline action bar's future Delete
// affordance. Does NOT participate in the existing layout undo
// system (which only captures property changes); the action bar
// confirmation flow is the only safety net. Returns NO if the
// model isn't present.
- (BOOL)deleteModel:(NSString*)modelName;

// YES iff at least one layout-property edit is staged in memory
// and not yet persisted. The Layout Editor uses this to gate a
// "Save" button + warn-on-close.
- (BOOL)hasUnsavedLayoutChanges;

// Drop the dirty-layout set without writing to disk. Used after
// Discard Changes — every undo restore re-marked the model dirty
// even though the in-memory state now matches what's on disk.
- (void)clearDirtyLayoutChanges;

// Phase J-2 — layout undo. Snapshot the named model's common-
// properties surface (centre, dimensions, rotation, locked,
// layoutGroup, controllerName) before making an edit; the most
// recent snapshot is restored by `undoLastLayoutChange`. Stack is
// capped at 100 entries inside the render context. Multiple
// pushes for the same model just stack — undo walks them one at
// a time. The drag handler pushes once at gesture-began, so a
// single drag is one undo entry.
- (void)pushLayoutUndoSnapshotForModel:(NSString*)modelName;

// Pop and restore the most recent undo entry. Returns YES if a
// snapshot was applied (model still exists, dirty marked); NO if
// the stack was empty or the model has since been removed.
- (BOOL)undoLastLayoutChange;

// YES iff there's at least one layout-undo snapshot pending.
- (BOOL)canUndoLayoutChange;

// Read-only snapshot of layout-editor display state. Keys:
//   "backgroundImage"        — NSString (FixFile-resolved, "" if none)
//   "backgroundBrightness"   — NSNumber (int 0..100)
//   "backgroundAlpha"        — NSNumber (int 0..100)
//   "scaleBackgroundImage"   — NSNumber (BOOL)
//   "display2DGrid"          — NSNumber (BOOL)
//   "display2DGridSpacing"   — NSNumber (long, world units)
//   "display2DBoundingBox"   — NSNumber (BOOL)
//   "display2DCenter0"       — NSNumber (BOOL)
//   "previewWidth"/"previewHeight" — NSNumber (int) virtual canvas size
//   "layoutMode3D"           — NSNumber (BOOL)
- (NSDictionary<NSString*, id>*)layoutDisplayState;

// Effect editing
- (BOOL)addEffectToRow:(int)rowIndex
                  name:(NSString*)effectName
               startMS:(int)startMS
                 endMS:(int)endMS;
- (BOOL)deleteEffectInRow:(int)rowIndex atIndex:(int)effectIndex;
- (BOOL)moveEffectInRow:(int)rowIndex
                atIndex:(int)effectIndex
              toStartMS:(int)newStartMS
                toEndMS:(int)newEndMS;
- (NSArray<NSString*>*)availableEffectNames;

// Effect settings for selected effect
- (NSDictionary<NSString*, NSString*>*)effectSettingsForRow:(int)rowIndex atIndex:(int)effectIndex;
- (NSDictionary<NSString*, NSString*>*)effectPaletteForRow:(int)rowIndex atIndex:(int)effectIndex;

// Effect metadata — returns JSON string as loaded from resources/effectmetadata/<Name>.json
// Empty string if no metadata is available for the effect.
- (NSString*)metadataJsonForEffectNamed:(NSString*)effectName;

// Shader dynamic properties — parses the .fs file at `shaderPath` and
// returns a JSON-encoded array of property entries that drop into
// `EffectPropertyView` using the same schema as static metadata. Empty
// string if the file isn't a parseable shader or the path is empty.
- (NSString*)shaderDynamicPropertiesJsonForPath:(NSString*)shaderPath;

// Shared metadata — returns JSON string for Buffer/Color/Timing shared panels.
// name must be one of "Buffer", "Color", "Timing".
- (NSString*)sharedMetadataJsonNamed:(NSString*)name;

// Read a single effect setting value by key.
// Keys with C_ prefix come from palette map; everything else from settings map.
- (NSString*)effectSettingValueForKey:(NSString*)key
                                inRow:(int)rowIndex
                              atIndex:(int)effectIndex;

// Write a single effect setting value. Returns YES if the value changed.
// Does NOT trigger a re-render — caller should invoke renderEffectForRow after
// batching setting changes.
- (BOOL)setEffectSettingValue:(NSString*)value
                       forKey:(NSString*)key
                        inRow:(int)rowIndex
                      atIndex:(int)effectIndex;

// Remove a setting from the effect (used for properties with
// suppressIfDefault=true when the new value equals the default, so the
// settings map doesn't persist a redundant default). Returns YES if the
// key was present and was removed.
- (BOOL)removeEffectSettingForKey:(NSString*)key
                            inRow:(int)rowIndex
                          atIndex:(int)effectIndex;

// Fade in/out (seconds, stored as T_TEXTCTRL_Fadein / T_TEXTCTRL_Fadeout).
- (float)effectFadeInSecondsForRow:(int)rowIndex atIndex:(int)effectIndex;
- (float)effectFadeOutSecondsForRow:(int)rowIndex atIndex:(int)effectIndex;
- (BOOL)setEffectFadeInSeconds:(float)seconds
                          forRow:(int)rowIndex
                         atIndex:(int)effectIndex;
- (BOOL)setEffectFadeOutSeconds:(float)seconds
                           forRow:(int)rowIndex
                          atIndex:(int)effectIndex;

// Resize one edge of an effect. edge: 0 = left/start, 1 = right/end.
// Clamps to neighbors; returns NO if clamp would invert the range.
- (BOOL)resizeEffectEdgeInRow:(int)rowIndex
                       atIndex:(int)effectIndex
                          edge:(int)edge
                        toMS:(int)newMS;

// Lock / render-disable state.
- (BOOL)effectIsLockedInRow:(int)rowIndex atIndex:(int)effectIndex;
- (void)setEffectLocked:(BOOL)locked inRow:(int)rowIndex atIndex:(int)effectIndex;
- (BOOL)effectIsRenderDisabledInRow:(int)rowIndex atIndex:(int)effectIndex;
- (void)setEffectRenderDisabled:(BOOL)disabled inRow:(int)rowIndex atIndex:(int)effectIndex;

// Copy: returns full settings string (xLights legacy format) and the effect's
// palette as a separate string. Empty strings on failure.
- (NSString*)effectSettingsStringForRow:(int)rowIndex atIndex:(int)effectIndex;
- (NSString*)effectPaletteStringForRow:(int)rowIndex atIndex:(int)effectIndex;
- (NSString*)effectNameForRow:(int)rowIndex atIndex:(int)effectIndex;

// B15: replace an effect's settings + palette wholesale. Used by
// Randomize / Reset bulk ops and (future) preset-apply. Empty
// palette string skips palette replacement.
- (BOOL)replaceEffectSettings:(NSString*)settings
                      palette:(NSString*)palette
                        inRow:(int)rowIndex
                      atIndex:(int)effectIndex
    NS_SWIFT_NAME(replaceEffectSettings(_:palette:inRow:atIndex:));

// B20: free-text description for an effect, stored in the
// `X_Effect_Description` settings key. The X_ prefix means
// `SetSettings(… keepxsettings=true)` preserves it across
// Randomise / Reset / preset-apply. Empty string clears the
// entry. Caller should call `renderEffectForRow` afterward if the
// effect is visible.
- (NSString*)effectDescriptionForRow:(int)rowIndex atIndex:(int)effectIndex
    NS_SWIFT_NAME(effectDescription(forRow:atIndex:));
- (BOOL)setEffectDescription:(NSString*)description
                        forRow:(int)rowIndex
                       atIndex:(int)effectIndex
    NS_SWIFT_NAME(setEffectDescription(_:forRow:atIndex:));

// Paste / scripted add: insert a new effect with settings+palette pre-populated.
// Returns the index of the new effect, or -1 on failure.
- (int)addEffectToRow:(int)rowIndex
                 name:(NSString*)effectName
             settings:(NSString*)settings
              palette:(NSString*)palette
              startMS:(int)startMS
                endMS:(int)endMS;

// Kick off a background re-render for the range of a single effect's model.
- (void)renderEffectForRow:(int)rowIndex atIndex:(int)effectIndex;

// Re-render an arbitrary time range on a row's model (used after delete so
// the cleared output is refreshed).
- (void)renderRangeForRow:(int)rowIndex
                  startMS:(int)startMS
                    endMS:(int)endMS
                    clear:(BOOL)clear;

// Controller output
- (BOOL)startOutput;
- (void)stopOutput;
- (BOOL)isOutputting;
- (void)outputFrame:(int)frameMS;
- (NSInteger)outputCount;

// Rendering
- (void)renderAll;
- (BOOL)isRenderDone;

// YES if the most recent render had at least one job aborted before
// completion (typically because of a memory-pressure signal — see
// HandleMemoryWarning). Counter resets at every render start, so this
// reflects only the latest pass. Callers about to persist
// `SequenceData` (fseq write, batch render) should consult it: an
// aborted render leaves the data buffer partly stale.
- (BOOL)wasRenderAborted;

// Coarse render-progress fraction (0..1) for the in-flight render of the
// currently-loaded sequence. Aggregates per-row job frame counters against
// the sequence's frame range. Returns 1.0 when no render is active.
- (float)renderProgressFraction;
// Signal all in-flight render jobs to abort and block until they've
// completed (or `timeoutSeconds` elapses). Returns YES if the render
// is fully quiesced by the time the call returns. Call on shutdown /
// sequence-close paths before tearing down `SequenceElements` et al —
// the render workers hold pointers into those structures and would
// crash otherwise.
- (BOOL)abortRenderAndWait:(NSTimeInterval)timeoutSeconds;

// Memory pressure
- (void)handleMemoryWarning;
- (void)handleMemoryCritical;
+ (int64_t)availableMemoryMB;

// House preview pixel data at a given time
// Returns NSData containing packed float x, y and uint8 r, g, b per pixel
- (int)pixelCountAtMS:(int)frameMS;
- (nullable NSData*)pixelDataAtMS:(int)frameMS;

// Audio playback
- (BOOL)hasAudio;
- (void)audioPlay;
- (void)audioPause;
- (void)audioStop;
- (void)audioSeekToMS:(long)positionMS;

// B40: play a short audio window (`lengthMS` ms starting at
// `positionMS`) without disturbing the main play/pause state. Used
// to give an audible cue while the user drags the playhead. Safe
// to call rapidly — AudioManager::Play(pos, len) handles overlapping
// requests by re-seeking. No-op when audio isn't loaded or when
// regular playback is in flight (we don't want to fight the
// main playback engine).
- (void)audioPlaySegmentFromMS:(long)positionMS lengthMS:(long)lengthMS
    NS_SWIFT_NAME(audioPlaySegment(fromMS:lengthMS:));
- (long)audioTellMS;
- (int)audioPlayingState;  // 0=PLAYING, 1=PAUSED, 2=STOPPED
- (void)setAudioVolume:(int)volume;
- (int)audioVolume;

// F-4 playback speed. Mirrors desktop's 8 discrete speed options
// (0.25x / 0.5x / 0.75x / 1.0x / 1.5x / 2x / 3x / 4x) routed
// through `AudioManager::SetPlaybackRate`. On iOS this drives the
// AVAudioEngine time-pitch unit so audio stays audible at off-1x
// speeds without a pitch shift. No-audio sequences scale their
// timer tick in Swift instead — this call is a no-op there.
- (void)setAudioPlaybackRate:(float)rate;

// Waveform data — returns downsampled peaks for display
// Returns per-bucket float triples `{min, max, rms}` for the given
// time range. `peaks[i*3+0]` = bucket min (<=0), `peaks[i*3+1]` = max
// (>=0), `peaks[i*3+2]` = RMS (>=0, used by A10 RMS overlay).
- (nullable NSData*)waveformDataFromMS:(long)startMS
                         toMS:(long)endMS
                   numSamples:(int)numSamples;

// B41: same as above, with a filter type parameter matching
// `AUDIOSAMPLETYPE` (0=RAW, 1=BASS, 2=TREBLE, 3=ALTO, 4=NONVOCALS).
// A9.1 extends the id space with 5=CUSTOM — a parametric band filter
// whose MIDI-note range is taken from `lowNote` / `highNote` (see the
// `lowNote:highNote:` overload). If `GetFilteredAudioData` returns
// null (unfiltered source), the method falls back to the raw waveform
// rather than returning an empty buffer.
- (nullable NSData*)waveformDataFromMS:(long)startMS
                         toMS:(long)endMS
                   numSamples:(int)numSamples
                   filterType:(int)filterType
    NS_SWIFT_NAME(waveformData(fromMS:toMS:numSamples:filterType:));

// A9.1 parametric-band variant. `lowNote` / `highNote` are MIDI note
// numbers (0–127) and are only consulted for filterType=5 (CUSTOM);
// other filter types use their hardcoded ranges. Callers that don't
// need the custom band should use the 4-argument overload.
- (nullable NSData*)waveformDataFromMS:(long)startMS
                         toMS:(long)endMS
                   numSamples:(int)numSamples
                   filterType:(int)filterType
                      lowNote:(int)lowNote
                     highNote:(int)highNote
    NS_SWIFT_NAME(waveformData(fromMS:toMS:numSamples:filterType:lowNote:highNote:));

// B43: alternate audio tracks (vocal stems, etc.). Used to switch
// the *waveform* view between the main sequence audio and any of
// the alt tracks declared in the .xsq. Playback always uses the
// main track. -1 = main, 0..altTrackCount-1 = alt index.
- (NSInteger)altTrackCount;
- (NSString*)altTrackDisplayNameAtIndex:(NSInteger)index
    NS_SWIFT_NAME(altTrackDisplayName(at:));
- (NSInteger)activeWaveformTrack;
- (void)setActiveWaveformTrack:(NSInteger)index;

// Phase E follow-up — alt-track CRUD for the Sequence Settings
// Audio Tracks tab. `addAltTrack:` / `setAltTrackPath:` route the
// path through `SequenceFile::AddAltTrack` /
// `SequenceFile::SetAltTrackPath` which use the show folder as the
// FixFile root. `altTrackPath:` returns the resolved absolute path
// (empty when unresolved); `altTrackShortname:` returns the user
// label (may be empty — display fallback is "Track N"). All
// mutators mark the sequence dirty.
- (NSString*)altTrackPathAtIndex:(NSInteger)index
    NS_SWIFT_NAME(altTrackPath(at:));
- (NSString*)altTrackShortnameAtIndex:(NSInteger)index
    NS_SWIFT_NAME(altTrackShortname(at:));
- (BOOL)addAltTrackAtPath:(NSString*)path shortname:(NSString*)shortname
    NS_SWIFT_NAME(addAltTrack(atPath:shortname:));
- (BOOL)removeAltTrackAtIndex:(NSInteger)index
    NS_SWIFT_NAME(removeAltTrack(at:));
- (BOOL)setAltTrackPathAtIndex:(NSInteger)index path:(NSString*)path
    NS_SWIFT_NAME(setAltTrackPath(at:path:));
- (BOOL)setAltTrackShortnameAtIndex:(NSInteger)index shortname:(NSString*)shortname
    NS_SWIFT_NAME(setAltTrackShortname(at:shortname:));

// A2 onset detection. Runs the spectral-flux detector over the full
// audio track, returning onset positions in milliseconds (ascending).
// `sensitivity` is the adaptive-threshold multiplier: higher = fewer
// onsets. Blocks until audio is loaded — expect a few hundred ms for
// typical 3–4 minute tracks on modern iPad hardware.
- (NSArray<NSNumber*>*)detectOnsetsWithSensitivity:(float)sensitivity
    NS_SWIFT_NAME(detectOnsets(sensitivity:));

// A7 sound classification. Runs Apple's SNClassifySoundRequest over
// the entire audio track and returns the top-N sound classes with
// per-second confidence curves. Keys of the returned dictionary are
// class names ("music.drums", "music.vocals", …); values are
// `[NSNumber]` floats in [0, 1], one per `timeStepSeconds` (also
// reported). Blocks — typical 3–4 minute tracks take a few seconds
// on Apple Silicon.
- (nullable NSDictionary*)classifySound
    NS_SWIFT_NAME(classifySound());
// Time-step (seconds) for the last `classifySound` call. Set by the
// call above; 0 if classification has never been run on this track.
@property(nonatomic, readonly) float lastClassificationTimeStep;

// A4 tempo detection. Runs autocorrelation on the onset envelope
// to find the most likely period, then phase-locks a Dirac comb to
// produce beat positions. Returns a dictionary with:
//   "bpm"        — NSNumber(float)    — detected BPM, 0 on failure
//   "confidence" — NSNumber(float)    — 0..1 heuristic confidence
//   "beats"      — NSArray<NSNumber*> — ascending ms beat positions
// Blocks; a 3–4 minute track finishes in well under a second.
- (NSDictionary*)detectTempo
    NS_SWIFT_NAME(detectTempo());

// A5 pitch contour. Per-frame fundamental-frequency estimate from
// FFT-based autocorrelation. Returns a flat Float array laid out as
// (timeMS, frequency, confidence) triples (3 entries per sample).
// Unvoiced frames have frequency=0 but still carry a confidence.
- (nullable NSData*)detectPitchContour
    NS_SWIFT_NAME(detectPitchContour());

// Route the currently-picked waveform filter into the audio engine
// so playback follows the display. `filterType` matches the int
// convention of `waveformData(...)` (0 RAW, 1 BASS, …, 5 CUSTOM,
// 6 LUFS, 7 VOCALS, 8..11 STEM_*). `lowNote` / `highNote` are the
// MIDI bounds for CUSTOM; ignored otherwise. Internally dispatches
// through `AudioManager::SwitchTo` which blocks on audio load +
// filter build, so callers should prefer a background queue (e.g.
// `Task.detached`) rather than the main actor. No-op when no audio
// is loaded.
- (void)applyPlaybackFilterType:(int)filterType
                          lowNote:(int)lowNote
                         highNote:(int)highNote
    NS_SWIFT_NAME(applyPlaybackFilter(type:lowNote:highNote:));

// A9 chord + key detection via chromagram + 24 major/minor templates
// and Krumhansl–Schmuckler. Returns:
//   "key"    — NSString (e.g. "C major", "A minor"; empty on failure)
//   "chords" — NSArray<NSDictionary*> with {startMS, endMS, name}
- (NSDictionary*)detectChords
    NS_SWIFT_NAME(detectChords());

// A6 spectrogram computation. Runs the STFT once and retains the
// magnitude buffer on the bridge; returns YES on success. Subsequent
// `spectrogramBGRAForRangeMS:...` calls resample the cached buffer
// into viewport-sized BGRA images without recomputing the FFT.
- (BOOL)ensureSpectrogramComputed
    NS_SWIFT_NAME(ensureSpectrogramComputed());

// A8 stem separation (macOS 12+ / iOS 15+ only — ML model uses
// Float16 multi-arrays). All four methods no-op on older OS.

// Roots (show folder + configured media folders) where the model
// could be installed, in preference order. UI uses this to present
// a picker.
- (NSArray<NSString*>*)stemModelCandidateRoots
    NS_SWIFT_NAME(stemModelCandidateRoots());

// Scans candidate roots for an already-installed
// `HTDemucs_SourceSeparation_F32.mlpackage` (including nested-from-
// zip layouts which get lifted to the canonical path). Returns the
// absolute path, or nil if not present.
- (nullable NSString*)findInstalledStemModelPath
    NS_SWIFT_NAME(findInstalledStemModelPath());

// First-run installer. Downloads the model zip to
// `<root>/ai-models/`, unzips, lifts the nested `.mlpackage` up to
// the canonical location. `progress` fires on the main queue with
// 0..100. `completion` fires on the main queue with the final model
// path (nil on failure / cancel).
- (void)installStemModelToRoot:(NSString*)root
                        progress:(void(^)(int pct))progress
                      completion:(void(^)(NSString* _Nullable installedPath))completion
    NS_SWIFT_NAME(installStemModel(toRoot:progress:completion:));

// Asynchronous stem separation. Dispatches CoreML inference to a
// background queue (required — CoreML warns if called on main) and
// invokes `progress` / `completion` on the main queue. On success
// the result is stashed on the underlying AudioManager via
// `SetStemData` so subsequent `waveformData(...filterType:8..11)`
// queries serve the stems.
- (void)runStemSeparationAtPath:(NSString*)modelPath
                        progress:(void(^)(int pct))progress
                      completion:(void(^)(BOOL ok))completion
    NS_SWIFT_NAME(runStemSeparation(atPath:progress:completion:));

// Renders the cached spectrogram at [startMS, endMS] into an
// `outWidth × outHeight` BGRA buffer (length = w*h*4). Returns nil
// if the spectrogram hasn't been computed yet.
- (nullable NSData*)spectrogramBGRAForRangeMS:(long)startMS
                                toMS:(long)endMS
                               width:(int)outWidth
                              height:(int)outHeight
    NS_SWIFT_NAME(spectrogramBGRA(fromMS:toMS:width:height:));

// Effect-background batch append. Mirrors desktop's
// `EffectsGrid::DrawEffectBackground` helper — resolves the
// RenderableEffect + color mask, then calls
// `RenderableEffect::DrawEffectBackground` with the bridge's current
// effect-background accumulator. Caller must have wrapped the visible-
// effects loop in `-beginEffectBackgroundBatch` / `-flushEffectBackgroundBatch`
// on the bridge. Coordinates are in logical pixel space, top-left
// origin (matches the grid's coord system — desktop uses bottom-left
// but the accumulator just stores the numbers, so we stay consistent
// with the grid's other calls).
//
// Returns the draw-icon hint desktop uses:
//   0 — effect drew a full background, skip the icon
//   1 — show the normal-size icon
//   2 — show a smaller icon (leaves room for partial background)
// Bridge is declared `id` in the header so this file doesn't have to
// import `XLGridMetalBridge.h`; the .mm casts it back.
- (int)appendEffectBackgroundForRow:(int)rowIndex
                            atIndex:(int)effectIndex
                                 x1:(float)x1
                                 y1:(float)y1
                                 x2:(float)x2
                                 y2:(float)y2
                             bridge:(id)bridge
                          drawRamps:(BOOL)drawRamps;

// Media picker — sequence-wide enumeration + thumbnails.
// `mediaPathsInSequence` returns every media file referenced by effects
// in the currently-open sequence as a list of
// @{@"path": NSString, @"type": NSString (image|svg|shader|text|binary|video)}
// dictionaries. Backed by `SequenceMedia::GetAllMediaPaths()`, which
// walks the media cache — iPad renders once on sequence open, so every
// referenced file has landed there by the time the user opens a picker.
- (NSArray<NSDictionary<NSString*, NSString*>*>*)mediaPathsInSequence;

// Richer sequence-wide media inventory (G28 — C5). Every entry in
// the sequence's media cache with status flags + metadata the media
// manager view surfaces:
//
//   path         — the stored path (key used by settings maps)
//   type         — image|svg|shader|text|binary|video
//   resolvedPath — FixFile-resolved absolute path (empty if
//                  unresolvable)
//   isEmbedded   — NSNumber BOOL: data lives in the .xsq
//   isBroken     — NSNumber BOOL: not embedded AND resolved file
//                  doesn't exist on disk
//   widthPx      — NSNumber int, images only (0 otherwise)
//   heightPx     — NSNumber int, images only
//   frameCount   — NSNumber int, animated images / video (0 for
//                  single-frame / unknown)
//
// Missing entries (isBroken=YES) drive the E-4 open-time
// relocation sheet; the full list drives the media manager.
- (NSArray<NSDictionary<NSString*, id>*>*)mediaInventoryInSequence;

// Embed / extract for sequence-wide media management (G29 — C5).
// Embedding copies the file's binary content into the in-memory
// `MediaCacheEntry` so the next `saveSequence` writes base64 into
// the `.xsq`. Extracting writes the embedded data to disk at the
// entry's resolved path and flips the entry back to external.
// Videos and (on desktop) large binary files aren't embeddable —
// `IsEmbeddable()` on the base class gates both calls.
//
// Bumps the sequence dirty count so the toolbar Save affordance
// lights up. Returns YES if the operation actually changed the
// entry's embedded state.
- (BOOL)embedMediaAtPath:(NSString*)path;
- (BOOL)extractMediaAtPath:(NSString*)path;

// Embed / extract every embeddable cache entry. `typeFilter` nil
// or empty operates on all types; specifying "image" / "svg" /
// "shader" / "text" / "binary" scopes to that type (match the
// strings returned by `mediaInventoryInSequence`). Videos are
// never touched — they're un-embeddable. Returns the count that
// actually changed state.
- (int)embedAllMediaOfType:(NSString*)typeFilter;
- (int)extractAllMediaOfType:(NSString*)typeFilter;

// Replace a missing / broken media file with a fresh pick from
// disk (E-4 — relocate). Copies the source file into the show
// folder at a path derived from the stored path's type + the
// picked file's basename, then either re-reads the cache entry
// in place (when the target path matches the stored path) or
// performs a full rename-with-reference-update (when the target
// path differs — typical when the stored path was an absolute
// cross-machine path).
//
// `sourcePath` must be an on-disk absolute path the caller has
// already obtained security-scoped access to. Returns the
// target show-relative path on success, nil on failure.
- (nullable NSString*)replaceMissingMediaAtPath:(NSString*)storedPath
                                  fromSourcePath:(NSString*)sourcePath;

// Rename a cache entry (G30 — C5). Works for both embedded
// entries (cache-key swap only) and external files (also moves
// the file on disk so the stored path resolves at the new
// location). For embedded entries the disk move is skipped.
// Walks every effect's settings + palette maps and rewrites
// values equal to `oldPath` to point at `newPath` so no effect
// ends up stranded.
//
// Fails if `newPath` already exists on disk (external) or in
// the media cache (either), if the on-disk rename fails, or if
// the old path isn't cached. Returns YES on success.
- (BOOL)renameMediaFromPath:(NSString*)oldPath
                      toPath:(NSString*)newPath;

// Remove cached media entries that aren't referenced by any
// effect in the sequence (G31 — C5). The set of "referenced"
// paths is computed by walking every effect's settings +
// palette map, so we don't have to wait for a full re-render
// the way `MarkAllUnused` + `RemoveUnusedMedia` would. Returns
// the count removed. Dirties the sequence when anything was
// removed.
- (int)removeUnusedMedia;

// Video compatibility check (G32 — C5). Wraps
// `MediaCompatibility::CheckVideoFile`: returns nil when the
// file is AVFoundation-decodable on iPad, or a human-readable
// reason string (e.g. "Unsupported video codec") when it isn't.
//
// iPad can't transcode — if AVFoundation can't decode the
// source, neither can the transcoder. The UI uses the reason
// in a warning alert pointing the user at Handbrake / ffmpeg
// on desktop. Desktop keeps its in-app convert flow via
// `VideoTranscoder`; that path is not exposed to iPad.
- (nullable NSString*)videoCompatibilityIssueForPath:(NSString*)path;

// Walk the loaded sequence and surface authoring issues. Goes
// through `src-core/diagnostics/SequenceChecker` so the iPad
// catches every check the desktop `xLightsFrame::CheckSequence`
// covers, minus the wx-only network / OS / preferences chunks
// (those don't apply on iPad anyway).
//
// `progress` (optional) is invoked from the checker thread —
// callers that update SwiftUI must hop to MainActor. Percent in
// [0, 100]; `step` is the human-readable section name. Returns
// an empty array when no sequence is loaded.
//
// Safe to call from a background queue: SequenceChecker only
// reads in-memory objects and the resulting NSArray is fully
// detached from any C++ state by the time it returns.
- (NSArray<XLCheckSequenceIssue*>*)runSequenceCheckWithProgress:
    (nullable void (^)(int percent, NSString* _Nonnull step))progress;

// Ensure a preview-frame bundle exists for `path` at the requested
// thumbnail bounds. Loads the entry if not yet loaded and calls
// `GeneratePreview`; subsequent calls with matching bounds re-use the
// cached frames. Returns the frame count (>= 1 on success, 0 if the
// path can't be resolved or has no preview support).
//
// `mediaType` must match the value returned for this path in
// `mediaPathsInSequence` ("image" / "video" / "svg" / "shader" /
// "text" / "binary"). The bridge uses the type to look up the right
// cache; without it, the per-type `Get…` accessors (which
// create-on-access) would otherwise mint a fresh cache entry of the
// wrong type for the path and corrupt the media inventory.
- (int)ensureThumbnailPreviewForPath:(NSString*)path
                            mediaType:(NSString*)mediaType
                            maxWidth:(int)maxWidth
                           maxHeight:(int)maxHeight;

// PNG-encoded pixel data for one frame of a path's preview strip.
// Caller must have called `ensureThumbnailPreviewForPath` first so the
// frame exists. Returns nil if the path / index is invalid or PNG
// encoding fails. `mediaType` disambiguates the cache lookup (see
// above).
- (nullable NSData*)thumbnailPNGForPath:(NSString*)path
                              mediaType:(NSString*)mediaType
                             frameIndex:(int)frameIndex;

// Duration of the frame at `frameIndex` in milliseconds. Driven by the
// underlying format: animated-GIF / WebP frame delays, video frame
// intervals from the container, SVG / still-image entries return 0
// (single-frame content).
- (long)thumbnailFrameTimeMSForPath:(NSString*)path
                          mediaType:(NSString*)mediaType
                         frameIndex:(int)frameIndex;

// Probe a video file's total duration in milliseconds. Resolves the
// path via `FileUtils::FixFile` (so relative / cross-machine paths
// relocate onto the current show/media folders) and goes through
// core `VideoReader::GetVideoLength` — the same code path the render
// engine uses successfully. Swift's direct `AVURLAsset` probe fails
// on iCloud-hosted files because it doesn't carry the security-
// scoped bookmark the show folder was opened with; routing through
// the bridge keeps everything on the sandboxed-access path that
// already works for playback. Returns 0 on failure.
- (long)videoDurationMSForPath:(NSString*)path;

// Effect icons. Returns BGRA-premultiplied pixel data (width*height*4
// bytes) plus the chosen bucket size — parsed directly from the
// RenderableEffect's compiled-in XPM data. `desiredSize` is rounded
// up to the nearest {16,24,32,48,64} bucket. Returns nil if the effect
// name is unknown or the XPM couldn't be parsed.
- (nullable NSData*)iconBGRAForEffectNamed:(NSString*)effectName
                               desiredSize:(int)desiredSize
                                outputSize:(int*)outputSize;

// MARK: - Moving Head fixture plumbing (G3 — C7)
//
// The Moving Head effect stores its actual renderable parameters
// as packed command strings in `E_TEXTCTRL_MH1_Settings` …
// `E_TEXTCTRL_MH8_Settings` (semicolon-separated `Key: value`
// pairs, with '@' as an escaped ';' inside VC blobs). A fixture
// is "active" iff its settings string is non-empty — desktop
// derives checkbox state the same way at open time
// (`MovingHeadPanel.cpp:1974-1985`).

/// Mask of which fixture slots (1..8) are active for the selected
/// Moving Head effect. Each bit: `1 << (fixture - 1)`. 0 when no
/// effect is selected, not a Moving Head effect, or no fixtures
/// are active.
- (int)movingHeadActiveFixturesForRow:(int)rowIndex
                               atIndex:(int)effectIndex;

/// Toggle a fixture slot active / inactive. Writes a seed
/// command string with the current slider values (Pan / Tilt /
/// offsets / groupings / cycles) when activating; clears
/// `E_TEXTCTRL_MH<fixture>_Settings` when deactivating. Also
/// rewrites every active fixture's `Heads:` list so it reflects
/// the new selection. Returns YES on change.
- (BOOL)setMovingHeadFixture:(int)fixture
                        active:(BOOL)active
                        forRow:(int)rowIndex
                       atIndex:(int)effectIndex;

/// Rewrite every active fixture's Pan / Tilt / offsets / groupings
/// / cycles commands from the current slider values, preserving
/// colour / path / dimmer / shutter settings untouched. Called
/// automatically by the view model whenever a slider the renderer
/// actually reads changes. Returns the count of fixtures updated.
- (int)syncMovingHeadPositionForRow:(int)rowIndex
                              atIndex:(int)effectIndex;

/// G3+ — read / write a packed MH command (e.g. "Color",
/// "Dimmer", "Path") on the effect's active fixtures. The reader
/// returns the value from the first active fixture (assumed
/// uniform across fixtures); the writer replaces the command on
/// every active fixture, or removes it when `value` is empty.
/// Triggers a model re-render on success so the preview updates
/// immediately. No-op when no fixtures are active or the row
/// isn't a Moving Head effect.
- (NSString*)movingHeadCommand:(NSString*)cmdName
                          forRow:(int)rowIndex
                         atIndex:(int)effectIndex
    NS_SWIFT_NAME(movingHeadCommand(_:forRow:atIndex:));
- (BOOL)setMovingHeadCommand:(NSString*)cmdName
                         value:(NSString*)value
                        forRow:(int)rowIndex
                       atIndex:(int)effectIndex
    NS_SWIFT_NAME(setMovingHeadCommand(_:value:forRow:atIndex:));

// MARK: - DMX state + remap plumbing (G8 — C7)
//
// Model states live on the `Model` object's in-memory
// `stateInfo` map. Desktop's Save-State writes a new entry then
// fires `EVT_RGBEFFECTS_CHANGED` to persist `xlights_rgbeffects.xml`;
// iPad v1 keeps the save in-memory for the session and does NOT
// persist (documented in the banner on the DMX panel). Loading
// desktop-authored states works across restarts because the
// states were already read out of the XML at show-folder open.

/// True iff a state with `stateName` already exists on the
/// effect's target model. Used for the save-overwrite prompt.
- (BOOL)dmxStateExistsForRow:(int)rowIndex
                      atIndex:(int)effectIndex
                     stateName:(NSString*)stateName;

/// Copy the current effect's `E_SLIDER_DMX1..48` values into a
/// new (or existing) state on the model. Builds the attribute
/// map matching desktop's `DMXPanel::OnSaveAsStateClick`
/// (CustomColors=1, Type=SingleNode, s<n>-Color="#XXXXXX"). In-
/// memory only — not persisted to disk in v1.
///
/// `overwrite=NO` aborts when the state already exists.
/// Returns YES on successful save.
- (BOOL)dmxSaveStateForRow:(int)rowIndex
                    atIndex:(int)effectIndex
                   stateName:(NSString*)stateName
                   overwrite:(BOOL)overwrite;

/// Pull a saved state's channel values back into the effect's
/// `E_SLIDER_DMX1..N` settings. Matches desktop's
/// `DMXPanel::OnLoadFromStateClick`: validates `Type=SingleNode`
/// and `CustomColors=1`, reads `s<n>-Color` hex, extracts the
/// red channel as the DMX byte, writes it via the settings map
/// (so the UI sliders pick up the change through the normal
/// observable path). Returns YES on successful apply, NO if the
/// state isn't present or doesn't match the expected shape.
- (BOOL)dmxLoadStateForRow:(int)rowIndex
                    atIndex:(int)effectIndex
                   stateName:(NSString*)stateName;

/// Preset channel remappings for the DMX effect. Smaller scope
/// than desktop's `RemapDMXChannelsDialog` — iPad v1 exposes a
/// handful of common transforms via a menu instead of the full
/// 48-row grid editor (deferred to post-v1).
///
///   0 = Shift +1     (channel n value → channel n+1, wrap)
///   1 = Shift -1     (channel n value → channel n-1, wrap)
///   2 = Reverse      (1↔48, 2↔47, …)
///   3 = Invert All   (each channel value = 255 - old)
///   4 = Double       (each channel value × 2, clamp to 255)
///   5 = Half         (each channel value / 2)
///
/// Returns YES when anything changed.
- (BOOL)dmxRemapChannelsForRow:(int)rowIndex
                        atIndex:(int)effectIndex
                         preset:(int)preset;

// Effect bracket palette — sourced from the show folder's <colors>
// node in xlights_rgbeffects.xml so user-customised desktop palettes
// round-trip to iPad. Falls back to ColorManager defaults when the
// show has no customised palette. Returns each component as a
// 0..1 CGFloat through out-params (kept as scalars rather than
// UIColor so the per-frame draw loop in EffectsMetalGridView can
// feed them straight into appendLineX1's r/g/b args).
typedef NS_ENUM(NSInteger, XLEffectBracketState) {
    XLEffectBracketStateDefault = 0,
    XLEffectBracketStateSelected,
    XLEffectBracketStateLocked,
    XLEffectBracketStateDisabled,
};
- (void)bracketColorForState:(XLEffectBracketState)state
                         outR:(CGFloat*)outR
                         outG:(CGFloat*)outG
                         outB:(CGFloat*)outB
    NS_SWIFT_NAME(bracketColor(forState:outR:outG:outB:));

@end

NS_ASSUME_NONNULL_END
