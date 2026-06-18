import SwiftUI

/// SEQ-2 — Color Replace (Edit ▸ Color Replace).
///
/// Mirrors the desktop `ColourReplaceDialog`: pick a colour currently
/// used, choose a replacement, and rewrite it across effect palettes.
/// Scope is the whole sequence, or — when effects are selected — the
/// "Selected effects only" toggle limits it to the selection. (The iPad
/// keeps selection in Swift, so the bridge mirrors it into the core
/// effect-`Selected` flags on-demand just for the op; see
/// `XLSequenceDocument.replaceColour(from:to:atRows:effectIndices:)`.)
/// The replace is undoable (palette-snapshot Foundation undo) and, like
/// the desktop, does not force a re-render — the user re-renders after.
struct ColorReplaceSheet: View {
    @Environment(SequencerViewModel.self) private var viewModel
    @Environment(\.dismiss) private var dismiss

    @State private var used: [String] = []
    @State private var fromColour: String = ""
    @State private var toColor: Color = .red
    @State private var resultMessage: String?
    @State private var selectedOnly = false   // scope: selection vs whole sequence

    var body: some View {
        NavigationStack {
            Form {
                Section {
                    if used.isEmpty {
                        Text("No colours are currently used in this sequence.")
                            .foregroundStyle(.secondary)
                    } else {
                        Picker("Existing colour", selection: $fromColour) {
                            ForEach(used, id: \.self) { c in
                                HStack(spacing: 8) {
                                    swatch(c)
                                    Text(c).font(.system(.body, design: .monospaced))
                                }
                                .tag(c)
                            }
                        }
                        ColorPicker("Replace with", selection: $toColor, supportsOpacity: false)
                    }
                    if viewModel.hasEffectSelection {
                        Toggle("Selected effects only", isOn: $selectedOnly)
                            .onChange(of: selectedOnly) { _, _ in refreshUsed() }
                    }
                } header: {
                    Text("Replace")
                } footer: {
                    Text(selectedOnly
                         ? "Rewrites the chosen colour in the selected effects' palettes only. You'll need to re-render afterward. This can be undone."
                         : "Rewrites the chosen colour everywhere it appears in effect palettes across the whole sequence. You'll need to re-render afterward. This can be undone.")
                }

                if let resultMessage {
                    Section { Text(resultMessage).foregroundStyle(.secondary) }
                }
            }
            .navigationTitle("Color Replace")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .topBarLeading) {
                    Button("Close") { dismiss() }
                }
                ToolbarItem(placement: .topBarTrailing) {
                    Button("Replace") { doReplace() }
                        .disabled(fromColour.isEmpty)
                }
            }
            .onAppear(perform: refreshUsed)
        }
    }

    @ViewBuilder
    private func swatch(_ hex: String) -> some View {
        RoundedRectangle(cornerRadius: 3)
            .fill(colorFromHex(hex) ?? .gray)
            .frame(width: 20, height: 20)
            .overlay(RoundedRectangle(cornerRadius: 3).stroke(.secondary.opacity(0.5)))
    }

    private func refreshUsed() {
        used = viewModel.usedColours(selectedOnly: selectedOnly)
        if fromColour.isEmpty || !used.contains(fromColour) {
            fromColour = used.first ?? ""
        }
    }

    private func doReplace() {
        guard let toHex = hexFromColor(toColor), !fromColour.isEmpty else { return }
        let n = viewModel.replaceColour(from: fromColour, to: toHex, selectedOnly: selectedOnly)
        resultMessage = "\(n) effect\(n == 1 ? "" : "s") updated."
        refreshUsed()
    }
}
