# 06 · Layout: Models, Groups, Preview, 3D, Submodels/Faces/States/DMX

> The iPad Layout Editor has full model/group CRUD, descriptor-driven
> property editing, multi-select **Align / Distribute / Match-Size /
> Flip / Rotate X-Y-Z / Duplicate / Group**, single-model **Replace
> Model** (clone a source over a checkbox list of targets), a complete
> **Model Data** section
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
> bulk/utility commands: **Faces/States now have a visual tap-to-pick
> node assignment on iPad (NodeRangePickerSheet, reusing the SubModel
> Metal node-picker) — only the matrix-face *image* mapping + xmodel
> face/state file-import remain desktop-only**; **CAD/DXF export, export-as-custom,
> import-from-RGBeffects / LOR-S5, the rest of the dedicated Bulk-Edit submenu
> (incl. dimming curves / controller direction), and unlink model/group from
> base show** are all desktop-only.
> **Model export-as-.xmodel, GDTF fixture import (now with a multi-mode
> DMX-mode picker), make-start-channel
> valid / all-valid / all-not-overlapping, SubModel import (from
> .xmodel file / another model / Layout RGBeffects-pull / CSV), the
> layout clipboard (⌘C/⌘V/⌘X over a `com.xlights.layoutmodels`
> pasteboard UTI, cross-sequence), the node-inspect tap tooltip,
> the manipulation-handle-size + zone-indicator view toggles, and
> ruler-calibrated real-world
> dimension readouts are now at parity** (bridge wrappers + roster
> context-menu / SubModel-editor / property-pane / canvas-controls
> surfaces). The theme-06
> single-shot layout commands **preview delete / rename, model-group
> clone, correct aspect ratio, set-center-offset, polyline
> enter-segment-size, the overlap-checks toggle, and the
> 2D-center-0 + canvas-size + global grid-spacing edits** are now at
> parity (bridge wrappers + sidebar/context entries). iPad-only
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
| Model clipboard copy/cut/paste (layout) | shortcut | ✅ | ✅ | parity | P3 | hard | feasible | Desktop DoCopy/DoPaste (CopyPasteBaseObject XML). iPad: bridge `copyModelsToString:` (XmlSerializer::SerializeModels → string) + `pasteModelsFromString:` (deserialize, uniquify, clear controller, +50/+50 offset, RecalcStartChannels) wired to ⌘C/⌘V/⌘X in `LayoutClipboardKeysModifier` (LayoutEditorView.swift). Pasteboard carries a custom `com.xlights.layoutmodels` UTI (declared in xLights-iPad Info.plist) + plain-text fallback, so paste works cross-sequence. |
| Model keyboard nudge (arrows ±) | shortcut | ✅ | ✅ | parity | P2 | easy | feasible | Desktop Nudge(); iPad arrow-key nudge + per-step undo. |
| Per-type property grid (all model types) | panel | ✅ | ✅ | parity | P1 | medium | feasible | iPad descriptor pane mirrors desktop adapters per type. |
| Model tag color | panel | ✅ | ✅ | parity | P2 | easy | feasible | tagColorPicker. |
| Model active/inactive | panel | ✅ | ✅ | parity | P1 | easy | feasible | boolBinding "active". |
| Model controller-port connection | panel | ✅ | ✅ | parity | P1 | medium | feasible | iPad controllerConnectionFields (port/protocol/smart-remote gated by caps). #6518 restored port management for from-base models in core ModelManager.cpp — shared, auto-applies to iPad. |
| Model start channel picker | dialog | ✅ | ✅ | parity | P1 | medium | feasible | iPad StartChannelEditorSheet. |
| Individual start channels per string | panel | ✅ | ✅ | parity | P3 | medium | feasible | iPad hasIndividualStartChannels toggle + per-string individualStartChannel fields (LayoutEditorView.swift:3973-3990). |
| Node Layout (node-grid visualization) | dialog | ✅ | ✅ | parity | P2 | hard | feasible | Desktop ShowNodeLayout/ChannelLayoutDialog (LayoutPanel.cpp:6889). iPad NodeLayoutSheet (NodeLayoutSheet.swift) — zoom/pan Canvas of node numbers at buffer coords, backed by `nodeLayout(forModel:)` (XLSequenceDocument.mm) over InitRenderBufferNodes. Launched from Model Data section (LayoutEditorView.swift). |
| Wiring View (model node wiring) | dialog | ✅ | 🟡 | parity | P2 | hard | feasible | Desktop ShowWiring/WiringDialog (LayoutPanel.cpp:6899). iPad NodeLayoutSheet `wiring:true` — per-string node path, numbered, front/rear flip. Desktop's print + DXF export + theme/multi-light render options stay desktop-only (deep-dialog cluster). |
| Pixel size / style | panel | ✅ | ✅ | parity | P2 | easy | feasible | intField pixelSize + pixelStylePicker. |
| Transparency / alpha | panel | ✅ | ✅ | parity | P2 | easy | feasible | intField transparency. |
| Black transparency | panel | ✅ | ✅ | parity | P3 | easy | feasible | iPad intField "blackTransparency" (Appearance section). |
| Shadow Model For | panel | ✅ | ✅ | parity | P3 | easy | feasible | iPad shadowModelPicker (LayoutEditorView.swift:4856). |
| Smart remote (per-port remote) | panel | ✅ | ✅ | parity | P2 | medium | feasible | iPad ModelSmartRemoteSheet + descriptor caps. |
| Correct Aspect Ratio | menu | ✅ | ✅ | parity | P3 | medium | feasible | Desktop ID_PREVIEW_MODEL_ASPECTRATIO. iPad: `correctAspectRatioForModel:` (XLSequenceDocument.mm) replicates the GetMWidth/MHeight ÷ GetRenderWi/Ht ratio adjustment + aspect button in InlineModelActionBar (LayoutEditorView.swift). |
| Model Data count badges (SubModels/Faces/States/Nodes) | panel | ✅ | ✅ | parity | P3 | easy | feasible | Desktop ModelPropertyAdapter Format*Label (SubModels (N) / Faces (N) / States (N) / Strand-Node (N nodes), #6512). iPad Model Data rows already show `list.count` per `modelDataRow` (LayoutEditorView.swift) fed by extrasFor: (faceNames/stateNames/submodelNames/nodeNames), plus nodeCount in the summary header — parity, shared/auto-refreshed via summaryToken. |
| SubModels create/edit/rename/delete | dialog | ✅ | ✅ | parity | P1 | medium | feasible | iPad SubModelListSheet (add/delete/rename + detail edit via replaceSubModelsOnModel). |
| SubModel aliases | dialog | ✅ | ✅ | parity | P3 | medium | feasible | iPad setSubmodelAliases / submodelAliases + alias editor (LayoutEditorView.swift:6704). |
| Faces editor | dialog | ✅ | 🟡 | parity | P2 | hard | feasible | iPad FaceStateEditorSheet (LayoutEditorView.swift) now has a visual node picker: each phoneme/part attribute (FaceOutline, Mouth-*, Eyes-*) gets a tap-to-pick button (FaceStateEntryDetailView) → NodeRangePickerSheet (NodePickerPane.swift) which reuses SubmodelPreviewPane's Metal node-picker (tap-toggle + two-finger marquee, XLMetalBridge.nodeNearPoint/setSubmodelHighlightedNodes) and writes back the compressed channel-range string via SubModelRangeOps.compressNodes — matching desktop ModelFaceDialog node assignment + NodeUtils::CompressNodes format. Remaining 🟡: matrix-face image assignment stays the attr-map text path (FaceStateAttr.isNodeRange excludes `-Eyes*` image keys), and there's no xmodel face/state file-import (ModelFaceDialog::ImportFaces, #6499). |
| States editor | dialog | ✅ | ✅ | parity | P2 | hard | feasible | Same node-picker as Faces: each state's node-list attribute (sNNN / part keys) gets the tap-to-pick NodeRangePickerSheet → SubmodelPreviewPane; `-Name`/`-Color`/Type/CustomColors metadata stay text fields. Matches desktop ModelStateDialog node-grid assignment. |
| Custom model visual editor | dialog | ✅ | ✅ | parity | P2 | medium | feasible | iPad CustomModelEditorSheet (grid + paint + Distribute-along-line). setCustomModelData bridge. Node-bounding-box centering (CustomModel.cpp InitCustomMatrix, #6452) is shared core — applies to iPad automatically. |
| Move background image in custom-model editor | dialog | ✅ | ❌ | ipad-missing | P3 | easy | infeasible-shared | Desktop #5506 (CustomModelDialog `_bkg_offset_x/_y`, BtnBkgLeft/Right/Up/Down/Reset). The offset is **transient desktop-UI state only** — explicitly NOT saved to the CustomModel or serialized (CustomModelDialog.cpp:1157 "Offsets are intentionally transient"). No shared-core data backs it, so an iPad port would be a separate session-only convenience in CustomModelEditorSheet, not a data-parity gap. Deferred; not a behavior gap. |
| Strand / Node names editor | dialog | ✅ | ✅ | parity | P2 | medium | feasible | iPad IndexedNamesEditorSheet (setStrandNames/setNodeNames + DMX generateNodeNames). |
| Per-model aliases editor | dialog | ✅ | ✅ | parity | P3 | medium | feasible | iPad AliasEditorSheet (setModelAliases). |
| Model dimming curve | dialog | ✅ | 🟡 | ipad-missing | P3 | hard | feasible | iPad edits dimmingInfo attr-map + can CLEAR; cannot author a new curve graphically (sheet warns). Desktop ModelDimmingCurveDialog full editor. |
| Model group create | menu | ✅ | ✅ | parity | P1 | medium | feasible | iPad createModelGroup + NewGroupSheet (also from multi-select). |
| Model group rename | dialog | ✅ | ✅ | parity | P1 | medium | feasible | iPad renameModelGroup + sheet. |
| Model group delete | menu | ✅ | ✅ | parity | P1 | medium | feasible | iPad deleteModelGroup + confirm. |
| Model group add members | dialog | ✅ | ✅ | parity | P1 | medium | feasible | iPad addModel:toGroup: + LayoutGroupPickerSheet / Add-to-Group. |
| Model group remove members | menu | ✅ | ✅ | parity | P2 | medium | feasible | iPad removeModel:fromGroup: via group property pane onRemoveMember. |
| Model group reorder members | panel | ✅ | ✅ | parity | P2 | medium | feasible | iPad onReorderMembers (commit "members"). |
| Model group properties (layout style/grid/tag) | panel | ✅ | ✅ | parity | P1 | medium | feasible | iPad LayoutEditorGroupPropertiesView descriptor. "Per Model Default" layout value (#4125, core `ModelGroup.cpp:36`) is present both sides — both UIs hardcode the option list (desktop `ModelGroupPanel.cpp:181`, iPad `XLSequenceDocument.mm:4976` `modelGroupLayoutSummary:` + `LayoutEditorView.swift:1789` label map). Verified iPad list already includes it. |
| Model group clone | menu | ✅ | ✅ | parity | P3 | medium | feasible | Desktop ID_MNU_CLONE_MODEL_GROUP. iPad: `cloneModelGroup:` (XLSequenceDocument.mm — serialize via XmlSerializingVisitor → GenerateModelName → CreateModel) + "Clone Group" button in LayoutEditorGroupPropertiesView. |
| Multi-select Align (8 + ground) | menu/toolbar | ✅ | ✅ | parity | P1 | medium | feasible | iPad MultiSelectActionBar Align menu (left/right/top/bottom/front/back/centerH/V/D/ground). |
| Multi-select Distribute (H/V/Depth) | menu/toolbar | ✅ | ✅ | parity | P1 | medium | feasible | iPad Distribute menu (>=3 selected). |
| Multi-select Match Size (W/H/Depth/All) | menu/toolbar | ✅ | ✅ | parity | P2 | medium | feasible | iPad Match-Size menu (XLMetalBridge.h:528 matchSizeOfModels:toLeader:dimension:). |
| Multi-select Flip H/V | menu/toolbar | ✅ | ✅ | parity | P2 | medium | feasible | iPad Flip menu (flipModels). |
| Swap Start/End (Single/Poly Line) | menu | ✅ | ✅ | parity | P2 | easy | feasible | Desktop #6483 ID_PREVIEW_SWAP_START_END (LayoutPanel.cpp). Core Model::SwapStartEnd() shared (PolyLineModel/SingleLineModel/PolyPointScreenLocation/TwoPointScreenLocation). iPad: bridge `swapStartEndForModels:forDocument:` (XLMetalBridge.mm) + ⇄ button in InlineModelActionBar (LayoutEditorView.swift, shown only for Single Line/Poly Line, gated on lock/from-base) → performSwapStartEnd, pushes layout undo. |
| Property-pane bulk edit across selection | panel | ✅ | ✅ | parity | P2 | medium | feasible | iPad J-4 applies a property-pane edit to all selected (bulkEditTargets, model-unique keys excluded). Desktop has dedicated Bulk-Edit submenu. |
| Bulk Edit: Rotate X/Y/Z | menu/toolbar | ✅ | ✅ | parity | P2 | medium | feasible | Desktop BulkEditRotateAxis (LayoutPanel.cpp:2391) → loc.SetRotateX/Y/Z + Reload/Init, [-180,180] clamp. iPad: bridge `rotateModels:axis:degrees:forDocument:` (XLMetalBridge.mm) + Rotate X/Y/Z menu in MultiSelectActionBar (LayoutEditorView.swift) → promptBulkRotate/performBulkRotate, degrees alert pre-filled from leader, per-model undo. |
| Bulk Edit: controller direction / colour-order / null-nodes / group-count | menu | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop mnuBulkEdit specific items. iPad property pane doesn't expose these per-key bulk ops. |
| Bulk Edit: dimming curves | dialog | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop ID_PREVIEW_BULKEDIT_DIMMINGCURVES. No iPad dimming authoring. |
| Replace model(s) with this model | menu | ✅ | ✅ | parity | P3 | hard | feasible | Desktop ID_PREVIEW_REPLACEMODEL / ReplaceModelDialog (#4462, rewritten to multi-target): source model + filtered checkbox target list, options keep-start-channel / keep-submodels / keep-size+pos. iPad: bridge `replaceModels:withSource:keepStartChannel:keepSubmodels:keepSizePosition:forDocument:` (XLMetalBridge.mm) — clones source via XmlSerializer per target, copies start-channel/controller/SR + submodels (SubModel copy ctor) + size/pos/rotation per the flags, atomic name swap, deletes old. ReplaceModelSheet (LayoutEditorView.swift, searchable multi-select + 3 toggles) launched from the InlineModelActionBar ↻ button. |
| Export as Custom / 3D Custom xLights model | menu | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop ID_PREVIEW_MODEL_EXPORTASCUSTOM(3D). iPad: import works, export missing. |
| Export xLights model (.xmodel) | menu | ✅ | ✅ | parity | P3 | medium | feasible | Desktop ID_PREVIEW_MODEL_EXPORTXLIGHTSMODEL. iPad: bridge `exportModelToXmodelFile:path:` (XLSequenceDocument.mm — `XmlSerializer::SerializeModel(m, true)` → `doc.save_file`, same call desktop's single-model export makes) + "Export as .xmodel…" in the model roster context menu (LayoutEditorView.swift) → temp-file + `.fileExporter` (`XmodelExportDoc` / `XmodelExportModifier`). Carries submodels / faces / states / aliases / dimming via the visitor. |
| Export CAD (DXF/STL/VRML) | menu | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop ID_PREVIEW_MODEL_CAD_EXPORT. iPad: no UI. |
| Export Faces/States/SubModels to other models | menu | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop ID_PREVIEW_EXPORT_FACESSTATESSUBMODELS. iPad: no UI. |
| Model import (.xmodel / multi-model) | dialog | ✅ | ✅ | parity | P1 | medium | feasible | iPad .fileImporter + multi-model handling (xmodelFileIsMultiModel). |
| Multi-model import preserves relative positions | dialog | ✅ | ✅ | parity | P3 | medium | feasible | Desktop #6438: importing several models from one .xmodel keeps their original relative layout positions. iPad already does this — `importXmodelFromPath:` (XLMetalBridge.mm:2520-2567) detects the `<models>` root, anchors the primary at the touch point, and re-applies each sibling's file-space `WorldPosX/Y/Z` delta from the primary (`primaryNew + (sibFilePos - primaryFilePos)`), the same min-corner-relative offset desktop's FinalizeModel computes. Verified at audit time; flipped. |
| Import fixture from GDTF file | dialog | ✅ | ✅ | parity | P2 | medium | feasible | Desktop GdtfParser → CreateDmxModelFromGdtf. iPad: `importXmodelFromPath:` (XLMetalBridge.mm) now branches on the `.gdtf` extension — `LoadGdtfDescriptionXml` unzips `description.xml` (minizip), then the shared core `XmlSerialize::ParseGdtfDescriptionXml` + `CreateDmxModelFromGdtfData` build the DMX model (same calls desktop makes), placed under the touch point via the existing import path. **Multi-mode fixtures now get a mode picker**: `gdtfModesForFile:` lists the DMX modes, `GdtfModePickerSheet` presents the chooser, and `importGdtfFromPath:mode:` forces the choice through ChooseFromList (`iPadGdtfModeCallbacks`); single-mode fixtures skip the sheet. |
| Import Previews/Models/Groups (from RGBeffects) | menu | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop ID_PREVIEW_IMPORTMODELSFROMRGBEFFECTS. iPad: no UI. |
| Import LOR S5 models/groups | menu | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop ID_PREVIEW_IMPORT_MODELS_FROM_LORS5. iPad: no UI. |
| Vendor model browser (download .xmodel/wiring) | dialog | ❌ | ✅ | desktop-missing | P2 | hard | feasible | iPad VendorBrowserSheet (XLVendorCatalog). Desktop downloads .xmodel manually. |
| Map-from-lights (FPP structured-light scan) | dialog | ❌ | ✅ | desktop-missing | P3 | hard | restricted | iPad MapFromLightsWizard (camera + FPP). Needs FPP controller + camera; desktop has no camera path. |
| Models report export (.xlsx) | menu | ✅ | ✅ | parity | P3 | medium | feasible | Desktop File→Export Models (xLightsMain.cpp:1122). iPad exportModelsReport + ModelsReportExportDoc. |
| Layout export As DXF | menu | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop ID_PREVIEW_LAYOUT_DXF_EXPORT. iPad: no UI. |
| Preview / layout-group select | panel | ✅ | ✅ | parity | P1 | easy | feasible | Desktop ChoiceLayoutGroups; iPad sidebar layout-group list. |
| Preview create | dialog | ✅ | ✅ | parity | P2 | medium | feasible | iPad createLayoutGroup + New Preview sheet. |
| Preview delete | menu | ✅ | ✅ | parity | P2 | medium | feasible | Desktop ID_PREVIEW_DELETE_ACTIVE. iPad: `deleteLayoutGroup:` (iPadRenderContext::DeleteNamedLayoutGroup reassigns members to "Unassigned", patches `<layoutGroups>` on save) + Delete-Preview menu item (LayoutEditorView.swift). Refuses "Default". |
| Preview rename | dialog | ✅ | ✅ | parity | P2 | medium | feasible | Desktop ID_PREVIEW_RENAME_ACTIVE. iPad: `renameLayoutGroup:to:` (iPadRenderContext::RenameNamedLayoutGroup reassigns members + patches `<layoutGroup>` name) + Rename-Preview sheet (LayoutEditorView.swift). |
| Preview background image picker | dialog | ✅ | ✅ | parity | P2 | medium | feasible | iPad Objects tab → 2D Background → backgroundImage .fileImporter. |
| Preview background brightness / alpha / scale-to-fit | panel | ✅ | 🟡 | ipad-weaker | P2 | easy | feasible | iPad intField backgroundBrightness (LayoutEditorView.swift:10617), backgroundAlpha (:10620), scaleBackgroundImage (:10624-10625); narrower than desktop's full display-options grid. |
| Preview 2D center-origin (X0=center) toggle | panel | ✅ | ✅ | parity | P3 | easy | feasible | iPad: `setDisplay2DCenter0:` (iPadRenderContext::SetDisplay2DCenter0 + `<settings>` patch) toggled from the Models-header Display menu (LayoutEditorView.swift). |
| Preview canvas size edit | panel | ✅ | ✅ | parity | P3 | easy | feasible | iPad: `setPreviewWidth:height:` (iPadRenderContext::SetPreviewSize + `<settings>` patch) via the "Edit Canvas / Grid…" sheet (LayoutEditorView.swift). |
| Preview 2D grid overlay toggle | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad PreviewSettings.showLayoutGrid → bridge. |
| Preview 2D grid spacing edit | panel | ✅ | ✅ | parity | P3 | easy | feasible | iPad: `setDisplay2DGridSpacing:` (iPadRenderContext::SetDisplay2DGridSpacing + `<settings>` patch) via the "Edit Canvas / Grid…" sheet (LayoutEditorView.swift). |
| Preview bounding-box overlay | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad showLayoutBoundingBox. |
| Preview model-name labels | panel/overlay | ✅ | ✅ | parity | P2 | easy | feasible | iPad showModelLabels (SwiftUI overlay). |
| Preview model-info labels (channels) | panel/overlay | ✅ | ✅ | parity | P2 | easy | feasible | iPad showModelInfo. |
| Preview 2D/3D toggle | panel | ✅ | ✅ | parity | P1 | easy | feasible | iPad settings.is3D + toolbar. |
| Preview pan / zoom / 3D orbit | gesture | ✅ | ✅ | parity | P1 | easy | feasible | iPad 1-finger drag / pinch / 3D orbit; desktop drag/wheel/right-drag. |
| Preview camera roll (twist) | gesture | 🟡 | ✅ | parity | P2 | easy | feasible | iPad two-finger rotate (Z twist). Desktop trackpad rotate. |
| Preview reset / fit view | gesture | ✅ | ✅ | parity | P2 | easy | feasible | iPad double-tap → resetCamera; desktop right-click Reset. |
| Preview first-pixel marker | panel | ✅ | ✅ | parity | P2 | medium | feasible | iPad showFirstPixel; desktop _showFirstPixel. |
| Preview snap-to-grid (drag) | panel | ✅ | ✅ | parity | P2 | easy | feasible | iPad PreviewSettings.snapToGrid. |
| Preview set center offset (group) | menu | ✅ | ✅ | parity | P3 | hard | feasible | Desktop ID_SET_CENTER_OFFSET. iPad: `setCenterOffsetForGroup:x:y:` (XLSequenceDocument.mm) replicates ModelPreview::SetCenterOffset's centre-bounds → ±1000 offset conversion + "Set Centre Offset" button in LayoutEditorGroupPropertiesView. |
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
| Polyline "Enter Segment Size" | dialog | ✅ | ✅ | parity | P3 | medium | feasible | Desktop ID_PREVIEW_MODEL_SET_SEGMENTS. iPad: `setSegmentSizeForModel:segment:size:` (XLSequenceDocument.mm → PolyLineModel::SetSegmentSize + Reinitialize) + "Set Segment Size…" entry in the poly-line segment long-press menu (LayoutEditorView.swift). |
| Model / group unlink from base show folder | menu/panel | ✅ | ✅ | parity | P1 | medium | feasible | iPad: bridge `unlinkModelFromBase:` / `unlinkGroupFromBase:` (XLSequenceDocument.mm) + context-menu entries in model roster and group list (LayoutEditorView.swift). Persistence: model path via SerialiseModel (BaseSerialisingVisitor writes FromBase); group path patches FromBase in SaveLayoutChanges() <modelGroups> branch. View-object unlink is out of scope (desktop-only). |
| Make Start Channel Valid (model) | menu | ✅ | ✅ | parity | P3 | hard | feasible | Desktop ID_MNU_MAKESCVALID. iPad: bridge `makeStartChannelValidForModel:` (XLSequenceDocument.mm — clears the controller name to NO_CONTROLLER when `!CouldComputeStartChannel || !IsValidStartChannelString`, then `RecalcStartChannels`, same trigger desktop uses) + "Make Start Channel Valid" in the model roster context menu (LayoutEditorView.swift). |
| Make All Start Channels Valid | menu | ✅ | ✅ | parity | P3 | hard | feasible | Desktop ID_MNU_MAKEALLSCVALID. iPad: bridge `makeAllStartChannelsValid` (same invalid-check across every non-group model) + roster context-menu entry. |
| Make All Start Channels Not Overlapping | menu | ✅ | ✅ | parity | P3 | hard | feasible | Desktop ID_MNU_MAKEALLSCNOTOVERLAPPING. iPad: bridge `makeAllStartChannelsNotOverlapping` (clears the controller name on every model where `ModelManager::IsModelOverlapping` is true, then recalcs) + roster context-menu entry. |
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
| SubModel import (from Model / File / Layout / Downloads / CSV) | dialog | ✅ | 🟡 | ipad-weaker | P2 | hard | feasible | Desktop SubModelsDialog.cpp:1126 ImportSubModel, :1138 ImportCSVSubModel, def :3537. **From Layout now wired** (`modelNamesInRGBEffectsFile:` + `submodelDetailsFromRGBEffectsFile:modelName:` → pick an external xlights_rgbeffects.xml, then `RGBEffectsModelPicker` chooses the model — same LoadSubModelsFromXml parse desktop's ReadRGBEffectsFile uses). iPad: four sources are wired — **From .xmodel File** (`submodelDetailsFromXmodelFile:` → core `XmlSerialize::LoadSubModelsFromXml`, same parse as desktop's ImportSubModelXML), **From Another Model** (`submodelDetailsFromModel:` copies a layout model's SubModel defs, source picked via `SubModelSourceModelPicker` over `modelNamesWithSubmodelsExcluding:`), and **From CSV** (`submodelDetailsFromCSVFile:`, each line compressed via `NodeUtils::CompressNodes`). All four return the `submodelDetailsForModel:` dict shape; the SubModelListSheet "Import SubModels…" menu merges (auto-uniquifying colliding names) and commits via `replaceSubModelsOnModel:`. Only the online **Downloads** (vendor catalog) source remains desktop-only. |
| SubModels output-to-lights live test toggle | dialog | ✅ | ❌ | ipad-missing | P2 | medium | feasible | Desktop SubModelsDialog output-to-lights. iPad startOutput (XLSequenceDocument.h:1650-1653) used only in SequencerViewModel, not in editors. |
| Real-world dimension readouts (ruler-calibrated) | panel | ✅ | ✅ | parity | P2 | medium | feasible | Desktop ScreenLocationProperties RealWidth/RealHeight/RealDepth via RulerObject::Measure. iPad: `modelLayoutSummary:` now adds a `realDimensions` sub-dict (width/height/depth + units) computed from `GetModelScreenLocation().GetRealWidth/Height/Depth()` whenever `RulerObject::GetRuler()` is non-null (a Ruler view-object has been placed). Surfaced as read-only "Real Width / Height / Depth" rows at the foot of the Size/Location section (`realDimensionRows`, LayoutEditorView.swift). Shared core, so the ruler calibration auto-applies. |
| Overlap checks toggle | panel | ✅ | ✅ | parity | P3 | easy | feasible | Desktop CheckBoxOverlap (LayoutPanel.cpp:636, :3456) + CheckModelForOverlaps (:7720). iPad: `modelsOverlappingModel:` (XLSequenceDocument.mm — same start/last-channel overlap test) + Overlap-Checks toggle in the Models-header Display menu; overlapping roster rows get an orange highlight + warning badge (LayoutEditorView.swift). |
| SubModel import from State / Face definitions | dialog | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop SubModelsDialog.cpp CreateSubmodel(name,nodes) (:3768) invoked from state/face context (:1064, :1117). iPad absent. |
| SubModel 'Import Custom Model Overlay' (matrix only) | dialog | ✅ | ❌ | ipad-missing | P3 | hard | feasible | Desktop SubModelsDialog.cpp:1132 ImportCustomModel, :3790/:3936/:4007 overlay helpers. iPad absent. |
| SubModels Symmetrize (rotational generator) | dialog | ✅ | ✅ | parity | P3 | hard | feasible | Algorithm extracted wx-free to `src-core/models/SubModelSymmetrize.{h,cpp}` (`Symmetrize` / `ShouldOfferSquarify`); desktop `SubModelsDialog::Symmetrize` (SubModelsDialog.cpp:1657) now calls it, keeping its file-log + interactive squarify / center-node prompts. iPad: bridge `symmetrizeRanges:onModel:degreeOfSymmetry:clockwise:bottomToTop:squarify:forDocument:` (XLMetalBridge.mm) feeds `Model::GetScreenLocations` over the SubModel preview into the core helper; SwiftUI `SubModelDetailEditor` Symmetrize button → option sheet (degree / CW-CCW / build-order / squarify, persisted via @AppStorage) → `SubmodelPreviewController.symmetrize`. |
| Faces output-to-lights live test toggle | dialog | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop ModelFaceDialog CheckBox_OutputToLights (:223). iPad startOutput not wired into the Faces editor. |
| States output-to-lights live test toggle | dialog | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop ModelStateDialog CheckBox_OutputToLights (:206). iPad startOutput not wired into the States editor. |
| RGBW PWM per-channel Brightness + Gamma | panel | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop DmxAbilityPropertyHelpers.cpp:265-320 per-color brightness/gamma. iPad XLSequenceDocument.mm:6278-6289 exposes RGBW channels only. |
| View object align / distribute / flip / resize | menu/toolbar | ✅ | ❌ | ipad-missing | P3 | medium | feasible | iPad alignModels/distributeModels/matchSizeOfModels/flipModels (XLMetalBridge.h:508-540) operate on models only, not view objects. Blocked on view-object **multi-select**: the iPad layout editor only supports single view-object selection (`layoutEditorSelectedObject`), and align/distribute needs 2+ items. Building object multi-select is the prerequisite, so this stays deferred (the model-side align/distribute is at parity). |
| Handle / crosshair scaling preferences | panel | ✅ | ✅ | parity | P3 | easy | feasible | Desktop adjustable handle size (xLightsFrame::GetModelHandleSize). iPad: `iPadModelPreview::GetHandleScale()` is now settable (1..10) via `XLMetalBridge.setHandleScale:`, seeded from `@AppStorage("layoutEditor.handleScale")` and surfaced as the "Handles N×" menu in LayoutEditorCanvasControls (LayoutEditorView.swift). Larger handles give bigger touch targets — the same `DrawHandles` scale arg desktop passes. |
| Zone indicator overlay | panel/overlay | ✅ | ✅ | parity | P3 | easy | feasible | Desktop 'Show Zone Indicator in Preview' (xLightsMain.h:1333, OtherSettingsPanel.cpp:203) drives `IModelPreview::GetShowZoneIndicator()`, read by DmxMovingHeadAdv. iPad: `iPadModelPreview::GetShowZoneIndicator()` is now settable via `XLMetalBridge.setShowZoneIndicator:` + a "Zones" toggle in LayoutEditorCanvasControls (LayoutEditorView.swift). Same shared-core draw path, so MovingHeadAdv position zones render identically. |
| Node tooltip on hover (2D) | overlay | ✅ | ✅ | parity | P3 | medium | feasible | Desktop layout hover shows node # / channel under the cursor. iPad: `nodeInfoNearPoint:viewSize:forDocument:` (XLMetalBridge.mm) walks every model, returns the hit node's number, absolute channel (NodeStartChannel+1), and controller/port (GetControllerName / GetControllerConnectionRangeString). Surfaced via a "Node Inspect" (scope) toggle in the LayoutEditor canvas controls: armed, a tap reports the node info in a bottom banner (`SequencerViewModel.nodeInspectHit`). Touch idiom for desktop's hover tooltip. |
| SpaceMouse 6-DOF input | other | ✅ | ❌ | ipad-missing | P3 | hard | infeasible | Desktop Mouse3DManager/SpaceMouseSession. No iOS HID SpaceMouse. |

## iPad gaps (desktop has, iPad missing)

### P2

- **Model Sets (layout move-linking).** Desktop links props into a
  persistent "Set" (`<modelSets>` in `xlights_rgbeffects.xml`,
  `src-core/models/ModelSetManager.h`) so dragging/rotating any member
  moves the whole Set; Alt-drag repositions a single member; a Set
  with a locked member is frozen. The data layer is in `src-core/` so
  the iPad already loads/saves it — the gap is honoring Set membership
  in `LayoutEditorView` drag gestures plus a management UI
  (link/unlink, member checklist). Desktop UI: `LayoutPanel.cpp`
  (`AddModelSetOptionsToMenu`, `DoLinkAsSet`, `DoManageSet`). See
  `plans/layout-group-move-lock.md`. Ease: medium.
- **Faces / States rich editor — visual node picker landed.** iPad
  `FaceStateEditorSheet` (`LayoutEditorView.swift`) edits the
  `faceInfo` / `stateInfo` attribute maps and is fully wired
  (`setFaceInfo:` / `setStateInfo:` / `faceInfoForModel:` /
  `stateInfoForModel:`). Each node-range attribute (face phoneme /
  outline parts, state `sNNN` node lists) now has a tap-to-pick
  button in `FaceStateEntryDetailView` → `NodeRangePickerSheet`
  (`NodePickerPane.swift`), which reuses `SubmodelPreviewPane`'s
  Metal node-picker (`XLMetalBridge.nodeNearPoint` /
  `setSubmodelHighlightedNodes`): tap toggles a node, two-finger
  long-press + drag paints a rectangle, and the result is compressed
  to a channel-range string via `SubModelRangeOps.compressNodes`
  (mirrors desktop `NodeUtils::CompressNodes`). This matches the
  desktop `ModelFaceDialog` / `ModelStateDialog` node assignment.
  Still open: **matrix-face image-channel mapping** (the
  `Mouth-…-Eyes…` image keys stay on the attr-map text path —
  `FaceStateAttr.isNodeRange` deliberately excludes them) and
  **xmodel face/state file-import** (`ModelFaceDialog::ImportFaces`,
  #6499). Ease: remaining work is medium (an image-assignment grid +
  a file-import bridge).
- **Preview delete / rename (layout group).** ✅ Done. Desktop
  `ID_PREVIEW_DELETE_ACTIVE` / `ID_PREVIEW_RENAME_ACTIVE`
  (`LayoutPanel.cpp:6319-6320`). iPad now has `deleteLayoutGroup:` /
  `renameLayoutGroup:to:` (backed by
  `iPadRenderContext::DeleteNamedLayoutGroup` /
  `RenameNamedLayoutGroup`, which reassign members to "Unassigned" on
  delete / to the new name on rename, mirroring desktop, and patch the
  `<layoutGroups>` element on save) + Delete / Rename Preview entries
  in the Models-section header menu.
- **GDTF fixture import.** ✅ Done. `importXmodelFromPath:`
  (`XLMetalBridge.mm`) branches on the `.gdtf` extension:
  `LoadGdtfDescriptionXml` unzips `description.xml` via minizip, then
  the shared core `XmlSerialize::ParseGdtfDescriptionXml` +
  `CreateDmxModelFromGdtfData` build the DMX model and the existing
  import path places it under the touch point. Multi-mode fixtures
  get a DMX-mode chooser: `gdtfModesForFile:` lists the modes,
  `GdtfModePickerSheet` presents them, and `importGdtfFromPath:mode:`
  forces the pick through `ChooseFromList` (`iPadGdtfModeCallbacks`).
  Single-mode fixtures skip straight to placement.
- **SubModel import (from Model / File / Layout / CSV).** 🟡 Mostly done.
  `submodelDetailsFromXmodelFile:` (core
  `XmlSerialize::LoadSubModelsFromXml`), `submodelDetailsFromModel:`
  (copy another layout model's SubModel defs, picked via
  `SubModelSourceModelPicker`), `submodelDetailsFromCSVFile:`
  (lines compressed via `NodeUtils::CompressNodes`), and **From Layout**
  (`modelNamesInRGBEffectsFile:` + `submodelDetailsFromRGBEffectsFile:modelName:`
  — pick an external `xlights_rgbeffects.xml`, then `RGBEffectsModelPicker`
  chooses the model, mirroring desktop `ReadRGBEffectsFile`) all return
  the `submodelDetailsForModel:` dict shape; the SubModelListSheet
  "Import SubModels…" menu merges them (auto-uniquifying colliding
  names) and commits via `replaceSubModelsOnModel:`. Only the online
  **Downloads** vendor catalog source remains desktop-only.
- **SubModels output-to-lights live test toggle.** Desktop drives the
  selected submodel to the lights for testing. iPad's `startOutput`
  (`XLSequenceDocument.h:1650-1653`) is used only by the sequencer, not
  the SubModel editor. Ease: medium.
- **Real-world dimension readouts (ruler-calibrated).** ✅ Done.
  `modelLayoutSummary:` adds a `realDimensions` sub-dict
  (width/height/depth + units) from
  `GetModelScreenLocation().GetRealWidth/Height/Depth()` whenever a
  Ruler view-object exists (`RulerObject::GetRuler() != nullptr`);
  SwiftUI shows read-only "Real Width / Height / Depth" rows at the
  foot of the Size/Location section (`realDimensionRows`).

### P3

- **Model copy/cut/paste to clipboard (layout).** ✅ Done. Desktop
  `DoCopy` / `DoPaste` (`LayoutPanel.cpp:8875`) round-trips
  `CopyPasteBaseObject` XML. iPad: `copyModelsToString:` /
  `pasteModelsFromString:` (XLMetalBridge.mm) serialize the selection
  to a `<models>` XML string and back (uniquify + clear controller +
  +50/+50 offset + RecalcStartChannels), wired to ⌘C / ⌘V / ⌘X in
  `LayoutClipboardAndGdtfModifier` (LayoutEditorView.swift). The
  pasteboard carries a custom `com.xlights.layoutmodels` UTI (declared
  in the xLights-iPad Info.plist) plus a plain-text fallback, so paste
  works **cross-sequence**. Touch-button surfaces (action-bar Copy) are
  a later nicety — the keyboard path is the desktop-parity surface.
- **Dimming-curve authoring.** iPad can edit the dimming attribute map
  and clear it, but the sheet explicitly says authoring a curve is
  desktop-only (`LayoutEditorView.swift:4081`). Desktop
  `ModelDimmingCurveDialog` has the graphical curve. Ease: hard.
- **Group clone, correct aspect ratio, set-center-offset,
  enter-segment-size.** ✅ Done. `cloneModelGroup:`,
  `correctAspectRatioForModel:`, `setCenterOffsetForGroup:x:y:`,
  `setSegmentSizeForModel:segment:size:` are bridged (replicating
  `ID_MNU_CLONE_MODEL_GROUP`, `ID_PREVIEW_MODEL_ASPECTRATIO`,
  `ID_SET_CENTER_OFFSET`, `ID_PREVIEW_MODEL_SET_SEGMENTS`) with
  context entries (group-properties Clone / Set-Centre buttons, the
  model action-bar aspect button, the poly-line segment menu's "Set
  Segment Size…"). **Model replace** (`ID_PREVIEW_REPLACEMODEL`)
  remains desktop-only. Ease: medium each.
- **Make-start-channel-valid / -all-valid / -not-overlapping (model +
  all).** ✅ Done. Bridge `makeStartChannelValidForModel:` /
  `makeAllStartChannelsValid` / `makeAllStartChannelsNotOverlapping`
  (XLSequenceDocument.mm) replicate the desktop handlers — clear the
  controller name (NO_CONTROLLER) on each offending model
  (`!CouldComputeStartChannel || !IsValidStartChannelString`, or
  `ModelManager::IsModelOverlapping`) then `RecalcStartChannels` — and
  are reachable from the model roster context menu.
- **Export-as-.xmodel** ✅ Done (`exportModelToXmodelFile:path:` →
  `XmlSerializer::SerializeModel(m, true)` + `.fileExporter`, roster
  context menu). **Export-as-custom / -3D-custom, CAD (DXF/STL/VRML),
  layout DXF, faces/states/submodels export, import-from-RGBeffects,
  import LOR-S5** remain desktop-only
  (`LayoutPanel.cpp:6164-6173`, `:6325-6327`). Most reuse shared
  serializers — needs file-exporter UI + bridge entry points. Ease:
  medium–hard.
- **Dedicated Bulk-Edit submenu items** (controller direction /
  colour-order / null-nodes / group-count / brightness / gamma /
  dimming). iPad applies the *property-pane* value across the selection
  (J-4) but doesn't expose the desktop's per-key bulk dialogs. The
  controller-specific keys aren't in the iPad property pane. Ease:
  medium. (Bulk Edit **Rotate X/Y/Z** is now done — Rotate menu in
  MultiSelectActionBar over bridge `rotateModels…`.)
- **2D center-0 toggle, canvas-size edit, global 2D grid-spacing edit.**
  ✅ Done. `setDisplay2DCenter0:`, `setPreviewWidth:height:`,
  `setDisplay2DGridSpacing:` (backed by `iPadRenderContext` setters +
  a top-level `<settings>` save patch) are wired to the Models-header
  Display menu's 2D-Centre toggle and the "Edit Canvas / Grid…" sheet.
- **Overlap checks toggle.** ✅ Done. Desktop `CheckBoxOverlap`
  (`LayoutPanel.cpp:636`, `:3456`) + `CheckModelForOverlaps` (`:7720`).
  iPad: `modelsOverlappingModel:` runs the same start/last-channel
  overlap test; a Display-menu toggle highlights the selected model's
  channel-overlapping neighbours (orange row tint + warning badge).
- **SubModel import from State / Face definitions** and **'Import Custom
  Model Overlay' (matrix only).** Desktop `SubModelsDialog.cpp`
  `CreateSubmodel` (`:3768`) from state/face context (`:1064`, `:1117`)
  and `ImportCustomModel` (`:1132`, `:3790`/`:3936`/`:4007`). iPad
  absent. Ease: medium / hard.
- **Faces / States output-to-lights live test toggle.** Desktop
  `ModelFaceDialog` `CheckBox_OutputToLights` (`:223`) and
  `ModelStateDialog` (`:206`) drive the lights from the editor. iPad's
  `startOutput` isn't wired into the Faces/States editors. Deliberately
  **not** bundled with the new node picker: `setSubmodelHighlightedNodes`
  colours the preview `Model`'s nodes but those colours aren't pushed to
  the output buffer, so a real live test needs a render/output loop
  (`startOutput` + per-frame node-colour push), which is heavier and
  risky to drive from a modal editor. Ease: medium.
- **RGBW PWM per-channel Brightness + Gamma.** Desktop
  `DmxAbilityPropertyHelpers.cpp:265-320` exposes per-color
  brightness/gamma. iPad (`XLSequenceDocument.mm:6278-6289`) exposes the
  RGBW channels only. Ease: medium.
- **View object align / distribute / flip / resize.** iPad
  `alignModels` / `distributeModels` / `matchSizeOfModels` /
  `flipModels` (`XLMetalBridge.h:508-540`) operate on models only;
  multi-select arrange isn't available for view objects. Ease: medium.
- **Handle / crosshair scaling preferences.** ✅ Done (handle size).
  `iPadModelPreview::GetHandleScale()` is now settable (1..10) via
  `XLMetalBridge.setHandleScale:`, seeded from
  `@AppStorage("layoutEditor.handleScale")` and exposed as the
  "Handles N×" menu in `LayoutEditorCanvasControls`. Crosshair size has
  no iPad-side equivalent yet (desktop draws a separate crosshair).
- **Zone indicator overlay.** ✅ Done. `iPadModelPreview::GetShowZoneIndicator()`
  is settable via `XLMetalBridge.setShowZoneIndicator:` + a "Zones"
  toggle in `LayoutEditorCanvasControls`; the shared-core
  `DmxMovingHeadAdv` draw path renders position zones identically to
  desktop.
- **Node tooltip on hover (2D).** ✅ Done (touch idiom).
  `nodeInfoNearPoint:viewSize:forDocument:` (`XLMetalBridge.mm`) walks
  every model and returns the hit node's number, absolute channel, and
  controller/port. A "Node Inspect" (scope) toggle in the LayoutEditor
  canvas controls arms a one-tap probe that reports the info in a
  bottom banner (`SequencerViewModel.nodeInspectHit`) — the touch
  equivalent of desktop's hover tooltip.
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

1. **Faces / States visual node picker (P2) — DONE.** Landed:
   `NodeRangePickerSheet` (`NodePickerPane.swift`) layers a tap-to-pick
   node-assignment view over the model preview by reusing
   `SubmodelPreviewPane`'s Metal node-picker, reachable from each
   node-range attribute in `FaceStateEntryDetailView`. Selections
   round-trip through `SubModelRangeOps.compressNodes`. Remaining
   Faces sub-gaps (matrix-face **image** assignment grid, xmodel
   face/state **file-import**) are smaller follow-ups, not the
   core node-picking gap.
2. **Preview delete / rename (P2)** — tiny bridge additions
   (`deleteLayoutGroup` / `renameLayoutGroup`) + two sidebar actions;
   completes preview/layout-group CRUD which is otherwise create-only.
3. **2D center-0 / canvas-size / grid-spacing editable rows (P3, easy)**
   — quick wins; the values are already surfaced read-only, just need
   setters and editable controls.
4. **Node Layout / Wiring View (P2)** — *done.* `NodeLayoutSheet`
   (zoom/pan Canvas) reads per-node buffer geometry from the new
   `nodeLayout(forModel:)` bridge accessor; the node-grid is at parity
   and the wiring path/front-rear toggle lands the WiringDialog basics
   (print + DXF export stay in the deep-dialog cluster). **Viewpoint
   save/restore (P2)** is likewise already landed (parity).
5. **Export family + make-SC-valid + group-clone / replace / aspect /
   center-offset / segment-size (P3)** — group-clone, correct-aspect,
   set-center-offset, enter-segment-size, model replace, **export-as-
   .xmodel**, and **make-SC-valid / all-valid / all-not-overlapping**
   are ✅ done (single-shot bridge wrappers + context entries). Also
   landed this pass: **GDTF fixture import**, **SubModel import** (from
   .xmodel file / another model / CSV), and **ruler-calibrated real-
   world dimension readouts**. Remaining export/import: export-as-
   custom / -3D-custom, CAD (DXF/STL/VRML), layout DXF, faces/states/
   submodels export, import-from-RGBeffects, import-LOR-S5, and the
   online SubModel **Downloads** / RGBeffects-pull **Layout** sources.
6. **Layout clipboard copy/paste (P3) — DONE.** ⌘C/⌘V/⌘X round-trip
   model XML through a `com.xlights.layoutmodels` pasteboard UTI
   (`copyModelsToString:` / `pasteModelsFromString:`), so paste works
   within and **across** sequences. Also landed this pass: the **GDTF
   multi-mode picker**, **SubModel import From Layout**, the
   **node-inspect tap tooltip**, and the **handle-size + zone-indicator**
   canvas-control toggles. Remaining P3 holdouts: **dimming-curve
   authoring** (Duplicate + clear-dimming cover the common cases) and
   **view-object align/distribute** (needs view-object multi-select
   infrastructure the iPad doesn't have yet — single-select only).
