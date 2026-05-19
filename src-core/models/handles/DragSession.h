#pragma once

// `DragSession` — explicit per-gesture state object that replaces
// the persistent `active_handle` / `active_axis` / `axis_tool` /
// `saved_*` fields on `ModelScreenLocation`. See the design at
// `plans/handle-system-refactor.md` for context.
//
// Lifecycle:
//   1. Frontend hit-tests via `handles::HitTest()` to identify a
//      `Id`. (Or accepts that no handle was hit and ignores the
//      gesture.)
//   2. Frontend calls `Model::BeginDrag(id, startWorldPoint)`.
//      Returns `unique_ptr<DragSession>` or nullptr if the model
//      isn't editable (locked / fromBase / submodel proxy).
//   3. Frontend calls `session->Update(currentWorldPoint, mods)`
//      on every gesture-changed event.
//   4. Frontend calls `session->Commit()` on gesture-ended
//      (returns dirty-field info for save), or `session->Revert()`
//      on cancel.
//
// No `latch` parameter — it's implicit in `BeginDrag` capturing
// the start state. No `active_*` shared state across gestures.

#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "Handles.h"

namespace handles {

// Which model fields a `DragSession` mutated. Returned by
// `Commit()` so the frontend knows what to save / what to dirty.
// Bit flags so a single drag can affect multiple fields.
enum class DirtyField : uint16_t {
    None        = 0,
    Position    = 1 << 0,    // worldPos_x/y/z
    Dimensions  = 1 << 1,    // width / height / depth (BoxedScreenLocation scale)
    Rotation    = 1 << 2,    // rotatex/y/z
    Endpoint    = 1 << 3,    // TwoPoint x2/y2/z2
    Vertex      = 1 << 4,    // PolyPoint vertex move
    Curve       = 1 << 5,    // PolyPoint curve control point move
    Shear       = 1 << 6,    // ThreePoint shear / height / angle
};
inline DirtyField operator|(DirtyField a, DirtyField b) {
    return static_cast<DirtyField>(static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
}
inline bool HasDirty(DirtyField flags, DirtyField check) {
    return (static_cast<uint16_t>(flags) & static_cast<uint16_t>(check)) != 0;
}

// Returned by `Update()` so the frontend can avoid redundant work
// (no point rebuilding the property panel if the update was a
// no-op due to e.g. clamped scale).
enum class UpdateResult : uint8_t {
    Unchanged,   // Update was a no-op (e.g. delta below clamp threshold)
    Updated,     // Position / dimensions / rotation changed
    NeedsInit,   // Caller should call Setup() to rebuild node positions (PolyPoint vertex move, ThreePoint shear-angle)
};

// World-space ray. Frontend builds these from the touch / mouse
// position + camera ProjView matrix. Used as input to
// `DragSession::Update` — the session intersects the ray with
// whatever constraint plane its handle implies (X-axis arrow →
// XZ or XY plane, etc.) to derive the drag delta. Passing the
// raw ray (rather than a "world point") preserves the constraint
// geometry for axis-locked / plane-locked drags.
struct WorldRay {
    glm::vec3 origin    {0.0f};
    glm::vec3 direction {0.0f}; // expected unit length
};

// Abstract base. One DragSession instance lives for one user
// gesture (mouse-down → mouse-up, or touch-down → touch-up).
class DragSession {
public:
    virtual ~DragSession() = default;

    // Apply an incremental update. `currentRay` is the world-
    // space ray through the user's pointer this frame.
    // Implementations intersect the ray with their handle's
    // constraint plane and compute the delta from the start state
    // captured by `BeginDrag`.
    virtual UpdateResult Update(const WorldRay& currentRay,
                                Modifier modifiers) = 0;

    // Roll back any mutations applied during this session. For
    // gesture cancel or undo. The caller's "before drag" undo
    // entry should still apply too — `Revert()` is for the case
    // where the frontend wants to abort mid-drag without leaving
    // intermediate dirty state.
    virtual void Revert() = 0;

    // Commit the cumulative mutation. Returns the dirty-field
    // bitfield + the model's name so the frontend can mark the
    // right rows in its dirty set.
    struct CommitResult {
        std::string modelName;
        DirtyField  dirty = DirtyField::None;
    };
    virtual CommitResult Commit() = 0;

    // The handle this session was started on. Frontend uses this
    // to drive UI feedback (highlight the dragged handle, show
    // axis label, etc.).
    [[nodiscard]] virtual Id GetHandleId() const = 0;
};

} // namespace handles
