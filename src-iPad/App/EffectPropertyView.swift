import SwiftUI

// Renders a single effect property as a SwiftUI control, based on its
// PropertyMetadata. Reads and writes through SequencerViewModel so changes
// propagate to the underlying Effect via XLSequenceDocument.
struct EffectPropertyView: View {
    @Environment(SequencerViewModel.self) var viewModel
    let property: PropertyMetadata
    let metadataPrefix: String      // "E_", "B_", "C_", "T_"

    private var settingKey: String { property.settingKey(prefix: metadataPrefix) }
    private var defaultValueString: String { property.defaultAsString() }

    /// Current raw value (string) from the underlying effect, falling back to
    /// the metadata default if not set. Empty selection → default.
    private var rawValue: String {
        viewModel.settingValue(forKey: settingKey, defaultValue: defaultValueString)
    }

    /// For properties flagged `suppressIfDefault: true` in the JSON metadata,
    /// pass the default through to the setter so it removes the key instead
    /// of persisting a redundant default. Nil → always write.
    private var suppressDefault: String? {
        (property.suppressIfDefault == true) ? defaultValueString : nil
    }

    private func writeValue(_ value: String) {
        viewModel.setSettingValue(value,
                                   forKey: settingKey,
                                   suppressIfDefault: suppressDefault)
    }

    /// Runtime disable state for a property driven by OTHER properties'
    /// current values. Mirrors desktop's `BlendingPanel::ValidateWindow`
    /// (can't be expressed via the JSON visibility engine — needs a
    /// compound `fade==0 OR type ∈ set` check).
    ///
    /// Today only the transition Adjust / Reverse rows on the Blending
    /// tab consult this; any future runtime-disable rule hangs off the
    /// same switch.
    private var runtimeDisabled: Bool {
        switch property.id {
        case "In_Transition_Adjust":
            return transitionAdjustDisabled(isIn: true)
        case "Out_Transition_Adjust":
            return transitionAdjustDisabled(isIn: false)
        case "In_Transition_Reverse":
            return transitionReverseDisabled(isIn: true)
        case "Out_Transition_Reverse":
            return transitionReverseDisabled(isIn: false)
        default:
            return false
        }
    }

    private func transitionAdjustDisabled(isIn: Bool) -> Bool {
        if fadeIsZero(isIn: isIn) { return true }
        let type = currentTransitionType(isIn: isIn)
        return kTransitionsNoAdjust.contains(type)
    }

    private func transitionReverseDisabled(isIn: Bool) -> Bool {
        if fadeIsZero(isIn: isIn) { return true }
        let type = currentTransitionType(isIn: isIn)
        return kTransitionsNoReverse.contains(type)
    }

    private func fadeIsZero(isIn: Bool) -> Bool {
        let key = isIn ? "T_TEXTCTRL_Fadein" : "T_TEXTCTRL_Fadeout"
        let v = viewModel.settingValue(forKey: key, defaultValue: "0.00")
            .trimmingCharacters(in: .whitespaces)
        return v.isEmpty || v == "0" || v == "0.0" || v == "0.00"
            || (Double(v) ?? 0) == 0
    }

    private func currentTransitionType(isIn: Bool) -> String {
        let key = isIn ? "T_CHOICE_In_Transition_Type"
                        : "T_CHOICE_Out_Transition_Type"
        let v = viewModel.settingValue(forKey: key, defaultValue: "Fade")
        return v.isEmpty ? "Fade" : v
    }

    var body: some View {
        switch property.controlType {
        case "slider":
            sliderView
        case "checkbox", "togglebutton":
            checkboxView
        case "choice", "combobox":
            choiceView
        case "spin":
            spinView
        case "text":
            textView
        case "filepicker":
            FilepickerPropertyView(property: property,
                                    currentPath: rawValue,
                                    onChoose: { writeValue($0) },
                                    onClear: { writeValue("") })
        case "fontpicker":
            FontpickerPropertyView(property: property,
                                    currentDesc: rawValue,
                                    onChange: { writeValue($0) })
        case "point2d":
            Point2DPropertyView(property: property,
                                  metadataPrefix: metadataPrefix)
        case "custom":
            customView
        default:
            unsupportedPlaceholder
        }
    }

    // MARK: - Custom property dispatcher

    // Route known custom property ids to their bespoke implementations.
    // Unknown custom rows show a neutral placeholder. C-6 fills this out
    // effect-by-effect in priority order.
    @ViewBuilder
    private var customView: some View {
        switch property.id {
        case "PaletteHeaderRow":
            ColorPaletteView()
        case "Pictures_FilenameBlock":
            EffectFilenameBlockView(label: "Image",
                                     settingKey: "E_TEXTCTRL_Pictures_Filename",
                                     fileFilter: "Images (*.png;*.jpg;*.jpeg;*.gif;*.bmp;*.webp)|*.png;*.jpg;*.jpeg;*.gif;*.bmp;*.webp",
                                     subdirectory: "Images")
        case "Video_FilenameBlock":
            EffectFilenameBlockView(label: "Video",
                                     settingKey: "E_FILEPICKERCTRL_Video_Filename",
                                     fileFilter: "Videos (*.mp4;*.mov;*.m4v;*.avi;*.webm;*.mkv)|*.mp4;*.mov;*.m4v;*.avi;*.webm;*.mkv",
                                     subdirectory: "Videos")
        case "Shader_FilenameBlock":
            EffectFilenameBlockView(label: "Shader",
                                     settingKey: "E_0FILEPICKERCTRL_IFS",
                                     fileFilter: "Shader (*.fs)|*.fs",
                                     subdirectory: "Shaders")
        case "Shader_DynamicParams":
            ShaderDynamicParamsView()
        case "Pictures_TransparentBlackRow":
            TransparentBlackRowView(effectKeyStem: "Pictures")
        case "Video_TransparentBlackRow":
            TransparentBlackRowView(effectKeyStem: "Video")
        case "Faces_TransparentBlackRow":
            TransparentBlackRowView(effectKeyStem: "Faces")
        case "Text_File_Row":
            EffectFilenameBlockView(label: "From File",
                                     settingKey: "E_FILEPICKERCTRL_Text_File",
                                     fileFilter: "Text (*.txt)|*.txt",
                                     subdirectory: "")
        case "Morph_Swap":
            MorphSwapRowView()
        case "LayerMorphRow":
            LayerMorphRowView()
        case "LayerMethodRow":
            LayerMethodRowView()
        case "CanvasRow":
            CanvasRowView()
        case "In_Transition_Header":
            TransitionHeaderRowView(isIn: true)
        case "Out_Transition_Header":
            TransitionHeaderRowView(isIn: false)
        case "ChromaKeyRow":
            ChromaKeyRowView()
        case "SparklesRow":
            SparklesRowView()
        case "Shader_SpeedRow":
            ShaderSpeedRowView()
        case "State_StateSource":
            StateStateSourceRowView()
        case "Faces_MouthMovements":
            FacesMouthMovementsRowView()
        case "Morph_QuickSet":
            MorphQuickSetRowView()
        case "Servo_StartEndRow":
            ServoStartEndRowView()
        case "Servo_ButtonRow":
            ServoButtonRowView()
        case "Shape_Font":
            ShapeFontRowView()
        case "Shape_Char":
            ShapeCharRowView()
        case "Shape_SkinTone":
            ShapeSkinToneRowView()
        case "SVG":
            ShapeSVGRowView()
        case "Ripple_SVG":
            RippleSVGRowView()
        case "BrightnessLevelRow":
            BrightnessLevelRowView()
        case "SubBuffer":
            SubBufferEditorView()
        case "RotoZoomPreset":
            RotoZoomPresetRowView()
        case "Text_Font_XL_Row":
            TextFontXLRowView(property: property)
        case "Sketch_Info":
            SketchInfoRowView()
        case "Sketch_DefRow":
            SketchDefRowView()
        case "Sketch_BackgroundRow":
            SketchBackgroundRowView()
        case "Video_DurationRow":
            VideoDurationRowView()
        case "DMX_ChannelsNotebook":
            DMXChannelsNotebookView()
        case "DMX_ButtonsRow":
            DMXButtonsRowView()
        case "ResetPanelRow":
            // Desktop's "Reset panel when changing effects" preference
            // checkbox — persisted to wxConfig (xLightsResetColorPanel /
            // xLightsResetBufferPanel / xLightsResetBlendingPanel), not
            // serialized with the effect. iPad doesn't currently reset
            // panel state on effect change, so the toggle has nothing to
            // govern. Render nothing (and don't leave a "(custom)"
            // placeholder) rather than show a dead control.
            EmptyView()
        default:
            customPlaceholder
        }
    }

    // MARK: - Slider

    private var sliderView: some View {
        let divisor = property.effectiveDivisor
        let minVal = property.min ?? 0
        let maxVal = property.max ?? 100

        // Derive live values from the observed rawValue on every body eval,
        // so the displayed text updates as the user slides.
        let storedInt = Int(rawValue) ?? 0

        let sliderBinding = Binding<Double>(
            get: { Double(storedInt) },
            set: { newVal in
                writeValue(String(Int(newVal)))
            }
        )

        // An active value curve replaces this slider's runtime value. Dim
        // the slider while one is active — the user can still scrub to
        // pre-set a fallback, but it won't apply until they toggle the
        // curve off in the VC editor.
        let vcKey = property.valueCurveKey(prefix: metadataPrefix)
        let vcActive = property.valueCurve == true
            && viewModel.settingValue(forKey: vcKey, defaultValue: "")
                .hasPrefix("Active=TRUE")

        // `runtimeDisabled` flips on when a sibling property's value
        // makes this one meaningless (e.g. fade==0 disables Adjust /
        // Reverse on a Blending transition). The slider still renders
        // so the user sees the setting exists + its current value —
        // just dimmed + non-editable. Separate from vcActive because
        // they have different triggers + different meanings.
        let disabled = vcActive || runtimeDisabled

        return VStack(alignment: .leading, spacing: 2) {
            HStack(spacing: 6) {
                Text(property.label)
                    .font(.caption)
                Spacer()
                // Editable value field — users can tap to type a precise
                // value instead of scrubbing the slider (critical for
                // large ranges and float-divisor sliders). Clamps to
                // [min, max] on commit; invalid input reverts on blur.
                EditableNumberField(
                    storedInt: storedInt,
                    min: Int(minVal),
                    max: Int(maxVal),
                    divisor: divisor,
                    commit: { newInt in writeValue(String(newInt)) }
                )
                .disabled(disabled)
                if property.valueCurve == true {
                    ValueCurveButton(property: property, prefix: metadataPrefix)
                        .disabled(runtimeDisabled)
                }
            }
            Slider(value: sliderBinding, in: minVal...maxVal, step: 1)
                .opacity(disabled ? 0.4 : 1.0)
                .disabled(disabled)
        }
        .padding(.vertical, 2)
        .propertyContextMenu(property: property, prefix: metadataPrefix)
    }

    // MARK: - Checkbox

    private var checkboxView: some View {
        let binding = Binding<Bool>(
            get: { rawValue == "1" || rawValue.lowercased() == "true" },
            set: { newVal in
                writeValue(newVal ? "1" : "0")
            }
        )
        let label = property.checkboxLabel ?? property.label
        return Toggle(isOn: binding) {
            Text(label.isEmpty ? property.label : label)
                .font(.caption)
                .foregroundStyle(runtimeDisabled ? .tertiary : .primary)
        }
        .toggleStyle(.switch)
        .disabled(runtimeDisabled)
        .opacity(runtimeDisabled ? 0.5 : 1.0)
        .padding(.vertical, 2)
        .propertyContextMenu(property: property, prefix: metadataPrefix)
    }

    // MARK: - Choice / Combobox

    /// Resolves the option list for this choice. `dynamicOptions` takes
    /// precedence over the metadata's static `options` array — matches
    /// desktop's JsonEffectPanel (settings re-populated per selection
    /// change). If the dynamic source returns empty the list degrades to
    /// just the current value so the menu isn't blank.
    private var choiceOptions: [String] {
        if let source = property.dynamicOptions, !source.isEmpty {
            let dyn = viewModel.dynamicOptions(source: source,
                                                propertyId: property.id)
            if !dyn.isEmpty { return dyn }
            // Keep the current value selectable even when the source is
            // empty (e.g. a model with no states yet).
            let v = rawValue.isEmpty ? defaultValueString : rawValue
            return v.isEmpty ? [] : [v]
        }
        return property.options ?? []
    }

    private var choiceView: some View {
        let options = choiceOptions
        let binding = Binding<String>(
            get: { rawValue.isEmpty ? defaultValueString : rawValue },
            set: { writeValue($0) }
        )
        return VStack(alignment: .leading, spacing: 2) {
            Text(property.label)
                .font(.caption)
            Picker(property.label, selection: binding) {
                ForEach(options, id: \.self) { opt in
                    Text(opt).tag(opt)
                }
            }
            .pickerStyle(.menu)
            .labelsHidden()
        }
        .padding(.vertical, 2)
        .propertyContextMenu(property: property, prefix: metadataPrefix)
    }

    // MARK: - Spin (Stepper)

    private var spinView: some View {
        let minVal = Int(property.min ?? 0)
        let maxVal = Int(property.max ?? 100)
        let stored = Int(rawValue) ?? Int(defaultValueString) ?? 0
        let binding = Binding<Int>(
            get: { stored },
            set: { writeValue(String($0)) }
        )
        // Editable value field + Stepper. The field is essential for
        // large default ranges like FreezeEffectAtFrame (default
        // 999999) — no user is scrolling from 999999 down to 100 via
        // ±1 taps. Commit on return / blur, clamped to [min, max].
        return HStack {
            Text(property.label)
                .font(.caption)
            Spacer()
            EditableNumberField(
                storedInt: stored,
                min: minVal,
                max: maxVal,
                divisor: 1,
                commit: { newInt in writeValue(String(newInt)) }
            )
            Stepper("", value: binding, in: minVal...maxVal)
                .labelsHidden()
        }
        .padding(.vertical, 2)
        .propertyContextMenu(property: property, prefix: metadataPrefix)
    }

    // MARK: - Text

    private var textView: some View {
        let binding = Binding<String>(
            get: { rawValue },
            set: { writeValue($0) }
        )
        return VStack(alignment: .leading, spacing: 2) {
            Text(property.label)
                .font(.caption)
            TextField("", text: binding)
                .textFieldStyle(.roundedBorder)
                .font(.caption2)
        }
        .padding(.vertical, 2)
        .propertyContextMenu(property: property, prefix: metadataPrefix)
    }

    // MARK: - Custom / Unsupported

    /// Preserve-on-touch fallback for any JSON property whose
    /// controlType we don't dispatch. Shows the property's current
    /// stored value (read-only) so the user can verify their
    /// desktop-authored setting survived the round-trip. Nothing
    /// here writes, so unknown types can't be corrupted.
    private var customPlaceholder: some View {
        placeholderRow(kind: "custom")
    }

    private var unsupportedPlaceholder: some View {
        placeholderRow(kind: property.controlType)
    }

    @ViewBuilder
    private func placeholderRow(kind: String) -> some View {
        let stored = rawValue
        VStack(alignment: .leading, spacing: 2) {
            HStack {
                Text(property.label.isEmpty ? property.id : property.label)
                    .font(.caption)
                    .foregroundStyle(.secondary)
                Spacer()
                Text("(\(kind))")
                    .font(.caption2)
                    .foregroundStyle(.tertiary)
            }
            // Surface the stored value so the user knows nothing was
            // silently dropped. Empty stored → omit the row rather
            // than showing a mysterious blank.
            if !stored.isEmpty {
                Text(stored)
                    .font(.caption2)
                    .foregroundStyle(.tertiary)
                    .lineLimit(2)
                    .truncationMode(.middle)
                    .textSelection(.enabled)
            }
        }
        .padding(.vertical, 2)
    }
}
