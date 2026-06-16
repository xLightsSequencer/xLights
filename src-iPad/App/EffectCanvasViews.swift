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
    /// Shared-edge resize commit (Pencil Pro squeeze + edge drag).
    /// Two butted effects on the same row have their shared
    /// boundary moved together — one grows while the other shrinks.
    /// Arg order: `(row, leftEffectIdx, leftStartMS, leftEndMS,
    /// rightEffectIdx, rightStartMS, rightEndMS)`. The view model
    /// commits both moves inside one undo group.
    var onResizeSharedEdge: ((_ row: Int,
                               _ leftIdx: Int, _ leftStart: Int, _ leftEnd: Int,
                               _ rightIdx: Int, _ rightStart: Int, _ rightEnd: Int) -> Void)?
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
    /// Fires when a long-press lands on a visible transition diamond
    /// (the in/out fade bar at the top corners of an effect). `isIn`
    /// distinguishes the in-side from the out-side so the picker writes
    /// to the right `T_CHOICE_*_Transition_Type` key.
    var onRequestTransitionMenu: (_ row: Int, _ effect: Int, _ isIn: Bool,
                                   _ anchorInCanvas: CGPoint) -> Void = { _,_,_,_ in }
    /// B18: double-tap in empty space. The canvas passes the row id
    /// plus the `ms` it landed on; the outer view decides whether
    /// to create an effect (palette armed) filling the cell.
    var onDoubleTapEmpty: (_ rowIndex: Int, _ ms: Int) -> Void = { _,_ in }
    /// Drag-create (desktop parity): a horizontal drag across empty
    /// space on a model row, with a palette effect armed, creates a
    /// new effect spanning `[startMS, endMS]`. Fired once on drag end.
    var onCreateEffectDrag: (_ rowIndex: Int, _ startMS: Int, _ endMS: Int) -> Void = { _,_,_ in }
    /// True iff a palette effect is currently armed. Gates the
    /// drag-create path so an empty-space pan still scrolls normally
    /// when nothing is armed.
    var isPaletteArmed: () -> Bool = { false }
    /// ⌘/Ctrl-tap on an effect — toggle its membership in the
    /// multi-selection (additive). Desktop `EffectsGrid` Ctrl-click.
    var onToggleSelectEffect: (_ row: Int, _ effect: Int) -> Void = { _,_ in }
    /// ⇧-tap on an effect — extend the selection from the current
    /// anchor to the tapped effect. Desktop `EffectsGrid` Shift-click.
    var onExtendSelectEffect: (_ row: Int, _ effect: Int) -> Void = { _,_ in }
}

/// State provider for per-effect lock / disabled look. Keeping this as a
/// struct of closures avoids threading every flag through the view data
/// array when the underlying C++ model is the source of truth.
struct EffectStateLookup {
    var isLocked: (_ row: Int, _ effect: Int) -> Bool = { _,_ in false }
    var isDisabled: (_ row: Int, _ effect: Int) -> Bool = { _,_ in false }
}
