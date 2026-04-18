import SwiftUI
import MetalKit

/// Shared preview pane — wraps an MTKView backed by an `XLMetalBridge` and
/// routes gestures (pinch to zoom, drag to pan/orbit, double-tap to reset)
/// through the bridge's PreviewCamera API. Both `HousePreviewView` and
/// `ModelPreviewView` construct this with different bridge names and
/// `previewModelName` bindings.
///
/// When `previewModelName` is a non-empty string, the bridge restricts drawing
/// to that model and skips view objects — that's Model Preview mode.
struct PreviewPaneView: UIViewRepresentable {
    @Environment(SequencerViewModel.self) var viewModel

    let previewName: String
    let previewModelName: String?
    @Binding var controlsVisible: Bool

    func makeCoordinator() -> Coordinator {
        Coordinator()
    }

    func makeUIView(context: Context) -> MTKView {
        let view = MTKView()
        view.device = MTLCreateSystemDefaultDevice()
        view.colorPixelFormat = .bgra8Unorm
        view.clearColor = MTLClearColorMake(0, 0, 0, 1)
        view.isPaused = true
        view.enableSetNeedsDisplay = true
        view.delegate = context.coordinator
        // UIView defaults isMultipleTouchEnabled to false, which silently drops
        // the second touch — pinch, two-finger pan, and rotation all never fire
        // without this flag.
        view.isMultipleTouchEnabled = true

        context.coordinator.bridge = XLMetalBridge(name: previewName)
        if let layer = view.layer as? CAMetalLayer {
            context.coordinator.bridge?.attach(layer)
        }
        context.coordinator.bridge?.setPreviewModel(previewModelName ?? "")
        context.coordinator.mtkView = view
        context.coordinator.registerNotifications(previewName: previewName)

        // Gestures — pinch to zoom, single-finger drag to orbit (3D) or pan
        // (2D), two-finger drag to pan, two-finger rotate to roll (3D), and
        // double-tap to reset.
        let pinch = UIPinchGestureRecognizer(
            target: context.coordinator,
            action: #selector(Coordinator.handlePinch(_:)))
        view.addGestureRecognizer(pinch)

        pinch.delegate = context.coordinator

        // One-finger drag only — orbit in 3D, pan in 2D. Capped at 1 touch so
        // adding a second finger cancels this recognizer and pinch takes over
        // (which also handles two-finger pan via its centroid).
        let oneFingerPan = UIPanGestureRecognizer(
            target: context.coordinator,
            action: #selector(Coordinator.handleOneFingerPan(_:)))
        oneFingerPan.minimumNumberOfTouches = 1
        oneFingerPan.maximumNumberOfTouches = 1
        oneFingerPan.delegate = context.coordinator
        view.addGestureRecognizer(oneFingerPan)

        // Two-finger pan is handled inside the pinch recognizer via its
        // centroid (see handlePinch). A dedicated UIPanGestureRecognizer
        // with min=2/max=2 refused to fire alongside pinch+rotate even with
        // simultaneous-recognition enabled — UIKit seems to starve the
        // translation component when pinch/rotate have already begun. Pinch
        // fires the instant two fingers touch down, so hanging pan off it
        // guarantees the pan updates while pinch is active.

        let rotate = UIRotationGestureRecognizer(
            target: context.coordinator,
            action: #selector(Coordinator.handleRotate(_:)))
        rotate.delegate = context.coordinator
        view.addGestureRecognizer(rotate)

        let dblTap = UITapGestureRecognizer(
            target: context.coordinator,
            action: #selector(Coordinator.handleDoubleTap(_:)))
        dblTap.numberOfTapsRequired = 2
        view.addGestureRecognizer(dblTap)

        return view
    }

    func updateUIView(_ uiView: MTKView, context: Context) {
        context.coordinator.viewModel = viewModel
        context.coordinator.bridge?.setPreviewModel(previewModelName ?? "")

        if viewModel.isPlaying {
            if context.coordinator.displayLink == nil {
                context.coordinator.startDisplayLink(frameIntervalMS: viewModel.frameIntervalMS)
            }
        } else {
            context.coordinator.stopDisplayLink()
            uiView.setNeedsDisplay()
        }
    }

    class Coordinator: NSObject, MTKViewDelegate, UIGestureRecognizerDelegate {
        var bridge: XLMetalBridge?
        var viewModel: SequencerViewModel?
        weak var mtkView: MTKView?
        // nonisolated(unsafe): touched from the nonisolated deinit. Real use is
        // main-thread-only (set/invalidated from display-link lifecycle calls).
        nonisolated(unsafe) var displayLink: CADisplayLink?
        private var lastDrawnMS: Int = -1

        private var pinchStartZoom: Float = 1.0
        private var panStartX: Float = 0
        private var panStartY: Float = 0
        private var pinchStartCentroid: CGPoint = .zero
        private var orbitStartAngleX: Float = 0
        private var orbitStartAngleY: Float = 0
        // nonisolated(unsafe): NSObjectProtocol isn't Sendable, but observers are
        // appended on the main thread during registerNotifications and only read
        // again in deinit.
        nonisolated(unsafe) private var notificationObservers: [NSObjectProtocol] = []

        // Allow pinch + rotate to fire together (plus the one-finger pan),
        // so rolling while pinching composes naturally.
        func gestureRecognizer(_ gestureRecognizer: UIGestureRecognizer,
                               shouldRecognizeSimultaneouslyWith other: UIGestureRecognizer) -> Bool {
            return true
        }

        // MARK: - Overlay button notifications

        /// Register for zoom/reset notifications scoped to this preview's
        /// name. The overlay buttons in HousePreviewView post with the
        /// preview name as `object` so each pane picks up only its own.
        func registerNotifications(previewName: String) {
            let center = NotificationCenter.default
            let step: Float = 1.5
            let apply: (Notification, (XLMetalBridge) -> Void) -> Void = { [weak self] note, block in
                guard let self,
                      let bridge = self.bridge,
                      (note.object as? String) == previewName else { return }
                block(bridge)
                self.mtkView?.setNeedsDisplay()
            }
            notificationObservers.append(center.addObserver(
                forName: .previewZoomIn, object: nil, queue: .main
            ) { note in apply(note) { b in
                let z = min(max(b.cameraZoom() * step, 0.1), 50.0)
                b.setCameraZoom(z)
            }})
            notificationObservers.append(center.addObserver(
                forName: .previewZoomOut, object: nil, queue: .main
            ) { note in apply(note) { b in
                let z = min(max(b.cameraZoom() / step, 0.1), 50.0)
                b.setCameraZoom(z)
            }})
            notificationObservers.append(center.addObserver(
                forName: .previewZoomReset, object: nil, queue: .main
            ) { note in apply(note) { b in b.setCameraZoom(1.0) }})
            notificationObservers.append(center.addObserver(
                forName: .previewResetCamera, object: nil, queue: .main
            ) { note in apply(note) { b in b.resetCamera() }})
        }

        // MARK: - Gestures

        // Pinch handles both zoom (scale) and two-finger pan (centroid
        // translation). Carrying pan here side-steps a UIKit quirk where a
        // dedicated UIPanGestureRecognizer with min=2/max=2 refuses to begin
        // while pinch/rotate are active, even with simultaneous-recognition
        // enabled. Pinch fires the moment two fingers touch, so the centroid
        // delta gives us reliable two-finger pan for free.
        @objc func handlePinch(_ recognizer: UIPinchGestureRecognizer) {
            guard let bridge else { return }
            switch recognizer.state {
            case .began:
                pinchStartZoom = bridge.cameraZoom()
                panStartX = bridge.cameraPanX()
                panStartY = bridge.cameraPanY()
                pinchStartCentroid = recognizer.location(in: mtkView)
            case .changed:
                let z = pinchStartZoom * Float(recognizer.scale)
                bridge.setCameraZoom(min(max(z, 0.1), 50.0))

                let loc = recognizer.location(in: mtkView)
                let dx = Float(loc.x - pinchStartCentroid.x)
                let dy = Float(loc.y - pinchStartCentroid.y)
                bridge.setCameraPanX(panStartX + dx, panY: panStartY - dy)

                mtkView?.setNeedsDisplay()
            default:
                break
            }
        }

        @objc func handleOneFingerPan(_ recognizer: UIPanGestureRecognizer) {
            guard let bridge else { return }
            switch recognizer.state {
            case .began:
                if bridge.is3D() {
                    orbitStartAngleX = bridge.cameraAngleX()
                    orbitStartAngleY = bridge.cameraAngleY()
                } else {
                    panStartX = bridge.cameraPanX()
                    panStartY = bridge.cameraPanY()
                }
            case .changed:
                let t = recognizer.translation(in: mtkView)
                if bridge.is3D() {
                    // 3D orbit: drag right -> rotate Y; drag down -> rotate X
                    let ay = orbitStartAngleY + Float(t.x) * 0.4
                    let ax = orbitStartAngleX + Float(t.y) * 0.4
                    bridge.setCameraAngleX(ax, angleY: ay)
                } else {
                    let px = panStartX + Float(t.x)
                    let py = panStartY - Float(t.y)
                    bridge.setCameraPanX(px, panY: py)
                }
                mtkView?.setNeedsDisplay()
            default:
                break
            }
        }

        @objc func handleRotate(_ recognizer: UIRotationGestureRecognizer) {
            guard let bridge else { return }
            // Only meaningful in 3D (angleZ has no visible effect in ortho 2D).
            // UIRotationGestureRecognizer's sign is opposite of what feels right
            // for a "grab the house and turn it" gesture — rotating fingers
            // clockwise returns a positive rotation, but the camera yaw needs
            // to move counter-clockwise to make the scene follow the fingers.
            switch recognizer.state {
            case .began:
                orbitStartAngleX = bridge.cameraAngleY()
            case .changed:
                let delta = -Float(recognizer.rotation) * 180.0 / .pi
                bridge.setCameraAngleX(bridge.cameraAngleX(),
                                       angleY: orbitStartAngleX + delta)
                mtkView?.setNeedsDisplay()
            default:
                break
            }
        }

        @objc func handleDoubleTap(_ recognizer: UITapGestureRecognizer) {
            bridge?.resetCamera()
            mtkView?.setNeedsDisplay()
        }

        // MARK: - Display link

        func startDisplayLink(frameIntervalMS: Int) {
            stopDisplayLink()
            let link = CADisplayLink(target: self, selector: #selector(displayLinkFired))
            let fps = max(1, 1000 / max(frameIntervalMS, 1))
            link.preferredFramesPerSecond = fps
            link.add(to: .main, forMode: .common)
            displayLink = link
        }

        func stopDisplayLink() {
            displayLink?.invalidate()
            displayLink = nil
        }

        @objc func displayLinkFired() {
            guard let viewModel else { return }
            let currentMS = viewModel.playPositionMS
            if currentMS != lastDrawnMS {
                lastDrawnMS = currentMS
                mtkView?.setNeedsDisplay()
            }
        }

        // MARK: - MTKViewDelegate

        func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {
            let scale = view.contentScaleFactor
            bridge?.setDrawableSize(size, scale: scale)
        }

        func draw(in view: MTKView) {
            guard let viewModel, let bridge else { return }
            bridge.drawModels(for: viewModel.document, atMS: Int32(viewModel.playPositionMS), pointSize: 2.0)
        }

        deinit {
            // Inline cleanup — deinit is nonisolated, so we can't call the
            // @MainActor stopDisplayLink() method. Invalidate directly.
            displayLink?.invalidate()
            displayLink = nil
            for obs in notificationObservers {
                NotificationCenter.default.removeObserver(obs)
            }
        }
    }
}
