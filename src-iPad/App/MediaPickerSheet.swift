import SwiftUI
import UIKit

// Sheet that replaces the plain Select… button on effect file-picker
// rows. Top section lists media already referenced by an effect in
// this sequence (one-tap reuse with thumbnails) — matches desktop's
// SelectMediaDialog, which is the biggest day-to-day friction on
// iPad (Gap G27). Bottom section keeps the Browse… escape hatch for
// new files plus a Clear action.
//
// Filter by media type so the list is scoped to what the caller
// needs: a Shader picker doesn't show images. `mediaType` values
// match the bridge ("image", "video", "shader", "svg", "text",
// "binary"). Nil shows everything.
struct MediaPickerSheet: View {
    @Environment(\.dismiss) private var dismiss
    @Environment(SequencerViewModel.self) private var viewModel
    let title: String
    let mediaType: String?
    /// The effect's currently-stored path (empty if nothing is
    /// picked). When non-empty and it matches one of the "used in
    /// this sequence" rows, that row gets a highlight + a leading
    /// checkmark so the user can see "this is the one you already
    /// have" at a glance.
    let currentPath: String
    let onPick: (String) -> Void
    let onBrowse: () -> Void
    let onClear: () -> Void

    @State private var entries: [Entry] = []
    @State private var groups: [FolderGroup] = []
    @State private var didLoad = false
    @State private var showManager = false

    // `|`-joined list of folder-group IDs the user has manually
    // collapsed. Empty default means "everything expanded". Persisted
    // across launches via `@AppStorage` so a user who collapsed
    // `Images/SuperStar/Snowflakes` on one sequence doesn't see it
    // expand again on the next sequence with the same folder
    // structure. Cross-sheet scope is fine — folder names are
    // distinctive enough that collisions would be intentional.
    @AppStorage("mediaPicker.collapsedGroups") private var collapsedGroupsRaw: String = ""

    struct Entry: Identifiable {
        let id = UUID()
        let path: String
        let type: String
        var filename: String {
            (path as NSString).lastPathComponent
        }
    }

    /// Entries bucketed by their parent-directory path (show-relative
    /// when under the show folder; absolute otherwise). Only
    /// materialised when there's more than one distinct parent
    /// directory — a flat list of entries all in the same folder
    /// wouldn't benefit from the extra chrome.
    struct FolderGroup: Identifiable {
        let id: String        // stable key: relative (or absolute) dir path
        let label: String     // short display label (last path segment, or "Show Folder" when root)
        let fullPath: String  // show-relative (or absolute) dir path for the header subtitle
        let entries: [Entry]
    }

    var body: some View {
        NavigationStack {
            List {
                if didLoad && entries.isEmpty {
                    Section {
                        Text("No \(typeLabel.lowercased()) in this sequence yet.")
                            .font(.callout)
                            .foregroundStyle(.secondary)
                    }
                } else if groups.count > 1 {
                    // Multiple folders → disclosure groups. Keeps
                    // hundreds of imported LOR / SuperStar files
                    // manageable without the user scrolling a flat
                    // alphabetical list of everything.
                    ForEach(groups) { group in
                        Section {
                            DisclosureGroup(
                                isExpanded: expansionBinding(for: group.id)
                            ) {
                                ForEach(group.entries) { entry in
                                    entryRow(entry)
                                }
                            } label: {
                                folderHeader(group)
                            }
                        }
                    }
                } else if !entries.isEmpty {
                    Section(header: Text("Used in this sequence")) {
                        ForEach(entries) { entry in
                            entryRow(entry)
                        }
                    }
                }
                Section {
                    Button {
                        // `onBrowse` is expected to present a
                        // UIDocumentPicker — it needs the sheet to
                        // dismiss first so the importer can come up
                        // in the parent.
                        dismiss()
                        DispatchQueue.main.async {
                            onBrowse()
                        }
                    } label: {
                        Label("Browse Files…", systemImage: "folder")
                    }
                    Button {
                        showManager = true
                    } label: {
                        Label("Manage All Media…",
                              systemImage: "rectangle.stack")
                    }
                    Button(role: .destructive) {
                        onClear()
                        dismiss()
                    } label: {
                        Label("Clear", systemImage: "xmark.circle")
                    }
                }
            }
            .navigationTitle(title)
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                }
            }
        }
        .onAppear { loadEntries() }
        .sheet(isPresented: $showManager) {
            MediaManagerSheet()
                .environment(viewModel)
        }
    }

    private var typeLabel: String {
        switch mediaType {
        case "image":  return "Images"
        case "video":  return "Videos"
        case "shader": return "Shaders"
        case "svg":    return "SVGs"
        case "text":   return "Text Files"
        case "binary": return "Binary Files"
        default:       return "Media"
        }
    }

    @ViewBuilder
    private func entryRow(_ entry: Entry) -> some View {
        let isCurrent = (entry.path == currentPath)
        Button {
            onPick(entry.path)
            dismiss()
        } label: {
            HStack(spacing: 10) {
                MediaThumbnailView(path: entry.path,
                                   mediaType: entry.type,
                                   size: 40)
                VStack(alignment: .leading, spacing: 2) {
                    Text(entry.filename)
                        .font(.callout)
                        .foregroundStyle(isCurrent
                                         ? Color.accentColor
                                         : .primary)
                        .fontWeight(isCurrent ? .semibold : .regular)
                        .lineLimit(1)
                        .truncationMode(.middle)
                    Text(entry.path)
                        .font(.caption2)
                        .foregroundStyle(.secondary)
                        .lineLimit(1)
                        .truncationMode(.middle)
                }
                if isCurrent {
                    Spacer()
                    Image(systemName: "checkmark")
                        .font(.body.weight(.semibold))
                        .foregroundStyle(Color.accentColor)
                }
            }
        }
        .listRowBackground(isCurrent
                           ? Color.accentColor.opacity(0.12)
                           : nil)
    }

    @ViewBuilder
    private func folderHeader(_ group: FolderGroup) -> some View {
        HStack(spacing: 8) {
            Image(systemName: "folder")
                .foregroundStyle(.secondary)
            VStack(alignment: .leading, spacing: 1) {
                Text(group.label)
                    .font(.callout)
                    .fontWeight(.medium)
                if group.fullPath != group.label && !group.fullPath.isEmpty {
                    Text(group.fullPath)
                        .font(.caption2)
                        .foregroundStyle(.secondary)
                        .lineLimit(1)
                        .truncationMode(.head)
                }
            }
            Spacer()
            Text("\(group.entries.count)")
                .font(.caption2)
                .monospacedDigit()
                .foregroundStyle(.secondary)
        }
    }

    // MARK: - Expansion state

    private var collapsedGroupIds: Set<String> {
        collapsedGroupsRaw.isEmpty
            ? []
            : Set(collapsedGroupsRaw.split(separator: "|").map(String.init))
    }

    private func expansionBinding(for groupId: String) -> Binding<Bool> {
        Binding(
            get: { !collapsedGroupIds.contains(groupId) },
            set: { isExpanded in
                var s = collapsedGroupIds
                if isExpanded {
                    s.remove(groupId)
                } else {
                    s.insert(groupId)
                }
                collapsedGroupsRaw = s.sorted().joined(separator: "|")
            }
        )
    }

    // MARK: - Loading + grouping

    private func loadEntries() {
        let all = viewModel.document.mediaPathsInSequence() ?? []
        let filtered: [Entry] = all.compactMap { dict in
            guard let path = dict["path"], let type = dict["type"] else {
                return nil
            }
            if let filter = mediaType, type != filter { return nil }
            return Entry(path: path, type: type)
        }
        .sorted { $0.filename.localizedCaseInsensitiveCompare($1.filename) == .orderedAscending }

        entries = filtered
        groups = buildGroups(from: filtered)
        didLoad = true

        // Auto-expand the group containing the current pick so the
        // user can see it without hunting. A subsequent manual
        // collapse re-collapses it for next time — we only override
        // the persisted state on the first render with a currentPath
        // set.
        if !currentPath.isEmpty,
           let matching = groups.first(where: {
               $0.entries.contains { $0.path == currentPath }
           }),
           collapsedGroupIds.contains(matching.id) {
            expansionBinding(for: matching.id).wrappedValue = true
        }
    }

    /// Partition entries by parent directory. Uses the bridge's
    /// `makeRelativePath` so paths under the show folder render as
    /// friendly `Images/BeepBeep/…` rather than the full iCloud
    /// container path. Absolute paths outside the show folder fall
    /// through unchanged — they still group correctly by their
    /// literal parent dir.
    private func buildGroups(from entries: [Entry]) -> [FolderGroup] {
        struct Bucket {
            var id: String
            var label: String
            var fullPath: String
            var entries: [Entry]
        }
        var order: [String] = []
        var buckets: [String: Bucket] = [:]
        let doc = viewModel.document

        for entry in entries {
            let rel = doc.makeRelativePath(entry.path) ?? entry.path
            // Parent-dir extraction that handles both absolute
            // ("/a/b/c.png" → "/a/b") and relative
            // ("Images/BeepBeep/c.png" → "Images/BeepBeep") paths.
            // Files at the root produce an empty dir → bucket
            // under "" with the label "Show Folder".
            let lastSlash = rel.lastIndex(of: "/")
            let dir: String
            if let idx = lastSlash {
                dir = String(rel[..<idx])
            } else {
                dir = ""
            }

            if buckets[dir] == nil {
                let label: String
                if dir.isEmpty {
                    label = "Show Folder"
                } else if let tail = dir.split(separator: "/").last {
                    label = String(tail)
                } else {
                    label = dir
                }
                buckets[dir] = Bucket(id: dir, label: label,
                                       fullPath: dir, entries: [])
                order.append(dir)
            }
            buckets[dir]?.entries.append(entry)
        }

        return order.compactMap { key in
            guard let b = buckets[key] else { return nil }
            return FolderGroup(id: b.id, label: b.label,
                               fullPath: b.fullPath, entries: b.entries)
        }
    }
}

// Async thumbnail loader with multi-frame cycling for animated
// content (GIF / WebP / video first-frame + subsequent samples).
// First pass ensures preview generation happens on a utility queue
// so the cold path (disk read, decode, rescale) doesn't block main.
// For frame counts > 1 a timer walks the frames at the cadence the
// underlying format declares (`thumbnailFrameTimeMS`), wrapping at
// the end — matches desktop MediaPickerCtrl / ManageMediaPanel. The
// cycle pauses when the view disappears (SwiftUI cancels .task), so
// offscreen rows and backgrounded scenes don't burn energy.
struct MediaThumbnailView: View {
    @Environment(SequencerViewModel.self) private var viewModel
    @Environment(\.scenePhase) private var scenePhase
    let path: String
    let mediaType: String
    let size: CGFloat

    @State private var frames: [UIImage] = []
    @State private var frameDurations: [TimeInterval] = []  // seconds
    @State private var currentFrame: Int = 0
    @State private var loadFailed = false
    @State private var cycleTask: Task<Void, Never>?

    var body: some View {
        ZStack {
            if !frames.isEmpty {
                Image(uiImage: frames[min(currentFrame, frames.count - 1)])
                    .resizable()
                    .interpolation(.medium)
                    .aspectRatio(contentMode: .fill)
                    .frame(width: size, height: size)
                    .clipped()
                    .cornerRadius(4)
            } else {
                RoundedRectangle(cornerRadius: 4)
                    .fill(Color.secondary.opacity(0.12))
                    .frame(width: size, height: size)
                    .overlay(
                        Image(systemName: loadFailed ? "questionmark" : "photo")
                            .foregroundStyle(.secondary)
                    )
            }
        }
        .task(id: path) { await loadAndCycle() }
        .onChange(of: scenePhase) { _, newPhase in
            // Background / inactive: cancel the cycle timer. SwiftUI
            // also pauses .task when the scene deactivates but being
            // explicit here keeps `currentFrame` from racing with a
            // stray tick if the view is recreated mid-transition.
            if newPhase != .active { cycleTask?.cancel() }
        }
    }

    private func loadAndCycle() async {
        frames = []
        frameDurations = []
        currentFrame = 0
        loadFailed = false
        cycleTask?.cancel()

        let doc = viewModel.document
        let p = path
        let t = mediaType
        let maxDim = Int32(size * 2)  // @2x for Retina

        // Cold-load preview on a utility queue; hop back to main to
        // populate state. Frames are small (thumbnail resolution)
        // so we cache all of them — typical animated GIF is < 100
        // frames at 80x80, a few MB at most.
        let loaded: ([UIImage], [TimeInterval]) = await withCheckedContinuation { cont in
            DispatchQueue.global(qos: .utility).async {
                let count = doc.ensureThumbnailPreview(forPath: p,
                                                        mediaType: t,
                                                        maxWidth: maxDim,
                                                        maxHeight: maxDim)
                guard count > 0 else { cont.resume(returning: ([], [])); return }
                var imgs: [UIImage] = []
                var durs: [TimeInterval] = []
                imgs.reserveCapacity(Int(count))
                durs.reserveCapacity(Int(count))
                for i in 0..<Int(count) {
                    guard let data = doc.thumbnailPNG(forPath: p,
                                                      mediaType: t,
                                                      frameIndex: Int32(i)) as Data?,
                          let ui = UIImage(data: data) else {
                        // Partial failure mid-strip — use what we have.
                        break
                    }
                    imgs.append(ui)
                    let ms = doc.thumbnailFrameTimeMS(forPath: p,
                                                      mediaType: t,
                                                      frameIndex: Int32(i))
                    durs.append(max(0.02, TimeInterval(ms) / 1000.0))
                }
                cont.resume(returning: (imgs, durs))
            }
        }

        frames = loaded.0
        frameDurations = loaded.1
        if frames.isEmpty {
            loadFailed = true
            return
        }
        if frames.count == 1 { return }

        // Multi-frame: run a cooperative cycler. Cancels on view
        // disappear (.task cancellation) or scene-phase change.
        cycleTask = Task { @MainActor in
            while !Task.isCancelled {
                let seconds = frameDurations[currentFrame]
                try? await Task.sleep(nanoseconds: UInt64(seconds * 1_000_000_000))
                if Task.isCancelled { return }
                currentFrame = (currentFrame + 1) % frames.count
            }
        }
    }
}
