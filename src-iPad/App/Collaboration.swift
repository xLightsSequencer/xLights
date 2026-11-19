import SwiftUI
import UIKit

// Live collaboration: the state SwiftUI reads (through
// `SequencerViewModel.collab`) and the view-model side of the
// XLCollabSession bridge. Everything runs on the main actor; the bridge
// delivers its events on the main queue.

/// What the grid draws for the other participants, in display-row terms.
struct CollabOverlay: Equatable {
    struct Key: Hashable {
        let row: Int
        let effect: Int
    }
    struct Mark: Equatable {
        var outlines: [UInt32] = []
        var initials: String = ""
        var lockColor: UInt32 = 0
        var flash: Bool = false
    }
    struct Band: Equatable {
        let startMS: Int
        let endMS: Int
        let color: UInt32
        let name: String
    }
    struct Ghost: Equatable {
        let kind: XLCollabGhostKind
        let row: Int
        let startMS: Int
        let endMS: Int
        let name: String
    }
    /// A row only one participant may edit.
    struct Assigned: Equatable {
        let color: UInt32
        let name: String
        let mine: Bool
        /// "Assigned to Sam" / "Assigned to you".
        let text: String
        let canRelease: Bool
    }
    var bands: [Band] = []
    var stripes: [Int: [UInt32]] = [:]
    var marks: [Key: Mark] = [:]
    var ghosts: [Ghost] = []
    /// Display row -> who holds it.
    var assigned: [Int: Assigned] = [:]

    var isEmpty: Bool { bands.isEmpty && stripes.isEmpty && marks.isEmpty && ghosts.isEmpty && assigned.isEmpty }

    static func rgb(_ v: UInt32) -> (CGFloat, CGFloat, CGFloat) {
        (CGFloat((v >> 16) & 0xFF) / 255.0,
         CGFloat((v >> 8) & 0xFF) / 255.0,
         CGFloat(v & 0xFF) / 255.0)
    }
    static func color(_ v: UInt32) -> Color {
        let c = rgb(v)
        return Color(red: c.0, green: c.1, blue: c.2)
    }
}

struct CollabParticipantInfo: Identifiable, Equatable {
    let id: Int
    let name: String
    let deviceKind: String
    let color: UInt32
    let isHost: Bool
    let isLocal: Bool
    var isViewer: Bool = false
    var assignedRows: [String] = []

    var deviceLabel: String {
        switch deviceKind {
        case "ipad": return "iPad"
        case "mac": return "Mac"
        default: return deviceKind
        }
    }
}

struct CollabAdvertInfo: Identifiable, Equatable {
    let id: String
    let sequenceName: String
    let hostName: String
    let deviceKind: String
    /// Offered over SharePlay: joined without a pairing code.
    let isSharePlay: Bool

    init(id: String, sequenceName: String, hostName: String, deviceKind: String, isSharePlay: Bool = false) {
        self.id = id
        self.sequenceName = sequenceName
        self.hostName = hostName
        self.deviceKind = deviceKind
        self.isSharePlay = isSharePlay
    }

    init(_ a: XLCollabAdvert, isSharePlay: Bool = false) {
        self.init(id: a.advertId, sequenceName: a.sequenceName, hostName: a.hostName,
                  deviceKind: a.deviceKind, isSharePlay: isSharePlay)
    }
}

struct CollabRejectToast: Identifiable, Equatable {
    let id = UUID()
    let rejectId: UInt64
    let text: String
    let offersNewLayer: Bool
}

struct CollabJoinReportInfo: Equatable {
    let showFolder: String
    let sessionCopy: Bool
    let fetched: [String]
    let missingMedia: [String]
    let differentMedia: [String]
    let layoutDifference: String
    let refusal: String
}

@Observable @MainActor
final class CollabState {
    /// The "Live collaboration (beta)" preference.
    static let enabledKey = "CollaborationEnabled"
    static let displayNameKey = "CollaborationDisplayName"

    @ObservationIgnored var session: XLCollabSession?

    var enabled: Bool = UserDefaults.standard.bool(forKey: CollabState.enabledKey)
    var available: Bool = false
    var role: XLCollabRole = .none
    var state: XLCollabState = .idle
    var statusText: String = ""
    var participants: [CollabParticipantInfo] = []
    var sessions: [CollabAdvertInfo] = []
    var pairingCode: String = ""
    /// SharePlay can be used on this iPad.
    var groupAvailable = false
    /// A FaceTime call is active: hosting starts in it rather than inviting.
    var groupInCall = false
    /// The SharePlay session someone in the call started.
    var offeredGroup: CollabAdvertInfo?
    /// The offer banner was dismissed for this session id.
    var dismissedOfferId: String?
    /// Join sheet: select this session when it opens.
    var joinPreselectId: String?
    /// The current (or last) session runs over SharePlay.
    var sessionIsSharePlay = false
    var joinReport: CollabJoinReportInfo?
    var browseError: String?
    /// Why the session ended, shown non-modally until dismissed.
    var endNotice: String?
    /// The last session this device took part in was a client one.
    var endedAsClient = false
    var rejectToast: CollabRejectToast?
    var overlay = CollabOverlay()
    /// Not a viewer: this user may change the sequence.
    var canEdit = true
    /// Host: people who join from now on are viewers.
    var joinAsViewers = false
    /// The session edits the layout live.
    var liveLayout = false
    /// Layout objects others have selected or open in a modal editor, by name.
    var layoutMarks: [String: CollabLayoutMark] = [:]
    /// A layout change of this user that wasn't taken, or an editor that
    /// couldn't open; shown non-modally.
    var layoutNotice: CollabLayoutNotice?
    /// Whoever else has the effect open in this user's inspector.
    var softLockHolder: String?
    var showingSheet = false
    var sheetMode: SheetMode = .host

    enum SheetMode { case host, join }

    /// The open sequence is a session copy received from a host (lives in
    /// a temporary folder); plain Save must not write it.
    var clientDocument = false
    /// Joining switched to a session copy of the host's show; the user's
    /// own show folder comes back when the sequence closes.
    var restoreShowFolderOnClose = false

    /// Collaboration UI is shown at all.
    var isVisible: Bool { enabled && (available || groupAvailable) }
    /// In a session that hasn't ended.
    var inSession: Bool { role != .none && state != .ended && state != .idle }
    /// A client in a live session can't save over the sequence.
    var blocksPlainSave: Bool { clientDocument }

    var otherCount: Int { participants.filter { !$0.isLocal }.count }

    /// Sessions on the local network, without the SharePlay one (which the
    /// controller may list as well).
    var lanSessions: [CollabAdvertInfo] {
        guard let offered = offeredGroup else { return sessions }
        return sessions.filter { $0.id != offered.id }
    }

    /// The SharePlay offer to announce outside the join sheet.
    var groupOfferToAnnounce: CollabAdvertInfo? {
        guard isVisible, let offer = offeredGroup, offer.id != dismissedOfferId,
              role == .none || state == .ended || state == .idle,
              !(showingSheet && sheetMode == .join) else { return nil }
        return offer
    }

    static var displayName: String {
        let stored = UserDefaults.standard.string(forKey: displayNameKey) ?? ""
        return stored.isEmpty ? UIDevice.current.name : stored
    }
}

extension SequencerViewModel {

    // MARK: - Setup

    /// Creates the bridge the first time the preference is on.
    @discardableResult
    func ensureCollabSession() -> XLCollabSession? {
        if let s = collab.session { return s }
        guard collab.enabled else { return nil }
        let s = XLCollabSession(document: document)
        s.eventHandler = { [weak self] event, payload in
            MainActor.assumeIsolated {
                self?.handleCollabEvent(event, payload: payload)
            }
        }
        collab.session = s
        collab.available = s.available
        refreshCollabState()
        return s
    }

    /// The preference was switched.
    func collabPreferenceChanged(_ on: Bool) {
        collab.enabled = on
        if on {
            ensureCollabSession()
        } else if let s = collab.session {
            switch collab.role {
            case .host: s.end()
            case .client: s.leave()
            default: break
            }
            s.stopBrowsing()
            collab.showingSheet = false
            refreshCollabState()
        }
    }

    func refreshCollabState() {
        guard let s = collab.session else { return }
        collab.available = s.available
        collab.role = s.role
        collab.state = s.state
        collab.statusText = s.statusText
        collab.pairingCode = s.pairingCode
        refreshCollabGroup()
        let live = s.liveLayout && collab.inSession
        if collab.liveLayout != live {
            collab.liveLayout = live
            if !live {
                collab.layoutMarks = [:]
                document.setCollabReadOnlyObjects([])
            }
        }
        refreshLayoutLock()
        collab.participants = s.participants.map {
            CollabParticipantInfo(id: $0.participantId, name: $0.name, deviceKind: $0.deviceKind,
                                  color: $0.colorRGB, isHost: $0.isHost, isLocal: $0.isLocal,
                                  isViewer: $0.isViewer, assignedRows: $0.assignedRows)
        }
        let canEdit = s.canEditSequence
        if collab.canEdit != canEdit { collab.canEdit = canEdit }
        let joinViewers = collab.role == .host && s.joinAsViewers
        if collab.joinAsViewers != joinViewers { collab.joinAsViewers = joinViewers }
    }

    func refreshCollabGroup() {
        guard let s = collab.session else { return }
        collab.groupAvailable = s.groupAvailable
        collab.groupInCall = s.groupInCall
        collab.sessionIsSharePlay = s.sessionNetwork == .sharePlay
        let offer = s.offeredGroupSession.map { CollabAdvertInfo($0, isSharePlay: true) }
        if collab.offeredGroup != offer { collab.offeredGroup = offer }
    }

    // MARK: - Events

    private func handleCollabEvent(_ event: XLCollabEvent, payload: Any?) {
        guard let s = collab.session else { return }
        switch event {
        case .stateChanged:
            let before = collab.state
            refreshCollabState()
            if collab.state == .active && before != .active {
                sendCollabLayoutSelection()
                refreshCollabLayoutMarks()
                // The controller cleared its history; keep the undo
                // stack the buttons read in step.
                undoManager.removeAllActions()
                collab.endNotice = nil
                refreshCollabPresence()
                refreshCollabOverlay()
            }
            if collab.state == .ended && before != .ended {
                undoManager.removeAllActions()
                collab.endedAsClient = collab.role == .client
                let text = s.endText
                collab.endNotice = text.isEmpty ? "The collaboration session ended." : text
                collab.overlay = CollabOverlay()
                collab.softLockHolder = nil
                collab.layoutMarks = [:]
            }
        case .participantsChanged:
            refreshCollabState()
            refreshCollabOverlay()
            refreshCollabLayoutMarks()
        case .presenceChanged:
            refreshCollabOverlay()
            refreshCollabSoftLock()
            refreshCollabLayoutMarks()
        case .sequenceChanged:
            if let change = payload as? XLCollabChange {
                applyCollabChange(change)
            }
        case .sequenceModified:
            isDirty = true
        case .rejected:
            if let r = payload as? XLCollabReject {
                showCollabToast(CollabRejectToast(rejectId: r.rejectId, text: r.text, offersNewLayer: r.offersNewLayer))
            }
            refreshCollabOverlay()
        case .undoStateChanged:
            break
        case .joinReport:
            if let r = payload as? XLCollabJoinReport {
                collab.joinReport = CollabJoinReportInfo(
                    showFolder: r.showFolder, sessionCopy: r.sessionCopy, fetched: r.fetched,
                    missingMedia: r.missingMedia, differentMedia: r.differentMedia,
                    layoutDifference: r.layoutDifference, refusal: r.refusal)
            }
        case .sessionsFound:
            collab.sessions = s.sessions.map { CollabAdvertInfo($0) }
            collab.browseError = nil
            refreshCollabGroup()
        case .groupChanged:
            refreshCollabGroup()
        case .browseError:
            let reason = (payload as? String) ?? ""
            collab.browseError = Self.collabBrowseErrorText(reason)
        case .showFolderChanged:
            if let folder = payload as? String {
                showFolderPath = folder
                collab.restoreShowFolderOnClose = true
            }
        case .willReplaceSequence:
            quiesceForCollabSequenceReplace()
        case .layoutApplied:
            if let c = payload as? XLCollabLayoutChange {
                applyCollabLayoutChange(c)
            }
        case .layoutPresenceChanged:
            refreshCollabLayoutMarks()
        case .layoutRejected:
            showCollabLayoutNotice((payload as? String) ?? "")
        case .notice:
            refreshCollabState()
            showCollabToast((payload as? String) ?? "")
        @unknown default:
            break
        }
    }

    private func showCollabToast(_ text: String) {
        guard !text.isEmpty else { return }
        showCollabToast(CollabRejectToast(rejectId: 0, text: text, offersNewLayer: false))
    }

    private func showCollabToast(_ toast: CollabRejectToast) {
        collab.rejectToast = toast
        DispatchQueue.main.asyncAfter(deadline: .now() + 8) { [weak self] in
            MainActor.assumeIsolated {
                if self?.collab.rejectToast?.id == toast.id { self?.collab.rejectToast = nil }
            }
        }
    }

    // MARK: - Roles and row assignments

    /// Host: make a participant a viewer, or an editor again.
    func setCollabViewer(_ viewer: Bool, participant: Int) {
        guard let s = collab.session else { return }
        if let err = s.setViewer(viewer, participant: participant) {
            showCollabToast(err)
        }
        refreshCollabState()
    }

    /// Host: whether people who join from now on are viewers.
    func setCollabJoinAsViewers(_ on: Bool) {
        collab.session?.joinAsViewers = on
        refreshCollabState()
    }

    /// The row header menu's "Assign to" / "Release" items for a display
    /// row, or nil when there is nothing to offer.
    func collabRowMenu(rowIndex: Int) -> CollabRowMenu? {
        guard collab.inSession, collab.state == .active, collab.canEdit else { return nil }
        if let a = collab.overlay.assigned[rowIndex] {
            return CollabRowMenu(assignedText: a.text, canRelease: a.canRelease, assignees: [],
                                 onAssign: { _ in },
                                 onRelease: { [weak self] in self?.releaseCollabRow(rowIndex) })
        }
        var assignees: [CollabRowMenu.Assignee] = []
        if collab.role == .host {
            for p in collab.participants where !p.isViewer {
                assignees.append(.init(id: p.id, label: p.isLocal ? "\(p.name) (you)" : p.name))
            }
        } else if let me = collab.participants.first(where: { $0.isLocal }) {
            assignees.append(.init(id: me.id, label: XLCollabSession.labelAssignToMe))
        }
        return CollabRowMenu(assignedText: nil, canRelease: false, assignees: assignees,
                             onAssign: { [weak self] who in self?.assignCollabRow(rowIndex, to: who) },
                             onRelease: {})
    }

    func assignCollabRow(_ rowIndex: Int, to participant: Int) {
        guard let s = collab.session else { return }
        if let err = s.assignRow(rowIndex, to: participant) {
            showCollabToast(err)
        }
        refreshCollabOverlay()
    }

    func releaseCollabRow(_ rowIndex: Int) {
        guard let s = collab.session else { return }
        if let err = s.releaseRow(rowIndex) {
            showCollabToast(err)
        }
        refreshCollabOverlay()
    }

    /// Browsing fails when iPadOS refuses Local Network access; say where
    /// to turn it on.
    static func collabBrowseErrorText(_ reason: String) -> String {
        let lower = reason.lowercased()
        if lower.contains("local network") || lower.contains("denied") || lower.contains("-65570") {
            return "Local network access is turned off for xLights. Turn it on in Settings > Privacy & Security > Local Network, then try again."
        }
        if reason.isEmpty {
            return "Can't look for sessions on the local network."
        }
        return "Can't look for sessions on the local network: \(reason)"
    }

    // MARK: - Session actions

    /// Host the open sequence. Returns an error message, or nil.
    func hostCollabSession() -> String? {
        guard let s = ensureCollabSession() else { return "Live collaboration is turned off." }
        guard isSequenceLoaded else { return "Open a sequence first." }
        var err: NSString?
        guard s.host(name: CollabState.displayName, error: &err) != nil else {
            refreshCollabState()
            return (err as String?) ?? "Couldn't start the session."
        }
        collab.endNotice = nil
        collab.clientDocument = false
        refreshCollabState()
        refreshCollabPresence()
        return nil
    }

    /// Host the open sequence over SharePlay: in the FaceTime call when
    /// there is one, otherwise the system asks whom to invite. Returns an
    /// error message, or nil.
    func hostCollabSharePlay() -> String? {
        guard let s = ensureCollabSession() else { return "Live collaboration is turned off." }
        guard isSequenceLoaded else { return "Open a sequence first." }
        var err: NSString?
        guard s.hostGroup(name: CollabState.displayName, error: &err) else {
            refreshCollabState()
            return (err as String?) ?? "Couldn't start the SharePlay session."
        }
        collab.endNotice = nil
        collab.clientDocument = false
        refreshCollabState()
        refreshCollabPresence()
        return nil
    }

    /// The offer banner's Join: the join sheet with the SharePlay session
    /// selected, so the user still picks whether to fetch show files.
    func showJoinForGroupOffer() {
        guard let offer = collab.offeredGroup else { return }
        collab.joinPreselectId = offer.id
        collab.sheetMode = .join
        collab.showingSheet = true
    }

    func startCollabBrowsing() {
        guard let s = ensureCollabSession() else { return }
        collab.browseError = nil
        s.startBrowsing()
        collab.sessions = s.sessions.map { CollabAdvertInfo($0) }
        refreshCollabGroup()
    }

    func stopCollabBrowsing() {
        collab.session?.stopBrowsing()
    }

    /// Returns an error message, or nil when the join started.
    func joinCollabSession(_ advert: CollabAdvertInfo, code: String, fetchFiles: Bool) -> String? {
        guard let s = ensureCollabSession() else { return "Live collaboration is turned off." }
        if isSequenceLoaded {
            return "Close the open sequence before joining a session."
        }
        collab.joinReport = nil
        collab.endNotice = nil
        var err: NSString?
        let ok: Bool
        if advert.isSharePlay {
            ok = s.joinGroup(name: CollabState.displayName, advertId: advert.id,
                             fetchFiles: fetchFiles, error: &err)
        } else {
            ok = s.join(name: CollabState.displayName, advertId: advert.id,
                        code: code.trimmingCharacters(in: .whitespaces),
                        fetchFiles: fetchFiles, error: &err)
        }
        refreshCollabState()
        return ok ? nil : ((err as String?) ?? "Couldn't join the session.")
    }

    func leaveCollabSession() {
        collab.session?.leave()
        refreshCollabState()
    }

    /// Host: ends the session for everyone. The caller has already asked
    /// whether to save.
    func endCollabSession() {
        collab.session?.end()
        refreshCollabState()
    }

    func putRejectedOnNewLayer(_ toast: CollabRejectToast) {
        _ = collab.session?.putOnNewLayer(toast.rejectId)
        collab.rejectToast = nil
    }

    /// Client after the session: keep the copied show files, then Save As.
    func saveCollabCopy() {
        if collab.joinReport?.sessionCopy == true {
            _ = collab.session?.keepSessionCopy()
        }
        saveAsRequestToken &+= 1
    }

    // MARK: - Presence

    /// The grid's visible time range changed.
    func collabVisibleRangeChanged(startMS: Int, endMS: Int) {
        // Always passed on: a sequence open resets the scheduler's range.
        if isSequenceLoaded {
            document.setVisibleTimeRange(fromMS: Int32(startMS), toMS: Int32(endMS))
        }
        guard collab.inSession else {
            collabVisibleRange = (startMS, endMS)
            return
        }
        if collabVisibleRange.0 == startMS && collabVisibleRange.1 == endMS { return }
        collabVisibleRange = (startMS, endMS)
        scheduleCollabPresence()
    }

    /// Selection, inspector or visible range changed: send once per run
    /// loop turn.
    func scheduleCollabPresence() {
        guard collab.inSession, !collabPresencePending else { return }
        collabPresencePending = true
        DispatchQueue.main.async { [weak self] in
            MainActor.assumeIsolated {
                guard let self else { return }
                self.collabPresencePending = false
                self.refreshCollabPresence()
            }
        }
    }

    func refreshCollabPresence() {
        guard collab.inSession, isSequenceLoaded, let s = collab.session else { return }
        let sel: [EffectSelection] = selectedEffects.isEmpty
            ? (selectedEffect.map { [$0] } ?? []) : Array(selectedEffects)
        let refs = sel.map { XLCollabEffectRef(row: $0.rowIndex, effect: $0.effectIndex) }
        let rowsFocused = Array(Set(sel.map { $0.rowIndex })).sorted().map { NSNumber(value: $0) }
        var lock: XLCollabEffectRef?
        if showInspector, sel.count == 1, let one = selectedEffect {
            lock = XLCollabEffectRef(row: one.rowIndex, effect: one.effectIndex)
        }
        s.setLocalPresence(startMS: collabVisibleRange.0, endMS: collabVisibleRange.1,
                           focusedRows: rowsFocused, selectedEffects: refs, softLock: lock)
        refreshCollabSoftLock()
    }

    func refreshCollabSoftLock() {
        guard collab.inSession, let s = collab.session, let sel = selectedEffect else {
            if collab.softLockHolder != nil { collab.softLockHolder = nil }
            return
        }
        let holder = s.softLockHolder(row: sel.rowIndex, effect: sel.effectIndex)
        if collab.softLockHolder != holder { collab.softLockHolder = holder }
    }

    /// Rebuild what the grid draws for the others.
    func refreshCollabOverlay() {
        guard let s = collab.session, collab.role != .none, isSequenceLoaded else {
            if !collab.overlay.isEmpty { collab.overlay = CollabOverlay() }
            return
        }
        let o = s.overlay()
        var out = CollabOverlay()
        out.bands = o.bands.map { CollabOverlay.Band(startMS: $0.startMS, endMS: $0.endMS, color: $0.colorRGB, name: $0.name) }
        for (k, v) in o.rowStripes {
            out.stripes[k.intValue] = v.map { $0.uint32Value }
        }
        for m in o.effectMarks {
            out.marks[CollabOverlay.Key(row: m.rowIndex, effect: m.effectIndex)] =
                CollabOverlay.Mark(outlines: m.outlineColors.map { $0.uint32Value },
                                   initials: m.lockInitials, lockColor: m.lockColorRGB, flash: m.flash)
        }
        out.ghosts = o.ghosts.map {
            CollabOverlay.Ghost(kind: $0.kind, row: $0.rowIndex, startMS: $0.startMS, endMS: $0.endMS, name: $0.effectName)
        }
        for (k, a) in o.rowAssignments {
            out.assigned[k.intValue] = CollabOverlay.Assigned(color: a.colorRGB, name: a.ownerName, mine: a.mine,
                                                              text: a.text, canRelease: a.canRelease)
        }
        if out != collab.overlay { collab.overlay = out }
        if o.flashSecondsLeft > 0 {
            DispatchQueue.main.asyncAfter(deadline: .now() + o.flashSecondsLeft + 0.02) { [weak self] in
                MainActor.assumeIsolated { self?.refreshCollabOverlay() }
            }
        }
    }

    // MARK: - Document changes

    private func applyCollabChange(_ change: XLCollabChange) {
        if change.cause == .resync || change.everything {
            applyCollabResync()
            return
        }
        guard isSequenceLoaded else { return }
        reloadRows()
        restoreSelectionAfterCollabChange()
        if change.mediaChanged {
            hasAudio = document.hasAudio()
            reloadAltTracks()
            loadWaveform(startMS: 0, endMS: sequenceDurationMS)
        }
        markCollabChangeForRender(change)
        inspectorRevision &+= 1
        refreshCollabOverlay()
    }

    /// Edits elsewhere can move the selected effects to other indices, or
    /// take them out; follow them by id.
    private func restoreSelectionAfterCollabChange() {
        guard selectedEffect != nil || !selectedEffects.isEmpty, let s = collab.session else { return }
        let locs = s.currentLocationsOfLocalSelection()
        var newSet = Set<EffectSelection>()
        for l in locs {
            guard l.rowIndex >= 0, l.rowIndex < rows.count else { continue }
            let row = rows[l.rowIndex]
            guard l.effectIndex >= 0, l.effectIndex < row.effects.count else { continue }
            let e = row.effects[l.effectIndex]
            newSet.insert(EffectSelection(rowIndex: l.rowIndex, effectIndex: l.effectIndex,
                                          name: e.name, startTimeMS: e.startTimeMS, endTimeMS: e.endTimeMS))
        }
        if newSet.isEmpty {
            clearSelection()
        } else if newSet != selectedEffects || selectedEffect.map({ !newSet.contains($0) }) == true {
            if newSet.count == 1, let only = newSet.first {
                selectedEffect = only
                selectedEffects = newSet
            } else {
                setMultiSelection(newSet)
            }
        }
        if let sel = selectedEffect {
            var merged: [String: String] = [:]
            for (k, v) in document.effectSettings(forRow: Int32(sel.rowIndex), at: Int32(sel.effectIndex)) { merged[k] = v }
            for (k, v) in document.effectPalette(forRow: Int32(sel.rowIndex), at: Int32(sel.effectIndex)) { merged[k] = v }
            if merged != selectedEffectSettings { selectedEffectSettings = merged }
        }
    }

    /// The whole sequence was replaced (joined, or re-synced): refresh
    /// everything the way opening a sequence does.
    private func applyCollabResync() {
        guard document.isSequenceLoaded() else {
            isSequenceLoaded = false
            return
        }
        collab.clientDocument = collab.role == .client
        clearSelection()
        isSequenceLoaded = true
        isDirty = false
        isReadOnly = false
        sequenceName = document.sequenceName()
        sequenceDurationMS = Int(document.sequenceDurationMS())
        refreshTimelineExtent()
        frameIntervalMS = Int(document.frameIntervalMS())
        hasAudio = document.hasAudio()
        reloadAltTracks()
        reloadRows()
        reloadTagPositions()
        reloadSongStructure()
        reloadPresets()
        loadAvailableEffects()
        loadWaveform(startMS: 0, endMS: sequenceDurationMS)
        startBackgroundRender()
        startCollabDirtyPolling()
        refreshCollabOverlay()
    }
}
