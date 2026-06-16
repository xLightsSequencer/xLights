# 11 · Preferences, Settings & Keyboard Shortcuts

> Desktop has a single centralized `wxPreferencesEditor`
> (`OnMenuItemPreferencesSelected`, File ▸ "Preferences…" / **Cmd+,**) with 10
> pages — Backup, View, Effects Grid, Sequences, Output, Check Sequence,
> Random Effects, Colors, Other, and Services (when `ENABLE_SERVICES`) — totaling
> ~80 individual settings. The iPad app deliberately has **no single Preferences
> screen** (a centralized dialog was **declined 2026-06-11 — won't build**;
> context sheets are the intended iPad model, and only the per-setting gaps
> remain backlog); it scatters the equivalents into context sheets: `FolderConfigView`
> (folders + app-wide render/cache/FSEQ — the de-facto "settings" entry, reached
> via the folder.badge.gearshape button), `SequenceSettingsSheet` (metadata,
> audio, render-cache/auto-save, timings), `AIServicesSettingsSheet`, and
> `CheckSequenceSheet` (results only — no disable-checks toggles). On keyboard
> shortcuts, desktop ships a **full rebinding editor**
> (`KeyBindingEditDialog`, File ▸ "Key bindings", wired at
> `xLightsMain.cpp:8653`) *and* a read-only dump (Help ▸ "Key Bindings",
> `xLightsMain.cpp:7966`) over 137 bindings in 4 scopes; the iPad has ~50
> hard-keyboard shortcuts declared in `XLightsCommands.swift` and **no rebinding
> UI**. Both platforms now ship the **command palette** (#6258, ⌘⇧K) — a fuzzy
> launcher over every menu command and effect type (`CommandPaletteSheet` /
> `CommandPaletteRegistry` on iPad) — which doubles as the discoverability
> surface that the read-only key-binding dump provides on desktop. Both platforms have Color Replace (desktop `ColourReplaceDialog`, iPad
> `ColorReplaceSheet`), and the iPad has user-settable auto-save
> (`autosaveIntervalMinutes`). The largest gap is that iPad
> exposes no equivalent for whole categories of desktop prefs (backup strategy,
> output-protocol tuning, video-export codec/bitrate, tip-of-day, random-effect
> weights, many grid-display toggles), and **neither platform lets iPad users
> rebind keys**.

## Parity scorecard

| Feature | Surface | Desktop | iPad | Gap | Priority | Ease | Feasibility | Notes |
|---|---|---|---|---|---|---|---|---|
| Centralized Preferences dialog | menu | ✅ | ❌ | desktop-missing* | P3 | hard | declined | Desktop `xLightsPreferences.cpp:86`, Cmd+,. **Declined 2026-06-11 — won't build**: context sheets are the intended iPad settings model. The *individual settings* gaps below remain backlog; only the single-entry-point goal is dropped. |
| Backup On Save | preference | ✅ | ✅ | parity | P2 | medium | feasible | `BackupSettingsPanel.cpp:54`. iPad: lightweight take — `FolderConfigView.swift` "Backup on Save" toggle (`@AppStorage("backupOnSave")`, default OFF) → `SequencerViewModel.writeSaveBackup`/`pruneSaveBackups` snapshots the saved `.xsq`/package into `<show>/Backup/<name>-<timestamp>.<ext>`, keeping the most recent 20. (Single timestamped-folder strategy, not the full desktop On-Launch/Subfolders/Purge matrix.) |
| Backup On Launch | preference | ✅ | ❌ | desktop-missing | P3 | medium | feasible | `BackupSettingsPanel.cpp:57`. |
| Backup Subfolders | preference | ✅ | ❌ | desktop-missing | P3 | medium | feasible | `BackupSettingsPanel.cpp:60`. |
| Purge Backups interval | preference | ✅ | ❌ | desktop-missing | P3 | medium | feasible | `BackupSettingsPanel.cpp:67` (Never/360/90/30/7 days). |
| Backup Directory + Use Show Folder | preference | ✅ | ❌ | desktop-missing | P3 | medium | feasible | `BackupSettingsPanel.cpp:75`. Plus an Alternative Backup Directory picker. |
| View ▸ Effect Icon (toolbar) Size | preference | ✅ | ❌ | desktop-missing | P3 | medium | feasible | `ViewSettingsPanel.cpp:62` Small/Med/Large/XL → 16/24/32/48px. iPad icons scale to device. |
| View ▸ Model Handle Size | preference | ✅ | ❌ | desktop-missing | P3 | medium | feasible | `ViewSettingsPanel.cpp:70`. Layout-editor concern; iPad layout editor has no setting. |
| View ▸ Effect Assist Window mode | preference | ✅ | ❌ | desktop-missing | P3 | easy | feasible | `ViewSettingsPanel.cpp:78` (Always On/Off/Auto). iPad has no Effect Assist panel. |
| View ▸ Show Play Controls on Preview | preference | ✅ | ❌ | desktop-missing | P3 | easy | feasible | `ViewSettingsPanel.cpp:83`. iPad always shows controls. |
| View ▸ Auto Show House Preview | preference | ✅ | ❌ | desktop-missing | P3 | easy | feasible | `ViewSettingsPanel.cpp:86`. iPad house preview is a detachable scene. |
| View ▸ Disable Key Acceleration (held) | preference | ✅ | ❌ | desktop-missing | P3 | easy | infeasible | `ViewSettingsPanel.cpp:89`. Desktop key-repeat concept; n/a to touch. |
| View ▸ Enable Base Show Folder Settings | preference | ✅ | ✅ | parity | P2 | easy | feasible | Desktop checkbox `ViewSettingsPanel.cpp:92`; iPad full Base Show Folder section `FolderConfigView.swift:292` (change/clear/auto-update/update-now). |
| View ▸ Timeline Zooming (play vs mouse marker) | preference | ✅ | ✅ | parity | P3 | easy | feasible | `ViewSettingsPanel.cpp:97`. iPad: `@AppStorage("zoomToPlayMarker")` (default ON) in `XLZoomCommands` (`XLightsCommands.swift`); when on, Zoom In/Out/To re-anchor `hScrollOffsetPx` so the play marker stays at the same screen pixel; off anchors to the viewport left edge. Toggle ("Zoom To Play Marker") in the View ▸ zoom menu. |
| View ▸ Hide Preset Previews | preference | ✅ | ✅ | parity | P3 | easy | feasible | `ViewSettingsPanel.cpp:101`. Wave 6 added rendered preset thumbnails (`PresetThumbnailView`); `hidePresetPreviews` @AppStorage toggle in `FolderConfigView` falls the browser back to SF-Symbol icons. |
| View ▸ Zoom To Cursor | preference | ✅ | ❌ | desktop-missing | P3 | easy | infeasible | `ViewSettingsPanel.cpp:104`. Mouse-only behavior. |
| View ▸ Group Center Crosshair Size | preference | ✅ | ❌ | desktop-missing | P3 | medium | feasible | `ViewSettingsPanel.cpp:107` (Large/Normal/Small/Tiny/None). Layout-editor concern. |
| View ▸ Color Palette Size | preference | ✅ | ❌ | desktop-missing | P3 | easy | feasible | `ViewSettingsPanel.cpp:117` (Normal/Large). iPad palette is responsive. |
| Effects Grid ▸ Effect Backgrounds | preference | ✅ | ✅ | parity | P3 | medium | feasible | `EffectsGridSettingsPanel.cpp:66`. iPad: `@AppStorage("grid.showEffectBackgrounds")` (default ON) in `SequencerGridV2View` → `EffectsMetalGridView.showEffectBackgrounds` gates the `beginEffectBackgroundBatch`/`appendEffectBackground` loop; toggle in the top-chrome options menu. |
| Effects Grid ▸ Node Values | preference | ✅ | ❌ | desktop-missing | P3 | medium | feasible | `EffectsGridSettingsPanel.cpp:72`. |
| Effects Grid ▸ Group Effect Indicator | preference | ✅ | ❌ | desktop-missing | P3 | medium | feasible | `EffectsGridSettingsPanel.cpp:77`. |
| Effects Grid ▸ Snap to Timing Marks | preference | ✅ | ✅ | parity | P2 | medium | feasible | `EffectsGridSettingsPanel.cpp:83`. iPad gates `EffectsMetalGridView.swift:1576` snapMS on the `snapToTimingMarks` @AppStorage (default ON via absent-key check), toggled in the top-chrome options menu. |
| Effects Grid ▸ Small Waveform | preference | ✅ | ✅ | parity | P3 | medium | feasible | Desktop `EffectsGridSettingsPanel.cpp:95`; iPad `SequencerGridV2View.swift:233` (`waveformDoubleHeight` @AppStorage, default false), toggle at 1370-1374. |
| Effects Grid ▸ Display Transition Marks | preference | ✅ | ✅ | parity | P3 | medium | feasible | `EffectsGridSettingsPanel.cpp:101`. iPad: `@AppStorage("grid.showTransitionMarks")` (default ON) → `EffectsMetalGridView.showTransitionMarks` gates the fade-bar fills + fade-handle diamonds; toggle in the top-chrome options menu. Drag hit-testing still works whenever a handle is visible. |
| Effects Grid ▸ Hide Color Update Warning | preference | ✅ | ✅ | parity | P3 | easy | feasible | `EffectsGridSettingsPanel.cpp:106`. iPad: `@AppStorage("hideColorUpdateWarning")` (default OFF) toggle in the top-chrome options menu; consumed by `ColorPaletteView`'s "Update Palette" action — when on it calls `updatePaletteOnAllSelected()` directly, skipping the confirmation alert. |
| Effects Grid ▸ Show Alternate Timing Format | preference | ✅ | ✅ | parity | P3 | medium | feasible | `EffectsGridSettingsPanel.cpp:111` (s.ms display). iPad: `@AppStorage("grid.alternateTimingFormat")` (default OFF) → `TopChromeMetalGridView.alternateTimingFormat`; ruler labels render absolute seconds.ms instead of min:sec.frac. Toggle in the top-chrome options menu. |
| Effects Grid ▸ Bell on Render Completion | preference | ✅ | ✅ | parity | P3 | easy | feasible | `EffectsGridSettingsPanel.cpp:117`. iPad: `@AppStorage("bellOnRenderComplete")` (default OFF) in `FolderConfigView`; `SequencerViewModel.beginFreshRender` plays `AudioServicesPlaySystemSound(1057)` when the full RenderAll poll reports done. |
| Effects Grid ▸ Paste As (By Cell / By Time) | preference | ✅ | ✅ | parity | P2 | medium | feasible | `EffectsGridSettingsPanel.cpp:124` (desktop PASTE_BY_CELL/PASTE_BY_TIME). iPad: `@AppStorage("pasteByCell")` (default ON) Paste-Mode picker in the top-chrome options menu; consumed by `SequencerViewModel` paste path (`SequencerViewModel.swift:7114`) — "By Cell" stretches a single-effect paste to fill the active timing cell, "By Time" preserves the copied duration. (Desktop's Relative/Layers vocabulary maps to the same two cell-vs-time behaviors.) |
| Effects Grid ▸ Grid Spacing | preference | ✅ | ✅ | parity | P3 | medium | feasible | `EffectsGridSettingsPanel.cpp:58` (XS/S/M/L/XL → row height). iPad: `@AppStorage("grid.spacing")` picker in the top-chrome options menu scales `GridMetrics.rowHeight`/`timingRowHeight` (XS 0.66× … XL 1.5×) in `SequencerGridV2View.metrics`; the Metal grid + row headers read it via `metrics`. |
| Effects Grid ▸ Double-Click (Play Timing) | preference | ✅ | ✅ | parity | P3 | easy | feasible | `EffectsGridSettingsPanel.cpp:90` (Edit Text/Play Timing). iPad: `@AppStorage("timingPlayOnDoubleTap")` (default ON) "Timing Play on Double-Tap" toggle in the top-chrome options menu; consumed in `SequencerViewModel.swift:2230` — double-tapping a timing mark plays that span when on. (Touch maps double-tap to the desktop double-click; the Edit-Text mode is the always-available tap-to-rename.) |
| Sequences ▸ Render on Save | preference | ✅ | ✅ | parity | P3 | easy | feasible | `SequenceFileSettingsPanel.cpp:72`. iPad: `@AppStorage("renderOnSave")` (default ON, preserving the prior always-write behavior) in `FolderConfigView` gates the fseq companion write in `SequencerViewModel.saveSequence`. |
| Sequences ▸ Save FSEQ on Save | preference | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `SequenceFileSettingsPanel.cpp:78`; iPad `FolderConfigView.swift:182`. |
| Sequences ▸ FSEQ Version / format | preference | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `SequenceFileSettingsPanel.cpp:107` (V1/V2 ZSTD/Uncompressed/ZLIB/ZSTD-sparse). iPad `FolderConfigView.swift:208` (Zstd/Zlib/None). Different vocabulary; sparse mode not on iPad. |
| Sequences ▸ FSEQ Zstd Level (1–22) | preference | ❌ | ✅ | desktop-missing | P3 | easy | feasible | iPad-only granular slider `FolderConfigView.swift:214`. Desktop bundles level into the version choice. |
| Sequences ▸ Low Definition Render | preference | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `SequenceFileSettingsPanel.cpp:75`; iPad app-wide `FolderConfigView.swift:156` (read by `iPadRenderContext::IsLowDefinitionRender`). |
| Sequences ▸ Render Cache mode | preference | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `SequenceFileSettingsPanel.cpp:89` (Enabled/Locked Only/Disabled); iPad `FolderConfigView.swift:166`. iPad app-wide vs desktop per-show. |
| Sequences ▸ Maximum Render Cache Size | preference | ✅ | ✅ | parity | P3 | easy | feasible | `SequenceFileSettingsPanel.cpp:124` (Unlimited…200GB). iPad: `@AppStorage("render.cacheMaxMB")` picker in `FolderConfigView` (Unlimited / 50 MB … 5 GB, default 50 MB) read by `iPadRenderContext::ReadRenderCacheMaxMB()`, fed to `RenderCache::SetMaximumSizeMB` at construction + each render kickoff (LRU eviction past the cap). Smaller MB caps than desktop since iPad disk/memory are scarce. |
| Sequences ▸ Render Cache Directory | preference | ✅ | ❌ | desktop-missing | P3 | medium | feasible | `SequenceFileSettingsPanel.cpp:114`. iPad cache location is app-managed. |
| Sequences ▸ Auto-Save Interval | preference | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `SequenceFileSettingsPanel.cpp:96` (Disabled/3/5/10/15/30 min). iPad: `SequenceSettingsSheet` autosave picker now Off/3/5/10/15/30 (matching desktop) and persisted via `@AppStorage("autosaveIntervalMinutes")`; `SequencerViewModel.autosaveIntervalMinutes` restores that key at startup so the choice survives relaunch (prior to this it reset to 5 each launch). |
| Sequences ▸ Default Model Blending for new seq | preference | ✅ | ✅ | parity | P3 | easy | feasible | Desktop `SequenceFileSettingsPanel.cpp:83` (Enabled/Disabled). iPad: `@AppStorage("sequence.defaultModelBlending")` picker in `FolderConfigView` (default "Enabled" to preserve prior behavior); the bridge `newSequenceAtPath:` reads it via CFPreferences and sets `SequenceElements::SetSupportsModelBlending` before the initial Save so the `ModelBlending` attribute is written. Per-sequence override still in `SequenceSettingsSheet`. |
| Sequences ▸ FSEQ Directory | preference | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `SequenceFileSettingsPanel.cpp:140`; iPad `FolderConfigView.swift:192` (folder or next-to-sequence). |
| Sequences ▸ Media/Resource Directories | preference | ✅ | ✅ | parity | P1 | easy | feasible | Desktop list `SequenceFileSettingsPanel.cpp:147`; iPad Media Folders section `FolderConfigView.swift:124`. |
| Sequences ▸ Default View for new sequences | preference | ✅ | ❌ | desktop-missing | P3 | medium | deferred | `SequenceFileSettingsPanel.cpp:164`. **Deferred (2026-06-12):** desktop's default-view list comes from show-level views persisted in `xlights_rgbeffects.xml` (`frame->GetSequenceViews()`) and is applied at creation to choose which models populate the new sequence's *master* view. iPad views are per-sequence (loaded from the `.xsq` DisplayElements), and `newSequenceAtPath:` creates an empty sequence whose master view is populated from all show models on open — there's no show-level view list to pick from before load, and selective master-view population isn't modeled. Needs show-level view persistence + a creation-time master-view filter first; tracked here as the remaining theme-11 backlog item. |
| Output ▸ Use Frame Sync (E1.31) | preference | ✅ | ❌ | desktop-missing | P3 | hard | restricted | `OutputSettingsPanel.cpp:48`. iPad has no live USB/serial/sACN output. |
| Output ▸ Force Local IP | preference | ✅ | ❌ | desktop-missing | P3 | hard | restricted | `OutputSettingsPanel.cpp:57`. Output-stack tuning; out of scope on iPad. |
| Output ▸ Duplicate Frames to Suppress | preference | ✅ | ❌ | desktop-missing | P3 | hard | restricted | `OutputSettingsPanel.cpp:60` (None/10/20/40). |
| Output ▸ xFade/xSchedule port | preference | ✅ | ❌ | desktop-missing | P3 | hard | infeasible | `OutputSettingsPanel.cpp:66`. Companion-app integration; not on iPad. |
| Check Sequence (run + filtered report) | dialog | ✅ | ✅ | parity | P2 | easy | feasible | Desktop report; iPad `CheckSequenceSheet.swift` (severity/section chips, tap-to-jump). Same core checker. |
| Check Sequence ▸ Disable Duplicate Universe checks | preference | ✅ | ✅ | parity | P3 | easy | feasible | `CheckSequenceSettingsPanel.cpp:50` ("DupUniv"). iPad: gear menu in `CheckSequenceSheet.swift`, `csDisableDupUniv` @AppStorage → bridge `setCheckSequenceDisabledOptions` → `iPadRenderContext` disable set, read by both `SequenceCheckerCallbacks::IsCheckOptionDisabled` and `GetUICallbacks`. |
| Check Sequence ▸ Disable Non-Contiguous Channel checks | preference | ✅ | ✅ | parity | P3 | easy | feasible | `CheckSequenceSettingsPanel.cpp:54` ("NonContigChOnPort"). Same `CheckSequenceSheet` gear menu plumbing. |
| Check Sequence ▸ Disable Preview-Group checks | preference | ✅ | ✅ | parity | P3 | easy | feasible | `CheckSequenceSettingsPanel.cpp:58` ("PreviewGroup"). Same plumbing. |
| Check Sequence ▸ Disable Duplicate Nodes in Groups | preference | ✅ | ✅ | parity | P3 | easy | feasible | `CheckSequenceSettingsPanel.cpp:62` ("DupNodeMG"). Same plumbing. |
| Check Sequence ▸ Disable Transition-Time checks | preference | ✅ | ✅ | parity | P3 | easy | feasible | `CheckSequenceSettingsPanel.cpp:66` ("TransTime"). Same plumbing. |
| Check Sequence ▸ Disable Custom-Model-Size checks | preference | ✅ | ✅ | parity | P3 | easy | feasible | `CheckSequenceSettingsPanel.cpp:70` ("CustomSizeCheck"). Honoured both at the SequenceChecker level and the model-level check (CustomModel.cpp:812 via the iPad UICallbacks). |
| Check Sequence ▸ Disable Sketch-Image checks | preference | ✅ | ✅ | parity | P3 | easy | feasible | `CheckSequenceSettingsPanel.cpp:74` ("SketchImage"). Routes through `iPadRenderContext::GetUICallbacks` so SketchEffect.cpp:193 honours it. |
| Random Effects ▸ eligible-effect selection | preference | ✅ | ❌ | desktop-missing | P3 | medium | feasible | `RandomEffectsSettingsPanel.cpp` per-effect checkboxes feeding `RandomEffectsToUse`. iPad has no random-effect generator. |
| Colors ▸ Suppress Dark Mode | preference | ✅ | ❌ | desktop-missing | P3 | easy | feasible | `ColorManagerSettingsPanel.cpp:75`. iPad follows system appearance. |
| Colors ▸ UI color customization (Timing/Grid/Layout) + Import/Export/Reset | preference | ✅ | ❌ | desktop-missing | P3 | hard | feasible | `ColorManagerSettingsPanel.cpp` (per-element color pickers). iPad uses fixed theme colors. |
| Other ▸ Hardware Video Decoding (+ renderer) | preference | ✅ | ❌ | desktop-missing | P3 | hard | infeasible | `OtherSettingsPanel.cpp:93`. iPad uses AVFoundation/VideoToolbox automatically. |
| Other ▸ GPU Rendering (Metal compute) | preference | ✅ | ❌ | desktop-missing | P3 | hard | infeasible | `OtherSettingsPanel.cpp:186`. macOS/Win only; hidden on Linux. iPad always uses Metal. |
| Other ▸ Shaders on Background Threads | preference | ✅ | ❌ | desktop-missing | P3 | hard | infeasible | `OtherSettingsPanel.cpp:106`. Hidden on macOS/Linux already; Win-only. |
| Other ▸ Video Export Codec | preference | ✅ | ❌ | desktop-missing | P3 | hard | feasible | `OtherSettingsPanel.cpp:114` (Auto/H.264/H.265/MPEG-4). iPad has video export (✅ landed 2026-06-03) but picks codec **per-export** (per-model submenu / house-preview sheet), not via a global preference. |
| Other ▸ Video Export Bitrate | preference | ✅ | ❌ | desktop-missing | P3 | hard | feasible | `OtherSettingsPanel.cpp:122`. iPad has video export but no explicit bitrate field — `VideoWriter` chooses (constant-quality for HEVC). |
| Other ▸ Exclude Videos from Package | preference | ✅ | ✅ | parity | P3 | easy | feasible | Desktop `OtherSettingsPanel.cpp:129,266`; iPad `PackageSequenceSheet.swift:14,25,99-101`. |
| Other ▸ Exclude Audio from Package | preference | ✅ | ✅ | parity | P3 | easy | feasible | Desktop `OtherSettingsPanel.cpp:132,265`; iPad `PackageSequenceSheet.swift:13,24,99-100`. |
| Other ▸ Prompt Issues During Batch Render | preference | ✅ | ❌ | desktop-missing | P3 | easy | feasible | `OtherSettingsPanel.cpp:137`. iPad has Batch Render (`BatchRenderSheet`) but no prompt setting. |
| Other ▸ Purge Download Cache at Startup | preference | ✅ | ✅ | parity | P3 | easy | feasible | `OtherSettingsPanel.cpp:140`. iPad: `@AppStorage("purgeDownloadCacheAtStartup")` (default OFF) in `FolderConfigView`; `XLightsApp.init` calls `viewModel.purgeDownloadCache()` at launch when set (alongside the existing manual Tools ▸ Purge Download Cache). |
| Other ▸ Ignore Vendor Model Recommendations | preference | ✅ | ❌ | desktop-missing | P3 | easy | feasible | `OtherSettingsPanel.cpp:143` (Win-only; hidden elsewhere unless `IGNORE_VENDORS`). |
| Other ▸ Tip of the Day (level + recycle) | preference | ✅ | ❌ | desktop-missing | P3 | medium | feasible | `OtherSettingsPanel.cpp:146`. iPad has no Tip-of-Day feature. |
| Other ▸ Link Controller Upload | preference | ✅ | ❌ | desktop-missing | P3 | medium | restricted | `OtherSettingsPanel.cpp:166` (None / Inputs+Outputs). Controller-config concern. |
| Other ▸ Model Rename Alias behavior | preference | ✅ | ✅ | parity | P3 | easy | feasible | `OtherSettingsPanel.cpp:174` (Always Prompt/Yes/No). iPad: `@AppStorage("modelRenameAliasMode")` picker in `FolderConfigView` seeds the default "Add alias" state of each row in `MissingModelAliasSheet` ("Always No" → off; Always Prompt / Always Yes → on). The per-row toggle is still the prompt — closest touch-idiom mapping of the desktop modes. |
| Other ▸ eMail address | preference | ✅ | ✅ | parity | P3 | easy | feasible | `OtherSettingsPanel.cpp:183`. iPad: `@AppStorage("xLightsEmail")` text field in `FolderConfigView` "Other" section; `XLDiagnosticUploader.buildMultipartBody` attaches it as the `email` multipart form field on uploaded diagnostic reports (skipped when blank), mirroring the desktop crash-report email. |
| Other ▸ Controller Ping Interval | preference | ✅ | ❌ | desktop-missing | P3 | hard | restricted | `OtherSettingsPanel.cpp:194`. Live-output tuning; out of scope. |
| Other ▸ Moving-Head Position Zones (enable + indicator) | preference | ✅ | ❌ | desktop-missing | P3 | medium | feasible | `OtherSettingsPanel.cpp:200`. Moving-head authoring concern. |
| Other ▸ Use Custom Color Picker | preference | ✅ | ❌ | desktop-missing | P3 | hard | infeasible | `OtherSettingsPanel.cpp:208`. iPad uses the native system ColorPicker. |
| Services ▸ AI Services config (keys/model/test) | preference | ✅ | ✅ | parity | P2 | easy | feasible | Desktop `ServicesPanel.cpp` (wxPropertyGrid + Test); iPad `AIServicesSettingsSheet.swift` (Form + Test + Refresh model list). Both via core `aiBase`. |
| Keyboard-shortcut rebinding editor | dialog | ✅ | ❌ | ipad-missing | P2 | hard | feasible | Desktop `KeyBindingEditDialog` (File ▸ "Key bindings", `xLightsMain.cpp:8653`). iPad shortcuts are static in `XLightsCommands.swift`. |
| Command palette (fuzzy command/effect launcher) | dialog | ✅ | ✅ | parity | — | done | done | Desktop #6258: `CommandPaletteDialog::OnCommandPalette` (`src-ui-wx/xLightsMain.cpp:8009`, Cmd+Shift+K) — fuzzy menu-command + effect search, runs the command or drops the effect on the selected timing region. iPad: `CommandPaletteSheet.swift` + `CommandPaletteRegistry.swift`, bound to ⌘⇧K in `XLightsCommands.swift` ("Command Palette…"), presented from `SequencerView.swift`. Two sections (Commands / Effects); the same subsequence scoring as desktop (`CommandPaletteMatch`, prefix > word-boundary > scattered), arrow/return/escape keyboard nav, SF-Symbol icons + effect-icon thumbnails, shortcut badges. Commands route through the same `viewModel` calls/tokens as the menu and honour their enabled-gates (disabled commands filtered out, matching desktop's `IsEnabled()` skip). Effects drop at the play head on the active row via `viewModel.dropEffectFromCommandPalette` (reuses `addEffectFromPaletteTap` timing-cell snapping). |
| Keyboard-shortcut reference (read-only list) | dialog | ✅ | ✅ | parity | P3 | easy | feasible | Desktop Help ▸ "Key Bindings" dump (`xLightsMain.cpp:7966`). iPad gets the system Menu Bar ▸ Keyboard Shortcuts overlay for free. |
| Save (Cmd+S) | shortcut | ✅ | ✅ | parity | P1 | easy | feasible | Desktop SAVE_CURRENT_TAB=Ctrl+S `KeyBindings.cpp:342`; iPad `XLightsCommands.swift:47`. |
| Save As (Cmd+Shift+S) | shortcut | ✅ | ✅ | parity | P1 | easy | feasible | Desktop SAVEAS_SEQUENCE (Ctrl+Shift+S) `KeyBindings.cpp:344`; iPad `XLightsCommands.swift:53`. |
| Close sequence (Cmd+W) | shortcut | ✅ | ✅ | parity | P1 | easy | feasible | `KeyBindings.cpp:310`; iPad `XLightsCommands.swift:41`. |
| Open sequence (Cmd+O) | shortcut | ✅ | ❌ | ipad-missing | P3 | easy | feasible | `KeyBindings.cpp:309`. iPad opens from picker UI; no menu command. |
| New sequence (Cmd+N) | shortcut | ✅ | ❌ | ipad-missing | P3 | medium | feasible | `KeyBindings.cpp:311`. iPad uses New Sequence wizard from picker. |
| Undo (Cmd+Z) / Redo (Cmd+Shift+Z) | shortcut | ✅ | ✅ | parity | P1 | easy | feasible | Desktop uses native edit menu; iPad `XLightsCommands.swift:74,81` (with action-name in label). |
| Copy / Paste / Duplicate / Delete / Select-All / Clear (Escape) | shortcut | ✅ | ✅ | parity | P1 | easy | feasible | Desktop SELECT_ALL=Ctrl+A `KeyBindings.cpp:303` + grid handlers; iPad `XLightsCommands.swift:113–148`. |
| Find / Replace (Cmd+F) | shortcut | ❌ | ✅ | desktop-missing | P2 | medium | feasible | iPad `XLightsCommands.swift:93` → `FindReplaceSheet` (timing-mark label search). Desktop has no sequencer find. |
| Color Replace | menu/dialog | ✅ | ✅ | parity | P2 | medium | feasible | Desktop `ColourReplaceDialog` (Edit menu, `xLightsMain.cpp:8848`, no shortcut); iPad `ColorReplaceSheet` (`XLightsCommands.swift:100`, no shortcut). |
| Effect Presets browser (Cmd+Shift+P) | shortcut | 🟡 | ✅ | parity | P2 | medium | feasible | Desktop SHOW_PRESETS / PRESETS_TOGGLE=F10 (`KeyBindings.cpp:374`) — a panel, not a Cmd+Shift+P. iPad `XLightsCommands.swift:106` → `PresetBrowserSheet`. Both have presets; binding differs. |
| Play / Pause (Space) | shortcut | ✅ | ✅ | parity | P1 | easy | feasible | TOGGLE_PLAY=SPACE `KeyBindings.cpp:320`; iPad `XLightsCommands.swift:382`. |
| Stop (menu, no key) | shortcut | ✅ | ✅ | parity | P2 | easy | feasible | Desktop STOP unbound; iPad menu item `XLightsCommands.swift:385`. |
| Render All (Cmd+R) | shortcut | 🟡 | ✅ | desktop-missing | P2 | easy | feasible | Desktop RENDER_ALL exists but **no default key** (`KeyBindings.cpp:312`) — toolbar only. iPad Cmd+R `XLightsCommands.swift:398`. |
| Rewind to Start (Home) / Jump to End (End) | shortcut | ✅ | ✅ | parity | P1 | easy | feasible | START_OF_SONG/END_OF_SONG `KeyBindings.cpp:317`; iPad `XLightsCommands.swift:404,410`. |
| Previous / Next Frame (, / .) | shortcut | ❌ | ✅ | desktop-missing | P3 | easy | feasible | iPad `XLightsCommands.swift:455,462`. Desktop has no per-frame step keys. |
| Back / Forward 10 s | shortcut | ❌ | ✅ | desktop-missing | P3 | easy | feasible | iPad menu items `XLightsCommands.swift:417,422` (no key — ⌥← reserved for nudge). |
| Previous / Next Effect (← / →) | shortcut | ❌ | ✅ | desktop-missing | P2 | medium | feasible | iPad `XLightsCommands.swift:471,475`. Desktop arrows do other things. |
| Effect Above / Below (↑ / ↓) | shortcut | ❌ | ✅ | desktop-missing | P2 | medium | feasible | iPad `XLightsCommands.swift:479,483` row navigation. |
| Tab / Shift+Tab effect navigation | shortcut | 🟡 | ✅ | desktop-missing | P3 | easy | feasible | iPad `XLightsCommands.swift:492,496`. Desktop grid may consume Tab but it's not a documented binding. |
| Stretch End Back/Forward (Shift+←/→) | shortcut | 🟡 | ✅ | desktop-missing | P3 | easy | feasible | iPad `XLightsCommands.swift:158,164`. Desktop stretches via mouse/handles; no documented Shift+arrow binding. |
| Fine nudge ±1 ms (Ctrl+←/→) | shortcut | ❌ | ✅ | desktop-missing | P3 | easy | feasible | iPad `XLightsCommands.swift:172,178`. |
| Nudge ±1 frame (⌥←/→) | shortcut | ❌ | ✅ | desktop-missing | P3 | easy | feasible | iPad `XLightsCommands.swift:186,192`. |
| Extend to Prev/Next Effect (Shift+Cmd+←/→) | shortcut | ❌ | ✅ | desktop-missing | P3 | easy | feasible | iPad `XLightsCommands.swift:203,209`. |
| Nudge to Prev/Next Timing Mark (Ctrl+PgUp/PgDn) | shortcut | ❌ | ✅ | desktop-missing | P3 | easy | feasible | iPad `XLightsCommands.swift:221,227`. Desktop has no documented binding for this. |
| Go To Tag N (Ctrl+0–9) | shortcut | ✅ | ✅ | parity | P2 | easy | feasible | Desktop PRIOR_TAG/NEXT_TAG only navigate; numbered jump is iPad-rich. iPad `XLightsCommands.swift:506`. |
| Set Tag N at play head (Ctrl+Shift+0–9) | shortcut | ❌ | ✅ | desktop-missing | P3 | easy | feasible | iPad `XLightsCommands.swift:516`. Desktop sets tags via UI. |
| Clear All Tags | shortcut | 🟡 | ✅ | desktop-missing | P3 | easy | feasible | iPad menu item `XLightsCommands.swift:527`. |
| Zoom In (Cmd+=) / Out (Cmd+-) | shortcut | ✅ | ✅ | parity | P1 | easy | feasible | ZOOM_IN=+ / ZOOM_OUT=- `KeyBindings.cpp:347`; iPad `XLightsCommands.swift:652,655`. |
| Zoom To… presets | shortcut | ❌ | ✅ | desktop-missing | P3 | medium | feasible | iPad `XLightsCommands.swift:663`. Desktop has ZOOM_SEL (unbound) but no preset ladder in the menu. |
| Toggle Preview (Cmd+1) / Inspector (Cmd+2) | shortcut | ❌ | ✅ | desktop-missing | P3 | medium | feasible | iPad `XLightsCommands.swift:245,251`. Desktop has no panel-toggle by Cmd+digit. |
| Edit Display Elements (Cmd+Shift+D) | shortcut | ✅ | ✅ | parity | P2 | easy | feasible | Desktop DISPLAY_ELEMENTS_TOGGLE=F7 `KeyBindings.cpp:359`; iPad Cmd+Shift+D `XLightsCommands.swift:280`. |
| Detach inspector tab in new window (⌥⌘E/C/B/U) | shortcut | ❌ | ✅ | desktop-missing | P3 | medium | feasible | iPad `XLightsCommands.swift:778`. Desktop's F1–F4 toggle inline panels, a different model. |
| Detach / Dock House & Model Preview | shortcut | 🟡 | ✅ | desktop-missing | P3 | medium | feasible | iPad `XLightsCommands.swift:679` (menu, no key). Desktop MODEL_PREVIEW_TOGGLE=F5 / HOUSE_PREVIEW_TOGGLE=F6 toggle inline panels (different concept). |
| Add Timing Mark (T) | shortcut | ✅ | 🟡 | ipad-missing | P3 | easy | feasible | Desktop TIMING_ADD=t `KeyBindings.cpp:345`. iPad *has* `addTimingMark` (`SequencerViewModel.swift:3612`) via long-press; no key. |
| Split Timing Mark (S) | shortcut | ✅ | 🟡 | ipad-missing | P3 | easy | feasible | Desktop TIMING_SPLIT=s `KeyBindings.cpp:346`. iPad has `splitTimingMark` but no key. |
| Insert Layer Above/Below (Shift+I / Shift+A) | shortcut | ✅ | 🟡 | ipad-missing | P3 | easy | feasible | Desktop `KeyBindings.cpp:371`. iPad has it via row-header context menu (`RowHeaderViews.swift:628`); no key. |
| Toggle Element Expand (Shift+X) | shortcut | ✅ | 🟡 | ipad-missing | P3 | easy | feasible | Desktop `KeyBindings.cpp:373`. iPad expands via tap; no key. |
| Lock (L) / Unlock (U) effect | shortcut | ✅ | 🟡 | ipad-missing | P3 | easy | feasible | Desktop `KeyBindings.cpp:361`. iPad has lock/unlock in context menu (`SequencerGridV2View.swift:578`); no key. |
| Mark Spot (Ctrl+.) / Return to Spot (Ctrl+/) | shortcut | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop `KeyBindings.cpp:363`. No iPad equivalent. |
| Effect Description dialog | shortcut/dialog | ✅ | ✅ | parity | P3 | easy | feasible | Desktop EFFECT_DESCRIPTION (unbound) `KeyBindings.cpp:367`. iPad has it (B20, `SequencerViewModel.swift:2992`); neither has a default key. |
| Inline panel toggles F1–F4 (Settings/Color/Layer/Blending) | shortcut | ✅ | 🟡 | ipad-missing | P2 | medium | feasible | Desktop `KeyBindings.cpp:351–355`. iPad inspector tabs are always available (Cmd+2 shows/hides, ⌥⌘ detaches); no F-key tab cycling. |
| Effect Assist toggle (F8) | shortcut | ✅ | ❌ | desktop-only | P3 | n/a | infeasible | `KeyBindings.cpp:352`. iPad has no Effect Assist panel. |
| Effects panel (F9) / Presets panel (F10) | shortcut | ✅ | 🟡 | ipad-missing | P3 | easy | feasible | `KeyBindings.cpp:358,375`. iPad uses the effect library + preset browser; no F-keys. |
| Value Curves dropper (F12) / Color Dropper (F11) | shortcut | ✅ | 🟡 | ipad-missing | P3 | medium | feasible | `KeyBindings.cpp:376,377`. iPad value curves live in the inspector; no separate toggle/key. |
| Search panel (F11) | shortcut | ✅ | 🟡 | ipad-missing | P3 | easy | feasible | SEARCH_TOGGLE=F11 `KeyBindings.cpp:379`. iPad has Find (Cmd+F) — overlapping intent. |
| Filter Sequencer (f) | shortcut | ✅ | ❌ | ipad-missing | P3 | medium | feasible | FILTER_SEQUENCER=f `KeyBindings.cpp:380`. iPad has no model-row filter. |
| Perspectives panel (F12) | shortcut | ✅ | ❌ | desktop-only | P3 | n/a | infeasible | `KeyBindings.cpp:381`. iPad has no perspectives feature. |
| Jukebox panel (Ctrl+F8) + buttons | shortcut | ✅ | ❌ | desktop-only | P3 | n/a | infeasible | `KeyBindings.cpp:360`. iPad has no Jukebox. |
| Random effect insertion (Shift+R) | shortcut | ✅ | ❌ | ipad-missing | P3 | medium | feasible | RANDOM `KeyBindings.cpp:350`. iPad has no random-effect generator. |
| ~20 single-key effect-insert bindings (o,m,c,i,b,y,f,g,p,r,x,w,n,…) | shortcut | ✅ | ❌ | ipad-missing | P3 | hard | feasible | `KeyBindings.cpp:413–432` insert preset On/Morph/Curtain/etc. iPad inserts effects via drag/long-press; mapping single keys to a touch grid is awkward. |
| Effect-update / fade-in (U) / fade-out (D) bindings | shortcut | ✅ | ❌ | ipad-missing | P3 | medium | feasible | `KeyBindings.cpp:382,431,432`. Apply transition presets to selection. |
| Backup (F10) / Alternate Backup (F11) / Select Show Folder (F9) | shortcut | ✅ | ❌ | desktop-only | P3 | n/a | feasible | `KeyBindings.cpp:305–307`. iPad has no manual-backup action; show-folder switch is in FolderConfig. |
| Lights toggle / FPP Connect (Ctrl+Shift) bindings | shortcut | ✅ | ❌ | desktop-only | P3 | n/a | restricted | `KeyBindings.cpp:308,340`. Live output / controller concerns. |

## iPad gaps (desktop has, iPad missing)

### P2

- **Keyboard-shortcut rebinding editor.** Desktop ships `KeyBindingEditDialog`
  (`src-ui-wx/app-shell/KeyBindingEditDialog.cpp`, invoked from File ▸ "Key
  bindings" at `xLightsMain.cpp:8653`) editing 137 bindings across 4 scopes
  (All/Sequence/Layout/…). iPad shortcuts are static in
  `XLightsCommands.swift`. Work: a Settings screen that maps the existing
  `XLSequencerCommands` actions to user-chosen `KeyEquivalent`s, persisted in
  `@AppStorage`, applied at command-build time. Medium-hard (SwiftUI key
  capture + a stable action registry). Lower urgency since hardware keyboards
  are a minority on iPad.
- ✅ **Snap to Timing Marks toggle.** Done. `EffectsGridSettingsPanel.cpp:83`.
  The `snapToTimingMarks` @AppStorage (default ON) gates `EffectsMetalGridView`'s
  `snapMS`; toggle lives in the top-chrome options menu.
- ✅ **Paste As (By Cell / By Time).** Done. `EffectsGridSettingsPanel.cpp:124`.
  `@AppStorage("pasteByCell")` Paste-Mode picker in the top-chrome options menu,
  consumed by the `SequencerViewModel` paste path (`SequencerViewModel.swift:7114`).
- **Inline panel toggle keys (F1–F4) parity.** Desktop F1/F2/F3/F4 cycle the
  Settings/Color/Layer/Blending panels (`KeyBindings.cpp:351`). iPad has the
  tabs but no F-key cycling — `XLightsCommands.swift` only offers Cmd+2
  show/hide and ⌥⌘ detach. Add F-key (or Cmd+digit) bindings that select the
  inspector tab. Easy once a rebinding/registry exists. Medium.

### P3 (representative — see scorecard for the full list)

- **Backup strategy** (On Save / On Launch / Subfolders / Purge / Directory):
  `BackupSettingsPanel.cpp`. ✅ **Backup On Save done 2026-06-12** — the
  lightweight take: `backupOnSave` @AppStorage snapshots the saved `.xsq`/package
  into `<show>/Backup/<name>-<timestamp>.<ext>` (keep last 20) via
  `SequencerViewModel.writeSaveBackup`. The On-Launch / Subfolders / Purge /
  Directory variants remain desktop-only for now (iPad also leans on autosave +
  iCloud/Files version history).
- **Grid-display toggles** (Node Values, Effect Backgrounds, Group Indicator,
  Small Waveform, Transition Marks, Alternate Timing Format, Bell on render,
  Grid Spacing): all in `EffectsGridSettingsPanel.cpp`. ✅ **Done 2026-06-12**
  for Effect Backgrounds, Transition Marks, Alternate Timing Format, Bell on
  render, and Grid Spacing — all five gated on `@AppStorage` and surfaced in
  the top-chrome options menu (Bell lives in `FolderConfigView`). ✅ **Hide
  Color Update Warning done 2026-06-12** — top-chrome toggle consumed by
  `ColorPaletteView`'s Update-Palette action (skips the confirm alert).
  Remaining: Node Values, Group Indicator (both still need the Metal-grid
  render path).
- **Render Cache size limit / directory, Max Render Cache, Default Model
  Blending, Default View, Render-on-Save**: `SequenceFileSettingsPanel.cpp`.
  ✅ **Done 2026-06-12**: Maximum Render Cache Size, Default Model Blending,
  and Render-on-Save (all in `FolderConfigView`, see scorecard). Render Cache
  *directory* stays app-managed; Default View is deferred (see its scorecard
  note — needs show-level view persistence first).
- ✅ **Check-Sequence disable-checks toggles** (7 of them): Done.
  `CheckSequenceSettingsPanel.cpp:50–74`. `CheckSequenceSheet` now has a gear
  menu (`slider.horizontal.3`) with all 7 toggles persisted via `@AppStorage`.
  They flow through the new bridge `setCheckSequenceDisabledOptions:` into
  `iPadRenderContext`'s disable set, consulted by both the
  `SequenceCheckerCallbacks::IsCheckOptionDisabled` path and (for the
  model/effect-level CustomSizeCheck + SketchImage checks) the newly-added
  `iPadRenderContext::GetUICallbacks()`. Flipping a toggle re-runs the check.
- **Tip of the Day, Random-Effect weights, Color customization, Suppress Dark
  Mode, Prompt batch-render issues.** Mostly easy settings; each maps to
  an existing core getter/setter. (✅ **Alias prompt mode**, ✅ **Purge
  download cache at startup**, and ✅ **eMail address** done in `FolderConfigView`
  — see scorecard. **Prompt batch-render issues** skipped: the iPad batch-render
  runner doesn't run a per-item check-sequence pass, so there's no issue prompt
  to gate — needs that subsystem first.)
- **Single-key effect-insert bindings (~20)** and **Random insert (Shift+R)**:
  `KeyBindings.cpp:350,413`. Touch-first iPad inserts via the drag/long-press
  effect library; remapping single keys to a touch grid is awkward and low
  value. Hard / low priority.
- **Add Timing (T), Split Timing (S), Insert Layer (Shift+I/A), Toggle Expand
  (Shift+X), Lock/Unlock (L/U):** iPad already *has* these features via
  context menus / long-press; only the keyboard shortcut is missing. Trivial to
  add once a binding table exists. Easy.

## Desktop gaps (iPad has, desktop missing)

- **Find / Replace (Cmd+F).** iPad `FindReplaceSheet` + `XLightsCommands.swift:93`
  searches timing-mark labels. Desktop has no sequencer-level find. P2, medium.
- **Render All keyboard shortcut.** iPad binds Cmd+R (`XLightsCommands.swift:398`);
  desktop RENDER_ALL has no default key (`KeyBindings.cpp:312`) — toolbar only.
  Trivial to add a default binding. P2/P3, easy.
- **Per-frame / per-effect navigation keys.** iPad has Previous/Next Frame
  (`,`/`.`), Previous/Next Effect (←/→), Effect Above/Below (↑/↓), Tab cycling,
  fine ±1 ms / ±1 frame nudges, extend-to-neighbour, nudge-to-timing-mark, and a
  Zoom-To preset ladder (`XLightsCommands.swift:451–671`). Desktop exposes few of
  these as keys; they could be added to the desktop binding defaults. P3, easy.
- **Set Tag N at play head (Ctrl+Shift+digit)** and **numbered Go-To-Tag
  (Ctrl+digit).** iPad `XLightsCommands.swift:506,516`. Desktop only has
  PRIOR_TAG/NEXT_TAG navigation. P3, easy.
- **FSEQ Zstd compression level (1–22).** iPad `FolderConfigView.swift:214`
  exposes a granular slider; desktop bundles level into the FSEQ-version choice.
  P3, easy.
- **Cmd+digit panel toggles & ⌥⌘ inspector-tab detach windows.** iPad's
  multi-window scene model (`XLightsCommands.swift:715–786`) is richer than
  desktop's inline F-key panels; these are touch/window idioms and need no
  desktop counterpart.

> Note: most "desktop-missing" items here are *missing keyboard bindings*, not
> missing features — desktop generally has the underlying capability and could
> just gain a default binding (editable in `KeyBindingEditDialog`).

## Infeasible / restricted on iPad

- **Output-protocol tuning** — Frame Sync (E1.31), Force Local IP, Suppress
  Duplicate Frames, Controller Ping Interval, xFade/xSchedule port,
  Link-Controller-Upload (`OutputSettingsPanel.cpp`, `OtherSettingsPanel.cpp`):
  iPad has no live USB/serial/sACN output stack. *restricted/infeasible.*
- **Hardware Video Decoding / renderer choice, GPU Rendering, Shaders on
  Background Threads** (`OtherSettingsPanel.cpp`): iPad always uses Metal +
  VideoToolbox automatically; these are platform-managed. *infeasible.*
- **Video Export Codec / Bitrate** (`OtherSettingsPanel.cpp:114,122`): iPad now
  has video export (house-preview + per-model, ✅ 2026-06-03) but selects codec
  **per-export** rather than via a global preference, and has no explicit bitrate
  field (`VideoWriter` chooses). *feasible as a pref, but per-export is the iPad idiom.*
- **Use Custom Color Picker** (`OtherSettingsPanel.cpp:208`): iPad uses the
  native system ColorPicker. *infeasible (n/a).*
- **Disable Key Acceleration / Zoom-To-Cursor** (`View` &
  `Effects Grid` panels): mouse/keyboard-repeat idioms with no touch analogue.
  *infeasible (n/a).* (Double-Click Mode is now mapped — double-tap toggle in
  the options menu, see scorecard.)
- **Jukebox, Perspectives, Effect Assist** F-key panels (`KeyBindings.cpp`):
  those features don't exist on iPad. *desktop-only.*
- **~20 single-key effect-insert bindings**: technically possible but a poor fit
  for a touch-first effect-library workflow. *feasible but very low priority.*

## Recommended sequencing

1. ✅ **Check-Sequence disable-checks toggles** — Done. 7 controls in the
   `CheckSequenceSheet` gear menu, plumbed through the bridge into the core
   disable set (both the SequenceChecker callbacks and the model/effect-level
   UICallbacks path).
2. ✅ **Render-cache size limit + Default Model Blending + Render-on-Save** —
   Done 2026-06-12 in `FolderConfigView` (+ the `newSequenceAtPath:` bridge for
   blending). **Default View** deferred — it needs show-level view persistence
   first (see its scorecard note).
3. ✅ **Paste As** (By Cell / By Time) and ✅ **Snap to Timing Marks** — Done.
   Both surfaced in the top-chrome options menu and consumed by the
   `SequencerViewModel` paste / drag paths.
4. ✅ **Lightweight Backup-on-save toggle** — Done 2026-06-12. Snapshots the
   saved `.xsq`/package into `<show>/Backup/` (keep last 20); simpler than the
   full desktop On-Launch/Subfolders/Purge matrix.
5. ✅ **Grid-display toggles** — Done 2026-06-12 for Effect Backgrounds,
   Transition Marks, Alternate Timing Format, Bell on render, and Grid Spacing
   (top-chrome options menu + `FolderConfigView` for the bell). Remaining
   incremental-parity items: Tip-of-Day, Random-Effect weights, Node Values,
   Group Indicator.
6. **Keyboard-shortcut rebinding editor + missing default bindings on both
   sides** — last because hardware keyboards are a minority on iPad and the
   feature surface (action registry, key capture, persistence) is the largest;
   in the same pass, add desktop default keys for Render All / per-frame nav so
   the two binding tables converge.
