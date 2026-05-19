import SwiftUI

/// Persists which sequences are selected for batch render, keyed by show
/// folder path so different shows track their own checkbox state. Stored
/// values are *relative* xsq paths (under the show folder) so renaming the
/// show folder doesn't invalidate the set, and so paths stay portable
/// between iCloud devices.
enum BatchRenderSelections {
    private static func suite() -> UserDefaults {
        UserDefaults(suiteName: "xLights-BatchRender") ?? .standard
    }

    private static func key(showFolder: String) -> String {
        "selections:" + showFolder
    }

    static func load(showFolder: String) -> Set<String> {
        guard let arr = suite().stringArray(forKey: key(showFolder: showFolder)) else {
            return []
        }
        return Set(arr)
    }

    static func save(_ selections: Set<String>, showFolder: String) {
        suite().set(Array(selections).sorted(),
                    forKey: key(showFolder: showFolder))
    }
}

/// Batch render sheet. Three phases mapped 1:1 to `BatchRenderRunner.Phase`:
/// the user selects sequences, watches progress, and then sees a summary.
/// Selection state is persisted per show folder; a "Render N selected"
/// button at the bottom kicks the runner.
struct BatchRenderSheet: View {
    @Environment(SequencerViewModel.self) var viewModel
    @Environment(\.dismiss) var dismiss

    @State private var entries: [SequenceEntry] = []
    @State private var selected: Set<String> = []
    @State private var runner: BatchRenderRunner?

    var body: some View {
        NavigationStack {
            Group {
                if let runner = runner {
                    switch runner.phase {
                    case .idle:
                        selectionList(runner: runner)
                    case .running(let i, let total, let name):
                        runningView(currentIndex: i, total: total, name: name)
                    case .done(let rendered, let failed, let cancelled):
                        doneView(rendered: rendered, failed: failed, cancelled: cancelled)
                    }
                } else {
                    ProgressView().controlSize(.large)
                }
            }
            .navigationTitle("Batch Render")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    if isRunning {
                        Button("Cancel", role: .destructive) {
                            runner?.cancel()
                        }
                    } else {
                        Button("Close") { dismiss() }
                    }
                }
            }
        }
        .interactiveDismissDisabled(isRunning)
        .onAppear {
            if runner == nil {
                runner = BatchRenderRunner(document: viewModel.document)
            }
            let folder = viewModel.showFolderPath ?? ""
            entries = SequenceScanner.scan(showFolder: folder)
            selected = BatchRenderSelections.load(showFolder: folder)
                .intersection(Set(entries.map(\.relativePath)))
        }
    }

    private var isRunning: Bool {
        if case .running = runner?.phase { return true }
        return false
    }

    // MARK: - Selection phase

    @ViewBuilder
    private func selectionList(runner: BatchRenderRunner) -> some View {
        if entries.isEmpty {
            ContentUnavailableView("No sequences in show folder",
                                    systemImage: "tray",
                                    description: Text("Add or move .xsq files into the show folder to render them in batch."))
        } else {
            List {
                Section {
                    HStack {
                        Button("Select All") { selectAll() }
                        Spacer()
                        Button("Select None") { selectNone() }
                            .disabled(selected.isEmpty)
                    }
                }
                Section("Sequences") {
                    ForEach(entries) { entry in
                        Button {
                            toggle(entry)
                        } label: {
                            HStack(spacing: 12) {
                                Image(systemName: selected.contains(entry.relativePath)
                                      ? "checkmark.square.fill"
                                      : "square")
                                    .foregroundStyle(selected.contains(entry.relativePath)
                                                     ? Color.accentColor
                                                     : Color.secondary)
                                VStack(alignment: .leading, spacing: 2) {
                                    Text(entry.displayName)
                                        .foregroundStyle(.primary)
                                    if !entry.parentRelativePath.isEmpty {
                                        Text(entry.parentRelativePath)
                                            .font(.caption2)
                                            .foregroundStyle(.secondary)
                                            .lineLimit(1)
                                            .truncationMode(.middle)
                                    }
                                }
                                Spacer()
                            }
                            .contentShape(Rectangle())
                        }
                        .buttonStyle(.plain)
                    }
                }
            }
            .safeAreaInset(edge: .bottom) {
                Button {
                    let toRun = entries.filter { selected.contains($0.relativePath) }
                    runner.start(entries: toRun)
                } label: {
                    Text(selected.isEmpty
                         ? "Select sequences to render"
                         : "Render \(selected.count) selected")
                        .font(.headline)
                        .frame(maxWidth: .infinity, minHeight: 44)
                }
                .buttonStyle(.borderedProminent)
                .disabled(selected.isEmpty)
                .padding(.horizontal)
                .padding(.vertical, 8)
                .background(.bar)
            }
        }
    }

    private func toggle(_ entry: SequenceEntry) {
        if selected.contains(entry.relativePath) {
            selected.remove(entry.relativePath)
        } else {
            selected.insert(entry.relativePath)
        }
        persistSelection()
    }

    private func selectAll() {
        selected = Set(entries.map(\.relativePath))
        persistSelection()
    }

    private func selectNone() {
        selected.removeAll()
        persistSelection()
    }

    private func persistSelection() {
        BatchRenderSelections.save(selected,
                                    showFolder: viewModel.showFolderPath ?? "")
    }

    // MARK: - Running phase

    private func runningView(currentIndex: Int, total: Int, name: String) -> some View {
        // Aggregate progress across the whole batch: the (currentIndex - 1)
        // sequences already done, plus the in-flight sequence's own
        // fraction. Gives one continuously-advancing 0..1 bar so the user
        // can eyeball "I'm 60% through the batch" at a glance.
        let perSequence = runner?.currentSequenceProgress ?? 0
        let overall = (Double(currentIndex - 1) + perSequence) / Double(total)
        return VStack(spacing: 24) {
            Spacer()
            // Indeterminate spinner: always animates so the sheet can't
            // *look* frozen even when one sequence's render dominates
            // wall time and the textual progress (X of N) is static.
            ProgressView()
                .controlSize(.large)
            VStack(spacing: 8) {
                Text("Rendering \(currentIndex) of \(total)")
                    .font(.title3.weight(.medium))
                Text(name)
                    .font(.body)
                    .foregroundStyle(.secondary)
                    .lineLimit(2)
                    .multilineTextAlignment(.center)
            }
            // Per-sequence bar: moves as the render workers complete
            // frames within the active sequence. On long renders this
            // is the user's reassurance that something is happening.
            VStack(spacing: 6) {
                ProgressView(value: perSequence)
                    .progressViewStyle(.linear)
                    .tint(.accentColor)
                Text("\(Int(perSequence * 100))% of this sequence")
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }
            .frame(maxWidth: 360)
            // Overall bar: combines completed sequences + the active
            // sequence's fraction so one number summarises batch progress.
            VStack(spacing: 6) {
                ProgressView(value: overall)
                    .progressViewStyle(.linear)
                    .tint(.secondary)
                Text("\(Int(overall * 100))% overall")
                    .font(.caption2)
                    .foregroundStyle(.secondary)
            }
            .frame(maxWidth: 360)
            Text("Don't switch apps — backgrounding may pause the render.")
                .font(.caption2)
                .foregroundStyle(.secondary)
            Spacer()
        }
        .padding()
    }

    // MARK: - Done phase

    private func doneView(rendered: Int, failed: Int, cancelled: Bool) -> some View {
        VStack(spacing: 16) {
            Spacer()
            Image(systemName: cancelled
                  ? "exclamationmark.triangle"
                  : (failed > 0 ? "exclamationmark.circle" : "checkmark.circle.fill"))
                .font(.system(size: 56))
                .foregroundStyle(cancelled
                                 ? .orange
                                 : (failed > 0 ? .orange : .green))
            VStack(spacing: 4) {
                Text(cancelled ? "Cancelled" : "Done")
                    .font(.title2.weight(.semibold))
                Text("\(rendered) rendered" + (failed > 0 ? "  •  \(failed) failed" : ""))
                    .foregroundStyle(.secondary)
            }
            Spacer()
            Button {
                runner?.reset()
                dismiss()
            } label: {
                Text("Close")
                    .font(.headline)
                    .frame(maxWidth: .infinity, minHeight: 44)
            }
            .buttonStyle(.borderedProminent)
            .padding(.horizontal)
            .padding(.bottom, 8)
        }
        .padding()
    }
}
