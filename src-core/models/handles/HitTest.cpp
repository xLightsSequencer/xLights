#include "HitTest.h"

#include <cmath>
#include <limits>

#include <glm/glm.hpp>

namespace handles {

namespace {

// Project a world-space point to screen coords. Returns nullopt
// if the point is behind the camera (clipped), since hit-testing
// shouldn't snap to handles the user can't see.
std::optional<glm::vec2> WorldToScreen(const glm::vec3& world,
                                       const ScreenProjection& proj) {
    glm::vec4 clip = proj.projViewMatrix * glm::vec4(world, 1.0f);
    if (clip.w <= 0.0f) return std::nullopt; // behind camera
    glm::vec3 ndc = glm::vec3(clip) / clip.w;
    // No near/far clamp here. The 2D layout uses `glm::ortho(0, w,
    // 0, h)` with the default `[-1, 1]` z range, but models live at
    // real-world Z far outside that — clamping would drop every
    // visible handle. Behind-camera is already filtered above.
    // NDC (x,y) is [-1, 1]. Map to viewport pixel coords.
    glm::vec2 screen;
    screen.x = (ndc.x * 0.5f + 0.5f) * static_cast<float>(proj.viewportWidth);
    // Flip Y to UIKit convention (top-left origin). Caller is
    // expected to pass `screenPoint` in the same convention.
    screen.y = (1.0f - (ndc.y * 0.5f + 0.5f)) * static_cast<float>(proj.viewportHeight);
    return screen;
}

} // namespace

std::optional<Hit> HitTest(
    const std::vector<Descriptor>& descriptors,
    const ScreenProjection& projection,
    glm::vec2 screenPoint,
    const HitTestOptions& opts) {

    std::optional<Hit> best;
    // -ffast-math (set on the desktop Release build) implies
    // -ffinite-math-only, under which the optimizer is allowed to
    // assume no operand is infinity. With -O3 + LTO that turns this
    // sentinel into 0 and the first valid hit fails `distSq < bestDistSq`
    // — silently dropping every non-axis click. Use a finite sentinel.
    float bestDistSq = std::numeric_limits<float>::max();
    bool  bestIsAxis = false;
    bool  bestIsSegment = false;

    for (const auto& d : descriptors) {
        if (opts.ignoreNonEditable && !d.editable) continue;
        auto projected = WorldToScreen(d.worldPos, projection);
        if (!projected) continue;
        float distSq;
        if (d.id.role == Role::Segment) {
            // Point-to-line-segment distance in screen space.
            // Both endpoints projected; fail if either is clipped.
            auto end = WorldToScreen(d.endPos, projection);
            if (!end) continue;
            const glm::vec2 a  = *projected;
            const glm::vec2 b  = *end;
            const glm::vec2 ab = b - a;
            const float lenSq  = ab.x * ab.x + ab.y * ab.y;
            float t = 0.0f;
            if (lenSq > 0.0f) {
                t = glm::dot(screenPoint - a, ab) / lenSq;
                t = glm::clamp(t, 0.0f, 1.0f);
            }
            const glm::vec2 closest = a + t * ab;
            // Endpoint deadzone: skip the segment when the closest
            // point is within handleTolerance of either endpoint
            // (otherwise the segment line "wraps around" the vertex
            // and steals hits from the Vertex descriptor whenever
            // the click is slightly off the vertex along the
            // segment direction).
            const float endTolSq = opts.handleTolerance * opts.handleTolerance;
            const glm::vec2 da = closest - a;
            if (da.x * da.x + da.y * da.y < endTolSq) continue;
            const glm::vec2 db = closest - b;
            if (db.x * db.x + db.y * db.y < endTolSq) continue;
            const glm::vec2 dvec = closest - screenPoint;
            distSq = dvec.x * dvec.x + dvec.y * dvec.y;
        } else {
            const glm::vec2 dvec = *projected - screenPoint;
            distSq = dvec.x * dvec.x + dvec.y * dvec.y;
        }
        bool isAxis = (d.id.role == Role::AxisArrow ||
                       d.id.role == Role::AxisCube  ||
                       d.id.role == Role::AxisRing);
        // Axis handles get their own (typically tighter) tolerance
        // so they don't swallow body-drag taps when their head
        // projects near the model surface. Falls back to the
        // general handleTolerance when axisHandleTolerance is 0.
        const float effectiveTol = (isAxis && opts.axisHandleTolerance > 0.0f)
            ? opts.axisHandleTolerance
            : opts.handleTolerance;
        if (distSq > effectiveTol * effectiveTol) continue;

        // Prefer axis-style handles when configured. They typically
        // overlap with the body of the model in screen space, so
        // without this preference body-pick would frequently win.
        bool isSegment = (d.id.role == Role::Segment);
        if (opts.preferAxisHandles) {
            if (isAxis && !bestIsAxis) {
                // axis trumps non-axis regardless of distance
                best = Hit{d.id, d.selectionOnly};
                bestDistSq = distSq;
                bestIsAxis = true;
                bestIsSegment = false;
                continue;
            }
            if (!isAxis && bestIsAxis) continue; // never demote
        }

        // Point-style handles always trump segments at the same
        // distance — segments cover an entire line so they almost
        // always tie or beat a coincident point handle, which makes
        // discrete handles unreachable when a segment passes
        // through them.
        if (isSegment && bestIsSegment == false && best.has_value()) {
            // A non-segment is already best; never demote to a
            // segment regardless of distance.
            continue;
        }
        if (!isSegment && bestIsSegment) {
            // Promote: any non-segment hit replaces the current
            // segment best, regardless of distance ordering.
            best = Hit{d.id, d.selectionOnly};
            bestDistSq = distSq;
            bestIsAxis = isAxis;
            bestIsSegment = false;
            continue;
        }

        if (distSq < bestDistSq) {
            best = Hit{d.id, d.selectionOnly};
            bestDistSq = distSq;
            bestIsAxis = isAxis;
            bestIsSegment = isSegment;
        }
    }
    return best;
}

} // namespace handles
