import Observation
import SwiftUI
import UIKit

/// A UIScrollView-backed container whose content offset is syncable with
/// shared state. Each instance can lock one or both axes to externally
/// supplied target offsets. When the user drags, the onScroll callback
/// fires so the caller can push the new offset back into shared state;
/// when shared state changes, the caller passes a new target offset and
/// the view updates programmatically (with the delegate suppressed to
/// avoid echo).
///
/// SwiftUI content is hosted via UIHostingController. The hosting
/// controller's view is sized via explicit width/height constraints so
/// the scroll view's contentSize matches.
struct SyncedScrollView<Content: View>: UIViewRepresentable {
    /// nil = this axis is free (user scroll only, no external sync).
    let targetHOffset: CGFloat?
    /// nil = this axis is free.
    let targetVOffset: CGFloat?
    let contentWidth: CGFloat
    let contentHeight: CGFloat
    /// Whether to show scroll indicators. For follower views (top chrome,
    /// row headers) we usually hide indicators; the driver grid shows them.
    let showsIndicators: Bool
    let onScroll: (CGPoint) -> Void
    /// Vertical offset to follow, read by the coordinator under its own
    /// observation tracking. Use this instead of `targetVOffset` for an offset
    /// that changes every scroll frame: passing it as a value means the caller's
    /// body reads it, so every frame re-runs that body and rebuilds `content`.
    var followVOffset: (() -> CGFloat)? = nil
    @ViewBuilder let content: () -> Content

    func makeCoordinator() -> Coordinator {
        Coordinator(onScroll: onScroll)
    }

    func makeUIView(context: Context) -> UIScrollView {
        let sv = UIScrollView()
        sv.delegate = context.coordinator
        sv.showsHorizontalScrollIndicator = showsIndicators && targetHOffset == nil
        sv.showsVerticalScrollIndicator = showsIndicators && targetVOffset == nil && followVOffset == nil
        sv.bounces = true
        sv.bouncesZoom = false
        sv.decelerationRate = .normal
        sv.contentInsetAdjustmentBehavior = .never
        sv.isDirectionalLockEnabled = false

        let host = UIHostingController(rootView: content())
        host.view.backgroundColor = .clear
        host.view.translatesAutoresizingMaskIntoConstraints = false
        sv.addSubview(host.view)

        let widthC = host.view.widthAnchor.constraint(equalToConstant: contentWidth)
        let heightC = host.view.heightAnchor.constraint(equalToConstant: contentHeight)
        NSLayoutConstraint.activate([
            host.view.leadingAnchor.constraint(equalTo: sv.contentLayoutGuide.leadingAnchor),
            host.view.topAnchor.constraint(equalTo: sv.contentLayoutGuide.topAnchor),
            widthC,
            heightC,
        ])
        context.coordinator.hostingController = host
        context.coordinator.widthConstraint = widthC
        context.coordinator.heightConstraint = heightC
        if let follow = followVOffset {
            context.coordinator.followVOffset = follow
            context.coordinator.scrollView = sv
            // Deferred so the first read happens outside this SwiftUI update:
            // a read here would be merged into the representable's own
            // tracking and bring back the per-frame update this avoids.
            DispatchQueue.main.async { [weak coordinator = context.coordinator] in
                coordinator?.trackVOffset()
            }
        }
        return sv
    }

    func updateUIView(_ sv: UIScrollView, context: Context) {
        // Latest callback closure.
        context.coordinator.onScroll = onScroll
        if let follow = followVOffset {
            context.coordinator.followVOffset = follow
        }

        // Keep the hosted content fresh — pushes any view-model changes
        // into the rendered content.
        context.coordinator.hostingController?.rootView = content()

        // Size the inner view and the scroll view's content to match.
        if context.coordinator.widthConstraint?.constant != contentWidth {
            context.coordinator.widthConstraint?.constant = contentWidth
        }
        if context.coordinator.heightConstraint?.constant != contentHeight {
            context.coordinator.heightConstraint?.constant = contentHeight
        }
        if sv.contentSize != CGSize(width: contentWidth, height: contentHeight) {
            sv.contentSize = CGSize(width: contentWidth, height: contentHeight)
        }

        // Programmatic offset sync, suppress delegate to avoid feedback
        // loops.
        let current = sv.contentOffset
        let newX = targetHOffset ?? current.x
        let newY = targetVOffset ?? current.y
        let target = CGPoint(x: newX, y: newY)
        if abs(current.x - target.x) > 0.5 || abs(current.y - target.y) > 0.5 {
            context.coordinator.ignoreDelegate = true
            sv.setContentOffset(target, animated: false)
            context.coordinator.ignoreDelegate = false
        }
    }

    final class Coordinator: NSObject, UIScrollViewDelegate {
        var onScroll: (CGPoint) -> Void
        var hostingController: UIHostingController<Content>?
        var widthConstraint: NSLayoutConstraint?
        var heightConstraint: NSLayoutConstraint?
        var ignoreDelegate = false
        var followVOffset: (() -> CGFloat)?
        weak var scrollView: UIScrollView?

        init(onScroll: @escaping (CGPoint) -> Void) { self.onScroll = onScroll }

        /// Applies the followed offset and re-arms tracking; Observation's
        /// onChange fires once per registration.
        func trackVOffset() {
            guard let follow = followVOffset, let sv = scrollView else { return }
            let y = withObservationTracking {
                follow()
            } onChange: { [weak self] in
                DispatchQueue.main.async { self?.trackVOffset() }
            }
            if abs(sv.contentOffset.y - y) > 0.5 {
                ignoreDelegate = true
                sv.setContentOffset(CGPoint(x: sv.contentOffset.x, y: y), animated: false)
                ignoreDelegate = false
            }
        }

        func scrollViewDidScroll(_ scrollView: UIScrollView) {
            guard !ignoreDelegate else { return }
            onScroll(scrollView.contentOffset)
        }
    }
}
