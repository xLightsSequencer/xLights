import Foundation

// E-5 — recent sequences list, scoped per show folder. Persists the
// last N opened `.xsq` paths in UserDefaults so they can be surfaced
// when the picker is on screen for a given show.
//
// Stored as plain path strings, not as security-scoped bookmarks:
// `ObtainAccessToURL` already maintains bookmarks for every folder
// the user has granted access to, so recording a bookmark alongside
// the path would be redundant. When the user taps a recent entry,
// the open flow goes through the normal `ObtainAccessToURL` machinery
// which resolves the stored bookmark for the show folder containing
// the sequence.
//
// On-disk shape: `[showFolderPath: [Entry]]` under one UserDefaults
// key. Switching show folders shows that show's own recent list,
// not a global cross-show jumble. Entries pointing to files that no
// longer exist on disk are pruned (and persisted-back) on every
// `load(forShowFolder:)`.
//
// iCloud-evicted files are NOT pruned: their on-device placeholder
// still satisfies `fileExists`, so notDownloaded entries keep
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

    /// Load the recent list for `showFolder`, most recent first.
    /// Returns an empty list if `showFolder` is empty or has no
    /// recorded entries. Drops entries whose underlying file no
    /// longer exists and persists the trimmed list.
    static func load(forShowFolder showFolder: String?) -> [Entry] {
        guard let showFolder, !showFolder.isEmpty else { return [] }
        var all = loadAll()
        let entries = all[showFolder] ?? []
        let valid = entries.filter {
            FileManager.default.fileExists(atPath: $0.path)
        }
        if valid.count != entries.count {
            all[showFolder] = valid
            save(all)
        }
        return valid.sorted { $0.lastOpened > $1.lastOpened }
    }

    /// Push a path to the top of the show's list. De-duplicates by
    /// path and trims to `maxEntries`. No-ops if either argument is
    /// empty (e.g. recording before a show folder has been picked).
    static func record(path: String, forShowFolder showFolder: String?) {
        guard !path.isEmpty,
              let showFolder, !showFolder.isEmpty else { return }
        var all = loadAll()
        var slot = (all[showFolder] ?? []).filter { $0.path != path }
        slot.insert(Entry(path: path, lastOpened: Date()), at: 0)
        if slot.count > maxEntries {
            slot.removeSubrange(maxEntries..<slot.count)
        }
        all[showFolder] = slot
        save(all)
    }

    /// Remove a single entry (e.g. via swipe-to-delete).
    static func remove(path: String, forShowFolder showFolder: String?) {
        guard let showFolder, !showFolder.isEmpty else { return }
        var all = loadAll()
        guard var slot = all[showFolder] else { return }
        slot.removeAll { $0.path == path }
        all[showFolder] = slot
        save(all)
    }

    /// Wipe this show's recent list. Other shows are untouched.
    static func clear(forShowFolder showFolder: String?) {
        guard let showFolder, !showFolder.isEmpty else { return }
        var all = loadAll()
        all.removeValue(forKey: showFolder)
        save(all)
    }

    /// Wipe every show's recent list.
    static func clearAll() {
        UserDefaults.standard.removeObject(forKey: key)
    }

    private static func loadAll() -> [String: [Entry]] {
        guard let data = UserDefaults.standard.data(forKey: key) else {
            return [:]
        }
        if let dict = try? JSONDecoder().decode([String: [Entry]].self,
                                                 from: data) {
            return dict
        }
        // Pre-per-show shape was a flat `[Entry]`. Drop on first read
        // — recents is non-critical and rebuilds itself in seconds of
        // normal use; keeping the legacy data around without a known
        // show folder would mean stale paths in every show's list.
        UserDefaults.standard.removeObject(forKey: key)
        return [:]
    }

    private static func save(_ all: [String: [Entry]]) {
        guard let data = try? JSONEncoder().encode(all) else { return }
        UserDefaults.standard.set(data, forKey: key)
    }
}
