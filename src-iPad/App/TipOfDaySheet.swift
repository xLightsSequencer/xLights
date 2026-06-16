import SwiftUI
import WebKit

/// Theme-13 — Help ▸ Tip of the Day (desktop TipOfTheDayDialog).
/// Loads the shared tod.xml tip index through `XLTipOfDay`, picks a
/// random tip honoring the "min level" filter, and renders its HTML
/// in a web view. "Show tips on startup" pref is stored in
/// UserDefaults and consulted at launch.
struct TipOfDaySheet: View {
    @Environment(\.dismiss) private var dismiss

    @State private var tips: [TipEntry] = []
    @State private var current: TipEntry? = nil
    @State private var html: String? = nil
    @State private var isLoading = true
    @State private var showAtStartup = TipOfDayPrefs.showAtStartup

    var body: some View {
        NavigationStack {
            Group {
                if isLoading {
                    ProgressView("Loading tip…")
                        .frame(maxWidth: .infinity, maxHeight: .infinity)
                } else if let html {
                    TipWebView(html: html, baseURL: URL(string: XLTipOfDay.baseURL()))
                } else {
                    ContentUnavailableView("No tips available",
                                           systemImage: "lightbulb.slash",
                                           description: Text("Couldn't load tips. Check your network connection."))
                }
            }
            .navigationTitle(current?.title ?? "Tip of the Day")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Done") { dismiss() }
                }
                ToolbarItem(placement: .primaryAction) {
                    Button {
                        showNextTip()
                    } label: {
                        Label("Next Tip", systemImage: "arrow.clockwise")
                    }
                    .disabled(tips.isEmpty)
                }
                ToolbarItem(placement: .bottomBar) {
                    Toggle("Show tips on startup", isOn: $showAtStartup)
                        .onChange(of: showAtStartup) { _, newValue in
                            TipOfDayPrefs.showAtStartup = newValue
                        }
                        .font(.caption)
                }
            }
        }
        .onAppear { load() }
    }

    private func load() {
        guard tips.isEmpty else { return }
        // The bridge dispatches the completion to the main queue;
        // hop back onto the MainActor so mutating @State with the
        // non-Sendable entry array is well-formed under Swift 6.
        XLTipOfDay.loadTips { entries in
            // Flatten the bridge's NSDictionary array into Sendable
            // value structs before hopping to the MainActor — the
            // raw [NSDictionary] isn't Sendable under Swift 6, but
            // [TipEntry] is.
            let parsed = entries.map { TipEntry($0) }
            MainActor.assumeIsolated {
                tips = parsed
                isLoading = false
                showNextTip()
            }
        }
    }

    private func showNextTip() {
        guard !tips.isEmpty else { return }
        let pick = tips.randomElement()
        current = pick
        guard let pick else { return }
        html = nil
        let url = pick.url
        XLTipOfDay.loadTipHTML(url) { body in
            MainActor.assumeIsolated {
                if current?.url == url {
                    html = body ?? "<html><body><p>Couldn't load this tip.</p></body></html>"
                }
            }
        }
    }
}

struct TipEntry: Identifiable, Hashable {
    var id: String { url }
    let title: String
    let url: String
    let category: String
    let level: String

    init(_ dict: [AnyHashable: Any]) {
        title    = (dict["title"]    as? String) ?? ""
        url      = (dict["url"]      as? String) ?? ""
        category = (dict["category"] as? String) ?? ""
        level    = (dict["level"]    as? String) ?? "Beginner"
    }
}

/// "Show tips at startup" preference (mirrors desktop's MinTipLevel
/// on/off). Default on for first-run discoverability, matching the
/// desktop dialog's default.
enum TipOfDayPrefs {
    private static let key = "xl.showTipAtStartup"
    private static let seenKey = "xl.tipStartupSeen"

    static var showAtStartup: Bool {
        get {
            // Default true the first time, then honor the stored value.
            if UserDefaults.standard.object(forKey: key) == nil { return true }
            return UserDefaults.standard.bool(forKey: key)
        }
        set { UserDefaults.standard.set(newValue, forKey: key) }
    }

    /// Whether the startup tip has already been shown this launch —
    /// prevents re-showing on every ContentView `.task` re-invocation.
    @MainActor static var shownThisLaunch = false
}

private struct TipWebView: UIViewRepresentable {
    let html: String
    let baseURL: URL?

    func makeUIView(context: Context) -> WKWebView {
        let config = WKWebViewConfiguration()
        let view = WKWebView(frame: .zero, configuration: config)
        view.isOpaque = false
        view.backgroundColor = .clear
        return view
    }

    func updateUIView(_ uiView: WKWebView, context: Context) {
        uiView.loadHTMLString(html, baseURL: baseURL)
    }
}
