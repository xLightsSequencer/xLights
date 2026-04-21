# Future — Effect Presets on iPad

Not in the first-pass Phase C scope. Captured here so it isn't lost.

## Gap

**G12 — Effect presets (save / load named bundles).** Desktop ships
`EffectPresetManager` + an effect-tree UI that lets users save a
full effect + palette + buffer + blending settings bundle under a
friendly name and re-apply it to new effects. iPad has no save /
load preset UI. Presets round-trip through a file format the
desktop already reads, so the iPad work is purely:

- Bridge method to list existing presets from `xLights/presets/`.
- Bridge method to read the full effect settings string at a
  preset path + apply it to the currently-selected effect.
- SwiftUI sheet: preset tree, Save-As, Apply, Delete.
- "Save Effect as Preset…" entry in the inspector's overflow menu
  or long-press on an effect bar in the grid.

## Why deferred

- Not day-to-day blocking — users primarily pick effect types from
  the palette and tweak per-instance. Presets are a power-user
  workflow.
- The file-format plumbing is already shared via desktop; no core
  changes needed when we get to it. Cleanly layered on top of the
  current inspector.

Re-scope into Phase C (or a dedicated follow-up phase) when first-
pass C lands and real users ask for it.
