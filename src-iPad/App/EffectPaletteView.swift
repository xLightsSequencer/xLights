import SwiftUI

struct EffectPaletteView: View {
    @Environment(SequencerViewModel.self) var viewModel
    @Environment(\.displayScale) private var displayScale

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
                        HStack(spacing: 4) {
                            // VIEW-1 — show the effect's icon (same BGRA bitmap
                            // the Metal grid uses) ahead of the label so the
                            // dropper reads visually, like the desktop toolbar.
                            if let icon = iconImage(for: name) {
                                Image(icon, scale: displayScale,
                                      label: Text(name))
                                    .resizable()
                                    .interpolation(.high)
                                    .frame(width: 18, height: 18)
                            }
                            Text(name)
                                .font(.caption2)
                        }
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
                    .accessibilityLabel(name)
                }
            }
            .padding(.horizontal)
        }
        .frame(height: 36)
        .background(.bar)
    }

    /// VIEW-1 — fetch the cached effect icon as a `CGImage` (nil if the
    /// effect has no compiled-in icon data).
    private func iconImage(for name: String) -> CGImage? {
        let bucket = EffectIconCache.bucket(forDesiredPx: 18 * displayScale)
        return EffectIconCache.shared.image(for: name,
                                             bucket: bucket,
                                             document: viewModel.document)
    }
}
