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
        case info, metadata, media, render
        var id: String { rawValue }
        var label: String {
            switch self {
            case .info:     return "Info"
            case .metadata: return "Metadata"
            case .media:    return "Media"
            case .render:   return "Render"
            }
        }
        var symbol: String {
            switch self {
            case .info:     return "info.circle"
            case .metadata: return "character.book.closed"
            case .media:    return "photo.stack"
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
