# Phase B — Effects Grid Parity (Pending Work)

Scope: the sequencer canvas the user spends 90 % of their time in —
the **effects grid**, **timeline ruler**, **waveform**, **row
headings**, and the **timing tracks** strip between them. Phase B and
B-Metal closed out the rebuild and the Metal render pipeline; this
sub-plan tracks the remaining authoring gaps vs desktop.

---

## Status (2026-04-21)

All original P0s closed, plus 20+ P1s landed across the 2026-04-20
and 2026-04-21 sessions. What remains is a short tail of P1 polish
+ a long tail of P2 nice-to-haves.

**Current counts:** 0 × P0, **0 × P1**, ~26 × P2, 0 × Verify,
3 × Deferred, 1 × Removed. (B26 ColorCurve gradient confirmed
landed via BM-6 device verify; B64 layer-count `[N]` indicator
confirmed already shipping in `RowHeaderViews.swift:352`. B34
tags + B35 tag menu + B49 export model + B94 scrollbars + B15
randomise/reset + B19 presets stub landed 2026-04-22. Second
2026-04-28 batch added B25 (ColorManager bracket palette), B40
(audio scrub during ruler drag), B43 (alt-track waveform
switch), B84-per-mark (single-phrase breakdown). Third
2026-04-28 batch added B55 (Convert Effects to Per Model), B56
Promote (Promote Node Effects), B97 (Find / Replace panel).
Fourth 2026-04-28 batch closed B23 + B60 as already-covered
elsewhere (multi-effect clipboard / F-6 Display Elements editor)
and deferred B24 (Find Possible Source Effects — needs new
diagnostic panel + per-element channel resolution bridge).
B56's Convert-To-Effect deferred (needs core helper lift).
B7 + B23 + B27 + B60 closed 2026-04-28 as no-desktop-counterpart
/ already covered after grep audit.) **All P1 gaps closed** —
remaining work is P2 polish only.

### What landed since the 2026-04-20 gap audit

- **Multi-select + align + split + playhead follow** — B1, B2, B5,
  B8, B9, B10, B11, B12, B14, B21, B31, B36, B37, B39, B93.
  Two-finger marquee, every align variant (start / end / both /
  centers / match / shift-start / shift-end / align-to-mark /
  close-gap), split-at-play-marker, exact-time dialog, zoom-to-fit
  / -selection, drag-to-scrub, follow-playhead, selection status
  readout.
- **Timing-mark editing** — B67, B68, B69, B70, B71, B72, B73.
  Long-press create, pan move / resize (snap to other marks),
  delete, rename, split, merge with next, add timing track.
- **Lyric rendering + breakdown** — B84 (phrase → words), B85
  (words → phonemes using the wx-free `PhonemeDictionary` ported
  to `src-core/lyrics/`), B87 (remove words/phonemes), B88
  (per-layer label colour coding), B78 (import lyrics sheet),
  B89 (auto-label marks).
- **Loop region + play-loop + render-selected** — B32, B33, B44,
  B45. Long-press on ruler sets region; menu has Play Loop /
  Render Loop Region / Clear Loop.
- **Waveform filters** — B41. Long-press on waveform → Full Range
  / Bass / Treble / Alto / Non-Vocals picker via
  `AudioManager::GetFilteredAudioData`.
- **Row-heading expansion** — B46 (rename layer), B50 (delete all
  effects on row), B51 (enable / disable render on model), B52
  (select all effects in model), B53 / B54 (cut / copy row +
  model), B57 (expand / collapse all), B61 (drag-resize header
  column width), B65 (tooltip on truncated row names).
- **Multi-effect clipboard + keyboard editing** — B4 (shift /
  ctrl / alt arrow stretch + nudge), B98 (relative-timing
  clipboard).
- **Subdivided timing tracks** — B80 (1/2 .. 8× submenu).
- **`.xtiming` I/O** — B74 import, B75 export.
- **Hover states** — B30. `.hoverEffect` on SwiftUI row-header
  controls + `UIHoverGestureRecognizer` on the Metal grid for
  Magic Keyboard pointer users.
- **Scroll polish** — B95 (trackpad / wheel).
- **Tenth P2 polish batch (2026-04-22)** — B96.
  - **B96**: Momentum scroll on the model-effects canvas. On
    pan-release the recognizer's velocity starts a
    `CADisplayLink`-driven coast with exponential decay (93 %
    per frame at 60 Hz ≈ ~0.5 s visible coast for typical touch
    flings). A new pan cancels the coast so users don't fight the
    inertia. Gated to `|v| > 80 px/s` so trackpad two-finger
    scrolls (Apple's gesture decomposition reports tiny terminal
    velocities) don't overshoot.
- **Ninth P2 polish batch (2026-04-22)** — B99.
  - **B99**: System-pasteboard integration. `ClipboardEntry`
    became `Codable` and every write to `clipboardEntries`
    publishes a JSON blob to `UIPasteboard.general` under the
    `com.xlights.effectclip` UTI. `hasClipboard` consults the
    pasteboard too, and paste paths call
    `syncClipboardFromPasteboard()` just-in-time so a fresh app
    launch (or a Universal Clipboard drop-in from another
    device) surfaces what was copied earlier. No new Info.plist
    declaration — the dynamic UTI is created on first write.
- **Eighth P2 polish batch (2026-04-22)** — B28 / B63.
  - **B28**: Previous-selection indicator. The view model tracks
    the prior `selectedEffect` across changes; the grid paints a
    muted amber outline on that effect so users can compare it
    against the current selection at a glance. Cleared on any
    `clearSelection`.
  - **B63**: Icon glyphs on lyric-breakdown timing layers
    (Phrases / Words / Phonemes). Uses SF Symbols
    (`text.bubble`, `textformat.abc`, `waveform.path`) so no
    desktop-XPM port is needed. Plain timing tracks stay
    un-decorated — the coloured dot already signals the row
    type. ModelGroup folder icon (already landed) covers the
    model-tree side.
- **Seventh P2 polish batch (2026-04-22)** — B29 / B58 / B100.
  - **B29**: Effect-bar label now font-steps (11 → 9 → 8 → 7 pt)
    and fades to 55 % at the smallest widths instead of hard-
    cutoff at 70 pt. The bracket-line `rightStart` math matches
    so the line never draws through the now-visible label at
    narrow widths.
  - **B58**: Toggle Strands / Nodes is already covered by the
    per-element "Show / Hide Strands/Submodels" and
    "Show / Hide Nodes" entries landed with B52/B54. Global
    versions are a no-op — desktop's ID_ROW_MNU_TOGGLE_* are
    also per-element. Verified + closed.
  - **B100**: Paste-replace-with-confirm. Effect context menu's
    "Paste Here" now detects overlap with existing effects;
    when overlapping, pops a destructive-role "Replace?" alert
    instead of silently failing. Confirm routes through new
    `pasteEffectReplacingOverlaps` which deletes the overlapping
    effects and pastes in a single undo group.
- **Sixth P2 polish batch (2026-04-22)** — B18 / B20 / B82.
  - **B18**: Double-tap empty area of a model row (palette armed)
    creates an effect that fills the enclosing cell — active
    timing cell if one brackets the tap, otherwise the gap
    between row-neighbours. New `onDoubleTapEmpty` action on
    `EffectCanvasActions` + `doubleTapCreateInCell` on the view
    model.
  - **B20**: "Edit Description…" entry on the effect context
    menu. Bridge stores the note at `X_Effect_Description` (the
    X_ prefix survives `SetSettings` so Randomise / Reset /
    preset-apply don't clobber it). Description appears inline
    in `SelectionReadout` in smart quotes after the timing data.
  - **B82**: "Add Timing Tracks to All Views" entry in the
    View-picker. Bridge iterates visible timing elements and
    calls `SequenceElements::AddTimingToAllViews` on each.
- **Fifth P2 polish batch (2026-04-22)** — B13 / B83.
  - **B13**: "Extend to Previous Effect" / "Extend to Next Effect"
    entries in Edit › Nudge Selection — grow the selected effect
    to butt its neighbour (or sequence bounds). ⇧⌘← / ⇧⌘→.
  - **B83**: "Create Timing from Selected Effect's Model" in the
    View-picker menu. Walks every row in the selected effect's
    element tree (sub-layers + submodels + strands + nodes),
    de-duplicates identical `[start, end]` ranges, and drops a
    new variable timing track with one mark per distinct range.
- **Fourth P2 polish batch (2026-04-22)** — B47 / B91 / B66.
  - **B47**: "Insert Multiple Layers Below…" on the model row
    menu. Alert prompts for a count; bridge `insertEffectLayersBelow(at:count:)`
    batches all inserts + a single `PopulateRowInformation`.
  - **B91**: "Halve Timing Marks" on the timing-track menu. Walks
    the row's marks last-to-first and splits each at its
    midpoint; one undo group covers the lot. Marks shorter than
    2 ms are skipped so `splitTimingMark`'s strict-between guard
    never fires.
  - **B66**: Render-disabled (a.k.a. "muted") model rows now
    render their row-heading at 45 % opacity so users can scan
    for which models won't contribute to the current render pass.
- **Third P2 polish batch (2026-04-22)** — B48 / B90 / B92.
  - **B48**: "Delete N Unused Layers" entry on the model row
    header, visible only when the element has ≥ 1 empty layer and
    > 1 layer total. Label reports the pending count. Bridge
    `deleteUnusedLayers(onElementAt:)` walks the layer list top-
    down so removals don't invalidate indices.
  - **B90**: Add / Remove "-shimmer" suffix entry on the timing-
    mark long-press menu. Flips the label between "Add" / "Remove"
    based on current state; routes through `renameTimingMark` so
    the undo path is shared.
  - **B92**: Double-tap a timing mark to auto-set the loop region
    to its `[startMS, endMS]`, enable loop-play, seek to the
    start, and begin playback — one-tap "isolate this word /
    phoneme". Added `onDoubleTapMark` callback on the timing Metal
    view; double-tap gesture is sequenced via `require(toFail:)`
    so single-tap seek still works.
- **Second P2 polish batch (2026-04-22)** — B6 / B17 / B42.
  - **B6**: Ctrl+PageUp / Ctrl+PageDown snap the selected effect's
    start to the previous / next active-timing-mark position
    (preserving duration). `activeTimingMarkTimes()` helper
    surfaced on the view model so other code can reuse it.
  - **B17**: Dice-icon random-effect button on the palette. Picks
    a random entry from `availableEffects` and arms it for
    tap-to-place.
  - **B42**: Double-height waveform toggle in the View-picker.
    Swaps the `GridMetrics.waveformHeight` between 48 and 96 pt;
    persisted via `@AppStorage("waveformDoubleHeight")`.
- **Small P2 polish batch (2026-04-22)** — B3 / B38 / B81 / B22.
  - **B3**: Tab → Next Effect / Shift+Tab → Previous Effect added
    alongside the arrow-key bindings in the Playback menu.
  - **B38**: Zoom-To… submenu with 8 geometric preset stops
    (10 min through 0.5 s fit) under View › Zoom. Complements the
    existing ⌘+ / ⌘- step-zoom.
  - **B81**: Hide / Show All Timing Tracks toggle in the View
    picker. Bridge wraps `SequenceElements::HideAllTimingTracks`
    and re-populates row info; the menu label flips based on
    current state.
  - **B22**: Per-effect Reset — already covered by B15's single-
    select path ("Reset to Defaults" in the effect context menu).
    Verified + closed.
- **Effect presets, session-only (2026-04-22)** — B19. Save-as /
  apply-preset entries on the effect context menu. Presets live
  as an in-memory `[EffectPreset]` on the view model — they
  capture effect type + settings + palette. Applying to a
  different-type target rebuilds the effect (delete + add) at the
  same time range; same-type target is a wholesale settings +
  palette replace. Multi-select apply works the same way with
  independent inserts per target. Persistence is Phase C work
  (`EffectPresetManager` + disk store); the menu entries are
  here so users have a place to save within a session.
- **Randomise / Reset selected effects (2026-04-22)** — B15.
  Effect context menu now has "Randomise Settings" /
  "Reset to Defaults" entries (single + multi-select). Desktop's
  implementation introspects live `wxSlider` / `wxCheckBox` /
  `wxChoice` controls; iPad instead walks `EffectMetadata.json`
  properties and picks random values per-control-type — sliders /
  spinners get a random int in `[min, max]`, checkboxes flip
  50/50, choices pick a random `options` entry. Shared panels
  (buffer `B_*`, blending/fade `T_*`) and palette (`C_*` +
  `xlColor`) are preserved so Fade, blend mode, and the colour
  set survive. Each effect in a multi-selection gets independent
  random values. New bridge `replaceEffectSettings(_:palette:inRow:atIndex:)`
  wraps `Effect::SetSettings` / `SetPalette` so the write + undo
  restore happen in one atomic call.
- **Visible scrollbars (2026-04-22)** — B94. New
  `ScrollbarOverlay` SwiftUI view draws a thin capsule track +
  draggable thumb overlaid on the model-effects Metal canvas.
  Horizontal bar spans the bottom edge bound to
  `TimelineState.hScrollOffsetPx`; vertical bar pins to the
  trailing edge bound to `RowsScrollState.vScrollOffsetPx`. Thumb
  size is proportional to viewport/content ratio with a 24 pt
  minimum; auto-fades 1.5 s after each scroll change; dragging
  the thumb repositions the corresponding offset proportionally.
  Only the thumb consumes hits — the track is hit-transparent so
  taps in the margin still reach the Metal grid beneath.
- **Export model as FSEQ (2026-04-22)** — B49 (first cut).
  Model row-heading menu now has "Export Model as FSEQ…" and
  "Export Model (Loop Range) as FSEQ…" entries. Bridge
  `XLSequenceDocument.exportModelAsFSEQ(atRow:toPath:startMS:endMS:)`
  wraps `RenderEngine::ExportModelData` (wx-free) and writes a v2
  zstd-compressed sub-sequence via the shared `FSEQFile` writer
  already in `src-core/render/`. SwiftUI `.fileExporter` handles
  the destination picker; temp file is written first then copied.
  Only the Falcon Player format ships for iPad v1 — desktop's LCB
  / Vixen / HLS / MP4 / GIF / Minleon variants can follow in a
  polish sweep; `.eseq` is the most-used format in practice.
- **Numbered tags (2026-04-21)** — B34 + B35. `_tagPositions[10]`
  bridged through `XLSequenceDocument` (`tagPosition(at:)` /
  `setTagPosition(at:positionMS:)` / `clearAllTags`); persistence
  comes free via `<TimingTags>` XML that `SequenceFile` already
  writes + reads. `SequencerViewModel.tagPositions` is the
  observable state with `setTag` / `goToTag` / `clearTag` /
  `clearAllTags` actions. `TopChromeMetalGridView` renders one
  coloured flag per tag on the ruler, hue-coded on a 10-step wheel
  with an inline numeric label. Menu-bar: Playback ▸ Set Tag at
  Play Head ▸ Tag N (⌃⇧N) and Go To Tag ▸ Tag N (⌃N) for all ten;
  Clear All Tags at the bottom. View-picker gets the same three
  entries as submenus for touch users.

- **Apple Pencil support (2026-04-22)** —
  `UIPencilInteraction` on the effect grid with three delegate
  methods covering both the legacy 12.1+ API
  (`pencilInteractionDidTap`) and the iOS 17.5+ unified APIs
  (`pencilInteraction(_:didReceiveTap:)` + `didReceiveSqueeze:`).
  Squeeze's `.began` / `.ended` phases are tracked separately so
  the action only fires on full squeeze-release.
  - **Plain squeeze / double-tap → `undo()`.**
  - **Squeeze + edge resize drag → shared-edge mode.** When the
    Pencil Pro squeeze is held and the user drags the left or
    right edge of an effect that's butted against a neighbour
    (zero-gap), the shared boundary moves for *both* effects
    simultaneously — one grows, the other shrinks. Live render
    shows both sizes animating. Commit routes through new
    `SequencerViewModel.resizeSharedEdge(rowIndex:leftIndex:...)`
    which orders the two `moveEffect` calls so the shrinking
    side clears space before the growing side advances, and
    both land in one undo group. The squeeze's
    `pencilSqueezeConsumedByDrag` flag suppresses the plain
    squeeze→undo action when the squeeze was used for a drag.
  Pencil hover uses the existing B30 highlight so edge handles
  light up before the tap. Hit slop kept at the finger-friendly
  24 pt default — narrowing to 12 pt for "precision" turned out
  narrower than real-world Pencil jitter + reach-and-tap wobble
  and broke edge reachability. `touchesBegan` still tracks
  `pencilActive` as a hook for future refinements.

### What's still open

- **Scattered P1s**: B15 randomize / reset (needs metadata-default
  plumbing), B19 effect-presets menu stub (Phase C storage), B49
  export-model (file I/O), B94 visible scrollbars (custom Metal
  overlay).
- **P2 polish** — ~40 items across editing, visual, row-heading,
  timing, scroll, find, clipboard.

**Out of Phase B:** B16 (drag-from-palette with ghost) deferred
pending user feedback; tap-to-arm + tap-to-place is working well
in practice. B26 (ColorCurve gradient) awaits a device-side verify
— the `DrawEffectBackground` path is already wired so this may
already work. B86 (Breakdown Phoneme) removed; not a real desktop
feature.

---

## 1. Effects grid canvas

### 1.1 Selection

- ~~**Gap B3** — Tab-to-next-effect. **✓ landed 2026-04-22.**~~

### 1.2 Editing — positional

- ~~**Gap B6** — Nudge by timing mark. **✓ landed 2026-04-22.**~~
- ~~**Gap B7** — Edge-unlink indicator + unlink command. **✓ closed —
  no desktop counterpart.** Searched `Effect`, `EffectLayer`,
  `EffectsGrid`, the grid menu IDs, and serialisation: there's no
  per-edge "unlinked" tag on desktop. The Effect class exposes
  `mProtected` / `mLocked` / `isRenderDisabled` but those are
  whole-effect state, not per-edge. Snap-to-neighbour during
  drag is implicit and has no per-edge override. If a future
  workflow needs this, it's a new feature for both clients —
  not a parity gap.~~

### 1.3 Editing — range / bulk

- ~~**Gap B13** — Extend to next / previous. **✓ landed
  2026-04-22** as ⇧⌘←/→.~~
- ~~**Gap B15** — Randomize selected / Reset to default. **✓
  landed 2026-04-22** via metadata-driven randomizer.~~

### 1.4 Create / drop

- ~~**Gap B17** — Random effect palette button. **✓ landed
  2026-04-22** as the dice-icon button.~~
- ~~**Gap B18** — Double-click-to-create in cell. **✓ landed
  2026-04-22** — double-tap empty row (palette armed).~~

### 1.5 Context menu entries

- ~~**Gap B19** — Effect Presets submenu. **✓ landed 2026-04-22**
  as session-only Save / Apply. Persistence = Phase C.~~
- ~~**Gap B20** — Description field. **✓ landed 2026-04-22** as
  `X_Effect_Description` + context-menu editor.~~
- ~~**Gap B22** — Reset effect (single-effect variant of B15).
  **✓ already covered** — B15's "Reset to Defaults" entry handles
  single-select.~~
- ~~**Gap B23** — Duplicate across models. **✓ closed — no desktop
  counterpart, equivalent already covered.** Audit found desktop
  has `DuplicateSelectedEffects` (duplicates across *time* in the
  same row, not across models) and `CopyModelEffects` (whole-row
  copy/paste); neither is "Copy Settings To N Models" as the
  followup described. The iPad's existing multi-effect clipboard
  (B98) + paste-replace (B100) already covers the user-facing
  workflow: copy the effect, multi-select destination rows, paste.~~
- **Gap B24 — Find Possible Source Effects.** Audited 2026-04-28:
  desktop's implementation in `EffectsGrid::FindEffectsForData`
  (~80 lines) walks every element + strand + node + submodel +
  data layer and uses `Model::ContainsChannel` + `FindChannel` to
  resolve the channel range. Results are presented in a dedicated
  `FindDataPanel` (separate wx panel under `src-ui-wx/diagnostics/`).
  Porting needs a new SwiftUI panel + bridge for channel resolution
  per element / strand / node / submodel — substantially bigger
  than other P2 polish items. **Deferred — P2.** Diagnostic-only;
  most users never invoke it.

### 1.6 Visual polish

- **Gap B25 — Bracket colours sourced from `ColorManager`.** iPad
  hardcodes bracket RGB (`EffectsMetalGridView.swift:292–295`);
  desktop sources from `ColorManager::COLOR_EFFECT_DEFAULT /
  _SELECTED / _LOCKED / _DISABLED` so user-customised palettes
  round-trip. Route iPad through the existing bridge. **P2.**
- ~~**Gap B27** — Node-level colour-channel stripes. **✓ already
  covered.** Misread of desktop behaviour: there's no separate
  "per-channel stripes" path — desktop's per-node colour handling
  for ChannelBlock and Single Color string-type models lives in
  `EffectsGrid::DrawEffectBackground` via `colorMask`
  (`EffectsGrid.cpp:6587–6603`). iPad mirrors this exact code at
  `XLSequenceDocument.mm:3850–3872`, so the effect background
  already paints with the node mask for these model types. The
  unrelated `mGridNodeValues` per-frame node-value preview is a
  separate user toggle (not B27) — could land later if anyone
  asks.~~
- ~~**Gap B28** — Previous-selection indicator. **✓ landed
  2026-04-22**.~~
- ~~**Gap B29** — Text fade / size-stepping. **✓ landed
  2026-04-22**.~~

---

## 2. Timeline (ruler)

- ~~**Gap B38** — Zoom-level presets. **✓ landed 2026-04-22** as
  an 8-stop Zoom-To… submenu.~~
- ~~**Gap B40** — Audio scrub. **✓ landed 2026-04-28.** New
  `audioPlaySegmentFromMS:lengthMS:` bridge calls
  `AudioManager::Play(pos, len)` for a short ~50 ms window and
  refuses to fire when normal playback is active. View model's
  new `scrubSeekTo(ms:)` runs that play burst on each ruler-drag
  tick (throttled to 50 ms) plus a regular `audioSeek` so the
  playback engine's cursor stays aligned for the next tick or the
  drag-end commit. `TopChromeMetalGridView` now exposes an
  optional `onScrubSeek` callback wired from `SequencerGridV2View`
  so non-scrub seeks (rewind / shortcuts) still go through the
  silent `seekTo(ms:)` path.~~

---

## 3. Waveform

- ~~**Gap B42** — Double-height waveform. **✓ landed 2026-04-22**
  as a View-menu toggle.~~
- ~~**Gap B43** — Audio track switching. **✓ landed 2026-04-28.**
  iPadRenderContext caches a `_waveformTrackIndex` (-1 = main,
  0..N-1 = alt) and exposes `GetWaveformMedia` which the bridge's
  `waveformDataFromMS:…` now uses instead of
  `GetCurrentMediaManager`. New bridge methods `altTrackCount`,
  `altTrackDisplayName(at:)`, `activeWaveformTrack`,
  `setActiveWaveformTrack:`. View model's `reloadAltTracks()` runs
  on every sequence open; `setActiveWaveformTrack(_:)` switches +
  reloads the waveform peaks. Waveform long-press menu surfaces
  "Main Audio" + each alt track when `altTrackCount > 0`.
  Playback is unaffected (deliberately) — only the displayed
  waveform changes.~~

---

## 4. Row headings

### 4.1 Layer management

- ~~**Gap B47** — Insert Multiple Layers Below. **✓ landed
  2026-04-22**.~~
- ~~**Gap B48** — Delete Unused Layers. **✓ landed 2026-04-22**
  on the model row header menu.~~

### 4.2 Model / row operations

- ~~**Gap B49** — Export model sequence / Render-and-Export. **✓
  landed 2026-04-22** as a single `.eseq` export path (whole
  sequence + loop-range variants). Extra formats (LCB / Vixen /
  HLS / MP4 / GIF / Minleon) can roll in as a polish item.~~
- ~~**Gap B55** — Convert Effects to 'Per Model'. **✓ landed
  2026-04-28.** New `convertEffectsToPerModelOnRow:acrossAllLayers:`
  bridge calls the existing core `EffectLayer::ConvertEffectsToPerModel`
  (already wx-free); model-scope variant on the model heading +
  layer-scope variant on per-layer rows mirror desktop's two
  RowHeading entries. Triggers a model re-render on success.~~
- **Gap B56 — Promote Node Effects / Convert To Effect.**
  Promote: ✓ landed 2026-04-28 — `promoteNodeEffectsOnRow:` ports
  the `xLightsFrame::DoPromoteEffects` algorithm (pure structural
  walk, no SequenceData), surfaced as "Promote Node Effects" on
  the model heading. Convert To Effect: deferred — desktop's
  `DoConvertDataRowToEffects` walks rendered SequenceData to
  detect colour ramps via `RampLenColor` / `isOnLineColor` helpers
  that currently live in `tabSequencer.cpp`. Porting needs those
  helpers lifted to core first, plus per-strand `SingleLineModel`
  construction. **Deferred — P2.**

### 4.3 Global row operations

- ~~**Gap B58** — Toggle Strands / Nodes / Models. **✓ already
  covered** by per-element Show/Hide entries (desktop is also
  per-element; B58's "global" framing was mis-scoped).~~
- **Gap B59 — Edit Display Elements.** Phase F scope.

### 4.4 Drag / resize

- ~~**Gap B60** — Drag row to reorder. **✓ already covered.**
  Audited 2026-04-28: desktop's `RowHeading` doesn't actually
  reorder via heading-drag — the `mouseLeftDown` handler covers
  select / collapse-expand / timing-active toggle and a column-
  resize drag on the right edge, but reordering goes through the
  separate Display Elements / ViewsModelsPanel dialogs. iPad's
  Phase F-6 Display Elements sheet already exposes `.onMove`-based
  reordering for the active view's members (`DisplayElementsSheet.swift:479`),
  so the user-facing capability exists. A direct row-heading drag
  on the sequencer would be iPad-only polish; not a parity gap.~~

### 4.5 Icons / visual

- ~~**Gap B63** — Element-type icon glyphs. **✓ landed
  2026-04-22** for lyric-breakdown timing layers (Phrases /
  Words / Phonemes) via SF Symbols. ModelGroup folder icon was
  already in place; FPP / Papagayo glyphs are deferred since
  those element types don't yet have a bridge-visible
  classifier on iPad — a follow-up polish item if ever needed.~~
- ~~**Gap B66** — Muted visual state. **✓ landed 2026-04-22** as
  row-heading 45 % opacity when render is disabled.~~

---

## 5. Timing tracks (remaining)

### 5.1 Track management

- **Gap B77 — Import Notes** (MIDI / note file). **P2.**
- **Gap B79 — AI Speech 2 Lyrics.** Needs an iOS bridge for the
  AI call path. **P2.**
- ~~**Gap B81** — Hide / Show All Timing bulk toggle. **✓ landed
  2026-04-22** in the View-picker menu.~~
- ~~**Gap B82** — Add Timing Tracks to All Views. **✓ landed
  2026-04-22** in the View-picker menu.~~
- ~~**Gap B83** — Create Timing From Effects. **✓ landed
  2026-04-22** in the View-picker menu.~~

### 5.2 Per-mark / misc

- ~~**Gap B84 (per-mark variant)** — single-mark "Breakdown
  Phrase". **✓ landed 2026-04-28.** New
  `breakdownPhraseAtRow:atIndex:` bridge method tokenises just the
  target phrase, wipes any existing words/phonemes inside its
  `[startMS, endMS]` window via `EffectLayer::GetAllEffectsByTime`
  + `DeleteEffect`, then writes fresh per-word marks. Reuses the
  same delimiter set + interval math as the row-level path so the
  two yield identical word boundaries on the same input. New
  "Breakdown This Phrase" entry on the timing-mark long-press
  menu, gated by `canBreakdownPhrase(rowIndex:markIndex:)` (must
  be a labelled mark on the phrase layer).~~
- ~~**Gap B90** — Add / Remove "-shimmer" suffix. **✓ landed
  2026-04-22** on the timing-mark long-press menu.~~
- ~~**Gap B91** — Divide Timings (Halve). **✓ landed 2026-04-22**
  on the timing-row menu.~~
- ~~**Gap B92** — Double-tap timing mark → loop-play that region.
  **✓ landed 2026-04-22**.~~

---

## 6. Scrolling, zoom, playback follow

- ~~**Gap B94** — Visible scrollbars. **✓ landed 2026-04-22** as
  `ScrollbarOverlay` with auto-hide + drag-to-scroll.~~
- ~~**Gap B96** — Scroll momentum. **✓ landed 2026-04-22** via a
  `CADisplayLink` coast with exponential decay.~~

---

## 7. Find / Replace

- ~~**Gap B97** — Find / Replace panel. **✓ landed 2026-04-28.**
  New `FindReplaceSheet` (bottom-sheet via `.presentationDetents`)
  bound to view-model state (`findText`, `findCaseSensitive`,
  `findResults`, `currentFindIndex`, `findReplacePresented`).
  Search runs against timing-mark labels only — matches desktop's
  `EffectsGrid::Find` / `FindNext` / `Replace` restriction (and is
  the only place mark labels live anyway). Buttons: Previous / Next
  (wrap), Replace Current, Replace All. ⌘F shortcut wired through
  `XLightsCommands` (CommandGroup after .pasteboard). Replace path
  routes through the existing `setTimingMarkLabel` bridge.~~

---

## 8. Copy / paste beyond single effect

- ~~**Gap B99** — System pasteboard integration. **✓ landed
  2026-04-22** with the `com.xlights.effectclip` UTI.~~
- ~~**Gap B100** — Paste-replace with confirmation. **✓ landed
  2026-04-22**.~~

---

## 9. Deferred / removed

- **Gap B16 — Drag from palette with live preview.** Deferred
  pending user feedback. Tap-to-arm + tap-to-place is working
  well on touch; revisit only if users ask for drag-cancel
  mid-gesture.
- **Gap B86 — Breakdown Phoneme.** Removed from scope. Not a real
  desktop feature (desktop has exactly Phrases + Words
  breakdowns; phonemes fall out of the Word breakdown).
- **Gap B59 — Edit Display Elements.** Phase F scope.
- **Gap B19 full-storage impl.** Phase C / G12 shares
  `EffectPresetManager`-backed storage; only the menu-entry stub
  lands here.

---

## 10. Suggested phasing for remaining work

**All P0 + P1 gaps are closed.** Remaining work is P2 polish —
best handled in a polish sweep rather than one-by-one. Candidates
for batching:

- Effect-bar visual polish: B25 (ColorManager bracket colours) +
  B29 (text fade / size stepping) + B27 (node-level channel
  stripes) + B28 (previous-selection indicator).
- Row-heading polish: B47 (Insert Multiple Layers Below) + B48
  (Delete Unused Layers) + B58 (Strand / Node / Model toggles) +
  B60 (drag to reorder) + B63 (Papagayo / FPP icon glyphs) + B66
  (muted state).
- Timing-track polish: B77 (Import Notes / MIDI) + B79 (AI Speech
  2 Lyrics) + B81 (Hide / Show All) + B82 (Add to All Views) +
  B83 (Create From Effects) + B84-per-mark (single-mark
  breakdown) + B90 (-shimmer) + B91 (Divide Timings) + B92
  (double-tap to loop-play mark region).
- Selection / editing polish: B3 (Tab / Shift+Tab navigation) +
  B6 (nudge by timing mark) + B7 (edge-unlink indicator) + B13
  (extend to next / previous).
- Palette / create polish: B17 (random-effect button) + B18
  (double-click create in range).
- Context-menu polish: B20 (description field) + B22 (per-effect
  reset) + B23 (duplicate across models) + B24 (find source
  effects).
- Timeline / waveform polish: B38 (zoom-level presets) + B40
  (audio scrub) + B42 (double-height waveform) + B43 (alt-track
  switch).
- Row-ops polish: B55 (convert to per-model) + B56 (promote
  node / convert-to-effect).
- Scroll polish: B96 (scroll momentum).
- Search / clipboard: B97 (Find / Replace panel) + B99
  (UIPasteboard) + B100 (paste-replace at same cell).

Phase C / G12 work (effect-preset persistence, custom palette
manager) and Phase F (Display Elements editor) remain on their
own tracks.

---

## 11. Out of scope

- Core rendering (complete, verified on-device).
- Model / layout editing — stays desktop-only.
- Controller output — deferred to post-MVP in the top-level plan.
- Sequence-lifecycle plumbing — Phase E (complete).
- App Store submission — Phase H.
- Document / iCloud handling — Phase G.
- Audio-filter enhancements beyond the four standard filters —
  tracked in `plans/audio-analysis-enhancements.md`.

---

## 12. Summary gap table (open items only)

Severity key: **P1** = blocks a specialised workflow; **P2** =
nice-to-have / expert shortcut; **Verify** = likely already works,
needs device check; **Deferred** = pending upstream work or user
feedback.

| # | Gap | Area | Severity |
|---|---|---|---|
| B3 | Tab / Shift+Tab navigation | Selection | ✓ landed |
| B6 | Nudge by timing mark | Editing | ✓ landed |
| B7 | Edge-unlink indicator + command | Editing | ✓ no desktop counterpart |
| B13 | Extend effect to next / previous | Editing | ✓ landed |
| B15 | Randomize / Reset-to-default on selection | Editing | ✓ landed |
| B16 | Drag-from-palette with preview ghost | Create | Deferred |
| B17 | Random-effect palette button | Create | ✓ landed |
| B18 | Double-click create in range | Create | ✓ landed |
| B19 | Effect presets menu entry | Ctx menu | ✓ landed |
| B20 | Description / tooltip field | Ctx menu | ✓ landed |
| B22 | Reset effect to defaults | Ctx menu | ✓ landed |
| B23 | Duplicate across models | Ctx menu | ✓ no desktop counterpart |
| B24 | Find possible source effects | Ctx menu | Deferred |
| B25 | Bracket colours sourced from `ColorManager` | Visual | ✓ landed |
| B26 | ColorCurve gradient preview in effect bar | Visual | ✓ landed |
| B27 | Node-level channel stripes | Visual | ✓ already covered |
| B28 | Reference / previous-selection indicator | Visual | ✓ landed |
| B29 | Text fade / size stepping | Visual | ✓ landed |
| B34 | Tags (0-9 numbered markers) | Timeline | ✓ landed |
| B35 | Tag context menu | Timeline | ✓ landed |
| B38 | Desktop zoom-level presets | Timeline | ✓ landed |
| B40 | Audio scrub | Timeline | ✓ landed |
| B42 | Double-height waveform | Waveform | ✓ landed |
| B43 | Audio-track switch (alt tracks) | Waveform | ✓ landed |
| B47 | Insert Multiple Layers Below | Row heading | ✓ landed |
| B48 | Delete Unused Layers | Row heading | ✓ landed |
| B49 | Export model / Render-and-Export | Row heading | ✓ landed |
| B55 | Convert Effects to 'Per Model' | Row heading | ✓ landed |
| B56 | Promote Node Effects | Row heading | ✓ landed |
| B56 | Convert To Effect (data → effects) | Row heading | Deferred |
| B58 | Toggle Strands / Nodes / Models | Row heading | ✓ landed |
| B60 | Drag row to reorder | Row heading | ✓ already covered |
| B63 | Element-type icon glyphs | Row heading | ✓ landed |
| B64 | Layer-count [N] indicator | Row heading | ✓ landed |
| B66 | Muted row visual state | Row heading | ✓ landed |
| B77 | Import Notes (MIDI) | Timing | P2 |
| B79 | AI Speech 2 Lyrics | Timing | P2 |
| B81 | Hide All / Show All Timing | Timing | ✓ landed |
| B82 | Add Timing Tracks to All Views | Timing | ✓ landed |
| B83 | Create Timing From Effects | Timing | ✓ landed |
| B84 (per-mark) | Single-mark phrase breakdown | Timing | ✓ landed |
| B86 | Breakdown Phoneme | Timing | Removed |
| B90 | Add / Remove "-shimmer" suffix | Timing | ✓ landed |
| B91 | Divide Timings (Halve) | Timing | ✓ landed |
| B92 | Double-tap timing mark → loop-play region | Timing | ✓ landed |
| B94 | Visible scrollbars | Scroll | ✓ landed |
| B96 | Scroll momentum | Scroll | ✓ landed |
| B97 | Find / Replace | Find | ✓ landed |
| B99 | System pasteboard (UIPasteboard) integration | Clipboard | ✓ landed |
| B100 | Paste-replacing-existing with confirmation | Clipboard | ✓ landed |

Counts (2026-04-28, post-batch-4): **0 × P0**, **0 × P1**,
**2 × P2** (B77 MIDI import, B79 AI Speech 2 Lyrics), **0 × Verify**,
**3 × Deferred** (B16 drag-from-palette ghost, B24 find-source-effects,
B56 convert-data-to-effects), **6 × Closed without code (already
covered or no desktop counterpart)**.
Phase B P0 + P1 gap audit is now complete — remaining work is
P2 polish only.
