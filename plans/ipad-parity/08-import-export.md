# 08 · Import & Export

> Both apps link the same `src-core/import_export/`
> (AutoMapper, EffectMapper, LOREdit, Vixen3, SuperStarImporter,
> TimingImport, MapHintsIO, ExportModels, VendorCatalog), so the *logic*
> for most import/export is already shared — the parity question is UI
> surface + ObjC++ bridge exposure. The iPad's **effect-import** wizard
> (`ImportEffectsView` + `XLImportSession`) is a true peer of the desktop
> `xLightsImportChannelMapDialog`: two-pane mapping, Auto Map, alias
> learning, save/load `.xmaphint`, timing-track selection, FPS/version
> warnings, missing-media report, and the three import options
> (erase/lock/convert-render). The iPad's **timing import** (in
> `SequenceSettingsSheet`) is at full format parity with desktop (xtiming,
> LOR lms/las, Papagayo pgo, SRT, Audacity txt, ElevenLabs json, Vixen3
> tim w/ track-select, LSP msq, xLights xsq). The real gaps are: iPad
> effect-import now covers **LMS/LAS** (shared core `LORMusic`), **LPE**
> (shared core `LORPixelEditor`), **HLS** (shared core `HLSFile`), and
> **Vixen2 vix** (shared core `Vixen2File`, incl. base64 event-stream decode
> + optional `.pro` profile lookup) but is still missing the remaining
> **legacy sequencer formats** (LSP msq-as-effects, VSA); the source-mapping
> list now shows a per-source **effect-count + duration timeline strip**
> (the iPad analogue of the desktop import timeline column) and `.xbkp`
> backups **open through the normal sequence path** (with a Save-As notice;
> the Files "Open in" UTI registration is a deferred submodule plist bump);
> iPad now has **video export**
> (house-preview *and* per-model, via the offscreen `XLHousePreviewVideoExporter`
> and core `ModelVideoExporter` → `VideoWriter`), but still has **no Convert
> tool** and **no HinksPix export**, and per-model non-video export stays
> `.eseq`-only.
> Map From Lights, AI Speech-to-Lyrics, online lyric search, preset
> export, and package exclude-audio all exist on **both** platforms.

## Parity scorecard

| Feature | Surface | Desktop | iPad | Gap | Priority | Ease | Feasibility | Notes |
|---|---|---|---|---|---|---|---|---|
| Import Effects from xLights `.xsq` / `.xml` (legacy) | menu | ✅ | ✅ | parity | P1 | easy | feasible | Shared core. iPad `XLImportSession.loadSourceSequence(atPath:)`. |
| Import Effects from xLights package `.xsqz` / `.zip` / `.piz` | menu | ✅ | ✅ | parity | P1 | easy | feasible | Both use `SequencePackage`; iPad allows these in the import picker. |
| Import Effects from SuperStar `.sup` | menu | ✅ | ✅ | parity | P1 | easy | feasible | iPad `SuperStarImportView` (single-model form) vs desktop multi-model `SuperStarImportDialog`; shared `SuperStarImporter`. |
| Import Effects from LOR S5 `.loredit` | menu | ✅ | ✅ | parity | P1 | easy | feasible | Shared core `LOREdit`. iPad `loadLOREditSource(atPath:)`. |
| Import Effects from Vixen 3 `.tim` (effect-level) | menu | ✅ | ✅ | parity | P1 | easy | feasible | Shared core `Vixen3`. iPad `loadVixen3Source(atPath:)` (needs sibling SystemConfig.xml). |
| Import Effects from LOR Music `.lms` | menu | ✅ | ✅ | parity | P1 | medium | feasible | Shared core `LORMusic` (`src-core/import_export/LORMusic.cpp`, ported from desktop `ImportLMS` `ImportEffects.cpp:1104`). iPad `loadLMSSource(atPath:)` + `ImportEffectsView` picker. Desktop `ImportLMS` still uses its own wx parser — core unification is a follow-up. |
| Import Effects from LOR Animation `.las` | menu | ✅ | ✅ | parity | P2 | medium | feasible | Same core `LORMusic` reader handles `.las` (identical schema); routed through `loadLMSSource(atPath:)`. |
| Import Effects from LOR Pixel Editor `.lpe` | menu | ✅ | ✅ | parity | P2 | medium | feasible | Shared core `LORPixelEditor` (`src-core/import_export/LORPixelEditor.cpp`, ported from desktop `ImportLPE` `ImportEffects.cpp:2984` incl. the full `LPEParseEffectSettings` effect-translation table). iPad `loadLPESource(atPath:)` + `ImportEffectsView` picker. Desktop `ImportLPE` still uses its own wx parser — core unification is a follow-up. |
| Import Effects from HLS `.hlsIdata` | menu | ✅ | ✅ | parity | P2 | medium | feasible | Shared core `HLSFile` (`src-core/import_export/HLSFile.cpp`, ported from desktop `ImportHLS` `ImportEffects.cpp:949` incl. the wx-free `ConvertDataRowToEffects` On/Color-Wash conversion). iPad `loadHLSSource(atPath:)` + picker. Desktop `ImportHLS` still uses its own wx parser — core unification is a follow-up. |
| Import Effects from Vixen 2.x `.vix` | menu | ✅ | ✅ | parity | P3 | medium | feasible | Shared core `Vixen2File` (`src-core/import_export/Vixen2File.cpp`, ported from desktop `ImportVix` `ImportEffects.cpp:802` incl. base64 `<EventValues>` decode, RGB-leg collapse, and optional sibling `<name>.pro` profile lookup; intensity→effect synthesis reuses the new shared `src-core/import_export/ImportDataRow.cpp` `ConvertDataRowToEffects`). iPad `loadVixen2Source(atPath:)` + `ImportEffectsView` `.vix` picker (`UTType(filenameExtension: "vix")`). Desktop `ImportVix` still uses its own wx parser + `FileConverter::LoadVixenProfile` — core unification is a follow-up. |
| Import Effects from LSP 2.x `.msq` (as effects) | menu | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop `ImportLSP()` (`ImportEffects.cpp:3353`). Niche. (Note: LSP *timing-only* import IS on iPad.) |
| Import Effects from VSA `.vsa` | menu | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop `ImportVsa()` (`ImportEffects.cpp:3603`) + `VsaImportDialog`. Niche. |
| Effect-import channel-mapping wizard (two-pane src/dest tree) | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `xLightsImportChannelMapDialog`; iPad `ImportEffectsView` mapping panes + search filters. |
| Auto Map (norm + aggressive + regex hints) | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Shared `AutoMapper`. iPad `runAutoMap` / `runAutoMapSelectedTargets`. |
| Update aliases from mapping (alias learning) | dialog | ✅ | ✅ | parity | P2 | easy | feasible | iPad `updateModelAliasesFromMapping` mirrors desktop "Update Aliases". |
| Save / load import hints (`.xmaphint`) | dialog | ✅ | ✅ | parity | P2 | easy | feasible | Shared `MapHintsIO`. iPad `saveMapHintsToPath:` / `loadMapHints(fromPath:)`. #6474 multi-file load: the Load Hints picker is now `allowsMultipleSelection: true`, and when mappings already exist a Keep Existing / Overwrite / Cancel `confirmationDialog` runs first ("Overwrite" calls `XLImportSession.clearAllMappings` since the regex pass only fills *unmapped* rows). Legacy `.xmap`/`.xjmap` are wx-bound (desktop only). |
| Effect-import timing-track selection | dialog | ✅ | ✅ | parity | P1 | easy | feasible | iPad timing popover (multi-select w/ count); desktop tree. Shared logic. |
| Effect-import: erase-existing option | dialog | ✅ | ✅ | parity | P2 | easy | feasible | iPad `applyImportWithEraseExisting:`. |
| Effect-import: merge/stack multiple sources onto one target | dialog | ✅ | ✅ | parity | P2 | medium | feasible | Desktop #6474 (Replace / Merge prompt + `_isStackDuplicate`). iPad now matches: mapping a 2nd source onto an already-mapped destination row in `ImportEffectsView` shows a Replace / Add Additional / Cancel `confirmationDialog`; "Add Additional" calls `XLImportSession.addStackedMapping(forRow:…)`. Stacked sources are carried on `BasicImportMappingNode::_stackedMappings` (`src-core/import_export/BasicImportMappingNode.h`) and applied at `XLImportSession.mm` apply via an `appendStacked` lambda — separator `AddEffectLayer()` + appended layers through `MapXLightsEffects`, mirroring desktop `ImportEffects.cpp:430-445,466-480`. Stacked rows render with a `＋ source` caption. |
| Effect-import: lock-imported option | dialog | ✅ | ✅ | parity | P2 | easy | feasible | iPad `lock:` arg. |
| Effect-import: convert-render-style option | dialog | ✅ | ✅ | parity | P2 | easy | feasible | iPad `convertRenderStyle:` → core `MapXLightsEffects(convertRender)`. |
| Effect-import: FPS / version mismatch warning | dialog | ✅ | ✅ | parity | P2 | easy | feasible | iPad `loadWarning` from `sourceFrequency`/`targetFrequency` (xsq/pkg only). |
| Effect-import: missing-media report (post-apply) | dialog | ✅ | ✅ | parity | P2 | easy | feasible | iPad `applyWarning` from `sourceMissingMedia` after the media walk. |
| Import effects by mapping from any on-disk `.xsq` | menu | ✅ | ✅ | parity | P1 | easy | feasible | This IS the iPad Import Effects flow (file-picker → map). |
| Open `.fseq` directly (FSEQ→effects round-trip) | dialog | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop Open Sequence wildcard accepts `.fseq` (`SeqFileUtilities.cpp:262`). iPad only round-trips a matching FSEQ sidecar via `tryLoadFseq`, not as an open format. Low value. 2026-07-16: shared `src-core/render/FSEQFile.cpp` gained `ReadPattern::Bulk` — a caller that reads every frame front-to-back (and has already sized its destination) gets the blocks decompressed ahead in parallel, ~10x on a multi-core host. **Auto-applied to iPad**, and `iPadRenderContext::TryLoadFseq` opts in (`src-iPad/Bridge/iPadRenderContext.cpp`), so the sidecar round-trip loads at the same speed as desktop. Default stays `Streaming`, so realtime one-frame-at-a-time readers are unchanged. |
| Open `.xbkp` backup directly | dialog/menu | ✅ | 🟡 | ipad-weaker | P3 | easy | feasible | Desktop Open wildcard accepts `.xbkp`. iPad routing is **in place**: `handleIncomingSequenceURL` (`XLightsApp.swift`) detects a `.xbkp` URL, opens it through the normal `openSequence` path (it IS `.xsq` XML), then sheets an "Opened a Backup File" alert offering **Save As…** (bumps `saveAsRequestToken`); the pre-existing newer-than-`.xsq` autosave *recovery* sheet is unchanged. **Blocking follow-up:** the `org.xlights.sequence-backup` UTI (extension `xbkp`) + CFBundleDocumentType must be added to `macOS/Assets/xLights-iPad/Info.plist` for Files/iCloud to deliver `.xbkp` taps to `.onOpenURL` — that plist lives in the pinned `macOS` submodule (read-only here), so registration is deferred to a submodule bump. Until then `.xbkp` opens only via in-app pickers that already hand us the URL. |
| Import timing track standalone (`.xtiming`) | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop SeqSettings + RowHeading; iPad `importXTiming(fromPath:)` in `SequenceSettingsSheet`. |
| Import timing from LOR `.lms` / `.las` | dialog | ✅ | ✅ | parity | P1 | easy | feasible | iPad `importLorTiming(fromPath:)`. |
| Import timing from Papagayo `.pgo` | dialog | ✅ | ✅ | parity | P2 | easy | feasible | iPad `importPapagayoTiming(fromPath:)`. |
| Import timing from SubRip `.srt` | dialog | ✅ | ✅ | parity | P2 | easy | feasible | iPad `importSRTTiming(fromPath:)`. |
| Import timing from Audacity label `.txt` | dialog | ✅ | ✅ | parity | P2 | easy | feasible | iPad `importAudacityTiming(fromPath:)`. |
| Import timing from ElevenLabs `.json` | dialog | ✅ | ✅ | parity | P2 | easy | feasible | iPad `importElevenLabsTiming(fromPath:)`. |
| Import timing from Vixen 3 `.tim` (w/ track multi-select) | dialog | ✅ | ✅ | parity | P2 | easy | feasible | iPad `vixen3TimingTrackNames` + `importVixen3Timing(fromPath:selectedIndices:)`. |
| Import timing from LSP `.msq` | dialog | ✅ | ✅ | parity | P3 | easy | feasible | iPad `importLSPTiming(fromPath:)`. |
| Import timing from xLights `.xsq` (w/ track multi-select) | dialog | ✅ | ✅ | parity | P2 | easy | feasible | iPad `xLightsTimingTrackNames` + `importXLightsSequenceTiming(fromPath:selectedIndices:)`. |
| Import lyrics (plain text → phrases, start/end range) | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `LyricsDialog` (`RowHeading.cpp:693`) is text-based too; iPad `ImportLyricsSheet`. |
| Online lyric search (LRCLIB → synced LRC) | dialog | ✅ | ✅ | parity | P2 | medium | feasible | Desktop "Search for Lyrics Online…" (`RowHeading.cpp:641`, `LRCLIBSearchDialog`); iPad `LRCLIBClient` + LRClib in `AddTimingTrackSheet`. Shared `lrc::ParseLRC` / `XLLyricsImport`. |
| AI Speech-to-Lyrics (audio → timed lyric track) | dialog | ✅ | ✅ | parity | P2 | hard | feasible | Desktop "AI Speech 2 Lyrics" (`RowHeading.cpp:636`, `GenerateLyricsDialog`); iPad on-device speech via `addLyricTimingTrack`. |
| Convert sequence/channel data between formats (Convert tool) | menu/dialog | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop `ConvertDialog` (`xLightsMain.cpp:4261`) maps `_seqData` channel data across formats. No iPad UI; substantial bridge work, low demand. |
| Export Effects to file (`<effects>` `.xsq` fragment) | menu | ✅ | ❌ | ipad-missing | P2 | medium | feasible | Desktop Tools → Export Effects (`ExportEffects.cpp`, `xLightsMain.cpp:5790`). iPad lacks UI + bridge; core exists. |
| Export Models report (`.xlsx`) | menu | ✅ | ✅ | parity | P2 | easy | feasible | Shared `ExportModels` (libxlsxwriter). iPad `exportModelsReport(toPath:)` from Layout Editor; desktop File → Export Models. |
| Export single timing track (`.xtiming`) | context-menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop RowHeading "Export" (pgo too); iPad `exportTimingTrack(atRow:toPath:)`. |
| Export multiple timing tracks in one `.xtiming` (`<timings>`) | dialog | ✅ | ✅ | parity | P2 | easy | feasible | Desktop SelectTimingsDialog multi-export; iPad `exportTimingTracks(atRows:toPath:)` ("Export Multiple Tracks"). |
| Export timing track as Papagayo `.pgo` | context-menu | ✅ | ✅ | parity | P3 | easy | feasible | Desktop RowHeading `GetPapagayoExport` (`RowHeading.cpp:1668`) ↔ iPad "Export as Papagayo (.pgo)…" (`RowHeaderViews.onExportPapagayo` → `XLSequenceDocument.exportTimingTrackAsPapagayo` → core `TimingElement::GetPapagayoExport`); gated on a 3-layer phrase/word/phoneme breakdown like desktop. |
| Export model rendered data as FSEQ/`.eseq` | context-menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `SeqExportDialog` "xLights/FPP *.fseq"; iPad row "Export Model as FSEQ…" (`exportModelAsFSEQ`). Desktop macOS-sandbox fix: `SeqExportDialog` is now a format listbox + Next that always opens the native Save As panel (no typeable filename field), so the sandbox grants a writable scope — wx-UI-only; iPad export already targets a document-picker-scoped path so no change needed. |
| Export model FSEQ over a sub-range | context-menu | ✅ | ✅ | parity | P2 | easy | feasible | Desktop "Export Model (selected effects)" range; iPad "Export Model (Loop Range) as FSEQ…". Different range source (selection vs loop region). |
| Export single model to LOR / Lcb / Vixen / LSP / HLS | context-menu/dialog | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop `SeqExportDialog` format list (`SeqExportDialog.cpp:87`). iPad per-model export is FSEQ-only. Niche legacy targets. |
| Export single model as Video (mp4 / mov) | context-menu | ✅ | ✅ | parity | P2 | hard | hard | Desktop `SeqExportDialog` Compressed/HQ/ProRes/Lossless video. iPad row submenu "Export Model as Video ▸" (Compressed .mp4 / High Quality .mp4 / ProRes 4444 .mov / Lossless RGB .mov) → `exportModelAsVideo` → core `ModelVideoExporter::WriteModelVideo`. (AVI is desktop-only — no AVFoundation AVI decoder.) |
| Export single model as Video — HD ProRes upscale (1920×1080) | context-menu | ✅ | ✅ | parity | P3 | easy | feasible | Desktop #6506: `SeqExportDialog` "HD ProRes Video, *.mov" → `RenderUI.cpp` `exportWidth=1920,exportHeight=1080`. iPad: new `exportModelAsVideo(atRow:…exportWidth:exportHeight:completion:)` bridge method (`XLSequenceDocument.h:535`, `.mm:1519`) passes dims to core `ModelVideoExporter::WriteModelVideo`. "HD ProRes 1080p (.mov)" entry added to "Export Model as Video ▸" submenu (`RowHeaderViews.swift:600`). `SequencerViewModel.exportModelAsVideo` routes to the width/height variant when non-zero (`SequencerViewModel.swift:3968`). |
| Export single model as GIF | context-menu/dialog | ✅ | ✅ | parity | P3 | medium | feasible | Now shared wx-free core `ModelGifExporter::WriteModelGif` (`src-core/render/`, reuses `ModelVideoExporter::FillXlImage` + vendored public-domain gif-h `dependencies/gif-h/gif.h`) — replaced desktop's old `wxQuantize`+`wxGIFHandler` path in `WriteGIFModelFile`. iPad: `exportModelAsGIF` bridge (`XLSequenceDocument`), `SequencerViewModel.exportModelAsGif`, "Export Model as GIF" row entry (`RowHeaderViews.swift`) → share sheet. |
| Export single model for Minleon `.bin` | context-menu/dialog | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop `SeqExportDialog` Minleon. Niche hardware. |
| Export House Preview Video (sequence + house layout) | menu | ✅ | ✅ | parity | P1 | hard | feasible | Desktop File → Export Video (`xLightsMain.cpp:3742`, `VideoExporter`). iPad Tools → "Export House Preview…" (`ExportHousePreviewSheet`) → `exportHousePreviewVideo` → `XLHousePreviewVideoExporter` renders the house preview **offscreen** at a chosen resolution (independent of the on-screen pane) and feeds core `VideoWriter`. AVFoundation writer hang/memory fix (smaller frame queue, pull-driven audio via `BeginAudio`) is in shared `src-apple-core` — auto-applies to both. |
| Export video: codec / quality selection | dialog | ✅ | 🟡 | partial | P2 | hard | feasible | Desktop `SeqExportDialog` video variants. iPad: per-model submenu picks codec (Compressed/HQ/ProRes/Lossless); house-preview sheet picks resolution (720/1080/4K/match) + Compressed-vs-HQ. No explicit bitrate field (`VideoWriter` chooses). |
| Batch render sequences → `.fseq` | sheet | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `BatchRenderDialog`; iPad `BatchRenderSheet`/`BatchRenderRunner` (home screen). Shared render loop + `writeFseq`. |
| Batch render honors configured FSEQ folder | sheet | ✅ | ✅ | parity | P1 | easy | feasible | Both consult `FolderConfig.fseqFolder`, falling back to the `.xsq` dir. |
| Write whole-sequence FSEQ (v2/zstd/sparse) | core/save | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `WriteFalconPiFile`; iPad `writeFseq(toPath:)`. Same format. |
| FPP Connect (discover + upload FSEQ/media) | menu/sheet | ✅ | ✅ | parity | P1 | easy | feasible | Open firmware. Desktop Tools → FPP Connect; iPad `FPPConnectSheet`. iPad currently honors `uploadMedia` only (Outputs/Cape config persisted but deferred). |
| Package Sequence for distribution (`.xsqz` + media) | menu/sheet | ✅ | ✅ | parity | P1 | easy | feasible | Shared `SequencePackage`. Desktop Tools → Package Sequence; iPad `PackageSequenceSheet`. |
| Package: exclude audio / exclude videos | preference / toggle | ✅ | ✅ | parity | P2 | easy | feasible | Desktop = Preferences → Other Settings checkboxes (`OtherSettingsPanel.cpp:129`); iPad = in-sheet toggles. Surface differs; both honored at pack time. |
| Export controller wiring as CSV | menu / context-menu | ✅ | ✅ | parity | P2 | easy | feasible | Shared `UDController::ExportAsCSV`. Desktop Tools menu; iPad ControllerVisualize export menu. |
| Export controller wiring as JSON | menu / context-menu | ✅ | ✅ | parity | P2 | easy | feasible | Shared `UDController::ExportAsJSON`. |
| Export / import effect presets (library round-trip) | dialog | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `EffectTreeDialog` Import/Export `.xpreset` (`EffectTreeDialog.cpp:774/873`); iPad `exportPresets`/`importPresets` (JSON / effects-tree XML). Both round-trip via shared preset blob. |
| Map From Lights (camera-scan → model geometry) | menu/wizard | ✅ | ✅ | parity | P2 | hard | feasible | macOS desktop via `KLightMapperBridge` (Continuity Camera, `xLightsMain.cpp:4296`); iPad `MapFromLightsWizard`. Windows/Linux desktop now also supported (local + RTSP/IP camera) via the prebuilt KLightMapper library + `KLightMapperBridge_win.cpp`, fetched by `ci_scripts/fetch_dependencies.ps1` / `fetch_klightmapper.sh`. |
| Export HinksPix configuration | menu/dialog | ✅ | ❌ | ipad-missing | P3 | hard | restricted | Desktop `HinksPixExportDialog` (`xLightsMain.cpp:6083`). Closed/proprietary controller firmware → IAP-gated, low priority. |
| Import vendor model / vendor music (online catalog) | dialog | ✅ | 🟡 | parity | P2 | medium | feasible | Desktop `VendorModelDialog` / `VendorMusicDialog`; iPad `VendorBrowserSheet` + `XLVendorCatalog` (shared `VendorCatalog`). Model browser present; verify music-download breadth. |
| Vendor catalog: live search/filter of models | dialog | ✅ | 🟡 | ipad-weaker | P3 | easy | feasible | Desktop: hierarchy-preserving live filter (`VendorModelDialog.cpp` `RebuildTreeUI`/`ApplyFilterNow`) — debounced, whitespace-tokenized case-insensitive AND across vendor/category/model/wiring, prunes non-matching branches, magnifier steps to next match. iPad: per-level `.searchable` on vendor + model lists (`VendorBrowserSheet.swift:40,164,206`) — single-term substring per level, no tokenized AND across levels. |
| Export Effects summary (CSV) | menu | ✅ | ✅ | parity | P2 | medium | feasible | Desktop `ExportEffects.cpp:347-360`; iPad `src-core/import_export/ExportEffectsReport.cpp` (EFX-1) + bridge `XLSequenceDocument.mm:exportEffectsReport(toPath:)` + Tools menu in `XLightsCommands.swift` + fileExporter in `SequencerView.swift`. |
| Import media options (sequence package asset destinations) | dialog | ✅ | ❌ | ipad-missing | P2 | medium | feasible | Core `SequencePackage::GetImportOptions` exists but is unused on iPad; desktop surfaces asset-destination choices on package import. No iPad UI. |
| Available-models list with CCR-strand mode and Used marking | dialog | ✅ | 🟡 | ipad-weaker | P3 | medium | feasible | Desktop `xLightsImportChannelMapDialog.cpp:537,575,674` (CCR strand mode + Used markers); iPad `ImportEffectsView.swift:209-247` lists models without CCR/Used. |
| Map-tree right-click menu (expand/collapse/clear/add group) | dialog | ✅ | 🟡 | ipad-weaker | P3 | medium | feasible | Desktop `xLightsImportChannelMapDialog` `OnRightDown` context menu. iPad `DestinationRowView` now has a `.contextMenu` (long-press) on model rows with **Sort Submodels By Name** (#4636, below); expand/collapse/clear/add-group entries still TODO. |
| Map-tree: sort submodels within a model | context-menu | ✅ | ✅ | parity | P3 | easy | feasible | Desktop #4636 right-click "Sort Submodels By Name" (`xLightsImportChannelMapDialog.cpp:782-785,253-259`, display-only `wxDataView Compare`). iPad: long-press a destination model row → "Sort Submodels By Name" → `XLImportSession.sortSubmodels(forRow:)` toggles a per-node display flag applied in `snapshotRow` (submodel children sorted by name; strands keep order so apply-time `GetSubModel(index)` routing is unaffected — same display-only model as desktop). |
| Import timeline preview column | dialog | ✅ | 🟡 | ipad-weaker | P3 | medium | feasible | Desktop `ImportEffects.cpp` `GenerateTimelineBitmap` renders a per-row pixel timeline. iPad now shows a lightweight per-source **effect-count + duration** caption plus a normalized density strip (`SourceTimelineStrip`) in `ImportEffectsView.swift`'s source list. Data flows from new `AvailableSource::effectCount`/`durationMs` (`src-core/import_export/ImportMappingNode.h`), populated in `XLImportSession.rebuildAvailableSources` (`.xsq`/package sources) and surfaced via `XLImportAvailableSource.effectCount`/`durationMs`. Legacy channel-data readers (vix/hls/lms/lpe) report 0 (no cheap per-channel count); the desktop's full per-effect rectangle strip is still richer. |
| Model-blending import toggle | dialog | ✅ | ❌ | ipad-missing | P3 | easy | feasible | Desktop per-model blend toggle `xLightsImportChannelMapDialog.cpp:538,593-594,1261`; iPad `XLSequenceDocument.h:228-229` exposes sequence-level blending only, not per-model on import. The iPad `ImportEffectsView` has no per-row blend column yet; the shared `AvailableSource` carries no blend flag. Deferred — needs a per-destination-row blend flag plumbed through `BasicImportMappingNode` + apply, plus a SwiftUI toggle column. |
| Model wiring remap generator | menu | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop `ModelRemap.cpp` generates a wiring remap; no iPad `ModelRemap` symbol/UI. |

## iPad gaps (desktop has, iPad missing)

### P1

- ~~**Import Effects from LOR Music `.lms` / Animation `.las`.**~~ ✅ **Landed.**
  Ported the desktop `xLightsFrame::ImportLMS` parser
  (`src-ui-wx/import_export/ImportEffects.cpp:1104` / `:1683`) into a wx-free
  core reader `LORMusic` (`src-core/import_export/LORMusic.{h,cpp}`, peer of
  `LOREdit`/`Vixen3`). `XLImportSession.loadLMSSource(atPath:)` builds the same
  available-source / destination-tree representation as the `.loredit` path, so
  Auto Map, manual mapping, AI Map, MapHints and apply all work unchanged; the
  apply branch synthesizes On / Color Wash / Twinkle effects (and CCR per-pixel
  fan-out) onto the mapped layers. `ImportEffectsView` allows `.lms` / `.las` in
  the picker and routes both to the same loader (the two share one schema).
  **Follow-up:** the *desktop* `ImportLMS` still uses its own wx parser — unify
  it onto the new core `LORMusic` reader so both clients share one code path.

- **Export House Preview Video.** ✅ **Landed.** Tools → "Export House
  Preview…" (`ExportHousePreviewSheet`) → `XLSequenceDocument.exportHousePreviewVideo`
  → `XLHousePreviewVideoExporter` (`src-iPad/Metal/`). Renders the house
  preview **offscreen** into a Metal texture at a chosen resolution
  (720/1080/4K/match preview) — independent of the on-screen pane size —
  and feeds the core `VideoWriter` (AVFoundation H.264 / HEVC). The full
  scene is rendered to match the live preview: models, **view objects**
  (house mesh / ground / terrain), and the **2D background image**, framed
  with the **live house-preview camera** (pan / rotation / 2D-3D mode,
  snapshotted by the on-screen bridge via `iPadRenderContext::SetHousePreviewCamera`;
  2D pan rescaled for the export resolution). GPU frame path: CoreImage
  renders the Metal texture straight into the encoder's NV12 `CVPixelBuffer`
  (no CPU readback), tagged `kCIImageColorSpace = DeviceRGB` so brightness/
  gamma match the on-screen look (a `nil` color space lifts dimmed content).
  Runs on a background queue with a progress sheet; the live preview skips
  drawing during export (`iPadRenderContext::IsExportInProgress`) to avoid
  racing per-model node colours. Offscreen support added to
  `iPadModelPreview` (`SetOffscreenTarget`). Resolution + codec choice
  persist via `@AppStorage`; output handed to the share sheet.

### P2

- ~~**Import Effects from LOR Pixel Editor `.lpe`.**~~ ✅ **Landed.**
  Ported the desktop `xLightsFrame::ImportLPE` parser
  (`src-ui-wx/import_export/ImportEffects.cpp:2984`, plus the `MapLPE` /
  `MapLPEEffects` / `LPEParseEffectSettings` family) into a wx-free core reader
  `LORPixelEditor` (`src-core/import_export/LORPixelEditor.{h,cpp}`, peer of
  `LORMusic`). The full per-effect translation table is carried over verbatim
  (Butterfly/Bars/Spirals/Curtain/Fire/Garlands/Meteors/Pinwheel/Snowflakes/
  Text/etc.), reading the effect VC min/max from the already-wx-free effect
  statics (`ButterflyEffect::sChunksMin` …). `wxString::Format` →
  `fmt::format`, `wxSplit` → `string_utils::Split`, `wxURI::Unescape` → a small
  local `UriUnescape`. `XLImportSession.loadLPESource(atPath:)` reuses the
  shared available-source / destination-tree flow; apply fans the LPE left/right
  sides + layers 0/1 across effect layers (and per-node strands), honouring the
  stacked-mapping separator-layer convention. `ImportEffectsView` allows `.lpe`
  in the picker. **Follow-up:** the *desktop* `ImportLPE` still uses its own wx
  parser — unify it onto the new core reader.

- ~~**Import Effects from HLS `.hlsIdata`.**~~ ✅ **Landed.**
  Ported the desktop `xLightsFrame::ImportHLS` parser
  (`src-ui-wx/import_export/ImportEffects.cpp:949`, plus `MapHLSChannelInformation`
  / `FindHLSStrandName` / `MapToStrandName` / `ReadHLSData`) into a wx-free core
  reader `HLSFile` (`src-core/import_export/HLSFile.{h,cpp}`). The On / Color
  Wash conversion (`xLightsFrame::DoConvertDataRowToEffects` +
  `RampLenColor`/`isOnLineColor` from `tabSequencer.cpp`) was already wx-free
  bar one `wxString::Format`, brought across as a static `ConvertDataRowToEffects`.
  `XLImportSession.loadHLSSource(atPath:)` reuses the shared flow; apply decodes
  each mapped channel's per-frame colour stream into effects, fanning CCR strand
  prefixes across node layers (the desktop MapByStrand path). `ImportEffectsView`
  allows `.hlsIdata` in the picker. **Follow-up:** the *desktop* `ImportHLS` still
  uses its own wx parser — unify it onto the new core reader.

- **Export Effects to file.** Desktop Tools → Export Effects
  (`src-ui-wx/import_export/ExportEffects.cpp`, handler `xLightsMain.cpp:5790`)
  writes selected/all effects as an `<effects>` `.xsq` fragment for
  re-import elsewhere. iPad needs a Tools entry + `XLSequenceDocument`
  exporter wrapping the existing core. Ease: medium.

- **Per-model Video export.** ✅ **Landed.** Row-heading submenu
  "Export Model as Video ▸" (Compressed .mp4 / High Quality .mp4 /
  ProRes 4444 .mov / Lossless RGB .mov) → `exportModelAsVideo` →
  `RenderEngine::ExportModelData` + core `ModelVideoExporter::WriteModelVideo`,
  then `.fileExporter`. AVI stays desktop-only (no AVFoundation AVI
  decoder). Synchronous (a single model's buffer is small).

- ~~**Export Effects summary (CSV).**~~ **Landed (EFX-1).** `src-core/import_export/ExportEffectsReport.cpp`,
  bridge `exportEffectsReport(toPath:)`, Tools menu + fileExporter in SequencerView. ✅

- **Import media options.** Core `SequencePackage::GetImportOptions`
  lets the user pick asset destinations when importing a package; the
  desktop surfaces it but the iPad leaves it unused. Needs a SwiftUI
  options sheet + bridge.

### P3 (low value)

- **Other effect-import formats** — LSP `.msq` as effects (`ImportLSP`,
  `:3353`), VSA `.vsa` (`ImportVsa`, `:3603`). Each parser is wx-bound; would
  need core extraction + a bridge entry. Low frequency / legacy. (Note:
  LSP/Vixen3 *timing-only* import already on iPad; `.lpe`, `.hlsIdata`, and
  Vixen 2 `.vix` effect-import now landed — `.vix` via shared core
  `Vixen2File`.)
- **Per-model LOR/Lcb/Vixen/LSP/HLS export, GIF export, Minleon `.bin`** —
  all in desktop `SeqExportDialog`. iPad per-model export is FSEQ-only.
- **Convert tool** — desktop `ConvertDialog` (`xLightsMain.cpp:4261`) maps
  raw channel data between formats; substantial bridge, niche.
- **Open `.fseq` directly** — desktop Open wildcard accepts `.fseq`
  (`SeqFileUtilities.cpp:262`). iPad round-trips an FSEQ sidecar
  (`tryLoadFseq`) but doesn't open it as a first-class format. (`.xbkp`
  direct open is now 🟡 — Swift routing + Save-As notice landed; the Files
  "Open in" UTI registration is a deferred submodule plist bump. See
  scorecard.)
- ~~**Export timing track as Papagayo `.pgo`**~~ — ✅ **Done.** iPad
  "Export as Papagayo (.pgo)…" on broken-down timing rows routes through
  the shared core `TimingElement::GetPapagayoExport` and shares the file.
- **Import-wizard refinements** — desktop's `xLightsImportChannelMapDialog`
  has an available-models list with CCR-strand mode and Used markers
  (`:537,575,674`), a map-tree right-click menu (expand/collapse/clear/add
  group, `OnRightDown`), a per-row timeline preview column
  (`GenerateTimelineBitmap`), and a per-model blending toggle (`:538,593-594,1261`).
  The iPad has now picked up the **merge/stack** prompt (#6474), **multi-file
  hint load** with keep/overwrite (#6474), **submodel sort** in a row
  `.contextMenu` (#4636), and **Edit Display Elements** mid-import (#6477,
  theme-10), and a lightweight **per-source timeline strip** — an
  effect-count + duration caption plus a normalized density bar
  (`SourceTimelineStrip` in `ImportEffectsView`, fed by new
  `AvailableSource::effectCount`/`durationMs`). Still missing on iPad:
  CCR/Used markers, the full expand/collapse/clear/add-group context menu,
  the desktop's richer per-effect rectangle strip (`GenerateTimelineBitmap` —
  the iPad strip is a single normalized bar, not per-effect rectangles), and
  the per-model blending toggle (`XLSequenceDocument.h:228-229` exposes
  sequence-level blending only).
- **Model wiring remap generator** — desktop `ModelRemap.cpp`; no iPad
  equivalent.

## Desktop gaps (iPad has, desktop missing)

For this theme there are **no true desktop-missing items**:

- *Multi-track `.xtiming` export* — desktop has it (SelectTimingsDialog).
- *Package exclude-audio* — desktop has it (Preferences → Other Settings).
- *Preset library export* — desktop has it (`EffectTreeDialog` Export
  `.xpreset`).
- *Map From Lights* — desktop has it on macOS, Windows, and Linux
  (`KLightMapperBridge` / `KLightMapperBridge_win.cpp`).
- *AI Speech-to-Lyrics / online lyric search* — desktop has both.

The only directional nuance: the iPad surfaces several of these more
prominently (in-sheet toggles, dedicated wizard) while the desktop tucks
them into preferences/dialogs — a UX-placement difference, not a missing
capability. Map From Lights is desktop-missing on **Windows/Linux** (it's
a macOS Continuity-Camera feature), but that is out of scope for an
iPad↔macOS-desktop parity audit.

## Infeasible / restricted on iPad

- **HinksPix configuration export** — `restricted`. Closed/proprietary
  controller firmware; IAP-gated, P3. Desktop `HinksPixExportDialog`.
- **Per-model video export** ✅ landed (row submenu → `ModelVideoExporter`).
  Minleon/LOR-binary exports remain *feasible* but low priority.
- Nothing here is hard-blocked by iOS sandbox: file I/O goes through
  `ObtainAccessToURL` security-scoped bookmarks, and audio/video encode
  uses AVFoundation/`VideoWriter` rather than FFmpeg, so the desktop's
  FFmpeg-only paths are not a wall for these export features.

## Recommended sequencing

1. ~~**Extract a core LMS/LAS reader → iPad `.lms`/`.las` effect import (P1).**~~
   ✅ **Done** — core `LORMusic` reader + `loadLMSSource(atPath:)` reuse the
   existing mapping/apply/auto-map/hints flow. Both formats land at once.
   **Follow-up:** unify the desktop `ImportLMS` onto the new core reader.
2. ~~**House Preview Video export (P1).**~~ ✅ **Done** — offscreen
   `XLHousePreviewVideoExporter` at chosen resolution + export sheet with
   progress. Per-model video export ✅ done alongside it.
3. ~~**Export Effects to file (P2).**~~ ✅ **Landed (EFX-1)** — `src-core/import_export/ExportEffectsReport.cpp` + bridge + Tools menu + fileExporter.
4. **Remaining legacy effect-import formats (P2/P3).** `.lpe` (core
   `LORPixelEditor`), `.hlsIdata` (core `HLSFile`), and `.vix` (core
   `Vixen2File`) ✅ **done** — all reuse the shared mapping/apply flow via
   `loadLPESource(atPath:)` / `loadHLSSource(atPath:)` /
   `loadVixen2Source(atPath:)`. The HLS + Vixen 2 readers share
   `src-core/import_export/ImportDataRow.cpp`'s `ConvertDataRowToEffects` /
   `Base64Decode`. Still open: `.msq`, `.vsa` — only as user demand warrants;
   each needs core parser extraction. **Follow-up:** unify the desktop
   `ImportLPE` / `ImportHLS` / `ImportVix` onto the new core readers.
5. **Convert tool / per-model LOR/Lcb/GIF/Minleon export / HinksPix
   (P3/restricted)** — defer; niche or IAP-gated.
