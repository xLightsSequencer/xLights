# Phase I — Import Effects (iPad)

**Status: I-1 landed; I-2 v1.1 + timing-tracks + alias polish
landed.** Tools menu → Import Effects… opens a SwiftUI sheet that
loads `.xsq` (loose) or `.xsqz` (vendor package — extracts to
temp dir via `SequencePackage::Extract`), builds the full model /
submodel / strand / node tree on both source and destination
sides, supports tap-to-map and Auto Map at every level (model,
submodel, strand, node) with alias-driven matching that now
strips punctuation on aliases too, surfaces source timing tracks
in a popover with per-track toggles + "already exists" hints,
and applies imports through the core `EffectMapper` family.
Both desktop and iPad-lib debug builds green; include policy
clean. Remaining in I-2: model-blending toggle (niche),
LSItemContentTypes registration. Manual end-to-end regression on
a real vendor sequence is the next checkpoint.

## Why this matters

A large fraction of xLights users buy pre-sequenced shows from
vendors and map them onto their own layout — this is the single
biggest workflow that has no iPad equivalent today. Without it,
the iPad app is limited to users who sequence from scratch.

Desktop entry point: File → Import Effects (`ID_IMPORT_EFFECTS`,
handler at `src-ui-wx/xLightsMain.cpp:1251` →
`src-ui-wx/import_export/ImportEffects.cpp:232`). The mapping UI
is `xLightsImportChannelMapDialog` (3.9k lines). The whole stack
lives in `src-ui-wx/import_export/` — none of it is in `src-core/`
today.

## Scope

**v1 priorities (in order):**

1. `.xsq` / `.xsqz` (xLights-to-xLights). Most vendor-purchased
   shows ship as `.xsq` or `.xsqz` today.
2. `.sup` (SuperStar). Still in active vendor use.
3. `.lms` / `.las` (LOR). Distant third — shipped only when I-1
   through I-3 are stable.

**Mapping features in v1:** the mapping tree (model / strand /
submodel / node), tap-to-map, **Auto Map** button (alias-driven,
the desktop's two-pass `norm` + `aggressive` strategy — explicitly
*not* AI Map), **maphints** read + write (`<showdir>/maphints/
*.xmaphint` regex-driven hint files — many users already have
these set up for their favorite vendors), import-options sheet
(erase existing, lock, time-adjust, timing-track selection,
import media for embedded images).

**Maximize core reuse.** Anything that runs on desktop today
should run on iPad through the same core code path. The desktop
dialog becomes a thin wx adapter on top of the new core; the
iPad gets a SwiftUI adapter on top of the same core. Drift
between the two clients is the failure mode to avoid.

## Non-goals (v1)

- AI Map (LLM-driven). Defer until a shared iOS AI bridge exists
  — same dependency `future-ai-palette-generate.md` is waiting on.
- Drag-drop mapping. Touch uses tap-source / tap-destination.
- Vixen 2/3, LSP, VSA, HLS, LOR S5 (`.loredit`), LOR Pixel Editor
  (`.lpe`), VSA. Long-tail formats — defer.
- CCR strand mapping. Niche; can come back if users ask.
- "Convert render style" toggle. Desktop has it; uncertain whether
  vendors still ship sequences that need it. Drop from v1 unless a
  vendor conversation flags it as still relevant — easy to add
  back later since `MapXLightsEffects` already implements the
  rewrite.
- "Map CCR strand" advanced toggle.

---

## Architecture

The work is one core extraction plus three UI surfaces (desktop
re-skin, iPad UI, then SuperStar/LMS layered on top).

### New core package

Create `src-core/import_export/` (peer to existing
`src-core/render/`, `src-core/effects/`, etc.). Everything wx-bound
in import logic moves here, becoming wx-free:

```
src-core/import_export/
  EffectMapper.{h,cpp}        // MapXLightsEffects family
  ImportMappingModel.{h,cpp}  // wx-free tree (model/strand/sub/node)
  AutoMapper.{h,cpp}          // DoAutoMap + matchers (norm/aggressive/regex)
  ImportOptions.{h,cpp}       // pre-existing? confirm — already in src-core/render
  XsqImporter.{h,cpp}         // Phase I-1 — parse source .xsq → import-source list
  SuperStarImporter.{h,cpp}   // Phase I-4
  LmsImporter.{h,cpp}         // Phase I-5
```

`ImportMappingModel` replaces `xLightsImportModelNode` (which
currently subclasses `wxDataViewTreeStoreNode`). The wx
dialog rebuilds its `wxDataViewModel` from the core model on
open; iPad's SwiftUI view does the same with its own list/outline.

### Model-lookup access

`AutoMapper` needs to ask the user's layout "what aliases does
*this model* have, and does this model have submodels named X?".
Today `DoAutoMap` reaches through `xlights->GetModel(name)` against
`xLightsFrame`, but that's just a wx-bound shortcut — `ModelManager`
itself is already core. The mapper already has a `RenderContext`
in scope (`target->GetParentElement()->GetSequenceElements()->GetRenderContext()`,
see `ImportEffects.cpp:366`), and `RenderContext::GetModel(name)`
gives back a `Model*` with `GetAliases()` / `GetSubModel()` directly.

Plan: thread the existing `RenderContext&` through the mapper /
auto-mapper API instead of going through `xLightsFrame`. No new
interface needed — both desktop and iPad already create a
`RenderContext` per sequence (`xLightsFrame` on desktop,
`iPadRenderContext` on iPad), and both already own a
`ModelManager` reachable through it.

### Camera resolution (already core)

`MapXLightsEffects` reaches `xLightsApp::GetFrame()->viewpoint_mgr`
at `ImportEffects.cpp:397` to confirm a 3D camera name exists.
`ViewpointMgr` is already in `src-core/render/ViewpointMgr.{h,cpp}`
and `RenderContext::GetViewpointMgr()` already exposes it on both
platforms (the iPad wires it up in `iPadRenderContext.cpp:236`).
Replace the `xLightsApp::GetFrame()` call with the same
`RenderContext&` we're already passing for model lookup —
`renderContext.GetViewpointMgr().GetNamedCamera3D(name)`. One-line
change.

### Gotchas in extraction

- `BufferPanel::CanRenderBufferUseCamera` is referenced from the
  effect mapper. It's effectively a static helper based on buffer
  style strings — hoist to a wx-free utility (probably
  `src-core/effects/BufferStyles.h`).
- `ImportEffects.cpp` is 5.2k lines mixing dispatch, file parse,
  mapping, dialog launch, and `xLightsFrame::ImportXLights*` glue.
  Don't try to move it whole — extract the inner mapping and parse
  helpers, leave the `xLightsFrame::ImportXLights*` wrappers in
  `src-ui-wx/`.

---

## Phases

### I-1 — Core extraction + desktop refactor ✓ (modulo manual regression)

What landed:

- `src-core/import_export/` package — included in `xLights.cbp`,
  `Xlights.vcxproj`, `Xlights.vcxproj.filters`, and added to
  `ci_scripts/check_core_include_boundaries.sh`'s enforced list.
- `EffectMapper.{h,cpp}` — `MapXLightsEffects` (both overloads) +
  `MapXLightsStrandEffects` moved out of
  `src-ui-wx/import_export/ImportEffects.cpp`. The
  `xLightsFrame::ImportXLights*` wrappers stay in `src-ui-wx/` as
  the wx-side glue. The `wxASSERT(false)` defensive check became
  a `spdlog::warn` with the model name.
- `BufferStyles.h` (in `src-core/effects/`) — `CanRenderBufferUseCamera`
  hoisted from `BufferPanel`. `BufferPanel` no longer owns a
  static helper that the import path depended on.
- Camera resolution: `RenderContext::GetNamedCamera3D` was already
  on the base interface — the import path now uses it directly
  instead of reaching through `xLightsApp::GetFrame()->viewpoint_mgr`.
  Added the matching `iPadRenderContext::GetNamedCamera3D` override
  so iPad also resolves cameras correctly through the same path.
- `MapHintsIO.{h,cpp}` — `LoadMapHintsFile`,
  `LoadMapHintsFromShowDir`, `WriteMapHintsFile`. Pure pugixml +
  std::filesystem, no wx. Desktop dialog refactored to use them
  (the show-dir scan dropped from 8 lines to 1 in two places).
- `ImportMappingNode.h` — abstract interface that AutoMapper drives
  off. `xLightsImportModelNode` now inherits publicly from both
  `wxDataViewTreeStoreNode` and `ImportMappingNode`, with one-line
  override accessors (`GetCoreModel`, `GetCoreStrand`, `GetCoreNode`,
  `GetMapping`); existing public fields and methods stay put so
  the dialog's 176 direct-field-access points are untouched.
  `AvailableSource` POD carries the source list pre-resolved
  (canonical name + display name + model type + selected flag).
- `AutoMapper.{h,cpp}` — `MatchNorm`, `MatchAggressive`,
  `MatchRegex` as free functions (no wx — `std::regex` replaces
  `wxRegEx`, character-stripping rewritten without `wxString::Replace`),
  and `AutoMapper::Run` that walks the tree exactly as the desktop
  `DoAutoMap` did. The dialog's `DoAutoMap` is now a 30-line
  adapter that builds the available-source list + selection set
  from wx state and delegates to core.

What's NOT done (deliberately):

- Removing the desktop dialog's per-instance `norm` /
  `aggressive` / `regex` lambdas. They still work (passed
  through `DoAutoMap` as `std::function`s into core's `Run`), so
  there's no runtime cost — just a parallel implementation. Drift
  risk is low; clean up in a follow-up if it becomes annoying.
- Manual regression test: load a real vendor `.xsq`, exercise
  Auto Map + Auto Map Selected + a mix of formats end-to-end.
  This needs a human eyeball.

### I-2 — iPad import UI for `.xsq` / `.xsqz`

**v1 landed 2026-04-29:**

- `src-core/import_export/BasicImportMappingNode.h` — wx-free
  concrete tree node (POD-style with public fields matching
  `xLightsImportModelNode`'s shape, plus `unique_ptr` children).
- `src-iPad/Bridge/XLImportSession.{h,mm}` — complete bridge API:
  load source `.xsq` (via core `SequenceFile::Open` +
  `SequenceElements::LoadSequencerFile`, no wx), build flat
  destination tree from active sequence's models, mutating
  ops (`setMapping`, `runAutoMap`, `runAutoMapSelectedTargets`,
  `saveMapHints`, `applyImport`). Snapshot accessors (`availableSources`,
  `destinationRows`) hand SwiftUI immutable rows.
- `src-iPad/App/ImportEffectsView.swift` — two-pane sheet with
  source / destination scroll lists, tap-to-select highlighting,
  Map / Unmap / Auto Map / Save Hints… buttons, erase / lock
  toggles. File picker first; mapping panes appear once a `.xsq`
  is loaded.
- Tools menu added in `XLightsCommands.swift` — `CommandMenu("Tools")`
  with "Import Effects…" as the first entry. Placeholder for the
  long list of desktop Tools entries (Test Lights, Convert,
  Package Sequence, etc.) that will land over time.
- `showingImportEffects` flag on `SequencerViewModel`; sheet wired
  in `SequencerView.swift` next to the existing Sequence Settings
  sheet.
- `iPad-Bridging-Header.h` updated to expose
  `XLImportSession.h` to Swift.
- Apply path uses core `EffectMapper::MapXLightsEffects(Element*, ...)`
  for each mapped destination. Embedded images flow through the
  existing `SequenceMedia::AddEmbeddedImage` path inside
  `MapXLightsEffects`. After apply, `iPadRenderContext::MarkRgbEffectsChanged`
  is called and the SwiftUI view triggers `viewModel.reloadRows()`.

**v1.1 landed 2026-04-29:**

- `.xsqz` (and `.zip` / `.piz`) package extraction in
  `loadSourceSequenceAtPath:`. Mirrors desktop: build a
  `SequencePackage`, call `Extract()` if `IsPkg()`, then point
  `SequenceFile` at the inner `.xsq` returned by `GetXsqFile()`.
  The session holds the package in `unique_ptr<SequencePackage>`
  so the temp extraction dir stays valid through apply.
- Hierarchical destination tree — `buildDestinationTree` now
  recurses into `Model::GetNumSubModels/GetNumStrands/GetStrandLength`
  with each submodel pulling its own aliases, and per-strand
  nodes generated under each strand row.
- Hierarchical source available list — `rebuildAvailableSources`
  emits slash-delimited names ("Model/Strand/Node") matching the
  desktop's `AddChannel` pass, plus `_sourceElementMap` /
  `_sourceLayerMap` for apply-time resolution.
- Apply path descends into both levels of children — calls
  `MapXLightsEffects(SubModelElement*, ...)` per strand/submodel
  mapping and `MapXLightsStrandEffects(NodeLayer*, ...)` per
  node mapping. `ModelElement::GetSubModel(int)` indexed by
  child position keeps source and target aligned.
- SwiftUI `DestinationRowView` — recursive disclosure-style row
  with chevron toggles, indent-by-depth, expansion state in a
  `Set<Int>` keyed by nodeID. Tap-to-select highlights at any
  depth; Map / Unmap / Auto Map operate against the selected row.
- UTType filter accepts `.xsq` / `.xsqz` / `.zip` so vendor
  packages show up in the Files picker.

**Also landed 2026-04-29 (post-v1.1):**

- Timing-track import. Bridge surfaces a `XLImportTimingTrack`
  list (name + alreadyExists + selected). SwiftUI hangs a
  "Timing (N/M)" popover button on the toolbar showing per-
  track toggles; default selection is `!alreadyExists`. Apply
  copies each selected timing element via the desktop's
  reuse-empty-then-add-with-suffix pattern, and flows each
  layer through `MapXLightsEffects` so settings/media fixups
  run consistently. Apply also unblocks when no model
  mappings exist but timing tracks are selected.
- Alias punctuation-strip polish in `MatchAggressive`. Aliases
  with spaces/punctuation (e.g. `"Mega Tree (Vendor)"`) now
  match a punctuation-free vendor candidate
  (`"megatreevendor"`) without the user having to maintain a
  duplicate stripped alias. Applies on both desktop and iPad.

**Still pending in I-2:**

- Model-blending toggle. Niche; deferred.
- `.xsq` / `.xsqz` UTType registration in the iPad app's
  `LSItemContentTypes` so the file picker auto-recognises the
  types instead of falling back to extension matching. Needs
  Info.plist edit.
- Apply still runs with hardcoded `convertRender=false`. Per
  earlier conversation this is deferred until vendor practice
  is confirmed.
- Time-adjust spinbox is **not** an `.xsq → .xsq` v1 item — the
  desktop hides it in this flow too, only showing it on HLS /
  SuperStar / Vixen imports. Will surface naturally with those
  format ports.

**Original plan items (still relevant):**

- Add a new top-level **Tools** menu to the iPad menu bar
  (Phase F-4 wired `.commands`). Desktop's Tools menu is large
  and many of its entries will eventually want iPad equivalents
  (Test Lights, Convert, Package Sequence, Color Manager,
  Effects Settings Search, Hinksix, Generate Custom Model, etc.)
  — getting the menu placeholder in place now means future
  tool ports drop in without menu reshuffling.
- Add "Import Effects…" as the first entry under Tools. Desktop
  splits Import out as its own File submenu, but with one item
  the separate menu is empty noise — Tools is the cleaner home
  on iPad.
- New SwiftUI sheet: `ImportEffectsView` in `src-iPad/App/`.
  - Step 1: source picker. `UIDocumentPicker` for
    `org.xlights.sequence` and `org.xlights.sequence-package`.
    Run `ObtainAccessToURL` so the security-scoped bookmark
    persists.
  - Step 2: mapping view. Two-pane on iPad-regular size class
    (sidebar: source models from incoming `.xsq`; main:
    destination tree). On compact size class, segmented control
    flips between panes.
  - Step 3: options sheet (erase existing, lock, time-adjust,
    timing-track checklist, import media, "Save Map Hints…"
    button to write current mapping state to
    `<showdir>/maphints/<name>.xmaphint`).
  - Confirm → run core mapper → reload sequence elements →
    refresh grid.
- New ObjC++ bridge in `src-iPad/Bridge/`:
  - `XLImportSession.h/.mm` — wraps the core
    `ImportMappingModel` + `AutoMapper`. Exposes:
    - `loadSourceXsqAtURL:error:` (async — do parsing on a
      background queue; the `.xsqz` extraction can be slow on
      large packages).
    - mapping array + assign / unassign methods (`NS_SWIFT_NAME`
      to nice Swift names).
    - `runAutoMap` (calls core's two-pass `norm` then
      `aggressive`, then runs the regex pass for every
      `*.xmaphint` file under `<showdir>/maphints/` — same
      sequence the desktop dialog runs at line 3399).
    - `saveMapHintsToURL:` — writes the current mapping state
      out as `.xmaphint` XML using the core writer.
    - `applyImportWithOptions:` — runs `EffectMapper` against
      the loaded sequence on the document, registers undo,
      triggers grid reload.
- iPad `RenderContext` access for the mapper: the sequence
  document already owns an `iPadRenderContext`; surface it on
  `XLImportSession` so the bridge can hand it to `EffectMapper` /
  `AutoMapper`.
- For embedded images on import: piggy-back on the existing
  `SequenceMedia::AddEmbeddedImage` path that `MapXLightsEffects`
  already calls (`ImportEffects.cpp:355-361`). External media
  references are out of scope for v1 — surface a warning toast
  per unresolved external file.

### I-3 — Auto Map polish + UX testing

- Verify alias-driven matching + maphints loading on real
  vendor sequences. Concretely: load a Holiday Coro / Wally
  Wally World pack with the user's existing `.xmaphint` files
  in place, run Auto Map, count matched models. Compare to
  desktop baseline on the same sequence + layout.
- Tune Auto Map UX (e.g. scroll to first unmapped row after
  the run; surface a count of "X of Y mapped").

### I-4 — SuperStar (`.sup`)

- Hoist `.sup` parsing from `SuperStarImportDialog.cpp` to
  `src-core/import_export/SuperStarImporter.{h,cpp}`. The
  parser builds a list of available source elements; the
  mapping UI is the same one already built in I-1/I-2.
- Add `.sup` to the iPad source picker's UTType filter.
- SuperStar-specific options (the desktop dialog has a few:
  resize, blend mode) — port if straightforward; defer if not.

### I-5 — LMS / LAS

- Hoist `.lms` / `.las` parsing from
  `LMSImportChannelMapDialog.cpp` to
  `src-core/import_export/LmsImporter.{h,cpp}`.
- Add formats to the iPad picker.
- LOREdit conversion (`LOREdit.{h,cpp}`) is referenced by both
  LMS and LOR S5 paths — investigate whether to hoist with LMS
  or leave for a later S5 import phase.

---

## Risks

- **Core extraction is large.** `ImportEffects.cpp` (5.2k) and
  `xLightsImportChannelMapDialog.cpp` (3.9k) are the biggest
  files in `import_export/`. Risk is breaking desktop import
  during the refactor. Mitigation: ship I-1 in a feature branch,
  exercise every import format against a regression suite before
  merging.
- **Map-onto-group + camera resolution** crosses a tricky
  boundary (`xLightsApp::GetFrame()` reaches into
  `viewpoint_mgr`). Worth scoping early in I-1 so the
  `IModelLookup` interface gets the right shape from the start.
- **`SequencePackage` ownership on import.** The mapper uses a
  `SequencePackage` for the *source* sequence (to access its
  embedded media). On iPad, we extract the source `.xsqz` to
  the app sandbox already (Phase G pattern). Confirm
  `SequencePackage` can represent a read-only source, not just
  the active sequence.
- **Auto Map false-positives** with `aggressive` matcher on
  layouts with similar model names. Desktop users live with
  this; iPad users will have a smaller screen for review. UX
  consideration: post-Auto-Map, scroll to the first unmapped
  row by default.

## Open questions

_(none open — maphints is in v1; convert-render-style is
deferred pending a vendor conversation about whether anyone
still ships sequences that need it.)_
