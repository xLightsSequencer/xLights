# xLights iPad — Implementation Plan

This file tracks overall status. Details for active work live under
[`plans/`](plans/README.md). Completed phases live in git history.

---

## Where we are (2026-05-17)

The iPad app shipped to the App Store and exercises the full
desktop rendering / effect / sequence pipeline through the same
`src-core/` it shares with the Mac. Phases A, B-Metal, C, D, E, F,
G, H are complete; phase B P0/P1 closed; Phase I MVP (`.xsq` /
`.xsqz` / `.sup`) shipped; Phase J landed J-0..J-32. What follows
is the remaining work.

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

## Remaining work

- **FPP Connect is feature-complete for iPad scope (2026-05-19).**
  Slices A + B + F + Slice-C auth shipped: discovery, password-
  protected FPP support (Keychain credentials + UIAlertController
  prompt on 401), full per-instance config drawer (Media / Cape /
  Add Proxies / Models / UDP Out / Playlist + Create New Playlist
  prompt), parallel transcode + curl-driven concurrent transfers,
  per-FPP progress gauges, full per-FPP restart + channel-range +
  playlist-finalize dance. Multi-window Tools-menu routing fix
  included. iPad targets FPP / ESPixelStick only (open-source
  firmware); proprietary controllers (Falcon V4/V5 / Genius /
  PowerDMX) are filtered out of discovery. Remaining FPP Connect
  work (manual Add FPP, Re-Discover, sequence-list polish, full
  settings persistence parity, UX-string parity) is polish — no
  concrete user need is blocking it today.
  Slice C (manual Add FPP + auth + re-discover) and Slice F
  (parallel transcode + concurrent curl transfers) remain the
  next P2s after B finishes. Slice D is sequence-list polish;
  G/H are settings persistence parity + UX strings. Slice E
  (proprietary vendor codecs) is P4 / out of iPad scope. The rest
  of the upload stack (Bulk Upload, HinksPix, drag-drop port
  mapping) stays parked at P4. Pixel Test stays P2.
- **Authenticated vendor downloads.** `VendorBrowserSheet` /
  `XLVendorCatalog` / `CachedFileDownloader` use anonymous libcurl
  HTTP — there is no auth path for behind-login catalog content
  today. Open whenever the catalog starts gating models behind
  accounts.
- **Layout Editor Controllers-tab polish.** Drag-reorder + sort
  modes + right-click Activate/Inactivate/Unlink + LED ping. The
  Discover-sheet HTTP-scan-with-auth (O-7) is now P4. See
  [`phase-j-layout-editor.md`](plans/phase-j-layout-editor.md).
- **Phase B-77 — MIDI Import Notes.** Deprioritized; low desktop use
  and no concrete iPad request open.
- **Phase IE-4 — LOR S5 `.loredit` import (discovery landed).** The
  effect-level LOR reader (`LOREdit.{h,cpp}`) was moved into the
  wx-free core (`src-core/import_export/`) so both desktop and iPad
  link the same parser; the desktop `ImportS5` path now uses the
  relocated core class. On iPad, `XLImportSession` gained a
  `loadLOREditSource(atPath:)` branch that parses the `.loredit` with
  the core reader and populates the same available-source list +
  timing tracks the `.xsq` path builds, so the existing mapping tree
  + AutoMapper / MapHints flow is reused; `ImportEffectsView`
  `allowedTypes` now includes `.loredit`. **Remaining:** the
  effect-synthesis apply path — the iPad analogue of desktop
  `MapS5` / `MapS5Effects` / `MapS5ChannelEffects` (which build
  effects from `LOREditEffect::GetxLightsEffect` / `GetSettings` /
  `GetPalette` onto target `EffectLayer*` / `NodeLayer*`, resolving
  node coords via `Model::GetNodeCoords`). Until that lands,
  `-applyImportWithEraseExisting:lock:error:` returns an error for a
  `.loredit` source.
- **Phase I-5 — `.lms`/`.las`.** Distant third format; parked
  until a vendor request lands.
- **3 deferred Phase B items** (B16 drag-from-palette ghost,
  B24 Find Possible Source Effects, B56 Convert-to-Effect) remain
  parked. See [`phase-b-grid-parity.md`](plans/phase-b-grid-parity.md).
- **Cross-phase follow-ups** (Data Layers tab, MH waypoint
  authoring, shader uniform grouping). See
  [`followups.md`](plans/followups.md).

**Catalogue of full post-MVP scope.** The 2026-04-23 gap analysis
(in the sibling working tree at
`xLights/plans/gap-analysis-2026-04-23.md`) inventoried the full
desktop surface and recommended ~12 phases beyond MVP totalling
20–30 person-months. Each major domain is now tracked in a
`plans/future-*.md` file — see [`plans/README.md`](plans/README.md).

## Deferred / explicitly out of MVP

- **JobPool requeue redesign** — desktop-scope refactor to replace
  block-on-other-model-frame with re-enqueue. Tracked separately;
  the iPad workaround is "more threads."
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
  review.

---

## Open questions

_(none open.)_
