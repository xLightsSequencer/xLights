# xLights iPad — Implementation Plan

This file tracks overall status. Details for each phase live under
[`plans/`](plans/README.md) — one focused sub-plan per phase, listing
only the work still to do. Completed phases have no sub-plan.

---

## Current state

### Goal of the work so far

The work so far has been scoped to the **core rendering path**: load a
sequence, render it through the same C++ effect pipeline as desktop
xLights, and display the house preview during playback. That is done
and verified on a physical iPad. Shaders, video effects, and most
effects render correctly.

Everything above the core baseline — effects grid, inspector, output,
preview chrome — has been rebuilt for touch. The effects grid is now
Metal-backed, the inspector is metadata-driven with a four-tab shell,
and both previews are interactive on device. The grid's rebuild got
the rendering substrate and basic single-effect editing right, but a
2026-04-20 parity audit against the desktop caught ~100 missing
behaviours (multi-select, align / split, timing-track editing, loop
region, follow-playhead, waveform variants, row-heading ops); see
[`plans/phase-b-grid-parity.md`](plans/phase-b-grid-parity.md).

### Code layout

iPad code lives at `src-iPad/` at the repo root (peer to `src-ui-wx/`).
It is not under `macOS/`: code in `macOS/` cannot depend on anything
outside `macOS/`, so the iPad UI sits alongside the other UI layer
instead.

```
src-iPad/
  App/            SwiftUI views + view model
  Bridge/         ObjC++ bridges (XLSequenceDocument, iPadRenderContext,
                  XLiPadInit, CoreGraphicsTextDrawingContext,
                  XLValueCurve)
  Metal/          xlStandaloneMetalCanvas, iPadModelPreview,
                  XLMetalBridge, iPadGridPreview, XLGridMetalBridge
  Metadata/       EffectMetadata.swift (JSON model for effectmetadata/*.json)
```

Shared core that iPad consumes:

- `src-core/` — wx-free C++ core (renderers, models, outputs, render
  engine, effect manager, sequence file/elements, audio manager).
- `macOS/src-apple-core/` — Apple shared code (Metal device manager,
  external hooks, Apple utilities in Swift and ObjC++).

### Xcode targets

| Target | Purpose | Multi-platform |
|---|---|---|
| `xLights-core` | wx-free core (`src-core/`) | macOS + iOS |
| `xLights-Apple-core` | Apple shared code (`macOS/src-apple-core/`) | macOS + iOS |
| `xLights-macOSLib-UI` | macOS-only UI (`macOS/src-mac-ui/`) | macOS only |
| `xLights` | Desktop app | macOS only |
| `EffectComputeFunctions` | Metal shaders | macOS + iOS |
| `UIMetalShaders` | UI Metal shaders | macOS only |
| `ISPCEffectComputeFunctions` | SIMD kernels | macOS + iOS |
| `xLights-iPadLib` | iPad bridge code (`src-iPad/`) | iOS |
| `xLights-iPad` | SwiftUI iPad app | iOS 26+ |

iOS dependencies live at `/opt/xLights-macOS-dependencies/lib-ios/`:
`libcurl.a`, `libEGL.xcframework`, `libGLESv2.xcframework` (ANGLE),
`libliquidfun.a`, `liblua.a`, `libxlsxwriter.a`, `libzstd.a`. Debug
variants in `libdbg-ios/`.

---

## Phase status

| Phase | Title | Status | Sub-plan |
|---|---|---|---|
| A | Core-path hardening | ✓ complete | — (one small follow-up in [`plans/followups.md`](plans/followups.md)) |
| B | Effects grid parity with desktop | Partial — gap audit vs desktop done 2026-04-20; Metal-backed grid renders and basic edit works, but ~100 desktop behaviours remain (multi-select, align/split, timing-track editing, loop region, follow-playhead, row-heading ops, waveform variants) | [`plans/phase-b-grid-parity.md`](plans/phase-b-grid-parity.md) |
| B-Metal | Grid render pipeline migration (CG → Metal) | ✓ complete | — |
| C | Effect settings inspector | In progress | [`plans/phase-c-inspector.md`](plans/phase-c-inspector.md) |
| D | Model Preview + preview polish | ✓ complete — layout-editor overlays parked in [`plans/future-layout-editing.md`](plans/future-layout-editing.md) | [`plans/phase-d-preview.md`](plans/phase-d-preview.md) |
| E | Sequence management (open / save / new / settings) | Not started — currently read-only; blocks any real user testing | [`plans/phase-e-sequence-management.md`](plans/phase-e-sequence-management.md) |
| F | Window system + Display Elements | Not started | [`plans/phase-f-window-system.md`](plans/phase-f-window-system.md) |
| G | Document / iCloud polish | Not started | [`plans/phase-g-document.md`](plans/phase-g-document.md) |
| H | App Store readiness | Not started | [`plans/phase-h-app-store.md`](plans/phase-h-app-store.md) |

**Parallelism.** Phases B / C / E are independent and can run in
parallel. B works against the Metal grid + `SequencerViewModel` + row
headers; C works against `EffectMetadata.swift` + the inspector view;
E works against `XLSequenceDocument` + `SequencerViewModel` save/new
paths. Phase D is complete. Phase F depends on the others — it
composes the finished pieces (previews, inspector, grid) plus the
File-menu commands (E) into the final window / menu-bar layout. G
and H are sequential at the end.

**Preview scope.** Phase D is preview *viewing and appearance*: camera,
overlays, background, labels, transport, export. Desktop `ModelPreview`
also hosts the layout editor (drag-to-move, resize handles, polyline
editing, align/distribute, property grid) — that behaviour stays
desktop-only and is not in Phase D or anywhere else in the iPad plan.
Per-view model visibility / layout-group management is Phase F.

### Deferred / explicitly out of MVP

- **Controller output** — infrastructure is in the tree (output
  manager, per-frame send, sACN / ArtNet / DDP / OPC) but not on the
  MVP critical path. Revisit after App Store submission.
- **JobPool requeue redesign** — desktop-scope refactor to replace
  block-on-other-model-frame with re-enqueue. Tracked separately; the
  iPad workaround is "more threads."
- **Layout editor, controller setup** — stays desktop-only.

---

## Risks

- **JobPool deadlock on complex sequences** — mitigated by raising the
  thread count; the underlying "workers block on peers" pattern is
  still there.
- **Memory on mid-tier iPads** — Phase A memory-pressure handling is
  in place but under-tested.
- **AVFoundation codec coverage** — video effects with unusual codecs
  fail; no FFmpeg fallback on iPad.

---

## Open questions

1. For Phase H (App Store), existing Apple Developer team? And is the
   iPad app a separate App Store record or shipped as a universal app
   alongside macOS? (Impacts bundle id and entitlements.)
