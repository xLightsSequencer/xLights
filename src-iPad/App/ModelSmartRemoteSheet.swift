import SwiftUI

// J-32.4 — Per-model smart-remote assignment, opened from the
// Visualize context menu on a pixel-port model row.
//
// Smart remotes (SR) are pixel-port range extenders — each
// "letter" (A, B, C, …) is a downstream hub the controller talks
// to over the same data line. Models on the same physical port
// route to a specific SR letter. Caps decide:
//   - whether SR is supported at all (FPP+SR, F32-B, etc.)
//   - the maximum letter count (4 = A..D, 16 = A..P, etc.)
//   - the available SR types (FPP, F-Amp, F-Amp-V2, …)
//
// Existing `setLayoutModelProperty` `cc.*` keys handle each write:
//   USE_SMART_REMOTE flag → cc.useSmartRemote (Bool)
//   letter index 0-based  → cc.smartRemoteIndex
//   type index           → cc.smartRemoteTypeIndex
//   max cascade           → cc.srMaxCascade
//   cascade on port       → cc.srCascadeOnPort

struct ModelSmartRemoteSheet: View {
    let modelName: String
    let controllerName: String
    let viewModel: SequencerViewModel
    let onCommit: () -> Void
    let onDismiss: () -> Void

    @State private var loaded: Bool = false
    @State private var useSmartRemote: Bool = false
    @State private var smartRemote: Int = 0          // 0=none, 1..N = A..
    @State private var smartRemoteTypeIndex: Int = 0
    @State private var typeOptions: [String] = []
    @State private var srMaxCascade: Int = 1
    @State private var srCascadeOnPort: Bool = false

    @State private var maxRemotes: Int = 0
    @State private var supportsSmartRemotes: Bool = false

    var body: some View {
        NavigationStack {
            Form {
                if supportsSmartRemotes {
                    Section {
                        Toggle("Use Smart Remote", isOn: $useSmartRemote)
                            .onChange(of: useSmartRemote) { _, v in
                                commit("cc.useSmartRemote", value: v as NSNumber)
                                if !v { smartRemote = 0 }
                            }
                    } header: {
                        Text("Routing")
                    } footer: {
                        Text("When off, the model talks directly to the controller port. Turn on to route it through a smart-remote downstream hub.")
                            .font(.caption)
                    }
                    if useSmartRemote {
                        letterPickerSection
                        if !typeOptions.isEmpty {
                            typeSection
                        }
                        cascadeSection
                    }
                } else {
                    Section {
                        Label("This controller doesn't support smart remotes.",
                              systemImage: "info.circle")
                            .font(.callout)
                    }
                }
            }
            .navigationTitle("Smart Remote")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Done") { onDismiss() }
                }
                ToolbarItem(placement: .principal) {
                    Text(modelName).font(.headline).lineLimit(1)
                }
            }
        }
        .task {
            guard !loaded else { return }
            loadCapabilities()
            loadState()
            loaded = true
        }
    }

    // MARK: - Sections

    @ViewBuilder
    private var letterPickerSection: some View {
        Section {
            // Letter buttons, 8 per row. Lays out as a grid so the
            // user gets a clean visual map of available SRs.
            let columns = Array(repeating: GridItem(.flexible(), spacing: 8),
                                count: 8)
            LazyVGrid(columns: columns, spacing: 8) {
                ForEach(1...max(maxRemotes, 1), id: \.self) { idx in
                    Button {
                        smartRemote = idx
                        // Bridge key is 0-based picker index (it then
                        // adds 1 to translate to A=1, B=2, …).
                        commit("cc.smartRemoteIndex",
                               value: NSNumber(value: idx - 1))
                    } label: {
                        Text(letter(idx))
                            .font(.body.weight(.semibold).monospaced())
                            .frame(maxWidth: .infinity)
                            .padding(.vertical, 8)
                            .background(idx == smartRemote
                                        ? Color.accentColor
                                        : Color.accentColor.opacity(0.12),
                                        in: RoundedRectangle(cornerRadius: 6))
                            .foregroundStyle(idx == smartRemote ? Color.white
                                                                 : Color.accentColor)
                    }
                    .buttonStyle(.plain)
                }
            }
        } header: {
            Text("Remote ID")
        } footer: {
            Text("Tap a letter to route this model through that smart remote.")
                .font(.caption)
        }
    }

    @ViewBuilder
    private var typeSection: some View {
        Section("Smart Remote Type") {
            Picker("Type", selection: $smartRemoteTypeIndex) {
                ForEach(typeOptions.indices, id: \.self) { i in
                    Text(typeOptions[i]).tag(i)
                }
            }
            .onChange(of: smartRemoteTypeIndex) { _, idx in
                commit("cc.smartRemoteTypeIndex", value: idx as NSNumber)
            }
        }
    }

    @ViewBuilder
    private var cascadeSection: some View {
        Section {
            Stepper(value: $srMaxCascade, in: 1...8) {
                LabeledContent("Max Cascade") {
                    Text("\(srMaxCascade)").monospacedDigit()
                }
            }
            .onChange(of: srMaxCascade) { _, v in
                commit("cc.srMaxCascade", value: v as NSNumber)
            }
            Toggle("Cascade on Port", isOn: $srCascadeOnPort)
                .onChange(of: srCascadeOnPort) { _, v in
                    commit("cc.srCascadeOnPort", value: v as NSNumber)
                }
        } header: {
            Text("Cascade")
        } footer: {
            Text("Cascade lets one SR feed pixels into the next (A→B→C). Max Cascade is the chain length; turn on Cascade on Port to chain across the next physical port instead.")
                .font(.caption)
        }
    }

    // MARK: - Helpers

    private func letter(_ oneBased: Int) -> String {
        // A=1, B=2, …, Z=26, then AA, AB, … — desktop uses single
        // letters up to P (16) on the highest-end FPP boards.
        guard oneBased >= 1 else { return "—" }
        if oneBased <= 26 {
            return String(UnicodeScalar(64 + oneBased)!)
        }
        let first = (oneBased - 1) / 26
        let second = (oneBased - 1) % 26 + 1
        return String(UnicodeScalar(64 + first)!) + String(UnicodeScalar(64 + second)!)
    }

    private func loadCapabilities() {
        let caps = (viewModel.document.smartRemoteCapabilities(forController: controllerName)
                    as? [String: Any]) ?? [:]
        supportsSmartRemotes = caps["supportsSmartRemotes"] as? Bool ?? false
        maxRemotes           = caps["maxRemotes"] as? Int ?? 0
        // Type list comes from the model itself (the caps list and
        // the per-model list match when caps are present).
    }

    private func loadState() {
        guard let d = viewModel.document.controllerConnection(forModel: modelName) as? [String: Any] else { return }
        useSmartRemote       = d["useSmartRemote"] as? Bool ?? false
        smartRemote          = d["smartRemote"] as? Int ?? 0
        typeOptions          = d["smartRemoteTypeOptions"] as? [String] ?? []
        let curType          = d["smartRemoteType"] as? String ?? ""
        smartRemoteTypeIndex = typeOptions.firstIndex(of: curType) ?? 0
        srMaxCascade         = max(1, d["srMaxCascade"] as? Int ?? 1)
        srCascadeOnPort      = d["srCascadeOnPort"] as? Bool ?? false
    }

    private func commit(_ key: String, value: Any) {
        let ok = viewModel.document.setLayoutModelProperty(modelName,
                                                            key: key,
                                                            value: value)
        if ok { onCommit() }
    }
}
