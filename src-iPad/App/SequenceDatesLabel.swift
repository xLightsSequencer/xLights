import SwiftUI

/// Caption row shown under each .xsq in the sequence picker and the
/// Batch Render sheet. Surfaces the xsq's mtime and the companion fseq's
/// mtime so the user can tell at a glance which sequences need a render
/// before upload.
///
/// - Both dates present, fseq newer/equal: "Modified <rel> · Rendered <rel>"
/// - Both dates present, fseq older:       "Modified <rel> · Rendered <rel> · Out of date" (orange)
/// - xsq has no fseq companion:            "Modified <rel> · Not rendered" (orange)
struct SequenceDatesLabel: View {
    let entry: SequenceEntry

    var body: some View {
        HStack(spacing: 4) {
            if let xsq = entry.modificationDate {
                Text("Modified \(xsq, format: .relative(presentation: .named))")
            }
            if let fseq = entry.fseqModificationDate {
                if entry.modificationDate != nil {
                    Text("·").foregroundStyle(.tertiary)
                }
                Text("Rendered \(fseq, format: .relative(presentation: .named))")
                    .foregroundStyle(entry.isFseqUpToDate ? Color.secondary : Color.orange)
                if !entry.isFseqUpToDate {
                    Text("·").foregroundStyle(.tertiary)
                    Text("Out of date").foregroundStyle(.orange)
                }
            } else if entry.modificationDate != nil {
                Text("·").foregroundStyle(.tertiary)
                Text("Not rendered").foregroundStyle(.orange)
            }
        }
        .font(.caption2)
        .foregroundStyle(.secondary)
        .lineLimit(1)
        .truncationMode(.tail)
    }
}
