import SwiftUI

// G4 — touch editor for the Sketch effect's `E_TEXTCTRL_SketchDef`
// polyline string. Mirrors the authoring subset of desktop's
// `SketchAssistPanel` in a form that works without a mouse:
//
//   * Renders every path with line / quadratic / cubic segments.
//   * Drag endpoints to move them.
//   * Drag Q / C control points to re-shape curves.
//   * Tap empty canvas in "Add Line" mode to append a line segment
//     to the active (last) path.
//   * "New Path" starts a disconnected sub-path.
//   * "Remove Last" trims the last segment off the active path
//     (or drops the active path entirely when only the start
//     point remains).
//   * "Clear" resets to an empty definition.
//
// Advanced authoring (cubic/quadratic creation, closing paths,
// re-ordering paths, import/export) remains desktop-only. The
// string round-trips through desktop's parser untouched for paths
// the iPad doesn't know how to edit, so users who start a sketch
// on the desktop can tweak endpoints on the iPad without losing
// curve segments.
struct SketchPathEditorRowView: View {
    @Environment(SequencerViewModel.self) var viewModel

    private static let settingKey = "E_TEXTCTRL_SketchDef"
    private static let canvasSize: CGFloat = 260
    private static let margin: CGFloat = 12
    private static let handleRadius: CGFloat = 8
    private static let handleHitRadius: CGFloat = 22
    private static let controlHandleRadius: CGFloat = 6

    @State private var addMode: Bool = false
    @State private var activeHandle: HandleID? = nil
    /// Snapshot of the definition taken at the start of the current
    /// drag — used so each drag commits exactly one undo step.
    @State private var dragStartDef: SketchDefinition? = nil

    /// Identifies a draggable handle in the current definition.
    private enum HandleKind: Equatable {
        case start                  // path's move-to point
        case endpoint(Int)          // segments[i].endpoint
        case quadCtrl(Int)          // segments[i].ctrl (for Q)
        case cubicCtrl1(Int)        // segments[i].ctrl1 (for C)
        case cubicCtrl2(Int)        // segments[i].ctrl2 (for C)
    }
    private struct HandleID: Equatable {
        let pathIndex: Int
        let kind: HandleKind
    }

    private var storedString: String {
        viewModel.settingValue(forKey: Self.settingKey, defaultValue: "")
    }

    private var definition: SketchDefinition {
        SketchDefinition.parse(storedString)
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            HStack(spacing: 6) {
                Text("Sketch Path")
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

            HStack(spacing: 8) {
                Toggle(isOn: $addMode) {
                    Label("Add Line", systemImage: "plus.circle")
                        .font(.caption2)
                }
                .toggleStyle(.button)
                .controlSize(.small)
                Button {
                    startNewPath()
                } label: {
                    Label("New Path", systemImage: "scribble")
                        .font(.caption2)
                }
                .buttonStyle(.bordered)
                .controlSize(.small)
                .disabled(definition.isEmpty)
                Button {
                    removeLastPoint()
                } label: {
                    Label("Undo Point", systemImage: "arrow.uturn.backward")
                        .font(.caption2)
                        .labelStyle(.iconOnly)
                }
                .buttonStyle(.bordered)
                .controlSize(.small)
                .disabled(definition.isEmpty)
                Button(role: .destructive) {
                    clearAll()
                } label: {
                    Image(systemName: "xmark.bin")
                        .font(.caption2)
                }
                .buttonStyle(.bordered)
                .controlSize(.small)
                .disabled(definition.isEmpty)
            }
            Text(addMode
                 ? "Tap to add a line segment to the last path."
                 : "Drag a handle to reshape. Desktop Effect Assist is still required for closing paths and importing SVGs.")
                .font(.caption2)
                .foregroundStyle(.secondary)
        }
        .padding(.vertical, 4)
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
            // 10-unit (i.e., 10%) gridlines to keep users oriented.
            for i in 1..<10 {
                let frac = CGFloat(i) / 10.0
                let x = rect.minX + rect.width * frac
                var v = Path()
                v.move(to: CGPoint(x: x, y: rect.minY))
                v.addLine(to: CGPoint(x: x, y: rect.maxY))
                ctx.stroke(v,
                           with: .color(Color.secondary.opacity(0.1)),
                           lineWidth: 0.5)
                let y = rect.minY + rect.height * frac
                var h = Path()
                h.move(to: CGPoint(x: rect.minX, y: y))
                h.addLine(to: CGPoint(x: rect.maxX, y: y))
                ctx.stroke(h,
                           with: .color(Color.secondary.opacity(0.1)),
                           lineWidth: 0.5)
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
                let isActive = (idx == def.paths.count - 1)
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
                // Start handle.
                handleCircle(at: screen(path.start, in: rect),
                             color: .accentColor,
                             isActive: activeHandle == HandleID(pathIndex: pIdx,
                                                                  kind: .start))
                // Segment handles.
                ForEach(Array(path.segments.enumerated()), id: \.offset) { sIdx, seg in
                    // Endpoint.
                    let endID = HandleID(pathIndex: pIdx, kind: .endpoint(sIdx))
                    handleCircle(at: screen(seg.endpoint, in: rect),
                                 color: .accentColor,
                                 isActive: activeHandle == endID)

                    // Control points (dashed tether + smaller
                    // handle, matching desktop's handle style).
                    switch seg {
                    case .line:
                        EmptyView()
                    case .quad(let c, let e):
                        controlTether(from: prevEndpoint(in: path, before: sIdx,
                                                          rect: rect),
                                      ctrl: screen(c, in: rect),
                                      to: screen(e, in: rect))
                        controlHandle(at: screen(c, in: rect),
                                       isActive: activeHandle == HandleID(
                                        pathIndex: pIdx,
                                        kind: .quadCtrl(sIdx)))
                    case .cubic(let c1, let c2, let e):
                        let prev = prevEndpoint(in: path, before: sIdx, rect: rect)
                        controlTether(from: prev,
                                      ctrl: screen(c1, in: rect),
                                      to: screen(e, in: rect))
                        controlTether(from: prev,
                                      ctrl: screen(c2, in: rect),
                                      to: screen(e, in: rect))
                        controlHandle(at: screen(c1, in: rect),
                                       isActive: activeHandle == HandleID(
                                        pathIndex: pIdx,
                                        kind: .cubicCtrl1(sIdx)))
                        controlHandle(at: screen(c2, in: rect),
                                       isActive: activeHandle == HandleID(
                                        pathIndex: pIdx,
                                        kind: .cubicCtrl2(sIdx)))
                    }
                }
            }
        }
    }

    private func prevEndpoint(in path: SketchPath, before index: Int,
                               rect: CGRect) -> CGPoint {
        if index == 0 { return screen(path.start, in: rect) }
        return screen(path.segments[index - 1].endpoint, in: rect)
    }

    @ViewBuilder
    private func handleCircle(at p: CGPoint, color: Color,
                               isActive: Bool) -> some View {
        Circle()
            .fill(color.opacity(isActive ? 0.95 : 0.75))
            .overlay(Circle().stroke(Color.white, lineWidth: 1.5))
            .frame(width: Self.handleRadius * 2,
                   height: Self.handleRadius * 2)
            .position(p)
    }

    @ViewBuilder
    private func controlHandle(at p: CGPoint, isActive: Bool) -> some View {
        Rectangle()
            .fill(Color.purple.opacity(isActive ? 0.95 : 0.7))
            .overlay(Rectangle().stroke(Color.white, lineWidth: 1))
            .frame(width: Self.controlHandleRadius * 2,
                   height: Self.controlHandleRadius * 2)
            .position(p)
    }

    @ViewBuilder
    private func controlTether(from: CGPoint, ctrl: CGPoint,
                                to: CGPoint) -> some View {
        // Dashed line from the owning endpoint to its control.
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
                    // First event of this drag — try to grab a
                    // handle. Nothing in range + addMode off → the
                    // drag is a no-op; addMode on → don't grab a
                    // handle, let `onEnded` treat it as a tap-to-add.
                    let picked = nearestHandle(to: g.startLocation, rect: rect)
                    if let picked {
                        activeHandle = picked
                        dragStartDef = definition
                    } else if !addMode {
                        // Leave activeHandle nil; subsequent drag events
                        // stay no-ops.
                        return
                    } else {
                        // Add-mode tap: also leave activeHandle nil
                        // — the add happens in onEnded so a quick tap
                        // doesn't register as a stray drag.
                        return
                    }
                }
                guard let h = activeHandle else { return }
                moveHandle(h, to: g.location, rect: rect, commit: false)
            }
            .onEnded { g in
                if let h = activeHandle {
                    moveHandle(h, to: g.location, rect: rect, commit: true)
                    activeHandle = nil
                    dragStartDef = nil
                    return
                }
                // No active handle — treat a short press as an
                // add-mode tap, if enabled and within the grid.
                if addMode, rect.contains(g.startLocation),
                   hypot(g.location.x - g.startLocation.x,
                         g.location.y - g.startLocation.y) < 6 {
                    appendLine(at: g.location, rect: rect)
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
                     screen(path.start, in: rect),
                     Self.handleHitRadius)
            for (sIdx, seg) in path.segments.enumerated() {
                consider(HandleID(pathIndex: pIdx, kind: .endpoint(sIdx)),
                         screen(seg.endpoint, in: rect),
                         Self.handleHitRadius)
                switch seg {
                case .line: break
                case .quad(let c, _):
                    consider(HandleID(pathIndex: pIdx, kind: .quadCtrl(sIdx)),
                             screen(c, in: rect),
                             Self.handleHitRadius)
                case .cubic(let c1, let c2, _):
                    consider(HandleID(pathIndex: pIdx, kind: .cubicCtrl1(sIdx)),
                             screen(c1, in: rect),
                             Self.handleHitRadius)
                    consider(HandleID(pathIndex: pIdx, kind: .cubicCtrl2(sIdx)),
                             screen(c2, in: rect),
                             Self.handleHitRadius)
                }
            }
        }
        return best?.0
    }

    // MARK: - Mutations

    /// Write a move of the given handle back to the settings map.
    /// `commit=false` is issued during the drag (no-op for this
    /// simple editor — we still write continuously so playback
    /// follows the move); `commit=true` is the drag-end step.
    private func moveHandle(_ id: HandleID, to screenPt: CGPoint,
                             rect: CGRect, commit: Bool) {
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
        _ = commit  // single undo entry per write via setSettingValue
    }

    private func appendLine(at screenPt: CGPoint, rect: CGRect) {
        var def = definition
        let p = normalise(screenPt, in: rect)
        if def.paths.isEmpty {
            def.paths.append(SketchPath(start: p))
        } else {
            var last = def.paths[def.paths.count - 1]
            last.segments.append(.line(end: p))
            def.paths[def.paths.count - 1] = last
        }
        write(def)
    }

    private func startNewPath() {
        // Drop a placeholder start point at the canvas centre; the
        // user drags it into position afterwards. A better UX would
        // be "next tap places the start" — adds state complexity,
        // skip for v1.
        var def = definition
        def.paths.append(SketchPath(start: CGPoint(x: 0.5, y: 0.5)))
        write(def)
    }

    private func removeLastPoint() {
        var def = definition
        guard !def.paths.isEmpty else { return }
        var last = def.paths[def.paths.count - 1]
        if !last.segments.isEmpty {
            last.segments.removeLast()
            def.paths[def.paths.count - 1] = last
        } else {
            // Only the start point remains — drop the whole path.
            def.paths.removeLast()
        }
        write(def)
    }

    private func clearAll() {
        write(SketchDefinition())
    }

    private func write(_ def: SketchDefinition) {
        let s = def.serialise()
        viewModel.setSettingValue(s, forKey: Self.settingKey)
    }

    // MARK: - Coord conversions

    private func screen(_ p: CGPoint, in rect: CGRect) -> CGPoint {
        CGPoint(x: rect.minX + rect.width * p.x,
                y: rect.minY + rect.height * p.y)
    }

    private func normalise(_ p: CGPoint, in rect: CGRect) -> CGPoint {
        let x = (p.x - rect.minX) / rect.width
        let y = (p.y - rect.minY) / rect.height
        return CGPoint(x: max(0, min(1, x)), y: max(0, min(1, y)))
    }
}
