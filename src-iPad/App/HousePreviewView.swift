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
    /// J-2 — Layout Editor overlay toggles. House / Model previews
    /// don't touch these. Defaults are unset so the bridge's first
    /// draw can seed from `xlights_rgbeffects.xml`.
    var showLayoutGrid: Bool
    var showLayoutBoundingBox: Bool
    /// J-2 — Layout Editor snap-to-grid for drag-to-move. Off by
    /// default so unaware users don't get unexpected stair-stepping.
    var snapToGrid: Bool
    /// J-2 — first-pixel highlight (cyan dot on each model's node 0).
    /// Off by default; mirrors desktop's `_showFirstPixel`.
    var showFirstPixel: Bool
    /// J-2 (touch UX) — Layout Editor toolbar state. Replace
    /// desktop's held-key modifiers (Shift / Ctrl).
    /// `axisTool` is one of: "translate", "rotate", "scale",
    /// "xy_trans", "elevate".
    var axisTool: String
    var uniformModifier: Bool
    /// 0 = Free, 1 = X, 2 = Y, 3 = Z (matches MSLAXIS enum).
    var lockAxis: Int
    /// J-2 (touch UX) — model-name labels rendered as a SwiftUI
    /// overlay above the Metal canvas. Off by default.
    var showModelLabels: Bool

    init(is3DDefault: Bool, showViewObjectsDefault: Bool) {
        self.is3D = is3DDefault
        self.showViewObjects = showViewObjectsDefault
        self.showLayoutGrid = false
        self.showLayoutBoundingBox = false
        self.snapToGrid = false
        self.showFirstPixel = false
        self.axisTool = "translate"
        self.uniformModifier = false
        self.lockAxis = 0
        self.showModelLabels = false
    }
}

/// House Preview — shows every model plus view objects.
///
/// `onDetach` is non-nil when this instance lives in the main
/// sequencer window; calling it opens the dedicated `house-preview`
/// scene and flips `viewModel.housePreviewDetached = true`. Detached
/// instances pass nil so no "Detach" button appears in their own
/// controls overlay.
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

    var onDetach: (() -> Void)? = nil
    /// When true, this instance is hosted in its own Window scene.
    /// The pane suppresses its corner title label (redundant with
    /// Stage Manager's window title) so the iPadOS 26 window-
    /// controls pill doesn't overlay it, and leaves room on the
    /// leading edge for the pill's overlay geometry.
    var detachedMode: Bool = false

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
                         activeLayoutGroup: $activeLayoutGroup,
                         onDetach: onDetach,
                         detachedMode: detachedMode)
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
        layoutGroups = viewModel.document.layoutGroups()
        activeLayoutGroup = viewModel.document.activeLayoutGroup()
        // Adopt desktop's last-used 3D/2D preference from
        // `<settings><LayoutMode3D>` on each show-folder load. Users
        // can still flip the toggle mid-session; we don't write back.
        if viewModel.isShowFolderLoaded {
            settings.is3D = viewModel.document.layoutMode3D()
        }
    }
}

/// F-1 — root view for the standalone House Preview scene. Hosts the
/// same `HousePreviewView` the main window does, just without the
/// onDetach hook (no "Detach" button inside the already-detached
/// window) and with a black backdrop that fills the scene window.
/// Flips `viewModel.housePreviewDetached` so the main window swaps
/// its embedded copy for a placeholder.
///
/// No NavigationStack wrapper — the title bar it adds imposes a
/// ~320pt minimum width that keeps the window from shrinking to a
/// thumbnail. Stage Manager already shows the `WindowGroup("House
/// Preview", ...)` title in its own chrome.
struct DetachedHousePreviewRoot: View {
    @Environment(SequencerViewModel.self) var viewModel
    @Environment(\.dismissWindow) private var dismissWindow
    @Environment(\.openWindow) private var openWindow
    @State private var suppressed: Bool = false

    var body: some View {
        Group {
            if suppressed {
                Color.black
            } else {
                HousePreviewView(detachedMode: true)
                    .frame(maxWidth: .infinity, maxHeight: .infinity)
                    .background(Color.black)
            }
        }
        .frame(minWidth: 140, minHeight: 80)
        // `.navigationTitle` on the scene root propagates to the
        // iPadOS 26 Window menu without requiring a NavigationStack
        // wrapper. Without this, the scene falls back to the app's
        // display name ("xLights", "xLights 1", …).
        .navigationTitle("House Preview")
        .onAppear {
            // F-1 restoration guard — if no token is waiting, this
            // scene was auto-restored by iPadOS on launch rather
            // than opened by the user. Dismiss ourselves so the
            // app comes back to its main window only. Also open
            // the sequencer scene first: when iPadOS picks this
            // aux session as the connecting one (last-quit with
            // House Preview open), the main session has already
            // been destroyed by the AppDelegate cleanup and
            // dismissing here would leave zero scenes.
            if viewModel.pendingDetachTokens.remove("house-preview") != nil {
                viewModel.housePreviewDetached = true
            } else {
                suppressed = true
                DispatchQueue.main.async {
                    openWindow(id: "sequencer")
                    dismissWindow(id: "house-preview")
                }
            }
        }
        .onDisappear {
            if !suppressed { viewModel.housePreviewDetached = false }
        }
    }
}

/// Placeholder swapped in for the embedded House Preview while the
/// dedicated window is open. Tapping "Dock Here" dismisses the
/// detached scene, which fires its `.onDisappear` and clears the
/// flag; the HousePreviewView reappears in its place.
struct HousePreviewDockedPlaceholder: View {
    @Environment(\.dismissWindow) private var dismissWindow

    var body: some View {
        VStack(spacing: 10) {
            Image(systemName: "rectangle.on.rectangle.angled")
                .font(.system(size: 34))
                .foregroundStyle(.secondary)
            Text("House Preview is in its own window")
                .font(.footnote)
                .foregroundStyle(.secondary)
                .multilineTextAlignment(.center)
            Button("Dock Here") {
                dismissWindow(id: "house-preview")
            }
            .buttonStyle(.borderedProminent)
            .controlSize(.small)
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .padding()
        .background(Color.black)
    }
}

/// Model Preview — always 2D on desktop; no 2D/3D toggle exposed.
///
/// `onDetach` is non-nil for the embedded instance in the main
/// sequencer window; calling it opens the dedicated
/// `model-preview` scene. See `HousePreviewView` for the
/// full F-1 pattern.
struct ModelPreviewView: View {
    @Environment(SequencerViewModel.self) var viewModel
    @State private var controlsVisible: Bool = false
    // is3D is wired off and the toggle is hidden via supportsIs3D below.
    @State private var settings = PreviewSettings(is3DDefault: false,
                                                  showViewObjectsDefault: false)

    var onDetach: (() -> Void)? = nil
    var detachedMode: Bool = false

    var body: some View {
        PreviewContainer(title: "Model",
                         previewName: "ModelPreview",
                         previewModelName: viewModel.previewModelName,
                         selectedModelName: viewModel.previewModelName,
                         controlsVisible: $controlsVisible,
                         settings: settings,
                         layoutGroups: [],
                         activeLayoutGroup: .constant("Default"),
                         onDetach: onDetach,
                         detachedMode: detachedMode)
    }
}

/// F-1 — root view for the standalone Model Preview scene. See
/// `DetachedHousePreviewRoot` for the pattern.
struct DetachedModelPreviewRoot: View {
    @Environment(SequencerViewModel.self) var viewModel
    @Environment(\.dismissWindow) private var dismissWindow
    @Environment(\.openWindow) private var openWindow
    @State private var suppressed: Bool = false

    var body: some View {
        Group {
            if suppressed {
                Color.black
            } else {
                ModelPreviewView(detachedMode: true)
                    .frame(maxWidth: .infinity, maxHeight: .infinity)
                    .background(Color.black)
            }
        }
        .frame(minWidth: 140, minHeight: 80)
        .navigationTitle("Model Preview")
        .onAppear {
            if viewModel.pendingDetachTokens.remove("model-preview") != nil {
                viewModel.modelPreviewDetached = true
            } else {
                // See DetachedHousePreviewRoot for why we open the
                // sequencer first.
                suppressed = true
                DispatchQueue.main.async {
                    openWindow(id: "sequencer")
                    dismissWindow(id: "model-preview")
                }
            }
        }
        .onDisappear {
            if !suppressed { viewModel.modelPreviewDetached = false }
        }
    }
}

/// Placeholder for the embedded Model Preview while its scene is open.
struct ModelPreviewDockedPlaceholder: View {
    @Environment(\.dismissWindow) private var dismissWindow

    var body: some View {
        VStack(spacing: 10) {
            Image(systemName: "rectangle.on.rectangle.angled")
                .font(.system(size: 34))
                .foregroundStyle(.secondary)
            Text("Model Preview is in its own window")
                .font(.footnote)
                .foregroundStyle(.secondary)
                .multilineTextAlignment(.center)
            Button("Dock Here") {
                dismissWindow(id: "model-preview")
            }
            .buttonStyle(.borderedProminent)
            .controlSize(.small)
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .padding()
        .background(Color.black)
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
    /// F-1 detach hook. Non-nil for embedded instances; the controls
    /// overlay surfaces a "Detach" button that fires this callback.
    /// Detached scene-root instances pass nil to hide the button.
    var onDetach: (() -> Void)? = nil
    /// F-1: suppresses the corner title label and indents the
    /// controls from the leading edge so the iPadOS 26 window-
    /// controls pill (which overlays the top-left of the scene
    /// window) doesn't sit on top of anything important.
    var detachedMode: Bool = false

    /// Model Preview ignores view objects entirely in XLMetalBridge, so the
    /// "Show View Objects" toggle is a no-op there — suppress it. Desktop
    /// Model Preview is 2D-only, so suppress the 2D/3D toggle there too.
    private var supportsViewObjects: Bool { previewName == "HousePreview" }
    private var supportsIs3D: Bool { previewName == "HousePreview" }

    @State private var status = PreviewStatus()

    var body: some View {
        ZStack(alignment: .topTrailing) {
            PreviewPaneView(previewName: previewName,
                            previewModelName: previewModelName,
                            controlsVisible: $controlsVisible,
                            settings: settings,
                            status: status)

            // Diagnostic banner — surfaces the bridge's silent-fail
            // reason after a short grace period so transient init
            // states (drawable not sized yet, document just loaded)
            // don't flicker the message. Empty preview-model case
            // ("No model selected") shows on Model Preview as a
            // helpful hint rather than a failure.
            if let banner = status.bannerMessage {
                PreviewDiagnosticBanner(message: banner,
                                         hasRendered: status.hasRenderedSuccessfully)
                    .allowsHitTesting(false)
                    .padding(16)
            }

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
                                           activeLayoutGroup: $activeLayoutGroup,
                                           onDetach: onDetach)
                }
            }
            .padding(6)

            // Small title label in the upper-left for orientation.
            // Suppressed in detached mode — Stage Manager's window
            // chrome shows the scene title there, and the iPadOS 26
            // window-controls pill would overlap this label anyway.
            if !detachedMode {
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
    /// F-1: when set, a "Detach" button in the controls overlay
    /// fires this callback (typically opens the pane's dedicated
    /// Window scene).
    var onDetach: (() -> Void)? = nil

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

            // F-1 Detach — opens the pane in its own Window scene so
            // the user can drag it to a second display (Stage Manager)
            // or reposition it independently of the main sequencer.
            // Hidden inside the already-detached scene (onDetach nil).
            if let onDetach {
                Button {
                    onDetach()
                } label: {
                    Image(systemName: "rectangle.on.rectangle.angled")
                }
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
    /// Phase J-2 — posted whenever a model on the Layout Editor
    /// canvas has been moved by direct manipulation (drag),
    /// keyboard nudge, or an undo restoring older state.
    /// `object` follows the preview-name convention used by zoom /
    /// reset / fit ("LayoutEditor"), so the bridge's coordinator
    /// can listen and `setNeedsDisplay` on the canvas.
    /// `userInfo["model"]` carries the affected model name; the
    /// LayoutEditor side panel uses it to refresh its summary.
    static let layoutEditorModelMoved = Notification.Name("LayoutEditorModelMoved")
    /// Phase J-2 (touch UX) — long-press on a handle posts this
    /// with `userInfo` matching the shape returned by
    /// `XLMetalBridge.inspectHandleAtScreenPoint:` (keys: `type`,
    /// `modelName`, plus the per-type indices).
    static let layoutEditorContextMenu = Notification.Name("LayoutEditorContextMenu")
    /// Phase J-3 (touch UX) — Pencil Pro squeeze on the canvas
    /// asks the LayoutEditor to undo the last layout edit. Posted
    /// by PreviewPaneView's UIPencilInteraction handler;
    /// LayoutEditorView listens and calls its `performUndo()`.
    /// No userInfo.
    static let layoutEditorPencilUndo = Notification.Name("LayoutEditorPencilUndo")
}

/// Diagnostic banner painted over the preview pane when the bridge
/// can't render. Two flavours:
///
/// - **Informational** ("No model selected", "No models in active
///   preview") — shown as a muted hint when `hasRendered == true`,
///   meaning the pane is drawing fine but there's nothing to show.
/// - **Failure** ("No CAMetalLayer attached", "Drawable size 0x0
///   (waiting for layout)", "Metal graphics context invalid (… see
///   MetalDeviceManager log)") — shown as a warning when
///   `hasRendered == false`, meaning the pane has never produced a
///   frame on this device.
///
/// In either case the message is also written to spdlog, so iPad
/// → Tools → Package Logs captures it for tester reports.
private struct PreviewDiagnosticBanner: View {
    let message: String
    let hasRendered: Bool

    /// Failure tone is a warning yellow / red; informational is the
    /// muted secondary-on-dark scheme used elsewhere in the previews.
    private var isFailure: Bool { !hasRendered }

    var body: some View {
        HStack(alignment: .top, spacing: 10) {
            Image(systemName: isFailure
                  ? "exclamationmark.triangle.fill"
                  : "info.circle.fill")
                .foregroundStyle(isFailure ? .yellow : .white.opacity(0.8))
                .imageScale(.medium)
            VStack(alignment: .leading, spacing: 2) {
                Text(isFailure ? "Preview unavailable" : "Preview")
                    .font(.caption.weight(.semibold))
                    .foregroundStyle(.white)
                Text(message)
                    .font(.caption2)
                    .foregroundStyle(.white.opacity(0.85))
                    .fixedSize(horizontal: false, vertical: true)
                if isFailure {
                    Text("Tools → Package Logs sends details for diagnosis.")
                        .font(.caption2)
                        .foregroundStyle(.white.opacity(0.6))
                        .padding(.top, 2)
                }
            }
            Spacer(minLength: 0)
        }
        .padding(10)
        .background(
            RoundedRectangle(cornerRadius: 8)
                .fill(.black.opacity(0.7))
                .overlay(
                    RoundedRectangle(cornerRadius: 8)
                        .stroke((isFailure ? Color.yellow : .white).opacity(0.4),
                                lineWidth: 0.5)
                )
        )
        .frame(maxWidth: 380, alignment: .leading)
    }
}
