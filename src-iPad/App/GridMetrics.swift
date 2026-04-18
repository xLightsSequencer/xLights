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
    var edgeHandleHitWidth: CGFloat = 24
    var transitionHandleHitWidth: CGFloat = 20
    var topLeftCornerHeight: CGFloat { rulerHeight + waveformHeight }

    var selectedRowHeight: CGFloat { rowHeight * selectedRowMultiplier }
    var topChromeHeight: CGFloat { rulerHeight + waveformHeight }

    static let standard = GridMetrics()
}
