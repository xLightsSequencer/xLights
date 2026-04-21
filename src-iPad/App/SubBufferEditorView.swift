import SwiftUI

// Interactive sub-buffer rectangle editor. The sub-buffer is the
// user-selected slice of the full effect buffer that the effect
// actually renders into — e.g. "render Galaxy into just the top-left
// quadrant of the model, leave the rest untouched". Desktop uses a
// `SubBufferPanel` canvas with draggable corner / edge handles plus
// right-click preset menu; this is the iPad equivalent.
//
// Storage (`B_CUSTOM_SubBuffer`): six fields separated by a literal
// `x` delimiter — `x1xy1xx2xy2xxcxyc`, each a `%.2f` percentage
// 0..100. Each slot can alternately be a full `ValueCurve` serialized
// string (`Active=TRUE|Id=...|...`), which the renderer samples over
// the effect's time range so the sub-buffer can animate. Default /
// empty = "full buffer, no offset" and serializes as "".
//
// See `SubBufferPanel::SetValue/GetValue` in src-ui-wx and
// `ComputeSubBuffer` in `src-core/render/PixelBuffer.cpp:1944`.
//
// iPad scope (this pass):
//   - Edit the four corners (x1 / y1 / x2 / y2) as plain floats via a
//     touch canvas + editable number fields.
//   - Preserve the xc / yc center offset slots AND any slot that
//     already contains a ValueCurve string — those round-trip
//     untouched. A slot occupied by a VC is shown as "(curve)" in the
//     corresponding number field and the canvas ignores its axis.
//   - Preset buttons (Full / Halves / Thirds / Quarters) match the
//     desktop right-click menu.

struct SubBufferEditorView: View {
    @Environment(SequencerViewModel.self) var viewModel

    private let key = "B_CUSTOM_SubBuffer"

    @State private var state = SubBufferState()

    /// Tracks drag targets so the canvas knows which handle (or the
    /// whole rect) to move on `DragGesture.onChanged`.
    @State private var draggingHandle: Handle? = nil
    @State private var dragStart = SubBufferState()

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            Text("Sub-Buffer")
                .font(.caption)
                .fontWeight(.medium)
                .foregroundStyle(.secondary)

            canvas
                .frame(height: 160)

            valueRows

            presetRow
        }
        .padding(.vertical, 2)
        .onAppear { loadFromSettings() }
        .onChange(of: storedString) { _, _ in
            if draggingHandle == nil {
                loadFromSettings()
            }
        }
    }

    // MARK: - Canvas

    @ViewBuilder
    private var canvas: some View {
        GeometryReader { geo in
            let w = geo.size.width
            let h = geo.size.height
            let pad: CGFloat = 10  // inset so corner handles aren't clipped
            let box = CGRect(x: pad, y: pad,
                             width: max(w - pad * 2, 1),
                             height: max(h - pad * 2, 1))

            let rectInView = rectInCanvas(box: box)

            ZStack {
                // Background (full buffer outline)
                RoundedRectangle(cornerRadius: 4)
                    .stroke(Color.secondary.opacity(0.35),
                            style: StrokeStyle(lineWidth: 1, dash: [3, 3]))
                    .background(
                        RoundedRectangle(cornerRadius: 4)
                            .fill(Color.secondary.opacity(0.08))
                    )
                    .frame(width: box.width, height: box.height)
                    .position(x: box.midX, y: box.midY)

                // Selected sub-buffer
                Rectangle()
                    .fill(Color.accentColor.opacity(0.18))
                    .overlay(
                        Rectangle()
                            .stroke(Color.accentColor, lineWidth: 1.5)
                    )
                    .frame(width: rectInView.width,
                           height: rectInView.height)
                    .position(x: rectInView.midX, y: rectInView.midY)

                // Corner handles
                ForEach(Handle.corners, id: \.self) { (h: Handle) in
                    let p = handlePoint(h, in: box)
                    Circle()
                        .fill(Color.accentColor)
                        .overlay(Circle().stroke(Color.white, lineWidth: 1.5))
                        .frame(width: 18, height: 18)
                        .position(p)
                }
            }
            .contentShape(Rectangle())
            .gesture(dragGesture(box: box))
        }
    }

    private func dragGesture(box: CGRect) -> some Gesture {
        DragGesture(minimumDistance: 0)
            .onChanged { g in
                if draggingHandle == nil {
                    draggingHandle = pickHandle(at: g.startLocation, box: box)
                    dragStart = state
                }
                guard let h = draggingHandle else { return }
                apply(handle: h, to: g.location, from: dragStart, box: box)
            }
            .onEnded { _ in
                draggingHandle = nil
                commit()
            }
    }

    /// Decide which handle the user grabbed at `point`. Corner handles
    /// win if the touch landed within ~24pt; otherwise if the touch is
    /// inside the current rect we grab the whole rect for panning;
    /// otherwise nil = no drag.
    private func pickHandle(at point: CGPoint, box: CGRect) -> Handle? {
        for h in Handle.corners {
            let p = handlePoint(h, in: box)
            let d = hypot(p.x - point.x, p.y - point.y)
            if d <= 26 { return h }
        }
        let rect = rectInCanvas(box: box)
        if rect.contains(point) { return .whole }
        return nil
    }

    /// Where the handle `h` sits in canvas coords, given the full
    /// bounding box. Y is flipped — the sub-buffer's y=0 is at the
    /// BOTTOM of the canvas, matching desktop's convention.
    private func handlePoint(_ h: Handle, in box: CGRect) -> CGPoint {
        let x1 = state.x1.isValue ? state.x1.value : 0
        let y1 = state.y1.isValue ? state.y1.value : 0
        let x2 = state.x2.isValue ? state.x2.value : 100
        let y2 = state.y2.isValue ? state.y2.value : 100
        let lx = box.minX + CGFloat(x1 / 100.0) * box.width
        let rx = box.minX + CGFloat(x2 / 100.0) * box.width
        let ty = box.maxY - CGFloat(y2 / 100.0) * box.height
        let by = box.maxY - CGFloat(y1 / 100.0) * box.height
        switch h {
        case .bottomLeft:  return CGPoint(x: lx, y: by)
        case .bottomRight: return CGPoint(x: rx, y: by)
        case .topLeft:     return CGPoint(x: lx, y: ty)
        case .topRight:    return CGPoint(x: rx, y: ty)
        case .whole:       return CGPoint(x: (lx + rx) / 2, y: (ty + by) / 2)
        }
    }

    private func rectInCanvas(box: CGRect) -> CGRect {
        let x1 = state.x1.isValue ? state.x1.value : 0
        let y1 = state.y1.isValue ? state.y1.value : 0
        let x2 = state.x2.isValue ? state.x2.value : 100
        let y2 = state.y2.isValue ? state.y2.value : 100
        let lx = box.minX + CGFloat(x1 / 100.0) * box.width
        let rx = box.minX + CGFloat(x2 / 100.0) * box.width
        let ty = box.maxY - CGFloat(y2 / 100.0) * box.height
        let by = box.maxY - CGFloat(y1 / 100.0) * box.height
        return CGRect(x: lx, y: ty,
                      width: max(rx - lx, 1),
                      height: max(by - ty, 1))
    }

    private func apply(handle h: Handle,
                        to point: CGPoint,
                        from origin: SubBufferState,
                        box: CGRect) {
        // Canvas-px → 0..100, clamped. Y flipped.
        let xPct = clamp(Double((point.x - box.minX) / box.width) * 100)
        let yPct = clamp(Double((box.maxY - point.y) / box.height) * 100)

        switch h {
        case .bottomLeft:
            if origin.x1.isValue { state.x1 = .value(min(xPct, origin.x2.asValue(fallback: 100))) }
            if origin.y1.isValue { state.y1 = .value(min(yPct, origin.y2.asValue(fallback: 100))) }
        case .bottomRight:
            if origin.x2.isValue { state.x2 = .value(max(xPct, origin.x1.asValue(fallback: 0))) }
            if origin.y1.isValue { state.y1 = .value(min(yPct, origin.y2.asValue(fallback: 100))) }
        case .topLeft:
            if origin.x1.isValue { state.x1 = .value(min(xPct, origin.x2.asValue(fallback: 100))) }
            if origin.y2.isValue { state.y2 = .value(max(yPct, origin.y1.asValue(fallback: 0))) }
        case .topRight:
            if origin.x2.isValue { state.x2 = .value(max(xPct, origin.x1.asValue(fallback: 0))) }
            if origin.y2.isValue { state.y2 = .value(max(yPct, origin.y1.asValue(fallback: 0))) }
        case .whole:
            // Pan by the delta between the drag start and current.
            let dragStartXPct = clamp(Double((dragStartLocation(h, box: box).x - box.minX) / box.width) * 100)
            let dragStartYPct = clamp(Double((box.maxY - dragStartLocation(h, box: box).y) / box.height) * 100)
            let dx = xPct - dragStartXPct
            let dy = yPct - dragStartYPct
            let ox1 = origin.x1.asValue(fallback: 0)
            let oy1 = origin.y1.asValue(fallback: 0)
            let ox2 = origin.x2.asValue(fallback: 100)
            let oy2 = origin.y2.asValue(fallback: 100)
            let w = ox2 - ox1
            let hh = oy2 - oy1
            var nx1 = ox1 + dx
            var ny1 = oy1 + dy
            nx1 = max(0, min(nx1, 100 - w))
            ny1 = max(0, min(ny1, 100 - hh))
            if origin.x1.isValue { state.x1 = .value(nx1) }
            if origin.x2.isValue { state.x2 = .value(nx1 + w) }
            if origin.y1.isValue { state.y1 = .value(ny1) }
            if origin.y2.isValue { state.y2 = .value(ny1 + hh) }
        }
    }

    /// For whole-rect panning, we need the drag-start anchor in
    /// canvas space — which is the center of the rect when the drag
    /// started.
    private func dragStartLocation(_ h: Handle, box: CGRect) -> CGPoint {
        let x1 = dragStart.x1.asValue(fallback: 0)
        let y1 = dragStart.y1.asValue(fallback: 0)
        let x2 = dragStart.x2.asValue(fallback: 100)
        let y2 = dragStart.y2.asValue(fallback: 100)
        return CGPoint(x: box.minX + CGFloat((x1 + x2) / 200.0) * box.width,
                       y: box.maxY - CGFloat((y1 + y2) / 200.0) * box.height)
    }

    private func clamp(_ v: Double) -> Double { min(100, max(0, v)) }

    // MARK: - Number rows

    @ViewBuilder
    private var valueRows: some View {
        HStack(spacing: 6) {
            axisField("Left",   slot: .x1)
            axisField("Right",  slot: .x2)
        }
        HStack(spacing: 6) {
            axisField("Bottom", slot: .y1)
            axisField("Top",    slot: .y2)
        }
        // Desktop hides center offsets behind a right-click-menu
        // "advanced" branch. iPad shows them inline since the sidebar
        // has the vertical room — the fields are readily ignorable at
        // their zero default.
        HStack(spacing: 6) {
            axisField("X Centre", slot: .xc)
            axisField("Y Centre", slot: .yc)
        }
    }

    @ViewBuilder
    private func axisField(_ title: String, slot: Slot) -> some View {
        HStack(spacing: 4) {
            Text(title)
                .font(.caption2)
                .foregroundStyle(.secondary)
                .frame(width: 52, alignment: .leading)
            axisValue(slot: slot)
                .frame(maxWidth: .infinity, alignment: .trailing)
            subBufferSlotVCButton(for: slot)
        }
    }

    @ViewBuilder
    private func axisValue(slot: Slot) -> some View {
        let range = vcRange(for: slot)
        // Scaled x100 to reuse EditableNumberField's integer storage
        // model (divisor 100 displays 0.00..100.00 / -100.00..100.00).
        let minScaled = Int((range.min * 100).rounded())
        let maxScaled = Int((range.max * 100).rounded())
        switch value(for: slot) {
        case .value(let v):
            EditableNumberField(
                storedInt: Int((v * 100).rounded()),
                min: minScaled, max: maxScaled, divisor: 100,
                commit: { scaled in
                    set(slot: slot, to: Double(scaled) / 100.0)
                    commit()
                })
        case .curve:
            // Active curve — the renderer samples it over time, so
            // there's no single "value" to edit numerically. Tap the
            // VC button to the right to edit the curve; dimming the
            // label matches how other VC-active rows render.
            Text("(curve)")
                .font(.caption2)
                .foregroundStyle(.tertiary)
                .frame(maxWidth: .infinity, alignment: .trailing)
        }
    }

    // MARK: - Presets

    @ViewBuilder
    private var presetRow: some View {
        Menu {
            Button("Full")    { applyPreset(0,  0, 100, 100) }
            Section("Halves") {
                Button("Left Half")   { applyPreset(0,  0,  50, 100) }
                Button("Right Half")  { applyPreset(50, 0, 100, 100) }
                Button("Top Half")    { applyPreset(0, 50, 100, 100) }
                Button("Bottom Half") { applyPreset(0,  0, 100,  50) }
            }
            Section("Thirds") {
                Button("Left Third")    { applyPreset(0,     0,  33.33, 100) }
                Button("Middle Third")  { applyPreset(33.33, 0,  66.67, 100) }
                Button("Right Third")   { applyPreset(66.67, 0, 100,    100) }
            }
            Section("Quarters") {
                Button("Top-Left")     { applyPreset(0,  50,  50, 100) }
                Button("Top-Right")    { applyPreset(50, 50, 100, 100) }
                Button("Bottom-Left")  { applyPreset(0,   0,  50,  50) }
                Button("Bottom-Right") { applyPreset(50,  0, 100,  50) }
            }
        } label: {
            HStack {
                Label("Preset", systemImage: "square.grid.3x3.square")
                    .font(.caption)
                Spacer()
                Image(systemName: "chevron.down")
                    .font(.caption2)
                    .foregroundStyle(.tertiary)
            }
            .padding(.horizontal, 8)
            .padding(.vertical, 6)
            .background(
                RoundedRectangle(cornerRadius: 6)
                    .fill(Color.secondary.opacity(0.12))
            )
        }
    }

    private func applyPreset(_ x1: Double, _ y1: Double, _ x2: Double, _ y2: Double) {
        state.x1 = .value(x1)
        state.y1 = .value(y1)
        state.x2 = .value(x2)
        state.y2 = .value(y2)
        commit()
    }

    // MARK: - Settings I/O

    private var storedString: String {
        viewModel.settingValue(forKey: key, defaultValue: "")
    }

    private func loadFromSettings() {
        state = SubBufferState.parse(storedString)
    }

    private func commit() {
        viewModel.setSettingValue(state.serialise(),
                                   forKey: key,
                                   suppressIfDefault: "")
    }

    // MARK: - Slot helpers

    /// Every editable sub-buffer slot. `xc` / `yc` are the center
    /// offsets desktop exposes in the advanced context menu; iPad
    /// surfaces them inline. Adding them to the enum keeps the
    /// axisField helper generic.
    enum Slot: CaseIterable {
        case x1, y1, x2, y2, xc, yc
    }

    private func value(for slot: Slot) -> SubBufferState.SlotValue {
        switch slot {
        case .x1: return state.x1
        case .y1: return state.y1
        case .x2: return state.x2
        case .y2: return state.y2
        case .xc: return state.xc
        case .yc: return state.yc
        }
    }

    /// Write to `state` (which is `@State`, so assignments are allowed
    /// even from a non-mutating method). Corner slots clamp to the
    /// sibling so `x1 <= x2` / `y1 <= y2` always holds. Center
    /// offsets aren't range-constrained on desktop either — they
    /// shift the rect as a whole at render time via
    /// `PixelBuffer.cpp:1964-1981`.
    private func set(slot: Slot, to v: Double) {
        switch slot {
        case .x1: state.x1 = .value(min(v, state.x2.asValue(fallback: 100)))
        case .y1: state.y1 = .value(min(v, state.y2.asValue(fallback: 100)))
        case .x2: state.x2 = .value(max(v, state.x1.asValue(fallback: 0)))
        case .y2: state.y2 = .value(max(v, state.y1.asValue(fallback: 0)))
        case .xc: state.xc = .value(v)
        case .yc: state.yc = .value(v)
        }
    }

    /// Per-slot natural range / id — drives the VC editor's min/max
    /// and the stable VC `Id=` string desktop expects (so round-trips
    /// preserve the authoring application's canonical id).
    func vcRange(for slot: Slot) -> (id: String, min: Double, max: Double) {
        switch slot {
        case .x1: return ("SB_Left",   0,   100)
        case .x2: return ("SB_Right",  0,   100)
        case .y1: return ("SB_Bottom", 0,   100)
        case .y2: return ("SB_Top",    0,   100)
        case .xc: return ("SB_XC",  -100,  100)
        case .yc: return ("SB_YC",  -100,  100)
        }
    }

    /// Replace the slot's underlying SlotValue — used by the per-slot
    /// VC editor when the user flips active on/off or edits the curve.
    private func setSlot(_ slot: Slot, to new: SubBufferState.SlotValue) {
        switch slot {
        case .x1: state.x1 = new
        case .y1: state.y1 = new
        case .x2: state.x2 = new
        case .y2: state.y2 = new
        case .xc: state.xc = new
        case .yc: state.yc = new
        }
    }

    private enum Handle: Hashable {
        case bottomLeft, bottomRight, topLeft, topRight, whole
        static let corners: [Handle] = [.bottomLeft, .bottomRight, .topLeft, .topRight]
    }

    // MARK: - Per-slot VC button

    /// Small inline button next to each slot's value field. Shows
    /// active state (`chart.xyaxis.line` in accent colour) when the
    /// slot is a VC, muted otherwise. Opens the full VC editor sheet
    /// pre-loaded with the slot's current curve (or a freshly-
    /// defaulted curve if the slot currently holds a plain value).
    ///
    /// Writes route through `customPersist` on `EditableValueCurve`
    /// so the curve's serialised string replaces the slot inside
    /// the composite `B_CUSTOM_SubBuffer` string — instead of
    /// trying to write to a dedicated `<prefix>VALUECURVE_<id>` key
    /// that doesn't exist for sub-buffer slots.
    @ViewBuilder
    private func subBufferSlotVCButton(for slot: Slot) -> some View {
        SubBufferSlotVCButton(
            slot: slot,
            slotValue: value(for: slot),
            range: vcRange(for: slot),
            onCommit: { newValue in
                setSlot(slot, to: newValue)
                commit()
            }
        )
    }
}

/// Extracted so each slot has its own `@State` for the sheet presenter
/// and VC state. Reusing a single sheet flag across all six buttons
/// would make only one slot editable at a time.
private struct SubBufferSlotVCButton: View {
    let slot: SubBufferEditorView.Slot
    let slotValue: SubBufferState.SlotValue
    let range: (id: String, min: Double, max: Double)
    let onCommit: (SubBufferState.SlotValue) -> Void

    @State private var showEditor = false

    private var isActive: Bool {
        if case .curve(let s) = slotValue, s.hasPrefix("Active=TRUE") {
            return true
        }
        return false
    }

    /// Current serialised string to seed the editor with. When the
    /// slot holds a plain value, start with an inactive curve so the
    /// user has a clean canvas (the plain value remains the fallback
    /// when the curve's active toggle is off).
    private var seededString: String {
        if case .curve(let s) = slotValue { return s }
        return ""   // empty → XLValueCurve init defaults to inactive
    }

    /// Synthesised PropertyMetadata for the editor sheet. `valueCurve:
    /// true` so the sheet shows the active toggle and curve-type picker.
    /// Per-axis range matches `PixelBuffer.cpp:1987/2009` limits.
    private var property: PropertyMetadata? {
        PropertyMetadata.makeSynthetic(
            id: range.id,
            label: "Sub-Buffer \(slot.labelSuffix)",
            type: "float",
            controlType: "slider",
            defaultValue: 0,
            min: range.min,
            max: range.max,
            divisor: 1,
            valueCurve: true)
    }

    var body: some View {
        Button(action: { showEditor = true }) {
            Image(systemName: "chart.xyaxis.line")
                .font(.caption)
                .frame(width: 22, height: 18)
                .foregroundStyle(isActive ? .white : .secondary)
                .background(
                    RoundedRectangle(cornerRadius: 4)
                        .fill(isActive ? Color.accentColor
                                       : Color.secondary.opacity(0.12))
                )
                .overlay(
                    RoundedRectangle(cornerRadius: 4)
                        .stroke(Color.secondary.opacity(0.35), lineWidth: 0.5)
                )
        }
        .buttonStyle(.plain)
        .sheet(isPresented: $showEditor) {
            if let prop = property {
                ValueCurveEditorSheet(
                    property: prop,
                    prefix: "B_",
                    storedString: seededString,
                    customPersist: { serialised in
                        // Serialised comes from the editor on every
                        // mutation. `Active=FALSE|` means the user
                        // toggled the curve off — collapse back to a
                        // plain value so the slot round-trips as a
                        // number, not a dead-curve string.
                        if serialised.hasPrefix("Active=FALSE")
                            || serialised.isEmpty {
                            // Revert to whatever plain value was last
                            // visible, or the slot's natural default
                            // if there was no prior value.
                            let fallback = slotValue.asValue(
                                fallback: slot.defaultValue)
                            onCommit(.value(fallback))
                        } else {
                            onCommit(.curve(serialised))
                        }
                    })
            }
        }
    }
}

// Pretty labels + per-slot defaults used by the VC button sheet.
extension SubBufferEditorView.Slot {
    var labelSuffix: String {
        switch self {
        case .x1: return "Left"
        case .y1: return "Bottom"
        case .x2: return "Right"
        case .y2: return "Top"
        case .xc: return "X Centre"
        case .yc: return "Y Centre"
        }
    }

    var defaultValue: Double {
        switch self {
        case .x1, .y1, .xc, .yc: return 0
        case .x2, .y2:           return 100
        }
    }
}

// MARK: - Model

/// Six-slot sub-buffer value. Each slot is either a plain 0..100 float
/// or a preserved value-curve string (which the iPad doesn't yet let
/// the user edit in this control). Desktop's parse/serialise is in
/// `SubBufferPanel.cpp:81-199`.
struct SubBufferState: Equatable {
    enum SlotValue: Equatable {
        case value(Double)
        case curve(String)

        var isValue: Bool {
            if case .value = self { return true }
            return false
        }
        var value: Double {
            if case .value(let v) = self { return v }
            return 0
        }
        func asValue(fallback: Double) -> Double {
            if case .value(let v) = self { return v }
            return fallback
        }
    }

    var x1: SlotValue = .value(0)
    var y1: SlotValue = .value(0)
    var x2: SlotValue = .value(100)
    var y2: SlotValue = .value(100)
    var xc: SlotValue = .value(0)
    var yc: SlotValue = .value(0)

    static func parse(_ s: String) -> SubBufferState {
        var out = SubBufferState()
        if s.isEmpty { return out }
        // `Max` → `yyz` temporarily so a literal `Max` in a VC string
        // doesn't confuse the `x` splitter (same trick as desktop).
        let neutered = s.replacingOccurrences(of: "Max", with: "yyz")
        let parts = neutered.split(separator: "x",
                                     omittingEmptySubsequences: false)
                            .map(String.init)
        let defaults: [SlotValue] = [
            .value(0), .value(0), .value(100), .value(100),
            .value(0), .value(0),
        ]
        var slots = defaults
        for i in 0..<min(parts.count, 6) {
            let p = parts[i]
            if p.contains("Active=TRUE") {
                let restored = p.replacingOccurrences(of: "yyz", with: "Max")
                slots[i] = .curve(restored)
            } else if let v = Double(p) {
                slots[i] = .value(v)
            }
        }
        out.x1 = slots[0]; out.y1 = slots[1]
        out.x2 = slots[2]; out.y2 = slots[3]
        out.xc = slots[4]; out.yc = slots[5]
        // Swap if the user somehow got them reversed (desktop does
        // the same at end of SetValue).
        if case .value(let a) = out.x1, case .value(let b) = out.x2, a > b {
            out.x1 = .value(b); out.x2 = .value(a)
        }
        if case .value(let a) = out.y1, case .value(let b) = out.y2, a > b {
            out.y1 = .value(b); out.y2 = .value(a)
        }
        return out
    }

    func serialise() -> String {
        // Suppress to empty when everything is the out-of-the-box
        // default — matches desktop, keeps the settings map clean.
        if isDefault() { return "" }
        let parts: [String] = [
            serialiseSlot(x1),
            serialiseSlot(y1),
            serialiseSlot(x2),
            serialiseSlot(y2),
            serialiseSlot(xc),
            serialiseSlot(yc),
        ]
        return parts.joined(separator: "x")
    }

    private func isDefault() -> Bool {
        func eq(_ s: SlotValue, _ d: Double) -> Bool {
            if case .value(let v) = s { return abs(v - d) < 0.001 }
            return false
        }
        return eq(x1, 0)
            && eq(y1, 0)
            && eq(x2, 100)
            && eq(y2, 100)
            && eq(xc, 0)
            && eq(yc, 0)
    }

    private func serialiseSlot(_ s: SlotValue) -> String {
        switch s {
        case .value(let v): return String(format: "%.2f", v)
        case .curve(let str): return str
        }
    }
}
