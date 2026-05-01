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
    /// B40: optional scrub-flavoured seek used while the user is
    /// actively dragging the playhead from the ruler. Falls back to
    /// `onSeek` when nil. The view model wires this to a path that
    /// also fires brief audio bursts so the drag is audible.
    var onScrubSeek: ((Int) -> Void)?
    let onPinchZoom: (CGFloat, CGFloat) -> Void
    var onUserInteraction: (() -> Void)?
    // B32 loop-region inputs + callbacks. When `hasLoop`, the region
    // is shaded across both ruler and waveform. Long-press + drag on
    // the ruler fires `onSetLoop(startMS, endMS)` on each .changed so
    // the outer view sees the live extent; long-press over an
    // existing loop (no drag) fires `onLoopMenu(x)` so the outer
    // view can present the loop context menu.
    var loopStartMS: Int = 0
    var loopEndMS: Int = 0
    var hasLoop: Bool = false
    var onSetLoop: ((_ startMS: Int, _ endMS: Int) -> Void)?
    var onLoopMenu: ((_ atXInView: CGFloat) -> Void)?
    /// B41: long-press on the waveform strip surfaces the filter-
    /// variant menu (bass / treble / alto / non-vocals / full).
    var onWaveformMenu: (() -> Void)?
    /// B34 tag positions (0..9; -1 = unset). Drawn as numbered
    /// flags on the ruler.
    var tagPositions: [Int] = Array(repeating: -1, count: 10)
    /// A2: optional onset overlay. When `showOnsets` is true, each
    /// `onsetMS` value draws a faint vertical line across the
    /// waveform strip.
    var showOnsets: Bool = false
    var onsetMS: [Int] = []
    /// A5: optional pitch-contour overlay — flat triples
    /// (timeMS, frequency, confidence). Unvoiced frames (freq=0)
    /// break the polyline so silence doesn't produce a spurious
    /// slope. Colour-coded by pitch class.
    var showPitchContour: Bool = false
    var pitchContour: [Float] = []
    /// A6: when true, the waveform strip renders the STFT
    /// spectrogram instead of the peak polygons. The image is
    /// fetched lazily via `spectrogramFetcher` each time the view
    /// needs to redraw at a new zoom / scroll / size.
    var showSpectrogram: Bool = false
    var spectrogramFetcher: ((Int, Int, Int, Int) -> Data?)? = nil

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
        c.onScrubSeek = onScrubSeek
        c.onPinchZoom = onPinchZoom
        c.onUpdateScrollX = { scrollOffsetX = $0 }
        c.onUserInteraction = onUserInteraction
        c.loopStartMS = loopStartMS
        c.loopEndMS = loopEndMS
        c.hasLoop = hasLoop
        c.onSetLoop = onSetLoop
        c.onLoopMenu = onLoopMenu
        c.onWaveformMenu = onWaveformMenu
        c.showOnsets = showOnsets
        c.onsetMS = onsetMS
        c.showPitchContour = showPitchContour
        c.pitchContour = pitchContour
        c.showSpectrogram = showSpectrogram
        c.spectrogramFetcher = spectrogramFetcher
        c.tagPositions = tagPositions
        view.setNeedsDisplay()
    }
    func makeCoordinator() -> Coordinator { Coordinator() }

    final class Coordinator {
        let bridge = XLGridMetalBridge(name: "TopChromeGrid")
        var durationMS: Int = 0
        var pixelsPerMS: CGFloat = 0.1
        var rulerHeight: CGFloat = 24
        var waveformHeight: CGFloat = 48
        var hasAudio: Bool = false
        var peaks: [Float] = []
        var scrollOffsetX: CGFloat = 0
        var onSeek: (Int) -> Void = { _ in }
        var onScrubSeek: ((Int) -> Void)?
        var onPinchZoom: (CGFloat, CGFloat) -> Void = { _, _ in }
        var onUpdateScrollX: (CGFloat) -> Void = { _ in }
        var onUserInteraction: (() -> Void)?
        var panStartScrollX: CGFloat = 0
        // B39: set on `.began` when the pan started in the ruler
        // strip (y < rulerHeight); `.changed` ticks drive continuous
        // `onSeek` instead of the scroll path.
        var scrubbingFromRuler: Bool = false
        // B32 loop-region inputs + live drag state.
        var loopStartMS: Int = 0
        var loopEndMS: Int = 0
        var hasLoop: Bool = false
        var onSetLoop: ((Int, Int) -> Void)?
        var onLoopMenu: ((CGFloat) -> Void)?
        var onWaveformMenu: (() -> Void)?
        var loopDragAnchorMS: Int?
        var loopDragCurrentMS: Int?
        // A2 onset overlay.
        var showOnsets: Bool = false
        var onsetMS: [Int] = []
        // A5 pitch overlay — flat [t,f,c,...] triples.
        var showPitchContour: Bool = false
        var pitchContour: [Float] = []
        // A6 spectrogram state.
        var showSpectrogram: Bool = false
        var spectrogramFetcher: ((Int, Int, Int, Int) -> Data?)? = nil
        // Cache key for the spectrogram texture so we only re-upload
        // when zoom / scroll / size actually change.
        var spectrogramCacheKey: String = ""
        // B34 numbered-tag positions (0..9).
        var tagPositions: [Int] = Array(repeating: -1, count: 10)
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

        // Backgrounds — match the SwiftUI row-header chrome
        // (`Color(white: 0.12)`) so the ruler + waveform strip reads
        // as part of the same dark-mode surface instead of a pure-
        // black cutout.
        let bgGray: CGFloat = 0.12
        bridge.beginFilledRectBatch()
        bridge.appendFilledRectX(0, y: 0, w: size.width, h: c.rulerHeight,
                                  r: bgGray, g: bgGray, b: bgGray, a: 1.0)
        bridge.appendFilledRectX(0, y: c.rulerHeight,
                                  w: size.width, h: c.waveformHeight,
                                  r: bgGray, g: bgGray, b: bgGray, a: 1.0)
        bridge.flushFilledRectBatch()

        // B32 loop-region highlight. Draws whichever of the
        // persisted region or the in-flight drag range is active.
        let activeLoopStart: Int?
        let activeLoopEnd: Int?
        if let a = c.loopDragAnchorMS, let b = c.loopDragCurrentMS {
            activeLoopStart = min(a, b)
            activeLoopEnd = max(a, b)
        } else if c.hasLoop {
            activeLoopStart = c.loopStartMS
            activeLoopEnd = c.loopEndMS
        } else {
            activeLoopStart = nil
            activeLoopEnd = nil
        }
        if let ls = activeLoopStart, let le = activeLoopEnd, le > ls,
           c.pixelsPerMS > 0 {
            let x1 = CGFloat(ls) * c.pixelsPerMS - c.scrollOffsetX
            let x2 = CGFloat(le) * c.pixelsPerMS - c.scrollOffsetX
            let totalH = c.rulerHeight + c.waveformHeight
            bridge.beginFilledRectBatch()
            // Soft blue band across the whole strip.
            bridge.appendFilledRectX(x1, y: 0, w: max(1, x2 - x1), h: totalH,
                                      r: 0.35, g: 0.60, b: 1.0, a: 0.18)
            bridge.flushFilledRectBatch()
            // Boundary lines at the edges for clarity.
            bridge.beginLineBatch()
            let bc: (CGFloat, CGFloat, CGFloat) = (0.45, 0.70, 1.0)
            bridge.appendLineX1(x1, y1: 0, x2: x1, y2: totalH,
                                 r: bc.0, g: bc.1, b: bc.2, a: 0.95)
            bridge.appendLineX1(x2, y1: 0, x2: x2, y2: totalH,
                                 r: bc.0, g: bc.1, b: bc.2, a: 0.95)
            bridge.flushLineBatch()
        }

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
                                         r: 0.65, g: 0.65, b: 0.65, a: 1.0)
                    if isMajor {
                        labels.append((x, Self.fmtTime(ms, majorMS: major)))
                    }
                }
                ms += minor
            }
            bridge.flushLineBatch()
            for lp in labels {
                // White labels to match the row-header text colour.
                bridge.drawText(lp.1, atX: lp.0 + 2, y: 2, fontSize: 9,
                                 r: 1.0, g: 1.0, b: 1.0, a: 1.0)
            }
        }

        // B34 numbered-tag flags on the ruler. Drawn after ticks so
        // the flag sits on top of the tick/label pair. Each tag gets a
        // distinct hue on a 10-step wheel plus a small numeric label.
        if c.pixelsPerMS > 0 && c.durationMS > 0 {
            let flagH: CGFloat = 11
            let flagW: CGFloat = 10
            let flagY: CGFloat = 1
            for i in 0..<min(10, c.tagPositions.count) {
                let pos = c.tagPositions[i]
                if pos < 0 { continue }
                let x = CGFloat(pos) * c.pixelsPerMS - c.scrollOffsetX
                if x < -flagW || x > size.width + flagW { continue }
                let hue = Float(i) / 10.0
                let (r, g, b) = hslToRGB(h: hue, s: 0.75, l: 0.55)
                // Stem (vertical line down to the ruler baseline).
                bridge.beginLineBatch()
                bridge.appendLineX1(x, y1: flagY,
                                     x2: x, y2: c.rulerHeight,
                                     r: CGFloat(r), g: CGFloat(g),
                                     b: CGFloat(b), a: 1.0)
                bridge.flushLineBatch()
                // Flag rectangle (triangle-pair fill).
                bridge.fillTriangleX1(x, y1: flagY,
                                      x2: x + flagW, y2: flagY + flagH / 2,
                                      x3: x, y3: flagY + flagH,
                                      r: CGFloat(r), g: CGFloat(g),
                                      b: CGFloat(b), a: 0.95)
                // Number inside the flag.
                bridge.drawText("\(i)", atX: x + 2, y: flagY + 1,
                                 fontSize: 9, r: 0, g: 0, b: 0, a: 1.0)
            }
        }

        // A6: spectrogram view. When active, the waveform strip
        // renders the STFT magnitude spectrum instead of the peak
        // polygons. Fetch the BGRA image covering the visible
        // time range at the strip's pixel size and upload to a
        // cached texture keyed on (range, size).
        var spectrogramDrawn = false
        if c.hasAudio && c.showSpectrogram, let fetch = c.spectrogramFetcher, c.pixelsPerMS > 0 {
            let scale = CGFloat(view.contentScaleFactor)
            let stripW = Int(size.width * scale)
            let stripH = Int(c.waveformHeight * scale)
            let visStartMS = Int(c.scrollOffsetX / c.pixelsPerMS)
            let visEndMS = Int((c.scrollOffsetX + size.width) / c.pixelsPerMS)
            if stripW > 8 && stripH > 8 && visEndMS > visStartMS {
                let key = "\(visStartMS)-\(visEndMS)-\(stripW)x\(stripH)"
                if key != c.spectrogramCacheKey,
                   let data = fetch(visStartMS, visEndMS, stripW, stripH) {
                    // `replaceTextureNamed:` evicts any cached entry
                    // with the same name before (re)uploading — needed
                    // because `ensureTextureNamed:` short-circuits on
                    // existing keys, so the texture would otherwise
                    // stick at the first-upload content and never
                    // scroll / zoom.
                    bridge.replaceTextureNamed("topChromeSpectrogram",
                                                bgraData: data,
                                                w: Int32(stripW),
                                                h: Int32(stripH))
                    c.spectrogramCacheKey = key
                }
                bridge.drawTextureNamed("topChromeSpectrogram",
                                         x: 0,
                                         y: c.rulerHeight,
                                         w: size.width,
                                         h: c.waveformHeight)
                spectrogramDrawn = true
            }
        }

        // Waveform fill + outline. Bridge returns `{min, max, rms}`
        // triples per bucket (A10) — stride 3. RMS is drawn as a
        // lighter inner polygon so "loud but compressed" sections
        // (where peaks plateau) still show a meaningful shape.
        if !spectrogramDrawn && c.hasAudio && c.peaks.count >= 6 {
            let numBuckets = c.peaks.count / 3
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
                        let yiMin = centerY - CGFloat(c.peaks[i*3]) * scale
                        let yiMax = centerY - CGFloat(c.peaks[i*3+1]) * scale
                        let yjMin = centerY - CGFloat(c.peaks[(i+1)*3]) * scale
                        let yjMax = centerY - CGFloat(c.peaks[(i+1)*3+1]) * scale
                        bridge.fillTriangleX1(xi, y1: yiMin,
                                              x2: xi, y2: yiMax,
                                              x3: xj, y3: yjMax,
                                              r: 130/255, g: 178/255, b: 207/255, a: 1.0)
                        bridge.fillTriangleX1(xi, y1: yiMin,
                                              x2: xj, y2: yjMax,
                                              x3: xj, y3: yjMin,
                                              r: 130/255, g: 178/255, b: 207/255, a: 1.0)
                    }
                    // RMS overlay: a narrower, brighter band centred
                    // on the axis. Symmetric ±rms since RMS is always
                    // >= 0. Skip buckets with ~zero RMS to avoid a
                    // razor line on silence.
                    for i in firstB..<lastB {
                        let ri = CGFloat(c.peaks[i*3+2])
                        let rj = CGFloat(c.peaks[(i+1)*3+2])
                        if ri < 0.001 && rj < 0.001 { continue }
                        let xi = (CGFloat(i)/CGFloat(numBuckets)) * timelineW - c.scrollOffsetX
                        let xj = (CGFloat(i+1)/CGFloat(numBuckets)) * timelineW - c.scrollOffsetX
                        let yiTop = centerY - ri * scale
                        let yiBot = centerY + ri * scale
                        let yjTop = centerY - rj * scale
                        let yjBot = centerY + rj * scale
                        bridge.fillTriangleX1(xi, y1: yiTop,
                                              x2: xi, y2: yiBot,
                                              x3: xj, y3: yjBot,
                                              r: 220/255, g: 236/255, b: 255/255, a: 0.85)
                        bridge.fillTriangleX1(xi, y1: yiTop,
                                              x2: xj, y2: yjBot,
                                              x3: xj, y3: yjTop,
                                              r: 220/255, g: 236/255, b: 255/255, a: 0.85)
                    }
                    bridge.beginLineBatch()
                    var pX: CGFloat = 0
                    var pY: CGFloat = 0
                    for (k, i) in (firstB...lastB).enumerated() {
                        let x = (CGFloat(i)/CGFloat(numBuckets)) * timelineW - c.scrollOffsetX
                        let yMin = centerY - CGFloat(c.peaks[i*3]) * scale
                        if k > 0 {
                            bridge.appendLineX1(pX, y1: pY, x2: x, y2: yMin,
                                                 r: 1, g: 1, b: 1, a: 1)
                        }
                        pX = x; pY = yMin
                    }
                    for (k, i) in (firstB...lastB).reversed().enumerated() {
                        let x = (CGFloat(i)/CGFloat(numBuckets)) * timelineW - c.scrollOffsetX
                        let yMax = centerY - CGFloat(c.peaks[i*3+1]) * scale
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

        // A2 onset overlay. Vertical amber lines across the ruler +
        // waveform strip at each detected onset. Skip when zoomed way
        // out and the strip would be a solid wall of lines (nothing
        // readable at <2 px per onset).
        if c.hasAudio && c.showOnsets && !c.onsetMS.isEmpty && c.pixelsPerMS > 0 {
            let top = 1.0 as CGFloat
            let bottom = c.rulerHeight + c.waveformHeight - 1
            bridge.beginLineBatch()
            for ms in c.onsetMS {
                let x = CGFloat(ms) * c.pixelsPerMS - c.scrollOffsetX
                if x < -1 || x > size.width + 1 { continue }
                bridge.appendLineX1(x, y1: top, x2: x, y2: bottom,
                                     r: 255/255, g: 170/255, b: 60/255, a: 0.85)
            }
            bridge.flushLineBatch()
        }

        // A5 pitch-contour overlay. Line segments colour-coded by
        // pitch class (12 colours on a hue wheel). Frequency maps to
        // a log scale across the waveform strip; unvoiced frames
        // (freq == 0) break the polyline.
        if c.hasAudio && c.showPitchContour && c.pitchContour.count >= 6 {
            let logMin = log2(80.0 as Float)    // low E bass ~82 Hz
            let logMax = log2(1000.0 as Float)  // soprano high
            let logRange = logMax - logMin
            let yTop = c.rulerHeight + 2
            let yBottom = c.rulerHeight + c.waveformHeight - 2
            let yRange = yBottom - yTop
            let triples = c.pitchContour.count / 3
            bridge.beginLineBatch()
            var prevHasPitch = false
            var prevX: CGFloat = 0
            var prevY: CGFloat = 0
            for i in 0..<triples {
                let t = c.pitchContour[i*3]
                let f = c.pitchContour[i*3 + 1]
                let x = CGFloat(t) * c.pixelsPerMS - c.scrollOffsetX
                if x < -8 || x > size.width + 8 { prevHasPitch = false; continue }
                if f <= 0 { prevHasPitch = false; continue }
                let logF = log2(f)
                let norm = max(0, min(1, (logF - logMin) / logRange))
                let y = yBottom - CGFloat(norm) * yRange
                // Colour by pitch class (semitones from A). `hue` is
                // the MIDI-note-class position on [0,1].
                let semitones = 12 * (logF - log2(440.0 as Float))
                let noteClass = ((Int(semitones.rounded()) % 12) + 12) % 12
                let hue = Float(noteClass) / 12.0
                let (r, g, b) = hslToRGB(h: hue, s: 0.7, l: 0.55)
                if prevHasPitch {
                    bridge.appendLineX1(prevX, y1: prevY, x2: x, y2: y,
                                         r: CGFloat(r), g: CGFloat(g), b: CGFloat(b), a: 0.95)
                }
                prevX = x; prevY = y; prevHasPitch = true
            }
            bridge.flushLineBatch()
        }
        bridge.endFrame()
    }

    /// HSL → RGB (each 0..1). Used by the pitch-contour colour code
    /// to spread the 12 pitch classes around the hue wheel.
    private func hslToRGB(h: Float, s: Float, l: Float) -> (Float, Float, Float) {
        func hue2rgb(_ p: Float, _ q: Float, _ t: Float) -> Float {
            var t = t
            if t < 0 { t += 1 }
            if t > 1 { t -= 1 }
            if t < 1.0/6 { return p + (q - p) * 6 * t }
            if t < 0.5   { return q }
            if t < 2.0/3 { return p + (q - p) * (2.0/3 - t) * 6 }
            return p
        }
        if s == 0 { return (l, l, l) }
        let q = l < 0.5 ? l * (1 + s) : l + s - l * s
        let p = 2 * l - q
        return (hue2rgb(p, q, h + 1.0/3), hue2rgb(p, q, h), hue2rgb(p, q, h - 1.0/3))
    }

    func installGestures() {
        let tap = UITapGestureRecognizer(target: self, action: #selector(onTap(_:)))
        addGestureRecognizer(tap)
        let pan = UIPanGestureRecognizer(target: self, action: #selector(onPan(_:)))
        pan.allowedScrollTypesMask = .all   // B95: trackpad + scroll-wheel scroll
        addGestureRecognizer(pan)
        let pinch = UIPinchGestureRecognizer(target: self, action: #selector(onPinch(_:)))
        addGestureRecognizer(pinch)
        // B32 loop region: long-press on the ruler establishes a
        // loop region; dragging extends it. Over an existing loop
        // band, a plain long-press pops the loop context menu.
        let lp = UILongPressGestureRecognizer(target: self,
                                                action: #selector(onLongPressLoop(_:)))
        lp.minimumPressDuration = 0.5
        // Allow drag during long-press so the finger can sweep out
        // the region without cancelling the gesture.
        lp.allowableMovement = 1_000
        addGestureRecognizer(lp)
    }

    @objc func onLongPressLoop(_ g: UILongPressGestureRecognizer) {
        guard let c = coordinator, c.pixelsPerMS > 0 else { return }
        let p = g.location(in: self)
        // B41: long-press in the waveform strip (below the ruler)
        // surfaces the filter-variant menu on .began. Only ruler
        // presses proceed to the loop-region path below.
        if p.y >= c.rulerHeight {
            if g.state == .began, c.hasAudio {
                c.onWaveformMenu?()
            }
            return
        }
        let ms = max(0, min(c.durationMS,
                             Int((p.x + c.scrollOffsetX) / c.pixelsPerMS)))
        switch g.state {
        case .began:
            // If the press lands inside the existing loop band AND
            // the finger hasn't moved yet, defer to the .changed
            // tick to decide between menu vs re-drag. Desktop
            // equivalent: shift-click to set vs right-click to get
            // menu — iPad collapses both to long-press so we use
            // drag-distance as the discriminator.
            c.loopDragAnchorMS = ms
            c.loopDragCurrentMS = ms
            setNeedsDisplay()
        case .changed:
            c.loopDragCurrentMS = ms
            if let anchor = c.loopDragAnchorMS {
                let lo = min(anchor, ms), hi = max(anchor, ms)
                if hi > lo {
                    c.onSetLoop?(lo, hi)
                }
            }
            setNeedsDisplay()
        case .ended:
            defer {
                c.loopDragAnchorMS = nil
                c.loopDragCurrentMS = nil
                setNeedsDisplay()
            }
            if let anchor = c.loopDragAnchorMS,
               let current = c.loopDragCurrentMS {
                let lo = min(anchor, current), hi = max(anchor, current)
                // Tiny drags count as "just a long-press" — surface
                // the menu if the press landed on the loop band.
                let dragPx = CGFloat(abs(current - anchor)) * c.pixelsPerMS
                if dragPx < 6, c.hasLoop,
                   ms >= c.loopStartMS, ms <= c.loopEndMS {
                    c.onLoopMenu?(p.x)
                } else if hi > lo {
                    c.onSetLoop?(lo, hi)
                }
            }
        case .cancelled, .failed:
            c.loopDragAnchorMS = nil
            c.loopDragCurrentMS = nil
            setNeedsDisplay()
        default:
            break
        }
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
            let p = g.location(in: self)
            // B39: drag starting in the ruler strip continuously
            // updates the play head; drag in the waveform area keeps
            // its existing scroll behavior.
            c.scrubbingFromRuler = (p.y < c.rulerHeight) && (c.pixelsPerMS > 0)
            if c.scrubbingFromRuler {
                let ms = max(0, min(c.durationMS,
                                     Int((p.x + c.scrollOffsetX) / c.pixelsPerMS)))
                (c.onScrubSeek ?? c.onSeek)(ms)
            } else {
                c.panStartScrollX = c.scrollOffsetX
            }
            c.onUserInteraction?()
        case .changed:
            let p = g.location(in: self)
            if c.scrubbingFromRuler, c.pixelsPerMS > 0 {
                let ms = max(0, min(c.durationMS,
                                     Int((p.x + c.scrollOffsetX) / c.pixelsPerMS)))
                (c.onScrubSeek ?? c.onSeek)(ms)
            } else {
                let t = g.translation(in: self)
                c.onUpdateScrollX(max(0, c.panStartScrollX - t.x))
            }
            c.onUserInteraction?()
        case .ended, .cancelled, .failed:
            c.scrubbingFromRuler = false
        default: break
        }
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
        default: break
        }
    }
    private var pinchAnchorX: CGFloat = 0
    private var pinchLastScale: CGFloat = 1
}
