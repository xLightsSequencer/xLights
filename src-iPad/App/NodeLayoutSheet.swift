import SwiftUI

/// Per-node geometry decoded from `XLSequenceDocument.nodeLayout(forModel:)`.
/// Mirrors the desktop ShowNodeLayout (ChannelLayoutDialog) and ShowWiring
/// (WiringDialog) data — buffer-screen coordinates, 1-based node number,
/// 0-based string number, and the start channel in display format.
private struct NodeLayoutData {
    struct Node: Identifiable {
        let id: Int
        let node: Int
        let string: Int
        let x: CGFloat
        let y: CGFloat
        let channel: String
    }
    let name: String
    let width: CGFloat
    let height: CGFloat
    let supportsWiring: Bool
    let nodes: [Node]

    init?(_ dict: [String: Any]?) {
        guard let dict else { return nil }
        name = (dict["name"] as? String) ?? ""
        width = CGFloat((dict["width"] as? NSNumber)?.doubleValue ?? 0)
        height = CGFloat((dict["height"] as? NSNumber)?.doubleValue ?? 0)
        supportsWiring = (dict["supportsWiring"] as? NSNumber)?.boolValue ?? false
        let raw = (dict["nodes"] as? [[String: Any]]) ?? []
        var parsed: [Node] = []
        parsed.reserveCapacity(raw.count)
        for (i, n) in raw.enumerated() {
            parsed.append(Node(
                id: i,
                node: (n["node"] as? NSNumber)?.intValue ?? 0,
                string: (n["string"] as? NSNumber)?.intValue ?? 0,
                x: CGFloat((n["x"] as? NSNumber)?.doubleValue ?? 0),
                y: CGFloat((n["y"] as? NSNumber)?.doubleValue ?? 0),
                channel: (n["channel"] as? String) ?? ""))
        }
        nodes = parsed
    }
}

/// Model-level Node Layout / Wiring View. `wiring == false` draws the
/// node-grid (numbers at buffer coordinates); `wiring == true` connects
/// the nodes in string order with a path and supports a front/rear flip.
/// Both back onto the same bridge accessor; the print / DXF-export
/// options of the desktop WiringDialog remain desktop-only (tracked in
/// the deep-dialog cluster).
struct NodeLayoutSheet: View {
    @Environment(SequencerViewModel.self) private var viewModel
    @Environment(\.dismiss) private var dismiss

    let modelName: String
    let wiring: Bool

    @State private var data: NodeLayoutData? = nil
    @State private var rear: Bool = false
    @State private var zoom: CGFloat = 1.0
    @State private var zoomCommitted: CGFloat = 1.0
    @State private var panOffset: CGSize = .zero
    @State private var panCommitted: CGSize = .zero

    private var title: String { wiring ? "Wiring View" : "Node Layout" }

    var body: some View {
        NavigationStack {
            Group {
                if let data, !data.nodes.isEmpty {
                    canvas(data)
                } else {
                    ContentUnavailableView("No nodes",
                        systemImage: "circle.grid.cross",
                        description: Text("This model has no node geometry to display."))
                }
            }
            .navigationTitle("\(title) — \(modelName)")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                if wiring {
                    ToolbarItem(placement: .topBarLeading) {
                        Picker("Side", selection: $rear) {
                            Text("Front").tag(false)
                            Text("Rear").tag(true)
                        }
                        .pickerStyle(.segmented)
                        .frame(width: 160)
                    }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") { dismiss() }
                }
            }
        }
        .presentationDetents([.large])
        .onAppear {
            if data == nil {
                let raw = viewModel.document.nodeLayout(forModel: modelName) as? [String: Any]
                data = NodeLayoutData(raw)
            }
        }
    }

    @ViewBuilder
    private func canvas(_ data: NodeLayoutData) -> some View {
        GeometryReader { geo in
            Canvas { ctx, size in
                draw(data, ctx: &ctx, size: size)
            }
            .frame(width: geo.size.width, height: geo.size.height)
            .background(Color.black)
            .contentShape(Rectangle())
            .gesture(SimultaneousGesture(panGesture, magnifyGesture))
            .onTapGesture(count: 2) {
                withAnimation(.easeOut(duration: 0.2)) {
                    zoom = 1.0; zoomCommitted = 1.0
                    panOffset = .zero; panCommitted = .zero
                }
            }
        }
    }

    /// Maps buffer coordinates → canvas points: fit the model bounds
    /// into the view, apply zoom + pan, and (for rear view) mirror X.
    private func transform(_ data: NodeLayoutData, size: CGSize)
        -> (scale: CGFloat, ox: CGFloat, oy: CGFloat) {
        let margin: CGFloat = 24
        let w = max(data.width, 1)
        let h = max(data.height, 1)
        let fit = min((size.width - 2 * margin) / w,
                      (size.height - 2 * margin) / h)
        let scale = max(fit, 0.0001) * zoom
        let drawnW = w * scale
        let drawnH = h * scale
        let ox = (size.width - drawnW) / 2 + panOffset.width
        let oy = (size.height - drawnH) / 2 + panOffset.height
        return (scale, ox, oy)
    }

    private func point(_ n: NodeLayoutData.Node, data: NodeLayoutData,
                       t: (scale: CGFloat, ox: CGFloat, oy: CGFloat),
                       size: CGSize) -> CGPoint {
        var x = n.x
        if rear { x = data.width - x }
        return CGPoint(x: t.ox + x * t.scale, y: t.oy + n.y * t.scale)
    }

    private func draw(_ data: NodeLayoutData, ctx: inout GraphicsContext, size: CGSize) {
        let t = transform(data, size: size)
        let count = data.nodes.count

        // Decimate labels when dense so tens of thousands of nodes stay
        // legible and cheap — only every Nth node gets a number.
        let labelStride = max(1, count / 400)
        let dotR: CGFloat = max(1.5, min(5.0, 4.0 * zoom))

        if wiring {
            // Connect nodes in string order. Each string is its own path,
            // coloured per string so adjacent strings are distinguishable.
            var byString: [Int: [NodeLayoutData.Node]] = [:]
            for n in data.nodes { byString[n.string, default: []].append(n) }
            for (idx, key) in byString.keys.sorted().enumerated() {
                let nodes = byString[key]!
                var path = Path()
                for (i, n) in nodes.enumerated() {
                    let p = point(n, data: data, t: t, size: size)
                    if i == 0 { path.move(to: p) } else { path.addLine(to: p) }
                }
                ctx.stroke(path, with: .color(stringColor(idx)), lineWidth: 1.2)
            }
        }

        for (i, n) in data.nodes.enumerated() {
            let p = point(n, data: data, t: t, size: size)
            let r = CGRect(x: p.x - dotR, y: p.y - dotR, width: dotR * 2, height: dotR * 2)
            let fill: Color = (i == 0) ? .green : (i == count - 1 ? .red : .white)
            ctx.fill(Path(ellipseIn: r), with: .color(fill))

            if i % labelStride == 0 {
                let text = Text("\(n.node)").font(.system(size: 9)).foregroundColor(.yellow)
                ctx.draw(text, at: CGPoint(x: p.x + dotR + 2, y: p.y), anchor: .leading)
            }
        }
    }

    private func stringColor(_ i: Int) -> Color {
        let palette: [Color] = [.cyan, .orange, .pink, .mint, .yellow, .purple, .teal, .indigo]
        return palette[i % palette.count]
    }

    private var magnifyGesture: some Gesture {
        MagnificationGesture()
            .onChanged { value in zoom = max(0.25, min(20.0, zoomCommitted * value)) }
            .onEnded { _ in zoomCommitted = zoom }
    }

    private var panGesture: some Gesture {
        DragGesture()
            .onChanged { value in
                panOffset = CGSize(width: panCommitted.width + value.translation.width,
                                   height: panCommitted.height + value.translation.height)
            }
            .onEnded { _ in panCommitted = panOffset }
    }
}
