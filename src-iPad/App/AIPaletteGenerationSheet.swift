import SwiftUI

/// AI color-palette generation sheet — invoked from the Color
/// Panel's palette menu. Mirrors the desktop `AIColorPaletteDialog`:
/// a Song / Free-Form prompt mode, a service picker, a generate
/// button, and an 8-swatch preview that the user applies to the
/// effect's palette slots.
///
/// Song mode pre-fills the prompt with the loaded sequence's
/// `<title>` (and " by <artist>" if present) so the typical
/// authoring flow is one-tap.
struct AIPaletteGenerationSheet: View {
    @Environment(SequencerViewModel.self) var viewModel
    @Environment(\.dismiss) private var dismiss

    @State private var availableServices: [String] = []
    @State private var selectedService: String = ""

    @State private var mode: PromptMode = .song
    @State private var promptText: String = ""

    @State private var status: Status = .idle
    @State private var resultColors: [XLAIPaletteColor] = []
    @State private var resultDescription: String = ""

    enum PromptMode: String, CaseIterable, Identifiable {
        case song = "Song"
        case freeform = "Free Form"
        var id: String { rawValue }
    }

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
                             No palette-generation service is configured. \
                             Open Tools → AI Services… to set up Claude or \
                             another provider with the Color Palette \
                             capability enabled.
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
                    }
                }

                Section {
                    Picker("Mode", selection: $mode) {
                        ForEach(PromptMode.allCases) { m in
                            Text(m.rawValue).tag(m)
                        }
                    }
                    .pickerStyle(.segmented)
                    .onChange(of: mode) { _, _ in seedPromptForCurrentMode() }

                    TextEditor(text: $promptText)
                        .frame(minHeight: 88)
                        .overlay(alignment: .topLeading) {
                            if promptText.isEmpty {
                                Text(placeholderForMode(mode))
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
                                Image(systemName: "paintpalette.fill")
                                Text(status == .ready ? "Regenerate" : "Generate")
                            }
                            Spacer()
                        }
                    }
                    .disabled(effectivePrompt().isEmpty
                              || selectedService.isEmpty
                              || status == .running)

                    if case .error(let msg) = status {
                        Label(msg, systemImage: "xmark.octagon.fill")
                            .foregroundStyle(.red)
                            .font(.footnote)
                    }
                }

                if !resultColors.isEmpty {
                    Section("Suggested Palette") {
                        if !resultDescription.isEmpty {
                            Text(resultDescription)
                                .font(.footnote)
                                .foregroundStyle(.secondary)
                        }
                        ForEach(0..<resultColors.count, id: \.self) { i in
                            paletteRow(index: i, color: resultColors[i])
                        }
                        Button {
                            applyPalette()
                        } label: {
                            Label("Apply to Palette", systemImage: "checkmark.circle.fill")
                        }
                    }
                }
            }
            .navigationTitle("AI Palette")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .topBarLeading) {
                    Button("Cancel") { dismiss() }
                }
            }
        }
        .onAppear {
            loadServices()
            seedPromptForCurrentMode()
        }
    }

    // MARK: - Result row

    @ViewBuilder
    private func paletteRow(index: Int, color: XLAIPaletteColor) -> some View {
        HStack(spacing: 12) {
            RoundedRectangle(cornerRadius: 6)
                .fill(swatchColor(color.hexValue) ?? .gray)
                .frame(width: 36, height: 36)
                .overlay(
                    RoundedRectangle(cornerRadius: 6)
                        .stroke(Color.secondary.opacity(0.3), lineWidth: 0.5)
                )
            VStack(alignment: .leading, spacing: 2) {
                Text(color.name.isEmpty ? "Color \(index + 1)" : color.name)
                    .font(.subheadline)
                Text(color.hexValue)
                    .font(.caption.monospaced())
                    .foregroundStyle(.secondary)
                if !color.descriptionText.isEmpty {
                    Text(color.descriptionText)
                        .font(.caption)
                        .foregroundStyle(.secondary)
                        .lineLimit(2)
                }
            }
            Spacer()
        }
    }

    private func swatchColor(_ hex: String) -> Color? {
        var s = hex.trimmingCharacters(in: .whitespaces)
        if s.hasPrefix("#") { s.removeFirst() }
        guard s.count == 6, let v = UInt64(s, radix: 16) else { return nil }
        return Color(red: Double((v >> 16) & 0xFF) / 255.0,
                     green: Double((v >> 8) & 0xFF) / 255.0,
                     blue: Double(v & 0xFF) / 255.0)
    }

    // MARK: - Service discovery

    private func loadServices() {
        let all = XLAIServices.shared().allServices()
        availableServices = all
            .filter { $0.available && $0.capabilities.contains(XLAICapabilityColorPalettes) }
            .map { $0.name }
        if selectedService.isEmpty || !availableServices.contains(selectedService) {
            selectedService = availableServices.first ?? ""
        }
    }

    // MARK: - Prompt seeding

    private func seedPromptForCurrentMode() {
        switch mode {
        case .song:
            // Match desktop's pre-fill: "<title>" or "<title> by <artist>"
            let doc = viewModel.document
            let title = doc.audioTitle()
            let artist = doc.audioArtist()
            if title.isEmpty && artist.isEmpty {
                // No metadata — leave whatever the user already typed.
                if promptText.isEmpty {
                    promptText = ""
                }
            } else {
                let combined = artist.isEmpty ? title : "\(title) by \(artist)"
                promptText = combined
            }
        case .freeform:
            // Don't clobber what the user typed when they switch
            // between modes deliberately.
            break
        }
    }

    private func placeholderForMode(_ m: PromptMode) -> String {
        switch m {
        case .song:
            return "Song title (and artist) — e.g. Carol of the Bells by Trans-Siberian Orchestra"
        case .freeform:
            return "Describe the mood, scene, or imagery you want…"
        }
    }

    private func effectivePrompt() -> String {
        let raw = promptText.trimmingCharacters(in: .whitespacesAndNewlines)
        if raw.isEmpty { return "" }
        switch mode {
        case .song:
            // Desktop wraps the song text in "from the song <title>" so
            // the LLM's palette is themed, not just based on the
            // literal title string. Match that.
            return "from the song \(raw)"
        case .freeform:
            return raw
        }
    }

    // MARK: - Generate / apply

    private func runGenerate() {
        let prompt = effectivePrompt()
        guard !prompt.isEmpty, !selectedService.isEmpty else { return }
        status = .running
        resultColors = []
        resultDescription = ""

        XLAIServices.shared().generateColorPalette(
            prompt: prompt,
            forService: selectedService) { colors, err in
            // The bridge marshals this completion to main, but
            // Swift sees the ObjC block as non-isolated @Sendable.
            // Hop to MainActor so the @State mutations are safe.
            Task { @MainActor in
                if let err = err {
                    status = .error(err)
                    return
                }
                guard let colors = colors, !colors.isEmpty else {
                    status = .error("AI returned no colors")
                    return
                }
                resultColors = Array(colors.prefix(8))
                // The core's AIColorPalette has a separate description;
                // we don't get it back through the bridge today (only
                // colors). Leave description blank — the per-color
                // descriptionText carries the useful detail.
                status = .ready
            }
        }
    }

    private func applyPalette() {
        guard !resultColors.isEmpty else { return }

        // Write hex into C_BUTTON_PaletteN and enable C_CHECKBOX_PaletteN.
        // Limit to 8 slots; trim any leading "#" the model may have
        // missed and re-add it (palette parser accepts both, but we
        // want consistent output).
        for (i, c) in resultColors.prefix(8).enumerated() {
            let slot = i + 1
            let hex = normaliseHex(c.hexValue)
            guard !hex.isEmpty else { continue }
            viewModel.setSettingValue(hex, forKey: "C_BUTTON_Palette\(slot)")
            viewModel.setSettingValue("1", forKey: "C_CHECKBOX_Palette\(slot)")
        }
        dismiss()
    }

    private func normaliseHex(_ raw: String) -> String {
        var s = raw.trimmingCharacters(in: .whitespaces)
        if s.hasPrefix("#") { s.removeFirst() }
        guard s.count == 6, s.allSatisfy({ $0.isHexDigit }) else { return "" }
        return "#\(s.uppercased())"
    }
}

private extension Character {
    var isHexDigit: Bool {
        return self.isNumber || ("a"..."f").contains(self.lowercased().first ?? " ")
    }
}
