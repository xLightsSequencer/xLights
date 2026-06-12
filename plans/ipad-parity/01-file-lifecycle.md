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
 > **iPad gaps** are the desktop **Backup / Alternate Backup /
> Restore Backup** family (Revert To → Last Saved now at parity), plus several **Sequence Settings
> Info-tab fields** (Render Mode, Hash, Pre/Post-milliseconds, Overwrite
> Tags, editable duration), **Data Layers**, the **New-Sequence wizard
> Metadata + Timings steps**, **quick-start importers**, and **Export
> House Preview Video** / **Download Sequences/Lyrics**. The real
> **desktop "gap"** is mostly platform-idiomatic iPad UX (autosave
> recovery alert, iCloud download badges, Media Relocation) rather than
> missing desktop capability. *File Encoding (UTF-8/ANSI)* and *Reopen
> Last Sequence preference* do not exist in the desktop code.

## Parity scorecard

| Feature | Surface | Desktop | iPad | Gap | Priority | Ease | Feasibility | Notes |
|---|---|---|---|---|---|---|---|---|
| New Sequence | menu/toolbar | ✅ | ✅ | parity | P1 | easy | feasible | Desktop wizard (≤5 steps) vs iPad 3-step wizard (type/content/save). |
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
| Close Sequence | menu/shortcut | ✅ | ✅ | parity | P1 | easy | feasible | iPad ⌘W; closeRequestToken → prompt. |
| Unsaved-changes prompt on close | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop SaveChangesDialog; iPad confirmationDialog (Save/Discard/Cancel). |
| Unsaved-changes handling on quit/background | dialog | ✅ | 🟡 | parity | P1 | easy | feasible | Desktop CheckUnsavedChanges prompt; iPad silently saves on `.inactive`/`.background` (Stage-Manager pill can't host an alert). |
| Sequence Settings dialog/sheet | menu/toolbar | ✅ | ✅ | parity | P1 | easy | feasible | Tabs differ (see below). |
| Settings — Info tab | dialog tab | ✅ | ✅ | parity | P1 | easy | feasible | iPad info is read-only subset (filename/version/duration/frame/model count) + type + audio swap. |
| Settings — Metadata tab | dialog tab | ✅ | ✅ | parity | P1 | easy | feasible | song/artist/album/author/email/website/url/comment. |
| Settings — Timings tab | dialog tab | ✅ | ✅ | parity | P1 | medium | feasible | iPad fully built (add/rename/delete/export single+multi/import 9 formats incl. multi-track Vixen3/xsq). |
| Settings — Audio Tracks tab | dialog tab | ✅ | ✅ | parity | P1 | easy | feasible | alt-track add/remove/rename/replace. |
| Settings — Media tab | dialog tab | ✅ | ✅ | parity | P1 | easy | feasible | per-effect media inventory embed/extract/rename/replace. |
| Settings — Render tab | dialog tab | ✅ | ✅ | parity | P1 | easy | feasible | iPad: model blending + frame interval + autosave interval picker. |
| Settings — Data Layers tab | dialog tab | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop tree importer; absent on iPad. |
| Settings — Render Mode selector | dialog | ✅ | ❌ | ipad-missing | P3 | medium | feasible | `RenderModeChoice`; iPad uses default. |
| Settings — Hash/Checksum display | dialog | ✅ | ✅ | parity | P3 | easy | feasible | Desktop `TextCtrl_Hash` (media hash); iPad Info-tab "Media Hash" row → bridge `mediaFileHash` (`AudioManager::Hash`). |
| Settings — Pre/Post milliseconds | dialog | ✅ | ❌ | ipad-missing | P3 | medium | feasible | global timing shift; not on iPad. |
| Settings — Overwrite Tags on media change | dialog | ✅ | ✅ | parity | P3 | easy | feasible | iPad Info-tab toggle (`overwriteTagsOnMediaChange` @AppStorage, default off): on audio swap, pulls Song/Artist/Album from the new file's tags via `audioTitle/Artist/Album`. |
| Settings — editable Duration override | dialog | ✅ | ✅ | parity | P3 | easy | feasible | Desktop `TextCtrl_Xml_Seq_Duration`; iPad Info-tab editable Duration field → bridge `setSequenceDurationMS` (resizes render data via `EnsureSequenceDataSized`). |
| Select Show Folder | menu / sheet | ✅ | 🟡 | ipad-weaker | P1 | easy | feasible | iPad FolderConfigView (gate-required before sequences); 'Change Show Folder…' button at `FolderConfigView.swift:90`, commit via `viewModel.loadShowFolder` at :400, but no validation/heuristic on the chosen folder. |
| Recent Show Folders | menu / sheet | ✅ | ✅ | parity | P1 | easy | feasible | iPad list w/ swipe-delete; tap-to-switch (apply on Done). |
| Change Show Folder Temporarily | menu / sheet | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop `xLightsMain.cpp:894` (`Button_CheckShowFolderTemporarily` "Change Temporarily"), :896 (`Button_ChangeTemporarilyAgain`); no iPad analogue in FolderConfigView. |
| Read-only mode guard | app/state | ✅ | ❌ | ipad-missing | P3 | easy | feasible | Desktop gates edits on `readOnlyMode` (`xLightsMain.cpp:9119-9120`); iPad has no read-only guard (only the J-0 layout-editor "read-only" display). |
| Base Show Folder (inheritance) | setup/sheet | ✅ | ✅ | parity | P2 | medium | feasible | Desktop Setup tab (`Button_ChangeBaseShowDir`); iPad FolderConfigView base section + Auto-Update + Update Now. |
| Media Folders configuration | preference/sheet | ✅ | ✅ | parity | P2 | medium | feasible | Desktop has media dirs concept; iPad FolderConfigView media-folder list. |
| Media Relocation on file pick | gesture/dialog | ❌ | ✅ | desktop-missing | P2 | medium | feasible | iPad enforces "copy under show/media folder"; desktop free path + Cleanup later. |
| Add media to sequence (multi-file) | manager/sheet | ✅ | ✅ | parity | P1 | medium | feasible | Desktop `ManageMediaPanel.cpp:2004` (`_addButton` "Add", `wxFD_MULTIPLE`); iPad MediaManagerSheet "Add…" toolbar button → multi-select `.fileImporter` → relocate under show folder + `addMedia(atPath:)` bridge (`MediaManagerSheet.swift` `addPickedFiles`; `XLSequenceDocument.mm` `addMediaAtPath:`). |
| Bulk find missing media | manager/sheet | ✅ | ✅ | parity | P2 | medium | feasible | iPad MediaManagerSheet "Bulk Find Missing…" menu action → folder `.fileImporter` → one-pass basename index → relink each missing entry via `replaceMissingMedia` (`MediaManagerSheet.swift` `bulkFindMissing(in:)`). |
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
| Restore Backup dialog | menu/dialog | ✅ | ❌ | ipad-missing | P2 | hard | feasible | `RestoreBackupDialog` (pick date, restore layouts/sequences). Could be reimplemented for in-show backup dirs — a future iPad Restore-Backup UI can consume the timestamped snapshots the new Backup-On-Save toggle writes into `<show>/Backup/` (`SequencerViewModel.writeSaveBackup`). |
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

- ✅ **Bulk find missing media** — **landed 2026-06-11.** iPad `MediaManagerSheet`'s overflow menu has
  "Bulk Find Missing…" → folder `.fileImporter`; one security-scoped pass builds a lowercased-basename
  index of the folder tree, then each missing inventory entry is relinked through the existing
  `replaceMissingMedia` primitive (`src-iPad/App/MediaManagerSheet.swift` `bulkFindMissing(in:)`).
  Reports "Relinked N, still missing M".

- **Restore Backup dialog** — desktop `src-ui-wx/app-shell/RestoreBackupDialog.cpp` + menu wiring at
  `src-ui-wx/xLightsMain.cpp:1062` (`ID_FILE_RESTOREBACKUP`). Lets the user pick a dated backup and
  restore layouts/sequences. iPad has **no restore UI**. *Work:* a SwiftUI sheet enumerating an
  in-show `Backup/` directory + a bridge method to copy a chosen backup `.xsq` over the current one;
  the whole-folder restore is harder under the sandbox. *Ease: hard.*
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
  2026-06-11** (bridge `mediaFileHash` / `setSequenceDurationMS` + Info-tab rows + audio-swap
  tag overwrite toggle). Still absent: **Render Mode** (`RenderModeChoice`) and
  **Pre/Post-milliseconds** (`TextCtrl_Premilliseconds` / `_Postmilliseconds`) — both *medium*
  (need re-render semantics). P3.
- **Data Layers tab** — desktop `Panel_DataLayers` tree importer. iPad absent. *Hard, P3.*
- **Default Model Blending for new sequences** preference (`ModelBlendDefaultChoice`). iPad only sets
  blending per-sequence in the Render tab. *Easy, P3.*
- **Download Sequences/Lyrics** (`ID_MNU_DOWNLOADSEQUENCES`). iPad has a vendor browser for models but
  not for sequences/lyrics. *Hard, P3.*
- **Reopen last sequence on launch** — deliberately omitted on iPad to dodge the 20-second launch
  watchdog when the show folder is iCloud-evicted (see `XLightsApp.swift:61` comment). Re-enabling
  would need a fully-async open-after-launch. *Medium, P3.*
- **Change Show Folder Temporarily** — desktop `src-ui-wx/xLightsMain.cpp:894`
  (`Button_CheckShowFolderTemporarily`, "Change Temporarily") + :896 (`Button_ChangeTemporarilyAgain`).
  iPad's FolderConfigView has only permanent switching. *Medium.*
- **Read-only mode guard** — desktop gates edits on `readOnlyMode` (`src-ui-wx/xLightsMain.cpp:9119-9120`).
  iPad has no read-only guard (only the J-0 layout-editor "read-only" display). *Easy.*
- **Media preview (still + animated)** — iPad shows a static `MediaThumbnailView` with no animation
  (`src-iPad/App/MediaManagerSheet.swift:503-518`; bridge `ensureThumbnailPreviewForPath`,
  `src-iPad/Bridge/XLSequenceDocument.h:2055`). Desktop previews animate. *Medium.*
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
4. **In-show Backup + Restore Backup sheet** (P2, hard) — a sandbox-friendly snapshot of the active
   sequence into an in-show `Backup/` dir, with a SwiftUI restore picker. Substitutes for desktop's
   whole-folder backup/restore within iOS limits.
5. **Render Mode + Default Model Blending + Pre/Post-ms** (P3, medium) — finish Sequence Settings
   parity once the cheap fields land.
6. Defer **Data Layers**, **quick-start importers**, **Download Sequences/Lyrics**, and **Export House
   Preview Video** — all hard and low-frequency.
