import SwiftUI

// Help ▸ Key Bindings — read-only reference of the hard-keyboard
// shortcuts declared in XLightsCommands.swift (desktop parity for
// Help ▸ "Key Bindings"; the system ⌘-hold HUD shows the same data
// but only while a hardware keyboard is attached and held).
//
// Maintained by hand: when adding a `.keyboardShortcut` in
// XLightsCommands.swift, add a row here.
struct KeyBindingsSheet: View {
    @Environment(\.dismiss) private var dismiss

    private struct Binding1: Identifiable {
        let id = UUID()
        let keys: String
        let action: String
    }

    private struct Group1: Identifiable {
        let id = UUID()
        let title: String
        let bindings: [Binding1]
    }

    private static let groups: [Group1] = [
        Group1(title: "File", bindings: [
            .init(keys: "⌘S", action: "Save"),
            .init(keys: "⌘⇧S", action: "Save As…"),
            .init(keys: "⌘W", action: "Close Sequence"),
        ]),
        Group1(title: "Edit", bindings: [
            .init(keys: "⌘Z / ⌘⇧Z", action: "Undo / Redo"),
            .init(keys: "⌘X", action: "Cut"),
            .init(keys: "⌘C", action: "Copy"),
            .init(keys: "⌘V", action: "Paste"),
            .init(keys: "⌘D", action: "Duplicate"),
            .init(keys: "⌘A", action: "Select All"),
            .init(keys: "Delete", action: "Delete Selection"),
            .init(keys: "Esc", action: "Clear Selection"),
            .init(keys: "⌘F", action: "Find / Replace"),
            .init(keys: "⌘⇧P", action: "Effect Presets"),
        ]),
        Group1(title: "Effect Editing", bindings: [
            .init(keys: "⌃← / ⌃→", action: "Nudge ±1 ms"),
            .init(keys: "⌥← / ⌥→", action: "Nudge ±1 frame"),
            .init(keys: "⇧← / ⇧→", action: "Stretch End Back / Forward"),
            .init(keys: "⇧⌘← / ⇧⌘→", action: "Extend to Previous / Next Effect"),
            .init(keys: "⌃PgUp / ⌃PgDn", action: "Nudge to Previous / Next Timing Mark"),
            .init(keys: "⌥↑ / ⌥↓", action: "Move Effect Up / Down a Layer"),
        ]),
        Group1(title: "Navigation", bindings: [
            .init(keys: "← / →", action: "Previous / Next Effect"),
            .init(keys: "↑ / ↓", action: "Effect Above / Below"),
            .init(keys: "Tab / ⇧Tab", action: "Next / Previous Effect"),
            .init(keys: "⌘= / ⌘−", action: "Zoom In / Out"),
            .init(keys: "Home / End", action: "Rewind to Start / Jump to End"),
        ]),
        Group1(title: "Playback", bindings: [
            .init(keys: "Space", action: "Play / Pause"),
            .init(keys: "⇧Space", action: "Replay Section"),
            .init(keys: "⌘R", action: "Render All"),
            .init(keys: ", / .", action: "Previous / Next Frame"),
            .init(keys: "⌃0–9", action: "Go To Tag"),
            .init(keys: "⌃⇧0–9", action: "Set Tag at Play Head"),
        ]),
        Group1(title: "View", bindings: [
            .init(keys: "⌘1", action: "Show / Hide Preview"),
            .init(keys: "⌘2", action: "Show / Hide Inspector"),
            .init(keys: "⌘⇧D", action: "Edit Display Elements"),
            .init(keys: "⌥⌘E / ⌥⌘C / ⌥⌘B / ⌥⌘U", action: "Detach Effect / Colors / Blending / Buffer Tab"),
        ]),
    ]

    var body: some View {
        NavigationStack {
            List(Self.groups) { group in
                Section(group.title) {
                    ForEach(group.bindings) { b in
                        HStack {
                            Text(b.action)
                            Spacer()
                            Text(b.keys)
                                .font(.system(.body, design: .monospaced))
                                .foregroundStyle(.secondary)
                        }
                    }
                }
            }
            .navigationTitle("Key Bindings")
            .toolbar {
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") { dismiss() }
                }
            }
        }
    }
}
