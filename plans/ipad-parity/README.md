# iPad ↔ Desktop Parity Plan

A code-level feature-parity map of the xLights **desktop** (`src-ui-wx/`) and
**iPad** (`src-iPad/`) apps over the shared wx-free `src-core/`. Every status is
grounded in the actual source of both apps, with `file:line` evidence in the
theme docs.

## Start here

- **[`00-overview.md`](00-overview.md)** — headline numbers, the per-theme
  scorecard, the full **P1/P2 roadmap**, **reverse-parity** (iPad→desktop)
  candidates, the **restricted** (closed-firmware/IAP) and **infeasible** lists,
  the deep-dialog surfaces still to enumerate, and recommended build sequencing.

## Theme docs

| # | Theme |
|---|---|
| [01](01-file-lifecycle.md) | File lifecycle & sequence management |
| [02](02-sequencer-grid-editing.md) | Sequencer grid & effect editing |
| [03](03-timing-audio.md) | Timing tracks & audio |
| [04](04-effects-and-panels.md) | Effects & effect setting panels |
| [05](05-color-and-value-curves.md) | Color panel, palettes & value/color curves |
| [06](06-layout-models-preview.md) | Layout: models, groups, preview, 3D, submodels/DMX |
| [07](07-setup-controllers-upload.md) | Setup, controllers, outputs & upload |
| [08](08-import-export.md) | Import & export |
| [09](09-render-playback.md) | Render & playback |
| [10](10-presets-jukebox-views-perspectives.md) | Presets, jukebox, display elements, views & perspectives |
| [11](11-preferences-settings.md) | Preferences, settings & keyboard shortcuts |
| [12](12-ai-automation-scripting.md) | AI, automation & scripting |
| [13](13-tools-diagnostics-help.md) | Tools, diagnostics & help |
| [14](14-reverse-parity-ipad-only.md) | Reverse parity — iPad features not in desktop |

## Conventions

- Status: ✅ present · 🟡 partial · ❌ absent. The **gap** for each feature is
  derived from the desktop/iPad status:
  - **iPad-missing** — desktop ✅/🟡, iPad ❌ → the parity backlog.
  - **iPad-weaker** — desktop ✅, iPad 🟡 → partial; finish it.
  - **desktop-missing / desktop-weaker** — iPad does it and desktop doesn't (or
    does it worse) → candidates to pull back into desktop.
  - **parity** — both platforms have it.
- Priority **P1/P2/P3**; Ease **easy/medium/hard**; Feasibility
  **feasible / hard / infeasible / restricted** (restricted = closed-firmware
  controller config/upload, IAP-gated, deliberately low priority).

## Deep-dialog surfaces still to enumerate

A handful of deep dialogs are tracked at cluster granularity rather than as
individual operations — chiefly the SubModels geometry toolbox, the Custom-Model
grid editor, the Wiring-View options, the Display-Elements bulk/sort menu, and
the Controller-Visualizer per-port/per-model menus. The full list lives in
`00-overview.md`; each needs a focused deepening pass folded into themes 06, 07,
08, and 10.
