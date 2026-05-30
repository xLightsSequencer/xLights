# Future — Help, About, Diagnostics

About / Help menu URLs / log export / crash telemetry / Check
Sequence / in-app log viewer (H-6 + T-2) all shipped during the
TestFlight sweep. What remains is the backup-restore surface and a
small set of Tools utilities.

Source: §2.16 of the 2026-04-23 gap analysis (Phase P + parts
of pre-TestFlight Phase I).

## Open items

All map to entries on the desktop **Tools** menu unless noted.

| # | Item | Severity | Effort |
|---|---|---|---|
| H-10 | EmailDialog (crash-report email collection) | P3 | S |
| H-11 | RestoreBackupDialog full UI (paired with [`future-preferences.md`](future-preferences.md) backup section) | P2 | M |
| H-12 | ShowFolderSearchDialog (= T-22) | P3 | S |
| T-3 | **Cleanup File Locations** — moves all files into / under the show folder | P2 | S |
| T-9 | **Purge Render Cache** + **Purge Download Cache** | P3 | S |
| T-15 | **Prepare Audio** (Reaper `.rpp` / `.xaudio`) | P3 | S |
| T-16 | **User Lyric Dictionary** (LyricUserDictDialog) | P3 | S |
| T-22 | **Search for Show Folders** | P3 | S |
| T-24 | **Generate 2D Path** — `PathGenerationDialog`; generates 2D node-path data for sketch / path-driven effects from a manually drawn path | P3 | M |

## Items handled by the platform

- **H-8 UpdaterDialog.** App Store handles iPad updates — N/A.
- **H-9 SplashDialog.** LaunchScreen handles it (Phase H-1).

## No plans to port

- **H-5 TipOfDay** — 50+ HTML / PNG tips in `TipOfDay/`, per-tip
  difficulty filter (`MinimumTipLevel` pref), carousel UI.
  Desktop-only by design. The on-disk tip authoring pipeline
  doesn't map cleanly to an iOS bundle, the carousel UX
  duplicates Apple's own "What's New" sheet, and the highest-
  value tips are workflow-specific to desktop features the iPad
  doesn't expose. If we want first-run guidance on iPad, build
  an Apple-style "What's New" sheet against the iPad feature set.
- **Tools → Run Scripts** — Lua user scripts via the desktop's
  Lua 5.4 host. Lua-via-JIT is App Store hostile; an interpreted
  fallback would lose the scripts users actually wrote against
  the desktop-specific API surface (UI automation, controller
  upload, show-folder mutation). Listed in the README hard-miss
  set; revisit only if a constrained scripting API emerges that
  is App-Store-safe and worth the porting cost.
- **Tools → Crash xLights** + **Tools → Log Render State** —
  developer diagnostics; the iPad equivalents (intentional
  crash for telemetry verification, render-state dump) would
  duplicate Package Logs without adding signal.

## When to come back

- H-11 RestoreBackupDialog pairs naturally with the backup
  stack in [`future-preferences.md`](future-preferences.md) — both
  warrant their own sprint once `Library/Backups/` snapshots exist.
- T-3 Cleanup File Locations + T-9 Purge caches are small sprints
  worth pulling in if testers complain about iCloud quota /
  shader-cache bloat.
