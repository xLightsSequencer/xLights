import SwiftUI

/// B97 Find / Replace sheet. Searches timing-mark labels (matches
/// desktop's intentional restriction in `EffectsGrid::Find`) and
/// supports navigation + single-mark / replace-all rewrites.
struct FindReplaceSheet: View {
    @Environment(SequencerViewModel.self) var viewModel
    @Binding var replaceText: String
    let onDone: () -> Void

    @FocusState private var findFieldFocused: Bool

    var body: some View {
        @Bindable var vm = viewModel

        NavigationStack {
            Form {
                Section {
                    TextField("Find", text: $vm.findText)
                        .textInputAutocapitalization(.never)
                        .autocorrectionDisabled()
                        .focused($findFieldFocused)
                        .onSubmit { viewModel.findNext() }
                        .onChange(of: vm.findText) { _, _ in
                            viewModel.runFind()
                        }
                    Toggle("Case Sensitive", isOn: Binding(
                        get: { vm.findCaseSensitive },
                        set: { vm.findCaseSensitive = $0; viewModel.runFind() }
                    ))
                } footer: {
                    if vm.findText.isEmpty {
                        Text("Searches timing-mark labels (phrases, words, phonemes, custom labels).")
                    } else if vm.findResults.isEmpty {
                        Text("No matches.")
                    } else if vm.currentFindIndex < 0 {
                        Text("\(vm.findResults.count) match\(vm.findResults.count == 1 ? "" : "es"). Tap Next to navigate.")
                    } else {
                        Text("Match \(vm.currentFindIndex + 1) of \(vm.findResults.count).")
                    }
                }

                Section {
                    HStack {
                        Button {
                            viewModel.findPrevious()
                        } label: {
                            Label("Previous", systemImage: "chevron.up")
                        }
                        .disabled(vm.findResults.isEmpty)
                        Spacer()
                        Button {
                            viewModel.findNext()
                        } label: {
                            Label("Next", systemImage: "chevron.down")
                        }
                        .disabled(vm.findResults.isEmpty)
                    }
                }

                Section("Replace") {
                    TextField("Replacement", text: $replaceText)
                        .textInputAutocapitalization(.never)
                        .autocorrectionDisabled()
                    HStack {
                        Button {
                            _ = viewModel.replaceCurrentFindMatch(with: replaceText)
                        } label: {
                            Label("Replace Current", systemImage: "arrow.right.square")
                        }
                        .disabled(vm.currentFindIndex < 0)
                        Spacer()
                        Button {
                            _ = viewModel.replaceAllFindMatches(with: replaceText)
                        } label: {
                            Label("Replace All", systemImage: "arrow.right.to.line")
                        }
                        .disabled(vm.findResults.isEmpty)
                    }
                }
            }
            .navigationTitle("Find / Replace")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") { onDone() }
                }
            }
            .onAppear {
                viewModel.runFind()
                findFieldFocused = true
            }
        }
    }
}
