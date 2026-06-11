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
    @State private var durationText: String = ""
    @AppStorage("overwriteTagsOnMediaChange") private var overwriteTags = false

    var body: some View {
        VStack(alignment: .leading, spacing: 10) {
            SettingsRow(label: "Filename", value: displayFilename)
            SettingsRow(label: "File Version",
                        value: viewModel.document.sequenceFileVersion())
            SettingsRow(label: "App Version",
                        value: viewModel.document.currentAppVersion())
            HStack {
                Text("Duration (sec)")
                    .font(.caption)
                    .foregroundStyle(.secondary)
                Spacer()
                TextField("Seconds", text: $durationText)
                    .textFieldStyle(.roundedBorder)
                    .frame(width: 110)
                    .multilineTextAlignment(.trailing)
                    .onSubmit { commitDuration() }
            }
            SettingsRow(label: "Frame Interval",
                        value: "\(viewModel.frameIntervalMS) ms")
            SettingsRow(label: "Models / Submodels",
                        value: "\(viewModel.document.sequenceModelCount())")
            SettingsRow(label: "Media Hash",
                        value: viewModel.document.mediaFileHash())
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
                    currentMediaPath = viewModel.document.currentMediaFilePath()
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
            // Desktop "Overwrite Tags" checkbox: when on, swapping the
            // audio pulls song/artist/album from the new file's tags
            // into the sequence metadata.
            Toggle("Overwrite metadata tags on audio change", isOn: $overwriteTags)
                .font(.caption)
            Text(overwriteTags
                 ? "Swapping the audio file replaces Song / Artist / Album from the new file's tags."
                 : "Swapping the audio file preserves the existing Song / Artist / Album metadata.")
                .font(.caption2)
                .foregroundStyle(.secondary)
        }
        .onAppear {
            seqType = viewModel.document.sequenceType()
            currentMediaPath = viewModel.document.currentMediaFilePath()
            durationText = String(format: "%.3f", Double(viewModel.sequenceDurationMS) / 1000.0)
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
            if overwriteTags {
                let title = viewModel.document.audioTitle()
                let artist = viewModel.document.audioArtist()
                let album = viewModel.document.audioAlbum()
                if !title.isEmpty { _ = viewModel.document.setHeaderInfo(title, forKey: "song") }
                if !artist.isEmpty { _ = viewModel.document.setHeaderInfo(artist, forKey: "artist") }
                if !album.isEmpty { _ = viewModel.document.setHeaderInfo(album, forKey: "album") }
            }
        }
    }

    private func commitDuration() {
        let secs = Double(durationText.trimmingCharacters(in: .whitespaces)) ?? 0
        let ms = Int(secs * 1000.0)
        if ms > 0, viewModel.document.setSequenceDurationMS(Int32(ms)) {
            viewModel.reloadRows()
        }
        durationText = String(format: "%.3f", Double(viewModel.sequenceDurationMS) / 1000.0)
    }

    private var displayFilename: String {
        let p = viewModel.document.currentSequencePath()
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
        song    = viewModel.document.headerInfo(forKey: "song")
        artist  = viewModel.document.headerInfo(forKey: "artist")
        album   = viewModel.document.headerInfo(forKey: "album")
        author  = viewModel.document.headerInfo(forKey: "author")
        email   = viewModel.document.headerInfo(forKey: "email")
        website = viewModel.document.headerInfo(forKey: "website")
        url     = viewModel.document.headerInfo(forKey: "url")
        comment = viewModel.document.headerInfo(forKey: "comment")
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
/// this tab adds bulk import (.xtiming, LOR .lms, Papagayo .pgo,
/// .srt, Audacity .txt, ElevenLabs .json, Vixen 3 .tim, LSP .msq,
/// and timing tracks from an xLights .xsq), per-track `.xtiming`
/// export, and a single-place overview of every timing track in the
/// sequence.
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
    // TIM-3 multi-track export.
    @State private var showingMultiExportPicker = false
    @State private var multiExportSelection: Set<Int> = []
    @State private var multiExportDoc: MultiXTimingFile?

    // Multi-select import for Vixen3 (.tim) / xLights (.xsq) sequences, which
    // can hold several timing tracks. Discover the names, let the user choose
    // which to import (matches the desktop wxMultiChoiceDialog).
    @State private var showingMultiImportPicker = false
    @State private var importTrackNames: [String] = []
    @State private var importTrackSelection: Set<Int> = []
    @State private var importPendingPath: String? = nil
    @State private var importPendingIsVixen3 = false

    // Settings is itself presented as a sheet, so the app-level
    // `.sheet(isPresented: viewModel.showingAddTimingTrack)` can't
    // open on top of it (iOS only allows one sheet per ancestor
    // chain — flipping the global flag from inside this sheet logs
    // "Currently, only presenting a single sheet is supported"). A
    // local @State + a local `.sheet` attached to this view's body
    // stacks the new sheet correctly.
    @State private var showingAddTimingTrackLocal = false

    private enum ImportFormat: String, CaseIterable, Identifiable {
        case xtiming, lor, papagayo
        case srt, audacity, elevenLabs, vixen3, lsp, xlightsSeq
        var id: String { rawValue }
        var label: String {
            switch self {
            case .xtiming:    return "xLights (.xtiming)"
            case .lor:        return "LOR (.lms / .las)"
            case .papagayo:   return "Papagayo (.pgo)"
            case .srt:        return "Subtitles (.srt)"
            case .audacity:   return "Audacity Labels (.txt)"
            case .elevenLabs: return "ElevenLabs (.json)"
            case .vixen3:     return "Vixen 3 (.tim)"
            case .lsp:        return "LSP (.msq)"
            case .xlightsSeq: return "xLights sequence (.xsq)"
            }
        }
        var ext: String {
            switch self {
            case .xtiming:    return "xtiming"
            case .lor:        return "lms"
            case .papagayo:   return "pgo"
            case .srt:        return "srt"
            case .audacity:   return "txt"
            case .elevenLabs: return "json"
            case .vixen3:     return "tim"
            case .lsp:        return "msq"
            case .xlightsSeq: return "xsq"
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
                allowedContentTypes: importContentTypes,
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
            // Local sheet — stacked on top of the Settings sheet
            // when the user taps "+ Add Timing Track…".
            .sheet(isPresented: $showingAddTimingTrackLocal) {
                AddTimingTrackSheet()
                    .environment(viewModel)
            }
            // TIM-3 multi-track export: pick tracks, then export one .xtiming.
            .sheet(isPresented: $showingMultiExportPicker) {
                multiExportPicker
            }
            // Multi-select import: pick which Vixen3/xLights timing tracks to bring in.
            .sheet(isPresented: $showingMultiImportPicker) {
                multiImportPicker
            }
            .fileExporter(
                isPresented: Binding(
                    get: { multiExportDoc != nil },
                    set: { if !$0 { multiExportDoc = nil } }
                ),
                document: multiExportDoc,
                contentType: kXTimingFileType,
                defaultFilename: "Timings"
            ) { _ in
                multiExportDoc = nil
            }
    }

    // TIM-3 — multi-select sheet listing every timing track; Export
    // writes the checked ones into a single `<timings>` .xtiming.
    @ViewBuilder
    private var multiExportPicker: some View {
        NavigationStack {
            List {
                ForEach(timingRows) { tr in
                    Button {
                        if multiExportSelection.contains(tr.id) {
                            multiExportSelection.remove(tr.id)
                        } else {
                            multiExportSelection.insert(tr.id)
                        }
                    } label: {
                        HStack {
                            Image(systemName: multiExportSelection.contains(tr.id)
                                  ? "checkmark.circle.fill" : "circle")
                                .foregroundStyle(.tint)
                            Text(tr.name)
                            Spacer()
                        }
                    }
                    .buttonStyle(.plain)
                }
            }
            .navigationTitle("Export Timing Tracks")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .topBarLeading) {
                    Button("Cancel") { showingMultiExportPicker = false }
                }
                ToolbarItem(placement: .topBarTrailing) {
                    Button("Export") {
                        let rows = timingRows.map(\.id).filter { multiExportSelection.contains($0) }
                        multiExportDoc = MultiXTimingFile(rowIndices: rows, viewModel: viewModel)
                        showingMultiExportPicker = false
                    }
                    .disabled(multiExportSelection.isEmpty)
                }
            }
        }
    }

    // Multi-select sheet listing the timing tracks found in the chosen
    // Vixen3/xLights file; Import brings in the checked ones (empty = none).
    @ViewBuilder
    private var multiImportPicker: some View {
        NavigationStack {
            List {
                ForEach(Array(importTrackNames.enumerated()), id: \.offset) { idx, name in
                    Button {
                        if importTrackSelection.contains(idx) {
                            importTrackSelection.remove(idx)
                        } else {
                            importTrackSelection.insert(idx)
                        }
                    } label: {
                        HStack {
                            Image(systemName: importTrackSelection.contains(idx)
                                  ? "checkmark.circle.fill" : "circle")
                                .foregroundStyle(.tint)
                            Text(name)
                            Spacer()
                        }
                    }
                    .buttonStyle(.plain)
                }
            }
            .navigationTitle("Import Timing Tracks")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .topBarLeading) {
                    Button("Cancel") {
                        showingMultiImportPicker = false
                        importPendingPath = nil
                    }
                }
                ToolbarItem(placement: .topBarTrailing) {
                    Button("Import") {
                        if let path = importPendingPath {
                            let indices = importTrackNames.indices.filter { importTrackSelection.contains($0) }
                            if importPendingIsVixen3 {
                                viewModel.importVixen3Timing(path: path, selectedIndices: indices)
                            } else {
                                viewModel.importXLightsSequenceTiming(path: path, selectedIndices: indices)
                            }
                        }
                        showingMultiImportPicker = false
                        importPendingPath = nil
                    }
                    .disabled(importTrackSelection.isEmpty)
                }
            }
        }
    }

    @ViewBuilder
    private var list: some View {
        List {
            Section {
                Button {
                    showingAddTimingTrackLocal = true
                } label: {
                    Label("Add Timing Track…", systemImage: "plus.circle.fill")
                }
                if !timingRows.isEmpty {
                    Button {
                        multiExportSelection = Set(timingRows.map(\.id))
                        showingMultiExportPicker = true
                    } label: {
                        Label("Export Multiple Tracks…", systemImage: "square.and.arrow.up.on.square")
                    }
                }
                if timingRows.isEmpty {
                    Text("No timing tracks. Add one above, or use the Import button below.")
                        .foregroundStyle(.secondary)
                } else {
                    ForEach(timingRows) { tr in
                        timingRow(tr)
                    }
                }
            } header: {
                Text("Tracks")
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
        case .xtiming:    return kXTimingFileType
        case .lor:        return UTType(filenameExtension: "lms") ?? .xml
        case .papagayo:   return UTType(filenameExtension: "pgo") ?? .text
        case .srt:        return UTType(filenameExtension: "srt") ?? .text
        case .audacity:   return UTType(filenameExtension: "txt") ?? .plainText
        case .elevenLabs: return UTType(filenameExtension: "json") ?? .json
        case .vixen3:     return UTType(filenameExtension: "tim") ?? .xml
        case .lsp:        return UTType(filenameExtension: "msq") ?? .data
        case .xlightsSeq: return UTType(filenameExtension: "xsq") ?? .xml
        }
    }

    // SEQ-7 — one extension per format, except LOR which accepts both .lms
    // (musical) and .las (animation), matching desktop's ImportTimingElement.
    private var importContentTypes: [UTType] {
        if importFormat == .lor {
            return [UTType(filenameExtension: "lms"), UTType(filenameExtension: "las")].compactMap { $0 }
        }
        return [importContentType]
    }

    private func handleImport(result: Result<[URL], Error>) {
        guard case .success(let urls) = result, let url = urls.first else { return }
        let needsAccess = url.startAccessingSecurityScopedResource()
        defer { if needsAccess { url.stopAccessingSecurityScopedResource() } }
        _ = XLSequenceDocument.obtainAccess(toPath: url.path,
                                              enforceWritable: false)

        // Vixen3 / xLights sequences can hold several timing tracks: offer a
        // multi-select picker when there's more than one. obtainAccess above
        // persists a security-scoped bookmark, so the file stays reachable for
        // the deferred import performed from the picker.
        if importFormat == .vixen3 || importFormat == .xlightsSeq {
            let names = importFormat == .vixen3
                ? viewModel.vixen3TimingTrackNames(path: url.path)
                : viewModel.xLightsTimingTrackNames(path: url.path)
            if names.count > 1 {
                importTrackNames = names
                importTrackSelection = Set(names.indices)
                importPendingPath = url.path
                importPendingIsVixen3 = (importFormat == .vixen3)
                showingMultiImportPicker = true
                return
            }
            // 0 or 1 track: import directly (empty selection imports all).
            let added = importFormat == .vixen3
                ? viewModel.importVixen3Timing(path: url.path, selectedIndices: [])
                : viewModel.importXLightsSequenceTiming(path: url.path, selectedIndices: [])
            if added > 0 { viewModel.reloadRows() }
            return
        }

        let added: Int
        switch importFormat {
        case .xtiming:    added = viewModel.importXTiming(path: url.path)
        case .lor:        added = viewModel.importLorTiming(path: url.path)
        case .papagayo:   added = viewModel.importPapagayoTiming(path: url.path)
        case .srt:        added = viewModel.importSRTTiming(path: url.path)
        case .audacity:   added = viewModel.importAudacityTiming(path: url.path)
        case .elevenLabs: added = viewModel.importElevenLabsTiming(path: url.path)
        case .lsp:        added = viewModel.importLSPTiming(path: url.path)
        case .vixen3, .xlightsSeq: added = 0 // handled above
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

/// TIM-3 — exports multiple timing tracks into one `<timings>` .xtiming.
private struct MultiXTimingFile: FileDocument {
    static var readableContentTypes: [UTType] { [kXTimingFileType] }
    static var writableContentTypes: [UTType] { [kXTimingFileType] }

    let payload: Data

    @MainActor
    init?(rowIndices: [Int], viewModel: SequencerViewModel) {
        guard !rowIndices.isEmpty else { return nil }
        let tmp = URL(fileURLWithPath: NSTemporaryDirectory())
            .appendingPathComponent(UUID().uuidString)
            .appendingPathExtension("xtiming")
        defer { try? FileManager.default.removeItem(at: tmp) }
        guard viewModel.exportTimingTracks(rowIndices: rowIndices, path: tmp.path),
              let data = try? Data(contentsOf: tmp) else {
            return nil
        }
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
        let displayName = viewModel.document.altTrackDisplayName(at: idx)
        let path = viewModel.document.altTrackPath(at: idx)
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
                    renameText = viewModel.document.altTrackShortname(at: idx)
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
            Text("Remove \"\(viewModel.document.altTrackDisplayName(at: idx))\"? The audio file on disk is not deleted.")
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
