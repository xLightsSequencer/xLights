import CoreGraphics
import Observation

/// Shared vertical-scroll state for the effects grid. Row 3's two cells
/// (model headers and main grid) bind to this. Rows 1 and 2 do not —
/// they're vertically locked.
@Observable
final class RowsScrollState {
    var vScrollOffsetPx: CGFloat = 0
}
