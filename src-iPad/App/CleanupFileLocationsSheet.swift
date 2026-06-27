import SwiftUI

// CLN-1 — Tools → Cleanup File Locations. Parity with desktop
// xLightsFrame::OnMenuItem_CleanupFileLocationsSelected: sweeps every
// referenced external media file that lives outside the show / media
// folders into the show folder and rewrites the effect references
// that pointed at the old location.
//
// The sheet previews the files that would move before doing anything,
// and warns the operation is NOT undoable (matching desktop, which
// warns rather than registering an undo step). An empty preview means
// every referenced file is already inside the show / media folders.

struct CleanupFileLocationsSheet: View {
    @Environment(\.dismiss) private var dismiss
    @Environment(SequencerViewModel.self) private var viewModel

    private struct Move: Identifiable {
        let id = UUID()
        let from: String
        let to: String
    }

    @State private var moves: [Move] = []
    @State private var didLoad = false
    @State private var resultCount: Int? = nil

    var body: some View {
        NavigationStack {
            Group {
                if !didLoad {
                    ProgressView()
                        .frame(maxWidth: .infinity, maxHeight: .infinity)
                } else if moves.isEmpty {
                    ContentUnavailableView(
                        "Nothing to Clean Up",
                        systemImage: "checkmark.circle",
                        description: Text("Every media file referenced by this sequence already lives inside your show or media folders.")
                    )
                } else {
                    List {
                        Section {
                            HStack(spacing: 8) {
                                Image(systemName: "exclamationmark.triangle.fill")
                                    .foregroundStyle(.orange)
                                Text("\(moves.count) file\(moves.count == 1 ? "" : "s") outside your show folder will be copied in and their effect references rewritten. This cannot be undone.")
                                    .font(.caption)
                            }
                        }
                        Section("Files to Move") {
                            ForEach(moves) { move in
                                VStack(alignment: .leading, spacing: 2) {
                                    Text((move.to as NSString).lastPathComponent)
                                        .font(.callout)
                                        .lineLimit(1)
                                        .truncationMode(.middle)
                                    Text(move.from)
                                        .font(.caption2)
                                        .foregroundStyle(.secondary)
                                        .lineLimit(1)
                                        .truncationMode(.middle)
                                    Text("→ \(move.to)")
                                        .font(.caption2)
                                        .foregroundStyle(.blue)
                                        .lineLimit(1)
                                        .truncationMode(.middle)
                                }
                            }
                        }
                    }
                }
            }
            .navigationTitle("Cleanup File Locations")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Clean Up") { performCleanup() }
                        .disabled(moves.isEmpty)
                }
            }
        }
        .onAppear { load() }
        .alert("Cleanup Complete",
               isPresented: Binding(
                get: { resultCount != nil },
                set: { if !$0 { resultCount = nil } }
               )) {
            Button("OK", role: .cancel) {
                resultCount = nil
                dismiss()
            }
        } message: {
            let n = resultCount ?? 0
            Text("Moved \(n) media file\(n == 1 ? "" : "s") into the show folder.")
        }
    }

    private func load() {
        let raw = viewModel.document.cleanupFileLocationsPreview()
        moves = raw.compactMap { d in
            guard let from = d["from"], let to = d["to"] else { return nil }
            return Move(from: from, to: to)
        }
        didLoad = true
    }

    private func performCleanup() {
        resultCount = Int(viewModel.document.performCleanupFileLocations())
    }
}
