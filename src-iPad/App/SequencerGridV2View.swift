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

            // Read duration inline from the view model instead of via a
            // @State-mirrored copy. With a mirrored copy, the first body
            // eval runs before `.onAppear` has copied the value across —
            // contentW collapses to the viewport width, the hosted Canvas
            // measures at that small size, and subsequent updates never
            // re-trigger a full re-measure from inside the scroll view's
            // UIHostingController. Reading viewModel directly means the
            // very first render already computes the correct width.
            let durationMS = viewModel.sequenceDurationMS
            let contentW = max(timeline.contentWidth(forDurationMS: durationMS),
                               geo.size.width - metrics.rowHeaderWidth)
            let availableGridH = max(geo.size.height - metrics.topChromeHeight, 1)
            let rawTimingH = CGFloat(timingRows.count) * metrics.timingRowHeight
            // Cap timing band at ~1/3 of available grid height.
            let timingBandH = min(rawTimingH, availableGridH / 3)
            let modelAreaH = max(CGFloat(modelRows.count) * metrics.rowHeight,
                                 availableGridH - timingBandH)

            ZStack(alignment: .topLeading) {
                VStack(spacing: 0) {
                    // Row 1
                    HStack(alignment: .top, spacing: 0) {
                        topLeftCorner
                            .frame(width: metrics.rowHeaderWidth,
                                   height: metrics.topChromeHeight)
                        Divider()
                        SyncedScrollView(
                            targetHOffset: timeline.hScrollOffsetPx,
                            targetVOffset: nil,
                            contentWidth: contentW,
                            contentHeight: metrics.topChromeHeight,
                            showsIndicators: false,
                            onScroll: { newOffset in
                                timeline.hScrollOffsetPx = newOffset.x
                            }
                        ) {
                            topChromeContent(contentWidth: contentW)
                        }
                        .frame(height: metrics.topChromeHeight)
                    }
                    .frame(height: metrics.topChromeHeight)
                    Divider()

                    // Row 2 — only render if there are timing tracks
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
                            SyncedScrollView(
                                targetHOffset: timeline.hScrollOffsetPx,
                                targetVOffset: timingScroll.vScrollOffsetPx,
                                contentWidth: contentW,
                                contentHeight: timingContentH,
                                showsIndicators: false,
                                onScroll: { newOffset in
                                    timeline.hScrollOffsetPx = newOffset.x
                                    timingScroll.vScrollOffsetPx = newOffset.y
                                }
                            ) {
                                timingEffectsPlaceholder(timingRows: timingRows,
                                                         contentWidth: contentW)
                            }
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
                        SyncedScrollView(
                            targetHOffset: timeline.hScrollOffsetPx,
                            targetVOffset: rowsScroll.vScrollOffsetPx,
                            contentWidth: contentW,
                            contentHeight: modelAreaH,
                            showsIndicators: true,
                            onScroll: { newOffset in
                                timeline.hScrollOffsetPx = newOffset.x
                                rowsScroll.vScrollOffsetPx = newOffset.y
                            }
                        ) {
                            modelEffectsPlaceholder(modelRows: modelRows,
                                                    contentWidth: contentW)
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

    // UIView-backed canvas for the ruler + waveform. A SwiftUI Canvas
    // inside a UIHostingController inside a UIScrollView does not render
    // reliably at multi-thousand-pixel widths, so both strips are drawn
    // in one Core Graphics pass — matching the approach used for the
    // timing band and effect grid so all three stay in sync.
    private func topChromeContent(contentWidth: CGFloat) -> some View {
        var actions = EffectCanvasActions()
        actions.onPinchZoom = pinchZoomAction
        return TopChromeCanvas(
            durationMS: viewModel.sequenceDurationMS,
            pixelsPerMS: timeline.pixelsPerMS,
            contentWidth: max(contentWidth, 1),
            rulerHeight: metrics.rulerHeight,
            waveformHeight: metrics.waveformHeight,
            hasAudio: viewModel.hasAudio,
            peaks: viewModel.hasAudio ? viewModel.waveformPeaks : [],
            actions: actions
        )
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
                    document: viewModel.document
                )
            }
        }
    }

    private func timingEffectsPlaceholder(
        timingRows: [SequencerViewModel.RowInfo],
        contentWidth: CGFloat
    ) -> some View {
        var actions = EffectCanvasActions()
        actions.onPinchZoom = pinchZoomAction
        return TimingEffectsCanvas(
            rows: timingRows,
            rowHeight: metrics.timingRowHeight,
            pixelsPerMS: timeline.pixelsPerMS,
            contentWidth: max(contentWidth, 1),
            contentHeight: CGFloat(timingRows.count) * metrics.timingRowHeight,
            actions: actions
        )
    }

    // MARK: - Row 3: model area

    private func modelHeaders(_ rows: [SequencerViewModel.RowInfo]) -> some View {
        VStack(spacing: 0) {
            ForEach(rows) { row in
                ModelRowHeader(
                    row: row,
                    height: metrics.rowHeight,
                    document: viewModel.document,
                    onSelect: { viewModel.selectPreviewModel(rowIndex: row.id) }
                )
            }
        }
    }

    private func modelEffectsPlaceholder(
        modelRows: [SequencerViewModel.RowInfo],
        contentWidth: CGFloat
    ) -> some View {
        let timingMarkTimes = collectActiveTimingMarkTimes()
        let canvasH = CGFloat(modelRows.count) * metrics.rowHeight
        var actions = EffectCanvasActions()
        actions.onTapEffect = { rowIdx, effIdx in
            viewModel.selectEffect(rowIndex: rowIdx, effectIndex: effIdx)
        }
        actions.onTapEmpty = {
            viewModel.clearSelection()
        }
        actions.onMoveEffect = { rowIdx, effIdx, newStart, newEnd in
            viewModel.moveEffect(rowIndex: rowIdx, effectIndex: effIdx,
                                 newStartMS: newStart, newEndMS: newEnd)
        }
        actions.onResizeEdge = { rowIdx, effIdx, edge, newMS in
            viewModel.resizeEffectEdge(rowIndex: rowIdx, effectIndex: effIdx,
                                       edge: edge, newMS: newMS)
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
        return ModelEffectsCanvas(
            rows: modelRows,
            metrics: metrics,
            pixelsPerMS: timeline.pixelsPerMS,
            contentWidth: max(contentWidth, 1),
            contentHeight: canvasH,
            timingMarkTimesMS: timingMarkTimes,
            selection: viewModel.selectedEffect,
            fadeProvider: { [document = viewModel.document] rowIdx, effIdx in
                let fi = document.effectFadeInSeconds(forRow: Int32(rowIdx), at: Int32(effIdx))
                let fo = document.effectFadeOutSeconds(forRow: Int32(rowIdx), at: Int32(effIdx))
                return (fi, fo)
            },
            stateLookup: stateLookup,
            actions: actions,
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

    var body: some View {
        let active = viewModel.isPlaying || viewModel.isPaused
        let worldX = CGFloat(viewModel.playPositionMS) * timeline.pixelsPerMS
        let visibleX = worldX - timeline.hScrollOffsetPx
        let availableW = gridWidth - rowHeaderWidth
        if active, visibleX >= 0, visibleX <= availableW {
            Rectangle()
                .fill(Color.red.opacity(0.85))
                .frame(width: 2, height: gridHeight)
                .offset(x: rowHeaderWidth + visibleX, y: 0)
        }
    }
}

