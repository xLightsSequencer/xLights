import SwiftUI

/// Shift Effects (Edit ▸ Shift Effects…).
///
/// Mirrors the desktop trio `OnMenuItemShiftEffects(AndTiming)Selected`
/// / `MenuItemShiftSelectedEffects` (`xLightsMain.cpp:5808,5902,5970`):
/// bulk-offset every effect (and optionally every timing mark, or only
/// the selected effects) by a signed millisecond delta. The delta is
/// rounded to the sequence frame interval and effects pushed below 0
/// are truncated / deleted, exactly as the desktop does. Unlike
/// desktop — which warns the op can't be undone — the iPad registers a
/// snapshot-based undo (`SequencerViewModel.shiftEffects`).
struct ShiftEffectsSheet: View {
    @Environment(SequencerViewModel.self) private var viewModel
    @Environment(\.dismiss) private var dismiss

    @State private var offsetMS: Int = 0
    @State private var scope: SequencerViewModel.ShiftScope = .allEffects

    var body: some View {
        NavigationStack {
            Form {
                Section {
                    HStack {
                        TextField("Milliseconds", value: $offsetMS, format: .number)
                            .keyboardType(.numbersAndPunctuation)
                            .textFieldStyle(.roundedBorder)
                        Stepper("",
                                value: $offsetMS,
                                step: viewModel.frameIntervalMS)
                            .labelsHidden()
                    }
                    Text(framesReadout)
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                } header: {
                    Text("Offset")
                } footer: {
                    Text("Positive shifts later, negative earlier. Rounded to the \(viewModel.frameIntervalMS) ms frame interval. Effects pushed before the start are truncated or removed.")
                }

                Section {
                    Picker("Scope", selection: $scope) {
                        Text("All Effects").tag(SequencerViewModel.ShiftScope.allEffects)
                        Text("All Effects And Timing").tag(SequencerViewModel.ShiftScope.allEffectsAndTiming)
                        Text("Selected Effects Only")
                            .tag(SequencerViewModel.ShiftScope.selectedOnly)
                    }
                    .pickerStyle(.inline)
                    .labelsHidden()
                } header: {
                    Text("Apply To")
                } footer: {
                    if !viewModel.canShiftSelectedEffects {
                        Text("Select one or more effects to enable the selected-only scope.")
                    }
                }
            }
            .navigationTitle("Shift Effects")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Apply") {
                        viewModel.shiftEffects(byMS: offsetMS, scope: scope)
                        dismiss()
                    }
                    .disabled(offsetMS == 0)
                }
            }
            .onAppear {
                scope = effectiveInitialScope
            }
        }
    }

    /// Honour the menu-chosen scope, but fall back to All Effects when
    /// selected-only was requested with nothing selected.
    private var effectiveInitialScope: SequencerViewModel.ShiftScope {
        if viewModel.shiftEffectsInitialScope == .selectedOnly
            && !viewModel.canShiftSelectedEffects {
            return .allEffects
        }
        return viewModel.shiftEffectsInitialScope
    }

    private var framesReadout: String {
        let frame = viewModel.frameIntervalMS
        guard frame > 0 else { return "" }
        let frames = offsetMS / frame
        let word = abs(frames) == 1 ? "frame" : "frames"
        return "\(frames) \(word) at \(frame) ms"
    }
}
