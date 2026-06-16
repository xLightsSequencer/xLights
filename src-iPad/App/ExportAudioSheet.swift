import SwiftUI

/// Tools → Export Audio. Encodes the audio the user currently has
/// displayed on the waveform (whole mix, an isolated stem, a filtered
/// band, …) to a chosen container and hands it to the system share
/// sheet. Mirrors the desktop's audio-export / encode path
/// (`AudioManager::WriteCurrentAudio`), with WAV (uncompressed) and
/// M4A (AAC, AudioToolbox) format choices.
struct ExportAudioSheet: View {
    @Environment(SequencerViewModel.self) var viewModel
    @Environment(\.dismiss) private var dismiss

    private enum AudioFormat: String, CaseIterable, Identifiable {
        case wav, m4a
        var id: String { rawValue }
        var ext: String { rawValue }
        var title: String {
            switch self {
            case .wav: return "WAV (uncompressed)"
            case .m4a: return "M4A (AAC)"
            }
        }
    }

    @State private var format: AudioFormat = .m4a
    @State private var working = false
    @State private var errorMessage: String? = nil

    var body: some View {
        NavigationStack {
            Form {
                Section("Format") {
                    Picker("Format", selection: $format) {
                        ForEach(AudioFormat.allCases) { f in
                            Text(f.title).tag(f)
                        }
                    }
                    .pickerStyle(.inline)
                    .labelsHidden()
                }
                Section {
                    Text("Exports the audio currently shown on the waveform — including any isolated stem or filter band you have selected.")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
            }
            .navigationTitle("Export Audio")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .topBarLeading) {
                    Button("Cancel") { dismiss() }
                }
                ToolbarItem(placement: .topBarTrailing) {
                    Button("Export") { exportAudio() }
                        .disabled(working || !viewModel.hasExportableAudio)
                }
            }
            .overlay {
                if working {
                    ZStack {
                        Color.black.opacity(0.4).ignoresSafeArea()
                        VStack(spacing: 12) {
                            ProgressView().controlSize(.large)
                            Text("Encoding audio…").font(.headline)
                        }
                        .padding(24)
                        .background(.thinMaterial, in: RoundedRectangle(cornerRadius: 14))
                    }
                }
            }
            .alert("Export Audio",
                   isPresented: Binding(
                    get: { errorMessage != nil },
                    set: { if !$0 { errorMessage = nil } }
                   )) {
                Button("OK", role: .cancel) { errorMessage = nil }
            } message: {
                Text(errorMessage ?? "")
            }
        }
    }

    private func exportAudio() {
        working = true
        let ext = format.ext
        Task { @MainActor in
            // Encode off the main run loop's immediate frame; the
            // AudioToolbox encode for a few-minute track is quick but
            // we still show the spinner.
            let url = viewModel.exportCurrentAudio(ext: ext)
            working = false
            guard let url = url else {
                errorMessage = "Couldn't encode the audio. Make sure a sequence with audio is loaded."
                return
            }
            dismiss()
            XLPresentShareSheet(items: [url])
        }
    }
}
