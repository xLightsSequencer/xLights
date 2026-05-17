# Future — Custom Model + Face / State + DMX Authoring

The model-authoring stack: Custom Model editor, SubModels,
Face / State authoring, DMX model deep authoring (servo, skull,
position-zones), and the Wiring view. Every serious xLights
user hits at least one of these dialogs; on iPad, none are
reachable.

Source: §2.4 + §2.5 + §2.7 + §2.6 of the 2026-04-23 gap analysis
(Phases T + U + V + W).

## Status

**Substantially shipped.** Phase J (2026-05-15..16) landed the
bulk of this stack:

- **J-23** — Custom Model visual editor + SubModel geometry editor
  (covers most of MA-1; MA-4 partial).
- **J-22** — Faces / States / Dimming Curve / SubModels editors
  (covers MA-9, MA-10, MA-11; MA-4 partial).
- **J-30** — DMX deep authoring path. All 8 DMX fixture types
  have per-type panes; preset list, wheel-colour list, position-
  zone, skull-config, mesh/servo linking, and start-channel
  editors all wired. Covers DM-1, DM-2, DM-3, DM-4, DM-5, DM-6,
  DM-7, DM-8, DM-9, DM-10, DM-11 (via per-servo surface), DM-12,
  DM-13, DM-16, DM-17. PWM brightness / gamma, mesh paths on
  some advanced fixtures, and the `dmxColor`/`PresetList`
  list-editing sheets are detailed in the J-30 closeout.
- **J-13** — TerrainObject heightmap painting (covers VO-4).
- **followups.md 2026-05-17** — Add-as-alias on missing-model
  prompt (partial MA-12 — alias half only; full Map-Models flow
  stays desktop-only).

What remains here is **advanced authoring polish**: video-pixel
detection (MA-2), face/state catalog downloader (MA-15), wiring
diagram (WV-1), Ruler view object (VO-5), and ViewObjectPanel
multi-select / reorder ergonomics (VO-6 partial).

## Custom Model authoring (Phase T)

| # | Dialog | Status | Severity | Effort |
|---|---|---|---|---|
| MA-1 | **CustomModelDialog** — 3D node-grid editor, ~40 ops (cut/copy/paste/find-replace/rotate/flip/reverse/shift/insert/delete row/column/compress/trim/shrink/expand/copy-layer-fwd/back/all/wire-cells-h/v/auto-number/background image overlay/zoom/wiring view/output to lights/import/export/import from controller) | ✓ shipped J-23 — `CustomModelEditorSheet` covers the core grid + zoom/pan + Bresenham distribute + background image. The full 40-op surface (compress/trim/wire-cells/auto-number/import-from-controller) is not all ported. | P2 | XL |
| MA-2 | **GenerateCustomModelDialog** — 6-step wizard (Prepare → Choose Media → Start Frame → Manual Identify → Bulb Identify with sensitivity/blur/despeckle/contrast/gamma/saturation → Custom Model) | open | P3 | XL |
| MA-3 | **ModelRemap** — load original + new wiring → generate remap | open | P3 | S |
| MA-4 | **SubModelsDialog** — list CRUD + node-ranges grid + sub-buffer panel + buffer style + vertical buffer + live 3D preview | ✓ shipped J-22/J-23.3 — SubModels list + SubModel geometry editor. Sub-buffer panel + vertical buffer details deferred. | P2 | L |
| MA-5 | SubModelGenerateDialog — base name + type + count → auto-generate N sub-models | open | P3 | S |
| MA-6 | EditSubmodelAliasesDialog — alias listbox + add/delete/move | open | P3 | S |
| MA-7 | NodeSelectGrid — ordered toggle, select all/none/invert/load-from-model, zoom, background-image overlay, find/search, output to lights | partial — reused inside J-22 face/state node selection; not exposed as a general-purpose dialog | P2 | M |
| MA-8 | ChannelLayoutDialog — HTML window with channel breakdown table; Print + View in Browser | open | P3 | S |

## Face / State / Aliases authoring (Phase U)

| # | Dialog | Status | Severity | Effort |
|---|---|---|---|---|
| MA-9 | **ModelFaceDialog** — Face type (Single Nodes / Node Ranges / Matrix); per-phoneme node selection (10 phonemes: AI, E, ETC, FV, L, MBP, O, REST, U, WQ) + Eyes Open / Closed / Open3 / Closed3 + Mouth variants; Force Custom Colors; Output to Lights live preview; Image Placement (Centered / Scaled / Aspect / Crop); MatrixFaceDownload integration; per-face Add / Import / Copy / Rename / Delete / Shift; embedded ModelPreview | ✓ shipped J-22 — `FaceStateEditorSheet`. MatrixFaceDownload catalog (MA-15) not wired. | P2 | XL |
| MA-10 | **ModelStateDialog** — State type (Single Nodes / Node Ranges); per-state name + nodes + colour grid (200 rows); Force Custom Colors; Output to Lights; SevenSegmentDialog integration; Add / Import / Copy / Rename / Shift / Reverse / Clear / Export / Download; Color Draw Mode (All Colors / White Only); embedded preview | ✓ shipped J-22 — same sheet. SevenSegmentDialog (MA-14) not wired. | P2 | XL |
| MA-11 | **ModelDimmingCurveDialog** — 4 modes (Single Brightness/Gamma, Single Curve File, RGB Brightness/Gamma per channel, RGB Curve Files); load curve files; per-channel visualisation | ✓ shipped J-22 | P2 | L |
| MA-12 | EditAliasesDialog — model alias listbox + move / add / delete | partial — add-as-alias shipped 2026-05-17 via `MissingModelAliasSheet`; standalone aliases-list editor open | P3 | S |
| MA-13 | StrandNodeNamesDialog — 2-column grids (strand names / node names); conditional Generate Node Names button (DMX models only) | open | P3 | M |
| MA-14 | SevenSegmentDialog — 6 segment checkboxes (Thousands / Hundreds / Colon / Tens / Decimal / Ones) + reference image | open | P3 | S |
| MA-15 | **MatrixFaceDownloadDialog** — tree navigator (categories / artists), search, image preview (256×128), face details, Insert Face. HTTP catalog from `nutcracker123.com/xlights/faces/xlights_faces.xml`; ZIP download with per-phoneme PNGs | open | P3 | XL |

## DMX models deep authoring (Phase V)

J-30 (2026-05-15) landed per-type panes for all 8 DMX fixture
types plus the heavy preset / wheel-colour / position-zone /
skull / mesh-servo-linking list editors. Rows below mark
caveats where partial.

| # | Item | Status | Severity | Effort |
|---|---|---|---|---|
| DM-1 | DmxMovingHead — Pan motor (16-bit coarse/fine, 540°), Tilt motor, RGB; basic property-grid | ✓ shipped J-30 — full surface (DMX style, fixture, channel count, Hide Body, Pan + Tilt motor blocks, Color Type picker, Dimmer/Shutter/Beam blocks). | P2 | S |
| DM-2 | **DmxMovingHeadAdv** — 5 mesh files (base/yoke/head + textures), position zones (collision avoidance), 3D mesh import, advanced motor config | ✓ shipped J-30 — motor + ability + mesh-file surface + position-zone editor. PWM brightness / gamma deferred (controller-caps wiring). | P2 | XL |
| DM-3 | DmxFloodlight — RGB-only basic | ✓ shipped J-30 | P2 | S |
| DM-4 | DmxFloodArea — extends floodlight with area beam | ✓ shipped J-30 (shares Floodlight property surface) | P3 | S |
| DM-5 | DmxGeneral — generic configurable channel layout | ✓ shipped J-30 | P3 | M |
| DM-6 | **DmxServo** — 1–25 servos, 1–24 static + motion meshes per servo, per-servo channel/range/style, 16-bit toggle, controller min/max pulse mapping | ✓ shipped J-30 — per-servo property surface + static/motion image files. | P3 | XL |
| DM-7 | **DmxServo3D** — 3D mesh + multi-servo puppet, mesh-to-servo / servo-to-mesh linking matrix | ✓ shipped J-30 — per-servo surface, mesh files, Mesh⇄Servo linking pickers. | P3 | XL |
| DM-8 | DmxSkull — preset skull animatronic (Jaw / Pan / Tilt / Nod / EyeUD / EyeLR servos + RGB), Skulltronix preset | ✓ shipped J-30 — per-servo blocks, servo-enable toggles, Apply Skulltronix Preset button. Mesh paths still desktop-only. | P3 | M |
| DM-9 | Color ability subsystems — RGB / RGBW / CMY / CMYW / ColorWheel (1 wheel + 1 dimmer + ≤25 custom colours with DMX value mapping) | ✓ shipped J-30 — scalar channels for all subsystems + `DmxWheelColorListEditorSheet` for the ≤25 wheel colours. PWM brightness / gamma still desktop-only. | P2 | M |
| DM-10 | Beam / Dimmer / Shutter / Preset abilities (mixin) | ✓ shipped J-30 — Beam / Dimmer / Shutter inline; `DmxPresetListEditorSheet` covers preset list editing. | P2 | M |
| DM-11 | ServoConfigDialog — 3 spinners (servos 1–25 / static meshes 1–24 / motion meshes 1–24) + 16-bit toggle | ✓ shipped J-30 (inlined into the Servo / Servo3d property surface — no separate dialog needed) | P3 | S |
| DM-12 | SkullConfigDialog — 8 servo enable checkboxes + 16-bit + Skulltronix | ✓ shipped J-30 | P3 | S |
| DM-13 | PositionZoneDialog — 6-column grid (Pan Min/Max, Tilt Min/Max, Channel, Value); collision avoidance | ✓ shipped J-30 — `DmxPositionZoneListEditorSheet`, wired only into MovingHeadAdv | P3 | M |
| DM-15 | ModelChainDialog — chain start channel after another model | partial — `StartChannelEditorSheet` covers the "End of Model" / "Start of Model" modes that the chain dialog is sugar for; a dedicated chain picker UI is open. | P2 | S |
| DM-16 | StartChannelDialog — 5 modes (None / Universe / End of Model / Start of Model / Controller) | ✓ shipped J-30 — `StartChannelEditorSheet` covers all modes, exposed via the pencil shortcut on both model-wide and per-string fields | P2 | M |
| DM-17 | **DMXEffect channel grid** — 48 channels per effect (slider + value curve + invert per channel); Remap Channels button → opens RemapDMXChannelsDialog (O-12); Save State / Load State buttons | ✓ shipped — `DMXChannelsNotebookView` (`EffectCustomRowsFinal.swift:510..645`) with 3 banks × 16 channels, Remap, Save/Load State | P2 | XL |
| DM-18 | MovingHeadEffect — 7 specialised value-curve domains (Pan / Tilt / Fan Pan / Fan Tilt / Pan Offset / Tilt Offset / Groupings / Time Offset / Path Scale) + shared color/wheel; partly covered by Phase C inspector | partial — Phase C inspector + G3+ MovingHead row cover Pan/Tilt/Offset/Groupings/Cycles; the full waypoint path authoring tab is a desktop-only fallback (tracked as G3+ in `followups.md`) | P2 | L (extra curves) |

## View Objects + Wiring view (Phase W)

| # | Item | Status | Severity | Effort |
|---|---|---|---|---|
| VO-1 | ImageObject editing — file + transparency + brightness | iPad has D-7 read-only canvas; full edit controls open | S (add controls) | — |
| VO-2 | GridlinesObject editing — line spacing, width, height, color, axis labels, point-to-front | partial — J-6 covers common properties (centre, size, rotate, locked, layoutGroup); type-specific knobs (line spacing/color/axis-labels/point-to-front) open | M | — |
| VO-3 | MeshObject — OBJ + MTL + textures, brightness, mesh-only flag, per-material color override | partial — J-6 covers common properties; OBJ path round-trips untouched; texture + brightness + mesh-only flag editing open | L | — |
| VO-4 | TerrainObject — heightmap image, parametric size, grid spacing/color, transparency, brightness, brush-paint heightmap edit-in-place | ✓ shipped J-13 — tap-on-canvas heightmap painting with raise/lower toggle, brush radius + cosine falloff, per-edit undo via `pushTerrainHeightmapUndoSnapshot` | XL (paint tool) | — |
| VO-5 | RulerObject — singleton; length + units (m/cm/mm/feet/yards/inches); 2-point line | open | M | — |
| VO-6 | ViewObjectPanel — tree, add/delete/reorder/rename/visibility/multi-select, alignment / distribute / flip / unlink-from-base | partial — sidebar Objects tab + selection + property pane shipped; multi-select / drag-reorder / unlink-from-base open | L | — |
| WV-1 | **Wiring diagram view** — strand-by-strand, Standard (1 px / node) vs MultiLight (RGB), color-coded by string, channel labels, Dark/Gray/Light themes, Front/Rear, 90° rotations, mouse-wheel zoom + pan | open — note: J-32 shipped a *controller* Visualize wiring view (port-mapping diagram, separate concept) but no per-model strand wiring diagram | P2 | L |
| WV-4 | PNG export (standard + large) | open | P2 | S |
| WV-5 | DXF vector export | open | P3 | M |
| WV-6 | Print | open | P3 | M |

## Why the rest is deferred

- Phase J shipped what 90% of users actually reach for (Custom
  Model editor, Faces / States / Dimming Curve, all 8 DMX
  fixtures, terrain painting). What remains is the long tail of
  vendor-specific or low-traffic dialogs.
- 90% of users still acquire stock models from VendorModelDialog
  (catalog browser shipped) or build them on desktop and copy
  the show folder over. The remaining authoring gaps don't
  obviously block that workflow.
- Some pieces (MA-2 GenerateCustomModelDialog video-pixel
  detection, MA-15 MatrixFaceDownloadDialog HTTP catalog,
  WV-1 wiring view) could each justify their own multi-week
  sub-plan once scheduled.

## When to come back

- WV-1 wiring view is the most cited "would help if I'm
  debugging in the field" piece — pull forward if testers ask
  for it.
- MA-15 MatrixFaceDownload completes the J-22 face-authoring
  story; small-effort win once the catalog HTTP cache settles.
- VO-5 RulerObject is the cheapest remaining VO win; the rest
  of the VO multi-select / reorder polish is ergonomics on
  top of the existing Objects tab.
