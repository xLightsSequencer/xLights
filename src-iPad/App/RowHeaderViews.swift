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
    /// B84: surfaced as "Breakdown Phrases" in the context menu. The
    /// outer view wires this to `SequencerViewModel.breakdownPhrases`;
    /// nil hides the menu entry. Guarded separately by
    /// `canBreakdownPhrases` so we don't flash an entry the user
    /// can't use.
    var canBreakdownPhrases: Bool = false
    var onBreakdownPhrases: (() -> Void)?
    /// B85: surfaced as "Breakdown Words" once a Words layer exists
    /// (i.e. Breakdown Phrases has already run).
    var canBreakdownWords: Bool = false
    var onBreakdownWords: (() -> Void)?
    /// B87: surfaced as "Remove Words / Phonemes" when the phrase
    /// layer has extra sub-layers to strip.
    var canRemoveWordsAndPhonemes: Bool = false
    var onRemoveWordsAndPhonemes: (() -> Void)?
    /// B76: surfaced as "Make Variable" on fixed-interval timing
    /// tracks. Nil hides the entry.
    var canMakeVariable: Bool = false
    var onMakeVariable: (() -> Void)?
    /// B80: subdivision callback. Fired with the chosen
    /// `SubdivisionMode.rawValue` so RowHeaderViews doesn't pull in
    /// the whole view-model enum by type.
    var onSubdivide: ((_ rawMode: Int) -> Void)?
    var canSubdivide: Bool = false
    /// B75: fires when the user picks "Export Timing Track…".
    var onExportTimingTrack: (() -> Void)?
    /// B78: fires when the user picks "Import Lyrics…".
    var onImportLyrics: (() -> Void)?
    /// B89: fires when the user picks "Auto-Label Marks…".
    var onAutoLabelMarks: (() -> Void)?
    /// B91: fires when the user picks "Halve Timing Marks" —
    /// splits every mark at its midpoint.
    var onHalveTimingMarks: (() -> Void)?

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
                    .hoverEffect(.highlight)   // B30
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
            // B63: glyph for timing-row type. Lyric breakdown layers
            // (Phrases / Words / Phonemes, surfaced via B84/B85) get
            // a distinct icon so users scanning a busy timing band
            // can tell lyric content from plain beat marks. Plain
            // timing tracks stay clean — the coloured dot already
            // signals the row type.
            if let iconName = Self.timingLayerIcon(layerName: row.timing?.layerName ?? "") {
                Image(systemName: iconName)
                    .font(.caption2)
                    .foregroundStyle(Color.white.opacity(0.8))
            }
            Text(row.displayName)
                .font(.caption)
                .foregroundStyle(.white)
                .lineLimit(1)
                .help(row.displayName)
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
        .hoverEffect(showCollapseOnHeading ? .highlight : .lift)   // B30
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
                if canBreakdownPhrases, let fire = onBreakdownPhrases {
                    Button { fire() } label: {
                        Label("Breakdown Phrases",
                               systemImage: "text.append")
                    }
                }
                if canBreakdownWords, let fire = onBreakdownWords {
                    Button { fire() } label: {
                        Label("Breakdown Words",
                               systemImage: "text.badge.plus")
                    }
                }
                if canRemoveWordsAndPhonemes, let fire = onRemoveWordsAndPhonemes {
                    Button(role: .destructive) { fire() } label: {
                        Label("Remove Words / Phonemes",
                               systemImage: "rectangle.stack.badge.minus")
                    }
                }
                if canMakeVariable, let fire = onMakeVariable {
                    Button { fire() } label: {
                        Label("Make Variable",
                               systemImage: "pencil.and.list.clipboard")
                    }
                }
                if canSubdivide, let fire = onSubdivide {
                    Menu {
                        Button("1/2") { fire(2) }
                        Button("1/3") { fire(3) }
                        Button("1/4") { fire(4) }
                        Button("1/6") { fire(6) }
                        Button("1/8") { fire(8) }
                        Divider()
                        Button("2×") { fire(-2) }
                        Button("4×") { fire(-4) }
                        Button("8×") { fire(-8) }
                    } label: {
                        Label("Generate Subdivided Timing Track…",
                               systemImage: "square.split.2x1")
                    }
                }
                if let fire = onExportTimingTrack {
                    Button { fire() } label: {
                        Label("Export Timing Track…",
                               systemImage: "square.and.arrow.up")
                    }
                }
                if let fire = onImportLyrics {
                    Button { fire() } label: {
                        Label("Import Lyrics…",
                               systemImage: "text.bubble")
                    }
                }
                if let fire = onAutoLabelMarks {
                    Button { fire() } label: {
                        Label("Auto-Label Marks…",
                               systemImage: "number")
                    }
                }
                if let fire = onHalveTimingMarks {
                    Button { fire() } label: {
                        Label("Halve Timing Marks",
                               systemImage: "square.split.1x2")
                    }
                }
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

    /// B63 — SF Symbol for a lyric-breakdown layer. Nil for plain
    /// timing tracks (no decoration; the coloured dot is enough).
    static func timingLayerIcon(layerName: String) -> String? {
        switch layerName.lowercased() {
        case "phrases":  return "text.bubble"
        case "words":    return "textformat.abc"
        case "phonemes": return "waveform.path"
        default:         return nil
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
    /// B2: fires when the user picks "Select All Effects in Row" from
    /// the row-heading long-press menu. Wired on the outer view to
    /// `SequencerViewModel.selectAllEffectsInRow(rowIndex:)`.
    var onSelectAllEffects: (() -> Void)?
    /// B52 select-every-effect-in-the-model (row + sub-layers +
    /// submodels + strands + nodes). Shown on model rows where it
    /// differs from `onSelectAllEffects` (which is row-scoped only).
    var onSelectAllEffectsInModel: (() -> Void)?
    /// B46 rename-layer callback + initial label. The outer view
    /// wires this to `SequencerViewModel.renameLayer(rowIndex:name:)`
    /// and then refreshes rows.
    var onRenameLayer: ((_ newName: String) -> Void)?
    /// B50 delete-all-effects-on-row + count (for the confirm alert).
    var effectCountOnRow: Int = 0
    var onDeleteAllEffectsOnRow: (() -> Void)?
    /// B51 toggle element render-disabled flag.
    var elementRenderDisabled: Bool = false
    var onToggleRenderDisabled: (() -> Void)?
    /// B53 / B54 row + model cut / copy. Paste is also exposed on
    /// the menu via `onPaste` (in addition to the global Cmd+V
    /// plumbed in B98) — fires `pasteAtRow` so a multi-layer
    /// clipboard auto-inserts layers in the destination, matching
    /// desktop PR #6363.
    var onCopyRow: (() -> Void)?
    var onCutRow: (() -> Void)?
    var onCopyModel: (() -> Void)?
    var onCutModel: (() -> Void)?
    var onPaste: (() -> Void)?
    var hasClipboard: Bool = false
    /// B49 export rendered-channel data for this row's model as a
    /// Falcon Player `.eseq` sub-sequence. The closure receives
    /// `true` when the caller should restrict the export to the
    /// active loop region, `false` for the whole sequence. The
    /// "selected range" entry only shows up when `hasLoopRegion`
    /// is true — on iPad, loop-region is how we indicate a time
    /// window (desktop uses a frame selection).
    var hasLoopRegion: Bool = false
    var onExportModelFSEQ: ((_ useLoopRegion: Bool) -> Void)?
    /// B55 — convert effects on the row's element to "Per Model"
    /// buffer styles. The closure decides scope (true = all layers
    /// of the model, false = just this row's layer); the outer view
    /// owns the menu copy.
    var onConvertToPerModel: ((_ allLayers: Bool) -> Void)?
    /// B56 — coalesce identical node / strand effects up the
    /// hierarchy. Only meaningful on ModelElement rows; outer view
    /// gates visibility.
    var onPromoteNodeEffects: (() -> Void)?
    /// B48 — delete every empty layer on this row's element.
    /// Shown only on the element's primary row (`layerIndex == 0`)
    /// and only when there's > 1 layer to begin with.
    var unusedLayerCount: Int = 0
    var onDeleteUnusedLayers: (() -> Void)?
    /// B47 — insert N empty layers in one shot. The outer view
    /// owns the count prompt; we just fire the closure with the
    /// chosen value.
    var onInsertMultipleLayersBelow: (() -> Void)?
    /// True when this row hosts the currently-selected effect.
    /// Drives a soft accent tint on the heading so the active row
    /// is recognisable at a glance — pairs with the height bump
    /// already applied by `SequencerGridV2View.modelHeaders`.
    var isSelected: Bool = false

    @State private var showDeleteLayerConfirm: Bool = false
    @State private var showDeleteAllEffectsConfirm: Bool = false
    @State private var showRenameLayer: Bool = false
    @State private var renameLayerText: String = ""

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
                .hoverEffect(.highlight)   // B30
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
                .hoverEffect(.highlight)   // B30
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
                    .help(row.displayName)
            } else if isNodeRow {
                Text(row.displayName)
                    .font(.caption)
                    .foregroundStyle(.secondary)
                    .lineLimit(1)
                    .help(row.displayName)
            } else if layerCount > 1 {
                Text("\(row.displayName) [\(layerCount)]")
                    .font(.caption)
                    .fontWeight(isSubmodelRow ? .regular : .medium)
                    .foregroundStyle(isSubmodelRow ? Color.secondary : Color.primary)
                    .lineLimit(1)
                    .help(row.displayName)
            } else {
                Text(row.displayName)
                    .font(.caption)
                    .fontWeight(isSubmodelRow ? .regular : .medium)
                    .foregroundStyle(isSubmodelRow ? Color.secondary : Color.primary)
                    .lineLimit(1)
                    .help(row.displayName)
            }
            Spacer(minLength: 0)
        }
        .padding(.horizontal, 6)
        .frame(maxWidth: .infinity, alignment: .leading)
        .frame(height: height)
        .background(row.id % 2 == 0
                    ? Color.black.opacity(0.25)
                    : Color.black.opacity(0.15))
        .background(isSelected ? Color.accentColor.opacity(0.22) : Color.clear)
        .overlay(alignment: .leading) {
            if isSelected {
                Rectangle()
                    .fill(Color.accentColor)
                    .frame(width: 3)
            }
        }
        // B66 — render-disabled (desktop calls it "muted") rows
        // show at 45 % opacity so the user can tell at a glance
        // which models won't contribute to the render pass.
        .opacity(elementRenderDisabled ? 0.45 : 1.0)
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
            if !row.effects.isEmpty, let fire = onSelectAllEffects {
                Button {
                    fire()
                } label: {
                    Label("Select All Effects in Row",
                           systemImage: "checkmark.rectangle.stack")
                }
            }
            if !isSubLayer && !isNodeRow,
               let fire = onSelectAllEffectsInModel {
                Button {
                    fire()
                } label: {
                    Label("Select All Effects in Model",
                           systemImage: "rectangle.stack.fill.badge.plus")
                }
            }
            if effectCountOnRow > 0, onDeleteAllEffectsOnRow != nil {
                Button(role: .destructive) {
                    showDeleteAllEffectsConfirm = true
                } label: {
                    Label("Delete All Effects on Row",
                           systemImage: "trash.slash")
                }
            }
            if effectCountOnRow > 0, let fire = onCopyRow {
                Button { fire() } label: {
                    Label("Copy Row", systemImage: "doc.on.doc")
                }
            }
            if effectCountOnRow > 0, let fire = onCutRow {
                Button(role: .destructive) { fire() } label: {
                    Label("Cut Row", systemImage: "scissors")
                }
            }
            if !isSubLayer && !isNodeRow, let fire = onCopyModel {
                Button { fire() } label: {
                    Label("Copy Model", systemImage: "square.stack.3d.up")
                }
            }
            if !isSubLayer && !isNodeRow, let fire = onCutModel {
                Button(role: .destructive) { fire() } label: {
                    Label("Cut Model",
                           systemImage: "square.stack.3d.up.trianglebadge.exclamationmark")
                }
            }
            if hasClipboard, let fire = onPaste {
                Button { fire() } label: {
                    Label("Paste", systemImage: "doc.on.clipboard")
                }
            }
            if !isSubLayer && !isNodeRow, let fire = onExportModelFSEQ {
                Button { fire(false) } label: {
                    Label("Export Model as FSEQ…",
                           systemImage: "square.and.arrow.up.on.square")
                }
                if hasLoopRegion {
                    Button { fire(true) } label: {
                        Label("Export Model (Loop Range) as FSEQ…",
                               systemImage: "arrow.up.doc")
                    }
                }
            }
            // B55 — model-scope variant on the model heading; layer-
            // scope variant on per-layer / submodel rows. Mirrors
            // desktop's two RowHeading entries.
            if let fire = onConvertToPerModel, !isNodeRow {
                if !isSubLayer {
                    Button { fire(true) } label: {
                        Label("Convert Effects to 'Per Model'",
                               systemImage: "rectangle.compress.vertical")
                    }
                } else {
                    Button { fire(false) } label: {
                        Label("Convert Effects on Layer to 'Per Model'",
                               systemImage: "rectangle.compress.vertical")
                    }
                }
            }
            // B56 — only the model heading itself; promotion walks
            // strands+nodes so it doesn't make sense on submodel /
            // node rows.
            if let fire = onPromoteNodeEffects, !isSubLayer, !isNodeRow {
                Button { fire() } label: {
                    Label("Promote Node Effects",
                           systemImage: "arrow.up.to.line.compact")
                }
            }
            if !isSubLayer && !isNodeRow, let fire = onToggleRenderDisabled {
                Button { fire() } label: {
                    Label(elementRenderDisabled ? "Enable Render" : "Disable Render",
                           systemImage: elementRenderDisabled ? "play.rectangle" : "stop.rectangle")
                }
            }
            if !row.effects.isEmpty || onToggleRenderDisabled != nil
                || onDeleteAllEffectsOnRow != nil {
                Divider()
            }
            if !isNodeRow {
                if onRenameLayer != nil {
                    Button {
                        renameLayerText = document.rowLayerName(at: Int32(row.id))
                        showRenameLayer = true
                    } label: { Label("Rename Layer", systemImage: "pencil") }
                }
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
                if let fire = onInsertMultipleLayersBelow {
                    Button { fire() } label: {
                        Label("Insert Multiple Layers Below…",
                               systemImage: "square.stack.3d.down.right")
                    }
                }
                if canDeleteLayer {
                    Button(role: .destructive) {
                        showDeleteLayerConfirm = true
                    } label: { Label("Delete Layer", systemImage: "trash") }
                }
                if !isSubLayer, unusedLayerCount > 0,
                   let fire = onDeleteUnusedLayers {
                    Button(role: .destructive) { fire() } label: {
                        Label("Delete \(unusedLayerCount) Unused Layer\(unusedLayerCount == 1 ? "" : "s")",
                               systemImage: "trash.square")
                    }
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
        .alert("Delete All Effects",
               isPresented: $showDeleteAllEffectsConfirm) {
            Button("Delete All", role: .destructive) {
                onDeleteAllEffectsOnRow?()
            }
            Button("Cancel", role: .cancel) {}
        } message: {
            Text("Delete all \(effectCountOnRow) effects on \(row.displayName)? Undo with ⌘Z.")
        }
        .alert("Rename Layer",
               isPresented: $showRenameLayer) {
            TextField("Name", text: $renameLayerText)
            Button("OK") {
                onRenameLayer?(renameLayerText)
            }
            Button("Cancel", role: .cancel) {}
        } message: {
            Text("Enter a layer name (used as the header label).")
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
