import SwiftUI

// Four-tab sidebar for the currently selected effect, matching desktop
// xLights' effect settings notebook. Tab identity persists across effect
// selection changes via @AppStorage so the user returns to the tab they
// were on, not always "Effect".
//
// Each tab is populated from JSON metadata in resources/effectmetadata/:
//   - Effect   — <EffectName>.json     (E_ prefix)
//   - Colors   — shared/Color.json     (C_ prefix, palette map)
//   - Blending — shared/Blending.json  (T_ prefix — layer-blend method,
//                                       morph, canvas, transitions)
//   - Buffer   — shared/Buffer.json    (B_ prefix — render style, transforms,
//                                       roto-zoom, sub-buffer)
//
// The tabs map to the same four dockable panes on desktop. Phase E will
// expose per-tab "Open in new window" actions; for now they're locked
// into the sidebar.
enum InspectorTab: String, CaseIterable, Identifiable, Hashable, Codable {
    case effect
    case colors
    case blending
    case buffer

    var id: String { rawValue }

    var label: String {
        switch self {
        case .effect:   return "Effect"
        case .colors:   return "Colors"
        case .blending: return "Blending"
        case .buffer:   return "Buffer"
        }
    }

    var symbol: String {
        switch self {
        case .effect:   return "slider.horizontal.3"
        case .colors:   return "paintpalette"
        case .blending: return "square.stack.3d.up"
        case .buffer:   return "square.grid.3x3"
        }
    }
}

struct EffectSettingsView: View {
    @Environment(SequencerViewModel.self) var viewModel

    // Persist the selected tab across sequence loads and app launches.
    @AppStorage("inspectorTab") private var storedTab: String = InspectorTab.effect.rawValue

    private var selectedTab: Binding<InspectorTab> {
        Binding<InspectorTab>(
            get: { InspectorTab(rawValue: storedTab) ?? .effect },
            set: { storedTab = $0.rawValue }
        )
    }

    var body: some View {
        Group {
            if viewModel.selectedEffect != nil {
                VStack(spacing: 0) {
                    header
                    timingRow
                    Divider()
                    tabBar
                    Divider()
                    ScrollView {
                        VStack(alignment: .leading, spacing: 0) {
                            tabContent
                        }
                        .padding(.horizontal, 8)
                        .padding(.vertical, 6)
                        .frame(maxWidth: .infinity, alignment: .leading)
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
                .lineLimit(1)
            Spacer()
            Button(action: { viewModel.deleteSelectedEffect() }) {
                Image(systemName: "trash")
                    .foregroundStyle(.red)
            }
        }
        .padding(.horizontal)
        .padding(.top, 8)
        .padding(.bottom, 4)
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
        .foregroundStyle(.secondary)
        .padding(.horizontal)
        .padding(.bottom, 6)
    }

    // MARK: - Tab bar

    private var tabBar: some View {
        Picker("Inspector Tab", selection: selectedTab) {
            ForEach(InspectorTab.allCases) { tab in
                Image(systemName: tab.symbol)
                    .accessibilityLabel(tab.label)
                    .tag(tab)
            }
        }
        .pickerStyle(.segmented)
        .padding(.horizontal, 8)
        .padding(.vertical, 6)
    }

    // MARK: - Tab content

    @ViewBuilder
    private var tabContent: some View {
        switch selectedTab.wrappedValue {
        case .effect:
            if let md = viewModel.selectedEffectMetadata {
                EffectMetadataPanel(metadata: md)
            } else {
                unavailable("No metadata available for this effect")
            }
        case .colors:
            if let md = viewModel.colorMetadata {
                EffectMetadataPanel(metadata: md)
            } else {
                unavailable("Color metadata not loaded")
            }
        case .blending:
            if let md = viewModel.blendingMetadata {
                EffectMetadataPanel(metadata: md)
            } else {
                unavailable("Blending metadata not loaded")
            }
        case .buffer:
            if let md = viewModel.bufferMetadata {
                EffectMetadataPanel(metadata: md)
            } else {
                unavailable("Buffer metadata not loaded")
            }
        }
    }

    private func unavailable(_ msg: String) -> some View {
        Text(msg)
            .font(.caption)
            .foregroundStyle(.secondary)
            .padding()
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
                        if prop.separator == true {
                            Divider().padding(.vertical, 2)
                        }
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
                                    if prop.separator == true {
                                        Divider().padding(.vertical, 2)
                                    }
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
                        if prop.separator == true {
                            Divider().padding(.vertical, 2)
                        }
                        EffectPropertyView(property: prop,
                                           metadataPrefix: metadata.settingKeyPrefix)
                    }
                }
            }
            .padding(.vertical, 4)
        case "xyCenter":
            if let xId = group.xProperty,
               let yId = group.yProperty,
               let xProp = propsById[xId],
               let yProp = propsById[yId] {
                XYCenterPadView(xProp: xProp,
                                yProp: yProp,
                                wrapX: group.wrapX.flatMap { propsById[$0] },
                                wrapY: group.wrapY.flatMap { propsById[$0] },
                                prefix: metadata.settingKeyPrefix)
            }
        default:
            EmptyView()
        }
    }

    /// Visibility-rule evaluation mirroring the desktop engine at
    /// `JsonEffectPanel.cpp:1516-1570`. A `show` rule keeps the property
    /// visible only when its condition is met; a `hide` rule removes the
    /// property when its condition is met. Absent / unmatched rules leave
    /// the property visible.
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

    /// Returns the current string value of the property whose id is `propId`
    /// within this metadata, falling back to the metadata default.
    private func propertyValue(forId propId: String) -> String? {
        let allProps = metadata.properties ?? []
        guard let target = allProps.first(where: { $0.id == propId }) else { return nil }
        let key = target.settingKey(prefix: metadata.settingKeyPrefix)
        return viewModel.settingValue(forKey: key, defaultValue: target.defaultAsString())
    }

    private func evaluateCondition(_ when: VisibilityRuleMetadata.WhenCondition) -> Bool {
        // `any`: OR across a list of checkbox-style properties. Matches
        // JsonEffectPanel.cpp:1521-1530 — conditionMet iff any listed
        // property is truthy.
        if let anyIds = when.any, !anyIds.isEmpty {
            for id in anyIds {
                if let v = propertyValue(forId: id), isTruthy(v) { return true }
            }
            return false
        }

        guard let propId = when.property,
              let value = propertyValue(forId: propId) else {
            return false
        }

        if let equals = when.equals {
            return valuesMatch(value, equals)
        }
        if let notEquals = when.notEquals {
            return !valuesMatch(value, notEquals)
        }
        if let oneOf = when.oneOf {
            return oneOf.contains(where: { valuesMatch(value, $0) })
        }
        if let notOneOf = when.notOneOf {
            return !notOneOf.contains(where: { valuesMatch(value, $0) })
        }
        if let greaterThan = when.greaterThan {
            let threshold = numericValue(greaterThan) ?? 0
            let lhs = Double(value) ?? 0
            return lhs > threshold
        }
        if let prefix = when.startsWith, !prefix.isEmpty {
            return value.hasPrefix(prefix)
        }
        return false
    }

    /// Compares a settings-map string value against a Codable literal,
    /// normalising booleans to their stored "1"/"0" form.
    private func valuesMatch(_ stored: String, _ literal: AnyCodable) -> Bool {
        if let b = literal.value as? Bool {
            return isTruthy(stored) == b
        }
        return stored == literal.stringValue
    }

    private func isTruthy(_ s: String) -> Bool {
        return s == "1" || s.lowercased() == "true"
    }

    private func numericValue(_ v: AnyCodable) -> Double? {
        switch v.value {
        case let d as Double: return d
        case let i as Int:    return Double(i)
        case let b as Bool:   return b ? 1 : 0
        case let s as String: return Double(s)
        default:              return nil
        }
    }
}
