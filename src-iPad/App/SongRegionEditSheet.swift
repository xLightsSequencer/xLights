import SwiftUI

/// #6268 Song Structure region editor. Mirrors the desktop
/// SongRegionEditDialog (TimeLine.cpp): a name field plus the 8-swatch
/// palette picker. Edits write straight through the view model (not
/// undoable, matching desktop).
struct SongRegionEditSheet: View {
    let regionID: Int
    @Binding var name: String
    let viewModel: SequencerViewModel
    let onDone: () -> Void

    @State private var selectedColor: UInt32 = 0

    private var region: SequencerViewModel.SongRegion? {
        viewModel.songRegions.first { $0.id == regionID }
    }

    var body: some View {
        NavigationStack {
            Form {
                Section("Name") {
                    TextField("Region name", text: $name)
                        .textInputAutocapitalization(.words)
                }
                Section("Color") {
                    let cols = Array(repeating: GridItem(.flexible()), count: 4)
                    LazyVGrid(columns: cols, spacing: 12) {
                        ForEach(0..<8, id: \.self) { i in
                            let argb = viewModel.songPaletteColor(i)
                            swatch(argb)
                        }
                    }
                    .padding(.vertical, 4)
                }
            }
            .navigationTitle("Edit Region")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { onDone() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") { apply() }
                }
            }
        }
        .onAppear {
            if let r = region {
                if name.isEmpty { name = r.name }
                selectedColor = r.colorARGB
            }
        }
    }

    private func swatch(_ argb: UInt32) -> some View {
        // Compare on RGB only — the stored region keeps its own alpha.
        let isSel = (selectedColor & 0x00FFFFFF) == (argb & 0x00FFFFFF)
        return Circle()
            .fill(Self.color(argb))
            .frame(width: 40, height: 40)
            .overlay(Circle().stroke(.primary, lineWidth: isSel ? 3 : 0))
            .onTapGesture {
                // Preserve the existing alpha, swap RGB.
                let a = selectedColor & 0xFF000000
                selectedColor = (a == 0 ? 0x40000000 : a) | (argb & 0x00FFFFFF)
            }
    }

    private func apply() {
        let trimmed = name.trimmingCharacters(in: .whitespacesAndNewlines)
        viewModel.setSongRegionName(id: regionID, name: trimmed)
        viewModel.setSongRegionColor(id: regionID, colorARGB: selectedColor)
        onDone()
    }

    private static func color(_ argb: UInt32) -> Color {
        Color(.sRGB,
              red: Double((argb >> 16) & 0xFF) / 255.0,
              green: Double((argb >> 8) & 0xFF) / 255.0,
              blue: Double(argb & 0xFF) / 255.0,
              opacity: 1.0)
    }
}
