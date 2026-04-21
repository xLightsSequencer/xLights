import SwiftUI

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
    @State private var metrics = GridMetrics.standard
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
                        topLeftCorner
                            .frame(width: metrics.rowHeaderWidth,
                                   height: metrics.topChromeHeight)
                        Divider()
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
                            onPinchZoom: pinchZoomAction
                        )
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
                            Divider()
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
                                onPinchZoom: pinchZoomAction
                            )
                            .frame(height: timingBandH)
                        }
                        .frame(height: timingBandH)
                        Divider()
                    }

                    // Row 3 — fills remaining space
                    HStack(alignment: .top, spacing: 0) {
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
                        .frame(width: metrics.rowHeaderWidth)
                        Divider()
                        modelEffectsMetalView(modelRows: modelRows)
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
            Button("Copy") { viewModel.copySelectedEffect() }
            if viewModel.hasClipboard {
                Button("Paste Here") {
                    let startMS = (viewModel.rows[target.rowIndex].effects[target.effectIndex]).startTimeMS
                    viewModel.pasteEffect(rowIndex: target.rowIndex, startMS: startMS)
                }
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
            Button("Delete", role: .destructive) {
                viewModel.deleteEffect(rowIndex: target.rowIndex,
                                       effectIndex: target.effectIndex)
            }
            Button("Cancel", role: .cancel) {}
        }
    }

    // MARK: - Row 1: view/time corner + top chrome

    private var topLeftCorner: some View {
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
                } label: {
                    HStack(spacing: 2) {
                        Text(currentName)
                            .font(.caption).fontWeight(.medium)
                            .foregroundStyle(.white)
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
            Spacer()
        }
        .padding(6)
        .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topLeading)
        .background(Color(white: 0.12))
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

    // MARK: - Row 2: timing band

    private func timingHeaders(_ rows: [SequencerViewModel.RowInfo]) -> some View {
        VStack(spacing: 0) {
            ForEach(rows) { row in
                TimingRowHeader(
                    row: row,
                    height: metrics.timingRowHeight,
                    document: viewModel.document,
                    onRowsChanged: { viewModel.reloadRows() }
                )
            }
        }
    }

    // MARK: - Row 3: model area

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
                    onRowsChanged: { viewModel.reloadRows() }
                )
            }
            Spacer(minLength: 0)
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
            document: viewModel.document
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

