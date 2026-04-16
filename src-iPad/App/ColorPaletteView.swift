import SwiftUI

// 8-slot color palette picker used by the Color shared panel. Each slot has:
//   - An enable toggle (C_CHECKBOX_Palette1..8)
//   - A color picker (C_BUTTON_Palette1..8, stored as "#RRGGBB")
// Disabled slots are ignored by the renderer but their color is preserved.
//
// ValueCurve/ColorCurve blobs in a palette slot are not supported yet —
// slots that contain a ColorCurve show gray and a "(curve)" hint and are
// read-only.
struct ColorPaletteView: View {
    @Environment(SequencerViewModel.self) var viewModel

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            Text("Palette")
                .font(.caption)
                .fontWeight(.medium)
                .foregroundStyle(.secondary)

            VStack(spacing: 4) {
                ForEach(1...8, id: \.self) { slot in
                    paletteRow(slot: slot)
                }
            }
        }
        .padding(.vertical, 4)
    }

    private func paletteRow(slot: Int) -> some View {
        let checkboxKey = "C_CHECKBOX_Palette\(slot)"
        let buttonKey = "C_BUTTON_Palette\(slot)"

        // Default palette colors — match desktop's first-run palette.
        let defaultHex = ["#FF0000", "#00FF00", "#0000FF", "#FFFF00",
                          "#FFFFFF", "#000000", "#FFA500", "#800080"][slot - 1]

        let enabledBinding = Binding<Bool>(
            get: {
                let v = viewModel.settingValue(forKey: checkboxKey, defaultValue: slot == 1 ? "1" : "0")
                return v == "1" || v.lowercased() == "true"
            },
            set: { viewModel.setSettingValue($0 ? "1" : "0", forKey: checkboxKey) }
        )

        let rawHex = viewModel.settingValue(forKey: buttonKey, defaultValue: defaultHex)
        let isColorCurve = rawHex.hasPrefix("Active=")  // ColorCurve serialized form starts with "Active=..."

        let colorBinding = Binding<Color>(
            get: {
                if isColorCurve { return .gray }
                return colorFromHex(rawHex) ?? .black
            },
            set: { newColor in
                if let hex = hexFromColor(newColor) {
                    viewModel.setSettingValue(hex, forKey: buttonKey)
                }
            }
        )

        return HStack(spacing: 8) {
            Toggle("", isOn: enabledBinding)
                .labelsHidden()
                .toggleStyle(.switch)
                .controlSize(.small)

            Text("\(slot)")
                .font(.caption2)
                .foregroundStyle(.secondary)
                .frame(width: 12)

            if isColorCurve {
                RoundedRectangle(cornerRadius: 4)
                    .fill(Color.gray)
                    .frame(height: 24)
                    .overlay(
                        Text("(curve)")
                            .font(.caption2)
                            .foregroundStyle(.white)
                    )
            } else {
                ColorPicker("", selection: colorBinding, supportsOpacity: false)
                    .labelsHidden()
                    .frame(maxWidth: .infinity, alignment: .leading)
            }
        }
    }

    private func colorFromHex(_ hex: String) -> Color? {
        var s = hex.trimmingCharacters(in: .whitespaces)
        if s.hasPrefix("#") { s.removeFirst() }
        guard s.count == 6 || s.count == 8, let val = UInt64(s, radix: 16) else { return nil }
        let r, g, b: Double
        if s.count == 6 {
            r = Double((val >> 16) & 0xFF) / 255.0
            g = Double((val >> 8) & 0xFF) / 255.0
            b = Double(val & 0xFF) / 255.0
        } else {
            r = Double((val >> 16) & 0xFF) / 255.0
            g = Double((val >> 8) & 0xFF) / 255.0
            b = Double(val & 0xFF) / 255.0
        }
        return Color(red: r, green: g, blue: b)
    }

    private func hexFromColor(_ color: Color) -> String? {
        #if canImport(UIKit)
        let ui = UIColor(color)
        var r: CGFloat = 0, g: CGFloat = 0, b: CGFloat = 0, a: CGFloat = 0
        guard ui.getRed(&r, green: &g, blue: &b, alpha: &a) else { return nil }
        let ri = Int((r * 255).rounded())
        let gi = Int((g * 255).rounded())
        let bi = Int((b * 255).rounded())
        return String(format: "#%02X%02X%02X", ri, gi, bi)
        #else
        return nil
        #endif
    }
}
