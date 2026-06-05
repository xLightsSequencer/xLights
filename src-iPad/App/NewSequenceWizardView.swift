import SwiftUI
import UniformTypeIdentifiers

// E-2 — New Sequence wizard. Multi-step sheet that mirrors the
// desktop SeqSettingsDialog "Wizard" mode. Three pages:
//
//   1. Type picker — Musical / Animation / Effect. Drives which
//      inputs appear on step 2.
//   2. Content — Musical asks for a media file (routed through
//      `MediaRelocation` so the file lands under the show folder
//      or a configured media folder); Animation / Effect ask for
//      a duration + frame rate.
//   3. Save — pick a destination under the show folder. Presents
//      the system file exporter with a suggested filename.
//
// On completion the view model's `newSequence(...)` call creates
// the on-disk `.xsq` via the bridge and hands off to the normal
// open path so the sequence is live immediately.
//
// Timing-track import is intentionally deferred to post-wizard
// (users can add timing tracks through the already-shipped
// row-header actions).

struct NewSequenceWizardView: View {
    @Environment(\.dismiss) private var dismiss
    @Environment(SequencerViewModel.self) var viewModel

    @State private var step: Step = .type
    @State private var type: SequenceType = .musical
    @State private var mediaPath: String = ""
    @State private var durationSeconds: Double = 30
    @State private var frameMS: Int = 50
    @State private var showingSaveExporter = false
    @State private var saveDoc: XLSequenceExportDoc? = nil
    @State private var defaultSaveName: String = "New Sequence.xsq"
    @State private var errorMessage: String? = nil

    // Picked-but-unresolved media URL passed through MediaRelocation
    @State private var pickedMediaURL: URL? = nil
    @State private var pickingMediaFile: Bool = false

    private enum Step: Int, Hashable { case type, content, save }

    private enum SequenceType: String, CaseIterable, Identifiable {
        case musical, animation, effect
        var id: String { rawValue }
        var label: String {
            switch self {
            case .musical:   return "Musical"
            case .animation: return "Animation"
            case .effect:    return "Effect"
            }
        }
        var description: String {
            switch self {
            case .musical:
                return "A sequence synced to an audio file. Duration is derived from the media."
            case .animation:
                return "A free-standing animation with a user-specified duration. No audio."
            case .effect:
                return "A short effect template for preset authoring. No models or audio."
            }
        }
        var bridgeString: String {
            switch self {
            case .musical:   return "Media"
            case .animation: return "Animation"
            case .effect:    return "Effect"
            }
        }
    }

    var body: some View {
        NavigationStack {
            VStack(spacing: 0) {
                stepIndicator
                Divider()
                Group {
                    switch step {
                    case .type:    typePage
                    case .content: contentPage
                    case .save:    savePage
                    }
                }
                .padding()
                Spacer()
                Divider()
                bottomBar
            }
            .navigationTitle("New Sequence")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                }
            }
            .alert("Couldn't Create Sequence",
                   isPresented: Binding(
                    get: { errorMessage != nil },
                    set: { if !$0 { errorMessage = nil } }
                   )) {
                Button("OK", role: .cancel) { errorMessage = nil }
            } message: {
                Text(errorMessage ?? "")
            }
            .fileImporter(isPresented: $pickingMediaFile,
                          allowedContentTypes: [.audio]) { result in
                if case .success(let url) = result {
                    pickedMediaURL = url
                }
            }
            .mediaRelocationPrompt(
                picked: $pickedMediaURL,
                subdirectory: ""  // Music lives at show root by convention
            ) { storedPath in
                mediaPath = storedPath
            }
            .fileExporter(
                isPresented: $showingSaveExporter,
                document: saveDoc,
                contentType: kXSQFileType,
                defaultFilename: defaultSaveName
            ) { result in
                if case .success(let url) = result {
                    finishWizard(with: url.path)
                } else {
                    saveDoc = nil
                }
            }
            // Sequences belong in the show folder, not wherever the
            // media file was sourced from. The system file picker
            // would otherwise default to the directory of the most
            // recently picked file (the music) — which on this app
            // is often a configured media folder, not the show
            // folder. Pin the picker to the show folder so the
            // default location matches user expectations.
            .fileDialogDefaultDirectory(showFolderURL)
        }
    }

    // MARK: - Step indicator

    private var stepIndicator: some View {
        HStack(spacing: 8) {
            stepPill("Type", isActive: step == .type, isDone: step.rawValue > Step.type.rawValue)
            connector
            stepPill("Content", isActive: step == .content, isDone: step.rawValue > Step.content.rawValue)
            connector
            stepPill("Save", isActive: step == .save, isDone: false)
        }
        .font(.caption)
        .padding(.horizontal)
        .padding(.vertical, 6)
        .frame(maxWidth: .infinity)
    }

    private var connector: some View {
        Rectangle()
            .fill(Color.secondary.opacity(0.3))
            .frame(height: 1)
            .frame(maxWidth: 40)
    }

    private func stepPill(_ label: String, isActive: Bool, isDone: Bool) -> some View {
        let tint: Color = isActive ? .accentColor : (isDone ? .green : .secondary)
        return HStack(spacing: 4) {
            Image(systemName: isDone ? "checkmark.circle.fill" : "circle")
                .foregroundStyle(tint)
            Text(label)
                .foregroundStyle(isActive ? .primary : .secondary)
        }
    }

    // MARK: - Type

    private var typePage: some View {
        VStack(alignment: .leading, spacing: 12) {
            Text("Choose sequence type")
                .font(.headline)
            ForEach(SequenceType.allCases) { t in
                Button {
                    type = t
                } label: {
                    HStack(alignment: .top, spacing: 10) {
                        Image(systemName: type == t ? "largecircle.fill.circle" : "circle")
                            .foregroundStyle(.tint)
                            .padding(.top, 2)
                        VStack(alignment: .leading, spacing: 2) {
                            Text(t.label)
                                .font(.body)
                                .fontWeight(.semibold)
                            Text(t.description)
                                .font(.caption)
                                .foregroundStyle(.secondary)
                        }
                    }
                    .frame(maxWidth: .infinity, alignment: .leading)
                    .contentShape(Rectangle())
                }
                .buttonStyle(.plain)
                .padding(8)
                .background(
                    RoundedRectangle(cornerRadius: 8)
                        .stroke(type == t ? Color.accentColor : Color.secondary.opacity(0.3),
                                lineWidth: type == t ? 2 : 1)
                )
            }
        }
    }

    // MARK: - Content

    @ViewBuilder
    private var contentPage: some View {
        VStack(alignment: .leading, spacing: 14) {
            if type == .musical {
                Text("Choose media file")
                    .font(.headline)
                Text("The sequence's duration will be inferred from the picked audio file. Copy the file into your show folder if it isn't already there.")
                    .font(.caption)
                    .foregroundStyle(.secondary)
                HStack {
                    Text(mediaPath.isEmpty ? "(none)" : mediaPath)
                        .font(.caption)
                        .foregroundStyle(.secondary)
                        .lineLimit(1)
                        .truncationMode(.middle)
                        .frame(maxWidth: .infinity, alignment: .leading)
                    Button("Browse…") { pickingMediaFile = true }
                        .buttonStyle(.bordered)
                }
            } else {
                Text("Duration and frame rate")
                    .font(.headline)
                HStack {
                    Text("Duration (seconds)")
                    Spacer()
                    TextField("30", value: $durationSeconds, format: .number)
                        .keyboardType(.decimalPad)
                        .multilineTextAlignment(.trailing)
                        .frame(width: 90)
                        .textFieldStyle(.roundedBorder)
                }
                HStack {
                    Text("Frame interval")
                    Spacer()
                    Picker("Frame", selection: $frameMS) {
                        Text("25 ms").tag(25)
                        Text("50 ms").tag(50)
                        Text("100 ms").tag(100)
                    }
                    .pickerStyle(.segmented)
                    .frame(width: 200)
                }
            }
        }
    }

    // MARK: - Save

    private var savePage: some View {
        VStack(alignment: .leading, spacing: 12) {
            Text("Save the new sequence")
                .font(.headline)
            Text("The next tap opens the system file picker so you can pick a destination under your show folder.")
                .font(.caption)
                .foregroundStyle(.secondary)
            summaryView
            Spacer(minLength: 8)
        }
    }

    private var summaryView: some View {
        VStack(alignment: .leading, spacing: 4) {
            summaryRow("Type", type.label)
            if type == .musical {
                summaryRow("Media", mediaPath.isEmpty ? "—" : (mediaPath as NSString).lastPathComponent)
            } else {
                summaryRow("Duration", String(format: "%.1f s", durationSeconds))
                summaryRow("Frame", "\(frameMS) ms")
            }
        }
        .padding(10)
        .background(
            RoundedRectangle(cornerRadius: 8)
                .fill(Color.secondary.opacity(0.08))
        )
    }

    private func summaryRow(_ label: String, _ value: String) -> some View {
        HStack {
            Text(label)
                .font(.caption)
                .foregroundStyle(.secondary)
            Spacer()
            Text(value)
                .font(.caption)
                .fontWeight(.semibold)
        }
    }

    // MARK: - Bottom bar

    private var bottomBar: some View {
        HStack(spacing: 10) {
            Button("Back") {
                if step.rawValue > 0 {
                    step = Step(rawValue: step.rawValue - 1)!
                }
            }
            .disabled(step == .type)

            Spacer()

            Button(step == .save ? "Save…" : "Next") {
                advance()
            }
            .buttonStyle(.borderedProminent)
            .disabled(!canAdvance)
        }
        .padding(.horizontal)
        .padding(.vertical, 8)
    }

    private var canAdvance: Bool {
        switch step {
        case .type: return true
        case .content:
            if type == .musical { return !mediaPath.isEmpty }
            return durationSeconds > 0 && frameMS > 0
        case .save: return true
        }
    }

    private func advance() {
        switch step {
        case .type:
            step = .content
        case .content:
            step = .save
        case .save:
            // Present the system file exporter. FileDocument writes
            // an empty placeholder; we call the bridge after the
            // user picks a URL (finishWizard) to actually build the
            // sequence on disk at that location.
            saveDoc = XLSequenceExportDoc(sourcePath: "")
            defaultSaveName = suggestedName + ".xsq"
            showingSaveExporter = true
        }
    }

    // Show folder as a URL for `.fileDialogDefaultDirectory`. Returns
    // nil if the user hasn't picked a show folder yet — the picker
    // then falls back to its system default location.
    private var showFolderURL: URL? {
        guard let path = viewModel.showFolderPath, !path.isEmpty else { return nil }
        return URL(fileURLWithPath: path, isDirectory: true)
    }

    private var suggestedName: String {
        if type == .musical, !mediaPath.isEmpty {
            let base = (mediaPath as NSString).lastPathComponent
            return (base as NSString).deletingPathExtension
        }
        switch type {
        case .musical:   return "New Sequence"
        case .animation: return "New Animation"
        case .effect:    return "New Effect"
        }
    }

    private func finishWizard(with path: String) {
        let durationMS: Int
        switch type {
        case .musical:
            // Musical: defer the true duration to the media probe
            // that happens during load. Seed with the user's value
            // only if we happen to have one; 30s is a safe default.
            durationMS = Int(durationSeconds * 1000)
        case .animation, .effect:
            durationMS = Int(durationSeconds * 1000)
        }
        let ok = viewModel.newSequence(
            type: type.bridgeString,
            mediaPath: mediaPath,
            durationMS: durationMS,
            frameMS: frameMS,
            savePath: path)
        if ok {
            dismiss()
        } else {
            errorMessage = "The sequence couldn't be created at the selected location. Check file permissions and try again."
        }
        saveDoc = nil
    }
}
