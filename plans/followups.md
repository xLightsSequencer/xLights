# Cross-phase follow-ups

Small open items left over from phases that otherwise landed.

## Landed fixes

- **MetalShaderEffect interop data race (crash sig `5d9f29a77c`,
  2026-05-29).** ShaderEffect renders on many worker threads at once
  (per-model render jobs + per-sub-buffer `parallel_for`). On
  `USE_GLES`, `GLContextManager::ExecuteOnGLThread` runs inline on the
  calling thread, and the size-1 GL context pool does *not* serialize
  the Metal-side interop calls (`getBytes` / `replaceRegion` / texture
  creation don't need the GL context current). Multiple threads were
  touching the single shared ANGLE EGLDisplay + Metal device — one in
  `createSharedTexture` while others were in `copyPixelDataFromTexture`
  — crashing in `createSharedTexture`
  (`src-core/effects/metal/MetalShaderEffect.mm:104`). Fix: a
  file-scope `static std::mutex sMetalInteropMutex` taken across the
  full body of `preparePixelTextures`, `copyPixelDataToTexture`,
  `copyPixelDataFromTexture`, and the cache destructor's
  `destroySharedTexture` calls. Follow-up option if lock contention
  ever shows: make `ExecuteOnGLThread` funnel to one GL thread on
  `USE_GLES` like the Windows path, removing the whole class of
  ANGLE-Metal threading bugs.

## 2026.10 desktop deltas — iPad parity triage

Reviewed every desktop commit in the 2026.10 cycle (since the
`2026.09` tag) for iPad parity. Most landed in `src-core/` and apply
to both clients automatically (Text effect multi-line color-per-word,
Shockwave end-time clamp, the hidden-timing-track fix, and the
render-engine crash/guard hardening). One UI gap was cheap enough to
close in the same review:

- **"Hide Unused Submodels"** is now on the iPad row-header context
  menu (`setHideUnusedSubmodels` / `hideUnusedSubmodels` bridge over
  the existing `SequenceElements::SetHideUnusedSubmodels`).
- **Per-Model Default group render style (#4125)** now ships on iPad
  too: added `perModelDefault` to the `layoutStyleOptions` list in the
  bridge's `modelGroupLayoutSummary` and a pretty-name case in
  `LayoutEditorView`. The J-5/J-7 group property editor's Layout-style
  picker already round-trips arbitrary `layout` strings through
  `setLayoutModelGroupProperty` → `ModelGroup::SetLayout`, and the
  render half is shared core (`ModelGroup` / `PixelBuffer`).
- **Duplicate "Include Submodels" (#6419)** — confirmed working on
  iPad: render half is core (`RenderEngine`) and the new checkbox
  rides in on `effectmetadata/Duplicate.json` via the metadata-driven
  Duplicate panel.

Already covered on iPad — no work needed:

- **Model-pane count badges (#6202).** The iPad J-18 `extrasFor:`
  bridge already surfaces Faces / States / SubModels / Strands / Nodes
  (counts via the entry arrays, plus tap-to-view) in the layout
  model-property editor.

The desktop-only items below are tracked but not yet worth their own
phase:

- **SelectPanel sort-by-time (#6389).** Enhancement to a panel iPad
  hasn't ported — folds into AP-3 in
  [`future-aux-panels.md`](future-aux-panels.md).
- Desktop-only with no iPad surface: EffectPreset restore option
  (backup/restore dialog), and the Layout-tab editing improvements
  (copy/paste placement, Bulk Edit Rotate X/Y/Z, 3D-object handle
  picking, multi-object pivot) — already covered by
  [`future-layout-editing.md`](future-layout-editing.md).

## 2026-05-31 desktop deltas — iPad parity triage

Reviewed the desktop commits from the last 48h. Most touch desktop-only
surfaces (layout editor start-channel/replace dialogs, controller
mDNS, log packaging, dark-mode, scripts, BatchRender Freeze/Thaw) or
are pure `src-core/` effect/render changes that render identically on
iPad (States-on-SubModels `FacesEffect`/`StateEffect`/`SubModel`, the
Sketch description `D<hex>` token round-trip, crash guards). Three
landed on the iPad side in the same review:

- **xmodel import: preserve individual + @/> start channels (#6447).**
  Desktop's `GetXlightsModel` was updated so importing a model with
  individual start channels (moving-head strands) or a model-relative
  `@Model:chan` / `>Model:chan` reference no longer forces
  `NO_CONTROLLER` (which would auto-reassign and wipe the reference).
  The iPad import (`XLMetalBridge.mm` `importXmodelFromPath…`) was
  calling `SetControllerName(NO_CONTROLLER, true)` *unconditionally* —
  it had neither the original `HasIndividualStartChannels()` exemption
  nor the new @/> one. Added the same guard at both the primary and
  multi-model-sibling call sites.
- **Multi-model xmodel: preserve relative positions (#6438).** Desktop
  `FinalizeModel` now keeps the author's relative WorldPos layout
  instead of spreading siblings horizontally. The iPad sibling loop
  still used the old `BATCH_PLACEMENT_PADDING` horizontal spread (the
  exact code desktop replaced). Reworked it to anchor the primary at
  the touch point (unchanged iPad idiom) and offset each sibling by its
  file-space `WorldPos` delta from the primary, so relative layout is
  preserved. Reads `WorldPosX/Y/Z` straight off each `<model>` node
  (deserialization overwrites position with the default).
- **Media relink: split basename on both / and \\.** Desktop
  `ManageMediaPanel` was fixed to extract a basename treating both
  separators so Windows-authored `B:\…\f.png` paths resolve on
  macOS/Linux. iPad's `findMediaFileForInfo`
  (`XLSequenceDocument.mm`) had the identical defect — it used
  `std::filesystem::path::filename()`, which on iOS leaves `\` as an
  ordinary character, so the media-folder basename match never hit for
  Windows paths (the `FixFile` fallback only covers the show dir).
  Now splits on `"/\\"` like the desktop helper.

Desktop-only / N/A:

- **Rename preview updates pick lists + View menu (#6430).** N/A
  today: iPad can *create* a preview/layout group
  (`LayoutEditorView` `newPreviewSheet` → `createLayoutGroup` →
  `AddNamedLayoutGroup`, and it already refreshes `layoutGroups()` +
  auto-switches afterward) but has **no rename** path at all. When
  iPad gains preview rename, mirror the desktop fix: after the rename,
  refresh every preview/layout-group picker (the layout-group picker
  sheets, the active-group selector, and any model-property
  layout-group field) the same way `createLayoutGroup` already does,
  so stale names don't linger in the pick lists.
- **BatchRenderDialog Freeze/Thaw optimization.** wx-only redraw
  suppression around the checklist box; SwiftUI batches view updates
  automatically, so there is no analog to port even though iPad has a
  batch-render surface.
- Layout-editor-only (start-channel relative preservation UI side,
  multi-target Replace-Model dialog, multi-model import position UI):
  covered by [`future-layout-editing.md`](future-layout-editing.md)
  except the import-position behavior already handled above.

## Phase E — Sequence management polish

- **Sequence Settings → Data Layers tab.** Image-data layers
  authoring. Lowest priority — deferred until someone actually
  uses them on iPad.

## Phase C — Effect Settings Inspector polish

- **G3+ — Moving Head full waypoint path authoring.** Path tab
  currently shows the existing `Path:` value with a Clear action.
  Sketch-style drag waypoint authoring still requires desktop's
  Effect Assist panel.

- **G2-c — Shader dynamic uniform grouping for large `.fs`
  files.** Most shaders declare < 10 uniforms so grouping isn't
  needed; packs with 20+ turn into a flat scroll. Respect
  `GLSL_GROUP:` comment conventions in
  `ShaderConfig::GetDynamicPropertiesJson()` so grouping carries
  across. Deferred until a real shader pack trips the issue. P2.

## Handle / gizmo system

- **R-8b leftover — descriptor-driven `DrawAxisTool` dispatch.**
  `DrawAxisTool` and `DrawActiveAxisIndicator` still read
  `axis_tool` / `active_axis` / `highlighted_handle` members
  directly. Inline-position math in the per-subclass `DrawHandles`
  is closed out (descriptors are now the source of truth for
  Vertex / CurveControl / Shear / Terrain-vertex positions), but
  the axis-gizmo helper itself hasn't been ported to a descriptor
  stream. Gates on the first concrete consumer that wants to vary
  per-axis styling (colour, locked state, hover) without touching
  the model. Low priority.
