import SwiftUI

/// Jukebox — desktop's `JukeboxPanel` as a sheet: 50 numbered buttons
/// that select and play a linked effect. Tap plays (or stops, when the
/// button is unassigned); touch-and-hold links or clears. Unassigned
/// buttons render red, as desktop paints them. Medium detent with
/// background interaction so the grid and preview stay live while
/// triggering buttons during playback.
struct JukeboxSheet: View {
    @Environment(SequencerViewModel.self) var viewModel
    @Environment(\.dismiss) var dismiss

    /// Desktop's JUKEBOXBUTTONS.
    static let buttonCount = 50

    private struct LinkTarget: Identifiable {
        let number: Int
        var id: Int { number }
    }

    @State private var buttons: [Int: SequencerViewModel.JukeboxButtonInfo] = [:]
    @State private var linkTarget: LinkTarget?

    /// Desktop's unassigned-button colour (255, 108, 108).
    private static let unassigned = Color(red: 1.0, green: 108.0 / 255.0, blue: 108.0 / 255.0)

    var body: some View {
        NavigationStack {
            ScrollView {
                LazyVGrid(columns: [GridItem(.adaptive(minimum: 64), spacing: 8)], spacing: 8) {
                    ForEach(1...Self.buttonCount, id: \.self) { number in
                        buttonCell(number)
                    }
                }
                .padding(.horizontal)
                .padding(.top, 8)
                Text("Tap a button to select and play its linked effect; an unassigned button stops playback. Touch and hold a button to link an effect.")
                    .font(.caption)
                    .foregroundStyle(.secondary)
                    .frame(maxWidth: .infinity, alignment: .leading)
                    .padding()
            }
            .navigationTitle("Jukebox")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") { dismiss() }
                }
            }
            .onAppear { reload() }
            // Local sheet state, not a view-model flag: iOS allows one
            // sheet per ancestor chain, so the link editor must present
            // from this sheet's own body (same pattern as
            // DisplayElementsSheet's nested sheets).
            .sheet(item: $linkTarget) { target in
                LinkJukeboxButtonSheet(buttonNumber: target.number,
                                       existing: buttons[target.number],
                                       onChanged: { reload() })
                    .environment(viewModel)
            }
        }
        .presentationDetents([.medium, .large])
        .presentationBackgroundInteraction(.enabled(upThrough: .medium))
    }

    @ViewBuilder
    private func buttonCell(_ number: Int) -> some View {
        let info = buttons[number]
        Button {
            viewModel.playJukeboxButton(number)
        } label: {
            VStack(spacing: 2) {
                Text("\(number)")
                    .font(.body.weight(.semibold).monospaced())
                if let tip = info?.tooltip, !tip.isEmpty {
                    Text(tip)
                        .font(.system(size: 9))
                        .lineLimit(1)
                        .truncationMode(.tail)
                }
            }
            .frame(maxWidth: .infinity)
            .padding(.vertical, 10)
            .background(info == nil ? Self.unassigned.opacity(0.45)
                                    : Color.accentColor.opacity(0.15),
                        in: RoundedRectangle(cornerRadius: 6))
            .foregroundStyle(info == nil ? Color.primary : Color.accentColor)
        }
        .buttonStyle(.plain)
        .contextMenu {
            Button {
                linkTarget = LinkTarget(number: number)
            } label: {
                Label(info == nil ? "Link Effect…" : "Edit Link…", systemImage: "link")
            }
            if info != nil {
                Button(role: .destructive) {
                    viewModel.clearJukeboxButton(number)
                    reload()
                } label: {
                    Label("Clear Button", systemImage: "trash")
                }
            }
        }
    }

    private func reload() {
        buttons = viewModel.jukeboxButtons()
    }
}

/// Desktop's `LinkJukeboxButtonDialog`: link a jukebox button to an
/// effect either by its free-text Description or by Model / Layer /
/// Start Time. The choice lists cascade exactly as desktop's do —
/// only elements with effects, only layers with effects (1-based),
/// only the actual effect start times on that layer.
struct LinkJukeboxButtonSheet: View {
    @Environment(SequencerViewModel.self) var viewModel
    @Environment(\.dismiss) var dismiss

    let buttonNumber: Int
    let existing: SequencerViewModel.JukeboxButtonInfo?
    let onChanged: () -> Void

    private enum Mode: String, CaseIterable, Identifiable {
        case mlt = "Model/Layer/Time"
        case description = "Description"
        var id: String { rawValue }
    }

    @State private var mode: Mode = .mlt
    @State private var tooltip = ""
    @State private var loop = true
    @State private var descriptions: [String] = []
    @State private var selectedDescription = ""
    @State private var elements: [String] = []
    @State private var selectedElement = ""
    @State private var layers: [Int] = []
    @State private var selectedLayer = 0        // 1-based; 0 = none
    @State private var times: [Int] = []
    @State private var selectedTime = -1        // ms; -1 = none
    @State private var loaded = false

    private var isValid: Bool {
        switch mode {
        case .description:
            return !selectedDescription.isEmpty
        case .mlt:
            return !selectedElement.isEmpty && selectedLayer >= 1 && selectedTime >= 0
        }
    }

    var body: some View {
        NavigationStack {
            Form {
                Section {
                    LabeledContent("Button", value: "\(buttonNumber)")
                    TextField("Tooltip", text: $tooltip)
                    Toggle("Loop effect playback", isOn: $loop)
                }
                Section {
                    Picker("Link by", selection: $mode) {
                        ForEach(Mode.allCases) { m in
                            Text(m.rawValue).tag(m)
                        }
                    }
                    .pickerStyle(.segmented)

                    switch mode {
                    case .description:
                        Picker("Description", selection: $selectedDescription) {
                            Text("Select…").tag("")
                            ForEach(descriptions, id: \.self) { d in
                                Text(d).tag(d)
                            }
                        }
                    case .mlt:
                        Picker("Model", selection: $selectedElement) {
                            Text("Select…").tag("")
                            ForEach(elements, id: \.self) { e in
                                Text(e).tag(e)
                            }
                        }
                        Picker("Layer", selection: $selectedLayer) {
                            Text("Select…").tag(0)
                            ForEach(layers, id: \.self) { l in
                                Text("\(l)").tag(l)
                            }
                        }
                        .disabled(selectedElement.isEmpty)
                        Picker("Start Time", selection: $selectedTime) {
                            Text("Select…").tag(-1)
                            ForEach(times, id: \.self) { t in
                                Text(Self.timeString(t)).tag(t)
                            }
                        }
                        .disabled(selectedLayer < 1)
                    }
                } footer: {
                    if mode == .description {
                        Text("Plays the first effect whose Description matches. Descriptions are set on an effect in the inspector.")
                    } else {
                        Text("Plays the effect at the chosen start time. Only models and layers that have effects are listed.")
                    }
                }
                if existing != nil {
                    Section {
                        Button("Remove Link", role: .destructive) {
                            viewModel.clearJukeboxButton(buttonNumber)
                            onChanged()
                            dismiss()
                        }
                    }
                }
            }
            .navigationTitle("Link Effect")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Save") { save() }
                        .disabled(!isValid)
                }
            }
            .onAppear { loadInitial() }
            .onChange(of: selectedElement) { _, newValue in
                layers = newValue.isEmpty ? [] : viewModel.document
                    .jukeboxLayersWithEffects(forElement: newValue).map(\.intValue)
                selectedLayer = layers.count == 1 ? layers[0] : 0
                reloadTimes()
            }
            .onChange(of: selectedLayer) { _, _ in
                reloadTimes()
            }
        }
    }

    private func loadInitial() {
        guard !loaded else { return }
        loaded = true
        descriptions = viewModel.document.jukeboxEffectDescriptions()
        elements = viewModel.document.jukeboxElementNamesWithEffects()
        guard let existing else { return }
        tooltip = existing.tooltip
        loop = existing.loop
        if existing.type == "DESCRIPTION" {
            mode = .description
            if descriptions.contains(existing.linkDescription) {
                selectedDescription = existing.linkDescription
            }
        } else {
            mode = .mlt
            if elements.contains(existing.element) {
                selectedElement = existing.element
                layers = viewModel.document
                    .jukeboxLayersWithEffects(forElement: existing.element).map(\.intValue)
                if layers.contains(existing.layer) {
                    selectedLayer = existing.layer
                    times = viewModel.document
                        .jukeboxEffectStartTimes(forElement: existing.element,
                                                 layer: Int32(existing.layer)).map(\.intValue)
                    if times.contains(existing.time) {
                        selectedTime = existing.time
                    }
                }
            }
        }
    }

    private func reloadTimes() {
        guard !selectedElement.isEmpty, selectedLayer >= 1 else {
            times = []
            selectedTime = -1
            return
        }
        times = viewModel.document
            .jukeboxEffectStartTimes(forElement: selectedElement,
                                     layer: Int32(selectedLayer)).map(\.intValue)
        selectedTime = times.count == 1 ? times[0] : -1
    }

    private func save() {
        let info = SequencerViewModel.JukeboxButtonInfo(
            number: buttonNumber,
            type: mode == .description ? "DESCRIPTION" : "MLT",
            linkDescription: mode == .description ? selectedDescription : "",
            element: mode == .mlt ? selectedElement : "",
            layer: mode == .mlt ? selectedLayer : -1,
            time: mode == .mlt ? selectedTime : -1,
            tooltip: tooltip,
            loop: loop)
        viewModel.setJukeboxButton(info)
        onChanged()
        dismiss()
    }

    private static func timeString(_ ms: Int) -> String {
        let totalSeconds = ms / 1000
        let minutes = totalSeconds / 60
        let seconds = totalSeconds % 60
        let millis = ms % 1000
        return String(format: "%d:%02d.%03d", minutes, seconds, millis)
    }
}
