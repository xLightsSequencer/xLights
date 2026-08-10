import SwiftUI

/// Restore Backup sheet — two restore sources in one place:
///
/// 1. **Show Folder Backups** — desktop-format backup runs
///    (`<show>/Backup/<date>-<time>/` directories written by File →
///    Back Up Show Folder, desktop's F10, or either platform's
///    backup-on-launch/save). Drills into a run to restore selected
///    configuration files and sequences, mirroring desktop's
///    `RestoreBackupDialog`.
/// 2. **Sequence snapshots** — the iPad's Backup-On-Save snapshot ring
///    for the currently-open sequence (flat timestamped files in the
///    same `Backup/` directory).
struct RestoreBackupSheet: View {
    @Environment(SequencerViewModel.self) var viewModel
    @Environment(\.dismiss) var dismiss

    @State private var runs: [ShowFolderBackup.BackupRun] = []
    @State private var snapshots: [SequencerViewModel.BackupSnapshot] = []
    @State private var restoreTarget: SequencerViewModel.BackupSnapshot?
    @State private var failureMessage: String?

    private static let dateFormatter: DateFormatter = {
        let f = DateFormatter()
        f.dateStyle = .medium
        f.timeStyle = .medium
        return f
    }()

    private var isPackaged: Bool { viewModel.document.isPackagedSequence() }
    private var showSnapshotSection: Bool { viewModel.isSequenceLoaded && !isPackaged }

    var body: some View {
        NavigationStack {
            Group {
                if runs.isEmpty && (!showSnapshotSection || snapshots.isEmpty) {
                    ContentUnavailableView(
                        "No Backups",
                        systemImage: "clock.arrow.circlepath",
                        description: Text("Show-folder backups appear here after using File → Back Up Show Folder (or desktop's F10 Backup). Sequence snapshots appear once Backup On Save is enabled and you save the sequence."))
                } else {
                    List {
                        if !runs.isEmpty {
                            Section {
                                ForEach(runs) { run in
                                    NavigationLink(value: run) {
                                        runRow(run)
                                    }
                                }
                            } header: {
                                Text("Show Folder Backups")
                            } footer: {
                                Text("Full backups of the show folder's sequence and configuration files. Backups made on the desktop appear here too.")
                            }
                        }
                        if showSnapshotSection && !snapshots.isEmpty {
                            Section {
                                ForEach(snapshots) { snap in
                                    Button {
                                        restoreTarget = snap
                                    } label: {
                                        VStack(alignment: .leading, spacing: 3) {
                                            Text(Self.dateFormatter.string(from: snap.modified))
                                                .foregroundStyle(.primary)
                                            HStack(spacing: 8) {
                                                Text(snap.fileName)
                                                    .lineLimit(1)
                                                    .truncationMode(.middle)
                                                Spacer()
                                                Text(byteString(snap.sizeBytes))
                                            }
                                            .font(.caption)
                                            .foregroundStyle(.secondary)
                                        }
                                    }
                                }
                            } header: {
                                Text("Sequence Snapshots")
                            } footer: {
                                Text("Restoring a snapshot overwrites the current sequence. The current state is snapshotted first, so a restore can itself be undone.")
                            }
                        }
                    }
                }
            }
            .navigationTitle("Restore Backup")
            .navigationBarTitleDisplayMode(.inline)
            .navigationDestination(for: ShowFolderBackup.BackupRun.self) { run in
                ShowFolderBackupRunView(run: run)
                    .environment(viewModel)
            }
            .toolbar {
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") { dismiss() }
                }
            }
            .onAppear {
                runs = ShowFolderBackup.availableRuns(showFolder: viewModel.showFolderPath ?? "")
                snapshots = viewModel.availableBackupSnapshots()
            }
            .confirmationDialog(
                "Restore this backup?",
                isPresented: Binding(
                    get: { restoreTarget != nil },
                    set: { if !$0 { restoreTarget = nil } }),
                titleVisibility: .visible,
                presenting: restoreTarget
            ) { snap in
                Button("Restore", role: .destructive) {
                    Task { @MainActor in
                        if await viewModel.restoreBackup(snap) {
                            dismiss()
                        } else {
                            failureMessage = "Could not restore \(snap.fileName)."
                        }
                    }
                }
                Button("Cancel", role: .cancel) { }
            } message: { snap in
                Text("The current sequence will be replaced with the snapshot from \(Self.dateFormatter.string(from: snap.modified)).")
            }
            .alert("Restore Failed",
                   isPresented: Binding(
                    get: { failureMessage != nil },
                    set: { if !$0 { failureMessage = nil } })) {
                Button("OK", role: .cancel) { failureMessage = nil }
            } message: {
                Text(failureMessage ?? "")
            }
        }
    }

    @ViewBuilder
    private func runRow(_ run: ShowFolderBackup.BackupRun) -> some View {
        VStack(alignment: .leading, spacing: 3) {
            HStack(spacing: 8) {
                Text(run.date.map { Self.dateFormatter.string(from: $0) } ?? run.name)
                    .foregroundStyle(.primary)
                if run.isOnStart {
                    Text("on open")
                        .font(.caption2)
                        .padding(.horizontal, 6)
                        .padding(.vertical, 2)
                        .background(.quaternary, in: Capsule())
                }
            }
            Text(run.name)
                .font(.caption)
                .foregroundStyle(.secondary)
        }
    }

    private func byteString(_ bytes: Int64) -> String {
        ByteCountFormatter.string(fromByteCount: bytes, countStyle: .file)
    }
}

/// Detail view for one show-folder backup run: pick which configuration
/// files (pre-checked, as desktop pre-checks them) and sequences (not
/// pre-checked) to copy back over the show folder. Restore saves and
/// closes any open sequence, takes a safety backup, copies, and reloads
/// the show folder.
struct ShowFolderBackupRunView: View {
    @Environment(SequencerViewModel.self) var viewModel
    let run: ShowFolderBackup.BackupRun

    @State private var configFiles: [String] = []
    @State private var sequenceFiles: [String] = []
    @State private var selected: Set<String> = []
    @State private var loaded = false
    @State private var confirmingRestore = false
    @State private var isRestoring = false
    @State private var errorMessage: String?

    var body: some View {
        List {
            if configFiles.isEmpty && sequenceFiles.isEmpty {
                ContentUnavailableView(
                    "Empty Backup",
                    systemImage: "folder",
                    description: Text("This backup contains no restorable files."))
            }
            if !configFiles.isEmpty {
                Section {
                    ForEach(configFiles, id: \.self) { file in
                        selectionRow(file, title: file)
                    }
                } header: {
                    Text("Configuration Files")
                } footer: {
                    Text("Controllers, layout, and effect presets. Restoring these replaces the show's current configuration.")
                }
            }
            if !sequenceFiles.isEmpty {
                Section {
                    ForEach(sequenceFiles, id: \.self) { file in
                        selectionRow(file, title: file)
                    }
                } header: {
                    Text("Sequences")
                }
            }
        }
        .navigationTitle(run.name)
        .navigationBarTitleDisplayMode(.inline)
        .toolbar {
            ToolbarItem(placement: .confirmationAction) {
                if isRestoring {
                    ProgressView()
                } else {
                    Button("Restore") { confirmingRestore = true }
                        .disabled(selected.isEmpty)
                }
            }
        }
        .onAppear {
            guard !loaded else { return }
            loaded = true
            configFiles = ShowFolderBackup.configFiles(inRun: run.path)
            sequenceFiles = ShowFolderBackup.sequenceFiles(inRun: run.path)
            selected = Set(configFiles)
        }
        .confirmationDialog(
            "Restore \(selected.count) file(s)?",
            isPresented: $confirmingRestore,
            titleVisibility: .visible
        ) {
            Button("Restore", role: .destructive) { performRestore() }
            Button("Cancel", role: .cancel) { }
        } message: {
            Text("This will overwrite the current files in the show folder. Any open sequence is saved and closed, a safety backup is taken first, and the show folder reloads afterwards.")
        }
        .alert("Restore Failed",
               isPresented: Binding(
                get: { errorMessage != nil },
                set: { if !$0 { errorMessage = nil } })) {
            Button("OK", role: .cancel) { errorMessage = nil }
        } message: {
            Text(errorMessage ?? "")
        }
        .interactiveDismissDisabled(isRestoring)
    }

    @ViewBuilder
    private func selectionRow(_ id: String, title: String) -> some View {
        Button {
            if selected.contains(id) {
                selected.remove(id)
            } else {
                selected.insert(id)
            }
        } label: {
            HStack {
                Image(systemName: selected.contains(id) ? "checkmark.circle.fill" : "circle")
                    .foregroundStyle(selected.contains(id) ? Color.accentColor : Color.secondary)
                Text(title)
                    .foregroundStyle(.primary)
                    .lineLimit(1)
                    .truncationMode(.middle)
            }
        }
        .disabled(isRestoring)
    }

    private func performRestore() {
        // Config files first, in desktop's order, then sequences.
        let files = configFiles.filter { selected.contains($0) }
                  + sequenceFiles.filter { selected.contains($0) }
        isRestoring = true
        Task { @MainActor in
            let errors = await viewModel.restoreShowFolderBackup(run: run, files: files)
            isRestoring = false
            if errors.isEmpty {
                viewModel.showFolderBackupResult =
                    "Restored \(files.count) file(s) from \(run.name). The show folder was reloaded."
                viewModel.showingRestoreBackup = false
            } else {
                errorMessage = errors.joined(separator: "\n")
            }
        }
    }
}
