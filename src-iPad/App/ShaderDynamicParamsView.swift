import SwiftUI

// Custom row for the Shader effect's `Shader_DynamicParams` slot.
// Parses the currently-picked `.fs` file via the bridge, decodes the
// JSON-shaped property array (same schema as static effect metadata),
// and renders each entry through `EffectPropertyView` so shader-specific
// uniforms get sliders / choices / checkboxes / point2d pads with the
// same feature set as a native effect (editable value field, VC button,
// suppressIfDefault, etc.).
//
// Re-parses when the shader path setting changes so the dynamic UI
// tracks shader switches without the user having to deselect/reselect.
struct ShaderDynamicParamsView: View {
    @Environment(SequencerViewModel.self) var viewModel

    /// The shader-filename setting the Shader_FilenameBlock writes.
    private let shaderPathKey = "E_0FILEPICKERCTRL_IFS"

    private var shaderPath: String {
        viewModel.settingValue(forKey: shaderPathKey, defaultValue: "")
    }

    @State private var cachedPath: String = ""
    @State private var properties: [PropertyMetadata] = []

    var body: some View {
        VStack(alignment: .leading, spacing: 0) {
            if shaderPath.isEmpty {
                Text("Pick a shader file to configure its parameters.")
                    .font(.caption)
                    .foregroundStyle(.secondary)
                    .padding(.vertical, 4)
            } else if properties.isEmpty {
                Text("No adjustable parameters for this shader.")
                    .font(.caption2)
                    .foregroundStyle(.tertiary)
                    .padding(.vertical, 4)
            } else {
                ForEach(Array(properties.enumerated()), id: \.offset) { _, prop in
                    EffectPropertyView(property: prop, metadataPrefix: "E_")
                }
            }
        }
        .onAppear { refreshIfNeeded() }
        .onChange(of: shaderPath) { _, _ in refreshIfNeeded() }
    }

    private func refreshIfNeeded() {
        let path = shaderPath
        if path == cachedPath && !(properties.isEmpty && !path.isEmpty) {
            // Same path as last parse; re-parsing is wasted work.
            return
        }
        cachedPath = path
        properties = loadProperties(for: path)
    }

    /// Parse the `.fs` via the bridge and extract a list of
    /// `PropertyMetadata`. Each entry in the JSON array is already
    /// schema-shaped so `JSONDecoder` handles it directly.
    private func loadProperties(for path: String) -> [PropertyMetadata] {
        guard !path.isEmpty else { return [] }
        let json = viewModel.document.shaderDynamicPropertiesJson(forPath: path) ?? ""
        guard !json.isEmpty,
              let data = json.data(using: .utf8) else {
            return []
        }
        do {
            return try JSONDecoder().decode([PropertyMetadata].self, from: data)
        } catch {
            // Don't hide the error silently — shader metadata changes
            // occasionally and a parse failure is worth surfacing
            // during development. Fall back to empty so the panel
            // still renders the rest of the effect.
            print("ShaderDynamicParamsView decode failed: \(error)")
            return []
        }
    }
}
