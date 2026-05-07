import SwiftUI

/// Tools → AI Services… settings sheet. Walks the registered AI
/// services from `XLAIServices.shared()` and renders each one's
/// declarative property list (`aiBase::GetProperties()`) using
/// SwiftUI controls. Mirrors the desktop's `ServicesPanel`
/// (wxPropertyGrid + Test button).
///
/// Property changes persist immediately via the bridge — same
/// "apply changes immediately" semantics as the desktop preference
/// panel. The Test button lives at the bottom and runs
/// `aiBase::TestLLM()` on a background queue.
struct AIServicesSettingsSheet: View {
    @Environment(\.dismiss) private var dismiss

    @State private var services: [XLAIServiceInfo] = []

    // Local working copy for text-typed fields. SwiftUI's TextField
    // reads from its bound `get` on every render; if the bridge's
    // snapshot lags a keystroke (it doesn't, today, but a future
    // reload() between keystrokes would), the field would visually
    // revert. Keeping the in-progress text in @State sidesteps that
    // entirely. Initialised on first appear and on sheet re-show.
    @State private var workingValues: [String: String] = [:]
    @State private var didInit: Bool = false

    @State private var testStatus: TestStatus = .idle
    @State private var testServiceName: String = ""

    private enum TestStatus: Equatable {
        case idle
        case running
        case ok(String)
        case error(String)
    }

    var body: some View {
        NavigationStack {
            Form {
                if services.isEmpty {
                    Text("No AI services registered.")
                        .foregroundStyle(.secondary)
                } else {
                    ForEach(services, id: \.name) { service in
                        section(for: service)
                    }
                    testSection
                }
            }
            .navigationTitle("AI Services")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .topBarTrailing) {
                    Button("Done") { dismiss() }
                }
            }
        }
        .onAppear {
            reload()
            if !didInit {
                seedWorkingValues()
                didInit = true
            }
        }
    }

    // MARK: - Per-service section

    @ViewBuilder
    private func section(for service: XLAIServiceInfo) -> some View {
        Section {
            // Capability + status hint at the top of each section so
            // it's obvious why a service might not test successfully.
            statusBanner(for: service)

            // Category-kind properties have an empty `propertyId`
            // (they're just section dividers in desktop's
            // wxPropertyGrid). Filtering them out before ForEach
            // does two things: skips a row that would render as
            // EmptyView anyway, and avoids ID collisions in
            // SwiftUI's diff (multiple items with id="" caused
            // toggles to leak state between rows + between
            // services). The Section header above already provides
            // the visual category break.
            ForEach(service.properties.filter { $0.kind != .category },
                    id: \.propertyId) { prop in
                row(for: prop)
            }
        } header: {
            HStack {
                Text(service.name)
                Spacer()
                if service.available {
                    Label("Ready", systemImage: "checkmark.circle.fill")
                        .labelStyle(.titleAndIcon)
                        .foregroundStyle(.green)
                        .font(.caption)
                } else if service.enabled {
                    Label("Needs key", systemImage: "exclamationmark.triangle.fill")
                        .labelStyle(.titleAndIcon)
                        .foregroundStyle(.orange)
                        .font(.caption)
                } else {
                    Label("Disabled", systemImage: "circle")
                        .labelStyle(.titleAndIcon)
                        .foregroundStyle(.secondary)
                        .font(.caption)
                }
            }
        } footer: {
            if !service.capabilities.isEmpty {
                Text("Capabilities: \(service.capabilities.joined(separator: ", "))")
            }
        }
    }

    @ViewBuilder
    private func statusBanner(for service: XLAIServiceInfo) -> some View {
        // External-service disclosure. Local services (Ollama,
        // GenericClient) hit the user's own endpoint, so the
        // "data sent to a third party" language is misleading
        // for them — gate the banner on the service name.
        let isThirdParty = ["Claude", "ChatGPT", "Gemini"].contains(service.name)
        if isThirdParty {
            Label("Prompts you submit are sent to \(service.name)'s servers.",
                  systemImage: "info.circle")
                .font(.caption)
                .foregroundStyle(.secondary)
        }
    }

    // MARK: - Per-property row

    @ViewBuilder
    private func row(for prop: XLAIServiceProperty) -> some View {
        switch prop.kind {
        case .category:
            // Categories are already rendered as Section headers; the
            // category-kind row from the property list is a desktop
            // wxPropertyGrid affordance we don't need on iPad.
            EmptyView()

        case .bool:
            Toggle(prop.label, isOn: Binding(
                get: { prop.boolValue },
                set: { newValue in
                    XLAIServices.shared().setBoolProperty(prop.propertyId, value: newValue)
                    reload()
                }))

        case .string:
            stringRow(for: prop, secure: false)

        case .secret:
            stringRow(for: prop, secure: true)

        case .int:
            HStack {
                Text(prop.label)
                Spacer()
                TextField(prop.label, value: Binding(
                    get: { prop.intValue },
                    set: { newValue in
                        XLAIServices.shared().setIntProperty(prop.propertyId, value: newValue)
                    }), format: .number)
                    .keyboardType(.numberPad)
                    .multilineTextAlignment(.trailing)
                    .frame(minWidth: 80)
            }

        case .choice:
            Picker(prop.label, selection: Binding(
                get: { prop.stringValue },
                set: { newValue in
                    XLAIServices.shared().setStringProperty(prop.propertyId, value: newValue)
                    reload()
                })) {
                ForEach(prop.choices, id: \.self) { choice in
                    Text(choice).tag(choice)
                }
            }

        @unknown default:
            EmptyView()
        }
    }

    @ViewBuilder
    private func stringRow(for prop: XLAIServiceProperty, secure: Bool) -> some View {
        let key = prop.propertyId
        let binding = Binding<String>(
            get: { workingValues[key] ?? prop.stringValue },
            set: { newValue in
                workingValues[key] = newValue
                XLAIServices.shared().setStringProperty(key, value: newValue)
            })

        VStack(alignment: .leading, spacing: 4) {
            HStack {
                Text(prop.label)
                Spacer()
                Group {
                    if secure {
                        SecureField(prop.label, text: binding)
                    } else {
                        TextField(prop.label, text: binding)
                    }
                }
                .multilineTextAlignment(.trailing)
                .textInputAutocapitalization(.never)
                .autocorrectionDisabled()
                .frame(minWidth: 200)
            }
            if !prop.helpText.isEmpty {
                Text(prop.helpText)
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }
        }
    }

    // MARK: - Test row

    @ViewBuilder
    private var testSection: some View {
        Section("Test") {
            // Available services only — testing a disabled / unkeyed
            // service can only ever return an "API key empty" error
            // and isn't useful.
            let available = services.filter { $0.available }
            if available.isEmpty {
                Text("Configure and enable a service above, then come back here to test it.")
                    .font(.footnote)
                    .foregroundStyle(.secondary)
            } else {
                Picker("Service", selection: $testServiceName) {
                    ForEach(available, id: \.name) { svc in
                        Text(svc.name).tag(svc.name)
                    }
                }
                Button {
                    runTest()
                } label: {
                    HStack {
                        Text("Test \(testServiceName.isEmpty ? "Service" : testServiceName)")
                        if case .running = testStatus {
                            Spacer()
                            ProgressView().controlSize(.small)
                        }
                    }
                }
                .disabled(testServiceName.isEmpty || testStatus == .running)

                switch testStatus {
                case .idle, .running:
                    EmptyView()
                case .ok(let msg):
                    Label(displayMessage(msg, prefix: "OK: "),
                          systemImage: "checkmark.circle.fill")
                        .foregroundStyle(.green)
                        .font(.footnote)
                case .error(let msg):
                    Label(displayMessage(msg, prefix: "Error: "),
                          systemImage: "xmark.octagon.fill")
                        .foregroundStyle(.red)
                        .font(.footnote)
                }
            }
        }
        .onChange(of: services) { _, newServices in
            // Auto-select the first available service if the previous
            // selection got disabled (or nothing was selected yet).
            let avail = newServices.filter { $0.available }.map { $0.name }
            if testServiceName.isEmpty || !avail.contains(testServiceName) {
                testServiceName = avail.first ?? ""
            }
        }
    }

    private func displayMessage(_ msg: String, prefix: String) -> String {
        // Truncate ridiculously long error bodies so the row stays readable.
        if msg.count > 240 {
            return prefix + msg.prefix(237) + "..."
        }
        return prefix + msg
    }

    // MARK: - Actions

    private func reload() {
        services = XLAIServices.shared().allServices()
    }

    private func seedWorkingValues() {
        // Snapshot the current persisted strings into the local
        // working buffer so SecureField/TextField start with the
        // right value (and any later snapshot reload doesn't blow
        // away mid-edit text). Updated in-place by the binding's set.
        for service in services {
            for prop in service.properties {
                if prop.kind == .string || prop.kind == .secret {
                    workingValues[prop.propertyId] = prop.stringValue
                }
            }
        }
    }

    private func runTest() {
        guard !testServiceName.isEmpty else { return }
        testStatus = .running
        XLAIServices.shared().testService(testServiceName) { ok, message in
            // The bridge marshals this completion to main, but
            // Swift sees the ObjC block as non-isolated @Sendable.
            // Hop to MainActor so the @State mutation is safe.
            Task { @MainActor in
                if ok {
                    // Use the response itself if short; otherwise just
                    // confirm the call completed without surfacing the
                    // raw text (which can be long for "Hello" prompts).
                    let text = message.count <= 200 ? message : "service responded"
                    testStatus = .ok(text)
                } else {
                    testStatus = .error(message)
                }
            }
        }
    }
}

// The ObjC bridge types XLAIServices hands back are value snapshots:
// every property is `readonly copy`, the backing storage is set once
// in the initializer and never mutated. That makes them safe to send
// across actor boundaries — but Swift can't infer Sendable for
// imported ObjC classes, so we declare it explicitly here. Lets the
// XLAIServices completion handlers hop from the bridge's @Sendable
// callback into a `Task { @MainActor }` without strict-mode warnings.
extension XLAIServiceInfo: @unchecked @retroactive Sendable {}
extension XLAIServiceProperty: @unchecked @retroactive Sendable {}
extension XLAIPaletteColor: @unchecked @retroactive Sendable {}

