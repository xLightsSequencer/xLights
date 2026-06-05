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
/// current selection as a new preset (optionally into a group), add a
/// group, rename, delete, and import/export the library. Mirrors the
/// desktop `EffectTreeDialog` feature set at a touch-friendly altitude.
struct PresetBrowserSheet: View {
    @Bindable var viewModel: SequencerViewModel
    @Environment(\.dismiss) private var dismiss

    // Inline prompts. Each carries the path the action targets.
    @State private var renameTarget: String? = nil
    @State private var renameText: String = ""

    @State private var addGroupParent: String? = nil   // nil = sheet closed
    @State private var addGroupName: String = ""

    @State private var saveSelectionGroup: String? = nil
    @State private var saveSelectionName: String = ""

    @State private var showImporter = false
    @State private var showExporter = false
    @State private var exportDoc: PresetExportDocument? = nil

    private var hasSelection: Bool {
        viewModel.selectedEffect != nil || viewModel.selectedEffects.count > 0
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
                Section {
                    ForEach(viewModel.presetTree) { item in
                        row(for: item)
                    }
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
                Button("Cancel", role: .cancel) { renameTarget = nil }
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
                Button("Cancel", role: .cancel) { addGroupParent = nil }
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
                Button("Cancel", role: .cancel) { saveSelectionGroup = nil }
            } message: {
                Text("Captures the current effect selection so it can be re-applied later and on desktop.")
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
                    _ = viewModel.applyPreset(atPath: item.path)
                    dismiss()
                }
                .buttonStyle(.borderless)
                .disabled(!hasSelection)
            }
        }
        .contextMenu {
            if !item.isGroup {
                Button {
                    _ = viewModel.applyPreset(atPath: item.path)
                    dismiss()
                } label: { Label("Apply to Selection", systemImage: "checkmark.circle") }
                .disabled(!hasSelection)
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
