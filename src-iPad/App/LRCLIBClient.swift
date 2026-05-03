import Foundation

// B91 — minimal LRCLIB (https://lrclib.net) search client.
//
// The desktop equivalent is `src-ui-wx/sequencer/LRCLIBSearchDialog.cpp`'s
// `DoSearch()` (libcurl + nlohmann/json). On iPad we use URLSession +
// Codable; the JSON shape and User-Agent header match the desktop call
// so vendors / publishers see consistent traffic from both clients.

struct LRCLIBResult: Codable, Identifiable, Equatable {
    /// LRCLIB returns the row's primary key as `id`. Stable across
    /// searches; safe for SwiftUI list selection.
    let id: Int
    let trackName: String
    let artistName: String
    let albumName: String
    let duration: Double?
    let instrumental: Bool?
    let plainLyrics: String?
    let syncedLyrics: String?

    var hasSynced: Bool {
        guard let s = syncedLyrics else { return false }
        return !s.isEmpty
    }

    var hasAnyLyrics: Bool {
        if hasSynced { return true }
        if let p = plainLyrics, !p.isEmpty { return true }
        return false
    }
}

enum LRCLIBClient {
    enum SearchError: Error, LocalizedError {
        case emptyQuery
        case network(String)
        case unexpectedResponse

        var errorDescription: String? {
            switch self {
            case .emptyQuery:           return "Please enter a search query."
            case .network(let msg):     return "Network error: \(msg)"
            case .unexpectedResponse:   return "Unexpected response from LRCLIB."
            }
        }
    }

    /// Search by free-form query string. The server tolerates
    /// "artist - track", "track artist", or just track titles.
    static func search(query: String) async throws -> [LRCLIBResult] {
        let trimmed = query.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !trimmed.isEmpty else { throw SearchError.emptyQuery }

        var components = URLComponents(string: "https://lrclib.net/api/search")!
        components.queryItems = [URLQueryItem(name: "q", value: trimmed)]
        guard let url = components.url else { throw SearchError.unexpectedResponse }

        var request = URLRequest(url: url)
        // Match the desktop User-Agent so LRCLIB sees consistent
        // identification for both clients.
        request.setValue("xLights (https://xlights.org)", forHTTPHeaderField: "User-Agent")
        request.timeoutInterval = 10

        let data: Data
        let response: URLResponse
        do {
            (data, response) = try await URLSession.shared.data(for: request)
        } catch {
            throw SearchError.network(error.localizedDescription)
        }
        guard let http = response as? HTTPURLResponse, (200..<300).contains(http.statusCode) else {
            let code = (response as? HTTPURLResponse)?.statusCode ?? -1
            throw SearchError.network("HTTP \(code)")
        }

        do {
            return try JSONDecoder().decode([LRCLIBResult].self, from: data)
        } catch {
            throw SearchError.unexpectedResponse
        }
    }
}
