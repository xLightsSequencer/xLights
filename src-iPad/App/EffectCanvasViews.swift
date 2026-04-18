import SwiftUI
import UIKit

/// A plain UIView whose `draw(_:)` routes to a closure that reads the
/// tile's horizontal offset within the parent canvas. Used as one slice
/// in a manually-tiled canvas: each `CanvasTileView` is sized under
/// Metal's ~16k texture limit, so very wide canvases (e.g. a 5-minute
/// sequence zoomed to 1 px/ms) can be rendered without CATiledLayer's
/// background-thread drawing (which asserted on a CoreAnimation queue
/// for our mix of CG / UIKit-text drawing).
final class CanvasTileView: UIView {
    /// Horizontal offset, in the parent canvas's coordinate space, of
    /// this tile's left edge.
    var tileOriginX: CGFloat = 0
    /// Closure that performs the actual drawing. `worldRect` is the
    /// tile's dirty rect translated into the parent canvas's full
    /// coordinate system — identical to what the original monolithic
    /// canvases used for rect culling.
    var drawContent: (CGContext, CGRect) -> Void = { _, _ in }

    override init(frame: CGRect) {
        super.init(frame: frame)
        isOpaque = false
        backgroundColor = .clear
        contentMode = .redraw
        // The parent canvas owns all gestures.
        isUserInteractionEnabled = false
    }
    required init?(coder: NSCoder) { fatalError() }

    override func draw(_ rect: CGRect) {
        guard let cg = UIGraphicsGetCurrentContext() else { return }
        // Translate so the drawing code can keep using world-x
        // coordinates — same coord system the monolithic canvases used.
        cg.saveGState()
        cg.translateBy(x: -tileOriginX, y: 0)
        let worldRect = rect.offsetBy(dx: tileOriginX, dy: 0)
        drawContent(cg, worldRect)
        cg.restoreGState()
    }
}

/// Mark only the tiles that overlap `xRanges` as needing display, using
/// `setNeedsDisplay(_ rect:)` in each tile's own coordinate space. Rows
/// span every tile vertically, so we always invalidate the tile's full
/// height — the x-range is the only thing that narrows the dirty region.
/// Callers pass ranges in the canvas's world-x coordinate system.
private func invalidateTileRanges(
    _ tiles: [CanvasTileView],
    xRanges: [ClosedRange<CGFloat>]
) {
    guard !tiles.isEmpty, !xRanges.isEmpty else { return }
    for tile in tiles {
        let tileMin = tile.tileOriginX
        let tileMax = tileMin + tile.bounds.width
        for range in xRanges {
            let lo = max(range.lowerBound, tileMin)
            let hi = min(range.upperBound, tileMax)
            if lo > hi { continue }
            let localX = lo - tileMin
            let localW = hi - lo
            tile.setNeedsDisplay(CGRect(x: localX, y: 0,
                                        width: localW,
                                        height: tile.bounds.height))
        }
    }
}

/// Lay out / reuse a set of `CanvasTileView`s as horizontal tiles
/// covering `size.width`, each at most `tileMaxWidth` wide. Called from
/// the parent's `layoutSubviews`.
private func layoutHorizontalTiles(
    in container: UIView,
    tiles: inout [CanvasTileView],
    size: CGSize,
    tileMaxWidth: CGFloat,
    drawClosure: @escaping (CGContext, CGRect) -> Void
) {
    let w = size.width
    let h = size.height
    guard w > 0, h > 0 else {
        for t in tiles { t.removeFromSuperview() }
        tiles.removeAll()
        return
    }
    let tileCount = max(1, Int(ceil(w / tileMaxWidth)))
    while tiles.count < tileCount {
        let t = CanvasTileView()
        container.addSubview(t)
        tiles.append(t)
    }
    while tiles.count > tileCount {
        let t = tiles.removeLast()
        t.removeFromSuperview()
    }
    for (i, t) in tiles.enumerated() {
        let x = CGFloat(i) * tileMaxWidth
        let tw = min(tileMaxWidth, w - x)
        t.tileOriginX = x
        t.drawContent = drawClosure
        t.frame = CGRect(x: x, y: 0, width: tw, height: h)
        t.setNeedsDisplay()
    }
}

/// Callbacks the grid canvas delivers to SwiftUI for user gestures.
/// The canvas is gesture-aware because a single UIView is the simplest
/// place to do precise hit-testing across thousands of effects.
struct EffectCanvasActions {
    var onTapEffect: (_ row: Int, _ effect: Int) -> Void = { _,_ in }
    var onTapEmpty: () -> Void = { }
    var onMoveEffect: (_ row: Int, _ effect: Int, _ newStartMS: Int, _ newEndMS: Int) -> Void = { _,_,_,_ in }
    var onResizeEdge: (_ row: Int, _ effect: Int, _ edge: Int, _ newMS: Int) -> Void = { _,_,_,_ in }
    var onPinchZoom: (_ scaleDelta: CGFloat, _ anchorX: CGFloat) -> Void = { _,_ in }
    /// Fires when a long-press selects an effect. The canvas converts
    /// the touch location into view-space coordinates so the menu can
    /// anchor near the finger.
    var onRequestContextMenu: (_ row: Int, _ effect: Int, _ anchorInCanvas: CGPoint) -> Void = { _,_,_ in }
}

/// State provider for per-effect lock / disabled look. Keeping this as a
/// struct of closures avoids threading every flag through the view data
/// array when the underlying C++ model is the source of truth.
struct EffectStateLookup {
    var isLocked: (_ row: Int, _ effect: Int) -> Bool = { _,_ in false }
    var isDisabled: (_ row: Int, _ effect: Int) -> Bool = { _,_ in false }
}

/// UIView-backed canvas that draws the model-area effects in a single
/// pass using Core Graphics. This mirrors desktop's
/// `EffectsGrid::DrawEffects` loop (`src-ui-wx/ui/sequencer/EffectsGrid.cpp`):
/// a full repaint each tick is cheaper than managing thousands of
/// SwiftUI views. Per-effect visual spec (brackets + centerline + icon
/// + fade bars) comes from the plan's section 3 "Effect visual spec."
struct ModelEffectsCanvas: UIViewRepresentable {
    let rows: [SequencerViewModel.RowInfo]
    let metrics: GridMetrics
    let pixelsPerMS: CGFloat
    let contentWidth: CGFloat
    let contentHeight: CGFloat
    let timingMarkTimesMS: [Int]
    let selection: SequencerViewModel.EffectSelection?
    let fadeProvider: (Int, Int) -> (Float, Float) // (row, effectIdx) -> (fadeInSec, fadeOutSec)
    let stateLookup: EffectStateLookup
    let actions: EffectCanvasActions
    let document: XLSequenceDocument

    func makeUIView(context: Context) -> EffectsCanvasUIView {
        let v = EffectsCanvasUIView()
        v.isOpaque = false
        v.backgroundColor = .clear
        v.contentMode = .redraw
        v.installGestures()
        return v
    }

    func updateUIView(_ view: EffectsCanvasUIView, context: Context) {
        let newSize = CGSize(width: contentWidth, height: contentHeight)

        // Changes where the whole canvas layout shifts (row count, row
        // heights, zoom, total size, timing-mark lines) force every tile
        // to redraw. Anything more localized (rows changed but same
        // structure, selection moved) is narrowed to a dirty x-range.
        let fullInvalidate =
            view.metrics != metrics ||
            view.pixelsPerMS != pixelsPerMS ||
            view.totalSize != newSize ||
            view.timingMarkTimesMS != timingMarkTimesMS ||
            !rowStructureMatches(old: view.rows, new: rows)

        var dirtyRanges: [ClosedRange<CGFloat>] = []
        if !fullInvalidate {
            // Per-effect changes (move / resize / rename / add-without-
            // restructure). Union of old + new x-ranges for each changed
            // slot, widened by a few pixels so brackets aren't clipped.
            dirtyRanges.append(contentsOf: rowEffectDiffRanges(
                old: view.rows, new: rows, pixelsPerMS: pixelsPerMS))

            // Selection moved: redraw old + new selected effect's range.
            if view.selection != selection {
                if let s = view.selection {
                    dirtyRanges.append(selectionRange(s, pixelsPerMS: pixelsPerMS))
                }
                if let s = selection {
                    dirtyRanges.append(selectionRange(s, pixelsPerMS: pixelsPerMS))
                }
            }
        }

        let sizeChanged = view.totalSize != newSize
        view.rows = rows
        view.metrics = metrics
        view.pixelsPerMS = pixelsPerMS
        view.totalSize = newSize
        view.timingMarkTimesMS = timingMarkTimesMS
        view.selection = selection
        view.fadeProvider = fadeProvider
        view.stateLookup = stateLookup
        view.actions = actions
        view.document = document
        if sizeChanged {
            view.frame.size = newSize
            view.invalidateIntrinsicContentSize()
        }
        if fullInvalidate {
            view.setNeedsDisplay()
        } else if !dirtyRanges.isEmpty {
            view.invalidate(xRanges: dirtyRanges)
        }
    }
}

// MARK: - Row diff helpers for localized tile invalidation

/// True when the two row lists share the same row count, row IDs,
/// layer indices, and effect counts per row — i.e. only per-effect
/// fields changed. Any structural difference (row added/removed/moved
/// or an effect added/removed from a row) returns false, which forces
/// a full-canvas invalidate.
private func rowStructureMatches(
    old: [SequencerViewModel.RowInfo],
    new: [SequencerViewModel.RowInfo]
) -> Bool {
    guard old.count == new.count else { return false }
    for (o, n) in zip(old, new) {
        if o.id != n.id || o.layerIndex != n.layerIndex { return false }
        if o.effects.count != n.effects.count { return false }
    }
    return true
}

/// Per-slot diff. Assumes `rowStructureMatches` already returned true.
/// Produces one x-range per slot whose `(startMS, endMS, name)` changed.
/// Each range is the union of the old and new effect extents, widened
/// a couple of pixels so the bracket strokes aren't clipped.
private func rowEffectDiffRanges(
    old: [SequencerViewModel.RowInfo],
    new: [SequencerViewModel.RowInfo],
    pixelsPerMS: CGFloat
) -> [ClosedRange<CGFloat>] {
    var out: [ClosedRange<CGFloat>] = []
    for (oRow, nRow) in zip(old, new) {
        for (oE, nE) in zip(oRow.effects, nRow.effects) {
            if oE.startTimeMS == nE.startTimeMS
                && oE.endTimeMS == nE.endTimeMS
                && oE.name == nE.name { continue }
            let startMS = min(oE.startTimeMS, nE.startTimeMS)
            let endMS   = max(oE.endTimeMS,   nE.endTimeMS)
            let x1 = CGFloat(startMS) * pixelsPerMS - 2
            let x2 = CGFloat(endMS) * pixelsPerMS + 2
            if x2 > x1 { out.append(x1...x2) }
        }
    }
    return out
}

private func selectionRange(
    _ sel: SequencerViewModel.EffectSelection,
    pixelsPerMS: CGFloat
) -> ClosedRange<CGFloat> {
    let x1 = CGFloat(sel.startTimeMS) * pixelsPerMS - 3
    let x2 = CGFloat(sel.endTimeMS) * pixelsPerMS + 3
    return x1...x2
}

final class EffectsCanvasUIView: UIView {
    // Maximum per-tile width. Kept well under Metal's ~16k texture limit
    // (logical × contentScale must fit), giving headroom on 3x Retina.
    private static let tileMaxWidth: CGFloat = 4096
    private var tiles: [CanvasTileView] = []

    var rows: [SequencerViewModel.RowInfo] = []
    var metrics: GridMetrics = .standard
    var pixelsPerMS: CGFloat = 0.1
    var totalSize: CGSize = .zero
    var timingMarkTimesMS: [Int] = []
    var selection: SequencerViewModel.EffectSelection?
    var fadeProvider: (Int, Int) -> (Float, Float) = { _,_ in (0, 0) }
    var stateLookup: EffectStateLookup = EffectStateLookup()
    var actions: EffectCanvasActions = EffectCanvasActions()
    // Set by the Representable during updateUIView. Used when the
    // canvas asks `EffectIconCache` for an effect's bitmap.
    weak var document: XLSequenceDocument?

    // Matches `MINIMUM_EFFECT_WIDTH_FOR_SMALL_RECT` on desktop; below
    // this pixel width we skip the icon and draw only brackets + a
    // centerline dash.
    private let minIconWidth: CGFloat = 14

    /// Fetch the decoded XPM icon for this effect at the appropriate
    /// size bucket for the current display scale. The cache is keyed by
    /// (name, bucket) so zooms that stay inside one bucket reuse.
    private func effectIcon(for name: String,
                            desiredSize: CGFloat,
                            doc: XLSequenceDocument) -> CGImage? {
        let scale = window?.screen.scale ?? UIScreen.main.scale
        let bucket = EffectIconCache.bucket(forDesiredPx: desiredSize * scale)
        return EffectIconCache.shared.image(for: name, bucket: bucket, document: doc)
    }

    // Pan drag state.
    private enum DragKind { case move, resizeLeft, resizeRight }
    private struct DragState {
        let rowIndex: Int
        let effectIndex: Int
        let origStartMS: Int
        let origEndMS: Int
        let kind: DragKind
    }
    private var drag: DragState?
    // Transient overrides so dragged effects render live under the finger.
    private var liveStartMS: Int?
    private var liveEndMS: Int?

    override var intrinsicContentSize: CGSize { totalSize }

    override func layoutSubviews() {
        super.layoutSubviews()
        layoutHorizontalTiles(in: self,
                              tiles: &tiles,
                              size: bounds.size,
                              tileMaxWidth: Self.tileMaxWidth) { [weak self] cg, worldRect in
            self?.drawContent(cg: cg, worldRect: worldRect)
        }
    }

    override func setNeedsDisplay() {
        super.setNeedsDisplay()
        for t in tiles { t.setNeedsDisplay() }
    }

    /// Targeted redraw of just the tiles whose frames overlap `xRanges`
    /// (world-x coordinates). Rows span full tile height, so the y-axis
    /// isn't narrowed.
    func invalidate(xRanges: [ClosedRange<CGFloat>]) {
        invalidateTileRanges(tiles, xRanges: xRanges)
    }

    fileprivate func drawContent(cg: CGContext, worldRect rect: CGRect) {
        // Row zebra-striping.
        var y: CGFloat = 0
        for row in rows {
            let h = metrics.rowHeight
            cg.setFillColor((row.id % 2 == 0
                             ? UIColor(white: 0.08, alpha: 1)
                             : UIColor(white: 0.10, alpha: 1)).cgColor)
            cg.fill(CGRect(x: rect.minX, y: y, width: rect.width, height: h))
            y += h
        }

        // Vertical timing lines from active timing rows — draw across
        // the whole content area so they span all model rows.
        if !timingMarkTimesMS.isEmpty {
            cg.setStrokeColor(UIColor.white.withAlphaComponent(0.15).cgColor)
            cg.setLineWidth(1)
            for ms in timingMarkTimesMS {
                let x = CGFloat(ms) * pixelsPerMS
                if x < rect.minX - 1 || x > rect.maxX + 1 { continue }
                cg.move(to: CGPoint(x: x + 0.5, y: 0))
                cg.addLine(to: CGPoint(x: x + 0.5, y: totalSize.height))
            }
            cg.strokePath()
        }

        // Effects per row.
        y = 0
        for (rIdx, row) in rows.enumerated() {
            let h = metrics.rowHeight
            let top = y + 1
            let bottom = y + h - 1
            let mid = (top + bottom) / 2

            for (eIdx, effect) in row.effects.enumerated() {
                let isDragged = (drag?.rowIndex == row.id && drag?.effectIndex == eIdx)
                let startMS = isDragged ? (liveStartMS ?? effect.startTimeMS) : effect.startTimeMS
                let endMS   = isDragged ? (liveEndMS   ?? effect.endTimeMS)   : effect.endTimeMS
                let x1 = CGFloat(startMS) * pixelsPerMS
                let x2 = CGFloat(endMS) * pixelsPerMS
                if x2 < rect.minX || x1 > rect.maxX { continue }
                let isSelected = (selection?.rowIndex == row.id && selection?.effectIndex == eIdx)
                drawEffect(cg: cg,
                           x1: x1, x2: x2, top: top, bottom: bottom, mid: mid,
                           effect: effect, isSelected: isSelected,
                           rowIndex: row.id, effectIndex: eIdx)
            }
            _ = rIdx
            y += h
        }
    }

    private func drawEffect(cg: CGContext,
                            x1: CGFloat, x2: CGFloat,
                            top: CGFloat, bottom: CGFloat, mid: CGFloat,
                            effect: SequencerViewModel.EffectInfo,
                            isSelected: Bool,
                            rowIndex: Int,
                            effectIndex: Int) {
        let width = max(x2 - x1, 1)
        let isLocked = stateLookup.isLocked(rowIndex, effectIndex)
        let isDisabled = stateLookup.isDisabled(rowIndex, effectIndex)
        let strokeColor: UIColor
        if isDisabled {
            strokeColor = UIColor(white: 0.45, alpha: 1)
        } else if isSelected {
            strokeColor = UIColor(red: 1, green: 0.85, blue: 0.25, alpha: 1)
        } else if isLocked {
            strokeColor = UIColor(red: 0.65, green: 0.8, blue: 1.0, alpha: 1)
        } else {
            strokeColor = UIColor(white: 0.85, alpha: 1)
        }

        if isDisabled {
            cg.setFillColor(UIColor(white: 0.15, alpha: 0.65).cgColor)
            cg.fill(CGRect(x: x1, y: top, width: width, height: bottom - top))
        }

        cg.setStrokeColor(strokeColor.cgColor)
        cg.setLineWidth(isSelected ? 2 : 1)

        // Left + right brackets.
        cg.move(to: CGPoint(x: x1 + 0.5, y: top))
        cg.addLine(to: CGPoint(x: x1 + 0.5, y: bottom))
        cg.move(to: CGPoint(x: x2 - 0.5, y: top))
        cg.addLine(to: CGPoint(x: x2 - 0.5, y: bottom))
        cg.strokePath()

        // Icon-or-dash centerline.
        let hasIconRoom = width > minIconWidth
        let iconSize = min(max(bottom - top - 4, 8), 22)
        let iconX = (x1 + x2) / 2
        let iconLeft = iconX - iconSize / 2
        let iconRight = iconX + iconSize / 2

        cg.setStrokeColor(strokeColor.cgColor)
        cg.setLineWidth(1)
        if hasIconRoom {
            cg.move(to: CGPoint(x: x1, y: mid + 0.5))
            cg.addLine(to: CGPoint(x: iconLeft, y: mid + 0.5))
            cg.move(to: CGPoint(x: iconRight, y: mid + 0.5))
            cg.addLine(to: CGPoint(x: x2, y: mid + 0.5))
            cg.strokePath()

            let iconRect = CGRect(x: iconLeft, y: mid - iconSize / 2,
                                  width: iconSize, height: iconSize)
            if let doc = document,
               let cgImage = effectIcon(for: effect.name, desiredSize: iconSize, doc: doc) {
                // Draw the desktop's XPM-derived icon. Flip the CTM so
                // the bitmap isn't upside-down (Core Graphics default Y
                // is bottom-up; our context is top-down).
                cg.saveGState()
                cg.translateBy(x: 0, y: iconRect.maxY)
                cg.scaleBy(x: 1, y: -1)
                let drawRect = CGRect(x: iconRect.origin.x, y: 0,
                                      width: iconRect.width, height: iconRect.height)
                cg.draw(cgImage, in: drawRect)
                cg.restoreGState()
            } else {
                // Fallback for effects that didn't resolve to an icon:
                // a 3-letter abbreviation in the stroke color. Same as
                // what was shipping before the cache wired up.
                let label = String(effect.name.prefix(3)).uppercased()
                let attrs: [NSAttributedString.Key: Any] = [
                    .font: UIFont.systemFont(ofSize: iconSize * 0.55, weight: .semibold),
                    .foregroundColor: strokeColor,
                ]
                let textSize = (label as NSString).size(withAttributes: attrs)
                let textRect = CGRect(x: iconX - textSize.width / 2,
                                      y: mid - textSize.height / 2,
                                      width: textSize.width,
                                      height: textSize.height)
                (label as NSString).draw(in: textRect, withAttributes: attrs)
            }
        } else {
            // Dash through center.
            cg.move(to: CGPoint(x: x1, y: mid + 0.5))
            cg.addLine(to: CGPoint(x: x2, y: mid + 0.5))
            cg.strokePath()
        }

        // Fade-in/fade-out bars (top edge).
        let (fadeInSec, fadeOutSec) = fadeProvider(rowIndex, effectIndex)
        let fadeInPx = CGFloat(fadeInSec) * 1000 * pixelsPerMS
        let fadeOutPx = CGFloat(fadeOutSec) * 1000 * pixelsPerMS
        let barH: CGFloat = 3

        if fadeInPx > 0 || fadeOutPx > 0 {
            let inRect = CGRect(x: x1, y: top, width: min(fadeInPx, width), height: barH)
            let outRect = CGRect(x: max(x1, x2 - fadeOutPx), y: top,
                                 width: min(fadeOutPx, width), height: barH)
            cg.setFillColor(UIColor.systemGreen.withAlphaComponent(0.85).cgColor)
            cg.fill(inRect)
            cg.setFillColor(UIColor.systemRed.withAlphaComponent(0.85).cgColor)
            cg.fill(outRect)
            let overlap = inRect.intersection(outRect)
            if !overlap.isNull && overlap.width > 0 {
                cg.setFillColor(UIColor.systemYellow.withAlphaComponent(0.95).cgColor)
                cg.fill(overlap)
            }
        }

        // Lock glyph in the upper-right if the effect is locked.
        if isLocked && width > 10 {
            let glyph = "🔒" as NSString
            let glyphAttrs: [NSAttributedString.Key: Any] = [
                .font: UIFont.systemFont(ofSize: 9),
            ]
            glyph.draw(at: CGPoint(x: x2 - 12, y: top), withAttributes: glyphAttrs)
        }
    }

    // MARK: - Gestures

    func installGestures() {
        let tap = UITapGestureRecognizer(target: self, action: #selector(onTap(_:)))
        addGestureRecognizer(tap)

        let pan = UIPanGestureRecognizer(target: self, action: #selector(onPan(_:)))
        pan.maximumNumberOfTouches = 1
        pan.delegate = gestureDelegate
        addGestureRecognizer(pan)

        let pinch = UIPinchGestureRecognizer(target: self, action: #selector(onPinch(_:)))
        pinch.delegate = gestureDelegate
        addGestureRecognizer(pinch)

        let longPress = UILongPressGestureRecognizer(target: self, action: #selector(onLongPress(_:)))
        longPress.minimumPressDuration = 0.45
        longPress.allowableMovement = 10
        longPress.delegate = gestureDelegate
        addGestureRecognizer(longPress)
    }

    @objc private func onLongPress(_ g: UILongPressGestureRecognizer) {
        guard g.state == .began else { return }
        let p = g.location(in: self)
        guard let hit = hitTestEffect(at: p) else { return }
        // Select first so the context menu's actions operate on the
        // effect the user pressed.
        actions.onTapEffect(hit.rowIndex, hit.effectIndex)
        actions.onRequestContextMenu(hit.rowIndex, hit.effectIndex, p)
    }

    // Allow the outer UIScrollView's pan/pinch to run alongside ours
    // when the hit test lands in an empty area (so pan-to-scroll still
    // works). We suppress scrolling while a drag is active via the
    // canGestureStart filter below.
    private lazy var gestureDelegate: CanvasGestureDelegate = {
        let d = CanvasGestureDelegate()
        d.owner = self
        return d
    }()

    private enum HitZone { case empty, center, leftEdge, rightEdge }
    private struct Hit {
        let rowIndex: Int
        let effectIndex: Int
        let zone: HitZone
    }

    private func hitTestEffect(at p: CGPoint) -> Hit? {
        let rowH = metrics.rowHeight
        let rIdxInList = Int(p.y / rowH)
        guard rIdxInList >= 0, rIdxInList < rows.count else { return nil }
        let row = rows[rIdxInList]
        let ms = Int(p.x / pixelsPerMS)
        let slop = metrics.edgeHandleHitWidth / pixelsPerMS // ms
        for (eIdx, effect) in row.effects.enumerated() {
            if ms < effect.startTimeMS - Int(slop) { break }
            if ms > effect.endTimeMS + Int(slop) { continue }
            let isSelected = (selection?.rowIndex == row.id && selection?.effectIndex == eIdx)
            if ms >= effect.startTimeMS - Int(slop/2) && ms <= effect.startTimeMS + Int(slop/2) && isSelected {
                return Hit(rowIndex: row.id, effectIndex: eIdx, zone: .leftEdge)
            }
            if ms >= effect.endTimeMS - Int(slop/2) && ms <= effect.endTimeMS + Int(slop/2) && isSelected {
                return Hit(rowIndex: row.id, effectIndex: eIdx, zone: .rightEdge)
            }
            if ms >= effect.startTimeMS && ms <= effect.endTimeMS {
                return Hit(rowIndex: row.id, effectIndex: eIdx, zone: .center)
            }
        }
        return nil
    }

    @objc private func onTap(_ g: UITapGestureRecognizer) {
        let p = g.location(in: self)
        if let hit = hitTestEffect(at: p), hit.zone != .empty {
            actions.onTapEffect(hit.rowIndex, hit.effectIndex)
        } else {
            actions.onTapEmpty()
        }
    }

    @objc private func onPan(_ g: UIPanGestureRecognizer) {
        switch g.state {
        case .began:
            let p = g.location(in: self)
            guard let hit = hitTestEffect(at: p) else { drag = nil; return }
            guard let row = rows.first(where: { $0.id == hit.rowIndex }),
                  hit.effectIndex < row.effects.count else { return }
            let effect = row.effects[hit.effectIndex]
            let kind: DragKind
            switch hit.zone {
            case .leftEdge:  kind = .resizeLeft
            case .rightEdge: kind = .resizeRight
            default:         kind = .move
            }
            drag = DragState(rowIndex: hit.rowIndex,
                             effectIndex: hit.effectIndex,
                             origStartMS: effect.startTimeMS,
                             origEndMS: effect.endTimeMS,
                             kind: kind)
            liveStartMS = effect.startTimeMS
            liveEndMS = effect.endTimeMS
        case .changed:
            guard let d = drag else { return }
            let tx = g.translation(in: self).x
            let dMS = Int(tx / pixelsPerMS)
            let prevStart = liveStartMS ?? d.origStartMS
            let prevEnd   = liveEndMS   ?? d.origEndMS
            switch d.kind {
            case .move:
                liveStartMS = max(0, d.origStartMS + dMS)
                liveEndMS   = max(liveStartMS! + 1, d.origEndMS + dMS)
            case .resizeLeft:
                let ns = max(0, d.origStartMS + dMS)
                liveStartMS = min(ns, d.origEndMS - 1)
                liveEndMS = d.origEndMS
            case .resizeRight:
                let ne = d.origEndMS + dMS
                liveEndMS = max(ne, d.origStartMS + 1)
                liveStartMS = d.origStartMS
            }
            // Invalidate only tiles covering the effect's old + new
            // positions this frame. For a small per-frame delta this
            // usually touches 1–2 tiles, not all of them.
            let lo = min(prevStart, liveStartMS ?? prevStart)
            let hi = max(prevEnd,   liveEndMS   ?? prevEnd)
            let x1 = CGFloat(lo) * pixelsPerMS - 3
            let x2 = CGFloat(hi) * pixelsPerMS + 3
            invalidate(xRanges: [x1...x2])
        case .ended, .cancelled, .failed:
            guard let d = drag, let ls = liveStartMS, let le = liveEndMS else {
                drag = nil; return
            }
            // Clear live overrides and invalidate the final path range
            // before committing — the subsequent render-triggered
            // row reload will diff-invalidate the committed position.
            let x1 = CGFloat(min(d.origStartMS, ls)) * pixelsPerMS - 3
            let x2 = CGFloat(max(d.origEndMS, le)) * pixelsPerMS + 3
            switch d.kind {
            case .move:
                actions.onMoveEffect(d.rowIndex, d.effectIndex, ls, le)
            case .resizeLeft:
                actions.onResizeEdge(d.rowIndex, d.effectIndex, 0, ls)
            case .resizeRight:
                actions.onResizeEdge(d.rowIndex, d.effectIndex, 1, le)
            }
            drag = nil
            liveStartMS = nil
            liveEndMS = nil
            invalidate(xRanges: [x1...x2])
        default:
            break
        }
    }

    private var pinchAnchorX: CGFloat = 0
    private var pinchLastScale: CGFloat = 1

    @objc private func onPinch(_ g: UIPinchGestureRecognizer) {
        switch g.state {
        case .began:
            pinchAnchorX = g.location(in: self).x
            pinchLastScale = 1
        case .changed:
            let delta = g.scale / pinchLastScale
            pinchLastScale = g.scale
            actions.onPinchZoom(delta, pinchAnchorX)
        default:
            break
        }
    }

    func hitTestEffectPublic(at p: CGPoint) -> (Int, Int)? {
        guard let h = hitTestEffect(at: p) else { return nil }
        return (h.rowIndex, h.effectIndex)
    }

    // Lets the owning SyncedScrollView's pan still scroll when the
    // touch doesn't land on an effect. The canvas's pan takes over
    // only after onPan(.began) finds a hit.
    func canvasHasDragCandidate(at p: CGPoint) -> Bool {
        guard let hit = hitTestEffect(at: p) else { return false }
        if hit.zone == .center {
            // Only the center of an *already selected* effect starts a
            // move — this keeps the finger free to scroll otherwise.
            return selection?.rowIndex == hit.rowIndex && selection?.effectIndex == hit.effectIndex
        }
        return hit.zone == .leftEdge || hit.zone == .rightEdge
    }
}

/// Keeps the outer UIScrollView's own gestures from fighting with our
/// pan/pinch unless a real drag target is under the finger.
final class CanvasGestureDelegate: NSObject, UIGestureRecognizerDelegate {
    weak var owner: EffectsCanvasUIView?

    func gestureRecognizerShouldBegin(_ g: UIGestureRecognizer) -> Bool {
        guard let view = owner else { return true }
        if g is UIPinchGestureRecognizer { return true }
        if g is UILongPressGestureRecognizer {
            let p = g.location(in: view)
            return view.hitTestEffectPublic(at: p) != nil
        }
        if let pan = g as? UIPanGestureRecognizer {
            let p = pan.location(in: view)
            return view.canvasHasDragCandidate(at: p)
        }
        return true
    }

    func gestureRecognizer(_ g: UIGestureRecognizer,
                           shouldRecognizeSimultaneouslyWith other: UIGestureRecognizer) -> Bool {
        return true
    }
}

/// Canvas for the timing effects row. Each timing effect draws as a
/// small vertical tick across the row height plus a label if the
/// effect has a non-empty name (lyrics words, phoneme labels, etc.).
struct TimingEffectsCanvas: UIViewRepresentable {
    let rows: [SequencerViewModel.RowInfo]
    let rowHeight: CGFloat
    let pixelsPerMS: CGFloat
    let contentWidth: CGFloat
    let contentHeight: CGFloat
    var actions: EffectCanvasActions = EffectCanvasActions()

    func makeUIView(context: Context) -> TimingCanvasUIView {
        let v = TimingCanvasUIView()
        v.isOpaque = false
        v.backgroundColor = .clear
        v.contentMode = .redraw
        v.installGestures()
        return v
    }

    func updateUIView(_ view: TimingCanvasUIView, context: Context) {
        let newSize = CGSize(width: contentWidth, height: contentHeight)

        let fullInvalidate =
            view.rowHeight != rowHeight ||
            view.pixelsPerMS != pixelsPerMS ||
            view.totalSize != newSize ||
            !rowStructureMatches(old: view.rows, new: rows)

        var dirtyRanges: [ClosedRange<CGFloat>] = []
        if !fullInvalidate {
            dirtyRanges.append(contentsOf: rowEffectDiffRanges(
                old: view.rows, new: rows, pixelsPerMS: pixelsPerMS))
        }

        let sizeChanged = view.totalSize != newSize
        view.rows = rows
        view.rowHeight = rowHeight
        view.pixelsPerMS = pixelsPerMS
        view.totalSize = newSize
        view.actions = actions
        if sizeChanged {
            view.frame.size = newSize
            view.invalidateIntrinsicContentSize()
        }
        if fullInvalidate {
            view.setNeedsDisplay()
        } else if !dirtyRanges.isEmpty {
            view.invalidate(xRanges: dirtyRanges)
        }
    }
}

final class TimingCanvasUIView: UIView {
    private static let tileMaxWidth: CGFloat = 4096
    private var tiles: [CanvasTileView] = []

    var rows: [SequencerViewModel.RowInfo] = []
    var rowHeight: CGFloat = 24
    var pixelsPerMS: CGFloat = 0.1
    var totalSize: CGSize = .zero
    var actions: EffectCanvasActions = EffectCanvasActions()

    override var intrinsicContentSize: CGSize { totalSize }

    override func layoutSubviews() {
        super.layoutSubviews()
        layoutHorizontalTiles(in: self,
                              tiles: &tiles,
                              size: bounds.size,
                              tileMaxWidth: Self.tileMaxWidth) { [weak self] cg, worldRect in
            self?.drawContent(cg: cg, worldRect: worldRect)
        }
    }

    override func setNeedsDisplay() {
        super.setNeedsDisplay()
        for t in tiles { t.setNeedsDisplay() }
    }

    func invalidate(xRanges: [ClosedRange<CGFloat>]) {
        invalidateTileRanges(tiles, xRanges: xRanges)
    }

    fileprivate func drawContent(cg: CGContext, worldRect rect: CGRect) {
        var y: CGFloat = 0
        // Alternate by element (track), not by row: all layers of one
        // timing track share a stripe color, the next track gets the
        // other. Tracks with no `timing.elementName` fall back to row
        // parity so we still draw something sensible.
        let bandEven = UIColor(hue: 0.33, saturation: 0.25, brightness: 0.17, alpha: 1)
        let bandOdd  = UIColor(hue: 0.33, saturation: 0.30, brightness: 0.11, alpha: 1)
        var elementOrder: [String: Int] = [:]
        for row in rows {
            let key = row.timing?.elementName ?? ""
            if !key.isEmpty, elementOrder[key] == nil {
                elementOrder[key] = elementOrder.count
            }
        }
        let tickColor = UIColor.white.withAlphaComponent(0.75)
        let labelAttrs: [NSAttributedString.Key: Any] = [
            .font: UIFont.systemFont(ofSize: 9, weight: .regular),
            .foregroundColor: UIColor.white.withAlphaComponent(0.75),
        ]

        for (rowIdx, row) in rows.enumerated() {
            let elementIdx: Int
            if let name = row.timing?.elementName, let idx = elementOrder[name] {
                elementIdx = idx
            } else {
                elementIdx = rowIdx
            }
            let bandBg = (elementIdx % 2 == 0) ? bandEven : bandOdd
            cg.setFillColor(bandBg.cgColor)
            cg.fill(CGRect(x: rect.minX, y: y, width: rect.width, height: rowHeight))

            for effect in row.effects {
                let x1 = CGFloat(effect.startTimeMS) * pixelsPerMS
                let x2 = CGFloat(effect.endTimeMS) * pixelsPerMS
                if x2 < rect.minX || x1 > rect.maxX { continue }

                // Mark bracket (thin vertical lines at start + end).
                cg.setStrokeColor(tickColor.cgColor)
                cg.setLineWidth(1)
                cg.move(to: CGPoint(x: x1 + 0.5, y: y + 1))
                cg.addLine(to: CGPoint(x: x1 + 0.5, y: y + rowHeight - 1))
                cg.move(to: CGPoint(x: x2 - 0.5, y: y + 1))
                cg.addLine(to: CGPoint(x: x2 - 0.5, y: y + rowHeight - 1))
                cg.strokePath()

                // Label centered, if it fits.
                if !effect.name.isEmpty {
                    let labelW = (effect.name as NSString).size(withAttributes: labelAttrs).width
                    if labelW < (x2 - x1) - 4 {
                        let tx = (x1 + x2) / 2 - labelW / 2
                        let ty = y + (rowHeight - 11) / 2
                        (effect.name as NSString).draw(
                            at: CGPoint(x: tx, y: ty),
                            withAttributes: labelAttrs)
                    }
                }
            }
            y += rowHeight
        }
    }

    // MARK: - Gestures

    private var pinchAnchorX: CGFloat = 0
    private var pinchLastScale: CGFloat = 1

    func installGestures() {
        let pinch = UIPinchGestureRecognizer(target: self, action: #selector(onPinch(_:)))
        pinch.delegate = passthroughDelegate
        addGestureRecognizer(pinch)
    }

    private lazy var passthroughDelegate: PassthroughGestureDelegate = {
        PassthroughGestureDelegate()
    }()

    @objc private func onPinch(_ g: UIPinchGestureRecognizer) {
        switch g.state {
        case .began:
            pinchAnchorX = g.location(in: self).x
            pinchLastScale = 1
        case .changed:
            let delta = g.scale / pinchLastScale
            pinchLastScale = g.scale
            actions.onPinchZoom(delta, pinchAnchorX)
        default:
            break
        }
    }
}

/// Gesture delegate that allows simultaneous recognition so a canvas's
/// pinch can coexist with the outer UIScrollView's pan (panning to
/// scroll continues to work while the user is pinching).
final class PassthroughGestureDelegate: NSObject, UIGestureRecognizerDelegate {
    func gestureRecognizer(_ g: UIGestureRecognizer,
                           shouldRecognizeSimultaneouslyWith other: UIGestureRecognizer) -> Bool {
        return true
    }
}

/// Combined ruler + waveform canvas. Drawn in a single UIView so the
/// content renders reliably at the same large widths the effect grid
/// and timing band handle (the SwiftUI Canvas inside a UIHostingController
/// does not, breaking first-load display on typical multi-minute
/// sequences). Also hosts a pinch recognizer so zoom gestures work here
/// the same way they do over the grid.
struct TopChromeCanvas: UIViewRepresentable {
    let durationMS: Int
    let pixelsPerMS: CGFloat
    let contentWidth: CGFloat
    let rulerHeight: CGFloat
    let waveformHeight: CGFloat
    let hasAudio: Bool
    let peaks: [Float]
    var actions: EffectCanvasActions = EffectCanvasActions()

    func makeUIView(context: Context) -> TopChromeCanvasUIView {
        let v = TopChromeCanvasUIView()
        v.isOpaque = false
        v.backgroundColor = .clear
        v.contentMode = .redraw
        v.installGestures()
        return v
    }

    func updateUIView(_ view: TopChromeCanvasUIView, context: Context) {
        let newSize = CGSize(width: contentWidth,
                             height: rulerHeight + waveformHeight)
        let visualChanged =
            view.durationMS != durationMS ||
            view.pixelsPerMS != pixelsPerMS ||
            view.totalSize != newSize ||
            view.rulerHeight != rulerHeight ||
            view.waveformHeight != waveformHeight ||
            view.hasAudio != hasAudio ||
            view.peaks != peaks
        let sizeChanged = view.totalSize != newSize
        view.durationMS = durationMS
        view.pixelsPerMS = pixelsPerMS
        view.totalSize = newSize
        view.rulerHeight = rulerHeight
        view.waveformHeight = waveformHeight
        view.hasAudio = hasAudio
        view.peaks = peaks
        view.actions = actions
        if sizeChanged {
            view.frame.size = newSize
            view.invalidateIntrinsicContentSize()
        }
        if visualChanged {
            view.setNeedsDisplay()
        }
    }
}

final class TopChromeCanvasUIView: UIView {
    private static let tileMaxWidth: CGFloat = 4096
    private var tiles: [CanvasTileView] = []

    var durationMS: Int = 0
    var pixelsPerMS: CGFloat = 0.1
    var totalSize: CGSize = .zero
    var rulerHeight: CGFloat = 24
    var waveformHeight: CGFloat = 48
    var hasAudio: Bool = false
    var peaks: [Float] = []
    var actions: EffectCanvasActions = EffectCanvasActions()

    override var intrinsicContentSize: CGSize { totalSize }

    override func layoutSubviews() {
        super.layoutSubviews()
        layoutHorizontalTiles(in: self,
                              tiles: &tiles,
                              size: bounds.size,
                              tileMaxWidth: Self.tileMaxWidth) { [weak self] cg, worldRect in
            self?.drawContent(cg: cg, worldRect: worldRect)
        }
    }

    override func setNeedsDisplay() {
        super.setNeedsDisplay()
        for t in tiles { t.setNeedsDisplay() }
    }

    fileprivate func drawContent(cg: CGContext, worldRect rect: CGRect) {
        // Ruler strip — only the slice covered by this tile's dirty rect.
        cg.setFillColor(UIColor.black.withAlphaComponent(0.2).cgColor)
        cg.fill(CGRect(x: rect.minX, y: 0, width: rect.width, height: rulerHeight))

        drawRulerTicks(cg: cg, rect: rect)

        // Waveform strip (or placeholder background if no audio).
        let waveRect = CGRect(x: rect.minX, y: rulerHeight,
                              width: rect.width, height: waveformHeight)
        if hasAudio {
            cg.setFillColor(UIColor.black.withAlphaComponent(0.3).cgColor)
            cg.fill(waveRect)
            drawWaveform(cg: cg, in: CGRect(x: 0, y: rulerHeight,
                                            width: totalSize.width,
                                            height: waveformHeight),
                         rect: rect)
        } else {
            cg.setFillColor(UIColor(white: 0.08, alpha: 1).cgColor)
            cg.fill(waveRect)
        }
    }

    // Matches desktop: label every major tick, minor tick at half the
    // major interval. Major spacing targets ~100 pixels so labels don't
    // collide. Intervals are picked from a table of "nice" ms values
    // that each divide cleanly in half, so minor ticks always fall on a
    // round time value (avoiding artifacts like 12.5ms half-steps).
    private static let niceMajorIntervalsMS: [Int] = [
        10, 20, 50, 100, 200, 500,
        1000, 2000, 5000, 10000, 20000, 30000, 60000,
        120000, 300000, 600000, 1200000, 3600000
    ]

    private func majorIntervalMS() -> Int {
        let targetPixels: CGFloat = 100
        guard pixelsPerMS > 0 else { return 1000 }
        let targetMS = Double(targetPixels / pixelsPerMS)
        for ms in Self.niceMajorIntervalsMS {
            if Double(ms) >= targetMS { return ms }
        }
        return Self.niceMajorIntervalsMS.last ?? 60000
    }

    private func drawRulerTicks(cg: CGContext, rect: CGRect) {
        let majorMS = majorIntervalMS()
        let minorMS = max(1, majorMS / 2)

        cg.setStrokeColor(UIColor.gray.cgColor)
        cg.setLineWidth(1)

        let labelAttrs: [NSAttributedString.Key: Any] = [
            .font: UIFont.systemFont(ofSize: 9),
            .foregroundColor: UIColor.gray,
        ]

        // Cull ticks to the dirty rect for performance at large widths.
        let startMS = max(0, Int(rect.minX / pixelsPerMS) / minorMS * minorMS)
        var ms = startMS
        while ms <= durationMS {
            let x = CGFloat(ms) * pixelsPerMS
            if x > rect.maxX + 60 { break }

            let isMajor = ms % majorMS == 0
            let tickH: CGFloat = isMajor ? 15 : 8
            cg.move(to: CGPoint(x: x, y: rulerHeight - tickH))
            cg.addLine(to: CGPoint(x: x, y: rulerHeight))

            if isMajor {
                let label = Self.formatTimeLabel(ms: ms, majorMS: majorMS) as NSString
                label.draw(at: CGPoint(x: x + 2, y: 2), withAttributes: labelAttrs)
            }

            ms += minorMS
        }
        cg.strokePath()
    }

    /// mm:ss.fff style, matching desktop. Trailing zero digits beyond
    /// the current major-tick precision are dropped so 1-second ticks
    /// show `0:05` instead of `0:05.000`.
    static func formatTimeLabel(ms: Int, majorMS: Int) -> String {
        let minutes = ms / 60000
        let seconds = (ms % 60000) / 1000
        let millis = ms % 1000
        let showFractional = majorMS < 1000
        if showFractional {
            // Decide precision from majorMS: sub-100ms → 3 digits, else 2.
            if majorMS < 100 {
                return String(format: "%d:%02d.%03d", minutes, seconds, millis)
            } else {
                return String(format: "%d:%02d.%02d", minutes, seconds, millis / 10)
            }
        } else {
            return String(format: "%d:%02d", minutes, seconds)
        }
    }

    // Desktop draws the waveform as a filled polygon (min line forward,
    // max line back) in a light-blue fill with a white outline. The
    // older per-bucket green vertical-line style obscured the envelope
    // shape at high zoom. See `Waveform::DrawWaveView` in
    // `src-ui-wx/ui/sequencer/Waveform.cpp` for the reference.
    private func drawWaveform(cg: CGContext, in waveRect: CGRect, rect: CGRect) {
        guard peaks.count >= 2 else { return }
        let numBuckets = peaks.count / 2
        let centerY = waveRect.midY
        let scale = (waveRect.height / 2) * 0.9
        let timelineWidth = CGFloat(durationMS) * pixelsPerMS
        guard timelineWidth > 0 else { return }

        // Cull to the dirty rect — one polygon per redraw, so we only
        // need to walk the visible bucket range.
        let firstBucket = max(0, Int((rect.minX / timelineWidth) * CGFloat(numBuckets)) - 1)
        let lastBucket  = min(numBuckets - 1,
                              Int((rect.maxX / timelineWidth) * CGFloat(numBuckets)) + 1)
        guard firstBucket <= lastBucket else { return }

        let path = CGMutablePath()
        // Forward along the min (bottom) edge.
        var startedPath = false
        for i in firstBucket...lastBucket {
            let x = (CGFloat(i) / CGFloat(numBuckets)) * timelineWidth
            let mn = CGFloat(peaks[i * 2])
            let yMin = centerY - mn * scale
            if !startedPath {
                path.move(to: CGPoint(x: x, y: yMin))
                startedPath = true
            } else {
                path.addLine(to: CGPoint(x: x, y: yMin))
            }
        }
        // Back along the max (top) edge to close the polygon.
        for i in stride(from: lastBucket, through: firstBucket, by: -1) {
            let x = (CGFloat(i) / CGFloat(numBuckets)) * timelineWidth
            let mx = CGFloat(peaks[i * 2 + 1])
            let yMax = centerY - mx * scale
            path.addLine(to: CGPoint(x: x, y: yMax))
        }
        path.closeSubpath()

        // Fill light blue (desktop COLOR_WAVEFORM = 130, 178, 207).
        cg.addPath(path)
        cg.setFillColor(UIColor(red: 130/255.0, green: 178/255.0,
                                blue: 207/255.0, alpha: 1.0).cgColor)
        cg.fillPath()

        // White outline on top.
        cg.addPath(path)
        cg.setStrokeColor(UIColor.white.cgColor)
        cg.setLineWidth(1)
        cg.strokePath()
    }

    // MARK: - Gestures

    private var pinchAnchorX: CGFloat = 0
    private var pinchLastScale: CGFloat = 1

    func installGestures() {
        let pinch = UIPinchGestureRecognizer(target: self, action: #selector(onPinch(_:)))
        pinch.delegate = passthroughDelegate
        addGestureRecognizer(pinch)
    }

    private lazy var passthroughDelegate: PassthroughGestureDelegate = {
        PassthroughGestureDelegate()
    }()

    @objc private func onPinch(_ g: UIPinchGestureRecognizer) {
        switch g.state {
        case .began:
            pinchAnchorX = g.location(in: self).x
            pinchLastScale = 1
        case .changed:
            let delta = g.scale / pinchLastScale
            pinchLastScale = g.scale
            actions.onPinchZoom(delta, pinchAnchorX)
        default:
            break
        }
    }
}
