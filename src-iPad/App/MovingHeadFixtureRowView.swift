import SwiftUI

// G3 — Moving Head fixture selection row. Desktop's
// `MovingHeadPanel` surfaces eight checkboxes (MH1-MH8) that pick
// which fixture slots this effect targets; iPad needs the same
// affordance but in a touch-friendly grid layout.
//
// A fixture is "active" iff its `E_TEXTCTRL_MH<n>_Settings` string
// is non-empty — desktop derives checkbox state the same way at
// open time (`MovingHeadPanel.cpp:1974-1985`). Toggling writes an
// initial command string (activate) or clears it (deactivate);
// the bridge then rewrites every active fixture's Pan / Tilt /
// Offsets / Groupings / Cycles from the current slider values so
// the `Heads:` list the renderer uses stays in sync with the new
// selection.
struct MovingHeadFixtureRowView: View {
    @Environment(SequencerViewModel.self) var viewModel

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            Text("Fixtures")
                .font(.caption)
                .foregroundStyle(.secondary)
            // Two rows of four so all eight fit in the 280pt
            // inspector without wrapping on smaller screens.
            HStack(spacing: 6) {
                ForEach(1...4, id: \.self) { i in
                    fixtureButton(i)
                }
            }
            HStack(spacing: 6) {
                ForEach(5...8, id: \.self) { i in
                    fixtureButton(i)
                }
            }
        }
        .padding(.vertical, 4)
    }

    @ViewBuilder
    private func fixtureButton(_ i: Int) -> some View {
        let active = isActive(i)
        Button {
            toggle(i)
        } label: {
            Text("\(i)")
                .font(.caption)
                .fontWeight(.semibold)
                .frame(maxWidth: .infinity, minHeight: 28)
                .foregroundStyle(active ? Color.white : Color.primary)
                .background(
                    RoundedRectangle(cornerRadius: 6)
                        .fill(active
                              ? Color.accentColor
                              : Color.secondary.opacity(0.15))
                )
        }
        .buttonStyle(.plain)
    }

    // MARK: - State

    /// True when fixture i's `E_TEXTCTRL_MH<i>_Settings` entry is
    /// non-empty. Read via the mask the bridge exposes —
    /// inspectorRevision ensures SwiftUI recomputes the button
    /// state after a slider edit triggers a sync write.
    private func isActive(_ i: Int) -> Bool {
        _ = viewModel.inspectorRevision
        guard let sel = viewModel.selectedEffect else { return false }
        let mask = Int(viewModel.document.movingHeadActiveFixtures(
            forRow: Int32(sel.rowIndex),
            at: Int32(sel.effectIndex)))
        return (mask & (1 << (i - 1))) != 0
    }

    private func toggle(_ i: Int) {
        guard let sel = viewModel.selectedEffect else { return }
        let nowActive = !isActive(i)
        _ = viewModel.document.setMovingHeadFixture(
            Int32(i),
            active: nowActive,
            forRow: Int32(sel.rowIndex),
            at: Int32(sel.effectIndex))
        // The bridge mutated the settings map directly (bypassing
        // `setEffectSettingValue` so we didn't register N undo
        // steps); refresh the Swift observable so the buttons and
        // any downstream readers pick up the new MH*_Settings
        // state, and kick a render so the scrub pane updates.
        viewModel.refreshSelectedEffectSettings()
        viewModel.inspectorRevision &+= 1
    }
}

/// Read-only banner above the Moving Head fixture row. Calls out
/// the parts of the desktop Effect Assist story that aren't yet
/// covered on iPad — currently just full waypoint path editing.
struct MovingHeadInfoRowView: View {
    var body: some View {
        HStack(alignment: .top, spacing: 6) {
            Image(systemName: "info.circle")
                .foregroundStyle(.tint)
                .font(.caption)
            Text("Full waypoint path drawing still uses desktop's Effect Assist panel. iPad supports fixture selection, Pan / Tilt / Offset / Groupings / Cycles, single-colour and dimmer-intensity authoring.")
                .font(.caption2)
                .foregroundStyle(.secondary)
        }
        .padding(.vertical, 4)
    }
}

// MARK: - G3+ — Colour, dimmer, path rows

/// Single-colour authoring for Moving Head effects. Writes a
/// `Color: H,S,V` triple (matches desktop's
/// `MHRgbPickerPanel::GetColour` format) into every active
/// fixture's `MH*_Settings` string. Multi-colour blends still
/// require desktop; for v1 the most common case (one colour for
/// the whole effect) is enough.
struct MovingHeadColorRowView: View {
    @Environment(SequencerViewModel.self) var viewModel

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            Text("Colour")
                .font(.caption)
                .foregroundStyle(.secondary)
            HStack {
                ColorPicker("Colour",
                             selection: Binding(
                                get: { currentColor },
                                set: { writeColor($0) }
                             ),
                             supportsOpacity: false)
                    .labelsHidden()
                Spacer()
                Button {
                    writeColor(nil)
                } label: {
                    Label("Clear", systemImage: "xmark.circle")
                        .labelStyle(.iconOnly)
                }
                .buttonStyle(.borderless)
                .disabled(rawColorString.isEmpty)
            }
            if !rawColorString.isEmpty {
                Text(rawColorString)
                    .font(.caption2)
                    .foregroundStyle(.secondary)
                    .lineLimit(1)
                    .truncationMode(.middle)
            }
        }
        .padding(.vertical, 4)
    }

    private var rawColorString: String {
        _ = viewModel.inspectorRevision
        guard let sel = viewModel.selectedEffect else { return "" }
        return viewModel.document.movingHeadCommand("Color",
                                                      forRow: Int32(sel.rowIndex),
                                                      atIndex: Int32(sel.effectIndex))
    }

    private var currentColor: Color {
        let parts = rawColorString.split(separator: ",")
        guard parts.count >= 3,
              let h = Double(parts[0]),
              let s = Double(parts[1]),
              let v = Double(parts[2]) else {
            return .red
        }
        return Color(hue: h, saturation: s, brightness: v)
    }

    private func writeColor(_ newColor: Color?) {
        guard let sel = viewModel.selectedEffect else { return }
        let value: String
        if let c = newColor {
            #if canImport(UIKit)
            let ui = UIColor(c)
            var h: CGFloat = 0, s: CGFloat = 0, b: CGFloat = 0, a: CGFloat = 0
            ui.getHue(&h, saturation: &s, brightness: &b, alpha: &a)
            value = String(format: "%f,%f,%f", h, s, b)
            #else
            value = ""
            #endif
        } else {
            value = ""   // empty value clears the command
        }
        _ = viewModel.document.setMovingHeadCommand(
            "Color", value: value,
            forRow: Int32(sel.rowIndex),
            atIndex: Int32(sel.effectIndex))
        viewModel.refreshSelectedEffectSettings()
        viewModel.inspectorRevision &+= 1
    }
}

/// Dimmer intensity authoring — a single 0..100% slider that
/// writes a flat dimmer ramp `Dimmer: 0,intensity,1,intensity` to
/// every active fixture. Mirrors desktop's
/// `MovingHeadDimmerPanel::GetDimmerCommands` waypoint format
/// (x, y pairs in 0..1) but constrains the iPad UI to the simple
/// "constant intensity" case. Curve / multi-segment authoring
/// stays on desktop.
struct MovingHeadDimmerRowView: View {
    @Environment(SequencerViewModel.self) var viewModel

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            Text("Dimmer Intensity")
                .font(.caption)
                .foregroundStyle(.secondary)
            HStack {
                Slider(value: Binding(
                    get: { intensity },
                    set: { writeIntensity($0) }
                ), in: 0...1)
                Text("\(Int(intensity * 100))%")
                    .font(.caption.monospacedDigit())
                    .foregroundStyle(.secondary)
                    .frame(width: 44, alignment: .trailing)
                Button {
                    writeRaw("")
                } label: {
                    Label("Clear", systemImage: "xmark.circle")
                        .labelStyle(.iconOnly)
                }
                .buttonStyle(.borderless)
                .disabled(rawDimmerString.isEmpty)
            }
            if isMultiSegment {
                Text("Multi-segment dimmer envelope authored on desktop. Editing here will replace it with a constant intensity.")
                    .font(.caption2)
                    .foregroundStyle(.orange)
            }
        }
        .padding(.vertical, 4)
    }

    private var rawDimmerString: String {
        _ = viewModel.inspectorRevision
        guard let sel = viewModel.selectedEffect else { return "" }
        return viewModel.document.movingHeadCommand("Dimmer",
                                                      forRow: Int32(sel.rowIndex),
                                                      atIndex: Int32(sel.effectIndex))
    }

    /// Desktop dimmer ramps are x,y pairs over 0..1. We surface the
    /// y of the *last* waypoint as "current intensity" — for a
    /// constant-on ramp `0,1,1,1` that's 1.0; for `0,0.5,1,0.5`
    /// it's 0.5; for an envelope it's the end value.
    private var intensity: Double {
        let s = rawDimmerString
        if s.isEmpty { return 1.0 }
        let parts = s.split(separator: ",")
        if parts.count < 2 { return 1.0 }
        return Double(parts[parts.count - 1]) ?? 1.0
    }

    private var isMultiSegment: Bool {
        let s = rawDimmerString
        if s.isEmpty { return false }
        let parts = s.split(separator: ",")
        // 2 pairs (4 values) is the canonical flat ramp; >2 pairs
        // means desktop authored a curve.
        return parts.count > 4
    }

    private func writeIntensity(_ y: Double) {
        let clamped = max(0.0, min(1.0, y))
        // Flat ramp from t=0 to t=1 at the chosen intensity.
        writeRaw(String(format: "0,%f,1,%f", clamped, clamped))
    }

    private func writeRaw(_ value: String) {
        guard let sel = viewModel.selectedEffect else { return }
        _ = viewModel.document.setMovingHeadCommand(
            "Dimmer", value: value,
            forRow: Int32(sel.rowIndex),
            atIndex: Int32(sel.effectIndex))
        viewModel.refreshSelectedEffectSettings()
        viewModel.inspectorRevision &+= 1
    }
}

/// Path read + clear. Full waypoint authoring still requires
/// desktop; iPad surfaces the current Path: command (so users
/// know whether one is set) and exposes a Clear action so a
/// stale path doesn't fight a fresh Pan / Tilt setup.
struct MovingHeadPathRowView: View {
    @Environment(SequencerViewModel.self) var viewModel

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            Text("Path")
                .font(.caption)
                .foregroundStyle(.secondary)
            if rawPath.isEmpty {
                Text("No path set. Use desktop's Effect Assist to draw one.")
                    .font(.caption)
                    .foregroundStyle(.secondary)
            } else {
                HStack(alignment: .top) {
                    Text(rawPath)
                        .font(.caption2.monospaced())
                        .foregroundStyle(.secondary)
                        .lineLimit(3)
                        .truncationMode(.middle)
                    Spacer()
                    Button {
                        clearPath()
                    } label: {
                        Label("Clear", systemImage: "xmark.circle")
                            .labelStyle(.iconOnly)
                    }
                    .buttonStyle(.borderless)
                }
            }
        }
        .padding(.vertical, 4)
    }

    private var rawPath: String {
        _ = viewModel.inspectorRevision
        guard let sel = viewModel.selectedEffect else { return "" }
        return viewModel.document.movingHeadCommand("Path",
                                                      forRow: Int32(sel.rowIndex),
                                                      atIndex: Int32(sel.effectIndex))
    }

    private func clearPath() {
        guard let sel = viewModel.selectedEffect else { return }
        _ = viewModel.document.setMovingHeadCommand(
            "Path", value: "",
            forRow: Int32(sel.rowIndex),
            atIndex: Int32(sel.effectIndex))
        viewModel.refreshSelectedEffectSettings()
        viewModel.inspectorRevision &+= 1
    }
}
