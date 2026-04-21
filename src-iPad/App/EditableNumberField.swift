import SwiftUI

// Reusable inline numeric field used by the inspector slider + spin
// controls. The value is stored as a raw int (optionally divided by
// `divisor` for display — e.g. divisor 100 means the stored value 150
// shows as "1.50"). Users can scrub the slider / stepper OR tap the
// field and type a precise value. The field:
//
//   - shows the formatted current value at rest (`"1.50"` / `"999999"`)
//   - opens a numeric keyboard on tap
//   - commits on return, blur, or keyboard-dismiss
//   - rejects non-numeric input by reverting to the stored value
//   - clamps the committed value to [min, max]
//
// Kept deliberately separate from `TextField(value:format:)` so the
// UX (selection-on-focus, dismiss action, decimal-pad keyboard for
// divisor > 1) is controllable in one place. The `storedInt` input
// is what the settings map actually holds; `commit` delivers back a
// post-clamp integer.
struct EditableNumberField: View {
    let storedInt: Int
    let min: Int
    let max: Int
    let divisor: Int
    let commit: (Int) -> Void

    @State private var draft: String = ""
    @FocusState private var focused: Bool

    private var displayString: String {
        if divisor > 1 {
            let d = Double(storedInt) / Double(divisor)
            return String(format: "%.\(decimalPlaces(divisor))f", d)
        }
        return String(storedInt)
    }

    var body: some View {
        TextField("", text: $draft)
            .multilineTextAlignment(.trailing)
            .monospacedDigit()
            .font(.caption2)
            .foregroundStyle(.secondary)
            .keyboardType(divisor > 1 ? .decimalPad : .numberPad)
            .textFieldStyle(.plain)
            // Tap anywhere on the trailing padding to focus as well —
            // tiny hit target otherwise. 60pt is the visual minimum
            // from the slider/spin rows.
            .frame(minWidth: 60, alignment: .trailing)
            .focused($focused)
            .onAppear { draft = displayString }
            .onChange(of: storedInt) { _, _ in
                // Outside writes (slider scrub, another editor, undo)
                // should repaint the field unless we're actively
                // editing it.
                if !focused { draft = displayString }
            }
            .onChange(of: focused) { _, nowFocused in
                if nowFocused {
                    // Fresh-start draft from the current stored value —
                    // makes typing predictable even if the previous
                    // draft was left mid-edit.
                    draft = displayString
                } else {
                    commitDraft()
                }
            }
            .onSubmit { commitDraft() }
            .toolbar {
                ToolbarItemGroup(placement: .keyboard) {
                    if focused {
                        Spacer()
                        Button("Done") {
                            focused = false
                        }
                    }
                }
            }
    }

    /// Parse the current draft and push it through `commit` after
    /// clamping. Malformed drafts snap back to the last known value.
    private func commitDraft() {
        let trimmed = draft.trimmingCharacters(in: .whitespaces)
        if trimmed.isEmpty {
            draft = displayString
            return
        }
        let parsedInt: Int?
        if divisor > 1 {
            if let d = Double(trimmed) {
                parsedInt = Int((d * Double(divisor)).rounded())
            } else {
                parsedInt = nil
            }
        } else {
            parsedInt = Int(trimmed)
        }
        guard let n = parsedInt else {
            draft = displayString
            return
        }
        let clamped = Swift.min(Swift.max(n, min), max)
        if clamped != storedInt {
            commit(clamped)
        }
        // Re-format the draft so the field shows the canonical value
        // (trailing zeros, clamped range) rather than whatever the
        // user typed.
        draft = divisor > 1
            ? String(format: "%.\(decimalPlaces(divisor))f",
                     Double(clamped) / Double(divisor))
            : String(clamped)
    }

    private func decimalPlaces(_ divisor: Int) -> Int {
        switch divisor {
        case 10:   return 1
        case 100:  return 2
        case 1000: return 3
        default:   return 2
        }
    }
}
