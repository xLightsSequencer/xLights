# Future — Controllers Tab on iPad

The single biggest desktop subsystem the iPad doesn't surface.
Engine is already in `src-core/` (wx-free, links into the iPad
library); the entire gap is UI.

Source: §2.1 of the 2026-04-23 gap analysis.

## What's already in place

- All 34 output protocols (E1.31, ArtNet, DDP, OPC, ZCPP, KiNet,
  Twinkly, OpenDMX, uDMX, Renard, LOR / LOROptimised, OpenPixelNet,
  …) compiled into the iPad lib.
- All 18 vendor controller handlers (Falcon F4–F96, FPP, WLED,
  Pixlite16, HinksPix, AlphaPix, Minleon, J1Sys, SanDevices,
  ESPixelStick, Experience, ILightThat, PowerDMX) compiled in.
- `iPadRenderContext` owns an `OutputManager`. Live output toggle
  shipped (lightbulb in the sequencer toolbar).
- **J-31 (2026-05-16) — Controllers tab inside the Layout Editor.**
  Sidebar roster of controllers with filter; Add Ethernet / Serial /
  Null actions; Discover sheet (Bonjour + broadcast); per-controller
  property pane via the descriptor pipeline; swipe-to-delete.
- **J-32 (2026-05-16) — Controllers Visualize (wiring view).**
  Read-only port-mapping diagram inside the Controllers tab —
  the iPad-side counterpart to desktop's ControllerModelDialog
  visualize button (port mapping only; the desktop dialog's
  drag-drop authoring path is not exposed).

## Gap (still open)

### Phase R-min — Basic discovery + edit + drive lights

| # | Item | Status | Severity | Effort |
|---|---|---|---|---|
| O-2 | Show-folder section UI extras — Recent show folders list (`L-1b` in [`followups.md`](followups.md)) and base-directory toggle (`L-10`). Path display + change already shipped via `FolderConfigView` (the `folder.badge.gearshape` toolbar button on the sequence picker). | open | P2 | S |
| O-3 | Controller list — 13 columns (Name/Protocol/Address/Universes/Channels/Vendor/Model/Variant/Active/AutoLayout/AutoSize/Description/Status), drag-reorder, multi-select, sort by 6 fields, status LED | ✓ shipped J-31 — sidebar list with filter + per-row status. Drag-reorder + sort-by-6-fields + multi-select still open. | P1 | M |
| O-4 | Toolbar — Add Ethernet / Serial / Null / Discover / FPP Connect / Save / Delete All | ✓ shipped J-31 — Add Ethernet / Serial / Null + Discover live; FPP Connect + Delete All open. | P1 | M |
| O-5 | Per-controller right-click — Insert Ethernet/Serial/Renard/LOR/DMX/NULL, Activate, Activate xLights Only, Inactivate, Delete, Unlink, Upload Output, Sort submenu (6 modes) | partial — swipe-to-delete only. Activate / Inactivate / Unlink / Upload / Sort still open. | P1 | M |
| O-6 | Controller property grid — 20–40 props per type (Ethernet vs Serial vs Null differ; full list in gap analysis) | ✓ shipped J-31 — descriptor-pipeline property pane covers the type-conditional grids. | P1 | L |
| O-7 | Discover sheet — Bonjour (FPP), broadcast (ArtNet, DDP), HTTP scan (Falcon, Pixlite16, Twinkly); 3-way conflict dialogs; DiscoveryAuthDialog credentials | partial — `runControllerDiscovery` + mismatch-resolution sheet (ip-update / rename / skip) shipped J-31. HTTP scan + DiscoveryAuthDialog credentials still open. | P1 | L |
| O-8 | ControllerConnectionDialog (legacy add wizard) | open | P2 | S |
| O-9 | IPEntryDialog (IP entry helper) | open | P2 | S |
| O-13 | Output-to-Lights status / fault notifications when a controller drops | open | P2 | M |
| O-16 | LED status column / async ping thread per controller | open | P2 | S |

### Phase R-pro — Heavy controller dialogs (separate)

| # | Item | Status | Severity | Effort |
|---|---|---|---|---|
| O-10 | **ControllerModelDialog** — port-mapping diagram (~4795 lines on desktop), drag-drop layout of models onto pixel/serial/virtual ports; per-port String/DMX/Virtual Matrix; per-port protocol; per-port brightness/gamma/null pixels/colour order/group count; smart-remote A–F (cascade-down-port toggle); auto-layout flag; bank visualisation; Print + XLSX export with smart-remote colour coding; right-click context menus; validation warnings | partial — J-32 shipped the read-only port-mapping Visualize view. Drag-drop authoring, per-port protocol / brightness / smart-remote, Print/XLSX export, validation warnings all still open. | P2 | XL |
| O-11 | **PixelTestDialog** — 12 standard tests (Off, Chase, Chase 1/3..1/5, Alternate, Twinkle 5/10/25/50%, Shimmer, Background); per-RGB tabs; 4 selection trees (Outputs / ModelGroups / Models / Controllers) cascade checkboxes; speed/highlight/background sliders; Save/Load presets; embedded preview; ChannelTracker overlap-merge | open | P2 | XL |
| O-12 | RemapDMXChannelsDialog — From/To/Scale/Offset/Invert grid (48 rows); .xdmxmap CSV load/save | open | P2 | M |
| O-14 | Visualise button → opens O-10 | ✓ shipped J-32 (read-only) | P2 | (= O-10) |
| O-15 | Print Layout button | open | P3 | M |

### Controller upload (separate phase, depends on R-min)

| # | Item | Status | Severity | Effort |
|---|---|---|---|---|
| EX-4 | **FPPConnectDialog** — FPP discovery (Bonjour + UDP); 13-column per-instance config; sequence + media file selection; HTTP REST API uploads | open | P2 | XL |
| EX-5 | FPPUploadProgressDialog — per-FPP gauges + cancel | open | P2 | S |
| EX-6 | MultiControllerUploadDialog — controller checklist + log + right-click filters; covers Falcon, WLED, PowerDMX, etc. | open | P2 | M |
| EX-7 | HinksPixExportDialog — vendor-specific HSEQ format, master+2 slaves, playlists, schedule grid, USB drive export | open | P3 | XL |

## Why deferred

- R-min has largely shipped via J-31/J-32 — the remaining holes
  (drag-reorder, sort modes, FPP Connect, Activate/Inactivate
  context menu) are ergonomics on top of a working baseline.
- R-pro Pixel Test (O-11) + the controller upload stack (EX-4..7)
  are each multi-week sprints; nobody has asked for upload-from-
  iPad yet.
- Live output (lightbulb toggle) covers the most common tester
  ask — "play the sequence to my actual lights".

## When to come back

- The R-min polish set (drag-reorder, sort modes, right-click
  Activate/Inactivate/Unlink, LED ping thread) is a natural
  small sprint once a tester complains about list management.
- FPP Connect is the next big leap; pull it in once external
  testers want one-tap uploads from the iPad.
