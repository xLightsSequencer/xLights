import SwiftUI

/// Phase J-4 (model download) — vendor catalog browser. Sheet
/// presented from the Layout Editor's canvas overlay. Fetches the
/// xlights.org vendor index through `XLVendorCatalog` (shared
/// `src-core/import_export/VendorCatalog`), then drills through
/// vendors → models → wirings. Tapping a wiring downloads its
/// `.xmodel` and hands the local path back via `onDownloaded` so
/// the editor can flip into "tap canvas to place imported model"
/// mode.
struct VendorBrowserSheet: View {
    let onDownloaded: (String) -> Void   // local xmodel path
    @Environment(\.dismiss) private var dismiss

    @State private var catalog: XLVendorCatalog?
    @State private var vendors: [XLVendor] = []
    @State private var loadPercent: Int = 0
    @State private var loadLabel: String = "Loading catalog…"
    @State private var loadError: String? = nil
    @State private var isLoading: Bool = true

    @State private var downloadingWiring: String? = nil
    @State private var downloadError: String? = nil

    /// J-4 (download) — vendor-list search text. Filters by
    /// vendor name AND by any model name / type inside the
    /// vendor, so a search like "icicle" surfaces vendors whose
    /// names don't mention icicles but who carry icicle models.
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
                                    prompt: "Search vendors or models")
                }
            }
            .navigationTitle("Download Model")
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
        .task {
            await startLoad()
        }
    }

    @MainActor
    private func startLoad() async {
        let c = XLVendorCatalog()
        catalog = c
        // The bridge (XLVendorCatalog.mm) dispatches both progress
        // and completion blocks to the main queue, but the imported
        // Objective-C blocks are `@Sendable` from Swift's view —
        // hop back onto the MainActor explicitly so mutating @State
        // and reading `c` (a non-Sendable class) is well-formed.
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
                        vendors = XLVendor.parseAll(catalogRef.vendors)
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
            if filtered.isEmpty && !vendorSearch.isEmpty {
                ContentUnavailableView.search(text: vendorSearch)
            } else {
                ForEach(filtered) { vendor in
                    NavigationLink {
                        VendorDetailView(vendor: vendor,
                                          downloadingWiringId: $downloadingWiring,
                                          catalog: catalog,
                                          onDownloaded: { path in
                                              onDownloaded(path)
                                              dismiss()
                                          },
                                          onError: { msg in
                                              downloadError = msg
                                          })
                    } label: {
                        VendorRow(vendor: vendor, catalog: catalog,
                                  matchedModelCount: matchedModelCount(in: vendor))
                    }
                }
            }
        }
    }

    /// Vendors filtered by `vendorSearch`. A vendor matches when
    /// either its own name contains the query, OR any of its
    /// models' names / types do — so searching "icicle" surfaces
    /// vendors whose names don't mention icicles but carry them.
    private var filteredVendors: [XLVendor] {
        let q = vendorSearch.trimmingCharacters(in: .whitespacesAndNewlines)
        if q.isEmpty { return vendors }
        return vendors.filter { vendor in
            if vendor.name.localizedCaseInsensitiveContains(q) { return true }
            for model in vendor.models {
                if model.name.localizedCaseInsensitiveContains(q) { return true }
                if model.type.localizedCaseInsensitiveContains(q) { return true }
            }
            return false
        }
    }

    /// Count of models inside `vendor` that match the active
    /// search query. Used by `VendorRow` to show a "N match" hint
    /// alongside the total. Zero when the vendor matched purely
    /// on its own name (or search is empty).
    private func matchedModelCount(in vendor: XLVendor) -> Int? {
        let q = vendorSearch.trimmingCharacters(in: .whitespacesAndNewlines)
        if q.isEmpty { return nil }
        let count = vendor.models.reduce(into: 0) { acc, m in
            if m.name.localizedCaseInsensitiveContains(q) ||
               m.type.localizedCaseInsensitiveContains(q) {
                acc += 1
            }
        }
        return count > 0 ? count : nil
    }
}

// MARK: - Vendor Detail

private struct VendorDetailView: View {
    let vendor: XLVendor
    @Binding var downloadingWiringId: String?
    let catalog: XLVendorCatalog?
    let onDownloaded: (String) -> Void
    let onError: (String) -> Void

    @State private var modelSearch: String = ""

    var body: some View {
        let filtered = filteredModels
        List {
            if !vendor.contact.isEmpty || !vendor.website.isEmpty || !vendor.notes.isEmpty {
                Section("About") {
                    if !vendor.contact.isEmpty {
                        LabeledContent("Contact", value: vendor.contact)
                    }
                    if !vendor.website.isEmpty, let url = URL(string: vendor.website) {
                        Link(destination: url) {
                            Label("Website", systemImage: "globe")
                        }
                    }
                    if !vendor.notes.isEmpty {
                        Text(vendor.notes)
                            .font(.caption)
                            .foregroundStyle(.secondary)
                    }
                }
            }
            let header = modelSearch.isEmpty
                ? "Models (\(vendor.models.count))"
                : "Models (\(filtered.count) of \(vendor.models.count))"
            Section(header) {
                if vendor.models.isEmpty {
                    Text("This vendor's inventory is empty or hidden.")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                } else if filtered.isEmpty {
                    ContentUnavailableView.search(text: modelSearch)
                }
                ForEach(filtered) { model in
                    NavigationLink {
                        ModelDetailView(model: model,
                                          downloadingWiringId: $downloadingWiringId,
                                          catalog: catalog,
                                          onDownloaded: onDownloaded,
                                          onError: onError)
                    } label: {
                        ModelRow(model: model, catalog: catalog)
                    }
                }
            }
        }
        .navigationTitle(vendor.name)
        .navigationBarTitleDisplayMode(.inline)
        .searchable(text: $modelSearch,
                    placement: .navigationBarDrawer(displayMode: .always),
                    prompt: "Search \(vendor.name) models")
    }

    /// Models filtered by `modelSearch`. Matches name OR type
    /// OR pixel description (case-insensitive contains) so a
    /// search like "16" surfaces 16-pixel matrices and a search
    /// like "arch" surfaces every arch regardless of vendor name.
    private var filteredModels: [XLVendorModel] {
        let q = modelSearch.trimmingCharacters(in: .whitespacesAndNewlines)
        if q.isEmpty { return vendor.models }
        return vendor.models.filter { m in
            m.name.localizedCaseInsensitiveContains(q) ||
            m.type.localizedCaseInsensitiveContains(q) ||
            m.pixelDescription.localizedCaseInsensitiveContains(q) ||
            m.notes.localizedCaseInsensitiveContains(q)
        }
    }
}

// MARK: - Model Detail

private struct ModelDetailView: View {
    let model: XLVendorModel
    @Binding var downloadingWiringId: String?
    let catalog: XLVendorCatalog?
    let onDownloaded: (String) -> Void
    let onError: (String) -> Void

    var body: some View {
        List {
            if !model.imageURLs.isEmpty || !model.imageFiles.isEmpty {
                Section {
                    HStack {
                        Spacer()
                        VendorImageView(
                            urls: model.imageURLs,
                            bundledFiles: model.imageFiles,
                            placeholderSystemName: "lightbulb",
                            dimension: 240,
                            catalog: catalog)
                        Spacer()
                    }
                    .listRowBackground(Color.clear)
                }
            }
            Section("Details") {
                if !model.type.isEmpty {
                    LabeledContent("Type", value: model.type)
                }
                if !model.width.isEmpty {
                    LabeledContent("Width", value: model.width)
                }
                if !model.height.isEmpty {
                    LabeledContent("Height", value: model.height)
                }
                if !model.depth.isEmpty {
                    LabeledContent("Depth", value: model.depth)
                }
                if !model.pixelCount.isEmpty {
                    LabeledContent("Pixel Count", value: model.pixelCount)
                }
                if !model.pixelDescription.isEmpty {
                    LabeledContent("Pixels", value: model.pixelDescription)
                }
                if !model.material.isEmpty {
                    LabeledContent("Material", value: model.material)
                }
                if !model.webpage.isEmpty, let url = URL(string: model.webpage) {
                    Link(destination: url) {
                        Label("Vendor product page", systemImage: "safari")
                    }
                }
                if !model.notes.isEmpty {
                    Text(model.notes)
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
            }
            Section("Wirings (\(model.wirings.count))") {
                if model.wirings.isEmpty {
                    Text("No downloadable wiring options for this model.")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
                ForEach(model.wirings) { wiring in
                    WiringRow(wiring: wiring,
                              catalog: catalog,
                              isDownloading: downloadingWiringId == wiring.id,
                              onTap: { download(wiring: wiring) })
                }
            }
        }
        .navigationTitle(model.name)
        .navigationBarTitleDisplayMode(.inline)
    }

    private func download(wiring: XLVendorWiring) {
        guard !wiring.xmodelLink.isEmpty else {
            onError("This wiring doesn't include a downloadable file yet.")
            return
        }
        guard downloadingWiringId == nil else { return }
        downloadingWiringId = wiring.id
        // Bridge dispatches completion to the main queue; hop back
        // onto the MainActor so mutating the binding and invoking
        // the parent's @MainActor-isolated callbacks is well-formed
        // under Swift 6 strict concurrency.
        nonisolated(unsafe) let onDownloaded = self.onDownloaded
        nonisolated(unsafe) let onError = self.onError
        catalog?.downloadWiringXmodel(
            fromURL: wiring.xmodelLink,
            pixelDescription: model.pixelDescription,
            pixelSpacing: model.pixelSpacing,
            pixelCount: model.pixelCount,
            widthMM: wiring.modelWidthMM,
            heightMM: wiring.modelHeightMM,
            depthMM: wiring.modelDepthMM) { localPath, errorMessage in
                MainActor.assumeIsolated {
                    downloadingWiringId = nil
                    if let path = localPath {
                        onDownloaded(path)
                    } else if let err = errorMessage {
                        onError(err)
                    }
                }
            }
    }
}

// MARK: - Lazy image cache

/// Phase J-4 (download) — process-wide image cache for the vendor
/// browser. The on-disk side is `CachedFileDownloader`'s long-
/// lived cache; this in-memory layer dedupes UIImage decode work
/// across SwiftUI list cells (rows scroll out and back; we don't
/// want to decode the same PNG 8 times). NSCache trims under
/// memory pressure, so it's safe to keep "forever". Keyed by URL.
@MainActor
final class VendorImageCache {
    static let shared = VendorImageCache()
    private let cache = NSCache<NSString, UIImage>()
    init() { cache.countLimit = 256 }

    func image(for url: String) -> UIImage? {
        cache.object(forKey: url as NSString)
    }
    func store(_ image: UIImage, for url: String) {
        cache.setObject(image, forKey: url as NSString)
    }
}

/// Lazily-loaded image backed by the catalog's image-URL list.
/// Tries each URL in order, picking the first that decodes to a
/// usable UIImage. Hits the in-memory `VendorImageCache` first,
/// then falls through to the bridge's `fetchImage(fromURL:)`
/// which uses the shared on-disk cache.
struct VendorImageView: View {
    let urls: [String]
    /// Optional bundled local files from the initial catalog
    /// dictionary (vendor logos arrive this way). Tried before
    /// any network fetch.
    let bundledFiles: [String]
    let placeholderSystemName: String
    let dimension: CGFloat
    let catalog: XLVendorCatalog?

    @State private var image: UIImage? = nil
    @State private var isLoading: Bool = false

    init(urls: [String] = [],
         bundledFiles: [String] = [],
         placeholderSystemName: String,
         dimension: CGFloat,
         catalog: XLVendorCatalog?) {
        self.urls = urls
        self.bundledFiles = bundledFiles
        self.placeholderSystemName = placeholderSystemName
        self.dimension = dimension
        self.catalog = catalog
    }

    var body: some View {
        Group {
            if let image {
                Image(uiImage: image)
                    .resizable()
                    .scaledToFit()
            } else if isLoading {
                ProgressView()
                    .controlSize(.small)
            } else {
                Image(systemName: placeholderSystemName)
                    .foregroundStyle(.secondary)
            }
        }
        .frame(width: dimension, height: dimension)
        .clipShape(RoundedRectangle(cornerRadius: 4))
        .task { await load() }
    }

    @MainActor
    private func load() async {
        // Already loaded for this view? Don't refire.
        if image != nil { return }
        // Bundled local files — vendor logos eagerly downloaded
        // by the catalog loader.
        for path in bundledFiles {
            if FileManager.default.fileExists(atPath: path),
               let img = UIImage(contentsOfFile: path) {
                image = img
                return
            }
        }
        // URLs — try the in-memory cache first, then the bridge's
        // disk-cached fetch.
        for url in urls where !url.isEmpty {
            if let cached = VendorImageCache.shared.image(for: url) {
                image = cached
                return
            }
            isLoading = true
            let path = await fetch(url: url)
            isLoading = false
            if let path,
               let img = UIImage(contentsOfFile: path) {
                VendorImageCache.shared.store(img, for: url)
                image = img
                return
            }
        }
    }

    private func fetch(url: String) async -> String? {
        guard let catalog else { return nil }
        return await withCheckedContinuation { (cont: CheckedContinuation<String?, Never>) in
            catalog.fetchImage(fromURL: url) { path in
                cont.resume(returning: path)
            }
        }
    }
}

// MARK: - Row views

private struct VendorRow: View {
    let vendor: XLVendor
    let catalog: XLVendorCatalog?
    /// Non-nil when the parent's search query is active. Number
    /// of models inside this vendor that match the query, or 0
    /// when the vendor was included purely on its own name.
    let matchedModelCount: Int?

    var body: some View {
        HStack(spacing: 10) {
            VendorImageView(
                bundledFiles: vendor.logoFile.isEmpty ? [] : [vendor.logoFile],
                placeholderSystemName: "shippingbox",
                dimension: 40,
                catalog: catalog)
            VStack(alignment: .leading) {
                Text(vendor.name).font(.body.weight(.medium))
                if vendor.models.isEmpty {
                    Text("Hidden")
                        .font(.caption2)
                        .foregroundStyle(.orange)
                } else if let matched = matchedModelCount, matched > 0 {
                    Text("\(matched) of \(vendor.models.count) match")
                        .font(.caption)
                        .foregroundStyle(Color.accentColor)
                } else {
                    Text("\(vendor.models.count) models")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
            }
        }
    }
}

private struct ModelRow: View {
    let model: XLVendorModel
    let catalog: XLVendorCatalog?
    var body: some View {
        HStack(spacing: 10) {
            VendorImageView(
                urls: model.imageURLs,
                bundledFiles: model.imageFiles,
                placeholderSystemName: "lightbulb",
                dimension: 44,
                catalog: catalog)
            VStack(alignment: .leading) {
                Text(model.name).font(.body)
                let dim = model.dimensionSummary
                if !dim.isEmpty {
                    Text(dim)
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
            }
            Spacer()
            if !model.hasDownload {
                Image(systemName: "exclamationmark.triangle")
                    .foregroundStyle(.orange)
                    .help("No downloadable wiring")
            }
        }
    }
}

private struct WiringRow: View {
    let wiring: XLVendorWiring
    let catalog: XLVendorCatalog?
    let isDownloading: Bool
    let onTap: () -> Void

    var body: some View {
        Button(action: onTap) {
            HStack(spacing: 10) {
                VendorImageView(
                    urls: wiring.imageURLs,
                    bundledFiles: wiring.imageFiles,
                    placeholderSystemName: "cable.connector",
                    dimension: 44,
                    catalog: catalog)
                VStack(alignment: .leading) {
                    Text(wiring.name.isEmpty ? "Default wiring" : wiring.name)
                        .font(.body)
                        .foregroundStyle(.primary)
                    if !wiring.wiringDescription.isEmpty {
                        Text(wiring.wiringDescription)
                            .font(.caption)
                            .foregroundStyle(.secondary)
                            .lineLimit(3)
                    }
                }
                Spacer()
                if isDownloading {
                    ProgressView()
                } else if wiring.xmodelLink.isEmpty {
                    Image(systemName: "xmark.octagon")
                        .foregroundStyle(.orange)
                } else {
                    Image(systemName: "arrow.down.circle")
                        .foregroundStyle(Color.accentColor)
                }
            }
        }
        .buttonStyle(.plain)
        .disabled(wiring.xmodelLink.isEmpty || isDownloading)
    }
}

// MARK: - Models

struct XLVendor: Identifiable, Hashable {
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
    var maxModels: Int
    var models: [XLVendorModel]

    static func parseAll(_ raw: [[AnyHashable: Any]]) -> [XLVendor] {
        return raw.map { dict in
            let modelsRaw = (dict["models"] as? [[AnyHashable: Any]]) ?? []
            return XLVendor(
                name:      (dict["name"]      as? String) ?? "",
                contact:   (dict["contact"]   as? String) ?? "",
                email:     (dict["email"]     as? String) ?? "",
                phone:     (dict["phone"]     as? String) ?? "",
                website:   (dict["website"]   as? String) ?? "",
                facebook:  (dict["facebook"]  as? String) ?? "",
                twitter:   (dict["twitter"]   as? String) ?? "",
                notes:     (dict["notes"]     as? String) ?? "",
                logoFile:  (dict["logoFile"]  as? String) ?? "",
                maxModels: (dict["maxModels"] as? NSNumber)?.intValue ?? -1,
                models:    modelsRaw.map(XLVendorModel.init))
        }.sorted { $0.name.localizedCaseInsensitiveCompare($1.name) == .orderedAscending }
    }
}

struct XLVendorModel: Identifiable, Hashable {
    var id: String { idValue.isEmpty ? name : idValue }
    var idValue: String
    var name: String
    var type: String
    var material: String
    var thickness: String
    var width: String
    var height: String
    var depth: String
    var pixelCount: String
    var pixelSpacing: String
    var pixelDescription: String
    var webpage: String
    var notes: String
    var imageURLs: [String]
    var imageFiles: [String]
    var wirings: [XLVendorWiring]

    init(_ dict: [AnyHashable: Any]) {
        idValue          = (dict["id"]               as? String) ?? ""
        name             = (dict["name"]             as? String) ?? ""
        type             = (dict["type"]             as? String) ?? ""
        material         = (dict["material"]         as? String) ?? ""
        thickness        = (dict["thickness"]        as? String) ?? ""
        width            = (dict["width"]            as? String) ?? ""
        height           = (dict["height"]           as? String) ?? ""
        depth            = (dict["depth"]            as? String) ?? ""
        pixelCount       = (dict["pixelCount"]       as? String) ?? ""
        pixelSpacing     = (dict["pixelSpacing"]     as? String) ?? ""
        pixelDescription = (dict["pixelDescription"] as? String) ?? ""
        webpage          = (dict["webpage"]          as? String) ?? ""
        notes            = (dict["notes"]            as? String) ?? ""
        imageURLs        = (dict["imageURLs"]        as? [String]) ?? []
        imageFiles       = (dict["imageFiles"]       as? [String]) ?? []
        let wirings      = (dict["wirings"]          as? [[AnyHashable: Any]]) ?? []
        self.wirings     = wirings.enumerated().map { XLVendorWiring($1, index: $0) }
    }

    var dimensionSummary: String {
        var parts: [String] = []
        if !width.isEmpty { parts.append("W \(width)") }
        if !height.isEmpty { parts.append("H \(height)") }
        if !depth.isEmpty { parts.append("D \(depth)") }
        if !pixelCount.isEmpty { parts.append("\(pixelCount)px") }
        return parts.joined(separator: " · ")
    }

    var hasDownload: Bool {
        wirings.contains { !$0.xmodelLink.isEmpty }
    }
}

struct XLVendorWiring: Identifiable, Hashable {
    var id: String                       // synthesized: "modelId#index"
    var name: String
    var wiringDescription: String
    var xmodelLink: String
    var imageURLs: [String]
    var imageFiles: [String]
    var modelWidthMM: Int
    var modelHeightMM: Int
    var modelDepthMM: Int

    init(_ dict: [AnyHashable: Any], index: Int) {
        id                  = ((dict["name"] as? String) ?? "") + "#\(index)"
        name                = (dict["name"]                as? String) ?? ""
        wiringDescription   = (dict["wiringDescription"]   as? String) ?? ""
        xmodelLink          = (dict["xmodelLink"]          as? String) ?? ""
        imageURLs           = (dict["imageURLs"]           as? [String]) ?? []
        imageFiles          = (dict["imageFiles"]          as? [String]) ?? []
        modelWidthMM        = (dict["modelWidthMM"]        as? NSNumber)?.intValue ?? -1
        modelHeightMM       = (dict["modelHeightMM"]       as? NSNumber)?.intValue ?? -1
        modelDepthMM        = (dict["modelDepthMM"]        as? NSNumber)?.intValue ?? -1
    }
}
