import SwiftUI
import UIKit
import MetalKit

/// Interactive Metal-backed ruler+waveform strip. Tap seeks the
/// play head, pinch zooms, horizontal drag scrolls. Vertical is
/// locked — top chrome has no vertical extent.
struct TopChromeMetalGridView: UIViewRepresentable {
    let durationMS: Int
    let pixelsPerMS: CGFloat
    let rulerHeight: CGFloat
    let waveformHeight: CGFloat
    let hasAudio: Bool
    let peaks: [Float]
    @Binding var scrollOffsetX: CGFloat
    let onSeek: (Int) -> Void
    let onPinchZoom: (CGFloat, CGFloat) -> Void

    func makeUIView(context: Context) -> TopChromeMetalMTKView {
        let v = TopChromeMetalMTKView()
        v.coordinator = context.coordinator
        v.installGestures()
        return v
    }
    func updateUIView(_ view: TopChromeMetalMTKView, context: Context) {
        let c = context.coordinator
        c.durationMS = durationMS
        c.pixelsPerMS = pixelsPerMS
        c.rulerHeight = rulerHeight
        c.waveformHeight = waveformHeight
        c.hasAudio = hasAudio
        c.peaks = peaks
        c.scrollOffsetX = scrollOffsetX
        c.onSeek = onSeek
        c.onPinchZoom = onPinchZoom
        c.onUpdateScrollX = { scrollOffsetX = $0 }
        view.setNeedsDisplay()
    }
    func makeCoordinator() -> Coordinator { Coordinator() }

    final class Coordinator {
        let bridge = XLGridMetalBridge(name: "TopChromeGrid")!
        var durationMS: Int = 0
        var pixelsPerMS: CGFloat = 0.1
        var rulerHeight: CGFloat = 24
        var waveformHeight: CGFloat = 48
        var hasAudio: Bool = false
        var peaks: [Float] = []
        var scrollOffsetX: CGFloat = 0
        var onSeek: (Int) -> Void = { _ in }
        var onPinchZoom: (CGFloat, CGFloat) -> Void = { _, _ in }
        var onUpdateScrollX: (CGFloat) -> Void = { _ in }
        var panStartScrollX: CGFloat = 0
    }
}

final class TopChromeMetalMTKView: MTKView, MTKViewDelegate {
    weak var coordinator: TopChromeMetalGridView.Coordinator?
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
        if let layer = self.layer as? CAMetalLayer { layer.isOpaque = false }
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

    private static let niceIntervals: [Int] = [
        10, 20, 50, 100, 200, 500,
        1000, 2000, 5000, 10000, 20000, 30000, 60000,
        120000, 300000, 600000, 1200000, 3600000
    ]
    private func majorIntervalMS(pixelsPerMS: CGFloat) -> Int {
        guard pixelsPerMS > 0 else { return 1000 }
        let target = Double(100 / pixelsPerMS)
        for ms in Self.niceIntervals where Double(ms) >= target { return ms }
        return Self.niceIntervals.last ?? 60000
    }
    private static func fmtTime(_ ms: Int, majorMS: Int) -> String {
        let minutes = ms / 60000
        let seconds = (ms % 60000) / 1000
        let millis  = ms % 1000
        if majorMS < 100 {
            return String(format: "%d:%02d.%03d", minutes, seconds, millis)
        } else if majorMS < 1000 {
            return String(format: "%d:%02d.%02d", minutes, seconds, millis / 10)
        } else {
            return String(format: "%d:%02d", minutes, seconds)
        }
    }

    func draw(in view: MTKView) {
        guard let c = coordinator else { return }
        guard c.bridge.beginFrame() else { return }
        let size = bounds.size
        let bridge = c.bridge

        // Backgrounds.
        bridge.beginFilledRectBatch()
        bridge.appendFilledRectX(0, y: 0, w: size.width, h: c.rulerHeight,
                                  r: 0, g: 0, b: 0, a: 0.2)
        if c.hasAudio {
            bridge.appendFilledRectX(0, y: c.rulerHeight,
                                      w: size.width, h: c.waveformHeight,
                                      r: 0, g: 0, b: 0, a: 0.3)
        } else {
            bridge.appendFilledRectX(0, y: c.rulerHeight,
                                      w: size.width, h: c.waveformHeight,
                                      r: 0.08, g: 0.08, b: 0.08, a: 1.0)
        }
        bridge.flushFilledRectBatch()

        // Ruler ticks + labels.
        if c.pixelsPerMS > 0 && c.durationMS > 0 {
            let major = majorIntervalMS(pixelsPerMS: c.pixelsPerMS)
            let minor = max(1, major / 2)
            bridge.beginLineBatch()
            var labels: [(CGFloat, String)] = []
            let startMS = max(0, (Int(c.scrollOffsetX / c.pixelsPerMS) / minor) * minor)
            var ms = startMS
            while ms <= c.durationMS {
                let x = CGFloat(ms) * c.pixelsPerMS - c.scrollOffsetX
                if x > size.width + 60 { break }
                if x >= -1 {
                    let isMajor = ms % major == 0
                    let tickH: CGFloat = isMajor ? 15 : 8
                    bridge.appendLineX1(x, y1: c.rulerHeight - tickH,
                                         x2: x, y2: c.rulerHeight,
                                         r: 0.5, g: 0.5, b: 0.5, a: 1.0)
                    if isMajor {
                        labels.append((x, Self.fmtTime(ms, majorMS: major)))
                    }
                }
                ms += minor
            }
            bridge.flushLineBatch()
            for lp in labels {
                bridge.drawText(lp.1, atX: lp.0 + 2, y: 2, fontSize: 9,
                                 r: 0.5, g: 0.5, b: 0.5, a: 1.0)
            }
        }

        // Waveform fill + outline.
        if c.hasAudio && c.peaks.count >= 4 {
            let numBuckets = c.peaks.count / 2
            let centerY = c.rulerHeight + c.waveformHeight / 2
            let scale = (c.waveformHeight / 2) * 0.9
            let timelineW = CGFloat(c.durationMS) * c.pixelsPerMS
            if timelineW > 0 {
                let visMinX = c.scrollOffsetX - 4
                let visMaxX = c.scrollOffsetX + size.width + 4
                let firstB = max(0, Int((visMinX / timelineW) * CGFloat(numBuckets)) - 1)
                let lastB = min(numBuckets - 1,
                                 Int((visMaxX / timelineW) * CGFloat(numBuckets)) + 1)
                if firstB <= lastB {
                    for i in firstB..<lastB {
                        let xi = (CGFloat(i)/CGFloat(numBuckets)) * timelineW - c.scrollOffsetX
                        let xj = (CGFloat(i+1)/CGFloat(numBuckets)) * timelineW - c.scrollOffsetX
                        let yiMin = centerY - CGFloat(c.peaks[i*2]) * scale
                        let yiMax = centerY - CGFloat(c.peaks[i*2+1]) * scale
                        let yjMin = centerY - CGFloat(c.peaks[(i+1)*2]) * scale
                        let yjMax = centerY - CGFloat(c.peaks[(i+1)*2+1]) * scale
                        bridge.fillTriangleX1(xi, y1: yiMin,
                                              x2: xi, y2: yiMax,
                                              x3: xj, y3: yjMax,
                                              r: 130/255, g: 178/255, b: 207/255, a: 1.0)
                        bridge.fillTriangleX1(xi, y1: yiMin,
                                              x2: xj, y2: yjMax,
                                              x3: xj, y3: yjMin,
                                              r: 130/255, g: 178/255, b: 207/255, a: 1.0)
                    }
                    bridge.beginLineBatch()
                    var pX: CGFloat = 0
                    var pY: CGFloat = 0
                    for (k, i) in (firstB...lastB).enumerated() {
                        let x = (CGFloat(i)/CGFloat(numBuckets)) * timelineW - c.scrollOffsetX
                        let yMin = centerY - CGFloat(c.peaks[i*2]) * scale
                        if k > 0 {
                            bridge.appendLineX1(pX, y1: pY, x2: x, y2: yMin,
                                                 r: 1, g: 1, b: 1, a: 1)
                        }
                        pX = x; pY = yMin
                    }
                    for (k, i) in (firstB...lastB).reversed().enumerated() {
                        let x = (CGFloat(i)/CGFloat(numBuckets)) * timelineW - c.scrollOffsetX
                        let yMax = centerY - CGFloat(c.peaks[i*2+1]) * scale
                        if k > 0 {
                            bridge.appendLineX1(pX, y1: pY, x2: x, y2: yMax,
                                                 r: 1, g: 1, b: 1, a: 1)
                        }
                        pX = x; pY = yMax
                    }
                    bridge.flushLineBatch()
                }
            }
        }
        bridge.endFrame()
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
        let ms = max(0, min(c.durationMS, Int((p.x + c.scrollOffsetX) / c.pixelsPerMS)))
        c.onSeek(ms)
    }

    @objc func onPan(_ g: UIPanGestureRecognizer) {
        guard let c = coordinator else { return }
        switch g.state {
        case .began:
            c.panStartScrollX = c.scrollOffsetX
        case .changed:
            let t = g.translation(in: self)
            c.onUpdateScrollX(max(0, c.panStartScrollX - t.x))
        default: break
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
        default: break
        }
    }
    private var pinchAnchorX: CGFloat = 0
    private var pinchLastScale: CGFloat = 1
}
