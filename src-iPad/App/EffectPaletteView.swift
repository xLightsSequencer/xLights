import SwiftUI

struct EffectPaletteView: View {
    @Environment(SequencerViewModel.self) var viewModel

    var body: some View {
        ScrollView(.horizontal, showsIndicators: false) {
            HStack(spacing: 8) {
                // B17 — random-pick button. Arms the palette with a
                // random available effect so tap-to-place adds a
                // surprise. Cheap discoverability affordance; desktop
                // equivalent sits on the top toolbar.
                Button {
                    if let random = viewModel.availableEffects.randomElement() {
                        viewModel.selectPaletteEffect(random)
                    }
                } label: {
                    Image(systemName: "dice")
                        .font(.caption2)
                        .padding(.horizontal, 8)
                        .padding(.vertical, 4)
                        .background(Color.gray.opacity(0.2))
                        .clipShape(RoundedRectangle(cornerRadius: 4))
                }
                .buttonStyle(.plain)
                .hoverEffect(.highlight)
                .accessibilityLabel("Random Effect")
                .disabled(viewModel.availableEffects.isEmpty)

                ForEach(viewModel.availableEffects, id: \.self) { name in
                    Button(action: { viewModel.selectPaletteEffect(name) }) {
                        Text(name)
                            .font(.caption2)
                            .padding(.horizontal, 8)
                            .padding(.vertical, 4)
                            .background(
                                viewModel.selectedPaletteEffect == name
                                    ? Color.accentColor.opacity(0.3)
                                    : Color.gray.opacity(0.2)
                            )
                            .clipShape(RoundedRectangle(cornerRadius: 4))
                    }
                    .buttonStyle(.plain)
                }
            }
            .padding(.horizontal)
        }
        .frame(height: 36)
        .background(.bar)
    }
}
