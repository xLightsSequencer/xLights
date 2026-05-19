# Future — Controller Upload + Pixel Test

The Layout Editor → Controllers tab (J-31 / J-32, 2026-05-16) covers
controller list / edit / discover / wiring-visualize. This file
tracks the **upload** and **test** stack — the Tools-menu entries
that talk to physical controllers (FPP Connect, Bulk Upload,
HinksPix, Pixel Test) plus the heavy port-mapping authoring
dialog and the RemapDMX helper.

The list-management polish that sits on top of the Layout Editor
Controllers tab itself (drag-reorder, sort modes, right-click
Activate/Inactivate, HTTP scan + auth, LED ping) is tracked in
[`phase-j-layout-editor.md`](phase-j-layout-editor.md).

## Priority bump

**FPP Connect (EX-4) is now P1.** External testers are increasingly
asking for one-tap uploads from the iPad so they can iterate on
the field rig without unwrapping a laptop. FPP / WLED /
ESPixelStick are the focus; the rest of the upload stack is parked
at P4 (very low) until that picture changes.

## Upload stack

| # | Item | Severity | Effort |
|---|---|---|---|
| **EX-4** | **FPPConnectDialog** — FPP discovery (Bonjour + UDP); 13-column per-instance config; sequence + media file selection; HTTP REST API uploads. **Tools → FPP Connect** on desktop. | **P1** | XL |
| EX-5 | FPPUploadProgressDialog — per-FPP gauges + cancel | P1 (pairs with EX-4) | S |
| EX-6 | MultiControllerUploadDialog — controller checklist + log + right-click filters; covers Falcon, WLED, PowerDMX, etc. **Tools → Bulk Controller Upload** on desktop. | P4 | M |
| EX-7 | HinksPixExportDialog — vendor-specific HSEQ format, master+2 slaves, playlists, schedule grid, USB drive export. **Tools → HinksPix Export** on desktop. | P4 | XL |
| O-12 | RemapDMXChannelsDialog — From/To/Scale/Offset/Invert grid (48 rows); .xdmxmap CSV load/save | P2 | M |
| O-15 | Print Layout button | P3 | M |

## Pixel Test

| # | Item | Severity | Effort |
|---|---|---|---|
| **O-11** | **PixelTestDialog** — 12 standard tests (Off, Chase, Chase 1/3..1/5, Alternate, Twinkle 5/10/25/50%, Shimmer, Background); per-RGB tabs; 4 selection trees (Outputs / ModelGroups / Models / Controllers) cascade checkboxes; speed/highlight/background sliders; Save/Load presets; embedded preview; ChannelTracker overlap-merge. **Tools → Test** on desktop. | P2 | XL |

Pixel Test is the field debugger that pairs with FPP Connect: after
uploading a sequence, the user wants to confirm each strand lights
up correctly before committing the show.

## Port-mapping authoring

| # | Item | Severity | Effort |
|---|---|---|---|
| O-10 | **ControllerModelDialog** drag-drop authoring — extends J-32's read-only Visualize: per-port String/DMX/Virtual Matrix, per-port protocol, per-port brightness/gamma/null pixels/colour order/group count, smart-remote A–F, auto-layout flag, bank visualisation, Print + XLSX export with smart-remote colour coding, right-click context menus, validation warnings | P4 | XL |

Also tracked alongside the strand-level wiring diagram (WV-1) in
[`future-custom-models.md`](future-custom-models.md) since both
live under "wiring authoring."

## Why deferred (apart from FPP Connect)

- Live output (lightbulb toggle) covers the most common tester
  ask — "play the sequence to my actual lights" — without needing
  any of the upload stack.
- Bulk upload / HinksPix / drag-drop port mapping are each
  multi-week sprints with no concrete user request, and they sit
  outside the FPP / WLED / ESPixelStick focus.

## When to come back

- **Now-ish:** FPP Connect (EX-4 + EX-5). External testers keep
  asking; one-tap uploads close the iPad-as-only-device story.
- After that, follow tester reports. Pixel Test (O-11) is the
  natural next pull-in once FPP Connect ships and testers want
  to verify uploads on-device.
- The P4 items (EX-6 Bulk Upload, EX-7 HinksPix, O-10 drag-drop
  port mapping) are parked indefinitely; revisit only if the
  scope of supported controllers changes.
