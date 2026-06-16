import SwiftUI
import UIKit
import UniformTypeIdentifiers

// Color-curve preset load / save sheets. Mirrors
// `ValueCurvePresetSheets.swift` but over the `.xcc` file-I/O bridge
// on `XLSequenceDocument` (`savedColorCurves`,
// `saveColorCurveSerialised(_:asName:)`, `deleteSavedColorCurve`).

// MARK: - Load

/// Preset browser: lists `.xcc` files under
/// `<showFolder>/colorcurves/` + any bundled `colorcurves/` in the
/// app resources. Each row shows a gradient-strip thumbnail of the
/// curve via `CCPresetThumbnail` + the filename. Swipe-to-delete on
/// user-writable entries.
struct ColorCurveLoadSheet: View {
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
                        "No Saved Color Curves",
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
                                    CCPresetThumbnail(serialised: e.serialised)
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
            .navigationTitle("Saved Color Curves")
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
        let raw = viewModel.document.savedColorCurves()
        entries = raw.compactMap { d in
            guard let f = d["filename"], let s = d["serialised"] else { return nil }
            return Entry(filename: f, serialised: s)
        }
        .sorted { $0.filename.localizedCaseInsensitiveCompare($1.filename) == .orderedAscending }
        didLoad = true
    }

    private func deleteEntry(_ e: Entry) {
        if viewModel.document.deleteSavedColorCurve(e.filename) {
            reload()
        }
    }
}

/// Canvas thumbnail painting the gradient at 1 px/column. Uses a
/// throwaway `XLColorCurve` so we don't build a full
/// `EditableColorCurve` per row.
struct CCPresetThumbnail: View {
    let serialised: String

    var body: some View {
        let core = XLColorCurve(serialised: serialised, identifier: "Dummy")
        Canvas { ctx, size in
            let w = Int(size.width)
            guard w > 0 else { return }
            for px in 0..<w {
                let frac = Float(px) / Float(w)
                let c = Color(uiColor: core.color(atOffset: frac))
                let r = CGRect(x: CGFloat(px), y: 0, width: 1, height: size.height)
                ctx.fill(Path(r), with: .color(c))
            }
        }
        .clipShape(RoundedRectangle(cornerRadius: 4))
    }
}

// MARK: - Save As

struct ColorCurveSaveAsSheet: View {
    @Environment(\.dismiss) private var dismiss
    let onSave: (String) -> Void

    @State private var name: String = ""

    var body: some View {
        NavigationStack {
            Form {
                Section {
                    TextField("Color curve name", text: $name)
                        .autocorrectionDisabled()
                } header: {
                    Text("Save As")
                } footer: {
                    Text("Filename: \(sanitised).xcc")
                        .font(.caption2)
                }
            }
            .navigationTitle("Save Color Curve")
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

// MARK: - Export document

/// `.xcc` document carrying the full XML built by the bridge, handed
/// to SwiftUI's `.fileExporter` so a color curve can be saved to an
/// arbitrary location (desktop's `ButtonExport` parity).
struct ColorCurveExportDocument: FileDocument {
    static let xccType: UTType = UTType(filenameExtension: "xcc") ?? .xml
    static var readableContentTypes: [UTType] { [xccType] }
    static var writableContentTypes: [UTType] { [xccType] }

    let text: String

    init(text: String) { self.text = text }
    init(configuration: ReadConfiguration) throws { self.text = "" }

    func fileWrapper(configuration: WriteConfiguration) throws -> FileWrapper {
        FileWrapper(regularFileWithContents: Data(text.utf8))
    }
}
