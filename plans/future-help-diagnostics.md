# Future — Help, About, Diagnostics

The desktop has a sizeable help / about / diagnostics surface
(About, Help menu, Tip-of-the-Day, log export, in-app log
viewer, crash telemetry, Package Show for Support). iPad
currently has the menu shell only.

Source: §2.16 of the 2026-04-23 gap analysis (Phase P + parts
of pre-TestFlight Phase I).

## Already shipped (verified 2026-05-02)

The TestFlight-quality items below all landed during the pre-
submission sweep — see the iPad-xLights-Plan.md "Could pull into
MVP" section for the implementation summary.

- **H-1 About** — `AboutSheet.swift` (icon, version, build, GPL
  text, Privacy + EULA links).
- **H-2 Help menu URLs** — seven external links in the Help
  group routed through `XLOpenURL`.
- **H-3 log export** — Tools → Package Logs zips
  `xLights.log` siblings, MetricKit JSON, threads + device-info
  sidecars; logs moved from `Documents/` to `Library/Logs/`.
- **H-4 crash telemetry** — `XLDiagnosticUploader.swift` posts
  staged zips to `crashUpload/index.php`; MetricKit subscription
  via shared `XLMetricKit`.
- **H-6 In-app log viewer** (2026-05-17) — `LogViewerSheet.swift`
  reads the rotating spdlog file at `Library/Logs/xLights.log`,
  with level / logger / text filters and a 1s follow-tail. Last
  2000 lines kept in memory; rotated `.1`/`.2` siblings remain
  reachable via Package Logs.
- **T-1 Check Sequence** — Tools → Check Sequence runs the
  shared `src-core/diagnostics/SequenceChecker`.
- **T-2 View Log** (2026-05-17) — Tools → View Log opens the
  H-6 viewer.

## Gap (still open)

| # | Item | Severity | Effort |
|---|---|---|---|
| ~~H-6~~ | ✓ shipped 2026-05-17 — `LogViewerSheet.swift` reads `Library/Logs/xLights.log`, level filter (Trace+/Debug+/Info+/Warn+/Error), per-logger filter (xLights/render/curl/job/work), text search, follow-tail toggle (1s periodic refresh), last 2000 lines | — | — |
| H-10 | EmailDialog (crash-report email collection) | P3 | S |
| H-11 | RestoreBackupDialog full UI (paired with [`future-preferences.md`](future-preferences.md) backup section) | P2 | M |
| H-12 | ShowFolderSearchDialog | P3 | S |
| ~~T-2~~ | ✓ shipped 2026-05-17 — Tools → View Log menu entry presents H-6 viewer | — | — |
| T-20 | ✓ shipped — Package Log Files lives in Tools (mirrors desktop's `PackageDebugFiles`); see "Already shipped" above | — | — |
| T-3 | Cleanup File Locations | P2 | S |
| T-9 | Purge Render / Download caches | P3 | S |
| T-15 | Prepare Audio (Reaper `.rpp`) | P3 | S |
| T-16 | User Lyric Dictionary (LyricUserDictDialog) | P3 | S |
| T-22 | Show Folder Search | P3 | S |

Note: **H-7 "Package Show for Support"** was removed 2026-05-17.
It described a hypothetical "zip logs + show folder + autosaves"
that doesn't match a real desktop feature. Desktop has two
distinct Package commands — `PackageDebugFiles` (already mirrored
on iPad as Tools → Package Logs / H-3 / T-20) and
`PackageSequence` (a `.xsqz` for sharing a sequence with another
user). The latter is tracked in
[`future-imports-exports.md`](future-imports-exports.md) under
EX-11.

## Items handled by the platform

- **H-8 UpdaterDialog.** App Store handles iPad updates — N/A.
- **H-9 SplashDialog.** LaunchScreen handles it — landed in Phase
  H-1.

## No plans to port

- **H-5 TipOfDay** — 50+ HTML / PNG tips in `TipOfDay/`, per-tip
  difficulty filter (`MinimumTipLevel` pref), carousel UI.
  Desktop-only by design. The on-disk tip authoring pipeline
  (HTML + companion PNGs in a sibling `TipOfDay/` folder)
  doesn't map cleanly to an iOS bundle, the carousel UX
  duplicates Apple's own "What's New" sheet, and the highest-
  value tips are workflow-specific to desktop features the iPad
  doesn't expose (controller setup, layout group editing in the
  layout panel, AUI perspectives). Not worth porting; if we
  want first-run guidance on iPad, build an Apple-style "What's
  New" sheet against the iPad feature set instead.

## Why deferred

- The followups bundle covers what TestFlight reviewers need.
  The items above are quality-of-life polish that mostly applies
  once the app is in front of long-term users.

## When to come back

- H-11 RestoreBackupDialog pairs naturally with the backup
  stack in [`future-preferences.md`](future-preferences.md) — both
  warrant their own sprint once `Library/Backups/` snapshots exist.
- T-3 Cleanup File Locations + T-9 Purge caches are small sprints
  worth pulling in if testers complain about iCloud quota /
  shader-cache bloat.
