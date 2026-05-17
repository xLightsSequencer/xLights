# Future — Custom Model + Face / State + DMX Authoring

Model-authoring stack. Phase J landed the bulk of this:

- **J-23** — Custom Model visual editor + SubModel geometry editor.
- **J-22** — Faces / States / Dimming Curve / SubModels editors.
- **J-30** — DMX deep authoring for all 8 fixture types
  (preset / wheel-colour / position-zone / skull / mesh-servo).
- **J-13** — TerrainObject heightmap painting.

What remains is the long tail of vendor-specific or low-traffic
dialogs.

Source: §2.4 + §2.5 + §2.7 + §2.6 of the 2026-04-23 gap analysis
(Phases T + U + V + W).

## Custom Model authoring (Phase T)

| # | Dialog | Severity | Effort |
|---|---|---|---|
| MA-1 | **CustomModelDialog** advanced ops on top of the J-23 core (compress / trim / wire-cells / auto-number / import-from-controller — the full 40-op surface) | P2 | XL |
| MA-2 | **GenerateCustomModelDialog** — 6-step wizard (Prepare → Choose Media → Start Frame → Manual Identify → Bulb Identify with sensitivity/blur/despeckle/contrast/gamma/saturation → Custom Model) | P3 | XL |
| MA-3 | **ModelRemap** — load original + new wiring → generate remap | P3 | S |
| MA-4 | **SubModelsDialog** — sub-buffer panel + vertical buffer details (list CRUD + node-ranges + geometry shipped J-22/J-23.3) | P2 | M |
| MA-5 | SubModelGenerateDialog — base name + type + count → auto-generate N sub-models | P3 | S |
| MA-6 | EditSubmodelAliasesDialog — alias listbox + add/delete/move | P3 | S |
| MA-7 | NodeSelectGrid — general-purpose dialog (used inside J-22 face/state node selection today) | P2 | M |
| MA-8 | ChannelLayoutDialog — HTML window with channel breakdown table; Print + View in Browser | P3 | S |

## Face / State / Aliases authoring (Phase U)

| # | Dialog | Severity | Effort |
|---|---|---|---|
| MA-12 | EditAliasesDialog — standalone aliases-list editor (add-as-alias shipped 2026-05-17) | P3 | S |
| MA-13 | StrandNodeNamesDialog — 2-column grids (strand names / node names); conditional Generate Node Names button (DMX models only) | P3 | M |
| MA-14 | SevenSegmentDialog — 6 segment checkboxes (Thousands / Hundreds / Colon / Tens / Decimal / Ones) + reference image | P3 | S |
| MA-15 | **MatrixFaceDownloadDialog** — tree navigator (categories / artists), search, image preview (256×128), face details, Insert Face. HTTP catalog from `nutcracker123.com/xlights/faces/xlights_faces.xml`; ZIP download with per-phoneme PNGs | P3 | XL |

## DMX models deep authoring (Phase V) — remaining gaps

J-30 landed per-type panes for all 8 DMX fixture types plus the
heavy preset / wheel-colour / position-zone / skull / mesh-servo
list editors. Outstanding:

| # | Item | Severity | Effort |
|---|---|---|---|
| DM-2 | PWM brightness / gamma on advanced fixtures (controller-caps wiring) | P2 | M |
| DM-8 | DmxSkull mesh paths (per-servo blocks + Skulltronix preset shipped) | P3 | M |
| DM-9 | PWM brightness / gamma on color-ability subsystems | P2 | M |
| DM-15 | ModelChainDialog — dedicated chain picker UI (StartChannelEditorSheet covers "End of Model" / "Start of Model" modes today) | P2 | S |
| DM-18 | MovingHeadEffect full waypoint path authoring tab | P2 | L |

## View Objects + Wiring view (Phase W)

| # | Item | Severity | Effort |
|---|---|---|---|
| VO-1 | ImageObject editing — file + transparency + brightness controls on top of the read-only canvas | — | S |
| VO-2 | GridlinesObject type-specific knobs (line spacing/color/axis-labels/point-to-front) — common properties shipped J-6 | — | M |
| VO-3 | MeshObject — texture + brightness + mesh-only flag editing (common properties + OBJ round-trip shipped J-6) | — | L |
| VO-5 | RulerObject — singleton; length + units (m/cm/mm/feet/yards/inches); 2-point line | P2 | M |
| VO-6 | ViewObjectPanel — multi-select / drag-reorder / unlink-from-base (sidebar Objects tab + selection + property pane shipped) | — | L |
| WV-1 | **Wiring diagram view** — per-model strand wiring (J-32 shipped *controller* port-mapping Visualize, separate concept): strand-by-strand, Standard (1 px / node) vs MultiLight (RGB), color-coded by string, channel labels, Dark/Gray/Light themes, Front/Rear, 90° rotations, mouse-wheel zoom + pan | P2 | L |
| WV-4 | PNG export (standard + large) | P2 | S |
| WV-5 | DXF vector export | P3 | M |
| WV-6 | Print | P3 | M |
| O-10 | **ControllerModelDialog drag-drop authoring** — extends J-32's read-only Visualize: per-port String/DMX/Virtual Matrix, per-port protocol, per-port brightness/gamma/null pixels/colour order/group count, smart-remote A–F, auto-layout flag, bank visualisation, Print + XLSX export with smart-remote colour coding, right-click context menus, validation warnings. Also tracked in [`future-controller-upload.md`](future-controller-upload.md). | P2 | XL |

## When to come back

- WV-1 wiring view is the most cited "would help if I'm
  debugging in the field" piece — pull forward if testers ask
  for it.
- MA-15 MatrixFaceDownload completes the J-22 face-authoring
  story; small-effort win once the catalog HTTP cache settles.
- VO-5 RulerObject is the cheapest remaining VO win.
