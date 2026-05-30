import SwiftUI

// H-6 / T-2 — Tools → View Log. Reads the rotating spdlog file
// at <sandbox>/Library/Logs/xLights.log (set up in XLiPadInit).
// Level + logger + text filters with optional follow-tail. v1
// scope: current log only — rotated `.1`/`.2` siblings are
// reachable via Package Logs.
struct LogViewerSheet: View {
    @Environment(\.dismiss) private var dismiss

    @State private var entries: [LogEntry] = []
    @State private var searchText: String = ""
    @State private var levelFilter: LogLevelFilter = .info
    @State private var loggerFilter: String = "All"
    @State private var followTail: Bool = true
    @State private var fileSize: UInt64 = 0

    private static let logURL: URL = {
        let lib = FileManager.default.urls(for: .libraryDirectory, in: .userDomainMask).first!
        return lib.appendingPathComponent("Logs/xLights.log")
    }()

    // Last N lines we keep in the viewer. Spdlog rotates at 5 MB
    // and pattern is one line per entry, so this caps memory at a
    // few hundred KB of formatted text plus parsed structs.
    private static let maxLines = 2000

    private static let allLoggers = ["All", "xLights", "render", "curl", "job", "work"]

    var body: some View {
        NavigationStack {
            VStack(spacing: 0) {
                filterBar
                Divider()
                logList
            }
            .navigationTitle("Log Viewer")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Done") { dismiss() }
                }
                ToolbarItem(placement: .primaryAction) {
                    Button {
                        refresh(force: true)
                    } label: {
                        Image(systemName: "arrow.clockwise")
                    }
                }
            }
            .onAppear { refresh(force: true) }
        }
    }

    private var filterBar: some View {
        VStack(spacing: 8) {
            HStack {
                Image(systemName: "magnifyingglass")
                    .foregroundStyle(.secondary)
                TextField("Search", text: $searchText)
                    .textFieldStyle(.roundedBorder)
                    .autocorrectionDisabled()
                    .textInputAutocapitalization(.never)
                Toggle(isOn: $followTail) {
                    Label("Tail", systemImage: "text.append")
                }
                .toggleStyle(.button)
                .controlSize(.small)
            }
            HStack {
                Picker("Level", selection: $levelFilter) {
                    ForEach(LogLevelFilter.allCases) { lvl in
                        Text(lvl.label).tag(lvl)
                    }
                }
                .pickerStyle(.menu)

                Picker("Logger", selection: $loggerFilter) {
                    ForEach(Self.allLoggers, id: \.self) { l in
                        Text(l).tag(l)
                    }
                }
                .pickerStyle(.menu)

                Spacer()

                Text(statusLine)
                    .font(.caption2)
                    .foregroundStyle(.secondary)
            }
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 8)
    }

    private var statusLine: String {
        let visible = filteredEntries.count
        let total = entries.count
        if visible == total {
            return "\(total) lines"
        }
        return "\(visible) / \(total)"
    }

    private var logList: some View {
        ScrollViewReader { proxy in
            ScrollView {
                LazyVStack(alignment: .leading, spacing: 1) {
                    ForEach(filteredEntries) { entry in
                        LogRow(entry: entry)
                            .id(entry.id)
                    }
                }
                .padding(.horizontal, 8)
                .padding(.vertical, 4)
            }
            .background(Color(.systemBackground))
            .overlay(alignment: .center) {
                if entries.isEmpty {
                    Text("No log entries — the log file is empty or hasn't been written yet.")
                        .foregroundStyle(.secondary)
                        .multilineTextAlignment(.center)
                        .padding()
                } else if filteredEntries.isEmpty {
                    Text("No entries match the current filters.")
                        .foregroundStyle(.secondary)
                        .padding()
                }
            }
            .onChange(of: entries.count) { _, _ in
                if followTail, let last = filteredEntries.last {
                    withAnimation(.linear(duration: 0.1)) {
                        proxy.scrollTo(last.id, anchor: .bottom)
                    }
                }
            }
            .background(
                // Periodic poll while the tail is enabled. 1s feels
                // live without thrashing the file system; spdlog
                // flushes on info+ so new lines land promptly.
                TimelineView(.periodic(from: .now, by: 1.0)) { _ in
                    Color.clear
                        .onAppear { if followTail { refresh(force: false) } }
                        .onChange(of: followTail) { _, on in
                            if on { refresh(force: false) }
                        }
                }
            )
        }
    }

    private var filteredEntries: [LogEntry] {
        var result = entries
        if loggerFilter != "All" {
            result = result.filter { $0.logger == loggerFilter }
        }
        result = result.filter { levelFilter.includes($0.level) }
        if !searchText.isEmpty {
            let needle = searchText
            result = result.filter { $0.raw.localizedCaseInsensitiveContains(needle) }
        }
        return result
    }

    // Re-read the log. `force` ignores the file-size short-circuit
    // (used by the explicit reload button and the first appear).
    private func refresh(force: Bool) {
        let fm = FileManager.default
        guard fm.fileExists(atPath: Self.logURL.path) else {
            entries = []
            fileSize = 0
            return
        }
        if !force,
           let attrs = try? fm.attributesOfItem(atPath: Self.logURL.path),
           let size = attrs[.size] as? UInt64,
           size == fileSize {
            return
        }

        guard let data = try? Data(contentsOf: Self.logURL, options: [.mappedIfSafe]) else {
            return
        }
        fileSize = UInt64(data.count)
        guard let text = String(data: data, encoding: .utf8) else { return }

        // Take the tail. Splitting the whole file is fine for the
        // 5 MB cap that spdlog enforces; if that ever changes,
        // walk backwards from end-of-data instead.
        let lines = text.split(separator: "\n", omittingEmptySubsequences: false)
        let tail = lines.suffix(Self.maxLines)
        entries = tail.compactMap { parse(String($0)) }
    }

    // Pattern from XLiPadInit: "%Y-%m-%d %H:%M:%S.%e [%n %l] %v"
    // Example: "2026-05-17 14:23:01.456 [xLights info] Starting up"
    private func parse(_ raw: String) -> LogEntry? {
        if raw.isEmpty { return nil }
        guard let openBracket = raw.range(of: " [") else {
            return LogEntry(raw: raw, timestamp: "", logger: "", level: "info", message: raw)
        }
        let timestamp = String(raw[..<openBracket.lowerBound])
        let afterOpen = raw[openBracket.upperBound...]
        guard let closeBracket = afterOpen.range(of: "] ") else {
            return LogEntry(raw: raw, timestamp: timestamp, logger: "", level: "info",
                            message: String(afterOpen))
        }
        let inside = afterOpen[..<closeBracket.lowerBound]
        let parts = inside.split(separator: " ", maxSplits: 1, omittingEmptySubsequences: true)
        let logger = parts.indices.contains(0) ? String(parts[0]) : ""
        let level  = parts.indices.contains(1) ? String(parts[1]) : "info"
        let message = String(afterOpen[closeBracket.upperBound...])
        return LogEntry(raw: raw, timestamp: timestamp, logger: logger, level: level, message: message)
    }
}

private struct LogEntry: Identifiable {
    let id = UUID()
    let raw: String
    let timestamp: String
    let logger: String
    let level: String
    let message: String
}

private enum LogLevelFilter: Int, CaseIterable, Identifiable {
    case trace, debug, info, warn, error

    var id: Int { rawValue }

    var label: String {
        switch self {
        case .trace: return "Trace+"
        case .debug: return "Debug+"
        case .info:  return "Info+"
        case .warn:  return "Warn+"
        case .error: return "Error"
        }
    }

    private static let order: [String: Int] = [
        "trace": 0,
        "debug": 1,
        "info": 2,
        "warning": 3, "warn": 3,
        "error": 4, "err": 4,
        "critical": 5, "crit": 5,
    ]

    func includes(_ level: String) -> Bool {
        let lvl = Self.order[level.lowercased()] ?? 2
        return lvl >= self.rawValue
    }
}

private struct LogRow: View {
    let entry: LogEntry

    var body: some View {
        Text(entry.raw)
            .font(.system(.caption2, design: .monospaced))
            .foregroundStyle(color)
            .textSelection(.enabled)
            .frame(maxWidth: .infinity, alignment: .leading)
    }

    private var color: Color {
        switch entry.level.lowercased() {
        case "error", "err", "critical", "crit": return .red
        case "warn", "warning":                   return .orange
        case "debug", "trace":                    return .secondary
        default:                                  return .primary
        }
    }
}
