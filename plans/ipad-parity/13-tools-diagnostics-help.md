# 13 · Tools, Diagnostics & Help

> **Status.** The desktop `Tools` menu (`Menu1` in
> `xLightsMain.cpp:1098-1160`) has ~30 entries; the iPad `Tools`
> `CommandMenu` (`XLightsCommands.swift:290-375`) exposes ~12. **Batch
> Render**, **Export Models**, and effect-format **Convert/Import** all
> exist on iPad — on a different surface (sequence-picker toolbar,
> Layout-Editor, and the Import sheet respectively) rather than in the
> Tools menu. iPad's **Tools → AI Services…** is the *AI-service-config*
> panel (= desktop **Preferences → AI Services**), not the desktop
> Tools → *Generate AI Image* / *Generate Lyrics From Data* tools — those
> two distinct generators are missing or only effect-embedded on iPad.
> The Help menu is at parity on external links; **Tip of the Day**,
> **Key Bindings**, and **Content/Help Contents** are now ✅ on iPad
> too, leaving only **Check for Updates** (App-Store-restricted).
> Remaining real iPad gaps are the controller/hardware specialists
> (Pixel Test, Bulk Upload, HinksPix, Export Controller Connections),
> the legacy generators (2D Path, Custom Model generate/remap, Generate
> Lyrics From Data), and the FFmpeg-bound Prepare Audio + sandbox-blocked
> Run Scripts. File-sweep utilities Cleanup File Locations + Search Show
> Folders are ✅, and **Download Sequences/Lyrics is now ✅** — a new
> wx-free core `music_catalog::Catalog` (parallel to `vendor_catalog`)
> parses the music-vendor schema and the iPad `MusicBrowserSheet`
> downloads sequences/lyrics into the show folder. The two diagnostic
> windows **Color Dropper** and
> **Find Effect Data**, plus the **User Lyric Dictionary** editor, are
> now ✅ on iPad too. Diagnostics (spdlog
> rotation, package-logs, crash auto-upload to the shared triage
> endpoint) are at parity, with iPad slightly ahead via automatic
> MetricKit capture + Settings opt-in.

## Parity scorecard

| Feature | Surface | Desktop | iPad | Gap | Priority | Ease | Feasibility | Notes |
|---|---|---|---|---|---|---|---|---|
| Tools → Test (Pixel/Light Test) | menu | ✅ | ❌ | ipad-missing | P2 | hard | hard | `xLightsMain.cpp:1099`, `PixelTestDialog`. Direct controller output; no raw USB/serial/output-manager driving on iOS sandbox. Use FPP path instead. |
| Tools → Check Sequence | menu | ✅ | ✅ | parity | P1 | easy | feasible | `xLightsMain.cpp:1102` / `CheckSequenceSheet.swift` (406 lines). Shared core `CheckSequence`, issues grouped by severity/section, tap row → jump grid+playhead. |
| Tools → Cleanup File Locations | menu | ✅ | ✅ | parity | P2 | medium | feasible | Desktop `xLightsMain.cpp:6451`, `OnMenuItem_CleanupFileLocationsSelected`. iPad Tools → "Cleanup File Locations…" (`XLightsCommands.swift`) → `CleanupFileLocationsSheet` previews the external files that would move, then `performCleanupFileLocations` bridge sweeps them into the show folder + rewrites effect references (`XLSequenceDocument.mm` `cleanupExternalMedia`). Non-undoable; the sheet warns (matches desktop). |
| Tools → Package Sequence | menu | ✅ | ✅ | parity | P1 | easy | feasible | `xLightsMain.cpp:1106` / `PackageSequenceSheet.swift`, `XLSequencePackager`. Builds `.xsqz` of sequence+media. |
| Tools → Download Sequences/Lyrics | menu | ✅ | ✅ | parity | P2 | hard | feasible | **Landed.** Desktop `xLightsMain.cpp:1108` → `VendorMusicDialog` (`MSLVendor` / `MSLSequenceLyric`) is wx-only and the shared `vendor_catalog::Catalog` is models-only. New wx-free core `src-core/import_export/MusicCatalog.{h,cpp}` (`music_catalog::Catalog`) parallels VendorCatalog: parses the `<musicvendor>` master-index entries + per-vendor `<musicinventory>`/`<song>` schema (sequence/lyric items), with `DownloadTo` landing files in the show folder (sequences `.zip`, lyrics `.xtiming`). Bridge `XLMusicCatalog.{h,mm}` (`loadWithProgress`, `downloadItemFromURL:fileName:destFolder:`). iPad Tools → "Download Sequences / Lyrics…" → `MusicBrowserSheet.swift` (vendors → items, search, download into show folder). Schema knowledge is duplicated from desktop's `MSL*` (not yet unified — same situation VendorCatalog started in). |
| Tools → Batch Render | menu (desktop) / picker toolbar (iPad) | ✅ | ✅ | parity | P1 | easy | feasible | `xLightsMain.cpp:1111`; iPad `BatchRenderSheet.swift` (326) + `BatchRenderRunner.swift`, launched from sequence-picker toolbar (`XLightsApp.swift:1070,1122`). Real multi-seq queue. Surface differs, capability present. |
| Tools → FPP Connect | menu | ✅ | ✅ | parity | P1 | easy | feasible | `xLightsMain.cpp:1113` / `FPPConnectSheet.swift` (1314), `FPPConnectMenuItem`. Discover FPP + upload `.fseq`. Open-firmware path. |
| Tools → Bulk Controller Upload | menu | ✅ | ❌ | ipad-missing | P3 | hard | restricted | `xLightsMain.cpp:1115`. Batch firmware/config to many controllers; vendor-closed firmwares are IAP-gated. Open-firmware (FPP/WLED/ESPixelStick) subset is in-scope but low priority. |
| Tools → HinksPix Export | menu | ✅ | ❌ | ipad-missing | P3 | medium | restricted | `xLightsMain.cpp:1117`, `HinksPixExportDialog`. HinksPix is closed firmware → restricted/IAP. |
| Tools → Run Scripts | menu | ✅ | ❌ | ipad-missing | P3 | hard | infeasible | `xLightsMain.cpp:1119`. Executes user automation scripts; iOS sandbox forbids arbitrary script/process execution. |
| Tools → Export Models | menu (desktop) / Layout Editor (iPad) | ✅ | ✅ | parity | P2 | easy | feasible | `xLightsMain.cpp:1122`; iPad `exportModelsReport(toPath:)` (`XLSequenceDocument.mm:15005`) → `.xlsx`, surfaced in `LayoutEditorView.swift:1671`. Surface differs. |
| Tools → Export Effects | menu | ✅ | ✅ | parity | P2 | easy | feasible | `xLightsMain.cpp:1124` / `XLSequenceDocument.mm:exportEffectsReport(toPath:)` (EFX-1). Core `src-core/import_export/ExportEffectsReport.cpp`; CSV format matching desktop. `XLightsCommands.swift` "Export Effects…" → fileExporter in `SequencerView.swift`. |
| Tools → Export Controller Connections | menu | ✅ | ❌ | ipad-missing | P3 | medium | feasible | `xLightsMain.cpp:1126`. CSV of controller/output mapping. Generic but model+output-manager bound; low demand on iPad. |
| Tools → View Log | menu | ✅ | ✅ | parity | P1 | easy | feasible | `xLightsMain.cpp:1129` / `LogViewerSheet.swift` (275). Live tail with level/logger/text filter. Same spdlog file. |
| Tools → Package Log Files | menu | ✅ | ✅ | parity | P1 | easy | feasible | `xLightsMain.cpp:1131` / `packageLogs()` + `XLLogPackager.mm`. iPad bundles logs+config+seq+device info+MetricKit; desktop bundles logs+config+seq. |
| Tools → Purge Render Cache | menu | ✅ | ✅ | parity | P2 | easy | feasible | `xLightsMain.cpp:1135` / `purgeRenderCache()`. |
| Tools → Purge Download Cache | menu | ✅ | ✅ | parity | P2 | easy | feasible | `xLightsMain.cpp:1133` (`iD_MNU_VENDORCACHEPURGE`) / `purgeDownloadCache()`. |
| Tools → Crash xLights | menu (hidden in release) | 🟡 | ❌ | ipad-missing | P3 | easy | feasible | `xLightsMain.cpp:1137`; removed from menu unless `EnableCrash` special option set (`:2074-2079`). Debug-only; low value to port. |
| Tools → Log Render State | menu (hidden in release) | 🟡 | ❌ | ipad-missing | P3 | easy | feasible | `xLightsMain.cpp:1139`; also removed unless `EnableCrash` set. Internal diagnostic dump. |
| Tools → Generate 2D Path | menu | ✅ | ❌ | ipad-missing | P3 | hard | feasible | `xLightsMain.cpp:1142` → `PathGenerationDialog` (`src-ui-wx/model/PathGenerationDialog.cpp`). NOT a model generator — it's a freehand 2D-path sketch tool that exports a **pair of `.xvc` value-curve files** (`NameX.xvc` + `NameY.xvc`, X/Y coordinate vs. normalized arc-length) optionally traced over a reference image, with point insert/drag/delete/undo + flip-X/flip-Y/rotate transforms. Deferred: a full freehand drawing canvas with magnetic snapping + per-axis `.xvc` save is substantial SwiftUI-canvas work for a low-mobile-demand P3 specialist. iPad already has `ValueCurveEditor`/`ValueCurveCanvases` + the Sketch path editor as nearby building blocks for a future port. |
| Tools → Generate Custom Model | menu | ✅ | ❌ | ipad-missing | P3 | hard | feasible | `xLightsMain.cpp:1144`, `GenerateCustomModelDialog`. Video/photo-driven custom-model builder; camera+dialog-heavy. |
| Tools → Remap Custom Model | menu | ✅ | ❌ | ipad-missing | P3 | hard | feasible | `xLightsMain.cpp:1146`. Node-index remap; specialist, dialog-heavy. |
| Tools → Generate AI Image | menu (desktop) / effect-picker (iPad) | ✅ | ✅ | parity | P2 | medium | feasible | `xLightsMain.cpp:1148`, `AIImageDialog`. iPad `AIImageGenerationSheet.swift` now also reachable from Tools menu (AI-2): `XLightsCommands.swift` "Generate AI Image…" → `showingStandaloneAIImage` → `SequencerView.swift` sheet. Saves to `AIImages/` show subfolder; no effect context needed. Gated on `XLAIServices.hasEnabledService(forCapability: XLAICapabilityImages)`. |
| Media Manager → AI Generate Image | media panel | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop media manager has an `AI Generate…` button (`ManageMediaPanel.cpp:633` `_aiGenerateButton`, gated at `:718-719`/`:926` on image+hasAI, `:2237` `OnAIGenerateButtonClick` → `AIImageDialog` at `:2261`). iPad has the `AIImageGenerationSheet` but no media-manager AI-generate entry. |
| Tools → Generate Lyrics From Data | menu | ✅ | ❌ | ipad-missing | P3 | hard | feasible | `xLightsMain.cpp:1150`, `GenerateLyricsDialog` (`OnMenuItem_GenerateLyricsSelected:7233`). Maps channel-data → face/phoneme tracks — *distinct* from iPad's AI speech-to-lyrics. Legacy; low demand. |
| Tools → Convert | menu (desktop) / Import sheet (iPad) | ✅ | 🟡 | ipad-missing | P2 | medium | feasible | `xLightsMain.cpp:1152`, `TabConvert`/`ConvertDialog`. iPad `ImportEffectsView.swift` already does `.xsq/.xsqz`, SuperStar `.sup`, LOR S5 `.loredit`, Vixen 3. Missing legacy formats (LMS/LSP/Vixen2/HLS/Glediator) + media transcode. |
| Tools → Prepare Audio | menu | ✅ | ❌ | ipad-missing | P2 | hard | hard | `xLightsMain.cpp:1154` (`OnMenuItem_PrepareAudioSelected:8031`). Resample/normalize via FFmpeg; FFmpeg-bound core excluded from iPad build. AVFoundation reimpl possible but heavy. |
| Tools → User Lyric Dictionary | menu | ✅ | ✅ | parity | P3 | medium | feasible | `xLightsMain.cpp:1156` / `LyricUserDictDialog`. iPad Tools → "User Lyric Dictionary…" → `UserLyricDictionarySheet.swift` lists/adds/edits/deletes word→phoneme entries, writing the show folder's `user_dictionary` in the desktop line format (`WORD PH1 PH2 …`, uppercased) via bridge `userLyricDictionaryEntries` / `saveUserLyricDictionaryEntries:` (`XLSequenceDocument.mm`). Save also calls `iPadRenderContext::ReloadPhonemeDictionary` so the core breakdown (`PhonemeDictionary`) picks edits up immediately. |
| Tools → Search for Show Folders | menu | ✅ | ✅ | parity | P2 | medium | feasible | `xLightsMain.cpp:1158` / `ShowFolderSearchDialog` (desktop sweeps the drive / log history). iPad Tools → "Search for Show Folders…" → `SearchShowFoldersSheet.swift`: the user picks a folder tree (security-scoped `ShowFolderPicker`) and it recurses for directories containing `xlights_rgbeffects.xml` (the show-folder marker), listing each with a tap-to-switch that registers a bookmark + calls `loadShowFolder`. Surface differs (no drive to sweep on iOS) but the capability is present. |
| Tools → Import Effects | menu (Import on desktop) | ✅ | ✅ | parity | P1 | easy | feasible | desktop `Menu2`/`ID_IMPORT_EFFECTS` (`:1265`); iPad `ImportEffectsView.swift` (642). Mapping + render-style options on both. |
| Tools → Edit Layout | menu (iPad) / Layout tab (desktop) | 🟡 | ✅ | desktop-missing | P3 | medium | feasible | iPad `EditLayoutMenuItem` opens a detached `WindowGroup`. Desktop reaches Layout via the `Layout` notebook tab (`xLightsMain.cpp:997`), not a Tools menu item — surface difference, not a true capability gap. |
| View → Windows → Color Dropper | menu (toggle) / preview eyedropper (iPad) | ✅ | ✅ | parity | P2 | medium | feasible | `xLightsMain.cpp:1204` (`ID_MNU_COLOURDROPPER`, checkable). Desktop docks an eyedropper that samples preview pixels. iPad: an eyedropper button in `PreviewControlsOverlay` (`HousePreviewView.swift`) arms a one-shot dropper; the next tap on the House/Model preview routes through `PreviewPaneView.handleSingleTap` → bridge `sampledColorHexNearPoint:viewSize:forDocument:` (`XLMetalBridge.mm`), which maps the tap to the nearest model node (`GetNodeNear`) and returns that node's current rendered colour (`GetNodeColor`) — a node-colour read, not a GPU pixel read-back, so no drawable stall. The sample is pushed into `XLRecentColors` with a transient banner. 2D-only (matches the node-pick path). |
| View → Windows → Find Effect Data | menu (toggle) / Tools sheet (iPad) | ✅ | ✅ | parity | P2 | medium | feasible | `xLightsMain.cpp:1218` (`ID_MNU_FINDDATA`), `FindDataPanel` + `SearchPanel`. iPad Tools → "Find Effect Data…" → `FindEffectDataSheet.swift`: query by effect-type picker (populated from `effectTypesInSequence`), settings-text substring, and model-name substring; bridge `findEffectsMatching(type:settingsText:modelFilter:maxResults:)` (`XLSequenceDocument.mm`) walks model/submodel/strand/node layers (mirrors SearchPanel coverage) matching on `key=value` settings + palette entries, returning `XLFindEffectResult` rows (effect / element / layer / start / matched-setting). Tapping a result reuses `SequencerViewModel.jumpToEffect` (same path CheckSequence uses) to select + seek. iPad Find/Replace (`FindReplaceSheet`) still covers timing-mark labels separately. |
| Help → Tip of the Day | menu | ✅ | ✅ | parity | P3 | easy | feasible | **Landed.** Desktop `xLightsMain.cpp:1269`, `TipOfTheDayDialog` reads `TipOfDay/tod.xml` (`<tip url title category level exclude>`) + per-tip HTML from the GitHub TipOfDay folder. iPad: bridge `XLTipOfDay.{h,mm}` fetches the same `tod.xml` + tip HTML via `CachedFileDownloader` (skipping `exclude="OSX"` tips, shared by Apple platforms); Help → "Tip of the Day…" → `TipOfDaySheet.swift` renders the HTML in a `WKWebView`, with a "Next Tip" button and a "Show tips on startup" toggle (`TipOfDayPrefs`, UserDefaults `xl.showTipAtStartup`, default on). Auto-shown once per launch from `XLightsApp` `.task` when the pref is on. |
| Help → User Manual | menu | ✅ | ✅ | parity | P1 | easy | feasible | `xLightsMain.cpp:1271` → manual.xlights.org; iPad `XLightsCommands.swift:544`. |
| Help → Zoom Room Help | menu | ✅ | ✅ | parity | P1 | easy | feasible | `xLightsMain.cpp:1273`; iPad `:578`. |
| Help → Key Bindings | menu | ✅ | ✅ | parity | P2 | medium | feasible | `xLightsMain.cpp:1275`. iPad: Help ▸ "Key Bindings…" → `KeyBindingsSheet.swift` (grouped read-only reference of the XLightsCommands bindings; complements the system ⌘-hold HUD). |
| Help → Content (F1) | menu/shortcut | ✅ | ✅ | parity | P3 | easy | feasible | **Landed.** Desktop `xLightsMain.cpp:1277` (`Content\tF1`) opens the manual. No F1 on iPad keyboards, so iPad Help → "Help Contents" (`XLightsCommands.swift`) opens the same manual landing page (`https://manual.xlights.org/`). Surface differs (no F1 accelerator); capability present. |
| Help → Forum | menu | ✅ | ✅ | parity | P1 | easy | feasible | `xLightsMain.cpp:1280`; iPad `:558`. |
| Help → Video Tutorials | menu | ✅ | ✅ | parity | P1 | easy | feasible | `xLightsMain.cpp:1281` → videos.xlights.org; iPad `:547`. |
| Help → Download / Website | menu | ✅ | ✅ | parity | P3 | easy | feasible | Desktop `Download` (`:1283`) opens xlights.org; iPad `xLights Website` (`:570`) opens same URL. Same target, different label; no "installer download" needed on App Store. |
| Help → Release Notes | menu | ✅ | ✅ | parity | P1 | easy | feasible | `xLightsMain.cpp:1285`; iPad `:550` (version-pinned README.txt). |
| Help → Issue Tracker | menu | ✅ | ✅ | parity | P1 | easy | feasible | `xLightsMain.cpp:1287`; iPad `:567`. |
| Help → Facebook | menu | ✅ | ✅ | parity | P1 | easy | feasible | `xLightsMain.cpp:1289`; iPad `:561`. |
| Help → Donate | menu | ✅ | ✅ | parity | P1 | easy | feasible | `xLightsMain.cpp:1291`; iPad `:581`. |
| Help → Check for Updates | menu | 🟡 | ❌ | ipad-missing | P3 | easy | infeasible | `xLightsMain.cpp:1293`, item created **disabled** (`:1295 Enable(false)`). App Store manages iPad updates; in-app check not allowed. |
| Help → About | menu | ✅ | ✅ | parity | P1 | easy | feasible | `xLightsMain.cpp:1296`, `OnAbout`; iPad `AboutSheet.swift` / `showingAbout`. |
| Diagnostics: spdlog rotation | other | ✅ | ✅ | parity | P1 | easy | feasible | Same rotating-file config in core + `XLiPadInit.mm`. |
| Diagnostics: crash capture + auto-upload | other | ✅ | ✅ | parity | P1 | easy | feasible | Desktop uploads to dankulp.com/crashUpload; iPad `XLDiagnosticUploader.swift` + MetricKit auto-stages & POSTs to the same endpoint, with Settings opt-in (`XLSendCrashReports`). iPad slightly ahead (automatic MetricKit). |
| Tools → Plugins (xLights UI Plugins - dockable panels/menu entries) | menu | ✅ | ❌ | ipad-missing | P3 | hard | infeasible | `XLightsPluginManager`/`XLightsPluginHost` (2026-07 addition) - loads DLLs exporting `IXLightsPlugin` at startup (after `CreateSequencer()`), each registering a dockable AUI pane + a `Tools > Plugins` menu toggle via a curated abstract widget facade (`IPluginPanel`/`IPluginListBox`/etc. - never real wx types, to avoid a second non-communicating copy of wx's statically-linked runtime). Doubly infeasible on iPad: same App Store code-loading restriction as the existing AI/effect plugin rows, **and** there is no wxWidgets on iPad at all to back the widget facade even if code-loading were allowed. |

## iPad gaps (desktop has, iPad missing)

### P1
None — every P1 Tools/Help/diagnostics surface is at parity (Check
Sequence, Package Sequence, Batch Render, FPP Connect, View Log, Package
Logs, Import Effects, all external Help links, About, crash capture).

### P2
- ✅ **Cleanup File Locations** — **landed 2026-06-11.** Desktop
  `xLightsMain.cpp:6451` → `OnMenuItem_CleanupFileLocationsSelected`
  sweeps every referenced media file under the show folder. iPad now
  has Tools → "Cleanup File Locations…" (`XLightsCommands.swift`) →
  `CleanupFileLocationsSheet`: the `cleanupFileLocationsPreview` bridge
  op lists every external file that would move ({from, to}); confirming
  runs `performCleanupFileLocations`, which copies each file outside the
  show/media folders into the show folder (type-canonical subdir, `_N`
  on collision) and rewrites the effect references
  (`XLSequenceDocument.mm` `cleanupExternalMedia`). The operation is NOT
  undoable and the sheet warns — matching desktop, which warns rather
  than registering an undo step.
- **Convert (full)** — desktop `TabConvert`/`ConvertDialog`
  (`:1152`). iPad `ImportEffectsView` already covers `.xsq/.xsqz`,
  SuperStar, LOR S5, Vixen 3; gap is the legacy formats
  (LMS/LSP/Vixen2/HLS/Glediator/LCB) + media transcode. Add readers to
  `XLImportSession` and entries to the import sheet. Medium.
- ✅ **Search for Show Folders** — **landed.** `xLightsMain.cpp:1158`.
  iPad `SearchShowFoldersSheet.swift` picks a security-scoped folder
  tree and recurses for `xlights_rgbeffects.xml`, tap-to-switch via
  `loadShowFolder`.
- ✅ **Download Sequences/Lyrics** — **landed.** New wx-free core
  `src-core/import_export/MusicCatalog.{h,cpp}` (`music_catalog::Catalog`)
  parallels `vendor_catalog::Catalog`: parses the `<musicvendor>`
  master-index entries + per-vendor `<musicinventory>`/`<song>` schema
  (the wx-only `MSLVendor`/`MSLSequenceLyric` knowledge, duplicated into
  core — not yet unified, same path VendorCatalog took). Bridge
  `XLMusicCatalog.{h,mm}`; iPad Tools → "Download Sequences / Lyrics…"
  → `MusicBrowserSheet.swift` browses vendors → items and downloads
  straight into the show folder (sequences `.zip`, lyrics `.xtiming`).
- **Prepare Audio** — `xLightsMain.cpp:1154`. FFmpeg-bound on desktop;
  see Infeasible/restricted (AVFoundation reimpl is heavy). hard.
- ✅ **Color Dropper** — **landed.** Desktop toggle
  `ID_MNU_COLOURDROPPER` (`:1204`). iPad eyedropper button in
  `PreviewControlsOverlay`; one-shot tap → bridge
  `sampledColorHexNearPoint` (node-colour read, not pixel read-back) →
  `XLRecentColors`.
- ✅ **Find Effect Data** — **landed.** Desktop `FindDataPanel` /
  `SearchPanel` (`:1218`). iPad `FindEffectDataSheet.swift` +
  `findEffectsMatching(type:settingsText:modelFilter:maxResults:)`
  bridge op; tap → `jumpToEffect`.
- ~~**Key Bindings (Help)**~~ — **landed 2026-06-11**: `KeyBindingsSheet.swift` grouped reference, Help-menu entry.

### P3
- **Bulk Controller Upload** (restricted for closed firmware),
  **HinksPix Export** (restricted), **Run Scripts** (infeasible),
  **Export Controller Connections**, **Generate 2D Path** (the
  `.xvc`-pair freehand-path sketch tool — substantial canvas work,
  deferred), **Generate Custom Model**, **Remap Custom Model**,
  **Generate Lyrics From Data**,
  **Crash xLights** / **Log Render State** (hidden debug),
  **Check for Updates** (infeasible),
  **Media Manager → AI Generate Image** (`ManageMediaPanel.cpp:633`).
  **Tip of the Day** and **Content/Help Contents** are now ✅.
  See scorecard for refs/reasons.

## Desktop gaps (iPad has, desktop missing)

- **Edit Layout in Tools menu** — iPad surfaces Layout via a Tools-menu
  `EditLayoutMenuItem` opening a detached window; desktop only exposes
  Layout through the `Layout` notebook tab (`xLightsMain.cpp:997`). This
  is a surface/idiom difference, not a true capability gap — desktop
  fully supports layout editing. P3, optional. (Adding a desktop Tools
  entry that switches to the Layout tab would mirror the iPad path.)
- **Automatic MetricKit crash staging + Settings opt-in** — iPad
  `XLDiagnosticUploader.swift` auto-stages MetricKit payloads and POSTs
  to the shared triage endpoint, honoring an `XLSendCrashReports`
  toggle. Desktop relies on the user invoking Package Log Files. Not a
  menu item; noted as iPad-ahead diagnostics. No desktop work proposed.

## Infeasible / restricted on iPad

- **Tools → Test / Pixel Test** — needs direct controller output
  (raw USB/serial/output-manager driving); the iOS sandbox has no raw
  device I/O. (hard, not strictly impossible via FPP, but not the same
  feature.)
- **Tools → Run Scripts** — iOS forbids executing arbitrary
  scripts/processes. Infeasible.
- **Tools → Prepare Audio** — desktop uses FFmpeg, which is excluded
  from the iPad build (`project_ffmpeg_core_ipad_exclusion`). An
  AVFoundation reimplementation is possible but substantial. (hard.)
- **Tools → Bulk Controller Upload / HinksPix Export** — closed-firmware
  controller config/upload is IAP-gated and low priority
  (**restricted**, P3). The open-firmware subset (FPP/WLED/ESPixelStick)
  of bulk upload is in-scope but deferred.
- **Help → Check for Updates** — App Store owns iPad updates; an in-app
  updater is not permitted. Infeasible (the desktop item is itself
  disabled by default).
- **Help → Content (F1)** — no F1 key on iPad keyboards; would need a
  remapped Help sheet instead.
- **Tools → Plugins (xLights UI Plugins)** — `XLightsPluginManager` loads
  native DLLs at startup, same App Store restriction as AI/effect plugin
  loading (theme 04/12). Additionally infeasible independent of that
  restriction: the plugin UI facade (`IPluginPanel` etc.) is implemented by
  wrapping real wxWidgets objects the host constructs, and there is no
  wxWidgets on iPad - the abstraction has nothing to wrap there even
  setting aside code-loading policy.

## Recommended sequencing

1. ~~**Cheap parity wins first.**~~ **Landed.** **Generate AI Image** Tools-menu
   entry (`XLightsCommands.swift`, `SequencerView.swift:showingStandaloneAIImage`) and
   **Export Effects** (`src-core/import_export/ExportEffectsReport.cpp`, bridge
   `exportEffectsReport(toPath:)`, `SequencerView.swift` fileExporter) are both ✅.
2. ~~**Cleanup File Locations**~~ **Landed 2026-06-11.** Tools menu entry
   + preview/confirm sheet (`CleanupFileLocationsSheet`) + bridge
   `cleanupFileLocationsPreview` / `performCleanupFileLocations`;
   complements the existing per-file relocation.
3. **Convert breadth** — extend `XLImportSession`/`ImportEffectsView`
   with the remaining legacy readers so "Convert" reaches real parity.
4. ~~**Diagnostic windows**~~ **Landed.** Color Dropper
   (`PreviewControlsOverlay` eyedropper → `sampledColorHexNearPoint`
   bridge → `XLRecentColors`) and Find Effect Data
   (`FindEffectDataSheet` + `findEffectsMatching…` bridge →
   `jumpToEffect`). **Search for Show Folders**
   (`SearchShowFoldersSheet`, scoped `xlights_rgbeffects.xml` scan) and
   the **User Lyric Dictionary** editor (`UserLyricDictionarySheet` →
   `user_dictionary` rewrite + `ReloadPhonemeDictionary`) landed
   alongside.
5. ~~**Help polish**~~ **Landed.** Key Bindings sheet, **Tip of the
   Day** (`TipOfDaySheet` + `XLTipOfDay` bridge, startup pref), and
   **Help Contents** (manual link, replacing desktop's F1). Only
   Check-for-Updates remains deferred (App-Store-restricted).
6. ~~**Download Sequences/Lyrics**~~ **Landed.** Core
   `music_catalog::Catalog` + `XLMusicCatalog` bridge + `MusicBrowserSheet`,
   downloading into the show folder.
7. **Defer** the controller-specialist (Bulk Upload, HinksPix, Export
   Controller Connections), legacy generators (2D Path — the `.xvc`-pair
   freehand-path sketch tool, Custom Model gen/remap, Generate Lyrics
   From Data), FFmpeg-bound Prepare Audio, and sandbox-blocked Run
   Scripts — all P3/restricted/heavy.
