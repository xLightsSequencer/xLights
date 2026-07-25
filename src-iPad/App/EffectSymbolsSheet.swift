import SwiftUI

// Effect Symbols manager — the iPad peer of desktop's `EffectSymbolDialog`
// (Tools ▸ "Effect Symbols") plus Tools ▸ "Convert All Symbols to Effects"
// (`xLightsMain.cpp:6106`).
//
// A symbol is a reusable effect definition stored in the `.xsq`; every effect
// linked to it shares its type, settings and palette, and editing any one of
// them rewrites the symbol and all the others. Creating and linking happen from
// the grid's effect menu — this sheet is for renaming, deleting, and the
// one-shot convert-to-plain-effects escape hatch.
struct EffectSymbolsSheet: View {
    let viewModel: SequencerViewModel
    @Environment(\.dismiss) private var dismiss

    @State private var symbols: [SequencerViewModel.EffectSymbolInfo] = []
    @State private var renameTarget: SequencerViewModel.EffectSymbolInfo?
    @State private var renameText: String = ""
    @State private var pendingDelete: SequencerViewModel.EffectSymbolInfo?
    @State private var confirmConvert = false
    @State private var resultMessage: String?

    var body: some View {
        NavigationStack {
            Group {
                if symbols.isEmpty {
                    ContentUnavailableView(
                        "No Effect Symbols",
                        systemImage: "link",
                        description: Text("Create one from the sequencer: long-press an effect → Effect Symbol → Create Symbol from Effect."))
                } else {
                    List {
                        Section {
                            ForEach(symbols) { sym in
                                VStack(alignment: .leading, spacing: 2) {
                                    Text(sym.name).font(.body)
                                    Text("\(sym.effectType) · linked to \(sym.linkedCount) effect\(sym.linkedCount == 1 ? "" : "s")")
                                        .font(.caption)
                                        .foregroundStyle(.secondary)
                                }
                                .contentShape(Rectangle())
                                .swipeActions(edge: .trailing) {
                                    Button(role: .destructive) {
                                        pendingDelete = sym
                                    } label: { Label("Delete", systemImage: "trash") }
                                    Button {
                                        renameText = sym.name
                                        renameTarget = sym
                                    } label: { Label("Rename", systemImage: "pencil") }
                                    .tint(.blue)
                                }
                            }
                        } footer: {
                            Text("Deleting a symbol unlinks its effects — each keeps the settings it has now.")
                        }

                        Section {
                            Button("Convert All Symbols to Effects…") { confirmConvert = true }
                        } footer: {
                            Text("Unlinks every effect and removes the symbols, for sharing the sequence with older versions of xLights.")
                        }
                    }
                }
            }
            .navigationTitle("Effect Symbols")
            .toolbar {
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") { dismiss() }
                }
            }
        }
        .onAppear { reload() }
        .alert("Rename Symbol", isPresented: Binding(get: { renameTarget != nil },
                                                      set: { if !$0 { renameTarget = nil } })) {
            TextField("Symbol name", text: $renameText)
            Button("Rename") {
                if let t = renameTarget {
                    if !viewModel.renameEffectSymbol(id: t.id, to: renameText) {
                        resultMessage = "That name is empty or already used by another symbol."
                    }
                }
                renameTarget = nil
                reload()
            }
            Button("Cancel", role: .cancel) { renameTarget = nil }
        }
        .confirmationDialog("Delete this symbol?",
                            isPresented: Binding(get: { pendingDelete != nil },
                                                 set: { if !$0 { pendingDelete = nil } }),
                            titleVisibility: .visible) {
            Button("Delete", role: .destructive) {
                if let t = pendingDelete { _ = viewModel.deleteEffectSymbol(id: t.id) }
                pendingDelete = nil
                reload()
            }
            Button("Cancel", role: .cancel) { pendingDelete = nil }
        } message: {
            Text(pendingDelete.map {
                "“\($0.name)” is linked to \($0.linkedCount) effect\($0.linkedCount == 1 ? "" : "s"). They keep their current settings and become ordinary effects."
            } ?? "")
        }
        .confirmationDialog("Convert all symbols to effects?",
                            isPresented: $confirmConvert,
                            titleVisibility: .visible) {
            Button("Convert", role: .destructive) {
                let r = viewModel.convertAllSymbolsToEffects()
                resultMessage = "Converted \(r.converted) effect\(r.converted == 1 ? "" : "s") and removed \(r.symbols) symbol\(r.symbols == 1 ? "" : "s")."
                reload()
            }
            Button("Cancel", role: .cancel) { }
        } message: {
            let linked = symbols.reduce(0) { $0 + $1.linkedCount }
            Text("This unlinks \(linked) effect\(linked == 1 ? "" : "s") and removes \(symbols.count) symbol\(symbols.count == 1 ? "" : "s"). The effects keep their current settings.")
        }
        .alert("Effect Symbols",
               isPresented: Binding(get: { resultMessage != nil },
                                    set: { if !$0 { resultMessage = nil } })) {
            Button("OK", role: .cancel) { resultMessage = nil }
        } message: {
            Text(resultMessage ?? "")
        }
    }

    private func reload() {
        symbols = viewModel.effectSymbols()
    }
}
