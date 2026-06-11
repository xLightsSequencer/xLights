import SwiftUI
import UniformTypeIdentifiers

/// PRE-1 — browser for the persistent effect-preset library.
///
/// Presents the hierarchical group/preset tree backed by
/// `SequencerViewModel.presetTree` (which mirrors the on-disk
/// `xlights_effectpresets.json`). All mutations funnel through the
/// view model so the on-disk JSON + the in-memory snapshot stay in
/// sync and the row reload / undo plumbing runs in one place.
///
/// Capabilities: apply a preset to the current selection, save the
/// current selection as a new preset (optionally into a group), update
/// a preset from the selected effect, add a group, rename, delete,
/// reorder (move up/down + move to another group), search, and
/// import/export the library. Mirrors the desktop `EffectTreeDialog`
/// feature set at a touch-friendly altitude.
struct PresetBrowserSheet: View {
    @Bindable var viewModel: SequencerViewModel
    @Environment(\.dismiss) private var dismiss

    @State private var searchText: String = ""

    /// Mirrors desktop EffectTreeDialog's Relative / Using Layers radios.
    /// `.auto` seeds from the preset's `\tLAYER:` token (desktop's
    /// auto-detect); the other two force the mode.
    enum ApplyMode: String, CaseIterable, Identifiable {
        case auto = "Auto"
        case relative = "Relative"
        case usingLayers = "Using Layers"
        var id: String { rawValue }
    }
    @State private var applyMode: ApplyMode = .auto

    // Inline prompts. Each carries the path the action targets.
    @State private var renameTarget: String? = nil
    @State private var renameText: String = ""

    @State private var addGroupParent: String? = nil   // nil = sheet closed
    @State private var addGroupName: String = ""

    @State private var saveSelectionGroup: String? = nil
    @State private var saveSelectionName: String = ""

    @State private var moveTarget: String? = nil        // path being re-grouped

    @State private var updateTarget: String? = nil      // confirm overwrite

    @State private var showImporter = false
    @State private var showExporter = false
    @State private var exportDoc: PresetExportDocument? = nil

    /// Resolve the effective Using-Layers flag for a preset path given
    /// the current picker. `.auto` defers to the preset's `\tLAYER:`
    /// auto-detect, matching desktop.
    private func usingLayers(for path: String) -> Bool {
        switch applyMode {
        case .auto: return viewModel.presetUsesLayers(atPath: path)
        case .relative: return false
        case .usingLayers: return true
        }
    }

    private var hasSelection: Bool {
        viewModel.selectedEffect != nil || viewModel.selectedEffects.count > 0
    }

    /// Update Preset overwrites a single effect's blob, so it requires
    /// exactly one selected effect (matches the desktop single-effect gate).
    private var hasSingleSelection: Bool {
        viewModel.selectedEffect != nil && viewModel.selectedEffects.count <= 1
    }

    /// Tree filtered by the search box: a node survives when its own name
    /// matches, and groups also survive when any descendant matches (so
    /// the path to a hit stays visible). Empty query → full tree.
    private var visibleTree: [SequencerViewModel.PresetTreeItem] {
        filtered(viewModel.presetTree)
    }

    private var visibleBaseTree: [SequencerViewModel.PresetTreeItem] {
        filtered(viewModel.basePresetTree)
    }

    private func filtered(_ tree: [SequencerViewModel.PresetTreeItem]) -> [SequencerViewModel.PresetTreeItem] {
        let q = searchText.trimmingCharacters(in: .whitespacesAndNewlines).lowercased()
        guard !q.isEmpty else { return tree }
        // Paths of groups that should remain because a descendant matches.
        var keepGroup = Set<String>()
        for item in tree where !item.isGroup && item.name.lowercased().contains(q) {
            for ancestor in ancestorPaths(of: item.path) { keepGroup.insert(ancestor) }
        }
        for item in tree where item.isGroup && item.name.lowercased().contains(q) {
            keepGroup.insert(item.path)
            for ancestor in ancestorPaths(of: item.path) { keepGroup.insert(ancestor) }
        }
        return tree.filter { item in
            if item.isGroup { return keepGroup.contains(item.path) }
            return item.name.lowercased().contains(q) || keepGroup.contains(parentPath(of: item.path))
        }
    }

    var body: some View {
        NavigationStack {
            List {
                if viewModel.presetTree.isEmpty {
                    Section {
                        Text("No presets yet. Select one or more effects, then \"Save Selection as Preset\" below.")
                            .font(.callout)
                            .foregroundStyle(.secondary)
                    }
                }
                if viewModel.hasBasePresets {
                    Section("From Base") {
                        ForEach(visibleBaseTree) { item in
                            baseRow(for: item)
                        }
                    }
                }
                Section {
                    ForEach(visibleTree) { item in
                        row(for: item)
                    }
                } header: {
                    Picker("Apply Mode", selection: $applyMode) {
                        ForEach(ApplyMode.allCases) { mode in
                            Text(mode.rawValue).tag(mode)
                        }
                    }
                    .pickerStyle(.segmented)
                    .textCase(nil)
                }
                Section {
                    Button {
                        addGroupName = ""
                        addGroupParent = ""   // top-level group
                    } label: {
                        Label("New Group", systemImage: "folder.badge.plus")
                    }
                    Button {
                        saveSelectionName = ""
                        saveSelectionGroup = ""
                    } label: {
                        Label("Save Selection as Preset", systemImage: "square.and.arrow.down")
                    }
                    .disabled(!hasSelection)
                    Button {
                        showImporter = true
                    } label: {
                        Label("Import…", systemImage: "tray.and.arrow.down")
                    }
                    Button {
                        exportDoc = PresetExportDocument(data: makeExportData())
                        showExporter = true
                    } label: {
                        Label("Export…", systemImage: "tray.and.arrow.up")
                    }
                    .disabled(viewModel.presetTree.isEmpty)
                }
            }
            .searchable(text: $searchText, placement: .navigationBarDrawer(displayMode: .always),
                        prompt: "Search presets")
            .navigationTitle("Effect Presets")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") { dismiss() }
                }
            }
            // Rename prompt.
            .alert("Rename", isPresented: Binding(
                get: { renameTarget != nil },
                set: { if !$0 { renameTarget = nil } })) {
                TextField("Name", text: $renameText)
                Button("Rename") {
                    if let path = renameTarget {
                        _ = viewModel.renamePreset(atPath: path, to: renameText)
                    }
                    renameTarget = nil
                }
                .disabled(renameCollides)
                Button("Cancel", role: .cancel) { renameTarget = nil }
            } message: {
                if renameCollides {
                    Text("A sibling named \"\(trimmed(renameText))\" already exists.")
                }
            }
            // New-group prompt.
            .alert("New Group", isPresented: Binding(
                get: { addGroupParent != nil },
                set: { if !$0 { addGroupParent = nil } })) {
                TextField("Group name", text: $addGroupName)
                Button("Create") {
                    if let parent = addGroupParent {
                        _ = viewModel.addPresetGroup(named: addGroupName, inGroupPath: parent)
                    }
                    addGroupParent = nil
                }
                .disabled(addGroupCollides)
                Button("Cancel", role: .cancel) { addGroupParent = nil }
            } message: {
                if addGroupCollides {
                    Text("A group named \"\(trimmed(addGroupName))\" already exists here.")
                }
            }
            // Save-selection prompt.
            .alert("Save Preset", isPresented: Binding(
                get: { saveSelectionGroup != nil },
                set: { if !$0 { saveSelectionGroup = nil } })) {
                TextField("Preset name", text: $saveSelectionName)
                Button("Save") {
                    if let group = saveSelectionGroup {
                        _ = viewModel.saveSelectedEffectAsPreset(name: saveSelectionName,
                                                                 groupPath: group)
                    }
                    saveSelectionGroup = nil
                }
                .disabled(saveSelectionCollides)
                Button("Cancel", role: .cancel) { saveSelectionGroup = nil }
            } message: {
                if saveSelectionCollides {
                    Text("A preset named \"\(trimmed(saveSelectionName))\" already exists here.")
                } else {
                    Text("Captures the current effect selection so it can be re-applied later and on desktop.")
                }
            }
            // Move-to-group picker.
            .confirmationDialog("Move to Group", isPresented: Binding(
                get: { moveTarget != nil },
                set: { if !$0 { moveTarget = nil } }),
                titleVisibility: .visible) {
                ForEach(moveDestinations, id: \.self) { dest in
                    Button(dest.isEmpty ? "(Top Level)" : dest.replacingOccurrences(of: "\\", with: " / ")) {
                        if let path = moveTarget {
                            _ = viewModel.movePreset(fromPath: path, toGroupPath: dest)
                        }
                        moveTarget = nil
                    }
                }
                Button("Cancel", role: .cancel) { moveTarget = nil }
            }
            // Update-from-selection confirm (overwrites the preset blob).
            .alert("Update Preset", isPresented: Binding(
                get: { updateTarget != nil },
                set: { if !$0 { updateTarget = nil } })) {
                Button("Update", role: .destructive) {
                    if let path = updateTarget {
                        _ = viewModel.updatePreset(atPath: path)
                    }
                    updateTarget = nil
                }
                Button("Cancel", role: .cancel) { updateTarget = nil }
            } message: {
                Text("Overwrite this preset with the currently selected effect's settings?")
            }
            .fileImporter(isPresented: $showImporter,
                          allowedContentTypes: [.xml],
                          allowsMultipleSelection: false) { result in
                if case .success(let urls) = result, let url = urls.first {
                    let needsStop = url.startAccessingSecurityScopedResource()
                    defer { if needsStop { url.stopAccessingSecurityScopedResource() } }
                    _ = viewModel.importPresets(fromPath: url.path)
                }
            }
            .fileExporter(isPresented: $showExporter,
                          document: exportDoc,
                          contentType: .json,
                          defaultFilename: "xlights_effectpresets") { _ in
                exportDoc = nil
            }
        }
    }

    // MARK: - Collision validation

    private func trimmed(_ s: String) -> String {
        s.trimmingCharacters(in: .whitespacesAndNewlines)
    }

    /// Names already used by direct children of `groupPath` (empty = root),
    /// excluding `exclude` (the item being renamed in place).
    private func siblingNames(in groupPath: String, excluding exclude: String? = nil) -> Set<String> {
        var out = Set<String>()
        for item in viewModel.presetTree where item.path != exclude
            && parentPath(of: item.path) == groupPath {
            out.insert(item.name.lowercased())
        }
        return out
    }

    private var renameCollides: Bool {
        guard let path = renameTarget else { return false }
        let name = trimmed(renameText).lowercased()
        if name.isEmpty || name == path.split(separator: "\\").last.map({ String($0).lowercased() }) {
            return false
        }
        return siblingNames(in: parentPath(of: path), excluding: path).contains(name)
    }

    private var addGroupCollides: Bool {
        guard let parent = addGroupParent else { return false }
        let name = trimmed(addGroupName).lowercased()
        return !name.isEmpty && siblingNames(in: parent).contains(name)
    }

    private var saveSelectionCollides: Bool {
        guard let group = saveSelectionGroup else { return false }
        let name = trimmed(saveSelectionName).lowercased()
        return !name.isEmpty && siblingNames(in: group).contains(name)
    }

    // MARK: - Reorder helpers

    /// All group paths a preset/group may move into, excluding its own
    /// current parent and (for groups) itself or any descendant.
    private var moveDestinations: [String] {
        guard let path = moveTarget else { return [] }
        let current = parentPath(of: path)
        let movingGroup = viewModel.presetTree.first { $0.path == path }?.isGroup ?? false
        return viewModel.presetGroupPaths.filter { dest in
            if dest == current { return false }
            if movingGroup {
                if dest == path { return false }
                if dest.hasPrefix(path + "\\") { return false }
            }
            return true
        }
    }

    /// Sibling items (same parent group, same kind ordering preserved)
    /// in tree order, for move up/down.
    private func siblings(of path: String) -> [SequencerViewModel.PresetTreeItem] {
        let parent = parentPath(of: path)
        return viewModel.presetTree.filter { parentPath(of: $0.path) == parent }
    }

    private func moveUp(_ item: SequencerViewModel.PresetTreeItem) {
        let sibs = siblings(of: item.path)
        guard let idx = sibs.firstIndex(where: { $0.path == item.path }), idx > 0 else { return }
        _ = viewModel.movePreset(fromPath: item.path,
                                 toGroupPath: parentPath(of: item.path),
                                 toIndex: idx - 1)
    }

    private func moveDown(_ item: SequencerViewModel.PresetTreeItem) {
        let sibs = siblings(of: item.path)
        guard let idx = sibs.firstIndex(where: { $0.path == item.path }),
              idx < sibs.count - 1 else { return }
        _ = viewModel.movePreset(fromPath: item.path,
                                 toGroupPath: parentPath(of: item.path),
                                 toIndex: idx + 1)
    }

    private func isFirstSibling(_ item: SequencerViewModel.PresetTreeItem) -> Bool {
        siblings(of: item.path).first?.path == item.path
    }

    private func isLastSibling(_ item: SequencerViewModel.PresetTreeItem) -> Bool {
        siblings(of: item.path).last?.path == item.path
    }

    // MARK: - Path utilities

    private func parentPath(of path: String) -> String {
        var parts = path.split(separator: "\\", omittingEmptySubsequences: false)
        guard parts.count > 1 else { return "" }
        parts.removeLast()
        return parts.joined(separator: "\\")
    }

    private func ancestorPaths(of path: String) -> [String] {
        var parts = path.split(separator: "\\", omittingEmptySubsequences: false).map(String.init)
        var out: [String] = []
        while parts.count > 1 {
            parts.removeLast()
            out.append(parts.joined(separator: "\\"))
        }
        return out
    }

    /// Serialize the library to a temp JSON file via the bridge, then
    /// read it back as bytes for the file exporter. Runs on the main
    /// actor (the view body) so the view-model call is legal.
    private func makeExportData() -> Data {
        let tmp = FileManager.default.temporaryDirectory
            .appendingPathComponent("xlights_effectpresets_export.json")
        if viewModel.exportPresets(toPath: tmp.path),
           let d = try? Data(contentsOf: tmp) {
            return d
        }
        return Data("{}".utf8)
    }

    private func baseUsingLayers(for path: String) -> Bool {
        switch applyMode {
        case .auto: return viewModel.basePresetUsesLayers(atPath: path)
        case .relative: return false
        case .usingLayers: return true
        }
    }

    /// Read-only row for a "From Base" preset (apply-only — no rename,
    /// delete, save, or reorder; desktop handles base save-back, which
    /// is intentionally out of scope on iPad).
    @ViewBuilder
    private func baseRow(for item: SequencerViewModel.PresetTreeItem) -> some View {
        HStack(spacing: 8) {
            if item.depth > 0 {
                Spacer().frame(width: CGFloat(item.depth) * 16)
            }
            Image(systemName: item.isGroup ? "folder" : "wand.and.stars")
                .foregroundStyle(item.isGroup ? Color.secondary : Color.accentColor)
            VStack(alignment: .leading, spacing: 2) {
                Text(item.name).fontWeight(.semibold)
                if !item.isGroup {
                    Text("\(item.layerCount) layer\(item.layerCount == 1 ? "" : "s"), \(item.durationMS) ms")
                        .font(.caption2)
                        .foregroundStyle(.secondary)
                }
            }
            Spacer()
            if !item.isGroup {
                Button("Apply") {
                    _ = viewModel.applyBasePreset(atPath: item.path,
                                                  usingLayers: baseUsingLayers(for: item.path))
                    dismiss()
                }
                .buttonStyle(.borderless)
                .disabled(!hasSelection)
            }
        }
    }

    @ViewBuilder
    private func row(for item: SequencerViewModel.PresetTreeItem) -> some View {
        HStack(spacing: 8) {
            // Indent by group depth so nesting reads clearly.
            if item.depth > 0 {
                Spacer().frame(width: CGFloat(item.depth) * 16)
            }
            Image(systemName: item.isGroup ? "folder" : "wand.and.stars")
                .foregroundStyle(item.isGroup ? Color.secondary : Color.accentColor)
            VStack(alignment: .leading, spacing: 2) {
                Text(item.name)
                if !item.isGroup {
                    Text("\(item.layerCount) layer\(item.layerCount == 1 ? "" : "s"), \(item.durationMS) ms")
                        .font(.caption2)
                        .foregroundStyle(.secondary)
                }
            }
            Spacer()
            if !item.isGroup {
                Button("Apply") {
                    _ = viewModel.applyPreset(atPath: item.path,
                                              usingLayers: usingLayers(for: item.path))
                    dismiss()
                }
                .buttonStyle(.borderless)
                .disabled(!hasSelection)
            }
        }
        .contextMenu {
            if !item.isGroup {
                Button {
                    _ = viewModel.applyPreset(atPath: item.path,
                                              usingLayers: usingLayers(for: item.path))
                    dismiss()
                } label: { Label("Apply to Selection", systemImage: "checkmark.circle") }
                .disabled(!hasSelection)
                Button {
                    updateTarget = item.path
                } label: { Label("Update from Selected Effect", systemImage: "arrow.triangle.2.circlepath") }
                .disabled(!hasSingleSelection)
            } else {
                Button {
                    addGroupName = ""
                    addGroupParent = item.path
                } label: { Label("New Subgroup", systemImage: "folder.badge.plus") }
                Button {
                    saveSelectionName = ""
                    saveSelectionGroup = item.path
                } label: { Label("Save Selection Here", systemImage: "square.and.arrow.down") }
                .disabled(!hasSelection)
            }
            Divider()
            Button {
                moveUp(item)
            } label: { Label("Move Up", systemImage: "arrow.up") }
            .disabled(isFirstSibling(item))
            Button {
                moveDown(item)
            } label: { Label("Move Down", systemImage: "arrow.down") }
            .disabled(isLastSibling(item))
            Button {
                moveTarget = item.path
            } label: { Label("Move to Group…", systemImage: "folder") }
            .disabled(moveDestinations.isEmpty)
            Divider()
            Button {
                renameText = item.name
                renameTarget = item.path
            } label: { Label("Rename", systemImage: "pencil") }
            Button(role: .destructive) {
                _ = viewModel.deletePreset(atPath: item.path)
            } label: { Label("Delete", systemImage: "trash") }
        }
    }
}

/// FileDocument wrapper so SwiftUI's `.fileExporter` can write the
/// preset library JSON. The data is serialized on the main actor by
/// the browser sheet (via the bridge → core manager) and handed in
/// here as bytes, keeping this type actor-agnostic.
struct PresetExportDocument: FileDocument {
    static var readableContentTypes: [UTType] { [.json] }

    private let data: Data

    init(data: Data) {
        self.data = data
    }

    init(configuration: ReadConfiguration) throws {
        data = configuration.file.regularFileContents ?? Data()
    }

    func fileWrapper(configuration: WriteConfiguration) throws -> FileWrapper {
        FileWrapper(regularFileWithContents: data)
    }
}
