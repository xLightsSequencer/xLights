# Phase B — Effects Grid Parity (Pending Work)

Scope: the sequencer canvas the user spends 90 % of their time in —
the **effects grid**, **timeline ruler**, **waveform**, **row
headings**, and the **timing tracks** strip between them. Phase B and
B-Metal closed out the rebuild and the Metal render pipeline; this
sub-plan is what *wasn't* included in either and still keeps the iPad
app well short of desktop parity.

Desktop surface (lines of code, for scale):

| Component | File | LOC |
|---|---|---:|
| Grid canvas | `src-ui-wx/sequencer/EffectsGrid.cpp` | 7606 |
| Row headings | `src-ui-wx/sequencer/RowHeading.cpp` | 2228 |
| Sequencer shell | `src-ui-wx/sequencer/MainSequencer.cpp` | 2276 |
| Timeline | `src-ui-wx/sequencer/TimeLine.cpp` | 1189 |
| Waveform | `src-ui-wx/sequencer/Waveform.cpp` | 797 |
| **Total** | | **~14 000** |

iPad surface:

| Component | File | LOC |
|---|---|---:|
| Effects grid (Metal) | `src-iPad/App/EffectsMetalGridView.swift` | 1045 |
| Sequencer shell | `src-iPad/App/SequencerViewModel.swift` | 1413 |
| Grid container | `src-iPad/App/SequencerGridV2View.swift` | 553 |
| Row headers | `src-iPad/App/RowHeaderViews.swift` | 343 |
| Top chrome (ruler + waveform) | `src-iPad/App/TopChromeMetalGridView.swift` | 268 |
| Timing grid | `src-iPad/App/TimingEffectsMetalGridView.swift` | 226 |
| Sequencer view | `src-iPad/App/SequencerView.swift` | 313 |
| Timeline state | `src-iPad/App/TimelineState.swift` | 21 |
| **Total** | | **~4 200** |

The ~10 000-line gap is not all missing features (wxWidgets is
verbose; Metal rendering is compact) but most of it is. What follows
is organised by surface, then severity.

---

## TL;DR

The iPad grid renders and handles the *basics*:

- Effects draw, snap to time, show icons + labels + fade bars.
- Single-effect tap / drag / resize / fade-edit / cross-row move all work.
- Lock / disable / delete / copy / paste / duplicate via long-press menu.
- Pinch zoom, pan scroll, play marker, tap-to-seek, ruler, waveform.
- Row headers for model / submodel / strand / node / timing rows with
  expand / collapse, and insert-layer / delete-layer on long-press.
- Arrow-key navigation, undo / redo, Cmd+Z / Cmd+Shift+Z, Cmd+C /
  Cmd+V / Cmd+D, ,/. frame-step, Home / End, Delete.

What's **conspicuously missing** — and where the user is going to
notice immediately:

1. **No multi-effect selection.** Single selection only. No marquee,
   no shift-click range, no select-all-in-row / column. Every
   bulk-edit primitive downstream of this (G11, G41 in Phase C,
   align, paste-range, bulk transform, bulk delete) has no reach.
2. **No timing-mark editing in the grid.** Timing rows are
   seek-only. Can't drag, can't create, can't split, can't breakdown
   phrases / words / phonemes, can't import / export timing, can't
   generate subdivisions. This is the whole lyrics / music
   workflow.
3. **Grid context menu is 6 items.** Desktop's is ~80 across the
   grid + row-heading menus. Whole categories — align, split,
   extend, presets, render / export, convert — have no iPad path.
4. **No follow-playhead.** Playback runs off-screen with no
   auto-scroll.
5. **No loop region, no tags.** Desktop has shift-click loop
   selection + Render-Selected-Region; 10 numbered tags
   (Ctrl+0-9); iPad has neither.
6. **Waveform is one fixed render.** No bass / alto / treble /
   non-vocals filters, no double-height mode, no audio-track
   switch.
7. **Row headings are thin.** No rename, no reorder, no column
   resize, no row-height resize, no model / row cut-copy-paste,
   no timing-track management menu (rename is the only one present).
8. **No drag-from-palette.** Palette effects are tap-to-arm /
   tap-to-place; no drag preview, no drop indicator.
9. **No Find / Replace, no effect presets, no duplicate-across-models,
   no "update all like this."**

None of these block playback — the renderer is shared core. All of
them block authorship on iPad as a first-class seat.

---

## 1. Effects grid canvas

Everything here is *inside* `EffectsMetalGridView.swift` /
`SequencerGridV2View.swift`. The rendering substrate is fine; what
follows is missing behaviour and missing chrome.

### 1.1 Selection

Desktop: single click, shift-click toggle, ctrl-click add/remove,
marquee / rubber-band rectangle, tab-to-next-effect,
`Select Effects in Row`, `Select Effects in Column`,
`Select Timing Marks`, range cursor for AC fill, auto-select
adjacent layer when clicking.

iPad: single tap.

- **Gap B1 — Multi-select.** Marquee (two-finger drag? long-press
  + drag? dedicated select-mode toggle?) returning a
  `Set<EffectSelection>` that the rest of the app reads. This is
  the blocker for most other gaps in the list. **P0.**
- **Gap B2 — Select all effects in row / column / timing mark.**
  Row-heading long-press entry *and* grid-context-menu entry.
  Trivial once B1 exists. **P1.**
- **Gap B3 — Tab-to-next-effect.** Keyboard hardware only; iPad
  arrow-keys already move selection row-ward and column-ward,
  but Tab / Shift+Tab isn't bound. **P2.**

### 1.2 Editing — positional

Desktop move / resize primitives all map cleanly to iPad; the gap is
the *range* and *snap* ops.

- **Gap B4 — Shift+arrow stretch, Ctrl+arrow fine nudge.** Arrow
  keys currently move selection *cursor*, not the effect's
  time. Desktop's modified-arrow semantics (Shift stretches,
  Ctrl fine-nudges by 1 ms, Alt by 1 frame) aren't implemented.
  **P1.**
- **Gap B5 — Snap-to-timing on resize / move.** The grid renders
  guide lines from the active timing track but dragging doesn't
  snap to them. Desktop snaps when within ~4 px.
  Tap-to-create already snaps to the active timing cell; this is
  the same primitive applied to the drag / resize path. **P1.**
- **Gap B6 — Nudge by timing mark.** Desktop Ctrl+PageUp /
  Ctrl+PageDown moves selection forward / back one timing mark.
  **P2.**
- **Gap B7 — Edge-unlink indicator + unlink command.** Desktop
  shows a visual tag on an effect edge that's been "unlinked"
  from the neighbour it used to touch (so paste / align won't
  re-butt them). Not rendered on iPad. **P2.**

### 1.3 Editing — range / bulk

All depend on B1.

- **Gap B8 — Align Start / End / Both / Centerpoints / Match
  Duration.** Five align ops in the desktop right-click menu. All
  trivial math; none exposed on iPad. **P0** (this is one of the
  top two things missing from the context menu).
- **Gap B9 — Shift-Align Start / End.** Slide selection to
  align without overlapping. **P1.**
- **Gap B10 — Align to Closest Timing Mark.** Snap selected
  effects' start / end to the nearest mark on the active timing
  track. **P1.**
- **Gap B11 — Close Gap.** Remove space between two selected
  effects. **P1.**
- **Gap B12 — Split.** Split selected effect at the play marker
  (or at the long-press location). Split-at-play-marker is a
  single-effect op, so it *does not* need B1. **P0.**
- **Gap B13 — Extend effect to next / previous.** Keyboard-only
  on desktop; handy. **P2.**
- **Gap B14 — Paste by cell.** Desktop paste respects the
  currently-selected grid cell (row + time range) and drops the
  clipboard effect into it. iPad paste targets the selected
  effect's row at play position. Different semantics — the grid
  has no "selected range" concept yet. **P1.**
- **Gap B15 — Randomize selected / Reset to default / Lock-all /
  Disable-all on selection.** All bulk-edit flavours that depend
  on B1. **P1.**

### 1.4 Create / drop

- **Gap B16 — Drag from palette with live preview.** Desktop
  drags the effect icon onto the grid with a ghost rectangle
  showing where it'll land. iPad arms a palette effect by tap
  and places it on the next empty-row tap. No preview ghost, no
  cancel-mid-drag. **P0** — this is the first-impression
  authoring interaction.
- **Gap B17 — Random effect.** Desktop has a "random effect"
  button in the `EffectIconPanel` that places a random type at
  the cursor. iPad palette is explicit-choose only. **P2.**
- **Gap B18 — Double-click-to-create in selected range.** Desktop
  double-clicks create an effect spanning the selected time
  range. iPad single-tap creates a 1 s (or active-timing-cell)
  effect. **P2** (power-user shortcut).

### 1.5 Context menu items

iPad long-press menu, full list: Copy · Paste Here · Lock/Unlock ·
Disable/Enable · Delete · Cancel. (6 items.)

Desktop's equivalent (effect right-click), missing on iPad:

- **Gap B19 — Effect Presets submenu** (save / load / apply
  named effect preset bundles). Full feature is Phase C / G12
  territory, but the grid entry-point menu item goes here. **P1.**
- **Gap B20 — Description field.** Free-text note stored on the
  effect. Rendered as tooltip. No iPad path. **P2.**
- **Gap B21 — Timing dialog.** Edit exact start / end ms.
  Essential for users who know the timecode they want. Useful
  even without B1. **P1.**
- **Gap B22 — Reset effect.** Revert the effect's settings to
  type defaults. **P2.**
- **Gap B23 — Duplicate across models.** Copy this effect onto
  every selected model. Desktop's `Copy Settings To N Models`.
  **P2.**
- **Gap B24 — Find Possible Source Effects.** Node-level search
  for effects that *could* have produced the data in the selected
  effect. Rare. **P2.**

Drag-up-row was verified to work but isn't in the long-press menu.
Surface a "Move to row…" action for discoverability? Noted, not
filed as a gap.

### 1.6 Visual polish

Things the iPad grid draws correctly: effect rectangles, brackets,
centerline, zebra rows, icons, labels (width-gated), lock glyph,
fade bars, yellow overlap, selection brackets, diamond fade handles,
timing guide lines, cross-row drag ghost, invalid-drop red tint,
drag pill label.

What desktop renders that iPad doesn't:

- **Gap B25 — Effect background palette colour.** Desktop's effect
  rectangle uses a per-effect-type colour (e.g. Spirals is purple,
  Fire is orange). iPad's rectangle is "light gray" for all
  unselected effects. Makes the grid look homogeneous from a
  distance. **P1.**
- **Gap B26 — Colour-curve / gradient preview inside effect bar.**
  If an effect's palette slot 1 carries a ColorCurve, desktop
  renders the gradient as the effect's fill so the user can see
  the time colour sweep at a glance. iPad: flat rectangle. **P1.**
- **Gap B27 — Node-level colour-channel stripes.** For node-level
  effects on multi-channel models (RGBW etc.), desktop paints
  thin stripes for each channel. **P2.**
- **Gap B28 — Reference effect / previous-selection indicator.**
  Desktop shows the previously-selected effect in a lighter shade
  so "this" vs "last" is visible. Useful during
  compare-and-adjust cycles. **P2.**
- **Gap B29 — Text fade / size-stepping at small widths.** iPad
  hides the label below a hard 70 px threshold; desktop
  progressively shrinks and fades. Cosmetic. **P2.**
- **Gap B30 — Hover / pointer-over states.** iPadOS 26 supports
  pointer (trackpad / Magic Keyboard) hover states via
  `.hoverEffect`. No hover rendering on the grid — no resize
  cursor, no edge-highlight before the drag begins. **P1.**
- **Gap B31 — Status bar.** Desktop updates the status bar with
  effect name / start / end / duration / row on selection. iPad's
  drag-pill shows this only *while* dragging. When selected-
  idle, no numeric readout anywhere. **P1.**

---

## 2. Timeline (ruler)

`TopChromeMetalGridView.swift` — covers both ruler and waveform in
one Metal strip.

- **Gap B32 — Loop region (shift-click start + end, drag to
  adjust).** Desktop draws a loop highlight and
  `Render Selected Region` runs against it. No iPad equivalent.
  **P0.**
- **Gap B33 — Play region from loop.** `Play Loop` / loop-repeat
  mode that plays between two markers indefinitely. **P1.**
- **Gap B34 — Tags (0-9 numbered markers).** Right-click-add,
  Ctrl+N-go-to, persisted with the sequence. Ten-tag budget
  matches the desktop. **P1.**
- **Gap B35 — Tag context menu** (rename, delete, delete-all).
  Rolls in with B34. **P1.**
- **Gap B36 — Zoom to selection.** Fits the selected range into
  the viewport. Depends on B1 for range. **P1.**
- **Gap B37 — Zoom to fit (sequence).** `fitIfNeeded` runs once
  on load; no button / shortcut to re-fit. **P1.**
- **Gap B38 — Zoom-level presets.** Desktop has 19 preset zoom
  levels (1 ms-per-tick through 40 s-per-tick). iPad smooth
  pinch only; no stepped zoom, no ⌘+/⌘- at-a-preset. Current
  toolbar `+` / `−` buttons apply a ×1.5 factor, which is fine
  as a cheap substitute but doesn't match desktop landmarks.
  **P2.**
- **Gap B39 — Drag-to-scrub playback.** iPad: tap-to-seek only.
  Desktop drags the play marker for continuous scrubbing
  (visual + optionally audio). **P1.**
- **Gap B40 — Audio scrub** (play a short window of audio as the
  marker is dragged). **P2.**

### Ruler visual

The ruler is functional. No tick-density complaints.

---

## 3. Waveform

Same file (`TopChromeMetalGridView.swift`).

- **Gap B41 — Filter-variant rendering** (bass / alto / treble /
  non-vocals / custom). Desktop's right-click waveform menu.
  Useful for timing-mark placement on vocal vs bass hits.
  `AudioManager` already produces these; the iPad waveform reads
  raw only. **P1.**
- **Gap B42 — Double-height mode.** Desktop toggle to stretch
  the waveform to 2× height for finer visual resolution.
  Straightforward given the waveform strip is already
  resizable in principle. **P2.**
- **Gap B43 — Audio track switching.** Desktop supports alt
  audio tracks (e.g. clean vocal stem vs full mix). iPad plays
  `main` only. **P2.**
- **Gap B44 — Render-selected-region.** Depends on B32
  (loop region). Right-click → Render Selected; kicks the
  renderer to re-run only the highlighted range. iPad has no
  equivalent because no loop. **P1.**
- **Gap B45 — Waveform click-to-seek / drag-for-range.** Tap
  works; drag-to-set-loop-region follows from B32.

---

## 4. Row headings

`RowHeaderViews.swift` (343 LOC). This is one of the biggest surface
gaps relative to desktop's `RowHeading.cpp` (2228 LOC).

### 4.1 Layer management

iPad long-press on a model row: Insert Layer Above · Insert Layer
Below · Delete Layer (if >1) · Show / Hide Strands/Submodels ·
Show / Hide Nodes.

Desktop context menu on a layer:

- **Gap B46 — Rename layer / Set layer name.** No iPad path.
  Layer names exist in the XML and are displayed but aren't
  editable from the iPad. **P1.**
- **Gap B47 — Insert Multiple Layers Below.** Batch create N
  layers; trivial. **P2.**
- **Gap B48 — Delete Unused Layers.** Scan the model's layers
  for ones with zero effects and drop them. **P2.**

### 4.2 Model / row operations

Desktop's nested `Model →` submenu. Missing on iPad:

- **Gap B49 — Export model sequence / Render-and-Export.** Save
  this model's effects as a standalone file. Desktop has 2 ×
  2 = 4 variants (whole model vs selected effects × with or
  without render). **P1.**
- **Gap B50 — Delete all effects / SubModel effects / Strand
  effects / Node effects.** Bulk clear scoped to the
  selected row type. **P1.**
- **Gap B51 — Enable / Disable Render on model.** Row-level
  render toggle. **P1.**
- **Gap B52 — "Select all effects in this model" /
  "Select all effects in row."** Multi-select + selection by
  row. Depends on B1. **P1.**
- **Gap B53 — Cut / Copy / Paste Row (whole layer + all
  effects).** Move a layer's worth of effects to another
  element. No iPad equivalent. **P1.**
- **Gap B54 — Cut / Copy / Paste Model (all layers + all
  effects).** Same at the model granularity. Desktop also has
  `Copy Model incl SubModels`. **P1.**
- **Gap B55 — Convert Effects to 'Per Model'.** Scope-change
  operation that collapses per-strand effects to a single
  model-level layer. **P2.**
- **Gap B56 — Promote Node Effects / Convert To Effect.** Move
  node-level effects up the hierarchy or convert a timing mark
  to an effect. **P2.**

### 4.3 Global row operations

- **Gap B57 — Show All Effects, Collapse All Models, Collapse
  All Layers.** Bulk visibility toggles. **P1.**
- **Gap B58 — Toggle Strands / Toggle Nodes / Toggle Models.**
  Row view-mode switch. iPad has per-row toggles only. **P2.**
- **Gap B59 — Edit Display Elements.** Opens the dialog that
  picks which elements are visible in the current sequence view.
  Phase F territory (Display Elements window). **Phase F** —
  not tracked here.

### 4.4 Drag / resize

- **Gap B60 — Drag row to reorder.** Reorder display of rows.
  Desktop supports, iPad does not. **P2.**
- **Gap B61 — Drag right edge of row-heading column to resize
  column width.** Row-heading column is fixed-width on iPad.
  **P1.**
- **Gap B62 — Per-row height adjustment.** Desktop grid has
  fixed 22 px row height, so this is parity — not a gap. iPad
  does have a "selected row is 36 px" behaviour which desktop
  doesn't. Parity-plus, not a gap.

### 4.5 Icons / visual

- **Gap B63 — Papagayo / FPP / model-group icon glyphs.**
  Desktop decorates row names with small icons when the element
  is a Papagayo lyric track, an FPP command / effect track, etc.
  iPad shows a folder for ModelGroup only. **P2.**
- **Gap B64 — Layer-count "[N]" indicator.** Desktop shows
  `Model Name [3]` when the element has 3 layers. iPad shows
  the layer toggle button but no count. Discoverability. **P2.**
- **Gap B65 — Tooltips on truncated row names.** iPad Row names
  truncate silently. Hover tooltip on Magic-Keyboard pointer
  would solve the discoverability problem with zero UI cost.
  **P1** for Magic Keyboard users; **P2** overall.
- **Gap B66 — Muted / hidden visual states.** Desktop has
  distinct rendering for a muted or hidden element; iPad shows
  collapse-only. Hidden-state rendering probably doesn't apply
  (hidden = not rendered at all), but muted (rendered darker /
  lower opacity) has no analog. **P2.**

---

## 5. Timing tracks

`TimingEffectsMetalGridView.swift` (226 LOC) — the big surface
gap. Timing tracks are **read-only** on iPad.

### 5.1 Mark creation and editing

All **P0** for a lyrics / music workflow.

- **Gap B67 — Tap to create timing mark** at play-position on
  the active track.
- **Gap B68 — Drag timing mark** edges to adjust start / end,
  with snap to adjacent marks.
- **Gap B69 — Delete timing mark** (long-press menu).
- **Gap B70 — Rename timing mark label** (double-tap → text
  field; desktop's Shift+double-click).
- **Gap B71 — Split timing mark at play position.**
- **Gap B72 — Combine / merge adjacent timing marks.**

### 5.2 Track management

Long-press on a timing-row header on iPad today: Rename Timing Track,
Delete Timing Track. That's it. Missing:

- **Gap B73 — Add Timing Track** (new variable timing track).
  Desktop has `NewTimingDialog`. **P0.**
- **Gap B74 — Import Timing Track** from `.xtiming`. **P1.**
- **Gap B75 — Export Timing Track** to `.xtiming`. **P1.**
- **Gap B76 — Make Timing Track Variable** (convert fixed →
  editable). **P1** — fixed tracks are common from
  beat-detection imports, so this is "unlock for editing."
- **Gap B77 — Import Notes** (MIDI / note file). **P2.**
- **Gap B78 — Import Lyrics** (text file). **P1.**
- **Gap B79 — AI Speech 2 Lyrics.** Desktop calls an AI service
  to turn audio into timed lyrics. Bigger piece — needs an iOS
  bridge for the AI call path. **P2.**
- **Gap B80 — Generate Subdivided Timing Tracks** (1/2, 1/3,
  1/4, 1/6, 1/8, ×2, ×4, ×8). Multi-select dialog on desktop.
  **P1** — common music-timing setup step.
- **Gap B81 — Hide All Timing / Show All Timing** bulk toggle.
  **P2.**
- **Gap B82 — Add Timing Tracks to All Views.** Replicate a
  timing track across every sequence view. Phase F adjacency.
  **P2.**
- **Gap B83 — Create Timing From Effects.** Generate timing
  marks from existing effects on a model. **P2.**

### 5.3 Phoneme / word / phrase breakdowns

These are how lyric videos get built. All **P0** for that workflow.

- **Gap B84 — Breakdown Phrase / Phrases.** Split the selected
  timing mark (or all timing marks) at phrase boundaries.
- **Gap B85 — Breakdown Word / Words.** Same at word granularity.
- **Gap B86 — Breakdown Phoneme.** Character-level splitting;
  drives the face / mouth-shape rendering.
- **Gap B87 — Remove Words / Phonemes / Words-and-Phonemes.**
  Bulk clear of sub-layer labels.
- **Gap B88 — Phoneme / word / phrase sub-layer rendering.**
  iPad renders a single-row timing track; desktop shows a
  stack of phrase-row / word-row / phoneme-row under it. This
  is a *rendering* gap in addition to the editing gaps —
  existing phoneme / word data in the XML is not visible on
  iPad. **P0.**
- **Gap B89 — Auto Label Timings.** Populate labels from the
  loaded lyric text. Follows from B78. **P1.**
- **Gap B90 — Add / Remove "-shimmer" suffix.** Desktop
  convenience operation on timing labels. **P2.**
- **Gap B91 — Divide Timings (Halve).** Subdivide each mark.
  **P2.**

### 5.4 Double-click to play looped effect

Desktop's "double-click a timing mark plays the effect on that
timing mark in a loop" — iPad single-taps seeks to the mark's start.
Partially present: the selection-scoped preview loop runs when an
*effect* is tapped. Timing-mark double-tap doesn't do this.

- **Gap B92 — Double-tap timing mark → loop-play that region.**
  **P2.**

---

## 6. Scrolling, zoom, playback follow

Most behaviour here works. The gaps:

- **Gap B93 — Follow-playhead during playback.** During play,
  the grid does not auto-scroll to keep the play marker in
  view. Desktop's behaviour: when the marker crosses the right
  edge, jump-scroll one viewport; configurable. **P0** —
  noticeable the first time you hit play on a long sequence.
- **Gap B94 — Visible scrollbars.** No scrollbars on iPad. Pan
  gesture is the only scroll. Fine on touch, rough with a
  trackpad. iPadOS 26 supports compact scrollbars via
  `ScrollView`; the Metal-backed grid needs a custom
  overlay. **P1.**
- **Gap B95 — Horizontal scroll via scroll-wheel / trackpad
  two-finger.** Pan gesture probably already catches this but
  not verified on-device with a trackpad. **P1** — verify,
  may already work.
- **Gap B96 — Scroll momentum.** iPad scroll stops when
  finger lifts. Small UX gap vs SwiftUI's stock ScrollView.
  **P2.**

---

## 7. Find / Replace

Desktop has Find / Find Next / Find Previous / Replace All across
effect names (case-insensitive).

- **Gap B97 — Find / Replace panel.** Bottom sheet or
  inspector-style overlay; cmd+F shortcut. Drives selection to
  matching effects, one at a time. **P2.**

---

## 8. Copy / paste beyond single effect

iPad clipboard holds one effect's name + settings + palette +
duration. It's app-internal, not the system pasteboard.

- **Gap B98 — Multi-effect clipboard** (array of effects
  preserving relative timing). Copy two effects 2 s apart,
  paste → two effects land 2 s apart at the target time.
  **P1.**
- **Gap B99 — System pasteboard integration.** Use
  `UIPasteboard` with a custom UTI so copy / paste can cross
  app restarts (and eventually cross-device via Universal
  Clipboard). **P2.**
- **Gap B100 — Paste replacing an existing effect at the
  same cell** (with confirmation). **P2.**

---

## 9. Summary gap table

Severity key:

- **P0** — blocks a common user workflow; the reason this
  plan exists.
- **P1** — blocks a specialised workflow; regular users hit this.
- **P2** — nice-to-have / parity items / expert shortcuts.

| # | Gap | Area | Severity |
|---|---|---|---|
| B1 | Multi-effect selection (marquee / range) | Selection | P0 |
| B2 | Select all in row / column / timing | Selection | P1 |
| B3 | Tab / Shift+Tab effect navigation | Selection | P2 |
| B4 | Shift / Ctrl arrow stretch / nudge | Editing | P1 |
| B5 | Snap-to-timing on resize + move | Editing | P1 |
| B6 | Nudge by timing mark | Editing | P2 |
| B7 | Edge-unlink indicator + command | Editing | P2 |
| B8 | Align Start / End / Both / Centers / Match | Editing | P0 |
| B9 | Shift-Align Start / End | Editing | P1 |
| B10 | Align to closest timing mark | Editing | P1 |
| B11 | Close Gap | Editing | P1 |
| B12 | Split at play marker | Editing | P0 |
| B13 | Extend effect to next / previous | Editing | P2 |
| B14 | Paste by cell | Editing | P1 |
| B15 | Randomize / Reset / Lock / Disable on selection | Editing | P1 |
| B16 | Drag-from-palette with preview ghost | Create | P0 |
| B17 | Random-effect palette button | Create | P2 |
| B18 | Double-click create in range | Create | P2 |
| B19 | Effect presets menu entry | Ctx menu | P1 |
| B20 | Description / tooltip field | Ctx menu | P2 |
| B21 | Timing dialog (exact ms) | Ctx menu | P1 |
| B22 | Reset effect to defaults | Ctx menu | P2 |
| B23 | Duplicate across models | Ctx menu | P2 |
| B24 | Find possible source effects | Ctx menu | P2 |
| B25 | Effect-type background palette colour | Visual | P1 |
| B26 | ColorCurve gradient preview in effect bar | Visual | P1 |
| B27 | Node-level channel stripes | Visual | P2 |
| B28 | Reference / previous-selection indicator | Visual | P2 |
| B29 | Text fade / size stepping | Visual | P2 |
| B30 | Pointer hover states | Visual | P1 |
| B31 | Status bar for selected effect | Visual | P1 |
| B32 | Loop region (shift-click + drag) | Timeline | P0 |
| B33 | Play-loop mode | Timeline | P1 |
| B34 | Tags (0-9 numbered markers) | Timeline | P1 |
| B35 | Tag context menu | Timeline | P1 |
| B36 | Zoom to selection | Timeline | P1 |
| B37 | Zoom to fit (button) | Timeline | P1 |
| B38 | Desktop zoom-level presets | Timeline | P2 |
| B39 | Drag to scrub | Timeline | P1 |
| B40 | Audio scrub | Timeline | P2 |
| B41 | Waveform filter variants (bass / alto / treble) | Waveform | P1 |
| B42 | Double-height waveform | Waveform | P2 |
| B43 | Audio-track switch (alt tracks) | Waveform | P2 |
| B44 | Render-selected-region | Waveform | P1 |
| B45 | Click-seek + drag-range on waveform | Waveform | P1 (with B32) |
| B46 | Rename layer | Row heading | P1 |
| B47 | Insert Multiple Layers Below | Row heading | P2 |
| B48 | Delete Unused Layers | Row heading | P2 |
| B49 | Export model / Render-and-Export | Row heading | P1 |
| B50 | Delete all effects / submodel / strand / node | Row heading | P1 |
| B51 | Enable / Disable render on model | Row heading | P1 |
| B52 | Select effects in model / row | Row heading | P1 |
| B53 | Cut / Copy / Paste Row | Row heading | P1 |
| B54 | Cut / Copy / Paste Model (+ submodels) | Row heading | P1 |
| B55 | Convert Effects to 'Per Model' | Row heading | P2 |
| B56 | Promote Node / Convert To Effect | Row heading | P2 |
| B57 | Show All / Collapse All Models / Collapse All Layers | Row heading | P1 |
| B58 | Toggle Strands / Nodes / Models | Row heading | P2 |
| B60 | Drag row to reorder | Row heading | P2 |
| B61 | Drag-resize row-heading column width | Row heading | P1 |
| B63 | Papagayo / FPP / group icon glyphs | Row heading | P2 |
| B64 | Layer-count [N] indicator | Row heading | P2 |
| B65 | Tooltip on truncated row name | Row heading | P2 |
| B66 | Muted / hidden row visual state | Row heading | P2 |
| B67 | Tap to create timing mark | Timing | P0 |
| B68 | Drag timing mark edges | Timing | P0 |
| B69 | Delete timing mark | Timing | P0 |
| B70 | Rename timing mark label | Timing | P0 |
| B71 | Split timing mark at play marker | Timing | P1 |
| B72 | Combine / merge timing marks | Timing | P1 |
| B73 | Add Timing Track | Timing | P0 |
| B74 | Import Timing Track (.xtiming) | Timing | P1 |
| B75 | Export Timing Track (.xtiming) | Timing | P1 |
| B76 | Make fixed timing track variable | Timing | P1 |
| B77 | Import Notes (MIDI) | Timing | P2 |
| B78 | Import Lyrics | Timing | P1 |
| B79 | AI Speech 2 Lyrics | Timing | P2 |
| B80 | Generate Subdivided Timing Tracks | Timing | P1 |
| B81 | Hide All / Show All Timing | Timing | P2 |
| B82 | Add Timing Tracks to All Views | Timing | P2 |
| B83 | Create Timing From Effects | Timing | P2 |
| B84 | Breakdown Phrase / Phrases | Timing | P0 |
| B85 | Breakdown Word / Words | Timing | P0 |
| B86 | Breakdown Phoneme | Timing | P0 |
| B87 | Remove Words / Phonemes | Timing | P1 |
| B88 | Phoneme / word / phrase sub-layer **rendering** | Timing | P0 |
| B89 | Auto Label Timings (from lyrics) | Timing | P1 |
| B90 | Add / Remove "-shimmer" suffix | Timing | P2 |
| B91 | Divide Timings (Halve) | Timing | P2 |
| B92 | Double-tap timing mark → loop-play region | Timing | P2 |
| B93 | Follow-playhead during playback | Scroll | P0 |
| B94 | Visible scrollbars | Scroll | P1 |
| B95 | Trackpad two-finger / wheel scroll | Scroll | P1 |
| B96 | Scroll momentum | Scroll | P2 |
| B97 | Find / Replace | Find | P2 |
| B98 | Multi-effect clipboard with relative timing | Clipboard | P1 |
| B99 | System pasteboard (UIPasteboard) integration | Clipboard | P2 |
| B100 | Paste-replacing-existing with confirmation | Clipboard | P2 |

Counts: 12 × P0, 35 × P1, 50 × P2.

---

## 10. Suggested phasing

The P0 list is long but clusters into four work items.

**B-I — Multi-select + align + split + playhead follow (2-3 weeks)**

The selection + align + split bundle (B1, B2, B8, B12, B14) + the
one behavioural miss that's visible every session (B93, follow-
playhead). One cohesive change to the grid's gesture model
(long-press-and-drag as marquee? two-finger-drag? selection mode
toggle?) unlocks eight P0/P1 items at once.

- B1 multi-select primitive.
- B2 select-in-row / -column / -timing via the row-heading and
  grid context menus (adds entries that are trivial once B1 lands).
- B8 + B9 + B10 + B11 align-family long-press menu.
- B12 split-at-marker single-effect long-press entry.
- B14 paste-by-cell semantics.
- B93 follow-playhead during play.

**B-II — Drag-from-palette + visual polish (1-2 weeks)**

- B16 drag-from-palette with ghost + drop-or-cancel.
- B25 effect-type background palette colour.
- B26 ColorCurve gradient preview inside effect bar.
- B30 pointer hover states (trackpad users).
- B31 status bar for selected effect.

**B-III — Timing tracks: editing + breakdowns (3-4 weeks)**

This is the biggest single block and the one that unblocks the
lyric-video workflow.

- B67 / B68 / B69 / B70 / B71 / B72 mark create / drag / delete
  / rename / split / merge.
- B73 add timing track.
- B76 fixed-to-variable conversion.
- B88 phoneme / word / phrase sub-layer rendering. **This is
  purely a rendering gap and can ship ahead of the editing
  work** — users whose sequences already have phoneme data
  would see it immediately.
- B84 / B85 / B86 breakdown phrase / word / phoneme.
- B78 import lyrics.
- B80 generate subdivided timing tracks.
- B74 / B75 import / export `.xtiming`.
- B87 / B89 bulk remove / auto-label.

**B-IV — Loop region, tags, waveform variants, render-selected
(1-2 weeks)**

- B32 / B33 / B44 / B45 loop region + play-loop + render-
  selected-region + waveform drag-range.
- B34 / B35 numbered tags + tag menu.
- B36 / B37 zoom-to-selection / zoom-to-fit.
- B39 drag-to-scrub.
- B41 waveform filter variants.

**B-V — Row-heading expansion (1-2 weeks)**

- B46 rename layer.
- B49 / B50 / B51 / B52 model-level export / bulk-delete /
  render-toggle / select-in-model.
- B53 / B54 cut-copy-paste row + model.
- B57 show-all / collapse-all.
- B61 drag-resize row-heading column.

**B-VI — Polish and parity sweep (1 week)**

The remaining P1s and any P2s that fell out of the above work.
Non-urgent. Examples: B4 shift-arrow stretch, B5 snap-to-timing,
B15 randomize-on-selection, B19 presets-menu-entry, B21 timing-
dialog, B94 visible scrollbars.

**Deferred to Phase F**

- B59 Edit Display Elements — Phase F already owns the
  Display Elements window.
- Menu-bar keyboard shortcut exposure.
- Tear-out / dock behaviour.

**Deferred to Phase C**

- B19 effect presets — share `EffectPresetManager`-backed
  storage with Phase C's G12.
- Long-press context-menu entries that also apply to the
  inspector tabs fold in with Phase C's G9.

---

## 11. Out of scope

- Core rendering (complete, verified on-device).
- Model / layout editing — stays desktop-only.
- Controller output — deferred to post-MVP in the top-level
  plan.
- Sequence-lifecycle plumbing (save / save-as / new / sequence
  settings / dirty prompt) — Phase E.
- App Store submission — Phase H.
- Document / iCloud handling — Phase G.

---

## 12. What this plan does **not** claim

This is a static analysis of source — `EffectsGrid.cpp` vs
`EffectsMetalGridView.swift` and friends. A few categories need
a **device-side pass** to confirm or rule out:

- Is every effect-type rectangle really flat grey, or does the
  palette colour flow through today and the gap is actually
  smaller than B25 says? (Skim `effectColor` lookups in the
  Metal bridge.)
- Does the trackpad two-finger scroll work in iPadOS 26 via the
  existing pan recogniser? (B95.)
- Do the existing phoneme / word data from a desktop-authored
  sequence render in *any* form on iPad, or is B88 a total
  blank? Quick check with a known Papagayo sequence.

Confirming / refuting these tightens the plan but doesn't change
the overall shape: the grid renders; the *authoring* surface
around it is where the work is.
