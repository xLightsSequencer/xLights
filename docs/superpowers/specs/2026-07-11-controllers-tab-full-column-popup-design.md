# Controllers Tab Double-Click Full-Column Popup — Design

**Date:** 2026-07-11
**Branch:** `mergeControllerTab`
**Scope:** New read-only popup dialog reachable from the Layout tab's "Groups/Models List" notebook (Models / Groups / Controllers / 3D Objects tab strip, `Notebook_Objects` in `LayoutPanel`). No changes to the docked `ControllerListPanel` itself.

## Problem

The Controllers page inside the Layout tab notebook is narrow (it shares the panel with the big layout/model preview), but the controller list has ~17 defined columns (see `2026-07-11` column-additions work). Only a handful fit at once, and users must scroll or toggle column visibility to see the rest. Switching to a full-width view means giving up the layout preview.

## Trigger

Double-clicking the **"Controllers" tab label itself** (in the `Notebook_Objects` tab strip, not a row inside the panel) opens a modal dialog showing every defined column, for every controller, at once — without navigating away from the layout preview.

- `Notebook_Objects` is a plain `wxNotebook` (`LayoutPanel.h:130`).
- Bind `wxEVT_LEFT_DCLICK` on `Notebook_Objects` in the manual (non-wxSmith) code immediately after the tabs are inserted, near `LayoutPanel.cpp:704-708` — this is outside the wxSmith guard block, so no `.wxs` sync is required.
- Handler `LayoutPanel::OnNotebookTabDClick(wxMouseEvent&)`: call `Notebook_Objects->HitTest(evt.GetPosition(), &flags)` to get the hit page index, compare against `FindNotebookPage("Controllers")` (existing helper, `LayoutPanel.cpp:12516`). If they match, construct and `ShowModal()` the new dialog. If not, ignore (event still propagates normally for tab switching).

## Dialog

New class, new files `src-ui-wx/layout/ControllerFullColumnsDialog.h/.cpp`:

- A single `wxTreeListCtrl`, built the same way `ControllerListPanel::CreateTree()`/`PopulatePorts()` populate rows today (same icons, same top-level-controller + port-child structure), sourced from the same `OutputManager`/`Controller` data — no new data-fetching code.
- **All** columns from a new curated ordering (see below) are added and left visible; there is no column-visibility menu, no `TreeListColumnVisibility` persistence, and no user column reordering — this view always shows everything, always in the same order.
- **Read-only**: no context menu, no drag/drop, no delete/activate/deactivate, no multi-select bulk actions. Row selection/expand-collapse work as normal tree behavior but do nothing beyond that (no properties pane, no preview highlighting).
- No filter/search box — bare list only.
- Modal (`wxDialog`, `ShowModal()`), single "Close" button. Sized generously (~90% of the main frame) since the goal is maximum visible information; no OK/Cancel distinction since nothing is editable.

## Column ordering

Each controller type (`ControllerEthernetPropertyAdapter`, Serial, Null) builds its "define/edit controller" property grid dynamically and with a different property set per type (e.g. `ControllerEthernetPropertyAdapter::AddProperties` appends `Multicast`, `IP`, `FPPProxy`, `Protocol`, ... — Serial's set differs). There is no single literal grid order common to every controller, so this dialog does not mirror any one adapter's live order.

Instead: a new fixed array of the existing stable column IDs (from the `CTRL_COLS` struct-based refactor in `ControllerListPanel.cpp`), hand-ordered to match the logical grouping a user encounters when defining a controller — identity/description first, then addressing, then upload/behavior settings, then everything else:

1. Name, Active, Vendor, Model, Variant, Description
2. Address / IP, Protocol
3. Auto Upload, Proxy, Keep Channels, Port Brightness, Port Gamma
4. Remaining columns (ports, channels, status, etc.) in their current `CTRL_COLS` order

This order is exclusive to this dialog. The docked `ControllerListPanel` keeps its existing alphabetical/user-toggleable column order untouched.

## Out of scope

- Any change to the docked `ControllerListPanel`'s columns, order, or behavior.
- Editing of any kind from this dialog — it is a reference view only.
- Persisting dialog size/position across app restarts — fixed initial size (~90% of the main frame) each time it opens, no persistence.
- A per-controller "define new controller"-style vertical property grid popup (considered and rejected in favor of the full multi-controller wide-list view).

## Testing / verification

1. Build: `xcodebuild -configuration Debug` (and NO_PCH variant per project convention).
2. Double-click the "Controllers" tab label → dialog opens with every column visible for every controller, in the curated order, regardless of the docked panel's current column visibility/order state.
3. Double-click the "Models", "Groups", or "3D Objects" tab labels → no dialog opens; normal tab-switch behavior is unaffected.
4. Single-click / normal tab switching to Controllers still works as before (double-click handling doesn't interfere with `OnNotebook_ObjectsPageChanged`).
5. Confirm no context menu, drag, or edit action is reachable from the popup's tree.
6. Confirm docked panel's column order/visibility is unchanged after opening and closing the popup.
