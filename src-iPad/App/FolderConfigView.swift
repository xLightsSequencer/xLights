import SwiftUI

/// Dialog for selecting the show folder and configuring media folders.
///
/// Presented at startup when no show folder has been configured, and from
/// the sequence picker/sequencer toolbar via a gear button.
struct FolderConfigView: View {
    @Environment(SequencerViewModel.self) var viewModel
    @Environment(\.dismiss) var dismiss

    @State private var showFolderPath: String?
    @State private var mediaFolderPaths: [String]
    @State private var fseqEnabled: Bool
    @State private var fseqFolderPath: String?
    @State private var pickerMode: PickerMode?
    @State private var recentFolders: [RecentShowFolders.Entry] = []

    enum PickerMode: Identifiable {
        case showFolder
        case addMediaFolder
        case fseqFolder
        var id: Int {
            switch self {
            case .showFolder: return 0
            case .addMediaFolder: return 1
            case .fseqFolder: return 2
            }
        }
    }

    init() {
        _showFolderPath = State(initialValue: FolderConfig.showFolder)
        _mediaFolderPaths = State(initialValue: FolderConfig.mediaFolders)
        _fseqEnabled = State(initialValue: FolderConfig.fseqEnabled)
        _fseqFolderPath = State(initialValue: FolderConfig.fseqFolder)
    }

    var body: some View {
        NavigationStack {
            Form {
                Section("Show Folder") {
                    if let path = showFolderPath {
                        Text(displayName(path))
                            .font(.body)
                        Text(path)
                            .font(.caption)
                            .foregroundStyle(.secondary)
                            .lineLimit(2)
                    } else {
                        Text("None selected")
                            .foregroundStyle(.secondary)
                    }
                    Button(showFolderPath == nil ? "Choose Show Folder…" : "Change Show Folder…") {
                        pickerMode = .showFolder
                    }
                }

                if !recentsToShow.isEmpty {
                    Section {
                        ForEach(recentsToShow) { entry in
                            Button {
                                showFolderPath = entry.path
                            } label: {
                                VStack(alignment: .leading, spacing: 2) {
                                    Text(entry.displayName)
                                        .foregroundStyle(.primary)
                                    Text(entry.path)
                                        .font(.caption)
                                        .foregroundStyle(.secondary)
                                        .lineLimit(2)
                                }
                            }
                        }
                        .onDelete { indexSet in
                            for idx in indexSet {
                                RecentShowFolders.remove(path: recentsToShow[idx].path)
                            }
                            recentFolders = RecentShowFolders.load()
                        }
                    } header: {
                        Text("Recent Show Folders")
                    } footer: {
                        Text("Tap to switch — remember to press Done to confirm.")
                    }
                }

                Section {
                    if mediaFolderPaths.isEmpty {
                        Text("No media folders configured")
                            .foregroundStyle(.secondary)
                    } else {
                        ForEach(mediaFolderPaths, id: \.self) { path in
                            VStack(alignment: .leading, spacing: 2) {
                                Text(displayName(path))
                                Text(path)
                                    .font(.caption)
                                    .foregroundStyle(.secondary)
                                    .lineLimit(2)
                            }
                        }
                        .onDelete { indexSet in
                            mediaFolderPaths.remove(atOffsets: indexSet)
                        }
                    }
                    Button("Add Media Folder…") {
                        pickerMode = .addMediaFolder
                    }
                } header: {
                    Text("Media Folders")
                } footer: {
                    Text("Media folders are searched for audio, shaders, and other assets referenced by sequences.")
                }

                Section {
                    Toggle("Save FSEQ on save", isOn: $fseqEnabled)
                    if fseqEnabled {
                        if let path = fseqFolderPath {
                            VStack(alignment: .leading, spacing: 2) {
                                Text(displayName(path))
                                Text(path)
                                    .font(.caption)
                                    .foregroundStyle(.secondary)
                                    .lineLimit(2)
                            }
                            Button("Change FSEQ Folder…") {
                                pickerMode = .fseqFolder
                            }
                            Button("Use sequence's folder", role: .destructive) {
                                fseqFolderPath = nil
                            }
                        } else {
                            Text("Next to sequence file")
                                .foregroundStyle(.secondary)
                            Button("Choose FSEQ Folder…") {
                                pickerMode = .fseqFolder
                            }
                        }
                    }
                } header: {
                    Text("FSEQ Files")
                } footer: {
                    Text("FSEQ files are pre-rendered playback files used by Falcon Player and other controllers. When no folder is chosen, the FSEQ is written next to the sequence file.")
                }
            }
            .navigationTitle("Folders")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") {
                        apply()
                        dismiss()
                    }
                    .disabled(showFolderPath == nil)
                }
            }
            .onAppear {
                recentFolders = RecentShowFolders.load()
            }
            .sheet(item: $pickerMode) { mode in
                ShowFolderPicker { url in
                    let path = url.path
                    FolderConfig.registerBookmark(from: url)
                    switch mode {
                    case .showFolder:
                        showFolderPath = path
                    case .addMediaFolder:
                        if !mediaFolderPaths.contains(path) {
                            mediaFolderPaths.append(path)
                        }
                    case .fseqFolder:
                        fseqFolderPath = path
                    }
                    pickerMode = nil
                }
            }
        }
    }

    private func displayName(_ path: String) -> String {
        (path as NSString).lastPathComponent
    }

    /// Recents minus whatever is currently selected — no point listing
    /// the show folder we're already on.
    private var recentsToShow: [RecentShowFolders.Entry] {
        guard let current = showFolderPath else { return recentFolders }
        return recentFolders.filter { $0.path != current }
    }

    private func apply() {
        guard let path = showFolderPath else { return }
        FolderConfig.showFolder = path
        FolderConfig.mediaFolders = mediaFolderPaths
        FolderConfig.fseqEnabled = fseqEnabled
        FolderConfig.fseqFolder = fseqEnabled ? fseqFolderPath : nil
        viewModel.loadShowFolder(path: path, mediaFolders: mediaFolderPaths)
    }
}
