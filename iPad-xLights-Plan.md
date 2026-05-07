# xLights iPad — Implementation Plan

This file tracks overall status. Details for each phase live under
[`plans/`](plans/README.md) — one focused sub-plan per phase, listing
only the work still to do. Completed phases keep a short residual
file documenting deferrals + caveats; landed implementation prose
lives in git history.

---

## Where we are (2026-05-02)

The iPad app builds, ships through Xcode Cloud, runs on TestFlight
external testers, and exercises the full desktop rendering / effect /
sequence pipeline through the same `src-core/` it shares with the
Mac. Phases A, B-Metal, C, D, E, F, G are complete. Phase B P0+P1
parity work is done (1 named P2 — B77 MIDI import — plus 3 deferred
remain; B79 AI Speech 2 Lyrics shipped via
`XLAIServices.generateLyricTrack`, B91 LRCLIB synced-lyrics import
shipped 2026-05-02 alongside the desktop release). Phase H is one
organizational push (H-5) from submission. Phase I shipped
`.xsq`/`.xsqz` 2026-04-29 and SuperStar `.sup` 2026-05-02; the
`.lms`/`.las` format slot is the remaining feature gap. Multicast
sACN was approved 2026-05-01 and is wired into the entitlements.

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
  engine, effect manager, sequence file/elements, audio manager,
  import_export).
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
| `UIMetalShaders` | UI Metal shaders | macOS + iOS |
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
| A | Core-path hardening | ✓ complete | — |
| B | Effects grid parity | ✓ P0 + P1 closed; 1 P2 (B77 MIDI) + 3 deferred remain | [`phase-b-grid-parity.md`](plans/phase-b-grid-parity.md) |
| B-Metal | Grid render pipeline (CG → Metal) | ✓ complete | — |
| C | Effect settings inspector | ✓ complete | 3 small follow-ups in [`followups.md`](plans/followups.md) |
| D | Model Preview + preview polish | ✓ complete | [`phase-d-preview.md`](plans/phase-d-preview.md) (residual) |
| E | Sequence management | ✓ complete | 1 follow-up (Data Layers tab) in [`followups.md`](plans/followups.md) |
| F | Window system + Display Elements | ✓ complete 2026-04-21 | [`phase-f-window-system.md`](plans/phase-f-window-system.md) (residual) |
| G | Document / iCloud polish | ✓ complete 2026-04-22 | [`phase-g-document.md`](plans/phase-g-document.md) (residual) |
| H | App Store readiness | H-0..H-4 ✓; **H-5 metadata + screenshots remaining** | [`phase-h-app-store.md`](plans/phase-h-app-store.md) |
| I | Import Effects | I-1 + I-2 (`.xsq`/`.xsqz`) ✓ 2026-04-29; I-4 (`.sup`) ✓ 2026-05-02; **I-3 vendor regression, I-5 `.lms`/`.las` remaining** | [`phase-i-import-effects.md`](plans/phase-i-import-effects.md) |

---

## What's left for MVP

In priority order:

1. **Phase H-5 — App Store submission metadata.** Organizational, not
   engineering. Screenshots (12.9"/13" + 11", landscape + portrait,
   3-5 frames per orientation), description, keywords, support URL,
   privacy policy URL, age rating, category, copyright, "What's
   New". Pin a TestFlight build, submit. Detailed checklist in
   [`phase-h-app-store.md`](plans/phase-h-app-store.md).

2. **Phase I-3 — Manual vendor-sequence regression of `.xsq`/`.xsqz`
   import.** Load a real Holiday Coro / Wally Wally World pack with
   user-supplied `.xmaphint` files, run Auto Map, count matched
   models against desktop baseline. Tune Auto Map UX in response
   (scroll-to-first-unmapped, "X of Y mapped" counter). Bug-fix
   anything the regression turns up.

3. **Phase B P2 — B77 (MIDI Import Notes).** Needs a new bridge
   surface (AVFoundation `MIDIFile` parser → timing marks). P2
   feature — desktop has it, iPad doesn't, but it's not on the
   critical authoring path.

The 3 deferred Phase B items (B16 drag-from-palette ghost, B24 Find
Possible Source Effects, B56 Convert-to-Effect) are explicitly
parked — substantial new work, not blocking submission.

The follow-ups in [`followups.md`](plans/followups.md) (Data Layers
tab, MH waypoint authoring, shader uniform grouping) are
quality-of-life and unblock independently.

## Could pull into MVP during testing

If H-5 prep + I-3 testing leaves spare cycles, these are the
remaining feature-additions worth a tester sprint.

**TestFlight loop quality (shipped pre-MVP).** Log export +
crash telemetry / MetricKit + auto-upload to `crashUpload/index.php`,
About / Help menu, Check Sequence runner (shared core via
`src-core/diagnostics/SequenceChecker`), incompatible-video
warning, ObtainAccessToURL re-prompt sheet, Recent Show Folders
list, SuperStar `.sup` import — all landed. Implementation prose
in git history.

`XLMetricKit` filters MetricKit `MXDiskWriteExceptionDiagnostic`
payloads on iPad — the `MAP_SHARED` SequenceData backing tmpfile
guarantees the kernel writes back every dirtied sequence page, so
iOS's writesCaused metric is structural noise, not signal. Crash /
hang / CPU / app-launch diagnostics still flow through.

**Remaining feature additions:**

- **B77 MIDI import.** Concrete user request; iOS-native via
  AVFoundation `MIDIFile`. New bridge surface needed.
- **Live-output controller list (read-only)** (gap-analysis O-3,
  P1, M). Even just *seeing* what's configured (without setup UI)
  is a tester clarity win.

Items **not** worth pulling in even with spare cycles:

- I-5 (`.lms`/`.las`) — distant third format, low vendor traffic
  today. Reconsider once I-4 has TestFlight mileage.
- MH full waypoint authoring — Sketch-style drag UI is a real
  design exercise, not a quick port.
- Shader uniform grouping (G2-c) — only matters for shader packs
  with 20+ uniforms, which are vanishingly rare.
- Anything from [`plans/future-layout-editing.md`](plans/future-layout-editing.md),
  [`plans/future-custom-models.md`](plans/future-custom-models.md),
  or [`plans/future-controllers-tab.md`](plans/future-controllers-tab.md) —
  all multi-month efforts.

**Catalogue of full post-MVP scope.** The 2026-04-23 gap analysis
(in the sibling working tree at
`xLights/plans/gap-analysis-2026-04-23.md`) inventoried the full
desktop surface and recommended ~12 phases beyond MVP totalling
20–30 person-months. Each major domain is now tracked in a
`plans/future-*.md` file — see [`plans/README.md`](plans/README.md).

## Preview scope

Phase D is preview *viewing and appearance*: camera, overlays,
background, labels, transport, export. Desktop `ModelPreview` also
hosts the layout editor (drag-to-move, resize handles, polyline
editing, align/distribute, property grid) — that behaviour stays
desktop-only and is parked in
[`future-layout-editing.md`](plans/future-layout-editing.md).
Per-view model visibility / layout-group management is Phase F-6
(Display Elements editor).

## Controller output

Lightbulb toggle in the `SequencerView` toolbar between play
controls and render-all (`lightbulb` / `lightbulb.fill`, yellow
tint when on). Tap-while-off routes through
`SequencerViewModel.toggleOutput()`; on failure it returns one of
two user-facing alerts: "no controllers configured" (the common
external-tester case — there's no iPad controller-setup UI, so
testers must configure controllers in desktop xLights and copy the
show folder over) or "couldn't reach configured controllers"
(network / multicast issue).

**sACN multicast.** Apple-issued
`com.apple.developer.networking.multicast` entitlement assigned
to the team account 2026-05-01; key added to
`macOS/Assets/xLights-iPad/xLights-iPad.entitlements`. iPads on
the same network as the controllers can now join `239.255.x.x`
for sACN multicast output alongside ArtNet, DDP, and sACN unicast.

## Deferred / explicitly out of MVP

- **JobPool requeue redesign** — desktop-scope refactor to replace
  block-on-other-model-frame with re-enqueue. Tracked separately;
  the iPad workaround is "more threads."
- **Layout editor, controller setup** — stays desktop-only.
- **Disk-persistent effect presets** — in-session ship is enough
  for now; on-disk store + preset-tree UI parked in
  [`future-effect-presets.md`](plans/future-effect-presets.md).

---

## Risks

- **JobPool deadlock on complex sequences** — mitigated by raising
  the thread count; the underlying "workers block on peers"
  pattern is still there.
- **Memory on mid-tier iPads** — Phase A memory-pressure handling
  is in place but under-tested. External tester reports will be
  the first stress signal.
- **AVFoundation codec coverage** — video effects with unusual
  codecs fail; no FFmpeg fallback on iPad.
- **Auto Map false-positives on similar model names** — desktop
  users live with this; iPad testers have a smaller screen for
  review. UX mitigation (scroll-to-first-unmapped) tracked in I-3.

---

## Open questions

_(none open.)_
