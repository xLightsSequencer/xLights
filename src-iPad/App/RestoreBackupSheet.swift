import SwiftUI

/// Restore Backup sheet — consumes the Backup-On-Save snapshots that
/// `SequencerViewModel.writeSaveBackup` writes into `<show>/Backup/`.
/// Lists the snapshots for the currently-open sequence (newest first)
/// with date + size, and restores a chosen one over the working file
/// after safety-snapshotting the current state. Desktop analogue:
/// `RestoreBackupDialog` (`xLightsMain.cpp` `ID_FILE_RESTOREBACKUP`),
/// scoped here to the per-sequence in-show snapshots iOS can manage.
struct RestoreBackupSheet: View {
    @Environment(SequencerViewModel.self) var viewModel
    @Environment(\.dismiss) var dismiss

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

    var body: some View {
        NavigationStack {
            Group {
                if isPackaged {
                    ContentUnavailableView(
                        "Packaged Sequence",
                        systemImage: "shippingbox",
                        description: Text("Restore Backup is available for unpackaged .xsq sequences. Open the sequence directly (not as a package) to restore a snapshot."))
                } else if snapshots.isEmpty {
                    ContentUnavailableView(
                        "No Backups",
                        systemImage: "clock.arrow.circlepath",
                        description: Text("Backup snapshots appear here once Backup On Save is enabled and you save the sequence. Snapshots are kept in the show folder's Backup directory."))
                } else {
                    List {
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
                        } footer: {
                            Text("Restoring overwrites the current sequence. The current state is snapshotted first, so a restore can itself be undone.")
                        }
                    }
                }
            }
            .navigationTitle("Restore Backup")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") { dismiss() }
                }
            }
            .onAppear { snapshots = viewModel.availableBackupSnapshots() }
            .confirmationDialog(
                "Restore this backup?",
                isPresented: Binding(
                    get: { restoreTarget != nil },
                    set: { if !$0 { restoreTarget = nil } }),
                titleVisibility: .visible,
                presenting: restoreTarget
            ) { snap in
                Button("Restore", role: .destructive) {
                    if viewModel.restoreBackup(snap) {
                        dismiss()
                    } else {
                        failureMessage = "Could not restore \(snap.fileName)."
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

    private func byteString(_ bytes: Int64) -> String {
        ByteCountFormatter.string(fromByteCount: bytes, countStyle: .file)
    }
}
