# xLights iPad — Status & Plan

The iPad app has **shipped to the App Store**. It is **not a port** — it's a
second SwiftUI UI over the same wx-free `src-core/` the desktop uses (render
engine, effect manager, sequence file / elements, audio manager,
import/export). Day-to-day work is now **desktop parity + ongoing app
updates**, not MVP build-out. The MVP-era phase plans have been retired (git
history holds the landed prose).

## Parity plans live in [`plans/ipad-parity/`](plans/ipad-parity/)

The detailed, per-area plans are there:

- **[`00-overview.md`](plans/ipad-parity/00-overview.md)** — the cross-theme
  map, P1/P2/P3 priorities, sequencing, and rolled-up status.
- **`01`–`13`** — one plan per functional theme, each with a "✅ landed"
  status callout at the top that's kept current as work ships:
  `01` sequencer editing/grid · `02` timing & audio · `03` effect panels ·
  `04` color & value curves · `05` layout / models · `06` submodels / DMX /
  preview · `07` controllers & upload · `08` import / export · `09`
  file-lifecycle / render / tools · `10` presets / jukebox / view · `11`
  preferences · `12` AI & automation · `13` help / diagnostics / tools.
- **[`99-out-of-scope.md`](plans/ipad-parity/99-out-of-scope.md)** — features
  deliberately not on iPad, each with a reason (VAMP host, Python/Lua
  scripting, FFmpeg, SpaceMouse, custom KeyBindings, AUI perspectives, …).
- `_raw-gap-analysis.md` / `_reconciliation.md` — the source gap analysis and
  the reconciliation against the retired MVP plans (reference only).

**Workflow:** update the relevant theme doc's status callout (pending → ✅) as
work lands; keep the "Recently landed" log below as the running iPad changelog.
Per `CLAUDE.md`, iPad-only changes are recorded here / in the theme docs, not
in `README.txt` (which is the desktop release notes).

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

## Recently landed (running log)

All entries below are **build-verified** (`xLights-iPadLib` + the `xLights-iPad`
app); on-device verification is the standing follow-up. Detail lives in the
matching `plans/ipad-parity/` theme doc.

- **Render cache now defaults OFF, and is user-selectable (RC-1).** The shared
  `RenderCache` was running on iPad at its raw constructor default — `_enabled`
  is a `std::string`, the ctor does `_enabled = true` (a 1-char `'\x01'`), and
  `IsEnabled()` is `!= "Disabled"` — so it behaved as full **"Enabled"** (caches
  *every* cache-supporting effect), more aggressive than the desktop's
  **"Locked Only"** default, and with no way for the user to change it. The cache
  trades extra memory + on-disk frames for re-render speed, and both are scarce on
  iPad. Now `iPadRenderContext::ReadRenderCacheMode()` reads
  `@AppStorage("render.cacheMode")` via CFPreferences and feeds
  `RenderCache::Enable("Disabled" | "Locked Only" | "Enabled")` — applied in the
  ctor and re-applied on every `EnsureRenderEngine()` so the picker is live (no
  restart). **Default "Disabled."** 3-way picker in **Folder Config → Rendering**
  (next to Low-Definition Render). Disk cap unchanged (`SetMaximumSizeMB(50)`).
  *Root cause:* the desktop sets the mode from config at startup
  (`xLightsMain.cpp` reads `xLightsEnableRenderCache`, default "Locked Only");
  the iPad never called `Enable()`, so the unintended ctor default leaked through.
  *Follow-up:* PREF-9 should relocate the picker into the unified Preferences shell
  and optionally expose the size cap; watch re-render latency on big sequences with
  the cache off ("Locked Only" is the middle ground). Plan docs 00/09/11 updated.
- **Full-definition render is now the default.** `iPadRenderContext::IsLowDefinitionRender()`
  was hard-coded `true`, forcing every iPad render into reduced-resolution
  effect buffers — pixelated output on large/high-res props and FSEQ that
  didn't match desktop fidelity. It now reads an opt-in app preference
  (`render.lowDefinition`, CFPreferences key written by an `@AppStorage` toggle
  in Folder Config → **Rendering**), **default OFF = full-definition**, matching
  the desktop. Low-def stays available as a deliberate memory-relief escape
  hatch for very large shows on 4 GB devices. *Root cause:* the constant was a
  blanket 4 GB-device safety default that silently degraded output on all
  devices; *follow-up:* the same pref should surface in the unified Preferences
  shell (PREF-1) when it lands, and large-show OOM under full-def is now the
  memory-pressure case to watch. Plan docs 05/09/11/12/99 updated; the frozen
  `_raw-gap-analysis.md` / `_reconciliation.md` snapshots still describe the old
  hard-coded behavior by design (audit trail).
- **Import / Export theme (08).** Fixed the iPad-app link by extracting the
  timing importers to wx-free core (new `src-core/utils/ZipUtils`, relocated
  `Vixen3`, new `src-core/import_export/TimingImport.cpp`); XLights/Vixen3
  timing import is two-step (desktop `wxMultiChoiceDialog` / iPad multi-select
  sheet) — **IE-1**. Plus **IE-4** LOR S5 `.loredit` effect import (`MapS5*` →
  core `EffectMapper`), **IE-25** Vixen 3 `.tim` effect import (`MapVixen3*` →
  core), **IE-7** import dispatch + FPS/missing-media warnings (`.piz`), and
  **IE-15** Export Models XLSX (`-lxlsxwriter` linked into the app via the
  `macOS` submodule).
- **Quick-wins batch (P1).** COL-2 (value-curve bulk edit), AUTO-3 (Speech to
  Lyrics menu), LAY-11 (Add to Group), AI-1 (AI image style presets), CTL-5
  (FPP Connect Add-by-IP + Re-Discover).
- **Easy-wins batch (code-verified sweep).** FX-5 / FX-5b (checkbox / file /
  font bulk-edit), SEQ-7 (LOR `.las`), SEQ-19 (Select All Marks), COL-3
  (palette reverse / shift), SEQ-15 (Show All Effects), MED-5 (remove embedded
  media), SEQ-18 (partial — Edit-Timing duration readout).
- **FPP Connect** is feature-complete for iPad scope (discovery + Keychain
  auth + full per-instance config + parallel transcode/upload + Add-by-IP /
  Re-Discover). iPad targets FPP / ESPixelStick only (open-source firmware);
  proprietary controllers are filtered out of discovery.

---

## Known deferred / blocked (tracked in the theme docs)

- **SEQ-18 (rest)** — editable Duration field + frame-stepped Steppers need an
  `.alert`→`.sheet` conversion (SwiftUI alerts can't host Steppers). Medium.
- **LAY-9** (Make Start Channels Valid) — needs a per-model action surface on
  the read-only Models tab + ~4 validation bridge methods. Medium.
- **LIFE-1** (Save-As show-folder guard) — `.fileExporter` re-prompt loop +
  nil-show-folder fallback. Medium.
- **CTL-4** (FPP-proxy validation pre-upload) — **blocked**: iPad has no proxy
  upload or proxy-IP source yet (prereq: a proxy-upload slice + a
  proxy-listing bridge method).
- **Authenticated vendor downloads**, **Layout-Editor Controllers-tab polish**
  (drag-reorder/sort, LED ping), and the legacy `.lms`/`.las` *full-sequence*
  effect import are parked in themes 05/07/08 until there's a concrete need.

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
