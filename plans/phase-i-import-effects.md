# Phase I — Import Effects (iPad)

**Status:** I-1 (core extraction + desktop refactor) and I-2 (iPad
UI for `.xsq` / `.xsqz`) ✓ landed 2026-04-29. Tools menu → Import
Effects… opens a SwiftUI sheet that loads `.xsq` (loose) or
`.xsqz` (vendor package via `SequencePackage::Extract`), builds
the full model / submodel / strand / node tree on both source and
destination sides, supports tap-to-map and Auto Map at every
level (alias-driven matching with punctuation-stripped aliases),
surfaces source timing tracks in a popover with per-track
toggles + "already exists" hints, saves map-hints, and applies
imports through the core `EffectMapper` family. Both desktop and
iPad-lib debug builds green; include policy clean. Next: I-3 —
Auto Map polish + manual end-to-end regression on a real vendor
sequence.

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
   shows ship as `.xsq` or `.xsqz` today. ✓ landed.
2. `.sup` (SuperStar). Still in active vendor use. (I-4)
3. `.lms` / `.las` (LOR). Distant third — shipped only when I-1
   through I-3 are stable. (I-5)

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

## Phases

### I-1 — Core extraction + desktop refactor ✓ landed 2026-04-29

`src-core/import_export/` package created (added to `.cbp` /
vcxproj / include-policy allowlist). `EffectMapper`,
`AutoMapper`, `MapHintsIO`, `ImportMappingNode`, and
`BasicImportMappingNode` all live there as wx-free code. The
desktop dialog is now a thin adapter on top of core. Camera
resolution flows through `RenderContext::GetNamedCamera3D` (with
the matching `iPadRenderContext` override). Manual end-to-end
regression on a real vendor sequence is the only outstanding
piece — see I-3.

### I-2 — iPad import UI for `.xsq` / `.xsqz` ✓ landed 2026-04-29

`XLImportSession.{h,mm}` bridges core + the SwiftUI
`ImportEffectsView`; Tools menu item wires it into
`SequencerView`. Hierarchical destination tree (model / submodel
/ strand / node) with tap-to-select; slash-delimited source list
with per-level apply via `MapXLightsEffects` and
`MapXLightsStrandEffects`. `.xsqz` / `.zip` / `.piz` packages
extract through `SequencePackage::Extract` (held in the session
so the temp dir survives apply). Timing-track import surfaces a
popover with per-track toggles (default = `!alreadyExists`).
Apply triggers `iPadRenderContext::MarkRgbEffectsChanged` and a
SwiftUI row reload.

**Carried forward as deferred (still pending):**

- Model-blending toggle. Niche.
- `.xsq` / `.xsqz` UTType registration in the iPad app's
  `LSItemContentTypes` (Info.plist edit) — file picker today
  works via extension matching.
- Apply still runs with hardcoded `convertRender=false`. Per
  earlier conversation this is deferred until vendor practice
  is confirmed.
- Removing the desktop dialog's per-instance `norm` /
  `aggressive` / `regex` lambdas. They still work (passed
  through `DoAutoMap` as `std::function`s into core's `Run`), so
  there's no runtime cost — clean up only if drift becomes
  annoying.

### I-3 — Auto Map polish + UX testing

- Verify alias-driven matching + maphints loading on real
  vendor sequences. Concretely: load a Holiday Coro / Wally
  Wally World pack with the user's existing `.xmaphint` files
  in place, run Auto Map, count matched models. Compare to
  desktop baseline on the same sequence + layout.
- Tune Auto Map UX (e.g. scroll to first unmapped row after
  the run; surface a count of "X of Y mapped").
- This phase is also where the I-1 desktop manual regression
  lives — same vendor sequence exercised through both clients.

### I-4 — SuperStar (`.sup`)

`.sup` is structurally nothing like `.xsq` — there's no source/dest
tree, no per-row mapping, no Auto Map. The user picks **one** target
model and the parser fans every morph / image / flowy (Spiral,
Shockwave, Fan) / scene / textAction / imageAction in the file out as
effects on that model's layers, with a small set of global knobs
(image-resize mode, ±timing offset, layer blend, X/Y size and
offset). The desktop dialog (`SuperStarImportDialog`) is just those
knobs; all parsing lives in `xLightsFrame::ImportSuperStar`.

iPad therefore needs a separate, much simpler import sheet — not the
`.xsq` channel-mapper.

- ✓ **Core hoist (done).** `xLightsFrame::ImportSuperStar(Element*,
  pugi::xml_document&, …)` body (~730 lines, all SuperStar-only
  helpers — `CalcPercentage`, `ImageInfo`, `ScaleImage`,
  `CreateSceneImage`, `IsPartOfModel`, `FindOpenLayer`,
  `ChannelBlend`, the in-stream XML preprocessor) lifted to
  `src-core/import_export/SuperStarImporter.{h,cpp}`. wx-free; uses
  `xlImage` instead of `wxImage`. Public surface:
  `SuperStar::Options` struct + `SuperStar::Importer` class with a
  `PrefixPromptCallback` so the wx side can keep using
  `wxTextEntryDialog` and the iPad side can pre-fill from the file
  basename. The desktop wrapper (`xLightsFrame::ImportSuperStar`)
  is now a ~30-line shim that builds Options, wires the prompt
  callback, and calls `Importer::Run()`. Covers every SuperStar
  element type the desktop importer covers (verified by porting
  faithfully, no logic rewrites).
- ✓ **iPad bridge (done).** `XLSuperStarImport.{h,mm}` in
  `src-iPad/Bridge/`. Two class methods on `XLSuperStarImport`:
  `+availableTargetModelNamesForDocument:` (lists every
  `Element` + `SubModelElement` in the active sequence) and
  `+applyImportFromPath:targetModelName:options:document:error:`.
  The apply method reads the file, runs `SuperStar::PreprocessXmlBuffer`,
  parses with pugixml, applies the timing offset, resolves the
  target name to an `Element*` (model or submodel), reads
  `GetBufferSize` for "Default"/"2D"/"None", calls
  `SuperStar::Importer::Run`, then `MarkRgbEffectsChanged`. Options
  flow through an `XLSuperStarImportOptions` ObjC class. No prompt
  callback — `imageGroupPrefix` is pre-resolved from the file
  basename. No session state.
- ✓ **iPad SwiftUI sheet (done).** `SuperStarImportView.swift` is
  a single `Form` with: file-name display, target-model `Picker`,
  4 `EditableNumberField`s (X/Y size, X/Y offset), Image Resize
  picker, Layer Blend picker, timing-adjust field, and an
  optional Image Group `TextField` (defaults to file stem).
  Import button calls the bridge and on success invokes
  `viewModel.reloadRows()` + the parent's `onApplied` closure.
- ✓ **Wire-up (done).** `ImportEffectsView` now branches on
  `url.pathExtension` after the file picker: `.sup` switches the
  sheet body to `SuperStarImportView` (different toolbar /
  navigation title); `.xsq`/`.xsqz` keeps the existing channel-
  mapping flow. `.sup` added to the picker's UTType list and
  declared as `org.xlights.superstar` in
  `UTImportedTypeDeclarations`.

Verified: desktop `xLights` Debug + `xLights-iPadLib` Debug both
build clean against the new files.

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

- **Auto Map false-positives** with `aggressive` matcher on
  layouts with similar model names. Desktop users live with
  this; iPad users will have a smaller screen for review. UX
  consideration: post-Auto-Map, scroll to the first unmapped
  row by default.
- **`SequencePackage` ownership on import.** Source-side already
  proven in I-2 (`unique_ptr<SequencePackage>` held by the
  session through apply). Watch for this same pattern when
  layering in `.sup` / `.lms` packages if those formats ever
  ship as archives.

## Open questions

_(none open — maphints is in v1; convert-render-style is
deferred pending a vendor conversation about whether anyone
still ships sequences that need it.)_
