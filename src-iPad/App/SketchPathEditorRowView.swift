import SwiftUI
import UniformTypeIdentifiers

// G4 — touch editor for the Sketch effect's `E_TEXTCTRL_SketchDef`
// polyline string. Mirrors the authoring surface of desktop's
// `SketchAssistPanel` in a form that works without a mouse:
//
//   * Renders every path with line / quadratic / cubic segments,
//     including closed paths.
//   * Drag endpoints to move them.
//   * Drag Q / C control points to re-shape curves.
//   * Tap empty canvas in "Add Line" mode to append a line segment
//     to the active (last) path.
//   * Tap empty canvas in "Add Curve" mode to append a cubic-bezier
//     segment whose control points seed automatically and are then
//     draggable.
//   * "New Path" starts a disconnected sub-path.
//   * "Close" closes / re-opens the active path (desktop's path-close
//     semantics — emits a `cN` token).
//   * "▲ / ▼" reorder the active path among its siblings (paths draw
//     in order, matching desktop's Move Path Up/Down).
//   * "Import SVG" converts an SVG file's paths into sketch paths
//     (shared nanosvg conversion with the desktop assist panel).
//   * "Remove Last" trims the last segment off the active path
//     (or drops the active path entirely when only the start
//     point remains).
//   * "Clear" resets to an empty definition.
//
// The raw `SketchDef` text field (SketchDefRowView) remains available
// for copy/paste import/export of definitions.
struct SketchPathEditorRowView: View {
    @Environment(SequencerViewModel.self) var viewModel

    private static let settingKey = "E_TEXTCTRL_SketchDef"
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
    @State private var importingSVG: Bool = false
    /// The path the toolbar acts on (append / close / reorder /
    /// highlight). `nil` means "the last path" — the common case after
    /// drawing. Set explicitly when the user reorders so the moved path
    /// stays selected, mirroring desktop's path-list selection.
    @State private var selectedPath: Int? = nil
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
                    Image(systemName: "scribble")
                        .font(.caption2)
                }
                .buttonStyle(.bordered)
                .controlSize(.small)

                Button {
                    importingSVG = true
                } label: {
                    Image(systemName: "square.and.arrow.down")
                        .font(.caption2)
                }
                .buttonStyle(.bordered)
                .controlSize(.small)
            }

            HStack(spacing: 6) {
                Button {
                    toggleCloseActivePath()
                } label: {
                    Label(activePathClosed ? "Open" : "Close",
                          systemImage: activePathClosed ? "lock.open" : "lock")
                        .font(.caption2)
                }
                .buttonStyle(.bordered)
                .controlSize(.small)
                .disabled(!canCloseActivePath)

                Button {
                    moveActivePath(by: -1)
                } label: {
                    Image(systemName: "arrow.up")
                        .font(.caption2)
                }
                .buttonStyle(.bordered)
                .controlSize(.small)
                .disabled(definition.paths.count < 2)

                Button {
                    moveActivePath(by: 1)
                } label: {
                    Image(systemName: "arrow.down")
                        .font(.caption2)
                }
                .buttonStyle(.bordered)
                .controlSize(.small)
                .disabled(definition.paths.count < 2)

                Button {
                    removeLastPoint()
                } label: {
                    Image(systemName: "arrow.uturn.backward")
                        .font(.caption2)
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

            pathManagementSection

            Text(modeHelpText)
                .font(.caption2)
                .foregroundStyle(.secondary)
        }
        .padding(.vertical, 4)
        .fileImporter(isPresented: $importingSVG,
                      allowedContentTypes: [UTType(filenameExtension: "svg") ?? .image]) { result in
            handleSVGImport(result)
        }
    }

    private var modeHelpText: String {
        switch editMode {
        case .drag:     return "Drag a handle to reshape. The active (highlighted) path is the last one."
        case .addLine:  return "Tap to add a line segment to the active path."
        case .addCurve: return "Tap to add a curve segment, then drag its control handles."
        }
    }

    // MARK: - Path management (#5871) — list, description, delete

    @ViewBuilder
    private var pathManagementSection: some View {
        let def = definition
        if !def.paths.isEmpty {
            VStack(alignment: .leading, spacing: 6) {
                VStack(spacing: 0) {
                    ForEach(Array(def.paths.enumerated()), id: \.offset) { idx, _ in
                        Button {
                            selectedPath = idx
                        } label: {
                            HStack(spacing: 6) {
                                Image(systemName: idx == activeIndex
                                      ? "largecircle.fill.circle" : "circle")
                                    .foregroundStyle(idx == activeIndex
                                                     ? AnyShapeStyle(Color.accentColor)
                                                     : AnyShapeStyle(.secondary))
                                Text(pathLabel(def, idx))
                                    .font(.caption)
                                    .lineLimit(1)
                                    .foregroundStyle(.primary)
                                Spacer()
                                Button {
                                    deletePath(idx)
                                } label: {
                                    Image(systemName: "trash")
                                        .font(.caption2)
                                        .foregroundStyle(.red)
                                }
                                .buttonStyle(.borderless)
                            }
                            .contentShape(Rectangle())
                            .padding(.vertical, 3)
                        }
                        .buttonStyle(.plain)
                        if idx != def.paths.count - 1 { Divider() }
                    }
                }
                .padding(.horizontal, 8)
                .background(Color.secondary.opacity(0.06))
                .clipShape(RoundedRectangle(cornerRadius: 6))

                if let active = activeIndex {
                    HStack(spacing: 6) {
                        Text("Desc")
                            .font(.caption2)
                            .foregroundStyle(.secondary)
                        TextField("Path description",
                                  text: descriptionBinding(active))
                            .font(.caption)
                            .textFieldStyle(.roundedBorder)
                            .submitLabel(.done)
                    }
                    // Hardware ⌫ deletes the selected path (desktop #5871).
                    Button("Delete Path") { deletePath(active) }
                        .keyboardShortcut(.delete, modifiers: [])
                        .hidden()
                        .frame(width: 0, height: 0)
                }
            }
        }
    }

    private func pathLabel(_ def: SketchDefinition, _ idx: Int) -> String {
        let d = def.paths[idx].descriptionText
        return d.isEmpty ? "Path \(idx + 1)" : "Path \(idx + 1) - \(d)"
    }

    private func descriptionBinding(_ idx: Int) -> Binding<String> {
        Binding(
            get: {
                let def = definition
                guard def.paths.indices.contains(idx) else { return "" }
                return def.paths[idx].descriptionText
            },
            set: { newValue in
                var def = definition
                guard def.paths.indices.contains(idx) else { return }
                def.paths[idx].descriptionText = newValue
                write(def)
            })
    }

    private func deletePath(_ idx: Int) {
        var def = definition
        guard def.paths.indices.contains(idx) else { return }
        def.paths.remove(at: idx)
        if def.paths.isEmpty {
            selectedPath = nil
        } else {
            selectedPath = min(idx, def.paths.count - 1)
        }
        write(def)
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
        // A path needs at least one segment to be meaningfully closed.
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
                    // handle. Nothing in range + drag mode → the
                    // drag is a no-op; an add mode → don't grab a
                    // handle, let `onEnded` treat it as a tap-to-add.
                    let picked = nearestHandle(to: g.startLocation, rect: rect)
                    if let picked {
                        activeHandle = picked
                        dragStartDef = definition
                        // Grabbing any handle selects that path so the
                        // toolbar's close / reorder act on it.
                        selectedPath = picked.pathIndex
                    } else {
                        // No handle in range — defer to onEnded which
                        // performs the add (if in an add mode) so a
                        // quick tap doesn't register as a stray drag.
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
        // Seed the two control points at the thirds of the chord so the
        // user has visible handles to grab and reshape immediately.
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

    private func moveActivePath(by delta: Int) {
        var def = definition
        guard let i = activeIndex else { return }
        let j = i + delta
        guard j >= 0, j < def.paths.count else { return }
        def.paths.swapAt(i, j)
        selectedPath = j
        write(def)
    }

    private func handleSVGImport(_ result: Result<URL, Error>) {
        guard case .success(let url) = result else { return }
        let needsAccess = url.startAccessingSecurityScopedResource()
        defer { if needsAccess { url.stopAccessingSecurityScopedResource() } }
        _ = XLSequenceDocument.obtainAccess(toPath: url.path, enforceWritable: false)
        let def = XLSequenceDocument.sketchDef(fromSVGFile: url.path)
        guard !def.isEmpty else { return }
        viewModel.setSettingValue(def, forKey: Self.settingKey)
    }

    private func startNewPath() {
        // Drop a placeholder start point at the canvas centre; the
        // user drags it into position afterwards. A better UX would
        // be "next tap places the start" — adds state complexity,
        // skip for v1.
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
            // Only the start point remains — drop the whole path.
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
