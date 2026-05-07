import SwiftUI

// File-pick post-processor that enforces the iPad media-location
// invariant: every stored media path MUST live under the show folder
// (or a subdirectory of it) or under a configured media folder.
//
// Usage:
//
//     @State private var pendingPick: URL? = nil
//     // inside `.fileImporter` callback:
//     if case .success(let url) = result {
//         pendingPick = url      // modifier takes over from here
//     }
//     ...
//     .mediaRelocationPrompt(
//         picked: $pendingPick,
//         subdirectory: "Images",
//         onCommit: { stored in
//             viewModel.setSettingValue(stored, forKey: settingKey)
//         })
//
// Flow:
//   - If the picked URL is already under the show folder or any media
//     folder: commit immediately with a show-relative path (media-folder
//     files stay absolute so the original location isn't clobbered).
//   - Otherwise: prompt the user to choose a destination (Show Folder
//     or one of the configured Media Folders). Cancelling aborts —
//     nothing is stored. There is intentionally NO "keep where it is"
//     option; the invariant is the whole point of this feature.
//
// Security-scoped access is started on pick and torn down after the
// commit (or cancel), so the bridge-side copy can read the source
// while the resource is live.

// MARK: - Relocation modifier

struct MediaRelocationPromptModifier: ViewModifier {
    @Environment(SequencerViewModel.self) var viewModel
    @Binding var picked: URL?
    let subdirectory: String
    let onCommit: (String) -> Void

    @State private var pendingPick: URL?

    func body(content: Content) -> some View {
        content
            .onChange(of: picked) { _, newURL in
                guard let url = newURL else { return }
                // Clear the incoming binding immediately so the next
                // pick retriggers even if it's the same URL.
                picked = nil
                handle(url)
            }
            .confirmationDialog(
                "Copy media into…",
                isPresented: promptBinding,
                titleVisibility: .visible,
                presenting: pendingPick
            ) { url in
                Button("Show Folder") { commitToShow(url) }
                ForEach(mediaFolders, id: \.self) { (mf: String) in
                    Button("Media: \(displayName(for: mf))") {
                        commitToMedia(url, folder: mf)
                    }
                }
                Button("Cancel", role: .cancel) { cancel(url) }
            } message: { _ in
                Text("""
                    This file is outside your show folder and configured \
                    media folders. Choose where to copy it.
                    """)
            }
    }

    private var promptBinding: Binding<Bool> {
        Binding(
            get: { pendingPick != nil },
            set: { isPresented in
                // The system-driven dismissal path fires on outside-tap
                // too; treat that the same as Cancel so the security-
                // scoped access isn't leaked.
                if !isPresented, let url = pendingPick {
                    cancel(url)
                }
            }
        )
    }

    private var mediaFolders: [String] {
        viewModel.document.mediaFolderPaths()
    }

    private func displayName(for path: String) -> String {
        URL(fileURLWithPath: path).lastPathComponent
    }

    // MARK: Handlers

    private func handle(_ url: URL) {
        _ = url.startAccessingSecurityScopedResource()

        if viewModel.document.pathIs(inShowOrMediaFolder: url.path) {
            // Already inside an enforced root — store as-is. Show-folder
            // paths become relative (portable); media-folder paths
            // stay absolute (preserves the original location).
            let showDir = viewModel.document.showFolderPath()
            let isInShow = !showDir.isEmpty && url.path.hasPrefix(showDir)
            let stored = isInShow
                ? (viewModel.document.makeRelativePath(url.path))
                : url.path
            onCommit(stored)
            url.stopAccessingSecurityScopedResource()
            return
        }

        // Source is outside both roots — user must pick a destination.
        pendingPick = url
    }

    private func commitToShow(_ url: URL) {
        // Stage the file into place, then dismiss the dialog before
        // running the consumer's onCommit. The deferred onCommit
        // sidesteps a SwiftUI crash on iPadOS 26.2 where the
        // confirmationDialog's button action mutating parent state
        // mid-dismissal can tear down this modifier's @State —
        // observed via TestFlight as
        // `StoredLocationBase.updateValue.getter` crash when the
        // user picks media from outside the show folder and clicks
        // Show / Media to copy.
        let copied = viewModel.document.moveFile(
            toShowFolder: url.path,
            subdirectory: subdirectory)
        let stored = copied.flatMap { viewModel.document.makeRelativePath($0) }
        url.stopAccessingSecurityScopedResource()
        pendingPick = nil
        if let stored = stored {
            DispatchQueue.main.async { onCommit(stored) }
        }
    }

    private func commitToMedia(_ url: URL, folder: String) {
        // See commitToShow for the deferred-onCommit rationale.
        let copied = viewModel.document.copyFile(
            toMediaFolder: url.path,
            mediaFolderPath: folder,
            subdirectory: subdirectory)
        url.stopAccessingSecurityScopedResource()
        pendingPick = nil
        if let copied = copied {
            // Media-folder destinations stay absolute — stored path
            // is the full copy location inside the media folder.
            DispatchQueue.main.async { onCommit(copied) }
        }
    }

    private func cancel(_ url: URL) {
        url.stopAccessingSecurityScopedResource()
        pendingPick = nil
    }
}

extension View {
    /// Attach relocation handling to a view that uses `.fileImporter`.
    /// `picked` is the `@State` the `.fileImporter` success-branch
    /// writes into; `onCommit` is called with the final stored path
    /// (relative to the show folder where possible) once the file is
    /// in place.
    func mediaRelocationPrompt(
        picked: Binding<URL?>,
        subdirectory: String,
        onCommit: @escaping (String) -> Void
    ) -> some View {
        modifier(MediaRelocationPromptModifier(
            picked: picked,
            subdirectory: subdirectory,
            onCommit: onCommit
        ))
    }
}
