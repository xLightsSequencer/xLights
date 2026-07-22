# iPad ↔ Desktop Feature-Parity Plan

> **A code-level audit of both apps.** Every status here is grounded in the
> actual source of the desktop (`src-ui-wx/`), the iPad app (`src-iPad/`), and
> the shared wx-free `src-core/`. The 14 theme docs carry the per-feature
> scorecards with `file:line` evidence; this overview is the cross-theme map.

## How to read this

- **1,200 features** were enumerated across 14 themes (menu items, context-menu
  entries, dialog operations, preferences, keyboard shortcuts, gestures).
- The **gap** for each feature is *derived from the `desktop`/`iPad` status*
  (✅ present / 🟡 partial / ❌ absent). Categories:
  - **iPad-missing** — desktop ✅/🟡, iPad ❌ → the parity backlog.
  - **iPad-weaker** — desktop ✅, iPad 🟡 → partial; finish it.
  - **Reverse (desktop-missing / desktop-weaker)** — iPad does it and desktop
    doesn't (or does it worse) → candidates to **pull back into desktop**.
  - **Parity** — both have it (859 features; see the per-theme scorecards).
- **priority** P1/P2/P3, **ease** easy/medium/hard, **feasibility**
  feasible / hard / infeasible / **restricted** (closed-firmware controller
  config/upload — IAP-gated, deliberately low priority) / **declined**
  (feasible but deliberately won't-build — a scope decision, not a platform
  limit).

## Headline numbers

| Metric | Count |
|---|--:|
| Features audited | 1,200 |
| **At parity** (both platforms) | 864 (**~72%**) |
| **iPad-missing** (desktop has, iPad doesn't) | 181 |
| **iPad-weaker** (partial on iPad) | 63 |
| **Reverse — desktop-missing/weaker** (iPad ahead) | ≈ 92 |
| Infeasible on iPad (platform limits) | 60 |
| Restricted (closed-firmware / IAP) | 23 |
| Declined — won't build (scope decision) | 5 |
| Deferred (AC toolbar cluster, P3) | 15 |
| Deep-dialog surfaces still to enumerate | 32 |

Each feature's gap is derived from its desktop/iPad status (✅/🟡/❌); the
**% Parity** column in the scorecard below is *parity ÷ features* per theme.
Overall the iPad sits at **~72% parity** with the desktop, with the biggest
remaining backlogs in Preferences (theme 11), Layout (06), and Tools (13).

The everyday workflow (create/open/save sequences, place/edit effects, color &
value curves, timing, render, playback, most effect panels) is **at or near
parity**. The gaps cluster in: the **AC-lights toolbar** (deferred to P3 on
2026-06-11 — revisit touch-first only on user demand), **deep layout
dialogs** (node layout, wiring, submodel/custom-model geometry tools — viewpoints
are at parity), **backup/restore**, **legacy importers**, **closed-firmware
controller upload** (intentionally IAP-gated), and **scripting/automation**
(infeasible on iOS). In the other direction, the iPad has grown **~114
capabilities the desktop lacks or does worse** (theme 14) — many are touch/pencil
idioms, but a real set are feasible desktop pull-backs led by a **P1**: adjusting
transition (fade) times by dragging on the effect grid (desktop has no grid
fade-drag), plus edge auto-scroll while dragging, snap-to-grid, a persistent loop
region, actionable Check-Sequence navigation, and `.xsqz` in-place round-trip.

## Parity scorecard by theme

| # | Theme | Feats | Parity | % Parity | iPad-missing | iPad-weaker | Reverse (dsk gap) | Infeasible/Restr |
|---|---|--:|--:|--:|--:|--:|--:|--:|
| 01 | [File Lifecycle & Sequence Management](01-file-lifecycle.md) | 71 | 47 | 66% | 13 | 6 | 6 | 4 |
| 02 | [Sequencer Grid & Effect Editing](02-sequencer-grid-editing.md) | 117 | 95 | 81% | 10 | 4 | 8 | 3 |
| 03 | [Timing Tracks & Audio](03-timing-audio.md) | 83 | 73 | 88% | 3 | 3 | 4 | 3 |
| 04 | [Effects & Effect Setting Panels](04-effects-and-panels.md) | 87 | 78 | 90% | 2 | 2 | 5 | 0 |
| 05 | [Color Panel](05-color-and-value-curves.md) | 81 | 65 | 80% | 7 | 3 | 6 | 1 |
| 06 | [Layout: Models](06-layout-models-preview.md) | 141 | 109 | 77% | 12 | 13 | 7 | 3 |
| 07 | [Setup](07-setup-controllers-upload.md) | 86 | 68 | 79% | 12 | 3 | 3 | 11 |
| 08 | [Import & Export](08-import-export.md) | 71 | 55 | 77% | 9 | 7 | 0 | 1 |
| 09 | [Render & Playback](09-render-playback.md) | 53 | 46 | 87% | 4 | 2 | 1 | 0 |
| 10 | [Presets](10-presets-jukebox-views-perspectives.md) | 85 | 67 | 79% | 12 | 5 | 0 | 7 |
| 11 | [Preferences](11-preferences-settings.md) | 131 | 53 | 40% | 67 | 10 | 1 | 19 |
| 12 | [AI](12-ai-automation-scripting.md) | 48 | 29 | 60% | 17 | 0 | 2 | 16 |
| 13 | [Tools](13-tools-diagnostics-help.md) | 49 | 33 | 67% | 13 | 2 | 1 | 4 |
| 14 | [Reverse Parity](14-reverse-parity-ipad-only.md) | 97 | 46 | 47% | 0 | 3 | 48 | 10 |
| — | **Total** | **1,200** | **864** | **~72%** | **181** | **63** | **≈92** | **82** |

## The roadmap — P1 iPad gaps (build first)

These are the highest-value desktop features iPad still lacks (or only
partially has). Sorted by ease. *(weaker)* = iPad has a partial version.

| Feature | Theme | Ease | Feasibility | Notes |
|---|---|---|---|---|
| Unsaved-changes handling on quit/background *(weaker)* | 01 | easy | feasible | Desktop prompts on quit; iPad silently saves on backgrounding because Stage-Manager pill close cannot host an alert (Not |
| Serial port selection *(weaker)* | 07 | easy | feasible | Desktop port dropdown; iPad enumerates system ports or freeform (no hw serial) + FPP tty/i2c/spi ports. |
| Drag-drop models in layout sidebar *(weaker)* | 14 | easy | feasible | Parity; iPad in progress. |
| Visualizer: drag-drop wiring *(weaker)* | 14 | medium | feasible | Desktop visualizer supports drag-drop wiring; iPad implementation in progress. |

> **Landed (2026-06-03):** Export House Preview Video — Tools → "Export House
> Preview…" renders the preview offscreen at a chosen resolution via
> `XLHousePreviewVideoExporter` → core `VideoWriter`. (Per-model video export
> landed alongside it; see theme 08.)

> **Reconciled (2026-06-11):** desktop commits since the 2026-06-08 plan sync,
> with dispositions:
> - **#6523 Transition Blur** — core `PixelBuffer` render + shared
>   `Blending.json` sliders: render and sliders auto-shared to iPad; only the
>   per-transition **enable gate** is an iPad gap (new ipad-weaker row, theme 04).
> - **#6515 Shift+double-click row → expand/collapse strands** — core op shared;
>   iPad reaches it via the row context menu. Parity by idiom (new row, theme 02).
> - **#6516 link icon on visualizer model cards** — iPad-inspired desktop add;
>   narrows the theme-14 "from-base badge" desktop-weaker row.
> - **#6518 port management for from-base models** — core `ModelManager` only;
>   auto-applies to iPad (note on theme 06 row).
> - **#6511 Lua/REST batch sequence importing** — desktop automation; infeasible
>   on iPad (new row, theme 12).
> - **`f9df1b052` MH color-wheel flip + double selector** — extends the existing
>   theme-04 Moving-Head preset-panel gap row.
> - **`356205b5d` default transition-adjust = 50** — core `PixelBuffer`;
>   auto-shared, no row needed.
> - Remaining June 9–11 commits are bug/crash fixes in shared core or
>   desktop-only code paths — no parity impact.

> **Landed (2026-06-11): Phase 1 "quick wins" wave — 30 rows flipped to
> parity** across themes 01/03/04/05/08/09/10/11/13: position/seek slider,
> Replay Section, render-progress gauge, Shift Effects / +Timing / Selected
> (with snapshot undo), preset search/reorder/update/collision-check,
> ColorCurve presets + export, ValueCurve export, Update Palette bulk-apply,
> Revert To Last Saved, Tools → Export Effects (CSV report via new core
> `import_export/ExportEffectsReport`), Tools → Generate AI Image entry,
> transition-blur enable gate, snap-to-timing-marks + double-tap-play
> toggles, and the 7 Check-Sequence disable toggles.

> **Landed (2026-06-11, second batch — 11 more rows to parity):** Cut effect
> (⌘X + context menu), Paste Mode picker (By Time / By Cell), move-effect-
> between-layers (⌥↑/⌥↓), Play Effect / Play Model loop-region idioms,
> Enable Render on All Models, Delete Multiple Layers (themes 02);
> per-tag Delete + Next/Prior tag navigation (theme 03); AI structured
> model auto-mapping in the import wizard (theme 12); tooltip rendering +
> canvasMode auto-enable (theme 04); Help ▸ Key Bindings sheet (theme 13);
> Info-tab Hash / Overwrite-Tags / editable-Duration fields (theme 01).

> **Landed (2026-06-11, Wave 2 — 18 more rows to parity):** controllers
> cluster (global output settings, per-universe editing, ping indicator,
> proxy pre-upload validation, auto-upload-on-enable, bulk OSF upload,
> Force Local IP, Open Proxy — theme 07); LOR `.lms`/`.las` import via new
> wx-free core reader `import_export/LORMusic` (theme 08); media manager
> Add/Bulk-Find/Reload + Cleanup File Locations (themes 01/13); sequencer
> drag-create, modifier-tap additive select, top-level row drag-reorder
> #6507 (theme 02); Sketch editor close-path/curve-authoring/reorder/SVG
> import via new shared core `effects/SketchSVGImport` — the desktop panel
> was refactored onto the same helper (theme 04).

> **Landed (2026-06-11, Wave 3):** Faces/States visual node picker
> (NodeRangePickerSheet over the SubModel Metal picker); Node Layout +
> Wiring View sheets; Song Structure Regions UI (band overlay, region
> edit/views menus, create-from-timing-track) — bulk effect ops + per-
> region export still open; Moving Head waypoint path canvas + preset
> strips + color-wheel picker; preset apply-mode (Relative/Using
> Layers/Auto) + "From Base" preset section + Display Elements bulk ops
> + Copy-To-Master + Import View; preview delete/rename, 2D
> center-0/canvas/grid setters, overlap highlighting, group clone,
> aspect-ratio fix, center offset, polyline segment size (themes
> 02/04/06/10). **Desktop pull-backs:** fade-drag confirmed already at
> parity via #6494; edge auto-scroll while dragging, layout
> snap-to-grid, case-sensitive Find, default ⌘R Render All, Copy
> Palette String, ValueCurve clipboard copy/paste — implemented on the
> **`desktop-pullbacks` branch** (desktop is in release freeze), pending
> post-release merge; the affected rows stay desktop-missing until then.

> **Landed (2026-06-11, Wave 4 — recent-desktop-feature parity):** Song
> Structure region bulk ops wired (Copy-to-Region, Apply-Palette,
> Fill-from-Marks; export analyzed + tracked); import wizard stacked/merge
> mappings (#6474), multi-xmaphint load, sort-submodels (#4636), Edit
> Display Elements mid-import (#6477); Command Palette ⌘⇧K (#6258);
> Replace Model multi-target dialog (#4462); Bulk Edit Rotate X/Y/Z;
> SubModels Symmetrize via new shared core `models/SubModelSymmetrize`
> (desktop dialog refactored onto it); Sketch path management (#5871,
> list + descriptions + delete); HD ProRes 1080p model export (#6506);
> FPP Player-Only→xLights-Only (#6453); AutoSize universe warning
> (#4123); Copy Layers/SubModels to Models; CSS + recent color swatches
> (`XLColorSwatchPicker`); WLED mDNS discovery wired into iPad discovery
> (+ `_wled._tcp` Bonjour entitlement); verified-shared: State/Faces on
> SubModels, AI image model picker, Per-Model-Default group option,
> Duplicate-include-submodels. A 678-commit git-log audit added 13
> missing README.txt entries and 3 new tracked gaps (#5064 paste-at-
> original-time, #6438 multi-model import positions, FPP-Connect input
> push #2747).

> **Landed (2026-06-12, Wave 5 — 45 more rows to parity, 829/1,187 ≈ 70%):**
> Preferences sweep (render-on-save, render-cache cap, default model
> blending, effect-background/transition-mark/alternate-timing/grid-spacing
> toggles, render bell, backup-on-save snapshots, startup cache purge,
> rename-alias mode); FPP Connect Slice B (input-universe push #2747,
> per-FPP config upload, non-FPP immediate-output, controller Sort menu,
> Export Controller Connections XLSX); Import Notes via new wx-free
> `media/NoteImporter` (self-contained MIDI reader + MusicXML + Audacity),
> Papagayo .pgo export, audio WAV/M4A export
> (`AudioManager::WriteCurrentAudioToFile`), two-finger waveform
> range-drag; LOR Pixel Editor `.lpe` + HLS `.hlsIdata` core importers
> (`import_export/LORPixelEditor`, `HLSFile`); SubModel import
> (.xmodel/CSV/other-model), Export as .xmodel, GDTF fixture import,
> make-start-channel-valid trio, real-world dimension readouts, #6438
> verified-shared; Paste at Original Time ⌥⌘V (#5064), Copy Effects incl
> SubModels, Find Possible Source Effects, per-region .xsq export with
> trimmed audio; Display Elements sort/keyboard-delete/effect-sequence
> mode, row-heading has-effects stripe + tag-color chip; Find Effect Data
> sheet, Color Dropper eyedropper, Search Show Folders, User Lyric
> Dictionary editor; AI image black-background removal + crop/resize.
> Notable deferrals: Default View for new sequences (needs show-level view
> persistence), Vixen 2 `.vix` (wx-bound FileConverter), Download
> Sequences/Lyrics (models-only vendor catalog core), Convert-To-Effect
> render-down, single-color row swatch.

> **Landed (2026-06-12, Wave 6 — 29 more rows to parity, 858/1,188 ≈ 72%):**
> Convert To Effects render-down (blocking `RenderModelAndWait` + faithful
> `DoConvertDataRowToEffects` port); Vixen 2 `.vix` importer (new shared
> `import_export/ImportDataRow` reused by HLS); Download Sequences/Lyrics
> via new wx-free `MusicCatalog` core + browser; Tip of the Day; Restore
> Backup sheet over the Wave-5 snapshots; read-only open guard; Render
> Mode picker; Change Show Folder Temporarily; import timeline density
> strips; open `.xbkp`; DMX channel remap grid; Pictures resize-on-add;
> per-effect Reset to Defaults; preset render thumbnails (+ Hide Preset
> Previews pref); suspend background render ⇧⌘R; single-color row swatch;
> layout model clipboard ⌘C/⌘V (`com.xlights.layoutmodels` UTI,
> cross-sequence); node-inspect probe; handle-size setting; GDTF mode
> picker; submodel import from external rgbeffects; zone indicator;
> Export All Regions; scoped deletes; collapse models/layers split;
> prefs batch 2 (persistent autosave interval, zoom-to-play-marker,
> diagnostic email, hide color-update warning). Re-scoped: Pre/Post-ms
> is an audio re-encode (feasible via Wave-5 encode path, P3); Effect
> Wheel declined (touch idiom); Data Layers has a 5-step plan recorded.

### Deferred — AC toolbar cluster (decision 2026-06-11)

The entire **AC / Auto-Color toolbar** (Select / Off / On / Shimmer / Twinkle /
Intensity / Ramp Up / Ramp Down / Ramp Up-Down / Fill / Cascade / Parm1-Parm2 /
Foreground / Background / Disabled — 15 rows, formerly most of the P1 table) is
**deferred to P3**. A moded toolbar + single-letter-key workflow is a
mouse/keyboard idiom of uncertain touch value. If user demand surfaces, revisit
as a *touch-first* design that applies the wx-free core AC render ops to the
iPad's existing selection idioms (marquee, column span-select) — not as a port
of the desktop `mRange*` cell-cursor. Full detail in
[theme 02](02-sequencer-grid-editing.md)'s Deferred section.

### Declined — won't build (decision 2026-06-11)

Feasible but deliberately out of scope; not backlog:

- **Jukebox on iPad** (theme 10, 4 rows) — low touch value. Core
  `JukeboxButtonData` must keep round-tripping through `.xsq` load/save on iPad
  (it does today); that data guarantee is the only ongoing obligation.
- **Centralized Preferences dialog on iPad** (theme 11) — context sheets are
  the intended iPad settings model. The *individual settings* gaps remain
  backlog; only the single-entry-point goal is dropped.

## P2 iPad gaps (build next)

The medium-priority parity backlog. Many are **easy** and make good quick wins
(seek slider, preset search/reorder, value/color-curve presets, Revert-To,
Export Effects). (The **AC-lights toolbar** cluster that used to dominate this
list is deferred — see above; Jukebox and the centralized Preferences dialog
are declined.)

| Feature | Theme | Ease | Feasibility | Notes |
|---|---|---|---|---|
| Playback volume *(weaker)* | 03 | easy | feasible | Functionally equivalent; iPad lacks the named radio presets (cosmetic). |
| View toolbar (panel toggles) *(weaker)* | 10 | easy | feasible | Preview/inspector toggles covered. |
| Preview delete | 06 | medium | feasible | Desktop ID_PREVIEW_DELETE_ACTIVE. No iPad delete-layout-group bridge. |
| Preview rename | 06 | medium | feasible | Desktop ID_PREVIEW_RENAME_ACTIVE. No iPad rename-layout-group bridge. |
| Import Effects from LOR Pixel Editor .lpe | 08 | medium | feasible | wx-bound parser; needs core extraction + bridge. |
| Import Effects from HLS .hlsIdata | 08 | medium | feasible | wx-bound; low frequency. |
| Export Effects to file (<effects> .xsq fragment) | 08 | medium | feasible | Core exists; needs Tools entry + XLSequenceDocument exporter. |
| Import vendor model / vendor music (online catalog) *(weaker)* | 08 | medium | feasible | Model browser present on iPad; verify music-download breadth matches desktop. |
| Apply mode: Relative vs Using Layers | 10 | medium | feasible | Desktop passes _layerMode to ApplyEffectsPreset; iPad applyPreset has no layer-mode arg surfaced. |
| File toolbar (Open/New/Save/SaveAs/RenderAll) *(weaker)* | 10 | medium | feasible | Equivalent via native idiom. |
| Backup On Save | 11 | medium | feasible | No iPad backup-strategy UI; iPad relies on autosave + iCloud/Files version history. |
| Effects Grid - Paste As (Relative/Layers) | 11 | medium | feasible | Desktop also has PASTE_BY_CELL/PASTE_BY_TIME bindings. iPad paste is fixed. |
| Inline panel toggles F1-F4 (Settings/Color/Layer/Blending) *(weaker)* | 11 | medium | feasible | iPad inspector tabs always available; no F-key tab cycling. |
| Tools > Download Sequences/Lyrics | 13 | medium | feasible | Marketplace/community sequence+lyric download. iPad needs a browser-style download-into-show-folder sheet. |
| Tools > Convert *(weaker)* | 13 | medium | feasible | iPad ImportEffectsView covers the modern formats; the gap is legacy formats (LMS/LSP/HLS/Vixen2). |
| Tools > Search for Show Folders | 13 | medium | feasible | Scans drive for show folders. iPad has Files.app integration; scoped scan + results sheet straightforward. |
| View > Windows > Color Dropper | 13 | medium | feasible | Dockable eyedropper to sample preview colors. iPad preview has no color sampler; add eyedropper gesture + bridge to read |
| View > Windows > Find Effect Data | 13 | medium | feasible | Distinct from iPad Find/Replace: FindDataPanel queries effects by property values; iPad FindReplaceSheet only searches timing-mark labels. |
| Restore Backup dialog | 01 | hard | feasible | Pick backup date, restore layouts/sequences. A narrower in-show .xsq snapshot restore is the realistic iPad substitute. |
| Moving Head: waypoint path canvas | 04 | hard | feasible | Desktop draws Bezier paths; iPad MovingHeadPathRowView only displays + clears. |
| Effect Assist as first-class surface *(weaker)* | 04 | hard | feasible | Only Sketch+Morph inlined on iPad; Pictures absent. No detached-window concept on iPad (acceptable touch idiom). |
| Node Layout (node-grid visualization) | 06 | hard | feasible | Desktop ShowNodeLayout/NodeSelectGrid (LayoutPanel.cpp:6156). iPad has no node-grid view. |
| Wiring View (model node wiring) | 06 | hard | feasible | Desktop ShowWiring (LayoutPanel.cpp:6159). iPad 'Visualize' is controller-level, not model wiring. |
| Faces editor *(weaker)* | 06 | hard | feasible | iPad FaceStateEditorSheet edits faceInfo as attr key/value maps (fully bridged setFaceInfo/faceInfoForModel). Desktop Mo |
| States editor *(weaker)* | 06 | hard | feasible | Same as Faces: iPad attr-map editor vs desktop ModelStateDialog node-grid UI. |
| Export video: bitrate field *(weaker)* | 08 | medium | feasible | Per-model submenu + house-preview sheet pick codec/resolution; no explicit bitrate field (VideoWriter chooses). Codec/quality selection itself ✅ landed 2026-06-03. |
| Keyboard-shortcut rebinding editor | 11 | hard | feasible | Desktop editor is wired (File ▸ Key Bindings, 137 bindings, 4 scopes). iPad shortcuts are static in XLightsCommands.swift. |
| Tools > Test (Pixel/Light Test) | 13 | hard | hard | Launches PixelTestDialog driving controller outputs directly (OnActionTestMenuItemSelected:4172 stops timers/output, ope |
| Tools > Prepare Audio | 13 | hard | hard | Resample/normalize via FFmpeg. FFmpeg core excluded from iPad build. AVFoundation reimpl possible but heavy. |
| Song Structure Regions (entire feature) | 02 | hard | feasible | New desktop #6268: named/colored timeline regions, named views, boundary drag, ruler+grid overlay, region bulk actions, per-region .xsq export. Core SongStructureManager is wx-free and already round-trips on iPad (data preserved); only SwiftUI editing UI + Metal overlay missing. Bulk-action ops mostly wrap existing core/SequenceElements mutators. |

### Additional P1 / P2 gaps

More iPad-missing gaps from the deep-dialog and interaction-level passes; full
evidence lives in each theme doc's scorecard.

| Feature | Theme | Pri | Gap | Ease |
|---|---|---|---|---|
| Convert selected effects to a different type | 04 | P2 | ipad-missing | hard |
| Moving Head: "Link" tab (snap end Pan/Tilt to next effect + force Dimmer to 0) | 04 | P2 | ipad-missing | medium |
| SubModel import: from Model / File / Layout / Downloads | 06 | P2 | ipad-missing | hard |
| SubModels: output-to-lights live test toggle | 06 | P2 | ipad-missing | medium |
| Real-world dimension readouts (ruler-calibrated) | 06 | P2 | ipad-missing | medium |
| FPP Connect: immediate-output upload for non-FPP devices | 07 | P2 | ipad-missing | medium |
| Import media options (sequence package asset destinations) | 08 | P2 | ipad-missing | medium |
| Has-effects yellow indicator stripe | 10 | P2 | ipad-missing | medium |
| Display Elements: Show All / Hide All / Hide Unused models | 10 | P2 | ipad-missing | medium |
| Display Elements: Select Used/Unused/All; Remove Unused | 10 | P2 | ipad-missing | medium |
| Display Elements: Sort models (many strategies) | 10 | P2 | ipad-missing | hard |
| Display Elements: Undo (panel-local) | 10 | P2 | ipad-missing | medium |
| Image save to file with black-background removal | 12 | P2 | ipad-missing | medium |


## Reverse parity — pull iPad innovations back into the desktop

The iPad app has accumulated genuinely better UX in places. These are ranked
candidates to upstream into `src-ui-wx/`. High-value picks: **per-property
Copy/Paste/Reset value** in effect panels, **1-click model Duplicate**, the
**Move/Rotate/Scale tool picker + axis lock**, **view-object rename/duplicate**,
**inline AI-image launch from the Pictures effect**, and **sequencer
Find/Replace**.

> **The full reverse-parity set lives in
> [theme 14](14-reverse-parity-ipad-only.md)**, including interaction-level
> rows. The table below is a high-value subset; the top pull-back is the **P1**
> below.

| iPad feature | Theme | Priority | Ease | Notes |
|---|---|---|---|---|
| **Adjust transition (fade) times by dragging on the effect grid** | 14 | **P1** | medium | Desktop has no fade-drag on the grid; transitions set only via the Blending panel. iPad `EffectsMetalGridView` `Kind.fadeIn/fadeOut`. |
| Create effect: select palette + tap/drag grid *(desktop weaker)* | 02 | P1 | hard | Desktop drag-from-toolbar; iPad tap palette (selectPaletteEffect) then tap empty cell (addEffectFromPaletteTap VM:5334). |
| Media Relocation on file pick | 01 | P2 | medium | iPad copies picked media under show/media folder at pick time; desktop free path. |
| Auto-Save recovery prompt on open *(desktop weaker)* | 01 | P2 | easy | iPad auto-detects newer .xbkp at open and offers recovery; desktop has no auto-prompt. |
| Select all effects in column (time span) | 02 | P2 | easy | iPad-only selectAllEffectsInColumn VM:4401 spans all model rows. Desktop AC cell-range is a different model. |
| Merge timing mark with next *(desktop weaker)* | 03 | P2 | medium | Desktop merges only via combine-every-N dialog; iPad has explicit single-pair 'Merge with Next'. Desktop could add a per |
| Per-property Copy Value | 04 | P2 | medium | Verified: desktop BulkEditSlider::OnRightDown only offers 'Bulk Edit' dialog, no per-property value clipboard. |
| Per-property Paste Value | 04 | P2 | medium | Prefix-guarded pasteboard. No desktop equivalent. |
| Per-property Reset to Default | 04 | P2 | medium | Writes metadata default + suppressIfDefault. No desktop per-property reset. |
| Multi-select Set Checked / Unchecked *(desktop weaker)* | 04 | P2 | easy | Desktop bulk-edit dialog; iPad explicit Set Checked/Unchecked on N. |
| ValueCurve copy to clipboard | 05 | P2 | easy | Desktop VC dialog has no clipboard buttons (uses dropper drag). iPad wraps the curve in an xlvc:v1: envelope to UIPasteboard. |
| ValueCurve paste from clipboard | 05 | P2 | easy | No desktop dialog paste. iPad unwraps the envelope + replaceWithSerialised. |
| Move/Rotate/Scale tool picker + axis lock *(desktop weaker)* | 06 | P2 | medium | iPad LayoutEditorToolToolbar (move/rotate/scale + X/Y/Z axis lock + snap). Desktop uses centre-cycle + direct gizmo hand |
| Model duplicate *(desktop weaker)* | 06 | P2 | medium | iPad explicit Duplicate (XLMetalBridge.duplicateModels +offset). Desktop has Copy/Paste but no 1-click Duplicate. |
| Vendor model browser (download .xmodel/wiring) | 06 | P2 | hard | iPad VendorBrowserSheet (XLVendorCatalog). Desktop downloads .xmodel manually. |
| Preview camera roll (twist) *(desktop weaker)* | 06 | P2 | easy | iPad two-finger rotate (Z twist); desktop trackpad rotate. |
| View object rename *(desktop weaker)* | 06 | P2 | easy | iPad renameViewObject + sheet. Desktop has no dedicated rename for view objects. |
| View object duplicate *(desktop weaker)* | 06 | P2 | easy | iPad duplicateViewObject. Desktop not exposed for view objects. |
| Audio scrub (audible bursts on drag) | 09 | P2 | medium | iPad plays 50ms snippets while dragging the ruler so users hear position. Reverse-parity; desktop would need a portable  |
| Play Loop for timing mark (double-tap) *(desktop weaker)* | 09 | P2 | easy | iPad double-tap a timing mark loops that mark's bounds. Desktop loops a region but lacks the one-gesture per-mark form. |
| Find / Replace (Cmd+F) | 11 | P2 | medium | Timing-mark label search. Desktop has no sequencer find. |
| Effect Presets browser (Cmd+Shift+P) *(desktop weaker)* | 11 | P2 | medium | Both have presets; binding differs (desktop panel/F10 vs iPad Cmd+Shift+P). |
| Render All (Cmd+R) *(desktop weaker)* | 11 | P2 | easy | Desktop RENDER_ALL exists but no default key — toolbar only. iPad binds Cmd+R. |
| Previous / Next Effect (Left/Right Arrow) | 11 | P2 | medium | Desktop arrows do other things. |
| Effect Above / Below (Up/Down Arrow) | 11 | P2 | medium | Row navigation. |
| AI Image inline launch from Pictures effect *(desktop weaker)* | 12 | P2 | easy | iPad UX superior: AI button on Pictures filename row writes path back to E_TEXTCTRL_Pictures_Filename. Desktop reaches g |
| Map-from-Lights structured-light scan → Custom model *(desktop weaker)* | 14 | P2 | hard | Desktop has this on macOS via Continuity Camera (DiscoverContinuityCameras + PresentScanWindow); iPad adds on-device camera + LiDAR. |
| Open via Files/AirDrop/share (.xsq) *(desktop weaker)* | 01 | P3 | medium | iPad obtains security scope + queues open until show folder loads. Desktop has no in-app share-target. |
| iCloud download status badges + download-then-open | 01 | P3 | medium | No on-demand cloud materialization model on desktop. |
| Sequence picker sort order (Name/Modified/Rendered/Out-of-date) *(desktop weaker)* | 01 | P3 | medium | iPad picker offers 4 sort orders persisted per-surface. |
| Pick transition TYPE from fade handle *(desktop weaker)* | 02 | P3 | medium | iPad long-press fade handle -> transition picker (onRequestTransitionMenu:1848). Desktop sets type only via Blending pan |
| Split timing mark at play marker (per-mark) *(desktop weaker)* | 02 | P3 | medium | Desktop has TIMING_SPLIT keybinding but no per-mark right-click item. iPad splitTimingMark VM:3634. |
| Merge timing mark with next (per-mark) | 02 | P3 | medium | iPad-only mergeTimingMarkWithNext VM:3662. Desktop merges by deleting boundary, no explicit op. |
| Two-finger marquee (touch idiom) | 02 | P3 | easy | iPad-only; desktop uses single-button drag rect. Touch idiom, no desktop need. |
| Pencil double-tap / squeeze -> undo | 02 | P3 | easy | iPad-only UIPencilInteraction. No desktop hardware. |
| Pointer-hover handle highlight (Magic Keyboard) *(desktop weaker)* | 02 | P3 | easy | iPad B30 hover; desktop has mouse-move cursor feedback (equivalent idiom). |
| Double-tap mark -> loop-play | 03 | P3 | easy | iPad-only touch idiom; sets loop region to mark range. |
| Apple Pencil precision edge grab | 03 | P3 | easy | iPad-only; halves edge slop with Pencil. No desktop analog needed. |
| Sound classification (SoundAnalysis, Apple) *(desktop weaker)* | 03 | P3 | hard | Both Apple-only; desktop Win/Linux lacks it (platform limit, not iPad). |
| Apply Filename Only to selected | 04 | P3 | easy | iPad-only nicety for media effects. |
| Palette: Copy palette string to clipboard | 05 | P3 | easy | iPad menu Copy Palette String -> UIPasteboard. No Copy item in desktop OnBitmapButton_MenuPaletteClick; trivial ID_MNU_C |
| ValueCurve delete preset | 05 | P3 | easy | Desktop dialog/panel has only a clear-curve button, no per-file delete. iPad swipe-deletes presets (deleteSavedValueCurve). |
| ColorCurve point position numeric field | 05 | P3 | easy | iPad lets you type the selected point x. Desktop only drags on the canvas. |
| Map-from-lights (FPP structured-light scan) | 06 | P3 | hard | iPad MapFromLightsWizard (camera + FPP). Needs FPP controller + device camera; desktop has no camera path. |
| Controller filter / search | 07 | P3 | easy | iPad name/vendor/model/IP search (filteredControllerRows). Desktop has no search box. |
| Base-show link badge *(desktop weaker)* | 07 | P3 | easy | iPad blue link icon on FromBase rows; desktop conveys FromBase via styling not a dedicated badge. |
| Open-source-firmware badge | 07 | P3 | easy | iPad green shield when caps.openSourceFirmware (LayoutEditorView.swift:1884). Desktop no indicator. |
| Volume continuous slider | 09 | P3 | easy | iPad slider is in the main toolbar (not the waveform pane). Desktop has no slider; reverse-parity nice-to-have. |
| Sequences - FSEQ Zstd compression level (1-22) | 11 | P3 | easy | iPad-only granular slider; desktop folds level into the version choice. |
| Previous / Next Frame (, / .) | 11 | P3 | easy | Desktop has no per-frame step keys. |
| Back / Forward 10 seconds | 11 | P3 | easy | iPad menu items (no key; option-arrow reserved for nudge). |
| Tab / Shift+Tab effect navigation *(desktop weaker)* | 11 | P3 | easy | Not a documented desktop binding. |
| Stretch End Back/Forward (Shift+Left/Right) *(desktop weaker)* | 11 | P3 | easy | No documented desktop Shift+arrow binding. |
| Fine nudge +-1ms (Ctrl+Left/Right) | 11 | P3 | easy | iPad-only fine nudge. |
| Nudge +-1 frame (Option+Left/Right) | 11 | P3 | easy | Frame-interval nudge. |
| Extend to Prev/Next Effect (Shift+Cmd+Left/Right) | 11 | P3 | easy | Butt against neighbour. |
| Nudge to Prev/Next Timing Mark (Ctrl+PgUp/PgDn) | 11 | P3 | easy | No documented desktop binding. |
| Set Tag N at play head (Ctrl+Shift+0-9) | 11 | P3 | easy | Desktop sets tags via UI. |
| Clear All Tags *(desktop weaker)* | 11 | P3 | easy | iPad menu item. |
| Zoom To preset ladder | 11 | P3 | medium | iPad has 8 preset zoom stops in menu. |
| Toggle Preview (Cmd+1) / Inspector (Cmd+2) | 11 | P3 | medium | Desktop has no Cmd+digit panel toggle. |
| Detach inspector tab in new window (Opt+Cmd+E/C/B/U) | 11 | P3 | medium | Desktop F1-F4 toggle inline panels — different model. |
| Detach / Dock House & Model Preview *(desktop weaker)* | 11 | P3 | medium | iPad detaches to scene windows (no key); desktop toggles inline panels. |
| Refresh model list (GET /v1/models) | 12 | P3 | medium | iPad fetches model list when supportsModelListing, flips model prop text→Choice. Desktop users type model name manually. |
| Bridge: AI service ObjC value-type snapshots | 12 | P3 | easy | Bridge plumbing for Swift safety, not a user-facing feature; no desktop analog needed. |
| Bridge: async image/palette/lyric MainActor marshalling | 12 | P3 | easy | Bridge plumbing only; not a user feature. |
| Tools > Edit Layout *(desktop weaker)* | 13 | P3 | medium | iPad has a Tools>Edit Layout menu opening a detached window; desktop only via the Layout tab. Surface difference, not a  |
| Two-finger marquee select (layout canvas) | 14 | P3 | hard | Two-finger long-press+drag rect select. Desktop uses single-pointer left-drag rubber-band (already its idiom). Touch-onl |
| Two-finger marquee select (effects grid) *(desktop weaker)* | 14 | P3 | hard | iPad two-finger marquee (1-finger=scroll). Desktop has equivalent left-drag rubber-band selection. Different input idiom |
| Apple Pencil Pro double-tap / squeeze → undo | 14 | P3 | hard | No stylus event layer in wxWidgets. Apple-only HW interaction. |
| Two-finger pan preview | 14 | P3 | hard | Genuinely iPad-only: no `wxEVT_GESTURE_PAN`/`wxTOUCH_PAN` binding; desktop pans via drag/keys. |
| Two-finger rotate (camera roll) *(desktop weaker)* | 14 | P3 | hard | Desktop binds `wxEVT_GESTURE_ROTATE` (`LayoutPanel::OnPreviewRotateGesture`) but it rotates the *selected model* (axis Shift=X/Ctrl=Y), not the camera. Only the camera-roll target is iPad-only. |
| Double-tap-to-reset camera *(desktop weaker)* | 14 | P3 | easy | Desktop resets via viewpoint menu / keyboard. Minor idiom difference. |
| iCloud Documents auto-sync + status indicator | 14 | P3 | hard | iPad is document-based on iCloud Drive; desktop is classic local-FS. No cloud-document model; separate product, not pari |
| In-app real-time log tail *(desktop weaker)* | 14 | P3 | medium | Desktop View Log hands the file to an external text editor; iPad tails in-app (no external editor on sandboxed iPad). De |
| Interactive media-relocation assistant *(desktop weaker)* | 14 | P3 | medium | Desktop auto-relocates missing media on load and notifies; iPad is an interactive picker. Polish gap only. |
| Find / Replace — case-sensitive toggle | 14 | P3 | easy | Desktop Find is hard-wired case-insensitive. iPad exposes a toggle. Both operate on timing-mark labels. Genuine small de |
| Layout model 8-mode sort picker *(desktop weaker)* | 14 | P3 | easy | Desktop sorts via clickable persisted tree columns; iPad adds explicit 8-mode picker. Polish; functionally equivalent. |
| Batch-render explicit sort-order picker *(desktop weaker)* | 14 | P3 | easy | Desktop sorts via clickable tree columns; iPad adds a dedicated picker. Polish. |
| MapFromLights — known-controller lookup / auto-name *(desktop weaker)* | 14 | P3 | hard | iPad maps scanned FPP host to xLights controller name; Mac scan saves via Save-As. Same core, different surface. |

## Restricted — closed-firmware controller config / upload (IAP-gated, P3)

Per the project direction, controller configuration / visualization / upload is
in-scope on iPad **only for open-source firmware** (FPP, WLED, ESPixelStick,
DDP/generic — these already work on iPad, gated by `caps.openSourceFirmware`).
Everything below is **proprietary/closed firmware** and is deliberately low
priority, likely behind an in-app purchase.

| Feature | Theme | iPad status | Notes |
|---|---|---|---|
| Map-from-lights (FPP structured-light scan) | 06 | present | iPad MapFromLightsWizard (camera + FPP). Needs FPP controller + device camera; desktop has no camera path. |
| Controller config upload (closed firmware) | 06 | partial | iPad caps-gated path; closed/proprietary uploads IAP-gated/restricted, low priority. |
| Upload output (wiring) to controller | 07 | present | iPad uploadOutputForController, gated to open-source firmware (osf). Closed FW = restricted P3. |
| Upload input (universes) to controller | 07 | present | iPad uploadInputForController, osf-gated. |
| Linked input+output upload | 07 | present | Desktop link checkbox; iPad runs input then output sequentially automatically (startControllerUpload). osf-gated. |
| Falcon controller upload | 07 | absent | Closed firmware. IAP-gated P3 on iPad. |
| SanDevices upload | 07 | absent | Closed firmware. Restricted/IAP. |
| HinksPix upload | 07 | absent | Closed firmware. Restricted/IAP. |
| AlphaPix upload | 07 | absent | Closed firmware. Restricted/IAP. |
| J1Sys upload | 07 | absent | Closed firmware. Restricted/IAP. |
| Minleon upload | 07 | absent | Closed firmware. Restricted/IAP. |
| Pixlite / Pixie upload | 07 | partial | Open-FW path in-scope; vendor-FW path restricted/IAP. |
| Export HinksPix configuration | 08 | absent | Closed/proprietary controller firmware -> IAP-gated, low priority. |
| Output - Use Frame Sync (E1.31) | 11 | absent | iPad has no live USB/serial/sACN output. |
| Output - Force Local IP | 11 | absent | Output-stack tuning; out of scope on iPad. |
| Output - Duplicate Frames to Suppress | 11 | absent | None/10/20/40. |
| Other - Link Controller Upload | 11 | absent | None / Inputs+Outputs. Controller-config concern. |
| Other - Controller Ping Interval | 11 | absent | Live-output tuning; out of scope. |
| Lights toggle / FPP Connect (Ctrl+Shift) bindings | 11 | absent | Live output / controller concerns; iPad FPP Connect is a menu item, not bound. |
| Tools > Bulk Controller Upload | 13 | absent | Batch firmware/config to many controllers. Closed-firmware = IAP-gated/restricted. Open-firmware (FPP/WLED/ESPixelStick) subset is |
| Tools > HinksPix Export | 13 | absent | Export to HinksPix (closed firmware) → restricted/IAP. |
| Layout Controllers tab in editor sidebar | 14 | partial | Reverse: desktop is ahead. iPad omits Controllers tab; restricted to open-firmware (FPP/WLED/ESPixelStick/DDP) upload path. Low pr |

## Infeasible on iPad (platform limits, not backlog)

These will not be built — recorded so they're not mistaken for gaps. Reasons
cluster as: **iOS sandbox** (no raw USB/serial/multicast → pixel test, live
output tuning, FPP/E1.31 sync; no whole-folder backup), **App Store policy**
(no runtime code execution → Lua/Python/REST automation; no in-app update
check), **no FFmpeg/VAMP on iPad** (Prepare Audio, VAMP analyzers), **no AUI**
(perspectives/dockable panes), and **no SpaceMouse HID**. Several rows are the
inverse — touch/Pencil idioms with no desktop hardware (these are also listed in
reverse parity as "touch idiom").

| Feature | Theme | Reason |
|---|---|---|
| Backup (manual, F10) | 01 | Whole-show-folder dated copy; no iOS-sandbox equivalent. Users rely on Files/iCloud. |
| Alternate Backup (F11) | 01 | Second offsite backup dir; no iPad equivalent. |
| Backup On Save/On Launch/Subfolders/Purge | 01 | All tied to whole-folder backup; infeasible under sandbox. |
| iCloud download status badges + download-then-open | 01 | No on-demand cloud materialization model on desktop. |
| Two-finger marquee (touch idiom) | 02 | iPad-only; desktop uses single-button drag rect. Touch idiom, no desktop need. |
| Pencil double-tap / squeeze -> undo | 02 | iPad-only UIPencilInteraction. No desktop hardware. |
| Pointer-hover handle highlight (Magic Keyboard) | 02 | iPad B30 hover; desktop has mouse-move cursor feedback (equivalent idiom). |
| VAMP Queen-Mary plugin analyzers | 03 | No VAMP host / FFmpeg on iPad. Built-in detectors cover common cases. |
| Apple Pencil precision edge grab | 03 | iPad-only; halves edge slop with Pencil. No desktop analog needed. |
| Prepare Audio (Reaper/xAudio import) | 03 | Reaper .rpp automation + FFmpeg transcoding; not viable in iOS sandbox. |
| COLOR_DROPPER_TOGGLE keyboard shortcut (F11) | 05 | Toggles the dockable Color Dropper pane (not a per-effect dropper tool). iPad has no such pane, so the binding has no target. |
| SpaceMouse 6-DOF input | 06 | Desktop Mouse3DManager/SpaceMouseSession. No iOS HID SpaceMouse. |
| Pixel test / test output | 07 | Desktop PixelTestDialog drives raw sACN/ArtNet/DDP/serial output frames. iOS sandbox blocks raw multicast UDP + USB/serial -> infeasible. |
| Perspectives: save current | 10 | Saves AUI dock layout; iPad fixed SwiftUI layout, no dockable panes. |
| Perspectives: save as new | 10 | No AUI on iPad. |
| Perspectives: edit / load | 10 | No AUI on iPad. |
| Perspectives: auto-save toggle | 10 | No AUI on iPad. |
| Perspectives: dynamic saved list | 10 | No AUI on iPad. |
| Perspectives panel (manage) | 10 | No iPad equivalent. |
| Suppress Dock (House/Model Preview) submenu | 10 | AUI re-docking control; N/A to SwiftUI scenes. |
| View - Disable Key Acceleration when held | 11 | Desktop key-repeat concept; n/a to touch. |
| View - Zoom To Cursor | 11 | Mouse-only behavior. |
| Effects Grid - Double Click Mode | 11 | Edit Text / Play Timing. No double-click on touch. |
| Output - API Port (xFade/xSchedule) | 11 | Companion-app integration; not on iPad. |
| Other - Hardware Video Decoding + renderer | 11 | iPad uses AVFoundation/VideoToolbox automatically. |
| Other - GPU Rendering (Metal compute) | 11 | iPad always uses Metal. |
| Other - Shaders on Background Threads | 11 | Hidden on macOS/Linux already; Windows-only. |
| Other - Video Export Codec | 11 | iPad picks codec per-export (model submenu / house-preview sheet) rather than via a global preference default. |
| Other - Video Export Bitrate | 11 | iPad has video export but no explicit bitrate field; VideoWriter chooses (constant-quality for HEVC). |
| Other - Use Custom Color Picker | 11 | iPad uses native system ColorPicker. |
| Effect Assist toggle (F8) | 11 | iPad has no Effect Assist panel. |
| Perspectives panel (F12) | 11 | iPad has no perspectives feature. |
| Jukebox panel (Ctrl+F8) + buttons | 11 | iPad has no Jukebox. |
| OpenVINO (local Intel) service config | 12 | Gated on HAVE_OPENVINO_GENAI = Windows/Intel only. Absent on macOS-arm and iPad. |
| Lua script execution | 12 | App Store 2.5.2 forbids runtime code execution. No Lua VM scripting on iOS. |
| Lua/REST batch sequence importing (#6511) | 12 | Doubly blocked: code execution + no HTTP listener. Interactive ImportEffectsView is the iPad path. |
| Python script execution | 12 | No Python interpreter allowed on iOS. |
| Scripts Dialog (browse/download/run/log) | 12 | Enumerates show scripts/, downloads from repos, runs with log capture. Blocked by sandbox + code-exec policy. |
| REST/HTTP automation server | 12 | iOS app sandbox prohibits a background listening-socket server. |
| REST API: sequence management | 12 | Requires the HTTP listener. |
| REST API: model/effect operations | 12 | No HTTP server on iOS. |
| REST API: controller operations | 12 | Server infeasible; closed-firmware uploads separately restricted/IAP-gated. |
| REST API: layout/view operations | 12 | No HTTP server on iOS. |
| REST API: runScript | 12 | Doubly blocked: server + code execution. |
| REST API: media/audio control | 12 | No HTTP server on iOS. |
| REST API: jukebox | 12 | iPad has no jukebox today; server infeasible. |
| REST API: utility functions | 12 | No HTTP server on iOS. |
| Command-line / xlDo automation | 12 | No CLI/argv automation surface on iOS. |
| AI plugin loading (dylib/DLL) | 12 | App Store forbids loading external executable code; iPad passes empty pluginDir. |
| Tools > Run Scripts | 13 | Executes user automation scripts. iOS forbids arbitrary script/process execution. Infeasible. |
| Help > Check for Updates | 13 | Desktop item itself is created DISABLED. App Store manages iPad updates; in-app check not allowed. Infeasible. |
| Map-from-Lights structured-light scan → Custom model | 14 | Desktop has this on macOS via Continuity Camera (DiscoverContinuityCameras + PresentScanWindow); iPad adds on-device camera + LiDAR. |
| Two-finger marquee select (layout canvas) | 14 | Two-finger long-press+drag rect select. Desktop uses single-pointer left-drag rubber-band (already its idiom). Touch-only. |
| Two-finger marquee select (effects grid) | 14 | iPad two-finger marquee (1-finger=scroll). Desktop has equivalent left-drag rubber-band selection. Different input idiom, same outcome. |
| Apple Pencil Pro double-tap / squeeze → undo | 14 | No stylus event layer in wxWidgets. Apple-only HW interaction. |
| Two-finger pan preview | 14 | Genuinely iPad-only: no `wxEVT_GESTURE_PAN`/`wxTOUCH_PAN` binding. (Pinch-zoom & pinch-scale are **parity** via `wxEVT_GESTURE_ZOOM`; two-finger rotate is desktop-weaker — all moved out of this table.) |
| iCloud Documents auto-sync + status indicator | 14 | iPad is document-based on iCloud Drive; desktop is classic local-FS. No cloud-document model; separate product, not parity. |
| MapFromLights — known-controller lookup / auto-name | 14 | iPad maps scanned FPP host to xLights controller name; Mac scan saves via Save-As. Same core, different surface. |

## Deep-dialog surfaces still to enumerate

These deep dialogs are tracked at cluster granularity. Each row below is a real
cluster of operations (often 10–30 sub-commands) that needs a dedicated deep
pass folded into the named theme doc. **The richest under-counted areas are the
SubModels geometry toolbox (~30 ops), the Custom-Model grid editor, the
Wiring-View options, the Display-Elements bulk/sort menu, and the Controller
Visualizer per-port/per-model menus.** Treat this as the punch-list for a
deepening of themes 06, 07, 08, and 10 — 32 surfaces in all.

| Missed surface | Surface type | Suggested theme | iPad guess | Note |
|---|---|---|---|---|
| View > Windows > Video Preview dock toggle | menu | Presets, Jukebox, Display Elements, Views & Perspectives | absent | Distinct dockable Video Preview pane toggle; covered list lists ~16 docks generically but never names this one |
| View > Windows > Find Effect Data dock toggle | menu | Presets, Jukebox, Display Elements, Views & Perspectives | absent | Toggles the Find-Effect-Data pane (node-row source-effect search UI); not individually enumerated in the docks |
| View > Windows > Effect Settings / Colors / Layer Blending / Layer Settings dock toggles | menu | Effects & Effect Setting Panels | partial | The four inspector panes each have a standalone View>Windows show/hide checkbox separate from the F1-F4 inline |
| Display Elements panel: Hide Unused / Remove Unused / Select Unused / Select Used / Select All | context-menu | Presets, Jukebox, Display Elements, Views & Perspectives | absent | Right-click menu in Display Elements has bulk hide/remove/select-unused/used operations; covered list only has |
| Display Elements panel: Sort submenu (By Name, Groups At Top by Size/Count, By Controller/Port, By Start Channel, By Type, Bubble Up Groups, Same as Master View) | context-menu | Presets, Jukebox, Display Elements, Views & Perspectives | absent | 13-way sort submenu for the Display Elements model list; entirely absent from covered list (Layout model-list  |
| Display Elements panel: in-panel Undo | context-menu | Presets, Jukebox, Display Elements, Views & Perspectives | absent | Per-panel undo of model show/hide/reorder operations inside Display Elements; not in covered list. |
| Master View import: 'View From Layout File' / 'Master View From Sequence File' | context-menu | Presets, Jukebox, Display Elements, Views & Perspectives | absent | Covered list has 'Display Elements: Import view config' generically but not these two concrete import sources. |
| Model Group members panel: Copy From... / Sort By Name / Sort By Location / Clear | context-menu | Layout: Models, Groups, Preview, 3D, Submodels/Faces/States/DMX | absent | Group-member editor right-click (copy member list from another group, sort, clear-all); covered list has reord |
| Layout canvas: 'Add to Existing Groups' / 'Remove from Existing Groups' | context-menu | Layout: Models, Groups, Preview, 3D, Submodels/Faces/States/DMX | absent | Right-click model -> add/remove the selection to/from existing groups; covered list has group add/remove membe |
| Layout canvas: 'Add/Edit SubModel Alias' | context-menu | Layout: Models, Groups, Preview, 3D, Submodels/Faces/States/DMX | absent | SubModel-row alias add/edit from the Models panel context menu (distinct from per-model aliases editor dialog) |
| SubModels dialog geometry tools (Generate Slices, Flip/Shift/Reverse all, Symmetrize, Combine/Expand/Compress Strands, Uniform Row Length, Pivot Rows/Columns, Convert Blanks/Zeros, Remove/Suppress Duplicates, Geometric Sort) | context-menu | Layout: Models, Groups, Preview, 3D, Submodels/Faces/States/DMX | partial | ~30 distinct submodel-editing operations; covered list captures SubModels only at create/edit/rename/delete +  |
| SubModels dialog Import (Model/File/Layout/Downloads/State/Face/Custom-overlay/CSV) and Export (CSV/xModel/To-Other-Models) | context-menu | Import & Export | absent | Eight import sources + three export targets for submodels; not in covered import/export lists. |
| Custom Model dialog grid-editing toolbox (Rotate90/Rotate-x, Reverse, Shift, Compress, Trim/Shrink/Expand Space, Convert to Single Node, Copy Layer Fwd/Bkwd, Wiring direction submenus, Create-SubModel-from-row/column/layer) | context-menu | Layout: Models, Groups, Preview, 3D, Submodels/Faces/States/DMX | partial | Large grid-editor right-click menu; covered 'Custom model visual editor (parity)' enumerates none of these per |
| Faces dialog: Import From Model/File/SubModel + 'Export Faces To Other Model(s)' + Shift/Reverse Nodes | context-menu | Import & Export | partial | Faces editor import/export-to-others/node-shift menu; covered 'Export Faces/States/SubModels to other models'  |
| States dialog: Import State Definition, Insert Line Before/After, Move Up/Down, Sort by State, Clear Selected/All, Import From Downloads, Export State(s) To Other Models, Shift/Reverse Nodes | context-menu | Layout: Models, Groups, Preview, 3D, Submodels/Faces/States/DMX | partial | States row-management + import/export menu; covered list only has 'States editor (partial)'. |
| Wiring View dialog: theme (Dark/Gray/Light), Front/Rear, Font Smaller/Larger, Rotate 90, Export Large, Export DXF, Print | context-menu | Layout: Models, Groups, Preview, 3D, Submodels/Faces/States/DMX | absent | Wiring-view display + export options; covered list has 'Wiring View (ipad-absent)' as a single entry with no s |
| Manage Media panel: Reload from Disk / Re-select Image / Bulk Find Images / Re-select <type> / Bulk Find <type>s | context-menu | File Lifecycle & Sequence Management | partial | Media manager right-click relocation/reload menu; iPad has a media-relocation assistant (desktop-missing in co |
| Controller Visualizer per-port ops: Set Smart Remote Type, Set Smart Remote ID and Increment, Remove Smart Remote, Remove All Models From Port, Move All Models To Port | context-menu | Setup, Controllers, Outputs & Upload | partial | Port-level visualizer ops; covered list has generic move/remove-model-to/from-port but not smart-remote-type/c |
| Controller Visualizer per-model ops: Change String Count, Set Brightness/Clear, Gamma, Start/End Nulls, Color Order, Group Count, Set Channel, Set Channel and Chain | context-menu | Setup, Controllers, Outputs & Upload | present | Per-model controller-property edits from the visualizer right-click; covered 'Model controller-properties' lis |
| Controller Visualizer canvas: Print, Save As Spreadsheet, Remove All Models From Controller | context-menu | Setup, Controllers, Outputs & Upload | absent | Visualizer-level print/CSV/clear-controller; CSV/JSON wiring export is covered but Print and Remove-All-from-C |
| Effect Presets tree: 'Sort Group' / 'Sort All' | context-menu | Presets, Jukebox, Display Elements, Views & Perspectives | absent | Right-click sort of preset group vs whole tree; covered list has reorder + search but not the explicit sort-gr |
| Keybinding action FOCUS_SEQUENCER | shortcut | Preferences, Settings & Keyboard Shortcuts | absent | Default binding to move keyboard focus to the sequencer grid; not in covered shortcut list (FILTER_SEQUENCER i |
| Keybinding action SAVE_CURRENT_TAB | shortcut | Preferences, Settings & Keyboard Shortcuts | absent | Context-sensitive 'save the active tab' binding (layout vs sequence); not enumerated among covered save shortc |
| Keybinding actions EXPORT_LAYOUT_DXF / EXPORT_MODEL_CAD / NODE_LAYOUT / WIRING_VIEW (as rebindable shortcuts) | shortcut | Preferences, Settings & Keyboard Shortcuts | absent | Layout-tab default keybindings for DXF/CAD export and node-layout/wiring-view; the menu items are partly cover |
| Keybinding actions TIMING_DIVIDE_12 / TIMING_DIVIDE_16 (1/12, 1/16) | shortcut | Timing Tracks & Audio | absent | Covered 'Divide Timing Marks by N' only lists 2/3/4/6/8; the 1/12 and 1/16 division keybindings/menu options a |
| Keybinding action ALTERNATE_PASTE | shortcut | Sequencer Grid & Effect Editing | absent | Separate paste action toggling by-cell/by-time mode at paste time; covered list has Paste By Cell/Time toolbar |
| Edit menu: 'Shift Selected Effects' | menu | Timing Tracks & Audio | absent | Third Shift variant (offset only the selected effects); covered list has 'Shift Effects' and 'Shift Effects An |
| File menu: 'Open New Instance' | menu | File Lifecycle & Sequence Management | absent | Launches a second xLights process; inherently desktop-only multi-process, never in covered list. |
| Global output settings individual fields: Controller Sync, E1.31 Sync Universe, Max Duplicate Frames To Suppress, Global FPP Proxy | panel | Setup, Controllers, Outputs & Upload | absent | Covered list bundles these as one 'Global output settings (ipad-absent)' line; each is a discrete editable pro |
| Controller list context: 'Insert NULL' / 'Insert DMX/LOR/DLight/Renard' (positional) and 'Activate in xLights Only' | context-menu | Setup, Controllers, Outputs & Upload | partial | Covered list has add-ethernet/serial/null as toolbar buttons but not the positional context-menu Insert varian |
| Sketch Assist canvas right-click menu | context-menu | Effects & Effect Setting Panels | partial | Sketch path-editor context menu (point ops); covered 'Sketch: path editor canvas (partial)' has no context-men |
| Shape / DMX / SubBuffer effect-panel right-click menus | context-menu | Effects & Effect Setting Panels | partial | Each of these effect panels exposes a context menu (emoji/SVG, channel remap/save-load state, subbuffer reset/ |

## Recommended build sequencing (roadmap approved 2026-06-11)

Direction: **iPad gaps first** (far more developers work the desktop side),
with desktop pull-backs **interleaved** as the relevant desktop code is
touched. Keep the plans reconciled as new desktop work lands — some desktop
changes are iPad-inspired and may *close* reverse-parity rows rather than open
iPad gaps.

**Phase 0 — re-baseline (✅ done 2026-06-11):** reconciled desktop commits
through 6/11 (see the *Reconciled* note above), deferred the AC toolbar,
declined Jukebox + centralized Preferences, updated counts.

**Phase 1 — iPad quick wins** ✅ **done 2026-06-11** (all ten items below landed):
1. **Position/seek slider** for coarse scrubbing (theme 09) — flagged the single
   highest-value iPad-missing item.
2. **Shift Effects / Shift Effects And Timing / Shift Selected** (theme 03) —
   the highest-value missing *operation*; common during audio re-syncs.
3. **Replay Section / loop the selection** (theme 10's only P1) — loop-region
   plumbing already exists.
4. **Preset browser polish** (theme 10) — wire the already-existing `movePreset`
   to `.onMove` (zero bridge work), `.searchable`, Update-Preset bridge,
   name-collision pre-check.
5. **ColorCurve presets (load/save/export)** + **VC export-to-file** (theme 05) —
   clone the working ValueCurve preset bridge trio.
6. **Revert To → Last Saved** (theme 01) — discard-confirm + re-open path.
7. **Palette "Update Palette"** bulk-apply + **Copy Palette String** (theme 05).
8. **Render progress indicator** (theme 09) — core already emits the callbacks.
9. **Tools → Export Effects** + **AI-image Tools entry** (theme 13) — clone the
   models-report bridge; the AI sheet already exists.
10. **Cheap pref gates** (themes 03/04/11) — snap-to-timing-marks toggle,
    double-tap-play toggle, Check-Sequence disable-checks toggles, transition
    Blur enable gate (#6523).

**Phase 2 — iPad medium clusters:**
1. **Setup/controllers** (theme 07) — global output settings, per-universe
   editing, ping/health probe, FPP-proxy validation, auto-upload-on-enable,
   then bulk multi-controller upload (open-firmware only).
2. **Legacy importers** (themes 08/13) — extract the LOR `.lms`/`.las` reader
   into `src-core/import_export/` first (unblocks both formats + sets the
   pattern for `.lpe`/HLS later); import merge/stack prompt (#6474 parity).
3. **AI structured model mapping** in the import wizard (theme 12) — core
   `GenerateModelMapping` already shared.
4. **Media manager** (theme 01/13) — Add media, bulk find missing, reload,
   Cleanup File Locations sweep.
5. **Sequencer odds & ends** (theme 02) — drag-create on empty range,
   modifier-click additive select, move-effect-between-layers,
   play-model/play-effect, paste-by-cell toggle, row drag-reorder (#6507
   parity), Cut.
6. **Sketch parity** (theme 04 P1) — close-path, SVG import, path reorder,
   curve segments.

**Phase 3 — desktop pull-backs (interleave as that code is touched):**
1. **Fade-time drag on the desktop grid** (theme 14's P1) — desktop gained fade
   *handles* (#6494); re-verify how much of this gap remains before scoping.
2. **Edge auto-scroll while dragging** + **snap-to-grid for layout models**.
3. **Persistent loop region, actionable Check-Sequence navigation, `.xsqz`
   in-place round-trip, Remove-Unused-Media**.
4. When next touching effect panels: **per-property Copy/Paste/Reset**; when
   next touching layout: **1-click Duplicate**, **view-object rename/duplicate**;
   plus **sequencer Find/Replace** and **inline AI-image from Pictures**.
5. Trivia batch: Copy-Palette-String, VC clipboard copy/paste, case-sensitive
   find, default ⌘R for Render All.

**Phase 4 — hard touch canvases (schedule individually):**
- **Faces/States visual node picker** (theme 06 — biggest qualitative layout
  gap; reuse the custom-model grid paint code), **Node Layout / Wiring View**
  sheets, **Song Structure Regions** SwiftUI/Metal UI (data already
  round-trips), **Moving Head path canvas** + preset/color-wheel panels
  (#6473 / `f9df1b052`).

**Deep-dialog parity (after Phase 2):** SubModels geometry toolbox,
Custom-Model grid editor, Wiring-View display/export options, Display-Elements
bulk/sort menu, Controller-Visualizer per-port menus. Enumerate fully (see the
deep-dialog-surfaces table) before estimating.

**Parked:** AC toolbar (deferred — touch-first redesign only on demand);
Jukebox + centralized Preferences (declined); closed-firmware uploads + bulk
upload + HinksPix export (restricted — behind the planned IAP); the infeasible
list (unchanged).

## Per-theme detail

Each theme doc has the full parity scorecard (including the 858 at-parity
features), the iPad-gap and desktop-gap breakdowns, infeasible/restricted lists,
and per-theme sequencing:

- **01** · [File Lifecycle & Sequence Management](01-file-lifecycle.md)
- **02** · [Sequencer Grid & Effect Editing](02-sequencer-grid-editing.md)
- **03** · [Timing Tracks & Audio](03-timing-audio.md)
- **04** · [Effects & Effect Setting Panels](04-effects-and-panels.md)
- **05** · [Color Panel, Palettes & Value/Color Curves](05-color-and-value-curves.md)
- **06** · [Layout: Models, Groups, Preview, 3D, Submodels](06-layout-models-preview.md)
- **07** · [Setup, Controllers, Outputs & Upload](07-setup-controllers-upload.md)
- **08** · [Import & Export](08-import-export.md)
- **09** · [Render & Playback](09-render-playback.md)
- **10** · [Presets, Jukebox, Display Elements, Views & Perspectives](10-presets-jukebox-views-perspectives.md)
- **11** · [Preferences, Settings & Keyboard Shortcuts](11-preferences-settings.md)
- **12** · [AI, Automation & Scripting](12-ai-automation-scripting.md)
- **13** · [Tools, Diagnostics & Help](13-tools-diagnostics-help.md)
- **14** · [Reverse Parity — iPad features not in Desktop](14-reverse-parity-ipad-only.md)
