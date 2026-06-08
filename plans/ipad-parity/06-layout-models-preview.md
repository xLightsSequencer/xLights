# 06 · Layout: Models, Groups, Preview, 3D, Submodels/Faces/States/DMX

> The iPad Layout Editor has full model/group CRUD, descriptor-driven
> property editing, multi-select **Align / Distribute / Match-Size /
> Flip / Duplicate / Group**, a complete **Model Data** section
> (SubModels, **Faces, States, Aliases, Strands, Nodes, Groups, Dimming
> Curve** editors — all wired to the bridge), a custom-model grid editor
> with point-distribution, polyline create + per-segment context menu,
> the full DMX model family (MovingHead/Adv, Servo/3D, Skull,
> Floodlight, General) with descriptor properties + DMX
> preset/wheel/position-zone editors, view-object CRUD, a 2D background
> object, and an entire Controllers sidebar tab (add / discover /
> visualize / upload / reorder). It also has full **viewpoint / saved-
> camera** save/apply/delete/restore over `GetViewpointMgr()`. The
> remaining real iPad gaps are the desktop's *rich visual* dialogs and
> bulk/utility commands: **Faces/States are attribute-map editors on
> iPad, not the desktop's node-grid / image face picker**; **Node Layout
> / Wiring View, CAD/DXF export, export-as-custom/xmodel,
> import-from-RGBeffects / LOR-S5, GDTF fixture import, make-start-
> channel-valid / non-overlapping, correct-aspect-ratio, group clone,
> model replace, set-center-offset, the dedicated Bulk-Edit submenu
> (incl. dimming curves / controller direction), unlink model/group from
> base show, 2D-center-0 + canvas-size + global grid-spacing edits, and
> layout clipboard copy/paste** are all desktop-only. iPad-only
> conveniences for this theme: the vendor model browser, map-from-lights
> (FPP camera scan), single-tap multi-select action bar, view-object
> rename/duplicate, and the integrated Controllers sidebar. Faces/States
> DMX save/load lives in the sequencer DMX panel on both. Controller
> create/discover/visualize/upload exist on **both** platforms — on
> desktop they live in the Controllers/Setup tab, on iPad inside the
> Layout sidebar — so they are parity, with closed-firmware uploads
> remaining IAP-gated/restricted.

## Parity scorecard

| Feature | Surface | Desktop | iPad | Gap | Priority | Ease | Feasibility | Notes |
|---|---|---|---|---|---|---|---|---|
| Model create (type picker) | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop: Add-model buttons. iPad: AddModelSheet (availableModelTypesForCreation). |
| Model select (single/multi/marquee) | gesture | ✅ | ✅ | parity | P1 | easy | feasible | iPad: tap + Select mode + drag marquee; desktop click+shift/ctrl+rubber-band. |
| Model drag-to-move (2D/3D) | gesture | ✅ | ✅ | parity | P1 | easy | feasible | iPad Move tool + axis lock. |
| Model rotate X/Y/Z | gesture/panel | ✅ | ✅ | parity | P1 | medium | feasible | iPad: Rotate tool gizmo + Rotate X/Y/Z numeric fields. UI idiom differs (no grab-handle hover). |
| Move/Rotate/Scale tool picker + axis lock | toolbar | 🟡 | ✅ | desktop-missing | P2 | medium | feasible | iPad LayoutEditorToolToolbar (touch idiom). Desktop uses centre-cycle + direct gizmo handles. |
| 3D transform gizmo with axis tools (move/scale/rotate/elevate) | gesture | ✅ | 🟡 | ipad-weaker | P2 | hard | feasible | iPad ray-cast 3D drag follows gizmo + cycleAxisTool (XLMetalBridge.h:246-247, :276); handle set narrower than desktop's per-axis grab handles. |
| Model position/size property fields (X/Y/Z/W/H/D/rot) | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad descriptor-driven, branches by ScreenLocation kind (two/three-point endpoints vs boxed). |
| Model lock / unlock | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad lockedBinding toggle + desktop menu/keybinding. |
| Model description / memo | panel | ✅ | ✅ | parity | P2 | easy | feasible | Both editable. |
| Model rename | dialog | ✅ | ✅ | parity | P1 | easy | feasible | iPad renameModel + rename sheet. |
| Model duplicate | toolbar/menu | 🟡 | ✅ | parity | P2 | medium | feasible | iPad explicit Duplicate (XLMetalBridge.duplicateModels, +offset). Desktop has Copy/Paste, no 1-click Duplicate. |
| Model delete | menu | ✅ | ✅ | parity | P1 | easy | feasible | iPad swipe/trash + confirm. |
| Model clipboard copy/cut/paste (layout) | shortcut | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop DoCopy/DoPaste (CopyPasteBaseObject XML). iPad uses Duplicate instead; no UIPasteboard model copy. |
| Model keyboard nudge (arrows ±) | shortcut | ✅ | ✅ | parity | P2 | easy | feasible | Desktop Nudge(); iPad arrow-key nudge + per-step undo. |
| Per-type property grid (all model types) | panel | ✅ | ✅ | parity | P1 | medium | feasible | iPad descriptor pane mirrors desktop adapters per type. |
| Model tag color | panel | ✅ | ✅ | parity | P2 | easy | feasible | tagColorPicker. |
| Model active/inactive | panel | ✅ | ✅ | parity | P1 | easy | feasible | boolBinding "active". |
| Model controller-port connection | panel | ✅ | ✅ | parity | P1 | medium | feasible | iPad controllerConnectionFields (port/protocol/smart-remote gated by caps). |
| Model start channel picker | dialog | ✅ | ✅ | parity | P1 | medium | feasible | iPad StartChannelEditorSheet. |
| Individual start channels per string | panel | ✅ | ✅ | parity | P3 | medium | feasible | iPad hasIndividualStartChannels toggle + per-string individualStartChannel fields (LayoutEditorView.swift:3973-3990). |
| Node Layout (node-grid visualization) | dialog | ✅ | ❌ | ipad-missing | P2 | hard | feasible | Desktop ShowNodeLayout/NodeSelectGrid (LayoutPanel.cpp:6156). iPad has no node-grid view. |
| Wiring View (model node wiring) | dialog | ✅ | ❌ | ipad-missing | P2 | hard | feasible | Desktop ShowWiring (LayoutPanel.cpp:6159). iPad's "Visualize" is controller-level, not model wiring. |
| Pixel size / style | panel | ✅ | ✅ | parity | P2 | easy | feasible | intField pixelSize + pixelStylePicker. |
| Transparency / alpha | panel | ✅ | ✅ | parity | P2 | easy | feasible | intField transparency. |
| Black transparency | panel | ✅ | ✅ | parity | P3 | easy | feasible | iPad intField "blackTransparency" (Appearance section). |
| Shadow Model For | panel | ✅ | ✅ | parity | P3 | easy | feasible | iPad shadowModelPicker (LayoutEditorView.swift:4856). |
| Smart remote (per-port remote) | panel | ✅ | ✅ | parity | P2 | medium | feasible | iPad ModelSmartRemoteSheet + descriptor caps. |
| Correct Aspect Ratio | menu | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop ID_PREVIEW_MODEL_ASPECTRATIO. iPad: no UI/bridge. |
| SubModels create/edit/rename/delete | dialog | ✅ | ✅ | parity | P1 | medium | feasible | iPad SubModelListSheet (add/delete/rename + detail edit via replaceSubModelsOnModel). |
| SubModel aliases | dialog | ✅ | ✅ | parity | P3 | medium | feasible | iPad setSubmodelAliases / submodelAliases + alias editor (LayoutEditorView.swift:6704). |
| Faces editor | dialog | ✅ | 🟡 | ipad-missing | P2 | hard | feasible | iPad FaceStateEditorSheet edits faceInfo as attr key/value maps. Desktop ModelFaceDialog is a rich node-grid + image-matrix face picker, and also imports faces/states from xmodel files (ModelFaceDialog::ImportFaces handles both new `<models><model>` and legacy `<custommodel>` roots, #6499); iPad has no file-import path. iPad lacks the visual node assignment. |
| States editor | dialog | ✅ | 🟡 | ipad-missing | P2 | hard | feasible | Same as Faces: iPad attr-map editor vs desktop ModelStateDialog node-grid UI. |
| Custom model visual editor | dialog | ✅ | ✅ | parity | P2 | medium | feasible | iPad CustomModelEditorSheet (grid + paint + Distribute-along-line). setCustomModelData bridge. Node-bounding-box centering (CustomModel.cpp InitCustomMatrix, #6452) is shared core — applies to iPad automatically. |
| Strand / Node names editor | dialog | ✅ | ✅ | parity | P2 | medium | feasible | iPad IndexedNamesEditorSheet (setStrandNames/setNodeNames + DMX generateNodeNames). |
| Per-model aliases editor | dialog | ✅ | ✅ | parity | P3 | medium | feasible | iPad AliasEditorSheet (setModelAliases). |
| Model dimming curve | dialog | ✅ | 🟡 | ipad-missing | P3 | hard | feasible | iPad edits dimmingInfo attr-map + can CLEAR; cannot author a new curve graphically (sheet warns). Desktop ModelDimmingCurveDialog full editor. |
| Model group create | menu | ✅ | ✅ | parity | P1 | medium | feasible | iPad createModelGroup + NewGroupSheet (also from multi-select). |
| Model group rename | dialog | ✅ | ✅ | parity | P1 | medium | feasible | iPad renameModelGroup + sheet. |
| Model group delete | menu | ✅ | ✅ | parity | P1 | medium | feasible | iPad deleteModelGroup + confirm. |
| Model group add members | dialog | ✅ | ✅ | parity | P1 | medium | feasible | iPad addModel:toGroup: + LayoutGroupPickerSheet / Add-to-Group. |
| Model group remove members | menu | ✅ | ✅ | parity | P2 | medium | feasible | iPad removeModel:fromGroup: via group property pane onRemoveMember. |
| Model group reorder members | panel | ✅ | ✅ | parity | P2 | medium | feasible | iPad onReorderMembers (commit "members"). |
| Model group properties (layout style/grid/tag) | panel | ✅ | ✅ | parity | P1 | medium | feasible | iPad LayoutEditorGroupPropertiesView descriptor. |
| Model group clone | menu | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop ID_MNU_CLONE_MODEL_GROUP. iPad: no clone bridge/UI. |
| Multi-select Align (8 + ground) | menu/toolbar | ✅ | ✅ | parity | P1 | medium | feasible | iPad MultiSelectActionBar Align menu (left/right/top/bottom/front/back/centerH/V/D/ground). |
| Multi-select Distribute (H/V/Depth) | menu/toolbar | ✅ | ✅ | parity | P1 | medium | feasible | iPad Distribute menu (>=3 selected). |
| Multi-select Match Size (W/H/Depth/All) | menu/toolbar | ✅ | ✅ | parity | P2 | medium | feasible | iPad Match-Size menu (XLMetalBridge.h:528 matchSizeOfModels:toLeader:dimension:). |
| Multi-select Flip H/V | menu/toolbar | ✅ | ✅ | parity | P2 | medium | feasible | iPad Flip menu (flipModels). |
| Swap Start/End (Single/Poly Line) | menu | ✅ | ✅ | parity | P2 | easy | feasible | Desktop #6483 ID_PREVIEW_SWAP_START_END (LayoutPanel.cpp). Core Model::SwapStartEnd() shared (PolyLineModel/SingleLineModel/PolyPointScreenLocation/TwoPointScreenLocation). iPad: bridge `swapStartEndForModels:forDocument:` (XLMetalBridge.mm) + ⇄ button in InlineModelActionBar (LayoutEditorView.swift, shown only for Single Line/Poly Line, gated on lock/from-base) → performSwapStartEnd, pushes layout undo. |
| Property-pane bulk edit across selection | panel | ✅ | ✅ | parity | P2 | medium | feasible | iPad J-4 applies a property-pane edit to all selected (bulkEditTargets, model-unique keys excluded). Desktop has dedicated Bulk-Edit submenu. |
| Bulk Edit: controller direction / colour-order / null-nodes / group-count | menu | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop mnuBulkEdit specific items. iPad property pane doesn't expose these per-key bulk ops. |
| Bulk Edit: dimming curves | dialog | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop ID_PREVIEW_BULKEDIT_DIMMINGCURVES. No iPad dimming authoring. |
| Replace model(s) with this model | menu | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop ID_PREVIEW_REPLACEMODEL. iPad: no UI. |
| Export as Custom / 3D Custom xLights model | menu | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop ID_PREVIEW_MODEL_EXPORTASCUSTOM(3D). iPad: import works, export missing. |
| Export xLights model (.xmodel) | menu | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop ID_PREVIEW_MODEL_EXPORTXLIGHTSMODEL. iPad: no UI. |
| Export CAD (DXF/STL/VRML) | menu | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop ID_PREVIEW_MODEL_CAD_EXPORT. iPad: no UI. |
| Export Faces/States/SubModels to other models | menu | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop ID_PREVIEW_EXPORT_FACESSTATESSUBMODELS. iPad: no UI. |
| Model import (.xmodel / multi-model) | dialog | ✅ | ✅ | parity | P1 | medium | feasible | iPad .fileImporter + multi-model handling (xmodelFileIsMultiModel). |
| Import fixture from GDTF file | dialog | ✅ | ❌ | ipad-missing | P2 | medium | feasible | Desktop GdtfParser → CreateDmxModelFromGdtf. iPad declares gdtf UTType (LayoutEditorView.swift:342, :515-532) but has no GdtfParser/ParseGdtf path. |
| Import Previews/Models/Groups (from RGBeffects) | menu | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop ID_PREVIEW_IMPORTMODELSFROMRGBEFFECTS. iPad: no UI. |
| Import LOR S5 models/groups | menu | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop ID_PREVIEW_IMPORT_MODELS_FROM_LORS5. iPad: no UI. |
| Vendor model browser (download .xmodel/wiring) | dialog | ❌ | ✅ | desktop-missing | P2 | hard | feasible | iPad VendorBrowserSheet (XLVendorCatalog). Desktop downloads .xmodel manually. |
| Map-from-lights (FPP structured-light scan) | dialog | ❌ | ✅ | desktop-missing | P3 | hard | restricted | iPad MapFromLightsWizard (camera + FPP). Needs FPP controller + camera; desktop has no camera path. |
| Models report export (.xlsx) | menu | ✅ | ✅ | parity | P3 | medium | feasible | Desktop File→Export Models (xLightsMain.cpp:1122). iPad exportModelsReport + ModelsReportExportDoc. |
| Layout export As DXF | menu | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop ID_PREVIEW_LAYOUT_DXF_EXPORT. iPad: no UI. |
| Preview / layout-group select | panel | ✅ | ✅ | parity | P1 | easy | feasible | Desktop ChoiceLayoutGroups; iPad sidebar layout-group list. |
| Preview create | dialog | ✅ | ✅ | parity | P2 | medium | feasible | iPad createLayoutGroup + New Preview sheet. |
| Preview delete | menu | ✅ | ❌ | ipad-missing | P2 | medium | feasible | Desktop ID_PREVIEW_DELETE_ACTIVE. No iPad delete-layout-group bridge. |
| Preview rename | dialog | ✅ | ❌ | ipad-missing | P2 | medium | feasible | Desktop ID_PREVIEW_RENAME_ACTIVE. No iPad rename-layout-group bridge. |
| Preview background image picker | dialog | ✅ | ✅ | parity | P2 | medium | feasible | iPad Objects tab → 2D Background → backgroundImage .fileImporter. |
| Preview background brightness / alpha / scale-to-fit | panel | ✅ | 🟡 | ipad-weaker | P2 | easy | feasible | iPad intField backgroundBrightness (LayoutEditorView.swift:10617), backgroundAlpha (:10620), scaleBackgroundImage (:10624-10625); narrower than desktop's full display-options grid. |
| Preview 2D center-origin (X0=center) toggle | panel | ✅ | 🟡 | ipad-missing | P3 | easy | feasible | iPad shows display2DCenter0 read-only; no setter. Needs bridge setDisplay2DCenter0. |
| Preview canvas size edit | panel | ✅ | 🟡 | ipad-missing | P3 | easy | feasible | iPad shows canvas WxH read-only. Needs bridge setters. |
| Preview 2D grid overlay toggle | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad PreviewSettings.showLayoutGrid → bridge. |
| Preview 2D grid spacing edit | panel | ✅ | 🟡 | ipad-missing | P3 | easy | feasible | Global preview grid spacing read-only on iPad (Gridlines view-object spacing is editable but separate). |
| Preview bounding-box overlay | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad showLayoutBoundingBox. |
| Preview model-name labels | panel/overlay | ✅ | ✅ | parity | P2 | easy | feasible | iPad showModelLabels (SwiftUI overlay). |
| Preview model-info labels (channels) | panel/overlay | ✅ | ✅ | parity | P2 | easy | feasible | iPad showModelInfo. |
| Preview 2D/3D toggle | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad settings.is3D + toolbar. |
| Preview pan / zoom / 3D orbit | gesture | ✅ | ✅ | parity | P1 | easy | feasible | iPad 1-finger drag / pinch / 3D orbit; desktop drag/wheel/right-drag. |
| Preview camera roll (twist) | gesture | 🟡 | ✅ | parity | P2 | easy | feasible | iPad two-finger rotate (Z twist). Desktop trackpad rotate. |
| Preview reset / fit view | gesture | ✅ | ✅ | parity | P2 | easy | feasible | iPad double-tap → resetCamera; desktop right-click Reset. |
| Preview first-pixel marker | panel | ✅ | ✅ | parity | P2 | medium | feasible | iPad showFirstPixel; desktop _showFirstPixel. |
| Preview snap-to-grid (drag) | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad PreviewSettings.snapToGrid. |
| Preview set center offset (group) | menu | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop ID_SET_CENTER_OFFSET. iPad: no UI. |
| Save layout image (PNG) | menu | ✅ | 🟡 | ipad-weaker | P3 | medium | feasible | iPad UIGraphicsImageRenderer + drawHierarchy → UIActivityViewController save/share (PreviewPaneView.swift:626-630); snapshots the preview rather than a dedicated full-layout image export. |
| Print layout image | menu | ✅ | 🟡 | ipad-weaker | P3 | medium | feasible | iPad shares via UIActivityViewController (print available through the share sheet); no dedicated print path. |
| Viewpoint save / restore / default / delete | menu | ✅ | ✅ | parity | P2 | hard | feasible | iPad full save/apply/delete/restore over rctx->GetViewpointMgr() + SaveViewpoints (XLMetalBridge.mm:469-583); applyViewpointNamed / deleteViewpointNamed / restoreDefaultViewpointForDocument wired from PreviewPaneView. |
| House preview (full show) | tab | ✅ | ✅ | parity | P1 | medium | feasible | Desktop HousePreviewPanel; iPad HousePreviewView (embeddable + detachable scene). |
| View object create (Image/Mesh/Gridlines/Terrain/Ruler) | menu | ✅ | ✅ | parity | P2 | medium | feasible | iPad AddViewObjectSheet → createViewObject. |
| View object properties edit | panel | ✅ | ✅ | parity | P2 | medium | feasible | iPad descriptor pane per type. |
| View object rename | dialog | 🟡 | ✅ | desktop-missing | P2 | easy | feasible | iPad renameViewObject + sheet. Desktop has no dedicated rename for view objects. |
| View object duplicate | menu | 🟡 | ✅ | desktop-missing | P2 | easy | feasible | iPad duplicateViewObject. Desktop: not exposed. |
| View object delete | menu | ✅ | ✅ | parity | P2 | medium | feasible | iPad deleteViewObject + confirm. |
| View object drag-move / rotate / scale | gesture | ✅ | ✅ | parity | P2 | medium | feasible | Both handle-based. |
| Image object file picker | dialog | ✅ | ✅ | parity | P2 | medium | feasible | iPad generic object file-importer. |
| Mesh object file picker (.obj/.3ds/.stl/.ply) | dialog | ✅ | ✅ | parity | P2 | medium | feasible | iPad mesh-file picker w/ UTType filter. |
| Terrain object height-map / editing | dialog | ✅ | 🟡 | ipad-missing | P3 | hard | feasible | iPad terrain edit target flow; full height-map paint scope narrower than desktop TerrainObjectPropertyAdapter. |
| DMX MovingHead / MovingHeadAdv create + props | menu/panel | ✅ | ✅ | parity | P2 | hard | feasible | iPad AddModelSheet + descriptor (fixture/mode), MovingHeadFixtureRowView. |
| DMX Servo / Servo3D create + props | menu/panel | ✅ | ✅ | parity | P2 | hard | feasible | iPad add types + descriptor. |
| DMX Skull create + props | menu/panel | ✅ | ✅ | parity | P2 | hard | feasible | iPad add types + descriptor (Skulltronix preset). |
| DMX Floodlight create + props | menu/panel | ✅ | ✅ | parity | P2 | hard | feasible | iPad add types + descriptor. |
| DMX General create + props | menu/panel | ✅ | ✅ | parity | P2 | hard | feasible | iPad add types + descriptor. |
| DMX preset / wheel-color / position-zone list editors | dialog | ✅ | ✅ | parity | P2 | hard | feasible | iPad DmxPresetListEditorSheet / DmxWheelColorListEditorSheet / DmxPositionZoneListEditorSheet. |
| Polyline create (tap-to-add-point) | gesture | ✅ | ✅ | parity | P2 | hard | feasible | iPad layoutPolylineInProgress + tap-to-place + Esc/Return commit. |
| Polyline segment context (Add/Delete Point, Define/Remove Curve) | context-menu | ✅ | ✅ | parity | P2 | hard | feasible | iPad long-press → confirmationDialog (vertex/segment/curve). |
| Polyline "Enter Segment Size" | dialog | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop ID_PREVIEW_MODEL_SET_SEGMENTS. iPad: no node-count-per-segment dialog. |
| Model / group unlink from base show folder | menu/panel | ✅ | ❌ | ipad-missing | P1 | medium | feasible | No unlinkModel/unlinkGroup/UnlinkSelectedModels in src-iPad; only controller unlink exists (XLSequenceDocument.h:2341 unlinkControllerFromBase). |
| Make Start Channel Valid (model) | menu | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop ID_MNU_MAKESCVALID. iPad: no UI. |
| Make All Start Channels Valid | menu | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop ID_MNU_MAKEALLSCVALID. iPad: no UI. |
| Make All Start Channels Not Overlapping | menu | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop ID_MNU_MAKEALLSCNOTOVERLAPPING. iPad: no UI. |
| Delete Empty Groups / Delete All Aliases (bulk) | menu | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop tree context. iPad: no bulk-housekeeping UI. |
| Model list sort | menu | ✅ | ✅ | parity | P2 | easy | feasible | iPad ModelSortMode picker; desktop column sort. |
| Model list filter / search | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad searchable list; desktop wxSearchCtrl. |
| Controller list (add/edit/delete/reorder) | tab/panel | ✅ | ✅ | parity | P2 | hard | feasible | Desktop Setup/Controllers tab; iPad Layout sidebar Controllers tab (addController, reorder). Different surface, same capability. |
| Controller discovery (FPP/WLED/ESPixelStick) | menu | ✅ | ✅ | parity | P2 | hard | feasible | Desktop DiscoveryAuthDialog; iPad J-31.4 discover + mismatch resolution. |
| Controller visualize (wiring) | dialog | ✅ | ✅ | parity | P2 | medium | feasible | Desktop ControllerModelDialog; iPad ControllerVisualizeView (wiringForController). |
| Controller config upload (open firmware) | menu | ✅ | ✅ | parity | P2 | hard | feasible | iPad uploadInput/uploadOutput gated by caps; open-source firmwares (FPP/WLED/ESPixelStick). |
| Controller config upload (closed firmware) | menu | ✅ | 🟡 | parity | P3 | hard | restricted | iPad path is caps-gated; closed/proprietary uploads are IAP-gated/restricted, low priority. |
| Controller properties (universe/channels/protocol) | panel | ✅ | ✅ | parity | P2 | hard | feasible | Desktop Controller adapters; iPad controller detail descriptor pane. |
| Controller unlink from base show folder | menu/panel | ✅ | ✅ | parity | P2 | medium | feasible | iPad controller property-pane Unlink; desktop in Setup tab. |
| Undo / redo layout changes | toolbar/shortcut | ✅ | ✅ | parity | P1 | medium | feasible | iPad pushLayoutUndoSnapshot + Undo button; desktop Ctrl+Z/Y. |
| SubModel import (from Model / File / Layout / Downloads / CSV) | dialog | ✅ | ❌ | ipad-missing | P2 | hard | feasible | Desktop SubModelsDialog.cpp:1126 ImportSubModel, :1138 ImportCSVSubModel, def :3537. iPad has no submodel-import symbols. |
| SubModels output-to-lights live test toggle | dialog | ✅ | ❌ | ipad-missing | P2 | medium | feasible | Desktop SubModelsDialog output-to-lights. iPad startOutput (XLSequenceDocument.h:1650-1653) used only in SequencerViewModel, not in editors. |
| Real-world dimension readouts (ruler-calibrated) | panel | ✅ | ❌ | ipad-missing | P2 | medium | feasible | Desktop ScreenLocationProperties RealWidth/RealHeight/RealDepth via ConvertDimension. iPad has no RealWidth/RealLength/ConvertDimension. |
| Overlap checks toggle | panel | ✅ | ❌ | ipad-missing | P3 | easy | feasible | Desktop CheckBoxOverlap (LayoutPanel.cpp:636, :3456) + CheckModelForOverlaps (:7720). iPad has no overlap-check UI. |
| SubModel import from State / Face definitions | dialog | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop SubModelsDialog.cpp CreateSubmodel(name,nodes) (:3768) invoked from state/face context (:1064, :1117). iPad absent. |
| SubModel 'Import Custom Model Overlay' (matrix only) | dialog | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop SubModelsDialog.cpp:1132 ImportCustomModel, :3790/:3936/:4007 overlay helpers. iPad absent. |
| SubModels Symmetrize (rotational generator) | dialog | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop SubModelsDialog::Symmetrize() — degree/direction(CW/CCW)/build-order, config-persisted (menu added in 259230f0f). Algorithm is wx-coupled (wxFile temp + NodeUtils::CompressNodes), not in src-core; iPad SubModelListSheet has no Symmetrize. Needs core extraction + bridge before iPad UI. |
| Faces output-to-lights live test toggle | dialog | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop ModelFaceDialog CheckBox_OutputToLights (:223). iPad startOutput not wired into the Faces editor. |
| States output-to-lights live test toggle | dialog | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop ModelStateDialog CheckBox_OutputToLights (:206). iPad startOutput not wired into the States editor. |
| RGBW PWM per-channel Brightness + Gamma | panel | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop DmxAbilityPropertyHelpers.cpp:265-320 per-color brightness/gamma. iPad XLSequenceDocument.mm:6278-6289 exposes RGBW channels only. |
| View object align / distribute / flip / resize | menu/toolbar | ✅ | ❌ | ipad-missing | P3 | medium | feasible | iPad alignModels/distributeModels/matchSizeOfModels/flipModels (XLMetalBridge.h:508-540) operate on models only, not view objects. |
| Handle / crosshair scaling preferences | panel | ✅ | ❌ | ipad-missing | P3 | easy | feasible | Desktop adjustable handle/crosshair size. iPad iPadModelPreview.h:58 GetHandleScale() returns fixed 1; no handleSize/crosshairSize setting. |
| Zone indicator overlay | panel/overlay | ✅ | ❌ | ipad-missing | P3 | easy | feasible | Desktop 'Show Zone Indicator in Preview' (xLightsMain.h:1331-1332, OtherSettingsPanel.cpp:203). iPad has no ZoneIndicator/showZone. |
| Node tooltip on hover (2D) | overlay | ✅ | ❌ | ipad-missing | P3 | medium | feasible | iPad nodeNearPoint (XLMetalBridge.mm:3331) used only by SubmodelPreviewPane; no hover tooltip in the layout preview. |
| SpaceMouse 6-DOF input | other | ✅ | ❌ | ipad-missing | P3 | hard | infeasible | Desktop Mouse3DManager/SpaceMouseSession. No iOS HID SpaceMouse. |

## iPad gaps (desktop has, iPad missing)

### P2

- **Node Layout / Wiring View (model-level).** Desktop `ShowNodeLayout`
  (NodeSelectGrid) and `ShowWiring` are launched from the model
  right-click (`LayoutPanel.cpp:6156`, `:6159`) and Layout keybindings.
  iPad has no node-grid view; its only "Visualize" is the
  *controller*-level wiring table. Needs a new `NodeLayoutSheet`
  (SwiftUI grid reading node positions/strand assignments) backed by a
  bridge accessor over `Model::GetNodeXY` / strand info. Ease: hard.
- **Faces / States rich editor.** iPad `FaceStateEditorSheet`
  (`LayoutEditorView.swift:8492`) edits the `faceInfo` / `stateInfo`
  attribute maps as key/value pairs and is fully wired
  (`setFaceInfo:` / `setStateInfo:` / `faceInfoForModel:` /
  `stateInfoForModel:`). What's missing vs desktop `ModelFaceDialog` /
  `ModelStateDialog` is the **visual node-grid / image-matrix face
  assignment** — tapping nodes to build a mouth/eye shape, and the
  matrix-face image-channel mapping. Ease: hard (needs a Metal/CG node
  picker over the model preview).
- **Preview delete / rename (layout group).** Desktop
  `ID_PREVIEW_DELETE_ACTIVE` / `ID_PREVIEW_RENAME_ACTIVE`
  (`LayoutPanel.cpp:6319-6320`). iPad has `createLayoutGroup` but no
  delete/rename bridge for layout groups. Add `deleteLayoutGroup:` /
  `renameLayoutGroup:to:` to `XLSequenceDocument` + sidebar actions.
  Ease: medium.
- **GDTF fixture import.** iPad declares the `gdtf` UTType
  (`LayoutEditorView.swift:342`, `:515-532`) but has no `GdtfParser` /
  `CreateDmxModelFromGdtf` path; importing a GDTF fixture does nothing.
  Needs a bridge entry point over the shared GDTF parser. Ease: medium.
- **SubModel import (from Model / File / Layout / Downloads / CSV).**
  Desktop `SubModelsDialog.cpp:1126` `ImportSubModel`, `:1138`
  `ImportCSVSubModel` (def `:3537`). iPad's SubModel editor only
  creates/edits/deletes inline — no import sources. Ease: hard.
- **SubModels output-to-lights live test toggle.** Desktop drives the
  selected submodel to the lights for testing. iPad's `startOutput`
  (`XLSequenceDocument.h:1650-1653`) is used only by the sequencer, not
  the SubModel editor. Ease: medium.
- **Real-world dimension readouts (ruler-calibrated).** Desktop
  ScreenLocationProperties show RealWidth/RealHeight/RealDepth via
  `ConvertDimension`. iPad has no real-world dimension fields. Ease:
  medium.

### P3

- **Model copy/cut/paste to clipboard (layout).** Desktop `DoCopy` /
  `DoPaste` (`LayoutPanel.cpp:8875`) round-trips `CopyPasteBaseObject`
  XML. iPad relies on Duplicate; a `UIPasteboard` model-XML copy/paste
  would add cross-document paste. Ease: hard.
- **Dimming-curve authoring.** iPad can edit the dimming attribute map
  and clear it, but the sheet explicitly says authoring a curve is
  desktop-only (`LayoutEditorView.swift:4081`). Desktop
  `ModelDimmingCurveDialog` has the graphical curve. Ease: hard.
- **Group clone, model replace, correct aspect ratio, set-center-offset,
  enter-segment-size.** All single desktop menu items
  (`ID_MNU_CLONE_MODEL_GROUP`, `ID_PREVIEW_REPLACEMODEL`,
  `ID_PREVIEW_MODEL_ASPECTRATIO`, `ID_SET_CENTER_OFFSET`,
  `ID_PREVIEW_MODEL_SET_SEGMENTS`). Each needs a small bridge wrapper +
  context entry. Ease: medium each.
- **Make-start-channel-valid / -not-overlapping (model + all).** Desktop
  `ID_MNU_MAKESCVALID` / `ID_MNU_MAKEALLSCVALID` /
  `ID_MNU_MAKEALLSCNOTOVERLAPPING` (`LayoutPanel.cpp:10773-10802`).
  Useful housekeeping; needs bridge ops over the channel solver.
- **Export-as-custom / -3D-custom / -xmodel, CAD (DXF/STL/VRML),
  layout DXF, faces/states/submodels export, import-from-RGBeffects,
  import LOR-S5.** All desktop export/import menu items
  (`LayoutPanel.cpp:6164-6173`, `:6325-6327`). Most reuse shared
  serializers — needs file-exporter UI + bridge entry points. Ease:
  medium–hard.
- **Dedicated Bulk-Edit submenu items** (controller direction /
  colour-order / null-nodes / group-count / brightness / gamma /
  dimming). iPad applies the *property-pane* value across the selection
  (J-4) but doesn't expose the desktop's per-key bulk dialogs. The
  controller-specific keys aren't in the iPad property pane. Ease:
  medium.
- **2D center-0 toggle, canvas-size edit, global 2D grid-spacing edit.**
  All three are read-only roll-ups on iPad
  (`LayoutEditorView.swift:2351`, `:10636`, `:10640`). Need bridge
  setters (`setDisplay2DCenter0`, canvas-size, `setDisplay2DGridSpacing`
  exists for the toggle path — confirm a global setter) + editable
  rows. Ease: easy.
- **Overlap checks toggle.** Desktop `CheckBoxOverlap`
  (`LayoutPanel.cpp:636`, `:3456`) + `CheckModelForOverlaps` (`:7720`)
  flags overlapping start channels. iPad has no overlap-check UI. Ease:
  easy.
- **SubModel import from State / Face definitions** and **'Import Custom
  Model Overlay' (matrix only).** Desktop `SubModelsDialog.cpp`
  `CreateSubmodel` (`:3768`) from state/face context (`:1064`, `:1117`)
  and `ImportCustomModel` (`:1132`, `:3790`/`:3936`/`:4007`). iPad
  absent. Ease: medium / hard.
- **Faces / States output-to-lights live test toggle.** Desktop
  `ModelFaceDialog` `CheckBox_OutputToLights` (`:223`) and
  `ModelStateDialog` (`:206`) drive the lights from the editor. iPad's
  `startOutput` isn't wired into the Faces/States editors. Ease: medium.
- **RGBW PWM per-channel Brightness + Gamma.** Desktop
  `DmxAbilityPropertyHelpers.cpp:265-320` exposes per-color
  brightness/gamma. iPad (`XLSequenceDocument.mm:6278-6289`) exposes the
  RGBW channels only. Ease: medium.
- **View object align / distribute / flip / resize.** iPad
  `alignModels` / `distributeModels` / `matchSizeOfModels` /
  `flipModels` (`XLMetalBridge.h:508-540`) operate on models only;
  multi-select arrange isn't available for view objects. Ease: medium.
- **Handle / crosshair scaling preferences.** Desktop lets users size
  the manipulation handles; iPad `iPadModelPreview.h:58`
  `GetHandleScale()` returns a fixed 1. Ease: easy.
- **Zone indicator overlay.** Desktop 'Show Zone Indicator in Preview'
  (`xLightsMain.h:1331-1332`, `OtherSettingsPanel.cpp:203`). iPad has no
  zone-indicator overlay. Ease: easy.
- **Node tooltip on hover (2D).** iPad `nodeNearPoint`
  (`XLMetalBridge.mm:3331`) is used only by the SubModel preview; the
  layout preview shows no per-node hover tooltip. Ease: medium.
- **Per-node individual start channels** — desktop has a per-node
  editor; iPad exposes only the start-channel string. Ease: medium.

### Infeasible

- **SpaceMouse 6-DOF** — desktop `Mouse3DManager` / `SpaceMouseSession`;
  no iOS HID path. Out of scope.

## Desktop gaps (iPad has, desktop missing)

### P2

- **Vendor model browser.** iPad `VendorBrowserSheet` (XLVendorCatalog)
  searches an online model catalog, picks a wiring, and downloads the
  `.xmodel`. Desktop users hunt for files manually. Would be a useful
  desktop dialog reusing the same catalog core.
- **View object rename / duplicate.** iPad `renameViewObject` /
  `duplicateViewObject` are first-class; desktop has no rename/duplicate
  menu items for view objects (only delete + property edit). Low-effort
  desktop add.
- **Touch-idiom Move/Rotate/Scale tool picker + axis-lock chips.** iPad
  `LayoutEditorToolToolbar`; desktop uses centre-cycle + gizmo. Not a
  real gap (desktop's gizmo is richer) — UI idiom only.

### P3

- **Map-from-lights (FPP camera scan).** iPad-only and *restricted* —
  requires an FPP controller + the device camera; desktop has no camera
  capture path. Keep desktop-free.

## Infeasible / restricted on iPad

- **Closed-firmware controller config upload.** The iPad upload path
  (`uploadInput` / `uploadOutput`) is caps-gated and in-scope for
  open-source firmwares (FPP / WLED / ESPixelStick / DDP / generic).
  Proprietary closed-firmware uploads remain **restricted** (IAP-gated,
  P3).
- **SpaceMouse / 3D mouse** — no iOS HID support. Infeasible.
- **Map-from-lights** is iPad-only but depends on an FPP controller +
  camera; treat as restricted (not a desktop gap to fill).

## Recommended sequencing

1. **Faces / States visual node picker (P2).** The single biggest
   *qualitative* gap. The attribute-map editor already exists and is
   bridged; layering a node-grid assignment view over the model preview
   (reuse the custom-model grid paint code) closes the most-requested
   layout gap.
2. **Preview delete / rename (P2)** — tiny bridge additions
   (`deleteLayoutGroup` / `renameLayoutGroup`) + two sidebar actions;
   completes preview/layout-group CRUD which is otherwise create-only.
3. **2D center-0 / canvas-size / grid-spacing editable rows (P3, easy)**
   — quick wins; the values are already surfaced read-only, just need
   setters and editable controls.
4. **Node Layout / Wiring View (P2)** and **Viewpoint save/restore
   (P2)** — both need new bridge accessors and a sheet; schedule after
   the cheap wins.
5. **Export family + make-SC-valid + group-clone / replace / aspect /
   center-offset / segment-size (P3)** — batch the single-shot menu
   wrappers; mostly mechanical bridge + context-entry work reusing
   shared core serializers/solvers.
6. **Layout clipboard copy/paste & dimming-curve authoring (P3, hard)**
   — lowest priority; Duplicate + clear-dimming cover the common cases.
