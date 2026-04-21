import SwiftUI

// Additional effect-specific `controlType: custom` rows (C-6). Kept in a
// single file because most are small — a <20-line picker or radio row
// per entry. Heavy, effect-unique customs (Shader_DynamicParams,
// DMX_ChannelsNotebook, Shape_*, Sketch_*) still live on their own.

// MARK: - Shader_SpeedRow

/// Time-speed slider for the Shader effect. Desktop builds it as a
/// BulkEditSliderF2 (divisor 100, range 0..2000) with a VC button; we
/// synthesize the same PropertyMetadata and route through
/// `EffectPropertyView` so the slider, value readout, and VC editor
/// flow come for free. Backing: `E_SLIDER_Shader_Speed`,
/// `E_VALUECURVE_Shader_Speed`.
struct ShaderSpeedRowView: View {
    var body: some View {
        if let prop = PropertyMetadata.makeSynthetic(
            id: "Shader_Speed",
            label: "Time Speed",
            controlType: "slider",
            defaultValue: 100,
            min: 0,
            max: 2000,
            divisor: 100,
            valueCurve: true,
            settingPrefix: "SLIDER") {
            EffectPropertyView(property: prop, metadataPrefix: "E_")
        } else {
            Text("Time Speed (synthesis failed)")
                .font(.caption)
                .foregroundStyle(.red)
        }
    }
}

// MARK: - State_StateSource

/// Radio between "Direct state" and "Timing track" modes for the
/// State effect. Each mode populates a different backing key; the
/// unused key is cleared on switch so the stored effect string only
/// advertises one source.
///
/// Backing:
///   `E_CHOICE_State_State`         — selected state name
///   `E_CHOICE_State_TimingTrack`   — timing-track name
/// Whichever is non-empty wins at render time (see
/// StateEffect.cpp:62-63). Empty stored values → "no selection".
struct StateStateSourceRowView: View {
    @Environment(SequencerViewModel.self) var viewModel

    private let stateKey = "E_CHOICE_State_State"
    private let trackKey = "E_CHOICE_State_TimingTrack"

    private enum Mode: String, CaseIterable, Identifiable {
        case state = "State"
        case timingTrack = "Timing Track"
        var id: String { rawValue }
    }

    private var currentState: String {
        viewModel.settingValue(forKey: stateKey, defaultValue: "")
    }
    private var currentTrack: String {
        viewModel.settingValue(forKey: trackKey, defaultValue: "")
    }

    private var mode: Mode {
        currentTrack.isEmpty ? .state : .timingTrack
    }

    var body: some View {
        let modeBinding = Binding<Mode>(
            get: { mode },
            set: { newMode in
                // Switching modes clears the other field so the stored
                // string reflects exactly one source. The just-selected
                // side keeps whatever's there (empty → user picks).
                switch newMode {
                case .state:
                    viewModel.setSettingValue("", forKey: trackKey,
                                               suppressIfDefault: "")
                case .timingTrack:
                    viewModel.setSettingValue("", forKey: stateKey,
                                               suppressIfDefault: "")
                }
            }
        )

        VStack(alignment: .leading, spacing: 4) {
            Picker("Source", selection: modeBinding) {
                ForEach(Mode.allCases) { m in
                    Text(m.rawValue).tag(m)
                }
            }
            .pickerStyle(.segmented)

            switch mode {
            case .state:
                stateDropdown
            case .timingTrack:
                trackDropdown
            }
        }
        .padding(.vertical, 2)
    }

    private var stateDropdown: some View {
        let states = viewModel.dynamicOptions(source: "states",
                                               propertyId: "State_State")
        let binding = Binding<String>(
            get: { currentState },
            set: { viewModel.setSettingValue($0, forKey: stateKey,
                                              suppressIfDefault: "") }
        )
        return Picker("State", selection: binding) {
            Text("(none)").tag("")
            ForEach(states, id: \.self) { s in
                Text(s).tag(s)
            }
            if !currentState.isEmpty && !states.contains(currentState) {
                Text("\(currentState) (missing)").tag(currentState)
            }
        }
        .pickerStyle(.menu)
        .labelsHidden()
    }

    private var trackDropdown: some View {
        let tracks = viewModel.dynamicOptions(source: "timingTracks",
                                               propertyId: "State_TimingTrack")
        let binding = Binding<String>(
            get: { currentTrack },
            set: { viewModel.setSettingValue($0, forKey: trackKey,
                                              suppressIfDefault: "") }
        )
        return Picker("Track", selection: binding) {
            Text("(none)").tag("")
            ForEach(tracks, id: \.self) { t in
                Text(t).tag(t)
            }
            if !currentTrack.isEmpty && !tracks.contains(currentTrack) {
                Text("\(currentTrack) (missing)").tag(currentTrack)
            }
        }
        .pickerStyle(.menu)
        .labelsHidden()
    }
}

// MARK: - Faces_MouthMovements

/// Same radio pattern as `State_StateSource` but for the Faces effect:
/// pick a fixed phoneme for a static mouth shape OR a timing track for
/// automatic lip-sync. Phoneme list matches `FacesPanel.cpp:64-74`.
///
/// Backing:
///   `E_CHOICE_Faces_Phoneme`        — phoneme name (AI/E/FV/...)
///   `E_CHOICE_Faces_TimingTrack`    — timing track name
struct FacesMouthMovementsRowView: View {
    @Environment(SequencerViewModel.self) var viewModel

    private let phonemeKey = "E_CHOICE_Faces_Phoneme"
    private let trackKey   = "E_CHOICE_Faces_TimingTrack"

    /// Desktop phoneme list from FacesPanel.cpp. Order preserved.
    private static let phonemes: [String] = [
        "AI", "E", "FV", "L", "MBP", "O", "U", "WQ", "etc", "rest", "(off)",
    ]

    private enum Mode: String, CaseIterable, Identifiable {
        case phoneme = "Phoneme"
        case timingTrack = "Timing Track"
        var id: String { rawValue }
    }

    private var currentPhoneme: String {
        viewModel.settingValue(forKey: phonemeKey, defaultValue: "")
    }
    private var currentTrack: String {
        viewModel.settingValue(forKey: trackKey, defaultValue: "")
    }
    private var mode: Mode {
        currentTrack.isEmpty ? .phoneme : .timingTrack
    }

    var body: some View {
        let modeBinding = Binding<Mode>(
            get: { mode },
            set: { newMode in
                switch newMode {
                case .phoneme:
                    viewModel.setSettingValue("", forKey: trackKey,
                                               suppressIfDefault: "")
                case .timingTrack:
                    viewModel.setSettingValue("", forKey: phonemeKey,
                                               suppressIfDefault: "")
                }
            }
        )

        VStack(alignment: .leading, spacing: 4) {
            Picker("Source", selection: modeBinding) {
                ForEach(Mode.allCases) { m in
                    Text(m.rawValue).tag(m)
                }
            }
            .pickerStyle(.segmented)

            switch mode {
            case .phoneme:
                phonemeDropdown
            case .timingTrack:
                trackDropdown
            }
        }
        .padding(.vertical, 2)
    }

    private var phonemeDropdown: some View {
        let binding = Binding<String>(
            get: { currentPhoneme.isEmpty ? "AI" : currentPhoneme },
            set: { viewModel.setSettingValue($0, forKey: phonemeKey,
                                              suppressIfDefault: "") }
        )
        return Picker("Phoneme", selection: binding) {
            ForEach(Self.phonemes, id: \.self) { p in
                Text(p).tag(p)
            }
        }
        .pickerStyle(.menu)
        .labelsHidden()
    }

    private var trackDropdown: some View {
        // Faces timing tracks must be lyric-style (3 layers). Desktop
        // filters via `lyricTimingTracks`; iPad does the same.
        let tracks = viewModel.dynamicOptions(source: "lyricTimingTracks",
                                               propertyId: "Faces_TimingTrack")
        let binding = Binding<String>(
            get: { currentTrack },
            set: { viewModel.setSettingValue($0, forKey: trackKey,
                                              suppressIfDefault: "") }
        )
        return Picker("Track", selection: binding) {
            Text("(none)").tag("")
            ForEach(tracks, id: \.self) { t in
                Text(t).tag(t)
            }
            if !currentTrack.isEmpty && !tracks.contains(currentTrack) {
                Text("\(currentTrack) (missing)").tag(currentTrack)
            }
        }
        .pickerStyle(.menu)
        .labelsHidden()
    }
}

// MARK: - Morph_QuickSet

/// Preset-picker for the Morph effect. Each preset writes a fixed set
/// of Start/End X/Y values into the eight slider fields, replicating
/// the common full-sweep / single-sweep configurations. Selection
/// resets to blank after apply so the user can pick the same preset
/// twice.
struct MorphQuickSetRowView: View {
    @Environment(SequencerViewModel.self) var viewModel

    /// Presets mirror `MorphPanel::OnQuickSetSelect`. Each tuple is
    /// (Start_X1, Start_Y1, Start_X2, Start_Y2,
    ///  End_X1,   End_Y1,   End_X2,   End_Y2).
    private static let presets: [(String, (Int, Int, Int, Int, Int, Int, Int, Int))] = [
        ("L→R Full Sweep",    (0, 50,  0, 50,  100, 50, 100, 50)),
        ("R→L Full Sweep",    (100, 50, 100, 50, 0, 50, 0, 50)),
        ("T→B Full Sweep",    (50, 100, 50, 100, 50, 0, 50, 0)),
        ("B→T Full Sweep",    (50, 0, 50, 0, 50, 100, 50, 100)),
        ("Diagonal ↘",        (0, 100, 0, 100, 100, 0, 100, 0)),
        ("Diagonal ↗",        (0, 0, 0, 0, 100, 100, 100, 100)),
        ("L→R Single Sweep",  (0, 50,  100, 50, 100, 50, 0, 50)),
        ("R→L Single Sweep",  (100, 50, 0, 50, 0, 50, 100, 50)),
    ]

    @State private var showingMenu = false

    var body: some View {
        Menu {
            ForEach(Array(Self.presets.enumerated()), id: \.offset) { _, entry in
                Button(entry.0) {
                    apply(entry.1)
                }
            }
        } label: {
            HStack {
                Label("Quick Set", systemImage: "sparkles.rectangle.stack")
                    .font(.caption)
                Spacer()
                Image(systemName: "chevron.down")
                    .font(.caption2)
                    .foregroundStyle(.tertiary)
            }
            .padding(.horizontal, 8)
            .padding(.vertical, 6)
            .background(
                RoundedRectangle(cornerRadius: 6)
                    .fill(Color.secondary.opacity(0.12))
            )
        }
        .padding(.vertical, 2)
    }

    private func apply(_ v: (Int, Int, Int, Int, Int, Int, Int, Int)) {
        // All eight slider values. Morph stores sliders with both
        // SLIDER and TEXTCTRL keys on desktop, but iPad writes through
        // TEXTCTRL (the primary path in MorphEffect.cpp:119-130).
        let mapping: [(String, Int)] = [
            ("E_TEXTCTRL_Morph_Start_X1", v.0),
            ("E_TEXTCTRL_Morph_Start_Y1", v.1),
            ("E_TEXTCTRL_Morph_Start_X2", v.2),
            ("E_TEXTCTRL_Morph_Start_Y2", v.3),
            ("E_TEXTCTRL_Morph_End_X1",   v.4),
            ("E_TEXTCTRL_Morph_End_Y1",   v.5),
            ("E_TEXTCTRL_Morph_End_X2",   v.6),
            ("E_TEXTCTRL_Morph_End_Y2",   v.7),
        ]
        for (key, val) in mapping {
            viewModel.setSettingValue(String(val), forKey: key)
        }
    }
}

// MARK: - Servo_StartEndRow / ButtonRow

/// Servo Start + End value sliders on a single row. Legacy key naming
/// from `ServoPanel.cpp:86-136`:
///   Start → `E_TEXTCTRL_Servo`      (0..1000, divisor 10 → 0.0..100.0)
///   End   → `E_TEXTCTRL_EndValue`   (same range)
/// VC button on the Start side only (desktop parity —
/// `E_VALUECURVE_Servo`). End VC is not supported by the effect engine.
struct ServoStartEndRowView: View {
    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            if let startProp = PropertyMetadata.makeSynthetic(
                id: "Servo",
                label: "Start",
                controlType: "slider",
                defaultValue: 0,
                min: 0,
                max: 1000,
                divisor: 10,
                valueCurve: true) {
                EffectPropertyView(property: startProp, metadataPrefix: "E_")
            }
            if let endProp = PropertyMetadata.makeSynthetic(
                id: "EndValue",
                label: "End",
                controlType: "slider",
                defaultValue: 0,
                min: 0,
                max: 1000,
                divisor: 10,
                valueCurve: false) {
                EffectPropertyView(property: endProp, metadataPrefix: "E_")
            }
        }
    }
}

/// Servo action buttons — Sync, Equal, Swap. Mirrors
/// `ServoPanel.cpp::BuildButtonRow`.
///   Sync  — copies Start to End
///   Equal — same as Sync (kept separate for desktop parity)
///   Swap  — exchanges Start and End values
struct ServoButtonRowView: View {
    @Environment(SequencerViewModel.self) var viewModel

    private let startKey = "E_TEXTCTRL_Servo"
    private let endKey   = "E_TEXTCTRL_EndValue"
    private let startVCKey = "E_VALUECURVE_Servo"

    var body: some View {
        HStack(spacing: 6) {
            Button("Equal", action: equal)
                .buttonStyle(.bordered)
                .controlSize(.small)
            Button("Swap", action: swap)
                .buttonStyle(.bordered)
                .controlSize(.small)
        }
        .padding(.vertical, 2)
    }

    private func equal() {
        let s = viewModel.settingValue(forKey: startKey, defaultValue: "0")
        viewModel.setSettingValue(s, forKey: endKey)
    }

    private func swap() {
        let s = viewModel.settingValue(forKey: startKey, defaultValue: "0")
        let e = viewModel.settingValue(forKey: endKey, defaultValue: "0")
        if s == e { return }
        viewModel.setSettingValue(e, forKey: startKey)
        viewModel.setSettingValue(s, forKey: endKey)
    }
}

// MARK: - Shape effect customs

/// `Shape_Font` — font picker for the Emoji branch of the Shape effect.
/// Synthesises a PropertyMetadata with `controlType: "fontpicker"` so
/// the existing `FontpickerPropertyView` does the work. Stores as
/// `E_FONTPICKER_Shape_Font`.
struct ShapeFontRowView: View {
    var body: some View {
        if let prop = PropertyMetadata.makeSynthetic(
            id: "Shape_Font",
            label: "Character Font",
            type: "font",
            controlType: "fontpicker",
            defaultValue: "") {
            EffectPropertyView(property: prop, metadataPrefix: "E_")
        }
    }
}

/// `Shape_Char` — Unicode code-point spinner for the emoji glyph.
/// Range 32..917631 matches the JSON definition (all emoji code points
/// live below 128000 but the upper bound allows the Unicode Plane 17
/// ideographs that occasionally appear in custom sequences). Stores as
/// `E_SPINCTRL_Shape_Char`.
struct ShapeCharRowView: View {
    @Environment(SequencerViewModel.self) var viewModel

    private let key = "E_SPINCTRL_Shape_Char"
    private let defaultValue = 127876

    private var code: Int {
        Int(viewModel.settingValue(forKey: key,
                                    defaultValue: String(defaultValue))) ?? defaultValue
    }

    var body: some View {
        let binding = Binding<Int>(
            get: { code },
            set: { viewModel.setSettingValue(String($0), forKey: key,
                                              suppressIfDefault: String(defaultValue)) }
        )
        // Two compact controls: a Stepper for precise increment and a
        // text field so the user can paste a decimal code directly
        // (emoji-fu / copied from Unicode charts).
        let textBinding = Binding<String>(
            get: { String(code) },
            set: { s in
                if let n = Int(s.trimmingCharacters(in: .whitespaces)),
                   n >= 32, n <= 917631 {
                    binding.wrappedValue = n
                }
            }
        )
        return VStack(alignment: .leading, spacing: 2) {
            HStack {
                Text("Character Code").font(.caption)
                Spacer()
                // Preview the glyph so the user knows which emoji they
                // picked. Falls back to a placeholder when the code
                // isn't a valid scalar.
                Text(emojiFor(code))
                    .font(.title3)
                    .frame(minWidth: 32, alignment: .trailing)
            }
            HStack {
                TextField("", text: textBinding)
                    .textFieldStyle(.roundedBorder)
                    .keyboardType(.numberPad)
                    .font(.caption2)
                    .frame(maxWidth: 100)
                Stepper("", value: binding, in: 32...917631)
                    .labelsHidden()
            }
        }
        .padding(.vertical, 2)
    }

    private func emojiFor(_ code: Int) -> String {
        guard let scalar = Unicode.Scalar(code) else { return "—" }
        return String(scalar)
    }
}

/// `Shape_SkinTone` — choice picker with the fixed skin-tone palette.
/// Synthesised with `controlType: "choice"` so it's rendered by the
/// standard choice path.
struct ShapeSkinToneRowView: View {
    var body: some View {
        // We can't pass an `options` array through the synthetic helper
        // (JSONSerialization strips it on round-trip), so the choice
        // list is rendered explicitly here.
        _ShapeSkinToneImpl()
    }
}

private struct _ShapeSkinToneImpl: View {
    @Environment(SequencerViewModel.self) var viewModel

    private let key = "E_CHOICE_Shape_SkinTone"
    private let defaultValue = "Default"
    private static let tones = [
        "Default", "Light", "Medium Light", "Medium",
        "Medium Dark", "Dark",
    ]

    var body: some View {
        let binding = Binding<String>(
            get: {
                let v = viewModel.settingValue(forKey: key,
                                                defaultValue: defaultValue)
                return v.isEmpty ? defaultValue : v
            },
            set: { viewModel.setSettingValue($0, forKey: key,
                                              suppressIfDefault: defaultValue) }
        )
        return VStack(alignment: .leading, spacing: 2) {
            Text("Skin Tone").font(.caption)
            Picker("Skin Tone", selection: binding) {
                ForEach(Self.tones, id: \.self) { t in
                    Text(t).tag(t)
                }
            }
            .pickerStyle(.menu)
            .labelsHidden()
        }
        .padding(.vertical, 2)
    }
}

/// `SVG` (Shape effect) — filepicker for the .svg asset. Reuses the
/// `EffectFilenameBlockView` and writes to `E_FILEPICKERCTRL_SVG`.
struct ShapeSVGRowView: View {
    var body: some View {
        EffectFilenameBlockView(label: "SVG File",
                                 settingKey: "E_FILEPICKERCTRL_SVG",
                                 fileFilter: "SVG (*.svg)|*.svg",
                                 subdirectory: "Images")
    }
}

// MARK: - Ripple_SVG

/// Same pattern as `ShapeSVGRowView` but for the Ripple effect's
/// Shape-source-SVG branch. Backing: `E_FILEPICKERCTRL_Ripple_SVG`.
struct RippleSVGRowView: View {
    var body: some View {
        EffectFilenameBlockView(label: "SVG File",
                                 settingKey: "E_FILEPICKERCTRL_Ripple_SVG",
                                 fileFilter: "SVG (*.svg)|*.svg",
                                 subdirectory: "Images")
    }
}
