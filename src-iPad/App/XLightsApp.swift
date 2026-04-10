import SwiftUI

@main
struct XLightsApp: App {
    @State private var viewModel: SequencerViewModel

    init() {
        // Must initialize xLights core (sets FileUtils::GetResourcesDir) BEFORE
        // creating the view model, since SequencerViewModel constructs an
        // iPadRenderContext whose EffectManager needs the resources directory
        // to load effectmetadata JSON files.
        XLiPadInit.initialize()
        _viewModel = State(initialValue: SequencerViewModel())
    }

    var body: some Scene {
        WindowGroup("xLights", id: "sequencer") {
            ContentView()
                .environment(viewModel)
        }
    }
}

struct ContentView: View {
    @Environment(SequencerViewModel.self) var viewModel

    var body: some View {
        if !viewModel.isShowFolderLoaded {
            ShowFolderSetupView()
        } else if !viewModel.isSequenceLoaded {
            SequencePickerView()
        } else {
            SequencerView()
        }
    }
}

struct ShowFolderSetupView: View {
    @Environment(SequencerViewModel.self) var viewModel
    @State private var showPicker = false

    var body: some View {
        VStack(spacing: 20) {
            Text("xLights")
                .font(.largeTitle)
            Text("Select your show folder to get started")
                .font(.headline)
                .foregroundStyle(.secondary)
            Button("Choose Show Folder") {
                showPicker = true
            }
            .buttonStyle(.borderedProminent)
        }
        .sheet(isPresented: $showPicker) {
            ShowFolderPicker { url in
                viewModel.loadShowFolder(url: url)
                showPicker = false
            }
        }
    }
}

struct SequencePickerView: View {
    @Environment(SequencerViewModel.self) var viewModel

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
                Button("Change Folder") {
                    viewModel.showFolderPickerPresented = true
                }
            }
            .sheet(isPresented: Bindable(viewModel).showFolderPickerPresented) {
                ShowFolderPicker { url in
                    viewModel.loadShowFolder(url: url)
                    viewModel.showFolderPickerPresented = false
                }
            }
        }
    }
}
