# Phase J — Layout Editor (iPad)

**Status:** J-0 → J-32 ✓ 2026-05-07..16. The Layout Editor is a
full-screen detachable window opened from Tools → Edit Layout…
with: tap-to-select (2D + 3D), descriptor-pipeline-driven
selection rendering, drag-to-move + per-type handle drag (2D + 3D),
rubber-band multi-select, grid + bounding-box overlays,
snap-to-grid, keyboard nudge, layout undo, per-type property pages
for 26 model types, model-group CRUD with drag-to-reorder +
AddMemberSheet, sidebar tabs for Models / Groups / Objects, a
Controllers tab + Controllers Visualize wiring view, a custom-model
visual editor with SubModel geometry editing, Faces / States /
Dimming Curve / SubModels editors, DMX deep authoring, and
view-object editing including a terrain heightmap editor.

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
| O-3 | Controller list polish — drag-reorder + sort-by-6-fields + multi-select | P1 |
| O-4 | Toolbar — FPP Connect entry + Delete All (Add Ethernet/Serial/Null + Discover shipped J-31; FPP Connect itself tracked in [`future-controller-upload.md`](future-controller-upload.md)) | P1 |
| O-5 | Per-controller right-click — Activate / Activate xLights Only / Inactivate / Unlink / Upload Output / Sort submenu (swipe-to-delete shipped) | P1 |
| O-7 | Discover sheet extras — HTTP scan (Falcon, Pixlite16, Twinkly) + DiscoveryAuthDialog credentials (Bonjour + broadcast + mismatch-resolution shipped J-31) | P4 |
| O-8 | ControllerConnectionDialog (legacy add wizard) | P2 |
| O-9 | IPEntryDialog (IP entry helper) | P2 |
| O-13 | Output-to-Lights status / fault notifications when a controller drops | P2 |
| O-16 | LED status column / async ping thread per controller | P2 |
| O-2 | Show-folder section UI extras — Recent show folders list + base-directory toggle (path display + change already shipped via `FolderConfigView`) | P2 |

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
