# Cross-phase follow-ups

Small items left over from phases that otherwise landed. No new
phase home; catalogued here so they don't fall off.

## Phase E — Sequence management polish

- **Sequence Settings → Data Layers tab.** Image-data layers
  authoring. Lowest priority — deferred until someone actually
  uses them on iPad.

- **Add-alias on missing-model prompt** (desktop landed
  2026-05-01, `e1b90a0fd`). Desktop's `SeqElementMismatchDialog`
  now offers an "Add as alias" checkbox so users can resolve a
  missing-model load by aliasing the requested name to an
  existing model. iPad currently silently drops missing models on
  load — the access-reprompt sheet pattern (`AccessRepromptSheet`)
  is the right shape to copy: add a `MissingModelAliasSheet` that
  presents missing names with an alias-target picker, persist via
  `Model::AddAlias()` through a new bridge. P2.

## Phase F — Window system polish

- **Display Elements filter** (desktop landed 2026-04-28,
  `188387c7e`). Desktop's Edit Display Elements sheet now has a
  search filter above the Available list — large shows with
  hundreds of models / groups / timings make scroll-and-eye
  picking tedious. iPad's `DisplayElementsSheet` should mirror
  with a `.searchable`-driven filter on the Available column;
  clear automatically when items are moved into the view. S effort.

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

- **Animated GIF → Pictures effect migration** (desktop landed
  2026-04-27, `6e9e50211`). Desktop now auto-detects animated
  GIFs sitting in Video effects on sequence load and converts
  them to Pictures effects (PicturesEffect plays GIF frames
  natively). The detection helper `MediaCompatibilityIssue::isAnimatedGif()`
  already lives in `src-core/`; the migration logic itself is in
  `src-ui-wx/import_export/SeqFileUtilities.cpp` (~200 lines). To
  bring this to iPad: lift the migration into core (so both
  clients call the same path) and either run it on sequence load
  or surface as a one-tap "Convert to Pictures" action in the
  Media Manager when an `isAnimatedGif` entry shows up in the
  broken / unsupported list. M effort.

## Phase J — Layout editor

- **Model download + import.** Desktop's Model menu offers
  "Download Models" (curated catalog from xlights.org / share
  community) and "Import Custom Model" (.xmodel files including
  multi-model groups). Both are common entry points for new shows
  — far less manual building. iPad needs both:
  - **Download.** Reuse the existing desktop catalog endpoint
    (`Model -> Download Models`). Bridge: list / fetch / install
    methods on `XLSequenceDocument`. UI: a `.sheet`-based browser
    triggered from the Add Model menu (or a separate "Download"
    button next to it), search + category filter, install button
    persists to the show's model directory and refreshes the
    sidebar. Needs an authenticated `URLSession` for any
    behind-login content.
  - **Import .xmodel.** File picker (UIDocumentPickerViewController
    wrapping the existing `Model::Import` core path) lets users
    pull in `.xmodel` files from iCloud / Files / AirDrop. Bridge
    + a sheet that previews the model before commit. `.xmodel`
    multi-model groups need each entry placed individually with
    layout-group assignment.
  - Both need to register destination files via
    `ObtainAccessToURL` so subsequent loads survive app restart.
  - P1 — gates many new-user onboarding flows (downloading
    Matt Brown / Steve Hopkins / etc. shared shows).
