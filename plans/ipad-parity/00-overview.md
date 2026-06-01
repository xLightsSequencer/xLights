# iPad ↔ Desktop Parity — Master Overview & Roadmap

This is the **front-door document** for the xLights iPad parity effort. It
summarizes where the iPad app stands against the desktop, states the principles
every plan follows, calls out the few true blockers, and sequences all of the
themed plans into one phased roadmap. Read this first; then dive into the
theme file that owns the work you care about (index at the bottom).

---

## 1. Executive summary

A fresh, code-level analysis of `src-iPad/` against the shared `src-core/` and
the desktop `src-ui-wx/` enumerated **1112 features across 23 functional areas**:

| Status | Count | Share | Meaning |
|--------|------:|------:|---------|
| **Implemented** | **525** | 47% | At functional parity (often via a touch idiom) |
| **Partial** | **223** | 20% | Present but with real gaps |
| **Missing** | **295** | 27% | Absent but portable |
| **Out-of-scope** | **69** | 6% | Not applicable / not viable on iPad (reason given) |

### The key finding: the core sequencer is already mature

The day-to-day sequencing surface — the thing the app is *for* — is essentially
done. Of the **84 P0-priority features**, the **3 P0 gaps are now closed**
(2026-06-01 — see [Implementation progress](#implementation-progress) and the
P0 table below); they were narrow fixes, not architecture work. The strongest
areas are exactly the ones a sequencer lives or dies on:

- **Effects (Area 14): 55/70 implemented, 0 missing, 0 out-of-scope.** Every one
  of the 55 effects renders through the *same* shared `src-core` engine; the 14
  partials are panel-level dynamic-UI polish, not rendering.
- **Render/FSEQ (Area 19): 28/41**, the highest-parity area — the iPad runs the
  identical shared RenderEngine / JobPool / RenderCache / FSEQFile.
- **Sequencer editing/grid/rows (Areas 2/11/13): 101/171 implemented** — the
  highest-implemented-share theme. Select/move/resize/align, clipboard, layers,
  Display Elements, and the full Views CRUD are all wired.
- **Layout model creation & properties (Areas 6/7): 76/126** — model creation
  for 16 types, full per-type property editor, multi-select edit, groups, view
  objects, 2D/3D, ARKit Map-from-Lights.
- **Timing/audio (Areas 12/17): 54/92** — the entire Metal waveform stack, every
  band/stem/vocal filter, on-device onset/tempo/chord/pitch analyzers, HTDemucs
  stems, and the lyric-breakdown pipeline.

### Where the remaining work actually concentrates

The gaps cluster in a handful of areas, and they are overwhelmingly **UI +
bridge wiring over shared core that already exists**, not new algorithms:

| Cluster | Areas | Why it's behind |
|---------|-------|-----------------|
| **Import / Export** | 16 (5/41 impl) | Second-weakest area. Channel-map polish, ~8 legacy/competitor importers, report exporters. Converters often live in `src-ui-wx` and need a core home. |
| **Preferences** | 20 (9/77 impl) | No unified Settings surface yet; ~40 missing rows, but most are single `@AppStorage` toggles over already-shared setters. |
| **Automation-as-commands** | 22 (0/58 impl, 28 partial) | The *operations* exist as bridge methods; what's missing is one JSON command dispatcher to invoke them by name. |
| **Presets / Jukebox** | 23 (2/28 impl) | Lowest-parity area. Preset tree + Jukebox panel — the data model is nearly portable already. |
| **Submodels / Faces / States** | 8 (9/43 impl) | Weakest layout sub-area; needs a touch node-select grid (the keystone) to lift Faces/States/SubModels at once. |
| **Layout model management** | 6 (35/68 impl) | Node-layout/wiring views, group sort/membership convenience, model copy/paste, custom-model grid transforms. |

The throughline: **almost nothing here is a new algorithm.** The core is shared
and linked by both apps. The work is SwiftUI views, ObjC++ bridge wrappers, and
a few targeted extractions of logic that currently lives in wx UI code.

---

## 2. Guiding principles

1. **Reuse shared `src-core/`; the gaps are bridge + SwiftUI wiring, not new
   algorithms.** The render engine, effect manager, sequence/elements model,
   serializers, converters, audio manager, and detectors are all in `src-core/`
   and already linked by `xLights-iPadLib`. Every plan item names the exact
   shared-core call or existing bridge method it builds on. When logic lives in
   `src-ui-wx/` (e.g. `EffectsGrid` shift/convert/AC, `FileConverter`,
   `EffectPresetManager`), the move is to **extract it to core once** — which
   benefits the desktop too — rather than reimplement per platform.

2. **Desktop ↔ iPad parity rule (from `CLAUDE.md`).** Any change touching a
   feature that exists on both platforms should land equivalent changes on the
   other in the same PR. Core changes affect both automatically; UI features in
   `src-ui-wx/App` with an iPad counterpart in `src-iPad/App/` move together;
   new mutating ops on `Effect`/`Element`/`SequenceElements` usually need a
   matching bridge wrapper (`NS_SWIFT_NAME(…)` convention).

3. **Native touch idioms over literal ports.** Sheets, long-press menus, and
   inspectors replace modal dialogs; drag-rectangle paint replaces keyboard
   single-key AC shortcuts; `UIColorPickerViewController` (SwiftUI `ColorPicker`)
   replaces the custom HSV dialog; the iOS share sheet replaces "reveal in
   Finder." A handful of iPad-only idioms (two-finger marquee, long-press menus)
   have no desktop equivalent and that's fine.

4. **Core-purity / include-boundary constraints.** Nothing under `src-core/`
   may include `src-ui-wx/` or any `wx/…` header (enforced by
   `ci_scripts/check_core_include_boundaries.sh`). New core code must honor the
   `-ffast-math` rules (use `xl::isnan/isinf/isfinite` and `numeric_limits::max()`
   sentinels, never `infinity()`), avoid throwing `std::stoi`/`std::format`, and
   funnel all mutating ops through the single `@Observable SequencerViewModel`
   so undo registration + row reload stay in one place.

5. **Open-source-firmware controller scope (product decision).** For now, the
   iPad app deliberately limits **controller upload, FPP Connect, and the
   Visualize wiring view to open-source-firmware controllers — FPP, WLED, and
   ESPixelStick** (exactly the three `resources/controllers/*.xcontroller`
   configs that declare `<OpenSourceFirmware>`; the `caps.openSourceFirmware`
   gate already admits all three). This is a scope/monetization choice, *not* a
   platform limit — iOS can HTTP-upload to any vendor. Proprietary / closed-
   firmware controllers (Falcon, Genius, HinksPix-network, PowerDMX, Pixlite/
   Advatek, SanDevices, J1Sys, …) **may** be supported in the future, but are
   **low priority and will likely be enabled only via an in-app purchase /
   subscription tier**. Theme **07** carries the gate details; truly
   platform-blocked pieces (HinksPix SD-card authoring, raw serial/DMX output)
   remain hard out-of-scope in **99**. Note FPP Connect *itself* is narrower
   than the general gate — it admits FPP + ESPixelStick fppTypes; WLED uploads
   via its own controller HTTP path, not the FPP multisync sheet.

---

## 3. The 3 P0 gaps — ✅ CLOSED (2026-06-01)

All three were in the sequencer-editing and timing themes and were narrow fixes,
not redesigns. **All three are now implemented and build-verified** (iPad lib,
Debug):

| P0 gap | Status | Fix landed | Plan / item |
|--------|--------|-----|-------------|
| **Undo** | ✅ **DONE** | Edit-menu now surfaces `undoManager.undoActionName` (e.g. "Undo Move Effect"); falls back to the bare verb when nothing is named. Foundation-`UndoManager` architecture kept. `XLightsCommands.swift`. | **01 / SEQ-1** |
| **Redo** | ✅ **DONE** | Same `redoActionName` surfacing on the Redo button. | **01 / SEQ-1** |
| **Add Timing Track dialog** | ✅ **DONE** | Added a "Fixed (custom interval)…" option with an arbitrary-ms field in `AddTimingTrackSheet.swift`, committing via the existing `addFixedIntervalTimingTrack` bridge. (The sheet already had 25/50/100 ms presets, metronome with arbitrary interval, FPP, audio onsets/tempo/chords, AI Lyrics, LRCLIB — only the arbitrary *plain fixed* interval was missing.) | **02 / TIM-1** |

> **Note on the remaining undo audit:** the SEQ-1 label fix is done. The broader
> audit for mutating ops that don't `registerUndo` at all (the plan flagged
> `toggleElementRenderDisabled`) is tracked as remaining work under 01 / SEQ-1.

## 3a. Implementation progress

Verified-landed against the live tree (each item build-checked on the
`xLights-iPadLib` Debug scheme). The plans were authored from a code snapshot
and are corrected here where the live code had already moved ahead.

| Item | Theme | What landed (2026-06-01) | Build |
|------|-------|--------------------------|:-----:|
| **SEQ-1** | 01 | Undo/Redo menu titles show the action name. | ✅ |
| **TIM-1** | 02 | Add-Timing "Fixed (custom interval)" option. | ✅ |
| **SEQ-3** | 01 | Global **Select All** (`selectAllEffects()` + ⌘A Edit-menu command) across all non-timing rows. | ✅ |
| **IE-1 / TIM-2** | 08 / 02 | Six more timing-import formats wired end-to-end (SRT, Audacity `.txt`, ElevenLabs `.json`, Vixen 3 `.tim`, LSP `.msq`, xLights `.xsq`) via new `XLSequenceDocument` bridge methods over the **already-shared** core `SequenceFile::Process*` processors + view-model wrappers + the `SequenceSettingsSheet` import picker. (LOR `.lms` + Papagayo `.pgo` were **already** wired before this work — a stale "missing" classification, now corrected.) | ✅ |
| **IE-2** | 08 | Import option **Convert Render Style** — threaded a `convertRenderStyle:` param through `XLImportSession.applyImport…` to all `MapXLightsEffects(convertRender:)` call sites (was hardcoded `false`) + a Toggle in `ImportEffectsView`. | ✅ |
| **FX-3** | 03 | `dynamicOptions` resolver `case "audioTracks"` in `SequencerViewModel` (over the `altTrack*` bridge), enabling the VU Meter audio-track + Text lyric-track choices that rendered empty. | ✅ |
| **FX-1 / FX-2 / FX-4a** | 03 | Effect-panel `visibilityRules` authored as JSON (runtime resources, decode-verified): VU Meter ~60-type enable matrix (11 rules), Text source-priority + movement enables (4 rules), Adjust action-dependent Value1/Value2 enables (2 rules). Done by 4 parallel JSON agents; `dynamicOptions` set on VUMeter/Text. Runtime greying-out pending on-device confirmation. | ✅ (JSON) |
| **FX-14** | 03 | **Cross-platform** `allOf` compound-`when` operator: recursive `WhenCondition`/`evaluateCondition` (iPad `EffectSettingsView`) + refactored `ParseWhenCondition`/`EvaluateRuleCondition` recursing on a new `VisibilityRule.allOf` (desktop `JsonEffectPanel`) + `_schema.json`. VUMeter `SVGFile` rule converted to `allOf` (Type==Level Shape AND Shape==SVG). | ✅ iPad + ✅ desktop |
| **FX-4b** | 03 | **Cross-platform** `dynamicLabel` schema field: `DynamicLabelMetadata` + `PropertyMetadata.dynamicLabel` + CodingKey + `_schema.json`; `EffectSettingsView` resolves it from the controlling sibling and passes `displayLabelOverride` to `EffectPropertyView`. Adjust `Value1`/`Value2` labels now follow `Action` (Adjust by:/Minimum:/…). Desktop keeps its custom `AdjustPanel`; field is additive for future JSON panels. | ✅ iPad + ✅ desktop schema |
| **SEQ-2** | 01 | **Color Replace** (whole sequence): new bridge `usedColours(selectedOnly:)` + `replaceColour(from:to:selectedOnly:)` over core `SequenceElements::GetUsedColours`/`ReplaceColours`; `ColorReplaceSheet` (from-picker of used colours + `ColorPicker`); Edit ▸ Color Replace menu + sheet. Symmetric palette-snapshot Foundation undo (core `ReplaceColours` records only into the core undo manager, which iPad Cmd-Z doesn't drive). | ✅ |
| **IE-9** | 08 | Incremental **search filters** on the import mapping view's Source and Destination panes (pure SwiftUI over the snapshot arrays; clear button + filtered/total count). | ✅ |
| **FX panels (Wave/Warp/Snowflakes/Ripple/Shape)** | 03 | `visibilityRules` authored for 5 more JSON-driven effects via a parallel `ValidateWindow` audit workflow (Warp uses the new `allOf`). Decode-verified; greying pending device check. NEEDS-SWIFT items (VC-clear, value-clamp, dynamic label) deferred, not faked. | ✅ (JSON) |
| **TIM-3** | 02 | **Multi-track timing export** — bridge `exportTimingTracksAtRows:toPath:` writes selected timing tracks into one `<timings>` `.xtiming`; multi-select picker sheet + "Export Multiple Tracks…" entry in the Timings tab. | ✅ |
| **IE-3 (hint load)** | 08 | **Load `.xmaphint`** — bridge `loadMapHintsFromPath:` reuses `MapHintsIO::LoadMapHintsFile` + the existing `runAutoMap` `MatchRegex` pass; "Load Hints…" button + `.fileImporter` in the import view. (`.xmap`/`.xjmap` are wx-bound — separate core-extraction, deferred.) | ✅ |
| **IE-12** | 08 | **Update Aliases from mapping** — bridge `updateModelAliasesFromMapping` loops `_destinationRoots` and `Model::AddAlias`(source name) on each mapped model (idempotent; marks dirty so `saveLayoutChanges` persists); "Update Aliases" button. Top-level models only (submodel guard deferred). | ✅ |
| **Selected-only Color Replace** | 01 | Completes SEQ-2 with a **"Selected effects only"** toggle. Vetted-safe **sync-on-demand**: bridge `replaceColour(from:to:atRows:effectIndices:)` / `usedColours(atRows:…)` mirror the Swift selection into core `Selected` flags only for the op, then `UnSelectAllEffects` (never left dirty; `SetSelected` doesn't dirty the doc; grid draws selection from the Swift set). | ✅ |
| **TOOLS-1b** | 09/13 | **Purge Render Cache** — `iPadRenderContext::PurgeRenderCache()` (→ `RenderCache::Purge`) + bridge `purgeRenderCache` + Tools-menu entry. | ✅ |
| **TOOL-8** | 13 | **Help-menu parity** — added the missing **Zoom Room Help** + **Donate** links (`XLOpenURL`). | ✅ |
| **IE-2 persistence** | 08 | Import-option toggles (erase / lock / convert-render-style) now persist across launches via `@AppStorage`. | ✅ |
| **PRE-1** | 10 | **Persistent effect preset library** (the keystone). `EffectPresetManager` relocated to wx-free `src-core/effects` (shared desktop↔iPad); `iPadRenderContext` loads/saves `xlights_effectpresets.json` (desktop format); bridge (`presetTree`/`savePreset`/`applyPreset`/group/rename/delete/move/import/export) + new `PresetBrowserSheet`; session-only presets retired. Implemented in a worktree agent + merged. | ✅ iPad + desktop |
| **IE-4** | 08 | **LOR S5 `.loredit` import** — discovery + effect-apply both done. `LOREdit` relocated to wx-free `src-core/import_export`; the 7 `MapS5*` apply fns moved to core `EffectMapper`; desktop `ImportS5` shares both; iPad `XLImportSession` `_loreditMode` loads + applies `.loredit` (mirrors desktop). | ✅ iPad + desktop |
| **IE-15** | 08 | **Export Models XLSX** — DONE. wx-free `src-core/import_export/ExportModels` (libxlsxwriter) + bridge + Layout-editor export; `-lxlsxwriter` now in the `xLights-iPad` `OTHER_LDFLAGS` (committed in the `macOS` submodule) so the app builds + links. | ✅ iPad + desktop |
| **IE-1 (full)** | 08 | **Timing-import core extraction** — fixed the iPad-app link bug (the LSP/xLights/Vixen3/ElevenLabs `Process*` lived in `src-ui-wx`, unlinked by the app). New wx-free `src-core/utils/ZipUtils`, relocated `Vixen3`, new core `TimingImport.cpp`; XLights/Vixen3 imports are now two-step (desktop `wxMultiChoiceDialog` / iPad multi-select sheet). | ✅ iPad + desktop |
| **IE-25** | 08 | **Vixen 3 `.tim` *effect* import** — `MapVixen3*` moved to core `EffectMapper`; iPad `_vixen3Mode` (load/discovery/apply mirrors desktop `ImportVixen3`); `.tim` routed in `ImportEffectsView` (Timings tab keeps timing-only `.tim`). | ✅ iPad + desktop |
| **IE-7** | 08 | **Import dispatch + warnings** — `.piz` in the picker; bridge source-metadata accessors; non-blocking FPS-mismatch (after load) + missing-media (after apply) warnings. | ✅ iPad |

> **Worktree-agent integration (2026-06-01):** the three items above were each implemented by a **worktree-isolated agent** (branched from the `ipad-parity-p1` commit, self-built), then merged via patches with one small conflict resolved (PRE-1's `SequencerViewModel`/`XLightsCommands` overlap with Color Replace — kept both). The **combined** tree was re-built green on iPad-lib + desktop and passes `check_core_include_boundaries.sh`. **Runtime-unverified** (build-green only): preset round-trip/apply, `.loredit` parse correctness, and XLSX validity — need device/desktop testing. (The IE-15 iPad-app link is now **resolved** — `-lxlsxwriter` was committed in the `macOS` submodule and recorded in the superproject, so the `xLights-iPad` app links.)

> **Post-worktree session (2026-06-01) — import/export theme largely closed:** beyond the three worktree items above, this session landed **IE-1 (full)** (timing-import core extraction — new `ZipUtils` + relocated `Vixen3` + core `TimingImport.cpp` — fixing the iPad-app link bug, with two-step XLights/Vixen3 track selection), **IE-4's effect-apply slice** (`MapS5*` → core `EffectMapper`), the **IE-15 app-link**, and **IE-25** (Vixen 3 `.tim` effect import via `MapVixen3*` → core + `_vixen3Mode`) + **IE-7** (`.piz`, FPS / missing-media warnings). All build-verified across desktop + `xLights-iPadLib` + the `xLights-iPad` app; core boundary-check clean; IE-25/IE-7 adversarially reviewed (no functional defects). See `08-import-export.md` for detail. **Remaining:** on-device verification with real `.loredit` / `.tim` / `.msq` / `.xsq` files + XLSX content.

**Deferred this round (with rationale):** **PREF-5** (snap-to-timing toggle) and **VIEW-2** (Show AC Ramps toggle) — both flip *runtime rendering behavior* in the Metal grid that a build can't confirm and whose reactive-persisted-state + redraw-on-toggle plumbing is easy to get subtly wrong un-runnable; deferred to a device-verifiable session. **SEQ-4** (Cut menu) — a disabled-but-⌘X-bound command can swallow text-field Cut on iOS (the documented reason it was originally omitted); deferred pending the safe-binding decision. (**FX-14** and **FX-4b** — previously deferred follow-ups — are now landed, above.)

**Correction to the plans (live code was ahead of the snapshot):** TIM-2's
LOR/Papagayo import was already shipped; the Add-Timing sheet already had
fixed-presets + metronome; and core already exposes `ProcessVixen3Timing` /
`ProcessLSPTiming` (the plan guessed those were wx-side and deferrable — they
were not, and are now wired). Verify-against-current-code before implementing.

These four close all P0s plus the highest-leverage Phase-1 quick wins.

---

## 4. Phased roadmap

Each theme file carries its own Phase 1 (P0/P1) / Phase 2 (P2) / Phase 3 (P3)
breakdown with item IDs, effort, and dependencies. The global sequencing below
groups that work by phase and shows which theme files carry it.

**Rough relative-effort per theme** (sum of its work items, S/M/L/XL):

| Theme | Effort | Shape of the work |
|-------|:------:|-------------------|
| 01 Sequencer editing | **L** | Many S/M wins; one XL outlier (AC draw mode) recommended as its own workstream |
| 02 Timing & audio | **M** | Mostly S/M wiring; one L (Note Import core relocation) |
| 03 Effect panels | **M** | Mostly data-only JSON `visibilityRules`; one XL (Moving Head canvases) |
| 04 Color & value curves | **M** | All S/M; no new color math |
| 05 Layout model mgmt | **L** | Node-select grid + node-layout/wiring views are the big L items |
| 06 Submodels/Faces/States/DMX/preview | **L** | Structured Faces/States editors (L each) ride on the shared node-select component |
| 07 Controllers & upload | **M** | Ethernet output-config descriptors (L) + many S/M; HinksPix SD-card out of scope |
| 08 Import/Export | **L** | Cheap Tier-A wins, then converter→core extractions (L/XL); the biggest cluster |
| 09 File lifecycle / render / tools | **M** | Backup/restore + FSEQ format + selection helpers, mostly S/M; one L (XLSX workbook) |
| 10 Presets / Jukebox / view | **L** | Preset tree relocation (M) unlocks a chain of L items + Jukebox |
| 11 Preferences | **M — but many small** | One M shell, then ~30 mostly-S `@AppStorage` toggles |
| 12 AI & automation | **L** | One L dispatcher keystone unlocks ~25 partial commands; AI editor + Lua engine |

### Phase 0 — close the 3 P0 gaps + top quick wins (Rollup C)

Smallest, highest-leverage items; ship first to remove the only blockers and
make the partials feel finished. (Effort in parens.)

- ✅ **01 / SEQ-1** — Undo/Redo label surfacing *(closes both P0s)* — **DONE 2026-06-01**. (Broader missing-`registerUndo` audit still open.)
- ✅ **02 / TIM-1** — Add Timing Track custom fixed interval *(closes the 3rd P0)* — **DONE 2026-06-01**.
- ✅ **01 / SEQ-3** — global Select All / Select-All-no-timing — **DONE 2026-06-01**.
- **01 / SEQ-4** — re-add per-effect **Cut** menu entry (S)
- ✅ **02 / TIM-2** — LOR `.lms` / Papagayo `.pgo` timing import — already shipped before this work (stale classification, corrected).
- ✅ **08 / IE-1** — 6 more timing-import formats (SRT/Audacity/ElevenLabs/Vixen3/LSP/xLights) — **DONE 2026-06-01**. ✅ **08 / IE-2** — Convert-Render-Style param — **DONE 2026-06-01**.
- ⏸ **11 / PREF-5, PREF-13** — Snap-to-Timing toggle, Show AC Ramps toggle — **deferred** (runtime-rendering behavior, not build-confirmable; needs device verification).
- ⏸ **10 / VIEW-2, VIEW-3** — Show AC Ramps + Dock All / Reset-to-Defaults — VIEW-2 deferred with PREF-13 (same Metal-grid redraw concern).
- ✅ **03 / FX-1, FX-2, FX-3, FX-4a** — VU Meter type matrix + Text + Adjust enables + audio/lyric `dynamicOptions` — **DONE 2026-06-01** (JSON decode-verified; greying-out pending device check).
- **05 / LAY-1**, **06 / LAY-1** — add DMX model creation from the picker (S)
- **07 / CTL-15, CTL-5** — sort controllers; Add-FPP-by-IP + Re-Discover (S)

### Phase 1 — P1 across themes (sequenced by dependency + user value)

The bulk of visible parity. Suggested order honors the cross-theme keystones:

1. **Keystone enablers first** (each unlocks several downstream items):
   - **05 / LAY-2 — touch Node-Select grid** → unlocks Faces/States/SubModels
     visual authoring (05 LAY-3/4/5, 06 LAY-2→LAY-5).
   - **10 / PRE-1 — relocate `EffectPresetManager` to `src-core/`** → unlocks the
     persistent preset store + browser (PRE-2/3/4) and is a desktop win too.
   - **12 / AUTO-1 — in-process JSON command dispatcher** → converts ~25 partial
     automation rows into working commands and gates the Lua engine.
   - **08 / IE-5 — lift `FileConverter` read paths into a core
     `LegacySequenceReader`** → unlocks LOR/HLS/Vixen/LSP importers.
2. **High-traffic editing & panels:** 01 (SEQ-2 Color Replace, SEQ-5 split
   fidelity, SEQ-6 move-layer, SEQ-7 timing-import formats, SEQ-8 delete-by-scope),
   03 (FX-1 VU-Meter matrix, FX-2 Text enables, FX-4 Adjust labels), 04 (COL-1/2
   palette + per-curve bulk edit).
3. **Layout & controllers:** 05 (LAY-6/7 node-layout & wiring views, LAY-8/9/10/11/12
   copy-paste / start-channel fixer / bulk edit / add-to-group / sort), 06 (LAY-3→9
   structured Faces/States, per-mesh/servo transforms, wiring toggle), 07 (CTL-1/2
   ethernet+protocol output config, CTL-3 bulk upload, CTL-4 proxy validate,
   CTL-6/7 FPP Connect).
4. **Import/Export & media & lifecycle:** 08 (IE-3 mapping save/load, IE-4 LOR S5,
   IE-6/7 tree + dispatch, IE-8 `.pgo` export), 02 (TIM-3/4 multi-track export,
   word breakdown; MED-1/2/3 add-media + placement + video warning), 09 (BKP-1/2
   backup/restore, FSEQ-1 format pref, FPP-1/2, LIFE-1/2 guards, XLSX-1 workbook).
5. **Presets / automation / AI surfaces:** 10 (PRE-2/3/4 store + selection-preset +
   browser, VIEW-1 dropper icons), 12 (AI-1/2/3/4 image editor + AI map, AUTO-2/3
   addEffect/get-set + Speech-to-Lyrics entry).
6. **Preferences shell:** 11 (PREF-1 the Settings shell, then PREF-2/3/4/6/7
   render-on-save / paste mode / grid spacing / autosave / Cmd-N/O).

### Phase 2 — P2

Convenience, breadth, and the second tier of each theme, built on Phase-1
foundations: 01 (SEQ-9 shift family, SEQ-11 convert-type, SEQ-12→22 Display
Elements bulk/sort/reorder + timing niceties), 02 (TIM-5/6/8 divide/subdivide/
waveform region; MED-4/5/6), 03 (FX-6 DMX value curves, FX-7 Music, FX-8 Shader
download, FX-9 Moving Head canvases [XL]), 04 (COL-3→7 palette toolbar, replace,
dropper, drag-drop), 05 (LAY-20→ custom-model grid transforms), 06 (Phase-2
preview/state items), 07 (CTL-8→14 input config, status, PWM upload), 08 (IE-9→17
find/filter, AI map, aliases, report exporters), 09 (CLEAN-1, BR-1/2 batch scope,
RENDER-1/2 progress + time-slice, RC-1, BKP-3), 10 (PRE-5→8 groups/import/preview,
JUK-1/2/3 jukebox, AC-1 [XL]), 11 (PREF-8→14 check-suppression, render cache,
E1.31 sync, backgrounds), 12 (AI-5, AUTO-4/5/6/7/8 command families + Lua engine).

### Phase 3 — P3

Long-tail polish and niche/legacy items; do on demand. Each theme's Phase-3
table holds these — e.g. 01 (SEQ-26 random fill, SEQ-28 find-data, SEQ-30 AC mode
[XL], SEQ-32 polish batch), 03 (FX-10→13 Piano/animated-preview/paint/lock), 05
(LAY-30→35 clone-group/aliases/DXF), 06 (LAY-24→30 GDTF/LOR-prop/submodel transforms),
07 (CTL-15→19 sort/ZCPP/HinksPix-network), 08 (IE-18→24 legacy importers + Convert
dialog), 09 (TOOLS-1, CHK-1, EXPORT-RH, RC-2, LIFE-3), 10 (PRE-9/10 reorder/purge,
JUK-4, VIEW-4/5/6 search/find-data/video), 11 (PREF-15→19 ZIP backup + polish toggles),
12 (AI-7/8, AUTO-9/10/11 Lua prompts + low-value commands + script download).

---

## 5. Touch / Pencil UX — design principles & cross-cutting items

The themed plans bake touch idioms (long-press menus, two-finger marquee, inline
action bars, drag-rectangle paint) into individual functional items, but a few
touch/Pencil concerns are **cross-cutting** and don't belong to any one theme.
They are gathered here as standing principles, open design questions, and a
genuine accessibility gap. (Source: `plans/phase-j-touch-ux.md`.) **OV-1.**

### Design principles (apply to every touch/Pencil item across the set)

1. **Recognisable as xLights.** Same handle palette and conceptual model as
   desktop.
2. **Touch-first, not touch-tolerant.** Multi-finger gestures are first-class.
3. **Pencil-enhanced, never Pencil-required.** Every operation reachable with
   finger-only.
4. **Discoverable.** No "click the orange sphere twice to cycle tools." Modes
   live in a persistent toolbar.
5. **Apple-HIG-compatible.** Pencil for precision, fingers for navigation.

### Open design questions (post-MVP — decide before building)

- **Pencil pressure** — does it actually help here, or is it a gimmick?
  Prototype, then decide.
- **Sketch-mode polyline create** — pencil-draws-a-path → auto-vertices.
  Compelling but post-MVP (the discrete multi-vertex polyline create already
  shipped 2026-05).
- **3D camera precision mode** — the current "drag-on-empty-3D = orbit" is fast
  and approximate; do we add a separate precision-camera mode (e.g. on-screen
  joystick) for fine framing? Cross-ref the deferred **SpaceMouse / 3Dconnexion
  6-DOF** item in `99-out-of-scope.md` (no iPadOS 6-DOF HID API) — this on-screen
  mode is the native substitute, not the HID device.
- **Pencil-button visibility policy** — show the Pencil affordance in the toolbar
  as a visible "Pencil enhances this app" indicator, or hide it until a Pencil is
  detected? Apple's HIG prefers hidden-until-detected.

### Accessibility / VoiceOver audit (standing cross-cutting concern)

VoiceOver users cannot drive most canvas-gesture-only editing (layout canvas
*and* sequencer grid) — there is no VoiceOver-navigable parallel path today. This
is a real functional gap, not just a question: it needs a "list of models +
property panel" path (partially present from the layout sidebar) plus an audit of
the sequencer grid. **The concrete owning work items live in the functional
docs:** sidebar multi-select + the accessibility audit in **`05-layout-models.md`**
(also flag the sequencer canvas there), and multi-select **body drag** (move the
whole selection by one delta, not just the touched model — bridge needs a
per-selected-model offset table at drag-begin) in **`06-layout-submodels-dmx-preview.md`**.

---

## 6. iOS build / dependency, repo-layout & easy-wins reconciliation notes

Orientation notes that don't belong to a theme but matter when scoping the work.

- **OV-2 — iOS dependency list / build-link confidence.** The prebuilt iOS
  static libs at `/opt/xLights-macOS-dependencies/lib-ios/` already include
  **`libxlsxwriter.a`** and **`liblua.a`**, alongside `libcurl.a`,
  `libliquidfun.a`, `libzstd.a`, and ANGLE (`libEGL.xcframework` +
  `libGLESv2.xcframework`). Consequences: the "verify libxlsxwriter links on
  iPad" risk behind the XLSX connections workbook (**09 / XLSX-1** and the
  import/export **IE-15** family) is **resolved** — the lib is present, so that
  work is link-safe — and the presence of `liblua.a` is the dependency
  precondition for the **deferred, heavily-caveated Lua engine in
  `12-ai-and-automation.md`** (it makes the engine *buildable*; App-Store review
  risk is a separate, unresolved concern tracked there and in `99`).
- **OV-3 — Why `src-iPad/` is a repo-root peer of `macOS/`, not under it.** Code
  under `macOS/` cannot depend on sources outside `macOS/`; the iPad app shares
  `src-core/` (and bridges it from `src-iPad/`), so both live at the repo root as
  peers. (Low-value orientation note; included so the layout isn't mistaken for
  an oversight.)
- **OV-4 — Easy-wins already shipped (don't re-pull).** Several `_raw-gap-analysis.md`
  §5 easy-wins are already in the app and must **not** be re-scoped: **H-1**
  About box, **H-2** Help, **A-3 / A-4 / A-11** audio overlays, and **AI-1** the
  color-palette surface. The remaining easy-wins map into each theme's
  Quick-Wins / Phase-0 (Rollup C) rather than being re-listed here.

---

## 7. Plan document index

| File | Scope (gap-analysis areas) | Headline work | Size |
|------|----------------------------|---------------|:----:|
| **01-sequencer-editing.md** | Sequencer Editing, Grid & Display Elements (2, 11, 13) | Undo/Redo P0 fix, Color Replace, Select All, split fidelity, Display Elements bulk/sort, AC mode | L |
| **02-timing-and-audio.md** | Timing Tracks, Waveform & Media/Audio (12, 17) | Add-Timing P0, multi-format timing import/export, lyric breakdown, add-media + placement | M |
| **03-effects-panels.md** | Effect Panels & Per-Effect UI (14) | JSON `visibilityRules` for VU-Meter/Text/Adjust, dynamic option choices, Moving Head canvases | M |
| **04-color-and-value-curves.md** | Color, Palettes & Value Curves (15) | Per-curve bulk edit, palette toolbar, color replace + dropper, drag-drop swatches | M |
| **05-layout-models.md** | Layout: Model Creation, Management & Properties (6, 7) | Node-select grid keystone, node-layout/wiring views, copy/paste, custom-model transforms | L |
| **06-layout-submodels-dmx-preview.md** | Layout: Submodels/Faces/States, DMX/Moving Heads/3D, Preview & View Objects (8, 9, 10) | Structured Faces/States editors, per-mesh/servo transforms, wiring toggle, DMX creation | L |
| **07-controllers-and-upload.md** | Controllers: Configuration & Upload Protocols (5, 18) | Ethernet/protocol output config, bulk upload, FPP Connect, HinksPix network (SD-card out of scope) | M |
| **08-import-export.md** | Import & Export — sequences & data (16) | Timing-import formats, channel-map save/load + find/filter + AI map, LOR/legacy importers, report exporters | L |
| **09-file-lifecycle-render-tools.md** | File & Sequence Lifecycle, Render/FSEQ & Tools Menu (1, 19, 3) | Backup/restore, FSEQ format pref, batch/FPP selection helpers, XLSX connections workbook | M |
| **10-presets-jukebox-view.md** | Effect Presets, Jukebox & View / Window Management (23, 4) | Portable preset tree, persistent preset browser, Jukebox panel, dock/ramps toggles | L |
| **11-preferences-settings.md** | Preferences & Settings (20) | Unified Settings shell, render-on-save, paste mode, grid spacing, check-suppression, render cache | M (many small) |
| **12-ai-and-automation.md** | AI Features & Automation/Scripting (21, 22) | AI image editor + AI model map, JSON command dispatcher keystone, command families, Lua engine | L |
| **13-help-diagnostics-tools.md** | Help, Diagnostics & un-homed Tools-menu utilities (2.16, plus help/about) | Help/diagnostics surface largely shipped (About, Help URLs, View/Package Logs, crash telemetry, launch screen); new work is un-homed Tools utilities — ShowFolderSearch→Files picker, Generate 2D Path, Prepare Audio, Tip-of-the-Day→"What's New", CharMap/ResizeImage substitution, EmailDialog prompt, Help-link parity (Donate/Zoom), Package-Show-for-Support scope | S–M |
| **99-out-of-scope.md** | Out-of-scope features grouped by root cause | 68 distinct items: App-Store code policy (incl. downloading/executing scripts per guideline 2.5.2), no removable media/serial, wxAUI perspectives, FFmpeg encode, CLI/argv, companion-app IPC | — |
| **_raw-gap-analysis.md** | All 23 areas, 1112 features | Raw, unedited per-feature data (status/priority/effort/iPad location/notes) + Summary table + Rollups A/B/C. Cite this, don't re-derive. | — |
| **_reconciliation.md** | Cross-check of this set vs. the prior iPad plans | Per-feature reconciliation of `plans/ipad-parity/` against the prior plans (root `iPad-xLights-Plan.md` + `plans/*.md`): coverage matrix, miss list, and the per-target-doc fix list that produced the additions in this set. | — |

---

## 8. How these plans were produced

This plan set is the output of a **fresh code-level analysis** of the live
codebase — `src-iPad/` (SwiftUI app + ObjC++ bridge + Metal canvases), the shared
`src-core/`, and the desktop `src-ui-wx/` — covering **23 functional areas and
1112 features**. Each feature was classified (implemented / partial / missing /
out-of-scope) with its priority, rough effort, the concrete iPad code location,
and the reasoning behind the call, recorded in **`_raw-gap-analysis.md`** (the
input data). The themed plans group that data into actionable workstreams and
name the exact shared-core call or bridge method behind each item. The analysis
was conducted **independently of any prior planning docs** (including
`iPad-xLights-Plan.md`); where a behavior is intentionally implemented
differently on iPad (e.g. Foundation `UndoManager` vs core `UndoManager`, native
analyzers vs VAMP), that is recorded as a deliberate divergence, not a gap.

After the independent pass, the set was **reconciled against the prior iPad
planning docs** (the root `iPad-xLights-Plan.md` plus `plans/*.md`); items those
plans flagged but this analysis missed were folded back in (see
`_reconciliation.md`), so the set is now a superset of both.
