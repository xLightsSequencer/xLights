# iPad Parity Plan 13 — Help, Diagnostics & Un-homed Tools

_Scope: gap-analysis §2.16 (Help / About / diagnostics / support) plus the cluster of desktop **Tools** menu utilities that have no functional-theme home — ShowFolderSearchDialog, PathGenerationDialog (Generate 2D Path), Prepare Audio (Reaper `.rpp`/`.xaudio`), Tip-of-the-Day, CharMapDialog, ResizeImageDialog, EmailDialog (crash-report prompt), Help-menu link parity, and Package-Show-for-Support scope. The high-traffic Help/diagnostics surface (About, Help URLs, log export, crash telemetry, in-app log viewer, log packager, launch screen) **already shipped during the TestFlight sweep** and is recorded here as done so it has a home in the plan set._

> **Status (2026-06-01) — landed & build-verified (`xLights-iPadLib` Debug):**
> - ✅ **TOOL-8 (Help-menu link parity)** — added the two missing desktop support links to the Help menu (`XLightsCommands.swift`): **Zoom Room Help** and **Donate** (same URLs as desktop `Help_Zoom`/`Help_Donate`), via `XLOpenURL`.
> - ✅ **TOOLS-1b (Purge Render Cache)** — added `iPadRenderContext::PurgeRenderCache()` (calls `RenderCache::Purge(&seqElements, true)`, mirroring desktop), a bridge `purgeRenderCache`, a view-model passthrough, and a **Tools ▸ Purge Render Cache** menu item (gated on a loaded sequence).
> - *Still open here:* TOOL-1 (ShowFolderSearch→Files picker), TOOL-2 (Generate 2D Path), TOOL-3 (Prepare Audio), TOOL-4 (Tip-of-the-Day / "What's New"), TOOL-5/6 (CharMap/ResizeImage substitutes), TOOL-7 (EmailDialog), TOOL-9 (Package-Show-for-Support scope).

## Current parity

The Help / diagnostics surface is the most-complete area in the entire report — it was prioritised for TestFlight and is effectively shipped:

- **About** — `AboutSheet` (version / build / dependency credits), opened from the SwiftUI `.help` `CommandGroup` (`XLightsCommands.swift:479`).
- **Help-menu URLs** — `XLightsCommands.swift:478-514` populates Manual, Tutorial Videos, Release Notes, Forum, Facebook Group, Issue Tracker, Website, each via `XLOpenURL` → `UIApplication.shared.open` (mirrors desktop `wxLaunchDefaultBrowser`).
- **View Log** — `LogViewerSheet` (in-app log viewer; level filter present, free-text search may be absent — minor follow-up).
- **Package Logs** — `XLLogPackager` zips rotated spdlog files + diagnostics → iOS share sheet (gap-analysis H-3 / T-2).
- **Crash telemetry** — `XLDiagnosticUploader` (gap-analysis H-4). Does **not** yet collect a user email (that hook lives in PREF-14 / TOOL-7).
- **Launch screen / splash** — `LaunchScreen` covers desktop `SplashDialog` (H-9).

Per gap-analysis §2.16 the H-family is 12 items: H-1/H-2/H-3/H-4/H-6/H-7 shipped, H-8/H-9 platform-handled, and only **H-5 (Tip-of-the-Day), H-10 (EmailDialog), H-11 (RestoreBackupDialog), H-12 (ShowFolderSearchDialog)** outstanding. H-11 (Restore Backup) is owned by `09-file-lifecycle-render-tools.md` (`BKP-2`) and not duplicated here. The Tools-menu utilities below were verified present in `src-ui-wx/xLightsMain.cpp` and are absent from every other new doc and the raw dataset — this doc is their home.

**Platform-handled (no work):**

- **Check-for-Updates / `UpdaterDialog`** (H-8) — the App Store handles iPad updates.
- **`SplashDialog`** (H-9) — `LaunchScreen` shipped.

## Approach

This theme is almost entirely **small SwiftUI sheets + thin bridge calls over already-linked `src-core`**, with two genuine re-conceives where a desktop dialog has no sandbox-compatible analogue:

1. **Re-conceive, don't port, the sandbox-hostile utilities.** ShowFolderSearchDialog's free filesystem scan is meaningless under the iOS sandbox — it becomes a `.fileImporter` folder picker (TOOL-1). CharMapDialog and ResizeImageDialog are likely fully substituted by the iOS keyboard and Photos editing respectively (TOOL-5/TOOL-6); we record the substitution decision rather than reimplement curated pickers.
2. **Reuse the inline-custom-row canvas idiom for Generate 2D Path.** PathGenerationDialog is a mouse-draw + smoothing-slider path editor emitting 2D node-path data; on iPad it maps cleanly onto the touch-draw canvas already built for the Sketch / Morph effects (cross-ref FX-9 / FX-12 in `04-effects.md`).
3. **Product calls over literal ports.** Tip-of-the-Day's 50+ HTML/PNG carousel duplicates Apple's own first-run idiom; recommend an Apple-style "What's New" sheet against the actual iPad feature set (TOOL-4). EmailDialog's one-time crash-report email prompt is a product decision (do we want a prompt, or is the buried PREF-14 stored-email field enough?) (TOOL-7).
4. **Help-menu link parity is a two-line add** — desktop has Donate and Zoom Room Help links the iPad menu lacks (TOOL-8).

The two dev-only diagnostics (Crash xLights, Log Render State) and the Tip-of-the-Day *preferences* are out of scope and tracked in `99-out-of-scope.md`; the Tip-of-the-Day *feature itself* is listed both as a P2 product call here (TOOL-4) and as a §7 out-of-scope row over there (with the "build What's New instead" rationale), kept consistent.

## Work breakdown

### Phase 1 — P0 / P1

_No P0/P1 items: the entire P0/P1 Help/diagnostics surface (About, Help URLs, log export, crash telemetry, in-app log viewer, package logs, launch screen) shipped during the TestFlight sweep — see Current parity. New work in this theme is P2/P3._

### Phase 2 — P2

| ID | Feature | What to build | Desktop ref | Effort | Deps |
|----|---------|---------------|-------------|--------|------|
| `TOOL-4` | Tip-of-the-Day → "What's New" sheet | Product call. Do **not** port the 50+ HTML/PNG carousel + per-tip difficulty filter. Build an Apple-style first-run / on-update "What's New" SwiftUI sheet authored against the actual iPad feature set; show once per version, dismissible, with a "show again" toggle. Also recorded in `99-out-of-scope.md §7` (the FEATURE, not just the prefs, since the raw analysis only marks the two prefs — MinimumTipLevel / RecycleTips — as out-of-scope). | `TipOfTheDayDialog.cpp` + `TipOfDay/` assets | M | — |
| `TOOL-9` | Package Show for Support (vs Package Logs) | Verify whether the shipped `XLLogPackager` bundles only logs + diagnostics, or the full show folder + autosaves like desktop H-7. If narrower, extend the packager (or add a second "Package Show for Support" action) to include the show folder tree + autosave dir → share sheet. If already full-scope, close as done. | `xLightsMain.cpp` Package Show for Support (H-7) | M | — |

### Phase 3 — P3

| ID | Feature | What to build | Desktop ref | Effort | Deps |
|----|---------|---------------|-------------|--------|------|
| `TOOL-1` | ShowFolderSearchDialog → Files-app picker | Re-conceive. Desktop scans the device filesystem for existing xLights show folders — meaningless in the iOS sandbox. Replace with a `.fileImporter` folder picker (security-scoped) that lets the user point at an existing show folder in Files/iCloud; obtain a persistent bookmark via `ObtainAccessToURL`. Absent from all other docs and the raw dataset. | `ShowFolderSearchDialog.cpp`; `xLightsMain.cpp:1158/9116` | S | — |
| `TOOL-2` | PathGenerationDialog / Generate 2D Path | Touch-draw canvas emitting 2D node-path data for sketch / path-driven effects. Desktop is a mouse/key draw surface + file picker + smoothing slider. On iPad reuse the Sketch / Morph inline-custom-row touch-draw idiom (cross-ref FX-9 / FX-12 in `04-effects.md`): finger/Pencil draw → smoothing slider → emit node-path. | `PathGenerationDialog.cpp`; `xLightsMain.cpp:1142/8015` | M | — |
| `TOOL-3` | Prepare Audio (Reaper `.rpp` / `.xaudio`) | Port-vs-drop decision needed. Desktop applies a Reaper `.rpp` / `.xaudio` change-description to the show audio. **Flag the dev-stub nature**: the live path reads a reaper file, but the leading body is a dead `#if 0` test stub — the team should decide port-vs-drop before any work. Likely depends on FFmpeg / audio editing (no FFmpeg encode in the iPad lib — see `99 §3`), so verify viability first; cross-ref `02-timing-and-audio.md`. | `xLightsMain.cpp:1154/8021` | S (gated on decision) | — |
| `TOOL-5` | CharMapDialog | Likely platform-substituted. Desktop is a curated character-map picker for the Text effect; the iOS emoji/character keyboard (and the standard text field) almost certainly covers this. Record the substitution decision; only reimplement a curated picker if testers hit a specific glyph the iOS keyboard can't reach. | `src-ui-wx/effects/CharMapDialog.cpp` | S | — |
| `TOOL-6` | ResizeImageDialog | Likely platform-substituted by iOS Photos editing (crop/scale on import). Desktop is a standalone crop/scale of an image asset on media import; note that AI-3 / AI-7 crop/resize is AI-image-only, so this is the only general-image crop path. Decide whether to surface an in-app crop step or rely on the user pre-editing in Photos. | `src-ui-wx/media/ResizeImageDialog.cpp` | S | — |
| `TOOL-7` | EmailDialog crash-report prompt | Product call. Desktop's one-time / at-report email-collection modal (`CollectUserEmail` seeds `noone@nowhere.xlights.org`, keeps the prior address on cancel). PREF-14 only adds a buried stored `userEmail` field; decide whether a prompt is wanted so `XLDiagnosticUploader` can attach a return address. If yes, a small SwiftUI sheet writing the same persisted key PREF-14 uses. | `EmailDialog.cpp`; `xLightsMain.cpp:8503` | S | PREF-14 |
| `TOOL-8` | Help menu — add Donate + Zoom Room Help | Two missing links. The iPad `.help` `CommandGroup` (`XLightsCommands.swift:478-514`) has Manual / Tutorials / Release Notes / Forum / Facebook / Issue Tracker / Website but lacks desktop's **Donate** (`https://www.paypal.com/donate/?hosted_button_id=BB6366BT755H6`, `DoDonate` at `xLightsMain.cpp:6537`) and **Zoom Room Help** (`MenuItem_Zoom`, `xLightsMain.cpp:1273`). Add two `Button`s opening those URLs via `XLOpenURL`. | `xLightsMain.cpp:1273` (Zoom) / `:1291,6537` (Donate) | S | — |

## Quick wins

- **`TOOL-8`** (Help menu — Donate + Zoom Room): two `Button`s + two URLs in the existing `.help` `CommandGroup` at `XLightsCommands.swift:478-514`. Pure parity, minutes of work. **(S)**
- **`TOOL-1`** (ShowFolderSearchDialog → Files picker): a single `.fileImporter` + `ObtainAccessToURL` bookmark; replaces a sandbox-meaningless scan with a one-tap picker. **(S)**
- **`TOOL-5` / `TOOL-6`** (CharMapDialog / ResizeImageDialog): mostly a *decision* — confirm the iOS keyboard / Photos editing substitutes and close, no code unless a concrete gap surfaces. **(S)**

## Out of scope for this theme

One-liners here; full reasons live in `99-out-of-scope.md`:

- **Check-for-Updates / `UpdaterDialog`** (H-8) — App Store handles iPad updates.
- **Tip-of-the-Day *preferences*** (MinimumTipLevel / RecycleTips) — moot once the carousel is replaced by the TOOL-4 "What's New" sheet; only these two prefs were flagged out-of-scope by the raw analysis, the FEATURE itself is the TOOL-4 product call (and also a §7 row in 99 with the same rationale).
- **Crash xLights** (`xLightsMain.cpp:1137`) — dev-only intentional-crash telemetry tool; the iPad equivalent duplicates Package Logs / `XLDiagnosticUploader` without adding signal. Tracked in `99 §7`.
- **Log Render State** (`xLightsMain.cpp:1139`) — dev-only render-state dump; iPad already has live View Log + Package Logs. Tracked in `99 §7`.
- **Run Scripts (Lua / Python) + script library management** — covered in `12-automation.md` / `99-out-of-scope.md`; the in-process JSON command dispatcher is the in-scope automation surface, downloaded community scripts are out (App Store guideline 2.5.2).

## Risks / open questions

- **TOOL-3 Prepare Audio is a dev stub.** The desktop entry point's leading body is dead `#if 0` test scaffolding; the live path reads a Reaper file. Resolve port-vs-drop *before* committing effort, and confirm whether the `.rpp`/`.xaudio` application needs FFmpeg audio editing (absent from the iPad lib per `99 §3`) — if so it may be effectively undeliverable on iPad and should drop.
- **TOOL-9 packager scope unknown.** Need to read what `XLLogPackager` actually bundles today — if it already includes the show folder + autosaves it's done; if logs-only, H-7 is a real gap. Verify before sizing.
- **TOOL-4 / TOOL-7 are product decisions, not engineering blockers.** Whether to ship a "What's New" sheet and whether to prompt for a crash-report email are owner calls; the engineering for both is small once decided. TOOL-7 must write the same persisted key PREF-14 uses so the two don't diverge.
- **TOOL-5 / TOOL-6 substitution confidence.** Assumed the iOS keyboard fully covers CharMapDialog and Photos editing covers ResizeImageDialog; if a tester hits a Text-effect glyph the keyboard can't produce, or a media-import crop the user can't pre-do, these reopen as small reimplementations.
- **TOOL-1 bookmark persistence.** The Files-app picker must round-trip a security-scoped bookmark via `ObtainAccessToURL` so the chosen show folder survives app restart; verify the existing folder-config bookmark flow covers a folder picked here.
- **View Log search.** `LogViewerSheet` shipped with a level filter; free-text search may be absent (gap-analysis H-6 listed both). Minor — confirm and, if missing, a one-line `searchable` add.
