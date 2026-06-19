import SwiftUI

/// J-4 (import) — pre-placement picker for multi-model `.xmodel`
/// imports. Lets the user choose which layout group the batch of
/// imported models will land in; single-model `.xmodel` imports
/// skip this sheet entirely and inherit the active layout group
/// as before.
struct LayoutGroupPickerSheet: View {
    let groups: [String]
    let initialSelection: String
    let fileName: String
    let onConfirm: (String) -> Void
    let onCancel: () -> Void

    @Environment(\.dismiss) private var dismiss
    @State private var selection: String

    init(groups: [String],
         initialSelection: String,
         fileName: String,
         onConfirm: @escaping (String) -> Void,
         onCancel: @escaping () -> Void) {
        self.groups = groups
        self.initialSelection = initialSelection
        self.fileName = fileName
        self.onConfirm = onConfirm
        self.onCancel = onCancel
        _selection = State(initialValue: groups.contains(initialSelection)
                           ? initialSelection
                           : (groups.first ?? "Default"))
    }

    var body: some View {
        NavigationStack {
            List {
                Section {
                    Text("\"\(fileName)\" contains multiple models. Pick the layout group they should land in.")
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                }
                Section("Destination Layout Group") {
                    ForEach(groups, id: \.self) { name in
                        Button {
                            selection = name
                        } label: {
                            HStack {
                                Text(name)
                                    .foregroundStyle(.primary)
                                Spacer()
                                if name == selection {
                                    Image(systemName: "checkmark")
                                        .foregroundStyle(.tint)
                                }
                            }
                        }
                    }
                }
            }
            .navigationTitle("Import Models")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { onCancel() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Import") { onConfirm(selection) }
                        .disabled(selection.isEmpty)
                }
            }
        }
        .presentationDetents([.medium, .large])
    }
}
