import SwiftUI

/// Visual node picker for Faces / States editing — the iPad
/// equivalent of tapping nodes on the model preview in desktop's
/// `ModelFaceDialog` / `ModelStateDialog`. Reuses
/// `SubmodelPreviewPane` (the same Metal node-picker that drives
/// the SubModel editor): the parent model is drawn fit-to-window,
/// the nodes belonging to the value being edited are painted
/// yellow, the rest dark grey. Tap toggles a node; two-finger
/// long-press + drag paints a rectangle of nodes. On commit the
/// selected node set is compressed back to a channel-range string
/// (`"1-5,9"`) with `SubModelRangeOps.compressNodes`, matching the
/// desktop `NodeUtils::CompressNodes` format stored in
/// `faceInfo` / `stateInfo`.
struct NodeRangePickerSheet: View {
    let document: XLSequenceDocument
    let modelName: String
    /// Human label for the part / state being edited (e.g.
    /// "Mouth-AI", "off") — shown in the title so the user knows
    /// which value they're painting.
    let label: String
    let initialRange: String
    let commit: (_ range: String) -> Void

    @State private var nodes: [Int]
    @Environment(\.dismiss) private var dismiss

    init(document: XLSequenceDocument,
         modelName: String,
         label: String,
         initialRange: String,
         commit: @escaping (_ range: String) -> Void) {
        self.document = document
        self.modelName = modelName
        self.label = label
        self.initialRange = initialRange
        self.commit = commit
        self._nodes = State(initialValue: SubModelRangeOps.expand(initialRange))
    }

    var body: some View {
        NavigationStack {
            VStack(spacing: 6) {
                SubmodelPreviewPane(
                    document: document,
                    parentModelName: modelName,
                    highlightedNodes: nodes,
                    onToggleNode: toggleNode,
                    onAddNodes: addNodes)
                    .background(Color.black)
                    .clipShape(RoundedRectangle(cornerRadius: 8))
                    .padding(.horizontal)

                Text("Tap a node to toggle it. Two-finger long-press + drag to add a rectangle. Pinch / drag / double-tap to navigate.")
                    .font(.caption2)
                    .foregroundStyle(.secondary)
                    .frame(maxWidth: .infinity, alignment: .leading)
                    .padding(.horizontal)

                HStack {
                    Text(rangeString.isEmpty ? "No nodes" : rangeString)
                        .font(.caption.monospaced())
                        .foregroundStyle(rangeString.isEmpty ? .tertiary : .primary)
                        .lineLimit(1)
                        .truncationMode(.middle)
                    Spacer()
                    Button(role: .destructive) {
                        nodes = []
                    } label: {
                        Label("Clear", systemImage: "xmark.circle")
                            .font(.caption)
                    }
                    .disabled(nodes.isEmpty)
                }
                .padding(.horizontal)
                .padding(.bottom, 6)
            }
            .navigationTitle("Pick Nodes — \(label)")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") {
                        commit(rangeString)
                        dismiss()
                    }
                }
            }
        }
        .presentationDetents([.large])
    }

    private var rangeString: String {
        SubModelRangeOps.compressNodes(nodes)
    }

    private func toggleNode(_ n: Int) {
        guard n >= 1 else { return }
        if let idx = nodes.firstIndex(of: n) {
            nodes.remove(at: idx)
        } else {
            nodes.append(n)
        }
    }

    private func addNodes(_ ns: [Int]) {
        guard !ns.isEmpty else { return }
        var set = Set(nodes)
        for n in ns where n >= 1 { set.insert(n) }
        nodes = Array(set).sorted()
    }
}

/// Identifiable wrapper so an attribute key can drive a
/// `.sheet(item:)` presentation of `NodeRangePickerSheet`.
struct NodePickerTarget: Identifiable {
    let key: String
    var id: String { key }
}

/// Classifies a face / state attribute key so the editor knows
/// whether its value is a paintable node-range string (gets the
/// visual node picker) or plain metadata / a file path (stays a
/// text field). Mirrors the desktop dialogs' key conventions
/// (`ModelFaceDialog` / `ModelStateDialog`):
///
///   - `Type`, `CustomColors`, `ImagePlacement` — config metadata.
///   - `*-Color` — RGB swatch for the matching part / state.
///   - `*-Name` — display label for a state row.
///   - matrix-face image keys (`Mouth-…-Eyes…`) — file paths.
///   - everything else (phoneme parts, `FaceOutline`, `sNNN`
///     state node lists) — node-range strings.
enum FaceStateAttr {
    static func isNodeRange(key: String) -> Bool {
        if key == "Type" || key == "CustomColors" || key == "ImagePlacement" {
            return false
        }
        if key.hasSuffix("-Color") || key.hasSuffix("-Name") {
            return false
        }
        // Matrix-face image assignment keys carry an eyes-state
        // segment and hold a file path, not a node range. These
        // stay on the attribute-map (image picker) path.
        if key.contains("-Eyes") {
            return false
        }
        return true
    }
}
