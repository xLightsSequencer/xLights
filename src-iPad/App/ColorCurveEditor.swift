import SwiftUI
import UIKit

// Palette ColorCurve editor (G16 — C5). Opens as a modal sheet from
// a long-press on a palette slot; mirrors desktop's ColorCurveDialog
// but consolidates the time / spatial-mode picker that desktop
// exposes as a cycle-button on the palette grid into a single
// segmented control inside the sheet.
//
// The backing ColorCurve string round-trips byte-for-byte with
// desktop via `XLColorCurve`, so a curve authored on iPad displays
// and renders identically when the sequence is opened on macOS.

// MARK: - Observable wrapper

@Observable
@MainActor
final class EditableColorCurve {
    @ObservationIgnored let core: XLColorCurve
    @ObservationIgnored let storageKey: String
    @ObservationIgnored weak var viewModel: SequencerViewModel?

    var revision: Int = 0  // bumped after every mutation to kick SwiftUI

    init(core: XLColorCurve, storageKey: String, viewModel: SequencerViewModel?) {
        self.core = core
        self.storageKey = storageKey
        self.viewModel = viewModel
    }

    /// Re-serialise and push to the underlying settings map. Inactive
    /// curves persist as `"Active=FALSE|"` which the palette hex
    /// reader then treats as "no curve" — `suppressIfDefault` would
    /// strip the key but the palette default is the slot's hex
    /// colour, so we always write the raw serialised form instead.
    private func persist() {
        guard let vm = viewModel else { return }
        let s = core.serialise()
        vm.setSettingValue(s, forKey: storageKey)
        revision &+= 1
    }

    var active: Bool {
        get { core.active }
        set { core.active = newValue; persist() }
    }

    var mode: XLColorCurveMode {
        get { core.mode }
        set { core.mode = newValue; persist() }
    }

    func setPoint(x: Float, color: UIColor) {
        core.setPoint(x: x, color: color)
        persist()
    }

    func deletePoint(x: Float) {
        core.deletePoint(x: x)
        persist()
    }

    func flip() {
        core.flip()
        persist()
    }

    func colorAt(offset: Float) -> UIColor {
        core.color(atOffset: offset)
    }

    /// Returns the stored points as (x, SwiftUI.Color) tuples,
    /// sorted by x. Rebuild on every access — count is small
    /// (usually ≤ 8 points).
    func points() -> [(x: Float, color: Color)] {
        let raw = core.points()
        return raw.compactMap { dict -> (Float, Color)? in
            guard let xnum = dict["x"] as? NSNumber,
                  let ui = dict["color"] as? UIColor else {
                return nil
            }
            return (Float(xnum.doubleValue), Color(uiColor: ui))
        }
        .sorted { $0.0 < $1.0 }
    }
}

// MARK: - Editor sheet

struct ColorCurveEditorSheet: View {
    @Environment(SequencerViewModel.self) var viewModel
    @Environment(\.dismiss) private var dismiss

    let slot: Int                // 1..8, drives identifier + storage key
    let storedString: String     // existing serialised form (may be empty)
    let supportsLinear: Bool     // greys-out spatial group if false
    let supportsRadial: Bool     // greys-out radial + rotation groups if false

    @State private var curve: EditableColorCurve?
    @State private var selectedPointX: Float? = nil

    private var storageKey: String { "C_BUTTON_Palette\(slot)" }
    private var identifier: String { "Palette\(slot)" }

    var body: some View {
        NavigationStack {
            Group {
                if let curve = curve {
                    editorContent(curve)
                } else {
                    ProgressView()
                }
            }
            .navigationTitle("Palette \(slot) Gradient")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Done") { dismiss() }
                }
            }
        }
        .onAppear { buildCurve() }
    }

    private func buildCurve() {
        let core = XLColorCurve(serialised: storedString, identifier: identifier)
        // Ensure a usable default when transitioning a plain-colour
        // slot into curve mode: seed with two endpoints (black→white)
        // so the gradient strip isn't a single flat colour.
        if !core.active && core.points().count <= 1 {
            core.setPoint(x: 0.0, color: .black)
            core.setPoint(x: 1.0, color: .white)
        }
        curve = EditableColorCurve(core: core,
                                   storageKey: storageKey,
                                   viewModel: viewModel)
    }

    @ViewBuilder
    private func editorContent(_ curve: EditableColorCurve) -> some View {
        Form {
            // Active toggle: unchecking writes `Active=FALSE|` so the
            // slot reverts to its plain hex colour on render.
            Section {
                Toggle(isOn: Binding(
                    get: { curve.active },
                    set: { curve.active = $0 }
                )) {
                    Text("Use gradient")
                        .font(.callout)
                }
                .disabled(false)
            }

            // Live preview strip with control points overlaid as
            // draggable markers (ColorCurveGradientStrip handles
            // tap-to-select / drag-to-move / long-press-to-delete).
            Section {
                ColorCurveGradientStrip(curve: curve,
                                        selectedX: $selectedPointX)
                    .frame(height: 56)
                    .padding(.vertical, 4)
                Text("Tap empty space to add a point. Drag a point to move it.")
                    .font(.caption2)
                    .foregroundStyle(.secondary)
            } header: {
                Text("Gradient")
            }

            // Colour picker + editable position for the currently-
            // selected point.
            if let sel = selectedPointX,
               let point = curve.points().first(where: { abs($0.x - sel) < 0.005 }) {
                Section {
                    ColorPicker("Colour", selection: Binding(
                        get: { point.color },
                        set: { newColor in
                            curve.setPoint(x: sel,
                                           color: UIColor(newColor))
                        }
                    ), supportsOpacity: false)
                    ColorCurvePositionField(
                        currentX: sel,
                        onCommit: { newX in
                            movePoint(curve: curve, fromX: sel, toX: newX)
                            // Keep the Selected Point section alive
                            // at the new x so the colour picker
                            // doesn't jump away mid-edit.
                            selectedPointX = newX
                        })
                    Button(role: .destructive) {
                        curve.deletePoint(x: sel)
                        selectedPointX = nil
                    } label: {
                        Label("Delete Point", systemImage: "trash")
                    }
                } header: {
                    Text("Selected Point")
                }
            }

            // Time / spatial mode picker. Groups are greyed out when
            // the current effect doesn't support that ColorCurve
            // kind.
            Section {
                ColorCurveModePicker(mode: Binding(
                    get: { curve.mode },
                    set: { curve.mode = $0 }
                ),
                supportsLinear: supportsLinear,
                supportsRadial: supportsRadial)
            } header: {
                Text("Mode")
            } footer: {
                Text(modeFooter)
                    .font(.caption2)
            }

            // Transforms.
            Section {
                Button {
                    curve.flip()
                } label: {
                    Label("Flip Horizontally", systemImage: "arrow.left.arrow.right")
                }
            }
        }
    }

    private var modeFooter: String {
        switch (supportsLinear, supportsRadial) {
        case (true, true):
            return "This effect supports time-swept, linear-spatial, and radial / rotational gradients."
        case (true, false):
            return "This effect supports time-swept and linear-spatial gradients. Radial / rotational modes are disabled."
        case (false, true):
            return "This effect supports time-swept and radial / rotational gradients. Linear spatial modes are disabled."
        case (false, false):
            return "This effect only supports time-swept gradients."
        }
    }
}

// MARK: - Gradient strip + point markers

struct ColorCurveGradientStrip: View {
    @Bindable var curve: EditableColorCurve
    @Binding var selectedX: Float?

    /// The point x currently being manipulated by the active drag.
    /// Nil when no drag is in flight. Tracked at the strip level
    /// (not per-marker) so SwiftUI re-rendering the ForEach after
    /// each point mutation doesn't lose the drag target.
    @State private var draggingX: Float? = nil

    /// Set to a fresh gesture start time so we can distinguish
    /// a tap (no movement) from a short drag. Per SwiftUI's
    /// DragGesture(minimumDistance:0), `onChanged` fires even for a
    /// single tap — we want the tap-on-empty-space path to add
    /// exactly one point rather than spamming onChanged.
    @State private var dragMoved: Bool = false

    /// Snap an x to ColorCurve's 1/100 grid. Mirrors
    /// `ccSortableColorPoint::Normalise` so Swift-side state matches
    /// what the curve actually stores — keeps `draggingX` aligned
    /// with the real point across consecutive drag ticks.
    private static func snap(_ x: Float) -> Float {
        let clamped = max(0, min(1, x))
        return (clamped * 100).rounded() / 100
    }

    var body: some View {
        GeometryReader { geom in
            ZStack(alignment: .topLeading) {
                // Rendered gradient — sample the curve at each
                // column so all 9 modes (time / linear / radial / CW)
                // show a plausible preview. For spatial/radial modes
                // the strip just shows the underlying point ramp —
                // desktop does the same.
                Canvas { ctx, size in
                    let w = Int(size.width)
                    guard w > 0 else { return }
                    for px in 0..<w {
                        let frac = Float(px) / Float(w)
                        let c = Color(uiColor: curve.colorAt(offset: frac))
                        let r = CGRect(x: CGFloat(px), y: 0,
                                       width: 1, height: size.height)
                        ctx.fill(Path(r), with: .color(c))
                    }
                }
                .clipShape(RoundedRectangle(cornerRadius: 6))
                .allowsHitTesting(false) // all gestures on the ZStack

                // Point markers — positioned, no per-marker gestures.
                // Gesture handling lives on the enclosing ZStack so
                // a drag that started on a marker keeps tracking that
                // marker even after the curve mutates and the
                // ForEach rebuilds.
                ForEach(curve.points(), id: \.x) { pt in
                    PointMarker(isSelected: selectedX == pt.x, color: pt.color)
                        .position(x: CGFloat(pt.x) * geom.size.width,
                                  y: geom.size.height / 2)
                        .allowsHitTesting(false)
                }
            }
            .contentShape(Rectangle())
            .gesture(
                DragGesture(minimumDistance: 0)
                    .onChanged { g in
                        let fingerX = Float(max(0, min(1,
                            g.location.x / max(geom.size.width, 1))))

                        // Drag-begin: either grab the nearest
                        // existing marker (within a finger-width in
                        // strip space) or queue a tap-to-add-point.
                        if draggingX == nil && !dragMoved {
                            let tolStrip: Float = Float(
                                max(16, min(28, geom.size.width * 0.04))
                                / max(geom.size.width, 1))
                            let points = curve.points()
                            let near = points.min {
                                abs($0.x - fingerX) < abs($1.x - fingerX)
                            }
                            if let n = near,
                               abs(n.x - fingerX) < tolStrip {
                                // Store the point's stored (snapped)
                                // x so the next-tick comparison
                                // against `cur` lines up with the
                                // curve's grid.
                                draggingX = n.x
                                selectedX = n.x
                            }
                            // Else: defer — onEnded will decide
                            // whether this was a tap (add a point)
                            // or a drag from empty space.
                        }

                        // If the gesture has moved, track
                        // displacement + relocate the active marker.
                        let moved = abs(g.translation.width) > 2
                                 || abs(g.translation.height) > 2
                        if moved { dragMoved = true }

                        if let cur = draggingX, dragMoved {
                            let newSnap = Self.snap(fingerX)
                            // Only relocate when the snapped grid
                            // cell actually changes — each drag
                            // frame mutation is expensive, and a
                            // sub-grid finger wiggle would otherwise
                            // thrash the point list.
                            if newSnap != cur {
                                // Look the colour up by snapped x
                                // so the match is exact against the
                                // curve's own storage.
                                let colour = curve.points()
                                    .first { $0.x == cur }?
                                    .color ?? .black
                                curve.deletePoint(x: cur)
                                curve.setPoint(x: newSnap,
                                               color: UIColor(colour))
                                draggingX = newSnap
                                selectedX = newSnap
                            }
                        }
                    }
                    .onEnded { g in
                        let fingerX = Float(max(0, min(1,
                            g.location.x / max(geom.size.width, 1))))
                        let wasTap = !dragMoved
                        let grabbedMarker = draggingX != nil

                        if wasTap && !grabbedMarker {
                            // Tap on empty strip → add a new point
                            // at this x, colour sampled from the
                            // current gradient.
                            let near = curve.points().contains {
                                abs($0.x - fingerX) < 0.02
                            }
                            if !near {
                                let colour = curve.colorAt(offset: fingerX)
                                curve.setPoint(x: fingerX, color: colour)
                                selectedX = fingerX
                            }
                        } else if wasTap && grabbedMarker {
                            // Tapped a marker without dragging →
                            // just select it (already selected
                            // during onChanged, keep for clarity).
                            if let x = draggingX { selectedX = x }
                        }

                        draggingX = nil
                        dragMoved = false
                    }
            )
        }
    }
}

/// Text field for the selected point's `x`. Accepts decimals in
/// [0, 1] (ColorCurve grid step is 1/100); commits on submit / blur.
/// Reverts to the current value on malformed input.
struct ColorCurvePositionField: View {
    let currentX: Float
    let onCommit: (Float) -> Void

    @State private var draft: String = ""
    @FocusState private var focused: Bool

    var body: some View {
        HStack {
            Text("Position")
                .font(.callout)
            Spacer()
            TextField("0.00", text: $draft)
                .multilineTextAlignment(.trailing)
                .monospacedDigit()
                .font(.callout)
                .keyboardType(.decimalPad)
                .textFieldStyle(.roundedBorder)
                .frame(maxWidth: 90)
                .focused($focused)
                .onAppear { draft = String(format: "%.2f", currentX) }
                .onChange(of: currentX) { _, newValue in
                    if !focused {
                        draft = String(format: "%.2f", newValue)
                    }
                }
                .onChange(of: focused) { _, isFocused in
                    if !isFocused { commit() }
                }
                .onSubmit { commit() }
                .toolbar {
                    ToolbarItemGroup(placement: .keyboard) {
                        if focused {
                            Spacer()
                            Button("Done") { focused = false }
                        }
                    }
                }
        }
    }

    private func commit() {
        let trimmed = draft.trimmingCharacters(in: .whitespaces)
        guard let parsed = Float(trimmed) else {
            draft = String(format: "%.2f", currentX)
            return
        }
        let clamped = max(0, min(1, parsed))
        if abs(clamped - currentX) > 0.005 {
            onCommit(clamped)
        } else {
            draft = String(format: "%.2f", currentX)
        }
    }
}

/// Move the point at `fromX` to `toX` (same colour). Used by the
/// keyboard-commit path on the position field + any other
/// keyboard-driven moves. Grid-snapping happens inside the ObjC
/// wrapper.
@MainActor
private func movePoint(curve: EditableColorCurve,
                       fromX: Float, toX: Float) {
    guard abs(toX - fromX) > 0.005 else { return }
    let colour = curve.points()
        .first { abs($0.x - fromX) < 0.005 }?
        .color ?? .black
    curve.deletePoint(x: fromX)
    curve.setPoint(x: toX, color: UIColor(colour))
}

private struct PointMarker: View {
    let isSelected: Bool
    let color: Color

    var body: some View {
        ZStack {
            Circle()
                .fill(color)
                .frame(width: 18, height: 18)
            Circle()
                .stroke(isSelected ? Color.accentColor : Color.white,
                        lineWidth: isSelected ? 3 : 2)
                .frame(width: 18, height: 18)
        }
        .shadow(color: .black.opacity(0.4), radius: 1, x: 0, y: 1)
    }
}

// MARK: - Mode picker

struct ColorCurveModePicker: View {
    @Binding var mode: XLColorCurveMode
    let supportsLinear: Bool
    let supportsRadial: Bool

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            ColorCurveModeRow(
                label: "Time",
                systemImage: "clock",
                modes: [(.time, "Time")],
                current: $mode,
                enabled: true)

            ColorCurveModeRow(
                label: "Linear (spatial)",
                systemImage: "arrow.left.and.right",
                modes: [(.right, "→"),
                        (.left, "←"),
                        (.down, "↓"),
                        (.up, "↑")],
                current: $mode,
                enabled: supportsLinear)

            ColorCurveModeRow(
                label: "Radial",
                systemImage: "target",
                modes: [(.radialIn, "In"), (.radialOut, "Out")],
                current: $mode,
                enabled: supportsRadial)

            ColorCurveModeRow(
                label: "Rotation",
                systemImage: "arrow.clockwise",
                modes: [(.cw, "CW"), (.ccw, "CCW")],
                current: $mode,
                enabled: supportsRadial)
        }
    }
}

private struct ColorCurveModeRow: View {
    let label: String
    let systemImage: String
    let modes: [(XLColorCurveMode, String)]
    @Binding var current: XLColorCurveMode
    let enabled: Bool

    var body: some View {
        HStack(spacing: 8) {
            Label(label, systemImage: systemImage)
                .font(.caption)
                .foregroundStyle(enabled ? .primary : .tertiary)
                .frame(width: 150, alignment: .leading)
            HStack(spacing: 4) {
                ForEach(modes, id: \.0) { m in
                    let isActive = current == m.0
                    Button {
                        current = m.0
                    } label: {
                        Text(m.1)
                            .font(.caption2)
                            .padding(.horizontal, 8)
                            .padding(.vertical, 4)
                            .background(
                                RoundedRectangle(cornerRadius: 4)
                                    .fill(isActive
                                          ? Color.accentColor
                                          : Color.secondary.opacity(0.12))
                            )
                            .foregroundStyle(isActive ? .white : .primary)
                    }
                    .buttonStyle(.plain)
                    .disabled(!enabled)
                    .opacity(enabled ? 1.0 : 0.4)
                }
            }
            Spacer()
        }
    }
}
