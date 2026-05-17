# Future — iPad layout editing (S-pro)

Phase S of the 2026-04-23 gap analysis was promoted to active work
2026-05-07 and shipped as [phase-j-layout-editor.md](phase-j-layout-editor.md).
This file collects the **S-pro** items that remain future work.

S-pro scope (from the gap analysis): LA-5 3D canvas (gizmo,
gestures, no SpaceMouse), LA-6 right-click menus, LA-8 alignment,
LA-12 bulk-edit (15+ ops), LA-13 ImportPreviewsModelsDialog,
LA-15 DXF/STL/VRML export, VO-3 MeshObject, VO-4 TerrainObject,
VO-5 RulerObject, VO-6 ViewObjectPanel.

Most of these landed inside Phase J. What's left:

- **LA-13 ImportPreviewsModelsDialog** — copy model layout from
  another show.
- **LA-15 DXF / STL / VRML export** — dumps the current layout
  to a CAD file.
- **VO-3 MeshObject** advanced controls — texture + brightness +
  mesh-only flag editing.
- **VO-5 RulerObject** — singleton; length + units; 2-point line.
- **VO-6 ViewObjectPanel** ergonomics — multi-select / drag-
  reorder / unlink-from-base.
- **Bulk edit / wiring view extras** — per-model strand wiring
  diagram (WV-1) tracked in
  [`future-custom-models.md`](future-custom-models.md).

Custom model authoring, Face/State editing, DMX deep authoring, and
the Wiring view live in [`future-custom-models.md`](future-custom-models.md).
