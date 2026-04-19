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
        view.setNeedsDisplay()
    }

    func makeCoordinator() -> Coordinator { Coordinator() }

    final class Coordinator {
        let bridge = XLGridMetalBridge(name: "TimingEffectsGrid")!
        var rows: [SequencerViewModel.RowInfo] = []
        var rowHeight: CGFloat = 24
        var pixelsPerMS: CGFloat = 0.1
        var scrollOffsetX: CGFloat = 0
        var scrollOffsetY: CGFloat = 0
        var onSeek: (Int) -> Void = { _ in }
        var onPinchZoom: (CGFloat, CGFloat) -> Void = { _, _ in }
        var onUpdateScrollX: (CGFloat) -> Void = { _ in }
        var onUpdateScrollY: (CGFloat) -> Void = { _ in }
        var panStartScrollX: CGFloat = 0
        var panStartScrollY: CGFloat = 0
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

        // Brackets + labels.
        bridge.beginLineBatch()
        var labels: [(x: CGFloat, y: CGFloat, text: String)] = []
        y = -c.scrollOffsetY
        for row in c.rows {
            if y + c.rowHeight < 0 { y += c.rowHeight; continue }
            if y > viewport.height { break }
            for effect in row.effects {
                let x1 = CGFloat(effect.startTimeMS) * c.pixelsPerMS - c.scrollOffsetX
                let x2 = CGFloat(effect.endTimeMS) * c.pixelsPerMS - c.scrollOffsetX
                if x2 < 0 || x1 > viewport.width { continue }
                bridge.appendLineX1(x1 + 0.5, y1: y + 1,
                                     x2: x1 + 0.5, y2: y + c.rowHeight - 1,
                                     r: 1, g: 1, b: 1, a: 0.75)
                bridge.appendLineX1(x2 - 0.5, y1: y + 1,
                                     x2: x2 - 0.5, y2: y + c.rowHeight - 1,
                                     r: 1, g: 1, b: 1, a: 0.75)
                if !effect.name.isEmpty {
                    let cx = (x1 + x2) / 2
                    labels.append((cx, y + (c.rowHeight - 11) / 2, effect.name))
                }
            }
            y += c.rowHeight
        }
        bridge.flushLineBatch()

        for l in labels {
            let s = bridge.size(ofText: l.text, fontSize: 9)
            if s.width <= 0 { continue }
            bridge.drawText(l.text, atX: l.x - s.width / 2, y: l.y,
                             fontSize: 9, r: 1, g: 1, b: 1, a: 0.75)
        }
    }

    func installGestures() {
        let tap = UITapGestureRecognizer(target: self, action: #selector(onTap(_:)))
        addGestureRecognizer(tap)
        let pan = UIPanGestureRecognizer(target: self, action: #selector(onPan(_:)))
        addGestureRecognizer(pan)
        let pinch = UIPinchGestureRecognizer(target: self, action: #selector(onPinch(_:)))
        addGestureRecognizer(pinch)
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
            c.panStartScrollX = c.scrollOffsetX
            c.panStartScrollY = c.scrollOffsetY
        case .changed:
            let t = g.translation(in: self)
            c.onUpdateScrollX(max(0, c.panStartScrollX - t.x))
            c.onUpdateScrollY(max(0, c.panStartScrollY - t.y))
        default:
            break
        }
    }

    @objc func onPinch(_ g: UIPinchGestureRecognizer) {
        guard let c = coordinator else { return }
        switch g.state {
        case .began:
            pinchAnchorX = g.location(in: self).x + c.scrollOffsetX
            pinchLastScale = 1
        case .changed:
            let delta = g.scale / pinchLastScale
            pinchLastScale = g.scale
            c.onPinchZoom(delta, pinchAnchorX)
        default:
            break
        }
    }
    private var pinchAnchorX: CGFloat = 0
    private var pinchLastScale: CGFloat = 1
}
