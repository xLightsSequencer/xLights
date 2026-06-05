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
    /// Full selection set (B1 marquee multi-select). Every member is
    /// drawn with the selection bracket colour. When `count == 1` the
    /// single member coincides with `selection` and shows edge + fade
    /// drag handles; when `count > 1`, handles are suppressed (bulk
    /// drag/resize are follow-up work).
    let selectedEffects: Set<SequencerViewModel.EffectSelection>
    let activeDrag: SequencerViewModel.ActiveDrag?
    let timingMarkTimesMS: [Int]
    /// Bumped by the view model whenever a render kickoff has
    /// completed. Only purpose on this view is to appear in the
    /// parameter list so SwiftUI re-runs `updateUIView` (which calls
    /// `setNeedsDisplay`) and the grid picks up the newly populated
    /// effect-background display lists for `DrawEffectBackground`.
    let renderedBackgroundsRevision: Int
    /// Bumped on every inspector setting edit, before the render
    /// finishes. Lets the grid redraw immediately so effects whose
    /// `DrawEffectBackground` computes its geometry from the current
    /// palette/settings at draw time (ColorWash, On without ramps,
    /// etc.) show the new color instantly without waiting for the
    /// render-completion poll. Cached-display-list effects still
    /// update a second time when `renderedBackgroundsRevision` bumps.
    let inspectorRevision: Int

    // Shared scroll state (pan writes, other canvases read)
    @Binding var scrollOffsetX: CGFloat
    @Binding var scrollOffsetY: CGFloat

    // Actions
    let actions: EffectCanvasActions
    let stateLookup: EffectStateLookup
    let fadeProvider: (Int, Int) -> (Float, Float)
    let iconProvider: (_ effectName: String, _ bucket: Int) -> Data?
    let document: XLSequenceDocument
    /// Fired on pan/pinch `.began` and during `.changed` ticks so the
    /// outer view can note when the user is actively scrolling. Used by
    /// B93 follow-playhead to suppress auto-scroll during + briefly
    /// after user interaction.
    var onUserInteraction: (() -> Void)?
    /// Called when a two-finger marquee drag lands, passing the set of
    /// selections that fall inside the rectangle. Wired to
    /// `SequencerViewModel.setMultiSelection`.
    var onMarqueeSelect: ((Set<SequencerViewModel.EffectSelection>) -> Void)?
    /// Pencil 2 double-tap / Pencil Pro squeeze → action. Outer
    /// view wires to `viewModel.undo()`.
    var onPencilTapAction: (() -> Void)?

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
        ctx.selectedEffects = selectedEffects
        ctx.activeDrag = activeDrag
        ctx.timingMarkTimesMS = timingMarkTimesMS
        ctx.scrollOffsetX = scrollOffsetX
        ctx.scrollOffsetY = scrollOffsetY
        ctx.actions = actions
        ctx.stateLookup = stateLookup
        ctx.fadeProvider = fadeProvider
        ctx.iconProvider = iconProvider
        // B25: refresh the bracket palette from the document's
        // ColorManager-fed cache. Cheap — four C++ map lookups
        // behind ObjC dispatch.
        var r: CGFloat = 0, g: CGFloat = 0, b: CGFloat = 0
        document.bracketColor(forState: .default, outR: &r, outG: &g, outB: &b)
        ctx.bracketDefault = (r, g, b)
        document.bracketColor(forState: .selected, outR: &r, outG: &g, outB: &b)
        ctx.bracketSelected = (r, g, b)
        document.bracketColor(forState: .locked, outR: &r, outG: &g, outB: &b)
        ctx.bracketLocked = (r, g, b)
        document.bracketColor(forState: .disabled, outR: &r, outG: &g, outB: &b)
        ctx.bracketDisabled = (r, g, b)
        ctx.onUpdateScrollX = { newX in scrollOffsetX = newX }
        ctx.onUpdateScrollY = { newY in scrollOffsetY = newY }
        ctx.onUserInteraction = onUserInteraction
        ctx.onMarqueeSelect = onMarqueeSelect
        ctx.onPencilTapAction = onPencilTapAction
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
        /// B96: active momentum-scroll display link, if one is
        /// coasting. Invalidated when a new pan begins or the
        /// view tears down.
        var momentumLink: CADisplayLink?
        var selectedEffects: Set<SequencerViewModel.EffectSelection> = []
        var activeDrag: SequencerViewModel.ActiveDrag?
        var timingMarkTimesMS: [Int] = []
        var scrollOffsetX: CGFloat = 0
        var scrollOffsetY: CGFloat = 0
        var actions = EffectCanvasActions()
        var stateLookup = EffectStateLookup()
        var fadeProvider: (Int, Int) -> (Float, Float) = { _, _ in (0, 0) }
        var iconProvider: (String, Int) -> Data? = { _, _ in nil }
        // B25: per-state bracket colours sourced from the show folder's
        // <colors> palette via XLSequenceDocument. Cached once per
        // updateUIView pass — dirt cheap (4 bridge calls) and picks
        // up palette edits on the next SwiftUI refresh.
        var bracketDefault:  (CGFloat, CGFloat, CGFloat) = (0.75, 0.75, 0.75)
        var bracketSelected: (CGFloat, CGFloat, CGFloat) = (0.80, 0.40, 1.00)
        var bracketLocked:   (CGFloat, CGFloat, CGFloat) = (0.78, 0.00, 0.00)
        var bracketDisabled: (CGFloat, CGFloat, CGFloat) = (0.78, 0.78, 0.00)
        var onUpdateScrollX: (CGFloat) -> Void = { _ in }
        var onUpdateScrollY: (CGFloat) -> Void = { _ in }
        var onUserInteraction: (() -> Void)?
        var onMarqueeSelect: ((Set<SequencerViewModel.EffectSelection>) -> Void)?

        // Marquee-select state. Stored in *world* (content) coords so
        // the rectangle stays anchored correctly if the user scrolls
        // the viewport while a marquee is in flight.
        var marqueeStartWorld: CGPoint?
        var marqueeCurrentWorld: CGPoint?

        // B30 pointer-hover state. Fired only by Magic Keyboard /
        // trackpad (UIHoverGestureRecognizer never triggers for
        // finger touches). When non-nil, the renderer paints a
        // handle highlight on the hovered zone.
        struct HoverHit {
            let rowIndex: Int
            let effectIndex: Int
            let zone: EffectsMetalGridMTKView.HitZone
        }
        var hover: HoverHit?

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
            // Pencil-Pro-squeeze-shared-edge (see pencilSqueezeActive).
            // When a resize drag starts with the Pencil squeeze held
            // AND there's a neighbouring effect butted against the
            // dragged edge (zero-gap), this captures the neighbour
            // so `updateEffectDrag` can grow one while shrinking the
            // other — the shared boundary slides for both effects.
            var pairedEffectIndex: Int?
            var pairedOrigStartMS: Int = 0
            var pairedOrigEndMS: Int = 0
        }
        var drag: DragLocal?
        var liveStartMS: Int?
        var liveEndMS: Int?
        var liveFadeInSec: Float?
        var liveFadeOutSec: Float?
        var liveRowId: Int?
        var liveDropInvalid: Bool = false
        // Shared-edge live state for the paired neighbour during a
        // squeeze+resize drag. Drawing substitutes these for the
        // neighbour's stored range while the drag is in flight.
        var pairedLiveStartMS: Int?
        var pairedLiveEndMS: Int?

        // Pencil Pro squeeze state. `.began` flips this true;
        // `.ended` flips it false. `beginEffectDrag` samples the
        // flag to decide whether to enter shared-edge mode.
        var pencilSqueezeActive: Bool = false
        // True while a paired drag is in flight consuming the
        // squeeze. Suppresses the `squeeze → undo` action on
        // `.ended` so a resize-while-squeezing doesn't also undo.
        var pencilSqueezeConsumedByDrag: Bool = false

        // Pan origin in world coords for delta math that stays
        // consistent when we scroll the content during the drag.
        var panStartScrollX: CGFloat = 0
        var panStartScrollY: CGFloat = 0

        // Auto-scroll during drag.
        var autoScrollLink: CADisplayLink?
        var autoScrollSpeedX: CGFloat = 0
        var autoScrollSpeedY: CGFloat = 0
        var lastTouchInSelf: CGPoint = .zero

        // Apple Pencil precision. Set by the MTKView's `touchesBegan`
        // override; cleared on `touchesEnded`/`touchesCancelled`.
        // When true, hit-test code uses a narrower slop on edge +
        // fade handles (Pencil input is precise enough to aim
        // inside those tiny zones without the finger's forgiveness
        // budget). Never fires for trackpad / Magic Keyboard because
        // those don't produce UITouches.
        var pencilActive: Bool = false

        // UIPencilInteraction routes Pencil 2 double-tap / Pencil
        // Pro squeeze to a handler callback. The handler calls
        // `viewModel.undo()` — the most universally useful reactive
        // action for this grid. The user can still override the
        // system-wide Pencil tap action in Settings ▸ Apple Pencil.
        var onPencilTapAction: (() -> Void)?

        // Reference to the document is needed for icon lookups
        // (width/height of the texture) outside the provider.
        unowned let document: XLSequenceDocument

        init(document: XLSequenceDocument) {
            self.document = document
            self.bridge = XLGridMetalBridge(name: "ModelEffectsGrid")
        }
    }
}

/// The MTKView subclass. Owns gesture recognizers and is the
/// entry point for `draw(_:)`. Pulls state off the
/// `EffectsMetalGridView.Coordinator` owned by its Representable.
final class EffectsMetalGridMTKView: MTKView, MTKViewDelegate, UIPencilInteractionDelegate {
    weak var coordinator: EffectsMetalGridView.Coordinator?
    private var gestureDelegate: GestureDelegate?
    private weak var ownPan: UIPanGestureRecognizer?
    /// Pinch recognizer (zoom). Disabled while the marquee long-
    /// press owns the two-finger gesture so pinch can't fire mid-
    /// marquee. Re-enabled on marquee end.
    private weak var ownPinch: UIPinchGestureRecognizer?
    private let vFadeHitStrip: CGFloat = 7
    private let minIconWidth: CGFloat = 14
    private var pencilInteraction: UIPencilInteraction?

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

        // Install the pencil interaction on first layout — delaying
        // until here avoids ordering problems with the window/scene
        // hierarchy that Pencil interactions are sensitive to.
        if pencilInteraction == nil {
            let pi = UIPencilInteraction()
            pi.delegate = self
            self.addInteraction(pi)
            pencilInteraction = pi
        }
    }

    // UIPencilInteractionDelegate. Three delegate methods cover the
    // full matrix of Pencil generations × iOS versions:
    //
    //   - `pencilInteractionDidTap(_:)` — iOS 12.1+: Pencil 2
    //     double-tap on the barrel.
    //   - `pencilInteraction(_:didReceiveTap:)` — iOS 17.5+ unified
    //     tap API (Pencil 2 double-tap, newer calls route through
    //     here instead of the legacy method).
    //   - `pencilInteraction(_:didReceiveSqueeze:)` — iOS 17.5+
    //     Pencil Pro squeeze. Has phases (.began / .changed /
    //     .ended); we fire on `.ended` so a partial squeeze that's
    //     released doesn't register.
    //
    // All three ultimately route to the same `onPencilTapAction`.
    func pencilInteractionDidTap(_ interaction: UIPencilInteraction) {
        coordinator?.onPencilTapAction?()
    }

    func pencilInteraction(_ interaction: UIPencilInteraction,
                            didReceiveTap tap: UIPencilInteraction.Tap) {
        coordinator?.onPencilTapAction?()
    }

    func pencilInteraction(_ interaction: UIPencilInteraction,
                            didReceiveSqueeze squeeze: UIPencilInteraction.Squeeze) {
        guard let c = coordinator else { return }
        switch squeeze.phase {
        case .began:
            c.pencilSqueezeActive = true
            c.pencilSqueezeConsumedByDrag = false
        case .ended:
            // Only fire the undo action if the squeeze wasn't
            // consumed by a shared-edge resize drag — otherwise the
            // drag committed + squeeze's release would both register
            // as "the user wants undo now."
            if !c.pencilSqueezeConsumedByDrag {
                c.onPencilTapAction?()
            }
            c.pencilSqueezeActive = false
            c.pencilSqueezeConsumedByDrag = false
        case .cancelled:
            c.pencilSqueezeActive = false
            c.pencilSqueezeConsumedByDrag = false
        default:
            break
        }
    }

    // MARK: - Apple Pencil detection
    //
    // Track when the primary incoming touch is a Pencil so
    // `hitTestEffect` can tighten its slop. UIKit dispatches
    // `touchesBegan` before gesture recognizers fire, so we get a
    // chance to stash the flag before hit-testing runs. Finger +
    // Pencil can be active simultaneously (iPadOS supports both at
    // once); we only flip `pencilActive` true while a Pencil touch
    // is down and clear on its own end-of-life event.
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
                // Shared-edge mode (Pencil Pro squeeze + edge drag):
                // the paired neighbour's range also renders at its
                // live position so both effects visibly animate the
                // shared boundary.
                let isPaired = (c.drag?.rowIndex == row.id
                                 && c.drag?.pairedEffectIndex == eIdx)
                let startMS: Int
                let endMS: Int
                if isDragged {
                    startMS = c.activeDrag?.liveStartMS ?? effect.startTimeMS
                    endMS   = c.activeDrag?.liveEndMS   ?? effect.endTimeMS
                } else if isPaired,
                          let ps = c.pairedLiveStartMS,
                          let pe = c.pairedLiveEndMS {
                    startMS = ps
                    endMS   = pe
                } else {
                    startMS = effect.startTimeMS
                    endMS   = effect.endTimeMS
                }
                let x1 = CGFloat(startMS) * c.pixelsPerMS - c.scrollOffsetX
                let x2 = CGFloat(endMS)   * c.pixelsPerMS - c.scrollOffsetX
                if x2 < 0 || x1 > viewport.width { continue }
                // Single-select when count <= 1 uses c.selection. For
                // multi-select we treat every member of `selectedEffects`
                // as selected; `selection` is nil in that case.
                let isSel: Bool
                if c.selectedEffects.count > 1 {
                    isSel = c.selectedEffects.contains(where: {
                        $0.rowIndex == row.id && $0.effectIndex == eIdx
                    })
                } else {
                    isSel = (c.selection?.rowIndex == row.id
                             && c.selection?.effectIndex == eIdx)
                }
                let locked = c.stateLookup.isLocked(row.id, eIdx)
                let disabled = c.stateLookup.isDisabled(row.id, eIdx)
                // B25: ColorManager-sourced bracket palette so user-
                // customised desktop colours show up here too.
                let col: (CGFloat, CGFloat, CGFloat)
                if disabled      { col = c.bracketDisabled }
                else if isSel    { col = c.bracketSelected }
                else if locked   { col = c.bracketLocked }
                else             { col = c.bracketDefault }
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
                    stroke: c.bracketSelected,
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
                if !e.name.isEmpty {
                    let ns = iconRight + 4
                    let nMax = e.x2 - 3
                    let avail = nMax - ns
                    if avail > 8 {
                        // B29: mirror the font-stepping used when
                        // drawing the label below so the horizontal
                        // bracket line doesn't cut through the text.
                        for fs in [nameFontSize, 9, 8, 7] as [CGFloat] {
                            let labelW = bridge.size(ofText: e.name, fontSize: fs).width
                            if labelW <= avail {
                                rightStart = ns + labelW + 2
                                break
                            }
                        }
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
        let screenScale = UITraitCollection.current.displayScale
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
            let iconTop = e.top + (e.bottom - e.top - iconSize) / 2
            let iconBottom = iconTop + iconSize
            if let data = c.iconProvider(e.name, bucket) {
                let key = "\(e.name)@\(bucket)"
                c.bridge.ensureTextureNamed(key, bgraData: data,
                                             w: Int32(bucket), h: Int32(bucket))
                c.bridge.drawTextureNamed(key, x: iconLeft,
                                           y: iconTop,
                                           w: iconSize, h: iconSize)
            }
            // Thin outline around the icon rect. Mirrors desktop's
            // `lines->AddRectAsLines(...)` call on every icon
            // (EffectsGrid.cpp:6832). Critical for the Off effect —
            // whose XPM is a solid-black square that would otherwise
            // vanish against the dark grid — and it visually ties the
            // other icons to their cell the same way desktop does.
            bridge.beginLineBatch()
            let oR = e.stroke.0, oG = e.stroke.1, oB = e.stroke.2
            bridge.appendLineX1(iconLeft - 0.5, y1: iconTop - 0.5,
                                 x2: iconRight + 0.5, y2: iconTop - 0.5,
                                 r: oR, g: oG, b: oB, a: 0.9)
            bridge.appendLineX1(iconLeft - 0.5, y1: iconBottom + 0.5,
                                 x2: iconRight + 0.5, y2: iconBottom + 0.5,
                                 r: oR, g: oG, b: oB, a: 0.9)
            bridge.appendLineX1(iconLeft - 0.5, y1: iconTop - 0.5,
                                 x2: iconLeft - 0.5, y2: iconBottom + 0.5,
                                 r: oR, g: oG, b: oB, a: 0.9)
            bridge.appendLineX1(iconRight + 0.5, y1: iconTop - 0.5,
                                 x2: iconRight + 0.5, y2: iconBottom + 0.5,
                                 r: oR, g: oG, b: oB, a: 0.9)
            bridge.flushLineBatch()
            if !e.name.isEmpty {
                let nameStartX = iconRight + 4
                let nameMaxX = e.x2 - 3
                let available = nameMaxX - nameStartX
                // B29: step font + fade instead of hard cutoff at 70 pt.
                // Desktop progressively shrinks the label; iPad mirrors
                // that so narrow effects still hint at their identity.
                // The chosen size is the largest that fits; opacity
                // dims at the smallest widths so the letters don't
                // compete with the icon for attention.
                if available > 8 {
                    let sizes: [CGFloat] = [nameFontSize, 9, 8, 7]
                    var chosenSize: CGFloat? = nil
                    var chosenWidth: CGFloat = 0
                    var chosenHeight: CGFloat = 0
                    for fs in sizes {
                        let sz = bridge.size(ofText: e.name, fontSize: fs)
                        if sz.width <= available {
                            chosenSize = fs
                            chosenWidth = sz.width
                            chosenHeight = sz.height
                            break
                        }
                    }
                    if let fs = chosenSize {
                        // Linear fade: full opacity at fontSize >= 9,
                        // fading to 0.55 at fontSize == 7.
                        let alpha: CGFloat = fs >= 9 ? 0.95
                                                      : 0.95 - (9 - fs) * 0.2
                        bridge.drawText(e.name,
                                         atX: nameStartX,
                                         y: e.top + (e.bottom - e.top - chosenHeight) / 2,
                                         fontSize: fs,
                                         r: e.stroke.0, g: e.stroke.1, b: e.stroke.2,
                                         a: max(0.5, alpha))
                        _ = chosenWidth   // reserved for future crop logic
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

        // B30 hover highlight — only painted when a
        // `UIHoverGestureRecognizer` is tracking a Magic Keyboard /
        // trackpad pointer. Touch-only users never see this. For
        // `leftEdge` / `rightEdge` the corresponding bracket edge
        // gets a brighter white overlay line; for fade zones the
        // top-strip of the effect rect lights up subtly.
        if c.drag == nil, let h = c.hover {
            drawHoverHighlight(bridge: bridge, viewport: viewport,
                                c: c, hover: h)
        }

        // Drag feedback pill.
        drawDragPill(bridge: bridge, viewport: viewport, c: c)

        // B1 marquee rectangle overlay. Semi-transparent blue fill
        // with a solid-blue stroke; keeps the viewport's local
        // coords since we translate world → view here.
        if let start = c.marqueeStartWorld,
           let end = c.marqueeCurrentWorld {
            let x1 = min(start.x, end.x) - c.scrollOffsetX
            let x2 = max(start.x, end.x) - c.scrollOffsetX
            let y1 = min(start.y, end.y) - c.scrollOffsetY
            let y2 = max(start.y, end.y) - c.scrollOffsetY
            let w = max(1, x2 - x1)
            let h = max(1, y2 - y1)
            bridge.beginFilledRectBatch()
            bridge.appendFilledRectX(x1, y: y1, w: w, h: h,
                                      r: 0.35, g: 0.60, b: 1.0, a: 0.20)
            bridge.flushFilledRectBatch()
            bridge.beginLineBatch()
            let lr: CGFloat = 0.45, lg: CGFloat = 0.70, lb: CGFloat = 1.0
            bridge.appendLineX1(x1, y1: y1, x2: x2, y2: y1,
                                 r: lr, g: lg, b: lb, a: 0.95)
            bridge.appendLineX1(x2, y1: y1, x2: x2, y2: y2,
                                 r: lr, g: lg, b: lb, a: 0.95)
            bridge.appendLineX1(x2, y1: y2, x2: x1, y2: y2,
                                 r: lr, g: lg, b: lb, a: 0.95)
            bridge.appendLineX1(x1, y1: y2, x2: x1, y2: y1,
                                 r: lr, g: lg, b: lb, a: 0.95)
            bridge.flushLineBatch()
        }
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

    /// B30 hover highlight. Locates the hovered effect's on-screen
    /// rect and draws a subtle brighter overlay on the relevant
    /// handle zone: a 3-pt white wedge on the hovered edge (left or
    /// right), or a white ring around the hovered fade diamond.
    /// Center-zone hover draws nothing — the whole effect is already
    /// styled, and a full-rect hover would fight with the selection
    /// stroke.
    private func drawHoverHighlight(bridge: XLGridMetalBridge,
                                     viewport: CGSize,
                                     c: EffectsMetalGridView.Coordinator,
                                     hover: EffectsMetalGridView.Coordinator.HoverHit) {
        // Resolve the on-screen rect of the hovered effect.
        guard let row = c.rows.first(where: { $0.id == hover.rowIndex }),
              hover.effectIndex < row.effects.count else { return }
        let effect = row.effects[hover.effectIndex]
        // Row y-top (replicating the layout walk used elsewhere).
        var y: CGFloat = -c.scrollOffsetY
        var rowTop: CGFloat = 0
        var rowH: CGFloat = c.metrics.rowHeight
        for r in c.rows {
            let h = (r.id == c.selection?.rowIndex)
                ? c.metrics.selectedRowHeight : c.metrics.rowHeight
            if r.id == hover.rowIndex { rowTop = y; rowH = h; break }
            y += h
        }
        let top = rowTop + 1
        let bottom = rowTop + rowH - 1
        let x1 = CGFloat(effect.startTimeMS) * c.pixelsPerMS - c.scrollOffsetX
        let x2 = CGFloat(effect.endTimeMS) * c.pixelsPerMS - c.scrollOffsetX

        switch hover.zone {
        case .leftEdge, .rightEdge:
            let edgeX = hover.zone == .leftEdge ? x1 + 0.5 : x2 - 0.5
            bridge.beginLineBatch()
            // Two adjacent lines for a 1-2 px brighter stroke.
            bridge.appendLineX1(edgeX, y1: top, x2: edgeX, y2: bottom,
                                 r: 1, g: 1, b: 1, a: 0.95)
            bridge.appendLineX1(edgeX + (hover.zone == .leftEdge ? 1 : -1),
                                 y1: top,
                                 x2: edgeX + (hover.zone == .leftEdge ? 1 : -1),
                                 y2: bottom,
                                 r: 1, g: 1, b: 1, a: 0.6)
            bridge.flushLineBatch()
        case .fadeIn, .fadeOut:
            // Small rect in the top fade strip of the hovered half.
            let midX = (x1 + x2) / 2
            let hx1 = hover.zone == .fadeIn ? x1 : midX
            let hx2 = hover.zone == .fadeIn ? midX : x2
            bridge.beginFilledRectBatch()
            bridge.appendFilledRectX(hx1, y: top,
                                      w: max(CGFloat(1), hx2 - hx1),
                                      h: CGFloat(7),
                                      r: 1, g: 1, b: 1, a: 0.18)
            bridge.flushFilledRectBatch()
        case .center:
            // No hover overlay — the whole effect rect is already
            // the primary affordance and a full overlay would fight
            // the selection stroke.
            break
        }
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
        // B18: double-tap an empty cell (palette armed) creates an
        // effect that fills the span between adjacent effects.
        // Chained via `require(toFail:)` so single-tap selection
        // still works immediately when double-tap isn't imminent.
        let doubleTap = UITapGestureRecognizer(target: self,
                                                 action: #selector(onDoubleTapCell(_:)))
        doubleTap.numberOfTapsRequired = 2
        doubleTap.delegate = del
        addGestureRecognizer(doubleTap)
        tap.require(toFail: doubleTap)
        // B30: pointer hover from Magic Keyboard / trackpad. Never
        // fires for finger touches, so this is a no-cost addition
        // for touch-only users.
        let hover = UIHoverGestureRecognizer(target: self,
                                               action: #selector(onHover(_:)))
        addGestureRecognizer(hover)
        let pan = UIPanGestureRecognizer(target: self, action: #selector(onPan(_:)))
        pan.delegate = del
        // Single-finger pan: scroll or effect-drag. Clamping prevents
        // it from also grabbing the two-finger marquee pan. `allowed-
        // ScrollTypesMask = .all` (B95) lets a trackpad two-finger
        // scroll or scroll-wheel fire this recognizer even though
        // Apple models those as discrete (non-continuous) pan events.
        pan.minimumNumberOfTouches = 1
        pan.maximumNumberOfTouches = 1
        pan.allowedScrollTypesMask = .all
        addGestureRecognizer(pan)
        ownPan = pan
        // B1: two-finger long-press + drag → marquee multi-select.
        // Earlier (pre-2026-05-15) this was a two-finger
        // UIPanGestureRecognizer, which raced with the one-finger
        // pan when the second finger landed an instant after the
        // first — observed as "sometimes scrolls, sometimes
        // marquees, sometimes both". Switching to a long-press
        // makes the marquee an explicit, distinct gesture: hold
        // two fingers still for ~0.4s, then drag to extend the
        // selection rect. Matches the layout-canvas pattern for
        // consistency.
        let marquee = UILongPressGestureRecognizer(
            target: self,
            action: #selector(onMarqueeLongPress(_:)))
        marquee.numberOfTouchesRequired = 2
        marquee.minimumPressDuration = 0.4
        marquee.allowableMovement = 16
        marquee.name = "marqueeLP"
        marquee.delegate = del
        addGestureRecognizer(marquee)
        let pinch = UIPinchGestureRecognizer(target: self, action: #selector(onPinch(_:)))
        pinch.delegate = del
        addGestureRecognizer(pinch)
        ownPinch = pinch
        let lp = UILongPressGestureRecognizer(target: self, action: #selector(onLongPress(_:)))
        lp.minimumPressDuration = 0.6
        lp.allowableMovement = 4
        lp.name = "contextMenuLP"
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
        // Hit slop stays at the finger-friendly 24 pt default for
        // both finger and Pencil. An earlier pass tried narrowing to
        // 12 pt for Pencil for "precision," but in practice that's
        // narrower than real Pencil jitter + reach-and-tap wobble,
        // so edge / fade zones became unreachable. The B30 hover
        // highlight already guides Pencil users toward the right
        // zone before they tap; keeping slop generous means the tap
        // actually lands where the hover promised.
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

    /// Long-press hit-test for the in/out transition diamonds drawn at
    /// the corners of the currently-selected effect (see
    /// `drawFadeDiamond` — diamonds only paint for the selection, so
    /// hit-testing follows suit). Uses generous finger-friendly slop
    /// around the diamond's actual drawn position rather than the
    /// 7-pt top strip the drag-time hit test uses, because long-press
    /// is an intentional gesture and the diamonds are only ~7 px tall.
    func hitTestFadeDiamond(at p: CGPoint) -> (rowIndex: Int, effectIndex: Int, isIn: Bool)? {
        guard let c = coordinator, c.pixelsPerMS > 0 else { return nil }
        guard let sel = c.selection else { return nil }
        // Locate the selected row's top y in viewport coords.
        var rowTop: CGFloat = -c.scrollOffsetY
        var found = false
        for row in c.rows {
            let h = (row.id == c.selection?.rowIndex)
                ? c.metrics.selectedRowHeight : c.metrics.rowHeight
            if row.id == sel.rowIndex {
                found = true
                break
            }
            rowTop += h
        }
        guard found else { return nil }
        guard let row = c.rows.first(where: { $0.id == sel.rowIndex }) else { return nil }
        guard sel.effectIndex >= 0, sel.effectIndex < row.effects.count else { return nil }
        let effect = row.effects[sel.effectIndex]
        let (fadeIn, fadeOut) = c.fadeProvider(sel.rowIndex, sel.effectIndex)
        // Diamond x positions mirror the drawing math at line ~801.
        let x1 = CGFloat(effect.startTimeMS) * c.pixelsPerMS - c.scrollOffsetX
        let x2 = CGFloat(effect.endTimeMS)   * c.pixelsPerMS - c.scrollOffsetX
        let width = x2 - x1
        if width < 8 { return nil }
        let fadeInPx = CGFloat(fadeIn) * 1000 * c.pixelsPerMS
        let fadeOutPx = CGFloat(fadeOut) * 1000 * c.pixelsPerMS
        let inX = x1 + min(fadeInPx, width)
        let outX = max(x1, x2 - fadeOutPx)
        // Vertical: top ~18 pt of the effect rect captures the diamond
        // (centred ~2.5 pt below row top) plus finger slop. Reject
        // anywhere below that so center-zone long-presses still reach
        // the standard context menu.
        let vSlop: CGFloat = 18
        guard p.y >= rowTop - 4 && p.y <= rowTop + vSlop else { return nil }
        // Horizontal: 18-pt half-width slop around each diamond centre.
        // Only match a side that actually has a fade — empty corners
        // of zero-fade effects fall through.
        let hSlop: CGFloat = 18
        if fadeIn > 0 && abs(p.x - inX) <= hSlop {
            return (sel.rowIndex, sel.effectIndex, true)
        }
        if fadeOut > 0 && abs(p.x - outX) <= hSlop {
            return (sel.rowIndex, sel.effectIndex, false)
        }
        return nil
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

    @objc func onDoubleTapCell(_ g: UITapGestureRecognizer) {
        guard let c = coordinator else { return }
        let p = g.location(in: self)
        // Skip if the tap landed on an existing effect — single-tap
        // selection already handles that and we don't want to mutate.
        if hitTestEffect(at: p) != nil { return }
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
        guard let rid = rowId, c.pixelsPerMS > 0 else { return }
        let ms = max(0, Int((p.x + c.scrollOffsetX) / c.pixelsPerMS))
        c.actions.onDoubleTapEmpty(rid, ms)
    }

    @objc func onHover(_ g: UIHoverGestureRecognizer) {
        guard let c = coordinator else { return }
        let newHover: EffectsMetalGridView.Coordinator.HoverHit?
        switch g.state {
        case .began, .changed:
            let p = g.location(in: self)
            if let hit = hitTestEffect(at: p) {
                newHover = .init(rowIndex: hit.rowIndex,
                                  effectIndex: hit.effectIndex,
                                  zone: hit.zone)
            } else {
                newHover = nil
            }
        default:
            newHover = nil
        }
        let changed = (newHover?.rowIndex != c.hover?.rowIndex)
            || (newHover?.effectIndex != c.hover?.effectIndex)
            || (newHover?.zone != c.hover?.zone)
        if changed {
            c.hover = newHover
            setNeedsDisplay()
        }
    }

    @objc func onLongPress(_ g: UILongPressGestureRecognizer) {
        guard g.state == .began, let c = coordinator else { return }
        let p = g.location(in: self)
        // Long-press on a visible transition diamond opens the
        // transition-type picker for that side. Checked before the
        // generic context-menu path so the gesture is always available
        // regardless of selection state.
        if let fade = hitTestFadeDiamond(at: p) {
            c.actions.onTapEffect(fade.rowIndex, fade.effectIndex)
            c.actions.onRequestTransitionMenu(fade.rowIndex, fade.effectIndex,
                                               fade.isIn, p)
            return
        }
        guard let hit = hitTestEffect(at: p) else { return }
        // When the long-press lands on a member of the current
        // multi-select, preserve the set and request a bulk context
        // menu. Otherwise fall through to the single-effect path
        // (collapsing any existing multi-select to this one effect).
        let isMemberOfMulti = c.selectedEffects.count > 1 && c.selectedEffects.contains(where: {
            $0.rowIndex == hit.rowIndex && $0.effectIndex == hit.effectIndex
        })
        if isMemberOfMulti {
            c.actions.onRequestContextMenu(hit.rowIndex, hit.effectIndex, p)
            return
        }
        c.actions.onTapEffect(hit.rowIndex, hit.effectIndex)
        c.actions.onRequestContextMenu(hit.rowIndex, hit.effectIndex, p)
    }

    @objc func onPinch(_ g: UIPinchGestureRecognizer) {
        guard let c = coordinator else { return }
        switch g.state {
        case .began:
            pinchAnchorX = g.location(in: self).x
            pinchLastScale = 1
            c.onUserInteraction?()
        case .changed:
            let delta = g.scale / pinchLastScale
            pinchLastScale = g.scale
            c.actions.onPinchZoom(delta, pinchAnchorX + c.scrollOffsetX)
            c.onUserInteraction?()
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
            // B96: any new pan cancels an in-flight momentum coast.
            stopMomentumScroll()
            let p = g.location(in: self)
            if let hit = hitTestEffect(at: p), canvasHasDragCandidate(at: p) {
                beginEffectDrag(hit: hit, c: c)
            } else {
                // Plain scroll pan — capture origin for delta math.
                c.drag = nil
                c.panStartScrollX = c.scrollOffsetX
                c.panStartScrollY = c.scrollOffsetY
            }
            c.onUserInteraction?()
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
            c.onUserInteraction?()
        case .ended, .cancelled, .failed:
            stopAutoScroll(c: c)
            if let _ = c.drag {
                endEffectDrag(c: c)
            } else if g.state == .ended {
                // B96: start momentum scroll from the pan's release
                // velocity. Touch pans carry real momentum; trackpad
                // two-finger scrolls report tiny velocities (the
                // gesture decomposition is done by the trackpad) so
                // we gate on a minimum magnitude to avoid over-
                // shooting on scroll-wheel-style inputs.
                let v = g.velocity(in: self)
                if abs(v.x) > 80 || abs(v.y) > 80 {
                    startMomentumScroll(velocity: v, c: c)
                }
            }
        default:
            break
        }
    }

    // MARK: - B96 momentum scroll

    private func startMomentumScroll(velocity: CGPoint,
                                      c: EffectsMetalGridView.Coordinator) {
        // Stop any prior coast first (user started a new pan).
        stopMomentumScroll()
        var vX = velocity.x
        var vY = velocity.y
        // Exponential decay @ ~60 Hz; factor picked so 300 px/s
        // velocity coasts visible for ~0.5 s.
        let decay: CGFloat = 0.93
        let stopThreshold: CGFloat = 20    // px/s
        let displayLink = CADisplayLink(target: MomentumTarget { [weak self] in
            guard let self = self, let c = self.coordinator else {
                self?.stopMomentumScroll()
                return
            }
            let dt: CGFloat = 1.0 / 60.0
            let newX = max(0, c.scrollOffsetX - vX * dt)
            let newY = max(0, c.scrollOffsetY - vY * dt)
            c.onUpdateScrollX(newX)
            c.onUpdateScrollY(newY)
            vX *= decay
            vY *= decay
            if abs(vX) < stopThreshold && abs(vY) < stopThreshold {
                self.stopMomentumScroll()
            }
        }, selector: #selector(MomentumTarget.tick(_:)))
        displayLink.add(to: .main, forMode: .common)
        coordinator?.momentumLink = displayLink
    }

    private func stopMomentumScroll() {
        coordinator?.momentumLink?.invalidate()
        coordinator?.momentumLink = nil
    }

    /// Shim object so we can pass a captured closure as a
    /// `CADisplayLink` target (which needs an `@objc` selector).
    private final class MomentumTarget: NSObject {
        let action: () -> Void
        init(_ action: @escaping () -> Void) { self.action = action }
        @objc func tick(_ link: CADisplayLink) { action() }
    }

    // MARK: - B1 marquee select (two-finger long-press + drag)

    /// Two-finger long-press → drag. On `.began` we cancel any
    /// in-flight one-finger pan (the user could have started a
    /// one-finger scroll, then dropped a second finger and held)
    /// and disable pinch so a spread mid-marquee doesn't fire a
    /// zoom. Both are restored on end / cancel.
    @objc func onMarqueeLongPress(_ g: UILongPressGestureRecognizer) {
        guard let c = coordinator else { return }
        switch g.state {
        case .began:
            // Cancel a concurrent one-finger pan if it had begun
            // (briefly drag-flag the recognizer off + on). The pan
            // handler's .cancelled branch tidies up its own scroll
            // state.
            if let pan = ownPan, pan.state == .began || pan.state == .changed {
                pan.isEnabled = false
                pan.isEnabled = true
            }
            // Quiet pinch so a finger-spread during marquee can't
            // also fire a zoom.
            ownPinch?.isEnabled = false

            let p = g.location(in: self)
            c.marqueeStartWorld = CGPoint(x: p.x + c.scrollOffsetX,
                                           y: p.y + c.scrollOffsetY)
            c.marqueeCurrentWorld = c.marqueeStartWorld
            setNeedsDisplay()
        case .changed:
            guard c.marqueeStartWorld != nil else { return }
            let p = g.location(in: self)
            c.marqueeCurrentWorld = CGPoint(x: p.x + c.scrollOffsetX,
                                             y: p.y + c.scrollOffsetY)
            setNeedsDisplay()
        case .ended:
            defer { ownPinch?.isEnabled = true }
            guard let start = c.marqueeStartWorld,
                  let end = c.marqueeCurrentWorld else {
                c.marqueeStartWorld = nil
                c.marqueeCurrentWorld = nil
                setNeedsDisplay()
                return
            }
            let rect = CGRect(x: min(start.x, end.x),
                              y: min(start.y, end.y),
                              width: abs(end.x - start.x),
                              height: abs(end.y - start.y))
            let hits = marqueeHits(worldRect: rect, c: c)
            c.marqueeStartWorld = nil
            c.marqueeCurrentWorld = nil
            setNeedsDisplay()
            if rect.width < 6 && rect.height < 6 { return }
            c.onMarqueeSelect?(hits)
        case .cancelled, .failed:
            ownPinch?.isEnabled = true
            c.marqueeStartWorld = nil
            c.marqueeCurrentWorld = nil
            setNeedsDisplay()
        default:
            break
        }
    }

    /// Compute which effects lie inside a world-space marquee rectangle.
    /// An effect qualifies when its row is vertically overlapped by the
    /// rectangle AND its `[startMS, endMS]` range overlaps the
    /// rectangle's x range. Pure overlap (not containment) — matches
    /// desktop's `EffectsGrid::SelectEffectsInRegion` behaviour.
    private func marqueeHits(worldRect: CGRect,
                              c: EffectsMetalGridView.Coordinator)
        -> Set<SequencerViewModel.EffectSelection> {
        guard c.pixelsPerMS > 0 else { return [] }
        var result: Set<SequencerViewModel.EffectSelection> = []
        var rowTop: CGFloat = 0
        for row in c.rows {
            let h = (row.id == c.selection?.rowIndex)
                ? c.metrics.selectedRowHeight : c.metrics.rowHeight
            let rowBot = rowTop + h
            // Vertical overlap check.
            if rowBot < worldRect.minY || rowTop > worldRect.maxY {
                rowTop += h
                continue
            }
            let leftMS = Int(worldRect.minX / c.pixelsPerMS)
            let rightMS = Int(worldRect.maxX / c.pixelsPerMS)
            for (eIdx, effect) in row.effects.enumerated() {
                if effect.endTimeMS < leftMS { continue }
                if effect.startTimeMS > rightMS { break }
                result.insert(.init(rowIndex: row.id,
                                     effectIndex: eIdx,
                                     name: effect.name,
                                     startTimeMS: effect.startTimeMS,
                                     endTimeMS: effect.endTimeMS))
            }
            rowTop += h
        }
        return result
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

        // Pencil squeeze + edge resize → shared-edge mode. The
        // paired neighbour must be butted against the dragged edge
        // (zero-gap); if there's a gap, shared-edge doesn't apply
        // and we fall through to normal resize.
        var pairedIdx: Int? = nil
        var pairedOrigStart = 0, pairedOrigEnd = 0
        if c.pencilSqueezeActive {
            if kind == .resizeLeft,
               hit.effectIndex > 0 {
                let prev = row.effects[hit.effectIndex - 1]
                if prev.endTimeMS == effect.startTimeMS {
                    pairedIdx = hit.effectIndex - 1
                    pairedOrigStart = prev.startTimeMS
                    pairedOrigEnd = prev.endTimeMS
                }
            } else if kind == .resizeRight,
                       hit.effectIndex + 1 < row.effects.count {
                let next = row.effects[hit.effectIndex + 1]
                if next.startTimeMS == effect.endTimeMS {
                    pairedIdx = hit.effectIndex + 1
                    pairedOrigStart = next.startTimeMS
                    pairedOrigEnd = next.endTimeMS
                }
            }
        }
        if pairedIdx != nil {
            c.pencilSqueezeConsumedByDrag = true
        }

        var drag = EffectsMetalGridView.Coordinator.DragLocal(
            rowIndex: hit.rowIndex,
            effectIndex: hit.effectIndex,
            origStartMS: effect.startTimeMS,
            origEndMS: effect.endTimeMS,
            origFadeInSec: curIn, origFadeOutSec: curOut,
            minStartMS: prevEnd, maxEndMS: nextStart,
            kind: kind)
        drag.pairedEffectIndex = pairedIdx
        drag.pairedOrigStartMS = pairedOrigStart
        drag.pairedOrigEndMS = pairedOrigEnd
        c.drag = drag
        c.liveStartMS = effect.startTimeMS
        c.liveEndMS = effect.endTimeMS
        c.liveFadeInSec = curIn
        c.liveFadeOutSec = curOut
        c.liveRowId = nil
        c.liveDropInvalid = false
        c.pairedLiveStartMS = pairedIdx != nil ? pairedOrigStart : nil
        c.pairedLiveEndMS = pairedIdx != nil ? pairedOrigEnd : nil
        publishDrag(c: c)
    }

    /// B5 snap-to-timing: return the active timing-mark time nearest
    /// to `ms` if it's within `snapPx` pixels; else nil. `timingMark-
    /// TimesMS` is already sorted and only contains active marks
    /// (filtering happens up in `collectActiveTimingMarkTimes`).
    private func snapMS(_ ms: Int, c: EffectsMetalGridView.Coordinator,
                         snapPx: CGFloat = 10) -> Int? {
        guard c.pixelsPerMS > 0, !c.timingMarkTimesMS.isEmpty else { return nil }
        let thresh = Int(snapPx / c.pixelsPerMS)
        var best: Int?
        var bestD = thresh + 1
        for m in c.timingMarkTimesMS {
            let d = abs(m - ms)
            if d < bestD { bestD = d; best = m }
        }
        return best
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
            var cs = invalid ? unclampedStart
                              : max(minStart, min(maxStart, unclampedStart))
            var ce = cs + duration
            // B5: snap whichever edge is nearer to a mark; preserves
            // duration so the effect slides, not stretches. Skip when
            // the drop is already invalid (snapping into an overlap
            // would be worse than leaving the user-held position).
            if !invalid,
               let snapStart = snapMS(cs, c: c),
               let snapEnd = snapMS(ce, c: c) {
                let dStart = abs(snapStart - cs)
                let dEnd = abs(snapEnd - ce)
                if dStart <= dEnd {
                    cs = max(minStart, min(maxStart, snapStart))
                    ce = cs + duration
                } else {
                    let shifted = snapEnd - duration
                    cs = max(minStart, min(maxStart, shifted))
                    ce = cs + duration
                }
            } else if !invalid, let snapStart = snapMS(cs, c: c) {
                cs = max(minStart, min(maxStart, snapStart))
                ce = cs + duration
            } else if !invalid, let snapEnd = snapMS(ce, c: c) {
                let shifted = snapEnd - duration
                cs = max(minStart, min(maxStart, shifted))
                ce = cs + duration
            }
            c.liveStartMS = cs
            c.liveEndMS = ce
        case .resizeLeft:
            // Shared-edge mode (Pencil squeeze + neighbour butted on
            // the left): the shared boundary is clamped against the
            // previous neighbour's *start* (so we can't shrink it to
            // zero) + the dragged effect's own end. The paired live
            // range mirrors: prev.end follows the boundary.
            let hardMin: Int
            let hardMax: Int
            if d.pairedEffectIndex != nil {
                hardMin = d.pairedOrigStartMS + 1
                hardMax = d.origEndMS - 1
            } else {
                hardMin = d.minStartMS
                hardMax = d.origEndMS - 1
            }
            let ns = max(hardMin, d.origStartMS + dMS)
            var liveStart = min(ns, hardMax)
            if let snap = snapMS(liveStart, c: c) {
                liveStart = max(hardMin, min(hardMax, snap))
            }
            c.liveStartMS = liveStart
            c.liveEndMS = d.origEndMS
            if d.pairedEffectIndex != nil {
                c.pairedLiveStartMS = d.pairedOrigStartMS
                c.pairedLiveEndMS = liveStart
            }
        case .resizeRight:
            // Symmetric shared-edge for the right neighbour: the
            // boundary can't swallow the neighbour, and the
            // neighbour's start follows the boundary.
            let hardMin: Int
            let hardMax: Int
            if d.pairedEffectIndex != nil {
                hardMin = d.origStartMS + 1
                hardMax = d.pairedOrigEndMS - 1
            } else {
                hardMin = d.origStartMS + 1
                hardMax = d.maxEndMS
            }
            let raw = d.origEndMS + dMS
            var liveEnd = max(hardMin, min(hardMax, raw))
            if let snap = snapMS(liveEnd, c: c) {
                liveEnd = min(hardMax, max(hardMin, snap))
            }
            c.liveEndMS = liveEnd
            c.liveStartMS = d.origStartMS
            if d.pairedEffectIndex != nil {
                c.pairedLiveStartMS = liveEnd
                c.pairedLiveEndMS = d.pairedOrigEndMS
            }
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
                if let pIdx = d.pairedEffectIndex,
                   let pe = c.pairedLiveEndMS {
                    // Shared-edge commit: previous effect's end +
                    // this effect's start both move to the boundary.
                    c.actions.onResizeSharedEdge?(
                        d.rowIndex,
                        pIdx, d.pairedOrigStartMS, pe,
                        d.effectIndex, ls, d.origEndMS)
                } else {
                    c.actions.onResizeEdge(d.rowIndex, d.effectIndex, 0, ls)
                }
            }
        case .resizeRight:
            if let le = c.liveEndMS {
                if let pIdx = d.pairedEffectIndex,
                   let ps = c.pairedLiveStartMS {
                    c.actions.onResizeSharedEdge?(
                        d.rowIndex,
                        d.effectIndex, d.origStartMS, le,
                        pIdx, ps, d.pairedOrigEndMS)
                } else {
                    c.actions.onResizeEdge(d.rowIndex, d.effectIndex, 1, le)
                }
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
        c.pairedLiveStartMS = nil
        c.pairedLiveEndMS = nil
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
            if let lp = g as? UILongPressGestureRecognizer {
                // Two long-presses live on this view: the marquee
                // (two-finger, "marqueeLP") and the context menu
                // (one-finger, "contextMenuLP"). The marquee never
                // needs a hit-test — it can start anywhere on the
                // grid (including over effects, since the user may
                // want a rectangle that includes them as the
                // anchor). The context-menu LP only begins when a
                // single finger lands on an effect, matching
                // desktop's right-click-on-effect behaviour.
                if lp.name == "marqueeLP" { return true }
                let p = g.location(in: v)
                return v.hitTestEffect(at: p) != nil
            }
            // Pan always begins — either a drag or a scroll.
            return true
        }
    }
}
