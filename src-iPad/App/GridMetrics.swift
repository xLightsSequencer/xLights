import CoreGraphics

/// Configurable sizing constants for the effects grid. All heights live here
/// so a future preference pane can adjust them without touching grid code.
/// Zoom does not change these values — desktop parity, zoom only changes
/// `TimelineState.pixelsPerMS`.
struct GridMetrics: Equatable {
    var rowHeight: CGFloat = 24
    var selectedRowMultiplier: CGFloat = 1.5
    var timingRowHeight: CGFloat = 24
    var rulerHeight: CGFloat = 24
    var waveformHeight: CGFloat = 48
    var rowHeaderWidth: CGFloat = 180
    // Finger hit-slop on effect edge handles. 32 pt → ±16 pt on
    // each side of the edge boundary, so a ~6 mm fingertip has
    // comfortable tolerance on short effects at medium zoom.
    // Bumped from 24 pt after real-device testing showed edges
    // were hard to grab.
    var edgeHandleHitWidth: CGFloat = 32
    var transitionHandleHitWidth: CGFloat = 20
    var topLeftCornerHeight: CGFloat { rulerHeight + waveformHeight }

    var selectedRowHeight: CGFloat { rowHeight * selectedRowMultiplier }
    var topChromeHeight: CGFloat { rulerHeight + waveformHeight }

    static let standard = GridMetrics()
}
