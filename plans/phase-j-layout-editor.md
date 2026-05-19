# Phase J — Layout Editor (iPad)

**Status:** J-0 → J-32 ✓ 2026-05-07..16, J-30 SubModel editor
parity ✓ 2026-05-18. The Layout Editor is a full-screen detachable
window opened from Tools → Edit Layout… with: tap-to-select (2D + 3D),
descriptor-pipeline-driven selection rendering, drag-to-move +
per-type handle drag (2D + 3D), rubber-band multi-select, grid +
bounding-box overlays, snap-to-grid, keyboard nudge, layout undo,
per-type property pages for 26 model types, model-group CRUD with
drag-to-reorder + AddMemberSheet, sidebar tabs for Models / Groups /
Objects, a Controllers tab + Controllers Visualize wiring view, a
custom-model visual editor with SubModel geometry editing, Faces /
States / Dimming Curve / SubModels editors, DMX deep authoring, and
view-object editing including a terrain heightmap editor.

### J-30 — SubModel editor parity (2026-05-18)

Mirrored the desktop `SubModelsDialog` action surface on iPad
(`src-iPad/App/LayoutEditorView.swift` `SubModelDetailEditor` +
`src-iPad/App/SubmodelPreviewPane.swift`):

- **Embedded parent-model preview** with pinch / pan / double-tap
  reset, fit-to-window in single-model mode. Active row's nodes
  paint white; the rest paint dark grey, matching desktop's
  `SelectRow` visual treatment.
- **Tap a node** on the preview → toggles it into the active row.
- **Two-finger long-press + drag** rectangle → adds every enclosed
  node to the active row as a compressed range.
- **Row management**: per-line Move Up / Move Down / Delete /
  Reverse / Sort, plus whole-submodel Reverse Nodes and Reverse
  Line Order. Mirrors desktop button cluster.
- **Animation playback** with Play / Stop + speed + trail
  steppers; walks the head + fading trail through strands in
  declaration order.
- **Sub-buffer visual editor** — labelled 0..100 box with
  draggable corners + body for non-ranges submodels; serializes
  to the `x1,y1,x2,y2` percentage string. Double-tap resets.
- **Generate Slices** sheet (Nodes mode only — splits the parent's
  nodes into N consecutive slices).
- **Copy submodel** swipe action on the list (` Copy` /
  ` Copy N` name dedupe).
- **Aliases editor** swipe action — list + add / delete /
  reorder, persisted via `setSubmodelAliases:onParent:submodel:`.
- New bridge surface on `XLMetalBridge`:
  `nodeNearPoint:onModel:viewSize:forDocument:`,
  `nodesInRect:onModel:viewSize:forDocument:`,
  `setSubmodelHighlightedNodes:onModel:forDocument:`,
  `clearSubmodelHighlightsOnModel:forDocument:`,
  `setSuppressChannelUpdate:` (skips per-frame
  `SetModelColors` so node-colour overrides stay visible). And
  on `XLSequenceDocument`: `nodeCount(forModel:)`,
  `setSubmodelAliases(onParent:submodel:aliases:)`,
  `submodelAliases(onParent:submodel:)`.

Deferred from this pass (low priority — text-only / file-format
work — moved to the "Out of scope" list below):

- **Non-"Nodes" Generate modes** (Vertical / Horizontal Slices,
  Segments 2×/3× Wide/High) — need `GetDefaultBufferWi/Ht` bridged
  and per-cell range math. Low frequency on iPad; users hitting
  these can fall back to desktop or hand-author.
- **Import / Export** (CSV, xModel, custom model, faces/states
  XML, SuperStar) — requires UIDocumentPicker integration and
  parser ports.
- **Output To Lights live preview** while editing — desktop only;
  iPad has no `OutputManager` plumbing on the editor sheets yet.
- **Custom-model background-image overlay** in the node picker —
  desktop NodeSelectGrid feature with limited iPad need given the
  embedded live preview.

---

## Remaining work

### Authenticated vendor downloads

`VendorBrowserSheet` / `XLVendorCatalog` route through
`CachedFileDownloader`'s anonymous libcurl path
(`src-iPad/App/VendorBrowserSheet.swift` →
`src-iPad/Bridge/XLVendorCatalog.mm:145` →
`src-core/utils/CachedFileDownloader`). No credential / cookie /
URLSessionConfiguration hookup exists. Open whenever the catalog
starts gating models behind accounts.

### Controllers tab — list-management polish

J-31 / J-32 shipped the Controllers tab inside the Layout Editor
(roster + filter + descriptor-driven property pane + Add Ethernet
/ Serial / Null + Discover sheet + swipe-to-delete + read-only
Visualize wiring view). What remains on top of that baseline:

| # | Item | Severity |
|---|---|---|
| O-3 | Controller list polish — multi-select (drag-reorder shipped). Sort-by-6-fields was deliberately dropped: list order drives start-channel assignment, and a stray tap on a sort option silently shuffles every model's channels with no undo — too easy to wreck a show. | P1 |
| O-4 | Toolbar — FPP Connect entry + Delete All (Add Ethernet/Serial/Null + Discover shipped J-31; FPP Connect itself tracked in [`future-controller-upload.md`](future-controller-upload.md)) | P1 |
| O-5 | Per-controller menu — Activate / Activate xLights Only / Inactivate and Unlink from Base shipped in `controllerContextMenu`; swipe-to-delete + Open + Upload + Visualize already there. **Remaining:** Upload Output is the action `pendingUploadConfirmName` already triggers — confirm the wording matches desktop. | P3 |
| O-7 | Discover sheet extras — HTTP scan (Falcon, Pixlite16, Twinkly) + DiscoveryAuthDialog credentials (Bonjour + broadcast + mismatch-resolution shipped J-31) | P4 |
| O-8 | ControllerConnectionDialog (legacy add wizard) | P2 |
| ~~O-9~~ | ~~IPEntryDialog (IP entry helper)~~ — desktop class is orphaned (`#include`d in `xLightsMain.cpp:83` but never instantiated; no call sites in git history). Skip until a concrete use case appears. | — |
| O-13 | Output-to-Lights status / fault notifications when a controller drops | P2 |
| O-16 | LED status column / async ping thread per controller | P2 |
| O-2 | Show-folder section UI extras — path display + change, Recent show folders, Base Show Folder (Change/Clear/Auto-Update/Update Now), in-flow Reselect prompt on stale bookmark (manual + auto-update-on-open via deferred `FolderConfig.pendingBaseDirReselectMessage`) all shipped via `FolderConfigView`. **Remaining:** Swift-side per-item Yes/No/All conflict-resolution sheet so the manual Update can promote locally-created clashes (currently silent, equivalent to desktop's `prompt=false`). Needs sync-from-bg-thread DispatchSemaphore + SwiftUI confirmationDialog wiring around a concrete `UICallbacks` subclass. | P3 |

### Delete-controller hardening (landed)

- Roster lists (Controllers, Groups, Objects) no longer use
  `Button(role: .destructive)` inside `.swipeActions` — iOS 26
  pre-commits the row removal to the underlying
  `UICollectionView` when a destructive swipe button is tapped,
  so an alert-gated delete that the user then cancels leaves the
  view's section count off-by-one from the data source and the
  next swipe trips `NSInternalInconsistencyException` ("invalid
  number of items in section 0 … 0 inserted, 1 deleted"). Plain
  `Button { … } .tint(.red)` keeps the visual cue without the
  pre-commit.
- Delete-controller confirmation alert now leads with the
  assigned-model count ("N models are assigned to this
  controller and will need re-assignment to render"), pulled
  through the existing `modelNamesForController:` bridge.
- `Model::DeleteController` / `ModelManager::DeleteController`
  (new in `src-core/models/`) clear any `_controllerName` that
  matches the deleted controller and zero out any
  `!<name>:<channel>` start-channel reference, so dependent
  models end up fully unassigned instead of stranded on a
  missing controller. Both the desktop deleters
  (`xLightsFrame::DeleteSelectedControllers`,
  `OnButtonControllerDeleteClick`) and the iPad bridge
  (`XLSequenceDocument deleteController:`) call it before
  `OutputManager::DeleteController`.

---

## Out of scope (S-pro / future)

These ride in [`future-layout-editing.md`](future-layout-editing.md)
or sibling `future-*.md` files:

- Custom-model authoring advanced ops. Lives in
  [`future-custom-models.md`](future-custom-models.md).
- DXF / STL / VRML export.
- ImportPreviewsModelsDialog.
- ViewObject ergonomics on the sidebar list — multi-select +
  drag-reorder (VO-6). Per-type editing for Image / Gridlines /
  Mesh / Terrain / Ruler all shipped.
- Controller upload + Pixel Test — lives in
  [`future-controller-upload.md`](future-controller-upload.md).
- Drag-drop port mapping authoring (O-10) — lives alongside the
  per-model wiring diagram (WV-1) in
  [`future-custom-models.md`](future-custom-models.md).
- The PR #6311 controller-source-tree.
