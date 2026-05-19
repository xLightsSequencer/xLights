import SwiftUI

// EX-4 — FPP Connect (Slice A).
//
// Three-phase sheet:
//   1. Discovering — broadcast-ping scan + version probe (5-10s).
//   2. Selection   — pick FPP instances and .fseq files to push.
//   3. Uploading   — sequential per-target × per-sequence upload
//      with per-task progress, overall progress, cancel.
//   4. Done        — summary with retry / close.
//
// Slice A intentionally limits the per-instance options to "upload
// this sequence to this FPP" — none of the UDP-Out / Models / Pixel
// Outputs / Cape configuration the desktop dialog exposes. Those
// land in Slice B once the basic flow has been validated on real
// FPP hardware.

// MARK: - Models

/// Swift-side mirror of one entry from
/// `XLSequenceDocument.discoverFPPInstances`. Decoded from the bridge's
/// NSDictionary. UUID is used as the persistence key for the per-instance
/// "do upload" toggle so the selection survives discovery re-runs even when
/// IPs shift via DHCP.
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

    var id: String { uuid }

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
/// protocol up to the SwiftUI runner. Progress is reported from the
/// background upload thread; the runner polls `currentProgress` from
/// MainActor every ~150ms. Cancellation is also flag-based — the bridge
/// polls `isCancelled` between frames and on each curl callback.
///
/// Conforms to `@unchecked Sendable` because every mutable field is
/// guarded by `NSLock` — the access pattern is concurrent-by-design.
@objc final class FPPUploadProgressForwarder: NSObject, XLFPPUploadProgress, @unchecked Sendable {
    private let lock = NSLock()
    private var _progress: Int32 = 0
    private var _cancelled: Bool = false

    @objc func setProgressValue(_ value: Int32) {
        lock.lock()
        _progress = value
        lock.unlock()
    }

    @objc func isCancelled() -> Bool {
        lock.lock()
        defer { lock.unlock() }
        return _cancelled
    }

    /// Read the last value the bridge reported (0..100).
    var currentProgress: Int {
        lock.lock()
        defer { lock.unlock() }
        return Int(_progress)
    }

    func reset() {
        lock.lock()
        _progress = 0
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
        case uploading(taskIndex: Int, taskCount: Int,
                       instanceName: String, sequenceName: String)
        case done(succeeded: Int, failed: Int, cancelled: Bool,
                  failures: [String])
    }

    private(set) var phase: Phase = .idle
    private(set) var instances: [FPPInstance] = []
    /// Fraction (0..1) of the *current* task — the bridge reports per-
    /// frame transcode + curl byte progress on the same scale.
    private(set) var currentTaskProgress: Double = 0

    private let document: XLSequenceDocument
    private let forwarder = FPPUploadProgressForwarder()
    private var task: Task<Void, Never>?
    private var pollTask: Task<Void, Never>?

    init(document: XLSequenceDocument) {
        self.document = document
    }

    func discover() {
        guard task == nil else { return }
        phase = .discovering
        task = Task { @MainActor in
            // Parse the NSDictionary results into Sendable Swift values
            // inside the detached task so we can return them cleanly
            // across the actor boundary (Swift 6 won't allow
            // [AnyHashable: Any] to cross).
            let parsed: [FPPInstance] = await Task.detached(priority: .userInitiated) { [document] in
                let raw = document.discoverFPPInstances()
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

    func startUpload(targets: [FPPInstance], sequences: [SequenceEntry]) {
        guard task == nil, !targets.isEmpty, !sequences.isEmpty else { return }
        forwarder.reset()
        let taskList: [(FPPInstance, SequenceEntry)] = targets.flatMap { tgt in
            sequences.map { (tgt, $0) }
        }
        let total = taskList.count

        // Poll the forwarder for the per-task progress fraction on the
        // main runloop so SwiftUI re-renders the bar without us having
        // to ping from the background thread (which would require
        // hopping back to MainActor on every byte).
        pollTask = Task { @MainActor in
            while !Task.isCancelled {
                self.currentTaskProgress = Double(self.forwarder.currentProgress) / 100.0
                try? await Task.sleep(nanoseconds: 150_000_000)
            }
        }

        task = Task { @MainActor in
            var succeeded = 0
            var failed = 0
            var failures: [String] = []
            for (i, pair) in taskList.enumerated() {
                let (target, sequence) = pair
                if forwarder.isCancelled() { break }
                currentTaskProgress = 0
                forwarder.reset()
                phase = .uploading(taskIndex: i + 1,
                                    taskCount: total,
                                    instanceName: target.displayName,
                                    sequenceName: sequence.displayName)
                await Task.yield()

                let fseqPath = batchRenderFseqPath(forXsq: sequence.fullPath)
                let outcome: UploadOutcome = await Task.detached(priority: .userInitiated) { [document, forwarder] in
                    let raw = document.uploadFseq(fseqPath,
                                                   mediaPath: nil,
                                                   type: 0,
                                                   toIPAddress: target.ipAddress,
                                                   progress: forwarder)
                    let dict = raw as? [String: Any] ?? [:]
                    return UploadOutcome(
                        ok: (dict["ok"] as? NSNumber)?.boolValue ?? false,
                        cancelled: (dict["cancelled"] as? NSNumber)?.boolValue ?? false,
                        message: (dict["message"] as? String) ?? "")
                }.value

                if outcome.cancelled {
                    break
                }
                if outcome.ok {
                    succeeded += 1
                } else {
                    failed += 1
                    let msg = outcome.message.isEmpty ? "Upload failed" : outcome.message
                    failures.append("\(target.displayName) ← \(sequence.displayName): \(msg)")
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

    var body: some View {
        NavigationStack {
            Group {
                if let runner = runner {
                    switch runner.phase {
                    case .idle, .discovering:
                        discoveringView()
                    case .selection:
                        selectionView(runner: runner)
                    case .uploading(let i, let total, let inst, let seq):
                        uploadingView(runner: runner,
                                      taskIndex: i, taskCount: total,
                                      instanceName: inst, sequenceName: seq)
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
            if runner == nil {
                runner = FPPConnectRunner(document: viewModel.document)
                runner?.discover()
            }
            selectedInstanceUUIDs = FPPConnectSelections.load()
            sequences = SequenceScanner.scan(showFolder: viewModel.showFolderPath ?? "")
        }
    }

    private var isUploading: Bool {
        if case .uploading = runner?.phase { return true }
        return false
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
            Section("FPP Instances") {
                if runner.instances.isEmpty {
                    ContentUnavailableView("No FPP instances found",
                                            systemImage: "antenna.radiowaves.left.and.right",
                                            description: Text("Make sure your iPad and your FPP devices are on the same network."))
                } else {
                    ForEach(runner.instances) { inst in
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
                                Spacer()
                            }
                            .contentShape(Rectangle())
                        }
                        .buttonStyle(.plain)
                        .disabled(!inst.supportedForFPPConnect)
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
                runner.startUpload(targets: targets, sequences: toUpload)
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

    // MARK: Phase 3 — Uploading

    private func uploadingView(runner: FPPConnectRunner,
                                taskIndex: Int,
                                taskCount: Int,
                                instanceName: String,
                                sequenceName: String) -> some View {
        let overall = (Double(taskIndex - 1) + runner.currentTaskProgress)
            / Double(max(taskCount, 1))
        return VStack(spacing: 20) {
            Spacer()
            ProgressView().controlSize(.large)
            VStack(spacing: 6) {
                Text("Uploading \(taskIndex) of \(taskCount)")
                    .font(.title3.weight(.medium))
                Text("\(sequenceName)  →  \(instanceName)")
                    .font(.body)
                    .foregroundStyle(.secondary)
                    .multilineTextAlignment(.center)
                    .lineLimit(2)
            }
            VStack(spacing: 4) {
                ProgressView(value: runner.currentTaskProgress)
                    .progressViewStyle(.linear)
                    .tint(.accentColor)
                Text("\(Int(runner.currentTaskProgress * 100))% of this upload")
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }
            .frame(maxWidth: 360)
            VStack(spacing: 4) {
                ProgressView(value: overall)
                    .progressViewStyle(.linear)
                    .tint(.secondary)
                Text("\(Int(overall * 100))% overall")
                    .font(.caption2)
                    .foregroundStyle(.secondary)
            }
            .frame(maxWidth: 360)
            Text("Keep the iPad on the same network as the FPPs. Backgrounding may pause the upload.")
                .font(.caption2)
                .foregroundStyle(.tertiary)
                .multilineTextAlignment(.center)
                .padding(.horizontal, 24)
            Spacer()
        }
        .padding()
    }

    // MARK: Phase 4 — Done

    private func doneView(runner: FPPConnectRunner,
                           succeeded: Int,
                           failed: Int,
                           cancelled: Bool,
                           failures: [String]) -> some View {
        VStack(spacing: 16) {
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
