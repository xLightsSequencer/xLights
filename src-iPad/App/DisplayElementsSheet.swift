import SwiftUI

// F-6 — Display Elements editor. Ports the desktop
// `ViewsModelsPanel` (src-ui-wx/layout/ViewsModelsPanel.cpp) into a
// modal sheet. Presented from the View picker's dropdown on the main
// sequencer.
//
// A "view" is a named, ordered subset of the show's models plus an
// implicit set of timing tracks (encoded in each TimingElement's
// `mViews` CSV). The Master View (index 0) always exists, can't be
// renamed or deleted, and contains every model element; the members
// list there just toggles global `Element::SetVisible` for each row.
//
// Mutations go through `XLSequenceDocument`'s F-6 bridge, which marks
// the sequence dirty and posts `XLViewsChanged`. The sheet listens on
// that notification and re-snapshots its data whenever anything
// changes.

struct DisplayElementsSheet: View {
    @Environment(\.dismiss) private var dismiss
    @Environment(SequencerViewModel.self) var viewModel
    @State private var model = DisplayElementsVM()
    @State private var selectedViewIdx: Int = 0

    // Lightweight alert plumbing for Add / Rename / Clone / Delete.
    private enum TextAlertKind {
        case addView, renameView(idx: Int, current: String), cloneView(idx: Int)
        case newTimingTrack
    }
    @State private var textAlert: (kind: TextAlertKind, text: String)? = nil
    private struct IdString: Identifiable { let id: String }
    @State private var confirmDelete: IdString? = nil
    @State private var errorText: String? = nil

    // Master-View remove confirmation. Removing from Master View
    // deletes the Element entirely (including any effects on it) —
    // desktop warns before doing so. `.confirmationDialog` needs an
    // identifiable value to drive both presentation and the delete
    // dispatch.
    private struct RemoveTarget: Identifiable {
        let name: String
        let kind: DisplayElementsVM.Member.Kind
        let hasEffects: Bool
        var id: String { "\(kind == .model ? "m" : "t"):\(name)" }
    }
    @State private var confirmRemove: RemoveTarget? = nil

    // Local "Add Timing Track" sheet state. iOS only allows one
    // sheet per ancestor chain at a time — flipping
    // `viewModel.showingAddTimingTrack` from inside this sheet would
    // log "Currently, only presenting a single sheet is supported"
    // because the app-level sheet can't open on top of us. A local
    // @State + .sheet on this body stacks the new sheet correctly.
    @State private var showingAddTimingTrackLocal = false

    var body: some View {
        NavigationStack {
            NavigationSplitView {
                sidebar
                    .navigationSplitViewColumnWidth(min: 220, ideal: 260, max: 340)
            } detail: {
                detail
            }
            .navigationTitle("Display Elements")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") { dismiss() }
                }
            }
        }
        .onAppear {
            model.attach(document: viewModel.document)
            model.refresh()
            // Seed to the currently-active view.
            let cur = Int(viewModel.document.currentViewIndex())
            selectedViewIdx = max(0, min(cur, max(model.views.count - 1, 0)))
        }
        .onDisappear {
            model.detach()
        }
        .onChange(of: model.views) { _, newViews in
            // If our selection fell off the end (e.g. user deleted the
            // currently-edited view), snap back to Master View.
            if selectedViewIdx >= newViews.count { selectedViewIdx = 0 }
        }
        .alert(alertTitle, isPresented: Binding(
            get: { textAlert != nil },
            set: { if !$0 { textAlert = nil } }
        )) {
            TextField("Name", text: Binding(
                get: { textAlert?.text ?? "" },
                set: { textAlert?.text = $0 }
            ))
            .textInputAutocapitalization(.words)
            .autocorrectionDisabled(true)
            Button("OK") { commitTextAlert() }
            Button("Cancel", role: .cancel) { textAlert = nil }
        }
        .confirmationDialog("Delete this view?",
                            isPresented: Binding(
                                get: { confirmDelete != nil },
                                set: { if !$0 { confirmDelete = nil } }
                            ),
                            titleVisibility: .visible) {
            Button("Delete", role: .destructive) {
                if let target = confirmDelete,
                   let idx = Int(target.id) {
                    performDelete(idx: idx)
                }
                confirmDelete = nil
            }
            Button("Cancel", role: .cancel) { confirmDelete = nil }
        } message: {
            Text("Models will stay in the sequence; the view and its layout are removed.")
        }
        .alert("Error", isPresented: Binding(
            get: { errorText != nil },
            set: { if !$0 { errorText = nil } }
        )) {
            Button("OK", role: .cancel) { errorText = nil }
        } message: {
            Text(errorText ?? "")
        }
        .confirmationDialog("Remove from Master View?",
                            isPresented: Binding(
                                get: { confirmRemove != nil },
                                set: { if !$0 { confirmRemove = nil } }
                            ),
                            titleVisibility: .visible) {
            Button("Remove", role: .destructive) { performMasterRemove() }
            Button("Cancel", role: .cancel) { confirmRemove = nil }
        } message: {
            Text(removeConfirmationMessage)
        }
        // Local "Add Timing Track" sheet — stacked on top of this
        // sheet so iOS doesn't reject it the way the app-level
        // viewModel.showingAddTimingTrack sheet would.
        .sheet(isPresented: $showingAddTimingTrackLocal) {
            AddTimingTrackSheet()
                .environment(viewModel)
        }
    }

    // MARK: - Sidebar: views list

    private var sidebar: some View {
        VStack(spacing: 0) {
            List {
                ForEach(Array(model.views.enumerated()), id: \.offset) { idx, name in
                    HStack {
                        Image(systemName: idx == 0 ? "lock.square" : "rectangle.stack")
                            .foregroundStyle(.secondary)
                        Text(name)
                            .lineLimit(1)
                        Spacer(minLength: 0)
                    }
                    .contentShape(Rectangle())
                    .onTapGesture { selectedViewIdx = idx }
                    .listRowBackground(
                        selectedViewIdx == idx
                            ? Color.accentColor.opacity(0.2)
                            : Color.clear
                    )
                    .contextMenu {
                        if idx != 0 {
                            Button("Rename…") {
                                textAlert = (.renameView(idx: idx, current: name), name)
                            }
                            Button("Clone…") {
                                textAlert = (.cloneView(idx: idx), "Copy Of \(name)")
                            }
                            Button("Delete", role: .destructive) {
                                confirmDelete = IdString(id: "\(idx)")
                            }
                        } else {
                            Button("Clone…") {
                                textAlert = (.cloneView(idx: idx), "Copy Of \(name)")
                            }
                        }
                    }
                }
            }
            .listStyle(.inset)

            Divider()

            HStack(spacing: 10) {
                Button {
                    textAlert = (.addView, "")
                } label: {
                    Image(systemName: "plus")
                }
                .help("Add View")

                Button(role: .destructive) {
                    confirmDelete = IdString(id: "\(selectedViewIdx)")
                } label: {
                    Image(systemName: "minus")
                }
                .disabled(selectedViewIdx == 0)
                .help("Delete View")

                Button {
                    guard selectedViewIdx > 0, selectedViewIdx < model.views.count else { return }
                    let current = model.views[selectedViewIdx]
                    textAlert = (.renameView(idx: selectedViewIdx, current: current), current)
                } label: {
                    Image(systemName: "pencil")
                }
                .disabled(selectedViewIdx == 0)
                .help("Rename View")

                Button {
                    let current = selectedViewIdx < model.views.count
                        ? model.views[selectedViewIdx] : ""
                    textAlert = (.cloneView(idx: selectedViewIdx), "Copy Of \(current)")
                } label: {
                    Image(systemName: "doc.on.doc")
                }
                .help("Clone View")

                Spacer()

                Button {
                    if viewModel.document.moveViewUp(atIndex: Int32(selectedViewIdx)) {
                        selectedViewIdx -= 1
                    }
                } label: {
                    Image(systemName: "arrow.up")
                }
                .disabled(selectedViewIdx <= 1)

                Button {
                    if viewModel.document.moveViewDown(atIndex: Int32(selectedViewIdx)) {
                        selectedViewIdx += 1
                    }
                } label: {
                    Image(systemName: "arrow.down")
                }
                .disabled(selectedViewIdx < 1
                          || selectedViewIdx >= model.views.count - 1)
            }
            .padding(10)
        }
    }

    // MARK: - Detail: members + available

    private var detail: some View {
        Group {
            if model.views.isEmpty {
                ContentUnavailableView("No views loaded",
                                        systemImage: "rectangle.stack",
                                        description: Text("Open a sequence to edit its views."))
            } else if selectedViewIdx == 0 {
                masterViewDetail
            } else {
                userViewDetail
            }
        }
    }

    // Master View: two-pane transfer UI. "Available" = models in the
    // show's layout that haven't been opted into this sequence yet
    // (plus an "Add Timing Track…" button). "In Master View" = the
    // Elements the sequence holds, with visibility toggles and remove
    // (with a confirmation warning when effects exist).
    private var masterViewDetail: some View {
        let members = model.masterMembers()
        let available = model.masterAvailable()
        return VStack(alignment: .leading, spacing: 0) {
            Text("Master View — models and timing tracks that this sequence can hold effects on. Adding a model brings it in from the show layout; removing deletes the element and any effects on it.")
                .font(.caption)
                .foregroundStyle(.secondary)
                .padding([.horizontal, .top])

            GeometryReader { geo in
                VStack(spacing: 0) {
                    // Available pane — show-layout models + Add Timing
                    // Track button. Models in ModelManager that aren't
                    // yet part of the sequence land here; an Effect
                    // Sequence starts with every model in this pane
                    // and zero members.
                    VStack(alignment: .leading, spacing: 4) {
                        HStack {
                            Text("Available in Show")
                                .font(.headline)
                            Spacer()
                            Button {
                                showingAddTimingTrackLocal = true
                            } label: {
                                Label("Add Timing Track…",
                                      systemImage: "plus.rectangle")
                            }
                            Button {
                                addAllMasterAvailable(available)
                            } label: {
                                Label("Add All", systemImage: "arrow.right.to.line")
                            }
                            .disabled(available.isEmpty)
                        }
                        .padding(.horizontal)
                        .padding(.top, 8)

                        List {
                            if available.isEmpty {
                                Text("Every model in the show layout is already in this sequence.")
                                    .foregroundStyle(.secondary)
                                    .font(.caption)
                            } else {
                                ForEach(available) { item in
                                    HStack {
                                        kindBadge(item.kind)
                                        Text(item.name)
                                        Spacer()
                                        Button {
                                            _ = viewModel.document.addModel(toMasterView: item.name)
                                        } label: {
                                            Image(systemName: "plus.circle.fill")
                                                .foregroundStyle(.blue)
                                        }
                                        .buttonStyle(.plain)
                                    }
                                }
                            }
                        }
                        .listStyle(.inset)
                    }
                    .frame(height: geo.size.height * 0.5)

                    Divider()

                    // Members pane — sequence Elements. Visibility eye
                    // toggles (.SetVisible / .SetMasterVisible); remove
                    // triggers a confirmation when effects exist.
                    VStack(alignment: .leading, spacing: 4) {
                        HStack {
                            Text("In Master View")
                                .font(.headline)
                            Spacer()
                            Text("\(members.count) element\(members.count == 1 ? "" : "s")")
                                .font(.caption)
                                .foregroundStyle(.secondary)
                        }
                        .padding(.horizontal)
                        .padding(.top, 8)

                        List {
                            if members.isEmpty {
                                Text("No elements yet. Add models from above, or create a timing track.")
                                    .foregroundStyle(.secondary)
                                    .font(.caption)
                            } else {
                                Section("Models") {
                                    ForEach(members.filter { $0.kind == .model }) { item in
                                        masterMemberRow(item)
                                    }
                                }
                                Section("Timing Tracks") {
                                    ForEach(members.filter { $0.kind == .timing }) { item in
                                        masterMemberRow(item)
                                    }
                                }
                            }
                        }
                        .listStyle(.inset)
                    }
                    .frame(height: geo.size.height * 0.5)
                }
            }
        }
    }

    // One row inside the Master-View members pane. Visibility eye +
    // remove button; the remove path goes through the confirmation
    // dialog when the element carries effects.
    @ViewBuilder
    private func masterMemberRow(_ item: DisplayElementsVM.Member) -> some View {
        HStack {
            kindBadge(item.kind)
            Text(item.name)
            Spacer()
            Button {
                _ = viewModel.document.setElementVisible(item.name,
                                                          visible: !item.visible)
            } label: {
                Image(systemName: item.visible ? "eye" : "eye.slash")
                    .foregroundStyle(item.visible ? .primary : .secondary)
            }
            .buttonStyle(.plain)
            Button {
                requestMasterRemove(item)
            } label: {
                Image(systemName: "minus.circle.fill")
                    .foregroundStyle(.red)
            }
            .buttonStyle(.plain)
        }
    }

    // User view: two panes. "Available" (pool) on top with buttons to
    // add to the view; "Members" below with reorder + remove.
    private var userViewDetail: some View {
        let membership = model.userViewMembership(viewIdx: selectedViewIdx)
        return VStack(alignment: .leading, spacing: 0) {
            Text("\(model.views[safe: selectedViewIdx] ?? "") — pick which models and timing tracks appear when this view is active. Reorder members with drag handles.")
                .font(.caption)
                .foregroundStyle(.secondary)
                .padding([.horizontal, .top])

            GeometryReader { geo in
                VStack(spacing: 0) {
                    // Available pane
                    VStack(alignment: .leading, spacing: 4) {
                        HStack {
                            Text("Available")
                                .font(.headline)
                            Spacer()
                            Button {
                                addAllAvailable()
                            } label: {
                                Label("Add All", systemImage: "arrow.right.to.line")
                            }
                            .disabled(membership.available.isEmpty)
                        }
                        .padding(.horizontal)
                        .padding(.top, 8)

                        List {
                            if membership.available.isEmpty {
                                Text("Every model and timing track is already in this view.")
                                    .foregroundStyle(.secondary)
                                    .font(.caption)
                            } else {
                                ForEach(membership.available) { item in
                                    HStack {
                                        kindBadge(item.kind)
                                        Text(item.name)
                                        Spacer()
                                        Button {
                                            addItem(item)
                                        } label: {
                                            Image(systemName: "plus.circle.fill")
                                                .foregroundStyle(.blue)
                                        }
                                        .buttonStyle(.plain)
                                    }
                                }
                            }
                        }
                        .listStyle(.inset)
                    }
                    .frame(height: geo.size.height * 0.5)

                    Divider()

                    // Members pane
                    VStack(alignment: .leading, spacing: 4) {
                        HStack {
                            Text("In This View")
                                .font(.headline)
                            Spacer()
                            Button(role: .destructive) {
                                removeAllMembers(membership.members)
                            } label: {
                                Label("Remove All", systemImage: "arrow.left.to.line")
                            }
                            .disabled(membership.members.isEmpty)
                        }
                        .padding(.horizontal)
                        .padding(.top, 8)

                        List {
                            if membership.members.isEmpty {
                                Text("No members yet. Add models or timing tracks above.")
                                    .foregroundStyle(.secondary)
                                    .font(.caption)
                            } else {
                                ForEach(membership.members) { item in
                                    HStack {
                                        kindBadge(item.kind)
                                        Text(item.name)
                                        Spacer()
                                        Button {
                                            removeItem(item)
                                        } label: {
                                            Image(systemName: "minus.circle.fill")
                                                .foregroundStyle(.red)
                                        }
                                        .buttonStyle(.plain)
                                    }
                                }
                                .onMove { indices, dest in
                                    // Model-only entries can be reordered
                                    // directly through the bridge; timing
                                    // membership has no explicit ordering
                                    // (CSV order is rebuilt on each edit),
                                    // so filter to model rows before
                                    // dispatching a move.
                                    moveMembers(membership.members,
                                                from: indices,
                                                to: dest)
                                }
                            }
                        }
                        .listStyle(.inset)
                        .environment(\.editMode, .constant(.active))
                    }
                    .frame(height: geo.size.height * 0.5)
                }
            }
        }
    }

    // MARK: - Shared row renderers

    @ViewBuilder
    private func memberRow(_ item: DisplayElementsVM.Member,
                            showsVisibility: Bool) -> some View {
        HStack {
            kindBadge(item.kind)
            Text(item.name)
            Spacer()
            if showsVisibility {
                Button {
                    _ = viewModel.document.setElementVisible(item.name,
                                                              visible: !item.visible)
                } label: {
                    Image(systemName: item.visible ? "eye" : "eye.slash")
                        .foregroundStyle(item.visible ? .primary : .secondary)
                }
                .buttonStyle(.plain)
            }
        }
    }

    @ViewBuilder
    private func kindBadge(_ kind: DisplayElementsVM.Member.Kind) -> some View {
        switch kind {
        case .model:
            Image(systemName: "cube.transparent")
                .foregroundStyle(.blue)
                .frame(width: 22)
        case .timing:
            Image(systemName: "metronome")
                .foregroundStyle(.orange)
                .frame(width: 22)
        }
    }

    // MARK: - Actions

    private var alertTitle: String {
        guard let alert = textAlert else { return "" }
        switch alert.kind {
        case .addView:                       return "Create View"
        case .renameView:                    return "Rename View"
        case .cloneView:                     return "Clone View"
        case .newTimingTrack:                return "Add Timing Track"
        }
    }

    private func commitTextAlert() {
        guard let alert = textAlert else { return }
        let name = alert.text.trimmingCharacters(in: .whitespacesAndNewlines)
        defer { textAlert = nil }
        if name.isEmpty { return }
        switch alert.kind {
        case .addView:
            if viewModel.document.addView(named: name) {
                model.refresh()
                // Select the freshly-added view (always appended at end).
                selectedViewIdx = model.views.firstIndex(of: name) ?? selectedViewIdx
            } else {
                errorText = "Couldn't create \"\(name)\" — name is empty or already in use."
            }
        case .renameView(let idx, let current):
            if name == current { return }
            if !viewModel.document.renameView(atIndex: Int32(idx), to: name) {
                errorText = "Couldn't rename to \"\(name)\"."
            }
        case .cloneView(let idx):
            if viewModel.document.cloneView(atIndex: Int32(idx), as: name) {
                model.refresh()
                selectedViewIdx = model.views.firstIndex(of: name) ?? selectedViewIdx
            } else {
                errorText = "Couldn't clone as \"\(name)\" — name is empty or already in use."
            }
        case .newTimingTrack:
            // Routes through the same bridge as the row-header B73
            // flow, so the new track lands as a fresh variable timing
            // track visible in Master.
            if !viewModel.addTimingTrack(name: name) {
                errorText = "Couldn't create timing track \"\(name)\"."
            }
        }
    }

    // Master-View remove path. Desktop shows a warning when the model
    // has effects ("Delete all effects and layers…?") and a stronger
    // warning for timings ("Removing from the Master View will delete
    // the timing track"). We collect both into a single
    // `.confirmationDialog` keyed off the chosen member.
    private func requestMasterRemove(_ item: DisplayElementsVM.Member) {
        let hasEffects = viewModel.document.elementHasEffects(item.name)
        if hasEffects || item.kind == .timing {
            confirmRemove = RemoveTarget(name: item.name,
                                          kind: item.kind,
                                          hasEffects: hasEffects)
        } else {
            // No effects and it's a model — safe to remove silently.
            _ = viewModel.document.removeElement(fromMasterView: item.name)
            viewModel.reloadRows()
        }
    }

    private var removeConfirmationMessage: String {
        guard let target = confirmRemove else { return "" }
        switch target.kind {
        case .timing:
            return "Removing \"\(target.name)\" from the Master View will delete the timing track from this sequence. This cannot be undone."
        case .model:
            return "\"\(target.name)\" has effects on it. Removing it will delete every effect and layer on the model. This cannot be undone."
        }
    }

    private func performMasterRemove() {
        guard let target = confirmRemove else { return }
        _ = viewModel.document.removeElement(fromMasterView: target.name)
        viewModel.reloadRows()
        confirmRemove = nil
    }

    private func addAllMasterAvailable(_ items: [DisplayElementsVM.Member]) {
        for item in items where item.kind == .model {
            _ = viewModel.document.addModel(toMasterView: item.name)
        }
    }

    private func performDelete(idx: Int) {
        guard idx > 0 else { return }
        if viewModel.document.deleteView(atIndex: Int32(idx)) {
            if selectedViewIdx == idx { selectedViewIdx = 0 }
            else if selectedViewIdx > idx { selectedViewIdx -= 1 }
            // If the deleted view was the currently-active view on the
            // main sequencer, the bridge has already fallen back to
            // Master — tell the main grid to reload rows so the
            // sequencer snaps in sync.
            viewModel.reloadRows()
        }
    }

    private func addItem(_ item: DisplayElementsVM.Member) {
        guard selectedViewIdx > 0,
              selectedViewIdx < model.views.count else { return }
        let viewName = model.views[selectedViewIdx]
        switch item.kind {
        case .model:
            _ = viewModel.document.addModel(item.name,
                                             toViewAtIndex: Int32(selectedViewIdx),
                                             atPosition: -1)
        case .timing:
            _ = viewModel.document.addTiming(item.name, toViewNamed: viewName)
        }
    }

    private func addAllAvailable() {
        let membership = model.userViewMembership(viewIdx: selectedViewIdx)
        for item in membership.available { addItem(item) }
    }

    private func removeItem(_ item: DisplayElementsVM.Member) {
        guard selectedViewIdx > 0,
              selectedViewIdx < model.views.count else { return }
        let viewName = model.views[selectedViewIdx]
        switch item.kind {
        case .model:
            _ = viewModel.document.removeModel(item.name,
                                                fromViewAtIndex: Int32(selectedViewIdx))
        case .timing:
            _ = viewModel.document.removeTiming(item.name, fromViewNamed: viewName)
        }
    }

    private func removeAllMembers(_ members: [DisplayElementsVM.Member]) {
        for item in members { removeItem(item) }
    }

    private func moveMembers(_ members: [DisplayElementsVM.Member],
                              from source: IndexSet,
                              to destination: Int) {
        // Reorder only operates on model entries. Construct the target
        // ordering and push each moved model to its new position via
        // `moveModel:inViewAtIndex:toPosition:`.
        var reordered = members
        reordered.move(fromOffsets: source, toOffset: destination)
        // Filter the reordered list to models (their stored index is
        // what the bridge moves against).
        let modelOrder = reordered.enumerated().compactMap { (idx, m) -> (String, Int)? in
            m.kind == .model ? (m.name, idx) : nil
        }
        for (name, _) in modelOrder {
            // Recompute position relative to model-only ordering.
            let modelNames = reordered.filter { $0.kind == .model }.map { $0.name }
            if let pos = modelNames.firstIndex(of: name) {
                _ = viewModel.document.moveModel(name,
                                                  inViewAtIndex: Int32(selectedViewIdx),
                                                  toPosition: Int32(pos))
            }
        }
    }
}

// MARK: - View model

@Observable
@MainActor
final class DisplayElementsVM {
    private(set) var views: [String] = []
    // Models the sequence has opted in (i.e. Master-View Elements).
    // Empty for a freshly-created Effect Sequence.
    private(set) var allModels: [String] = []
    // Models present in the show's `ModelManager` but not yet an
    // Element in this sequence. These feed the Master-View
    // "Available" pane; adding one creates a fresh Element via
    // `addModelToMasterView:`.
    private(set) var modelsAvailableInShow: [String] = []
    private(set) var allTimings: [String] = []
    // `TimingElement::mViews` CSV split per timing name.
    private var timingViews: [String: Set<String>] = [:]
    // `Element::GetVisible()` / `TimingElement::GetMasterVisible()`.
    private var visibility: [String: Bool] = [:]
    // Per-user-view ordered model lists, indexed by view index.
    private var modelsByViewIdx: [Int: [String]] = [:]

    @ObservationIgnored private weak var document: XLSequenceDocument?
    @ObservationIgnored private var observer: NSObjectProtocol?

    struct Member: Identifiable, Hashable {
        enum Kind { case model, timing }
        let kind: Kind
        let name: String
        let visible: Bool
        var id: String { "\(kind == .model ? "m" : "t"):\(name)" }
    }

    struct Membership {
        var members: [Member]
        var available: [Member]
    }

    func attach(document: XLSequenceDocument) {
        self.document = document
        if observer == nil {
            observer = NotificationCenter.default.addObserver(
                forName: NSNotification.Name("XLViewsChanged"),
                object: nil,
                queue: .main
            ) { [weak self] _ in
                MainActor.assumeIsolated {
                    self?.refresh()
                }
            }
        }
    }

    // Swift 6 concurrency disallows touching MainActor-isolated
    // properties from a nonisolated `deinit`, so the sheet calls
    // `detach()` from `.onDisappear` to release the notification
    // observer. Good enough — the VM is @State-owned by the sheet
    // and has the same lifetime.
    func detach() {
        if let observer {
            NotificationCenter.default.removeObserver(observer)
            self.observer = nil
        }
        document = nil
    }

    func refresh() {
        guard let doc = document else {
            views = []; allModels = []; modelsAvailableInShow = []
            allTimings = []
            timingViews = [:]; visibility = [:]; modelsByViewIdx = [:]
            return
        }
        views = (doc.availableViews() as [String])
        allModels = (doc.allModelNamesInShow() as [String])
        modelsAvailableInShow = (doc.modelsAvailableInShowLayout() as [String])
        allTimings = (doc.allTimingTrackNames() as [String])

        var tv: [String: Set<String>] = [:]
        var vis: [String: Bool] = [:]
        for t in allTimings {
            tv[t] = Set((doc.viewsContainingTiming(t) as [String]))
            vis[t] = doc.elementVisible(t)
        }
        for m in allModels {
            vis[m] = doc.elementVisible(m)
        }
        timingViews = tv
        visibility = vis

        var mvi: [Int: [String]] = [:]
        for idx in 0..<views.count {
            mvi[idx] = (doc.modelsInView(atIndex: Int32(idx)) as [String])
        }
        modelsByViewIdx = mvi
    }

    func masterMembers() -> [Member] {
        let modelMembers = allModels.map { name in
            Member(kind: .model, name: name, visible: visibility[name] ?? true)
        }
        let timingMembers = allTimings.map { name in
            Member(kind: .timing, name: name, visible: visibility[name] ?? false)
        }
        return modelMembers + timingMembers
    }

    // Show-layout models not yet in the sequence — feeds the Master
    // View Available pane. Timing tracks are intentionally not listed
    // here: the only way to create one is via the Add-Timing-Track
    // flow (desktop uses a full `NewTimingDialog`; iPad routes through
    // `addTimingTrackNamed:`). The sheet exposes that as an explicit
    // button in the pane header instead of a list entry.
    func masterAvailable() -> [Member] {
        modelsAvailableInShow.map { name in
            Member(kind: .model, name: name, visible: true)
        }
    }

    func userViewMembership(viewIdx: Int) -> Membership {
        guard viewIdx > 0, viewIdx < views.count else {
            return Membership(members: [], available: [])
        }
        let viewName = views[viewIdx]
        let memberModels = modelsByViewIdx[viewIdx] ?? []
        let memberTimings = allTimings.filter { (timingViews[$0] ?? []).contains(viewName) }

        let members = memberModels.map { Member(kind: .model, name: $0, visible: true) }
                    + memberTimings.map { Member(kind: .timing, name: $0, visible: true) }

        let memberModelSet = Set(memberModels)
        let availModels = allModels.filter { !memberModelSet.contains($0) }
        let memberTimingSet = Set(memberTimings)
        let availTimings = allTimings.filter { !memberTimingSet.contains($0) }
        let available = availModels.map { Member(kind: .model, name: $0, visible: true) }
                      + availTimings.map { Member(kind: .timing, name: $0, visible: true) }

        return Membership(members: members, available: available)
    }
}

private extension Array {
    subscript(safe index: Int) -> Element? {
        indices.contains(index) ? self[index] : nil
    }
}
