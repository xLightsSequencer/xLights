# Future — Preferences, ColorManager & Backup

Desktop xLights ships ~96 preference options across 10 panels plus
a 38-colour ColorManager and a backup/restore stack. iPad today
exposes ~10 `@AppStorage` keys covering session/UI state only.

Source: §2.17 + §2.21 of the 2026-04-23 gap analysis (Phase M).

PR-9 Services (AI keys / endpoints) shipped via
`AIServicesSettingsSheet`. PR-8 SequenceFile has two knobs
(AutoSaveInterval, DefaultBlending) on the Render tab; B25
ColorManager bracket palette is a small slice of PR-3.
Everything else below is desktop-only.

## Preference panels (still open)

| # | Panel | Options | Persistence | Severity |
|---|---|---|---|---|
| PR-1 | **Backup** | 6 (BackupOnSave, BackupOnLaunch, BackupSubfolders, PurgeInterval, UseShowFolder, BackupDirectory) | wxConfig | P2 |
| PR-2 | **CheckSequence** | 7 disable-check toggles (DupUniverse, NonContigCh, PreviewGroup, DupNodeMG, TransTime, CustomSize, SketchImage) | per-show | P3 |
| PR-3 | **ColorManager** | 38 colour swatches (Timing Tracks ×5, Effect Grid ×28, Layout ×4) + global text colours ×2 + Suppress Dark Mode + Import / Export / Reset Defaults | per-show | P2 |
| PR-4 | **EffectsGrid** | 11 (Spacing, EffectBackgrounds, NodeValues, GroupEffectIndicator, SnapToTimingMarks, DoubleClickMode, SmallWaveform, DisplayTransitionMarks, HideColorWarning, ShowAlternateTimingFormat, BellOnRender) | wxConfig | P2 |
| PR-5 | **Other** | 18 (Email, HardwareVideoDecode, HardwareVideoRender (7 backends), ShadersBgThreads, GPURendering, VideoCodec, Bitrate, ExcludeVideos, ExcludeAudio, PromptBatchIssues, PurgeDownloadCache, IgnoreVendorRecs, LinkControllerUpload, ModelRenamingAlias, MinimumTipLevel, RecycleTips, ControllerPingInterval, EnablePositionZones, ShowZoneIndicator) | wxConfig | P2 |
| PR-6 | **Output** | 4 (UseFrameSync, ForceLocalIP, DuplicateFramesToSuppress, xFade/xSchedule mode) | wxConfig | P2 |
| PR-7 | **RandomEffects** | dynamic — checkbox per installed effect (~80 toggles) | wxConfig | P3 |
| PR-8 | **SequenceFile** (remaining 12 keys: RenderOnSave, LowDef, SaveFSEQ, RenderCache mode, FSEQVersion (5 options), UseShowFolderRenderCache, RenderCacheDir, MaxRenderCacheSize, UseShowFolderFSEQ, FSEQDir, MediaDirs list, DefaultViewForNew) | per-show | P2 |
| PR-10 | **ViewSettings** | 12 (EffectIconSize, ModelHandleSize, EffectAssistWindow, ShowPlayControlsOnPreview, AutoShowHousePreview, EnableBaseShowSettings, TimelineZooming, HidePresetPreviews, ZoomToCursor, DisableKeyAcceleration, GroupCenterCrosshairSize, ColorPaletteSize) | wxConfig | P2 |

## ColorManager (PR-3 in detail)

PR-3 is the largest single-panel piece and the one most likely
to surface in tester feedback. The desktop panel exposes:

- 5 Timing-track colours (Fixed, Single Mark, Bracket, Phrase,
  Word).
- 28 Effect-grid colours (selection halo, timeline cursor, bracket
  fills, ruler labels, etc.). B25 already covers the bracket
  palette — the rest of the 28 is still desktop-only.
- 4 Layout-tab colours.
- 2 global text colours.
- Suppress Dark Mode toggle.
- Per-show Import / Export / Reset Defaults buttons.

iPad coupling: B25's persistence already routes through the
`xLightsImportChannelMapDialog`-adjacent show-folder XML; full
ColorManager would extend that round-trip rather than introduce
a new store.

## Backup / restore

| # | Item | Severity | Effort |
|---|---|---|---|
| L-2 | Full show-folder backup zip / `.xsqz` snapshot | P2 | M |
| L-3 | RestoreBackupDialog (multi-panel browser) | P2 | M |
| L-4 | Backup purge (auto-delete > N days) | P3 | S |
| L-6 | External-change watcher for `xlights_rgbeffects.xml` | P3 | M |
| L-7 | Conflict-resolution UI for concurrent edits | — | Defer |
| L-8 | Backup On Save / Backup On Launch toggles (PR-1 surface) | P2 | S |
| L-9 | Alternate Backup destination (F11 hotkey on desktop) | P3 | S |
| L-10 | Show base directory toggle | P2 | M |

## Why deferred

- iPad lives off `@AppStorage` for UI state and per-show XML for
  document state. Both paths work; what's missing is the *user-
  facing settings surface*.
- ColorManager (PR-3) is the highest-value individual entry —
  several testers are likely to ask for darker / higher-contrast
  themes once they're using the app daily.
- Backup (L-2..L-9) is partially covered by iCloud Drive's own
  versioning + the Phase G `.xsqz` packager.

## When to come back

- Roughly in this order: PR-3 ColorManager → PR-4 + PR-10
  (effect-grid view settings) → PR-8 (sequence-save knobs) →
  backup stack → the rest.
