import CoreGraphics
import Observation

/// Shared horizontal-scroll + zoom state for the effects grid. The three
/// right-column cells (ruler+waveform, timing effects, main grid) all bind
/// to this so they stay aligned. Sequence duration is NOT held here — it
/// lives on the view model and is read directly to avoid initial-render
/// races where a stale zero duration would yield a collapsed content width
/// that the scroll views' hosted SwiftUI Canvases couldn't recover from.
@Observable
final class TimelineState {
    var hScrollOffsetPx: CGFloat = 0
    var pixelsPerMS: CGFloat = 0.1

    func contentWidth(forDurationMS durationMS: Int) -> CGFloat {
        max(CGFloat(durationMS) * pixelsPerMS, 1)
    }

    func timeMS(atX x: CGFloat) -> Int { Int(x / pixelsPerMS) }
    func x(forTimeMS ms: Int) -> CGFloat { CGFloat(ms) * pixelsPerMS }
}
