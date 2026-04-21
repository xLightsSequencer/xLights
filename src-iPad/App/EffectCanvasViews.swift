import SwiftUI
import UIKit

/// Callbacks the grid canvas delivers to SwiftUI for user gestures.
/// The canvas is gesture-aware because a single UIView is the simplest
/// place to do precise hit-testing across thousands of effects.
struct EffectCanvasActions {
    var onTapEffect: (_ row: Int, _ effect: Int) -> Void = { _,_ in }
    /// Fires on any tap that didn't land on an effect. When the tap
    /// was inside a model row (but on empty time), `rowIndex` and `ms`
    /// are non-nil so the caller can create an effect there; on an
    /// outside-rows tap both are nil.
    var onTapEmpty: (_ rowIndex: Int?, _ ms: Int?) -> Void = { _,_ in }
    var onMoveEffect: (_ row: Int, _ effect: Int, _ newStartMS: Int, _ newEndMS: Int) -> Void = { _,_,_,_ in }
    /// Move an effect to a *different* row. Fires only when the drag
    /// ended on a row whose id differs from the effect's home row.
    /// Source-row effects shift up to fill the gap, target row shifts
    /// to accommodate.
    var onMoveEffectToRow: (_ srcRow: Int, _ effect: Int, _ dstRow: Int,
                             _ newStartMS: Int, _ newEndMS: Int) -> Void = { _,_,_,_,_ in }
    var onResizeEdge: (_ row: Int, _ effect: Int, _ edge: Int, _ newMS: Int) -> Void = { _,_,_,_ in }
    /// Fires when a fade-in or fade-out drag ends. `edge`: 0 = fade-in,
    /// 1 = fade-out. `seconds` is the new committed fade duration.
    var onAdjustFade: (_ row: Int, _ effect: Int, _ edge: Int, _ seconds: Float) -> Void = { _,_,_,_ in }
    /// Called from every drag tick with the current active-drag
    /// snapshot (nil on end). Plumbed into `SequencerViewModel.activeDrag`
    /// so the Metal grid can render live feedback.
    var onActiveDragChanged: (_ drag: SequencerViewModel.ActiveDrag?) -> Void = { _ in }
    var onPinchZoom: (_ scaleDelta: CGFloat, _ anchorX: CGFloat) -> Void = { _,_ in }
    /// Fires when the user taps the ruler / waveform to set the play
    /// position. `ms` is the time the tap landed on.
    var onSeekToMS: (_ ms: Int) -> Void = { _ in }
    /// Fires when a long-press selects an effect. The canvas converts
    /// the touch location into view-space coordinates so the menu can
    /// anchor near the finger.
    var onRequestContextMenu: (_ row: Int, _ effect: Int, _ anchorInCanvas: CGPoint) -> Void = { _,_,_ in }
}

/// State provider for per-effect lock / disabled look. Keeping this as a
/// struct of closures avoids threading every flag through the view data
/// array when the underlying C++ model is the source of truth.
struct EffectStateLookup {
    var isLocked: (_ row: Int, _ effect: Int) -> Bool = { _,_ in false }
    var isDisabled: (_ row: Int, _ effect: Int) -> Bool = { _,_ in false }
}
