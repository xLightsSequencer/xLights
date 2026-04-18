import SwiftUI

// Drawing subviews used inside the value-curve editor: a live preview
// strip that re-samples `ValueCurve::GetValueAt` at 1 px per column, and a
// touch canvas for editing the Custom / Random point list.

// MARK: - Preview strip

/// Strokes the curve 0..1 → 0..1 using the same evaluator the renderer
/// uses. Replicates the algorithm of
/// `src-ui-wx/ui/shared/utils/ValueCurveRendering.cpp:27-70` but with
/// SwiftUI Canvas instead of wxMemoryDC.
struct ValueCurvePreviewStrip: View {
    let vc: EditableValueCurve

    var body: some View {
        Canvas { ctx, size in
            // Background
            let bg = Path(roundedRect: CGRect(origin: .zero, size: size),
                           cornerRadius: 4)
            ctx.fill(bg, with: .color(.secondary.opacity(0.08)))

            guard size.width > 1 && size.height > 1 else { return }

            // Plot curve: x steps 1 px at a time, y = GetValueAt(x/w).
            // Desktop uses 0..1 for both axes; we flip y so high values
            // render up the screen.
            var path = Path()
            let w = size.width
            let h = size.height
            for xPx in stride(from: CGFloat(0), through: w, by: 1) {
                let offset = Double(xPx / w)
                let v = vc.valueAt(offset: offset)
                let y = h - CGFloat(v) * h
                if xPx == 0 {
                    path.move(to: CGPoint(x: xPx, y: y))
                } else {
                    path.addLine(to: CGPoint(x: xPx, y: y))
                }
            }
            ctx.stroke(path,
                        with: .color(.accentColor),
                        lineWidth: 1.5)
        }
    }
}

// MARK: - Custom-point editor

/// Touch canvas for Custom / Random curves. Tap on empty space to add a
/// point, drag an existing point to move it, swipe up (y > 1) or long-
/// press to delete. Points are normalised to the VC's 1/200 x grid by the
/// underlying `SetValueAt`, matching desktop.
struct ValueCurveCustomPointEditor: View {
    let vc: EditableValueCurve

    @State private var draggingIndex: Int? = nil
    @State private var hoverLocation: CGPoint? = nil

    var body: some View {
        GeometryReader { geo in
            canvasSurface(in: geo.size)
        }
    }

    @ViewBuilder
    private func canvasSurface(in size: CGSize) -> some View {
        let points = vc.pointPairs()
        let pixelPoints = points.map { valueToPoint($0, in: size) }

        ZStack {
            // Background + curve trace
            Canvas { ctx, sz in
                let bg = Path(roundedRect: CGRect(origin: .zero, size: sz),
                               cornerRadius: 4)
                ctx.fill(bg, with: .color(.secondary.opacity(0.08)))
                ctx.stroke(bg, with: .color(.secondary.opacity(0.35)),
                           lineWidth: 0.5)

                // Subtle grid
                let gridColour = GraphicsContext.Shading.color(.secondary.opacity(0.12))
                for i in 1..<4 {
                    let fr = CGFloat(i) / 4
                    var vLine = Path()
                    vLine.move(to: CGPoint(x: fr * sz.width, y: 0))
                    vLine.addLine(to: CGPoint(x: fr * sz.width, y: sz.height))
                    ctx.stroke(vLine, with: gridColour,
                                style: StrokeStyle(lineWidth: 1, dash: [2, 2]))
                    var hLine = Path()
                    hLine.move(to: CGPoint(x: 0, y: fr * sz.height))
                    hLine.addLine(to: CGPoint(x: sz.width, y: fr * sz.height))
                    ctx.stroke(hLine, with: gridColour,
                                style: StrokeStyle(lineWidth: 1, dash: [2, 2]))
                }

                // Live curve trace through the evaluator (respects clamping
                // between points, wrap, etc.)
                guard sz.width > 1 else { return }
                var trace = Path()
                for xPx in stride(from: CGFloat(0), through: sz.width, by: 1) {
                    let offset = Double(xPx / sz.width)
                    let v = vc.valueAt(offset: offset)
                    let y = sz.height - CGFloat(v) * sz.height
                    if xPx == 0 {
                        trace.move(to: CGPoint(x: xPx, y: y))
                    } else {
                        trace.addLine(to: CGPoint(x: xPx, y: y))
                    }
                }
                ctx.stroke(trace,
                            with: .color(.accentColor.opacity(0.6)),
                            lineWidth: 1.2)

                // Point dots
                for pt in pixelPoints {
                    let dot = Path(ellipseIn:
                        CGRect(x: pt.x - 5, y: pt.y - 5, width: 10, height: 10))
                    ctx.fill(dot, with: .color(.accentColor))
                    ctx.stroke(dot, with: .color(.white),
                                lineWidth: 1.5)
                }
            }
            .contentShape(Rectangle())
            .gesture(dragGesture(size: size, points: points))
        }
    }

    private func dragGesture(size: CGSize,
                              points: [(x: Double, y: Double)])
    -> some Gesture {
        DragGesture(minimumDistance: 0)
            .onChanged { g in
                // On first tick of a drag, pick the nearest point or
                // create one if none are close.
                if draggingIndex == nil {
                    if let idx = nearestPointIndex(to: g.location,
                                                    points: points,
                                                    size: size,
                                                    hitRadius: 22) {
                        draggingIndex = idx
                    } else {
                        // Create a new point at this location (but only if
                        // we're inside the pad — g.startLocation being
                        // inside is a reasonable signal).
                        let (nx, ny) = pointToValue(g.location, in: size)
                        vc.setPoint(x: nx, y: ny)
                        // Re-fetch and find the freshly-added point's
                        // index so the rest of the drag updates it.
                        let refreshed = vc.pointPairs()
                        if let idx = nearestPointIndex(to: g.location,
                                                        points: refreshed,
                                                        size: size,
                                                        hitRadius: 80) {
                            draggingIndex = idx
                        }
                    }
                }
                guard let idx = draggingIndex,
                      idx < points.count else { return }
                let oldX = points[idx].x
                let clamped = CGPoint(
                    x: min(max(0, g.location.x), size.width),
                    y: min(max(0, g.location.y), size.height)
                )
                let (nx, ny) = pointToValue(clamped, in: size)
                // Drop the old point, write the new one. Same effect as
                // desktop's drag — the list is keyed by x so moving x
                // requires delete+add.
                if abs(oldX - nx) > 1e-6 {
                    vc.deletePoint(x: oldX)
                }
                vc.setPoint(x: nx, y: ny)
            }
            .onEnded { _ in
                draggingIndex = nil
            }
    }

    // MARK: - Mapping

    private func valueToPoint(_ p: (x: Double, y: Double),
                               in size: CGSize) -> CGPoint {
        return CGPoint(x: CGFloat(p.x) * size.width,
                       y: size.height - CGFloat(p.y) * size.height)
    }

    private func pointToValue(_ pt: CGPoint,
                               in size: CGSize) -> (Double, Double) {
        let x = Double(pt.x / size.width).clamped(to: 0...1)
        let y = Double(1 - pt.y / size.height).clamped(to: 0...1)
        return (x, y)
    }

    private func nearestPointIndex(to pt: CGPoint,
                                    points: [(x: Double, y: Double)],
                                    size: CGSize,
                                    hitRadius: CGFloat) -> Int? {
        var bestIdx: Int? = nil
        var bestDist: CGFloat = hitRadius
        for (i, p) in points.enumerated() {
            let px = valueToPoint(p, in: size)
            let d = hypot(px.x - pt.x, px.y - pt.y)
            if d < bestDist {
                bestDist = d
                bestIdx = i
            }
        }
        return bestIdx
    }
}

private extension Comparable {
    func clamped(to range: ClosedRange<Self>) -> Self {
        return Swift.min(Swift.max(self, range.lowerBound), range.upperBound)
    }
}
