import SwiftUI

// G5 — touch editor for the Morph effect's Start / End endpoint
// pairs. Mirrors desktop's `xlGridCanvasMorph`: a 100×100 grid
// with four draggable handles. The two "start" handles define one
// end of the morph (blue); the two "end" handles define the other
// (orange). Connecting lines show the current start / end line
// geometry.
//
// Writes update the eight `E_SLIDER_Morph_*` settings the renderer
// reads (MorphEffect.cpp:119-133). Start_Link / End_Link keep the
// paired handle pinned so a "single point" morph stays a point.
// Attached via `controlType: "Morph_LineEditor"` in Morph.json —
// rendered above the slider tabs so users on any tab can see the
// geometry without switching back.
struct MorphLineEditorRowView: View {
    @Environment(SequencerViewModel.self) var viewModel

    // Grid + canvas sizing. The canvas is always square; the grid
    // is drawn in the inner area, leaving a margin for the labels
    // and touch slop around the handles.
    private static let canvasSize: CGFloat = 260
    private static let margin: CGFloat = 16
    private static let handleRadius: CGFloat = 10
    private static let handleHitRadius: CGFloat = 22

    /// Which handle the current drag gesture is moving. Nil when no
    /// drag is in progress.
    @State private var activeHandle: Handle? = nil

    private enum Handle: String, Hashable {
        case s1a, s1b  // start line endpoints
        case e2a, e2b  // end line endpoints
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            Text("Morph Editor")
                .font(.caption)
                .foregroundStyle(.secondary)
            canvas
                .frame(width: Self.canvasSize, height: Self.canvasSize)
                .frame(maxWidth: .infinity, alignment: .center)
        }
        .padding(.vertical, 4)
    }

    // MARK: - Canvas

    private var canvas: some View {
        GeometryReader { geo in
            let rect = gridRect(in: geo.size)
            ZStack {
                gridBackground(rect: rect)
                startLine(rect: rect)
                endLine(rect: rect)
                handleView(.s1a, rect: rect, color: .blue, label: "1a")
                handleView(.s1b, rect: rect, color: .blue, label: "1b")
                handleView(.e2a, rect: rect, color: .orange, label: "2a")
                handleView(.e2b, rect: rect, color: .orange, label: "2b")
                if let h = activeHandle {
                    coordTooltip(for: h, rect: rect)
                }
            }
            .contentShape(Rectangle())
            .gesture(dragGesture(rect: rect))
        }
    }

    private func gridRect(in size: CGSize) -> CGRect {
        // Square inset, sized to leave a margin on all sides for
        // handle halos to stay inside the view.
        let inset = Self.margin
        let dim = min(size.width, size.height) - inset * 2
        return CGRect(x: inset, y: inset, width: dim, height: dim)
    }

    @ViewBuilder
    private func gridBackground(rect: CGRect) -> some View {
        // Fill + border. 10-unit gridlines faint, 25-unit heavier.
        Canvas { ctx, _ in
            ctx.fill(Path(rect), with: .color(Color.secondary.opacity(0.06)))
            ctx.stroke(Path(rect), with: .color(Color.secondary.opacity(0.4)),
                       lineWidth: 1)

            for i in 1..<10 {
                let frac = CGFloat(i) / 10.0
                let isQuarter = (i % 25 == 0) || (i == 5)
                let alpha = isQuarter ? 0.25 : 0.12

                let x = rect.minX + rect.width * frac
                var vPath = Path()
                vPath.move(to: CGPoint(x: x, y: rect.minY))
                vPath.addLine(to: CGPoint(x: x, y: rect.maxY))
                ctx.stroke(vPath, with: .color(Color.secondary.opacity(alpha)),
                           lineWidth: 0.5)

                let y = rect.minY + rect.height * frac
                var hPath = Path()
                hPath.move(to: CGPoint(x: rect.minX, y: y))
                hPath.addLine(to: CGPoint(x: rect.maxX, y: y))
                ctx.stroke(hPath, with: .color(Color.secondary.opacity(alpha)),
                           lineWidth: 0.5)
            }
        }
        .frame(width: rect.width + Self.margin * 2,
               height: rect.height + Self.margin * 2)
    }

    @ViewBuilder
    private func startLine(rect: CGRect) -> some View {
        let a = point(for: .s1a, rect: rect)
        let b = point(for: .s1b, rect: rect)
        Path { p in
            p.move(to: a)
            p.addLine(to: b)
        }
        .stroke(Color.blue.opacity(0.85), lineWidth: 2)
    }

    @ViewBuilder
    private func endLine(rect: CGRect) -> some View {
        let a = point(for: .e2a, rect: rect)
        let b = point(for: .e2b, rect: rect)
        Path { p in
            p.move(to: a)
            p.addLine(to: b)
        }
        .stroke(Color.orange.opacity(0.85), lineWidth: 2)
    }

    @ViewBuilder
    private func handleView(_ handle: Handle,
                             rect: CGRect,
                             color: Color,
                             label: String) -> some View {
        let p = point(for: handle, rect: rect)
        let isActive = activeHandle == handle
        let isLinked = isLinkedSlave(handle)

        ZStack {
            Circle()
                .fill(color.opacity(isActive ? 0.9 : 0.55))
                .frame(width: Self.handleRadius * 2,
                       height: Self.handleRadius * 2)
                .overlay(
                    Circle()
                        .stroke(Color.white, lineWidth: 2)
                )
            Text(label)
                .font(.system(size: 9, weight: .bold))
                .foregroundStyle(.white)
        }
        .position(p)
        .opacity(isLinked ? 0.45 : 1.0)
        .allowsHitTesting(!isLinked)
    }

    @ViewBuilder
    private func coordTooltip(for handle: Handle, rect: CGRect) -> some View {
        let vals = value(for: handle)
        let p = point(for: handle, rect: rect)
        let below = p.y < rect.midY
        let anchor: CGPoint = below
            ? CGPoint(x: p.x, y: p.y + 22)
            : CGPoint(x: p.x, y: p.y - 22)
        Text("(\(vals.x), \(vals.y))")
            .font(.caption2)
            .monospacedDigit()
            .padding(.horizontal, 6)
            .padding(.vertical, 2)
            .background(
                RoundedRectangle(cornerRadius: 4)
                    .fill(Color.black.opacity(0.72))
            )
            .foregroundStyle(.white)
            .position(anchor)
    }

    // MARK: - Drag

    private func dragGesture(rect: CGRect) -> some Gesture {
        DragGesture(minimumDistance: 0, coordinateSpace: .local)
            .onChanged { g in
                if activeHandle == nil {
                    activeHandle = nearestHandle(to: g.startLocation, rect: rect)
                }
                guard let h = activeHandle else { return }
                applyDrag(h, to: g.location, rect: rect)
            }
            .onEnded { _ in
                activeHandle = nil
            }
    }

    /// Pick the handle whose centre is closest to `loc` within the
    /// touch slop radius. Returns nil if nothing is in range — that
    /// lets the user tap empty canvas without accidentally grabbing
    /// a faraway handle.
    private func nearestHandle(to loc: CGPoint, rect: CGRect) -> Handle? {
        let candidates: [Handle] = [.s1a, .s1b, .e2a, .e2b]
        var best: (Handle, CGFloat)? = nil
        for h in candidates {
            if isLinkedSlave(h) { continue }
            let p = point(for: h, rect: rect)
            let d = hypot(p.x - loc.x, p.y - loc.y)
            if d > Self.handleHitRadius { continue }
            if best == nil || d < best!.1 {
                best = (h, d)
            }
        }
        return best?.0
    }

    private func applyDrag(_ handle: Handle, to loc: CGPoint, rect: CGRect) {
        let x = clamp(Int(round((loc.x - rect.minX) / rect.width * 100)),
                      min: 0, max: 100)
        let y = clamp(Int(round((loc.y - rect.minY) / rect.height * 100)),
                      min: 0, max: 100)
        write(handle, x: x, y: y)
    }

    // MARK: - Settings <-> screen

    private func xKey(for h: Handle) -> String {
        switch h {
        case .s1a: return "E_SLIDER_Morph_Start_X1"
        case .s1b: return "E_SLIDER_Morph_Start_X2"
        case .e2a: return "E_SLIDER_Morph_End_X1"
        case .e2b: return "E_SLIDER_Morph_End_X2"
        }
    }

    private func yKey(for h: Handle) -> String {
        switch h {
        case .s1a: return "E_SLIDER_Morph_Start_Y1"
        case .s1b: return "E_SLIDER_Morph_Start_Y2"
        case .e2a: return "E_SLIDER_Morph_End_Y1"
        case .e2b: return "E_SLIDER_Morph_End_Y2"
        }
    }

    private func defaultsFor(_ h: Handle) -> (Int, Int) {
        // Match Morph.json defaults.
        switch h {
        case .s1a: return (0, 0)
        case .s1b: return (100, 0)
        case .e2a: return (0, 100)
        case .e2b: return (100, 100)
        }
    }

    private func value(for h: Handle) -> (x: Int, y: Int) {
        let def = defaultsFor(h)
        let xs = viewModel.settingValue(forKey: xKey(for: h),
                                         defaultValue: String(def.0))
        let ys = viewModel.settingValue(forKey: yKey(for: h),
                                         defaultValue: String(def.1))
        return (Int(xs) ?? def.0, Int(ys) ?? def.1)
    }

    /// When Start_Link is set, the 1b handle tracks 1a; ditto End_Link
    /// for 2b and 2a. The linked slave is non-interactive.
    private func isLinkedSlave(_ h: Handle) -> Bool {
        switch h {
        case .s1b:
            return boolSetting("E_CHECKBOX_Morph_Start_Link")
        case .e2b:
            return boolSetting("E_CHECKBOX_Morph_End_Link")
        default:
            return false
        }
    }

    private func boolSetting(_ key: String) -> Bool {
        let s = viewModel.settingValue(forKey: key, defaultValue: "0")
        return s == "1" || s == "true"
    }

    private func point(for h: Handle, rect: CGRect) -> CGPoint {
        // Linked-slave handles draw on top of their master so the
        // line still renders with endpoints at the correct spot.
        let effective: Handle
        if h == .s1b && boolSetting("E_CHECKBOX_Morph_Start_Link") {
            effective = .s1a
        } else if h == .e2b && boolSetting("E_CHECKBOX_Morph_End_Link") {
            effective = .e2a
        } else {
            effective = h
        }
        let v = value(for: effective)
        return CGPoint(
            x: rect.minX + rect.width * CGFloat(v.x) / 100.0,
            y: rect.minY + rect.height * CGFloat(v.y) / 100.0
        )
    }

    private func write(_ h: Handle, x: Int, y: Int) {
        let curr = value(for: h)
        if curr.x != x {
            viewModel.setSettingValue(String(x), forKey: xKey(for: h))
        }
        if curr.y != y {
            viewModel.setSettingValue(String(y), forKey: yKey(for: h))
        }
    }

    private func clamp(_ v: Int, min lo: Int, max hi: Int) -> Int {
        Swift.max(lo, Swift.min(hi, v))
    }
}
