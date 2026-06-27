# Layout: Model Sets (linked-translation)

**Status:** implemented on desktop (branch `model-sets`); iPad parity pending
(tracked in `plans/ipad-parity/06-layout-models-preview.md`).
Decisions made during implementation: name prompt on Link-as-Set
(auto-name pre-filled, in-place collision re-prompt); Manage dialog =
filterable checkbox list with editable name field; right-click shows a
single "Set" submenu; Alt/Option-drag repositions a single member; a Set
containing a locked model is frozen (no member moves); 3D axis-arrow
translate and axis-ring rotate propagate to the Set; scale stays
per-model.
**Issue:** [#3703](https://github.com/xLightsSequencer/xLights/issues/3703)
**Author:** heffneil
**Target release:** 2026.11+

## Background

xLights has two existing "grouping" concepts on the Layout tab:

- **Model Group** (`src-core/models/ModelGroup.h`, stored under
  `<modelGroup>` in `xlights_rgbeffects.xml`) — a *sequencer* aggregation
  used to route effects to multiple models with one drop. Does NOT
  constrain layout positions.
- **Multi-selection** — a transient set of selected models that translate
  together while you drag. Once you click away, the relationship is gone.

Issue [#3703](https://github.com/xLightsSequencer/xLights/issues/3703)
asks for a *third*, distinct concept: a **persistent positional
relationship** between props, so that e.g. an EFL tree and its star always
translate together as one logical prop, even after deselecting. The user
explicitly wants this kept separate from sequencer Groups (different word,
different XML element, no effect-routing side effects).

The chosen name is **Set** (or "Model Set"). Word "group" is reserved for
the existing sequencer concept.

## The rule

**Translation propagates across Set membership. Everything else does not.**

| User action | Behavior |
|---|---|
| Click a prop that belongs to a Set | That single prop is selected. Normal property-grid editing, normal resize/rotate handles, normal everything. |
| Click + drag any Set member | Drag delta is applied to **every** member. The relative arrangement of members is preserved. |
| Resize corner / rotate handle on a Set member | Operates on that one prop only. Reshape is per-prop; Sets exist to preserve translation invariants, not geometry. |
| Property-grid edit on a Set member | Applies to that one prop only. Sets do not propagate property changes. |
| Right-click a Set member | Menu includes: **Add to Set…** / **Remove from Set** / **Manage Set…** |
| Right-click a multi-selection of 2+ props | Menu includes: **Link as Set** (creates new auto-named Set, or merges into an existing Set if any member is already in one). |

There is **no visual indicator** (no bracket, no bbox highlight) when a
Set is selected. The user knows the prop is in a Set because they put it
there; the right-click menu and the per-prop property grid both surface
"belongs to Set: `Set 3`" for confirmation.

A prop belongs to **at most one Set**. Adding a prop to a Set while it
already belongs to another prompts the user: *"`<prop>` is already in
`<Set X>`. Move it to `<Set Y>`?"* — Yes / Cancel.

## Goals

1. Persistent translation lock between any user-chosen set of props.
2. Survives save / reload (lives in `xlights_rgbeffects.xml`).
3. Rotation around the Set's bounding-box centroid when applied to a
   Set (via the rotation handle or via Bulk Edit Rotate — see #6521).
4. Works in 2D and 3D layout.
5. iPad app: parity ships in a follow-up PR (this doc is desktop-first).

## Non-goals

- Scaling a Set as a unit (defer — most users don't want this).
- Property propagation across Set members (effects, dimming, pixel
  style, etc. — all per-prop, by design).
- A new top-level "Composite Model" type with rendering/effect surface
  of its own — overkill for this feature.
- Bundling images or physical-dimension metadata into the Set (that's
  what #3825 covers, already resolved).
- A dedicated "Sets" tree-pane row — canvas right-click is the only
  management UI for v1. (Can be added later if users ask.)

## Design — `<modelSet>` XML element

A new lightweight XML element, sibling to `<modelGroup>`, stored in
`xlights_rgbeffects.xml`:

```xml
<modelSets>
    <modelSet name="Set 1" models="Tree,Star" />
    <modelSet name="Set 2" models="Wreath,Bow,Ribbon" />
</modelSets>
```

The element holds **only** a name and a comma-separated list of member
model names. No coordinates of its own — each member retains its own
absolute coords exactly as today. The Set is a *constraint*, not a
container.

Default name is `Set N` where N is the lowest unused integer. User can
rename via the right-click → Manage Set… dialog (or via the property
grid surface mentioned below).

### Why a new element instead of extending `<modelGroup>`

| Approach | Pros | Cons | Verdict |
|---|---|---|---|
| Extend `ModelGroup` with `_moveLocked` flag | Reuses existing aggregation, minimal new code | Conflates sequencer-group and positional-link in one object. Users who want a positional link but no sequencer aggregation get one anyway (or vice versa). Word "group" already overloaded. | **Rejected** |
| Parent/anchor link on each Model | Compact, no new top-level structure | Per-model field; chain depth unbounded; "anchor" model gets implicit special status which users find confusing | **Rejected** |
| New `<modelSet>` sibling element | Clean separation of concerns. Models themselves unchanged. Sequencer paths untouched. New name = no terminology conflict. | One new top-level structure to serialize / load / migrate | **Chosen** |

Models themselves get **zero new attributes**. The Set knows its members;
the members don't know they're in a Set (except via a runtime lookup
helper).

### Runtime API

A new `ModelSetManager` (or fold into existing `ModelManager`):

```cpp
class ModelSetManager {
public:
    ModelSet* GetSetContaining(const std::string& modelName) const;
    std::vector<std::string> GetMemberModels(const ModelSet* set) const;
    ModelSet* CreateSet(const std::vector<std::string>& members);
    void AddToSet(ModelSet* set, const std::string& modelName);
    void RemoveFromSet(const std::string& modelName);  // also deletes the set if it falls below 2 members
    void DeleteSet(ModelSet* set);
    void RenameSet(ModelSet* set, const std::string& newName);

    void Load(wxXmlNode* rgbEffects);
    void Save(wxXmlNode* rgbEffects);
};
```

Living in `src-core/models/`. iPad picks this up automatically via
`src-core/` linkage.

### Drag-time propagation

In `LayoutPanel::OnPreviewMouseMove`, the existing drag loop at
`LayoutPanel.cpp:6011-6023` iterates models with `Selected() ||
GroupSelected()` and applies the delta. The minimal new piece:

- On `OnPreviewLeftDown`, when a model is clicked and added to selection,
  check `ModelSetManager::GetSetContaining(model)`. If non-null, mark
  every other Set member as `GroupSelected = true`. The existing drag
  loop now naturally translates them all.
- On `OnPreviewMouseUp` (drag end), clear the `GroupSelected` flags that
  were set by Set propagation (so single-click after drag still selects
  one prop for property editing).

This is intentionally tiny — Sets piggyback on machinery that already
exists for multi-selection drag.

### Rotation around the Set centroid

When the rotation handle is used on a Set member, or when `BulkEditRotateZ`
runs on a selection that maps to a Set, compute the union bbox centroid
of all Set members, then rotate each member's center around that point
while also setting `loc.SetRotateZ(angle)` on each. Reuse the
`BulkEditRotateAxis` plumbing from #6521; the only new piece is the
"translate around centroid" step.

Phase 1 can ship without centroid rotation (rotation-handle on a Set
member rotates only that member, same as today); centroid rotation is
Phase 3.

### Right-click affordances

**On a single prop:**

- *not in any Set:* — no Set-related items in the menu.
- *in Set X:* — submenu **Set 'X' →** with:
  - *Remove from Set*
  - *Rename Set…*
  - *Delete Set*
  - *Show Set Members* (highlights all members on the canvas briefly)

**On a multi-selection of 2+ props:**

- If none of the selection is in a Set: **Link as Set** → creates new
  auto-named `Set N`.
- If some are already in Set X and others aren't: **Add to Set 'X'** →
  pulls the loose ones in. Conflict prompt if multiple Sets are
  represented.
- If all are in the same Set: **Remove from Set** / **Delete Set**.

### Property-grid surface

When a single Set member is selected, the property grid shows an extra
read-only-ish field:

```
Set                Set 3   [Manage…]
```

The `[Manage…]` button opens the same dialog as the right-click *Manage
Set…* item (member add/remove, rename, delete).

This satisfies "I need a way to edit the set" without adding a dedicated
tree pane.

## Implementation phases

Each phase is a self-contained PR.

| # | PR | Scope | Approx size |
|---|---|---|---|
| 1 | **Data + persistence + Manage dialog** | `ModelSet` + `ModelSetManager` in `src-core/models/`. `<modelSets>` XML load/save in `xlights_rgbeffects.xml`. Right-click → Link as Set / Manage Set… / Remove from Set. Manage Set… dialog (member list, rename, delete). NO drag behavior change yet — Sets are creatable and persistable but don't do anything on drag. | ~400 LOC |
| 2 | **Drag-time propagation** | In `LayoutPanel::OnPreviewLeftDown`, propagate `GroupSelected` to other Set members when one is clicked. Clear on mouse-up. Set membership conflict prompt on Add-to-Set. | ~150 LOC |
| 3 | **Centroid rotation** | When rotating a Set member (via handle or `BulkEditRotateZ`), rotate around the union centroid instead of per-member origin. | ~200 LOC |
| 4 | **Property-grid surface + polish** | `Set: Set 3 [Manage…]` row on selected member's property grid. Set name shown in tooltip. Undo grouping: a Set drag is one undo step, not one-per-member. | ~150 LOC |
| 5 | **iPad parity** (separate PR, after desktop ships) | `ModelSetManager` is already in `src-core/`, so the data is automatic; iPad's `SequencerViewModel` and drag handlers honor Set membership; long-press-drag moves the whole set. | TBD |

## Risks

- **Undo granularity.** A drag of a Set must be one undo step covering
  all members, not one-per-member. The existing
  `CreateUndoPoint("SingleModel", ...)` at `LayoutPanel.cpp:6014` fires
  per drag start — fine — but the label should read `LockedSetMove` or
  similar so the undo label is meaningful.
- **Membership conflict at load.** If a project file lists a model in
  two different `<modelSet>` elements (corruption, manual editing, merge
  artifacts), the loader picks the first occurrence and logs the
  conflict — no crash, no silent data loss.
- **Renaming a model.** When a model is renamed via the Layout tree,
  every Set that references it by name needs to update. Existing
  rename plumbing in `ModelManager` walks ModelGroup references; we
  hook the same point.
- **Deleting a model.** Same — strip from all Sets, delete the Set
  itself if it drops below 2 members.
- **File-format compatibility.** Older xLights opening a project with
  `<modelSets>` simply ignores the unknown element — no crash, Sets
  become invisible until reopened with current xLights. Forward-compat
  is free.
- **Performance.** Set membership lookup happens on every left-click on
  the canvas. With a `std::unordered_map<modelName → setPtr>` cache
  rebuilt on Set mutation, lookup is O(1). Negligible.

## Open questions for review

1. **Property-grid `[Manage…]` button placement.** Top of the model's
   own property panel, or in a new "Set" category section at the bottom?
2. **"Show Set Members" indicator.** Brief flash / outline highlight? Or
   add a momentary bracket bbox just for the "show me" affordance (since
   we agreed no persistent indicator)?
3. **Phase 2 mouse-up cleanup.** Should `GroupSelected` propagation
   persist after a click (so the user sees the Set highlighted as a
   multi-selection in the tree until they click elsewhere)? Or revert
   immediately on mouse-up so the visible selection matches "single
   prop"? I lean toward reverting — matches the "no special handle / no
   persistent indicator" rule — but worth confirming.

## Acceptance criteria

- [ ] Select two props, right-click → Link as Set. Save and reload the
      project. Dragging either prop translates both by the same delta.
- [ ] Resizing a prop in a Set affects only that prop.
- [ ] Editing a prop's property (e.g. pixel size) affects only that prop.
- [ ] Right-click → Remove from Set un-links a single prop. Set itself
      persists if 2+ remain; auto-deletes if it falls to 0 or 1.
- [ ] Right-click → Delete Set removes the Set; members revert to
      independent props with their current absolute positions intact.
- [ ] Renaming a model updates every Set that references it.
- [ ] Deleting a model strips it from any Set it belonged to.
- [ ] Adding a prop to a Set while it's in another Set prompts the user
      to confirm the move.
- [ ] Older xLights opens the same project without crashing or losing
      non-Set data.
- [ ] One undo step rolls back a Set drag.
- [ ] iPad app opens the project without errors (parity in a follow-up
      PR; must not crash).
