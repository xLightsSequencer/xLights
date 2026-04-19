import SwiftUI
import UniformTypeIdentifiers

// File-picker row for JSON properties with `controlType: "filepicker"`
// (Glediator_Filename, VUMeter_Filename). Mirrors the desktop wxFilePicker:
// a button that opens a document picker, a label showing the current file's
// last-path component, and a Clear button.
//
// Picked files route through `MediaRelocation` — if the source is
// outside the show / media folders the user is prompted for a
// destination and the file is copied there before the path is
// committed. Nothing outside the enforced roots is ever stored.
struct FilepickerPropertyView: View {
    let property: PropertyMetadata
    let currentPath: String
    let onChoose: (String) -> Void
    let onClear: () -> Void

    @State private var presentingPicker = false
    @State private var pendingPick: URL?

    private var filename: String {
        if currentPath.isEmpty { return "(none)" }
        return (currentPath as NSString).lastPathComponent
    }

    /// Derive the canonical subdirectory from the property id. The two
    /// real filepicker uses in the metadata today are Glediator's `.gled`
    /// files (→ `Glediator`, matching desktop's `GlediatorEffect.cpp:287`)
    /// and VUMeter's SVG file (→ `Images`, matching
    /// `VUMeterEffect.cpp:254`). Unknown ids fall back to the show
    /// root — the file still lands inside the enforced media roots, it
    /// just isn't tucked into a subfolder.
    private var subdirectory: String {
        let id = property.id.lowercased()
        if id.contains("glediator") { return "Glediator" }
        if id.contains("svg") || id.contains("vumeter") { return "Images" }
        return ""
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 2) {
            Text(property.label)
                .font(.caption)
            HStack(spacing: 6) {
                Text(filename)
                    .font(.caption2)
                    .foregroundStyle(.secondary)
                    .lineLimit(1)
                    .truncationMode(.middle)
                    .frame(maxWidth: .infinity, alignment: .leading)
                Button("Select…") { presentingPicker = true }
                    .buttonStyle(.bordered)
                    .controlSize(.small)
                if !currentPath.isEmpty {
                    Button(action: onClear) {
                        Image(systemName: "xmark.circle.fill")
                            .foregroundStyle(.secondary)
                    }
                    .buttonStyle(.plain)
                }
            }
        }
        .padding(.vertical, 2)
        .fileImporter(isPresented: $presentingPicker,
                      allowedContentTypes: allowedTypes()) { result in
            if case .success(let url) = result {
                pendingPick = url
            }
        }
        .mediaRelocationPrompt(
            picked: $pendingPick,
            subdirectory: subdirectory
        ) { storedPath in
            onChoose(storedPath)
        }
    }

    /// JSON's `fileFilter` follows wx's pipe-delimited format (e.g.
    /// `"Glediator Files (*.gled)|*.gled|CSV files (*.csv)|*.csv"`). iOS
    /// wants UTTypes — extract the *.ext globs and map to UTTypes by
    /// filename extension. Unknown extensions fall back to `.data` so the
    /// picker at least opens (the file still stores its real extension).
    private func allowedTypes() -> [UTType] {
        guard let filter = property.fileFilter, !filter.isEmpty else {
            return [.data]
        }
        var types: [UTType] = []
        // Walk the pipe groups; every other segment is a pattern list.
        let parts = filter.split(separator: "|").map(String.init)
        for (i, part) in parts.enumerated() where i % 2 == 1 {
            for pattern in part.split(separator: ";") {
                let glob = String(pattern).trimmingCharacters(in: .whitespaces)
                if let dotIdx = glob.lastIndex(of: ".") {
                    let ext = String(glob[glob.index(after: dotIdx)...])
                    if ext == "*" { continue }
                    if let t = UTType(filenameExtension: ext) {
                        types.append(t)
                    }
                }
            }
        }
        return types.isEmpty ? [.data] : types
    }
}
