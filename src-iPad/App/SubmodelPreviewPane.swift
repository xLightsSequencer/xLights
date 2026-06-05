import SwiftUI
import MetalKit
import UIKit

/// Embedded model-preview pane for the SubModelDetailEditor.
/// Renders the parent model fit-to-window, with the active row's
/// nodes painted white and the rest painted dark grey (mirrors
/// desktop `SubModelsDialog::SelectRow`). Supports:
///
///   - Pinch to zoom.
///   - Single-finger drag to pan.
///   - Single-tap to toggle the tapped node into the active row.
///   - Two-finger long-press + drag → rectangle selection of
///     multiple nodes; on release every enclosed node is added
///     to the active row as a compressed range.
///
/// The pane owns a dedicated `XLMetalBridge` named uniquely per
/// editor instance so its channel-update suppression + node
/// colour overrides don't leak into the Layout Editor or House
/// Preview panes.
struct SubmodelPreviewPane: UIViewRepresentable {
    let document: XLSequenceDocument
    let parentModelName: String
    let highlightedNodes: [Int]
    let onToggleNode: (Int) -> Void
    let onAddNodes: ([Int]) -> Void

    func makeCoordinator() -> Coordinator {
        Coordinator(parent: parentModelName)
    }

    func makeUIView(context: Context) -> MTKView {
        let view = MTKView()
        view.device = MTLCreateSystemDefaultDevice()
        view.colorPixelFormat = .bgra8Unorm
        view.clearColor = MTLClearColorMake(0.05, 0.05, 0.07, 1)
        view.isPaused = true
        view.enableSetNeedsDisplay = true
        view.delegate = context.coordinator
        view.isMultipleTouchEnabled = true

        let bridgeName = "SubmodelEditor-\(parentModelName)-\(ObjectIdentifier(context.coordinator).hashValue)"
        let bridge = XLMetalBridge(name: bridgeName)
        if let layer = view.layer as? CAMetalLayer {
            bridge.attach(layer)
        }
        bridge.setPreviewModel(parentModelName)
        bridge.setSingleModelMode(true)
        bridge.setSuppressChannelUpdate(true)
        bridge.setIs3D(false)
        context.coordinator.bridge = bridge
        context.coordinator.mtkView = view
        context.coordinator.document = document
        context.coordinator.onToggleNode = onToggleNode
        context.coordinator.onAddNodes = onAddNodes

        let pinch = UIPinchGestureRecognizer(
            target: context.coordinator,
            action: #selector(Coordinator.handlePinch(_:)))
        pinch.delegate = context.coordinator
        view.addGestureRecognizer(pinch)

        let pan = UIPanGestureRecognizer(
            target: context.coordinator,
            action: #selector(Coordinator.handlePan(_:)))
        pan.minimumNumberOfTouches = 1
        pan.maximumNumberOfTouches = 1
        pan.delegate = context.coordinator
        view.addGestureRecognizer(pan)

        let tap = UITapGestureRecognizer(
            target: context.coordinator,
            action: #selector(Coordinator.handleTap(_:)))
        tap.numberOfTapsRequired = 1
        view.addGestureRecognizer(tap)

        let dblTap = UITapGestureRecognizer(
            target: context.coordinator,
            action: #selector(Coordinator.handleDoubleTap(_:)))
        dblTap.numberOfTapsRequired = 2
        view.addGestureRecognizer(dblTap)
        tap.require(toFail: dblTap)

        let marquee = UILongPressGestureRecognizer(
            target: context.coordinator,
            action: #selector(Coordinator.handleMarquee(_:)))
        marquee.numberOfTouchesRequired = 2
        marquee.minimumPressDuration = 0.3
        marquee.allowableMovement = 20
        marquee.delegate = context.coordinator
        view.addGestureRecognizer(marquee)

        context.coordinator.applyHighlights(highlightedNodes)
        // MTKView is paused (`isPaused = true`) — it only draws when
        // we ask. The initial drawableSizeWillChange callback may
        // fire before the bridge has a layer attached, leaving the
        // view stuck on a black clear with nothing drawn. Schedule
        // a redraw on the next runloop so layout has finished and
        // the bridge can render the first frame. Without this, the
        // pane is black until the user does something that bumps
        // highlightedNodes (selects a line, hits play, etc.).
        DispatchQueue.main.async { [weak view] in
            view?.setNeedsDisplay()
        }
        return view
    }

    func updateUIView(_ uiView: MTKView, context: Context) {
        context.coordinator.document = document
        context.coordinator.onToggleNode = onToggleNode
        context.coordinator.onAddNodes = onAddNodes
        if context.coordinator.lastHighlight != highlightedNodes {
            context.coordinator.applyHighlights(highlightedNodes)
        }
        // Always request a redraw on update. SwiftUI re-evaluates
        // bodies often, so this stays cheap (MTKView coalesces
        // pending setNeedsDisplay into a single frame), and it
        // covers the first-appearance case where neither the
        // highlight nor the drawable size has changed but the
        // view hasn't yet drawn anything visible.
        uiView.setNeedsDisplay()
    }

    static func dismantleUIView(_ uiView: MTKView, coordinator: Coordinator) {
        coordinator.teardown()
    }

    final class Coordinator: NSObject, MTKViewDelegate, UIGestureRecognizerDelegate {
        var bridge: XLMetalBridge?
        weak var mtkView: MTKView?
        var document: XLSequenceDocument?
        var onToggleNode: ((Int) -> Void)?
        var onAddNodes: (([Int]) -> Void)?
        var lastHighlight: [Int] = []
        private let parent: String
        private var pinchStartZoom: Float = 1.0
        private var panStartX: Float = 0
        private var panStartY: Float = 0
        private var marqueeLayer: CAShapeLayer?
        private var marqueeStart: CGPoint = .zero

        init(parent: String) {
            self.parent = parent
            super.init()
        }

        func teardown() {
            if let bridge, let document {
                bridge.clearSubmodelHighlights(onModel: parent, for: document)
            }
            bridge = nil
            mtkView = nil
            document = nil
        }

        func applyHighlights(_ nodes: [Int]) {
            lastHighlight = nodes
            guard let bridge, let document else { return }
            bridge.setSubmodelHighlightedNodes(nodes.map { NSNumber(value: $0) },
                                                onModel: parent,
                                                for: document)
        }

        // MARK: - MTKView delegate

        func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {
            bridge?.setDrawableSize(size, scale: view.contentScaleFactor)
            view.setNeedsDisplay()
        }

        func draw(in view: MTKView) {
            guard let bridge, let document else { return }
            bridge.setDrawableSize(view.drawableSize, scale: view.contentScaleFactor)
            bridge.drawModels(for: document, atMS: 0, pointSize: 4.0)
        }

        // MARK: - Gestures

        @objc func handlePinch(_ g: UIPinchGestureRecognizer) {
            guard let bridge else { return }
            switch g.state {
            case .began:
                pinchStartZoom = bridge.cameraZoom()
            case .changed:
                let raw = max(0.05, min(50.0, pinchStartZoom * Float(g.scale)))
                bridge.setCameraZoom(raw)
                mtkView?.setNeedsDisplay()
            default:
                break
            }
        }

        @objc func handlePan(_ g: UIPanGestureRecognizer) {
            guard let bridge, let view = g.view else { return }
            switch g.state {
            case .began:
                panStartX = bridge.cameraPanX()
                panStartY = bridge.cameraPanY()
            case .changed:
                let t = g.translation(in: view)
                let scale: CGFloat = 1.0 / CGFloat(max(bridge.cameraZoom(), 0.01))
                bridge.setCameraPanX(panStartX + Float(t.x * scale),
                                     panY: panStartY - Float(t.y * scale))
                mtkView?.setNeedsDisplay()
            default:
                break
            }
        }

        @objc func handleTap(_ g: UITapGestureRecognizer) {
            guard let bridge, let view = g.view, let document else { return }
            let pt = g.location(in: view)
            let n = bridge.nodeNearPoint(pt,
                                          onModel: parent,
                                          viewSize: view.bounds.size,
                                          for: document)
            if n > 0 {
                onToggleNode?(Int(n))
            }
        }

        @objc func handleDoubleTap(_ g: UITapGestureRecognizer) {
            bridge?.resetCamera()
            mtkView?.setNeedsDisplay()
        }

        @objc func handleMarquee(_ g: UILongPressGestureRecognizer) {
            guard let view = g.view else { return }
            switch g.state {
            case .began:
                marqueeStart = g.location(in: view)
                let layer = CAShapeLayer()
                layer.strokeColor = UIColor.systemBlue.cgColor
                layer.fillColor = UIColor.systemBlue.withAlphaComponent(0.15).cgColor
                layer.lineWidth = 1.5
                layer.lineDashPattern = [4, 3]
                view.layer.addSublayer(layer)
                marqueeLayer = layer
            case .changed:
                let cur = g.location(in: view)
                let rect = CGRect(x: min(marqueeStart.x, cur.x),
                                  y: min(marqueeStart.y, cur.y),
                                  width: abs(cur.x - marqueeStart.x),
                                  height: abs(cur.y - marqueeStart.y))
                marqueeLayer?.path = UIBezierPath(rect: rect).cgPath
            case .ended:
                let cur = g.location(in: view)
                let rect = CGRect(x: min(marqueeStart.x, cur.x),
                                  y: min(marqueeStart.y, cur.y),
                                  width: abs(cur.x - marqueeStart.x),
                                  height: abs(cur.y - marqueeStart.y))
                marqueeLayer?.removeFromSuperlayer()
                marqueeLayer = nil
                guard let bridge, let document,
                      rect.width > 4, rect.height > 4 else { return }
                let nodes = bridge.nodes(in: rect,
                                          onModel: parent,
                                          viewSize: view.bounds.size,
                                          for: document)
                let ints = nodes.compactMap { $0.intValue > 0 ? $0.intValue : nil }
                if !ints.isEmpty { onAddNodes?(ints) }
            case .cancelled, .failed:
                marqueeLayer?.removeFromSuperlayer()
                marqueeLayer = nil
            default:
                break
            }
        }

        // Pinch + pan coexist; long-press marquee is exclusive.
        func gestureRecognizer(_ g: UIGestureRecognizer,
                                shouldRecognizeSimultaneouslyWith other: UIGestureRecognizer) -> Bool {
            if g is UILongPressGestureRecognizer || other is UILongPressGestureRecognizer {
                return false
            }
            return true
        }
    }
}
