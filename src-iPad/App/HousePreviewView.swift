import SwiftUI

/// House Preview — shows every model plus view objects.
struct HousePreviewView: View {
    @State private var controlsVisible: Bool = false

    var body: some View {
        PreviewContainer(title: "House",
                         previewName: "HousePreview",
                         previewModelName: nil,
                         controlsVisible: $controlsVisible)
    }
}

/// Model Preview — shows the single selected model.
struct ModelPreviewView: View {
    @Environment(SequencerViewModel.self) var viewModel
    @State private var controlsVisible: Bool = false

    var body: some View {
        PreviewContainer(title: "Model",
                         previewName: "ModelPreview",
                         previewModelName: viewModel.previewModelName,
                         controlsVisible: $controlsVisible)
    }
}

/// Shared container that hosts a PreviewPaneView and overlays the controls
/// toggle and — when visible — camera shortcut buttons.
private struct PreviewContainer: View {
    let title: String
    let previewName: String
    let previewModelName: String?
    @Binding var controlsVisible: Bool

    var body: some View {
        ZStack(alignment: .topTrailing) {
            PreviewPaneView(previewName: previewName,
                            previewModelName: previewModelName,
                            controlsVisible: $controlsVisible)

            VStack(alignment: .trailing, spacing: 4) {
                Button {
                    controlsVisible.toggle()
                } label: {
                    Image(systemName: controlsVisible
                          ? "slider.horizontal.3"
                          : "slider.horizontal.below.rectangle")
                }
                .buttonStyle(.bordered)
                .controlSize(.small)

                if controlsVisible {
                    PreviewControlsOverlay(previewName: previewName)
                }
            }
            .padding(6)

            // Small title label in the upper-left for orientation.
            VStack {
                HStack {
                    Text(title)
                        .font(.caption2.weight(.semibold))
                        .foregroundStyle(.white.opacity(0.7))
                        .padding(.horizontal, 6)
                        .padding(.vertical, 2)
                        .background(.black.opacity(0.4), in: RoundedRectangle(cornerRadius: 4))
                    Spacer()
                }
                Spacer()
            }
            .padding(6)
            .allowsHitTesting(false)
        }
        .background(Color.black)
        .clipped()
    }
}

/// Camera shortcut buttons — drive the preview's camera through the bridge
/// via a small proxy that `PreviewPaneView` writes into its Coordinator on
/// makeUIView. For now we pipe the buttons through the SwiftUI environment
/// using a shared notification so both panes pick up their own button.
private struct PreviewControlsOverlay: View {
    let previewName: String

    var body: some View {
        HStack(spacing: 4) {
            Button { post(.zoomOut) } label: { Image(systemName: "minus.magnifyingglass") }
            Button { post(.zoomReset) } label: { Text("1×").font(.caption.monospacedDigit()) }
            Button { post(.zoomIn) } label: { Image(systemName: "plus.magnifyingglass") }
            Button { post(.reset) } label: { Image(systemName: "arrow.counterclockwise") }
        }
        .buttonStyle(.bordered)
        .controlSize(.small)
    }

    private enum Action {
        case zoomIn, zoomOut, zoomReset, reset
    }

    private func post(_ action: Action) {
        let name: Notification.Name
        switch action {
        case .zoomIn: name = .previewZoomIn
        case .zoomOut: name = .previewZoomOut
        case .zoomReset: name = .previewZoomReset
        case .reset: name = .previewResetCamera
        }
        NotificationCenter.default.post(name: name, object: previewName)
    }
}

extension Notification.Name {
    static let previewZoomIn = Notification.Name("PreviewZoomIn")
    static let previewZoomOut = Notification.Name("PreviewZoomOut")
    static let previewZoomReset = Notification.Name("PreviewZoomReset")
    static let previewResetCamera = Notification.Name("PreviewResetCamera")
}
