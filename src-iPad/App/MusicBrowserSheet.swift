import SwiftUI

/// Theme-13 — sequence / lyric vendor browser (desktop's
/// Tools ▸ Download Sequences/Lyrics, `VendorMusicDialog`).
/// Fetches the music-vendor catalog through `XLMusicCatalog`
/// (shared `src-core/import_export/MusicCatalog`), drills through
/// vendors → sequence/lyric items, and downloads the selected item
/// straight into the show folder (sequences as `.zip`, lyrics as
/// `.xtiming`). Mirrors `VendorBrowserSheet` (the models side).
struct MusicBrowserSheet: View {
    /// Show folder downloads land in. When empty the sheet shows a
    /// "no show folder" notice and the download buttons are off.
    let showFolder: String
    /// Called with the saved local path after a successful download.
    let onDownloaded: (String) -> Void
    @Environment(\.dismiss) private var dismiss

    @State private var catalog: XLMusicCatalog?
    @State private var vendors: [XLMusicVendor] = []
    @State private var loadPercent: Int = 0
    @State private var loadLabel: String = "Loading catalog…"
    @State private var loadError: String? = nil
    @State private var isLoading: Bool = true

    @State private var downloadingItemId: String? = nil
    @State private var downloadError: String? = nil
    @State private var downloadedName: String? = nil

    @State private var vendorSearch: String = ""

    var body: some View {
        NavigationStack {
            Group {
                if isLoading {
                    loadingView
                } else if let err = loadError {
                    errorView(err)
                } else {
                    vendorList
                        .searchable(text: $vendorSearch,
                                    placement: .navigationBarDrawer(displayMode: .always),
                                    prompt: "Search vendors or songs")
                }
            }
            .navigationTitle("Download Sequences / Lyrics")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Close") { dismiss() }
                }
            }
        }
        .alert("Download failed",
               isPresented: Binding(get: { downloadError != nil },
                                    set: { if !$0 { downloadError = nil } })) {
            Button("OK", role: .cancel) { }
        } message: {
            Text(downloadError ?? "")
        }
        .alert("Downloaded",
               isPresented: Binding(get: { downloadedName != nil },
                                    set: { if !$0 { downloadedName = nil } })) {
            Button("OK", role: .cancel) { }
        } message: {
            Text("Saved \(downloadedName ?? "") into the show folder.")
        }
        .task { await startLoad() }
    }

    @MainActor
    private func startLoad() async {
        let c = XLMusicCatalog()
        catalog = c
        nonisolated(unsafe) let catalogRef = c
        await withCheckedContinuation { (continuation: CheckedContinuation<Void, Never>) in
            catalogRef.load(progress: { pct, label in
                MainActor.assumeIsolated {
                    loadPercent = Int(pct)
                    loadLabel = label
                }
            }, completion: { errorMessage in
                MainActor.assumeIsolated {
                    if let err = errorMessage {
                        loadError = err
                    } else {
                        vendors = XLMusicVendor.parseAll(catalogRef.vendors)
                    }
                    isLoading = false
                    continuation.resume()
                }
            })
        }
    }

    // MARK: - Views

    private var loadingView: some View {
        VStack(spacing: 14) {
            ProgressView(value: Double(loadPercent), total: 100)
                .progressViewStyle(.linear)
                .frame(maxWidth: 320)
            Text("\(loadPercent)% · \(loadLabel)")
                .font(.caption)
                .foregroundStyle(.secondary)
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .padding()
    }

    private func errorView(_ message: String) -> some View {
        VStack(spacing: 12) {
            Image(systemName: "wifi.exclamationmark")
                .font(.largeTitle)
                .foregroundStyle(.orange)
            Text("Couldn't load the catalog")
                .font(.headline)
            Text(message)
                .font(.caption)
                .foregroundStyle(.secondary)
                .multilineTextAlignment(.center)
            Button("Try Again") {
                isLoading = true
                loadError = nil
                Task { await startLoad() }
            }
            .buttonStyle(.borderedProminent)
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .padding()
    }

    @ViewBuilder
    private var vendorList: some View {
        let filtered = filteredVendors
        List {
            if showFolder.isEmpty {
                Section {
                    Label("Open a show folder first to download into it.",
                          systemImage: "exclamationmark.triangle")
                        .font(.caption)
                        .foregroundStyle(.orange)
                }
            }
            if filtered.isEmpty && !vendorSearch.isEmpty {
                ContentUnavailableView.search(text: vendorSearch)
            } else {
                ForEach(filtered) { vendor in
                    NavigationLink {
                        MusicVendorDetailView(vendor: vendor,
                                              showFolder: showFolder,
                                              downloadingItemId: $downloadingItemId,
                                              catalog: catalog,
                                              onDownloaded: { path, name in
                                                  downloadedName = name
                                                  onDownloaded(path)
                                              },
                                              onError: { msg in downloadError = msg })
                    } label: {
                        MusicVendorRow(vendor: vendor,
                                       matchedCount: matchedItemCount(in: vendor))
                    }
                }
            }
        }
    }

    private var filteredVendors: [XLMusicVendor] {
        let q = vendorSearch.trimmingCharacters(in: .whitespacesAndNewlines)
        if q.isEmpty { return vendors }
        return vendors.filter { vendor in
            if vendor.name.localizedCaseInsensitiveContains(q) { return true }
            for item in vendor.items {
                if item.title.localizedCaseInsensitiveContains(q) { return true }
                if item.artist.localizedCaseInsensitiveContains(q) { return true }
            }
            return false
        }
    }

    private func matchedItemCount(in vendor: XLMusicVendor) -> Int? {
        let q = vendorSearch.trimmingCharacters(in: .whitespacesAndNewlines)
        if q.isEmpty { return nil }
        let count = vendor.items.reduce(into: 0) { acc, it in
            if it.title.localizedCaseInsensitiveContains(q) ||
               it.artist.localizedCaseInsensitiveContains(q) {
                acc += 1
            }
        }
        return count > 0 ? count : nil
    }
}

// MARK: - Vendor Detail

private struct MusicVendorDetailView: View {
    let vendor: XLMusicVendor
    let showFolder: String
    @Binding var downloadingItemId: String?
    let catalog: XLMusicCatalog?
    let onDownloaded: (String, String) -> Void
    let onError: (String) -> Void

    @State private var itemSearch: String = ""

    var body: some View {
        let filtered = filteredItems
        List {
            if !vendor.contact.isEmpty || !vendor.website.isEmpty || !vendor.notes.isEmpty {
                Section("About") {
                    if !vendor.contact.isEmpty {
                        LabeledContent("Contact", value: vendor.contact)
                    }
                    if !vendor.website.isEmpty, let url = URL(string: vendor.website) {
                        Link(destination: url) { Label("Website", systemImage: "globe") }
                    }
                    if !vendor.notes.isEmpty {
                        Text(vendor.notes).font(.caption).foregroundStyle(.secondary)
                    }
                }
            }
            let header = itemSearch.isEmpty
                ? "Items (\(vendor.items.count))"
                : "Items (\(filtered.count) of \(vendor.items.count))"
            Section(header) {
                if vendor.items.isEmpty {
                    Text("This vendor's catalog is empty or hidden.")
                        .font(.caption).foregroundStyle(.secondary)
                } else if filtered.isEmpty {
                    ContentUnavailableView.search(text: itemSearch)
                }
                ForEach(filtered) { item in
                    MusicItemRow(item: item,
                                 canDownload: !showFolder.isEmpty && !item.download.isEmpty,
                                 isDownloading: downloadingItemId == item.id,
                                 onTap: { download(item) })
                }
            }
        }
        .navigationTitle(vendor.name)
        .navigationBarTitleDisplayMode(.inline)
        .searchable(text: $itemSearch,
                    placement: .navigationBarDrawer(displayMode: .always),
                    prompt: "Search \(vendor.name)")
    }

    private var filteredItems: [XLMusicItem] {
        let q = itemSearch.trimmingCharacters(in: .whitespacesAndNewlines)
        if q.isEmpty { return vendor.items }
        return vendor.items.filter { it in
            it.title.localizedCaseInsensitiveContains(q) ||
            it.artist.localizedCaseInsensitiveContains(q) ||
            it.creator.localizedCaseInsensitiveContains(q) ||
            it.typeName.localizedCaseInsensitiveContains(q)
        }
    }

    private func download(_ item: XLMusicItem) {
        guard !item.download.isEmpty else {
            onError("This item doesn't include a download link.")
            return
        }
        guard !showFolder.isEmpty else {
            onError("Open a show folder before downloading.")
            return
        }
        guard downloadingItemId == nil else { return }
        downloadingItemId = item.id
        nonisolated(unsafe) let onDownloaded = self.onDownloaded
        nonisolated(unsafe) let onError = self.onError
        let name = item.fileName
        catalog?.downloadItem(fromURL: item.download,
                              fileName: name,
                              destFolder: showFolder) { localPath, errorMessage in
            MainActor.assumeIsolated {
                downloadingItemId = nil
                if let path = localPath {
                    onDownloaded(path, name)
                } else if let err = errorMessage {
                    onError(err)
                }
            }
        }
    }
}

// MARK: - Row views

private struct MusicVendorRow: View {
    let vendor: XLMusicVendor
    let matchedCount: Int?

    var body: some View {
        HStack(spacing: 10) {
            Image(systemName: "music.note.list")
                .foregroundStyle(.secondary)
                .frame(width: 28)
            VStack(alignment: .leading) {
                Text(vendor.name).font(.body.weight(.medium))
                if vendor.items.isEmpty {
                    Text("Empty").font(.caption2).foregroundStyle(.orange)
                } else if let matched = matchedCount, matched > 0 {
                    Text("\(matched) of \(vendor.items.count) match")
                        .font(.caption).foregroundStyle(Color.accentColor)
                } else {
                    Text("\(vendor.items.count) items")
                        .font(.caption).foregroundStyle(.secondary)
                }
            }
        }
    }
}

private struct MusicItemRow: View {
    let item: XLMusicItem
    let canDownload: Bool
    let isDownloading: Bool
    let onTap: () -> Void

    var body: some View {
        Button(action: onTap) {
            HStack(spacing: 10) {
                Image(systemName: item.typeName == "Lyrics" ? "text.quote" : "waveform")
                    .foregroundStyle(.secondary)
                    .frame(width: 28)
                VStack(alignment: .leading) {
                    Text(item.title.isEmpty ? "Untitled" : item.title)
                        .font(.body).foregroundStyle(.primary)
                    HStack(spacing: 6) {
                        Text(item.typeName).font(.caption2)
                            .padding(.horizontal, 5).padding(.vertical, 1)
                            .background(Color.secondary.opacity(0.15))
                            .clipShape(Capsule())
                        if !item.artist.isEmpty {
                            Text(item.artist).font(.caption).foregroundStyle(.secondary)
                        }
                    }
                }
                Spacer()
                if isDownloading {
                    ProgressView()
                } else if item.download.isEmpty {
                    Image(systemName: "xmark.octagon").foregroundStyle(.orange)
                } else {
                    Image(systemName: "arrow.down.circle")
                        .foregroundStyle(canDownload ? Color.accentColor : Color.secondary)
                }
            }
        }
        .buttonStyle(.plain)
        .disabled(!canDownload || isDownloading)
    }
}

// MARK: - Models

struct XLMusicVendor: Identifiable, Hashable {
    var id: String { name }
    var name: String
    var contact: String
    var email: String
    var phone: String
    var website: String
    var facebook: String
    var twitter: String
    var notes: String
    var logoFile: String
    var items: [XLMusicItem]

    static func parseAll(_ raw: [[AnyHashable: Any]]) -> [XLMusicVendor] {
        return raw.map { dict in
            let itemsRaw = (dict["items"] as? [[AnyHashable: Any]]) ?? []
            return XLMusicVendor(
                name:     (dict["name"]     as? String) ?? "",
                contact:  (dict["contact"]  as? String) ?? "",
                email:    (dict["email"]    as? String) ?? "",
                phone:    (dict["phone"]    as? String) ?? "",
                website:  (dict["website"]  as? String) ?? "",
                facebook: (dict["facebook"] as? String) ?? "",
                twitter:  (dict["twitter"]  as? String) ?? "",
                notes:    (dict["notes"]    as? String) ?? "",
                logoFile: (dict["logoFile"] as? String) ?? "",
                items:    itemsRaw.enumerated().map { XLMusicItem($1, index: $0) })
        }.sorted { $0.name.localizedCaseInsensitiveCompare($1.name) == .orderedAscending }
    }
}

struct XLMusicItem: Identifiable, Hashable {
    var id: String
    var title: String
    var artist: String
    var creator: String
    var notes: String
    var typeName: String
    var webpage: String
    var download: String
    var video: String
    var music: String
    var ext: String
    var fileName: String

    init(_ dict: [AnyHashable: Any], index: Int) {
        title    = (dict["title"]    as? String) ?? ""
        artist   = (dict["artist"]   as? String) ?? ""
        creator  = (dict["creator"]  as? String) ?? ""
        notes    = (dict["notes"]    as? String) ?? ""
        typeName = (dict["typeName"] as? String) ?? ""
        webpage  = (dict["webpage"]  as? String) ?? ""
        download = (dict["download"] as? String) ?? ""
        video    = (dict["video"]    as? String) ?? ""
        music    = (dict["music"]    as? String) ?? ""
        ext      = (dict["ext"]      as? String) ?? ""
        fileName = (dict["fileName"] as? String) ?? ""
        id = (title.isEmpty ? "item" : title) + "#\(index)"
    }
}
