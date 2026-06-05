#pragma once

// `SpaceMouseSession` — 6-DOF input adapter for the SpaceMouse
// gesture. Distinct from `DragSession` because the input is
// per-frame delta vectors (rotation + translation around the
// three world axes) rather than a world ray, and there's no
// commit / revert lifecycle — each frame is independent and the
// LayoutPanel's existing CreateUndoPoint per event handles
// rollback granularity.
//
// Replaces `ModelScreenLocation::MoveHandle3D(scale, int, rot,
// mov)`, with `Id` (not legacy int) identifying which handle the
// SpaceMouse is currently acting on. The session is constructed
// once when the user begins manipulating a model and reused for
// every frame thereafter — captured baseline scale / rotation /
// position lives inside the session, not on the screen location.

#include <memory>
#include <optional>
#include <glm/glm.hpp>

#include "Handles.h"

namespace handles {

// Outcome of a single Apply() frame. Lets the caller queue the
// right output-model-manager work item without re-checking which
// handle was being driven.
enum class SpaceMouseResult : uint8_t {
    Unchanged,    // delta below clamp / locked handle / unsupported axis
    Dirty,        // geometry changed → save rgbeffects + redraw
    NeedsInit,    // geometry changed AND node positions must be rebuilt
                  // (PolyPoint vertex / curve move, ThreePoint shear)
};

class SpaceMouseSession {
public:
    virtual ~SpaceMouseSession() = default;

    // Apply a single 6-DOF frame.
    //   `scale`        — pixels-per-unit scaling derived from the
    //                    preview's zoom and screen DPI (caller-
    //                    computed). Used to convert the unitless
    //                    SpaceMouse delta into world units.
    //   `rotations`    — per-axis rotation deltas in driver units
    //                    (radians-per-second × frame dt).
    //   `translations` — per-axis translation deltas in driver
    //                    units (world-units-per-second × dt).
    //
    // The returned `SpaceMouseResult` tells the frontend whether
    // the mutation needs persisting (`Dirty`) and whether the
    // model also needs a `Setup()` rebuild of its node positions
    // (`NeedsInit`).
    virtual SpaceMouseResult Apply(float scale,
                                    const glm::vec3& rotations,
                                    const glm::vec3& translations) = 0;

    // The handle this session is operating on. Stays constant for
    // the lifetime of the session — callers that want to act on a
    // different handle should drop the session and Begin a new
    // one.
    [[nodiscard]] virtual std::optional<Id> GetHandleId() const = 0;
};

} // namespace handles
