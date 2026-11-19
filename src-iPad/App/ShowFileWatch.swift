import SwiftUI

/// Someone else sharing the show folder saved the layout, controllers or
/// presets: their changes were brought into the open show, or some of them
/// need a decision first. Desktop shows the same text in its status bar
/// (`xLightsFrame::CheckShowFilesOnDisk`).
struct ShowFileUpdate: Identifiable, Equatable {
    let id = UUID()
    let message: String
    /// Some of the changes need a decision: Review opens the conflict sheet.
    let needsReview: Bool
}

extension SequencerViewModel {
    /// Key of the Settings toggle; on by default.
    static let autoReloadShowFilesKey = "autoReloadShowFiles"

    func startShowFileWatch() {
        if showFileWatchTimer == nil {
            showFileWatchTimer = Timer.scheduledTimer(withTimeInterval: 1.5, repeats: true) { [weak self] _ in
                MainActor.assumeIsolated { self?.tickShowFileWatch() }
            }
        }
    }

    func tickShowFileWatch() {
        if let hideAt = showFileUpdateHideAt, Date() >= hideAt {
            showFileUpdateHideAt = nil
            if showFileUpdate?.needsReview == false { showFileUpdate = nil }
        }
        // Mid-load or mid-open the document is being rebuilt on another thread.
        guard isShowFolderLoaded, !loadInFlight, !openInFlight else { return }
        let enabled = UserDefaults.standard.object(forKey: Self.autoReloadShowFilesKey) as? Bool ?? true
        guard enabled else {
            document.stopWatchingShowFiles()
            if showFileUpdate != nil { showFileUpdate = nil }
            return
        }
        // A merge sheet (save or review) is open on the same files.
        if showFileMergeSheet || document.isShowFileMergeLive() { return }
        // Not while playing (a controller change restarts output) or while a
        // drag or scrub holds row indices the merge could shift. The watcher
        // keeps the change until it is reported.
        if isPlaying || isScrubbing || activeDrag != nil { return }
        reviewingIncomingShowChanges = false
        handleIncomingShowChanges(document.checkShowFilesOnDisk())
    }

    /// Review in the banner: true when the conflict sheet should open.
    func reviewIncomingShowChanges() -> Bool {
        handleIncomingShowChanges(document.reviewIncomingShowChanges())
        let open = document.isShowFileMergeLive()
        reviewingIncomingShowChanges = open
        return open
    }

    /// After the conflict sheet applied the reviewed changes.
    func finishIncomingShowChangesReview() {
        guard reviewingIncomingShowChanges else { return }
        reviewingIncomingShowChanges = false
        showFileUpdate = nil
        afterIncomingShowChanges()
    }

    private func handleIncomingShowChanges(_ result: [String: Any]) {
        let state = (result["state"] as? NSNumber)?.intValue ?? 0
        let banner = result["banner"] as? String ?? ""
        switch state {
        case 1:
            showFileUpdate = ShowFileUpdate(message: banner, needsReview: false)
            showFileUpdateHideAt = Date().addingTimeInterval(30)
            afterIncomingShowChanges()
        case 2:
            showFileUpdate = ShowFileUpdate(message: banner, needsReview: true)
            showFileUpdateHideAt = nil
        case 3:
            showFileUpdate = ShowFileUpdate(message: banner, needsReview: false)
            showFileUpdateHideAt = Date().addingTimeInterval(30)
        case 4:
            if showFileUpdate?.needsReview == true { showFileUpdate = nil }
        default:
            break
        }
    }

    /// Their changes reached memory: refresh what shows the models,
    /// controllers and presets.
    func afterIncomingShowChanges() {
        showFilesReloadToken &+= 1
        reloadPresets()
        if isSequenceLoaded { reloadRows() }
    }
}

/// The banner for a `ShowFileUpdate`, in the sequencer and the layout editor.
struct ShowFileUpdateBanner: View {
    let update: ShowFileUpdate
    let onReview: () -> Void
    let onDismiss: () -> Void

    var body: some View {
        HStack(spacing: 8) {
            Image(systemName: update.needsReview ? "arrow.triangle.merge" : "arrow.down.circle")
            Text(update.needsReview ? update.message + ". Until you decide, saving asks the same questions." : update.message)
                .font(.caption)
                .lineLimit(2)
            Spacer()
            if update.needsReview {
                Button("Review", action: onReview)
                    .buttonStyle(.bordered)
                    .controlSize(.small)
                    .tint(.white)
            }
            Button(action: onDismiss) {
                Image(systemName: "xmark")
            }
            .buttonStyle(.plain)
            .accessibilityLabel("Dismiss")
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 6)
        .foregroundStyle(.white)
        .background(update.needsReview ? Color.orange : Color.blue)
    }
}
