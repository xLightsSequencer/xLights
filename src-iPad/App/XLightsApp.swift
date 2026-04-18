import SwiftUI

@main
struct XLightsApp: App {
    @State private var viewModel: SequencerViewModel
    @Environment(\.scenePhase) private var scenePhase

    init() {
        // Must initialize xLights core (sets FileUtils::GetResourcesDir) BEFORE
        // creating the view model, since SequencerViewModel constructs an
        // iPadRenderContext whose EffectManager needs the resources directory
        // to load effectmetadata JSON files.
        XLiPadInit.initialize()
        let vm = SequencerViewModel()
        vm.startMemoryMonitoring()
        // Attempt to restore the previously-selected show folder + media
        // folders via their persistent security-scoped bookmarks.
        vm.restorePersistedShowFolder()
        _viewModel = State(initialValue: vm)
    }

    var body: some Scene {
        WindowGroup("xLights", id: "sequencer") {
            ContentView()
                .environment(viewModel)
        }
        .onChange(of: scenePhase) { _, newPhase in
            // Going to .background typically precedes termination; iOS
            // may kill the process without further notice. Abort the
            // render and block briefly so the worker threads exit
            // cleanly before the teardown race — otherwise they crash
            // mid-frame on freed SequenceElements/SequenceData.
            if newPhase == .background {
                viewModel.shutdownForBackground()
            }
        }
    }
}

struct ContentView: View {
    @Environment(SequencerViewModel.self) var viewModel
    @State private var showFolderConfig = false

    var body: some View {
        VStack(spacing: 0) {
            if viewModel.memoryWarning {
                MemoryWarningBanner()
            }
            Group {
                if !viewModel.isShowFolderLoaded {
                    ShowFolderSetupView(showFolderConfig: $showFolderConfig)
                } else if !viewModel.isSequenceLoaded {
                    SequencePickerView(showFolderConfig: $showFolderConfig)
                } else {
                    SequencerView()
                }
            }
        }
        .sheet(isPresented: $showFolderConfig) {
            FolderConfigView()
                .environment(viewModel)
        }
        .onAppear {
            // Auto-open the dialog on first launch when nothing is configured.
            if !viewModel.isShowFolderLoaded && FolderConfig.showFolder == nil {
                showFolderConfig = true
            }
        }
    }
}

struct MemoryWarningBanner: View {
    var body: some View {
        HStack(spacing: 8) {
            Image(systemName: "exclamationmark.triangle.fill")
            Text("Low memory — renders paused")
                .font(.caption)
            Spacer()
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 6)
        .foregroundStyle(.white)
        .background(Color.orange)
    }
}

struct ShowFolderSetupView: View {
    @Binding var showFolderConfig: Bool

    var body: some View {
        VStack(spacing: 20) {
            Text("xLights")
                .font(.largeTitle)
            Text("Select your show folder to get started")
                .font(.headline)
                .foregroundStyle(.secondary)
            Button("Configure Folders…") {
                showFolderConfig = true
            }
            .buttonStyle(.borderedProminent)
        }
    }
}

struct SequencePickerView: View {
    @Environment(SequencerViewModel.self) var viewModel
    @Binding var showFolderConfig: Bool

    var body: some View {
        NavigationStack {
            List(viewModel.sequenceFiles, id: \.self) { file in
                Button(file) {
                    let path = (viewModel.showFolderPath ?? "") + "/" + file
                    viewModel.openSequence(path: path)
                }
            }
            .navigationTitle("Sequences")
            .toolbar {
                Button {
                    showFolderConfig = true
                } label: {
                    Image(systemName: "folder.badge.gearshape")
                }
            }
        }
    }
}
