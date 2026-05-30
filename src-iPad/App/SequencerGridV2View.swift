import SwiftUI
import UniformTypeIdentifiers

/// Dynamic UTType for `.xtiming` timing-track files. Declared here
/// (not in Info.plist) because the import / export UI needs a
/// content type but the iPad doesn't yet own the file type. Falls
/// back to `.xml` so the system picker never no-ops.
let kXTimingFileType: UTType = UTType(filenameExtension: "xtiming") ?? .xml

/// B49 — Falcon Player sub-sequence (`.eseq`). We ship v2
/// zstd-compressed files. Falls back to a generic binary UTI so
/// the exporter still presents a picker on devices that don't
/// know the file type.
let kESEQFileType: UTType = UTType(filenameExtension: "eseq") ?? .data

/// File document wrapper for the Save / Export timing-track flow.
/// Holds the bytes already-written to a temp path so SwiftUI's
/// `.fileExporter` can copy them to the user's destination.
struct XTimingExportDoc: FileDocument {
    static var readableContentTypes: [UTType] { [kXTimingFileType] }
    static var writableContentTypes: [UTType] { [kXTimingFileType] }
    let sourcePath: String
    init(sourcePath: String) { self.sourcePath = sourcePath }
    init(configuration: ReadConfiguration) throws { sourcePath = "" }
    func fileWrapper(configuration: WriteConfiguration) throws -> FileWrapper {
        if !sourcePath.isEmpty,
           let data = try? Data(contentsOf: URL(fileURLWithPath: sourcePath)) {
            return FileWrapper(regularFileWithContents: data)
        }
        return FileWrapper(regularFileWithContents: Data())
    }
}

/// B100 paste-replace confirmation wrapper. Same extraction
/// pattern as the other alert modifiers — keeps the main body
/// expression under SwiftUI's type-check budget. Anchors on the
/// existing effect's (row, idx); the paste drops at that
/// effect's `startMS` after the user confirms the overwrite.
private struct PasteReplaceAlert: ViewModifier {
    @Binding var target: EditDescriptionTarget?
    let viewModel: SequencerViewModel
    func body(content: Content) -> some View {
        content.alert("Replace Effect?",
                      isPresented: Binding(
                        get: { target != nil },
                        set: { if !$0 { target = nil } }
                      ),
                      presenting: target) { tgt in
            Button("Replace", role: .destructive) {
                guard tgt.rowIndex >= 0, tgt.rowIndex < viewModel.rows.count,
                      tgt.effectIndex >= 0,
                      tgt.effectIndex < viewModel.rows[tgt.rowIndex].effects.count else {
                    target = nil
                    return
                }
                let startMS = viewModel.rows[tgt.rowIndex]
                    .effects[tgt.effectIndex].startTimeMS
                viewModel.pasteEffectReplacingOverlaps(
                    rowIndex: tgt.rowIndex, startMS: startMS)
                target = nil
            }
            Button("Cancel", role: .cancel) { target = nil }
        } message: { _ in
            Text("Pasting here would overwrite at least one existing effect. Replace it?")
        }
    }
}

/// Long-press on a transition diamond opens a picker for the
/// transition type. `isIn` selects which `T_CHOICE_*_Transition_Type`
/// key the chosen value writes to.
struct TransitionMenuTarget: Identifiable {
    let rowIndex: Int
    let effectIndex: Int
    let isIn: Bool
    var id: String { "\(rowIndex)-\(effectIndex)-\(isIn ? "in" : "out")" }
}

/// Transition-type picker confirmation dialog. Pulled out as its own
/// `ViewModifier` (same reason as the other alert wrappers in this
/// file) so the main body's modifier chain stays under SwiftUI's
/// type-check budget.
private struct TransitionPickerDialog: ViewModifier {
    @Binding var target: TransitionMenuTarget?
    let viewModel: SequencerViewModel
    func body(content: Content) -> some View {
        content.confirmationDialog(
            "Transition",
            isPresented: Binding(
                get: { target != nil },
                set: { if !$0 { target = nil } }
            ),
            presenting: target
        ) { tgt in
            let typeKey = tgt.isIn ? "T_CHOICE_In_Transition_Type"
                                   : "T_CHOICE_Out_Transition_Type"
            ForEach(kTransitionTypes, id: \.self) { (t: String) in
                Button(t) {
                    viewModel.selectEffect(rowIndex: tgt.rowIndex,
                                            effectIndex: tgt.effectIndex)
                    viewModel.setSettingValue(t, forKey: typeKey,
                                               suppressIfDefault: "Fade")
                }
            }
            Button("Cancel", role: .cancel) {}
        } message: { tgt in
            let typeKey = tgt.isIn ? "T_CHOICE_In_Transition_Type"
                                   : "T_CHOICE_Out_Transition_Type"
            let current = viewModel.document.effectSettingValue(
                forKey: typeKey,
                inRow: Int32(tgt.rowIndex),
                at: Int32(tgt.effectIndex))
            let display = current.isEmpty ? "Fade" : current
            Text(tgt.isIn
                 ? "In transition (current: \(display))"
                 : "Out transition (current: \(display))")
        }
    }
}

/// B20 edit-description alert wrapper. Same extraction pattern
/// as `InsertLayersAlert` — keeps the main body expression under
/// SwiftUI's type-check budget. Target is encoded as a simple
/// `(row, idx)` pair so the modifier doesn't reach into the
/// private `ContextMenuTarget` declared inside `SequencerGridV2View`.
struct EditDescriptionTarget: Equatable {
    let rowIndex: Int
    let effectIndex: Int
}

private struct EditDescriptionAlert: ViewModifier {
    @Binding var target: EditDescriptionTarget?
    @Binding var text: String
    let viewModel: SequencerViewModel
    func body(content: Content) -> some View {
        content.alert("Effect Description",
                      isPresented: Binding(
                        get: { target != nil },
                        set: { if !$0 { target = nil } }
                      ),
                      presenting: target) { tgt in
            TextField("Description", text: $text, axis: .vertical)
                .lineLimit(3...6)
            Button("Save") {
                viewModel.setEffectDescription(rowIndex: tgt.rowIndex,
                                                 effectIndex: tgt.effectIndex,
                                                 text)
                target = nil
            }
            Button("Cancel", role: .cancel) { target = nil }
        } message: { _ in
            Text("Free-text note on this effect. Shown in the selection readout.")
        }
    }
}

/// B47 insert-multiple-layers alert wrapper. Extracted as its
/// own ViewModifier because tacking it onto the body's long
/// modifier chain pushed SwiftUI's type-checker over budget.
private struct InsertLayersAlert: ViewModifier {
    @Binding var targetRow: Int?
    @Binding var countText: String
    let viewModel: SequencerViewModel
    func body(content: Content) -> some View {
        content.alert("Insert Multiple Layers Below",
                      isPresented: Binding(
                        get: { targetRow != nil },
                        set: { if !$0 { targetRow = nil } }
                      ),
                      presenting: targetRow) { row in
            TextField("Count", text: $countText)
                .keyboardType(.numberPad)
            Button("Insert") {
                let count = Int(countText) ?? 0
                _ = viewModel.insertLayersBelow(rowIndex: row, count: count)
                targetRow = nil
            }
            Button("Cancel", role: .cancel) { targetRow = nil }
        } message: { _ in
            Text("How many empty layers to add below the current one?")
        }
    }
}

/// B49 — Falcon Player sub-sequence export document. Same
/// temp-path → `.fileExporter` pattern as `XTimingExportDoc`.
struct FSEQExportDoc: FileDocument {
    static var readableContentTypes: [UTType] { [kESEQFileType] }
    static var writableContentTypes: [UTType] { [kESEQFileType] }
    let sourcePath: String
    init(sourcePath: String) { self.sourcePath = sourcePath }
    init(configuration: ReadConfiguration) throws { sourcePath = "" }
    func fileWrapper(configuration: WriteConfiguration) throws -> FileWrapper {
        if !sourcePath.isEmpty,
           let data = try? Data(contentsOf: URL(fileURLWithPath: sourcePath)) {
            return FileWrapper(regularFileWithContents: data)
        }
        return FileWrapper(regularFileWithContents: Data())
    }
}

/// Six-region effects grid shell with synchronized scrolling. Placeholder
/// content in each cell — the drawing (effects, timing marks, icons,
/// transitions) comes in Phase B-3. This view exists to prove sticky-top
/// and sticky-left behavior holds up on device before we invest in the
/// per-pixel drawing.
///
/// Layout:
///
///   row 1: View/time corner | ruler + waveform  (v-locked)
///   row 2: timing headers   | timing effects    (v-locked)
///   row 3: model headers    | model effects     (both axes)
///
/// Horizontal scroll shared across row-2 and row-3 right cells via
/// `TimelineState.hScrollOffsetPx`. Vertical scroll shared across row-3
/// left and right cells via `RowsScrollState.vScrollOffsetPx`.
struct SequencerGridV2View: View {
    @Environment(SequencerViewModel.self) var viewModel
    // Timeline state is owned by the parent (SequencerView) so toolbar
    // zoom controls and pinch-to-zoom here share the same state.
    let timeline: TimelineState
    // B61 — row-heading column width is user-resizable via a drag
    // handle between the header column and the grid canvas. Persisted
    // so the setting survives app restarts. Clamped to a sensible
    // range (80..400 pt) both here and in the drag handler so a
    // corrupted UserDefaults value can't strand the column.
    @AppStorage("gridRowHeaderWidth") private var rowHeaderWidthStorage: Double = 180
    private static let rowHeaderMinWidth: Double = 80
    private static let rowHeaderMaxWidth: Double = 400
    /// B42 — toggle between the default 48 pt waveform strip and a
    /// 96 pt "double-height" mode. Persisted so the user's choice
    /// survives app restarts.
    @AppStorage("waveformDoubleHeight") private var waveformDoubleHeight: Bool = false
    private var metrics: GridMetrics {
        var m = GridMetrics.standard
        m.rowHeaderWidth = CGFloat(
            min(Self.rowHeaderMaxWidth,
                max(Self.rowHeaderMinWidth, rowHeaderWidthStorage)))
        if waveformDoubleHeight {
            m.waveformHeight = 96
        }
        return m
    }
    @State private var rowsScroll = RowsScrollState()
    @State private var timingScroll = RowsScrollState()
    @State private var contextMenuTarget: ContextMenuTarget?
    // Tracks which sequence duration we've already auto-fit to, so we
    // only zoom-to-fit once per sequence load.
    @State private var fitDurationMS: Int = -1

    private struct ContextMenuTarget: Identifiable {
        let rowIndex: Int
        let effectIndex: Int
        var id: String { "\(rowIndex)-\(effectIndex)" }
    }

    @State private var transitionMenuTarget: TransitionMenuTarget?

    /// B67 / B69 timing-mark long-press target. `markIndex == nil`
    /// means "empty space at `ms`" (→ Add Mark Here menu); non-nil
    /// points to an existing mark (→ Delete Mark menu).
    private struct TimingMarkMenuTarget: Identifiable {
        let rowIndex: Int
        let markIndex: Int?
        let ms: Int
        var id: String { "\(rowIndex)-\(markIndex ?? -1)-\(ms)" }
    }
    @State private var timingMarkMenuTarget: TimingMarkMenuTarget?

    /// Empty-area long-press menu (model-band filler space below
    /// the last row). Drives a confirmationDialog with "Add Timing
    /// Track…" / "Edit Display Elements…" entries.
    @State private var emptyAreaMenuPresented: Bool = false

    /// B70 rename-timing-mark alert state.
    @State private var renameMarkTarget: TimingMarkMenuTarget?
    @State private var renameMarkText: String = ""

    /// B19 save-as-preset alert state.
    @State private var savePresetRequested: Bool = false
    @State private var savePresetName: String = ""

    /// B47 insert-N-layers prompt state.
    @State private var insertLayersTargetRow: Int? = nil
    @State private var insertLayersCountText: String = "3"

    /// B20 edit-description prompt state.
    @State private var editDescriptionTarget: EditDescriptionTarget? = nil
    @State private var editDescriptionText: String = ""

    /// B100 paste-replace confirmation state.
    @State private var pasteReplaceTarget: EditDescriptionTarget? = nil

    /// B32 loop-region context-menu trigger. Set non-nil when the
    /// user long-presses inside the existing loop band; cleared
    /// when the confirmation dialog dismisses.
    @State private var loopMenuPresented: Bool = false
    /// B41 waveform filter-picker trigger.
    @State private var waveformMenuPresented: Bool = false
    /// A9.1 custom-band picker sheet trigger.
    @State private var customBandSheetPresented: Bool = false
    /// A7 sound-class picker sheet trigger.
    @State private var classifyPickerPresented: Bool = false
    /// B97 Find / Replace replace-text buffer (sheet trigger lives
    /// on the view model so the Edit menu can flip it via ⌘F).
    @State private var findReplaceText: String = ""

    /// A8 stem-model install-location picker sheet trigger.
    private var iOS15Available: Bool {
        if #available(iOS 15.0, *) { return true }
        return false
    }

    /// Binding used by the A8 install-picker sheet's
    /// `.sheet(isPresented:)`. Setting false routes through the view
    /// model's cancel path.
    private var stemsInstallPickerBinding: Binding<Bool> {
        Binding(
            get: { viewModel.stemsPhase == .pickingRoot },
            set: { new in if !new { viewModel.cancelStemsInstall() } })
    }

    /// Binding for the A8 progress sheet — modal, non-cancellable.
    private var stemsProgressBinding: Binding<Bool> {
        Binding(
            get: { viewModel.stemsPhase == .downloading ||
                   viewModel.stemsPhase == .separating },
            set: { _ in })
    }

    /// B74 import-xtiming file-picker trigger.
    @State private var showingXTimingImporter: Bool = false
    /// B78 import-lyrics sheet state.
    @State private var importLyricsTargetRow: Int? = nil
    @State private var importLyricsText: String = ""
    @State private var importLyricsStart: String = "0.000"
    @State private var importLyricsEnd: String = ""
    /// B89 auto-label sheet state.
    @State private var autoLabelTargetRow: Int? = nil
    @State private var autoLabelStart: String = "1"
    @State private var autoLabelEnd: String = "100"
    @State private var autoLabelOverwrite: Bool = false
    /// B75 export-xtiming state. Target row is captured when the
    /// menu fires; the bridge writes to a temp path which the
    /// fileExporter then copies to the user's chosen destination.
    @State private var xtimingExportDoc: XTimingExportDoc? = nil
    @State private var showingXTimingExporter: Bool = false
    @State private var xtimingDefaultName: String = "Timing.xtiming"

    /// B49 export-model state. Target row captured when the menu
    /// fires; the bridge writes to a temp path, then `.fileExporter`
    /// copies to the user's chosen destination.
    @State private var fseqExportDoc: FSEQExportDoc? = nil
    @State private var showingFSEQExporter: Bool = false
    @State private var fseqDefaultName: String = "Model.eseq"

    /// B21 edit-timing dialog state. Fields are bound to seconds
    /// strings so users enter `5.25` and see `0.75` for duration;
    /// commit parses with `strtod`.
    @State private var editTimingTarget: ContextMenuTarget?
    @State private var editTimingStartText: String = ""
    @State private var editTimingEndText: String = ""

    var body: some View {
        GeometryReader { geo in
            // Partition rows into timing band (row 2) vs model band (row 3).
            // Uses the bridge's explicit timing-row index list.
            let timingIdxSet: Set<Int> = Set(
                (viewModel.document.timingRowIndices() as [NSNumber]).map { $0.intValue }
            )
            let timingRows = viewModel.rows.filter { timingIdxSet.contains($0.id) }
            let modelRows = viewModel.rows.filter { !timingIdxSet.contains($0.id) }

            let durationMS = viewModel.sequenceDurationMS
            let availableGridH = max(geo.size.height - metrics.topChromeHeight, 1)
            let rawTimingH = CGFloat(timingRows.count) * metrics.timingRowHeight
            // Cap timing band at ~1/3 of available grid height.
            let timingBandH = min(rawTimingH, availableGridH / 3)
            let selectedRowId = viewModel.selectedEffect?.rowIndex
            let modelAreaH = modelRows.reduce(CGFloat(0)) { sum, r in
                sum + ((r.id == selectedRowId) ? metrics.selectedRowHeight : metrics.rowHeight)
            }

            ZStack(alignment: .topLeading) {
                VStack(spacing: 0) {
                    // Row 1: view-picker corner + ruler/waveform strip.
                    HStack(alignment: .top, spacing: 0) {
                        topLeftCorner(availableWidth: geo.size.width)
                            .frame(width: metrics.rowHeaderWidth,
                                   height: metrics.topChromeHeight)
                        rowHeaderResizeHandle(height: metrics.topChromeHeight)
                        topChromeStrip(durationMS: durationMS)
                            .frame(height: metrics.topChromeHeight)
                    }
                    .frame(height: metrics.topChromeHeight)
                    Divider()

                    // Row 2: timing headers on the left (SwiftUI row
                    // labels in a scroll view for vertical sync), Metal
                    // canvas on the right. Only rendered when the
                    // current view has timing tracks.
                    if !timingRows.isEmpty {
                        let timingContentH = CGFloat(timingRows.count) * metrics.timingRowHeight
                        HStack(alignment: .top, spacing: 0) {
                            SyncedScrollView(
                                targetHOffset: nil,
                                targetVOffset: timingScroll.vScrollOffsetPx,
                                contentWidth: metrics.rowHeaderWidth,
                                contentHeight: timingContentH,
                                showsIndicators: false,
                                onScroll: { newOffset in
                                    timingScroll.vScrollOffsetPx = newOffset.y
                                }
                            ) {
                                timingHeaders(timingRows)
                            }
                            .frame(width: metrics.rowHeaderWidth,
                                   height: timingBandH)
                            rowHeaderResizeHandle(height: timingBandH)
                            TimingEffectsMetalGridView(
                                rows: timingRows,
                                rowHeight: metrics.timingRowHeight,
                                pixelsPerMS: timeline.pixelsPerMS,
                                scrollOffsetX: Binding(
                                    get: { timeline.hScrollOffsetPx },
                                    set: { timeline.hScrollOffsetPx = $0 }),
                                scrollOffsetY: Binding(
                                    get: { timingScroll.vScrollOffsetPx },
                                    set: { timingScroll.vScrollOffsetPx = $0 }),
                                onSeek: { ms in viewModel.seekTo(ms: ms) },
                                onPinchZoom: pinchZoomAction,
                                onUserInteraction: { timeline.noteUserInteraction() },
                                onLongPressMark: { rowId, markIdx, ms in
                                    timingMarkMenuTarget = TimingMarkMenuTarget(
                                        rowIndex: rowId, markIndex: markIdx, ms: ms)
                                },
                                onMarkDragEnd: { rowId, markIdx, newStart, newEnd in
                                    _ = viewModel.moveTimingMark(
                                        rowIndex: rowId, markIndex: markIdx,
                                        newStartMS: newStart, newEndMS: newEnd)
                                },
                                onDoubleTapMark: { rowId, markIdx in
                                    viewModel.playLoopForTimingMark(rowIndex: rowId,
                                                                      markIndex: markIdx)
                                }
                            )
                            .frame(height: timingBandH)
                        }
                        .frame(height: timingBandH)
                        Divider()
                    }

                    // Row 3 — fills remaining space
                    HStack(alignment: .top, spacing: 0) {
                        modelRowHeaderColumn(modelRows: modelRows,
                                              modelAreaH: modelAreaH,
                                              availableModelBandH: availableGridH - timingBandH)
                        rowHeaderResizeHandle(height: nil)
                        modelEffectsMetalView(modelRows: modelRows)
                            .overlay(alignment: .trailing) {
                                ScrollbarOverlay(
                                    orientation: .vertical,
                                    viewportSize: availableGridH - timingBandH,
                                    contentSize: modelAreaH,
                                    offset: Binding(
                                        get: { rowsScroll.vScrollOffsetPx },
                                        set: { rowsScroll.vScrollOffsetPx = $0 }),
                                    onUserInteraction: { timeline.noteUserInteraction() }
                                )
                                .padding(.trailing, 2)
                            }
                            .overlay(alignment: .bottom) {
                                ScrollbarOverlay(
                                    orientation: .horizontal,
                                    viewportSize: geo.size.width - metrics.rowHeaderWidth,
                                    contentSize: timeline.contentWidth(forDurationMS: durationMS),
                                    offset: Binding(
                                        get: { timeline.hScrollOffsetPx },
                                        set: { timeline.hScrollOffsetPx = $0 }),
                                    onUserInteraction: { timeline.noteUserInteraction() }
                                )
                                .padding(.bottom, 2)
                            }
                    }
                }

                // Full-height play-position marker spanning ruler, waveform,
                // timing band, and effect grid. Only this subview reads
                // playPositionMS, so the main grid body no longer re-renders
                // (and canvases don't re-draw) on every playback tick.
                PlayPositionMarker(
                    timeline: timeline,
                    rowHeaderWidth: metrics.rowHeaderWidth,
                    gridWidth: geo.size.width,
                    gridHeight: geo.size.height
                )
                .allowsHitTesting(false)

                // B93: jump-scroll to keep the play marker visible during
                // playback. Isolated in its own view so the onChange that
                // fires on every playback tick only invalidates this
                // zero-sized placeholder, not the main grid body.
                AutoFollowPlayhead(
                    timeline: timeline,
                    availableContentWidth: max(0, geo.size.width - metrics.rowHeaderWidth)
                )
            }
            .onAppear {
                fitIfNeeded(durationMS: durationMS, availableWidth: geo.size.width)
                viewModel.refreshWaveformForZoom(pixelsPerMS: timeline.pixelsPerMS)
            }
            .onChange(of: durationMS) { _, newDuration in
                fitIfNeeded(durationMS: newDuration, availableWidth: geo.size.width)
            }
            .onChange(of: geo.size.width) { _, newWidth in
                // If the view is laid out after the sequence loaded with
                // zero width, retry the fit once a real width is known.
                if fitDurationMS != durationMS {
                    fitIfNeeded(durationMS: durationMS, availableWidth: newWidth)
                }
            }
            .onChange(of: timeline.pixelsPerMS) { _, newPPMS in
                viewModel.refreshWaveformForZoom(pixelsPerMS: newPPMS)
            }
            .onChange(of: viewModel.selectedEffect) { _, sel in
                scrollSelectionIntoView(
                    sel,
                    viewportWidth: geo.size.width - metrics.rowHeaderWidth,
                    availableGridH: availableGridH,
                    modelRows: modelRows,
                    timingBandH: timingBandH)
            }
        }
        .confirmationDialog(
            "Effect",
            isPresented: Binding(
                get: { contextMenuTarget != nil },
                set: { if !$0 { contextMenuTarget = nil } }
            ),
            presenting: contextMenuTarget
        ) { target in
            if viewModel.selectedEffects.count > 1 {
                // Multi-select bulk menu.
                let n = viewModel.selectedEffects.count
                Button("Align Start Times") {
                    viewModel.alignSelectedEffects(.startTimes)
                }
                Button("Align End Times") {
                    viewModel.alignSelectedEffects(.endTimes)
                }
                Button("Align Both Times") {
                    viewModel.alignSelectedEffects(.bothTimes)
                }
                Button("Align Centers") {
                    viewModel.alignSelectedEffects(.centerPoints)
                }
                Button("Match Duration") {
                    viewModel.alignSelectedEffects(.matchDuration)
                }
                Button("Shift-Align Start") {
                    viewModel.alignSelectedEffects(.startTimesShift)
                }
                Button("Shift-Align End") {
                    viewModel.alignSelectedEffects(.endTimesShift)
                }
                Button("Align to Closest Timing Mark") {
                    viewModel.alignSelectedEffectsToTimingMarks()
                }
                if viewModel.canCloseGapInSelection {
                    Button("Close Gap") {
                        viewModel.closeGapInSelectedEffects()
                    }
                }
                Button("Delete \(n) Effects", role: .destructive) {
                    viewModel.deleteSelectedEffects()
                }
                Button("Lock / Unlock \(n) Effects") {
                    viewModel.toggleLockSelectedEffects()
                }
                Button("Disable / Enable \(n) Effects") {
                    viewModel.toggleDisableSelectedEffects()
                }
                Button("Randomise \(n) Effects") {
                    viewModel.randomizeSelectedEffects()
                }
                Button("Reset \(n) Effects", role: .destructive) {
                    viewModel.resetSelectedEffectsToDefaults()
                }
                ForEach(viewModel.presets) { preset in
                    Button("Apply Preset: \(preset.name) to \(n)") {
                        _ = viewModel.applyPreset(preset)
                    }
                }
                Button("Deselect All") {
                    viewModel.clearSelection()
                }
                Button("Cancel", role: .cancel) {}
            } else {
                Button("Copy") { viewModel.copySelectedEffect() }
                if viewModel.hasClipboard {
                    Button("Paste Here") {
                        let startMS = (viewModel.rows[target.rowIndex].effects[target.effectIndex]).startTimeMS
                        // B100: pasting on top of an existing
                        // effect asks for confirmation before
                        // overwriting. When there's no overlap
                        // (which "Paste Here" on an occupied cell
                        // always has), just do the normal paste.
                        if viewModel.pasteWouldOverlap(rowIndex: target.rowIndex,
                                                        startMS: startMS) {
                            pasteReplaceTarget = EditDescriptionTarget(
                                rowIndex: target.rowIndex,
                                effectIndex: target.effectIndex)
                        } else {
                            viewModel.pasteEffect(rowIndex: target.rowIndex,
                                                   startMS: startMS)
                        }
                    }
                }
                if viewModel.canSplitSelectedAtPlayMarker {
                    Button("Split at Play Marker") {
                        viewModel.splitSelectedEffectAtPlayMarker()
                    }
                }
                Button("Edit Timing…") {
                    let e = viewModel.rows[target.rowIndex].effects[target.effectIndex]
                    editTimingStartText = Self.formatMS(e.startTimeMS)
                    editTimingEndText = Self.formatMS(e.endTimeMS)
                    editTimingTarget = target
                }
                Button("Select All in Row") {
                    viewModel.selectAllEffectsInRow(rowIndex: target.rowIndex)
                }
                Button("Select All in Model") {
                    viewModel.selectAllEffectsInModel(rowIndex: target.rowIndex)
                }
                Button("Select All in Column") {
                    let e = viewModel.rows[target.rowIndex].effects[target.effectIndex]
                    viewModel.selectAllEffectsInColumn(spanStartMS: e.startTimeMS,
                                                       spanEndMS: e.endTimeMS)
                }
                Button(viewModel.isEffectLocked(rowIndex: target.rowIndex,
                                                 effectIndex: target.effectIndex)
                       ? "Unlock" : "Lock") {
                    viewModel.toggleLock(rowIndex: target.rowIndex,
                                         effectIndex: target.effectIndex)
                }
                Button(viewModel.isEffectRenderDisabled(rowIndex: target.rowIndex,
                                                        effectIndex: target.effectIndex)
                       ? "Enable" : "Disable") {
                    viewModel.toggleDisable(rowIndex: target.rowIndex,
                                            effectIndex: target.effectIndex)
                }
                Button("Randomise Settings") {
                    viewModel.randomizeSelectedEffects()
                }
                Button("Reset to Defaults", role: .destructive) {
                    viewModel.resetSelectedEffectsToDefaults()
                }
                // B19 — session-only effect presets. Save captures
                // the current effect; apply replaces settings on
                // every selected effect.
                Button("Edit Description…") {
                    editDescriptionText = viewModel.effectDescription(
                        rowIndex: target.rowIndex,
                        effectIndex: target.effectIndex)
                    editDescriptionTarget = EditDescriptionTarget(
                        rowIndex: target.rowIndex,
                        effectIndex: target.effectIndex)
                }
                Button("Save as Preset…") {
                    savePresetName = ""
                    savePresetRequested = true
                }
                ForEach(viewModel.presets) { preset in
                    Button("Apply Preset: \(preset.name)") {
                        _ = viewModel.applyPreset(preset)
                    }
                }
                Button("Delete", role: .destructive) {
                    viewModel.deleteEffect(rowIndex: target.rowIndex,
                                           effectIndex: target.effectIndex)
                }
                Button("Cancel", role: .cancel) {}
            }
        }
        // B67 / B69 timing-mark long-press menu. Distinct dialog from
        // the effect context menu so both can coexist without menu
        // content cross-contamination.
        .confirmationDialog(
            "Timing",
            isPresented: Binding(
                get: { timingMarkMenuTarget != nil },
                set: { if !$0 { timingMarkMenuTarget = nil } }
            ),
            presenting: timingMarkMenuTarget
        ) { target in
            if let markIdx = target.markIndex {
                Button("Rename Mark") {
                    let current = viewModel.rows[target.rowIndex].effects[markIdx].name
                    renameMarkText = current
                    renameMarkTarget = target
                }
                // B90: quickly flip the "-shimmer" suffix. The
                // Papagayo lipsync pipeline uses it as a convention
                // to request a shimmer modifier on the resolved
                // face. Label flips between "Add" / "Remove" based
                // on current state.
                let hasShimmer = viewModel.rows[target.rowIndex].effects[markIdx]
                    .name.hasSuffix("-shimmer")
                Button(hasShimmer ? "Remove -shimmer Suffix"
                                  : "Add -shimmer Suffix") {
                    _ = viewModel.toggleShimmerSuffixOnMark(
                        rowIndex: target.rowIndex, markIndex: markIdx)
                }
                if viewModel.canSplitMarkAtPlayMarker(rowIndex: target.rowIndex,
                                                      markIndex: markIdx) {
                    Button("Split at Play Marker") {
                        _ = viewModel.splitTimingMark(rowIndex: target.rowIndex,
                                                       markIndex: markIdx,
                                                       atMS: viewModel.playPositionMS)
                    }
                }
                if viewModel.canMergeMarkWithNext(rowIndex: target.rowIndex,
                                                   markIndex: markIdx) {
                    Button("Merge with Next") {
                        _ = viewModel.mergeTimingMarkWithNext(rowIndex: target.rowIndex,
                                                               markIndex: markIdx)
                    }
                }
                // B84 per-mark: only the phrase layer with a labelled
                // mark can be broken down into words.
                if viewModel.canBreakdownPhrase(rowIndex: target.rowIndex,
                                                 markIndex: markIdx) {
                    Button("Breakdown This Phrase") {
                        _ = viewModel.breakdownPhrase(rowIndex: target.rowIndex,
                                                        markIndex: markIdx)
                    }
                }
                Button("Delete Mark", role: .destructive) {
                    _ = viewModel.deleteTimingMark(rowIndex: target.rowIndex,
                                                    markIndex: markIdx)
                }
                Button("Cancel", role: .cancel) {}
            } else {
                Button("Add Mark Here") {
                    addTimingMarkFromTap(rowIndex: target.rowIndex, atMS: target.ms)
                }
                Button("Cancel", role: .cancel) {}
            }
        }
        // B21 edit-timing alert. Two fields (start, end) in seconds
        // with 3 decimal places; parses with `strtod` per repo rule
        // (no throwing std::stod). Calls `moveEffect` on commit.
        .alert("Edit Timing",
               isPresented: Binding(
                get: { editTimingTarget != nil },
                set: { if !$0 { editTimingTarget = nil } }
               ),
               presenting: editTimingTarget) { target in
            TextField("Start (seconds)", text: $editTimingStartText)
                .keyboardType(.decimalPad)
            TextField("End (seconds)", text: $editTimingEndText)
                .keyboardType(.decimalPad)
            Button("OK") {
                if let startMS = Self.parseSeconds(editTimingStartText),
                   let endMS = Self.parseSeconds(editTimingEndText),
                   endMS > startMS {
                    viewModel.moveEffect(rowIndex: target.rowIndex,
                                          effectIndex: target.effectIndex,
                                          newStartMS: startMS, newEndMS: endMS)
                }
                editTimingTarget = nil
            }
            Button("Cancel", role: .cancel) {
                editTimingTarget = nil
            }
        } message: { _ in
            Text("Enter start and end times in seconds.")
        }
        // B89 auto-label-marks alert.
        .alert("Auto-Label Marks",
               isPresented: Binding(
                get: { autoLabelTargetRow != nil },
                set: { if !$0 { autoLabelTargetRow = nil } }
               ),
               presenting: autoLabelTargetRow) { rowIdx in
            TextField("Start number", text: $autoLabelStart)
                .keyboardType(.numberPad)
            TextField("End number (wraps)", text: $autoLabelEnd)
                .keyboardType(.numberPad)
            Toggle("Overwrite existing labels", isOn: $autoLabelOverwrite)
            Button("Label") {
                let start = Int(autoLabelStart) ?? 1
                let end = Int(autoLabelEnd) ?? start
                _ = viewModel.autoLabelTimingMarks(
                    rowIndex: rowIdx, startNum: start, endNum: end,
                    overwrite: autoLabelOverwrite)
                autoLabelTargetRow = nil
            }
            Button("Cancel", role: .cancel) {
                autoLabelTargetRow = nil
            }
        } message: { _ in
            Text("Number the marks starting at Start; the count wraps back when it passes End. With Overwrite off, only unlabeled marks get numbers.")
        }
        // B78 import-lyrics sheet.
        .sheet(isPresented: Binding(
            get: { importLyricsTargetRow != nil },
            set: { if !$0 { importLyricsTargetRow = nil } }
        )) {
            if let rowIdx = importLyricsTargetRow {
                ImportLyricsSheet(
                    rowIndex: rowIdx,
                    text: $importLyricsText,
                    startText: $importLyricsStart,
                    endText: $importLyricsEnd,
                    onCommit: { start, end in
                        let startMS = Int((Double(start) ?? 0.0) * 1000)
                        let endMS = Int((Double(end) ?? 0.0) * 1000)
                        _ = viewModel.importLyrics(
                            rowIndex: rowIdx,
                            lyrics: importLyricsText,
                            startMS: startMS, endMS: endMS)
                        importLyricsTargetRow = nil
                    },
                    onCancel: { importLyricsTargetRow = nil }
                )
            }
        }
        // B74 .xtiming import.
        .fileImporter(
            isPresented: $showingXTimingImporter,
            allowedContentTypes: [kXTimingFileType],
            allowsMultipleSelection: false
        ) { result in
            guard case .success(let urls) = result, let url = urls.first else { return }
            let path = url.path
            _ = XLSequenceDocument.obtainAccess(toPath: path,
                                                  enforceWritable: false)
            _ = viewModel.importXTiming(path: path)
        }
        // B75 .xtiming export.
        .fileExporter(
            isPresented: $showingXTimingExporter,
            document: xtimingExportDoc,
            contentType: kXTimingFileType,
            defaultFilename: xtimingDefaultName
        ) { _ in
            // Nothing more to do — bridge already wrote the temp
            // file; the exporter copied it to the user's pick.
            xtimingExportDoc = nil
        }
        // B49 export model as Falcon Player `.eseq` sub-sequence.
        .fileExporter(
            isPresented: $showingFSEQExporter,
            document: fseqExportDoc,
            contentType: kESEQFileType,
            defaultFilename: fseqDefaultName
        ) { _ in
            fseqExportDoc = nil
        }
        // B41 waveform filter picker.
        .confirmationDialog(
            "Waveform",
            isPresented: $waveformMenuPresented
        ) {
            ForEach(SequencerViewModel.WaveformFilter.allCases, id: \.rawValue) { filter in
                // HTDemucs stems require macOS 12 / iOS 15 for the
                // Float16 MLMultiArray I/O the model uses. Hide the
                // stem entries on older iOS.
                if !filter.requiresStems || iOS15Available {
                    Button {
                        if filter == .custom {
                            // Opening the custom-band sheet implicitly
                            // activates the filter so the user can see
                            // live preview as they drag the sliders.
                            viewModel.waveformFilter = .custom
                            customBandSheetPresented = true
                        } else if filter.requiresStems {
                            // First tap kicks off the install / separator
                            // flow; the view model flips through its
                            // phases and flips `waveformFilter` when
                            // stems are ready.
                            viewModel.prepareStems(for: filter)
                        } else {
                            viewModel.waveformFilter = filter
                        }
                    } label: {
                        if viewModel.waveformFilter == filter {
                            Label(filter.displayName, systemImage: "checkmark")
                        } else {
                            Text(filter.displayName)
                        }
                    }
                }
            }
            // A2 onset overlay — independent of the filter radio
            // group so it stacks on top of any chosen filter band.
            Button {
                viewModel.toggleShowOnsets()
            } label: {
                if viewModel.showOnsets {
                    Label("Hide Onsets", systemImage: "checkmark")
                } else {
                    Text("Show Onsets")
                }
            }
            // A5 pitch contour overlay. Same "overlay on top of any
            // filter choice" idiom as onsets.
            Button {
                viewModel.toggleShowPitchContour()
            } label: {
                if viewModel.showPitchContour {
                    Label("Hide Pitch Contour", systemImage: "checkmark")
                } else {
                    Text("Show Pitch Contour")
                }
            }
            // A6 spectrogram view — replaces the peak polygons with
            // an STFT magnitude image. Not an overlay: mutually
            // exclusive with the waveform view, though other overlays
            // (onsets, pitch) still render on top.
            Button {
                viewModel.toggleShowSpectrogram()
            } label: {
                if viewModel.showSpectrogram {
                    Label("Switch to Waveform", systemImage: "checkmark")
                } else {
                    Text("View as Spectrogram")
                }
            }
            // B43 alt-track switch — only surfaced when the sequence
            // declares at least one alternate audio track. Selection
            // is purely cosmetic (waveform-only); playback stays on
            // the main track.
            if viewModel.altAudioTrackNames.count > 0 {
                Button {
                    viewModel.setActiveWaveformTrack(-1)
                } label: {
                    if viewModel.activeWaveformTrack == -1 {
                        Label("Main Audio", systemImage: "checkmark")
                    } else {
                        Text("Main Audio")
                    }
                }
                ForEach(Array(viewModel.altAudioTrackNames.enumerated()),
                         id: \.offset) { idx, name in
                    Button {
                        viewModel.setActiveWaveformTrack(idx)
                    } label: {
                        if viewModel.activeWaveformTrack == idx {
                            Label(name, systemImage: "checkmark")
                        } else {
                            Text(name)
                        }
                    }
                }
            }
            // A7 sound classification — stacks on top of the filter
            // pick too. First tap kicks off classification (or re-
            // presents the picker if already done); tapping again
            // with a class selected clears it.
            if viewModel.selectedSoundClass == nil {
                Button {
                    classifyPickerPresented = true
                } label: {
                    Text("Classify Audio…")
                }
            } else {
                Button {
                    viewModel.selectedSoundClass = nil
                } label: {
                    Label("Clear \(viewModel.selectedSoundClass ?? "")",
                           systemImage: "checkmark")
                }
                Button {
                    classifyPickerPresented = true
                } label: {
                    Text("Change Class…")
                }
            }
            Button("Cancel", role: .cancel) {}
        }
        // A9 chord-detection preview sheet.
        // A8 stem install-location picker (first run).
        .sheet(isPresented: stemsInstallPickerBinding) {
            StemInstallSheet(
                roots: (viewModel.document.stemModelCandidateRoots() as [String]),
                onCommit: { root in viewModel.commitStemsInstall(toRoot: root) },
                onCancel: { viewModel.cancelStemsInstall() })
        }
        // A8 stem download + separation progress sheet.
        .sheet(isPresented: stemsProgressBinding) {
            StemProgressSheet(
                phase: viewModel.stemsPhase,
                pct: viewModel.stemsProgressPct)
        }
        // A7 class picker sheet.
        .sheet(isPresented: $classifyPickerPresented) {
            SoundClassifyPickerSheet(
                classes: viewModel.soundClasses,
                isBusy: viewModel.isClassifyingSound,
                onClassify: {
                    viewModel.classifySound()
                },
                onPick: { name in
                    viewModel.selectedSoundClass = name
                    classifyPickerPresented = false
                },
                onCancel: { classifyPickerPresented = false })
        }
        // A9.1 custom parametric band editor.
        .sheet(isPresented: $customBandSheetPresented) {
            CustomBandSheet(
                lowNote: Binding(
                    get: { viewModel.customBandLowNote },
                    set: { viewModel.customBandLowNote = $0 }),
                highNote: Binding(
                    get: { viewModel.customBandHighNote },
                    set: { viewModel.customBandHighNote = $0 }),
                onDone: { customBandSheetPresented = false })
        }
        // B32 loop-region context menu (long-press inside the loop
        // band). Actions: toggle Play Loop (B33), Render Loop Region
        // (B44), Clear Loop.
        .confirmationDialog(
            "Loop Region",
            isPresented: $loopMenuPresented
        ) {
            Button(viewModel.loopPlayEnabled ? "Stop Play Loop" : "Play Loop Region") {
                if !viewModel.isPlaying {
                    viewModel.loopPlayEnabled = true
                    viewModel.seekTo(ms: viewModel.loopStartMS)
                    viewModel.play()
                } else {
                    viewModel.toggleLoopPlay()
                }
            }
            Button("Render Loop Region") {
                viewModel.renderLoopRegion()
            }
            Button("Clear Loop", role: .destructive) {
                viewModel.clearLoopRegion()
            }
            Button("Cancel", role: .cancel) {}
        }
        // F-6 — Display Elements editor modal. State lives on the
        // view model so F-4 menu-bar "Edit Display Elements…" can
        // flip the same flag.
        .sheet(isPresented: Bindable(viewModel).showingDisplayElements) {
            DisplayElementsSheet()
                .environment(viewModel)
        }
        // B97 Find / Replace inspector sheet. ⌘F shortcut wires up
        // through the Edit menu in `XLightsCommands` too.
        .sheet(isPresented: Bindable(viewModel).findReplacePresented) {
            FindReplaceSheet(replaceText: $findReplaceText,
                              onDone: { viewModel.findReplacePresented = false })
                .environment(viewModel)
                .presentationDetents([.medium, .large])
        }
        // B70 rename-timing-mark alert.
        .alert("Rename Mark",
               isPresented: Binding(
                get: { renameMarkTarget != nil },
                set: { if !$0 { renameMarkTarget = nil } }
               ),
               presenting: renameMarkTarget) { target in
            TextField("Label", text: $renameMarkText)
            Button("OK") {
                if let markIdx = target.markIndex {
                    _ = viewModel.renameTimingMark(rowIndex: target.rowIndex,
                                                    markIndex: markIdx,
                                                    label: renameMarkText)
                }
                renameMarkText = ""
                renameMarkTarget = nil
            }
            Button("Cancel", role: .cancel) {
                renameMarkText = ""
                renameMarkTarget = nil
            }
        } message: { _ in
            Text("Timing-mark label (leave blank to clear).")
        }
        // B19 save-as-preset alert.
        .alert("Save Preset", isPresented: $savePresetRequested) {
            TextField("Preset name", text: $savePresetName)
            Button("Save") {
                _ = viewModel.saveSelectedEffectAsPreset(name: savePresetName)
                savePresetName = ""
            }
            Button("Cancel", role: .cancel) { savePresetName = "" }
        } message: {
            Text("Saves the current effect's settings + palette under a name you can re-apply to other effects in this session.")
        }
        .modifier(InsertLayersAlert(
            targetRow: $insertLayersTargetRow,
            countText: $insertLayersCountText,
            viewModel: viewModel))
        .modifier(EditDescriptionAlert(
            target: $editDescriptionTarget,
            text: $editDescriptionText,
            viewModel: viewModel))
        .modifier(PasteReplaceAlert(
            target: $pasteReplaceTarget,
            viewModel: viewModel))
        .modifier(TransitionPickerDialog(
            target: $transitionMenuTarget,
            viewModel: viewModel))
    }

    /// B21 time formatting / parsing helpers. `formatMS` emits
    /// `5.250` for 5250 ms (3 decimal places, trimmed trailing
    /// zero-run if none are needed — actually keep them for
    /// consistent alignment). `parseSeconds` goes the other way
    /// using strtod (the repo avoids std::stod / std::stoi because
    /// they throw on bad input).
    static func formatMS(_ ms: Int) -> String {
        return String(format: "%.3f", Double(ms) / 1000.0)
    }

    static func parseSeconds(_ s: String) -> Int? {
        let trimmed = s.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !trimmed.isEmpty else { return nil }
        var endPtr: UnsafeMutablePointer<CChar>? = nil
        let val = trimmed.withCString { cStr -> Double in
            strtod(cStr, &endPtr)
        }
        // Reject if strtod didn't consume anything meaningful.
        if endPtr == nil { return nil }
        if val < 0 || !val.isFinite { return nil }
        return Int((val * 1000.0).rounded())
    }

    /// B61 drag handle between the row-header column and the grid
    /// canvas. Visually looks like the `Divider()` it replaced (thin
    /// hairline) but owns a wider invisible hit strip so the grab
    /// affordance is a comfortable touch target. Drag horizontally
    /// updates `rowHeaderWidthStorage`; the `metrics` computed
    /// property re-clamps so the column never escapes its bounds.
    /// `.hoverEffect` gives Magic Keyboard pointer users a proper
    /// resize visual.
    private func rowHeaderResizeHandle(height: CGFloat?) -> some View {
        ColumnResizeHandle(
            height: height,
            minWidth: Self.rowHeaderMinWidth,
            maxWidth: Self.rowHeaderMaxWidth,
            width: Binding(
                get: { rowHeaderWidthStorage },
                set: { rowHeaderWidthStorage = $0 }
            )
        )
    }

    /// B49: write the rendered channel data for the row's model to a
    /// temp `.eseq` file and hand that path off to SwiftUI's
    /// `.fileExporter`. When `useLoopRegion` is true and a loop is
    /// active, only the loop range is exported; otherwise the full
    /// sequence is written.
    private func startFSEQExport(rowIndex: Int, useLoopRegion: Bool) {
        let modelName = (viewModel.document.rowModelName(at: Int32(rowIndex)) as String?) ?? "Model"
        let safeName = modelName.isEmpty ? "Model" : modelName
        let tempDir = FileManager.default.temporaryDirectory
        let tempPath = tempDir.appendingPathComponent(
            "\(safeName)-\(UUID().uuidString).eseq").path
        let startMS: Int?
        let endMS: Int?
        if useLoopRegion, viewModel.hasLoopRegion {
            startMS = viewModel.loopStartMS
            endMS = viewModel.loopEndMS
        } else {
            startMS = nil
            endMS = nil
        }
        guard viewModel.exportModelAsFSEQ(rowIndex: rowIndex, path: tempPath,
                                           startMS: startMS, endMS: endMS) else {
            return
        }
        fseqExportDoc = FSEQExportDoc(sourcePath: tempPath)
        fseqDefaultName = "\(safeName).eseq"
        showingFSEQExporter = true
    }

    /// B75: write the timing track to a temp `.xtiming` file and
    /// hand that path off to SwiftUI's `.fileExporter` so the user
    /// picks a destination. The exporter then copies the bytes.
    private func startXTimingExport(rowIndex: Int, trackName: String) {
        let safeName = trackName.isEmpty ? "Timing" : trackName
        let tempDir = FileManager.default.temporaryDirectory
        let tempPath = tempDir.appendingPathComponent("\(safeName)-\(UUID().uuidString).xtiming").path
        guard viewModel.exportTimingTrack(rowIndex: rowIndex, path: tempPath) else {
            return
        }
        xtimingExportDoc = XTimingExportDoc(sourcePath: tempPath)
        xtimingDefaultName = "\(safeName).xtiming"
        showingXTimingExporter = true
    }

    /// B67: default add-mark duration is 500 ms, clamped against the
    /// next existing mark on that row (min 100 ms) and the sequence
    /// end. Start = tap time (clamped >= previous mark's end).
    private func addTimingMarkFromTap(rowIndex: Int, atMS: Int) {
        guard rowIndex >= 0, rowIndex < viewModel.rows.count else { return }
        let row = viewModel.rows[rowIndex]
        var startMS = atMS
        var endMS = atMS + 500
        var prevEnd = 0
        var nextStart = viewModel.sequenceDurationMS
        for e in row.effects {
            if e.endTimeMS <= startMS { prevEnd = max(prevEnd, e.endTimeMS) }
            if e.startTimeMS >= startMS && e.startTimeMS < nextStart {
                nextStart = e.startTimeMS
            }
        }
        startMS = max(prevEnd, startMS)
        endMS = min(endMS, nextStart)
        if endMS <= startMS + 50 {
            // Collapsed window — fall back to 100 ms minimum or skip.
            endMS = startMS + 100
            if endMS > nextStart || endMS > viewModel.sequenceDurationMS {
                return
            }
        }
        _ = viewModel.addTimingMark(rowIndex: rowIndex,
                                     startMS: startMS, endMS: endMS)
    }

    // MARK: - Row 1: view/time corner + top chrome

    private func topLeftCorner(availableWidth: CGFloat) -> some View {
        let views = (viewModel.document.availableViews() as [String])
        let currentIdx = Int(viewModel.document.currentViewIndex())
        let currentName = (currentIdx >= 0 && currentIdx < views.count)
            ? views[currentIdx]
            : "Master View"
        return VStack(alignment: .leading, spacing: 2) {
            HStack(spacing: 4) {
                Text("View:").font(.caption).foregroundStyle(.secondary)
                Menu {
                    ForEach(Array(views.enumerated()), id: \.offset) { idx, name in
                        Button {
                            viewModel.document.setCurrentViewIndex(Int32(idx))
                            viewModel.reloadRows()
                        } label: {
                            if idx == currentIdx {
                                Label(name, systemImage: "checkmark")
                            } else {
                                Text(name)
                            }
                        }
                    }
                    // Add Timing Track + Audio Onsets / Tempo / Chords
                    // / AI Lyrics now all live in the unified
                    // AddTimingTrackSheet (presented at app level).
                    // Just one entry point here that flips the flag.
                    Divider()
                    Button {
                        viewModel.showingAddTimingTrack = true
                    } label: {
                        Label("Add Timing Track…", systemImage: "plus.rectangle")
                    }
                    Button {
                        showingXTimingImporter = true
                    } label: {
                        Label("Import Timing Track…",
                               systemImage: "square.and.arrow.down")
                    }
                    // B83: derive a timing track from the selected
                    // effect's owning model. One mark per
                    // distinct effect range across the model's rows.
                    // Gated on a selection since we need a model to
                    // source from. Stays here (rather than moving to
                    // the unified sheet) because it's bound to the
                    // current effect selection — surfacing it from a
                    // sheet that doesn't know which effect is
                    // selected would be more confusing than useful.
                    Button {
                        if let sel = viewModel.selectedEffect {
                            _ = viewModel.createTimingTrackFromEffects(
                                modelRowIndex: sel.rowIndex, trackName: "")
                        }
                    } label: {
                        Label("Create Timing from Selected Effect's Model",
                               systemImage: "waveform.path.badge.plus")
                    }
                    .disabled(viewModel.selectedEffect == nil)
                    // B37: re-fit the whole sequence into the viewport.
                    Divider()
                    Button {
                        zoomToFitSequence(availableWidth: availableWidth)
                    } label: {
                        Label("Zoom to Fit", systemImage: "arrow.up.left.and.arrow.down.right")
                    }
                    // B36: fit the current selection (single or multi).
                    if viewModel.selectedEffect != nil
                        || !viewModel.selectedEffects.isEmpty {
                        Button {
                            zoomToSelection(availableWidth: availableWidth)
                        } label: {
                            Label("Zoom to Selection", systemImage: "arrow.up.backward.and.arrow.down.forward")
                        }
                    }
                    // B57: global collapse / expand.
                    Divider()
                    Button {
                        viewModel.collapseAllModels()
                    } label: {
                        Label("Collapse All", systemImage: "chevron.up.chevron.down")
                    }
                    Button {
                        viewModel.expandAllElements()
                    } label: {
                        Label("Expand All", systemImage: "arrow.up.and.down")
                    }
                    // B81: hide / show every timing row at once.
                    let allHidden = viewModel.allTimingTracksHidden
                    Button {
                        viewModel.setAllTimingTracksHidden(!allHidden)
                    } label: {
                        Label(allHidden ? "Show All Timing Tracks"
                                        : "Hide All Timing Tracks",
                              systemImage: allHidden ? "eye" : "eye.slash")
                    }
                    // B82: copy every visible timing track into
                    // every non-master view.
                    Button {
                        _ = viewModel.addAllTimingTracksToAllViews()
                    } label: {
                        Label("Add Timing Tracks to All Views",
                              systemImage: "rectangle.stack.badge.plus")
                    }
                    // B42: double the waveform strip height when the
                    // user wants finer detail on the peaks.
                    Button {
                        waveformDoubleHeight.toggle()
                    } label: {
                        Label(waveformDoubleHeight ? "Standard Waveform Height"
                                                   : "Double Waveform Height",
                              systemImage: waveformDoubleHeight
                                ? "rectangle.compress.vertical"
                                : "rectangle.expand.vertical")
                    }
                    // B34 / B35 numbered-tag markers. Set / go-to /
                    // clear at the current play head. Desktop parity
                    // with the 0..9 bookmarks on the sequencer ruler.
                    Divider()
                    tagsMenuSection()
                    // F-6: Display Elements editor. Launches a modal
                    // sheet that lets the user create / edit views
                    // and manage per-view model + timing membership.
                    Divider()
                    Button {
                        viewModel.showingDisplayElements = true
                    } label: {
                        Label("Edit Display Elements…",
                              systemImage: "rectangle.stack.badge.plus")
                    }
                } label: {
                    HStack(spacing: 2) {
                        Text(currentName)
                            .font(.caption).fontWeight(.medium)
                            .foregroundStyle(.primary)
                            .lineLimit(1)
                        Image(systemName: "chevron.down")
                            .font(.caption2)
                            .foregroundStyle(.secondary)
                    }
                }
                Spacer()
            }
            TimeDisplayLabel()
                .font(.system(.caption, design: .monospaced))
            SelectionReadout()
                .font(.system(.caption2, design: .monospaced))
                .foregroundStyle(.secondary)
                .lineLimit(1)
            Spacer()
        }
        .padding(6)
        .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topLeading)
        .background(Color(.secondarySystemBackground))
    }

    /// Row-1 ruler + waveform strip. Extracted from `body` so the
    /// main GeometryReader expression stays within Swift's type-
    /// check budget — the Metal view takes ~20 parameters and was
    /// pushing the enclosing body over the limit after B34 tags were
    /// added.
    @ViewBuilder
    private func topChromeStrip(durationMS: Int) -> some View {
        TopChromeMetalGridView(
            durationMS: durationMS,
            pixelsPerMS: timeline.pixelsPerMS,
            rulerHeight: metrics.rulerHeight,
            waveformHeight: metrics.waveformHeight,
            hasAudio: viewModel.hasAudio,
            peaks: viewModel.hasAudio ? viewModel.waveformPeaks : [],
            scrollOffsetX: Binding(
                get: { timeline.hScrollOffsetPx },
                set: { timeline.hScrollOffsetPx = $0 }),
            onSeek: { ms in viewModel.seekTo(ms: ms) },
            onScrubSeek: { ms in viewModel.scrubSeekTo(ms: ms) },
            onPinchZoom: pinchZoomAction,
            onUserInteraction: { timeline.noteUserInteraction() },
            loopStartMS: viewModel.loopStartMS,
            loopEndMS: viewModel.loopEndMS,
            hasLoop: viewModel.hasLoopRegion,
            onSetLoop: { start, end in
                viewModel.setLoopRegion(startMS: start, endMS: end)
            },
            onLoopMenu: { _ in loopMenuPresented = true },
            onWaveformMenu: { waveformMenuPresented = true },
            tagPositions: viewModel.tagPositions,
            showOnsets: viewModel.showOnsets,
            onsetMS: viewModel.showOnsets ? viewModel.onsetTimesMS : [],
            showPitchContour: viewModel.showPitchContour,
            pitchContour: viewModel.showPitchContour ? viewModel.pitchContour : [],
            showSpectrogram: viewModel.showSpectrogram,
            spectrogramFetcher: { s, e, w, h in
                viewModel.spectrogramBGRA(fromMS: s, toMS: e,
                                           width: w, height: h)
            }
        )
    }

    /// B34 / B35 — tag menu entries for the View-picker. Extracted
    /// from `topLeftCorner` so the enclosing body stays small enough
    /// for Swift's type-checker.
    @ViewBuilder
    private func tagsMenuSection() -> some View {
        Menu("Set Tag") {
            ForEach(0..<10, id: \.self) { i in
                Button("Set Tag \(i) at Play Head") {
                    viewModel.setTag(i, atMS: viewModel.playPositionMS)
                }
            }
        }
        Menu("Go To Tag") {
            ForEach(0..<10, id: \.self) { i in
                Button("Tag \(i)") { viewModel.goToTag(i) }
                    .disabled(viewModel.tagPositions[i] < 0)
            }
        }
        Button("Clear All Tags", role: .destructive) {
            viewModel.clearAllTags()
        }
        .disabled(!viewModel.tagPositions.contains(where: { $0 >= 0 }))
    }

    /// Shared pinch-to-zoom handler used by all three canvases so zoom
    /// steps and anchor behavior match everywhere.
    private var pinchZoomAction: (CGFloat, CGFloat) -> Void {
        { scaleDelta, anchorX in
            let oldPPMS = timeline.pixelsPerMS
            let newPPMS = min(max(oldPPMS * scaleDelta, 0.005), 2.0)
            if abs(newPPMS - oldPPMS) < 1e-6 { return }
            let anchorMS = oldPPMS > 0 ? anchorX / oldPPMS : 0
            let newAnchorX = anchorMS * newPPMS
            timeline.pixelsPerMS = newPPMS
            timeline.hScrollOffsetPx += (newAnchorX - anchorX)
        }
    }

    /// Ensure the newly-selected effect is visible: adjust horizontal
    /// scroll so the effect's x-range sits inside the viewport, and
    /// vertical scroll so its row is on-screen. Runs on any selection
    /// change — clicked effects already hit the viewport, but arrow-
    /// key navigation can land on off-screen effects.
    private func scrollSelectionIntoView(
        _ sel: SequencerViewModel.EffectSelection?,
        viewportWidth: CGFloat,
        availableGridH: CGFloat,
        modelRows: [SequencerViewModel.RowInfo],
        timingBandH: CGFloat
    ) {
        guard let sel = sel, viewportWidth > 0 else { return }
        // Horizontal: center the effect in the viewport if it's
        // fully off-screen; otherwise nudge the closest edge into
        // view with a bit of padding.
        let x1 = CGFloat(sel.startTimeMS) * timeline.pixelsPerMS
        let x2 = CGFloat(sel.endTimeMS)   * timeline.pixelsPerMS
        let curOffset = timeline.hScrollOffsetPx
        let pad: CGFloat = 24
        if x2 < curOffset + pad {
            timeline.hScrollOffsetPx = max(0, x1 - pad)
        } else if x1 > curOffset + viewportWidth - pad {
            timeline.hScrollOffsetPx = max(0, x2 - viewportWidth + pad)
        }

        // Vertical: only applies to the model-rows scroll area.
        guard let rowIdx = modelRows.firstIndex(where: { $0.id == sel.rowIndex })
        else { return }
        var rowTop: CGFloat = 0
        for i in 0..<rowIdx {
            rowTop += (modelRows[i].id == sel.rowIndex)
                ? metrics.selectedRowHeight : metrics.rowHeight
        }
        let rowH = metrics.selectedRowHeight
        let visibleH = max(0, availableGridH - timingBandH)
        let curV = rowsScroll.vScrollOffsetPx
        if rowTop < curV + pad {
            rowsScroll.vScrollOffsetPx = max(0, rowTop - pad)
        } else if rowTop + rowH > curV + visibleH - pad {
            rowsScroll.vScrollOffsetPx = max(0, rowTop + rowH - visibleH + pad)
        }
    }

    /// Zoom out so the full sequence duration fits inside the available
    /// horizontal content width. Runs once per sequence load (tracked by
    /// `fitDurationMS`) so later user zoom isn't clobbered.
    private func fitIfNeeded(durationMS: Int, availableWidth: CGFloat) {
        guard durationMS > 0 else { return }
        let contentAvail = availableWidth - metrics.rowHeaderWidth
        guard contentAvail > 1 else { return }
        if fitDurationMS == durationMS { return }
        let ppms = contentAvail / CGFloat(durationMS)
        timeline.pixelsPerMS = min(max(ppms, 0.005), 2.0)
        timeline.hScrollOffsetPx = 0
        fitDurationMS = durationMS
    }

    /// B37: unconditional zoom-to-fit (ignores the load-once guard on
    /// `fitDurationMS`). Wired to the View-picker menu entry.
    private func zoomToFitSequence(availableWidth: CGFloat) {
        let durationMS = viewModel.sequenceDurationMS
        guard durationMS > 0 else { return }
        let contentAvail = availableWidth - metrics.rowHeaderWidth
        guard contentAvail > 1 else { return }
        let ppms = contentAvail / CGFloat(durationMS)
        timeline.pixelsPerMS = min(max(ppms, 0.005), 2.0)
        timeline.hScrollOffsetPx = 0
    }

    /// B36: zoom so the selected effect's range (or the union of all
    /// selected effects' ranges) fills the horizontal viewport with
    /// small margins on each side. No-op when nothing is selected or
    /// when the resulting range would clamp against the zoom limits.
    private func zoomToSelection(availableWidth: CGFloat) {
        var minStart = Int.max
        var maxEnd = Int.min
        if let single = viewModel.selectedEffect {
            minStart = single.startTimeMS
            maxEnd = single.endTimeMS
        } else {
            for sel in viewModel.selectedEffects {
                minStart = min(minStart, sel.startTimeMS)
                maxEnd = max(maxEnd, sel.endTimeMS)
            }
        }
        guard minStart < maxEnd else { return }
        let contentAvail = availableWidth - metrics.rowHeaderWidth
        guard contentAvail > 1 else { return }
        let rangeMS = maxEnd - minStart
        // Reserve ~15% margin total (7.5% each side) so selection
        // doesn't kiss the edges.
        let margin: CGFloat = 0.15
        let targetPx = contentAvail * (1 - margin)
        let ppms = targetPx / CGFloat(rangeMS)
        timeline.pixelsPerMS = min(max(ppms, 0.005), 2.0)
        let selCenterMS = CGFloat(minStart + rangeMS / 2)
        let viewCenterPx = contentAvail / 2
        timeline.hScrollOffsetPx = max(0,
            selCenterMS * timeline.pixelsPerMS - viewCenterPx)
    }

    // MARK: - Row 2: timing band

    private func timingHeaders(_ rows: [SequencerViewModel.RowInfo]) -> some View {
        VStack(spacing: 0) {
            ForEach(rows) { row in
                TimingRowHeader(
                    row: row,
                    height: metrics.timingRowHeight,
                    document: viewModel.document,
                    onRowsChanged: { viewModel.reloadRows() },
                    canBreakdownPhrases: viewModel.canBreakdownPhrases(rowIndex: row.id),
                    onBreakdownPhrases: {
                        _ = viewModel.breakdownPhrases(rowIndex: row.id)
                    },
                    canBreakdownWords: viewModel.canBreakdownWords(rowIndex: row.id),
                    onBreakdownWords: {
                        _ = viewModel.breakdownWords(rowIndex: row.id)
                    },
                    canRemoveWordsAndPhonemes: viewModel.canRemoveWordsAndPhonemes(rowIndex: row.id),
                    onRemoveWordsAndPhonemes: {
                        _ = viewModel.removeWordsAndPhonemes(rowIndex: row.id)
                    },
                    canMakeVariable: viewModel.timingTrackIsFixed(rowIndex: row.id),
                    onMakeVariable: {
                        _ = viewModel.makeTimingTrackVariable(rowIndex: row.id)
                    },
                    onSubdivide: { raw in
                        if let mode = SequencerViewModel.SubdivisionMode(rawValue: raw) {
                            _ = viewModel.generateSubdividedTimingTrack(
                                sourceRowIndex: row.id, mode: mode)
                        }
                    },
                    canSubdivide: row.layerIndex == 0 && !row.effects.isEmpty,
                    onExportTimingTrack: {
                        startXTimingExport(rowIndex: row.id,
                                             trackName: row.timing?.elementName ?? row.displayName)
                    },
                    onImportLyrics: {
                        importLyricsTargetRow = row.id
                        importLyricsText = ""
                        importLyricsStart = "0.000"
                        let endSec = Double(viewModel.sequenceDurationMS) / 1000.0
                        importLyricsEnd = String(format: "%.3f", endSec)
                    },
                    onAutoLabelMarks: {
                        autoLabelTargetRow = row.id
                        autoLabelStart = "1"
                        autoLabelEnd = "\(max(1, row.effects.count))"
                        autoLabelOverwrite = false
                    },
                    onHalveTimingMarks: {
                        _ = viewModel.halveTimingMarks(rowIndex: row.id)
                    }
                )
            }
        }
    }

    // MARK: - Row 3: model area

    /// Row-headers column for the model band. The scroll view is
    /// constrained to the natural row-content height (or the full
    /// model band's height if rows overflow); any leftover vertical
    /// space below is occupied by an invisible Color.clear that
    /// hosts the empty-area long-press menu. Doing it as a sibling
    /// of the scroll view (instead of inside its content) sidesteps
    /// the pan recogniser eating the long-press, and gives the
    /// empty area visible pixels to register the gesture in (a
    /// Color.clear inside the scroll view would be sized to zero by
    /// the bounded contentHeight).
    @ViewBuilder
    private func modelRowHeaderColumn(modelRows: [SequencerViewModel.RowInfo],
                                        modelAreaH: CGFloat,
                                        availableModelBandH: CGFloat) -> some View {
        VStack(spacing: 0) {
            SyncedScrollView(
                targetHOffset: nil,
                targetVOffset: rowsScroll.vScrollOffsetPx,
                contentWidth: metrics.rowHeaderWidth,
                contentHeight: modelAreaH,
                showsIndicators: false,
                onScroll: { newOffset in
                    rowsScroll.vScrollOffsetPx = newOffset.y
                }
            ) {
                modelHeaders(modelRows)
            }
            .frame(width: metrics.rowHeaderWidth,
                   height: min(modelAreaH, max(0, availableModelBandH)))
            if modelAreaH < availableModelBandH {
                Color.clear
                    .contentShape(Rectangle())
                    .frame(width: metrics.rowHeaderWidth)
                    .frame(maxHeight: .infinity)
                    .onLongPressGesture(minimumDuration: 0.5) {
                        emptyAreaMenuPresented = true
                    }
            }
        }
        .frame(width: metrics.rowHeaderWidth)
        .confirmationDialog("",
                             isPresented: $emptyAreaMenuPresented,
                             titleVisibility: .hidden) {
            Button {
                viewModel.showingAddTimingTrack = true
            } label: {
                Label("Add Timing Track…", systemImage: "plus.rectangle")
            }
            Button {
                viewModel.showingDisplayElements = true
            } label: {
                Label("Edit Display Elements…",
                       systemImage: "rectangle.stack.badge.plus")
            }
            Button("Cancel", role: .cancel) {}
        }
    }

    private func modelHeaders(_ rows: [SequencerViewModel.RowInfo]) -> some View {
        let selectedRowId = viewModel.selectedEffect?.rowIndex
        return VStack(spacing: 0) {
            ForEach(rows) { row in
                let h: CGFloat = (row.id == selectedRowId)
                    ? metrics.selectedRowHeight : metrics.rowHeight
                ModelRowHeader(
                    row: row,
                    height: h,
                    document: viewModel.document,
                    onSelect: { viewModel.selectPreviewModel(rowIndex: row.id) },
                    onRowsChanged: { viewModel.reloadRows() },
                    onSelectAllEffects: {
                        viewModel.selectAllEffectsInRow(rowIndex: row.id)
                    },
                    onSelectAllEffectsInModel: {
                        viewModel.selectAllEffectsInModel(rowIndex: row.id)
                    },
                    onRenameLayer: { newName in
                        _ = viewModel.renameLayer(rowIndex: row.id, name: newName)
                    },
                    effectCountOnRow: row.effects.count,
                    onDeleteAllEffectsOnRow: {
                        _ = viewModel.deleteAllEffectsOnRow(rowIndex: row.id)
                    },
                    elementRenderDisabled: viewModel.isElementRenderDisabled(rowIndex: row.id),
                    onToggleRenderDisabled: {
                        viewModel.toggleElementRenderDisabled(rowIndex: row.id)
                    },
                    onCopyRow: { viewModel.copyRow(rowIndex: row.id) },
                    onCutRow: { viewModel.cutRow(rowIndex: row.id) },
                    onCopyModel: { viewModel.copyModel(rowIndex: row.id) },
                    onCutModel: { viewModel.cutModel(rowIndex: row.id) },
                    onPaste: { viewModel.pasteAtRow(rowIndex: row.id) },
                    hasClipboard: viewModel.hasClipboard,
                    hasLoopRegion: viewModel.hasLoopRegion,
                    onExportModelFSEQ: { useLoop in
                        startFSEQExport(rowIndex: row.id, useLoopRegion: useLoop)
                    },
                    onConvertToPerModel: { allLayers in
                        viewModel.convertEffectsToPerModel(rowIndex: row.id,
                                                            allLayers: allLayers)
                    },
                    onPromoteNodeEffects: {
                        viewModel.promoteNodeEffects(rowIndex: row.id)
                    },
                    unusedLayerCount: Int(viewModel.document.unusedLayerCount(atRow: Int32(row.id))),
                    onDeleteUnusedLayers: {
                        _ = viewModel.deleteUnusedLayers(rowIndex: row.id)
                    },
                    onInsertMultipleLayersBelow: {
                        insertLayersCountText = "3"
                        insertLayersTargetRow = row.id
                    },
                    isSelected: row.id == selectedRowId
                )
            }
        }
        .frame(maxHeight: .infinity, alignment: .top)
    }

    /// Interactive Metal grid for the model-effect column. Builds
    /// the same `EffectCanvasActions` + `EffectStateLookup` the CG
    /// path constructs so the action semantics stay identical.
    private func modelEffectsMetalView(
        modelRows: [SequencerViewModel.RowInfo]
    ) -> some View {
        var actions = EffectCanvasActions()
        actions.onTapEffect = { rowIdx, effIdx in
            viewModel.selectEffect(rowIndex: rowIdx, effectIndex: effIdx)
        }
        actions.onTapEmpty = { rowIdx, ms in
            if let row = rowIdx, let atMS = ms,
               viewModel.selectedPaletteEffect != nil {
                viewModel.addEffectFromPaletteTap(rowIndex: row, atMS: atMS)
            } else {
                viewModel.clearSelection()
            }
        }
        actions.onMoveEffect = { rowIdx, effIdx, newStart, newEnd in
            viewModel.moveEffect(rowIndex: rowIdx, effectIndex: effIdx,
                                 newStartMS: newStart, newEndMS: newEnd)
        }
        actions.onMoveEffectToRow = { srcRow, effIdx, dstRow, newStart, newEnd in
            viewModel.moveEffectToRow(srcRowIndex: srcRow, effectIndex: effIdx,
                                       dstRowIndex: dstRow,
                                       newStartMS: newStart, newEndMS: newEnd)
        }
        actions.onResizeEdge = { rowIdx, effIdx, edge, newMS in
            viewModel.resizeEffectEdge(rowIndex: rowIdx, effectIndex: effIdx,
                                       edge: edge, newMS: newMS)
        }
        actions.onResizeSharedEdge = { rowIdx, lIdx, lStart, lEnd, rIdx, rStart, rEnd in
            viewModel.resizeSharedEdge(rowIndex: rowIdx,
                                        leftIndex: lIdx,
                                        leftStartMS: lStart, leftEndMS: lEnd,
                                        rightIndex: rIdx,
                                        rightStartMS: rStart, rightEndMS: rEnd)
        }
        actions.onAdjustFade = { rowIdx, effIdx, edge, seconds in
            viewModel.adjustFade(rowIndex: rowIdx, effectIndex: effIdx,
                                 fadeInSec:  edge == 0 ? seconds : -1,
                                 fadeOutSec: edge == 1 ? seconds : -1)
        }
        actions.onActiveDragChanged = { snapshot in
            viewModel.activeDrag = snapshot
        }
        actions.onPinchZoom = pinchZoomAction
        actions.onRequestContextMenu = { rowIdx, effIdx, _ in
            contextMenuTarget = ContextMenuTarget(rowIndex: rowIdx, effectIndex: effIdx)
        }
        actions.onRequestTransitionMenu = { rowIdx, effIdx, isIn, _ in
            transitionMenuTarget = TransitionMenuTarget(
                rowIndex: rowIdx, effectIndex: effIdx, isIn: isIn)
        }
        actions.onDoubleTapEmpty = { rowIdx, ms in
            viewModel.doubleTapCreateInCell(rowIndex: rowIdx, atMS: ms)
        }
        var stateLookup = EffectStateLookup()
        stateLookup.isLocked = { [document = viewModel.document] rowIdx, effIdx in
            document.effectIsLocked(inRow: Int32(rowIdx), at: Int32(effIdx))
        }
        stateLookup.isDisabled = { [document = viewModel.document] rowIdx, effIdx in
            document.effectIsRenderDisabled(inRow: Int32(rowIdx), at: Int32(effIdx))
        }
        return EffectsMetalGridView(
            rows: modelRows,
            metrics: metrics,
            pixelsPerMS: timeline.pixelsPerMS,
            selection: viewModel.selectedEffect,
            selectedEffects: viewModel.selectedEffects,
            activeDrag: viewModel.activeDrag,
            timingMarkTimesMS: collectActiveTimingMarkTimes(),
            renderedBackgroundsRevision: viewModel.renderedBackgroundsRevision,
            inspectorRevision: viewModel.inspectorRevision,
            scrollOffsetX: Binding(
                get: { timeline.hScrollOffsetPx },
                set: { timeline.hScrollOffsetPx = $0 }),
            scrollOffsetY: Binding(
                get: { rowsScroll.vScrollOffsetPx },
                set: { rowsScroll.vScrollOffsetPx = $0 }),
            actions: actions,
            stateLookup: stateLookup,
            fadeProvider: { [document = viewModel.document] rowIdx, effIdx in
                let fi = document.effectFadeInSeconds(forRow: Int32(rowIdx), at: Int32(effIdx))
                let fo = document.effectFadeOutSeconds(forRow: Int32(rowIdx), at: Int32(effIdx))
                return (fi, fo)
            },
            iconProvider: { [document = viewModel.document] name, bucket in
                var outSize: Int32 = 0
                guard let data = document.iconBGRA(
                    forEffectNamed: name,
                    desiredSize: Int32(bucket),
                    outputSize: &outSize),
                    outSize > 0
                else { return nil }
                return data
            },
            document: viewModel.document,
            onUserInteraction: { timeline.noteUserInteraction() },
            onMarqueeSelect: { hits in viewModel.setMultiSelection(hits) },
            onPencilTapAction: { viewModel.undo() }
        )
    }

    /// Gather all timing-effect start times from timing rows whose
    /// element has `GetActive() == true`. These drive the vertical
    /// guide lines across the model effects canvas.
    private func collectActiveTimingMarkTimes() -> [Int] {
        let timingIdx = (viewModel.document.timingRowIndices() as [NSNumber]).map { $0.intValue }
        var out: [Int] = []
        for idx in timingIdx {
            guard viewModel.document.timingRowIsActive(at: Int32(idx)) else { continue }
            guard let row = viewModel.rows.first(where: { $0.id == idx }) else { continue }
            for e in row.effects {
                out.append(e.startTimeMS)
                out.append(e.endTimeMS)
            }
        }
        return out
    }

}

// MARK: - Subviews whose bodies read high-churn view-model state

/// B31: compact readout for the currently-selected effect. Shows
/// name, time range, duration, and row name when one effect is
/// selected; shows "N effects selected" when multi-selected; blank
/// when idle. Isolated as a subview so its re-renders on selection
/// change stay scoped to this small label.
private struct SelectionReadout: View {
    @Environment(SequencerViewModel.self) var viewModel
    var body: some View {
        if let sel = viewModel.selectedEffect {
            let dur = sel.endTimeMS - sel.startTimeMS
            let rowName = (sel.rowIndex >= 0 && sel.rowIndex < viewModel.rows.count)
                ? viewModel.rows[sel.rowIndex].displayName
                : ""
            let description = viewModel.effectDescription(
                rowIndex: sel.rowIndex, effectIndex: sel.effectIndex)
            let base = "\(sel.name) · \(Self.ms(sel.startTimeMS))–\(Self.ms(sel.endTimeMS)) · \(Self.dur(dur)) · \(rowName)"
            if description.isEmpty {
                Text(base)
            } else {
                // B20: appended in italics-ish so the eye groups it
                // as a note, not part of the timing data.
                Text("\(base) · “\(description)”")
            }
        } else if viewModel.selectedEffects.count > 1 {
            Text("\(viewModel.selectedEffects.count) effects selected")
        } else {
            // Reserve the line so layout doesn't shift on select/deselect.
            Text(" ").hidden()
        }
    }
    private static func ms(_ m: Int) -> String {
        return String(format: "%d:%02d.%03d",
                      m / 60000, (m / 1000) % 60, m % 1000)
    }
    private static func dur(_ m: Int) -> String {
        let s = Double(m) / 1000.0
        return s >= 10 ? String(format: "%.1fs", s)
                        : String(format: "%.2fs", s)
    }
}

/// Isolated time-display subview so the main grid body doesn't re-evaluate
/// every playback tick. SwiftUI's @Observable tracks reads per-body, so
/// moving `viewModel.playPositionMS` into its own view confines the
/// invalidation to just that label.
private struct TimeDisplayLabel: View {
    @Environment(SequencerViewModel.self) var viewModel
    var body: some View {
        Text(formatTime(viewModel.playPositionMS))
    }
    private func formatTime(_ ms: Int) -> String {
        let totalSeconds = ms / 1000
        let minutes = totalSeconds / 60
        let seconds = totalSeconds % 60
        let frac = (ms % 1000) / 10
        return String(format: "%d:%02d.%02d", minutes, seconds, frac)
    }
}

/// Invisible observer that keeps the play marker on-screen during
/// playback (B93). Watches `viewModel.playPositionMS`; when the marker
/// nears the right edge of the viewport, jump-scrolls so the marker
/// sits ~10% from the left (one-viewport desktop parity). Also handles
/// the marker having wandered off the left edge (seek-backwards during
/// playback, or a sequence that wrapped).
///
/// Suppressed for 1.2 s after the user last touched any of the grid
/// canvases so a scroll-during-playback has time to be inspected
/// before the playhead yanks the viewport back. Suppressed outright
/// during effect-scrub (`isScrubbing`) so the scrub loop doesn't
/// reel the viewport around its narrow range.
private struct AutoFollowPlayhead: View {
    @Environment(SequencerViewModel.self) var viewModel
    let timeline: TimelineState
    let availableContentWidth: CGFloat

    private static let suppressionWindow: CFTimeInterval = 1.2
    private static let leftMarginFrac: CGFloat = 0.10

    var body: some View {
        Color.clear
            .frame(width: 0, height: 0)
            .allowsHitTesting(false)
            .onChange(of: viewModel.playPositionMS) { _, newMS in
                guard viewModel.isPlaying else { return }
                if viewModel.isScrubbing { return }
                if availableContentWidth <= 1 { return }
                let since = CACurrentMediaTime() - timeline.lastUserInteractionAt
                if since < Self.suppressionWindow { return }

                let worldX = CGFloat(newMS) * timeline.pixelsPerMS
                let cur = timeline.hScrollOffsetPx
                let leftMargin = availableContentWidth * Self.leftMarginFrac
                let rightEdge = cur + availableContentWidth
                // Marker fell off the right: jump so marker is at leftMargin.
                if worldX > rightEdge {
                    timeline.hScrollOffsetPx = max(0, worldX - leftMargin)
                }
                // Marker fell off the left: same — reseat at leftMargin.
                else if worldX < cur {
                    timeline.hScrollOffsetPx = max(0, worldX - leftMargin)
                }
            }
    }
}

/// Full-height vertical line marking the current playback position. Spans
/// ruler + waveform + timing band + effect grid — desktop parity. Only
/// visible when transport is active (play or pause); hidden during pure
/// effect-scrub so the scrub-loop cursor doesn't leak into the main view.
/// Isolated as its own view so position updates don't re-invalidate the
/// surrounding grid canvases.
private struct PlayPositionMarker: View {
    @Environment(SequencerViewModel.self) var viewModel
    let timeline: TimelineState
    let rowHeaderWidth: CGFloat
    let gridWidth: CGFloat
    let gridHeight: CGFloat

    private static let flagHalf: CGFloat = 10
    private static let flagHeight: CGFloat = 10

    var body: some View {
        let active = viewModel.isPlaying || viewModel.isPaused
        let worldX = CGFloat(viewModel.playPositionMS) * timeline.pixelsPerMS
        let visibleX = worldX - timeline.hScrollOffsetPx
        let availableW = gridWidth - rowHeaderWidth
        if active, visibleX >= 0, visibleX <= availableW {
            // Both the line and the triangle are drawn by a single
            // Shape so their x centers align automatically — nothing
            // to offset mismatch. The shape's width is 2*flagHalf;
            // the outer offset places the shape's center exactly on
            // the play line's world position.
            PlayheadShape(flagHalf: Self.flagHalf,
                           flagHeight: Self.flagHeight)
                .fill(Color.red)
                .frame(width: Self.flagHalf * 2, height: gridHeight)
                .offset(x: rowHeaderWidth + visibleX - Self.flagHalf, y: 0)
                .allowsHitTesting(false)
        }
    }
}

/// Single shape that combines the play-head triangle flag at the top
/// and the full-height vertical line, both centered on the shape's
/// midX so there's no sub-pixel offset between them.
private struct PlayheadShape: Shape {
    let flagHalf: CGFloat
    let flagHeight: CGFloat

    func path(in rect: CGRect) -> Path {
        var p = Path()
        let cx = rect.midX
        // Triangle flag (apex pointing down).
        p.move(to: CGPoint(x: cx - flagHalf, y: 0))
        p.addLine(to: CGPoint(x: cx + flagHalf, y: 0))
        p.addLine(to: CGPoint(x: cx, y: flagHeight))
        p.closeSubpath()
        // Full-height line.
        p.addRect(CGRect(x: cx - 1, y: 0, width: 2, height: rect.height))
        return p
    }
}

/// B61 column-resize handle. 1-px visible hairline + 12-pt
/// transparent hit strip with a `.hoverEffect(.highlight)` for
/// Magic Keyboard pointer users. Horizontal drag updates the
/// bound width directly; the receiving view re-clamps.
private struct ColumnResizeHandle: View {
    /// Optional explicit height. `nil` makes the handle stretch to
    /// its parent (used in the model-rows HStack where the row
    /// area has no fixed height).
    let height: CGFloat?
    let minWidth: Double
    let maxWidth: Double
    @Binding var width: Double
    @State private var dragStartWidth: Double? = nil

    var body: some View {
        ZStack {
            Rectangle()
                .fill(Color(white: 0.25))
                .frame(width: 0.5)
            // Wider transparent hit target — a touch on a 0.5-pt
            // line is essentially impossible.
            Color.clear
                .frame(width: 12)
                .contentShape(Rectangle())
                .hoverEffect(.highlight)
                .gesture(
                    DragGesture()
                        .onChanged { value in
                            if dragStartWidth == nil { dragStartWidth = width }
                            if let start = dragStartWidth {
                                let proposed = start + Double(value.translation.width)
                                width = min(maxWidth, max(minWidth, proposed))
                            }
                        }
                        .onEnded { _ in dragStartWidth = nil }
                )
        }
        .frame(width: 12, height: height)
    }
}

/// B78 lyrics-import sheet. Multi-line text field + start/end
/// seconds. On commit, the parent view dispatches to
/// `SequencerViewModel.importLyrics(rowIndex:lyrics:startMS:endMS:)`.
private struct ImportLyricsSheet: View {
    let rowIndex: Int
    @Binding var text: String
    @Binding var startText: String
    @Binding var endText: String
    let onCommit: (_ startSec: String, _ endSec: String) -> Void
    let onCancel: () -> Void

    var body: some View {
        NavigationStack {
            VStack(alignment: .leading, spacing: 12) {
                Text("Paste lyrics below — one phrase per line. The full time range is divided evenly into phrases. Blank lines are skipped.")
                    .font(.footnote)
                    .foregroundStyle(.secondary)
                TextEditor(text: $text)
                    .font(.system(.body, design: .monospaced))
                    .frame(minHeight: 240)
                    .overlay(RoundedRectangle(cornerRadius: 6).stroke(Color.secondary.opacity(0.4)))
                HStack {
                    VStack(alignment: .leading) {
                        Text("Start (seconds)").font(.caption).foregroundStyle(.secondary)
                        TextField("0.000", text: $startText)
                            .keyboardType(.decimalPad)
                            .textFieldStyle(.roundedBorder)
                    }
                    VStack(alignment: .leading) {
                        Text("End (seconds)").font(.caption).foregroundStyle(.secondary)
                        TextField("0.000", text: $endText)
                            .keyboardType(.decimalPad)
                            .textFieldStyle(.roundedBorder)
                    }
                }
            }
            .padding()
            .navigationTitle("Import Lyrics")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { onCancel() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Import") { onCommit(startText, endText) }
                        .disabled(text.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty)
                }
            }
        }
    }
}

// MARK: - A9.1 Custom Band sheet

/// Parametric waveform band editor. Sliders are MIDI note numbers
/// (0–127) to match `AudioManager::SwitchTo(CUSTOM, lowNote,
/// highNote)`. Shows the resolved Hz range so users who think in
/// frequencies can sanity-check. Presets pick common percussion /
/// vocal ranges.
struct CustomBandSheet: View {
    @Binding var lowNote: Int
    @Binding var highNote: Int
    let onDone: () -> Void

    private func midiToHz(_ n: Int) -> Double {
        440.0 * pow(2.0, (Double(n) - 69.0) / 12.0)
    }
    private func hzLabel(_ n: Int) -> String {
        let hz = midiToHz(n)
        if hz >= 1000 { return String(format: "%.1f kHz", hz / 1000.0) }
        return String(format: "%.0f Hz", hz)
    }

    /// Name, lowNote, highNote. Note numbers are MIDI (C4 = 60).
    private static let presets: [(String, Int, Int)] = [
        ("Kick",   24, 40),   // ~32 Hz – ~105 Hz
        ("Bass",   36, 55),   // ~65 Hz – ~247 Hz
        ("Snare",  50, 74),   // ~147 Hz – ~587 Hz
        ("Vocal",  48, 84),   // ~131 Hz – ~1047 Hz
        ("Lead",   60, 96),   // ~262 Hz – ~2093 Hz
        ("Hat",    90, 120),  // ~1.5 kHz – ~8.4 kHz
    ]

    var body: some View {
        NavigationStack {
            Form {
                Section("Range") {
                    VStack(alignment: .leading, spacing: 4) {
                        HStack {
                            Text("Low").font(.caption).foregroundStyle(.secondary)
                            Spacer()
                            Text("\(lowNote)  (\(hzLabel(lowNote)))")
                                .font(.caption.monospacedDigit())
                        }
                        Slider(value: Binding(
                            get: { Double(lowNote) },
                            set: { v in
                                lowNote = min(Int(v), highNote - 1)
                            }), in: 0...126, step: 1)
                    }
                    VStack(alignment: .leading, spacing: 4) {
                        HStack {
                            Text("High").font(.caption).foregroundStyle(.secondary)
                            Spacer()
                            Text("\(highNote)  (\(hzLabel(highNote)))")
                                .font(.caption.monospacedDigit())
                        }
                        Slider(value: Binding(
                            get: { Double(highNote) },
                            set: { v in
                                highNote = max(Int(v), lowNote + 1)
                            }), in: 1...127, step: 1)
                    }
                }
                Section("Presets") {
                    ForEach(Self.presets, id: \.0) { preset in
                        Button {
                            lowNote = preset.1
                            highNote = preset.2
                        } label: {
                            HStack {
                                Text(preset.0)
                                Spacer()
                                Text("\(hzLabel(preset.1)) – \(hzLabel(preset.2))")
                                    .font(.caption.monospacedDigit())
                                    .foregroundStyle(.secondary)
                            }
                        }
                    }
                }
            }
            .navigationTitle("Custom Band")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") { onDone() }
                }
            }
        }
    }
}

// MARK: - A9 Chord preview sheet

/// Shows detected key + a sample of the chord progression so the
/// user can sanity-check before a timing track is created.
struct ChordPreviewSheet: View {
    let key: String
    let chords: [(Int, Int, String)]
    let onCommit: () -> Void
    let onCancel: () -> Void

    private var uniqueChordCount: Int {
        Set(chords.map { $0.2 }).count
    }

    /// First dozen segments in progression order — enough to see
    /// the flavour (verse → chorus) without dumping the whole song.
    private var previewRow: String {
        chords.prefix(12).map { $0.2 }.joined(separator: " → ")
    }

    var body: some View {
        NavigationStack {
            Form {
                Section {
                    HStack {
                        Text("Key")
                        Spacer()
                        Text(key.isEmpty ? "—" : key)
                            .font(.title3.bold())
                    }
                    HStack {
                        Text("Segments")
                        Spacer()
                        Text("\(chords.count)")
                            .font(.callout.monospacedDigit())
                    }
                    HStack {
                        Text("Unique chords")
                        Spacer()
                        Text("\(uniqueChordCount)")
                            .font(.callout.monospacedDigit())
                    }
                }
                if !previewRow.isEmpty {
                    Section("Opening progression") {
                        Text(previewRow)
                            .font(.callout.monospaced())
                            .lineLimit(2)
                    }
                }
                Section {
                    Text("Chord detection is a rough guide. Works best on tonal music with clean harmonic content — jazz voicings, ambiguous modal passages, and dense mixes will produce false positives. Treat it as a starting point for manual review.")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
            }
            .navigationTitle("Detected Chords")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { onCancel() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Create Timing Track") { onCommit() }
                        .disabled(chords.isEmpty)
                }
            }
        }
    }
}

// MARK: - A8 Stem install sheet

/// Prompts the user to pick an install root (show folder or a
/// media folder) for the HTDemucs stem-separation model.
struct StemInstallSheet: View {
    let roots: [String]
    let onCommit: (String) -> Void
    let onCancel: () -> Void

    @State private var selected: Int = 0

    var body: some View {
        NavigationStack {
            Form {
                Section {
                    Text("Stem separation uses an on-device ML model (HTDemucs) to split your audio into drums, bass, vocals, and other. The model is about 65 MB to download (expands to ~180 MB on disk) and runs entirely offline once installed.")
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                }
                Section("Install location") {
                    ForEach(Array(roots.enumerated()), id: \.offset) { idx, root in
                        Button {
                            selected = idx
                        } label: {
                            HStack {
                                VStack(alignment: .leading, spacing: 2) {
                                    Text(idx == 0 ? "Show folder" : "Media folder")
                                        .font(.body)
                                    Text(root)
                                        .font(.caption.monospaced())
                                        .foregroundStyle(.secondary)
                                        .lineLimit(1)
                                        .truncationMode(.middle)
                                }
                                Spacer()
                                if idx == selected {
                                    Image(systemName: "checkmark")
                                        .foregroundStyle(.tint)
                                }
                            }
                        }
                        .buttonStyle(.plain)
                    }
                }
                Section {
                    Text("Download runs over Wi-Fi only. You'll need enough free space on the chosen root.")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
            }
            .navigationTitle("Install Stem Model")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { onCancel() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Download") {
                        if selected >= 0 && selected < roots.count {
                            onCommit(roots[selected])
                        }
                    }
                    .disabled(roots.isEmpty)
                }
            }
        }
    }
}

// MARK: - A8 Stem progress sheet

/// Non-cancellable modal progress bar shown while the model is
/// downloading or the separator is running.
struct StemProgressSheet: View {
    let phase: SequencerViewModel.StemPhase
    let pct: Int

    private var title: String {
        switch phase {
        case .downloading: return "Downloading Model"
        case .separating:  return "Separating Stems"
        default:           return "Preparing Stems"
        }
    }
    private var message: String {
        switch phase {
        case .downloading: return "Fetching HTDemucs — stay on Wi-Fi."
        case .separating:  return "Running HTDemucs — drums, bass, vocals, other…"
        default:           return ""
        }
    }

    var body: some View {
        VStack(spacing: 16) {
            ProgressView(value: Double(pct) / 100.0)
                .progressViewStyle(.linear)
            Text(title).font(.headline)
            Text(message)
                .font(.callout)
                .foregroundStyle(.secondary)
                .multilineTextAlignment(.center)
            Text("\(pct)%")
                .font(.title3.monospacedDigit())
        }
        .padding(24)
        .presentationDetents([.medium])
        .interactiveDismissDisabled(true)
    }
}

// MARK: - A4 Tempo preview sheet

/// Shows the detected BPM + confidence + beat count and gives the
/// user a last chance to bail before a timing track is added.
struct TempoPreviewSheet: View {
    let bpm: Float
    let confidence: Float
    let beatCount: Int
    let onCommit: () -> Void
    let onCancel: () -> Void

    var body: some View {
        NavigationStack {
            Form {
                Section {
                    HStack {
                        Text("BPM")
                        Spacer()
                        Text(String(format: "%.1f", bpm))
                            .font(.title2.monospacedDigit().bold())
                    }
                    HStack {
                        Text("Confidence")
                        Spacer()
                        Text(String(format: "%.0f%%", confidence * 100))
                            .font(.callout.monospacedDigit())
                            .foregroundStyle(confidence > 0.4 ? .primary : .secondary)
                    }
                    HStack {
                        Text("Beats")
                        Spacer()
                        Text("\(beatCount)")
                            .font(.callout.monospacedDigit())
                    }
                } footer: {
                    if confidence < 0.3 {
                        Text("Low confidence — the audio may be too loose / free-form for a fixed BPM, or mostly ambient. Consider A2 onset detection instead.")
                            .font(.caption)
                    }
                }
            }
            .navigationTitle("Detected Tempo")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { onCancel() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Create Timing Track") { onCommit() }
                        .disabled(beatCount == 0)
                }
            }
        }
    }
}

// MARK: - A7 Sound Classify picker sheet

/// Surface the top detected sound classes with average-confidence
/// readouts. If classification hasn't been run yet, show a "Classify"
/// button that triggers it (blocking — the view model manages its own
/// busy state). Picking a class invokes `onPick` and dismisses.
struct SoundClassifyPickerSheet: View {
    let classes: [String: [Float]]
    let isBusy: Bool
    let onClassify: () -> Void
    let onPick: (String) -> Void
    let onCancel: () -> Void

    /// Friendlier label than the raw identifier (`"music.drums"` →
    /// `"Music / Drums"`). Dots and underscores become separators;
    /// leading component is title-cased.
    private func prettyName(_ id: String) -> String {
        let parts = id.replacingOccurrences(of: "_", with: " ")
            .split(separator: ".", omittingEmptySubsequences: true)
            .map { $0.capitalized }
        return parts.joined(separator: " / ")
    }

    private var sorted: [(String, Float, [Float])] {
        classes.map { (key, conf) -> (String, Float, [Float]) in
            let avg = conf.isEmpty ? 0 : conf.reduce(0, +) / Float(conf.count)
            return (key, avg, conf)
        }
        .sorted { $0.1 > $1.1 }
    }

    var body: some View {
        NavigationStack {
            Group {
                if isBusy {
                    VStack(spacing: 12) {
                        ProgressView()
                        Text("Classifying audio…")
                            .font(.caption).foregroundStyle(.secondary)
                    }
                    .frame(maxWidth: .infinity, maxHeight: .infinity)
                } else if classes.isEmpty {
                    VStack(spacing: 12) {
                        Text("Apple's built-in classifier identifies percussion, vocals, instruments, and ambient sounds in your audio. The waveform is then scaled to show only moments where the chosen class is present.")
                            .font(.footnote)
                            .foregroundStyle(.secondary)
                            .multilineTextAlignment(.center)
                            .padding(.horizontal, 24)
                        Button {
                            onClassify()
                        } label: {
                            Label("Classify", systemImage: "waveform.and.magnifyingglass")
                                .padding(.horizontal, 20).padding(.vertical, 8)
                        }
                        .buttonStyle(.borderedProminent)
                    }
                    .frame(maxWidth: .infinity, maxHeight: .infinity)
                } else {
                    Form {
                        Section("Detected classes") {
                            ForEach(sorted, id: \.0) { triple in
                                Button {
                                    onPick(triple.0)
                                } label: {
                                    HStack {
                                        Text(prettyName(triple.0))
                                        Spacer()
                                        Text(String(format: "%.0f%%", triple.1 * 100))
                                            .font(.caption.monospacedDigit())
                                            .foregroundStyle(.secondary)
                                    }
                                }
                            }
                        }
                    }
                }
            }
            .navigationTitle("Classify Audio")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { onCancel() }
                }
                if !classes.isEmpty && !isBusy {
                    ToolbarItem(placement: .confirmationAction) {
                        Button("Reclassify") { onClassify() }
                    }
                }
            }
        }
    }
}

