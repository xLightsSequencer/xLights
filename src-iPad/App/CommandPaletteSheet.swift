import SwiftUI

// Command palette — desktop #6258 parity. A fuzzy launcher over every
// menu command and every effect type, bound to ⌘⇧K. Two sections:
//   Commands — the iPad's own menu actions (registry below mirrors
//              `XLightsCommands.swift`), filtered to the ones whose
//              enabled-gate is satisfied (desktop only lists enabled
//              menu items via `IsEnabled()` in `CommandPaletteDialog`).
//   Effects  — `viewModel.availableEffects`; selecting one drops it at
//              the play position on the active row (mirrors desktop's
//              effect branch in `xLightsFrame::OnCommandPalette`).
//
// Selecting a command runs its closure; selecting an effect drops it.
// Arrow keys move the highlight, return activates, escape dismisses.

struct PaletteCommand: Identifiable {
    let id = UUID()
    let name: String
    let category: String
    let shortcut: String
    let symbol: String
    let action: () -> Void
}

private enum PaletteRow: Identifiable {
    case command(PaletteCommand)
    case effect(String)

    var id: String {
        switch self {
        case .command(let c): return "cmd:\(c.id)"
        case .effect(let n): return "eff:\(n)"
        }
    }
}

struct CommandPaletteSheet: View {
    @Environment(SequencerViewModel.self) private var viewModel
    @Environment(\.dismiss) private var dismiss
    @Environment(\.displayScale) private var displayScale

    /// View-owned timeline (zoom commands need it). Optional so the
    /// sheet works even before a timeline is established.
    let timeline: TimelineState?

    @State private var query = ""
    @State private var highlight = 0
    @FocusState private var searchFocused: Bool

    var body: some View {
        VStack(spacing: 0) {
            TextField("Search commands and effects…", text: $query)
                .textFieldStyle(.plain)
                .font(.title3)
                .autocorrectionDisabled()
                .textInputAutocapitalization(.never)
                .focused($searchFocused)
                .padding(.horizontal, 16)
                .padding(.vertical, 12)
                .onChange(of: query) { _, _ in highlight = 0 }
                .onSubmit { activate() }

            Divider()

            ScrollViewReader { proxy in
                List {
                    let cmds = filteredCommands
                    let effs = filteredEffects
                    if !cmds.isEmpty {
                        Section("Commands") {
                            ForEach(Array(cmds.enumerated()), id: \.element.id) { idx, cmd in
                                commandRow(cmd, index: idx)
                            }
                        }
                    }
                    if !effs.isEmpty {
                        Section("Effects") {
                            ForEach(Array(effs.enumerated()), id: \.element) { idx, name in
                                effectRow(name, index: cmds.count + idx)
                            }
                        }
                    }
                    if cmds.isEmpty && effs.isEmpty {
                        Text("No matching commands or effects")
                            .foregroundStyle(.secondary)
                    }
                }
                .listStyle(.plain)
                .onChange(of: highlight) { _, h in
                    if let id = allRows[safe: h]?.id {
                        withAnimation(.linear(duration: 0.05)) {
                            proxy.scrollTo(id, anchor: .center)
                        }
                    }
                }
            }
        }
        .frame(minWidth: 520, minHeight: 420)
        .onAppear { searchFocused = true }
        .onKeyPress(.downArrow) { moveHighlight(1); return .handled }
        .onKeyPress(.upArrow) { moveHighlight(-1); return .handled }
        .onKeyPress(.return) { activate(); return .handled }
        .onKeyPress(.escape) { dismiss(); return .handled }
    }

    // MARK: - Rows

    private func commandRow(_ cmd: PaletteCommand, index: Int) -> some View {
        Button {
            highlight = index
            activate()
        } label: {
            HStack(spacing: 10) {
                Image(systemName: cmd.symbol)
                    .frame(width: 22)
                    .foregroundStyle(.secondary)
                highlightedText(cmd.name)
                Spacer()
                if !cmd.shortcut.isEmpty {
                    Text(cmd.shortcut)
                        .font(.caption.monospaced())
                        .foregroundStyle(.secondary)
                        .padding(.horizontal, 6)
                        .padding(.vertical, 2)
                        .background(Color.secondary.opacity(0.15),
                                    in: RoundedRectangle(cornerRadius: 4))
                }
                Text(cmd.category)
                    .font(.caption2)
                    .foregroundStyle(.tertiary)
            }
            .contentShape(Rectangle())
        }
        .buttonStyle(.plain)
        .listRowBackground(highlight == index
                           ? Color.accentColor.opacity(0.25) : Color.clear)
        .id(PaletteRow.command(cmd).id)
    }

    private func effectRow(_ name: String, index: Int) -> some View {
        Button {
            highlight = index
            activate()
        } label: {
            HStack(spacing: 10) {
                if let icon = iconImage(for: name) {
                    Image(icon, scale: displayScale, label: Text(name))
                        .resizable()
                        .interpolation(.high)
                        .frame(width: 20, height: 20)
                } else {
                    Image(systemName: "sparkles")
                        .frame(width: 22)
                        .foregroundStyle(.secondary)
                }
                highlightedText(name)
                Spacer()
                Text("Add Effect")
                    .font(.caption2)
                    .foregroundStyle(.tertiary)
            }
            .contentShape(Rectangle())
        }
        .buttonStyle(.plain)
        .listRowBackground(highlight == index
                           ? Color.accentColor.opacity(0.25) : Color.clear)
        .id(PaletteRow.effect(name).id)
    }

    @ViewBuilder
    private func highlightedText(_ text: String) -> some View {
        let positions = Set(CommandPaletteMatch.positions(in: text, query: query))
        if positions.isEmpty {
            Text(text)
        } else {
            Text(AttributedString(text).withMatches(positions))
        }
    }

    private func iconImage(for name: String) -> CGImage? {
        let bucket = EffectIconCache.bucket(forDesiredPx: 20 * displayScale)
        return EffectIconCache.shared.image(for: name,
                                             bucket: bucket,
                                             document: viewModel.document)
    }

    // MARK: - Filtering / ordering

    private var commandRegistry: [PaletteCommand] {
        CommandPaletteRegistry.commands(viewModel: viewModel, timeline: timeline)
    }

    private var filteredCommands: [PaletteCommand] {
        let q = query
        if q.isEmpty { return commandRegistry }
        return commandRegistry
            .compactMap { cmd -> (PaletteCommand, Int)? in
                let n = CommandPaletteMatch.score(cmd.name, query: q)
                let c = CommandPaletteMatch.score(cmd.category, query: q)
                guard let best = [n, c].compactMap({ $0 }).max() else { return nil }
                return (cmd, best)
            }
            .sorted { $0.1 > $1.1 }
            .map { $0.0 }
    }

    private var filteredEffects: [String] {
        let q = query
        if q.isEmpty { return viewModel.availableEffects }
        return viewModel.availableEffects
            .compactMap { name -> (String, Int)? in
                guard let s = CommandPaletteMatch.score(name, query: q) else { return nil }
                return (name, s)
            }
            .sorted { $0.1 > $1.1 }
            .map { $0.0 }
    }

    private var allRows: [PaletteRow] {
        filteredCommands.map { .command($0) } + filteredEffects.map { .effect($0) }
    }

    // MARK: - Activation / navigation

    private func moveHighlight(_ delta: Int) {
        let count = allRows.count
        guard count > 0 else { return }
        highlight = min(max(highlight + delta, 0), count - 1)
    }

    private func activate() {
        guard let row = allRows[safe: highlight] else { return }
        switch row {
        case .command(let cmd):
            dismiss()
            DispatchQueue.main.async { cmd.action() }
        case .effect(let name):
            dismiss()
            DispatchQueue.main.async {
                viewModel.dropEffectFromCommandPalette(name: name)
            }
        }
    }
}

private extension Array {
    subscript(safe index: Int) -> Element? {
        indices.contains(index) ? self[index] : nil
    }
}

private extension AttributedString {
    func withMatches(_ positions: Set<Int>) -> AttributedString {
        var attr = self
        var idx = attr.startIndex
        var offset = 0
        while idx < attr.endIndex {
            let next = attr.index(afterCharacter: idx)
            if positions.contains(offset) {
                attr[idx..<next].foregroundColor = .accentColor
                attr[idx..<next].font = .body.bold()
            }
            idx = next
            offset += 1
        }
        return attr
    }
}
