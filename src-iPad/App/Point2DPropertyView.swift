import SwiftUI

// Compound two-slider view for JSON `controlType: "point2d"` (shader
// dynamic uniform params, etc.). Matches desktop's builder in
// `JsonEffectPanel.cpp:1427-1601` — one slider row per axis, each
// keyed `E_<settingPrefix>_<id>X` / `<id>Y`, optional value-curve
// button per axis. Per-axis bounds fall back to single-valued
// `min`/`max`/`default` when `minX` etc. are absent.
//
// Rendering is delegated to `EffectPropertyView` via two synthesised
// `PropertyMetadata` siblings (one per axis) so every slider feature —
// editable value field, VC button, VC-active dimming — comes "free"
// without reimplementation.
struct Point2DPropertyView: View {
    let property: PropertyMetadata
    let metadataPrefix: String

    private var divisor: Int { property.effectiveDivisor }

    var body: some View {
        VStack(alignment: .leading, spacing: 2) {
            if !property.label.isEmpty {
                Text(property.label)
                    .font(.caption)
                    .fontWeight(.medium)
                    .foregroundStyle(.secondary)
            }
            axisView(axis: "X")
            axisView(axis: "Y")
        }
        .padding(.vertical, 2)
    }

    @ViewBuilder
    private func axisView(axis: String) -> some View {
        let bounds = axisBounds(axis)
        if let axProp = PropertyMetadata.makeSynthetic(
            id: property.id + axis,
            label: "\(property.label) \(axis)",
            type: "float",
            controlType: "slider",
            defaultValue: bounds.defaultValue,
            min: bounds.min,
            max: bounds.max,
            divisor: divisor,
            valueCurve: property.valueCurve ?? false,
            settingPrefix: property.settingPrefix ?? "SLIDER") {
            EffectPropertyView(property: axProp,
                                metadataPrefix: metadataPrefix)
        }
    }

    /// Resolve one axis's bounds/default. Per-axis fields win; a bare
    /// `min`/`max`/`default` applies to both axes when the per-axis
    /// variant is omitted (desktop parity).
    private func axisBounds(_ axis: String) -> (min: Double, max: Double, defaultValue: Any) {
        let perAxisMin = axis == "X" ? property.minX : property.minY
        let perAxisMax = axis == "X" ? property.maxX : property.maxY
        let perAxisDef = axis == "X" ? property.defaultX : property.defaultY

        let minV = perAxisMin ?? property.min ?? 0
        let maxV = perAxisMax ?? property.max ?? 100

        // Per-axis defaults arrive as unscaled floats from shader
        // metadata (e.g. 0.5); the slider stores scaled ints. Scale
        // by divisor to match JsonEffectPanel.cpp:1466's axDef * divisor.
        let rawDef: Double
        if let val = perAxisDef?.value {
            switch val {
            case let d as Double: rawDef = d
            case let i as Int:    rawDef = Double(i)
            case let s as String: rawDef = Double(s) ?? 0
            default:              rawDef = 0
            }
        } else if let val = property.defaultValue?.value {
            switch val {
            case let d as Double: rawDef = d
            case let i as Int:    rawDef = Double(i)
            case let s as String: rawDef = Double(s) ?? 0
            default:              rawDef = 0
            }
        } else {
            rawDef = 0
        }
        let scaledDef = Int((rawDef * Double(max(divisor, 1))).rounded())
        return (minV, maxV, scaledDef)
    }
}
