import SwiftUI

// J-32.3 — Edit a single model's controller-connection
// properties (brightness / gamma / colour order / group count /
// start nulls / end nulls / DMX channel) from the Visualize
// sheet. Mirrors the per-model right-click menu on desktop's
// ControllerModelDialog (CONTROLLER_BRIGHTNESS etc).
//
// Each pixel-side property is gated by an "Override" toggle —
// when off, the corresponding CTRL_PROPS active flag is cleared
// and the controller falls back to its own defaults at upload
// time. DMX Channel has no override (it's only meaningful when
// set and only on serial ports).
//
// Reads current state from the bridge on appear; pushes changes
// back via `setLayoutModelProperty` so the existing `cc.*` keys
// handle the writes.

struct ModelControllerPropertiesSheet: View {
    let modelName: String
    let portKind: String          // "pixel" / "serial" / "pwm" / etc
    let viewModel: SequencerViewModel
    let onCommit: () -> Void
    let onDismiss: () -> Void

    @State private var loaded: Bool = false
    @State private var brightnessActive: Bool = false
    @State private var brightness: Int = 100
    @State private var gammaActive: Bool = false
    @State private var gammaX10: Int = 10        // stored as int * 10 for the field
    @State private var colorOrderActive: Bool = false
    @State private var colorOrderIndex: Int = 0
    @State private var colorOrderOptions: [String] = []
    @State private var groupCountActive: Bool = false
    @State private var groupCount: Int = 1
    @State private var startNullsActive: Bool = false
    @State private var startNulls: Int = 0
    @State private var endNullsActive: Bool = false
    @State private var endNulls: Int = 0
    @State private var dmxChannel: Int = 1

    private var showsPixelProps: Bool { portKind == "pixel" }
    private var showsDMXChannel: Bool { portKind == "serial" }

    var body: some View {
        NavigationStack {
            Form {
                if showsPixelProps {
                    pixelSection
                }
                if showsDMXChannel {
                    dmxSection
                }
                if !showsPixelProps && !showsDMXChannel {
                    Section {
                        Text("No editable controller properties for this port type.")
                            .font(.callout)
                            .foregroundStyle(.secondary)
                    }
                }
            }
            .navigationTitle("Controller Properties")
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
            load()
            loaded = true
        }
    }

    @ViewBuilder
    private var pixelSection: some View {
        Section("Brightness") {
            Toggle("Override controller default", isOn: $brightnessActive)
                .onChange(of: brightnessActive) { _, v in
                    commit("cc.brightnessActive", value: v as NSNumber)
                }
            HStack {
                Text("Brightness")
                Spacer()
                EditableNumberField(storedInt: brightness, min: 0, max: 100, divisor: 1) { v in
                    brightness = v
                    commit("cc.brightness", value: v as NSNumber)
                }
                Text("%").foregroundStyle(.secondary)
            }
            .disabled(!brightnessActive)
        }
        Section("Gamma") {
            Toggle("Override controller default", isOn: $gammaActive)
                .onChange(of: gammaActive) { _, v in
                    commit("cc.gammaActive", value: v as NSNumber)
                }
            HStack {
                Text("Gamma")
                Spacer()
                EditableNumberField(storedInt: gammaX10, min: 1, max: 100, divisor: 10) { v in
                    gammaX10 = v
                    commit("cc.gamma", value: NSNumber(value: Double(v) / 10.0))
                }
            }
            .disabled(!gammaActive)
        }
        Section("Color Order") {
            Toggle("Override controller default", isOn: $colorOrderActive)
                .onChange(of: colorOrderActive) { _, v in
                    commit("cc.colorOrderActive", value: v as NSNumber)
                }
            Picker("Color Order", selection: $colorOrderIndex) {
                ForEach(colorOrderOptions.indices, id: \.self) { i in
                    Text(colorOrderOptions[i]).tag(i)
                }
            }
            .onChange(of: colorOrderIndex) { _, idx in
                commit("cc.colorOrderIndex", value: idx as NSNumber)
            }
            .disabled(!colorOrderActive)
        }
        Section("Group Count") {
            Toggle("Override controller default", isOn: $groupCountActive)
                .onChange(of: groupCountActive) { _, v in
                    commit("cc.groupCountActive", value: v as NSNumber)
                }
            HStack {
                Text("Group Count")
                Spacer()
                EditableNumberField(storedInt: groupCount, min: 1, max: 500, divisor: 1) { v in
                    groupCount = v
                    commit("cc.groupCount", value: v as NSNumber)
                }
            }
            .disabled(!groupCountActive)
        }
        Section("Null Pixels") {
            Toggle("Override start nulls", isOn: $startNullsActive)
                .onChange(of: startNullsActive) { _, v in
                    commit("cc.startNullsActive", value: v as NSNumber)
                }
            HStack {
                Text("Start Nulls")
                Spacer()
                EditableNumberField(storedInt: startNulls, min: 0, max: 100, divisor: 1) { v in
                    startNulls = v
                    commit("cc.startNulls", value: v as NSNumber)
                }
            }
            .disabled(!startNullsActive)

            Toggle("Override end nulls", isOn: $endNullsActive)
                .onChange(of: endNullsActive) { _, v in
                    commit("cc.endNullsActive", value: v as NSNumber)
                }
            HStack {
                Text("End Nulls")
                Spacer()
                EditableNumberField(storedInt: endNulls, min: 0, max: 100, divisor: 1) { v in
                    endNulls = v
                    commit("cc.endNulls", value: v as NSNumber)
                }
            }
            .disabled(!endNullsActive)
        }
    }

    @ViewBuilder
    private var dmxSection: some View {
        Section {
            HStack {
                Text("Channel")
                Spacer()
                EditableNumberField(storedInt: dmxChannel, min: 1, max: 512, divisor: 1) { v in
                    dmxChannel = v
                    commit("cc.dmxChannel", value: v as NSNumber)
                }
            }
        } header: {
            Text("DMX Channel")
        } footer: {
            Text("DMX channel offset on this serial port. Models on the same port chain start channels in sequence.")
                .font(.caption)
        }
    }

    private func load() {
        guard let d = viewModel.document.controllerConnection(forModel: modelName) as? [String: Any] else {
            return
        }
        brightnessActive = d["brightnessActive"] as? Bool ?? false
        brightness       = d["brightness"]       as? Int  ?? 100
        gammaActive      = d["gammaActive"]      as? Bool ?? false
        let g = d["gamma"] as? Double ?? 1.0
        gammaX10 = Int((g * 10.0).rounded())
        colorOrderActive  = d["colorOrderActive"] as? Bool ?? false
        colorOrderOptions = d["colorOrderOptions"] as? [String] ?? []
        let idx = d["colorOrderIndex"] as? Int ?? -1
        colorOrderIndex   = (idx < 0 || idx >= colorOrderOptions.count) ? 0 : idx
        groupCountActive  = d["groupCountActive"] as? Bool ?? false
        groupCount        = d["groupCount"]       as? Int  ?? 1
        startNullsActive  = d["startNullsActive"] as? Bool ?? false
        startNulls        = d["startNulls"]       as? Int  ?? 0
        endNullsActive    = d["endNullsActive"]   as? Bool ?? false
        endNulls          = d["endNulls"]         as? Int  ?? 0
        dmxChannel        = d["dmxChannel"]       as? Int  ?? 1
    }

    private func commit(_ key: String, value: Any) {
        let ok = viewModel.document.setLayoutModelProperty(modelName,
                                                            key: key,
                                                            value: value)
        if ok { onCommit() }
    }
}
