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
    @State private var pickerMode: PickerMode?

    enum PickerMode: Identifiable {
        case showFolder
        case addMediaFolder
        var id: Int { self == .showFolder ? 0 : 1 }
    }

    init() {
        _showFolderPath = State(initialValue: FolderConfig.showFolder)
        _mediaFolderPaths = State(initialValue: FolderConfig.mediaFolders)
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
                    }
                    pickerMode = nil
                }
            }
        }
    }

    private func displayName(_ path: String) -> String {
        (path as NSString).lastPathComponent
    }

    private func apply() {
        guard let path = showFolderPath else { return }
        FolderConfig.showFolder = path
        FolderConfig.mediaFolders = mediaFolderPaths
        viewModel.loadShowFolder(path: path, mediaFolders: mediaFolderPaths)
    }
}
