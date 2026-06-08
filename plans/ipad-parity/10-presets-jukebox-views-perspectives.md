# 10 · Presets, Jukebox, Display Elements, Views & Perspectives

> The **effect-preset library** is at strong parity:
> iPad's `PresetBrowserSheet` + the `XLSequenceDocument (EffectPresets)`
> bridge implement apply / save / group / rename / delete / import /
> export against the same `EffectPresetManager` core the desktop
> `EffectTreeDialog` uses. The real preset gaps are the *secondary*
> EffectTreeDialog affordances: **Update Preset** (no bridge method
> exists), the **Relative vs Using Layers** paste radio, **search**,
> **reorder buttons** (the `movePreset` bridge+VM method exists but is
> wired to *no UI*), and the animated **preview GIF**. **Display
> Elements / Views** is also near-parity (create / rename / clone /
> delete / reorder views, add-to / remove-from Master + user views,
> visibility toggles, timing-track add/remove, filter), missing only
> **Copy To Master** and **Import view config from another sequence**.
> **Jukebox is 100% desktop-only** — no iPad UI, bridge, or VM (core
> `JukeboxButtonData` exists and is loaded/saved with the sequence, so
> the data round-trips, but nothing on iPad surfaces it). **AUI
> perspectives are architecturally infeasible** on the fixed SwiftUI
> layout; iPad instead has its own detach-to-window scene system that
> desktop covers via AUI float/dock. The desktop **AUI toolbar set**
> (File / Playback / AC / View / Edit toolbars, Reset Toolbars) maps to
> iPad's native menu bar + floating toolbar buttons — mostly parity by
> different idiom, with the **AC toolbar** being the only real
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
| **Update preset from current effect** | menu | ✅ | ❌ | ipad-missing | P2 | medium | feasible | Desktop `OnbtUpdateClick` overwrites preset blob with current settings. **No iPad bridge method** (`updatePreset` absent in `XLSequenceDocument.h`). New bridge + VM + button needed. |
| **Apply mode: Relative vs Using Layers** | preference | ✅ | ❌ | ipad-missing | P2 | medium | feasible | Desktop `btPosition`/`btLayers` radios + auto-detect from `\tLAYER:` token; passes `_layerMode` to `ApplyEffectsPreset`. iPad `applyPreset` has no layer-mode arg surfaced. |
| **Search presets in tree** | toolbar | ✅ | ❌ | ipad-missing | P2 | easy | feasible | Desktop `TextCtrl1`+`ETButton1`/`SearchForText`. `PresetBrowserSheet` has no `.searchable`. Trivial client-side filter on `presetTree`. |
| **Reorder presets within / across groups** | gesture | ✅ | ❌ | ipad-missing | P2 | easy | feasible | Desktop drag + Top/Up/Down/Bottom buttons. **iPad bridge+VM `movePreset(fromPath:toGroupPath:)` already exists** (`SequencerViewModel.swift:3181`) but **no UI calls it** — only needs a `.onMove`/menu wire-up. |
| **Animated preset preview GIF** | panel | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop `GenerateGifImage`/`TimerGif` renders + animates a GIF of the selected preset. iPad would need Metal-render→thumbnail pipeline. Polish. |
| Preset name-collision validation | dialog | ✅ | 🟡 | ipad-missing | P2 | easy | feasible | Desktop `NameCollissionInGroup` pre-check; core `EffectPresetManager` de-dups silently on add. iPad has no client-side pre-check (relies on core fix-up). |
| **Presets: "From Base" section (base show folder)** | panel | ✅ | ❌ | ipad-missing | P2 | medium | feasible | Desktop #6450: `EffectTreeDialog` loads the base-folder `xlights_effectpresets.json` into `_basePresetManager`, shows a bold "From Base" root (`IsInBaseSection`), and prompts save-back via `PromptAndSaveBasePresets`. iPad has base-folder infra (`FolderConfigView.swift:245`, `baseShowDirectory()`) + `PresetBrowserSheet` but no base-presets bridge/section. Needs a `basePresets`-style bridge method + a sectioned list. |
| **Jukebox: button grid (50 buttons)** | panel | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop `media/JukeboxPanel.cpp`. Core `JukeboxButtonData` round-trips in the .xsq but **no iPad UI/bridge/VM at all**. |
| **Jukebox: link button to effect** | dialog | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop `LinkJukeboxButtonDialog.cpp`. No iPad counterpart. |
| **Jukebox: play button (trigger effect)** | gesture | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop click/`JUKEBOX_BTN_1..` keybindings fire `EVT_PLAYJUKEBOXITEM`. No iPad path. |
| **Jukebox: toggle panel visibility** | menu | ✅ | ❌ | ipad-missing | P3 | easy | feasible | Desktop View>Windows>Jukebox (`ID_MNU_JUKEBOX`) + `JUKEBOX_TOGGLE` (F8). Blocked until iPad jukebox exists. |
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
| **Display Elements: Copy To Master** | menu | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop `Button_MakeMaster`/`DoMakeMaster` copies a user view's element list+order into Master. No iPad bridge/UI. |
| **Display Elements: Import view config** | dialog | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop `OnButtonImportClick` (import RGBEffects / from another sequence). No iPad bridge/UI. |
| **Display Elements: Show All / Hide All / Hide Unused models** | menu | ✅ | ❌ | ipad-missing | P2 | medium | feasible | Desktop `ViewsModelsPanel.cpp:1659-1661`. No counterpart in `DisplayElementsSheet.swift`. |
| **Display Elements: Select Used/Unused/All; Remove Unused** | menu | ✅ | ❌ | ipad-missing | P2 | medium | feasible | Desktop `ViewsModelsPanel.cpp:1662-1665`. No counterpart in `DisplayElementsSheet.swift`. |
| **Display Elements: Sort models (many strategies)** | menu | ✅ | ❌ | ipad-missing | P2 | hard | feasible | Desktop `ViewsModelsPanel.cpp:1667-1682`. No sort symbol in `DisplayElementsSheet.swift` or `XLSequenceDocument.h`. |
| **Edit Display Elements from import wizard** | dialog | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop #6477: `xLightsImportChannelMapDialog::EditDisplayElements` embeds `ViewsModelsPanel` mid-import, lets you add master-view models without leaving the wizard, then merges new models into the map tree. iPad has `DisplayElementsSheet` + `ImportEffectsView` as separate flows; no cross-link from import to Display Elements. |
| **Display Elements: panel-local Undo** | menu | ✅ | ❌ | ipad-missing | P2 | medium | feasible | Desktop `ViewsModelsPanel.cpp:1658` (`ID_MODELS_UNDO` + `_undo` stack). No undo stack in `DisplayElementsSheet.swift`. |
| **Display Elements: keyboard delete in lists** | gesture | ✅ | ❌ | ipad-missing | P3 | easy | feasible | Desktop `ViewsModelsPanel.cpp` `List_KEY_DOWN`. No `onDeleteCommand` in `DisplayElementsSheet.swift`. |
| **Display Elements: per-view checkbox (toolbar view visibility)** | other | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop `ViewsModelsPanel.cpp:205` (`wxCheckedListCtrl ListCtrlViews`). No per-view checkbox in `DisplayElementsSheet.swift`. |
| **Display Elements: effect-sequence mode (hide Views UI)** | other | ✅ | ❌ | ipad-missing | P3 | easy | feasible | Desktop `ViewsModelsPanel.cpp` `SetEffectSequenceMode`. No effect-only handling in `DisplayElementsSheet.swift`. |
| **Row heading: has-effects yellow indicator stripe** | panel | ✅ | ❌ | ipad-missing | P2 | medium | feasible | No has-effects heading stripe in `RowHeaderViews.swift` (only `document.elementHasEffects` bridge `XLSequenceDocument.h:749`); `EffectsMetalGridView.swift:73`. |
| **Row heading: single-color model swatch** | panel | ✅ | ❌ | ipad-missing | P3 | medium | feasible | No mask/single-color swatch symbol in `src-iPad`; desktop `RowHeading.cpp:2502-2520` `GetNodeMaskColor` + `DrawRectangle`. |
| **Row heading: model tag-color chip** | panel | ✅ | ❌ | ipad-missing | P3 | medium | feasible | No tag-color heading draw in `RowHeaderViews.swift`; `XLSequenceDocument.h:1314` `tagColor` is a group-layout property setter only; desktop draws it in `RowHeading`. |
| Effect dropper / icon palette | toolbar | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `EffectIconPanel` grid; iPad `EffectPaletteView` horizontal scroll. Both show icon+name; iPad tap-to-place vs desktop drag. |
| Palette: select effect | menu | ✅ | ✅ | parity | P1 | easy | feasible | iPad `selectPaletteEffect` toggles armed effect for tap-to-place. |
| Palette: random effect | toolbar | ✅ | ✅ | parity | P2 | easy | feasible | Desktop dice button; iPad dice → `availableEffects.randomElement()`. |
| Palette: cached icon images | panel | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `EffectIconCache` wxBitmap; iPad `EffectIconCache` CGImage (18px). |
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
| **AC Lights toolbar toggle** | menu | ✅ | ❌ | desktop-only | P2 | hard | feasible | Desktop View>AC Lights Toolbar (`MNU_ID_ACLIGHTS`) shows/hides the AC toolbar. iPad has no AC editing surface. |
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
| Playback toolbar (Play/Pause/Stop/First/Last/Replay) | toolbar | ✅ | 🟡 | both-missing | P1 | medium | feasible | iPad has Play/Pause/Stop/Rewind + Playback menu but **no Replay-Section** button (`ID_AUITOOLBAR_REPLAY_SECTION`). |
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

### P1

- **Playback "Replay Section"** — desktop `ID_AUITOOLBAR_REPLAY_SECTION`
  loops the currently-selected time range. iPad's Playback menu has
  Play / Stop / Render / seek / frame-step / speed but no replay-range
  loop. *Work:* add a `replaySection` op in `SequencerViewModel`
  (loop between selection start/end) + a Playback-menu button + toolbar
  button in `SequencerView`. Bridge already exposes range/selection
  state. *Ease: medium.*

### P2

- **Update Preset from current effect** — desktop `OnbtUpdateClick`.
  This is the only preset operation with **no bridge method at all**
  (`XLSequenceDocument.h` has no `updatePreset`). *Work:* add
  `-updatePresetAtPath:fromRows:effectIndices:` to the bridge
  (reuse the `savePreset(fromRows:…)` serializer to rebuild the blob),
  a VM wrapper, and an "Update from Selection" context-menu item in
  `PresetBrowserSheet`. *Ease: medium.*
- **Relative vs Using Layers apply mode** — desktop `btPosition` /
  `btLayers` radios; desktop also auto-detects layer mode from the
  preset's `\tLAYER:` token and passes `_layerMode` to
  `ApplyEffectsPreset`. iPad's `applyPreset(atPath:)` doesn't surface
  the choice. *Work:* thread a `usingLayers:` arg through
  `applyPreset` (bridge already calls the core paste) and add a toggle
  in the sheet (or auto-detect silently for parity). *Ease: medium.*
- **Search presets** — desktop `TextCtrl1` + `SearchForText`.
  `PresetBrowserSheet` lacks `.searchable`. *Work:* client-side filter
  on the already-loaded `viewModel.presetTree` (path/name substring).
  *Ease: easy.*
- **Reorder presets** — the **bridge + VM already exist**
  (`document.movePreset(fromPath:toGroupPath:)` →
  `SequencerViewModel.movePreset` at `SequencerViewModel.swift:3181`)
  but **no SwiftUI surface invokes them**. *Work:* wire a `.onMove`
  on the preset List rows (or Up/Down context-menu items) into
  `movePreset`. *Ease: easy.* (Lowest-effort highest-value preset gap.)
- **Preset name-collision pre-check** — desktop
  `NameCollissionInGroup`; iPad relies on the core manager's silent
  de-dup. *Work:* validate the name against the sibling set in the
  alert before committing. *Ease: easy.*
- **Display Elements bulk model ops** — desktop
  `ViewsModelsPanel.cpp:1659-1665`: Show All / Hide All / Hide Unused,
  Select Used/Unused/All, Remove Unused. None present in
  `DisplayElementsSheet.swift`. *Work:* bridge wrappers + a toolbar
  menu in the sheet. *Ease: medium.*
- **Display Elements sort** — desktop offers many sort strategies
  (`ViewsModelsPanel.cpp:1667-1682`); no sort symbol exists in
  `DisplayElementsSheet.swift` or `XLSequenceDocument.h`. *Work:*
  bridge sort op + a sort picker. *Ease: hard.*
- **Display Elements panel-local Undo** — desktop keeps a panel undo
  stack (`ViewsModelsPanel.cpp:1658`, `ID_MODELS_UNDO`); the iPad sheet
  has no undo stack. *Work:* mirror the stack in the sheet or route
  through the shared undo manager. *Ease: medium.*
- **Row-heading has-effects stripe** — desktop draws a yellow
  has-effects indicator on row headings; `RowHeaderViews.swift` draws
  none (the `document.elementHasEffects` bridge exists,
  `XLSequenceDocument.h:749`; grid at `EffectsMetalGridView.swift:73`).
  *Work:* read the flag and draw the stripe in the row header.
  *Ease: medium.*

### P3

- **Animated preset preview GIF** — desktop `GenerateGifImage` /
  `TimerGif`. iPad would need a Metal-render→thumbnail pipeline
  (see MEMORY: `project_media_preview_generation`). *Ease: hard.*
- **Jukebox (whole subsystem)** — `media/JukeboxPanel.cpp` +
  `LinkJukeboxButtonDialog.cpp`; core `JukeboxButtonData` already
  round-trips through the .xsq, so data survives an iPad edit. *Work:*
  a `JukeboxSheet` SwiftUI grid + bridge wrappers over the existing
  `JukeboxButtonMap` (get/set/play). *Ease: hard*, low value on touch.
- **Display Elements: Copy To Master** — desktop `DoMakeMaster`
  copies a user view's ordered element list into Master. *Work:* bridge
  `copyViewToMaster:` + a button in `DisplayElementsSheet` user-view
  detail. *Ease: medium.*
- **Display Elements: Import view config** — desktop
  `OnButtonImportClick` imports views/RGBEffects from another sequence.
  *Work:* bridge + `.fileImporter`. *Ease: medium.*
- **Display Elements keyboard delete** — desktop deletes selected list
  rows via `List_KEY_DOWN` (`ViewsModelsPanel.cpp`); the iPad sheet has
  no `onDeleteCommand`. *Work:* add a delete key handler to the lists.
  *Ease: easy.*
- **Display Elements per-view checkbox** — desktop toolbar-view
  visibility uses `wxCheckedListCtrl ListCtrlViews`
  (`ViewsModelsPanel.cpp:205`); the iPad sheet has no per-view
  checkbox. *Work:* add a checkbox column to the views list.
  *Ease: medium.*
- **Display Elements effect-sequence mode** — desktop hides the Views
  UI in effect-sequence mode (`ViewsModelsPanel.cpp`
  `SetEffectSequenceMode`); the iPad sheet has no effect-only handling.
  *Work:* conditionally hide the Views section. *Ease: easy.*
- **Row-heading single-color model swatch** — desktop draws a swatch
  for single-color models (`RowHeading.cpp:2502-2520`,
  `GetNodeMaskColor` + `DrawRectangle`); no equivalent symbol exists in
  `src-iPad`. *Work:* read the mask color and draw the swatch in the
  row header. *Ease: medium.*
- **Row-heading model tag-color chip** — desktop draws the model
  tag-color chip in the row heading; `RowHeaderViews.swift` draws none
  (the `tagColor` setter at `XLSequenceDocument.h:1314` is a
  group-layout property only). *Work:* draw the chip from the model's
  tag color. *Ease: medium.*

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
  toggles have nothing to act on. Feasible to build but out of MVP
  scope for now.
- No controller-firmware items fall in this theme (jukebox/views/
  presets/perspectives are sequence-local), so the closed-firmware
  IAP restriction does not apply here.

## Recommended sequencing

1. **Wire `movePreset` into the UI** (P2, easy) — the bridge + VM
   already exist; a `.onMove` on `PresetBrowserSheet` rows is the
   cheapest meaningful preset win.
2. **Preset search filter** (P2, easy) — client-side filter over the
   in-memory `presetTree`; no bridge work.
3. **Update Preset from selection** (P2, medium) — add the missing
   bridge method + context-menu item; reuse the existing save serializer.
4. **Relative/Using-Layers apply mode + collision pre-check** (P2) —
   small bridge-arg + alert validation; closes the EffectTreeDialog
   feature delta.
5. **Playback Replay-Section** (P1, medium) — only P1 gap in the theme;
   add a loop-range op + menu/toolbar entry.
6. **Display Elements Copy-To-Master + Import view config** (P3) —
   round out `ViewsModelsPanel` parity once the higher-value items land.
7. **Jukebox** and **animated preset GIF preview** (P3, hard) — defer;
   low touch-value / heavy pipeline work. Perspectives and AC stay
   desktop-only.
