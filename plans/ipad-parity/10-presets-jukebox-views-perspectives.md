# 10 · Presets, Jukebox, Display Elements, Views & Perspectives

> The **effect-preset library** is at strong parity:
> iPad's `PresetBrowserSheet` + the `XLSequenceDocument (EffectPresets)`
> bridge implement apply / save / group / rename / delete / import /
> export against the same `EffectPresetManager` core the desktop
> `EffectTreeDialog` uses. **Update Preset**, **search**, **reorder**
> (Move Up/Down within a group + Move to Group across groups), and
> **name-collision validation**, the **Relative vs Using Layers** paste
> mode (segmented Auto/Relative/Using-Layers picker with `\tLAYER:`
> auto-detect), and the read-only **"From Base"** section (base show
> folder's preset library, apply-only) are now at parity in
> `PresetBrowserSheet`. The preset preview now shows a **static
> representative still** per row (bridge `presetThumbnailBGRA` renders the
> preset's anchor effect on the shared preset matrix model and caches the
> last frame as the row icon); the only remaining preset gap is
> *animating* that still (multi-frame GIF playback).
> **Display Elements / Views** is now at near-full parity:
> in addition to create / rename / clone / delete / reorder views,
> add-to / remove-from Master + user views, visibility toggles,
> timing-track add/remove, filter, **Copy To Master**, **Show/Hide All
> + Hide/Remove Unused** bulk ops, and **Import view config from another
> `.xsq`**, the latest pass added **sort strategies** (8 sort modes via a
> "Sort" toolbar menu in the Master-View pane, matching
> `ViewsModelsPanel.cpp:1667-1682`), **keyboard delete** (hardware Delete
> key via `.keyboardShortcut` on the selected member), **effect-sequence
> mode** (Views sidebar hidden when `sequenceType == "Effect"`), and
> **per-view checkbox at parity** (the desktop checkbox is a view
> selector, not a visibility toggle — iPad sidebar selection is the
> equivalent). The RGBEffects-layout import variant and per-panel
> undo remain desktop-only. **Row headings** now include the **has-effects
> amber stripe** and the **model tag-color chip** (two new bridge methods
> `rowHasEffects(at:)` and `rowTagColor(at:)` + `RowInfo` fields), plus
> the **single-color model swatch** (bridge `rowNodeMaskColor(at:)` →
> `GetNodeMaskColor(0)`, drawn as a 12×12 trailing swatch chip in
> `ModelRowHeader`).
> **Jukebox is 100% desktop-only** — no iPad UI, bridge, or VM (core
> `JukeboxButtonData` exists and is loaded/saved with the sequence, so
> the data round-trips, but nothing on iPad surfaces it); **declined
> 2026-06-11 — won't build on iPad** (low touch value; data continues to
> round-trip safely). **AUI
> perspectives are architecturally infeasible** on the fixed SwiftUI
> layout; iPad instead has its own detach-to-window scene system that
> desktop covers via AUI float/dock. The desktop **AUI toolbar set**
> (File / Playback / AC / View / Edit toolbars, Reset Toolbars) maps to
> iPad's native menu bar + floating toolbar buttons — mostly parity by
> different idiom (the Playback toolbar's **Replay Section** now ships
> on iPad), with the **AC toolbar** being the only real
> desktop-only sequencing surface. **Find / Replace** is at full parity
> (both restrict to timing-track labels). The desktop **View > Windows**
> submenu exposes ~16 panel-visibility toggles; iPad collapses these to
> Show/Hide Preview + Show/Hide Inspector + detach scenes.

## Parity scorecard

| Feature | Surface | Desktop | iPad | Gap | Priority | Ease | Feasibility | Notes |
|---|---|---|---|---|---|---|---|---|
| Effect preset browser / tree | panel | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `EffectTreeDialog` wxTreeCtrl; iPad `PresetBrowserSheet` flat indented List. Same core `EffectPresetManager`. |
| Create preset (save selection) | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `OnbtNewPresetClick`+`PromptForName`; iPad "Save Selection as Preset" alert → `saveSelectedEffectAsPreset`. |
| Save preset into a group | dialog | ✅ | ✅ | parity | P2 | easy | feasible | iPad group context-menu "Save Selection Here"; desktop respects selected group. |
| Add preset group / subgroup | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `OnbtAddGroupClick`; iPad "New Group"/"New Subgroup" → `addPresetGroup(named:inGroupPath:)`. |
| Rename preset / group | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop Rename button; iPad context-menu Rename → `renamePreset(atPath:to:)`. |
| Delete preset / group (recursive) | menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop Delete button; iPad context-menu Delete → `deletePreset(atPath:)`. |
| Apply preset to selection | menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop Apply + dbl-click; iPad inline Apply + context "Apply to Selection" → `applyPreset(atPath:)`. Both gated on selection. |
| Import presets from file | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `OnbtImportClick` (.xml); iPad `.fileImporter([.xml])` → `importPresets(fromPath:)`. |
| Export presets to file | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `OnbtExportClick` (.xpreset XML); iPad `.fileExporter` JSON → `exportPresets(toPath:)`. Format differs, data equivalent. |
| Persist preset library to disk | other | ✅ | ✅ | parity | P1 | easy | feasible | Both write `xlights_effectpresets.json`; iPad `savePresets` (+ .jbkp backup). |
| **Update preset from current effect** | menu | ✅ | ✅ | parity | P2 | medium | feasible | Desktop `OnbtUpdateClick`; iPad context-menu "Update from Selected Effect" → `updatePreset(atPath:)` (`SequencerViewModel.swift:3217`) → bridge `updatePresetAtPath:fromRow:effectIndex:` (`XLSequenceDocument.mm`) reusing the CopyFormat1 serializer + core `UpdatePresetSettings`. Gated on single-effect selection, confirm alert. |
| **Apply mode: Relative vs Using Layers** | preference | ✅ | ✅ | parity | P2 | medium | feasible | iPad `PresetBrowserSheet` segmented Auto / Relative / Using Layers picker (`PresetBrowserSheet.swift`); Auto defers to the preset's `\tLAYER:` token via bridge `presetUsesLayers(atPath:)`, matching desktop's `_layerMode` auto-detect. `applyPreset(atPath:…usingLayers:)` threads the flag to bridge `applyPresetAtPath:…usingLayers:`, which stacks effects onto the anchor element's successive effect layers (Using Layers) or spreads them across rows (Relative). |
| **Search presets in tree** | toolbar | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `TextCtrl1`+`ETButton1`/`SearchForText`; iPad `.searchable` on `PresetBrowserSheet` filters the in-memory `presetTree` (`PresetBrowserSheet.swift:visibleTree`), keeping ancestor groups of any match visible. Case-insensitive substring. |
| **Reorder presets within / across groups** | gesture | ✅ | ✅ | parity | P2 | easy | feasible | Desktop drag + Top/Up/Down/Bottom buttons; iPad context-menu Move Up / Move Down (within group, index move) + "Move to Group…" (cross-group) → `movePreset(fromPath:toGroupPath:)` / `movePreset(fromPath:toGroupPath:toIndex:)` (`SequencerViewModel.swift:3199/3209`). Bridge `movePresetItemFromPath:toGroupPath:toIndex:` translates the slot to a core `MoveItem(insertAfter)`. |
| **Animated preset preview GIF → still thumbnail** | panel | ✅ | 🟡 | ipad-weaker | P3 | hard | feasible | Desktop `GenerateGifImage`/`TimerGif` renders + *animates* a GIF of the selected preset. iPad now renders a **static representative still** (pragmatic take): bridge `presetThumbnailBGRA(atPath:outputSize:)` (`XLSequenceDocument.mm`) parses the preset's first CopyFormat1 effect, renders it on the shared preset matrix model via `iPadRenderContext::RenderEffectToFrames` (reusing the shader-preview scaffolding), and returns the last frame as BGRA. `PresetThumbnailCache` / `PresetThumbnailView` (`PresetBrowserSheet.swift`) lazy-load + cache it as the row's leading icon. Animation (multi-frame GIF playback) is the remaining gap. |
| Preset name-collision validation | dialog | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `NameCollissionInGroup` pre-check; iPad validates the entered name against the sibling set at the target path in the rename / new-group / save alerts (`PresetBrowserSheet.swift:siblingNames`), disabling the confirm action + showing inline feedback on collision. |
| **Presets: "From Base" section (base show folder)** | panel | ✅ | ✅ | parity | P2 | medium | feasible | iPad `iPadRenderContext::LoadBasePresets` loads the base-folder `xlights_effectpresets.json` into a second `_basePresetManager`; bridge `basePresetTree` / `applyBasePreset(atPath:…)` / `hasBasePresets` expose it apply-only; `PresetBrowserSheet` shows a bold **From Base** section above the user library. Read-only on iPad (desktop save-back via `PromptAndSaveBasePresets` is intentionally out of scope — see *Infeasible / restricted*). Reloads on base-folder change (`FolderConfigView.swift`). |
| **Jukebox: button grid (50 buttons)** | panel | ✅ | ❌ | ipad-missing | P3 | hard | declined | Desktop `media/JukeboxPanel.cpp`. Core `JukeboxButtonData` round-trips in the .xsq but **no iPad UI/bridge/VM at all**. |
| **Jukebox: link button to effect** | dialog | ✅ | ❌ | ipad-missing | P3 | hard | declined | Desktop `LinkJukeboxButtonDialog.cpp`. No iPad counterpart. |
| **Jukebox: play button (trigger effect)** | gesture | ✅ | ❌ | ipad-missing | P3 | hard | declined | Desktop click/`JUKEBOX_BTN_1..` keybindings fire `EVT_PLAYJUKEBOXITEM`. No iPad path. |
| **Jukebox: toggle panel visibility** | menu | ✅ | ❌ | ipad-missing | P3 | easy | declined | Desktop View>Windows>Jukebox (`ID_MNU_JUKEBOX`) + `JUKEBOX_TOGGLE` (F8). Blocked until iPad jukebox exists. |
| Views: create view | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `OnButton_AddViewClick`; iPad `addView(named:)` + alert. |
| Views: rename view | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `OnButtonRenameClick`; iPad `renameView(atIndex:to:)`. Master immobile both. |
| Views: clone view | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `OnButtonCloneClick`; iPad `cloneView(atIndex:as:)` (default "Copy Of X"). |
| Views: delete view | menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `OnButton_DeleteViewClick` (Master disabled); iPad `deleteView(atIndex:)` (idx 0 disabled). |
| Views: reorder (up/down) | gesture | ✅ | ✅ | parity | P2 | easy | feasible | Desktop View Up/Down buttons; iPad `moveViewUp/Down`. |
| Master View: toggle model visibility | gesture | ✅ | ✅ | parity | P1 | easy | feasible | Desktop visibility checkbox; iPad eye/eye.slash → master-visible flag. |
| Master View: add model | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `>`/`>>`/drag; iPad plus button → `addModel(toMasterView:)`. |
| Master View: remove model (warn) | menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `<` + warning; iPad minus + confirm → `removeElement(fromMasterView:)`. |
| Display Elements: add timing track | dialog | ✅ | ✅ | parity | P1 | medium | feasible | Desktop via context/SequenceElements; iPad explicit "Add Timing Track…" sheet (`AddTimingTrackSheet`). iPad more discoverable. |
| Display Elements: remove timing track | menu | ✅ | ✅ | parity | P1 | easy | feasible | Both warn before delete; iPad minus + "will delete timing track" alert. |
| Display Elements: filter available | toolbar | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `TextCtrl_NonModelsFilter`; iPad search TextField. Case-insensitive substring both. |
| User View: add model | menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `>`; iPad plus button. |
| User View: remove model | menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `<`; iPad minus (no warning — view membership only). |
| User View: add all / remove all | menu | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `>>`/`<<`; iPad Add All / Remove All. |
| User View: reorder members | gesture | ✅ | ✅ | parity | P2 | easy | feasible | Desktop Top/Up/Down/Bottom; iPad `.onMove` drag-reorder. |
| **Display Elements: Copy To Master** | menu | ✅ | ✅ | parity | P3 | medium | feasible | iPad bridge `copyViewToMaster(atIndex:)` mirrors `DoMakeMaster` (adds the view's models to Master, reorders to match, drops effect-free Master models absent from the view, returns the effects-kept names); "Copy To Master" button in the user-view detail (`DisplayElementsSheet.swift`) with a confirm + kept-models info alert. |
| **Display Elements: Import view config** | dialog | ✅ | 🟡 | parity | P3 | medium | feasible | iPad bridge `importViewConfig(fromSequencePath:)` reads another sequence's `<DisplayElements>` and creates an "Imported Master" view from the resolvable models + timings (mirrors `ImportSequenceMasterView`/`ImportViewData`); "Import View…" toolbar button + `.fileImporter([.xsq])` in `DisplayElementsSheet.swift`. The RGBEffects-layout-file import variant (`ImportRGBEffectsView`) is not yet on iPad. |
| **Display Elements: Show All / Hide All / Hide Unused models** | menu | ✅ | ✅ | parity | P2 | medium | feasible | iPad bridge `setAllElementsVisible(_:)` / `hideUnusedElements()` (toggle each element's master-visible flag, model/timing split like `setElementVisible:`); Master-View "Bulk Actions" toolbar menu in `DisplayElementsSheet.swift`. |
| **Display Elements: Select Used/Unused/All; Remove Unused** | menu | ✅ | 🟡 | parity | P2 | medium | feasible | iPad bridge `removeUnusedElements()` deletes every effect-free element (desktop Select Unused + Remove Unused) — "Remove Unused" in the Bulk Actions menu with a confirm. The pure list-selection ops (Select Used/Unused/All) have no standalone iPad analogue — they only seed Remove on desktop, which iPad folds directly into Remove Unused. |
| **Display Elements: Sort models (many strategies)** | menu | ✅ | ✅ | parity | P2 | hard | feasible | Desktop `ViewsModelsPanel.cpp:1667-1682`. iPad "Sort" toolbar menu in `DisplayElementsSheet.swift` (8 strategies: by name, groups-at-top variants, by start channel, by controller/port, by type, bubble-up groups). Client-side sort in Swift using `modelsListSummary` + `modelGroupsListSummary` bridge data, dispatched via `moveTopLevelElement(named:beforeNamed:)`. Node-count-by-group variant approximates with `modelCount` (pixel-count API absent in brief summaries). |
| **Edit Display Elements from import wizard** | dialog | ✅ | ✅ | parity | P3 | medium | feasible | Desktop #6477: `xLightsImportChannelMapDialog::EditDisplayElements` embeds `ViewsModelsPanel` mid-import then merges new models into the map tree. iPad: `ImportEffectsView` now has an "Edit Display Elements…" button in the options bar that presents the existing `DisplayElementsSheet`; on dismiss it calls `XLImportSession.rebuildDestinationTree` (rebuilds destination roots from the active sequence, preserving existing mappings by name) so newly-added models appear as mapping targets. |
| **Display Elements: panel-local Undo** | menu | ✅ | ❌ | ipad-missing | P2 | medium | feasible | Desktop `ViewsModelsPanel.cpp:1658` (`ID_MODELS_UNDO` + `_undo` stack) is a *panel-scoped* undo for view-membership ops. iPad `DisplayElementsSheet` view ops (`addView`/`deleteView`/`addModel(toMasterView:)`…) don't register undo with the global `undoManager`, so there's no undo for them yet. NOTE: this is **distinct from effect-panel slider undo**, which *is* at parity — every `setSettingValue` registers a discrete `undoManager` entry per key (`SequencerViewModel.swift:3246`), reachable via the global ⌘Z (`XLightsCommands.swift:77`). |
| **Display Elements: keyboard delete in lists** | gesture | ✅ | ✅ | parity | P3 | easy | feasible | Desktop `ViewsModelsPanel.cpp` `List_KEY_DOWN`. iPad: tap to select row (`focusedMemberID`), hardware Delete key fires `requestMasterRemove` via `.keyboardShortcut(.delete, modifiers:[])` on a background hidden Button in `DisplayElementsSheet.swift`. `onDeleteCommand` is iOS-unavailable; hardware-keyboard shortcut is the iPadOS equivalent. |
| **Display Elements: per-view checkbox (toolbar view visibility)** | other | ✅ | ✅ | parity | P3 | medium | feasible | Desktop `ViewsModelsPanel.cpp:205` (`wxCheckedListCtrl ListCtrlViews`). Semantics analysed: the checkbox is a radio-button view selector (one checked row = active view), not a separate visibility toggle. iPad sidebar tap-to-select in `DisplayElementsSheet.swift` is the direct equivalent — full parity by different idiom. No additional checkbox UI needed. |
| **Display Elements: effect-sequence mode (hide Views UI)** | other | ✅ | ✅ | parity | P3 | easy | feasible | Desktop `ViewsModelsPanel.cpp` `SetEffectSequenceMode` hides `ListCtrlViews`. iPad: `isEffectSequence` state set from `document.sequenceType() == "Effect"` in `.onAppear`; `DisplayElementsSheet.swift` shows only `masterViewDetail` (no NavigationSplitView sidebar) when effect-sequence. |
| **Row heading: has-effects yellow indicator stripe** | panel | ✅ | ✅ | parity | P2 | medium | feasible | Bridge `rowHasEffectsAtIndex:` / `rowHasEffects(at:)` added to `XLSequenceDocument.h:262` + `.mm`; `RowInfo.hasEffects` populated in `reloadRows()` for top-level model rows; `ModelRowHeader` draws a 3 pt amber (R0.918 G0.667 B0.0) trailing-edge rectangle overlay when `row.hasEffects`. Matches desktop `RowHeading.cpp:2706` `effectNoticePen/Brush` mustard-yellow stripe. |
| **Row heading: single-color model swatch** | panel | ✅ | ✅ | parity | P3 | medium | feasible | Bridge `rowNodeMaskColorAtIndex:` / `rowNodeMaskColor(at:)` added to `XLSequenceDocument.h` + `.mm` (returns `(std::string)GetNodeMaskColor(0)` hex for `"Single Color"`/`"Node Single Color"` string types with nodes, else empty); `RowInfo.nodeMaskColor` populated in `reloadRows()` for top-level model rows; `ModelRowHeader` draws a 12×12 black-outlined `RoundedRectangle` swatch at the trailing edge (offset 9 pt, inside the has-effects stripe) when non-empty. Matches desktop `RowHeading.cpp:2668-2688` `GetNodeMaskColor` swatch. |
| **Row heading: model tag-color chip** | panel | ✅ | ✅ | parity | P3 | medium | feasible | Bridge `rowTagColorAtIndex:` / `rowTagColor(at:)` added to `XLSequenceDocument.h:268` + `.mm` (returns `m->GetTagColourAsString()` hex, empty when black); `RowInfo.tagColor` populated in `reloadRows()` for top-level model rows; `ModelRowHeader` draws a 6×(height−6) `RoundedRectangle` chip in the model's tag color at leading+16 pt offset when non-empty. Matches desktop `RowHeading.cpp:2547-2553` tag-colour draw. Uses existing `Color(hex:)` extension from `PaletteIOSheets.swift`. |
| Effect dropper / icon palette | toolbar | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `EffectIconPanel` grid; iPad `EffectPaletteView` horizontal scroll. Both show icon+name; iPad tap-to-place vs desktop drag. |
| Palette: select effect | menu | ✅ | ✅ | parity | P1 | easy | feasible | iPad `selectPaletteEffect` toggles armed effect for tap-to-place. |
| Palette: random effect | toolbar | ✅ | ✅ | parity | P2 | easy | feasible | Desktop dice button; iPad dice → `availableEffects.randomElement()`. |
| Palette: cached icon images | panel | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `EffectIconCache` wxBitmap; iPad `EffectIconCache` CGImage (18px). |
| **Effect-panel slider/setting undo** | gesture | ✅ | ✅ | parity | P2 | easy | feasible | Desktop effect panels register a panel-level undo of slider/control tweaks. iPad: every `SequencerViewModel.setSettingValue` (`:3246`) registers a discrete `undoManager` undo per key with a descriptive action name ("Edit \<key\>"), reachable via the global ⌘Z (`XLightsCommands.swift:77`). Parity by idiom — finer-grained than a panel-local stack since it threads into the unified undo history. |
| Inspector notebook tabs | tab | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `wxNotebook`; iPad `NotebookTabsView` segmented (≤4) / menu (5+). Single-mode dispatch (e.g. SingleStrand). |
| Notebook: persist tab to settings | preference | ✅ | ✅ | parity | P1 | easy | feasible | Both store tab *label* in settings (e.g. `E_NOTEBOOK_SSEFFECT_TYPE`); both suppress first-tab default. |
| Notebook: ephemeral (no settingKey) tabs | preference | ✅ | ✅ | parity | P2 | easy | feasible | Buffer panel tabs; iPad uses `@State localSelection`. |
| View: toggle preview visibility | menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop View>Windows>Model/House Preview checks; iPad Show/Hide Preview (⌘1). |
| View: toggle inspector visibility | menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop AUI panes; iPad Show/Hide Inspector (⌘2). |
| View: detach preview to window | menu | ✅ | ✅ | parity | P2 | medium | feasible | Desktop AUI float; iPad Detach House/Model Preview WindowGroup scenes. |
| View: detach inspector tab to window | menu | ✅ | ✅ | parity | P2 | medium | feasible | Desktop AUI tab float; iPad Open Effect/Colors/Blending/Buffer in New Window (⌥⌘E/C/B/U). |
| View: dock all windows | menu | ✅ | ✅ | parity | P2 | easy | feasible | Desktop reset/dock; iPad "Dock All Windows". |
| View: reset pane sizes | menu | ✅ | ✅ | parity | P2 | easy | feasible | Desktop perspective reset; iPad "Reset Pane Sizes" clears @AppStorage keys. |
| View: edit display elements | menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop View>Windows>Display Elements; iPad "Edit Display Elements…" (⌘⇧D) → `DisplayElementsSheet`. |
| View: zoom in / out | menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop ⌘+/⌘−; iPad ⌘=/⌘− via `XLZoomCommands`. |
| View: zoom presets / fit | menu | ✅ | ✅ | parity | P2 | easy | feasible | Desktop 19-step ladder; iPad "Zoom To…" 8 geometric stops. |
| **AC Lights toolbar toggle** | menu | ✅ | ❌ | desktop-only | P3 | hard | feasible | Desktop View>AC Lights Toolbar (`MNU_ID_ACLIGHTS`) shows/hides the AC toolbar. iPad has no AC editing surface. AC cluster deferred 2026-06-11 (see theme 02). |
| **Show AC Ramps** | menu | ✅ | ❌ | desktop-only | P3 | hard | feasible | Desktop `ID_MNU_SHOWRAMPS` renders AC ramps. No iPad AC. |
| **AC editing toolbar (Off/On/Shimmer/Twinkle/Intensity/Ramp/Fill/Cascade)** | toolbar | ✅ | ❌ | desktop-only | P3 | hard | feasible | Desktop `ID_AUITOOLBAR_AC` mode buttons. No iPad AC sequencing surface. |
| **Perspectives: save current** | menu | ✅ | ❌ | desktop-only | P3 | hard | infeasible | Desktop `ID_MENUITEM_SAVE_PERSPECTIVE` saves AUI dock layout. iPad layout is fixed SwiftUI — no dockable panes. |
| **Perspectives: save as new** | menu | ✅ | ❌ | desktop-only | P3 | hard | infeasible | `ID_MENUITEM_SAVE_AS_PERSPECTIVE`. No AUI on iPad. |
| **Perspectives: edit / load** | menu | ✅ | ❌ | desktop-only | P3 | hard | infeasible | `ID_MENUITEM_LOAD_PERSPECTIVE`. No AUI on iPad. |
| **Perspectives: auto-save toggle** | menu | ✅ | ❌ | desktop-only | P3 | hard | infeasible | `ID_MNU_PERSPECTIVES_AUTOSAVE`. No AUI on iPad. |
| **Perspectives: dynamic saved list** | menu | ✅ | ❌ | desktop-only | P3 | hard | infeasible | `LoadPerspectivesMenu` auto-generates entries. No AUI on iPad. |
| **Perspectives panel (manage)** | panel | ✅ | ❌ | desktop-only | P3 | hard | infeasible | Desktop `PerspectivesPanel.cpp`. No iPad equivalent. |
| **Reset Toolbars** | menu | ✅ | ❌ | desktop-only | P3 | hard | feasible | Desktop View>Reset Toolbars (`ID_MENUITEM5`) restores AUI toolbar layout. iPad toolbars are fixed. |
| File toolbar (Open/New/Save/SaveAs/RenderAll) | toolbar | ✅ | 🟡 | parity | P2 | medium | feasible | Desktop `ID_AUITOOLBAR_*`. iPad uses native File menu + floating buttons — equivalent function, different idiom. |
| Playback toolbar (Play/Pause/Stop/First/Last/Replay) | toolbar | ✅ | ✅ | parity | P1 | medium | feasible | iPad has Play/Pause/Stop/Rewind + Playback menu **and now a Replay-Section** `repeat` toolbar button + Playback ▸ Replay Section (⇧Space) → `viewModel.replaySection()` (loops the selection's time bounds via the existing loop-region plumbing). |
| View toolbar (panel toggles) | toolbar | ✅ | 🟡 | parity | P2 | easy | feasible | Desktop `ID_AUITOOLBAR_VIEW`; iPad header buttons + View menu cover preview/inspector toggles. |
| View>Windows panel-toggle list (~16 docks) | menu | ✅ | 🟡 | desktop-only | P3 | hard | feasible | Desktop toggles each dock (Value Curves, Color Dropper, Effect Assist, Search Effects, Video Preview, Find Effect Data, …). iPad has no separate docks for most. |
| Suppress Dock (House/Model Preview) submenu | menu | ✅ | ❌ | desktop-only | P3 | hard | infeasible | Desktop `ID_MNU_SUPPRESSDOCK_HP/MP` for AUI re-docking control. Not applicable to SwiftUI scenes. |
| Find effects (timing-track labels) | menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `EffectsGrid::Find`/`FindNext` (timing tracks only); iPad `FindReplaceSheet` ⌘F → `runFind`/`findNext`. Both same restriction. |
| Replace effect labels | menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `EffectsGrid::Replace` (ID_GRID_MNU_REPLACE, timing tracks); iPad Replace Current / Replace All. |
| Row heading: resize column width | gesture | ✅ | ✅ | parity | P2 | easy | feasible | iPad `SequencerGridV2View.swift:227` `@AppStorage(gridRowHeaderWidth)=180` + `:1173-1183` `rowHeaderResizeHandle`. Shared `src-core`. |
| Row heading: model-group icon | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad `RowHeaderViews.swift:457-461` `Image(systemName: "folder")` when group; `:404-405` `isGroup`. |
| Row heading: tooltip for long names | panel | ✅ | ✅ | parity | P3 | easy | feasible | iPad `RowHeaderViews.swift:468/474/481/488` `.help(row.displayName)` on model rows + `:136` on timing rows. |
| Timing-track active toggle (radio dot) | gesture | ✅ | ✅ | parity | P1 | easy | feasible | iPad `RowHeaderViews.swift:100-111` active Circle (filled/outlined) + `onTapGesture document.setTimingRowActive`. |

## iPad gaps (desktop has, iPad missing)

### P2

- **Update Preset from current effect** — *landed.* Bridge
  `updatePresetAtPath:fromRow:effectIndex:` (`XLSequenceDocument.mm`)
  rebuilds the CopyFormat1 blob from the selected effect and calls core
  `EffectPresetManager::UpdatePresetSettings`; VM `updatePreset(atPath:)`
  (`SequencerViewModel.swift:3217`); `PresetBrowserSheet` context-menu
  "Update from Selected Effect" gated on a single-effect selection with
  a confirm alert (it overwrites the preset). Parity with desktop
  `OnbtUpdateClick`.
- **Relative vs Using Layers apply mode** — *landed.*
  `PresetBrowserSheet` adds a segmented Auto / Relative / Using Layers
  picker; Auto reads the preset's `\tLAYER:` token via bridge
  `presetUsesLayers(atPath:)` (desktop's `_layerMode` auto-detect).
  `applyPreset(atPath:…usingLayers:)` / `applyBasePreset(…)` thread the
  flag to bridge `applyPresetAtPath:…usingLayers:`, which in Using
  Layers mode stacks the preset's effects onto the anchor element's
  successive effect layers (growing the element as needed) and in
  Relative mode spreads them across rows.
- **Search presets** — *landed.* `PresetBrowserSheet` adds a
  `.searchable` box; `visibleTree` filters the in-memory
  `viewModel.presetTree` case-insensitively by name substring and keeps
  the ancestor groups of any match visible. Parity with desktop
  `SearchForText`.
- **Reorder presets** — *landed.* `PresetBrowserSheet` context menu
  exposes Move Up / Move Down (within-group index reorder) and "Move to
  Group…" (cross-group). Within-group moves go through the new
  `movePreset(fromPath:toGroupPath:toIndex:)` VM + bridge
  `movePresetItemFromPath:toGroupPath:toIndex:`, which converts the drop
  slot to a core `EffectPresetManager::MoveItem(insertAfter)` (the core
  already keeps ordered children). Cross-group uses the existing
  two-arg `movePreset`.
- **Preset name-collision pre-check** — *landed.* The rename,
  new-group, and save-selection alerts in `PresetBrowserSheet` validate
  the entered name against the sibling set at the target path
  (`siblingNames`), disabling the confirm button and showing inline
  feedback on collision. Parity with desktop `NameCollissionInGroup`.
- **Display Elements bulk model ops** — *landed.* Bridge
  `setAllElementsVisible(_:)` (Show/Hide All), `hideUnusedElements()`
  (Hide Unused), and `removeUnusedElements()` (Select Unused + Remove
  Unused) over every sequence element, with the model/timing
  master-visible split from `setElementVisible:`. Master-View "Bulk
  Actions" toolbar menu in `DisplayElementsSheet.swift` (Remove Unused
  behind a confirm). The pure list-selection ops (Select Used/Unused/
  All) have no standalone iPad surface — they only feed Remove on
  desktop, which iPad folds into Remove Unused. **From Base presets** —
  *landed.* See the scorecard row + intro.
- **Display Elements sort** — *landed.* "Sort" toolbar menu in
  `DisplayElementsSheet.swift` adds 8 strategies: By Name, By Name
  Groups At Top (alphabetical / by member count / by member count for
  "by node count" approximation), By Start Channel (+ by size), By
  Controller/Port (+ by size), By Type, and Bubble Up Groups. Client-
  side sort in Swift — `modelsListSummary` / `modelGroupsListSummary`
  supply the metadata; `moveTopLevelElement(named:beforeNamed:)`
  reorders master-view elements. Node-count sort approximates with
  `modelCount` (pixel-count API not in brief summaries).
- **Display Elements panel-local Undo** — desktop keeps a panel undo
  stack (`ViewsModelsPanel.cpp:1658`, `ID_MODELS_UNDO`); the iPad sheet
  has no undo stack. *Work:* mirror the stack in the sheet or route
  through the shared undo manager. *Ease: medium.*
- **Row-heading has-effects stripe** — *landed.* Bridge
  `rowHasEffectsAtIndex:` / `rowHasEffects(at:)` added
  (`XLSequenceDocument.h:262`, `.mm`); `RowInfo.hasEffects` field
  populated in `reloadRows()` for top-level model rows only; `ModelRowHeader`
  draws a 3 pt amber trailing-edge `Rectangle` overlay when
  `row.hasEffects`. Matches desktop `RowHeading.cpp:2706` mustard-yellow
  stripe.

### P3

- **Animated preset preview GIF → still thumbnail** — *landed (still
  variant).* Bridge `presetThumbnailBGRA(atPath:outputSize:)`
  (`XLSequenceDocument.mm`) parses the preset's first CopyFormat1 effect,
  loads it onto the shared preset matrix model, renders via
  `iPadRenderContext::RenderEffectToFrames` (the same scaffolding the
  shader-preview path uses, guarded by a mutex), and returns the last
  frame as BGRA. `PresetThumbnailCache` + `PresetThumbnailView`
  (`PresetBrowserSheet.swift`) lazy-load it off the main thread and cache
  it as the preset row's leading icon, falling back to `wand.and.stars`
  while loading / on failure. The remaining gap is *animating* the
  preview (multi-frame GIF playback) — the still covers the
  "what does this preset look like" need at a fraction of the cost.
- **Jukebox (whole subsystem)** — **DECLINED (won't-do, 2026-06-11).**
  `media/JukeboxPanel.cpp` + `LinkJukeboxButtonDialog.cpp`; core
  `JukeboxButtonData` already round-trips through the .xsq, so data
  survives an iPad edit — that round-trip guarantee is the only
  obligation going forward. No iPad UI will be built.
- **Display Elements: Copy To Master** — *landed.* Bridge
  `copyViewToMaster(atIndex:)` mirrors `DoMakeMaster` (add the view's
  models to Master, reorder to match, drop effect-free Master models
  absent from the view, return the effects-kept names) + "Copy To
  Master" button in the user-view detail with a confirm and kept-models
  info alert.
- **Display Elements: Import view config** — *landed (sequence
  variant).* Bridge `importViewConfig(fromSequencePath:)` reads another
  `.xsq`'s `<DisplayElements>` and builds an "Imported Master" view
  from the resolvable models + timings (mirrors
  `ImportSequenceMasterView`/`ImportViewData`); "Import View…" toolbar
  button + `.fileImporter([.xsq])`. The RGBEffects-layout-file import
  variant (`ImportRGBEffectsView`) remains desktop-only.
- **Display Elements keyboard delete** — *landed.* Tap to select a
  member row (`focusedMemberID`), then hardware Delete key fires
  `requestMasterRemove` via `.keyboardShortcut(.delete, modifiers:[])` on
  a hidden background Button in `DisplayElementsSheet.swift`.
  (`onDeleteCommand` is iOS-unavailable; hardware-keyboard shortcut is
  the iPadOS equivalent.)
- **Display Elements per-view checkbox** — *at parity by idiom.* The
  desktop checkbox (`wxCheckedListCtrl ListCtrlViews`) is a radio-button
  view selector (checked row = active view). iPad's sidebar
  tap-to-select in `DisplayElementsSheet.swift` is the direct
  equivalent. No extra checkbox UI needed.
- **Display Elements effect-sequence mode** — *landed.* `isEffectSequence`
  state set from `document.sequenceType() == "Effect"` in `.onAppear`;
  sheet shows only `masterViewDetail` (bypassing the NavigationSplitView
  sidebar) when effect-sequence, matching desktop `SetEffectSequenceMode`
  hiding `ListCtrlViews`.
- **Row-heading single-color model swatch** — *landed.* Bridge
  `rowNodeMaskColorAtIndex:` / `rowNodeMaskColor(at:)`
  (`XLSequenceDocument.h` + `.mm`) returns `(std::string)GetNodeMaskColor(0)`
  hex for `"Single Color"`/`"Node Single Color"` string types that have
  nodes, empty otherwise; `RowInfo.nodeMaskColor` populated in
  `reloadRows()` for top-level model rows; `ModelRowHeader` draws a 12×12
  black-outlined `RoundedRectangle` swatch at the trailing edge (9 pt in,
  beside the has-effects stripe). Matches desktop `RowHeading.cpp:2668-2688`.
- **Row-heading model tag-color chip** — *landed.* Bridge
  `rowTagColorAtIndex:` / `rowTagColor(at:)` added
  (`XLSequenceDocument.h:268`, `.mm`) returning `m->GetTagColourAsString()`
  hex or empty when black; `RowInfo.tagColor` populated in `reloadRows()`;
  `ModelRowHeader` draws a 6×(height−6) `RoundedRectangle` chip at
  leading+16 pt when non-empty, using existing `Color(hex:)` from
  `PaletteIOSheets.swift`.

## Desktop gaps (iPad has, desktop missing)

None of substance for this theme. iPad's preset/view/inspector surface
is a strict subset or idiom-translation of desktop. The iPad-only
*idioms* (multi-window scene detach for previews/inspector tabs,
tap-to-place palette arming, drag-reorder via `.onMove`) are touch /
multi-window analogues of desktop's AUI float-dock and drag-drop, not
net-new functionality desktop lacks. (The detach-to-`WindowGroup`
scene model is arguably richer than AUI float on iPad hardware, but
the user-facing capability — separate preview/inspector windows —
exists on both.)

## Infeasible / restricted on iPad

- **AUI perspectives** (save / save-as / edit-load / auto-save /
  dynamic list / `PerspectivesPanel` / Suppress-Dock submenu) —
  architecturally **infeasible**: the iPad UI is a fixed SwiftUI
  layout with no dockable/floatable panes to serialize. iPad's
  equivalent need (detached previews/inspector + reset pane sizes) is
  already met by the WindowGroup scene system.
- **Reset Toolbars / Show-Hide individual toolbars** — iPad toolbars
  are fixed SwiftUI chrome, not user-rearrangeable AUI toolbars, so
  there's no layout to reset (effectively infeasible / N/A).
- **AC editing (AC Lights toolbar, Show AC Ramps, AC mode toolbar)** —
  desktop-only sequencing surface; iPad has no AC editing mode, so the
  toggles have nothing to act on. **AC is deferred (P3) by decision
  2026-06-11** — see theme 02's Deferred section.
- **Jukebox — declined (won't-do, 2026-06-11).** Not a platform limit;
  a deliberate scope decision. Core `JukeboxButtonData` must keep
  round-tripping through `.xsq` load/save on iPad (it does today).
- **"From Base" preset save-back** — the iPad "From Base" section is
  read-only / apply-only by design. Desktop's `PromptAndSaveBasePresets`
  writes edits back into the *base show folder's*
  `xlights_effectpresets.json`; on iOS that folder is a separate
  security-scoped location and mutating a shared base library from a
  per-sequence client is out of scope (users curate the base library on
  desktop). Browsing + applying base presets is at parity.
- **Display Elements: RGBEffects-layout view import** — desktop
  `ImportRGBEffectsView` imports a view from an `xlights_rgbeffects.xml`
  layout file. iPad currently imports view config only from another
  `.xsq` sequence's `<DisplayElements>`; the rgbeffects-layout variant
  is deferred (low value, redundant with the sequence path for most
  users).
- No controller-firmware items fall in this theme (jukebox/views/
  presets/perspectives are sequence-local), so the closed-firmware
  IAP restriction does not apply here.

## Recommended sequencing

1. ✅ **Done — reorder presets** (P2) — context-menu Move Up / Move
   Down (within-group index move via `movePreset(…toIndex:)`) + "Move to
   Group…" in `PresetBrowserSheet`.
2. ✅ **Done — preset search filter** (P2) — `.searchable` over the
   in-memory `presetTree` (`visibleTree`), ancestor groups stay visible.
3. ✅ **Done — Update Preset from selection** (P2) — bridge
   `updatePresetAtPath:fromRow:effectIndex:` + VM `updatePreset(atPath:)`
   + context-menu item with confirm; reuses the CopyFormat1 serializer.
4. ✅ **Done — Relative/Using-Layers apply mode + From Base presets +
   Display Elements bulk ops** (P2) — `applyPreset(…usingLayers:)` with
   an Auto/Relative/Using-Layers picker (`\tLAYER:` auto-detect); a
   read-only From Base section over a second `EffectPresetManager`; and
   Show/Hide All + Hide/Remove Unused bulk model ops in the Master-View
   Bulk Actions menu. (Collision pre-check ✅ done alongside items 1-3.)
5. ✅ **Done — Playback Replay-Section** (P1) — `replaySection`
   op + `repeat` toolbar button + Playback ▸ Replay Section (⇧Space),
   reusing the loop-region plumbing.
6. ✅ **Done — Display Elements Copy-To-Master + Import view config**
   (P3) — `copyViewToMaster(atIndex:)` + a user-view button, and
   `importViewConfig(fromSequencePath:)` + an Import View toolbar button
   (`.xsq` sequence variant; rgbeffects-layout import stays
   desktop-only). Remaining `ViewsModelsPanel` gaps (sort strategies,
   panel-local undo, per-view checkbox, keyboard delete) stay tracked
   above.
7. ✅ **Done — Display Elements sort + keyboard delete + effect-sequence
   mode + per-view checkbox (parity-by-idiom) + row-heading has-effects
   stripe + row-heading tag-color chip** (P2/P3) — Sort toolbar menu in
   `DisplayElementsSheet.swift` (8 strategies via `modelsListSummary` +
   `moveTopLevelElement`); hardware Delete key via hidden Button with
   `.keyboardShortcut`; `isEffectSequence` hides the Views sidebar;
   per-view checkbox confirmed at parity by idiom; `rowHasEffects(at:)` +
   `rowTagColor(at:)` bridge methods + `RowInfo` fields + overlay draws in
   `ModelRowHeader`. Single-color swatch still deferred.
8. ✅ **Done — preset still-thumbnail + row-heading single-color swatch**
   (P3). Preset preview ships a static representative still
   (`presetThumbnailBGRA` → `RenderEffectToFrames` → cached row icon);
   *animating* it (GIF playback) is the only remaining preset gap.
   Single-color swatch ships via `rowNodeMaskColor(at:)` +
   `ModelRowHeader` trailing swatch. **Jukebox is declined (won't-do)**;
   Perspectives and AC stay desktop-only (AC deferred per theme 02).
