import SwiftUI

/// A save found that someone else (another person sharing the show
/// folder, or another app) rewrote the file since it was opened here.
/// Desktop asks the same question (`xLightsFrame::ResolveShowFileConflict`).
struct ShowFileSaveConflict: Identifiable {
    enum Kind { case sequence, layout, presets }

    let id = UUID()
    let kind: Kind
    let fileNames: [String]

    var message: String {
        let names = fileNames.joined(separator: " and ")
        let verb = fileNames.count == 1 ? "was" : "were"
        var text = "\(names) \(verb) changed by someone else (or another app) since you opened it."
        // The iPad patches xlights_rgbeffects.xml rather than rewriting it,
        // so only items edited here replace theirs; the controllers file
        // is written whole.
        if kind == .layout && fileNames == ["xlights_rgbeffects.xml"] {
            text += " Saving now keeps their other changes but replaces anything they changed in the items you edited."
        } else {
            text += " Saving now would overwrite their changes."
        }
        return text
    }

    /// Text for the follow-up notice after "Save a Copy".
    static func copyNotice(copies: [String], originals: [String]) -> String {
        let copyNames = copies.map { ($0 as NSString).lastPathComponent }.joined(separator: ", ")
        let originalNames = originals.joined(separator: " and ")
        let verb = originals.count == 1 ? "was" : "were"
        return "Your version was saved as \(copyNames). \(originalNames) \(verb) left as the other person saved it. Your changes are still open here and are still unsaved."
    }
}

extension View {
    /// Overwrite / Save a Copy / Cancel for a `ShowFileSaveConflict`,
    /// plus the notice shown after a copy is written.
    func showFileConflictAlert(_ conflict: Binding<ShowFileSaveConflict?>,
                               notice: Binding<String?>,
                               onOverwrite: @escaping (ShowFileSaveConflict) -> Void,
                               onSaveCopy: @escaping (ShowFileSaveConflict) -> Void) -> some View {
        self
            .alert("Changed by Someone Else",
                   isPresented: Binding(
                    get: { conflict.wrappedValue != nil },
                    set: { if !$0 { conflict.wrappedValue = nil } }),
                   presenting: conflict.wrappedValue) { c in
                Button("Overwrite", role: .destructive) { onOverwrite(c) }
                Button("Save a Copy") { onSaveCopy(c) }
                Button("Cancel", role: .cancel) { }
            } message: { c in
                Text(c.message)
            }
            .alert("Saved a Copy",
                   isPresented: Binding(
                    get: { notice.wrappedValue != nil },
                    set: { if !$0 { notice.wrappedValue = nil } })) {
                Button("OK", role: .cancel) { notice.wrappedValue = nil }
            } message: {
                Text(notice.wrappedValue ?? "")
            }
    }
}
