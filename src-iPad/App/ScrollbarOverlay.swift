import SwiftUI

/// B94 — thin draggable scrollbar overlay for the Metal-backed
/// grid. The effects canvas scrolls via explicit bindings
/// (`TimelineState.hScrollOffsetPx`, `RowsScrollState.vScrollOffsetPx`)
/// not a native ScrollView, so system scroll indicators never
/// render. This subview paints its own track + thumb on top of
/// the Metal canvas so a trackpad user has both a position
/// readout and a draggable handle.
///
/// Pinned at the trailing edge (vertical) or bottom edge
/// (horizontal) of the parent canvas. Consumes hits only on the
/// thumb rect; the track itself is hit-transparent so taps in
/// the margin still fall through to the Metal canvas.
///
/// Auto-hide: the scrollbar fades in on any change to `offset` or
/// to `viewportSize` / `contentSize`, then fades out 1.5 s after
/// the most recent change. Drag-in-progress keeps it visible.
struct ScrollbarOverlay: View {
    enum Orientation { case horizontal, vertical }

    let orientation: Orientation
    let viewportSize: CGFloat
    let contentSize: CGFloat
    @Binding var offset: CGFloat

    /// Called on `.onChanged` / `.onEnded` so the parent can defer
    /// the follow-playhead auto-scroll (B93) while the user drags.
    var onUserInteraction: (() -> Void)? = nil

    var thickness: CGFloat = 6
    var minThumbLength: CGFloat = 24

    @State private var visible: Bool = false
    @State private var hideWorkItem: DispatchWorkItem?
    @State private var dragStartOffset: CGFloat = 0

    var body: some View {
        GeometryReader { geo in
            let trackLength = orientation == .horizontal ? geo.size.width : geo.size.height
            let needsBar = contentSize > viewportSize + 0.5 && trackLength > 0
            if needsBar {
                let maxOffset = max(1, contentSize - viewportSize)
                let ratio = viewportSize / contentSize
                let thumbLen = max(minThumbLength, trackLength * ratio)
                let trackFree = max(1, trackLength - thumbLen)
                let clampedOffset = max(0, min(maxOffset, offset))
                let thumbPos = trackFree * (clampedOffset / maxOffset)
                ZStack(alignment: orientation == .horizontal ? .leading : .top) {
                    // Track — very faint, only visible when bar is visible.
                    Capsule()
                        .fill(Color.white.opacity(0.08))
                        .frame(
                            width: orientation == .horizontal ? trackLength : thickness,
                            height: orientation == .horizontal ? thickness : trackLength
                        )
                        .allowsHitTesting(false)
                    // Thumb — the only hit-target.
                    Capsule()
                        .fill(Color.white.opacity(0.45))
                        .frame(
                            width: orientation == .horizontal ? thumbLen : thickness,
                            height: orientation == .horizontal ? thickness : thumbLen
                        )
                        .offset(
                            x: orientation == .horizontal ? thumbPos : 0,
                            y: orientation == .horizontal ? 0 : thumbPos
                        )
                        .contentShape(Capsule())
                        .hoverEffect(.highlight)
                        .gesture(dragGesture(trackFree: trackFree, maxOffset: maxOffset))
                }
                .opacity(visible ? 1.0 : 0.0)
                .animation(.easeInOut(duration: 0.15), value: visible)
                .onAppear { showThenHide() }
                .onChange(of: offset) { _, _ in showThenHide() }
                .onChange(of: viewportSize) { _, _ in showThenHide() }
                .onChange(of: contentSize) { _, _ in showThenHide() }
            }
        }
        .frame(
            width: orientation == .vertical ? thickness : nil,
            height: orientation == .horizontal ? thickness : nil
        )
    }

    private func dragGesture(trackFree: CGFloat, maxOffset: CGFloat) -> some Gesture {
        DragGesture(minimumDistance: 0)
            .onChanged { value in
                if value.translation == .zero {
                    dragStartOffset = offset
                    visible = true
                    cancelHide()
                    return
                }
                let delta = orientation == .horizontal
                    ? value.translation.width
                    : value.translation.height
                let newOffset = dragStartOffset + (delta / trackFree) * maxOffset
                offset = max(0, min(maxOffset, newOffset))
                onUserInteraction?()
            }
            .onEnded { _ in
                onUserInteraction?()
                showThenHide()
            }
    }

    private func showThenHide() {
        visible = true
        cancelHide()
        let work = DispatchWorkItem { visible = false }
        hideWorkItem = work
        DispatchQueue.main.asyncAfter(deadline: .now() + 1.5, execute: work)
    }

    private func cancelHide() {
        hideWorkItem?.cancel()
        hideWorkItem = nil
    }
}
