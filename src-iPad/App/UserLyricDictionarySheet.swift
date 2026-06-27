import SwiftUI

// Tools → User Lyric Dictionary (desktop Tools → User Lyric
// Dictionary / LyricUserDictDialog). Edits the show folder's
// `user_dictionary` — the highest-precedence word→phoneme file the
// core PhonemeDictionary consults during lyric breakdown. Saving
// rewrites the file (desktop line format) and refreshes the live
// dictionary via the bridge.
struct UserLyricDictionarySheet: View {
    @Environment(\.dismiss) private var dismiss
    @Environment(SequencerViewModel.self) private var viewModel

    struct Entry: Identifiable {
        let id = UUID()
        var word: String
        var phonemes: String
    }

    @State private var entries: [Entry] = []
    @State private var editing: Entry?
    @State private var dirty = false

    var body: some View {
        NavigationStack {
            Group {
                if entries.isEmpty {
                    ContentUnavailableView {
                        Label("No Dictionary Entries", systemImage: "character.book.closed")
                    } description: {
                        Text("Add words and their phoneme breakdowns to override the built-in lyric dictionary for this show.")
                    } actions: {
                        Button("Add Word…") { editing = Entry(word: "", phonemes: "") }
                            .buttonStyle(.borderedProminent)
                    }
                } else {
                    List {
                        ForEach(entries) { e in
                            Button { editing = e } label: {
                                HStack {
                                    Text(e.word).font(.callout.weight(.semibold))
                                    Spacer()
                                    Text(e.phonemes)
                                        .font(.caption.monospaced())
                                        .foregroundStyle(.secondary)
                                        .lineLimit(1)
                                        .truncationMode(.tail)
                                }
                            }
                            .buttonStyle(.plain)
                        }
                        .onDelete { idx in
                            entries.remove(atOffsets: idx)
                            dirty = true
                        }
                    }
                }
            }
            .navigationTitle("User Lyric Dictionary")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                }
                ToolbarItem(placement: .primaryAction) {
                    Button {
                        editing = Entry(word: "", phonemes: "")
                    } label: {
                        Image(systemName: "plus")
                    }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Save") { save() }
                        .disabled(!dirty)
                }
            }
            .sheet(item: $editing) { e in
                UserLyricEntryEditor(entry: e) { updated in
                    apply(updated)
                }
            }
        }
        .onAppear(perform: load)
    }

    private func load() {
        entries = viewModel.document.userLyricDictionaryEntries().map {
            Entry(word: $0["word"] ?? "", phonemes: $0["phonemes"] ?? "")
        }
    }

    private func apply(_ updated: Entry) {
        if let i = entries.firstIndex(where: { $0.id == updated.id }) {
            entries[i] = updated
        } else {
            entries.append(updated)
        }
        entries.sort { $0.word.localizedCaseInsensitiveCompare($1.word) == .orderedAscending }
        dirty = true
    }

    private func save() {
        let payload: [[String: String]] = entries
            .filter { !$0.word.trimmingCharacters(in: .whitespaces).isEmpty }
            .map { ["word": $0.word, "phonemes": $0.phonemes] }
        _ = viewModel.document.saveUserLyricDictionaryEntries(payload)
        dismiss()
    }
}

private struct UserLyricEntryEditor: View {
    @Environment(\.dismiss) private var dismiss
    @State var entry: UserLyricDictionarySheet.Entry
    let onCommit: (UserLyricDictionarySheet.Entry) -> Void

    var body: some View {
        NavigationStack {
            Form {
                Section("Word") {
                    TextField("Word", text: $entry.word)
                        .textInputAutocapitalization(.characters)
                        .autocorrectionDisabled()
                }
                Section {
                    TextField("AI E1 …", text: $entry.phonemes)
                        .textInputAutocapitalization(.characters)
                        .autocorrectionDisabled()
                } header: {
                    Text("Phonemes")
                } footer: {
                    Text("Space-separated phoneme list (e.g. AI E1 etc). These map to the mouth shapes used by face effects.")
                }
            }
            .navigationTitle(entry.word.isEmpty ? "Add Word" : "Edit Word")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") {
                        onCommit(entry)
                        dismiss()
                    }
                    .disabled(entry.word.trimmingCharacters(in: .whitespaces).isEmpty)
                }
            }
        }
    }
}
