import SwiftUI
import UniformTypeIdentifiers

// E-3 — Sequence Settings dialog. Post-open editor for
// sequence-wide settings. Four tabs: Info (read-only metadata +
// sequence type + audio file swap), Metadata (song / artist /
// album / author / website / comment / music URL), Media
// (per-effect media inventory — embed/extract/rename/replace,
// mirrors desktop's "Manage All Media" panel), Render
// (model-blending + frame rate + autosave). Timings
// import/export and Data Layers are deferred — covered elsewhere
// (row-header long-press for basic timing management) or out of
// MVP.
//
// Entry: gear icon in the sequencer toolbar. Changes mark the
// sequence dirty through the bridge's normal
// `IncrementChangeCount` path so the toolbar Save dot lights
// up for unsaved setting edits.

struct SequenceSettingsSheet: View {
    @Environment(\.dismiss) private var dismiss
    @Environment(SequencerViewModel.self) var viewModel

    private enum Tab: String, CaseIterable, Identifiable {
        case info, metadata, media, timings, audio, render
        var id: String { rawValue }
        var label: String {
            switch self {
            case .info:     return "Info"
            case .metadata: return "Metadata"
            case .media:    return "Media"
            case .timings:  return "Timings"
            case .audio:    return "Audio"
            case .render:   return "Render"
            }
        }
        var symbol: String {
            switch self {
            case .info:     return "info.circle"
            case .metadata: return "character.book.closed"
            case .media:    return "photo.stack"
            case .timings:  return "metronome"
            case .audio:    return "waveform"
            case .render:   return "cpu"
            }
        }
    }

    @State private var selectedTab: Tab = .info

    var body: some View {
        NavigationStack {
            VStack(spacing: 0) {
                Picker("Tab", selection: $selectedTab) {
                    ForEach(Tab.allCases) { t in
                        Label(t.label, systemImage: t.symbol).tag(t)
                    }
                }
                .pickerStyle(.segmented)
                .padding(.horizontal)
                .padding(.vertical, 8)
                Divider()
                Group {
                    switch selectedTab {
                    case .info:
                        ScrollView { InfoTab().padding() }
                    case .metadata:
                        ScrollView { MetadataTab().padding() }
                    case .media:
                        // Embedded variant — outer sheet supplies
                        // the Done button; the manager contributes
                        // its ellipsis "Remove Unused…" menu to the
                        // shared toolbar via SwiftUI's toolbar
                        // composition.
                        MediaManagerContent(showsDoneButton: false)
                    case .timings:
                        TimingsTab()
                    case .audio:
                        AudioTracksTab()
                    case .render:
                        ScrollView { RenderTab().padding() }
                    }
                }
            }
            .navigationTitle("Sequence Settings")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") { dismiss() }
                }
            }
        }
    }
}

// MARK: - Info tab (read-only summary + type selector)

private struct InfoTab: View {
    @Environment(SequencerViewModel.self) var viewModel

    @State private var seqType: String = ""
    @State private var currentMediaPath: String = ""
    @State private var pickingMediaFile = false
    @State private var pickedMediaURL: URL? = nil

    var body: some View {
        VStack(alignment: .leading, spacing: 10) {
            SettingsRow(label: "Filename", value: displayFilename)
            SettingsRow(label: "File Version",
                        value: viewModel.document.sequenceFileVersion() ?? "")
            SettingsRow(label: "App Version",
                        value: viewModel.document.currentAppVersion() ?? "")
            SettingsRow(label: "Duration",
                        value: formatDuration(viewModel.sequenceDurationMS))
            SettingsRow(label: "Frame Interval",
                        value: "\(viewModel.frameIntervalMS) ms")
            SettingsRow(label: "Models / Submodels",
                        value: "\(viewModel.document.sequenceModelCount())")
            Divider().padding(.vertical, 4)
            // Changing from Musical to Animation clears the audio
            // file (bridge's SetSequenceType handles that). Changing
            // to Musical doesn't auto-pick a file — the user picks
            // one via the Audio File row below.
            Text("Sequence Type")
                .font(.caption)
                .foregroundStyle(.secondary)
            Picker("", selection: $seqType) {
                Text("Musical").tag("Media")
                Text("Animation").tag("Animation")
                Text("Effect").tag("Effect")
            }
            .pickerStyle(.segmented)
            .onChange(of: seqType) { _, new in
                guard !new.isEmpty else { return }
                if viewModel.document.sequenceType() != new {
                    _ = viewModel.document.setSequenceType(new)
                    currentMediaPath = viewModel.document.currentMediaFilePath() ?? ""
                }
            }
            Text("Changing from Musical clears the attached audio file.")
                .font(.caption2)
                .foregroundStyle(.secondary)
            Divider().padding(.vertical, 4)
            Text("Audio File")
                .font(.caption)
                .foregroundStyle(.secondary)
            HStack {
                Text(currentMediaPath.isEmpty ? "(none)" : currentMediaPath)
                    .font(.caption)
                    .foregroundStyle(.secondary)
                    .lineLimit(2)
                    .truncationMode(.middle)
                    .frame(maxWidth: .infinity, alignment: .leading)
                if !currentMediaPath.isEmpty {
                    Button {
                        _ = viewModel.document.setMediaFilePath("")
                        currentMediaPath = ""
                    } label: {
                        Image(systemName: "xmark.circle.fill")
                            .foregroundStyle(.secondary)
                    }
                    .buttonStyle(.plain)
                }
            }
            Button {
                pickingMediaFile = true
            } label: {
                Label("Choose Audio File…", systemImage: "music.note.list")
            }
            .buttonStyle(.bordered)
            Text("Swapping the audio file does not re-run song / artist auto-tagging. Use the Metadata tab to update text fields if needed.")
                .font(.caption2)
                .foregroundStyle(.secondary)
        }
        .onAppear {
            seqType = viewModel.document.sequenceType() ?? "Animation"
            currentMediaPath = viewModel.document.currentMediaFilePath() ?? ""
        }
        .fileImporter(isPresented: $pickingMediaFile,
                      allowedContentTypes: [.audio]) { result in
            if case .success(let url) = result {
                pickedMediaURL = url
            }
        }
        .mediaRelocationPrompt(
            picked: $pickedMediaURL,
            subdirectory: ""
        ) { storedPath in
            _ = viewModel.document.setMediaFilePath(storedPath)
            currentMediaPath = storedPath
        }
    }

    private var displayFilename: String {
        let p = viewModel.document.currentSequencePath() ?? ""
        if p.isEmpty { return "(unsaved)" }
        return (p as NSString).lastPathComponent
    }

    private func formatDuration(_ ms: Int) -> String {
        let s = ms / 1000
        let m = s / 60
        return String(format: "%d:%02d.%03d", m, s % 60, ms % 1000)
    }
}

// MARK: - Metadata tab (header text fields)

private struct MetadataTab: View {
    @Environment(SequencerViewModel.self) var viewModel

    @State private var song: String = ""
    @State private var artist: String = ""
    @State private var album: String = ""
    @State private var author: String = ""
    @State private var email: String = ""
    @State private var website: String = ""
    @State private var url: String = ""
    @State private var comment: String = ""

    var body: some View {
        VStack(alignment: .leading, spacing: 10) {
            metadataField("Song", text: $song, key: "song")
            metadataField("Artist", text: $artist, key: "artist")
            metadataField("Album", text: $album, key: "album")
            Divider().padding(.vertical, 4)
            metadataField("Author", text: $author, key: "author")
            metadataField("Author Email", text: $email, key: "email")
            metadataField("Website", text: $website, key: "website")
            metadataField("Music URL", text: $url, key: "url")
            Divider().padding(.vertical, 4)
            VStack(alignment: .leading, spacing: 4) {
                Text("Comment")
                    .font(.caption)
                    .foregroundStyle(.secondary)
                TextEditor(text: $comment)
                    .frame(minHeight: 70)
                    .padding(4)
                    .background(
                        RoundedRectangle(cornerRadius: 6)
                            .stroke(Color.secondary.opacity(0.3))
                    )
                    .onChange(of: comment) { _, new in
                        _ = viewModel.document.setHeaderInfo(new, forKey: "comment")
                    }
            }
        }
        .onAppear { load() }
    }

    private func load() {
        song    = viewModel.document.headerInfo(forKey: "song") ?? ""
        artist  = viewModel.document.headerInfo(forKey: "artist") ?? ""
        album   = viewModel.document.headerInfo(forKey: "album") ?? ""
        author  = viewModel.document.headerInfo(forKey: "author") ?? ""
        email   = viewModel.document.headerInfo(forKey: "email") ?? ""
        website = viewModel.document.headerInfo(forKey: "website") ?? ""
        url     = viewModel.document.headerInfo(forKey: "url") ?? ""
        comment = viewModel.document.headerInfo(forKey: "comment") ?? ""
    }

    @ViewBuilder
    private func metadataField(_ label: String,
                                text: Binding<String>,
                                key: String) -> some View {
        VStack(alignment: .leading, spacing: 2) {
            Text(label)
                .font(.caption)
                .foregroundStyle(.secondary)
            TextField(label, text: text)
                .textFieldStyle(.roundedBorder)
                .onChange(of: text.wrappedValue) { _, new in
                    _ = viewModel.document.setHeaderInfo(new, forKey: key)
                }
        }
    }
}

// MARK: - Timings tab (per-track rename / delete / export + bulk import)

/// Phase E follow-up — central management of timing tracks. Per-row
/// rename/delete already exists on the row-header long-press menu;
/// this tab adds bulk import (.xtiming, .lms, .pgo), per-track
/// `.xtiming` export, and a single-place overview of every timing
/// track in the sequence.
private struct TimingsTab: View {
    @Environment(SequencerViewModel.self) var viewModel

    fileprivate struct TimingRow: Identifiable, Hashable {
        let id: Int        // row index in the visible-row list
        let name: String
        let layerName: String
    }

    @State private var renameTarget: TimingRow?
    @State private var renameText: String = ""
    @State private var deleteTarget: TimingRow?
    @State private var importing = false
    @State private var importFormat: ImportFormat = .xtiming
    @State private var exportTarget: TimingRow?
    @State private var exportDoc: XTimingFile?

    private enum ImportFormat: String, CaseIterable, Identifiable {
        case xtiming, lor, papagayo
        var id: String { rawValue }
        var label: String {
            switch self {
            case .xtiming:  return "xLights (.xtiming)"
            case .lor:      return "LOR (.lms)"
            case .papagayo: return "Papagayo (.pgo)"
            }
        }
        var ext: String {
            switch self {
            case .xtiming:  return "xtiming"
            case .lor:      return "lms"
            case .papagayo: return "pgo"
            }
        }
    }

    private var timingRows: [TimingRow] {
        // Layer 0 of each timing element is the canonical "this is
        // the timing track" row; sub-layers (Words / Phonemes) hang
        // off layer > 0 of the same element. Filter to layer 0 so
        // the user manipulates whole tracks rather than sub-layers.
        viewModel.rows.enumerated().compactMap { idx, row in
            guard let timing = row.timing, row.layerIndex == 0 else { return nil }
            return TimingRow(id: idx, name: timing.elementName,
                              layerName: timing.layerName)
        }
    }

    var body: some View {
        list
            .listStyle(.inset)
            .fileImporter(
                isPresented: $importing,
                allowedContentTypes: [importContentType],
                allowsMultipleSelection: false
            ) { result in
                handleImport(result: result)
            }
            .modifier(RenameAlertModifier(target: $renameTarget,
                                            text: $renameText,
                                            viewModel: viewModel))
            .modifier(DeleteAlertModifier(target: $deleteTarget,
                                            viewModel: viewModel))
            .fileExporter(
                isPresented: Binding(
                    get: { exportDoc != nil },
                    set: { if !$0 { exportDoc = nil; exportTarget = nil } }
                ),
                document: exportDoc,
                contentType: kXTimingFileType,
                defaultFilename: exportDoc?.suggestedName ?? "Timing"
            ) { _ in
                exportDoc = nil
                exportTarget = nil
            }
    }

    @ViewBuilder
    private var list: some View {
        List {
            Section("Tracks") {
                if timingRows.isEmpty {
                    Text("No timing tracks. Use the Import button to add one.")
                        .foregroundStyle(.secondary)
                } else {
                    ForEach(timingRows) { tr in
                        timingRow(tr)
                    }
                }
            }
            Section {
                Picker("Format", selection: $importFormat) {
                    ForEach(ImportFormat.allCases) { f in
                        Text(f.label).tag(f)
                    }
                }
                .pickerStyle(.menu)
                Button {
                    importing = true
                } label: {
                    Label("Import…", systemImage: "square.and.arrow.down")
                }
            } header: {
                Text("Import")
            } footer: {
                Text("Imported tracks are appended to the sequence and become the active timing.")
            }
        }
    }

    @ViewBuilder
    private func timingRow(_ tr: TimingRow) -> some View {
        HStack {
            VStack(alignment: .leading) {
                Text(tr.name)
                if !tr.layerName.isEmpty && tr.layerName != tr.name {
                    Text(tr.layerName)
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
            }
            Spacer()
            Menu {
                Button {
                    renameText = tr.name
                    renameTarget = tr
                } label: { Label("Rename…", systemImage: "pencil") }
                Button {
                    exportTarget = tr
                    exportDoc = XTimingFile(rowIndex: tr.id,
                                              suggestedName: tr.name,
                                              viewModel: viewModel)
                } label: { Label("Export…", systemImage: "square.and.arrow.up") }
                Button(role: .destructive) {
                    deleteTarget = tr
                } label: { Label("Delete", systemImage: "trash") }
            } label: {
                Image(systemName: "ellipsis.circle")
            }
        }
    }

    private var importContentType: UTType {
        switch importFormat {
        case .xtiming:  return kXTimingFileType
        case .lor:      return UTType(filenameExtension: "lms") ?? .xml
        case .papagayo: return UTType(filenameExtension: "pgo") ?? .text
        }
    }

    private func handleImport(result: Result<[URL], Error>) {
        guard case .success(let urls) = result, let url = urls.first else { return }
        let needsAccess = url.startAccessingSecurityScopedResource()
        defer { if needsAccess { url.stopAccessingSecurityScopedResource() } }
        _ = XLSequenceDocument.obtainAccess(toPath: url.path,
                                              enforceWritable: false)
        let added: Int
        switch importFormat {
        case .xtiming:  added = viewModel.importXTiming(path: url.path)
        case .lor:      added = viewModel.importLorTiming(path: url.path)
        case .papagayo: added = viewModel.importPapagayoTiming(path: url.path)
        }
        if added > 0 { viewModel.reloadRows() }
    }
}

private struct RenameAlertModifier: ViewModifier {
    @Binding var target: TimingsTab.TimingRow?
    @Binding var text: String
    let viewModel: SequencerViewModel
    func body(content: Content) -> some View {
        content.alert("Rename Timing Track",
                      isPresented: Binding(
                        get: { target != nil },
                        set: { if !$0 { target = nil } }
                      ),
                      presenting: target) { _ in
            TextField("Name", text: $text)
            Button("OK") {
                if let tgt = target, !text.isEmpty, text != tgt.name {
                    _ = viewModel.document.renameTimingTrack(
                        at: Int32(tgt.id), newName: text)
                    viewModel.reloadRows()
                }
                target = nil
            }
            Button("Cancel", role: .cancel) { target = nil }
        } message: { tgt in
            Text("Rename \"\(tgt.name)\" to:")
        }
    }
}

private struct DeleteAlertModifier: ViewModifier {
    @Binding var target: TimingsTab.TimingRow?
    let viewModel: SequencerViewModel
    func body(content: Content) -> some View {
        content.alert("Delete Timing Track",
                      isPresented: Binding(
                        get: { target != nil },
                        set: { if !$0 { target = nil } }
                      ),
                      presenting: target) { _ in
            Button("Delete", role: .destructive) {
                if let tgt = target {
                    _ = viewModel.document.deleteTimingTrack(at: Int32(tgt.id))
                    viewModel.reloadRows()
                }
                target = nil
            }
            Button("Cancel", role: .cancel) { target = nil }
        } message: { tgt in
            Text("Delete timing track \"\(tgt.name)\"? All marks on this track will be lost.")
        }
    }
}

/// Value-type FileDocument carrying a pre-rendered `.xtiming` blob.
/// The export call (which has to run on the main actor since it
/// touches the bridge) happens at construction time on the main
/// queue; `fileWrapper(_:)` then just hands the bytes back without
/// needing actor isolation.
private struct XTimingFile: FileDocument {
    static var readableContentTypes: [UTType] { [kXTimingFileType] }
    static var writableContentTypes: [UTType] { [kXTimingFileType] }

    let suggestedName: String
    let payload: Data

    /// Run the bridge export into a temp file and slurp the bytes
    /// back. Returns nil when no row matches or the export failed.
    @MainActor
    init?(rowIndex: Int, suggestedName: String, viewModel: SequencerViewModel) {
        let tmp = URL(fileURLWithPath: NSTemporaryDirectory())
            .appendingPathComponent(UUID().uuidString)
            .appendingPathExtension("xtiming")
        defer { try? FileManager.default.removeItem(at: tmp) }
        guard viewModel.exportTimingTrack(rowIndex: rowIndex, path: tmp.path),
              let data = try? Data(contentsOf: tmp) else {
            return nil
        }
        self.suggestedName = suggestedName
        self.payload = data
    }

    init(configuration: ReadConfiguration) throws {
        throw CocoaError(.featureUnsupported)
    }

    func fileWrapper(configuration: WriteConfiguration) throws -> FileWrapper {
        FileWrapper(regularFileWithContents: payload)
    }
}

// MARK: - Audio tracks tab (alt-track CRUD)

/// Phase E follow-up — alternate audio tracks (vocal stems etc.).
/// Tracks already round-trip through the .xsq XML and B43 added
/// the waveform-side switcher; this tab adds authoring (add /
/// remove / rename / replace path).
private struct AudioTracksTab: View {
    @Environment(SequencerViewModel.self) var viewModel

    @State private var refreshTick: Int = 0
    @State private var pickingAdd: URL?
    @State private var replaceTarget: Int?
    @State private var pickingReplace: URL?
    @State private var renameTarget: Int?
    @State private var renameText: String = ""
    @State private var deleteTarget: Int?

    private var trackCount: Int {
        // refreshTick is read so SwiftUI re-evaluates after a mutation.
        _ = refreshTick
        return Int(viewModel.document.altTrackCount())
    }

    var body: some View {
        list
            .listStyle(.inset)
            .mediaRelocationPrompt(picked: $pickingAdd,
                                     subdirectory: "Audio") { stored in
                _ = viewModel.document.addAltTrack(atPath: stored, shortname: "")
                bumpRefresh()
            }
            .mediaRelocationPrompt(picked: $pickingReplace,
                                     subdirectory: "Audio") { stored in
                if let idx = replaceTarget {
                    _ = viewModel.document.setAltTrackPath(at: idx, path: stored)
                    bumpRefresh()
                }
                replaceTarget = nil
            }
            .modifier(AltTrackRenameAlertModifier(
                target: $renameTarget,
                text: $renameText,
                viewModel: viewModel,
                onCommit: bumpRefresh))
            .modifier(AltTrackDeleteAlertModifier(
                target: $deleteTarget,
                viewModel: viewModel,
                onCommit: bumpRefresh))
    }

    @ViewBuilder
    private var list: some View {
        List {
            Section {
                if trackCount == 0 {
                    Text("No alternate audio tracks. Use Add… to import a vocal stem, instrumental, click track, etc.")
                        .foregroundStyle(.secondary)
                } else {
                    ForEach(0..<trackCount, id: \.self) { idx in
                        trackRow(idx)
                    }
                }
            } header: {
                Text("Tracks")
            } footer: {
                Text("Alternate tracks appear in the waveform's long-press menu so the user can switch which track is displayed. Playback stays on the main audio track.")
            }
            Section {
                Button {
                    // Picker will fire — actual file pick is handled
                    // in the .fileImporter modifier below.
                    pickingAddTrigger = true
                } label: {
                    Label("Add Track…", systemImage: "plus.circle")
                }
            }
        }
        .fileImporter(isPresented: $pickingAddTrigger,
                      allowedContentTypes: [.audio],
                      allowsMultipleSelection: false) { result in
            if case .success(let urls) = result, let url = urls.first {
                pickingAdd = url
            }
        }
        .fileImporter(isPresented: $pickingReplaceTrigger,
                      allowedContentTypes: [.audio],
                      allowsMultipleSelection: false) { result in
            if case .success(let urls) = result, let url = urls.first {
                pickingReplace = url
            } else {
                replaceTarget = nil
            }
        }
    }

    @State private var pickingAddTrigger = false
    @State private var pickingReplaceTrigger = false

    @ViewBuilder
    private func trackRow(_ idx: Int) -> some View {
        let displayName = viewModel.document.altTrackDisplayName(at: idx) ?? ""
        let path = viewModel.document.altTrackPath(at: idx) ?? ""
        HStack {
            VStack(alignment: .leading) {
                Text(displayName)
                Text(path.isEmpty
                     ? "(no file resolved — pick a replacement)"
                     : URL(fileURLWithPath: path).lastPathComponent)
                    .font(.caption)
                    .foregroundStyle(.secondary)
                    .lineLimit(1)
                    .truncationMode(.middle)
            }
            Spacer()
            Menu {
                Button {
                    renameText = viewModel.document.altTrackShortname(at: idx) ?? ""
                    renameTarget = idx
                } label: { Label("Rename…", systemImage: "pencil") }
                Button {
                    replaceTarget = idx
                    pickingReplaceTrigger = true
                } label: { Label("Replace File…", systemImage: "arrow.triangle.2.circlepath") }
                Button(role: .destructive) {
                    deleteTarget = idx
                } label: { Label("Remove", systemImage: "trash") }
            } label: {
                Image(systemName: "ellipsis.circle")
            }
        }
    }

    private func bumpRefresh() {
        refreshTick &+= 1
        // The waveform-switcher menu mirrors `altAudioTrackNames` on
        // the view model; refresh after every CRUD op so the
        // submenu picks up adds / renames / removes immediately.
        viewModel.reloadAltTracks()
    }
}

private struct AltTrackRenameAlertModifier: ViewModifier {
    @Binding var target: Int?
    @Binding var text: String
    let viewModel: SequencerViewModel
    let onCommit: () -> Void

    func body(content: Content) -> some View {
        content.alert("Rename Audio Track",
                      isPresented: Binding(
                        get: { target != nil },
                        set: { if !$0 { target = nil } }
                      ),
                      presenting: target) { _ in
            TextField("Short name", text: $text)
                .textInputAutocapitalization(.words)
            Button("OK") {
                if let idx = target {
                    _ = viewModel.document.setAltTrackShortname(at: idx,
                                                                 shortname: text)
                    onCommit()
                }
                target = nil
            }
            Button("Cancel", role: .cancel) { target = nil }
        } message: { _ in
            Text("A short, descriptive label (e.g. \"Drums\", \"Vocals\"). Leave blank to fall back to \"Track N\".")
        }
    }
}

private struct AltTrackDeleteAlertModifier: ViewModifier {
    @Binding var target: Int?
    let viewModel: SequencerViewModel
    let onCommit: () -> Void

    func body(content: Content) -> some View {
        content.alert("Remove Audio Track",
                      isPresented: Binding(
                        get: { target != nil },
                        set: { if !$0 { target = nil } }
                      ),
                      presenting: target) { _ in
            Button("Remove", role: .destructive) {
                if let idx = target {
                    _ = viewModel.document.removeAltTrack(at: idx)
                    onCommit()
                }
                target = nil
            }
            Button("Cancel", role: .cancel) { target = nil }
        } message: { idx in
            Text("Remove \"\(viewModel.document.altTrackDisplayName(at: idx) ?? "")\"? The audio file on disk is not deleted.")
        }
    }
}

// MARK: - Render tab (blending + frame rate)

private struct RenderTab: View {
    @Environment(SequencerViewModel.self) var viewModel

    @State private var blendingEnabled: Bool = false
    @State private var frameMS: Int = 50
    @State private var autosaveInterval: Int = 5

    var body: some View {
        VStack(alignment: .leading, spacing: 14) {
            Toggle(isOn: $blendingEnabled) {
                VStack(alignment: .leading, spacing: 2) {
                    Text("Model Blending")
                        .font(.body)
                    Text("When enabled, models composite using their blend mode rather than overwriting. Required for transparency-over-other-models effects.")
                        .font(.caption2)
                        .foregroundStyle(.secondary)
                }
            }
            .onChange(of: blendingEnabled) { _, new in
                _ = viewModel.document.setSequenceSupportsModelBlending(new)
            }

            Divider()

            VStack(alignment: .leading, spacing: 4) {
                Text("Frame Interval")
                    .font(.caption)
                    .foregroundStyle(.secondary)
                Picker("", selection: $frameMS) {
                    Text("25 ms (40 FPS)").tag(25)
                    Text("33 ms (30 FPS)").tag(33)
                    Text("50 ms (20 FPS)").tag(50)
                    Text("100 ms (10 FPS)").tag(100)
                }
                .pickerStyle(.segmented)
                .onChange(of: frameMS) { _, new in
                    _ = viewModel.document.setFrameIntervalMS(Int32(new))
                }
                Text("Changing the frame rate requires a full re-render on next save.")
                    .font(.caption2)
                    .foregroundStyle(.secondary)
            }

            Divider()

            VStack(alignment: .leading, spacing: 4) {
                Text("Autosave")
                    .font(.caption)
                    .foregroundStyle(.secondary)
                Picker("", selection: $autosaveInterval) {
                    Text("Off").tag(0)
                    Text("2 min").tag(2)
                    Text("5 min").tag(5)
                    Text("10 min").tag(10)
                    Text("30 min").tag(30)
                }
                .pickerStyle(.segmented)
                .onChange(of: autosaveInterval) { _, new in
                    viewModel.autosaveIntervalMinutes = new
                    if viewModel.isSequenceLoaded {
                        if new > 0 {
                            viewModel.startAutosaveTimer()
                        } else {
                            viewModel.stopAutosaveTimer()
                        }
                    }
                }
                Text("Writes a .xbkp snapshot alongside the .xsq at the chosen cadence. Offered for recovery on next open.")
                    .font(.caption2)
                    .foregroundStyle(.secondary)
            }
        }
        .onAppear {
            blendingEnabled = viewModel.document.sequenceSupportsModelBlending()
            frameMS = viewModel.frameIntervalMS
            autosaveInterval = viewModel.autosaveIntervalMinutes
        }
    }
}

// MARK: - Helpers

private struct SettingsRow: View {
    let label: String
    let value: String
    var body: some View {
        HStack {
            Text(label)
                .font(.caption)
                .foregroundStyle(.secondary)
            Spacer()
            Text(value.isEmpty ? "—" : value)
                .font(.caption)
                .fontWeight(.semibold)
                .lineLimit(1)
                .truncationMode(.middle)
        }
    }
}
