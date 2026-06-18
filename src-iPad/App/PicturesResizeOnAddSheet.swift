import SwiftUI
import UIKit

// Resize-on-add for the Pictures effect — desktop parity for the
// ResizeImageDialog (src-ui-wx/media/ResizeImageDialog.cpp) that lets a
// user shrink a large image when it's added to a Pictures effect. The
// desktop dialog seeds Width/Height spin controls from the image's
// current pixel size and writes a resized copy; this sheet mirrors that:
// it loads the just-picked image, offers new Width/Height (optionally
// keeping aspect ratio), writes a resized copy alongside the original in
// the Images/ folder, and re-points the effect's filename setting at it.
//
// Triggered only for the Pictures effect (subdirectory == "Images"); the
// user can always Skip to keep the original image untouched.
struct PicturesResizeOnAddSheet: View {
    @Environment(SequencerViewModel.self) var viewModel
    @Environment(\.dismiss) private var dismiss

    /// Stored (show-relative or absolute) path of the picked image.
    let storedPath: String
    /// Called with the new stored path when a resized copy is written.
    let onResized: (String) -> Void

    @State private var originalSize: CGSize?
    @State private var width: Int = 0
    @State private var height: Int = 0
    @State private var keepAspect = true
    @State private var working = false
    @State private var loadFailed = false

    private var aspect: CGFloat {
        guard let s = originalSize, s.height > 0 else { return 1 }
        return s.width / s.height
    }

    var body: some View {
        NavigationStack {
            Form {
                if loadFailed {
                    Text("Couldn't read the image to resize it. The original was kept.")
                        .foregroundStyle(.secondary)
                } else if let s = originalSize {
                    Section("Original") {
                        LabeledContent("Dimensions",
                                       value: "\(Int(s.width)) × \(Int(s.height))")
                    }
                    Section("Resize To") {
                        Stepper(value: $width, in: 1...10000, step: 1) {
                            HStack {
                                Text("Width")
                                Spacer()
                                Text("\(width)").monospacedDigit()
                                    .foregroundStyle(.secondary)
                            }
                        }
                        .onChange(of: width) { _, newW in
                            if keepAspect { height = max(1, Int((CGFloat(newW) / aspect).rounded())) }
                        }
                        Stepper(value: $height, in: 1...10000, step: 1) {
                            HStack {
                                Text("Height")
                                Spacer()
                                Text("\(height)").monospacedDigit()
                                    .foregroundStyle(.secondary)
                            }
                        }
                        .onChange(of: height) { _, newH in
                            if keepAspect { width = max(1, Int((CGFloat(newH) * aspect).rounded())) }
                        }
                        Toggle("Keep aspect ratio", isOn: $keepAspect)
                            .onChange(of: keepAspect) { _, on in
                                if on { height = max(1, Int((CGFloat(width) / aspect).rounded())) }
                            }
                    }
                } else {
                    ProgressView()
                }
            }
            .navigationTitle("Resize Image")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Skip") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Resize") { resizeAndCommit() }
                        .disabled(originalSize == nil || working || loadFailed)
                }
            }
        }
        .task { loadOriginal() }
    }

    private func resolvedURL() -> URL? {
        if storedPath.hasPrefix("/") {
            return URL(fileURLWithPath: storedPath)
        }
        let showDir = viewModel.document.showFolderPath()
        guard !showDir.isEmpty else { return nil }
        return URL(fileURLWithPath: showDir).appendingPathComponent(storedPath)
    }

    private func loadOriginal() {
        guard let url = resolvedURL(),
              let img = UIImage(contentsOfFile: url.path) else {
            loadFailed = true
            return
        }
        let px = CGSize(width: img.size.width * img.scale,
                        height: img.size.height * img.scale)
        originalSize = px
        width = max(1, Int(px.width))
        height = max(1, Int(px.height))
    }

    private func resizeAndCommit() {
        guard let srcURL = resolvedURL() else { return }
        working = true
        let targetW = width, targetH = height
        let showDir = viewModel.document.showFolderPath()
        DispatchQueue.global(qos: .userInitiated).async {
            let outURL = Self.writeResized(srcURL: srcURL,
                                           width: targetW, height: targetH)
            DispatchQueue.main.async {
                working = false
                if let outURL {
                    let stored = (!showDir.isEmpty && outURL.path.hasPrefix(showDir))
                        ? viewModel.document.makeRelativePath(outURL.path)
                        : outURL.path
                    onResized(stored)
                }
                dismiss()
            }
        }
    }

    private nonisolated static func writeResized(srcURL: URL, width: Int, height: Int) -> URL? {
        guard let img = UIImage(contentsOfFile: srcURL.path) else { return nil }
        let size = CGSize(width: width, height: height)
        let fmt = UIGraphicsImageRendererFormat()
        fmt.scale = 1
        fmt.opaque = false
        let resized = UIGraphicsImageRenderer(size: size, format: fmt).image { _ in
            img.draw(in: CGRect(origin: .zero, size: size))
        }
        let ext = srcURL.pathExtension.lowercased()
        let isPNG = (ext != "jpg" && ext != "jpeg")
        guard let data = isPNG ? resized.pngData()
                               : resized.jpegData(compressionQuality: 0.95) else {
            return nil
        }
        let base = srcURL.deletingPathExtension().lastPathComponent
        let outExt = isPNG ? "png" : ext
        let outName = "\(base)_\(width)x\(height).\(outExt)"
        let outURL = srcURL.deletingLastPathComponent().appendingPathComponent(outName)
        do {
            try data.write(to: outURL, options: .atomic)
        } catch {
            return nil
        }
        return outURL
    }
}
