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
    private static let fseqEnabledKey = "xl.fseqEnabled"
    private static let fseqFolderKey = "xl.fseqFolderPath"
    private static let pendingBaseDirReselectKey = "xl.pendingBaseDirReselectMessage"

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

    /// When true, the iPad app reads/writes .fseq files alongside the .xsq.
    /// Default is false so existing users see no behavior change.
    static var fseqEnabled: Bool {
        get { UserDefaults.standard.bool(forKey: fseqEnabledKey) }
        set { UserDefaults.standard.set(newValue, forKey: fseqEnabledKey) }
    }

    /// Optional dedicated folder for .fseq files. If nil/empty, fseqs are
    /// written next to the .xsq (so subfolders of the show folder are honored
    /// automatically). When set, fseqs are flattened into this folder using
    /// just the sequence basename — matching desktop's wxFileName::SetPath rule.
    static var fseqFolder: String? {
        get {
            let s = UserDefaults.standard.string(forKey: fseqFolderKey)
            return (s?.isEmpty ?? true) ? nil : s
        }
        set {
            if let v = newValue, !v.isEmpty {
                UserDefaults.standard.set(v, forKey: fseqFolderKey)
            } else {
                UserDefaults.standard.removeObject(forKey: fseqFolderKey)
            }
        }
    }

    /// Pending message from a silent auto-update-on-open run that
    /// couldn't access the base show folder. FolderConfigView reads
    /// this on appear so the user has a one-tap path to reselect.
    /// Cleared after the user is shown the prompt — a fresh failure
    /// will repopulate it on the next show-folder open.
    static var pendingBaseDirReselectMessage: String? {
        get { UserDefaults.standard.string(forKey: pendingBaseDirReselectKey) }
        set {
            if let v = newValue, !v.isEmpty {
                UserDefaults.standard.set(v, forKey: pendingBaseDirReselectKey)
            } else {
                UserDefaults.standard.removeObject(forKey: pendingBaseDirReselectKey)
            }
        }
    }

    /// Resolve the on-disk FSEQ path for a sequence, applying the FSEQ
    /// folder rule:
    ///   - feature disabled → nil (caller should not read/write fseq)
    ///   - no fseq folder configured → next to the .xsq (so subfolders of
    ///     the show folder are honored automatically)
    ///   - fseq folder configured → flatten into that folder using just the
    ///     sequence basename (matches desktop's `wxFileName::SetPath`).
    /// `xsqPath` must be a path to a `.xsq` file; the extension is replaced
    /// with `.fseq` regardless of input casing.
    static func fseqPath(forXsq xsqPath: String) -> String? {
        guard fseqEnabled, !xsqPath.isEmpty else { return nil }
        let xsqURL = URL(fileURLWithPath: xsqPath)
        let basename = xsqURL.deletingPathExtension().lastPathComponent + ".fseq"
        if let folder = fseqFolder {
            return URL(fileURLWithPath: folder).appendingPathComponent(basename).path
        }
        return xsqURL.deletingLastPathComponent().appendingPathComponent(basename).path
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
