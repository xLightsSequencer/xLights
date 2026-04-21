import SwiftUI

/// Timing-row header (row 2 left). Renders a colored background, an
/// active indicator circle (filled when active, outlined when not),
/// and the timing element's display name. Tapping the row toggles
/// `TimingElement::GetActive/SetActive` via the bridge. Long-press
/// brings up the rename / delete menu — iPad equivalent of desktop's
/// `RowHeading` right-click on a timing row.
struct TimingRowHeader: View {
    let row: SequencerViewModel.RowInfo
    let height: CGFloat
    let document: XLSequenceDocument
    let onRowsChanged: () -> Void

    // Active state is carried on `row.timing?.isActive` so a toggle
    // here flips the struct equality and re-runs the grid body —
    // otherwise the model-effects grid's vertical timing lines stay
    // stale until the next scroll/zoom.
    private var isActive: Bool { row.timing?.isActive ?? false }

    @State private var showRename: Bool = false
    @State private var showDelete: Bool = false
    @State private var renameText: String = ""

    // Desktop timing color palette (Timing1..Timing5): cyan, red, green, blue, yellow.
    // Darkened so white text remains readable on top.
    private static let timingPalette: [Color] = [
        Color(red: 0.00, green: 0.45, blue: 0.55), // cyan
        Color(red: 0.60, green: 0.10, blue: 0.10), // red
        Color(red: 0.10, green: 0.45, blue: 0.15), // green
        Color(red: 0.15, green: 0.25, blue: 0.60), // blue
        Color(red: 0.55, green: 0.45, blue: 0.05)  // yellow
    ]

    private var backgroundColor: Color {
        let idx = row.timing?.colorIndex ?? 0
        return Self.timingPalette[((idx % Self.timingPalette.count) + Self.timingPalette.count) % Self.timingPalette.count]
    }

    var body: some View {
        let layerCount = row.layerIndex == 0
            ? Int(document.rowLayerCount(at: Int32(row.id)))
            : 0
        let collapsed = row.layerIndex == 0
            && document.rowIsElementCollapsed(at: Int32(row.id))
        let showCollapseOnHeading = layerCount > 1

        HStack(spacing: 6) {
            if row.layerIndex == 0 {
                Circle()
                    .fill(isActive ? Color.white : Color.clear)
                    .overlay(
                        Circle().strokeBorder(Color.white.opacity(0.8), lineWidth: 1)
                    )
                    .frame(width: 14, height: 14)
                    .contentShape(Rectangle())
                    .onTapGesture {
                        document.setTimingRowActive(!isActive, at: Int32(row.id))
                        onRowsChanged()
                    }
            } else {
                Spacer().frame(width: 14, height: 14)
            }
            if showCollapseOnHeading {
                Image(systemName: collapsed ? "chevron.right" : "chevron.down")
                    .font(.caption2)
                    .foregroundStyle(Color.white.opacity(0.8))
                    .frame(width: 10)
            }
            Text(row.displayName)
                .font(.caption)
                .foregroundStyle(.white)
                .lineLimit(1)
            Spacer(minLength: 0)
        }
        .padding(.horizontal, 6)
        .frame(maxWidth: .infinity, alignment: .leading)
        .frame(height: height)
        .background(backgroundColor)
        // Heading area (everything right of the dot) toggles the
        // expand/collapse on the element when the timing track has
        // more than one layer — iPad equivalent of desktop's heading
        // double-click. The dot itself owns its own tap above and
        // isn't included because its `.contentShape` carves out its
        // own hit area.
        .contentShape(Rectangle())
        .onTapGesture {
            if showCollapseOnHeading {
                document.toggleElementCollapsed(at: Int32(row.id))
                onRowsChanged()
            }
        }
        .contextMenu {
            if row.layerIndex == 0 {
                Button {
                    renameText = row.timing?.elementName ?? row.displayName
                    showRename = true
                } label: { Label("Rename Timing Track", systemImage: "pencil") }
                Button(role: .destructive) {
                    showDelete = true
                } label: { Label("Delete Timing Track", systemImage: "trash") }
            }
        }
        .alert("Rename Timing Track", isPresented: $showRename) {
            TextField("Name", text: $renameText)
            Button("OK") {
                let trimmed = renameText.trimmingCharacters(in: .whitespacesAndNewlines)
                guard !trimmed.isEmpty else { return }
                if document.renameTimingTrack(at: Int32(row.id), newName: trimmed) {
                    onRowsChanged()
                }
            }
            Button("Cancel", role: .cancel) {}
        } message: {
            Text("Enter a new name for the timing track.")
        }
        .alert("Delete Timing Track",
               isPresented: $showDelete) {
            Button("Delete", role: .destructive) {
                if document.deleteTimingTrack(at: Int32(row.id)) {
                    onRowsChanged()
                }
            }
            Button("Cancel", role: .cancel) {}
        } message: {
            Text("Delete \(row.timing?.elementName ?? row.displayName)? This also removes its layers and any references from effect settings.")
        }
    }
}

/// Model-row header (row 3 left). Shows the element display name (or
/// indented "Layer N" label for nested layer rows), a collapse toggle
/// when the element has more than one layer, a group icon when the
/// element is a ModelGroup, and a submodel/strand/node disclosure
/// chevron when the element has sub-rows that can be expanded.
///
/// Gestures:
/// - Single tap: select the row's model for the model-preview pane.
/// - Double tap: toggle submodel visibility on a plain model row, or
///   node visibility on a strand row. Matches desktop's
///   `RowHeading::leftDoubleClick → ToggleExpand` muscle memory.
/// - Tap the chevron itself: same toggle as double-tap, single-tap
///   so discoverable users don't have to know the double-tap shortcut.
/// - Tap the layer ±: hide or show this row's sub-layers.
/// - Long-press: context menu with Insert Layer Above/Below, Delete
///   Layer, and Show/Hide Strands/Nodes — iPad equivalent of
///   desktop's right-click row-heading menu.
struct ModelRowHeader: View {
    let row: SequencerViewModel.RowInfo
    let height: CGFloat
    let document: XLSequenceDocument
    let onSelect: () -> Void
    let onRowsChanged: () -> Void

    @State private var showDeleteLayerConfirm: Bool = false

    var body: some View {
        let isSubLayer = row.layerIndex > 0
        let isSubmodelRow = row.isSubmodel
        let isStrandRow = row.strandIndex >= 0
        let isNodeRow = row.nodeIndex >= 0
        let isGroup = !isSubLayer && !isSubmodelRow
            && document.rowIsModelGroup(at: Int32(row.id))
        let layerCount = isNodeRow ? 0 : Int(document.rowLayerCount(at: Int32(row.id)))
        let collapsed = !isSubLayer && !isNodeRow
            && document.rowIsElementCollapsed(at: Int32(row.id))
        let showLayerToggle = !isSubLayer && !isNodeRow && layerCount > 1

        // Submodel/node disclosure — show only on the element's primary
        // row (first layer, not a sub-layer) and only when there's
        // something to disclose.
        let canToggleSubmodels = !isSubLayer && !isNodeRow
            && document.rowHasSubmodels(at: Int32(row.id))
        let canToggleNodes = !isSubLayer && isStrandRow
            && document.rowHasNodes(at: Int32(row.id))
        let showsChildren = canToggleNodes
            ? document.rowShowsNodes(at: Int32(row.id))
            : document.rowShowsSubmodels(at: Int32(row.id))
        let hasDisclosure = canToggleSubmodels || canToggleNodes

        let indent = CGFloat(row.nestDepth) * 10
        let canDeleteLayer = !isNodeRow && layerCount > 1

        HStack(spacing: 4) {
            if indent > 0 { Spacer().frame(width: indent) }

            if showLayerToggle {
                Button {
                    document.toggleElementCollapsed(at: Int32(row.id))
                    onRowsChanged()
                } label: {
                    Image(systemName: collapsed ? "plus.square" : "minus.square")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
                .buttonStyle(.plain)
            } else {
                Spacer().frame(width: 14)
            }

            if hasDisclosure {
                Button {
                    toggleDisclosure(kind: canToggleNodes ? .nodes : .submodels)
                } label: {
                    Image(systemName: showsChildren ? "chevron.down" : "chevron.right")
                        .font(.caption2)
                        .foregroundStyle(.secondary)
                }
                .buttonStyle(.plain)
                .frame(width: 12)
            }

            if isGroup {
                Image(systemName: "folder")
                    .font(.caption2)
                    .foregroundStyle(.secondary)
            }

            if isSubLayer {
                Text("[\(row.layerIndex + 1)]")
                    .font(.caption)
                    .foregroundStyle(.secondary)
                    .lineLimit(1)
            } else if isNodeRow {
                Text(row.displayName)
                    .font(.caption)
                    .foregroundStyle(.secondary)
                    .lineLimit(1)
            } else if layerCount > 1 {
                Text("\(row.displayName) [\(layerCount)]")
                    .font(.caption)
                    .fontWeight(isSubmodelRow ? .regular : .medium)
                    .foregroundStyle(isSubmodelRow ? Color.secondary : Color.primary)
                    .lineLimit(1)
            } else {
                Text(row.displayName)
                    .font(.caption)
                    .fontWeight(isSubmodelRow ? .regular : .medium)
                    .foregroundStyle(isSubmodelRow ? Color.secondary : Color.primary)
                    .lineLimit(1)
            }
            Spacer(minLength: 0)
        }
        .padding(.horizontal, 6)
        .frame(maxWidth: .infinity, alignment: .leading)
        .frame(height: height)
        .background(row.id % 2 == 0
                    ? Color.black.opacity(0.25)
                    : Color.black.opacity(0.15))
        .contentShape(Rectangle())
        // Double-tap comes first so SwiftUI's recognizer chain treats
        // it as the primary gesture on rows that have something to
        // expand; single-tap-to-select stays the default fallback.
        .modifier(ExpandableRowTapModifier(
            hasDisclosure: hasDisclosure,
            onSelect: onSelect,
            onDoubleTap: {
                toggleDisclosure(kind: canToggleNodes ? .nodes : .submodels)
            }
        ))
        .contextMenu {
            if !isNodeRow {
                Button {
                    if document.insertEffectLayerAbove(at: Int32(row.id)) {
                        onRowsChanged()
                    }
                } label: { Label("Insert Layer Above", systemImage: "square.3.layers.3d.top.filled") }
                Button {
                    if document.insertEffectLayerBelow(at: Int32(row.id)) {
                        onRowsChanged()
                    }
                } label: { Label("Insert Layer Below", systemImage: "square.3.layers.3d.bottom.filled") }
                if canDeleteLayer {
                    Button(role: .destructive) {
                        showDeleteLayerConfirm = true
                    } label: { Label("Delete Layer", systemImage: "trash") }
                }
            }
            if canToggleSubmodels {
                Divider()
                Button {
                    document.toggleRowShowSubmodels(at: Int32(row.id))
                    onRowsChanged()
                } label: {
                    Label(showsChildren ? "Hide Strands/Submodels" : "Show Strands/Submodels",
                          systemImage: showsChildren ? "eye.slash" : "eye")
                }
            }
            if canToggleNodes {
                Divider()
                Button {
                    document.toggleRowShowNodes(at: Int32(row.id))
                    onRowsChanged()
                } label: {
                    Label(showsChildren ? "Hide Nodes" : "Show Nodes",
                          systemImage: showsChildren ? "eye.slash" : "eye")
                }
            }
        }
        .alert("Delete Layer",
               isPresented: $showDeleteLayerConfirm) {
            Button("Delete", role: .destructive) {
                if document.removeEffectLayer(at: Int32(row.id)) {
                    onRowsChanged()
                }
            }
            Button("Cancel", role: .cancel) {}
        } message: {
            Text("Delete layer \(row.layerIndex + 1) of \(row.displayName)? All effects on this layer will be lost.")
        }
    }

    private enum DisclosureKind { case submodels, nodes }
    private func toggleDisclosure(kind: DisclosureKind) {
        switch kind {
        case .submodels:
            document.toggleRowShowSubmodels(at: Int32(row.id))
        case .nodes:
            document.toggleRowShowNodes(at: Int32(row.id))
        }
        onRowsChanged()
    }
}

/// Wires the right gesture combination based on whether the row has a
/// disclosure toggle: rows that can expand get single-tap-select +
/// double-tap-expand (and pay the SwiftUI ~0.25 s single-tap wait);
/// rows that can't stay on a plain single-tap so selection is snappy.
private struct ExpandableRowTapModifier: ViewModifier {
    let hasDisclosure: Bool
    let onSelect: () -> Void
    let onDoubleTap: () -> Void

    func body(content: Content) -> some View {
        if hasDisclosure {
            content
                .onTapGesture(count: 2) { onDoubleTap() }
                .onTapGesture(count: 1) { onSelect() }
        } else {
            content.onTapGesture { onSelect() }
        }
    }
}
