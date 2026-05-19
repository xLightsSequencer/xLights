import Foundation
import CoreGraphics

// G4 — parser / serialiser for the `E_TEXTCTRL_SketchDef` string
// that the Sketch effect stores. Mirrors desktop's
// `SketchEffectDrawing` / `SketchEffectPath` grammar in a wx-free
// Swift value type so the iPad path editor can round-trip edits.
//
// Grammar (matches desktop regex):
//
//     <sketch> = <path> ('|' <path>)*
//     <path>   = <start> (';' <segment>)*
//     <start>  = <x> ',' <y>                       // float, 0..1
//     <segment> = 'L' <x> ',' <y>                  // line to end
//               | 'Q' <cx>','<cy>','<x>','<y>       // quadratic
//               | 'C' <c1x>','<c1y>','<c2x>','<c2y>','<x>','<y>
//               | 'c'[0-3]                         // close (0..3 state)
//
// Coordinates are [0,1] normalised; Y increases downward (matches
// SwiftUI canvas coords). Endpoints for Q / C segments are the
// last pair of numbers in the token — control points come first.
//
// iPad v1 parses every segment type but only surfaces editing for
// endpoints and Q / C control points. Path closing (the 'c' token)
// round-trips unchanged.

struct SketchDefinition: Equatable {
    var paths: [SketchPath] = []

    var isEmpty: Bool { paths.isEmpty }

    /// Total endpoint count across all paths — used by the editor
    /// for a "N points" readout in the toolbar.
    var totalPoints: Int {
        paths.reduce(0) { $0 + $1.totalPoints }
    }

    static func parse(_ s: String) -> SketchDefinition {
        var out = SketchDefinition()
        let trimmed = s.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !trimmed.isEmpty else { return out }
        for pathStr in trimmed.split(separator: "|", omittingEmptySubsequences: true) {
            if let p = SketchPath.parse(String(pathStr)) {
                out.paths.append(p)
            }
        }
        return out
    }

    func serialise() -> String {
        paths.map { $0.serialise() }.joined(separator: "|")
    }
}

struct SketchPath: Equatable {
    /// First point — path's move-to.
    var start: CGPoint
    var segments: [SketchSegment] = []
    /// Trailing close token, e.g. "c0" / "c1" / "c2" / "c3". Empty
    /// when the path isn't closed. Round-tripped verbatim since iPad
    /// v1 doesn't expose a "close path" action.
    var closeToken: String = ""

    /// Endpoint count (includes the start point + one per segment).
    var totalPoints: Int { 1 + segments.count }

    /// All endpoints in draw order — `points[0]` is the start, each
    /// subsequent point is the endpoint of the corresponding segment.
    /// Useful for the editor when it needs a flat list for hit-
    /// testing and drag updates.
    var endpoints: [CGPoint] {
        var pts = [start]
        for seg in segments { pts.append(seg.endpoint) }
        return pts
    }

    static func parse(_ s: String) -> SketchPath? {
        let parts = s.split(separator: ";", omittingEmptySubsequences: true)
        guard let first = parts.first else { return nil }
        guard let start = parsePoint(String(first)) else { return nil }

        var path = SketchPath(start: start)
        for i in 1..<parts.count {
            let tok = String(parts[i])
            if tok.isEmpty { continue }
            // Close-token: "c" + optional digit.
            if tok.first == "c", !tok.hasPrefix("C") {
                path.closeToken = tok
                continue
            }
            if let seg = SketchSegment.parse(tok) {
                path.segments.append(seg)
            }
        }
        return path
    }

    func serialise() -> String {
        var parts = [formatPoint(start)]
        for seg in segments { parts.append(seg.serialise()) }
        if !closeToken.isEmpty { parts.append(closeToken) }
        return parts.joined(separator: ";")
    }
}

enum SketchSegment: Equatable {
    case line(end: CGPoint)
    case quad(ctrl: CGPoint, end: CGPoint)
    case cubic(ctrl1: CGPoint, ctrl2: CGPoint, end: CGPoint)

    var endpoint: CGPoint {
        switch self {
        case .line(let e): return e
        case .quad(_, let e): return e
        case .cubic(_, _, let e): return e
        }
    }

    static func parse(_ tok: String) -> SketchSegment? {
        guard let first = tok.first else { return nil }
        let body = String(tok.dropFirst())
        let floats = body
            .split(separator: ",")
            .compactMap { Double($0) }
            .map { CGFloat($0) }
        switch first {
        case "L":
            guard floats.count >= 2 else { return nil }
            return .line(end: CGPoint(x: floats[0], y: floats[1]))
        case "Q":
            guard floats.count >= 4 else { return nil }
            return .quad(ctrl: CGPoint(x: floats[0], y: floats[1]),
                         end:  CGPoint(x: floats[2], y: floats[3]))
        case "C":
            guard floats.count >= 6 else { return nil }
            return .cubic(
                ctrl1: CGPoint(x: floats[0], y: floats[1]),
                ctrl2: CGPoint(x: floats[2], y: floats[3]),
                end:   CGPoint(x: floats[4], y: floats[5]))
        default:
            return nil
        }
    }

    func serialise() -> String {
        switch self {
        case .line(let e):
            return "L\(formatFloat(e.x)),\(formatFloat(e.y))"
        case .quad(let c, let e):
            return "Q\(formatFloat(c.x)),\(formatFloat(c.y))," +
                   "\(formatFloat(e.x)),\(formatFloat(e.y))"
        case .cubic(let c1, let c2, let e):
            return "C\(formatFloat(c1.x)),\(formatFloat(c1.y))," +
                   "\(formatFloat(c2.x)),\(formatFloat(c2.y))," +
                   "\(formatFloat(e.x)),\(formatFloat(e.y))"
        }
    }
}

// MARK: - Helpers

private func parsePoint(_ s: String) -> CGPoint? {
    let parts = s.split(separator: ",")
    guard parts.count >= 2,
          let x = Double(parts[0]),
          let y = Double(parts[1]) else { return nil }
    return CGPoint(x: CGFloat(x), y: CGFloat(y))
}

private func formatPoint(_ p: CGPoint) -> String {
    "\(formatFloat(p.x)),\(formatFloat(p.y))"
}

/// Clamp to [0,1] then emit with enough precision to round-trip
/// cleanly against desktop's parser. 6 decimals matches desktop's
/// serialisation precision closely enough for visual parity.
private func formatFloat(_ v: CGFloat) -> String {
    let clamped = max(0, min(1, Double(v)))
    // %g drops trailing zeros and uses scientific form for very
    // small / large — neither of which is a concern for 0..1
    // normalised coords. 6 sig figs matches desktop.
    return String(format: "%g", clamped)
}
