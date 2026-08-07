import SwiftUI
import UniformTypeIdentifiers

/// Layout ▸ Import From Another Show — the counterpart to desktop's
/// `ImportPreviewsModelsDialog` (Layout ▸ "Import Models From RGB
/// Effects"). Pick another show's `xlights_rgbeffects.xml`, tick the
/// models, groups and viewpoints to bring across, and they merge into
/// the preview currently being edited.
///
/// The tree mirrors desktop's grouping: a row per preview in the source
/// file, groups listed ahead of models, and the file's named viewpoints
/// in their own section.
struct ImportFromShowSheet: View {
    let viewModel: SequencerViewModel
    /// Preview everything lands in — desktop imports into the layout
    /// group being viewed rather than recreating the source's previews.
    let targetLayoutGroup: String
    let onFinished: (String) -> Void

    @Environment(\.dismiss) private var dismiss

    private struct Preview: Identifiable {
        let name: String
        let items: [Item]
        var id: String { name }
    }
    private struct Item: Identifiable, Hashable {
        let name: String
        let isGroup: Bool
        var id: String { "\(isGroup ? "g" : "m"):\(name)" }
    }
    private struct Viewpoint: Identifiable, Hashable {
        let name: String
        let is3D: Bool
        var id: String { name }
    }

    @State private var sourcePath: String = ""
    @State private var previews: [Preview] = []
    @State private var viewpoints: [Viewpoint] = []
    /// Checked item ids, keyed by preview name.
    @State private var checked: [String: Set<String>] = [:]
    @State private var checkedViewpoints: Set<String> = []
    @State private var includeEmptyGroups = false
    @State private var filterText = ""
    @State private var showingPicker = false
    @State private var resultMessage: String?

    var body: some View {
        NavigationStack {
            Group {
                if sourcePath.isEmpty {
                    ContentUnavailableView {
                        Label("Choose a Show", systemImage: "square.and.arrow.down.on.square")
                    } description: {
                        Text("Pick another show's xlights_rgbeffects.xml to import its models, groups and viewpoints.")
                    } actions: {
                        Button("Choose File…") { showingPicker = true }
                            .buttonStyle(.borderedProminent)
                    }
                } else if previews.isEmpty && viewpoints.isEmpty {
                    ContentUnavailableView("Nothing to Import",
                                           systemImage: "tray",
                                           description: Text("That file has no models, groups or viewpoints."))
                } else {
                    contentList
                }
            }
            .navigationTitle("Import From Show")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Import") { runImport() }
                        .disabled(totalChecked == 0)
                }
            }
        }
        .fileImporter(isPresented: $showingPicker,
                      allowedContentTypes: [.xml],
                      allowsMultipleSelection: false) { result in
            guard case .success(let urls) = result, let url = urls.first else { return }
            let needsStop = url.startAccessingSecurityScopedResource()
            defer { if needsStop { url.stopAccessingSecurityScopedResource() } }
            _ = XLSequenceDocument.obtainAccess(toPath: url.path, enforceWritable: false)
            sourcePath = url.path
            load()
        }
        .alert("Import", isPresented: Binding(
            get: { resultMessage != nil },
            set: { if !$0 { resultMessage = nil } }
        )) {
            Button("OK") {
                let msg = resultMessage ?? ""
                resultMessage = nil
                onFinished(msg)
                dismiss()
            }
        } message: {
            Text(resultMessage ?? "")
        }
    }

    private var contentList: some View {
        List {
            Section {
                HStack {
                    Image(systemName: "magnifyingglass").foregroundStyle(.secondary)
                    TextField("Filter", text: $filterText)
                        .textInputAutocapitalization(.never)
                        .autocorrectionDisabled()
                }
                Toggle("Include Empty Groups", isOn: $includeEmptyGroups)
                Text("Imported into “\(targetLayoutGroup)”. A name already in use is imported under a new name.")
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }

            ForEach(previews) { preview in
                let items = visibleItems(preview)
                if !items.isEmpty {
                    Section(preview.name) {
                        Button(allChecked(preview) ? "Deselect All" : "Select All") {
                            toggleAll(preview)
                        }
                        .font(.caption)
                        ForEach(items) { item in
                            row(title: item.name,
                                subtitle: item.isGroup ? "Group" : nil,
                                isOn: checked[preview.name]?.contains(item.id) == true) {
                                toggle(item, in: preview)
                            }
                        }
                    }
                }
            }

            let vps = visibleViewpoints
            if !vps.isEmpty {
                Section("Viewpoints") {
                    ForEach(vps) { vp in
                        row(title: vp.name,
                            subtitle: vp.is3D ? "3D" : "2D",
                            isOn: checkedViewpoints.contains(vp.name)) {
                            if checkedViewpoints.contains(vp.name) {
                                checkedViewpoints.remove(vp.name)
                            } else {
                                checkedViewpoints.insert(vp.name)
                            }
                        }
                    }
                }
            }
        }
    }

    private func row(title: String, subtitle: String?,
                     isOn: Bool, toggle: @escaping () -> Void) -> some View {
        Button(action: toggle) {
            HStack {
                Image(systemName: isOn ? "checkmark.circle.fill" : "circle")
                    .foregroundStyle(isOn ? Color.accentColor : .secondary)
                Text(title).foregroundStyle(.primary)
                if let subtitle {
                    Text(subtitle).font(.caption).foregroundStyle(.secondary)
                }
                Spacer()
            }
        }
        .buttonStyle(.plain)
    }

    // MARK: - Data

    private func load() {
        checked = [:]
        checkedViewpoints = []
        let contents = viewModel.document.importableContents(ofRGBEffectsFile: sourcePath)
        let rawPreviews = contents["previews"] as? [[String: Any]] ?? []
        previews = rawPreviews.map { p in
            let items = (p["items"] as? [[String: Any]] ?? []).map { i in
                Item(name: i["name"] as? String ?? "",
                     isGroup: (i["kind"] as? String) == "group")
            }
            return Preview(name: p["name"] as? String ?? "", items: items)
        }
        let rawVps = contents["viewpoints"] as? [[String: Any]] ?? []
        viewpoints = rawVps.map {
            Viewpoint(name: $0["name"] as? String ?? "",
                      is3D: ($0["is3D"] as? NSNumber)?.boolValue ?? false)
        }
    }

    private func matches(_ s: String) -> Bool {
        let q = filterText.trimmingCharacters(in: .whitespaces)
        if q.isEmpty { return true }
        return s.range(of: q, options: .caseInsensitive) != nil
    }

    private func visibleItems(_ p: Preview) -> [Item] {
        p.items.filter { matches($0.name) }
    }

    private var visibleViewpoints: [Viewpoint] {
        viewpoints.filter { matches($0.name) }
    }

    private var totalChecked: Int {
        checked.values.reduce(0) { $0 + $1.count } + checkedViewpoints.count
    }

    private func allChecked(_ p: Preview) -> Bool {
        let items = visibleItems(p)
        guard !items.isEmpty else { return false }
        let set = checked[p.name] ?? []
        return items.allSatisfy { set.contains($0.id) }
    }

    private func toggle(_ item: Item, in p: Preview) {
        var set = checked[p.name] ?? []
        if set.contains(item.id) { set.remove(item.id) } else { set.insert(item.id) }
        checked[p.name] = set
    }

    private func toggleAll(_ p: Preview) {
        let items = visibleItems(p)
        if allChecked(p) {
            var set = checked[p.name] ?? []
            for i in items { set.remove(i.id) }
            checked[p.name] = set
        } else {
            var set = checked[p.name] ?? []
            for i in items { set.insert(i.id) }
            checked[p.name] = set
        }
    }

    // MARK: - Import

    private func runImport() {
        // Bridge takes plain item names per preview; ids carry the
        // kind only so the two lists can't collide in the UI.
        var selection: [String: [String]] = [:]
        for p in previews {
            let ids = checked[p.name] ?? []
            let names = p.items.filter { ids.contains($0.id) }.map { $0.name }
            if !names.isEmpty { selection[p.name] = names }
        }
        let result = viewModel.document.importFromRGBEffectsFile(
            sourcePath,
            selection: selection,
            viewpointNames: Array(checkedViewpoints),
            intoLayoutGroup: targetLayoutGroup,
            includeEmptyGroups: includeEmptyGroups)

        let models = (result["models"] as? NSNumber)?.intValue ?? 0
        let groups = (result["groups"] as? NSNumber)?.intValue ?? 0
        let vps = (result["viewpoints"] as? NSNumber)?.intValue ?? 0
        let renamed = result["renamed"] as? [String] ?? []
        let skipped = result["skippedEmptyGroups"] as? [String] ?? []

        var parts: [String] = []
        parts.append("Imported \(models) model\(models == 1 ? "" : "s"), \(groups) group\(groups == 1 ? "" : "s"), \(vps) viewpoint\(vps == 1 ? "" : "s").")
        if !renamed.isEmpty {
            parts.append("Renamed to avoid collisions:\n" + renamed.joined(separator: "\n"))
        }
        if !skipped.isEmpty {
            parts.append("Skipped (no members present here): " + skipped.joined(separator: ", "))
        }
        resultMessage = parts.joined(separator: "\n\n")
    }
}
