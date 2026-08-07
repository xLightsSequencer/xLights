import SwiftUI

/// Tools → Test. Drives the show's controllers with the shared core test
/// engine (`src-core/outputs/TestPatternEngine`) — the same pattern maths
/// the desktop `PixelTestDialog` runs.
///
/// The field case this exists for: you are standing in the display with the
/// iPad, and need to know which string is on which port, whether a prop is
/// wired in the order the model says, and where a dead pixel starts.
///
/// Output goes out over sACN / ArtNet / DDP. Serial / USB controllers have
/// no iPadOS transport and are listed as untestable rather than silently
/// doing nothing.
struct LightTestSheet: View {
    @Environment(SequencerViewModel.self) var viewModel
    @Environment(\.dismiss) private var dismiss

    private enum Tab: String, CaseIterable {
        case models = "Models"
        case groups = "Groups"
        case outputs = "Outputs"
        case controllers = "Controllers"
    }

    /// The pattern families the Models tab can run. Controller tests live on
    /// the other tab because they walk ports rather than the channel list.
    private enum Family: String, CaseIterable {
        case standard = "Standard"
        case rgb = "RGB"
        case rgbCycle = "RGB Cycle"

        var mode: XLLightTestMode {
            switch self {
            case .standard: return .standard
            case .rgb: return .RGB
            case .rgbCycle: return .rgbCycle
            }
        }
    }

    /// One selectable pattern. `grouping` feeds chase spacing; `ratio` feeds
    /// twinkle density. Mirrors the desktop radio-button sets one-for-one.
    private struct Pattern: Identifiable, Hashable {
        let id: String
        let title: String
        let function: XLLightTestFunction
        var grouping: Int = 1
        var whole: Bool = false
        var ratio: Int = 10
    }

    private static let chasePatterns: [Pattern] = [
        Pattern(id: "off", title: "Off", function: .off),
        Pattern(id: "chase", title: "Chase", function: .chase, whole: true),
        Pattern(id: "chase13", title: "Chase 1/3", function: .chase, grouping: 3),
        Pattern(id: "chase14", title: "Chase 1/4", function: .chase, grouping: 4),
        Pattern(id: "chase15", title: "Chase 1/5", function: .chase, grouping: 5),
        Pattern(id: "alternate", title: "Alternate", function: .chase, grouping: 2),
        Pattern(id: "tw5", title: "Twinkle 5%", function: .twinkle, ratio: 20),
        Pattern(id: "tw10", title: "Twinkle 10%", function: .twinkle, ratio: 10),
        Pattern(id: "tw25", title: "Twinkle 25%", function: .twinkle, ratio: 4),
        Pattern(id: "tw50", title: "Twinkle 50%", function: .twinkle, ratio: 2),
        Pattern(id: "shimmer", title: "Shimmer", function: .shimmer),
        Pattern(id: "background", title: "Background", function: .dim),
    ]

    private static let cyclePatterns: [Pattern] = [
        Pattern(id: "off", title: "Off", function: .off),
        Pattern(id: "abc", title: "R-G-B", function: .chase, grouping: 3),
        Pattern(id: "abcall", title: "R-G-B-All", function: .chase, grouping: 4),
        Pattern(id: "abcallnone", title: "R-G-B-All-None", function: .chase, grouping: 5),
        Pattern(id: "mixed", title: "Mixed Colors", function: .dim),
        Pattern(id: "rgbw", title: "R-G-B-W", function: .RGBW),
    ]

    private static let controllerPatterns: [Pattern] = [
        Pattern(id: "off", title: "Off", function: .off),
        Pattern(id: "portcycle", title: "Cycle Ports", function: .portCycle),
        Pattern(id: "pixelcount", title: "Pixel Count Blocks", function: .colorBlocks),
    ]

    @State private var tab: Tab = .models
    @State private var family: Family = .standard
    @State private var patternID: String = "off"

    @State private var speed: Double = 50
    @State private var backgroundIntensity: Double = 0
    @State private var highlightIntensity: Double = 255
    @State private var backgroundColor: Color = .black
    @State private var highlightColor: Color = .white
    @State private var tag50th = false
    @State private var suppressUnused = false

    @State private var running = false
    @State private var status = ""
    @State private var selectedCount = 0
    @State private var errorMessage: String?

    @State private var models: [[String: Any]] = []
    @State private var groups: [[String: Any]] = []
    @State private var outputs: [[String: Any]] = []
    @State private var controllers: [[String: Any]] = []
    /// Desktop puts a filter box above every target tree
    /// (PixelTestDialog.cpp:1403-1406). One field here, applied to
    /// whichever tab is showing — a name match keeps the row, and a
    /// parent stays when any child matches so the path to a hit is
    /// still reachable.
    @State private var filterText: String = ""
    @State private var expandedModels: Set<String> = []
    @State private var nodeCache: [String: [[String: Any]]] = [:]

    @State private var presetNames: [String] = []
    @State private var showingSavePreset = false
    @State private var newPresetName = ""

    private let ticker = Timer.publish(every: 0.05, on: .main, in: .common).autoconnect()

    private var test: LightTest? { viewModel.document.lightTest }

    private var patterns: [Pattern] {
        switch tab {
        case .controllers: return Self.controllerPatterns
        // Models, Groups and Outputs all select a channel range, so
        // they share the chase / cycle patterns; only the Controllers
        // tab drives the port-oriented tests.
        case .models, .groups, .outputs:
            return family == .rgbCycle ? Self.cyclePatterns : Self.chasePatterns
        }
    }

    private var pattern: Pattern {
        patterns.first { $0.id == patternID } ?? patterns[0]
    }

    var body: some View {
        NavigationStack {
            content
                .navigationTitle("Test Lights")
                .navigationBarTitleDisplayMode(.inline)
                .toolbar {
                    ToolbarItem(placement: .topBarLeading) { presetMenu }
                    ToolbarItem(placement: .topBarTrailing) {
                        Button("Done") {
                            stop()
                            dismiss()
                        }
                    }
                }
        }
        .onAppear(perform: reload)
        .onDisappear(perform: stop)
        .onReceive(ticker) { _ in
            guard running, let test else { return }
            test.tick()
            status = test.status
        }
        .alert("Test", isPresented: Binding(
            get: { errorMessage != nil },
            set: { if !$0 { errorMessage = nil } }
        )) {
            Button("OK", role: .cancel) { errorMessage = nil }
        } message: {
            Text(errorMessage ?? "")
        }
    }

    // MARK: - Layout

    private var content: some View {
        VStack(spacing: 0) {
            Picker("", selection: $tab) {
                ForEach(Tab.allCases, id: \.self) { Text($0.rawValue).tag($0) }
            }
            .pickerStyle(.segmented)
            .padding(.horizontal)
            .padding(.top, 8)
            .onChange(of: tab) { _, _ in
                patternID = "off"
                syncParameters()
            }

            selectionList

            Divider()
            controlPanel
        }
    }

    @ViewBuilder
    private var selectionList: some View {
        List {
            Section {
                HStack {
                    Image(systemName: "magnifyingglass")
                        .foregroundStyle(.secondary)
                    TextField("Filter", text: $filterText)
                        .textInputAutocapitalization(.never)
                        .autocorrectionDisabled()
                    if !filterText.isEmpty {
                        Button {
                            filterText = ""
                        } label: {
                            Image(systemName: "xmark.circle.fill")
                                .foregroundStyle(.secondary)
                        }
                        .buttonStyle(.plain)
                    }
                }
            }
            Section {
                switch tab {
                case .models: modelRows
                case .groups: groupRows
                case .outputs: outputRows
                case .controllers: controllerRows
                }
            } header: {
                HStack {
                    Text(sectionTitle)
                    Spacer()
                    Text("\(selectedCount) channels selected")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
            }
        }
        .listStyle(.insetGrouped)
    }

    @ViewBuilder
    private var modelRows: some View {
        let visibleModels = models.filter { matches($0["name"] as? String ?? "") }
        ForEach(visibleModels.indices, id: \.self) { i in
            let m = visibleModels[i]
            let name = m["name"] as? String ?? ""
            let start = m["startChannel"] as? UInt32 ?? 0
            let end = m["endChannel"] as? UInt32 ?? 0
            let testable = m["testable"] as? Bool ?? false

            selectableRow(
                title: name,
                subtitle: "\(m["displayAs"] as? String ?? "") · \(m["nodeCount"] as? Int ?? 0) nodes · ch \(start)-\(end)",
                start: start,
                end: end,
                testable: testable,
                reason: m["untestableReason"] as? String ?? "",
                indent: 0
            )

            if testable {
                // Submodels first — they are the usual field target ("just
                // the left arch"), then individual nodes for pinning a fault.
                ForEach(submodels(of: m).indices, id: \.self) { s in
                    let sm = submodels(of: m)[s]
                    selectableRow(
                        title: sm["name"] as? String ?? "",
                        subtitle: "submodel · \(sm["nodeCount"] as? Int ?? 0) nodes",
                        start: sm["startChannel"] as? UInt32 ?? 0,
                        end: sm["endChannel"] as? UInt32 ?? 0,
                        testable: true,
                        reason: "",
                        indent: 1
                    )
                }

                Button {
                    toggleNodes(for: name)
                } label: {
                    Label(expandedModels.contains(name) ? "Hide nodes" : "Show \(m["nodeCount"] as? Int ?? 0) nodes",
                          systemImage: expandedModels.contains(name) ? "chevron.down" : "chevron.right")
                        .font(.caption)
                }
                .padding(.leading, 20)

                if expandedModels.contains(name), let nodes = nodeCache[name] {
                    ForEach(nodes.indices, id: \.self) { n in
                        let node = nodes[n]
                        let ns = node["startChannel"] as? UInt32 ?? 0
                        let nc = node["channels"] as? Int ?? 3
                        selectableRow(
                            title: "Node \(node["node"] as? Int ?? 0)",
                            subtitle: "ch \(ns)-\(ns + UInt32(max(nc, 1)) - 1) · \(node["colours"] as? String ?? "")",
                            start: ns,
                            end: ns + UInt32(max(nc, 1)) - 1,
                            testable: true,
                            reason: "",
                            indent: 2
                        )
                    }
                }
            }
        }
    }

    @ViewBuilder
    private var controllerRows: some View {
        let visible = controllers.filter { c in
            matches(c["name"] as? String ?? "")
                || ports(of: c).contains { matches($0["name"] as? String ?? "") }
        }
        ForEach(visible.indices, id: \.self) { i in
            let c = visible[i]
            let testable = c["testable"] as? Bool ?? false

            VStack(alignment: .leading, spacing: 2) {
                Text(c["name"] as? String ?? "").font(.headline)
                Text("\(c["vendor"] as? String ?? "") \(c["model"] as? String ?? "") · \(c["ip"] as? String ?? "")")
                    .font(.caption)
                    .foregroundStyle(.secondary)
                if !testable {
                    Text(c["untestableReason"] as? String ?? "")
                        .font(.caption)
                        .foregroundStyle(.orange)
                }
            }

            if testable {
                ForEach(ports(of: c).indices, id: \.self) { p in
                    let port = ports(of: c)[p]
                    let models = (port["models"] as? [String] ?? []).joined(separator: ", ")
                    selectableRow(
                        title: port["name"] as? String ?? "",
                        subtitle: "\(port["pixels"] as? Int ?? 0) px · \(models)",
                        start: port["startChannel"] as? UInt32 ?? 0,
                        end: port["endChannel"] as? UInt32 ?? 0,
                        testable: true,
                        reason: "",
                        indent: 1
                    )
                }
            }
        }
    }

    private var sectionTitle: String {
        switch tab {
        case .models: return "Models"
        case .groups: return "Model Groups"
        case .outputs: return "Outputs"
        case .controllers: return "Controller Ports"
        }
    }

    /// Case-insensitive contains, with an empty filter matching
    /// everything.
    private func matches(_ s: String) -> Bool {
        let q = filterText.trimmingCharacters(in: .whitespaces)
        if q.isEmpty { return true }
        return s.range(of: q, options: .caseInsensitive) != nil
    }

    @ViewBuilder
    private var groupRows: some View {
        let visible = groups.filter { g in
            matches(g["name"] as? String ?? "")
                || groupModels(of: g).contains { matches($0["name"] as? String ?? "") }
        }
        ForEach(visible.indices, id: \.self) { i in
            let g = visible[i]
            Text(g["name"] as? String ?? "").font(.headline)
            // Members are listed individually because a group's channels
            // need not be contiguous — there is no single range to select.
            ForEach(groupModels(of: g).indices, id: \.self) { j in
                let m = groupModels(of: g)[j]
                selectableRow(
                    title: m["name"] as? String ?? "",
                    subtitle: "\(m["nodeCount"] as? Int ?? 0) nodes",
                    start: m["startChannel"] as? UInt32 ?? 0,
                    end: m["endChannel"] as? UInt32 ?? 0,
                    testable: m["testable"] as? Bool ?? false,
                    reason: m["untestableReason"] as? String ?? "",
                    indent: 1
                )
            }
        }
    }

    @ViewBuilder
    private var outputRows: some View {
        let visible = outputs.filter { c in
            matches(c["name"] as? String ?? "")
                || controllerOutputs(of: c).contains { matches($0["description"] as? String ?? "") }
        }
        ForEach(visible.indices, id: \.self) { i in
            let c = visible[i]
            let testable = c["testable"] as? Bool ?? false
            VStack(alignment: .leading, spacing: 2) {
                Text(c["name"] as? String ?? "").font(.headline)
                if !testable {
                    Text(c["untestableReason"] as? String ?? "")
                        .font(.caption)
                        .foregroundStyle(.orange)
                }
            }
            ForEach(controllerOutputs(of: c).indices, id: \.self) { j in
                let o = controllerOutputs(of: c)[j]
                selectableRow(
                    title: o["description"] as? String ?? "",
                    subtitle: "Universe \(o["universe"] as? Int ?? 0) · \(o["channels"] as? Int ?? 0) ch",
                    start: o["startChannel"] as? UInt32 ?? 0,
                    end: o["endChannel"] as? UInt32 ?? 0,
                    testable: testable,
                    reason: c["untestableReason"] as? String ?? "",
                    indent: 1
                )
            }
        }
    }

    private func groupModels(of g: [String: Any]) -> [[String: Any]] {
        g["models"] as? [[String: Any]] ?? []
    }

    private func controllerOutputs(of c: [String: Any]) -> [[String: Any]] {
        c["outputs"] as? [[String: Any]] ?? []
    }

    private func selectableRow(title: String, subtitle: String, start: UInt32, end: UInt32,
                               testable: Bool, reason: String, indent: Int) -> some View {
        let state = test?.selectionState(from: start, to: end) ?? 0

        return Button {
            guard testable, let test else { return }
            if state == 1 {
                test.deselect(from: start, to: end)
            } else {
                test.select(from: start, to: end)
            }
            selectedCount = test.selectedChannelCount
        } label: {
            HStack {
                Image(systemName: state == 1 ? "checkmark.square.fill"
                                             : (state < 0 ? "minus.square.fill" : "square"))
                    .foregroundStyle(testable ? Color.accentColor : Color.secondary)
                VStack(alignment: .leading, spacing: 1) {
                    Text(title).font(indent == 0 ? .body : .callout)
                    Text(testable ? subtitle : reason)
                        .font(.caption2)
                        .foregroundStyle(testable ? Color.secondary : Color.orange)
                }
                Spacer()
            }
            .padding(.leading, CGFloat(indent) * 20)
        }
        .buttonStyle(.plain)
        .disabled(!testable)
    }

    // MARK: - Controls

    @ViewBuilder
    private var controlPanel: some View {
        VStack(alignment: .leading, spacing: 10) {
            if tab == .models {
                Picker("", selection: $family) {
                    ForEach(Family.allCases, id: \.self) { Text($0.rawValue).tag($0) }
                }
                .pickerStyle(.segmented)
                .onChange(of: family) { _, _ in
                    patternID = "off"
                    syncParameters()
                }
            }

            ScrollView(.horizontal, showsIndicators: false) {
                HStack(spacing: 8) {
                    ForEach(patterns) { p in
                        Button(p.title) {
                            patternID = p.id
                            syncParameters()
                        }
                        .buttonStyle(.bordered)
                        .tint(patternID == p.id ? .accentColor : .secondary)
                    }
                }
            }

            HStack {
                Text("Speed").font(.caption).frame(width: 60, alignment: .leading)
                Slider(value: $speed, in: 0...100, step: 1)
                    .onChange(of: speed) { _, _ in syncParameters() }
                Text("\(Int(speed))").font(.caption.monospacedDigit()).frame(width: 32)
            }

            if tab == .models && family == .standard {
                intensityRow("Background", value: $backgroundIntensity)
                intensityRow("Highlight", value: $highlightIntensity)
            } else if tab == .models && family == .rgb {
                HStack(spacing: 16) {
                    ColorPicker("Background", selection: $backgroundColor, supportsOpacity: false)
                        .onChange(of: backgroundColor) { _, _ in syncParameters() }
                    ColorPicker("Highlight", selection: $highlightColor, supportsOpacity: false)
                        .onChange(of: highlightColor) { _, _ in syncParameters() }
                }
                .font(.caption)
            } else if tab == .models && family == .rgbCycle {
                Toggle("Mark every 50th pixel", isOn: $tag50th)
                    .font(.caption)
                    .onChange(of: tag50th) { _, _ in syncParameters() }
            }

            Toggle("Suppress unused outputs", isOn: $suppressUnused)
                .font(.caption)
                .onChange(of: suppressUnused) { _, _ in syncParameters() }

            HStack {
                Button(running ? "Stop Output" : "Start Output") {
                    running ? stop() : start()
                }
                .buttonStyle(.borderedProminent)
                .tint(running ? .red : .accentColor)

                Spacer()

                Text(status.isEmpty ? (running ? "Running" : "Idle") : status)
                    .font(.caption)
                    .foregroundStyle(.secondary)
                    .lineLimit(1)
            }
        }
        .padding()
        .background(.bar)
    }

    private func intensityRow(_ label: String, value: Binding<Double>) -> some View {
        HStack {
            Text(label).font(.caption).frame(width: 60, alignment: .leading)
            Slider(value: value, in: 0...255, step: 1)
                .onChange(of: value.wrappedValue) { _, _ in syncParameters() }
            Text("\(Int(value.wrappedValue))").font(.caption.monospacedDigit()).frame(width: 32)
        }
    }

    // MARK: - Presets

    private var presetMenu: some View {
        Menu {
            if presetNames.isEmpty {
                Text("No saved tests")
            } else {
                ForEach(presetNames, id: \.self) { name in
                    Button(name) {
                        _ = test?.loadPreset(name)
                        selectedCount = test?.selectedChannelCount ?? 0
                    }
                }
            }
            Divider()
            Button("Save Selection…") {
                newPresetName = ""
                showingSavePreset = true
            }
            Button("Clear Selection", role: .destructive) {
                test?.clearSelection()
                selectedCount = 0
            }
        } label: {
            Label("Presets", systemImage: "square.and.arrow.down")
        }
        .alert("Save Test Selection", isPresented: $showingSavePreset) {
            TextField("Name", text: $newPresetName)
            Button("Cancel", role: .cancel) {}
            Button("Save") {
                let trimmed = newPresetName.trimmingCharacters(in: .whitespacesAndNewlines)
                guard !trimmed.isEmpty, let test else { return }
                if test.savePreset(trimmed) {
                    presetNames = test.presetNames()
                } else {
                    errorMessage = "Could not save that test selection."
                }
            }
        } message: {
            Text("Saved tests are stored with the show and are shared with desktop xLights.")
        }
    }

    // MARK: - Wiring

    private func submodels(of model: [String: Any]) -> [[String: Any]] {
        model["submodels"] as? [[String: Any]] ?? []
    }

    private func ports(of controller: [String: Any]) -> [[String: Any]] {
        controller["ports"] as? [[String: Any]] ?? []
    }

    private func toggleNodes(for model: String) {
        if expandedModels.contains(model) {
            expandedModels.remove(model)
        } else {
            expandedModels.insert(model)
            if nodeCache[model] == nil {
                nodeCache[model] = test?.nodeItems(forModel: model) ?? []
            }
        }
    }

    private func reload() {
        guard let test else { return }
        models = test.modelItems()
        groups = test.groupItems()
        outputs = test.outputItems()
        controllers = test.controllerItems()
        presetNames = test.presetNames()
        selectedCount = test.selectedChannelCount
        running = test.running
        syncParameters()
    }

    /// Push the UI state onto the bridge. Cheap enough to call on every
    /// control change; the engine only reacts when something actually moves.
    private func syncParameters() {
        guard let test else { return }
        let p = pattern

        test.mode = (tab == .controllers) ? .controller : family.mode
        test.function = p.function
        test.chaseGrouping = p.grouping
        test.chaseWholeSelection = p.whole
        test.twinkleRatio = p.ratio
        test.speed = Int(speed)
        test.backgroundIntensity = Int(backgroundIntensity)
        test.highlightIntensity = Int(highlightIntensity)
        test.tag50th = tag50th
        test.suppressUnusedOutputs = suppressUnused

        let bg = rgbComponents(backgroundColor)
        let hi = rgbComponents(highlightColor)
        test.backgroundRed = bg.0
        test.backgroundGreen = bg.1
        test.backgroundBlue = bg.2
        test.highlightRed = hi.0
        test.highlightGreen = hi.1
        test.highlightBlue = hi.2
    }

    private func rgbComponents(_ color: Color) -> (Int, Int, Int) {
        let resolved = UIColor(color)
        var r: CGFloat = 0, g: CGFloat = 0, b: CGFloat = 0, a: CGFloat = 0
        resolved.getRed(&r, green: &g, blue: &b, alpha: &a)
        return (Int(r * 255), Int(g * 255), Int(b * 255))
    }

    private func start() {
        guard let test else { return }
        guard selectedCount > 0 else {
            errorMessage = "Select at least one model, submodel, node or controller port first."
            return
        }
        syncParameters()
        if test.start() {
            running = true
        } else {
            errorMessage = viewModel.document.outputCount() == 0
                ? "No controllers are configured for this show."
                : "Couldn't reach any of the configured controllers. Check that the iPad is on the same network."
        }
    }

    private func stop() {
        guard running, let test else { return }
        test.stop()
        running = false
        status = ""
    }
}
