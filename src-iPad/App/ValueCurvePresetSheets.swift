import SwiftUI
import UIKit

// Value-curve preset load / save sheets + clipboard helper
// (G36 / G37 — C6). Works over the `.xvc` file-I/O bridge on
// `XLSequenceDocument` (`savedValueCurves`,
// `saveValueCurveSerialised(_:asName:)`, `deleteSavedValueCurve`).

// MARK: - Load

/// Preset browser: lists `.xvc` files under
/// `<showFolder>/valuecurves/` + any bundled `valuecurves/` in the
/// app resources. Each row shows a small thumbnail of the curve
/// shape via `ValueCurvePreviewStrip` + the filename. Swipe-to-
/// delete on user-writable entries.
struct ValueCurveLoadPresetSheet: View {
    @Environment(\.dismiss) private var dismiss
    @Environment(SequencerViewModel.self) private var viewModel
    let onApply: (String) -> Void

    @State private var entries: [Entry] = []
    @State private var didLoad = false

    struct Entry: Identifiable {
        let id = UUID()
        let filename: String
        let serialised: String
    }

    var body: some View {
        NavigationStack {
            Group {
                if didLoad && entries.isEmpty {
                    ContentUnavailableView(
                        "No Saved Value Curves",
                        systemImage: "folder.badge.questionmark",
                        description: Text("Saved presets appear here after you tap Save As Preset in the editor.")
                    )
                } else {
                    List {
                        ForEach(entries) { e in
                            Button {
                                onApply(e.serialised)
                                dismiss()
                            } label: {
                                HStack(spacing: 12) {
                                    VCPresetThumbnail(serialised: e.serialised)
                                        .frame(width: 80, height: 28)
                                    VStack(alignment: .leading, spacing: 2) {
                                        Text(displayName(e.filename))
                                            .font(.callout)
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
            .navigationTitle("Saved Value Curves")
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
        (filename as NSString).deletingPathExtension
    }

    private func reload() {
        let raw = viewModel.document.savedValueCurves() ?? []
        entries = raw.compactMap { d in
            guard let f = d["filename"], let s = d["serialised"] else { return nil }
            return Entry(filename: f, serialised: s)
        }
        .sorted { $0.filename.localizedCaseInsensitiveCompare($1.filename) == .orderedAscending }
        didLoad = true
    }

    private func deleteEntry(_ e: Entry) {
        if viewModel.document.deleteSavedValueCurve(e.filename) {
            reload()
        }
    }
}

/// Canvas thumbnail sampling the curve at 1 px/column. Uses a
/// throwaway `XLValueCurve` so we don't build a full
/// `EditableValueCurve` per row.
struct VCPresetThumbnail: View {
    let serialised: String

    var body: some View {
        let core = XLValueCurve(serialised: serialised)
        Canvas { ctx, size in
            let w = Int(size.width)
            let h = Int(size.height)
            guard w > 1, h > 1 else { return }
            ctx.fill(Path(CGRect(origin: .zero, size: size)),
                     with: .color(Color.secondary.opacity(0.1)))
            var path = Path()
            for px in 0..<w {
                let frac = Double(px) / Double(w - 1)
                let v = core.value(atOffset: frac)
                let y = CGFloat(1.0 - v) * size.height
                if px == 0 {
                    path.move(to: CGPoint(x: 0, y: y))
                } else {
                    path.addLine(to: CGPoint(x: CGFloat(px), y: y))
                }
            }
            ctx.stroke(path, with: .color(Color.accentColor), lineWidth: 1.5)
        }
        .clipShape(RoundedRectangle(cornerRadius: 4))
    }
}

// MARK: - Save As

struct ValueCurveSaveAsSheet: View {
    @Environment(\.dismiss) private var dismiss
    let onSave: (String) -> Void

    @State private var name: String = ""

    var body: some View {
        NavigationStack {
            Form {
                Section {
                    TextField("Value curve name", text: $name)
                        .autocorrectionDisabled()
                } header: {
                    Text("Save As")
                } footer: {
                    Text("Filename: \(sanitised).xvc")
                        .font(.caption2)
                }
            }
            .navigationTitle("Save Value Curve")
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
                    .disabled(sanitised.isEmpty)
                }
            }
        }
    }

    private var sanitised: String {
        name.unicodeScalars
            .filter { CharacterSet.alphanumerics.contains($0) }
            .map(Character.init)
            .reduce(into: "") { $0.append($1) }
    }
}

// MARK: - Clipboard helper

/// Round-trip a serialised VC string through `UIPasteboard`. We
/// wrap it in a typed prefix so a stray paste into a text field
/// somewhere else doesn't land as raw curve machinery, and so the
/// Paste button in the editor can detect whether there's a real
/// VC on the clipboard (vs. arbitrary text).
enum ValueCurveClipboard {
    static let prefix = "xlvc:v1:"

    static func wrap(_ serialised: String) -> String {
        return prefix + serialised
    }

    static func unwrap(_ raw: String?) -> String? {
        guard let s = raw, s.hasPrefix(prefix) else { return nil }
        return String(s.dropFirst(prefix.count))
    }

    static func isValid(_ raw: String?) -> Bool {
        unwrap(raw) != nil
    }
}
