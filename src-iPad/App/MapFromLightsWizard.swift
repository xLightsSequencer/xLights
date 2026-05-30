import ARKit
import KLightMapper
import SceneKit
import SwiftUI

// "Map from lights..." — multi-step wizard that drives an FPP
// controller through a structured-light scan, captures the
// camera frames, decodes pixel positions, and hands back a
// `MapFromLightsResult` for the layout editor to materialize as
// a Custom model.
//
// All of the FPP / DDP / camera / analyzer machinery lives
// behind `LightMapping.Session` — see
// `macOS/src-iPad-core/Public/`. This file is purely SwiftUI
// glue around that facade.

/// Final payload the wizard hands back when the user accepts.
/// `xmodelURL` points at the on-disk `.xmodel` file the wizard
/// emitted; LayoutEditorView feeds the path to
/// `viewModel.layoutPendingImportPath` so the existing xmodel
/// import flow places the model on the next canvas tap.
struct MapFromLightsResult: Identifiable {
    let id = UUID()
    let xmodelURL: URL
    let modelName: String
    let layout: [LightMapping.PortLayout]
    let host: String
    let capeId: String?
    let dominantColorOrder: LightMapping.ColorOrder?
    let gridWidth: Int
    let gridHeight: Int
    let filledCellCount: Int
    let missingCount: Int
}

/// One entry in the controller-IP picker shown on the Connect
/// step. Sourced from xLights's configured controllers list
/// (`controllersListSummary`); the wizard filters for ones with
/// IPs and presents them in a Menu alongside a "custom IP" and
/// "last used IP" options.
struct MapFromLightsKnownController: Identifiable, Hashable {
    let id = UUID()
    let name: String
    let ip: String
    let vendor: String
}

struct MapFromLightsWizard: View {
    @Binding var isPresented: Bool
    let onComplete: (MapFromLightsResult) -> Void
    /// Root of the iCloud-synced show folder. When non-nil, the
    /// wizard dumps every captured scan frame (plus a metadata
    /// JSON) into `<showFolder>/MapFromLightsDebug/scan_<ts>/`
    /// after the scan completes so the developer can analyse
    /// off-device without rerunning.
    let scanDumpParent: URL?
    /// Closure that returns the controllers configured in the
    /// user's xLights show folder. Provided as a closure (rather
    /// than a pre-computed array) so we can defer the ObjC-bridge
    /// + ControllerCaps lookup work until the wizard's `.task`
    /// fires — otherwise the call ran on every parent body
    /// re-render and added noticeable latency to the sheet's
    /// initial appearance.
    let knownControllersProvider: () -> [MapFromLightsKnownController]
    /// Lookup hook: given the FPP host the user scanned, return
    /// the matching xLights controller name (or nil when no
    /// configured controller has that IP / resolved IP). The
    /// wizard uses this in `acceptResult` so the generated xmodel
    /// binds to the user's existing controller entry.
    var controllerNameLookup: (String) -> String? = { _ in nil }

    @State private var step: Step = .connect
    /// Last manually-entered IP, persisted between wizard runs.
    /// Empty string means "no previous entry."
    @AppStorage("MapFromLightsLastIP") private var lastEnteredIP: String = ""
    /// Whether to keep the scan dump (scan.mov + per-pattern
    /// PNGs + metadata.json) after the wizard completes. Off by
    /// default — a scan dump is ~500 MB and useful only for
    /// debugging analyzer behaviour. Devs flip this on to keep
    /// samples for offline analysis; setting persists across
    /// sessions.
    @AppStorage("MapFromLightsSaveScanData") private var saveScanData: Bool = false
    // The wizard always prepares the session in `.threeD` mode —
    // matching the macOS flow, where the user doesn't have to commit
    // to 2D vs 3D upfront. A single-view scan still saves as 2D
    // (Custom or MultiPoint format picked at Review); the user opts
    // into 3D by tapping "Capture Another Angle" from Review and
    // adding a second viewpoint, which lights up the multi-view
    // triangulator. On LiDAR-capable iPads multi-view uses depth-
    // based Procrustes; on non-LiDAR iPads it would need the depth-
    // free path that lives in the macOS branch today (see follow-up
    // task — for now non-LiDAR iPads still get the streamlined
    // single-scan UI, just without the multi-view "Add Another
    // Angle" button).
    /// True when the device supports ARKit scene depth (LiDAR).
    /// Computed once at view init — the answer doesn't change at
    /// runtime. Drives whether the 2D/3D radio is shown on the
    /// port-config step and whether the capture pipeline starts
    /// an `ARSession` alongside the `AVCaptureSession`.
    private let supportsLiDAR: Bool = ARWorldTrackingConfiguration
        .supportsFrameSemantics(.sceneDepth)
    /// Tracks foreground/background so the idle-timer override is
    /// cleared whenever the wizard isn't active.
    @Environment(\.scenePhase) private var scenePhase
    @State private var host: String = ""
    /// LightMapping facade session — wraps the FPP probe, camera,
    /// DDP scan driver, snapshot store. Constructed on successful
    /// `connect()`; torn down via `cancel()` / `restoreAnd-
    /// Disconnect()` on every exit path.
    @State private var session: LightMapping.Session?
    @State private var portsToScan: [LightMapping.PortLayout] = []
    /// Latest successfully captured scan. Drives the review step
    /// and is what the wizard hands back to the layout editor
    /// (via `acceptResult`) on completion. Replaces the legacy
    /// per-field `decoderResult` / `gridResult` / `scanFrames` /
    /// `scan3DPoints` / `scanPlaneFit` quartet — Session retains
    /// the raw analyzer outputs internally so the wizard only
    /// needs to hold this one handle.
    @State private var lastResult: LightMapping.Result?
    @State private var errorMessage: String?
    @State private var statusMessage: String = ""
    @State private var savedDumpDir: URL?
    /// Per-scan IDs persisted in `metadata.json`. `scanID` groups
    /// multiple viewpoints of the same prop (slice 3 multi-view
    /// triangulation); slice 1 always emits a fresh UUID with
    /// `viewID = 0` so future-3D tooling can ingest these dumps
    /// without a migration pass.
    @State private var scanID: UUID = UUID()
    @State private var viewID: Int = 0
    /// Accumulated captured viewpoints. In 2D mode this list
    /// always has exactly 1 entry once a scan completes. In 3D
    /// mode the user can append additional viewpoints via the
    /// "Capture Another Angle" button on the Review step; the
    /// multi-view solver (next iteration) will consume the
    /// whole list. Cleared on cancel/accept/full-restart.
    @State private var capturedViewpoints: [LightMapping.Result] = []
    /// User toggle on the Review step (LiDAR devices only). Default OFF
    /// — the first scan always processes + previews as 2D so a one-shot
    /// scan doesn't pay multi-view triangulation cost. Flipping it ON
    /// triggers the 3D solve (single-view depth or multi-view) and is
    /// the gate that unlocks "Capture Another Angle". Preserved across
    /// Capture-Another-Angle round-trips so the user stays in 3D mode
    /// once they've opted in.
    @State private var show3DPreview: Bool = false
    /// Cached multi-view solve result, populated when `show3DPreview`
    /// is ON and `capturedViewpoints.count >= 2`. Nil before any solve,
    /// or when the toggle is OFF, or when the viewpoint list changes
    /// (new scan / discard last) and the cache hasn't been refreshed
    /// yet. The Review step renders from this when in 3D mode + multi-
    /// view; falls back to `lastResult.points3D` (single-view LiDAR)
    /// otherwise.
    @State private var mergedResult: LightMapping.Result?
    // The DDP driver lives entirely inside `session` — the wizard
    // talks to it only via public Session methods
    // (`armFramingHeartbeat`, `startVerification`, etc.).
    /// Loaded from `knownControllersProvider` in `.task` so the
    /// initial sheet appearance isn't gated on the ObjC-bridge
    /// `controllersListSummary()` call (which iterates Controller
    /// objects + ControllerCaps lookups synchronously on main).
    @State private var knownControllers: [MapFromLightsKnownController] = []
    /// FPP password sent on every controller call this session.
    /// Loaded from the keychain on first connect attempt;
    /// updated when the user supplies a password through the
    /// 401-recovery sheet.
    @State private var credentials: LightMapping.Credentials?
    /// When non-nil, the SwiftUI sheet for the password prompt
    /// is up. The host string lets the prompt show which
    /// controller the user is logging into.
    @State private var passwordPromptHost: String?
    /// True while the pattern sequence is actively being
    /// captured to the .mov. The "Don't move the iPad" notice
    /// is gated on this — once the sequence has finished
    /// recording, the analysis phase has no motion sensitivity
    /// and the user can lower the iPad.
    @State private var motionSensitive: Bool = false
    /// User's "Save As" choice on the Review step. Persists per
    /// user via @AppStorage so devs / repeat users don't have
    /// to keep re-selecting MultiPoint on curve-heavy props.
    @AppStorage("MapFromLightsSaveAsFormat") private var saveAsFormatRaw: String = LightMapping.OutputFormat.custom.rawValue

    enum Step {
        case connect
        case portConfig
        case framing
        case scanning
        case review
        case error
    }

    var body: some View {
        NavigationStack {
            content
                .navigationTitle(stepTitle)
                .navigationBarTitleDisplayMode(.inline)
                .toolbar {
                    ToolbarItem(placement: .cancellationAction) {
                        Button("Cancel") {
                            cancel()
                        }
                    }
                }
        }
        .interactiveDismissDisabled(true)
        // Keep the screen awake for the whole wizard. The scan flow has
        // the user holding the device steady through framing + a ~10 s
        // pattern recording — a 30 s auto-lock could dim the screen
        // mid-scan. iOS resets the idle-timer on background, but we also
        // clear it explicitly on disappear and on scenePhase != .active
        // as belt-and-suspenders.
        .onAppear { UIApplication.shared.isIdleTimerDisabled = true }
        .onDisappear { UIApplication.shared.isIdleTimerDisabled = false }
        .onChange(of: scenePhase) { _, phase in
            if phase != .active { UIApplication.shared.isIdleTimerDisabled = false }
        }
        .task {
            LightMapping.prewarmEncoders()
            if knownControllers.isEmpty {
                knownControllers = knownControllersProvider()
            }
        }
        .sheet(item: Binding(
            get: { passwordPromptHost.map { PasswordPromptID(host: $0) } },
            set: { passwordPromptHost = $0?.host }
        )) { ctx in
            FPPPasswordPromptSheet(
                host: ctx.host,
                initialUsername: credentials?.username ?? "admin",
                onCancel: { passwordPromptHost = nil },
                onSubmit: submitPasswordPrompt)
        }
    }

    /// Identifiable wrapper for the sheet binding — SwiftUI
    /// requires a `Hashable & Identifiable` item, plain Strings
    /// don't qualify.
    private struct PasswordPromptID: Identifiable, Hashable {
        let host: String
        var id: String { host }
    }

    @ViewBuilder
    private var content: some View {
        switch step {
        case .connect:
            ConnectStep(host: $host,
                        statusMessage: $statusMessage,
                        saveScanData: $saveScanData,
                        knownControllers: knownControllers,
                        lastEnteredIP: lastEnteredIP,
                        onConnect: connectToController)
        case .portConfig:
            if let session {
                PortConfigStep(controllerInfo: session.controllerInfo,
                               selection: $portsToScan,
                               onContinue: { enterFraming() })
            } else {
                ProgressView()
            }
        case .framing:
            if let session {
                FramingStep(session: session,
                            onStartScan: startScan)
            } else {
                ProgressView()
            }
        case .scanning:
            if let session {
                ScanningStep(session: session,
                             statusMessage: statusMessage,
                             motionSensitive: motionSensitive)
            } else {
                ProgressView()
            }
        case .review:
            if let r = lastResult {
                ReviewStep(result: r,
                           mergedResult: mergedResult,
                           savedDumpDir: saveScanData ? savedDumpDir : nil,
                           // Every iPad can do multi-view now —
                           // LiDAR via the depth-based solver,
                           // non-LiDAR via the depth-free fallback
                           // (same path macOS Continuity Camera
                           // uses, surfaced through Session.solve-
                           // MultiView's UIKit branch).
                           supportsMultiView: true,
                           // The "Show in 3D" toggle is LiDAR-only.
                           // Non-LiDAR devices follow the macOS
                           // pattern instead: viewpoint count is
                           // the 2D-vs-3D signal (auto-3D ≥2 views).
                           showsThreeDToggle: supportsLiDAR,
                           viewpointCount: capturedViewpoints.count,
                           show3DPreview: Binding(
                               get: { show3DPreview },
                               set: { newValue in
                                   show3DPreview = newValue
                                   // Flipping ON with ≥2 views: solve now
                                   // so the 3D preview has something to
                                   // render. Single-view 3D uses the
                                   // existing `result.points3D` (LiDAR
                                   // depth from the lone scan) — no solve
                                   // needed. Flipping OFF clears the
                                   // cache so the next ON pays a fresh
                                   // solve (cheap insurance against
                                   // stale data after a discard).
                                   if newValue {
                                       refreshMergedIfNeeded()
                                   } else {
                                       mergedResult = nil
                                   }
                               }),
                           saveAsFormat: Binding(
                               get: { LightMapping.OutputFormat(rawValue: saveAsFormatRaw) ?? .custom },
                               set: { saveAsFormatRaw = $0.rawValue }
                           ),
                           onAccept: acceptResult,
                           // "Discard Last View" — pops the most-recent
                           // capture + its dump dir, then back to framing
                           // so the user can re-shoot just that angle. If
                           // it was the only capture, falls through to
                           // the same path as Discard & Rescan.
                           onDiscardLast: {
                               if let session {
                                   Task {
                                       await session.stopVerification()
                                       try? await session.armFramingHeartbeat()
                                   }
                               }
                               if !capturedViewpoints.isEmpty {
                                   capturedViewpoints.removeLast()
                               }
                               mergedResult = nil           // viewpoints changed → invalidate
                               // Non-LiDAR auto-toggle: dropping back
                               // to 1 view falls out of 3D mode (matches
                               // macOS — no toggle, count drives it).
                               if !supportsLiDAR {
                                   show3DPreview = capturedViewpoints.count >= 2
                               }
                               cleanupScanDirIfNeeded()
                               step = .framing
                           },
                           // "Discard & Rescan" — drops *every* captured
                           // viewpoint and its dump dir, returns to
                           // framing. Use when the whole session went
                           // sideways and the user wants to start clean.
                           // Also resets the 3D toggle so the rebuilt
                           // session starts in the default 2D-preview
                           // mode.
                           onDiscardAll: {
                               if let session {
                                   Task {
                                       await session.stopVerification()
                                       try? await session.armFramingHeartbeat()
                                   }
                               }
                               capturedViewpoints.removeAll()
                               mergedResult = nil
                               show3DPreview = false
                               cleanupScanDirIfNeeded()
                               step = .framing
                           },
                           // "Capture Another Angle" — keep accumulated
                           // viewpoints, next scan appends at view_id =
                           // capturedViewpoints.count. Driver + FPP scan
                           // config stay live so we go straight to framing.
                           // Only visible when show3DPreview is on.
                           onCaptureAnother: {
                               if let session {
                                   Task {
                                       await session.stopVerification()
                                       try? await session.armFramingHeartbeat()
                                   }
                               }
                               step = .framing
                           })
            } else {
                ProgressView()
            }
        case .error:
            ErrorStep(message: errorMessage ?? "Unknown error.",
                      onDismiss: cancel,
                      onRetry: {
                          // Honour the save-data toggle on the
                          // failed scan's dir before restarting.
                          cleanupScanDirIfNeeded()
                          step = .connect
                          errorMessage = nil
                      })
        }
    }

    private var stepTitle: String {
        switch step {
        case .connect:     return "Connect to Controller"
        case .portConfig:  return "Choose Ports"
        case .framing:     return "Aim the Camera"
        case .scanning:    return "Scanning…"
        case .review:      return "Review Mapping"
        case .error:       return "Error"
        }
    }

    // MARK: - Flow

    private func connectToController() {
        statusMessage = "Probing FPP at \(host)…"
        // First attempt uses whatever credentials we already have
        // (from a prior in-session login or the keychain).
        let initial: LightMapping.Credentials? =
            credentials ?? LightMapping.Keychain.load(host: host)
        Task {
            await attemptConnect(with: initial, isRetryAfterPrompt: false)
        }
    }

    /// Inner connect that the prompt-recovery loop calls back
    /// into. Two outcomes route the user UI:
    ///   • success → portConfig step
    ///   • `.authRequired` → password prompt sheet, then a
    ///     re-attempt with the new credentials
    /// Anything else throws into the error step.
    private func attemptConnect(with creds: LightMapping.Credentials?,
                                isRetryAfterPrompt: Bool) async {
        do {
            let s = try await LightMapping.Session.connect(host: host,
                                                           credentials: creds)
            // Pipe progress updates into the wizard's status banner.
            // Motion-sensitive flag is derived from phase so the
            // "Don't move the iPad" notice only shows while the
            // pattern sequence is actively recording.
            s.onProgress = { progress in
                statusMessage = progress.detail
                let recording = progress.phase == .recording
                    || progress.phase == .sendingPatterns
                if motionSensitive != recording {
                    motionSensitive = recording
                }
            }
            await MainActor.run {
                if let creds {
                    // Persist on first successful auth.
                    LightMapping.Keychain.store(host: host, credentials: creds)
                    self.credentials = creds
                }
                if !knownControllers.contains(where: { $0.ip == host }) {
                    lastEnteredIP = host
                }
                self.session = s
                portsToScan = []  // user picks ports next step
                step = .portConfig
            }
        } catch LightMapping.Error.authRequired(_) {
            // If we just retried with prompted credentials and
            // still got 401, the password the user typed was
            // wrong — drop the stored entry so the next prompt
            // starts blank, surface a useful error instead of
            // looping.
            await MainActor.run {
                if isRetryAfterPrompt {
                    LightMapping.Keychain.delete(host: host,
                                                 username: creds?.username ?? "admin")
                    self.credentials = nil
                    statusMessage = "Password rejected by \(host). Try again."
                }
                passwordPromptHost = host
            }
        } catch {
            await MainActor.run {
                errorMessage = error.localizedDescription
                step = .error
            }
        }
    }

    /// Called when the password-prompt sheet submits. Stores in
    /// keychain, then re-runs `attemptConnect` with the new
    /// credentials. The sheet dismisses itself via the binding;
    /// we set `passwordPromptHost = nil` to be explicit.
    private func submitPasswordPrompt(username: String, password: String) {
        let creds = LightMapping.Credentials(username: username, password: password)
        passwordPromptHost = nil
        statusMessage = "Authenticating with \(host)…"
        Task {
            await attemptConnect(with: creds, isRetryAfterPrompt: true)
        }
    }

    private func startScan() {
        // Fresh per-session ID on the FIRST scan; subsequent
        // viewpoints in a Mode B 3D session reuse it with an
        // incrementing `view_id` so the multi-view solver can
        // group dumps by `scan_id`. Retry-from-Review keeps the
        // same `view_id` (replaces, not appends) — see
        // `retryScan()`.
        if capturedViewpoints.isEmpty {
            self.scanID = UUID()
            self.viewID = 0
        } else {
            self.viewID = capturedViewpoints.count
        }
        // The scan driver + DDP heartbeat are already running from
        // `enterFraming` — currently pushing the dim-white framing
        // buffer. `Session.lockAndCaptureScan` swaps the heartbeat
        // buffer to calibration → red-full → all-off → each
        // pattern in turn, so the controller never goes silent
        // and FPP's watchdog never trips.
        guard let session else {
            errorMessage = "Session not ready. Please retry from the start."
            step = .error
            return
        }
        // Stream `scan.mov` straight into the show folder's
        // `MapFromLightsDebug/scan_<timestamp>/` directory when
        // a dump parent is configured. Session handles the
        // sandbox-access bookmark + dir creation; the wizard just
        // hands over the URL.
        if let parent = scanDumpParent {
            _ = XLSequenceDocument.obtainAccess(toPath: parent.path,
                                                enforceWritable: true)
        }
        step = .scanning
        motionSensitive = false

        Task {
            do {
                try await session.runFocusBracket()
                let result = try await session.lockAndCaptureScan(
                    scanID: scanID,
                    viewID: viewID,
                    scanDumpParent: scanDumpParent)
                savedDumpDir = result.scanDir
                capturedViewpoints.append(result)
                // Always keep lastResult as the latest single-view scan
                // — Review defaults to 2D preview so we don't pay multi-
                // view triangulation cost for users who just want a
                // one-shot 2D mapping. The 3D solve (and the merged
                // result installed on Session) is deferred until the
                // user flips the "Show in 3D" toggle, see
                // refreshMergedIfNeeded(). Discarding adds invalidates
                // the cache.
                lastResult = result
                mergedResult = nil
                // LiDAR devices have a "Show in 3D" toggle (default
                // OFF), so the merged solve only runs when the user
                // has opted in. Non-LiDAR devices follow the macOS
                // flow — there's no toggle (single-view 3D needs
                // LiDAR depth they don't have), so viewpoint count
                // drives 2D vs 3D: auto-3D-preview when ≥2 views.
                // The depth-free path lives in Session.solveMulti-
                // View's UIKit branch when arkitSnapshot is missing.
                if !supportsLiDAR {
                    show3DPreview = capturedViewpoints.count >= 2
                }
                let autoSolve = (!supportsLiDAR) || show3DPreview
                if autoSolve && capturedViewpoints.count >= 2 {
                    mergedResult = session.solveMultiView(capturedViewpoints)
                }
                motionSensitive = false
                step = .review
                session.startVerification(for: result)
            } catch {
                motionSensitive = false
                errorMessage = error.localizedDescription
                step = .error
            }
        }
    }

    // MARK: - Scan-dump cleanup

    /// Honour the `saveScanData` toggle. When off (default),
    /// remove the current scan-dump directory and any earlier
    /// viewpoint dumps in `capturedViewpoints`, plus the
    /// `MapFromLightsDebug/` parent if it ends up empty. When
    /// on, the dumps stay on disk for developer analysis.
    /// Idempotent + nil-safe.
    private func cleanupScanDirIfNeeded() {
        guard !saveScanData else { return }
        let fm = FileManager.default
        var dirs: [URL] = []
        if let cur = savedDumpDir { dirs.append(cur) }
        dirs.append(contentsOf: capturedViewpoints.compactMap { $0.scanDir })
        let unique = Array(Set(dirs.map { $0.path })).map { URL(fileURLWithPath: $0) }
        var parents: Set<String> = []
        for d in unique {
            try? fm.removeItem(at: d)
            parents.insert(d.deletingLastPathComponent().path)
        }
        for parentPath in parents {
            let parent = URL(fileURLWithPath: parentPath)
            if let contents = try? fm.contentsOfDirectory(at: parent,
                                                          includingPropertiesForKeys: nil,
                                                          options: [.skipsHiddenFiles]),
               contents.isEmpty {
                try? fm.removeItem(at: parent)
            }
        }
        savedDumpDir = nil
    }


    /// Parsed `LightMapping.OutputMode` for the user's choice.
    /// Always-`.threeD` — the wizard no longer has an upfront 2D/3D
    /// picker. Single-view captures still produce flat output (the
    /// review step's Custom/MultiPoint format picker handles that);
    /// multi-view captures unlock 3D triangulation when the user
    /// adds another angle. Matches the macOS Continuity-Camera flow.
    private var outputMode: LightMapping.OutputMode { .threeD }

    /// Run `solveMultiView` and cache the combined result for the 3D
    /// preview. Single-view sessions don't need a solve (the single
    /// scan already carries `result.points3D` from the LiDAR depth
    /// snapshot) so this is a no-op below ≥2 viewpoints. Also a no-op
    /// when no session is wired up yet (defensive — flow only enters
    /// Review with a live session).
    private func refreshMergedIfNeeded() {
        guard let session, capturedViewpoints.count >= 2 else { return }
        mergedResult = session.solveMultiView(capturedViewpoints)
    }

    private func acceptResult() {
        guard let session, let r = lastResult else { return }
        let modelName = LightMapping.defaultModelName()
        let matchedController = controllerNameLookup(host)
        let binding = LightMapping.ControllerBinding(
            host: host,
            ports: portsToScan,
            colorOrder: r.dominantColorOrder,
            matchedControllerName: matchedController)
        // Honour the format picker in both 2D and 3D modes —
        // <custommodel> now supports depth layers via snap3D (planar
        // clouds collapse to a flat custom model; volumetric props
        // produce a depth-layered xmodel). MultiPoint in 3D writes
        // the 3D <multipointmodel>. For 3D ≥2 views, re-install the
        // merged solve on the session so writeXModel emits the
        // triangulated cloud, not the single-view fallback that
        // prepare() last set.
        let format = LightMapping.OutputFormat(rawValue: saveAsFormatRaw) ?? .custom
        if show3DPreview && capturedViewpoints.count >= 2 {
            _ = session.solveMultiView(capturedViewpoints)
        }
        do {
            let url = try session.writeXModel(format: format,
                                              modelName: modelName,
                                              binding: binding,
                                              directory: nil)
            let filled = r.gridCells.first?.flatMap { $0 }.filter { $0 != 0 }.count ?? 0
            let result = MapFromLightsResult(
                xmodelURL: url,
                modelName: modelName,
                layout: portsToScan,
                host: host,
                capeId: session.controllerInfo.capeInfo?.id,
                dominantColorOrder: r.dominantColorOrder,
                gridWidth: r.gridWidth,
                gridHeight: r.gridHeight,
                filledCellCount: filled,
                missingCount: r.missingCount
            )
            // The driver + FPP scan config were kept alive past
            // the scan in case the user wanted to Retry. They've
            // accepted now, so tear everything down (stops
            // heartbeat, sends all-off, disconnects DDP, restores
            // FPP's pre-scan port config) before dismissing.
            Task { await teardownScanDriver() }
            cleanupScanDirIfNeeded()
            isPresented = false
            onComplete(result)
        } catch {
            errorMessage = "Could not save mapped model: \(error.localizedDescription)"
            step = .error
        }
    }

    private func cancel() {
        // Tear down the DDP driver (stops heartbeat, all-off,
        // disconnects) and restore the FPP config in one call;
        // Session.restoreAndDisconnect also stops the camera.
        Task { await teardownScanDriver() }
        cleanupScanDirIfNeeded()
        isPresented = false
    }

    /// Transition into the framing step. This is now where the
    /// real FPP setup happens (formerly done at the start of
    /// `startScan`):
    ///   1. Snapshot FPP's current config so `restoreOriginalState`
    ///      can put everything back.
    ///   2. Push the scan config — selected ports moved to a
    ///      channel range past every other port (`safeStartChannel`),
    ///      other ports left untouched so they stay dark when our
    ///      DDP traffic only writes scan-range channels.
    ///   3. Create the driver, connect DDP, start the heartbeat
    ///      with a dim-white buffer so the user can aim the camera
    ///      at lit LEDs.
    ///
    /// Pushing the scan config *here* instead of at "Start Scan"
    /// time is critical: FPP resets pixel output when it accepts
    /// a new config, which used to land in the middle of the
    /// focus bracket and break it. Doing the push before framing
    /// means the controller is stable + lit by the time the user
    /// is aiming the camera, and the focus-bracket / pattern
    /// sequence run against a settled FPP.
    private func enterFraming() {
        guard let session, !portsToScan.isEmpty else {
            step = .framing
            return
        }
        // Avoid double-setup if the user backs up from framing
        // and re-hits Continue.
        if session.isPrepared {
            step = .framing
            return
        }
        statusMessage = "Pushing scan configuration to FPP…"
        step = .scanning  // borrow the spinner UI while setup runs
        Task {
            do {
                try await session.prepare(portLayout: portsToScan,
                                          outputMode: outputMode)
                // FPP needs a moment to settle after accepting
                // the new config — pixel output gets re-initialised
                // and the first DDP frames immediately after the
                // push are sometimes dropped.
                try? await Task.sleep(nanoseconds: 500_000_000)

                // Light the rig with a dim-white framing
                // heartbeat so the user can see which LEDs the
                // scan will address.
                try await session.armFramingHeartbeat()

                try await session.startPreview()

                await MainActor.run {
                    statusMessage = ""
                    step = .framing
                }
            } catch {
                // Setup failed — restore FPP and surface the
                // error. Don't fall back to `.framing` because
                // there's no live heartbeat to drive the dim
                // light and the scan would fail too.
                await session.restoreAndDisconnect()
                await MainActor.run {
                    errorMessage = "Couldn't configure controller for scan: \(error.localizedDescription)"
                    step = .error
                }
            }
        }
    }

    /// Tear the scan driver down (stop heartbeat, all-off, dis-
    /// connect) and restore FPP's original config. Idempotent;
    /// safe to call from cancel / error / success paths.
    private func teardownScanDriver() async {
        if let session {
            await session.restoreAndDisconnect()
        }
    }
}

// MARK: - Host view (presents wizard + result alert)

/// Invisible host attached via `.background(...)` so the wizard
/// sheet + summary alert don't grow `LayoutEditorView`'s already-
/// huge modifier chain past the SwiftUI type-checker threshold.
///
/// On accept, the host invokes `onApply` with the on-disk
/// `.xmodel` URL so the parent can route it through the existing
/// xmodel-import flow (`viewModel.layoutPendingImportPath`). The
/// alert offers the user a Place / Discard choice between the
/// scan and the canvas tap.
struct MapFromLightsHost: View {
    @Binding var isPresented: Bool
    @Binding var result: MapFromLightsResult?
    let onApply: (MapFromLightsResult) -> Void
    /// Forwarded to the wizard so scan frames can be persisted
    /// into `<showFolder>/MapFromLightsDebug/` after each run.
    let scanDumpParent: URL?
    /// Provider for the controllers configured in the user's
    /// show. Passed as a closure (not a pre-computed array) so
    /// the relatively-expensive `controllersListSummary()` bridge
    /// call only runs once when the wizard's `.task` fires —
    /// not on every parent re-render.
    let knownControllersProvider: () -> [MapFromLightsKnownController]
    /// Maps an FPP host string to the matching xLights controller
    /// name. Plumbed through to the wizard so `acceptResult` can
    /// stamp the generated xmodel with the right Controller=
    /// attribute. Defaults to "no match" so callers that don't
    /// care (tests, previews) keep working.
    var controllerNameLookup: (String) -> String? = { _ in nil }

    var body: some View {
        Color.clear
            .frame(width: 0, height: 0)
            .sheet(isPresented: $isPresented) {
                MapFromLightsWizard(isPresented: $isPresented,
                                    onComplete: { r in result = r },
                                    scanDumpParent: scanDumpParent,
                                    knownControllersProvider: knownControllersProvider,
                                    controllerNameLookup: controllerNameLookup)
            }
            .alert("Mapping Complete",
                   isPresented: alertBinding,
                   presenting: result) { r in
                Button("Place on Canvas") {
                    onApply(r)
                    result = nil
                }
                Button("Discard", role: .cancel) {
                    discardXModel(at: r.xmodelURL)
                    result = nil
                }
            } message: { r in
                Text(Self.summary(for: r))
            }
    }

    private var alertBinding: Binding<Bool> {
        Binding(
            get: { result != nil },
            set: { if !$0 { result = nil } }
        )
    }

    private func discardXModel(at url: URL) {
        // Best-effort cleanup; the temp dir is wiped by iOS
        // eventually anyway, but tidying immediately keeps the
        // temp dir from accumulating files across cancelled scans.
        try? FileManager.default.removeItem(at: url)
    }

    private static func summary(for r: MapFromLightsResult) -> String {
        let total = r.gridWidth * r.gridHeight
        var msg = "\(r.modelName): \(r.filledCellCount) of \(total) cells filled in a \(r.gridWidth)×\(r.gridHeight) grid."
        if r.missingCount > 0  { msg += " \(r.missingCount) missing." }
        msg += " Tap Place on Canvas to drop the model on your layout."
        return msg
    }
}

// MARK: - Steps

private struct FPPPasswordPromptSheet: View {
    let host: String
    let initialUsername: String
    let onCancel: () -> Void
    let onSubmit: (_ username: String, _ password: String) -> Void

    @State private var username: String
    @State private var password: String = ""
    @FocusState private var focusedField: Field?
    @Environment(\.dismiss) private var dismiss

    private enum Field { case username, password }

    init(host: String,
         initialUsername: String,
         onCancel: @escaping () -> Void,
         onSubmit: @escaping (_ username: String, _ password: String) -> Void) {
        self.host = host
        self.initialUsername = initialUsername
        self.onCancel = onCancel
        self.onSubmit = onSubmit
        _username = State(initialValue: initialUsername)
    }

    var body: some View {
        NavigationStack {
            Form {
                Section {
                    TextField("Username", text: $username)
                        .textContentType(.username)
                        .autocapitalization(.none)
                        .disableAutocorrection(true)
                        .focused($focusedField, equals: .username)
                    SecureField("Password", text: $password)
                        .textContentType(.password)
                        .focused($focusedField, equals: .password)
                        .onSubmit(submit)
                } header: {
                    Text("Sign in to \(host)")
                } footer: {
                    Text("FPP returned 401 — this controller has password protection enabled. The password is stored in the iPad's keychain and re-used on future scans.")
                }
            }
            .navigationTitle("FPP Password")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") {
                        onCancel()
                        dismiss()
                    }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Sign In", action: submit)
                        .disabled(password.isEmpty)
                }
            }
            .onAppear {
                focusedField = username.isEmpty ? .username : .password
            }
        }
    }

    private func submit() {
        guard !password.isEmpty else { return }
        onSubmit(username, password)
        dismiss()
    }
}

private struct ConnectStep: View {
    @Binding var host: String
    @Binding var statusMessage: String
    @Binding var saveScanData: Bool
    let knownControllers: [MapFromLightsKnownController]
    let lastEnteredIP: String
    let onConnect: () -> Void

    /// User chose to type a custom IP — show the inline text
    /// field beneath the picker.
    @State private var showingCustomEntry = false

    var body: some View {
        Form {
            Section("FPP Controller") {
                Menu {
                    if !knownControllers.isEmpty {
                        Section("From this show") {
                            ForEach(knownControllers) { c in
                                Button {
                                    showingCustomEntry = false
                                    host = c.ip
                                } label: {
                                    HStack {
                                        Text("\(c.name) (\(c.ip))")
                                        if c.vendor.lowercased() != "fpp"
                                            && !c.vendor.isEmpty {
                                            Text("— \(c.vendor)")
                                                .foregroundStyle(.secondary)
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if !lastEnteredIP.isEmpty
                        && !knownControllers.contains(where: { $0.ip == lastEnteredIP }) {
                        Section("Recently used") {
                            Button("Last entered (\(lastEnteredIP))") {
                                showingCustomEntry = false
                                host = lastEnteredIP
                            }
                        }
                    }
                    Section {
                        Button("Enter IP manually…") {
                            showingCustomEntry = true
                        }
                    }
                } label: {
                    HStack {
                        Text(host.isEmpty ? "Choose controller" : host)
                            .foregroundStyle(host.isEmpty ? .secondary : .primary)
                        Spacer()
                        Image(systemName: "chevron.up.chevron.down")
                            .foregroundStyle(.secondary)
                            .font(.footnote)
                    }
                    .contentShape(Rectangle())
                }
                if showingCustomEntry {
                    TextField("IP address or hostname", text: $host)
                        .keyboardType(.URL)
                        .autocapitalization(.none)
                        .disableAutocorrection(true)
                }
                Text(captionText)
                    .font(.footnote)
                    .foregroundStyle(.secondary)
            }
            if !statusMessage.isEmpty {
                Section { Text(statusMessage).foregroundStyle(.secondary) }
            }
            Section {
                Toggle("Save scan data", isOn: $saveScanData)
                Text("Off by default. When on, the raw scan video + per-pattern frames are kept under MapFromLightsDebug/ in the show folder for off-device analysis. Roughly 500 MB per scan.")
                    .font(.footnote)
                    .foregroundStyle(.secondary)
            }
            Section {
                Button("Connect") { onConnect() }
                    .buttonStyle(.borderedProminent)
                    .disabled(host.trimmingCharacters(in: .whitespaces).isEmpty)
            }
        }
        .onAppear {
            // Default the field to whatever the user last connected
            // to. If they have no history at all, leave it blank and
            // let them pick from the menu.
            if host.isEmpty {
                host = lastEnteredIP
            }
            // If the host they pre-populated isn't in the known list,
            // open the custom-entry field by default so the value is
            // immediately editable.
            if !host.isEmpty
                && !knownControllers.contains(where: { $0.ip == host }) {
                showingCustomEntry = true
            }
        }
    }

    private var captionText: String {
        if knownControllers.isEmpty {
            return "Enter the IP address or `*.local` hostname of the Falcon Player controller driving your lights."
        }
        return "Pick a controller from your show or enter an IP manually."
    }
}

private struct PortConfigStep: View {
    let controllerInfo: LightMapping.ControllerInfo
    @Binding var selection: [LightMapping.PortLayout]
    let onContinue: () -> Void

    var body: some View {
        Form {
            Section {
                Text(headerText)
                    .font(.headline)
                Text("Enable each port you want to scan. Most scans only need one or two ports. Pixel count and channels per pixel default to the controller's current configuration; edit them if the strings you're scanning differ.")
                    .font(.footnote)
                    .foregroundStyle(.secondary)
                // Surface the total here (not below the port list) so
                // the user sees scope without scrolling past every port
                // row on long-cape controllers.
                Text("Total LEDs to scan: \(selection.reduce(0) { $0 + $1.pixelCount })")
                    .font(.footnote)
                    .foregroundStyle(.secondary)
            }
            Section("Ports") {
                ForEach(controllerInfo.ports, id: \.portNumber) { port in
                    PortRow(port: port,
                            layout: layoutFor(port: port),
                            onToggle: { toggle(port: port) },
                            onUpdate: { updated in update(port: port, to: updated) })
                }
            }
            // The upfront 2D/3D Output picker used to live here on
            // LiDAR-capable iPads. Removed — single-view captures
            // produce 2D output, multi-view captures (via Review's
            // "Capture Another Angle") unlock 3D triangulation. The
            // user picks the *save format* (Custom vs MultiPoint) at
            // Review once they've seen the wireframe; the
            // dimensionality is implicit in the viewpoint count.
        }
        .toolbar {
            ToolbarItem(placement: .confirmationAction) {
                Button("Continue", action: onContinue)
                    .disabled(!hasUsableSelection)
            }
        }
    }

    private var hasUsableSelection: Bool {
        !selection.isEmpty && selection.allSatisfy { $0.pixelCount > 0 }
    }

    private var headerText: String {
        let hostText: String
        if let hn = controllerInfo.hostName, !hn.isEmpty {
            hostText = hn
        } else {
            hostText = controllerInfo.host
        }
        if let cape = controllerInfo.capeInfo?.name, !cape.isEmpty {
            return "\(hostText) — \(cape)"
        }
        return hostText
    }

    private func layoutFor(port: LightMapping.PortInfo) -> LightMapping.PortLayout? {
        selection.first(where: { $0.portNumber == port.portNumber })
    }

    private func toggle(port: LightMapping.PortInfo) {
        if let idx = selection.firstIndex(where: { $0.portNumber == port.portNumber }) {
            selection.remove(at: idx)
            repackStartChannels()
        } else {
            // Default the new port's pixel count to whatever FPP
            // currently has configured (so the common case is one
            // tap); fall back to 50 for ports FPP has never seen.
            let defaultPixels = port.pixelCount > 0 ? port.pixelCount : 50
            // Seed `nullPixels` from FPP's existing config so the
            // common case (port already configured with null
            // pixels) needs zero user input.
            selection.append(LightMapping.PortLayout(
                portNumber: port.portNumber,
                pixelCount: defaultPixels,
                startChannel: 0,
                channelsPerPixel: 3,
                nullPixels: port.nullNodes))
            selection.sort { $0.portNumber < $1.portNumber }
            repackStartChannels()
        }
    }

    private func update(port: LightMapping.PortInfo, to updated: LightMapping.PortLayout) {
        guard let idx = selection.firstIndex(where: { $0.portNumber == port.portNumber }) else { return }
        selection[idx] = updated
        repackStartChannels()
    }

    /// Pack `startChannel` contiguously starting one past the
    /// highest channel any non-selected port uses in the
    /// controller's existing config. We deliberately *don't*
    /// start at 0/1 because the FPP controller is probably
    /// sharing channels 1..N with other props that aren't part of
    /// this scan; writing scan DDP into channels 1..N would
    /// modulate their LEDs in lock-step with our binary patterns
    /// and steal valid IDs in the camera frames. Packing beyond
    /// every other port's range keeps the scan strictly local to
    /// channels nothing else owns.
    ///
    /// Called after every add / remove / pixelCount /
    /// channelsPerPixel change so the DDP frame addresses each
    /// port's first byte at exactly the offset its scan-config
    /// JSON declares.
    private func repackStartChannels() {
        let safeStart = safeStartChannel()
        var ch = safeStart
        selection = selection.map { p in
            let layout = LightMapping.PortLayout(
                portNumber: p.portNumber,
                pixelCount: p.pixelCount,
                startChannel: ch,
                channelsPerPixel: p.channelsPerPixel,
                nullPixels: p.nullPixels)
            ch += p.pixelCount * p.channelsPerPixel
            return layout
        }
    }

    /// First channel past every non-selected port's configured
    /// range. Selected ports are excluded because the scan config
    /// push rewrites their channel allocation anyway — only the
    /// ports we're leaving alone can collide. Channels are
    /// 1-based in FPP; returning `1` means the controller is
    /// otherwise empty.
    private func safeStartChannel() -> Int {
        let selectedPortNumbers = Set(selection.map { $0.portNumber })
        var maxEnd = 0
        for p in controllerInfo.ports where p.pixelCount > 0 {
            if selectedPortNumbers.contains(p.portNumber) { continue }
            let portEnd = p.startChannel + p.pixelCount * 3 - 1
            if portEnd > maxEnd { maxEnd = portEnd }
        }
        return max(1, maxEnd + 1)
    }
}

private struct PortRow: View {
    let port: LightMapping.PortInfo
    let layout: LightMapping.PortLayout?
    let onToggle: () -> Void
    let onUpdate: (LightMapping.PortLayout) -> Void

    @FocusState private var pixelFieldFocused: Bool

    private var isSelected: Bool { layout != nil }

    var body: some View {
        VStack(alignment: .leading, spacing: 10) {
            Button(action: onToggle) {
                HStack {
                    Image(systemName: isSelected ? "checkmark.circle.fill" : "circle")
                        .foregroundStyle(isSelected ? Color.accentColor : .secondary)
                    VStack(alignment: .leading) {
                        // FPP's `portNumber` is 0-based; users
                        // (and the controller's silkscreen) label
                        // ports starting at 1.
                        Text("Port \(port.portNumber + 1)").font(.body)
                        Text(currentConfigCaption)
                            .font(.caption)
                            .foregroundStyle(.secondary)
                    }
                    Spacer()
                }
                .contentShape(Rectangle())
            }
            .buttonStyle(.plain)

            if let layout {
                editor(for: layout)
                    .padding(.leading, 30)
            }
        }
    }

    private var currentConfigCaption: String {
        if port.pixelCount > 0 {
            return "Current FPP config: \(port.pixelCount) pixels · \(port.colorOrder)"
        }
        return "Not configured on FPP"
    }

    @ViewBuilder
    private func editor(for layout: LightMapping.PortLayout) -> some View {
        VStack(alignment: .leading, spacing: 8) {
            HStack(spacing: 16) {
                HStack(spacing: 6) {
                    Text("Pixels")
                        .font(.subheadline)
                        .foregroundStyle(.secondary)
                    TextField("0", value: pixelBinding(for: layout),
                              format: .number)
                        .textFieldStyle(.roundedBorder)
                        .keyboardType(.numberPad)
                        .frame(width: 80)
                        .focused($pixelFieldFocused)
                }
                HStack(spacing: 6) {
                    Text("Null")
                        .font(.subheadline)
                        .foregroundStyle(.secondary)
                    TextField("0", value: nullBinding(for: layout),
                              format: .number)
                        .textFieldStyle(.roundedBorder)
                        .keyboardType(.numberPad)
                        .frame(width: 60)
                }
                HStack(spacing: 6) {
                    Text("Channels")
                        .font(.subheadline)
                        .foregroundStyle(.secondary)
                    Picker("Channels", selection: channelsBinding(for: layout)) {
                        Text("3 (RGB)").tag(3)
                        Text("4 (RGBW)").tag(4)
                    }
                    .pickerStyle(.segmented)
                    .labelsHidden()
                    .frame(width: 160)
                }
                Spacer()
            }
            if layout.nullPixels > 0 {
                Text("Controller skips the first \(layout.nullPixels) pixel\(layout.nullPixels == 1 ? "" : "s") on this port; scanned IDs start at the first addressable pixel.")
                    .font(.caption2)
                    .foregroundStyle(.secondary)
            }
        }
    }

    private func pixelBinding(for layout: LightMapping.PortLayout) -> Binding<Int> {
        Binding(
            get: { layout.pixelCount },
            set: { newValue in
                onUpdate(LightMapping.PortLayout(
                    portNumber: layout.portNumber,
                    pixelCount: max(0, newValue),
                    startChannel: layout.startChannel,
                    channelsPerPixel: layout.channelsPerPixel,
                    nullPixels: layout.nullPixels))
            }
        )
    }

    private func nullBinding(for layout: LightMapping.PortLayout) -> Binding<Int> {
        Binding(
            get: { layout.nullPixels },
            set: { newValue in
                onUpdate(LightMapping.PortLayout(
                    portNumber: layout.portNumber,
                    pixelCount: layout.pixelCount,
                    startChannel: layout.startChannel,
                    channelsPerPixel: layout.channelsPerPixel,
                    nullPixels: max(0, newValue)))
            }
        )
    }

    private func channelsBinding(for layout: LightMapping.PortLayout) -> Binding<Int> {
        Binding(
            get: { layout.channelsPerPixel },
            set: { newValue in
                onUpdate(LightMapping.PortLayout(
                    portNumber: layout.portNumber,
                    pixelCount: layout.pixelCount,
                    startChannel: layout.startChannel,
                    channelsPerPixel: newValue,
                    nullPixels: layout.nullPixels))
            }
        )
    }
}

private struct FramingStep: View {
    let session: LightMapping.Session
    let onStartScan: () -> Void
    @State private var startedPreview = false
    // 1.0 = Auto (default). Lower darkens to tame LED bloom — most
    // useful in dim rooms / on lower-quality cameras. Persisted so
    // the user doesn't re-tune every scan.
    @AppStorage("mapFromLightsExposure") private var exposure: Double = 1.0

    var body: some View {
        VStack(spacing: 16) {
            LightMappingPreviewRepresentable(previewView: session.previewView)
                .background(Color.black)
                .clipShape(RoundedRectangle(cornerRadius: 12))

            if session.supportsManualExposure {
                VStack(spacing: 4) {
                    HStack {
                        Image(systemName: "sun.min")
                        Slider(value: $exposure, in: 0.2...1.0) { editing in
                            if !editing { /* committed */ }
                        }
                        .onChange(of: exposure) { _, v in
                            session.setExposureFraction(Float(v))
                        }
                        Image(systemName: "sun.max")
                    }
                    Text(exposure >= 0.999
                         ? "Exposure: Auto"
                         : "Exposure: −\(String(format: "%.1f", 4 * (1 - exposure))) stops — lower to reduce LED bloom")
                        .font(.caption2)
                        .foregroundStyle(.secondary)
                }
                .padding(.horizontal)
            }

            Text("Frame the lights so every pixel is visible and in focus. The camera locks exposure once the scan starts. Moderate room lighting works best — a normal-lit room where the wall behind the prop is faintly visible at the camera's locked exposure. Pitch-dark rooms cause heavy bounce off nearby surfaces and visible lens diffraction off the bright LEDs, both of which create phantom peaks. Bright rooms wash out the LED contrast. Aim for the middle. If the LEDs bloom into each other, lower the exposure.")
                .font(.footnote)
                .foregroundStyle(.secondary)
                .multilineTextAlignment(.center)
                .padding(.horizontal)
            Button("Start Scan") { onStartScan() }
                .buttonStyle(.borderedProminent)
                .controlSize(.large)
        }
        .padding()
        .task {
            if !startedPreview {
                startedPreview = true
                try? await session.startPreview()
                // Re-apply any persisted exposure once the preview is up.
                if exposure < 0.999 { session.setExposureFraction(Float(exposure)) }
            }
        }
    }
}

/// Tiny SwiftUI wrapper so `LightMapping.PreviewView` (a UIView)
/// can sit inside SwiftUI's view tree. Kept inside the wizard
/// rather than the facade because it's a UI-host concern; a
/// macOS host using Continuity Camera would write a sibling
/// `NSViewRepresentable` against the same UIView surface.
private struct LightMappingPreviewRepresentable: UIViewRepresentable {
    let previewView: LightMapping.PreviewView

    func makeUIView(context: Context) -> LightMapping.PreviewView {
        previewView
    }

    func updateUIView(_ uiView: LightMapping.PreviewView, context: Context) {
        uiView.applyRotation()
    }
}

private struct ScanningStep: View {
    let session: LightMapping.Session
    let statusMessage: String
    let motionSensitive: Bool

    var body: some View {
        ZStack(alignment: .bottom) {
            // Keep the live camera up during the scan so the user can
            // confirm the iPad holds its aim on the prop while the
            // pattern sequence records — the most common cause of a
            // bad scan is drifting off-target mid-capture. The capture
            // session is already running (it's recording), so the
            // preview layer renders for free; no extra camera cost.
            LightMappingPreviewRepresentable(previewView: session.previewView)
                .background(Color.black)
                .clipShape(RoundedRectangle(cornerRadius: 12))

            // Compact status pill, bottom-anchored so it doesn't hide
            // the framing. Translucent material stays readable over the
            // (exposure-locked, often dim) preview.
            VStack(spacing: 8) {
                HStack(spacing: 10) {
                    ProgressView()
                    Text(statusMessage)
                        .font(.subheadline.weight(.medium))
                }
                Text(motionSensitive
                     ? "Hold steady — keep every pixel framed. Don't move the iPad or the lights."
                     : "Pattern recorded — you can lower the iPad while analysis runs.")
                    .font(.footnote)
                    .foregroundStyle(.secondary)
                    .multilineTextAlignment(.center)
            }
            .padding(.horizontal, 16)
            .padding(.vertical, 12)
            .background(.ultraThinMaterial, in: RoundedRectangle(cornerRadius: 14))
            .padding(.bottom, 24)
            .padding(.horizontal)
        }
        .padding()
    }
}

private struct ReviewStep: View {
    /// Latest single-view scan. Always non-nil on Review entry; powers
    /// the 2D wireframe preview + Summary section, and is the 3D-preview
    /// source for the 1-view-LiDAR case (uses its `points3D`).
    let result: LightMapping.Result
    /// Multi-view solve result, computed by the parent when the user
    /// toggles 3D ON with ≥2 viewpoints. Nil when toggle is off, or
    /// in the single-view case, or before the solve runs.
    let mergedResult: LightMapping.Result?
    let savedDumpDir: URL?
    /// True when this device can accumulate multiple viewpoints into
    /// a fused 3D model. Always true on iOS today — LiDAR via the
    /// depth-based solver, non-LiDAR via the depth-free fallback (the
    /// macOS Continuity-Camera path, reused on every iPad).
    let supportsMultiView: Bool
    /// Whether the "Show in 3D" toggle is user-facing. LiDAR-only —
    /// non-LiDAR has no 1-view 3D path so the toggle wouldn't have
    /// anything to do at viewpointCount=1, and at viewpointCount>=2
    /// the wizard auto-flips 3D on (matching macOS where viewpoint
    /// count IS the 2D-vs-3D signal). When false, the toggle row is
    /// hidden but `show3DPreview` is still consumed.
    let showsThreeDToggle: Bool
    /// Number of viewpoints captured so far. 1 after the first scan;
    /// ≥2 once the user has added another angle.
    let viewpointCount: Int
    /// Bound to the wizard's 2D-vs-3D state. On LiDAR the user owns
    /// it via the toggle; on non-LiDAR the wizard auto-sets it from
    /// viewpoint count (false for 1 view, true for ≥2). Either way
    /// this drives the preview swap + the Capture-Another button.
    @Binding var show3DPreview: Bool
    @Binding var saveAsFormat: LightMapping.OutputFormat
    let onAccept: () -> Void
    /// Drop *only* the most-recent viewpoint + its dump dir, then
    /// re-enter framing. If it was the only viewpoint, behaviour
    /// matches `onDiscardAll`.
    let onDiscardLast: () -> Void
    /// Drop *every* captured viewpoint + dump dirs, return to
    /// framing for a clean restart.
    let onDiscardAll: () -> Void
    /// Keep accumulated viewpoints, go capture another angle (next
    /// scan appends at view_id = viewpointCount). Only shown when
    /// `show3DPreview` is true.
    let onCaptureAnother: () -> Void

    /// The 3D point cloud to render in the preview. Prefers the
    /// multi-view merged solve when it's available; falls back to the
    /// single scan's own `points3D` (which the LiDAR depth path filled
    /// in during the single capture). Empty if the toggle's on but no
    /// 3D data exists yet (which shouldn't happen on a LiDAR device).
    private var threeDPoints: [LightMapping.Point3D] {
        if let m = mergedResult { return m.points3D }
        return result.points3D
    }

    var body: some View {
        Form {
            Section("Preview") {
                if show3DPreview && !threeDPoints.isEmpty {
                    GridPreview3D(points: threeDPoints)
                        .frame(minHeight: 240, maxHeight: 360)
                        .clipShape(RoundedRectangle(cornerRadius: 8))
                    Text("Drag to rotate, pinch to zoom. Green dot is LED 1; orange dots are estimated positions (curve-fit through neighbours).")
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                } else {
                    GridPreview(result: result)
                        .frame(minHeight: 160, maxHeight: 280)
                        .clipShape(RoundedRectangle(cornerRadius: 8))
                    Text("White dots are the decoded pixels. Faint lines connect them in wiring order — the line should trace your physical string if the scan is good.")
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                }
            }
            Section("Summary") {
                LabeledContent("Grid") {
                    Text("\(result.gridWidth) × \(result.gridHeight)")
                }
                LabeledContent("Detected") {
                    Text("\(result.measuredCount) of \(result.ledCount) LEDs")
                }
                if result.missingCount > 0 {
                    LabeledContent("Missing") {
                        Text("\(result.missingCount)").foregroundStyle(.orange)
                    }
                }
                if result.gridCollisionCount > 0 {
                    LabeledContent("Grid collisions") {
                        Text("\(result.gridCollisionCount)").foregroundStyle(.orange)
                    }
                }
            }
            // 3D toggle — LiDAR devices only. Default OFF; flipping it
            // triggers the solve (parent-owned via the binding setter)
            // and rearranges the rest of the Review UI: Capture
            // Another Angle appears. Non-LiDAR devices hide this and
            // auto-drive show3DPreview from viewpointCount instead
            // (matches macOS).
            if showsThreeDToggle {
                Section {
                    Toggle("Show in 3D", isOn: $show3DPreview)
                    Text(show3DPreview
                         ? (viewpointCount >= 2
                            ? "Triangulated from \(viewpointCount) viewpoints. Add more angles only if the cloud still looks off."
                            : "Single-view 3D from LiDAR depth. Tap Capture Another Angle to triangulate from multiple viewpoints.")
                         : "Single-view 2D mapping — fastest, no 3D processing. Flip on to recover depth from LiDAR (and add more angles for a triangulated 3D model).")
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                }
            }
            // Save-format picker — meaningful in both 2D and 3D modes.
            // Custom models support depth layers (snap3D produces a
            // depth-layered grid for volumetric props; planar clouds
            // collapse to a flat custom model). MultiPoint in 3D writes
            // exact (x,y,z); in 2D writes (x,y).
            Section("Save As") {
                Picker("Model Type", selection: $saveAsFormat) {
                    Text("Custom Model").tag(LightMapping.OutputFormat.custom)
                    Text("MultiPoint Model").tag(LightMapping.OutputFormat.multiPoint)
                }
                .pickerStyle(.segmented)
                Text(saveAsFormat == .custom
                     ? (show3DPreview
                        ? "Custom (3D): LEDs snap to a (width × height × depth) voxel grid. Planar props get a flat grid; volumetric props get depth layers."
                        : "Custom: each LED snaps to a cell in a rectangular grid. Best for matrix-shaped props.")
                     : (show3DPreview
                        ? "MultiPoint (3D): each LED keeps its triangulated (x, y, z). Best for curved or organic props where grid snapping loses shape."
                        : "MultiPoint: each LED keeps its exact decoded (x, y). Best for curved props (snowflakes, arches) where grid snapping loses shape."))
                    .font(.footnote)
                    .foregroundStyle(.secondary)
            }
            if let dumpDir = savedDumpDir {
                Section("Saved Frames") {
                    Text("Raw scan frames saved for analysis at:")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                    Text(dumpDir.path)
                        .font(.caption2.monospaced())
                        .textSelection(.enabled)
                }
            }
            Section {
                Button("Use This Mapping") { onAccept() }
                    .buttonStyle(.borderedProminent)
                // Capture Another Angle — the multi-view path's entry
                // point. On LiDAR (toggle visible), gated on the user
                // having flipped the toggle on. On non-LiDAR (no
                // toggle), always shown — viewpoint count is the only
                // 2D-vs-3D signal so the user needs the button to
                // reach the 3D form.
                if supportsMultiView && (!showsThreeDToggle || show3DPreview) {
                    Button("Capture Another Angle") { onCaptureAnother() }
                }
                if viewpointCount >= 2 {
                    // Two distinct destructive paths once the session
                    // has more than one viewpoint: pop just the bad
                    // one, or wipe everything.
                    Button("Discard Last View") { onDiscardLast() }
                    Button("Discard & Rescan", role: .destructive) { onDiscardAll() }
                } else {
                    // Single-view: the two paths collapse — both toss
                    // the only viewpoint. Match macOS phrasing.
                    Button("Discard & Rescan", role: .destructive) { onDiscardAll() }
                }
            }
        }
    }

}

/// Wire-frame preview of the mapped grid — white dots at each
/// filled cell over a dark background, faint lines connecting
/// dots in LED-ID order. Lets the user eyeball whether the scan
/// captured the prop's shape and wiring order correctly before
/// they commit the model.
private struct GridPreview: View {
    let result: LightMapping.Result

    var body: some View {
        Canvas { ctx, size in
            // Frame inset so dots near edges stay visible.
            let pad: CGFloat = 8
            let drawRect = CGRect(x: pad, y: pad,
                                  width: max(1, size.width  - 2 * pad),
                                  height: max(1, size.height - 2 * pad))

            ctx.fill(Path(CGRect(origin: .zero, size: size)),
                     with: .color(.black))

            let positions = orderedLEDPositions(in: drawRect)
            guard !positions.isEmpty else { return }

            // Wire — line through every LED in scan-ID order.
            if positions.count >= 2 {
                var path = Path()
                path.move(to: positions[0].pt)
                for p in positions.dropFirst() { path.addLine(to: p.pt) }
                ctx.stroke(path,
                           with: .color(.white.opacity(0.35)),
                           lineWidth: 1)
            }

            // Dots — emphasize the first LED so wiring direction
            // is obvious.
            let dotRadius = max(2.0, min(drawRect.width / CGFloat(result.gridWidth),
                                         drawRect.height / CGFloat(result.gridHeight)) * 0.4)
            for (i, p) in positions.enumerated() {
                let r = (i == 0) ? dotRadius * 1.4 : dotRadius
                let rect = CGRect(x: p.pt.x - r, y: p.pt.y - r,
                                  width: r * 2, height: r * 2)
                ctx.fill(Path(ellipseIn: rect),
                         with: .color(i == 0 ? .green : .white))
            }
        }
        .aspectRatio(CGFloat(result.gridWidth) / CGFloat(max(1, result.gridHeight)),
                     contentMode: .fit)
    }

    private struct LEDPosition {
        let id: Int
        let pt: CGPoint
    }

    private func orderedLEDPositions(in rect: CGRect) -> [LEDPosition] {
        guard !result.gridCells.isEmpty else { return [] }
        let layer = result.gridCells[0]
        let w = result.gridWidth
        let h = result.gridHeight
        let cellW = rect.width  / CGFloat(w)
        let cellH = rect.height / CGFloat(h)

        var out: [LEDPosition] = []
        for y in 0..<h {
            guard y < layer.count else { break }
            for x in 0..<w {
                guard x < layer[y].count else { break }
                let id = layer[y][x]
                if id > 0 {
                    let pt = CGPoint(
                        x: rect.origin.x + (CGFloat(x) + 0.5) * cellW,
                        y: rect.origin.y + (CGFloat(y) + 0.5) * cellH)
                    out.append(LEDPosition(id: id, pt: pt))
                }
            }
        }
        out.sort { $0.id < $1.id }
        return out
    }
}

/// 3D preview backed by SceneKit. Renders each LED as a small
/// sphere at its plane-frame position, with thin lines tracing
/// the wire path between consecutive IDs. The view's built-in
/// camera controls (`allowsCameraControl`) give the user
/// one-finger rotate, two-finger pan, and pinch zoom for free —
/// the user's "drag, rotate, spin" ask.
///
/// `points` come from `LightMapping.Point3D` and are
/// already centered + scaled to the prop's longest dimension, so
/// the default camera position works for any prop without
/// per-scan tuning.
private struct GridPreview3D: UIViewRepresentable {
    let points: [LightMapping.Point3D]

    func makeUIView(context: Context) -> SCNView {
        let view = SCNView(frame: .zero)
        view.allowsCameraControl = true
        view.autoenablesDefaultLighting = true
        view.backgroundColor = .black
        view.antialiasingMode = .multisampling4X
        view.scene = buildScene()
        return view
    }

    func updateUIView(_ uiView: SCNView, context: Context) {
        // Rebuild on every points-update. The scene is small
        // (≲ 500 nodes) so the rebuild is well under a frame; a
        // diff-based update isn't worth the complexity here.
        uiView.scene = buildScene()
    }

    private func buildScene() -> SCNScene {
        let scene = SCNScene()
        let root = scene.rootNode

        // Sort by ID so the wire path always traces ID 1 → 2 → …
        // — same convention as the 2D `GridPreview`.
        let sorted = points.sorted { $0.id < $1.id }

        // LED spheres. Scale chosen relative to the unit cube the
        // points live in (Point3D is normalised to [-0.5, 0.5] on
        // the larger axis), so the dots look right regardless of
        // prop size.
        let sphereRadius: CGFloat = 0.012
        let standardMat = SCNMaterial()
        standardMat.diffuse.contents = UIColor.white
        let firstMat = SCNMaterial()
        firstMat.diffuse.contents = UIColor.systemGreen
        let estimatedMat = SCNMaterial()
        estimatedMat.diffuse.contents = UIColor.systemOrange

        for (i, p) in sorted.enumerated() {
            let sphere = SCNSphere(radius: sphereRadius)
            sphere.segmentCount = 10  // low-poly; 139 spheres × 1000 tris would dominate the frame
            if i == 0 {
                sphere.materials = [firstMat]
            } else if p.estimated {
                sphere.materials = [estimatedMat]
            } else {
                sphere.materials = [standardMat]
            }
            let node = SCNNode(geometry: sphere)
            // Y flipped so "down in the camera image" maps to
            // "down in the 3D view" — otherwise the snowflake's
            // bottom arm would end up at the top of the preview.
            node.position = SCNVector3(p.x, -p.y, p.z)
            root.addChildNode(node)
        }

        // Wire-path lines as a single SCNGeometry with .line
        // primitive type. One geometry beats N×SCNCylinder draw
        // calls and looks right (thin, antialiased by the
        // multisampling).
        if sorted.count >= 2 {
            var vertices: [SCNVector3] = []
            var indices: [Int32] = []
            vertices.reserveCapacity(sorted.count)
            indices.reserveCapacity((sorted.count - 1) * 2)
            for (i, p) in sorted.enumerated() {
                vertices.append(SCNVector3(p.x, -p.y, p.z))
                if i > 0 {
                    indices.append(Int32(i - 1))
                    indices.append(Int32(i))
                }
            }
            let src = SCNGeometrySource(vertices: vertices)
            let elem = SCNGeometryElement(indices: indices, primitiveType: .line)
            let lineGeom = SCNGeometry(sources: [src], elements: [elem])
            let lineMat = SCNMaterial()
            lineMat.diffuse.contents = UIColor.white.withAlphaComponent(0.35)
            lineMat.lightingModel = .constant   // ignore scene lighting; faint white stays faint
            lineMat.isDoubleSided = true
            lineGeom.materials = [lineMat]
            root.addChildNode(SCNNode(geometry: lineGeom))
        }

        // Camera. Placed straight in front of the prop along +Z
        // (looking at the origin) at a distance that frames the
        // unit-scale point cloud comfortably. SCNView's camera
        // controls take over once the user touches the view.
        let camera = SCNCamera()
        camera.usesOrthographicProjection = false
        camera.fieldOfView = 30
        let cameraNode = SCNNode()
        cameraNode.camera = camera
        cameraNode.position = SCNVector3(0, 0, 2.2)
        cameraNode.look(at: SCNVector3Zero)
        root.addChildNode(cameraNode)
        scene.rootNode.camera = nil  // ensure scene-level camera doesn't shadow ours

        return scene
    }
}

private struct ErrorStep: View {
    let message: String
    let onDismiss: () -> Void
    let onRetry: () -> Void

    var body: some View {
        VStack(spacing: 20) {
            Image(systemName: "exclamationmark.triangle.fill")
                .font(.largeTitle)
                .foregroundStyle(.orange)
            Text(message)
                .multilineTextAlignment(.center)
                .padding(.horizontal)
            HStack {
                Button("Cancel", role: .cancel) { onDismiss() }
                Button("Retry") { onRetry() }
                    .buttonStyle(.borderedProminent)
            }
        }
        .padding()
    }
}

// MARK: - Helpers
//
// `autoSelectPorts` + `buildScanConfig` + `mostCommonColorOrder`
// + `buildSidecarAttributes` were retired in step 1 of the
// LightMapping facade extraction. The wizard now starts the
// port-config step with an empty selection (`portsToScan = []`
// in `attemptConnect`), the scan-config rewrite lives inside
// `LightMapping.Session.prepare()`, the dominant color order
// is computed by `LightMapping.Result.dominantColorOrder`, and
// the sidecar attributes are produced inside
// `LightMapping.Session.writeXModel()`.
