import SwiftUI
import MetalKit

struct HousePreviewView: View {
    @State private var zoom: CGFloat = 1.0

    private static let minZoom: CGFloat = 0.5
    private static let maxZoom: CGFloat = 20.0
    private static let step: CGFloat = 1.5

    var body: some View {
        ZStack(alignment: .topTrailing) {
            HousePreviewMetalView(zoom: zoom)

            HStack(spacing: 4) {
                Button {
                    zoom = max(zoom / Self.step, Self.minZoom)
                } label: {
                    Image(systemName: "minus.magnifyingglass")
                }
                Button {
                    zoom = 1.0
                } label: {
                    Text("1×").font(.caption.monospacedDigit())
                }
                Button {
                    zoom = min(zoom * Self.step, Self.maxZoom)
                } label: {
                    Image(systemName: "plus.magnifyingglass")
                }
            }
            .buttonStyle(.bordered)
            .controlSize(.small)
            .padding(8)
        }
    }
}

struct HousePreviewMetalView: UIViewRepresentable {
    @Environment(SequencerViewModel.self) var viewModel
    let zoom: CGFloat

    func makeCoordinator() -> Coordinator {
        Coordinator()
    }

    func makeUIView(context: Context) -> MTKView {
        let view = MTKView()
        view.device = MTLCreateSystemDefaultDevice()
        view.colorPixelFormat = .bgra8Unorm
        view.clearColor = MTLClearColorMake(0, 0, 0, 1)
        // Use on-demand drawing — we trigger via setNeedsDisplay
        view.isPaused = true
        view.enableSetNeedsDisplay = true
        view.delegate = context.coordinator

        context.coordinator.bridge = XLMetalBridge(name: "HousePreview")
        if let layer = view.layer as? CAMetalLayer {
            context.coordinator.bridge?.attach(layer)
        }
        context.coordinator.mtkView = view

        let pinch = UIPinchGestureRecognizer(target: context.coordinator,
                                             action: #selector(Coordinator.handlePinch(_:)))
        view.addGestureRecognizer(pinch)

        return view
    }

    func updateUIView(_ uiView: MTKView, context: Context) {
        context.coordinator.viewModel = viewModel
        context.coordinator.zoom = zoom
        context.coordinator.bridge?.setCameraZoom(Float(zoom))
        if viewModel.isPlaying {
            if context.coordinator.displayLink == nil {
                context.coordinator.startDisplayLink(frameIntervalMS: viewModel.frameIntervalMS)
            }
        } else {
            context.coordinator.stopDisplayLink()
            uiView.setNeedsDisplay()
        }
    }

    class Coordinator: NSObject, MTKViewDelegate {
        var bridge: XLMetalBridge?
        var viewModel: SequencerViewModel?
        weak var mtkView: MTKView?
        var displayLink: CADisplayLink?
        private var lastDrawnMS: Int = -1

        // Camera zoom — applied by the bridge to iPadModelPreview's view
        // matrix (moves the camera closer/farther along Z).
        var zoom: CGFloat = 1.0 {
            didSet { if zoom != oldValue { mtkView?.setNeedsDisplay() } }
        }
        private var pinchStartZoom: CGFloat = 1.0

        @objc func handlePinch(_ recognizer: UIPinchGestureRecognizer) {
            switch recognizer.state {
            case .began:
                pinchStartZoom = zoom
            case .changed:
                let next = pinchStartZoom * recognizer.scale
                zoom = min(max(next, 0.5), 20.0)
            default:
                break
            }
        }

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

        func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {
            let scale = view.contentScaleFactor
            bridge?.setDrawableSize(size, scale: scale)
        }

        func draw(in view: MTKView) {
            guard let viewModel, let bridge else { return }

            bridge.drawModels(for: viewModel.document, atMS: Int32(viewModel.playPositionMS), pointSize: 2.0)
        }

        deinit {
            stopDisplayLink()
        }
    }
}
