import SwiftUI

/// Per-preview appearance / camera-mode state shared between the pane and
/// its controls overlay. Kept here (rather than inside the bridge) so the
/// SwiftUI overlay can render toggle state without round-tripping through
/// ObjC each frame; PreviewPaneView syncs the relevant bits into the
/// bridge in updateUIView.
@Observable @MainActor
final class PreviewSettings {
    var is3D: Bool
    var showViewObjects: Bool

    init(is3DDefault: Bool, showViewObjectsDefault: Bool) {
        self.is3D = is3DDefault
        self.showViewObjects = showViewObjectsDefault
    }
}

/// House Preview — shows every model plus view objects.
struct HousePreviewView: View {
    @Environment(SequencerViewModel.self) var viewModel
    @State private var controlsVisible: Bool = false
    @State private var settings = PreviewSettings(is3DDefault: true,
                                                  showViewObjectsDefault: true)
    // Layout-group list and active-group are cached in @State because
    // the C++ render context is loaded in-place on the same document
    // instance SwiftUI already holds. The document reference never
    // changes, so reading `document.layoutGroups()` directly from
    // `body` wouldn't trigger a re-render after load. We refresh
    // explicitly via .onAppear + isShowFolderLoaded.
    @State private var layoutGroups: [String] = ["Default"]
    @State private var activeLayoutGroup: String = "Default"

    var body: some View {
        // Pass nil for previewModelName — House Preview must draw
        // every model, not single-model mode. `selectedModelName` is
        // the independent signal that drives "Fit Selected".
        PreviewContainer(title: "House",
                         previewName: "HousePreview",
                         previewModelName: nil,
                         selectedModelName: viewModel.previewModelName,
                         controlsVisible: $controlsVisible,
                         settings: settings,
                         layoutGroups: layoutGroups,
                         activeLayoutGroup: $activeLayoutGroup)
            .onAppear { refreshLayoutGroups() }
            .onChange(of: viewModel.isShowFolderLoaded) { _, _ in refreshLayoutGroups() }
            .onChange(of: activeLayoutGroup) { _, newValue in
                // Route selection through the document so the
                // render-context state updates and
                // `XLLayoutGroupChanged` fires for texture invalidate.
                viewModel.document.setActiveLayoutGroup(newValue)
            }
    }

    private func refreshLayoutGroups() {
        layoutGroups = (viewModel.document.layoutGroups() as? [String]) ?? ["Default"]
        activeLayoutGroup = viewModel.document.activeLayoutGroup() ?? "Default"
        // Adopt desktop's last-used 3D/2D preference from
        // `<settings><LayoutMode3D>` on each show-folder load. Users
        // can still flip the toggle mid-session; we don't write back.
        if viewModel.isShowFolderLoaded {
            settings.is3D = viewModel.document.layoutMode3D()
        }
    }
}

/// Model Preview — always 2D on desktop; no 2D/3D toggle exposed.
struct ModelPreviewView: View {
    @Environment(SequencerViewModel.self) var viewModel
    @State private var controlsVisible: Bool = false
    // is3D is wired off and the toggle is hidden via supportsIs3D below.
    @State private var settings = PreviewSettings(is3DDefault: false,
                                                  showViewObjectsDefault: false)

    var body: some View {
        PreviewContainer(title: "Model",
                         previewName: "ModelPreview",
                         previewModelName: viewModel.previewModelName,
                         selectedModelName: viewModel.previewModelName,
                         controlsVisible: $controlsVisible,
                         settings: settings,
                         layoutGroups: [],
                         activeLayoutGroup: .constant("Default"))
    }
}

/// Shared container that hosts a PreviewPaneView and overlays the controls
/// toggle and — when visible — camera shortcut buttons.
private struct PreviewContainer: View {
    let title: String
    let previewName: String
    let previewModelName: String?
    /// Name of the model the user currently has selected (drives Fit
    /// Selected). Independent of `previewModelName` which controls the
    /// bridge's single-model draw mode.
    let selectedModelName: String?
    @Binding var controlsVisible: Bool
    let settings: PreviewSettings
    let layoutGroups: [String]
    @Binding var activeLayoutGroup: String

    /// Model Preview ignores view objects entirely in XLMetalBridge, so the
    /// "Show View Objects" toggle is a no-op there — suppress it. Desktop
    /// Model Preview is 2D-only, so suppress the 2D/3D toggle there too.
    private var supportsViewObjects: Bool { previewName == "HousePreview" }
    private var supportsIs3D: Bool { previewName == "HousePreview" }

    var body: some View {
        ZStack(alignment: .topTrailing) {
            PreviewPaneView(previewName: previewName,
                            previewModelName: previewModelName,
                            controlsVisible: $controlsVisible,
                            settings: settings)

            VStack(alignment: .trailing, spacing: 4) {
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
                    PreviewControlsOverlay(previewName: previewName,
                                           settings: settings,
                                           supportsViewObjects: supportsViewObjects,
                                           supportsIs3D: supportsIs3D,
                                           selectedModelName: selectedModelName,
                                           layoutGroups: layoutGroups,
                                           activeLayoutGroup: $activeLayoutGroup)
                }
            }
            .padding(6)

            // Small title label in the upper-left for orientation.
            VStack {
                HStack {
                    Text(title)
                        .font(.caption2.weight(.semibold))
                        .foregroundStyle(.white.opacity(0.7))
                        .padding(.horizontal, 6)
                        .padding(.vertical, 2)
                        .background(.black.opacity(0.4), in: RoundedRectangle(cornerRadius: 4))
                    Spacer()
                }
                Spacer()
            }
            .padding(6)
            .allowsHitTesting(false)
        }
        .background(Color.black)
        .clipped()
    }
}

/// Camera shortcut buttons plus per-preview appearance toggles (2D/3D for
/// House only, view-object visibility). Zoom / reset actions are still
/// routed through NotificationCenter so each pane's Coordinator picks up
/// only its own — the mode/appearance state lives on the shared
/// `PreviewSettings` and is synced to the bridge in updateUIView.
private struct PreviewControlsOverlay: View {
    let previewName: String
    @Bindable var settings: PreviewSettings
    let supportsViewObjects: Bool
    let supportsIs3D: Bool
    let selectedModelName: String?
    let layoutGroups: [String]
    @Binding var activeLayoutGroup: String

    /// Current list of saved viewpoints for this pane (filtered to its
    /// 2D/3D mode, updated via `.previewViewpointListChanged`).
    @State private var viewpoints: [String] = []
    /// Name of the viewpoint most recently applied / saved from this
    /// pane. Displayed next to the viewpoint icon so users can see which
    /// saved view they're looking at, parity with the layout-group
    /// picker. Best-effort — user gestures (pinch, drag, rotate) will
    /// drift the camera away from the saved state, but we keep showing
    /// the name until they explicitly pick a different viewpoint or
    /// restore the default.
    @State private var appliedViewpoint: String? = nil
    /// Prompt state for "Save current view as…" — SwiftUI's .alert with
    /// a text field requires a backing binding, so these live here.
    @State private var savePromptVisible: Bool = false
    @State private var savePromptName: String = ""

    var body: some View {
        VStack(alignment: .trailing, spacing: 4) {
            HStack(spacing: 4) {
                Button { post(.zoomOut) } label: { Image(systemName: "minus.magnifyingglass") }
                Button { post(.zoomReset) } label: { Text("1×").font(.caption.monospacedDigit()) }
                Button { post(.zoomIn) } label: { Image(systemName: "plus.magnifyingglass") }
                Button { post(.reset) } label: { Image(systemName: "arrow.counterclockwise") }
            }
            .buttonStyle(.bordered)
            .controlSize(.small)

            // Fit All / Fit Selected — frame visible models in the
            // viewport. Fit Selected is only meaningful when a model
            // is picked (via Model Preview); greyed out otherwise.
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

            // 2D / 3D — House Preview only. Desktop Model Preview is 2D-only,
            // so no toggle there. Persisting at the scene level is Phase F.
            if supportsIs3D {
                Picker("", selection: $settings.is3D) {
                    Text("2D").tag(false)
                    Text("3D").tag(true)
                }
                .pickerStyle(.segmented)
                .frame(width: 96)
            }

            if supportsViewObjects {
                Toggle(isOn: $settings.showViewObjects) {
                    Text("View Objs").font(.caption2)
                }
                .toggleStyle(.button)
                .controlSize(.small)
            }

            // Layout group picker — House Preview only, always shown
            // (even when the show has only "Default") so the user has
            // visibility into which preview is active. Filters models
            // drawn in the House Preview to those assigned to the
            // chosen group (or "All Previews"), and swaps the
            // background image if the named group carries its own.
            // View objects render only in "Default".
            if supportsIs3D && !layoutGroups.isEmpty {
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
                    HStack(spacing: 3) {
                        Image(systemName: "square.stack.3d.up")
                        Text(activeLayoutGroup)
                            .font(.caption2)
                            .lineLimit(1)
                    }
                }
                .menuStyle(.borderlessButton)
                .buttonStyle(.bordered)
                .controlSize(.small)
            }

            // Viewpoints — saved camera positions, per pane and
            // filtered to the pane's current 2D/3D mode. Empty list
            // still shows the menu (so the user can Save / Restore).
            // Disabled in Model Preview (desktop's Model Preview
            // doesn't expose viewpoints either).
            if supportsIs3D {
                Menu {
                    ForEach(viewpoints, id: \.self) { name in
                        Button {
                            postViewpointCommand(action: "apply", name: name)
                            appliedViewpoint = name
                        } label: {
                            HStack {
                                Text(name)
                                if name == appliedViewpoint {
                                    Spacer()
                                    Image(systemName: "checkmark")
                                }
                            }
                        }
                    }
                    // "Default" is a virtual entry at the top of the
                    // list and must not appear in Delete. Also skip the
                    // submenu when there are no user-saved viewpoints.
                    let deletable = viewpoints.filter { $0 != "Default" }
                    if !deletable.isEmpty {
                        Divider()
                        Menu("Delete…") {
                            ForEach(deletable, id: \.self) { name in
                                Button(role: .destructive) {
                                    postViewpointCommand(action: "delete", name: name)
                                    if appliedViewpoint == name { appliedViewpoint = nil }
                                } label: {
                                    Text(name)
                                }
                            }
                        }
                    }
                    Divider()
                    Button("Save Current View As…") {
                        savePromptName = ""
                        savePromptVisible = true
                    }
                } label: {
                    HStack(spacing: 3) {
                        Image(systemName: "camera.viewfinder")
                        Text(appliedViewpoint ?? "View")
                            .font(.caption2)
                            .lineLimit(1)
                    }
                }
                .menuStyle(.borderlessButton)
                .buttonStyle(.bordered)
                .controlSize(.small)
            }

            // Share / save the current preview contents. Presents the
            // standard iOS share sheet (Photos, Files, Mail, AirDrop,
            // Copy, Print). No separate "Copy" button — the share sheet
            // already includes a copy action on all iPadOS versions.
            Button {
                NotificationCenter.default.post(name: .previewSaveImage,
                                                object: previewName)
            } label: {
                Image(systemName: "square.and.arrow.up")
            }
            .buttonStyle(.bordered)
            .controlSize(.small)
        }
        // Ask the coordinator to push the current list the moment the
        // overlay becomes visible, and whenever the 2D/3D toggle flips
        // (the available viewpoints list differs between modes).
        .onAppear {
            postViewpointCommand(action: "refresh", name: nil)
        }
        .onChange(of: settings.is3D) { _, _ in
            postViewpointCommand(action: "refresh", name: nil)
        }
        .onReceive(NotificationCenter.default.publisher(for: .previewViewpointListChanged)) { note in
            guard (note.object as? String) == previewName,
                  let names = note.userInfo?["names"] as? [String] else { return }
            viewpoints = names
        }
        .alert("Save Viewpoint", isPresented: $savePromptVisible) {
            TextField("Name", text: $savePromptName)
            Button("Save") {
                let trimmed = savePromptName.trimmingCharacters(in: .whitespaces)
                guard !trimmed.isEmpty else { return }
                postViewpointCommand(action: "save", name: trimmed)
                appliedViewpoint = trimmed
            }
            Button("Cancel", role: .cancel) { }
        } message: {
            Text("Saves the current camera position under this name.")
        }
    }

    private func postViewpointCommand(action: String, name: String?) {
        var info: [String: Any] = ["action": action]
        if let name { info["name"] = name }
        NotificationCenter.default.post(name: .previewViewpointCommand,
                                        object: previewName,
                                        userInfo: info)
    }

    private enum Action {
        case zoomIn, zoomOut, zoomReset, reset
    }

    private func post(_ action: Action) {
        let name: Notification.Name
        switch action {
        case .zoomIn: name = .previewZoomIn
        case .zoomOut: name = .previewZoomOut
        case .zoomReset: name = .previewZoomReset
        case .reset: name = .previewResetCamera
        }
        NotificationCenter.default.post(name: name, object: previewName)
    }
}

extension Notification.Name {
    static let previewZoomIn = Notification.Name("PreviewZoomIn")
    static let previewZoomOut = Notification.Name("PreviewZoomOut")
    static let previewZoomReset = Notification.Name("PreviewZoomReset")
    static let previewResetCamera = Notification.Name("PreviewResetCamera")
    static let previewFitAll = Notification.Name("PreviewFitAll")
    /// Fit to a specific named model. `userInfo["name"]` carries the
    /// model name. object is the preview name.
    static let previewFitModel = Notification.Name("PreviewFitModel")
    static let previewSaveImage = Notification.Name("PreviewSaveImage")
    /// Posted by `PreviewControlsOverlay` (object = previewName) when
    /// the user asks the coordinator to refresh its viewpoint list,
    /// apply a named viewpoint, save the current view, or restore the
    /// default. The userInfo carries `action` ("refresh" / "apply" /
    /// "save" / "delete" / "restore") and an optional `name` string.
    static let previewViewpointCommand = Notification.Name("PreviewViewpointCommand")
    /// Posted by the coordinator (object = previewName) whenever the
    /// viewpoint list for the pane changes (load, apply switch, save,
    /// delete). Carries `names: [String]` in userInfo. The overlay
    /// refreshes its menu from this.
    static let previewViewpointListChanged = Notification.Name("PreviewViewpointListChanged")
}
