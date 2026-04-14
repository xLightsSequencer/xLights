import Foundation

/// Persisted show folder + media folder configuration.
///
/// Paths are stored in UserDefaults. Security-scoped bookmarks are managed
/// separately by `ObtainAccessToURL` (in xlAppleUtils), keyed by path. Calling
/// `registerBookmark(for:)` right after the user picks a folder creates/refreshes
/// the bookmark so access survives app restart.
enum FolderConfig {
    private static let showFolderKey = "xl.showFolderPath"
    private static let mediaFoldersKey = "xl.mediaFolderPaths"

    static var showFolder: String? {
        get { UserDefaults.standard.string(forKey: showFolderKey) }
        set {
            if let v = newValue {
                UserDefaults.standard.set(v, forKey: showFolderKey)
            } else {
                UserDefaults.standard.removeObject(forKey: showFolderKey)
            }
        }
    }

    static var mediaFolders: [String] {
        get { UserDefaults.standard.stringArray(forKey: mediaFoldersKey) ?? [] }
        set { UserDefaults.standard.set(newValue, forKey: mediaFoldersKey) }
    }

    /// Create a persistent security-scoped bookmark from a freshly-picked URL.
    ///
    /// On iOS, bookmarks must be created from a URL that carries a live security
    /// scope — i.e. the URL returned by UIDocumentPicker while
    /// `startAccessingSecurityScopedResource()` is active on it. A bookmark
    /// created from a plain `URL(fileURLWithPath:)` will resolve but its scope
    /// won't grant access to the folder contents.
    ///
    /// Writes to the "xLights-Bookmarks" suite so `obtainAccessToURL` (in
    /// xlAppleUtils.swift) can later resolve it at launch.
    static func registerBookmark(from url: URL) {
        let path = url.path
        do {
            let data = try url.bookmarkData(options: [],
                                            includingResourceValuesForKeys: nil,
                                            relativeTo: nil)
            let store = UserDefaults(suiteName: "xLights-Bookmarks") ?? .standard
            store.set(data.base64EncodedString(), forKey: path)
        } catch {
            print("FolderConfig.registerBookmark failed for \(path): \(error)")
        }
    }
}
