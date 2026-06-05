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
    @State private var baseShowFolderPath: String?
    @State private var autoUpdateFromBase: Bool = false
    @State private var updateResult: BaseDirUpdateResult?
    @State private var reselectPrompt: ReselectPrompt?
    // App-wide render preference (default OFF = full-definition render). Not
    // per-sequence — `iPadRenderContext::IsLowDefinitionRender()` reads this
    // same `render.lowDefinition` key via CFPreferences.
    @AppStorage("render.lowDefinition") private var lowDefinitionRender: Bool = false
    // App-wide render-cache mode (default "Disabled"). Not per-sequence —
    // `iPadRenderContext::ReadRenderCacheMode()` reads this same `render.cacheMode`
    // key via CFPreferences and feeds it to `RenderCache::Enable`. Values must
    // match exactly: "Disabled" | "Locked Only" | "Enabled".
    @AppStorage("render.cacheMode") private var renderCacheMode: String = "Disabled"

    // FSEQ-1 — `iPadRenderContext::ReadFseqCompression()` /
    // `ReadFseqCompressionLevel()` read these same keys via CFPreferences and
    // feed `FSEQFile::createFSEQFile`. Compression values must match exactly:
    // "zstd" | "zlib" | "none"; level is the zstd level 1..22.
    @AppStorage("fseq.compression") private var fseqCompression: String = "zstd"
    @AppStorage("fseq.compressionLevel") private var fseqCompressionLevel: Int = 2

    enum PickerMode: Identifiable {
        case showFolder
        case addMediaFolder
        case fseqFolder
        case baseShowFolder
        // Picks a base show folder then immediately retries the merge — used by the stale-bookmark reselect flow.
        case baseShowFolderRetryUpdate
        var id: Int {
            switch self {
            case .showFolder: return 0
            case .addMediaFolder: return 1
            case .fseqFolder: return 2
            case .baseShowFolder: return 3
            case .baseShowFolderRetryUpdate: return 4
            }
        }
    }

    struct BaseDirUpdateResult: Identifiable {
        let id = UUID()
        let success: Bool
        let title: String
        let message: String
    }

    struct ReselectPrompt: Identifiable {
        let id = UUID()
        let message: String
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

                if showFolderPath != nil {
                    baseShowFolderSection
                }

                Section {
                    Toggle(isOn: $lowDefinitionRender) {
                        VStack(alignment: .leading, spacing: 2) {
                            Text("Low Definition Render")
                            Text("When on, large matrix/sphere models render effects into reduced-resolution buffers (which look blocky on high-resolution props) to lower memory use. Leave OFF for full-resolution output — the default, matching the desktop. Turn it on only if very large shows run out of memory. Applies when a sequence is opened.")
                                .font(.caption2)
                                .foregroundStyle(.secondary)
                        }
                    }

                    VStack(alignment: .leading, spacing: 6) {
                        Picker("Render Cache", selection: $renderCacheMode) {
                            Text("Disabled").tag("Disabled")
                            Text("Locked Only").tag("Locked Only")
                            Text("Enabled").tag("Enabled")
                        }
                        Text("Caches rendered effect frames to disk so re-renders are faster. This costs both memory and disk, which are limited on iPad, so it's OFF by default. \"Locked Only\" caches just the effects you've locked (the desktop default); \"Enabled\" caches every supported effect. Takes effect on the next render.")
                            .font(.caption2)
                            .foregroundStyle(.secondary)
                    }
                } header: {
                    Text("Rendering")
                } footer: {
                    Text("App-wide settings (not saved in the sequence).")
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

                        // FSEQ-1 — compression format for written FSEQ files.
                        VStack(alignment: .leading, spacing: 6) {
                            Picker("Compression", selection: $fseqCompression) {
                                Text("Zstd (default)").tag("zstd")
                                Text("Zlib").tag("zlib")
                                Text("None").tag("none")
                            }
                            if fseqCompression == "zstd" {
                                Stepper(value: $fseqCompressionLevel, in: 1...22) {
                                    Text("Zstd Level: \(fseqCompressionLevel)")
                                }
                            }
                            Text("Compression shrinks the FSEQ at some render-time cost. Zstd is fastest and the desktop default (level 1 = fast/larger, 22 = slow/smaller); Zlib is slightly smaller but slower; None is uncompressed. Takes effect on the next save/render.")
                                .font(.caption2)
                                .foregroundStyle(.secondary)
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
                baseShowFolderPath = viewModel.document.baseShowDirectory()
                autoUpdateFromBase = viewModel.document.autoUpdateFromBaseShowDirectory()
                if let deferred = FolderConfig.pendingBaseDirReselectMessage {
                    FolderConfig.pendingBaseDirReselectMessage = nil
                    reselectPrompt = ReselectPrompt(message: deferred)
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
                    case .fseqFolder:
                        fseqFolderPath = path
                    case .baseShowFolder:
                        baseShowFolderPath = path
                    case .baseShowFolderRetryUpdate:
                        baseShowFolderPath = path
                        commitBaseShowDirectoryEdits()
                        performUpdateFromBase()
                    }
                    pickerMode = nil
                }
            }
            .alert(item: $updateResult) { result in
                Alert(title: Text(result.title),
                      message: Text(result.message),
                      dismissButton: .default(Text("OK")))
            }
            .alert(item: $reselectPrompt) { prompt in
                Alert(title: Text("Reselect Base Show Folder"),
                      message: Text(prompt.message),
                      primaryButton: .default(Text("Reselect…")) {
                          pickerMode = .baseShowFolderRetryUpdate
                      },
                      secondaryButton: .cancel())
            }
        }
    }

    @ViewBuilder
    private var baseShowFolderSection: some View {
        Section {
            if let path = baseShowFolderPath {
                VStack(alignment: .leading, spacing: 2) {
                    Text(displayName(path))
                    Text(path)
                        .font(.caption)
                        .foregroundStyle(.secondary)
                        .lineLimit(2)
                }
                Button("Change Base Show Folder…") {
                    pickerMode = .baseShowFolder
                }
                Button("Clear Base Show Folder", role: .destructive) {
                    baseShowFolderPath = nil
                    autoUpdateFromBase = false
                }
                Toggle("Auto Update On Open", isOn: $autoUpdateFromBase)
                Button("Update From Base Now") {
                    performUpdateFromBase()
                }
                .disabled(path == showFolderPath)
            } else {
                Text("No base show folder")
                    .foregroundStyle(.secondary)
                Button("Choose Base Show Folder…") {
                    pickerMode = .baseShowFolder
                }
            }
        } header: {
            Text("Base Show Folder")
        } footer: {
            Text("A base show folder lets this show pull in controllers, models, and view objects from a master folder. Turn on Auto Update to merge changes every time the show opens; tap Update From Base Now to merge on demand.")
        }
    }

    private func performUpdateFromBase() {
        // Flush pending edits before the merge so Update Now operates against the current path, not the previously-saved one.
        commitBaseShowDirectoryEdits()

        let result = viewModel.document.updateFromBaseShowDirectory()

        if let error = result["error"] as? String {
            let needsReselect = result["needsReselect"] as? Bool ?? false
            if needsReselect {
                reselectPrompt = ReselectPrompt(message: error)
            } else {
                updateResult = BaseDirUpdateResult(
                    success: false,
                    title: "Update Failed",
                    message: error)
            }
            return
        }

        let controllersChanged = result["controllersChanged"] as? Bool ?? false
        let modelsChanged = result["modelsChanged"] as? Bool ?? false
        let objectsChanged = result["objectsChanged"] as? Bool ?? false
        _ = viewModel.document.saveLayoutChanges()

        if !controllersChanged && !modelsChanged && !objectsChanged {
            updateResult = BaseDirUpdateResult(
                success: true,
                title: "Already Up To Date",
                message: "Nothing to merge — the base show folder hasn't changed anything since the last update.")
            return
        }

        var parts: [String] = []
        if controllersChanged { parts.append("controllers") }
        if modelsChanged { parts.append("models") }
        if objectsChanged { parts.append("view objects") }
        updateResult = BaseDirUpdateResult(
            success: true,
            title: "Updated From Base",
            message: "Merged " + parts.joined(separator: ", ") + " from the base show folder.")
    }

    private func commitBaseShowDirectoryEdits() {
        viewModel.document.setBaseShowDirectory(baseShowFolderPath)
        viewModel.document.setAutoUpdateFromBaseShowDirectory(autoUpdateFromBase)
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
        let priorShowFolder = FolderConfig.showFolder

        FolderConfig.showFolder = path
        FolderConfig.mediaFolders = mediaFolderPaths
        FolderConfig.fseqEnabled = fseqEnabled
        FolderConfig.fseqFolder = fseqEnabled ? fseqFolderPath : nil

        if path == priorShowFolder {
            // Same show: base-dir edits apply to the loaded OutputManager — flush them. Different show: edits target the about-to-be-unloaded OM; loadShowFolder brings up the new show's own base-dir state.
            commitBaseShowDirectoryEdits()
            _ = viewModel.document.saveLayoutChanges()
        } else {
            viewModel.loadShowFolder(path: path, mediaFolders: mediaFolderPaths)
        }
    }
}
