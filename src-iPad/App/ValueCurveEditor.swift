import SwiftUI

// Value-curve UI (C-5). Renders next to every property whose metadata
// declares `valueCurve: true` and lets the user match desktop parity: the
// full type set (23 curves), P1-P4 with per-type ranges, Wrap / Real Values /
// Time Offset, timing / audio / filter-label fields, and a touch canvas for
// Custom points. All serialisation routes through XLValueCurve (wrapping
// src-core/render/ValueCurve) so the string stored at
// `<prefix>VALUECURVE_<id>` is byte-for-byte identical to what desktop
// writes.

// MARK: - Storage-key helper

extension PropertyMetadata {
    /// Key this property's curve is stored under (e.g.
    /// `E_VALUECURVE_Morph_Start_X1`). Independent of the control key used
    /// for the base slider.
    func valueCurveKey(prefix: String) -> String {
        return "\(prefix)VALUECURVE_\(id)"
    }
}

// MARK: - Observable wrapper

/// SwiftUI-observable façade over XLValueCurve. Every setter writes through
/// to the ObjC core then re-serialises + persists via
/// `SequencerViewModel.setSettingValue`. That way the effect's settings map
/// is always in sync and an Undo of a setting change can re-read the
/// canonical string.
@Observable
@MainActor
final class EditableValueCurve {
    @ObservationIgnored let core: XLValueCurve
    @ObservationIgnored let storageKey: String
    @ObservationIgnored weak var viewModel: SequencerViewModel?

    /// Canonical "nothing persisted" value. When a curve drops back to
    /// inactive we pass this as `suppressIfDefault` so the settings map
    /// entry is removed rather than storing a dead `Active=FALSE|`.
    static let inactiveSerialised = "Active=FALSE|"

    init(serialised: String,
         id: String,
         min: Double,
         max: Double,
         divisor: Double,
         storageKey: String,
         viewModel: SequencerViewModel) {
        self.core = XLValueCurve(serialised: serialised,
                                  forId: id,
                                  min: min,
                                  max: max,
                                  divisor: divisor)
        self.storageKey = storageKey
        self.viewModel = viewModel
    }

    // Bump to force SwiftUI re-evaluation after mutations that don't
    // flow through an observed property (Custom-point writes).
    private var revision: Int = 0
    private func bump() { revision &+= 1 }

    // MARK: Core properties

    var isActive: Bool {
        get { _ = revision; return core.active }
        set { core.active = newValue; bump(); persist() }
    }

    var type: String {
        get { _ = revision; return core.type }
        set { core.type = newValue; bump(); persist() }
    }

    var minValue: Double {
        get { _ = revision; return core.minValue }
        set { core.minValue = newValue; bump(); persist() }
    }

    var maxValue: Double {
        get { _ = revision; return core.maxValue }
        set { core.maxValue = newValue; bump(); persist() }
    }

    var parameter1: Double {
        get { _ = revision; return core.parameter1 }
        set { core.parameter1 = newValue; bump(); persist() }
    }
    var parameter2: Double {
        get { _ = revision; return core.parameter2 }
        set { core.parameter2 = newValue; bump(); persist() }
    }
    var parameter3: Double {
        get { _ = revision; return core.parameter3 }
        set { core.parameter3 = newValue; bump(); persist() }
    }
    var parameter4: Double {
        get { _ = revision; return core.parameter4 }
        set { core.parameter4 = newValue; bump(); persist() }
    }

    var wrap: Bool {
        get { _ = revision; return core.wrap }
        set { core.wrap = newValue; bump(); persist() }
    }

    var realValues: Bool {
        get { _ = revision; return core.realValues }
        set { core.realValues = newValue; bump(); persist() }
    }

    var timeOffset: Int {
        get { _ = revision; return Int(core.timeOffset) }
        set { core.timeOffset = Int32(newValue); bump(); persist() }
    }

    var timingTrack: String {
        get { _ = revision; return core.timingTrack }
        set { core.timingTrack = newValue; bump(); persist() }
    }

    var audioTrack: String {
        get { _ = revision; return core.audioTrack }
        set { core.audioTrack = newValue; bump(); persist() }
    }

    var filterText: String {
        get { _ = revision; return core.filterText }
        set { core.filterText = newValue; bump(); persist() }
    }

    var filterIsRegex: Bool {
        get { _ = revision; return core.filterIsRegex }
        set { core.filterIsRegex = newValue; bump(); persist() }
    }

    // MARK: Custom points

    /// Flat [x1, y1, x2, y2, ...] read-through to ObjC. SwiftUI views call
    /// `pointPairs()` when they want structured tuples.
    func pointPairs() -> [(x: Double, y: Double)] {
        _ = revision
        let arr = core.customPoints()
        var out: [(Double, Double)] = []
        out.reserveCapacity(arr.count / 2)
        var i = 0
        while i + 1 < arr.count {
            out.append((arr[i].doubleValue, arr[i + 1].doubleValue))
            i += 2
        }
        return out
    }

    func setPoint(x: Double, y: Double) {
        core.addCustomPoint(x: x, y: y)
        bump()
        persist()
    }

    func deletePoint(x: Double) {
        core.deleteCustomPoint(x: x)
        bump()
        persist()
    }

    // MARK: Evaluation (for preview strip)

    func valueAt(offset: Double) -> Double {
        return core.value(atOffset: offset)
    }

    // MARK: Persistence

    private func persist() {
        let s = core.serialise()
        viewModel?.setSettingValue(s,
                                    forKey: storageKey,
                                    suppressIfDefault: Self.inactiveSerialised)
    }
}

// MARK: - VC button

/// Small pill rendered alongside a base slider when its metadata has
/// `valueCurve: true`. Shows active / inactive state and opens the full
/// editor on tap. Matches the "VC" chip desktop uses next to
/// BulkEditSlider in the settings panels.
struct ValueCurveButton: View {
    @Environment(SequencerViewModel.self) var viewModel
    let property: PropertyMetadata
    let prefix: String

    @State private var showEditor = false

    private var storageKey: String { property.valueCurveKey(prefix: prefix) }

    private var storedString: String {
        viewModel.settingValue(forKey: storageKey, defaultValue: "")
    }

    /// Active without deserialising — the first token of any serialised VC
    /// is `Active=TRUE|` or `Active=FALSE|`.
    private var isActive: Bool {
        storedString.hasPrefix("Active=TRUE")
    }

    var body: some View {
        Button(action: { showEditor = true }) {
            Image(systemName: "chart.xyaxis.line")
                .font(.caption)
                .frame(width: 24, height: 20)
                .foregroundStyle(isActive ? .white : .secondary)
                .background(
                    RoundedRectangle(cornerRadius: 4)
                        .fill(isActive ? Color.accentColor : Color.secondary.opacity(0.12))
                )
                .overlay(
                    RoundedRectangle(cornerRadius: 4)
                        .stroke(Color.secondary.opacity(0.35), lineWidth: 0.5)
                )
        }
        .buttonStyle(.plain)
        .sheet(isPresented: $showEditor) {
            ValueCurveEditorSheet(property: property,
                                  prefix: prefix,
                                  storedString: storedString)
        }
    }
}

// MARK: - Editor sheet

/// Modal editor. Creates a fresh EditableValueCurve from the stored string
/// on open; every mutation persists immediately through the view model so
/// closing the sheet or force-quitting the app doesn't lose edits.
struct ValueCurveEditorSheet: View {
    @Environment(SequencerViewModel.self) var viewModel
    @Environment(\.dismiss) private var dismiss
    let property: PropertyMetadata
    let prefix: String
    let storedString: String

    @State private var vc: EditableValueCurve?

    var body: some View {
        NavigationStack {
            Group {
                if let vc = vc {
                    editorContent(vc)
                } else {
                    ProgressView()
                }
            }
            .navigationTitle(property.label.isEmpty ? property.id : property.label)
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") { dismiss() }
                }
            }
        }
        .onAppear {
            if vc == nil {
                let storageKey = property.valueCurveKey(prefix: prefix)
                let minV = property.vcMin ?? property.min ?? 0
                let maxV = property.vcMax ?? property.max ?? 100
                let divisor = Double(property.effectiveDivisor)
                vc = EditableValueCurve(serialised: storedString,
                                         id: property.id,
                                         min: minV,
                                         max: maxV,
                                         divisor: divisor,
                                         storageKey: storageKey,
                                         viewModel: viewModel)
            }
        }
    }

    @ViewBuilder
    private func editorContent(_ vc: EditableValueCurve) -> some View {
        @Bindable var vc = vc
        Form {
            Section {
                Toggle("Active", isOn: $vc.isActive)
                    .toggleStyle(.switch)
                if vc.isActive {
                    ValueCurvePreviewStrip(vc: vc)
                        .frame(height: 64)
                        .listRowInsets(EdgeInsets(top: 4, leading: 8, bottom: 4, trailing: 8))
                }
            }

            if vc.isActive {
                Section("Type") {
                    Picker("Type", selection: $vc.type) {
                        ForEach(XLValueCurve.availableTypes(), id: \.self) { t in
                            Text(t).tag(t)
                        }
                    }
                    .pickerStyle(.menu)
                }

                let typeHasCustom = XLValueCurve.typeHasCustomPoints(vc.type)

                if typeHasCustom {
                    Section("Points") {
                        ValueCurveCustomPointEditor(vc: vc)
                            .frame(height: 220)
                            .listRowInsets(EdgeInsets(top: 8, leading: 8, bottom: 8, trailing: 8))
                    }
                }

                Section("Parameters") {
                    parameterRow(label: parameterLabel(type: vc.type, index: 1),
                                 value: $vc.parameter1, type: vc.type, parm: 1)
                    parameterRow(label: parameterLabel(type: vc.type, index: 2),
                                 value: $vc.parameter2, type: vc.type, parm: 2)
                    parameterRow(label: parameterLabel(type: vc.type, index: 3),
                                 value: $vc.parameter3, type: vc.type, parm: 3)
                    parameterRow(label: parameterLabel(type: vc.type, index: 4),
                                 value: $vc.parameter4, type: vc.type, parm: 4)
                }

                Section("Range") {
                    HStack {
                        Text("Min")
                        Spacer()
                        TextField("Min", value: $vc.minValue, format: .number)
                            .multilineTextAlignment(.trailing)
                            .keyboardType(.numbersAndPunctuation)
                    }
                    HStack {
                        Text("Max")
                        Spacer()
                        TextField("Max", value: $vc.maxValue, format: .number)
                            .multilineTextAlignment(.trailing)
                            .keyboardType(.numbersAndPunctuation)
                    }
                }

                Section {
                    Toggle("Wrap", isOn: $vc.wrap).toggleStyle(.switch)
                    Toggle("Real Values", isOn: $vc.realValues).toggleStyle(.switch)
                    HStack {
                        Text("Time Offset (ms)")
                        Spacer()
                        TextField("0", value: $vc.timeOffset, format: .number)
                            .multilineTextAlignment(.trailing)
                            .keyboardType(.numberPad)
                            .frame(width: 80)
                    }
                }

                // Timing track / audio track / filter fields — surfaced for
                // curves that consume them. Until C-4 lands the dynamicOptions
                // bridge, the text fields are free-entry to match desktop's
                // fallback behaviour when the track name isn't in the list.
                if typeUsesTimingTrack(vc.type) {
                    Section("Timing Track") {
                        TextField("Track name", text: $vc.timingTrack)
                    }
                }
                if typeUsesAudioTrack(vc.type) {
                    Section("Audio Track") {
                        TextField("Track name", text: $vc.audioTrack)
                    }
                }
                if typeUsesFilterLabel(vc.type) {
                    Section("Filter Label") {
                        TextField("Label text", text: $vc.filterText)
                        Toggle("Regex", isOn: $vc.filterIsRegex).toggleStyle(.switch)
                    }
                }
            }
        }
    }

    /// Per-type parameter range via core. Returned as a tuple so the
    /// ViewBuilder'd parameterRow can consume it inline (a ViewBuilder
    /// body can't contain a void call, which is what the C pointer-out
    /// signature produces).
    private func parameterRange(type: String, parm: Int32) -> (Double, Double) {
        var low: Double = 0
        var high: Double = 100
        XLValueCurve.range(forParameter: parm, type: type, outLow: &low, outHigh: &high)
        return (low, high)
    }

    @ViewBuilder
    private func parameterRow(label: String,
                               value: Binding<Double>,
                               type: String,
                               parm: Int32) -> some View {
        let (low, high) = parameterRange(type: type, parm: parm)
        VStack(alignment: .leading, spacing: 2) {
            HStack {
                Text(label)
                    .font(.caption)
                Spacer()
                Text(String(format: "%.2f", value.wrappedValue))
                    .monospacedDigit()
                    .font(.caption2)
                    .foregroundStyle(.secondary)
            }
            if high > low {
                Slider(value: value, in: low...high)
            } else {
                // Type doesn't use this parm — render a disabled placeholder
                // so the row height stays consistent.
                Slider(value: .constant(0), in: 0...1).disabled(true)
            }
        }
    }

    // MARK: - Per-type hints

    private func parameterLabel(type: String, index: Int) -> String {
        // Labels mirror desktop's ValueCurveDialog where param semantics
        // differ per type. For anything not special-cased we fall back to
        // "P<n>" so the user still has a live value to manipulate.
        switch (type, index) {
        case ("Ramp", 1): return "Start value"
        case ("Ramp", 2): return "End value"
        case ("Ramp Up/Down", 1): return "Start value"
        case ("Ramp Up/Down", 2): return "Mid value"
        case ("Ramp Up/Down", 3): return "End value"
        case ("Ramp Up/Down Hold", 1): return "Start value"
        case ("Ramp Up/Down Hold", 2): return "Mid value"
        case ("Ramp Up/Down Hold", 3): return "End value"
        case ("Ramp Up/Down Hold", 4): return "Hold"
        case ("Parabolic Up", 1), ("Parabolic Down", 1): return "Start value"
        case ("Parabolic Up", 2), ("Parabolic Down", 2): return "End value"
        case ("Parabolic Up", 3), ("Parabolic Down", 3): return "Shape"
        case ("Logarithmic Up", 1), ("Logarithmic Down", 1): return "Start value"
        case ("Logarithmic Up", 2), ("Logarithmic Down", 2): return "End value"
        case ("Exponential Up", 1), ("Exponential Down", 1): return "Start value"
        case ("Exponential Up", 2), ("Exponential Down", 2): return "End value"
        case ("Sine", 1), ("Abs Sine", 1), ("Decaying Sine", 1): return "Centre"
        case ("Sine", 2), ("Abs Sine", 2), ("Decaying Sine", 2): return "Amplitude"
        case ("Sine", 3), ("Abs Sine", 3), ("Decaying Sine", 3): return "Frequency"
        case ("Sine", 4), ("Abs Sine", 4), ("Decaying Sine", 4): return "Phase"
        case ("Square", 1): return "Low"
        case ("Square", 2): return "High"
        case ("Square", 3): return "Frequency"
        case ("Square", 4): return "Phase"
        case ("Saw Tooth", 1): return "Start"
        case ("Saw Tooth", 2): return "End"
        case ("Saw Tooth", 3): return "Frequency"
        case ("Music", 1), ("Inverted Music", 1): return "Low"
        case ("Music", 2), ("Inverted Music", 2): return "High"
        case ("Music", 3), ("Inverted Music", 3): return "Gain"
        case ("Music Trigger Fade", 1): return "Start value"
        case ("Music Trigger Fade", 2): return "Peak value"
        case ("Music Trigger Fade", 3): return "Fade time"
        case ("Timing Track Toggle", 1): return "Off value"
        case ("Timing Track Toggle", 2): return "On value"
        case ("Timing Track Fade Fixed", 1): return "Off value"
        case ("Timing Track Fade Fixed", 2): return "On value"
        case ("Timing Track Fade Fixed", 3): return "Fade time"
        case ("Timing Track Fade Proportional", 1): return "Off value"
        case ("Timing Track Fade Proportional", 2): return "On value"
        case ("Random", 1): return "Min"
        case ("Random", 2): return "Max"
        case ("Random", 3): return "Points"
        default: return "P\(index)"
        }
    }

    private func typeUsesTimingTrack(_ t: String) -> Bool {
        return t.hasPrefix("Timing Track")
    }
    private func typeUsesAudioTrack(_ t: String) -> Bool {
        return t == "Music" || t == "Inverted Music" || t == "Music Trigger Fade"
    }
    private func typeUsesFilterLabel(_ t: String) -> Bool {
        return t.hasPrefix("Timing Track")
    }
}
