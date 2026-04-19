import SwiftUI
import UIKit
import MetalKit

/// Interactive Metal-backed model-effects grid. Owns the MTKView,
/// the `XLGridMetalBridge`, all gesture recognizers, and the
/// drag-state machinery.
///
/// Coordinate system: view is viewport-sized. World coords come
/// from `scrollOffsetX/Y` + the viewport's local coords. Scroll is
/// driven by the pan gesture when it doesn't hit a drag target —
/// there's no enclosing `UIScrollView`, so we don't have momentum
/// or bouncing. Trade-off for dropping the 16k texture limit.
struct EffectsMetalGridView: UIViewRepresentable {
    // Data
    let rows: [SequencerViewModel.RowInfo]
    let metrics: GridMetrics
    let pixelsPerMS: CGFloat
    let selection: SequencerViewModel.EffectSelection?
    let activeDrag: SequencerViewModel.ActiveDrag?
    let timingMarkTimesMS: [Int]

    // Shared scroll state (pan writes, other canvases read)
    @Binding var scrollOffsetX: CGFloat
    @Binding var scrollOffsetY: CGFloat

    // Actions
    let actions: EffectCanvasActions
    let stateLookup: EffectStateLookup
    let fadeProvider: (Int, Int) -> (Float, Float)
    let iconProvider: (_ effectName: String, _ bucket: Int) -> Data?
    let document: XLSequenceDocument

    func makeUIView(context: Context) -> EffectsMetalGridMTKView {
        let v = EffectsMetalGridMTKView()
        v.coordinator = context.coordinator
        v.isOpaque = true
        v.backgroundColor = UIColor(white: 0.05, alpha: 1.0)
        v.isMultipleTouchEnabled = true
        v.installGestures()
        return v
    }

    func updateUIView(_ view: EffectsMetalGridMTKView, context: Context) {
        // Push all inputs down. The MTKView redraws itself when any
        // of these change via the revision counter so we don't
        // plumb individual setNeedsDisplay calls from SwiftUI.
        let ctx = context.coordinator
        ctx.rows = rows
        ctx.metrics = metrics
        ctx.pixelsPerMS = pixelsPerMS
        ctx.selection = selection
        ctx.activeDrag = activeDrag
        ctx.timingMarkTimesMS = timingMarkTimesMS
        ctx.scrollOffsetX = scrollOffsetX
        ctx.scrollOffsetY = scrollOffsetY
        ctx.actions = actions
        ctx.stateLookup = stateLookup
        ctx.fadeProvider = fadeProvider
        ctx.iconProvider = iconProvider
        ctx.onUpdateScrollX = { newX in scrollOffsetX = newX }
        ctx.onUpdateScrollY = { newY in scrollOffsetY = newY }
        view.setNeedsDisplay()
    }

    func makeCoordinator() -> Coordinator {
        Coordinator(document: document)
    }

    /// Holds the mutable state the MTKView needs to render + drive
    /// interactions. Kept on the coordinator (rather than the
    /// MTKView itself) so SwiftUI can hand fresh values in via
    /// `updateUIView` without the MTKView's init signature needing
    /// them.
    final class Coordinator {
        let bridge: XLGridMetalBridge
        weak var view: EffectsMetalGridMTKView?

        // State pushed down from SwiftUI.
        var rows: [SequencerViewModel.RowInfo] = []
        var metrics = GridMetrics.standard
        var pixelsPerMS: CGFloat = 0.1
        var selection: SequencerViewModel.EffectSelection?
        var activeDrag: SequencerViewModel.ActiveDrag?
        var timingMarkTimesMS: [Int] = []
        var scrollOffsetX: CGFloat = 0
        var scrollOffsetY: CGFloat = 0
        var actions = EffectCanvasActions()
        var stateLookup = EffectStateLookup()
        var fadeProvider: (Int, Int) -> (Float, Float) = { _, _ in (0, 0) }
        var iconProvider: (String, Int) -> Data? = { _, _ in nil }
        var onUpdateScrollX: (CGFloat) -> Void = { _ in }
        var onUpdateScrollY: (CGFloat) -> Void = { _ in }

        // Gesture-local state.
        struct DragLocal {
            enum Kind { case move, resizeLeft, resizeRight, fadeIn, fadeOut }
            let rowIndex: Int
            let effectIndex: Int
            let origStartMS: Int
            let origEndMS: Int
            let origFadeInSec: Float
            let origFadeOutSec: Float
            let minStartMS: Int
            let maxEndMS: Int
            let kind: Kind
        }
        var drag: DragLocal?
        var liveStartMS: Int?
        var liveEndMS: Int?
        var liveFadeInSec: Float?
        var liveFadeOutSec: Float?
        var liveRowId: Int?
        var liveDropInvalid: Bool = false

        // Pan origin in world coords for delta math that stays
        // consistent when we scroll the content during the drag.
        var panStartScrollX: CGFloat = 0
        var panStartScrollY: CGFloat = 0

        // Auto-scroll during drag.
        var autoScrollLink: CADisplayLink?
        var autoScrollSpeedX: CGFloat = 0
        var autoScrollSpeedY: CGFloat = 0
        var lastTouchInSelf: CGPoint = .zero

        // Reference to the document is needed for icon lookups
        // (width/height of the texture) outside the provider.
        unowned let document: XLSequenceDocument

        init(document: XLSequenceDocument) {
            self.document = document
            self.bridge = XLGridMetalBridge(name: "ModelEffectsGrid")!
        }
    }
}

/// The MTKView subclass. Owns gesture recognizers and is the
/// entry point for `draw(_:)`. Pulls state off the
/// `EffectsMetalGridView.Coordinator` owned by its Representable.
final class EffectsMetalGridMTKView: MTKView, MTKViewDelegate {
    weak var coordinator: EffectsMetalGridView.Coordinator?
    private var gestureDelegate: GestureDelegate?
    private weak var ownPan: UIPanGestureRecognizer?
    private let vFadeHitStrip: CGFloat = 7
    private let minIconWidth: CGFloat = 14

    override init(frame frameRect: CGRect, device: MTLDevice?) {
        super.init(frame: frameRect,
                    device: device ?? MTLCreateSystemDefaultDevice())
        common()
    }
    required init(coder: NSCoder) {
        super.init(coder: coder)
        common()
    }

    private func common() {
        self.colorPixelFormat = .bgra8Unorm
        self.clearColor = MTLClearColorMake(0.05, 0.05, 0.05, 1.0)
        self.isPaused = true
        self.enableSetNeedsDisplay = true
        self.delegate = self
        if let layer = self.layer as? CAMetalLayer {
            layer.isOpaque = true
        }
    }

    override func layoutSubviews() {
        super.layoutSubviews()
        guard let c = coordinator else { return }
        if let layer = self.layer as? CAMetalLayer,
           c.bridge.responds(to: #selector(XLGridMetalBridge.attach(_:))) {
            // Attach once.
            let key = "attachedLayer_marker_set"
            if objc_getAssociatedObject(c.bridge, key) == nil {
                c.bridge.attach(layer)
                objc_setAssociatedObject(c.bridge, key, NSNumber(value: true),
                                          .OBJC_ASSOCIATION_RETAIN)
            }
        }
        c.bridge.setDrawableSize(self.drawableSize,
                                  scale: self.contentScaleFactor)
    }

    // MARK: - MTKViewDelegate

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

    // MARK: - Rendering

    private func drawAll(bridge: XLGridMetalBridge, viewport: CGSize,
                          c: EffectsMetalGridView.Coordinator) {
        // Row zebra stripes.
        bridge.beginFilledRectBatch()
        var y: CGFloat = -c.scrollOffsetY
        for row in c.rows {
            let h = (row.id == c.selection?.rowIndex)
                ? c.metrics.selectedRowHeight : c.metrics.rowHeight
            if y + h < 0 { y += h; continue }
            if y > viewport.height { break }
            let shade: CGFloat = (row.id % 2 == 0) ? 0.08 : 0.10
            bridge.appendFilledRectX(0, y: y,
                                      w: viewport.width, h: h,
                                      r: shade, g: shade, b: shade, a: 1.0)
            y += h
        }
        bridge.flushFilledRectBatch()

        // Vertical timing mark lines.
        if !c.timingMarkTimesMS.isEmpty {
            bridge.beginLineBatch()
            for ms in c.timingMarkTimesMS {
                let x = CGFloat(ms) * c.pixelsPerMS - c.scrollOffsetX
                if x < -1 || x > viewport.width + 1 { continue }
                bridge.appendLineX1(x + 0.5, y1: 0,
                                     x2: x + 0.5, y2: viewport.height,
                                     r: 1, g: 1, b: 1, a: 0.15)
            }
            bridge.flushLineBatch()
        }

        // Collect visible effects.
        struct Visible {
            let rowId: Int
            let effectIndex: Int
            let name: String
            let x1: CGFloat
            let x2: CGFloat
            let top: CGFloat
            let bottom: CGFloat
            let stroke: (CGFloat, CGFloat, CGFloat)
            let disabled: Bool
            let locked: Bool
            let fadeInSec: Float
            let fadeOutSec: Float
            // Populated by the background pass below: 0 = effect drew a
            // complete background (skip icon), 1 = show normal icon,
            // 2 = show smaller icon (leaves room for partial background).
            // Default 1 covers cross-row ghosts that skip the BG pass.
            var drawIconHint: Int = 1
        }
        var vis: [Visible] = []
        vis.reserveCapacity(256)
        var rowTopsById: [Int: (CGFloat, CGFloat)] = [:]
        y = -c.scrollOffsetY
        for row in c.rows {
            let h = (row.id == c.selection?.rowIndex)
                ? c.metrics.selectedRowHeight : c.metrics.rowHeight
            rowTopsById[row.id] = (y, h)
            if y + h < 0 { y += h; continue }
            if y > viewport.height { break }
            let top = y + 1
            let bottom = y + h - 1
            for (eIdx, effect) in row.effects.enumerated() {
                let isDragged = (c.activeDrag?.srcRowId == row.id
                                  && c.activeDrag?.effectIndex == eIdx)
                if isDragged, c.activeDrag?.liveRowId != nil { continue }
                let startMS = isDragged ? (c.activeDrag?.liveStartMS ?? effect.startTimeMS) : effect.startTimeMS
                let endMS   = isDragged ? (c.activeDrag?.liveEndMS   ?? effect.endTimeMS)   : effect.endTimeMS
                let x1 = CGFloat(startMS) * c.pixelsPerMS - c.scrollOffsetX
                let x2 = CGFloat(endMS)   * c.pixelsPerMS - c.scrollOffsetX
                if x2 < 0 || x1 > viewport.width { continue }
                let isSel = (c.selection?.rowIndex == row.id && c.selection?.effectIndex == eIdx)
                let locked = c.stateLookup.isLocked(row.id, eIdx)
                let disabled = c.stateLookup.isDisabled(row.id, eIdx)
                let col: (CGFloat, CGFloat, CGFloat)
                if disabled      { col = (0.45, 0.45, 0.45) }
                else if isSel    { col = (1.00, 0.85, 0.25) }
                else if locked   { col = (0.65, 0.80, 1.00) }
                else             { col = (0.85, 0.85, 0.85) }
                let fadeIn: Float
                let fadeOut: Float
                if isDragged, let d = c.activeDrag {
                    fadeIn = d.liveFadeInSec
                    fadeOut = d.liveFadeOutSec
                } else {
                    (fadeIn, fadeOut) = c.fadeProvider(row.id, eIdx)
                }
                vis.append(Visible(
                    rowId: row.id, effectIndex: eIdx, name: effect.name,
                    x1: x1, x2: x2, top: top, bottom: bottom,
                    stroke: col, disabled: disabled, locked: locked,
                    fadeInSec: fadeIn, fadeOutSec: fadeOut))
            }
            y += h
        }

        // Cross-row ghost.
        if let d = c.activeDrag,
           d.kind == .move,
           let dstId = d.liveRowId,
           let dst = rowTopsById[dstId],
           let srcRow = c.rows.first(where: { $0.id == d.srcRowId }),
           d.effectIndex < srcRow.effects.count {
            let effect = srcRow.effects[d.effectIndex]
            let x1 = CGFloat(d.liveStartMS) * c.pixelsPerMS - c.scrollOffsetX
            let x2 = CGFloat(d.liveEndMS)   * c.pixelsPerMS - c.scrollOffsetX
            let top = dst.0 + 1
            let bottom = dst.0 + dst.1 - 1
            if x2 >= 0 && x1 <= viewport.width {
                vis.append(Visible(
                    rowId: d.srcRowId, effectIndex: d.effectIndex, name: effect.name,
                    x1: x1, x2: x2, top: top, bottom: bottom,
                    stroke: (1, 0.85, 0.25),
                    disabled: false, locked: false,
                    fadeInSec: d.liveFadeInSec, fadeOutSec: d.liveFadeOutSec))
            }
        }

        // Effect backgrounds (desktop parity via BM-6). Each visible
        // effect asks `RenderableEffect::DrawEffectBackground` to
        // append its geometry into a shared accumulator, then we flush
        // once. The returned hint (0/1/2) drives icon sizing below.
        bridge.beginEffectBackgroundBatch()
        for i in vis.indices {
            let e = vis[i]
            let hint = c.document.appendEffectBackground(
                forRow: Int32(e.rowId),
                at: Int32(e.effectIndex),
                x1: Float(e.x1), y1: Float(e.top),
                x2: Float(e.x2), y2: Float(e.bottom),
                bridge: c.bridge,
                drawRamps: true)
            vis[i].drawIconHint = Int(hint)
        }
        bridge.flushEffectBackgroundBatch()

        // Disabled overlays.
        bridge.beginFilledRectBatch()
        for e in vis where e.disabled {
            bridge.appendFilledRectX(e.x1, y: e.top, w: max(1, e.x2 - e.x1),
                                      h: e.bottom - e.top,
                                      r: 0.15, g: 0.15, b: 0.15, a: 0.65)
        }
        bridge.flushFilledRectBatch()

        // Brackets + centerlines.
        let nameFontSize: CGFloat = 11
        bridge.beginLineBatch()
        for e in vis {
            let mid = (e.top + e.bottom) / 2
            bridge.appendLineX1(e.x1 + 0.5, y1: e.top,
                                 x2: e.x1 + 0.5, y2: e.bottom,
                                 r: e.stroke.0, g: e.stroke.1, b: e.stroke.2, a: 1.0)
            bridge.appendLineX1(e.x2 - 0.5, y1: e.top,
                                 x2: e.x2 - 0.5, y2: e.bottom,
                                 r: e.stroke.0, g: e.stroke.1, b: e.stroke.2, a: 1.0)
            // Centerline only when the icon pass is going to show an
            // icon. When `drawIconHint == 0` the effect drew a full
            // background — desktop skips the centerline there too.
            if e.drawIconHint == 0 { continue }
            let width = e.x2 - e.x1
            if width > minIconWidth {
                let iconCap: CGFloat = e.drawIconHint == 2 ? 14 : 22
                let iconSize = min(max(e.bottom - e.top - 4, 8), iconCap)
                let iconX = (e.x1 + e.x2) / 2
                let iconLeft = iconX - iconSize / 2
                let iconRight = iconX + iconSize / 2
                var rightStart = iconRight
                if width > 70, !e.name.isEmpty {
                    let ns = iconRight + 4
                    let nMax = e.x2 - 3
                    if nMax - ns > 14 {
                        let labelW = bridge.size(ofText: e.name, fontSize: nameFontSize).width
                        if labelW <= nMax - ns { rightStart = ns + labelW + 2 }
                    }
                }
                bridge.appendLineX1(e.x1, y1: mid + 0.5, x2: iconLeft, y2: mid + 0.5,
                                     r: e.stroke.0, g: e.stroke.1, b: e.stroke.2, a: 1.0)
                bridge.appendLineX1(rightStart, y1: mid + 0.5, x2: e.x2, y2: mid + 0.5,
                                     r: e.stroke.0, g: e.stroke.1, b: e.stroke.2, a: 1.0)
            } else {
                bridge.appendLineX1(e.x1, y1: mid + 0.5, x2: e.x2, y2: mid + 0.5,
                                     r: e.stroke.0, g: e.stroke.1, b: e.stroke.2, a: 1.0)
            }
        }
        bridge.flushLineBatch()

        // Icons + name labels.
        let screenScale = UIScreen.main.scale
        for e in vis {
            let width = e.x2 - e.x1
            guard width > minIconWidth else { continue }
            // Effect drew a complete background — skip the icon entirely.
            if e.drawIconHint == 0 { continue }
            // Hint 2 = smaller icon; keeps room for partial background.
            let iconCap: CGFloat = e.drawIconHint == 2 ? 14 : 22
            let iconSize = min(max(e.bottom - e.top - 4, 8), iconCap)
            let iconX = (e.x1 + e.x2) / 2
            let iconLeft = iconX - iconSize / 2
            let iconRight = iconX + iconSize / 2
            let screenPx = iconSize * screenScale
            let bucket: Int
            if      screenPx <= 16 { bucket = 16 }
            else if screenPx <= 24 { bucket = 24 }
            else if screenPx <= 32 { bucket = 32 }
            else if screenPx <= 48 { bucket = 48 }
            else                   { bucket = 64 }
            if let data = c.iconProvider(e.name, bucket) {
                let key = "\(e.name)@\(bucket)"
                c.bridge.ensureTextureNamed(key, bgraData: data,
                                             w: Int32(bucket), h: Int32(bucket))
                c.bridge.drawTextureNamed(key, x: iconLeft,
                                           y: e.top + (e.bottom - e.top - iconSize) / 2,
                                           w: iconSize, h: iconSize)
            }
            if width > 70, !e.name.isEmpty {
                let nameStartX = iconRight + 4
                let nameMaxX = e.x2 - 3
                if nameMaxX - nameStartX > 14 {
                    let labelSize = bridge.size(ofText: e.name, fontSize: nameFontSize)
                    if labelSize.width <= (nameMaxX - nameStartX) {
                        bridge.drawText(e.name,
                                         atX: nameStartX,
                                         y: e.top + (e.bottom - e.top - labelSize.height) / 2,
                                         fontSize: nameFontSize,
                                         r: e.stroke.0, g: e.stroke.1, b: e.stroke.2, a: 0.95)
                    }
                }
            }
        }

        // Fade bars + yellow overlap stripe.
        bridge.beginFilledRectBatch()
        for e in vis {
            let fadeInPx = CGFloat(e.fadeInSec) * 1000 * c.pixelsPerMS
            let fadeOutPx = CGFloat(e.fadeOutSec) * 1000 * c.pixelsPerMS
            let barH: CGFloat = 3
            let width = max(e.x2 - e.x1, 1)
            if fadeInPx > 0 {
                bridge.appendFilledRectX(e.x1, y: e.top, w: min(fadeInPx, width), h: barH,
                                          r: 0.20, g: 0.78, b: 0.35, a: 0.85)
            }
            if fadeOutPx > 0 {
                bridge.appendFilledRectX(max(e.x1, e.x2 - fadeOutPx), y: e.top,
                                          w: min(fadeOutPx, width), h: barH,
                                          r: 0.95, g: 0.30, b: 0.25, a: 0.85)
            }
            if fadeInPx + fadeOutPx > width, width > 0 {
                let s = max(e.x1, e.x2 - fadeOutPx)
                let eX = min(e.x1 + fadeInPx, e.x2)
                if eX > s {
                    bridge.appendFilledRectX(s, y: e.top, w: eX - s, h: barH,
                                              r: 0.95, g: 0.85, b: 0.15, a: 0.95)
                }
            }
        }
        bridge.flushFilledRectBatch()

        // Invalid-drop tint.
        if let d = c.activeDrag, d.liveDropInvalid,
           let dstId = d.liveRowId,
           let dst = rowTopsById[dstId] {
            let x1 = CGFloat(d.liveStartMS) * c.pixelsPerMS - c.scrollOffsetX
            let x2 = CGFloat(d.liveEndMS)   * c.pixelsPerMS - c.scrollOffsetX
            let top = dst.0 + 1
            let bottom = dst.0 + dst.1 - 1
            if x2 >= 0 && x1 <= viewport.width {
                bridge.beginFilledRectBatch()
                bridge.appendFilledRectX(x1, y: top,
                                          w: max(1, x2 - x1), h: bottom - top,
                                          r: 1.0, g: 0.18, b: 0.18, a: 0.35)
                bridge.flushFilledRectBatch()
            }
        }

        // Lock glyphs.
        for e in vis where e.locked {
            if e.x2 - e.x1 > 10 {
                bridge.drawText("🔒", atX: e.x2 - 12, y: e.top,
                                 fontSize: 9, r: 1, g: 1, b: 1, a: 1)
            }
        }

        // Fade-handle diamonds for selected effect.
        for e in vis where c.selection?.rowIndex == e.rowId
                            && c.selection?.effectIndex == e.effectIndex {
            if e.x2 - e.x1 < 8 { continue }
            let fadeInPx = CGFloat(e.fadeInSec) * 1000 * c.pixelsPerMS
            let fadeOutPx = CGFloat(e.fadeOutSec) * 1000 * c.pixelsPerMS
            let width = e.x2 - e.x1
            let barH: CGFloat = 3
            drawFadeDiamond(bridge: bridge,
                             x: e.x1 + min(fadeInPx, width),
                             y: e.top + barH / 2,
                             fill: (0.20, 0.78, 0.35))
            drawFadeDiamond(bridge: bridge,
                             x: max(e.x1, e.x2 - fadeOutPx),
                             y: e.top + barH / 2,
                             fill: (0.95, 0.30, 0.25))
        }

        // Drag feedback pill.
        drawDragPill(bridge: bridge, viewport: viewport, c: c)
    }

    private func drawFadeDiamond(bridge: XLGridMetalBridge,
                                  x: CGFloat, y: CGFloat,
                                  fill: (CGFloat, CGFloat, CGFloat)) {
        let r: CGFloat = 3.5
        bridge.fillTriangleX1(x, y1: y - r,
                              x2: x + r, y2: y,
                              x3: x - r, y3: y,
                              r: fill.0, g: fill.1, b: fill.2, a: 0.95)
        bridge.fillTriangleX1(x, y1: y + r,
                              x2: x + r, y2: y,
                              x3: x - r, y3: y,
                              r: fill.0, g: fill.1, b: fill.2, a: 0.95)
        bridge.beginLineBatch()
        bridge.appendLineX1(x,        y1: y - r, x2: x + r,    y2: y,
                             r: 1, g: 1, b: 1, a: 1)
        bridge.appendLineX1(x + r,    y1: y,     x2: x,        y2: y + r,
                             r: 1, g: 1, b: 1, a: 1)
        bridge.appendLineX1(x,        y1: y + r, x2: x - r,    y2: y,
                             r: 1, g: 1, b: 1, a: 1)
        bridge.appendLineX1(x - r,    y1: y,     x2: x,        y2: y - r,
                             r: 1, g: 1, b: 1, a: 1)
        bridge.flushLineBatch()
    }

    private func drawDragPill(bridge: XLGridMetalBridge, viewport: CGSize,
                               c: EffectsMetalGridView.Coordinator) {
        guard let d = c.activeDrag else { return }
        let anchorRowId = d.liveRowId ?? d.srcRowId
        var y: CGFloat = -c.scrollOffsetY
        var rowTop: CGFloat = 0
        var rowH: CGFloat = c.metrics.rowHeight
        for row in c.rows {
            let h = (row.id == c.selection?.rowIndex)
                ? c.metrics.selectedRowHeight : c.metrics.rowHeight
            if row.id == anchorRowId { rowTop = y; rowH = h; break }
            y += h
        }
        let x1 = CGFloat(d.liveStartMS) * c.pixelsPerMS - c.scrollOffsetX
        let x2 = CGFloat(d.liveEndMS)   * c.pixelsPerMS - c.scrollOffsetX
        let top = rowTop + 1
        let text: String
        switch d.kind {
        case .move:
            text = "\(Self.timeStr(d.liveStartMS)) – \(Self.timeStr(d.liveEndMS))"
        case .resizeLeft:
            text = "\(Self.timeStr(d.liveStartMS)) (\(Self.durStr(d.liveEndMS - d.liveStartMS)))"
        case .resizeRight:
            text = "\(Self.timeStr(d.liveEndMS)) (\(Self.durStr(d.liveEndMS - d.liveStartMS)))"
        case .fadeIn:
            text = "fade in: \(String(format: "%.2fs", d.liveFadeInSec))"
        case .fadeOut:
            text = "fade out: \(String(format: "%.2fs", d.liveFadeOutSec))"
        }
        let tS = bridge.size(ofText: text, fontSize: 10)
        let padX: CGFloat = 5
        let padY: CGFloat = 2
        let w = tS.width + padX * 2
        let h = tS.height + padY * 2
        let cx = (x1 + x2) / 2
        let pillX = max(2, min(viewport.width - w - 2, cx - w / 2))
        let pillY = (top - h - 2) >= 0 ? top - h - 2 : top + rowH + 2
        bridge.beginFilledRectBatch()
        bridge.appendFilledRectX(pillX, y: pillY, w: w, h: h,
                                  r: 0, g: 0, b: 0, a: 0.75)
        bridge.flushFilledRectBatch()
        bridge.drawText(text, atX: pillX + padX, y: pillY + padY,
                         fontSize: 10, r: 1, g: 1, b: 1, a: 1)
    }

    private static func timeStr(_ ms: Int) -> String {
        let m = abs(ms)
        let sign = ms < 0 ? "-" : ""
        return String(format: "%@%d:%02d.%03d", sign,
                      m / 60000, (m / 1000) % 60, m % 1000)
    }
    private static func durStr(_ ms: Int) -> String {
        let s = Double(ms) / 1000.0
        return s >= 10 ? String(format: "%.1fs", s)
                        : String(format: "%.2fs", s)
    }

    // MARK: - Gestures

    func installGestures() {
        let del = GestureDelegate(owner: self)
        self.gestureDelegate = del
        let tap = UITapGestureRecognizer(target: self, action: #selector(onTap(_:)))
        tap.delegate = del
        addGestureRecognizer(tap)
        let pan = UIPanGestureRecognizer(target: self, action: #selector(onPan(_:)))
        pan.delegate = del
        addGestureRecognizer(pan)
        ownPan = pan
        let pinch = UIPinchGestureRecognizer(target: self, action: #selector(onPinch(_:)))
        pinch.delegate = del
        addGestureRecognizer(pinch)
        let lp = UILongPressGestureRecognizer(target: self, action: #selector(onLongPress(_:)))
        lp.minimumPressDuration = 0.6
        lp.allowableMovement = 4
        lp.delegate = del
        addGestureRecognizer(lp)
    }

    // Hit test zones reused from CG path.
    enum HitZone { case center, leftEdge, rightEdge, fadeIn, fadeOut }
    struct Hit { let rowIndex: Int; let effectIndex: Int; let zone: HitZone }

    func hitTestEffect(at p: CGPoint) -> Hit? {
        guard let c = coordinator else { return nil }
        var y: CGFloat = -c.scrollOffsetY
        var rowIdx = -1
        var rowTop: CGFloat = 0
        var rowH: CGFloat = 0
        for (i, row) in c.rows.enumerated() {
            let h = (row.id == c.selection?.rowIndex)
                ? c.metrics.selectedRowHeight : c.metrics.rowHeight
            if p.y >= y && p.y < y + h {
                rowIdx = i
                rowTop = y
                rowH = h
                break
            }
            y += h
        }
        guard rowIdx >= 0 else { return nil }
        let row = c.rows[rowIdx]
        let ms = Int((p.x + c.scrollOffsetX) / c.pixelsPerMS)
        let slop = c.metrics.edgeHandleHitWidth / c.pixelsPerMS
        let inFadeStrip = (p.y - rowTop) < vFadeHitStrip + 1
        _ = rowH
        for (eIdx, effect) in row.effects.enumerated() {
            if ms < effect.startTimeMS - Int(slop) { break }
            if ms > effect.endTimeMS + Int(slop) { continue }
            let isSel = (c.selection?.rowIndex == row.id
                          && c.selection?.effectIndex == eIdx)
            if isSel && inFadeStrip
                && ms >= effect.startTimeMS && ms <= effect.endTimeMS {
                let midMS = (effect.startTimeMS + effect.endTimeMS) / 2
                return Hit(rowIndex: row.id, effectIndex: eIdx,
                           zone: ms <= midMS ? .fadeIn : .fadeOut)
            }
            if isSel
                && ms >= effect.startTimeMS - Int(slop/2)
                && ms <= effect.startTimeMS + Int(slop/2) {
                return Hit(rowIndex: row.id, effectIndex: eIdx, zone: .leftEdge)
            }
            if isSel
                && ms >= effect.endTimeMS - Int(slop/2)
                && ms <= effect.endTimeMS + Int(slop/2) {
                return Hit(rowIndex: row.id, effectIndex: eIdx, zone: .rightEdge)
            }
            if ms >= effect.startTimeMS && ms <= effect.endTimeMS {
                return Hit(rowIndex: row.id, effectIndex: eIdx, zone: .center)
            }
        }
        return nil
    }

    func canvasHasDragCandidate(at p: CGPoint) -> Bool {
        guard let hit = hitTestEffect(at: p), let c = coordinator else { return false }
        if hit.zone == .center {
            return c.selection?.rowIndex == hit.rowIndex
                && c.selection?.effectIndex == hit.effectIndex
        }
        return true
    }

    @objc func onTap(_ g: UITapGestureRecognizer) {
        guard let c = coordinator else { return }
        let p = g.location(in: self)
        if let hit = hitTestEffect(at: p) {
            c.actions.onTapEffect(hit.rowIndex, hit.effectIndex)
        } else {
            // Outside any effect — forward (rowId, ms) like the CG
            // path so palette tap-to-add still works.
            var y: CGFloat = -c.scrollOffsetY
            var rowId: Int? = nil
            for row in c.rows {
                let h = (row.id == c.selection?.rowIndex)
                    ? c.metrics.selectedRowHeight : c.metrics.rowHeight
                if p.y >= y && p.y < y + h {
                    if row.timing == nil { rowId = row.id }
                    break
                }
                y += h
            }
            let ms: Int? = rowId != nil
                ? max(0, Int((p.x + c.scrollOffsetX) / c.pixelsPerMS))
                : nil
            c.actions.onTapEmpty(rowId, ms)
        }
    }

    @objc func onLongPress(_ g: UILongPressGestureRecognizer) {
        guard g.state == .began, let c = coordinator else { return }
        let p = g.location(in: self)
        guard let hit = hitTestEffect(at: p) else { return }
        c.actions.onTapEffect(hit.rowIndex, hit.effectIndex)
        c.actions.onRequestContextMenu(hit.rowIndex, hit.effectIndex, p)
    }

    @objc func onPinch(_ g: UIPinchGestureRecognizer) {
        guard let c = coordinator else { return }
        switch g.state {
        case .began:
            pinchAnchorX = g.location(in: self).x
            pinchLastScale = 1
        case .changed:
            let delta = g.scale / pinchLastScale
            pinchLastScale = g.scale
            c.actions.onPinchZoom(delta, pinchAnchorX + c.scrollOffsetX)
        default:
            break
        }
    }
    private var pinchAnchorX: CGFloat = 0
    private var pinchLastScale: CGFloat = 1

    @objc func onPan(_ g: UIPanGestureRecognizer) {
        guard let c = coordinator else { return }
        switch g.state {
        case .began:
            let p = g.location(in: self)
            if let hit = hitTestEffect(at: p), canvasHasDragCandidate(at: p) {
                beginEffectDrag(hit: hit, c: c)
            } else {
                // Plain scroll pan — capture origin for delta math.
                c.drag = nil
                c.panStartScrollX = c.scrollOffsetX
                c.panStartScrollY = c.scrollOffsetY
            }
        case .changed:
            if c.drag != nil {
                updateEffectDrag(g: g, c: c)
                lastTouchInSelf = g.location(in: self)
                updateAutoScroll(c: c, viewport: bounds.size)
            } else {
                // Scrolling.
                let t = g.translation(in: self)
                c.onUpdateScrollX(max(0, c.panStartScrollX - t.x))
                c.onUpdateScrollY(max(0, c.panStartScrollY - t.y))
            }
        case .ended, .cancelled, .failed:
            stopAutoScroll(c: c)
            if let _ = c.drag {
                endEffectDrag(c: c)
            }
        default:
            break
        }
    }

    // MARK: - Drag lifecycle

    private var lastTouchInSelf: CGPoint = .zero

    private func beginEffectDrag(hit: Hit, c: EffectsMetalGridView.Coordinator) {
        guard let row = c.rows.first(where: { $0.id == hit.rowIndex }),
              hit.effectIndex < row.effects.count else { return }
        let effect = row.effects[hit.effectIndex]
        let kind: EffectsMetalGridView.Coordinator.DragLocal.Kind
        switch hit.zone {
        case .leftEdge:  kind = .resizeLeft
        case .rightEdge: kind = .resizeRight
        case .fadeIn:    kind = .fadeIn
        case .fadeOut:   kind = .fadeOut
        case .center:    kind = .move
        }
        let (curIn, curOut) = c.fadeProvider(hit.rowIndex, hit.effectIndex)
        let prevEnd = hit.effectIndex > 0
            ? row.effects[hit.effectIndex - 1].endTimeMS : 0
        let nextStart = hit.effectIndex + 1 < row.effects.count
            ? row.effects[hit.effectIndex + 1].startTimeMS : Int.max
        c.drag = .init(rowIndex: hit.rowIndex,
                        effectIndex: hit.effectIndex,
                        origStartMS: effect.startTimeMS,
                        origEndMS: effect.endTimeMS,
                        origFadeInSec: curIn, origFadeOutSec: curOut,
                        minStartMS: prevEnd, maxEndMS: nextStart,
                        kind: kind)
        c.liveStartMS = effect.startTimeMS
        c.liveEndMS = effect.endTimeMS
        c.liveFadeInSec = curIn
        c.liveFadeOutSec = curOut
        c.liveRowId = nil
        c.liveDropInvalid = false
        publishDrag(c: c)
    }

    private func updateEffectDrag(g: UIPanGestureRecognizer,
                                    c: EffectsMetalGridView.Coordinator) {
        guard let d = c.drag else { return }
        let tx = g.translation(in: self).x
        let dMS = Int(tx / c.pixelsPerMS)
        switch d.kind {
        case .move:
            // Target-row awareness for cross-row drag.
            let p = g.location(in: self)
            var hoverId: Int? = nil
            var ySum: CGFloat = -c.scrollOffsetY
            for row in c.rows {
                let h = (row.id == c.selection?.rowIndex)
                    ? c.metrics.selectedRowHeight : c.metrics.rowHeight
                if p.y >= ySum && p.y < ySum + h {
                    hoverId = row.id
                    break
                }
                ySum += h
            }
            if let h = hoverId,
               let r = c.rows.first(where: { $0.id == h }),
               r.timing != nil {
                hoverId = nil
            }
            c.liveRowId = (hoverId == d.rowIndex) ? nil : hoverId
            let duration = d.origEndMS - d.origStartMS
            let unclampedStart = d.origStartMS + dMS
            let unclampedEnd = unclampedStart + duration
            var minStart = d.minStartMS
            var maxEnd = d.maxEndMS
            var invalid = false
            if let dstId = c.liveRowId,
               let dstRow = c.rows.first(where: { $0.id == dstId }) {
                var prevEnd = 0
                var nextStart = Int.max
                for e in dstRow.effects {
                    if unclampedStart < e.endTimeMS && unclampedEnd > e.startTimeMS {
                        invalid = true
                    }
                    if e.endTimeMS <= unclampedStart {
                        prevEnd = max(prevEnd, e.endTimeMS)
                    } else if e.startTimeMS >= unclampedEnd {
                        nextStart = min(nextStart, e.startTimeMS)
                    }
                }
                minStart = prevEnd
                maxEnd = nextStart
            }
            let maxStart = maxEnd == Int.max ? Int.max : maxEnd - duration
            if maxStart < minStart { invalid = true }
            c.liveDropInvalid = invalid
            let cs = invalid ? unclampedStart
                              : max(minStart, min(maxStart, unclampedStart))
            c.liveStartMS = cs
            c.liveEndMS = cs + duration
        case .resizeLeft:
            let ns = max(d.minStartMS, d.origStartMS + dMS)
            c.liveStartMS = min(ns, d.origEndMS - 1)
            c.liveEndMS = d.origEndMS
        case .resizeRight:
            let raw = d.origEndMS + dMS
            let capped = min(d.maxEndMS, raw)
            c.liveEndMS = max(capped, d.origStartMS + 1)
            c.liveStartMS = d.origStartMS
        case .fadeIn, .fadeOut:
            let durMS = max(1, d.origEndMS - d.origStartMS)
            let otherFade = d.kind == .fadeIn ? d.origFadeOutSec : d.origFadeInSec
            let otherMS = Int(otherFade * 1000)
            let maxMS = max(0, durMS - otherMS)
            let deltaMS = d.kind == .fadeIn ? dMS : -dMS
            let origMS = Int((d.kind == .fadeIn ? d.origFadeInSec : d.origFadeOutSec) * 1000)
            let newMS = max(0, min(maxMS, origMS + deltaMS))
            let sec = Float(newMS) / 1000.0
            if d.kind == .fadeIn { c.liveFadeInSec = sec }
            else                 { c.liveFadeOutSec = sec }
        }
        publishDrag(c: c)
        setNeedsDisplay()
    }

    private func endEffectDrag(c: EffectsMetalGridView.Coordinator) {
        guard let d = c.drag else { return }
        switch d.kind {
        case .move:
            if let ls = c.liveStartMS, let le = c.liveEndMS {
                if let dst = c.liveRowId, dst != d.rowIndex {
                    if !c.liveDropInvalid {
                        c.actions.onMoveEffectToRow(d.rowIndex, d.effectIndex, dst, ls, le)
                    }
                } else {
                    c.actions.onMoveEffect(d.rowIndex, d.effectIndex, ls, le)
                }
            }
        case .resizeLeft:
            if let ls = c.liveStartMS {
                c.actions.onResizeEdge(d.rowIndex, d.effectIndex, 0, ls)
            }
        case .resizeRight:
            if let le = c.liveEndMS {
                c.actions.onResizeEdge(d.rowIndex, d.effectIndex, 1, le)
            }
        case .fadeIn:
            if let v = c.liveFadeInSec {
                c.actions.onAdjustFade(d.rowIndex, d.effectIndex, 0, v)
            }
        case .fadeOut:
            if let v = c.liveFadeOutSec {
                c.actions.onAdjustFade(d.rowIndex, d.effectIndex, 1, v)
            }
        }
        c.drag = nil
        c.liveStartMS = nil
        c.liveEndMS = nil
        c.liveFadeInSec = nil
        c.liveFadeOutSec = nil
        c.liveRowId = nil
        c.liveDropInvalid = false
        c.actions.onActiveDragChanged(nil)
        setNeedsDisplay()
    }

    private func publishDrag(c: EffectsMetalGridView.Coordinator) {
        guard let d = c.drag else { return }
        let kind: SequencerViewModel.ActiveDrag.Kind
        switch d.kind {
        case .move: kind = .move
        case .resizeLeft: kind = .resizeLeft
        case .resizeRight: kind = .resizeRight
        case .fadeIn: kind = .fadeIn
        case .fadeOut: kind = .fadeOut
        }
        let snap = SequencerViewModel.ActiveDrag(
            kind: kind,
            srcRowId: d.rowIndex, effectIndex: d.effectIndex,
            origStartMS: d.origStartMS, origEndMS: d.origEndMS,
            liveStartMS: c.liveStartMS ?? d.origStartMS,
            liveEndMS: c.liveEndMS ?? d.origEndMS,
            liveFadeInSec: c.liveFadeInSec ?? d.origFadeInSec,
            liveFadeOutSec: c.liveFadeOutSec ?? d.origFadeOutSec,
            liveRowId: c.liveRowId, liveDropInvalid: c.liveDropInvalid)
        c.actions.onActiveDragChanged(snap)
    }

    // MARK: - Auto-scroll

    private func updateAutoScroll(c: EffectsMetalGridView.Coordinator,
                                    viewport: CGSize) {
        let margin: CGFloat = 60
        let maxSpeed: CGFloat = 18
        let p = lastTouchInSelf
        var sx: CGFloat = 0
        var sy: CGFloat = 0
        if p.x < margin {
            sx = -maxSpeed * max(0, min(1, (margin - p.x) / margin))
        } else if p.x > viewport.width - margin {
            sx = maxSpeed * max(0, min(1, (margin - (viewport.width - p.x)) / margin))
        }
        if p.y < margin {
            sy = -maxSpeed * max(0, min(1, (margin - p.y) / margin))
        } else if p.y > viewport.height - margin {
            sy = maxSpeed * max(0, min(1, (margin - (viewport.height - p.y)) / margin))
        }
        c.autoScrollSpeedX = sx
        c.autoScrollSpeedY = sy
        if sx == 0 && sy == 0 {
            stopAutoScroll(c: c)
        } else if c.autoScrollLink == nil {
            let link = CADisplayLink(target: self, selector: #selector(autoScrollTick))
            link.add(to: .main, forMode: .common)
            c.autoScrollLink = link
        }
    }

    @objc private func autoScrollTick() {
        guard let c = coordinator else { return }
        let newX = max(0, c.scrollOffsetX + c.autoScrollSpeedX)
        let newY = max(0, c.scrollOffsetY + c.autoScrollSpeedY)
        if newX != c.scrollOffsetX { c.onUpdateScrollX(newX) }
        if newY != c.scrollOffsetY { c.onUpdateScrollY(newY) }
    }

    private func stopAutoScroll(c: EffectsMetalGridView.Coordinator) {
        c.autoScrollLink?.invalidate()
        c.autoScrollLink = nil
        c.autoScrollSpeedX = 0
        c.autoScrollSpeedY = 0
    }

    // MARK: - Gesture delegate

    final class GestureDelegate: NSObject, UIGestureRecognizerDelegate {
        weak var owner: EffectsMetalGridMTKView?
        init(owner: EffectsMetalGridMTKView) {
            self.owner = owner
        }
        func gestureRecognizer(_ g: UIGestureRecognizer,
                               shouldRecognizeSimultaneouslyWith other: UIGestureRecognizer) -> Bool {
            return true
        }
        func gestureRecognizerShouldBegin(_ g: UIGestureRecognizer) -> Bool {
            guard let v = owner else { return true }
            if g is UIPinchGestureRecognizer { return true }
            if g is UILongPressGestureRecognizer {
                let p = g.location(in: v)
                return v.hitTestEffect(at: p) != nil
            }
            // Pan always begins — either a drag or a scroll.
            return true
        }
    }
}
