import SwiftUI
import MetalKit
import Observation

/// Diagnostic state surfaced by the preview pane to its host view.
/// `XLMetalBridge` reports each silent-fail reason via `errorReason`;
/// the coordinator copies it here on every draw attempt. The host
/// reads `bannerMessage` — non-nil iff the failure has persisted past
/// a small grace period, so transient init failures (drawable not yet
/// sized, document not loaded yet) don't flicker the banner.
@MainActor
@Observable
final class PreviewStatus {
    /// Latest reason from the bridge, or `nil` when the most recent
    /// frame drew cleanly. Bare reason text is set by the bridge —
    /// e.g. "No Metal layer attached", "Drawable size is 0x0
    /// (waiting for layout)", "No model selected".
    fileprivate(set) var errorReason: String? = nil
    /// True after the first frame that completed `EndDrawing`.
    fileprivate(set) var hasRenderedSuccessfully: Bool = false
    /// Wall-clock deadline; while now < deadline, suppress the banner.
    /// Set to `now + initialGrace` whenever a fresh error appears.
    private var bannerDeadline: Date = .distantFuture
    private static let initialGrace: TimeInterval = 1.0

    func update(reason: String?, rendered: Bool) {
        let cleaned = (reason?.isEmpty == false) ? reason : nil
        if cleaned != errorReason {
            errorReason = cleaned
            // New error → start a fresh grace period; cleared error
            // → no deadline at all so banner clears immediately.
            bannerDeadline = (cleaned != nil)
                ? Date().addingTimeInterval(Self.initialGrace)
                : .distantFuture
        }
        hasRenderedSuccessfully = rendered
    }

    /// What the host view should render. nil = no banner.
    var bannerMessage: String? {
        guard let reason = errorReason else { return nil }
        if Date() < bannerDeadline { return nil }
        return reason
    }
}

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
    let settings: PreviewSettings
    /// Optional diagnostic surface. When non-nil, the coordinator
    /// posts the bridge's error state into it on every draw so the
    /// host view (HousePreviewView et al.) can paint a banner over
    /// the pane when the preview can't render.
    let status: PreviewStatus?

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
        context.coordinator.previewNameForNotifications = previewName
        context.coordinator.registerNotifications(previewName: previewName)
        // Phase J-2 (touch UX) — register the bridge so the
        // floating inline action bar (which lives outside this
        // UIViewRepresentable's view tree) can reach it without
        // an explicit injection. The registry holds a weak ref,
        // so no explicit teardown is needed when the bridge dies.
        if let bridge = context.coordinator.bridge {
            XLightsBridgeBox.register(bridge, forPreviewName: previewName)
        }

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

        // Phase J-2 — single-tap to select a model. Only consumes the
        // tap on the LayoutEditor pane (the handler bails on others).
        // Set to require dblTap to fail so a real double-tap doesn't
        // also fire two single-taps.
        let singleTap = UITapGestureRecognizer(
            target: context.coordinator,
            action: #selector(Coordinator.handleSingleTap(_:)))
        singleTap.numberOfTapsRequired = 1
        singleTap.require(toFail: dblTap)
        view.addGestureRecognizer(singleTap)

        // Phase J-2 (touch UX) — Pencil hover + trackpad pointer
        // hover. Cursor-equivalent for handle highlight; reads the
        // hover location and asks the bridge to update
        // `highlighted_handle` so DrawAxisTool tints the matched
        // axis. LayoutEditor pane only — the handler bails on
        // others. Available on iPadOS 13.4+ (works for trackpad)
        // and gets full Pencil hover on M2+ iPads.
        let hover = UIHoverGestureRecognizer(
            target: context.coordinator,
            action: #selector(Coordinator.handleHover(_:)))
        view.addGestureRecognizer(hover)

        // Phase J-2 (touch UX) — long-press → contextual menu.
        // LayoutEditor pane only. Hit-tests for vertex / segment /
        // curve-control handles and posts a notification with the
        // hit info so the SwiftUI editor view can present the
        // matching menu (Delete Point / Add Point / Define Curve /
        // Remove Curve).
        let longPress = UILongPressGestureRecognizer(
            target: context.coordinator,
            action: #selector(Coordinator.handleLongPress(_:)))
        longPress.minimumPressDuration = 0.45
        view.addGestureRecognizer(longPress)

        // Phase J-3 (touch UX) — Pencil 2 / Pencil Pro hardware
        // gestures. Double-tap cycles the axis tool on the
        // selected model (Move → Scale → Rotate); Pro-only squeeze
        // triggers layout-undo. Both LayoutEditor-only — the
        // coordinator gates by `previewNameForNotifications`
        // before acting. Pencil 1 has neither — nothing breaks,
        // delegate just never fires.
        let pencil = UIPencilInteraction()
        pencil.delegate = context.coordinator
        view.addInteraction(pencil)

        return view
    }

    func updateUIView(_ uiView: MTKView, context: Context) {
        context.coordinator.viewModel = viewModel
        context.coordinator.settings = settings
        context.coordinator.status = status
        context.coordinator.bridge?.setPreviewModel(previewModelName ?? "")

        // Push appearance toggles that the bridge needs to know about. Mode
        // and view-object visibility both affect draw output, so flip
        // setNeedsDisplay when they change even outside of playback.
        if let bridge = context.coordinator.bridge {
            if bridge.is3D() != settings.is3D {
                bridge.setIs3D(settings.is3D)
                uiView.setNeedsDisplay()
            }
            if bridge.showViewObjects() != settings.showViewObjects {
                bridge.setShowViewObjects(settings.showViewObjects)
                uiView.setNeedsDisplay()
            }
            // J-2 — push the LayoutEditor selection through to the
            // bridge whenever the view-model state changes (sidebar
            // pick, canvas tap, group switch deselect). Repaint after
            // the push so the ring appears immediately.
            if previewName == "LayoutEditor" {
                let newSel = viewModel.layoutEditorSelectedModel ?? ""
                // J-4 (multi-select) — the extras set is the secondary
                // selection: every model in the selection except the
                // primary. Tracked separately so a sidebar pick / tap
                // doesn't redraw N rings on each update unless the set
                // changed. Encoded as a stable string for cheap diff.
                let primary = viewModel.layoutEditorSelectedModel
                let extras = viewModel.layoutEditorSelection
                    .filter { $0 != primary }
                let extrasKey = extras.sorted().joined(separator: "\n")
                if context.coordinator.lastPushedSelection != newSel {
                    // Clear any lingering hover highlight on the
                    // previously-selected model — without this, the
                    // old model's last-hovered handle stays yellow
                    // until the user re-hovers there. `clearHovered`
                    // reads `_selectedModelName` which still holds
                    // the old value at this point.
                    _ = bridge.clearHoveredHandle(for: viewModel.document)
                    bridge.setSelectedModel(viewModel.layoutEditorSelectedModel)
                    context.coordinator.lastPushedSelection = newSel
                    uiView.setNeedsDisplay()
                }
                if context.coordinator.lastPushedExtras != extrasKey {
                    bridge.setExtraSelectedModels(Array(extras))
                    context.coordinator.lastPushedExtras = extrasKey
                    uiView.setNeedsDisplay()
                }
                // Grid + canvas-bbox overlays. Bridge holds the last-
                // pushed value and repaints on change.
                if bridge.showLayoutGrid() != settings.showLayoutGrid {
                    bridge.setShowLayoutGrid(settings.showLayoutGrid)
                    uiView.setNeedsDisplay()
                }
                if bridge.showLayoutBoundingBox() != settings.showLayoutBoundingBox {
                    bridge.setShowLayoutBoundingBox(settings.showLayoutBoundingBox)
                    uiView.setNeedsDisplay()
                }
                if bridge.snapToGrid() != settings.snapToGrid {
                    bridge.setSnapToGrid(settings.snapToGrid)
                    // No repaint needed — snap only affects future
                    // moves, not current rendered state.
                }
                if bridge.uniformModifier() != settings.uniformModifier {
                    bridge.setUniformModifier(settings.uniformModifier)
                }
                if bridge.lockAxis() != settings.lockAxis {
                    bridge.setLockAxis(settings.lockAxis)
                }
                if bridge.showFirstPixel() != settings.showFirstPixel {
                    bridge.setShowFirstPixel(settings.showFirstPixel)
                    uiView.setNeedsDisplay()
                }
            }
        }

        // The display link drives per-frame redraws. Keep it running not
        // just during full playback but also while the selection-scoped
        // scrub loop is advancing `playPositionMS` — otherwise the
        // preview freezes on the effect start frame.
        let needsAnimation = viewModel.isPlaying || viewModel.isScrubbing
        if needsAnimation {
            if context.coordinator.displayLink == nil {
                context.coordinator.startDisplayLink(frameIntervalMS: viewModel.frameIntervalMS)
            }
        } else {
            context.coordinator.stopDisplayLink()
            uiView.setNeedsDisplay()
        }
    }

    class Coordinator: NSObject, MTKViewDelegate, UIGestureRecognizerDelegate, UIPencilInteractionDelegate {
        var bridge: XLMetalBridge?
        var viewModel: SequencerViewModel?
        var settings: PreviewSettings?
        var status: PreviewStatus?
        weak var mtkView: MTKView?
        /// J-2 — most recent selection name pushed into the bridge,
        /// so updateUIView can detect Δ vs. the live view-model
        /// state and repaint exactly when the selection changes.
        var lastPushedSelection: String = ""
        // J-4 (multi-select) — sorted-newline-joined extras keys.
        // Empty when the secondary selection is empty.
        var lastPushedExtras: String = ""
        /// J-2 — true while a one-finger pan started on the
        /// LayoutEditor's selected model and is dragging the model
        /// rather than the camera. Set in .began, cleared in .ended.
        var draggingLayoutModel: Bool = false
        var layoutDragModelName: String? = nil
        /// J-2 — index of the resize handle being dragged
        /// (0..3 corners), -1 when not dragging a handle.
        /// Mutually exclusive with `draggingLayoutModel`.
        var draggingLayoutHandle: Int = -1
        /// J-2 UX — pinch-on-model = uniform scale of the selected
        /// model. Mutually exclusive with camera-zoom pinch.
        var pinchingLayoutModel: Bool = false
        var pinchScaleModelName: String? = nil
        /// J-2 UX — two-finger twist on model = rotate Z of the
        /// selected model. Mutually exclusive with camera-rotate
        /// twist.
        var twistingLayoutModel: Bool = false
        var twistRotateModelName: String? = nil
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

        /// Apply a per-bridge mutation if `objectName` (extracted from
        /// the notification's `object`) matches this pane's preview
        /// name. Refreshes the MTKView afterwards. `@MainActor` because
        /// every observer pins itself to `queue: .main` and the body
        /// touches main-actor state (`bridge`, `mtkView`).
        @MainActor
        private func applyToBridge(objectName: String?,
                                    previewName: String,
                                    block: (XLMetalBridge) -> Void) {
            guard let bridge, objectName == previewName else { return }
            block(bridge)
            mtkView?.setNeedsDisplay()
        }

        /// Register for zoom/reset notifications scoped to this preview's
        /// name. The overlay buttons in HousePreviewView post with the
        /// preview name as `object` so each pane picks up only its own.
        ///
        /// `Notification` isn't Sendable, so each observer extracts the
        /// Sendable bits (`object` as String, `userInfo` strings)
        /// before crossing into `MainActor.assumeIsolated` — Swift 6
        /// strict concurrency flags moving the whole notification
        /// across the boundary even though the assume-isolated body
        /// runs synchronously on the same thread.
        func registerNotifications(previewName: String) {
            let center = NotificationCenter.default
            let step: Float = 1.5
            notificationObservers.append(center.addObserver(
                forName: .previewZoomIn, object: nil, queue: .main
            ) { [weak self] note in
                let objectName = note.object as? String
                MainActor.assumeIsolated {
                    self?.applyToBridge(objectName: objectName,
                                         previewName: previewName) { b in
                        let z = min(max(b.cameraZoom() * step, 0.1), 50.0)
                        b.setCameraZoom(z)
                    }
                }
            })
            notificationObservers.append(center.addObserver(
                forName: .previewZoomOut, object: nil, queue: .main
            ) { [weak self] note in
                let objectName = note.object as? String
                MainActor.assumeIsolated {
                    self?.applyToBridge(objectName: objectName,
                                         previewName: previewName) { b in
                        let z = min(max(b.cameraZoom() / step, 0.1), 50.0)
                        b.setCameraZoom(z)
                    }
                }
            })
            notificationObservers.append(center.addObserver(
                forName: .previewZoomReset, object: nil, queue: .main
            ) { [weak self] note in
                let objectName = note.object as? String
                MainActor.assumeIsolated {
                    self?.applyToBridge(objectName: objectName,
                                         previewName: previewName) { b in
                        b.setCameraZoom(1.0)
                    }
                }
            })
            notificationObservers.append(center.addObserver(
                forName: .previewResetCamera, object: nil, queue: .main
            ) { [weak self] note in
                let objectName = note.object as? String
                MainActor.assumeIsolated {
                    self?.applyToBridge(objectName: objectName,
                                         previewName: previewName) { b in
                        b.resetCamera()
                    }
                }
            })

            // Fit-all / fit-model — both resolve model bounds through
            // the document's render context, so we have to pull the
            // doc off the view model each time (it can change across
            // sequence loads).
            notificationObservers.append(center.addObserver(
                forName: .previewFitAll, object: nil, queue: .main
            ) { [weak self] note in
                let objectName = note.object as? String
                MainActor.assumeIsolated {
                    guard let self,
                          let bridge = self.bridge,
                          let viewModel = self.viewModel,
                          objectName == previewName else { return }
                    _ = bridge.fitAllModels(for: viewModel.document)
                    self.mtkView?.setNeedsDisplay()
                }
            })
            notificationObservers.append(center.addObserver(
                forName: .previewFitModel, object: nil, queue: .main
            ) { [weak self] note in
                let objectName = note.object as? String
                let modelName = note.userInfo?["name"] as? String ?? ""
                MainActor.assumeIsolated {
                    guard let self,
                          let bridge = self.bridge,
                          let viewModel = self.viewModel,
                          objectName == previewName else { return }
                    _ = bridge.fitModelNamed(modelName, for: viewModel.document)
                    self.mtkView?.setNeedsDisplay()
                }
            })

            // Image export — capture the current MTKView contents and push
            // the resulting UIImage through a share sheet. Scoped by preview
            // name like the zoom / reset observers above.
            notificationObservers.append(center.addObserver(
                forName: .previewSaveImage, object: nil, queue: .main
            ) { [weak self] note in
                let objectName = note.object as? String
                MainActor.assumeIsolated {
                    guard let self, objectName == previewName else { return }
                    self.captureAndShare()
                }
            })

            // Layout-group change — every pane drops its cached
            // background texture so the next draw reloads the correct
            // file for the new group. Not scoped by preview name: the
            // render context is shared across panes, so the change
            // applies globally.
            notificationObservers.append(center.addObserver(
                forName: Notification.Name("XLLayoutGroupChanged"),
                object: nil, queue: .main
            ) { [weak self] _ in
                MainActor.assumeIsolated {
                    guard let self else { return }
                    self.bridge?.invalidateBackgroundCache()
                    self.mtkView?.setNeedsDisplay()
                }
            })

            // J-2 — repaint the canvas after a Layout Editor model
            // edit that didn't go through the gesture path (keyboard
            // nudge, undo, sidebar property commit). Drag's
            // per-frame setNeedsDisplay handles the gesture path on
            // its own.
            notificationObservers.append(center.addObserver(
                forName: .layoutEditorModelMoved, object: nil, queue: .main
            ) { [weak self] note in
                let objectName = note.object as? String
                MainActor.assumeIsolated {
                    guard let self, objectName == previewName else { return }
                    self.mtkView?.setNeedsDisplay()
                }
            })

            // Viewpoint commands — scoped to this pane by previewName
            // (each pane has its own camera, so apply/save target the
            // correct one). The reply broadcasts via
            // `previewViewpointListChanged` so the overlay's menu can
            // refresh after a save/delete.
            notificationObservers.append(center.addObserver(
                forName: .previewViewpointCommand, object: nil, queue: .main
            ) { [weak self] note in
                let objectName = note.object as? String
                let action = (note.userInfo?["action"] as? String) ?? ""
                let name = note.userInfo?["name"] as? String
                MainActor.assumeIsolated {
                    guard let self, objectName == previewName else { return }
                    self.handleViewpointCommand(action: action, name: name)
                }
            })
        }

        private func handleViewpointCommand(action: String, name: String?) {
            guard let bridge, let viewModel else { return }
            let doc = viewModel.document
            switch action {
            case "refresh":
                break
            case "apply":
                guard let n = name else { return }
                _ = bridge.applyViewpointNamed(n, for: doc)
                mtkView?.setNeedsDisplay()
            case "save":
                guard let n = name, !n.isEmpty else { return }
                _ = bridge.saveCurrentView(as: n, for: doc)
            case "delete":
                guard let n = name else { return }
                _ = bridge.deleteViewpointNamed(n, for: doc)
            case "restore":
                bridge.restoreDefaultViewpoint(for: doc)
                mtkView?.setNeedsDisplay()
            default:
                return
            }
            // Broadcast the current (possibly updated) list back to the
            // overlay for this pane so its menu reflects the latest.
            let names = bridge.viewpointNames(for: doc)
            NotificationCenter.default.post(name: .previewViewpointListChanged,
                                            object: self.previewNameForNotifications,
                                            userInfo: ["names": names])
        }

        /// Captured at registration time so the command handler can
        /// echo back on the same channel the overlay listens on.
        fileprivate var previewNameForNotifications: String = ""

        /// Capture the current drawable into a UIImage and present a share
        /// sheet. `drawHierarchy(in:afterScreenUpdates:)` handles CAMetalLayer
        /// — `afterScreenUpdates: true` forces a redraw before the capture
        /// so the snapshot never misses the latest frame when playback is
        /// paused (the MTKView defaults to isPaused+enableSetNeedsDisplay).
        private func captureAndShare() {
            guard let mtkView else { return }
            // Ensure the layer is up-to-date for panes that aren't actively
            // animating. setNeedsDisplay alone isn't enough because the
            // snapshot happens synchronously.
            mtkView.setNeedsDisplay()
            let renderer = UIGraphicsImageRenderer(bounds: mtkView.bounds)
            let image = renderer.image { _ in
                mtkView.drawHierarchy(in: mtkView.bounds, afterScreenUpdates: true)
            }
            let activityVC = UIActivityViewController(activityItems: [image],
                                                     applicationActivities: nil)
            // iPad requires a source for the popover — anchor at the
            // preview's centre. On iPhone it's a sheet and the source is
            // ignored.
            if let popover = activityVC.popoverPresentationController {
                popover.sourceView = mtkView
                popover.sourceRect = CGRect(x: mtkView.bounds.midX,
                                            y: mtkView.bounds.midY,
                                            width: 0, height: 0)
                popover.permittedArrowDirections = []
            }
            Self.presentTopmost(activityVC)
        }

        /// Walk the foreground scene's root view controller chain and
        /// present from whatever is topmost — keeps the share sheet working
        /// whether or not another sheet/popover is already up.
        private static func presentTopmost(_ vc: UIViewController) {
            guard let scene = UIApplication.shared.connectedScenes
                .compactMap({ $0 as? UIWindowScene })
                .first(where: { $0.activationState == .foregroundActive })
                ?? UIApplication.shared.connectedScenes
                    .compactMap({ $0 as? UIWindowScene }).first,
                  let window = scene.keyWindow ?? scene.windows.first,
                  var presenter = window.rootViewController else { return }
            while let next = presenter.presentedViewController {
                presenter = next
            }
            presenter.present(vc, animated: true)
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
                // J-2 UX — pinch on the LayoutEditor's selected model body
                // = uniform scale. Pinch elsewhere stays camera zoom.
                pinchingLayoutModel = false
                if previewNameForNotifications == "LayoutEditor",
                   let viewModel,
                   let view = mtkView,
                   let sel = viewModel.layoutEditorSelectedModel,
                   !sel.isEmpty {
                    let centroid = recognizer.location(in: view)
                    let hit = bridge.pickModel(atScreenPoint: centroid,
                                                viewSize: view.bounds.size,
                                                for: viewModel.document)
                    if hit == sel,
                       bridge.beginPinchScale(forModel: sel,
                                              for: viewModel.document) {
                        pinchingLayoutModel = true
                        pinchScaleModelName = sel
                        viewModel.document.pushLayoutUndoSnapshot(forModel: sel)
                    }
                }
                if !pinchingLayoutModel {
                    pinchStartZoom = bridge.cameraZoom()
                    panStartX = bridge.cameraPanX()
                    panStartY = bridge.cameraPanY()
                    pinchStartCentroid = recognizer.location(in: mtkView)
                }
            case .changed:
                if pinchingLayoutModel, let viewModel {
                    _ = bridge.applyPinchScaleFactor(recognizer.scale,
                                                     for: viewModel.document)
                    mtkView?.setNeedsDisplay()
                    return
                }
                let z = pinchStartZoom * Float(recognizer.scale)
                bridge.setCameraZoom(min(max(z, 0.1), 50.0))

                let loc = recognizer.location(in: mtkView)
                let dx = Float(loc.x - pinchStartCentroid.x)
                let dy = Float(loc.y - pinchStartCentroid.y)
                bridge.setCameraPanX(panStartX + dx, panY: panStartY - dy)

                mtkView?.setNeedsDisplay()
            case .ended, .cancelled, .failed:
                if pinchingLayoutModel {
                    bridge.endPinchScale()
                    if let name = pinchScaleModelName {
                        NotificationCenter.default.post(
                            name: .layoutEditorModelMoved,
                            object: previewNameForNotifications,
                            userInfo: ["model": name])
                    }
                    pinchingLayoutModel = false
                    pinchScaleModelName = nil
                }
            default:
                break
            }
        }

        @objc func handleOneFingerPan(_ recognizer: UIPanGestureRecognizer) {
            guard let bridge else { return }
            switch recognizer.state {
            case .began:
                // J-3 (touch UX) — Add Model drag-to-size. If the
                // user picked a type from the Add menu, the next
                // drag creates the model at touch-down and sizes
                // it via the descriptor `BeginCreate` session as
                // the finger moves. Mirrors desktop's click-drag-
                // release flow.
                if previewNameForNotifications == "LayoutEditor",
                   let viewModel,
                   let view = mtkView,
                   let pendingType = viewModel.layoutPendingNewModelType {
                    let touch = recognizer.location(in: view)
                    if let newName = bridge.createModel(
                        ofType: pendingType,
                        atScreenPoint: touch,
                        viewSize: view.bounds.size,
                        for: viewModel.document) {
                        viewModel.layoutPendingNewModelType = nil
                        viewModel.layoutSelectSingle(newName)
                        // Polyline-style models enter mid-create
                        // mode after this drag; the .ended branch
                        // keeps the in-progress flag set so the
                        // next tap appends a vertex rather than
                        // creating a fresh model.
                        if bridge.modelUsesPolyPointLocation(newName,
                                                              for: viewModel.document) {
                            viewModel.layoutPolylineInProgress = newName
                        }
                        layoutDragModelName = newName
                        // Sentinel: any non-negative value routes
                        // .changed through `dragHandle`, which
                        // forwards to whichever `_dragSession`
                        // the bridge has open — including the
                        // creation session.
                        draggingLayoutHandle = 0
                        draggingLayoutModel = false
                        return
                    }
                }
                // J-3 (touch UX) — polyline mid-creation drag.
                // Each follow-on touch appends a vertex; pan
                // continues to size that new segment. Same
                // sentinel scheme as the fresh-create path so
                // .changed routes through `dragHandle`.
                if previewNameForNotifications == "LayoutEditor",
                   let viewModel,
                   let view = mtkView,
                   let polyName = viewModel.layoutPolylineInProgress {
                    let touch = recognizer.location(in: view)
                    if bridge.appendVertex(toPolyline: polyName,
                                            atScreenPoint: touch,
                                            viewSize: view.bounds.size,
                                            for: viewModel.document) {
                        layoutDragModelName = polyName
                        draggingLayoutHandle = 0
                        draggingLayoutModel = false
                        return
                    }
                }
                // J-2 — on the LayoutEditor pane, a one-finger drag
                // whose origin lands on a resize handle resizes the
                // selected model. If it lands on the model's body
                // (not a handle) it moves the model. Other panes
                // (and drags on empty space / unselected models /
                // 3D mode / locked models) keep the existing
                // camera-pan behaviour.
                draggingLayoutModel = false
                draggingLayoutHandle = -1
                if previewNameForNotifications == "LayoutEditor",
                   let viewModel,
                   let view = mtkView,
                   let sel = viewModel.layoutEditorSelectedModel,
                   !sel.isEmpty {
                    let touch = recognizer.location(in: view)
                    let viewSize = view.bounds.size
                    // Handle hit-test runs first in both 2D and 3D —
                    // `pickHandle` returns a legacy handle id (corner,
                    // rotate, vertex, segment-CP, etc.) and the bridge
                    // opens a descriptor drag session for it. If no
                    // handle is hit, fall back to the 2D body-drag
                    // path. 3D body-drag requires camera-aware delta
                    // math we haven't shipped yet, so it gates on
                    // `!is3D()`.
                    let handle = bridge.pickHandle(atScreenPoint: touch,
                                                   viewSize: viewSize,
                                                   for: viewModel.document)
                    if handle >= 0 {
                        draggingLayoutHandle = handle
                        layoutDragModelName = sel
                        viewModel.document.pushLayoutUndoSnapshot(forModel: sel)
                        recognizer.setTranslation(.zero, in: view)
                    } else {
                        let hit = bridge.pickModel(atScreenPoint: touch,
                                                   viewSize: viewSize,
                                                   for: viewModel.document)
                        if hit == sel {
                            if bridge.is3D() {
                                // 3D body-drag: latch a plane-anchor
                                // through the model's centre Z so
                                // subsequent .changed updates can
                                // compute an absolute-touch-to-world
                                // delta. Falls through to orbit if
                                // the camera is parallel to the plane.
                                if bridge.beginBodyDrag3D(forModel: sel,
                                                          atScreenPoint: touch,
                                                          viewSize: viewSize,
                                                          for: viewModel.document) {
                                    draggingLayoutModel = true
                                    layoutDragModelName = sel
                                    viewModel.document.pushLayoutUndoSnapshot(forModel: sel)
                                }
                            } else {
                                draggingLayoutModel = true
                                layoutDragModelName = sel
                                viewModel.document.pushLayoutUndoSnapshot(forModel: sel)
                                recognizer.setTranslation(.zero, in: view)
                            }
                        }
                    }
                }
                if !draggingLayoutModel && draggingLayoutHandle < 0 {
                    if bridge.is3D() {
                        orbitStartAngleX = bridge.cameraAngleX()
                        orbitStartAngleY = bridge.cameraAngleY()
                    } else {
                        panStartX = bridge.cameraPanX()
                        panStartY = bridge.cameraPanY()
                    }
                }
            case .changed:
                if draggingLayoutHandle >= 0,
                   let view = mtkView,
                   let viewModel {
                    // Resize uses the absolute touch location (not a
                    // delta) so the dragged corner stays under the
                    // finger. The bridge anchors the opposite corner
                    // and rebuilds width/height/centre.
                    let touch = recognizer.location(in: view)
                    bridge.dragHandle(draggingLayoutHandle,
                                      toScreenPoint: touch,
                                      viewSize: view.bounds.size,
                                      for: viewModel.document)
                    mtkView?.setNeedsDisplay()
                    return
                }
                if draggingLayoutModel,
                   let view = mtkView,
                   let viewModel,
                   let name = layoutDragModelName {
                    if bridge.is3D() {
                        // 3D path uses the absolute touch point — the
                        // bridge tracks the latched plane anchor.
                        let touch = recognizer.location(in: view)
                        bridge.dragBody3D(toScreenPoint: touch,
                                          viewSize: view.bounds.size,
                                          for: viewModel.document)
                    } else {
                        let t = recognizer.translation(in: view)
                        bridge.moveModel(name,
                                         byDeltaDX: t.x,
                                         dY: t.y,
                                         viewSize: view.bounds.size,
                                         for: viewModel.document)
                        // Reset so the next .changed reports an
                        // incremental delta rather than a cumulative one.
                        recognizer.setTranslation(.zero, in: view)
                    }
                    mtkView?.setNeedsDisplay()
                    return
                }
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
            case .ended, .cancelled, .failed:
                if (draggingLayoutModel || draggingLayoutHandle >= 0),
                   let name = layoutDragModelName {
                    // Final summary refresh so the side panel sees
                    // the new centre + dimension values. Convention:
                    // object = previewName so the bridge coordinator
                    // can listen for repaint, model name in userInfo.
                    NotificationCenter.default.post(
                        name: .layoutEditorModelMoved,
                        object: previewNameForNotifications,
                        userInfo: ["model": name])
                }
                if draggingLayoutHandle >= 0,
                   let viewModel {
                    // Clear active_axis on the model so the next pick
                    // starts clean (without this, MoveHandle3D's
                    // switch hits stale axis state on the next drag).
                    bridge.endHandleDrag(for: viewModel.document)
                }
                if draggingLayoutModel && bridge.is3D() {
                    bridge.endBodyDrag3D()
                }
                draggingLayoutModel = false
                draggingLayoutHandle = -1
                layoutDragModelName = nil
            default:
                break
            }
        }

        @objc func handleRotate(_ recognizer: UIRotationGestureRecognizer) {
            guard let bridge else { return }
            // UIRotationGestureRecognizer's sign is opposite of what feels right
            // for a "grab the house and turn it" gesture — rotating fingers
            // clockwise returns a positive rotation, but the camera yaw needs
            // to move counter-clockwise to make the scene follow the fingers.
            switch recognizer.state {
            case .began:
                // J-2 UX — twist on the LayoutEditor's selected model body
                // = rotate Z. Twist elsewhere stays camera-yaw.
                twistingLayoutModel = false
                if previewNameForNotifications == "LayoutEditor",
                   let viewModel,
                   let view = mtkView,
                   let sel = viewModel.layoutEditorSelectedModel,
                   !sel.isEmpty {
                    let centroid = recognizer.location(in: view)
                    let hit = bridge.pickModel(atScreenPoint: centroid,
                                                viewSize: view.bounds.size,
                                                for: viewModel.document)
                    if hit == sel,
                       bridge.beginTwistRotate(forModel: sel,
                                                for: viewModel.document) {
                        twistingLayoutModel = true
                        twistRotateModelName = sel
                        viewModel.document.pushLayoutUndoSnapshot(forModel: sel)
                    }
                }
                if !twistingLayoutModel {
                    orbitStartAngleX = bridge.cameraAngleY()
                }
            case .changed:
                if twistingLayoutModel, let viewModel {
                    _ = bridge.applyTwistRotationRadians(recognizer.rotation,
                                                          for: viewModel.document)
                    mtkView?.setNeedsDisplay()
                    return
                }
                let delta = -Float(recognizer.rotation) * 180.0 / .pi
                bridge.setCameraAngleX(bridge.cameraAngleX(),
                                       angleY: orbitStartAngleX + delta)
                mtkView?.setNeedsDisplay()
            case .ended, .cancelled, .failed:
                if twistingLayoutModel {
                    bridge.endTwistRotate()
                    if let name = twistRotateModelName {
                        NotificationCenter.default.post(
                            name: .layoutEditorModelMoved,
                            object: previewNameForNotifications,
                            userInfo: ["model": name])
                    }
                    twistingLayoutModel = false
                    twistRotateModelName = nil
                }
            default:
                break
            }
        }

        @objc func handleDoubleTap(_ recognizer: UITapGestureRecognizer) {
            bridge?.resetCamera()
            mtkView?.setNeedsDisplay()
        }

        /// Phase J-2 (touch UX) — long-press on a handle posts a
        /// `.layoutEditorContextMenu` notification with the hit
        /// info. The LayoutEditorView observes this and presents a
        /// `.confirmationDialog` whose items match the hit type.
        @objc func handleLongPress(_ recognizer: UILongPressGestureRecognizer) {
            guard recognizer.state == .began,
                  previewNameForNotifications == "LayoutEditor",
                  let viewModel,
                  let bridge,
                  let view = mtkView else { return }
            let point = recognizer.location(in: view)
            guard let info = bridge.inspectHandle(
                atScreenPoint: point,
                viewSize: view.bounds.size,
                for: viewModel.document
            ) else { return }
            NotificationCenter.default.post(
                name: .layoutEditorContextMenu,
                object: previewNameForNotifications,
                userInfo: info)
        }

        // MARK: - Pencil interactions (J-3)

        /// Phase J-3 (touch UX) — Pencil 2 / Pencil Pro double-tap
        /// on the barrel. Cycles the axis tool on the selected
        /// model so the user can swap between Move / Scale /
        /// Rotate without going to the toolbar. Fires once on
        /// `.ended` so a rapid double-tap doesn't double-advance.
        /// Only acts when the editor pane has focus + a model is
        /// selected. iOS hands us the user's preferred action via
        /// `tap.action` (Switch Eraser / Show Palette / etc.); we
        /// override entirely since we don't have eraser semantics.
        func pencilInteraction(_ interaction: UIPencilInteraction,
                                didReceiveTap tap: UIPencilInteraction.Tap) {
            // Tap events are atomic (no phase) — fire once per
            // double-tap. Just gate by pane + selection.
            guard previewNameForNotifications == "LayoutEditor",
                  let viewModel,
                  let bridge,
                  let view = mtkView else { return }
            if bridge.cycleAxisToolForSelectedModel(for: viewModel.document),
               let sel = viewModel.layoutEditorSelectedModel {
                // Sync the SwiftUI toolbar's highlighted tool with
                // the new bridge-side axis_tool. The toolbar reads
                // from `settings.axisTool` (not the bridge), so
                // without this push the gizmo cycles but the
                // toolbar pill stays stale.
                let next = viewModel.document.axisTool(forModel: sel)
                if next != "none" {
                    settings?.axisTool = next
                }
                // Repaint + property-panel refresh via the standard
                // mutation notification.
                NotificationCenter.default.post(
                    name: .layoutEditorModelMoved,
                    object: previewNameForNotifications,
                    userInfo: ["model": sel])
                view.setNeedsDisplay()
            }
        }

        /// Phase J-3 (touch UX) — Pencil Pro squeeze on the barrel.
        /// Maps to layout-undo; LayoutEditorView listens for the
        /// notification and routes through its `performUndo()`.
        /// Fires once on `.ended` so a quick squeeze undoes one
        /// step, not the whole stack.
        func pencilInteraction(_ interaction: UIPencilInteraction,
                                didReceiveSqueeze squeeze: UIPencilInteraction.Squeeze) {
            guard squeeze.phase == .ended else { return }
            guard previewNameForNotifications == "LayoutEditor" else { return }
            NotificationCenter.default.post(
                name: .layoutEditorPencilUndo,
                object: previewNameForNotifications)
        }

        /// Phase J-2 (touch UX) — Pencil / trackpad hover.
        /// `UIHoverGestureRecognizer` fires for both Pencil hover
        /// (M2+ iPads, Pencil 2 / Pencil Pro) and trackpad pointer
        /// movement on any iPad. The bridge does a quick hit-test
        /// and updates `highlighted_handle` on the selected model
        /// so `DrawAxisTool` tints the matched axis yellow — the
        /// touch equivalent of desktop's mouse-hover affordance.
        @objc func handleHover(_ recognizer: UIHoverGestureRecognizer) {
            guard previewNameForNotifications == "LayoutEditor",
                  let viewModel,
                  let bridge,
                  let view = mtkView else { return }
            switch recognizer.state {
            case .began, .changed:
                let point = recognizer.location(in: view)
                let size = view.bounds.size
                let changed = bridge.setHoveredHandleAtScreenPoint(
                    point, viewSize: size, for: viewModel.document)
                if changed { view.setNeedsDisplay() }
            case .ended, .cancelled, .failed:
                if bridge.clearHoveredHandle(for: viewModel.document) {
                    view.setNeedsDisplay()
                }
            default:
                break
            }
        }

        /// Phase J-2 — single-tap selects the topmost model under the
        /// touch on the LayoutEditor pane only. Other panes ignore the
        /// tap (the recognizer is installed for everyone, but the
        /// handler is preview-name gated). 3D mode currently bails
        /// (full ray-cast hit testing lands with the gizmo work).
        @objc func handleSingleTap(_ recognizer: UITapGestureRecognizer) {
            guard previewNameForNotifications == "LayoutEditor",
                  let viewModel,
                  let bridge,
                  let view = mtkView else { return }
            let point = recognizer.location(in: view)
            let size = view.bounds.size
            // J-3 (touch UX) — polyline mid-creation. Each follow-
            // on tap appends a vertex to the in-progress polyline.
            // Checked before the fresh-model branch so the second-
            // through-Nth tap routes here instead of starting a
            // new model.
            if let polyName = viewModel.layoutPolylineInProgress {
                if bridge.appendVertex(toPolyline: polyName,
                                        atScreenPoint: point,
                                        viewSize: size,
                                        for: viewModel.document) {
                    // Tap-only: commit the BeginExtend session so
                    // the new vertex stays put. The next tap opens
                    // a fresh AddHandle + BeginExtend cycle.
                    bridge.endHandleDrag(for: viewModel.document)
                    NotificationCenter.default.post(
                        name: .layoutEditorModelMoved,
                        object: previewNameForNotifications,
                        userInfo: ["model": polyName])
                    view.setNeedsDisplay()
                }
                return
            }
            // J-4 (import) — pending .xmodel import. The user
            // already picked a file via the importer sheet; this
            // tap drops it at the touch point. Run before the
            // creation-mode branch so a stray creation flag
            // doesn't intercept.
            if let importPath = viewModel.layoutPendingImportPath {
                if let newName = bridge.importXmodel(fromPath: importPath,
                                                      atScreenPoint: point,
                                                      viewSize: size,
                                                      for: viewModel.document) {
                    viewModel.layoutPendingImportPath = nil
                    viewModel.layoutSelectSingle(newName)
                    NotificationCenter.default.post(
                        name: .layoutEditorModelMoved,
                        object: previewNameForNotifications,
                        userInfo: ["model": newName])
                    view.setNeedsDisplay()
                }
                return
            }
            // J-3 (touch UX) — Add Model. Tap-without-drag: the
            // user picked a type but didn't drag, so commit the
            // model at its `CreateDefaultModel` geometry. The
            // pan handler's drag-to-size path takes over when
            // there IS movement; this branch only fires when the
            // gesture never qualified as a pan.
            if let type = viewModel.layoutPendingNewModelType {
                if let newName = bridge.createModel(ofType: type,
                                                     atScreenPoint: point,
                                                     viewSize: size,
                                                     for: viewModel.document) {
                    viewModel.layoutPendingNewModelType = nil
                    viewModel.layoutSelectSingle(newName)
                    // Polyline-style models stay in "append vertex"
                    // mode until the user taps Done. Single-vertex
                    // models commit the BeginCreate session here.
                    if bridge.modelUsesPolyPointLocation(newName,
                                                         for: viewModel.document) {
                        viewModel.layoutPolylineInProgress = newName
                        bridge.endHandleDrag(for: viewModel.document)
                    } else {
                        bridge.endHandleDrag(for: viewModel.document)
                    }
                    NotificationCenter.default.post(
                        name: .layoutEditorModelMoved,
                        object: previewNameForNotifications,
                        userInfo: ["model": newName])
                    view.setNeedsDisplay()
                }
                return
            }
            // 3D-only: tapping the active centre handle cycles
            // translate / scale / rotate (mirrors desktop
            // LayoutPanel.cpp:3725). Try this first so the tap is
            // consumed before model-selection runs.
            if bridge.handleCenterHandleTap(atScreenPoint: point,
                                             viewSize: size,
                                             for: viewModel.document) {
                // Sync the SwiftUI toolbar's highlighted tool with
                // the bridge-side axis_tool we just advanced.
                if let sel = viewModel.layoutEditorSelectedModel {
                    let next = viewModel.document.axisTool(forModel: sel)
                    if next != "none" {
                        settings?.axisTool = next
                    }
                }
                view.setNeedsDisplay()
                return
            }
            // Bridge hit-test returns the model name (or nil for empty
            // space). Tap on empty space deselects so the side panel
            // collapses its property form back to the model list.
            let hit = bridge.pickModel(atScreenPoint: point,
                                       viewSize: size,
                                       for: viewModel.document)
            if viewModel.layoutEditMode {
                // J-4 (multi-select) — toggle membership. Tap on
                // empty space is treated as a no-op so the user
                // doesn't accidentally wipe a selection by missing
                // a model with a finger. They use the toolbar's
                // Clear / Done to leave edit mode entirely.
                if let name = hit, !name.isEmpty {
                    if viewModel.layoutEditorSelection.contains(name) {
                        viewModel.layoutEditorSelection.remove(name)
                        // If the primary just got dropped, promote
                        // whatever's still in the set to the
                        // primary slot so the gizmo / action bar
                        // re-anchor cleanly.
                        if viewModel.layoutEditorSelectedModel == name {
                            viewModel.layoutEditorSelectedModel =
                                viewModel.layoutEditorSelection.first
                        }
                    } else {
                        viewModel.layoutEditorSelection.insert(name)
                        // Most-recently-tapped becomes the primary
                        // — matches Adobe / Figma "last-clicked is
                        // the key model" convention. Align/match
                        // ops use this as the leader.
                        viewModel.layoutEditorSelectedModel = name
                    }
                }
            } else {
                // Single-select: replace selection wholesale.
                viewModel.layoutEditorSelectedModel = hit
                if let name = hit, !name.isEmpty {
                    viewModel.layoutEditorSelection = [name]
                } else {
                    viewModel.layoutEditorSelection.removeAll()
                }
            }
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
            // Post the bridge's diagnostic state to the host so the
            // overlay banner can decide whether to surface a message.
            // Cheap (two property reads) and the @Observable update
            // collapses to a no-op when the values haven't changed.
            if let status {
                status.update(reason: bridge.errorReason(),
                              rendered: bridge.hasRenderedSuccessfully())
            }
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
