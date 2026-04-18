import SwiftUI

struct SequencerView: View {
    @Environment(SequencerViewModel.self) var viewModel
    // Shared with SequencerGridV2View so toolbar zoom and in-grid
    // pinch-to-zoom drive the same state.
    @State private var timeline = TimelineState()

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

            HStack(spacing: 0) {
                SequencerGridV2View(timeline: timeline)
                    .frame(maxWidth: .infinity, maxHeight: .infinity)
                if viewModel.showInspector {
                    Divider()
                    EffectSettingsView()
                        .frame(width: 280)
                }
            }

            EffectPaletteView()
        }
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

            // Undo / Redo
            Button(action: { viewModel.undo() }) {
                Image(systemName: "arrow.uturn.backward")
            }
            .keyboardShortcut("z", modifiers: [.command])
            .disabled(!viewModel.undoManager.canUndo)

            Button(action: { viewModel.redo() }) {
                Image(systemName: "arrow.uturn.forward")
            }
            .keyboardShortcut("z", modifiers: [.command, .shift])
            .disabled(!viewModel.undoManager.canRedo)

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

            // Zoom — shares state with pinch-to-zoom on the grid.
            HStack(spacing: 4) {
                Button(action: {
                    timeline.pixelsPerMS = max(0.005, timeline.pixelsPerMS / 1.5)
                }) {
                    Image(systemName: "minus.magnifyingglass")
                }
                Button(action: {
                    timeline.pixelsPerMS = min(2.0, timeline.pixelsPerMS * 1.5)
                }) {
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
}

