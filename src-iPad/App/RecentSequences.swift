import Foundation

// E-5 — recent sequences list. Persists the last N opened `.xsq`
// file paths in UserDefaults so they can be surfaced at launch
// (and later, via the File → Open Recent menu in Phase F-4).
//
// Stored as plain path strings, not as security-scoped bookmarks:
// `ObtainAccessToURL` already maintains bookmarks for every
// folder the user has granted access to, so recording a bookmark
// alongside the path would be redundant. When the user taps a
// recent entry, the open flow goes through the normal
// `ObtainAccessToURL` machinery which resolves the stored
// bookmark for the show folder containing the sequence.
//
// Entries pointing to files that no longer exist on disk are
// pruned (and persisted-back) on every `load()`. iCloud-evicted
// files are NOT pruned: their on-device placeholder still
// satisfies `fileExists`, so notDownloaded entries keep
// appearing and the user can tap to trigger a download.
enum RecentSequences {
    private static let key = "xLights.recentSequences"
    static let maxEntries = 12

    struct Entry: Identifiable, Hashable, Codable {
        let path: String
        let lastOpened: Date

        var id: String { path }

        /// Basename without the `.xsq` extension for display.
        var displayName: String {
            let base = (path as NSString).lastPathComponent
            return (base as NSString).deletingPathExtension
        }

        /// Parent directory of the sequence — useful when two
        /// sequences share a name across different show folders.
        var parentFolder: String {
            (path as NSString).deletingLastPathComponent
        }
    }

    /// Load the full recent list (most recent first). Drops any
    /// entries whose underlying file no longer exists on disk and
    /// persists the trimmed list so subsequent reads are clean.
    static func load() -> [Entry] {
        guard let data = UserDefaults.standard.data(forKey: key),
              let entries = try? JSONDecoder().decode([Entry].self, from: data)
        else { return [] }
        let valid = entries.filter {
            FileManager.default.fileExists(atPath: $0.path)
        }
        if valid.count != entries.count {
            save(valid)
        }
        return valid.sorted { $0.lastOpened > $1.lastOpened }
    }

    /// Push a path to the top of the list. De-duplicates by path;
    /// trims to `maxEntries`.
    static func record(path: String) {
        guard !path.isEmpty else { return }
        var all = load().filter { $0.path != path }
        all.insert(Entry(path: path, lastOpened: Date()), at: 0)
        if all.count > maxEntries {
            all.removeSubrange(maxEntries..<all.count)
        }
        save(all)
    }

    /// Remove a single entry (e.g. via swipe-to-delete).
    static func remove(path: String) {
        let all = load().filter { $0.path != path }
        save(all)
    }

    /// Wipe the whole list — surfaced as a menu action on an
    /// empty-state screen so the user can start fresh.
    static func clear() {
        UserDefaults.standard.removeObject(forKey: key)
    }

    private static func save(_ entries: [Entry]) {
        guard let data = try? JSONEncoder().encode(entries) else { return }
        UserDefaults.standard.set(data, forKey: key)
    }
}
