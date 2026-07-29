# Bulk Controller Upload — status icons design

GitHub issue: https://github.com/xLightsSequencer/xLights/issues/6760

## Problem

`MultiControllerUploadDialog` ("Bulk Controller Upload") uploads configuration to a
checked list of controllers one at a time. Progress is only visible as scrolling text
in the log `wxTextCtrl` at the bottom of the dialog, which the user has to scroll back
through to find out which controller(s) failed. There is also no indication, before
starting an upload, of which controllers are currently online/offline — the same
information the Controllers tab already shows.

## Goals

- Show each controller's online/offline ping status in the row, between the checkbox
  and the controller name — matching what the Controllers tab already shows.
- Show a per-row upload-result icon (green check / red X / yellow "processing") to the
  right of the controller name, updated live as the dialog's existing one-by-one
  upload loop runs, so failures are visible without scrolling the log.

## Non-goals

- No change to the upload mechanism itself: it stays synchronous, one controller at a
  time, on the UI thread. No worker thread/async upload is introduced.
- No change to the log `wxTextCtrl` at the bottom.
- No change to which controllers are eligible for bulk upload, or to the persisted
  checked-selection behavior beyond adapting it to the new control's API.

## Control replacement

`CheckListBox_Controllers` is currently a plain `wxCheckListBox`, which can only show a
checkbox and a line of text — no per-row icons are possible with it. It is replaced
with a `wxListCtrl` in report mode (`wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL`)
with `EnableCheckBoxes(true)`.

This was chosen over two alternatives:
- **`wxDataViewListCtrl`** (toggle renderer + bitmap renderer + text columns) — more
  "correct" in the abstract, but nothing in this codebase currently combines a toggle
  column with bitmap columns this way; it would be a new pattern with its own
  platform quirks, for no benefit over the `wxListCtrl` approach.
- **Keep `wxCheckListBox`, fake icons with colored Unicode glyphs** in the item text
  (🟢/🔴 prefix, ✓/✗ suffix) — avoids a control swap, but glyph rendering is
  font/platform-dependent (risk of inconsistent look across Windows/macOS/Linux),
  can't reuse the existing procedurally-drawn LED bitmaps, and can't align into a
  fixed-width column the way a dedicated image column can.

Verified directly in the vendored wxWidgets 3.3.3 source
(`wxWidgets-xlights_2026.07/src/generic/listctrl.cpp`) that `wxListCtrl`'s checkbox is
drawn *before* column 0's icon+text, in exactly the required order:
`[checkbox] [icon] [name]`. This matches the request to place the status icon "between
the checkbox and the controller name" without needing a third column.

`SetItemColumnImage(item, column, image)` (confirmed present in
`wxWidgets-xlights_2026.07/include/wx/generic/listctrl.h` and already used elsewhere in
this codebase, e.g. `xLightsImportChannelMapDialog.cpp`) is used to set the icon in
column 1 for the upload-result icon.

## Layout

Two columns, no header (`wxLC_NO_HEADER`):
- **Column 0**: status LED icon (via image list) + controller display text — the same
  string built today (`"<IP> [(via FPP <proxy>)] <Description> <Name>"`).
- **Column 1**: upload-result icon only, narrow fixed width, no text.

One shared `wxImageList`, all icons drawn at a uniform 16×16 size. All images going
into a single `wxListCtrl` image list must be the same size — mixing sizes previously
caused macOS to normalize every image up to the largest one in the list (see prior
Import dialog work: separate 25px icon column + name column was needed there for the
same reason). Keeping every icon here at a uniform 16×16 avoids that problem outright.

## Icon set

**Ping status** (column 0), reusing existing logic:
- `ControllerTree::ClassifyControllerPing(const Controller*)` classifies a controller
  into green / red / gray, exactly as the Controllers tab does.
- A same-size (16×16) variant of the existing `ControllerTree::CreateLedBitmap()` is
  used to draw the dot, so the visual language matches the Controllers tab.

**Upload result** (column 1), new: three same-size (16×16) procedurally-drawn icons —
green check, red X, yellow/orange dot for "processing" — plus a blank/no-image state
for rows not yet part of the current run.

## Ping status behavior

- On dialog open: one explicit refresh pass, pinging just the controllers listed in
  this dialog (reusing existing `Controller::Ping()`/`AsyncPing()`), so the LED
  reflects current state rather than a value that may be stale from whenever the
  Controllers tab was last active.
- No dedicated recurring timer is added by this dialog.
- If the app's existing Controllers-tab ping timer happens to update a controller's
  ping state in the background while this dialog is open (which today only happens
  when the Controllers tab is the active page underneath), that update is picked up
  and reflected live in the dialog's row — including while an upload run is in
  progress. No special-case freezing/suppression logic is added for "during upload,"
  since this dialog never issues its own repeat ping calls after the initial one.

## Upload-result behavior

The existing synchronous per-controller loop in `OnButton_UploadClick`
(`MultiControllerUploadDialog.cpp`) gains, per iteration:
1. Set that row's column-1 icon to "processing" (yellow) and force a repaint
   (`Refresh()`/`Update()` on the list) before the blocking
   `UploadInputToController`/`UploadOutputToController` calls run.
2. After both calls return, set the icon to green (success) or red (failure) based on
   the result.

Unchecked rows never get a result icon. A row's result icon from a previous run is
left as-is until that row is actually reprocessed in a subsequent run — so re-running
"Upload" only overwrites rows that are still checked; a row unchecked before a rerun
keeps showing its result from the previous run.

## Preserved behavior

- Checked-state persistence (`SaveChecked()`/`LoadChecked()`, storing selected IPs
  under the `MultiControllerUploadSelection` config key) continues to work, adapted
  from `wxCheckListBox::IsChecked()` calls to the `wxListCtrl` checkbox API
  (`CheckItem()`/`IsItemChecked()`).
- The right-click context menu (Select All / None / Active / Auto Config / Deselect
  Inactive / select-by-FPP-proxy submenu) continues to work against the new control.
- ESC-to-cancel behavior during the upload loop is unchanged.

## wxSmith sync

The control swap touches wxSmith-guarded declarations in
`MultiControllerUploadDialog.h`/`.cpp` (the `//(*Headers...*)`, `//(*Declarations...*)`,
`//(*Identifiers...*)`, and `//(*Initialize...*)` blocks). The corresponding
`src-ui-wx/wxsmith/MultiControllerUploadDialog.wxs` must be hand-edited to match, or
wxSmith will silently discard the new control next time the file is opened in the
wxSmith designer.

## Testing

- Manual: open Bulk Controller Upload with a mix of online/offline/unreachable
  controllers, confirm LED colors match the Controllers tab for the same controllers.
- Manual: run an upload against a mix of controllers expected to succeed and fail
  (e.g. one with a bad IP), confirm yellow appears before each controller's network
  calls, then green/red afterward, and that unchecked rows stay blank.
- Manual: rerun after unchecking a previously-failed row, confirm it keeps its old red
  icon while newly-processed rows update.
- Manual: verify checked-selection persistence still saves/restores across a dialog
  close/reopen, and that the right-click context menu options still work.
