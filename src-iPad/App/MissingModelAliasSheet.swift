import SwiftUI

/// Pending "missing model" reconciliation prompt surfaced after
/// `openSequence` returns. `originalNames` is the list of model
/// element names from the freshly-loaded sequence whose models
/// aren't in the show's `ModelManager` AND that carry at least
/// one effect (silent drop for the no-effects case mirrors
/// desktop's `CheckForValidModels`).
struct MissingModelPrompt: Identifiable {
    let id = UUID()
    let originalNames: [String]
}

/// Per-row decision for `MissingModelAliasSheet`. `rename` carries
/// the target model name; `delete` discards the orphan element.
private enum MissingModelAction: Equatable, Hashable {
    case rename(target: String)
    case delete
}

private struct MissingModelRowState: Identifiable {
    let id: String
    let originalName: String
    var action: MissingModelAction
    var addAlias: Bool
}

struct MissingModelAliasSheet: View {
    let prompt: MissingModelPrompt
    let availableModels: [String]
    /// Decisions keyed by original missing-name. Caller applies them
    /// in one pass when the sheet is dismissed.
    let onCommit: ([(originalName: String, action: MissingModelDecision)]) -> Void
    let onCancel: () -> Void

    @Environment(\.dismiss) private var dismiss
    @State private var rows: [MissingModelRowState] = []

    var body: some View {
        NavigationStack {
            Group {
                if rows.isEmpty {
                    ContentUnavailableView("No missing models",
                                           systemImage: "checkmark.seal",
                                           description: Text("Nothing to reconcile."))
                } else {
                    List {
                        Section {
                            Text("Some effects in this sequence reference models that aren't in your show layout. Pick what to do with each.")
                                .font(.callout)
                                .foregroundStyle(.secondary)
                        }
                        ForEach($rows) { $row in
                            rowView(for: $row)
                        }
                    }
                }
            }
            .navigationTitle("Missing Models")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") {
                        onCancel()
                        dismiss()
                    }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") {
                        commit()
                        dismiss()
                    }
                    .disabled(rows.isEmpty)
                }
            }
        }
        .onAppear { setupRowsIfNeeded() }
    }

    @ViewBuilder
    private func rowView(for row: Binding<MissingModelRowState>) -> some View {
        VStack(alignment: .leading, spacing: 8) {
            Text(row.wrappedValue.originalName)
                .font(.headline)
            Picker("Action", selection: actionBinding(for: row)) {
                if !availableModels.isEmpty {
                    ForEach(availableModels, id: \.self) { name in
                        Text("Rename to \(name)").tag(MissingModelAction.rename(target: name))
                    }
                }
                Text("Delete element").tag(MissingModelAction.delete)
            }
            .pickerStyle(.menu)
            if case .rename = row.wrappedValue.action {
                Toggle("Add alias (auto-remap future sequences)",
                       isOn: row.addAlias)
                    .font(.subheadline)
            }
        }
        .padding(.vertical, 4)
    }

    private func actionBinding(for row: Binding<MissingModelRowState>) -> Binding<MissingModelAction> {
        Binding(
            get: { row.wrappedValue.action },
            set: { row.wrappedValue.action = $0 }
        )
    }

    private func setupRowsIfNeeded() {
        guard rows.isEmpty else { return }
        let defaultAction: MissingModelAction =
            availableModels.first.map { .rename(target: $0) } ?? .delete
        rows = prompt.originalNames.map { name in
            MissingModelRowState(id: name,
                                 originalName: name,
                                 action: defaultAction,
                                 addAlias: true)
        }
    }

    private func commit() {
        let decisions: [(originalName: String, action: MissingModelDecision)] = rows.map { row in
            switch row.action {
            case .rename(let target):
                return (row.originalName, .rename(target: target, addAlias: row.addAlias))
            case .delete:
                return (row.originalName, .delete)
            }
        }
        onCommit(decisions)
    }
}

/// Caller-facing decision shape (`MissingModelAction` is private to
/// the sheet because it's bound to the Picker tag). Lets
/// `SequencerViewModel` apply each row through the bridge without
/// re-deriving the sub-cases.
enum MissingModelDecision {
    case rename(target: String, addAlias: Bool)
    case delete
}
