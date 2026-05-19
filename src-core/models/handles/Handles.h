#pragma once

// Handle / drag-session subsystem. See
// `plans/handle-system-refactor.md` for the full design.
//
// This file defines the pure-data layer:
//   - `handles::Id`           — stable identifier for one handle
//   - `handles::Descriptor`   — drawable / hit-testable handle snapshot
//   - `handles::DescriptorStream` — what `Model::GetHandleStream()` returns
//
// `handles::DragSession` lives in `DragSession.h`. Front-end
// hit-testing utility lives in `HitTest.h`.
//
// wx-free; lives in src-core so both the desktop wx UI and the iPad
// SwiftUI client link the same definitions.

#include <compare>
#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

namespace handles {

// ---------------------------------------------------------------
// Role — what kind of handle this is. Drives drawing style + which
// drag math the session implementation runs.
// ---------------------------------------------------------------
enum class Role : uint8_t {
    Body,           // tap to select — the model's bounding shape
    Move,           // drag to translate (e.g. centre handle in 2D)
    ResizeCorner,   // 4 (2D) or 8 (3D, with z-corners) on Boxed
    ResizeEdge,     // mid-edge handles where applicable
    Rotate,         // BoxedScreenLocation's offset rotation handle
    Endpoint,       // TwoPoint START / END
    Vertex,         // PolyPoint vertex
    CurveControl,   // PolyPoint Bézier control point on a segment
    Segment,        // PolyPoint segment between two vertices — line hit-test
    Shear,          // ThreePoint shear / arch height
    AxisArrow,      // 3D translate gizmo arrow
    AxisCube,       // 3D scale gizmo cube
    AxisRing,       // 3D rotate gizmo ring
    CentreCycle,    // 3D centre sphere — tap to cycle TRANSLATE/SCALE/ROTATE
};

// ---------------------------------------------------------------
// Axis — only meaningful for AxisArrow / AxisCube / AxisRing.
// (Unlike the legacy `MSLAXIS` enum, there is no `NO_AXIS` value;
// callers either have an axis or they use a non-axis Role.)
// ---------------------------------------------------------------
enum class Axis : uint8_t { X = 0, Y = 1, Z = 2 };

// Shift an axis one step. Used by 3D rotate gizmos: the user
// grabs the cube one step ahead of the axis they rotate around
// (RED-X cube → rotate around Y, etc.). Mirrors the legacy
// `ModelScreenLocation::NextAxis` convention.
inline Axis NextAxis(Axis a) {
    switch (a) {
        case Axis::X: return Axis::Y;
        case Axis::Y: return Axis::Z;
        case Axis::Z: return Axis::X;
    }
    return Axis::X;
}

// ---------------------------------------------------------------
// Tool — the active "what does the gizmo do" mode. Frontend-owned;
// passed to `Model::GetHandleStream()` so the model only emits the
// gizmo handles relevant to the current tool.
// ---------------------------------------------------------------
enum class Tool : uint8_t {
    Translate,
    Scale,
    Rotate,
    XYTranslate,    // ThreePoint shear-handle special
    Elevate,        // Y-axis-only gizmo
};

// ViewParams — frontend-supplied parameters that affect where
// gizmo handle descriptors land in world space. Mirrors the
// (zoom, scale) inputs the legacy `GetAxisArrowLength` /
// `GetAxisRadius` use so the descriptor positions match the
// visible gizmo across the camera's full zoom range. Defaults
// give the minimum-size gizmo (60 unit arrow, 4 unit radius).
struct ViewParams {
    float axisArrowLength = 60.0f;
    float axisHeadLength  = 12.0f;     // Length of the arrow head (cone tip).
    float axisRadius      = 4.0f;
};

// ---------------------------------------------------------------
// ViewMode — `GetHandleStream()` parameter so the model can return
// the right handle set for 2D vs 3D.
// ---------------------------------------------------------------
enum class ViewMode : uint8_t { TwoD, ThreeD };

// ---------------------------------------------------------------
// Modifier flags — keyboard modifiers active during a drag.
// `DragSession::Update` takes one of these. Bitfield so multiple
// can apply simultaneously.
// ---------------------------------------------------------------
enum class Modifier : uint8_t {
    None    = 0,
    Shift   = 1 << 0,
    Control = 1 << 1,
    Option  = 1 << 2,   // alt / option
    Command = 1 << 3,   // cmd / meta
};
inline Modifier operator|(Modifier a, Modifier b) {
    return static_cast<Modifier>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}
inline bool HasModifier(Modifier flags, Modifier check) {
    return (static_cast<uint8_t>(flags) & static_cast<uint8_t>(check)) != 0;
}

// ---------------------------------------------------------------
// Id — stable identifier for one handle on one model.
//
// Replaces the legacy bit-packed `int handle` returns that mixed
// `HANDLE_AXIS`, `HANDLE_SEGMENT`, `HANDLE_CP0/1` flags with a plain
// numeric index in the lower 20 bits. The frontend stores `Id`
// values between events and hands them back to the model verbatim.
//
// Comparable + hashable so frontends can keep them in containers.
// ---------------------------------------------------------------
struct Id {
    Role role     = Role::Body;
    int  index    = 0;     // Vertex/Endpoint: 0..N-1; ResizeCorner: 0..7; AxisArrow: ignored (axis encodes)
    Axis axis     = Axis::X;  // Only meaningful for axis* roles
    int  segment  = -1;    // PolyPoint CurveControl: which segment; -1 otherwise

    auto operator<=>(const Id&) const = default;
    bool operator==(const Id&) const = default;
};

// ---------------------------------------------------------------
// Descriptor — one handle's worth of "everything the frontend needs
// to draw and hit-test it." Pure data; `GetHandleStream()` produces
// a `vector<Descriptor>` per call.
//
// Drawing reads from this. Hit-testing (in `handles::HitTest`)
// reads from this. Neither writes to it. The `Model` does not keep
// a persistent copy — descriptors are recomputed each call.
//
// `isActive` and `isHovered` are EXCLUDED from this; those are
// frontend-owned UI state, not model state. The frontend overlays
// them when rendering.
// ---------------------------------------------------------------
struct Descriptor {
    Id        id;
    glm::vec3 worldPos       {0.0f}; // World-space centre (or first endpoint for Role::Segment)
    glm::vec3 endPos         {0.0f}; // Role::Segment: second endpoint. HitTest uses point-to-line-segment
                                      //                 distance instead of point-to-point. Otherwise unused.
    float     suggestedRadius{0.0f}; // World units the model would draw at scale=1, zoom=1 — frontend may scale up for touch
    bool      editable       {true};  // false if locked / fromBase / shared with parent
    bool      selectionOnly  {false}; // handle is hit-testable for selection but not draggable. CreateDragSession
                                      // returns nullptr; frontend translates the descriptor id into a SetActiveHandle
                                      // call. Used in 3D for sphere-style sub-handle picks (CENTER/START/END/Vertex etc.).
};

} // namespace handles
