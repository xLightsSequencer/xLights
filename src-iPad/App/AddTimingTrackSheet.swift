import SwiftUI

/// Unified "Add Timing Track" sheet — replaces the per-call-site
/// confirmationDialogs that lived in DisplayElementsSheet, the
/// view-selection menu, and (eventually) the row-heading menus.
///
/// Mirrors the desktop's `NewTimingDialog` + `RowHeading::OnLayerPopup`
/// flow: pick a type, configure parameters, commit. Each option gets
/// its own parameter section that appears when selected; the bottom
/// "Add" button commits via the matching `SequencerViewModel`
/// helper. Audio-analysis options run on the main thread (they're
/// fast on modern iPads); AI Lyrics dispatches to a worker and
/// shows a progress overlay while SFSpeech runs.
///
/// The sheet drives off `viewModel.showingAddTimingTrack`, which any
/// caller can flip — currently the Display Elements sheet, the
/// Settings → Timings tab "+" button, and the row-heading
/// long-press menus. A single binding means the sheet's behaviour
/// stays identical regardless of where the user invoked it from.
struct AddTimingTrackSheet: View {
    @Environment(SequencerViewModel.self) var viewModel
    @Environment(\.dismiss) private var dismiss

    private enum TimingType: String, CaseIterable, Identifiable {
        case empty
        case fixed25, fixed50, fixed100
        case metronome
        case fppCommands, fppEffects
        case audioOnsets, audioTempo, audioChords
        case aiLyrics
        var id: String { rawValue }
    }

    // MARK: - State

    @State private var selectedType: TimingType = .empty
    @State private var trackName: String = "Timing"

    // Metronome-specific parameters.
    @State private var metronomeIntervalMS: Int = 500
    @State private var metronomeUseRandomRange: Bool = false
    @State private var metronomeMinIntervalMS: Int = 250
    @State private var metronomeUseTags: Bool = false
    @State private var metronomeTags: String = "1, 2, 3, 4"
    @State private var metronomeRandomizeTags: Bool = false

    // AI Lyrics service picker (only matters when multiple SPEECH2TEXT
    // services are configured).
    @State private var aiLyricsServiceName: String = ""

    // Async-flow state.
    @State private var working: Bool = false
    @State private var workingLabel: String = "Working…"
    @State private var errorMessage: String? = nil

    // MARK: - Body

    var body: some View {
        NavigationStack {
            Form {
                typeSection
                nameSection
                typeParamsSection
            }
            .navigationTitle("Add Timing Track")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .topBarLeading) {
                    Button("Cancel") { dismiss() }
                }
                ToolbarItem(placement: .topBarTrailing) {
                    Button("Add") { commit() }
                        .disabled(!canCommit || working)
                }
            }
            .overlay {
                if working {
                    progressOverlay
                }
            }
            .alert("Add Timing Track",
                   isPresented: Binding(
                    get: { errorMessage != nil },
                    set: { if !$0 { errorMessage = nil } }
                   )) {
                Button("OK", role: .cancel) { errorMessage = nil }
            } message: {
                Text(errorMessage ?? "")
            }
        }
        .onAppear {
            // Pick a sensible default — Audio Onsets when audio is
            // loaded but the user just opened the sheet from a non-
            // audio context, otherwise Empty. Either way, the user
            // can change it; we just reduce typing for the common
            // case.
            seedDefaultsForType(selectedType)
        }
        .onChange(of: selectedType) { _, newType in
            seedDefaultsForType(newType)
        }
    }

    // MARK: - Sections

    @ViewBuilder
    private var typeSection: some View {
        Section("Type") {
            Picker("Type", selection: $selectedType) {
                Text("Empty").tag(TimingType.empty)
                Text("Fixed 25 ms").tag(TimingType.fixed25)
                Text("Fixed 50 ms").tag(TimingType.fixed50)
                Text("Fixed 100 ms").tag(TimingType.fixed100)
                Text("Metronome…").tag(TimingType.metronome)
                Text("FPP Commands").tag(TimingType.fppCommands)
                Text("FPP Effects").tag(TimingType.fppEffects)
                if hasMedia {
                    Text("Audio Onsets").tag(TimingType.audioOnsets)
                    Text("Audio Tempo").tag(TimingType.audioTempo)
                    Text("Audio Chords").tag(TimingType.audioChords)
                }
                if canOfferAILyrics {
                    Text("AI Lyrics from Audio").tag(TimingType.aiLyrics)
                }
            }
            .pickerStyle(.menu)

            // Brief description so users understand what each
            // option does without having to find docs.
            Text(descriptionForType(selectedType))
                .font(.footnote)
                .foregroundStyle(.secondary)
        }
    }

    @ViewBuilder
    private var nameSection: some View {
        Section("Name") {
            TextField("Track name", text: $trackName)
                .textInputAutocapitalization(.words)
                .autocorrectionDisabled()
            Text("If a track with this name already exists, a numeric suffix is appended automatically.")
                .font(.caption)
                .foregroundStyle(.secondary)
        }
    }

    @ViewBuilder
    private var typeParamsSection: some View {
        switch selectedType {
        case .metronome:
            metronomeParamsSection
        case .aiLyrics:
            aiLyricsParamsSection
        default:
            EmptyView()
        }
    }

    @ViewBuilder
    private var metronomeParamsSection: some View {
        Section("Interval") {
            HStack {
                Text("Interval")
                Spacer()
                TextField("ms", value: $metronomeIntervalMS, format: .number)
                    .keyboardType(.numberPad)
                    .multilineTextAlignment(.trailing)
                    .frame(minWidth: 80)
                Text("ms").foregroundStyle(.secondary)
            }
            Toggle("Random within a range", isOn: $metronomeUseRandomRange)
            if metronomeUseRandomRange {
                HStack {
                    Text("Minimum")
                    Spacer()
                    TextField("ms", value: $metronomeMinIntervalMS, format: .number)
                        .keyboardType(.numberPad)
                        .multilineTextAlignment(.trailing)
                        .frame(minWidth: 80)
                    Text("ms").foregroundStyle(.secondary)
                }
                Text("Each mark gets a random interval between Minimum and Interval.")
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }
        }
        Section {
            Toggle("Use custom tag labels", isOn: $metronomeUseTags)
            if metronomeUseTags {
                TextField("Tags (comma-separated)", text: $metronomeTags)
                    .autocorrectionDisabled()
                Toggle("Randomize tag order", isOn: $metronomeRandomizeTags)
            }
        } header: {
            Text("Tags")
        } footer: {
            Text(metronomeUseTags
                 ? "Marks are labelled by cycling through these tags."
                 : "Without custom tags marks are labelled 1 through 10 in sequence.")
                .font(.caption)
        }
    }

    @ViewBuilder
    private var aiLyricsParamsSection: some View {
        let services = availableSpeechServices()
        Section("Service") {
            if services.isEmpty {
                Text("No speech-to-text service is configured. Open Tools → AI Services… and enable one.")
                    .font(.footnote)
                    .foregroundStyle(.secondary)
            } else if services.count == 1 {
                LabeledContent("Service", value: services[0])
            } else {
                Picker("Service", selection: $aiLyricsServiceName) {
                    ForEach(services, id: \.self) { name in
                        Text(name).tag(name)
                    }
                }
            }
            Text("Transcribes the loaded audio (or the cached vocals stem when available) into a populated timing track. Audio stays on the device.")
                .font(.caption)
                .foregroundStyle(.secondary)
        }
        .onAppear {
            if !services.isEmpty,
               aiLyricsServiceName.isEmpty || !services.contains(aiLyricsServiceName) {
                aiLyricsServiceName = services[0]
            }
        }
    }

    @ViewBuilder
    private var progressOverlay: some View {
        ZStack {
            Color.black.opacity(0.4).ignoresSafeArea()
            VStack(spacing: 12) {
                ProgressView().controlSize(.large)
                Text(workingLabel)
                    .font(.headline)
            }
            .padding(24)
            .background(.thinMaterial, in: RoundedRectangle(cornerRadius: 14))
        }
    }

    // MARK: - Helpers

    private var hasMedia: Bool {
        let p = viewModel.document.sequenceAudioFilePath() ?? ""
        return !p.isEmpty
    }

    private var canOfferAILyrics: Bool {
        guard hasMedia else { return false }
        return XLAIServices.shared().hasEnabledService(forCapability: XLAICapabilitySpeech2Text)
    }

    private func availableSpeechServices() -> [String] {
        XLAIServices.shared().allServices()
            .filter { $0.available && $0.capabilities.contains(XLAICapabilitySpeech2Text) }
            .map { $0.name }
    }

    private var canCommit: Bool {
        if trackName.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty {
            return false
        }
        switch selectedType {
        case .metronome:
            if metronomeIntervalMS <= 0 { return false }
            if metronomeUseRandomRange,
               metronomeMinIntervalMS <= 0 || metronomeMinIntervalMS >= metronomeIntervalMS {
                return false
            }
            return true
        case .aiLyrics:
            return !availableSpeechServices().isEmpty
        case .audioOnsets, .audioTempo, .audioChords:
            return hasMedia
        default:
            return true
        }
    }

    private func seedDefaultsForType(_ type: TimingType) {
        // Adjust the default name to something type-appropriate when
        // the user hasn't customised it. We treat any name that
        // matches one of the known defaults as "still on the
        // default" — preserves user-typed names but updates the
        // placeholder cleanly.
        let knownDefaults: Set<String> = [
            "Timing", "25ms", "50ms", "100ms", "Metronome",
            "FPP Commands", "FPP Effects",
            "Onsets", "Tempo", "Chords", "AutoGen"
        ]
        if knownDefaults.contains(trackName) {
            trackName = defaultNameForType(type)
        }
    }

    private func defaultNameForType(_ type: TimingType) -> String {
        switch type {
        case .empty:        return "Timing"
        case .fixed25:      return "25ms"
        case .fixed50:      return "50ms"
        case .fixed100:     return "100ms"
        case .metronome:    return "Metronome"
        case .fppCommands:  return "FPP Commands"
        case .fppEffects:   return "FPP Effects"
        case .audioOnsets:  return "Onsets"
        case .audioTempo:   return "Tempo"
        case .audioChords:  return "Chords"
        case .aiLyrics:     return "AutoGen"
        }
    }

    private func descriptionForType(_ type: TimingType) -> String {
        switch type {
        case .empty:        return "An empty timing track. Add marks manually."
        case .fixed25:      return "A timing mark every 25 ms from the start of the sequence to the end."
        case .fixed50:      return "A timing mark every 50 ms from the start of the sequence to the end."
        case .fixed100:     return "A timing mark every 100 ms from the start of the sequence to the end."
        case .metronome:    return "Timing marks at a custom interval, optionally with cycling labels."
        case .fppCommands:  return "FPP Commands track — events the Falcon Player can run during playback."
        case .fppEffects:   return "FPP Effects track — events that trigger pixel effects on FPP-managed shows."
        case .audioOnsets:  return "Detect percussive onsets in the audio and create one mark per onset."
        case .audioTempo:   return "Detect the audio's tempo and create marks on each beat."
        case .audioChords:  return "Detect chord changes in the audio and label each segment with its chord name."
        case .aiLyrics:     return "Transcribe vocals into a populated timing track using on-device speech recognition."
        }
    }

    // MARK: - Commit

    private func commit() {
        let name = trackName.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !name.isEmpty else { return }

        switch selectedType {
        case .empty:
            commitFixed(intervalMS: 0, name: name, action: "Add Timing Track")
        case .fixed25:
            commitFixed(intervalMS: 25, name: name, action: "Add Fixed Timing Track")
        case .fixed50:
            commitFixed(intervalMS: 50, name: name, action: "Add Fixed Timing Track")
        case .fixed100:
            commitFixed(intervalMS: 100, name: name, action: "Add Fixed Timing Track")
        case .metronome:
            commitMetronome(name: name)
        case .fppCommands:
            commitFPP(name: name, subType: "FPP Commands")
        case .fppEffects:
            commitFPP(name: name, subType: "FPP Effects")
        case .audioOnsets:
            commitAudioOnsets(name: name)
        case .audioTempo:
            commitAudioTempo(name: name)
        case .audioChords:
            commitAudioChords(name: name)
        case .aiLyrics:
            commitAILyrics(name: name)
        }
    }

    private func commitFixed(intervalMS: Int, name: String, action: String) {
        guard viewModel.addFixedIntervalTimingTrack(name: name, intervalMS: intervalMS) != nil else {
            errorMessage = "Couldn't add the timing track."
            return
        }
        dismiss()
    }

    private func commitMetronome(name: String) {
        let tags: [String] = metronomeUseTags
            ? metronomeTags.split(separator: ",")
                            .map { $0.trimmingCharacters(in: .whitespaces) }
                            .filter { !$0.isEmpty }
            : []
        let minMS = metronomeUseRandomRange ? metronomeMinIntervalMS : -1
        guard viewModel.addMetronomeTimingTrack(name: name,
                                                 intervalMS: metronomeIntervalMS,
                                                 tags: tags,
                                                 minIntervalMS: minMS,
                                                 randomize: metronomeRandomizeTags) != nil else {
            errorMessage = "Couldn't add the metronome timing track. Check the interval values."
            return
        }
        dismiss()
    }

    private func commitFPP(name: String, subType: String) {
        guard viewModel.addFPPTimingTrack(name: name, subType: subType) != nil else {
            errorMessage = "Couldn't add the \(subType) track."
            return
        }
        dismiss()
    }

    private func commitAudioOnsets(name: String) {
        // Onset detection blocks for ~tens of ms on a 3 min track,
        // so a sync call is fine. The bridge already caches results
        // when called twice in a row.
        guard viewModel.generateTimingTrackFromOnsets(name: name) != nil else {
            errorMessage = "Onset detection produced no marks. Try a track with more obvious percussive content."
            return
        }
        dismiss()
    }

    private func commitAudioTempo(name: String) {
        let result = viewModel.detectTempo()
        if result.beats.isEmpty {
            errorMessage = "Tempo detection produced no beats. The audio may be too short or too sparse."
            return
        }
        // The view-model's existing helper labels the track with
        // the detected BPM ("Tempo (120 BPM)"). For the unified
        // sheet we want the user-supplied name to win — call the
        // bridge directly to add the timing-and-marks track with
        // that name.
        let createdName = viewModel.addFixedIntervalTimingTrack(name: name, intervalMS: 0) ?? ""
        if createdName.isEmpty {
            errorMessage = "Couldn't create the tempo timing track."
            return
        }
        let rowIdx = viewModel.rows.lastIndex { $0.timing?.elementName == createdName }
        if let row = rowIdx {
            for (i, beatMS) in result.beats.enumerated() {
                let endMS = i + 1 < result.beats.count
                    ? result.beats[i + 1]
                    : viewModel.sequenceDurationMS
                if endMS <= beatMS { continue }
                _ = viewModel.document.addTimingMark(atRow: Int32(row),
                                                       startMS: Int32(beatMS),
                                                       endMS: Int32(endMS),
                                                       label: "")
            }
            viewModel.reloadRows()
        }
        dismiss()
    }

    private func commitAudioChords(name: String) {
        let result = viewModel.detectChords()
        if result.chords.isEmpty {
            errorMessage = "Chord detection produced no segments. The audio may be too short or too quiet."
            return
        }
        let createdName = viewModel.addFixedIntervalTimingTrack(name: name, intervalMS: 0) ?? ""
        if createdName.isEmpty {
            errorMessage = "Couldn't create the chords timing track."
            return
        }
        let rowIdx = viewModel.rows.lastIndex { $0.timing?.elementName == createdName }
        if let row = rowIdx {
            for c in result.chords {
                if c.1 <= c.0 { continue }
                _ = viewModel.document.addTimingMark(atRow: Int32(row),
                                                       startMS: Int32(c.0),
                                                       endMS: Int32(c.1),
                                                       label: c.2)
            }
            viewModel.reloadRows()
        }
        dismiss()
    }

    private func commitAILyrics(name: String) {
        // Override the name the view-model would default to ("AutoGen")
        // by writing it into the result post-hoc. Simpler to just
        // call the existing flow as-is and rename below.
        working = true
        workingLabel = "Transcribing audio…"
        viewModel.generateAILyricTrack(name: name,
                                         serviceName: aiLyricsServiceName.isEmpty ? nil : aiLyricsServiceName) { error in
            working = false
            if let error = error {
                errorMessage = error
            } else {
                dismiss()
            }
        }
    }
}
