# iPad Parity Plan Reconciliation

This document maps **every item from the prior iPad plans** (the master tracker
`iPad-xLights-Plan.md`, the `gap-analysis-2026-04-23.md`, the various
`plans/future-*.md` and `plans/phase-*.md` sub-plans, `plans/followups.md`,
`plans/README.md`, and the Android feasibility study) onto the **new themed
plan set** in `plans/ipad-parity/` (docs `00-overview.md` … `12-ai-and-automation.md`,
`99-out-of-scope.md`, and the `_raw-gap-analysis.md` dataset).

Its purpose is to confirm the new plan set lost nothing as the old phase/future
docs are retired, and to produce an **actionable fix list** of what must be
added or enriched in the new docs before the old ones are deleted.

Each prior item was classified as one of:

| Classification | Meaning |
|---|---|
| **covered** | Fully represented in a new themed doc; no action needed. |
| **covered-needs-detail** | Conceptually present in a new doc, but the prior plan carries richer spec / file refs / rationale / priority that should be copied in. |
| **missing** | A real desktop feature (verified in code) with **no home** in any new doc. Must be added. |
| **stale-superseded** | Listed as pending in a prior plan but actually already shipped on iPad (verified), or superseded by a newer decision. No action — record so it isn't re-planned. |
| **platform-handled** | The platform (App Store / LaunchScreen) handles it; note only. |
| **hard-miss-oos** | Genuinely out of scope; belongs in `99-out-of-scope.md`. |

> **Note on the two big upstream summaries:** the `iPad-xLights-Plan.md` master
> tracker and `gap-analysis-2026-04-23.md` use slightly different conventions
> for the same items (e.g. O-7, O-8). Where both reference a feature, the fix
> list below de-duplicates so the target doc gets **one** enriched item.

---

## 1. Summary table — per source file, counts by classification

| Source file | covered | covered-needs-detail | missing | stale | platform-handled | hard-miss-oos | total |
|---|---:|---:|---:|---:|---:|---:|---:|
| `iPad-xLights-Plan.md` (master tracker) | 9 | 9 | 6 | 2 | 0 | 2 | 28 |
| `gap-analysis-2026-04-23.md` | 110 | 16 | 18 | 20 | 4 | 13 | 181 |
| `followups.md` | 1 | 5 | 3 | 11 | 0 | 0 | 20 |
| `future-controller-upload.md` | 0 | 9 | 6 | 5 | 0 | 1 | 21 |
| `future-aux-panels.md` | 9 | 10 | 2 | 4 | 0 | 1 | 26 |
| `future-custom-models.md` | 5 | 11 | 2 | 12 | 0 | 1 | 31 |
| `future-effect-presets.md` | 6 | 1 | 0 | 1 | 0 | 0 | 8 |
| `future-help-diagnostics.md` | 3 | 2 | 5 | 1 | 2 | 5 | 18 |
| `future-imports-exports.md` | 14 | 8 | 1 | 0 | 0 | 1 | 24 |
| `future-layout-editing.md` | 3 | 3 | 0 | 7 | 0 | 1 | 14 |
| `future-pictures-frame-editor.md` | 0 | 3 | 2 | 0 | 0 | 1 | 6 |
| `future-preferences.md` | 6 | 11 | 11 | 4 | 0 | 6 | 38 |
| `phase-b-grid-parity.md` | 2 | 2 | 1 | 1 | 0 | 0 | 6 |
| `phase-i-import-effects.md` | 2 | 3 | 0 | 3 | 0 | 0 | 8 |
| `phase-j-layout-editor.md` | 9 | 6 | 2 | 6 | 0 | 1 | 24 |
| `phase-j-touch-ux.md` | 0 | 1 | 7 | 1 | 0 | 0 | 9 |
| `README.md` (plan index) | 21 | 8 | 4 | 1 | 0 | 6 | 40 |
| `android-port-feasibility.md` | 2 | 0 | 0 | 3 | 0 | 4 | 9 |
| **TOTAL** | **202** | **108** | **70** | **82** | **6** | **44** | **512** |

> Counts include cross-cutting / context rows so every item is preserved. Many
> `missing`/`covered-needs-detail` rows are duplicates of the same underlying
> feature reported by multiple prior plans (e.g. PixelTestDialog appears in
> three source files, ShowFolderSearchDialog in three, Shader uniform grouping
> in three); these collapse to ~25 distinct real features in the fix list.

---

## 2. STRUCTURAL GAPS — whole topics with no home in the new plans

Two themes have no owning doc in the new set. Several real desktop features and
cross-cutting concerns fall through the cracks as a result.

### GAP A (confirmed) — Help / Diagnostics / Tools theme

The new plan set has **no Help/Diagnostics/Tools doc**. The prior plans treated
the high-traffic Help/diagnostics items (About, Help-menu URLs, log export,
crash telemetry, View Log) as *already shipped during the TestFlight sweep*, so
no theme doc was created. But a cluster of **real, still-shipping desktop
features** and several **already-shipped-on-iPad** items have nowhere to be
recorded:

- **Genuinely missing Tools utilities** (verified present in
  `src-ui-wx/xLightsMain.cpp`, and absent from every new doc *and* from
  `_raw-gap-analysis.md`): **ShowFolderSearchDialog** (T-8/T-22/H-12),
  **PathGenerationDialog / Generate 2D Path** (T-13/T-24/U-7), **Prepare Audio
  (Reaper .rpp / .xaudio)** (T-15), **Tip-of-the-Day** feature itself (H-5/U-2),
  **CharMapDialog** (U-14), **ResizeImageDialog** (U-6), **Purge Render Cache**
  action (the un-homed half of T-9), **EmailDialog** crash-report prompt (H-10).
- **Already-shipped-on-iPad** items needing a "done, record here" home: About
  (AboutSheet), Help-menu URLs, View Log (LogViewerSheet), Package Logs
  (XLLogPackager), crash telemetry (XLDiagnosticUploader), in-app log viewer.
- **Platform-handled** notes: UpdaterDialog / Check-for-Updates, SplashDialog /
  LaunchScreen, Crash xLights / Log Render State (dev-only).

**Recommended new doc: `13-help-diagnostics-tools.md`.** It houses the missing
Tools utilities (with re-conceive notes for the iOS sandbox), records the
shipped Help/diagnostics surface as done, and notes platform-handled items.
Several dev-only / platform-handled items (Crash xLights, Log Render State,
Tip-of-the-Day feature, UpdaterDialog) should *also* get explicit rows in
`99-out-of-scope.md` (see fix list § 99).

### GAP B (confirmed) — Touch / Pencil UX cross-cutting note

The new plan set bakes touch idioms (long-press menus, two-finger marquee,
inline action bars) into individual functional items, but tracks **none** of the
cross-cutting touch+Pencil UX items from `phase-j-touch-ux.md` as discrete work.
None of these appear anywhere in `plans/ipad-parity/` outside the raw dataset:

- **Multi-select body drag** (move the whole selection by a delta — verified
  desktop `LayoutPanel.cpp:5961+`; iPad body-drag bridge is single-model
  `XLMetalBridge.mm:1928/1986`).
- **Sidebar multi-select UI** (Models roster is single-`String?` bound,
  `LayoutEditorView.swift:2378`; needs `EditMode`/checkmark rows).
- **Accessibility / VoiceOver audit** (canvas-gesture-only editing has no
  VoiceOver-navigable parallel path — a genuine functional gap, not just a
  question).
- Open design questions: **Pencil pressure**, **Sketch-mode polyline create**,
  **3D camera precision mode**, **Pencil-button visibility policy**.
- The **5 design principles** (recognisable-as-xLights / touch-first /
  Pencil-enhanced-never-required / discoverable / Apple-HIG-compatible).

**Recommended:** add a **"Touch / Pencil UX design principles & cross-cutting
items"** section to `00-overview.md` (copy the 5 principles verbatim and list
the open questions + accessibility audit), **plus** concrete work items in the
owning functional docs: multi-select body drag → `06`, sidebar multi-select +
accessibility audit → `05` (and note the sequencer canvas for accessibility too).

---

## 3. FIX LIST — grouped by TARGET NEW DOC

Each bullet: **`<item code+name>`** — *spec to incorporate* — `code evidence` —
**suggested new item ID**. Items marked **(DETAIL)** are `covered-needs-detail`
(enrich an existing item); unmarked are `missing` (add a new item).

> Where multiple prior plans flagged the same feature, the bullet cites the
> primary code with a "+ also" cross-reference and a single suggested ID.

### → `00-overview.md`

- **(DETAIL) Touch/Pencil design principles & cross-cutting items** — Add a new
  section: copy the 5 design principles verbatim (recognisable-as-xLights;
  touch-first not touch-tolerant; Pencil-enhanced never Pencil-required;
  discoverable / modes in a persistent toolbar; Apple-HIG-compatible). List the
  open design questions (Pencil pressure prototype; Sketch-mode polyline create;
  3D precision-camera mode — cross-ref the deferred SpaceMouse item in 99;
  Pencil-button visibility policy) and the accessibility/VoiceOver audit as a
  standing cross-cutting concern. — `phase-j-touch-ux.md`; `00-overview.md ~L83`
  already states "native touch idioms over literal ports". — **OV-1**.
- **(DETAIL) iOS dependency list / build-link confidence** — Record the concrete
  `lib-ios/` contents: `libxlsxwriter.a` IS present (resolves the open
  "verify libxlsxwriter links on iPad" risk in IE-15 and XLSX-1 — downgrade
  those), `liblua.a` is present (relevant to the deferred Lua engine in 12),
  plus libcurl/ANGLE/liquidfun/zstd. — `/opt/xLights-macOS-dependencies/lib-ios/`.
  — **OV-2**.
- **(DETAIL) "Why src-iPad/ is a repo-root peer, not under macOS/"** — low-value
  orientation note (macOS/ code cannot depend outside macOS/). — **OV-3**.
- **(DETAIL) Easy-wins reconciliation** — note which §5 easy-wins are already
  shipped (H-1 About, H-2 Help, A-3/A-4/A-11 audio overlays, AI-1 palette) so
  they aren't re-pulled; remaining map to each theme's Quick-Wins/Rollup-C. —
  **OV-4**.

### → `01-sequencer-editing.md`

- **B56 Convert To Effect (data-row → effects)** — Add a P3 item: lift the
  `RampLenColor` / `isOnLineColor` helpers from `tabSequencer.cpp` into wx-free
  core + per-strand `SingleLineModel` construction + bridge wrapper + row-menu
  "Convert To Effect" entry. Note the other half (Promote-Node-Effects) already
  shipped 2026-04-28. — `xLightsFrame::DoConvertDataRowToEffects`
  `tabSequencer.cpp:4173`, menu `RowHeading.cpp:541`; `_raw-gap-analysis.md:703`
  (Missing P3/L). + also `phase-b-grid-parity.md` B56. — **SEQ-33**.
- **B3 Effect::Reverse() wrapper** — Add a small (S) per-effect "Reverse" action
  (reverse settings/value-curves in time) if iPad lacks it; desktop exposes via
  keybinding. — `RenderableEffect`/`Effect` reverse semantics; no bridge in
  src-iPad. — **SEQ-34**.
- **(DETAIL) Model-tree sort submenu — exact "11 options"** — SEQ-13/LAY-12
  enumerate ~6-8 strategies; the prior plan pins the count at **eleven**. Open
  the desktop LayoutPanel sort submenu and enumerate all 11; preserve "11" as
  the acceptance checklist count. — `future-aux-panels.md` (model-tree sort). —
  enrich **SEQ-13** (+ LAY-12).
- **(DETAIL) 16-menu context-menu parity sweep** — the prior plan calls out a
  16-menu right-click parity sweep; the new plans distribute context-menus
  per-theme but never consolidate an audit. Add a tracking note that the
  Layout-canvas / model-group / View-Objects / Color-panel long-press trees
  still need an explicit sweep. — enrich **SEQ-32**.

### → `02-timing-and-audio.md`

- **(DETAIL) TIM-7 MIDI Import — packed/Type-0 (#6434) + note-name labels** —
  TIM-7/SEQ-29 omit two critical specs: (1) the iPad parser must handle
  **packed/Type-0 single-track merged-channel** `.mid` files per desktop's
  2026.10 fix (#6434), not only multi-track Type-1; (2) AVFoundation `MIDIFile`
  is a viable iOS-native alternative to relocating wx `LoadMIDIFile`; (3)
  acceptance: pick .mid via `.fileImporter`, choose target timing track, marks
  land at note-on times **with note-name labels**. — `LoadMIDIFile`
  `tabSequencer.cpp`; `phase-b-grid-parity.md` B77 + master tracker B-77. —
  enrich **TIM-7**.
- **IO-24 LyricUserDictDialog (custom phoneme dictionary)** — *Note:* TIM-10
  already covers the phoneme user-dictionary editor (`LyricUserDictDialog.cpp` +
  `PhonemeDictionary::InsertPhoneme/RemovePhoneme`, user_dictionary read at
  `iPadRenderContext.cpp ~1677`). The `future-imports-exports.md` row flagging
  IO-24 as "missing" is **superseded by TIM-10** — verify TIM-10 references
  LyricUserDictDialog by name; no new item needed. — reconcile under **TIM-10**.
- **(DETAIL) A-5 Audio scrub during drag** — verify whether iPad scrubs audio on
  playhead drag; if not, add a small grid-gesture-polish note. — **TIM-11**.
- **(DETAIL) R-1 video codec failure at RENDER time** — MED-3 surfaces
  incompatible-video warnings on IMPORT; the prior R-1 wants warn-on-RENDER for
  VP9/AV1/ProRes-RAW/WMV. Add a render-time codec-failure surfacing note +
  standing risk (no FFmpeg fallback; canonical failure H.264 High 4:4:4 <64px →
  VideoToolbox -8969). — enrich **MED-3** + Risks.
- **(DETAIL) EX-16 Generate Lyrics From Data — preserve detail in deferred note**
  — keep the `.xfacemap` reconstruction detail and the "distinct from B79 AI
  Speech-to-Lyrics" clarification in 02's deferred note. — `GenerateLyricsDialog.cpp`;
  `_raw-gap-analysis.md:659`. — enrich 02 deferred note.

### → `03-effects-panels.md`

- **G2-c Shader dynamic uniform grouping (`GLSL_GROUP:`)** — Add a P2 item:
  respect `GLSL_GROUP:` comment conventions in
  `ShaderConfig::GetDynamicPropertiesJson()` so 20+-uniform packs group into
  sections instead of a flat scroll in `ShaderDynamicParamsView`; shared-core
  change (benefits desktop too); deferred until a real pack trips it. —
  `src-core/effects/ShaderEffect.cpp:1549`; reported by master tracker, EI-6,
  and `followups.md` G2-c. — **FX-13**.
- **O-12 RemapDMXChannelsDialog — full grid + .xdmxmap CSV** — Add a dedicated
  item (or expand FX-6): full From/To/Scale/Offset/Invert grid (48 rows) across
  selected DMX effects, with **.xdmxmap CSV load/save**. iPad has only a reduced
  6-preset menu. Rated P2/M by prior plans (higher than FX-6's "lower-value
  follow-up" framing). — `src-ui-wx/setup/RemapDMXChannelsDialog.cpp`;
  `_raw-gap-analysis.md:241/820`. — **FX-14**.
- **R-2 Silent PicturesEffect format failures** — Add a small note: warn when an
  image format (JPEG2000, WebP variants) can't decode on iOS. — `PicturesEffect`;
  near FX-11. — **FX-15**.
- **BE-3 BulkEditFontPickerDialog** — Add `fontpicker` to FX-5's enumerated
  bulk-apply control types (apply one font to N Text effects); verify the iPad
  Text effect has a font picker at all. — `src-ui-wx/sequencer/BulkEditFontPickerDialog.cpp`.
  + also `future-aux-panels.md` BE-3. — enrich **FX-5** (call it **FX-5b**).
- **U-18 BufferSizeDialog** — Verify the Buffer inspector tab covers custom W/H
  buffer sizing; if not, add a small (P3) custom buffer-size entry. —
  `src-ui-wx/sequencer/BufferSizeDialog.cpp`. — **FX-16**.
- **Pictures/GIF per-frame TIMING editor (G6)** — Add an FX item: author
  per-frame durations / frame selection for animated-GIF Pictures (the headline
  of `future-pictures-frame-editor.md`). FX-11 is read-only preview, FX-12 is
  paint — **neither** is a frame-timing editor. Carry the deferral rationale
  (niche; desktop impl is poor and due for redesign — co-design once, land on
  both, consider moving per-frame-timing into `src-core` `AnimatedImageData`). —
  `xlGridCanvasPictures.h:98-99` (vestigial, frame 0 hardcoded). — **FX-17**.
- **(DETAIL) FX-12 Pictures paint — add eyedropper/eraser, palette manager,
  color picker, image new/load/save/resize, PNG/BMP/JPG/GIF/WEBP** — EA-2 lists
  these; FX-12 mentions only paint+copy/paste/clear. — `PicturesAssistPanel.cpp:116-203`.
  — enrich **FX-12**.
- **(DETAIL) Sketch residual desktop-only ops** — Sketch is treated as done, but
  cubic/quadratic curve creation, path closing, and **SVG import/export** remain
  desktop-only. Add a small follow-up. — `_raw-gap-analysis.md:192`. — **FX-18**.
- **(DETAIL) Morph 4-corner quad parity** — verify the iPad
  `MorphLineEditorRowView` matches the desktop 4-corner quadrilateral editor
  (region drag, start/end linking) vs endpoint-only. — `xlGridCanvasMorph`. —
  enrich Morph note.
- **(DETAIL) DM-17 DMX per-channel buddy text fields** — FX-6 defers per-channel
  buddy text fields; call the gap out explicitly. — enrich **FX-6**.
- **(DETAIL) DM-18 MovingHead specialised VC domains** — verify each VC domain
  (Fan Pan/Fan Tilt/Path Scale/Time Offset) is present in FX-9, not just
  pan/tilt. — enrich **FX-9**.
- **(DETAIL) FX-9 Moving Head waypoint canvas — current-state** — note the iPad
  Path tab today shows the existing Path: value + Clear (a partial); FX-9's
  waypoint canvas is the G3+ piece, can land incrementally as an upgrade of that
  row using the proven Sketch/Morph inline-custom-row template. — enrich **FX-9**.
- **(DETAIL) EI-5 Pictures GIF editor blocking dependency** — note FX-11/FX-12
  inherit the desktop-redesign block (`plans/future-pictures-frame-editor.md`).
  — enrich FX-11/FX-12.

### → `04-color-and-value-curves.md`

- **(DETAIL) COL-10 ColorManager — 38-swatch breakdown + tester-demand priority**
  — Copy the structure: 5 Timing-track colours (Fixed/Single-Mark/Bracket/
  Phrase/Word), 28 Effect-grid colours, 4 Layout colours, 2 global text colours,
  Suppress-Dark-Mode toggle, Import/Export/Reset. Note B25 (bracket palette)
  already routes through show-folder XML — full ColorManager extends that
  round-trip. **Priority conflict:** prior plan rates ColorManager the #1
  deferred item (tester demand for darker/higher-contrast themes); new COL-10
  defers it "unless a broader Preferences screen lands" — record the
  tester-demand rationale so the downgrade is a conscious decision. —
  `src-ui-wx/color/ColorManager.cpp` + `ColorManagerSettingsPanel.cpp`;
  `future-preferences.md` PR-3. — enrich **COL-10**.
- **(DETAIL) MA-11 ModelDimmingCurveDialog 4 modes** — reflect the 4-mode
  structure (Single B/G, Single Curve File, RGB per-channel, RGB Curve Files) +
  per-channel visualisation panels in COL-11 (editor exists; COL-11 only adds
  the preview canvas). — enrich **COL-11**.
- **(DETAIL) SEQ-2 / COL-5 ColourReplace dedup** — SEQ-2 and COL-5 are the same
  deliverable described twice; cross-reference so it isn't built twice. — enrich
  both.
- **(DETAIL) BE-4 BulkEditSlider numeric+VC** — note the desktop dialog handles
  both a flat numeric value and a value-curve in one dialog; confirm COL-2 (VC
  half, gated `selectedEffects.count>1`) plus the numeric path both land. —
  enrich **COL-2**.

### → `05-layout-models.md`

- **Authenticated vendor downloads** — Add a low-priority, on-demand item: add a
  credential/cookie/`URLSessionConfiguration` hookup to `CachedFileDownloader`
  (the iPad NSURLSession fetcher hook exists at `CachedFileDownloader.h:83-93`)
  so `VendorBrowserSheet` can fetch behind-login catalog content. Trigger:
  catalog starts gating models behind accounts. — `XLVendorCatalog.mm:145`
  (zero auth refs); `VendorCatalog.cpp` routes all fetches anonymous. Reported by
  master tracker + phase-j-layout-editor. — **LAY-36**.
- **LA-11 Model locking** — Add a small (S) per-model lock toggle (prevent
  moves/edits) with tree + property-grid lock indicator. — `Model::IsLocked/Lock`
  in `src-core/models/Model`; no toggle in src-iPad LayoutEditorView. — **LAY-37**.
- **WV-4 Wiring diagram PNG export** — Add a sub-item to LAY-7: export wiring
  diagram as PNG (standard + large), via Metal/Canvas render-to-image + share
  sheet. — `WiringDialog.cpp:733/742`. — **LAY-38**.
- **WV-5 Wiring diagram DXF export** — Add a sub-item to LAY-7: per-model
  wiring-diagram DXF (`WiringDialog::Export_DXF` using `cad/DXFWriter`
  WriteWire/WriteNode), distinct from layout-level DXF (LAY-35). Marginal on iPad
  like other CAD exports. — `WiringDialog.cpp:745/959`. — **LAY-39**.
- **WV-2/WV-3 Wiring theme picker + Front/Rear + 90° rotation** — minor P3
  sub-details of LAY-7: Dark/Gray/Light theme picker, Front/Rear flip, 90°
  rotations. — `WiringDialog.cpp:672-798`. — **LAY-40** (fold into LAY-7).
- **AP-3 SelectPanel — select existing effects by criteria** — Add a (S) item:
  select existing placed effects by **effect-type / buffer-style / color (with
  sensitivity slider)**, scoped to a model multi-select, with the four
  Select-All buttons. (The raw dataset misread this as an add-effect picker;
  it is a selection FILTER.) *Note: target is arguably `10` (View panels) — see
  that doc; cross-listed here because it touches selection.* —
  `src-ui-wx/sequencer/SelectPanel.cpp`; `_raw-gap-analysis.md:194`. — **LAY/VIEW-7**.
- **(DETAIL) Sidebar multi-select + accessibility audit (Touch UX)** — switch the
  Models roster List from `Binding<String?>` (`layoutSelectSingle`) to a
  multi-selection `Set<String>` gated by `EditMode=.active` with checkmark rows;
  feed into the existing multi-select model. Add the accessibility/VoiceOver
  audit (parallel roster+inspector path so non-gesture editing works). —
  `LayoutEditorView.swift:2378/932`; `DisplayElementsSheet.swift:527` proves the
  EditMode pattern exists. — **LAY-41**.
- **(DETAIL) LAY-25 ImportPreviewsModels scope decision** — phase-j listed
  ImportPreviewsModelsDialog out-of-scope; LAY-25 promotes it to in-scope P2/L.
  Reconcile the scope decision (don't assume out-of-scope). Also preserve the
  S-pro one-liner "copy model layout from another show". — enrich **LAY-25**.
- **(DETAIL) LAY-35 CAD export — STL/VRML, not just DXF** — desktop
  `ModelToCAD::ExportCAD` supports dxf/stl/wrl. New docs dropped STL/VRML.
  Per-MODEL CAD (DXF/STL/VRML) is out-of-scope; LAYOUT-level DXF is LAY-35 —
  preserve that split and the format list. — `src-core/cad/ModelToCAD.cpp:25-30`.
  — enrich **LAY-35**.
- **(DETAIL) MA-1 Custom Model "Import From Controller"** — discover controllers
  → download the custom-model layout; not covered by LAY-20..22/33 (only in 05's
  out-of-scope as "deferred P3"). Record it. — `CustomModelDialog.cpp:3265`. —
  enrich **LAY-20/33**.
- **(DETAIL) MA-7 NodeSelectGrid feature list → LAY-2** — copy ordered selection,
  background-IMAGE overlay (deprioritized note), brightness, find/search,
  output-to-lights into LAY-2's scope. — `NodeSelectGrid.cpp`. — enrich **LAY-2**.
- **(DETAIL) MA-13 Generate Node Names (DMX-only button)** — text strand/node
  naming is shipped; the conditional auto "Generate Node Names" button (DMX
  models only) is not. — `StrandNodeNamesDialog.cpp`; `_raw-gap-analysis.md:345`.
  — small add near strand-node naming.
- **(DETAIL) LA-12 / MA-1 bulk-edit & custom-model op enumerations** — copy the
  full LA-12 bulk-edit op list (Tag/Pixel-Size/Transparency/Brightness/Gamma/
  Colour-Order/Null-Nodes/Group-Count/Preview/Dimming) into LAY-10, and the full
  MA-1 op list (compress/trim/shrink/expand/push-pull-between-layers) into
  LAY-20/22. — enrich **LAY-10 / LAY-20 / LAY-22**.
- **(DETAIL) O-6 controller per-type property enumerations** — copy the base
  property list (Default Port Brightness/Gamma, Suppress Duplicate Frames,
  Monitor, Full xLights Control) and serial Port/Speed/Prefix/Postfix detail.
  *(Target overlaps 07; place the controller half in 07/CTL-1/2 — see that doc.)*
- **(DETAIL) S-pro residual: multi-target Replace-Model + relative-start-channel
  preservation UI** — carry forward from the retiring `future-layout-editing.md`:
  (1) multi-target Replace-Model dialog; (2) relative-start-channel preservation
  UI (editing-side counterpart to the #6447 import fix). P3. — enrich LAY items.

### → `06-layout-submodels-dmx-preview.md`

- **DM-13 PositionZoneDialog** — Add a P3 item: DMX position-zones
  collision-avoidance grid (Pan Min/Max, Tilt Min/Max, Channel, Value rows) for
  MovingHeadAdv; governed by the `EnablePositionZones`/`ShowZoneIndicator` prefs.
  — `src-ui-wx/setup/PositionZoneDialog.cpp`. — **LAY-31**.
- **R-8b descriptor-driven DrawAxisTool / DrawActiveAxisIndicator** — Add a
  very-low-priority internal shared-core refactor item: port the axis-gizmo
  helper to a descriptor stream (currently reads `axis_tool`/`active_axis`/
  `highlighted_handle` directly); per-subclass DrawHandles inline math already
  descriptor-driven. Gates on first consumer needing per-axis styling. —
  `src-core/models/ModelScreenLocation.cpp`; reported by master tracker +
  followups R-8b. — **LAY-32-int**.
- **Multi-select body drag (Touch UX)** — Add: bridge must capture a per-selected
  offset table at drag-begin and apply the delta to ALL selected models on
  update+commit (mirror align/distribute/match-size per-model undo). Desktop ref
  `LayoutPanel.cpp:5961-5984`. — iPad `beginBodyDrag3D`/`dragBody3D`
  `XLMetalBridge.mm:1928/1986` are single-model. — **LAY-33-touch**.
- **(DETAIL) LAY-21 View-Objects multi-select + drag-reorder (VO-6)** — LAY-21
  covers VO align/distribute/flip/resize + unlink-from-base but **not** VO-6's
  actual ask: multi-select + drag-reorder on the Objects sidebar list (reuse the
  Controllers `.onMove`+handleReorder pattern). Also **verify** VO unlink-from-
  base is real (the prior plan says it was a Controllers-tab feature, not a VO
  one). — `ViewObjectPanel.cpp:68-78`. — enrich **LAY-21**.
- **(DETAIL) VO-4 Terrain brush-paint vs LAY-23 / "Real Spacing"** — confirm iPad
  terrain editing includes the brush-paint heightmap tool (XL part), and fold
  the GridlinesObject "Real Spacing" readout into LAY-23 (RulerObject::
  ConvertDimension family). — enrich **LAY-23**.
- **(DETAIL) MA-9/MA-10 phoneme/eyes/placement & ColorDrawMode enumerations** —
  copy the 10 phonemes (AI,E,ETC,FV,L,MBP,O,REST,U,WQ) + eyes (Open/Closed/
  Open3/Closed3) + Image Placement (Centered/Scaled/Aspect/Crop) into LAY-3/5/10;
  copy States Color-Draw-Mode (All Colors / White Only) + 200-row grid into
  LAY-4/17. — enrich **LAY-3/4/5/10/17**.
- **(DETAIL) MA-14 7-Segment reference image** — add the segment-layout reference
  diagram to LAY-17. — enrich **LAY-17**.
- **(DETAIL) MA-15 MatrixFaceDownload UI + URL correction** — copy the UI
  breakdown (tree navigator categories/artists, search, 256×128 preview, Insert
  Face) into LAY-28, and **correct the catalog URL**: current code uses
  `raw.githubusercontent.com/xLightsSequencer/xLights/refs/heads/master/download/
  xlights_faces.xml` (NOT the stale `nutcracker123.com/...`). ZIP-per-phoneme-PNG
  format still accurate; gate behind LAY-10. — `MatrixFaceDownloadDialog.cpp:396/401`.
  — enrich **LAY-28**.
- **(DETAIL) DM-9 ColorWheel editing** — verify ColorWheel (1 wheel + 1 dimmer +
  up to 25 custom colours with DMX value mapping) editing coverage. — enrich
  DMX per-type editors.
- **(DETAIL) #6430 rename refreshes all pickers (LAY-19/LAY-26)** — append to the
  preview/layout-group rename items: after rename, refresh every preview/
  layout-group picker (picker sheets, active-group selector, model-property
  layout-group field) the way `createLayoutGroup` already does. — enrich
  **LAY-19 / LAY-26**.

### → `07-controllers-and-upload.md`

- **O-11 PixelTestDialog — network test-pattern panel** — Add a numbered P2/XL
  item (not just the deferred parenthetical): network output (sACN/ArtNet/DDP/
  E1.31) is already real on iPad via `startOutput`/`outputFrame`, so a
  test-pattern engine is feasible; serial/USB-DMX test is the only out-of-scope
  slice. Copy the spec: 12 functions (Off, Chase, Chase 1/3·1/4·1/5, Alternate,
  Twinkle 5/10/25/50%, Shimmer, Background), per-RGB tabs, 4 cascading selection
  trees (Outputs/Groups/Models/Controllers), speed/highlight/background sliders,
  Save/Load presets, embedded preview, ChannelTracker overlap-merge. Flag the
  core `TestFunctions`→channel-frame-generator extraction as the prerequisite
  (why it's XL). — `src-ui-wx/setup/PixelTestDialog.cpp`; `_raw-gap-analysis.md:248`.
  Reported by master tracker, gap-analysis, future-controller-upload, README. —
  **CTL-20**.
- **O-7 Discover HTTP-scan-with-auth + DiscoveryAuthDialog** — Add a P4/P3 item:
  HTTP-scan discovery for Falcon / Pixlite16 / Twinkly + a DiscoveryAuthDialog
  credential prompt. Bonjour + UDP broadcast + mismatch-resolution already
  shipped (J-31). — `src-ui-wx/setup/DiscoveryAuthDialog.cpp`,
  `DiscoveryHelpers.cpp`. Reported by master tracker + phase-j O-7. — **CTL-21**.
- **O-8 ControllerConnectionDialog (legacy add wizard)** — Add a note (likely
  superseded by the native Add Ethernet/Serial/Null flow): either document as
  superseded-by-descriptor-Add, or list as low-priority. — `ControllerConnectionDialog.cpp`.
  — **CTL-22** (or fold into CTL-19 as a "superseded" note).
- **O-13 Output-to-Lights fault notification mid-show** — Add: when output is
  enabled and a previously-reachable controller goes unreachable, surface a
  non-modal fault notification (distinct from CTL-8 static reachability ping). —
  output enable `tabSequencer.cpp:3439`, ping `TabSetup.cpp:1061`. — **CTL-23**
  (sub-bullet of CTL-8/CTL-12).
- **FPP Connect Slice C — Host/IP sort, Cape/Upload/Media bulk-toggle menus** —
  Add P3 items for the FPP Connect SHEET (not the Setup tab): HostName/IP column
  sort (numeric via `SplitIP`, `FPPConnectDialog.cpp:340-378`); Cape
  Select-All/Deselect-All (`:394-420`); Upload Select-All/Deselect-All/Sort-by-
  Upload/**Select-Subnet** (`:422-457`); Media Deselect-All (`:493-514`). —
  **CTL-24**.
- **FPP Connect Slice D — Failed-uploads retry + pre-select from Batch-Render** —
  Add P2 items: persist last-run failures to a CSV key (desktop
  `FPPConnectFailedList`, `:745/1065/1463`) for one-tap re-select; pre-select
  sequences from the last Batch-Render selection (`BatchRendererItemList`,
  `:741-742`). — **CTL-25**.
- **FPP Connect Slice G — settings-persistence parity** — Add: persist FPP
  Connect sequence selection (`FPPConnectSelectedSequences` CSV), filter mode,
  folder, and sequence sort col/order across sessions, mirroring desktop keys
  (`FPPConnectDialog.cpp:1567-1597/1622-1654`). Per-instance upload toggles
  already persist via `FPPInstanceConfigStore` — confirm semantics parity
  (Models 0=None/1=All/2=Local; UDP 0=None/1=All/2=Proxied). — **CTL-26**.
- **FPP Connect Slice H — UX status-string parity** — Add a small item: mirror
  desktop phase strings ("Preparing Configuration", "Checking Media and FSEQ
  file for {media}/{fseq}", "Preparing FSEQ File for {fseq} ({i}/{n})",
  "Uploading {fseq} ({i}/{n})", "FPP Connect Upload Complete"/"Cancelled"/"…had
  errors or warnings"; error title "Problems Uploading"). — **CTL-27**.
- **(DETAIL) CTL-5 Add-FPP-by-IP — validation + persistence + auth-already-done**
  — validate via `ip_utils::IsIPValidOrHostname`; after forced-address probe run
  `AuthenticateAndUpdateVersions` + `probePixelControllerType`; persist forced
  IPs (`FPPConnectForcedIPs`, '|'-delimited). **Note the 401/Keychain auth half
  is already shipped** — scope CTL-5 to Add-by-IP + Re-Discover UI only. —
  `FPPConnectDialog.cpp:1737-1787/1682-1735`. — enrich **CTL-5**.
- **(DETAIL) CTL-6 sequence picker** — copy desktop specifics: folder dropdown
  from `GetFolderList` over show-folder subdirs (`:1799-1826`); filter "Recursive
  Search" vs "Only Current Directory" (`:132-134`); Date-Modified
  (`:185-187/1832`, reuse SequenceDatesLabel formatter); Channel-Count via
  `getChannelCount()` (`:191-193/1836-1847`). — enrich **CTL-6**.
- **(DETAIL) CTL-11 FSEQ-type mapping** — copy the per-fppType FSEQ-type-index
  mapping that drives `PrepareUploadSequence`'s `type` arg (FPP: 0=V1, 1=V2-zstd,
  2=V2-Sparse-zstd, 3=V2-Sparse-uncompressed; FALCONV4V5/POWERDMX: 0=V1, 1→5
  zlib, 2→6 sparse-zlib, 3=sparse-uncompressed, 4=uncompressed; ESPIXELSTICK/
  Genius: fixed 3). — `FPPConnectDialog.cpp:593-614/1242-1252`. — enrich **CTL-11**.
- **(DETAIL) CTL-7 / Slice E proprietary codec exclusion** — name the three
  excluded vendor codec paths (Falcon V4/V5 `:1336-1364`, Genius `:1365-1384`,
  PowerDMX `:1385-1404`); iPad behavior is to filter non-FPP fppType out of the
  sheet (or disable with a "Use desktop xLights" hint). Distinct from CTL-7's
  BaseController immediate-output path. — enrich **CTL-7** + 99.
- **(DETAIL) CTL-15 Sort controllers — SAFETY rationale (CONFLICT)** — phase-j /
  master tracker **deliberately dropped** controller sort because list order
  drives start-channel assignment and a stray sort tap silently re-channels every
  model with no undo. CTL-15 proposes re-adding it. Either gate behind a confirm
  + auto-recalc + **undo snapshot**, or warn that sorting reassigns channels.
  Copy this rationale into CTL-15's risk note. — `OutputManager::SortControllersby*`.
  — enrich **CTL-15**.
- **(DETAIL) CTL-9 full 13-column list** — copy the full desktop column set
  (Name/Protocol/Address/Universes/Channels/Vendor/Model/Variant/Active/
  AutoLayout/AutoSize/Description/Status); CTL-9 lists only a partial subset. —
  enrich **CTL-9**.
- **(DETAIL) O-5 per-controller menu — Upload Output wording check** — verify the
  iPad "Upload Output" action wording (`pendingUploadConfirmName`) matches
  desktop; Activate / Activate-xLights-Only / Inactivate / Unlink-from-Base
  already shipped in `controllerContextMenu`. — enrich **CTL-19**.
- **(DETAIL) CTL-4 IPEntryDialog is dead code** — note the desktop
  `IPEntryDialog` is orphaned (#included at `xLightsMain.cpp:83`, never
  instantiated); CTL-4's inline dest-IP sheet is net-new, not a port. — enrich
  **CTL-4**.
- **(DETAIL) CTL-1/CTL-2 per-type property enumerations (O-6)** — copy base
  props (Default Port Brightness/Gamma, Suppress Duplicate Frames, Monitor, Full
  xLights Control) + serial Port/Speed(8 baud)/Prefix/Postfix + ethernet
  Multicast/IP/FPP-Proxy/Protocol(10)/Force-Local-IP/Priority/Universe-count/
  Per-String/Individual-Sizes. — enrich **CTL-1 / CTL-2**.
- **(DETAIL) CTL-16 iOS-26 swipe-delete pitfall** — add an implementation note:
  do NOT use `Button(role:.destructive)` inside `.swipeActions` on iOS 26 (it
  pre-commits row removal → off-by-one `NSInternalInconsistencyException`); use
  plain `Button{}.tint(.red)`. CTL-16's multi-select rework could regress this.
  — enrich **CTL-16**.
- **(DETAIL) O-10 ControllerModelDialog residuals** — most port-mapping authoring
  already ships (`ControllerVisualizeView`); remaining: per-port bank
  visualisation + auto-layout flag, in-diagram validation overlays, enrich the
  visualizer long-press menu to match desktop in-place per-port/per-model menus,
  and the cross-selection **Port+Increment** bulk-edit helper. — enrich **CTL-19**
  + smart-remote colour banding into **CTL-13**.
- **FPP Connect Slice F (Slice F note)** — *stale:* parallel transcode +
  concurrent curl + per-FPP gauges shipped 2026-05-19; recorded so CTL-6/7/11
  aren't treated as re-listing it. (See § STALE appendix.)

### → `08-import-export.md`

- **IO-6 Vixen 3 FULL-sequence import** — Add a row: full-sequence Vixen 3 effect
  import (the biggest single P2 import). IE-1/TIM-2 cover only Vixen3 .tim
  **timing**; IE-18 lists HLS/Vixen2/LSP/VSA but **omits Vixen3 full sequence**.
  Carry the "biggest single P2 import, pull in if a vendor flags it" rationale.
  — full parse `src-ui-wx/import_export/Vixen3.cpp`. — **IE-25**.
- **IO-20 SeqElementMismatchDialog (on-open Map-Models)** — Add a P2 item: the
  on-open mismatch resolver shown when a loaded sequence references models not in
  the current layout (remap mismatched elements). Note the alias half already
  shipped (IE-12 only covers writing aliases). — `src-ui-wx/diagnostics/
  SeqElementMismatchDialog.cpp`. Reported by master tracker + gap-analysis +
  future-imports-exports. — **IE-26**.
- **(DETAIL) IE-5 — `.las` extension + LegacySequenceReader naming + parked
  priority** — confirm `.las` (LOR animation, no audio) is handled alongside
  `.lms` via the same `ReadLorFile` path; note the target-filename decision
  (LegacySequenceReader vs the prior LmsImporter); carry "distant third format,
  parked until a vendor request lands" priority. — `FileConverter.cpp:219`. —
  enrich **IE-5**.
- **(DETAIL) IE-4 LOR S5 .loredit — severity** — prior plan rates P3/M; IE-4
  rates XL. .loredit is the current LOR product, so user-value may exceed P3 —
  reconcile. `LOREdit.cpp` confirmed; resolves the prior "hoist LOREdit with LMS
  or defer" question (it goes with S5/IE-4, LMS uses FileConverter/IE-5). —
  enrich **IE-4**.
- **(DETAIL) EX-1 / IE-20 — enumerate the 14 export formats** — copy the explicit
  list into IE-20: LOR .lms/.las, LOR Clipboard .lcb (v3), LOR S5 Clipboard,
  Vixen .vix, Vixen Routine .vir, LSP, HLS .hlsnc, FSEQ (done), Minleon NEC .bin;
  and the OUT-OF-SCOPE video/GIF (.mp4/.avi/.mov/.gif). Note desktop Export
  Models/Effects route through SeqExportDialog. — `SeqExportDialog.cpp`. — enrich
  **IE-20**.
- **(DETAIL) EX-2 / IE-20 — ExportModelSelect multi-model picker** — add the
  multi-model selection picker + "small standalone PR, engine path already there"
  rationale (`exportModelAsFSEQAtRow:toPath:` exists). — enrich **IE-20 / EXPORT-RH**.
- **(DETAIL) EX-13 / IE-14 — XLSX vs CSV format discrepancy** — desktop Export
  Effects writes **.xlsx**; IE-14 plans CSV. Reconcile: match desktop XLSX
  (consistent with IE-15) or note the iPad CSV simplification + why. —
  `ExportEffects.cpp`. — enrich **IE-14**.
- **(DETAIL) CV-1 / IE-24 — full format counts + LOR Import Options** — copy 6
  inputs (.fseq/.lms/.las/.vix/.gled/.seq/.hlsidata) / 9 outputs (incl. Lynx
  Conductor/Glediator/LedBlinky .lwax) and especially the **LOR Import Options**
  block (Map Empty Channels, Verbose Channel Map, Time Resolution 25/50/100ms)
  which IE-24 omits. — `ConvertDialog.cpp`. — enrich **IE-24**.
- **(DETAIL) IO-19 / IE-13 — MediaImportOptionsDialog folder-mapping** — add the
  exact dialog name + "folder mapping" framing (remap media folder paths on
  import), distinct from .xsqz package destinations. —
  `src-ui-wx/media/MediaImportOptionsDialog.cpp`. — enrich **IE-13**.
- **(DETAIL) I-2 carry-forward — model-blending toggle source label** — IE-22's
  toggle should pair with surfacing the SOURCE-file blending-state label
  (`SetModelBlending()`), and the desktop control `CheckBox_Import_Blend_Mode`
  defaults ON. — `xLightsImportChannelMapDialog.cpp:593`. — enrich **IE-22**.
- **(DETAIL) I-2 carry-forward — convertRender persistence** — IE-2 should
  persist the choice (desktop config key `ImportEffectsRenderStyle`, default
  false). — enrich **IE-2**.
- **(DETAIL) IE-6/9/10 — Auto Map false-positive risk** — add to 08 Risks: Auto
  Map false-positives on similar model names are a known issue; the iPad's
  smaller review screen makes the find/filter (IE-9), Hide-Unmapped, and
  per-row effect-count badge (IE-6) the mitigation, not just convenience. —
  enrich **08 Risks**.
- *(stale)* SuperStar .sup, Package Sequence (.xsqz), xmodel start-channel/relative
  fixes, media-relink basename split — see § STALE appendix.

### → `09-file-lifecycle-render-tools.md`

- **JobPool requeue redesign + deadlock RISK** — Add a shared-core deferred/risk
  item: the JobPool "workers block on other-model-frame" pattern can deadlock
  complex sequences; the redesign replaces block with re-enqueue. iPad workaround
  today is raising the thread count. Desktop-scope refactor (benefits both
  clients). — shared core JobPool/RenderEngine `iPadRenderContext.cpp:1754`;
  `_raw-gap-analysis.md:1057` only notes JobPool/progress Partial for UI reasons,
  not the deadlock. — **RENDER-2** (item) + **09 Risks** (risk).
- **Purge Render Cache action** — Add a sibling to TOOLS-1: a user-invokable
  "Purge Render Cache" (trivial bridge call to the existing `RenderCache` purge +
  Tools entry). iPad already owns a RenderCache (`iPadRenderContext.h:610`). —
  `xLightsMain.cpp:1135/7827`. — **TOOLS-1b**.
- **L-6 External-change watcher for rgbeffects** — *Note:* prior plans tracked
  this, but a code grep found **no `wxFileSystemWatcher`/rgbeffects-reload
  watcher on desktop** — it was aspirational, not a shippable desktop feature.
  Add a one-line deferred note (relevant for iCloud multi-device); low value. —
  **09 deferred note** (or 99).
- **(DETAIL) Memory-pressure standing risk** — the 4GB low-def-render lock is
  captured (99 + 09), but add the broader standing risk: Phase-A memory-pressure
  handling is in place yet under-tested; external tester reports are the first
  stress signal. — enrich **09 Risks** (or 00).
- **(DETAIL) L-5 stale-bookmark reprompt** — promote the "reprompt when a
  security-scoped bookmark goes stale" from a Risks note to a discrete small LIFE
  item. — `ObtainAccessToURL` failure path. — enrich **LIFE-1/LIFE-2**.
- **(DETAIL) EX-3 / BR — Force HD deliberately OUT + already-shipped columns** —
  record that BatchRenderSheet date columns + sort modes + "Out of date only"
  filter ALREADY shipped (don't re-plan), and carry the deliberate DECISION that
  Force HD is out of batch render (HD precision tracks sequence settings, not a
  per-batch override) so it isn't re-added. — enrich **BR-1/BR-2**.
- **(DETAIL) L-2 / BKP-1 .xsqz reuse** — confirm whether the Phase-G .xsqz
  packager already exists and can be reused by BKP-1's ZIP routine. — enrich
  **BKP-1**.
- *(stale)* Data Layers tab (correctly deferred), Recent Show Folders,
  SaveChanges/Close-dirty prompt, base-show-folder toggle, Check Sequence — see
  § STALE appendix.

### → `10-presets-jukebox-view.md`

- **AP-3 SelectPanel — select existing effects by criteria** — Add a dedicated
  (P2/S) item (e.g. **VIEW-7**): select existing placed effects by effect-type /
  buffer-style / color (with color-sensitivity slider), scoped to a model
  multi-select, with four Select-All buttons. This is a selection FILTER (the raw
  dataset misreads it as an add-picker). Overlaps SEQ-3 global Select-All but is
  a filtered superset. — `src-ui-wx/sequencer/SelectPanel.cpp`;
  `_raw-gap-analysis.md:194`. Reported by gap-analysis AP-3, followups #6389,
  README. — **VIEW-7**.
- **AP-6 TopEffectsPanel — recently-used quick-bar** — Add a small (P3) item or
  fold into VIEW-1: an auto-populated "recently/most-used effects" quick-access
  strip (distinct from VIEW-1's full palette/dropper icon upgrade). —
  `TopEffectsPanel.cpp`. — **VIEW-8**.
- **Presets-file autosave + restore-on-open prompt** — Add a (low-priority) item:
  desktop saves an xeffects backup (`XLIGHTS_PRESETS_FILE_BACKUP`) and on next
  open prompts to restore a newer backup (`TabSequence.cpp:293-318`, `wxRenameFile`).
  PRE-1..3 persist presets to the rgbeffects `<effects>` node but have no
  corrupt/lost-presets recovery path. — **PRE-11**.
- **(DETAIL) B16 drag-from-palette — record deliberate deferral** — note near
  VIEW-1: the live-ghost drag-from-palette is deliberately NOT ported (tap-to-arm
  + tap-to-place is the chosen touch idiom; revisit only if users ask for
  drag-cancel mid-gesture). — `phase-b-grid-parity.md` B16. — enrich **VIEW-1**.
- **(DETAIL) AP-1 SearchPanel — affordances + priority** — VIEW-4 omits the
  desktop affordances: Model-name filter dropdown, **Regex toggle**, results LIST
  (value + [start,end] + effectName + model + live count), and a **Select-All
  from results** button. Clarify scope: searches effect SETTINGS values and skips
  timing tracks. **Priority:** prior plan rates SearchPanel P1 (highest-value
  navigation aid on large sequences); VIEW-4 is P2/P3 — reconcile or record the
  rationale so the downgrade is conscious. — `SearchPanel.cpp`. — enrich **VIEW-4**.
- **(DETAIL) HidePresetPreviews toggle** — add a toggle in the presets panel (or
  PREF-16) to suppress thumbnail previews on large libraries. — `ViewSettingsPanel.cpp`.
  — enrich presets panel.
- **(DETAIL) AP-8 / VIEW-6 reference-video framing + priority** — preserve "embedded
  REFERENCE video synced to playhead" framing (SequenceVideoPreview, a
  scrub/scratch reference, not the model preview); reconcile P2 (prior) vs P3
  (VIEW-6). — enrich **VIEW-6**.
- **(DETAIL) AP-2 / VIEW-5 — "search by attribute" framing + B24 cross-ref** —
  preserve the "query effects whose rendered output matches a brightness/hue/
  channel-value criterion" framing, and the legacy "covers B24" cross-reference.
  — enrich **VIEW-5**.

### → `11-preferences-settings.md`

- **PR-7 RandomEffects preference panel** — Add a P3 item: per-installed-effect
  inclusion toggles (~80 checkboxes) controlling which effect types the
  random-fill (SEQ-26) can pick. Without it the iPad random generator has no way
  to exclude effects. — `RandomEffectsSettingsPanel.cpp` (`OnEffectCheckBoxClick`).
  Reported by master tracker PR-7 + future-preferences. — **PREF-20**.
- **PR-4 DoubleClickMode** — Add (name explicitly): effect-grid double-click
  behavior mode. iPad uses double-tap but has no settable mode. Low value on
  touch; record port-vs-drop decision. — `EffectsGridSettingsPanel.cpp`. — **PREF-21**.
- **PR-4 HideColorWarning** — Add: suppress the palette-color-clash warning.
  Small @AppStorage Bool. — `EffectsGridSettingsPanel.cpp`. — **PREF-22**.
- **PR-5 ExcludeVideos / ExcludeAudio from packaged sequences** — Add two
  checkboxes (to the iPad Package action or PREF-18). —
  `OtherSettingsPanel.cpp:265-266`. — **PREF-23**.
- **PR-5 PromptBatchIssues** — Add: toggle for prompting on batch-render issues.
  — `OtherSettingsPanel.cpp`. — **PREF-24** (or fold into BR cluster).
- **PR-5 ControllerPingInterval pref** — *Note:* belongs in **07** (no
  ping/online item exists there at all) — see CTL-8 / CTL-23. Record here that
  the pref is a desktop feature (`OtherSettingsPanel.cpp:278`). — cross-ref **07**.
- **PR-5 ModelRenamingAlias prompt behavior** — Add a one-line note: iPad always
  prompts via MissingModelAliasSheet; no persistent Always-Prompt/Yes/No pref.
  Low value. — `_raw-gap-analysis.md:1169`. — **PREF-25**.
- **PR-10 ShowPlayControlsOnPreview** — Add (or note touch-handled): overlay
  transport controls on the preview. — `ViewSettingsPanel.cpp`. — **PREF-26**.
- **PR-10 TimelineZooming** — Add (note touch-equivalent = pinch). —
  `ViewSettingsPanel.cpp`. — **PREF-27**.
- **PR-8 multi-MediaDirs list** — Verify FolderConfigView supports a LIST of
  media directories (not just one); if not, add the multi-dir list editor. —
  `SequenceFileSettingsPanel.cpp`. — **PREF-28**.
- **Custom KeyBindings editor (CONFLICT)** — README marks it a hard miss; the raw
  dataset (`:79`) marks it Missing-but-feasible P3 (a remap UI over the shared
  KeyBindings core); `99` does NOT list it. **Decide:** either add a P3
  KeyBindings-editor item here, OR add it to 99 with the gesture rationale.
  Currently it falls through the cracks. — `KeyBindingEditDialog.cpp` +
  `KeyBindings.cpp` exist; iPad shortcuts hard-coded in `XLightsCommands.swift`.
  — **PREF-29 (or 99)**.
- **(DETAIL) PR-1 / PREF-15 — 6 backup options enumeration** — copy
  BackupOnSave/OnLaunch/Subfolders/PurgeInterval/UseShowFolder/BackupDirectory so
  the "folded into one feature" claim is auditable; note BackupSubfolders +
  PurgeInterval explicitly accounted-for (PREF-15 skips purge unless asked). —
  `BackupSettingsPanel.cpp`. — enrich **PREF-15**.
- **(DETAIL) PR-4/PR-5/PR-8/PR-10 — full option enumerations** — copy the 11
  EffectsGrid, 18 Other, 14 SequenceFile, 12 ViewSettings option lists into the
  PREF-16/18 batches so nothing is silently dropped; copy the FSEQ version option
  list ("V1, V2 ZSTD, V2 Uncompressed, V2 ZLIB, V2 ZSTD/sparse (Default)") into
  FSEQ-1. — `OtherSettingsPanel.cpp`, `SequenceFileSettingsPanel.cpp:107-112`. —
  enrich **PREF-16/18 / FSEQ-1**.
- **(DETAIL) L-4 backup purge — record deliberate drop** — PREF-15 deliberately
  skips the purge-interval machinery; record L-4 (P3/S) as an explicit deferred
  sub-item so the drop is conscious. — enrich **PREF-15**.
- **(DETAIL) Preferences total surface** — record "10 panels, 96 options" in
  11's intro so the ~30 in-scope vs ~40 out-of-scope (99 §7) accounting is
  explicit. — enrich **11 intro**.

### → `12-ai-and-automation.md`

- *(no new missing items)* — AI Image/Palette/Services, Lua engine (AUTO-7/8),
  REST→dispatcher (AUTO-1) all covered. **One CONFLICT to resolve:** the existing
  `future-help-diagnostics.md` and `README.md` classify Run Scripts / Lua as a
  hard-miss, but AUTO-7/AUTO-8 make interpreted Lua in-scope and **AUTO-11
  proposes GitHub download of community scripts** — which `99 §1` explicitly
  rules out. Reconcile AUTO-11 against the 99 ruling (downloading community
  scripts is the prohibited behavior). Update the stale README/help "no plans for
  Lua" framing (the JIT concern is moot — iPad Lua is interpreted). — enrich
  **AUTO-11** + README/help note.

### → `13-help-diagnostics-tools.md` (NEW DOC — create)

- **ShowFolderSearchDialog (T-8 / T-22 / H-12)** — Add P3: scan device for
  existing xLights show folders. In the iOS sandbox a free filesystem scan is
  largely meaningless — **re-conceive as a Files-app folder picker**. Absent from
  all new docs and the raw dataset. — `ShowFolderSearchDialog.cpp`;
  `xLightsMain.cpp:1158/9116`. — **TOOL-1**.
- **PathGenerationDialog / Generate 2D Path (T-13 / T-24 / U-7)** — Add P3:
  interactive draw-a-path canvas (mouse/key handlers + file picker + smoothing
  slider) emitting 2D node-path data for sketch/path-driven effects. On iPad
  maps to a touch-draw canvas (reuse the Sketch/Morph inline-custom-row idiom —
  cross-ref FX-9/FX-12). Effort M. — `PathGenerationDialog.cpp`;
  `xLightsMain.cpp:1142/8015`. — **TOOL-2**.
- **Prepare Audio (Reaper .rpp / .xaudio) (T-15)** — Add P3: applies a Reaper
  .rpp / .xaudio change-description to the show audio. Flag its dev-stub nature
  (live path reads a reaper file; leading body is a dead `#if 0` test stub) so
  the team decides port-vs-drop; cross-ref 02-timing-and-audio. May depend on
  FFmpeg/audio editing — verify viability. — `xLightsMain.cpp:1154/8021`. — **TOOL-3**.
- **Tip-of-the-Day FEATURE (H-5 / U-2)** — Add P2 (product call): the dialog +
  50+ HTML/PNG tips. Genuinely missing from all plans — the raw analysis marks
  only the two PREFERENCE toggles (min level / recycle) as out-of-scope.
  Recommendation: build an Apple-style "What's New" sheet against the iPad
  feature set rather than port the carousel. **Also add the FEATURE to 99 §7**
  with this rationale (only the prefs are listed there today). — `TipOfTheDayDialog.cpp`
  + `TipOfDay/` assets. — **TOOL-4** + 99.
- **CharMapDialog (U-14)** — Add P3 (likely platform-substituted by the iOS
  emoji/character keyboard): curated character-map picker for the Text effect. —
  `src-ui-wx/effects/CharMapDialog.cpp`. — **TOOL-5**.
- **ResizeImageDialog (U-6)** — Add P3 (or note iOS Photos editing substitutes):
  standalone crop/scale of an image asset on media import (AI-3/AI-7 crop/resize
  is AI-image-only). — `src-ui-wx/media/ResizeImageDialog.cpp`. — **TOOL-6**.
- **EmailDialog crash-report prompt (H-10)** — Add P3: the one-time/at-report
  email-collection modal (`CollectUserEmail` seeds `noone@nowhere.xlights.org`,
  keeps prior address on cancel). PREF-14 only adds a buried stored userEmail
  field — decide whether a prompt is wanted. — `EmailDialog.cpp`;
  `xLightsMain.cpp:8503`. — **TOOL-7**.
- **(DETAIL) Help-menu — add Donate + Zoom Room** — the iPad Help menu is
  populated (`XLightsCommands.swift:478-514`: Manual, Tutorials, Release Notes,
  Forum, Facebook, Issue Tracker, Website) but MISSING desktop's **Donate** and
  **Zoom Room** links. Add those two small items. — enrich Help section. — **TOOL-8**.
- **(DETAIL) H-7 Package Show for Support (vs Package Logs)** — confirm whether
  iPad's XLLogPackager bundles the entire show folder + autosaves, or only logs +
  diagnostics; if narrower, note the gap. — **TOOL-9**.
- **(record-as-done)** About (AboutSheet), Help-menu URLs, View Log
  (LogViewerSheet, search may be absent — minor), Package Logs (XLLogPackager),
  crash telemetry (XLDiagnosticUploader — does not yet collect email; PREF-14),
  in-app log viewer, SplashDialog (LaunchScreen). — see § STALE appendix.

### → `99-out-of-scope.md` — hard-miss items not yet present there

- **PixelTestDialog** — add the serial/DMX-test-blocked + network-test-deferred
  rationale (in-scope half is now CTL-20). — `PixelTestDialog.cpp`.
- **Generate Custom Model wizard (MA-2 / T-11)** — add: OpenCV-style
  recorded-video bulb-detection pipeline is wx/desktop-only; iPad ARKit
  Map-from-Lights is the partial substitute (deferred, not impossible). —
  `GenerateCustomModelDialog.cpp`.
- **Tip-of-the-Day FEATURE** — add a §7 row (today only the prefs are listed). —
  `TipOfTheDayDialog.cpp`.
- **Crash xLights** (intentional-crash telemetry tool) — add (dev diagnostic;
  iPad equivalent duplicates Package Logs). — `xLightsMain.cpp:1137`.
- **Log Render State** (render-state dump) — add (dev diagnostic; iPad has live
  View Log + Package Logs). — `xLightsMain.cpp:1139`.
- **Proprietary vendor FSEQ codecs (Falcon V4/V5 / Genius / PowerDMX)** — add by
  name (the open-firmware gate is documented, but the proprietary-codec exclusion
  isn't named). — `07-controllers-and-upload.md:25`.
- **Wiring-diagram Print (WV-6)** — add a brief "per-model wiring print → share
  PNG instead" note (printing is a weak iPad need). — `WiringDialog.cpp:751-763`.
- **Controller-layout Print (O-15)** — add alongside the layout-print
  out-of-scope rationale (and note iPad can route capture → UIActivityViewController
  → AirPrint). — `LayoutPanel.cpp:6324/6395`.
- **UpdaterDialog / Check-for-Updates** — add a one-line platform-handled note
  (App Store handles updates).
- **PR #6311 controller-source-tree** — resolve what this opaque desktop-PR
  feature is and classify properly, or drop the unexplained out-of-scope entry.
- **(DETAIL) Custom KeyBindings editor** — add here (with the gesture rationale)
  **only if** the PREF-29 decision is "out of scope" (see 11 CONFLICT above);
  currently 99 omits it and it disagrees with the raw dataset's P3-Missing.
- **(DETAIL) FFmpeg whole-library rationale** — enrich §3: add the quantified
  reasoning (no bundling: ~50MB binary bloat + LICENSING + App-Store-REVIEW risk;
  AVFoundation/AVAssetWriter would cover MP4/MOV only, no GIF; decode via
  VideoToolbox works). §3 today only says "no FFmpeg encode in the iPad lib". —
  enrich **99 §3**.
- *(already present, confirm)* VAMP host, Python scripting, SpaceMouse/3Dconnexion,
  AUI perspectives, xSchedule/xFade, HinksPix family (Export + SD-card),
  Effect Assist Window mode, Perspectives — already in 99, no action.

---

## 4. STALE / ALREADY-DONE appendix

These were listed as pending in one or more prior plans but are **actually
shipped/covered on iPad** (verified). No action — recorded so the team doesn't
re-plan them.

**Controllers / upload (07):**
- Controller drag-reorder (`LayoutEditorView.swift:1143`), right-click
  Activate / Activate-xLights-Only / Inactivate / Unlink-from-Base
  (`:1803-1854`), controller delete-confirm with assigned-model count
  (`modelNames(forController:)`), Model/ModelManager::DeleteController cascade
  cleanup, the iOS-26 swipe-delete crash fix, J-31/J-32 controller
  CRUD/property-grid/discovery/Visualize. FPP Connect Slices A/B/F + Slice-C
  auth (parallel transcode, concurrent curl, per-FPP gauges, full restart +
  channel-range + playlist-finalize, 401/Keychain auth). Drag-drop port
  assignment + per-model connection editing in `ControllerVisualizeView`.

**Layout / models / view-objects (05/06):**
- 3D canvas/gizmo (J-23/LA-5), right-click menus (LA-6), align/distribute/
  match-size (LA-8 — `XLMetalBridge.h:508/519/528`), bulk-edit propagation
  (LA-12 base), Mesh/Terrain/Ruler/Image/Gridlines view-objects (VO-1..5,
  verified 2026-05-17), Aliases (MA-12/BE-7), submodel aliases (MA-6),
  strand/node text naming (MA-13 text half), Model Chain (DM-15), Start Channel
  editor (DM-16), Skull config + Skulltronix preset (DM-12/DM-8), per-type
  property setters (LA-3/B-5), dimming-curve editor, custom-model visual grid +
  sub-buffer editor (J-22/J-23), Hide Unused Submodels, Per-Model Default group
  render style (#4125), Promote-Node-Effects, model-pane count badges (#6202 via
  J-18), vendor model download (VendorModelDialog/XLVendorCatalog), Base Show
  Folder linking + auto-update + EnableBaseShowSettings (L-10 — more fully
  realized than desktop), xmodel start-channel/relative-position import fixes
  (#6447/#6438), media-relink basename split, the MetalShaderEffect interop
  mutex crash fix. Promote-Node-Effects (the non-data half of B56).

**Sequencer / effects / color (01/03/04):**
- Four inspector tabs (Effect/Colors/Blending/Buffer — AP-9/AP-13 "verify gap"
  resolved), full Value Curve editor all 23 types (AP-12/U-22), ColorCurve +
  Viewpoint editors (U-21/U-23), Buffer panel (AP-9), Blending panel (AP-13),
  Duplicate "Include Submodels" (#6419), grid long-press menus (shipped
  2026-04-20).

**Timing / audio (02):**
- Audio Onsets/Tempo/Chords generation (A-1/A-2), onset/pitch/spectrogram
  overlays (A-3/A-9/A-10/A-11), advanced waveform/stem filters incl. HTDemucs
  (A-4), alternate audio-track CRUD (A-6), Metronome with Tags (IO-21),
  Papagayo .pgo import bridge (IO-12), lyric breakdown pipeline (IO-23), Music
  Generator (A-12, self-flagged probably-nonexistent on desktop).

**Import/export & files (08/09):**
- SuperStar .sup import (IO-7), Package Sequence .xsqz (T-19), Recent Show
  Folders (L-1), SaveChanges/Close-dirty prompt (U-5), Check Sequence (T-1 —
  only per-check toggles remain), xLights→xLights effect import + channel-map +
  Auto Map (IO-11/I-1..I-4). BatchRenderDialog Freeze/Thaw (wx-only, no SwiftUI
  analog).

**AI / automation (12):**
- AI Color Palette (T-6/AI-1), AI Image embed (T-5/AI-2), Apple Intelligence +
  ChatGPT providers (AI-3/AI-4), ServicesPanel + Keychain + Test (AI-5/PR-9).

**Help / diagnostics (→ 13):**
- About (AboutSheet — H-1/U-3), Help-menu URLs (H-2), Log export / Package Logs
  (H-3/T-2/T-20 — the P0-for-TestFlight blocker is resolved), crash telemetry
  (H-4 — XLDiagnosticUploader; does not yet collect email), View Log /
  in-app log viewer (H-6/T-2 — search may be absent), SplashDialog (H-9/U-4 —
  LaunchScreen, shipped Phase H-1).

**Misc / build:**
- ISPC effects link clean for iOS ARM64 (R-3 — the app ships all effects),
  ShaderEffect iPad perf baseline (R-4 — runs via ANGLE/GLES single-threaded by
  design), on-device verification matrix (R-5 — QA, not a plan item), portable
  FreeType+HarfBuzz TextDrawingContext + main-thread render-queue removal (from
  the Android study — that infrastructure has ALREADY been removed from the
  tree; `CanRenderOnBackgroundThread`, `RenderMainThreadEffects`,
  `_mainThreadRenderEvents` all gone), iPad font picker wx-font-desc round-trip,
  Shape emoji/skin-tone rendering, animated WebP/GIF loader registration.

**Superseded by a newer decision (not a literal port):**
- Disk-persistent presets — *promoted* from "in-session is enough" deferral to a
  Phase-1 keystone (PRE-1..4). Lua scripting — *upgraded* from README hard-miss to
  in-scope interpreted Lua (AUTO-7/8). EffectAssist Window-mode toggle (EA-4) —
  out-of-scope (inline inspector rows). B-1 Undo bridge — kept Foundation
  UndoManager, labels-only (SEQ-1). AP-3 SelectPanel sort-by-time (#6389) — folds
  into the new SelectPanel item (VIEW-7).
