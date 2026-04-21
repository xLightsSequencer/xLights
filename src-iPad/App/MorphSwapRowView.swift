import SwiftUI

// Action button for the Morph effect's `Morph_Swap` custom control.
// Swaps every Start_* / End_* paired setting (and its VALUECURVE sibling)
// so the morph plays in reverse. Matches MorphPanel::OnSwapClick on
// desktop.
//
// Pairs handled:
//   Morph_Start_X1 ↔ Morph_End_X1
//   Morph_Start_Y1 ↔ Morph_End_Y1
//   Morph_Start_X2 ↔ Morph_End_X2
//   Morph_Start_Y2 ↔ Morph_End_Y2
//   Morph_Start_Link ↔ Morph_End_Link   (checkbox pair)
// For each pair we also swap the matching `E_VALUECURVE_<id>` string.
struct MorphSwapRowView: View {
    @Environment(SequencerViewModel.self) var viewModel

    private static let pairs: [(String, String)] = [
        ("Morph_Start_X1", "Morph_End_X1"),
        ("Morph_Start_Y1", "Morph_End_Y1"),
        ("Morph_Start_X2", "Morph_End_X2"),
        ("Morph_Start_Y2", "Morph_End_Y2"),
    ]

    private static let checkboxPairs: [(String, String)] = [
        ("Morph_Start_Link", "Morph_End_Link"),
    ]

    var body: some View {
        Button(action: performSwap) {
            Label("Swap Start / End", systemImage: "arrow.left.arrow.right")
                .font(.caption)
                .frame(maxWidth: .infinity)
        }
        .buttonStyle(.bordered)
        .controlSize(.small)
        .padding(.vertical, 2)
    }

    private func performSwap() {
        for (a, b) in Self.pairs {
            // Slider-backed pair (TEXTCTRL for Morph's float pairs) + VC.
            swap(key("E_TEXTCTRL_", a), with: key("E_TEXTCTRL_", b))
            swap(key("E_SLIDER_", a),   with: key("E_SLIDER_", b))
            swap(key("E_VALUECURVE_", a), with: key("E_VALUECURVE_", b))
        }
        for (a, b) in Self.checkboxPairs {
            swap(key("E_CHECKBOX_", a), with: key("E_CHECKBOX_", b))
        }
    }

    private func key(_ prefix: String, _ id: String) -> String {
        return "\(prefix)\(id)"
    }

    private func swap(_ a: String, with b: String) {
        // Read-read-write-write so an empty vs set asymmetry still
        // round-trips (empty ends up removing the key via the bridge).
        let va = viewModel.settingValue(forKey: a, defaultValue: "")
        let vb = viewModel.settingValue(forKey: b, defaultValue: "")
        if va == vb { return }
        viewModel.setSettingValue(vb, forKey: a)
        viewModel.setSettingValue(va, forKey: b)
    }
}
