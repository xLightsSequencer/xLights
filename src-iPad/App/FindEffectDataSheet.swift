import SwiftUI

// Find Effect Data (desktop View ▸ Windows ▸ Find Effect Data —
// FindDataPanel + SearchPanel). Queries the loaded sequence for
// effects by type / settings-text / model filter and lets the user
// tap a result to jump the grid + playhead to it (reusing
// `SequencerViewModel.jumpToEffect`, the same path CheckSequenceSheet
// uses).
struct FindEffectDataSheet: View {
    @Environment(\.dismiss) private var dismiss
    @Environment(SequencerViewModel.self) private var viewModel

    private static let anyType = "Any Effect"

    @State private var effectType: String = anyType
    @State private var settingsText: String = ""
    @State private var modelFilter: String = ""
    @State private var availableTypes: [String] = []
    @State private var results: [XLFindEffectResult] = []
    @State private var didSearch = false
    @State private var searching = false

    private let maxResults = 1000

    var body: some View {
        NavigationStack {
            VStack(spacing: 0) {
                queryControls
                Divider()
                resultsList
            }
            .navigationTitle("Find Effect Data")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Done") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Find") { runSearch() }
                }
            }
        }
        .onAppear {
            availableTypes = [Self.anyType] + viewModel.document.effectTypesInSequence()
        }
    }

    @ViewBuilder private var queryControls: some View {
        Form {
            Section("Query") {
                Picker("Effect Type", selection: $effectType) {
                    ForEach(availableTypes, id: \.self) { Text($0).tag($0) }
                }
                TextField("Setting contains…", text: $settingsText)
                    .textInputAutocapitalization(.never)
                    .autocorrectionDisabled()
                    .submitLabel(.search)
                    .onSubmit { runSearch() }
                TextField("Model name contains…", text: $modelFilter)
                    .textInputAutocapitalization(.never)
                    .autocorrectionDisabled()
                    .submitLabel(.search)
                    .onSubmit { runSearch() }
            }
        }
        .frame(maxHeight: 240)
        .scrollDisabled(true)
    }

    @ViewBuilder private var resultsList: some View {
        if searching {
            ProgressView().frame(maxWidth: .infinity, maxHeight: .infinity)
        } else if !didSearch {
            ContentUnavailableView(
                "Find Effect Data",
                systemImage: "magnifyingglass",
                description: Text("Choose an effect type, type a setting or model substring, then tap Find.")
            )
        } else if results.isEmpty {
            ContentUnavailableView(
                "No Matches",
                systemImage: "magnifyingglass",
                description: Text("No effects in this sequence match the query.")
            )
        } else {
            List {
                Section {
                    ForEach(Array(results.enumerated()), id: \.offset) { _, r in
                        Button { jump(to: r) } label: { resultRow(r) }
                            .buttonStyle(.plain)
                    }
                } header: {
                    Text(results.count >= maxResults
                         ? "First \(maxResults) matches"
                         : "\(results.count) match\(results.count == 1 ? "" : "es")")
                }
            }
        }
    }

    @ViewBuilder private func resultRow(_ r: XLFindEffectResult) -> some View {
        VStack(alignment: .leading, spacing: 2) {
            HStack {
                Text(r.effectName).font(.callout.weight(.semibold))
                Spacer()
                Text(timeLabel(Int(r.startTimeMS)))
                    .font(.caption.monospacedDigit())
                    .foregroundStyle(.secondary)
            }
            Text(elementLabel(r))
                .font(.caption)
                .foregroundStyle(.secondary)
                .lineLimit(1)
                .truncationMode(.middle)
            if !r.matchedSetting.isEmpty {
                Text(r.matchedSetting)
                    .font(.caption2)
                    .foregroundStyle(.blue)
                    .lineLimit(1)
                    .truncationMode(.middle)
            }
        }
        .contentShape(Rectangle())
    }

    private func elementLabel(_ r: XLFindEffectResult) -> String {
        var s = r.elementName.isEmpty ? r.modelName : r.elementName
        if r.elementName != r.modelName && !r.modelName.isEmpty {
            s = "\(r.modelName) ▸ \(r.elementName)"
        }
        return "\(s) · Layer \(r.layerIndex + 1)"
    }

    private func timeLabel(_ ms: Int) -> String {
        String(format: "%.2fs", Double(ms) / 1000.0)
    }

    private func runSearch() {
        searching = true
        let type = effectType == Self.anyType ? "" : effectType
        let setText = settingsText
        let model = modelFilter
        let doc = viewModel.document
        let cap = maxResults
        Task { @MainActor in
            let hits = await Task.detached(priority: .userInitiated) {
                doc.findEffectsMatching(type: type,
                                        settingsText: setText,
                                        modelFilter: model,
                                        maxResults: cap)
            }.value
            results = hits
            didSearch = true
            searching = false
        }
    }

    private func jump(to r: XLFindEffectResult) {
        viewModel.jumpToEffect(modelName: r.modelName,
                               effectName: r.effectName,
                               startTimeMS: Int(r.startTimeMS),
                               layerIndex: Int(r.layerIndex))
        dismiss()
    }
}
