import SwiftUI

// Arbitrary per-channel DMX remap grid — the touch analogue of desktop's
// RemapDMXChannelsDialog (src-ui-wx/setup/RemapDMXChannelsDialog.cpp).
// Each of the 48 target channels gets a "source channel" stepper: the
// value written to target channel N becomes the *current* value of the
// chosen source channel (0 = leave that channel off). A live preview
// column shows the resulting value so the user can see the permutation
// before committing. Apply writes the whole mapping through the
// `dmxRemapChannels(forRow:atIndex:mapping:)` bridge (snapshot-then-write
// so the permutation can't step on itself), in one undoable step.
struct DMXRemapGridSheet: View {
    @Environment(SequencerViewModel.self) var viewModel
    @Environment(\.dismiss) private var dismiss

    let rowIndex: Int
    let effectIndex: Int
    let onApplied: () -> Void

    private static let channelCount = 48

    // mapping[i] = source channel for target channel (i+1). Identity by default.
    @State private var mapping: [Int] = Array(1...DMXRemapGridSheet.channelCount)
    @State private var values: [Int] = []  // 1-based, values[0] unused

    private func currentValue(_ channel: Int) -> Int {
        guard channel >= 1, channel < values.count else { return 0 }
        return values[channel]
    }

    private func previewValue(forTarget target: Int) -> Int {
        let src = mapping[target - 1]
        return (src >= 1 && src <= Self.channelCount) ? currentValue(src) : 0
    }

    var body: some View {
        NavigationStack {
            List {
                Section {
                    Text("Each target channel takes the current value of its chosen source channel. Set the source to “Off” to clear a channel.")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
                Section {
                    ForEach(1...Self.channelCount, id: \.self) { target in
                        HStack(spacing: 10) {
                            Text("Ch \(target)")
                                .frame(width: 52, alignment: .leading)
                                .font(.callout.monospacedDigit())
                            Stepper(value: Binding(
                                get: { mapping[target - 1] },
                                set: { mapping[target - 1] = $0 }
                            ), in: 0...Self.channelCount) {
                                Text(mapping[target - 1] == 0
                                     ? "← Off"
                                     : "← Ch \(mapping[target - 1])")
                                    .font(.caption.monospacedDigit())
                            }
                            Spacer()
                            Text("\(previewValue(forTarget: target))")
                                .font(.caption.monospacedDigit())
                                .foregroundStyle(.secondary)
                                .frame(width: 36, alignment: .trailing)
                        }
                    }
                }
            }
            .navigationTitle("Remap DMX Channels")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                }
                ToolbarItemGroup(placement: .topBarTrailing) {
                    Button("Reset") {
                        mapping = Array(1...Self.channelCount)
                    }
                    Button("Apply") { apply() }
                }
            }
        }
        .task { reload() }
    }

    private func reload() {
        let raw = viewModel.document.dmxChannelValues(forRow: Int32(rowIndex),
                                                      atIndex: Int32(effectIndex))
        values = raw.map { $0.intValue }
        if mapping.count != Self.channelCount {
            mapping = Array(1...Self.channelCount)
        }
    }

    private func apply() {
        let nsMapping = mapping.map { NSNumber(value: $0) }
        let ok = viewModel.document.dmxRemapChannels(forRow: Int32(rowIndex),
                                                     atIndex: Int32(effectIndex),
                                                     mapping: nsMapping)
        if ok {
            viewModel.refreshSelectedEffectSettings()
            viewModel.inspectorRevision &+= 1
            onApplied()
        }
        dismiss()
    }
}
