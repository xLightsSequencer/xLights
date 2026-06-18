import SwiftUI

/// Find Possible Source Effects results (EffectsGrid.cpp:476). Lists
/// the model-level effects rendering onto the picked node at the play
/// head; tapping one jumps the play head there and selects it when its
/// row is visible.
struct SourceEffectsSheet: View {
    @Environment(SequencerViewModel.self) private var viewModel

    private func timeLabel(_ ms: Int) -> String {
        String(format: "%.2fs", Double(ms) / 1000.0)
    }

    var body: some View {
        NavigationStack {
            Group {
                if viewModel.sourceEffectMatches.isEmpty {
                    ContentUnavailableView(
                        "No Source Effects",
                        systemImage: "scope",
                        description: Text("No model effects render onto this node at the play head."))
                } else {
                    List(viewModel.sourceEffectMatches) { match in
                        Button {
                            viewModel.jumpToSourceEffect(match)
                            viewModel.sourceEffectsPresented = false
                        } label: {
                            HStack {
                                VStack(alignment: .leading, spacing: 2) {
                                    Text(match.label)
                                        .font(.callout)
                                        .foregroundStyle(.primary)
                                    Text("\(timeLabel(match.startMS)) – \(timeLabel(match.endMS))")
                                        .font(.caption)
                                        .foregroundStyle(.secondary)
                                }
                                Spacer()
                                if match.visibleRow >= 0 && match.effectIndex >= 0 {
                                    Image(systemName: "arrow.right.circle")
                                        .foregroundStyle(.tint)
                                }
                            }
                        }
                    }
                }
            }
            .navigationTitle("Possible Source Effects")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") { viewModel.sourceEffectsPresented = false }
                }
            }
        }
    }
}
