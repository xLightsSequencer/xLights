import SwiftUI
import UIKit

// Palette save / load / import / export sheets (G17 — C3). Works
// over the palette-string round-trip bridge on `XLSequenceDocument`
// (`savedPalettes`, `savePaletteString(_:asName:)`,
// `deleteSavedPalette`, `applyPaletteString(_:toRow:atIndex:)`).

// MARK: - Load

/// Lists `.xpalette` files visible to the app — user-saved under
/// `<showFolder>/Palettes/` and any bundled palettes in Resources.
/// Each row renders the 8-swatch preview so the user can pick
/// visually without knowing the filename. Swipe-to-delete on
/// user-writable entries.
struct PaletteLoadSheet: View {
    @Environment(\.dismiss) private var dismiss
    @Environment(SequencerViewModel.self) private var viewModel
    let onApply: (String) -> Void

    @State private var entries: [Entry] = []
    @State private var didLoad = false

    struct Entry: Identifiable {
        let id = UUID()
        let filename: String
        let palette: String
        /// The 8 hex / curve-blob slots parsed from `palette`.
        var slots: [String] {
            PaletteSerializer.splitSlots(palette)
        }
    }

    var body: some View {
        NavigationStack {
            Group {
                if didLoad && entries.isEmpty {
                    ContentUnavailableView(
                        "No Saved Palettes",
                        systemImage: "folder.badge.questionmark",
                        description: Text("Saved palettes appear here after you tap Save Palette.")
                    )
                } else {
                    List {
                        ForEach(entries) { e in
                            Button {
                                onApply(e.palette)
                                dismiss()
                            } label: {
                                HStack(spacing: 10) {
                                    PaletteSwatchRow(slots: e.slots)
                                        .frame(height: 22)
                                    VStack(alignment: .leading, spacing: 2) {
                                        Text(displayName(e.filename))
                                            .font(.callout)
                                            .foregroundStyle(.primary)
                                            .lineLimit(1)
                                        Text(e.filename)
                                            .font(.caption2)
                                            .foregroundStyle(.secondary)
                                            .lineLimit(1)
                                    }
                                }
                            }
                            .swipeActions(edge: .trailing) {
                                Button(role: .destructive) {
                                    deleteEntry(e)
                                } label: {
                                    Label("Delete", systemImage: "trash")
                                }
                            }
                        }
                    }
                }
            }
            .navigationTitle("Saved Palettes")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                }
            }
        }
        .onAppear { reload() }
    }

    private func displayName(_ filename: String) -> String {
        let base = (filename as NSString).deletingPathExtension
        return base
    }

    private func reload() {
        let raw = viewModel.document.savedPalettes()
        entries = raw.compactMap { d in
            guard let f = d["filename"], let p = d["palette"] else { return nil }
            return Entry(filename: f, palette: p)
        }
        .sorted { $0.filename.localizedCaseInsensitiveCompare($1.filename) == .orderedAscending }
        didLoad = true
    }

    private func deleteEntry(_ e: Entry) {
        if viewModel.document.deleteSavedPalette(e.filename) {
            reload()
        }
    }
}

/// Row of up to 8 colour swatches from a palette string. ColorCurve
/// slots render as a grey box with a `~` glyph since we don't want
/// to run the gradient sampler just for a preview row.
struct PaletteSwatchRow: View {
    let slots: [String]

    var body: some View {
        HStack(spacing: 2) {
            ForEach(0..<min(slots.count, 8), id: \.self) { i in
                let slot = slots[i]
                if slot.hasPrefix("Active=TRUE") {
                    RoundedRectangle(cornerRadius: 3)
                        .fill(Color.secondary.opacity(0.2))
                        .overlay(Text("~").font(.caption2).foregroundStyle(.secondary))
                } else {
                    RoundedRectangle(cornerRadius: 3)
                        .fill(Color(hex: slot) ?? .black)
                }
            }
        }
    }
}

// MARK: - Import

/// Simple paste-a-string dialog. Validates that the input looks
/// like a palette (comma-separated `#RRGGBB` entries) before
/// applying so a stray paste doesn't destroy the user's current
/// palette.
struct PaletteImportSheet: View {
    @Environment(\.dismiss) private var dismiss
    let onImport: (String) -> Void

    @State private var text: String = ""
    @State private var error: String? = nil

    var body: some View {
        NavigationStack {
            Form {
                Section {
                    TextField("e.g. #FF0000,#00FF00,#0000FF,…",
                              text: $text,
                              axis: .vertical)
                        .lineLimit(3...6)
                        .textInputAutocapitalization(.characters)
                        .autocorrectionDisabled()
                        .monospaced()
                } header: {
                    Text("Palette Text String")
                } footer: {
                    Text("Paste a comma-separated list of up to 8 `#RRGGBB` hex colours. ColorCurve blobs from desktop are accepted too.")
                        .font(.caption2)
                }
                if let error = error {
                    Section {
                        Text(error)
                            .font(.caption)
                            .foregroundStyle(.red)
                    }
                }
            }
            .navigationTitle("Import Palette")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Apply") { commit() }
                }
            }
        }
        .onAppear {
            // Pre-populate from the pasteboard if it looks like a
            // palette string — saves one extra tap for the
            // "copied on desktop, pasting here" flow.
            if let pb = UIPasteboard.general.string,
               PaletteSerializer.isValidPalette(pb) {
                text = pb
            }
        }
    }

    private func commit() {
        let cleaned = PaletteSerializer.normalise(text)
        if !PaletteSerializer.isValidPalette(cleaned) {
            error = "Doesn't look like a palette. Expected comma-separated `#RRGGBB` colours."
            return
        }
        onImport(cleaned)
        dismiss()
    }
}

// MARK: - Save As

/// Named-save dialog. Filename is the user's string sanitised to
/// alphanumerics — matches desktop's `RemoveNonAlphanumeric` so
/// palette files round-trip visually across platforms.
struct PaletteSaveAsSheet: View {
    @Environment(\.dismiss) private var dismiss
    let onSave: (String) -> Void

    @State private var name: String = ""

    var body: some View {
        NavigationStack {
            Form {
                Section {
                    TextField("Palette name", text: $name)
                        .autocorrectionDisabled()
                } header: {
                    Text("Save As")
                } footer: {
                    Text("Filename: \(displayFilename).xpalette")
                        .font(.caption2)
                }
            }
            .navigationTitle("Save Palette As")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Save") {
                        onSave(name)
                        dismiss()
                    }
                    .disabled(displayFilename.isEmpty)
                }
            }
        }
    }

    private var displayFilename: String {
        name.unicodeScalars
            .filter { CharacterSet.alphanumerics.contains($0) }
            .map(Character.init)
            .reduce(into: "") { $0.append($1) }
    }
}

// MARK: - Helpers

enum PaletteSerializer {
    /// Split a palette string into top-level slots, respecting
    /// `Active=TRUE|…|` ColorCurve blobs which contain their own
    /// commas.
    static func splitSlots(_ s: String) -> [String] {
        var slots: [String] = []
        var current = ""
        var inCurve = false
        let chars = Array(s)
        var i = 0
        while i < chars.count {
            let c = chars[i]
            if !inCurve {
                if c == "," {
                    slots.append(current)
                    current = ""
                } else {
                    // Look ahead for "Active=TRUE|" start marker at
                    // the beginning of a slot.
                    if current.isEmpty,
                       chars[i...].starts(with: Array("Active=TRUE|")) {
                        inCurve = true
                    }
                    current.append(c)
                }
            } else {
                current.append(c)
                if c == "|", i + 1 < chars.count, chars[i + 1] == "," {
                    inCurve = false
                }
            }
            i += 1
        }
        if !current.isEmpty { slots.append(current) }
        // Drop trailing filename trailer if present (desktop writes
        // "palette,<filename>.xpalette").
        return slots.filter { !$0.lowercased().hasSuffix(".xpalette") }
    }

    /// Is `s` plausibly a palette string? We require at least one
    /// `#RRGGBB` or `Active=TRUE|…|` slot — rejects pasted random
    /// text so import doesn't wipe the current palette.
    static func isValidPalette(_ s: String) -> Bool {
        let slots = splitSlots(s)
        guard !slots.isEmpty else { return false }
        for slot in slots {
            if slot.hasPrefix("Active=TRUE") { return true }
            let t = slot.trimmingCharacters(in: .whitespaces)
            if t.hasPrefix("#"), t.count == 7 || t.count == 9,
               Int(t.dropFirst(), radix: 16) != nil {
                return true
            }
        }
        return false
    }

    /// Trim whitespace + ensure trailing comma so the string
    /// matches desktop's serialised shape.
    static func normalise(_ s: String) -> String {
        let trimmed = s.trimmingCharacters(in: .whitespacesAndNewlines)
        return trimmed.hasSuffix(",") ? trimmed : trimmed + ","
    }
}

// MARK: - Color(hex:)

extension Color {
    init?(hex: String) {
        var s = hex.trimmingCharacters(in: .whitespaces)
        if s.hasPrefix("#") { s.removeFirst() }
        guard s.count == 6 || s.count == 8, let val = UInt64(s, radix: 16) else { return nil }
        self = Color(red: Double((val >> 16) & 0xFF) / 255.0,
                     green: Double((val >> 8) & 0xFF) / 255.0,
                     blue: Double(val & 0xFF) / 255.0)
    }
}
