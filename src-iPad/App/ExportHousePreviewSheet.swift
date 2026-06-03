/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

import SwiftUI
import UniformTypeIdentifiers

/// Tools → Export House Preview. Pick a resolution + codec, render the whole
/// house preview offscreen at that size (independent of the on-screen pane),
/// encode to `.mp4`, then hand the file to `.fileExporter` to save. The encode
/// runs on a background queue and reports progress; the sheet blocks
/// interactive dismissal while it's running so nothing else renders the
/// preview concurrently.
struct ExportHousePreviewSheet: View {
    @Environment(SequencerViewModel.self) private var viewModel
    @Environment(\.dismiss) private var dismiss

    enum Resolution: String, CaseIterable, Identifiable {
        case p720  = "1280 × 720"
        case p1080 = "1920 × 1080"
        case p2160 = "3840 × 2160 (4K)"
        case match = "Match preview"
        var id: String { rawValue }
    }

    // Remembered across opens / launches (UserDefaults-backed). Resolution is a
    // String-RawRepresentable enum, which @AppStorage persists directly.
    @AppStorage("housePreviewExport.resolution") private var resolution: Resolution = .p1080
    @AppStorage("housePreviewExport.highQuality") private var highQuality: Bool = false

    @State private var exporting = false
    @State private var progress: Double = 0
    @State private var exportFailed = false

    private func dimensions() -> (Int, Int) {
        switch resolution {
        case .p720:  return (1280, 720)
        case .p1080: return (1920, 1080)
        case .p2160: return (3840, 2160)
        case .match:
            let s = viewModel.layoutPreviewSize()
            // Even dimensions keep H.264/HEVC happy; floor anything odd.
            let w = max(2, (s.width  / 2) * 2)
            let h = max(2, (s.height / 2) * 2)
            return (w == 0 ? 1920 : w, h == 0 ? 1080 : h)
        }
    }

    private var matchLabel: String {
        let s = viewModel.layoutPreviewSize()
        if s.width > 0 && s.height > 0 {
            return "Match preview (\(s.width) × \(s.height))"
        }
        return "Match preview"
    }

    var body: some View {
        NavigationStack {
            Form {
                Section("Resolution") {
                    Picker("Resolution", selection: $resolution) {
                        ForEach(Resolution.allCases) { r in
                            Text(r == .match ? matchLabel : r.rawValue).tag(r)
                        }
                    }
                    .pickerStyle(.inline)
                    .labelsHidden()
                    .disabled(exporting)
                }
                Section("Format") {
                    Picker("Format", selection: $highQuality) {
                        Text("Compressed (H.264)").tag(false)
                        Text("High Quality (HEVC)").tag(true)
                    }
                    .pickerStyle(.inline)
                    .labelsHidden()
                    .disabled(exporting)
                }
                if exporting {
                    Section {
                        ProgressView(value: progress) {
                            Text("Rendering… \(Int((progress * 100).rounded()))%")
                        }
                    }
                } else {
                    Section {
                        Text("Renders the entire show. Larger resolutions and longer sequences take longer to encode.")
                            .font(.footnote)
                            .foregroundStyle(.secondary)
                    }
                }
            }
            .navigationTitle("Export House Preview")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                        .disabled(exporting)
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Export") { startExport() }
                        .disabled(exporting)
                }
            }
            .interactiveDismissDisabled(exporting)
            .alert("Export Failed", isPresented: $exportFailed) {
                Button("OK", role: .cancel) { }
            } message: {
                Text("The House Preview video could not be exported. Make sure the sequence has rendered, then try again.")
            }
        }
    }

    private func startExport() {
        let (w, h) = dimensions()
        let tempDir = FileManager.default.temporaryDirectory
        let tempPath = tempDir
            .appendingPathComponent("HousePreview-\(UUID().uuidString).mp4").path
        exporting = true
        progress = 0
        viewModel.exportHousePreview(
            path: tempPath, width: w, height: h, highQuality: highQuality,
            progress: { f in progress = f },
            completion: { success in
                exporting = false
                guard success else {
                    exportFailed = true   // stays open; alert explains the failure
                    return
                }
                // Present the system share sheet ("Save to Files", AirDrop, …)
                // OVER this sheet — presenting from the root after dismiss() races
                // the dismiss transition and silently no-ops. Close this sheet
                // once the user finishes (saves / cancels) the share.
                XLPresentShareSheet(items: [URL(fileURLWithPath: tempPath)]) {
                    dismiss()
                }
            })
    }
}
