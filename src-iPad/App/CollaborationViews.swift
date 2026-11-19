import SwiftUI

// Live collaboration UI: the host / join sheet, the toolbar status chip and
// the non-modal banners. State lives in `SequencerViewModel.collab`.

/// Root-hosted sheet: hosting the open sequence, or joining a session
/// from the sequence browser.
struct CollaborationSheet: View {
    @Environment(SequencerViewModel.self) var viewModel
    @Environment(\.dismiss) private var dismiss

    var body: some View {
        NavigationStack {
            Group {
                if viewModel.collab.role == .host && viewModel.collab.state != .ended
                    && viewModel.collab.state != .idle {
                    CollabHostSessionView()
                } else if viewModel.collab.role == .client && viewModel.collab.state != .idle
                            && !(viewModel.collab.state == .ended && !viewModel.collab.clientDocument) {
                    CollabClientSessionView()
                } else if viewModel.collab.sheetMode == .host {
                    CollabHostStartView()
                } else {
                    CollabJoinView()
                }
            }
            .toolbar {
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") { dismiss() }
                }
            }
        }
        .environment(viewModel)
    }
}

/// The name others see, editable before starting or joining.
private struct CollabNameField: View {
    @AppStorage(CollabState.displayNameKey) private var displayName: String = ""

    var body: some View {
        TextField("Your name", text: $displayName, prompt: Text(UIDevice.current.name))
            .textInputAutocapitalization(.words)
            .autocorrectionDisabled()
    }
}

private struct CollabParticipantRow: View {
    let p: CollabParticipantInfo
    /// Host: switches this participant between viewer and editor.
    var onToggleRole: (() -> Void)?

    var body: some View {
        HStack(spacing: 10) {
            Circle()
                .fill(CollabOverlay.color(p.color))
                .frame(width: 14, height: 14)
            VStack(alignment: .leading, spacing: 2) {
                HStack(spacing: 6) {
                    Text(p.name)
                    if p.isLocal {
                        Text("(you)").foregroundStyle(.secondary)
                    }
                    if p.isViewer {
                        Text(XLCollabSession.labelViewer)
                            .font(.caption2.weight(.semibold))
                            .padding(.horizontal, 6)
                            .padding(.vertical, 1)
                            .background(Capsule().fill(Color.secondary.opacity(0.2)))
                    }
                }
                if !p.assignedRows.isEmpty {
                    Text("\(XLCollabSession.labelRows): \(p.assignedRows.joined(separator: ", "))")
                        .font(.caption)
                        .foregroundStyle(CollabOverlay.color(p.color))
                        .lineLimit(2)
                }
            }
            Spacer()
            Text(p.isHost ? "\(p.deviceLabel) · host" : p.deviceLabel)
                .font(.caption)
                .foregroundStyle(.secondary)
            if let toggle = onToggleRole {
                Button(p.isViewer ? XLCollabSession.labelMakeEditor : XLCollabSession.labelMakeViewer) { toggle() }
                    .buttonStyle(.bordered)
                    .controlSize(.small)
            }
        }
    }
}

/// What a row header's menu offers for live collaboration.
struct CollabRowMenu {
    struct Assignee: Identifiable {
        let id: Int
        let label: String
    }
    /// "Assigned to Sam" when someone holds the row.
    let assignedText: String?
    let canRelease: Bool
    /// Host: everyone who edits; anyone else: themselves ("Assign to Me").
    let assignees: [Assignee]
    let onAssign: (Int) -> Void
    let onRelease: () -> Void
}

struct CollabRowMenuItems: View {
    let menu: CollabRowMenu

    var body: some View {
        Divider()
        if let text = menu.assignedText {
            Text(text)
            if menu.canRelease {
                Button { menu.onRelease() } label: {
                    Label(XLCollabSession.labelRelease, systemImage: "lock.open")
                }
            }
        } else if menu.assignees.count == 1, menu.assignees[0].label == XLCollabSession.labelAssignToMe {
            Button { menu.onAssign(menu.assignees[0].id) } label: {
                Label(XLCollabSession.labelAssignToMe, systemImage: "person.crop.circle.badge.checkmark")
            }
        } else if !menu.assignees.isEmpty {
            Menu {
                ForEach(menu.assignees) { a in
                    Button(a.label) { menu.onAssign(a.id) }
                }
            } label: {
                Label(XLCollabSession.labelAssignTo, systemImage: "person.crop.circle.badge.checkmark")
            }
        }
    }
}

/// A row header's band and owner name for a row only one participant may edit.
struct CollabAssignedMark: View {
    let assigned: CollabOverlay.Assigned?
    var showName: Bool = true

    var body: some View {
        if let a = assigned {
            let color = CollabOverlay.color(a.color)
            ZStack(alignment: .bottomTrailing) {
                Rectangle()
                    .fill(color)
                    .frame(height: 3)
                    .frame(maxWidth: .infinity, alignment: .bottom)
                if showName {
                    Text(a.mine ? "you" : a.name)
                        .font(.system(size: 9, weight: .semibold))
                        .foregroundStyle(color)
                        .lineLimit(1)
                        .padding(.trailing, 26)
                        .padding(.bottom, 4)
                }
            }
            .allowsHitTesting(false)
            .accessibilityLabel(a.text)
        }
    }
}

// MARK: - Host

private struct CollabHostStartView: View {
    @Environment(SequencerViewModel.self) var viewModel
    @State private var error: String?

    var body: some View {
        Form {
            Section {
                CollabNameField()
            } header: {
                Text("Your Name")
            }
            Section {
                if viewModel.isDirty {
                    Text("Save the sequence before starting a session, so everyone starts from the saved file.")
                        .font(.callout)
                    Button("Save") { _ = viewModel.saveSequence() }
                        .disabled(viewModel.isReadOnly)
                }
                Button("Start Session") {
                    error = viewModel.hostCollabSession()
                }
                .disabled(!viewModel.isSequenceLoaded || viewModel.isDirty)
                if let error {
                    Text(error).foregroundStyle(.red).font(.callout)
                }
            } footer: {
                Text("Others on the same network can join from their sequence list with the code shown next. Editors can change the layout during the session too; you save it, like the sequence.")
            }
            if viewModel.collab.groupAvailable {
                Section {
                    Button {
                        error = viewModel.hostCollabSharePlay()
                    } label: {
                        Label(viewModel.collab.groupInCall ? "Start in FaceTime call" : "Invite with Messages…",
                              systemImage: "shareplay")
                    }
                    .disabled(!viewModel.isSequenceLoaded || viewModel.isDirty)
                } header: {
                    Text("Host via SharePlay")
                } footer: {
                    Text(viewModel.collab.groupInCall
                         ? "Everyone in the FaceTime call can join, wherever they are. No code is needed."
                         : "Invite people through Messages or FaceTime, wherever they are. No code is needed.")
                }
            }
            if let ended = viewModel.collab.endNotice {
                Section("Last Session") {
                    Text(ended).font(.callout)
                }
            }
        }
        .navigationTitle("Collaborate")
        .navigationBarTitleDisplayMode(.inline)
    }
}

private struct CollabHostSessionView: View {
    @Environment(SequencerViewModel.self) var viewModel
    @State private var confirmingEnd = false

    var body: some View {
        Form {
            if viewModel.collab.sessionIsSharePlay {
                Section {
                    Label("SharePlay", systemImage: "shareplay")
                        .font(.system(size: 34, weight: .semibold))
                        .frame(maxWidth: .infinity)
                } header: {
                    Text("Session")
                } footer: {
                    Text(viewModel.collab.statusText)
                }
            } else {
                Section {
                    Text(viewModel.collab.pairingCode)
                        .font(.system(size: 54, weight: .semibold, design: .monospaced))
                        .tracking(6)
                        .frame(maxWidth: .infinity)
                        .textSelection(.enabled)
                        .accessibilityLabel("Pairing code \(viewModel.collab.pairingCode.map { String($0) }.joined(separator: " "))")
                } header: {
                    Text("Pairing Code")
                } footer: {
                    Text(viewModel.collab.statusText)
                }
            }
            Section("Participants") {
                ForEach(viewModel.collab.participants) { p in
                    CollabParticipantRow(p: p, onToggleRole: p.isHost ? nil : {
                        viewModel.setCollabViewer(!p.isViewer, participant: p.id)
                    })
                }
                Toggle(XLCollabSession.labelJoinAsViewers, isOn: Binding(
                    get: { viewModel.collab.joinAsViewers },
                    set: { viewModel.setCollabJoinAsViewers($0) }))
            }
            Section {
                Button("End Session", role: .destructive) {
                    if viewModel.checkDirtyNow() {
                        confirmingEnd = true
                    } else {
                        viewModel.endCollabSession()
                    }
                }
            } footer: {
                Text("Ends the session for everyone. Save whenever you like during the session; only you save the sequence and the layout.")
            }
        }
        .navigationTitle("Collaborate")
        .navigationBarTitleDisplayMode(.inline)
        .confirmationDialog("Save changes before ending the session?",
                            isPresented: $confirmingEnd, titleVisibility: .visible) {
            Button("Save and End") {
                if viewModel.saveSequence() {
                    viewModel.endCollabSession()
                }
            }
            Button("End Without Saving", role: .destructive) {
                viewModel.endCollabSession()
            }
            Button("Cancel", role: .cancel) {}
        } message: {
            Text("The sequence has changes from this session that aren't saved yet.")
        }
    }
}

// MARK: - Join

private struct CollabJoinView: View {
    @Environment(SequencerViewModel.self) var viewModel
    @State private var selected: CollabAdvertInfo?
    @State private var code: String = ""
    @State private var fetchFiles = true
    @State private var error: String?

    var body: some View {
        Form {
            Section("Your Name") {
                CollabNameField()
            }
            if let offer = viewModel.collab.offeredGroup {
                Section {
                    sessionRow(offer)
                } header: {
                    Text("SharePlay")
                }
            }
            Section {
                if let err = viewModel.collab.browseError {
                    Label(err, systemImage: "wifi.exclamationmark")
                        .foregroundStyle(.orange)
                        .font(.callout)
                }
                if viewModel.collab.lanSessions.isEmpty {
                    HStack(spacing: 8) {
                        ProgressView()
                        Text("Looking for sessions on this network…")
                            .foregroundStyle(.secondary)
                    }
                }
                ForEach(viewModel.collab.lanSessions) { s in
                    sessionRow(s)
                }
            } header: {
                Text(viewModel.collab.offeredGroup == nil ? "Sessions" : "On This Network")
            }
            if let s = selected, s.isSharePlay {
                Section {
                    fetchFilesToggle
                    Button("Join \(s.sequenceName)") {
                        error = viewModel.joinCollabSession(s, code: "", fetchFiles: fetchFiles)
                    }
                    .disabled(viewModel.collab.inSession)
                    if let error {
                        Text(error).foregroundStyle(.red).font(.callout)
                    }
                } header: {
                    Text("SharePlay")
                } footer: {
                    Text("\(s.hostName) started this session in your FaceTime call or Messages conversation. No code is needed.")
                }
            } else if let s = selected {
                Section {
                    TextField("6-digit code", text: $code)
                        .keyboardType(.numberPad)
                        .font(.system(.title2, design: .monospaced))
                    fetchFilesToggle
                    Button("Join \(s.sequenceName)") {
                        error = viewModel.joinCollabSession(s, code: code, fetchFiles: fetchFiles)
                    }
                    .disabled(code.trimmingCharacters(in: .whitespaces).count < 6 || viewModel.collab.inSession)
                    if let error {
                        Text(error).foregroundStyle(.red).font(.callout)
                    }
                } header: {
                    Text("Pairing Code")
                } footer: {
                    Text("Ask \(s.hostName) for the code shown on their screen.")
                }
            }
            CollabJoinProgressSection()
        }
        .navigationTitle("Join Session")
        .navigationBarTitleDisplayMode(.inline)
        .onAppear {
            viewModel.startCollabBrowsing()
            if let want = viewModel.collab.joinPreselectId, let offer = viewModel.collab.offeredGroup,
               offer.id == want {
                selected = offer
            }
            viewModel.collab.joinPreselectId = nil
        }
        .onDisappear { viewModel.stopCollabBrowsing() }
        // The selected session went away (a SharePlay offer withdrawn, or
        // a host that stopped advertising).
        .onChange(of: viewModel.collab.offeredGroup) { _, offer in
            if let sel = selected, sel.isSharePlay, sel.id != offer?.id { selected = nil }
        }
    }

    private func sessionRow(_ s: CollabAdvertInfo) -> some View {
        Button {
            selected = s
            error = nil
        } label: {
            HStack {
                if s.isSharePlay {
                    Image(systemName: "shareplay").foregroundStyle(.tint)
                }
                VStack(alignment: .leading, spacing: 2) {
                    Text(s.sequenceName).foregroundStyle(.primary)
                    Text("Hosted by \(s.hostName)")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
                Spacer()
                if selected?.id == s.id {
                    Image(systemName: "checkmark").foregroundStyle(.tint)
                }
            }
        }
    }

    private var fetchFilesToggle: some View {
        Toggle(isOn: $fetchFiles) {
            VStack(alignment: .leading, spacing: 2) {
                Text("Fetch show files that differ")
                Text("Copies the host's audio and media into a separate session folder when yours differ. Off: a different audio file refuses the join. A different layout is replaced by the session's in a separate copy either way; your show folder is never changed.")
                    .font(.caption2)
                    .foregroundStyle(.secondary)
            }
        }
    }
}

/// Connecting -> joining -> syncing, then the join report.
private struct CollabJoinProgressSection: View {
    @Environment(SequencerViewModel.self) var viewModel

    var body: some View {
        let c = viewModel.collab
        if c.role == .client && c.state != .idle {
            Section("Progress") {
                HStack(spacing: 8) {
                    if c.state == .connecting || c.state == .joining || c.state == .syncing || c.state == .migrating {
                        ProgressView()
                    }
                    Text(c.statusText)
                }
                if c.state == .ended, let notice = c.endNotice {
                    Text(notice).foregroundStyle(.red).font(.callout)
                }
            }
        }
        if let r = c.joinReport {
            CollabJoinReportSection(report: r)
        }
    }
}

private struct CollabJoinReportSection: View {
    let report: CollabJoinReportInfo

    var body: some View {
        Section("Show Files") {
            if !report.refusal.isEmpty {
                Label(report.refusal, systemImage: "xmark.octagon")
                    .foregroundStyle(.red)
            }
            if !report.layoutDifference.isEmpty {
                Label(report.layoutDifference, systemImage: "square.3.layers.3d")
                    .font(.callout)
            }
            if report.sessionCopy {
                Text("Using a copy of the host's show for this session.")
                    .font(.callout)
                    .foregroundStyle(.secondary)
            }
            if !report.missingMedia.isEmpty {
                fileList("Missing media (those effects render blank)",
                         report.missingMedia, icon: "exclamationmark.triangle", color: .orange)
            }
            if !report.differentMedia.isEmpty {
                fileList("Different media (those effects render differently)",
                         report.differentMedia, icon: "exclamationmark.triangle", color: .orange)
            }
            if !report.fetched.isEmpty {
                fileList("Fetched from the host", report.fetched, icon: "arrow.down.circle", color: .secondary)
            }
            if report.refusal.isEmpty && report.missingMedia.isEmpty && report.differentMedia.isEmpty
                && report.layoutDifference.isEmpty && report.fetched.isEmpty {
                Text("Your show files match the host's.")
                    .foregroundStyle(.secondary)
            }
        }
    }

    private func fileList(_ title: String, _ files: [String], icon: String, color: Color) -> some View {
        VStack(alignment: .leading, spacing: 4) {
            Label(title, systemImage: icon).foregroundStyle(color)
            ForEach(files, id: \.self) { f in
                Text(f).font(.caption).foregroundStyle(.secondary).lineLimit(1).truncationMode(.middle)
            }
        }
    }
}

private struct CollabClientSessionView: View {
    @Environment(SequencerViewModel.self) var viewModel

    var body: some View {
        let c = viewModel.collab
        Form {
            Section {
                HStack(spacing: 8) {
                    if c.state == .connecting || c.state == .joining || c.state == .syncing || c.state == .migrating {
                        ProgressView()
                    }
                    Text(c.statusText)
                }
                if c.state == .active && !c.canEdit {
                    Label(XLCollabSession.labelViewOnly, systemImage: "eye")
                        .foregroundStyle(.secondary)
                }
            } header: {
                Text("Status")
            }
            if c.state != .ended {
                Section("Participants") {
                    ForEach(c.participants) { p in
                        CollabParticipantRow(p: p)
                    }
                }
            }
            if let r = c.joinReport {
                CollabJoinReportSection(report: r)
            }
            Section {
                if c.state == .ended {
                    if let notice = c.endNotice {
                        Text(notice).font(.callout)
                    }
                    if viewModel.isSequenceLoaded {
                        Button("Save a Copy…") { viewModel.saveCollabCopy() }
                    }
                } else {
                    Button("Leave Session", role: .destructive) {
                        viewModel.leaveCollabSession()
                    }
                }
            } footer: {
                Text("The host saves the sequence. You can keep working on your copy after leaving and save it under another name.")
            }
        }
        .navigationTitle("Collaborate")
        .navigationBarTitleDisplayMode(.inline)
    }
}

// MARK: - Toolbar chip and banners

/// Toolbar status: connection state and how many others are here.
struct CollabStatusChip: View {
    @Environment(SequencerViewModel.self) var viewModel

    var body: some View {
        let c = viewModel.collab
        Button {
            c.sheetMode = c.role == .client ? .join : .host
            c.showingSheet = true
        } label: {
            HStack(spacing: 5) {
                Circle()
                    .fill(c.state == .active ? Color.green : (c.state == .ended ? Color.red : Color.orange))
                    .frame(width: 8, height: 8)
                if c.state == .active && !c.canEdit {
                    Label(XLCollabSession.labelViewOnly, systemImage: "eye")
                        .font(.caption.weight(.semibold))
                        .labelStyle(.titleAndIcon)
                }
                Text(c.statusText)
                    .font(.caption)
                    .lineLimit(1)
                    .truncationMode(.tail)
                    .frame(maxWidth: 220, alignment: .leading)
                if c.otherCount > 0 {
                    Label("\(c.otherCount)", systemImage: "person.2.fill")
                        .font(.caption)
                        .labelStyle(.titleAndIcon)
                }
            }
            .padding(.horizontal, 8)
            .padding(.vertical, 3)
            .background(Capsule().fill(Color.secondary.opacity(0.18)))
        }
        .buttonStyle(.plain)
        .help("Collaboration session")
    }
}

/// Why the session ended; a client can save a copy from here.
struct CollabEndedBanner: View {
    let message: String
    let showSaveCopy: Bool
    let onSaveCopy: () -> Void
    let onDismiss: () -> Void

    var body: some View {
        HStack(spacing: 8) {
            Image(systemName: "person.2.slash")
            Text(message)
                .font(.caption)
                .lineLimit(2)
            Spacer()
            if showSaveCopy {
                Button("Save a Copy…", action: onSaveCopy)
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
        .background(Color.indigo)
    }
}

/// Someone in the FaceTime call started a SharePlay session.
struct CollabGroupOfferBanner: View {
    let offer: CollabAdvertInfo
    let onJoin: () -> Void
    let onDismiss: () -> Void

    var body: some View {
        HStack(spacing: 8) {
            Image(systemName: "shareplay")
            Text("\(offer.hostName) started a SharePlay session for '\(offer.sequenceName)'")
                .font(.caption)
                .lineLimit(2)
            Spacer()
            Button("Join", action: onJoin)
                .buttonStyle(.bordered)
                .controlSize(.small)
                .tint(.white)
            Button(action: onDismiss) {
                Image(systemName: "xmark")
            }
            .buttonStyle(.plain)
            .accessibilityLabel("Dismiss")
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 6)
        .foregroundStyle(.white)
        .background(Color.green.opacity(0.85))
    }
}

/// An edit the host didn't take.
struct CollabRejectBanner: View {
    let toast: CollabRejectToast
    let onNewLayer: () -> Void
    let onDismiss: () -> Void

    var body: some View {
        HStack(spacing: 8) {
            Image(systemName: "arrow.uturn.backward.circle")
            Text(toast.text)
                .font(.caption)
                .lineLimit(2)
            Spacer()
            if toast.offersNewLayer {
                Button("Put on New Layer", action: onNewLayer)
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
        .background(Color.orange)
    }
}

/// Inspector banner: someone else has this effect open.
struct CollabSoftLockBanner: View {
    let name: String

    var body: some View {
        HStack(spacing: 6) {
            Image(systemName: "pencil.and.outline")
            Text("\(name) is editing this effect")
                .font(.caption)
            Spacer()
        }
        .padding(.horizontal, 10)
        .padding(.vertical, 5)
        .foregroundStyle(.white)
        .background(Color.orange.opacity(0.9))
    }
}
