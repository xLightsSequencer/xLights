import SwiftUI

/// Theme-09 — per-model render progress, the touch counterpart to
/// desktop's `RenderProgressDialog` (one gauge + status line per render
/// job). Desktop opens it by double-clicking the status-bar gauge; here
/// it's a long-press on the toolbar render button.
///
/// Polls the bridge on a timer rather than being pushed to, because the
/// status strings are only worth building while someone is looking at
/// them — the same reason desktop gates its own poll on the dialog
/// being shown.
struct RenderProgressSheet: View {
    let viewModel: SequencerViewModel
    @Environment(\.dismiss) private var dismiss

    @State private var jobs: [RenderJobRow] = []
    @State private var pollTimer: Timer? = nil

    var body: some View {
        NavigationStack {
            Group {
                if jobs.isEmpty {
                    ContentUnavailableView("No render in progress",
                                           systemImage: "paintpalette",
                                           description: Text("Per-model progress appears here while a render is running."))
                } else {
                    List(jobs) { job in
                        VStack(alignment: .leading, spacing: 4) {
                            HStack {
                                Text(job.model)
                                    .font(.body.weight(.medium))
                                    .lineLimit(1)
                                    .truncationMode(.middle)
                                Spacer()
                                Text("\(job.percent)%")
                                    .font(.caption.monospacedDigit())
                                    .foregroundStyle(.secondary)
                            }
                            ProgressView(value: Double(job.percent), total: 100)
                            if !job.status.isEmpty {
                                Text(job.status)
                                    .font(.caption)
                                    .foregroundStyle(.secondary)
                                    .lineLimit(2)
                            }
                        }
                        .padding(.vertical, 2)
                    }
                }
            }
            .navigationTitle("Render Progress")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Done") { dismiss() }
                }
            }
        }
        .presentationDetents([.medium, .large])
        .onAppear { startPolling() }
        .onDisappear { stopPolling() }
    }

    private func refresh() {
        jobs = viewModel.document.renderJobProgress().map { d in
            RenderJobRow(model: (d["model"] as? String) ?? "",
                          percent: (d["percent"] as? NSNumber)?.intValue ?? 0,
                          status: (d["status"] as? String) ?? "")
        }
    }

    private func startPolling() {
        refresh()
        stopPolling()
        pollTimer = Timer.scheduledTimer(withTimeInterval: 0.25, repeats: true) { _ in
            MainActor.assumeIsolated { refresh() }
        }
    }

    private func stopPolling() {
        pollTimer?.invalidate()
        pollTimer = nil
    }
}

struct RenderJobRow: Identifiable {
    // Job names are per-model and unique within a render batch.
    var id: String { model }
    let model: String
    let percent: Int
    let status: String
}
