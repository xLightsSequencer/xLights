# 08. Import & Export

_Generated from code on 2026-07-31. Status: ✅ parity | 🟡 partial | ❌ missing on iPad | 🚫 infeasible/restricted on iPad (reason required) | 🔵 iPad-only._

## Import format matrix

Desktop's single entry point is **Tools → Import Effects** (`src-ui-wx/import_export/ImportEffects.cpp:71`), whose
file-dialog filter list is `ImportEffects.cpp:74-87` and whose extension dispatch is `ImportEffects.cpp:134-154`.
The iPad's single entry point is **Tools → Import Effects…** (`src-iPad/App/XLightsCommands.swift:379`) →
`ImportEffectsView` (`src-iPad/App/ImportEffectsView.swift:16`), whose accepted UTType list is
`ImportEffectsView.swift:742-768` and whose extension dispatch is `ImportEffectsView.swift:477-538`.

Rows 14-20 are the **legacy Convert tab** (`Tools → Convert`, `src-ui-wx/xLightsMain.cpp:1062` →
`OnMenuItemConvertSelected` at `xLightsMain.cpp:4207` → `ConvertDialog`), a *channel-data* converter separate from
effect import. Its input dispatch is `src-ui-wx/import_export/ConvertDialog.cpp:1655-1740`. The whole tab is absent
from the iPad: `grep -ril ConvertDialog|Conductor|Glediator|xseq src-iPad/` returns no converter code (the only
`Glediator` hit is `src-iPad/App/FilepickerPropertyView.swift`, an *effect* file picker; the only `xseq` hit is the
`xsequence` XML root in `src-iPad/Bridge/XLSequenceDocument.mm:794`; the only `FileConverter` hit is a comment at
`src-iPad/Bridge/iPadRenderContext.cpp:2471`).

| # | Format | Desktop evidence | iPad status | Notes |
|---|---|---|---|---|
| 1 | xLights sequence `.xsq` (effects) | `ImportEffects.cpp:148` → `ImportXLights` `ImportEffects.cpp:161`; mapping dialog `ImportEffects.cpp:261` | ✅ | `ImportEffectsView.swift:528` → `XLImportSession.h:77` `loadSourceSequence(atPath:)`; parses via the same core `SequenceFile`/`SequencePackage` (`XLImportSession.mm:292-294`) |
| 2 | Legacy xLights sequence `.xml` (effects) | `ImportEffects.cpp:82`, dispatched with `.xsq` at `ImportEffects.cpp:148` | ✅ | `.xml` is in the picker list (`ImportEffectsView.swift:750`) and falls through to the same `loadSourceSequence` branch (`ImportEffectsView.swift:528`) |
| 3 | xLights sequence package `.xsqz` / `.zip` / `.piz` | `ImportEffects.cpp:80`, `:148`; `SequencePackage::Extract` at `ImportEffects.cpp:167` | ✅ | `ImportEffectsView.swift:747-749,766`; bridge builds the same `SequencePackage` for loose and packaged sources (`XLImportSession.mm:270-292`) |
| 4 | SuperStar `.sup` | `ImportEffects.cpp:142` → `ImportSuperStar` `ImportEffects.cpp:1189` | ✅ | `ImportEffectsView.swift:479-489` swaps in `SuperStarImportView` (`src-iPad/App/SuperStarImportView.swift:14`) → `XLSuperStarImport.mm:89`; same core `SuperStar::Importer` |
| 5 | LOR Music `.lms` | `ImportEffects.cpp:134` → `ImportLMS` `ImportEffects.cpp:1162` / `:1741` | ✅ | `ImportEffectsView.swift:506-510` → `XLImportSession.h:224` `loadLMSSource(atPath:)`; core `LORMusic` reader |
| 6 | LOR Animation `.las` | `ImportEffects.cpp:134` (shares the `.lms` schema/reader) | ✅ | Same branch, `ImportEffectsView.swift:506`; picker entry `ImportEffectsView.swift:758` |
| 7 | LOR Pixel Editor `.lpe` | `ImportEffects.cpp:136` → `ImportLPE` `ImportEffects.cpp:1171` / `:3052` | ✅ | `ImportEffectsView.swift:511-515` → `XLImportSession.h:233` `loadLPESource(atPath:)`; core `LORPixelEditor` reader |
| 8 | LOR S5 `.loredit` | `ImportEffects.cpp:138` → `ImportS5` `ImportEffects.cpp:1180` / `:2901` | ✅ | `ImportEffectsView.swift:495-499` → `XLImportSession.h:205` `loadLOREditSource(atPath:)`; core `LOREdit` reader |
| 9 | HLS `.hlsIdata` (effects) | `ImportEffects.cpp:140` → `ImportHLS` `ImportEffects.cpp:1007` | ✅ | `ImportEffectsView.swift:516-520` → `XLImportSession.h:242` `loadHLSSource(atPath:)`; core `HLSFile` reader |
| 10 | Vixen 2.x `.vix` (effects) | `ImportEffects.cpp:146` → `ImportVix` `ImportEffects.cpp:811` | ✅ | `ImportEffectsView.swift:521-526` → `XLImportSession.h:252` `loadVixen2Source(atPath:)`; core `Vixen2File` reader incl. sibling `.pro` profile |
| 11 | Vixen 3 `.tim` (effects) | `ImportEffects.cpp:144` → `ImportVixen3` `ImportEffects.cpp:3183` | ✅ | `ImportEffectsView.swift:500-505` → `XLImportSession.h:215` `loadVixen3Source(atPath:)`; needs the sibling `SystemConfig.xml`, same as desktop |
| 12 | LSP 2.x `.msq` (effects) | `ImportEffects.cpp:150` → `ImportLSP` `ImportEffects.cpp:3429` (zip of `Sequence` + per-controller XML, mapped via `LMSImportChannelMapDialog` at `:3434`) | ❌ | No LSP **effect** reader on iPad. `grep -rn "msq" src-iPad/` hits only the *timing-only* path: `SequenceSettingsSheet.swift:386,399,656,709` → `SequencerViewModel.swift:5311` → `XLSequenceDocument.mm:1723` (`ProcessLSPTiming`). `.msq` is absent from `ImportEffectsView.swift:742-768` and there is no `loadLSPSource` in `XLImportSession.h` |
| 13 | VSA `.vsa` (servo / DMX tracks) | `ImportEffects.cpp:152` → `ImportVsa` `ImportEffects.cpp:3679`; `VsaImportDialog` at `:3683`; reader `src-ui-wx/import_export/VSAFile.cpp` | ❌ | `grep -rli "vsa\|VSA" src-iPad/` → **no matches at all**. Note the reader itself (`VSAFile.cpp`) lives in `src-ui-wx/`, not `src-core/`, so it isn't even linkable from the iPad today |
| 14 | Convert input: xLights/FPP `.fseq` | `ConvertDialog.cpp:1681-1690` → `xLightsFrame::ReadFalconFile` (`FileConverter.cpp:1317`) | ❌ | No Convert tab on iPad (see preamble sweep) |
| 15 | Convert input: legacy xLights `.xseq` | `ConvertDialog.cpp:1673-1681` (`XLIGHTS_SEQUENCE_EXT` = `"xseq"`, `include/globals.h:34`) → `ReadXlightsFile` | ❌ | Same |
| 16 | Convert input: Lynx Conductor `.seq` | `ConvertDialog.cpp:1690-1700` → `ReadConductorFile` (`ConvertDialog.cpp:713`); guarded `#ifndef FPP` | ❌ | Same |
| 17 | Convert input: Glediator `.gled` | `ConvertDialog.cpp:1701-1711` → `ReadGlediatorFile` (`ConvertDialog.cpp:751`) | ❌ | Same. `grep -rn "Glediator" src-iPad/` → only the effect file picker |
| 18 | Convert input: HLS `.hlsIdata` (channel data) | `ConvertDialog.cpp:1712-1714` → `ReadHLSFile` (`ConvertDialog.cpp:964`) | ❌ | Distinct from row 9 — Convert reads it as raw channel data, not effects |
| 19 | Convert input: LOR `.lms` / `.las` (channel data, with 25/50/100 ms resolution choice) | `ConvertDialog.cpp:1715-1735`; resolution picker `ConvertDialog.cpp:144-146` | ❌ | Distinct from rows 5-6 |
| 20 | Convert input: Vixen `.vix` (channel data) | `ConvertDialog.cpp:1664-1673` → `ReadVixFile` (`ConvertDialog.cpp:827`) | ❌ | Distinct from row 10 |

## Features

| # | Feature | Desktop evidence | iPad status | iPad evidence / gap |
|---|---|---|---|---|
| **Import mapping dialog** — desktop `xLightsImportChannelMapDialog` (`src-ui-wx/import_export/xLightsImportChannelMapDialog.cpp`), iPad `ImportEffectsView` + `XLImportSession` | | | | |
| 1 | Two-pane source (available) / destination (mapping) layout | `xLightsImportChannelMapDialog.cpp:583` (dialog "Map Channels"), splitter `:534`, available list `:532`, mapping tree `:1312` | ✅ | `ImportEffectsView.swift:215-320` (`sourcePane` / `destinationPane`) |
| 2 | Hierarchical destination tree (model → strand/submodel → node) | `AddModel` `xLightsImportChannelMapDialog.cpp:1603`; `xLightsImportTreeModel` 3-level nodes | ✅ | `XLImportSession.h:40-52` (`XLImportMappingRow.children`), recursive `DestinationRowView` `ImportEffectsView.swift:788-877` |
| 3 | Assign a source to a destination row | Double-click `OnItemActivated` `:1761` → `Map` `:1779`; drag-drop `OnBeginDrag` `:2838` / `OnDragDrop` `:2809` / `OnDrop` `:2938` | 🟡 | Tap-select-then-"Map →" only (`ImportEffectsView.swift:325`, `mapSelected` `:552`). **Gap:** no drag-and-drop from source list onto a tree row |
| 4 | Unassign one destination row | `Unmap` `:1788` | ✅ | `ImportEffectsView.swift:327` → `unmapSelected` `:600` |
| 5 | Clear All mappings | `ClearAll` `:996`, context menu `:800` | 🟡 | Bridge op exists (`XLImportSession.h:163` `clearAllMappings`) but the only caller is the hint-overwrite branch (`ImportEffectsView.swift:462`). **Gap:** no user-facing "Clear All" button/menu |
| 6 | Clear Selected mappings | `ClearSelected` `:1055`, context menu `:801` | ❌ | No per-selection clear. `grep -rn "clearSelected\|ClearSelected" src-iPad/` → no matches; `ImportEffectsView.swift:322-374` has no such control |
| 7 | Auto Map (norm pass + aggressive pass + `<showdir>/maphints/*.xmaphint` regex pass) | `Button_AutoMap` `:665` → `OnButton_AutoMapClick`; core `AutoMapper` (`src-core/import_export/AutoMapper.cpp`), `MatchAggressive` `AutoMapper.h:43`, `MatchRegex` `AutoMapper.h:50` | ✅ | `ImportEffectsView.swift:329` → `autoMap()` `:608` → `XLImportSession.h:129` `runAutoMap` (same core passes, documented `XLImportSession.h:127-128`) |
| 8 | Auto Map Selected (limit to selected targets/sources) | `OnButton_AutoMapSelClick`; tree context menu `:791`, available-list context menu `RightClickModelsAvail` `:811` | ❌ | Bridge op exists (`XLImportSession.h:134` `runAutoMapSelectedTargets:sources:`) but **no Swift caller** — `grep -rn "runAutoMapSelected" src-iPad/App/` → no matches. Core-shared, no iPad UI |
| 9 | AI Map (LLM-suggested target→source mapping) | `Button_AIMap` `:667` → `OnButton_AIMapClick` | ✅ | `ImportEffectsView.swift:331-342` → `aiMap()` `:614` → `XLImportSession.h:143` `runAIMap(completion:)`; gated on a MAPPING-capable service |
| 10 | Update Aliases w/ Maps (alias the source name onto the destination model) | `Button_UpdateAliases` `:669` → `OnButton_UpdateAliasesClick`, status text `:4543` | ✅ | `ImportEffectsView.swift:359` → `updateAliases()` `:441` → `XLImportSession.h:172` `updateModelAliasesFromMapping` |
| 11 | Save mapping — text `.xmap` | `SaveMapping` `:2332` (filter `:2334`), `SaveXMapMapping` `:2348` | ❌ | Only `.xmaphint` is written (`ImportEffectsView.swift:376-386` → `XLImportSession.h:148` `saveMapHintsToPath:`). `grep -rn "xmap" src-iPad/` finds no `.xmap`/`SaveXMap` writer; the bridge header itself records the gap at `XLImportSession.h:154-155` |
| 12 | Save mapping — JSON `.xjmap` | `SaveMapping` `:2334`, `SaveJSONMapping` `:2443` | ❌ | `grep -rn "xjmap" src-iPad/` → single hit, the "not implemented" comment at `XLImportSession.h:154` |
| 13 | Save / load `.xmaphint` (regex hint file) | `SaveMapping` `:2336`, `LoadMappingFile` `:1967`; core `MapHintsIO` (`src-core/import_export/MapHintsIO.cpp`) | ✅ | Save `ImportEffectsView.swift:356,382`; load `:358,388-392` → `XLImportSession.h:156` `loadMapHints(fromPath:)` |
| 14 | Load mapping — text `.xmap` | `LoadMapping` `:1919` (filter `:1926`), `LoadXMapMapping` `:2224` | ❌ | Load picker accepts only `xmaphint` (`ImportEffectsView.swift:389`); no `LoadXMap` equivalent in `XLImportSession.h` |
| 15 | Load mapping — JSON `.xjmap` | `LoadJSONMapping` `:2065` | ❌ | Same evidence as row 14 |
| 16 | **Multi-file** hint load (selecting several `.xmaphint` in one go) | — desktop `wxFileDialog` at `:1926` is single-select (`wxFD_OPEN \| wxFD_FILE_MUST_EXIST`, no `wxFD_MULTIPLE`), so a load applies exactly one file | 🔵 | iPad-only: `allowsMultipleSelection: true` (`ImportEffectsView.swift:390`), hints from every picked file applied in one pass (`applyHints` `:460`). **The keep-vs-overwrite prompt itself is *not* iPad-only** — desktop shows the same choice before applying (`LoadMappingFile` `xLightsImportChannelMapDialog.cpp:1947-1961`, "Clear & Replace" / "Merge" / "Cancel", `ClearAll()` on replace `:1957`), matching the iPad's Keep Existing / Overwrite confirmation (`:414-426`) which calls `clearAllMappings` on overwrite (`:462`) |
| 17 | Mapping onto an already-mapped row → Replace / Add Additional (stacked sources) | `PromptAndApplyMapping` `:3197`, `InsertStackDuplicate` `:3122` | ✅ | `ImportEffectsView.swift:402-412` → `applyPendingMap(stack:)` `:569` → `XLImportSession.h:108` `addStackedMapping(forRow:…)`; stacked entries rendered `ImportEffectsView.swift:827-831` |
| 18 | Stashed-mappings warning + viewer (mappings a load couldn't apply) | `UpdateStashWarning` `:2016`, `OnStashWarningClick` `:2028`, button `ID_BITMAPBUTTON_STASH` `:528` | ❌ | No stash concept. `grep -rn "stash\|Stash" src-iPad/` → no matches; unapplied hints are only summarised as a count (`ImportEffectsView.swift:472-474`) |
| 19 | "Erase existing effects on imported models" toggle | `CheckBox_EraseExistingEffects` `:607` | ✅ | `ImportEffectsView.swift:364`, persisted `@AppStorage` `:48`, passed at `:645` |
| 20 | "Lock effects on import" toggle | `CheckBox_LockEffects` `:610`, `IsLockEffects` `:1446` | ✅ | `ImportEffectsView.swift:365`, `@AppStorage` `:49`, passed at `:647` |
| 21 | "Convert Render Style" toggle (model→group renders as Per Model) | `CheckBox_ConvertRenderStyle` `:613`, tooltip `:615` | ✅ | `ImportEffectsView.swift:366`, `@AppStorage` `:50`, passed at `:647`; maps to core `MapXLightsEffects(convertRender)` (`XLImportSession.h:185-186`) |
| 22 | "Import Model Blend Mode" toggle + source-blend indicator text | `CheckBox_Import_Blend_Mode` `:619`, `StaticText_Blend_Type` `:622`, `SetModelBlending` `:1435`, `GetImportModelBlending` `:1441` | ❌ | No blend-mode import control or source-blend banner. `grep -rn "Import.*[Bb]lend\|modelBlending" src-iPad/App/ImportEffectsView.swift` → no matches; `applyImportWithEraseExisting:…` (`XLImportSession.h:192-196`) has no blend parameter |
| 23 | "Import matrix face definitions into the sequence (embedded)" toggle | `CheckBox_ImportFacesToSequence` `:626`, tooltip `:628`, hidden for non-xLights sources `InitImport:1182-1185` | ✅ | `ImportEffectsView.swift:367-370`, gated by `sourceHasRGBEffects` (`:532`, `XLImportSession.h:179`), applied via `SequencePackage::SetImportFacesToSequence` (`XLImportSession.mm:2105`) |
| 24 | "Import Media" on/off toggle | `CheckBoxImportMedia` `:632`, `OnCheckBoxImportMediaClick` `:4172` → `SeqPkgImportOptions::SetImportActive`; hidden for non-package sources `InitImport:1175-1180` | 🟡 | Core-shared, no iPad UI. The iPad apply path reuses the package's default `SeqPkgImportOptions` (`XLImportSession.mm:2104`) and never calls `SetImportActive` — `grep -n "ImportOptions\|SetImportActive\|ImportMedia" src-iPad/Bridge/XLImportSession.mm` → no matches. **Gap:** media import can't be turned off |
| 25 | Media import target-directory options (Faces / Glediators / Images / Shaders / Videos, Restore Defaults) | `ButtonImportOptions` `:635` → `OnButtonImportOptionsClick` `:4163` → `MediaImportOptionsDialog` (`src-ui-wx/media/MediaImportOptionsDialog.cpp:62-120`); core state `src-core/render/SequencePackage.h:53-65` | 🟡 | Core-shared (`SeqPkgImportOptions`), no iPad UI. Same empty grep as row 24 |
| 26 | "Map CCR/Strand" toggle (fan a source model out per-pixel/per-strand) | `CheckBox_MapCCRStrand` `:602`, `OnCheckBox_MapCCRStrandClick` `:3243`, `PopulateAvailable(ccr)` `:1459`, label override `InitImport:1244` | ❌ | No CCR/strand toggle. `grep -rn "ccrStrand\|MapCCR" src-iPad/` → no matches (the `CCR` hits in `XLImportSession.mm:187-204,665` are internal per-reader CCR-prop *detection* for `.lms`/`.hlsIdata`, not a user control) |
| 27 | Time Adjust (ms) offset applied to imported effects | `TimeAdjustSpinCtrl` `:597` (range −10000…600000), hidden when `!_allowTimingOffset` `:1201` | ❌ | Hard-coded to 0 on iPad, and the source says so: `XLImportSession.mm:1877` `int const offset = 0; // iPad has no time-offset control yet` and `XLImportSession.mm:1985` |
| 28 | Per-timing-track import selection (with "already exists" defaulting) | `TimingTrackListBox` `:639`, checkbox grid + default `InitImport:1204-1240` | ✅ | `XLImportSession.h:30-34,91,95`; popover UI `ImportEffectsView.swift:344-355,677-709` |
| 29 | Timing-track Select All / Select None | `RightClickTimingTracks` `:775-782`, `OnPopupTimingTracks` `:1145` | ❌ | Per-track toggles only (`ImportEffectsView.swift:685-705`); no bulk action. `grep -n "Select All\|selectAll" src-iPad/App/ImportEffectsView.swift` → no matches |
| 30 | Find / narrow the destination list | `TextCtrl_FindTo` `:646` → `OnTextCtrl_FindToText` (scroll-to-first-match) | ✅ | Different idiom, same job: incremental filter `ImportEffectsView.swift:305`, `filteredDestinationRows` `:84-90` |
| 31 | Find / narrow the available-source list | `TextCtrl_FindFrom` `:701` → `OnTextCtrl_FindFromText` `:4432` | ✅ | Incremental filter `ImportEffectsView.swift:257`, `filteredSources` `:62-67` |
| 32 | "Hide Unmapped" destination filter | `CheckBox_HideUnmapped` `:1257`, binds `_dataModel->SetHideUnmapped` `:1264` | ❌ | `grep -rn "hideUnmapped\|Hide Unmapped" src-iPad/` → no matches |
| 33 | "Show Timeline" — per-source effect timeline column | `CheckBox_ShowTimeline` `:1276`, persisted `ImportShowTimeline` `:1277`, repopulates `PopulateAvailable` `:1283` | 🟡 | A single-bar density strip (`SourceTimelineStrip` `ImportEffectsView.swift:883-902`) driven by `effectCount`/`durationMs` (`XLImportSession.h:21-22`). **Gap:** no per-effect rectangles, no on/off toggle |
| 34 | Collapse All / Expand All in the destination tree | `CollapseAll` `:971`, `ExpandAll` `:980`, menu `:788-789` | ❌ | Per-row disclosure only (`ImportEffectsView.swift:806-815`). `grep -n "collapseAll\|expandAll" src-iPad/App/ImportEffectsView.swift` → no matches (the `collapseAll`/`expandAll` hits elsewhere in `src-iPad/` are the sequencer grid, `SequencerGridV2View.swift`) |
| 35 | "Show All Mapped Models" | `ShowAllMapped` `:1125`, menu `:790` | ❌ | `grep -rn "showAllMapped\|Show All Mapped" src-iPad/` → no matches |
| 36 | Sort Submodels By Name (+ Reset Submodel Sort) | menu `:792-797` → `_dataModel->GetSortSubmodelsByName()` | 🟡 | Sort only: `ImportEffectsView.swift:848-858` → `XLImportSession.h:117` `sortSubmodels(forRow:)`. **Gap:** no "Reset Submodel Sort" inverse |
| 37 | "Add Empty Group" as a mapping target | `AddEmptyGroup` `:924`, menu `:803` | ❌ | `grep -rn "addEmptyGroup\|Add Empty Group" src-iPad/` → no matches |
| 38 | "Edit Display Elements…" without leaving the import wizard | `EditDisplayElements` `:880`, menu `:805`; re-adds new master-view items `AddNewMasterViewItems` `:893` | ✅ | `ImportEffectsView.swift:361,429-435` → `DisplayElementsSheet`, then `XLImportSession.h:124` `rebuildDestinationTree` re-keys mappings by name |
| 39 | Sort the available-source list by column | `OnListCtrl_AvailableColumnClick` `:2706` | ❌ | Fixed order from `availableSources()` (`ImportEffectsView.swift:261`); no sort control. `grep -n "sort" src-iPad/App/ImportEffectsView.swift` → only `sortSubmodels` |
| 40 | Bulk-map all nodes / submodels of a dropped source model | `BulkMapNodes` `:3014`, `BulkMapSubmodelsStrands` `:3039`, reached from `HandleDropAvailable` `:3060` | ❌ | Follows from row 3 — drop handling is the only trigger and there is no drag-and-drop on iPad; `grep -n "bulkMap\|BulkMap" src-iPad/` → no matches |
| 41 | Pre-import version / frame-rate mismatch warning | `ImportEffects.cpp:187-192` (version-newer + higher-FPS message boxes) | ✅ | `checkSourceWarnings` `ImportEffectsView.swift:543-550` using `XLImportSession.h:260-262` (`sourceVersion` / `sourceFrequency` / `targetFrequency`) |
| 42 | Post-import missing-source-media report | `SequencePackage` media walk; surfaced during `ImportXLights` | ✅ | `ImportEffectsView.swift:657-664` using `XLImportSession.h:263` `sourceMissingMedia` |
| 43 | Mapped / total counters | dialog title + tree state | ✅ | `ImportEffectsView.swift:249-251` (source count) and `:299` (`N / M mapped`) |
| 44 | Remember last import directory + filter | `xLightsLastImportType` / `xLightsLastImportDir` (`ImportEffects.cpp:99-127`) | 🟡 | Option toggles persist (`@AppStorage`, `ImportEffectsView.swift:48-51`) but the picker location doesn't — iOS `.fileImporter` (`:175`) has no equivalent seed. Low impact: the system picker remembers its own last location |
| 45 | Missing-model reconciliation on sequence open reuses the map dialog | `CheckForValidModels` → `ImportXLights(_sequenceElements, modelElements, …)` (`src-ui-wx/sequencer/tabSequencer.cpp:744`) | 🟡 | iPad substitutes a simpler sheet — rename-to-model / delete / add-alias per row (`src-iPad/App/MissingModelAliasSheet.swift:9-36`). **Gap:** cannot map the orphan's effects onto a different model at open time |
| **SuperStar import** — desktop `SuperStarImportDialog`, iPad `SuperStarImportView` + `XLSuperStarImport` | | | | |
| 46 | Target-model picker including submodels / strands | `SuperStarImportDialog.cpp:65`; populated with submodel full names `ImportEffects.cpp:1193-1205` | ✅ | `SuperStarImportView.swift:66-80` populated by `XLSuperStarImport.mm:66-87` (same submodel walk) |
| 47 | X / Y size and X / Y offset | `SuperStarImportDialog.cpp:73,77,82,86` | ✅ | `SuperStarImportView.swift:82-109` → `XLSuperStarImportOptions` `:169-172` |
| 48 | Image resizing mode (None / Exact Width / Exact Height / Exact W-or-H / All) | `SuperStarImportDialog.cpp:93-98` | ✅ | `SuperStarImportView.swift:112-118` (same five cases) → `:173` |
| 49 | Layer blend (Average / Normal / 2 reveals 1) | `SuperStarImportDialog.cpp:107-110` (default "2 reveals 1") | ✅ | `SuperStarImportView.swift:37-41,119-123` (same default at `:32`) |
| 50 | Timing adjustment (ms) | `SuperStarImportDialog.cpp:102` (−60000…60000) | ✅ | `SuperStarImportView.swift:124-128` (same range) → `:175` |
| 51 | Embedded-image group name / prefix | Prompted on demand via `SetPrefixPromptCallback` (`ImportEffects.cpp:1288-1297`), default = source filename stem (`opt.defaultGroupName` `:1286`) | ✅ | Up-front field instead of a prompt: `SuperStarImportView.swift:129-136`, placeholder = file stem `:47-49`, passed at `:176` |
| **Exports** | | | | |
| 52 | House-preview video export | `File → Export House Preview Video` (`src-ui-wx/xLightsMain.cpp:954`) → `OnMenuItem_File_Export_VideoSelected` `:3681` → `ExportVideoPreview` `:3695` | ✅ | `Tools → Export House Preview…` (`src-iPad/App/CommandPaletteRegistry.swift:178`) → `ExportHousePreviewSheet.swift:20` → `SequencerViewModel.exportHousePreview` (`SequencerViewModel.swift:5440`) → `XLHousePreviewVideoExporter.mm:91` |
| 53 | House-preview export — audio muxed in | `xLightsMain.cpp:3733-3762` (`setGetAudioCallback`) | ✅ | `XLHousePreviewVideoExporter.mm:116-120,189,342-366` (same `AudioManager` raw-float pull) |
| 54 | House-preview export — output resolution choice | — desktop always uses the on-screen preview size (`xLightsMain.cpp:3727-3728` `housePreview->getWidth()/getHeight()`) | 🔵 | iPad-only: 720p / 1080p / 4K / Match-preview picker (`ExportHousePreviewSheet.swift:24-53`), rendered offscreen independent of the pane (`XLSequenceDocument.h:683-686`) |
| 55 | House-preview export — codec / bitrate selection | Preference-driven: `_videoExportCodec` / `_videoExportBitrate` (`xLightsMain.cpp:1775-1779`), passed at `:3745` | 🟡 | iPad exposes an in-sheet H.264 / HEVC picker (`ExportHousePreviewSheet.swift:76-84` → `XLHousePreviewVideoExporter.mm:195`) but leaves bitrate/quality on "Auto" (`:191-195`). **Gap:** no bitrate control; conversely desktop has no in-dialog codec choice |
| 56 | House-preview export — cancellable progress | `videoExporter.Export(_appProgress.get())` `xLightsMain.cpp:3776` | 🟡 | Progress reported (`ExportHousePreviewSheet.swift:85-90,129`) and dismissal blocked (`:111`), but no cancel button. **Gap:** cannot abort a long encode |
| 57 | Export model → FPP sub-sequence `.eseq` (uncompressed v1) | `SeqExportDialog.cpp:119` "FPP Sub sequence. *.eseq"; `RenderUI.cpp:496-501` (`v2 = format.find("Compressed")`) → `WriteFalconPiModelFile` (`src-ui-wx/app-shell/TabConvert.cpp:603`) | ❌ | iPad always writes v2+zstd: `XLSequenceDocument.mm:1883-1885` (`createFSEQFile(outPath, 2, CompressionType::zstd, -99)`); no uncompressed option in `RowHeaderViews.swift:712-728` |
| 58 | Export model → FPP compressed sub-sequence `.eseq` (v2) | `SeqExportDialog.cpp:120`; `RenderUI.cpp:501` | ✅ | `RowHeaderViews.swift:712-714` "Export Model as FSEQ…" → `SequencerViewModel.exportModelAsFSEQ` (`SequencerViewModel.swift:5371`) → `XLSequenceDocument.mm:1837` |
| 59 | Export model → Compressed Video `.mp4` (H.264) | `SeqExportDialog.cpp:78`; `RenderUI.cpp:502-506` | ✅ | `RowHeaderViews.swift:732-734` → `XLSequenceDocument.h:643`; shared encoder `src-core/render/ModelVideoExporter.cpp:178` |
| 60 | Export model → High Quality Video `.mp4` (HEVC, constant quality) | `SeqExportDialog.cpp:79`; `RenderUI.cpp:507-512` | ✅ | `RowHeaderViews.swift:735-737`; `ModelVideoExporter.cpp:169-177` |
| 61 | Export model → HD ProRes `.mov` (ProRes 4444 upscaled to 1920×1080) | `SeqExportDialog.cpp:80`; `RenderUI.cpp:529-538` | ✅ | `RowHeaderViews.swift:741-743` (passes 1920/1080) → `XLSequenceDocument.h:665-671` |
| 62 | Export model → ProRes 4444 `.mov` | `SeqExportDialog.cpp:84`; `RenderUI.cpp:522-528` | ✅ | `RowHeaderViews.swift:738-740`; `ModelVideoExporter.cpp:163-165` |
| 63 | Export model → Lossless RGB `.mov` (rawvideo / ProRes by width) | `SeqExportDialog.cpp:85`; `RenderUI.cpp:517-521` | ✅ | `RowHeaderViews.swift:744-746`; codec policy `ModelVideoExporter.cpp:166-167` |
| 64 | Export model → Uncompressed Video `.avi` | `SeqExportDialog.cpp:81-83` — Windows/Linux only (`#ifndef __APPLE__`); rawvideo via FFmpeg (`ModelVideoExporter.cpp:168-169`) | 🚫 | Infeasible on iPad: the AVI/rawvideo path is FFmpeg-only and iOS builds link **no** FFmpeg — `src-core/media/VideoWriter.cpp:36-41` selects `AVFoundationVideoWriter` alone under `TARGET_OS_IPHONE` ("iPad: AVFoundation only, no FFmpeg"). Also already absent on macOS desktop |
| 65 | Export model → animated GIF | `SeqExportDialog.cpp:87`; `RenderUI.cpp:544-548` → `WriteGIFModelFile` (`TabConvert.cpp:909`, delegating to core `ModelGifExporter`) | ✅ | `RowHeaderViews.swift:751-755` → `SequencerViewModel.exportModelAsGif` (`SequencerViewModel.swift:5420`) → `XLSequenceDocument.h:678`; same core encoder |
| 66 | Export model → LOR Clipboard `.lcb` (v1) | `SeqExportDialog.cpp:71`; `RenderUI.cpp:476-483` → `WriteLcbFile` (`TabConvert.cpp:422`) | ❌ | `grep -rli "lcb" src-iPad/` → **no matches**; not in the row-header export menu (`RowHeaderViews.swift:712-755`). Writer lives in `src-ui-wx/`, not core |
| 67 | Export model → LOR S5 Clipboard `.lcb` (v2) | `SeqExportDialog.cpp:72`; `RenderUI.cpp:479-483` (`lcbVer = 2` at `:481`) | ❌ | Same empty grep as row 66 |
| 68 | Export model → Vixen Routine `.vir` | `SeqExportDialog.cpp:74`; `RenderUI.cpp:484-487` → `WriteVirFile` (`TabConvert.cpp:121`) | ❌ | `grep -rn "\.vir" src-iPad/` → one false positive (`XLMetalBridge.mm`, unrelated); no export entry |
| 69 | Export model → LSP UserPatterns `.xml` | `SeqExportDialog.cpp:75`; `RenderUI.cpp:449-451,488-491` → `WriteLSPFile` (`TabConvert.cpp:142`) | ❌ | `grep -rli "UserPatterns" src-iPad/` → no matches |
| 70 | Export model → HLS `.hlsnc` | `SeqExportDialog.cpp:76` ("HLS, Hinkle Lighte Sequencer"); `RenderUI.cpp:492-495` → `WriteHLSFile` (`TabConvert.cpp:389`) | ❌ | `grep -rli "hlsnc" src-iPad/` → no matches |
| 71 | Export model → Minleon Network Effects Controller `.bin` | `SeqExportDialog.cpp:86`; `RenderUI.cpp:539-543` → `WriteMinleonNECModelFile` (`TabConvert.cpp:918`) | ❌ | `grep -rli "Minleon" src-iPad/` → no matches |
| 72 | "Render and Export" — force a full render before writing | `RowHeading.cpp:751`; `command.GetInt() == 1` → `DoExportModel(…, doRender = true)` → `RenderAll()` (`RenderUI.cpp:462-467`) | 🟡 | iPad exports whatever the continuous background render has produced — `XLSequenceDocument.h:620-624` explicitly says "callers should ensure a full render has completed first". **Gap:** no export-time render trigger |
| 73 | Export only the selected effects' time span | `RowHeading.cpp:752-753`, frame range packed into the event `:1994-2005` | ✅ | `RowHeaderViews.swift:723-727` ("Export Selected Model Effects as FSEQ…"), plus an iPad loop-range variant `:717-721` (`XLSequenceDocument.h:630-632` takes `startMS`/`endMS`) |
| 74 | Group-model gating of video/GIF/Minleon exports | `SeqExportDialog::ModelExportTypes(isgroup)` `SeqExportDialog.cpp:100-115`; `ExportModel` `RenderUI.cpp:580,588`; `DoExportModel` early-returns for groups `:440-441` | ✅ | `RowHeaderViews.swift:730,751` gate video + GIF on `!isGroup`; bridge re-checks (`XLSequenceDocument.h:640-642`) |
| 75 | Export Models spreadsheet (`.xlsx`) | `File → Export Models` (`xLightsMain.cpp:1032`) → `OnmExportModelsMenuItemSelected` `:5035` → core `ExportModels` (`src-core/import_export/ExportModels.cpp`, libxlsxwriter at `:29`) | ✅ | `LayoutEditorView.swift:1936-1947` → `XLSequenceDocument.h:3571` `exportModelsReport(toPath:)` → same core `::ExportModels` (`XLSequenceDocument.mm:18874`) |
| 76 | Export Effects report (`.csv`) | `Tools → Export Effects` (`xLightsMain.cpp:1034`) → `OnMenuItem_ExportEffectsSelected` `:5757` → core `ExportEffects` (`src-core/import_export/ExportEffectsReport.cpp`) | ✅ | `XLightsCommands.swift:458` → `SequencerView.swift:307,358-365` → `XLSequenceDocument.h:3578` `exportEffectsReport(toPath:)` → same core `::ExportEffects` (`XLSequenceDocument.mm:18888`) |
| 77 | Export the sequence's current audio | — no desktop counterpart (`grep -rn "ExportAudio\|Export.*Audio" src-ui-wx/` finds only the video-mux `setGetAudioCallback` at `xLightsMain.cpp:3764` and `VideoExporter.cpp:64`) | 🔵 | iPad-only: `Tools → Export Audio` (`src-iPad/App/ExportAudioSheet.swift:3-9`) → `SequencerViewModel.exportCurrentAudio` (`SequencerViewModel.swift:6799`) → `XLSequenceDocument.h:824`; exports the waveform as shown, including an isolated stem / filter band (`ExportAudioSheet.swift:42`) |
| 78 | Delivering an exported file (save panel vs. share sheet) | `wxFileDialog` / `wxFileSelector` writes straight to the chosen path (e.g. `xLightsMain.cpp:3685`, `:5758`) | 🔵 | iPad writes to a temp file then presents the system share sheet — Save to Files, AirDrop, Mail, etc. (`ExportHousePreviewSheet.swift:120-143` → `ShareSheetPresenter.swift:12-41`), or `.fileExporter` for report-style outputs (`LayoutEditorView.swift:1944-1946`) |
| **Legacy Convert tab (channel-data file converter)** | | | | |
| 79 | Convert tab itself (multi-file batch, progress log, media prompt) | `Tools → Convert` `xLightsMain.cpp:1062` → `OnMenuItemConvertSelected` `:4207` → `ConvertDialog` (`src-ui-wx/import_export/ConvertDialog.cpp`); multi-select input `:172`, `ConvertLogDialog` | ❌ | No converter on iPad — `grep -rli "ConvertDialog\|FileConverter" src-iPad/` yields only an unrelated comment (`iPadRenderContext.cpp:2471`). `ConvertEffectTypeSheet.swift` is an *effect-type* converter, unrelated |
| 80 | Convert output → xLights/FPP `.fseq` | `ConvertDialog.cpp:107` | ❌ | Follows from row 79 (whole tab absent). Note the iPad does render `.fseq` — that's theme 09, not a Convert equivalent |
| 81 | Convert output → Lynx Conductor `.seq` | `ConvertDialog.cpp:108` | ❌ | Follows from row 79; `grep -rli "Conductor" src-iPad/` → no matches |
| 82 | Convert output → Vixen 2.1 `.vix` | `ConvertDialog.cpp:109`; `WriteVixenFile` `:324` | ❌ | Follows from row 79 |
| 83 | Convert output → Vixen Routine `.vir` | `ConvertDialog.cpp:110`; `WriteVirFile` `:417` | ❌ | Follows from row 79 (see also row 68) |
| 84 | Convert output → LOR `.las` / `.lms` | `ConvertDialog.cpp:111`; `WriteLorFile` `:468`; `LorConvertDialog` (`src-ui-wx/import_export/LorConvertDialog.cpp`) | ❌ | Follows from row 79; `grep -rli "LorConvert" src-iPad/` → no matches |
| 85 | Convert output → Glediator Record `.gled` | `ConvertDialog.cpp:112` | ❌ | Follows from row 79 |
| 86 | Convert output → LOR Clipboard `.lcb` | `ConvertDialog.cpp:113`; `WriteLcbFile` `:626` | ❌ | Follows from row 79 (see also row 66) |
| 87 | Convert output → HLS `.hlsnc` | `ConvertDialog.cpp:114`; `WriteHLSFile` `:422` | ❌ | Follows from row 79 (see also row 70) |
| 88 | Convert output → LedBlinky Animation `.lwax` | `ConvertDialog.cpp:115` | ❌ | Follows from row 79; `grep -rli "lwax" src-iPad/` → no matches |
| **Import mapping dialog (continued)** | | | | |
| 89 | "Used" marking — already-mapped sources greyed out in the available list (unmapped model *groups* additionally tinted) | `MarkUsed` `xLightsImportChannelMapDialog.cpp:3262`, recolour loop `:3299-3315` (`LightOrMediumGreyOverride` for used, `CyanOrBlueOverride` for unused groups); re-run after every map/unmap/clear/auto-map — `:1052`, `:1122`, `:1785`, `:1801`, `:1811`, `:2011`, `:2981`, `:3163`, `:4067`, `:4087`, `:4110`, `:4566` | ❌ | No used/unused distinction in the iPad source list. `grep -n "isUsed\|Used\|greyed\|grey\|gray" src-iPad/App/ImportEffectsView.swift` → **zero matches**, and `XLImportAvailableSource` (`XLImportSession.h:12-23`) carries only `displayName` / `canonicalName` / `modelType` / `effectCount` / `durationMs` — no used flag. The source row body (`ImportEffectsView.swift:261-286`) draws the name plus the row-33 density strip; its only conditional styling is the tap-selection highlight `:280`, so a source already mapped onto another destination looks identical to an unmapped one |

## Desktop platform differences (macOS vs Windows vs Linux)

| Behavior | macOS | Windows | Linux | Evidence |
|---|---|---|---|---|
| "Uncompressed Video, *.avi" model-export format | absent | offered | offered | `src-ui-wx/import_export/SeqExportDialog.cpp:82-83` (`#ifndef __APPLE__`) |
| Video encoder backend for model / house-preview export | AVFoundation, with a transparent FFmpeg fallback if AVFoundation fails to initialize | FFmpeg | FFmpeg | `src-core/media/VideoWriter.cpp:36-41` (backend selection), `:103-121` (`initialize()` fallback, `#if defined(__APPLE__) && !TARGET_OS_IPHONE`) |
| Mapping tree drag-and-drop implementation | native `wxDataViewCtrl` DnD (`EnableDropTarget` + `EVT_DATAVIEW_ITEM_DROP`) | custom `MDTextDropTarget` | custom `MDTextDropTarget` | `xLightsImportChannelMapDialog.cpp:1347-1354` (`#ifdef __WXOSX__`) |
| Drop-index guard while dragging over the mapping tree | rejects drops with a proposed insert index (between-rows drops) | n/a | n/a | `xLightsImportChannelMapDialog.cpp:2800-2806` (`#ifdef __WXOSX__`) |
| Mapping tree re-sort timing after model insertion | resort at build time | resort after `TreeContainsModel` miss | resort at build time | `xLightsImportChannelMapDialog.cpp:1307-1311` (`#ifndef __WXMSW__`) and `:1425-1431` (`#ifdef __WXMSW__`) |
| "Find" with an empty box scrolls the mapping tree back to the top | no (no API) | yes | no (no API) | `xLightsImportChannelMapDialog.cpp:4483-4490` (`#ifdef __WXMSW__`), with the in-source note that `wxDataViewCtrl` has no portable scroll |
| Model-video export: UI yielding during the render wait | none | `wxSafeYield` each poll | none | `src-ui-wx/app-shell/TabConvert.cpp:707-711` and `:896-900` (`#ifdef _WIN32`) |
| House-preview video export: content-scale (Retina/HiDPI) factor | preview's `GetContentScaleFactor()` honoured (exports at backing-store resolution) | forced to 1.0 | preview's factor honoured | `src-ui-wx/xLightsMain.cpp:3727-3731` (`#ifdef _WIN32 contentScaleFactor = 1.`) |
| zstd include mode used by the converter/exporter chain | `ZSTD_STATIC_LINKING_ONLY` (thread-pool API) | `ZSTD_STATIC_LINKING_ONLY` | plain `<zstd.h>` — Debian 11's zstd lacks the thread-pool API | `src-ui-wx/import_export/FileConverter.cpp:22-28` (`#ifndef LINUX`) |
| Sandbox bookmark acquisition before writing an export | required (`ObtainAccessToURL`, e.g. `RenderUI.cpp:593`) | no-op | no-op | `RenderUI.cpp:593`; `ObtainAccessToURL` is a no-op off macOS (per AGENTS.md §6) — the iPad shares this requirement, e.g. `XLSequenceDocument.mm:18873` |

Not a platform split but a **build-flavor** one worth recording alongside these: Convert's Lynx Conductor `.seq`
reader is compiled out of FPP builds (`src-ui-wx/import_export/ConvertDialog.cpp:1690` `#ifndef FPP`).

## Notes

- **Import readers are already core-shared; exporters largely are not.** Every effect-import reader lives in
  `src-core/import_export/` (`LOREdit`, `LORMusic`, `LORPixelEditor`, `HLSFile`, `Vixen2File`, `Vixen3`,
  `SuperStarImporter`, `AutoMapper`, `EffectMapper`, `MapHintsIO`), which is why the iPad reached 11/11 of the
  formats desktop's Import Effects offers. The two misses are exactly the two whose readers/dialogs never moved:
  `.msq` is parsed inline in `ImportEffects.cpp:3429` and `.vsa` in `src-ui-wx/import_export/VSAFile.cpp`. On the
  export side the reverse holds — only `ModelVideoExporter` and `ModelGifExporter` are in `src-core/render/`; the
  `.lcb`, `.vir`, LSP, `.hlsnc` and Minleon writers are `xLightsFrame` methods in
  `src-ui-wx/app-shell/TabConvert.cpp:121-950`, so nothing on iPad can call them.
- **Suggested porting order within this theme.** (1) `XLImportSession` time-offset — the two `int const offset = 0`
  sites (`XLImportSession.mm:1877`, `:1985`) are already parameterised, so this is a bridge argument plus one
  field. (2) Wire the existing but unreferenced bridge ops: `runAutoMapSelectedTargets:sources:`
  (`XLImportSession.h:134`) and a user-facing Clear All / Clear Selected on top of `clearAllMappings`
  (`XLImportSession.h:163`) — no new core work. (3) Map CCR/Strand: the readers already compute CCR names
  (`XLImportSession.mm:187-204`), so this is a `PopulateAvailable(ccr)` equivalent plus a toggle. (4) Import-media
  on/off and the media target-dir sheet — pure UI over the existing `SeqPkgImportOptions`
  (`src-core/render/SequencePackage.h:53-65`). (5) `.xmap` / `.xjmap` load/save: needs a new wx-free
  `MappingIO` core module (the bridge header flags this itself at `XLImportSession.h:154-155`); doing it unlocks
  four rows at once (11, 12, 14, 15). (6) Lift `WriteLcbFile` / `WriteVirFile` / `WriteLSPFile` / `WriteHLSFile` /
  `WriteMinleonNECModelFile` out of `TabConvert.cpp` into `src-core/` — they already take plain
  `SequenceData*`/`Model*`, so the move is mostly mechanical and would let both platforms share one implementation.
- **Three desktop `SeqExportDialog` entries are unreachable dead code.** "LOR. *.lms or *.las"
  (`SeqExportDialog.cpp:70`), "Vixen, Vixen sequence file *.vix" (`:73`) and "xLights/FPP, *.fseq" (`:77`) are
  added by the constructor but deleted again by `ModelExportTypes` (`:116-118`), and the only construction site
  (`RenderUI.cpp:587`) always calls `ModelExportTypes` on the next line (`:588`). They are therefore not counted as
  desktop capabilities above. Worth deleting or re-plumbing on the desktop side regardless of iPad parity.
- **Two structurally different import UIs, both fed by one bridge.** `ImportEffectsView` hosts the channel-mapper
  for ten of the eleven formats and hands `.sup` off to `SuperStarImportView` inside the same sheet
  (`ImportEffectsView.swift:110-126`) — mirroring desktop, where `ImportSuperStar` opens its own dialog rather
  than the map-channels dialog. Adding `.msq` would reuse the mapper wholesale (desktop routes it through
  `LMSImportChannelMapDialog`, the same shape); `.vsa` would need a third form (per-track model + layer + channel
  picker), matching desktop's separate `VsaImportDialog`.
- **`SourceEffectsSheet.swift` is not an import surface.** Despite the name it renders "Find Possible Source
  Effects" results for a node row (`SourceEffectsSheet.swift:3-6`) — a sequencer-grid feature (theme 02).
  Likewise `MissingModelAliasSheet.swift` belongs to sequence-open (theme 01); it appears here only as row 45
  because desktop reuses the *import* mapping dialog for that job.
- **Export delivery is idiomatically different and that's fine.** Desktop writes directly to a chosen path;
  iPad writes a temp file then shares it (rows 77-78). The one place this leaks into capability is cancellation
  and progress: the desktop model-video export drives a cancellable `wxProgressDialog`
  (`TabConvert.cpp:748-757`), while the iPad's async encodes report completion only
  (`XLSequenceDocument.h:654-659`).
