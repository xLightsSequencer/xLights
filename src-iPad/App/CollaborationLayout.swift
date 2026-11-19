import SwiftUI

// Live layout editing in a collaboration session: the layout editor's side
// of XLCollabSession (selection presence, modal-editor locks, applying the
// session's layout).

/// What the layout editor shows for a model, group or view object others
/// are working on.
struct CollabLayoutMark: Equatable {
    /// Colours of the others who have it selected.
    var outlines: [UInt32] = []
    /// Who has it open in a modal editor; empty when nobody does.
    var lockHolder: String = ""
    var lockColor: UInt32 = 0
    /// "Sam is editing Arch 1's submodels".
    var lockText: String = ""

    var isLocked: Bool { !lockHolder.isEmpty }
}

struct CollabLayoutNotice: Identifiable, Equatable {
    let id = UUID()
    let text: String
}

extension SequencerViewModel {

    /// Only the host of a collaboration session drives real hardware.
    var collabUploadRefusal: String? {
        collab.inSession && collab.role != .host ? "Only the session host can upload to controllers." : nil
    }
    var collabOutputRefusal: String? {
        collab.inSession && collab.role != .host ? "Only the session host can output to lights." : nil
    }

    /// The layout editor's selection changed: send it once per run loop turn.
    func scheduleCollabLayoutSelection() {
        guard collab.liveLayout, !collabLayoutSelectionPending else { return }
        collabLayoutSelectionPending = true
        DispatchQueue.main.async { [weak self] in
            MainActor.assumeIsolated {
                guard let self else { return }
                self.collabLayoutSelectionPending = false
                self.sendCollabLayoutSelection()
            }
        }
    }

    func sendCollabLayoutSelection() {
        guard collab.liveLayout, let s = collab.session else { return }
        var models = Array(layoutEditorSelection)
        if let one = layoutEditorSelectedModel, !models.contains(one) { models.append(one) }
        if let group = layoutEditorSelectedGroup, !group.isEmpty, !models.contains(group) { models.append(group) }
        var objects: [String] = []
        if let vo = layoutEditorSelectedObject, !vo.isEmpty, vo != "2D Background" { objects.append(vo) }
        s.setLayoutSelection(models: models.sorted(), objects: objects)
    }

    func refreshCollabLayoutMarks() {
        guard collab.liveLayout, let s = collab.session else {
            if !collab.layoutMarks.isEmpty { collab.layoutMarks = [:] }
            document.setCollabReadOnlyObjects([])
            return
        }
        var marks: [String: CollabLayoutMark] = [:]
        for m in s.layoutMarks() {
            marks[m.name] = CollabLayoutMark(outlines: m.outlineColors.map { $0.uint32Value },
                                             lockHolder: m.lockHolder,
                                             lockColor: m.lockColorRGB,
                                             lockText: m.lockText)
        }
        if marks != collab.layoutMarks { collab.layoutMarks = marks }
        document.setCollabReadOnlyObjects(marks.filter { $0.value.isLocked }.map { $0.key })
    }

    /// Why `name` can't be edited here right now (someone else has it open
    /// in a modal editor), or nil.
    func collabLayoutReadOnlyReason(_ name: String?) -> String? {
        guard collab.liveLayout, let name, !name.isEmpty else { return nil }
        if let m = collab.layoutMarks[name], m.isLocked {
            return m.lockText.isEmpty ? "\(m.lockHolder) is editing \(name)" : m.lockText
        }
        return nil
    }

    /// Before a modal editor of one object opens. False (with the reason
    /// shown) when someone else has it open; otherwise this user holds it
    /// until `endCollabLayoutLock`.
    func beginCollabLayoutLock(on name: String, what: String) -> Bool {
        guard collab.liveLayout, let s = collab.session else { return true }
        if let refusal = s.beginLayoutLock(on: name, what: what) {
            showCollabLayoutNotice(refusal)
            return false
        }
        return true
    }

    func endCollabLayoutLock() {
        collab.session?.endLayoutLock()
    }

    func showCollabLayoutNotice(_ text: String) {
        guard !text.isEmpty else { return }
        let notice = CollabLayoutNotice(text: text)
        collab.layoutNotice = notice
        DispatchQueue.main.asyncAfter(deadline: .now() + 8) { [weak self] in
            MainActor.assumeIsolated {
                if self?.collab.layoutNotice?.id == notice.id { self?.collab.layoutNotice = nil }
            }
        }
    }

    /// The session's layout reached memory: refresh what shows models and
    /// controllers and follow renames in the selection. The changed models
    /// are re-rendered by the sequence change that follows (its Models
    /// plan), not here.
    func applyCollabLayoutChange(_ change: XLCollabLayoutChange) {
        var renames: [String: String] = [:]
        for pair in change.modelsRenamed where pair.count == 2 {
            renames[pair[0]] = pair[1]
        }
        let removed = Set(change.modelsRemoved)
        if !renames.isEmpty || !removed.isEmpty {
            let fix: (String) -> String? = { name in
                if removed.contains(name) { return nil }
                return renames[name] ?? name
            }
            layoutEditorSelection = Set(layoutEditorSelection.compactMap(fix))
            if let one = layoutEditorSelectedModel { layoutEditorSelectedModel = fix(one) }
        }
        afterIncomingShowChanges()
        NotificationCenter.default.post(name: .layoutEditorModelMoved, object: "LayoutEditor", userInfo: [:])
        refreshCollabLayoutMarks()
    }
}

/// Lock badge for the model / object lists: someone else has it open in a
/// modal editor.
struct CollabLayoutLockBadge: View {
    let mark: CollabLayoutMark

    var body: some View {
        HStack(spacing: 3) {
            Image(systemName: "lock.fill")
            Text(mark.lockHolder)
                .lineLimit(1)
        }
        .font(.caption2)
        .foregroundStyle(CollabOverlay.color(mark.lockColor))
        .help(mark.lockText)
        .accessibilityLabel(mark.lockText.isEmpty ? "Locked by \(mark.lockHolder)" : mark.lockText)
    }
}

/// Selection dots for the model / object lists: the colours of the others
/// who have it selected.
struct CollabLayoutSelectionDots: View {
    let colors: [UInt32]

    var body: some View {
        HStack(spacing: 2) {
            ForEach(Array(colors.prefix(4).enumerated()), id: \.offset) { _, c in
                Circle()
                    .fill(CollabOverlay.color(c))
                    .frame(width: 7, height: 7)
            }
        }
        .accessibilityHidden(true)
    }
}

/// The layout notice, in the layout editor window and the main window.
struct CollabLayoutNoticeBanner: View {
    let notice: CollabLayoutNotice
    let onDismiss: () -> Void

    var body: some View {
        HStack(spacing: 8) {
            Image(systemName: "person.2.badge.gearshape")
            Text(notice.text)
                .font(.caption)
                .lineLimit(2)
            Spacer()
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

/// Read-only banner above an object's properties while someone else has it
/// open in a modal editor.
struct CollabLayoutReadOnlyBanner: View {
    let text: String

    var body: some View {
        HStack(spacing: 6) {
            Image(systemName: "lock.fill")
            Text(text)
                .font(.caption)
            Spacer()
        }
        .padding(.horizontal, 10)
        .padding(.vertical, 5)
        .foregroundStyle(.white)
        .background(Color.orange.opacity(0.9))
    }
}

/// A layout property field reports its focus: a live collaboration session
/// holds this user's layout changes back while one holds uncommitted typing.
struct CollabLayoutFieldFocus: ViewModifier {
    let focused: Bool
    @State private var reported = false

    func body(content: Content) -> some View {
        content
            .onChange(of: focused) { _, now in report(now) }
            .onDisappear { report(false) }
    }

    private func report(_ now: Bool) {
        guard now != reported else { return }
        reported = now
        XLCollabSession.noteLayoutFieldFocused(now)
    }
}
