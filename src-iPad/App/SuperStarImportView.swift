import SwiftUI

// Phase I-4 — SuperStar (`.sup`) import form.
//
// SuperStar's data model is structurally nothing like `.xsq` import:
// no source/dest tree, no per-row mapping. The user picks **one**
// target model and the parser fans every morph / image / flowy /
// scene / textAction / imageAction in the file out as effects on that
// model's layers. So this is a single-page form, not the
// `ImportEffectsView` channel-mapper.
//
// Defaults match `SuperStarImportDialog` on the desktop.

struct SuperStarImportView: View {
    let viewModel: SequencerViewModel
    // The URL (not just the path) — we re-acquire the security scope
    // around the actual file read in runImport(). Storing the path string
    // would lose the document picker's security-scoped bookmark and the
    // `std::ifstream` open in XLSuperStarImport would fail with errno 1
    // for any file outside the app sandbox.
    let sourceURL: URL
    let onCancel: () -> Void
    let onApplied: () -> Void

    @State private var availableModels: [String] = []
    @State private var selectedModel: String = ""
    @State private var xSize: Int = 12
    @State private var ySize: Int = 50
    @State private var xOffset: Int = 0
    @State private var yOffset: Int = 0
    @State private var imageResize: XLSuperStarImageResize = .none
    @State private var layerBlend: String = XLSuperStarLayerBlend2Reveals1
    @State private var timingOffsetMs: Int = 0
    @State private var imageGroupPrefix: String = ""
    @State private var importError: String?

    private static let blendOptions: [String] = [
        XLSuperStarLayerBlendAverage,
        XLSuperStarLayerBlendNormal,
        XLSuperStarLayerBlend2Reveals1,
    ]

    private var fileBasename: String {
        sourceURL.lastPathComponent
    }

    private var fileStem: String {
        sourceURL.deletingPathExtension().lastPathComponent
    }

    private var canImport: Bool {
        !selectedModel.isEmpty && xSize > 0 && ySize > 0
    }

    var body: some View {
        Form {
            Section {
                LabeledContent("File") {
                    Text(fileBasename)
                        .lineLimit(1)
                        .truncationMode(.middle)
                        .foregroundStyle(.secondary)
                }
            }

            Section("Target") {
                Picker("Target Model", selection: $selectedModel) {
                    if availableModels.isEmpty {
                        Text("(no models in sequence)")
                            .tag("")
                    } else {
                        if selectedModel.isEmpty {
                            Text("Select a model…").tag("")
                        }
                        ForEach(availableModels, id: \.self) { name in
                            Text(name).tag(name)
                        }
                    }
                }
            }

            Section {
                LabeledContent("X Size") {
                    EditableNumberField(storedInt: xSize, min: 1, max: 1000, divisor: 1) {
                        xSize = $0
                    }
                }
                LabeledContent("X Offset") {
                    EditableNumberField(storedInt: xOffset, min: -1000, max: 1000, divisor: 1) {
                        xOffset = $0
                    }
                }
                LabeledContent("Y Size") {
                    EditableNumberField(storedInt: ySize, min: 1, max: 1000, divisor: 1) {
                        ySize = $0
                    }
                }
                LabeledContent("Y Offset") {
                    EditableNumberField(storedInt: yOffset, min: -1000, max: 1000, divisor: 1) {
                        yOffset = $0
                    }
                }
            } header: {
                Text("Grid")
            } footer: {
                Text("X/Y Size + Offset only apply to SuperStar Visualization sequences. " +
                     "CCR ribbon-tree sequences ignore the size fields.")
                    .font(.footnote)
            }

            Section("Options") {
                Picker("Image Resize", selection: $imageResize) {
                    Text("None").tag(XLSuperStarImageResize.none)
                    Text("Exact Width").tag(XLSuperStarImageResize.exactWidth)
                    Text("Exact Height").tag(XLSuperStarImageResize.exactHeight)
                    Text("Exact Width or Height").tag(XLSuperStarImageResize.exactWidthOrHeight)
                    Text("All").tag(XLSuperStarImageResize.all)
                }
                Picker("Layer Blend", selection: $layerBlend) {
                    ForEach(Self.blendOptions, id: \.self) { name in
                        Text(name).tag(name)
                    }
                }
                LabeledContent("Timing Adjust (ms)") {
                    EditableNumberField(storedInt: timingOffsetMs, min: -60000, max: 60000, divisor: 1) {
                        timingOffsetMs = $0
                    }
                }
                HStack {
                    Text("Image Group")
                    Spacer()
                    TextField(fileStem, text: $imageGroupPrefix)
                        .multilineTextAlignment(.trailing)
                        .foregroundStyle(.secondary)
                        .frame(minWidth: 120)
                }
            }
        }
        .navigationTitle("Import SuperStar")
        .toolbar {
            ToolbarItem(placement: .cancellationAction) {
                Button("Cancel") { onCancel() }
            }
            ToolbarItem(placement: .confirmationAction) {
                Button("Import") { runImport() }
                    .disabled(!canImport)
            }
        }
        .alert("Import Error", isPresented: errorBinding, presenting: importError) { _ in
            Button("OK", role: .cancel) { importError = nil }
        } message: { msg in
            Text(msg)
        }
        .onAppear {
            availableModels = XLSuperStarImport.availableTargetModelNames(forDocument: viewModel.document)
            if selectedModel.isEmpty, let first = availableModels.first {
                selectedModel = first
            }
        }
    }

    private var errorBinding: Binding<Bool> {
        Binding(get: { importError != nil },
                set: { if !$0 { importError = nil } })
    }

    private func runImport() {
        let opts = XLSuperStarImportOptions()
        opts.xSize = Int32(xSize)
        opts.ySize = Int32(ySize)
        opts.xOffset = Int32(xOffset)
        opts.yOffset = Int32(yOffset)
        opts.imageResize = imageResize
        opts.layerBlend = layerBlend
        opts.timingOffsetMs = Int32(timingOffsetMs)
        opts.imageGroupPrefix = imageGroupPrefix

        // Document-picker URLs are security-scoped — the file at
        // `sourceURL` is unreadable from the bridge unless this scope is
        // held. Re-acquire it around the bridge call (the
        // ImportEffectsView's earlier acquisition was released when its
        // `loadSource` returned).
        let started = sourceURL.startAccessingSecurityScopedResource()
        defer { if started { sourceURL.stopAccessingSecurityScopedResource() } }

        do {
            try XLSuperStarImport.applyImport(fromPath: sourceURL.path,
                                              targetModelName: selectedModel,
                                              options: opts,
                                              document: viewModel.document)
            viewModel.reloadRows()
            onApplied()
        } catch {
            importError = error.localizedDescription
        }
    }
}
