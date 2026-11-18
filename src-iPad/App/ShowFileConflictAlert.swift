import SwiftUI

/// A save found that someone else (another person sharing the show
/// folder, or another app) rewrote the file since it was opened here.
/// Desktop asks the same question (`xLightsFrame::AskShowFileConflict`).
struct ShowFileSaveConflict: Identifiable {
    enum Kind { case sequence, layout, presets }

    let id = UUID()
    let kind: Kind
    let fileNames: [String]

    /// Layout and presets merge; sequences are not merged.
    var canMerge: Bool { kind != .sequence }

    var message: String {
        let names = fileNames.joined(separator: " and ")
        let verb = fileNames.count == 1 ? "was" : "were"
        var text = "\(names) \(verb) changed by someone else (or another app) since you opened it."
        if canMerge {
            text += " Merge keeps their changes and yours; if you both changed the same thing you choose which to keep. Overwrite replaces their changes with yours."
        } else {
            text += " Saving now would overwrite their changes."
        }
        return text
    }

    /// Text for the follow-up notice after "Save a Copy".
    static func copyNotice(copies: [String], originals: [String]) -> String {
        let copyNames = copies.map { ($0 as NSString).lastPathComponent }.joined(separator: ", ")
        let originalNames = originals.joined(separator: " and ")
        let verb = originals.count == 1 ? "was" : "were"
        return "Your version was saved as \(copyNames). \(originalNames) \(verb) left as the other person saved it. Your changes are still open here and are still unsaved."
    }
}

extension View {
    /// Merge / Overwrite / Save a Copy / Cancel for a `ShowFileSaveConflict`,
    /// plus the notice shown after a copy is written. Merge is offered when
    /// `onMerge` is given and the file kind merges.
    func showFileConflictAlert(_ conflict: Binding<ShowFileSaveConflict?>,
                               notice: Binding<String?>,
                               onOverwrite: @escaping (ShowFileSaveConflict) -> Void,
                               onSaveCopy: @escaping (ShowFileSaveConflict) -> Void,
                               onMerge: ((ShowFileSaveConflict) -> Void)? = nil) -> some View {
        self
            .alert("Changed by Someone Else",
                   isPresented: Binding(
                    get: { conflict.wrappedValue != nil },
                    set: { if !$0 { conflict.wrappedValue = nil } }),
                   presenting: conflict.wrappedValue) { c in
                if c.canMerge, let onMerge {
                    Button("Merge") { onMerge(c) }
                }
                Button("Overwrite", role: .destructive) { onOverwrite(c) }
                Button("Save a Copy") { onSaveCopy(c) }
                Button("Cancel", role: .cancel) { }
            } message: { c in
                Text(c.message)
            }
            .alert("Saved a Copy",
                   isPresented: Binding(
                    get: { notice.wrappedValue != nil },
                    set: { if !$0 { notice.wrappedValue = nil } })) {
                Button("OK", role: .cancel) { notice.wrappedValue = nil }
            } message: {
                Text(notice.wrappedValue ?? "")
            }
    }
}

/// One thing both this session and someone else changed differently.
struct ShowFileMergeItem: Identifiable {
    let id: Int
    let file: String
    let record: String
    let what: String
    let base: String
    let mine: String
    let theirs: String
    /// 0 undecided, 1 mine, 2 theirs, 3 keep both, 4 map.
    let resolution: Int
    let keepBothAllowed: Bool
    /// Someone else deleted a model or group the open sequence has
    /// effects on: Keep (1) / Delete with its effects (2) / Map (4).
    let deletedWithEffects: Bool
    let mapTarget: String

    static func load(_ document: XLSequenceDocument) -> [ShowFileMergeItem] {
        document.showFileMergeConflicts().enumerated().map { index, d in
            ShowFileMergeItem(id: index,
                              file: d["file"] as? String ?? "",
                              record: d["record"] as? String ?? "",
                              what: d["what"] as? String ?? "",
                              base: d["base"] as? String ?? "",
                              mine: d["mine"] as? String ?? "",
                              theirs: d["theirs"] as? String ?? "",
                              resolution: (d["resolution"] as? NSNumber)?.intValue ?? 0,
                              keepBothAllowed: (d["keepBothAllowed"] as? NSNumber)?.boolValue ?? false,
                              deletedWithEffects: (d["deletedWithEffects"] as? NSNumber)?.boolValue ?? false,
                              mapTarget: d["mapTarget"] as? String ?? "")
        }
    }
}

/// Starting and finishing a merge of the shared show files. Desktop runs
/// the same core merge (`xLightsFrame::SaveShowFiles`).
enum ShowFileMergeStart {
    case needsDecisions
    case committed(report: String)
    case failed(message: String)

    @MainActor
    static func start(document: XLSequenceDocument, presets: Bool) -> ShowFileMergeStart {
        guard document.prepareShowFileMerge(presets: presets) else {
            document.cancelShowFileMerge()
            return .failed(message: "The changes can't be merged: the version you opened is no longer known. Use Overwrite or Save a Copy.")
        }
        if !document.showFileMergeConflicts().isEmpty {
            return .needsDecisions
        }
        return commit(document: document)
    }

    @MainActor
    static func commit(document: XLSequenceDocument) -> ShowFileMergeStart {
        let r = document.commitShowFileMerge()
        if (r["ok"] as? NSNumber)?.boolValue == true {
            return .committed(report: r["report"] as? String ?? "")
        }
        if (r["needsDecisions"] as? NSNumber)?.boolValue == true {
            return .needsDecisions
        }
        document.cancelShowFileMerge()
        return .failed(message: r["error"] as? String ?? "Couldn't save the merged files.")
    }
}

/// The conflicts of a pending merge, each with Mine / Theirs (Keep Both
/// where allowed). Save stays disabled until every one is decided; Cancel
/// saves nothing.
struct ShowFileMergeSheet: View {
    let document: XLSequenceDocument
    let onCommitted: (String) -> Void
    let onCancel: () -> Void

    @State private var items: [ShowFileMergeItem] = []
    @State private var mapTargets: [String] = []
    @State private var errorMessage: String?

    private var undecided: Int { items.filter { $0.resolution == 0 }.count }

    var body: some View {
        NavigationStack {
            List {
                Section {
                    Text("Someone else saved changes to the same things you changed. Choose which version to keep for each one. Everything else you and they changed is merged.")
                        .font(.callout)
                    if let errorMessage {
                        Text(errorMessage).font(.callout).foregroundStyle(.red)
                    }
                }
                ForEach(items) { item in
                    Section {
                        Text(item.what).font(.subheadline)
                        if item.deletedWithEffects {
                            deletedRow(item)
                        } else {
                            if !item.base.isEmpty {
                                valueRow("Before", item.base)
                            }
                            valueRow("Yours", item.mine)
                            valueRow("Theirs", item.theirs)
                            Picker("Keep", selection: Binding(
                                get: { item.resolution },
                                set: { resolve(item.id, $0) })) {
                                Text("Mine").tag(1)
                                Text("Theirs").tag(2)
                                if item.keepBothAllowed {
                                    Text("Keep Both").tag(3)
                                }
                            }
                            .pickerStyle(.segmented)
                        }
                    } header: {
                        Text("\(item.record) — \(item.file)")
                    }
                }
            }
            .navigationTitle("Merge Changes")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { onCancel() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Save") { save() }.disabled(undecided > 0 || items.isEmpty)
                }
                ToolbarItemGroup(placement: .bottomBar) {
                    Button("All Mine") { resolveAll(1) }
                    Spacer()
                    Text(undecided == 0 ? "Every conflict has a choice" : "\(undecided) of \(items.count) still to choose")
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                    Spacer()
                    Button("All Theirs") { resolveAll(2) }
                }
            }
        }
        .interactiveDismissDisabled(true)
        .onAppear { reload() }
    }

    /// The choices opening a sequence offers for a model the layout lacks
    /// (`MissingModelAliasSheet`), with Keep first.
    private func deletedRow(_ item: ShowFileMergeItem) -> some View {
        let selection = Binding<String>(
            get: {
                switch item.resolution {
                case 1: return "keep"
                case 2: return "delete"
                case 4: return "map:" + item.mapTarget
                default: return ""
                }
            },
            set: { value in
                if value == "keep" {
                    resolve(item.id, 1)
                } else if value == "delete" {
                    resolve(item.id, 2)
                } else if value.hasPrefix("map:") {
                    _ = document.resolveShowFileMergeConflict(item.id, mapTo: String(value.dropFirst(4)))
                    reload()
                }
            })
        return Picker("Action", selection: selection) {
            if item.resolution == 0 {
                Text("Choose…").tag("")
            }
            Text("Keep the model and its effects").tag("keep")
            Text("Delete it and its effects from the sequence").tag("delete")
            ForEach(mapTargets, id: \.self) { name in
                Text("Map its effects to \(name)").tag("map:" + name)
            }
        }
        .pickerStyle(.menu)
    }

    private func reload() {
        items = ShowFileMergeItem.load(document)
        mapTargets = document.showFileMergeMapTargets()
    }

    private func valueRow(_ label: String, _ value: String) -> some View {
        HStack(alignment: .top) {
            Text(label).font(.caption).foregroundStyle(.secondary).frame(width: 56, alignment: .leading)
            Text(value).font(.caption.monospaced()).textSelection(.enabled)
        }
    }

    private func resolve(_ index: Int, _ resolution: Int) {
        _ = document.resolveShowFileMergeConflict(index, resolution: resolution)
        // A decision on a controller can change the layout's conflicts.
        reload()
    }

    private func resolveAll(_ resolution: Int) {
        document.resolveAllShowFileMergeConflicts(resolution)
        reload()
    }

    private func save() {
        switch ShowFileMergeStart.commit(document: document) {
        case .committed(let report):
            onCommitted(report)
        case .needsDecisions:
            errorMessage = "The files changed again while saving. Review the choices below."
            reload()
        case .failed(let message):
            errorMessage = message
        }
    }
}

/// The merge sheet and the save report for one screen.
struct ShowFileMergeFlow: ViewModifier {
    let document: XLSequenceDocument
    @Binding var showSheet: Bool
    @Binding var report: String?
    let onFinished: () -> Void

    func body(content: Content) -> some View {
        content
            .sheet(isPresented: $showSheet) {
                ShowFileMergeSheet(document: document,
                                   onCommitted: { r in
                                       showSheet = false
                                       onFinished()
                                       if !r.isEmpty { report = r }
                                   },
                                   onCancel: {
                                       document.cancelShowFileMerge()
                                       showSheet = false
                                   })
            }
            .alert("Saved with Someone Else's Changes",
                   isPresented: Binding(get: { report != nil },
                                        set: { if !$0 { report = nil } })) {
                Button("OK", role: .cancel) { report = nil }
            } message: {
                Text(report ?? "")
            }
    }
}

/// Someone else sharing the show folder has the sequence being opened
/// open too (from their presence file). Desktop asks the same question
/// (`xLightsFrame::ConfirmOpenSequenceOpenElsewhere`).
struct PresenceOpenWarning: Identifiable {
    let id = UUID()
    let path: String
    let message: String
}

/// Open Anyway / Open Read-Only / Cancel for a `PresenceOpenWarning`.
struct PresenceOpenWarningModifier: ViewModifier {
    let viewModel: SequencerViewModel

    func body(content: Content) -> some View {
        content.alert("Sequence Open Elsewhere",
                      isPresented: Binding(
                        get: { viewModel.pendingPresenceOpen != nil },
                        set: { if !$0 { viewModel.pendingPresenceOpen = nil } }),
                      presenting: viewModel.pendingPresenceOpen) { w in
            Button("Open Anyway") { viewModel.openDespitePresence(w, readOnly: false) }
            Button("Open Read-Only") { viewModel.openDespitePresence(w, readOnly: true) }
            Button("Cancel", role: .cancel) { viewModel.pendingPresenceOpen = nil }
        } message: { w in
            Text(w.message)
        }
    }
}

/// A one-line notice that someone else is editing the layout or
/// controllers in the shared show folder.
struct PresenceHintBanner: View {
    let message: String

    var body: some View {
        HStack(spacing: 8) {
            Image(systemName: "person.2.fill")
            Text(message + ". Saving changes to the same files at the same time can lose one of them.")
                .font(.caption)
                .lineLimit(2)
            Spacer()
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 6)
        .foregroundStyle(.white)
        .background(Color.orange)
    }
}
