import SwiftUI
import UIKit

/// F-5 — destroys any persisted scene sessions iPadOS would
/// otherwise restore from a previous launch. iPadOS 26's default
/// state restoration conflates geometry across our multiple
/// `WindowGroup`s — closing the main sequencer with a detached
/// preview open can result in the main coming back at a detached
/// pane's position/size on the next launch (see
/// `plans/phase-f-window-system.md` § F-5 carryover).
///
/// We do this in `configurationForConnecting`, not
/// `didFinishLaunchingWithOptions`, because the connecting
/// session is *itself* in `openSessions` at that early stage —
/// destroying it leaves iPadOS with no scene to display and the
/// app immediately exits ("Finished Running…" on first cold
/// launch, fine on the second). Skipping the actively-connecting
/// session preserves the main window while still wiping the
/// stale auxiliary sessions.
///
/// Tradeoff: the main window no longer remembers its size between
/// launches. Predictability wins — every launch starts at the
/// main WindowGroup's `.defaultSize`, which is a vastly better
/// experience than "small thumbnail surprise" on reopen.
class XLAppDelegate: NSObject, UIApplicationDelegate {
    private var didCleanStaleSessions = false

    func application(_ application: UIApplication,
                     configurationForConnecting connectingSceneSession: UISceneSession,
                     options: UIScene.ConnectionOptions) -> UISceneConfiguration {
        if !didCleanStaleSessions {
            didCleanStaleSessions = true
            let live = ObjectIdentifier(connectingSceneSession)
            for session in UIApplication.shared.openSessions
            where ObjectIdentifier(session) != live {
                UIApplication.shared.requestSceneSessionDestruction(
                    session, options: nil, errorHandler: nil)
            }
        }
        return UISceneConfiguration(name: nil,
                                     sessionRole: connectingSceneSession.role)
    }
}

@main
struct XLightsApp: App {
    @UIApplicationDelegateAdaptor(XLAppDelegate.self) private var appDelegate
    @State private var viewModel: SequencerViewModel
    @Environment(\.scenePhase) private var scenePhase

    init() {
        // Must initialize xLights core (sets FileUtils::GetResourcesDir) BEFORE
        // creating the view model, since SequencerViewModel constructs an
        // iPadRenderContext whose EffectManager needs the resources directory
        // to load effectmetadata JSON files.
        XLiPadInit.initialize()
        XLDiagnosticUploader.shared.bootstrap()
        let vm = SequencerViewModel()
        vm.startMemoryMonitoring()
        _viewModel = State(initialValue: vm)
        // restorePersistedShowFolder is deliberately NOT called here.
        // It triggers ObtainAccessToURL + xlights_rgbeffects.xml parse +
        // model construction + per-model FileExists, all synchronous. When
        // the show folder lives in iCloud Drive that chain can wall-clock
        // past iOS's 20-second launch watchdog (0x8BADF00D), killing the
        // app before the first frame ever draws. The restore now runs from
        // ContentView's `.task` modifier so launch completes first.
    }

    var body: some Scene {
        WindowGroup("xLights", id: "sequencer") {
            ContentView()
                .environment(viewModel)
                // F-5: declare a content-size minimum so iPadOS is
                // forced to resize main up to at least this on
                // launch. Without it, iPadOS inherits the
                // last-active scene's size (often a detached
                // pane's small thumbnail) and main opens tiny in a
                // corner. The floor must fit the smallest supported
                // iPad portrait width (iPad Mini ~744 pt) — anything
                // larger forces content to overflow the screen edges
                // in portrait, clipping row headings and list views.
                .frame(minWidth: 700, minHeight: 700)
        }
        .windowResizability(.contentSize)
        .commands {
            XLSequencerCommands(viewModel: viewModel)
        }
        .onChange(of: scenePhase) { _, newPhase in
            switch newPhase {
            case .inactive:
                // Temporary suspension (app switcher, incoming call,
                // Control Center). Pause playback + scrub so the
                // frame timers and audio stop burning energy;
                // keep render state so returning to .active is
                // instant. `.background` follows if the app really
                // goes away — that's handled below.
                viewModel.quiesceForInactive()
            case .background:
                // May precede termination; iOS can kill the process
                // without further notice. Pause playback, stop scrub,
                // then abort the render and block briefly so worker
                // threads exit cleanly before the teardown race —
                // otherwise they crash mid-frame on freed
                // SequenceElements / SequenceData.
                XLDiagnosticUploader.shared.endCurrentSession()
                viewModel.shutdownForBackground()
            case .active:
                XLDiagnosticUploader.shared.beginCurrentSession()
                XLDiagnosticUploader.shared.kickoff()
            @unknown default:
                break
            }
        }

        // F-1 — detachable House Preview. Opened via
        // `openWindow(id: "house-preview")` from the embedded
        // preview's controls overlay or the View menu. The scene
        // root flips `viewModel.housePreviewDetached` via its
        // onAppear / onDisappear so the embedded version in
        // SequencerView swaps for a dock placeholder while the
        // detached window is alive. Stage Manager on iPadOS 26
        // lets users drag this to an external display without us
        // routing screens directly.
        //
        // `WindowGroup` (not `Window`) because `Window` is macOS-
        // only. The Detach button on the embedded overlay is
        // suppressed when the placeholder is showing, which keeps
        // the WindowGroup from spawning a second instance.
        // iPadOS doesn't expose `.defaultLaunchBehavior(.suppressed)`
        // / `.restorationBehavior(.disabled)` (those are macOS-only),
        // so the "don't auto-restore detached scenes" behaviour is
        // implemented in the scene roots via a token check on
        // `onAppear`: an explicit user detach sets a token, the
        // detached scene's onAppear consumes it and proceeds;
        // absence of a token means the scene was system-restored
        // on launch and the root dismisses itself immediately. That
        // fixes the "relaunch restores the last-closed scene as
        // the main window's geometry" bug without losing genuine
        // user-opened scenes.

        WindowGroup("House Preview", id: "house-preview") {
            DetachedHousePreviewRoot()
                .environment(viewModel)
        }
        .defaultSize(width: 560, height: 360)
        .windowResizability(.contentSize)

        // F-1 — detachable Model Preview. Same pattern as House.
        WindowGroup("Model Preview", id: "model-preview") {
            DetachedModelPreviewRoot()
                .environment(viewModel)
        }
        .defaultSize(width: 420, height: 320)
        .windowResizability(.contentSize)

        // F-1c — detachable inspector tabs. Keyed by `InspectorTab`
        // so each of the four tabs (Effect / Colors / Blending /
        // Buffer) opens as its own scene window. Opening the same
        // tab twice focuses the existing window in Stage Manager.
        // The scene root flips an entry in
        // `viewModel.detachedInspectorTabs` so the embedded sidebar
        // swaps to a dock placeholder.
        WindowGroup(id: "inspector-tab", for: InspectorTab.self) { $tab in
            DetachedInspectorRoot(tab: tab ?? .effect)
                .environment(viewModel)
        }
        .defaultSize(width: 380, height: 620)
        .windowResizability(.contentSize)

        // J-0 — Layout Editor scene. Opened from Tools → Edit Layout.
        // Same token-guarded auto-restore protection as the F-1
        // detached previews; see `LayoutEditorWindowRoot`.
        WindowGroup("Edit Layout", id: "layout-editor") {
            LayoutEditorWindowRoot()
                .environment(viewModel)
        }
        .defaultSize(width: 1100, height: 720)
        .windowResizability(.contentSize)
    }
}

struct ContentView: View {
    @Environment(SequencerViewModel.self) var viewModel
    @Environment(\.scenePhase) private var scenePhase
    @Environment(\.dismissWindow) private var dismissWindow
    @Environment(\.openWindow) private var openWindow
    @State private var showFolderConfig = false

    @State private var showMediaManager = false

    /// Autosave recovery state. When a sequence opens with a
    /// newer `.xbkp` alongside its `.xsq`, we sheet the user:
    /// Recover (promote .xbkp → .xsq + reopen) or Discard
    /// (delete .xbkp). Checked once per open; the
    /// `lastCheckedSequencePath` guard prevents re-offering on
    /// every re-render of the shell.
    @State private var autosaveRecoveryDate: Date? = nil
    @State private var lastCheckedSequencePath: String = ""

    // F-5 detach-state persistence. iPadOS's native scene restoration
    // is disabled (sessions destroyed at launch to avoid geometry
    // conflation), so we record which panes were detached at the
    // moment of main-close and re-open them explicitly on the next
    // launch. Flags persist across launches; geometry does not.
    @AppStorage("f5.houseDetachedOnClose")
    private var houseDetachedOnClose: Bool = false
    @AppStorage("f5.modelDetachedOnClose")
    private var modelDetachedOnClose: Bool = false
    @AppStorage("f5.inspectorTabsDetachedOnClose")
    private var inspectorTabsDetachedCSV: String = ""

    /// Guards the post-launch auto-open so it only fires once per
    /// ContentView instance (the view reappears during sheet
    /// presentations etc.).
    @State private var didRestoreDetachedScenes: Bool = false

    /// Guards the deferred show-folder restore so it runs once even
    /// across `.task` re-invocations (sheet present/dismiss cycles).
    @State private var didKickoffShowFolderRestore: Bool = false

    /// G-3 — sequence URL handed to us by the system (Files /
    /// AirDrop / share sheet) before the show folder finished
    /// loading. Replayed via the
    /// `viewModel.isShowFolderLoaded` onChange below.
    @State private var pendingOpenURL: URL? = nil

    /// User-facing error when an incoming package (`.xsqz` tapped
    /// in Files) can't be copied into the app sandbox. Non-nil
    /// values surface an alert; the caller sets this from the
    /// URL-handling path.
    @State private var openURLErrorMessage: String? = nil

    var body: some View {
        VStack(spacing: 0) {
            if viewModel.memoryWarning {
                MemoryWarningBanner()
            }
            if viewModel.isSequenceLoaded && viewModel.brokenMediaCount > 0 {
                MissingMediaBanner(
                    count: viewModel.brokenMediaCount,
                    onReview: { showMediaManager = true })
            }
            if let msg = viewModel.fseqWriteSkippedMessage {
                FseqSkippedBanner(message: msg) {
                    viewModel.fseqWriteSkippedMessage = nil
                }
            }
            Group {
                if !viewModel.isShowFolderLoaded {
                    ShowFolderSetupView(showFolderConfig: $showFolderConfig)
                } else if !viewModel.isSequenceLoaded {
                    SequencePickerView(showFolderConfig: $showFolderConfig)
                } else {
                    SequencerView()
                }
            }
        }
        .sheet(isPresented: $showMediaManager) {
            MediaManagerSheet()
                .environment(viewModel)
        }
        .sheet(isPresented: $showFolderConfig) {
            FolderConfigView()
                .environment(viewModel)
        }
        .sheet(isPresented: Binding(
            get: { viewModel.showingAbout },
            set: { viewModel.showingAbout = $0 }
        )) {
            AboutSheet()
        }
        .sheet(isPresented: Binding(
            get: { viewModel.showingCheckSequence },
            set: { viewModel.showingCheckSequence = $0 }
        )) {
            CheckSequenceSheet()
                .environment(viewModel)
        }
        .sheet(isPresented: Binding(
            get: { viewModel.showingAIServices },
            set: { viewModel.showingAIServices = $0 }
        )) {
            AIServicesSettingsSheet()
        }
        // Unified Add Timing Track sheet. Driven by
        // viewModel.showingAddTimingTrack so all call sites
        // (Display Elements sheet, Settings → Timings tab, the
        // row-heading long-press menus, the empty-space long-press
        // below the last row) flip the same flag and present the
        // same sheet.
        .sheet(isPresented: Binding(
            get: { viewModel.showingAddTimingTrack },
            set: { viewModel.showingAddTimingTrack = $0 }
        )) {
            AddTimingTrackSheet()
                .environment(viewModel)
        }
        // Phase A re-prompt UX. When a persisted security-scoped
        // bookmark goes stale (iCloud eviction, iOS aging out the
        // bookmark) `SequencerViewModel` queues an
        // `AccessRepromptRequest` here instead of silently degrading
        // to empty models / missing media. Bind `item:` to the
        // observable so swipe-down ⇒ cancelReprompt (drains queue +
        // surfaces the next stale path).
        .sheet(item: Binding(
            get: { viewModel.accessReprompt },
            set: { newValue in
                if newValue == nil { viewModel.cancelReprompt() }
            })) { req in
            AccessRepromptSheet(
                request: req,
                onPicked: { url in viewModel.acceptReprompt(pickedURL: url) },
                onCancel: { viewModel.cancelReprompt() })
        }
        .onAppear {
            // Auto-open the dialog on first launch when nothing is configured.
            if !viewModel.isShowFolderLoaded && FolderConfig.showFolder == nil {
                showFolderConfig = true
            }
            // F-5: re-open detached scenes that were open at the
            // last main-close, honouring the user's working layout
            // without relying on iPadOS's broken auto-restoration.
            restoreDetachedScenesIfNeeded()
        }
        // Show-folder restore is deferred to here (instead of running
        // synchronously in `XLightsApp.init`) so the first frame draws
        // before we touch potentially-slow iCloud paths. The heavy work
        // inside `restorePersistedShowFolder` (obtainAccess pre-flight,
        // the C++ load, the recursive .xsq scan) now detaches onto a
        // background task so this `.task` returns quickly even with a
        // fully-evicted iCloud show folder — the launch UI stays
        // responsive and `isShowFolderLoaded` flips when the background
        // load completes.
        .task {
            guard !didKickoffShowFolderRestore else { return }
            didKickoffShowFolderRestore = true
            viewModel.restorePersistedShowFolder()
        }
        // G-3 — handle "Open in xLights" from Files / share sheets /
        // AirDrop. The system delivers a `file://` URL to the
        // app's registered .xsq UTI; we obtain security-scoped
        // access to it (so subsequent reads / writes work even
        // after the app restarts) and route into the existing
        // openSequence flow. If the app is mid-launch and the
        // show folder isn't loaded yet, defer until it is. `.xsqz`
        // support is intentionally deferred — SequencePackage
        // extraction isn't plumbed into the iPad bridge yet.
        .onOpenURL { url in
            handleIncomingSequenceURL(url)
        }
        .onChange(of: viewModel.isSequenceLoaded) { _, loaded in
            if loaded {
                checkAutosaveRecovery()
                // `.onAppear` opens the folder-config sheet for
                // fresh-install cases (no show folder configured).
                // When an `.xsqz` Files-tap races the launch and
                // opens a packaged sequence first, the auto-open
                // sheet is still visible on top of the sequencer —
                // the user has to manually tap Cancel. Detecting
                // isSequenceLoaded true is the clean signal that
                // the setup prompt is no longer needed; dismiss it
                // so the App-Store-review flow is a single tap.
                if showFolderConfig {
                    showFolderConfig = false
                }
            } else {
                autosaveRecoveryDate = nil
                lastCheckedSequencePath = ""
            }
        }
        // G-3 — replay any deferred Open-in-xLights once the show
        // folder finishes loading (typical on cold-launch via Files
        // tap, where the URL arrives before bookmarks restore).
        // Note: `.xsqz` opens never land here — those go direct in
        // `handleIncomingSequenceURL` since the package brings its
        // own show folder. Only `.xsq` taps defer.
        .onChange(of: viewModel.isShowFolderLoaded) { _, loaded in
            guard loaded, let url = pendingOpenURL else { return }
            pendingOpenURL = nil
            viewModel.openSequence(path: url.path)
        }
        // F-1 runtime coupling — when the main window is about to
        // close, dismiss the detached preview / inspector scenes
        // and wipe their persisted sessions BEFORE main commits
        // its own close, so iPadOS doesn't carry a detached's
        // geometry forward as "the app state".
        //
        // We hook `.active → .inactive` (first step of the close
        // sequence), not `.background` — by the time `.background`
        // fires, main is already gone and iPadOS has persisted
        // whatever it was going to persist.
        //
        // Differentiating close-this-window from app-wide
        // background / Control Center: when the user taps the
        // pill X on main, main alone transitions; the detached
        // scenes stay `.foregroundActive`. In app-wide
        // backgrounding, all scenes go `.inactive` simultaneously
        // so no sibling stays `.foregroundActive`. Using "another
        // scene is foregroundActive" as the predicate narrows to
        // the window-close case.
        //
        // Dirty handling: Stage Manager's pill close is not
        // interceptable — any confirmation alert we'd present is
        // torn down with the scene. Follow the iPad-native pattern
        // (Notes / Pages / Numbers) and silently save-on-close so
        // no work is lost. Users who want explicit save/discard
        // can still use the toolbar X, which prompts via
        // `showingUnsavedPrompt` as before.
        .onChange(of: scenePhase) { _, newPhase in
            guard newPhase == .inactive else { return }
            let siblingsActive = UIApplication.shared.connectedScenes.contains { scene in
                scene.activationState == .foregroundActive
            }
            guard siblingsActive else { return }

            if viewModel.isDirty {
                _ = viewModel.saveSequence()
            }
            // F-5: snapshot detach state before auto-dismiss so
            // next launch restores the user's exit layout. Must
            // come BEFORE the dismissWindow calls below — those
            // flip the live flags to false as each scene
            // disappears.
            snapshotDetachState()

            dismissWindow(id: "house-preview")
            dismissWindow(id: "model-preview")
            for tab in InspectorTab.allCases {
                dismissWindow(id: "inspector-tab", value: tab)
            }
            // Destroy the foreground-active scene sessions (the
            // detached panes). Running synchronously here, during
            // main's `.inactive` window, gets them torn down
            // before main commits its close — iPadOS then sees
            // main as the last-used scene and persists its
            // geometry, not a detached pane's.
            for scene in UIApplication.shared.connectedScenes
                where scene.activationState == .foregroundActive {
                UIApplication.shared.requestSceneSessionDestruction(
                    scene.session, options: nil, errorHandler: nil)
            }
        }
        // F-5 fix: the `.inactive` handler above only fires for
        // the "pill-close main while detached is alive" case. For
        // every other exit path (home button, swipe-up from the
        // app switcher, force-quit from memory pressure), scene
        // phase eventually reaches `.background` — snapshot there
        // too so the AppStorage flags reflect the actual exit
        // state regardless of how the user left the app.
        //
        // The snapshot is idempotent: if `.inactive` already ran
        // it, the flags are already current. If `.inactive` was
        // skipped (because sibling check failed), this catches it.
        .onChange(of: scenePhase) { _, newPhase in
            guard newPhase == .background else { return }
            snapshotDetachState()
        }
        .alert("Couldn't open package",
               isPresented: Binding(
                get: { openURLErrorMessage != nil },
                set: { if !$0 { openURLErrorMessage = nil } }
               )) {
            Button("OK", role: .cancel) { openURLErrorMessage = nil }
        } message: {
            Text(openURLErrorMessage ?? "")
        }
        .alert("Recover Autosave Backup?",
               isPresented: Binding(
                get: { autosaveRecoveryDate != nil },
                set: { if !$0 { autosaveRecoveryDate = nil } }
               )) {
            Button("Recover") {
                _ = viewModel.applyAutosaveBackup()
                autosaveRecoveryDate = nil
            }
            Button("Discard Backup", role: .destructive) {
                viewModel.suppressAutosaveBackup()
                autosaveRecoveryDate = nil
            }
            Button("Keep for Later", role: .cancel) {
                autosaveRecoveryDate = nil
            }
        } message: {
            if let date = autosaveRecoveryDate {
                Text("An autosave backup newer than the sequence file was found (saved \(date.formatted(date: .abbreviated, time: .shortened))). Recover changes from the backup, or discard it?")
            } else {
                Text("")
            }
        }
        // F-1 scene title — propagates to the iPadOS 26 Window menu's
        // "Open Windows" listing. Without this, the nested
        // `SequencePickerView` NavigationStack title ("Sequences")
        // leaks up to the scene title and sticks even after a
        // sequence is loaded. Prefer the active sequence's name
        // when one is open, otherwise fall back to the app name.
        .navigationTitle(mainSceneTitle)
    }

    /// Current title for the main sequencer scene. Used by
    /// iPadOS's Window menu and Stage Manager chrome.
    private var mainSceneTitle: String {
        if viewModel.isSequenceLoaded,
           let name = viewModel.sequenceName,
           !name.isEmpty {
            return name
        }
        return "xLights"
    }

    /// F-5 — write the current live detach state (view model
    /// flags + detached-inspector-tabs set) into AppStorage so the
    /// next launch can restore the same layout. Called from both
    /// the `.inactive` handler (pill-close path, before auto-
    /// dismiss clears the flags) and the `.background` handler
    /// (home / swipe-up / force-quit path). Idempotent.
    private func snapshotDetachState() {
        houseDetachedOnClose = viewModel.housePreviewDetached
        modelDetachedOnClose = viewModel.modelPreviewDetached
        inspectorTabsDetachedCSV = viewModel.detachedInspectorTabs
            .sorted()
            .joined(separator: ",")
    }

    /// G-3 — open a sequence file the system handed us (Files-app
    /// "Open in xLights", AirDrop accept, share-sheet target).
    /// Obtains security-scoped access via the bridge so subsequent
    /// reads/writes work after the app relaunches, then routes
    /// into the standard `openSequence` flow. If the show folder
    /// isn't loaded yet we wait — the open is queued in
    /// `pendingOpenURL` and re-tried whenever the folder becomes
    /// available.
    ///
    /// `.xsqz` (sequence package) taps route to the packaged-open
    /// path instead. Packages carry their own temp show folder, so
    /// they don't need `isShowFolderLoaded` and can open immediately
    /// even on first launch before any show folder is configured —
    /// this is the primary review/self-service flow (reviewer
    /// downloads an `.xsqz`, taps it, the app opens it end-to-end).
    private func handleIncomingSequenceURL(_ url: URL) {
        let path = url.path
        guard !path.isEmpty else { return }

        if isPackageURL(url) {
            // `.xsqz` on iOS: the URL typically points at
            // `~/Library/Mobile Documents/...` (iCloud Drive) or
            // another Files-provider location outside our sandbox.
            // Security scope grants URL-based read access, but the
            // bridge's minizip-backed extract uses path-based POSIX
            // syscalls which the sandbox rejects with EPERM for those
            // locations (unless the app holds iCloud-documents
            // entitlements, which we don't). Copy the package into
            // our app sandbox using NSFileCoordinator + URL-based
            // FileManager APIs — both of which honour the temporary
            // security scope — and hand the sandboxed path to the
            // extractor. On save, we copy the freshly-repacked
            // package back to the original URL (also URL-based, same
            // scope mechanism) so the user's edits land in the
            // original `.xsqz` they tapped.
            guard let sandboxURL = copyPackageToSandbox(originalURL: url) else {
                openURLErrorMessage =
                    "Couldn't open \(url.lastPathComponent). "
                    + "xLights was unable to read the file from its source — if it lives in iCloud Drive, "
                    + "make sure the item is downloaded and try again."
                return
            }
            viewModel.openPackagedSequence(sandboxPath: sandboxURL.path,
                                            originalURL: url)
            return
        }

        // Non-package (.xsq) path — obtain access normally. iCloud
        // Drive .xsq taps have the same sandbox limitation in
        // theory, but in practice .xsq files almost always live
        // inside an already-bookmarked show folder whose ancestor
        // bookmark covers the child path. Leaving that branch as-is.
        let scopeStarted = url.startAccessingSecurityScopedResource()
        _ = XLSequenceDocument.obtainAccess(toPath: path,
                                             enforceWritable: true)
        if scopeStarted {
            url.stopAccessingSecurityScopedResource()
        }

        if viewModel.isShowFolderLoaded {
            viewModel.openSequence(path: path)
        } else {
            pendingOpenURL = url
        }
    }

    /// Copy a Files-provider `.xsqz` into the app sandbox so the
    /// bridge's path-based extract APIs work. iCloud Drive items are
    /// downloaded on demand by NSFileCoordinator; non-iCloud items
    /// are just byte-copied. Returns the sandboxed URL on success.
    private func copyPackageToSandbox(originalURL: URL) -> URL? {
        let scopeStarted = originalURL.startAccessingSecurityScopedResource()
        defer {
            if scopeStarted {
                originalURL.stopAccessingSecurityScopedResource()
            }
        }

        // Use a per-open subdirectory so closeSequence can wipe just
        // this session's copy without disturbing any concurrent open.
        let sandboxDir = FileManager.default.temporaryDirectory
            .appendingPathComponent("xsqz-incoming", isDirectory: true)
            .appendingPathComponent(UUID().uuidString, isDirectory: true)
        do {
            try FileManager.default.createDirectory(at: sandboxDir,
                                                     withIntermediateDirectories: true)
        } catch {
            return nil
        }
        let dest = sandboxDir.appendingPathComponent(originalURL.lastPathComponent)

        var coordErr: NSError?
        var copyErr: Error?
        let coordinator = NSFileCoordinator()
        coordinator.coordinate(readingItemAt: originalURL,
                                options: [],
                                error: &coordErr) { readURL in
            do {
                try FileManager.default.copyItem(at: readURL, to: dest)
            } catch {
                copyErr = error
            }
        }
        if coordErr != nil || copyErr != nil {
            return nil
        }
        return dest
    }

    /// True iff the URL's pathExtension marks it as a sequence
    /// package the iPad bridge can unpack. Matches
    /// `SequencePackage`'s constructor detection on the core side.
    private func isPackageURL(_ url: URL) -> Bool {
        let ext = url.pathExtension.lowercased()
        return ext == "xsqz" || ext == "zip" || ext == "piz"
    }

    /// Run once per open: compare `.xbkp` mtime vs. `.xsq` and
    /// surface the recovery alert when the backup is newer.
    private func checkAutosaveRecovery() {
        let path = viewModel.document.currentSequencePath()
        guard !path.isEmpty, path != lastCheckedSequencePath else { return }
        lastCheckedSequencePath = path
        let (has, when) = viewModel.hasRecoverableBackup()
        if has { autosaveRecoveryDate = when }
    }

    /// F-5 — re-opens any detached scenes that the user had open
    /// at the last main-close. Idempotent (guarded by
    /// `didRestoreDetachedScenes`), so repeated `onAppear` calls
    /// triggered by sheet presentations don't respawn scenes.
    ///
    /// Deferred to the next run loop so SwiftUI has finished
    /// constructing the main scene before we try to open siblings
    /// — opening during the main's own `onAppear` is racy with
    /// the scene session hand-off.
    private func restoreDetachedScenesIfNeeded() {
        guard !didRestoreDetachedScenes else { return }
        didRestoreDetachedScenes = true

        DispatchQueue.main.async {
            if houseDetachedOnClose {
                viewModel.pendingDetachTokens.insert("house-preview")
                openWindow(id: "house-preview")
            }
            if modelDetachedOnClose {
                viewModel.pendingDetachTokens.insert("model-preview")
                openWindow(id: "model-preview")
            }
            let tabs = inspectorTabsDetachedCSV
                .split(separator: ",")
                .map(String.init)
                .compactMap(InspectorTab.init(rawValue:))
            for tab in tabs {
                viewModel.pendingDetachTokens.insert("inspector-tab:\(tab.rawValue)")
                openWindow(id: "inspector-tab", value: tab)
            }
        }
    }

}

struct MemoryWarningBanner: View {
    var body: some View {
        HStack(spacing: 8) {
            Image(systemName: "exclamationmark.triangle.fill")
            Text("Low memory — renders paused")
                .font(.caption)
            Spacer()
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 6)
        .foregroundStyle(.white)
        .background(Color.orange)
    }
}

/// Banner shown after `saveSequence` skipped its fseq companion
/// because the render had been aborted (memory pressure or an
/// explicit cancel) before completing. The `.xsq` save itself still
/// succeeded — the message just nudges the user to re-render +
/// re-save once memory recovers so the fseq lands on disk.
struct FseqSkippedBanner: View {
    let message: String
    let onDismiss: () -> Void

    var body: some View {
        HStack(spacing: 8) {
            Image(systemName: "exclamationmark.triangle.fill")
            Text(message)
                .font(.caption)
            Spacer()
            Button {
                onDismiss()
            } label: {
                Image(systemName: "xmark")
                    .font(.caption.weight(.semibold))
            }
            .accessibilityLabel("Dismiss")
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 6)
        .foregroundStyle(.white)
        .background(Color.orange.opacity(0.9))
    }
}

/// Banner shown at the top of the app when the just-opened
/// sequence references media files that don't resolve on this
/// device (missing files, evicted-from-iCloud, revoked bookmarks).
/// Tapping "Review" opens the sequence-wide `MediaManagerSheet` so
/// the user can see which files are missing. Actual relocation UI
/// lands with G30 (rename-with-reference-update) in a later pass.
struct MissingMediaBanner: View {
    let count: Int
    let onReview: () -> Void

    var body: some View {
        HStack(spacing: 8) {
            Image(systemName: "exclamationmark.triangle.fill")
            // "Has issues" covers both shapes: missing files (FileExists
            // false) and codec-incompatible videos (AVFoundation can't
            // decode — VP9, AV1, etc.). The Media Manager review sheet
            // shows the per-entry `brokenReason` so the user knows
            // which is which.
            Text(count == 1
                 ? "1 media file has issues"
                 : "\(count) media files have issues")
                .font(.caption)
            Spacer()
            Button("Review", action: onReview)
                .font(.caption.weight(.semibold))
                .foregroundStyle(.white)
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 6)
        .foregroundStyle(.white)
        .background(Color.red.opacity(0.85))
    }
}

struct ShowFolderSetupView: View {
    @Binding var showFolderConfig: Bool

    /// True when a show-folder path is persisted in `FolderConfig`
    /// but the deferred restore hasn't reported back yet. We show a
    /// "Loading…" affordance instead of the configure prompt so the
    /// user understands the app is materializing their iCloud-backed
    /// folder, not waiting on them to pick one.
    private var isRestoring: Bool {
        FolderConfig.showFolder != nil
    }

    var body: some View {
        VStack(spacing: 20) {
            Text("xLights")
                .font(.largeTitle)
            if isRestoring {
                ProgressView()
                Text("Loading show folder…")
                    .font(.headline)
                    .foregroundStyle(.secondary)
                Text("Large iCloud-backed folders may take a few moments.")
                    .font(.caption)
                    .foregroundStyle(.secondary)
                    .multilineTextAlignment(.center)
            } else {
                Text("Select your show folder to get started")
                    .font(.headline)
                    .foregroundStyle(.secondary)
                Button("Configure Folders…") {
                    showFolderConfig = true
                }
                .buttonStyle(.borderedProminent)
            }
        }
    }
}

/// G-2 — iCloud download state for a sequence file. Surfaced as an
/// inline icon in the picker so users see at a glance which
/// sequences are available offline vs. still materializing from
/// iCloud.
enum UbiquityStatus {
    /// Not an iCloud file, or can't be classified — no icon shown.
    case local
    /// Is in iCloud and fully downloaded + current. Small cloud-
    /// check icon, purely informative.
    case downloaded
    /// Is in iCloud but not yet downloaded to this device. Tap
    /// should trigger a download before attempting to open.
    case notDownloaded
    /// Is in iCloud and actively downloading right now.
    case downloading
}

/// Query the iCloud state for a file URL. Returns `.local` for
/// non-ubiquitous or unreadable URLs, `.downloaded` /
/// `.notDownloaded` / `.downloading` per Apple's
/// `ubiquitousItemDownloadingStatus` + `ubiquitousItemIsDownloading`
/// resource values.
func ubiquityStatus(for url: URL) -> UbiquityStatus {
    let keys: Set<URLResourceKey> = [
        .isUbiquitousItemKey,
        .ubiquitousItemDownloadingStatusKey,
        .ubiquitousItemIsDownloadingKey,
    ]
    guard let values = try? url.resourceValues(forKeys: keys),
          values.isUbiquitousItem == true else {
        return .local
    }
    if values.ubiquitousItemIsDownloading == true {
        return .downloading
    }
    switch values.ubiquitousItemDownloadingStatus {
    case .current, .downloaded:
        return .downloaded
    case .notDownloaded:
        return .notDownloaded
    default:
        return .downloaded
    }
}

/// G-2 — small icon reflecting a file's `UbiquityStatus`. Suppressed
/// for `.local` so non-iCloud files read as normal.
struct UbiquityBadge: View {
    let status: UbiquityStatus
    var body: some View {
        Group {
            switch status {
            case .local:
                EmptyView()
            case .downloaded:
                Image(systemName: "icloud")
                    .foregroundStyle(.secondary)
            case .downloading:
                Image(systemName: "icloud.and.arrow.down")
                    .foregroundStyle(.blue)
            case .notDownloaded:
                Image(systemName: "icloud.and.arrow.down")
                    .foregroundStyle(.secondary)
            }
        }
        .font(.caption)
        .imageScale(.small)
    }
}

struct SequencePickerView: View {
    @Environment(SequencerViewModel.self) var viewModel
    @Binding var showFolderConfig: Bool

    @State private var recent: [RecentSequences.Entry] = []
    @State private var showingNewWizard: Bool = false
    @State private var showingBatchRender: Bool = false
    @State private var openErrorMessage: String? = nil

    var body: some View {
        NavigationStack {
            List {
                if !recent.isEmpty {
                    Section("Recent") {
                        ForEach(recent) { entry in
                            let status = ubiquityStatus(for: URL(fileURLWithPath: entry.path))
                            Button {
                                openWithDownloadIfNeeded(path: entry.path, status: status)
                            } label: {
                                HStack(spacing: 8) {
                                    VStack(alignment: .leading, spacing: 2) {
                                        Text(entry.displayName)
                                            .font(.body)
                                            .foregroundStyle(.primary)
                                        Text(entry.parentFolder)
                                            .font(.caption2)
                                            .foregroundStyle(.secondary)
                                            .lineLimit(1)
                                            .truncationMode(.middle)
                                    }
                                    Spacer()
                                    UbiquityBadge(status: status)
                                }
                            }
                            .swipeActions(edge: .trailing) {
                                Button(role: .destructive) {
                                    RecentSequences.remove(path: entry.path,
                                                            forShowFolder: viewModel.showFolderPath)
                                    recent = RecentSequences.load(forShowFolder: viewModel.showFolderPath)
                                } label: {
                                    Label("Remove", systemImage: "xmark.bin")
                                }
                            }
                        }
                    }
                }
                Section(recent.isEmpty ? "Sequences" : "In This Show Folder") {
                    ForEach(viewModel.sequenceFiles) { entry in
                        let status = ubiquityStatus(for: URL(fileURLWithPath: entry.fullPath))
                        Button {
                            openWithDownloadIfNeeded(path: entry.fullPath, status: status)
                        } label: {
                            HStack(spacing: 8) {
                                VStack(alignment: .leading, spacing: 2) {
                                    Text(entry.displayName)
                                        .font(.body)
                                        .foregroundStyle(.primary)
                                    if !entry.parentRelativePath.isEmpty {
                                        Text(entry.parentRelativePath)
                                            .font(.caption2)
                                            .foregroundStyle(.secondary)
                                            .lineLimit(1)
                                            .truncationMode(.middle)
                                    }
                                }
                                Spacer()
                                UbiquityBadge(status: status)
                            }
                        }
                    }
                }
            }
            .navigationTitle("Sequences")
            .toolbar {
                ToolbarItem(placement: .topBarTrailing) {
                    HStack(spacing: 10) {
                        Button {
                            showingNewWizard = true
                        } label: {
                            Image(systemName: "plus")
                        }
                        Button {
                            showingBatchRender = true
                        } label: {
                            Image(systemName: "square.stack.3d.up")
                        }
                        .disabled(viewModel.sequenceFiles.isEmpty)
                        Button {
                            showFolderConfig = true
                        } label: {
                            Image(systemName: "folder.badge.gearshape")
                        }
                    }
                }
                if !recent.isEmpty {
                    ToolbarItem(placement: .topBarLeading) {
                        Menu {
                            Button(role: .destructive) {
                                RecentSequences.clear(forShowFolder: viewModel.showFolderPath)
                                recent = []
                            } label: {
                                Label("Clear Recent", systemImage: "clock.badge.xmark")
                            }
                        } label: {
                            Image(systemName: "ellipsis.circle")
                        }
                    }
                }
            }
            .onAppear {
                recent = RecentSequences.load(forShowFolder: viewModel.showFolderPath)
            }
            .onChange(of: viewModel.showFolderPath) { _, _ in
                // Show-folder change via the folder-config sheet —
                // swap the picker's recent list to the new show's
                // entries so users don't see stale cross-show paths.
                recent = RecentSequences.load(forShowFolder: viewModel.showFolderPath)
            }
            .sheet(isPresented: $showingNewWizard) {
                NewSequenceWizardView()
                    .environment(viewModel)
                    .onDisappear {
                        recent = RecentSequences.load(forShowFolder: viewModel.showFolderPath)
                    }
            }
            .sheet(isPresented: $showingBatchRender) {
                BatchRenderSheet()
                    .environment(viewModel)
            }
            .alert("Cannot Open Sequence",
                   isPresented: Binding(
                    get: { openErrorMessage != nil },
                    set: { if !$0 { openErrorMessage = nil } }
                   )) {
                Button("OK", role: .cancel) { openErrorMessage = nil }
            } message: {
                Text(openErrorMessage ?? "")
            }
        }
    }

    /// G-2 — open a sequence, starting a download first when the
    /// file is in iCloud but not yet materialized on this device.
    /// `FileExists()` already triggers an iCloud download as a side
    /// effect (per Phase A), but the download is synchronous and
    /// can be slow; calling
    /// `startDownloadingUbiquitousItem` proactively lets iCloud
    /// run it in the background, and the poll below attempts the
    /// open once it's local. For `.downloading` / `.downloaded`
    /// cases we just open immediately.
    private func openWithDownloadIfNeeded(path: String,
                                           status: UbiquityStatus) {
        // Race guard for files deleted between display and tap
        // (e.g. a recent entry whose source vanished while the
        // picker was on screen). Skip the iCloud branch — those
        // files don't have to be locally present yet.
        if status != .notDownloaded
            && !FileManager.default.fileExists(atPath: path) {
            let name = (path as NSString).lastPathComponent
            openErrorMessage = "\"\(name)\" no longer exists. It has been removed from Recent."
            RecentSequences.remove(path: path, forShowFolder: viewModel.showFolderPath)
            recent = RecentSequences.load(forShowFolder: viewModel.showFolderPath)
            return
        }
        guard status == .notDownloaded else {
            viewModel.openSequence(path: path)
            return
        }
        let url = URL(fileURLWithPath: path)
        try? FileManager.default.startDownloadingUbiquitousItem(at: url)
        // Poll briefly for the file to land (up to ~5s). This is a
        // best-effort UX — if the download is slow we just open what's
        // there; openSequence itself calls FileExists() which kicks
        // the download and can block.
        waitForDownload(url: url, attemptsRemaining: 10) { [weak viewModel] in
            viewModel?.openSequence(path: path)
        }
    }

    private func waitForDownload(url: URL,
                                  attemptsRemaining: Int,
                                  then complete: @escaping () -> Void) {
        let status = ubiquityStatus(for: url)
        if status != .notDownloaded || attemptsRemaining <= 0 {
            complete()
            return
        }
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
            waitForDownload(url: url,
                             attemptsRemaining: attemptsRemaining - 1,
                             then: complete)
        }
    }
}
