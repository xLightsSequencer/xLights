import SwiftUI

// J-32.5 — Transferable payload for Visualize drag-drop. Uses
// a String ProxyRepresentation so the transferred bytes are
// just the model name. NOT Codable — Swift's combined
// Codable+Transferable conformance auto-registers a custom
// UTType derived from the bundle ID + struct name ("org.
// xlights.iPad.visualizeModelDrag"), which then trips the
// "Type Declaration Issues" plist warning. ProxyRepresentation
// alone routes everything through `public.utf8-plain-text`.
//
// The bridge refuses any model name it doesn't recognise, so a
// stray text drop from outside the sheet is a no-op rather
// than a corruption hazard.
struct VisualizeModelDrag: Transferable {
    let name: String
    static var transferRepresentation: some TransferRepresentation {
        ProxyRepresentation(
            exporting: { $0.name },
            importing: { VisualizeModelDrag(name: $0) })
    }
}

// J-32.1 — Controllers Visualize (read-only wiring view).
//
// Renders the per-port model assignment graph for a single
// controller as a scrollable list. Source data is the bridge's
// `wiringForController:` which mirrors `UDController` on the
// C++ side (the same data the desktop Visualize dialog draws,
// and the same `UDController::Check` validity decisions).
// Subsequent J-32 sub-phases will layer in port property edits,
// smart-remote assignment, and drag-drop model assignment.

struct ControllerVisualizePayload: Identifiable {
    let id = UUID()
    let controllerName: String
}

struct ControllerVisualizeView: View {
    let controllerName: String
    let viewModel: SequencerViewModel
    let onTapModel: (String) -> Void
    let onDismiss: () -> Void

    @State private var wiring: [String: Any] = [:]
    @State private var loadToken: Int = 0
    @State private var protocolPickerTarget: PortProtocolTarget? = nil
    @State private var portEditError: String? = nil
    @State private var modelPropertiesTarget: ModelPropertiesTarget? = nil
    @State private var smartRemoteTarget: SmartRemoteTarget? = nil
    @State private var moveToPortTarget: MoveToPortTarget? = nil
    @State private var startChannelTarget: StartChannelTarget? = nil
    @State private var availableFilter: String = ""
    @State private var showOnlyUnassigned: Bool = true

    var body: some View {
        NavigationStack {
            content
                .navigationTitle("Visualize")
                .navigationBarTitleDisplayMode(.inline)
                .toolbar {
                    ToolbarItem(placement: .cancellationAction) {
                        Button("Done") { onDismiss() }
                    }
                    ToolbarItem(placement: .principal) {
                        Text(controllerName).font(.headline)
                    }
                    ToolbarItem(placement: .primaryAction) {
                        exportMenu
                    }
                }
        }
        .task(id: loadToken) {
            wiring = (viewModel.document.wiring(forController: controllerName)
                      as? [String: Any]) ?? [:]
        }
        .confirmationDialog(
            "Set Protocol",
            isPresented: Binding(get: { protocolPickerTarget != nil },
                                  set: { if !$0 { protocolPickerTarget = nil } }),
            titleVisibility: .visible,
            presenting: protocolPickerTarget
        ) { target in
            ForEach(target.options, id: \.self) { proto in
                Button(proto) {
                    applyPortProtocol(target: target, protocol: proto)
                }
            }
            Button("Cancel", role: .cancel) { }
        } message: { target in
            Text("\(target.displayName) — currently \(target.currentProtocol.isEmpty ? "—" : target.currentProtocol)")
        }
        .alert("Couldn't apply",
               isPresented: Binding(get: { portEditError != nil },
                                    set: { if !$0 { portEditError = nil } })) {
            Button("OK", role: .cancel) { }
        } message: {
            Text(portEditError ?? "")
        }
        .sheet(item: $modelPropertiesTarget) { target in
            ModelControllerPropertiesSheet(
                modelName: target.modelName,
                portKind: target.portKind,
                viewModel: viewModel,
                onCommit: { loadToken &+= 1 },
                onDismiss: { modelPropertiesTarget = nil })
        }
        .sheet(item: $smartRemoteTarget) { target in
            ModelSmartRemoteSheet(
                modelName: target.modelName,
                controllerName: controllerName,
                viewModel: viewModel,
                onCommit: { loadToken &+= 1 },
                onDismiss: { smartRemoteTarget = nil })
        }
        .sheet(item: $moveToPortTarget) { target in
            MoveToPortSheet(
                modelName: target.modelName,
                controllerName: controllerName,
                viewModel: viewModel,
                onCommit: { loadToken &+= 1 },
                onDismiss: { moveToPortTarget = nil })
        }
        .sheet(item: $startChannelTarget) { target in
            SetStartChannelSheet(
                modelName: target.modelName,
                viewModel: viewModel,
                onCommit: { loadToken &+= 1 },
                onDismiss: { startChannelTarget = nil })
        }
    }

    private func applyPortProtocol(target: PortProtocolTarget, protocol proto: String) {
        let ok = viewModel.document.setPortProtocol(onController: controllerName,
                                                     kind: target.kind,
                                                     port: Int32(target.port),
                                                     protocol: proto)
        if ok {
            loadToken &+= 1
        } else {
            portEditError = "The bridge refused to set \(proto) on \(target.displayName)."
        }
    }

    @ViewBuilder
    private var content: some View {
        if wiring.isEmpty {
            ContentUnavailableView(
                "Controller not found",
                systemImage: "exclamationmark.triangle",
                description: Text("No wiring data is available for '\(controllerName)'.")
            )
        } else {
            HStack(alignment: .top, spacing: 0) {
                wiringList
                Divider()
                availableModelsList
                    .frame(width: 300)
            }
        }
    }

    @ViewBuilder
    private var wiringList: some View {
        VStack(spacing: 0) {
            summaryStrip
            Divider()
            List {
                ForEach(portSections) { section in
                    portSection(section)
                }
                if !noConnectionEntries.isEmpty {
                    noConnectionSection
                }
            }
            .listStyle(.insetGrouped)
        }
    }

    @ViewBuilder
    private var noConnectionSection: some View {
        Section {
            ForEach(noConnectionEntries) { entry in
                modelRow(entry, showInvalidIcon: true)
                    .draggable(VisualizeModelDrag(name: entry.name)) {
                        modelDragPreview(entry.name)
                    }
                    .contextMenu {
                        Button {
                            moveToPortTarget = MoveToPortTarget(modelName: entry.name)
                        } label: {
                            Label("Assign to Port…",
                                  systemImage: "arrow.right.circle")
                        }
                        Button {
                            startChannelTarget = StartChannelTarget(modelName: entry.name)
                        } label: {
                            Label("Set Start Channel…",
                                  systemImage: "number.circle")
                        }
                    }
            }
        } header: {
            Label("No Connection (\(noConnectionEntries.count))",
                  systemImage: "exclamationmark.circle")
                .dropDestination(for: VisualizeModelDrag.self) { payloads, _ in
                    guard let drop = payloads.first else { return false }
                    let ok = viewModel.document.removeModelFromController(drop.name)
                    if ok { loadToken &+= 1 }
                    return ok
                }
                .foregroundStyle(.orange)
        } footer: {
            Text("Models claim this controller but aren't assigned to a port. Set the port + smart-remote in the model's Controller Connection properties.")
                .font(.caption)
        }
    }

    @State private var availableDropHover: Bool = false

    @ViewBuilder
    private var availableModelsList: some View {
        VStack(alignment: .leading, spacing: 0) {
            HStack(spacing: 6) {
                Image(systemName: "rectangle.stack")
                    .foregroundStyle(.secondary)
                Text("Available Models")
                    .font(.headline)
                Spacer()
                Text("\(filteredAvailableEntries.count)")
                    .font(.caption.monospacedDigit())
                    .foregroundStyle(.secondary)
                    .padding(.horizontal, 6)
                    .padding(.vertical, 2)
                    .background(Color.secondary.opacity(0.15), in: Capsule())
            }
            .padding(.horizontal, 12)
            .padding(.top, 12)
            .padding(.bottom, 6)

            Toggle(isOn: $showOnlyUnassigned) {
                Text("Unassigned only (\(unassignedCount))")
                    .font(.caption)
            }
            .toggleStyle(.switch)
            .controlSize(.mini)
            .padding(.horizontal, 12)
            .padding(.bottom, 6)

            TextField("Filter…", text: $availableFilter)
                .textFieldStyle(.roundedBorder)
                .textInputAutocapitalization(.never)
                .autocorrectionDisabled()
                .padding(.horizontal, 12)
                .padding(.bottom, 8)

            if availableEntries.isEmpty {
                Spacer()
                Text("Every model is already on this controller.")
                    .font(.callout)
                    .foregroundStyle(.secondary)
                    .multilineTextAlignment(.center)
                    .padding(.horizontal, 18)
                    .frame(maxWidth: .infinity)
                Spacer()
            } else if filteredAvailableEntries.isEmpty {
                Spacer()
                Text("No matches for '\(availableFilter)'.")
                    .font(.callout)
                    .foregroundStyle(.secondary)
                    .frame(maxWidth: .infinity)
                Spacer()
            } else {
                List {
                    ForEach(filteredAvailableEntries) { entry in
                        availableModelRow(entry)
                            .draggable(VisualizeModelDrag(name: entry.name)) {
                                modelDragPreview(entry.name)
                            }
                            .contextMenu {
                                Button {
                                    moveToPortTarget = MoveToPortTarget(modelName: entry.name)
                                } label: {
                                    Label("Assign to Port…",
                                          systemImage: "arrow.right.circle")
                                }
                            }
                    }
                }
                .listStyle(.plain)
            }
        }
        .frame(maxHeight: .infinity)
        .background(availableDropHover
                     ? Color.accentColor.opacity(0.18)
                     : Color(.secondarySystemBackground))
        .overlay(alignment: .top) {
            if availableDropHover {
                Label("Drop to remove from controller",
                      systemImage: "minus.circle.fill")
                    .font(.caption.weight(.semibold))
                    .padding(.horizontal, 10)
                    .padding(.vertical, 5)
                    .background(.regularMaterial, in: Capsule())
                    .padding(.top, 8)
            }
        }
        .contentShape(Rectangle())
        .dropDestination(for: VisualizeModelDrag.self) { payloads, _ in
            guard let drop = payloads.first else { return false }
            let ok = viewModel.document.removeModelFromController(drop.name)
            if ok { loadToken &+= 1 }
            return ok
        } isTargeted: { hovering in
            availableDropHover = hovering
        }
    }

    @ViewBuilder
    private func availableModelRow(_ entry: AvailableModelEntry) -> some View {
        HStack(alignment: .firstTextBaseline, spacing: 8) {
            Image(systemName: "rectangle.connected.to.line.below")
                .foregroundStyle(.secondary)
                .frame(width: 18)
            VStack(alignment: .leading, spacing: 2) {
                Text(entry.name)
                if !entry.currentLocation.isEmpty {
                    Text(entry.currentLocation)
                        .font(.caption2)
                        .foregroundStyle(.secondary)
                }
            }
            Spacer()
            Text("\(entry.channels)ch")
                .font(.caption.monospacedDigit())
                .foregroundStyle(.tertiary)
        }
    }

    // MARK: - Summary strip

    @ViewBuilder
    private var summaryStrip: some View {
        let vendor  = stringValue("vendor")
        let model   = stringValue("model")
        let variant = stringValue("variant")
        let vm      = [vendor, model].filter { !$0.isEmpty }.joined(separator: " ")
        let descriptor = variant.isEmpty
            ? vm
            : (vm.isEmpty ? variant : "\(vm) (\(variant))")
        let ip = stringValue("ip")

        let totalsLine: String? = {
            guard let totals = wiring["totals"] as? [String: Any] else { return nil }
            let m = totals["models"]   as? Int ?? 0
            let c = totals["channels"] as? Int ?? 0
            let mLabel = "\(m) model\(m == 1 ? "" : "s")"
            let cLabel = "\(c.formatted(.number)) channels"
            return "\(mLabel) · \(cLabel)"
        }()

        let isInvalid = (wiring["valid"] as? Bool == false)
        let errorMsg = wiring["errorMessage"] as? String ?? ""

        VStack(alignment: .leading, spacing: 2) {
            HStack(alignment: .firstTextBaseline, spacing: 8) {
                if !descriptor.isEmpty {
                    Text(descriptor)
                        .font(.caption.weight(.semibold))
                        .lineLimit(1)
                        .truncationMode(.middle)
                }
                Spacer(minLength: 8)
                if !ip.isEmpty {
                    Text(ip)
                        .font(.caption.monospacedDigit())
                        .foregroundStyle(.secondary)
                        .textSelection(.enabled)
                }
            }
            if let totalsLine {
                Text(totalsLine)
                    .font(.caption2.monospacedDigit())
                    .foregroundStyle(.secondary)
            }
            if isInvalid, !errorMsg.isEmpty {
                Label(errorMsg, systemImage: "exclamationmark.triangle.fill")
                    .font(.caption2)
                    .foregroundStyle(.orange)
                    .lineLimit(2)
            }
        }
        .padding(.horizontal, 14)
        .padding(.vertical, 6)
        .frame(maxWidth: .infinity, alignment: .leading)
        .background(Color(uiColor: .secondarySystemBackground))
    }

    // MARK: - Port sections

    @ViewBuilder
    private func portSection(_ section: PortSection) -> some View {
        Section {
            ForEach(section.modelEntries) { entry in
                modelRow(entry, showInvalidIcon: !section.valid)
                    .draggable(VisualizeModelDrag(name: entry.name)) {
                        modelDragPreview(entry.name)
                    }
                    .dropDestination(for: VisualizeModelDrag.self) { payloads, _ in
                        return handleDrop(payloads: payloads,
                                           section: section,
                                           afterModel: entry.name)
                    }
                    .contextMenu {
                        Button {
                            onTapModel(entry.name)
                        } label: {
                            Label("Go to Model on Canvas",
                                  systemImage: "scope")
                        }
                        Button {
                            modelPropertiesTarget = ModelPropertiesTarget(
                                modelName: entry.name,
                                portKind: section.kind)
                        } label: {
                            Label("Edit Controller Properties…",
                                  systemImage: "slider.horizontal.3")
                        }
                        if section.kind == "pixel" {
                            Button {
                                smartRemoteTarget = SmartRemoteTarget(
                                    modelName: entry.name)
                            } label: {
                                Label("Set Smart Remote…",
                                      systemImage: "antenna.radiowaves.left.and.right.circle")
                            }
                        }
                        Button {
                            moveToPortTarget = MoveToPortTarget(modelName: entry.name)
                        } label: {
                            Label("Move to Port…",
                                  systemImage: "arrow.right.circle")
                        }
                        Button {
                            startChannelTarget = StartChannelTarget(modelName: entry.name)
                        } label: {
                            Label("Set Start Channel…",
                                  systemImage: "number.circle")
                        }
                        Button(role: .destructive) {
                            unassignModel(entry.name)
                        } label: {
                            Label("Remove from Controller",
                                  systemImage: "minus.circle")
                        }
                    }
            }
            if section.modelEntries.isEmpty {
                Text("Drop a model here to assign it to this port")
                    .font(.callout)
                    .foregroundStyle(.secondary)
                    .frame(maxWidth: .infinity, alignment: .leading)
                    .contentShape(Rectangle())
                    .dropDestination(for: VisualizeModelDrag.self) { payloads, _ in
                        return handleDrop(payloads: payloads,
                                           section: section,
                                           afterModel: nil)
                    }
            }
        } header: {
            portHeader(section)
                .dropDestination(for: VisualizeModelDrag.self) { payloads, _ in
                    return handleDrop(payloads: payloads,
                                       section: section,
                                       afterModel: nil)
                }
        } footer: {
            if !section.valid, !section.invalidReason.isEmpty {
                Label(section.invalidReason, systemImage: "exclamationmark.triangle")
                    .font(.caption)
                    .foregroundStyle(.orange)
            }
        }
    }

    @ViewBuilder
    private func modelDragPreview(_ name: String) -> some View {
        HStack(spacing: 6) {
            Image(systemName: "rectangle.connected.to.line.below")
                .foregroundStyle(.tint)
            Text(name).font(.body)
        }
        .padding(.horizontal, 10)
        .padding(.vertical, 6)
        .background(.thinMaterial, in: Capsule())
    }

    private func handleDrop(payloads: [VisualizeModelDrag],
                             section: PortSection,
                             afterModel: String?) -> Bool {
        guard let drop = payloads.first else { return false }
        // Drop on itself or on the model immediately after it
        // (which would be the destination's "after") is a no-op.
        if drop.name == afterModel { return false }
        let ok = viewModel.document.assignModel(
            drop.name,
            toController: controllerName,
            kind: section.kind,
            port: Int32(section.port),
            afterModel: afterModel,
            smartRemote: -1)
        if ok {
            loadToken &+= 1
            return true
        }
        portEditError = "Couldn't move \(drop.name) onto \(section.displayName)."
        return false
    }

    private func unassignModel(_ name: String) {
        if viewModel.document.removeModelFromController(name) {
            loadToken &+= 1
        } else {
            portEditError = "Couldn't remove \(name) from the controller."
        }
    }

    @ViewBuilder
    private var exportMenu: some View {
        Menu {
            if let csv = viewModel.document.exportWiringCSV(forController: controllerName) {
                ShareLink(item: csv,
                          preview: SharePreview("\(controllerName) wiring.csv")) {
                    Label("Export as CSV", systemImage: "tablecells")
                }
            }
            if let json = viewModel.document.exportWiringJSON(forController: controllerName) {
                ShareLink(item: json,
                          preview: SharePreview("\(controllerName) wiring.json")) {
                    Label("Export as JSON",
                          systemImage: "curlybraces.square")
                }
            }
        } label: {
            Image(systemName: "square.and.arrow.up")
        }
    }

    @ViewBuilder
    private func portHeader(_ section: PortSection) -> some View {
        let isEditable = section.kind == "pixel" || section.kind == "serial"
        if isEditable {
            Menu {
                Button {
                    openProtocolPicker(section: section)
                } label: {
                    Label("Set Protocol…",
                          systemImage: "antenna.radiowaves.left.and.right")
                }
            } label: {
                portHeaderRow(section, asMenu: true)
            }
        } else {
            portHeaderRow(section, asMenu: false)
        }
    }

    @ViewBuilder
    private func portHeaderRow(_ section: PortSection, asMenu: Bool) -> some View {
        HStack(alignment: .firstTextBaseline) {
            Text(section.displayName)
                .font(.headline)
            if !section.proto.isEmpty {
                Text(section.proto)
                    .font(.caption.weight(.semibold))
                    .padding(.horizontal, 6)
                    .padding(.vertical, 2)
                    .background(Color.accentColor.opacity(0.15),
                                in: Capsule())
                    .foregroundStyle(Color.accentColor)
            }
            if asMenu {
                Image(systemName: "ellipsis.circle")
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }
            Spacer()
            if section.channels > 0 {
                Text("ch \(section.startChannel)–\(section.endChannel) · \(section.channels)ch")
                    .font(.caption.monospacedDigit())
                    .foregroundStyle(.secondary)
            }
        }
        .textCase(nil)
    }

    private func openProtocolPicker(section: PortSection) {
        let options = viewModel.document.availableProtocols(forController: controllerName,
                                                             kind: section.kind)
        guard !options.isEmpty else {
            portEditError = "No protocols are available for this port."
            return
        }
        protocolPickerTarget = PortProtocolTarget(
            kind: section.kind,
            port: section.port,
            displayName: section.displayName,
            currentProtocol: section.proto,
            options: options)
    }

    @ViewBuilder
    private func modelRow(_ entry: ModelEntry, showInvalidIcon: Bool) -> some View {
        Button {
            onTapModel(entry.name)
        } label: {
            HStack(alignment: .firstTextBaseline, spacing: 8) {
                if !entry.valid {
                    Image(systemName: "exclamationmark.triangle.fill")
                        .foregroundStyle(.orange)
                } else if entry.smartRemoteLetter.isEmpty == false {
                    Text(entry.smartRemoteLetter)
                        .font(.caption.weight(.bold).monospaced())
                        .frame(width: 18, height: 18)
                        .background(Color.accentColor.opacity(0.2),
                                    in: Circle())
                        .foregroundStyle(Color.accentColor)
                } else {
                    Image(systemName: "circle.fill")
                        .font(.system(size: 6))
                        .foregroundStyle(.secondary)
                        .frame(width: 18)
                }
                VStack(alignment: .leading, spacing: 2) {
                    HStack {
                        Text(entry.name)
                            .font(.body)
                        if entry.string > 0 {
                            Text("string \(entry.string + 1)")
                                .font(.caption2)
                                .foregroundStyle(.secondary)
                        }
                    }
                    if !entry.valid, !entry.invalidReason.isEmpty {
                        Text(entry.invalidReason)
                            .font(.caption2)
                            .foregroundStyle(.orange)
                    }
                }
                Spacer()
                VStack(alignment: .trailing, spacing: 2) {
                    Text("\(entry.startChannel)–\(entry.endChannel)")
                        .font(.caption.monospacedDigit())
                        .foregroundStyle(.secondary)
                    Text("\(entry.channels)ch")
                        .font(.caption2.monospacedDigit())
                        .foregroundStyle(.tertiary)
                }
            }
            .contentShape(Rectangle())
        }
        .buttonStyle(.plain)
    }

    // MARK: - Data shaping

    private struct PortSection: Identifiable {
        let id = UUID()
        let kind: String
        let port: Int
        let displayName: String
        let proto: String
        let valid: Bool
        let invalidReason: String
        let startChannel: Int
        let endChannel: Int
        let channels: Int
        let modelEntries: [ModelEntry]
    }

    private struct ModelPropertiesTarget: Identifiable {
        let id = UUID()
        let modelName: String
        let portKind: String
    }

    private struct SmartRemoteTarget: Identifiable {
        let id = UUID()
        let modelName: String
    }

    private struct MoveToPortTarget: Identifiable {
        let id = UUID()
        let modelName: String
    }

    private struct StartChannelTarget: Identifiable {
        let id = UUID()
        let modelName: String
    }

    private struct AvailableModelEntry: Identifiable {
        let id = UUID()
        let name: String
        let channels: Int
        let currentLocation: String  // e.g. "On FalconF16, Pixel 3" or ""
        let isUnassigned: Bool
    }

    private struct PortProtocolTarget: Identifiable {
        let id = UUID()
        let kind: String   // "pixel" / "serial"
        let port: Int
        let displayName: String
        let currentProtocol: String
        let options: [String]
    }

    private struct ModelEntry: Identifiable {
        let id = UUID()
        let name: String
        let string: Int
        let startChannel: Int
        let endChannel: Int
        let channels: Int
        let smartRemote: Int
        let smartRemoteLetter: String
        let valid: Bool
        let invalidReason: String
    }

    private var portSections: [PortSection] {
        guard let ports = wiring["ports"] as? [[String: Any]] else { return [] }
        return ports.map { p in
            let kind  = p["kind"]  as? String ?? ""
            let port  = p["port"]  as? Int ?? 0
            let name  = p["name"]  as? String ?? ""
            let proto = p["protocol"] as? String ?? ""
            let valid = p["valid"] as? Bool ?? true
            let reason = p["invalidReason"] as? String ?? ""
            let sc = p["startChannel"] as? Int ?? 0
            let ec = p["endChannel"]   as? Int ?? 0
            let ch = p["channels"]     as? Int ?? 0
            let modelDicts = p["models"] as? [[String: Any]] ?? []
            return PortSection(
                kind: kind, port: port, displayName: name,
                proto: proto, valid: valid, invalidReason: reason,
                startChannel: sc, endChannel: ec, channels: ch,
                modelEntries: modelDicts.map(makeEntry))
        }
    }

    private var noConnectionEntries: [ModelEntry] {
        guard let arr = wiring["noConnection"] as? [[String: Any]] else { return [] }
        return arr.map(makeEntry)
    }

    private var availableEntries: [AvailableModelEntry] {
        guard let arr = wiring["availableModels"] as? [[String: Any]] else { return [] }
        return arr.compactMap { d in
            let name = d["name"] as? String ?? ""
            if name.isEmpty { return nil }
            let ch = d["channels"] as? Int ?? 0
            let curCtrl = d["controllerName"] as? String ?? ""
            let curPort = d["controllerPort"] as? Int ?? 0
            let isUnassigned = curCtrl.isEmpty || curCtrl == "No Controller"
            var loc = ""
            if !isUnassigned {
                loc = curPort > 0
                    ? "Currently on \(curCtrl), port \(curPort)"
                    : "Assigned to \(curCtrl)"
            }
            return AvailableModelEntry(name: name, channels: ch,
                                        currentLocation: loc,
                                        isUnassigned: isUnassigned)
        }
        .sorted { $0.name.localizedCaseInsensitiveCompare($1.name) == .orderedAscending }
    }

    private var filteredAvailableEntries: [AvailableModelEntry] {
        let needle = availableFilter.trimmingCharacters(in: .whitespaces)
        return availableEntries.filter { entry in
            if showOnlyUnassigned && !entry.isUnassigned { return false }
            if !needle.isEmpty,
               !entry.name.localizedCaseInsensitiveContains(needle) { return false }
            return true
        }
    }

    private var unassignedCount: Int {
        availableEntries.reduce(0) { $0 + ($1.isUnassigned ? 1 : 0) }
    }

    private func makeEntry(_ d: [String: Any]) -> ModelEntry {
        ModelEntry(
            name: d["name"] as? String ?? "",
            string: d["string"] as? Int ?? 0,
            startChannel: d["startChannel"] as? Int ?? 0,
            endChannel: d["endChannel"] as? Int ?? 0,
            channels: d["channels"] as? Int ?? 0,
            smartRemote: d["smartRemote"] as? Int ?? 0,
            smartRemoteLetter: d["smartRemoteLetter"] as? String ?? "",
            valid: d["valid"] as? Bool ?? true,
            invalidReason: d["invalidReason"] as? String ?? "")
    }

    private func stringValue(_ key: String) -> String {
        wiring[key] as? String ?? ""
    }
}
