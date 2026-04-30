import SwiftUI
import UIKit
import UniformTypeIdentifiers

// Sequence-wide media manager (G28 — C5). Lists every media file
// referenced by any effect in the currently-open sequence with
// status (Embedded / External / Missing), size / frame metadata, and
// a per-type grouped layout. Read-only for now — embed / extract,
// rename with reference update, remove-unused, and video compat
// checks land in later sessions (G29–G32).
//
// Backed by `XLSequenceDocument.mediaInventoryInSequence`. Opened
// from the per-effect picker's "Manage All Media…" entry and from
// the missing-media banner on sequence open.

struct MediaInventoryItem: Identifiable {
    let id = UUID()
    let path: String
    let type: String
    let resolvedPath: String
    let isEmbedded: Bool
    let isBroken: Bool
    let widthPx: Int
    let heightPx: Int
    let frameCount: Int

    var filename: String {
        (path as NSString).lastPathComponent
    }

    /// Status label for the trailing chip.
    var statusLabel: String {
        if isBroken { return "Missing" }
        if isEmbedded { return "Embedded" }
        return "External"
    }

    var statusColor: Color {
        if isBroken { return .red }
        if isEmbedded { return .purple }
        return .secondary
    }

    /// Video + binary entries are un-embeddable. Mirrors desktop's
    /// `MediaCacheEntry::IsEmbeddable` overrides.
    var isEmbeddable: Bool {
        type != "video" && type != "binary"
    }

    /// Human summary: "1920×1080 · 12 frames" / "2:34 mm:ss" /
    /// "Embedded 480 kB". Kept concise so rows stay at one line.
    var metaSummary: String {
        var parts: [String] = []
        if widthPx > 0 && heightPx > 0 {
            parts.append("\(widthPx)×\(heightPx)")
        }
        if frameCount > 1 {
            parts.append("\(frameCount) frames")
        }
        return parts.joined(separator: " · ")
    }
}

struct MediaManagerSheet: View {
    var body: some View {
        NavigationStack {
            MediaManagerContent(showsDoneButton: true)
                .navigationTitle("Media")
                .navigationBarTitleDisplayMode(.inline)
        }
    }
}

struct MediaManagerContent: View {
    @Environment(\.dismiss) private var dismiss
    @Environment(SequencerViewModel.self) private var viewModel

    /// `true` when presented as a standalone sheet (shows a Done
    /// button in the cancellation slot). `false` when embedded in
    /// another sheet (e.g. SequenceSettingsSheet's Media tab) that
    /// already provides its own dismiss.
    var showsDoneButton: Bool = true

    @State private var items: [MediaInventoryItem] = []
    @State private var didLoad = false
    @State private var renameTarget: MediaInventoryItem? = nil
    @State private var showingRemoveUnusedConfirm = false
    @State private var removedCount: Int? = nil

    // E-4 relocation: the row a "Replace from Disk…" swipe just
    // fired on. Drives the `.fileImporter` below — when non-nil
    // the picker is presented and its success callback pipes
    // the picked URL through the bridge.
    @State private var replaceTarget: MediaInventoryItem? = nil
    @State private var replaceError: String? = nil

    private static let typeOrder: [String] = [
        "image", "video", "svg", "shader", "text", "binary"
    ]

    var body: some View {
        Group {
                if didLoad && items.isEmpty {
                    ContentUnavailableView(
                        "No Media in this Sequence",
                        systemImage: "folder",
                        description: Text("Media files referenced by effects show up here after the sequence has been rendered once.")
                    )
                } else {
                    List {
                        if brokenCount > 0 {
                            Section {
                                HStack(spacing: 8) {
                                    Image(systemName: "exclamationmark.triangle.fill")
                                        .foregroundStyle(.red)
                                    Text("\(brokenCount) missing media \(brokenCount == 1 ? "file" : "files"). Effects referencing missing files will render as black until you restore or replace the file.")
                                        .font(.caption)
                                }
                            }
                        }
                        ForEach(Self.typeOrder, id: \.self) { typeKey in
                            let group = items.filter { $0.type == typeKey }
                            if !group.isEmpty {
                                Section {
                                    ForEach(group) { item in
                                        MediaInventoryRow(
                                            item: item,
                                            onEmbed: { embedOne(item) },
                                            onExtract: { extractOne(item) },
                                            onRename: { beginRename(item) },
                                            onReplace: { beginReplace(item) })
                                    }
                                    // Bulk actions on the section
                                    // footer — desktop has Embed All
                                    // / Extract All per type on
                                    // ManageMediaPanel. Only shown
                                    // for embeddable types; even
                                    // then, only when there's
                                    // something to do.
                                    if supportsBulk(typeKey) {
                                        let embeddable = group.filter {
                                            $0.isEmbeddable && !$0.isEmbedded
                                        }
                                        let extractable = group.filter {
                                            $0.isEmbedded
                                        }
                                        if !embeddable.isEmpty {
                                            Button {
                                                bulkEmbed(typeKey)
                                            } label: {
                                                Label("Embed All (\(embeddable.count))",
                                                      systemImage: "tray.and.arrow.down")
                                            }
                                        }
                                        if !extractable.isEmpty {
                                            Button {
                                                bulkExtract(typeKey)
                                            } label: {
                                                Label("Extract All (\(extractable.count))",
                                                      systemImage: "tray.and.arrow.up")
                                            }
                                        }
                                    }
                                } header: {
                                    Text(sectionLabel(typeKey))
                                }
                            }
                        }
                    }
                }
            }
            .toolbar {
                if showsDoneButton {
                    ToolbarItem(placement: .cancellationAction) {
                        Button("Done") { dismiss() }
                    }
                }
                ToolbarItem(placement: .primaryAction) {
                    Menu {
                        Button(role: .destructive) {
                            showingRemoveUnusedConfirm = true
                        } label: {
                            Label("Remove Unused Media…",
                                  systemImage: "trash")
                        }
                        .disabled(items.isEmpty)
                    } label: {
                        Image(systemName: "ellipsis.circle")
                    }
                }
            }
        .onAppear { reload() }
        .sheet(item: $renameTarget) { target in
            MediaRenameSheet(item: target) { newName in
                renameItem(target, newName: newName)
            }
        }
        .confirmationDialog(
            "Remove Unused Media?",
            isPresented: $showingRemoveUnusedConfirm,
            titleVisibility: .visible
        ) {
            Button("Remove Unused", role: .destructive) {
                removeUnused()
            }
            Button("Cancel", role: .cancel) { }
        } message: {
            Text("Media files not referenced by any effect will be dropped from the sequence. Embedded payloads are deleted; external files on disk are not touched.")
        }
        .alert("Removed \(removedCount ?? 0) Unused Media",
               isPresented: Binding(
                get: { removedCount != nil },
                set: { if !$0 { removedCount = nil } }
               )) {
            Button("OK", role: .cancel) { removedCount = nil }
        }
        // E-4 relocation picker. `allowedContentTypes` is broad
        // (`.item`) because the user is telling us this IS the
        // replacement — filtering to the type-derived UTIs would
        // mostly annoy the user whose picked file might have the
        // wrong extension but right contents.
        .fileImporter(
            isPresented: Binding(
                get: { replaceTarget != nil },
                set: { if !$0 { replaceTarget = nil } }
            ),
            allowedContentTypes: [.item]
        ) { result in
            guard let target = replaceTarget else { return }
            replaceTarget = nil
            switch result {
            case .success(let url):
                handleReplacePick(target: target, url: url)
            case .failure:
                // User cancelled or picker errored — leave the
                // entry alone.
                break
            }
        }
        .alert("Replace Failed",
               isPresented: Binding(
                get: { replaceError != nil },
                set: { if !$0 { replaceError = nil } }
               )) {
            Button("OK", role: .cancel) { replaceError = nil }
        } message: {
            Text(replaceError ?? "")
        }
    }

    private var brokenCount: Int {
        items.reduce(0) { $0 + ($1.isBroken ? 1 : 0) }
    }

    private func sectionLabel(_ key: String) -> String {
        switch key {
        case "image":  return "Images"
        case "video":  return "Videos"
        case "svg":    return "SVGs"
        case "shader": return "Shaders"
        case "text":   return "Text Files"
        case "binary": return "Binary Files"
        default:       return key.capitalized
        }
    }

    private func supportsBulk(_ typeKey: String) -> Bool {
        // Un-embeddable types (video / binary) have nothing to bulk
        // on. The manager still lists them for context.
        typeKey != "video" && typeKey != "binary"
    }

    // MARK: - Per-row + bulk embed / extract

    private func embedOne(_ item: MediaInventoryItem) {
        if viewModel.document.embedMedia(atPath: item.path) {
            reload()
        }
    }

    private func extractOne(_ item: MediaInventoryItem) {
        if viewModel.document.extractMedia(atPath: item.path) {
            reload()
        }
    }

    private func bulkEmbed(_ typeKey: String) {
        if viewModel.document.embedAllMedia(ofType: typeKey) > 0 {
            reload()
        }
    }

    private func bulkExtract(_ typeKey: String) {
        if viewModel.document.extractAllMedia(ofType: typeKey) > 0 {
            reload()
        }
    }

    // MARK: - Replace from Disk (E-4 — broken rows only)

    fileprivate func beginReplace(_ item: MediaInventoryItem) {
        replaceTarget = item
    }

    private func handleReplacePick(target: MediaInventoryItem, url: URL) {
        let scoped = url.startAccessingSecurityScopedResource()
        defer {
            if scoped { url.stopAccessingSecurityScopedResource() }
        }
        let stored = target.path
        if let result = viewModel.document.replaceMissingMedia(
            atPath: stored, fromSourcePath: url.path) as String?
        {
            // `result` is the new show-relative path the file now
            // lives at. The bridge has already rewritten every
            // referencing effect; reloading the inventory picks up
            // the status flip (isBroken → false).
            _ = result
            reload()
        } else {
            replaceError = "Could not copy \(url.lastPathComponent) into the show folder. Check free space and try again."
        }
    }

    // MARK: - Rename (embedded only) + remove unused

    fileprivate func beginRename(_ item: MediaInventoryItem) {
        renameTarget = item
    }

    private func renameItem(_ item: MediaInventoryItem, newName: String) {
        let sanitised = sanitiseRename(newName, fromOldPath: item.path)
        guard !sanitised.isEmpty, sanitised != item.path else { return }
        if viewModel.document.renameMedia(
            fromPath: item.path, toPath: sanitised) {
            reload()
        }
    }

    /// Build a target path for a rename. Preserves the original
    /// directory + extension when the user types a bare basename —
    /// "Images/foo.png" + "bar" → "Images/bar.png".
    private func sanitiseRename(_ input: String, fromOldPath old: String) -> String {
        let trimmed = input.trimmingCharacters(in: .whitespaces)
        if trimmed.isEmpty { return "" }
        let oldNS = old as NSString
        let dir = oldNS.deletingLastPathComponent
        let oldExt = oldNS.pathExtension
        let inputNS = trimmed as NSString
        let hasExt = !inputNS.pathExtension.isEmpty
        var filename = trimmed
        if !hasExt && !oldExt.isEmpty {
            filename = "\(trimmed).\(oldExt)"
        }
        if dir.isEmpty { return filename }
        return "\(dir)/\(filename)"
    }

    private func removeUnused() {
        let n = Int(viewModel.document.removeUnusedMedia())
        removedCount = n
        if n > 0 { reload() }
    }

    private func reload() {
        let raw = viewModel.document.mediaInventoryInSequence() ?? []
        items = raw.compactMap { d in
            guard let path = d["path"] as? String,
                  let type = d["type"] as? String else { return nil }
            return MediaInventoryItem(
                path: path,
                type: type,
                resolvedPath: (d["resolvedPath"] as? String) ?? "",
                isEmbedded: (d["isEmbedded"] as? NSNumber)?.boolValue ?? false,
                isBroken:   (d["isBroken"]   as? NSNumber)?.boolValue ?? false,
                widthPx:    (d["widthPx"]    as? NSNumber)?.intValue ?? 0,
                heightPx:   (d["heightPx"]   as? NSNumber)?.intValue ?? 0,
                frameCount: (d["frameCount"] as? NSNumber)?.intValue ?? 0)
        }
        .sorted { lhs, rhs in
            if lhs.type != rhs.type {
                let li = Self.typeOrder.firstIndex(of: lhs.type) ?? 99
                let ri = Self.typeOrder.firstIndex(of: rhs.type) ?? 99
                return li < ri
            }
            return lhs.filename.localizedCaseInsensitiveCompare(rhs.filename) == .orderedAscending
        }
        didLoad = true
    }
}

struct MediaInventoryRow: View {
    @Environment(SequencerViewModel.self) private var viewModel
    let item: MediaInventoryItem
    let onEmbed: () -> Void
    let onExtract: () -> Void
    let onRename: () -> Void
    let onReplace: () -> Void

    var body: some View {
        HStack(spacing: 10) {
            thumbnail
            VStack(alignment: .leading, spacing: 2) {
                Text(item.filename)
                    .font(.callout)
                    .foregroundStyle(item.isBroken ? .red : .primary)
                    .lineLimit(1)
                    .truncationMode(.middle)
                HStack(spacing: 6) {
                    Text(item.statusLabel)
                        .font(.caption2)
                        .foregroundStyle(.white)
                        .padding(.horizontal, 6)
                        .padding(.vertical, 1)
                        .background(
                            Capsule().fill(item.statusColor)
                        )
                    if !item.metaSummary.isEmpty {
                        Text(item.metaSummary)
                            .font(.caption2)
                            .foregroundStyle(.secondary)
                    }
                }
                Text(item.path)
                    .font(.caption2)
                    .foregroundStyle(.secondary)
                    .lineLimit(1)
                    .truncationMode(.middle)
            }
            Spacer(minLength: 0)
        }
        .swipeActions(edge: .trailing) {
            if item.isBroken {
                // Broken entry — only actionable path is to point
                // it at a replacement file on disk (E-4).
                Button {
                    onReplace()
                } label: {
                    Label("Replace from Disk…", systemImage: "arrow.triangle.2.circlepath")
                }
                .tint(.red)
            } else {
                Button {
                    onRename()
                } label: {
                    Label("Rename", systemImage: "pencil")
                }
                .tint(.orange)
                if item.isEmbedded {
                    Button {
                        onExtract()
                    } label: {
                        Label("Extract", systemImage: "tray.and.arrow.up")
                    }
                    .tint(.blue)
                } else if item.isEmbeddable {
                    Button {
                        onEmbed()
                    } label: {
                        Label("Embed", systemImage: "tray.and.arrow.down")
                    }
                    .tint(.purple)
                }
            }
        }
    }

    @ViewBuilder
    private var thumbnail: some View {
        if item.isBroken {
            RoundedRectangle(cornerRadius: 4)
                .fill(Color.red.opacity(0.12))
                .overlay(
                    Image(systemName: "questionmark")
                        .foregroundStyle(.red)
                )
                .frame(width: 40, height: 40)
        } else {
            MediaThumbnailView(path: item.path,
                               mediaType: item.type,
                               size: 40)
        }
    }
}

// MARK: - Rename sheet (embedded)

struct MediaRenameSheet: View {
    @Environment(\.dismiss) private var dismiss
    let item: MediaInventoryItem
    let onRename: (String) -> Void

    @State private var newName: String = ""

    var body: some View {
        NavigationStack {
            Form {
                Section {
                    TextField("New name", text: $newName)
                        .autocorrectionDisabled()
                } header: {
                    Text("Rename")
                } footer: {
                    VStack(alignment: .leading, spacing: 2) {
                        Text("Current: \(item.filename)")
                        Text("Rewrites every effect that references this file.")
                    }
                    .font(.caption2)
                }
            }
            .navigationTitle("Rename")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Rename") {
                        onRename(newName)
                        dismiss()
                    }
                    .disabled(newName.trimmingCharacters(in: .whitespaces).isEmpty)
                }
            }
        }
        .onAppear {
            // Seed with the current filename (without extension)
            // so the user can type a replacement quickly.
            let base = (item.filename as NSString).deletingPathExtension
            newName = base
        }
    }
}
