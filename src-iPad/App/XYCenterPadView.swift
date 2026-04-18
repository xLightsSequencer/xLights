import SwiftUI

// Touch XY-pad control for the `xyCenter` group type in JSON metadata.
// Replaces the desktop paired X/Y sliders + wrap checkboxes
// (`JsonEffectPanel.cpp:658-764`) with a draggable handle inside a square,
// plus inline wrap toggles when the group declares them. X and Y settings
// are stored as ints in their declared slider range (commonly -100..100
// for "center offset" or 0..200 for wrapped coordinates); the pad reads
// and writes those directly through `SequencerViewModel.setSettingValue`.
struct XYCenterPadView: View {
    @Environment(SequencerViewModel.self) var viewModel
    let xProp: PropertyMetadata
    let yProp: PropertyMetadata
    let wrapX: PropertyMetadata?
    let wrapY: PropertyMetadata?
    let prefix: String

    private var xKey: String { xProp.settingKey(prefix: prefix) }
    private var yKey: String { yProp.settingKey(prefix: prefix) }

    private var xMin: Double { xProp.min ?? 0 }
    private var xMax: Double { xProp.max ?? 100 }
    private var yMin: Double { yProp.min ?? 0 }
    private var yMax: Double { yProp.max ?? 100 }

    private var xDefault: String { xProp.defaultAsString() }
    private var yDefault: String { yProp.defaultAsString() }

    private var xValue: Double {
        Double(viewModel.settingValue(forKey: xKey, defaultValue: xDefault)) ?? 0
    }
    private var yValue: Double {
        Double(viewModel.settingValue(forKey: yKey, defaultValue: yDefault)) ?? 0
    }

    // Wrap-checkbox accessors — each optional since not every pad declares
    // one or both wrap toggles.
    private func wrapBinding(for prop: PropertyMetadata) -> Binding<Bool> {
        let key = prop.settingKey(prefix: prefix)
        let def = prop.defaultAsString()
        return Binding<Bool>(
            get: {
                let v = viewModel.settingValue(forKey: key, defaultValue: def)
                return v == "1" || v.lowercased() == "true"
            },
            set: { newVal in
                let suppress = (prop.suppressIfDefault == true) ? def : nil
                viewModel.setSettingValue(newVal ? "1" : "0",
                                           forKey: key,
                                           suppressIfDefault: suppress)
            }
        )
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            // Group title — desktop uses a static-box labeled with the first
            // property's generic prefix. "Position" is the most common
            // reading; we keep it neutral so any xyCenter user is happy.
            Text("Position")
                .font(.caption)
                .fontWeight(.medium)
                .foregroundStyle(.secondary)

            GeometryReader { geo in
                padSurface(in: geo.size)
            }
            .aspectRatio(1, contentMode: .fit)
            .frame(maxWidth: 200)

            HStack(spacing: 12) {
                Text("X: \(Int(xValue.rounded()))")
                    .monospacedDigit()
                Text("Y: \(Int(yValue.rounded()))")
                    .monospacedDigit()
                Spacer()
            }
            .font(.caption2)
            .foregroundStyle(.secondary)

            if let wx = wrapX {
                Toggle(isOn: wrapBinding(for: wx)) {
                    Text(wx.checkboxLabel?.isEmpty == false ? wx.checkboxLabel! : wx.label)
                        .font(.caption)
                }
                .toggleStyle(.switch)
                .controlSize(.small)
            }
            if let wy = wrapY {
                Toggle(isOn: wrapBinding(for: wy)) {
                    Text(wy.checkboxLabel?.isEmpty == false ? wy.checkboxLabel! : wy.label)
                        .font(.caption)
                }
                .toggleStyle(.switch)
                .controlSize(.small)
            }
        }
        .padding(.vertical, 4)
    }

    // MARK: - Pad surface

    @ViewBuilder
    private func padSurface(in size: CGSize) -> some View {
        let pt = valueToPoint(x: xValue, y: yValue, in: size)
        ZStack {
            // Background + crosshair lines through the centre of the range
            // so users have a reference at x=0 / y=0 (or midpoint when the
            // range is 0..max).
            RoundedRectangle(cornerRadius: 6)
                .fill(Color.secondary.opacity(0.08))
                .overlay(
                    RoundedRectangle(cornerRadius: 6)
                        .stroke(Color.secondary.opacity(0.35), lineWidth: 1)
                )

            let midX = valueToPoint(x: (xMin + xMax) / 2, y: yValue, in: size).x
            let midY = valueToPoint(x: xValue, y: (yMin + yMax) / 2, in: size).y
            Path { p in
                p.move(to: CGPoint(x: midX, y: 0))
                p.addLine(to: CGPoint(x: midX, y: size.height))
                p.move(to: CGPoint(x: 0, y: midY))
                p.addLine(to: CGPoint(x: size.width, y: midY))
            }
            .stroke(Color.secondary.opacity(0.25), style: StrokeStyle(lineWidth: 1, dash: [3, 3]))

            // Handle — solid dot with a ring to keep it visible against the
            // dashed crosshairs at the origin.
            Circle()
                .strokeBorder(Color.accentColor.opacity(0.4), lineWidth: 6)
                .background(Circle().fill(Color.accentColor))
                .frame(width: 18, height: 18)
                .position(pt)
        }
        .contentShape(Rectangle())
        .gesture(
            DragGesture(minimumDistance: 0)
                .onChanged { g in
                    let clamped = CGPoint(
                        x: min(max(0, g.location.x), size.width),
                        y: min(max(0, g.location.y), size.height)
                    )
                    let (nx, ny) = pointToValue(clamped, in: size)
                    writeX(nx)
                    writeY(ny)
                }
        )
    }

    // MARK: - Mapping

    private func valueToPoint(x: Double, y: Double, in size: CGSize) -> CGPoint {
        let px = CGFloat((x - xMin) / max(1e-9, xMax - xMin)) * size.width
        // Y range runs bottom-to-top on desktop (higher values → up), match
        // that here so users find the familiar orientation.
        let py = size.height - CGFloat((y - yMin) / max(1e-9, yMax - yMin)) * size.height
        return CGPoint(x: px, y: py)
    }

    private func pointToValue(_ pt: CGPoint, in size: CGSize) -> (Double, Double) {
        let nx = xMin + Double(pt.x / size.width) * (xMax - xMin)
        let ny = yMin + Double(1 - pt.y / size.height) * (yMax - yMin)
        return (nx.rounded(), ny.rounded())
    }

    private func writeX(_ v: Double) {
        let suppress = (xProp.suppressIfDefault == true) ? xDefault : nil
        viewModel.setSettingValue(String(Int(v)), forKey: xKey,
                                   suppressIfDefault: suppress)
    }
    private func writeY(_ v: Double) {
        let suppress = (yProp.suppressIfDefault == true) ? yDefault : nil
        viewModel.setSettingValue(String(Int(v)), forKey: yKey,
                                   suppressIfDefault: suppress)
    }
}
