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
