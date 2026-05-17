# Cross-phase follow-ups

Small items left over from phases that otherwise landed. No new
phase home; catalogued here so they don't fall off.

## Phase E — Sequence management polish

- **Sequence Settings → Data Layers tab.** Image-data layers
  authoring. Lowest priority — deferred until someone actually
  uses them on iPad.

- ~~**Add-alias on missing-model prompt**~~ — shipped 2026-05-17.
  Bridge (`missingModelNamesWithEffects` / `resolveMissingModel:…`)
  + `MissingModelAliasSheet` mirror desktop's
  `SeqElementMismatchDialog` "Add as alias" path; rename + delete
  cover the iPad MVP, full Map-Models flow stays desktop-only.

## Phase F — Window system polish

- ~~**Display Elements filter**~~ — done 2026-05-17. Added a
  case-insensitive substring filter above the Available pane in
  both Master and user-view detail of `DisplayElementsSheet`;
  resets on add (per-row, "Add All") so cleared rows don't leave
  stale needles in the field.

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

- ~~**Shockwave Timing-Track field verification**~~. Verified +
  fixed 2026-05-02. The new `Shockwave_TimingTrack` choice and
  its `dynamicOptions: "timingTracks"` populate correctly through
  the existing `EffectPropertyView` choice path. The change ALSO
  introduced `enable`/`disable` clauses in `visibilityRules`
  (`Filter`, `Regex`, `Duration` fields grey out when no timing
  track is selected) — iPad's rule engine previously honoured
  only `show`/`hide`, silently ignoring `enable`/`disable`. Added
  `EffectSettingsView.isPropertyEnabledByRules` mirroring the
  existing `isPropertyVisible`, plus a `ruleDisabled` param on
  `EffectPropertyView` (combined with `runtimeDisabled` into a
  single `effectiveDisabled`). Applied across slider / choice /
  checkbox / spin / text branches via `.disabled(...)` +
  `.opacity(0.5)`. 16 metadata files use `enable`/`disable` (Bars,
  Pictures, Fire, Faces, Fireworks, Liquid, Morph, Meteors,
  Sketch, Shockwave, SingleStrand, Tendril, Video, Circles,
  Guitar, plus the schema) — all benefit from this single fix.

## Phase E / G — Media handling

- **Animated GIF → Pictures effect migration.** Shipped
  2026-05-17: lifted from `src-ui-wx/import_export/SeqFileUtilities.cpp`
  into `src-core/render/SeqMediaMigration.{h,cpp}`; desktop is now
  a thin wrapper and iPad runs the same migration on sequence
  open in `iPadRenderContext::OpenSequence`.

## Phase J — Layout editor

- **Model download + import.** ✓ substantially shipped
  2026-05-15..16 alongside J-23 / J-31 / J-32. The Layout Editor
  canvas overlay has three buttons
  (`src-iPad/App/LayoutEditorView.swift:1953..1982`):
  - **+ (Add Model)** opens `AddModelSheet` with the 18 built-in
    types.
  - **Import (↓↑)** runs `.fileImporter` for `.xmodel` /
    `.gdtf` / `.lff` / `.lpf`, persists the bookmark via
    `ObtainAccessToURL` (line 409), and routes through
    `XLMetalBridge.importXmodel`. Multi-model `<models>` files
    are unpacked and placed in a grid
    (`src-iPad/Metal/XLMetalBridge.mm:2363..2409`) — desktop
    PR #6365 parity.
  - **Download (cloud ↓)** opens `VendorBrowserSheet`
    (`src-iPad/App/VendorBrowserSheet.swift`), which fetches the
    xlights.org vendor index through `XLVendorCatalog` (shared
    core `vendor_catalog::Catalog`), browses vendors → models →
    wirings, downloads via `CachedFileDownloader`, and flips the
    canvas into tap-to-place mode.

  **Remaining gaps** (now tracked in
  [`phase-j-layout-editor.md`](phase-j-layout-editor.md) header):

  - Authenticated downloads. `CachedFileDownloader` is
    anonymous-only; no auth path exists for behind-login content.
  - Layout-group selection on multi-model `.xmodel` placement —
    every model lands in `GetActiveLayoutGroup()` with no
    prompt. A pre-placement sheet would close this.
