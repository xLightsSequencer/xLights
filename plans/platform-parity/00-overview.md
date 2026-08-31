# 00. Overview — xLights Platform Feature Parity

_Full-code audit generated 2026-07-31, adversarially cross-checked against the prior plan set
2026-08-01 (see [Cross-check](#cross-check-vs-prior-plans-2026-08-01) below). Every claim
carries `file:line` evidence on both platforms. Statuses: ✅ parity | 🟡 partial | ❌ missing
on iPad | 🚫 infeasible or platform-restricted on iPad | 🔵 iPad-only | ➖ not applicable._

The audit covers two axes:

1. **iPad ↔ Desktop** (themes 01–14) — the primary axis. The iPad links the same `src-core/`
   as desktop; parity gaps are almost always UI/bridge surface, not engine capability.
2. **macOS ↔ Windows ↔ Linux desktop builds** (theme 15 plus per-theme platform sections).

## Headline numbers

Row tallies per theme after the cross-check pass (a row is one feature, deliberately
fine-grained — a menu entry, a dialog field, a gesture):

| Theme | ✅ | 🟡 | ❌ | 🚫 | 🔵 | Biggest gap in one line |
|---|---|---|---|---|---|---|
| 01 File & show lifecycle | 79 | 47 | 28 | 7 | 18 | Show-folder backup/restore landed 2026-08-10 in desktop's interchange format; biggest remaining: Data Layers and the legacy converter bridge |
| 02 Sequencer grid & editing | 122 | 35 | 27 | 2 | 12 | No cell-range selection (blocks paste-to-region, random effects, half the context menu); AC mode absent (formally deferred) |
| 03 Timing, lyrics & audio | 118 | 19 | 17 | 6 | 6 | No keyboard timing-mark entry during playback; dictionary editor saves unvalidated phonemes |
| 04 Effects catalog & panels | 84 | 10 | 2 | 1 | 3 | 49/56 effects fully ✅ (52/56 render, 50/56 settings UI); gaps are assist surfaces + Moving Head preset/authoring depth |
| 05 Color, palettes & curves | 72 | 16 | 27 | 3 | 9 | Curve editors have no session-scoped Cancel/revert; no drag-and-drop for colors/curves |
| 06 Layout, models, 3D | 167 | 50 | 63 | 5 | 8 | Deep grid dialogs (custom-model transforms, Faces/States forms), CAD/print export, cross-show import |
| 07 Controllers, outputs, upload | 70 | 30 | 28 | 12 | 0 | Closed-firmware uploads deliberately out of scope (policy); real bugs: Visualize wrongly policy-gated, ESPixelStick missing its open-firmware caps node (known, deferred) |
| 08 Import & export | 50 | 11 | 44 | 1 | 3 | 11/13 effect-import formats work; exporters (.lcb/.vir/LSP/HLS) still trapped in desktop `TabConvert.cpp` |
| 09 Render & playback | 63 | 11 | 18 | 9 | 14 | No render dependency tracking (stale effects); no per-model render progress; no FSEQ version selector |
| 10 Presets, views, jukebox | 57 | 13 | 14 | 5 | 3 | Preset formats don't interchange; jukebox ported 2026-08-10; no workspace layouts |
| 11 Preferences & shortcuts | 33 | 19 | 51 | 0 | 2 | No unified settings surface — 33 parity settings scattered across six unrelated places (redo approved 2026-08-01; see Decisions) |
| 12 AI, automation, scripting | 41 | 6 | 3 | 103 | 4 | AI at near-parity; automation/scripting at zero (no HTTP listener, no interpreter on iOS — App Intents is the sanctioned path) |
| 13 Tools, diagnostics, help | 59 | 12 | 11 | 4 | 6 | Light test & Check Sequence share core engines; gaps are targeting trees, report export, crash-time capture |
| **Total (01–13)** | **1015** | **279** | **333** | **158** | **88** | |

Theme 11 additionally has 8 ➖ rows. Theme 14 (reverse parity) now has **48** 🔵 rows with a
14-rank desktop-adoption shortlist. Theme 15 has 143 desktop cross-OS rows with no iPad status.

**Parity index:** of the 1,627 rows where an iPad status is meaningful (✅+🟡+❌), **62%** are
at full parity and **80%** at full-or-partial. Counting partials at half weight the iPad sits
at **≈71% of desktop**, with the shortfall concentrated in Layout depth (06), Import/Export
writers (08), Preferences (11), and a long tail of small grid/file affordances. The 🚫 bucket (159) is dominated by one block: 103 automation verbs/endpoints iOS cannot host (theme 12).

**The structural headline:** the iPad is not a viewer. It creates 25/28 model types, runs all
six discovery scanners, renders 52/56 effects with shared-core fidelity, imports 11 of 13
effect formats, and has genuine controller CRUD + upload plumbing. Most remaining gaps are one
of: (a) a missing bridge setter over core code that already works, (b) a missing SwiftUI
affordance over a bridge method that already exists, or (c) a deep desktop dialog (3–5 kloc)
that was never rebuilt. Category (c) is the only genuinely expensive work.

---

## Recorded product decisions

Scope decisions that pure code-reading cannot see, preserved here so the roadmap doesn't
silently re-litigate them (originally recorded 2026-06-11; items 1–3 reviewed and updated
2026-08-01):

1. **Closed-firmware vendor upload is out of scope on iPad (policy).** Controller
   config/upload is in scope for open-source-firmware controllers only (FPP, WLED,
   ESPixelStick, Kulp/K-boards, DDP/generic). The ten closed-firmware vendor rows in theme 07
   are therefore **policy-restricted, not parity bugs**. Status of the in-scope set:
   Kulp needs nothing — its variants inherit `<OpenSourceFirmware/>` through their
   `Base="FPP:…"` chain at caps-load time (`ControllerCaps.cpp:46-59`); ESPixelStick is a
   real gap (`espixelstick.xcontroller` lacks the node, no FPP base) — **known, deliberately
   deferred**. The remaining defect worth fixing now: **Visualize** is gated on firmware
   policy though it writes nothing (desktop gates it on `CanVisualise()`).
2. **Jukebox on iPad: approved (2026-08-01, reversing the 2026-06-11 decline).** Low usage,
   but the audit showed the port is cheap — data model core-shared and round-tripping, UI +
   thin bridge only. Low priority; slot opportunistically.
3. **Settings: redo approved (2026-08-01, superseding the 2026-06-11 decline of a
   centralized dialog).** Some settings genuinely belong in a global settings surface; the
   current placement (33 at-parity settings scattered across six unrelated surfaces) needs
   cleanup, along with the drifted key naming.
4. **AC toolbar cluster: DEFERRED to P3** — if demand surfaces, revisit as a *touch-first*
   design applying core AC render ops to the iPad's selection idioms, not a port of the
   desktop cell-cursor.

---

## iPad gap inventory, by severity

### S1 — Data loss and silent correctness (fix before any feature work)

These damage user data or produce wrong output with no error. Each cites its theme doc + row.
All survived adversarial re-verification.

| # | Gap | Where | Why it's S1 |
|---|---|---|---|
| 1 | ~~View create/rename/clone/delete/membership edits **evaporate on relaunch**~~ | 10 r71 | **FIXED 2026-08-06** — `iPadRenderContext::SaveViews()` writes the `<views>` subtree from the shared `SequenceViewManager::Save`; every view mutation writes through, and model rename now renames through the views too |
| 2 | ~~No show-folder backup or restore; snapshot ring covers the current `.xsq` only~~ | 01 r108–110 | **FIXED 2026-08-10** — `ShowFolderBackup.swift` writes desktop's exact `Backup/<date>-<time>/` run format (seven globs, 30 MB gate, `_OnStart` runs), so backups interchange with desktop both ways; manual command + opt-in backup-on-open + forced pre-recovery/pre-restore backups; the Restore sheet restores the four config files and multi-selected sequences, then reloads the show folder |
| 3 | ~~Autosave never covers `xlights_rgbeffects.xml` / `xlights_effectpresets.json`~~ | 01 r95–96 | **FIXED 2026-08-06** — layout edits autosave to `.xbkp` with a Use/Discard prompt at load. The presets half of this claim was **wrong**: every preset mutation already saves immediately (with a `.jbkp`), so there was no unsaved window to protect |
| 4 | ~~Frame-interval change rewrites timing without desktop's save/close/reopen snap cycle~~ | 01 r146 | **FIXED 2026-08-06** — the cycle is ported, with desktop's two confirmations; the reopen is what snaps effects to the new grid |
| 5 | Base-show-folder merge re-runs unconditionally on every show open (desktop skips when unchanged) — **controllers half FIXED 2026-08-06**; models/objects still re-merge | 01 r179 | The controller pass now gates on the core `NeedsBaseControllersUpdate()`. The models/objects pass **cannot** be gated until its merge is persisted: it only mutates the in-memory ModelManager, and the unconditional re-merge is what makes it reappear each open. Persist first, then gate — that half is where the mesh-access cost is |
| 6 | ~~No render dependency tracking — effects depending on a timing track or another model go stale until Render All~~ | 09 r7 | **FIXED 2026-08-06** — `RenderDependentModels()` drains the set from the 0.5 s dirty poll (the iPad has no output timer, which is where desktop drains it) |
| 7 | ~~Stop doesn't blank outputs — lights hold the last frame~~ | 09 r88 | **FIXED 2026-08-06** — `blankOutputs` calls `AllOff()` from Stop and both natural end-of-playback paths; `stopOutput` blanks before closing |
| 8 | FPP Connect FSEQ type — **the three-mis-served-families claim was wrong**: iPad discovery admits only FPP and ESPixelStick, so Falcon V4/V5 and Genius/PowerDMX never receive an upload at all. The one real case, a master-mode FPP getting sparse, is **FIXED 2026-08-06** | 07 r69 | Downgraded from S1 to a missing picker (07 r69, still partial). A reminder that a ❌ needs the reachability check, not just the code read |
| 9 | ~~`DidConvert` never consulted; no `NetworkChangesAllowed()` guard~~ | 07 r81 / r26 | **BOTH FIXED 2026-08-06** — the load path now flags a converted legacy networks file dirty so the migration persists, and controller edits are blocked while outputting |
| 10 | 57 | 13 | 14 |
| 11 | 33 | 19 | 51 |
| 12 | 41 | 6 | 3 |
| 13 | 59 | 12 | 11 |
| 14 | ~~Different default palette colors *and* default-enabled slots~~ | 05 r5–6 | **FIXED 2026-08-06** — the new-effect seed now carries desktop's eight colours in desktop's slot order plus `C_CHECKBOX_Palette1/2=1`; without the checkboxes `ParseColorMap` gave the effect an empty colour list |
| 15 | ~~Missing `SetDefaultParameters` seeding on effect drop~~ | 04 | **FIXED 2026-08-06** — the bridge supplies the list-derived defaults at creation (State's first state, Faces' phoneme source) |
| 16 | ~~AI-generated images land as loose files, never embedded~~ | 12 r34 | **FIXED 2026-08-06** — embedded under the same `AIImages/…` key desktop uses, so the image travels inside the `.xsq`; loose file remains the fallback |
| 17 | ~~Alt-timing-track playback routes different audio than desktop~~ | 03 r125 | **FIXED 2026-08-06** — playback follows the selected track; switching mid-playback parks the transport and re-seeks |
| 18 | ~~iPad-only VC Min/Max + irreversible "Real Values" toggle~~ | 05 r111–112 | **FIXED 2026-08-06** — both iPad-only affordances removed; Min/Max are read-only and Done sets the real-value flag, as desktop does |
| 19 | Tip-of-Day ignores its own level filter (**won't fix** — feature declined on iPad, 2026-08-06). Release-notes URL 404s on untagged patch releases: **FIXED 2026-08-06** | 13 r57/66 | Small bug-fix pair; only the release-notes half was in scope |
| 20 | ~~Two undo stacks — future bridge-level ops will undo out of order~~ | 02 r60 | **FIXED 2026-08-06** — worse than recorded: nothing ever called `UndoLastStep`, so eight bulk bridge ops were not undoable at all. Foundation is now the single owner; those ops register a Foundation step that unwinds the core one |
| 21 | Desktop allows multiple active timing tracks; the iPad bridge forces radio behaviour on the same data | 03 r49 | Divergent semantics both docs had missed |

### S2 — Missing feature blocks (workflow blockers)

Highest-leverage first within rough effort bands.

**Nearly free (data fix / wrong gate / one call):**
- **Visualize re-gate** (07): gate on a bridged `canVisualise` instead of the firmware policy —
  restores a read-only view the policy never meant to remove. (ESPixelStick's missing caps
  node is known and deferred by decision; Kulp already inherits open-firmware caps via its
  FPP base chain.)
- ~~**Render progress sink** (09 r14–15)~~ — **DONE 2026-08-06**: per-model progress + status
  text ship as a long-press sheet over the toolbar render button. No sink was needed; the same
  job list desktop's poll loop reads is available directly off `RenderProgressInfo`.
- ~~**Keyboard timing entry** (03 r58–61)~~ — **DONE 2026-08-07**: "t" / "s" act on the active
  timing track at the play marker, as menu commands with bare-key shortcuts.
- ~~**Viewpoint menu in Layout Editor** (06 r232–234)~~ — **DONE 2026-08-06**: the Layout
  Editor overlay now posts the pane-scoped command `PreviewPaneView` already answered. Only
  "set as default viewpoint" is still missing (no bridge method).
- **`FixRgbEffects` on load; Master-View reorder; preset interchange** (10): each small,
  plumbing exists.
- ~~**AI image sheet renders only `.choice` properties** (12 r27)~~ — **DONE 2026-08-06**: all
  property kinds render; the session gained the bool/int setters to match.

**Medium (bridge surface + a sheet):**
- ~~**Cell-range selection** (02)~~ — **DONE 2026-08-07**: `CellRange` (rows × timing-mark
  columns) established from the marquee, with paste-into-range, Create Random Effects and the
  empty-cell menu on top of it. Keyboard column nav is still not wired to it.
- **Grid drop target** (02 r35–36) — **external media DONE 2026-08-07** via a
  `UIDropInteraction` on the canvas; palette drag-onto-grid is still arm-then-tap, which is the
  touch idiom and works, so r35 stays partial rather than blocking.
- ~~**Per-protocol output properties** (07)~~ — **DONE 2026-08-07** for every Ethernet
  protocol: DDP, ZCPP, KiNET (incl. Version + port labelling), OPC, Twinkly, xxx Ethernet,
  plus Player Only in the caps-less fallback. **LOR Optimised's per-device tree is still
  missing** (07 r17) — it is serial-only, and serial output is impossible on iPadOS, so
  configuring it there is low value.
- **Settings redo** (11) — approved 2026-08-01: promote the genuinely global settings into a
  global settings surface, keep per-context sheets where they fit, clean up placement and
  reconcile the drifted key naming (`renderOnSave`, `pasteByCell`). 33 already-working
  settings just need re-hosting; the ❌ backlog becomes visible in the process.
- ~~**Show-folder backup + config-file restore** (01, pairs with S1 #2/#3)~~ — **DONE 2026-08-10** in desktop's run format, so backups interchange across platforms.
- ~~**Light-test Outputs/Groups trees + search** (13 r8–9/11); **Check Sequence export**
  (13 r21)~~ — **DONE 2026-08-07**: Outputs and Groups tabs plus one filter field over the
  visible tab; Check Sequence exports HTML to the share sheet.
- **Import mapping depth** (08): time-offset, auto-map wiring (bridge op exists, no Swift
  caller), CCR/strand rows, import-media toggle, "Used"-source marking (r89),
  `.xmap`/`.xjmap` (needs a small wx-free `MappingIO`).
- ~~**Selection-scoped lyric breakdown** (03 r98–101)~~ — **DONE 2026-08-07**: per-word
  breakdown plus selection-scoped phrase and word variants.
- **ColorCurve blend-mode accessor** (05 r77); **drag-and-drop for colors/curves + dropper
  panes** (05 r35–37/121).
- **Layout-group lifecycle** (06) — **delete/rename + multi-model .xmodel export DONE
  2026-08-07** (r83/84, r211). Still open in this cluster: the persisted 2D bounding-box and
  grid-spacing settings (r77/78 — the live view toggles work, the stored show setting is
  read-only) and the persistent 3D-mode setter.
- **Moving Head authoring depth** (04 r29/40): multi-handle wheel/picker, user `.xmh` preset
  scan, panel reset.
- **Preview transport overlay + auto-show** (09 r95–98); **Force High Definition in batch
  render** (09 r47); **FSEQ version selector** (09 r30).
- ~~**Discovery seeding + FPP auth on the discovery path** (07 r51–52); **visualizer bulk port
  ops** (07 r34–36, r39); **smart-remote TYPE block propagation** (07 r89)~~ — **DONE
  2026-08-07**. Still open in that cluster: **bulk smart-remote assignment per SR group**
  (07 r38) — the ops added are port-scoped, where r38 groups by remote instead.
- ~~**Cleanup File Locations: rgbeffects half** (13 r28)~~ — **DONE 2026-08-06**: the sweep now
  covers the preview background, models and view objects via the shared-core
  `CleanupFileLocations(RenderContext*)`.

**Large (rebuilt surfaces):**
- **AC mode** (02 #181) — deferred by decision; revisit touch-first if demand surfaces.
- **Data Layers** (01/09) — `DataLayerSet` is wx-free and half-wired; missing converter
  plumbing + bridge + UI.
- ~~**Jukebox** (10 r32–40)~~ — **DONE 2026-08-10** (approved 2026-08-01, reversing the earlier
  decline): `JukeboxSheet` + link editor over a thin bridge editing the core map in place.
  Only the shortcuts (r39) and platform-restricted automation verbs (r40) remain open.
- **Custom-model grid transform/import/export suite** (06 r156–162) and **Generate Custom
  Model wizard** (06 r164, the largest single item).
- **Structured Faces/States editors** (06 r188–197).
- **CAD / print / layout-image export cluster** (06): `src-core/cad/` is wx-free and entirely
  unreferenced from iPad; coherent porting unit.
- **Cross-show import** (06 r218–219) — **r218 DONE 2026-08-07** (models / groups / viewpoints
  from another show's rgbeffects, with desktop's two-pass merge). r219, the LOR S5 layout
  importer, is untouched: `LORPreview.cpp` is wx-side with no core lift, and S5 *sequence*
  import already works, which is the commoner need.
- **`.msq` (LSP) and `.vsa` import** (08) + **legacy exporter lift** (`.lcb`, `.vir`, LSP,
  HLS, Minleon out of `TabConvert.cpp` into `src-core/` — both platforms then share one
  implementation).
- **ColorManager → `src-core/`** (11 r56–64): unblocks six rows, deletes three hand-copied
  default tables.
- **Editable key bindings** (11); first step: generate `KeyBindingsSheet` +
  `CommandPaletteRegistry` from one command table (kills two live drift hazards).
- **Pictures assist pixel editor** (04); shader download browser; Shape character-map grid.
- **Workspace layouts** (10 r74–79): the perspectives *idea* is portable even though wxAUI
  geometry is not — needs a product decision.

### S3 — Polish and discoverability

- **Menu placement drift** (02): several operations live on different surfaces than desktop;
  users switching clients hunt.
- **Hardware-gated ops need touch fallbacks** (02): multi-select requires Magic Keyboard,
  shared-edge resize requires Pencil Pro squeeze.
- **Empty-cell context menu, double-click effect picker, scroll-selection-into-view** (02).
- Settings-key naming drift (`renderOnSave`, `pasteByCell`) blocks future shared settings (11).
- Palette discovery misses show-folder root + `.scss`; Save-As silently overwrites (05).
- Log-package completeness (13); palette/keybindings registries generated, not hand-written
  (13 r84 / 11 r91); stale in-code comments (07: "iPad doesn't yet have a controller editor"
  — it does; 03/06 others noted in theme docs).

### Platform-restricted (🚫) — accepted, with the honest reason

- **VAMP plugins** (03): third-party `dlopen` — iOS platform rule. Built-in detectors cover
  beats/tempo/chords/pitch; the genuine loss is Polyphonic Transcription. Zipped `.mxl`
  MusicXML also desktop-only (03 r166).
- **Serial/DMX live output** (07/09): `SerialPort` stubbed for `TARGET_OS_IPHONE`.
- **CLI modes** (09/12) and **REST listener + Lua/Python scripting** (12): 102 rows; App Store
  2.5.2. The sanctioned replacement is **App Intents / Shortcuts** (12 r150) — the largest
  unclaimed opportunity in that theme.
- **Window-manager items** (09/10): keep-on-top, dock suppression, wxAUI perspectives.

---

## Prioritized iPad → Desktop roadmap

**P1 — Correctness first (small, high-trust):** the S1 table, roughly in order. Most of it is
now done (2026-08-06; show-folder backup/restore followed 2026-08-10); what remains is the
base-show-folder models/objects re-merge (S1 #5, persist-then-gate) and the timing-track
multi-active semantics (03 r49, still an open product question). The Visualize re-gate (07) is
held pending desktop/iPad work already in flight.
Everything in P1 is bridge-level or view-level work with no new architecture.

**P2 — Close the daily-driver workflow:** cell-range selection + grid drop target (02);
the settings redo (11 — approved: global settings surface + placement/naming cleanup);
per-protocol output
properties, DDP first (07); keyboard timing entry + dictionary validation (03); import mapping
depth (08); light-test trees + Check Sequence export (13); layout-group lifecycle + viewpoint
menu + background setters (06); preview transport + batch-render HD (09); Moving Head
authoring depth (04). Done, this makes the iPad a self-sufficient primary editor for the
common sequencing loop.

**P3 — The big builds, in value order:** structured Faces/States forms → custom-model
transform suite → cross-show import → CAD/print/export cluster → `.msq`/`.vsa` + legacy
exporter lift to core → ColorManager to core + editor → editable keybindings → Pictures pixel
editor → App Intents story → Data Layers → Generate Custom Model wizard → ~~Jukebox~~ (done
2026-08-10) → [AC mode, touch-first, if demand surfaces].

Cross-cutting engineering moves that pay for multiple rows: (a) single command table
generating menus, palette, and keybindings sheet; (b) move duplicated lists (layer methods,
transitions — three hand-copies verified identical today) into core-served metadata; (c) one
undo-stack owner; (d) bridge setters batch for the read-only-with-comment layout properties.

---

## Desktop cross-OS summary (macOS vs Windows vs Linux)

Full detail in [15-desktop-platform-matrix.md](15-desktop-platform-matrix.md) (143 rows).
The ones that matter most:

1. **Render byte-parity across OSes does not exist** — fast-math on mac/Linux but not
   Windows, `-O3`+ThinLTO vs `/O2` vs `-O2`, ISPC `--math-lib=fast` on Linux only, runtime
   GPU-vs-CPU selection. FSEQ regression diffs must be same-OS.
2. **macOS has no in-app update check — mostly moot**: ~98% of macOS installs come from the
   App Store, which updates automatically (Windows likewise skips its check for
   Store-packaged builds). Only the ~2% direct-download tail gets no new-version signal.
3. **Linux is the weak operational leg:** logs in `/tmp`, no mDNS/WLED discovery (silent
   empty function), software-only video encode, no stem separation, fixed 16-name serial
   list, no fonts in packaged sequences, no render-cache memory back-off. Several are
   single-function fixes.
4. **GPU effect compute is near-parity by design** — `effects/vulkan/` mirrors
   `effects/metal/` effect-for-effect; divergence is *when acceleration is declined* — except
   **Windows ARM**, an unflagged third target with no Vulkan, no shaders, no SIMD.
5. **Hidden-but-live preferences**: five controls hidden per-OS while the setting is read
   everywhere. Support advice of the form "uncheck X" is wrong on two OSes out of three.
6. Per-subsystem splits — text rendering (CoreText / DirectWrite / FreeType), audio analysis
   rate (48k mac vs 44.1k elsewhere → timing marks differ per OS), audio speed-change
   (instant on mac, stalls elsewhere), controller ping (real ICMP only on Windows), video
   decode HW-default (mac only), crash-bundle content, ML features (CoreML/ONNX/none),
   bundled-scripts location (12).
7. The two divergence *mechanisms* to watch in review: `ExternalHooks.h` silent no-op stubs
   and `#ifdef` panel constructors hiding live settings.

## Reverse parity — iPad features desktop should adopt

From [14-reverse-parity-ipad-only.md](14-reverse-parity-ipad-only.md), now 48 verified
iPad-only behaviors with a 14-rank adoption shortlist. The top of the list:

1. **Remove Unused Media** — core API exists with no desktop caller; menu item + confirm.
2. **Merge timing mark with next** — the missing inverse of Split.
3. **Value-curve Copy/Paste** — clipboard beats drag-between-panels.
4. **Select All in Column** — row/model scopes exist; column is the natural third.
5. **Media-folder targets in the file-picker copy prompt** — fixes desktop's own internal
   inconsistency.
6. **Go to Model on Canvas** from the visualizer.
7. **`.xsqz` in-place round-trip** (`SequencePackage::Repack` has zero desktop callers —
   desktop opens packages read-only).
8. **Actionable Check Sequence results** (tap-to-navigate vs desktop's static HTML).

…plus case-sensitive find, wiring-JSON export, persistent loop region, layout snap-to-grid,
palette clipboard, draggable XY pad (ranks 9–14). The most reliable marker found for these:
**core APIs with zero `src-ui-wx` callers** (`RemoveUnusedMedia`, `Repack`, `ExportAsJSON`,
`UndoManager::SetMaxSteps`). Also worth pulling over from other themes: async AI service
test / off-thread palette generation (12 — desktop's is a synchronous UI freeze), pre-restore
safety snapshot (01), the independent fseq compression picker (09), and memory-pressure
batch-render retry (09).

## Desktop-side defects found while auditing

Not parity items — genuine desktop bugs surfaced by the evidence standard:

- "Link controller upload" preference discards user changes (`SetLinkedControllerUpload`
  declared, never defined, never called) — 11.
- "Zoom To Cursor" checkbox never restored — reopening Preferences + OK silently re-enables — 11.
- Three dead `SeqExportDialog` entries (LOR/Vixen/fseq added then always deleted) — 08.
- Sequence Settings → Timings → New is a stale copy of the add-track flow (omits
  Onsets/Tempo/Chords/AI, always analyses the main track) — 03.
- `ID_MNU_MAKESCVALID` menu item runs the wrong handler — 06.
- `ViewpointDialog` is fully dead code (four empty handlers) — 06.
- `AIColorPaletteDialog` generates synchronously on the UI thread — 12.
- Wx-free `MusicCatalog` in core has no desktop caller; desktop runs an older duplicate — 14.
- Layer-method + transition lists hand-copied in three places (identical today; nothing
  enforces it) — 04.
- Desktop reuses an up-to-date fseq on open but never checks its mtime against the sequence —
  a freshness-guard nit found in the 14 pass, belongs to 09.
- Effect resize-drag never arms the edge auto-scroll timer that move-drag uses
  (`EffectsGrid.cpp:6810` vs `:8536-8545`) — 02/14.

## Cross-check vs prior plans (2026-08-01)

After the fresh audit, all 14 themes were adversarially diffed against the prior
`plans/ipad-parity/` set, with every disagreement settled by reading current code. Outcome:

- **~27 wrong statuses/evidence blocks in the new docs found and fixed**, and **~75 rows
  added** (30 of them behavior-level iPad-only items in theme 14). The dominant new-doc
  failure mode: **negative claims proven by greps that could never have gone red** — searching
  desktop symbol spellings inside `src-iPad/` (or vice versa), or the zsh `grep -E "a\|b"`
  literal-pipe trap. Every such claim in future updates should name a search that was
  demonstrated to match the counterpart platform's real identifiers.
- **The old plans were substantially stale in both directions**: ~10 theme-06 rows marked
  "✅ Done" for iPad features that were never written; theme-05/14 features credited to a
  `desktop-pullbacks` branch that does not exist; five "desktop-missing" claims for features
  desktop has since shipped; dead line references throughout; and coarse feature-name-level
  "parity" rows concealing sub-gaps (worst case: two different ColorCurve pickers collapsed
  into one ✅ row, hiding a whole missing feature).
- **The new docs won the large majority of status conflicts** (e.g. 11-of-16 in theme 01;
  17-of-17 in theme 13), and **every load-bearing S1 finding survived** — most importantly the
  theme-10 view-persistence data-loss bug, re-traced definitively.
- The old plans' unique value was **recorded product decisions** (the Decisions section
  above) and pointers to under-enumerated deep-dialog clusters, all of which are now folded
  into the theme docs.

## Document index

| Doc | Theme |
|---|---|
| [01-file-lifecycle.md](01-file-lifecycle.md) | File & show lifecycle, backups, packaging, media manager |
| [02-sequencer-grid.md](02-sequencer-grid.md) | Grid editing, selection, undo, row headers, timeline |
| [03-timing-audio.md](03-timing-audio.md) | Timing tracks, lyrics, dictionaries, audio & analysis |
| [04-effects-catalog.md](04-effects-catalog.md) | Per-effect matrix (56), settings engine, blending/transitions |
| [05-color-value-curves.md](05-color-value-curves.md) | Color panel, palettes, color & value curves |
| [06-layout-models.md](06-layout-models.md) | Model types (28), layout editing, 3D, submodels, DMX |
| [07-controllers-setup-upload.md](07-controllers-setup-upload.md) | Outputs (21), discovery, vendor uploads (14), FPP Connect |
| [08-import-export.md](08-import-export.md) | Import formats (13), mapping, exporters, Convert tab |
| [09-render-playback.md](09-render-playback.md) | Render engine control, fseq, transport, previews, live output |
| [10-presets-views-perspectives.md](10-presets-views-perspectives.md) | Presets, jukebox, display elements/views, perspectives |
| [11-preferences-settings.md](11-preferences-settings.md) | Every preference, Color Manager, key bindings |
| [12-ai-automation-scripting.md](12-ai-automation-scripting.md) | AI services, REST automation, Lua/Python, CLI |
| [13-tools-diagnostics-help.md](13-tools-diagnostics-help.md) | Light test, Check Sequence, logs, crash, help |
| [14-reverse-parity-ipad-only.md](14-reverse-parity-ipad-only.md) | iPad-only features; desktop-adoption shortlist |
| [15-desktop-platform-matrix.md](15-desktop-platform-matrix.md) | macOS vs Windows vs Linux desktop differences |
