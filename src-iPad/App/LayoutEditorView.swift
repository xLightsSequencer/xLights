import SwiftUI
import UniformTypeIdentifiers

/// J-5 (sidebar tabs) — Layout Editor left-pane roster picker.
/// Controllers tab is intentionally omitted in J-5; iPad doesn't
/// yet have a controller editor and surfacing the tab with no UI
/// behind it would be a regression in clarity.
enum LayoutSidebarTab: String, CaseIterable, Identifiable {
    case models, groups, objects
    var id: String { rawValue }
    var label: String {
        switch self {
        case .models: return "Models"
        case .groups: return "Groups"
        case .objects: return "Objects"
        }
    }
    var systemImage: String {
        switch self {
        case .models: return "cube"
        case .groups: return "square.stack.3d.up"
        case .objects: return "scribble.variable"
        }
    }
}

/// Phase J-0 / J-1 — Layout Editor screen. Opens via Tools → Edit
/// Layout… in its own `WindowGroup("layout-editor")` scene. The
/// user picks a layout group, sees its models in a Metal canvas,
/// and selects a model from the side panel to inspect or edit its
/// common-properties surface (J-1: centre, dimensions, rotation,
/// locked, layout group, controller name).
///
/// J-2 adds tap-to-select on the canvas, drag/resize/rotate
/// handles, and overlay rendering (names, first-pixel, grid,
/// bounding boxes). J-3 adds Add Model + per-type properties.
/// J-4 adds multi-select align/distribute/flip/resize-to-match.
/// See `plans/phase-j-layout-editor.md`.
struct LayoutEditorView: View {
    @Environment(SequencerViewModel.self) var viewModel

    /// Cached at view onAppear / show-folder change so SwiftUI re-
    /// renders when the underlying C++ render context swaps groups.
    /// `viewModel.document` is a stable reference; reading it inside
    /// `body` won't re-fire when its internals change.
    @State private var layoutGroups: [String] = ["Default"]
    @State private var activeLayoutGroup: String = "Default"
    @State private var modelNames: [String] = []
    @State private var displayState: [String: Any] = [:]
    @State private var settings = PreviewSettings(is3DDefault: true,
                                                  showViewObjectsDefault: true)
    @State private var controlsVisible: Bool = false
    /// Bumped by every successful `setLayoutModelProperty` call so
    /// the summary view re-reads from the bridge. Avoids holding
    /// our own copy of the property snapshot in @State (which
    /// could drift from the live Model behind the bridge).
    @State private var summaryToken: Int = 0
    /// Mirror of `document.hasUnsavedLayoutChanges` — Swift can't
    /// observe ObjC method results, so we update this manually
    /// after every set + save.
    @State private var hasUnsavedChanges: Bool = false
    @State private var saveErrorMessage: String? = nil
    /// J-2 — mirrors `document.canUndoLayoutChange`. Updated after
    /// every edit + undo so the toolbar button can grey itself out.
    @State private var canUndo: Bool = false
    /// J-2 — confirm-before-save. The toolbar Save button flips
    /// this on; the alert's primary action calls saveLayoutChanges
    /// for real. Avoids accidental writes to xlights_rgbeffects.xml.
    @State private var showingSaveConfirm: Bool = false
    /// J-2 — Layout-Editor-only overlay toggles. Initial values are
    /// seeded from rgbeffects.xml on first draw inside the bridge;
    /// these mirror the bridge state so the SwiftUI toggle UI
    /// reflects what the user sees on the canvas.
    @State private var showLayoutGrid: Bool = false
    @State private var showLayoutBoundingBox: Bool = false
    @State private var overlaysInitialized: Bool = false
    /// Phase J-2 (touch UX) — long-press contextual menu target.
    /// Set by the `.layoutEditorContextMenu` notification; cleared
    /// when the user picks an item or dismisses. The
    /// `.confirmationDialog` shows the appropriate item set.
    @State private var contextMenuTarget: LayoutContextMenuTarget? = nil
    /// Phase J-3 (touch UX) — cached list of model-type strings
    /// the Add-Model picker shows. Read from the bridge at
    /// refresh time so changes to the curated list don't require
    /// a rebuild.
    @State private var availableModelTypes: [String] = []
    /// J-3 (touch UX) — drives the Add-Model sheet. Using a sheet
    /// instead of an inline Menu avoids whatever launch-time issue
    /// the SwiftUI Menu in the canvas overlay triggers.
    @State private var addModelSheetVisible: Bool = false
    /// J-3 (touch UX) — pending delete confirmation. Set when the
    /// user taps the trash icon in the inline action bar; cleared
    /// after the alert resolves either way.
    @State private var pendingDeleteModelName: String? = nil
    /// J-4 (import) — drives the .xmodel file picker.
    @State private var importerVisible: Bool = false
    /// J-4 (import) — set when a fresh import fails (file unreadable,
    /// XML parse failure, unknown model type). Surfaced as an alert.
    @State private var importErrorMessage: String? = nil
    /// J-4 (download) — drives the vendor catalog browser sheet.
    @State private var downloadBrowserVisible: Bool = false

    /// J-5 (sidebar tabs) — which roster the top half of the sidebar
    /// shows. Each tab keeps its own selection so flipping tabs to
    /// inspect a group or object doesn't lose the model selection
    /// driving the canvas.
    @State private var sidebarTab: LayoutSidebarTab = .models
    /// J-5 — fraction of the sidebar's height occupied by the top
    /// roster (list). The bottom (property pane) takes the rest.
    /// Persists per-session via @State; clamped to [0.2, 0.8] by the
    /// divider's drag handler so neither pane vanishes.
    @State private var sidebarTopFraction: CGFloat = 0.45
    /// J-5 — Groups roster. The selection itself lives on the
    /// view model (`layoutEditorSelectedGroup`) so PreviewPaneView
    /// can sync it to the canvas tint.
    @State private var groupNames: [String] = []
    /// J-5 — ViewObjects roster. Selection on the view model so
    /// the canvas picks up handles.
    @State private var objectNames: [String] = []
    /// J-5 — search text per tab. Reset on editor close.
    @State private var modelFilter: String = ""
    @State private var groupFilter: String = ""
    @State private var objectFilter: String = ""

    /// J-7 (group CRUD) — sheet visibility flags + targets.
    @State private var newGroupSheetVisible: Bool = false
    @State private var addMemberSheetVisible: Bool = false
    @State private var pendingDeleteGroupName: String? = nil

    /// J-16 (group rename) — rename-sheet visibility + target.
    @State private var renameGroupSheetVisible: Bool = false
    @State private var pendingRenameGroupOldName: String? = nil

    /// J-7 (multi-select) — when true, the next NewGroupSheet
    /// "Create" passes the current selection as the initial member
    /// list instead of creating an empty group. Cleared after use.
    @State private var pendingGroupFromSelection: [String]? = nil

    /// J-8 — file picker for the 2D Background image.
    @State private var backgroundImagePickerVisible: Bool = false
    // J-20.2 — Image model file picker.
    @State private var imageFilePickerVisible: Bool = false
    @State private var pendingImagePickKey: String = ""
    @State private var pendingImagePickModel: String = ""

    // J-23 — Custom-model visual editor payload. When non-nil
    // the `.sheet(item:)` opens the editor and the payload
    // carries the captured-at-open grid + dims + bg image.
    @State private var customModelEditorPayload: CustomModelPayload? = nil

    /// J-12 — generic view-object file picker (Mesh `.obj`,
    /// Image bitmap, Terrain image). `pendingObjectFilePickKey`
    /// tracks which property key to write on completion;
    /// `objectFilePickerTypes` controls the allowed UTTypes.
    @State private var objectFilePickerVisible: Bool = false
    @State private var objectFilePickerTypes: [UTType] = []
    @State private var pendingObjectFilePickKey: String? = nil

    /// J-12 — Add View Object sheet visibility.
    @State private var addViewObjectSheetVisible: Bool = false
    /// J-12 — pending delete confirmation for a view object.
    @State private var pendingDeleteObjectName: String? = nil

    /// J-17 (view object rename) — rename sheet visibility + target.
    @State private var renameObjectSheetVisible: Bool = false
    @State private var pendingRenameObjectOldName: String? = nil

    /// J-18 (model rename) — rename sheet visibility + target.
    @State private var renameModelSheetVisible: Bool = false
    @State private var pendingRenameModelOldName: String? = nil

    /// J-4 (import) — UTTypes the .fileImporter accepts. Declared
    /// as a static so the SwiftUI type-checker can resolve the
    /// `[UTType]` literal in reasonable time (the `?? .data` chain
    /// inline tripped its budget).
    private static let importableModelTypes: [UTType] = {
        ["xmodel", "gdtf", "lff", "lpf"].compactMap {
            UTType(filenameExtension: $0)
        }
    }()

    /// J-12 — Add-View-Object sheet → bridge create. Auto-
    /// selects the new object so the property pane opens on
    /// the right summary.
    private func handleCreateViewObject(_ type: String) {
        addViewObjectSheetVisible = false
        guard let name = viewModel.document.createViewObject(withType: type) else { return }
        viewModel.layoutEditorSelectedObject = name
        hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
        refreshModelList()
        NotificationCenter.default.post(name: .layoutEditorModelMoved,
                                         object: "LayoutEditor",
                                         userInfo: ["model": name])
    }

    /// J-18 — model rename sheet "Rename" callback. Re-points
    /// the sidebar selection to the new name on success so the
    /// property pane reopens on the renamed model.
    private func handleRenameModel(_ oldName: String, _ newName: String) {
        guard viewModel.document.renameModel(oldName, to: newName) else { return }
        if viewModel.layoutEditorSelectedModel == oldName {
            viewModel.layoutSelectSingle(newName)
        }
        summaryToken &+= 1
        hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
        refreshModelList()
    }

    /// J-17 — view-object rename sheet "Rename" callback.
    private func handleRenameViewObject(_ oldName: String, _ newName: String) {
        guard viewModel.document.renameViewObject(oldName, to: newName) else { return }
        if viewModel.layoutEditorSelectedObject == oldName {
            viewModel.layoutEditorSelectedObject = newName
        }
        summaryToken &+= 1
        hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
        refreshModelList()
    }

    /// J-17 — duplicate the named view object and auto-select
    /// the new copy so the user can immediately drag it into
    /// position.
    private func handleDuplicateViewObject(_ name: String) {
        guard let dup = viewModel.document.duplicateViewObject(name) else { return }
        viewModel.layoutEditorSelectedObject = dup
        summaryToken &+= 1
        hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
        refreshModelList()
        NotificationCenter.default.post(name: .layoutEditorModelMoved,
                                         object: "LayoutEditor",
                                         userInfo: ["model": dup])
    }

    /// J-12 — Delete-confirmation alert → bridge delete.
    private func handleDeleteViewObject(_ name: String) {
        guard viewModel.document.deleteViewObject(name) else { return }
        if viewModel.layoutEditorSelectedObject == name {
            viewModel.layoutEditorSelectedObject = nil
        }
        hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
        refreshModelList()
        NotificationCenter.default.post(name: .layoutEditorModelMoved,
                                         object: "LayoutEditor",
                                         userInfo: ["model": name])
    }

    /// J-12 — Per-type file picker completion. Routes the picked
    /// path back through commitObjectProperty under whatever key
    /// the property pane stashed in `pendingObjectFilePickKey`.
    private func handleObjectFilePick(_ result: Result<[URL], Error>) {
        let key = pendingObjectFilePickKey ?? ""
        pendingObjectFilePickKey = nil
        guard !key.isEmpty,
              let name = viewModel.layoutEditorSelectedObject else { return }
        switch result {
        case .success(let urls):
            guard let url = urls.first else { return }
            let granted = url.startAccessingSecurityScopedResource()
            let path = url.path
            _ = XLSequenceDocument.obtainAccess(toPath: path, enforceWritable: false)
            if granted { url.stopAccessingSecurityScopedResource() }
            commitObjectProperty(objectName: name, key: key, value: path as NSString)
        case .failure(let err):
            saveErrorMessage = err.localizedDescription
        }
    }

    /// J-8 — `.fileImporter` completion for the 2D Background
    /// image. Same security-scoped access dance as the model
    /// importer: start access, persist the bookmark via
    /// ObtainAccessToURL, stop access, then push the path
    /// through the regular object-property commit so the
    /// summaryToken bumps and the canvas repaints.
    private func handleBackgroundImagePick(_ result: Result<[URL], Error>) {
        switch result {
        case .success(let urls):
            guard let url = urls.first else { return }
            let granted = url.startAccessingSecurityScopedResource()
            let path = url.path
            _ = XLSequenceDocument.obtainAccess(toPath: path, enforceWritable: false)
            if granted { url.stopAccessingSecurityScopedResource() }
            commitObjectProperty(objectName: "2D Background",
                                 key: "backgroundImage",
                                 value: path as NSString)
        case .failure(let err):
            saveErrorMessage = err.localizedDescription
        }
    }

    /// J-20.2 — Image-model file picker completion. The key was
    /// captured when the user tapped the folder button on the
    /// `imageFile` descriptor row; `pendingImagePickModel` is
    /// set so we can route the per-type commit to the right
    /// model even if selection drifts before the picker returns.
    private func handleImageFilePick(_ result: Result<[URL], Error>) {
        switch result {
        case .success(let urls):
            guard let url = urls.first,
                  let modelName = viewModel.layoutEditorSelectedModel else { return }
            let granted = url.startAccessingSecurityScopedResource()
            let path = url.path
            _ = XLSequenceDocument.obtainAccess(toPath: path, enforceWritable: false)
            if granted { url.stopAccessingSecurityScopedResource() }
            commitPerTypeProperty(modelName: modelName,
                                   key: pendingImagePickKey,
                                   value: path as NSString)
        case .failure(let err):
            saveErrorMessage = err.localizedDescription
        }
    }

    /// J-4 (import) — `.fileImporter` completion. Stashes the
    /// picked path for the next canvas-tap to consume.
    private func handleImportPick(_ result: Result<[URL], Error>) {
        switch result {
        case .success(let urls):
            guard let url = urls.first else { return }
            let granted = url.startAccessingSecurityScopedResource()
            let path = url.path
            _ = XLSequenceDocument.obtainAccess(toPath: path, enforceWritable: false)
            if granted { url.stopAccessingSecurityScopedResource() }
            viewModel.layoutPendingImportPath = path
        case .failure(let err):
            importErrorMessage = err.localizedDescription
        }
    }

    var body: some View {
        @Bindable var vm = viewModel
        NavigationSplitView {
            sidebar
                .navigationSplitViewColumnWidth(min: 240, ideal: 320, max: 420)
        } detail: {
            canvas
        }
        .navigationTitle("Edit Layout")
        // NavigationSplitView in this scene hides its column chrome,
        // so `.toolbar`'s `.primaryAction` slots never render.
        // Layout-group switcher / Undo / Save / Add Model / Select
        // all live in the canvas overlay (top-left + top-right) so
        // they're actually reachable.
        .onAppear { refresh() }
        .onDisappear {
            // Drop any half-started Add-Model so the editor opens
            // fresh next time rather than re-entering creation mode
            // on a stale type.
            viewModel.layoutPendingNewModelType = nil
            viewModel.layoutPolylineInProgress = nil
            viewModel.layoutPendingImportPath = nil
            // J-4 (multi-select) — exit edit mode and collapse to
            // the primary so the next open starts in a known
            // single-select state.
            viewModel.layoutEditMode = false
            if let primary = viewModel.layoutEditorSelectedModel {
                viewModel.layoutEditorSelection = [primary]
            } else {
                viewModel.layoutEditorSelection.removeAll()
            }
            // J-6 — clear group / object picks so a re-open starts
            // clean (matches model-selection lifecycle).
            viewModel.layoutEditorSelectedGroup = nil
            viewModel.layoutEditorSelectedObject = nil
            // J-13 — exit any active terrain edit session so the
            // next open doesn't paint heightmap data on the first
            // tap.
            viewModel.terrainEditTarget = nil
        }
        .onChange(of: viewModel.isShowFolderLoaded) { _, _ in refresh() }
        .onChange(of: activeLayoutGroup) { _, newValue in
            viewModel.document.setActiveLayoutGroup(newValue)
            refreshModelList()
        }
        .onChange(of: viewModel.layoutEditorSelectedModel) { _, newSelection in
            // Seed the toolbar tool from the newly-selected
            // model's axis_tool so switching models doesn't leak
            // the previous selection's mode.
            if let name = newSelection, !name.isEmpty {
                let current = viewModel.document.axisTool(forModel: name)
                if current != "none" {
                    settings.axisTool = current
                }
            }
        }
        .modifier(SidebarSelectionMutex(sidebarTab: $sidebarTab))
        .focusable()
        // J-2 — keyboard nudge for the selected model. Arrow keys
        // move 1 unit, shift+arrow moves 10. Pushed undo per nudge
        // so each tap is independently reversible.
        .onKeyPress(.upArrow,    phases: .down) { keyPress in nudge(0,  +1, keyPress) }
        .onKeyPress(.downArrow,  phases: .down) { keyPress in nudge(0,  -1, keyPress) }
        .onKeyPress(.leftArrow,  phases: .down) { keyPress in nudge(-1,  0, keyPress) }
        .onKeyPress(.rightArrow, phases: .down) { keyPress in nudge(+1,  0, keyPress) }
        // J-3 (touch UX) — Esc/Return end mid-polyline creation
        // (mirrors desktop's polyline create commit hot-keys).
        // Also drops fresh-model placement mode if armed.
        .onKeyPress(.escape, phases: .down) { _ in endCreationModes() }
        .onKeyPress(.return, phases: .down) { _ in endCreationModes() }
        .onReceive(NotificationCenter.default.publisher(for: .layoutEditorModelMoved)) { note in
            // Drag-to-move on the canvas (or a keyboard nudge / undo)
            // mutates the bridge directly; refresh the summary +
            // dirty-state so the side panel reflects the new centre
            // and the Save button enables. Convention: object =
            // previewName ("LayoutEditor"), userInfo["model"] = name.
            let movedName = note.userInfo?["model"] as? String
            // Add Model creates a model the side panel hasn't seen
            // yet — refresh the model list so the new name appears.
            if let name = movedName, !modelNames.contains(name) {
                refreshModelList()
            }
            guard movedName == viewModel.layoutEditorSelectedModel else { return }
            summaryToken &+= 1
            hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
            canUndo = viewModel.document.canUndoLayoutChange()
        }
        .onReceive(NotificationCenter.default.publisher(for: .layoutEditorContextMenu)) { note in
            // Long-press on a handle. The bridge has already done the
            // hit-test and packaged the result. Translate to our
            // typed target and let the confirmationDialog open.
            guard let info = note.userInfo,
                  let type = info["type"] as? String,
                  let modelName = info["modelName"] as? String else { return }
            switch type {
            case "vertex":
                if let idx = (info["vertexIndex"] as? NSNumber)?.intValue {
                    contextMenuTarget = .vertex(modelName: modelName, vertexIndex: idx)
                }
            case "segment":
                if let idx = (info["segmentIndex"] as? NSNumber)?.intValue {
                    let curve = (info["hasCurve"] as? NSNumber)?.boolValue ?? false
                    contextMenuTarget = .segment(modelName: modelName,
                                                  segmentIndex: idx,
                                                  hasCurve: curve)
                }
            case "curve_control":
                if let idx = (info["segmentIndex"] as? NSNumber)?.intValue {
                    contextMenuTarget = .curveControl(modelName: modelName,
                                                       segmentIndex: idx)
                }
            default:
                break
            }
        }
        // J-3 (touch UX) — Pencil Pro squeeze maps to layout undo.
        // Posted by PreviewPaneView's UIPencilInteraction; same
        // entry point as the toolbar's Undo button so undo state /
        // dirty markers / canvas repaint stay consistent.
        .onReceive(NotificationCenter.default.publisher(for: .layoutEditorPencilUndo)) { _ in
            if canUndo { performUndo() }
        }
        .confirmationDialog(contextMenuTarget?.title ?? "",
                            isPresented: Binding(
                                get: { contextMenuTarget != nil },
                                set: { if !$0 { contextMenuTarget = nil } }),
                            titleVisibility: .visible) {
            contextMenuButtons
        }
        .sheet(isPresented: $addModelSheetVisible) {
            AddModelSheet(types: availableModelTypes,
                           labelFor: modelTypeLabel) { type in
                viewModel.layoutPendingNewModelType = type
                addModelSheetVisible = false
            }
        }
        // J-4 (download) — vendor catalog browser. On download
        // we reuse the same `layoutPendingImportPath` path Import
        // uses, so the user gets the familiar "tap canvas to
        // place" banner regardless of where the file came from.
        .sheet(isPresented: $downloadBrowserVisible) {
            VendorBrowserSheet(onDownloaded: { path in
                viewModel.layoutPendingImportPath = path
                downloadBrowserVisible = false
            })
        }
        .modifier(GroupCrudModifiers(
            newGroupSheetVisible: $newGroupSheetVisible,
            addMemberSheetVisible: $addMemberSheetVisible,
            pendingDeleteGroupName: $pendingDeleteGroupName,
            renameGroupSheetVisible: $renameGroupSheetVisible,
            pendingRenameGroupOldName: $pendingRenameGroupOldName,
            groupNames: groupNames,
            modelNames: modelNames,
            onCreateGroup: handleCreateGroup,
            onAddMembers: handleAddMembers,
            onDeleteGroup: handleDeleteGroup,
            onRenameGroup: handleRenameGroup,
            selectedGroupName: viewModel.layoutEditorSelectedGroup,
            currentMembers: selectedGroupMembers,
            submodelsFor: { parent in
                viewModel.document.submodels(forModel: parent)
            },
            sanitizeName: { raw in
                viewModel.document.sanitizedModelName(raw)
            }
        ))
        // J-4 (import) — .xmodel file picker. iPadOS's
        // `.fileImporter` is the SwiftUI-native wrapping of
        // UIDocumentPickerViewController; the resulting URL has
        // security-scoped access already, so we call
        // `ObtainAccessToURL` to persist the bookmark and stash
        // the path for the next canvas-tap placement.
        .fileImporter(isPresented: $importerVisible,
                      allowedContentTypes: Self.importableModelTypes,
                      allowsMultipleSelection: false,
                      onCompletion: handleImportPick)
        // J-8 — 2D Background image picker. Accepts common image
        // formats; the bridge calls `ObtainAccessToURL` so the
        // sandbox bookmark persists across launches.
        .fileImporter(isPresented: $backgroundImagePickerVisible,
                      allowedContentTypes: [.png, .jpeg, .tiff, .bmp, .gif, .image],
                      allowsMultipleSelection: false,
                      onCompletion: handleBackgroundImagePick)
        // J-20.2 — Image-model file picker. Same security-scoped
        // dance as the background importer; on success, route
        // through the per-type commit so the descriptor pipeline
        // picks up the new path.
        .fileImporter(isPresented: $imageFilePickerVisible,
                      allowedContentTypes: [.png, .jpeg, .tiff, .bmp, .gif, .image],
                      allowsMultipleSelection: false,
                      onCompletion: handleImageFilePick)
        // J-23 / J-23.4 — Custom-model visual editor. Full-screen
        // cover on iPad so the canvas gets the whole window (the
        // detent-clamped sheet was way too small for placing
        // pixels on dense grids).
        .fullScreenCover(item: $customModelEditorPayload) { payload in
            CustomModelEditorSheet(payload: payload,
                                   commit: { w, h, d, locs in
                                       commitCustomModelGrid(
                                           modelName: payload.modelName,
                                           w: w, h: h, d: d, locations: locs)
                                   })
        }
        .sheet(isPresented: $renameModelSheetVisible) {
            if let oldName = pendingRenameModelOldName {
                RenameGroupSheet(
                    oldName: oldName,
                    existingNames: Set(
                        (modelNames + groupNames + objectNames).filter { $0 != oldName }
                    ),
                    onRename: { newName in
                        handleRenameModel(oldName, newName)
                        renameModelSheetVisible = false
                        pendingRenameModelOldName = nil
                    },
                    onCancel: {
                        renameModelSheetVisible = false
                        pendingRenameModelOldName = nil
                    },
                    sanitize: { raw in
                        viewModel.document.sanitizedModelName(raw)
                    },
                    kindLabel: "Model"
                )
            }
        }
        .modifier(ViewObjectCrudModifiers(
            objectFilePickerVisible: $objectFilePickerVisible,
            objectFilePickerTypes: objectFilePickerTypes,
            addViewObjectSheetVisible: $addViewObjectSheetVisible,
            pendingDeleteObjectName: $pendingDeleteObjectName,
            renameObjectSheetVisible: $renameObjectSheetVisible,
            pendingRenameObjectOldName: $pendingRenameObjectOldName,
            availableTypes: viewModel.document.availableViewObjectTypes(),
            onCreateObject: handleCreateViewObject,
            onDeleteObject: handleDeleteViewObject,
            onRenameObject: handleRenameViewObject,
            onFilePicked: handleObjectFilePick,
            existingNames: Set(objectNames + modelNames + groupNames),
            sanitizeName: { raw in
                viewModel.document.sanitizedModelName(raw)
            }
        ))
        .alert("Import failed",
               isPresented: Binding(get: { importErrorMessage != nil },
                                    set: { if !$0 { importErrorMessage = nil } })) {
            Button("OK", role: .cancel) { }
        } message: {
            Text(importErrorMessage ?? "")
        }
        .alert("Save failed",
               isPresented: Binding(get: { saveErrorMessage != nil },
                                    set: { if !$0 { saveErrorMessage = nil } })) {
            Button("OK", role: .cancel) { }
        } message: {
            Text(saveErrorMessage ?? "")
        }
        // J-3 (touch UX) — delete-model confirmation. Triggered by
        // the trash icon in the inline action bar. The delete is
        // an in-memory mutation through the bridge; user must still
        // hit Save to persist (matches every other layout edit).
        .alert("Delete \(pendingDeleteModelName ?? "")?",
               isPresented: Binding(get: { pendingDeleteModelName != nil },
                                    set: { if !$0 { pendingDeleteModelName = nil } })) {
            Button("Delete", role: .destructive) {
                if let name = pendingDeleteModelName {
                    deleteModel(name: name)
                }
            }
            Button("Cancel", role: .cancel) { }
        } message: {
            Text("Removes this model from the current layout. Save the layout to make the change permanent; Undo or Discard will roll it back.")
        }
        // Confirm-before-save. xlights_rgbeffects.xml is the show's
        // master layout file; an unintended save during testing is
        // expensive to recover from. Discard-changes uses the undo
        // stack to roll back every staged in-memory mutation.
        .confirmationDialog("Save layout changes?",
                            isPresented: $showingSaveConfirm,
                            titleVisibility: .visible) {
            Button("Save to xlights_rgbeffects.xml") {
                saveLayoutChanges()
            }
            Button("Discard Changes", role: .destructive) {
                discardChanges()
            }
            Button("Cancel", role: .cancel) { }
        } message: {
            Text("Saving overwrites the show's xlights_rgbeffects.xml. Discarding rolls back every in-memory edit through the undo stack.")
        }
    }

    // MARK: - Sidebar (J-5: vertical split, tabbed roster + property pane)

    @ViewBuilder
    private var sidebar: some View {
        GeometryReader { geo in
            let total = geo.size.height
            let topHeight = max(120, min(total - 160, total * sidebarTopFraction))
            VStack(spacing: 0) {
                rosterPane
                    .frame(height: topHeight)
                sidebarDivider(totalHeight: total)
                propertyPane
                    .frame(maxHeight: .infinity)
            }
        }
    }

    /// Top half: tab picker + search + filtered list. The list's
    /// selection is routed back to the right `@State` via
    /// `rosterSelectionBinding` so each tab keeps its own.
    @ViewBuilder
    private var rosterPane: some View {
        VStack(spacing: 0) {
            Picker("", selection: $sidebarTab) {
                ForEach(LayoutSidebarTab.allCases) { tab in
                    Label(tab.label, systemImage: tab.systemImage).tag(tab)
                }
            }
            .pickerStyle(.segmented)
            .padding(.horizontal, 8)
            .padding(.top, 6)
            .padding(.bottom, 4)

            // Per-tab search field. Trimming + case-insensitive
            // contains is enough for the largest realistic show
            // (~500 models); switch to substring index if it ever
            // turns into a bottleneck.
            HStack(spacing: 6) {
                Image(systemName: "magnifyingglass")
                    .foregroundStyle(.secondary)
                    .font(.caption)
                TextField("Filter", text: currentFilterBinding)
                    .textFieldStyle(.plain)
                    .font(.caption)
                    .autocorrectionDisabled()
                    .textInputAutocapitalization(.never)
                if !currentFilterBinding.wrappedValue.isEmpty {
                    Button {
                        currentFilterBinding.wrappedValue = ""
                    } label: {
                        Image(systemName: "xmark.circle.fill")
                            .foregroundStyle(.secondary)
                    }
                    .buttonStyle(.plain)
                }
            }
            .padding(.horizontal, 10)
            .padding(.vertical, 4)
            .background(.quaternary, in: RoundedRectangle(cornerRadius: 6, style: .continuous))
            .padding(.horizontal, 8)
            .padding(.bottom, 4)

            rosterList
        }
    }

    @ViewBuilder
    private var rosterList: some View {
        switch sidebarTab {
        case .models:
            List(selection: modelListBinding) {
                Section("\(activeLayoutGroup) (\(filteredModelNames.count)\(modelNames.count != filteredModelNames.count ? " of \(modelNames.count)" : ""))") {
                    ForEach(filteredModelNames, id: \.self) { name in
                        Text(name)
                            .lineLimit(1)
                            .truncationMode(.middle)
                            .tag(name)
                    }
                }
            }
            .listStyle(.sidebar)
            .overlay {
                if modelNames.isEmpty {
                    ContentUnavailableView("No models",
                                            systemImage: "cube",
                                            description: Text("This layout group has no models. Tap + on the canvas to add one."))
                } else if filteredModelNames.isEmpty {
                    ContentUnavailableView.search(text: modelFilter)
                }
            }
        case .groups:
            List(selection: groupListBinding) {
                Section {
                    ForEach(filteredGroupNames, id: \.self) { name in
                        Text(name)
                            .lineLimit(1)
                            .truncationMode(.middle)
                            .tag(name)
                            .swipeActions(edge: .trailing, allowsFullSwipe: false) {
                                Button(role: .destructive) {
                                    pendingDeleteGroupName = name
                                } label: {
                                    Label("Delete", systemImage: "trash")
                                }
                            }
                    }
                } header: {
                    HStack(spacing: 6) {
                        Text("Groups (\(filteredGroupNames.count)\(groupNames.count != filteredGroupNames.count ? " of \(groupNames.count)" : ""))")
                        Spacer()
                        Button {
                            newGroupSheetVisible = true
                        } label: {
                            Image(systemName: "plus.circle.fill")
                        }
                        .buttonStyle(.plain)
                        .accessibilityLabel("New group")
                    }
                }
            }
            .listStyle(.sidebar)
            .overlay {
                if groupNames.isEmpty {
                    ContentUnavailableView {
                        Label("No groups", systemImage: "square.stack.3d.up")
                    } description: {
                        Text("Model groups assigned to \(activeLayoutGroup) or All Previews will appear here.")
                    } actions: {
                        Button("Create a Group") {
                            newGroupSheetVisible = true
                        }
                        .buttonStyle(.borderedProminent)
                    }
                } else if filteredGroupNames.isEmpty {
                    ContentUnavailableView.search(text: groupFilter)
                }
            }
        case .objects:
            List(selection: objectListBinding) {
                Section {
                    ForEach(filteredObjectNames, id: \.self) { name in
                        // 2D Background is a synthetic pseudo-object;
                        // skip the delete swipe so the user can't try
                        // to remove it.
                        if name == "2D Background" {
                            Text(name)
                                .lineLimit(1)
                                .truncationMode(.middle)
                                .tag(name)
                        } else {
                            Text(name)
                                .lineLimit(1)
                                .truncationMode(.middle)
                                .tag(name)
                                .swipeActions(edge: .trailing, allowsFullSwipe: false) {
                                    Button(role: .destructive) {
                                        pendingDeleteObjectName = name
                                    } label: {
                                        Label("Delete", systemImage: "trash")
                                    }
                                }
                        }
                    }
                } header: {
                    HStack(spacing: 6) {
                        Text("Objects (\(filteredObjectNames.count)\(objectNames.count != filteredObjectNames.count ? " of \(objectNames.count)" : ""))")
                        Spacer()
                        Button {
                            addViewObjectSheetVisible = true
                        } label: {
                            Image(systemName: "plus.circle.fill")
                        }
                        .buttonStyle(.plain)
                        .accessibilityLabel("New view object")
                    }
                }
            }
            .listStyle(.sidebar)
            .overlay {
                // Note: 2D Background pseudo-object always returns
                // a non-empty array, so the empty-state path here
                // would only fire if the list were truly empty —
                // which shouldn't happen unless the show isn't
                // loaded.
                if filteredObjectNames.isEmpty && objectNames.isEmpty {
                    ContentUnavailableView {
                        Label("No view objects", systemImage: "scribble.variable")
                    } description: {
                        Text("Meshes, images, gridlines, terrain, and rulers in this preview will appear here.")
                    } actions: {
                        Button("Add a View Object") {
                            addViewObjectSheetVisible = true
                        }
                        .buttonStyle(.borderedProminent)
                    }
                } else if filteredObjectNames.isEmpty {
                    ContentUnavailableView.search(text: objectFilter)
                }
            }
        }
    }

    /// Bottom half: scrollable property editor bound to the
    /// current tab's selection. Empty-state hint when nothing is
    /// picked. Each tab's selection is independent — switching tabs
    /// shows the *other* tab's selection's properties.
    @ViewBuilder
    private var propertyPane: some View {
        VStack(alignment: .leading, spacing: 0) {
            propertyPaneHeader
            Divider()
            ScrollView {
                propertyPaneBody
                    .frame(maxWidth: .infinity, alignment: .leading)
                    .padding(.horizontal, 10)
                    .padding(.vertical, 8)
            }
        }
        .background(Color(uiColor: .secondarySystemBackground))
    }

    @ViewBuilder
    private var propertyPaneHeader: some View {
        HStack(spacing: 6) {
            Image(systemName: sidebarTab.systemImage)
                .foregroundStyle(.secondary)
                .font(.caption)
            Text(propertyPaneHeaderText)
                .font(.caption.weight(.semibold))
                .lineLimit(1)
                .truncationMode(.middle)
            Spacer()
        }
        .padding(.horizontal, 10)
        .padding(.vertical, 6)
    }

    private var propertyPaneHeaderText: String {
        switch sidebarTab {
        case .models:
            return viewModel.layoutEditorSelectedModel.map { "Model: \($0)" } ?? "No model selected"
        case .groups:
            return viewModel.layoutEditorSelectedGroup.map { "Group: \($0)" } ?? "No group selected"
        case .objects:
            return viewModel.layoutEditorSelectedObject.map { "Object: \($0)" } ?? "No object selected"
        }
    }

    @ViewBuilder
    private var propertyPaneBody: some View {
        switch sidebarTab {
        case .models:
            if let name = viewModel.layoutEditorSelectedModel,
               let summary = viewModel.document.modelLayoutSummary(name) {
                // ObjC bridges NSArray<NSDictionary*> as
                // `[[AnyHashable: Any]]` — coerce keys to String so
                // the descriptor view's `[[String: Any]]` matches.
                let rawDescriptors = viewModel.document.perTypeProperties(forModel: name)
                let descriptors: [[String: Any]] = rawDescriptors.compactMap { entry in
                    var out: [String: Any] = [:]
                    for (k, v) in entry {
                        if let key = k as? String { out[key] = v }
                    }
                    return out.isEmpty ? nil : out
                }
                LayoutEditorPropertiesView(
                    modelName: name,
                    summary: summary,
                    typeDescriptors: descriptors,
                    layoutGroups: layoutGroups,
                    token: summaryToken,
                    commit: { key, value in
                        commitProperty(modelName: name, key: key, value: value)
                    },
                    typeCommit: { key, value in
                        commitPerTypeProperty(modelName: name, key: key, value: value)
                    },
                    onRenameRequest: {
                        pendingRenameModelOldName = name
                        renameModelSheetVisible = true
                    },
                    onCommitAliases: { aliases in
                        commitAliases(modelName: name, aliases: aliases)
                    },
                    onCommitIndexedNames: { kind, names in
                        commitIndexedNames(modelName: name, kind: kind, names: names)
                    },
                    onDeleteSubModel: { submodelName in
                        deleteSubModel(modelName: name, submodelName: submodelName)
                    },
                    onAddSubModel: { submodelName in
                        addSubModel(modelName: name, submodelName: submodelName)
                    },
                    onRenameSubModel: { oldName, newName in
                        renameSubModel(modelName: name, oldName: oldName, newName: newName)
                    },
                    onLoadSubModelDetails: {
                        loadSubModelDetails(modelName: name)
                    },
                    onCommitSubModelDetails: { entries in
                        commitSubModelDetails(modelName: name, entries: entries)
                    },
                    onNavigateToGroup: { groupName in
                        sidebarTab = .groups
                        viewModel.layoutEditorSelectedGroup = groupName
                        viewModel.layoutEditorSelectedModel = nil
                    },
                    onClearDimmingCurve: {
                        clearDimmingCurve(modelName: name)
                    },
                    onPickImageFile: { key in
                        pendingImagePickKey = key
                        imageFilePickerVisible = true
                    },
                    onLoadFaceState: { kind in
                        // J-22 — bridge returns NSDictionary which
                        // Swift bridges to [String: AnyHashable].
                        // Cast through NSDictionary to keep the
                        // nested-dict shape intact regardless of
                        // type-erasure quirks.
                        let raw: NSDictionary
                        switch kind {
                        case .faces:
                            raw = viewModel.document.faceInfo(forModel: name) as NSDictionary
                        case .states:
                            raw = viewModel.document.stateInfo(forModel: name) as NSDictionary
                        default:
                            return [:]
                        }
                        var out: [String: [String: String]] = [:]
                        for (k, v) in raw {
                            guard let key = k as? String,
                                  let inner = v as? NSDictionary else { continue }
                            var attrs: [String: String] = [:]
                            for (ak, av) in inner {
                                if let kk = ak as? String, let vv = av as? String {
                                    attrs[kk] = vv
                                }
                            }
                            out[key] = attrs
                        }
                        return out
                    },
                    onCommitFaceState: { kind, entries in
                        commitFaceState(modelName: name, kind: kind, entries: entries)
                    },
                    onLoadDimming: {
                        let raw = viewModel.document.dimmingInfo(forModel: name) as NSDictionary
                        var out: [String: [String: String]] = [:]
                        for (k, v) in raw {
                            guard let key = k as? String,
                                  let inner = v as? NSDictionary else { continue }
                            var attrs: [String: String] = [:]
                            for (ak, av) in inner {
                                if let kk = ak as? String, let vv = av as? String {
                                    attrs[kk] = vv
                                }
                            }
                            out[key] = attrs
                        }
                        return out
                    },
                    onCommitDimming: { entries in
                        commitDimming(modelName: name, entries: entries)
                    },
                    onOpenCustomModelEditor: {
                        openCustomModelEditor(modelName: name)
                    }
                )
            } else {
                emptyPropertyHint(
                    title: "Pick a model",
                    body: "Tap a model in the list above to edit its position, size, rotation, layout group, and controller mapping."
                )
            }
        case .groups:
            if let name = viewModel.layoutEditorSelectedGroup,
               let summary = viewModel.document.modelGroupLayoutSummary(name) {
                LayoutEditorGroupPropertiesView(
                    groupName: name,
                    summary: summary,
                    layoutGroups: layoutGroups,
                    token: summaryToken,
                    commit: { key, value in
                        commitGroupProperty(groupName: name, key: key, value: value)
                    },
                    onRemoveMember: { memberName in
                        if viewModel.document.removeModel(memberName, fromGroup: name) {
                            summaryToken &+= 1
                            hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
                        }
                    },
                    onAddMember: { addMemberSheetVisible = true },
                    onReorderMembers: { newOrder in
                        commitGroupProperty(groupName: name,
                                            key: "members",
                                            value: newOrder as NSArray)
                    },
                    onRenameRequest: {
                        pendingRenameGroupOldName = name
                        renameGroupSheetVisible = true
                    }
                )
            } else {
                emptyPropertyHint(
                    title: "Pick a group",
                    body: "Model groups bundle related models together. Tap a group above to inspect its members and edit its layout settings."
                )
            }
        case .objects:
            if let name = viewModel.layoutEditorSelectedObject,
               let summary = viewModel.document.viewObjectLayoutSummary(name) {
                if (summary["isBackground"] as? Bool) ?? false {
                    LayoutEditorBackgroundPropertiesView(
                        summary: summary,
                        token: summaryToken,
                        commit: { key, value in
                            commitObjectProperty(objectName: name, key: key, value: value)
                        },
                        onPickImage: { backgroundImagePickerVisible = true }
                    )
                } else {
                    LayoutEditorObjectPropertiesView(
                        objectName: name,
                        summary: summary,
                        layoutGroups: layoutGroups,
                        token: summaryToken,
                        commit: { key, value in
                            commitObjectProperty(objectName: name, key: key, value: value)
                        },
                        onPickFile: { key, types in
                            pendingObjectFilePickKey = key
                            objectFilePickerTypes = types
                            objectFilePickerVisible = true
                        },
                        onRenameRequest: {
                            pendingRenameObjectOldName = name
                            renameObjectSheetVisible = true
                        },
                        onDuplicate: {
                            handleDuplicateViewObject(name)
                        }
                    )
                }
            } else {
                emptyPropertyHint(
                    title: "Pick a view object",
                    body: "View objects (meshes, images, gridlines, terrain, rulers) are decorative elements that don't take channels. Tap one to inspect its position and size."
                )
            }
        }
    }

    @ViewBuilder
    private func emptyPropertyHint(title: String, body: String) -> some View {
        VStack(alignment: .leading, spacing: 6) {
            Text(title)
                .font(.caption.weight(.semibold))
                .foregroundStyle(.secondary)
            Text(body)
                .font(.caption)
                .foregroundStyle(.secondary)
                .fixedSize(horizontal: false, vertical: true)
        }
    }

    /// Display info — moved out of the roster list into the property
    /// pane's footer so it's always visible without scrolling.
    @ViewBuilder
    private var displaySection: some View {
        // J-8 — Background row moved to the Objects tab's
        // synthetic "2D Background" entry; rest of the display
        // properties (canvas size, default mode, grid, bbox) stay
        // here as a read-only roll-up.
        VStack(alignment: .leading, spacing: 4) {
            Text("Display")
                .font(.caption.weight(.semibold))
                .foregroundStyle(.secondary)
            displayRow("Canvas",
                       value: "\(displayState["previewWidth"] as? Int ?? 0) × \(displayState["previewHeight"] as? Int ?? 0)")
            displayRow("2D centre = 0",
                       value: (displayState["display2DCenter0"] as? Bool ?? false) ? "Yes" : "No")
            displayRow("Default mode",
                       value: (displayState["layoutMode3D"] as? Bool ?? true) ? "3D" : "2D")
            displayRow("Grid", value: gridLabel)
            displayRow("Bounding box",
                       value: (displayState["display2DBoundingBox"] as? Bool ?? false) ? "On" : "Off")
        }
        .font(.caption.monospacedDigit())
    }

    @ViewBuilder
    private func displayRow(_ label: String, value: String) -> some View {
        HStack(alignment: .firstTextBaseline) {
            Text(label).foregroundStyle(.secondary)
            Spacer(minLength: 8)
            Text(value).lineLimit(1).truncationMode(.middle)
        }
    }

    /// Draggable divider between the roster pane and the property
    /// pane. Vertical drag adjusts `sidebarTopFraction`. Clamps so
    /// neither pane shrinks below a usable threshold.
    @ViewBuilder
    private func sidebarDivider(totalHeight: CGFloat) -> some View {
        ZStack {
            Rectangle()
                .fill(Color(uiColor: .separator))
                .frame(height: 0.5)
            // Wider hit-target so the drag feels touchable even with
            // a hair-thin line. The capsule grip is purely cosmetic.
            Capsule()
                .fill(.secondary.opacity(0.4))
                .frame(width: 36, height: 4)
        }
        .frame(height: 16)
        .frame(maxWidth: .infinity)
        .contentShape(Rectangle())
        .gesture(
            DragGesture(minimumDistance: 0)
                .onChanged { value in
                    let proposed = sidebarTopFraction + value.translation.height / totalHeight
                    sidebarTopFraction = max(0.2, min(0.8, proposed))
                }
        )
    }

    // MARK: - Roster filtering

    private var filteredModelNames: [String] {
        let q = modelFilter.trimmingCharacters(in: .whitespaces)
        guard !q.isEmpty else { return modelNames }
        return modelNames.filter { $0.localizedCaseInsensitiveContains(q) }
    }

    private var filteredGroupNames: [String] {
        let q = groupFilter.trimmingCharacters(in: .whitespaces)
        guard !q.isEmpty else { return groupNames }
        return groupNames.filter { $0.localizedCaseInsensitiveContains(q) }
    }

    private var filteredObjectNames: [String] {
        let q = objectFilter.trimmingCharacters(in: .whitespaces)
        guard !q.isEmpty else { return objectNames }
        return objectNames.filter { $0.localizedCaseInsensitiveContains(q) }
    }

    private var currentFilterBinding: Binding<String> {
        switch sidebarTab {
        case .models:  return $modelFilter
        case .groups:  return $groupFilter
        case .objects: return $objectFilter
        }
    }

    /// Models list uses the same SequencerViewModel selection slot
    /// the canvas reads from — so picking a model in the sidebar
    /// drives the canvas handles, action bar, and keyboard nudge.
    private var modelListBinding: Binding<String?> {
        Binding(
            get: { viewModel.layoutEditorSelectedModel },
            set: { viewModel.layoutSelectSingle($0) }
        )
    }

    /// J-6 — Group list binding writes to the view model so
    /// PreviewPaneView can sync the cyan-member tint to the canvas.
    private var groupListBinding: Binding<String?> {
        Binding(
            get: { viewModel.layoutEditorSelectedGroup },
            set: { viewModel.layoutEditorSelectedGroup = $0 }
        )
    }
    private var objectListBinding: Binding<String?> {
        Binding(
            get: { viewModel.layoutEditorSelectedObject },
            set: { viewModel.layoutEditorSelectedObject = $0 }
        )
    }

    private var gridLabel: String {
        let on = (displayState["display2DGrid"] as? Bool) ?? false
        let spacing = (displayState["display2DGridSpacing"] as? Int)
            ?? Int((displayState["display2DGridSpacing"] as? NSNumber)?.intValue ?? 100)
        return on ? "On (\(spacing))" : "Off"
    }

    // MARK: - Canvas

    @ViewBuilder
    private var canvas: some View {
        ZStack(alignment: .topTrailing) {
            // Reuse the PreviewPaneView used by House Preview. New
            // previewName so its NotificationCenter routing
            // (zoom/reset/fit) is independent of the embedded house
            // pane that may also be visible.
            //
            // previewModelName is nil so the canvas draws every
            // model in the active layout group, not single-model
            // mode. selectedModelName drives Fit Selected.
            PreviewPaneView(previewName: "LayoutEditor",
                            previewModelName: nil,
                            controlsVisible: $controlsVisible,
                            settings: settings,
                            status: PreviewStatus())
                .background(Color.black)

            // J-4 — top-left overlay: document-state actions.
            // NavigationSplitView's primary-action toolbar items
            // never paint in this scene (the column chrome is
            // hidden), so the layout-group switcher / Undo / Save
            // would otherwise be unreachable. Same story for the
            // top-right "+", which has always been canvas-overlay.
            VStack {
                HStack(spacing: 6) {
                    if layoutGroups.count > 1 {
                        Menu {
                            ForEach(layoutGroups, id: \.self) { name in
                                Button {
                                    activeLayoutGroup = name
                                } label: {
                                    HStack {
                                        Text(name)
                                        if name == activeLayoutGroup {
                                            Spacer()
                                            Image(systemName: "checkmark")
                                        }
                                    }
                                }
                            }
                        } label: {
                            HStack(spacing: 4) {
                                Image(systemName: "square.stack.3d.up")
                                Text(activeLayoutGroup)
                                    .lineLimit(1)
                                    .frame(maxWidth: 140)
                                Image(systemName: "chevron.down")
                                    .font(.caption2)
                            }
                            .font(.caption)
                        }
                        .menuStyle(.borderlessButton)
                        .padding(.horizontal, 8)
                        .padding(.vertical, 4)
                        .background(.regularMaterial, in: Capsule())
                    }

                    Button {
                        performUndo()
                    } label: {
                        Image(systemName: "arrow.uturn.backward")
                    }
                    .buttonStyle(.bordered)
                    .controlSize(.small)
                    .disabled(!canUndo)

                    Button {
                        showingSaveConfirm = true
                    } label: {
                        ZStack(alignment: .topTrailing) {
                            Image(systemName: "square.and.arrow.down")
                            if hasUnsavedChanges {
                                // Unsaved-changes dot.
                                Circle()
                                    .fill(.orange)
                                    .frame(width: 8, height: 8)
                                    .offset(x: 3, y: -3)
                            }
                        }
                    }
                    .buttonStyle(.bordered)
                    .controlSize(.small)
                    .disabled(!hasUnsavedChanges)

                    Spacer()
                }
                .padding(8)
                Spacer()
            }

            VStack(alignment: .trailing, spacing: 4) {
                // J-3 (touch UX) — Add Model.
                Button {
                    addModelSheetVisible = true
                } label: {
                    Image(systemName: "plus.circle.fill")
                        .font(.title3)
                }
                .buttonStyle(.bordered)
                .controlSize(.small)
                .disabled(!viewModel.isShowFolderLoaded || availableModelTypes.isEmpty)

                // J-4 (import) — Import .xmodel. Opens the iPadOS
                // file picker (iCloud Drive, Files, AirDrop receive
                // folder, etc.). Picked file flips the editor into
                // placement mode; the next canvas tap drops the
                // imported model at the touch point.
                Button {
                    importerVisible = true
                } label: {
                    Image(systemName: "square.and.arrow.down.on.square")
                        .font(.title3)
                }
                .buttonStyle(.bordered)
                .controlSize(.small)
                .disabled(!viewModel.isShowFolderLoaded)

                // J-4 (download) — Download from vendor catalog.
                // Sheet fetches xlights.org's vendor list +
                // inventories through the shared core catalog
                // code, then lets the user pick a wiring. On
                // download, the local xmodel path flips us into
                // the same placement flow as Import.
                Button {
                    downloadBrowserVisible = true
                } label: {
                    Image(systemName: "icloud.and.arrow.down")
                        .font(.title3)
                }
                .buttonStyle(.bordered)
                .controlSize(.small)
                .disabled(!viewModel.isShowFolderLoaded)

                // J-4 (multi-select) — Photos-style Select toggle.
                Button {
                    let entering = !viewModel.layoutEditMode
                    viewModel.layoutEditMode = entering
                    if !entering, let primary = viewModel.layoutEditorSelectedModel {
                        viewModel.layoutEditorSelection = [primary]
                    } else if entering, viewModel.layoutEditorSelection.isEmpty,
                              let seed = viewModel.layoutEditorSelectedModel {
                        viewModel.layoutEditorSelection = [seed]
                    }
                } label: {
                    Image(systemName: viewModel.layoutEditMode
                                       ? "checkmark.circle.fill"
                                       : "checkmark.circle")
                        .font(.title3)
                }
                .buttonStyle(.bordered)
                .controlSize(.small)
                .tint(viewModel.layoutEditMode ? .accentColor : .secondary)
                .disabled(!viewModel.isShowFolderLoaded)

                Button {
                    controlsVisible.toggle()
                } label: {
                    Image(systemName: controlsVisible
                          ? "slider.horizontal.3"
                          : "slider.horizontal.below.rectangle")
                }
                .buttonStyle(.bordered)
                .controlSize(.small)

                if controlsVisible {
                    LayoutEditorCanvasControls(previewName: "LayoutEditor",
                                               settings: settings,
                                               selectedModelName:
                                                viewModel.layoutEditorSelectedModel)
                }
            }
            .frame(maxWidth: .infinity, alignment: .trailing)
            .padding(8)

            // J-3 (touch UX) — creation-mode banner. Visible while
            // `layoutPendingNewModelType` is set (first-vertex tap)
            // OR `layoutPolylineInProgress` is set (mid-polyline
            // appending). The polyline branch swaps Cancel for Done
            // so the user can stop adding vertices.
            if let pendingType = viewModel.layoutPendingNewModelType {
                VStack {
                    HStack(spacing: 12) {
                        Image(systemName: "plus.circle.fill")
                            .foregroundStyle(.green)
                        Text("Tap canvas to place \(Text(modelTypeLabel(pendingType)).fontWeight(.semibold))")
                            .foregroundStyle(.white)
                        Button("Cancel") {
                            viewModel.layoutPendingNewModelType = nil
                        }
                        .buttonStyle(.borderedProminent)
                        .controlSize(.small)
                    }
                    .padding(.horizontal, 14)
                    .padding(.vertical, 8)
                    .background(.regularMaterial, in: Capsule())
                    .shadow(radius: 3, y: 2)
                    .padding(.top, 12)
                    Spacer()
                }
                .frame(maxWidth: .infinity)
                .allowsHitTesting(true)
            } else if let polyName = viewModel.layoutPolylineInProgress {
                VStack {
                    HStack(spacing: 12) {
                        Image(systemName: "scribble.variable")
                            .foregroundStyle(.green)
                        Text("Tap to add vertex to \(Text(polyName).fontWeight(.semibold))")
                            .foregroundStyle(.white)
                        Button("Done") {
                            viewModel.layoutPolylineInProgress = nil
                        }
                        .buttonStyle(.borderedProminent)
                        .controlSize(.small)
                    }
                    .padding(.horizontal, 14)
                    .padding(.vertical, 8)
                    .background(.regularMaterial, in: Capsule())
                    .shadow(radius: 3, y: 2)
                    .padding(.top, 12)
                    Spacer()
                }
                .frame(maxWidth: .infinity)
                .allowsHitTesting(true)
            } else if let importPath = viewModel.layoutPendingImportPath {
                // J-4 (import) — pending-import banner. The file
                // was picked; the next canvas tap drops the model
                // at the touch point.
                let fileName = (importPath as NSString).lastPathComponent
                VStack {
                    HStack(spacing: 12) {
                        Image(systemName: "square.and.arrow.down.on.square.fill")
                            .foregroundStyle(.green)
                        Text("Tap canvas to place \(Text(fileName).fontWeight(.semibold))")
                            .foregroundStyle(.white)
                        Button("Cancel") {
                            viewModel.layoutPendingImportPath = nil
                        }
                        .buttonStyle(.borderedProminent)
                        .controlSize(.small)
                    }
                    .padding(.horizontal, 14)
                    .padding(.vertical, 8)
                    .background(.regularMaterial, in: Capsule())
                    .shadow(radius: 3, y: 2)
                    .padding(.top, 12)
                    Spacer()
                }
                .frame(maxWidth: .infinity)
                .allowsHitTesting(true)
            }

            // J-2 UX — model-name labels overlay. Off by default;
            // user enables via the canvas controls. Renders one
            // small Text per visible model at its projected centre;
            // refreshes each animation frame.
            if settings.showModelLabels {
                ModelLabelsOverlay()
                    .allowsHitTesting(false)
            }

            // Inline action bar floating above the selected model
            // — see plans/phase-j-touch-ux.md. Anchored to the
            // model's top-centre in screen coords; re-queries
            // every animation frame so it tracks pan / zoom /
            // orbit / drag without observer wiring. Hidden in
            // multi-select mode (the multi-select bar takes over
            // since the inline bar's per-model actions don't make
            // sense across a set).
            if viewModel.layoutEditorSelection.count < 2,
               let selected = viewModel.layoutEditorSelectedModel,
               !selected.isEmpty {
                InlineModelActionBar(modelName: selected,
                                      summaryToken: summaryToken,
                                      onPropertyChange: {
                                          summaryToken &+= 1
                                          hasUnsavedChanges =
                                              viewModel.document.hasUnsavedLayoutChanges()
                                      },
                                      onRequestDelete: {
                                          pendingDeleteModelName = selected
                                      },
                                      onRequestDuplicate: {
                                          performDuplicate(of: [selected])
                                      })
                    .allowsHitTesting(true)
            }

            // J-4 (multi-select) — operations bar. Visible
            // whenever 2+ models are selected. Hosts Align ▾,
            // Distribute ▾, Match Size ▾, and a Clear action.
            // Top-centered like the creation banner so it doesn't
            // fight the bottom tool toolbar.
            if viewModel.layoutEditorSelection.count >= 2 {
                VStack {
                    MultiSelectActionBar(
                        selection: viewModel.layoutEditorSelection,
                        leader: viewModel.layoutEditorSelectedModel,
                        onAlign: { edge in performAlign(by: edge) },
                        onDistribute: { axis in performDistribute(axis: axis) },
                        onMatchSize: { dim in performMatchSize(dimension: dim) },
                        onFlip: { axis in performFlip(axis: axis) },
                        onDuplicate: { performDuplicate(of: Array(viewModel.layoutEditorSelection)) },
                        onGroup: { newGroupFromSelectionPrompt() },
                        onClear: {
                            viewModel.layoutEditorSelection.removeAll()
                            viewModel.layoutEditorSelectedModel = nil
                        })
                        .padding(.top, 12)
                    Spacer()
                }
                .frame(maxWidth: .infinity)
                .allowsHitTesting(true)
            }

            // Bottom tool toolbar — see plans/phase-j-touch-ux.md.
            // Replaces desktop's CentreCycle (tap centre sphere to
            // advance axis tool) with a persistent picker. Visible
            // only when a model is selected.
            if let selected = viewModel.layoutEditorSelectedModel,
               !selected.isEmpty {
                VStack {
                    Spacer()
                    LayoutEditorToolToolbar(settings: settings,
                                             selectedModelName: selected,
                                             onToolChange: { tool in
                                                 syncToolToBridge(tool: tool, modelName: selected)
                                             })
                    .padding(.bottom, 12)
                }
            }
        }
    }

    /// Phase J-2 (touch UX) — buttons shown in the long-press
    /// `.confirmationDialog`. The menu's content depends on what
    /// the user long-pressed on (vertex / segment / curve control).
    @ViewBuilder
    private var contextMenuButtons: some View {
        switch contextMenuTarget {
        case .vertex(let name, let idx):
            Button("Delete Point", role: .destructive) {
                _ = viewModel.document.deleteVertex(at: idx, forModel: name)
                postLayoutMutation(modelName: name)
            }
            Button("Cancel", role: .cancel) { }
        case .segment(let name, let idx, let hasCurve):
            Button("Add Point") {
                _ = viewModel.document.insertVertex(inSegment: idx, forModel: name)
                postLayoutMutation(modelName: name)
            }
            if hasCurve {
                Button("Remove Curve", role: .destructive) {
                    _ = viewModel.document.setCurve(false, onSegment: idx, forModel: name)
                    postLayoutMutation(modelName: name)
                }
            } else {
                Button("Define Curve") {
                    _ = viewModel.document.setCurve(true, onSegment: idx, forModel: name)
                    postLayoutMutation(modelName: name)
                }
            }
            Button("Cancel", role: .cancel) { }
        case .curveControl(let name, let idx):
            Button("Remove Curve", role: .destructive) {
                _ = viewModel.document.setCurve(false, onSegment: idx, forModel: name)
                postLayoutMutation(modelName: name)
            }
            Button("Cancel", role: .cancel) { }
        case nil:
            EmptyView()
        }
    }

    private func postLayoutMutation(modelName: String) {
        NotificationCenter.default.post(name: .layoutEditorModelMoved,
                                         object: "LayoutEditor",
                                         userInfo: ["model": modelName])
    }

    /// Push toolbar tool selection through to the document. The
    /// settings update happens in the Toolbar's binding; this
    /// follow-up call writes to the screen location's `axis_tool`
    /// so `GetHandles` emits the matching descriptor set on the
    /// next draw. Repaint is triggered by the
    /// `.layoutEditorModelMoved` notification (covers refresh of
    /// the canvas overlay so the new gizmo appears immediately).
    private func syncToolToBridge(tool: String, modelName: String) {
        _ = viewModel.document.setAxisTool(tool, forModel: modelName)
        NotificationCenter.default.post(name: .layoutEditorModelMoved,
                                         object: "LayoutEditor",
                                         userInfo: ["model": modelName])
    }

    // MARK: - Refresh

    private func refresh() {
        layoutGroups = viewModel.document.layoutGroups()
        activeLayoutGroup = viewModel.document.activeLayoutGroup()
        if viewModel.isShowFolderLoaded {
            settings.is3D = viewModel.document.layoutMode3D()
        }
        displayState = viewModel.document.layoutDisplayState()
        availableModelTypes = viewModel.document.availableModelTypesForCreation()
        // Seed overlay toggles from the show's saved rgbeffects.xml
        // values on first refresh after the editor opens. Subsequent
        // refreshes don't clobber whatever the user has toggled
        // since.
        if !overlaysInitialized {
            settings.showLayoutGrid =
                (displayState["display2DGrid"] as? Bool) ?? false
            settings.showLayoutBoundingBox =
                (displayState["display2DBoundingBox"] as? Bool) ?? false
            overlaysInitialized = true
        }
        refreshModelList()
    }

    /// Friendlier labels for the Add-Model picker. Falls back to
    /// the raw type string when no mapping is defined — keeps the
    /// menu functional even as the curated set grows.
    private func modelTypeLabel(_ type: String) -> String {
        switch type {
        case "Poly Line":   return "Poly Line"
        case "Single Line": return "Single Line"
        case "Window Frame":return "Window Frame"
        case "Candy Canes": return "Candy Canes"
        case "Channel Block": return "Channel Block"
        default:            return type
        }
    }

    private func refreshModelList() {
        modelNames = viewModel.document.modelsInActiveLayoutGroup()
        groupNames = viewModel.document.modelGroupsInActiveLayoutGroup()
        objectNames = viewModel.document.viewObjectsInActiveLayoutGroup()
        // If the previously-selected model isn't in the new list,
        // clear selection so the side panel doesn't show stale data.
        if let sel = viewModel.layoutEditorSelectedModel,
           !modelNames.contains(sel) {
            viewModel.layoutSelectSingle(nil)
        }
        if let g = viewModel.layoutEditorSelectedGroup, !groupNames.contains(g) {
            viewModel.layoutEditorSelectedGroup = nil
        }
        if let o = viewModel.layoutEditorSelectedObject, !objectNames.contains(o) {
            viewModel.layoutEditorSelectedObject = nil
        }
    }

    /// J-6 / J-17 (objects) — commit a view-object property edit.
    /// Pushes an undo snapshot before the edit (skipping the
    /// 2D Background pseudo-object — its undo would have to walk
    /// per-group background settings). Bumps the summary token +
    /// repaints the canvas like the model path.
    private func commitObjectProperty(objectName: String, key: String, value: Any) {
        if objectName != "2D Background" {
            viewModel.document.pushLayoutUndoSnapshot(forViewObject: objectName)
        }
        let changed = viewModel.document.setLayoutViewObjectProperty(objectName,
                                                                     key: key,
                                                                     value: value)
        if changed {
            summaryToken &+= 1
            hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
            canUndo = viewModel.document.canUndoLayoutChange()
            if key == "layoutGroup" {
                refreshModelList()
            }
            NotificationCenter.default.post(name: .layoutEditorModelMoved,
                                            object: "LayoutEditor",
                                            userInfo: ["model": objectName])
        }
    }

    /// J-7 (group CRUD) — current group's member set, used by the
    /// add-member sheet to filter the candidate list. Returns an
    /// empty set when no group is selected.
    private var selectedGroupMembers: Set<String> {
        guard let name = viewModel.layoutEditorSelectedGroup,
              let summary = viewModel.document.modelGroupLayoutSummary(name) else {
            return []
        }
        return Set(summary["models"] as? [String] ?? [])
    }

    /// J-7 — new-group sheet "Create" callback. Bridge does the
    /// validation (name collision); on success we select the new
    /// group so the user can immediately add members.
    /// `pendingGroupFromSelection`, when non-nil, supplies the
    /// initial member list (multi-select "Group" action populates
    /// it before opening the sheet).
    private func handleCreateGroup(_ name: String) {
        let members = pendingGroupFromSelection
        pendingGroupFromSelection = nil
        if viewModel.document.createModelGroup(name, members: members) {
            viewModel.layoutEditorSelectedGroup = name
            hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
            refreshModelList()
            // If the user grouped from a model selection, flip to
            // the Groups tab so the new group + its members appear.
            if members != nil {
                sidebarTab = .groups
                viewModel.layoutEditorSelection.removeAll()
                viewModel.layoutEditorSelectedModel = nil
            }
        }
        newGroupSheetVisible = false
    }

    /// J-7 — add-member sheet "Add" callback. Each member fires
    /// its own bridge call (each one marks the group dirty
    /// independently, but the UX presents it as a single batch).
    private func handleAddMembers(_ picked: [String]) {
        guard let groupName = viewModel.layoutEditorSelectedGroup else {
            addMemberSheetVisible = false
            return
        }
        for name in picked {
            _ = viewModel.document.addModel(name, toGroup: groupName)
        }
        if !picked.isEmpty {
            summaryToken &+= 1
            hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
        }
        addMemberSheetVisible = false
    }

    /// J-7 — delete-group confirmation "Delete" callback.
    private func handleDeleteGroup(_ name: String) {
        if viewModel.document.deleteModelGroup(name) {
            if viewModel.layoutEditorSelectedGroup == name {
                viewModel.layoutEditorSelectedGroup = nil
            }
            hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
            refreshModelList()
        }
    }

    /// J-16 — rename-group sheet "Rename" callback. On success,
    /// re-points the sidebar selection to the new name (so the
    /// property pane reopens on the renamed group) and refreshes
    /// the roster.
    private func handleRenameGroup(_ oldName: String, _ newName: String) {
        guard viewModel.document.renameModelGroup(oldName, to: newName) else { return }
        if viewModel.layoutEditorSelectedGroup == oldName {
            viewModel.layoutEditorSelectedGroup = newName
        }
        summaryToken &+= 1
        hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
        refreshModelList()
    }

    /// J-5 (groups) — commit a group property edit. Mirrors
    /// `commitProperty` for models: pushes an undo snapshot first
    /// (so the user can revert), invokes the bridge setter, and
    /// updates the dirty / undo flags on success. Group transforms
    /// flow through the same `_dirtyLayoutModels` set; save handler
    /// rewrites the `<modelGroup>` element in place.
    private func commitGroupProperty(groupName: String, key: String, value: Any) {
        viewModel.document.pushLayoutUndoSnapshot(forModel: groupName)
        let changed = viewModel.document.setLayoutModelGroupProperty(groupName,
                                                                     key: key,
                                                                     value: value)
        if changed {
            summaryToken &+= 1
            hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
            canUndo = viewModel.document.canUndoLayoutChange()
            if key == "layoutGroup" {
                refreshModelList()
            }
        }
    }

    /// J-6 (per-type properties) — commit a Tree/Arch/Matrix/etc.
    /// type-specific edit. Pushes undo, invokes the bridge, then
    /// bumps the summary token so both panes (common + per-type)
    /// re-read from the bridge. Mirrors `commitProperty` for common
    /// props; structural changes (string count etc.) refresh the
    /// model list too because channel ranges can shift.
    private func commitPerTypeProperty(modelName: String, key: String, value: Any) {
        viewModel.document.pushLayoutUndoSnapshot(forModel: modelName)
        let changed = viewModel.document.setPerTypeProperty(key,
                                                            onModel: modelName,
                                                            value: value)
        if changed {
            summaryToken &+= 1
            hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
            canUndo = viewModel.document.canUndoLayoutChange()
            refreshModelList()
            NotificationCenter.default.post(name: .layoutEditorModelMoved,
                                            object: "LayoutEditor",
                                            userInfo: ["model": modelName])
        }
    }

    /// J-18 pass 2 — wholesale-replace this model's alias list.
    /// Separate path from `commitProperty` because aliases are a
    /// collection (not a scalar prop) and round-trip through the
    /// dedicated `setModelAliases:aliases:` bridge method.
    private func commitAliases(modelName: String, aliases: [String]) {
        viewModel.document.pushLayoutUndoSnapshot(forModel: modelName)
        guard viewModel.document.setModelAliases(modelName, aliases: aliases) else { return }
        summaryToken &+= 1
        hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
        canUndo = viewModel.document.canUndoLayoutChange()
    }

    /// J-22 — add a new submodel on `modelName`. Returns the
    /// sanitized name on success.
    private func addSubModel(modelName: String, submodelName: String) -> String? {
        viewModel.document.pushLayoutUndoSnapshot(forModel: modelName)
        guard let added = viewModel.document.addSubModel(toModel: modelName,
                                                          name: submodelName) else {
            return nil
        }
        summaryToken &+= 1
        hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
        canUndo = viewModel.document.canUndoLayoutChange()
        return added
    }

    /// J-23.3 — Load full per-submodel details from the bridge.
    private func loadSubModelDetails(modelName: String) -> [SubModelEntry] {
        let raw = viewModel.document.submodelDetails(forModel: modelName) as NSArray
        var out: [SubModelEntry] = []
        for case let d as NSDictionary in raw {
            let name        = (d["name"] as? String) ?? ""
            let isRanges    = (d["isRanges"] as? Bool) ?? true
            let isVertical  = (d["isVertical"] as? Bool) ?? false
            let bufferStyle = (d["bufferStyle"] as? String) ?? "Default"
            let strands     = (d["strands"] as? [String]) ?? []
            let subBuffer   = (d["subBuffer"] as? String) ?? ""
            out.append(SubModelEntry(
                name: name,
                isRanges: isRanges,
                isVertical: isVertical,
                bufferStyle: bufferStyle,
                strands: strands,
                subBuffer: subBuffer))
        }
        return out
    }

    /// J-23.3 — Wholesale-replace all submodels (matches
    /// desktop's RemoveAllSubModels + re-add pattern).
    private func commitSubModelDetails(modelName: String,
                                        entries: [SubModelEntry]) -> Bool {
        viewModel.document.pushLayoutUndoSnapshot(forModel: modelName)
        let arr: NSMutableArray = NSMutableArray()
        for e in entries {
            let d: NSMutableDictionary = NSMutableDictionary()
            d["name"]        = e.name
            d["isRanges"]    = NSNumber(value: e.isRanges)
            d["isVertical"]  = NSNumber(value: e.isVertical)
            d["bufferStyle"] = e.bufferStyle
            d["strands"]     = e.strands
            d["subBuffer"]   = e.subBuffer
            arr.add(d)
        }
        let ok = viewModel.document.replaceSubModels(
            onModel: modelName,
            withEntries: arr as? [[String : Any]] ?? [])
        guard ok else { return false }
        summaryToken &+= 1
        hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
        canUndo = viewModel.document.canUndoLayoutChange()
        return true
    }

    /// J-22 — rename a submodel on its parent.
    private func renameSubModel(modelName: String,
                                 oldName: String,
                                 newName: String) -> Bool {
        viewModel.document.pushLayoutUndoSnapshot(forModel: modelName)
        guard viewModel.document.renameSubModelNamed(oldName,
                                                      onModel: modelName,
                                                      to: newName) else {
            return false
        }
        summaryToken &+= 1
        hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
        canUndo = viewModel.document.canUndoLayoutChange()
        return true
    }

    /// J-23 — open the Custom-model visual grid editor. Reads
    /// the current 3D grid + dims + bg-image fields from the
    /// bridge into a payload, then surfaces the sheet.
    private func openCustomModelEditor(modelName: String) {
        let raw = viewModel.document.customModelData(forModel: modelName) as NSDictionary
        let width  = (raw["width"]  as? NSNumber)?.intValue ?? 1
        let height = (raw["height"] as? NSNumber)?.intValue ?? 1
        let depth  = (raw["depth"]  as? NSNumber)?.intValue ?? 1
        var locs: [[[Int]]] = []
        if let arr = raw["locations"] as? NSArray {
            for layer in arr {
                guard let layerArr = layer as? NSArray else { continue }
                var layerOut: [[Int]] = []
                for row in layerArr {
                    guard let rowArr = row as? NSArray else { continue }
                    var rowOut: [Int] = []
                    for cell in rowArr {
                        rowOut.append((cell as? NSNumber)?.intValue ?? 0)
                    }
                    layerOut.append(rowOut)
                }
                locs.append(layerOut)
            }
        }
        let bgPath  = raw["backgroundImage"] as? String ?? ""
        let bgScale = (raw["backgroundScale"] as? NSNumber)?.intValue ?? 100
        let bgBri   = (raw["backgroundBrightness"] as? NSNumber)?.intValue ?? 100
        customModelEditorPayload = CustomModelPayload(
            modelName: modelName,
            initialWidth: width,
            initialHeight: height,
            initialDepth: depth,
            initialLocations: locs,
            initialBackground: bgPath,
            initialBkgScale: bgScale,
            initialBkgBrightness: bgBri)
    }

    /// J-23 — wholesale-replace the custom-model grid + dims.
    private func commitCustomModelGrid(modelName: String,
                                         w: Int, h: Int, d: Int,
                                         locations: [[[Int]]]) {
        viewModel.document.pushLayoutUndoSnapshot(forModel: modelName)
        // SwiftUI [[[Int]]] doesn't bridge to NSArray<NSArray<
        // NSArray<NSNumber>>>* automatically — build it.
        let nsLocs: NSMutableArray = NSMutableArray()
        for layer in locations {
            let layerArr: NSMutableArray = NSMutableArray()
            for row in layer {
                let rowArr: NSMutableArray = NSMutableArray()
                for v in row {
                    rowArr.add(NSNumber(value: v))
                }
                layerArr.add(rowArr)
            }
            nsLocs.add(layerArr)
        }
        let ok = viewModel.document.setCustomModelData(
            modelName,
            width: Int32(w), height: Int32(h), depth: Int32(d),
            locations: nsLocs as! [[[NSNumber]]])
        guard ok else { return }
        summaryToken &+= 1
        hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
        canUndo = viewModel.document.canUndoLayoutChange()
    }

    /// J-22 — wholesale-replace a model's face / state map.
    /// The Swift dictionary is bridged to an NSDictionary that
    /// the ObjC++ bridge converts into the C++ `FaceStateData`.
    private func commitFaceState(modelName: String,
                                  kind: ModelDataKind,
                                  entries: [String: [String: String]]) {
        viewModel.document.pushLayoutUndoSnapshot(forModel: modelName)
        let ok: Bool
        switch kind {
        case .faces:
            ok = viewModel.document.setFaceInfo(modelName, entries: entries)
        case .states:
            ok = viewModel.document.setStateInfo(modelName, entries: entries)
        default:
            return
        }
        guard ok else { return }
        summaryToken &+= 1
        hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
        canUndo = viewModel.document.canUndoLayoutChange()
    }

    /// J-22 — wholesale-replace the dimming-curve map.
    private func commitDimming(modelName: String,
                                entries: [String: [String: String]]) {
        viewModel.document.pushLayoutUndoSnapshot(forModel: modelName)
        guard viewModel.document.setDimmingInfo(modelName,
                                                 entries: entries) else { return }
        summaryToken &+= 1
        hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
        canUndo = viewModel.document.canUndoLayoutChange()
    }

    /// J-18 pass 6 — clear the dimming curve on a model. Curve
    /// editing isn't on iPad yet; clear is the only mutation.
    private func clearDimmingCurve(modelName: String) {
        viewModel.document.pushLayoutUndoSnapshot(forModel: modelName)
        guard viewModel.document.clearDimmingCurve(onModel: modelName) else { return }
        summaryToken &+= 1
        hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
        canUndo = viewModel.document.canUndoLayoutChange()
    }

    /// J-18 pass 4 — delete one submodel from its parent. Bridge
    /// call is per-submodel rather than wholesale-replace because
    /// SubModels carry per-instance geometry (range/lines), and
    /// rebuilding them from the iPad isn't in scope yet — delete
    /// is the only edit operation we expose.
    private func deleteSubModel(modelName: String, submodelName: String) {
        viewModel.document.pushLayoutUndoSnapshot(forModel: modelName)
        guard viewModel.document.deleteSubModelNamed(submodelName,
                                                    onModel: modelName) else { return }
        summaryToken &+= 1
        hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
        canUndo = viewModel.document.canUndoLayoutChange()
    }

    /// J-18 pass 3 — wholesale-replace strand or node names.
    /// Empty trailing slots are kept so positional indexing is
    /// preserved on the bridge side (comma-join puts blanks
    /// between commas).
    private func commitIndexedNames(modelName: String,
                                     kind: ModelDataKind,
                                     names: [String]) {
        viewModel.document.pushLayoutUndoSnapshot(forModel: modelName)
        let ok: Bool
        switch kind {
        case .strands: ok = viewModel.document.setStrandNames(modelName, names: names)
        case .nodes:   ok = viewModel.document.setNodeNames(modelName, names: names)
        default:       return
        }
        guard ok else { return }
        summaryToken &+= 1
        hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
        canUndo = viewModel.document.canUndoLayoutChange()
    }

    private func commitProperty(modelName: String, key: String, value: Any) {
        // Capture undo BEFORE the edit so the snapshot reflects the
        // pre-edit state. Pushed unconditionally — even no-op edits
        // leave a stale entry, which we accept as cheap (the user
        // can just hit Undo twice).
        viewModel.document.pushLayoutUndoSnapshot(forModel: modelName)
        let changed = viewModel.document.setLayoutModelProperty(modelName,
                                                                key: key,
                                                                value: value)
        if changed {
            summaryToken &+= 1
            hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
            canUndo = viewModel.document.canUndoLayoutChange()
            // layoutGroup edits change which models are filtered into
            // the active group; controllerName edits affect the
            // displayed channel range / strings via Reinitialize. Both
            // mean the sidebar list / summary need to repaint.
            if key == "layoutGroup" || key == "controllerName" {
                refreshModelList()
            }
        }
    }

    /// Arrow-key nudge: +1 unit per tap, +10 with shift. dx/dy are
    /// signed direction (-1, 0, +1); the magnitude is decided here.
    /// Returns `.handled` only when there's a selected model so the
    /// arrow key still scrolls the sidebar list when no model is
    /// active. Each tap creates one undo entry.
    private func nudge(_ dxSign: Float, _ dySign: Float, _ keyPress: KeyPress) -> KeyPress.Result {
        guard let name = viewModel.layoutEditorSelectedModel else { return .ignored }
        guard let summary = viewModel.document.modelLayoutSummary(name) else { return .ignored }
        let big = keyPress.modifiers.contains(.shift)
        let step: Float = big ? 10.0 : 1.0
        let dx = dxSign * step
        let dy = dySign * step

        viewModel.document.pushLayoutUndoSnapshot(forModel: name)

        if dx != 0 {
            let cur = (summary["centerX"] as? NSNumber)?.floatValue ?? 0
            _ = viewModel.document.setLayoutModelProperty(name,
                                                          key: "centerX",
                                                          value: NSNumber(value: cur + dx))
        }
        if dy != 0 {
            let cur = (summary["centerY"] as? NSNumber)?.floatValue ?? 0
            _ = viewModel.document.setLayoutModelProperty(name,
                                                          key: "centerY",
                                                          value: NSNumber(value: cur + dy))
        }

        summaryToken &+= 1
        hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
        canUndo = viewModel.document.canUndoLayoutChange()
        // Repaint the canvas — the bridge state changed but updateUIView
        // doesn't notice without a setNeedsDisplay nudge.
        NotificationCenter.default.post(name: .layoutEditorModelMoved,
                                        object: "LayoutEditor",
                                        userInfo: ["model": name])
        return .handled
    }

    private func performUndo() {
        guard viewModel.document.undoLastLayoutChange() else { return }
        summaryToken &+= 1
        hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
        canUndo = viewModel.document.canUndoLayoutChange()
        // The undo may have moved a model out of the active group
        // (or back into it) — refresh the list so the sidebar
        // reflects post-undo state.
        refreshModelList()
        // Force a canvas repaint by posting our model-moved
        // notification; the bridge will see the bumped model state
        // on the next draw.
        if let sel = viewModel.layoutEditorSelectedModel {
            NotificationCenter.default.post(name: .layoutEditorModelMoved,
                                            object: "LayoutEditor",
                                            userInfo: ["model": sel])
        }
    }

    /// J-3 (touch UX) — exit fresh-model placement and / or mid-
    /// polyline create. Returns `.handled` if either mode was
    /// active so the keypress is consumed (otherwise Esc/Return
    /// would fall through to focused controls / sidebar).
    private func endCreationModes() -> KeyPress.Result {
        var consumed = false
        if viewModel.layoutPendingNewModelType != nil {
            viewModel.layoutPendingNewModelType = nil
            consumed = true
        }
        if viewModel.layoutPolylineInProgress != nil {
            viewModel.layoutPolylineInProgress = nil
            consumed = true
        }
        if viewModel.layoutPendingImportPath != nil {
            viewModel.layoutPendingImportPath = nil
            consumed = true
        }
        return consumed ? .handled : .ignored
    }

    /// J-3 (touch UX) — delete the named model from the bridge,
    /// clear selection so the action bar/toolbar go away, then
    /// refresh the sidebar list and dirty/undo state. Repaint the
    /// canvas via the standard layout-mutation notification so the
    /// model disappears immediately. Persisting requires Save —
    /// matches every other layout edit.
    private func deleteModel(name: String) {
        guard viewModel.document.deleteModel(name) else { return }
        if viewModel.layoutEditorSelectedModel == name {
            viewModel.layoutSelectSingle(nil)
        }
        summaryToken &+= 1
        hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
        canUndo = viewModel.document.canUndoLayoutChange()
        refreshModelList()
        NotificationCenter.default.post(name: .layoutEditorModelMoved,
                                        object: "LayoutEditor",
                                        userInfo: ["model": name])
    }

    // MARK: - J-4 multi-select operations

    /// Align all selected models' named edge / centre to the
    /// primary (leader). Pushes an undo snapshot per moved model
    /// so the user can revert individual moves; bumps the summary
    /// token + dirty state and repaints the canvas.
    private func performAlign(by edge: String) {
        guard let leader = viewModel.layoutEditorSelectedModel,
              viewModel.layoutEditorSelection.count >= 2 else { return }
        let names = Array(viewModel.layoutEditorSelection)
        for n in names where n != leader {
            viewModel.document.pushLayoutUndoSnapshot(forModel: n)
        }
        guard let bridge = XLightsBridgeBox.bridgeForLayoutEditor() else { return }
        let moved = bridge.alignModels(names,
                                        toLeader: leader,
                                        by: edge,
                                        for: viewModel.document)
        if moved {
            summaryToken &+= 1
            hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
            canUndo = viewModel.document.canUndoLayoutChange()
            NotificationCenter.default.post(name: .layoutEditorModelMoved,
                                             object: "LayoutEditor",
                                             userInfo: ["model": leader])
        }
    }

    /// Distribute centres along the named axis. Snapshots every
    /// candidate before mutating so single-step undo works.
    private func performDistribute(axis: String) {
        guard viewModel.layoutEditorSelection.count >= 3 else { return }
        let names = Array(viewModel.layoutEditorSelection)
        for n in names {
            viewModel.document.pushLayoutUndoSnapshot(forModel: n)
        }
        guard let bridge = XLightsBridgeBox.bridgeForLayoutEditor() else { return }
        let moved = bridge.distributeModels(names,
                                              axis: axis,
                                              for: viewModel.document)
        if moved {
            summaryToken &+= 1
            hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
            canUndo = viewModel.document.canUndoLayoutChange()
            NotificationCenter.default.post(name: .layoutEditorModelMoved,
                                             object: "LayoutEditor",
                                             userInfo: [:])
        }
    }

    /// Resize the selection (except leader) so the named dimension
    /// matches the leader. `dim` ∈ {"width","height","depth","all"}.
    private func performMatchSize(dimension dim: String) {
        guard let leader = viewModel.layoutEditorSelectedModel,
              viewModel.layoutEditorSelection.count >= 2 else { return }
        let names = Array(viewModel.layoutEditorSelection)
        for n in names where n != leader {
            viewModel.document.pushLayoutUndoSnapshot(forModel: n)
        }
        guard let bridge = XLightsBridgeBox.bridgeForLayoutEditor() else { return }
        let resized = bridge.matchSize(ofModels: names,
                                         toLeader: leader,
                                         dimension: dim,
                                         for: viewModel.document)
        if resized {
            summaryToken &+= 1
            hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
            canUndo = viewModel.document.canUndoLayoutChange()
            NotificationCenter.default.post(name: .layoutEditorModelMoved,
                                             object: "LayoutEditor",
                                             userInfo: ["model": leader])
        }
    }

    /// J-7 — Flip the multi-selection 180° about the given axis.
    /// Uses the bridge implementation (matches desktop flip math).
    /// Each model flips in place; pushing undo per model so each
    /// flip is independently revertible.
    private func performFlip(axis: String) {
        guard !viewModel.layoutEditorSelection.isEmpty else { return }
        let names = Array(viewModel.layoutEditorSelection)
        for n in names {
            viewModel.document.pushLayoutUndoSnapshot(forModel: n)
        }
        guard let bridge = XLightsBridgeBox.bridgeForLayoutEditor() else { return }
        let flipped = bridge.flipModels(names, axis: axis, for: viewModel.document)
        if flipped {
            summaryToken &+= 1
            hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
            canUndo = viewModel.document.canUndoLayoutChange()
            NotificationCenter.default.post(name: .layoutEditorModelMoved,
                                             object: "LayoutEditor",
                                             userInfo: [:])
        }
    }

    /// J-7 — Duplicate each model in `names`. The bridge clones
    /// them with a small (+50, +50) offset so they don't overlap
    /// the originals; the new selection becomes the duplicates so
    /// the user can immediately drag them to position.
    private func performDuplicate(of names: [String]) {
        guard !names.isEmpty else { return }
        guard let bridge = XLightsBridgeBox.bridgeForLayoutEditor() else { return }
        let dups = bridge.duplicateModels(names, for: viewModel.document)
        guard !dups.isEmpty else { return }
        summaryToken &+= 1
        hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
        refreshModelList()
        // Shift selection to the new duplicates so the user can
        // drag / nudge them right away.
        viewModel.layoutEditorSelection = Set(dups)
        viewModel.layoutEditorSelectedModel = dups.first
        NotificationCenter.default.post(name: .layoutEditorModelMoved,
                                         object: "LayoutEditor",
                                         userInfo: [:])
    }

    /// J-7 — Group-from-selection. Reuses NewGroupSheet for the
    /// name prompt; the create handler reads
    /// `pendingGroupFromSelection` to decide whether to pass
    /// members through to the bridge.
    private func newGroupFromSelectionPrompt() {
        guard !viewModel.layoutEditorSelection.isEmpty else { return }
        pendingGroupFromSelection = Array(viewModel.layoutEditorSelection)
        newGroupSheetVisible = true
    }

    private func saveLayoutChanges() {
        let ok = viewModel.document.saveLayoutChanges()
        if ok {
            hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
        } else {
            saveErrorMessage = "Couldn't write xlights_rgbeffects.xml. Check the Tools → Package Logs output for details."
        }
    }

    /// J-2 — roll back every staged in-memory layout edit by
    /// repeatedly popping the undo stack. Walks until empty (or
    /// until 200 iterations as a safety bound). The dirty set
    /// stays populated — `MarkLayoutModelDirty` was called by each
    /// undo restore — but it's now redundant since the in-memory
    /// state matches what was on disk before the edit session.
    /// Caller refresh repaints the canvas + sidebar summary.
    private func discardChanges() {
        var iterations = 0
        while viewModel.document.canUndoLayoutChange() && iterations < 200 {
            _ = viewModel.document.undoLastLayoutChange()
            iterations += 1
        }
        // Undo's restore path re-marked every reverted model
        // dirty even though the in-memory state now matches what's
        // on disk. Drop that bookkeeping so the Save button
        // disables cleanly.
        viewModel.document.clearDirtyLayoutChanges()
        summaryToken &+= 1
        hasUnsavedChanges = viewModel.document.hasUnsavedLayoutChanges()
        canUndo = viewModel.document.canUndoLayoutChange()
        refreshModelList()
        if let sel = viewModel.layoutEditorSelectedModel {
            NotificationCenter.default.post(name: .layoutEditorModelMoved,
                                            object: "LayoutEditor",
                                            userInfo: ["model": sel])
        }
    }
}

/// J-8 — Editable properties panel for the selected model.
///
/// Ordered to match desktop's wxPropertyGrid:
/// 1. Header (Name / Type / always visible)
/// 2. Model Properties (per-type descriptors — expanded by default)
/// 3. Controller Connection
/// 4. String Properties
/// 5. Appearance
/// 6. Dimensions
/// 7. Size/Location
///
/// Each section after #1 is a `DisclosureGroup` so the user can
/// collapse what they don't need. Default-expanded: per-type only —
/// matching desktop's "the bits that vary per model first" intent.
private struct LayoutEditorPropertiesView: View {
    let modelName: String
    let summary: [String: Any]
    let typeDescriptors: [[String: Any]]
    let layoutGroups: [String]
    let token: Int
    let commit: (_ key: String, _ value: Any) -> Void
    let typeCommit: (_ key: String, _ value: Any) -> Void
    let onRenameRequest: () -> Void
    let onCommitAliases: (_ aliases: [String]) -> Void
    let onCommitIndexedNames: (_ kind: ModelDataKind, _ names: [String]) -> Void
    let onDeleteSubModel: (_ submodelName: String) -> Void
    let onAddSubModel: (_ submodelName: String) -> String?
    let onRenameSubModel: (_ oldName: String, _ newName: String) -> Bool
    let onLoadSubModelDetails: () -> [SubModelEntry]
    let onCommitSubModelDetails: (_ entries: [SubModelEntry]) -> Bool
    let onNavigateToGroup: (_ groupName: String) -> Void
    let onClearDimmingCurve: () -> Void
    let onPickImageFile: (_ key: String) -> Void
    let onLoadFaceState: (_ kind: ModelDataKind) -> [String: [String: String]]
    let onCommitFaceState: (_ kind: ModelDataKind, _ entries: [String: [String: String]]) -> Void
    let onLoadDimming: () -> [String: [String: String]]
    let onCommitDimming: (_ entries: [String: [String: String]]) -> Void
    let onOpenCustomModelEditor: () -> Void

    @State private var expandedTypeProps: Bool = true
    @State private var expandedController: Bool = false
    @State private var expandedControllerConnection: Bool = false
    @State private var expandedStringProps: Bool = false
    @State private var expandedModelData: Bool = false
    @State private var expandedAppearance: Bool = false
    @State private var expandedDimensions: Bool = false
    @State private var expandedSizeLocation: Bool = false
    // J-18 — which read-only popup viewer (Faces / States /
    // SubModels / etc) is currently presented. nil = none.
    @State private var modelDataViewer: ModelDataKind? = nil
    @State private var pendingClearDimmingCurve: Bool = false
    // J-22 — dimming curve editor sheet visibility.
    @State private var dimmingEditorVisible: Bool = false
    // J-19 — Layered Arches layer-size editor.
    // J-20.7 — Switched to `.sheet(item:)` so the sheet is bound
    // to the data's identity rather than a separate boolean flag.
    // The old `.sheet(isPresented:)` + `pendingLayerSizes` pattern
    // had a race where the sheet's @State sometimes initialised
    // from a stale capture of `pendingLayerSizes`.
    @State private var layerSizesEditorPayload: LayerSizesPayload? = nil
    // J-20 — Start Channel structured editor. Initial value /
    // commit key are captured at open time so the sheet doesn't
    // need to know about per-string vs. model-wide routing.
    @State private var startChannelEditorVisible: Bool = false
    @State private var pendingStartChannelKey: String = ""
    @State private var pendingStartChannelInitial: String = ""

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            // 1. Header — non-collapsible identity block.
            row("Name") {
                HStack(spacing: 6) {
                    Text(modelName)
                        .truncationMode(.middle)
                    Button {
                        onRenameRequest()
                    } label: {
                        Image(systemName: "pencil")
                            .font(.caption2)
                    }
                    .buttonStyle(.plain)
                    .foregroundStyle(.secondary)
                    .accessibilityLabel("Rename model")
                }
            }
            row("Type") { Text(summary["displayAs"] as? String ?? "—") }
            // J-19 — Description belongs in the model-header block
            // (matching desktop), not under Appearance.
            row("Description") { descriptionField }

            // 2. Model Properties (per-type). Title carries the
            // model's `displayAs` for parity with desktop's
            // category-header convention.
            if !typeDescriptors.isEmpty {
                section($expandedTypeProps,
                        title: typeSectionTitle) {
                    ForEach(Array(typeDescriptors.enumerated()), id: \.offset) { _, d in
                        typeDescriptorRow(d)
                    }
                }
            }

            // J-19/J-20 — model-header block (between Type and the
            // real Controller Connection): Preview, Controller
            // picker, Start Channel + indiv per-string,
            // Model Chain, channel range. Desktop has these as
            // un-categorised rows above the Controller Connection
            // section; iPad gives them their own collapsible to
            // keep the property pane tidy.
            section($expandedController, title: "Model") {
                row("Preview") { layoutGroupPicker }
                row("Controller") { controllerField }
                // J-20 — Low Definition Factor is only meaningful
                // on models whose render path supports it (matrix
                // and image types). Hidden otherwise.
                if (summary["supportsLowDefinition"] as? Bool) ?? false {
                    row("Low Def Factor") { lowDefFactorField }
                }
                row("Shadow Model For") { shadowModelPicker }
                let hasMulti = (summary["hasMultipleStrings"] as? Bool) ?? false
                let hasIndiv = (summary["hasIndividualStartChannels"] as? Bool) ?? false
                if hasMulti {
                    row("Indiv Start Chans") {
                        Toggle("", isOn: Binding(
                            get: { hasIndiv },
                            set: { commit("hasIndividualStartChannels", NSNumber(value: $0)) }
                        ))
                        .labelsHidden()
                        .controlSize(.mini)
                    }
                    if hasIndiv {
                        let chans = (summary["individualStartChannels"] as? [String]) ?? []
                        ForEach(Array(chans.enumerated()), id: \.offset) { idx, ch in
                            row("String \(idx + 1)") {
                                startChannelField(key: "individualStartChannel\(idx)",
                                                   initial: ch)
                            }
                        }
                    } else {
                        row("Start Channel") { modelStartChannelField }
                    }
                } else {
                    row("Start Channel") { modelStartChannelField }
                }
                if (summary["modelChainApplicable"] as? Bool) ?? false {
                    row("Model Chain") { modelChainPicker }
                }
                row("Channel range") {
                    Text("\(uintVal("startChannel"))..\(uintVal("endChannel"))")
                        .foregroundStyle(.secondary)
                }
            }

            // J-20 — real Controller Connection section. Mirrors
            // desktop ModelPropertyAdapter::AddControllerProperties:
            // port (gated max by protocol), protocol enum, smart
            // remote subsection (pixel + caps>0), serial-only
            // DMX channel + Speed, PWM gamma/brightness, per-pixel
            // toggles for null pixels / brightness / gamma / color
            // order / direction / group count / zig-zag / smart Ts
            // — each row only shown when the controller caps say
            // it's supported.
            // J-20.5 — Model Data section moved here so it sits
            // between the model-header (Model) and the real
            // Controller Connection — matches desktop's ordering
            // where the popup-dialog properties appear under the
            // header block, above the controller-connection
            // category.
            section($expandedModelData, title: "Model Data") {
                modelDataRow(kind: .submodels)
                modelDataRow(kind: .faces)
                modelDataRow(kind: .states)
                modelDataRow(kind: .aliases)
                modelDataRow(kind: .strands)
                modelDataRow(kind: .nodes)
                modelDataRow(kind: .groups)
                row("Dimming Curve") {
                    HStack(spacing: 6) {
                        Text(hasDimmingCurve ? "Set" : "—")
                            .foregroundStyle(.secondary)
                        if !isDisabled("dimmingCurve") {
                            // J-22 — pencil opens the same nested-
                            // map editor used for faces / states.
                            // Dimming channels are "all", "red",
                            // "green", "blue", "white"; each has
                            // its own attribute map (type / gamma
                            // / offset / etc.).
                            Button {
                                dimmingEditorVisible = true
                            } label: {
                                Image(systemName: "pencil")
                                    .font(.caption2)
                            }
                            .buttonStyle(.plain)
                            .foregroundStyle(.secondary)
                            .accessibilityLabel("Edit dimming curve")
                            if hasDimmingCurve {
                                Button {
                                    pendingClearDimmingCurve = true
                                } label: {
                                    Image(systemName: "trash")
                                        .font(.caption2)
                                }
                                .buttonStyle(.plain)
                                .foregroundStyle(.secondary)
                                .accessibilityLabel("Clear dimming curve")
                            }
                        }
                    }
                }
                .opacity(isDisabled("dimmingCurve") ? 0.45 : 1.0)
            }
            .sheet(isPresented: $dimmingEditorVisible) {
                FaceStateEditorSheet(
                    payload: FaceStatePayload(
                        kind: .dimming,
                        entries: onLoadDimming(),
                        modelName: modelName),
                    commit: onCommitDimming)
            }
            .alert("Clear dimming curve?",
                   isPresented: $pendingClearDimmingCurve) {
                Button("Cancel", role: .cancel) {}
                Button("Clear", role: .destructive) {
                    onClearDimmingCurve()
                }
            } message: {
                Text("Removes the dimming curve from \(modelName). Editing curves isn't yet available on iPad — you'll need the desktop to add one back.")
            }

            section($expandedControllerConnection, title: "Controller Connection") {
                controllerConnectionFields
            }

            // J-20.5 — String Properties. Strips the bogus
            // "Strings" + "Nodes" rows that weren't in desktop's
            // section. Only String Type + dynamic colour controls
            // + RGBW Handling, matching desktop verbatim.
            section($expandedStringProps, title: "String Properties") {
                row("String Type") { stringTypePicker }
                    .opacity(isDisabled("stringType") ? 0.45 : 1.0)
                    .disabled(isDisabled("stringType"))
                let mode = summary["stringColorMode"] as? String ?? "none"
                if mode == "single" {
                    row("Color") { stringColorRow }
                } else if mode == "superstring" {
                    row("Colours") {
                        LayoutEditorIntSpin(
                            id: "\(modelName).superStringCount.\(token)",
                            initial: (summary["superStringCount"] as? NSNumber)?.intValue ?? 1,
                            range: 1...32,
                            commit: { commit("superStringCount", NSNumber(value: $0)) }
                        )
                        .frame(maxWidth: 140, alignment: .trailing)
                    }
                    let colours = (summary["superStringColours"] as? [String]) ?? []
                    ForEach(Array(colours.enumerated()), id: \.offset) { idx, hex in
                        row("Colour \(idx + 1)") {
                            // J-21 — Editable Superstring colour via
                            // the new per-index bridge setter. sRGB
                            // round-trip via the shared helpers.
                            ColorPicker("",
                                         selection: Binding(
                                            get: { hexColor(hex) },
                                            set: { commit("superStringColour\(idx)",
                                                          hexFromColor($0) as NSString) }
                                         ),
                                         supportsOpacity: false)
                                .labelsHidden()
                                .frame(width: 40, height: 24)
                        }
                    }
                } else {
                    row("Color") {
                        Text("—").foregroundStyle(.tertiary)
                    }
                }
                row("RGBW Handling") { rgbwHandlingPicker }
            }

            // 5. Appearance.
            section($expandedAppearance, title: "Appearance") {
                row("Active") {
                    Toggle("", isOn: boolBinding(key: "active"))
                        .labelsHidden()
                        .controlSize(.mini)
                }
                row("Pixel Size") { intField(key: "pixelSize", min: 1, max: 100) }
                    .opacity(isDisabled("pixelSize") ? 0.45 : 1.0)
                    .disabled(isDisabled("pixelSize"))
                row("Pixel Style") { pixelStylePicker }
                    .opacity(isDisabled("pixelStyle") ? 0.45 : 1.0)
                    .disabled(isDisabled("pixelStyle"))
                row("Transparency") { intField(key: "transparency", min: 0, max: 100) }
                row("Black Transparency") {
                    intField(key: "blackTransparency", min: 0, max: 100)
                }
                    .opacity(isDisabled("blackTransparency") ? 0.45 : 1.0)
                    .disabled(isDisabled("blackTransparency"))
                row("Tag Color") { tagColorPicker }
            }

            // 6. Dimensions.
            section($expandedDimensions, title: "Dimensions") {
                row("Width")  { numberField(key: "width",  min: 0) }
                row("Height") { numberField(key: "height", min: 0) }
                row("Depth")  { numberField(key: "depth",  min: 0) }
            }

            // 7. Size/Location. J-19 — branch by screen-location
            // class so Arches / two-point / three-point models see
            // their actual endpoints (X1..Z2 + Height) and Boxed
            // models see World + Scale + Rotate. Mirrors desktop's
            // ScreenLocationPropertyHelper.
            section($expandedSizeLocation, title: "Size/Location") {
                row("Locked") {
                    Toggle("", isOn: lockedBinding)
                        .labelsHidden()
                        .controlSize(.mini)
                }
                sizeLocationFields
            }
        }
        .font(.caption.monospacedDigit())
        .sheet(item: $layerSizesEditorPayload) { payload in
            LayerSizesEditorSheet(initial: payload.sizes,
                                  commit: { sizes in
                                      typeCommit("LayerSizes",
                                                  sizes.map { NSNumber(value: $0) } as NSArray)
                                  })
        }
        .sheet(isPresented: $startChannelEditorVisible) {
            StartChannelEditorSheet(
                initial: pendingStartChannelInitial,
                modelOptions: (summary["controllerOptions"] as? [String]) ?? [],
                controllerOptions: ((summary["controllerOptions"] as? [String]) ?? [])
                    .filter { $0 != "Use Start Channel" && $0 != "No Controller" },
                otherModelOptions: otherModelNames,
                commit: { newValue in
                    commit(pendingStartChannelKey, newValue as NSString)
                })
        }
        .sheet(item: $modelDataViewer) { kind in
            if kind == .aliases {
                AliasEditorSheet(modelName: modelName,
                                 initial: entries(for: kind),
                                 commit: onCommitAliases)
            } else if kind == .strands || kind == .nodes {
                IndexedNamesEditorSheet(
                    modelName: modelName,
                    title: kind.title,
                    placeholderFor: { idx in
                        kind == .strands ? "Strand \(idx + 1)" : "Node \(idx + 1)"
                    },
                    initial: entries(for: kind),
                    commit: { names in
                        onCommitIndexedNames(kind, names)
                    })
            } else if kind == .submodels {
                SubModelListSheet(
                    modelName: modelName,
                    initial: entries(for: kind),
                    delete: onDeleteSubModel,
                    add: onAddSubModel,
                    rename: onRenameSubModel,
                    loadDetails: onLoadSubModelDetails,
                    commitDetails: onCommitSubModelDetails)
            } else if kind == .faces || kind == .states {
                // J-22 — full Faces / States editor. Bridge read
                // / write happens in the parent — we just hand
                // the entries through callbacks.
                FaceStateEditorSheet(
                    payload: FaceStatePayload(
                        kind: kind == .faces ? .faces : .states,
                        entries: onLoadFaceState(kind),
                        modelName: modelName),
                    commit: { newEntries in
                        onCommitFaceState(kind, newEntries)
                    })
            } else if kind == .groups {
                GroupRefListSheet(modelName: modelName,
                                  groups: entries(for: kind),
                                  onTap: { name in
                                      modelDataViewer = nil
                                      onNavigateToGroup(name)
                                  })
            } else {
                ModelDataViewerSheet(title: kind.title,
                                     entries: entries(for: kind))
            }
        }
    }


    // MARK: - Section header

    private var typeSectionTitle: String {
        // Desktop uses the type string verbatim as the category
        // label. Falls back to "Model Properties" if missing.
        let t = summary["displayAs"] as? String ?? ""
        return t.isEmpty ? "Model Properties" : t
    }

    @ViewBuilder
    private func section<Content: View>(_ expanded: Binding<Bool>,
                                         title: String,
                                         @ViewBuilder _ content: () -> Content) -> some View {
        // DisclosureGroup's content closure is escaping, so we
        // materialize `content` once here and let DisclosureGroup
        // capture the resulting View. Otherwise SwiftUI complains
        // that the non-escaping `content` is captured by an
        // escaping closure.
        let body = VStack(alignment: .leading, spacing: 4) { content() }
            .padding(.vertical, 4)
        DisclosureGroup(isExpanded: expanded) {
            body
        } label: {
            Text(title)
                .font(.caption.weight(.semibold))
                .foregroundStyle(.primary)
        }
        .accentColor(.secondary)
    }

    // MARK: - Rows

    @ViewBuilder
    private func row(_ label: String, @ViewBuilder _ content: () -> some View) -> some View {
        HStack(alignment: .firstTextBaseline) {
            Text(label)
                .foregroundStyle(.secondary)
                .frame(minWidth: 110, alignment: .leading)
            Spacer(minLength: 8)
            content()
                .lineLimit(1)
        }
    }

    // MARK: - Editors

    /// J-20.7 — Numeric field for layout properties (positions,
    /// dimensions, appearance ints). Always renders as a spin
    /// widget (editable TextField + ± Stepper) so the user can
    /// type a value AND nudge by a small step. Caller passes
    /// `min` / `max` / `step` to gate the ± range; missing
    /// bounds fall back to ±1e9 so the Stepper still works for
    /// unbounded world coordinates.
    private func numberField(key: String,
                              min: Double? = nil,
                              max: Double? = nil,
                              step: Double = 1.0,
                              precision: Int = 2) -> some View {
        let lo = min ?? -1_000_000_000
        let hi = max ??  1_000_000_000
        return LayoutEditorDoubleSpin(
            id: "\(modelName).\(key).\(token)",
            initial: doubleVal(key),
            range: lo...hi,
            step: step,
            precision: precision,
            commit: { newValue in commit(key, NSNumber(value: newValue)) }
        )
        .frame(maxWidth: 160, alignment: .trailing)
    }

    private func intField(key: String, min: Double?, max: Double?) -> some View {
        let lo = Int((min ?? -1_000_000).rounded())
        let hi = Int((max ??  1_000_000).rounded())
        return LayoutEditorIntSpin(
            id: "\(modelName).\(key).\(token)",
            initial: intVal(key),
            range: lo...hi,
            commit: { newValue in commit(key, NSNumber(value: newValue)) }
        )
        .frame(maxWidth: 140, alignment: .trailing)
    }

    private var layoutGroupPicker: some View {
        Menu {
            ForEach(layoutGroups, id: \.self) { name in
                Button {
                    commit("layoutGroup", name as NSString)
                } label: {
                    HStack {
                        Text(name)
                        if name == (summary["layoutGroup"] as? String) {
                            Spacer()
                            Image(systemName: "checkmark")
                        }
                    }
                }
            }
        } label: {
            Text(summary["layoutGroup"] as? String ?? "Default")
                .truncationMode(.middle)
        }
        .menuStyle(.button)
        .controlSize(.mini)
    }

    private var stringTypePicker: some View {
        let options = (summary["stringTypeOptions"] as? [String]) ?? []
        let current = summary["stringType"] as? String ?? ""
        return Menu {
            ForEach(options, id: \.self) { name in
                Button {
                    commit("stringType", name as NSString)
                } label: {
                    HStack {
                        Text(name)
                        if name == current {
                            Spacer()
                            Image(systemName: "checkmark")
                        }
                    }
                }
            }
        } label: {
            Text(current.isEmpty ? "—" : current)
                .truncationMode(.middle)
                .frame(maxWidth: 160, alignment: .trailing)
        }
        .menuStyle(.button)
        .controlSize(.mini)
    }

    private var pixelStylePicker: some View {
        let options = (summary["pixelStyleOptions"] as? [String]) ?? []
        let idx = intVal("pixelStyle")
        let current = (idx >= 0 && idx < options.count) ? options[idx] : "—"
        return Menu {
            ForEach(Array(options.enumerated()), id: \.offset) { i, name in
                Button {
                    commit("pixelStyle", NSNumber(value: i))
                } label: {
                    HStack {
                        Text(name)
                        if i == idx {
                            Spacer()
                            Image(systemName: "checkmark")
                        }
                    }
                }
            }
        } label: {
            Text(current)
                .truncationMode(.middle)
                .frame(maxWidth: 130, alignment: .trailing)
        }
        .menuStyle(.button)
        .controlSize(.mini)
    }

    /// Tag color is stored on Model as a string (typically
    /// `#RRGGBB`). SwiftUI's ColorPicker gives us a Color; we
    /// convert to hex on commit. Showing the swatch + label
    /// matches desktop's "block + text" presentation.
    private var tagColorPicker: some View {
        let hex = summary["tagColor"] as? String ?? ""
        let parsed = Color(hexString: hex) ?? .black
        return HStack(spacing: 6) {
            ColorPicker("", selection: Binding(
                get: { parsed },
                set: { newColor in
                    let s = newColor.toHexString()
                    commit("tagColor", s as NSString)
                }
            ), supportsOpacity: false)
                .labelsHidden()
            Text(hex.isEmpty ? "—" : hex)
                .font(.caption2)
                .foregroundStyle(.secondary)
                .frame(maxWidth: 80, alignment: .leading)
        }
    }

    private var descriptionField: some View {
        LayoutEditorStringField(
            id: "\(modelName).description.\(token)",
            initial: summary["description"] as? String ?? "",
            commit: { commit("description", $0 as NSString) }
        )
        .frame(maxWidth: 160, alignment: .trailing)
    }

    private var lockedBinding: Binding<Bool> {
        Binding(
            get: { summary["locked"] as? Bool ?? false },
            set: { commit("locked", NSNumber(value: $0)) }
        )
    }

    private func boolBinding(key: String) -> Binding<Bool> {
        Binding(
            get: { summary[key] as? Bool ?? false },
            set: { commit(key, NSNumber(value: $0)) }
        )
    }

    /// J-20 — Controller Connection body. Reads everything from
    /// the `controllerConnection` sub-dictionary the bridge
    /// builds — pixel/serial/PWM branching is decided there based
    /// on the model's current protocol + caps.
    @ViewBuilder
    private var controllerConnectionFields: some View {
        let cc = (summary["controllerConnection"] as? [String: Any]) ?? [:]
        if cc.isEmpty {
            Text("No controller selected.")
                .font(.caption2)
                .foregroundStyle(.tertiary)
        } else {
            ccPortRow(cc)
            ccProtocolRow(cc)
            // Smart-Remote subsection appears above the per-pixel
            // toggles when the protocol supports it.
            if ((cc["smartRemoteCount"] as? NSNumber)?.intValue ?? 0) > 0 {
                ccSmartRemoteRows(cc)
            }
            // Serial-only DMX channel + Speed.
            if (cc["isSerialProtocol"] as? Bool) ?? false {
                let dmxMax = (cc["dmxChannelMax"] as? NSNumber)?.intValue ?? 512
                row("DMX Channel") {
                    ccIntField("cc.dmxChannel", value: cc["dmxChannel"], minV: 1, maxV: Double(dmxMax))
                }
                if let speeds = cc["speedOptions"] as? [String], !speeds.isEmpty {
                    let idx = (cc["speedIndex"] as? NSNumber)?.intValue ?? 0
                    row("Speed") { ccEnumPicker(key: "cc.speedIndex", index: idx, options: speeds) }
                }
            }
            // PWM-only gamma + brightness (no toggles).
            if (cc["isPWMProtocol"] as? Bool) ?? false {
                row("Gamma")      { ccDoubleField("cc.pwmGamma", value: cc["pwmGamma"], minV: 0.1, maxV: 5.0, step: 0.1, precision: 1) }
                row("Brightness") { ccIntField("cc.pwmBrightness", value: cc["pwmBrightness"], minV: 0, maxV: 100) }
            }
            // Pixel per-pixel toggles. Each row only renders when
            // the caps flag is on; toggling the Active checkbox
            // flips the corresponding CTRL_PROPS flag in the
            // bridge.
            if (cc["isPixelProtocol"] as? Bool) ?? false {
                ccPixelToggleRow(cc, label: "Start Null Pixels",
                                  activeKey: "cc.startNullsActive", activeFlag: "startNullsActive",
                                  valueKey: "cc.startNulls", value: cc["startNulls"],
                                  minV: 0, maxV: 100, supportsFlag: "supportsStartNulls")
                ccPixelToggleRow(cc, label: "End Null Pixels",
                                  activeKey: "cc.endNullsActive", activeFlag: "endNullsActive",
                                  valueKey: "cc.endNulls", value: cc["endNulls"],
                                  minV: 0, maxV: 100, supportsFlag: "supportsEndNulls")
                ccPixelToggleRow(cc, label: "Brightness",
                                  activeKey: "cc.brightnessActive", activeFlag: "brightnessActive",
                                  valueKey: "cc.brightness", value: cc["brightness"],
                                  minV: 0, maxV: 100, supportsFlag: "supportsBrightness")
                ccPixelToggleRow(cc, label: "Gamma",
                                  activeKey: "cc.gammaActive", activeFlag: "gammaActive",
                                  valueKey: "cc.gamma", value: cc["gamma"],
                                  minV: 0.1, maxV: 5.0, supportsFlag: "supportsGamma",
                                  precision: 1, step: 0.1)
                ccPixelEnumToggleRow(cc, label: "Color Order",
                                      activeKey: "cc.colorOrderActive", activeFlag: "colorOrderActive",
                                      valueKey: "cc.colorOrderIndex", indexFlag: "colorOrderIndex",
                                      optionsFlag: "colorOrderOptions", supportsFlag: "supportsColorOrder")
                ccPixelEnumToggleRow(cc, label: "Direction",
                                      activeKey: "cc.directionActive", activeFlag: "directionActive",
                                      valueKey: "cc.directionIndex", indexFlag: "directionIndex",
                                      optionsFlag: "directionOptions", supportsFlag: "supportsDirection")
                ccPixelToggleRow(cc, label: "Group Count",
                                  activeKey: "cc.groupCountActive", activeFlag: "groupCountActive",
                                  valueKey: "cc.groupCount", value: cc["groupCount"],
                                  minV: 1, maxV: 500, supportsFlag: "supportsGroupCount")
                ccPixelToggleRow(cc, label: "Zig Zag",
                                  activeKey: "cc.zigZagActive", activeFlag: "zigZagActive",
                                  valueKey: "cc.zigZag", value: cc["zigZag"],
                                  minV: 0, maxV: 1000, supportsFlag: "supportsZigZag")
                ccPixelToggleRow(cc, label: "Smart Ts",
                                  activeKey: "cc.smartTsActive", activeFlag: "smartTsActive",
                                  valueKey: "cc.smartTs", value: cc["smartTs"],
                                  minV: 0, maxV: 20, supportsFlag: "supportsSmartTs")
            }
        }
    }

    @ViewBuilder
    private func ccPortRow(_ cc: [String: Any]) -> some View {
        let maxV = (cc["portMax"] as? NSNumber)?.intValue ?? 128
        row("Port") {
            ccIntField("cc.port", value: cc["port"], minV: 0, maxV: Double(maxV))
        }
    }

    @ViewBuilder
    private func ccProtocolRow(_ cc: [String: Any]) -> some View {
        let opts = (cc["protocolOptions"] as? [String]) ?? []
        let idx  = (cc["protocolIndex"] as? NSNumber)?.intValue ?? -1
        if !opts.isEmpty {
            row("Protocol") {
                Menu {
                    ForEach(Array(opts.enumerated()), id: \.offset) { i, name in
                        Button {
                            commit("cc.protocol", name as NSString)
                        } label: {
                            HStack {
                                Text(name)
                                if i == idx {
                                    Spacer()
                                    Image(systemName: "checkmark")
                                }
                            }
                        }
                    }
                } label: {
                    Text(idx >= 0 && idx < opts.count ? opts[idx] : "—")
                        .truncationMode(.middle)
                        .frame(maxWidth: 160, alignment: .trailing)
                }
                .menuStyle(.button)
                .controlSize(.mini)
            }
        }
    }

    @ViewBuilder
    private func ccSmartRemoteRows(_ cc: [String: Any]) -> some View {
        let useSR = (cc["useSmartRemote"] as? Bool) ?? false
        row("Use Smart Remote") {
            Toggle("", isOn: Binding(
                get: { useSR },
                set: { commit("cc.useSmartRemote", NSNumber(value: $0)) }
            ))
            .labelsHidden().controlSize(.mini)
        }
        if useSR {
            if let typeOpts = cc["smartRemoteTypeOptions"] as? [String], !typeOpts.isEmpty {
                let i = (cc["smartRemoteTypeIndex"] as? NSNumber)?.intValue ?? 0
                row("Smart Remote Type") { ccEnumPicker(key: "cc.smartRemoteTypeIndex", index: i, options: typeOpts) }
            } else if let typeText = cc["smartRemoteType"] as? String, !typeText.isEmpty {
                row("Smart Remote Type") {
                    Text(typeText).foregroundStyle(.secondary)
                }
            }
            let srOpts = (cc["smartRemoteOptions"] as? [String]) ?? []
            let srIdx  = (cc["smartRemoteIndex"] as? NSNumber)?.intValue ?? 0
            row("Smart Remote") { ccEnumPicker(key: "cc.smartRemoteIndex", index: srIdx, options: srOpts) }
            if let _ = cc["srMaxCascade"] {
                let mx = (cc["srMaxCascadeMax"] as? NSNumber)?.intValue ?? 15
                row("Max Cascade Remotes") {
                    ccIntField("cc.srMaxCascade", value: cc["srMaxCascade"], minV: 1, maxV: Double(mx))
                }
                row("Cascade On Port") {
                    Toggle("", isOn: Binding(
                        get: { (cc["srCascadeOnPort"] as? Bool) ?? false },
                        set: { commit("cc.srCascadeOnPort", NSNumber(value: $0)) }
                    ))
                    .labelsHidden().controlSize(.mini)
                }
            }
        }
    }

    @ViewBuilder
    private func ccPixelToggleRow(_ cc: [String: Any],
                                    label: String,
                                    activeKey: String,
                                    activeFlag: String,
                                    valueKey: String,
                                    value: Any?,
                                    minV: Double, maxV: Double,
                                    supportsFlag: String,
                                    precision: Int = 0,
                                    step: Double = 1.0) -> some View {
        let supported = (cc[supportsFlag] as? Bool) ?? true
        if supported {
            let active = (cc[activeFlag] as? Bool) ?? false
            row(label) {
                HStack(spacing: 8) {
                    Toggle("", isOn: Binding(
                        get: { active },
                        set: { commit(activeKey, NSNumber(value: $0)) }
                    ))
                    .labelsHidden().controlSize(.mini)
                    if active {
                        if precision > 0 {
                            ccDoubleField(valueKey, value: value, minV: minV, maxV: maxV, step: step, precision: precision)
                        } else {
                            ccIntField(valueKey, value: value, minV: minV, maxV: maxV)
                        }
                    } else {
                        Text("—").foregroundStyle(.tertiary)
                    }
                }
            }
        }
    }

    @ViewBuilder
    private func ccPixelEnumToggleRow(_ cc: [String: Any],
                                        label: String,
                                        activeKey: String,
                                        activeFlag: String,
                                        valueKey: String,
                                        indexFlag: String,
                                        optionsFlag: String,
                                        supportsFlag: String) -> some View {
        let supported = (cc[supportsFlag] as? Bool) ?? true
        if supported {
            let active = (cc[activeFlag] as? Bool) ?? false
            let opts = (cc[optionsFlag] as? [String]) ?? []
            let idx  = (cc[indexFlag] as? NSNumber)?.intValue ?? 0
            row(label) {
                HStack(spacing: 8) {
                    Toggle("", isOn: Binding(
                        get: { active },
                        set: { commit(activeKey, NSNumber(value: $0)) }
                    ))
                    .labelsHidden().controlSize(.mini)
                    if active {
                        ccEnumPicker(key: valueKey, index: idx, options: opts)
                    } else {
                        Text("—").foregroundStyle(.tertiary)
                    }
                }
            }
        }
    }

    @ViewBuilder
    private func ccIntField(_ key: String, value: Any?, minV: Double, maxV: Double) -> some View {
        let v = (value as? NSNumber)?.intValue ?? 0
        let lo = Int(minV.rounded()), hi = Int(maxV.rounded())
        // J-20.5 — always spin widget for ints. Range is bounded
        // by the controller-cap so no unbounded fallback needed.
        LayoutEditorIntSpin(
            id: "\(modelName).\(key).\(token)",
            initial: v, range: lo...hi,
            commit: { commit(key, NSNumber(value: $0)) }
        )
        .frame(maxWidth: 140, alignment: .trailing)
    }

    @ViewBuilder
    private func ccDoubleField(_ key: String, value: Any?,
                                 minV: Double, maxV: Double,
                                 step: Double = 0.1, precision: Int = 2) -> some View {
        let v = (value as? NSNumber)?.doubleValue ?? 0
        // J-20.5 — always spin widget for doubles given a step.
        LayoutEditorDoubleSpin(
            id: "\(modelName).\(key).\(token)",
            initial: v, range: minV...maxV,
            step: step, precision: precision,
            commit: { commit(key, NSNumber(value: $0)) }
        )
        .frame(maxWidth: 160, alignment: .trailing)
    }

    private func ccEnumPicker(key: String, index: Int, options: [String]) -> some View {
        Menu {
            ForEach(Array(options.enumerated()), id: \.offset) { i, label in
                Button {
                    commit(key, NSNumber(value: i))
                } label: {
                    HStack {
                        Text(label)
                        if i == index {
                            Spacer()
                            Image(systemName: "checkmark")
                        }
                    }
                }
            }
        } label: {
            Text(index >= 0 && index < options.count ? options[index] : "—")
                .truncationMode(.middle)
                .frame(maxWidth: 140, alignment: .trailing)
        }
        .menuStyle(.button)
        .controlSize(.mini)
    }

    /// J-19 — Size/Location body keyed off
    /// `screenLocationKind`. Boxed → X/Y/Z + Scale + Rotate;
    /// two/three-point → World + X1/Y1/Z1 + X2/Y2/Z2 (+ Height/
    /// Shear/RotateX for three-point); other → fallback World.
    @ViewBuilder
    private var sizeLocationFields: some View {
        let kind = summary["screenLocationKind"] as? String ?? "boxed"
        let fields = (summary["screenLocationFields"] as? [String: Any]) ?? [:]
        switch kind {
        case "twoPoint":
            row("World X") { screenLocField("worldX", fields: fields) }
            row("World Y") { screenLocField("worldY", fields: fields) }
            row("World Z") { screenLocField("worldZ", fields: fields) }
            row("X1") { screenLocField("x1", fields: fields) }
            row("Y1") { screenLocField("y1", fields: fields) }
            row("Z1") { screenLocField("z1", fields: fields) }
            row("X2") { screenLocField("x2", fields: fields) }
            row("Y2") { screenLocField("y2", fields: fields) }
            row("Z2") { screenLocField("z2", fields: fields) }
        case "threePoint":
            row("World X") { screenLocField("worldX", fields: fields) }
            row("World Y") { screenLocField("worldY", fields: fields) }
            row("World Z") { screenLocField("worldZ", fields: fields) }
            row("X1") { screenLocField("x1", fields: fields) }
            row("Y1") { screenLocField("y1", fields: fields) }
            row("Z1") { screenLocField("z1", fields: fields) }
            row("X2") { screenLocField("x2", fields: fields) }
            row("Y2") { screenLocField("y2", fields: fields) }
            row("Z2") { screenLocField("z2", fields: fields) }
            row("Height") { screenLocField("modelHeight", fields: fields, precision: 2) }
            if (fields["supportsShear"] as? Bool) ?? false {
                row("Shear") { screenLocField("modelShear", fields: fields, precision: 2) }
            }
            row("Rotate X") { screenLocField("rotateX", fields: fields, min: -180, max: 180) }
        case "boxed":
            row("X") { screenLocField("worldX", fields: fields) }
            row("Y") { screenLocField("worldY", fields: fields) }
            row("Z") { screenLocField("worldZ", fields: fields) }
            row("Scale X") { screenLocField("scaleX", fields: fields, precision: 3) }
            row("Scale Y") { screenLocField("scaleY", fields: fields, precision: 3) }
            if (fields["supportsZScaling"] as? Bool) ?? false {
                row("Scale Z") { screenLocField("scaleZ", fields: fields, precision: 3) }
            }
            row("Rotate X") { screenLocField("rotateX", fields: fields, min: -180, max: 180) }
            row("Rotate Y") { screenLocField("rotateY", fields: fields, min: -180, max: 180) }
            row("Rotate Z") { screenLocField("rotateZ", fields: fields, min: -180, max: 180) }
        default:
            row("World X") { screenLocField("worldX", fields: fields) }
            row("World Y") { screenLocField("worldY", fields: fields) }
            row("World Z") { screenLocField("worldZ", fields: fields) }
            Text("Position editing for this model type happens via canvas handles.")
                .font(.caption2)
                .foregroundStyle(.tertiary)
                .fixedSize(horizontal: false, vertical: true)
        }
    }

    /// J-20.7 — Size/Location spin-widget field. Bounded by min/
    /// max where caller supplies them (e.g. RotateX = -180..180);
    /// world / scale coords get a wide ±1e9 fallback so the
    /// Stepper still works while the editable TextField is the
    /// primary input.
    private func screenLocField(_ key: String,
                                  fields: [String: Any],
                                  min: Double? = nil,
                                  max: Double? = nil,
                                  precision: Int = 2,
                                  step: Double = 0.5) -> some View {
        let v = (fields[key] as? NSNumber)?.doubleValue ?? 0.0
        let lo = min ?? -1_000_000_000
        let hi = max ??  1_000_000_000
        return LayoutEditorDoubleSpin(
            id: "\(modelName).\(key).\(token)",
            initial: v,
            range: lo...hi,
            step: step,
            precision: precision,
            commit: { newValue in commit(key, NSNumber(value: newValue)) }
        )
        .frame(maxWidth: 160, alignment: .trailing)
    }

    /// J-20.3 — Low Definition Factor as an editable spin button
    /// (TextField + Stepper). Range 1..100 mirrors the desktop's
    /// wxUIntProperty bounds.
    private var lowDefFactorField: some View {
        let v = (summary["lowDefinitionFactor"] as? NSNumber)?.intValue ?? 1
        return LayoutEditorIntSpin(
            id: "\(modelName).lowDefinitionFactor.\(token)",
            initial: v,
            range: 1...100,
            commit: { commit("lowDefinitionFactor", NSNumber(value: $0)) }
        )
        .frame(maxWidth: 150, alignment: .trailing)
    }

    /// J-20 — Shadow Model For. The empty-string sentinel means
    /// "not a shadow"; show it as "(none)" so the picker reads
    /// cleanly.
    private var shadowModelPicker: some View {
        let opts = (summary["shadowModelOptions"] as? [String]) ?? [""]
        let current = summary["shadowModelFor"] as? String ?? ""
        return Menu {
            ForEach(opts, id: \.self) { name in
                Button {
                    commit("shadowModelFor", name as NSString)
                } label: {
                    HStack {
                        Text(name.isEmpty ? "(none)" : name)
                        if name == current {
                            Spacer()
                            Image(systemName: "checkmark")
                        }
                    }
                }
            }
        } label: {
            Text(current.isEmpty ? "(none)" : current)
                .truncationMode(.middle)
                .frame(maxWidth: 180, alignment: .trailing)
        }
        .menuStyle(.button)
        .controlSize(.mini)
    }

    /// J-19 — String color for single-color string types. Uses a
    /// SwiftUI ColorPicker that round-trips through the bridge's
    /// hex setter.
    private var stringColorRow: some View {
        let hex = summary["stringColor"] as? String ?? "#FF0000"
        return ColorPicker("",
                            selection: Binding(
                                get: { hexColor(hex) },
                                set: { commit("stringColor", hexFromColor($0) as NSString) }
                            ),
                            supportsOpacity: false)
            .labelsHidden()
            .frame(width: 40, height: 24)
    }

    /// J-19 — RGBW Color Handling. Enabled only when the string
    /// type carries ≥4 channels (matches desktop's gate).
    private var rgbwHandlingPicker: some View {
        let opts = (summary["rgbwHandlingOptions"] as? [String]) ?? []
        let idx = (summary["rgbwHandlingIndex"] as? NSNumber)?.intValue ?? 0
        let enabled = (summary["rgbwHandlingEnabled"] as? Bool) ?? false
        return Menu {
            ForEach(Array(opts.enumerated()), id: \.offset) { i, label in
                Button {
                    commit("rgbwHandlingIndex", NSNumber(value: i))
                } label: {
                    HStack {
                        Text(label)
                        if i == idx {
                            Spacer()
                            Image(systemName: "checkmark")
                        }
                    }
                }
            }
        } label: {
            Text(idx >= 0 && idx < opts.count ? opts[idx] : "—")
                .truncationMode(.middle)
                .frame(maxWidth: 160, alignment: .trailing)
        }
        .menuStyle(.button)
        .controlSize(.mini)
        .disabled(!enabled)
        .opacity(enabled ? 1.0 : 0.5)
    }

    /// J-19 — Hex ↔ Color round-trip must use sRGB explicitly,
    /// NOT the device color space. iOS color pickers default to
    /// the device (extended-range Display P3 on modern iPads),
    /// which mangles RGB values the user typed as exact sRGB hex
    /// — e.g. #FF0000 round-trips as something slightly redder.
    /// Pinning to `Color(.sRGB, ...)` and reading the same way
    /// keeps the bits the user entered intact.
    private func hexColor(_ hex: String) -> Color {
        var s = hex
        if s.hasPrefix("#") { s.removeFirst() }
        guard let v = UInt32(s, radix: 16), s.count == 6 else {
            return Color(.sRGB, red: 1, green: 0, blue: 0, opacity: 1)
        }
        let r = Double((v >> 16) & 0xff) / 255.0
        let g = Double((v >> 8)  & 0xff) / 255.0
        let b = Double(v & 0xff) / 255.0
        return Color(.sRGB, red: r, green: g, blue: b, opacity: 1)
    }
    private func hexFromColor(_ c: Color) -> String {
        // Pull components in the sRGB color space explicitly —
        // UIColor(c).getRed in the device space distorts the hex.
        let cg = c.resolve(in: EnvironmentValues()).cgColor
        let srgb = CGColorSpace(name: CGColorSpace.sRGB)!
        guard let conv = cg.converted(to: srgb,
                                       intent: .defaultIntent,
                                       options: nil),
              let comps = conv.components,
              comps.count >= 3 else {
            return "#FF0000"
        }
        let ri = Int(round(comps[0] * 255))
        let gi = Int(round(comps[1] * 255))
        let bi = Int(round(comps[2] * 255))
        return String(format: "#%02X%02X%02X",
                      max(0, min(255, ri)),
                      max(0, min(255, gi)),
                      max(0, min(255, bi)))
    }

    /// J-19 — Controller picker. Mirrors the desktop's enum: the
    /// two sentinels ("Use Start Channel", "No Controller") sit
    /// at the top of the list, followed by every auto-layout
    /// controller name. Selecting "Use Start Channel" clears the
    /// model's controllerName (writes ""); the others round-trip
    /// verbatim.
    private var controllerField: some View {
        let opts = (summary["controllerOptions"] as? [String])
            ?? ["Use Start Channel", "No Controller"]
        let current = summary["controllerSelection"] as? String ?? "Use Start Channel"
        return Menu {
            ForEach(opts, id: \.self) { name in
                Button {
                    commit("controllerSelection", name as NSString)
                } label: {
                    HStack {
                        Text(name)
                        if name == current {
                            Spacer()
                            Image(systemName: "checkmark")
                        }
                    }
                }
            }
        } label: {
            Text(current)
                .truncationMode(.middle)
                .frame(maxWidth: 180, alignment: .trailing)
        }
        .menuStyle(.button)
        .controlSize(.mini)
    }

    /// J-18 — model-wide Start Channel field. Accepts the same
    /// notation desktop does ("1", ">Model1:1", "@ip:univ:ch",
    /// etc.). J-19 — read-only when the Controller picker is set
    /// to anything other than "Use Start Channel". J-20 — pencil
    /// button opens the structured `StartChannelEditorSheet`
    /// (the iPad equivalent of desktop's StartChannelDialog) so
    /// the user doesn't have to memorise the wire format.
    private var modelStartChannelField: some View {
        let editable = (summary["startChannelEditable"] as? Bool) ?? true
        let current = summary["modelStartChannel"] as? String ?? ""
        return HStack(spacing: 6) {
            if editable {
                LayoutEditorStringField(
                    id: "\(modelName).modelStartChannel.\(token)",
                    initial: current,
                    commit: { commit("modelStartChannel", $0 as NSString) }
                )
                .frame(maxWidth: 110, alignment: .trailing)
                Button {
                    pendingStartChannelKey = "modelStartChannel"
                    pendingStartChannelInitial = current
                    startChannelEditorVisible = true
                } label: {
                    Image(systemName: "pencil")
                        .font(.caption2)
                }
                .buttonStyle(.plain)
                .foregroundStyle(.secondary)
                .accessibilityLabel("Pick start channel format")
            } else {
                Text(current.isEmpty ? "—" : current)
                    .foregroundStyle(.secondary)
                    .lineLimit(1)
                    .truncationMode(.middle)
                    .frame(maxWidth: 140, alignment: .trailing)
            }
        }
    }

    /// J-18 — Per-string start channel. Re-used by ForEach when
    /// Indiv Start Chans is ON. The `key` carries the string
    /// index so the bridge can route to the right slot. Read-only
    /// when the controller drives the assignment (same gate as the
    /// model-wide Start Channel).
    private func startChannelField(key: String, initial: String) -> some View {
        let editable = (summary["startChannelEditable"] as? Bool) ?? true
        return Group {
            if editable {
                LayoutEditorStringField(
                    id: "\(modelName).\(key).\(token)",
                    initial: initial,
                    commit: { commit(key, $0 as NSString) }
                )
                .frame(maxWidth: 140, alignment: .trailing)
            } else {
                Text(initial.isEmpty ? "—" : initial)
                    .foregroundStyle(.secondary)
                    .lineLimit(1)
                    .truncationMode(.middle)
                    .frame(maxWidth: 140, alignment: .trailing)
            }
        }
    }

    /// J-18 — Model Chain picker. Lists other models on the same
    /// controller + protocol + port, plus the "Beginning"
    /// sentinel. Hidden when the controller isn't fully set up.
    private var modelChainPicker: some View {
        let opts = (summary["modelChainOptions"] as? [String]) ?? ["Beginning"]
        let current = summary["modelChain"] as? String ?? "Beginning"
        return Menu {
            ForEach(opts, id: \.self) { name in
                Button {
                    commit("modelChain", name as NSString)
                } label: {
                    HStack {
                        Text(name)
                        if name == current {
                            Spacer()
                            Image(systemName: "checkmark")
                        }
                    }
                }
            }
        } label: {
            Text(current)
                .truncationMode(.middle)
                .frame(maxWidth: 160, alignment: .trailing)
        }
        .menuStyle(.button)
        .controlSize(.mini)
    }

    // MARK: - Model Data (read-only popup summaries)

    private var extras: [String: Any] {
        summary["extras"] as? [String: Any] ?? [:]
    }
    private var hasDimmingCurve: Bool {
        (extras["hasDimmingCurve"] as? Bool) ?? false
    }

    /// J-20.2 — list of iPad-bridge keys the current model type's
    /// adapter wants disabled. SwiftUI renders the matching rows
    /// grayed-out / non-interactive. Matches desktop's
    /// DisableUnusedProperties.
    private var disabledKeys: Set<String> {
        Set((summary["disabledKeys"] as? [String]) ?? [])
    }
    private func isDisabled(_ key: String) -> Bool { disabledKeys.contains(key) }
    private func isDisabled(_ kind: ModelDataKind) -> Bool {
        switch kind {
        case .submodels: return disabledKeys.contains("submodels")
        case .faces:     return disabledKeys.contains("faces")
        case .states:    return disabledKeys.contains("states")
        case .strands:   return disabledKeys.contains("strands")
        case .nodes:     return disabledKeys.contains("nodes")
        default:         return false
        }
    }

    @ViewBuilder
    private func modelDataRow(kind: ModelDataKind) -> some View {
        let list = entries(for: kind)
        // Editable categories show a pencil even when empty so the
        // user can add the first entry. Read-only categories only
        // expose the bullet icon when there's something to view.
        let editable = kind.isEditable
        let disabled = isDisabled(kind)
        row(kind.title) {
            HStack(spacing: 6) {
                Text(list.isEmpty ? "—" : "\(list.count)")
                    .foregroundStyle(.secondary)
                if !disabled && (editable || !list.isEmpty) {
                    Button {
                        modelDataViewer = kind
                    } label: {
                        Image(systemName: editable ? "pencil" : "list.bullet")
                            .font(.caption2)
                    }
                    .buttonStyle(.plain)
                    .foregroundStyle(.secondary)
                    .accessibilityLabel("\(editable ? "Edit" : "View") \(kind.title)")
                }
            }
        }
        .opacity(disabled ? 0.45 : 1.0)
    }

    /// J-20 — Start-channel picker's "From Model" / "@Model"
    /// options. Pulled from the modelChainOptions list (every
    /// other model on the same controller chain) plus any model
    /// from the per-summary `allModelNames` slot if exposed.
    /// Today we use modelChainOptions as the proxy; it filters out
    /// the current model and ModelGroups, which is what desktop's
    /// StartChannelDialog does too.
    private var otherModelNames: [String] {
        if let chain = summary["modelChainOptions"] as? [String], chain.count > 1 {
            return chain.filter { $0 != "Beginning" }
        }
        return []
    }

    private func entries(for kind: ModelDataKind) -> [String] {
        switch kind {
        case .submodels: return (extras["submodelNames"] as? [String]) ?? []
        case .faces:     return (extras["faceNames"]     as? [String]) ?? []
        case .states:    return (extras["stateNames"]    as? [String]) ?? []
        case .aliases:   return (extras["aliasNames"]    as? [String]) ?? []
        case .strands:   return (extras["strandNames"]   as? [String]) ?? []
        case .nodes:     return (extras["nodeNames"]     as? [String]) ?? []
        case .groups:    return (extras["inModelGroups"] as? [String]) ?? []
        }
    }

    // MARK: - Per-type row dispatcher

    @ViewBuilder
    private func typeDescriptorRow(_ d: [String: Any]) -> some View {
        let key = d["key"] as? String ?? ""
        let label = d["label"] as? String ?? key
        let kind = d["kind"] as? String ?? ""
        let enabled = (d["enabled"] as? Bool) ?? true
        HStack(alignment: .firstTextBaseline) {
            Text(label)
                .foregroundStyle(.secondary)
                .frame(minWidth: 110, alignment: .leading)
            Spacer(minLength: 8)
            typeDescriptorControl(kind: kind, key: key, d: d)
                .disabled(!enabled)
                .opacity(enabled ? 1.0 : 0.5)
                .lineLimit(1)
        }
    }

    @ViewBuilder
    private func typeDescriptorControl(kind: String, key: String, d: [String: Any]) -> some View {
        switch kind {
        case "int":
            let v = (d["value"] as? NSNumber)?.doubleValue ?? 0
            let minV = (d["min"] as? NSNumber)?.doubleValue
            let maxV = (d["max"] as? NSNumber)?.doubleValue
            // J-20.5 — always use the spin widget for ints. The
            // text field handles big jumps (Nodes/String can be
            // 1..10000, user types the value); the +/- buttons
            // handle small nudges. Mirrors desktop's universal
            // wxSpinCtrl on `wxUIntProperty`.
            let lo = Int((minV ?? 0).rounded())
            let hi = Int((maxV ?? Double(Int32.max)).rounded())
            LayoutEditorIntSpin(
                id: "\(modelName).\(key).\(token)",
                initial: Int(v.rounded()),
                range: lo...hi,
                commit: { typeCommit(key, NSNumber(value: $0)) }
            )
            .frame(maxWidth: 150, alignment: .trailing)
        case "double":
            let v = (d["value"] as? NSNumber)?.doubleValue ?? 0
            let minV = (d["min"] as? NSNumber)?.doubleValue
            let maxV = (d["max"] as? NSNumber)?.doubleValue
            let stepV = (d["step"] as? NSNumber)?.doubleValue ?? 0.1
            let precision = (d["precision"] as? NSNumber)?.intValue ?? 2
            // J-20.5 — always use the spin widget for doubles
            // when the descriptor provides a finite range.
            // Without a range we fall back to a plain numeric
            // field so the SwiftUI Stepper doesn't choke on
            // unbounded inputs.
            if let lo = minV, let hi = maxV, stepV > 0 {
                LayoutEditorDoubleSpin(
                    id: "\(modelName).\(key).\(token)",
                    initial: v, range: lo...hi, step: stepV, precision: precision,
                    commit: { typeCommit(key, NSNumber(value: $0)) }
                )
                .frame(maxWidth: 160, alignment: .trailing)
            } else {
                LayoutEditorDoubleField(
                    id: "\(modelName).\(key).\(token)",
                    initial: v, min: minV, max: maxV, precision: precision,
                    commit: { newValue in typeCommit(key, NSNumber(value: newValue)) }
                )
                .frame(maxWidth: 110, alignment: .trailing)
            }
        case "bool":
            let v = (d["value"] as? Bool) ?? false
            Toggle("", isOn: Binding(
                get: { v },
                set: { typeCommit(key, NSNumber(value: $0)) }
            ))
            .labelsHidden()
            .controlSize(.mini)
        case "enum":
            let idx = (d["value"] as? NSNumber)?.intValue ?? 0
            let opts = (d["options"] as? [String]) ?? []
            Menu {
                ForEach(Array(opts.enumerated()), id: \.offset) { i, label in
                    Button {
                        typeCommit(key, NSNumber(value: i))
                    } label: {
                        HStack {
                            Text(label)
                            if i == idx {
                                Spacer()
                                Image(systemName: "checkmark")
                            }
                        }
                    }
                }
            } label: {
                Text(idx >= 0 && idx < opts.count ? opts[idx] : "—")
                    .truncationMode(.middle)
                    .frame(maxWidth: 160, alignment: .trailing)
            }
            .menuStyle(.button)
            .controlSize(.mini)
        case "string":
            let v = d["value"] as? String ?? ""
            LayoutEditorStringField(
                id: "\(modelName).\(key).\(token)",
                initial: v,
                commit: { typeCommit(key, $0 as NSString) }
            )
            .frame(maxWidth: 160, alignment: .trailing)
        case "color":
            // J-20 — per-type Color row. Mirrors stringColorRow's
            // sRGB-pinned round-trip so the user's hex stays
            // bit-exact through the picker.
            let hex = d["value"] as? String ?? "#FFFFFF"
            ColorPicker("",
                         selection: Binding(
                            get: { hexColor(hex) },
                            set: { typeCommit(key, hexFromColor($0) as NSString) }
                         ),
                         supportsOpacity: false)
                .labelsHidden()
                .frame(width: 40, height: 24)
        case "customModelData":
            // J-23 — opens the visual grid editor (replaces
            // desktop's type-numbers-into-cells dialog).
            Button {
                onOpenCustomModelEditor()
            } label: {
                Label("Edit Grid…", systemImage: "square.grid.3x3.fill")
                    .font(.caption2)
            }
            .buttonStyle(.bordered)
            .controlSize(.mini)
        case "imageFile":
            // J-20.2 — path label + folder button. Tapping the
            // button opens a .fileImporter scoped to image UTTypes
            // and bubbles the picked path up to the parent so the
            // shared file-picker plumbing handles security scope.
            let path = d["value"] as? String ?? ""
            let display = path.isEmpty ? "—" : (path as NSString).lastPathComponent
            HStack(spacing: 6) {
                Text(display)
                    .foregroundStyle(.secondary)
                    .lineLimit(1)
                    .truncationMode(.middle)
                    .frame(maxWidth: 110, alignment: .trailing)
                Button {
                    onPickImageFile(key)
                } label: {
                    Image(systemName: "folder")
                        .font(.caption2)
                }
                .buttonStyle(.plain)
                .foregroundStyle(.secondary)
                .accessibilityLabel("Pick image file")
                if !path.isEmpty {
                    Button(role: .destructive) {
                        typeCommit(key, "" as NSString)
                    } label: {
                        Image(systemName: "xmark.circle")
                            .font(.caption2)
                    }
                    .buttonStyle(.plain)
                    .foregroundStyle(.secondary)
                    .accessibilityLabel("Clear image")
                }
            }
        case "layerSizes":
            // J-20.4/7 — NSArray<NSNumber*> bridging is unreliable
            // when the value comes through `[String: Any]`; route
            // via NSArray + compactMap. Sheet uses .sheet(item:)
            // so the payload is captured at present time and the
            // sheet's @State always initialises from the right
            // data.
            let sizes = layerSizesFromDescriptor(d)
            HStack(spacing: 6) {
                Text(layerSizesLabel(sizes))
                    .foregroundStyle(.secondary)
                    .truncationMode(.middle)
                    .frame(maxWidth: 130, alignment: .trailing)
                Button {
                    layerSizesEditorPayload = LayerSizesPayload(sizes: sizes)
                } label: {
                    Image(systemName: "pencil")
                        .font(.caption2)
                }
                .buttonStyle(.plain)
                .foregroundStyle(.secondary)
                .accessibilityLabel("Edit layer sizes")
            }
        default:
            Text("?").foregroundStyle(.tertiary)
        }
    }

    private func layerSizesFromDescriptor(_ d: [String: Any]) -> [Int] {
        if let arr = d["value"] as? NSArray {
            return arr.compactMap { ($0 as? NSNumber)?.intValue }
        }
        if let arr = d["value"] as? [Any] {
            return arr.compactMap { ($0 as? NSNumber)?.intValue ?? ($0 as? Int) }
        }
        return []
    }

    private func layerSizesLabel(_ sizes: [Int]) -> String {
        if sizes.isEmpty { return "—" }
        return sizes.map { "\($0)" }.joined(separator: ", ")
    }

    // MARK: - Lookups

    private func doubleVal(_ key: String) -> Double {
        (summary[key] as? NSNumber)?.doubleValue ?? 0.0
    }

    private func intVal(_ key: String) -> Int {
        (summary[key] as? NSNumber)?.intValue ?? 0
    }

    private func uintVal(_ key: String) -> UInt64 {
        (summary[key] as? NSNumber)?.uint64Value ?? 0
    }
}

/// J-18 — read-only popup category. Each case identifies one of
/// desktop's popup-button categories (Faces, States, …) so a
/// single sheet can render whichever list the user tapped.
private enum ModelDataKind: String, Identifiable {
    case submodels, faces, states, aliases, strands, nodes, groups
    var id: String { rawValue }
    var title: String {
        switch self {
        case .submodels: return "SubModels"
        case .faces:     return "Faces"
        case .states:    return "States"
        case .aliases:   return "Aliases"
        case .strands:   return "Strand Names"
        case .nodes:     return "Node Names"
        case .groups:    return "In Model Groups"
        }
    }
    /// J-18 / J-22 — categories with a dedicated bridge writer
    /// open the editable sheet; the rest fall back to the read-
    /// only viewer.
    var isEditable: Bool {
        switch self {
        case .aliases, .strands, .nodes, .submodels,
             .faces, .states:
            return true
        default:
            return false
        }
    }
}

/// J-18 — generic read-only list sheet for popup-style model
/// data. Editing comes in a later pass; the sheet only needs to
/// render the strings and offer a Done button.
private struct ModelDataViewerSheet: View {
    let title: String
    let entries: [String]
    @Environment(\.dismiss) private var dismiss

    var body: some View {
        NavigationStack {
            Group {
                if entries.isEmpty {
                    ContentUnavailableView("Nothing defined",
                        systemImage: "list.bullet",
                        description: Text("This model has no \(title.lowercased()) yet."))
                } else {
                    List {
                        ForEach(Array(entries.enumerated()), id: \.offset) { _, e in
                            Text(e).font(.body.monospaced())
                        }
                    }
                }
            }
            .navigationTitle(title)
            .toolbar {
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") { dismiss() }
                }
            }
        }
        .presentationDetents([.medium, .large])
    }
}

/// J-18 pass 2 — edit a model's alias list. Aliases are alternate
/// names a model also responds to during sequence import. Stored
/// lowercase; the bridge re-applies that normalization on Save,
/// so what the user types is just a hint.
private struct AliasEditorSheet: View {
    let modelName: String
    let initial: [String]
    let commit: (_ aliases: [String]) -> Void

    @State private var aliases: [String] = []
    @State private var draft: String = ""
    @Environment(\.dismiss) private var dismiss

    var body: some View {
        NavigationStack {
            Form {
                Section("Add alias") {
                    HStack {
                        TextField("e.g. snowflake-top-left", text: $draft)
                            .textInputAutocapitalization(.never)
                            .autocorrectionDisabled(true)
                            .submitLabel(.done)
                            .onSubmit(addDraft)
                        Button("Add", action: addDraft)
                            .disabled(normalizedDraft.isEmpty || isDuplicate(normalizedDraft))
                    }
                    if !draft.isEmpty {
                        Text("Will save as '\(normalizedDraft)'")
                            .font(.caption2)
                            .foregroundStyle(.secondary)
                    }
                }
                Section(aliases.isEmpty ? "No aliases yet" : "Aliases (\(aliases.count))") {
                    if aliases.isEmpty {
                        Text("This model has no aliases.")
                            .font(.caption)
                            .foregroundStyle(.secondary)
                    } else {
                        ForEach(aliases, id: \.self) { a in
                            Text(a).font(.body.monospaced())
                        }
                        .onDelete { idx in
                            aliases.remove(atOffsets: idx)
                        }
                    }
                }
            }
            .navigationTitle("Aliases — \(modelName)")
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Save") {
                        commit(aliases)
                        dismiss()
                    }
                }
            }
        }
        .presentationDetents([.medium, .large])
        .onAppear {
            aliases = initial
        }
    }

    private var normalizedDraft: String {
        draft.trimmingCharacters(in: .whitespacesAndNewlines).lowercased()
    }

    private func isDuplicate(_ candidate: String) -> Bool {
        // A model can't alias to its own (lowercased) name —
        // matches the AddAlias guard.
        if candidate == modelName.lowercased() { return true }
        return aliases.contains(candidate)
    }

    private func addDraft() {
        let n = normalizedDraft
        guard !n.isEmpty, !isDuplicate(n) else { return }
        aliases.append(n)
        draft = ""
    }
}

/// J-18 pass 3 — edit per-index strand or node names. Slot count
/// is fixed (= GetNumStrands or GetNodeCount on the source model)
/// so this is a renamer, not a list manager. Empty slots are
/// kept; commas are stripped by the bridge before joining.
private struct IndexedNamesEditorSheet: View {
    let modelName: String
    let title: String
    let placeholderFor: (Int) -> String
    let initial: [String]
    let commit: (_ names: [String]) -> Void

    @State private var names: [String] = []
    @Environment(\.dismiss) private var dismiss

    var body: some View {
        NavigationStack {
            Group {
                if names.isEmpty {
                    ContentUnavailableView("Nothing to name",
                        systemImage: "list.number",
                        description: Text("This model has no \(title.lowercased())."))
                } else {
                    List {
                        ForEach(Array(names.enumerated()), id: \.offset) { idx, _ in
                            HStack {
                                Text("\(idx + 1)")
                                    .frame(minWidth: 28, alignment: .trailing)
                                    .foregroundStyle(.secondary)
                                    .font(.caption.monospacedDigit())
                                TextField(placeholderFor(idx),
                                          text: Binding(
                                            get: { names[idx] },
                                            set: { names[idx] = $0 }
                                          ))
                                    .textInputAutocapitalization(.never)
                                    .autocorrectionDisabled(true)
                            }
                        }
                    }
                }
            }
            .navigationTitle("\(title) — \(modelName)")
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Save") {
                        commit(names)
                        dismiss()
                    }
                }
            }
        }
        .presentationDetents([.medium, .large])
        .onAppear {
            names = initial
        }
    }
}

/// J-18 pass 4 — list this model's submodels with swipe-to-delete.
/// Add / rename / geometry editing are not in scope (each one
/// needs its own slice — submodels carry per-instance range or
/// line geometry that the iPad has no UI for yet).
/// J-23.3 — Editable SubModel record. Mirrors the desktop's
/// SubModelInfo struct: each field maps directly to one of
/// SubModel's constructor args + the ranges / sub-buffer
/// strings we recreate after `RemoveAllSubModels`.
struct SubModelEntry: Identifiable, Equatable {
    var id = UUID()
    var name: String
    var isRanges: Bool
    var isVertical: Bool
    var bufferStyle: String
    var strands: [String]      // when isRanges
    var subBuffer: String      // when !isRanges
}

private struct SubModelListSheet: View {
    let modelName: String
    let initial: [String]
    let delete: (_ submodelName: String) -> Void
    let add: (_ submodelName: String) -> String?
    let rename: (_ oldName: String, _ newName: String) -> Bool
    let loadDetails: () -> [SubModelEntry]
    let commitDetails: (_ entries: [SubModelEntry]) -> Bool

    @State private var names: [String]
    @State private var entries: [SubModelEntry] = []
    @State private var newSubName: String = ""
    @Environment(\.dismiss) private var dismiss

    init(modelName: String,
         initial: [String],
         delete: @escaping (String) -> Void,
         add: @escaping (String) -> String?,
         rename: @escaping (String, String) -> Bool,
         loadDetails: @escaping () -> [SubModelEntry],
         commitDetails: @escaping ([SubModelEntry]) -> Bool) {
        self.modelName = modelName
        self.initial = initial
        self.delete = delete
        self.add = add
        self.rename = rename
        self.loadDetails = loadDetails
        self.commitDetails = commitDetails
        self._names = State(initialValue: initial)
    }

    var body: some View {
        NavigationStack {
            List {
                Section("Add") {
                    HStack {
                        TextField("New submodel name", text: $newSubName)
                            .textInputAutocapitalization(.never)
                            .autocorrectionDisabled(true)
                            .submitLabel(.done)
                            .onSubmit(addPressed)
                        Button("Add", action: addPressed)
                            .disabled(normalizedNewName.isEmpty ||
                                      names.contains(normalizedNewName))
                    }
                }
                Section(footer: Text("Tap a submodel to edit its name + geometry (ranges, lines, sub-buffer). Swipe to delete.")
                            .font(.caption2)
                            .foregroundStyle(.secondary)) {
                    if names.isEmpty {
                        Text("No submodels yet.")
                            .font(.caption)
                            .foregroundStyle(.secondary)
                    } else {
                        ForEach(names, id: \.self) { sub in
                            NavigationLink(value: sub) {
                                HStack {
                                    Text(sub).font(.body.monospaced())
                                    Spacer()
                                    let entry = entryFor(sub)
                                    Text(entry?.isRanges ?? true
                                         ? "ranges (\(entry?.strands.count ?? 0))"
                                         : "sub-buffer")
                                        .font(.caption2)
                                        .foregroundStyle(.secondary)
                                }
                            }
                        }
                        .onDelete { idx in
                            for i in idx {
                                let sub = names[i]
                                delete(sub)
                                entries.removeAll { $0.name == sub }
                            }
                            names.remove(atOffsets: idx)
                        }
                    }
                }
            }
            .navigationDestination(for: String.self) { sub in
                if let idx = entries.firstIndex(where: { $0.name == sub }) {
                    SubModelDetailEditor(
                        entry: Binding(
                            get: { entries[idx] },
                            set: { entries[idx] = $0 }
                        ),
                        existingNames: Set(entries.filter { $0.name != sub }.map { $0.name }),
                        onSave: {
                            let oldName = sub
                            let newName = entries[idx].name
                            _ = commitDetails(entries)
                            // Re-sync names list in case the
                            // detail renamed the submodel or
                            // re-keyed its identity.
                            if oldName != newName,
                               let ni = names.firstIndex(of: oldName) {
                                names[ni] = newName
                            }
                            entries = loadDetails()
                        })
                }
            }
            .navigationTitle("SubModels — \(modelName)")
            .toolbar {
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") { dismiss() }
                }
            }
        }
        .presentationDetents([.large])
        .onAppear {
            entries = loadDetails()
        }
    }

    private func entryFor(_ name: String) -> SubModelEntry? {
        entries.first(where: { $0.name == name })
    }

    private var normalizedNewName: String {
        newSubName.trimmingCharacters(in: .whitespacesAndNewlines)
    }

    private func addPressed() {
        let n = normalizedNewName
        guard !n.isEmpty, !names.contains(n) else { return }
        if let added = add(n) {
            names.append(added)
            entries = loadDetails()
        }
        newSubName = ""
    }
}

/// J-23.3 — Per-submodel detail editor. Edits all fields
/// (name / type / orientation / buffer style / ranges OR
/// sub-buffer) of one submodel. Saving commits via the
/// wholesale-replace bridge, since changing type / orientation
/// / buffer style requires reconstructing the C++ SubModel.
private struct SubModelDetailEditor: View {
    @Binding var entry: SubModelEntry
    let existingNames: Set<String>
    let onSave: () -> Void

    @State private var newRange: String = ""

    private static let bufferStyles = [
        "Default", "Keep XY", "Stacked", "Stacked Right",
        "Stacked Left", "Stacked Up", "Stacked Down",
        "Stacked Vertical Concatenate",
    ]

    var body: some View {
        Form {
            Section("Identity") {
                TextField("Name", text: $entry.name)
                    .textInputAutocapitalization(.never)
                    .autocorrectionDisabled(true)
                if existingNames.contains(entry.name) {
                    Text("⚠ Name already used by another submodel")
                        .font(.caption)
                        .foregroundStyle(.red)
                }
            }
            Section("Type") {
                Picker("Type", selection: $entry.isRanges) {
                    Text("Ranges / Lines").tag(true)
                    Text("Sub-buffer").tag(false)
                }
                .pickerStyle(.segmented)
                if entry.isRanges {
                    Toggle("Vertical orientation", isOn: $entry.isVertical)
                    Picker("Buffer Style", selection: $entry.bufferStyle) {
                        ForEach(Self.bufferStyles, id: \.self) { s in
                            Text(s).tag(s)
                        }
                    }
                    .pickerStyle(.menu)
                }
            }
            if entry.isRanges {
                Section(footer: Text("One range per line, e.g. \"1-50\" or \"5,10-15,30\". Swipe to delete; tap Add to append. Range numbers refer to pixel indices on the parent model.")
                            .font(.caption2)
                            .foregroundStyle(.secondary)) {
                    HStack {
                        TextField("e.g. 1-50", text: $newRange)
                            .textInputAutocapitalization(.never)
                            .autocorrectionDisabled(true)
                            .submitLabel(.done)
                            .onSubmit(addRange)
                        Button("Add", action: addRange)
                            .disabled(newRange.trimmingCharacters(in: .whitespaces).isEmpty)
                    }
                    if entry.strands.isEmpty {
                        Text("No ranges yet.")
                            .font(.caption)
                            .foregroundStyle(.secondary)
                    } else {
                        ForEach(Array(entry.strands.enumerated()), id: \.offset) { idx, _ in
                            HStack {
                                Text("Line \(idx + 1)")
                                    .frame(minWidth: 60, alignment: .leading)
                                    .foregroundStyle(.secondary)
                                    .font(.caption.monospacedDigit())
                                TextField("range",
                                          text: Binding(
                                            get: { entry.strands[idx] },
                                            set: { entry.strands[idx] = $0 }
                                          ))
                                    .textInputAutocapitalization(.never)
                                    .autocorrectionDisabled(true)
                                    .multilineTextAlignment(.trailing)
                            }
                        }
                        .onDelete { idx in
                            entry.strands.remove(atOffsets: idx)
                        }
                    }
                }
            } else {
                Section(footer: Text("Format: x1,y1,x2,y2 as percentages (0..100) of the parent model bounds.")
                            .font(.caption2)
                            .foregroundStyle(.secondary)) {
                    TextField("e.g. 0,0,100,100", text: $entry.subBuffer)
                        .textInputAutocapitalization(.never)
                        .autocorrectionDisabled(true)
                }
            }
        }
        .navigationTitle(entry.name)
        .toolbar {
            ToolbarItem(placement: .confirmationAction) {
                Button("Save") { onSave() }
                    .disabled(entry.name.trimmingCharacters(in: .whitespaces).isEmpty ||
                              existingNames.contains(entry.name))
            }
        }
    }

    private func addRange() {
        let r = newRange.trimmingCharacters(in: .whitespaces)
        guard !r.isEmpty else { return }
        entry.strands.append(r)
        newRange = ""
    }
}

/// J-20 — structured Start Channel editor. Equivalent of
/// desktop's StartChannelDialog. Parses the existing value into
/// a `Mode + fields` representation; recomposes on save. Modes:
///   - .none      → raw channel number, e.g. "100"
///   - .universe  → "#<universe>:<channel>" or "#<ip>:<univ>:<ch>"
///   - .fromModel → ">ModelName:N" (chained relative to a model)
///   - .startOf   → "@ModelName:N" (from start of a model)
///   - .controller→ "!ControllerName:N"
private struct StartChannelEditorSheet: View {
    let initial: String
    let modelOptions: [String]            // unused — kept for future fanout
    let controllerOptions: [String]
    let otherModelOptions: [String]
    let commit: (_ value: String) -> Void

    enum Mode: String, CaseIterable, Identifiable {
        case noneFmt = "Channel #"
        case universe = "Universe"
        case fromModel = "From Model"
        case startOf = "Start Of Model"
        case controller = "Controller"
        var id: String { rawValue }
    }

    @State private var mode: Mode = .noneFmt
    @State private var channel: Int = 1
    @State private var universe: Int = 1
    @State private var ip: String = "ANY"
    @State private var pickedModel: String = ""
    @State private var pickedController: String = ""
    @Environment(\.dismiss) private var dismiss

    var body: some View {
        NavigationStack {
            Form {
                Section("Format") {
                    Picker("Mode", selection: $mode) {
                        ForEach(Mode.allCases) { m in
                            Text(m.rawValue).tag(m)
                        }
                    }
                    .pickerStyle(.menu)
                }
                Section("Channel") {
                    HStack {
                        Text("Channel")
                        Spacer()
                        // J-20.3 — spin-button so the user can both
                        // type a value and bump it ±1. Wide range
                        // (1..1e8) keeps the field-style display
                        // but the +/- is still useful for fine
                        // adjustments.
                        LayoutEditorIntSpin(
                            id: "sc.channel",
                            initial: channel,
                            range: 1...100_000_000,
                            commit: { channel = $0 }
                        )
                        .frame(maxWidth: 160, alignment: .trailing)
                    }
                }
                switch mode {
                case .universe:
                    Section("Universe") {
                        HStack {
                            Text("Universe")
                            Spacer()
                            LayoutEditorIntSpin(
                                id: "sc.universe",
                                initial: universe,
                                range: 1...64_000,
                                commit: { universe = $0 }
                            )
                            .frame(maxWidth: 160, alignment: .trailing)
                        }
                        TextField("IP (ANY for any)", text: $ip)
                            .textInputAutocapitalization(.never)
                            .autocorrectionDisabled(true)
                    }
                case .fromModel, .startOf:
                    Section(mode == .fromModel ? "Reference model" : "Start of model") {
                        if otherModelOptions.isEmpty {
                            Text("No other models on the same controller chain.")
                                .font(.caption)
                                .foregroundStyle(.secondary)
                        } else {
                            Picker("Model", selection: $pickedModel) {
                                ForEach(otherModelOptions, id: \.self) { Text($0).tag($0) }
                            }
                            .pickerStyle(.menu)
                        }
                    }
                case .controller:
                    Section("Controller") {
                        if controllerOptions.isEmpty {
                            Text("No auto-layout controllers configured.")
                                .font(.caption)
                                .foregroundStyle(.secondary)
                        } else {
                            Picker("Controller", selection: $pickedController) {
                                ForEach(controllerOptions, id: \.self) { Text($0).tag($0) }
                            }
                            .pickerStyle(.menu)
                        }
                    }
                case .noneFmt:
                    EmptyView()
                }
                Section("Preview") {
                    Text(composedValue.isEmpty ? "—" : composedValue)
                        .font(.body.monospaced())
                        .foregroundStyle(.secondary)
                }
            }
            .navigationTitle("Start Channel")
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Save") {
                        commit(composedValue)
                        dismiss()
                    }
                    .disabled(composedValue.isEmpty)
                }
            }
        }
        .presentationDetents([.medium, .large])
        .onAppear { parseInitial() }
    }

    private var composedValue: String {
        switch mode {
        case .noneFmt:
            return "\(channel)"
        case .universe:
            let trimmedIP = ip.trimmingCharacters(in: .whitespaces)
            if trimmedIP.isEmpty || trimmedIP.uppercased() == "ANY" {
                return "#\(universe):\(channel)"
            }
            return "#\(trimmedIP):\(universe):\(channel)"
        case .fromModel:
            return pickedModel.isEmpty ? "" : ">\(pickedModel):\(channel)"
        case .startOf:
            return pickedModel.isEmpty ? "" : "@\(pickedModel):\(channel)"
        case .controller:
            return pickedController.isEmpty ? "" : "!\(pickedController):\(channel)"
        }
    }

    private func parseInitial() {
        let s = initial.trimmingCharacters(in: .whitespaces)
        if s.isEmpty {
            mode = .noneFmt
            return
        }
        if s.hasPrefix("#") {
            mode = .universe
            let body = String(s.dropFirst())
            let parts = body.split(separator: ":").map(String.init)
            if parts.count == 2 {
                universe = Int(parts[0]) ?? 1
                channel  = Int(parts[1]) ?? 1
                ip = "ANY"
            } else if parts.count == 3 {
                ip = parts[0]
                universe = Int(parts[1]) ?? 1
                channel  = Int(parts[2]) ?? 1
            }
            return
        }
        if s.hasPrefix(">") || s.hasPrefix("@") {
            mode = s.hasPrefix(">") ? .fromModel : .startOf
            let body = String(s.dropFirst())
            // Split on the LAST colon — model names can contain
            // colons (legal in submodel paths).
            if let i = body.lastIndex(of: ":") {
                pickedModel = String(body[..<i])
                channel     = Int(body[body.index(after: i)...]) ?? 1
            } else {
                pickedModel = body
                channel = 1
            }
            return
        }
        if s.hasPrefix("!") {
            mode = .controller
            let body = String(s.dropFirst())
            if let i = body.lastIndex(of: ":") {
                pickedController = String(body[..<i])
                channel          = Int(body[body.index(after: i)...]) ?? 1
            } else {
                pickedController = body
                channel = 1
            }
            return
        }
        // Plain integer (None mode)
        if let v = Int(s) {
            mode = .noneFmt
            channel = v
        }
    }
}

/// J-19 — per-layer size editor for Layered Arches (and any
/// other model that uses the same wholesale-replace LayerSizes
/// pipeline in the future). Slot count is variable — Add adds a
/// new size-1 layer, swipe-to-delete removes a layer. Each row
/// has an int field for the layer's node count.
/// J-23 — Custom-model visual editor. Replaces desktop's
/// type-numbers-into-cells dialog with a point/click/drag flow:
///   - Set the grid size (Width × Height; Depth gates additional
///     layers but defaults to 1 for the common 2D case).
///   - Optionally pick a background image.
///   - Tap an empty cell to place the next pixel; tap an
///     existing pixel to select it.
///   - Selected pixel: a chip shows its number; the user can
///     drag it to a new cell, change the number (with
///     auto-shift of neighbours), or delete it.
///
/// The grid is rendered as a SwiftUI Canvas so we can draw the
/// background image, grid lines, numbered points, and selection
/// highlight in a single pass without spawning a UIView tree
/// per cell — important for grids with thousands of cells.
private struct CustomModelPayload: Identifiable {
    let id = UUID()
    let modelName: String
    let initialWidth: Int
    let initialHeight: Int
    let initialDepth: Int
    let initialLocations: [[[Int]]]
    let initialBackground: String
    let initialBkgScale: Int
    let initialBkgBrightness: Int
}

private struct CustomModelEditorSheet: View {
    let payload: CustomModelPayload
    let commit: (_ w: Int, _ h: Int, _ d: Int, _ locations: [[[Int]]]) -> Void

    // Grid state
    @State private var w: Int
    @State private var h: Int
    @State private var d: Int
    @State private var layer: Int = 0
    /// 3D grid indexed [d][h][w]. Each cell is 0 (empty) or the
    /// 1-based pixel number that lights at that position.
    @State private var grid: [[[Int]]]
    /// Currently selected cell (row, col) in the active layer.
    @State private var selection: GridPos? = nil
    /// J-23.2 — lazily-loaded background image (UIImage so we
    /// can hand a CGImage to the Canvas). Loaded from the bg
    /// path on first body evaluation.
    @State private var backgroundImage: UIImage? = nil

    /// J-23.4 — canvas zoom. 1.0 = fit cellSide to available
    /// space; >1 zooms in (cells get bigger, possibly clipped at
    /// the canvas edges). Pinch updates this live, plus a slider
    /// in the toolbar for explicit control.
    @State private var zoom: CGFloat = 1.0
    /// Accumulator while a MagnificationGesture is in flight.
    @State private var zoomCommitted: CGFloat = 1.0
    /// Pan offset in screen points. Allows scrolling the zoomed
    /// canvas. Reset when zoom returns to 1.0.
    @State private var panOffset: CGSize = .zero
    @State private var panCommitted: CGSize = .zero
    /// True when a multi-touch gesture is active so the single-
    /// finger DragGesture suppresses its tap/move side effects.
    @State private var multiTouchActive: Bool = false
    /// J-23.5 — Live canvas size captured by GeometryReader so
    /// the gesture closures can re-derive cellSize / xOff / yOff
    /// from fresh @State values rather than stale local lets
    /// captured at body-eval time.
    @State private var canvasGeoSize: CGSize = .zero
    /// J-23.5 — When true, single-finger drag pans the canvas
    /// instead of placing / moving pixels. Toggled by a button
    /// in the top toolbar. SwiftUI's DragGesture has no built-
    /// in two-finger detection that works reliably alongside
    /// the single-tap drag, so a mode toggle is the simplest
    /// reliable UX.
    @State private var panMode: Bool = false

    @Environment(\.dismiss) private var dismiss

    init(payload: CustomModelPayload,
         commit: @escaping (Int, Int, Int, [[[Int]]]) -> Void) {
        self.payload = payload
        self.commit = commit
        self._w = State(initialValue: max(1, payload.initialWidth))
        self._h = State(initialValue: max(1, payload.initialHeight))
        self._d = State(initialValue: max(1, payload.initialDepth))
        self._grid = State(initialValue: payload.initialLocations.isEmpty
            ? Self.emptyGrid(w: payload.initialWidth,
                              h: payload.initialHeight,
                              d: payload.initialDepth)
            : payload.initialLocations)
        // Image loading happens on .task in the body — keeps
        // init synchronous and off the main-thread file-system
        // path.
    }

    private struct GridPos: Equatable {
        var row: Int
        var col: Int
    }

    /// J-23.1 — in-flight drag state. `originValue > 0` means the
    /// user started the drag on an occupied cell, so the gesture
    /// is a move rather than a tap. `hoverCell` is the cell
    /// currently under the touch — used for the orange drop-
    /// target outline while dragging.
    @State private var dragOrigin: GridPos? = nil
    @State private var dragOriginValue: Int = 0
    @State private var dragHoverCell: GridPos? = nil
    /// J-23.5 — Initial touch location (in canvas coords) so the
    /// gesture's .onEnded handler can hit-test line segments on
    /// taps that didn't end up moving any pixels.
    @State private var initialTouch: CGPoint = .zero
    /// J-23.8 — Apple Pencil hover. iPadOS 16.1+ delivers
    /// hover events to `.onContinuousHover`. Tracks the cell
    /// currently under the pencil tip so the canvas can paint
    /// a highlight ring.
    @State private var hoverCell: GridPos? = nil

    /// J-23.5 — Show lines connecting consecutive pixels.
    /// Tapping a line segment opens a sheet to add N evenly-
    /// distributed pixels along it. Captured at sheet-open so
    /// the segment's endpoints survive subsequent edits.
    @State private var showLines: Bool = true
    @State private var distributePayload: DistributePayload? = nil
    /// Plain-Int fields (instead of `GridPos`) so the payload
    /// can carry a publicly-visible cell ref to the sheet
    /// without exposing the editor's private nested type.
    struct DistributePayload: Identifiable {
        let id = UUID()
        let fromNumber: Int
        let toNumber: Int
        let fromRow: Int
        let fromCol: Int
        let toRow: Int
        let toCol: Int
    }

    var body: some View {
        NavigationStack {
            VStack(spacing: 0) {
                gridSettingsBar
                Divider()
                if d > 1 {
                    layerSlider
                    Divider()
                }
                gridCanvas
                    .frame(maxWidth: .infinity, maxHeight: .infinity)
                    .background(Color(uiColor: .systemBackground))
                Divider()
                selectionBar
            }
            .navigationTitle("Custom Model — \(payload.modelName)")
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Save") {
                        commit(w, h, d, grid)
                        dismiss()
                    }
                }
            }
            .alert("Renumber Pixel",
                   isPresented: $renumberSheetVisible) {
                TextField("New number", text: $renumberDraft)
                    .keyboardType(.numberPad)
                Button("Cancel", role: .cancel) {}
                Button("Apply") {
                    applyRenumber()
                }
            } message: {
                Text("Enter the new pixel number. If another cell already uses it, that cell will be swapped to the previous number.")
            }
            .sheet(item: $distributePayload) { payload in
                DistributePointsSheet(payload: payload,
                                       commit: { count in
                                           distributePixels(payload, count: count)
                                       })
            }
        }
    }

    // MARK: - Top: grid size

    private var gridSettingsBar: some View {
        HStack(spacing: 16) {
            HStack(spacing: 4) {
                Text("W").foregroundStyle(.secondary).font(.caption2)
                LayoutEditorIntSpin(
                    id: "cm.w",
                    initial: w,
                    range: 1...500,
                    commit: { newW in
                        guard newW != w else { return }
                        grid = Self.resized(grid, fromW: w, fromH: h,
                                              toW: newW, toH: h)
                        w = newW
                        selection = nil
                        panOffset = .zero
                        panCommitted = .zero
                    })
                .frame(maxWidth: 160)
            }
            HStack(spacing: 4) {
                Text("H").foregroundStyle(.secondary).font(.caption2)
                LayoutEditorIntSpin(
                    id: "cm.h",
                    initial: h,
                    range: 1...500,
                    commit: { newH in
                        guard newH != h else { return }
                        grid = Self.resized(grid, fromW: w, fromH: h,
                                              toW: w, toH: newH)
                        h = newH
                        selection = nil
                        panOffset = .zero
                        panCommitted = .zero
                    })
                .frame(maxWidth: 160)
            }
            HStack(spacing: 4) {
                Text("D").foregroundStyle(.secondary).font(.caption2)
                LayoutEditorIntSpin(
                    id: "cm.d",
                    initial: d,
                    range: 1...50,
                    commit: { newD in
                        guard newD != d else { return }
                        grid = Self.resizedDepth(grid, fromW: w, fromH: h,
                                                   fromD: d, toD: newD)
                        d = newD
                        if layer >= newD { layer = newD - 1 }
                    })
                .frame(maxWidth: 140)
            }
            Spacer()
            // J-23.4 — zoom slider (0.5x..5.0x). Pinch on the
            // canvas updates the same `zoom` state so the two
            // controls are kept in sync.
            HStack(spacing: 4) {
                Image(systemName: "minus.magnifyingglass")
                    .font(.caption2)
                    .foregroundStyle(.secondary)
                Slider(value: $zoom, in: 0.5...5.0) { editing in
                    if !editing { zoomCommitted = zoom }
                }
                .frame(width: 140)
                Image(systemName: "plus.magnifyingglass")
                    .font(.caption2)
                    .foregroundStyle(.secondary)
                Button {
                    zoom = 1.0
                    zoomCommitted = 1.0
                    panOffset = .zero
                    panCommitted = .zero
                } label: {
                    Text("1×").font(.caption2.monospacedDigit())
                }
                .buttonStyle(.bordered)
                .controlSize(.mini)
            }
            // J-23.5 — Pan-mode toggle. When on, single-finger
            // drag scrolls the canvas; turns off automatically
            // when the user resets zoom to 1×.
            Button {
                panMode.toggle()
            } label: {
                Image(systemName: panMode ? "hand.draw.fill"
                                          : "hand.draw")
            }
            .buttonStyle(.bordered)
            .controlSize(.small)
            .tint(panMode ? .accentColor : .secondary)
            .help(panMode ? "Pan mode on — single-finger drag scrolls"
                          : "Pan mode off — single-finger drag places pixels")
            Text("\(pointCount()) pts")
                .foregroundStyle(.secondary)
                .font(.caption.monospacedDigit())
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 8)
    }

    @ViewBuilder
    private var layerSlider: some View {
        HStack(spacing: 8) {
            Text("Layer").foregroundStyle(.secondary).font(.caption2)
            LayoutEditorIntSpin(
                id: "cm.layer",
                initial: layer + 1,
                range: 1...d,
                commit: { newL in
                    layer = max(0, min(d - 1, newL - 1))
                    selection = nil
                })
            .frame(maxWidth: 130)
            Spacer()
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 6)
    }

    // MARK: - Canvas

    private var gridCanvas: some View {
        GeometryReader { geo in
            // J-23.5 — also stash geo.size in @State so the
            // gesture closures (which may outlive the body
            // eval) can compute fresh hit-test coords.
            let _ = capture(geoSize: geo.size)
            // Cell sizing — square cells; canvas centres in
            // available rect. `zoom` scales the cellSide;
            // `panOffset` offsets the centred origin so zoomed
            // canvases can be scrolled.
            let baseCellSide = min(geo.size.width / CGFloat(w),
                                     geo.size.height / CGFloat(h))
            let cellSide = baseCellSide * zoom
            let canvasW = cellSide * CGFloat(w)
            let canvasH = cellSide * CGFloat(h)
            let xOff = (geo.size.width  - canvasW) / 2 + panOffset.width
            let yOff = (geo.size.height - canvasH) / 2 + panOffset.height
            ZStack(alignment: .topLeading) {
                // Grid lines + cells.
                Canvas { ctx, _ in
                    // Background fill
                    let bgRect = CGRect(x: xOff, y: yOff,
                                         width: canvasW, height: canvasH)
                    ctx.fill(Path(bgRect),
                              with: .color(Color(uiColor: .secondarySystemBackground)))
                    // J-23.2 — background image. Drawn first so
                    // gridlines + points sit on top. Scale +
                    // brightness mirror desktop: scale% sizes the
                    // image relative to the model bounds (100 =
                    // fit), brightness% modulates alpha.
                    if let img = backgroundImage,
                       let cg = img.cgImage {
                        let scale = CGFloat(payload.initialBkgScale) / 100.0
                        let bri   = max(0.0, min(1.0,
                                          CGFloat(payload.initialBkgBrightness) / 100.0))
                        let iw = canvasW * scale
                        let ih = canvasH * scale
                        let ix = xOff + (canvasW - iw) / 2
                        let iy = yOff + (canvasH - ih) / 2
                        let dest = CGRect(x: ix, y: iy, width: iw, height: ih)
                        let imageRect = CGRect(origin: .zero,
                                                 size: CGSize(width: cg.width,
                                                              height: cg.height))
                        ctx.opacity = bri
                        ctx.draw(Image(cg, scale: 1.0, label: Text("bg")),
                                  in: dest)
                        ctx.opacity = 1.0
                        _ = imageRect  // silence "unused" warning
                    }
                    // Grid lines.
                    var path = Path()
                    for c in 0...w {
                        let x = xOff + CGFloat(c) * cellSide
                        path.move(to: CGPoint(x: x, y: yOff))
                        path.addLine(to: CGPoint(x: x, y: yOff + canvasH))
                    }
                    for r in 0...h {
                        let y = yOff + CGFloat(r) * cellSide
                        path.move(to: CGPoint(x: xOff, y: y))
                        path.addLine(to: CGPoint(x: xOff + canvasW, y: y))
                    }
                    ctx.stroke(path, with: .color(.gray.opacity(0.3)), lineWidth: 0.5)

                    // J-23.5 — Lines between consecutive pixels.
                    // Drawn before the cell dots so the dots sit
                    // on top of the lines. Sorted by pixel number
                    // — gaps in numbering still produce a single
                    // segment between the two nearest defined
                    // pixels.
                    let segments = self.lineSegments(cellSide: cellSide,
                                                       xOff: xOff, yOff: yOff)
                    if showLines && segments.count > 0 {
                        var linePath = Path()
                        for s in segments {
                            linePath.move(to: s.from)
                            linePath.addLine(to: s.to)
                        }
                        ctx.stroke(linePath,
                                    with: .color(.accentColor.opacity(0.45)),
                                    style: StrokeStyle(lineWidth: 1.5,
                                                        lineCap: .round))
                    }

                    // Points: numbered circles on filled cells.
                    let layerGrid = grid[safe: layer] ?? []
                    for r in 0..<h {
                        let rowArr = layerGrid[safe: r] ?? []
                        for c in 0..<w {
                            let v = rowArr[safe: c] ?? 0
                            if v > 0 {
                                let cellRect = CGRect(
                                    x: xOff + CGFloat(c) * cellSide,
                                    y: yOff + CGFloat(r) * cellSide,
                                    width: cellSide,
                                    height: cellSide)
                                let dotRect = cellRect.insetBy(
                                    dx: cellSide * 0.15,
                                    dy: cellSide * 0.15)
                                ctx.fill(Path(ellipseIn: dotRect),
                                          with: .color(.accentColor))
                                let txt = Text("\(v)")
                                    .font(.system(size: max(8, cellSide * 0.4),
                                                  weight: .semibold))
                                    .foregroundStyle(.white)
                                ctx.draw(txt, at: CGPoint(x: cellRect.midX,
                                                           y: cellRect.midY))
                            }
                        }
                    }

                    // Selection highlight.
                    if let sel = selection {
                        let cellRect = CGRect(
                            x: xOff + CGFloat(sel.col) * cellSide,
                            y: yOff + CGFloat(sel.row) * cellSide,
                            width: cellSide,
                            height: cellSide)
                        ctx.stroke(Path(cellRect),
                                    with: .color(.orange),
                                    lineWidth: 2.0)
                    }
                    // J-23.8 — Pencil hover ring. Subtle cyan so
                    // it doesn't fight the orange selection
                    // outline if the user hovers the selected
                    // cell.
                    if let hov = hoverCell, hov != selection {
                        let cellRect = CGRect(
                            x: xOff + CGFloat(hov.col) * cellSide,
                            y: yOff + CGFloat(hov.row) * cellSide,
                            width: cellSide,
                            height: cellSide)
                        let inset = cellRect.insetBy(dx: 1, dy: 1)
                        ctx.stroke(Path(inset),
                                    with: .color(.cyan.opacity(0.75)),
                                    lineWidth: 1.5)
                    }
                    // Drag drop-target preview.
                    if dragOriginValue > 0, let hover = dragHoverCell,
                       hover != dragOrigin {
                        let cellRect = CGRect(
                            x: xOff + CGFloat(hover.col) * cellSide,
                            y: yOff + CGFloat(hover.row) * cellSide,
                            width: cellSide,
                            height: cellSide)
                        let dotRect = cellRect.insetBy(
                            dx: cellSide * 0.15,
                            dy: cellSide * 0.15)
                        ctx.stroke(Path(ellipseIn: dotRect),
                                    with: .color(.accentColor.opacity(0.7)),
                                    style: StrokeStyle(lineWidth: 2,
                                                        dash: [4, 3]))
                        let txt = Text("\(dragOriginValue)")
                            .font(.system(size: max(8, cellSide * 0.4),
                                          weight: .semibold))
                            .foregroundStyle(Color.accentColor.opacity(0.7))
                        ctx.draw(txt, at: CGPoint(x: cellRect.midX,
                                                   y: cellRect.midY))
                    }
                }
                .gesture(canvasGesture(geoSize: geo.size))
                .simultaneousGesture(magnificationGesture)
                // J-23.8 — Apple Pencil hover. With pencil hover
                // turned on at the iPadOS level (Settings →
                // Apple Pencil → Hover), this fires as the user
                // floats the tip over the canvas — used to
                // highlight whichever cell would be acted on.
                .onContinuousHover(coordinateSpace: .local) { phase in
                    switch phase {
                    case .active(let location):
                        let m = currentMetrics(geoSize: geo.size)
                        let r = Int(((location.y - m.yOff) / m.cellSide).rounded(.down))
                        let c = Int(((location.x - m.xOff) / m.cellSide).rounded(.down))
                        if r >= 0, r < h, c >= 0, c < w {
                            hoverCell = GridPos(row: r, col: c)
                        } else {
                            hoverCell = nil
                        }
                    case .ended:
                        hoverCell = nil
                    }
                }
            }
            .clipped()
            .task(id: payload.initialBackground) {
                await loadBackgroundImage()
            }
        }
    }

    private func capture(geoSize: CGSize) -> Int {
        if canvasGeoSize != geoSize {
            DispatchQueue.main.async {
                if canvasGeoSize != geoSize {
                    canvasGeoSize = geoSize
                }
            }
        }
        return 0
    }

    /// J-23.5/.6 — Compute current cellSide / xOff / yOff. The
    /// gesture-time path passes `geoSize` from the GeometryReader
    /// so it doesn't depend on the async-updated canvasGeoSize
    /// @State; the no-arg variant falls back to that @State for
    /// cases where the gesture didn't capture the size (none
    /// today — kept for symmetry).
    private func currentMetrics(geoSize: CGSize? = nil)
            -> (cellSide: CGFloat, xOff: CGFloat, yOff: CGFloat) {
        let size = geoSize ?? canvasGeoSize
        guard size.width > 0, size.height > 0,
              w > 0, h > 0 else { return (1, 0, 0) }
        let baseCellSide = min(size.width  / CGFloat(w),
                                size.height / CGFloat(h))
        let cellSide = baseCellSide * zoom
        let canvasW = cellSide * CGFloat(w)
        let canvasH = cellSide * CGFloat(h)
        let xOff = (size.width  - canvasW) / 2 + panOffset.width
        let yOff = (size.height - canvasH) / 2 + panOffset.height
        return (cellSide, xOff, yOff)
    }

    /// J-23.4 — Pinch-to-zoom. Multiplies the committed zoom by
    /// the gesture's relative magnitude so the live value tracks
    /// the user's fingers smoothly; on .ended we snap the
    /// committed value to the live value and clamp to the
    /// slider's range.
    private var magnificationGesture: some Gesture {
        MagnificationGesture()
            .onChanged { value in
                multiTouchActive = true
                let z = zoomCommitted * value
                zoom = max(0.5, min(5.0, z))
            }
            .onEnded { _ in
                zoomCommitted = zoom
                // Multi-touch flag clears on the next single-
                // finger touch — leave it true here so the
                // trailing finger-up doesn't fire a stray tap.
                DispatchQueue.main.async { multiTouchActive = false }
            }
    }


    /// J-23.2 — Async background image load. Loads off the main
    /// thread so a giant texture doesn't stall the canvas.
    /// `task(id:)` re-fires if the path changes (won't happen in
    /// this MVP — bg is captured at sheet-open — but future
    /// "change image inside the editor" lands here).
    @MainActor
    private func loadBackgroundImage() async {
        let path = payload.initialBackground
        if path.isEmpty {
            backgroundImage = nil
            return
        }
        let img = await Task.detached(priority: .userInitiated) { () -> UIImage? in
            return UIImage(contentsOfFile: path)
        }.value
        backgroundImage = img
    }

    /// J-23.1 — Tap + drag in one gesture. The DragGesture fires
    /// .onChanged on every touch event including the initial
    /// touch-down (because `minimumDistance: 0`). We record the
    /// cell under the first .onChanged and remember its value;
    /// subsequent .onChanged events update `dragHoverCell` so
    /// the canvas can render the drop-target preview. .onEnded
    /// is where the actual mutation happens — drag-to-move if
    /// origin/end differ and origin was occupied, otherwise a
    /// regular tap (handleTap takes over).
    private func canvasGesture(geoSize: CGSize) -> some Gesture {
        DragGesture(minimumDistance: 0)
            .onChanged { value in
                // J-23.4/.5/.6 — when a pinch is active the
                // single-finger handler must not touch pixels;
                // pinch fingers register here first and would
                // otherwise look like a tap on an arbitrary
                // cell. cellSide/xOff/yOff are recomputed each
                // event from the geoSize captured at body-eval
                // time + live @State so a mid-flight zoom can't
                // desync the math.
                if multiTouchActive {
                    dragOrigin = nil
                    dragHoverCell = nil
                    return
                }
                // J-23.5 — Pan mode: single-finger drag scrolls
                // the canvas instead of editing pixels. The
                // committed pan offset is restored each gesture
                // start so the user can swipe-pan repeatedly.
                if panMode {
                    panOffset = CGSize(
                        width:  panCommitted.width  + value.translation.width,
                        height: panCommitted.height + value.translation.height)
                    return
                }
                let m = currentMetrics(geoSize: geoSize)
                let p = value.location
                let r = Int(((p.y - m.yOff) / m.cellSide).rounded(.down))
                let c = Int(((p.x - m.xOff) / m.cellSide).rounded(.down))
                guard r >= 0, r < h, c >= 0, c < w else {
                    dragHoverCell = nil
                    return
                }
                let cell = GridPos(row: r, col: c)
                if dragOrigin == nil {
                    // J-23.8 — Fat-finger snap. The drawn dot only
                    // covers the middle 70% of the cell, so a
                    // finger touch near the edge lands in an
                    // adjacent empty cell and the user gets a new
                    // pixel placed instead of selecting the
                    // existing one. If the touched cell is empty
                    // but there's a pixel within `slop` screen
                    // distance of the touch point, snap to that
                    // pixel. Pencil taps hit precisely and skip
                    // the snap (the empty cell stays empty).
                    let touchedVal = grid[safe: layer]?[safe: r]?[safe: c] ?? 0
                    if touchedVal == 0 {
                        let slop = max(m.cellSide * 0.85, 28)
                        if let snapped = nearestPixelCell(to: p,
                                                            cellSide: m.cellSide,
                                                            xOff: m.xOff,
                                                            yOff: m.yOff,
                                                            slop: slop) {
                            dragOrigin = snapped
                            dragOriginValue = grid[layer][snapped.row][snapped.col]
                        } else {
                            dragOrigin = cell
                            dragOriginValue = 0
                        }
                    } else {
                        dragOrigin = cell
                        dragOriginValue = touchedVal
                    }
                    initialTouch = p
                }
                dragHoverCell = cell
            }
            .onEnded { value in
                defer {
                    dragOrigin = nil
                    dragOriginValue = 0
                    dragHoverCell = nil
                }
                if multiTouchActive { return }
                if panMode {
                    panCommitted = panOffset
                    return
                }
                guard let origin = dragOrigin else { return }
                // J-23.7 — Removed the tap-on-line distribute
                // detection. Taps on the line through an empty
                // cell were ambiguous with "place new pixel
                // here" and the math kept dropping legitimate
                // line hits. Replaced with an explicit
                // "Distribute to #N" button in the selection
                // bar — tap a pixel, then the button to
                // distribute along the segment going to the
                // next-numbered pixel.
                let endCell = dragHoverCell ?? origin
                // J-23.8 — Drag-to-move now gates on physical
                // movement distance, not just cell-difference.
                // The snap-to-nearest-pixel for fat fingers
                // moves dragOrigin into a different cell from
                // the touched cell, so the old "endCell !=
                // origin" check would fire on every tap-near-
                // pixel. 16 pt threshold ≈ a deliberate drag;
                // small finger jitter on a tap stays under.
                let movement = hypot(value.location.x - initialTouch.x,
                                      value.location.y - initialTouch.y)
                let isDrag = movement > 16
                if dragOriginValue > 0,
                   isDrag,
                   endCell != origin,
                   endCell.row >= 0, endCell.row < h,
                   endCell.col >= 0, endCell.col < w {
                    let endVal = grid[layer][endCell.row][endCell.col]
                    if endVal <= 0 {
                        // Move into an empty cell. xLights's
                        // CustomModel uses -1 for blank in loaded
                        // models and 0 for blank in new grids, so
                        // treat anything <= 0 as empty.
                        grid[layer][origin.row][origin.col] = 0
                        grid[layer][endCell.row][endCell.col] = dragOriginValue
                    } else {
                        // Swap with another pixel (same semantics
                        // as the Renumber alert's swap).
                        grid[layer][origin.row][origin.col] = endVal
                        grid[layer][endCell.row][endCell.col] = dragOriginValue
                    }
                    selection = endCell
                    return
                }
                // No drag → tap. origin may already be snapped
                // (J-23.8) so handleTap on it lands on the
                // right pixel cell.
                handleTap(row: origin.row, col: origin.col)
            }
    }

    // MARK: - Bottom: selection actions

    @ViewBuilder
    private var selectionBar: some View {
        let selectedValue = selectedValue
        HStack(spacing: 10) {
            if let sel = selection, selectedValue > 0 {
                Text("Pixel #\(selectedValue) @ (\(sel.col + 1), \(sel.row + 1))")
                    .font(.caption.monospacedDigit())
                    .foregroundStyle(.secondary)
                Spacer()
                // J-23.7 — Distribute along the segment to the
                // next-numbered pixel. Replaces the unreliable
                // tap-on-line detection with an explicit
                // button: tap a pixel, tap this, sheet opens
                // with the segment (selected → next) preselected.
                if let next = nextPixelSegmentFromSelection() {
                    Button {
                        distributePayload = DistributePayload(
                            fromNumber: next.fromNum,
                            toNumber: next.toNum,
                            fromRow: next.fromCell.row,
                            fromCol: next.fromCell.col,
                            toRow: next.toCell.row,
                            toCol: next.toCell.col)
                    } label: {
                        Label("Distribute → #\(next.toNum)",
                              systemImage: "arrow.left.and.right")
                    }
                    .buttonStyle(.bordered)
                    .controlSize(.small)
                }
                Button {
                    renumberSelected()
                } label: {
                    Label("Renumber", systemImage: "number")
                }
                .buttonStyle(.bordered)
                .controlSize(.small)
                Button(role: .destructive) {
                    deleteSelected()
                } label: {
                    Label("Delete", systemImage: "trash")
                }
                .buttonStyle(.bordered)
                .controlSize(.small)
            } else if let _ = selection {
                Text("Empty cell — tap to place next pixel")
                    .font(.caption)
                    .foregroundStyle(.secondary)
                Spacer()
            } else {
                Text("Tap a cell to place a pixel.")
                    .font(.caption)
                    .foregroundStyle(.secondary)
                Spacer()
                Button(role: .destructive) {
                    grid = Self.emptyGrid(w: w, h: h, d: d)
                    selection = nil
                } label: {
                    Label("Clear All", systemImage: "trash")
                }
                .buttonStyle(.bordered)
                .controlSize(.small)
            }
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 8)
    }

    /// J-23.7 — Find the segment going from the currently-
    /// selected pixel to the next-higher-numbered pixel. Used
    /// to power the "Distribute → #N" button. Returns nil
    /// when there's no selection or the selected pixel is the
    /// highest-numbered one.
    private func nextPixelSegmentFromSelection() ->
            (fromNum: Int, toNum: Int,
             fromCell: GridPos, toCell: GridPos)? {
        guard let sel = selection else { return nil }
        let curVal = selectedValue
        guard curVal > 0 else { return nil }
        // Walk every layer / row / col to find the next-higher
        // pixel by value. Could memoise but the cost is one
        // O(w·h·d) scan per body re-eval, which is fine for
        // realistic grids.
        var next: (num: Int, cell: GridPos)? = nil
        for li in 0..<grid.count {
            for r in 0..<grid[li].count {
                for c in 0..<grid[li][r].count {
                    let v = grid[li][r][c]
                    if v <= curVal { continue }
                    if next == nil || v < next!.num {
                        next = (v, GridPos(row: r, col: c))
                    }
                }
            }
        }
        guard let n = next else { return nil }
        return (curVal, n.num, sel, n.cell)
    }

    private var selectedValue: Int {
        guard let sel = selection,
              let row = grid[safe: layer]?[safe: sel.row],
              let v = row[safe: sel.col] else { return 0 }
        return v
    }

    // MARK: - Actions

    /// Tap on a cell. Empty cell → place next free pixel.
    /// Occupied cell → select. (Drag handles "move" now —
    /// J-23.1.)
    private func handleTap(row: Int, col: Int) {
        let v = grid[safe: layer]?[safe: row]?[safe: col] ?? 0
        if v > 0 {
            selection = GridPos(row: row, col: col)
        } else {
            let next = nextFreeNumber()
            grid[layer][row][col] = next
            selection = GridPos(row: row, col: col)
        }
    }

    /// J-23 — apply the renumber draft. Swap semantics: if the
    /// target number already exists, that pixel takes the
    /// selected pixel's old number. Otherwise just renumber in
    /// place.
    private func applyRenumber() {
        guard let sel = selection else { return }
        let old = grid[layer][sel.row][sel.col]
        guard let newN = Int(renumberDraft.trimmingCharacters(in: .whitespaces)),
              newN > 0, newN != old else { return }
        // Look for an existing cell with the target number.
        for li in 0..<grid.count {
            for r in 0..<grid[li].count {
                for c in 0..<grid[li][r].count {
                    if grid[li][r][c] == newN {
                        grid[li][r][c] = old
                    }
                }
            }
        }
        grid[layer][sel.row][sel.col] = newN
    }

    /// J-23.8 — Snap-to-nearest-pixel for fat-finger taps. Walks
    /// the active layer and returns the cell of the pixel whose
    /// centre is closest to `tap` (in canvas screen coords), so
    /// long as that distance is within `slop`. nil otherwise.
    private func nearestPixelCell(to tap: CGPoint,
                                    cellSide: CGFloat,
                                    xOff: CGFloat,
                                    yOff: CGFloat,
                                    slop: CGFloat) -> GridPos? {
        var best: (dist: CGFloat, cell: GridPos)? = nil
        let layerGrid = grid[safe: layer] ?? []
        let slop2 = slop * slop
        for r in 0..<h {
            let rowArr = layerGrid[safe: r] ?? []
            for c in 0..<w {
                if (rowArr[safe: c] ?? 0) <= 0 { continue }
                let cx = xOff + (CGFloat(c) + 0.5) * cellSide
                let cy = yOff + (CGFloat(r) + 0.5) * cellSide
                let dx = tap.x - cx, dy = tap.y - cy
                let d2 = dx * dx + dy * dy
                if d2 > slop2 { continue }
                if best == nil || d2 < (best!.dist * best!.dist) {
                    best = (d2.squareRoot(), GridPos(row: r, col: c))
                }
            }
        }
        return best?.cell
    }

    // J-23.5 — Line-segment + distribute helpers.

    /// Build pixel-to-pixel segments for the current layer,
    /// sorted by pixel number. Includes screen-space endpoints
    /// so the canvas can draw + the gesture handler can hit-
    /// test the same data.
    private func lineSegments(cellSide: CGFloat,
                                xOff: CGFloat,
                                yOff: CGFloat) ->
            [(fromNum: Int, toNum: Int,
              fromCell: GridPos, toCell: GridPos,
              from: CGPoint, to: CGPoint)] {
        var dots: [(num: Int, cell: GridPos, center: CGPoint)] = []
        let layerGrid = grid[safe: layer] ?? []
        for r in 0..<h {
            let rowArr = layerGrid[safe: r] ?? []
            for c in 0..<w {
                let v = rowArr[safe: c] ?? 0
                if v > 0 {
                    let cx = xOff + (CGFloat(c) + 0.5) * cellSide
                    let cy = yOff + (CGFloat(r) + 0.5) * cellSide
                    dots.append((v, GridPos(row: r, col: c),
                                  CGPoint(x: cx, y: cy)))
                }
            }
        }
        dots.sort { $0.num < $1.num }
        var out: [(Int, Int, GridPos, GridPos, CGPoint, CGPoint)] = []
        for i in 0..<(dots.count - (dots.isEmpty ? 0 : 1)) {
            let a = dots[i], b = dots[i + 1]
            out.append((a.num, b.num, a.cell, b.cell, a.center, b.center))
        }
        return out
    }

    /// Find the closest line segment to a tap point. Returns
    /// nil when no segment is within the supplied threshold.
    /// Caller supplies the threshold so dense grids (tiny
    /// cellSide) can override the cellSide-derived default.
    private func nearestSegment(to p: CGPoint,
                                  cellSide: CGFloat,
                                  xOff: CGFloat,
                                  yOff: CGFloat,
                                  threshold: CGFloat) -> (fromNum: Int, toNum: Int,
                                                            fromCell: GridPos,
                                                            toCell: GridPos)? {
        let segs = lineSegments(cellSide: cellSide, xOff: xOff, yOff: yOff)
        var best: (dist: CGFloat,
                   fromNum: Int, toNum: Int,
                   fromCell: GridPos, toCell: GridPos)? = nil
        for s in segs {
            let d = pointToSegmentDistance(p, a: s.from, b: s.to)
            if d > threshold { continue }
            if best == nil || d < best!.dist {
                best = (d, s.fromNum, s.toNum, s.fromCell, s.toCell)
            }
        }
        if let b = best {
            return (b.fromNum, b.toNum, b.fromCell, b.toCell)
        }
        return nil
    }

    /// Standard point-to-segment distance. Returns the distance
    /// from `p` to the segment `a`-`b` (closest point clamped
    /// to within the segment endpoints).
    private func pointToSegmentDistance(_ p: CGPoint,
                                          a: CGPoint, b: CGPoint) -> CGFloat {
        let dx = b.x - a.x
        let dy = b.y - a.y
        let len2 = dx * dx + dy * dy
        if len2 < 0.0001 {
            let ex = p.x - a.x, ey = p.y - a.y
            return (ex * ex + ey * ey).squareRoot()
        }
        var t = ((p.x - a.x) * dx + (p.y - a.y) * dy) / len2
        if t < 0 { t = 0 } else if t > 1 { t = 1 }
        let cx = a.x + t * dx
        let cy = a.y + t * dy
        let ex = p.x - cx, ey = p.y - cy
        return (ex * ex + ey * ey).squareRoot()
    }

    /// J-23.5/.8 — Insert N evenly-distributed pixels along the
    /// segment from pixel `payload.fromNumber` to
    /// `payload.toNumber`. The new pixels take numbers
    /// (fromNumber+1)...(fromNumber+placed); any existing pixel
    /// numbered > fromNumber shifts up to keep the numbering
    /// contiguous.
    ///
    /// Cell picking uses Bresenham's line algorithm — this
    /// avoids the lerp-and-round failure where two adjacent
    /// endpoints (from=(5,3), to=(5,4)) round most fractions to
    /// either endpoint and leave no room for intermediate
    /// pixels.
    private func distributePixels(_ payload: DistributePayload, count: Int) {
        guard count > 0 else { return }
        let fromCell = GridPos(row: payload.fromRow, col: payload.fromCol)
        let toCell   = GridPos(row: payload.toRow,   col: payload.toCol)
        let fromNum  = payload.fromNumber

        // Walk the cells along the line; strip the two
        // endpoints (which already hold pixels).
        let line = cellsAlongLine(from: fromCell, to: toCell)
        let intermediate: [GridPos] = line.count > 2
            ? Array(line.dropFirst().dropLast())
            : []
        if intermediate.isEmpty { return }

        // Pick `count` evenly-spaced cells from the intermediate
        // list. Each i in 0..<n samples at fraction
        // ((i + 0.5) / n) of the intermediate strip.
        let n = min(count, intermediate.count)
        var picked: [GridPos] = []
        for i in 0..<n {
            let frac = (Double(i) + 0.5) / Double(n)
            var idx = Int((frac * Double(intermediate.count)).rounded(.down))
            if idx < 0 { idx = 0 }
            if idx >= intermediate.count { idx = intermediate.count - 1 }
            picked.append(intermediate[idx])
        }
        if picked.isEmpty { return }

        // Shift existing pixel numbers > fromNum up by `picked
        // .count` so we have a contiguous (fromNum+1) ...
        // (fromNum+n) range to assign.
        let shift = picked.count
        for li in 0..<grid.count {
            for r in 0..<grid[li].count {
                for c in 0..<grid[li][r].count {
                    if grid[li][r][c] > fromNum {
                        grid[li][r][c] += shift
                    }
                }
            }
        }

        // Place the new pixels. xLights's CustomModel uses -1 for
        // a blank cell in loaded models and 0 for blank in new
        // grids; treat anything <= 0 as empty so we don't refuse
        // to fill cells that the canvas already shows as blank.
        for (i, p) in picked.enumerated() {
            guard p.row >= 0, p.row < h, p.col >= 0, p.col < w else { continue }
            if grid[layer][p.row][p.col] <= 0 {
                grid[layer][p.row][p.col] = fromNum + i + 1
            }
        }
    }

    /// J-23.8 — Bresenham's line-cell walk between two grid
    /// positions. Returns every cell intersected by the line,
    /// including both endpoints. Useful for picking
    /// intermediate cells when distributing pixels.
    private func cellsAlongLine(from: GridPos, to: GridPos) -> [GridPos] {
        var out: [GridPos] = []
        var x = from.col, y = from.row
        let x1 = to.col, y1 = to.row
        let dx = abs(x1 - x)
        let dy = -abs(y1 - y)
        let sx = x  < x1 ? 1 : -1
        let sy = y  < y1 ? 1 : -1
        var err = dx + dy
        while true {
            out.append(GridPos(row: y, col: x))
            if x == x1 && y == y1 { break }
            let e2 = 2 * err
            if e2 >= dy {
                err += dy
                x   += sx
            }
            if e2 <= dx {
                err += dx
                y   += sy
            }
        }
        return out
    }

    private func nextFreeNumber() -> Int {
        var seen = Set<Int>()
        for layerArr in grid {
            for rowArr in layerArr {
                for v in rowArr where v > 0 {
                    seen.insert(v)
                }
            }
        }
        var n = 1
        while seen.contains(n) { n += 1 }
        return n
    }

    private func deleteSelected() {
        guard let sel = selection else { return }
        grid[layer][sel.row][sel.col] = 0
        selection = nil
    }

    /// Renumber the selected pixel — show an alert with the
    /// current number; user types a new one. If the target
    /// number already exists elsewhere, shift the existing
    /// pixel out of the way by giving it the old number
    /// (swap semantics).
    @State private var renumberSheetVisible: Bool = false
    @State private var renumberDraft: String = ""
    private func renumberSelected() {
        renumberDraft = "\(selectedValue)"
        renumberSheetVisible = true
    }

    // MARK: - Helpers

    private func pointCount() -> Int {
        var n = 0
        for layerArr in grid {
            for rowArr in layerArr {
                for v in rowArr where v > 0 { n += 1 }
            }
        }
        return n
    }

    static func emptyGrid(w: Int, h: Int, d: Int) -> [[[Int]]] {
        let safeW = max(1, w), safeH = max(1, h), safeD = max(1, d)
        return Array(repeating:
            Array(repeating:
                Array(repeating: 0, count: safeW),
            count: safeH),
        count: safeD)
    }

    /// Resize the grid in W/H dimensions. Existing cells outside
    /// the new bounds are dropped; new cells are zero.
    static func resized(_ src: [[[Int]]],
                          fromW: Int, fromH: Int,
                          toW: Int, toH: Int) -> [[[Int]]] {
        let depth = src.count
        var dst = emptyGrid(w: toW, h: toH, d: depth)
        let copyH = min(fromH, toH)
        let copyW = min(fromW, toW)
        for layer in 0..<depth {
            for r in 0..<copyH where r < src[layer].count {
                for c in 0..<copyW where c < src[layer][r].count {
                    dst[layer][r][c] = src[layer][r][c]
                }
            }
        }
        return dst
    }

    /// Resize the depth dimension. Cropping drops back layers;
    /// new layers are empty.
    static func resizedDepth(_ src: [[[Int]]],
                               fromW: Int, fromH: Int,
                               fromD: Int, toD: Int) -> [[[Int]]] {
        if fromD == toD { return src }
        if toD < fromD {
            return Array(src.prefix(toD))
        }
        var dst = src
        for _ in fromD..<toD {
            dst.append(emptyGrid(w: fromW, h: fromH, d: 1)[0])
        }
        return dst
    }
}

private extension Array {
    subscript(safe i: Int) -> Element? {
        return (i >= 0 && i < count) ? self[i] : nil
    }
}

/// J-22 — Generic FaceState-shaped editor (`map<name, map<attr,
/// value>>`). Faces, States, and Dimming Curve all share this
/// shape on the C++ side, so one editor + one bridge surface
/// covers all three. The editor uses a two-level navigation:
/// list of entries (face / state / channel names) → tap to drill
/// into the per-attribute editor. Top-level supports
/// Add / Rename / Delete entries; detail supports Add / Edit /
/// Delete attribute key-value pairs.
private struct FaceStatePayload: Identifiable {
    let id = UUID()
    /// `faces`, `states`, or `dimming` — drives the title + the
    /// initial-keys hint shown when the user adds a new entry.
    let kind: Kind
    let entries: [String: [String: String]]
    let modelName: String
    enum Kind: String {
        case faces, states, dimming
        var title: String {
            switch self {
            case .faces:   return "Faces"
            case .states:  return "States"
            case .dimming: return "Dimming Curve"
            }
        }
        /// Suggested keys when the user creates a new entry —
        /// matches desktop's standard phoneme set for faces and
        /// the channel list for dimming. States are user-named
        /// so no suggestions.
        var suggestedAttributeKeys: [String] {
            switch self {
            case .faces:
                return ["AI", "E", "FV", "L", "MBP", "O", "U", "WQ", "etc",
                        "Eyes-Open", "Eyes-Closed"]
            case .states:  return []
            case .dimming: return ["gamma", "offset", "subtract", "scale", "file"]
            }
        }
    }
}

private struct FaceStateEditorSheet: View {
    let payload: FaceStatePayload
    let commit: (_ entries: [String: [String: String]]) -> Void

    @State private var entries: [String: [String: String]]
    @State private var newEntryName: String = ""
    @State private var editingEntry: String? = nil
    @Environment(\.dismiss) private var dismiss

    init(payload: FaceStatePayload,
         commit: @escaping (_ entries: [String: [String: String]]) -> Void) {
        self.payload = payload
        self.commit = commit
        self._entries = State(initialValue: payload.entries)
    }

    var body: some View {
        NavigationStack {
            List {
                Section("Add") {
                    HStack {
                        TextField("New \(payload.kind.title.lowercased()) name",
                                  text: $newEntryName)
                            .textInputAutocapitalization(.never)
                            .autocorrectionDisabled(true)
                            .submitLabel(.done)
                            .onSubmit(addEntry)
                        Button("Add", action: addEntry)
                            .disabled(normalizedNewName.isEmpty ||
                                      entries[normalizedNewName] != nil)
                    }
                }
                Section(entries.isEmpty ? "Nothing defined"
                                        : "\(entries.count) entr\(entries.count == 1 ? "y" : "ies")") {
                    if entries.isEmpty {
                        Text("Tap Add to create the first \(payload.kind.title.lowercased()) entry.")
                            .font(.caption)
                            .foregroundStyle(.secondary)
                    } else {
                        ForEach(entries.keys.sorted(), id: \.self) { name in
                            NavigationLink(value: name) {
                                HStack {
                                    Text(name).font(.body.monospaced())
                                    Spacer()
                                    let attrCount = entries[name]?.count ?? 0
                                    Text("\(attrCount) attr\(attrCount == 1 ? "" : "s")")
                                        .font(.caption2)
                                        .foregroundStyle(.secondary)
                                }
                            }
                        }
                        .onDelete { idx in
                            let sorted = entries.keys.sorted()
                            for i in idx {
                                entries.removeValue(forKey: sorted[i])
                            }
                        }
                    }
                }
            }
            .navigationDestination(for: String.self) { name in
                FaceStateEntryDetailView(
                    entryName: name,
                    suggestedKeys: payload.kind.suggestedAttributeKeys,
                    attributes: Binding(
                        get: { entries[name] ?? [:] },
                        set: { entries[name] = $0 }
                    )
                )
            }
            .navigationTitle("\(payload.kind.title) — \(payload.modelName)")
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Save") {
                        commit(entries)
                        dismiss()
                    }
                }
            }
        }
        .presentationDetents([.large])
    }

    private var normalizedNewName: String {
        newEntryName.trimmingCharacters(in: .whitespacesAndNewlines)
    }

    private func addEntry() {
        let n = normalizedNewName
        guard !n.isEmpty, entries[n] == nil else { return }
        entries[n] = [:]
        newEntryName = ""
    }
}

/// J-22 — Detail view for one face / state / dimming entry. Lists
/// the attribute key-value pairs and lets the user edit values,
/// add new keys (with optional suggested-key buttons), and remove
/// entries via swipe.
private struct FaceStateEntryDetailView: View {
    let entryName: String
    let suggestedKeys: [String]
    @Binding var attributes: [String: String]
    @State private var newKey: String = ""

    var body: some View {
        List {
            if !suggestedKeys.isEmpty {
                Section("Suggested keys") {
                    ScrollView(.horizontal, showsIndicators: false) {
                        HStack(spacing: 6) {
                            ForEach(suggestedKeys, id: \.self) { k in
                                Button {
                                    if attributes[k] == nil {
                                        attributes[k] = ""
                                    }
                                } label: {
                                    Text(k)
                                        .font(.caption)
                                        .padding(.horizontal, 8)
                                        .padding(.vertical, 4)
                                        .background(
                                            Capsule().fill(
                                                attributes[k] == nil
                                                ? Color.accentColor.opacity(0.2)
                                                : Color.gray.opacity(0.2)
                                            )
                                        )
                                }
                                .disabled(attributes[k] != nil)
                                .buttonStyle(.plain)
                            }
                        }
                    }
                }
            }
            Section("Add attribute") {
                HStack {
                    TextField("Key", text: $newKey)
                        .textInputAutocapitalization(.never)
                        .autocorrectionDisabled(true)
                        .submitLabel(.done)
                        .onSubmit(addAttr)
                    Button("Add", action: addAttr)
                        .disabled(normalizedNewKey.isEmpty ||
                                  attributes[normalizedNewKey] != nil)
                }
            }
            Section(attributes.isEmpty ? "No attributes"
                                       : "Attributes (\(attributes.count))") {
                if attributes.isEmpty {
                    Text("Tap a suggested key, or type a new key in 'Add attribute' above.")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                } else {
                    ForEach(attributes.keys.sorted(), id: \.self) { key in
                        HStack(alignment: .firstTextBaseline) {
                            Text(key)
                                .font(.caption.monospacedDigit())
                                .frame(minWidth: 100, alignment: .leading)
                                .foregroundStyle(.secondary)
                            TextField("value", text: Binding(
                                get: { attributes[key] ?? "" },
                                set: { attributes[key] = $0 }
                            ))
                                .textInputAutocapitalization(.never)
                                .autocorrectionDisabled(true)
                                .multilineTextAlignment(.trailing)
                        }
                    }
                    .onDelete { idx in
                        let sorted = attributes.keys.sorted()
                        for i in idx {
                            attributes.removeValue(forKey: sorted[i])
                        }
                    }
                }
            }
        }
        .navigationTitle(entryName)
    }

    private var normalizedNewKey: String {
        newKey.trimmingCharacters(in: .whitespacesAndNewlines)
    }

    private func addAttr() {
        let k = normalizedNewKey
        guard !k.isEmpty, attributes[k] == nil else { return }
        attributes[k] = ""
        newKey = ""
    }
}


/// J-23.5 — Sheet asking "how many points to add along this
/// line?" Opens when the user taps a line segment in the
/// custom-model editor. New pixels are inserted between the
/// two endpoints (numbered fromNum+1 .. fromNum+N) and any
/// existing pixel > fromNum shifts up by N to make room.
private struct DistributePointsSheet: View {
    let payload: CustomModelEditorSheet.DistributePayload
    let commit: (_ count: Int) -> Void

    @State private var count: Int = 1
    @Environment(\.dismiss) private var dismiss

    var body: some View {
        NavigationStack {
            Form {
                Section("Segment") {
                    LabeledContent("From pixel", value: "#\(payload.fromNumber)")
                    LabeledContent("To pixel",   value: "#\(payload.toNumber)")
                }
                Section("Points to add") {
                    LayoutEditorIntSpin(
                        id: "distribute.count",
                        initial: count,
                        range: 1...500,
                        commit: { count = $0 })
                }
                Section {
                    Text("New pixels will be numbered #\(payload.fromNumber + 1) through #\(payload.fromNumber + count). Pixels above #\(payload.fromNumber) shift up by \(count) to keep numbering contiguous.")
                        .font(.caption2)
                        .foregroundStyle(.secondary)
                }
            }
            .navigationTitle("Distribute Along Line")
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Add") {
                        commit(count)
                        dismiss()
                    }
                    .disabled(count <= 0)
                }
            }
        }
        .presentationDetents([.large])
    }
}

/// J-20.7 — Identifiable payload for the layer-sizes editor.
/// `.sheet(item:)` recreates the sheet whenever the item changes,
/// so the sheet's @State always initialises from the right sizes.
private struct LayerSizesPayload: Identifiable {
    let id = UUID()
    let sizes: [Int]
}

private struct LayerSizesEditorSheet: View {
    let commit: (_ sizes: [Int]) -> Void

    @State private var sizes: [Int]
    @Environment(\.dismiss) private var dismiss

    // J-20.7 — initialise `sizes` from `initial` at construction
    // time. Previous version used `@State private var sizes: [Int] = []`
    // + .onAppear copy, which lost the data when SwiftUI cached
    // the view across re-presentations.
    init(initial: [Int], commit: @escaping (_ sizes: [Int]) -> Void) {
        self.commit = commit
        self._sizes = State(initialValue: initial.isEmpty ? [1] : initial)
    }

    var body: some View {
        NavigationStack {
            List {
                Section(footer: Text("Each layer's node count. The Layered Arches Nodes property is the sum across layers — desktop's behaviour.")
                            .font(.caption2)
                            .foregroundStyle(.secondary)) {
                    ForEach(Array(sizes.enumerated()), id: \.offset) { idx, _ in
                        HStack {
                            Text("Layer \(idx + 1)")
                                .frame(minWidth: 70, alignment: .leading)
                                .foregroundStyle(.secondary)
                            Spacer()
                            // J-20.4 — spin button so each layer's
                            // size is editable + nudge-able with
                            // the +/- buttons.
                            LayoutEditorIntSpin(
                                id: "layerSize.\(idx)",
                                initial: sizes[idx],
                                range: 1...10_000,
                                commit: { sizes[idx] = $0 }
                            )
                            .frame(maxWidth: 160, alignment: .trailing)
                        }
                    }
                    .onDelete { idx in
                        sizes.remove(atOffsets: idx)
                    }
                }
                Section {
                    Button {
                        sizes.append(1)
                    } label: {
                        Label("Add layer", systemImage: "plus")
                    }
                }
            }
            .navigationTitle("Layer Sizes")
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Save") {
                        commit(sizes)
                        dismiss()
                    }
                    .disabled(sizes.isEmpty)
                }
            }
        }
        .presentationDetents([.medium, .large])
    }
}

/// J-18 pass 5 — read-only list of groups containing this model,
/// with tap-to-navigate. Tapping a row closes the sheet and
/// switches the sidebar to Groups with that group preselected.
/// Editing membership stays on the Groups tab — this is just a
/// shortcut to get there.
private struct GroupRefListSheet: View {
    let modelName: String
    let groups: [String]
    let onTap: (_ groupName: String) -> Void
    @Environment(\.dismiss) private var dismiss

    var body: some View {
        NavigationStack {
            Group {
                if groups.isEmpty {
                    ContentUnavailableView("No groups",
                        systemImage: "rectangle.3.group",
                        description: Text("This model isn't in any groups."))
                } else {
                    List {
                        Section(footer: Text("Tap a group to open it in the Groups tab.")
                                    .font(.caption2)
                                    .foregroundStyle(.secondary)) {
                            ForEach(groups, id: \.self) { g in
                                Button {
                                    onTap(g)
                                } label: {
                                    HStack {
                                        Text(g).font(.body.monospaced())
                                            .foregroundStyle(.primary)
                                        Spacer()
                                        Image(systemName: "chevron.right")
                                            .font(.caption2)
                                            .foregroundStyle(.secondary)
                                    }
                                }
                            }
                        }
                    }
                }
            }
            .navigationTitle("In Model Groups — \(modelName)")
            .toolbar {
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") { dismiss() }
                }
            }
        }
        .presentationDetents([.medium, .large])
    }
}

/// J-12 — Add-View-Object sheet + delete-confirmation alert +
/// per-type file picker. Factored out so the modifier chain on
/// LayoutEditorView's body stays inside the Swift type-checker's
/// complexity budget.
private struct ViewObjectCrudModifiers: ViewModifier {
    @Binding var objectFilePickerVisible: Bool
    let objectFilePickerTypes: [UTType]
    @Binding var addViewObjectSheetVisible: Bool
    @Binding var pendingDeleteObjectName: String?
    @Binding var renameObjectSheetVisible: Bool
    @Binding var pendingRenameObjectOldName: String?
    let availableTypes: [String]
    let onCreateObject: (String) -> Void
    let onDeleteObject: (String) -> Void
    let onRenameObject: (String, String) -> Void
    let onFilePicked: (Result<[URL], Error>) -> Void
    /// J-17 — existing-name lookup for rename collision check.
    let existingNames: Set<String>
    let sanitizeName: (String) -> String

    func body(content: Content) -> some View {
        content
            .fileImporter(isPresented: $objectFilePickerVisible,
                          allowedContentTypes: objectFilePickerTypes.isEmpty
                                                  ? [.data]
                                                  : objectFilePickerTypes,
                          allowsMultipleSelection: false,
                          onCompletion: onFilePicked)
            .sheet(isPresented: $addViewObjectSheetVisible) {
                AddViewObjectSheet(
                    types: availableTypes,
                    onSelect: { type in
                        onCreateObject(type)
                    },
                    onCancel: { addViewObjectSheetVisible = false }
                )
            }
            .sheet(isPresented: $renameObjectSheetVisible) {
                if let oldName = pendingRenameObjectOldName {
                    RenameGroupSheet(
                        oldName: oldName,
                        existingNames: Set(existingNames.filter { $0 != oldName }),
                        onRename: { newName in
                            onRenameObject(oldName, newName)
                            renameObjectSheetVisible = false
                            pendingRenameObjectOldName = nil
                        },
                        onCancel: {
                            renameObjectSheetVisible = false
                            pendingRenameObjectOldName = nil
                        },
                        sanitize: sanitizeName,
                        kindLabel: "Object"
                    )
                }
            }
            .alert(deleteTitle,
                   isPresented: Binding(
                        get: { pendingDeleteObjectName != nil },
                        set: { if !$0 { pendingDeleteObjectName = nil } })) {
                Button("Delete", role: .destructive) {
                    if let name = pendingDeleteObjectName { onDeleteObject(name) }
                }
                Button("Cancel", role: .cancel) { }
            } message: {
                Text("Removes this view object from the show. Save the layout to make the change permanent.")
            }
    }

    private var deleteTitle: String {
        "Delete \(pendingDeleteObjectName ?? "")?"
    }
}

/// J-12 — view-object type picker shown by the Objects tab's
/// "+" button.
private struct AddViewObjectSheet: View {
    let types: [String]
    let onSelect: (String) -> Void
    let onCancel: () -> Void
    @Environment(\.dismiss) private var dismiss

    var body: some View {
        NavigationStack {
            List(types, id: \.self) { type in
                Button {
                    onSelect(type)
                } label: {
                    HStack {
                        Image(systemName: iconFor(type))
                            .foregroundStyle(.secondary)
                        Text(type)
                            .foregroundStyle(.primary)
                        Spacer()
                    }
                }
            }
            .navigationTitle("Add View Object")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { onCancel() }
                }
            }
        }
        .presentationDetents([.medium])
    }

    private func iconFor(_ type: String) -> String {
        switch type {
        case "Image":     return "photo"
        case "Mesh":      return "cube.transparent"
        case "Gridlines": return "grid"
        case "Terrain":   return "mountain.2"
        case "Ruler":     return "ruler"
        default:          return "square.on.square"
        }
    }
}

/// J-11 — enforce exactly-one-active-selection across the three
/// sidebar tabs (Models / Groups / Objects). Setting any one
/// clears the other two; explicit tab switch drops the previous
/// tab's selection; a canvas tap that lands on a model flips
/// the sidebar to the Models tab so the property pane shows the
/// right surface.
///
/// Factored out as a ViewModifier so LayoutEditorView's body
/// chain stays under the Swift type-checker's complexity budget.
private struct SidebarSelectionMutex: ViewModifier {
    @Environment(SequencerViewModel.self) var viewModel
    @Binding var sidebarTab: LayoutSidebarTab

    func body(content: Content) -> some View {
        content
            .onChange(of: viewModel.layoutEditorSelectedModel) { _, newSel in
                guard let name = newSel, !name.isEmpty else { return }
                if viewModel.layoutEditorSelectedGroup != nil {
                    viewModel.layoutEditorSelectedGroup = nil
                }
                if viewModel.layoutEditorSelectedObject != nil {
                    viewModel.layoutEditorSelectedObject = nil
                }
                if sidebarTab != .models {
                    sidebarTab = .models
                }
            }
            .onChange(of: viewModel.layoutEditorSelectedGroup) { _, newSel in
                guard newSel != nil else { return }
                if viewModel.layoutEditorSelectedModel != nil ||
                   !viewModel.layoutEditorSelection.isEmpty {
                    viewModel.layoutSelectSingle(nil)
                }
                if viewModel.layoutEditorSelectedObject != nil {
                    viewModel.layoutEditorSelectedObject = nil
                }
            }
            .onChange(of: viewModel.layoutEditorSelectedObject) { oldSel, newSel in
                guard newSel != nil else {
                    // Selection cleared — drop any terrain edit
                    // session so the next tap doesn't paint.
                    if viewModel.terrainEditTarget != nil {
                        viewModel.terrainEditTarget = nil
                    }
                    return
                }
                if viewModel.layoutEditorSelectedModel != nil ||
                   !viewModel.layoutEditorSelection.isEmpty {
                    viewModel.layoutSelectSingle(nil)
                }
                if viewModel.layoutEditorSelectedGroup != nil {
                    viewModel.layoutEditorSelectedGroup = nil
                }
                // J-13 — if the user picks a different VO than the
                // terrain currently being edited, exit edit mode.
                if let editing = viewModel.terrainEditTarget,
                   newSel != editing {
                    viewModel.terrainEditTarget = nil
                }
            }
            .onChange(of: sidebarTab) { _, newTab in
                // Explicit tab switch drops the leaving tab's
                // selection so the canvas tint reflects where the
                // user is now looking.
                switch newTab {
                case .models:
                    viewModel.layoutEditorSelectedGroup = nil
                    viewModel.layoutEditorSelectedObject = nil
                case .groups:
                    if viewModel.layoutEditorSelectedModel != nil ||
                       !viewModel.layoutEditorSelection.isEmpty {
                        viewModel.layoutSelectSingle(nil)
                    }
                    viewModel.layoutEditorSelectedObject = nil
                case .objects:
                    if viewModel.layoutEditorSelectedModel != nil ||
                       !viewModel.layoutEditorSelection.isEmpty {
                        viewModel.layoutSelectSingle(nil)
                    }
                    viewModel.layoutEditorSelectedGroup = nil
                }
                // J-20.6 — mirror tab state to the view model so
                // the canvas can gate VO picks (don't snag the
                // House mesh when the user clicks off a model on
                // the Models tab).
                viewModel.layoutEditorActiveTab = newTab.rawValue
            }
            .onAppear {
                viewModel.layoutEditorActiveTab = sidebarTab.rawValue
            }
    }
}

/// J-7 (group CRUD) — extracted modifier hosting the new-group
/// sheet, add-member sheet, and delete-confirmation alert.
/// LayoutEditorView's body was over the type-checker's complexity
/// budget once these landed inline; factoring keeps the call site
/// to a single `.modifier(...)`.
private struct GroupCrudModifiers: ViewModifier {
    @Binding var newGroupSheetVisible: Bool
    @Binding var addMemberSheetVisible: Bool
    @Binding var pendingDeleteGroupName: String?
    @Binding var renameGroupSheetVisible: Bool
    @Binding var pendingRenameGroupOldName: String?
    let groupNames: [String]
    let modelNames: [String]
    let onCreateGroup: (String) -> Void
    let onAddMembers: ([String]) -> Void
    let onDeleteGroup: (String) -> Void
    let onRenameGroup: (String, String) -> Void
    let selectedGroupName: String?
    let currentMembers: Set<String>
    /// J-9 — bridge lookup so the AddMemberSheet's tree can lazily
    /// fetch submodels for a parent. Captured at the call site so
    /// the sheet itself doesn't need a view-model handle.
    let submodelsFor: (String) -> [String]
    /// J-16 — name sanitizer (wraps `Model::SafeModelName`).
    let sanitizeName: (String) -> String

    func body(content: Content) -> some View {
        content
            .sheet(isPresented: $newGroupSheetVisible) {
                NewGroupSheet(
                    existingNames: Set(groupNames + modelNames),
                    onCreate: onCreateGroup,
                    onCancel: { newGroupSheetVisible = false },
                    sanitize: sanitizeName
                )
            }
            .sheet(isPresented: $addMemberSheetVisible) {
                if let groupName = selectedGroupName {
                    let candidates = modelNames.filter { !currentMembers.contains($0) }
                    AddMemberSheet(
                        groupName: groupName,
                        candidates: candidates,
                        existingMembers: currentMembers,
                        submodelsFor: submodelsFor,
                        onAdd: onAddMembers,
                        onCancel: { addMemberSheetVisible = false }
                    )
                }
            }
            .sheet(isPresented: $renameGroupSheetVisible) {
                if let oldName = pendingRenameGroupOldName {
                    // Exclude the current name from the
                    // collision set (computed inline via filter
                    // so the closure stays expression-only and
                    // SwiftUI's ViewBuilder is happy).
                    RenameGroupSheet(
                        oldName: oldName,
                        existingNames: Set(
                            (groupNames + modelNames).filter { $0 != oldName }
                        ),
                        onRename: { newName in
                            onRenameGroup(oldName, newName)
                            renameGroupSheetVisible = false
                            pendingRenameGroupOldName = nil
                        },
                        onCancel: {
                            renameGroupSheetVisible = false
                            pendingRenameGroupOldName = nil
                        },
                        sanitize: sanitizeName
                    )
                }
            }
            .alert(deleteAlertTitle,
                   isPresented: Binding(
                        get: { pendingDeleteGroupName != nil },
                        set: { if !$0 { pendingDeleteGroupName = nil } })) {
                Button("Delete", role: .destructive) {
                    if let name = pendingDeleteGroupName { onDeleteGroup(name) }
                }
                Button("Cancel", role: .cancel) { }
            } message: {
                Text("Removes this group from the active layout. Save the layout to make the change permanent.")
            }
    }

    private var deleteAlertTitle: String {
        "Delete \(pendingDeleteGroupName ?? "")?"
    }
}

/// J-16 / J-17 — generic rename sheet. Used for both ModelGroup
/// and ViewObject renames; the `kindLabel` parameter swaps "Group"
/// vs. "Object" in the title and description text. Mirrors
/// NewGroupSheet's UX (text field + collision check + Cancel /
/// Rename buttons) but pre-fills with the current name.
private struct RenameGroupSheet: View {
    let oldName: String
    let existingNames: Set<String>
    let onRename: (String) -> Void
    let onCancel: () -> Void
    let sanitize: (String) -> String
    /// "Group" / "Object" — used in the navigation title and the
    /// description footer. Defaults to "Group" for backwards
    /// compatibility with J-16 group-rename call sites.
    var kindLabel: String = "Group"
    @Environment(\.dismiss) private var dismiss
    @State private var name: String = ""
    @FocusState private var nameFocused: Bool

    private var trimmedName: String {
        name.trimmingCharacters(in: .whitespacesAndNewlines)
    }
    private var sanitizedName: String { sanitize(trimmedName) }
    private var nameChanged: Bool { sanitizedName != trimmedName }
    private var collision: Bool {
        !sanitizedName.isEmpty && existingNames.contains(sanitizedName)
    }
    private var canRename: Bool {
        !sanitizedName.isEmpty && !collision && sanitizedName != oldName
    }
    private var footerText: String {
        switch kindLabel {
        case "Group":
            return "Other groups that reference this one (and any sequences targeting it) update their references automatically. Save the layout to persist."
        case "Model":
            return "Groups containing this model and sequences targeting it update their references automatically. Save the layout to persist."
        default:
            return "References to this item update automatically. Save the layout to persist."
        }
    }

    var body: some View {
        NavigationStack {
            Form {
                Section("Old Name") {
                    Text(oldName)
                        .foregroundStyle(.secondary)
                }
                Section("New Name") {
                    TextField("Group name", text: $name)
                        .focused($nameFocused)
                        .autocorrectionDisabled()
                        .textInputAutocapitalization(.words)
                }
                if nameChanged && !sanitizedName.isEmpty {
                    Section {
                        Label("Will save as \"\(sanitizedName)\"",
                               systemImage: "info.circle")
                            .foregroundStyle(.blue)
                        Text("These characters can't appear in group names: , ~ ! ; < > \" ' & : | @ / \\")
                            .font(.caption2)
                            .foregroundStyle(.secondary)
                    }
                }
                if collision {
                    Section {
                        Label("\"\(sanitizedName)\" is already in use",
                               systemImage: "exclamationmark.triangle.fill")
                            .foregroundStyle(.orange)
                    }
                }
                Section {
                    Text(footerText)
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
            }
            .navigationTitle("Rename \(kindLabel)")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { onCancel() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Rename") { onRename(sanitizedName) }
                        .disabled(!canRename)
                }
            }
            .onAppear {
                name = oldName
                nameFocused = true
            }
        }
        .presentationDetents([.medium])
    }
}

/// J-6 (per-type properties) — renders the descriptor list
/// returned by `perTypePropertiesForModel:`. Generic over kind so
/// new model types can be wired by adding a bridge case without
/// touching SwiftUI.
private struct LayoutEditorTypePropertiesView: View {
    let modelName: String
    let descriptors: [[String: Any]]
    let token: Int
    let commit: (_ key: String, _ value: Any) -> Void

    var body: some View {
        if descriptors.isEmpty {
            EmptyView()
        } else {
            VStack(alignment: .leading, spacing: 6) {
                Text("Type Properties")
                    .font(.caption.weight(.semibold))
                    .foregroundStyle(.secondary)
                ForEach(Array(descriptors.enumerated()), id: \.offset) { _, d in
                    descriptorRow(d)
                }
            }
            .font(.caption.monospacedDigit())
        }
    }

    @ViewBuilder
    private func descriptorRow(_ d: [String: Any]) -> some View {
        let key = d["key"] as? String ?? ""
        let label = d["label"] as? String ?? key
        let kind = d["kind"] as? String ?? ""
        let enabled = (d["enabled"] as? Bool) ?? true

        HStack(alignment: .firstTextBaseline) {
            Text(label)
                .foregroundStyle(.secondary)
                .frame(minWidth: 110, alignment: .leading)
            Spacer(minLength: 8)
            controlFor(kind: kind, key: key, d: d)
                .disabled(!enabled)
                .opacity(enabled ? 1.0 : 0.5)
                .lineLimit(1)
        }
    }

    @ViewBuilder
    private func controlFor(kind: String, key: String, d: [String: Any]) -> some View {
        switch kind {
        case "int":
            let v = (d["value"] as? NSNumber)?.doubleValue ?? 0
            let minV = (d["min"] as? NSNumber)?.doubleValue
            let maxV = (d["max"] as? NSNumber)?.doubleValue
            LayoutEditorDoubleField(
                id: "\(modelName).\(key).\(token)",
                initial: v,
                min: minV,
                max: maxV,
                precision: 0,
                commit: { newValue in commit(key, NSNumber(value: Int(newValue))) }
            )
            .frame(maxWidth: 110, alignment: .trailing)
        case "double":
            let v = (d["value"] as? NSNumber)?.doubleValue ?? 0
            let minV = (d["min"] as? NSNumber)?.doubleValue
            let maxV = (d["max"] as? NSNumber)?.doubleValue
            let precision = (d["precision"] as? NSNumber)?.intValue ?? 2
            LayoutEditorDoubleField(
                id: "\(modelName).\(key).\(token)",
                initial: v,
                min: minV,
                max: maxV,
                precision: precision,
                commit: { newValue in commit(key, NSNumber(value: newValue)) }
            )
            .frame(maxWidth: 110, alignment: .trailing)
        case "bool":
            let v = (d["value"] as? Bool) ?? false
            Toggle("", isOn: Binding(
                get: { v },
                set: { commit(key, NSNumber(value: $0)) }
            ))
            .labelsHidden()
            .controlSize(.mini)
        case "enum":
            let idx = (d["value"] as? NSNumber)?.intValue ?? 0
            let opts = (d["options"] as? [String]) ?? []
            Menu {
                ForEach(Array(opts.enumerated()), id: \.offset) { i, label in
                    Button {
                        commit(key, NSNumber(value: i))
                    } label: {
                        HStack {
                            Text(label)
                            if i == idx {
                                Spacer()
                                Image(systemName: "checkmark")
                            }
                        }
                    }
                }
            } label: {
                Text(idx >= 0 && idx < opts.count ? opts[idx] : "—")
                    .truncationMode(.middle)
                    .frame(maxWidth: 160, alignment: .trailing)
            }
            .menuStyle(.button)
            .controlSize(.mini)
        case "string":
            let v = d["value"] as? String ?? ""
            LayoutEditorStringField(
                id: "\(modelName).\(key).\(token)",
                initial: v,
                commit: { commit(key, $0 as NSString) }
            )
            .frame(maxWidth: 160, alignment: .trailing)
        default:
            Text("?").foregroundStyle(.tertiary)
        }
    }
}

/// J-5 → J-7 — property editor for a ModelGroup. Settings
/// surface (layout group / camera / layout style / grid size /
/// centre) plus an editable member list (swipe to delete, "+ Add
/// Member" to open the picker sheet).
private struct LayoutEditorGroupPropertiesView: View {
    let groupName: String
    let summary: [String: Any]
    let layoutGroups: [String]
    let token: Int
    let commit: (_ key: String, _ value: Any) -> Void
    let onRemoveMember: (_ memberName: String) -> Void
    let onAddMember: () -> Void
    let onReorderMembers: (_ newOrder: [String]) -> Void
    let onRenameRequest: () -> Void

    /// J-10 — drag-target tracking for the manual VStack member
    /// list. Storing the dragged name + hovered index here lets us
    /// commit reorders without an internal-scroll List (which was
    /// shrinking the visible window to 2 rows inside the outer
    /// property pane's ScrollView).
    @State private var draggingMember: String? = nil
    @State private var dropTargetIndex: Int? = nil

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            row("Name") {
                HStack(spacing: 6) {
                    Text(groupName)
                        .truncationMode(.middle)
                    Button {
                        onRenameRequest()
                    } label: {
                        Image(systemName: "pencil")
                            .font(.caption2)
                    }
                    .buttonStyle(.plain)
                    .foregroundStyle(.secondary)
                    .accessibilityLabel("Rename group")
                }
            }
            row("Type") { Text(summary["displayAs"] as? String ?? "ModelGroup") }
            row("Layout group") { layoutGroupPicker }
            row("Locked") {
                Toggle("", isOn: lockedBinding)
                    .labelsHidden()
                    .controlSize(.mini)
            }

            Divider().padding(.vertical, 2)

            row("Default camera") { defaultCameraPicker }
            row("Layout style")   { layoutStylePicker }
            row("Grid size")      { gridSizeField }
            row("Tag color")      { tagColorPicker }

            Divider().padding(.vertical, 2)

            row("2D centre") {
                Text((summary["centerDefined"] as? Bool ?? false) ? "Custom" : "Auto")
                    .foregroundStyle(.secondary)
            }
            row("Centre X") { numberField(key: "centerX") }
            row("Centre Y") { numberField(key: "centerY") }

            Divider().padding(.vertical, 2)

            membersHeader

            if let members = summary["models"] as? [String], !members.isEmpty {
                // J-10 — manual VStack of rows + per-row drag.
                // Avoids embedding a SwiftUI List inside the
                // property pane's outer ScrollView (the inner List
                // collapses to ~2 rows). The outer ScrollView is
                // already in charge of vertical scrolling, so this
                // grows-to-fit naturally.
                VStack(alignment: .leading, spacing: 0) {
                    ForEach(Array(members.enumerated()), id: \.element) { idx, m in
                        memberRow(name: m,
                                  index: idx,
                                  members: members,
                                  isDropTarget: dropTargetIndex == idx)
                    }
                }
                .padding(.leading, 8)
                .padding(.top, 2)
                .dropDestination(for: String.self) { items, _ in
                    // Drop landing OUTSIDE any specific row → move
                    // to end. Per-row dropDestination already
                    // committed any "drop on row" case before we
                    // get here.
                    guard let dragged = items.first ?? draggingMember,
                          let from = members.firstIndex(of: dragged) else {
                        draggingMember = nil
                        dropTargetIndex = nil
                        return false
                    }
                    var reordered = members
                    reordered.remove(at: from)
                    reordered.append(dragged)
                    onReorderMembers(reordered)
                    draggingMember = nil
                    dropTargetIndex = nil
                    return true
                }
            }
        }
        .font(.caption.monospacedDigit())
    }

    @ViewBuilder
    private var membersHeader: some View {
        HStack(alignment: .firstTextBaseline) {
            Text("Members")
                .foregroundStyle(.secondary)
                .frame(minWidth: 100, alignment: .leading)
            Spacer(minLength: 8)
            Text("\(summary["modelCount"] as? Int ?? 0)")
                .foregroundStyle(.secondary)
            Button {
                onAddMember()
            } label: {
                Image(systemName: "plus.circle.fill")
                    .font(.caption)
            }
            .buttonStyle(.plain)
            .accessibilityLabel("Add member")
        }
    }

    /// J-10 — single member row inside the manual VStack list.
    /// Drag handle on the left, delete on the right. Long-press
    /// or grab-the-handle to drag; drop-on-target inserts before
    /// that row. SwiftUI's `.draggable` + per-row
    /// `.dropDestination` handle the data transport.
    @ViewBuilder
    private func memberRow(name: String,
                            index: Int,
                            members: [String],
                            isDropTarget: Bool) -> some View {
        HStack(spacing: 6) {
            Image(systemName: "line.3.horizontal")
                .font(.caption2)
                .foregroundStyle(.tertiary)
                .frame(width: 12)
            memberIcon(for: name)
                .font(.caption2)
                .foregroundStyle(.tertiary)
            Text(name)
                .lineLimit(1)
                .truncationMode(.middle)
                .font(.caption2)
                .foregroundStyle(.secondary)
            Spacer()
            Button {
                onRemoveMember(name)
            } label: {
                Image(systemName: "minus.circle")
                    .font(.caption2)
                    .foregroundStyle(.red)
            }
            .buttonStyle(.plain)
            .accessibilityLabel("Remove \(name)")
        }
        .padding(.vertical, 3)
        .padding(.horizontal, 4)
        .background(rowBackground(name: name, isDropTarget: isDropTarget))
        .draggable(name) {
            // Drag preview — small, opaque pill that mirrors the
            // row visually so the user sees what they're moving.
            HStack(spacing: 4) {
                memberIcon(for: name)
                Text(name).font(.caption2)
            }
            .padding(.horizontal, 6)
            .padding(.vertical, 2)
            .background(.thinMaterial, in: Capsule())
        }
        .dropDestination(for: String.self) { items, _ in
            guard let dragged = items.first,
                  let from = members.firstIndex(of: dragged) else {
                draggingMember = nil
                dropTargetIndex = nil
                return false
            }
            var to = index
            // Inserting after the source position needs the index
            // shifted back by one because we remove first.
            if from < to { to -= 0 }
            if from == to {
                draggingMember = nil
                dropTargetIndex = nil
                return false
            }
            var reordered = members
            reordered.remove(at: from)
            // Clamp in case `to` ended up past the new end.
            let insertAt = min(to, reordered.count)
            reordered.insert(dragged, at: insertAt)
            onReorderMembers(reordered)
            draggingMember = nil
            dropTargetIndex = nil
            return true
        } isTargeted: { targeted in
            if targeted {
                dropTargetIndex = index
            } else if dropTargetIndex == index {
                dropTargetIndex = nil
            }
        }
    }

    @ViewBuilder
    private func rowBackground(name: String, isDropTarget: Bool) -> some View {
        if isDropTarget {
            // Light tint to show where the dragged row will land.
            RoundedRectangle(cornerRadius: 4)
                .fill(Color.accentColor.opacity(0.18))
        } else {
            Color.clear
        }
    }

    @ViewBuilder
    private func memberIcon(for name: String) -> some View {
        // Members can be top-level models, ModelGroups, or
        // submodels (Parent/Sub form). Use the cube icon for
        // simple models and a layered icon for everything else
        // so the eye can scan the list.
        if name.contains("/") {
            Image(systemName: "square.on.square")
        } else {
            Image(systemName: "cube")
        }
    }

    // MARK: - Cells

    @ViewBuilder
    private func row(_ label: String, @ViewBuilder _ content: () -> some View) -> some View {
        HStack(alignment: .firstTextBaseline) {
            Text(label)
                .foregroundStyle(.secondary)
                .frame(minWidth: 100, alignment: .leading)
            Spacer(minLength: 8)
            content()
                .lineLimit(1)
        }
    }

    private func numberField(key: String) -> some View {
        LayoutEditorDoubleField(
            id: "\(groupName).\(key).\(token)",
            initial: doubleVal(key),
            min: nil,
            commit: { v in commit(key, NSNumber(value: v)) }
        )
        .frame(maxWidth: 110, alignment: .trailing)
    }

    private var lockedBinding: Binding<Bool> {
        Binding(
            get: { summary["locked"] as? Bool ?? false },
            set: { commit("locked", NSNumber(value: $0)) }
        )
    }

    private var layoutGroupPicker: some View {
        Menu {
            ForEach(layoutGroups, id: \.self) { name in
                Button {
                    commit("layoutGroup", name as NSString)
                } label: {
                    HStack {
                        Text(name)
                        if name == (summary["layoutGroup"] as? String) {
                            Spacer()
                            Image(systemName: "checkmark")
                        }
                    }
                }
            }
        } label: {
            Text(summary["layoutGroup"] as? String ?? "Default")
                .truncationMode(.middle)
        }
        .menuStyle(.button)
        .controlSize(.mini)
    }

    /// J-9 — Default Camera picker. Source list comes from the
    /// bridge so any user-defined 3D camera (saved viewpoint) is
    /// included alongside the always-present "2D".
    private var defaultCameraPicker: some View {
        let current = summary["defaultCamera"] as? String ?? "2D"
        let opts = (summary["defaultCameraOptions"] as? [String]) ?? ["2D"]
        return Menu {
            ForEach(opts, id: \.self) { name in
                Button {
                    commit("defaultCamera", name as NSString)
                } label: {
                    HStack {
                        Text(name)
                        if name == current {
                            Spacer()
                            Image(systemName: "checkmark")
                        }
                    }
                }
            }
        } label: {
            Text(current)
                .truncationMode(.middle)
                .frame(maxWidth: 140, alignment: .trailing)
        }
        .menuStyle(.button)
        .controlSize(.mini)
    }

    /// J-9 — Layout Style picker. Options come from the bridge as
    /// {value, label} pairs (xml wire form vs user-facing). If the
    /// stored value isn't in the list (legacy XML), we still show
    /// it but mark it as "Custom: …".
    private var layoutStylePicker: some View {
        let current = summary["layout"] as? String ?? "minimalGrid"
        let rawOpts = (summary["layoutStyleOptions"] as? [[String: String]]) ?? []
        let opts: [(value: String, label: String)] = rawOpts.compactMap {
            guard let v = $0["value"], let l = $0["label"] else { return nil }
            return (v, l)
        }
        let knownLabel = opts.first(where: { $0.value == current })?.label
        return Menu {
            ForEach(opts, id: \.value) { entry in
                Button {
                    commit("layout", entry.value as NSString)
                } label: {
                    HStack {
                        Text(entry.label)
                        if entry.value == current {
                            Spacer()
                            Image(systemName: "checkmark")
                        }
                    }
                }
            }
            if knownLabel == nil && !current.isEmpty {
                Divider()
                Text("Currently: \(current)")
            }
        } label: {
            Text(knownLabel ?? (current.isEmpty ? "—" : "Custom: \(current)"))
                .truncationMode(.middle)
                .frame(maxWidth: 160, alignment: .trailing)
        }
        .menuStyle(.button)
        .controlSize(.mini)
    }

    private var gridSizeField: some View {
        LayoutEditorDoubleField(
            id: "\(groupName).gridSize.\(token)",
            initial: Double(summary["gridSize"] as? Int ?? 400),
            min: 1,
            max: nil,
            precision: 0,
            commit: { v in commit("gridSize", NSNumber(value: Int(v))) }
        )
        .frame(maxWidth: 90, alignment: .trailing)
    }

    /// J-9 — Tag color picker. Same `#RRGGBB` bridge as the Models
    /// tab. Empty string → black fallback.
    private var tagColorPicker: some View {
        let hex = summary["tagColor"] as? String ?? ""
        let parsed = Color(hexString: hex) ?? .black
        return HStack(spacing: 6) {
            ColorPicker("", selection: Binding(
                get: { parsed },
                set: { newColor in
                    commit("tagColor", newColor.toHexString() as NSString)
                }
            ), supportsOpacity: false)
                .labelsHidden()
            Text(hex.isEmpty ? "—" : hex)
                .font(.caption2)
                .foregroundStyle(.secondary)
                .frame(maxWidth: 80, alignment: .leading)
        }
    }

    private func doubleVal(_ key: String) -> Double {
        (summary[key] as? NSNumber)?.doubleValue ?? 0.0
    }
}

/// J-8 — Editor for the synthetic "2D Background" pseudo-object.
/// The active layout group owns its own background settings; this
/// view edits whichever group the user has picked in the layout-
/// group switcher (top-of-canvas overlay). No transform surface —
/// the desktop's background isn't an object you move around, it's
/// an attribute of the preview.
private struct LayoutEditorBackgroundPropertiesView: View {
    let summary: [String: Any]
    let token: Int
    let commit: (_ key: String, _ value: Any) -> Void
    let onPickImage: () -> Void

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            row("Name") {
                Text(summary["name"] as? String ?? "2D Background")
                    .truncationMode(.middle)
            }
            row("Type") {
                Text(summary["displayAs"] as? String ?? "2D Background")
                    .foregroundStyle(.secondary)
            }
            row("Layout group") {
                Text(summary["layoutGroup"] as? String ?? "Default")
                    .foregroundStyle(.secondary)
            }
            Divider().padding(.vertical, 2)
            row("Image") {
                HStack(spacing: 6) {
                    Text(imageLabel)
                        .foregroundStyle(.secondary)
                        .lineLimit(1)
                        .truncationMode(.middle)
                        .frame(maxWidth: 110, alignment: .trailing)
                    Button {
                        onPickImage()
                    } label: {
                        Image(systemName: "folder")
                            .font(.caption)
                    }
                    .buttonStyle(.plain)
                    if !imagePath.isEmpty {
                        Button(role: .destructive) {
                            commit("backgroundImage", "" as NSString)
                        } label: {
                            Image(systemName: "xmark.circle")
                                .font(.caption)
                        }
                        .buttonStyle(.plain)
                    }
                }
            }
            Divider().padding(.vertical, 2)
            row("Brightness") {
                intField(key: "backgroundBrightness", min: 0, max: 100)
            }
            row("Alpha") {
                intField(key: "backgroundAlpha", min: 0, max: 100)
            }
            row("Scale to fit") {
                Toggle("", isOn: Binding(
                    get: { summary["scaleBackgroundImage"] as? Bool ?? false },
                    set: { commit("scaleBackgroundImage", NSNumber(value: $0)) }
                ))
                    .labelsHidden()
                    .controlSize(.mini)
            }
            // J-19 — Layout-group display rolled up here so the
            // Background row carries everything that affects the 2D
            // preview's canvas. Read-only for now — editing these
            // is a follow-up (canvas size + grid / bbox / center-0
            // toggles each need bridge setters).
            Divider().padding(.vertical, 2)
            row("Canvas") {
                Text("\(canvasWidth) × \(canvasHeight)")
                    .foregroundStyle(.secondary)
            }
            row("2D centre = 0") {
                Text((summary["display2DCenter0"] as? Bool ?? false) ? "Yes" : "No")
                    .foregroundStyle(.secondary)
            }
            row("Grid") {
                Text(gridLabel)
                    .foregroundStyle(.secondary)
            }
            row("Bounding box") {
                Text((summary["display2DBoundingBox"] as? Bool ?? false) ? "On" : "Off")
                    .foregroundStyle(.secondary)
            }

            Divider().padding(.vertical, 4)
            Text("The 2D background is a per-layout-group attribute, not a real view object. It draws behind your models in the 2D preview at the configured brightness / alpha.")
                .font(.caption2)
                .foregroundStyle(.tertiary)
                .fixedSize(horizontal: false, vertical: true)
        }
        .font(.caption.monospacedDigit())
    }

    private var canvasWidth: Int {
        (summary["previewWidth"] as? NSNumber)?.intValue
            ?? (summary["previewWidth"] as? Int)
            ?? 0
    }
    private var canvasHeight: Int {
        (summary["previewHeight"] as? NSNumber)?.intValue
            ?? (summary["previewHeight"] as? Int)
            ?? 0
    }
    private var gridLabel: String {
        let on = (summary["display2DGrid"] as? Bool) ?? false
        let spacing = (summary["display2DGridSpacing"] as? NSNumber)?.intValue
            ?? (summary["display2DGridSpacing"] as? Int)
            ?? 100
        return on ? "On (\(spacing))" : "Off"
    }

    private var imagePath: String {
        summary["backgroundImage"] as? String ?? ""
    }

    private var imageLabel: String {
        if imagePath.isEmpty { return "—" }
        return (imagePath as NSString).lastPathComponent
    }

    @ViewBuilder
    private func row(_ label: String, @ViewBuilder _ content: () -> some View) -> some View {
        HStack(alignment: .firstTextBaseline) {
            Text(label)
                .foregroundStyle(.secondary)
                .frame(minWidth: 100, alignment: .leading)
            Spacer(minLength: 8)
            content()
                .lineLimit(1)
        }
    }

    private func intField(key: String, min: Double, max: Double) -> some View {
        LayoutEditorDoubleField(
            id: "background.\(key).\(token)",
            initial: Double((summary[key] as? NSNumber)?.intValue ?? 0),
            min: min,
            max: max,
            precision: 0,
            commit: { newValue in commit(key, NSNumber(value: Int(newValue))) }
        )
        .frame(maxWidth: 90, alignment: .trailing)
    }
}

/// J-12 — Editable properties for a ViewObject. Mirrors the
/// Models tab's collapsible-section layout: Header (always
/// visible) → per-type → Appearance → Dimensions → Size/Location.
/// Per-type section opens by default since that's the unique
/// surface for this object.
private struct LayoutEditorObjectPropertiesView: View {
    @Environment(SequencerViewModel.self) var viewModel
    let objectName: String
    let summary: [String: Any]
    let layoutGroups: [String]
    let token: Int
    let commit: (_ key: String, _ value: Any) -> Void
    let onPickFile: (_ key: String, _ accepting: [UTType]) -> Void
    let onRenameRequest: () -> Void
    let onDuplicate: () -> Void

    @State private var expandedTypeProps: Bool = true
    @State private var expandedAppearance: Bool = false
    @State private var expandedDimensions: Bool = false
    @State private var expandedSizeLocation: Bool = false

    var typeKind: String { summary["typeKind"] as? String ?? "other" }

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            row("Name") {
                HStack(spacing: 6) {
                    Text(objectName)
                        .truncationMode(.middle)
                    Button {
                        onRenameRequest()
                    } label: {
                        Image(systemName: "pencil")
                            .font(.caption2)
                    }
                    .buttonStyle(.plain)
                    .foregroundStyle(.secondary)
                    .accessibilityLabel("Rename object")
                    Button {
                        onDuplicate()
                    } label: {
                        Image(systemName: "plus.square.on.square")
                            .font(.caption2)
                    }
                    .buttonStyle(.plain)
                    .foregroundStyle(.secondary)
                    .accessibilityLabel("Duplicate object")
                }
            }
            row("Type") { Text(summary["displayAs"] as? String ?? "—") }

            if typeKind != "other" {
                section($expandedTypeProps, title: typeSectionTitle) {
                    perTypeBody
                }
            }

            section($expandedAppearance, title: "Appearance") {
                row("Active") {
                    Toggle("", isOn: boolBinding("active"))
                        .labelsHidden()
                        .controlSize(.mini)
                }
                row("Layout group") { layoutGroupPicker }
            }

            if !((summary["twoPoint"] as? Bool) ?? false) {
                // Two-point screen locations (e.g. Ruler) don't
                // have a meaningful bounding-box width/height/depth
                // — the line's extent comes from its endpoints.
                section($expandedDimensions, title: "Dimensions") {
                    row("Width")  { numberField(key: "width",  min: 0) }
                    row("Height") { numberField(key: "height", min: 0) }
                    row("Depth")  { numberField(key: "depth",  min: 0) }
                }
            }

            section($expandedSizeLocation, title: "Size/Location") {
                if (summary["twoPoint"] as? Bool) ?? false {
                    // J-14 — two-point screen location (Ruler).
                    // Point 1 is the world origin; point 2 is the
                    // absolute coord of the second endpoint.
                    row("Point 1 X") { numberField(key: "p1X") }
                    row("Point 1 Y") { numberField(key: "p1Y") }
                    row("Point 1 Z") { numberField(key: "p1Z") }
                    row("Point 2 X") { numberField(key: "p2X") }
                    row("Point 2 Y") { numberField(key: "p2Y") }
                    row("Point 2 Z") { numberField(key: "p2Z") }
                } else {
                    row("Centre X") { numberField(key: "centerX") }
                    row("Centre Y") { numberField(key: "centerY") }
                    row("Centre Z") { numberField(key: "centerZ") }
                    row("Rotate X") { numberField(key: "rotateX") }
                    row("Rotate Y") { numberField(key: "rotateY") }
                    row("Rotate Z") { numberField(key: "rotateZ") }
                }
                row("Locked") {
                    Toggle("", isOn: boolBinding("locked"))
                        .labelsHidden()
                        .controlSize(.mini)
                }
            }
        }
        .font(.caption.monospacedDigit())
    }

    private var typeSectionTitle: String {
        let t = summary["displayAs"] as? String ?? ""
        return t.isEmpty ? "Object Properties" : t
    }

    // MARK: - Per-type editor bodies

    @ViewBuilder
    private var perTypeBody: some View {
        switch typeKind {
        case "mesh":
            row("Object file") { fileRow(key: "objFile", types: meshTypes) }
            row("Mesh only") {
                Toggle("", isOn: boolBinding("meshOnly"))
                    .labelsHidden()
                    .controlSize(.mini)
            }
            row("Brightness") { intField(key: "brightness", min: 0, max: 100) }
            row("Scale X") { scaleField(key: "scaleX") }
            row("Scale Y") { scaleField(key: "scaleY") }
            row("Scale Z") { scaleField(key: "scaleZ") }
        case "image":
            row("Image") { fileRow(key: "imageFile", types: imageTypes) }
            row("Brightness")   { intField(key: "brightness",   min: 0, max: 100) }
            row("Transparency") { intField(key: "transparency", min: 0, max: 100) }
            row("Scale X") { scaleField(key: "scaleX") }
            row("Scale Y") { scaleField(key: "scaleY") }
            row("Scale Z") { scaleField(key: "scaleZ") }
        case "gridlines":
            row("Spacing")  { intField(key: "gridSpacing", min: 1, max: 10000) }
            row("Width")    { intField(key: "gridWidth",   min: 1, max: 100000) }
            row("Height")   { intField(key: "gridHeight",  min: 1, max: 100000) }
            row("Color")    { gridColorPicker }
            row("Axis")     {
                Toggle("", isOn: boolBinding("hasAxis"))
                    .labelsHidden()
                    .controlSize(.mini)
            }
            row("Point to front") {
                Toggle("", isOn: boolBinding("pointToFront"))
                    .labelsHidden()
                    .controlSize(.mini)
            }
        case "terrain":
            row("Image")     { fileRow(key: "imageFile", types: imageTypes) }
            row("Spacing")   { intField(key: "gridSpacing", min: 1, max: 10000) }
            row("Width")     { intField(key: "gridWidth",   min: 1, max: 100000) }
            row("Depth")     { intField(key: "gridDepth",   min: 1, max: 100000) }
            row("Grid color") { gridColorPicker }
            row("Brightness")   { intField(key: "brightness",   min: 0, max: 100) }
            row("Transparency") { intField(key: "transparency", min: 0, max: 100) }
            row("Hide grid")  {
                Toggle("", isOn: boolBinding("hideGrid"))
                    .labelsHidden()
                    .controlSize(.mini)
            }
            row("Hide image") {
                Toggle("", isOn: boolBinding("hideImage"))
                    .labelsHidden()
                    .controlSize(.mini)
            }
            Divider().padding(.vertical, 2)
            terrainEditControls
        case "ruler":
            row("Units") { unitsPicker }
            row("Length") { doubleField(key: "length", min: 0.0001) }
            Divider().padding(.vertical, 2)
            Text("The Ruler defines real-world scale for the layout — set its length to match a known dimension in your show.")
                .font(.caption2)
                .foregroundStyle(.tertiary)
                .fixedSize(horizontal: false, vertical: true)
        default:
            EmptyView()
        }
    }

    // MARK: - Terrain edit controls (J-13)

    @ViewBuilder
    private var terrainEditControls: some View {
        let editing = viewModel.terrainEditTarget == objectName
        VStack(alignment: .leading, spacing: 4) {
            HStack {
                Text("Edit Heightmap")
                    .foregroundStyle(.secondary)
                    .frame(minWidth: 110, alignment: .leading)
                Spacer()
                Toggle("", isOn: Binding(
                    get: { editing },
                    set: { isOn in
                        viewModel.terrainEditTarget = isOn ? objectName : nil
                    }
                ))
                .labelsHidden()
                .controlSize(.mini)
            }
            if editing {
                row("Direction") {
                    Picker("", selection: Binding(
                        get: { viewModel.terrainEditRaise },
                        set: { viewModel.terrainEditRaise = $0 }
                    )) {
                        Text("Raise").tag(true)
                        Text("Lower").tag(false)
                    }
                    .pickerStyle(.segmented)
                    .frame(maxWidth: 160, alignment: .trailing)
                }
                row("Step") {
                    Slider(value: Binding(
                        get: { Double(viewModel.terrainEditDelta) },
                        set: { viewModel.terrainEditDelta = Float($0) }
                    ), in: 0.1...10.0, step: 0.1)
                    .frame(maxWidth: 140)
                    Text(String(format: "%.1f", viewModel.terrainEditDelta))
                        .font(.caption2.monospacedDigit())
                        .foregroundStyle(.secondary)
                        .frame(width: 32, alignment: .trailing)
                }
                row("Brush") {
                    Slider(value: Binding(
                        get: { Double(viewModel.terrainEditBrushPoints) },
                        set: { viewModel.terrainEditBrushPoints = CGFloat($0) }
                    ), in: 0...80, step: 1)
                    .frame(maxWidth: 140)
                    Text(viewModel.terrainEditBrushPoints == 0
                          ? "point"
                          : "\(Int(viewModel.terrainEditBrushPoints))pt")
                        .font(.caption2.monospacedDigit())
                        .foregroundStyle(.secondary)
                        .frame(width: 36, alignment: .trailing)
                }
                Text("Tap the terrain on the canvas to \(viewModel.terrainEditRaise ? "raise" : "lower") the nearest grid point. The brush radius applies a cosine falloff to neighbours; 0 edits a single point.")
                    .font(.caption2)
                    .foregroundStyle(.tertiary)
                    .fixedSize(horizontal: false, vertical: true)
            }
        }
    }

    // MARK: - Section helper

    @ViewBuilder
    private func section<Content: View>(_ expanded: Binding<Bool>,
                                         title: String,
                                         @ViewBuilder _ content: () -> Content) -> some View {
        let body = VStack(alignment: .leading, spacing: 4) { content() }
            .padding(.vertical, 4)
        DisclosureGroup(isExpanded: expanded) {
            body
        } label: {
            Text(title)
                .font(.caption.weight(.semibold))
                .foregroundStyle(.primary)
        }
        .accentColor(.secondary)
    }

    // MARK: - Cells

    @ViewBuilder
    private func row(_ label: String, @ViewBuilder _ content: () -> some View) -> some View {
        HStack(alignment: .firstTextBaseline) {
            Text(label)
                .foregroundStyle(.secondary)
                .frame(minWidth: 110, alignment: .leading)
            Spacer(minLength: 8)
            content()
                .lineLimit(1)
        }
    }

    private func numberField(key: String, min: Double? = nil) -> some View {
        LayoutEditorDoubleField(
            id: "\(objectName).\(key).\(token)",
            initial: doubleVal(key),
            min: min,
            commit: { newValue in commit(key, NSNumber(value: newValue)) }
        )
        .frame(maxWidth: 110, alignment: .trailing)
    }

    private func doubleField(key: String, min: Double?) -> some View {
        LayoutEditorDoubleField(
            id: "\(objectName).\(key).\(token)",
            initial: doubleVal(key),
            min: min,
            commit: { newValue in commit(key, NSNumber(value: newValue)) }
        )
        .frame(maxWidth: 110, alignment: .trailing)
    }

    /// J-14 — scale field with higher precision than the default
    /// numberField (scales are typically 0.5, 1.25, etc., not
    /// whole numbers). Allows negative values (flips).
    private func scaleField(key: String) -> some View {
        LayoutEditorDoubleField(
            id: "\(objectName).\(key).\(token)",
            initial: doubleVal(key),
            min: nil,
            max: nil,
            precision: 3,
            commit: { newValue in commit(key, NSNumber(value: newValue)) }
        )
        .frame(maxWidth: 110, alignment: .trailing)
    }

    private func intField(key: String, min: Double, max: Double) -> some View {
        LayoutEditorDoubleField(
            id: "\(objectName).\(key).\(token)",
            initial: Double(intVal(key)),
            min: min,
            max: max,
            precision: 0,
            commit: { newValue in commit(key, NSNumber(value: Int(newValue))) }
        )
        .frame(maxWidth: 90, alignment: .trailing)
    }

    @ViewBuilder
    private func fileRow(key: String, types: [UTType]) -> some View {
        let path = summary[key] as? String ?? ""
        let label = path.isEmpty ? "—" : (path as NSString).lastPathComponent
        HStack(spacing: 6) {
            Text(label)
                .foregroundStyle(.secondary)
                .lineLimit(1)
                .truncationMode(.middle)
                .frame(maxWidth: 110, alignment: .trailing)
            Button {
                onPickFile(key, types)
            } label: {
                Image(systemName: "folder")
                    .font(.caption)
            }
            .buttonStyle(.plain)
            if !path.isEmpty {
                Button(role: .destructive) {
                    commit(key, "" as NSString)
                } label: {
                    Image(systemName: "xmark.circle")
                        .font(.caption)
                }
                .buttonStyle(.plain)
            }
        }
    }

    private var gridColorPicker: some View {
        let hex = summary["gridColor"] as? String ?? ""
        let parsed = Color(hexString: hex) ?? .green
        return HStack(spacing: 6) {
            ColorPicker("", selection: Binding(
                get: { parsed },
                set: { newColor in
                    commit("gridColor", newColor.toHexString() as NSString)
                }
            ), supportsOpacity: false)
                .labelsHidden()
            Text(hex.isEmpty ? "—" : hex)
                .font(.caption2)
                .foregroundStyle(.secondary)
                .frame(maxWidth: 80, alignment: .leading)
        }
    }

    private var unitsPicker: some View {
        // Index → label mapping from RulerObject.h:
        //   0=m, 1=cm, 2=mm, 3=yd, 4=ft, 5=in
        let opts = (summary["unitOptions"] as? [String]) ?? ["m", "cm", "mm", "yd", "ft", "in"]
        let idx = intVal("units")
        return Menu {
            ForEach(Array(opts.enumerated()), id: \.offset) { i, label in
                Button {
                    commit("units", NSNumber(value: i))
                } label: {
                    HStack {
                        Text(label)
                        if i == idx {
                            Spacer()
                            Image(systemName: "checkmark")
                        }
                    }
                }
            }
        } label: {
            Text(idx >= 0 && idx < opts.count ? opts[idx] : "—")
                .truncationMode(.middle)
                .frame(maxWidth: 80, alignment: .trailing)
        }
        .menuStyle(.button)
        .controlSize(.mini)
    }

    private func boolBinding(_ key: String) -> Binding<Bool> {
        Binding(
            get: { summary[key] as? Bool ?? false },
            set: { commit(key, NSNumber(value: $0)) }
        )
    }

    private var layoutGroupPicker: some View {
        Menu {
            ForEach(layoutGroups, id: \.self) { name in
                Button {
                    commit("layoutGroup", name as NSString)
                } label: {
                    HStack {
                        Text(name)
                        if name == (summary["layoutGroup"] as? String) {
                            Spacer()
                            Image(systemName: "checkmark")
                        }
                    }
                }
            }
        } label: {
            Text(summary["layoutGroup"] as? String ?? "Default")
                .truncationMode(.middle)
        }
        .menuStyle(.button)
        .controlSize(.mini)
    }

    private func doubleVal(_ key: String) -> Double {
        (summary[key] as? NSNumber)?.doubleValue ?? 0.0
    }
    private func intVal(_ key: String) -> Int {
        (summary[key] as? NSNumber)?.intValue ?? 0
    }

    // J-12 — UTType lists for the file pickers. `.image` covers
    // most user-friendly bitmap formats; mesh is intentionally
    // narrow (xLights only loads `.obj`).
    private var imageTypes: [UTType] {
        [.png, .jpeg, .tiff, .bmp, .gif, .image]
    }
    private var meshTypes: [UTType] {
        // No native UTType for OBJ — use a filename-extension type.
        [UTType(filenameExtension: "obj") ?? .data]
    }
}

/// Inline double-typed text field. Re-creates state from the
/// `initial` value whenever `id` changes (so a token bump elsewhere
/// repaints us) but otherwise lets the user keep typing without
/// fighting an outside writer.
private struct LayoutEditorDoubleField: View {
    let id: String
    let initial: Double
    let min: Double?
    /// J-6 — optional clamps + precision; backwards compatible
    /// with call sites that only set `min`.
    var max: Double? = nil
    var precision: Int = 2
    let commit: (Double) -> Void

    @State private var draft: String = ""
    @FocusState private var focused: Bool

    var body: some View {
        TextField("", text: $draft)
            .multilineTextAlignment(.trailing)
            .keyboardType(.numbersAndPunctuation)
            .textFieldStyle(.roundedBorder)
            .focused($focused)
            .id(id)
            .onAppear { draft = format(initial) }
            .onChange(of: id) { _, _ in
                if !focused { draft = format(initial) }
            }
            .onChange(of: focused) { _, nowFocused in
                if !nowFocused { commitDraft() }
            }
            .onSubmit { commitDraft() }
    }

    private func commitDraft() {
        let trimmed = draft.trimmingCharacters(in: .whitespaces)
        guard let parsed = Double(trimmed) else {
            draft = format(initial)
            return
        }
        var v = parsed
        if let lo = min, v < lo { v = lo }
        if let hi = max, v > hi { v = hi }
        commit(v)
    }

    private func format(_ v: Double) -> String {
        // 2 decimals matches desktop's wxPropertyGrid default;
        // overridden via `precision` for int fields (0) or higher-
        // precision floats.
        String(format: "%.\(Swift.max(0, precision))f", v)
    }
}

/// J-20.3 — Editable spin button. Compose a numeric TextField
/// with a SwiftUI Stepper +/-: shows the current value, lets the
/// user type a new one, and supports ±-by-step taps. Replaces
/// the J-20 "Stepper-only" rows that hid the value when
/// `.labelsHidden()` suppressed the label closure.
///
/// Two variants — `Int` and `Double` — so the on-screen text
/// matches the underlying type (no spurious decimal for ints,
/// configurable precision + step for floats).
private struct LayoutEditorIntSpin: View {
    let id: String
    let initial: Int
    let range: ClosedRange<Int>
    var step: Int = 1
    let commit: (Int) -> Void

    @State private var draft: String = ""
    @FocusState private var focused: Bool

    var body: some View {
        HStack(spacing: 4) {
            TextField("", text: $draft)
                .multilineTextAlignment(.trailing)
                .keyboardType(.numbersAndPunctuation)
                .textFieldStyle(.roundedBorder)
                // J-23.7 — bumped from 70 to 95 so 4-digit
                // values (e.g. matrix dims, channel counts) fit
                // without being clipped.
                .frame(maxWidth: 95)
                .focused($focused)
                .id(id)
                .onAppear { draft = "\(initial)" }
                .onChange(of: id) { _, _ in
                    if !focused { draft = "\(initial)" }
                }
                .onChange(of: initial) { _, newValue in
                    if !focused { draft = "\(newValue)" }
                }
                .onChange(of: focused) { _, nowFocused in
                    if !nowFocused { commitDraft() }
                }
                .onSubmit { commitDraft() }
            Stepper("",
                    value: Binding<Int>(
                        get: { currentValue() },
                        set: { newVal in
                            let clamped = clamp(newVal)
                            draft = "\(clamped)"
                            commit(clamped)
                        }
                    ),
                    in: range,
                    step: step)
            .labelsHidden()
            .controlSize(.mini)
        }
    }

    private func currentValue() -> Int {
        if let v = Int(draft.trimmingCharacters(in: .whitespaces)) {
            return clamp(v)
        }
        return initial
    }
    private func clamp(_ v: Int) -> Int {
        Swift.max(range.lowerBound, Swift.min(range.upperBound, v))
    }
    private func commitDraft() {
        guard let parsed = Int(draft.trimmingCharacters(in: .whitespaces)) else {
            draft = "\(initial)"
            return
        }
        let v = clamp(parsed)
        if "\(v)" != draft { draft = "\(v)" }
        commit(v)
    }
}

private struct LayoutEditorDoubleSpin: View {
    let id: String
    let initial: Double
    let range: ClosedRange<Double>
    var step: Double = 0.1
    var precision: Int = 2
    let commit: (Double) -> Void

    @State private var draft: String = ""
    @FocusState private var focused: Bool

    var body: some View {
        HStack(spacing: 4) {
            TextField("", text: $draft)
                .multilineTextAlignment(.trailing)
                .keyboardType(.numbersAndPunctuation)
                .textFieldStyle(.roundedBorder)
                .frame(maxWidth: 80)
                .focused($focused)
                .id(id)
                .onAppear { draft = format(initial) }
                .onChange(of: id) { _, _ in
                    if !focused { draft = format(initial) }
                }
                .onChange(of: initial) { _, newValue in
                    if !focused { draft = format(newValue) }
                }
                .onChange(of: focused) { _, nowFocused in
                    if !nowFocused { commitDraft() }
                }
                .onSubmit { commitDraft() }
            Stepper("",
                    value: Binding<Double>(
                        get: { currentValue() },
                        set: { newVal in
                            let clamped = clamp(newVal)
                            draft = format(clamped)
                            commit(clamped)
                        }
                    ),
                    in: range,
                    step: step)
            .labelsHidden()
            .controlSize(.mini)
        }
    }

    private func currentValue() -> Double {
        if let v = Double(draft.trimmingCharacters(in: .whitespaces)) {
            return clamp(v)
        }
        return initial
    }
    private func clamp(_ v: Double) -> Double {
        Swift.max(range.lowerBound, Swift.min(range.upperBound, v))
    }
    private func commitDraft() {
        guard let parsed = Double(draft.trimmingCharacters(in: .whitespaces)) else {
            draft = format(initial)
            return
        }
        let v = clamp(parsed)
        if format(v) != draft { draft = format(v) }
        commit(v)
    }
    private func format(_ v: Double) -> String {
        String(format: "%.\(Swift.max(0, precision))f", v)
    }
}

/// Inline string field for controller name, etc. Mirrors
/// `LayoutEditorDoubleField` but with no parse / clamp.
private struct LayoutEditorStringField: View {
    let id: String
    let initial: String
    let commit: (String) -> Void

    @State private var draft: String = ""
    @FocusState private var focused: Bool

    var body: some View {
        TextField("", text: $draft)
            .multilineTextAlignment(.trailing)
            .textFieldStyle(.roundedBorder)
            .focused($focused)
            .id(id)
            .onAppear { draft = initial }
            .onChange(of: id) { _, _ in
                if !focused { draft = initial }
            }
            .onChange(of: focused) { _, nowFocused in
                if !nowFocused { commit(draft) }
            }
            .onSubmit { commit(draft) }
    }
}

/// Canvas-side toggles: 2D/3D, view-objects, fit-all/fit-selected,
/// camera reset. Subset of `PreviewControlsOverlay` — the Layout
/// Editor doesn't need viewpoints, save-image, or detach (no
/// embedded counterpart to detach from).
// Phase J-2 (touch UX) — floating action bar anchored above the
// selected model. Queries the bridge each animation frame so it
// tracks pan / zoom / orbit / drag. Hides when the model is off-
// screen.
//
// First-cut actions: Lock toggle + a "deselect" affordance. Add
// Duplicate / Delete once the bridge supports them.
// Phase J-3 (touch UX) — model-type picker. A sheet (rather than
// an inline Menu) so the list of 18 types is comfortable to
// browse on touch and presents a familiar iOS modal model.
// Cancel via swipe-down or tap outside; selection dismisses
// automatically.
private struct AddModelSheet: View {
    let types: [String]
    let labelFor: (String) -> String
    let onSelect: (String) -> Void
    @Environment(\.dismiss) private var dismiss

    var body: some View {
        NavigationStack {
            List(types, id: \.self) { type in
                Button {
                    onSelect(type)
                } label: {
                    HStack {
                        Image(systemName: "plus.rectangle.on.rectangle")
                            .foregroundStyle(.secondary)
                        Text(labelFor(type))
                            .foregroundStyle(.primary)
                    }
                }
            }
            .navigationTitle("Add Model")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                }
            }
        }
        .presentationDetents([.medium, .large])
    }
}

/// J-7 (group CRUD) — name-only sheet for creating a fresh
/// ModelGroup in the active layout group. Member list is populated
/// from the property pane after creation (one-step-at-a-time UX is
/// less error-prone than a sheet with two distinct sections).
private struct NewGroupSheet: View {
    let existingNames: Set<String>
    let onCreate: (String) -> Void
    let onCancel: () -> Void
    /// J-16 — sanitizer lookup (wraps `Model::SafeModelName` via
    /// the bridge). Lets the sheet show a live preview of what
    /// illegal characters will be stripped on commit.
    let sanitize: (String) -> String
    @Environment(\.dismiss) private var dismiss
    @State private var name: String = ""
    @FocusState private var nameFocused: Bool

    private var trimmedName: String {
        name.trimmingCharacters(in: .whitespacesAndNewlines)
    }
    private var sanitizedName: String { sanitize(trimmedName) }
    private var nameChanged: Bool { sanitizedName != trimmedName }
    private var collision: Bool {
        !sanitizedName.isEmpty && existingNames.contains(sanitizedName)
    }
    private var canCreate: Bool {
        !sanitizedName.isEmpty && !collision
    }

    var body: some View {
        NavigationStack {
            Form {
                Section("Name") {
                    TextField("Group name", text: $name)
                        .focused($nameFocused)
                        .autocorrectionDisabled()
                        .textInputAutocapitalization(.words)
                }
                if nameChanged && !sanitizedName.isEmpty {
                    Section {
                        Label("Will save as \"\(sanitizedName)\"",
                               systemImage: "info.circle")
                            .foregroundStyle(.blue)
                        Text("These characters can't appear in group names: , ~ ! ; < > \" ' & : | @ / \\")
                            .font(.caption2)
                            .foregroundStyle(.secondary)
                    }
                }
                if collision {
                    Section {
                        Label("\"\(sanitizedName)\" is already in use",
                               systemImage: "exclamationmark.triangle.fill")
                            .foregroundStyle(.orange)
                    }
                }
                Section {
                    Text("The new group lands in the active layout group with empty members. Add models from the property pane after it's created.")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
            }
            .navigationTitle("New Group")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { onCancel() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Create") {
                        onCreate(sanitizedName)
                    }
                    .disabled(!canCreate)
                }
            }
            .onAppear { nameFocused = true }
        }
        .presentationDetents([.medium])
    }
}

/// J-7 → J-9 (group CRUD) — multi-select member picker with
/// submodel tree. Top-level rows are models / groups; tapping the
/// chevron expands a row to surface that model's submodels
/// (full "Parent/Sub" names). Tap-to-toggle works at any level
/// so the user can pick "Arch1" or specifically "Arch1/Inner".
/// Existing members of the target group are pre-filtered out at
/// the call site.
private struct AddMemberSheet: View {
    let groupName: String
    /// Top-level model / group names (no submodels yet).
    let candidates: [String]
    /// Names of submodels already-in-the-group. The sheet
    /// dims / hides these inside the tree so the user can't
    /// re-add them.
    let existingMembers: Set<String>
    /// Async submodel lookup: returns the "Parent/Sub" full
    /// names for one parent. Called lazily when the user
    /// expands a row.
    let submodelsFor: (String) -> [String]
    let onAdd: ([String]) -> Void
    let onCancel: () -> Void

    @State private var picked: Set<String> = []
    @State private var expanded: Set<String> = []
    @State private var filter: String = ""

    /// Filtered top-level candidates. Match is OR across model
    /// name and any submodel name (so searching "Inner" surfaces
    /// the parent even if its submodel matches, like the vendor
    /// search does).
    private var filteredCandidates: [String] {
        let q = filter.trimmingCharacters(in: .whitespaces)
        guard !q.isEmpty else { return candidates }
        return candidates.filter { name in
            if name.localizedCaseInsensitiveContains(q) { return true }
            return submodelsFor(name).contains(where: {
                $0.localizedCaseInsensitiveContains(q)
            })
        }
    }

    var body: some View {
        NavigationStack {
            if candidates.isEmpty {
                ContentUnavailableView {
                    Label("Nothing left to add", systemImage: "checkmark.circle")
                } description: {
                    Text("This group already contains every available model.")
                }
                .navigationTitle("Add to \(groupName)")
                .navigationBarTitleDisplayMode(.inline)
                .toolbar {
                    ToolbarItem(placement: .cancellationAction) {
                        Button("Done") { onCancel() }
                    }
                }
            } else {
                List {
                    ForEach(filteredCandidates, id: \.self) { name in
                        AddMemberRow(
                            name: name,
                            picked: $picked,
                            expanded: $expanded,
                            existingMembers: existingMembers,
                            submodelsFor: submodelsFor
                        )
                    }
                }
                .listStyle(.plain)
                .searchable(text: $filter,
                            placement: .navigationBarDrawer(displayMode: .always),
                            prompt: "Filter models or submodels")
                .navigationTitle("Add to \(groupName)")
                .navigationBarTitleDisplayMode(.inline)
                .toolbar {
                    ToolbarItem(placement: .cancellationAction) {
                        Button("Cancel") { onCancel() }
                    }
                    ToolbarItem(placement: .confirmationAction) {
                        Button("Add\(picked.isEmpty ? "" : " \(picked.count)")") {
                            onAdd(Array(picked))
                        }
                        .disabled(picked.isEmpty)
                    }
                }
            }
        }
        .presentationDetents([.medium, .large])
    }
}

/// J-9 — single row in AddMemberSheet. Handles the chevron-toggle
/// for expanding submodels and the tap-to-pick gesture for the
/// row itself (model or submodel).
private struct AddMemberRow: View {
    let name: String
    @Binding var picked: Set<String>
    @Binding var expanded: Set<String>
    let existingMembers: Set<String>
    let submodelsFor: (String) -> [String]

    private var submodels: [String] { submodelsFor(name) }
    private var hasSubmodels: Bool { !submodels.isEmpty }
    private var isExpanded: Bool { expanded.contains(name) }
    private var isPicked: Bool { picked.contains(name) }

    var body: some View {
        VStack(alignment: .leading, spacing: 0) {
            row(name: name,
                indent: 0,
                showChevron: hasSubmodels,
                isExpanded: isExpanded,
                isPicked: isPicked,
                already: false,
                onChevron: {
                    if isExpanded {
                        expanded.remove(name)
                    } else {
                        expanded.insert(name)
                    }
                },
                onPick: { toggle(name) })

            if isExpanded {
                ForEach(submodels, id: \.self) { sub in
                    let already = existingMembers.contains(sub)
                    row(name: sub,
                        indent: 1,
                        showChevron: false,
                        isExpanded: false,
                        isPicked: picked.contains(sub),
                        already: already,
                        onChevron: {},
                        onPick: { if !already { toggle(sub) } })
                }
            }
        }
    }

    private func toggle(_ n: String) {
        if picked.contains(n) {
            picked.remove(n)
        } else {
            picked.insert(n)
        }
    }

    @ViewBuilder
    private func row(name: String,
                      indent: Int,
                      showChevron: Bool,
                      isExpanded: Bool,
                      isPicked: Bool,
                      already: Bool,
                      onChevron: @escaping () -> Void,
                      onPick: @escaping () -> Void) -> some View {
        HStack(spacing: 8) {
            if indent > 0 {
                Spacer().frame(width: CGFloat(indent) * 18)
            }
            if showChevron {
                Button {
                    onChevron()
                } label: {
                    Image(systemName: isExpanded ? "chevron.down" : "chevron.right")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                        .frame(width: 16)
                }
                .buttonStyle(.plain)
            } else {
                Spacer().frame(width: 16)
            }
            Button {
                onPick()
            } label: {
                HStack(spacing: 8) {
                    Image(systemName: isPicked
                           ? "checkmark.circle.fill"
                           : (already ? "checkmark.circle" : "circle"))
                        .foregroundStyle(already
                                          ? AnyShapeStyle(.secondary)
                                          : (isPicked
                                              ? AnyShapeStyle(Color.accentColor)
                                              : AnyShapeStyle(.secondary)))
                    Image(systemName: name.contains("/") ? "square.on.square" : "cube")
                        .foregroundStyle(.tertiary)
                    Text(displayName(name))
                        .lineLimit(1)
                        .truncationMode(.middle)
                        .foregroundStyle(already ? .secondary : .primary)
                    if already {
                        Text("(already a member)")
                            .font(.caption2)
                            .foregroundStyle(.tertiary)
                    }
                    Spacer()
                }
            }
            .buttonStyle(.plain)
            .disabled(already)
        }
        .padding(.vertical, 4)
    }

    private func displayName(_ n: String) -> String {
        // Submodel rows are indented under their parent — show
        // just the trailing portion to keep visual hierarchy clean.
        if let slash = n.firstIndex(of: "/") {
            return String(n[n.index(after: slash)...])
        }
        return n
    }
}

/// Phase J-4 (multi-select) — operations bar shown when 2+ models
/// are selected. Hosts Align ▾, Distribute ▾, Match Size ▾, and
/// Clear. Top-centered like the creation banner.
private struct MultiSelectActionBar: View {
    let selection: Set<String>
    let leader: String?
    let onAlign: (String) -> Void
    let onDistribute: (String) -> Void
    let onMatchSize: (String) -> Void
    let onFlip: (String) -> Void
    let onDuplicate: () -> Void
    let onGroup: () -> Void
    let onClear: () -> Void

    private var canDistribute: Bool { selection.count >= 3 }

    var body: some View {
        HStack(spacing: 10) {
            // Count + leader hint. Leader is shown so the user
            // knows which model the align/match ops target.
            VStack(alignment: .leading, spacing: 0) {
                Text("\(selection.count) selected")
                    .font(.caption.weight(.semibold))
                    .foregroundStyle(.white)
                if let leader = leader, !leader.isEmpty {
                    Text("Leader: \(leader)")
                        .font(.caption2)
                        .foregroundStyle(.white.opacity(0.7))
                        .lineLimit(1)
                        .truncationMode(.middle)
                        .frame(maxWidth: 180, alignment: .leading)
                }
            }

            Divider()
                .frame(height: 24)
                .background(.white.opacity(0.3))

            Menu {
                Button("Align Left")   { onAlign("left") }
                Button("Align Right")  { onAlign("right") }
                Button("Align Top")    { onAlign("top") }
                Button("Align Bottom") { onAlign("bottom") }
                Divider()
                Button("Center Horizontal") { onAlign("centerH") }
                Button("Center Vertical")   { onAlign("centerV") }
                Button("Center Depth")      { onAlign("centerD") }
            } label: {
                Label("Align", systemImage: "align.horizontal.left")
                    .font(.caption.weight(.medium))
            }
            .menuStyle(.borderlessButton)
            .foregroundStyle(.white)

            Menu {
                Button("Distribute Horizontally") { onDistribute("horizontal") }
                Button("Distribute Vertically")   { onDistribute("vertical") }
                Button("Distribute Depth")        { onDistribute("depth") }
            } label: {
                Label("Distribute", systemImage: "rectangle.split.3x1")
                    .font(.caption.weight(.medium))
            }
            .menuStyle(.borderlessButton)
            .foregroundStyle(canDistribute ? .white : .white.opacity(0.4))
            .disabled(!canDistribute)

            Menu {
                Button("Same Width")  { onMatchSize("width") }
                Button("Same Height") { onMatchSize("height") }
                Button("Same Depth")  { onMatchSize("depth") }
                Divider()
                Button("Match All")   { onMatchSize("all") }
            } label: {
                Label("Match Size", systemImage: "rectangle.expand.vertical")
                    .font(.caption.weight(.medium))
            }
            .menuStyle(.borderlessButton)
            .foregroundStyle(.white)

            Menu {
                Button("Flip Horizontal") { onFlip("horizontal") }
                Button("Flip Vertical")   { onFlip("vertical") }
            } label: {
                Label("Flip", systemImage: "arrow.left.and.right.righttriangle.left.righttriangle.right")
                    .font(.caption.weight(.medium))
            }
            .menuStyle(.borderlessButton)
            .foregroundStyle(.white)

            Divider()
                .frame(height: 24)
                .background(.white.opacity(0.3))

            Button {
                onDuplicate()
            } label: {
                Label("Duplicate", systemImage: "plus.square.on.square")
                    .font(.caption.weight(.medium))
            }
            .buttonStyle(.plain)
            .foregroundStyle(.white)

            Button {
                onGroup()
            } label: {
                Label("Group", systemImage: "square.stack.3d.up")
                    .font(.caption.weight(.medium))
            }
            .buttonStyle(.plain)
            .foregroundStyle(.white)

            Divider()
                .frame(height: 24)
                .background(.white.opacity(0.3))

            Button {
                onClear()
            } label: {
                Image(systemName: "xmark.circle.fill")
                    .font(.body)
                    .foregroundStyle(.white.opacity(0.8))
            }
            .buttonStyle(.plain)
        }
        .padding(.horizontal, 14)
        .padding(.vertical, 8)
        .background(.black.opacity(0.55), in: Capsule())
        .shadow(radius: 3, y: 2)
    }
}

private struct InlineModelActionBar: View {
    @Environment(SequencerViewModel.self) var viewModel
    let modelName: String
    let summaryToken: Int
    let onPropertyChange: () -> Void
    let onRequestDelete: () -> Void
    let onRequestDuplicate: () -> Void

    var body: some View {
        // `TimelineView(.animation)` refreshes its content every
        // CADisplayLink tick — exactly when we want to recompute
        // the screen anchor (matches Metal redraw cadence).
        // GeometryReader gives us the canvas height for clamping
        // the bottom-anchored bar above the viewport's lower edge.
        GeometryReader { geo in
            TimelineView(.animation) { _ in
                anchoredBar(canvasHeight: geo.size.height)
            }
        }
    }

    private var locked: Bool {
        guard let summary = viewModel.document.modelLayoutSummary(modelName) else {
            return false
        }
        return (summary["locked"] as? NSNumber)?.boolValue ?? false
    }

    @ViewBuilder
    private func anchoredBar(canvasHeight: CGFloat) -> some View {
        if let value = XLightsBridgeBox.bridgeForLayoutEditor()?
                                       .screenAnchorPoint(forModel: modelName,
                                                          for: viewModel.document) {
            let anchor = value.cgPointValue
            HStack(spacing: 6) {
                Button {
                    let newLocked = !locked
                    _ = viewModel.document.setLayoutModelProperty(
                        modelName, key: "locked",
                        value: NSNumber(value: newLocked))
                    onPropertyChange()
                } label: {
                    Label(locked ? "Locked" : "Unlocked",
                          systemImage: locked ? "lock.fill" : "lock.open")
                        .labelStyle(.titleAndIcon)
                        .font(.caption2.weight(.medium))
                }
                .buttonStyle(.borderedProminent)
                .tint(locked ? .red.opacity(0.85) : .blue.opacity(0.85))
                .controlSize(.mini)

                Text(modelName)
                    .font(.caption2.weight(.semibold))
                    .lineLimit(1)
                    .truncationMode(.middle)
                    .frame(maxWidth: 140)

                Button {
                    // Fit Selected — reuses the existing notification
                    // the canvas controls overlay uses for its own
                    // viewfinder button. Quick "where is this model?"
                    // affordance for users whose canvas is panned.
                    NotificationCenter.default.post(name: .previewFitModel,
                                                     object: "LayoutEditor",
                                                     userInfo: ["name": modelName])
                } label: {
                    Image(systemName: "viewfinder")
                        .font(.caption)
                }
                .buttonStyle(.plain)
                .foregroundStyle(.secondary)

                Button {
                    onRequestDuplicate()
                } label: {
                    Image(systemName: "plus.square.on.square")
                        .font(.caption)
                }
                .buttonStyle(.plain)
                .foregroundStyle(.blue)

                Button(role: .destructive) {
                    onRequestDelete()
                } label: {
                    Image(systemName: "trash")
                        .font(.caption)
                }
                .buttonStyle(.plain)
                .foregroundStyle(.red)

                Button {
                    viewModel.layoutSelectSingle(nil)
                } label: {
                    Image(systemName: "xmark.circle.fill")
                        .font(.body)
                }
                .buttonStyle(.plain)
                .foregroundStyle(.secondary)
            }
            .padding(.horizontal, 8)
            .padding(.vertical, 4)
            .background(.regularMaterial, in: Capsule())
            .shadow(radius: 2, y: 1)
            // Position the bar BELOW the model's screen-bottom
            // anchor with a small offset. Bottom-anchor avoids
            // overlap with the gizmo handles (Y axis arrow, rotate
            // ring, shear puck) which all live at or above the
            // model's top edge. Reads the GeometryReader's height
            // to clamp the bar above the canvas's bottom edge so
            // it stays visible when the model is near the bottom.
            .position(x: anchor.x,
                       y: min(canvasHeight - 28, anchor.y + 30))
            .transition(.opacity)
        }
    }
}

// Phase J-2 (touch UX) — model-name label overlay. Renders one
// small Text view per on-screen model at its projected centre.
// The bridge does a single batched query each frame returning
// `[(name, anchor)]`; off-screen / behind-camera models are
// filtered out at the bridge so SwiftUI never sees them.
//
// Cost: ~one bridge call + N Text views per frame. Cheap for
// typical 10–50 model shows; verify with a 200+ model show if
// needed.
private struct ModelLabelsOverlay: View {
    @Environment(SequencerViewModel.self) var viewModel

    var body: some View {
        TimelineView(.animation(minimumInterval: 1.0 / 30.0)) { _ in
            content
        }
    }

    @ViewBuilder
    private var content: some View {
        if let bridge = XLightsBridgeBox.bridgeForLayoutEditor() {
            let anchors = bridge.modelLabelAnchors(for: viewModel.document)
            ForEach(0..<anchors.count, id: \.self) { i in
                let entry = anchors[i]
                if let name = entry["name"] as? String,
                   let value = entry["anchor"] as? NSValue {
                    let p = value.cgPointValue
                    Text(name)
                        .font(.caption2.weight(.medium))
                        .foregroundStyle(.white)
                        .padding(.horizontal, 4)
                        .padding(.vertical, 1)
                        .background(Color.black.opacity(0.45),
                                    in: RoundedRectangle(cornerRadius: 3))
                        .fixedSize()
                        .position(x: p.x, y: p.y)
                }
            }
        }
    }
}

/// Bridge resolver. The XLMetalBridge instance is created inside
/// PreviewPaneView's coordinator; it doesn't live on the view
/// model. Maintain a tiny registry keyed by preview name so
/// detached views (e.g. the inline action bar) can reach the
/// active bridge without an explicit injection.
@MainActor
enum XLightsBridgeBox {
    private static var byName: [String: WeakBridge] = [:]
    private struct WeakBridge { weak var bridge: XLMetalBridge? }

    static func register(_ bridge: XLMetalBridge, forPreviewName name: String) {
        byName[name] = WeakBridge(bridge: bridge)
    }
    static func unregister(previewName name: String) {
        byName.removeValue(forKey: name)
    }
    static func bridgeForLayoutEditor() -> XLMetalBridge? {
        byName["LayoutEditor"]?.bridge
    }
}

// Phase J-2 (touch UX) — typed target for the long-press
// contextual menu. The `.confirmationDialog` switches on this to
// decide which buttons to show. Mirrors the keys produced by
// `XLMetalBridge.inspectHandleAtScreenPoint:`.
private enum LayoutContextMenuTarget: Equatable {
    case vertex(modelName: String, vertexIndex: Int)
    case segment(modelName: String, segmentIndex: Int, hasCurve: Bool)
    case curveControl(modelName: String, segmentIndex: Int)

    var title: String {
        switch self {
        case .vertex(_, let i):            return "Point \(i + 1)"
        case .segment(_, let i, _):        return "Segment \(i + 1)"
        case .curveControl(_, let i):      return "Curve on Segment \(i + 1)"
        }
    }
}

// Phase J-2 (touch UX) — bottom-anchored tool toolbar.
//
// Replaces desktop's CentreCycle ("tap the orange centre sphere
// to advance axis_tool") with a persistent picker so the active
// tool is always visible + reachable in 44pt touch targets.
// Plus persistent modifier toggles — Uniform / Lock Axis —
// that replace held-key modifiers from the desktop UI.
//
// See `plans/phase-j-touch-ux.md` for the design rationale.
private struct LayoutEditorToolToolbar: View {
    @Bindable var settings: PreviewSettings
    let selectedModelName: String
    let onToolChange: (String) -> Void

    var body: some View {
        HStack(spacing: 16) {
            toolPicker
            Divider().frame(height: 28)
            modifierToggles
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 8)
        .background(.regularMaterial, in: RoundedRectangle(cornerRadius: 12))
        .shadow(radius: 2, y: 1)
    }

    private var toolPicker: some View {
        // Radio-style. The third position (Scale) cycles through
        // XY_TRANS / Elevate when those make sense for the model
        // type, but the toolbar surface only exposes the three
        // common tools — XY_TRANS / Elevate are still reachable
        // via subclass auto-promotion in `SetActiveHandle`.
        HStack(spacing: 4) {
            toolButton(tool: "translate", label: "Move",  systemImage: "arrow.up.and.down.and.arrow.left.and.right")
            toolButton(tool: "rotate",    label: "Rotate", systemImage: "arrow.triangle.2.circlepath")
            toolButton(tool: "scale",     label: "Scale",  systemImage: "arrow.up.left.and.arrow.down.right")
        }
    }

    private func toolButton(tool: String, label: String, systemImage: String) -> some View {
        let isActive = settings.axisTool == tool
        return Button {
            settings.axisTool = tool
            // Free up axis-specific modifiers when leaving the
            // mode that consumes them, so a stale Lock Axis from
            // a previous tool doesn't haunt the next gesture.
            if tool == "rotate" {
                settings.lockAxis = 0
            }
            onToolChange(tool)
        } label: {
            VStack(spacing: 2) {
                Image(systemName: systemImage).font(.system(size: 18))
                Text(label).font(.caption2)
            }
            .frame(minWidth: 44, minHeight: 36)
            .padding(.horizontal, 4)
        }
        .buttonStyle(.plain)
        .background(isActive ? Color.accentColor.opacity(0.25) : Color.clear,
                    in: RoundedRectangle(cornerRadius: 6))
        .foregroundStyle(isActive ? Color.accentColor : Color.primary)
    }

    @ViewBuilder
    private var modifierToggles: some View {
        // Uniform — only meaningful in Scale mode (mirrors the
        // desktop convention where Shift = aspect-lock during a
        // resize). Hidden otherwise.
        if settings.axisTool == "scale" {
            Toggle(isOn: $settings.uniformModifier) {
                Label("Uniform", systemImage: "lock.fill")
                    .labelStyle(.titleAndIcon)
                    .font(.caption2)
            }
            .toggleStyle(.button)
            .controlSize(.small)
        }

        // Lock Axis — visible in Move + Scale (matches the desktop
        // axis-arrow constrain-to-axis semantics). Hidden in
        // Rotate where the axis is already implicit in the gizmo.
        if settings.axisTool == "translate" || settings.axisTool == "scale" {
            HStack(spacing: 2) {
                Text("Axis:").font(.caption2).foregroundStyle(.secondary)
                axisChip(label: "Free", value: 0)
                axisChip(label: "X",    value: 1)
                axisChip(label: "Y",    value: 2)
                axisChip(label: "Z",    value: 3)
            }
        }

        // Snap — visible in Move (only place where rounding-to-grid
        // matters). Mirrors the existing canvas-controls Snap
        // toggle so users have it without opening the gear menu.
        if settings.axisTool == "translate" {
            Toggle(isOn: $settings.snapToGrid) {
                Label("Snap", systemImage: "square.grid.3x3")
                    .labelStyle(.titleAndIcon)
                    .font(.caption2)
            }
            .toggleStyle(.button)
            .controlSize(.small)
        }
    }

    private func axisChip(label: String, value: Int) -> some View {
        let isActive = settings.lockAxis == value
        return Button {
            settings.lockAxis = value
        } label: {
            Text(label)
                .font(.caption2.weight(.medium))
                .frame(minWidth: 28, minHeight: 24)
        }
        .buttonStyle(.plain)
        .background(isActive ? Color.accentColor.opacity(0.3) : Color.gray.opacity(0.15),
                    in: RoundedRectangle(cornerRadius: 4))
        .foregroundStyle(isActive ? Color.accentColor : Color.primary)
    }
}

private struct LayoutEditorCanvasControls: View {
    let previewName: String
    @Bindable var settings: PreviewSettings
    let selectedModelName: String?

    var body: some View {
        VStack(alignment: .trailing, spacing: 4) {
            HStack(spacing: 4) {
                Button { post(.zoomOut) }   label: { Image(systemName: "minus.magnifyingglass") }
                Button { post(.zoomReset) } label: { Text("1×").font(.caption.monospacedDigit()) }
                Button { post(.zoomIn) }    label: { Image(systemName: "plus.magnifyingglass") }
                Button { post(.reset) }     label: { Image(systemName: "arrow.counterclockwise") }
            }
            .buttonStyle(.bordered)
            .controlSize(.small)

            HStack(spacing: 4) {
                Button {
                    NotificationCenter.default.post(name: .previewFitAll,
                                                    object: previewName)
                } label: {
                    Image(systemName: "arrow.up.left.and.arrow.down.right.rectangle")
                }
                Button {
                    guard let sel = selectedModelName, !sel.isEmpty else { return }
                    NotificationCenter.default.post(name: .previewFitModel,
                                                    object: previewName,
                                                    userInfo: ["name": sel])
                } label: {
                    Image(systemName: "viewfinder")
                }
                .disabled(selectedModelName?.isEmpty ?? true)
            }
            .buttonStyle(.bordered)
            .controlSize(.small)

            Picker("", selection: $settings.is3D) {
                Text("2D").tag(false)
                Text("3D").tag(true)
            }
            .pickerStyle(.segmented)
            .frame(width: 96)

            Toggle(isOn: $settings.showViewObjects) {
                Text("View Objs").font(.caption2)
            }
            .toggleStyle(.button)
            .controlSize(.small)

            // J-2 — overlay toggles. Hidden in 3D mode (overlays
            // currently 2D-only). Tinted "on" state matches the
            // sequencer's lightbulb output toggle for consistency.
            if !settings.is3D {
                Toggle(isOn: $settings.showLayoutGrid) {
                    Text("Grid").font(.caption2)
                }
                .toggleStyle(.button)
                .controlSize(.small)

                Toggle(isOn: $settings.showLayoutBoundingBox) {
                    Text("Bounds").font(.caption2)
                }
                .toggleStyle(.button)
                .controlSize(.small)

                Toggle(isOn: $settings.snapToGrid) {
                    Text("Snap").font(.caption2)
                }
                .toggleStyle(.button)
                .controlSize(.small)
            }

            // First-pixel highlight applies in 2D and 3D — keep
            // outside the 2D-only block above.
            Toggle(isOn: $settings.showFirstPixel) {
                Text("Pixel 1").font(.caption2)
            }
            .toggleStyle(.button)
            .controlSize(.small)

            // J-2 UX — model-name labels (SwiftUI overlay).
            // Lives in 2D and 3D; bridge filters off-screen
            // models from the per-frame label list.
            Toggle(isOn: $settings.showModelLabels) {
                Text("Labels").font(.caption2)
            }
            .toggleStyle(.button)
            .controlSize(.small)
        }
    }

    private enum Action {
        case zoomIn, zoomOut, zoomReset, reset
    }

    private func post(_ action: Action) {
        let name: Notification.Name
        switch action {
        case .zoomIn:    name = .previewZoomIn
        case .zoomOut:   name = .previewZoomOut
        case .zoomReset: name = .previewZoomReset
        case .reset:     name = .previewResetCamera
        }
        NotificationCenter.default.post(name: name, object: previewName)
    }
}

/// Scene root for the standalone Layout Editor window. Mirrors the
/// `DetachedHousePreviewRoot` pattern: a token check guards against
/// iPadOS auto-restoring this scene on launch (we want the user to
/// reopen it explicitly via the Tools menu, not have it pop up
/// behind the main sequencer window).
/// J-8 — Hex string ↔ SwiftUI Color helpers for the Tag Color
/// picker. Desktop stores tag colours as `#RRGGBB`; the picker
/// round-trips through these to keep the on-disk representation
/// unchanged. Returns nil for unparseable strings so the call
/// site can fall back to a sensible default (.black).
fileprivate extension Color {
    init?(hexString: String) {
        var s = hexString.trimmingCharacters(in: .whitespacesAndNewlines)
        if s.hasPrefix("#") { s.removeFirst() }
        guard s.count == 6 || s.count == 8,
              let v = UInt32(s, radix: 16) else { return nil }
        let r, g, b, a: Double
        if s.count == 8 {
            r = Double((v >> 24) & 0xff) / 255.0
            g = Double((v >> 16) & 0xff) / 255.0
            b = Double((v >>  8) & 0xff) / 255.0
            a = Double( v        & 0xff) / 255.0
        } else {
            r = Double((v >> 16) & 0xff) / 255.0
            g = Double((v >>  8) & 0xff) / 255.0
            b = Double( v        & 0xff) / 255.0
            a = 1.0
        }
        self.init(.sRGB, red: r, green: g, blue: b, opacity: a)
    }

    /// Emits `#RRGGBB`; alpha is dropped (desktop tag colours don't
    /// carry alpha and we don't want a round-trip to introduce one).
    ///
    /// Routes through the sRGB color space explicitly. `UIColor(self)
    /// .getRed` reads in the device color space — on Display-P3 iPads
    /// that mangles the exact hex the user typed (e.g. #FF0000 becomes
    /// something slightly off). Converting via CGColor → sRGB keeps the
    /// round-trip bit-exact.
    func toHexString() -> String {
        let ui = UIColor(self)
        let srgbCG = ui.cgColor.converted(to: CGColorSpace(name: CGColorSpace.sRGB)!,
                                           intent: .defaultIntent,
                                           options: nil)
        if let c = srgbCG?.components, c.count >= 3 {
            let ir = Int((max(0, min(1, c[0])) * 255).rounded())
            let ig = Int((max(0, min(1, c[1])) * 255).rounded())
            let ib = Int((max(0, min(1, c[2])) * 255).rounded())
            return String(format: "#%02X%02X%02X", ir, ig, ib)
        }
        // Fall back to device-space read if the conversion fails —
        // shouldn't happen for any color we hand out, but better
        // than crashing.
        var r: CGFloat = 0, g: CGFloat = 0, b: CGFloat = 0, a: CGFloat = 0
        ui.getRed(&r, green: &g, blue: &b, alpha: &a)
        let ir = Int((r * 255).rounded())
        let ig = Int((g * 255).rounded())
        let ib = Int((b * 255).rounded())
        return String(format: "#%02X%02X%02X", ir, ig, ib)
    }
}

struct LayoutEditorWindowRoot: View {
    @Environment(SequencerViewModel.self) var viewModel
    @Environment(\.dismissWindow) private var dismissWindow
    @Environment(\.openWindow) private var openWindow
    @State private var suppressed: Bool = false

    var body: some View {
        Group {
            if suppressed {
                Color.black
            } else {
                LayoutEditorView()
                    .frame(maxWidth: .infinity, maxHeight: .infinity)
            }
        }
        .frame(minWidth: 480, minHeight: 360)
        .navigationTitle("Edit Layout")
        .onAppear {
            if viewModel.pendingDetachTokens.remove("layout-editor") != nil {
                viewModel.layoutEditorOpen = true
            } else {
                // F-5 follow-up: when iPadOS picks an aux scene
                // as the "connecting" session (i.e. last-quit
                // with the layout editor open), the AppDelegate
                // has already destroyed the main `sequencer`
                // session. Dismissing ourselves with no main
                // window alive leaves the app at a black screen
                // with no controls. Open the sequencer first.
                suppressed = true
                DispatchQueue.main.async {
                    openWindow(id: "sequencer")
                    dismissWindow(id: "layout-editor")
                }
            }
        }
        .onDisappear {
            if !suppressed { viewModel.layoutEditorOpen = false }
        }
    }
}
