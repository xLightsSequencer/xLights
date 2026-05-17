import SwiftUI

// EX-11 — Tools → Package Sequence. Two toggles (Include Audio /
// Include Videos) then a single Pack action. On Pack the sheet
// dismisses, runs `SequencePackage::Pack` off the main actor, and
// hands the resulting `.xsqz` to the system share sheet. Errors
// surface as an inline alert; non-fatal warnings (e.g. missing
// referenced media) surface as a follow-up alert with the list.
struct PackageSequenceSheet: View {
    @Environment(\.dismiss) private var dismiss
    @Environment(SequencerViewModel.self) private var viewModel

    @State private var includeAudio: Bool = true
    @State private var includeVideos: Bool = true
    @State private var isPacking: Bool = false
    @State private var errorMessage: String? = nil
    @State private var pendingWarnings: [String] = []
    @State private var pendingShareURL: URL? = nil

    var body: some View {
        NavigationStack {
            Form {
                Section {
                    Toggle("Include audio", isOn: $includeAudio)
                    Toggle("Include videos", isOn: $includeVideos)
                } header: {
                    Text("Contents")
                } footer: {
                    Text("Excluding videos keeps the package smaller and avoids shipping copyrighted content. Audio is usually safe to include for a support repro.")
                }

                if let err = errorMessage {
                    Section {
                        Label(err, systemImage: "exclamationmark.triangle.fill")
                            .foregroundStyle(.red)
                    }
                }
            }
            .navigationTitle("Package Sequence")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                        .disabled(isPacking)
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Pack") { Task { await pack() } }
                        .disabled(isPacking)
                }
            }
            .overlay {
                if isPacking {
                    ZStack {
                        Color.black.opacity(0.2).ignoresSafeArea()
                        VStack(spacing: 12) {
                            ProgressView()
                            Text("Packing sequence…")
                                .font(.callout)
                                .foregroundStyle(.secondary)
                        }
                        .padding(24)
                        .background(.regularMaterial, in: RoundedRectangle(cornerRadius: 12))
                    }
                }
            }
            .alert("Sequence packaged with warnings", isPresented: warningsBinding) {
                Button("Share") {
                    if let url = pendingShareURL {
                        XLPresentShareSheet(items: [url])
                    }
                    pendingWarnings = []
                    pendingShareURL = nil
                    dismiss()
                }
                Button("Cancel", role: .cancel) {
                    pendingWarnings = []
                    pendingShareURL = nil
                }
            } message: {
                Text(pendingWarnings.prefix(8).joined(separator: "\n")
                     + (pendingWarnings.count > 8 ? "\n…and \(pendingWarnings.count - 8) more" : ""))
            }
        }
    }

    private var warningsBinding: Binding<Bool> {
        Binding(
            get: { !pendingWarnings.isEmpty },
            set: { if !$0 { pendingWarnings = [] } }
        )
    }

    @MainActor
    private func pack() async {
        errorMessage = nil
        isPacking = true
        defer { isPacking = false }

        let result = await viewModel.packageSequence(
            excludeAudio: !includeAudio,
            excludeVideos: !includeVideos)

        guard let result else {
            errorMessage = "Failed to package the sequence. Check the log for details."
            return
        }

        if result.warnings.isEmpty {
            XLPresentShareSheet(items: [result.url])
            dismiss()
        } else {
            // Defer the share sheet until the warnings alert is
            // dismissed so the two UIs don't fight.
            pendingShareURL = result.url
            pendingWarnings = result.warnings
        }
    }
}
