# LOR `.loredit` (S5/S6) import — enhancement plan

Status: **plan, nothing implemented yet.** Written from a full audit of one
real-world LOR 6.6 file (a vendor store sequence for a "Pixel Bright" pixel
layout, `saveFileVersion="16"`, ~165 s, 408 sequenced props, 67,158 effects)
run through the *actual* importer code, not a reading of it: a scratch
harness compiled `src-core/import_export/LOREdit.cpp` unchanged and pushed
every effect through the same calls `ImportS5` makes (`GetModelsWithEffects`,
`GetSequencingType`, `GetModelLayers`, `GetTrackEffects`,
`GetChannelEffects`, `GetPalette`, `GetxLightsEffect`, `GetSettings`).

## 1. Where we stand today

The S6 file uses the same `SeqProp` / `track` / `channel` structure as S5, so
it parses and imports without error. Quality splits cleanly in two:

| Side | Effects | Today |
|---|---|---|
| Channel (`<channel>`: AC props, dumb RGB, faces) | 26,601 | **Correct.** INTENSITY → On (level ramps → `Eff_On_Start/End`, RGB-encoded intensities → colour / colour-curve), SHIMMER → On+shimmer, TWINKLE → Twinkle. No issues found. |
| Pixel (`<track>`: motion rows) | 40,557 | **Imports, mostly renders wrong** — see the table below and §2. |

Pixel effects by LOR type:

| LOR effect | Count | Today | Target |
|---|---|---|---|
| colorwash | 22,221 | Color Wash, colour + brightness ramp correct | Keep; fix regions (§2.1); `blink_in_unison` (§6.2); gradient modes (§6.3) |
| singleblock | 14,261 | `SingleStrand` with **no settings** (default chase) — wrong | Morph (§4) |
| sketch | 1,910 | `Sketch` with empty def → **renders nothing** | Pictures + embedded SVG (§5) |
| spirals | 1,370 | Spirals, params mapped (speed heuristic) | Keep; calibrate (§6.4) |
| bars | 875 | Bars, but at 51% opacity (§2.3) | Fix opacity |
| picturexy | 240 | Pictures → nonexistent `*name*<guid>*.png` → **red/blank** | Extract embedded PNGs (§3.4) |
| starfield | 160 | **Dropped** (`Starfield` isn't an xLights effect; `EffectLayer::AddEffect` returns null silently) | Shape (§3.5). Note: median duration 18.9 s — these are long background effects on the matrices, so they matter far more than the count suggests |
| ripple | 132 | Ripple, 51% opacity | Fix opacity |
| curtain | 104 | Curtain, 51% opacity; "Progress" ignored | Fix opacity; progress (§6.5) |
| garland | 90 | Garlands | OK |
| plasma | 36 | Plasma with default settings | Map params (§3.5) |
| text | 36 | Text reading literally `<P><SPAN>RUN</SPAN></P>` | Strip HTML (§3.3) |
| movingshapes | 3 | **Dropped** | Shape (§3.5) |

### 1.1 Second reference file

A second vendor sequence on the **same Pixel Bright preview layout**
(`saveFileVersion="15"`, LOR 6.6.0.4, ~241 s; 34,190 track sub-effects +
34,701 channel effects), delivered as a `SequenceDistributor` self-extracting
`.exe` — a plain zip with a stub, so `unzip` pulls the `.loredit` out without
running anything. It stresses different paths:

| | File 1 | File 2 |
|---|---|---|
| `custom` rows | 3,132 (8%) | **14,108 (41%)** — prop parts: Lollipop Outline/Stick/Stripe, Ornament Full Outline/Waves, Present Bow/Boxes, Candy Outline/Stripes, snowflake arms |
| `rectangle` rows | 9,456 | 84 |
| top effects | colorwash, singleblock | colorwash 13,820, **bars 11,357** (8,214 `down_right`, 1,868 `up_right`), curtain 3,562 |
| new LOR types | — | fire (S6 params), twinkle, meteors (9 params), wave, **blendedbars, spinfade, simpleshape**, picture (embedded **GIF**/JPG), butterfly, snowflakes, fireworks, pinwheel, spinner, marquee |
| mixes | Average, Overlay/49, Maximum | Overlay at **0/48/49/51/100**, **Dissolve_Left_Right**, **Dissolve_Bottom_Top**, Alpha_Blend, Rt_Reveals_Lt; 2,087 two-sided effects |
| faces | one voice | **several voices** (lead + backing + a second matrix face) |

New breakages it exposes (harness output):

- **`Overlay/100` → invisible.** We emit `T_SLIDER_EffectLayerMix=100`,
  i.e. 0% opacity: 1,140 one-sided effects (colorwash, singleblock,
  spirals, butterfly) render nothing. Video: they are fully visible (§2.3).
- **Speed truncation.** Bars/Spirals convert speed with
  `(int)(speed / (20 / durationSec))` — for typical 0.2–0.6 s effects that is
  **0 for 11,130 of 13,122** bars+spirals, so they import static.
- **Diagonal bars** (`down_right`, `up_right`, …, 10k effects) pass through
  as invalid `E_CHOICE_Bars_Direction` values. xLights Bars has no diagonal;
  emulate with the buffer **Rotation** (±45°) plus zoom so corners stay
  covered, or add a diagonal direction to Bars.
- `spinfade` (arc spinner fade) and `simpleshape` (circle with oscillating
  size `O100O174…`, `fade_inner_out`) are **dropped**; `blendedbars` maps to
  Bars with no settings. Candidates: spinfade → Pinwheel/Fan, simpleshape →
  Shape (circle, growth) or Ripple.
- Text uses `<FONT color=#ff0000>` / `style="BACKGROUND-COLOR…"` HTML — strip
  and take the colour (§3.3).
- Wave still emits the missing-comma bug (`…Left to RightE_CHOICE_Fill_Colors…`).
- Colorwash with 2–3 active colours (3,956 effects; modes `single_color`,
  `dither`) — we take only the first colour. Video of a red+black
  `single_color` wash shows plain red, so first-colour is right for
  `single_color`; `dither` should scatter all active colours.

## 2. Structural problems (bigger than any per-effect mapping)

### 2.1 Motion-row regions are ignored — 19,512 effects (48% of pixel effects)

Each LOR 6 `<track>` has a `type` defining *where on the prop* its effects
render. The importer ignores it and turns every track into a full-model layer,
so e.g. a wash on a tree's "Strand 3" row lights the whole tree.

| `type` | Effects | Region attributes | Example rows |
|---|---|---|---|
| `none` | 21,045 | — (whole prop) | "Whole Tree 01", mouth-shape rows on the singing matrix |
| `rectangle` | 9,456 | `subx`, `suby`, `subw`, `subh` (fractions 0–1) | "Strand 1..16" (w = 1/16), "Tier 1..4", matrix "Column 1..4" |
| `custom_horizontal_buffer` | 6,924 | `subc` (or `subc2`): ordered `x,y;x,y;…` point list | "Vertical 01 - Hrz Buffer" on vertical outlines; HD mandala rows |
| `custom` | 3,132 | `subc`: unordered `x,y;…` node set (normalised) | "Snowflake Arm 01 D" … on the crystal snowflakes |

Same row definitions also appear as `MotionRowDefaults/MotionRowDefault` under
each `PropClass` / `PropGroup` (1,436 of them) — the prop's catalogue of rows.

Handling: `rectangle` → §3.1 (automatic, exact). `custom` and
`custom_horizontal_buffer` → §7 (exposed as separate import sources).

### 2.2 Layer explosion

`GetModelLayers` allocates one xLights layer per track side that has effects.
Results on this file: 16-strand trees get 21–22 layers, the singing-matrix
group 32, the HD mandala 26. Once regions are honoured, most of those rows are
disjoint in space and could share layers — §8.

### 2.3 One-sided `Mix_Overlay|49` renders at 51% opacity — 2,161 effects

Every LOR track effect has a left and right sub-effect plus a mix mode and mix
position. 2,161 effects (bars 875, singleblock 824, ripple 132, colorwash 130,
curtain 104, starfield 60, plasma 36) are `Mix_Overlay|49` with the **right
side `lightorama_none`**. `GetSettings` emits `T_CHOICE_LayerMethod=Normal`
plus `T_SLIDER_EffectLayerMix=49`, and `Mix_Normal` applies
`fg.alpha *= (1 - mix)` (`PixelBuffer.cpp:1208`), i.e. 51% opacity against
whatever is below.

**Video-verified (§10.2): LOR renders these at full brightness and fully
opaque.** Single-block tier bands on the 96×50 matrix measure ~85–100% of
their palette colour (red 245/255, gold 196/208, blue 128/149) and completely
hide the plasma underneath. The mix position only means something when both
sides are present.

File 2 confirms this for **every** position: one-sided `Overlay/100`
single-blocks on the driveway candy canes render at full brightness (today
they are invisible, §1.1); the rule applies equally to `/0` and `/51`.

Fix: when only one side exists, emit plain `Normal` with no
`T_SLIDER_EffectLayerMix` and no brightness change, whatever the mix and
position. Two-sided mixes → §2.5.

### 2.4 Row stacking order — **reversed today** (video-verified)

Every `SeqProp` has `OverlayMotionRows="1"`. **Later LOR tracks draw on top
of earlier ones**: on the matrix groups the `Tier N` / `Column N`
single-blocks (later tracks) fully cover the plasma and moving shapes on
`Whole Matrix 01/02` (tracks 0/1), and the singing-face mouth rows (last
tracks) sit over the column blocks. We currently put track 0 on the *top*
xLights layer, which inverts every overlap. Fix: last LOR track → xLights
layer 0. Within one track's L/R pair the order depends on the mix (§2.5 —
for Overlay the **right** side is on top). This also fixes the starfield/plasma backgrounds hiding the
foreground once §3.2 makes them opaque.

### 2.5 Two-sided mixes (2,087 effects in file 2)

| LOR mix | Count | Video behaviour | xLights mapping |
|---|---|---|---|
| `Mix_Overlay/100` | 1,479 (mostly colorwash L + curtain R) | **right drawn over left, opaque**: garage-roof snowflakes show the white R curtain sweeping over the red L wash, then the reverse, matching event times to the frame | R layer **above** L, `Normal`. Generalise: position p = R's opacity → `T_SLIDER_EffectLayerMix = 100 − p` on the R layer (unverified for p < 100) |
| `Mix_Overlay/0,48,49` | 69 | — | same rule; p = 0 would hide R, so check in video first |
| `Dissolve_Left_Right` `R0R100…` | 241 (spirals pairs) | L dissolves into R over the effect — outlines show **separate red and green pixels** mid-way, never the yellow a colour crossfade gives | L above R; L gets out-transition **Dissolve** spanning the whole effect (`T_CHOICE_Out_Transition_Type=Dissolve`, fade-out = duration). `R100R0` (6) → swap roles |
| `Dissolve_Bottom_Top` | 35 | not checked | same, or a bottom→top wipe transition if dissolve isn't right |
| `Mix_Maximum` | singing faces | — | `Max` (already) |
| `Mix_Alpha_Blend`, `Mix_Rt_Reveals_Lt` | 10 | — | `Normal` alpha / `2 reveals 1` |

`T_CHECKBOX_LayerMorph` (mix 0→1 over the effect, `PixelBuffer.cpp:4149`) is
a *colour crossfade*, so it does **not** match LOR's dissolve.

## 3. Phase 1 — quick wins

### 3.1 `rectangle` rows → per-effect sub-buffer

Emit `B_CUSTOM_SubBuffer=x1xy1xx2xy2` (percent, `SubBufferSpec::Parse`,
`PixelBuffer.cpp:2016`) on every effect from a `rectangle` track:

```
x1 = subx * 100            x2 = (subx + subw) * 100
y1 = (1 - suby - subh)*100 y2 = (1 - suby) * 100     // LOR y is top-down, xLights bottom-up
```

**y flip video-verified:** on the 16×50 180° tree the `Tier 1` row
(`suby=0`) is the *top* band, Tier 4 the bottom. Fixes 9,456 effects with no
mapping-dialog change.

`LOREditEffect` needs the owning track's attributes: extend `AddEffects` /
`GetTrackEffects` to carry `type` + `subx/suby/subw/subh/subc` into each
effect.

### 3.2 Opacity and stacking fixes (§2.3, §2.4)

- In `GetSettings`: only emit `T_SLIDER_EffectLayerMix` / non-Normal
  `T_CHOICE_LayerMethod` when both sides are present; otherwise plain
  `Normal` at full brightness.
- In the layer allocation (`GetModelLayers` / `GetTrackEffects` callers):
  assign the **last** LOR track to xLights layer 0 so later rows draw on top.

### 3.3 Text

- Strip HTML from the decoded text (`<P>`, `<SPAN>`, `<BR>` → newline, etc.)
  before the existing entity replacements. Per-character colour/font lives
  in `<SPAN style=…>`; with no style the video shows **white** text even
  though the palette has red/blue/green/gold active — so default the palette
  to white unless a span sets a colour (use the first span colour when
  present).
- Video: "RUN" fills roughly half the matrix height, vertically centred,
  scrolling right→left (`left`) once across the effect (fit to duration).
- S6 parameter list grew: `text, fontSize, movement, position,
  peekabooHold, <empty>, speed, repeat(once_fit_to_duration|…), wrap,
  peekabooExit(reverse|…), smooth`. Today's code reads `[6]` as speed, which
  is still right; map `repeat` (fit-to-duration → `E_TEXTCTRL_Text_Speed`
  derived from duration) and ignore wrap/smooth.

### 3.4 Picture XY embedded images

The `.loredit` embeds its pictures: `<pictures><picture name="*guitar*<guid>*.png">`
with base64 PNG content (2 in this file, 640×457 and 660×380; decode
verified). The `picturexy` filename parameter references that name.

Video: the guitar at width/height 100/100 is **stretched to the whole prop**
(fills the 96×50 matrix edge to edge), so 100/100 = Scale To Fit, and other
values are percentages of the prop size. Today's `E_CHOICE_Scaling=No Scaling`
is wrong — on a 96×50 buffer it would show an unscaled 640×457 crop.

Import: decode each `<picture>` once, add it to the sequence's `SequenceMedia`
as an **embedded** image keyed by a clean name (strip the `*…*<guid>*`
decoration, keep it unique), and point `E_TEXTCTRL_Pictures_Filename` at it.
No files written to the show folder; the image travels in the `.xsq`.

S6 `picturexy` params: `file, width, height, left, top, rotation, R, G, B,
opacity?, anchor(FFFF), …, flipH, flipV` — current code maps width/left/top
ramps to vector movement; keep that, and map R/G/B < 100 to a colour
adjustment if any appear (none here).

### 3.5 Effects with no xLights equivalent name

- **starfield** (`shape, colorMode, style, density, growth, speed, tail,
  arms, rotation, newStarLocation, pattern`; shapes used: `heart`, `tree`) →
  **Shape** effect (has Heart/Tree/Star/Snowflake; random location, grow,
  lifetime). Video: on the matrices it reads as sparse, small coloured
  shapes drifting outward — so small start size, low count, outward
  velocity. Approximate, but these are long background effects so "close"
  beats "absent".
- **movingshapes** (`star5, count, size, speed, palette, solid,
  random_wrap, direction, rotationMode, …`) → Shape (`Star`, 5 points,
  `Shapes_Velocity`/`Direction`, `Shape_Rotation`). Video: ~8 large **solid**
  gold stars wrapping across the matrix and rotating. xLights Shape draws
  outlines, so use a large thickness to approximate the fill.
- **plasma** (`colorMode(blended|single|dual), style, density, speed`) →
  Plasma colour/style/line density/speed.
- Any remaining unmapped type: log a warning with the LOR type name and count
  (today `AddEffect` returns null silently, so effects vanish without a
  trace). Ideally a post-import summary dialog: "N effects of types X, Y
  could not be converted".

### 3.6 Latent bugs found while reading `LOREdit.cpp`

- Speed truncation to 0 for Bars/Spirals (§1.1) — compute in float, map
  LOR speed (≈ cycles over the effect) directly to Bars cycles / Spirals
  movement, then calibrate against video.

- `GetPalette` 3-part (gradient) colour: `palette += "C_BUTTON_Palette"`
  is missing its leading `,` — breaks the settings string whenever the
  gradient colour isn't the first palette entry (`LOREdit.cpp:146`).
- Bars: `settings += "E_CHECKBOX_Bars_Highlight=1"` missing `,`; and
  `highlight` is now `none|white|last color`, not `True|False`.
- Wave: `"E_CHOICE_Fill_Colors=…"` missing `,`.
- Spirals: `settings += ",E_CHECKBOX_Spirals_Blend=1,"` trailing comma.

## 4. Phase 2 — Single Block → Morph (14,261 effects)

LOR's Single Block (docs: *Motion Effect Reference*) draws a block made of
head (fade-in), body (solid) and tail (fade-out) that crosses the prop at
"fit to duration" speed. Parameter order in the file:
`direction, headLen, bodyLen, tailLen, position, size, offset, colorMode`.
All 14,261 here are `0,0,50,50,100,50,single_color` and differ only in
direction (down/right/left/up): **no head, no body, a 50% fading tail, full
cross-section, centred** — a hard leading edge trailing a fade. That's Morph's
shape.

Mapping:

- Start line = the edge the block enters from; end line = the opposite edge;
  each spanning the full cross-section (`size` 100, centred by `offset` 50).
  e.g. `down`: start `(0,100)–(100,100)`, end `(0,0)–(100,0)`.
- Colour: **single-entry palette `[c]`**. The video shows the tail fading
  to *transparent* — the plasma under a matrix tier shows through the tail
  while the solid leading part hides it — not fading to black. Morph's ≤3
  colour path does exactly that: the back half of the tail ramps alpha to 0
  (`MorphEffect.cpp:484-490`), honoured because `Normal` layers allow alpha
  (`PixelBuffer.cpp:1170`). A `[c, c, black]` palette would paint black over
  lower layers.
- Intensity ramp → brightness VC, as today.
- **Tail length / timing — video-verified (§10.2):** "fit to duration" means
  the *whole block, tail included, has left the prop when the effect ends*.
  Fitting the head position of a 0.63 s left-moving block on the 96×50
  matrix frame by frame: head enters at t = 0, crosses at a constant ~2.3
  widths/s, reaches the far edge at **t ≈ 0.67–0.69**, and the faded tail is
  gone by t ≈ 0.95. That is travel = width + tail = 1.5 widths in the
  effect's duration, so head-at-edge = 1/(1 + tail).
  Morph's tail is derived from `MorphDuration` d (head-travel % of the
  effect): tail = travel × (1/d − 1) (`MorphEffect.cpp:442`). So
  **`MorphDuration = 100 / (1 + tail/100)`** — 66.7 for this file's tail 50 —
  with start/end lines at the prop edges. Morph then has the tail's end
  leave exactly at the end of the effect.
- Non-zero head/body (not in this file): head → `MorphStartLength`/
  `MorphEndLength`; body has no direct equivalent — approximate by adding
  it to head length.

## 5. Phase 3 — SVG in the Pictures effect, and Sketch → SVG

### 5.1 Pictures: accept and rasterize SVG

Useful on its own, independent of LOR. Most plumbing exists:
`SVGMediaCacheEntry` (`SequenceMedia.h:268`) already handles embedded SVG in
the `.xsq`, packaging (`SequencePackage.cpp:1134`) and thumbnails; Ripple
already loads SVGs via `GetSVG` (`RippleEffect.cpp:976`); nanosvg +
nanosvgrast are in `dependencies/nanosvg`.

- `PicturesEffect.cpp:368` extension check + desktop/iPad file-picker
  filters: add `svg`.
- On an SVG, `Render` gets the `SVGMediaCacheEntry` instead of an
  `ImageCacheEntry` and asks it for a raster **at the size the current
  scaling mode actually draws** (Scale To Fit → `BufferWi×BufferHt`; Keep
  Aspect (±Crop) → uniform fit; No Scaling + start/end scale → intrinsic size
  × scale%). Everything downstream (movement, vector, peekaboo, wrap) keeps
  consuming an `xlImage`.
- Cache rasters **on the media entry**, keyed by (w, h), mirroring
  `ImageCacheEntry::GetScaledImage` — not in `PicturesRenderCache`, because
  frame-parallel clones get a fresh per-buffer cache and would re-rasterize
  every frame. Parse the `NSVGimage` once per entry.
- Scale ramps: rasterize at each distinct scale rather than zooming a raster
  (a 200% zoom of a raster is blocky; re-rasterizing stays sharp). Bound the
  cache (LRU) since a ramp produces many sizes.
- nanosvg's `nsvgRasterize` scales uniformly only. For Scale To Fit's
  non-uniform stretch either transform the parsed points by (sx, sy) before
  rasterizing, or update nanosvg if upstream's `nsvgRasterizeXY` is newer
  than our copy.
- Rendering straight at LED resolution gives nanosvg's coverage
  anti-aliasing — a better result than downscaling a large bitmap.

### 5.2 Importer: LOR sketch → embedded SVG + Pictures

The 1,910 sketch effects contain only **16 unique drawings**, all using one
group-header form (`Append … None <colours> 0 True 100 None Butt`): plain
filled groups — no chases, dashes, morphs, colour patterns or progress
animation that a static SVG would lose.

Sketch definition grammar (from this file):

```
<groups joined by '~'>,width,height,left,top,rotation,anchor
group  : GSVG%20<n> <combine> 0 0 0 0 <progress-ramp> 0 <pattern> <colours> <penWidth> <fill> <?> <dash> <cap>
         colours: AARRGGBB-<0|1>[;…]   (first/active entry is the fill colour)
path   : PPath%20<n> Mix  <SVG path data: M/L/C, normalised 0–1, y down>
```

Conversion (prototyped with nanosvg; reindeer and Santa singing-face frames
render correctly):

- One `<svg viewBox="0 0 1 1">` per unique drawing; dedupe by definition.
- **Each group → one compound `<path fill-rule="evenodd">`** containing all of
  the group's paths, in the group's colour. Emitting paths individually, or
  with non-zero fill, buries the artwork under its own black detail layers —
  that was the bug in the first prototype.
- `fill=False` groups → stroke with pen width.
- Embed each SVG via `SequenceMedia`; Pictures effect with that SVG.
- Transform params → Pictures: width/height % → start/end scale; left/top →
  centre position; `R…R…` ramps → vector movement (start/end). Rotation is
  only used as a wobble (`O5O0…`, 3 effects) or `-360` (≡ 0, but with anchor
  `FFTF` — check whether that's a flip); wobble → layer Rotation value curve
  if worth it.
- Static distribution here: 1,758 effects are a fixed 200% zoom + offset
  (the two singing faces); 88 are 100%; a handful zoom/pan.

## 6. Phase 4 — fidelity items

### 6.1 Effect-level settings string

`mix | mixPos | sparkle | blinkMode | blinkRate | <left> | <right> [| extra]`.
Values seen: mix `Mix_Average/Overlay/Maximum`; mixPos `0|49`; sparkle `0`;
blinkMode `full|blink_in_unison`; blinkRate `20|10`. Effect segment:
`lightorama_<type>:<palette>:<params>`. Palette entry `AARRGGBB,active` or
`AARRGGBB,AARRGGBB,active` (time gradient). Numeric params may be ramps
`R<start>R<end>R<accel?>R<?>R<?>` or oscillations `O…`.

### 6.2 `blink_in_unison` (872 effects, mostly colorwash)

**Video-verified (§10.2):** a whole-effect on/off strobe with a **0.10 s
period** (10 Hz), whose on-level follows the effect's intensity ramp (a
100→0 pixel-stake wash blinks at 126, 116, 109, 79, 39, 24 … then out).
All 872 use rate `20`, i.e. 20 on/off toggles per second. Mapping: at the
common 50 ms frame time this is exactly Color Wash's existing **Shimmer**
(`ColorWashEffect.cpp:101`, alternate frames black) and On's shimmer; for
other frame times, or other rates, use a brightness square-wave value curve
instead (period = 2 / rate seconds).

### 6.3 Color Wash modes

`single_color` (22,205), `diagonal_up_gradient` (8), `diagonal_down_gradient`
(6), `dither` (2). Gradients → palette colour curve with a spatial
`Timecurve` (no diagonal in xLights — nearest of up/down/left/right).
Horizontal/vertical fade fields are all `full` here (already mapped).

### 6.4 Spirals / Bars speed calibration

Both convert speed with `speed / (20 / durationSeconds)` heuristics. Calibrate
against the video: spirals here are `1, left_to_right, 20, 50, 0, False,
trail_left, 50` at 0.18–1.2 s durations.

### 6.5 Curtain "Progress"

S6 added a sixth param (`R0R100R1.00R2.00R0.00` = progress ramp 0→100 with
`once_fit_to_duration`). Map to Curtain speed so the open/close completes in
the effect duration.

### 6.6 Ripple

S6 params: `shape, repeat, ringWidth, spacing, speed, x, y, highlightAngle,
inward, outerLimit` — already mapped except spacing/speed (speed is 0 on all
132 here, with 0.2–0.4 s durations — check what LOR renders).

## 7. Phase 5 — `custom` / `custom_horizontal_buffer` rows as import sources

These rows describe a node subset (or an ordered node path) of the prop.
There's no sub-buffer equivalent; the right xLights target is a submodel or a
group, which only the user can pick.

File 2 makes this the single biggest item: 41% of its track effects sit on
`custom` rows that are part decompositions (Bow / Present Boxes, Lollipop
Outline / Stick / Stripe, Ornament Full Outline / Waves, Candy Outline /
Stripes). Those are exactly what xLights submodels are, and the names will
often auto-map to same-named submodels.

Proposal: list every such row as **its own source** in the mapping dialog,
named `<prop>/<row>` (e.g. a crystal snowflake's eight `Snowflake Arm NN …`
rows, a vertical outline's `Vertical 01 - Hrz Buffer`). The user then maps
them to submodels, or drags them onto a group such as "Snowflake Arms".
Effects on those rows are **not** applied when the parent prop is mapped —
unmapped means "not imported", which beats today's full-prop smear.

- Core: `GetModelsWithEffects` emits the extra `<prop>/<row>` names;
  `GetSequencingType` / `GetTrackEffects` resolve them to the single track;
  `GetModelLayers` / `GetTrackEffects` for the bare prop skip `custom*`
  rows. Pick a separator that can't collide with LOR names (they already
  contain `/`, e.g. "Vertical/Outline") — e.g. ` ▸ ` or `::`.
- `custom_horizontal_buffer`: the point order defines the buffer axis. When
  mapped to a single-line target, emit the "Single Line" buffer style;
  a vertical outline with a top-down point list maps directly.
- Both apps pick this up automatically because the desktop dialog and the
  iPad import session both enumerate sources via `GetModelsWithEffects`
  (`ImportEffects.cpp:3022`, `XLImportSession.mm:443`).

## 7a. Singing faces → timing track + Faces effect

**What LOR stores:** no lyrics, words or phonemes. The file has one
free-form grid ("Default Free", 829 beat marks) and a fixed 50 ms grid named
"Singing Faces" used only for snapping. The faces are **baked**: each mouth
shape is its own prop (or, on the matrix, its own track) carrying plain
effects, exactly one shape active at a time.

| Face style | Shape sources | Shapes |
|---|---|---|
| "FaceV2" coro/bulb faces | one DumbRGB **channel prop per shape**, `<face> Mouth <shape>`, plus `<face> Eyes Open/Closed` | Closed, AI (Full Open), E (Half Open), Ah, OU, WQ, L, FV, MBP |
| Older "Face" faces | same pattern | Closed, Half Open, Full Open, "OH" |
| Matrix singing face | **tracks** on one matrix group, `C1 Mouth <shape>`, each a sketch drawing (§5.2) | same nine as FaceV2 (`L(th)` for L) |

In file 1 all three are the same lip-sync: FaceV2 and the matrix agree on 877 of 879
shape-change times with identical shapes, and the 4-shape faces are a
projection of the same sequence. FaceV2 mouth effects are 50 ms chunks
(1,082 effects → 870 runs after merging identical neighbours), contiguous
and non-overlapping; ~200 one-frame ramps to/from black soften transitions.
Eyes are explicit: closed for 0.15–0.25 s, ~48 blinks.

File 2 has **several voices**: a lead (Zuzu / Bulb4 / matrix `C1`,
~1,430 shape changes), backing voices (Elden = Felix = Bulb1 = Bulb2;
Ralphie = Bulb3 differ slightly) and a second matrix face (`C2`). So build
one timing track **per distinct shape sequence**, merging faces whose change
times agree (≥ 95% common), and name it after the faces that share it.

**Today** the mouth props import as On effects on whatever model each is
mapped to, which only works if the user maps every LOR shape prop onto a
matching xLights node range one by one. Nothing produces a Faces effect or a
lip-sync timing track.

**Plan:**

1. **Detect face sets** in `LOREdit`: group props (or tracks) whose names
   match `<face> Mouth <shape>` / `<face> Eyes <Open|Closed>`, with a shape
   table:

   | LOR | xLights phoneme |
   |---|---|
   | Closed | rest |
   | AI (Full Open), Full Open | AI |
   | E (Half Open), Half Open | E |
   | OU, "OH" | O |
   | WQ / L, L(th) / FV / MBP | same |
   | Ah | AI (the 4-shape face projects it onto Full Open); verify visually |

   `U` and `etc` are never produced.
2. **Synthesize a timing track** (offered in the dialog's timing list, e.g.
   "LOR Lip Sync – <face>", one per distinct sequence; here all faces
   share one). The Faces effect requires a **3-layer** track and reads
   phonemes from layer 2 (`FacesEffect.cpp:496`):
   - phonemes: merged runs, `rest` omitted;
   - phrases: split on Closed runs ≥ ~0.3 s (gives 23 phrases here, e.g.
     7.2–10.7 s), labelled "phrase N" since the text is unknown;
   - words: same spans as phrases (no word data exists).
3. **Map a face set as one source**: list `<face> (singing face)` in the
   mapping dialog (alongside the individual shape props for users who want
   the old per-node behaviour). Mapping it onto an xLights model with a face
   definition emits one Faces effect per phrase (or one over the song) with
   `E_CHOICE_Faces_TimingTrack` set to the synthesized track and the mouth
   colour from the LOR effects (yellow here). Matrix face sets map the same
   way when the target has a matrix face definition; otherwise fall back to
   the §5.2 SVG Pictures path, which reproduces LOR's own artwork.
4. **Eyes**: the Faces effect has no external eye track — it blinks on its
   own (`Eyes = Auto`). Either accept Auto (simplest), or keep LOR's
   explicit blinks by also importing the `Eyes Closed` props as On effects
   onto the target's eyes-closed submodel with Faces eyes set to Open.
   Default to Auto.

## 8. Phase 6 — layer consolidation

After §3.1, pack rows into the fewest layers: tracks whose regions are
disjoint, or whose effects never overlap in time, can share a layer. Greedy
interval packing that preserves relative order between overlapping effects
(§2.4) is enough. Optional: name layers after the LOR row when not packed.

## 9. Phase 7 — de-duplicate the S5 orchestration

`xLightsFrame::ImportS5` (`ImportEffects.cpp:3005`) and the iPad
`XLImportSession` apply loop (`XLImportSession.mm:1860-1960`) carry
hand-mirrored copies of the per-model / per-strand / per-node dispatch into
`MapS5*`. Every phase above touches that dispatch (new source names, region
handling, embedded media). Extract it into one core function in
`EffectMapper.cpp` first, or each change has to be made twice.

## 10. Verification

1. **Conversion harness.** Rebuild the scratch harness as a reusable check:
   link `LOREdit.cpp` + `string_utils` + `Color` + pugixml, dump per LOR
   type → xLights effect name, settings and palette. Run it before/after each
   phase; diff the dump. (Effect min/max statics must come from the real
   metadata, not stubs, for value-curve bounds to be meaningful.)
2. **Render.** Import into a show whose models match the LOR layout
   (16-strand trees, 32×50 / 20×40 / 96×50 matrices, snowflakes), render
   headless, scrub against the vendor's sample video.
3. **Video check.** §10.2 below is what the vendor's sample video already
   settled; re-scrub the same moments against an xLights render after each
   phase.

### 10.1 Time alignment (measured)

- The sample video has a ≈3.9 s title card. Cross-correlating its audio
  with the mp3: **video = song + 3.913 s** (sharp peak).
- The lights in the video lead its own audio by ≈0.34 s (visually video ≈
  seq + 3.575 s, from tier-chase and block-entry frames).
- That is an audio-file difference, not a video artefact: the mp3's first
  sound is at 0.25 s and its big guitar onset at 0.65 s, versus the
  sequence's first effects at 0.54 s and its 77-effect burst at 0.96 s;
  correlating the 829 timing-grid marks against mp3 onsets has an alias
  peak at −0.29 s.
- **Conclusion: with this mp3, import with ≈ −300 ms time adjust.** LOR store
  audio evidently carries ~0.3 s more lead-in than retail copies. Worth a
  note in the import dialog; an automatic onset-based offset suggestion is
  possible but not required.

File 2 (same method; retail m4a): video = song + 1.24 s by audio
cross-correlation, and video = seq + 1.25 s by correlating frame-to-frame
brightness increases against effect start times — so **no offset** for that
file. Re-running the brightness method on file 1 gives video = seq + 3.65 s
(a clean peak), i.e. seq − song = −0.26 s, agreeing with −0.30 s within a
frame. The offset is per-release, so the import dialog can't assume one; an
automatic suggestion (onset correlation of the chosen media against effect
starts) is feasible and cheap.

### 10.2 What the video settled

| Question | Answer | Evidence (seq time) |
|---|---|---|
| `rectangle` y origin | top-down (`suby=0` is the top) — flip needed | tree tier chase 7.26–10.19: Tier 1 lights the top band |
| Row stacking | later tracks on top — reverse of today | matrix tier blocks hide `Whole Matrix 01` plasma (5.46–7.26); mouths over column blocks (7.3–7.9) |
| One-sided `Overlay/49` | full brightness, opaque | tier bands 85–100% of palette colour over plasma (5.6) |
| Single Block timing | travel = width + tail in the effect duration; tail fades to transparent | head reaches far edge at t ≈ 0.67–0.69, tail gone ≈ 0.95 (5.46–6.09) |
| `blink_in_unison\|20` | 10 Hz on/off, on-level follows intensity ramp | pixel stakes 36.86–37.49 |
| Text colour | white when the HTML has no colour; palette ignored | "RUN" 13.29–13.68 |
| Picture XY 100/100 | stretched to the prop | guitar 46.29–46.47 |
| Sketch transform | 200% zoom + offset frames Santa's head on the singing matrix; mouth frames swap every 50–100 ms | singing matrix 7.3–7.9 — matches the nanosvg prototype |
| Moving shapes | ~8 large solid rotating gold stars, wrapping | 53.35–54.56 |
| Starfield look | sparse small coloured shapes drifting outward | matrices from 7.26 |
| *File 2:* one-sided `Overlay/100` | fully visible | candy-cane single-blocks 57.25–59.8 |
| *File 2:* two-sided `Overlay/100` | R over L, opaque | garage-roof snowflakes 57.25–58.6 |
| *File 2:* `Dissolve_Left_Right` | pixel dissolve L→R, no colour blend | rooflines/outlines/tree 108.25–113.58 |
| *File 2:* `down_right` bars | diagonal candy stripes on the canes | canes 57.25–59.9 |

## 11. Parity / release notes

- All of §2–§8 is `src-core/` (`LOREdit`, `EffectMapper`, `PicturesEffect`,
  `SequenceMedia`) — auto-applies to iPad. Note it on
  `plans/platform-parity/08-import-export.md` row 8 (LOR S5 `.loredit`).
- §5.1 needs the iPad Pictures file picker to accept `.svg` (same PR) and a
  `04-effects-catalog` row note.
- §9 removes a desktop/iPad duplication — note on row 8.
- README: one `-enh` per user-visible phase (sub-regions, single block,
  sketches/SVG pictures, embedded pictures), plus one for "Pictures effect
  supports SVG".

## 12. Open questions

- Text: font face and exact size mapping (LOR font size 50 ≈ half the
  matrix height here).
- Sketch anchor `FFTF` with rotation `-360`: flip or no-op (§5.2) — the face
  in the video isn't mirrored, so probably a no-op.
- LOR row stacking within one track's L/R pair when the mix is not Overlay
  (e.g. the singing-face `Mix_Maximum` pairs) — Max is symmetric, so it
  shouldn't matter here.
- How LOR mixes rows when `OverlayMotionRows="0"` (not present in this file).
