# iPad xLights — Plan Index

Pending work to ship the iPad app is tracked across focused
sub-plans here. The top-level
[`iPad-xLights-Plan.md`](../iPad-xLights-Plan.md) (one directory
up) keeps the overall situation; this file indexes the details.

## Active sub-plans

| File | Phase | What's left |
|---|---|---|
| [phase-j-layout-editor.md](phase-j-layout-editor.md) | J — Layout Editor | Authenticated vendor downloads |
| [phase-j-touch-ux.md](phase-j-touch-ux.md) | J — Touch + Pencil UX | Multi-select body drag, sidebar multi-select UI |
| [handle-system-refactor.md](handle-system-refactor.md) | Handle / gizmo redesign | R-9 SpaceMouse 6-DOF, R-10b legacy-constant cleanup, R-8b DrawHandles polish |
| [phase-b-grid-parity.md](phase-b-grid-parity.md) | B — Effects grid parity | B77 MIDI (deprioritized) + 3 deferred |
| [phase-i-import-effects.md](phase-i-import-effects.md) | I — Import Effects | I-5 `.lms`/`.las` (parked) |
| [followups.md](followups.md) | Cross-phase | Data Layers tab, MH waypoint authoring, shader uniform grouping |

## Future / post-MVP

No commitment — captured so we don't lose the design context. Most
items here trace back to the 2026-04-23 gap analysis.

| File | Topic |
|---|---|
| [future-controller-upload.md](future-controller-upload.md) | Controller upload + Pixel Test — **FPP Connect bumped to P1**, plus Bulk Upload, HinksPix, Pixel Test, drag-drop port mapping |
| [future-aux-panels.md](future-aux-panels.md) | Search / Find / EffectTree / Jukebox / SequenceVideo (AP-1 SearchPanel + AP-4 EffectTreeDialog highest value) |
| [future-preferences.md](future-preferences.md) | Preferences (10 panels, 96 options) + ColorManager + backup |
| [future-help-diagnostics.md](future-help-diagnostics.md) | Backup restore + remaining Tools utilities (Cleanup File Locations, Purge caches, Prepare Audio, Generate 2D Path, …) |
| [future-imports-exports.md](future-imports-exports.md) | Vixen 3, MIDI, 14-format export, Convert dialog, Export Models/Effects/Controller-Connections, Download Sequences/Lyrics, Generate Lyrics From Data |
| [future-effect-presets.md](future-effect-presets.md) | Disk-persistent effect presets (G12) + EffectTree pairing |
| [future-pictures-frame-editor.md](future-pictures-frame-editor.md) | Pictures / GIF frame-timing editor + Effect Assist panels (EA-2/EA-3) |
| [future-layout-editing.md](future-layout-editing.md) | Phase S-pro items (active S work moved to `phase-j-layout-editor.md`) |
| [future-custom-models.md](future-custom-models.md) | WV-1 strand wiring diagram, O-10 drag-drop port authoring, VO-5 RulerObject, MA-15 MatrixFaceDownload, advanced VO polish |

The Layout-Editor Controllers tab itself (list management,
right-click Activate/Inactivate, HTTP-scan-with-auth in Discover,
LED ping, etc.) is tracked under "Remaining work" in
[phase-j-layout-editor.md](phase-j-layout-editor.md), not in a
separate `future-*` file — the desktop's Controllers tab has
largely been subsumed by the iPad's Layout Editor Controllers tab.

## Hard misses (no realistic iOS path)

Documented for the record so we don't keep re-litigating:

- **VAMP plugin host** (polyphonic transcription, custom user
  plugins). No iOS replacement.
- **Python scripting** (pybind11 + Python 3 embedding). App Store
  hostile.
- **Lua scripting via JIT** (Tools → Run Scripts on desktop).
  Same App Store concern; the desktop scripts target a UI /
  controller-upload / show-folder API that doesn't have an iPad
  counterpart, so even an interpreted fallback would mostly
  break out of the gate. Logged in
  [`future-help-diagnostics.md`](future-help-diagnostics.md)
  under "No plans to port."
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
4. When a phase completes, delete its sub-plan — git history holds
   the landed prose.
5. The `future-*.md` files are not commitments. Severities and
   effort estimates are recommendations from the gap analysis;
   the team decides what ships.
