import SwiftUI

// Custom rows for the shared Color panel (`shared/Color.json`,
// `C_` prefix). Mirrors the compound widgets the desktop `ColorPanel`
// hand-builds alongside the palette grid.
//
// Covered:
//   ChromaKeyRow — enable + ChromaSensitivity slider + ChromaColour picker
//   SparklesRow  — SparkleFrequency slider + Music toggle + SparklesColour

// MARK: - Chroma Key

/// `ChromaKeyRow` — disables/enables chroma-key colour removal on the
/// layer and picks the target colour + sensitivity. When the enable
/// toggle is off the slider and colour picker are dimmed to match
/// desktop's enable-state behaviour. Backing keys (read by
/// PixelBuffer::PrepareAllLayers at `PixelBuffer.cpp:2111-2115`):
///   `C_CHECKBOX_Chroma`
///   `C_SLIDER_ChromaSensitivity`  (1..255, default 1 — matches
///     `ColorPanel.cpp:374` and the raw distance threshold the
///     `PixelBuffer.cpp:1138` check compares against: larger values
///     trigger on a wider neighbourhood around the chroma colour)
///   `C_COLOURPICKERCTRL_ChromaColour`  (#RRGGBB)
struct ChromaKeyRowView: View {
    @Environment(SequencerViewModel.self) var viewModel

    private let enableKey = "C_CHECKBOX_Chroma"
    private let sensKey   = "C_SLIDER_ChromaSensitivity"
    private let colorKey  = "C_COLOURPICKERCTRL_ChromaColour"

    private var enabled: Bool {
        let v = viewModel.settingValue(forKey: enableKey, defaultValue: "0")
        return v == "1" || v.lowercased() == "true"
    }

    /// 1..255. The raw slider units are the distance threshold fed
    /// into `ColourDistance` at render time — no further scaling.
    private var sensitivity: Int {
        Int(viewModel.settingValue(forKey: sensKey, defaultValue: "1")) ?? 1
    }

    private var colorHex: String {
        viewModel.settingValue(forKey: colorKey, defaultValue: "#000000")
    }

    var body: some View {
        let enableBinding = Binding<Bool>(
            get: { enabled },
            set: { viewModel.setSettingValue($0 ? "1" : "0",
                                              forKey: enableKey,
                                              suppressIfDefault: "0") }
        )
        let sensBinding = Binding<Double>(
            get: { Double(sensitivity) },
            set: { viewModel.setSettingValue(String(Int($0)),
                                              forKey: sensKey,
                                              suppressIfDefault: "1") }
        )
        let colorBinding = Binding<Color>(
            get: { colorFromHex(colorHex) ?? .black },
            set: { newColor in
                if let hex = hexFromColor(newColor) {
                    viewModel.setSettingValue(hex, forKey: colorKey,
                                               suppressIfDefault: "#000000")
                }
            }
        )

        VStack(alignment: .leading, spacing: 2) {
            HStack {
                Toggle(isOn: enableBinding) {
                    Text("Chroma Key").font(.caption)
                }
                .toggleStyle(.switch)
                .controlSize(.small)
                Spacer()
                ColorPicker("", selection: colorBinding,
                             supportsOpacity: false)
                    .labelsHidden()
                    .disabled(!enabled)
                    .opacity(enabled ? 1.0 : 0.4)
            }
            HStack {
                Text("Sensitivity")
                    .font(.caption2)
                    .foregroundStyle(.secondary)
                Spacer()
                Text("\(sensitivity)")
                    .monospacedDigit()
                    .font(.caption2)
                    .foregroundStyle(.secondary)
            }
            Slider(value: sensBinding, in: 1...255, step: 1)
                .disabled(!enabled)
                .opacity(enabled ? 1.0 : 0.4)
        }
        .padding(.vertical, 2)
    }
}

// MARK: - Sparkles

/// `SparklesRow` — adds sparkle pixels over the layer output. Frequency
/// counts sparkles per 200 buffer pixels (roughly); the Music toggle
/// modulates the count by the audio volume at each frame. Backing
/// (read by PixelBuffer at `PixelBuffer.cpp:2107-2116`):
///   `C_SLIDER_SparkleFrequency`  (0..200)
///   `C_CHECKBOX_MusicSparkles`
///   `C_COLOURPICKERCTRL_SparklesColour`  (#RRGGBB, default #FFFFFF)
///
/// Sparkles also supports a value curve (`C_VALUECURVE_SparkleFrequency`).
/// The desktop puts the VC button inline next to the frequency slider;
/// iPad matches that by synthesising a PropertyMetadata for the
/// frequency slider and reusing the shared `ValueCurveButton`. When the
/// curve is active the base slider dims + disables — same "curve takes
/// over" behaviour every other VC-capable slider in the inspector uses.
struct SparklesRowView: View {
    @Environment(SequencerViewModel.self) var viewModel

    private let freqKey = "C_SLIDER_SparkleFrequency"
    private let musicKey = "C_CHECKBOX_MusicSparkles"
    private let colorKey = "C_COLOURPICKERCTRL_SparklesColour"
    private let vcKey = "C_VALUECURVE_SparkleFrequency"

    /// Synthesised metadata driving the VC button's editor sheet.
    /// `prefix: "C_"` + `id: "SparkleFrequency"` → storage key
    /// `C_VALUECURVE_SparkleFrequency`, matching `PixelBuffer.cpp:1880`.
    private var frequencyVCProp: PropertyMetadata? {
        PropertyMetadata.makeSynthetic(
            id: "SparkleFrequency",
            label: "Sparkle Frequency",
            type: "int",
            controlType: "slider",
            defaultValue: 0,
            min: 0,
            max: 200,
            divisor: 1,
            valueCurve: true)
    }

    private var frequency: Int {
        Int(viewModel.settingValue(forKey: freqKey, defaultValue: "0")) ?? 0
    }
    private var musicOn: Bool {
        let v = viewModel.settingValue(forKey: musicKey, defaultValue: "0")
        return v == "1" || v.lowercased() == "true"
    }
    private var colorHex: String {
        viewModel.settingValue(forKey: colorKey, defaultValue: "#FFFFFF")
    }
    private var vcActive: Bool {
        viewModel.settingValue(forKey: vcKey, defaultValue: "")
            .hasPrefix("Active=TRUE")
    }

    var body: some View {
        let freqBinding = Binding<Double>(
            get: { Double(frequency) },
            set: { viewModel.setSettingValue(String(Int($0)),
                                              forKey: freqKey,
                                              suppressIfDefault: "0") }
        )
        let musicBinding = Binding<Bool>(
            get: { musicOn },
            set: { viewModel.setSettingValue($0 ? "1" : "0",
                                              forKey: musicKey,
                                              suppressIfDefault: "0") }
        )
        let colorBinding = Binding<Color>(
            get: { colorFromHex(colorHex) ?? .white },
            set: { newColor in
                if let hex = hexFromColor(newColor) {
                    viewModel.setSettingValue(hex, forKey: colorKey,
                                               suppressIfDefault: "#FFFFFF")
                }
            }
        )

        VStack(alignment: .leading, spacing: 2) {
            HStack {
                Text("Sparkles").font(.caption)
                Spacer()
                Text("\(frequency)")
                    .monospacedDigit()
                    .font(.caption2)
                    .foregroundStyle(.secondary)
                if let vcProp = frequencyVCProp {
                    ValueCurveButton(property: vcProp, prefix: "C_")
                }
                ColorPicker("", selection: colorBinding,
                             supportsOpacity: false)
                    .labelsHidden()
            }
            Slider(value: freqBinding, in: 0...200, step: 1)
                .opacity(vcActive ? 0.4 : 1.0)
                .disabled(vcActive)
            Toggle(isOn: musicBinding) {
                Text("Reflect music").font(.caption2)
            }
            .toggleStyle(.switch)
            .controlSize(.small)
        }
        .padding(.vertical, 2)
    }
}

// MARK: - Shared colour helpers

/// Parse a `#RRGGBB` hex string into a SwiftUI Color. Returns nil on
/// malformed input so the caller can fall back to a sentinel.
///
/// Constructs the Color in sRGB explicitly — `Color(red:green:blue:)` uses
/// the device color space, which on Display-P3 iPads silently drifts the
/// hex the user typed. Pinning to `.sRGB` keeps round-trip exact.
func colorFromHex(_ hex: String) -> Color? {
    var s = hex.trimmingCharacters(in: .whitespaces)
    if s.hasPrefix("#") { s.removeFirst() }
    guard s.count == 6 || s.count == 8, let val = UInt64(s, radix: 16) else {
        return nil
    }
    let r = Double((val >> 16) & 0xFF) / 255.0
    let g = Double((val >> 8) & 0xFF) / 255.0
    let b = Double(val & 0xFF) / 255.0
    return Color(.sRGB, red: r, green: g, blue: b, opacity: 1)
}

/// Serialise a SwiftUI Color as a `#RRGGBB` string matching desktop's
/// wxColour format. Drops alpha — the desktop panel widgets don't
/// store alpha for chroma / sparkles / palette colours either.
///
/// On wide-gamut iPads the SwiftUI ColorPicker may hand back a Color in
/// Display P3. UIColor.getRed then returns extended-sRGB components that
/// can fall outside [0,1] for out-of-gamut colors, producing garbage hex.
/// Route through CGColor/sRGB so the serialized value matches what the
/// user sees and round-trips cleanly back through colorFromHex.
func hexFromColor(_ color: Color) -> String? {
    #if canImport(UIKit)
    guard let srgb = CGColorSpace(name: CGColorSpace.sRGB),
          let srgbCG = UIColor(color).cgColor.converted(to: srgb, intent: .defaultIntent, options: nil),
          let c = srgbCG.components, c.count >= 3 else { return nil }
    let ri = Int((max(0, min(1, c[0])) * 255).rounded())
    let gi = Int((max(0, min(1, c[1])) * 255).rounded())
    let bi = Int((max(0, min(1, c[2])) * 255).rounded())
    return String(format: "#%02X%02X%02X", ri, gi, bi)
    #else
    return nil
    #endif
}
