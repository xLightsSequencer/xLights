import SwiftUI
import UniformTypeIdentifiers

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

    /// J-4 (import) — UTTypes the .fileImporter accepts. Declared
    /// as a static so the SwiftUI type-checker can resolve the
    /// `[UTType]` literal in reasonable time (the `?? .data` chain
    /// inline tripped its budget).
    private static let importableModelTypes: [UTType] = {
        ["xmodel", "gdtf", "lff", "lpf"].compactMap {
            UTType(filenameExtension: $0)
        }
    }()

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

    // MARK: - Sidebar

    @ViewBuilder
    private var sidebar: some View {
        List(selection: bindingSelection) {
            Section("Models in \(activeLayoutGroup) (\(modelNames.count))") {
                ForEach(modelNames, id: \.self) { name in
                    Text(name)
                        .lineLimit(1)
                        .truncationMode(.middle)
                        .tag(name)
                }
            }

            if let selectedName = viewModel.layoutEditorSelectedModel,
               let summary = viewModel.document.modelLayoutSummary(selectedName) {
                Section("Selected Model") {
                    LayoutEditorPropertiesView(
                        modelName: selectedName,
                        summary: summary,
                        layoutGroups: layoutGroups,
                        token: summaryToken,
                        commit: { key, value in
                            commitProperty(modelName: selectedName,
                                           key: key, value: value)
                        }
                    )
                }
            }

            Section("Display") {
                let bg = (displayState["backgroundImage"] as? String) ?? ""
                LabeledContent("Background", value: bg.isEmpty
                               ? "—"
                               : (bg as NSString).lastPathComponent)
                LabeledContent("Canvas",
                               value: "\(displayState["previewWidth"] as? Int ?? 0) × \(displayState["previewHeight"] as? Int ?? 0)")
                LabeledContent("2D centre = 0",
                               value: (displayState["display2DCenter0"] as? Bool ?? false) ? "Yes" : "No")
                LabeledContent("Default mode",
                               value: (displayState["layoutMode3D"] as? Bool ?? true) ? "3D" : "2D")
                LabeledContent("Grid",
                               value: gridLabel)
                LabeledContent("Bounding box",
                               value: (displayState["display2DBoundingBox"] as? Bool ?? false) ? "On" : "Off")
            }
        }
        .listStyle(.sidebar)
    }

    private var gridLabel: String {
        let on = (displayState["display2DGrid"] as? Bool) ?? false
        let spacing = (displayState["display2DGridSpacing"] as? Int)
            ?? Int((displayState["display2DGridSpacing"] as? NSNumber)?.intValue ?? 100)
        return on ? "On (\(spacing))" : "Off"
    }

    /// Sidebar List uses Set<String?> via @Bindable, but we want
    /// single-select semantics. Wrap it manually.
    private var bindingSelection: Binding<String?> {
        Binding(
            get: { viewModel.layoutEditorSelectedModel },
            set: { viewModel.layoutSelectSingle($0) }
        )
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
        // If the previously-selected model isn't in the new list,
        // clear selection so the side panel doesn't show stale data.
        if let sel = viewModel.layoutEditorSelectedModel,
           !modelNames.contains(sel) {
            viewModel.layoutSelectSingle(nil)
        }
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

/// Editable properties panel for the selected model. Common-
/// properties surface only (J-1) — per-type properties (number of
/// arches, tree branches, custom-model matrix, DMX channel mapping)
/// land in J-3.
///
/// Each editable cell pushes its value through `commit(key, value)`
/// the moment it loses focus / commits, which routes through
/// `setLayoutModelProperty` and bumps the parent's `summaryToken`
/// so this view re-binds against the fresh summary. The on-disk
/// rgbeffects.xml is *not* updated until the parent calls
/// `saveLayoutChanges` (Save button in toolbar).
private struct LayoutEditorPropertiesView: View {
    let modelName: String
    let summary: [String: Any]
    let layoutGroups: [String]
    let token: Int
    let commit: (_ key: String, _ value: Any) -> Void

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            row("Name") { Text(modelName).truncationMode(.middle) }
            row("Type") { Text(summary["displayAs"] as? String ?? "—") }
            row("Layout group") { layoutGroupPicker }
            row("Locked") {
                Toggle("", isOn: lockedBinding)
                    .labelsHidden()
                    .controlSize(.mini)
            }

            Divider().padding(.vertical, 2)

            row("Centre X") { numberField(key: "centerX") }
            row("Centre Y") { numberField(key: "centerY") }
            row("Centre Z") { numberField(key: "centerZ") }

            Divider().padding(.vertical, 2)

            row("Width")  { numberField(key: "width",  min: 0) }
            row("Height") { numberField(key: "height", min: 0) }
            row("Depth")  { numberField(key: "depth",  min: 0) }

            Divider().padding(.vertical, 2)

            row("Rotate X") { numberField(key: "rotateX") }
            row("Rotate Y") { numberField(key: "rotateY") }
            row("Rotate Z") { numberField(key: "rotateZ") }

            Divider().padding(.vertical, 2)

            row("Controller") { controllerField }
            row("Channel range") {
                Text("\(uintVal("startChannel"))..\(uintVal("endChannel"))")
                    .foregroundStyle(.secondary)
            }
            row("Strings") {
                Text("\(intVal("stringCount"))").foregroundStyle(.secondary)
            }
            row("Nodes") {
                Text("\(uintVal("nodeCount"))").foregroundStyle(.secondary)
            }
        }
        .font(.caption.monospacedDigit())
    }

    // MARK: - Cells

    @ViewBuilder
    private func row(_ label: String, @ViewBuilder _ content: () -> some View) -> some View {
        HStack(alignment: .firstTextBaseline) {
            Text(label)
                .foregroundStyle(.secondary)
                .frame(minWidth: 90, alignment: .leading)
            Spacer(minLength: 8)
            content()
                .lineLimit(1)
        }
    }

    private func numberField(key: String, min: Double? = nil) -> some View {
        LayoutEditorDoubleField(
            // Reset edits when token bumps (after a commit elsewhere)
            // so the field re-reads the bridge value.
            id: "\(modelName).\(key).\(token)",
            initial: doubleVal(key),
            min: min,
            commit: { newValue in commit(key, NSNumber(value: newValue)) }
        )
        .frame(maxWidth: 110, alignment: .trailing)
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

    private var lockedBinding: Binding<Bool> {
        Binding(
            get: { summary["locked"] as? Bool ?? false },
            set: { commit("locked", NSNumber(value: $0)) }
        )
    }

    private var controllerField: some View {
        LayoutEditorStringField(
            id: "\(modelName).controllerName.\(token)",
            initial: summary["controllerName"] as? String ?? "",
            commit: { commit("controllerName", $0 as NSString) }
        )
        .frame(maxWidth: 140, alignment: .trailing)
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

/// Inline double-typed text field. Re-creates state from the
/// `initial` value whenever `id` changes (so a token bump elsewhere
/// repaints us) but otherwise lets the user keep typing without
/// fighting an outside writer.
private struct LayoutEditorDoubleField: View {
    let id: String
    let initial: Double
    let min: Double?
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
        commit(v)
    }

    private func format(_ v: Double) -> String {
        // 2 decimals matches desktop's wxPropertyGrid display
        // precision for layout properties.
        String(format: "%.2f", v)
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

/// Phase J-4 (multi-select) — operations bar shown when 2+ models
/// are selected. Hosts Align ▾, Distribute ▾, Match Size ▾, and
/// Clear. Top-centered like the creation banner.
private struct MultiSelectActionBar: View {
    let selection: Set<String>
    let leader: String?
    let onAlign: (String) -> Void
    let onDistribute: (String) -> Void
    let onMatchSize: (String) -> Void
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
struct LayoutEditorWindowRoot: View {
    @Environment(SequencerViewModel.self) var viewModel
    @Environment(\.dismissWindow) private var dismissWindow
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
                suppressed = true
                DispatchQueue.main.async {
                    dismissWindow(id: "layout-editor")
                }
            }
        }
        .onDisappear {
            if !suppressed { viewModel.layoutEditorOpen = false }
        }
    }
}
