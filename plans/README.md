# iPad xLights — Plan Index

Work to ship the iPad app is tracked across focused sub-plans here. The
top-level `iPad-xLights-Plan.md` (one directory up) keeps the "what is the
overall situation" material (current state, phase summary, risks, open
questions) and links here for the details.

| File | Phase | Status |
|---|---|---|
| [phase-b-grid-parity.md](phase-b-grid-parity.md) | B — Effects grid parity with desktop | Partial — gap analysis landed, ~100 items pending |
| [phase-c-inspector.md](phase-c-inspector.md) | C — Effect settings inspector | In progress — biggest remaining inspector surface |
| [phase-d-preview.md](phase-d-preview.md) | D — Model Preview + preview polish | Partial — viewpoints, overlays (grid / background / labels), export, transport parity all outstanding |
| [phase-e-sequence-management.md](phase-e-sequence-management.md) | E — Sequence management (open / save / new / settings) | Not started — app is read-only today; blocks real user testing |
| [phase-f-window-system.md](phase-f-window-system.md) | F — Window system + Display Elements | Not started |
| [phase-g-document.md](phase-g-document.md) | G — Document / iCloud polish | Not started |
| [phase-h-app-store.md](phase-h-app-store.md) | H — App Store readiness | Not started |
| [followups.md](followups.md) | Cross-phase small items | In progress |
| [future-effect-presets.md](future-effect-presets.md) | Deferred — G12 effect presets | Not first-pass |
| [future-pictures-frame-editor.md](future-pictures-frame-editor.md) | Deferred — G6 Pictures / GIF frame-timing editor | Desktop needs redesign too |
| [future-ai-palette-generate.md](future-ai-palette-generate.md) | Deferred — AI palette generation | Needs iOS AI bridge first |
| [future-palette-drag.md](future-palette-drag.md) | Deferred — G18 drag colours between palette slots | Low impact |

Phase A (core-path hardening) and B-Metal (grid render pipeline
migration) are complete and intentionally have no sub-plan. Phase B
shipped the Metal-backed grid, basic selection, drag / resize, and
long-press menu — but substantial desktop parity gaps remain around
multi-select, align / split, timing-track editing, loop region,
follow-playhead, row-heading operations, and waveform variants. Those
are tracked in [phase-b-grid-parity.md](phase-b-grid-parity.md).
Anything else that predates these sub-plans lives in `followups.md`.

Ground rules for sub-plans:

1. Only track **pending** work. Finished items are git history, not plan
   noise.
2. Each bullet should be concrete enough to scope against without
   re-reading the codebase.
3. Keep files focused. If a sub-plan grows past ~600 lines it probably
   needs to be split.
