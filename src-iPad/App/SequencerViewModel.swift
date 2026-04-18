import SwiftUI
import UIKit

@Observable
class SequencerViewModel {
    var document = XLSequenceDocument()
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

    // Waveform cache
    var waveformPeaks: [Float] = []
    var waveformStartMS: Int = 0
    var waveformEndMS: Int = 0

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

    struct EffectSelection {
        let rowIndex: Int
        let effectIndex: Int
        let name: String
        let startTimeMS: Int
        let endTimeMS: Int
    }

    private var playbackTimer: Timer?
    private var renderPollTimer: Timer?
    private var playbackStartTime: CFAbsoluteTime = 0  // wall clock when play started
    private var playbackStartMS: Int = 0                // sequence position when play started

    struct RowInfo: Identifiable {
        let id: Int
        let displayName: String
        let layerIndex: Int
        let isCollapsed: Bool
        let effects: [EffectInfo]
    }

    struct EffectInfo: Identifiable {
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
        if document.deleteEffect(inRow: Int32(sel.rowIndex), at: Int32(sel.effectIndex)) {
            clearSelection()
            reloadRows()
        }
    }

    func addEffect(rowIndex: Int, startMS: Int, endMS: Int) {
        let name = selectedPaletteEffect ?? "On"
        if document.addEffect(toRow: Int32(rowIndex), name: name, startMS: Int32(startMS), endMS: Int32(endMS)) {
            reloadRows()
        }
    }

    func moveEffect(rowIndex: Int, effectIndex: Int, newStartMS: Int, newEndMS: Int) {
        if document.moveEffect(inRow: Int32(rowIndex), at: Int32(effectIndex),
                               toStartMS: Int32(newStartMS), toEndMS: Int32(newEndMS)) {
            reloadRows()
            if selectedEffect?.rowIndex == rowIndex && selectedEffect?.effectIndex == effectIndex {
                selectEffect(rowIndex: rowIndex, effectIndex: effectIndex)
            }
        }
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

    func loadWaveform(startMS: Int, endMS: Int) {
        guard hasAudio else { return }
        let numSamples = 2000
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
    }

    // MARK: - Rows

    func reloadRows() {
        var newRows: [RowInfo] = []
        let count = Int(document.visibleRowCount())

        for i in 0..<count {
            let idx = Int32(i)
            let displayName = document.rowDisplayName(at: idx) ?? ""
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

            newRows.append(RowInfo(
                id: i,
                displayName: displayName,
                layerIndex: layerIndex,
                isCollapsed: isCollapsed,
                effects: effects
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
