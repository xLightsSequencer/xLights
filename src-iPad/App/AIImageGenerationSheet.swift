import SwiftUI
import UIKit

/// AI image generation sheet — presented from the Pictures effect's
/// filename block when the user taps "AI…". Wraps `XLAIImageSession`
/// (which owns an `aiBase::AIImageGenerator`). Mirrors the desktop
/// `AIImageDialog` in embedded mode: prompt entry, generate button,
/// preview, and a Use Image action that saves the PNG into the
/// show folder's `AIImages/` subdirectory and writes the resulting
/// path back to the calling effect's `E_TEXTCTRL_Pictures_Filename`
/// setting.
///
/// `onCommitPath` receives a relative path (`AIImages/foo.png`) where
/// possible, falling back to the absolute path if the show folder
/// hasn't been resolved.
struct AIImageGenerationSheet: View {
    @Environment(SequencerViewModel.self) var viewModel
    @Environment(\.dismiss) private var dismiss

    let onCommitPath: (String) -> Void

    /// All services that have IMAGES enabled + a configured key.
    @State private var availableServices: [String] = []
    @State private var selectedService: String = ""

    @State private var session: XLAIImageSession? = nil
    @State private var prompt: String = ""

    @State private var status: Status = .idle
    @State private var pngData: Data? = nil
    @State private var preview: UIImage? = nil

    private enum Status: Equatable {
        case idle
        case running
        case ready
        case error(String)
    }

    var body: some View {
        NavigationStack {
            Form {
                Section("Service") {
                    if availableServices.isEmpty {
                        Text("""
                             No image-generation service is configured. \
                             Open Tools → AI Services… to set up Claude, \
                             ChatGPT, or another image-capable provider.
                             """)
                            .font(.footnote)
                            .foregroundStyle(.secondary)
                    } else if availableServices.count == 1 {
                        LabeledContent("Service", value: availableServices[0])
                    } else {
                        Picker("Service", selection: $selectedService) {
                            ForEach(availableServices, id: \.self) { name in
                                Text(name).tag(name)
                            }
                        }
                        .onChange(of: selectedService) { _, newValue in
                            session = XLAIImageSession.session(forService: newValue)
                            resetResult()
                        }
                    }
                }

                Section {
                    TextEditor(text: $prompt)
                        .frame(minHeight: 120)
                        .overlay(alignment: .topLeading) {
                            if prompt.isEmpty {
                                Text("Describe the image you want…")
                                    .foregroundStyle(.secondary)
                                    .padding(.top, 8)
                                    .padding(.leading, 5)
                                    .allowsHitTesting(false)
                            }
                        }
                } header: {
                    Text("Prompt")
                } footer: {
                    Text("Prompts you submit are sent to \(selectedService.isEmpty ? "the configured AI service" : selectedService)'s servers.")
                        .font(.caption)
                }

                Section {
                    Button {
                        runGenerate()
                    } label: {
                        HStack {
                            if status == .running {
                                ProgressView().controlSize(.small)
                                Text("Generating…")
                            } else {
                                Image(systemName: "wand.and.stars")
                                Text(status == .ready ? "Regenerate" : "Generate")
                            }
                            Spacer()
                        }
                    }
                    .disabled(prompt.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty
                              || session == nil
                              || status == .running)

                    if case .error(let msg) = status {
                        Label(msg, systemImage: "xmark.octagon.fill")
                            .foregroundStyle(.red)
                            .font(.footnote)
                    }
                }

                if let preview = preview {
                    Section("Preview") {
                        Image(uiImage: preview)
                            .resizable()
                            .scaledToFit()
                            .frame(maxHeight: 360)
                            .frame(maxWidth: .infinity)
                            .background(
                                RoundedRectangle(cornerRadius: 8)
                                    .fill(Color(uiColor: .secondarySystemBackground))
                            )
                            .overlay(
                                RoundedRectangle(cornerRadius: 8)
                                    .stroke(Color.secondary.opacity(0.3), lineWidth: 0.5)
                            )

                        Button {
                            commitImage()
                        } label: {
                            Label("Use This Image", systemImage: "checkmark.circle.fill")
                        }
                    }
                }
            }
            .navigationTitle("AI Image")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .topBarLeading) {
                    Button("Cancel") { dismiss() }
                }
            }
        }
        .onAppear { loadServices() }
    }

    // MARK: - Service discovery

    private func loadServices() {
        let all = XLAIServices.shared().allServices()
        availableServices = all
            .filter { $0.available && $0.capabilities.contains(XLAICapabilityImages) }
            .map { $0.name }

        if selectedService.isEmpty || !availableServices.contains(selectedService) {
            selectedService = availableServices.first ?? ""
        }
        if !selectedService.isEmpty && session == nil {
            session = XLAIImageSession.session(forService: selectedService)
        }
    }

    // MARK: - Generate

    private func runGenerate() {
        guard let session = session else { return }
        let p = prompt.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !p.isEmpty else { return }

        status = .running
        pngData = nil
        preview = nil
        session.generate(p) { png, err in
            // The bridge marshals this completion to main, but
            // Swift sees the ObjC block as non-isolated @Sendable.
            // Hop to MainActor so the @State mutations are safe.
            Task { @MainActor in
                if let err = err {
                    status = .error(err)
                    return
                }
                guard let png = png, let img = UIImage(data: png) else {
                    status = .error("Couldn't decode generated image")
                    return
                }
                pngData = png
                preview = img
                status = .ready
            }
        }
    }

    private func resetResult() {
        status = .idle
        pngData = nil
        preview = nil
    }

    // MARK: - Save + commit

    private func commitImage() {
        guard let png = pngData else { return }

        let doc = viewModel.document
        let showDir = doc.showFolderPath()
        guard !showDir.isEmpty else {
            status = .error("Open a show folder before saving generated images.")
            return
        }

        // Use a millisecond timestamp so re-rolls don't collide.
        let ts = Int(Date().timeIntervalSince1970 * 1000)
        let fileName = "ai_generated_\(ts).png"

        // Stage in a temp file so the existing moveFileToShowFolder
        // path can do its collision-detection / iCloud-friendly copy.
        let tmpDir = FileManager.default.temporaryDirectory
        let tmpURL = tmpDir.appendingPathComponent(fileName)
        do {
            try png.write(to: tmpURL, options: .atomic)
        } catch {
            status = .error("Couldn't stage image to disk: \(error.localizedDescription)")
            return
        }

        guard let copied = doc.moveFile(toShowFolder: tmpURL.path,
                                         subdirectory: "AIImages") else {
            status = .error("Couldn't copy image into the show folder.")
            try? FileManager.default.removeItem(at: tmpURL)
            return
        }
        try? FileManager.default.removeItem(at: tmpURL)

        let stored = doc.makeRelativePath(copied)
        onCommitPath(stored)
        dismiss()
    }
}
