import SwiftUI

struct SequencerView: View {
    @Environment(SequencerViewModel.self) var viewModel
    @State private var pixelsPerMS: CGFloat = 0.1
    @State private var verticalOffset: CGFloat = 0

    private let rowHeight: CGFloat = 24
    private let rulerHeight: CGFloat = 30
    private let waveformHeight: CGFloat = 60
    private let rowHeaderWidth: CGFloat = 200

    var body: some View {
        VStack(spacing: 0) {
            toolbar
            Divider()

            if viewModel.showPreview {
                HStack(spacing: 0) {
                    ModelPreviewView()
                        .frame(maxWidth: .infinity)
                    Divider()
                    HousePreviewView()
                        .frame(maxWidth: .infinity)
                }
                .frame(height: 350)
                Divider()
            }

            // Main sequencer area
            GeometryReader { geo in
                HStack(spacing: 0) {
                    // Left column: row headers
                    VStack(spacing: 0) {
                        // Spacer for ruler
                        Color.clear.frame(height: rulerHeight)
                        // Spacer for waveform
                        if viewModel.hasAudio {
                            Color.clear.frame(height: waveformHeight)
                        }
                        // Row headers, offset to match vertical scroll. A plain
                        // VStack (not ScrollView) — ScrollView.disabled(true)
                        // blocks tap gestures on the header cells.
                        VStack(spacing: 0) {
                            ForEach(viewModel.rows) { row in
                                rowHeaderCell(row)
                            }
                            Spacer(minLength: 0)
                        }
                        .offset(y: verticalOffset)
                        .frame(maxHeight: .infinity, alignment: .top)
                        .clipped()
                    }
                    .frame(width: rowHeaderWidth)

                    Divider()

                    // Right column: timeline + waveform + effects grid
                    timelineScrollView(availableHeight: geo.size.height)

                    if viewModel.showInspector {
                        Divider()
                        EffectSettingsView()
                            .frame(width: 280)
                    }
                }
            }

            EffectPaletteView()
        }
    }

    // MARK: - Timeline Scroll View

    private func timelineScrollView(availableHeight: CGFloat) -> some View {
        let totalWidth = max(CGFloat(viewModel.sequenceDurationMS) * pixelsPerMS, 1)
        let headerHeight = rulerHeight + (viewModel.hasAudio ? waveformHeight : 0)
        let gridHeight = CGFloat(viewModel.rows.count) * rowHeight
        let totalHeight = headerHeight + gridHeight

        return ScrollView([.horizontal, .vertical]) {
            VStack(spacing: 0) {
                // Timeline ruler (sticks to top via horizontal scroll only)
                TimelineRulerView(
                    durationMS: viewModel.sequenceDurationMS,
                    pixelsPerMS: pixelsPerMS,
                    scrollOffset: 0
                )
                .frame(width: totalWidth, height: rulerHeight)

                // Waveform
                if viewModel.hasAudio {
                    WaveformView(
                        peaks: viewModel.waveformPeaks,
                        durationMS: viewModel.sequenceDurationMS,
                        pixelsPerMS: pixelsPerMS,
                        playPositionMS: viewModel.playPositionMS,
                        isPlaying: viewModel.isPlaying
                    )
                    .frame(width: totalWidth, height: waveformHeight)
                }

                // Effects grid
                VStack(spacing: 0) {
                    if viewModel.rows.isEmpty {
                        Text("No sequence data loaded")
                            .foregroundStyle(.secondary)
                            .frame(width: totalWidth, height: 100)
                    } else {
                        ForEach(viewModel.rows) { row in
                            effectRow(row: row, totalWidth: totalWidth)
                        }
                    }
                }
            }
            .frame(width: totalWidth, height: totalHeight)
            .background(
                GeometryReader { inner in
                    Color.clear.preference(
                        key: ScrollOffsetKey.self,
                        value: inner.frame(in: .named("timelineScroll")).origin.y
                    )
                }
            )
        }
        .coordinateSpace(name: "timelineScroll")
        .onPreferenceChange(ScrollOffsetKey.self) { value in
            verticalOffset = value
        }
    }

    // MARK: - Row Header Cell

    private func rowHeaderCell(_ row: SequencerViewModel.RowInfo) -> some View {
        let isPreviewed = viewModel.previewModelName != nil
            && row.displayName == viewModel.previewModelName
        return HStack {
            if row.layerIndex > 0 {
                Text("  Layer \(row.layerIndex)")
                    .font(.caption)
                    .foregroundStyle(.secondary)
            } else {
                Text(row.displayName)
                    .font(.caption)
                    .fontWeight(.medium)
                    .lineLimit(1)
            }
            Spacer()
        }
        .padding(.horizontal, 6)
        .frame(height: rowHeight)
        .background(
            isPreviewed
                ? Color.accentColor.opacity(0.35)
                : (row.id % 2 == 0 ? Color.black.opacity(0.05) : Color.gray.opacity(0.15))
        )
        .contentShape(Rectangle())
        .onTapGesture {
            viewModel.selectPreviewModel(rowIndex: row.id)
        }
    }

    // MARK: - Effect Row

    private func effectRow(row: SequencerViewModel.RowInfo, totalWidth: CGFloat) -> some View {
        ZStack(alignment: .leading) {
            // Row background
            Rectangle()
                .fill(row.id % 2 == 0 ? Color.black.opacity(0.3) : Color.black.opacity(0.2))
                .onTapGesture { location in
                    let ms = Int(location.x / pixelsPerMS)
                    let snapped = snapToFrame(ms)
                    if viewModel.selectedPaletteEffect != nil {
                        viewModel.addEffect(
                            rowIndex: row.id,
                            startMS: snapped,
                            endMS: snapped + viewModel.frameIntervalMS * 20
                        )
                    } else {
                        viewModel.clearSelection()
                    }
                }

            // Effects
            ForEach(row.effects) { effect in
                let x = CGFloat(effect.startTimeMS) * pixelsPerMS
                let w = CGFloat(effect.endTimeMS - effect.startTimeMS) * pixelsPerMS
                let isSelected = viewModel.selectedEffect?.rowIndex == row.id &&
                                 viewModel.selectedEffect?.effectIndex == effect.id

                EffectBlockView(
                    name: effect.name,
                    width: w,
                    height: rowHeight - 2,
                    color: colorForEffect(effect.name),
                    isSelected: isSelected
                )
                .offset(x: x, y: 1)
                .onTapGesture {
                    viewModel.selectEffect(rowIndex: row.id, effectIndex: effect.id)
                }
                .gesture(
                    DragGesture(minimumDistance: 5)
                        .onEnded { drag in
                            let deltaMS = Int(drag.translation.width / pixelsPerMS)
                            let newStart = snapToFrame(effect.startTimeMS + deltaMS)
                            let duration = effect.endTimeMS - effect.startTimeMS
                            viewModel.moveEffect(
                                rowIndex: row.id,
                                effectIndex: effect.id,
                                newStartMS: max(0, newStart),
                                newEndMS: max(0, newStart) + duration
                            )
                        }
                )
            }
        }
        .frame(width: totalWidth, height: rowHeight)
    }

    private func snapToFrame(_ ms: Int) -> Int {
        let frameMS = viewModel.frameIntervalMS
        return (ms / frameMS) * frameMS
    }

    // MARK: - Toolbar

    private var toolbar: some View {
        HStack(spacing: 12) {
            Button(action: { viewModel.closeSequence() }) {
                Image(systemName: "xmark")
            }

            Divider().frame(height: 24)

            // Playback controls — always shown, works with or without audio
            Button(action: { viewModel.stop() }) {
                Image(systemName: "stop.fill")
            }
            Button(action: { viewModel.togglePlayPause() }) {
                Image(systemName: viewModel.isPlaying ? "pause.fill" : "play.fill")
            }
            .keyboardShortcut(.space, modifiers: [])

            Text(formatTime(viewModel.playPositionMS))
                .monospacedDigit()
                .frame(width: 80)
            Text("/").foregroundStyle(.secondary)
            Text(formatTime(viewModel.sequenceDurationMS))
                .monospacedDigit()
                .foregroundStyle(.secondary)
                .frame(width: 80)

            if viewModel.isRendering {
                ProgressView()
                    .controlSize(.small)
            }

            Spacer()

            Text(viewModel.sequenceName ?? "")
                .font(.headline)

            Spacer()

            // Output toggle
            Button(action: { viewModel.toggleOutput() }) {
                Label(
                    viewModel.isOutputting ? "Stop Output" : "Start Output",
                    systemImage: viewModel.isOutputting ? "antenna.radiowaves.left.and.right.circle.fill" : "antenna.radiowaves.left.and.right"
                )
            }
            .tint(viewModel.isOutputting ? .green : nil)

            Divider().frame(height: 24)

            // Inspector toggle
            Button(action: { viewModel.showInspector.toggle() }) {
                Image(systemName: "sidebar.trailing")
            }

            Divider().frame(height: 24)

            // Preview toggle
            Button(action: { viewModel.togglePreview() }) {
                Label(
                    viewModel.showPreview ? "Hide Preview" : "Show Preview",
                    systemImage: viewModel.showPreview ? "eye.fill" : "eye"
                )
            }

            Divider().frame(height: 24)

            // Zoom
            HStack(spacing: 4) {
                Button(action: { pixelsPerMS = max(0.01, pixelsPerMS / 1.5) }) {
                    Image(systemName: "minus.magnifyingglass")
                }
                Button(action: { pixelsPerMS = min(1.0, pixelsPerMS * 1.5) }) {
                    Image(systemName: "plus.magnifyingglass")
                }
            }

            if viewModel.hasAudio {
                Image(systemName: "speaker.fill")
                    .foregroundStyle(.secondary)
                Slider(value: Binding(
                    get: { Double(viewModel.volume) },
                    set: { viewModel.setVolume(Int($0)) }
                ), in: 0...100)
                .frame(width: 80)
            }
        }
        .padding(.horizontal)
        .padding(.vertical, 8)
        .background(.bar)
    }

    // MARK: - Helpers

    private func formatTime(_ ms: Int) -> String {
        let totalSeconds = ms / 1000
        let minutes = totalSeconds / 60
        let seconds = totalSeconds % 60
        let frac = (ms % 1000) / 10
        return String(format: "%d:%02d.%02d", minutes, seconds, frac)
    }

    private func colorForEffect(_ name: String) -> Color {
        switch name {
        case "On": return .yellow.opacity(0.8)
        case "ColorWash": return .blue.opacity(0.7)
        case "Color Wash": return .blue.opacity(0.7)
        case "Bars": return .green.opacity(0.7)
        case "Butterfly": return .purple.opacity(0.7)
        case "Fire": return .orange.opacity(0.7)
        case "Morph": return .cyan.opacity(0.7)
        case "Twinkle": return .pink.opacity(0.7)
        case "Spirals": return .mint.opacity(0.7)
        case "Off": return .gray.opacity(0.3)
        default:
            let hash = abs(name.hashValue)
            let hue = Double(hash % 360) / 360.0
            return Color(hue: hue, saturation: 0.6, brightness: 0.7)
        }
    }
}

// Preference key for tracking vertical scroll offset
private struct ScrollOffsetKey: PreferenceKey {
    nonisolated(unsafe) static var defaultValue: CGFloat = 0
    static func reduce(value: inout CGFloat, nextValue: () -> CGFloat) {
        value = nextValue()
    }
}

struct TimelineRulerView: View {
    let durationMS: Int
    let pixelsPerMS: CGFloat
    let scrollOffset: CGFloat

    var body: some View {
        Canvas { context, size in
            let tickIntervalMS: Int
            if pixelsPerMS > 0.5 {
                tickIntervalMS = 100
            } else if pixelsPerMS > 0.1 {
                tickIntervalMS = 500
            } else if pixelsPerMS > 0.05 {
                tickIntervalMS = 1000
            } else {
                tickIntervalMS = 5000
            }

            var ms = 0
            while ms <= durationMS {
                let x = CGFloat(ms) * pixelsPerMS
                guard x < size.width + 10 else { break }

                let isMajor = ms % (tickIntervalMS * 5) == 0
                let tickHeight: CGFloat = isMajor ? 15 : 8
                let tickPath = Path { p in
                    p.move(to: CGPoint(x: x, y: size.height - tickHeight))
                    p.addLine(to: CGPoint(x: x, y: size.height))
                }
                context.stroke(tickPath, with: .color(.gray), lineWidth: 1)

                if isMajor {
                    let seconds = ms / 1000
                    let minutes = seconds / 60
                    let secs = seconds % 60
                    let label = String(format: "%d:%02d", minutes, secs)
                    context.draw(
                        Text(label).font(.system(size: 9)).foregroundColor(.gray),
                        at: CGPoint(x: x + 2, y: 8),
                        anchor: .leading
                    )
                }

                ms += tickIntervalMS
            }
        }
        .background(Color.black.opacity(0.2))
    }
}
