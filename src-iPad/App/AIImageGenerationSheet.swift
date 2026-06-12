import SwiftUI
import UIKit
import CoreGraphics

/// AI image generation sheet — presented from the Pictures effect's
/// filename block when the user taps "AI…". Wraps `XLAIImageSession`
/// (which owns an `aiBase::AIImageGenerator`). Mirrors the desktop
/// `AIImageDialog` in embedded mode: prompt entry, generate button,
/// preview, and a Use Image action that saves the PNG into the
/// show folder's `AIImages/` subdirectory and writes the resulting
/// path back to the calling effect's `E_TEXTCTRL_Pictures_Filename`
/// setting.
///
/// `onCommitPath` receives a relative path (`AIImages/foo.png`) where
/// possible, falling back to the absolute path if the show folder
/// hasn't been resolved.
struct AIImageGenerationSheet: View {
    @Environment(SequencerViewModel.self) var viewModel
    @Environment(\.dismiss) private var dismiss

    let onCommitPath: (String) -> Void

    /// All services that have IMAGES enabled + a configured key.
    @State private var availableServices: [String] = []
    @State private var selectedService: String = ""

    @State private var session: XLAIImageSession? = nil
    // AI-1 — the selected generator's style/choice properties (e.g. Apple
    // Intelligence / Gemini image styles), rendered as Pickers.
    @State private var styleProperties: [XLAIServiceProperty] = []
    @State private var prompt: String = ""

    @State private var status: Status = .idle
    @State private var pngData: Data? = nil
    @State private var preview: UIImage? = nil

    // Post-processing state
    @State private var removeBlackBackground: Bool = true
    @State private var showCropResize: Bool = false

    private enum Status: Equatable {
        case idle
        case running
        case ready
        case error(String)
    }

    var body: some View {
        NavigationStack {
            Form {
                Section("Service") {
                    if availableServices.isEmpty {
                        Text("""
                             No image-generation service is configured. \
                             Open Tools → AI Services… to set up Claude, \
                             ChatGPT, or another image-capable provider.
                             """)
                            .font(.footnote)
                            .foregroundStyle(.secondary)
                    } else if availableServices.count == 1 {
                        LabeledContent("Service", value: availableServices[0])
                    } else {
                        Picker("Service", selection: $selectedService) {
                            ForEach(availableServices, id: \.self) { name in
                                Text(name).tag(name)
                            }
                        }
                        .onChange(of: selectedService) { _, newValue in
                            session = XLAIImageSession.session(forService: newValue)
                            resetResult()
                            refreshStyleProperties()
                        }
                    }
                }

                styleSection

                Section {
                    TextEditor(text: $prompt)
                        .frame(minHeight: 120)
                        .overlay(alignment: .topLeading) {
                            if prompt.isEmpty {
                                Text("Describe the image you want…")
                                    .foregroundStyle(.secondary)
                                    .padding(.top, 8)
                                    .padding(.leading, 5)
                                    .allowsHitTesting(false)
                            }
                        }
                } header: {
                    Text("Prompt")
                } footer: {
                    Text("Prompts you submit are sent to \(selectedService.isEmpty ? "the configured AI service" : selectedService)'s servers.")
                        .font(.caption)
                }

                Section {
                    Button {
                        runGenerate()
                    } label: {
                        HStack {
                            if status == .running {
                                ProgressView().controlSize(.small)
                                Text("Generating…")
                            } else {
                                Image(systemName: "wand.and.stars")
                                Text(status == .ready ? "Regenerate" : "Generate")
                            }
                            Spacer()
                        }
                    }
                    .disabled(prompt.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty
                              || session == nil
                              || status == .running)

                    if case .error(let msg) = status {
                        Label(msg, systemImage: "xmark.octagon.fill")
                            .foregroundStyle(.red)
                            .font(.footnote)
                    }
                }

                if let preview = preview {
                    Section("Preview") {
                        Image(uiImage: preview)
                            .resizable()
                            .scaledToFit()
                            .frame(maxHeight: 360)
                            .frame(maxWidth: .infinity)
                            .background(
                                RoundedRectangle(cornerRadius: 8)
                                    .fill(Color(uiColor: .secondarySystemBackground))
                            )
                            .overlay(
                                RoundedRectangle(cornerRadius: 8)
                                    .stroke(Color.secondary.opacity(0.3), lineWidth: 0.5)
                            )

                        Toggle("Remove black background", isOn: $removeBlackBackground)

                        Button {
                            showCropResize = true
                        } label: {
                            Label("Crop / Resize…", systemImage: "crop")
                        }

                        Button {
                            commitImage()
                        } label: {
                            Label("Use This Image", systemImage: "checkmark.circle.fill")
                        }
                    }
                }
            }
            .navigationTitle("AI Image")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .topBarLeading) {
                    Button("Cancel") { dismiss() }
                }
            }
        }
        .onAppear { loadServices() }
        .sheet(isPresented: $showCropResize) {
            if let img = preview {
                CropResizeSheet(image: img) { result in
                    preview = result
                    pngData = result.pngData()
                }
            }
        }
    }

    // MARK: - Service discovery

    private func loadServices() {
        let all = XLAIServices.shared().allServices()
        availableServices = all
            .filter { $0.available && $0.capabilities.contains(XLAICapabilityImages) }
            .map { $0.name }

        if selectedService.isEmpty || !availableServices.contains(selectedService) {
            selectedService = availableServices.first ?? ""
        }
        if !selectedService.isEmpty && session == nil {
            session = XLAIImageSession.session(forService: selectedService)
        }
        refreshStyleProperties()
    }

    /// AI-1 — pull the current generator's tunable properties (only Choice
    /// properties are rendered, e.g. the style preset).
    private func refreshStyleProperties() {
        styleProperties = session?.imageProperties() ?? []
    }

    @ViewBuilder
    private var styleSection: some View {
        let choices = styleProperties.filter { $0.kind == .choice && !$0.choices.isEmpty }
        if !choices.isEmpty {
            Section("Style") {
                ForEach(choices, id: \.propertyId) { prop in
                    Picker(prop.label.isEmpty ? "Style" : prop.label,
                           selection: Binding(
                            get: { prop.stringValue },
                            set: { newValue in
                                session?.setStringProperty(prop.propertyId, value: newValue)
                                refreshStyleProperties()
                            })) {
                        ForEach(prop.choices, id: \.self) { choice in
                            Text(choice).tag(choice)
                        }
                    }
                }
            }
        }
    }

    // MARK: - Generate

    private func runGenerate() {
        guard let session = session else { return }
        let p = prompt.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !p.isEmpty else { return }

        status = .running
        pngData = nil
        preview = nil
        session.generate(p) { png, err in
            Task { @MainActor in
                if let err = err {
                    status = .error(err)
                    return
                }
                guard let png = png, let img = UIImage(data: png) else {
                    status = .error("Couldn't decode generated image")
                    return
                }
                pngData = png
                preview = img
                status = .ready
            }
        }
    }

    private func resetResult() {
        status = .idle
        pngData = nil
        preview = nil
    }

    // MARK: - Save + commit

    private func commitImage() {
        guard var img = preview else { return }

        if removeBlackBackground {
            img = removeBlackBackgroundFromImage(img) ?? img
        }

        guard let png = img.pngData() else {
            status = .error("Couldn't encode image as PNG")
            return
        }

        let doc = viewModel.document
        let showDir = doc.showFolderPath()
        guard !showDir.isEmpty else {
            status = .error("Open a show folder before saving generated images.")
            return
        }

        let ts = Int(Date().timeIntervalSince1970 * 1000)
        let fileName = "ai_generated_\(ts).png"

        let tmpDir = FileManager.default.temporaryDirectory
        let tmpURL = tmpDir.appendingPathComponent(fileName)
        do {
            try png.write(to: tmpURL, options: .atomic)
        } catch {
            status = .error("Couldn't stage image to disk: \(error.localizedDescription)")
            return
        }

        guard let copied = doc.moveFile(toShowFolder: tmpURL.path,
                                         subdirectory: "AIImages") else {
            status = .error("Couldn't copy image into the show folder.")
            try? FileManager.default.removeItem(at: tmpURL)
            return
        }
        try? FileManager.default.removeItem(at: tmpURL)

        let stored = doc.makeRelativePath(copied)
        onCommitPath(stored)
        dismiss()
    }

    // MARK: - Black background removal

    /// Flood-fill from all near-black border pixels (tolerance 25/255 per channel),
    /// setting their alpha to 0. Mirrors desktop AIImageDialog.cpp RemoveBlackBackground.
    private func removeBlackBackgroundFromImage(_ src: UIImage) -> UIImage? {
        guard let cgSrc = src.cgImage else { return nil }
        let width = cgSrc.width
        let height = cgSrc.height
        guard width > 0 && height > 0 else { return nil }

        let bytesPerPixel = 4
        let bytesPerRow = width * bytesPerPixel
        var pixels = [UInt8](repeating: 0, count: height * bytesPerRow)

        guard let ctx = CGContext(
            data: &pixels,
            width: width,
            height: height,
            bitsPerComponent: 8,
            bytesPerRow: bytesPerRow,
            space: CGColorSpaceCreateDeviceRGB(),
            bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue
        ) else { return nil }

        ctx.draw(cgSrc, in: CGRect(x: 0, y: 0, width: width, height: height))

        // Un-premultiply so channel comparisons are against the actual colour values.
        for i in stride(from: 0, to: pixels.count, by: bytesPerPixel) {
            let a = pixels[i + 3]
            if a > 0 && a < 255 {
                let scale = 255.0 / Double(a)
                pixels[i]     = UInt8(min(255, Double(pixels[i])     * scale))
                pixels[i + 1] = UInt8(min(255, Double(pixels[i + 1]) * scale))
                pixels[i + 2] = UInt8(min(255, Double(pixels[i + 2]) * scale))
            }
        }

        let tolerance: UInt8 = 25
        var visited = [Bool](repeating: false, count: width * height)
        var queue: [(Int, Int)] = []
        queue.reserveCapacity(width * 2 + height * 2)

        func isBlack(_ x: Int, _ y: Int) -> Bool {
            let base = (y * width + x) * bytesPerPixel
            return pixels[base] <= tolerance
                && pixels[base + 1] <= tolerance
                && pixels[base + 2] <= tolerance
        }

        func seed(_ x: Int, _ y: Int) {
            let idx = y * width + x
            guard !visited[idx] && isBlack(x, y) else { return }
            visited[idx] = true
            pixels[(y * width + x) * bytesPerPixel + 3] = 0
            queue.append((x, y))
        }

        for x in 0..<width {
            seed(x, 0)
            seed(x, height - 1)
        }
        for y in 0..<height {
            seed(0, y)
            seed(width - 1, y)
        }

        let dx = [0, 0, -1, 1]
        let dy = [-1, 1, 0, 0]

        var qi = 0
        while qi < queue.count {
            let (cx, cy) = queue[qi]
            qi += 1
            for d in 0..<4 {
                let nx = cx + dx[d]
                let ny = cy + dy[d]
                guard nx >= 0 && nx < width && ny >= 0 && ny < height else { continue }
                let idx = ny * width + nx
                if !visited[idx] && isBlack(nx, ny) {
                    visited[idx] = true
                    pixels[(ny * width + nx) * bytesPerPixel + 3] = 0
                    queue.append((nx, ny))
                }
            }
        }

        guard let outCG = ctx.makeImage() else { return nil }
        return UIImage(cgImage: outCG, scale: src.scale, orientation: src.imageOrientation)
    }
}

// MARK: - Crop / Resize sheet

private struct CropResizeSheet: View {
    let image: UIImage
    let onApply: (UIImage) -> Void

    @Environment(\.dismiss) private var dismiss

    @State private var cropRect: CGRect = .zero
    @State private var isDragging: Bool = false
    @State private var dragStart: CGPoint = .zero
    @State private var renderedSize: CGSize = .zero

    @State private var targetWidth: String = ""
    @State private var targetHeight: String = ""
    @State private var lockAspect: Bool = true
    @State private var scaleQuality: ScaleQuality = .high

    private enum ScaleQuality: String, CaseIterable, Identifiable {
        case normal = "Normal"
        case bilinear = "Bilinear"
        case bicubic = "Bicubic"
        case high = "High"
        var id: String { rawValue }
    }

    var body: some View {
        NavigationStack {
            Form {
                Section(content: {
                    GeometryReader { geo in
                        let displaySize = fitSize(image: image, in: geo.size)
                        ZStack(alignment: .topLeading) {
                            Image(uiImage: image)
                                .resizable()
                                .scaledToFit()
                                .frame(width: displaySize.width, height: displaySize.height)
                                .background(GeometryReader { g in
                                    Color.clear.onAppear { renderedSize = g.size }
                                })

                            if cropRect != .zero {
                                Rectangle()
                                    .stroke(Color.yellow, lineWidth: 2)
                                    .background(Color.yellow.opacity(0.1))
                                    .frame(width: max(2, cropRect.width), height: max(2, cropRect.height))
                                    .offset(x: cropRect.minX, y: cropRect.minY)
                                    .allowsHitTesting(false)
                            }
                        }
                        .contentShape(Rectangle())
                        .gesture(
                            DragGesture(minimumDistance: 2)
                                .onChanged { v in
                                    if !isDragging {
                                        isDragging = true
                                        dragStart = v.startLocation
                                    }
                                    let origin = CGPoint(
                                        x: min(dragStart.x, v.location.x),
                                        y: min(dragStart.y, v.location.y)
                                    )
                                    let sz = CGSize(
                                        width: abs(v.location.x - dragStart.x),
                                        height: abs(v.location.y - dragStart.y)
                                    )
                                    let ds = fitSize(image: image, in: geo.size)
                                    cropRect = CGRect(origin: origin, size: sz)
                                        .intersection(CGRect(origin: .zero, size: ds))
                                }
                                .onEnded { _ in isDragging = false }
                        )
                        .frame(width: displaySize.width, height: displaySize.height)
                    }
                    .frame(height: 300)
                }, header: {
                    Text("Crop")
                }, footer: {
                    Text("Drag to select a crop region. Leave empty to skip cropping.")
                        .font(.caption)
                })

                if cropRect != .zero {
                    Section {
                        Button("Clear crop selection") {
                            cropRect = .zero
                        }
                        .foregroundStyle(.red)
                    }
                }

                Section(content: {
                    LabeledContent("Current size") {
                        Text("\(Int(image.size.width)) × \(Int(image.size.height))")
                            .foregroundStyle(.secondary)
                    }
                    HStack {
                        Text("Width")
                        Spacer()
                        TextField("px", text: $targetWidth)
                            .keyboardType(.numberPad)
                            .multilineTextAlignment(.trailing)
                            .frame(width: 80)
                            .onChange(of: targetWidth) { _, w in
                                guard lockAspect, let wv = Int(w), wv > 0 else { return }
                                let ratio = image.size.height / image.size.width
                                targetHeight = "\(Int((Double(wv) * ratio).rounded()))"
                            }
                    }
                    HStack {
                        Text("Height")
                        Spacer()
                        TextField("px", text: $targetHeight)
                            .keyboardType(.numberPad)
                            .multilineTextAlignment(.trailing)
                            .frame(width: 80)
                            .onChange(of: targetHeight) { _, h in
                                guard lockAspect, let hv = Int(h), hv > 0 else { return }
                                let ratio = image.size.width / image.size.height
                                targetWidth = "\(Int((Double(hv) * ratio).rounded()))"
                            }
                    }
                    Toggle("Lock aspect ratio", isOn: $lockAspect)
                    Picker("Quality", selection: $scaleQuality) {
                        ForEach(ScaleQuality.allCases) { q in
                            Text(q.rawValue).tag(q)
                        }
                    }
                }, header: {
                    Text("Resize")
                }, footer: {
                    Text("Leave width/height empty to skip resizing.")
                        .font(.caption)
                })
            }
            .navigationTitle("Crop / Resize")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .topBarLeading) {
                    Button("Cancel") { dismiss() }
                }
                ToolbarItem(placement: .topBarTrailing) {
                    Button("Apply") { applyAndDismiss() }
                }
            }
        }
        .onAppear {
            targetWidth = "\(Int(image.size.width))"
            targetHeight = "\(Int(image.size.height))"
        }
    }

    private func fitSize(image: UIImage, in container: CGSize) -> CGSize {
        guard container.width > 0 && container.height > 0 else { return container }
        let ir = image.size.width / image.size.height
        let cr = container.width / container.height
        if ir > cr {
            return CGSize(width: container.width, height: container.width / ir)
        } else {
            return CGSize(width: container.height * ir, height: container.height)
        }
    }

    private func applyAndDismiss() {
        var result = image

        if cropRect != .zero {
            let ds = fitSize(image: image, in: renderedSize.width > 0 ? renderedSize : CGSize(width: 300, height: 300))
            let scaleX = image.size.width / ds.width
            let scaleY = image.size.height / ds.height
            let imgCrop = CGRect(
                x: cropRect.minX * scaleX,
                y: cropRect.minY * scaleY,
                width: cropRect.width * scaleX,
                height: cropRect.height * scaleY
            )
            if let cropped = cropImage(result, to: imgCrop) {
                result = cropped
            }
        }

        let wv = Int(targetWidth) ?? 0
        let hv = Int(targetHeight) ?? 0
        if wv > 0 && hv > 0 && CGSize(width: wv, height: hv) != result.size {
            if let scaled = scaleImage(result, to: CGSize(width: wv, height: hv)) {
                result = scaled
            }
        }

        onApply(result)
        dismiss()
    }

    private func cropImage(_ img: UIImage, to rect: CGRect) -> UIImage? {
        let scale = img.scale
        let scaledRect = CGRect(
            x: rect.origin.x * scale,
            y: rect.origin.y * scale,
            width: rect.width * scale,
            height: rect.height * scale
        )
        guard let cgImg = img.cgImage,
              let cropped = cgImg.cropping(to: scaledRect) else { return nil }
        return UIImage(cgImage: cropped, scale: scale, orientation: img.imageOrientation)
    }

    private func scaleImage(_ img: UIImage, to size: CGSize) -> UIImage? {
        let format = UIGraphicsImageRendererFormat()
        format.scale = 1.0
        let renderer = UIGraphicsImageRenderer(size: size, format: format)
        return renderer.image { _ in
            img.draw(in: CGRect(origin: .zero, size: size))
        }
    }
}
