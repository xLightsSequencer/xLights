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
and both previews are interactive on device. A 2026-04-20 parity
audit caught ~100 missing authoring behaviours vs desktop; across the
2026-04-20 and 2026-04-21 sessions the full P0 bundle and 20+ P1s
landed (multi-select + marquee, every align variant, timing-mark
editing + loop region + tags-less-tags, phrase and word breakdown,
lyric sub-layer rendering, follow-playhead, trackpad scroll, drag-to-
scrub, waveform filters, pointer hover, `.xtiming` I/O, import lyrics,
auto-label, cut/copy row + model, multi-effect clipboard with relative
timing, column-resize, numbered tags B34/B35, FSEQ export B49, visible
scrollbars B94, randomise/reset B15, presets stub B19). **All P1 gaps
closed.** Remaining ~40 items are P2 polish — see
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
| B | Effects grid parity with desktop | **P0 + P1 closed** — all original P0s + 24 P1s landed across 2026-04-20/22. **Only 2 × P2 left after a 2026-04-28 polish sweep**: B77 (MIDI Import Notes) and B79 (AI Speech 2 Lyrics). The sweep landed B25 (ColorManager palette), B40 (audio scrub on ruler drag), B43 (alt-track waveform switch), B55 (Convert to Per Model), B56-promote (Promote Node Effects), B84-per-mark (single-phrase breakdown), B97 (Find / Replace panel). Closed without code: B7 + B23 + B27 + B60 (no desktop counterpart / already covered via colorMask / multi-effect clipboard / F-6 Display Elements). Deferred: B16 (drag-from-palette ghost — pending feedback), B24 (find-source-effects — needs diagnostic panel), B56-convert (data → effects — needs core helper lift). | [`plans/phase-b-grid-parity.md`](plans/phase-b-grid-parity.md) |
| B-Metal | Grid render pipeline migration (CG → Metal) | ✓ complete | — |
| C | Effect settings inspector | ✓ complete — DMX state persistence (G8+) landed 2026-04-28; Moving Head colour + dimmer-intensity + path-clear authoring (G3+) landed 2026-04-29 in the Color / Dimmer / Pathing tabs of the MH inspector. Full waypoint path authoring still desktop-only — tracked in [`plans/followups.md`](plans/followups.md). | — |
| D | Model Preview + preview polish | ✓ complete — layout-editor overlays parked in [`plans/future-layout-editing.md`](plans/future-layout-editing.md) | [`plans/phase-d-preview.md`](plans/phase-d-preview.md) |
| E | Sequence management (open / save / new / settings) | ✓ complete — E-1 through E-6 shipped 2026-04-21; `.fseq` save+load short-circuit landed 2026-04-27; Batch Render tool + recursive picker landed 2026-04-28; Sequence Settings → Timings tab (rename/delete/export per track + bulk import of `.xtiming` / `.lms` / `.pgo`) landed 2026-04-28; Audio Tracks tab (add/remove/rename/replace alt tracks, file-pick routed through MediaRelocation) landed 2026-04-29. Data Layers tab still deferred — tracked in [`plans/followups.md`](plans/followups.md) | — |
| F | Window system + Display Elements | ✓ complete 2026-04-21 — F-1 scene-level split, F-2/F-3 responsive+docked layout, F-4 menu bar / `.commands`, F-5 persistence (session destruction + main-window minimum-size floor + detach-state AppStorage), and F-6 Display Elements editor all landed. Known iPadOS limitation: Stage Manager caches window position outside SwiftUI's reach, so main can relaunch in the last-active scene's corner — self-corrects after one drag-reposition. Detached scene-owned preview state (is3D, camera, layoutGroup) deferred — not worth the refactor given session-destruction policy. | [`plans/phase-f-window-system.md`](plans/phase-f-window-system.md) |
| G | Document / iCloud polish | ✓ complete — G-1 file coordination, G-2 iCloud ubiquity badges, G-3 `.xsq` + `.xsqz` document registration with Files/AirDrop `.onOpenURL` handling, G-4 clean background shutdown. `.xsqz` round-trip: tap in Files → copy into app sandbox via `NSFileCoordinator` (iOS path-based POSIX can't read `~/Library/Mobile Documents` without iCloud entitlements) → extract with `SequencePackage::Extract` → edit → Save repacks via `SequencePackage::Pack` and copies back to the original URL with write-coordination. Fresh-install tap opens straight to the sequencer with no show-folder configuration. Shared desktop+iPad `SequencePackage::Pack()` replaces the legacy wxZipOutputStream packager: walks SequenceMedia + models + view objects, preserves show-relative paths, relocates externals under typed subdirs (Images/Videos/Shaders/Glediators/Objects/Faces), per-object group colocation for mesh + `.mtl` + textures, folder-based collision disambiguation (won't trip PicturesEffect's animation-sequence detection), pre-flight readability check (no orphan stub zip entries), and per-file warnings surfaced to the desktop packaging dialog. | [`plans/phase-g-document.md`](plans/phase-g-document.md) |
| H | App Store readiness | H-0/H-1/H-2/H-3/H-4 ✓ complete — Universal Purchase unified bundle ID (`org.xlights` shared with Mac, iOS platform added to the existing App Store Connect record); Icon Composer `.icon` bundle with Liquid Glass appearance variants on iOS 26 and raster fallbacks for older; launch screen with centered xLights logo + light/dark background; `PrivacyInfo.xcprivacy` declaring only FileTimestamp / UserDefaults / SystemBootTime required-reason APIs; `ITSAppUsesNonExemptEncryption=false`; `NSLocalNetworkUsageDescription` + Bonjour services. Xcode Cloud workflow building and archiving cleanly; `SequencePackage::Pack` landed the comprehensive wx-free packager. H-4 TestFlight external group cleared Beta App Review and went live to external testers 2026-04-28. **H-5 submission metadata / screenshots remaining** (organizational). | [`plans/phase-h-app-store.md`](plans/phase-h-app-store.md) |
| I | Import Effects (iPad) | I-1 (core extraction + desktop refactor) landed 2026-04-27 — `EffectMapper`, `AutoMapper` (with `ImportMappingNode` interface + matchers), `MapHintsIO`, `BufferStyles` all in `src-core/import_export/` (or `src-core/effects/`), desktop `xLightsImportChannelMapDialog::DoAutoMap` is now a 30-line adapter, both desktop + iPad-lib debug builds green. Manual import regression on a vendor sequence outstanding before merge. I-2 iPad UI (`.xsq`/`.xsqz` picker → mapping view → import) is the next big push. | [`plans/phase-i-import-effects.md`](plans/phase-i-import-effects.md) |

**Remaining work.** Phase B P2 polish items (~40), Phase H-5
App Store submission metadata / screenshots, and Phase I
(Import Effects) — the next major feature gap after H.

**Preview scope.** Phase D is preview *viewing and appearance*: camera,
overlays, background, labels, transport, export. Desktop `ModelPreview`
also hosts the layout editor (drag-to-move, resize handles, polyline
editing, align/distribute, property grid) — that behaviour stays
desktop-only and is not in Phase D or anywhere else in the iPad plan.
Per-view model visibility / layout-group management is Phase F.

### Controller output

Pulled forward from "Deferred" 2026-04-28 after external testers
flagged the missing toggle. The plumbing was already in place
(`OutputManager` owned by `iPadRenderContext`, `XLSequenceDocument`
exposes `startOutput` / `stopOutput` / `outputFrame:` / `outputCount`,
`SequencerViewModel.sendOutputFrame()` runs every playback tick,
`shutdownForBackground` / `closeSequence` both halt output cleanly);
the gap was just the user-visible toggle.

Lightbulb button now lives in the `SequencerView` toolbar between
the play controls and render-all (`lightbulb` / `lightbulb.fill`,
yellow tint when on). Tap-while-off routes through
`SequencerViewModel.toggleOutput()`; on failure it returns a
user-facing string that the view promotes to an alert. Two
distinct messages: "no controllers configured" (the common
external-tester case — there's no iPad controller-setup UI, so
they need to set controllers up in desktop xLights and copy the
show folder over) vs. "couldn't reach configured controllers"
(network / multicast issue).

**Caveat — sACN multicast.** iOS gates `239.255.x.x` joins
behind `com.apple.developer.networking.multicast`, which Apple
approves manually. Entitlement request submitted 2026-04-28; the
"couldn't reach" alert text mentions the limitation so testers
know to use ArtNet, DDP, or sACN unicast in the meantime. Add
the entitlement key to `macOS/Assets/xLights-iPad/xLights-iPad.entitlements`
once approval lands.

### Deferred / explicitly out of MVP

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

_(none — the Phase H Universal Purchase question was resolved:
single `org.xlights` record, iOS platform added alongside the
existing macOS app in App Store Connect.)_
