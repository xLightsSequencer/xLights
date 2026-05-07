import SwiftUI
import UIKit
import MetalKit

/// Interactive Metal-backed timing-band canvas. Renders the
/// timing marks per element (alternating stripes), handles
/// tap-to-seek with snap-to-mark, and relays pinch-to-zoom to the
/// shared pinch action.
struct TimingEffectsMetalGridView: UIViewRepresentable {
    let rows: [SequencerViewModel.RowInfo]
    let rowHeight: CGFloat
    let pixelsPerMS: CGFloat
    @Binding var scrollOffsetX: CGFloat
    @Binding var scrollOffsetY: CGFloat
    let onSeek: (Int) -> Void
    let onPinchZoom: (CGFloat, CGFloat) -> Void
    var onUserInteraction: (() -> Void)?
    /// Fired on long-press in the timing band. `rowIndex` is the
    /// timing row hit (in `rows`), `markIndex` is non-nil when the
    /// press landed on an existing mark, `ms` is the touch's time
    /// when it was on empty space. The outer view decides which
    /// menu (delete-mark vs add-mark) to present.
    var onLongPressMark: ((_ rowIndex: Int, _ markIndex: Int?, _ ms: Int) -> Void)?
    /// B68: called when a drag of a timing mark completes. The view
    /// model should move the mark via its existing `moveEffect` path.
    var onMarkDragEnd: ((_ rowIndex: Int, _ markIndex: Int,
                          _ newStartMS: Int, _ newEndMS: Int) -> Void)?
    /// B92: fired on double-tap over an existing timing mark.
    /// Carries the global row id + mark index so the outer view can
    /// set the loop region to the mark range and start playback.
    var onDoubleTapMark: ((_ rowIndex: Int, _ markIndex: Int) -> Void)?

    func makeUIView(context: Context) -> TimingEffectsMetalMTKView {
        let v = TimingEffectsMetalMTKView()
        v.coordinator = context.coordinator
        v.installGestures()
        return v
    }

    func updateUIView(_ view: TimingEffectsMetalMTKView, context: Context) {
        let c = context.coordinator
        c.rows = rows
        c.rowHeight = rowHeight
        c.pixelsPerMS = pixelsPerMS
        c.scrollOffsetX = scrollOffsetX
        c.scrollOffsetY = scrollOffsetY
        c.onSeek = onSeek
        c.onPinchZoom = onPinchZoom
        c.onUpdateScrollX = { scrollOffsetX = $0 }
        c.onUpdateScrollY = { scrollOffsetY = $0 }
        c.onUserInteraction = onUserInteraction
        c.onLongPressMark = onLongPressMark
        c.onMarkDragEnd = onMarkDragEnd
        c.onDoubleTapMark = onDoubleTapMark
        view.setNeedsDisplay()
    }

    func makeCoordinator() -> Coordinator { Coordinator() }

    final class Coordinator {
        let bridge = XLGridMetalBridge(name: "TimingEffectsGrid")
        var rows: [SequencerViewModel.RowInfo] = []
        var rowHeight: CGFloat = 24
        var pixelsPerMS: CGFloat = 0.1
        var scrollOffsetX: CGFloat = 0
        var scrollOffsetY: CGFloat = 0
        var onSeek: (Int) -> Void = { _ in }
        var onPinchZoom: (CGFloat, CGFloat) -> Void = { _, _ in }
        var onUpdateScrollX: (CGFloat) -> Void = { _ in }
        var onUpdateScrollY: (CGFloat) -> Void = { _ in }
        var onUserInteraction: (() -> Void)?
        var onLongPressMark: ((Int, Int?, Int) -> Void)?
        var onMarkDragEnd: ((Int, Int, Int, Int) -> Void)?
        var onDoubleTapMark: ((Int, Int) -> Void)?
        var panStartScrollX: CGFloat = 0
        var panStartScrollY: CGFloat = 0

        // Apple Pencil precision flag — set by the MTKView's
        // `touchesBegan` override. When true, mark-edge hit slop
        // halves so the Pencil can grab narrow timing-mark edges
        // directly without the finger's forgiveness budget.
        var pencilActive: Bool = false

        // B68 live drag state for a timing mark. When set, draw
        // renders the mark at `liveStartMS…liveEndMS` instead of
        // its stored position.
        enum MarkDragKind { case move, resizeLeft, resizeRight }
        struct MarkDrag {
            let rowIndex: Int      // global row id (into viewModel.rows)
            let rowLocal: Int      // local index into this canvas's rows
            let markIndex: Int
            let kind: MarkDragKind
            let origStartMS: Int
            let origEndMS: Int
            let minStartMS: Int    // clamp: previous mark's end (or 0)
            let maxEndMS: Int      // clamp: next mark's start (or duration)
        }
        var markDrag: MarkDrag?
        var liveMarkStartMS: Int?
        var liveMarkEndMS: Int?
    }
}

final class TimingEffectsMetalMTKView: MTKView, MTKViewDelegate {
    weak var coordinator: TimingEffectsMetalGridView.Coordinator?
    private var layerAttached = false

    override init(frame: CGRect, device: MTLDevice?) {
        super.init(frame: frame, device: device ?? MTLCreateSystemDefaultDevice())
        common()
    }
    required init(coder: NSCoder) {
        super.init(coder: coder)
        common()
    }
    private func common() {
        self.colorPixelFormat = .bgra8Unorm
        self.clearColor = MTLClearColorMake(0, 0, 0, 0)
        self.isPaused = true
        self.enableSetNeedsDisplay = true
        self.isOpaque = false
        self.delegate = self
        if let layer = self.layer as? CAMetalLayer {
            layer.isOpaque = false
        }
    }

    override func layoutSubviews() {
        super.layoutSubviews()
        guard let c = coordinator else { return }
        if !layerAttached, let layer = self.layer as? CAMetalLayer {
            c.bridge.attach(layer)
            layerAttached = true
        }
        c.bridge.setDrawableSize(drawableSize, scale: contentScaleFactor)
    }

    // Apple Pencil tracking — mirrors the effect grid. Sets
    // `coordinator.pencilActive` so hit tests in
    // `startMarkDrag` / `onLongPress` tighten their slop.
    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
        if touches.contains(where: { $0.type == .pencil }) {
            coordinator?.pencilActive = true
        }
        super.touchesBegan(touches, with: event)
    }
    override func touchesEnded(_ touches: Set<UITouch>, with event: UIEvent?) {
        if touches.contains(where: { $0.type == .pencil }) {
            coordinator?.pencilActive = false
        }
        super.touchesEnded(touches, with: event)
    }
    override func touchesCancelled(_ touches: Set<UITouch>, with event: UIEvent?) {
        if touches.contains(where: { $0.type == .pencil }) {
            coordinator?.pencilActive = false
        }
        super.touchesCancelled(touches, with: event)
    }

    func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {
        coordinator?.bridge.setDrawableSize(size, scale: view.contentScaleFactor)
    }

    func draw(in view: MTKView) {
        guard let c = coordinator else { return }
        guard c.bridge.beginFrame() else { return }
        let size = bounds.size
        drawAll(bridge: c.bridge, viewport: size, c: c)
        c.bridge.endFrame()
    }

    private func drawAll(bridge: XLGridMetalBridge, viewport: CGSize,
                          c: TimingEffectsMetalGridView.Coordinator) {
        var elementOrder: [String: Int] = [:]
        for row in c.rows {
            if let n = row.timing?.elementName, !n.isEmpty,
               elementOrder[n] == nil {
                elementOrder[n] = elementOrder.count
            }
        }
        bridge.beginFilledRectBatch()
        var y: CGFloat = -c.scrollOffsetY
        for (rIdx, row) in c.rows.enumerated() {
            if y + c.rowHeight < 0 { y += c.rowHeight; continue }
            if y > viewport.height { break }
            let elementIdx: Int
            if let n = row.timing?.elementName, let idx = elementOrder[n] {
                elementIdx = idx
            } else {
                elementIdx = rIdx
            }
            let isEven = (elementIdx % 2 == 0)
            let r = isEven ? 0.128 : 0.078
            let g = isEven ? 0.213 : 0.143
            let b = isEven ? 0.128 : 0.078
            bridge.appendFilledRectX(0, y: y, w: viewport.width, h: c.rowHeight,
                                      r: r, g: g, b: b, a: 1.0)
            y += c.rowHeight
        }
        bridge.flushFilledRectBatch()

        // B88: detect which timing elements expose multiple layers
        // (Papagayo-style phrase/word/phoneme). The first-layer row
        // of such an element uses the "Phrases" label color; plain
        // single-layer timing tracks keep the default white label.
        var elementIsMultiLayer: [String: Bool] = [:]
        for row in c.rows {
            if let t = row.timing, !t.elementName.isEmpty {
                if let cur = elementIsMultiLayer[t.elementName] {
                    if !cur { elementIsMultiLayer[t.elementName] = true }
                } else {
                    // Seed with "single-layer" on the first sighting;
                    // bump to true the moment any subsequent layer of
                    // this element shows up.
                    elementIsMultiLayer[t.elementName] = false
                }
            }
        }
        // Second pass: flip to true for any element that has >1 row.
        var elementLayerRows: [String: Int] = [:]
        for row in c.rows {
            if let n = row.timing?.elementName, !n.isEmpty {
                elementLayerRows[n, default: 0] += 1
            }
        }
        for (n, count) in elementLayerRows where count > 1 {
            elementIsMultiLayer[n] = true
        }

        // Brackets.
        bridge.beginLineBatch()
        struct Label {
            let x: CGFloat
            let y: CGFloat
            let text: String
            /// nil → draw as white-text no-background (plain timing
            /// tracks). Non-nil → Papagayo layer color used as
            /// label background; text renders black.
            let bg: (CGFloat, CGFloat, CGFloat)?
        }
        var labels: [Label] = []
        y = -c.scrollOffsetY
        for (rIdx, row) in c.rows.enumerated() {
            _ = rIdx
            if y + c.rowHeight < 0 { y += c.rowHeight; continue }
            if y > viewport.height { break }
            // Pick label background per the effect's row type.
            // Matches desktop EffectsGrid.cpp:7032-7039:
            //   layer 0 + multi-layer element → COLOR_PHRASES
            //   layer 1 → COLOR_WORDS
            //   layer 2 → COLOR_PHONEMES
            //   else → plain white label
            let bg: (CGFloat, CGFloat, CGFloat)?
            if let t = row.timing {
                if t.layerName == "Phrases" || (row.layerIndex == 0 && (elementIsMultiLayer[t.elementName] ?? false)) {
                    bg = (153.0/255, 255.0/255, 153.0/255) // COLOR_PHRASES
                } else if t.layerName == "Words" || row.layerIndex == 1 {
                    bg = (255.0/255, 218.0/255, 145.0/255) // COLOR_WORDS
                } else if t.layerName == "Phonemes" || row.layerIndex == 2 {
                    bg = (255.0/255, 181.0/255, 218.0/255) // COLOR_PHONEMES
                } else {
                    bg = nil
                }
            } else {
                bg = nil
            }
            for (eIdx, effect) in row.effects.enumerated() {
                let isDragged = (c.markDrag?.rowIndex == row.id
                                  && c.markDrag?.markIndex == eIdx)
                let startMS = isDragged ? (c.liveMarkStartMS ?? effect.startTimeMS)
                                         : effect.startTimeMS
                let endMS = isDragged ? (c.liveMarkEndMS ?? effect.endTimeMS)
                                       : effect.endTimeMS
                let x1 = CGFloat(startMS) * c.pixelsPerMS - c.scrollOffsetX
                let x2 = CGFloat(endMS) * c.pixelsPerMS - c.scrollOffsetX
                if x2 < 0 || x1 > viewport.width { continue }
                // Brighter during drag for live-feedback; otherwise
                // the usual soft-white stroke.
                let a: CGFloat = isDragged ? 1.0 : 0.75
                let rr: CGFloat = isDragged ? 1.0 : 1.0
                let gg: CGFloat = isDragged ? 0.85 : 1.0
                let bb: CGFloat = isDragged ? 0.25 : 1.0
                bridge.appendLineX1(x1 + 0.5, y1: y + 1,
                                     x2: x1 + 0.5, y2: y + c.rowHeight - 1,
                                     r: rr, g: gg, b: bb, a: a)
                bridge.appendLineX1(x2 - 0.5, y1: y + 1,
                                     x2: x2 - 0.5, y2: y + c.rowHeight - 1,
                                     r: rr, g: gg, b: bb, a: a)
                if !effect.name.isEmpty {
                    let cx = (x1 + x2) / 2
                    labels.append(Label(x: cx,
                                         y: y + (c.rowHeight - 11) / 2,
                                         text: effect.name,
                                         bg: bg))
                }
            }
            y += c.rowHeight
        }
        bridge.flushLineBatch()

        // Label background rects first (so text renders on top).
        let anyBg = labels.contains { $0.bg != nil }
        if anyBg {
            bridge.beginFilledRectBatch()
            for l in labels {
                guard let b = l.bg else { continue }
                let s = bridge.size(ofText: l.text, fontSize: 9)
                if s.width <= 0 { continue }
                let pad: CGFloat = 3
                bridge.appendFilledRectX(l.x - s.width / 2 - pad,
                                          y: l.y - 2,
                                          w: s.width + pad * 2,
                                          h: s.height + 4,
                                          r: b.0, g: b.1, b: b.2, a: 1.0)
            }
            bridge.flushFilledRectBatch()
            // Thin outline around each colored label — matches
            // desktop's COLOR_LABEL_OUTLINE at (103,103,103).
            bridge.beginLineBatch()
            for l in labels {
                guard l.bg != nil else { continue }
                let s = bridge.size(ofText: l.text, fontSize: 9)
                if s.width <= 0 { continue }
                let pad: CGFloat = 3
                let x1 = l.x - s.width / 2 - pad
                let x2 = x1 + s.width + pad * 2
                let y1 = l.y - 2
                let y2 = y1 + s.height + 4
                let or_: CGFloat = 103.0/255
                bridge.appendLineX1(x1, y1: y1, x2: x2, y2: y1,
                                     r: or_, g: or_, b: or_, a: 1.0)
                bridge.appendLineX1(x2, y1: y1, x2: x2, y2: y2,
                                     r: or_, g: or_, b: or_, a: 1.0)
                bridge.appendLineX1(x2, y1: y2, x2: x1, y2: y2,
                                     r: or_, g: or_, b: or_, a: 1.0)
                bridge.appendLineX1(x1, y1: y2, x2: x1, y2: y1,
                                     r: or_, g: or_, b: or_, a: 1.0)
            }
            bridge.flushLineBatch()
        }

        for l in labels {
            let s = bridge.size(ofText: l.text, fontSize: 9)
            if s.width <= 0 { continue }
            if l.bg != nil {
                // Dark text on the pale Papagayo background.
                bridge.drawText(l.text, atX: l.x - s.width / 2, y: l.y,
                                 fontSize: 9, r: 0, g: 0, b: 0, a: 1.0)
            } else {
                bridge.drawText(l.text, atX: l.x - s.width / 2, y: l.y,
                                 fontSize: 9, r: 1, g: 1, b: 1, a: 0.75)
            }
        }
    }

    func installGestures() {
        let tap = UITapGestureRecognizer(target: self, action: #selector(onTap(_:)))
        addGestureRecognizer(tap)
        // B92: double-tap a mark to set the loop region to that
        // mark's range + start playback. Attach before the single
        // tap so `require(toFail:)` defers the seek.
        let doubleTap = UITapGestureRecognizer(target: self,
                                                 action: #selector(onDoubleTap(_:)))
        doubleTap.numberOfTapsRequired = 2
        addGestureRecognizer(doubleTap)
        tap.require(toFail: doubleTap)
        let pan = UIPanGestureRecognizer(target: self, action: #selector(onPan(_:)))
        pan.allowedScrollTypesMask = .all   // B95: trackpad + scroll-wheel scroll
        addGestureRecognizer(pan)
        let pinch = UIPinchGestureRecognizer(target: self, action: #selector(onPinch(_:)))
        addGestureRecognizer(pinch)
        // B67 / B69: long-press brings up the mark context menu.
        // Resolved hit is either an existing mark (→ Delete) or
        // empty space (→ Add Mark Here).
        let lp = UILongPressGestureRecognizer(target: self,
                                                action: #selector(onLongPress(_:)))
        lp.minimumPressDuration = 0.5
        lp.allowableMovement = 6
        addGestureRecognizer(lp)
    }

    @objc func onDoubleTap(_ g: UITapGestureRecognizer) {
        guard let c = coordinator, c.pixelsPerMS > 0 else { return }
        let p = g.location(in: self)
        var y: CGFloat = -c.scrollOffsetY
        var rowIdx = -1
        for (i, _) in c.rows.enumerated() {
            if p.y >= y && p.y < y + c.rowHeight { rowIdx = i; break }
            y += c.rowHeight
        }
        guard rowIdx >= 0 else { return }
        let row = c.rows[rowIdx]
        let ms = max(0, Int((p.x + c.scrollOffsetX) / c.pixelsPerMS))
        for (mi, m) in row.effects.enumerated() {
            if ms >= m.startTimeMS && ms <= m.endTimeMS {
                c.onDoubleTapMark?(row.id, mi)
                return
            }
        }
    }

    @objc func onLongPress(_ g: UILongPressGestureRecognizer) {
        guard g.state == .began, let c = coordinator, c.pixelsPerMS > 0 else { return }
        let p = g.location(in: self)
        // Find the row.
        var y: CGFloat = -c.scrollOffsetY
        var rowIdx = -1
        for (i, _) in c.rows.enumerated() {
            if p.y >= y && p.y < y + c.rowHeight {
                rowIdx = i; break
            }
            y += c.rowHeight
        }
        guard rowIdx >= 0 else { return }
        let row = c.rows[rowIdx]
        let ms = max(0, Int((p.x + c.scrollOffsetX) / c.pixelsPerMS))
        var markIdx: Int? = nil
        for (mi, m) in row.effects.enumerated() {
            if ms >= m.startTimeMS && ms <= m.endTimeMS { markIdx = mi; break }
        }
        // Pass the row's global id (its `viewModel.rows` index) rather
        // than the local timing-band index, so the view model can
        // look up via `rows[id]` without reversing any filters.
        c.onLongPressMark?(row.id, markIdx, ms)
    }

    @objc func onTap(_ g: UITapGestureRecognizer) {
        guard let c = coordinator, c.pixelsPerMS > 0 else { return }
        let p = g.location(in: self)
        let ms = max(0, Int((p.x + c.scrollOffsetX) / c.pixelsPerMS))
        // Snap to nearest mark within ~8 px.
        let thresh = Int(8.0 / c.pixelsPerMS)
        var best = ms
        var bestD = thresh + 1
        for row in c.rows {
            for e in row.effects {
                for cand in [e.startTimeMS, e.endTimeMS] {
                    let d = abs(cand - ms)
                    if d < bestD { bestD = d; best = cand }
                }
            }
        }
        c.onSeek(best)
    }

    @objc func onPan(_ g: UIPanGestureRecognizer) {
        guard let c = coordinator else { return }
        switch g.state {
        case .began:
            let p = g.location(in: self)
            if let drag = startMarkDrag(at: p, c: c) {
                c.markDrag = drag
                c.liveMarkStartMS = drag.origStartMS
                c.liveMarkEndMS = drag.origEndMS
                setNeedsDisplay()
            } else {
                c.markDrag = nil
                c.panStartScrollX = c.scrollOffsetX
                c.panStartScrollY = c.scrollOffsetY
            }
            c.onUserInteraction?()
        case .changed:
            if let drag = c.markDrag {
                updateMarkDrag(g: g, drag: drag, c: c)
            } else {
                let t = g.translation(in: self)
                c.onUpdateScrollX(max(0, c.panStartScrollX - t.x))
                c.onUpdateScrollY(max(0, c.panStartScrollY - t.y))
            }
            c.onUserInteraction?()
        case .ended:
            if let drag = c.markDrag,
               let ls = c.liveMarkStartMS, let le = c.liveMarkEndMS {
                if ls != drag.origStartMS || le != drag.origEndMS {
                    c.onMarkDragEnd?(drag.rowIndex, drag.markIndex, ls, le)
                }
            }
            c.markDrag = nil
            c.liveMarkStartMS = nil
            c.liveMarkEndMS = nil
            setNeedsDisplay()
        case .cancelled, .failed:
            c.markDrag = nil
            c.liveMarkStartMS = nil
            c.liveMarkEndMS = nil
            setNeedsDisplay()
        default:
            break
        }
    }

    /// Hit-test a touch point against the rendered timing marks.
    /// Returns a `MarkDrag` whose kind is `.resizeLeft` / `.resizeRight`
    /// when the press lands within ~10 px of an edge, `.move` when it
    /// lands in the middle of a mark, or nil when it lands on empty
    /// space (→ fall through to scroll). Min/max bounds are the
    /// neighboring marks' positions so the drag can't plow into them.
    private func startMarkDrag(at p: CGPoint,
                                c: TimingEffectsMetalGridView.Coordinator)
        -> TimingEffectsMetalGridView.Coordinator.MarkDrag? {
        guard c.pixelsPerMS > 0 else { return nil }
        var y: CGFloat = -c.scrollOffsetY
        var rowLocal = -1
        for (i, _) in c.rows.enumerated() {
            if p.y >= y && p.y < y + c.rowHeight {
                rowLocal = i; break
            }
            y += c.rowHeight
        }
        guard rowLocal >= 0 else { return nil }
        let row = c.rows[rowLocal]
        let ms = Int((p.x + c.scrollOffsetX) / c.pixelsPerMS)
        // Keep 10 px slop for both finger and Pencil — narrower
        // values broke edge hit detection in practice.
        let edgeSlopMS = Int(10 / c.pixelsPerMS)
        for (mIdx, m) in row.effects.enumerated() {
            if ms < m.startTimeMS - edgeSlopMS { break }
            if ms > m.endTimeMS + edgeSlopMS { continue }
            let prevEnd = mIdx > 0 ? row.effects[mIdx - 1].endTimeMS : 0
            let nextStart = mIdx + 1 < row.effects.count
                ? row.effects[mIdx + 1].startTimeMS : Int.max
            let kind: TimingEffectsMetalGridView.Coordinator.MarkDragKind
            if abs(ms - m.startTimeMS) <= edgeSlopMS && ms < m.startTimeMS + (m.endTimeMS - m.startTimeMS) / 2 {
                kind = .resizeLeft
            } else if abs(ms - m.endTimeMS) <= edgeSlopMS && ms > m.startTimeMS + (m.endTimeMS - m.startTimeMS) / 2 {
                kind = .resizeRight
            } else if ms >= m.startTimeMS && ms <= m.endTimeMS {
                kind = .move
            } else {
                return nil
            }
            return .init(rowIndex: row.id, rowLocal: rowLocal,
                          markIndex: mIdx, kind: kind,
                          origStartMS: m.startTimeMS, origEndMS: m.endTimeMS,
                          minStartMS: prevEnd, maxEndMS: nextStart)
        }
        return nil
    }

    private func updateMarkDrag(g: UIPanGestureRecognizer,
                                 drag: TimingEffectsMetalGridView.Coordinator.MarkDrag,
                                 c: TimingEffectsMetalGridView.Coordinator) {
        let tx = g.translation(in: self).x
        let dMS = Int(tx / c.pixelsPerMS)
        let duration = drag.origEndMS - drag.origStartMS
        var newStart = drag.origStartMS
        var newEnd = drag.origEndMS
        switch drag.kind {
        case .move:
            newStart = max(drag.minStartMS,
                           min(drag.maxEndMS - duration, drag.origStartMS + dMS))
            newEnd = newStart + duration
        case .resizeLeft:
            // Keep the right edge put; min start = prev-mark end; max
            // start = one ms before current end.
            newStart = max(drag.minStartMS,
                           min(drag.origEndMS - 1, drag.origStartMS + dMS))
            newEnd = drag.origEndMS
        case .resizeRight:
            newStart = drag.origStartMS
            newEnd = min(drag.maxEndMS,
                         max(drag.origStartMS + 1, drag.origEndMS + dMS))
        }
        // Snap to the NEAREST other-mark edge within 10 px, preserving
        // the drag semantic (move snaps whichever edge is nearer +
        // preserves duration; resize snaps just the dragged edge).
        let snapCandidates = markEdgeMSCandidates(skipRowIndex: drag.rowIndex,
                                                    skipMarkIndex: drag.markIndex,
                                                    c: c)
        if let snapStart = snap(value: drag.kind == .resizeRight ? newEnd : newStart,
                                 candidates: snapCandidates, pxThreshold: 10, c: c) {
            switch drag.kind {
            case .move:
                // Determine which edge is closer
                let snapEnd = snap(value: newEnd, candidates: snapCandidates,
                                    pxThreshold: 10, c: c)
                let dStart = abs(snapStart - newStart)
                let dEnd = snapEnd.map { abs($0 - newEnd) } ?? Int.max
                if dStart <= dEnd {
                    newStart = max(drag.minStartMS,
                                   min(drag.maxEndMS - duration, snapStart))
                    newEnd = newStart + duration
                } else if let se = snapEnd {
                    newEnd = se
                    newStart = max(drag.minStartMS,
                                   min(drag.maxEndMS - duration, se - duration))
                }
            case .resizeLeft:
                newStart = max(drag.minStartMS,
                               min(drag.origEndMS - 1, snapStart))
            case .resizeRight:
                break
            }
        }
        if drag.kind == .resizeRight,
           let snapEnd = snap(value: newEnd, candidates: snapCandidates,
                               pxThreshold: 10, c: c) {
            newEnd = min(drag.maxEndMS,
                         max(drag.origStartMS + 1, snapEnd))
        }
        c.liveMarkStartMS = newStart
        c.liveMarkEndMS = newEnd
        setNeedsDisplay()
    }

    /// Gather every other-mark edge time on every timing row — used
    /// as the snap source. Skips the mark currently being dragged.
    private func markEdgeMSCandidates(skipRowIndex: Int,
                                       skipMarkIndex: Int,
                                       c: TimingEffectsMetalGridView.Coordinator)
        -> [Int] {
        var out: [Int] = []
        for row in c.rows {
            for (mi, m) in row.effects.enumerated() {
                if row.id == skipRowIndex && mi == skipMarkIndex { continue }
                out.append(m.startTimeMS)
                out.append(m.endTimeMS)
            }
        }
        return out
    }

    private func snap(value: Int, candidates: [Int],
                       pxThreshold: CGFloat,
                       c: TimingEffectsMetalGridView.Coordinator) -> Int? {
        guard c.pixelsPerMS > 0, !candidates.isEmpty else { return nil }
        let thresh = Int(pxThreshold / c.pixelsPerMS)
        var best: Int?
        var bestD = thresh + 1
        for cand in candidates {
            let d = abs(cand - value)
            if d < bestD { bestD = d; best = cand }
        }
        return best
    }

    @objc func onPinch(_ g: UIPinchGestureRecognizer) {
        guard let c = coordinator else { return }
        switch g.state {
        case .began:
            pinchAnchorX = g.location(in: self).x + c.scrollOffsetX
            pinchLastScale = 1
            c.onUserInteraction?()
        case .changed:
            let delta = g.scale / pinchLastScale
            pinchLastScale = g.scale
            c.onPinchZoom(delta, pinchAnchorX)
            c.onUserInteraction?()
        default:
            break
        }
    }
    private var pinchAnchorX: CGFloat = 0
    private var pinchLastScale: CGFloat = 1
}
