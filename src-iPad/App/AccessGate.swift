import SwiftUI

/// A pending re-grant prompt for a security-scoped path whose persisted
/// bookmark went stale (most often: iCloud evicted the show folder
/// between launches, or iOS aged out the bookmark). Surfaced by
/// `SequencerViewModel` and presented by `ContentView` as a sheet.
struct AccessRepromptRequest: Identifiable {
    let id = UUID()
    /// Human-readable description of *what* is being re-granted, e.g.
    /// "show folder", "media folder", "sequence file". Lower-cased so
    /// it can flow into a sentence.
    let label: String
    /// The original path whose `obtainAccessToPath` failed. Used for
    /// retry after the user picks; also used to derive `displayName`.
    let originalPath: String
    /// The folder the user should pick to restore access. Equals
    /// `originalPath` for folders; for files (sequence file) it's the
    /// parent directory, since iOS grants child access transitively
    /// through the parent's security scope.
    let pickPath: String

    var displayName: String {
        (originalPath as NSString).lastPathComponent
    }
}

/// SwiftUI shell for an access re-grant. Explains the situation, then
/// launches `ShowFolderPicker` via `fullScreenCover` when the user taps
/// "Choose Folder…". Skip dismisses without action; the caller decides
/// what to do with the cancelled state.
struct AccessRepromptSheet: View {
    let request: AccessRepromptRequest
    let onPicked: (URL) -> Void
    let onCancel: () -> Void

    @State private var showPicker = false
    @Environment(\.dismiss) private var dismiss

    var body: some View {
        NavigationStack {
            VStack(spacing: 20) {
                Spacer()
                Image(systemName: "lock.shield")
                    .font(.system(size: 56))
                    .foregroundStyle(.tint)
                Text("Access Expired")
                    .font(.title2.bold())
                Text("xLights lost access to your \(request.label) “\(request.displayName)”. iOS occasionally revokes folder permissions — re-select the folder to restore access.")
                    .multilineTextAlignment(.center)
                    .foregroundStyle(.secondary)
                    .padding(.horizontal)
                Spacer()
                Button {
                    showPicker = true
                } label: {
                    Text("Choose Folder…")
                        .frame(maxWidth: .infinity)
                }
                .buttonStyle(.borderedProminent)
                .controlSize(.large)
                Button(role: .cancel) {
                    onCancel()
                    dismiss()
                } label: {
                    Text("Skip")
                        .frame(maxWidth: .infinity)
                }
                .controlSize(.large)
            }
            .padding(32)
            .navigationBarTitleDisplayMode(.inline)
        }
        .fullScreenCover(isPresented: $showPicker) {
            ShowFolderPicker { url in
                showPicker = false
                onPicked(url)
                dismiss()
            }
        }
    }
}
