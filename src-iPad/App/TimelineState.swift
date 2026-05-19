import CoreGraphics
import Observation
import QuartzCore
import SwiftUI

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

    /// Mach-time stamp of the most recent user pan/pinch on any of the
    /// grid canvases. Follow-playhead auto-scroll (B93) suppresses itself
    /// for a short grace window after the last interaction so a user who
    /// scrolls during playback has a moment to look before the playhead
    /// drags the viewport back. Read from the AutoFollowPlayhead subview;
    /// deliberately *not* observed — updates must not invalidate views.
    @ObservationIgnored var lastUserInteractionAt: CFTimeInterval = 0

    func contentWidth(forDurationMS durationMS: Int) -> CGFloat {
        max(CGFloat(durationMS) * pixelsPerMS, 1)
    }

    func timeMS(atX x: CGFloat) -> Int { Int(x / pixelsPerMS) }
    func x(forTimeMS ms: Int) -> CGFloat { CGFloat(ms) * pixelsPerMS }

    func noteUserInteraction() {
        lastUserInteractionAt = CACurrentMediaTime()
    }
}

// F-4 — focusable bridge between the scene's command bar and the
// SequencerView's local timeline state. The menu-bar zoom items
// (View → Zoom In / Zoom Out) read the current instance via
// `@FocusedValue(\.timeline)`; SequencerView exposes it with
// `.focusedValue(\.timeline, timeline)`.
struct TimelineFocusedValueKey: FocusedValueKey {
    typealias Value = TimelineState
}

extension FocusedValues {
    var timeline: TimelineState? {
        get { self[TimelineFocusedValueKey.self] }
        set { self[TimelineFocusedValueKey.self] = newValue }
    }
}
