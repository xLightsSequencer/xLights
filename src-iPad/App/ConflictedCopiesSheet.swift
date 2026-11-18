import SwiftUI

/// A copy a sync client (iCloud, Dropbox, OneDrive, Google Drive,
/// Syncthing) left of a show-wide file when two saves crossed. Desktop
/// offers the same at load (`xLightsFrame::OfferConflictedCopies`).
struct ConflictedCopyOffer: Identifiable, Equatable {
    let id: String // the copy's path
    let name: String
    let file: String
    let client: String
    let sameAsCurrent: Bool

    static func load(_ document: XLSequenceDocument) -> [ConflictedCopyOffer] {
        document.conflictedCopies().map { d in
            ConflictedCopyOffer(id: d["path"] as? String ?? "",
                                name: d["name"] as? String ?? "",
                                file: d["file"] as? String ?? "",
                                client: d["client"] as? String ?? "",
                                sameAsCurrent: (d["sameAsCurrent"] as? NSNumber)?.boolValue ?? false)
        }
    }
}

/// Lists the conflicted copies with Merge / Ignore for each; "Not Now"
/// leaves them for the next time the show loads. A merge that needs
/// decisions shows the usual conflict list (`ShowFileMergeSheet`) in
/// place, with the copy as "Copy" and the current file as "Current".
struct ConflictedCopiesSheet: View {
    let document: XLSequenceDocument
    @Binding var offers: [ConflictedCopyOffer]
    let onMerged: () -> Void
    let onClose: () -> Void

    @State private var merging: ConflictedCopyOffer?
    @State private var results: [String] = []
    @State private var errorMessage: String?

    var body: some View {
        if let merging {
            ShowFileMergeSheet(document: document,
                               onCommitted: { report in finished(merging, report) },
                               onCancel: {
                                   document.cancelShowFileMerge()
                                   self.merging = nil
                               })
        } else {
            NavigationStack {
                List {
                    Section {
                        Text("Your sync service kept a second copy of a show file because two saves happened at the same time. Merge each copy into the file to keep both sets of changes. A merged copy is moved to the Backup folder.")
                            .font(.callout)
                        if let errorMessage {
                            Text(errorMessage).font(.callout).foregroundStyle(.red)
                        }
                    }
                    ForEach(offers) { offer in
                        Section {
                            Text(offer.name).font(.subheadline)
                            Text("Left by \(offer.client)").font(.caption).foregroundStyle(.secondary)
                            if offer.sameAsCurrent {
                                Text("It holds the same as the current file; merging only moves it out of the way.")
                                    .font(.caption).foregroundStyle(.secondary)
                            }
                            HStack {
                                Button("Merge") { merge(offer) }
                                    .buttonStyle(.borderedProminent)
                                Spacer()
                                Button("Ignore") {
                                    _ = document.ignoreConflictedCopy(offer.id)
                                    offers.removeAll { $0.id == offer.id }
                                }
                                .buttonStyle(.bordered)
                            }
                        } header: {
                            Text(offer.file)
                        }
                    }
                    if !results.isEmpty {
                        Section("Merged") {
                            ForEach(results, id: \.self) { Text($0).font(.caption) }
                        }
                    }
                }
                .navigationTitle("Conflicted Copies")
                .navigationBarTitleDisplayMode(.inline)
                .toolbar {
                    ToolbarItem(placement: .cancellationAction) {
                        Button(offers.isEmpty ? "Done" : "Not Now") { onClose() }
                    }
                }
            }
        }
    }

    private func merge(_ offer: ConflictedCopyOffer) {
        errorMessage = nil
        if let error = document.prepareConflictedCopyMerge(offer.id) {
            errorMessage = error
            return
        }
        if !document.showFileMergeConflicts().isEmpty {
            merging = offer
            return
        }
        switch ShowFileMergeStart.commit(document: document) {
        case .committed(let report):
            finished(offer, report)
        case .needsDecisions:
            merging = offer
        case .failed(let message):
            errorMessage = message
        }
    }

    private func finished(_ offer: ConflictedCopyOffer, _ report: String) {
        merging = nil
        offers.removeAll { $0.id == offer.id }
        results.append(report.isEmpty ? "\(offer.name) was merged into \(offer.file)." : report)
        onMerged()
    }
}

/// The notices a show load and a sequence open can raise on their own:
/// conflicted copies to merge, and rows renamed from the rename hints.
struct ShowFolderNoticesModifier: ViewModifier {
    let viewModel: SequencerViewModel

    func body(content: Content) -> some View {
        content
            .sheet(isPresented: Binding(get: { viewModel.showConflictedCopies },
                                        set: { viewModel.showConflictedCopies = $0 })) {
                ConflictedCopiesSheet(document: viewModel.document,
                                      offers: Binding(get: { viewModel.conflictedCopyOffers },
                                                      set: { viewModel.conflictedCopyOffers = $0 }),
                                      onMerged: {
                                          viewModel.reloadPresets()
                                          if viewModel.isSequenceLoaded { viewModel.reloadRows() }
                                      },
                                      onClose: { viewModel.showConflictedCopies = false })
                    .interactiveDismissDisabled(true)
            }
            .alert("Renamed Models",
                   isPresented: Binding(get: { viewModel.renamedModelsNotice != nil },
                                        set: { if !$0 { viewModel.renamedModelsNotice = nil } })) {
                Button("OK", role: .cancel) { viewModel.renamedModelsNotice = nil }
            } message: {
                Text(viewModel.renamedModelsNotice ?? "")
            }
    }
}
