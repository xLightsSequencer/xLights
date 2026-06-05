import SwiftUI

// Custom rows for the shared Blending panel (shared/Blending.json,
// `T_` prefix). These compound controls are hand-built on desktop in
// BlendingPanel.cpp and here on iPad — they don't map to any single
// schema control type. Each row writes its stored values directly to
// their `T_*` setting keys; `GetBlendingString` on desktop already
// emits T_-prefixed keys, so iPad-written values round-trip cleanly.
//
// Covered:
//   LayerMorphRow          — Morph checkbox + Effect Layer Mix slider
//   LayerMethodRow         — Layer Method choice (24 mix modes)
//   CanvasRow              — Canvas checkbox + Layers... button
//   In_Transition_Header   — In transition type + fade-in time
//   Out_Transition_Header  — Out transition type + fade-out time

// MARK: - Layer Method list

/// Desktop order from BlendingPanel.cpp (not alphabetical — preserved
/// because existing user muscle-memory and screenshots follow it).
/// Canvas is an additional option appended when CanvasRow's checkbox
/// enables it, but is ALSO selectable here for parity with desktop's
/// choice — the render pipeline accepts it either way.
let kLayerMethods: [String] = [
    "Normal", "Effect 1", "Effect 2",
    "1 is Mask", "2 is Mask",
    "1 is Unmask", "2 is Unmask",
    "1 is True Unmask", "2 is True Unmask",
    "1 reveals 2", "2 reveals 1",
    "Shadow 1 on 2", "Shadow 2 on 1",
    "Layered", "Average", "Bottom-Top", "Left-Right",
    "Highlight", "Highlight Vibrant",
    "Additive", "Subtractive", "Brightness", "Max", "Min",
]

// MARK: - Transition type lists

/// Verbatim from BlendingPanel.cpp:255. Alphabetical (wxCB_SORT), the
/// iPad mirrors that by sorting our own list — the array below is
/// already sorted so no runtime sort needed.
let kTransitionTypes: [String] = [
    "Blend", "Blinds", "Blobs", "Bow Tie", "Circle Explode", "Circles",
    "Circular Swirl", "Clock", "Dissolve", "Doorway", "Fade", "Fold",
    "From Middle", "Pinwheel", "Shatter", "Slide Bars", "Slide Checks",
    "Square Explode", "Star", "Swap", "Wipe", "Zoom",
]

/// Transitions that ignore the Adjustment slider. Desktop disables the
/// Adjust row when the current type is in this list; iPad currently
/// just renders the slider uneditable with reduced opacity to match.
let kTransitionsNoAdjust: Set<String> = [
    "Fade", "Square Explode", "Circle Explode", "Fold", "Dissolve",
    "Circular Swirl", "Zoom", "Doorway", "Swap", "Shatter",
]

/// Transitions that ignore the Reverse checkbox. Kept in sync with
/// desktop `BlendingPanel.cpp:76` (`TRANSITIONS_NO_REVERSE`) so the
/// Reverse row enables / disables for the same types on both
/// platforms.
let kTransitionsNoReverse: Set<String> = [
    "Fade", "Slide Bars", "Blend", "Dissolve", "Circular Swirl", "Zoom",
    "Doorway", "Blobs", "Pinwheel", "Swap", "Shatter", "Circles",
]

/// Common preset fade times (seconds) shown in the drop-down — matches
/// BlendingPanel.cpp:283.
let kFadePresets: [String] = ["0.00", "0.25", "0.50", "0.75", "1.00", "1.50", "2.00"]

// MARK: - LayerMorphRow

/// `LayerMorphRow` — Morph checkbox + Effect Layer Mix slider on a
/// single row. Morph enables a gradual cross-fade between Effect 1 and
/// Effect 2; the Mix slider (0..100) controls the steady-state blend.
/// Backing: `T_CHECKBOX_LayerMorph`, `T_SLIDER_EffectLayerMix`.
struct LayerMorphRowView: View {
    @Environment(SequencerViewModel.self) var viewModel

    private let morphKey = "T_CHECKBOX_LayerMorph"
    private let mixKey = "T_SLIDER_EffectLayerMix"

    private var morphOn: Bool {
        let v = viewModel.settingValue(forKey: morphKey, defaultValue: "0")
        return v == "1" || v.lowercased() == "true"
    }

    private var mix: Int {
        Int(viewModel.settingValue(forKey: mixKey, defaultValue: "0")) ?? 0
    }

    var body: some View {
        let morphBinding = Binding<Bool>(
            get: { morphOn },
            set: { viewModel.setSettingValue($0 ? "1" : "0",
                                              forKey: morphKey,
                                              suppressIfDefault: "0") }
        )
        let mixBinding = Binding<Double>(
            get: { Double(mix) },
            set: { viewModel.setSettingValue(String(Int($0)),
                                              forKey: mixKey,
                                              suppressIfDefault: "0") }
        )

        VStack(alignment: .leading, spacing: 2) {
            HStack {
                Toggle(isOn: morphBinding) {
                    Text("Morph").font(.caption)
                }
                .toggleStyle(.switch)
                .controlSize(.small)
                Spacer()
                Text("\(mix)")
                    .monospacedDigit()
                    .font(.caption2)
                    .foregroundStyle(.secondary)
            }
            Slider(value: mixBinding, in: 0...100, step: 1)
        }
        .padding(.vertical, 2)
    }
}

// MARK: - LayerMethodRow

/// `LayerMethodRow` — Layer Method choice picker. Controls how Effect 1
/// and Effect 2 combine in the pixel buffer. Backing:
/// `T_CHOICE_LayerMethod` (default "Normal"). The desktop "?" help
/// button surfaces a long tooltip — on iPad we expose the same text
/// via a `.help` modifier and an info chevron for tap-to-present.
struct LayerMethodRowView: View {
    @Environment(SequencerViewModel.self) var viewModel

    private let key = "T_CHOICE_LayerMethod"
    private let defaultValue = "Normal"

    @State private var showingHelp = false

    var body: some View {
        let binding = Binding<String>(
            get: {
                let v = viewModel.settingValue(forKey: key,
                                                defaultValue: defaultValue)
                return v.isEmpty ? defaultValue : v
            },
            set: { viewModel.setSettingValue($0,
                                              forKey: key,
                                              suppressIfDefault: defaultValue) }
        )

        // Stack label over picker — a `.menu`-style Picker with
        // `labelsHidden()` crammed next to a Text label in a narrow
        // inspector sidebar gets squashed to 2-3 visible characters
        // and wraps the current selection ("Nor-/mal"). Full-width
        // picker shows the whole selection.
        VStack(alignment: .leading, spacing: 2) {
            HStack {
                Text("Layer Method")
                    .font(.caption)
                Spacer()
                Button(action: { showingHelp = true }) {
                    Image(systemName: "questionmark.circle")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
                .buttonStyle(.plain)
            }
            Picker("Layer Method", selection: binding) {
                ForEach(kLayerMethods, id: \.self) { m in
                    Text(m).tag(m)
                }
            }
            .pickerStyle(.menu)
            .labelsHidden()
            .frame(maxWidth: .infinity, alignment: .leading)
        }
        .padding(.vertical, 2)
        .sheet(isPresented: $showingHelp) {
            LayerMethodHelpSheet()
        }
    }
}

private struct LayerMethodHelpSheet: View {
    @Environment(\.dismiss) private var dismiss

    var body: some View {
        NavigationStack {
            ScrollView {
                VStack(alignment: .leading, spacing: 8) {
                    Text(Self.helpText)
                        .font(.callout)
                        .textSelection(.enabled)
                }
                .padding()
            }
            .navigationTitle("Layer Methods")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") { dismiss() }
                }
            }
        }
    }

    // Lifted verbatim from BlendingPanel.cpp:LAYER_METHOD_TOOLTIP so the
    // iPad and desktop help text stay in sync.
    static let helpText: String = """
Layering defines how Effect 1 and Effect 2 will be mixed together.
Here are the Choices
• Normal: Same as 1 reveals 2.
• Effect 1: Shows only Effect 1. Slide right to blend in Effect 2.
• Effect 2: Shows only Effect 2. Slide right to blend in Effect 1.
• 1 is Mask: (Shadow) Effect 1 casts a shadow onto Effect 2 wherever Effect 1 has a non-black pixel.
• 2 is Mask: (Shadow) Effect 2 casts a shadow onto Effect 1 wherever Effect 2 has a non-black pixel.
• 1 is Unmask: Like mask but colours are revealed without fade. Black becomes white.
• 2 is Unmask: Mirror of the above.
• 1 is True Unmask: (Mask) Only allow Effect 2 to show where Effect 1 has a non-black pixel.
• 2 is True Unmask: Mirror of the above.
• Shadow 1 on 2: Take brightness and saturation from 1, hue from 2.
• Shadow 2 on 1: Take brightness and saturation from 2, hue from 1.
• 1 reveals 2: (Superimpose) Effect 1 reveals Effect 2.
• 2 reveals 1: (Superimpose) Effect 2 reveals Effect 1.
• Layered: Effect 1 only shows in black regions of Effect 2.
• Average: Add the two values and average the sum.
• Bottom-top: Effect 1 on the bottom, Effect 2 on the top (split screen).
• Left-Right: Effect 1 on the left, Effect 2 on the right (split screen).
• Highlight: Show Effect 1's colour where available, use Effect 2's colour where Effect 1 has none.
• Highlight Vibrant: Intensify Effect 2's colour where Effect 1 has content, without affecting black or dark areas.
• Additive: Take value of Effect 1 and add it to Effect 2.
• Subtractive: Subtract Effect 1's value from Effect 2.
• Brightness: Multiply each colour channel of both layers and divide by 255.
• Max: Take the maximum value for each channel from both effects.
• Min: Take the minimum value for each channel from both effects.
• Canvas: Blend the selected layers into this layer.
"""
}

// MARK: - CanvasRow

/// `CanvasRow` — Canvas checkbox + Layers... button. When enabled the
/// effect's output is composited over the selected source layers.
/// Backing: `T_CHECKBOX_Canvas` and `T_LayersSelected`.
///
/// `T_LayersSelected` format matches desktop `LayerSelectDialog`:
/// a `|`-separated list of layer offsets below the current effect's
/// layer — i.e. `"0"` is the layer immediately beneath the effect,
/// `"1"` the next one down, etc. `RenderEngine.cpp:683` adds
/// `layer + 1` to each stored index to recover the absolute layer.
struct CanvasRowView: View {
    @Environment(SequencerViewModel.self) var viewModel

    private let checkKey = "T_CHECKBOX_Canvas"
    private let layersKey = "T_LayersSelected"

    @State private var showingPicker = false

    private var enabled: Bool {
        let v = viewModel.settingValue(forKey: checkKey, defaultValue: "0")
        return v == "1" || v.lowercased() == "true"
    }

    private var storedLayers: String {
        viewModel.settingValue(forKey: layersKey, defaultValue: "")
    }

    /// Number of layers of the parent element that sit below the
    /// selected effect's own layer. 0 → nothing to composite.
    private var layersBelowCount: Int {
        guard let sel = viewModel.selectedEffect else { return 0 }
        let row = Int32(sel.rowIndex)
        let currentLayer = Int(viewModel.document.rowLayerIndex(at: row))
        let total = Int(viewModel.document.rowLayerCount(at: row))
        return max(0, total - currentLayer - 1)
    }

    var body: some View {
        let enabledBinding = Binding<Bool>(
            get: { enabled },
            set: { viewModel.setSettingValue($0 ? "1" : "0",
                                              forKey: checkKey,
                                              suppressIfDefault: "0") }
        )

        VStack(alignment: .leading, spacing: 4) {
            Toggle(isOn: enabledBinding) {
                Text("Canvas").font(.caption)
            }
            .toggleStyle(.switch)
            .controlSize(.small)

            if enabled {
                HStack {
                    Text(layerSummary())
                        .font(.caption2)
                        .foregroundStyle(.secondary)
                        .lineLimit(1)
                        .truncationMode(.tail)
                        .frame(maxWidth: .infinity, alignment: .leading)
                    Button("Layers…") { showingPicker = true }
                        .buttonStyle(.bordered)
                        .controlSize(.small)
                        .disabled(layersBelowCount == 0)
                }
            }
        }
        .padding(.vertical, 2)
        .sheet(isPresented: $showingPicker) {
            CanvasLayerPickerSheet(
                total: layersBelowCount,
                initialSelection: storedLayers,
                onCommit: { selected in
                    viewModel.setSettingValue(selected,
                                               forKey: layersKey,
                                               suppressIfDefault: "")
                })
        }
    }

    private func layerSummary() -> String {
        if layersBelowCount == 0 {
            return "No layers below"
        }
        if storedLayers.isEmpty {
            return "All \(layersBelowCount) layer\(layersBelowCount == 1 ? "" : "s")"
        }
        let count = storedLayers.split(separator: "|").count
        return "\(count) of \(layersBelowCount) layer\(layersBelowCount == 1 ? "" : "s")"
    }
}

/// Modal picker for the Canvas layer selection. Mirrors
/// `LayerSelectDialog` — a check-list over the layers below the
/// current effect. Selection is committed as a `|`-joined list of
/// offsets when the user taps Done.
private struct CanvasLayerPickerSheet: View {
    @Environment(\.dismiss) private var dismiss
    let total: Int
    let initialSelection: String
    let onCommit: (String) -> Void

    @State private var checked: Set<Int>

    init(total: Int,
         initialSelection: String,
         onCommit: @escaping (String) -> Void) {
        self.total = total
        self.initialSelection = initialSelection
        self.onCommit = onCommit
        // Seed @State from the stored string. Empty string =
        // "all layers" on desktop (LayerSelectDialog:76-77), so we
        // pre-check everything in that case.
        let initial: Set<Int>
        if initialSelection.isEmpty {
            initial = Set(0..<total)
        } else {
            initial = Set(initialSelection.split(separator: "|")
                            .compactMap { Int($0) }
                            .filter { $0 < total })
        }
        _checked = State(initialValue: initial)
    }

    var body: some View {
        let items = Array(0..<total)
        NavigationStack {
            List {
                Section {
                    Button("Select All") {
                        checked = Set(items)
                    }
                    Button("Select None") {
                        checked = []
                    }
                }
                Section("Layers below current effect") {
                    ForEach(items, id: \.self) { (idx: Int) in
                        Button(action: { toggle(idx) }) {
                            HStack {
                                Image(systemName: checked.contains(idx)
                                      ? "checkmark.square.fill"
                                      : "square")
                                    .foregroundStyle(checked.contains(idx)
                                                     ? Color.accentColor
                                                     : .secondary)
                                Text("Layer \(idx + 1)")
                                Spacer()
                            }
                        }
                        .buttonStyle(.plain)
                    }
                }
            }
            .navigationTitle("Canvas Layers")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") {
                        onCommit(encode())
                        dismiss()
                    }
                }
            }
        }
    }

    private func toggle(_ idx: Int) {
        if checked.contains(idx) { checked.remove(idx) }
        else { checked.insert(idx) }
    }

    /// Encode the checked set back into desktop's `|`-joined
    /// format. When everything is selected we emit the empty
    /// string, matching desktop's "no value = all layers" shortcut
    /// (see `LayerSelectDialog.cpp:76`).
    private func encode() -> String {
        let sorted = checked.sorted()
        if sorted.count == total { return "" }
        return sorted.map(String.init).joined(separator: "|")
    }
}

// MARK: - Transition headers

/// `In_Transition_Header` / `Out_Transition_Header` — transition type
/// choice + fade-time (seconds) entry. `isIn` selects the direction so
/// one view services both sides of the Blending tab. Backing:
/// `T_CHOICE_In_Transition_Type` / `T_TEXTCTRL_Fadein` (or `Out`).
struct TransitionHeaderRowView: View {
    @Environment(SequencerViewModel.self) var viewModel
    let isIn: Bool

    private var typeKey: String {
        isIn ? "T_CHOICE_In_Transition_Type" : "T_CHOICE_Out_Transition_Type"
    }
    private var fadeKey: String {
        isIn ? "T_TEXTCTRL_Fadein" : "T_TEXTCTRL_Fadeout"
    }
    private var defaultType: String { "Fade" }
    private var defaultFade: String { "0.00" }

    private var type: String {
        let v = viewModel.settingValue(forKey: typeKey, defaultValue: defaultType)
        return v.isEmpty ? defaultType : v
    }
    private var fadeSeconds: String {
        let v = viewModel.settingValue(forKey: fadeKey, defaultValue: defaultFade)
        return v.isEmpty ? defaultFade : v
    }

    var body: some View {
        let typeBinding = Binding<String>(
            get: { type },
            set: { viewModel.setSettingValue($0,
                                              forKey: typeKey,
                                              suppressIfDefault: defaultType) }
        )
        let fadeBinding = Binding<String>(
            get: { fadeSeconds },
            set: { viewModel.setSettingValue(normalizeFade($0),
                                              forKey: fadeKey,
                                              suppressIfDefault: defaultFade) }
        )

        VStack(alignment: .leading, spacing: 4) {
            // Type picker on its own row so the chosen transition name
            // doesn't get truncated in the narrow inspector sidebar.
            Text("Type")
                .font(.caption)
                .foregroundStyle(.secondary)
            Picker("Type", selection: typeBinding) {
                ForEach(kTransitionTypes, id: \.self) { t in
                    Text(t).tag(t)
                }
            }
            .pickerStyle(.menu)
            .labelsHidden()
            .frame(maxWidth: .infinity, alignment: .leading)

            HStack {
                Text("Time (s)")
                    .font(.caption2)
                    .foregroundStyle(.secondary)
                Spacer()
                TextField("0.00", text: fadeBinding)
                    .textFieldStyle(.roundedBorder)
                    .keyboardType(.decimalPad)
                    .font(.caption2)
                    .frame(maxWidth: 90)
                    .multilineTextAlignment(.trailing)
            }

            // Preset chips for the common fade durations. Desktop's
            // BulkEditComboBox dropdown shows the same values.
            HStack(spacing: 4) {
                ForEach(kFadePresets, id: \.self) { (v: String) in
                    Button(action: { fadeBinding.wrappedValue = v }) {
                        Text(v)
                            .font(.caption2)
                            .padding(.horizontal, 6)
                            .padding(.vertical, 2)
                            .background(
                                RoundedRectangle(cornerRadius: 4)
                                    .fill(Color.secondary.opacity(0.12))
                            )
                    }
                    .buttonStyle(.plain)
                }
                Spacer()
            }
        }
        .padding(.vertical, 2)
    }

    /// The stored fade time is a plain decimal string; trim whitespace
    /// and reject non-numeric or negative input so the settings map
    /// doesn't get garbage strings. Empty / "0" / negative variants
    /// normalise to "0.00" which matches desktop's BlendingPanel
    /// ValidateWindow clamp.
    private func normalizeFade(_ s: String) -> String {
        let trimmed = s.trimmingCharacters(in: .whitespaces)
        if trimmed.isEmpty { return defaultFade }
        guard let d = Double(trimmed) else { return defaultFade }
        if d < 0 { return defaultFade }
        return trimmed
    }
}
