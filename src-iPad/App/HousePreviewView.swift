import SwiftUI
import MetalKit

struct HousePreviewView: UIViewRepresentable {
    @Environment(SequencerViewModel.self) var viewModel

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

        return view
    }

    func updateUIView(_ uiView: MTKView, context: Context) {
        context.coordinator.viewModel = viewModel
        if viewModel.isRenderDone {
            // Start display link for continuous updates during playback
            if viewModel.isPlaying {
                if context.coordinator.displayLink == nil {
                    context.coordinator.startDisplayLink(frameIntervalMS: viewModel.frameIntervalMS)
                }
            } else {
                context.coordinator.stopDisplayLink()
                // Static redraw for current position
                uiView.setNeedsDisplay()
            }
        } else {
            context.coordinator.stopDisplayLink()
        }
    }

    class Coordinator: NSObject, MTKViewDelegate {
        var bridge: XLMetalBridge?
        var viewModel: SequencerViewModel?
        weak var mtkView: MTKView?
        var displayLink: CADisplayLink?
        private var lastDrawnMS: Int = -1

        func startDisplayLink(frameIntervalMS: Int) {
            stopDisplayLink()
            let link = CADisplayLink(target: self, selector: #selector(displayLinkFired))
            // Match sequence frame rate
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
            guard let viewModel, viewModel.isRenderDone else { return }
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
            guard viewModel.isRenderDone else { return }

            bridge.drawModels(for: viewModel.document, atMS: Int32(viewModel.playPositionMS), pointSize: 2.0)
        }

        deinit {
            stopDisplayLink()
        }
    }
}
