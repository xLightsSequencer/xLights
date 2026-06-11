import SwiftUI
import UniformTypeIdentifiers

// Phase I-2 — Tools → Import Effects.
//
// Two-pane sheet: source list on the left, destination list on the
// right. Tap-to-select on either side; "Map →" assigns the selected
// source to the selected destination. "Auto Map" runs the desktop's
// two-pass alias-driven matcher plus every regex hint under
// `<showdir>/maphints/*.xmaphint`.
//
// v1 scope: top-level model rows only on both sides (the bridge
// builds flat trees). Submodel/strand/node hierarchy lands once the
// bridge gains hierarchical tree-builders.

struct ImportEffectsView: View {
    let viewModel: SequencerViewModel
    let onDismiss: () -> Void

    // Branch on the picked file's extension. `.xsq`/`.xsqz` use the
    // existing channel-mapping flow; `.sup` swaps in the SuperStar
    // single-target-model form (Phase I-4). The two flows share the
    // same outer sheet so the user-facing entry point stays "Tools →
    // Import Effects" regardless of file format.
    private enum Mode { case xsq, superstar }

    @State private var mode: Mode = .xsq
    @State private var session: XLImportSession?
    @State private var sourcePath: String?
    // `.sup` only — held so the SuperStar form can re-acquire the
    // document picker's security scope around the file read.
    @State private var superStarURL: URL?
    @State private var availableSources: [XLImportAvailableSource] = []
    @State private var destinationRows: [XLImportMappingRow] = []
    @State private var timingTracks: [XLImportTimingTrack] = []
    @State private var selectedSourceDisplay: String?
    @State private var selectedDestNodeID: Int = 0
    @State private var rowExpansion: Set<Int> = []
    @State private var showingFilePicker = false
    @State private var importError: String?
    // IE-7 — non-blocking pre/post-import warnings. `loadWarning` (FPS / version
    // mismatch) appears after the source loads and just informs; `applyWarning`
    // (missing source media) appears after Apply and dismisses the sheet on ack.
    @State private var loadWarning: String?
    @State private var applyWarning: String?
    // IE-2 — persist the import-option toggles across launches (mirrors
    // desktop's persisted import options, e.g. ImportEffectsRenderStyle).
    @AppStorage("import.eraseExisting") private var eraseExisting = false
    @AppStorage("import.lockEffects") private var lockEffects = false
    @AppStorage("import.convertRenderStyle") private var convertRenderStyle = false
    // IE-9 — incremental filters over the source / destination lists.
    // High leverage on large shows where the model count is in the
    // hundreds. Pure-SwiftUI over the existing snapshot arrays.
    @State private var sourceSearch = ""
    @State private var destSearch = ""

    private var filteredSources: [XLImportAvailableSource] {
        guard !sourceSearch.isEmpty else { return availableSources }
        return availableSources.filter {
            $0.displayName.localizedCaseInsensitiveContains(sourceSearch)
        }
    }

    private var filteredDestinationRows: [XLImportMappingRow] {
        guard !destSearch.isEmpty else { return destinationRows }
        return destinationRows.filter {
            $0.model.localizedCaseInsensitiveContains(destSearch)
                || $0.mapping.localizedCaseInsensitiveContains(destSearch)
        }
    }
    @State private var showingSaveHints = false
    @State private var showingLoadHints = false      // IE-3
    @State private var resultMessage: String?        // IE-3 / IE-12 result alert
    @State private var showingTimingPopover = false
    @State private var aiMapRunning = false           // AI structured mapping in flight

    var body: some View {
        NavigationStack {
            VStack(spacing: 0) {
                if sourcePath == nil {
                    sourcePicker
                } else if mode == .superstar, let url = superStarURL {
                    SuperStarImportView(viewModel: viewModel,
                                        sourceURL: url,
                                        onCancel: { onDismiss() },
                                        onApplied: { onDismiss() })
                } else {
                    mappingPanes
                    Divider()
                    optionsBar
                }
            }
            .navigationTitle(mode == .superstar ? "Import SuperStar" : "Import Effects")
            .toolbar {
                if mode != .superstar {
                    ToolbarItem(placement: .cancellationAction) {
                        Button("Cancel") { onDismiss() }
                    }
                    if sourcePath != nil {
                        ToolbarItem(placement: .confirmationAction) {
                            Button("Apply") { applyImport() }
                                .disabled(mappedCount == 0 && selectedTimingCount == 0)
                        }
                    }
                }
            }
            .alert("Import Error", isPresented: errorBinding,
                   presenting: importError) { _ in
                Button("OK", role: .cancel) { importError = nil }
            } message: { msg in
                Text(msg)
            }
            .alert("Frame Rate Differs", isPresented: Binding(
                       get: { loadWarning != nil },
                       set: { if !$0 { loadWarning = nil } }),
                   presenting: loadWarning) { _ in
                Button("OK", role: .cancel) { loadWarning = nil }
            } message: { msg in
                Text(msg)
            }
            .alert("Missing Media", isPresented: Binding(
                       get: { applyWarning != nil },
                       set: { if !$0 { applyWarning = nil } }),
                   presenting: applyWarning) { _ in
                Button("OK", role: .cancel) { applyWarning = nil; onDismiss() }
            } message: { msg in
                Text(msg)
            }
        }
        .onAppear {
            // Build the session up-front (it populates the destination
            // tree from the active sequence's models) and immediately
            // raise the file picker — the very first thing the user
            // sees should be a "pick the source `.xsq` / `.xsqz`"
            // dialog, not an empty mapping pane.
            if session == nil {
                session = XLImportSession(document: viewModel.document)
                refreshSnapshots()
                showingFilePicker = true
            }
        }
        .fileImporter(isPresented: $showingFilePicker,
                      allowedContentTypes: ImportEffectsView.allowedTypes,
                      allowsMultipleSelection: false) { result in
            switch result {
            case .success(let urls):
                if let url = urls.first { loadSource(url: url) }
            case .failure(let err):
                importError = err.localizedDescription
            }
        }
    }

    // MARK: - Steps

    private var sourcePicker: some View {
        VStack(spacing: 16) {
            Spacer()
            Image(systemName: "arrow.down.doc")
                .font(.system(size: 56))
                .foregroundStyle(.secondary)
            Text("Pick a sequence to import effects from.")
                .font(.title3)
                .multilineTextAlignment(.center)
            Text(".xsq / .xsqz xLights sequences, .sup SuperStar files, .loredit LOR S5, and .lms / .las LOR Music / Animation exports are supported.")
                .font(.subheadline)
                .foregroundStyle(.secondary)
                .multilineTextAlignment(.center)
            Button {
                showingFilePicker = true
            } label: {
                Label("Choose Source Sequence…", systemImage: "folder")
                    .padding(.horizontal, 8)
            }
            .buttonStyle(.borderedProminent)
            .controlSize(.large)
            Spacer()
        }
        .padding()
    }

    private var mappingPanes: some View {
        HStack(spacing: 0) {
            sourcePane
            Divider()
            destinationPane
        }
    }

    // IE-9 reusable filter field for the source / destination panes.
    @ViewBuilder
    private func searchField(_ text: Binding<String>, prompt: String) -> some View {
        HStack(spacing: 6) {
            Image(systemName: "magnifyingglass").foregroundStyle(.secondary)
            TextField(prompt, text: text)
                .textFieldStyle(.plain)
                .autocorrectionDisabled()
                .textInputAutocapitalization(.never)
            if !text.wrappedValue.isEmpty {
                Button { text.wrappedValue = "" } label: {
                    Image(systemName: "xmark.circle.fill").foregroundStyle(.secondary)
                }
                .buttonStyle(.plain)
            }
        }
        .padding(.horizontal)
        .padding(.vertical, 6)
    }

    private var sourcePane: some View {
        VStack(alignment: .leading, spacing: 0) {
            HStack {
                Text("Source")
                    .font(.headline)
                Spacer()
                Text(sourceSearch.isEmpty
                     ? "\(availableSources.count)"
                     : "\(filteredSources.count)/\(availableSources.count)")
                    .foregroundStyle(.secondary)
                    .monospacedDigit()
            }
            .padding(.horizontal)
            .padding(.vertical, 8)
            searchField($sourceSearch, prompt: "Filter source models")
            Divider()
            ScrollView {
                LazyVStack(alignment: .leading, spacing: 0) {
                    ForEach(filteredSources, id: \.displayName) { src in
                        let isSelected = selectedSourceDisplay == src.displayName
                        HStack {
                            Text(src.displayName)
                            Spacer()
                        }
                        .padding(.horizontal)
                        .padding(.vertical, 8)
                        .frame(maxWidth: .infinity, alignment: .leading)
                        .background(isSelected ? Color.accentColor.opacity(0.2) : Color.clear)
                        .contentShape(Rectangle())
                        .onTapGesture {
                            selectedSourceDisplay = isSelected ? nil : src.displayName
                        }
                        Divider()
                    }
                }
            }
        }
        .frame(maxWidth: .infinity)
    }

    private var destinationPane: some View {
        VStack(alignment: .leading, spacing: 0) {
            HStack {
                Text("Destination")
                    .font(.headline)
                Spacer()
                Text("\(mappedCount) / \(totalDestinationCount) mapped")
                    .foregroundStyle(.secondary)
                    .monospacedDigit()
            }
            .padding(.horizontal)
            .padding(.vertical, 8)
            searchField($destSearch, prompt: "Filter destination models")
            Divider()
            ScrollView {
                LazyVStack(alignment: .leading, spacing: 0) {
                    ForEach(filteredDestinationRows, id: \.nodeID) { row in
                        DestinationRowView(row: row,
                                            depth: 0,
                                            selectedID: $selectedDestNodeID,
                                            expansion: $rowExpansion)
                    }
                }
            }
        }
        .frame(maxWidth: .infinity)
    }

    private var optionsBar: some View {
        VStack(spacing: 8) {
            HStack(spacing: 12) {
                Button("Map →") { mapSelected() }
                    .disabled(selectedSourceDisplay == nil || selectedDestNodeID == 0)
                Button("Unmap") { unmapSelected() }
                    .disabled(selectedDestNodeID == 0)
                Button("Auto Map") { autoMap() }
                    .buttonStyle(.borderedProminent)
                if XLAIServices.shared().hasEnabledService(forCapability: XLAICapabilityMapping) {
                    Button {
                        aiMap()
                    } label: {
                        if aiMapRunning {
                            ProgressView().controlSize(.small)
                        } else {
                            Label("AI Map", systemImage: "wand.and.stars")
                        }
                    }
                    .disabled(aiMapRunning)
                }
                Spacer()
                if !timingTracks.isEmpty {
                    Button {
                        showingTimingPopover = true
                    } label: {
                        Label("Timing (\(selectedTimingCount)/\(timingTracks.count))",
                              systemImage: "metronome")
                    }
                    .popover(isPresented: $showingTimingPopover) {
                        timingTrackPopover
                            .frame(minWidth: 280, minHeight: 200)
                    }
                }
                Button("Save Hints…") { showingSaveHints = true }
                    .disabled(mappedCount == 0)
                Button("Load Hints…") { showingLoadHints = true }   // IE-3
                Button("Update Aliases") { updateAliases() }        // IE-12
                    .disabled(mappedCount == 0)
            }
            HStack(spacing: 16) {
                Toggle("Erase existing", isOn: $eraseExisting)
                Toggle("Lock imported", isOn: $lockEffects)
                Toggle("Convert render style", isOn: $convertRenderStyle)
                Spacer()
            }
            .toggleStyle(.switch)
        }
        .padding()
        .fileExporter(isPresented: $showingSaveHints,
                      document: MapHintsDocument(),
                      contentType: .xml,
                      defaultFilename: "import.xmaphint") { result in
            switch result {
            case .success(let url):
                _ = session?.saveMapHints(toPath: url.path)
            case .failure(let err):
                importError = err.localizedDescription
            }
        }
        // IE-3 — load a user-picked .xmaphint and apply its regex hints.
        .fileImporter(isPresented: $showingLoadHints,
                      allowedContentTypes: [UTType(filenameExtension: "xmaphint") ?? .xml],
                      allowsMultipleSelection: false) { result in
            loadHints(result: result)
        }
        .alert("Import",
               isPresented: Binding(get: { resultMessage != nil },
                                    set: { if !$0 { resultMessage = nil } })) {
            Button("OK", role: .cancel) { resultMessage = nil }
        } message: {
            Text(resultMessage ?? "")
        }
    }

    // MARK: - Actions

    // IE-12 — alias each mapped source name onto its destination model.
    private func updateAliases() {
        guard let session else { return }
        let n = Int(session.updateModelAliasesFromMapping())
        resultMessage = "\(n) alias\(n == 1 ? "" : "es") added. Future imports of these models will auto-map."
    }

    // IE-3 — apply a chosen .xmaphint file's regex hints to the tree.
    private func loadHints(result: Result<[URL], Error>) {
        guard let session else { return }
        guard case .success(let urls) = result, let url = urls.first else { return }
        let needs = url.startAccessingSecurityScopedResource()
        defer { if needs { url.stopAccessingSecurityScopedResource() } }
        _ = XLSequenceDocument.obtainAccess(toPath: url.path, enforceWritable: false)
        let applied = Int(session.loadMapHints(fromPath: url.path))
        refreshSnapshots()
        resultMessage = applied > 0
            ? "Applied \(applied) hint\(applied == 1 ? "" : "s") from the file."
            : "No hints were applied (the file may be empty or none matched the source)."
    }

    private func loadSource(url: URL) {
        let ext = url.pathExtension.lowercased()
        if ext == "sup" {
            // SuperStar — switch the sheet's body to the single-model
            // form. We deliberately do NOT acquire / release the
            // security scope here: the SuperStar form re-acquires it
            // itself around the actual file read so the scope is held
            // for the lifetime of one read instead of the lifetime of
            // this function.
            mode = .superstar
            superStarURL = url
            sourcePath = url.path
            return
        }
        let started = url.startAccessingSecurityScopedResource()
        defer { if started { url.stopAccessingSecurityScopedResource() } }
        guard let session else { return }
        do {
            if ext == "loredit" {
                // LOR S5 effect-level export — parsed by the core LOREdit
                // reader. Shares the same mapping panes / AutoMapper flow as
                // `.xsq`; only the source-discovery entry point differs.
                try session.loadLOREditSource(atPath: url.path)
            } else if ext == "tim" {
                // Vixen 3 effect-level export — parsed by the core Vixen3
                // reader (needs the sibling SystemConfig.xml). Shares the
                // mapping panes with `.xsq`; only source discovery differs.
                // (The Settings → Timings tab handles `.tim` as timing-only.)
                try session.loadVixen3Source(atPath: url.path)
            } else if ext == "lms" || ext == "las" {
                // LOR Music / Animation effect-level export — parsed by the
                // core LORMusic reader. Shares the mapping panes / AutoMapper
                // flow with `.xsq`; only source discovery differs.
                try session.loadLMSSource(atPath: url.path)
            } else {
                try session.loadSourceSequence(atPath: url.path)
            }
            mode = .xsq
            sourcePath = url.path
            refreshSnapshots()
            checkSourceWarnings()
        } catch {
            importError = error.localizedDescription
        }
    }

    // IE-7 — surface a non-blocking FPS-mismatch notice after the source loads
    // (mirrors desktop's pre-import frame-rate warning). Only `.xsq`/package
    // sources expose a frequency; `.loredit`/`.tim` report 0 and are skipped.
    private func checkSourceWarnings() {
        guard let session else { return }
        let src = session.sourceFrequency()
        let tgt = session.targetFrequency()
        if src > 0 && tgt > 0 && src != tgt {
            loadWarning = "This sequence was created at \(src) FPS but the current sequence runs at \(tgt) FPS. Imported effect timings will be snapped to the current frame rate."
        }
    }

    private func mapSelected() {
        guard let session, let src = selectedSourceDisplay else { return }
        let modelType = availableSources.first(where: { $0.displayName == src })?.modelType ?? "Model"
        session.setMappingForRow(intptr_t(selectedDestNodeID),
                                  sourceDisplayName: src,
                                  modelType: modelType)
        refreshSnapshots()
    }

    private func unmapSelected() {
        guard let session else { return }
        session.setMappingForRow(intptr_t(selectedDestNodeID),
                                  sourceDisplayName: nil as String?,
                                  modelType: nil as String?)
        refreshSnapshots()
    }

    private func autoMap() {
        guard let session else { return }
        session.runAutoMap()
        refreshSnapshots()
    }

    private func aiMap() {
        guard let session else { return }
        aiMapRunning = true
        session.runAIMap { applied, error in
            aiMapRunning = false
            if let error {
                resultMessage = error
            } else {
                resultMessage = applied > 0
                    ? "AI mapped \(applied) model\(applied == 1 ? "" : "s")."
                    : "AI returned no new mappings."
                refreshSnapshots()
            }
        }
    }

    private func applyImport() {
        guard let session else { return }
        do {
            try session.applyImport(withEraseExisting: eraseExisting,
                                     lock: lockEffects,
                                     convertRenderStyle: convertRenderStyle)
            viewModel.reloadRows()
            // IE-7 — the source's missing-media list is only populated by the
            // SequencePackage media walk that runs during an `.xsq`/package
            // apply, so surface it now. (`.loredit`/`.tim` carry no package
            // media, so this is empty for them — which correctly means "no
            // warning, dismiss normally".) Defer the dismiss until the user
            // acknowledges, otherwise onDismiss() closes the sheet first and
            // the alert never shows.
            let missing = session.sourceMissingMedia()
            if !missing.isEmpty {
                let shown = missing.prefix(10).joined(separator: "\n")
                let more = missing.count > 10 ? "\n…and \(missing.count - 10) more" : ""
                applyWarning = "Effects were imported, but \(missing.count) media file\(missing.count == 1 ? "" : "s") referenced by the source could not be found:\n\(shown)\(more)\n\nAdd them to your show or media folder and re-import to restore those effects' media."
            } else {
                onDismiss()
            }
        } catch {
            importError = error.localizedDescription
        }
    }

    private func refreshSnapshots() {
        guard let session else { return }
        availableSources = session.availableSources()
        destinationRows = session.destinationRows()
        timingTracks = session.timingTracks()
    }

    private var timingTrackPopover: some View {
        VStack(alignment: .leading, spacing: 0) {
            Text("Import Timing Tracks")
                .font(.headline)
                .padding()
            Divider()
            ScrollView {
                LazyVStack(alignment: .leading, spacing: 0) {
                    ForEach(timingTracks, id: \.name) { track in
                        let binding = Binding<Bool>(
                            get: { track.selected },
                            set: { newValue in
                                session?.setTimingTrackImport(track.name, enabled: newValue)
                                timingTracks = session?.timingTracks() ?? []
                            })
                        Toggle(isOn: binding) {
                            HStack {
                                Text(track.name)
                                if track.alreadyExists {
                                    Text("(already in sequence)")
                                        .font(.caption)
                                        .foregroundStyle(.secondary)
                                }
                            }
                        }
                        .padding(.horizontal)
                        .padding(.vertical, 6)
                        Divider()
                    }
                }
            }
        }
    }

    private var selectedTimingCount: Int {
        timingTracks.reduce(0) { $0 + ($1.selected ? 1 : 0) }
    }

    // MARK: - Helpers

    private var mappedCount: Int {
        var n = 0
        func walk(_ row: XLImportMappingRow) {
            if !row.mapping.isEmpty { n += 1 }
            for child in row.children { walk(child) }
        }
        for row in destinationRows { walk(row) }
        return n
    }

    private var totalDestinationCount: Int {
        var n = 0
        func walk(_ row: XLImportMappingRow) {
            n += 1
            for child in row.children { walk(child) }
        }
        for row in destinationRows { walk(row) }
        return n
    }

    private var errorBinding: Binding<Bool> {
        Binding(get: { importError != nil },
                set: { if !$0 { importError = nil } })
    }

    private static let allowedTypes: [UTType] = {
        var types: [UTType] = []
        // `.xsq` (loose) and `.xsqz` (package — vendor distribution
        // format with embedded audio + media). The bridge handles
        // both via SequencePackage::Extract.
        if let xsq = UTType(filenameExtension: "xsq") { types.append(xsq) }
        if let xsqz = UTType(filenameExtension: "xsqz") { types.append(xsqz) }
        if let zip = UTType(filenameExtension: "zip") { types.append(zip) }
        types.append(.xml)  // legacy .xml sequences
        if let sup = UTType(filenameExtension: "sup") { types.append(sup) }
        // LOR S5 effect-level export (parsed by the core LOREdit reader).
        if let loredit = UTType(filenameExtension: "loredit") { types.append(loredit) }
        // Vixen 3 effect-level export (parsed by the core Vixen3 reader).
        if let tim = UTType(filenameExtension: "tim") { types.append(tim) }
        // LOR Music / Animation effect-level export (parsed by core LORMusic).
        if let lms = UTType(filenameExtension: "lms") { types.append(lms) }
        if let las = UTType(filenameExtension: "las") { types.append(las) }
        // xLights package (zip-based, like .xsqz) — handled by SequencePackage.
        if let piz = UTType(filenameExtension: "piz") { types.append(piz) }
        return types
    }()
}

// Lightweight FileDocument used only to drive `.fileExporter` for the
// "Save Hints…" save panel. The actual write is done by the session
// directly to the picked URL — the document body is unused.
private struct MapHintsDocument: FileDocument {
    static let readableContentTypes: [UTType] = [.xml]
    init() {}
    init(configuration: ReadConfiguration) throws {}
    func fileWrapper(configuration: WriteConfiguration) throws -> FileWrapper {
        FileWrapper(regularFileWithContents: Data())
    }
}

// Recursive row view for the destination tree. Strands and submodels
// appear as children indented by `depth`; node-level children render
// at depth 2. Expansion is tracked in a Set<Int> keyed by nodeID so
// SwiftUI re-renders on toggle. Selection highlights only the
// tapped row regardless of depth.
private struct DestinationRowView: View {
    let row: XLImportMappingRow
    let depth: Int
    @Binding var selectedID: Int
    @Binding var expansion: Set<Int>

    var body: some View {
        let id = Int(row.nodeID)
        let isSelected = selectedID == id
        let hasChildren = !row.children.isEmpty
        let isExpanded = expansion.contains(id)
        VStack(alignment: .leading, spacing: 0) {
            HStack(spacing: 4) {
                if hasChildren {
                    Button {
                        if isExpanded { expansion.remove(id) }
                        else { expansion.insert(id) }
                    } label: {
                        Image(systemName: isExpanded ? "chevron.down" : "chevron.right")
                            .font(.caption)
                            .frame(width: 16)
                    }
                    .buttonStyle(.plain)
                } else {
                    Color.clear.frame(width: 20)
                }
                VStack(alignment: .leading, spacing: 2) {
                    Text(rowLabel)
                        .font(depth == 0 ? .body : .callout)
                    if !row.mapping.isEmpty {
                        Text("← \(row.mapping)")
                            .font(.caption)
                            .foregroundStyle(.tint)
                    }
                }
                Spacer()
                if row.isGroup {
                    Image(systemName: "rectangle.3.group")
                        .foregroundStyle(.secondary)
                }
            }
            .padding(.leading, CGFloat(8 + depth * 18))
            .padding(.trailing)
            .padding(.vertical, 6)
            .frame(maxWidth: .infinity, alignment: .leading)
            .background(isSelected ? Color.accentColor.opacity(0.2) : Color.clear)
            .contentShape(Rectangle())
            .onTapGesture {
                selectedID = isSelected ? 0 : id
            }
            Divider()
            if isExpanded {
                ForEach(row.children, id: \.nodeID) { child in
                    DestinationRowView(row: child,
                                        depth: depth + 1,
                                        selectedID: $selectedID,
                                        expansion: $expansion)
                }
            }
        }
    }

    private var rowLabel: String {
        if !row.node.isEmpty { return row.node }
        if !row.strand.isEmpty { return row.strand }
        return row.model
    }
}
