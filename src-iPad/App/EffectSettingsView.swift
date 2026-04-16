import SwiftUI

// Inspector sidebar for the currently selected effect. Builds its UI from the
// effect's JSON metadata (resources/effectmetadata/<Name>.json) plus the three
// shared panels (Buffer, Color, Timing). Each shared panel is a collapsible
// section to save screen space on iPad.
struct EffectSettingsView: View {
    @Environment(SequencerViewModel.self) var viewModel

    @State private var bufferExpanded = false
    @State private var colorExpanded = false
    @State private var timingExpanded = false

    var body: some View {
        Group {
            if viewModel.selectedEffect != nil {
                ScrollView {
                    VStack(alignment: .leading, spacing: 0) {
                        header
                        Divider()
                        timingRow
                        Divider()

                        // Effect-specific properties (expanded by default)
                        if let md = viewModel.selectedEffectMetadata {
                            EffectMetadataPanel(metadata: md)
                                .padding(.horizontal, 8)
                                .padding(.vertical, 6)
                        } else {
                            Text("No metadata available for this effect")
                                .font(.caption)
                                .foregroundStyle(.secondary)
                                .padding()
                        }

                        Divider()

                        // Shared Buffer panel
                        sharedSection(
                            title: "Buffer",
                            metadata: viewModel.bufferMetadata,
                            expanded: $bufferExpanded
                        )

                        Divider()

                        // Shared Color panel
                        sharedSection(
                            title: "Color",
                            metadata: viewModel.colorMetadata,
                            expanded: $colorExpanded
                        )

                        Divider()

                        // Shared Timing panel
                        sharedSection(
                            title: "Timing",
                            metadata: viewModel.timingMetadata,
                            expanded: $timingExpanded
                        )
                    }
                }
            } else {
                VStack {
                    Spacer()
                    Text("Select an effect to view settings")
                        .foregroundStyle(.secondary)
                    Spacer()
                }
            }
        }
    }

    // MARK: - Header

    private var header: some View {
        HStack {
            Text(viewModel.selectedEffect?.name ?? "")
                .font(.headline)
            Spacer()
            Button(action: { viewModel.deleteSelectedEffect() }) {
                Image(systemName: "trash")
                    .foregroundStyle(.red)
            }
        }
        .padding()
    }

    private var timingRow: some View {
        HStack {
            Text("Start:")
            Text(formatMS(viewModel.selectedEffect?.startTimeMS ?? 0))
                .monospacedDigit()
            Spacer()
            Text("End:")
            Text(formatMS(viewModel.selectedEffect?.endTimeMS ?? 0))
                .monospacedDigit()
        }
        .font(.caption)
        .padding(.horizontal)
        .padding(.vertical, 4)
    }

    // MARK: - Shared Section (collapsible)

    @ViewBuilder
    private func sharedSection(title: String,
                                metadata: EffectMetadata?,
                                expanded: Binding<Bool>) -> some View {
        VStack(alignment: .leading, spacing: 0) {
            Button(action: { expanded.wrappedValue.toggle() }) {
                HStack {
                    Image(systemName: expanded.wrappedValue ? "chevron.down" : "chevron.right")
                        .font(.caption2)
                    Text(title)
                        .font(.subheadline)
                        .fontWeight(.medium)
                    Spacer()
                }
                .padding(.horizontal, 8)
                .padding(.vertical, 6)
                .contentShape(Rectangle())
            }
            .buttonStyle(.plain)

            if expanded.wrappedValue, let md = metadata {
                EffectMetadataPanel(metadata: md)
                    .padding(.horizontal, 8)
                    .padding(.bottom, 6)
            }
        }
    }

    private func formatMS(_ ms: Int) -> String {
        let s = ms / 1000
        let m = s / 60
        let frac = (ms % 1000) / 10
        return String(format: "%d:%02d.%02d", m, s % 60, frac)
    }
}

// A panel that lays out all properties from an EffectMetadata. Handles the
// metadata's `groups` (tabs + sections) if present, otherwise shows a flat
// list of all properties.
struct EffectMetadataPanel: View {
    @Environment(SequencerViewModel.self) var viewModel
    let metadata: EffectMetadata

    // Pre-computed layout: walks the flat properties list in declared order,
    // skipping ones that belong to a group, and inserts each group at the
    // position of its first member property (matches JsonEffectPanel.cpp).
    private struct LayoutItem: Identifiable {
        enum Kind {
            case property(PropertyMetadata)
            case group(GroupMetadata)
        }
        let id: String
        let kind: Kind
    }

    private var layoutItems: [LayoutItem] {
        let properties = metadata.properties ?? []
        let groups = metadata.groups ?? []

        // Collect every prop id that belongs to any group (tabs/section).
        var groupedIds = Set<String>()
        // Map group → first appearance index in flat properties list.
        var anchoredGroups: [Int: GroupMetadata] = [:]

        for group in groups {
            var memberIds: [String] = []
            switch group.type {
            case "tabs":
                for tab in (group.tabs ?? []) {
                    memberIds.append(contentsOf: tab.properties)
                }
            case "section":
                memberIds.append(contentsOf: group.properties ?? [])
            case "xyCenter":
                // Not yet supported — just hide the XY member props.
                if let x = group.xProperty { memberIds.append(x) }
                if let y = group.yProperty { memberIds.append(y) }
                if let x = group.wrapX { memberIds.append(x) }
                if let y = group.wrapY { memberIds.append(y) }
            default:
                break
            }
            for id in memberIds { groupedIds.insert(id) }

            // Anchor at the first member's index in the flat list.
            if let firstIdx = properties.firstIndex(where: { memberIds.contains($0.id) }) {
                anchoredGroups[firstIdx] = group
            }
        }

        var items: [LayoutItem] = []
        for (idx, prop) in properties.enumerated() {
            // Insert any group anchored at this position BEFORE the property.
            if let group = anchoredGroups[idx] {
                items.append(LayoutItem(id: "group-\(idx)", kind: .group(group)))
            }
            // Skip properties that are inside a group.
            if groupedIds.contains(prop.id) { continue }
            items.append(LayoutItem(id: "prop-\(prop.id)", kind: .property(prop)))
        }
        return items
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            ForEach(layoutItems) { item in
                switch item.kind {
                case .property(let prop):
                    if isPropertyVisible(prop) {
                        EffectPropertyView(property: prop,
                                           metadataPrefix: metadata.settingKeyPrefix)
                    }
                case .group(let group):
                    groupView(group: group)
                }
            }
        }
    }

    @ViewBuilder
    private func groupView(group: GroupMetadata) -> some View {
        let allProps = metadata.properties ?? []
        let propsById: [String: PropertyMetadata] =
            Dictionary(uniqueKeysWithValues: allProps.map { ($0.id, $0) })

        switch group.type {
        case "tabs":
            if let tabs = group.tabs {
                ForEach(Array(tabs.enumerated()), id: \.offset) { _, tab in
                    DisclosureGroup(tab.label) {
                        VStack(alignment: .leading, spacing: 0) {
                            ForEach(tab.properties, id: \.self) { propId in
                                if let prop = propsById[propId], isPropertyVisible(prop) {
                                    EffectPropertyView(property: prop,
                                                       metadataPrefix: metadata.settingKeyPrefix)
                                }
                            }
                        }
                    }
                    .font(.caption)
                }
            }
        case "section":
            let label = group.label ?? ""
            VStack(alignment: .leading, spacing: 2) {
                if !label.isEmpty {
                    Text(label)
                        .font(.caption)
                        .fontWeight(.medium)
                        .foregroundStyle(.secondary)
                }
                ForEach(group.properties ?? [], id: \.self) { propId in
                    if let prop = propsById[propId], isPropertyVisible(prop) {
                        EffectPropertyView(property: prop,
                                           metadataPrefix: metadata.settingKeyPrefix)
                    }
                }
            }
            .padding(.vertical, 4)
        default:
            EmptyView()
        }
    }

    /// Simple visibility-rule evaluation: supports `equals` and `oneOf`.
    /// Everything else is treated as "visible". Full rule engine is TODO.
    private func isPropertyVisible(_ prop: PropertyMetadata) -> Bool {
        guard let rules = metadata.visibilityRules else { return true }
        for rule in rules {
            let hides = rule.hide?.contains(prop.id) ?? false
            let shows = rule.show?.contains(prop.id) ?? false
            if !hides && !shows { continue }

            let conditionMet = evaluateCondition(rule.when)
            if hides && conditionMet { return false }
            if shows && !conditionMet { return false }
        }
        return true
    }

    private func evaluateCondition(_ when: VisibilityRuleMetadata.WhenCondition) -> Bool {
        guard let propId = when.property else { return false }
        // Look up the target property's current value via its metadata.
        let allProps = metadata.properties ?? []
        guard let target = allProps.first(where: { $0.id == propId }) else { return false }

        let key = target.settingKey(prefix: metadata.settingKeyPrefix)
        let value = viewModel.settingValue(forKey: key, defaultValue: target.defaultAsString())

        if let equals = when.equals {
            return value == equals.stringValue
        }
        if let oneOf = when.oneOf {
            return oneOf.contains(where: { $0.stringValue == value })
        }
        return false
    }
}
