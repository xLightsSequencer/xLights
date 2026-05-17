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
- **T-1 Check Sequence** — Tools → Check Sequence runs the
  shared `src-core/diagnostics/SequenceChecker`.

## Gap (still open)

| # | Item | Severity | Effort |
|---|---|---|---|
| H-6 | **In-app log viewer** — level filter, search, follow-tail toggle. Reads the spdlog rotate-files. | P2 | M |
| H-7 | **Package Show for Support** — zips logs + show folder + autosaves into one file the user can attach to a forum post. Larger sibling of H-3 log export. | P2 | M |
| H-10 | EmailDialog (crash-report email collection) | P3 | S |
| H-11 | RestoreBackupDialog full UI (paired with [`future-preferences.md`](future-preferences.md) backup section) | P2 | M |
| H-12 | ShowFolderSearchDialog | P3 | S |
| T-2 | View Log (Tools menu entry → opens H-6 viewer) | P2 | S |
| T-20 | Package Log Files (Tools menu entry → runs H-3 log export) | P2 | M |
| T-3 | Cleanup File Locations | P2 | S |
| T-9 | Purge Render / Download caches | P3 | S |
| T-15 | Prepare Audio (Reaper `.rpp`) | P3 | S |
| T-16 | User Lyric Dictionary (LyricUserDictDialog) | P3 | S |
| T-22 | Show Folder Search | P3 | S |

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
- H-6 in-app log viewer + H-7 Package Show for Support are the
  highest-value remaining items — both surface when a tester
  needs to debug an issue without a Mac handy.

## When to come back

- Bundle H-6 in-app log viewer + H-7 Package Show for Support
  as a small follow-up release — they fit into a single 1–2
  week sprint and are the natural next help/diagnostics
  delivery.
