import SwiftUI

// Tools → Search for Show Folders (desktop xLightsMain.cpp:1158,
// ShowFolderSearchDialog). The desktop scans a drive; on iPad the
// sandbox has no drive to sweep, so the user picks a folder tree
// (security-scoped) and we recurse it for directories containing
// `xlights_rgbeffects.xml` — the file that marks a show folder. Each
// hit can be switched to via the same `loadShowFolder` path the
// folder-config dialog uses.
struct SearchShowFoldersSheet: View {
    @Environment(\.dismiss) private var dismiss
    @Environment(SequencerViewModel.self) private var viewModel

    private struct Found: Identifiable {
        let id = UUID()
        let path: String
        var name: String { (path as NSString).lastPathComponent }
    }

    @State private var picking = false
    @State private var scanning = false
    @State private var found: [Found] = []
    @State private var scannedRoot: String?
    @State private var didScan = false

    var body: some View {
        NavigationStack {
            Group {
                if scanning {
                    ProgressView("Scanning…")
                        .frame(maxWidth: .infinity, maxHeight: .infinity)
                } else if !didScan {
                    ContentUnavailableView {
                        Label("Search for Show Folders", systemImage: "folder.badge.questionmark")
                    } description: {
                        Text("Pick a folder to scan. Every subfolder containing xlights_rgbeffects.xml is a show folder you can switch to.")
                    } actions: {
                        Button("Choose Folder to Scan…") { picking = true }
                            .buttonStyle(.borderedProminent)
                    }
                } else if found.isEmpty {
                    ContentUnavailableView(
                        "No Show Folders Found",
                        systemImage: "folder",
                        description: Text("No xlights_rgbeffects.xml was found under \(scannedRoot.map { ($0 as NSString).lastPathComponent } ?? "the chosen folder").")
                    )
                } else {
                    List {
                        Section {
                            ForEach(found) { f in
                                Button { switchTo(f) } label: {
                                    VStack(alignment: .leading, spacing: 2) {
                                        Text(f.name).font(.callout.weight(.semibold))
                                        Text(f.path)
                                            .font(.caption2)
                                            .foregroundStyle(.secondary)
                                            .lineLimit(2)
                                            .truncationMode(.middle)
                                    }
                                }
                                .buttonStyle(.plain)
                            }
                        } header: {
                            Text("\(found.count) show folder\(found.count == 1 ? "" : "s")")
                        } footer: {
                            Text("Tap a folder to switch the active Show Folder to it.")
                        }
                    }
                }
            }
            .navigationTitle("Search for Show Folders")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Done") { dismiss() }
                }
                if didScan {
                    ToolbarItem(placement: .confirmationAction) {
                        Button("Rescan…") { picking = true }
                    }
                }
            }
        }
        .sheet(isPresented: $picking) {
            ShowFolderPicker { url in
                picking = false
                scan(root: url)
            }
        }
    }

    private func scan(root: URL) {
        scanning = true
        didScan = false
        scannedRoot = root.path
        FolderConfig.registerBookmark(from: root)
        Task { @MainActor in
            let hits = await Task.detached(priority: .userInitiated) {
                Self.findShowFolders(under: root)
            }.value
            found = hits.map { Found(path: $0) }
            scanning = false
            didScan = true
        }
    }

    // Recurse `root` (already security-scoped) for directories holding
    // xlights_rgbeffects.xml. A show folder is not nested inside
    // another, so once one is found we don't descend further into it.
    private nonisolated static func findShowFolders(under root: URL) -> [String] {
        var results: [String] = []
        let fm = FileManager.default
        let marker = "xlights_rgbeffects.xml"
        guard let en = fm.enumerator(at: root,
                                     includingPropertiesForKeys: [.isDirectoryKey],
                                     options: [.skipsHiddenFiles]) else {
            return results
        }
        for case let url as URL in en {
            let isDir = (try? url.resourceValues(forKeys: [.isDirectoryKey]))?.isDirectory ?? false
            if isDir {
                if fm.fileExists(atPath: url.appendingPathComponent(marker).path) {
                    results.append(url.path)
                    en.skipDescendants()
                }
            }
        }
        // Include the root itself if it is a show folder.
        if fm.fileExists(atPath: root.appendingPathComponent(marker).path) {
            results.insert(root.path, at: 0)
        }
        return results.sorted()
    }

    private func switchTo(_ f: Found) {
        let url = URL(fileURLWithPath: f.path)
        FolderConfig.registerBookmark(from: url)
        FolderConfig.showFolder = f.path
        // A different show carries its own (or no) media folders — drop
        // the previous show's so they aren't resolved against the new one.
        FolderConfig.mediaFolders = []
        viewModel.loadShowFolder(path: f.path, mediaFolders: [])
        dismiss()
    }
}
