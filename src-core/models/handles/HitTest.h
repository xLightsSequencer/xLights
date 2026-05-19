#pragma once

// Hit-test utility — given a flat list of handle descriptors,
// pick the one closest to a screen-space point. Lives in
// src-core/handles/ so both desktop wx and iPad SwiftUI link the
// same logic with their input-device-specific tolerance. See
// `plans/handle-system-refactor.md`.
//
// wx-free.

#include <optional>
#include <vector>

#include "Handles.h"

namespace handles {

// Tolerances are in SCREEN points (or pixels — caller picks the
// unit and is consistent with the camera projection it passes).
// Touch UIs want larger values; mouse UIs are happy with tighter
// ones. The same descriptors work for both.
struct HitTestOptions {
    float handleTolerance     = 4.0f;   // mouse default; touch should be ~24-40
    // Axis handles (X/Y/Z arrows, cubes, rings) project to fixed
    // screen positions near the model centre, so a generous touch
    // slop on them swallows nearby body-drag taps. The 0-default
    // means "use handleTolerance for everything"; touch callers
    // should set this tighter (e.g. 24-28) than `handleTolerance`.
    float axisHandleTolerance = 0.0f;
    bool  preferAxisHandles   = true;   // axis arrows beat body-shaped handles
                                        // when both are within tolerance
    bool  ignoreNonEditable   = false;  // skip descriptors with editable=false
};

// Project worldPoint to screen coords using the supplied
// projection-view matrix + viewport. Returned (x, y) shares units
// with the screen point passed to `HitTest`.
struct ScreenProjection {
    glm::mat4 projViewMatrix;
    int       viewportWidth;
    int       viewportHeight;
};

// Result of a successful HitTest. `selectionOnly` is the picked
// descriptor's flag — surfaced here so callers don't need to
// re-scan the descriptor list to recover it.
struct Hit {
    Id   id;
    bool selectionOnly = false;
};

// Pick the closest handle to `screenPoint` (in same units as
// viewport size), or return nullopt if none are within tolerance.
// Implementation is naive O(N) over the handle list — there are
// rarely more than a few dozen handles per model so a spatial
// index is overkill.
std::optional<Hit> HitTest(
    const std::vector<Descriptor>& handles,
    const ScreenProjection& projection,
    glm::vec2 screenPoint,
    const HitTestOptions& opts = {});

} // namespace handles
