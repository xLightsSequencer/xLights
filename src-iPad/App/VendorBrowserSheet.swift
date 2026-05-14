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

    var body: some View {
        NavigationStack {
            Group {
                if isLoading {
                    loadingView
                } else if let err = loadError {
                    errorView(err)
                } else {
                    vendorList
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
        await withCheckedContinuation { (continuation: CheckedContinuation<Void, Never>) in
            c.load(progress: { pct, label in
                loadPercent = Int(pct)
                loadLabel = label
            }, completion: { errorMessage in
                if let err = errorMessage {
                    loadError = err
                } else {
                    vendors = XLVendor.parseAll(c.vendors)
                }
                isLoading = false
                continuation.resume()
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
        List(vendors) { vendor in
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
                VendorRow(vendor: vendor, catalog: catalog)
            }
        }
    }
}

// MARK: - Vendor Detail

private struct VendorDetailView: View {
    let vendor: XLVendor
    @Binding var downloadingWiringId: String?
    let catalog: XLVendorCatalog?
    let onDownloaded: (String) -> Void
    let onError: (String) -> Void

    var body: some View {
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
            Section("Models (\(vendor.models.count))") {
                if vendor.models.isEmpty {
                    Text("This vendor's inventory is empty or hidden.")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
                ForEach(vendor.models) { model in
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
        catalog?.downloadWiringXmodel(
            fromURL: wiring.xmodelLink,
            pixelDescription: model.pixelDescription,
            pixelSpacing: model.pixelSpacing,
            pixelCount: model.pixelCount,
            widthMM: wiring.modelWidthMM,
            heightMM: wiring.modelHeightMM,
            depthMM: wiring.modelDepthMM) { localPath, errorMessage in
                downloadingWiringId = nil
                if let path = localPath {
                    onDownloaded(path)
                } else if let err = errorMessage {
                    onError(err)
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
