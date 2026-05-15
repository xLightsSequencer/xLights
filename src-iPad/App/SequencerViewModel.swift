import SwiftUI
import UIKit

// XLSequenceDocument is the Objective-C bridge over the C++ render
// context. It's main-actor-owned for normal mutation but several
// flows deliberately hand it to background work (`renderAll` runs
// on a Thread{}, `openPackagedSequence` and `abortRenderAndWait`
// dispatch off main, the stem-separation pipeline does CoreML on a
// utility queue). The bridge serialises that access internally —
// this conformance is the type-system promise that mirrors the
// runtime contract so Swift 6 stops flagging the captures.
extension XLSequenceDocument: @retroactive @unchecked Sendable {}

@Observable @MainActor
class SequencerViewModel {
    var document = XLSequenceDocument()
    @ObservationIgnored let undoManager = UndoManager()
    var showFolderPath: String?
    var mediaFolderPaths: [String] = []
    var sequenceName: String?
    var isShowFolderLoaded = false
    var isSequenceLoaded = false
    /// True while `openPackagedSequence` is unzipping an `.xsqz`
    /// off the main queue. UI shells can show a progress overlay
    /// on this; extraction of a multi-MB package with embedded
    /// media takes long enough (seconds) to warrant a spinner.
    var isExtractingPackage = false
    /// For iOS packages opened from iCloud Drive / external Files
    /// providers: the original URL the user tapped. We copy the
    /// package into the app sandbox before extract (path-based ops
    /// can't read `~/Library/Mobile Documents` without iCloud
    /// entitlements) and copy the repacked package back to this URL
    /// on save so edits land in the original file. Nil for packages
    /// opened from inside the sandbox already.
    @ObservationIgnored var packageOriginalURL: URL?
    /// Sandbox directory holding the incoming-copy of an opened
    /// `.xsqz`. Wiped in closeSequence so repeated opens don't leak
    /// tmp dirs.
    @ObservationIgnored var packageSandboxDir: URL?
    /// Dirty marker mirrored from `XLSequenceDocument.isSequenceDirty`.
    /// Updated by the dirty-poll timer (500ms) while a sequence is
    /// loaded — toolbar Save / close-with-prompt key off it. Poll
    /// rather than observer so every mutation path (bridge + core)
    /// updates it without having to hook each entry individually.
    var isDirty: Bool = false
    /// Count of referenced media files that couldn't be resolved on
    /// the current device (file not present, no iCloud copy, etc.).
    /// Populated on sequence open via the media-inventory bridge.
    /// Drives the missing-media banner + Media Manager badge.
    var brokenMediaCount: Int = 0
    var sequenceDurationMS: Int = 0
    var frameIntervalMS: Int = 50
    var rows: [RowInfo] = []
    var sequenceFiles: [SequenceEntry] = []

    // Audio state
    var hasAudio = false
    var isPlaying = false
    var isPaused = false
    var playPositionMS: Int = 0
    var volume: Int = 100
    /// B43: -1 = main sequence audio, 0..N-1 = alt track index. Only
    /// affects the waveform display — playback always uses the main
    /// track. Mirrored from the bridge so SwiftUI can refresh on
    /// sequence load. `altAudioTrackNames` is sized to the alt-track
    /// count and surfaces in the waveform long-press menu.
    var activeWaveformTrack: Int = -1
    var altAudioTrackNames: [String] = []

    /// B40: timestamp (CFAbsoluteTime) of the last audio scrub burst
    /// fired from `scrubSeekTo`. Used to throttle bursts to ~50ms.
    @ObservationIgnored var lastScrubAudioBurstAt: TimeInterval = 0

    /// Phase A re-prompt UX. When `obtainAccess(toPath:)` fails for a
    /// persisted bookmark (show folder, media folder, sequence file),
    /// the affected operation enqueues an `AccessRepromptRequest` here
    /// and ContentView surfaces a `UIDocumentPickerViewController` so
    /// the user can re-grant access. `accessRepromptQueue` lets a
    /// single `loadShowFolder` call surface stale show-folder + media
    /// folders one at a time; `afterAccessQueueEmpty` runs the
    /// deferred operation (the real `document.loadShowFolder` /
    /// `document.openSequence`) once the queue drains.
    var accessReprompt: AccessRepromptRequest?
    @ObservationIgnored private var accessRepromptQueue: [AccessRepromptRequest] = []
    @ObservationIgnored private var afterAccessQueueEmpty: (() -> Void)?

    /// B97: Find / Replace state. Search is over timing-mark labels
    /// only (matches desktop's intentional restriction in
    /// `EffectsGrid::Find`). `findResults` caches `(rowIndex,
    /// markIndex)` for every visible timing-row mark whose name
    /// contains `findText`; navigation cycles through these.
    var findText: String = ""
    var findCaseSensitive: Bool = false
    struct FindMatch: Equatable, Hashable {
        let rowIndex: Int
        let markIndex: Int
    }
    var findResults: [FindMatch] = []
    var currentFindIndex: Int = -1
    /// Toggled by ⌘F (XLightsCommands) and by `Done` in the sheet.
    var findReplacePresented: Bool = false
    // F-4 playback speed. Desktop exposes 0.25/0.5/0.75/1.0/1.5/2/3/4x
    // — we match that set in the Playback menu. Applied to the
    // AVAudioEngine time-pitch unit via the bridge on `play()`, and
    // also used to scale the wall-clock elapsed for no-audio
    // sequences in `startPlaybackTimer`.
    var playSpeed: Float = 1.0

    // B32 loop region: when `hasLoopRegion`, playback (B33 Play
    // Loop mode) wraps from `loopEndMS` back to `loopStartMS`, and
    // B44 Render Selected Region operates on this range.
    // `loopStartMS == loopEndMS` means cleared; UI also treats 0..0
    // as "no loop."
    var loopStartMS: Int = 0
    var loopEndMS: Int = 0
    /// B33 Play Loop mode — when on + hasLoopRegion, the playback
    /// timer wraps at `loopEndMS` back to `loopStartMS`.
    var loopPlayEnabled: Bool = false
    var hasLoopRegion: Bool { loopEndMS > loopStartMS }

    /// B34 — 10 numbered tag positions. -1 = unset. Mirrored from
    /// the bridge's `SequenceElements::_tagPositions` so SwiftUI can
    /// observe + render tags on the ruler. `reloadTagPositions()`
    /// pulls from the bridge on sequence load and after any mutating
    /// op; tag writes go through `setTag(_:atMS:)` which updates
    /// both sides.
    var tagPositions: [Int] = Array(repeating: -1, count: 10)
    // True while a selection-scoped preview loop is advancing
    // `playPositionMS`. Observed by the preview panes so they keep their
    // MTKView display link running during scrub (not just real playback).
    var isScrubbing = false

    // Waveform cache
    var waveformPeaks: [Float] = []
    var waveformStartMS: Int = 0
    var waveformEndMS: Int = 0

    // B41 waveform filter type. Matches the bridge's filterType
    // parameter (0=RAW, 1=BASS, 2=TREBLE, 3=ALTO, 4=NONVOCALS,
    // 5=CUSTOM). A9.1 adds `.custom`; the low/high MIDI notes live in
    // `customBandLowNote` / `customBandHighNote` on the view model so
    // the enum stays a plain `Int` for the confirmationDialog ForEach.
    // Observed so the ruler right-click menu's radio state stays in
    // sync and a zoom-triggered reload keeps the filter.
    enum WaveformFilter: Int, CaseIterable {
        case raw = 0, bass = 1, treble = 2, alto = 3, nonVocals = 4, custom = 5, lufs = 6, vocals = 7
        case stemDrums = 8, stemBass = 9, stemOther = 10, stemVocals = 11
        var displayName: String {
            switch self {
            case .raw:        return "Full Range"
            case .bass:       return "Bass"
            case .treble:     return "Treble"
            case .alto:       return "Alto"
            case .nonVocals:  return "Non-Vocals"
            case .custom:     return "Custom Band…"
            case .lufs:       return "Perceptual (LUFS)"
            case .vocals:     return "Vocals (center-extract)"
            case .stemDrums:  return "Stem — Drums"
            case .stemBass:   return "Stem — Bass"
            case .stemOther:  return "Stem — Other"
            case .stemVocals: return "Stem — Vocals (ML)"
            }
        }
        /// True when this filter depends on the HTDemucs stem cache
        /// being populated.
        var requiresStems: Bool {
            switch self {
            case .stemDrums, .stemBass, .stemOther, .stemVocals: return true
            default: return false
            }
        }
    }
    var waveformFilter: WaveformFilter = .raw {
        didSet { if oldValue != waveformFilter { reloadWaveformCurrent() } }
    }
    /// A9.1: MIDI note bounds for `WaveformFilter.custom`. Defaults
    /// roughly cover "vocal" (C3–C5). Changing either while `custom`
    /// is active re-samples the waveform through the new band.
    var customBandLowNote: Int = 48 {
        didSet {
            if customBandLowNote != oldValue, waveformFilter == .custom {
                reloadWaveformCurrent()
            }
        }
    }
    var customBandHighNote: Int = 72 {
        didSet {
            if customBandHighNote != oldValue, waveformFilter == .custom {
                reloadWaveformCurrent()
            }
        }
    }
    // Peak count used to build the current `waveformPeaks`. Compared against
    // the target-for-current-zoom to decide whether to re-sample.
    @ObservationIgnored private var waveformSampleCount: Int = 0
    // Debounce task for zoom-driven waveform re-sampling so rapid pinches
    // don't swamp the main actor with redundant bridge calls.
    @ObservationIgnored private var waveformReloadTask: Task<Void, Never>?

    // A2 onset detection. `onsetTimesMS` is populated by
    // `computeOnsets()` (spectral-flux detector in the bridge);
    // `showOnsets` flips the overlay on the waveform strip. Cached
    // per-sequence — we invalidate when a new sequence loads.
    var onsetTimesMS: [Int] = []
    var showOnsets: Bool = false
    @ObservationIgnored var isComputingOnsets: Bool = false
    @ObservationIgnored private var onsetsComputed: Bool = false

    // A5 pitch contour overlay. `pitchContour` is per-frame
    // (timeMS, frequency Hz, confidence) triples from the bridge's
    // detector. `showPitchContour` toggles the overlay on the Metal
    // waveform. Unvoiced frames keep frequency=0 — the Metal view
    // breaks the polyline across them so silence doesn't produce a
    // spurious slope.
    var pitchContour: [Float] = []   // flat [t,f,c, t,f,c, ...]
    var showPitchContour: Bool = false
    @ObservationIgnored var isComputingPitch: Bool = false
    @ObservationIgnored private var pitchComputed: Bool = false

    // A6 spectrogram view mode. When `showSpectrogram` is true, the
    // waveform strip renders the STFT magnitude spectrum (log-
    // frequency y-axis, dB-scaled colormap) instead of the peak
    // polygons. Compute is lazy + cached in the bridge.
    var showSpectrogram: Bool = false
    @ObservationIgnored var spectrogramReady: Bool = false

    // A8 HTDemucs stem separation state.
    enum StemPhase { case idle, pickingRoot, downloading, separating, ready }
    var stemsPhase: StemPhase = .idle
    var stemsProgressPct: Int = 0
    var stemsAvailable: Bool = false
    /// Pending filter that triggered the install flow — switched to
    /// once `stemsAvailable` becomes true.
    @ObservationIgnored var stemsPendingFilter: WaveformFilter? = nil
    /// Root the user picked in the installer sheet. Non-nil while
    /// the install is running so cancellation can clean up.
    @ObservationIgnored var stemsInstallRoot: String? = nil

    // A7 SoundAnalysis classification. `soundClasses` maps class
    // identifier ("music.drums" etc.) to a per-`soundClassTimeStep`
    // confidence array. `selectedSoundClass` selects one for gating
    // the waveform — each bucket's peaks are multiplied by the
    // class's interpolated confidence at that time, so picking
    // "Drums" produces a waveform that's tall where drums dominate
    // and near-zero elsewhere.
    var soundClasses: [String: [Float]] = [:]
    var selectedSoundClass: String? = nil {
        didSet {
            if oldValue != selectedSoundClass { reloadWaveformCurrent() }
        }
    }
    var isClassifyingSound: Bool = false
    @ObservationIgnored var soundClassTimeStep: Float = 1.0

    // Controller output
    var isOutputting = false

    // Rendering (background)
    var isRendering = false
    var isRenderDone = false
    /// Bumped whenever a render kickoff has completed. Observed by
    /// the effect grid so `DrawEffectBackground` picks up newly
    /// populated `xlDisplayList`s — setting changes (e.g. a
    /// SingleStrand palette swap) redraw the background the moment
    /// the render finishes instead of the user having to scroll or
    /// zoom to force a redraw.
    var renderedBackgroundsRevision: Int = 0
    /// Coalesces multiple render kickoffs into one poll. Set while
    /// `trackRenderCompletion()` has an asyncAfter chain in flight.
    @ObservationIgnored private var renderPollInFlight: Bool = false

    // Preview
    var showPreview = false
    /// Name of the model currently shown in the Model Preview pane, or nil
    /// when no model is selected. Set via `selectPreviewModel(rowIndex:)`
    /// when a row is tapped in the effects grid.
    var previewModelName: String? = nil

    // Selection & editing
    var selectedEffect: EffectSelection?
    /// Full set of selected effects. Always mirrors `selectedEffect`:
    /// single-select sets it to `[primary]`, multi-select via marquee
    /// fills it with N, clear empties it. When multi-selected, the
    /// inspector and scrub are suppressed (selectedEffect == nil) but
    /// the grid highlights every member. Bulk ops (delete, lock,
    /// disable) iterate this set; single-effect ops fall back to
    /// selectedEffect.
    var selectedEffects: Set<EffectSelection> = []
    var selectedEffectSettings: [String: String] = [:]     // legacy: raw key/value map
    var availableEffects: [String] = []
    var selectedPaletteEffect: String?
    var showInspector = false

    // F-1 scene-level routing. Set to true when the corresponding
    // preview is showing in its own Window scene. The embedded
    // version in `SequencerView` swaps for a "docked elsewhere"
    // placeholder so the user can't accidentally drive two copies.
    // Flipped by the detached scene's `.onAppear` / `.onDisappear`.
    var housePreviewDetached: Bool = false
    var modelPreviewDetached: Bool = false

    // J-0 layout editor — independent selection state so the editor's
    // model-list highlight doesn't clobber `previewModelName` (which
    // drives the Model Preview's single-model render mode and the
    // effect grid's row-tap state). Empty / nil = nothing selected.
    // `layoutEditorSelectedModel` is the "primary" / most-recently-
    // selected model — used by single-model UI (inline action bar,
    // property panel, drag origin). The full selection set lives in
    // `layoutEditorSelection`; the primary is conventionally
    // `selection.first` of insertion order (the last model added).
    var layoutEditorSelectedModel: String? = nil
    /// Phase J-4 (multi-select) — full selection set. Empty when
    /// nothing's selected. Contains exactly `layoutEditorSelectedModel`
    /// for single-select; multiple entries when the user has
    /// activated edit mode and tapped / marqueed multiple models.
    /// Align / distribute / multi-delete operate on this set.
    var layoutEditorSelection: Set<String> = []
    /// Phase J-4 (multi-select) — when true, taps on the canvas
    /// toggle a model's membership in the selection set instead of
    /// replacing the selection. Driven by a "Select" toggle in the
    /// Layout Editor toolbar (Photos-style). Cleared on editor
    /// close and on "Done" / explicit exit.
    var layoutEditMode: Bool = false

    /// Phase J-6 (sidebar canvas sync) — Group / ViewObject picks
    /// in the sidebar. Independent of `layoutEditorSelectedModel`
    /// so flipping tabs doesn't lose the model pick. The canvas
    /// reads these via PreviewPaneView and tints / draws handles
    /// accordingly.
    var layoutEditorSelectedGroup: String? = nil
    var layoutEditorSelectedObject: String? = nil

    /// J-20.6 — active sidebar tab in the Layout Editor, mirrored
    /// from `LayoutEditorView`'s `@State sidebarTab`. Lives on the
    /// view model so the canvas / preview pane can gate picking
    /// behaviour (e.g. don't fall through to view-object picks
    /// when the user is on the Models tab — desktop only lets
    /// you select objects from the Layout panel's object list).
    /// String-typed to avoid pulling `LayoutSidebarTab` into the
    /// view model module.
    var layoutEditorActiveTab: String = "models"

    /// J-13 — name of the Terrain view object currently in
    /// heightmap edit mode. When non-nil, canvas taps modify
    /// the terrain's PointData rather than selecting models.
    var terrainEditTarget: String? = nil
    /// J-13 — magnitude of the per-tap height change in world
    /// units. Sign comes from `terrainEditRaise`.
    var terrainEditDelta: Float = 1.0
    /// J-13 — brush radius in screen points. 0 = single-point;
    /// >0 applies a cosine falloff to neighbour grid points.
    var terrainEditBrushPoints: CGFloat = 24.0
    /// J-13 — true → tap-to-raise; false → tap-to-lower.
    var terrainEditRaise: Bool = true

    /// Set both primary + selection set to a single model (or
    /// clear them both for nil/empty). All single-select call
    /// sites should use this to keep the two fields in sync —
    /// the primary IS the only entry of the selection set in
    /// single-select mode.
    func layoutSelectSingle(_ name: String?) {
        if let n = name, !n.isEmpty {
            layoutEditorSelectedModel = n
            layoutEditorSelection = [n]
        } else {
            layoutEditorSelectedModel = nil
            layoutEditorSelection.removeAll()
        }
    }
    /// Phase J-3 (touch UX) — when non-nil, the Layout Editor is
    /// in "creation mode": the next tap on the canvas creates a
    /// new model of this type at the touch point. Cleared on
    /// creation, on Cancel, or when the editor closes.
    var layoutPendingNewModelType: String? = nil
    /// Phase J-3 (touch UX) — when non-nil, a polyline-style model
    /// (Poly Line / MultiPoint) is mid-creation and each follow-on
    /// tap appends a vertex. Cleared by an explicit Done, Esc /
    /// Return key, or when the editor closes. Set after the first
    /// vertex of a polyline is placed; mutually exclusive with
    /// `layoutPendingNewModelType` (which only governs the first
    /// vertex / fresh-model placement).
    var layoutPolylineInProgress: String? = nil
    /// Phase J-4 (import) — file path of a pending .xmodel import.
    /// When non-nil the Layout Editor is in placement mode; the
    /// next tap on the canvas calls `bridge.importXmodelFromPath:`
    /// with the touch as the placement target. Cleared after a
    /// successful import or Cancel.
    var layoutPendingImportPath: String? = nil
    /// True when the standalone Layout Editor scene is open. The
    /// Tools menu entry disables itself on a second press so we
    /// don't fight `WindowGroup`'s "focus existing instance"
    /// behaviour with a redundant token push.
    var layoutEditorOpen: Bool = false
    // F-1c: set of inspector tabs currently open in their own
    // `inspector-tab` scene windows. The sidebar swaps to a
    // placeholder for any tab in this set.
    var detachedInspectorTabs: Set<String> = []

    // F-1 restoration guard. iPadOS restores any WindowGroup that
    // was alive at shutdown, so closing everything and relaunching
    // can re-spawn a detached preview without the main sequencer.
    // An explicit user detach inserts the scene's id / tab rawValue
    // here, and the detached scene's `onAppear` removes it. If the
    // token is absent when `onAppear` fires, the scene was system-
    // restored, not user-requested — the root dismisses itself.
    //
    // `@ObservationIgnored` because this is ephemeral control flow,
    // not observable state — we don't want views re-rendering on
    // token churn.
    @ObservationIgnored
    var pendingDetachTokens: Set<String> = []

    // F-4 menu-bar routing. The WindowGroup's `.commands { }` block
    // lives at app level, but several actions (Save As file exporter
    // + alerts, Sequence Settings sheet, Display Elements sheet) are
    // owned by `SequencerView`. Flipping a flag here lets the
    // SwiftUI view observe and present the appropriate UI without
    // hard-coupling the command handler to view internals.
    var showingSequenceSettings = false
    var showingDisplayElements = false
    // Phase I-2 — Tools → Import Effects sheet. Reset to false after
    // the user dismisses the sheet (Apply or Cancel).
    var showingImportEffects = false
    // Help → About xLights. Sheet is presented at ContentView level
    // so it works regardless of show-folder / sequence load state.
    var showingAbout = false
    // Tools → Check Sequence. Sheet is presented at ContentView
    // level (gated on `isSequenceLoaded` because it walks the
    // currently-loaded SequenceElements via the bridge).
    var showingCheckSequence = false
    // Tools → AI Services. Sheet at ContentView level so the user
    // can configure API keys / models even before opening a sequence.
    var showingAIServices = false
    // Unified Add Timing Track sheet — replaces the per-call-site
    // confirmationDialogs that used to live in DisplayElementsSheet
    // and the view-selection menu. Any call site that wants to add
    // a timing track flips this; the sheet (presented at
    // ContentView level) handles all the option branches: Empty,
    // fixed interval, metronome, FPP, audio analysis (Onsets /
    // Tempo / Chords), and AI Lyrics.
    var showingAddTimingTrack = false

    /// Run Check Sequence on a background queue and report progress
    /// while it walks. The bridge's `SequenceChecker` calls back from
    /// the worker thread; the callback hops to MainActor so SwiftUI
    /// state updates happen on the right thread.
    ///
    /// Returns an empty array when no sequence is loaded.
    func runSequenceCheckAsync(
        progress: @escaping @MainActor (Int, String) -> Void
    ) async -> [XLCheckSequenceIssue] {
        let doc = document
        return await Task.detached(priority: .userInitiated) {
            return doc.runSequenceCheck { percent, step in
                Task { @MainActor in progress(Int(percent), step) }
            }
        }.value
    }

    /// Move the playhead to `startTimeMS`, locate the row whose model
    /// name matches `modelName` (switching to Master View when not
    /// already showing it — Master is the only view guaranteed to
    /// contain every model), and select the effect at that time on
    /// that row. The grid's `onChange(of: selectedEffect)` then
    /// scrolls the row + timeline into view automatically.
    ///
    /// Returns `true` when an effect was selected. Returns `false`
    /// (caller should fall back to a plain `seekTo`) when:
    /// - `modelName` is empty or no matching row exists in any view;
    /// - no effect at that time matches `effectName` on the row.
    @discardableResult
    func jumpToEffect(modelName: String?,
                       effectName: String?,
                       startTimeMS: Int,
                       layerIndex: Int) -> Bool {
        let cleanModel = (modelName ?? "").trimmingCharacters(in: .whitespaces)
        guard !cleanModel.isEmpty else {
            seekTo(ms: max(0, startTimeMS))
            return false
        }

        // Pass 1: try the current view. Submodel / strand rows return
        // their parent model name from `rowModelName`, so a top-level
        // model match catches both. Prefer top-level (lowest nestDepth)
        // to avoid landing on a strand row when the issue is anchored
        // to the model itself.
        if let hit = findEffectRow(matchingModel: cleanModel,
                                    effectName: effectName,
                                    startTimeMS: startTimeMS,
                                    layerIndex: layerIndex) {
            applyEffectSelection(hit)
            return true
        }

        // Pass 2: jump to Master View (index 0) and retry. Master
        // contains every model in the layout; if it's still not
        // there, the sequence references a model that doesn't exist
        // in the current show — surface to the caller.
        if document.currentViewIndex() != 0 {
            document.setCurrentViewIndex(0)
            reloadRows()
            if let hit = findEffectRow(matchingModel: cleanModel,
                                        effectName: effectName,
                                        startTimeMS: startTimeMS,
                                        layerIndex: layerIndex) {
                applyEffectSelection(hit)
                return true
            }
        }

        // Couldn't find the row / effect — at least move the playhead.
        seekTo(ms: max(0, startTimeMS))
        return false
    }

    private struct EffectLocation {
        let rowIndex: Int
        let effectIndex: Int
    }

    private func findEffectRow(matchingModel modelName: String,
                                 effectName: String?,
                                 startTimeMS: Int,
                                 layerIndex: Int) -> EffectLocation? {
        // Prefer the top-level model row (smallest nestDepth) so the
        // selection lands on the canonical row rather than a strand /
        // node child. Within the matching rows, prefer one whose
        // `layerIndex` matches the issue's: when an effect type
        // appears on multiple layers of the same model only one of
        // them is anchored to a real check-sequence issue, and the
        // layer is the only signal that disambiguates them.
        var candidates: [Int] = []
        for row in rows {
            guard row.timing == nil else { continue }
            let rowName = (document.rowModelName(at: Int32(row.id)) as String?) ?? ""
            if rowName == modelName {
                candidates.append(row.id)
            }
        }
        if candidates.isEmpty { return nil }
        candidates.sort { (a, b) in
            let rowA = rows.first(where: { $0.id == a })
            let rowB = rows.first(where: { $0.id == b })
            // 1) shallower nest first (top-level model row before
            //    strands / nodes).
            let nestA = rowA?.nestDepth ?? Int.max
            let nestB = rowB?.nestDepth ?? Int.max
            if nestA != nestB { return nestA < nestB }
            // 2) exact-layer match wins. -1 means the issue isn't
            //    anchored to a layer (rare; treat all layers equal).
            if layerIndex >= 0 {
                let matchA = (rowA?.layerIndex == layerIndex) ? 0 : 1
                let matchB = (rowB?.layerIndex == layerIndex) ? 0 : 1
                if matchA != matchB { return matchA < matchB }
            }
            // 3) deterministic fall-through: earlier-numbered row wins.
            return a < b
        }
        for rowId in candidates {
            guard let row = rows.first(where: { $0.id == rowId }) else { continue }
            // If the issue is anchored to a specific layer, only
            // accept rows on that layer. We leave the candidate
            // sort above in place (instead of pre-filtering) so the
            // diagnostic at the end of the function — fall-back to
            // any matching row — still has something to work with
            // when the layerIndex didn't survive the round trip.
            if layerIndex >= 0 && row.layerIndex != layerIndex { continue }
            // Try to match the exact (effectName, startTimeMS) pair
            // first; fall back to "effect at startTimeMS" if the name
            // is empty or doesn't line up.
            if let name = effectName, !name.isEmpty {
                if let idx = row.effects.firstIndex(where: {
                    $0.startTimeMS == startTimeMS && $0.name == name
                }) {
                    return EffectLocation(rowIndex: rowId, effectIndex: idx)
                }
            }
            if let idx = row.effects.firstIndex(where: { $0.startTimeMS == startTimeMS }) {
                return EffectLocation(rowIndex: rowId, effectIndex: idx)
            }
        }

        // Layer-strict pass found nothing: try again ignoring the
        // layer. Better to land on a same-time / same-name effect on
        // a sibling layer than to silently fail.
        if layerIndex >= 0 {
            for rowId in candidates {
                guard let row = rows.first(where: { $0.id == rowId }) else { continue }
                if let name = effectName, !name.isEmpty {
                    if let idx = row.effects.firstIndex(where: {
                        $0.startTimeMS == startTimeMS && $0.name == name
                    }) {
                        return EffectLocation(rowIndex: rowId, effectIndex: idx)
                    }
                }
                if let idx = row.effects.firstIndex(where: { $0.startTimeMS == startTimeMS }) {
                    return EffectLocation(rowIndex: rowId, effectIndex: idx)
                }
            }
        }
        return nil
    }

    private func applyEffectSelection(_ loc: EffectLocation) {
        selectEffect(rowIndex: loc.rowIndex, effectIndex: loc.effectIndex)
    }

    // Build the Package Logs zip on a background queue. Returned URL
    // points into NSTemporaryDirectory and the caller (share sheet)
    // owns its lifecycle — iOS cleans up tmp anyway, but explicit
    // deletion after the share completes is preferable.
    func packageLogs() async -> URL? {
        let doc = document
        return await Task.detached(priority: .utility) { () -> URL? in
            do {
                return try XLLogPackager.packageLogs(for: doc)
            } catch {
                return nil
            }
        }.value
    }
    // Save As is a multi-step flow (persist, open exporter, handle
    // errors). The menu command bumps this counter; the SequencerView
    // `.onChange` reacts exactly once per command invocation even if
    // the user hits ⇧⌘S twice back-to-back.
    var saveAsRequestToken: Int = 0
    // Same pattern for Close — surfaces the existing dirty-prompt
    // flow from the view without duplicating the logic.
    var closeRequestToken: Int = 0

    // Active drag snapshot, set by the grid's gesture handlers and
    // consumed by any view that needs to render live drag feedback
    // (cross-row ghost, drag-pill label, live position of the
    // dragged effect). Moving this out of the UIKit canvas lets
    // the Metal grid draw identical feedback during migration.
    var activeDrag: ActiveDrag?

    /// Rendering-relevant subset of what happens during a drag. All
    /// the gesture-internal machinery (scroll suppression, auto-
    /// scroll display link, drag high-water marks for invalidation,
    /// pan translation origin) stays inside the gesture-handling
    /// UIView; only the fields both paths want to paint live show
    /// up here.
    struct ActiveDrag: Equatable {
        enum Kind: Equatable {
            case move, resizeLeft, resizeRight, fadeIn, fadeOut
        }
        let kind: Kind
        let srcRowId: Int
        let effectIndex: Int
        let origStartMS: Int
        let origEndMS: Int
        var liveStartMS: Int
        var liveEndMS: Int
        var liveFadeInSec: Float
        var liveFadeOutSec: Float
        /// `nil` when the drag is hovering over its source row. A
        /// non-nil row id flags a cross-row move; the ghost should
        /// render at that row instead of the source.
        var liveRowId: Int?
        /// True when a cross-row drop would collide with an effect
        /// in the target row — the ghost shows a red tint and
        /// `.ended` cancels the move rather than committing it.
        var liveDropInvalid: Bool
    }

    /// Bumps every time an inspector edit writes a setting. Observed
    /// by the Metal grid so fade-bar widths, bracket colours, etc.
    /// redraw when the user changes a setting without moving /
    /// resizing the effect.
    var inspectorRevision: Int = 0

    // Metadata for the currently selected effect and shared panels.
    // Keys in `blendingMetadata` serialize with the T_ prefix (the panel was
    // historically called "Timing" — desktop renamed it "Blending" without
    // changing the on-disk prefix).
    var selectedEffectMetadata: EffectMetadata?
    var bufferMetadata: EffectMetadata?
    var colorMetadata: EffectMetadata?
    var blendingMetadata: EffectMetadata?

    // Parsed metadata cache per effect name — avoids re-parsing JSON on every selection.
    private var metadataCache: [String: EffectMetadata] = [:]

    // Memory pressure
    /// True when available memory is below the low-memory threshold.
    /// Drives the low-memory banner in the UI.
    var memoryWarning = false

    /// Non-nil when the most recent save skipped the fseq companion
    /// because the in-flight render had been aborted (typically by
    /// memory pressure — `_renderEngine->SignalAbort()` runs from
    /// `HandleMemoryWarning`). Drives a tap-to-dismiss banner the
    /// user can clear once they re-render and re-save.
    var fseqWriteSkippedMessage: String?
    private static let memoryWarningThresholdMB: Int64 = 256
    private static let memoryRecoveredThresholdMB: Int64 = 384  // hysteresis
    private var memoryPressureSource: DispatchSourceMemoryPressure?
    private var memoryPollTimer: Timer?
    private var memoryWarningObserver: NSObjectProtocol?

    struct EffectSelection: Hashable {
        let rowIndex: Int
        let effectIndex: Int
        let name: String
        let startTimeMS: Int
        let endTimeMS: Int
    }

    private var playbackTimer: Timer?
    private var scrubTimer: Timer?
    private var scrubStartMS: Int = 0
    private var scrubEndMS: Int = 0
    private var renderPollTimer: Timer?
    private var dirtyPollTimer: Timer?
    private var autosaveTimer: Timer?
    /// Change-count snapshot at the last autosave write. Skips
    /// repeat writes when nothing changed between ticks.
    private var lastAutosaveChangeCount: UInt64 = 0
    /// Autosave interval in minutes. 0 disables autosave.
    /// Persisted via `@AppStorage` on the view that owns the
    /// settings UI; the view model reads it on open.
    var autosaveIntervalMinutes: Int = 5
    private var playbackStartTime: CFAbsoluteTime = 0  // wall clock when play started
    private var playbackStartMS: Int = 0                // sequence position when play started

    struct RowInfo: Identifiable, Equatable {
        let id: Int
        let displayName: String
        let layerIndex: Int
        let isCollapsed: Bool
        let effects: [EffectInfo]
        /// Non-nil iff this row is a timing-element row. Layers inside a
        /// single timing element share the same element name + color index;
        /// only the first layer (layerIndex == 0) renders the active dot
        /// and the element name. Higher layers show "[N] layerName".
        let timing: TimingRowInfo?
        /// Mirror of `Row_Information_Struct.submodel` — true for rows
        /// that live underneath a parent model via ShowStrands().
        let isSubmodel: Bool
        /// Visual indentation depth (0 = top-level model, 1 = direct
        /// submodel / strand, 2 = node row). Matches desktop's
        /// `nestDepth`.
        let nestDepth: Int
        /// -1 for non-strand rows; else the strand index within the
        /// parent model.
        let strandIndex: Int
        /// -1 for non-node rows; else the node index within the parent
        /// strand.
        let nodeIndex: Int
    }

    struct TimingRowInfo: Equatable {
        /// Desktop ColorManager maps colorIndex%5 → cyan/red/green/blue/yellow.
        let colorIndex: Int
        let elementName: String
        /// e.g. "Phrases", "Words", "Phonemes" for lyric layers.
        let layerName: String
        /// Mirror of `TimingElement::GetActive()`. Stored in the
        /// RowInfo diff so a toggle on the header row flips the
        /// equality and propagates through SwiftUI body invalidation —
        /// otherwise the grid's `collectActiveTimingMarkTimes()` would
        /// stay pinned to its stale result until the next scroll/zoom
        /// happened to re-evaluate the body for other reasons.
        let isActive: Bool
    }

    struct EffectInfo: Identifiable, Equatable {
        let id: Int
        let name: String
        let startTimeMS: Int
        let endTimeMS: Int
    }

    // MARK: - Show Folder

    func loadShowFolder(url: URL) {
        loadShowFolder(path: url.path, mediaFolders: mediaFolderPaths)
    }

    /// Load a show folder with optional media folders. Both show folder and
    /// each media folder have their security-scoped bookmarks refreshed via
    /// iPadRenderContext::LoadShowFolder (which calls ObtainAccessToURL for
    /// each) before reading files.
    ///
    /// Phase A re-prompt UX: a stale persisted bookmark (iCloud eviction,
    /// iOS aging out the security-scoped bookmark) silently fails inside
    /// `ObtainAccessToURL`, leaving every subsequent file open broken.
    /// Pre-check here from Swift; any path whose access we can't refresh
    /// is queued for an `AccessRepromptRequest` so the user gets a
    /// `UIDocumentPickerViewController` to re-grant. The C++ load is
    /// deferred until the queue drains; media folders we still can't
    /// access after the re-grant pass are dropped (matches the previous
    /// silent-drop behavior in `iPadRenderContext::LoadShowFolder`).
    ///
    /// The actual work — obtainAccess pre-flight, the C++
    /// iPadRenderContext::LoadShowFolder call, and the recursive .xsq scan
    /// — runs on a detached task. Each step does synchronous file I/O that
    /// blocks on iCloud download for evicted folders; on the main actor
    /// (this method is called from the Done button in FolderConfigView and
    /// from the launch-time .task in XLightsApp), the cumulative time was
    /// exceeding the 5-second 0x8BADF00D watchdog deadline and getting
    /// the app killed. The detach keeps the main actor free while the
    /// load runs, then we hop back to apply state. The method returns
    /// immediately; observers should watch `isShowFolderLoaded`.
    func loadShowFolder(path: String, mediaFolders: [String]) {
        showFolderPath = path
        mediaFolderPaths = mediaFolders

        Task.detached { [document, weak self] in
            let showOK = XLSequenceDocument.obtainAccess(toPath: path, enforceWritable: false)
            var stale: [(path: String, label: String)] = []
            if !showOK {
                stale.append((path, "show folder"))
            }
            for folder in mediaFolders where !XLSequenceDocument.obtainAccess(toPath: folder, enforceWritable: false) {
                stale.append((folder, "media folder"))
            }

            if stale.isEmpty {
                let loaded = document.loadShowFolder(path, mediaFolders: mediaFolders)
                let scanned: [SequenceEntry] = loaded ? SequenceScanner.scan(showFolder: path) : []
                await self?.applyLoadResult(loaded: loaded, sequenceFiles: scanned, path: path)
                return
            }

            await self?.queueStaleRepromptsThenReload(stale: stale, path: path, mediaFolders: mediaFolders)
        }
    }

    /// MainActor-side completion handler for a successful (or failed)
    /// detached show-folder load. Lives separately from `loadShowFolder`
    /// so the Task closure only needs to send Sendable values across the
    /// actor boundary.
    private func applyLoadResult(loaded: Bool, sequenceFiles: [SequenceEntry], path: String) {
        self.isShowFolderLoaded = loaded
        self.sequenceFiles = sequenceFiles
        if loaded {
            // L-1b: record successful loads so the folder picker can
            // surface them as one-tap MRU entries on the next visit.
            RecentShowFolders.record(path: path)
        }
    }

    /// MainActor-side handler for the stale-bookmark branch: surfaces the
    /// re-prompt sheets, then on completion re-enters `loadShowFolder` so
    /// the second pass picks up the freshly-granted bookmarks (and detaches
    /// again for its own heavy work).
    private func queueStaleRepromptsThenReload(
        stale: [(path: String, label: String)],
        path: String,
        mediaFolders: [String]
    ) {
        for entry in stale {
            enqueueAccessReprompt(label: entry.label, originalPath: entry.path, pickPath: entry.path)
        }
        afterAccessQueueEmpty = { [weak self] in
            guard let self else { return }
            // After the re-grant pass: if the show folder is still not
            // accessible (user skipped, picked a different folder),
            // leave the app un-loaded so ContentView keeps showing the
            // setup affordance. Otherwise re-enter loadShowFolder, which
            // detaches the heavy work again. Media folders the user
            // still hasn't granted are dropped to match the previous
            // silent-drop behavior in iPadRenderContext::LoadShowFolder.
            guard XLSequenceDocument.obtainAccess(toPath: path, enforceWritable: false) else {
                self.isShowFolderLoaded = false
                return
            }
            let surviving = mediaFolders.filter {
                XLSequenceDocument.obtainAccess(toPath: $0, enforceWritable: false)
            }
            self.loadShowFolder(path: path, mediaFolders: surviving)
        }
        runNextAccessReprompt()
    }

    /// Attempt to load the persisted show folder at app startup.
    /// Returns true if a show folder is configured (and a load is being
    /// kicked off). Since the load itself runs on a detached task,
    /// `isShowFolderLoaded` is generally still false at the moment this
    /// returns — callers should observe `viewModel.isShowFolderLoaded`
    /// with `.onChange` to react to completion.
    @discardableResult
    func restorePersistedShowFolder() -> Bool {
        guard let path = FolderConfig.showFolder else { return false }
        let mediaFolders = FolderConfig.mediaFolders
        loadShowFolder(path: path, mediaFolders: mediaFolders)
        return true
    }

    // MARK: - Access re-prompt queue

    private func enqueueAccessReprompt(label: String, originalPath: String, pickPath: String) {
        accessRepromptQueue.append(AccessRepromptRequest(
            label: label,
            originalPath: originalPath,
            pickPath: pickPath))
    }

    private func runNextAccessReprompt() {
        if accessReprompt != nil { return }   // sheet still showing
        if let next = accessRepromptQueue.first {
            accessRepromptQueue.removeFirst()
            accessReprompt = next
        } else {
            let cb = afterAccessQueueEmpty
            afterAccessQueueEmpty = nil
            cb?()
        }
    }

    /// Called by `AccessRepromptSheet` when the user picks a folder.
    /// We register a fresh bookmark for the picked URL via
    /// `obtainAccess(toPath:)` (which writes the persisted bookmark
    /// keyed by path), then advance the queue. The deferred operation
    /// (in `afterAccessQueueEmpty`) re-checks access for its original
    /// paths and proceeds with whatever now resolves.
    func acceptReprompt(pickedURL: URL) {
        // Picker URL carries an active security scope; calling
        // obtainAccess records a persistent bookmark so future launches
        // can resolve it. iOS grants children of a picked folder
        // transitively, so picking the parent restores access to a
        // previously-stale sequence file as well.
        _ = XLSequenceDocument.obtainAccess(toPath: pickedURL.path,
                                             enforceWritable: false)
        FolderConfig.registerBookmark(from: pickedURL)
        accessReprompt = nil
        runNextAccessReprompt()
    }

    /// Called when the user dismisses the re-grant sheet (Skip /
    /// swipe-down). The deferred operation runs anyway and decides
    /// what to do with the still-stale path — typically log + drop or
    /// leave the app un-loaded so ContentView keeps showing the setup
    /// affordance.
    func cancelReprompt() {
        accessReprompt = nil
        runNextAccessReprompt()
    }

    // MARK: - Sequence

    /// Create a fresh sequence on disk and load it as the active
    /// document (E-2). Matches the desktop New wizard's output —
    /// `type` is one of "Media" / "Animation" / "Effect";
    /// `mediaPath` is used for Media sequences only.
    @discardableResult
    func newSequence(type: String,
                      mediaPath: String,
                      durationMS: Int,
                      frameMS: Int,
                      savePath: String) -> Bool {
        let ok = document.newSequence(
            atPath: savePath,
            type: type,
            mediaPath: mediaPath,
            durationMS: Int32(durationMS),
            frameMS: Int32(frameMS))
        if ok {
            isSequenceLoaded = true
            isDirty = false
            sequenceName = document.sequenceName()
            sequenceDurationMS = Int(document.sequenceDurationMS())
            frameIntervalMS = Int(document.frameIntervalMS())
            hasAudio = document.hasAudio()
            reloadAltTracks()
            reloadRows()
            reloadTagPositions()
            syncClipboardFromPasteboard()   // B99
            loadAvailableEffects()
            loadWaveform(startMS: 0, endMS: sequenceDurationMS)
            startBackgroundRender()
            startDirtyPolling()
            scheduleBrokenMediaScan()
            RecentSequences.record(path: savePath, forShowFolder: showFolderPath)
        }
        return ok
    }

    func openSequence(path: String, forceRender: Bool = false) {
        // Phase A re-prompt UX: pre-check the sequence file's
        // security-scoped bookmark from Swift. Stale bookmark =>
        // queue a re-pick of the parent folder (iOS grants child
        // access transitively). The actual `document.openSequence`
        // call is deferred until the user re-grants.
        if !XLSequenceDocument.obtainAccess(toPath: path, enforceWritable: false) {
            let parent = (path as NSString).deletingLastPathComponent
            enqueueAccessReprompt(label: "sequence file",
                                   originalPath: path,
                                   pickPath: parent)
            afterAccessQueueEmpty = { [weak self] in
                guard let self else { return }
                guard XLSequenceDocument.obtainAccess(toPath: path, enforceWritable: false) else { return }
                self.performOpenSequence(path: path, forceRender: forceRender)
            }
            runNextAccessReprompt()
            return
        }
        performOpenSequence(path: path, forceRender: forceRender)
    }

    private func performOpenSequence(path: String, forceRender: Bool) {
        if document.openSequence(path) {
            isSequenceLoaded = true
            isDirty = false
            sequenceName = document.sequenceName()
            sequenceDurationMS = Int(document.sequenceDurationMS())
            frameIntervalMS = Int(document.frameIntervalMS())
            hasAudio = document.hasAudio()
            reloadAltTracks()
            reloadRows()
            reloadTagPositions()
            syncClipboardFromPasteboard()   // B99
            loadAvailableEffects()
            // Initial load uses a modest sample count; grid re-requests
            // a higher-resolution waveform once it knows the zoom level
            // via `refreshWaveformForZoom`.
            loadWaveform(startMS: 0, endMS: sequenceDurationMS)
            // FSEQ short-circuit: if the user has the feature on and a
            // matching .fseq exists, skip the render and load frame data
            // straight from disk. On any mismatch (stale, wrong shape,
            // missing) we fall through to the normal background render.
            // `forceRender` (used by Batch Render) bypasses the
            // short-circuit so we always re-render against the current
            // layout regardless of any cached fseq.
            if !forceRender,
               let fseqPath = FolderConfig.fseqPath(forXsq: path),
               document.tryLoadFseq(fseqPath: fseqPath, xsqPath: path) {
                isRendering = false
                isRenderDone = true
            } else {
                startBackgroundRender()
            }
            startDirtyPolling()
            // Scan for missing media on open — the full render pass
            // populates the media cache which the scan walks. We run
            // the scan on a utility queue after a short delay so the
            // cache has settled, then hop back to main to update the
            // banner count. Keeps the open path from blocking on I/O.
            scheduleBrokenMediaScan()
            // E-5 — push to the Recent list (scoped to the current
            // show folder) so the next visit to the picker surfaces
            // it. Different show folders keep independent lists.
            RecentSequences.record(path: path, forShowFolder: showFolderPath)
            // E-6 — begin autosave writes for this session. The
            // recovery prompt (when the `.xbkp` is newer than the
            // `.xsq`) is presented by the UI shell after open.
            startAutosaveTimer()
        }
    }

    /// Open an `.xsqz` / `.zip` / `.piz` sequence package. The bridge
    /// extracts into a temp dir, swaps the show folder to it, and opens
    /// the inner `.xsq`. On save, the bridge re-packs the temp dir
    /// back into the original `.xsqz` — see
    /// `XLSequenceDocument.openPackagedSequence(atPath:)` for the
    /// lifecycle details.
    ///
    /// Extraction is dispatched off the main queue because even
    /// modest packages with embedded media can take seconds to
    /// unzip, and the main queue is the SwiftUI update thread.
    /// `isExtractingPackage` flips to true for the duration so UI
    /// shells can show a progress affordance.
    /// Primary entry point for opening an `.xsqz` already located
    /// inside the app sandbox (no iCloud / Files-provider plumbing
    /// needed). Used by the internal Repack + Extract paths where
    /// the sandbox constraint doesn't apply. iOS callers coming in
    /// from a Files tap should use `openPackagedSequence(sandboxPath:
    /// originalURL:)` below, which handles the copy-in + copy-out
    /// book-ending.
    func openPackagedSequence(path: String) {
        openPackagedSequence(sandboxPath: path, originalURL: nil)
    }

    /// Open an `.xsqz` that was copied into the app sandbox from a
    /// Files-provider URL. `originalURL` is the user-tapped URL we
    /// should copy back to on save — iCloud Drive / external items
    /// need this round-trip because their path-based filesystem
    /// access is denied without special entitlements. Pass nil when
    /// the package already lives inside the sandbox.
    func openPackagedSequence(sandboxPath: String, originalURL: URL?) {
        guard !isExtractingPackage else { return }
        isExtractingPackage = true

        // Remember the book-ending info so `saveSequence` can copy
        // the freshly-repacked sandbox file back to the original URL
        // and `closeSequence` can wipe the sandbox scratch dir.
        packageOriginalURL = originalURL
        packageSandboxDir = URL(fileURLWithPath: sandboxPath).deletingLastPathComponent()

        let doc = document
        let path = sandboxPath
        DispatchQueue.global(qos: .userInitiated).async { [weak self] in
            let ok = doc.openPackagedSequence(atPath: path)
            DispatchQueue.main.async { [weak self] in
                guard let self else { return }
                self.isExtractingPackage = false
                if !ok { return }

                // Same post-open wire-up as `openSequence` —
                // identical semantics once the bridge has the
                // inner `.xsq` loaded. If the two paths ever
                // grow more shared work than differs, refactor
                // the body into a `finishSequenceLoaded` helper.
                //
                // Flip `isShowFolderLoaded` true here even when the
                // user hadn't configured a show folder yet (fresh
                // install / App-Store-review case). The bridge's
                // context is now pointing at the extracted temp
                // dir, which is a valid self-contained show folder.
                // Without this the ContentView keeps rendering
                // ShowFolderSetupView forever because the sequencer
                // branch is gated behind isShowFolderLoaded.
                // Mirror the bridge's show-dir path into
                // showFolderPath so any UI that displays the
                // current show folder shows something sensible
                // during the package session.
                self.showFolderPath = self.document.showFolderPath()
                self.isShowFolderLoaded = true
                self.isSequenceLoaded = true
                self.isDirty = false
                self.sequenceName = self.document.sequenceName()
                self.sequenceDurationMS = Int(self.document.sequenceDurationMS())
                self.frameIntervalMS = Int(self.document.frameIntervalMS())
                self.hasAudio = self.document.hasAudio()
                self.reloadAltTracks()
                self.reloadRows()
                self.reloadTagPositions()
                self.syncClipboardFromPasteboard()
                self.loadAvailableEffects()
                self.loadWaveform(startMS: 0, endMS: self.sequenceDurationMS)
                self.startBackgroundRender()
                self.startDirtyPolling()
                self.scheduleBrokenMediaScan()
                // Packaged sequences are deliberately NOT added to
                // recents. They're typically one-shot vendor /
                // downloaded `.xsqz` files outside the user's show
                // folder, the sandbox copy is wiped on close, and
                // the original Files-provider URL's bookmark is not
                // reliably reusable cold. Cluttering "Recent" with
                // entries that can't be re-opened is worse than
                // making the user re-tap from Files.
                self.startAutosaveTimer()
            }
        }
    }

    /// Schedule a media-inventory walk to populate `brokenMediaCount`.
    /// Runs 750 ms after call so the initial render has populated
    /// every referenced entry's cache slot (GetAllMediaPaths reads
    /// the cache, not the settings map).
    private func scheduleBrokenMediaScan() {
        brokenMediaCount = 0
        let doc = document
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.75) { [weak self] in
            guard let self, self.isSequenceLoaded else { return }
            DispatchQueue.global(qos: .utility).async {
                let inv = doc.mediaInventoryInSequence()
                let broken = inv.reduce(0) { acc, dict -> Int in
                    let b = (dict["isBroken"] as? NSNumber)?.boolValue ?? false
                    return acc + (b ? 1 : 0)
                }
                DispatchQueue.main.async { [weak self] in
                    self?.brokenMediaCount = broken
                }
            }
        }
    }

    // MARK: - Save / dirty tracking (E-1)

    /// Attempt to write the sequence to its current on-disk path.
    /// Returns true on success. Failed writes leave `isDirty`
    /// alone — the user keeps their unsaved changes and can retry.
    @discardableResult
    func saveSequence() -> Bool {
        guard isSequenceLoaded else { return false }
        // Playback + scrub mutate state the writer reads; pause
        // before writing so the XML snapshot is stable.
        if isPlaying { pause() }
        if isScrubbing { stopScrub() }
        let path = document.currentSequencePath()
        let ok = coordinatedWrite(at: path) {
            document.saveSequence()
        }
        if ok {
            // For Files-provider `.xsqz` opens: the bridge just
            // repacked the sandbox copy. Copy it back to the
            // original URL (iCloud Drive / external provider) so
            // the user's edits land in the file they actually
            // tapped. Uses URL-based APIs + NSFileCoordinator so
            // the security scope on the original URL (re-resolved
            // from the persistent bookmark) is honoured. A failure
            // here is a genuine save failure — surface it by
            // flipping `ok` false so isDirty stays set and the
            // caller knows.
            if let originalURL = packageOriginalURL,
               document.isPackagedSequence() {
                let sandboxPkgPath = document.packagePath()
                if sandboxPkgPath.isEmpty { return false }
                let sandboxURL = URL(fileURLWithPath: sandboxPkgPath)
                if !copySandboxBackToOriginal(sandboxURL: sandboxURL,
                                               originalURL: originalURL) {
                    return false
                }
            }
            isDirty = false
            // FSEQ companion write: when the user has the feature enabled,
            // emit a v2/zstd/sparse fseq next to (or in the configured fseq
            // folder for) the sequence so FPP / Falcon Connect / Batch
            // Render can pick it up without re-rendering. We only do this
            // on user-initiated saves — autosave goes through
            // writeAutosaveBackup which doesn't touch the fseq.
            if let fseqPath = FolderConfig.fseqPath(forXsq: path) {
                if document.wasRenderAborted() {
                    // The most recent render was aborted before
                    // completion (almost always memory pressure —
                    // see iPadRenderContext::HandleMemoryWarning).
                    // SequenceData is partly stale, so a fseq written
                    // from it would play back wrong. Skip the fseq
                    // write; surface a banner so the user knows to
                    // re-render and re-save once memory recovers.
                    print("saveSequence: skipping fseq write — render was aborted before completing")
                    fseqWriteSkippedMessage =
                        "FSEQ not written — the render was interrupted (likely low memory). Re-render and save again to regenerate it."
                } else {
                    _ = XLSequenceDocument.obtainAccess(toPath: fseqPath,
                                                        enforceWritable: true)
                    if !document.writeFseq(toPath: fseqPath) {
                        print("saveSequence: fseq write to \(fseqPath) failed; .xsq save still succeeded.")
                    } else {
                        // Success path — clear any stale banner from
                        // a prior aborted save.
                        fseqWriteSkippedMessage = nil
                    }
                }
            }
            // Tier 1 memory mitigation — post-save is a safe
            // checkpoint to drop the undo / redo history.
            // Per-step snapshots (settings + palette strings for
            // every captured effect) add up in long editing
            // sessions. Users expect undo within a session, not
            // across save-close-reopen.
            document.clearUndoHistory()
        }
        return ok
    }

    /// Copy a freshly-repacked sandbox `.xsqz` back to the Files-
    /// provider URL the user originally tapped. Uses
    /// NSFileCoordinator for write coordination and URL-based
    /// FileManager APIs so iCloud Drive writes propagate properly.
    private func copySandboxBackToOriginal(sandboxURL: URL, originalURL: URL) -> Bool {
        // Re-activate scope via the persistent bookmark minted at
        // open time. `obtainAccess(toPath:)` resolves the bookmark
        // and starts a fresh scope on a resolved URL that backs the
        // subsequent file-coordinator call.
        _ = XLSequenceDocument.obtainAccess(toPath: originalURL.path,
                                             enforceWritable: true)
        let scopeStarted = originalURL.startAccessingSecurityScopedResource()
        defer {
            if scopeStarted {
                originalURL.stopAccessingSecurityScopedResource()
            }
        }
        var coordErr: NSError?
        var writeErr: Error?
        let coordinator = NSFileCoordinator()
        coordinator.coordinate(writingItemAt: originalURL,
                                options: .forReplacing,
                                error: &coordErr) { writeURL in
            do {
                // .forReplacing hands us a scratch URL to write
                // into; the coordinator atomically swaps it for the
                // real item when the block returns successfully.
                if FileManager.default.fileExists(atPath: writeURL.path) {
                    try FileManager.default.removeItem(at: writeURL)
                }
                try FileManager.default.copyItem(at: sandboxURL, to: writeURL)
            } catch {
                writeErr = error
            }
        }
        if coordErr != nil || writeErr != nil {
            return false
        }
        return true
    }

    /// Save to a new path. Caller is responsible for invoking a
    /// `UIDocumentPicker` / `.fileExporter` to get the destination
    /// URL. The path must end in `.xsq`; security-scoped access is
    /// obtained via `XLSequenceDocument.obtainAccessToPath`.
    @discardableResult
    func saveSequenceAs(path: String) -> Bool {
        guard isSequenceLoaded else { return false }
        if isPlaying { pause() }
        if isScrubbing { stopScrub() }
        XLSequenceDocument.obtainAccess(toPath: path, enforceWritable: true)
        let ok = coordinatedWrite(at: path) {
            document.saveSequence(as: path)
        }
        if ok {
            isDirty = false
            // Tier 1 memory mitigation — same rationale as
            // `saveSequence`: drop undo/redo on a successful
            // save-as.
            document.clearUndoHistory()
            // The bridge updates the underlying SequenceFile's
            // path; refresh the display name so the toolbar picks
            // up the new sequence name.
            sequenceName = document.sequenceName()
        }
        return ok
    }

    /// G-1: wrap the bridge's actual file-write call in
    /// `NSFileCoordinator.coordinate(writingItemAt:options:error:byAccessor:)`
    /// so concurrent Files-app / iCloud-daemon activity can't
    /// corrupt the `.xsq`. Coordinator blocks other file presenters
    /// (Files.app, `UIDocumentBrowserViewController`, iCloud sync)
    /// for the duration of the write. If `path` is empty (new
    /// unsaved sequence) we skip the coordinator — there's nothing
    /// to coordinate against.
    private func coordinatedWrite(at path: String,
                                   _ body: () -> Bool) -> Bool {
        guard !path.isEmpty else {
            return body()
        }
        let url = URL(fileURLWithPath: path)
        let coordinator = NSFileCoordinator(filePresenter: nil)
        var coordinatorError: NSError?
        var writeResult = false
        coordinator.coordinate(writingItemAt: url,
                                options: .forReplacing,
                                error: &coordinatorError) { _ in
            writeResult = body()
        }
        if let err = coordinatorError {
            print("NSFileCoordinator write failed: \(err)")
            return false
        }
        return writeResult
    }

    /// Present-time dirty-state check from SwiftUI views that
    /// haven't already observed `isDirty`. Reads through the bridge
    /// for the freshest value — the poll timer updates the
    /// observable at 500ms cadence, which may lag a very recent
    /// mutation for a few hundred milliseconds otherwise.
    func checkDirtyNow() -> Bool {
        guard isSequenceLoaded else { return false }
        let dirty = document.isSequenceDirty()
        if dirty != isDirty {
            isDirty = dirty
        }
        return dirty
    }

    private func startDirtyPolling() {
        stopDirtyPolling()
        dirtyPollTimer = Timer.scheduledTimer(withTimeInterval: 0.5,
                                               repeats: true) { [weak self] _ in
            // Timer was scheduled on the main runloop from a @MainActor
            // method, so it always fires here on main; assumeIsolated
            // is the type-system bridge to that runtime contract.
            MainActor.assumeIsolated {
                guard let self else { return }
                let dirty = self.document.isSequenceDirty()
                if dirty != self.isDirty {
                    self.isDirty = dirty
                }
            }
        }
    }

    private func stopDirtyPolling() {
        dirtyPollTimer?.invalidate()
        dirtyPollTimer = nil
    }

    // MARK: - Autosave (E-6)

    /// Start the `.xbkp` autosave timer. Fires every
    /// `autosaveIntervalMinutes`; each tick asks the bridge to
    /// serialise the current in-memory sequence to
    /// `<basename>.xbkp` alongside the `.xsq`. Skips writes when
    /// playback / scrub is active (consumer of CPU) or when the
    /// dirty count hasn't advanced since the last successful
    /// autosave — matches desktop's `OnTimer_AutoSaveTrigger`
    /// gate behaviour in `xLightsMain.cpp:4635-4682`.
    func startAutosaveTimer() {
        stopAutosaveTimer()
        guard autosaveIntervalMinutes > 0 else { return }
        let interval = TimeInterval(autosaveIntervalMinutes * 60)
        autosaveTimer = Timer.scheduledTimer(withTimeInterval: interval,
                                              repeats: true) { [weak self] _ in
            MainActor.assumeIsolated {
                self?.tickAutosave()
            }
        }
    }

    func stopAutosaveTimer() {
        autosaveTimer?.invalidate()
        autosaveTimer = nil
    }

    private func tickAutosave() {
        guard isSequenceLoaded else { return }
        if isPlaying || isScrubbing { return }
        // Only write when there's something dirty to protect —
        // matches desktop's "skip autosave when no changes since
        // last save" branch.
        if !document.isSequenceDirty() { return }
        // G-1: coordinate the .xbkp write too; same concurrency
        // risk as the main sequence path.
        _ = coordinatedWrite(at: autosaveBackupPath) {
            document.writeAutosaveBackup()
        }
    }

    /// Path of the `.xbkp` that sits alongside the current
    /// sequence. Empty when no sequence is open.
    var autosaveBackupPath: String {
        let p = document.currentSequencePath()
        if p.isEmpty { return "" }
        return (p as NSString).deletingPathExtension + ".xbkp"
    }

    /// True when an autosave backup sits next to the current
    /// sequence AND is newer than the `.xsq` by more than a few
    /// seconds. Used by the open-time recovery sheet to decide
    /// whether the user needs to be offered the backup.
    func hasRecoverableBackup() -> (has: Bool, when: Date?) {
        let xsqPath = document.currentSequencePath()
        if xsqPath.isEmpty { return (false, nil) }
        let bkpPath = (xsqPath as NSString).deletingPathExtension + ".xbkp"
        let fm = FileManager.default
        guard fm.fileExists(atPath: bkpPath) else { return (false, nil) }
        guard let bkpAttrs = try? fm.attributesOfItem(atPath: bkpPath),
              let bkpDate = bkpAttrs[.modificationDate] as? Date
        else { return (false, nil) }
        if fm.fileExists(atPath: xsqPath) {
            if let xsqAttrs = try? fm.attributesOfItem(atPath: xsqPath),
               let xsqDate = xsqAttrs[.modificationDate] as? Date,
               bkpDate <= xsqDate.addingTimeInterval(2) {
                return (false, nil)
            }
        }
        return (true, bkpDate)
    }

    /// Replace the canonical `.xsq` with the `.xbkp` contents by
    /// swapping filenames on disk, then reloading. Desktop's
    /// recovery flow renames `foo.xbkp` → `foo.xsq` to claim the
    /// backup; we do the same through FileManager.
    func applyAutosaveBackup() -> Bool {
        let xsqPath = document.currentSequencePath()
        if xsqPath.isEmpty { return false }
        let bkpPath = (xsqPath as NSString).deletingPathExtension + ".xbkp"
        let fm = FileManager.default
        guard fm.fileExists(atPath: bkpPath) else { return false }
        // Move xsq aside (just in case) and promote the backup.
        let archived = xsqPath + ".pre-xbkp-recovery"
        _ = try? fm.removeItem(atPath: archived)
        try? fm.moveItem(atPath: xsqPath, toPath: archived)
        do {
            try fm.moveItem(atPath: bkpPath, toPath: xsqPath)
        } catch {
            // Rollback: put the original back.
            try? fm.moveItem(atPath: archived, toPath: xsqPath)
            return false
        }
        try? fm.removeItem(atPath: archived)
        // Re-open so in-memory state matches the promoted file.
        closeSequence()
        openSequence(path: xsqPath)
        return true
    }

    /// Touch the `.xbkp`'s mtime so it's older than the `.xsq`
    /// (or nearly equal), suppressing the recovery offer next
    /// time the sequence opens. Called from the "Discard
    /// Recovery" button and from the close-with-save path.
    func suppressAutosaveBackup() {
        let xsqPath = document.currentSequencePath()
        if xsqPath.isEmpty { return }
        let bkpPath = (xsqPath as NSString).deletingPathExtension + ".xbkp"
        let fm = FileManager.default
        guard fm.fileExists(atPath: bkpPath) else { return }
        try? fm.removeItem(atPath: bkpPath)
    }

    /// Called when the app scene moves to `.background` — iOS may kill
    /// the process at any time from that point. Abort all in-flight
    /// render jobs and wait briefly so the workers unwind before we
    /// (or the system) tear down the sequence data they're reading.
    /// Also pause playback / scrub so the 30fps UI timers, audio
    /// playback, and preview display-links (which key off `isPlaying`
    /// / `isScrubbing`) all stop burning energy in the background.
    /// Pauses rather than stops so the playhead position is preserved
    /// if the user returns without the app being killed.
    func shutdownForBackground() {
        quiesceForInactive()
        // G-4: stop controller output cleanly. iOS throttles
        // backgrounded apps' network traffic, so an active
        // sACN/ArtNet/DDP stream becomes unreliable; better to
        // halt it than send partial frames. User re-enables on
        // foreground via the existing output toggle.
        if isOutputting {
            document.stopOutput()
            isOutputting = false
        }
        guard isSequenceLoaded else { return }
        cancelBackgroundRender()
        _ = document.abortRenderAndWait(3.0)
    }

    /// Called on `.inactive` (multitasking switcher, incoming call,
    /// Control Center pulled down, etc.) — temporary suspension. Stop
    /// anything that drives continuous work but keep the render state
    /// intact so a return to `.active` doesn't have to rebuild it.
    /// `.background` follows `.inactive` if the app goes away for
    /// real; that path also calls this via `shutdownForBackground()`.
    func quiesceForInactive() {
        if isPlaying {
            pause()
        }
        if isScrubbing {
            stopScrub()
        }
    }

    func closeSequence() {
        if isOutputting { toggleOutput() }
        stopPlayback()
        stopDirtyPolling()
        stopAutosaveTimer()
        cancelBackgroundRender()
        // Wait for any background render jobs to exit before tearing
        // down SequenceElements / SequenceData — the render workers
        // hold pointers into those structures and would crash on next
        // frame access if we proceeded with close while they're busy.
        _ = document.abortRenderAndWait(5.0)
        let wasPackaged = packageSandboxDir != nil
        document.closeSequence()

        // Wipe the sandbox scratch dir used for Files-provider
        // `.xsqz` opens (if any). The bridge's closeSequence wiped
        // the *extraction* temp dir; this wipes the separate
        // "incoming copy" dir that held the sandbox `.xsqz` itself.
        if let sandbox = packageSandboxDir {
            try? FileManager.default.removeItem(at: sandbox)
        }
        packageSandboxDir = nil
        packageOriginalURL = nil

        // Post-package cleanup: if we opened an `.xsqz` on a fresh
        // install with no pre-existing show folder, the bridge's
        // show dir was pointed at the (now-wiped) temp dir. There's
        // nothing sensible to route the user to, so drop back to
        // the show-folder setup screen. FolderConfig.showFolder
        // (the persisted config) tells us whether a "real" show
        // folder was ever configured — if so, the bridge's
        // closeSequence already re-loaded it and we leave
        // isShowFolderLoaded true.
        if wasPackaged && FolderConfig.showFolder == nil {
            isShowFolderLoaded = false
            showFolderPath = ""
        }

        isSequenceLoaded = false
        isDirty = false
        isRenderDone = false
        isRendering = false
        sequenceName = nil
        hasAudio = false
        rows = []
        waveformPeaks = []
        onsetTimesMS = []
        showOnsets = false
        onsetsComputed = false
        soundClasses = [:]
        selectedSoundClass = nil
        isClassifyingSound = false
        pitchContour = []
        showPitchContour = false
        pitchComputed = false
        isComputingPitch = false
        showSpectrogram = false
        spectrogramReady = false
        stemsPhase = .idle
        stemsProgressPct = 0
        stemsAvailable = false
        stemsPendingFilter = nil
        stemsInstallRoot = nil
    }

    // MARK: - Memory Pressure

    /// Register for memory-pressure signals and start polling
    /// `os_proc_available_memory()`. Call once at app startup.
    func startMemoryMonitoring() {
        guard memoryPressureSource == nil else { return }

        let source = DispatchSource.makeMemoryPressureSource(
            eventMask: [.warning, .critical],
            queue: .main
        )
        source.setEventHandler { [weak self, weak source] in
            guard let self, let source else { return }
            let event = source.data
            if event.contains(.critical) {
                self.document.handleMemoryCritical()
            } else if event.contains(.warning) {
                self.document.handleMemoryWarning()
            }
            // Tier 1 memory mitigation — XLGridMetalBridge and any
            // other Swift-side observers subscribe to this
            // notification to drop their caches. Posted after the
            // C++ handlers so the core has already trimmed what it
            // can before the UI layer starts rebuilding textures.
            NotificationCenter.default.post(
                name: NSNotification.Name("XLMemoryWarning"), object: nil)
            self.memoryWarning = true
        }
        source.activate()
        memoryPressureSource = source

        memoryWarningObserver = NotificationCenter.default.addObserver(
            forName: UIApplication.didReceiveMemoryWarningNotification,
            object: nil,
            queue: .main
        ) { [weak self] _ in
            // queue: .main pins the observer block to the main thread.
            MainActor.assumeIsolated {
                self?.document.handleMemoryWarning()
                NotificationCenter.default.post(
                    name: NSNotification.Name("XLMemoryWarning"), object: nil)
                self?.memoryWarning = true
            }
        }

        memoryPollTimer = Timer.scheduledTimer(withTimeInterval: 5.0, repeats: true) { [weak self] _ in
            MainActor.assumeIsolated {
                guard let self else { return }
                let mb = XLSequenceDocument.availableMemoryMB()
                if self.memoryWarning {
                    // Clear once we're comfortably above the threshold (hysteresis).
                    if mb >= Self.memoryRecoveredThresholdMB {
                        self.memoryWarning = false
                    }
                } else if mb < Self.memoryWarningThresholdMB {
                    self.memoryWarning = true
                }
            }
        }
    }

    func stopMemoryMonitoring() {
        memoryPressureSource?.cancel()
        memoryPressureSource = nil
        memoryPollTimer?.invalidate()
        memoryPollTimer = nil
        if let obs = memoryWarningObserver {
            NotificationCenter.default.removeObserver(obs)
            memoryWarningObserver = nil
        }
    }

    // MARK: - Playback Controls

    func play() {
        stopScrub()
        if hasAudio {
            // Apply current rate before starting — AVAudioEngine
            // reads the time-pitch unit's rate at Play time and
            // applies it to the about-to-start playback.
            document.setAudioPlaybackRate(playSpeed)
            document.audioPlay()
        }
        // Record wall clock reference for timer-driven playback
        playbackStartTime = CFAbsoluteTimeGetCurrent()
        playbackStartMS = playPositionMS
        isPlaying = true
        isPaused = false
        startPlaybackTimer()
    }

    func pause() {
        if hasAudio {
            document.audioPause()
        }
        isPlaying = false
        isPaused = true
        stopPlaybackTimer()
    }

    func stop() {
        if hasAudio {
            document.audioStop()
        }
        isPlaying = false
        isPaused = false
        playPositionMS = 0
        stopPlaybackTimer()
    }

    func togglePlayPause() {
        if isPlaying {
            pause()
        } else {
            if isPaused {
                play()
            } else {
                if hasAudio {
                    document.audioSeek(toMS: Int(playPositionMS))
                }
                play()
            }
        }
    }

    func seekTo(ms: Int) {
        let clamped = max(0, min(ms, sequenceDurationMS))
        playPositionMS = clamped
        if hasAudio {
            document.audioSeek(toMS: Int(clamped))
        }
    }

    /// B40: variant of `seekTo` for ruler drag-to-scrub. Plays a tiny
    /// audio window (~50 ms) at the new position so the user can hear
    /// where they are. Throttled to 50 ms between bursts so a fast
    /// drag doesn't fire dozens of overlapping plays — the burst length
    /// matches the throttle so back-to-back calls cover the swept range
    /// continuously without piling up.
    func scrubSeekTo(ms: Int) {
        let clamped = max(0, min(ms, sequenceDurationMS))
        playPositionMS = clamped
        guard hasAudio, !isPlaying else { return }
        let now = Date().timeIntervalSinceReferenceDate
        if now - lastScrubAudioBurstAt >= 0.050 {
            document.audioPlaySegment(fromMS: Int(clamped), lengthMS: 50)
            lastScrubAudioBurstAt = now
        }
        // Even when we throttle the audible burst, keep the seek
        // honest so the next bullet (or the final commit on drag-end)
        // resumes from the right spot.
        document.audioSeek(toMS: Int(clamped))
    }

    func stopPlayback() {
        stop()
    }

    // MARK: - Loop region (B32 / B33 / B44)

    /// B32: set the loop region to `[startMS, endMS]`, clamped to
    /// the sequence bounds. If the range collapses (end <= start)
    /// the region is cleared.
    func setLoopRegion(startMS: Int, endMS: Int) {
        let lo = max(0, min(sequenceDurationMS, min(startMS, endMS)))
        let hi = max(0, min(sequenceDurationMS, max(startMS, endMS)))
        if hi <= lo {
            clearLoopRegion()
            return
        }
        loopStartMS = lo
        loopEndMS = hi
    }

    /// B32: clear the loop region + turn off play-loop mode.
    func clearLoopRegion() {
        loopStartMS = 0
        loopEndMS = 0
        loopPlayEnabled = false
    }

    /// B33: toggle play-loop. When enabled and `hasLoopRegion`, the
    /// playback timer wraps `playPositionMS` back to `loopStartMS`
    /// each time it crosses `loopEndMS`. No-op when the region is
    /// cleared.
    func toggleLoopPlay() {
        guard hasLoopRegion else { loopPlayEnabled = false; return }
        loopPlayEnabled.toggle()
    }

    /// B92: set the loop region to the given timing-mark bounds
    /// and start looping playback from the mark's start. Used by
    /// double-tap on a timing mark — gives users a quick way to
    /// preview one phrase / word / phoneme in isolation without
    /// hand-dragging the loop region.
    func playLoopForTimingMark(rowIndex: Int, markIndex: Int) {
        guard rowIndex >= 0, rowIndex < rows.count else { return }
        let row = rows[rowIndex]
        guard markIndex >= 0, markIndex < row.effects.count else { return }
        let mark = row.effects[markIndex]
        setLoopRegion(startMS: mark.startTimeMS, endMS: mark.endTimeMS)
        loopPlayEnabled = true
        seekTo(ms: mark.startTimeMS)
        if !isPlaying { togglePlayPause() }
    }

    // MARK: - Tags (B34 / B35)

    /// Pull the 10 tag positions from the bridge into the observable
    /// `tagPositions` array. Call on sequence open / post-save.
    func reloadTagPositions() {
        var out: [Int] = []
        out.reserveCapacity(10)
        for i in 0..<10 {
            out.append(Int(document.tagPosition(at: Int32(i))))
        }
        tagPositions = out
    }

    /// B34 — set tag `idx` (0..9) to `atMS`. Passing -1 clears the
    /// tag (but callers should use `clearTag(_:)` for readability).
    /// Marks the sequence dirty via the bridge write.
    func setTag(_ idx: Int, atMS: Int) {
        guard idx >= 0 && idx < 10 else { return }
        document.setTagPosition(at: Int32(idx), positionMS: Int32(atMS))
        reloadTagPositions()
    }

    /// B34 — clear tag `idx` (sets position to -1).
    func clearTag(_ idx: Int) {
        guard idx >= 0 && idx < 10 else { return }
        document.setTagPosition(at: Int32(idx), positionMS: -1)
        reloadTagPositions()
    }

    /// B35 — clear all tags at once (desktop "Delete All").
    func clearAllTags() {
        document.clearAllTags()
        reloadTagPositions()
    }

    /// B34 — jump the play head to tag `idx` (no-op if unset).
    func goToTag(_ idx: Int) {
        guard idx >= 0 && idx < 10 else { return }
        let pos = tagPositions[idx]
        if pos >= 0 { seekTo(ms: pos) }
    }

    /// B44: render only the loop region. Uses the existing
    /// per-model render primitive; touches every row so the output
    /// buffer gets refreshed across the range.
    func renderLoopRegion() {
        guard hasLoopRegion else { return }
        for (idx, row) in rows.enumerated() {
            if row.timing != nil { continue }
            renderRangeAndTrack(rowIndex: idx,
                                startMS: loopStartMS,
                                endMS: loopEndMS,
                                clear: false)
        }
    }

    func setVolume(_ vol: Int) {
        volume = max(0, min(100, vol))
        document.setAudioVolume(Int32(volume))
    }

    // F-4 playback speed. Applies the new rate to the audio engine
    // (if audible) and rebases the wall-clock reference so the
    // no-audio timer path keeps advancing from the current position
    // without an instantaneous jump.
    func setPlaybackSpeed(_ speed: Float) {
        let clamped = max(0.1, min(speed, 8.0))
        if abs(clamped - playSpeed) < 0.001 { return }
        playSpeed = clamped
        // Always push to the audio engine — it's a no-op when no
        // sequence is loaded and harmless to set ahead of a play().
        document.setAudioPlaybackRate(clamped)
        // Rebase no-audio timer anchor so the tick keeps advancing
        // from the current position at the new rate.
        if isPlaying && !hasAudio {
            playbackStartTime = CFAbsoluteTimeGetCurrent()
            playbackStartMS = playPositionMS
        }
    }

    // MARK: - Playback Timer

    private func startPlaybackTimer() {
        stopPlaybackTimer()
        let interval = Double(frameIntervalMS) / 1000.0
        playbackTimer = Timer.scheduledTimer(withTimeInterval: interval, repeats: true) { [weak self] _ in
            MainActor.assumeIsolated {
                guard let self else { return }

                if self.hasAudio {
                    // Audio-driven: poll audio position
                    let pos = self.document.audioTellMS()
                    self.playPositionMS = Int(pos)

                    let state = self.document.audioPlayingState()
                    // Audio naturally ending past the sequence length doesn't
                    // always flip _media_state to STOPPED — the backend only does
                    // that on explicit Stop(). Treat "past end" as end-of-playback.
                    if state == 2 || self.playPositionMS >= self.sequenceDurationMS {
                        self.playPositionMS = self.sequenceDurationMS
                        self.document.audioStop()
                        self.isPlaying = false
                        self.isPaused = false
                        self.stopPlaybackTimer()
                        return
                    }
                    // B33 play-loop: wrap back to loopStart when the
                    // head crosses loopEnd. Audio seek is used to keep
                    // the audio stream in sync; playback continues
                    // without re-issuing `audioPlay`.
                    if self.loopPlayEnabled, self.hasLoopRegion,
                       self.playPositionMS >= self.loopEndMS {
                        self.playPositionMS = self.loopStartMS
                        self.document.audioSeek(toMS: self.loopStartMS)
                    }
                } else {
                    // Timer-driven: use wall clock elapsed since play
                    // started, scaled by the current playback speed
                    // (animation sequences without audio can't ride the
                    // audio engine's time-pitch unit).
                    let wallElapsed = CFAbsoluteTimeGetCurrent() - self.playbackStartTime
                    let elapsedMS = Int(wallElapsed * 1000.0 * Double(self.playSpeed))
                    let pos = self.playbackStartMS + elapsedMS
                    if pos >= self.sequenceDurationMS {
                        self.playPositionMS = self.sequenceDurationMS
                        self.isPlaying = false
                        self.isPaused = false
                        self.stopPlaybackTimer()
                        return
                    }
                    self.playPositionMS = pos
                    // B33 play-loop (no-audio path): reset the wall-clock
                    // anchor so the next tick starts measuring from the
                    // loop's start.
                    if self.loopPlayEnabled, self.hasLoopRegion,
                       self.playPositionMS >= self.loopEndMS {
                        self.playPositionMS = self.loopStartMS
                        self.playbackStartMS = self.loopStartMS
                        self.playbackStartTime = CFAbsoluteTimeGetCurrent()
                    }
                }

                self.sendOutputFrame()
            }
        }
    }

    private func stopPlaybackTimer() {
        playbackTimer?.invalidate()
        playbackTimer = nil
    }

    // MARK: - Selection-scoped Preview Scrub

    /// Start a preview loop over `[startMS, endMS]`. Advances
    /// `playPositionMS` at frame rate; wraps back to `startMS` on
    /// reaching `endMS`. Audio isn't played. Silently skips if audio
    /// is playing — desktop parity: real playback wins.
    func startScrub(startMS: Int, endMS: Int) {
        guard endMS > startMS else { return }
        guard !isPlaying else { return }
        stopScrub()
        scrubStartMS = startMS
        scrubEndMS = endMS
        playPositionMS = startMS
        isScrubbing = true
        let interval = Double(frameIntervalMS) / 1000.0
        scrubTimer = Timer.scheduledTimer(withTimeInterval: interval, repeats: true) { [weak self] _ in
            MainActor.assumeIsolated {
                guard let self else { return }
                let next = self.playPositionMS + self.frameIntervalMS
                self.playPositionMS = (next >= self.scrubEndMS) ? self.scrubStartMS : next
            }
        }
    }

    func stopScrub() {
        scrubTimer?.invalidate()
        scrubTimer = nil
        isScrubbing = false
    }

    // MARK: - Background Rendering

    func startBackgroundRender() {
        guard isSequenceLoaded else { return }

        // Already rendering → abort cleanly before spinning up a
        // fresh worker. Skipping this would reallocate `SequenceData.
        // _frames` via `SequenceData::init()` while the current
        // render's worker threads are still dereferencing it via
        // `operator[](frame)`, which libc++ catches as an OOB access
        // assertion. Abort runs off-main so the UI stays responsive
        // (spinner keeps spinning) while we wait for workers to
        // unwind; once they're done we chain into the fresh render.
        if isRendering {
            renderPollTimer?.invalidate()
            renderPollTimer = nil
            let doc = document
            DispatchQueue.global(qos: .userInitiated).async { [weak self] in
                _ = doc.abortRenderAndWait(3.0)
                DispatchQueue.main.async {
                    guard let self else { return }
                    self.beginFreshRender()
                }
            }
            return
        }

        beginFreshRender()
    }

    private func beginFreshRender() {
        isRendering = true
        isRenderDone = false
        let doc = document
        let thread = Thread {
            doc.renderAll()
        }
        thread.qualityOfService = .userInitiated
        thread.start()

        // Poll for completion
        renderPollTimer?.invalidate()
        renderPollTimer = Timer.scheduledTimer(withTimeInterval: 0.25, repeats: true) { [weak self] timer in
            // Timer isn't Sendable, so we can't reference `timer` from
            // inside the assumeIsolated body — invalidate it out here
            // before crossing into main-actor land.
            let done = doc.isRenderDone()
            if done { timer.invalidate() }
            MainActor.assumeIsolated {
                if done {
                    self?.renderPollTimer = nil
                    self?.isRendering = false
                    self?.isRenderDone = true
                }
            }
        }
    }

    private func cancelBackgroundRender() {
        renderPollTimer?.invalidate()
        renderPollTimer = nil
    }

    /// Start (or reuse) a short poll that bumps
    /// `renderedBackgroundsRevision` once `isRenderDone()` returns
    /// true. Call this after any render kickoff (per-effect edit,
    /// range re-render, move/delete) so the effect grid redraws once
    /// the renderer finishes populating effect background geometry.
    /// Multiple concurrent render kickoffs coalesce into a single
    /// running poll via `renderPollInFlight`. Uses
    /// `DispatchQueue.main.asyncAfter` rather than `Timer` because
    /// Timers schedule on RunLoop `.default` only — during pinch /
    /// scroll interactions the RunLoop is in `.tracking` mode and
    /// the timer pauses. asyncAfter fires in all RunLoop modes.
    @MainActor
    func trackRenderCompletion() {
        guard !renderPollInFlight else { return }
        renderPollInFlight = true
        scheduleRenderPoll()
    }

    @MainActor
    private func scheduleRenderPoll() {
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.15) { [weak self] in
            guard let self = self else { return }
            if self.document.isRenderDone() {
                self.renderPollInFlight = false
                self.renderedBackgroundsRevision &+= 1
            } else {
                self.scheduleRenderPoll()
            }
        }
    }

    /// Wrappers around the two `document.render…` entry points that
    /// always start a completion-tracking poll. Every render-triggering
    /// call site in this file goes through one of these so the grid
    /// stays in sync with the renderer.
    private func renderEffectAndTrack(rowIndex: Int, effectIndex: Int) {
        document.renderEffect(forRow: Int32(rowIndex), at: Int32(effectIndex))
        trackRenderCompletion()
    }
    private func renderRangeAndTrack(rowIndex: Int,
                                      startMS: Int, endMS: Int,
                                      clear: Bool) {
        document.renderRange(forRow: Int32(rowIndex),
                              startMS: Int32(startMS),
                              endMS: Int32(endMS),
                              clear: clear)
        trackRenderCompletion()
    }

    func togglePreview() {
        showPreview.toggle()
    }

    /// Set the model shown in the Model Preview pane to the model at the given
    /// grid row. Tapping the same row again clears the selection. Non-model
    /// rows (e.g. timings) are ignored.
    func selectPreviewModel(rowIndex: Int) {
        let name = document.rowModelName(at: Int32(rowIndex))
        if name.isEmpty { return }
        if previewModelName == name {
            previewModelName = nil
        } else {
            previewModelName = name
        }
    }

    /// Set the Model Preview's model without toggling — used when effect
    /// selection should drive the preview and repeated selections on the
    /// same row must not deselect.
    func setPreviewModel(rowIndex: Int) {
        let name = document.rowModelName(at: Int32(rowIndex))
        if name.isEmpty { return }
        previewModelName = name
    }

    // MARK: - Controller Output

    /// Toggle controller output. Returns nil on success (either
    /// successfully started, or successfully stopped); returns a
    /// user-facing error message when a start attempt fails so the
    /// caller can surface it. The two failure modes — no controllers
    /// configured vs. controllers configured but none reachable — get
    /// distinct messages because the remediation differs (set up
    /// controllers vs. check the network).
    @discardableResult
    func toggleOutput() -> String? {
        if isOutputting {
            document.stopOutput()
            isOutputting = false
            return nil
        }
        if document.outputCount() == 0 {
            return "No controllers are configured for this show. Set up controllers in desktop xLights and copy the show folder to the iPad."
        }
        isOutputting = document.startOutput()
        if !isOutputting {
            return "Couldn't reach any of the configured controllers. Check that the iPad is on the same network as the controllers."
        }
        return nil
    }

    private func sendOutputFrame() {
        if isOutputting {
            document.outputFrame(Int32(playPositionMS))
        }
    }

    // MARK: - Effect Selection & Editing

    func selectEffect(rowIndex: Int, effectIndex: Int) {
        let row = rows[rowIndex]
        let effect = row.effects[effectIndex]
        let sel = EffectSelection(
            rowIndex: rowIndex,
            effectIndex: effectIndex,
            name: effect.name,
            startTimeMS: effect.startTimeMS,
            endTimeMS: effect.endTimeMS
        )
        selectedEffect = sel
        selectedEffects = [sel]
        showInspector = true
        // Route the Model Preview to this effect's model so the scrub
        // loop below is visible in the pane.
        setPreviewModel(rowIndex: rowIndex)
        startScrub(startMS: effect.startTimeMS, endMS: effect.endTimeMS)

        // Merge settings map (E_/B_/T_) and palette map (C_) into a single
        // observed dictionary so the SwiftUI controls re-render on change.
        var merged: [String: String] = [:]
        let settings = document.effectSettings(forRow: Int32(rowIndex), at: Int32(effectIndex))
        for (k, v) in settings { merged[k] = v }
        let palette = document.effectPalette(forRow: Int32(rowIndex), at: Int32(effectIndex))
        for (k, v) in palette { merged[k] = v }
        selectedEffectSettings = merged

        // Load (and cache) the effect-specific metadata.
        selectedEffectMetadata = loadEffectMetadata(effect.name)

        // Shared metadata is the same for every effect; load once and reuse.
        if bufferMetadata == nil { bufferMetadata = loadSharedMetadata("Buffer") }
        if colorMetadata == nil { colorMetadata = loadSharedMetadata("Color") }
        if blendingMetadata == nil { blendingMetadata = loadSharedMetadata("Blending") }
    }

    func clearSelection() {
        selectedEffect = nil
        selectedEffects = []
        selectedEffectSettings = [:]
        selectedEffectMetadata = nil
        stopScrub()
    }

    /// Re-read the selected effect's settings + palette map from the
    /// document and kick the SwiftUI observables. Used when a bulk
    /// write happens outside the usual per-setting `setSettingValue`
    /// path — e.g. applying a saved palette file replaces all 8
    /// `C_BUTTON_Palette*` at once on the native side.
    func refreshSelectedEffectSettings() {
        guard let sel = selectedEffect else { return }
        var merged: [String: String] = [:]
        let settings = document.effectSettings(forRow: Int32(sel.rowIndex),
                                                at: Int32(sel.effectIndex))
        for (k, v) in settings { merged[k] = v }
        let palette = document.effectPalette(forRow: Int32(sel.rowIndex),
                                              at: Int32(sel.effectIndex))
        for (k, v) in palette { merged[k] = v }
        selectedEffectSettings = merged
        renderEffectAndTrack(rowIndex: sel.rowIndex, effectIndex: sel.effectIndex)
    }

    /// Replace the full selection set (marquee result). If `newSet` is
    /// empty, acts like `clearSelection`. If size 1, promotes that
    /// effect to the single-select path so inspector + scrub + drag
    /// handles all work normally. For N > 1 (G41): picks the top-left
    /// effect as the anchor (smallest row, then smallest effectIndex)
    /// and drives the inspector from that one. Individual property
    /// controls read the anchor's values; the header chrome counts the
    /// full set so bulk operations (G11/G14) can target every member.
    /// Scrub is suppressed — the scrub loop is single-effect only.
    func setMultiSelection(_ newSet: Set<EffectSelection>) {
        if newSet.isEmpty {
            clearSelection()
            return
        }
        if newSet.count == 1, let only = newSet.first {
            selectEffect(rowIndex: only.rowIndex, effectIndex: only.effectIndex)
            return
        }
        stopScrub()

        // Anchor = top-left: smallest rowIndex, then smallest
        // effectIndex. Matches desktop's "first selected" in the
        // common marquee case. Falls back to the raw first on ties
        // (won't happen — (row,idx) tuples are unique).
        let anchor = newSet.min(by: { a, b in
            if a.rowIndex != b.rowIndex { return a.rowIndex < b.rowIndex }
            return a.effectIndex < b.effectIndex
        })!

        selectedEffect = anchor
        selectedEffects = newSet
        showInspector = true
        setPreviewModel(rowIndex: anchor.rowIndex)

        // Load anchor's merged settings + metadata so the inspector
        // renders current values. Identical to the single-select
        // path except scrub is skipped.
        var merged: [String: String] = [:]
        let settings = document.effectSettings(forRow: Int32(anchor.rowIndex),
                                                at: Int32(anchor.effectIndex))
        for (k, v) in settings { merged[k] = v }
        let palette = document.effectPalette(forRow: Int32(anchor.rowIndex),
                                              at: Int32(anchor.effectIndex))
        for (k, v) in palette { merged[k] = v }
        selectedEffectSettings = merged
        selectedEffectMetadata = loadEffectMetadata(anchor.name)
        if bufferMetadata == nil { bufferMetadata = loadSharedMetadata("Buffer") }
        if colorMetadata == nil { colorMetadata = loadSharedMetadata("Color") }
        if blendingMetadata == nil { blendingMetadata = loadSharedMetadata("Blending") }
    }

    /// Whether the inspector is in multi-effect mode. True when more
    /// than one effect is selected; drives the "N effects selected"
    /// header chrome (G41) and gates bulk context-menu entries
    /// (G11/G14).
    var isMultiEffectSelection: Bool {
        selectedEffects.count > 1
    }

    // MARK: - Metadata Loading

    private func loadEffectMetadata(_ effectName: String) -> EffectMetadata? {
        if let cached = metadataCache[effectName] { return cached }
        let json = document.metadataJson(forEffectNamed: effectName)
        guard let md = parseEffectMetadata(json) else { return nil }
        metadataCache[effectName] = md
        return md
    }

    private func loadSharedMetadata(_ name: String) -> EffectMetadata? {
        let json = document.sharedMetadataJsonNamed(name)
        return parseEffectMetadata(json)
    }

    // MARK: - Effect Setting Access (by key)

    /// Read a single setting value, falling back to the metadata default if the
    /// effect hasn't explicitly stored a value for this key. Reads from the
    /// observed selectedEffectSettings dict so SwiftUI tracks the dependency
    /// and re-renders controls when values change.
    func settingValue(forKey key: String, defaultValue: String) -> String {
        if let cached = selectedEffectSettings[key], !cached.isEmpty {
            return cached
        }
        return defaultValue
    }

    /// Resolve a JSON metadata `dynamicOptions` source to an option list.
    /// Mirrors desktop's JsonEffectPanel repopulate lambdas. Returns [] if
    /// no effect is selected (state/face/node sources need the effect's
    /// parent element's model) or if the source is unknown.
    /// `propertyId` is only consulted for `source == "effect"`, where it
    /// becomes the setting id passed to `RenderableEffect::GetSettingOptions`.
    func dynamicOptions(source: String, propertyId: String) -> [String] {
        switch source {
        case "timingTracks":
            return document.timingTrackNames()
        case "lyricTimingTracks":
            return document.lyricTimingTrackNames()
        case "cameras":
            // PerPreviewCamera: "2D" plus every 3D camera defined in
            // the show's ViewpointMgr. Populated by Phase D-3's
            // ViewpointMgr bridging at show-load time.
            return document.perPreviewCameraNames()
        default:
            break
        }

        guard let sel = selectedEffect else { return [] }
        let row = Int32(sel.rowIndex)
        let idx = Int32(sel.effectIndex)
        switch source {
        case "states":
            return document.states(forRow: row, at: idx)
        case "faces":
            return document.faces(forRow: row, at: idx)
        case "modelNodeNames":
            return document.modelNodeNames(forRow: row, at: idx)
        case "effect":
            return document.effectSettingOptions(forRow: row,
                                                  at: idx,
                                                  settingId: propertyId)
        default:
            return []
        }
    }

    /// range if the value actually changed. When `suppressIfDefault` is
    /// non-nil and the new value equals that default, the settings map
    /// entry is removed instead of written — matches the desktop
    /// `suppressIfDefault:true` JSON metadata flag so we don't persist
    /// redundant defaults in the effect string. Registers an undo step
    /// so Cmd+Z reverts the inspector change; the undo re-writes the
    /// previous value via `setSettingValueAt` even if the forward action
    /// was a remove.
    func setSettingValue(_ value: String,
                          forKey key: String,
                          suppressIfDefault: String? = nil) {
        guard let sel = selectedEffect else { return }

        // Capture the previous value BEFORE writing so the undo closure
        // restores exactly what was stored (empty string if the key
        // wasn't set yet — the bridge treats "" as "remove").
        let prev = document.effectSettingValue(forKey: key,
                                                inRow: Int32(sel.rowIndex),
                                                at: Int32(sel.effectIndex))
        guard prev != value else { return }

        let shouldSuppress = (suppressIfDefault != nil && value == suppressIfDefault!)
        let changed: Bool
        if shouldSuppress {
            changed = document.removeEffectSetting(forKey: key,
                                                    inRow: Int32(sel.rowIndex),
                                                    at: Int32(sel.effectIndex))
            if changed || selectedEffectSettings[key] != nil {
                selectedEffectSettings.removeValue(forKey: key)
            }
        } else {
            changed = document.setEffectSettingValue(value,
                                                      forKey: key,
                                                      inRow: Int32(sel.rowIndex),
                                                      at: Int32(sel.effectIndex))
            if changed {
                selectedEffectSettings[key] = value
            }
        }

        if changed {
            // Moving Head post-write sync (G3 — C7). The renderer
            // reads position commands out of `MH*_Settings`, not
            // the JSON-backed sliders, so whenever the user edits
            // a position slider we re-assemble every active
            // fixture's command string.
            if Self.isMovingHeadPositionKey(key),
               sel.name == "Moving Head" {
                _ = document.syncMovingHeadPosition(
                    forRow: Int32(sel.rowIndex),
                    at: Int32(sel.effectIndex))
            }

            renderEffectAndTrack(rowIndex: sel.rowIndex, effectIndex: sel.effectIndex)
            // Tell the effects canvas that something on this effect
            // changed so it redraws the selected slot (fade bars,
            // colour pickers reflected in brackets, etc.). The render
            // result lands asynchronously; this invalidation covers the
            // UI-derived visuals that don't need the pixel data.
            inspectorRevision &+= 1
            let rowIndex = sel.rowIndex
            let effectIndex = sel.effectIndex
            undoManager.registerUndo(withTarget: self) { vm in
                vm.setSettingValueAt(rowIndex: rowIndex,
                                     effectIndex: effectIndex,
                                     key: key,
                                     value: prev)
            }
            undoManager.setActionName("Edit \(key)")
        }
    }

    /// Keys the Moving Head bridge sync needs to re-fan into
    /// `MH*_Settings`. Covers the float-slider TEXTCTRL storage,
    /// the int-slider SLIDER storage, and the value-curve sibling
    /// for each of the six position commands.
    private static func isMovingHeadPositionKey(_ key: String) -> Bool {
        let cmds = ["Pan", "Tilt", "PanOffset", "TiltOffset",
                    "Groupings", "Cycles", "PathScale", "TimeOffset"]
        for cmd in cmds {
            if key == "E_TEXTCTRL_MH\(cmd)" { return true }
            if key == "E_SLIDER_MH\(cmd)" { return true }
            if key == "E_VALUECURVE_MH\(cmd)" { return true }
        }
        return false
    }

    // MARK: - Multi-effect bulk edit (G11 / G14)

    /// Rule for whether `key` is safe to propagate from the anchor
    /// effect to `targetName`. Shared prefixes (B_/C_/T_) apply to
    /// every effect; E_-prefixed keys are effect-specific and only
    /// safe when the target is the same effect type or already has
    /// that key in its settings map.
    private func bulkKeyAppliesTo(_ key: String,
                                   target: EffectSelection,
                                   anchorName: String) -> Bool {
        if key.hasPrefix("B_") || key.hasPrefix("C_") || key.hasPrefix("T_") {
            return true
        }
        if key.hasPrefix("E_") {
            if target.name == anchorName { return true }
            // Allow override when the target already has the same
            // key set (carried over from an earlier edit) — otherwise
            // an E_ key from "On" would pollute a "Text" effect.
            let existing = document.effectSettingValue(
                forKey: key,
                inRow: Int32(target.rowIndex),
                at: Int32(target.effectIndex))
            return !existing.isEmpty
        }
        return false
    }

    /// Apply `value` at `key` to every effect in `selectedEffects`
    /// other than the anchor (already has `value`). Skips targets
    /// where the rule above rejects the key. Collects previous values
    /// into a single undo step so Cmd+Z reverts the whole batch.
    /// Called from the per-control "Apply to all selected" context
    /// menu entry (G11).
    func applyValueToAllSelected(_ value: String, forKey key: String) {
        guard let anchor = selectedEffect,
              selectedEffects.count > 1 else { return }

        struct Prev {
            let rowIndex: Int
            let effectIndex: Int
            let key: String
            let value: String
        }
        var prevValues: [Prev] = []
        var changedCount = 0

        for sel in selectedEffects {
            if sel.rowIndex == anchor.rowIndex
                && sel.effectIndex == anchor.effectIndex { continue }
            if !bulkKeyAppliesTo(key, target: sel, anchorName: anchor.name) {
                continue
            }
            let prev = document.effectSettingValue(forKey: key,
                                                    inRow: Int32(sel.rowIndex),
                                                    at: Int32(sel.effectIndex))
            if prev == value { continue }

            let changed = document.setEffectSettingValue(
                value, forKey: key,
                inRow: Int32(sel.rowIndex),
                at: Int32(sel.effectIndex))
            if changed {
                prevValues.append(Prev(rowIndex: sel.rowIndex,
                                        effectIndex: sel.effectIndex,
                                        key: key, value: prev))
                renderEffectAndTrack(rowIndex: sel.rowIndex,
                                     effectIndex: sel.effectIndex)
                changedCount += 1
            }
        }

        if changedCount > 0 {
            inspectorRevision &+= 1
            undoManager.registerUndo(withTarget: self) { vm in
                vm.restoreBulkSettings(prevValues.map {
                    ($0.rowIndex, $0.effectIndex, $0.key, $0.value)
                })
            }
            undoManager.setActionName("Apply \(key) to \(changedCount) Effects")
        }
    }

    /// Copy every inspector value (settings + palette) from the
    /// anchor effect to every other selected effect (G14 — "Update
    /// all like this"). Keys apply through the same
    /// `bulkKeyAppliesTo` filter so E_ keys don't leak across effect
    /// types. One compound undo step.
    func updateAllLikeAnchor() {
        guard let anchor = selectedEffect,
              selectedEffects.count > 1 else { return }

        var anchorValues: [(String, String)] = []
        let settings = document.effectSettings(
            forRow: Int32(anchor.rowIndex),
            at: Int32(anchor.effectIndex))
        for (k, v) in settings { anchorValues.append((k, v)) }
        let palette = document.effectPalette(
            forRow: Int32(anchor.rowIndex),
            at: Int32(anchor.effectIndex))
        for (k, v) in palette { anchorValues.append((k, v)) }

        struct Prev {
            let rowIndex: Int
            let effectIndex: Int
            let key: String
            let value: String
        }
        var prevValues: [Prev] = []
        var affectedTargets = Set<String>()

        for sel in selectedEffects {
            if sel.rowIndex == anchor.rowIndex
                && sel.effectIndex == anchor.effectIndex { continue }
            var anyChange = false
            for (key, value) in anchorValues {
                if !bulkKeyAppliesTo(key, target: sel, anchorName: anchor.name) {
                    continue
                }
                let prev = document.effectSettingValue(forKey: key,
                                                        inRow: Int32(sel.rowIndex),
                                                        at: Int32(sel.effectIndex))
                if prev == value { continue }
                let changed = document.setEffectSettingValue(
                    value, forKey: key,
                    inRow: Int32(sel.rowIndex),
                    at: Int32(sel.effectIndex))
                if changed {
                    prevValues.append(Prev(rowIndex: sel.rowIndex,
                                            effectIndex: sel.effectIndex,
                                            key: key, value: prev))
                    anyChange = true
                }
            }
            if anyChange {
                renderEffectAndTrack(rowIndex: sel.rowIndex,
                                     effectIndex: sel.effectIndex)
                affectedTargets.insert("\(sel.rowIndex):\(sel.effectIndex)")
            }
        }

        if !prevValues.isEmpty {
            inspectorRevision &+= 1
            let rollback = prevValues.map {
                ($0.rowIndex, $0.effectIndex, $0.key, $0.value)
            }
            undoManager.registerUndo(withTarget: self) { vm in
                vm.restoreBulkSettings(rollback)
            }
            undoManager.setActionName("Update \(affectedTargets.count) Effects")
        }
    }

    /// Undo restore for bulk writes — re-applies the captured
    /// previous values as a single compound step, registering a
    /// redo that re-writes the current values back.
    func restoreBulkSettings(_ entries: [(Int, Int, String, String)]) {
        struct Curr {
            let rowIndex: Int
            let effectIndex: Int
            let key: String
            let value: String
        }
        var forwardValues: [Curr] = []
        for (r, i, k, v) in entries {
            let curr = document.effectSettingValue(forKey: k,
                                                    inRow: Int32(r),
                                                    at: Int32(i))
            if curr == v { continue }
            if document.setEffectSettingValue(v, forKey: k,
                                               inRow: Int32(r), at: Int32(i)) {
                forwardValues.append(Curr(rowIndex: r, effectIndex: i,
                                           key: k, value: curr))
                if selectedEffect?.rowIndex == r
                    && selectedEffect?.effectIndex == i {
                    selectedEffectSettings[k] = v
                }
                renderEffectAndTrack(rowIndex: r, effectIndex: i)
            }
        }
        if !forwardValues.isEmpty {
            inspectorRevision &+= 1
            let redo = forwardValues.map {
                ($0.rowIndex, $0.effectIndex, $0.key, $0.value)
            }
            undoManager.registerUndo(withTarget: self) { vm in
                vm.restoreBulkSettings(redo)
            }
        }
    }

    /// Undo-redirect entry: writes a setting on a specific effect by
    /// row/index rather than via the current selection. Used as the
    /// undo target for `setSettingValue` so the change still applies
    /// if the user has deselected between change and undo.
    private func setSettingValueAt(rowIndex: Int, effectIndex: Int,
                                    key: String, value: String) {
        let prev = document.effectSettingValue(forKey: key,
                                                inRow: Int32(rowIndex),
                                                at: Int32(effectIndex))
        guard prev != value else { return }
        let changed = document.setEffectSettingValue(value,
                                                      forKey: key,
                                                      inRow: Int32(rowIndex),
                                                      at: Int32(effectIndex))
        if changed {
            if selectedEffect?.rowIndex == rowIndex
                && selectedEffect?.effectIndex == effectIndex {
                selectedEffectSettings[key] = value
            }
            renderEffectAndTrack(rowIndex: rowIndex, effectIndex: effectIndex)
            inspectorRevision &+= 1
            undoManager.registerUndo(withTarget: self) { vm in
                vm.setSettingValueAt(rowIndex: rowIndex, effectIndex: effectIndex,
                                     key: key, value: prev)
            }
            undoManager.setActionName("Edit \(key)")
        }
    }

    func deleteSelectedEffect() {
        guard let sel = selectedEffect else { return }
        deleteEffect(rowIndex: sel.rowIndex, effectIndex: sel.effectIndex)
    }

    /// Delete every effect in `selectedEffects` as one undo step. Groups
    /// by row and deletes effects in descending `effectIndex` order
    /// within each row so earlier deletions don't shift indices of
    /// later ones. Falls through to the single-select path when the
    /// set is size 0 or 1.
    func deleteSelectedEffects() {
        if selectedEffects.count <= 1 {
            deleteSelectedEffect()
            return
        }
        undoManager.beginUndoGrouping()
        let byRow = Dictionary(grouping: selectedEffects, by: { $0.rowIndex })
        let count = selectedEffects.count
        for (_, effects) in byRow {
            let sortedDesc = effects.sorted { $0.effectIndex > $1.effectIndex }
            for sel in sortedDesc {
                deleteEffect(rowIndex: sel.rowIndex, effectIndex: sel.effectIndex)
            }
        }
        undoManager.endUndoGrouping()
        undoManager.setActionName("Delete \(count) Effects")
        clearSelection()
    }

    /// Lock/unlock every effect in `selectedEffects` (multi-select bulk
    /// op). If the set is mixed (some locked, some not), the bulk
    /// action locks all of them; if all are already locked it unlocks
    /// all.
    func toggleLockSelectedEffects() {
        if selectedEffects.count <= 1 {
            toggleLockSelected()
            return
        }
        let allLocked = selectedEffects.allSatisfy {
            document.effectIsLocked(inRow: Int32($0.rowIndex), at: Int32($0.effectIndex))
        }
        let targetLocked = !allLocked
        undoManager.beginUndoGrouping()
        for sel in selectedEffects {
            let cur = document.effectIsLocked(inRow: Int32(sel.rowIndex), at: Int32(sel.effectIndex))
            if cur != targetLocked {
                toggleLock(rowIndex: sel.rowIndex, effectIndex: sel.effectIndex)
            }
        }
        undoManager.endUndoGrouping()
        undoManager.setActionName(targetLocked ? "Lock Effects" : "Unlock Effects")
    }

    // MARK: - Layer management (B48)

    /// B48: remove every empty layer on the row's element (keeping
    /// the ≥ 1 layer invariant). Returns the count removed so the
    /// caller can flip to a "no unused layers" toast when zero.
    @discardableResult
    func deleteUnusedLayers(rowIndex: Int) -> Int {
        let n = Int(document.deleteUnusedLayers(onElementAt: Int32(rowIndex)))
        if n > 0 { reloadRows() }
        return n
    }

    /// B55: convert effects on the row's element from "Per Preview" /
    /// "Default" / "Single Line" buffer styles to their "Per Model …"
    /// counterparts. `allLayers == true` walks every layer of the
    /// element (model-scope menu); false touches only the row's layer.
    /// Returns the number of effects whose style was rewritten so the
    /// caller can show a result toast / no-op feedback.
    @discardableResult
    func convertEffectsToPerModel(rowIndex: Int, allLayers: Bool) -> Int {
        let n = Int(document.convertEffectsToPerModel(onRow: Int32(rowIndex),
                                                        acrossAllLayers: allLayers))
        if n > 0 {
            // Re-render is kicked off by the bridge; refresh rows
            // so the effect bars repaint with their new buffer style.
            reloadRows()
        }
        return n
    }

    /// B56: collapse identical node / strand effects up to the model
    /// level. Returns count of effects promoted. Only meaningful on
    /// `ModelElement` rows that have strand+node layers — typical
    /// targets are sequences imported with strand-level data.
    @discardableResult
    func promoteNodeEffects(rowIndex: Int) -> Int {
        let n = Int(document.promoteNodeEffects(onRow: Int32(rowIndex)))
        if n > 0 { reloadRows() }
        return n
    }

    /// B47: insert `count` empty layers immediately below the row's
    /// own layer. Returns the number actually inserted.
    @discardableResult
    func insertLayersBelow(rowIndex: Int, count: Int) -> Int {
        guard count > 0 else { return 0 }
        let clamped = min(count, 32)  // sanity cap
        let n = Int(document.insertEffectLayersBelow(at: Int32(rowIndex),
                                                       count: Int32(clamped)))
        if n > 0 { reloadRows() }
        return n
    }

    // MARK: - Description (B20)

    /// B20: read the `X_Effect_Description` value off the effect
    /// at `(row, idx)`. Empty string when no description is set.
    func effectDescription(rowIndex: Int, effectIndex: Int) -> String {
        return document.effectDescription(forRow: Int32(rowIndex),
                                           atIndex: Int32(effectIndex))
    }

    /// B20: update an effect's description. Empty string clears.
    /// Undo-able; tracks the previous value and restores it.
    func setEffectDescription(rowIndex: Int, effectIndex: Int,
                               _ description: String) {
        let before = effectDescription(rowIndex: rowIndex, effectIndex: effectIndex)
        if before == description { return }
        _ = document.setEffectDescription(description,
                                            forRow: Int32(rowIndex),
                                            atIndex: Int32(effectIndex))
        undoManager.registerUndo(withTarget: self) { vm in
            vm.setEffectDescription(rowIndex: rowIndex,
                                     effectIndex: effectIndex,
                                     before)
        }
        undoManager.setActionName("Edit Description")
    }

    // MARK: - Effect presets (B19, session-only)

    /// Snapshot of an effect's type + settings + palette. Session-
    /// only for now — persistence is Phase C work. Identity is
    /// `name` (preset name chosen by the user).
    struct EffectPreset: Identifiable, Hashable {
        let id: UUID
        var name: String
        let effectName: String
        let settings: String
        let palette: String
    }

    /// Session-only preset store. Cleared on app launch; a future
    /// Phase C `EffectPresetManager` will persist to disk. Exposed
    /// for the effect context menu to list + apply.
    var presets: [EffectPreset] = []

    /// Capture the selected effect's current settings as a named
    /// preset. No-ops if nothing is selected or the name is blank.
    @discardableResult
    func saveSelectedEffectAsPreset(name: String) -> Bool {
        let trimmed = name.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !trimmed.isEmpty else { return false }
        guard let sel = selectedEffect else { return false }
        let row = Int32(sel.rowIndex)
        let idx = Int32(sel.effectIndex)
        let effectName = document.effectName(forRow: row, at: idx)
        let settings = document.effectSettingsString(forRow: row, at: idx)
        let palette = document.effectPaletteString(forRow: row, at: idx)
        let preset = EffectPreset(id: UUID(), name: trimmed,
                                    effectName: effectName,
                                    settings: settings, palette: palette)
        // Replace an existing preset with the same name — matches
        // desktop's overwrite-on-save behaviour.
        if let existing = presets.firstIndex(where: { $0.name == trimmed }) {
            presets[existing] = preset
        } else {
            presets.append(preset)
        }
        return true
    }

    /// Apply `preset` to every selected effect. If the preset's
    /// effect type matches the target's type the settings + palette
    /// replace the target's wholesale; otherwise the effect is
    /// deleted and a new one of the preset's type is added at the
    /// same time range. Registered as a single undo step.
    @discardableResult
    func applyPreset(_ preset: EffectPreset) -> Bool {
        let targets: [EffectSelection]
        if selectedEffects.count > 1 {
            targets = Array(selectedEffects)
        } else if let one = selectedEffect {
            targets = [one]
        } else { return false }
        undoManager.beginUndoGrouping()
        for sel in targets {
            let row = Int32(sel.rowIndex)
            let idx = Int32(sel.effectIndex)
            let currentName = document.effectName(forRow: row, at: idx)
            if currentName == preset.effectName {
                applySettingsTransform(selection: sel) { _, _ in
                    Self.parseSettingsString(preset.settings)
                }
                // applySettingsTransform re-uses the target's existing
                // palette; override it by calling replaceEffectSettings
                // once more with the preset's palette so colour
                // customisations travel with the preset.
                _ = document.replaceEffectSettings(preset.settings,
                                                     palette: preset.palette,
                                                     inRow: row, atIndex: idx)
            } else {
                // Different effect type — delete + re-add. Preserves
                // [startMS, endMS] so the replacement sits in the
                // same slot.
                let startMS = sel.startTimeMS
                let endMS = sel.endTimeMS
                deleteEffect(rowIndex: sel.rowIndex, effectIndex: sel.effectIndex)
                _ = document.addEffect(toRow: row, name: preset.effectName,
                                         settings: preset.settings,
                                         palette: preset.palette,
                                         startMS: Int32(startMS),
                                         endMS: Int32(endMS))
            }
        }
        undoManager.endUndoGrouping()
        undoManager.setActionName("Apply Preset \(preset.name)")
        refreshSelectedEffectSettings()
        reloadRows()
        return true
    }

    /// Remove a preset from the session store. The UI's "Manage
    /// Presets…" sheet will drive this once we have one; exposed
    /// now so tests can prune.
    func deletePreset(_ preset: EffectPreset) {
        presets.removeAll { $0.id == preset.id }
    }

    // MARK: - Randomize / Reset (B15)

    /// B15: reset every `E_*` setting on the selected effect(s) back
    /// to type defaults. Preserves the palette (C_*) and shared
    /// Buffer / Blending settings (B_* / T_*) so Fade, blending mode,
    /// and colour choices survive. Registered as a single undo step.
    func resetSelectedEffectsToDefaults() {
        let targets: [EffectSelection]
        if selectedEffects.count > 1 {
            targets = Array(selectedEffects)
        } else if let one = selectedEffect {
            targets = [one]
        } else { return }
        undoManager.beginUndoGrouping()
        for sel in targets {
            applySettingsTransform(selection: sel) { current, _ in
                var out = current
                out.removeAll { $0.key.hasPrefix("E_") }
                return out
            }
        }
        undoManager.endUndoGrouping()
        undoManager.setActionName(targets.count > 1
                                   ? "Reset \(targets.count) Effects"
                                   : "Reset Effect")
        refreshSelectedEffectSettings()
    }

    /// B15: randomise every `E_*` setting on each selected effect by
    /// walking the effect's metadata and picking a random value in
    /// range per control type. Sliders / spinners pick an int in
    /// [min, max]; choices pick a random option; checkboxes flip a
    /// 50/50 coin. Each selected effect gets independent random
    /// values. Preserves palette + shared panels.
    func randomizeSelectedEffects() {
        let targets: [EffectSelection]
        if selectedEffects.count > 1 {
            targets = Array(selectedEffects)
        } else if let one = selectedEffect {
            targets = [one]
        } else { return }
        undoManager.beginUndoGrouping()
        for sel in targets {
            applySettingsTransform(selection: sel) { [self] current, effectName in
                let metadata = loadEffectMetadata(effectName)
                return randomizedSettings(current: current, metadata: metadata)
            }
        }
        undoManager.endUndoGrouping()
        undoManager.setActionName(targets.count > 1
                                   ? "Randomise \(targets.count) Effects"
                                   : "Randomise Effect")
        refreshSelectedEffectSettings()
    }

    /// Helper: snapshot current settings, let `transform` produce a
    /// new list, write it back via the bridge, register an undo that
    /// restores the snapshot. `transform` receives the parsed current
    /// settings and the effect name; returns the new settings.
    private func applySettingsTransform(
        selection sel: EffectSelection,
        transform: (_ current: [(key: String, value: String)],
                     _ effectName: String) -> [(key: String, value: String)]
    ) {
        let row = Int32(sel.rowIndex)
        let idx = Int32(sel.effectIndex)
        let beforeSettings = document.effectSettingsString(forRow: row, at: idx)
        let beforePalette  = document.effectPaletteString(forRow: row, at: idx)
        let effectName = document.effectName(forRow: row, at: idx)

        let parsed = Self.parseSettingsString(beforeSettings)
        let newPairs = transform(parsed, effectName)
        let afterSettings = Self.encodeSettingsString(newPairs)

        guard document.replaceEffectSettings(afterSettings,
                                              palette: beforePalette,
                                              inRow: row, atIndex: idx) else {
            return
        }
        undoManager.registerUndo(withTarget: self) { vm in
            _ = vm.document.replaceEffectSettings(beforeSettings,
                                                    palette: beforePalette,
                                                    inRow: row, atIndex: idx)
            vm.refreshSelectedEffectSettings()
        }
    }

    /// Walk `metadata` and produce a new settings list with random
    /// values for every `E_*` property. Entries outside the effect's
    /// metadata (B_ / T_ / X_) are preserved from `current`. When
    /// `metadata` is nil, the input is returned unchanged so the
    /// caller's undo / snapshot logic still exercises the same code
    /// path.
    private func randomizedSettings(
        current: [(key: String, value: String)],
        metadata: EffectMetadata?
    ) -> [(key: String, value: String)] {
        var out = current
        // Drop every existing E_* entry; we'll rebuild from metadata.
        out.removeAll { $0.key.hasPrefix("E_") }
        guard let md = metadata else { return out }
        let prefix = md.settingKeyPrefix
        // Only randomise E-prefixed panels; shared panels have B_/T_/C_.
        guard prefix == "E_" else { return out }
        let props = md.properties ?? []
        for prop in props {
            guard prop.isForIPad else { continue }
            let key = prop.settingKey(prefix: prefix)
            guard let randomValue = randomValueFor(property: prop) else { continue }
            out.append((key: key, value: randomValue))
        }
        return out
    }

    /// Pick a single random value appropriate for `prop.controlType`.
    /// Returns nil for control types we don't randomise (custom
    /// canvases, file pickers, etc.) — those are left at their
    /// current / default value.
    private func randomValueFor(property prop: PropertyMetadata) -> String? {
        switch prop.controlType {
        case "slider", "spin":
            // Slider/spin ranges are inclusive integer ticks. Float
            // sliders still store the int tick value — the divisor
            // is applied at display time, so we don't need to know it.
            let lo = Int((prop.min ?? 0).rounded())
            let hi = Int((prop.max ?? 100).rounded())
            if hi <= lo { return "\(lo)" }
            return "\(Int.random(in: lo...hi))"
        case "checkbox", "togglebutton":
            return Bool.random() ? "1" : "0"
        case "choice", "combobox":
            let opts = prop.options ?? []
            guard !opts.isEmpty else { return nil }
            return opts.randomElement()
        default:
            return nil
        }
    }

    /// Parse the `key=value,key=value` settings string that
    /// `Effect::GetSettingsAsString` emits. Values are un-escaped
    /// (`&amp;` → `&`, `&comma;` → `,`) so callers see the raw
    /// stored strings.
    static func parseSettingsString(_ s: String) -> [(key: String, value: String)] {
        guard !s.isEmpty else { return [] }
        var out: [(key: String, value: String)] = []
        for pair in s.split(separator: ",", omittingEmptySubsequences: true) {
            guard let eq = pair.firstIndex(of: "=") else { continue }
            let k = String(pair[..<eq])
            var v = String(pair[pair.index(after: eq)...])
            v = v.replacingOccurrences(of: "&comma;", with: ",")
            v = v.replacingOccurrences(of: "&amp;", with: "&")
            out.append((key: k, value: v))
        }
        return out
    }

    /// Inverse of `parseSettingsString`: escapes commas + ampersands
    /// in values so the resulting string round-trips cleanly through
    /// `SettingsMap::Parse`.
    static func encodeSettingsString(_ pairs: [(key: String, value: String)]) -> String {
        var parts: [String] = []
        for (k, rawV) in pairs {
            var v = rawV
            v = v.replacingOccurrences(of: "&", with: "&amp;")
            v = v.replacingOccurrences(of: ",", with: "&comma;")
            parts.append("\(k)=\(v)")
        }
        return parts.joined(separator: ",")
    }

    // MARK: - Timing tracks & marks (B67 / B69 / B73)

    /// B81: whether every timing track is currently hidden.
    var allTimingTracksHidden: Bool {
        document.allTimingTracksHidden()
    }

    /// B81: toggle the visibility of every timing track at once.
    /// Desktop equivalent: right-click any timing row →
    /// "Hide All Timing Tracks" / "Show All Timing Tracks".
    func setAllTimingTracksHidden(_ hidden: Bool) {
        document.setAllTimingTracksHidden(hidden)
        reloadRows()
    }

    /// B82: ensure every visible timing track appears in every
    /// non-master view. Returns the number of timing-tracks added.
    @discardableResult
    func addAllTimingTracksToAllViews() -> Int {
        let n = Int(document.addAllTimingTracksToAllViews())
        if n > 0 { reloadRows() }
        return n
    }

    /// AI-driven lyric timing track. Fires off SFSpeech (or whichever
    /// service is configured) on a background queue, prefers the
    /// HTDemucs vocals stem when it's been computed, and on success
    /// creates a populated timing track named "AutoGen" (uniquified
    /// by the bridge if there's a collision).
    ///
    /// `serviceName` picks which SPEECH2TEXT-capable service to use.
    /// Pass `nil` to auto-select the first available one (the
    /// caller's UI is responsible for prompting when multiple are
    /// configured — we don't want a silent default to surprise
    /// users).
    ///
    /// Always calls `completion` on the main queue with `nil` on
    /// success or an error string on failure. The view-model side
    /// handles the row reload + undo registration so the sheet just
    /// surfaces UI state.
    func generateAILyricTrack(name: String = "AutoGen",
                                serviceName: String?,
                                completion: @escaping @Sendable @MainActor (String?) -> Void) {
        let speechServices = XLAIServices.shared().allServices()
            .filter { $0.available && $0.capabilities.contains(XLAICapabilitySpeech2Text) }
        let service: XLAIServiceInfo?
        if let name = serviceName, !name.isEmpty {
            service = speechServices.first(where: { $0.name == name })
        } else {
            service = speechServices.first
        }
        guard let service = service else {
            completion("No speech-to-text AI service is configured. Open Tools → AI Services… and enable one.")
            return
        }

        let doc = self.document

        // Use whichever audio track the user currently has selected
        // in the waveform display. -1 = main sequence audio;
        // 0..altTrackCount-1 = an attached alt track. That lets the
        // user point the recogniser at a Moises-generated vocals
        // stem, an HTDemucs export, or whatever else they've added
        // under Settings → Audio Tracks. Beats us second-guessing
        // which track is cleanest.
        let active = doc.activeWaveformTrack()
        let altPath: String = active >= 0 ? doc.altTrackPath(at: Int(active)) : ""
        let audioPath: String = !altPath.isEmpty
            ? altPath
            : (doc.sequenceAudioFilePath() ?? "")
        guard !audioPath.isEmpty else {
            completion("No audio file is loaded for this sequence.")
            return
        }

        XLAIServices.shared().generateLyricTrack(audioPath: audioPath,
                                                   forService: service.name) { words, starts, ends, error in

            // The bridge dispatches every completion path to the
            // main queue (XLAIServices.mm:371-418). Convert the
            // bridged NSArrays into Sendable Swift value types
            // *before* entering the MainActor isolation so Swift 6
            // strict concurrency doesn't flag the closure capture
            // as a data-race risk. NSArray<NSNumber> isn't Sendable
            // under strict mode; [Int] / [String] are.
            let safeWords: [String]? = words
            let safeStarts: [Int]? = starts?.map { $0.intValue }
            let safeEnds: [Int]? = ends?.map { $0.intValue }
            let safeError: String? = error

            MainActor.assumeIsolated {
                if let error = safeError {
                    completion(error)
                    return
                }
                guard let words = safeWords, let starts = safeStarts, let ends = safeEnds,
                      !words.isEmpty else {
                    completion("Recognizer returned no words. Try again with the vocals stem (Tools → Stem Separation) or a clearer audio source.")
                    return
                }

                let trackName = doc.addLyricTimingTrack(
                    named: name,
                    words: words,
                    startMS: starts.map { NSNumber(value: $0) },
                    endMS: ends.map { NSNumber(value: $0) })
                if trackName.isEmpty {
                    completion("Couldn't create the lyric timing track.")
                    return
                }

                self.reloadRows()
                self.undoManager.registerUndo(withTarget: self) { vm in
                    // Find and delete the freshly-added track on undo.
                    let tIdx = (vm.document.timingRowIndices() as [NSNumber]).map { $0.intValue }
                    for i in tIdx {
                        if i < vm.rows.count, vm.rows[i].timing?.elementName == trackName {
                            _ = vm.document.deleteTimingTrack(at: Int32(i))
                            vm.reloadRows()
                            return
                        }
                    }
                }
                self.undoManager.setActionName("Generate AI Lyrics")
                completion(nil)
            }
        }
    }

    /// Fixed-interval timing track (Empty when intervalMS == 0,
    /// otherwise marks every `intervalMS` from 0 to sequence end).
    /// Reloads rows + registers undo. Returns the actual track name
    /// the bridge committed (may be uniquified on name collision)
    /// or nil if nothing was added.
    @discardableResult
    func addFixedIntervalTimingTrack(name: String, intervalMS: Int) -> String? {
        let trimmed = name.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !trimmed.isEmpty else { return nil }
        let actual = document.addFixedIntervalTimingTrack(named: trimmed,
                                                            intervalMS: Int32(intervalMS))
        guard !actual.isEmpty else { return nil }
        reloadRows()
        registerUndoForAddedTimingTrack(name: actual,
                                         actionName: intervalMS > 0 ? "Add Fixed Timing Track" : "Add Timing Track")
        return actual
    }

    /// Metronome timing track. Marks are placed every `intervalMS`
    /// (or randomised in `[minIntervalMS, intervalMS]` when
    /// `minIntervalMS > 0`); each gets a label cycled from `tags`
    /// (or "1"–"10" when tags is empty), randomised when `randomize`
    /// is true.
    @discardableResult
    func addMetronomeTimingTrack(name: String,
                                  intervalMS: Int,
                                  tags: [String] = [],
                                  minIntervalMS: Int = -1,
                                  randomize: Bool = false) -> String? {
        let trimmed = name.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !trimmed.isEmpty, intervalMS > 0 else { return nil }
        let actual = document.addMetronomeTimingTrack(named: trimmed,
                                                        intervalMS: Int32(intervalMS),
                                                        tags: tags,
                                                        minIntervalMS: Int32(minIntervalMS),
                                                        randomize: randomize)
        guard !actual.isEmpty else { return nil }
        reloadRows()
        registerUndoForAddedTimingTrack(name: actual, actionName: "Add Metronome")
        return actual
    }

    /// FPP Commands / FPP Effects timing track. `subType` must be
    /// "FPP Commands" or "FPP Effects" — anything else is rejected
    /// by the bridge.
    @discardableResult
    func addFPPTimingTrack(name: String, subType: String) -> String? {
        let trimmed = name.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !trimmed.isEmpty else { return nil }
        let actual = document.addFPPTimingTrack(named: trimmed, subType: subType)
        guard !actual.isEmpty else { return nil }
        reloadRows()
        registerUndoForAddedTimingTrack(name: actual, actionName: "Add FPP Timing")
        return actual
    }

    /// Shared undo registration for the "added a timing track named
    /// X" pattern. The undo action looks the track up by name and
    /// deletes it. Names are uniquified by the bridge on creation
    /// so this reverses reliably even with multiple tracks of the
    /// same base name.
    private func registerUndoForAddedTimingTrack(name: String, actionName: String) {
        undoManager.registerUndo(withTarget: self) { vm in
            let tIdx = (vm.document.timingRowIndices() as [NSNumber]).map { $0.intValue }
            for i in tIdx {
                if i < vm.rows.count, vm.rows[i].timing?.elementName == name {
                    _ = vm.document.deleteTimingTrack(at: Int32(i))
                    vm.reloadRows()
                    return
                }
            }
        }
        undoManager.setActionName(actionName)
    }

    /// B73: add a new variable timing track. On success the new track
    /// is made active and the rows are reloaded. Undo-able.
    @discardableResult
    func addTimingTrack(name: String) -> Bool {
        let trimmed = name.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !trimmed.isEmpty else { return false }
        if !document.addTimingTrackNamed(trimmed) { return false }
        reloadRows()
        undoManager.registerUndo(withTarget: self) { vm in
            // On undo, find the most-recently-added timing track by
            // its name and delete it. Names are uniquified by the
            // bridge on creation, so this reverses reliably.
            let idxSet: Set<Int> = Set(
                (vm.document.timingRowIndices() as [NSNumber]).map { $0.intValue }
            )
            for i in idxSet {
                if i < vm.rows.count, vm.rows[i].timing?.elementName == trimmed {
                    _ = vm.document.deleteTimingTrack(at: Int32(i))
                    vm.reloadRows()
                    return
                }
            }
        }
        undoManager.setActionName("Add Timing Track")
        return true
    }

    /// B67: add a timing mark to the given timing row. `startMS` /
    /// `endMS` must be a non-zero range that doesn't overlap an
    /// existing mark on the same layer. Returns true on success.
    @discardableResult
    func addTimingMark(rowIndex: Int, startMS: Int, endMS: Int, label: String = "") -> Bool {
        guard rowIndex >= 0, rowIndex < rows.count else { return false }
        guard rows[rowIndex].timing != nil else { return false }
        let idx = Int(document.addTimingMark(atRow: Int32(rowIndex),
                                              startMS: Int32(startMS),
                                              endMS: Int32(endMS),
                                              label: label))
        if idx < 0 { return false }
        reloadRows()
        undoManager.registerUndo(withTarget: self) { vm in
            vm.deleteTimingMark(rowIndex: rowIndex, markIndex: idx)
        }
        undoManager.setActionName("Add Timing Mark")
        return true
    }

    /// B71: split a timing mark at `atMS`. The left half keeps the
    /// original label and runs `[origStart, atMS]`; the right half
    /// is a fresh mark with an empty label running `[atMS, origEnd]`.
    /// No-op if `atMS` isn't strictly inside the mark. Single undo
    /// group rolls up the delete + two adds.
    @discardableResult
    func splitTimingMark(rowIndex: Int, markIndex: Int, atMS: Int) -> Bool {
        guard rowIndex >= 0, rowIndex < rows.count else { return false }
        let row = rows[rowIndex]
        guard row.timing != nil,
              markIndex >= 0, markIndex < row.effects.count else { return false }
        let mark = row.effects[markIndex]
        guard atMS > mark.startTimeMS, atMS < mark.endTimeMS else { return false }
        let origStart = mark.startTimeMS
        let origEnd = mark.endTimeMS
        let origLabel = mark.name
        undoManager.beginUndoGrouping()
        _ = deleteTimingMark(rowIndex: rowIndex, markIndex: markIndex)
        _ = addTimingMark(rowIndex: rowIndex,
                           startMS: origStart, endMS: atMS, label: origLabel)
        _ = addTimingMark(rowIndex: rowIndex,
                           startMS: atMS, endMS: origEnd, label: "")
        undoManager.endUndoGrouping()
        undoManager.setActionName("Split Timing Mark")
        return true
    }

    /// B72: merge a timing mark with its right-neighbor on the same
    /// row. The merged mark spans `[leftStart, rightEnd]` and its
    /// label joins the two labels with a space (or the left label
    /// alone if the right was empty). Leaves a gap between the
    /// original marks intact (merging across a gap is desktop
    /// behavior too — they just stitch together).
    @discardableResult
    func mergeTimingMarkWithNext(rowIndex: Int, markIndex: Int) -> Bool {
        guard rowIndex >= 0, rowIndex < rows.count else { return false }
        let row = rows[rowIndex]
        guard row.timing != nil,
              markIndex >= 0, markIndex + 1 < row.effects.count else { return false }
        let left = row.effects[markIndex]
        let right = row.effects[markIndex + 1]
        let newStart = left.startTimeMS
        let newEnd = right.endTimeMS
        let newLabel: String
        if left.name.isEmpty { newLabel = right.name }
        else if right.name.isEmpty { newLabel = left.name }
        else { newLabel = "\(left.name) \(right.name)" }
        undoManager.beginUndoGrouping()
        // Delete right first (higher index) so the left index stays
        // stable for the next delete.
        _ = deleteTimingMark(rowIndex: rowIndex, markIndex: markIndex + 1)
        _ = deleteTimingMark(rowIndex: rowIndex, markIndex: markIndex)
        _ = addTimingMark(rowIndex: rowIndex,
                           startMS: newStart, endMS: newEnd, label: newLabel)
        undoManager.endUndoGrouping()
        undoManager.setActionName("Merge Timing Marks")
        return true
    }

    /// B80: subdivision modes — positive values split each source
    /// mark into N equal sub-marks; negative values combine every
    /// |N| consecutive source marks into one.
    enum SubdivisionMode: Int {
        case half = 2, third = 3, quarter = 4, sixth = 6, eighth = 8
        case combine2 = -2, combine4 = -4, combine8 = -8
        var suffix: String {
            switch self {
            case .half:     return " - 1/2"
            case .third:    return " - 1/3"
            case .quarter:  return " - 1/4"
            case .sixth:    return " - 1/6"
            case .eighth:   return " - 1/8"
            case .combine2: return " - 2x"
            case .combine4: return " - 4x"
            case .combine8: return " - 8x"
            }
        }
    }

    /// B80: for each source mark, subdivide / combine and populate
    /// a new timing track whose name is `sourceName + suffix`.
    /// Collides silently (returns false) if the target name already
    /// exists — matches desktop's "Skipping" behaviour which is
    /// friendlier than the user losing work.
    @discardableResult
    func generateSubdividedTimingTrack(sourceRowIndex: Int,
                                        mode: SubdivisionMode) -> Bool {
        guard sourceRowIndex >= 0, sourceRowIndex < rows.count else { return false }
        let src = rows[sourceRowIndex]
        guard let t = src.timing, !src.effects.isEmpty else { return false }
        let newName = t.elementName + mode.suffix
        // Build planned marks in Swift first so we can bail
        // without mutating on collision / empty result.
        var plannedMarks: [(Int, Int)] = []
        let divisor = mode.rawValue
        for (j, m) in src.effects.enumerated() {
            let start = m.startTimeMS
            let end = m.endTimeMS
            let dur = end - start
            if divisor > 0 {
                let sub = Double(dur) / Double(divisor)
                for k in 0..<divisor {
                    let s = start + Int(sub * Double(k))
                    let e = (k == divisor - 1) ? end
                                                : start + Int(sub * Double(k + 1))
                    if e > s { plannedMarks.append((s, e)) }
                }
            } else {
                let mul = -divisor
                if j % mul != 0 { continue }
                var groupEnd = end
                for extra in 1..<mul where j + extra < src.effects.count {
                    groupEnd = src.effects[j + extra].endTimeMS
                }
                plannedMarks.append((start, groupEnd))
            }
        }
        if plannedMarks.isEmpty { return false }
        // Add the new timing track.
        if !document.addTimingTrackNamed(newName) { return false }
        reloadRows()
        // Find the just-added row by name (bridge uniquifies on
        // collision but the addTimingTrack call would have failed
        // at our pre-check if there was a user-visible collision).
        guard let newRowIdx = rows.firstIndex(where: {
            $0.timing?.elementName == newName
        }) else { return false }
        for (s, e) in plannedMarks {
            _ = addTimingMark(rowIndex: newRowIdx,
                               startMS: s, endMS: e, label: "")
        }
        return true
    }

    /// B89: label every mark on the given timing row with an
    /// incrementing integer in `[startNum, endNum]`, wrapping back
    /// to `startNum` when it rolls past `endNum`. `overwrite=false`
    /// preserves any mark that already has a non-empty label
    /// (matches desktop `EffectsGrid::AUTOLABEL`). Undo is wired
    /// via the existing per-mark `renameTimingMark` path.
    @discardableResult
    func autoLabelTimingMarks(rowIndex: Int, startNum: Int, endNum: Int,
                               overwrite: Bool) -> Int {
        guard rowIndex >= 0, rowIndex < rows.count else { return 0 }
        let row = rows[rowIndex]
        guard row.timing != nil else { return 0 }
        if row.effects.isEmpty { return 0 }
        let increment = startNum <= endNum ? 1 : -1
        var current = startNum
        undoManager.beginUndoGrouping()
        var count = 0
        for (idx, mark) in row.effects.enumerated() {
            if !overwrite && !mark.name.isEmpty { continue }
            _ = renameTimingMark(rowIndex: rowIndex, markIndex: idx,
                                  label: "\(current)")
            count += 1
            current += increment
            if increment == 1 && current > endNum {
                current = startNum
            } else if increment == -1 && current < endNum {
                current = startNum
            }
        }
        undoManager.endUndoGrouping()
        undoManager.setActionName("Auto-Label Marks")
        return count
    }

    /// B78: populate a timing row with phrase marks from raw lyrics
    /// text (one line per phrase). Replaces all existing layers on
    /// the target row. `startMS`/`endMS` default to the full sequence
    /// range if zero. Returns the number of marks added.
    @discardableResult
    func importLyrics(rowIndex: Int, lyrics: String,
                       startMS: Int = 0, endMS: Int = 0) -> Int {
        let lines = lyrics.components(separatedBy: .newlines)
        let added = Int(document.importLyrics(atRow: Int32(rowIndex),
                                                phrases: lines,
                                                startMS: Int32(startMS),
                                                endMS: Int32(endMS)))
        if added > 0 { reloadRows() }
        return added
    }

    // MARK: - xtiming I/O (B74 / B75)

    /// B74: import an `.xtiming` file. Returns the number of tracks
    /// added (0 on parse failure). Caller must ensure security-scoped
    /// access to `path` before calling.
    @discardableResult
    func importXTiming(path: String) -> Int {
        let added = Int(document.importXTiming(fromPath: path))
        if added > 0 { reloadRows() }
        return added
    }

    /// LOR `.lms` timing import — Phase E follow-up. Same return
    /// semantics as `importXTiming(path:)`.
    @discardableResult
    func importLorTiming(path: String) -> Int {
        let added = Int(document.importLorTiming(fromPath: path))
        if added > 0 { reloadRows() }
        return added
    }

    /// Papagayo `.pgo` lyric-sync import — Phase E follow-up.
    @discardableResult
    func importPapagayoTiming(path: String) -> Int {
        let added = Int(document.importPapagayoTiming(fromPath: path))
        if added > 0 { reloadRows() }
        return added
    }

    /// B75: export the given timing row to `path` as `.xtiming`.
    @discardableResult
    func exportTimingTrack(rowIndex: Int, path: String) -> Bool {
        return document.exportTimingTrack(atRow: Int32(rowIndex), toPath: path)
    }

    /// B49: export the model at `rowIndex` as a Falcon Player v2
    /// compressed sub-sequence (`.eseq`). When `startMS`/`endMS` is
    /// nil, the whole sequence is exported; when both are provided,
    /// only that [startMS, endMS] window is written.
    @discardableResult
    func exportModelAsFSEQ(rowIndex: Int, path: String,
                           startMS: Int? = nil, endMS: Int? = nil) -> Bool {
        return document.exportModelAsFSEQ(
            atRow: Int32(rowIndex),
            toPath: path,
            startMS: Int32(startMS ?? -1),
            endMS: Int32(endMS ?? -1))
    }

    /// B76: convert a fixed-interval timing track to variable
    /// (user-editable). Existing marks are kept in place; only the
    /// fixed-period flag is cleared. Not undo-able for first cut.
    @discardableResult
    func makeTimingTrackVariable(rowIndex: Int) -> Bool {
        guard rowIndex >= 0, rowIndex < rows.count else { return false }
        if !document.makeTimingTrackVariable(atRow: Int32(rowIndex)) {
            return false
        }
        reloadRows()
        return true
    }

    func timingTrackIsFixed(rowIndex: Int) -> Bool {
        guard rowIndex >= 0, rowIndex < rows.count else { return false }
        return document.timingTrackIsFixed(atRow: Int32(rowIndex))
    }

    /// B84: break every phrase mark on the given timing row into
    /// per-word sub-marks on layer 1. The timing element's existing
    /// word + phoneme layers are discarded first (matches desktop).
    /// Not currently undo-able: the op mutates layer structure and
    /// our undo plumbing only registers per-mark adds/deletes —
    /// adding layer-level undo is follow-up work.
    @discardableResult
    func breakdownPhrases(rowIndex: Int) -> Bool {
        guard rowIndex >= 0, rowIndex < rows.count else { return false }
        guard rows[rowIndex].timing != nil else { return false }
        if !document.breakdownPhrases(atRow: Int32(rowIndex)) { return false }
        reloadRows()
        return true
    }

    /// B85: break every word mark on layer 1 of the target timing
    /// element into per-phoneme sub-marks on layer 2. Uses the
    /// bundled `PhonemeDictionary` via the iPadRenderContext. Gated
    /// by `canBreakdownWords` (requires a Words layer). Not
    /// undo-able for first cut (mutates layer structure).
    @discardableResult
    func breakdownWords(rowIndex: Int) -> Bool {
        guard rowIndex >= 0, rowIndex < rows.count else { return false }
        guard rows[rowIndex].timing != nil else { return false }
        if !document.breakdownWords(atRow: Int32(rowIndex)) { return false }
        reloadRows()
        return true
    }

    /// True iff the given timing row is layer 0 of an element with
    /// at least a Words layer (i.e. BreakdownPhrases already ran).
    func canBreakdownWords(rowIndex: Int) -> Bool {
        guard rowIndex >= 0, rowIndex < rows.count else { return false }
        let row = rows[rowIndex]
        guard row.timing != nil, row.layerIndex == 0 else { return false }
        return Int(document.rowLayerCount(at: Int32(rowIndex))) >= 2
    }

    /// Returns true if the given timing row is the phrase layer
    /// (layer 0) of an element that has at least one mark with a
    /// non-empty label — gate for the "Breakdown Phrases" menu.
    func canBreakdownPhrases(rowIndex: Int) -> Bool {
        guard rowIndex >= 0, rowIndex < rows.count else { return false }
        let row = rows[rowIndex]
        guard row.timing != nil, row.layerIndex == 0 else { return false }
        return row.effects.contains(where: { !$0.name.isEmpty })
    }

    /// B84 (per-mark): break a single phrase mark into per-word
    /// sub-marks, leaving sibling phrases on the row alone. Gated by
    /// `canBreakdownPhrase(rowIndex:markIndex:)`.
    @discardableResult
    func breakdownPhrase(rowIndex: Int, markIndex: Int) -> Bool {
        guard rowIndex >= 0, rowIndex < rows.count else { return false }
        guard rows[rowIndex].timing != nil else { return false }
        if !document.breakdownPhrase(atRow: Int32(rowIndex),
                                       atIndex: Int32(markIndex)) { return false }
        reloadRows()
        return true
    }

    /// True iff the mark is on the phrase layer and has a non-empty
    /// label — gates the per-mark "Breakdown This Phrase" menu entry.
    func canBreakdownPhrase(rowIndex: Int, markIndex: Int) -> Bool {
        guard rowIndex >= 0, rowIndex < rows.count else { return false }
        let row = rows[rowIndex]
        guard row.timing != nil, row.layerIndex == 0 else { return false }
        guard markIndex >= 0, markIndex < row.effects.count else { return false }
        return !row.effects[markIndex].name.isEmpty
    }

    // MARK: - Find / Replace (B97)

    /// Recompute `findResults` against `findText` — searches every
    /// timing-row mark's `name`. Mirrors desktop's intentional
    /// timing-tracks-only restriction (`EffectsGrid::Find`).
    /// Resets `currentFindIndex` so the next `findNext()` selects
    /// the first match.
    func runFind() {
        guard !findText.isEmpty else {
            findResults = []
            currentFindIndex = -1
            return
        }
        let needle = findCaseSensitive ? findText : findText.lowercased()
        var matches: [FindMatch] = []
        for (rowIdx, row) in rows.enumerated() {
            guard row.timing != nil else { continue }
            for (mIdx, eff) in row.effects.enumerated() {
                if eff.name.isEmpty { continue }
                let hay = findCaseSensitive ? eff.name : eff.name.lowercased()
                if hay.contains(needle) {
                    matches.append(FindMatch(rowIndex: rowIdx, markIndex: mIdx))
                }
            }
        }
        findResults = matches
        currentFindIndex = -1
    }

    /// Advance `currentFindIndex` and select + seek to that match.
    /// No-op when there are no results. Wraps at the end.
    @discardableResult
    func findNext() -> Bool {
        guard !findResults.isEmpty else { return false }
        currentFindIndex = (currentFindIndex + 1) % findResults.count
        focusFindMatch()
        return true
    }

    @discardableResult
    func findPrevious() -> Bool {
        guard !findResults.isEmpty else { return false }
        currentFindIndex = (currentFindIndex - 1 + findResults.count) % findResults.count
        focusFindMatch()
        return true
    }

    /// Replace every current match's label with `replacement`. Returns
    /// the count actually changed. Bridge calls re-walk the timing
    /// element after each, so we re-run the search at the end to
    /// drain stale entries (e.g. when the replacement no longer
    /// matches `findText`).
    @discardableResult
    func replaceAllFindMatches(with replacement: String) -> Int {
        guard !findResults.isEmpty else { return 0 }
        var n = 0
        for match in findResults {
            if document.setTimingMarkLabel(atRow: Int32(match.rowIndex),
                                            at: Int32(match.markIndex),
                                            label: replacement) {
                n += 1
            }
        }
        if n > 0 {
            reloadRows()
            // Re-run the search since renamed marks may no longer match.
            runFind()
        }
        return n
    }

    /// Replace just the currently-focused match. Advances to the
    /// next match after the replace so the panel feels live.
    @discardableResult
    func replaceCurrentFindMatch(with replacement: String) -> Bool {
        guard currentFindIndex >= 0, currentFindIndex < findResults.count else { return false }
        let match = findResults[currentFindIndex]
        let ok = document.setTimingMarkLabel(atRow: Int32(match.rowIndex),
                                              at: Int32(match.markIndex),
                                              label: replacement)
        if ok {
            reloadRows()
            // Re-run the search and try to advance to the same
            // ordinal position — feels closer to the user's intent
            // than always jumping to result 0.
            let priorIndex = currentFindIndex
            runFind()
            if !findResults.isEmpty {
                currentFindIndex = min(priorIndex, findResults.count - 1) - 1
                findNext()
            }
        }
        return ok
    }

    private func focusFindMatch() {
        guard currentFindIndex >= 0, currentFindIndex < findResults.count else { return }
        let m = findResults[currentFindIndex]
        guard m.rowIndex < rows.count, m.markIndex < rows[m.rowIndex].effects.count else { return }
        let mark = rows[m.rowIndex].effects[m.markIndex]
        let centreMS = (mark.startTimeMS + mark.endTimeMS) / 2
        seekTo(ms: centreMS)
        // Selecting the timing mark is a no-op for selection state
        // (timing marks aren't EffectSelections), so just centre on
        // the mark; row scroll is handled by the seek + follow-
        // playhead path if enabled.
    }

    /// Returns true if the mark at the given index on the given row
    /// strictly contains `playPositionMS` — used to gate the split
    /// menu entry.
    func canSplitMarkAtPlayMarker(rowIndex: Int, markIndex: Int) -> Bool {
        guard rowIndex >= 0, rowIndex < rows.count else { return false }
        let row = rows[rowIndex]
        guard row.timing != nil,
              markIndex >= 0, markIndex < row.effects.count else { return false }
        let m = row.effects[markIndex]
        return playPositionMS > m.startTimeMS && playPositionMS < m.endTimeMS
    }

    /// Returns true if the mark has a right-neighbor on the same row
    /// — used to gate the merge menu entry.
    func canMergeMarkWithNext(rowIndex: Int, markIndex: Int) -> Bool {
        guard rowIndex >= 0, rowIndex < rows.count else { return false }
        let row = rows[rowIndex]
        guard row.timing != nil,
              markIndex >= 0, markIndex + 1 < row.effects.count else { return false }
        return true
    }

    /// B68: move / resize a timing mark. Routes through the same
    /// `moveEffect` bridge call used for ordinary effects (marks are
    /// Effects on a timing layer), which already validates overlap.
    @discardableResult
    func moveTimingMark(rowIndex: Int, markIndex: Int,
                         newStartMS: Int, newEndMS: Int) -> Bool {
        guard rowIndex >= 0, rowIndex < rows.count else { return false }
        let row = rows[rowIndex]
        guard row.timing != nil,
              markIndex >= 0, markIndex < row.effects.count else { return false }
        let prev = row.effects[markIndex]
        if prev.startTimeMS == newStartMS && prev.endTimeMS == newEndMS { return true }
        moveEffect(rowIndex: rowIndex, effectIndex: markIndex,
                    newStartMS: newStartMS, newEndMS: newEndMS)
        undoManager.setActionName("Move Timing Mark")
        return true
    }

    /// B91: halve every mark on a timing row — each mark becomes
    /// two by inserting a midpoint split. Walks the marks from
    /// last to first so each split doesn't shift the indices of
    /// the ones we haven't processed yet. Marks with < 2 ms span
    /// are skipped since splitTimingMark requires atMS strictly
    /// between start and end. Registers one undo step for the
    /// whole operation.
    @discardableResult
    func halveTimingMarks(rowIndex: Int) -> Int {
        guard rowIndex >= 0, rowIndex < rows.count else { return 0 }
        let row = rows[rowIndex]
        guard row.timing != nil else { return 0 }
        let count = row.effects.count
        guard count > 0 else { return 0 }
        undoManager.beginUndoGrouping()
        var split = 0
        for i in stride(from: count - 1, through: 0, by: -1) {
            let m = row.effects[i]
            let mid = (m.startTimeMS + m.endTimeMS) / 2
            guard mid > m.startTimeMS && mid < m.endTimeMS else { continue }
            if splitTimingMark(rowIndex: rowIndex, markIndex: i, atMS: mid) {
                split += 1
            }
        }
        undoManager.endUndoGrouping()
        undoManager.setActionName("Halve Timing Marks")
        return split
    }

    /// B90: toggle the `-shimmer` suffix on a timing mark's label.
    /// Convenience op — adds the suffix if missing, strips it if
    /// present. Passes through `renameTimingMark` so undo reuses
    /// the same inverse. Returns the new label (nil on failure).
    @discardableResult
    func toggleShimmerSuffixOnMark(rowIndex: Int, markIndex: Int) -> String? {
        guard rowIndex >= 0, rowIndex < rows.count else { return nil }
        let row = rows[rowIndex]
        guard row.timing != nil,
              markIndex >= 0, markIndex < row.effects.count else { return nil }
        let current = row.effects[markIndex].name
        let newLabel: String
        if current.hasSuffix("-shimmer") {
            newLabel = String(current.dropLast("-shimmer".count))
        } else {
            newLabel = current + "-shimmer"
        }
        return renameTimingMark(rowIndex: rowIndex, markIndex: markIndex,
                                 label: newLabel) ? newLabel : nil
    }

    /// B70: set a timing mark's label. Empty string clears it.
    /// Undo-able.
    @discardableResult
    func renameTimingMark(rowIndex: Int, markIndex: Int, label: String) -> Bool {
        guard rowIndex >= 0, rowIndex < rows.count else { return false }
        let row = rows[rowIndex]
        guard row.timing != nil,
              markIndex >= 0, markIndex < row.effects.count else { return false }
        let origLabel = row.effects[markIndex].name
        if origLabel == label { return true }
        if !document.setTimingMarkLabel(atRow: Int32(rowIndex),
                                         at: Int32(markIndex),
                                         label: label) {
            return false
        }
        reloadRows()
        undoManager.registerUndo(withTarget: self) { vm in
            vm.renameTimingMark(rowIndex: rowIndex, markIndex: markIndex, label: origLabel)
        }
        undoManager.setActionName("Rename Timing Mark")
        return true
    }

    /// B69: delete a timing mark. Registers an inverse add so ⌘Z
    /// brings it back with the same start/end/label.
    @discardableResult
    func deleteTimingMark(rowIndex: Int, markIndex: Int) -> Bool {
        guard rowIndex >= 0, rowIndex < rows.count else { return false }
        let row = rows[rowIndex]
        guard row.timing != nil,
              markIndex >= 0, markIndex < row.effects.count else { return false }
        let e = row.effects[markIndex]
        let origStart = e.startTimeMS
        let origEnd = e.endTimeMS
        let origLabel = e.name
        if !document.deleteTimingMark(atRow: Int32(rowIndex), at: Int32(markIndex)) {
            return false
        }
        reloadRows()
        undoManager.registerUndo(withTarget: self) { vm in
            vm.addTimingMark(rowIndex: rowIndex,
                              startMS: origStart, endMS: origEnd,
                              label: origLabel)
        }
        undoManager.setActionName("Delete Timing Mark")
        return true
    }

    // MARK: - Select in row / column (B2)

    /// Select every effect on the given row. Entry point from
    /// `ModelRowHeader`'s long-press menu. No-op for rows with no
    /// effects. If the row is a timing row (mark-only), does nothing
    /// since timing-mark editing is deferred (B67-B72).
    func selectAllEffectsInRow(rowIndex: Int) {
        guard rowIndex >= 0, rowIndex < rows.count else { return }
        let row = rows[rowIndex]
        if row.timing != nil { return }
        let hits: Set<EffectSelection> = Set(
            row.effects.enumerated().map { (i, e) in
                EffectSelection(rowIndex: rowIndex, effectIndex: i,
                                 name: e.name,
                                 startTimeMS: e.startTimeMS,
                                 endTimeMS: e.endTimeMS)
            }
        )
        setMultiSelection(hits)
    }

    /// B52: select every effect on the given model row plus every
    /// row below it that belongs to the same model — sub-layers,
    /// submodels, strands, nodes. Walks back from `rowIndex` to
    /// find the model's top row (first row with `nestDepth == 0
    /// && layerIndex == 0 && !isSubmodel`), then forward until the
    /// next such row (or end of list), collecting effects on every
    /// non-timing row along the way.
    func selectAllEffectsInModel(rowIndex: Int) {
        guard rowIndex >= 0, rowIndex < rows.count else { return }
        // Walk back to find the top model row.
        var top = rowIndex
        while top > 0 {
            let r = rows[top]
            if r.timing == nil, r.nestDepth == 0, r.layerIndex == 0,
               !r.isSubmodel {
                break
            }
            top -= 1
        }
        // If the loop bailed at 0 but that row isn't a top model row,
        // nothing to do.
        let startRow = rows[top]
        if startRow.timing != nil || startRow.nestDepth != 0
            || startRow.layerIndex != 0 || startRow.isSubmodel {
            return
        }
        // Walk forward including every related row.
        var hits: Set<EffectSelection> = []
        var i = top
        while i < rows.count {
            let r = rows[i]
            // Stop at the next top model row (but include the start).
            if i > top, r.timing == nil, r.nestDepth == 0,
               r.layerIndex == 0, !r.isSubmodel {
                break
            }
            if r.timing != nil { i += 1; continue }
            for (eIdx, e) in r.effects.enumerated() {
                hits.insert(.init(rowIndex: i, effectIndex: eIdx,
                                   name: e.name,
                                   startTimeMS: e.startTimeMS,
                                   endTimeMS: e.endTimeMS))
            }
            i += 1
        }
        setMultiSelection(hits)
    }

    /// Select every effect across every non-timing row whose time
    /// range overlaps `[spanStartMS, spanEndMS]`. Entry point from
    /// the single-effect context menu ("Select All in Column") where
    /// the clicked effect's own range defines the column extent.
    func selectAllEffectsInColumn(spanStartMS: Int, spanEndMS: Int) {
        var hits: Set<EffectSelection> = []
        for (rIdx, row) in rows.enumerated() {
            if row.timing != nil { continue }
            for (eIdx, e) in row.effects.enumerated() {
                if e.endTimeMS < spanStartMS { continue }
                if e.startTimeMS > spanEndMS { break }
                hits.insert(.init(rowIndex: rIdx, effectIndex: eIdx,
                                   name: e.name,
                                   startTimeMS: e.startTimeMS,
                                   endTimeMS: e.endTimeMS))
            }
        }
        setMultiSelection(hits)
    }

    // MARK: - Split (B12)

    /// Returns true if the single selected effect spans the current
    /// `playPositionMS` (exclusive of the boundaries). Drives the
    /// enabled state of the "Split at Play Marker" context-menu item.
    var canSplitSelectedAtPlayMarker: Bool {
        guard let sel = selectedEffect else { return false }
        let m = playPositionMS
        return m > sel.startTimeMS && m < sel.endTimeMS
    }

    /// Split the single-selected effect at `playPositionMS`. The left
    /// half gets the original effect's settings + palette, range
    /// `[origStart, marker]`. The right half gets the same
    /// settings+palette, range `[marker, origEnd]`. Registers one
    /// undo group so ⌘Z reverses both halves.
    func splitSelectedEffectAtPlayMarker() {
        guard let sel = selectedEffect else { return }
        let marker = playPositionMS
        guard marker > sel.startTimeMS, marker < sel.endTimeMS else { return }
        let rowIndex = sel.rowIndex
        let effectIndex = sel.effectIndex
        let name = document.effectName(forRow: Int32(rowIndex), at: Int32(effectIndex))
        let settings = document.effectSettingsString(forRow: Int32(rowIndex), at: Int32(effectIndex))
        let palette = document.effectPaletteString(forRow: Int32(rowIndex), at: Int32(effectIndex))
        let origStart = sel.startTimeMS
        let origEnd = sel.endTimeMS
        undoManager.beginUndoGrouping()
        // Delete original, then add two halves. deleteEffect registers
        // its own undo (recreate original); addEffectWithSettings
        // registers its own (delete new). The group rolls them up.
        deleteEffect(rowIndex: rowIndex, effectIndex: effectIndex)
        let leftIdx = addEffectWithSettings(rowIndex: rowIndex, name: name,
                                              settings: settings, palette: palette,
                                              startMS: origStart, endMS: marker)
        _ = addEffectWithSettings(rowIndex: rowIndex, name: name,
                                    settings: settings, palette: palette,
                                    startMS: marker, endMS: origEnd)
        undoManager.endUndoGrouping()
        undoManager.setActionName("Split Effect")
        // Auto-select the left half — saves a tap for the common
        // "split then edit the first segment" flow. User can Tab /
        // right-arrow to the right half if needed.
        if leftIdx >= 0 {
            selectEffect(rowIndex: rowIndex, effectIndex: leftIdx)
        }
    }

    // MARK: - Align (B8)

    enum AlignMode {
        case startTimes       // all starts → anchor's start (ends stay, widen/narrow)
        case endTimes         // all ends   → anchor's end (starts stay, widen/narrow)
        case bothTimes        // all starts + ends → anchor's range
        case centerPoints     // all midpoints → anchor's midpoint
        case matchDuration    // each effect keeps its start, end = start + anchor.duration
        case startTimesShift  // B9: slide so start matches anchor.start, duration preserved
        case endTimesShift    // B9: slide so end matches anchor.end, duration preserved
    }

    /// Align every effect in `selectedEffects` to the anchor according
    /// to `mode`. Anchor selection rule:
    ///  - start/both/match/centers  → earliest-starting selected effect
    ///  - end                       → latest-ending selected effect
    ///
    /// Same-row aligns that would collide on an overlap are rejected
    /// by the bridge's validation per-effect (moves partially); users
    /// multi-selecting across rows is the common case and works
    /// straight through. Implemented as one undo group so a single
    /// `Cmd+Z` reverses the whole alignment.
    func alignSelectedEffects(_ mode: AlignMode) {
        guard selectedEffects.count >= 2 else { return }
        let anchor: EffectSelection
        switch mode {
        case .startTimes, .startTimesShift, .bothTimes, .matchDuration, .centerPoints:
            anchor = selectedEffects.min(by: { $0.startTimeMS < $1.startTimeMS })!
        case .endTimes, .endTimesShift:
            anchor = selectedEffects.max(by: { $0.endTimeMS < $1.endTimeMS })!
        }

        // Snapshot every target move up-front (rowIndex, origStartMS,
        // new start/end). `origStartMS` is used to re-find the effect
        // on each row after earlier moves in the iteration may have
        // shifted indices.
        struct AlignMove {
            let rowIndex: Int
            let origStartMS: Int
            let newStartMS: Int
            let newEndMS: Int
        }
        var moves: [AlignMove] = []
        let anchorDuration = anchor.endTimeMS - anchor.startTimeMS
        let anchorCenter = anchor.startTimeMS + anchorDuration / 2
        for sel in selectedEffects {
            if sel == anchor { continue }  // anchor stays put
            let dur = sel.endTimeMS - sel.startTimeMS
            let newStart: Int
            let newEnd: Int
            switch mode {
            case .startTimes:
                newStart = anchor.startTimeMS
                // Keep original end unless it would invert.
                newEnd = max(sel.endTimeMS, newStart + 1)
            case .endTimes:
                newEnd = anchor.endTimeMS
                newStart = min(sel.startTimeMS, newEnd - 1)
            case .bothTimes:
                newStart = anchor.startTimeMS
                newEnd = anchor.endTimeMS
            case .centerPoints:
                let selCenter = sel.startTimeMS + dur / 2
                let delta = anchorCenter - selCenter
                newStart = max(0, sel.startTimeMS + delta)
                newEnd = newStart + dur
            case .matchDuration:
                newStart = sel.startTimeMS
                newEnd = newStart + anchorDuration
            case .startTimesShift:
                // Slide, don't stretch. Duration preserved.
                newStart = anchor.startTimeMS
                newEnd = newStart + dur
            case .endTimesShift:
                newEnd = anchor.endTimeMS
                newStart = newEnd - dur
            }
            moves.append(AlignMove(rowIndex: sel.rowIndex,
                                    origStartMS: sel.startTimeMS,
                                    newStartMS: max(0, newStart),
                                    newEndMS: max(1, newEnd)))
        }
        if moves.isEmpty { return }

        undoManager.beginUndoGrouping()
        for m in moves {
            // Re-find the effect by its original startMS — earlier
            // moves in this loop may have shifted effectIndex on the
            // same row. Skips if the effect can't be found (would
            // only happen if a prior move collided and triggered
            // some external refresh).
            guard m.rowIndex >= 0, m.rowIndex < rows.count else { continue }
            let row = rows[m.rowIndex]
            guard let eIdx = row.effects.firstIndex(where: {
                $0.startTimeMS == m.origStartMS
            }) else { continue }
            moveEffect(rowIndex: m.rowIndex, effectIndex: eIdx,
                       newStartMS: m.newStartMS, newEndMS: m.newEndMS)
        }
        undoManager.endUndoGrouping()
        let action: String
        switch mode {
        case .startTimes:      action = "Align Start Times"
        case .endTimes:        action = "Align End Times"
        case .bothTimes:       action = "Align Both Times"
        case .centerPoints:    action = "Align Centers"
        case .matchDuration:   action = "Match Duration"
        case .startTimesShift: action = "Shift-Align Start"
        case .endTimesShift:   action = "Shift-Align End"
        }
        undoManager.setActionName(action)
    }

    /// B4: stretch the selected effect's end edge by `deltaMS`
    /// (positive extends right, negative shrinks). Clamped against
    /// the next effect on the same row and against `startMS + 1`
    /// as a minimum-duration floor. Routes through the existing
    /// `moveEffect` pipeline so undo works.
    func stretchSelectedEffectEnd(by deltaMS: Int) {
        guard let sel = selectedEffect else { return }
        guard sel.rowIndex < rows.count,
              sel.effectIndex < rows[sel.rowIndex].effects.count else { return }
        let row = rows[sel.rowIndex]
        var maxEnd = Int.max
        if sel.effectIndex + 1 < row.effects.count {
            maxEnd = row.effects[sel.effectIndex + 1].startTimeMS
        } else {
            maxEnd = sequenceDurationMS
        }
        let newEnd = max(sel.startTimeMS + 1,
                          min(maxEnd, sel.endTimeMS + deltaMS))
        if newEnd == sel.endTimeMS { return }
        moveEffect(rowIndex: sel.rowIndex, effectIndex: sel.effectIndex,
                    newStartMS: sel.startTimeMS, newEndMS: newEnd)
    }

    /// B4: slide the selected effect by `deltaMS` (positive forward,
    /// negative back). Duration preserved. Clamped against both
    /// neighbors on the same row. Routes through `moveEffect`.
    func nudgeSelectedEffect(by deltaMS: Int) {
        guard let sel = selectedEffect else { return }
        guard sel.rowIndex < rows.count,
              sel.effectIndex < rows[sel.rowIndex].effects.count else { return }
        let row = rows[sel.rowIndex]
        let prevEnd = sel.effectIndex > 0
            ? row.effects[sel.effectIndex - 1].endTimeMS : 0
        let nextStart = sel.effectIndex + 1 < row.effects.count
            ? row.effects[sel.effectIndex + 1].startTimeMS : sequenceDurationMS
        let dur = sel.endTimeMS - sel.startTimeMS
        let newStart = max(prevEnd,
                            min(nextStart - dur, sel.startTimeMS + deltaMS))
        if newStart == sel.startTimeMS { return }
        moveEffect(rowIndex: sel.rowIndex, effectIndex: sel.effectIndex,
                    newStartMS: newStart, newEndMS: newStart + dur)
    }

    /// B83: create a new variable timing track whose marks align
    /// with every effect's `[start, end]` on `modelRowIndex`. Walks
    /// every row that belongs to the same element (so sub-layer /
    /// submodel / strand / node effects all contribute) and
    /// de-duplicates identical ranges. Returns the number of marks
    /// added; 0 if the row has no effects or the track couldn't be
    /// created (name collision already uniquified by the bridge).
    @discardableResult
    func createTimingTrackFromEffects(modelRowIndex rowIndex: Int,
                                       trackName name: String) -> Int {
        guard rowIndex >= 0, rowIndex < rows.count else { return 0 }
        let base = (document.rowModelName(at: Int32(rowIndex)) as String?) ?? "Model"
        // Gather every row that forwards to the same element. The
        // bridge's `rowModelName` returns the element name, so rows
        // with a matching name belong to the same element tree.
        var ranges: Set<String> = []
        var pairs: [(start: Int, end: Int)] = []
        for row in rows {
            let rowName = (document.rowModelName(at: Int32(row.id)) as String?) ?? ""
            if rowName != base { continue }
            for e in row.effects {
                let key = "\(e.startTimeMS)-\(e.endTimeMS)"
                if ranges.insert(key).inserted {
                    pairs.append((start: e.startTimeMS, end: e.endTimeMS))
                }
            }
        }
        guard !pairs.isEmpty else { return 0 }
        pairs.sort { $0.start < $1.start }

        let trimmed = name.trimmingCharacters(in: .whitespacesAndNewlines)
        let proposed = trimmed.isEmpty ? "\(base) Timing" : trimmed
        undoManager.beginUndoGrouping()
        if !addTimingTrack(name: proposed) {
            undoManager.endUndoGrouping()
            return 0
        }
        // `addTimingTrack` uniquifies names + reloads rows. Find
        // the track's row index by matching its element name.
        let addedName = addedTimingTrackName(prefix: proposed)
        let timingRowIdx = rows.firstIndex(where: {
            $0.timing?.elementName == addedName
        }) ?? -1
        guard timingRowIdx >= 0 else {
            undoManager.endUndoGrouping()
            return 0
        }
        var added = 0
        for p in pairs {
            if addTimingMark(rowIndex: timingRowIdx,
                              startMS: p.start, endMS: p.end,
                              label: "") {
                added += 1
            }
        }
        undoManager.endUndoGrouping()
        undoManager.setActionName("Create Timing From Effects")
        return added
    }

    /// Find the exact name the bridge settled on after
    /// `addTimingTrack` — `addTimingTrackNamed:` appends a suffix
    /// on name collisions. We scan timing rows for any name
    /// starting with `prefix` (taking the most-recently-added).
    private func addedTimingTrackName(prefix: String) -> String {
        let timingIdx = (document.timingRowIndices() as [NSNumber]).map { $0.intValue }
        var last: String? = nil
        for idx in timingIdx {
            guard idx < rows.count else { continue }
            if let name = rows[idx].timing?.elementName, name.hasPrefix(prefix) {
                last = name
            }
        }
        return last ?? prefix
    }

    /// B18: double-tap on empty area of a model row creates an
    /// effect spanning the gap between the previous and next
    /// neighbour (or an active timing cell if one brackets `atMS`).
    /// Requires an armed palette effect; no-op otherwise.
    func doubleTapCreateInCell(rowIndex: Int, atMS: Int) {
        guard selectedPaletteEffect != nil,
              rowIndex >= 0, rowIndex < rows.count else { return }
        let row = rows[rowIndex]
        guard row.timing == nil else { return }
        // Prefer the active timing cell if one brackets the tap —
        // matches `pasteEffect`'s cell-aware behaviour.
        let startMS: Int
        let endMS: Int
        if let cell = activeTimingCell(forMS: atMS) {
            startMS = cell.startMS
            endMS = min(cell.endMS, sequenceDurationMS)
        } else {
            // Fill the gap between neighbours on the row.
            var prevEnd = 0
            var nextStart = sequenceDurationMS
            for e in row.effects {
                if e.endTimeMS <= atMS { prevEnd = max(prevEnd, e.endTimeMS) }
                if e.startTimeMS > atMS, e.startTimeMS < nextStart {
                    nextStart = e.startTimeMS
                }
            }
            startMS = prevEnd
            endMS = nextStart
        }
        guard endMS > startMS else { return }
        addEffect(rowIndex: rowIndex, startMS: startMS, endMS: endMS)
    }

    /// B13: extend the selected effect's end to butt against the
    /// next effect on the same row (or the sequence end when it's
    /// the last one). Preserves start; grows duration. No-op when
    /// the end is already flush with the neighbour.
    func extendSelectedEffectToNext() {
        guard let sel = selectedEffect,
              sel.rowIndex >= 0, sel.rowIndex < rows.count,
              sel.effectIndex >= 0,
              sel.effectIndex < rows[sel.rowIndex].effects.count else { return }
        let row = rows[sel.rowIndex]
        let nextStart = sel.effectIndex + 1 < row.effects.count
            ? row.effects[sel.effectIndex + 1].startTimeMS
            : sequenceDurationMS
        if nextStart <= sel.endTimeMS { return }
        moveEffect(rowIndex: sel.rowIndex, effectIndex: sel.effectIndex,
                    newStartMS: sel.startTimeMS, newEndMS: nextStart)
    }

    /// B13: extend the selected effect's start back to butt against
    /// the previous effect on the same row (or sequence start).
    /// Preserves end; grows duration.
    func extendSelectedEffectToPrevious() {
        guard let sel = selectedEffect,
              sel.rowIndex >= 0, sel.rowIndex < rows.count,
              sel.effectIndex >= 0,
              sel.effectIndex < rows[sel.rowIndex].effects.count else { return }
        let row = rows[sel.rowIndex]
        let prevEnd = sel.effectIndex > 0
            ? row.effects[sel.effectIndex - 1].endTimeMS
            : 0
        if prevEnd >= sel.startTimeMS { return }
        moveEffect(rowIndex: sel.rowIndex, effectIndex: sel.effectIndex,
                    newStartMS: prevEnd, newEndMS: sel.endTimeMS)
    }

    /// B6: nudge the selected effect's start to the previous or
    /// next active timing-mark position (preserving duration).
    /// `direction` < 0 → previous mark; > 0 → next mark. Snaps to
    /// the effect's current neighbour range so the nudge still
    /// respects butted effects. No-op when there are no active
    /// timing marks or nothing is selected.
    func nudgeSelectedEffectToTimingMark(direction: Int) {
        guard let sel = selectedEffect, direction != 0 else { return }
        let marks = activeTimingMarkTimes().sorted()
        guard !marks.isEmpty else { return }
        let current = sel.startTimeMS
        let target: Int
        if direction < 0 {
            guard let prev = marks.last(where: { $0 < current }) else { return }
            target = prev
        } else {
            guard let next = marks.first(where: { $0 > current }) else { return }
            target = next
        }
        let dur = sel.endTimeMS - sel.startTimeMS
        guard sel.rowIndex < rows.count,
              sel.effectIndex < rows[sel.rowIndex].effects.count else { return }
        let row = rows[sel.rowIndex]
        let prevEnd = sel.effectIndex > 0
            ? row.effects[sel.effectIndex - 1].endTimeMS : 0
        let nextStart = sel.effectIndex + 1 < row.effects.count
            ? row.effects[sel.effectIndex + 1].startTimeMS : sequenceDurationMS
        let newStart = max(prevEnd, min(nextStart - dur, target))
        if newStart == current { return }
        moveEffect(rowIndex: sel.rowIndex, effectIndex: sel.effectIndex,
                    newStartMS: newStart, newEndMS: newStart + dur)
    }

    /// Union of every start/end from every row on an active timing
    /// track. Used by `nudgeSelectedEffectToTimingMark` and by the
    /// grid view's guide-line overlay.
    func activeTimingMarkTimes() -> [Int] {
        let timingIdx = (document.timingRowIndices() as [NSNumber]).map { $0.intValue }
        var out: Set<Int> = []
        for idx in timingIdx {
            guard document.timingRowIsActive(at: Int32(idx)) else { continue }
            guard let row = rows.first(where: { $0.id == idx }) else { continue }
            for e in row.effects {
                out.insert(e.startTimeMS)
                out.insert(e.endTimeMS)
            }
        }
        return Array(out)
    }

    /// B53: copy every effect on a given row into the clipboard
    /// (preserving relative timing). Selection is set to the copied
    /// set — mirrors desktop's "selection lingers after Copy."
    func copyRow(rowIndex: Int) {
        selectAllEffectsInRow(rowIndex: rowIndex)
        copySelectedEffects()
    }

    /// B53: cut every effect on a given row — copy then delete.
    func cutRow(rowIndex: Int) {
        copyRow(rowIndex: rowIndex)
        _ = deleteAllEffectsOnRow(rowIndex: rowIndex)
    }

    /// B54: copy every effect across a model (layers + submodels +
    /// strands + nodes) into the clipboard.
    func copyModel(rowIndex: Int) {
        selectAllEffectsInModel(rowIndex: rowIndex)
        copySelectedEffects()
    }

    /// B54: cut every effect across a model — copy then delete on
    /// every participating row. Operates over the set copied above
    /// so rows shift without stranding references.
    func cutModel(rowIndex: Int) {
        copyModel(rowIndex: rowIndex)
        deleteSelectedEffects()
    }

    /// B50: delete every effect on a given row. Uses the existing
    /// `deleteEffect` pipeline (which re-renders cleared ranges +
    /// registers per-effect undo) wrapped in one undo group so ⌘Z
    /// reverses the whole bulk clear.
    @discardableResult
    func deleteAllEffectsOnRow(rowIndex: Int) -> Bool {
        guard rowIndex >= 0, rowIndex < rows.count else { return false }
        let row = rows[rowIndex]
        if row.effects.isEmpty { return false }
        let count = row.effects.count
        undoManager.beginUndoGrouping()
        // Descending index order so earlier deletes don't shift
        // later ones (same pattern as `deleteSelectedEffects`).
        for i in stride(from: count - 1, through: 0, by: -1) {
            deleteEffect(rowIndex: rowIndex, effectIndex: i)
        }
        undoManager.endUndoGrouping()
        undoManager.setActionName("Delete All Effects on Row")
        return true
    }

    /// B51: toggle the Element-level render-disabled flag. Rendering
    /// skips the whole element (model + submodels + strands + nodes)
    /// while disabled. Not undo-able in first cut.
    func toggleElementRenderDisabled(rowIndex: Int) {
        guard rowIndex >= 0, rowIndex < rows.count else { return }
        let cur = document.elementRenderDisabled(atRow: Int32(rowIndex))
        document.setElementRenderDisabled(!cur, atRow: Int32(rowIndex))
        reloadRows()
    }

    func isElementRenderDisabled(rowIndex: Int) -> Bool {
        guard rowIndex >= 0, rowIndex < rows.count else { return false }
        return document.elementRenderDisabled(atRow: Int32(rowIndex))
    }

    /// B46: rename an effect layer in-place. Undo-able with the
    /// original name.
    @discardableResult
    func renameLayer(rowIndex: Int, name: String) -> Bool {
        guard rowIndex >= 0, rowIndex < rows.count else { return false }
        let origName = document.rowLayerName(at: Int32(rowIndex))
        if !document.renameLayer(atRow: Int32(rowIndex), name: name) { return false }
        reloadRows()
        undoManager.registerUndo(withTarget: self) { vm in
            vm.renameLayer(rowIndex: rowIndex, name: origName)
        }
        undoManager.setActionName("Rename Layer")
        return true
    }

    /// B87: strip word + phoneme layers off a phrase timing row —
    /// inverse of `breakdownPhrases`. Not currently undo-able (the
    /// operation removes layer structure; layer-level undo is
    /// follow-up work).
    @discardableResult
    func removeWordsAndPhonemes(rowIndex: Int) -> Bool {
        guard rowIndex >= 0, rowIndex < rows.count else { return false }
        guard rows[rowIndex].timing != nil else { return false }
        if !document.removeWordsAndPhonemes(atRow: Int32(rowIndex)) { return false }
        reloadRows()
        return true
    }

    /// True when the given timing row is the phrase layer (layer 0)
    /// of an element that has extra layers to strip. Gates the
    /// "Remove Words / Phonemes" menu entry.
    func canRemoveWordsAndPhonemes(rowIndex: Int) -> Bool {
        guard rowIndex >= 0, rowIndex < rows.count else { return false }
        let row = rows[rowIndex]
        guard row.timing != nil, row.layerIndex == 0 else { return false }
        return Int(document.rowLayerCount(at: Int32(rowIndex))) > 1
    }

    /// B57: global expand / collapse. `collapseAllModels` folds
    /// every non-timing element to its first layer (hides sub-
    /// layers + submodels + nodes); `expandAll` undoes that across
    /// the sequence. Not undo-able for the first cut — users flip
    /// back via the opposite button.
    func collapseAllModels() {
        document.collapseAllElements()
        reloadRows()
    }
    func expandAllElements() {
        document.expandAllElements()
        reloadRows()
    }

    /// B10: snap every selected effect's start / end edges to the
    /// nearest active timing-track mark edge within ~1/2 of the
    /// effect's current duration (so a hugely-misaligned effect
    /// doesn't grab an irrelevant nearby mark). Duration is NOT
    /// preserved — each edge moves independently. Rejects when no
    /// timing rows are active.
    func alignSelectedEffectsToTimingMarks() {
        guard selectedEffects.count >= 1 else { return }
        let markTimes = collectActiveMarkTimes()
        if markTimes.isEmpty { return }

        struct AlignMove {
            let rowIndex: Int
            let origStartMS: Int
            let newStartMS: Int
            let newEndMS: Int
        }
        var moves: [AlignMove] = []
        for sel in selectedEffects {
            let dur = sel.endTimeMS - sel.startTimeMS
            // Snap each edge independently to its nearest mark,
            // threshold = half the duration (roughly matches
            // desktop's "snap within a reasonable window" for the
            // per-edge variant).
            let thresh = max(100, dur / 2)
            let newStart = nearest(markTimes, to: sel.startTimeMS, threshold: thresh)
                ?? sel.startTimeMS
            let newEnd = nearest(markTimes, to: sel.endTimeMS, threshold: thresh)
                ?? sel.endTimeMS
            if newStart == sel.startTimeMS && newEnd == sel.endTimeMS { continue }
            // Guard against collapsing to a 0-length effect.
            if newEnd <= newStart { continue }
            moves.append(AlignMove(rowIndex: sel.rowIndex,
                                    origStartMS: sel.startTimeMS,
                                    newStartMS: newStart,
                                    newEndMS: newEnd))
        }
        if moves.isEmpty { return }

        undoManager.beginUndoGrouping()
        for m in moves {
            guard m.rowIndex >= 0, m.rowIndex < rows.count else { continue }
            let row = rows[m.rowIndex]
            guard let eIdx = row.effects.firstIndex(where: {
                $0.startTimeMS == m.origStartMS
            }) else { continue }
            moveEffect(rowIndex: m.rowIndex, effectIndex: eIdx,
                       newStartMS: m.newStartMS, newEndMS: m.newEndMS)
        }
        undoManager.endUndoGrouping()
        undoManager.setActionName("Align to Timing Marks")
    }

    /// B11: slide every consecutive pair of selected same-row
    /// effects together (the later one moves back so its start =
    /// earlier's end). Pairs across rows are ignored since "close
    /// gap" only makes sense on one track. Rejects when no two
    /// selected effects share a row.
    func closeGapInSelectedEffects() {
        guard selectedEffects.count >= 2 else { return }
        // Group by row, sort each group by startMS ascending.
        let byRow = Dictionary(grouping: selectedEffects, by: { $0.rowIndex })
        var moves: [(rowIndex: Int, origStartMS: Int,
                     newStartMS: Int, newEndMS: Int)] = []
        for (rowIdx, effects) in byRow {
            let sorted = effects.sorted { $0.startTimeMS < $1.startTimeMS }
            guard sorted.count >= 2 else { continue }
            // For each consecutive pair, push the later effect's
            // start back to the earlier effect's end (preserving the
            // later effect's duration).
            var prevEnd = sorted[0].endTimeMS
            for i in 1..<sorted.count {
                let e = sorted[i]
                let dur = e.endTimeMS - e.startTimeMS
                if e.startTimeMS > prevEnd {
                    let newStart = prevEnd
                    let newEnd = newStart + dur
                    moves.append((rowIndex: rowIdx,
                                  origStartMS: e.startTimeMS,
                                  newStartMS: newStart,
                                  newEndMS: newEnd))
                    prevEnd = newEnd
                } else {
                    prevEnd = e.endTimeMS
                }
            }
        }
        if moves.isEmpty { return }

        undoManager.beginUndoGrouping()
        // Process each row's moves in order of original startMS so
        // later effects (which actually need to move) shift into
        // space we know is clear — moving them first would overlap
        // with earlier effects still at their old positions.
        let sortedMoves = moves.sorted { $0.origStartMS < $1.origStartMS }
        for m in sortedMoves {
            guard m.rowIndex >= 0, m.rowIndex < rows.count else { continue }
            let row = rows[m.rowIndex]
            guard let eIdx = row.effects.firstIndex(where: {
                $0.startTimeMS == m.origStartMS
            }) else { continue }
            moveEffect(rowIndex: m.rowIndex, effectIndex: eIdx,
                       newStartMS: m.newStartMS, newEndMS: m.newEndMS)
        }
        undoManager.endUndoGrouping()
        undoManager.setActionName("Close Gap")
    }

    /// True iff any pair of selected effects share a row with a
    /// positive gap between them — gates the Close Gap menu entry.
    var canCloseGapInSelection: Bool {
        guard selectedEffects.count >= 2 else { return false }
        let byRow = Dictionary(grouping: selectedEffects, by: { $0.rowIndex })
        for (_, effects) in byRow {
            let sorted = effects.sorted { $0.startTimeMS < $1.startTimeMS }
            for i in 1..<sorted.count {
                if sorted[i].startTimeMS > sorted[i - 1].endTimeMS { return true }
            }
        }
        return false
    }

    /// Gather all effect start+end times on every active timing
    /// row — used by B10 and the grid-internal drag snap. Sorted
    /// ascending; duplicates left in (cheap, hot-path doesn't care).
    private func collectActiveMarkTimes() -> [Int] {
        var out: [Int] = []
        for row in rows {
            guard let t = row.timing, t.isActive else { continue }
            for e in row.effects {
                out.append(e.startTimeMS)
                out.append(e.endTimeMS)
            }
        }
        return out.sorted()
    }

    private func nearest(_ times: [Int], to target: Int,
                          threshold: Int) -> Int? {
        guard !times.isEmpty else { return nil }
        var best: Int?
        var bestD = threshold + 1
        for t in times {
            let d = abs(t - target)
            if d < bestD { bestD = d; best = t }
        }
        return best
    }

    func toggleDisableSelectedEffects() {
        if selectedEffects.count <= 1 {
            toggleDisableSelected()
            return
        }
        let allDisabled = selectedEffects.allSatisfy {
            document.effectIsRenderDisabled(inRow: Int32($0.rowIndex), at: Int32($0.effectIndex))
        }
        let targetDisabled = !allDisabled
        undoManager.beginUndoGrouping()
        for sel in selectedEffects {
            let cur = document.effectIsRenderDisabled(inRow: Int32(sel.rowIndex), at: Int32(sel.effectIndex))
            if cur != targetDisabled {
                toggleDisable(rowIndex: sel.rowIndex, effectIndex: sel.effectIndex)
            }
        }
        undoManager.endUndoGrouping()
        undoManager.setActionName(targetDisabled ? "Disable Effects" : "Enable Effects")
    }

    /// Delete a specific effect, capturing its state for undo.
    func deleteEffect(rowIndex: Int, effectIndex: Int) {
        guard rowIndex < rows.count, effectIndex < rows[rowIndex].effects.count else { return }
        let prev = rows[rowIndex].effects[effectIndex]
        let name = document.effectName(forRow: Int32(rowIndex), at: Int32(effectIndex))
        let settings = document.effectSettingsString(forRow: Int32(rowIndex), at: Int32(effectIndex))
        let palette = document.effectPaletteString(forRow: Int32(rowIndex), at: Int32(effectIndex))
        let startMS = prev.startTimeMS
        let endMS = prev.endTimeMS

        if document.deleteEffect(inRow: Int32(rowIndex), at: Int32(effectIndex)) {
            // Clear the removed effect's output from SequenceData.
            renderRangeAndTrack(rowIndex: rowIndex,
                                 startMS: startMS, endMS: endMS,
                                 clear: true)
            if selectedEffect?.rowIndex == rowIndex && selectedEffect?.effectIndex == effectIndex {
                clearSelection()
            }
            reloadRows()
            undoManager.registerUndo(withTarget: self) { vm in
                vm.addEffectWithSettings(rowIndex: rowIndex, name: name,
                                          settings: settings, palette: palette,
                                          startMS: startMS, endMS: endMS)
            }
            undoManager.setActionName("Delete Effect")
        }
    }

    func addEffect(rowIndex: Int, startMS: Int, endMS: Int) {
        let name = selectedPaletteEffect ?? "On"
        let seed = seedsForNewEffect(ofType: name)
        addEffectWithSettings(rowIndex: rowIndex, name: name,
                               settings: seed.settings, palette: seed.palette,
                               startMS: startMS, endMS: endMS)
    }

    /// Pick the settings + palette strings to seed a brand-new effect
    /// placement with. Mirrors desktop's "the panels you're currently
    /// looking at become the next drop's starting point":
    /// - Palette (C_*): always inherit from the currently-selected
    ///   effect if there is one. Matches the persistent ColorPanel
    ///   sidebar — set up your colours once and drop several effects
    ///   against them.
    /// - Effect settings (E_/B_/T_): inherit only when the selected
    ///   effect is the same type as the one being dropped. Different
    ///   type → fall back to the effect's metadata defaults (empty
    ///   string means "use defaults" at render time).
    /// With nothing selected we emit a hardcoded 8-slot palette so
    /// `C_BUTTON_PaletteN` is always populated — otherwise flipping
    /// a `C_CHECKBOX_PaletteN` toggle on would parse as
    /// `xlColor("") == black`.
    private func seedsForNewEffect(ofType name: String) -> (settings: String, palette: String) {
        guard let sel = selectedEffect else {
            return ("", Self.defaultPaletteString)
        }
        let palette: String = {
            let pal = document.effectPaletteString(forRow: Int32(sel.rowIndex),
                                                    at: Int32(sel.effectIndex))
            return pal.isEmpty ? Self.defaultPaletteString : pal
        }()
        let settings: String = (sel.name == name)
            ? (document.effectSettingsString(forRow: Int32(sel.rowIndex),
                                              at: Int32(sel.effectIndex)))
            : ""
        return (settings, palette)
    }

    /// Fallback palette used when no effect is currently selected at
    /// drop time. All eight `C_BUTTON_Palette*` entries are populated
    /// with the hex values the palette UI shows as swatch defaults,
    /// so flipping a `C_CHECKBOX_PaletteN` toggle on produces the
    /// expected colour instead of `xlColor("") == black`. Checkboxes
    /// stay off — matches the desktop ColorPanel-emitted string for
    /// a fresh effect.
    static let defaultPaletteString = [
        "C_BUTTON_Palette1=#FF0000",
        "C_BUTTON_Palette2=#00FF00",
        "C_BUTTON_Palette3=#0000FF",
        "C_BUTTON_Palette4=#FFFF00",
        "C_BUTTON_Palette5=#FFFFFF",
        "C_BUTTON_Palette6=#000000",
        "C_BUTTON_Palette7=#FFA500",
        "C_BUTTON_Palette8=#800080"
    ].joined(separator: ",")

    /// Tap-to-add flow from the grid: insert a new effect of the
    /// currently armed palette type at `atMS`, spanning to the next
    /// neighbor (or a default 1-second duration, whichever is shorter).
    /// Clamped against the previous neighbor's end so we never start
    /// inside another effect.
    func addEffectFromPaletteTap(rowIndex: Int, atMS: Int) {
        guard let paletteName = selectedPaletteEffect,
              rowIndex < rows.count else { return }
        let row = rows[rowIndex]

        // Scan the row's existing effects for the neighbors around
        // `atMS`. If the tap landed inside an existing effect, bail
        // (the user meant to tap that effect — hit-test catches it
        // normally, but rapid-tap edge cases can slip through).
        var prevEnd = 0
        var nextStart = Int.max
        for e in row.effects {
            if atMS >= e.startTimeMS && atMS < e.endTimeMS { return }
            if e.endTimeMS <= atMS {
                prevEnd = max(prevEnd, e.endTimeMS)
            } else if e.startTimeMS > atMS {
                nextStart = min(nextStart, e.startTimeMS)
            }
        }

        // If a timing track is active, snap the new effect's range to
        // the timing cell (the mark pair bracketing `atMS`). Desktop
        // does the same when an active timing track is selected —
        // you drop an effect and it fills the whole cell. Falls back
        // to the 1-second default when no track is active or the tap
        // lands outside any cell.
        let cell = activeTimingCell(forMS: atMS)
        let startMS: Int
        let endMS: Int
        if let cell = cell {
            startMS = max(prevEnd, cell.startMS)
            endMS = min(nextStart, cell.endMS)
        } else {
            startMS = max(prevEnd, atMS)
            let defaultLen = 1000
            endMS = min(nextStart, startMS + defaultLen)
        }
        guard endMS > startMS + 10 else { return } // too tight to fit

        let seed = seedsForNewEffect(ofType: paletteName)
        let newIdx = addEffectWithSettings(rowIndex: rowIndex,
                                            name: paletteName,
                                            settings: seed.settings, palette: seed.palette,
                                            startMS: startMS, endMS: endMS)
        // Drop-and-edit: select the freshly-placed effect so the
        // inspector opens immediately. Saves a round-trip tap the
        // user was almost certainly going to make.
        if newIdx >= 0 {
            selectEffect(rowIndex: rowIndex, effectIndex: newIdx)
        }
    }

    /// Return the timing cell (mark pair) on the active timing track
    /// that brackets `atMS`, or nil if no timing track is active or
    /// the tap lands outside every cell on the active track.
    private func activeTimingCell(forMS atMS: Int) -> (startMS: Int, endMS: Int)? {
        // First active timing track wins. Desktop enforces single-
        // active so the first hit is the only hit, but we loop
        // defensively in case the invariant breaks.
        for r in rows {
            guard let t = r.timing, t.isActive else { continue }
            // Timing marks live as effects on the timing row. Find the
            // effect whose [start, end) brackets the tap time.
            for e in r.effects {
                if atMS >= e.startTimeMS && atMS < e.endTimeMS {
                    return (e.startTimeMS, e.endTimeMS)
                }
            }
        }
        return nil
    }

    /// Add an effect with pre-populated settings/palette (used for paste + undo-of-delete).
    /// Returns the new effect's index, or -1 on failure.
    @discardableResult
    func addEffectWithSettings(rowIndex: Int, name: String,
                               settings: String, palette: String,
                               startMS: Int, endMS: Int) -> Int {
        let idx = Int(document.addEffect(toRow: Int32(rowIndex),
                                          name: name,
                                          settings: settings,
                                          palette: palette,
                                          startMS: Int32(startMS),
                                          endMS: Int32(endMS)))
        if idx < 0 { return -1 }
        renderRangeAndTrack(rowIndex: rowIndex,
                             startMS: startMS, endMS: endMS,
                             clear: false)
        reloadRows()
        undoManager.registerUndo(withTarget: self) { vm in
            vm.deleteEffect(rowIndex: rowIndex, effectIndex: idx)
        }
        undoManager.setActionName("Add Effect")
        return idx
    }

    /// Pencil-Pro-squeeze shared-edge commit. Two adjacent effects
    /// on the same row, originally butted against each other, have
    /// their shared boundary moved to a new time. Both moves land
    /// in one undo group so ⌘Z reverses the whole operation. The
    /// `moveEffect` bridge call rejects overlaps, so we order the
    /// two moves so whichever side is shrinking goes first (making
    /// room for the other to grow).
    func resizeSharedEdge(rowIndex: Int,
                          leftIndex: Int, leftStartMS: Int, leftEndMS: Int,
                          rightIndex: Int, rightStartMS: Int, rightEndMS: Int) {
        guard rowIndex >= 0, rowIndex < rows.count else { return }
        let row = rows[rowIndex]
        guard leftIndex < row.effects.count,
              rightIndex < row.effects.count else { return }
        let origLeftStart = row.effects[leftIndex].startTimeMS
        let origRightEnd = row.effects[rightIndex].endTimeMS
        let origLeftEnd = row.effects[leftIndex].endTimeMS
        let boundaryMovingRight = leftEndMS > origLeftEnd
        undoManager.beginUndoGrouping()
        if boundaryMovingRight {
            // Right effect shrinks first, then left grows into the
            // freed space. Re-find indices after each move since
            // `moveEffect` reloads rows.
            moveEffect(rowIndex: rowIndex, effectIndex: rightIndex,
                       newStartMS: rightStartMS, newEndMS: rightEndMS)
            if let newLeftIdx = rows[rowIndex].effects.firstIndex(where: {
                $0.startTimeMS == origLeftStart
            }) {
                moveEffect(rowIndex: rowIndex, effectIndex: newLeftIdx,
                           newStartMS: leftStartMS, newEndMS: leftEndMS)
            }
        } else {
            moveEffect(rowIndex: rowIndex, effectIndex: leftIndex,
                       newStartMS: leftStartMS, newEndMS: leftEndMS)
            if let newRightIdx = rows[rowIndex].effects.firstIndex(where: {
                $0.endTimeMS == origRightEnd
            }) {
                moveEffect(rowIndex: rowIndex, effectIndex: newRightIdx,
                           newStartMS: rightStartMS, newEndMS: rightEndMS)
            }
        }
        undoManager.endUndoGrouping()
        undoManager.setActionName("Resize Shared Edge")
    }

    func moveEffect(rowIndex: Int, effectIndex: Int, newStartMS: Int, newEndMS: Int) {
        guard rowIndex < rows.count, effectIndex < rows[rowIndex].effects.count else { return }
        let prev = rows[rowIndex].effects[effectIndex]
        let oldStart = prev.startTimeMS
        let oldEnd = prev.endTimeMS
        if document.moveEffect(inRow: Int32(rowIndex), at: Int32(effectIndex),
                               toStartMS: Int32(newStartMS), toEndMS: Int32(newEndMS)) {
            renderEffectAndTrack(rowIndex: rowIndex, effectIndex: effectIndex)
            reloadRows()
            if selectedEffect?.rowIndex == rowIndex && selectedEffect?.effectIndex == effectIndex {
                selectEffect(rowIndex: rowIndex, effectIndex: effectIndex)
            }
            undoManager.registerUndo(withTarget: self) { vm in
                vm.moveEffect(rowIndex: rowIndex, effectIndex: effectIndex,
                              newStartMS: oldStart, newEndMS: oldEnd)
            }
            undoManager.setActionName("Move Effect")
        }
    }

    /// Move an effect from one row to another in a single action. On
    /// success, the effect is recreated on `dstRowIndex` at the
    /// requested range with the original settings+palette intact and
    /// the selection is repointed at the new slot. Registers a single
    /// undo step that reverses both ends.
    func moveEffectToRow(srcRowIndex: Int, effectIndex: Int,
                          dstRowIndex: Int, newStartMS: Int, newEndMS: Int) {
        guard srcRowIndex != dstRowIndex else {
            moveEffect(rowIndex: srcRowIndex, effectIndex: effectIndex,
                       newStartMS: newStartMS, newEndMS: newEndMS)
            return
        }
        guard srcRowIndex < rows.count,
              effectIndex < rows[srcRowIndex].effects.count,
              dstRowIndex < rows.count else { return }

        // Snapshot the original effect so we can restore it on undo
        // and so the add-side preserves settings/palette/name/range.
        let prev = rows[srcRowIndex].effects[effectIndex]
        let name = document.effectName(forRow: Int32(srcRowIndex),
                                        at: Int32(effectIndex))
        let settings = document.effectSettingsString(
            forRow: Int32(srcRowIndex), at: Int32(effectIndex))
        let palette = document.effectPaletteString(
            forRow: Int32(srcRowIndex), at: Int32(effectIndex))
        let oldStart = prev.startTimeMS
        let oldEnd = prev.endTimeMS

        // Delete first so the target row has the original effect's
        // time range free — if the source and destination rows happen
        // to be the same this would matter, but we already bailed out
        // above. Then add on the destination row. If the add fails
        // (overlap in target row), restore the deleted effect so the
        // user isn't left with a hole.
        guard document.deleteEffect(inRow: Int32(srcRowIndex),
                                     at: Int32(effectIndex)) else { return }
        renderRangeAndTrack(rowIndex: srcRowIndex,
                             startMS: oldStart, endMS: oldEnd,
                             clear: true)

        let newIdx = Int(document.addEffect(toRow: Int32(dstRowIndex),
                                             name: name,
                                             settings: settings,
                                             palette: palette,
                                             startMS: Int32(newStartMS),
                                             endMS: Int32(newEndMS)))
        if newIdx < 0 {
            // Target-row collision: re-insert on the source row so
            // the effect isn't lost. Same-row add at the old range.
            _ = document.addEffect(toRow: Int32(srcRowIndex),
                                    name: name,
                                    settings: settings,
                                    palette: palette,
                                    startMS: Int32(oldStart),
                                    endMS: Int32(oldEnd))
            renderEffectAndTrack(rowIndex: srcRowIndex,
                                  effectIndex: effectIndex)
            reloadRows()
            return
        }

        renderEffectAndTrack(rowIndex: dstRowIndex, effectIndex: newIdx)
        reloadRows()
        selectEffect(rowIndex: dstRowIndex, effectIndex: newIdx)
        undoManager.registerUndo(withTarget: self) { vm in
            vm.moveEffectToRow(srcRowIndex: dstRowIndex,
                                effectIndex: newIdx,
                                dstRowIndex: srcRowIndex,
                                newStartMS: oldStart,
                                newEndMS: oldEnd)
        }
        undoManager.setActionName("Move Effect")
    }

    /// Commit new fade-in / fade-out seconds for an effect. Either
    /// argument == -1 leaves that value unchanged. Registers a single
    /// undo step covering both edges. Triggers a targeted re-render.
    func adjustFade(rowIndex: Int, effectIndex: Int,
                    fadeInSec: Float, fadeOutSec: Float) {
        guard rowIndex < rows.count, effectIndex < rows[rowIndex].effects.count else { return }
        let oldIn  = document.effectFadeInSeconds(forRow: Int32(rowIndex),
                                                    at: Int32(effectIndex))
        let oldOut = document.effectFadeOutSeconds(forRow: Int32(rowIndex),
                                                     at: Int32(effectIndex))
        var changed = false
        if fadeInSec >= 0, abs(fadeInSec - oldIn) > 1e-4 {
            if document.setEffectFadeInSeconds(fadeInSec,
                                                forRow: Int32(rowIndex),
                                                at: Int32(effectIndex)) {
                changed = true
            }
        }
        if fadeOutSec >= 0, abs(fadeOutSec - oldOut) > 1e-4 {
            if document.setEffectFadeOutSeconds(fadeOutSec,
                                                 forRow: Int32(rowIndex),
                                                 at: Int32(effectIndex)) {
                changed = true
            }
        }
        guard changed else { return }
        renderEffectAndTrack(rowIndex: rowIndex, effectIndex: effectIndex)
        // Don't reloadRows — fade changes don't affect rows[] geometry,
        // and we don't want to invalidate all tiles. The canvas reads
        // live fades via the fadeProvider closure, which hits the
        // document directly, so its next invalidate(xRanges:) picks up
        // the new values. Do bump the @Observable by nil-ing and
        // re-setting the selection so SwiftUI re-evaluates bodies that
        // depend on selected settings.
        undoManager.registerUndo(withTarget: self) { vm in
            vm.adjustFade(rowIndex: rowIndex, effectIndex: effectIndex,
                          fadeInSec: oldIn, fadeOutSec: oldOut)
        }
        undoManager.setActionName("Adjust Fade")
    }

    /// Resize one edge of an effect. `edge`: 0 = left/start, 1 = right/end.
    func resizeEffectEdge(rowIndex: Int, effectIndex: Int, edge: Int, newMS: Int) {
        guard rowIndex < rows.count, effectIndex < rows[rowIndex].effects.count else { return }
        let prev = rows[rowIndex].effects[effectIndex]
        let oldMS = edge == 0 ? prev.startTimeMS : prev.endTimeMS
        if document.resizeEffectEdge(inRow: Int32(rowIndex),
                                      at: Int32(effectIndex),
                                      edge: Int32(edge),
                                      toMS: Int32(newMS)) {
            renderEffectAndTrack(rowIndex: rowIndex, effectIndex: effectIndex)
            reloadRows()
            if selectedEffect?.rowIndex == rowIndex && selectedEffect?.effectIndex == effectIndex {
                selectEffect(rowIndex: rowIndex, effectIndex: effectIndex)
            }
            undoManager.registerUndo(withTarget: self) { vm in
                vm.resizeEffectEdge(rowIndex: rowIndex, effectIndex: effectIndex,
                                     edge: edge, newMS: oldMS)
            }
            undoManager.setActionName("Resize Effect")
        }
    }

    func undo() { undoManager.undo() }
    func redo() { undoManager.redo() }

    // MARK: - Clipboard (B98 multi-effect)

    /// One effect snapshot, relative to a paste anchor so a multi-
    /// effect copy preserves inter-effect timing + cross-row layout
    /// when pasted at a new cell.
    struct ClipboardEntry: Codable {
        let rowOffset: Int        // target = pasteRow + rowOffset
        let startOffsetMS: Int    // target = pasteStartMS + startOffsetMS
        let endOffsetMS: Int
        let name: String
        let settings: String
        let palette: String
    }

    /// B99 — custom UTI for effect clipboard data on `UIPasteboard`.
    /// Lowercase reverse-DNS matches Apple's convention for dynamic
    /// UTIs (no Info.plist declaration needed since nothing outside
    /// the app consumes it yet).
    static let effectClipboardUTI = "com.xlights.effectclip"

    private var clipboardEntries: [ClipboardEntry] = [] {
        didSet { publishClipboardToPasteboard() }
    }
    /// True when the in-memory buffer has entries or the system
    /// pasteboard carries xLights effect data written by another
    /// session / another device via Universal Clipboard.
    var hasClipboard: Bool {
        if !clipboardEntries.isEmpty { return true }
        return UIPasteboard.general.data(forPasteboardType: Self.effectClipboardUTI) != nil
    }

    /// Copy the current `clipboardEntries` to `UIPasteboard` so
    /// other processes / Universal Clipboard can pick them up.
    /// Clears the pasteboard entry when the local buffer empties.
    private func publishClipboardToPasteboard() {
        let pb = UIPasteboard.general
        if clipboardEntries.isEmpty {
            pb.items = pb.items.filter { !$0.keys.contains(Self.effectClipboardUTI) }
            return
        }
        do {
            let data = try JSONEncoder().encode(clipboardEntries)
            pb.setData(data, forPasteboardType: Self.effectClipboardUTI)
        } catch {
            // JSON encoding can't actually fail for these types;
            // swallow to keep the call-site noise-free.
        }
    }

    /// Inverse of `publishClipboardToPasteboard`. When the in-memory
    /// buffer is empty but the system pasteboard carries xLights
    /// data, rehydrate the local `clipboardEntries`. Called on
    /// sequence open + just-in-time from the paste paths so a fresh
    /// launch or drop-in-from-another-device surfaces the clipboard.
    @discardableResult
    func syncClipboardFromPasteboard() -> Bool {
        if !clipboardEntries.isEmpty { return true }
        let pb = UIPasteboard.general
        guard let data = pb.data(forPasteboardType: Self.effectClipboardUTI),
              let decoded = try? JSONDecoder().decode([ClipboardEntry].self, from: data),
              !decoded.isEmpty else {
            return false
        }
        // Assign directly to the backing storage so `didSet`'s re-
        // publish doesn't churn the pasteboard for a no-op round
        // trip.
        self.clipboardEntries = decoded
        return true
    }
    /// Shortcut for legacy callers: the duration of the first
    /// clipboard entry (0 if empty). Used by the inspector's
    /// "Duplicate" path which still expects a single-effect model.
    var clipboardDurationMS: Int {
        guard let first = clipboardEntries.first else { return 0 }
        return first.endOffsetMS - first.startOffsetMS
    }

    // MARK: - Arrow-key navigation

    /// Move selection to the previous effect in the same row. Wraps
    /// within the row at the ends so repeated presses cycle.
    func selectPreviousEffect() {
        guard let sel = selectedEffect,
              sel.rowIndex < rows.count else { return }
        let row = rows[sel.rowIndex]
        guard !row.effects.isEmpty else { return }
        let newIdx = sel.effectIndex > 0
            ? sel.effectIndex - 1
            : row.effects.count - 1
        selectEffect(rowIndex: sel.rowIndex, effectIndex: newIdx)
    }

    /// Move selection to the next effect in the same row. Wraps at
    /// the end.
    func selectNextEffect() {
        guard let sel = selectedEffect,
              sel.rowIndex < rows.count else { return }
        let row = rows[sel.rowIndex]
        guard !row.effects.isEmpty else { return }
        let newIdx = sel.effectIndex + 1 < row.effects.count
            ? sel.effectIndex + 1
            : 0
        selectEffect(rowIndex: sel.rowIndex, effectIndex: newIdx)
    }

    /// Move selection to the nearest model row above / below,
    /// preferring the effect whose time range most overlaps the
    /// current selection and falling back to the effect whose center
    /// is closest in time. Timing rows are skipped.
    func selectEffectAbove() { navigateRow(direction: -1) }
    func selectEffectBelow() { navigateRow(direction: 1) }

    private func navigateRow(direction: Int) {
        guard let sel = selectedEffect else { return }
        let currentRowId = sel.rowIndex
        let modelRows = rows.filter { $0.timing == nil }
        guard let curPos = modelRows.firstIndex(where: { $0.id == currentRowId })
        else { return }
        let targetPos = curPos + direction
        guard targetPos >= 0, targetPos < modelRows.count else { return }
        let targetRow = modelRows[targetPos]
        let selStart = sel.startTimeMS
        let selEnd = sel.endTimeMS
        var bestIdx: Int? = nil
        var bestOverlap: Int = 0
        var closestIdx: Int? = nil
        var closestDist: Int = Int.max
        for (i, e) in targetRow.effects.enumerated() {
            let overlap = max(0, min(selEnd, e.endTimeMS) - max(selStart, e.startTimeMS))
            if overlap > bestOverlap {
                bestOverlap = overlap
                bestIdx = i
            }
            let mid = (e.startTimeMS + e.endTimeMS) / 2
            let selMid = (selStart + selEnd) / 2
            let dist = abs(mid - selMid)
            if dist < closestDist {
                closestDist = dist
                closestIdx = i
            }
        }
        guard let newIdx = bestIdx ?? closestIdx else { return }
        selectEffect(rowIndex: targetRow.id, effectIndex: newIdx)
    }

    func copySelectedEffect() {
        guard let sel = selectedEffect else { return }
        let name = document.effectName(forRow: Int32(sel.rowIndex), at: Int32(sel.effectIndex))
        let settings = document.effectSettingsString(forRow: Int32(sel.rowIndex), at: Int32(sel.effectIndex))
        let palette = document.effectPaletteString(forRow: Int32(sel.rowIndex), at: Int32(sel.effectIndex))
        clipboardEntries = [ClipboardEntry(
            rowOffset: 0, startOffsetMS: 0,
            endOffsetMS: sel.endTimeMS - sel.startTimeMS,
            name: name, settings: settings, palette: palette)]
    }

    /// B98: copy every effect in `selectedEffects` into the clipboard,
    /// preserving relative timing + row offsets. Anchor = min row
    /// (tiebreak: min start). If only one effect is selected, falls
    /// through to the single-effect path.
    func copySelectedEffects() {
        if selectedEffects.count <= 1 {
            copySelectedEffect()
            return
        }
        // Anchor: earliest-starting, lowest-rowIndex selected effect.
        guard let anchor = selectedEffects.min(by: { a, b in
            if a.rowIndex != b.rowIndex { return a.rowIndex < b.rowIndex }
            return a.startTimeMS < b.startTimeMS
        }) else { return }
        var entries: [ClipboardEntry] = []
        for sel in selectedEffects {
            let name = document.effectName(forRow: Int32(sel.rowIndex),
                                             at: Int32(sel.effectIndex))
            let settings = document.effectSettingsString(forRow: Int32(sel.rowIndex),
                                                           at: Int32(sel.effectIndex))
            let palette = document.effectPaletteString(forRow: Int32(sel.rowIndex),
                                                         at: Int32(sel.effectIndex))
            entries.append(ClipboardEntry(
                rowOffset: sel.rowIndex - anchor.rowIndex,
                startOffsetMS: sel.startTimeMS - anchor.startTimeMS,
                endOffsetMS: sel.endTimeMS - anchor.startTimeMS,
                name: name, settings: settings, palette: palette))
        }
        clipboardEntries = entries
    }

    /// Duplicate the selected effect(s) immediately after the
    /// rightmost selection end. For single-select: paste at end of
    /// that effect. For multi-select: paste shifted forward so the
    /// anchor lands at (max-end of selection, same row as anchor).
    /// Undo-safe through the usual add path. Preserves the prior
    /// clipboard so an earlier Cmd+C isn't clobbered.
    func duplicateSelectedEffect() {
        let prevClipboard = clipboardEntries
        if selectedEffects.count > 1 {
            var maxEnd = 0
            for sel in selectedEffects { maxEnd = max(maxEnd, sel.endTimeMS) }
            copySelectedEffects()
            guard let anchor = selectedEffects.min(by: { a, b in
                if a.rowIndex != b.rowIndex { return a.rowIndex < b.rowIndex }
                return a.startTimeMS < b.startTimeMS
            }) else { return }
            pasteEffect(rowIndex: anchor.rowIndex, startMS: maxEnd)
        } else if let sel = selectedEffect {
            copySelectedEffect()
            pasteEffect(rowIndex: sel.rowIndex, startMS: sel.endTimeMS)
        }
        if !prevClipboard.isEmpty {
            clipboardEntries = prevClipboard
        }
    }

    /// Paste the clipboard onto `(rowIndex, startMS)`. Single-entry
    /// clipboards drop on the target cell; multi-entry clipboards
    /// apply each entry's `rowOffset`/`startOffsetMS` to produce the
    /// target position. Each entry goes through `addEffectWithSettings`
    /// so the bridge validates overlap per-effect; silent
    /// partial-paste on conflict.
    /// B100: true iff a paste at `(rowIndex, startMS)` would land
    /// on an existing effect. Looks at the clipboard's primary
    /// entry range (single-entry) or any of the multi-entry
    /// target ranges. Used by the UI to decide whether to surface
    /// an "overwrite?" confirmation before calling `pasteEffect`.
    func pasteWouldOverlap(rowIndex: Int, startMS: Int) -> Bool {
        guard !clipboardEntries.isEmpty,
              rowIndex >= 0, rowIndex < rows.count else { return false }
        for entry in clipboardEntries {
            let targetRow = rowIndex + entry.rowOffset
            if targetRow < 0 || targetRow >= rows.count { continue }
            if rows[targetRow].timing != nil { continue }
            let tStart = startMS + entry.startOffsetMS
            let tEnd = min(startMS + entry.endOffsetMS, sequenceDurationMS)
            guard tEnd > tStart else { continue }
            for e in rows[targetRow].effects {
                if e.startTimeMS < tEnd && e.endTimeMS > tStart {
                    return true
                }
            }
        }
        return false
    }

    /// B100: delete any effects the incoming paste would overlap,
    /// then call `pasteEffect`. Single undo group covers both the
    /// deletes and the paste so ⌘Z brings everything back. Caller
    /// should have already confirmed the overwrite with the user.
    func pasteEffectReplacingOverlaps(rowIndex: Int, startMS: Int) {
        guard !clipboardEntries.isEmpty,
              rowIndex >= 0, rowIndex < rows.count else { return }
        undoManager.beginUndoGrouping()
        for entry in clipboardEntries {
            let targetRow = rowIndex + entry.rowOffset
            if targetRow < 0 || targetRow >= rows.count { continue }
            if rows[targetRow].timing != nil { continue }
            let tStart = startMS + entry.startOffsetMS
            let tEnd = min(startMS + entry.endOffsetMS, sequenceDurationMS)
            guard tEnd > tStart else { continue }
            // Collect overlaps first (indices shift after each delete).
            // Walk high-to-low so each delete leaves lower indices
            // valid for the next iteration.
            let overlaps = rows[targetRow].effects.enumerated()
                .filter { _, e in e.startTimeMS < tEnd && e.endTimeMS > tStart }
                .map { idx, _ in idx }
                .sorted(by: >)
            for idx in overlaps {
                deleteEffect(rowIndex: targetRow, effectIndex: idx)
            }
        }
        pasteEffect(rowIndex: rowIndex, startMS: startMS)
        undoManager.endUndoGrouping()
        undoManager.setActionName(clipboardEntries.count > 1
                                   ? "Replace & Paste \(clipboardEntries.count) Effects"
                                   : "Replace & Paste Effect")
    }

    func pasteEffect(rowIndex: Int, startMS: Int) {
        // B99: pick up clipboard data written by a previous session
        // (or another device via Universal Clipboard) just before a
        // paste, so the user doesn't have to copy again post-launch.
        if clipboardEntries.isEmpty { syncClipboardFromPasteboard() }
        guard !clipboardEntries.isEmpty else { return }
        guard rowIndex >= 0 && rowIndex < rows.count else { return }
        // B14 paste-by-cell: with exactly one clipboard entry AND an
        // active timing cell bracketing `startMS`, stretch the
        // pasted effect to fill the cell rather than preserving the
        // copied duration. Matches desktop's "paste respects the
        // selected cell" semantic — the cell is defined by the
        // active timing track's marks. Multi-entry clipboards fall
        // through to the relative-offset layout (B98) since "fill
        // this cell with N effects" has no obvious extension.
        if clipboardEntries.count == 1,
           let entry = clipboardEntries.first,
           rows[rowIndex].timing == nil,
           let cell = activeTimingCell(forMS: startMS) {
            let targetStart = cell.startMS
            let targetEnd = min(cell.endMS, sequenceDurationMS)
            if targetEnd > targetStart {
                undoManager.beginUndoGrouping()
                let newIdx = addEffectWithSettings(rowIndex: rowIndex,
                                                     name: entry.name,
                                                     settings: entry.settings,
                                                     palette: entry.palette,
                                                     startMS: targetStart,
                                                     endMS: targetEnd)
                undoManager.endUndoGrouping()
                undoManager.setActionName("Paste Effect in Cell")
                // Auto-select the freshly-pasted effect so the
                // inspector opens immediately.
                if newIdx >= 0 {
                    selectEffect(rowIndex: rowIndex, effectIndex: newIdx)
                }
                return
            }
        }

        // Single-paste: select the one effect on commit. Multi-paste:
        // capture the first successful (row, index) pair and select
        // that anchor — users almost always paste intending to
        // continue editing the primary entry.
        var anchorRow: Int = -1
        var anchorIdx: Int = -1
        undoManager.beginUndoGrouping()
        for entry in clipboardEntries {
            let targetRow = rowIndex + entry.rowOffset
            if targetRow < 0 || targetRow >= rows.count { continue }
            // Skip timing rows — pasting effects there isn't meaningful.
            if rows[targetRow].timing != nil { continue }
            let targetStart = startMS + entry.startOffsetMS
            let targetEnd = min(startMS + entry.endOffsetMS, sequenceDurationMS)
            guard targetEnd > targetStart else { continue }
            let newIdx = addEffectWithSettings(rowIndex: targetRow, name: entry.name,
                                                 settings: entry.settings,
                                                 palette: entry.palette,
                                                 startMS: targetStart, endMS: targetEnd)
            if newIdx >= 0, anchorRow < 0 {
                anchorRow = targetRow
                anchorIdx = newIdx
            }
        }
        undoManager.endUndoGrouping()
        undoManager.setActionName(clipboardEntries.count > 1
                                    ? "Paste \(clipboardEntries.count) Effects"
                                    : "Paste Effect")
        if anchorRow >= 0 {
            selectEffect(rowIndex: anchorRow, effectIndex: anchorIdx)
        }
    }

    // MARK: - Lock / Disable

    func toggleLockSelected() {
        guard let sel = selectedEffect else { return }
        toggleLock(rowIndex: sel.rowIndex, effectIndex: sel.effectIndex)
    }

    func toggleLock(rowIndex: Int, effectIndex: Int) {
        let nowLocked = document.effectIsLocked(inRow: Int32(rowIndex), at: Int32(effectIndex))
        document.setEffectLocked(!nowLocked, inRow: Int32(rowIndex), at: Int32(effectIndex))
        undoManager.registerUndo(withTarget: self) { vm in
            vm.toggleLock(rowIndex: rowIndex, effectIndex: effectIndex)
        }
        undoManager.setActionName(nowLocked ? "Unlock Effect" : "Lock Effect")
        reloadRows()
    }

    func toggleDisableSelected() {
        guard let sel = selectedEffect else { return }
        toggleDisable(rowIndex: sel.rowIndex, effectIndex: sel.effectIndex)
    }

    func toggleDisable(rowIndex: Int, effectIndex: Int) {
        let nowDisabled = document.effectIsRenderDisabled(inRow: Int32(rowIndex), at: Int32(effectIndex))
        document.setEffectRenderDisabled(!nowDisabled, inRow: Int32(rowIndex), at: Int32(effectIndex))
        renderEffectAndTrack(rowIndex: rowIndex, effectIndex: effectIndex)
        undoManager.registerUndo(withTarget: self) { vm in
            vm.toggleDisable(rowIndex: rowIndex, effectIndex: effectIndex)
        }
        undoManager.setActionName(nowDisabled ? "Enable Effect" : "Disable Effect")
        reloadRows()
    }

    func isEffectLocked(rowIndex: Int, effectIndex: Int) -> Bool {
        document.effectIsLocked(inRow: Int32(rowIndex), at: Int32(effectIndex))
    }

    func isEffectRenderDisabled(rowIndex: Int, effectIndex: Int) -> Bool {
        document.effectIsRenderDisabled(inRow: Int32(rowIndex), at: Int32(effectIndex))
    }

    func selectPaletteEffect(_ name: String) {
        selectedPaletteEffect = (selectedPaletteEffect == name) ? nil : name
    }

    func loadAvailableEffects() {
        availableEffects = document.availableEffectNames().filter { !$0.isEmpty }
    }

    // MARK: - Waveform

    /// Roughly one min/max bucket per 2 pixels of rendered timeline. At
    /// `pixelsPerMS == 0.01` (start-zoomed-out) a 5-minute sequence is
    /// ~3k pixels -> ~1500 buckets; at `pixelsPerMS == 1.0` the same
    /// sequence is 300k pixels -> ~150k buckets. The bridge clamps
    /// `samplesPerBucket >= 1`, so asking for more buckets than raw
    /// audio samples is self-limiting.
    private static let waveformPixelsPerBucket: CGFloat = 2
    private static let waveformMinSamples: Int = 2000
    private static let waveformMaxSamples: Int = 400_000

    private func targetWaveformSamples(pixelsPerMS: CGFloat) -> Int {
        guard sequenceDurationMS > 0, pixelsPerMS > 0 else {
            return Self.waveformMinSamples
        }
        let width = CGFloat(sequenceDurationMS) * pixelsPerMS
        let ideal = Int((width / Self.waveformPixelsPerBucket).rounded())
        return max(Self.waveformMinSamples,
                   min(Self.waveformMaxSamples, ideal))
    }

    /// A7: run Apple's SNClassifySoundRequest over the audio track
    /// and populate `soundClasses`. Synchronous — a 3–4 minute track
    /// takes a couple of seconds on modern Apple Silicon. Idempotent.
    func classifySound() {
        guard hasAudio else { return }
        if isClassifyingSound { return }
        isClassifyingSound = true
        defer { isClassifyingSound = false }
        let dict = document.classifySound() as? [String: [NSNumber]] ?? [:]
        soundClassTimeStep = document.lastClassificationTimeStep
        var result: [String: [Float]] = [:]
        for (key, numbers) in dict {
            result[key] = numbers.map { $0.floatValue }
        }
        soundClasses = result
    }

    /// A7: linear-interpolated confidence lookup for the currently
    /// selected class at a given absolute track time (ms). Returns 1.0
    /// when no class is selected so the waveform passes through
    /// unchanged.
    func soundClassGate(atMS ms: Int) -> Float {
        guard let key = selectedSoundClass, let arr = soundClasses[key], !arr.isEmpty else {
            return 1.0
        }
        let step = max(0.001, Float(soundClassTimeStep))
        let t = Float(ms) / (step * 1000.0)
        let i0 = max(0, min(arr.count - 1, Int(floor(t))))
        let i1 = max(0, min(arr.count - 1, i0 + 1))
        let frac = t - Float(i0)
        return arr[i0] + (arr[i1] - arr[i0]) * max(0, min(1, frac))
    }

    /// A2: compute percussive onsets via the bridge's spectral-flux
    /// detector. Idempotent — skips the work if already computed for
    /// this sequence. Synchronous; 3–4 minute tracks finish in tens
    /// of milliseconds on modern iPad hardware, so the main-thread
    /// stall is imperceptible. (A `Task.detached` pass would need a
    /// Sendable bridge — cross that bridge later if profiling shows
    /// the block.)
    func computeOnsets(force: Bool = false) {
        guard hasAudio else { return }
        if isComputingOnsets { return }
        if onsetsComputed && !force { return }
        isComputingOnsets = true
        let arr = (document.detectOnsets(sensitivity: 1.5) as [NSNumber])
        onsetTimesMS = arr.map { $0.intValue }
        onsetsComputed = true
        isComputingOnsets = false
    }

    /// A2: toggle the onset-overlay on the waveform strip. First
    /// activation kicks off the detector if it hasn't run yet.
    func toggleShowOnsets() {
        showOnsets.toggle()
        if showOnsets && !onsetsComputed {
            computeOnsets()
        }
    }

    /// A9: detect chord progression + estimated key. Returns
    /// `(key, chords: [(startMS, endMS, name)])`. Synchronous —
    /// a 4-minute track finishes in well under a second.
    func detectChords() -> (key: String, chords: [(Int, Int, String)]) {
        guard hasAudio else { return ("", []) }
        let dict = document.detectChords() as? [String: Any] ?? [:]
        let key = (dict["key"] as? String) ?? ""
        let raw = (dict["chords"] as? [[String: Any]]) ?? []
        let chords: [(Int, Int, String)] = raw.compactMap { d in
            guard let s = (d["startMS"] as? NSNumber)?.intValue,
                  let e = (d["endMS"] as? NSNumber)?.intValue,
                  let n = d["name"] as? String else { return nil }
            return (s, e, n)
        }
        return (key, chords)
    }

    /// A9: create a variable timing track labelled with chord names
    /// at the detected segment boundaries. Track name carries the
    /// key so users can tell auto-generated tracks apart. Each mark's
    /// label is the chord name, matching the existing lyric / phoneme
    /// labelled-mark idiom so chord names render directly in the
    /// timeline.
    @discardableResult
    func generateChordTimingTrack() -> Int? {
        guard hasAudio else { return nil }
        let result = detectChords()
        if result.chords.isEmpty { return nil }
        let name = result.key.isEmpty
            ? "Chords"
            : "Chords in \(result.key)"
        guard document.addTimingTrackNamed(name) else { return nil }
        reloadRows()
        guard let rowIdx = rows.lastIndex(where: {
            guard let t = $0.timing else { return false }
            return t.elementName == name || t.elementName.hasPrefix(name + "_")
        }) else { return nil }
        let duration = sequenceDurationMS
        for (startMS, endMS, chordName) in result.chords {
            let s = max(0, startMS)
            let e = min(duration, endMS)
            if e <= s { continue }
            _ = document.addTimingMark(atRow: Int32(rowIdx),
                                        startMS: Int32(s),
                                        endMS: Int32(e),
                                        label: chordName)
        }
        reloadRows()
        return rowIdx
    }

    /// A8: ensure HTDemucs stem data is cached on the AudioManager
    /// for the current audio, triggering the first-run install flow
    /// if needed. `filter` is the filter the user tapped — we stash
    /// it as `stemsPendingFilter` so the UI can switch to it once
    /// separation finishes. Returns immediately after phase
    /// transition; subsequent callbacks drive the rest of the flow.
    func prepareStems(for filter: WaveformFilter) {
        guard hasAudio else { return }
        stemsPendingFilter = filter
        if stemsAvailable {
            applyPendingStemFilter()
            return
        }
        if stemsPhase != .idle { return }
        if let existing = document.findInstalledStemModelPath() {
            runStemSeparation(with: existing)
        } else {
            stemsPhase = .pickingRoot
        }
    }

    /// A8: user picked an install root in the sheet — kick off the
    /// download.
    func commitStemsInstall(toRoot root: String) {
        stemsInstallRoot = root
        stemsPhase = .downloading
        stemsProgressPct = 0
        let doc = document
        doc.installStemModel(toRoot: root,
                              progress: { [weak self] pct in
            // Bridge documents these callbacks fire on the main queue.
            MainActor.assumeIsolated {
                self?.stemsProgressPct = Int(pct)
            }
        },
                              completion: { [weak self] installedPath in
            MainActor.assumeIsolated {
                guard let self = self else { return }
                if let path = installedPath, !path.isEmpty {
                    self.runStemSeparation(with: path)
                } else {
                    self.stemsPhase = .idle
                    self.stemsPendingFilter = nil
                    self.stemsInstallRoot = nil
                }
            }
        })
    }

    /// A8: user cancelled the install picker.
    func cancelStemsInstall() {
        stemsPhase = .idle
        stemsPendingFilter = nil
        stemsInstallRoot = nil
        stemsProgressPct = 0
    }

    /// A8: kicks off stem separation. The bridge dispatches CoreML
    /// to a background queue internally; all callbacks here fire on
    /// the main queue, so we stay on the main actor.
    private func runStemSeparation(with modelPath: String) {
        stemsPhase = .separating
        stemsProgressPct = 0
        document.runStemSeparation(atPath: modelPath,
                                    progress: { [weak self] pct in
            MainActor.assumeIsolated {
                self?.stemsProgressPct = Int(pct)
            }
        },
                                    completion: { [weak self] ok in
            MainActor.assumeIsolated {
                guard let self = self else { return }
                if ok {
                    self.stemsAvailable = true
                    self.stemsPhase = .ready
                    self.applyPendingStemFilter()
                } else {
                    self.stemsPhase = .idle
                    self.stemsPendingFilter = nil
                }
            }
        })
    }

    private func applyPendingStemFilter() {
        guard let f = stemsPendingFilter else { return }
        stemsPendingFilter = nil
        waveformFilter = f
        // Reset the phase ribbon once the user sees the stem.
        stemsPhase = .idle
    }

    /// A6: toggle spectrogram view mode. First activation kicks the
    /// bridge's STFT so subsequent redraws just resample the cached
    /// magnitude buffer into a viewport-sized BGRA.
    func toggleShowSpectrogram() {
        showSpectrogram.toggle()
        if showSpectrogram && !spectrogramReady {
            spectrogramReady = document.ensureSpectrogramComputed()
            if !spectrogramReady {
                showSpectrogram = false
            }
        }
    }

    /// A6: fetch a rendered BGRA spectrogram image for the given
    /// time range at the given pixel size. Returns nil if the bridge
    /// doesn't have a spectrogram cached.
    func spectrogramBGRA(fromMS: Int, toMS: Int, width: Int, height: Int) -> Data? {
        guard spectrogramReady else { return nil }
        return document.spectrogramBGRA(fromMS: fromMS, toMS: toMS,
                                         width: Int32(width), height: Int32(height)) as Data?
    }

    /// A5: compute pitch contour via the bridge's FFT-ACF detector.
    /// Synchronous; a 4-minute track takes a second or so. Idempotent.
    func computePitchContour(force: Bool = false) {
        guard hasAudio else { return }
        if isComputingPitch { return }
        if pitchComputed && !force { return }
        isComputingPitch = true
        defer { isComputingPitch = false }
        guard let data = document.detectPitchContour() else {
            pitchContour = []
            pitchComputed = true
            return
        }
        let count = data.count / MemoryLayout<Float>.size
        var floats = [Float](repeating: 0, count: count)
        floats.withUnsafeMutableBufferPointer { buf in
            _ = data.copyBytes(to: buf)
        }
        pitchContour = floats
        pitchComputed = true
    }

    /// A5: toggle the pitch overlay. First activation computes the
    /// contour if needed.
    func toggleShowPitchContour() {
        showPitchContour.toggle()
        if showPitchContour && !pitchComputed {
            computePitchContour()
        }
    }

    /// A4: detect tempo + beat positions. Returns (bpm, confidence,
    /// beatMS). Synchronous; autocorrelation over a 4-minute track is
    /// comfortably sub-second on modern iPad hardware.
    func detectTempo() -> (bpm: Float, confidence: Float, beats: [Int]) {
        guard hasAudio else { return (0, 0, []) }
        let dict = document.detectTempo() as? [String: Any] ?? [:]
        let bpm = (dict["bpm"] as? NSNumber)?.floatValue ?? 0
        let conf = (dict["confidence"] as? NSNumber)?.floatValue ?? 0
        let beats = (dict["beats"] as? [NSNumber])?.map { $0.intValue } ?? []
        return (bpm, conf, beats)
    }

    /// A4: create a fixed timing track populated with marks at each
    /// detected beat. Track name carries the detected BPM so users
    /// can tell auto-generated tracks from hand-placed ones at a
    /// glance. Each mark is 20 ms wide (matches the onset track).
    @discardableResult
    func generateTempoTimingTrack() -> Int? {
        guard hasAudio else { return nil }
        let result = detectTempo()
        if result.beats.isEmpty { return nil }
        let name = "Tempo (\(Int(result.bpm.rounded())) BPM)"
        guard document.addTimingTrackNamed(name) else { return nil }
        reloadRows()
        guard let rowIdx = rows.lastIndex(where: {
            guard let t = $0.timing else { return false }
            return t.elementName == name || t.elementName.hasPrefix(name + "_")
        }) else { return nil }
        let duration = sequenceDurationMS
        // Back-to-back marks: each beat region runs to the next beat
        // (last one to track end) so effects can snap to the full
        // bar segment.
        for (i, ms) in result.beats.enumerated() {
            let start = max(0, ms)
            let end = i + 1 < result.beats.count
                ? min(duration, result.beats[i + 1])
                : duration
            if end <= start { continue }
            _ = document.addTimingMark(atRow: Int32(rowIdx),
                                        startMS: Int32(start),
                                        endMS: Int32(end),
                                        label: "")
        }
        reloadRows()
        return rowIdx
    }

    /// A2: create a new variable timing track populated with marks at
    /// each detected onset. Marks are labelled empty (plain timing
    /// marks, not lyric words) and given a 20 ms nominal width so the
    /// existing overlap check doesn't reject adjacent onsets. Returns
    /// the row index of the new track, or nil if nothing was created.
    @discardableResult
    func generateTimingTrackFromOnsets(name: String = "Onsets") -> Int? {
        guard hasAudio else { return nil }
        if !onsetsComputed {
            // Synchronous fallback — the user just asked for this and
            // expects marks to appear. Still fast on modern iPads.
            let arr = (document.detectOnsets(sensitivity: 1.5) as [NSNumber])
            onsetTimesMS = arr.map { $0.intValue }
            onsetsComputed = true
        }
        if onsetTimesMS.isEmpty { return nil }
        guard document.addTimingTrackNamed(name) else { return nil }
        reloadRows()
        // The bridge's addTimingTrackNamed auto-uniquifies the name
        // on collision. Look up by prefix match, preferring the last
        // row that matches — the new track is the most recent one.
        guard let rowIdx = rows.lastIndex(where: {
            guard let t = $0.timing else { return false }
            return t.elementName == name || t.elementName.hasPrefix(name + "_")
        }) else { return nil }
        let duration = sequenceDurationMS
        // Back-to-back marks: each region runs from one onset to the
        // next (last one to track end).
        for (i, ms) in onsetTimesMS.enumerated() {
            let start = max(0, ms)
            let end = i + 1 < onsetTimesMS.count
                ? min(duration, onsetTimesMS[i + 1])
                : duration
            if end <= start { continue }
            _ = document.addTimingMark(atRow: Int32(rowIdx),
                                        startMS: Int32(start),
                                        endMS: Int32(end),
                                        label: "")
        }
        reloadRows()
        return rowIdx
    }

    func loadWaveform(startMS: Int, endMS: Int, numSamples: Int = 2000) {
        guard hasAudio else { return }
        guard let data = document.waveformData(fromMS: Int(startMS),
                                                toMS: Int(endMS),
                                                numSamples: Int32(numSamples),
                                                filterType: Int32(waveformFilter.rawValue),
                                                lowNote: Int32(customBandLowNote),
                                                highNote: Int32(customBandHighNote)) else { return }

        let count = data.count / MemoryLayout<Float>.size
        var floats = [Float](repeating: 0, count: count)
        floats.withUnsafeMutableBufferPointer { buf in
            _ = data.copyBytes(to: buf)
        }
        // A7: if a sound class is selected, multiply each bucket's
        // min/max/rms triplet by the class's confidence at the bucket
        // midpoint so the waveform visually collapses where the
        // selected class isn't present.
        if selectedSoundClass != nil, !soundClasses.isEmpty {
            let buckets = count / 3
            if buckets > 0 {
                let rangeMS = endMS - startMS
                for i in 0..<buckets {
                    let midMS = startMS + rangeMS * i / buckets
                    let gain = soundClassGate(atMS: midMS)
                    floats[i*3]   *= gain
                    floats[i*3+1] *= gain
                    floats[i*3+2] *= gain
                }
            }
        }
        waveformPeaks = floats
        waveformStartMS = startMS
        waveformEndMS = endMS
        waveformSampleCount = numSamples
    }

    /// B41: re-read the waveform for the currently-cached range +
    /// sample count, using whatever `waveformFilter` is now. Used
    /// by the `didSet` observer on the filter property.
    private func reloadWaveformCurrent() {
        guard hasAudio, waveformSampleCount > 0 else { return }
        let startMS = waveformStartMS
        let endMS = waveformEndMS > 0 ? waveformEndMS : sequenceDurationMS
        loadWaveform(startMS: startMS, endMS: endMS,
                      numSamples: waveformSampleCount)
        syncPlaybackToWaveformFilter()
    }

    /// B43: refresh `altAudioTrackNames` and `activeWaveformTrack`
    /// from the bridge. Called after every sequence load.
    func reloadAltTracks() {
        let count = Int(document.altTrackCount())
        altAudioTrackNames = (0..<count).map {
            document.altTrackDisplayName(at: $0)
        }
        activeWaveformTrack = Int(document.activeWaveformTrack())
        if activeWaveformTrack >= count { activeWaveformTrack = -1 }
    }

    /// B43: switch the waveform display source. -1 = main; 0..N-1 =
    /// alt track. Reloads the waveform peaks for the current view
    /// range. Playback is unaffected.
    func setActiveWaveformTrack(_ index: Int) {
        guard hasAudio else { return }
        let clamped = (index < -1 || index >= altAudioTrackNames.count) ? -1 : index
        document.setActiveWaveformTrack(clamped)
        activeWaveformTrack = clamped
        reloadWaveformCurrent()
    }

    /// Route the current `waveformFilter` through `AudioManager::
    /// SwitchTo` so playback plays the filtered / stem signal, not
    /// the raw audio. The bridge internally dispatches SwitchTo to
    /// a global queue, so this call returns immediately and the
    /// filter-cache build + PCM memcpy run in the background.
    private func syncPlaybackToWaveformFilter() {
        guard hasAudio else { return }
        document.applyPlaybackFilter(
            type: Int32(waveformFilter.rawValue),
            lowNote: Int32(customBandLowNote),
            highNote: Int32(customBandHighNote))
    }

    /// Re-sample the waveform when zoom changes enough that the current
    /// peak array no longer gives ~1 bucket per couple of pixels. Runs
    /// debounced so a continuous pinch gesture only does a single reload
    /// at the end.
    func refreshWaveformForZoom(pixelsPerMS: CGFloat) {
        guard hasAudio, isSequenceLoaded else { return }
        let target = targetWaveformSamples(pixelsPerMS: pixelsPerMS)
        // Skip if the current buffer is already within 1.5x of the target
        // in either direction -- the polygon will still look fine after
        // the x-scale stretch.
        if waveformSampleCount > 0 {
            let ratio = Double(target) / Double(max(1, waveformSampleCount))
            if ratio > 0.66 && ratio < 1.5 { return }
        }
        waveformReloadTask?.cancel()
        let startMS = waveformStartMS
        let endMS = waveformEndMS > 0 ? waveformEndMS : sequenceDurationMS
        waveformReloadTask = Task { @MainActor [weak self] in
            try? await Task.sleep(nanoseconds: 120_000_000) // 120ms debounce
            guard let self, !Task.isCancelled else { return }
            self.loadWaveform(startMS: startMS, endMS: endMS, numSamples: target)
        }
    }

    // MARK: - Rows

    func reloadRows() {
        var newRows: [RowInfo] = []
        let count = Int(document.visibleRowCount())
        let timingIdxSet: Set<Int> = Set(
            (document.timingRowIndices() as [NSNumber]).map { $0.intValue }
        )

        for i in 0..<count {
            let idx = Int32(i)
            let rawDisplayName = document.rowDisplayName(at: idx)
            let layerIndex = Int(document.rowLayerIndex(at: idx))
            let isCollapsed = document.rowIsCollapsed(at: idx)

            let effectNames = document.effectNames(forRow: idx)
            let effectStarts = document.effectStartTimes(forRow: idx)
            let effectEnds = document.effectEndTimes(forRow: idx)

            var effects: [EffectInfo] = []
            for j in 0..<effectNames.count {
                let name = effectNames[j]
                let start = effectStarts[j].intValue
                let end = effectEnds[j].intValue
                effects.append(EffectInfo(id: j, name: name, startTimeMS: start, endTimeMS: end))
            }

            let timingInfo: TimingRowInfo?
            let displayName: String
            if timingIdxSet.contains(i) {
                let elementName = document.timingRowElementName(at: idx)
                let layerName = document.rowLayerName(at: idx)
                timingInfo = TimingRowInfo(
                    colorIndex: Int(document.timingRowColorIndex(at: idx)),
                    elementName: elementName,
                    layerName: layerName,
                    isActive: document.timingRowIsActive(at: idx)
                )
                // Timing rows don't always populate displayName (multi-layer
                // non-collapsed rows leave it blank on the C++ side), so
                // fall back to the element name for layer 0 and a layer
                // label for subsequent layers.
                if layerIndex == 0 {
                    displayName = rawDisplayName.isEmpty ? elementName : rawDisplayName
                } else if !layerName.isEmpty {
                    displayName = "[\(layerIndex + 1)] \(layerName)"
                } else {
                    displayName = "[\(layerIndex + 1)]"
                }
            } else {
                timingInfo = nil
                // Strand / node rows may have no assigned name; mirror
                // desktop's "Strand N" / "Node N" fallback (RowHeading.cpp:
                // 2035-2039) so the header reads sensibly instead of blank.
                let strandIndex = Int(document.rowStrandIndex(at: idx))
                let nodeIndex = Int(document.rowNodeIndex(at: idx))
                if rawDisplayName.isEmpty && nodeIndex >= 0 {
                    displayName = "Node \(nodeIndex + 1)"
                } else if rawDisplayName.isEmpty && strandIndex >= 0 {
                    displayName = "Strand \(strandIndex + 1)"
                } else {
                    displayName = rawDisplayName
                }
            }

            newRows.append(RowInfo(
                id: i,
                displayName: displayName,
                layerIndex: layerIndex,
                isCollapsed: isCollapsed,
                effects: effects,
                timing: timingInfo,
                isSubmodel: document.rowIsSubmodel(at: idx),
                nestDepth: Int(document.rowNestDepth(at: idx)),
                strandIndex: Int(document.rowStrandIndex(at: idx)),
                nodeIndex: Int(document.rowNodeIndex(at: idx))
            ))
        }

        rows = newRows
    }
}

/// A single .xsq surfaced by the picker / batch render scanners.
/// `relativePath` is the path under the show folder, with forward slashes
/// (e.g. "Halloween 2026/skeleton.xsq"); `parentRelativePath` is just the
/// directory portion ("Halloween 2026", or "" for sequences at the root).
struct SequenceEntry: Hashable, Identifiable {
    let fullPath: String
    let relativePath: String
    var id: String { fullPath }
    var displayName: String {
        (relativePath as NSString).lastPathComponent
    }
    var parentRelativePath: String {
        let parent = (relativePath as NSString).deletingLastPathComponent
        return parent
    }
}

/// Recursive show-folder walker shared by the picker and the batch render
/// sheet. Skips directories named "Backup" (case-insensitive) and any
/// dot-folder so iOS / iCloud / xLights metadata doesn't clutter the list.
/// Returns entries sorted by relative path so subfolder mates cluster.
enum SequenceScanner {
    static func scan(showFolder: String) -> [SequenceEntry] {
        guard !showFolder.isEmpty else { return [] }
        let root = URL(fileURLWithPath: showFolder)
        let fm = FileManager.default
        var results: [SequenceEntry] = []
        var stack: [URL] = [root]
        while let dir = stack.popLast() {
            guard let children = try? fm.contentsOfDirectory(
                at: dir,
                includingPropertiesForKeys: [.isDirectoryKey],
                options: [.skipsHiddenFiles]
            ) else { continue }
            for child in children {
                let name = child.lastPathComponent
                if name.hasPrefix(".") { continue }
                let isDir = (try? child.resourceValues(forKeys: [.isDirectoryKey]).isDirectory) ?? false
                if isDir {
                    if name.caseInsensitiveCompare("Backup") == .orderedSame { continue }
                    stack.append(child)
                } else if name.lowercased().hasSuffix(".xsq") {
                    let rel = relativePath(of: child, under: root)
                    results.append(SequenceEntry(fullPath: child.path, relativePath: rel))
                }
            }
        }
        results.sort { $0.relativePath.localizedStandardCompare($1.relativePath) == .orderedAscending }
        return results
    }

    private static func relativePath(of file: URL, under root: URL) -> String {
        let rootComps = root.standardizedFileURL.pathComponents
        let fileComps = file.standardizedFileURL.pathComponents
        guard fileComps.count > rootComps.count,
              Array(fileComps.prefix(rootComps.count)) == rootComps else {
            return file.lastPathComponent
        }
        return fileComps.dropFirst(rootComps.count).joined(separator: "/")
    }
}
