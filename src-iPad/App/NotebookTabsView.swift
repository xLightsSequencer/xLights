import SwiftUI

// Segmented notebook for JSON `type: "tabs"` groups. On desktop these
// render as `wxNotebook` — tab strip on top, one tab's properties
// visible at a time. Previous iPad behaviour rendered them as stacked
// `DisclosureGroup`s, which doesn't match the "pick one mode" semantic
// several effects rely on (SingleStrand's Chase/Skips/FX choice, Fan's
// notebook, etc.).
//
// When the group declares a `settingKey` (e.g.
// `E_NOTEBOOK_SSEFFECT_TYPE`), the selected tab's *label* is persisted
// to the settings map. SingleStrandEffect reads that string verbatim
// at render time to dispatch between RenderSingleStrandChase /
// RenderSingleStrandSkips / RenderSingleStrandFX — so iPad MUST write
// the label (not the index) for the render to switch modes.
//
// Groups without a `settingKey` (Buffer panel's Buffer / Roto-Zoom
// tabs) keep their selection as local `@State` — purely a UI toggle.
struct NotebookTabsView: View {
    @Environment(SequencerViewModel.self) var viewModel

    let tabs: [GroupMetadata.TabDef]
    /// Full-prefix settings-map key (e.g. `E_NOTEBOOK_SSEFFECT_TYPE`).
    /// When nil the picker selection is ephemeral.
    let settingKey: String?
    let metadataPrefix: String
    let propsById: [String: PropertyMetadata]
    let isVisible: (PropertyMetadata) -> Bool
    /// Returns false when a JSON visibility-rule `enable`/`disable`
    /// clause greys this prop out. Defaults to "always enabled" so
    /// existing callers don't need to update if they don't care.
    var isEnabled: (PropertyMetadata) -> Bool = { _ in true }

    /// Local fallback selection for groups without a settingKey.
    @State private var localSelection: String = ""

    private var labels: [String] { tabs.map(\.label) }

    private var activeLabel: String {
        if let key = settingKey {
            let stored = viewModel.settingValue(forKey: key, defaultValue: "")
            if labels.contains(stored) { return stored }
        } else if labels.contains(localSelection) {
            return localSelection
        }
        return labels.first ?? ""
    }

    var body: some View {
        let selection = Binding<String>(
            get: { activeLabel },
            set: { newLabel in
                if let key = settingKey {
                    // Desktop suppresses the default (first tab's
                    // label) from the effect string — empty string in
                    // the map means "use first tab". Writing the first
                    // tab's label explicitly would bloat the string
                    // without changing behaviour, so match that and
                    // clear on the default branch.
                    let first = labels.first ?? ""
                    viewModel.setSettingValue(newLabel,
                                               forKey: key,
                                               suppressIfDefault: first)
                } else {
                    localSelection = newLabel
                }
            }
        )

        VStack(alignment: .leading, spacing: 4) {
            // Pick the presentation based on tab count: segmented when
            // it fits (≤4), menu otherwise — inspector sidebar width
            // can't comfortably render 5+ segments.
            if tabs.count <= 4 {
                Picker("Tab", selection: selection) {
                    ForEach(labels, id: \.self) { (l: String) in
                        Text(l).tag(l)
                    }
                }
                .pickerStyle(.segmented)
            } else {
                HStack {
                    Text("Mode")
                        .font(.caption)
                    Spacer()
                    Picker("Tab", selection: selection) {
                        ForEach(labels, id: \.self) { (l: String) in
                            Text(l).tag(l)
                        }
                    }
                    .pickerStyle(.menu)
                    .labelsHidden()
                }
            }

            // Render only the active tab's properties. Hidden tabs are
            // completely absent from the view tree so their property
            // bindings can't generate stray writes.
            if let active = tabs.first(where: { $0.label == activeLabel }) {
                VStack(alignment: .leading, spacing: 0) {
                    ForEach(active.properties, id: \.self) { (propId: String) in
                        if let prop = propsById[propId], isVisible(prop) {
                            if prop.separator == true {
                                Divider().padding(.vertical, 2)
                            }
                            EffectPropertyView(property: prop,
                                               metadataPrefix: metadataPrefix,
                                               ruleDisabled: !isEnabled(prop))
                        }
                    }
                }
            }
        }
        .padding(.vertical, 2)
    }
}
