import SwiftUI

// Touch path editor for the Moving Head effect's `Path:` command.
// Desktop's `MovingHeadCanvasPanel` / `SketchCanvasPanel` draws the
// same Bezier waypoint path; the string format is identical to the
// Sketch effect's `SketchDef` (start point + L / Q / C / c tokens in
// [0,1] coords) so this view reuses `SketchDefinition` for parse /
// serialise.
//
// Coordinate note: the renderer treats stored-Y as "up" (it maps
// `(0.5 - y)` to the up axis in `MovingHeadEffect::CalculatePathPositions`),
// and desktop's canvas flips Y at the UI layer so the top of the
// canvas stores Y≈1. SwiftUI's canvas Y grows downward, so we flip Y
// in `screen` / `normalise` — a waypoint dragged to the top of the
// iPad canvas serialises to Y≈1, matching desktop's stored paths
// exactly (round-trips cleanly + renders identically).
//
// Unlike the Sketch editor this view reads / writes through the MH
// command bridge (`movingHeadCommand("Path", …)` /
// `setMovingHeadCommand("Path", …)`) rather than a settings key, so
// the path is written to every active fixture.
struct MovingHeadPathEditorRowView: View {
    @Environment(SequencerViewModel.self) var viewModel

    private static let canvasSize: CGFloat = 260
    private static let margin: CGFloat = 12
    private static let handleRadius: CGFloat = 8
    private static let handleHitRadius: CGFloat = 22
    private static let controlHandleRadius: CGFloat = 6

    private enum EditMode: Equatable {
        case drag
        case addLine
        case addCurve
    }

    @State private var editMode: EditMode = .drag
    @State private var activeHandle: HandleID? = nil
    @State private var selectedPath: Int? = nil

    private enum HandleKind: Equatable {
        case start
        case endpoint(Int)
        case quadCtrl(Int)
        case cubicCtrl1(Int)
        case cubicCtrl2(Int)
    }
    private struct HandleID: Equatable {
        let pathIndex: Int
        let kind: HandleKind
    }

    private var storedString: String {
        _ = viewModel.inspectorRevision
        guard let sel = viewModel.selectedEffect else { return "" }
        return viewModel.document.movingHeadCommand("Path",
                                                     forRow: Int32(sel.rowIndex),
                                                     atIndex: Int32(sel.effectIndex))
    }

    private var definition: SketchDefinition {
        SketchDefinition.parse(storedString)
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            HStack(spacing: 6) {
                Text("Path")
                    .font(.caption)
                Spacer()
                Text("\(definition.paths.count) paths · \(definition.totalPoints) pts")
                    .font(.caption2)
                    .foregroundStyle(.secondary)
                    .monospacedDigit()
            }

            canvas
                .frame(width: Self.canvasSize, height: Self.canvasSize)
                .frame(maxWidth: .infinity, alignment: .center)

            HStack(spacing: 6) {
                Picker("", selection: $editMode) {
                    Image(systemName: "hand.draw").tag(EditMode.drag)
                    Image(systemName: "line.diagonal").tag(EditMode.addLine)
                    Image(systemName: "point.topleft.down.to.point.bottomright.curvepath").tag(EditMode.addCurve)
                }
                .pickerStyle(.segmented)
                .controlSize(.small)
                .fixedSize()

                Button {
                    startNewPath()
                } label: {
                    Image(systemName: "scribble").font(.caption2)
                }
                .buttonStyle(.bordered)
                .controlSize(.small)

                Button {
                    toggleCloseActivePath()
                } label: {
                    Image(systemName: activePathClosed ? "lock.open" : "lock")
                        .font(.caption2)
                }
                .buttonStyle(.bordered)
                .controlSize(.small)
                .disabled(!canCloseActivePath)

                Button {
                    removeLastPoint()
                } label: {
                    Image(systemName: "arrow.uturn.backward").font(.caption2)
                }
                .buttonStyle(.bordered)
                .controlSize(.small)
                .disabled(definition.isEmpty)

                Button(role: .destructive) {
                    clearAll()
                } label: {
                    Image(systemName: "xmark.bin").font(.caption2)
                }
                .buttonStyle(.bordered)
                .controlSize(.small)
                .disabled(definition.isEmpty)
            }

            MovingHeadPathPresetStrip()

            Text(modeHelpText)
                .font(.caption2)
                .foregroundStyle(.secondary)
        }
        .padding(.vertical, 4)
    }

    private var modeHelpText: String {
        switch editMode {
        case .drag:     return "Drag a handle to reshape. Top of the canvas is the up direction."
        case .addLine:  return "Tap to add a line waypoint to the active path."
        case .addCurve: return "Tap to add a curve waypoint, then drag its control handles."
        }
    }

    private var activeIndex: Int? {
        let count = definition.paths.count
        guard count > 0 else { return nil }
        if let s = selectedPath, s >= 0, s < count { return s }
        return count - 1
    }
    private var activePathClosed: Bool {
        guard let i = activeIndex else { return false }
        return definition.paths[i].isClosed
    }
    private var canCloseActivePath: Bool {
        guard let i = activeIndex else { return false }
        return !definition.paths[i].segments.isEmpty
    }

    // MARK: - Canvas

    private var canvas: some View {
        GeometryReader { geo in
            let rect = gridRect(in: geo.size)
            ZStack {
                background(rect: rect)
                pathsView(rect: rect)
                handlesView(rect: rect)
            }
            .contentShape(Rectangle())
            .gesture(primaryGesture(rect: rect))
        }
    }

    private func gridRect(in size: CGSize) -> CGRect {
        let inset = Self.margin
        let dim = min(size.width, size.height) - inset * 2
        return CGRect(x: inset, y: inset, width: dim, height: dim)
    }

    @ViewBuilder
    private func background(rect: CGRect) -> some View {
        Canvas { ctx, _ in
            ctx.fill(Path(rect), with: .color(Color.secondary.opacity(0.06)))
            ctx.stroke(Path(rect),
                       with: .color(Color.secondary.opacity(0.4)),
                       lineWidth: 1)
            for i in 1..<10 {
                let frac = CGFloat(i) / 10.0
                let x = rect.minX + rect.width * frac
                var v = Path()
                v.move(to: CGPoint(x: x, y: rect.minY))
                v.addLine(to: CGPoint(x: x, y: rect.maxY))
                ctx.stroke(v, with: .color(Color.secondary.opacity(0.1)), lineWidth: 0.5)
                let y = rect.minY + rect.height * frac
                var h = Path()
                h.move(to: CGPoint(x: rect.minX, y: y))
                h.addLine(to: CGPoint(x: rect.maxX, y: y))
                ctx.stroke(h, with: .color(Color.secondary.opacity(0.1)), lineWidth: 0.5)
            }
        }
    }

    @ViewBuilder
    private func pathsView(rect: CGRect) -> some View {
        let def = definition
        Canvas { ctx, _ in
            for (idx, path) in def.paths.enumerated() {
                var bezier = Path()
                bezier.move(to: screen(path.start, in: rect))
                for seg in path.segments {
                    switch seg {
                    case .line(let e):
                        bezier.addLine(to: screen(e, in: rect))
                    case .quad(let c, let e):
                        bezier.addQuadCurve(to: screen(e, in: rect),
                                            control: screen(c, in: rect))
                    case .cubic(let c1, let c2, let e):
                        bezier.addCurve(to: screen(e, in: rect),
                                        control1: screen(c1, in: rect),
                                        control2: screen(c2, in: rect))
                    }
                }
                if path.isClosed {
                    bezier.addLine(to: screen(path.start, in: rect))
                    bezier.closeSubpath()
                }
                let isActive = (idx == activeIndex)
                let color = isActive ? Color.accentColor : Color.secondary
                ctx.stroke(bezier,
                           with: .color(color.opacity(isActive ? 0.9 : 0.5)),
                           lineWidth: isActive ? 2 : 1.4)
            }
        }
    }

    @ViewBuilder
    private func handlesView(rect: CGRect) -> some View {
        let def = definition
        ZStack {
            ForEach(Array(def.paths.enumerated()), id: \.offset) { pIdx, path in
                handleCircle(at: screen(path.start, in: rect),
                             isActive: activeHandle == HandleID(pathIndex: pIdx, kind: .start))
                ForEach(Array(path.segments.enumerated()), id: \.offset) { sIdx, seg in
                    let endID = HandleID(pathIndex: pIdx, kind: .endpoint(sIdx))
                    handleCircle(at: screen(seg.endpoint, in: rect),
                                 isActive: activeHandle == endID)
                    switch seg {
                    case .line:
                        EmptyView()
                    case .quad(let c, let e):
                        controlTether(from: prevEndpoint(in: path, before: sIdx, rect: rect),
                                      ctrl: screen(c, in: rect),
                                      to: screen(e, in: rect))
                        controlHandle(at: screen(c, in: rect),
                                      isActive: activeHandle == HandleID(pathIndex: pIdx, kind: .quadCtrl(sIdx)))
                    case .cubic(let c1, let c2, let e):
                        let prev = prevEndpoint(in: path, before: sIdx, rect: rect)
                        controlTether(from: prev, ctrl: screen(c1, in: rect), to: screen(e, in: rect))
                        controlTether(from: prev, ctrl: screen(c2, in: rect), to: screen(e, in: rect))
                        controlHandle(at: screen(c1, in: rect),
                                      isActive: activeHandle == HandleID(pathIndex: pIdx, kind: .cubicCtrl1(sIdx)))
                        controlHandle(at: screen(c2, in: rect),
                                      isActive: activeHandle == HandleID(pathIndex: pIdx, kind: .cubicCtrl2(sIdx)))
                    }
                }
            }
        }
    }

    private func prevEndpoint(in path: SketchPath, before index: Int, rect: CGRect) -> CGPoint {
        if index == 0 { return screen(path.start, in: rect) }
        return screen(path.segments[index - 1].endpoint, in: rect)
    }

    @ViewBuilder
    private func handleCircle(at p: CGPoint, isActive: Bool) -> some View {
        Circle()
            .fill(Color.accentColor.opacity(isActive ? 0.95 : 0.75))
            .overlay(Circle().stroke(Color.white, lineWidth: 1.5))
            .frame(width: Self.handleRadius * 2, height: Self.handleRadius * 2)
            .position(p)
    }

    @ViewBuilder
    private func controlHandle(at p: CGPoint, isActive: Bool) -> some View {
        Rectangle()
            .fill(Color.purple.opacity(isActive ? 0.95 : 0.7))
            .overlay(Rectangle().stroke(Color.white, lineWidth: 1))
            .frame(width: Self.controlHandleRadius * 2, height: Self.controlHandleRadius * 2)
            .position(p)
    }

    @ViewBuilder
    private func controlTether(from: CGPoint, ctrl: CGPoint, to: CGPoint) -> some View {
        Canvas { ctx, _ in
            var p1 = Path()
            p1.move(to: from)
            p1.addLine(to: ctrl)
            ctx.stroke(p1, with: .color(Color.purple.opacity(0.5)),
                       style: StrokeStyle(lineWidth: 1, dash: [3, 2]))
            var p2 = Path()
            p2.move(to: ctrl)
            p2.addLine(to: to)
            ctx.stroke(p2, with: .color(Color.purple.opacity(0.5)),
                       style: StrokeStyle(lineWidth: 1, dash: [3, 2]))
        }
    }

    // MARK: - Gestures

    private func primaryGesture(rect: CGRect) -> some Gesture {
        DragGesture(minimumDistance: 0, coordinateSpace: .local)
            .onChanged { g in
                if activeHandle == nil {
                    let picked = nearestHandle(to: g.startLocation, rect: rect)
                    if let picked {
                        activeHandle = picked
                        selectedPath = picked.pathIndex
                    } else {
                        return
                    }
                }
                guard let h = activeHandle else { return }
                moveHandle(h, to: g.location, rect: rect)
            }
            .onEnded { g in
                if let h = activeHandle {
                    moveHandle(h, to: g.location, rect: rect)
                    activeHandle = nil
                    return
                }
                guard editMode != .drag,
                      rect.contains(g.startLocation),
                      hypot(g.location.x - g.startLocation.x,
                            g.location.y - g.startLocation.y) < 6 else { return }
                switch editMode {
                case .addLine:  appendLine(at: g.location, rect: rect)
                case .addCurve: appendCurve(at: g.location, rect: rect)
                case .drag:     break
                }
            }
    }

    private func nearestHandle(to loc: CGPoint, rect: CGRect) -> HandleID? {
        let def = definition
        var best: (HandleID, CGFloat)? = nil
        func consider(_ id: HandleID, _ p: CGPoint, _ radius: CGFloat) {
            let d = hypot(p.x - loc.x, p.y - loc.y)
            if d > radius { return }
            if best == nil || d < best!.1 { best = (id, d) }
        }
        for (pIdx, path) in def.paths.enumerated() {
            consider(HandleID(pathIndex: pIdx, kind: .start),
                     screen(path.start, in: rect), Self.handleHitRadius)
            for (sIdx, seg) in path.segments.enumerated() {
                consider(HandleID(pathIndex: pIdx, kind: .endpoint(sIdx)),
                         screen(seg.endpoint, in: rect), Self.handleHitRadius)
                switch seg {
                case .line: break
                case .quad(let c, _):
                    consider(HandleID(pathIndex: pIdx, kind: .quadCtrl(sIdx)),
                             screen(c, in: rect), Self.handleHitRadius)
                case .cubic(let c1, let c2, _):
                    consider(HandleID(pathIndex: pIdx, kind: .cubicCtrl1(sIdx)),
                             screen(c1, in: rect), Self.handleHitRadius)
                    consider(HandleID(pathIndex: pIdx, kind: .cubicCtrl2(sIdx)),
                             screen(c2, in: rect), Self.handleHitRadius)
                }
            }
        }
        return best?.0
    }

    // MARK: - Mutations

    private func moveHandle(_ id: HandleID, to screenPt: CGPoint, rect: CGRect) {
        var def = definition
        guard id.pathIndex >= 0, id.pathIndex < def.paths.count else { return }
        let p = normalise(screenPt, in: rect)
        var path = def.paths[id.pathIndex]
        switch id.kind {
        case .start:
            path.start = p
        case .endpoint(let sIdx):
            guard sIdx < path.segments.count else { return }
            switch path.segments[sIdx] {
            case .line:
                path.segments[sIdx] = .line(end: p)
            case .quad(let c, _):
                path.segments[sIdx] = .quad(ctrl: c, end: p)
            case .cubic(let c1, let c2, _):
                path.segments[sIdx] = .cubic(ctrl1: c1, ctrl2: c2, end: p)
            }
        case .quadCtrl(let sIdx):
            guard sIdx < path.segments.count else { return }
            if case .quad(_, let e) = path.segments[sIdx] {
                path.segments[sIdx] = .quad(ctrl: p, end: e)
            }
        case .cubicCtrl1(let sIdx):
            guard sIdx < path.segments.count else { return }
            if case .cubic(_, let c2, let e) = path.segments[sIdx] {
                path.segments[sIdx] = .cubic(ctrl1: p, ctrl2: c2, end: e)
            }
        case .cubicCtrl2(let sIdx):
            guard sIdx < path.segments.count else { return }
            if case .cubic(let c1, _, let e) = path.segments[sIdx] {
                path.segments[sIdx] = .cubic(ctrl1: c1, ctrl2: p, end: e)
            }
        }
        def.paths[id.pathIndex] = path
        write(def)
    }

    private func appendLine(at screenPt: CGPoint, rect: CGRect) {
        var def = definition
        let p = normalise(screenPt, in: rect)
        if let i = activeIndex {
            def.paths[i].segments.append(.line(end: p))
        } else {
            def.paths.append(SketchPath(start: p))
            selectedPath = nil
        }
        write(def)
    }

    private func appendCurve(at screenPt: CGPoint, rect: CGRect) {
        var def = definition
        let end = normalise(screenPt, in: rect)
        guard let i = activeIndex else {
            def.paths.append(SketchPath(start: end))
            selectedPath = nil
            write(def)
            return
        }
        let from = def.paths[i].segments.last?.endpoint ?? def.paths[i].start
        let c1 = CGPoint(x: from.x + (end.x - from.x) / 3,
                         y: from.y + (end.y - from.y) / 3)
        let c2 = CGPoint(x: from.x + 2 * (end.x - from.x) / 3,
                         y: from.y + 2 * (end.y - from.y) / 3)
        def.paths[i].segments.append(.cubic(ctrl1: c1, ctrl2: c2, end: end))
        write(def)
    }

    private func toggleCloseActivePath() {
        var def = definition
        guard let i = activeIndex else { return }
        if def.paths[i].isClosed {
            def.paths[i].open()
        } else {
            def.paths[i].close()
        }
        write(def)
    }

    private func startNewPath() {
        var def = definition
        def.paths.append(SketchPath(start: CGPoint(x: 0.5, y: 0.5)))
        selectedPath = def.paths.count - 1
        write(def)
    }

    private func removeLastPoint() {
        var def = definition
        guard let i = activeIndex else { return }
        if def.paths[i].isClosed {
            def.paths[i].open()
        } else if !def.paths[i].segments.isEmpty {
            def.paths[i].segments.removeLast()
        } else {
            def.paths.remove(at: i)
            selectedPath = nil
        }
        write(def)
    }

    private func clearAll() {
        selectedPath = nil
        write(SketchDefinition())
    }

    private func write(_ def: SketchDefinition) {
        guard let sel = viewModel.selectedEffect else { return }
        let s = def.isEmpty ? "" : def.serialise()
        _ = viewModel.document.setMovingHeadCommand(
            "Path", value: s,
            forRow: Int32(sel.rowIndex),
            atIndex: Int32(sel.effectIndex))
        viewModel.refreshSelectedEffectSettings()
        viewModel.inspectorRevision &+= 1
    }

    // MARK: - Coord conversions (Y flipped: stored-Y up = canvas top)

    private func screen(_ p: CGPoint, in rect: CGRect) -> CGPoint {
        CGPoint(x: rect.minX + rect.width * p.x,
                y: rect.minY + rect.height * (1.0 - p.y))
    }

    private func normalise(_ p: CGPoint, in rect: CGRect) -> CGPoint {
        let x = (p.x - rect.minX) / rect.width
        let y = 1.0 - (p.y - rect.minY) / rect.height
        return CGPoint(x: max(0, min(1, x)), y: max(0, min(1, y)))
    }
}

// Path quick-set presets — the two shipping desktop `.xmh` path
// presets (Circle, Diamond) plus a horizontal line sweep. Tapping
// one replaces the `Path:` command on every active fixture, matching
// desktop's path-preset bitmap buttons. The literal strings are the
// `data=` attributes from `resources/mhpresets/*.xmh`.
struct MovingHeadPathPresetStrip: View {
    @Environment(SequencerViewModel.self) var viewModel

    private struct Preset: Identifiable {
        let id = UUID()
        let name: String
        let icon: String
        let path: String
    }

    private static let presets: [Preset] = [
        Preset(name: "Circle", icon: "circle",
               path: "0.5,0.704819;Q0.683908,0.686747,0.695402,0.503012;Q0.686782,0.319277,0.5,0.304217;Q0.310345,0.316265,0.298851,0.503012;Q0.313218,0.686747,0.5,0.704819;c3"),
        Preset(name: "Diamond", icon: "diamond",
               path: "0.502874,0.903614;L0.899425,0.5;L0.497126,0.0993976;L0.0948276,0.5;c2"),
        Preset(name: "Sweep", icon: "arrow.left.and.right",
               path: "0.1,0.5;L0.9,0.5"),
    ]

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            Text("Path Presets")
                .font(.caption2)
                .foregroundStyle(.secondary)
            ScrollView(.horizontal, showsIndicators: false) {
                HStack(spacing: 8) {
                    ForEach(Self.presets) { preset in
                        Button {
                            apply(preset.path)
                        } label: {
                            VStack(spacing: 2) {
                                Image(systemName: preset.icon)
                                    .font(.caption)
                                Text(preset.name)
                                    .font(.caption2)
                            }
                            .frame(minWidth: 52)
                            .padding(.vertical, 4)
                        }
                        .buttonStyle(.bordered)
                        .controlSize(.small)
                    }
                }
            }
        }
    }

    private func apply(_ path: String) {
        guard let sel = viewModel.selectedEffect else { return }
        _ = viewModel.document.setMovingHeadCommand(
            "Path", value: path,
            forRow: Int32(sel.rowIndex),
            atIndex: Int32(sel.effectIndex))
        viewModel.refreshSelectedEffectSettings()
        viewModel.inspectorRevision &+= 1
    }
}
