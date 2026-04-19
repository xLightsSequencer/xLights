import SwiftUI
import UniformTypeIdentifiers

// Compound file-selection row used by the Pictures / Video / Shader
// effects' `*_FilenameBlock` custom controls. Mirrors the desktop widget:
// file-name label + Select + Clear, backed by a hidden text/filepicker
// control that stores the absolute path.
//
// Pictures_FilenameBlock  → E_TEXTCTRL_Pictures_Filename
// Video_FilenameBlock     → E_FILEPICKERCTRL_Video_Filename
// Shader_FilenameBlock    → E_0FILEPICKERCTRL_IFS
//
// Picked files that are outside the show / media folders go through
// `MediaRelocation` → prompt user for a destination (show folder or
// one of the configured media folders) and copy the file there before
// storing the path. Files already inside an enforced root are stored
// as-is (relative to the show folder when possible). No stored path
// ever points outside the iPad's enforced media roots.
struct EffectFilenameBlockView: View {
    @Environment(SequencerViewModel.self) var viewModel
    let label: String
    let settingKey: String
    /// wx pipe-delimited filter string, e.g. `"Images (*.png)|*.png"`.
    let fileFilter: String
    /// Destination subdirectory under the chosen root (show or media
    /// folder). Desktop uses `/Images`, `/Videos`, `/Shaders`, etc. —
    /// each effect has its own canonical location.
    let subdirectory: String

    @State private var presentingPicker = false
    @State private var pendingPick: URL?

    private var currentPath: String {
        viewModel.settingValue(forKey: settingKey, defaultValue: "")
    }

    private var filename: String {
        if currentPath.isEmpty { return "(none)" }
        return (currentPath as NSString).lastPathComponent
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 2) {
            Text(label)
                .font(.caption)
            HStack(spacing: 6) {
                Text(filename)
                    .font(.caption2)
                    .foregroundStyle(.secondary)
                    .lineLimit(1)
                    .truncationMode(.middle)
                    .frame(maxWidth: .infinity, alignment: .leading)
                Button("Select…") { presentingPicker = true }
                    .buttonStyle(.bordered)
                    .controlSize(.small)
                if !currentPath.isEmpty {
                    Button(action: { viewModel.setSettingValue("", forKey: settingKey) }) {
                        Image(systemName: "xmark.circle.fill")
                            .foregroundStyle(.secondary)
                    }
                    .buttonStyle(.plain)
                }
            }
        }
        .padding(.vertical, 2)
        .fileImporter(isPresented: $presentingPicker,
                      allowedContentTypes: allowedTypes()) { result in
            if case .success(let url) = result {
                pendingPick = url
            }
        }
        .mediaRelocationPrompt(
            picked: $pendingPick,
            subdirectory: subdirectory
        ) { storedPath in
            viewModel.setSettingValue(storedPath, forKey: settingKey)
        }
    }

    private func allowedTypes() -> [UTType] {
        var types: [UTType] = []
        let parts = fileFilter.split(separator: "|").map(String.init)
        for (i, part) in parts.enumerated() where i % 2 == 1 {
            for pattern in part.split(separator: ";") {
                let glob = String(pattern).trimmingCharacters(in: .whitespaces)
                if let dotIdx = glob.lastIndex(of: ".") {
                    let ext = String(glob[glob.index(after: dotIdx)...])
                    if ext == "*" { continue }
                    if let t = UTType(filenameExtension: ext) {
                        types.append(t)
                    }
                }
            }
        }
        return types.isEmpty ? [.data] : types
    }
}

// Compound Transparent-Black row used by the Pictures / Video / Faces
// effects' `*_TransparentBlackRow` custom controls. Two backing settings:
// `E_CHECKBOX_<Prefix>_TransparentBlack` (bool)
// `E_TEXTCTRL_<Prefix>_TransparentBlack` (int, 0-300 threshold)
// Threshold slider is disabled when the checkbox is off (matches desktop).
struct TransparentBlackRowView: View {
    @Environment(SequencerViewModel.self) var viewModel
    /// Prefix injected into the setting keys — "Pictures", "Video", "Faces".
    let effectKeyStem: String

    private var checkKey: String { "E_CHECKBOX_\(effectKeyStem)_TransparentBlack" }
    private var levelKey: String { "E_TEXTCTRL_\(effectKeyStem)_TransparentBlack" }

    private var enabled: Bool {
        let v = viewModel.settingValue(forKey: checkKey, defaultValue: "0")
        return v == "1" || v.lowercased() == "true"
    }

    private var level: Int {
        Int(viewModel.settingValue(forKey: levelKey, defaultValue: "0")) ?? 0
    }

    var body: some View {
        let enabledBinding = Binding<Bool>(
            get: { enabled },
            set: { viewModel.setSettingValue($0 ? "1" : "0", forKey: checkKey) }
        )
        let levelBinding = Binding<Double>(
            get: { Double(level) },
            set: { viewModel.setSettingValue(String(Int($0)), forKey: levelKey) }
        )

        VStack(alignment: .leading, spacing: 2) {
            HStack {
                Toggle(isOn: enabledBinding) {
                    Text("Transparent Black").font(.caption)
                }
                .toggleStyle(.switch)
                .controlSize(.small)
                Spacer()
                Text("\(level)")
                    .monospacedDigit()
                    .font(.caption2)
                    .foregroundStyle(.secondary)
            }
            Slider(value: levelBinding, in: 0...300, step: 1)
                .disabled(!enabled)
                .opacity(enabled ? 1.0 : 0.4)
        }
        .padding(.vertical, 2)
    }
}
