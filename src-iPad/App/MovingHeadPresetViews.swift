import SwiftUI

// Dimmer quick-set presets for the Moving Head effect. Each writes a
// flat / ramped `Dimmer:` command (x,y pairs over 0..1, matching
// desktop's `MovingHeadDimmerPanel` waypoint format) to every active
// fixture. Desktop ships dimmer presets as user-saved `.xmh` files;
// these built-ins cover the common envelopes (the values match the
// On / Off ramps desktop seeds and the obvious fade / pulse shapes).
struct MovingHeadDimmerPresetStrip: View {
    @Environment(SequencerViewModel.self) var viewModel

    private struct Preset: Identifiable {
        let id = UUID()
        let name: String
        let icon: String
        let dimmer: String
    }

    private static let presets: [Preset] = [
        Preset(name: "On", icon: "sun.max", dimmer: "0,1,1,1"),
        Preset(name: "Off", icon: "moon", dimmer: "0,0,1,0"),
        Preset(name: "Fade In", icon: "arrow.up.right", dimmer: "0,0,1,1"),
        Preset(name: "Fade Out", icon: "arrow.down.right", dimmer: "0,1,1,0"),
        Preset(name: "Pulse", icon: "waveform.path", dimmer: "0,0,0.5,1,1,0"),
    ]

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            Text("Dimmer Presets")
                .font(.caption2)
                .foregroundStyle(.secondary)
            ScrollView(.horizontal, showsIndicators: false) {
                HStack(spacing: 8) {
                    ForEach(Self.presets) { preset in
                        Button {
                            apply(preset.dimmer)
                        } label: {
                            VStack(spacing: 2) {
                                Image(systemName: preset.icon)
                                    .font(.caption)
                                Text(preset.name)
                                    .font(.caption2)
                            }
                            .frame(minWidth: 56)
                            .padding(.vertical, 4)
                        }
                        .buttonStyle(.bordered)
                        .controlSize(.small)
                    }
                }
            }
        }
    }

    private func apply(_ dimmer: String) {
        guard let sel = viewModel.selectedEffect else { return }
        _ = viewModel.document.setMovingHeadCommand(
            "Dimmer", value: dimmer,
            forRow: Int32(sel.rowIndex),
            atIndex: Int32(sel.effectIndex))
        viewModel.refreshSelectedEffectSettings()
        viewModel.inspectorRevision &+= 1
    }
}

// Colour-wheel picker for colour-wheel Moving Head fixtures. Only
// shown when the effect's target model is a colour-wheel type (the
// bridge returns nil otherwise, and the generic `MovingHeadColorRowView`
// covers RGB / CMYW fixtures). Picking a slot writes a single-colour
// `Wheel:` command (`hue,sat,val` in 0..1) — desktop's
// `MHColorWheelPanel::GetColour` format. Multi-colour wheel sequences
// (more than one slot, which animate across the effect) still require
// the desktop double-selector canvas.
struct MovingHeadColorWheelRowView: View {
    @Environment(SequencerViewModel.self) var viewModel

    private struct Slot: Identifiable {
        let id = UUID()
        let hex: String
        let hue: Double
        let sat: Double
        let val: Double
        var command: String { "\(hue),\(sat),\(val)" }
    }

    private var slots: [Slot] {
        _ = viewModel.inspectorRevision
        guard let sel = viewModel.selectedEffect else { return [] }
        guard let raw = viewModel.document.movingHeadWheelColors(
            forRow: Int32(sel.rowIndex),
            atIndex: Int32(sel.effectIndex)) else { return [] }
        return raw.compactMap { dict in
            guard let hex = dict["hex"] as? String,
                  let hue = (dict["hue"] as? NSNumber)?.doubleValue,
                  let sat = (dict["sat"] as? NSNumber)?.doubleValue,
                  let val = (dict["val"] as? NSNumber)?.doubleValue else { return nil }
            return Slot(hex: hex, hue: hue, sat: sat, val: val)
        }
    }

    private var currentCommand: String {
        _ = viewModel.inspectorRevision
        guard let sel = viewModel.selectedEffect else { return "" }
        return viewModel.document.movingHeadCommand("Wheel",
                                                    forRow: Int32(sel.rowIndex),
                                                    atIndex: Int32(sel.effectIndex))
    }

    var body: some View {
        let wheelSlots = slots
        if !wheelSlots.isEmpty {
            VStack(alignment: .leading, spacing: 6) {
                Text("Colour Wheel")
                    .font(.caption)
                    .foregroundStyle(.secondary)
                ScrollView(.horizontal, showsIndicators: false) {
                    HStack(spacing: 8) {
                        ForEach(wheelSlots) { slot in
                            Button {
                                apply(slot.command)
                            } label: {
                                Circle()
                                    .fill(colorFromHex(slot.hex))
                                    .frame(width: 30, height: 30)
                                    .overlay(
                                        Circle().stroke(
                                            currentCommand == slot.command
                                                ? Color.accentColor : Color.secondary.opacity(0.4),
                                            lineWidth: currentCommand == slot.command ? 3 : 1))
                            }
                            .buttonStyle(.plain)
                        }
                        Button {
                            apply("")
                        } label: {
                            Image(systemName: "xmark.circle")
                                .font(.title3)
                                .foregroundStyle(.secondary)
                        }
                        .buttonStyle(.plain)
                        .disabled(currentCommand.isEmpty)
                    }
                }
            }
            .padding(.vertical, 4)
        }
    }

    private func apply(_ command: String) {
        guard let sel = viewModel.selectedEffect else { return }
        // A wheel pick and a generic colour are mutually exclusive —
        // clear any stray Color command so the two don't fight.
        if !command.isEmpty {
            _ = viewModel.document.setMovingHeadCommand(
                "Color", value: "",
                forRow: Int32(sel.rowIndex),
                atIndex: Int32(sel.effectIndex))
        }
        _ = viewModel.document.setMovingHeadCommand(
            "Wheel", value: command,
            forRow: Int32(sel.rowIndex),
            atIndex: Int32(sel.effectIndex))
        viewModel.refreshSelectedEffectSettings()
        viewModel.inspectorRevision &+= 1
    }

    private func colorFromHex(_ hex: String) -> Color {
        var s = hex
        if s.hasPrefix("#") { s.removeFirst() }
        guard let v = UInt32(s, radix: 16), s.count == 6 else { return .gray }
        return Color(red: Double((v >> 16) & 0xFF) / 255.0,
                     green: Double((v >> 8) & 0xFF) / 255.0,
                     blue: Double(v & 0xFF) / 255.0)
    }
}
