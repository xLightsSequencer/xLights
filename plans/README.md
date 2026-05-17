# iPad xLights — Plan Index

Work to ship the iPad app is tracked across focused sub-plans here. The
top-level [`iPad-xLights-Plan.md`](../iPad-xLights-Plan.md) (one
directory up) keeps the overall situation (current state, phase
summary, MVP-remaining list, risks, open questions) and links here
for the details.

## Active sub-plans

| File | Phase | What's left |
|---|---|---|
| [phase-j-layout-editor.md](phase-j-layout-editor.md) | J — Layout Editor | J-0 → J-32 ✓ 2026-05-07..16 (per-type properties for 26 model types, group CRUD + drag-reorder, Controllers tab + Visualize, custom-model editor, DMX deep authoring). Two small import gaps remain: authenticated vendor downloads + layout-group prompt on multi-model `.xmodel` placement |
| [phase-j-touch-ux.md](phase-j-touch-ux.md) | J — Touch + Pencil UX design | Interaction-layer design for the iPad layout editor — toolbar tool selection, persistent modifier toggles, Pencil hover/barrel-tap/double-tap, gesture vocabulary, descriptor-pipeline mapping |
| [handle-system-refactor.md](handle-system-refactor.md) | Handle / gizmo redesign | R-1 → R-10 ✓ 2026-05-11. Descriptor pipeline (`GetHandles` / `DragSession`) owns every user-driven mutation + drawing across Boxed / TwoPoint / ThreePoint / PolyPoint / MultiPoint / Terrain + multi-select on both desktop + iPad. R-9 SpaceMouse + R-10b constant-deletion deferred |
| [phase-b-grid-parity.md](phase-b-grid-parity.md) | B — Effects grid parity | B77 MIDI (deprioritized — low desktop use) + 3 deferred |
| [followups.md](followups.md) | Cross-phase | Small items left over from C / E / J (Data Layers tab, MH waypoint authoring, shader uniform grouping, animated GIF migration, Display Elements filter, add-alias on missing model) |

## Residual sub-plans (phase complete)

Kept around for deferral / caveat reference; landed implementation
prose is in git history.

| File | Phase | Why kept |
|---|---|---|
| [phase-d-preview.md](phase-d-preview.md) | D — Model Preview | "View Objects" coarse toggle, Fit Selected silent no-op caveat |
| [phase-f-window-system.md](phase-f-window-system.md) | F — Window system | Stage Manager position quirk, detached-scene preview state deferral |
| [phase-g-document.md](phase-g-document.md) | G — Document / iCloud | `.piz`/`.zip` UTI deferral, save-back from non-Files providers |
| [phase-h-app-store.md](phase-h-app-store.md) | H — App Store readiness | App shipped; file kept for the submission-prep checklist as a reference for future platforms |
| [phase-i-import-effects.md](phase-i-import-effects.md) | I — Import Effects | I-1/I-2/I-3/I-4 ✓; I-5 (`.lms`/`.las`) parked pending vendor request |

## Future / post-MVP

No commitment — captured so we don't lose the design context. Most
items here trace back to the 2026-04-23 gap analysis. "Status"
column reflects current iPad coverage (verified 2026-05-02);
files marked **shipped** or **partial** keep around residual
notes / cross-references rather than open work.

| File | Topic | Status |
|---|---|---|
| [future-controllers-tab.md](future-controllers-tab.md) | Controllers tab — discovery, list, edit, upload (Phase R / R-pro) | partial — J-31 shipped Controllers tab inside the Layout Editor (list + property pane + Add Ethernet/Serial/Null + Discover sheet); J-32 shipped read-only Visualize wiring view (port mapping). Remaining: drag-reorder/sort modes, FPP Connect, Activate/Inactivate context menu, Pixel Test, controller upload (EX-4..7) |
| [future-aux-panels.md](future-aux-panels.md) | Search / Find / EffectTree / Jukebox / SequenceVideo (Phase L) | partial — Buffer / ValueCurves / Blending / ColorPanel-compound-rows shipped; per-property "Apply to all selected" covers common bulk-edit case; AP-1 SearchPanel + AP-4 EffectTreeDialog still open |
| [future-preferences.md](future-preferences.md) | Preferences (10 panels, 96 options) + ColorManager + backup (Phase M) | partial — PR-9 Services shipped; PR-8 SequenceFile partial (AutoSaveInterval + DefaultBlending only); ColorManager, backup stack, every other panel desktop-only |
| [future-help-diagnostics.md](future-help-diagnostics.md) | RestoreBackup + Tools utilities (Phase P) | partial — About / Help menu / log export / crash telemetry / Check Sequence / in-app log viewer (H-6+T-2 2026-05-17) shipped; backup restore + minor Tools utilities remain (TipOfDay parked, no plans to port) |
| [future-imports-exports.md](future-imports-exports.md) | Vixen 3, MIDI, 14-format export, Convert dialog (Phase N) | partial — `.xsq`/`.xsqz`/`.sup`/`.xtiming`/Papagayo/Audacity/Metronome/Lyrics/LOR-timing + AutoLabel + simple BatchRender + Package Sequence (EX-11 2026-05-17) shipped; Vixen 3 / MIDI / Music XML / 14-format export / Convert dialog still open |
| [future-effect-presets.md](future-effect-presets.md) | Disk-persistent effect presets (G12) + EffectTree pairing | partial — in-session presets shipped; disk persistence + tree UI open |
| [future-pictures-frame-editor.md](future-pictures-frame-editor.md) | Pictures / GIF frame-timing editor + Effect Assist panels (EA-1..3) | partial — EA-1 path editing shipped via `SketchPathEditorRowView`; EA-2 / EA-3 + G6 frame editor open |
| [future-audio-authoring.md](future-audio-authoring.md) | Onset / tempo → timing tracks, spectrogram, pitch contour (Phase J) | shipped — A-1..A-3, A-9..A-11 all on iPad; A-7/A-13 VAMP-blocked |
| [future-ai-palette-generate.md](future-ai-palette-generate.md) | AI palette generation (AI-1) | shipped |
| [future-ai-image-generate.md](future-ai-image-generate.md) | AI image generation (AI-2..AI-5) | shipped |
| [future-layout-editing.md](future-layout-editing.md) | Layout panel — model placement, world layout (Phase S / S-pro) | Phase S promoted to [phase-j-layout-editor.md](phase-j-layout-editor.md) 2026-05-07; S-pro items still here |
| [future-custom-models.md](future-custom-models.md) | Custom model + Face/State + DMX deep + Wiring (Phases T/U/V/W) | substantially shipped — J-22 (Faces / States / Dimming Curve / SubModels), J-23 (custom-model visual editor + SubModel geometry), J-30 (all 8 DMX fixture types + preset/wheel-colour/position-zone/skull/mesh-servo editors), J-13 (TerrainObject heightmap painting). Remaining: WV-1 wiring diagram, VO-5 RulerObject, MA-15 MatrixFaceDownload, advanced VO polish |

## Hard misses (no realistic iOS path)

Documented for the record so we don't keep re-litigating:

- **VAMP plugin host** (polyphonic transcription, custom user
  plugins). No iOS replacement. CoreML-based feature extraction is
  XXL and doesn't preserve plugin extensibility.
- **Python scripting** (pybind11 + Python 3 embedding). App Store
  hostile.
- **Lua scripting via JIT.** Same App Store concern; could ship
  interpreted but workflow value is low.
- **3D Connexion / SpaceMouse input.** Desktop peripheral.
- **FFmpeg whole library.** Bundling adds ~50 MB + licensing /
  review risk; AVFoundation-only is the policy.
- **Custom KeyBindings editor.** Touch-first iPad uses gestures +
  menu items.
- **AUI Manager perspectives.** Single-window iPad layout doesn't
  benefit.

---

## Ground rules for sub-plans

1. Only track **pending** work. Finished items are git history, not
   plan noise.
2. Each bullet should be concrete enough to scope against without
   re-reading the codebase.
3. Keep files focused. If a sub-plan grows past ~600 lines it
   probably needs to be split.
4. When a phase completes, the sub-plan shrinks to a residual file
   covering deferrals + caveats only — or is deleted entirely if
   nothing future-relevant remains.
5. The `future-*.md` files are not commitments. Severities and
   effort estimates are recommendations from the gap analysis;
   the team decides what ships.
