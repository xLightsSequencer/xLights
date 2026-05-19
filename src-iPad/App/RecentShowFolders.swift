import Foundation

// L-1b — recent show folders list. Persists the last N show folder paths
// the user has loaded so the FolderConfigView can offer a tap-to-switch
// row instead of "go re-pick from Files every time."
//
// Storage shape mirrors `RecentSequences` but flat (one global MRU list,
// not scoped per parent — show folders ARE the scope).
//
// Stored as plain path strings — `ObtainAccessToURL` already maintains
// a security-scoped bookmark for every folder the user has granted
// access to, so a parallel bookmark store would be redundant. If a
// stored bookmark has aged out, the existing access-reprompt sheet
// handles re-grant on first read attempt.
//
// Entries are pruned on load if the underlying directory no longer
// exists. iCloud-evicted folders are NOT pruned: the on-device
// placeholder still satisfies `fileExists`.
enum RecentShowFolders {
    private static let key = "xLights.recentShowFolders"
    static let maxEntries = 8

    struct Entry: Identifiable, Hashable, Codable {
        let path: String
        let lastOpened: Date

        var id: String { path }

        /// Folder name without the parent path — e.g. "MyShow2026".
        var displayName: String {
            (path as NSString).lastPathComponent
        }

        /// Parent directory — disambiguates two show folders that share
        /// a basename across iCloud Drive vs. local containers.
        var parentDisplay: String {
            (path as NSString).deletingLastPathComponent
        }
    }

    /// Load the recent list, MRU first. Drops entries whose underlying
    /// directory no longer exists and persists the trimmed list.
    static func load() -> [Entry] {
        var entries = decode()
        let valid = entries.filter {
            FileManager.default.fileExists(atPath: $0.path)
        }
        if valid.count != entries.count {
            entries = valid
            encode(entries)
        }
        return entries.sorted { $0.lastOpened > $1.lastOpened }
    }

    /// Push `path` to the top of the list. De-duplicates by path and
    /// trims to `maxEntries`. No-ops on empty path.
    static func record(path: String) {
        guard !path.isEmpty else { return }
        var entries = decode().filter { $0.path != path }
        entries.insert(Entry(path: path, lastOpened: Date()), at: 0)
        if entries.count > maxEntries {
            entries.removeSubrange(maxEntries..<entries.count)
        }
        encode(entries)
    }

    /// Remove a single entry (e.g. via swipe-to-delete).
    static func remove(path: String) {
        let entries = decode().filter { $0.path != path }
        encode(entries)
    }

    /// Wipe the entire list.
    static func clearAll() {
        UserDefaults.standard.removeObject(forKey: key)
    }

    private static func decode() -> [Entry] {
        guard let data = UserDefaults.standard.data(forKey: key) else {
            return []
        }
        return (try? JSONDecoder().decode([Entry].self, from: data)) ?? []
    }

    private static func encode(_ entries: [Entry]) {
        guard let data = try? JSONEncoder().encode(entries) else { return }
        UserDefaults.standard.set(data, forKey: key)
    }
}
