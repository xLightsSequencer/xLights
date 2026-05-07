import Foundation
import UIKit

// Auto-uploads diagnostic zips to the same endpoint the desktop app
// uses (https://dankulp.com/crashUpload/index.php), so iPad reports
// land in the existing triage bin alongside Mac/Win/Linux ones.
//
// Pipeline:
//
//   1. App launch → bootstrap()
//      - reads any stale session sentinel (= previous run crashed
//        without an orderly background handoff) and stages a small
//        diagnostic zip via XLLogPackager.stagePendingUpload()
//      - writes a fresh session sentinel
//      - subscribes to XLMetricKitDidReceivePayloads to stage on
//        every MetricKit delivery (~24h after the underlying event)
//      - kickoff()
//
//   2. scenePhase = .active → beginCurrentSession() + kickoff()
//   3. scenePhase = .background → endCurrentSession()
//
//   kickoff() walks Library/Logs/PendingUpload/*.zip and POSTs each
//   via foreground URLSession. On HTTP 200 the file is deleted; on
//   any other outcome it stays for the next sweep.
//
// Honors the opt-in toggle exposed via Settings.bundle (UserDefaults
// key XLSendCrashReports, default true). Toggling off short-circuits
// kickoff() but does NOT delete already-staged files — they sit
// indefinitely until either the user turns the toggle back on or
// reinstalls.

@MainActor
final class XLDiagnosticUploader {
    static let shared = XLDiagnosticUploader()

    nonisolated private static let endpointURL =
        URL(string: "https://dankulp.com/crashUpload/index.php")!
    nonisolated private static let optInDefaultsKey = "XLSendCrashReports"

    // Multipart boundary chosen to byte-match the desktop wxDebugReport
    // upload, since the receiving server already expects this exact
    // shape from desktop clients.
    nonisolated private static let multipartBoundary = "------------------------b29a7c2fe47b9481"

    nonisolated private let session: URLSession
    nonisolated private let workQueue = DispatchQueue(
        label: "org.xlights.diagnostic-upload", qos: .utility)
    private var didSubscribeToMetricKit = false

    nonisolated static var isOptedIn: Bool {
        // Debug builds never upload. Stopping the debugger, killing
        // the app from Xcode, and other normal dev interactions all
        // synthesise crash-like signals; uploading those would
        // pollute the triage bin. The session sentinel still clears
        // and re-arms in bootstrap() so a debug run can't leave a
        // stale breadcrumb that a later release-build run picks up.
        #if DEBUG
        return false
        #else
        let defaults = UserDefaults.standard
        if defaults.object(forKey: optInDefaultsKey) == nil {
            return true
        }
        return defaults.bool(forKey: optInDefaultsKey)
        #endif
    }

    private init() {
        let cfg = URLSessionConfiguration.default
        cfg.allowsCellularAccess = true
        cfg.waitsForConnectivity = true
        cfg.timeoutIntervalForRequest = 60
        cfg.timeoutIntervalForResource = 120
        session = URLSession(configuration: cfg)
    }

    func bootstrap() {
        if XLDiagnosticSession.readStaleSentinel() != nil {
            stagePendingUploadAsync()
            XLDiagnosticSession.clearStaleSentinel()
        }
        XLDiagnosticSession.beginCurrentSession()

        if !didSubscribeToMetricKit {
            didSubscribeToMetricKit = true
            NotificationCenter.default.addObserver(
                forName: Notification.Name("XLMetricKitDidReceivePayloads"),
                object: nil,
                queue: nil
            ) { [weak self] _ in
                self?.stagePendingUploadAsync()
                self?.kickoff()
            }
        }

        kickoff()
    }

    func beginCurrentSession() {
        XLDiagnosticSession.beginCurrentSession()
    }

    func endCurrentSession() {
        XLDiagnosticSession.endCurrentSession()
    }

    nonisolated func kickoff() {
        guard Self.isOptedIn else { return }
        workQueue.async {
            let fm = FileManager.default
            guard let lib = try? fm.url(
                for: .libraryDirectory, in: .userDomainMask,
                appropriateFor: nil, create: false
            ) else { return }
            let pending = lib
                .appendingPathComponent("Logs", isDirectory: true)
                .appendingPathComponent("PendingUpload", isDirectory: true)

            guard let entries = try? fm.contentsOfDirectory(
                at: pending,
                includingPropertiesForKeys: nil,
                options: [.skipsHiddenFiles]
            ) else { return }

            for entry in entries where entry.pathExtension.lowercased() == "zip" {
                self.uploadFile(at: entry)
            }
        }
    }

    nonisolated private func stagePendingUploadAsync() {
        // Same opt-out as kickoff(): skip the zip build entirely in
        // debug builds and when the user has disabled crash reports.
        guard Self.isOptedIn else { return }
        workQueue.async {
            do {
                _ = try XLLogPackager.stagePendingUpload()
            } catch {
                NSLog("XLDiagnosticUploader: stagePendingUpload failed: \(error)")
            }
        }
    }

    nonisolated private func uploadFile(at fileURL: URL) {
        guard let body = buildMultipartBody(fileURL: fileURL) else { return }

        var request = URLRequest(url: Self.endpointURL)
        request.httpMethod = "POST"
        request.setValue(
            "multipart/form-data; boundary=\(Self.multipartBoundary)",
            forHTTPHeaderField: "Content-Type")

        let task = session.uploadTask(with: request, from: body) {
            _, response, _ in
            guard let http = response as? HTTPURLResponse,
                  http.statusCode == 200 else {
                return
            }
            try? FileManager.default.removeItem(at: fileURL)
        }
        task.resume()
    }

    nonisolated private func buildMultipartBody(fileURL: URL) -> Data? {
        guard let fileData = try? Data(contentsOf: fileURL) else { return nil }

        let info = Bundle.main.infoDictionary ?? [:]
        let version = (info["CFBundleShortVersionString"] as? String) ?? "0"
        let build = (info["CFBundleVersion"] as? String) ?? "0"

        let stampFmt = DateFormatter()
        stampFmt.dateFormat = "yyyy-MM-dd_HH-mm-ss-SSS"
        stampFmt.timeZone = TimeZone(abbreviation: "UTC")
        let stamp = stampFmt.string(from: Date())

        // Match the desktop wxDebugReport filename shape exactly so
        // server-side filing and grep-by-platform stays coherent.
        let arch: String
        #if arch(arm64)
        arch = "arm64"
        #elseif arch(x86_64)
        arch = "x86_64"
        #else
        arch = "unknown"
        #endif
        let postName = "xLights-iPad-iOS_\(arch)_\(version)-\(build)_\(stamp).zip"

        var body = Data()
        body.append("--\(Self.multipartBoundary)\n".data(using: .utf8)!)
        body.append("Content-Type: application/octet-stream\n".data(using: .utf8)!)
        body.append(
            "Content-Disposition: form-data; name=\"userfile\"; filename=\"\(postName)\"\n\n"
                .data(using: .utf8)!)
        body.append(fileData)
        body.append("\n".data(using: .utf8)!)
        body.append("--\(Self.multipartBoundary)--\n".data(using: .utf8)!)
        return body
    }
}
