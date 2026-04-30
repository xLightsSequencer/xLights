import SwiftUI
import UIKit

/// Reports whether the host scene currently fills the device screen
/// (no Stage Manager / Slide Over chrome). iPadOS does not expose
/// this directly via SwiftUI — Apple recommends comparing the
/// scene's coordinate-space bounds to the screen's bounds. The
/// probe attaches as a hidden background view, observes layout
/// changes (which fire on rotation, Stage Manager resize, and
/// initial mount), and pushes the result into the bound flag.
///
/// We use this to suppress the 80pt leading toolbar padding when
/// the Stage Manager close/-/full pill is not present — that pill
/// only appears when the scene is windowed (Stage Manager / Slide
/// Over). In fullscreen the toolbar can hug the leading edge.
struct WindowingModeProbe: UIViewRepresentable {
    @Binding var isFullScreen: Bool

    func makeUIView(context: Context) -> ProbeView {
        ProbeView { [self] full in
            DispatchQueue.main.async {
                if isFullScreen != full { isFullScreen = full }
            }
        }
    }

    func updateUIView(_ uiView: ProbeView, context: Context) {}

    final class ProbeView: UIView {
        let onLayout: (Bool) -> Void

        init(onLayout: @escaping (Bool) -> Void) {
            self.onLayout = onLayout
            super.init(frame: .zero)
            isHidden = true
            isUserInteractionEnabled = false
        }

        required init?(coder: NSCoder) { fatalError("not supported") }

        override func didMoveToWindow() {
            super.didMoveToWindow()
            report()
        }

        override func layoutSubviews() {
            super.layoutSubviews()
            report()
        }

        private func report() {
            guard let scene = window?.windowScene else {
                onLayout(true)
                return
            }
            let sceneSize = scene.effectiveGeometry.coordinateSpace.bounds.size
            let screenSize = scene.screen.bounds.size
            let tol: CGFloat = 1
            let full = abs(sceneSize.width - screenSize.width) < tol
                && abs(sceneSize.height - screenSize.height) < tol
            onLayout(full)
        }
    }
}
