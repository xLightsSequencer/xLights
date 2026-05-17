# Handle / Gizmo System — Remaining Work

**Status:** R-1 through R-10 (descriptor pipeline, hit-test
decoupling, `optional<Id>` state, DrawHandles rewrite,
right-click + property panel migration) landed 2026-05-09..11.
The descriptor pipeline owns hover/click/drag/draw on both clients.

What's left is the SpaceMouse 6-DOF migration and the legacy-int
cleanup that gates on it, plus low-priority DrawHandles polish.

---

## Phase R-8b — DrawHandles polish (low priority)

- **`active_axis` / `axis_tool` as members** — still consumed by
  `DrawAxisTool` directly. Plan called for descriptor-driven
  dispatch in `DrawAxisTool` but that's a shared helper across
  all subclasses; refactoring gates on first wanting it. See
  `Boxed::DrawHandles` proof-of-concept comment.
- **Per-subclass `DrawHandles` walks** — Boxed walks
  `GetHandles()` for 2D. 3D still has body-specific draw code
  (the wireframe is appropriate as a per-subclass concern).
  TwoPoint/ThreePoint/PolyPoint/Terrain partially consume
  descriptors but still build positions inline in places.

## Phase R-9 — SpaceMouse 6-DOF migration (deferred)

User has SpaceMouse hardware off-site; testing requires it. Plan:

- Add `BeginSpaceMouseSession(modelName, handle::Id)` returning
  a `DragSession` variant that takes 6-DOF updates instead of
  ray-based ones.
- Migrate the single `LayoutPanel::OnPreviewMotion3D` caller of
  `MoveHandle3D(scale, int handle, rot, mov)` to the new session.
- Delete `BaseObject::MoveHandle3D(scale, ...)`,
  `ModelScreenLocation::MoveHandle3D(scale, ...)`, and the
  `GetHandlePosition(int)` adapter that the SpaceMouse path is
  the last consumer of.

## Phase R-10b — Legacy-constant cleanup (blocked on R-9)

- `LayoutPanel::OnPreviewMotion3D` (the 6-DOF handler) still
  reads `GetActiveHandle()` as int to feed `MoveHandle3D`.
  Until R-9 replaces `MoveHandle3D` with a SpaceMouse-flavoured
  `DragSession`, this is the last external `GetActiveHandle()`
  caller.
- After R-9 lands and `GetActiveHandle()` /
  `GetHandlePosition(int)` lose their last callers, we can
  delete `HandleIdToLegacyHandle`, every `*_HANDLE` legacy
  constant (`CENTER_HANDLE`, `L_TOP_HANDLE`, `HANDLE_CP0/1`,
  `HANDLE_AXIS`, `HANDLE_SEGMENT`, `HANDLE_MASK`, etc.), and
  the int-taking `SetActiveHandle(int)` virtual + every subclass
  override. Significant internal cleanup — many descriptor-side
  `id.index = CENTER_HANDLE` constructions need replacement, and
  `PolyPoint::SetSelectedHandle(int)` still uses
  `HANDLE_CP0/CP1` bit-packing for curve-control handles.

## Deliberately kept legacy surface

| Legacy entry point | Why kept |
|---|---|
| `ModelScreenLocation::DrawHandles` per-subclass overrides | Drawing is still per-subclass; Boxed walks GetHandles for 2D, others still build in-place. Tightening is polish, not blocking. |
| `MoveHandle3D(scale, glm::vec3& rot, glm::vec3& mov)` | SpaceMouse 6-DOF. Targeted by **R-9**. |
| `active_axis`, `axis_tool` on ModelScreenLocation | Consumed by `DrawAxisTool` directly. Refactoring gates on shared-helper descriptor-driven dispatch (R-8b polish). |
| `saved_size`, `saved_scale`, `saved_rotate` on ModelScreenLocation | SpaceMouse baseline state. Die with R-9. |
| `HandleIdToLegacyHandle` adapter + every `*_HANDLE` constant | Right-click menu + property panel read int views via the three int-typed accessors (`GetActiveHandle()`, `GetSelectedHandle()`, `GetHandlePosition(int)`). Die with R-9 + R-10b. |
| `SetActiveHandle(int)` virtual + subclass overrides | Legacy int callers (model-init paths). Sunset with R-10b along with `*_HANDLE` constants. |

---

## Open questions (still live)

- **Per-frame descriptor cost.** `GetHandles(viewMode)` runs every
  frame for the selected model. Should be cheap; if profile shows
  it isn't, the descriptor stream can cache and invalidate via the
  existing `IncrementChangeCount` signal.
- **3D gizmo styling parity.** Desktop's gizmo is opinionated (red /
  green / blue arrows, orange centre, yellow on hover, purple from-base,
  red locked). Descriptors carry `suggestedColor`; eventually iPad
  might want a different visual language. Frontend-local mapping
  keeps this open without core changes.
- **Hover state.** `MouseOverHandle` lives on the screen location
  today, sets `highlighted_handle`, drives the
  `xlYELLOWTRANSLUCENT` highlight. Desktop's mouse-move sees it;
  iPad doesn't have hover. The new API should let the frontend
  set hover state per descriptor without touching the model.
- **Programmatic mutation API.** Bulk operations (align, distribute,
  scripting) currently use `MoveHandle3D(scale, handle, rot, mov)`.
  Replace with explicit `Model::ApplyOffset(vec3)` /
  `Model::ApplyRotation(...)` rather than going through `BeginDrag`.
