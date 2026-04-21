import SwiftUI

// Final batch of `controlType: "custom"` handlers (C-6 tail). Every row
// in here preserves whatever the user's desktop-authored sequence had
// stored for its property — either by binding directly to the backing
// setting key, or by surfacing the stored value in an editable /
// read-only form so the user can verify it's still there. Nothing in
// this pass is "pretty"; the goal is **no data loss on touch**. Full
// fidelity ports are follow-up work.

// MARK: - Color panel: BrightnessLevelRow

/// `BrightnessLevelRow` — a single "Brightness Level" checkbox. The
/// serialized key is `C_CHECKBOXBRIGHTNESSLEVEL` (note: no underscore
/// between CHECKBOX and BRIGHTNESSLEVEL, matching the legacy render
/// name in `PixelBuffer.cpp:1903`).
struct BrightnessLevelRowView: View {
    @Environment(SequencerViewModel.self) var viewModel

    private let key = "C_CHECKBOXBRIGHTNESSLEVEL"

    var body: some View {
        let binding = Binding<Bool>(
            get: {
                let v = viewModel.settingValue(forKey: key, defaultValue: "0")
                return v == "1" || v.lowercased() == "true"
            },
            set: { viewModel.setSettingValue($0 ? "1" : "0",
                                              forKey: key,
                                              suppressIfDefault: "0") }
        )
        return Toggle(isOn: binding) {
            Text("Brightness Level").font(.caption)
        }
        .toggleStyle(.switch)
        .controlSize(.small)
        .padding(.vertical, 2)
    }
}

// MARK: - Buffer panel: SubBuffer + RotoZoomPreset

// SubBuffer row moved to a dedicated interactive editor — see
// `SubBufferEditorView.swift`.

/// `RotoZoomPreset` — preset menu that writes values (and value
/// curves) into the other Roto-Zoom sliders on the Buffer panel. Not
/// serialised itself; the menu itself resets to idle after applying.
/// Matches `BufferPanel::OnPresetSelect` in
/// `src-ui-wx/sequencer/BufferPanel.cpp:313-416` — same reset
/// values, same VC shapes, same presets.
struct RotoZoomPresetRowView: View {
    @Environment(SequencerViewModel.self) var viewModel

    private enum Preset: String, CaseIterable, Identifiable {
        case noneReset   = "None - Reset"
        case revCW       = "1 Rev CW"
        case revCCW      = "1 Rev CCW"
        case explode     = "Explode"
        case collapse    = "Collapse"
        case explodeSpin = "Explode + Spin CW"
        case shake       = "Shake"
        case spinAccel   = "Spin CW Accelerate"
        var id: String { rawValue }
    }

    var body: some View {
        Menu {
            ForEach(Preset.allCases) { (p: Preset) in
                Button(p.rawValue) { apply(p) }
            }
        } label: {
            HStack {
                Label("Roto-Zoom Preset", systemImage: "wand.and.stars")
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

    // MARK: Slider / VC helpers (mirror BufferPanel.cpp lambdas)

    /// Set an integer slider value (stored as-is).
    private func setInt(_ id: String, _ value: Int, defaultValue: String) {
        viewModel.setSettingValue(String(value),
                                   forKey: "B_SLIDER_\(id)",
                                   suppressIfDefault: defaultValue)
    }

    /// Set a float slider value, scaled by `divisor` to match the
    /// property's stored-int representation.
    private func setFloat(_ id: String, _ value: Double,
                           divisor: Int, defaultValue: String) {
        let scaled = Int((value * Double(divisor)).rounded())
        viewModel.setSettingValue(String(scaled),
                                   forKey: "B_SLIDER_\(id)",
                                   suppressIfDefault: defaultValue)
    }

    /// Replace a VC slot with a `Ramp` curve from `p1` → `p2`. Min /
    /// max come from the property's natural slider range so the VC
    /// produces the right output scale when sampled.
    private func setRampVC(_ id: String,
                            p1: Double, p2: Double,
                            min: Double, max: Double, divisor: Int) {
        guard let vc = XLValueCurve(serialised: "",
                                      forId: id,
                                      min: min,
                                      max: max,
                                      divisor: Double(divisor)) else { return }
        vc.type = "Ramp"
        vc.parameter1 = p1
        vc.parameter2 = p2
        vc.active = true
        viewModel.setSettingValue(vc.serialise(),
                                   forKey: "B_VALUECURVE_\(id)",
                                   suppressIfDefault: "Active=FALSE|")
    }

    /// Replace a VC slot with a wrapped `Sine`. Only used by the
    /// Shake preset today but the helper stays general.
    private func setSineVC(_ id: String,
                            p1: Double, p2: Double, p3: Double, p4: Double,
                            min: Double, max: Double, divisor: Int,
                            wrap: Bool = true) {
        guard let vc = XLValueCurve(serialised: "",
                                      forId: id,
                                      min: min,
                                      max: max,
                                      divisor: Double(divisor)) else { return }
        vc.type = "Sine"
        vc.parameter1 = p1
        vc.parameter2 = p2
        vc.parameter3 = p3
        vc.parameter4 = p4
        vc.wrap = wrap
        vc.active = true
        viewModel.setSettingValue(vc.serialise(),
                                   forKey: "B_VALUECURVE_\(id)",
                                   suppressIfDefault: "Active=FALSE|")
    }

    /// Clear a VC slot (write `Active=FALSE|`, which the
    /// suppressIfDefault check then erases from the map).
    private func clearVC(_ id: String) {
        viewModel.setSettingValue("Active=FALSE|",
                                   forKey: "B_VALUECURVE_\(id)",
                                   suppressIfDefault: "Active=FALSE|")
    }

    // MARK: Apply

    private func apply(_ p: Preset) {
        // Match desktop OnPresetSelect: reset every Roto-Zoom slider +
        // clear every Roto-Zoom VC first, then apply the preset's
        // overrides. JSON defaults for each property are preserved as
        // the suppress-if-default target so the settings map stays
        // compact when a preset reverts a value to its default.
        //
        // Defaults from `shared/Buffer.json`:
        //   Zoom:        slider value 10 (= 1.0x), divisor 10, max 30
        //   PivotPointX/Y: 50, range 0..100
        //   Rotation:    0, range 0..100 (int)
        //   Rotations:   slider value 0 (= 0.0), divisor 10, max 200
        //   ZoomQuality: 1, range 1..10
        //   XPivot/YPivot: 50, range 0..100
        //   XRotation/YRotation: 0, range 0..360
        setFloat("Zoom",        1.0, divisor: 10, defaultValue: "10")
        setInt("PivotPointX",   50,                defaultValue: "50")
        setInt("PivotPointY",   50,                defaultValue: "50")
        setInt("Rotation",       0,                defaultValue: "0")
        setFloat("Rotations",   0.0, divisor: 10, defaultValue: "0")
        setInt("ZoomQuality",    1,                defaultValue: "1")
        setInt("XPivot",        50,                defaultValue: "50")
        setInt("YPivot",        50,                defaultValue: "50")
        setInt("XRotation",      0,                defaultValue: "0")
        setInt("YRotation",      0,                defaultValue: "0")
        for id in ["Zoom", "PivotPointX", "PivotPointY", "Rotation",
                   "Rotations", "XPivot", "YPivot",
                   "XRotation", "YRotation"] {
            clearVC(id)
        }

        switch p {
        case .noneReset:
            break

        case .revCW:
            setFloat("Rotations", 1.0, divisor: 10, defaultValue: "0")
            setRampVC("Rotation", p1: 0, p2: 100,
                      min: 0, max: 100, divisor: 1)
            setInt("ZoomQuality", 2, defaultValue: "1")

        case .revCCW:
            setFloat("Rotations", 1.0, divisor: 10, defaultValue: "0")
            setRampVC("Rotation", p1: 100, p2: 0,
                      min: 0, max: 100, divisor: 1)
            setInt("ZoomQuality", 2, defaultValue: "1")

        case .explode:
            // Zoom range 0..30 stored with divisor 10 → VC in same
            // stored units (min 0, max 30).
            setRampVC("Zoom", p1: 0, p2: 10,
                      min: 0, max: 30, divisor: 10)

        case .collapse:
            setRampVC("Zoom", p1: 10, p2: 0,
                      min: 0, max: 30, divisor: 10)

        case .explodeSpin:
            setRampVC("Zoom", p1: 0, p2: 10,
                      min: 0, max: 30, divisor: 10)
            setFloat("Rotations", 1.0, divisor: 10, defaultValue: "0")
            setRampVC("Rotation", p1: 0, p2: 100,
                      min: 0, max: 100, divisor: 1)
            setInt("ZoomQuality", 2, defaultValue: "1")

        case .shake:
            setFloat("Rotations", 1.0, divisor: 10, defaultValue: "0")
            setSineVC("Rotation",
                      p1: 0, p2: 10, p3: 50, p4: 25,
                      min: 0, max: 100, divisor: 1,
                      wrap: true)
            setInt("ZoomQuality", 2, defaultValue: "1")

        case .spinAccel:
            setRampVC("Rotation", p1: 0, p2: 100,
                      min: 0, max: 100, divisor: 1)
            // Rotations stored with divisor 10; VC's Min/Max are in
            // SCALED-int units (matches desktop `SetLimits` on the
            // pre-scaled slider bounds).
            setRampVC("Rotations", p1: 0, p2: 10,
                      min: 0, max: 200, divisor: 10)
            setInt("ZoomQuality", 2, defaultValue: "1")
        }

        // Desktop also resets the rotation-order choice after every
        // preset (index 0 = "X-Y-Z").
        viewModel.setSettingValue("X-Y-Z",
                                   forKey: "B_CHOICE_RZ_RotationOrder",
                                   suppressIfDefault: "X-Y-Z")
    }
}

// MARK: - Text_Font_XL_Row

/// `Text_Font_XL_Row` — picker for the xLights bundled bitmap fonts.
/// The option list lives in `Text.json`'s `options` array (kept in
/// sync with `FontManager::get_font_names()` on desktop, which is
/// also driven by that JSON now). "Use OS Fonts" is the default
/// sentinel — empty storage means "fall through to the regular
/// Text_Font OS fontpicker above".
///
/// Can't be a plain `choice` controlType because the id "Text_Font"
/// already belongs to the OS fontpicker (same id, different setting
/// prefix — CHOICE vs FONTPICKER). The legacy on-disk key
/// `E_CHOICE_Text_Font` is kept intact by continuing to route through
/// the custom dispatcher.
struct TextFontXLRowView: View {
    @Environment(SequencerViewModel.self) var viewModel
    let property: PropertyMetadata

    private let key = "E_CHOICE_Text_Font"
    private var defaultValue: String {
        property.defaultAsString().isEmpty ? "Use OS Fonts"
                                            : property.defaultAsString()
    }

    var body: some View {
        let options = property.options ?? ["Use OS Fonts"]
        let binding = Binding<String>(
            get: {
                let v = viewModel.settingValue(forKey: key,
                                                defaultValue: defaultValue)
                return v.isEmpty ? defaultValue : v
            },
            set: { viewModel.setSettingValue($0,
                                              forKey: key,
                                              suppressIfDefault: defaultValue) }
        )
        return VStack(alignment: .leading, spacing: 2) {
            Text(property.label.isEmpty ? "XL Font" : property.label)
                .font(.caption)
            Picker("XL Font", selection: binding) {
                ForEach(options, id: \.self) { (name: String) in
                    Text(name).tag(name)
                }
                // Preserve a stored value that isn't in the option
                // list (e.g. sequence authored against a newer font
                // set) so selecting it later doesn't drop the tag.
                let current = binding.wrappedValue
                if !current.isEmpty && !options.contains(current) {
                    Text("\(current) (missing)").tag(current)
                }
            }
            .pickerStyle(.menu)
            .labelsHidden()
        }
        .padding(.vertical, 2)
    }
}

// MARK: - Sketch

/// `Sketch_Info` — pure display text. Nothing to preserve; keep empty
/// so the inspector doesn't carry dead weight.
struct SketchInfoRowView: View {
    var body: some View {
        Text("Sketch paths are drawn via the desktop Effect Assist. " +
             "iPad playback works, but the path editor is desktop-only.")
            .font(.caption2)
            .foregroundStyle(.secondary)
            .padding(.vertical, 2)
    }
}

/// `Sketch_DefRow` — read-only display of the stored sketch definition
/// string (the polyline encoding). Backing `E_TEXTCTRL_SketchDef`. Not
/// editable on iPad — desktop's sketch assistant is the only way to
/// author these. The full string is stored in the settings map and
/// round-trips untouched.
struct SketchDefRowView: View {
    @Environment(SequencerViewModel.self) var viewModel

    private let key = "E_TEXTCTRL_SketchDef"

    private var stored: String {
        viewModel.settingValue(forKey: key, defaultValue: "")
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 2) {
            Text("Sketch Definition")
                .font(.caption)
            Text(stored.isEmpty
                 ? "(no sketch — drawn via desktop Effect Assist)"
                 : summary(of: stored))
                .font(.caption2)
                .foregroundStyle(.secondary)
                .lineLimit(2)
                .truncationMode(.middle)
                .frame(maxWidth: .infinity, alignment: .leading)
                .textSelection(.enabled)
        }
        .padding(.vertical, 2)
    }

    private func summary(of s: String) -> String {
        "\(s.count) chars · \(s.split(separator: ";").count) segments"
    }
}

/// `Sketch_BackgroundRow` — background image for the sketch-tracing
/// reference. Reuses the existing `EffectFilenameBlockView` so the
/// file lands in the enforced media roots.
struct SketchBackgroundRowView: View {
    var body: some View {
        EffectFilenameBlockView(
            label: "Background Image",
            settingKey: "E_FILEPICKER_SketchBackground",
            fileFilter: "Images (*.png;*.jpg;*.jpeg;*.gif;*.bmp;*.webp)|*.png;*.jpg;*.jpeg;*.gif;*.bmp;*.webp",
            subdirectory: "Images")
    }
}

// MARK: - Video_DurationRow

/// `Video_DurationRow` — shows the picked video's duration (probed
/// via the `videoDurationMS` bridge, which shares the duration
/// cached on the `VideoMediaCacheEntry` with the render engine) and
/// a "Match effect length" button that resizes the effect's end
/// time so `end - start == video.duration`. Matches desktop's
/// `VideoPanel::OnMatchToVideoDuration` behaviour.
///
/// The probe runs async off the main thread and re-runs whenever
/// the `E_FILEPICKERCTRL_Video_Filename` setting changes. "Duration
/// unknown" stays visible while the probe is in flight or if the
/// file can't be resolved.
struct VideoDurationRowView: View {
    @Environment(SequencerViewModel.self) var viewModel

    private let videoKey = "E_FILEPICKERCTRL_Video_Filename"

    @State private var durationMS: Int? = nil
    @State private var probedPath: String = ""
    @State private var probeError: String? = nil

    private var storedPath: String {
        viewModel.settingValue(forKey: videoKey, defaultValue: "")
    }

    private var filename: String {
        if storedPath.isEmpty { return "(no video selected)" }
        return (storedPath as NSString).lastPathComponent
    }

    private var durationLabel: String {
        guard let ms = durationMS else {
            if storedPath.isEmpty { return "—" }
            if probeError != nil   { return "Unknown" }
            return "Probing…"
        }
        return formatMS(ms)
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 2) {
            HStack {
                Text("Video Duration").font(.caption)
                Spacer()
                Text(durationLabel)
                    .monospacedDigit()
                    .font(.caption2)
                    .foregroundStyle(.secondary)
            }
            Text(filename)
                .font(.caption2)
                .foregroundStyle(.tertiary)
                .lineLimit(1)
                .truncationMode(.middle)
            Button("Match Effect to Video Duration",
                    action: matchEffectToDuration)
                .buttonStyle(.bordered)
                .controlSize(.small)
                .disabled(durationMS == nil)
            if let err = probeError, !storedPath.isEmpty {
                Text(err)
                    .font(.caption2)
                    .foregroundStyle(.orange)
                    .lineLimit(2)
                    .truncationMode(.tail)
            }
        }
        .padding(.vertical, 2)
        .task(id: storedPath) { await probeDuration() }
    }

    private func probeDuration() async {
        let path = storedPath
        guard path != probedPath || durationMS == nil else { return }
        probedPath = path
        durationMS = nil
        probeError = nil
        guard !path.isEmpty else { return }
        let doc = viewModel.document
        // Route through the bridge: it resolves the path via
        // FixFile, honours the show/media folder security-scoped
        // bookmark, and reuses the duration cached on the entry's
        // `VideoMediaCacheEntry` when the video has already been
        // loaded for preview / render. First call is cheap (just a
        // container-header probe via `VideoReader::GetVideoLength`);
        // repeat calls hit the atomic cache.
        let ms: Int = await withCheckedContinuation { cont in
            DispatchQueue.global(qos: .utility).async {
                cont.resume(returning: Int(doc.videoDurationMS(forPath: path)))
            }
        }
        if ms > 0 {
            durationMS = ms
        } else {
            probeError = "Couldn't read video duration."
        }
    }

    private func matchEffectToDuration() {
        guard let ms = durationMS,
              let sel = viewModel.selectedEffect else { return }
        // Desktop sets end = start + duration (clamped to sequence
        // length upstream). We let `resizeEffectEdge` apply any
        // neighbour-overlap clamping.
        let newEndMS = sel.startTimeMS + ms
        viewModel.resizeEffectEdge(rowIndex: sel.rowIndex,
                                    effectIndex: sel.effectIndex,
                                    edge: 1,
                                    newMS: newEndMS)
    }

    private func formatMS(_ ms: Int) -> String {
        let totalSeconds = ms / 1000
        let minutes = totalSeconds / 60
        let seconds = totalSeconds % 60
        let hundredths = (ms % 1000) / 10
        return String(format: "%d:%02d.%02d", minutes, seconds, hundredths)
    }
}

// MARK: - DMX

/// `DMX_ChannelsNotebook` — up to 48 channels, each a slider (0..255)
/// plus optional invert checkbox. This minimal version surfaces all
/// 48 sliders + invert toggles so desktop-authored sequences fully
/// round-trip. Value-curve buttons per channel are follow-up work.
///
/// Keys:
///   `E_SLIDER_DMX<N>`         (0..255)
///   `E_CHECKBOX_INVDMX<N>`    (bool)
///
/// Channels at their default zero are NOT written (suppressIfDefault)
/// so the settings map stays compact.
struct DMXChannelsNotebookView: View {
    @Environment(SequencerViewModel.self) var viewModel

    private static let channelCount = 48

    // Expand groups one-at-a-time so the inspector sidebar doesn't
    // become a 48-row monster by default. Persisted via @AppStorage
    // so a user editing banks 17-32 across a pile of DMX effects
    // doesn't have to re-expand the bank on every selection change
    // (G2-b). -1 = all collapsed; 0/1/2 = that bank expanded.
    @AppStorage("DMXExpandedGroup") private var expandedGroupRaw: Int = 0

    var body: some View {
        VStack(alignment: .leading, spacing: 2) {
            Text("DMX Channels")
                .font(.caption)
                .fontWeight(.medium)
                .foregroundStyle(.secondary)

            // Three groups of 16 channels each — matches desktop's
            // notebook-with-3-tabs layout.
            ForEach(0..<3, id: \.self) { (group: Int) in
                let start = group * 16 + 1
                let end = min(start + 15, Self.channelCount)
                DisclosureGroup(
                    isExpanded: Binding(
                        get: { expandedGroupRaw == group },
                        set: { expandedGroupRaw = $0 ? group : -1 }
                    )
                ) {
                    VStack(spacing: 0) {
                        ForEach(start...end, id: \.self) { (ch: Int) in
                            DMXChannelRow(channel: ch)
                        }
                    }
                } label: {
                    Text("Channels \(start)–\(end)")
                        .font(.caption2)
                }
            }
        }
        .padding(.vertical, 2)
    }
}

private struct DMXChannelRow: View {
    @Environment(SequencerViewModel.self) var viewModel
    let channel: Int

    private var sliderKey: String { "E_SLIDER_DMX\(channel)" }
    private var invertKey: String { "E_CHECKBOX_INVDMX\(channel)" }
    private var vcKey:     String { "E_VALUECURVE_DMX\(channel)" }

    private var value: Int {
        Int(viewModel.settingValue(forKey: sliderKey, defaultValue: "0")) ?? 0
    }
    private var inverted: Bool {
        let v = viewModel.settingValue(forKey: invertKey, defaultValue: "0")
        return v == "1" || v.lowercased() == "true"
    }
    private var vcActive: Bool {
        viewModel.settingValue(forKey: vcKey, defaultValue: "")
            .hasPrefix("Active=TRUE")
    }

    /// Synthesised PropertyMetadata driving the VC button. `id` is
    /// `DMX<N>` so the editor stores under `E_VALUECURVE_DMX<N>`,
    /// matching desktop's DMXEffect keys (`DMXEffect.cpp:45-76`).
    private var vcProp: PropertyMetadata? {
        PropertyMetadata.makeSynthetic(
            id: "DMX\(channel)",
            label: "Channel \(channel)",
            type: "int",
            controlType: "slider",
            defaultValue: 0,
            min: 0,
            max: 255,
            divisor: 1,
            valueCurve: true)
    }

    var body: some View {
        let sliderBinding = Binding<Double>(
            get: { Double(value) },
            set: { viewModel.setSettingValue(String(Int($0)),
                                              forKey: sliderKey,
                                              suppressIfDefault: "0") }
        )
        let invertBinding = Binding<Bool>(
            get: { inverted },
            set: { viewModel.setSettingValue($0 ? "1" : "0",
                                              forKey: invertKey,
                                              suppressIfDefault: "0") }
        )

        VStack(alignment: .leading, spacing: 2) {
            HStack(spacing: 6) {
                Text("Ch \(channel)")
                    .font(.caption2)
                    .frame(width: 44, alignment: .leading)
                EditableNumberField(
                    storedInt: value, min: 0, max: 255, divisor: 1,
                    commit: { newInt in
                        viewModel.setSettingValue(String(newInt),
                                                    forKey: sliderKey,
                                                    suppressIfDefault: "0")
                    })
                .disabled(vcActive)
                if let prop = vcProp {
                    ValueCurveButton(property: prop, prefix: "E_")
                }
                Toggle(isOn: invertBinding) { Text("Inv").font(.caption2) }
                    .toggleStyle(.switch)
                    .controlSize(.small)
                    .labelsHidden()
            }
            Slider(value: sliderBinding, in: 0...255, step: 1)
                .opacity(vcActive ? 0.4 : 1.0)
                .disabled(vcActive)
        }
        .padding(.vertical, 2)
    }
}

/// `DMX_ButtonsRow` — Remap / Save As State / Load From State. Each
/// depends on desktop dialog flow (model state read/write) that
/// hasn't been ported. Render disabled buttons so the user sees the
/// desktop feature exists but can't accidentally trigger something
/// half-built.
struct DMXButtonsRowView: View {
    var body: some View {
        HStack(spacing: 6) {
            Button("Remap") {}
                .buttonStyle(.bordered)
                .controlSize(.small)
                .disabled(true)
            Button("Save State") {}
                .buttonStyle(.bordered)
                .controlSize(.small)
                .disabled(true)
            Button("Load State") {}
                .buttonStyle(.bordered)
                .controlSize(.small)
                .disabled(true)
        }
        .padding(.vertical, 2)
    }
}
