# xLights iPad — Implementation Plan

This file tracks overall status. Details for each phase live under
[`plans/`](plans/README.md) — one focused sub-plan per phase, listing
only the work still to do. Completed phases keep a short residual
file documenting deferrals + caveats; landed implementation prose
lives in git history.

---

## Where we are (2026-05-17)

The iPad app **shipped to the App Store** and exercises the full
desktop rendering / effect / sequence pipeline through the same
`src-core/` it shares with the Mac. Phases A, B-Metal, C, D, E, F,
G, H are complete. Phase B P0+P1 parity work is done (1 named P2 —
B77 MIDI import, explicitly deprioritized as low desktop use —
plus 3 deferred remain). Phase I shipped `.xsq`/`.xsqz` 2026-04-29,
SuperStar `.sup` 2026-05-02, and the vendor-sequence regression
(I-3) closed; the `.lms`/`.las` format slot is the remaining
parked feature. Phase J landed J-0 through J-32 over 2026-05-07
through 2026-05-16, including per-type properties for 26 model
types, model-group CRUD + membership editing, the Controllers tab
+ Visualize wiring view (which subsumed the "live-output controller
list" gap), the custom-model visual editor, and the DMX deep
authoring path. Multicast sACN was approved 2026-05-01 and is
wired into the entitlements.

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
| B | Effects grid parity | ✓ P0 + P1 closed; B77 MIDI deprioritized + 3 deferred remain | [`phase-b-grid-parity.md`](plans/phase-b-grid-parity.md) |
| B-Metal | Grid render pipeline (CG → Metal) | ✓ complete | — |
| C | Effect settings inspector | ✓ complete | 3 small follow-ups in [`followups.md`](plans/followups.md) |
| D | Model Preview + preview polish | ✓ complete | [`phase-d-preview.md`](plans/phase-d-preview.md) (residual) |
| E | Sequence management | ✓ complete | 1 follow-up (Data Layers tab) in [`followups.md`](plans/followups.md) |
| F | Window system + Display Elements | ✓ complete 2026-04-21 | [`phase-f-window-system.md`](plans/phase-f-window-system.md) (residual) |
| G | Document / iCloud polish | ✓ complete 2026-04-22 | [`phase-g-document.md`](plans/phase-g-document.md) (residual) |
| H | App Store readiness | ✓ complete — shipped to App Store | [`phase-h-app-store.md`](plans/phase-h-app-store.md) (residual) |
| I | Import Effects | ✓ MVP complete — `.xsq`/`.xsqz` ✓ 2026-04-29, `.sup` ✓ 2026-05-02, I-3 vendor regression ✓; I-5 `.lms`/`.las` parked | [`phase-i-import-effects.md`](plans/phase-i-import-effects.md) (residual) |
| J | Layout Editor | ✓ J-0..J-32 complete (per-type props for 26 model types, group CRUD, Controllers tab + Visualize, custom-model editor, DMX deep authoring); 2 small import gaps remain | [`phase-j-layout-editor.md`](plans/phase-j-layout-editor.md) |

---

## MVP shipped

The App Store submission landed and the headline phases (A–I plus
the bulk of J) are complete. What was the MVP-blocking list:

- **Phase H — App Store submission.** Done; app is live.
- **Phase I-3 — Vendor-sequence regression.** Done.
- **Phase J — Layout editor.** J-0 → J-32 landed across
  2026-05-07..16. The descriptor pipeline owns hover/click/drag/draw
  on both clients; 26 model types have full per-type property
  pages; model groups have CRUD + drag-reorder + an expandable
  AddMemberSheet; the Controllers tab and Visualize wiring view
  shipped (which closed the "live-output controller list" gap);
  the custom-model visual editor, DMX deep authoring, and Faces /
  States / Dimming Curve / SubModels editors all shipped.

## Remaining small work

- **Authenticated vendor downloads.** `VendorBrowserSheet` /
  `XLVendorCatalog` / `CachedFileDownloader` use anonymous libcurl
  HTTP — there is no auth path for behind-login catalog content
  today. Open whenever the catalog starts gating models behind
  accounts.
- **Phase B-77 — MIDI Import Notes.** Deprioritized; the
  feature sees little use on desktop and no concrete iPad request
  is open.
- **Phase I-5 — `.lms`/`.las`.** Distant third format; parked
  until a vendor request lands.

The follow-ups in [`followups.md`](plans/followups.md) (Data Layers
tab, MH waypoint authoring, shader uniform grouping, animated GIF →
Pictures migration, Display Elements filter, add-alias on
missing-model prompt) are quality-of-life and unblock independently.

The 3 deferred Phase B items (B16 drag-from-palette ghost, B24 Find
Possible Source Effects, B56 Convert-to-Effect) remain parked.

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
