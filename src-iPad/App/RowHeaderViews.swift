import SwiftUI

/// Timing-row header (row 2 left). Renders a colored background, an
/// active indicator circle (filled when active, outlined when not),
/// and the timing element's display name. Tapping the row toggles
/// `TimingElement::GetActive/SetActive` via the bridge.
struct TimingRowHeader: View {
    let row: SequencerViewModel.RowInfo
    let height: CGFloat
    let document: XLSequenceDocument

    @State private var isActive: Bool = true

    // Desktop timing color palette (Timing1..Timing5): cyan, red, green, blue, yellow.
    // Darkened so white text remains readable on top.
    private static let timingPalette: [Color] = [
        Color(red: 0.00, green: 0.45, blue: 0.55), // cyan
        Color(red: 0.60, green: 0.10, blue: 0.10), // red
        Color(red: 0.10, green: 0.45, blue: 0.15), // green
        Color(red: 0.15, green: 0.25, blue: 0.60), // blue
        Color(red: 0.55, green: 0.45, blue: 0.05)  // yellow
    ]

    private var backgroundColor: Color {
        let idx = row.timing?.colorIndex ?? 0
        return Self.timingPalette[((idx % Self.timingPalette.count) + Self.timingPalette.count) % Self.timingPalette.count]
    }

    var body: some View {
        HStack(spacing: 6) {
            if row.layerIndex == 0 {
                Circle()
                    .fill(isActive ? Color.white : Color.clear)
                    .overlay(
                        Circle().strokeBorder(Color.white.opacity(0.8), lineWidth: 1)
                    )
                    .frame(width: 10, height: 10)
            } else {
                Spacer().frame(width: 10, height: 10)
            }
            Text(row.displayName)
                .font(.caption)
                .foregroundStyle(.white)
                .lineLimit(1)
            Spacer(minLength: 0)
        }
        .padding(.horizontal, 6)
        .frame(maxWidth: .infinity, alignment: .leading)
        .frame(height: height)
        .background(backgroundColor)
        .contentShape(Rectangle())
        .onTapGesture {
            guard row.layerIndex == 0 else { return }
            let newActive = !isActive
            document.setTimingRowActive(newActive, at: Int32(row.id))
            isActive = newActive
        }
        .onAppear {
            isActive = document.timingRowIsActive(at: Int32(row.id))
        }
    }
}

/// Model-row header (row 3 left). Shows the element display name (or
/// indented "Layer N" label for nested layer rows), a collapse toggle
/// when the element has more than one layer, and a group icon when the
/// element is a ModelGroup. Tapping selects the model for the preview
/// pane.
struct ModelRowHeader: View {
    let row: SequencerViewModel.RowInfo
    let height: CGFloat
    let document: XLSequenceDocument
    let onSelect: () -> Void

    var body: some View {
        let isSubLayer = row.layerIndex > 0
        let isGroup   = !isSubLayer && document.rowIsModelGroup(at: Int32(row.id))
        let layerCount = isSubLayer ? 0 : Int(document.rowLayerCount(at: Int32(row.id)))
        let collapsed = !isSubLayer && document.rowIsElementCollapsed(at: Int32(row.id))
        let showCollapse = !isSubLayer && layerCount > 1

        HStack(spacing: 4) {
            if showCollapse {
                Button {
                    document.toggleElementCollapsed(at: Int32(row.id))
                } label: {
                    Image(systemName: collapsed ? "plus.square" : "minus.square")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
                .buttonStyle(.plain)
            } else {
                Spacer().frame(width: 14)
            }

            if isGroup {
                Image(systemName: "folder")
                    .font(.caption2)
                    .foregroundStyle(.secondary)
            }

            if isSubLayer {
                Text("[\(row.layerIndex + 1)]")
                    .font(.caption)
                    .foregroundStyle(.secondary)
                    .lineLimit(1)
            } else if layerCount > 1 {
                Text("\(row.displayName) [\(layerCount)]")
                    .font(.caption)
                    .fontWeight(.medium)
                    .lineLimit(1)
            } else {
                Text(row.displayName)
                    .font(.caption)
                    .fontWeight(.medium)
                    .lineLimit(1)
            }
            Spacer(minLength: 0)
        }
        .padding(.horizontal, 6)
        .frame(maxWidth: .infinity, alignment: .leading)
        .frame(height: height)
        .background(row.id % 2 == 0
                    ? Color.black.opacity(0.25)
                    : Color.black.opacity(0.15))
        .contentShape(Rectangle())
        .onTapGesture { onSelect() }
    }
}
