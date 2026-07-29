import SwiftUI

// J-32.6 — companion sheets for the Visualize context menu:
// "Move to Port…" (port picker) and "Set Start Channel…"
// (raw start-channel string editor). Each is a small focused
// sheet so the calling Visualize view stays in budget for
// Swift's type-checker.

struct MoveToPortSheet: View {
    let modelName: String
    let controllerName: String
    let viewModel: SequencerViewModel
    let onCommit: () -> Void
    let onDismiss: () -> Void

    @State private var maxPixelPort: Int = 0
    @State private var maxSerialPort: Int = 0
    @State private var maxPanelPort: Int = 0
    @State private var errorMessage: String? = nil

    private static func portLabel(_ kind: String) -> String {
        switch kind {
        case "pixel":  return "Pixel Port"
        case "serial": return "Serial Port"
        default:       return "LED Panel"
        }
    }

    @ViewBuilder
    private func portRow(kind: String, port p: Int) -> some View {
        Button {
            assign(kind: kind, port: p)
        } label: {
            HStack {
                Image(systemName: "circle.fill")
                    .foregroundStyle(.tint)
                    .font(.system(size: 8))
                Text("\(Self.portLabel(kind)) \(p)")
                Spacer()
                Image(systemName: "chevron.right")
                    .foregroundStyle(.tertiary)
                    .font(.caption)
            }
        }
        .buttonStyle(.plain)
    }

    var body: some View {
        NavigationStack {
            Form {
                if maxPixelPort > 0 {
                    Section("Pixel Ports") {
                        ForEach(1...maxPixelPort, id: \.self) { p in
                            portRow(kind: "pixel", port: p)
                        }
                    }
                }
                if maxSerialPort > 0 {
                    Section("Serial Ports") {
                        ForEach(1...maxSerialPort, id: \.self) { p in
                            portRow(kind: "serial", port: p)
                        }
                    }
                }
                if maxPanelPort > 0 {
                    Section {
                        ForEach(1...maxPanelPort, id: \.self) { p in
                            portRow(kind: "ledPanelMatrix", port: p)
                        }
                    } header: {
                        Text("LED Panel Matrices")
                    } footer: {
                        Text("The number is the panel matrix the controller's own LED Panels page shows, so it must already be configured there.")
                    }
                }
                if maxPixelPort == 0 && maxSerialPort == 0 && maxPanelPort == 0 {
                    Section {
                        Text("Controller capabilities aren't available — choose a vendor + model on the controller to enable port assignment.")
                            .font(.callout)
                            .foregroundStyle(.secondary)
                    }
                }
            }
            .navigationTitle("Move to Port")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { onDismiss() }
                }
                ToolbarItem(placement: .principal) {
                    Text(modelName).font(.headline).lineLimit(1)
                }
            }
        }
        .task {
            let counts = (viewModel.document.portCounts(forController: controllerName)
                          as? [String: Any]) ?? [:]
            maxPixelPort  = counts["maxPixelPort"]  as? Int ?? 0
            maxSerialPort = counts["maxSerialPort"] as? Int ?? 0
            maxPanelPort  = counts["maxLEDPanelMatrixPort"] as? Int ?? 0
        }
        .alert("Move failed",
               isPresented: Binding(get: { errorMessage != nil },
                                    set: { if !$0 { errorMessage = nil } })) {
            Button("OK", role: .cancel) { }
        } message: { Text(errorMessage ?? "") }
    }

    private func assign(kind: String, port: Int) {
        let ok = viewModel.document.assignModel(
            modelName,
            toController: controllerName,
            kind: kind,
            port: Int32(port),
            afterModel: nil,
            smartRemote: -1)
        if ok {
            onCommit()
            onDismiss()
        } else {
            errorMessage = "Couldn't assign \(modelName) to \(Self.portLabel(kind)) \(port)."
        }
    }
}

struct SetStartChannelSheet: View {
    let modelName: String
    let viewModel: SequencerViewModel
    let onCommit: () -> Void
    let onDismiss: () -> Void

    @State private var draft: String = ""
    @State private var loaded: Bool = false
    @State private var errorMessage: String? = nil

    var body: some View {
        NavigationStack {
            Form {
                Section {
                    TextField("e.g. 1 / @PrevModel:1 / >Model:5 / 1:1",
                              text: $draft)
                        .textInputAutocapitalization(.never)
                        .autocorrectionDisabled()
                        .font(.body.monospaced())
                } header: {
                    Text("Start Channel")
                } footer: {
                    Text("Accepted forms: absolute (1), universe (1:1), chained off a model (@Tree:1), universe chain (>Tree:1), no controller (!ControllerName:1).")
                        .font(.caption)
                }
            }
            .navigationTitle("Set Start Channel")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { onDismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Set") { commit() }
                        .disabled(draft.trimmingCharacters(in: .whitespaces).isEmpty)
                }
                ToolbarItem(placement: .principal) {
                    Text(modelName).font(.headline).lineLimit(1)
                }
            }
        }
        .task {
            guard !loaded else { return }
            // Pre-fill with whatever start channel the model
            // currently has — looking it up via the existing
            // bridge requires a separate read; for simplicity,
            // leave the field empty so the user types fresh.
            // (The footer documents the accepted forms.)
            loaded = true
        }
        .alert("Couldn't set start channel",
               isPresented: Binding(get: { errorMessage != nil },
                                    set: { if !$0 { errorMessage = nil } })) {
            Button("OK", role: .cancel) { }
        } message: { Text(errorMessage ?? "") }
    }

    private func commit() {
        let value = draft.trimmingCharacters(in: .whitespaces)
        let ok = viewModel.document.setLayoutModelProperty(
            modelName,
            key: "modelStartChannel",
            value: value as NSString)
        if ok {
            onCommit()
            onDismiss()
        } else {
            errorMessage = "'\(value)' isn't a valid start-channel expression."
        }
    }
}
