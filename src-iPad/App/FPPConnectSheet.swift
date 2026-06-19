import SwiftUI

// EX-4 — FPP Connect (Slice A + Slice B).
//
// Multi-phase sheet:
//   1. Discovering — broadcast-ping scan + version probe (5-10s).
//   2. Selection   — pick FPP instances and .fseq files to push,
//      plus per-instance config (Media / Cape / Proxies / UDP Out /
//      Models / Playlist).
//   3. Configuring — per-FPP applyConfig pass (Slice B): pushes the
//      pixel/panel/serial/virtual-matrix outputs + input universes
//      (bridge mode), UDP Out, Models, proxies, playlist setup.
//   4. Uploading   — per-sequence fan-out to every selected target.
//   5. Finalizing  — playlist commit + restart-if-needed.
//   6. Done        — summary with retry / close.
//
// Slice B wires the full per-instance config through the
// `applyConfigToFPP` bridge (FPP devices) and an immediate-output
// controller upload for non-FPP discovered devices (e.g.
// ESPixelStick), matching the desktop FPPConnectDialog upload loop.

// MARK: - Models

/// Swift-side mirror of one entry from
/// `XLSequenceDocument.discoverFPPInstances`. Decoded from the bridge's
/// NSDictionary. UUID is used as the persistence key for the per-instance
/// "do upload" toggle so the selection survives discovery re-runs even when
/// IPs shift via DHCP.
/// Sentinel selection value used by the Playlist picker to mean
/// "open the name-prompt alert and create a new one." Deliberately
/// noisy so it can't collide with any real playlist name a user
/// might already have on their FPP.
private let kCreateNewPlaylistSentinel = "\u{1F195} __xlights_create_new_playlist__ \u{1F195}"

struct FPPInstance: Identifiable, Hashable, Sendable {
    let ipAddress: String
    let hostName: String
    let description: String
    let platform: String
    let model: String
    let mode: String
    let version: String
    let uuid: String
    let fppType: String
    let supportedForFPPConnect: Bool
    /// Playlists hosted on the FPP (player/master mode only; empty
    /// otherwise). Surfaced into the per-instance Playlist picker so
    /// the user can opt-in to "add my uploaded sequences to playlist X".
    let playlists: [String]
    /// User-facing cape / hat model name (e.g. "K8-Pro", "F32-B",
    /// "PiHat - 64x32"). Empty when this FPP has no cape worth
    /// configuring — the Pixel Hat/Cape toggle is hidden entirely in
    /// that case, mirroring desktop's `PopulateFPPInstanceList`
    /// behavior. Non-empty values include the panel size suffix when
    /// the FPP reports one.
    let capeModel: String

    /// True when the FPP has a real cape / hat the user can configure.
    /// Drives whether the Pixel Hat/Cape toggle row even renders.
    var hasCape: Bool { !capeModel.isEmpty }

    /// True for genuine FPP instances (vs. non-FPP discovered devices
    /// like ESPixelStick). Non-FPP devices only get the single
    /// "Upload Controller Config" immediate-output toggle — none of the
    /// FPP-specific Models / UDP Out / Cape / Playlist config applies.
    var isFPP: Bool { fppType == "FPP" }

    var id: String { uuid }

    /// True for FPPs that can host a playlist (player/master mode).
    /// Disables the Playlist picker for remote / bridge-mode FPPs.
    var canHostPlaylist: Bool {
        let m = mode.lowercased()
        return m == "player" || m == "master" || !playlists.isEmpty
    }

    var displayName: String {
        hostName.isEmpty ? ipAddress : hostName
    }

    var subtitle: String {
        var parts: [String] = []
        parts.append(ipAddress)
        if !platform.isEmpty { parts.append(platform) }
        if !version.isEmpty { parts.append("v\(version)") }
        if !mode.isEmpty { parts.append(mode) }
        return parts.joined(separator: " · ")
    }

    init?(dict: [String: Any]) {
        guard let ip = dict["ipAddress"] as? String,
              let host = dict["hostName"] as? String,
              let uuid = dict["uuid"] as? String else { return nil }
        self.ipAddress = ip
        self.hostName = host
        self.description = (dict["description"] as? String) ?? ""
        self.platform = (dict["platform"] as? String) ?? ""
        self.model = (dict["model"] as? String) ?? ""
        self.mode = (dict["mode"] as? String) ?? ""
        self.version = (dict["version"] as? String) ?? ""
        self.uuid = uuid
        self.fppType = (dict["fppType"] as? String) ?? "FPP"
        self.supportedForFPPConnect = (dict["supportedForFPPConnect"] as? NSNumber)?.boolValue ?? false
        self.playlists = (dict["playlists"] as? [String]) ?? []
        self.capeModel = (dict["capeModel"] as? String) ?? ""
    }
}

/// Per-instance configuration for FPP Connect. One per FPP UUID,
/// persisted as JSON inside `FPPInstanceConfigStore`. Mirrors the
/// desktop dialog's per-row columns: Media checkbox, Models dropdown
/// (None/All/Local), UDP Out dropdown (None/All/Proxied), Pixel
/// Hat/Cape checkbox, Add Proxies checkbox, Playlist combobox.
///
/// All fields are now honored: `uploadMedia` rides along with the
/// fseq upload; the rest drive the per-FPP `applyConfigToFPP` config
/// pass (Cape → pixel/panel/serial/virtual-matrix + input universes,
/// Add Proxies, UDP Out, Models, Playlist). For non-FPP discovered
/// devices `uploadCape` instead triggers an immediate-output
/// controller upload.
struct FPPInstanceConfig: Codable, Equatable, Sendable {
    var uploadMedia: Bool = false
    var modelsMode: ModelsMode = .none
    var udpOutMode: UDPOutMode = .none
    var uploadCape: Bool = false
    var uploadProxies: Bool = false
    /// Empty string means "don't add uploaded sequences to a playlist".
    var playlist: String = ""

    enum ModelsMode: String, Codable, CaseIterable, Identifiable, Sendable {
        case none, all, local
        var id: String { rawValue }
        var label: String {
            switch self {
            case .none:  return "None"
            case .all:   return "All Models"
            case .local: return "Local Models Only"
            }
        }
    }

    enum UDPOutMode: String, Codable, CaseIterable, Identifiable, Sendable {
        case none, all, proxied
        var id: String { rawValue }
        var label: String {
            switch self {
            case .none:    return "None"
            case .all:     return "All Controllers"
            case .proxied: return "Proxied Only"
            }
        }
    }
}

/// Per-UUID config store, backed by a single UserDefaults JSON entry.
/// Migrates the legacy `xLights-FPPConnect-mediaPerUUID` UUID-set into
/// the new map on first read so users who opted into Media uploads in
/// B1 don't lose the setting.
enum FPPInstanceConfigStore {
    private static let key = "xLights-FPPConnect-instanceConfig.v1"
    private static let legacyMediaKey = "xLights-FPPConnect-mediaPerUUID"

    static func loadAll() -> [String: FPPInstanceConfig] {
        var dict: [String: FPPInstanceConfig] = [:]
        if let data = UserDefaults.standard.data(forKey: key),
           let decoded = try? JSONDecoder().decode([String: FPPInstanceConfig].self, from: data) {
            dict = decoded
        }
        // One-time migration of the B1 media-only persistence.
        if let legacy = UserDefaults.standard.stringArray(forKey: legacyMediaKey) {
            for uuid in legacy where dict[uuid] == nil {
                var cfg = FPPInstanceConfig()
                cfg.uploadMedia = true
                dict[uuid] = cfg
            }
            UserDefaults.standard.removeObject(forKey: legacyMediaKey)
            persist(dict)
        }
        return dict
    }

    static func config(for uuid: String) -> FPPInstanceConfig {
        loadAll()[uuid] ?? FPPInstanceConfig()
    }

    static func set(_ config: FPPInstanceConfig, for uuid: String) {
        var all = loadAll()
        all[uuid] = config
        persist(all)
    }

    private static func persist(_ all: [String: FPPInstanceConfig]) {
        if let data = try? JSONEncoder().encode(all) {
            UserDefaults.standard.set(data, forKey: key)
        }
    }
}

/// Persisted per-UUID upload checkbox state. The user's choice of which
/// FPPs to push to typically doesn't change between runs, so we
/// remember it. Keyed globally (not per-show) — an FPP is the same
/// physical device regardless of which show is loaded.
enum FPPConnectSelections {
    private static let key = "xLights-FPPConnect-instanceUUIDs"

    static func load() -> Set<String> {
        Set(UserDefaults.standard.stringArray(forKey: key) ?? [])
    }

    static func save(_ selection: Set<String>) {
        UserDefaults.standard.set(Array(selection).sorted(), forKey: key)
    }
}


// MARK: - Upload progress forwarder

/// Thread-safe bridge from the ObjC bridge's `XLFPPUploadProgress`
/// protocol up to the SwiftUI runner. Progress is per-IP since one
/// upload call may fan out to N FPPs in parallel (Slice F); the runner
/// reads `progress(forIP:)` from MainActor every ~150ms and renders
/// per-FPP gauges. Cancellation is one shared flag — the bridge polls
/// `isCancelled` between frame batches and on each curl pump tick.
///
/// Conforms to `@unchecked Sendable` because every mutable field is
/// guarded by `NSLock` — the access pattern is concurrent-by-design.
@objc final class FPPUploadProgressForwarder: NSObject, XLFPPUploadProgress, @unchecked Sendable {
    private let lock = NSLock()
    /// Per-IP progress, 0..100. Routed by the bridge based on which
    /// FPP's internal progress callback fired.
    private var _progress: [String: Int32] = [:]
    private var _cancelled: Bool = false

    @objc func setProgress(_ value: Int32, forIPAddress ip: String) {
        lock.lock()
        _progress[ip] = value
        lock.unlock()
    }

    @objc func isCancelled() -> Bool {
        lock.lock()
        defer { lock.unlock() }
        return _cancelled
    }

    /// Read the last value the bridge reported for one FPP (0..100).
    /// Returns 0 for FPPs the bridge hasn't reported on yet.
    func progress(forIP ip: String) -> Int {
        lock.lock()
        defer { lock.unlock() }
        return Int(_progress[ip] ?? 0)
    }

    /// Snapshot of every reported per-IP progress value. Used by the
    /// SwiftUI sheet's per-FPP gauge row.
    var allProgress: [String: Int] {
        lock.lock()
        defer { lock.unlock() }
        var out: [String: Int] = [:]
        for (k, v) in _progress { out[k] = Int(v) }
        return out
    }

    func reset() {
        lock.lock()
        _progress.removeAll()
        _cancelled = false
        lock.unlock()
    }

    func cancel() {
        lock.lock()
        _cancelled = true
        lock.unlock()
    }
}

// MARK: - Runner

@MainActor
@Observable
final class FPPConnectRunner {
    enum Phase: Equatable {
        case idle
        case discovering
        case selection
        /// "Configuring N of M — <FPP name>". Per-FPP applyConfig
        /// (Cape, Proxies, UDP Out, Models, Playlist setup pass).
        /// FPPs whose config is all-default skip this phase.
        case configuring(targetIndex: Int, targetCount: Int,
                         instanceName: String)
        /// Slice F — one sequence fans out to every selected FPP in
        /// parallel. The phase carries the *sequence index* + the live
        /// target list so the UI can show per-FPP gauges. Per-target
        /// progress is read separately from `currentSequenceProgress`
        /// since `@Observable` doesn't diff associated values cheaply.
        case uploading(sequenceIndex: Int, sequenceCount: Int,
                       sequenceName: String, targets: [FPPInstance])
        /// Post-sequence finalize — UploadPlaylist (commits the
        /// just-uploaded fseqs into the playlist) + Restart(true) per
        /// FPP. Mirrors FPPConnectDialog.cpp:1427-1432. FPPs with no
        /// playlist still get a restart so a config-only restart flag
        /// from earlier in the run actually fires.
        case finalizing(targetIndex: Int, targetCount: Int,
                         instanceName: String)
        case done(succeeded: Int, failed: Int, cancelled: Bool,
                  failures: [String])
    }

    private(set) var phase: Phase = .idle
    private(set) var instances: [FPPInstance] = []
    /// Single-target task progress (0..1). Used during `.configuring`
    /// and `.finalizing` phases, where one FPP is in flight at a time.
    /// Reads the lone reporting IP from the forwarder.
    private(set) var currentTaskProgress: Double = 0
    /// Per-FPP-IP progress map (0..100). Populated during `.uploading`
    /// when frames fan out to every selected target at once.
    private(set) var currentSequenceProgress: [String: Int] = [:]

    private let document: XLSequenceDocument
    private let forwarder = FPPUploadProgressForwarder()
    private var task: Task<Void, Never>?
    private var pollTask: Task<Void, Never>?

    /// CTL-5 — user-entered FPP IPs/hostnames that weren't auto-discovered,
    /// persisted across launches. Broadcast discovery still runs alongside.
    private(set) var forcedIPs: [String] = []
    private static let forcedIPsKey = "FPPConnectForcedIPs"

    init(document: XLSequenceDocument) {
        self.document = document
        self.forcedIPs = UserDefaults.standard.stringArray(forKey: Self.forcedIPsKey) ?? []
    }

    /// CTL-5 — append a forced IP (deduped), persist, and re-run discovery.
    func addForcedIP(_ ip: String) {
        let trimmed = ip.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !trimmed.isEmpty, !forcedIPs.contains(trimmed) else { return }
        forcedIPs.append(trimmed)
        UserDefaults.standard.set(forcedIPs, forKey: Self.forcedIPsKey)
        discover()
    }

    /// CTL-5 — forget a forced IP (persists; does not auto-rediscover).
    func removeForcedIP(_ ip: String) {
        forcedIPs.removeAll { $0 == ip }
        UserDefaults.standard.set(forcedIPs, forKey: Self.forcedIPsKey)
    }

    func discover() {
        guard task == nil else { return }
        phase = .discovering
        task = Task { @MainActor in
            // Parse the NSDictionary results into Sendable Swift values
            // inside the detached task so we can return them cleanly
            // across the actor boundary (Swift 6 won't allow
            // [AnyHashable: Any] to cross).
            let parsed: [FPPInstance] = await Task.detached(priority: .userInitiated) { [document, forcedIPs] in
                let raw = document.discoverFPPInstances(withForcedAddresses: forcedIPs)
                return raw.compactMap { entry -> FPPInstance? in
                    guard let dict = entry as? [String: Any] else { return nil }
                    return FPPInstance(dict: dict)
                }
            }.value
            instances = parsed
            phase = .selection
            task = nil
        }
    }

    /// Sendable result of one upload pass. Built inside the detached
    /// task that calls the bridge; ferried back to the main actor.
    struct UploadOutcome: Sendable {
        let ok: Bool
        let cancelled: Bool
        let message: String
    }

    /// Sendable result of a multi-target sequence upload. The bridge
    /// returns @{ @"outcomes": { ip → {ok, cancelled, message} },
    /// @"cancelled": BOOL, @"globalError": NSString? } — parsed into
    /// pure-value form inside the detached task so the result can
    /// cross the actor boundary cleanly.
    struct MultiUploadResult: Sendable {
        let outcomes: [String: UploadOutcome]
        let cancelled: Bool
        let globalError: String?

        static func from(bridgeResult: [AnyHashable: Any]) -> MultiUploadResult {
            var parsed: [String: UploadOutcome] = [:]
            if let raw = bridgeResult["outcomes"] as? [String: Any] {
                for (ip, value) in raw {
                    guard let dict = value as? [String: Any] else { continue }
                    parsed[ip] = UploadOutcome(
                        ok: (dict["ok"] as? NSNumber)?.boolValue ?? false,
                        cancelled: (dict["cancelled"] as? NSNumber)?.boolValue ?? false,
                        message: (dict["message"] as? String) ?? "")
                }
            }
            return MultiUploadResult(
                outcomes: parsed,
                cancelled: (bridgeResult["cancelled"] as? NSNumber)?.boolValue ?? false,
                globalError: bridgeResult["globalError"] as? String)
        }
    }

    func startUpload(targets: [FPPInstance],
                      sequences: [SequenceEntry],
                      configs: [String: FPPInstanceConfig]) {
        guard task == nil, !targets.isEmpty, !sequences.isEmpty else { return }
        forwarder.reset()

        // Poll the forwarder for per-target progress every ~150 ms.
        // During config / finalize phases there's one IP reporting at
        // a time — we surface it as `currentTaskProgress`. During the
        // upload phase, every selected target reports; the per-IP map
        // feeds the per-FPP gauge row in the sheet.
        pollTask = Task { @MainActor in
            while !Task.isCancelled {
                let map = self.forwarder.allProgress
                self.currentSequenceProgress = map
                // `currentTaskProgress` echoes a single reporter for
                // the configuring / finalizing phases.
                if let single = map.values.max() {
                    self.currentTaskProgress = Double(single) / 100.0
                } else {
                    self.currentTaskProgress = 0
                }
                try? await Task.sleep(nanoseconds: 150_000_000)
            }
        }

        task = Task { @MainActor in
            var succeeded = 0
            var failed = 0
            var failures: [String] = []

            // Phase 1 — per-FPP configuration. Mirrors the desktop
            // dialog's pre-sequence config loop (FPPConnectDialog.cpp
            // 1148-1212): apply settings, restart (ifNeeded), then
            // refresh channel ranges. Any failure here is recorded and
            // the FPP is skipped for the rest of this run so we don't
            // pump frames into a half-configured target.
            var skipTargets: Set<String> = []
            for (idx, tgt) in targets.enumerated() {
                if forwarder.isCancelled() { break }
                let cfg = configs[tgt.uuid] ?? FPPInstanceConfig()
                let needsConfig = cfg.uploadCape || cfg.uploadProxies
                    || cfg.modelsMode != .none
                    || cfg.udpOutMode != .none
                    || !cfg.playlist.isEmpty
                if !needsConfig {
                    continue
                }
                currentTaskProgress = 0
                forwarder.reset()
                phase = .configuring(targetIndex: idx + 1,
                                      targetCount: targets.count,
                                      instanceName: tgt.displayName)
                await Task.yield()

                let outcome: UploadOutcome = await Task.detached(priority: .userInitiated) { [document, forwarder] in
                    let raw = document.applyConfig(toFPP: tgt.ipAddress,
                                                    settings: [
                                                        "uploadCape": cfg.uploadCape,
                                                        "uploadProxies": cfg.uploadProxies,
                                                        "modelsMode": cfg.modelsMode.rawValue,
                                                        "udpOutMode": cfg.udpOutMode.rawValue,
                                                        "playlist": cfg.playlist,
                                                    ],
                                                    progress: forwarder)
                    let dict = raw as? [String: Any] ?? [:]
                    return UploadOutcome(
                        ok: (dict["ok"] as? NSNumber)?.boolValue ?? false,
                        cancelled: (dict["cancelled"] as? NSNumber)?.boolValue ?? false,
                        message: (dict["message"] as? String) ?? "")
                }.value

                if outcome.cancelled { break }
                if !outcome.ok {
                    skipTargets.insert(tgt.uuid)
                    let msg = outcome.message.isEmpty ? "Configuration failed" : outcome.message
                    failures.append("\(tgt.displayName) (config): \(msg)")
                    continue
                }
                // Post-restart channel-range refresh. Even a clean run
                // with no actual restart is safe to call — the FPP just
                // re-reports its current ranges.
                _ = document.updateChannelRanges(forFPP: tgt.ipAddress)
            }

            // Phase 2 — sequence upload. For each sequence, fan out to
            // every live target (skipping those that failed Phase 1).
            // The bridge transcodes per target in parallel via
            // dispatch_apply and drives the curl pump so the bulk
            // fseq transfers happen concurrently over the wire.
            let liveTargets = targets.filter { !skipTargets.contains($0.uuid) }
            let sequenceCount = sequences.count
            for (i, sequence) in sequences.enumerated() {
                if forwarder.isCancelled() { break }
                if liveTargets.isEmpty { break }
                forwarder.reset()
                currentSequenceProgress = [:]
                currentTaskProgress = 0
                phase = .uploading(sequenceIndex: i + 1,
                                    sequenceCount: sequenceCount,
                                    sequenceName: sequence.displayName,
                                    targets: liveTargets)
                await Task.yield()

                // Resolve per-target media paths on the main actor —
                // FPPInstanceConfig.uploadMedia gates whether the .xsq's
                // media reference rides along. `document.mediaPath` is
                // a cheap XML scan + filesystem lookups; keeping it on
                // MainActor avoids a second capture of `document` into
                // the detached closure.
                let resolvedMedia: String? = {
                    let anyWantsMedia = liveTargets.contains { tgt in
                        configs[tgt.uuid]?.uploadMedia == true
                    }
                    guard anyWantsMedia else { return nil }
                    return document.mediaPath(forXsq: sequence.fullPath)
                }()

                // Build a Sendable Swift representation of the target
                // list; the NSDictionary form (with Any values) is
                // constructed inside the detached task so we don't
                // cross the actor boundary with non-Sendable values.
                let targetPairs: [(ip: String, media: String?)] = liveTargets.map { tgt in
                    let wantsMedia = configs[tgt.uuid]?.uploadMedia == true
                    return (ip: tgt.ipAddress,
                             media: wantsMedia ? resolvedMedia : nil)
                }

                let fseqPath = batchRenderFseqPath(forXsq: sequence.fullPath)
                let result: MultiUploadResult = await Task.detached(priority: .userInitiated) { [document, forwarder] in
                    let dicts: [[String: Any]] = targetPairs.map { pair in
                        var d: [String: Any] = ["ipAddress": pair.ip]
                        if let m = pair.media { d["mediaPath"] = m }
                        return d
                    }
                    let raw = document.uploadFseq(fseqPath,
                                                    toFPPInstances: dicts,
                                                    progress: forwarder)
                    return MultiUploadResult.from(bridgeResult: raw)
                }.value

                if result.cancelled { break }

                // Roll the per-target outcomes into succeeded / failed
                // counters and failure list. A successful (target, seq)
                // counts as one "succeeded" so the summary scales with
                // total work, not just sequence count.
                for tgt in liveTargets {
                    if let outcome = result.outcomes[tgt.ipAddress] {
                        if outcome.ok {
                            succeeded += 1
                        } else if !outcome.cancelled {
                            failed += 1
                            let msg = outcome.message.isEmpty ? "Upload failed" : outcome.message
                            failures.append("\(tgt.displayName) ← \(sequence.displayName): \(msg)")
                        }
                    } else {
                        failed += 1
                        failures.append("\(tgt.displayName) ← \(sequence.displayName): no outcome reported")
                    }
                }
                if let globalErr = result.globalError, !globalErr.isEmpty {
                    failures.append("\(sequence.displayName): \(globalErr)")
                }
            }

            // Phase 3 — finalize. Per-FPP post-sequence UploadPlaylist
            // (commits the just-uploaded fseqs to the playlist) +
            // Restart(true). Skips FPPs that failed Phase 1. Also
            // skips when cancelled — partial state is recorded by the
            // earlier phases.
            if !forwarder.isCancelled() {
                let finalizeTargets = targets.filter { !skipTargets.contains($0.uuid) }
                for (idx, tgt) in finalizeTargets.enumerated() {
                    if forwarder.isCancelled() { break }
                    let cfg = configs[tgt.uuid] ?? FPPInstanceConfig()
                    currentTaskProgress = 0
                    forwarder.reset()
                    phase = .finalizing(targetIndex: idx + 1,
                                         targetCount: finalizeTargets.count,
                                         instanceName: tgt.displayName)
                    await Task.yield()

                    let ok: Bool = await Task.detached(priority: .userInitiated) { [document] in
                        document.finalize(fpp: tgt.ipAddress,
                                           playlist: cfg.playlist.isEmpty ? nil : cfg.playlist)
                    }.value
                    if !ok {
                        failures.append("\(tgt.displayName) (finalize): playlist or restart returned an error")
                    }
                }
            }

            pollTask?.cancel()
            pollTask = nil
            phase = .done(succeeded: succeeded,
                          failed: failed,
                          cancelled: forwarder.isCancelled(),
                          failures: failures)
            task = nil
        }
    }

    func cancel() {
        forwarder.cancel()
    }

    func resetToSelection() {
        guard task == nil else { return }
        phase = .selection
        forwarder.reset()
        currentTaskProgress = 0
    }

    func releaseInstances() {
        document.releaseFPPInstances()
    }
}

// MARK: - Sheet

struct FPPConnectSheet: View {
    @Environment(SequencerViewModel.self) var viewModel
    @Environment(\.dismiss) var dismiss

    @State private var runner: FPPConnectRunner?
    @State private var sequences: [SequenceEntry] = []
    @State private var selectedInstanceUUIDs: Set<String> = []
    @State private var selectedSequencePaths: Set<String> = []
    /// All known per-instance configs, keyed by UUID. Loaded from
    /// `FPPInstanceConfigStore` on appear; mutations write through
    /// immediately. Independent of `selectedInstanceUUIDs` so the user
    /// can configure an FPP without selecting it for this upload (and
    /// vice versa).
    @State private var instanceConfigs: [String: FPPInstanceConfig] = [:]
    /// Which rows are currently expanded to show per-instance settings.
    /// Persisted across sheet present/dismiss within the app session,
    /// but not across launches — the per-row state is just transient UI.
    @State private var expandedUUIDs: Set<String> = []
    /// Drives the "create new playlist" alert. When non-nil, the alert
    /// is showing for that UUID; the user's typed name gets committed
    /// to that instance's config on Create. The actual playlist gets
    /// materialized on the FPP when the Slice B4 `UploadPlaylist`
    /// bridge call lands.
    @State private var newPlaylistTargetUUID: String? = nil
    @State private var newPlaylistName: String = ""
    /// CTL-5 — drives the "Add FPP by IP" alert and its text field.
    @State private var showingAddFPPByIP = false
    @State private var addFPPIPText = ""

    var body: some View {
        NavigationStack {
            Group {
                if let runner = runner {
                    switch runner.phase {
                    case .idle, .discovering:
                        discoveringView()
                    case .selection:
                        selectionView(runner: runner)
                    case .configuring(let i, let total, let inst):
                        configuringView(runner: runner,
                                         targetIndex: i, targetCount: total,
                                         instanceName: inst)
                    case .uploading(let i, let total, let seq, let tgts):
                        uploadingView(runner: runner,
                                      sequenceIndex: i, sequenceCount: total,
                                      sequenceName: seq, targets: tgts)
                    case .finalizing(let i, let total, let inst):
                        finalizingView(targetIndex: i, targetCount: total,
                                        instanceName: inst)
                    case .done(let succeeded, let failed, let cancelled, let failures):
                        doneView(runner: runner,
                                 succeeded: succeeded, failed: failed,
                                 cancelled: cancelled, failures: failures)
                    }
                } else {
                    ProgressView().controlSize(.large)
                }
            }
            .navigationTitle("FPP Connect")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    if isUploading {
                        Button("Cancel", role: .destructive) {
                            runner?.cancel()
                        }
                    } else {
                        Button("Close") {
                            runner?.releaseInstances()
                            dismiss()
                        }
                    }
                }
            }
        }
        .interactiveDismissDisabled(isUploading)
        .onAppear {
            // Register the auth prompt before kicking discovery so a
            // 401 from a password-protected FPP routes through the
            // UIAlertController instead of failing silently. Cleared
            // in `.onDisappear` so a dismissed sheet stops popping
            // prompts mid-app.
            viewModel.document.setFPPAuthPromptHandler { host, completion in
                FPPAuthPrompt.present(host: host, completion: completion)
            }
            if runner == nil {
                runner = FPPConnectRunner(document: viewModel.document)
                runner?.discover()
            }
            selectedInstanceUUIDs = FPPConnectSelections.load()
            instanceConfigs = FPPInstanceConfigStore.loadAll()
            sequences = SequenceScanner.scan(showFolder: viewModel.showFolderPath ?? "")
        }
        .onDisappear {
            viewModel.document.setFPPAuthPromptHandler(nil)
        }
    }

    private var isUploading: Bool {
        switch runner?.phase {
        case .uploading, .configuring, .finalizing: return true
        default: return false
        }
    }

    // MARK: Phase 1 — Discovering

    private func discoveringView() -> some View {
        VStack(spacing: 16) {
            Spacer()
            ProgressView().controlSize(.large)
            Text("Discovering FPP instances on the network…")
                .font(.body)
                .foregroundStyle(.secondary)
            Text("Listening for FPP broadcasts. This usually takes about 5 seconds.")
                .font(.caption)
                .foregroundStyle(.tertiary)
                .multilineTextAlignment(.center)
                .padding(.horizontal, 32)
            Spacer()
        }
    }

    // MARK: Phase 2 — Selection

    @ViewBuilder
    private func selectionView(runner: FPPConnectRunner) -> some View {
        List {
            Section {
                Button {
                    runner.discover()
                } label: {
                    Label("Re-Discover", systemImage: "arrow.clockwise")
                }
                Button {
                    addFPPIPText = ""
                    showingAddFPPByIP = true
                } label: {
                    Label("Add FPP by IP…", systemImage: "plus.circle")
                }
                ForEach(runner.forcedIPs, id: \.self) { ip in
                    HStack {
                        Image(systemName: "network").foregroundStyle(.secondary)
                        Text(ip)
                        Spacer()
                    }
                    .swipeActions {
                        Button(role: .destructive) {
                            runner.removeForcedIP(ip)
                        } label: { Label("Remove", systemImage: "trash") }
                    }
                }
            } footer: {
                Text("Add an FPP / ESPixelStick that isn't auto-discovered (e.g. on a different subnet). Broadcast discovery still runs.")
            }
            Section("FPP Instances") {
                if runner.instances.isEmpty {
                    ContentUnavailableView("No FPP instances found",
                                            systemImage: "antenna.radiowaves.left.and.right",
                                            description: Text("Make sure your iPad and your FPP devices are on the same network."))
                } else {
                    ForEach(runner.instances) { inst in
                        VStack(alignment: .leading, spacing: 0) {
                            instanceRowHeader(inst)
                            if expandedUUIDs.contains(inst.uuid) {
                                instanceSettings(inst)
                                    .padding(.leading, 36)
                                    .padding(.top, 8)
                            }
                        }
                    }
                }
            }
            Section("Sequences") {
                if sequences.isEmpty {
                    Text("No .xsq files in show folder.")
                        .font(.callout)
                        .foregroundStyle(.secondary)
                } else {
                    Toggle("Only show ready (rendered) sequences",
                            isOn: $showOnlyReady)
                    ForEach(visibleSequences) { entry in
                        let stale = !entry.isFseqUpToDate
                        let missing = entry.fseqModificationDate == nil
                        Button {
                            if !missing { toggleSequence(entry) }
                        } label: {
                            HStack(spacing: 12) {
                                Image(systemName: selectedSequencePaths.contains(entry.relativePath)
                                      ? "checkmark.square.fill" : "square")
                                    .foregroundStyle(selectedSequencePaths.contains(entry.relativePath)
                                                     ? Color.accentColor : Color.secondary)
                                VStack(alignment: .leading, spacing: 2) {
                                    Text(entry.displayName)
                                        .foregroundStyle(missing
                                                         ? Color.secondary
                                                         : (stale ? Color.orange : Color.primary))
                                    SequenceDatesLabel(entry: entry)
                                    if missing {
                                        Text("No .fseq on disk — batch-render first.")
                                            .font(.caption2)
                                            .foregroundStyle(.orange)
                                    }
                                }
                                Spacer()
                            }
                            .contentShape(Rectangle())
                        }
                        .buttonStyle(.plain)
                        .disabled(missing)
                    }
                }
            }
        }
        .safeAreaInset(edge: .bottom) {
            let targets = runner.instances.filter {
                selectedInstanceUUIDs.contains($0.uuid) && $0.supportedForFPPConnect
            }
            let toUpload = sequences.filter {
                selectedSequencePaths.contains($0.relativePath)
                && $0.fseqModificationDate != nil
            }
            let disabled = targets.isEmpty || toUpload.isEmpty
            Button {
                runner.startUpload(targets: targets,
                                    sequences: toUpload,
                                    configs: instanceConfigs)
            } label: {
                Text(disabled
                     ? "Select at least one FPP and one sequence"
                     : "Upload \(toUpload.count) to \(targets.count) FPP\(targets.count == 1 ? "" : "s")")
                    .font(.headline)
                    .frame(maxWidth: .infinity, minHeight: 44)
            }
            .buttonStyle(.borderedProminent)
            .disabled(disabled)
            .padding(.horizontal)
            .padding(.vertical, 8)
            .background(.bar)
        }
        .alert("Add FPP by IP", isPresented: $showingAddFPPByIP) {
            TextField("IP address or hostname", text: $addFPPIPText)
                .textInputAutocapitalization(.never)
                .autocorrectionDisabled()
            Button("Cancel", role: .cancel) { addFPPIPText = "" }
            Button("Add") {
                let ip = addFPPIPText.trimmingCharacters(in: .whitespaces)
                addFPPIPText = ""
                if isValidIPOrHostname(ip) { runner.addForcedIP(ip) }
            }
        } message: {
            Text("Enter an FPP / ESPixelStick address that wasn't auto-discovered. Broadcast discovery still runs too.")
        }
    }

    /// CTL-5 — light validation mirroring desktop's IsIPValidOrHostname:
    /// dotted IPv4 digits, or a hostname (alphanumerics, dots, hyphens).
    private func isValidIPOrHostname(_ s: String) -> Bool {
        guard !s.isEmpty else { return false }
        return s.range(of: "^[0-9.]+$|^[A-Za-z0-9][A-Za-z0-9.\\-]*$",
                       options: .regularExpression) != nil
    }

    @State private var showOnlyReady: Bool = false

    private var visibleSequences: [SequenceEntry] {
        showOnlyReady
            ? sequences.filter { $0.fseqModificationDate != nil }
            : sequences
    }

    private func toggleInstance(_ inst: FPPInstance) {
        if selectedInstanceUUIDs.contains(inst.uuid) {
            selectedInstanceUUIDs.remove(inst.uuid)
        } else {
            selectedInstanceUUIDs.insert(inst.uuid)
        }
        FPPConnectSelections.save(selectedInstanceUUIDs)
    }

    private func toggleSequence(_ entry: SequenceEntry) {
        if selectedSequencePaths.contains(entry.relativePath) {
            selectedSequencePaths.remove(entry.relativePath)
        } else {
            selectedSequencePaths.insert(entry.relativePath)
        }
    }

    // MARK: - Per-instance row + settings

    /// Header HStack for one FPP row: selection checkbox + name/subtitle
    /// + chevron toggle for the per-instance settings drawer. The two
    /// halves are independently tappable so a user can configure an FPP
    /// without selecting it for the current upload.
    @ViewBuilder
    private func instanceRowHeader(_ inst: FPPInstance) -> some View {
        HStack(spacing: 12) {
            Button {
                toggleInstance(inst)
            } label: {
                HStack(spacing: 12) {
                    Image(systemName: selectedInstanceUUIDs.contains(inst.uuid)
                          ? "checkmark.square.fill" : "square")
                        .foregroundStyle(selectedInstanceUUIDs.contains(inst.uuid)
                                         ? Color.accentColor : Color.secondary)
                    VStack(alignment: .leading, spacing: 2) {
                        Text(inst.displayName)
                            .foregroundStyle(inst.supportedForFPPConnect
                                             ? Color.primary : Color.secondary)
                        Text(inst.subtitle)
                            .font(.caption2)
                            .foregroundStyle(.secondary)
                        if !inst.supportedForFPPConnect {
                            Text("Mode or version does not support FPP Connect uploads.")
                                .font(.caption2)
                                .foregroundStyle(.orange)
                        }
                    }
                    Spacer(minLength: 0)
                }
                .contentShape(Rectangle())
            }
            .buttonStyle(.plain)
            .disabled(!inst.supportedForFPPConnect)

            Button {
                withAnimation(.snappy(duration: 0.18)) {
                    if expandedUUIDs.contains(inst.uuid) {
                        expandedUUIDs.remove(inst.uuid)
                    } else {
                        expandedUUIDs.insert(inst.uuid)
                    }
                }
            } label: {
                Image(systemName: "chevron.right")
                    .rotationEffect(.degrees(expandedUUIDs.contains(inst.uuid) ? 90 : 0))
                    .font(.body.weight(.semibold))
                    .foregroundStyle(.secondary)
                    .frame(width: 32, height: 32)
                    .contentShape(Rectangle())
            }
            .buttonStyle(.plain)
            .disabled(!inst.supportedForFPPConnect)
            .accessibilityLabel(expandedUUIDs.contains(inst.uuid)
                                ? "Collapse \(inst.displayName) settings"
                                : "Expand \(inst.displayName) settings")
        }
    }

    /// "Pixel Hat / Cape Outputs - K8-Pro". Called only when the FPP
    /// has a cape (the toggle row is hidden otherwise; see `hasCape`).
    /// Mirrors desktop's per-row cape-model string from
    /// `PopulateFPPInstanceList`.
    private func capeLabel(for inst: FPPInstance) -> String {
        "Pixel Hat / Cape Outputs - \(inst.capeModel)"
    }

    /// Binding into `instanceConfigs[uuid]` that writes through to the
    /// persistent store on every change. Defaults to a fresh
    /// `FPPInstanceConfig()` for unseen UUIDs so the picker / toggle
    /// always has something to read.
    private func configBinding(for uuid: String) -> Binding<FPPInstanceConfig> {
        Binding(
            get: { instanceConfigs[uuid] ?? FPPInstanceConfig() },
            set: { newValue in
                instanceConfigs[uuid] = newValue
                FPPInstanceConfigStore.set(newValue, for: uuid)
            }
        )
    }

    /// Per-instance settings drawer. Every field is wired through to
    /// the upload pipeline: Media rides with the fseq upload, the rest
    /// drive the per-FPP `applyConfigToFPP` config pass. Non-FPP
    /// devices show only the single immediate-output controller toggle.
    @ViewBuilder
    private func instanceSettings(_ inst: FPPInstance) -> some View {
        let cfg = configBinding(for: inst.uuid)
        if !inst.isFPP {
            // Non-FPP discovered device (e.g. ESPixelStick). Only an
            // immediate-output controller upload applies — none of the
            // FPP-specific Models / UDP Out / Cape / Playlist config.
            VStack(alignment: .leading, spacing: 10) {
                Toggle("Upload Controller Config", isOn: cfg.uploadCape)
                    .help("Push the show's pixel-output configuration to this controller (immediate output).")
                Text("This device isn't an FPP. Only the controller's pixel-output config can be pushed; FPP Models / UDP Out / Playlist options don't apply.")
                    .font(.caption2)
                    .foregroundStyle(.tertiary)
                    .padding(.top, 4)
            }
            .padding(.bottom, 4)
        } else {
        VStack(alignment: .leading, spacing: 10) {
            Toggle("Media (MP3 / MP4 / WAV)", isOn: cfg.uploadMedia)
                .help("Upload the sequence's audio companion alongside the .fseq. Typically only the player / master FPP needs this.")

            if inst.hasCape {
                Toggle(capeLabel(for: inst), isOn: cfg.uploadCape)
                    .help("Push pixel, panel, virtual-matrix and serial output configuration. Restarts FPPD.")
            }

            Toggle("Add Show Proxies", isOn: cfg.uploadProxies)
                .help("Upload proxy IP addresses configured in the show.")

            HStack {
                Text("Models")
                Spacer()
                Picker("Models", selection: cfg.modelsMode) {
                    ForEach(FPPInstanceConfig.ModelsMode.allCases) { mode in
                        Text(mode.label).tag(mode)
                    }
                }
                .labelsHidden()
                .pickerStyle(.menu)
            }

            HStack {
                Text("UDP Out")
                Spacer()
                Picker("UDP Out", selection: cfg.udpOutMode) {
                    ForEach(FPPInstanceConfig.UDPOutMode.allCases) { mode in
                        Text(mode.label).tag(mode)
                    }
                }
                .labelsHidden()
                .pickerStyle(.menu)
            }

            HStack {
                Text("Playlist")
                Spacer()
                if !inst.canHostPlaylist {
                    Text("Not available in \(inst.mode) mode")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                } else {
                    // Intercept the sentinel selection so the picker
                    // never actually settles on it — it just triggers
                    // the alert and the picker reverts to the previous
                    // value until the user confirms a real name.
                    let pickerBinding = Binding<String>(
                        get: { cfg.wrappedValue.playlist },
                        set: { newValue in
                            if newValue == kCreateNewPlaylistSentinel {
                                newPlaylistName = ""
                                newPlaylistTargetUUID = inst.uuid
                            } else {
                                cfg.wrappedValue.playlist = newValue
                            }
                        }
                    )
                    Picker("Playlist", selection: pickerBinding) {
                        Text("None").tag("")
                        // Render the current selection even when it
                        // isn't in the FPP's reported playlist list
                        // (typical for a name created via this alert
                        // before the FPP has been re-discovered).
                        let current = cfg.wrappedValue.playlist
                        if !current.isEmpty
                            && !inst.playlists.contains(current) {
                            Text("\(current) (pending)").tag(current)
                        }
                        ForEach(inst.playlists, id: \.self) { name in
                            Text(name).tag(name)
                        }
                        Divider()
                        Text("Create New…").tag(kCreateNewPlaylistSentinel)
                    }
                    .labelsHidden()
                    .pickerStyle(.menu)
                }
            }

            Text("Models uploads require a full FPPD restart. UDP Out and Pixel Hat/Cape uploads restart only if needed. Playlist additions take effect after the upload completes.")
                .font(.caption2)
                .foregroundStyle(.tertiary)
                .padding(.top, 4)
        }
        .padding(.bottom, 4)
        // "Create New Playlist" alert. Hosted per-row so the bound
        // UUID is unambiguous — only the row whose UUID matches the
        // target shows the alert.
        .alert("New Playlist on \(inst.displayName)",
                isPresented: Binding(
                    get: { newPlaylistTargetUUID == inst.uuid },
                    set: { showing in
                        if !showing { newPlaylistTargetUUID = nil }
                    }
                )) {
            TextField("Playlist name", text: $newPlaylistName)
                .textInputAutocapitalization(.words)
                .autocorrectionDisabled()
            Button("Cancel", role: .cancel) {
                newPlaylistName = ""
            }
            Button("Create") {
                let trimmed = newPlaylistName.trimmingCharacters(in: .whitespacesAndNewlines)
                guard !trimmed.isEmpty else { return }
                var c = instanceConfigs[inst.uuid] ?? FPPInstanceConfig()
                c.playlist = trimmed
                instanceConfigs[inst.uuid] = c
                FPPInstanceConfigStore.set(c, for: inst.uuid)
                newPlaylistName = ""
            }
        } message: {
            Text("Sequences uploaded to this FPP will be added to a playlist with this name. The playlist is created on the FPP at upload time.")
        }
        }
    }

    // MARK: Phase 3a — Configuring

    /// Per-FPP config screen — shown while applyConfig is running for
    /// targets with Cape or Add Proxies enabled. Indeterminate spinner
    /// because applyConfig doesn't report per-step progress yet (the
    /// FPP class's progress callback fires during the curl uploads
    /// inside each UploadXOutputs call, but spans multiple of them).
    private func configuringView(runner: FPPConnectRunner,
                                  targetIndex: Int,
                                  targetCount: Int,
                                  instanceName: String) -> some View {
        VStack(spacing: 20) {
            Spacer()
            ProgressView().controlSize(.large)
            VStack(spacing: 6) {
                Text("Configuring FPP \(targetIndex) of \(targetCount)")
                    .font(.title3.weight(.medium))
                Text(instanceName)
                    .font(.body)
                    .foregroundStyle(.secondary)
                    .lineLimit(2)
                    .multilineTextAlignment(.center)
            }
            Text("Pushing pixel outputs / proxies and restarting FPPD if needed. Sequence uploads start once every FPP is configured.")
                .font(.caption2)
                .foregroundStyle(.tertiary)
                .multilineTextAlignment(.center)
                .padding(.horizontal, 24)
            Spacer()
        }
        .padding()
    }

    // MARK: Phase 4 — Finalizing

    /// Per-FPP finalize screen — committing the playlist (if any) and
    /// firing the final restart so any restart-flagged config from
    /// Phase 1 actually takes effect.
    private func finalizingView(targetIndex: Int,
                                 targetCount: Int,
                                 instanceName: String) -> some View {
        VStack(spacing: 20) {
            Spacer()
            ProgressView().controlSize(.large)
            VStack(spacing: 6) {
                Text("Finalizing FPP \(targetIndex) of \(targetCount)")
                    .font(.title3.weight(.medium))
                Text(instanceName)
                    .font(.body)
                    .foregroundStyle(.secondary)
                    .lineLimit(2)
                    .multilineTextAlignment(.center)
            }
            Text("Committing playlist additions and restarting FPPD to apply any pending configuration changes.")
                .font(.caption2)
                .foregroundStyle(.tertiary)
                .multilineTextAlignment(.center)
                .padding(.horizontal, 24)
            Spacer()
        }
        .padding()
    }

    // MARK: Phase 3 — Uploading (parallel fan-out)

    /// Slice F UI — one sequence is in flight to every selected FPP
    /// in parallel. Show per-target gauges so the user can see if one
    /// FPP is dragging the batch. Per-FPP progress is read from
    /// `runner.currentSequenceProgress` (0..100 per IP).
    private func uploadingView(runner: FPPConnectRunner,
                                sequenceIndex: Int,
                                sequenceCount: Int,
                                sequenceName: String,
                                targets: [FPPInstance]) -> some View {
        let avg: Double = {
            guard !targets.isEmpty else { return 0 }
            let sum = targets.map { runner.currentSequenceProgress[$0.ipAddress] ?? 0 }
                .reduce(0, +)
            return Double(sum) / Double(targets.count) / 100.0
        }()
        let overall = (Double(sequenceIndex - 1) + avg) / Double(max(sequenceCount, 1))

        return VStack(spacing: 16) {
            VStack(spacing: 4) {
                Text("Uploading \(sequenceIndex) of \(sequenceCount)")
                    .font(.title3.weight(.medium))
                Text(sequenceName)
                    .font(.body)
                    .foregroundStyle(.secondary)
                    .lineLimit(2)
                    .multilineTextAlignment(.center)
                Text("\(targets.count) FPP\(targets.count == 1 ? "" : "s") in parallel")
                    .font(.caption2)
                    .foregroundStyle(.tertiary)
            }
            .padding(.top, 8)

            ScrollView {
                VStack(spacing: 10) {
                    ForEach(targets) { tgt in
                        let pct = runner.currentSequenceProgress[tgt.ipAddress] ?? 0
                        VStack(alignment: .leading, spacing: 4) {
                            HStack {
                                Text(tgt.displayName)
                                    .font(.callout.weight(.medium))
                                Spacer()
                                Text("\(pct)%")
                                    .font(.caption.monospacedDigit())
                                    .foregroundStyle(.secondary)
                            }
                            ProgressView(value: Double(pct) / 100.0)
                                .progressViewStyle(.linear)
                                .tint(.accentColor)
                        }
                        .padding(.horizontal, 16)
                    }
                }
                .padding(.vertical, 8)
            }
            .frame(maxHeight: 280)

            VStack(spacing: 4) {
                ProgressView(value: overall)
                    .progressViewStyle(.linear)
                    .tint(.secondary)
                Text("\(Int(overall * 100))% overall")
                    .font(.caption2)
                    .foregroundStyle(.secondary)
            }
            .frame(maxWidth: 360)
            .padding(.horizontal, 16)

            Text("Keep the iPad on the same network as the FPPs. Backgrounding may pause uploads.")
                .font(.caption2)
                .foregroundStyle(.tertiary)
                .multilineTextAlignment(.center)
                .padding(.horizontal, 24)
                .padding(.bottom, 8)
        }
    }

    // MARK: Phase 4 — Done

    private func doneView(runner: FPPConnectRunner,
                           succeeded: Int,
                           failed: Int,
                           cancelled: Bool,
                           failures: [String]) -> some View {
        VStack(spacing: 16) {
            // CTL-27 — desktop status-bar parity: surface the same final
            // outcome string FPPConnectDialog shows ("FPP Connect Upload
            // Complete" / "…had errors or warnings" / "…Cancelled").
            Text(cancelled
                 ? "FPP Connect Upload Cancelled"
                 : (failed > 0 ? "FPP Connect Upload had errors or warnings"
                               : "FPP Connect Upload Complete"))
                .font(.caption.weight(.medium))
                .foregroundStyle(cancelled || failed > 0 ? .orange : .green)
            Image(systemName: cancelled
                  ? "exclamationmark.triangle"
                  : (failed > 0 ? "exclamationmark.circle" : "checkmark.circle.fill"))
                .font(.system(size: 56))
                .foregroundStyle(cancelled
                                 ? .orange
                                 : (failed > 0 ? .orange : .green))
            Text(cancelled ? "Cancelled" : (failed > 0 ? "Some uploads failed" : "Done"))
                .font(.title2.weight(.semibold))
            Text("\(succeeded) uploaded" + (failed > 0 ? "  ·  \(failed) failed" : ""))
                .foregroundStyle(.secondary)
            if !failures.isEmpty {
                List {
                    Section("Failures") {
                        ForEach(failures, id: \.self) { msg in
                            Text(msg).font(.caption)
                        }
                    }
                }
                .frame(maxHeight: 200)
            }
            Spacer()
            HStack {
                Button {
                    runner.resetToSelection()
                } label: {
                    Text("Back").frame(maxWidth: .infinity, minHeight: 44)
                }
                .buttonStyle(.bordered)

                Button {
                    runner.releaseInstances()
                    dismiss()
                } label: {
                    Text("Close").frame(maxWidth: .infinity, minHeight: 44)
                }
                .buttonStyle(.borderedProminent)
            }
            .padding(.horizontal)
            .padding(.bottom, 8)
        }
        .padding()
    }
}
