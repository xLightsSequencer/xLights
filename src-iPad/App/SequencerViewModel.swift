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

    // Metadata for the currently selected effect and shared panels
    var selectedEffectMetadata: EffectMetadata?
    var bufferMetadata: EffectMetadata?
    var colorMetadata: EffectMetadata?
    var timingMetadata: EffectMetadata?

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
    }

    struct TimingRowInfo: Equatable {
        /// Desktop ColorManager maps colorIndex%5 → cyan/red/green/blue/yellow.
        let colorIndex: Int
        let elementName: String
        /// e.g. "Phrases", "Words", "Phonemes" for lyric layers.
        let layerName: String
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

    func closeSequence() {
        if isOutputting { toggleOutput() }
        stopPlayback()
        cancelBackgroundRender()
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
        if timingMetadata == nil { timingMetadata = loadSharedMetadata("Timing") }
    }

    func clearSelection() {
        selectedEffect = nil
        selectedEffectSettings = [:]
        selectedEffectMetadata = nil
        stopScrub()
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

    /// Write a single setting value and trigger a re-render of the affected
    /// range if the value actually changed.
    func setSettingValue(_ value: String, forKey key: String) {
        guard let sel = selectedEffect else { return }
        let changed = document.setEffectSettingValue(value,
                                                      forKey: key,
                                                      inRow: Int32(sel.rowIndex),
                                                      at: Int32(sel.effectIndex))
        if changed {
            selectedEffectSettings[key] = value
            document.renderEffect(forRow: Int32(sel.rowIndex), at: Int32(sel.effectIndex))
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
            document.renderRange(forRow: Int32(rowIndex),
                                  startMS: Int32(startMS),
                                  endMS: Int32(endMS),
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
        document.renderRange(forRow: Int32(rowIndex),
                              startMS: Int32(startMS),
                              endMS: Int32(endMS),
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
            document.renderEffect(forRow: Int32(rowIndex), at: Int32(effectIndex))
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

    /// Resize one edge of an effect. `edge`: 0 = left/start, 1 = right/end.
    func resizeEffectEdge(rowIndex: Int, effectIndex: Int, edge: Int, newMS: Int) {
        guard rowIndex < rows.count, effectIndex < rows[rowIndex].effects.count else { return }
        let prev = rows[rowIndex].effects[effectIndex]
        let oldMS = edge == 0 ? prev.startTimeMS : prev.endTimeMS
        if document.resizeEffectEdge(inRow: Int32(rowIndex),
                                      at: Int32(effectIndex),
                                      edge: Int32(edge),
                                      toMS: Int32(newMS)) {
            document.renderEffect(forRow: Int32(rowIndex), at: Int32(effectIndex))
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

    func copySelectedEffect() {
        guard let sel = selectedEffect else { return }
        let name = document.effectName(forRow: Int32(sel.rowIndex), at: Int32(sel.effectIndex)) ?? sel.name
        let settings = document.effectSettingsString(forRow: Int32(sel.rowIndex), at: Int32(sel.effectIndex)) ?? ""
        let palette = document.effectPaletteString(forRow: Int32(sel.rowIndex), at: Int32(sel.effectIndex)) ?? ""
        clipboard = EffectClipboard(name: name, settings: settings, palette: palette,
                                    durationMS: sel.endTimeMS - sel.startTimeMS)
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
        document.renderEffect(forRow: Int32(rowIndex), at: Int32(effectIndex))
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
                    layerName: layerName
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
                displayName = rawDisplayName
            }

            newRows.append(RowInfo(
                id: i,
                displayName: displayName,
                layerIndex: layerIndex,
                isCollapsed: isCollapsed,
                effects: effects,
                timing: timingInfo
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
