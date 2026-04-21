import SwiftUI
import UIKit

@Observable @MainActor
class SequencerViewModel {
    var document = XLSequenceDocument()
    @ObservationIgnored let undoManager = UndoManager()
    var showFolderPath: String?
    var mediaFolderPaths: [String] = []
    var sequenceName: String?
    var isShowFolderLoaded = false
    var isSequenceLoaded = false
    var sequenceDurationMS: Int = 0
    var frameIntervalMS: Int = 50
    var rows: [RowInfo] = []
    var sequenceFiles: [String] = []

    // Audio state
    var hasAudio = false
    var isPlaying = false
    var isPaused = false
    var playPositionMS: Int = 0
    var volume: Int = 100
    // True while a selection-scoped preview loop is advancing
    // `playPositionMS`. Observed by the preview panes so they keep their
    // MTKView display link running during scrub (not just real playback).
    var isScrubbing = false

    // Waveform cache
    var waveformPeaks: [Float] = []
    var waveformStartMS: Int = 0
    var waveformEndMS: Int = 0
    // Peak count used to build the current `waveformPeaks`. Compared against
    // the target-for-current-zoom to decide whether to re-sample.
    @ObservationIgnored private var waveformSampleCount: Int = 0
    // Debounce task for zoom-driven waveform re-sampling so rapid pinches
    // don't swamp the main actor with redundant bridge calls.
    @ObservationIgnored private var waveformReloadTask: Task<Void, Never>?

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
    var selectedEffectSettings: [String: String] = [:]     // legacy: raw key/value map
    var availableEffects: [String] = []
    var selectedPaletteEffect: String?
    var showInspector = false

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
    private static let memoryWarningThresholdMB: Int64 = 256
    private static let memoryRecoveredThresholdMB: Int64 = 384  // hysteresis
    private var memoryPressureSource: DispatchSourceMemoryPressure?
    private var memoryPollTimer: Timer?
    private var memoryWarningObserver: NSObjectProtocol?

    struct EffectSelection: Equatable {
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
    func loadShowFolder(path: String, mediaFolders: [String]) {
        showFolderPath = path
        mediaFolderPaths = mediaFolders
        isShowFolderLoaded = document.loadShowFolder(path, mediaFolders: mediaFolders)
        if isShowFolderLoaded {
            scanForSequenceFiles(at: path)
        }
    }

    /// Attempt to load the persisted show folder at app startup.
    /// Returns true if a show folder was configured and loaded successfully.
    @discardableResult
    func restorePersistedShowFolder() -> Bool {
        guard let path = FolderConfig.showFolder else { return false }
        let mediaFolders = FolderConfig.mediaFolders
        loadShowFolder(path: path, mediaFolders: mediaFolders)
        return isShowFolderLoaded
    }

    // MARK: - Sequence

    func openSequence(path: String) {
        if document.openSequence(path) {
            isSequenceLoaded = true
            sequenceName = document.sequenceName()
            sequenceDurationMS = Int(document.sequenceDurationMS())
            frameIntervalMS = Int(document.frameIntervalMS())
            hasAudio = document.hasAudio()
            reloadRows()
            loadAvailableEffects()
            // Initial load uses a modest sample count; grid re-requests
            // a higher-resolution waveform once it knows the zoom level
            // via `refreshWaveformForZoom`.
            loadWaveform(startMS: 0, endMS: sequenceDurationMS)
            // Kick off background render so SequenceData is populated
            startBackgroundRender()
        }
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
        cancelBackgroundRender()
        // Wait for any background render jobs to exit before tearing
        // down SequenceElements / SequenceData — the render workers
        // hold pointers into those structures and would crash on next
        // frame access if we proceeded with close while they're busy.
        _ = document.abortRenderAndWait(5.0)
        document.closeSequence()
        isSequenceLoaded = false
        isRenderDone = false
        isRendering = false
        sequenceName = nil
        hasAudio = false
        rows = []
        waveformPeaks = []
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
            self.memoryWarning = true
        }
        source.activate()
        memoryPressureSource = source

        memoryWarningObserver = NotificationCenter.default.addObserver(
            forName: UIApplication.didReceiveMemoryWarningNotification,
            object: nil,
            queue: .main
        ) { [weak self] _ in
            self?.document.handleMemoryWarning()
            self?.memoryWarning = true
        }

        memoryPollTimer = Timer.scheduledTimer(withTimeInterval: 5.0, repeats: true) { [weak self] _ in
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

    func stopPlayback() {
        stop()
    }

    func setVolume(_ vol: Int) {
        volume = max(0, min(100, vol))
        document.setAudioVolume(Int32(volume))
    }

    // MARK: - Playback Timer

    private func startPlaybackTimer() {
        stopPlaybackTimer()
        let interval = Double(frameIntervalMS) / 1000.0
        playbackTimer = Timer.scheduledTimer(withTimeInterval: interval, repeats: true) { [weak self] _ in
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
            } else {
                // Timer-driven: use wall clock elapsed since play started
                let elapsedMS = Int((CFAbsoluteTimeGetCurrent() - self.playbackStartTime) * 1000.0)
                let pos = self.playbackStartMS + elapsedMS
                if pos >= self.sequenceDurationMS {
                    self.playPositionMS = self.sequenceDurationMS
                    self.isPlaying = false
                    self.isPaused = false
                    self.stopPlaybackTimer()
                    return
                }
                self.playPositionMS = pos
            }

            self.sendOutputFrame()
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
            guard let self else { return }
            let next = self.playPositionMS + self.frameIntervalMS
            self.playPositionMS = (next >= self.scrubEndMS) ? self.scrubStartMS : next
        }
    }

    func stopScrub() {
        scrubTimer?.invalidate()
        scrubTimer = nil
        isScrubbing = false
    }

    // MARK: - Background Rendering

    func startBackgroundRender() {
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
            if doc.isRenderDone() {
                timer.invalidate()
                self?.renderPollTimer = nil
                self?.isRendering = false
                self?.isRenderDone = true
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
        let name = document.rowModelName(at: Int32(rowIndex)) ?? ""
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
        let name = document.rowModelName(at: Int32(rowIndex)) ?? ""
        if name.isEmpty { return }
        previewModelName = name
    }

    // MARK: - Controller Output

    func toggleOutput() {
        if isOutputting {
            document.stopOutput()
            isOutputting = false
        } else {
            isOutputting = document.startOutput()
        }
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
        selectedEffect = EffectSelection(
            rowIndex: rowIndex,
            effectIndex: effectIndex,
            name: effect.name,
            startTimeMS: effect.startTimeMS,
            endTimeMS: effect.endTimeMS
        )
        showInspector = true
        // Route the Model Preview to this effect's model so the scrub
        // loop below is visible in the pane.
        setPreviewModel(rowIndex: rowIndex)
        startScrub(startMS: effect.startTimeMS, endMS: effect.endTimeMS)

        // Merge settings map (E_/B_/T_) and palette map (C_) into a single
        // observed dictionary so the SwiftUI controls re-render on change.
        var merged: [String: String] = [:]
        if let settings = document.effectSettings(forRow: Int32(rowIndex), at: Int32(effectIndex)) as? [String: String] {
            for (k, v) in settings { merged[k] = v }
        }
        if let palette = document.effectPalette(forRow: Int32(rowIndex), at: Int32(effectIndex)) as? [String: String] {
            for (k, v) in palette { merged[k] = v }
        }
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
        if let settings = document.effectSettings(forRow: Int32(sel.rowIndex),
                                                   at: Int32(sel.effectIndex)) as? [String: String] {
            for (k, v) in settings { merged[k] = v }
        }
        if let palette = document.effectPalette(forRow: Int32(sel.rowIndex),
                                                 at: Int32(sel.effectIndex)) as? [String: String] {
            for (k, v) in palette { merged[k] = v }
        }
        selectedEffectSettings = merged
        renderEffectAndTrack(rowIndex: sel.rowIndex, effectIndex: sel.effectIndex)
    }

    // MARK: - Metadata Loading

    private func loadEffectMetadata(_ effectName: String) -> EffectMetadata? {
        if let cached = metadataCache[effectName] { return cached }
        let json = document.metadataJson(forEffectNamed: effectName) ?? ""
        guard let md = parseEffectMetadata(json) else { return nil }
        metadataCache[effectName] = md
        return md
    }

    private func loadSharedMetadata(_ name: String) -> EffectMetadata? {
        let json = document.sharedMetadataJsonNamed(name) ?? ""
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
            return (document.timingTrackNames() as? [String]) ?? []
        case "lyricTimingTracks":
            return (document.lyricTimingTrackNames() as? [String]) ?? []
        case "cameras":
            // PerPreviewCamera: "2D" plus every 3D camera defined in
            // the show's ViewpointMgr. Populated by Phase D-3's
            // ViewpointMgr bridging at show-load time.
            return (document.perPreviewCameraNames() as? [String]) ?? []
        default:
            break
        }

        guard let sel = selectedEffect else { return [] }
        let row = Int32(sel.rowIndex)
        let idx = Int32(sel.effectIndex)
        switch source {
        case "states":
            return (document.states(forRow: row, at: idx) as? [String]) ?? []
        case "faces":
            return (document.faces(forRow: row, at: idx) as? [String]) ?? []
        case "modelNodeNames":
            return (document.modelNodeNames(forRow: row, at: idx) as? [String]) ?? []
        case "effect":
            return (document.effectSettingOptions(forRow: row,
                                                   at: idx,
                                                   settingId: propertyId)
                    as? [String]) ?? []
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
                                                at: Int32(sel.effectIndex)) ?? ""
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

    /// Undo-redirect entry: writes a setting on a specific effect by
    /// row/index rather than via the current selection. Used as the
    /// undo target for `setSettingValue` so the change still applies
    /// if the user has deselected between change and undo.
    private func setSettingValueAt(rowIndex: Int, effectIndex: Int,
                                    key: String, value: String) {
        let prev = document.effectSettingValue(forKey: key,
                                                inRow: Int32(rowIndex),
                                                at: Int32(effectIndex)) ?? ""
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

    /// Delete a specific effect, capturing its state for undo.
    func deleteEffect(rowIndex: Int, effectIndex: Int) {
        guard rowIndex < rows.count, effectIndex < rows[rowIndex].effects.count else { return }
        let prev = rows[rowIndex].effects[effectIndex]
        let name = document.effectName(forRow: Int32(rowIndex), at: Int32(effectIndex)) ?? prev.name
        let settings = document.effectSettingsString(forRow: Int32(rowIndex), at: Int32(effectIndex)) ?? ""
        let palette = document.effectPaletteString(forRow: Int32(rowIndex), at: Int32(effectIndex)) ?? ""
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
        addEffectWithSettings(rowIndex: rowIndex, name: name,
                               settings: "", palette: "",
                               startMS: startMS, endMS: endMS)
    }

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

        addEffectWithSettings(rowIndex: rowIndex,
                               name: paletteName,
                               settings: "", palette: "",
                               startMS: startMS, endMS: endMS)
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
                                        at: Int32(effectIndex)) ?? prev.name
        let settings = document.effectSettingsString(
            forRow: Int32(srcRowIndex), at: Int32(effectIndex)) ?? ""
        let palette = document.effectPaletteString(
            forRow: Int32(srcRowIndex), at: Int32(effectIndex)) ?? ""
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

    // MARK: - Clipboard

    struct EffectClipboard {
        let name: String
        let settings: String
        let palette: String
        let durationMS: Int
    }

    private var clipboard: EffectClipboard?
    var hasClipboard: Bool { clipboard != nil }

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
        let name = document.effectName(forRow: Int32(sel.rowIndex), at: Int32(sel.effectIndex)) ?? sel.name
        let settings = document.effectSettingsString(forRow: Int32(sel.rowIndex), at: Int32(sel.effectIndex)) ?? ""
        let palette = document.effectPaletteString(forRow: Int32(sel.rowIndex), at: Int32(sel.effectIndex)) ?? ""
        clipboard = EffectClipboard(name: name, settings: settings, palette: palette,
                                    durationMS: sel.endTimeMS - sel.startTimeMS)
    }

    /// Duplicate the selected effect immediately after itself on the
    /// same row (paste at sel.endTimeMS). Quiet no-op if there isn't
    /// enough room before the next effect — the bridge's add path
    /// rejects the overlap.
    func duplicateSelectedEffect() {
        guard let sel = selectedEffect else { return }
        let prevClipboard = clipboard
        copySelectedEffect()
        pasteEffect(rowIndex: sel.rowIndex, startMS: sel.endTimeMS)
        // Restore the clipboard so duplicate doesn't stomp whatever
        // the user had on the Paste Here buffer from Cmd+C earlier.
        if let prev = prevClipboard {
            clipboard = prev
        }
    }

    /// Paste the clipboard onto `rowIndex` starting at `startMS`. Preserves the
    /// copied duration.
    func pasteEffect(rowIndex: Int, startMS: Int) {
        guard let clip = clipboard else { return }
        guard rowIndex >= 0 && rowIndex < rows.count else { return }
        let endMS = min(startMS + clip.durationMS, sequenceDurationMS)
        let idx = addEffectWithSettings(rowIndex: rowIndex, name: clip.name,
                                         settings: clip.settings, palette: clip.palette,
                                         startMS: startMS, endMS: endMS)
        if idx >= 0 {
            undoManager.setActionName("Paste Effect")
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
        if let names = document.availableEffectNames() as? [String] {
            availableEffects = names.filter { !$0.isEmpty }
        }
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

    func loadWaveform(startMS: Int, endMS: Int, numSamples: Int = 2000) {
        guard hasAudio else { return }
        guard let data = document.waveformData(fromMS: Int(startMS),
                                                toMS: Int(endMS),
                                                numSamples: Int32(numSamples)) else { return }

        let count = data.count / MemoryLayout<Float>.size
        var floats = [Float](repeating: 0, count: count)
        floats.withUnsafeMutableBufferPointer { buf in
            _ = data.copyBytes(to: buf)
        }
        waveformPeaks = floats
        waveformStartMS = startMS
        waveformEndMS = endMS
        waveformSampleCount = numSamples
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
            let rawDisplayName = document.rowDisplayName(at: idx) ?? ""
            let layerIndex = Int(document.rowLayerIndex(at: idx))
            let isCollapsed = document.rowIsCollapsed(at: idx)

            let effectNames = document.effectNames(forRow: idx) ?? []
            let effectStarts = document.effectStartTimes(forRow: idx) ?? []
            let effectEnds = document.effectEndTimes(forRow: idx) ?? []

            var effects: [EffectInfo] = []
            for j in 0..<effectNames.count {
                let name = effectNames[j] as? String ?? ""
                let start = (effectStarts[j] as? NSNumber)?.intValue ?? 0
                let end = (effectEnds[j] as? NSNumber)?.intValue ?? 0
                effects.append(EffectInfo(id: j, name: name, startTimeMS: start, endTimeMS: end))
            }

            let timingInfo: TimingRowInfo?
            let displayName: String
            if timingIdxSet.contains(i) {
                let elementName = document.timingRowElementName(at: idx) ?? ""
                let layerName = document.rowLayerName(at: idx) ?? ""
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

    private func scanForSequenceFiles(at path: String) {
        sequenceFiles = []
        let fm = FileManager.default
        guard let items = try? fm.contentsOfDirectory(atPath: path) else { return }
        sequenceFiles = items
            .filter { $0.hasSuffix(".xsq") }
            .sorted()
    }
}
