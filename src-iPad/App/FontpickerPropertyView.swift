import SwiftUI
import UIKit

// Font-picker row for JSON properties with `controlType: "fontpicker"`
// (Text_Font on the Text effect). Stored value uses wx's `NativeFontInfo`
// user-desc string — the same format desktop writes, parsed on iPad by
// `ParseFontString` (src-core, tolerant of token order per Phase A-2).
// Example: `"bold italic Arial 26 utf-8"`.
//
// We expose only the fields that round-trip cleanly: family, size,
// bold, italic. Charset is always "utf-8" so CoreText can find the face.
struct FontpickerPropertyView: View {
    let property: PropertyMetadata
    let currentDesc: String
    let onChange: (String) -> Void

    @State private var showingSheet = false

    var body: some View {
        VStack(alignment: .leading, spacing: 2) {
            Text(property.label)
                .font(.caption)
            Button(action: { showingSheet = true }) {
                HStack {
                    Text(summary())
                        .font(.caption2)
                        .foregroundStyle(.secondary)
                        .lineLimit(1)
                        .truncationMode(.middle)
                    Spacer()
                    Image(systemName: "chevron.right")
                        .font(.caption2)
                        .foregroundStyle(.tertiary)
                }
                .padding(.horizontal, 8)
                .padding(.vertical, 6)
                .background(
                    RoundedRectangle(cornerRadius: 6)
                        .fill(Color.secondary.opacity(0.08))
                )
            }
            .buttonStyle(.plain)
        }
        .padding(.vertical, 2)
        .sheet(isPresented: $showingSheet) {
            FontPickerSheet(initialDesc: currentDesc) { desc in
                onChange(desc)
            }
        }
    }

    private func summary() -> String {
        if currentDesc.isEmpty { return "(system default)" }
        let p = parseWxFontDesc(currentDesc)
        var parts: [String] = []
        if p.bold { parts.append("Bold") }
        if p.italic { parts.append("Italic") }
        parts.append(p.family.isEmpty ? "System" : p.family)
        parts.append("\(p.size) pt")
        return parts.joined(separator: " ")
    }
}

/// Parsed representation of a wx font user-desc. Matches the relaxed
/// parsing in `src-core/render/TextDrawingContext::ParseFontString` —
/// each token is classified independently so desktop-authored strings
/// with varying token order round-trip.
struct WxFontDesc {
    var family: String = ""
    var size: Int = 12
    var bold: Bool = false
    var italic: Bool = false
}

func parseWxFontDesc(_ s: String) -> WxFontDesc {
    var out = WxFontDesc()
    let tokens = s.split(separator: " ").map(String.init)
    var familyTokens: [String] = []
    for tok in tokens {
        let lower = tok.lowercased()
        if lower == "bold" { out.bold = true; continue }
        if lower == "italic" || lower == "oblique" { out.italic = true; continue }
        if lower == "regular" || lower == "normal" { continue }
        if lower == "utf-8" || lower.hasPrefix("iso-") { continue }
        if let n = Int(tok), n > 3 && n < 500 { out.size = n; continue }
        familyTokens.append(tok)
    }
    out.family = familyTokens.joined(separator: " ")
    return out
}

func formatWxFontDesc(_ d: WxFontDesc) -> String {
    var parts: [String] = []
    if d.bold { parts.append("bold") }
    if d.italic { parts.append("italic") }
    if !d.family.isEmpty { parts.append(d.family) }
    parts.append(String(d.size))
    parts.append("utf-8")
    return parts.joined(separator: " ")
}

/// Sheet — family list, size stepper, bold/italic toggles. Writes the
/// wx user-desc string when the user confirms.
private struct FontPickerSheet: View {
    @Environment(\.dismiss) private var dismiss
    let initialDesc: String
    let onCommit: (String) -> Void

    @State private var family: String
    @State private var size: Int
    @State private var bold: Bool
    @State private var italic: Bool

    private static let families: [String] = UIFont.familyNames.sorted()

    // Seed @State from initialDesc here rather than in `.onAppear`. When a
    // NavigationLink-pushed child (e.g. the family-picker list) pops back,
    // SwiftUI re-fires `.onAppear` on the form that hosted it — which
    // resets `family` to the original value, throwing away the user's
    // selection. Doing it in init runs exactly once per sheet instance.
    init(initialDesc: String, onCommit: @escaping (String) -> Void) {
        self.initialDesc = initialDesc
        self.onCommit = onCommit
        let p = parseWxFontDesc(initialDesc)
        _family = State(initialValue: p.family)
        _size = State(initialValue: p.size)
        _bold = State(initialValue: p.bold)
        _italic = State(initialValue: p.italic)
    }

    var body: some View {
        NavigationStack {
            Form {
                Section("Family") {
                    // Tapping pushes a searchable list so the user can
                    // scroll the 80+ system families without fighting a
                    // huge menu. `.pickerStyle(.navigationLink)` is NOT
                    // a real SwiftUI PickerStyle — it silently falls
                    // back to a layout that hides the current selection,
                    // so we use an explicit NavigationLink instead.
                    NavigationLink {
                        FontFamilyList(families: Self.families,
                                        selection: $family)
                    } label: {
                        HStack {
                            Text("Family")
                            Spacer()
                            Text(family.isEmpty ? "System" : family)
                                .foregroundStyle(.secondary)
                        }
                    }
                }

                Section("Size") {
                    Stepper(value: $size, in: 6...200) {
                        HStack {
                            Text("Size")
                            Spacer()
                            Text("\(size) pt").monospacedDigit()
                        }
                    }
                }

                Section("Style") {
                    Toggle("Bold", isOn: $bold).toggleStyle(.switch)
                    Toggle("Italic", isOn: $italic).toggleStyle(.switch)
                }

                Section("Preview") {
                    Text("The quick brown fox")
                        .font(previewFont())
                        .frame(maxWidth: .infinity, alignment: .center)
                        .padding(.vertical, 8)
                }
            }
            .navigationTitle("Font")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") {
                        onCommit(formatWxFontDesc(
                            WxFontDesc(family: family, size: size,
                                        bold: bold, italic: italic)))
                        dismiss()
                    }
                }
            }
        }
    }

    /// SwiftUI's `Font.custom(_, size:).bold().italic()` applies hint
    /// modifiers, not real trait overrides — if the family doesn't have
    /// a `-Bold` / `-Italic` variant registered you get the regular
    /// face back. `UIFontDescriptor.withSymbolicTraits` asks the font
    /// system for the proper variant (or synthesises a slant / weight
    /// when the face doesn't ship one), which is what the Preview
    /// section needs to actually show a change when the toggles flip.
    private func previewFont() -> Font {
        let base: UIFont
        if family.isEmpty {
            base = UIFont.systemFont(ofSize: CGFloat(size))
        } else {
            base = UIFont(name: family, size: CGFloat(size))
                ?? UIFont.systemFont(ofSize: CGFloat(size))
        }
        var traits: UIFontDescriptor.SymbolicTraits = []
        if bold { traits.insert(.traitBold) }
        if italic { traits.insert(.traitItalic) }
        if !traits.isEmpty,
           let desc = base.fontDescriptor.withSymbolicTraits(traits) {
            return Font(UIFont(descriptor: desc, size: CGFloat(size)))
        }
        return Font(base)
    }
}

/// Searchable push-list for font families. Selecting a row writes back
/// to the parent's binding and pops automatically.
private struct FontFamilyList: View {
    let families: [String]
    @Binding var selection: String
    @Environment(\.dismiss) private var dismiss

    @State private var query: String = ""

    private var filtered: [String] {
        if query.isEmpty { return families }
        return families.filter { $0.localizedCaseInsensitiveContains(query) }
    }

    var body: some View {
        // Swift 6 sometimes picks the Binding-based `ForEach` overload when
        // the closure mutates an external @Binding inside a `List`; snap
        // the filtered array to a local + explicit closure parameter type
        // to pin the plain RandomAccessCollection overload.
        let items: [String] = filtered
        return List {
            // "System" (empty string) option at the top so the user can
            // opt into the OS default.
            Button(action: { selection = ""; dismiss() }) {
                HStack {
                    Text("System")
                    Spacer()
                    if selection.isEmpty {
                        Image(systemName: "checkmark")
                            .foregroundStyle(Color.accentColor)
                    }
                }
            }
            .buttonStyle(.plain)

            ForEach(items, id: \.self) { (f: String) in
                Button(action: { selection = f; dismiss() }) {
                    HStack {
                        Text(f)
                        Spacer()
                        if selection == f {
                            Image(systemName: "checkmark")
                                .foregroundStyle(Color.accentColor)
                        }
                    }
                }
                .buttonStyle(.plain)
            }
        }
        .navigationTitle("Family")
        .navigationBarTitleDisplayMode(.inline)
        .searchable(text: $query)
    }
}
