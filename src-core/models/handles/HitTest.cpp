#include "HitTest.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

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

struct Ray {
    glm::dvec3 origin;
    glm::dvec3 dir; // unit length
};

// Inverse of WorldToScreen: the world-space pick ray through a screen
// point. Done in double — the far-plane unprojection loses too much
// precision in float with large show coordinates.
std::optional<Ray> ScreenToWorldRay(glm::vec2 screen, const ScreenProjection& proj) {
    if (proj.viewportWidth <= 0 || proj.viewportHeight <= 0) return std::nullopt;
    const double nx = 2.0 * screen.x / proj.viewportWidth - 1.0;
    const double ny = 1.0 - 2.0 * screen.y / proj.viewportHeight;
    const glm::dmat4 inv = glm::inverse(glm::dmat4(proj.projViewMatrix));
    const glm::dvec4 nearH = inv * glm::dvec4(nx, ny, -1.0, 1.0);
    const glm::dvec4 farH  = inv * glm::dvec4(nx, ny,  1.0, 1.0);
    if (nearH.w == 0.0 || farH.w == 0.0) return std::nullopt;
    const glm::dvec3 nearP = glm::dvec3(nearH) / nearH.w;
    const glm::dvec3 dir   = glm::dvec3(farH) / farH.w - nearP;
    const double len = glm::length(dir);
    if (!(len > 0.0)) return std::nullopt;
    return Ray{ nearP, dir / len };
}

// Coarse reject before the cone math.
bool RayHitsSphere(const Ray& ray, const glm::dvec3& centre, double radius) {
    const glm::dvec3 oc = centre - ray.origin;
    const double tc = glm::dot(oc, ray.dir);
    const double distSq = glm::dot(oc, oc) - tc * tc;
    return distSq <= radius * radius && tc + radius >= 0.0;
}

// Nearest ray distance (t >= 0) at which the ray meets the solid cone
// with the given apex, base centre and base radius — side or base cap.
std::optional<double> RayConeHit(const Ray& ray, const glm::dvec3& apex, const glm::dvec3& base, double radius) {
    const glm::dvec3 axis = base - apex;
    const double height = glm::length(axis);
    if (!(height > 0.0) || !(radius > 0.0)) return std::nullopt;
    const glm::dvec3 v = axis / height;
    const double cos2 = (height * height) / (height * height + radius * radius);

    std::optional<double> best;
    auto keep = [&](double t) {
        if (t >= 0.0 && (!best || t < *best)) best = t;
    };

    // Side: infinite double cone, then clip to the apex→base span
    // (which also discards the mirrored nappe behind the apex).
    const glm::dvec3 co = ray.origin - apex;
    const double dv  = glm::dot(ray.dir, v);
    const double cov = glm::dot(co, v);
    const double a = dv * dv - cos2;
    const double b = 2.0 * (dv * cov - glm::dot(ray.dir, co) * cos2);
    const double c = cov * cov - glm::dot(co, co) * cos2;
    auto keepSide = [&](double t) {
        const double along = cov + t * dv;
        if (along >= 0.0 && along <= height) keep(t);
    };
    constexpr double kEps = 1e-12;
    if (std::abs(a) > kEps) {
        const double disc = b * b - 4.0 * a * c;
        if (disc >= 0.0) {
            const double s = std::sqrt(disc);
            keepSide((-b - s) / (2.0 * a));
            keepSide((-b + s) / (2.0 * a));
        }
    } else if (std::abs(b) > kEps) {
        keepSide(-c / b); // ray parallel to a generator line
    }

    // Base cap.
    if (std::abs(dv) > kEps) {
        const double t = glm::dot(base - ray.origin, v) / dv;
        const glm::dvec3 p = ray.origin + ray.dir * t - base;
        if (glm::dot(p, p) <= radius * radius) keep(t);
    }
    return best;
}

// Nearest ray distance (t >= 0) at which the ray meets the solid
// axis-aligned cube with the given centre and half-side (slab test).
std::optional<double> RayCubeHit(const Ray& ray, const glm::dvec3& centre, double half) {
    if (!(half > 0.0)) return std::nullopt;
    double tNear = 0.0;
    double tFar  = std::numeric_limits<double>::max();
    for (int i = 0; i < 3; ++i) {
        const double o = ray.origin[i] - centre[i];
        const double d = ray.dir[i];
        if (std::abs(d) < 1e-12) {
            if (std::abs(o) > half) return std::nullopt; // parallel and outside this slab
            continue;
        }
        double t1 = (-half - o) / d;
        double t2 = ( half - o) / d;
        if (t1 > t2) std::swap(t1, t2);
        tNear = std::max(tNear, t1);
        tFar  = std::min(tFar, t2);
        if (tNear > tFar) return std::nullopt;
    }
    return tNear;
}

// Exact pick of the drawn gizmo shapes (arrowhead cones, tip cubes,
// handle spheres).
// Returns the one nearest the camera under the cursor, if any.
std::optional<Hit> HitTestShapes(const std::vector<Descriptor>& descriptors,
                                 const ScreenProjection& projection,
                                 glm::vec2 screenPoint,
                                 const HitTestOptions& opts) {
    std::optional<Ray> ray;
    bool rayBuilt = false;
    std::optional<Hit> best;
    double bestT = std::numeric_limits<double>::max();
    for (const auto& d : descriptors) {
        if (d.hitShape == HitShape::Point) continue;
        if (opts.ignoreNonEditable && !d.editable) continue;
        if (!rayBuilt) {
            ray = ScreenToWorldRay(screenPoint, projection);
            rayBuilt = true;
        }
        if (!ray) return std::nullopt;
        // worldPos is the shape's centre, so it is also the centre of
        // the bounding sphere used for the coarse reject.
        const glm::dvec3 mid(d.worldPos);
        const double r = d.suggestedRadius;
        std::optional<double> t;
        if (d.hitShape == HitShape::Cone) {
            // worldPos is the midpoint of the head, so base = 2*mid - apex.
            const glm::dvec3 apex(d.endPos);
            const glm::dvec3 base = 2.0 * mid - apex;
            const double halfLen = 0.5 * glm::length(apex - base);
            if (!RayHitsSphere(*ray, mid, std::sqrt(halfLen * halfLen + r * r))) continue;
            t = RayConeHit(*ray, apex, base, r);
        } else {
            if (!RayHitsSphere(*ray, mid, r * std::sqrt(3.0))) continue;
            t = RayCubeHit(*ray, mid, r);
        }
        if (t && *t < bestT) {
            best = Hit{ d.id, d.selectionOnly };
            bestT = *t;
        }
    }
    return best;
}

} // namespace

std::optional<Hit> HitTest(
    const std::vector<Descriptor>& descriptors,
    const ScreenProjection& projection,
    glm::vec2 screenPoint,
    const HitTestOptions& opts) {

    // The cursor is on a drawn gizmo shape: that wins outright.
    if (auto shape = HitTestShapes(descriptors, projection, screenPoint, opts)) {
        return shape;
    }

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
        if (d.hitShape != HitShape::Point && opts.exactGizmoShapes) continue;
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
