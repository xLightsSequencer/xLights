# iPad xLights — Plan Index

Work to ship the iPad app is tracked across focused sub-plans here. The
top-level `iPad-xLights-Plan.md` (one directory up) keeps the "what is the
overall situation" material (current state, phase summary, risks, open
questions) and links here for the details.

| File | Phase | Status |
|---|---|---|
| [phase-b-grid-parity.md](phase-b-grid-parity.md) | B — Effects grid parity with desktop | In progress — P0 + P1 closed; ~40 P2 polish items open |
| [phase-d-preview.md](phase-d-preview.md) | D — Model Preview + preview polish | ✓ complete |
| [phase-f-window-system.md](phase-f-window-system.md) | F — Window system + Display Elements | ✓ complete |
| [phase-g-document.md](phase-g-document.md) | G — Document / iCloud polish | ✓ complete |
| [phase-h-app-store.md](phase-h-app-store.md) | H — App Store readiness | H-0 through H-3 ✓ complete; H-4 / H-5 (TestFlight external group + submission metadata) remaining |
| [phase-i-import-effects.md](phase-i-import-effects.md) | I — Import Effects (iPad) | I-1 + I-2 v1.1 + timing-tracks + alias punctuation-strip polish landed. Tools menu, `.xsq` + `.xsqz` source picker, full submodel/strand/node tree, Auto Map at every level, timing-track import popover, apply via core `EffectMapper` family. Pending: model-blending toggle, LSItemContentTypes registration, manual import regression on a vendor sequence. |
| [followups.md](followups.md) | Cross-phase small items | In progress |
| [future-effect-presets.md](future-effect-presets.md) | Deferred — G12 effect presets | Not first-pass |
| [future-pictures-frame-editor.md](future-pictures-frame-editor.md) | Deferred — G6 Pictures / GIF frame-timing editor | Desktop needs redesign too |
| [future-ai-palette-generate.md](future-ai-palette-generate.md) | Deferred — AI palette generation | Needs iOS AI bridge first |
| [future-ai-image-generate.md](future-ai-image-generate.md) | Deferred — G33 AI image generation | Shares AI bridge with palette-generate |
| [future-palette-drag.md](future-palette-drag.md) | Deferred — G18 drag colours between palette slots | Low impact |
| [future-layout-editing.md](future-layout-editing.md) | Deferred — layout-editing overlays / model manipulation | Post-MVP |

Phases A, B-Metal, C, D, E, F, and G are complete. Phase B's
P0 + P1 buckets closed across 2026-04-20/22 (Metal grid, multi-
select marquee, align family, split, timing-mark editing, loop
region, lyric sub-layer rendering + word breakdown, follow-
playhead, trackpad scroll, waveform filter variants, `.xtiming`
I/O, import lyrics + auto-label, cut-copy row/model, multi-
effect clipboard, pointer hover, column resize, tags B34/B35,
randomize/reset B15, presets stub B19, export B49, scrollbars
B94). ~40 P2 polish items remain in
[phase-b-grid-parity.md](phase-b-grid-parity.md). Phase F
landed F-1 through F-6 (Display Elements editor, menu bar /
`.commands`, scene-level split for detachable preview +
inspector, size-class responsive layout, docked layout, detach-
state persistence). Phase G landed G-1/2/3/4 (file coordination,
iCloud ubiquity badges, `.xsq` + `.xsqz` document registration,
clean background shutdown) plus the shared desktop/iPad
`SequencePackage::Pack` refactor and the iCloud-Drive sandbox
round-trip on iPad. Phase H has H-0/1/2/3 shipped (unified
bundle ID, Liquid Glass icon + launch screen, privacy manifest,
network usage strings, export compliance, Xcode Cloud builds);
only H-4 TestFlight external group + H-5 submission metadata
remain — both organizational. Cross-phase odds and ends in
`followups.md`.

Ground rules for sub-plans:

1. Only track **pending** work. Finished items are git history, not plan
   noise.
2. Each bullet should be concrete enough to scope against without
   re-reading the codebase.
3. Keep files focused. If a sub-plan grows past ~600 lines it probably
   needs to be split.
