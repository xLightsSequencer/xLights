import SwiftUI

// "Convert To…" — retype the selected effect(s). Desktop reaches the same
// operation from the Effects panel's right-click, which puts up a
// `wxSingleChoiceDialog` over the effect list (`EffectsPanel.cpp:348`)
// and then calls `MainSequencer::ConvertSelectedEffectsTo`. A flat
// 57-item menu is unusable on touch, so iPad uses a searchable list
// instead — same choice set, same operation.
struct ConvertEffectTypeSheet: View {
    @Environment(\.dismiss) private var dismiss
    let viewModel: SequencerViewModel

    @State private var search: String = ""

    /// The type every selected effect currently is, or nil when the
    /// selection is mixed — used to mark the current row and to keep the
    /// user from "converting" to what they already have.
    private var currentType: String? {
        let names = Set(targets.map {
            viewModel.document.effectName(forRow: Int32($0.rowIndex),
                                           at: Int32($0.effectIndex))
        })
        return names.count == 1 ? names.first : nil
    }

    private var targets: [SequencerViewModel.EffectSelection] {
        if viewModel.selectedEffects.count > 1 {
            return Array(viewModel.selectedEffects)
        }
        if let one = viewModel.selectedEffect { return [one] }
        return []
    }

    private var matches: [String] {
        let all = viewModel.availableEffects
        let q = search.trimmingCharacters(in: .whitespaces)
        if q.isEmpty { return all }
        return all.filter { $0.localizedCaseInsensitiveContains(q) }
    }

    var body: some View {
        NavigationStack {
            List {
                Section {
                    ForEach(matches, id: \.self) { name in
                        Button {
                            viewModel.convertSelectedEffects(to: name)
                            dismiss()
                        } label: {
                            HStack {
                                Text(name)
                                    .foregroundStyle(.primary)
                                Spacer()
                                if name == currentType {
                                    Image(systemName: "checkmark")
                                        .foregroundStyle(.tint)
                                }
                            }
                        }
                        .disabled(name == currentType)
                    }
                } footer: {
                    Text("Keeps timing, the colour palette and the Buffer / Blending panels. The old effect's own settings are dropped — the converted effect starts at the new type's defaults, exactly as if you had just dropped it.")
                }
            }
            .searchable(text: $search, prompt: "Effect type")
            .navigationTitle(targets.count > 1
                              ? "Convert \(targets.count) Effects"
                              : "Convert Effect")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                }
            }
        }
    }
}
