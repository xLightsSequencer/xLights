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
    @Environment(\.openWindow) private var openWindow

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
                    if viewModel.isMultiEffectSelection {
                        multiSelectChrome
                    }
                    timingRow
                    Divider()
                    tabBar
                    Divider()
                    ScrollView {
                        VStack(alignment: .leading, spacing: 0) {
                            // F-1c: when the selected tab is open in
                            // its own `inspector-tab` scene window,
                            // show a dock placeholder here; the
                            // content lives in the detached window.
                            if viewModel.detachedInspectorTabs.contains(selectedTab.wrappedValue.rawValue) {
                                InspectorTabDockedPlaceholder(tab: selectedTab.wrappedValue)
                            } else {
                                InspectorTabContent(tab: selectedTab.wrappedValue)
                            }
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

    /// "N effects selected" chrome shown when more than one effect is
    /// selected (G41). Values in the controls below reflect the anchor
    /// effect; long-pressing a control exposes "Apply to all selected"
    /// (G11), and the top-bar "Update All" button flushes every
    /// current inspector value across the set (G14).
    @ViewBuilder
    private var multiSelectChrome: some View {
        let count = viewModel.selectedEffects.count
        let uniqueNames = Set(viewModel.selectedEffects.map { $0.name }).count
        HStack(spacing: 8) {
            Image(systemName: "square.stack.3d.up.fill")
                .foregroundStyle(.tint)
            VStack(alignment: .leading, spacing: 1) {
                Text("\(count) effects selected")
                    .font(.caption)
                    .fontWeight(.semibold)
                Text(uniqueNames > 1
                     ? "Showing anchor values — mixed effect types"
                     : "Showing anchor values")
                    .font(.caption2)
                    .foregroundStyle(.secondary)
            }
            Spacer()
            Button {
                viewModel.updateAllLikeAnchor()
            } label: {
                Label("Update All", systemImage: "arrow.up.and.down.and.arrow.left.and.right")
                    .labelStyle(.iconOnly)
            }
            .buttonStyle(.bordered)
            .controlSize(.small)
            .help("Copy every inspector value from the anchor effect to all other selected effects")
            Button {
                // Collapse multi-select to the anchor.
                if let sel = viewModel.selectedEffect {
                    viewModel.selectEffect(rowIndex: sel.rowIndex,
                                            effectIndex: sel.effectIndex)
                }
            } label: {
                Image(systemName: "xmark.circle.fill")
                    .foregroundStyle(.secondary)
            }
            .buttonStyle(.plain)
            .help("Clear multi-selection")
        }
        .padding(.horizontal)
        .padding(.vertical, 6)
        .background(Color.accentColor.opacity(0.08))
    }

    /// Live start / end + play-position readout. Reads the effect's
    /// current times from `rows[…].effects[…]` so a user dragging to
    /// move or resize the selected effect sees the header update in
    /// real time instead of staying at the values captured when the
    /// effect was first selected. During the selection-scoped preview
    /// scrub loop, a third column tracks `playPositionMS` inside the
    /// `[start, end]` range so the user can see where the preview head
    /// is without having to look at the timeline.
    private var timingRow: some View {
        let (start, end) = currentTimes()
        return HStack(spacing: 10) {
            Text("Start:")
            Text(formatMS(start))
                .monospacedDigit()
            Spacer(minLength: 4)
            if viewModel.isScrubbing {
                Text("▶")
                    .foregroundStyle(Color.accentColor)
                Text(formatMS(viewModel.playPositionMS))
                    .monospacedDigit()
                    .foregroundStyle(Color.accentColor)
                Spacer(minLength: 4)
            }
            Text("End:")
            Text(formatMS(end))
                .monospacedDigit()
        }
        .font(.caption)
        .foregroundStyle(.secondary)
        .padding(.horizontal)
        .padding(.bottom, 6)
    }

    /// Resolve the selected effect's current times from the live rows
    /// instead of trusting the captured `selectedEffect` snapshot —
    /// the snapshot doesn't update during drag / resize / undo.
    private func currentTimes() -> (Int, Int) {
        guard let sel = viewModel.selectedEffect else { return (0, 0) }
        if let row = viewModel.rows.first(where: { $0.id == sel.rowIndex }),
           sel.effectIndex >= 0,
           sel.effectIndex < row.effects.count {
            let e = row.effects[sel.effectIndex]
            return (e.startTimeMS, e.endTimeMS)
        }
        return (sel.startTimeMS, sel.endTimeMS)
    }

    // MARK: - Tab bar

    private var tabBar: some View {
        HStack(spacing: 6) {
            Picker("Inspector Tab", selection: selectedTab) {
                ForEach(InspectorTab.allCases) { tab in
                    Image(systemName: tab.symbol)
                        .accessibilityLabel(tab.label)
                        .tag(tab)
                }
            }
            .pickerStyle(.segmented)

            // F-1c: open the currently-selected tab in its own
            // `inspector-tab` scene window. Disabled while that tab
            // is already detached; in that case the user docks it
            // via the placeholder's button (which dismissWindow's
            // the detached scene).
            Button {
                let tab = selectedTab.wrappedValue
                viewModel.pendingDetachTokens.insert("inspector-tab:\(tab.rawValue)")
                openWindow(id: "inspector-tab", value: tab)
            } label: {
                Image(systemName: "rectangle.on.rectangle.angled")
            }
            .disabled(viewModel.detachedInspectorTabs.contains(selectedTab.wrappedValue.rawValue))
            .help("Open this tab in its own window")
        }
        .padding(.horizontal, 8)
        .padding(.vertical, 6)
    }

    // MARK: - Tab content

    @ViewBuilder
    private var _legacyTabContent: some View {
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

// F-1c — reusable "content of a single inspector tab". Extracted so
// it can be rendered either inside the embedded sidebar or inside a
// standalone `inspector-tab` scene window.
struct InspectorTabContent: View {
    @Environment(SequencerViewModel.self) var viewModel
    let tab: InspectorTab

    var body: some View {
        switch tab {
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
}

// F-1c — placeholder shown in the embedded sidebar when the
// selected tab is open in its own window. Tapping "Dock Here"
// dismisses the detached scene, which fires its onDisappear and
// restores the content here.
struct InspectorTabDockedPlaceholder: View {
    let tab: InspectorTab
    @Environment(\.dismissWindow) private var dismissWindow

    var body: some View {
        VStack(spacing: 10) {
            Image(systemName: "rectangle.on.rectangle.angled")
                .font(.system(size: 32))
                .foregroundStyle(.secondary)
            Text("\(tab.label) is in its own window")
                .font(.footnote)
                .foregroundStyle(.secondary)
                .multilineTextAlignment(.center)
            Button("Dock Here") {
                dismissWindow(id: "inspector-tab", value: tab)
            }
            .buttonStyle(.borderedProminent)
            .controlSize(.small)
        }
        .frame(maxWidth: .infinity)
        .padding(.vertical, 24)
    }
}

// F-1c — root view for a detached inspector-tab scene window. Shows
// the effect header + timing row for context, then the single tab's
// content. Tracks its presence in `viewModel.detachedInspectorTabs`
// so the embedded sidebar swaps to the dock placeholder.
//
// No NavigationStack — the title bar it adds imposes a ~320pt
// minimum width that keeps the scene window from shrinking. Stage
// Manager already displays the scene title (`WindowGroup` id) in
// its chrome. The header row leaves ~80pt of leading padding so the
// iPadOS 26 window-controls pill doesn't overlay the effect name.
struct DetachedInspectorRoot: View {
    let tab: InspectorTab
    @Environment(SequencerViewModel.self) var viewModel
    @Environment(\.dismissWindow) private var dismissWindow
    @Environment(\.openWindow) private var openWindow
    @State private var suppressed: Bool = false

    var body: some View {
        Group {
            if suppressed {
                Color(.systemBackground)
            } else if viewModel.selectedEffect != nil {
                VStack(spacing: 0) {
                    header
                    timingRow
                    Divider()
                    ScrollView {
                        VStack(alignment: .leading, spacing: 0) {
                            InspectorTabContent(tab: tab)
                        }
                        .padding(.horizontal, 8)
                        .padding(.vertical, 6)
                        .frame(maxWidth: .infinity, alignment: .leading)
                    }
                }
            } else {
                VStack {
                    Spacer()
                    Text("Select an effect to view \(tab.label.lowercased()) settings")
                        .foregroundStyle(.secondary)
                    Spacer()
                }
            }
        }
        .frame(minWidth: 220, minHeight: 280)
        .navigationTitle("\(tab.label) — Inspector")
        .onAppear {
            // F-1 restoration guard (see HousePreviewView for detail).
            // Also open the sequencer first when this scene was the
            // auto-restored connecting session — otherwise we leave
            // zero scenes alive and the app goes to a black screen.
            let token = "inspector-tab:\(tab.rawValue)"
            if viewModel.pendingDetachTokens.remove(token) != nil {
                viewModel.detachedInspectorTabs.insert(tab.rawValue)
            } else {
                suppressed = true
                DispatchQueue.main.async {
                    openWindow(id: "sequencer")
                    dismissWindow(id: "inspector-tab", value: tab)
                }
            }
        }
        .onDisappear {
            if !suppressed {
                viewModel.detachedInspectorTabs.remove(tab.rawValue)
            }
        }
    }

    private var header: some View {
        HStack {
            Image(systemName: tab.symbol)
                .foregroundStyle(.secondary)
            Text(viewModel.selectedEffect?.name ?? "")
                .font(.headline)
                .lineLimit(1)
            Spacer()
        }
        .padding(.horizontal)
        // iPadOS 26 Stage Manager pill overlays the leading edge;
        // reserve ~80pt here so the effect name stays visible.
        .padding(.leading, 80)
        .padding(.top, 8)
        .padding(.bottom, 4)
    }

    private var timingRow: some View {
        let (start, end) = currentTimes()
        return HStack(spacing: 10) {
            Text("Start:")
            Text(formatMS(start))
                .monospacedDigit()
            Spacer(minLength: 4)
            Text("End:")
            Text(formatMS(end))
                .monospacedDigit()
        }
        .font(.caption)
        .foregroundStyle(.secondary)
        .padding(.horizontal)
        .padding(.bottom, 6)
    }

    private func currentTimes() -> (Int, Int) {
        guard let sel = viewModel.selectedEffect else { return (0, 0) }
        if let row = viewModel.rows.first(where: { $0.id == sel.rowIndex }),
           sel.effectIndex >= 0,
           sel.effectIndex < row.effects.count {
            let e = row.effects[sel.effectIndex]
            return (e.startTimeMS, e.endTimeMS)
        }
        return (sel.startTimeMS, sel.endTimeMS)
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
                                           metadataPrefix: metadata.settingKeyPrefix,
                                           ruleDisabled: !isPropertyEnabledByRules(prop))
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
            if let tabs = group.tabs, !tabs.isEmpty {
                NotebookTabsView(
                    tabs: tabs,
                    settingKey: group.settingKey,
                    metadataPrefix: metadata.settingKeyPrefix,
                    propsById: propsById,
                    isVisible: isPropertyVisible,
                    isEnabled: isPropertyEnabledByRules
                )
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
                                           metadataPrefix: metadata.settingKeyPrefix,
                                           ruleDisabled: !isPropertyEnabledByRules(prop))
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
                                prefix: metadata.settingKeyPrefix,
                                label: xyCenterLabel(for: group))
            }
        default:
            EmptyView()
        }
    }

    /// Resolve an xyCenter group's header label from the enclosing
    /// `tabs` group's tab label when its xProperty appears in one.
    /// Effects like Pictures declare separate "Start Position" /
    /// "End Position" tabs but reference the same xyCenter pad -- we
    /// want the pad to pick up the tab's label rather than all
    /// collapsing to a generic "Position".
    private func xyCenterLabel(for group: GroupMetadata) -> String {
        guard let xId = group.xProperty else { return "Position" }
        for g in (metadata.groups ?? []) where g.type == "tabs" {
            for tab in (g.tabs ?? []) {
                if tab.properties.contains(xId) {
                    return tab.label
                }
            }
        }
        return group.label?.isEmpty == false ? group.label! : "Position"
    }

    /// Visibility-rule evaluation mirroring the desktop engine at
    /// `JsonEffectPanel.cpp:1516-1570`. A `show` rule keeps the property
    /// visible only when its condition is met; a `hide` rule removes the
    /// property when its condition is met. Absent / unmatched rules leave
    /// the property visible.
    private func isPropertyVisible(_ prop: PropertyMetadata) -> Bool {
        // Platform gate — `"platform": "desktop"` entries are never
        // surfaced on iPad. Mirrors the desktop gate in
        // `JsonEffectPanel::BuildPropertyRow` for `"platform": "ipad"`.
        if !prop.isForIPad { return false }
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

    /// Visibility-rule "enable / disable" semantics — controls stay
    /// visible but lose interactivity (greyed out) when their gating
    /// condition isn't met. Mirrors desktop's
    /// `JsonEffectPanel::ApplyVisibilityRules` setEnabled path
    /// (`JsonEffectPanel.cpp:1758-1773`). Returns false when at least
    /// one rule's enable/disable clause forces this prop disabled.
    /// `show`/`hide` are handled separately via `isPropertyVisible`.
    private func isPropertyEnabledByRules(_ prop: PropertyMetadata) -> Bool {
        guard let rules = metadata.visibilityRules else { return true }
        for rule in rules {
            let disables = rule.disable?.contains(prop.id) ?? false
            let enables = rule.enable?.contains(prop.id) ?? false
            if !disables && !enables { continue }

            let conditionMet = evaluateCondition(rule.when)
            // disable when condition met → grey out.
            if disables && conditionMet { return false }
            // enable when condition met → grey out when condition is NOT met.
            if enables && !conditionMet { return false }
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
