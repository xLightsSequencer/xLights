# xLights iPad — Status & Plan

The iPad app has **shipped to the App Store**. It is **not a port** — it's a
second SwiftUI UI over the same wx-free `src-core/` the desktop uses (render
engine, effect manager, sequence file / elements, audio manager,
import/export). Day-to-day work is now **desktop parity + ongoing app
updates**, not MVP build-out. The MVP-era phase plans have been retired (git
history holds the landed prose).

## Parity plans live in [`plans/ipad-parity/`](plans/ipad-parity/)

A code-level parity audit of the desktop (`src-ui-wx/`) and iPad (`src-iPad/`)
apps over the shared `src-core/`. Every status is grounded in source, with
`file:line` evidence in the theme docs. Start at
[`README.md`](plans/ipad-parity/README.md).

- **[`00-overview.md`](plans/ipad-parity/00-overview.md)** — the cross-theme
  map: headline numbers + **% parity** per theme, the full **P1/P2 roadmap**,
  **reverse-parity** (iPad→desktop) candidates, the **restricted**
  (closed-firmware/IAP) and **infeasible** lists, and recommended sequencing.
- **`01`–`14`** — one plan per functional theme, each a full parity scorecard
  (Feature · Desktop · iPad · Gap · Priority · Ease · Feasibility · Notes):
  `01` file lifecycle · `02` sequencer grid & editing · `03` timing & audio ·
  `04` effects & panels · `05` color & value/color curves · `06` layout /
  models / preview / 3D / submodels / DMX · `07` setup / controllers / upload ·
  `08` import / export · `09` render & playback · `10` presets / jukebox /
  display elements / views · `11` preferences & keyboard shortcuts · `12` AI /
  automation / scripting · `13` tools / diagnostics / help · `14` reverse
  parity (iPad features not in desktop).

Each theme doc carries its own **Infeasible / restricted** section for features
deliberately not on iPad (VAMP host, Python/Lua scripting, FFmpeg, SpaceMouse,
custom KeyBindings, AUI perspectives, closed-firmware uploads, …); these roll up
into the Infeasible / Restricted lists in `00-overview.md`.

**Workflow:** as work lands, flip the matching feature's status in the relevant
theme doc's scorecard (→ ✅ / 🟡). Git history is the changelog — there's no
running landed-log to maintain. Per `CLAUDE.md`, iPad-only changes are recorded
in the theme docs, not in `README.txt` (which is the desktop release notes).

---

## Code layout

iPad code lives at `src-iPad/` at the repo root (peer to `src-ui-wx/`), not
under `macOS/` (code in `macOS/` can't depend on anything outside `macOS/`).

```
src-iPad/
  App/            SwiftUI views + @Observable view model
  Bridge/         ObjC++ bridges (XLSequenceDocument, iPadRenderContext,
                  XLImportSession, XLAIImageSession, XLValueCurve, …)
  Metal/          xlStandaloneMetalCanvas, iPadModelPreview, XLGridMetalBridge
  Metadata/       EffectMetadata.swift (JSON model for effectmetadata/*.json)
```

Shared core the iPad consumes: `src-core/` (wx-free C++ core) and
`macOS/src-apple-core/` (Apple shared code — Metal device manager, external
hooks, Apple utilities).

### Xcode targets

| Target | Purpose | Platforms |
|---|---|---|
| `xLights-core` | wx-free core (`src-core/`) | macOS + iOS |
| `xLights-Apple-core` | Apple shared code (`macOS/src-apple-core/`) | macOS + iOS |
| `xLights-macOSLib-UI` | macOS-only UI (`macOS/src-mac-ui/`) | macOS |
| `xLights` | Desktop app | macOS |
| `EffectComputeFunctions` / `UIMetalShaders` / `ISPCEffectComputeFunctions` | Metal / SIMD kernels | macOS + iOS |
| `xLights-iPadLib` | iPad bridge + core static lib (`src-iPad/`) | iOS |
| `xLights-iPad` | SwiftUI iPad app | iOS 26+ |

iOS deps at `/opt/xLights-macOS-dependencies/lib-ios/`: `libcurl.a`,
`libEGL`/`libGLESv2` xcframeworks (ANGLE), `libliquidfun.a`, `liblua.a`,
`libxlsxwriter.a`, `libzstd.a` (Debug variants in `libdbg-ios/`). Build the
`xLights-iPadLib` scheme for fast iterative checks; build the `xLights-iPad`
scheme to catch link errors and validate the SwiftUI app.

---

## Known deferred / blocked (tracked in the theme docs)

- **Editable Duration field** + frame-stepped Steppers need an
  `.alert`→`.sheet` conversion (SwiftUI alerts can't host Steppers). Medium.
- **Make Start Channels Valid** — needs a per-model action surface on
  the read-only Models tab + ~4 validation bridge methods. Medium.
- **Save-As show-folder guard** — `.fileExporter` re-prompt loop +
  nil-show-folder fallback. Medium.
- **FPP-proxy validation pre-upload** — **blocked**: iPad has no proxy
  upload or proxy-IP source yet (prereq: a proxy-upload slice + a
  proxy-listing bridge method).
- **Authenticated vendor downloads**, **Layout-Editor Controllers-tab polish**
  (drag-reorder/sort, LED ping), and the legacy `.lms`/`.las` *full-sequence*
  effect import are parked in themes 06/07/08 until there's a concrete need.

---

## Risks

- **JobPool deadlock on complex sequences** — mitigated by raising the thread
  count; the underlying "workers block on peers" pattern remains (a
  desktop-scope requeue redesign is the real fix).
- **Memory on mid-tier iPads** — memory-pressure handling is in place but
  under-tested; on-device/tester reports are the first stress signal.
- **AVFoundation codec coverage** — video effects with unusual codecs fail; no
  FFmpeg fallback on iPad.
- **Auto-Map false positives** on similar model names — smaller iPad screen
  makes review harder.
