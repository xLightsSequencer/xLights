import SwiftUI
import MetalKit

struct MetalCanvasView: UIViewRepresentable {
    func makeCoordinator() -> Coordinator {
        Coordinator()
    }

    func makeUIView(context: Context) -> MTKView {
        let view = MTKView()
        view.device = MTLCreateSystemDefaultDevice()
        view.colorPixelFormat = .bgra8Unorm
        view.clearColor = MTLClearColorMake(0, 0, 0, 1)
        view.enableSetNeedsDisplay = true
        view.isPaused = true
        view.delegate = context.coordinator

        context.coordinator.bridge = XLMetalBridge(name: "iPadCanvas")
        if let layer = view.layer as? CAMetalLayer {
            context.coordinator.bridge?.attach(layer)
        }

        return view
    }

    func updateUIView(_ uiView: MTKView, context: Context) {
        uiView.setNeedsDisplay()
    }

    class Coordinator: NSObject, MTKViewDelegate {
        var bridge: XLMetalBridge?

        func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {
            let scale = view.contentScaleFactor
            bridge?.setDrawableSize(size, scale: scale)
        }

        func draw(in view: MTKView) {
            // No-op — this is the test view, use HousePreviewView for actual rendering
        }
    }
}
