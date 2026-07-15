# 01 · File Lifecycle & Sequence Management

> **Status:** The day-to-day file
> lifecycle is at near parity. Both platforms have New / Open / Save /
> Save As / Close, Recent Sequences, Recent Show Folders, Sequence
> Settings (Info / Metadata / Timings / Audio / Media / Render), dirty
> tracking, unsaved-changes prompts, configurable auto-save (`.xbkp`),
> Package Sequence, Package Logs, and Check Sequence. The **Base Show
> Folder**, **Render Cache**, **Low-Definition Render**, **Render on
> Save**, and **FSEQ folder** features are at **parity** — desktop
> exposes them in the Setup tab / Preferences
> (`SequenceFileSettingsPanel`, `BackupSettingsPanel`). The real
 > **iPad gaps** narrowed further in the 2026-06-12 pass: **Restore
> Backup** (now consumes the in-show `<show>/Backup/` snapshots),
> **Render Mode** (Erase/Canvas), **Read-only guard**, and **Change
> Show Folder Temporarily** all landed at parity. Whole-folder
> **Backup / Alternate Backup** stay infeasible under the sandbox.
> **Pre/Post-milliseconds** was re-scoped (audio-editor, not a setting) — it is a
> desktop FFmpeg audio editor (re-encodes media with silence), not a
> stored setting. Remaining real gaps: **Data Layers** (impl plan
> below), the **New-Sequence wizard Metadata + Timings steps**,
> **quick-start importers**, animated **media preview**, and
> **Download Sequences/Lyrics**. The real **desktop "gap"** is mostly
> platform-idiomatic iPad UX (autosave recovery alert, iCloud download
> badges, Media Relocation) rather than missing desktop capability.
> *File Encoding (UTF-8/ANSI)* and *Reopen Last Sequence preference* do
> not exist in the desktop code.

## Parity scorecard

| Feature | Surface | Desktop | iPad | Gap | Priority | Ease | Feasibility | Notes |
|---|---|---|---|---|---|---|---|---|
| New Sequence | menu/toolbar | ✅ | ✅ | parity | P1 | easy | feasible | Desktop wizard (≤5 steps) vs iPad 3-step wizard (type/content/save). 2026-07-05: create runs detached off the main actor (`SequencerViewModel.newSequence` async) — the bridge's close/abort-render wait was blowing the 0x8BADF00D watchdog (crash sig 03308ce269). |
| New Seq — type picker | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Musical/Animation/Effect both. |
| New Seq — media pick | dialog | ✅ | ✅ | parity | P1 | easy | feasible | iPad routes through MediaRelocation; desktop free path. |
| New Seq — duration & frame rate | dialog | ✅ | ✅ | parity | P1 | easy | feasible | iPad TextField + segmented picker (25/50/100). |
| New Seq — save location | dialog | ✅ | ✅ | parity | P1 | easy | feasible | iPad fileExporter pinned to show folder. |
| New Seq — metadata wizard step | dialog | ✅ | ❌ | ipad-missing | P3 | medium | feasible | iPad defers to Settings sheet post-open. |
| New Seq — timing-import wizard step | dialog | ✅ | ❌ | ipad-missing | P3 | medium | feasible | iPad defers to row-header / Settings Timings tab. |
| New Seq — quick-start importers (LOR/Vixen/GLD/HLS/Lynx/xLights) | dialog | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Layout/controller import templates; not built on iPad. |
| Open Sequence | menu/toolbar | ✅ | ✅ | parity | P1 | easy | feasible | iPad SequencePickerView is the "open" surface (list + Files). |
| Open Recent Sequence | menu/panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad per-show-folder MRU (RecentSequences), swipe-remove, Clear Recent. |
| Open via Files / AirDrop / share (.xsq) | gesture/system | 🟡 | ✅ | desktop-missing | P3 | medium | feasible | iPad `.onOpenURL` w/ security-scope + deferred-open queue; desktop is file-association/CLI only. |
| Open Packaged Sequence (.xsqz) | system | 🟡 | ✅ | desktop-weaker | P2 | medium | feasible | iPad opens an `.xsqz` **and repacks it back to the original on Save** (round-trip; `SequencerViewModel.swift:32-37`). Desktop opens by *extracting* read-only (`xLightsApp.cpp:840` `readOnlyZipFile`), NOT in File>Open, and cannot save back into the same package — its `SequencePackage::Pack` is a separate export. Reverse-parity item; see theme 14. |
| Save Sequence | menu/toolbar/shortcut | ✅ | ✅ | parity | P1 | easy | feasible | iPad ⌘S; clears undo + writes fseq when enabled. |
| Save Sequence As | menu/shortcut | ✅ | ✅ | parity | P1 | easy | feasible | iPad ⌘⇧S → fileExporter; updates path. |
| Close Sequence | menu/shortcut | ✅ | ✅ | parity | P1 | easy | feasible | iPad ⌘W; closeRequestToken → prompt. 2026-07-05: teardown (abort-render wait + bridge close) runs detached off the main actor (`closeSequence` async, watchdog sig 0c45db2f0e); `abortRenderAndWait:` now passes its timeout through to `AbortRender`. |
| Unsaved-changes prompt on close | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop SaveChangesDialog; iPad confirmationDialog (Save/Discard/Cancel). |
| Unsaved-changes handling on quit/background | dialog | ✅ | 🟡 | parity | P1 | easy | feasible | Desktop CheckUnsavedChanges prompt; iPad silently saves on `.inactive`/`.background` (Stage-Manager pill can't host an alert). |
| Sequence Settings dialog/sheet | menu/toolbar | ✅ | ✅ | parity | P1 | easy | feasible | Tabs differ (see below). |
| Settings — Info tab | dialog tab | ✅ | ✅ | parity | P1 | easy | feasible | iPad info is read-only subset (filename/version/duration/frame/model count) + type + audio swap. |
| Settings — Metadata tab | dialog tab | ✅ | ✅ | parity | P1 | easy | feasible | song/artist/album/author/email/website/url/comment. |
| Settings — Timings tab | dialog tab | ✅ | ✅ | parity | P1 | medium | feasible | iPad fully built (add/rename/delete/export single+multi/import 9 formats incl. multi-track Vixen3/xsq). |
| Settings — Audio Tracks tab | dialog tab | ✅ | ✅ | parity | P1 | easy | feasible | alt-track add/remove/rename/replace. |
| Settings — Media tab | dialog tab | ✅ | ✅ | parity | P1 | easy | feasible | per-effect media inventory embed/extract/rename/replace. |
| Settings — Render tab | dialog tab | ✅ | ✅ | parity | P1 | easy | feasible | iPad: model blending + frame interval + autosave interval picker. |
| Settings — Data Layers tab | dialog tab | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop tree importer; absent on iPad. **Impl plan added 2026-06-12 (below).** |
| Settings — Render Mode selector | dialog | ✅ | ✅ | parity | P3 | medium | feasible | ✅ 2026-06-12. Desktop `RenderModeChoice` (Erase/Canvas) stored on the Nutcracker data layer. iPad: Render tab segmented Erase/Canvas picker (`SequenceSettingsSheet.swift` RenderTab) → bridge `renderMode` / `setRenderMode:` (`XLSequenceDocument.mm`) maps the UI strings ↔ `SequenceFile::GetRenderMode/SetRenderMode`, normalizing the legacy ERASE_MODE/CANVAS_MODE sentinels. |
| Settings — Hash/Checksum display | dialog | ✅ | ✅ | parity | P3 | easy | feasible | Desktop `TextCtrl_Hash` (media hash); iPad Info-tab "Media Hash" row → bridge `mediaFileHash` (`AudioManager::Hash`). |
| Settings — Pre/Post milliseconds | dialog | ✅ | ❌ | ipad-missing | P3 | hard | feasible | **Re-scoped 2026-06-12.** NOT a stored timing-shift setting — desktop's `OnButton_AddMilliseconds` (`SeqSettingsDialog.cpp:2088`) is an *audio editor*: decode, prepend/append silence (or trim) by the entered ms, re-encode a new media file. Now plausible on iPad via the Wave-5 encode path (`AudioManager::WriteCurrentAudioToFile` → AudioToolbox `EncodeToFile`): snapshot buffers, pad/trim sample arrays, encode to a new .m4a, repoint media. P3 backlog, not infeasible. |
| Settings — Overwrite Tags on media change | dialog | ✅ | ✅ | parity | P3 | easy | feasible | iPad Info-tab toggle (`overwriteTagsOnMediaChange` @AppStorage, default off): on audio swap, pulls Song/Artist/Album from the new file's tags via `audioTitle/Artist/Album`. |
| Settings — editable Duration override | dialog | ✅ | ✅ | parity | P3 | easy | feasible | Desktop `TextCtrl_Xml_Seq_Duration`; iPad Info-tab editable Duration field → bridge `setSequenceDurationMS` (resizes render data via `EnsureSequenceDataSized`). |
| Select Show Folder | menu / sheet | ✅ | 🟡 | ipad-weaker | P1 | easy | feasible | iPad FolderConfigView (gate-required before sequences); 'Change Show Folder…' button at `FolderConfigView.swift:90`, commit via `viewModel.loadShowFolder` at :400, but no validation/heuristic on the chosen folder. |
| Recent Show Folders | menu / sheet | ✅ | ✅ | parity | P1 | easy | feasible | iPad list w/ swipe-delete; tap-to-switch (apply on Done). |
| Change Show Folder Temporarily | menu / sheet | ✅ | ✅ | parity | P3 | medium | feasible | ✅ 2026-06-12. FolderConfigView "Switch Temporarily…" button (`FolderConfigView.swift` `.showFolderTemporary` picker mode) → `viewModel.loadShowFolderTemporarily(path:mediaFolders:)` (`SequencerViewModel.swift`) loads the folder without writing `FolderConfig.showFolder` or recording to `RecentShowFolders` (one-shot `suppressRecentRecording` flag); `temporaryShowFolderActive` drives a "default unchanged, restored on next launch" note. Matches desktop `xLightsMain.cpp:894`. |
| Read-only mode guard | app/state | ✅ | ✅ | parity | P3 | easy | feasible | ✅ 2026-06-12. On open, `SequencerViewModel.detectReadOnly(path:)` probes writable access (write-scope bookmark + `FileManager.isWritableFile`) and sets `isReadOnly`. Save (toolbar `SequencerView.swift`, ⌘S `XLightsCommands.swift`) and autosave are disabled; a `ReadOnlyBanner` (`XLightsApp.swift`) shows "Opened read-only" with a Save As shortcut. Save As / export remain enabled and clear the flag. Matches desktop `xLightsMain.cpp:9119-9120`. |
| Base Show Folder (inheritance) | setup/sheet | ✅ | ✅ | parity | P2 | medium | feasible | Desktop Setup tab (`Button_ChangeBaseShowDir`); iPad FolderConfigView base section + Auto-Update + Update Now. |
| Media Folders configuration | preference/sheet | ✅ | ✅ | parity | P2 | medium | feasible | Desktop has media dirs concept; iPad FolderConfigView media-folder list. |
| Media Relocation on file pick | gesture/dialog | ❌ | ✅ | desktop-missing | P2 | medium | feasible | iPad enforces "copy under show/media folder"; desktop free path + Cleanup later. |
| Add media to sequence (multi-file) | manager/sheet | ✅ | ✅ | parity | P1 | medium | feasible | Desktop `ManageMediaPanel.cpp:2004` (`_addButton` "Add", `wxFD_MULTIPLE`); iPad MediaManagerSheet "Add…" toolbar button → multi-select `.fileImporter` → relocate under show folder + `addMedia(atPath:)` bridge (`MediaManagerSheet.swift` `addPickedFiles`; `XLSequenceDocument.mm` `addMediaAtPath:`). |
| Bulk find/repoint media | manager/sheet | ✅ | 🟡 | ipad-weaker | P2 | small | feasible | Desktop `ManageMediaPanel.cpp` (2026-07) generalized "Bulk Find `<Type>`s…" to offer whenever there's more than one item of a media type, and to search/relink *every* item of that type — not just currently-broken ones — so users can bulk-redirect a whole set of already-working files to a new folder (e.g. a show copied to a new year). iPad `MediaManagerSheet`'s "Bulk Find Missing…" is still `.disabled(brokenCount == 0)` and only relinks broken entries (`bulkFindMissing(in:)`). |
| Media preview (still + animated) | manager/sheet | ✅ | 🟡 | ipad-weaker | P3 | medium | feasible | iPad shows a static `MediaThumbnailView` with no animation (`MediaManagerSheet.swift:503-518`); bridge `ensureThumbnailPreviewForPath` (`XLSequenceDocument.h:2055`). |
| Reload media from disk | manager/sheet | ✅ | ✅ | parity | P3 | easy | feasible | iPad MediaManagerSheet external rows have a "Reload from Disk" swipe action → `reloadMedia(atPath:)` bridge purges the cache entry so the next render re-reads the file (`MediaManagerSheet.swift` `reloadFromDisk`; `XLSequenceDocument.mm` `reloadMediaAtPath:` / `reloadAllMedia`). |
| Render Cache mode | preference/sheet | ✅ | ✅ | parity | P2 | easy | feasible | Desktop Preferences `RenderCacheChoice` (Enabled/Locked/Disabled); iPad FolderConfigView picker. |
| Low-Definition Render toggle | preference/sheet | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `CheckBox_LowDefinitionRender`; iPad FolderConfigView toggle. |
| FSEQ folder / save-on-save | preference/sheet | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `FSEQSaveCheckBox`; iPad FolderConfigView FSEQ section. |
| FSEQ compression format + level | preference/sheet | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `FSEQVersionChoice` (V1/V2 ZSTD/…); iPad zstd/zlib/none + zstd level 1–22. |
| Render on Save | preference | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `RenderOnSaveCheckBox`; iPad via FolderConfig.fseqEnabled (fseq write on save). |
| Default Model Blending for new sequences | preference | ✅ | ❌ | ipad-missing | P3 | easy | feasible | Desktop `ModelBlendDefaultChoice`; iPad sets per-sequence in Render tab only. |
| Auto-Save (timed `.xbkp`) | preference | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `AutoSaveIntervalChoice` (Off/3/5/10/15/30); iPad picker (Off/2/5/10/30), default 5 min. |
| Auto-Save recovery prompt on open | dialog | 🟡 | ✅ | desktop-missing | P2 | easy | feasible | iPad alert (Recover/Discard/Keep) when `.xbkp` newer than `.xsq`; desktop opening `.xbkp` is manual via Open dialog. |
| Backup (manual, F10) | menu/shortcut | ✅ | ❌ | ipad-missing | P2 | hard | infeasible | Whole-show-folder dated copy; iOS has no equivalent (Files app / iCloud). |
| Alternate Backup (F11) | menu/shortcut | ✅ | ❌ | ipad-missing | P3 | hard | infeasible | Second backup dir (offsite/external). No iPad equivalent. |
| Backup On Save / On Launch / Subfolders / Purge | preference | ✅ | 🟡 | ipad-weaker | P3 | hard | feasible | `BackupSettingsPanel`; tied to whole-folder backup. iPad now has a lightweight **Backup On Save** (✅ 2026-06-12, see [11-preferences-settings](11-preferences-settings.md)) snapshotting the saved `.xsq`/package into `<show>/Backup/<name>-<timestamp>.<ext>` (keep last 20) — a per-sequence timestamped copy rather than the whole-folder On-Launch/Subfolders/Purge matrix. |
| Restore Backup dialog | menu/dialog | ✅ | ✅ | parity | P2 | hard | feasible | ✅ 2026-06-12. File menu "Restore Backup…" (`XLightsCommands.swift`) → `RestoreBackupSheet` (`src-iPad/App/RestoreBackupSheet.swift`) lists the current sequence's `<show>/Backup/<base>-<timestamp>.xsq` snapshots (newest first, with date + size) written by Backup-On-Save. Restore safety-snapshots the current state, closes, copies the snapshot over the working `.xsq`, and reopens (`SequencerViewModel.availableBackupSnapshots()` / `restoreBackup(_:)`). Scoped to unpackaged `.xsq` (packages show an unavailable state — see notes). The whole-folder desktop `RestoreBackupDialog` (layouts) stays out of scope under the sandbox. |
| Revert To → Last Saved | menu | ✅ | ✅ | parity | P2 | medium | feasible | ✅ landed 2026-06-11. File menu "Revert to Last Saved…" (`XLightsCommands.swift:55`); confirmation dialog in `SequencerView.swift:204`; `revertRequestToken` token + `revertToLastSaved()` in `SequencerViewModel.swift:722,1719`. Deletes `.xbkp` before reopen so no spurious recovery offer. |
| Revert To → backup history | menu | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Submenu from backup folder versions. |
| Cleanup File Locations | menu | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Relocate all files under show folder. iPad's MediaRelocation enforces invariant at pick time → largely moot. |
| Package Sequence (.xsqz) | menu/sheet | ✅ | ✅ | parity | P1 | easy | feasible | iPad sheet w/ Include Audio/Videos toggles → share sheet; shared `SequencePackage::Pack`. |
| Package Log Files | menu | ✅ | ✅ | parity | P2 | easy | feasible | iPad bundles logs+MetricKit+show+seq+device sidecar → share. |
| Export House Preview Video | menu | ✅ | ✅ | parity | P1 | hard | feasible | ✅ landed 2026-06-03. Tools → "Export House Preview…" (`ExportHousePreviewSheet`) → `XLHousePreviewVideoExporter` renders the house preview offscreen at a chosen resolution (models + view objects + 2D background, live camera/viewport, brightness-correct) → core `VideoWriter`. See theme 08. |
| Download Sequences/Lyrics | menu | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Vendor catalog browser; not built on iPad (vendor browser exists for models, not seq/lyrics). |
| Check Sequence | menu/sheet | ✅ | ✅ | parity | P2 | easy | feasible | iPad CheckSequenceSheet (subset of checks, severity grouping, jump-to-effect). |
| Effect Presets (persistent library) | menu/panel/sheet | ✅ | ✅ | parity | P2 | easy | feasible | Desktop checkable pane (`ShowHideEffectPresetsWindow`); iPad ⌘⇧P PresetBrowserSheet. |
| Sequence name display | title/chrome | ✅ | ✅ | parity | P2 | easy | feasible | iPad `.navigationTitle(mainSceneTitle)`. |
| Unsaved indicator (dirty state) | toolbar/title | ✅ | ✅ | parity | P2 | easy | feasible | Desktop title `*`; iPad Save-button dot + disabled-state. |
| Show-folder persistence across restarts | preference | ✅ | ✅ | parity | P1 | easy | feasible | Desktop wxConfig; iPad FolderConfig/UserDefaults (deferred restore via `.task`). |
| Reopen last sequence on launch | preference | 🟡 | ❌ | ipad-missing | P3 | medium | feasible | iPad deliberately does NOT auto-reopen the sequence (launch watchdog); only the show folder restores. Desktop CLI/last-state opens. (Not a discrete pref panel.) |
| iCloud download status badges | gesture/UI | ❌ | ✅ | desktop-missing | P3 | medium | infeasible | iPad UbiquityBadge + download-then-open; no desktop analogue (no iCloud-on-demand). |
| Sequence picker sort (Name/Modified/Rendered/Out-of-date) | panel | 🟡 | ✅ | desktop-missing | P3 | medium | feasible | iPad SequenceSortOrder in picker; desktop Open dialog uses OS file sorting only. |
| Batch Render (open surface) | toolbar/sheet | ✅ | ✅ | parity | P2 | medium | feasible | iPad BatchRenderSheet reachable from picker; desktop Batch Render dialog. (Detailed in render theme.) |

## iPad gaps (desktop has, iPad missing)

### P1

- ✅ **Add media to sequence (multi-file)** — **landed 2026-06-11.** Desktop
  `src-ui-wx/media/ManageMediaPanel.cpp:2004` (`_addButton` "Add", `wxFD_MULTIPLE`). iPad's
  `MediaManagerSheet` now has an "Add…" toolbar button → multi-select `.fileImporter`; each picked
  file is registered in place when already under the show/media folder, otherwise copied into the
  show folder (type-canonical subdir, `_N`-suffixed on collision via `MoveToShowFolder`), then
  registered via the new `addMedia(atPath:)` bridge (`src-iPad/Bridge/XLSequenceDocument.mm`
  `addMediaAtPath:` → `SequenceMedia::ForceRefreshEntry`).

### P2

- 🟡 **Bulk find missing media** — landed on iPad 2026-06-11 (`MediaManagerSheet`'s overflow menu
  "Bulk Find Missing…" → folder `.fileImporter`; one security-scoped pass builds a lowercased-basename
  index of the folder tree, then each missing inventory entry is relinked through the existing
  `replaceMissingMedia` primitive, `bulkFindMissing(in:)`; reports "Relinked N, still missing M"), but
  desktop's `ManageMediaPanel.cpp` moved ahead 2026-07: "Bulk Find `<Type>`s…" now offers whenever
  there's more than one item of a type (not just broken ones) and relinks by filename match against
  *every* item, so a whole set of already-working files can be redirected to a new folder in one pass
  (e.g. a show folder copied to a new year). iPad's version is still `.disabled(brokenCount == 0)` and
  broken-only. New gap, not yet closed.

- ✅ **Restore Backup dialog** — **landed 2026-06-12.** File menu "Restore Backup…"
  (`src-iPad/App/XLightsCommands.swift`) → `src-iPad/App/RestoreBackupSheet.swift` enumerates the
  current sequence's `<show>/Backup/<base>-<timestamp>.xsq` snapshots (newest first, date + size) and
  restores a chosen one: safety-snapshot current state → close → copy snapshot over the working `.xsq`
  → reopen (`SequencerViewModel.availableBackupSnapshots()` / `restoreBackup(_:)`). Scoped to
  unpackaged `.xsq`; the desktop whole-folder layout restore stays out of scope under the sandbox.
- ✅ **Revert To → Last Saved** — **landed 2026-06-11.** File menu item
  (`src-iPad/App/XLightsCommands.swift:55`), confirmation dialog
  (`src-iPad/App/SequencerView.swift:204`), `revertRequestToken` counter + `revertToLastSaved()`
  (`src-iPad/App/SequencerViewModel.swift:722,1719`). Matches desktop
  `src-ui-wx/xLightsMain.cpp:2182-2205` "Last Saved" case.
- ✅ **Cleanup File Locations** — **landed 2026-06-11** (tracked in theme 13). Tools → Cleanup File
  Locations sweeps every referenced external media file living outside the show/media folders into
  the show folder and rewrites effect references. See `plans/ipad-parity/13-tools-diagnostics-help.md`.
  (Media-Relocation-on-pick parity going the OTHER way remains an iPad-only idiom.)

### P3

- **New-Sequence wizard Metadata step** (desktop `WizardPage4` in `SeqSettingsDialog`) and
  **Timing-import step** (`WizardPage5`). iPad's `NewSequenceWizardView` is intentionally 3 steps and
  punts both to post-open (Settings sheet). Low value — the post-open paths exist. *Medium.*
- **Quick-start importers** (LOR/Vixen/GLD/HLS/Lynx/xLights) — desktop wizard buttons that seed a new
  show's models/controllers from another tool's file. None on iPad. *Hard.*
- **Sequence Settings Info-tab fields**: ~~Hash, Overwrite Tags, editable Duration~~ **landed
  2026-06-11**; ~~Render Mode~~ **landed 2026-06-12** (Render-tab Erase/Canvas picker → bridge
  `renderMode` / `setRenderMode:`). **Pre/Post-milliseconds** re-scoped — it's a
  desktop FFmpeg audio editor (`SeqSettingsDialog.cpp:2088` `OnButton_AddMilliseconds` re-encodes the
  media with prepended/appended silence), not a stored timing-shift; doable via the Wave-5
  AudioToolbox encode path (pad buffers + EncodeToFile) — P3 backlog.
- **Data Layers tab** — desktop `Panel_DataLayers` tree importer (`SeqSettingsDialog.cpp`). iPad
  absent. *Hard, P3.* **Implementation plan (2026-06-12):**
  1. *Core is already shared.* `SequenceFile::GetDataLayers()` returns a `DataLayerSet`; the
     Nutcracker (auto) layer always exists, and `.iseq`/`.fseq` imports add `DataLayer`s with a
     name, source path, channel offset, channel count, and num-frames. `SequenceFile::Save`
     round-trips them (`SequenceFile.cpp:1390`). No new core data structures are needed.
  2. *Bridge surface* (`XLSequenceDocument.{h,mm}`): add read accessors returning an array of dicts
     (name, source, channelOffset, channels, frames, isNutcracker) for the data-layer tree; mutators
     `setRenderMode:` (done), `setDataLayerChannelOffset:forLayer:`, `removeDataLayer:`, and
     `moveDataLayer:toIndex:` (render order matters — desktop renders layers bottom-up). Each bumps
     the change count.
  3. *fseq/iseq import:* a new bridge `importDataLayer(fromPath:)` that calls the same core import
     desktop uses (`xLightsMain.cpp` data-layer import → `ConvertDataRowToEffects` is desktop-only;
     the raw-channel `.iseq` path that just attaches a `DataLayer` + reads frames at render time is
     the portable one). Gate to `.fseq`/`.iseq` only — proprietary sequence formats (LOR/Vixen) need
     the desktop converters and stay desktop-only.
  4. *SwiftUI:* a new `dataLayers` tab in `SequenceSettingsSheet` — a reorderable `List` of layers
     (Nutcracker pinned top, render-mode shown there), per-row channel-offset stepper + delete swipe,
     and an "Import Data Layer…" `.fileImporter` (`.fseq`/`.iseq`) routed through `MediaRelocation`.
  5. *Render:* verify `iPadRenderContext` honors data layers in the render loop (desktop reads them in
     `RenderEffectFromMap`/buffer init). If the iPad render path skips non-Nutcracker layers, that's
     the load-bearing work item — scope a render-engine pass before committing UI.
  Net: medium UI, but the render-integration verification (step 5) is the risk and is why this stays
  ❌ until a focused render pass confirms data layers composite on iPad.
- **Default Model Blending for new sequences** preference (`ModelBlendDefaultChoice`). iPad only sets
  blending per-sequence in the Render tab. *Easy, P3.*
- **Download Sequences/Lyrics** (`ID_MNU_DOWNLOADSEQUENCES`). iPad has a vendor browser for models but
  not for sequences/lyrics. *Hard, P3.*
- **Reopen last sequence on launch** — deliberately omitted on iPad to dodge the 20-second launch
  watchdog when the show folder is iCloud-evicted (see `XLightsApp.swift:61` comment). Re-enabling
  would need a fully-async open-after-launch. *Medium, P3.*
- ✅ **Change Show Folder Temporarily** — **landed 2026-06-12.** FolderConfigView "Switch
  Temporarily…" button (`src-iPad/App/FolderConfigView.swift`, `.showFolderTemporary` picker mode) →
  `viewModel.loadShowFolderTemporarily(path:mediaFolders:)` loads the folder without writing
  `FolderConfig.showFolder` or recording into `RecentShowFolders` (one-shot `suppressRecentRecording`
  flag consumed in `applyLoadResult`); `temporaryShowFolderActive` drives a "default unchanged"
  note. Next launch restores the persisted default. Matches desktop `xLightsMain.cpp:894`.
- ✅ **Read-only mode guard** — **landed 2026-06-12.** `SequencerViewModel.detectReadOnly(path:)`
  probes writable access on open (write-scope bookmark + `FileManager.isWritableFile`) and sets
  `isReadOnly`; Save (toolbar + ⌘S) and autosave are gated, a `ReadOnlyBanner` (`XLightsApp.swift`)
  offers Save As, and Save As clears the flag. Matches desktop `xLightsMain.cpp:9119-9120`.
- **Media preview (still + animated)** — iPad shows a static `MediaThumbnailView` with no animation
  (`src-iPad/App/MediaManagerSheet.swift:503-518`; bridge `ensureThumbnailPreviewForPath`,
  `src-iPad/Bridge/XLSequenceDocument.h:2055`). Desktop previews animate. *Deferred 2026-06-12 —
  needs an AVPlayerLayer (video) / animated-GIF frame-stepper view + a bridge to expose frame
  sequences; medium SwiftUI/AVFoundation work, low value vs. the static thumbnail. Left ipad-weaker.*
- ✅ **Reload media from disk** — **landed 2026-06-11.** External rows in `MediaManagerSheet` have a
  "Reload from Disk" swipe action (`src-iPad/App/MediaManagerSheet.swift` `reloadFromDisk`) backed by
  the now-exposed `reloadMedia(atPath:)` / `reloadAllMedia` bridge
  (`src-iPad/Bridge/XLSequenceDocument.mm` `reloadMediaAtPath:`), which purges the cache entry so the
  next render re-reads the on-disk file.

## Desktop gaps (iPad has, desktop missing)

These are iPad touch/cloud idioms with no desktop equivalent — most are *feasible-but-pointless* on desktop.

- **Auto-Save recovery prompt on open** — iPad surfaces a Recover/Discard/Keep alert when a `.xbkp`
  is newer than its `.xsq` (`XLightsApp.swift` `checkAutosaveRecovery` + `applyAutosaveBackup`).
  Desktop only lets you manually open the `.xbkp` from the Open dialog (with a warning). A desktop
  "newer backup found — recover?" prompt at open time would be a genuine UX improvement. *P2, easy.*
- **Media Relocation on file pick** — iPad's `MediaRelocation.swift` forces media under the show/media
  folder at pick time. Desktop relies on Cleanup File Locations after the fact. *P2 idiom; infeasible
  to port the modal flow but the invariant could inform desktop.*
- **iCloud download badges + download-then-open** — `UbiquityBadge` / `ubiquityStatus`. No desktop
  analogue (no on-demand cloud files). *Infeasible / N/A on desktop.*
- **Sequence picker sort order** (Name / Modified / Rendered / Out-of-date First) — iPad
  `SequenceSortOrder`. Desktop Open dialog uses OS sorting; the Open Recent submenu is MRU-only. *P3.*
- **Open via Files / AirDrop / share-sheet (.xsq + .xsqz)** with security-scoped bookmarks — iPad
  `.onOpenURL`. Desktop has file-association/CLI open but not an in-app share-target flow. *P3.*

## Infeasible / restricted on iPad

- **Backup / Alternate Backup / Backup-on-save / Backup-on-launch / Purge** — these copy the *entire
  show folder* to a dated subdir (and optionally a second offsite dir). Under the iOS sandbox there is
  no equivalent of a free-roaming whole-folder copy to arbitrary locations; users rely on the Files app
  / iCloud Drive versioning. *Infeasible.*  (A narrower in-show `Backup/` snapshot of just the active
  sequence is feasible and is the realistic substitute — see Restore Backup above.)
- **iCloud download badges** — desktop-direction "gap" that is infeasible on desktop (no on-demand
  cloud materialization model).
- **Pre/Post-milliseconds** (desktop SeqSettings "Add Milliseconds") — re-classified infeasible
  2026-06-12. It's an FFmpeg audio editor (`SeqSettingsDialog.cpp:2088`): decode the media, prepend /
  append silence (or trim) by the entered ms, re-encode a new media file. iPad has no audio
  encode-with-silence path; same family as the other FFmpeg-only audio filters. Desktop-only.
- *No controller-firmware items in this theme* — controller upload/config is covered by the outputs
  theme; nothing here is firmware-gated.

## Recommended sequencing

1. ✅ **Revert To → Last Saved** (P2, medium) — **landed 2026-06-11.** File menu "Revert to Last
   Saved…" with a destructive confirmation dialog; bumps `revertRequestToken`; `revertToLastSaved()`
   suppresses the `.xbkp` then closes and reopens from the canonical `.xsq`. Disabled when not dirty.
2. **Auto-Save recovery prompt on desktop** (P2, easy) — port the iPad recovery alert so desktop users
   don't have to know about manually opening `.xbkp`. Core/`.xbkp` logic already shared.
3. ~~**Sequence Settings Info-tab quick fields** (P3, easy)~~ — ✅ **done 2026-06-11** (Hash,
   Overwrite-Tags, editable Duration).
4. ✅ **In-show Backup + Restore Backup sheet** (P2) — **landed.** Backup-On-Save snapshots (Wave 5)
   plus the 2026-06-12 `RestoreBackupSheet` restore picker. Sandbox-friendly per-sequence substitute
   for desktop's whole-folder backup/restore.
5. ✅ **Render Mode** (P3) and **Read-only guard** / **Change Show Folder Temporarily** — **landed
   2026-06-12.** **Pre/Post-ms** re-classified infeasible (desktop FFmpeg audio editor). **Default
   Model Blending for new sequences** remains the one cheap Sequence-Settings gap (`ModelBlendDefaultChoice`).
6. Defer **Data Layers** (impl plan above — gated on a render-engine verification pass),
   **quick-start importers**, **Download Sequences/Lyrics**, and **animated media preview** — all
   hard / low-frequency.
